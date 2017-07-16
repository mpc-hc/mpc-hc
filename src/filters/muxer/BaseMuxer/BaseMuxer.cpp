/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "BaseMuxer.h"

//
// CBaseMuxerFilter
//

CBaseMuxerFilter::CBaseMuxerFilter(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid)
    : CBaseFilter(NAME("CBaseMuxerFilter"), pUnk, this, clsid)
    , m_rtCurrent(0)
{
    if (phr) {
        *phr = S_OK;
    }
    m_pOutput.Attach(DEBUG_NEW CBaseMuxerOutputPin(L"Output", this, this, phr));
    AddInput();
}

CBaseMuxerFilter::~CBaseMuxerFilter()
{
}

STDMETHODIMP CBaseMuxerFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    *ppv = nullptr;

    return
        QI(IMediaSeeking)
        QI(IPropertyBag)
        QI(IPropertyBag2)
        QI(IDSMPropertyBag)
        QI(IDSMResourceBag)
        QI(IDSMChapterBag)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

//

void CBaseMuxerFilter::AddInput()
{
    POSITION pos = m_pInputs.GetHeadPosition();
    while (pos) {
        CBasePin* pPin = m_pInputs.GetNext(pos);
        if (!pPin->IsConnected()) {
            return;
        }
    }

    CStringW name;

    name.Format(L"Input %u", m_pInputs.GetCount() + 1);

    CBaseMuxerInputPin* pInputPin = nullptr;
    if (FAILED(CreateInput(name, &pInputPin)) || !pInputPin) {
        ASSERT(0);
        return;
    }
    CAutoPtr<CBaseMuxerInputPin> pAutoPtrInputPin(pInputPin);

    name.Format(L"~Output %u", m_pRawOutputs.GetCount() + 1);

    CBaseMuxerRawOutputPin* pRawOutputPin = nullptr;
    if (FAILED(CreateRawOutput(name, &pRawOutputPin)) || !pRawOutputPin) {
        ASSERT(0);
        return;
    }
    CAutoPtr<CBaseMuxerRawOutputPin> pAutoPtrRawOutputPin(pRawOutputPin);

    pInputPin->SetRelatedPin(pRawOutputPin);
    pRawOutputPin->SetRelatedPin(pInputPin);

    m_pInputs.AddTail(pAutoPtrInputPin);
    m_pRawOutputs.AddTail(pAutoPtrRawOutputPin);
}

HRESULT CBaseMuxerFilter::CreateInput(CStringW name, CBaseMuxerInputPin** ppPin)
{
    CheckPointer(ppPin, E_POINTER);
    HRESULT hr = S_OK;
    *ppPin = DEBUG_NEW CBaseMuxerInputPin(name, this, this, &hr);
    return hr;
}

HRESULT CBaseMuxerFilter::CreateRawOutput(CStringW name, CBaseMuxerRawOutputPin** ppPin)
{
    CheckPointer(ppPin, E_POINTER);
    HRESULT hr = S_OK;
    *ppPin = DEBUG_NEW CBaseMuxerRawOutputPin(name, this, this, &hr);
    return hr;
}

//

#pragma warning(push)
#pragma warning(disable: 4702)
DWORD CBaseMuxerFilter::ThreadProc()
{
    SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);

    POSITION pos;

    for (;;) {
        DWORD cmd = GetRequest();

        switch (cmd) {
            default:
            case CMD_EXIT:
                CAMThread::m_hThread = nullptr;
                Reply(S_OK);
                return 0;

            case CMD_RUN:
                m_pActivePins.RemoveAll();
                m_pPins.RemoveAll();

                pos = m_pInputs.GetHeadPosition();
                while (pos) {
                    CBaseMuxerInputPin* pPin = m_pInputs.GetNext(pos);
                    if (pPin->IsConnected()) {
                        m_pActivePins.AddTail(pPin);
                        m_pPins.AddTail(pPin);
                    }
                }

                m_rtCurrent = 0;

                Reply(S_OK);

                MuxInit();

                try {
                    MuxHeaderInternal();

                    while (!CheckRequest(nullptr) && m_pActivePins.GetCount()) {
                        if (m_State == State_Paused) {
                            Sleep(10);
                            continue;
                        }

                        CAutoPtr<MuxerPacket> pPacket(GetPacket().Detach());
                        if (!pPacket) {
                            Sleep(1);
                            continue;
                        }

                        if (pPacket->IsTimeValid()) {
                            m_rtCurrent = pPacket->rtStart;
                        }

                        if (pPacket->IsEOS()) {
                            m_pActivePins.RemoveAt(m_pActivePins.Find(pPacket->pPin));
                        }

                        MuxPacketInternal(pPacket);
                    }

                    MuxFooterInternal();
                } catch (HRESULT hr) {
                    CComQIPtr<IMediaEventSink>(m_pGraph)->Notify(EC_ERRORABORT, hr, 0);
                }

                m_pOutput->DeliverEndOfStream();

                pos = m_pRawOutputs.GetHeadPosition();
                while (pos) {
                    m_pRawOutputs.GetNext(pos)->DeliverEndOfStream();
                }

                m_pActivePins.RemoveAll();
                m_pPins.RemoveAll();

                break;
        }
    }
    UNREACHABLE_CODE(); // we should only exit via CMD_EXIT
#pragma warning(pop)
}

void CBaseMuxerFilter::MuxHeaderInternal()
{
    TRACE(_T("MuxHeader\n"));

    if (CComQIPtr<IBitStream> pBitStream = m_pOutput->GetBitStream()) {
        MuxHeader(pBitStream);
    }

    MuxHeader();

    //

    POSITION pos = m_pPins.GetHeadPosition();
    while (pos) {
        if (CBaseMuxerInputPin* pInput = m_pPins.GetNext(pos))
            if (CBaseMuxerRawOutputPin* pOutput = dynamic_cast<CBaseMuxerRawOutputPin*>(pInput->GetRelatedPin())) {
                pOutput->MuxHeader(pInput->CurrentMediaType());
            }
    }
}

void CBaseMuxerFilter::MuxPacketInternal(const MuxerPacket* pPacket)
{
    TRACE(_T("MuxPacket pPin=%x, size=%d, s%d e%d b%d, rt=(%I64d-%I64d)\n"),
          pPacket->pPin->GetID(),
          pPacket->pData.GetCount(),
          !!(pPacket->flags & MuxerPacket::syncpoint),
          !!(pPacket->flags & MuxerPacket::eos),
          !!(pPacket->flags & MuxerPacket::bogus),
          pPacket->rtStart / 10000, pPacket->rtStop / 10000);

    if (CComQIPtr<IBitStream> pBitStream = m_pOutput->GetBitStream()) {
        MuxPacket(pBitStream, pPacket);
    }

    MuxPacket(pPacket);

    if (CBaseMuxerInputPin* pInput = pPacket->pPin)
        if (CBaseMuxerRawOutputPin* pOutput = dynamic_cast<CBaseMuxerRawOutputPin*>(pInput->GetRelatedPin())) {
            pOutput->MuxPacket(pInput->CurrentMediaType(), pPacket);
        }
}

void CBaseMuxerFilter::MuxFooterInternal()
{
    TRACE(_T("MuxFooter\n"));

    if (CComQIPtr<IBitStream> pBitStream = m_pOutput->GetBitStream()) {
        MuxFooter(pBitStream);
    }

    MuxFooter();

    //

    POSITION pos = m_pPins.GetHeadPosition();
    while (pos) {
        if (CBaseMuxerInputPin* pInput = m_pPins.GetNext(pos))
            if (CBaseMuxerRawOutputPin* pOutput = dynamic_cast<CBaseMuxerRawOutputPin*>(pInput->GetRelatedPin())) {
                pOutput->MuxFooter(pInput->CurrentMediaType());
            }
    }
}

CAutoPtr<MuxerPacket> CBaseMuxerFilter::GetPacket()
{
    REFERENCE_TIME rtMin = _I64_MAX;
    CBaseMuxerInputPin* pPinMin = nullptr;
    int i = int(m_pActivePins.GetCount());

    POSITION pos = m_pActivePins.GetHeadPosition();
    while (pos) {
        CBaseMuxerInputPin* pPin = m_pActivePins.GetNext(pos);

        CAutoLock cAutoLock(&pPin->m_csQueue);
        if (!pPin->m_queue.GetCount()) {
            continue;
        }

        MuxerPacket* p = pPin->m_queue.GetHead();

        if (p->IsBogus() || !p->IsTimeValid() || p->IsEOS()) {
            pPinMin = pPin;
            i = 0;
            break;
        }

        if (p->rtStart < rtMin) {
            rtMin = p->rtStart;
            pPinMin = pPin;
        }

        i--;
    }

    CAutoPtr<MuxerPacket> pPacket;

    if (pPinMin && i == 0) {
        pPacket.Attach(pPinMin->PopPacket().Detach());
    } else {
        pos = m_pActivePins.GetHeadPosition();
        while (pos) {
            m_pActivePins.GetNext(pos)->m_evAcceptPacket.Set();
        }
    }

    return pPacket;
}

//

int CBaseMuxerFilter::GetPinCount()
{
    return int(m_pInputs.GetCount()) + (m_pOutput ? 1 : 0) + int(m_pRawOutputs.GetCount());
}

CBasePin* CBaseMuxerFilter::GetPin(int n)
{
    CAutoLock cAutoLock(this);

    if (n >= 0 && n < (int)m_pInputs.GetCount()) {
        if (POSITION pos = m_pInputs.FindIndex(n)) {
            return m_pInputs.GetAt(pos);
        }
    }

    n -= int(m_pInputs.GetCount());

    if (n == 0 && m_pOutput) {
        return m_pOutput;
    }

    n--;

    if (n >= 0 && n < (int)m_pRawOutputs.GetCount()) {
        if (POSITION pos = m_pRawOutputs.FindIndex(n)) {
            return m_pRawOutputs.GetAt(pos);
        }
    }

    //n -= int(m_pRawOutputs.GetCount());

    return nullptr;
}

STDMETHODIMP CBaseMuxerFilter::Stop()
{
    CAutoLock cAutoLock(this);

    HRESULT hr = __super::Stop();
    if (FAILED(hr)) {
        return hr;
    }

    CallWorker(CMD_EXIT);

    return hr;
}

STDMETHODIMP CBaseMuxerFilter::Pause()
{
    CAutoLock cAutoLock(this);

    FILTER_STATE fs = m_State;

    HRESULT hr = __super::Pause();
    if (FAILED(hr)) {
        return hr;
    }

    if (fs == State_Stopped && m_pOutput) {
        CAMThread::Create();
        CallWorker(CMD_RUN);
    }

    return hr;
}

STDMETHODIMP CBaseMuxerFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cAutoLock(this);

    HRESULT hr = __super::Run(tStart);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}

// IMediaSeeking

STDMETHODIMP CBaseMuxerFilter::GetCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);

    *pCapabilities = AM_SEEKING_CanGetDuration | AM_SEEKING_CanGetCurrentPos;

    return S_OK;
}

STDMETHODIMP CBaseMuxerFilter::CheckCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);
    if (*pCapabilities == 0) {
        return S_OK;
    }
    DWORD caps;
    GetCapabilities(&caps);
    caps &= *pCapabilities;
    return caps == 0 ? E_FAIL : caps == *pCapabilities ? S_OK : S_FALSE;
}

STDMETHODIMP CBaseMuxerFilter::IsFormatSupported(const GUID* pFormat)
{
    return !pFormat ? E_POINTER : *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

STDMETHODIMP CBaseMuxerFilter::QueryPreferredFormat(GUID* pFormat)
{
    return GetTimeFormat(pFormat);
}

STDMETHODIMP CBaseMuxerFilter::GetTimeFormat(GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);

    *pFormat = TIME_FORMAT_MEDIA_TIME;

    return S_OK;
}

STDMETHODIMP CBaseMuxerFilter::IsUsingTimeFormat(const GUID* pFormat)
{
    return IsFormatSupported(pFormat);
}

STDMETHODIMP CBaseMuxerFilter::SetTimeFormat(const GUID* pFormat)
{
    return S_OK == IsFormatSupported(pFormat) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CBaseMuxerFilter::GetDuration(LONGLONG* pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    *pDuration = 0;
    POSITION pos = m_pInputs.GetHeadPosition();
    while (pos) {
        REFERENCE_TIME rt = m_pInputs.GetNext(pos)->GetDuration();
        if (rt > *pDuration) {
            *pDuration = rt;
        }
    }
    return S_OK;
}

STDMETHODIMP CBaseMuxerFilter::GetStopPosition(LONGLONG* pStop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseMuxerFilter::GetCurrentPosition(LONGLONG* pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    *pCurrent = m_rtCurrent;
    return S_OK;
}

STDMETHODIMP CBaseMuxerFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseMuxerFilter::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    FILTER_STATE fs;

    if (SUCCEEDED(GetState(0, &fs)) && fs == State_Stopped) {
        POSITION pos = m_pInputs.GetHeadPosition();
        while (pos) {
            CBasePin* pPin = m_pInputs.GetNext(pos);
            CComQIPtr<IMediaSeeking> pMS = pPin->GetConnected();
            if (!pMS) {
                pMS = GetFilterFromPin(pPin->GetConnected());
            }
            if (pMS) {
                pMS->SetPositions(pCurrent, dwCurrentFlags, pStop, dwStopFlags);
            }
        }

        return S_OK;
    }

    return VFW_E_WRONG_STATE;
}

STDMETHODIMP CBaseMuxerFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseMuxerFilter::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseMuxerFilter::SetRate(double dRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseMuxerFilter::GetRate(double* pdRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseMuxerFilter::GetPreroll(LONGLONG* pllPreroll)
{
    return E_NOTIMPL;
}
