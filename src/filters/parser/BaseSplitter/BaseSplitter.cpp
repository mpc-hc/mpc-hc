/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "../../../DSUtil/DSUtil.h"
#include "moreuuids.h"
#include "../../switcher/AudioSwitcher/AudioSwitcher.h"
#include "BaseSplitter.h"
#include <algorithm>


//
// CPacketQueue
//

CPacketQueue::CPacketQueue() : m_size(0)
{
}

void CPacketQueue::Add(CAutoPtr<Packet> p)
{
    CAutoLock cAutoLock(this);

    if (p) {
        m_size += p->GetDataSize();

        if (p->bAppendable && !p->bDiscontinuity && !p->pmt
                && p->rtStart == Packet::INVALID_TIME
                && !IsEmpty() && GetTail()->rtStart != Packet::INVALID_TIME) {
            Packet* tail = GetTail();
            size_t oldsize = tail->GetCount();
            size_t newsize = tail->GetCount() + p->GetCount();
            tail->SetCount(newsize, std::max(1024, (int)newsize)); // doubles the reserved buffer size
            memcpy(tail->GetData() + oldsize, p->GetData(), p->GetCount());
            /*
            GetTail()->Append(*p); // too slow
            */
            return;
        }
    }

    AddTail(p);
}

CAutoPtr<Packet> CPacketQueue::Remove()
{
    CAutoLock cAutoLock(this);
    ASSERT(__super::GetCount() > 0);
    CAutoPtr<Packet> p(RemoveHead().Detach());
    if (p) {
        m_size -= p->GetDataSize();
    }
    return p;
}

void CPacketQueue::RemoveAll()
{
    CAutoLock cAutoLock(this);
    m_size = 0;
    __super::RemoveAll();
}

int CPacketQueue::GetCount()
{
    CAutoLock cAutoLock(this);
    return (int)__super::GetCount();
}

int CPacketQueue::GetSize()
{
    CAutoLock cAutoLock(this);
    return m_size;
}

//
// CBaseSplitterInputPin
//

CBaseSplitterInputPin::CBaseSplitterInputPin(LPCTSTR pName, CBaseSplitterFilter* pFilter, CCritSec* pLock, HRESULT* phr)
    : CBasePin(pName, pFilter, pLock, phr, L"Input", PINDIR_INPUT)
{
}

CBaseSplitterInputPin::~CBaseSplitterInputPin()
{
}

HRESULT CBaseSplitterInputPin::GetAsyncReader(IAsyncReader** ppAsyncReader)
{
    CheckPointer(ppAsyncReader, E_POINTER);
    *ppAsyncReader = nullptr;
    CheckPointer(m_pAsyncReader, VFW_E_NOT_CONNECTED);
    (*ppAsyncReader = m_pAsyncReader)->AddRef();
    return S_OK;
}

STDMETHODIMP CBaseSplitterInputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CBaseSplitterInputPin::CheckMediaType(const CMediaType* pmt)
{
    return S_OK;
    /*
        return pmt->majortype == MEDIATYPE_Stream
            ? S_OK
            : E_INVALIDARG;
    */
}

HRESULT CBaseSplitterInputPin::CheckConnect(IPin* pPin)
{
    HRESULT hr;
    if (FAILED(hr = __super::CheckConnect(pPin))) {
        return hr;
    }

    return CComQIPtr<IAsyncReader>(pPin) ? S_OK : E_NOINTERFACE;
}

HRESULT CBaseSplitterInputPin::BreakConnect()
{
    HRESULT hr;

    if (FAILED(hr = __super::BreakConnect())) {
        return hr;
    }

    if (FAILED(hr = (static_cast<CBaseSplitterFilter*>(m_pFilter))->BreakConnect(PINDIR_INPUT, this))) {
        return hr;
    }

    m_pAsyncReader.Release();

    return S_OK;
}

HRESULT CBaseSplitterInputPin::CompleteConnect(IPin* pPin)
{
    HRESULT hr;

    if (FAILED(hr = __super::CompleteConnect(pPin))) {
        return hr;
    }

    CheckPointer(pPin, E_POINTER);
    m_pAsyncReader = pPin;
    CheckPointer(m_pAsyncReader, E_NOINTERFACE);

    if (FAILED(hr = (static_cast<CBaseSplitterFilter*>(m_pFilter))->CompleteConnect(PINDIR_INPUT, this))) {
        return hr;
    }

    return S_OK;
}

STDMETHODIMP CBaseSplitterInputPin::BeginFlush()
{
    return E_UNEXPECTED;
}

STDMETHODIMP CBaseSplitterInputPin::EndFlush()
{
    return E_UNEXPECTED;
}

//
// CBaseSplitterOutputPin
//

CBaseSplitterOutputPin::CBaseSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, int nBuffers, int QueueMaxPackets)
    : CBaseOutputPin(NAME("CBaseSplitterOutputPin"), pFilter, pLock, phr, pName)
    , m_hrDeliver(S_OK) // just in case it were asked before the worker thread could be created and reset it
    , m_fFlushing(false)
    , m_fFlushed(false)
    , m_eEndFlush(TRUE)
    , m_QueueMaxPackets(QueueMaxPackets)
    , m_rtStart(0)
{
    m_mts.Copy(mts);
    m_nBuffers = std::max(nBuffers, 1);
}

CBaseSplitterOutputPin::CBaseSplitterOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, int nBuffers, int QueueMaxPackets)
    : CBaseOutputPin(NAME("CBaseSplitterOutputPin"), pFilter, pLock, phr, pName)
    , m_hrDeliver(S_OK) // just in case it were asked before the worker thread could be created and reset it
    , m_fFlushing(false)
    , m_fFlushed(false)
    , m_eEndFlush(TRUE)
    , m_QueueMaxPackets(QueueMaxPackets)
    , m_rtStart(0)
{
    m_nBuffers = std::max(nBuffers, 1);
}

CBaseSplitterOutputPin::~CBaseSplitterOutputPin()
{
}

STDMETHODIMP CBaseSplitterOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        //      riid == __uuidof(IMediaSeeking) ? m_pFilter->QueryInterface(riid, ppv) :
        QI(IMediaSeeking)
        QI(IPropertyBag)
        QI(IPropertyBag2)
        QI(IDSMPropertyBag)
        QI(IBitRateInfo)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CBaseSplitterOutputPin::SetName(LPCWSTR pName)
{
    CheckPointer(pName, E_POINTER);
    if (m_pName) {
        delete [] m_pName;
    }
    m_pName = DEBUG_NEW WCHAR[wcslen(pName) + 1];
    CheckPointer(m_pName, E_OUTOFMEMORY);
    wcscpy_s(m_pName, wcslen(pName) + 1, pName);
    return S_OK;
}

HRESULT CBaseSplitterOutputPin::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    pProperties->cBuffers = std::max<long>(pProperties->cBuffers, m_nBuffers);
    pProperties->cbBuffer = std::max((long)m_mt.lSampleSize, 1l);

    // TODO: is this still needed ?
    if (m_mt.subtype == MEDIASUBTYPE_Vorbis && m_mt.formattype == FORMAT_VorbisFormat) {
        // oh great, the oggds vorbis decoder assumes there will be two at least, stupid thing...
        pProperties->cBuffers = std::max(pProperties->cBuffers, 2l);
    }

    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CBaseSplitterOutputPin::CheckMediaType(const CMediaType* pmt)
{
    for (size_t i = 0; i < m_mts.GetCount(); i++) {
        if (*pmt == m_mts[i]) {
            return S_OK;
        }
    }

    return E_INVALIDARG;
}

HRESULT CBaseSplitterOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pLock);

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if ((size_t)iPosition >= m_mts.GetCount()) {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pmt = m_mts[iPosition];

    return S_OK;
}

STDMETHODIMP CBaseSplitterOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
    return E_NOTIMPL;
}

//

HRESULT CBaseSplitterOutputPin::Active()
{
    CAutoLock cAutoLock(m_pLock);

    if (m_Connected) {
        Create();
    }

    return __super::Active();
}

HRESULT CBaseSplitterOutputPin::Inactive()
{
    CAutoLock cAutoLock(m_pLock);

    if (ThreadExists()) {
        CallWorker(CMD_EXIT);
    }

    return __super::Inactive();
}

HRESULT CBaseSplitterOutputPin::DeliverBeginFlush()
{
    m_eEndFlush.Reset();
    m_fFlushed = false;
    m_fFlushing = true;
    m_hrDeliver = S_FALSE;
    m_queue.RemoveAll();
    HRESULT hr = IsConnected() ? GetConnected()->BeginFlush() : S_OK;
    if (S_OK != hr) {
        m_eEndFlush.Set();
    }
    return hr;
}

HRESULT CBaseSplitterOutputPin::DeliverEndFlush()
{
    if (!ThreadExists()) {
        return S_FALSE;
    }
    HRESULT hr = IsConnected() ? GetConnected()->EndFlush() : S_OK;
    m_hrDeliver = S_OK;
    m_fFlushing = false;
    m_fFlushed = true;
    m_eEndFlush.Set();
    return hr;
}

HRESULT CBaseSplitterOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    m_BitRate.rtLastDeliverTime = Packet::INVALID_TIME;
    if (m_fFlushing) {
        return S_FALSE;
    }
    m_rtStart = tStart;
    if (!ThreadExists()) {
        return S_FALSE;
    }
    HRESULT hr = __super::DeliverNewSegment(tStart, tStop, dRate);
    if (S_OK != hr) {
        return hr;
    }
    MakeISCRHappy();
    return hr;
}

int CBaseSplitterOutputPin::QueueCount()
{
    return m_queue.GetCount();
}

int CBaseSplitterOutputPin::QueueSize()
{
    return m_queue.GetSize();
}

HRESULT CBaseSplitterOutputPin::QueueEndOfStream()
{
    CAutoPtr<Packet> p;
    return QueuePacket(p); // NULL means EndOfStream
}

HRESULT CBaseSplitterOutputPin::QueuePacket(CAutoPtr<Packet> p)
{
    if (!ThreadExists()) {
        return S_FALSE;
    }

    while (S_OK == m_hrDeliver
            && ((m_queue.GetCount() > (m_QueueMaxPackets * 2) || m_queue.GetSize() > (MAXPACKETSIZE * 3 / 2))
                || ((m_queue.GetCount() > m_QueueMaxPackets || m_queue.GetSize() > MAXPACKETSIZE) && !(static_cast<CBaseSplitterFilter*>(m_pFilter))->IsAnyPinDrying()))) {
        Sleep(10);
    }

    if (S_OK != m_hrDeliver) {
        return m_hrDeliver;
    }

    m_queue.Add(p);

    return m_hrDeliver;
}

bool CBaseSplitterOutputPin::IsDiscontinuous()
{
    return m_mt.majortype    == MEDIATYPE_Text
           || m_mt.majortype == MEDIATYPE_ScriptCommand
           || m_mt.majortype == MEDIATYPE_Subtitle
           || m_mt.subtype   == MEDIASUBTYPE_DVD_SUBPICTURE
           || m_mt.subtype   == MEDIASUBTYPE_CVD_SUBPICTURE
           || m_mt.subtype   == MEDIASUBTYPE_SVCD_SUBPICTURE;
}

bool CBaseSplitterOutputPin::IsActive()
{
    CComPtr<IPin> pPin = this;
    do {
        CComPtr<IPin> pPinTo;
        CComQIPtr<IStreamSwitcherInputPin> pSSIP;
        if (S_OK == pPin->ConnectedTo(&pPinTo) && (pSSIP = pPinTo) && !pSSIP->IsActive()) {
            return false;
        }
        pPin = GetFirstPin(GetFilterFromPin(pPinTo), PINDIR_OUTPUT);
    } while (pPin);

    return true;
}

DWORD CBaseSplitterOutputPin::ThreadProc()
{
    SetThreadName(DWORD(-1), "CBaseSplitterOutputPin");
    m_hrDeliver = S_OK;
    m_fFlushing = m_fFlushed = false;
    m_eEndFlush.Set();

    // fix for Microsoft DTV-DVD Video Decoder - video freeze after STOP/PLAY
    bool iHaaliRenderConnect = false;
    CComPtr<IPin> pPinTo = this, pTmp;
    while (pPinTo && SUCCEEDED(pPinTo->ConnectedTo(&pTmp)) && (pPinTo = pTmp)) {
        pTmp = nullptr;
        CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinTo);
        if (GetCLSID(pBF) == CLSID_DXR) { // Haali Renderer
            iHaaliRenderConnect = true;
            break;
        }
        pPinTo = GetFirstPin(pBF, PINDIR_OUTPUT);
    }
    if (IsConnected() && !iHaaliRenderConnect) {
        GetConnected()->BeginFlush();
        GetConnected()->EndFlush();
    }

    for (;;) {
        Sleep(1);

        DWORD cmd;
        if (CheckRequest(&cmd)) {
            m_hThread = nullptr;
            cmd = GetRequest();
            Reply(S_OK);
            ASSERT(cmd == CMD_EXIT);
            return 0;
        }

        int cnt = 0;
        do {
            CAutoPtr<Packet> p;

            {
                CAutoLock cAutoLock(&m_queue);
                if ((cnt = m_queue.GetCount()) > 0) {
                    p.Attach(m_queue.Remove().Detach());
                }
            }

            if (S_OK == m_hrDeliver && cnt > 0) {
                ASSERT(!m_fFlushing);

                m_fFlushed = false;

                // flushing can still start here, to release a blocked deliver call

                HRESULT hr = p
                             ? DeliverPacket(p)
                             : DeliverEndOfStream();

                m_eEndFlush.Wait(); // .. so we have to wait until it is done

                if (hr != S_OK && !m_fFlushed) { // and only report the error in m_hrDeliver if we didn't flush the stream
                    // CAutoLock cAutoLock(&m_csQueueLock);
                    m_hrDeliver = hr;
                    break;
                }
            }
        } while (--cnt > 0);
    }
}

HRESULT CBaseSplitterOutputPin::DeliverPacket(CAutoPtr<Packet> p)
{
    HRESULT hr;

    long nBytes = (long)p->GetCount();

    if (nBytes == 0) {
        return S_OK;
    }

    m_BitRate.nBytesSinceLastDeliverTime += nBytes;

    if (p->rtStart != Packet::INVALID_TIME) {
        if (m_BitRate.rtLastDeliverTime == Packet::INVALID_TIME) {
            m_BitRate.rtLastDeliverTime = p->rtStart;
            m_BitRate.nBytesSinceLastDeliverTime = 0;
        }

        if (m_BitRate.rtLastDeliverTime + 10000000 < p->rtStart) {
            REFERENCE_TIME rtDiff = p->rtStart - m_BitRate.rtLastDeliverTime;

            double dSecs, dBits;

            dSecs = rtDiff / 10000000.0;
            dBits = 8.0 * m_BitRate.nBytesSinceLastDeliverTime;
            m_BitRate.nCurrentBitRate = (DWORD)(dBits / dSecs);

            m_BitRate.rtTotalTimeDelivered += rtDiff;
            m_BitRate.nTotalBytesDelivered += m_BitRate.nBytesSinceLastDeliverTime;

            dSecs = m_BitRate.rtTotalTimeDelivered / 10000000.0;
            dBits = 8.0 * m_BitRate.nTotalBytesDelivered;
            m_BitRate.nAverageBitRate = (DWORD)(dBits / dSecs);

            m_BitRate.rtLastDeliverTime = p->rtStart;
            m_BitRate.nBytesSinceLastDeliverTime = 0;
            /*
                        TRACE(_T("[%d] c: %d kbps, a: %d kbps\n"),
                            p->TrackNumber,
                            (m_brs.nCurrentBitRate+500)/1000,
                            (m_brs.nAverageBitRate+500)/1000);
            */
        }

        double dRate = 1.0;
        if (SUCCEEDED((static_cast<CBaseSplitterFilter*>(m_pFilter))->GetRate(&dRate))) {
            p->rtStart = (REFERENCE_TIME)((double)p->rtStart / dRate);
            p->rtStop = (REFERENCE_TIME)((double)p->rtStop / dRate);
        }
    }

    do {
        CComPtr<IMediaSample> pSample;
        if (S_OK != (hr = GetDeliveryBuffer(&pSample, nullptr, nullptr, 0))) {
            break;
        }

        if (nBytes > pSample->GetSize()) {
            pSample.Release();

            ALLOCATOR_PROPERTIES props, actual;
            if (S_OK != (hr = m_pAllocator->GetProperties(&props))) {
                break;
            }
            props.cbBuffer = nBytes * 3 / 2;

            if (props.cBuffers > 1) {
                if (S_OK != (hr = __super::DeliverBeginFlush())) {
                    break;
                }
                if (S_OK != (hr = __super::DeliverEndFlush())) {
                    break;
                }
            }

            if (S_OK != (hr = m_pAllocator->Decommit())) {
                break;
            }
            if (S_OK != (hr = m_pAllocator->SetProperties(&props, &actual))) {
                break;
            }
            if (S_OK != (hr = m_pAllocator->Commit())) {
                break;
            }
            if (S_OK != (hr = GetDeliveryBuffer(&pSample, nullptr, nullptr, 0))) {
                break;
            }
        }

        if (p->pmt) {
            pSample->SetMediaType(p->pmt);
            p->bDiscontinuity = true;

            CAutoLock cAutoLock(m_pLock);
            m_mts.RemoveAll();
            m_mts.Add(*p->pmt);
        }

        bool fTimeValid = p->rtStart != Packet::INVALID_TIME;

#if defined(_DEBUG) && 0
        TRACE(_T("[%d]: d%d s%d p%d, b=%d, [%20I64d - %20I64d]\n"),
              p->TrackNumber,
              p->bDiscontinuity, p->bSyncPoint, fTimeValid && p->rtStart < 0,
              nBytes, p->rtStart, p->rtStop);
#endif

        ASSERT(!p->bSyncPoint || fTimeValid);

        BYTE* pData = nullptr;
        if (S_OK != (hr = pSample->GetPointer(&pData)) || !pData) {
            break;
        }
        memcpy(pData, p->GetData(), nBytes);
        if (S_OK != (hr = pSample->SetActualDataLength(nBytes))) {
            break;
        }
        if (S_OK != (hr = pSample->SetTime(fTimeValid ? &p->rtStart : nullptr, fTimeValid ? &p->rtStop : nullptr))) {
            break;
        }
        if (S_OK != (hr = pSample->SetMediaTime(nullptr, nullptr))) {
            break;
        }
        if (S_OK != (hr = pSample->SetDiscontinuity(p->bDiscontinuity))) {
            break;
        }
        if (S_OK != (hr = pSample->SetSyncPoint(p->bSyncPoint))) {
            break;
        }
        if (S_OK != (hr = pSample->SetPreroll(fTimeValid && p->rtStart < 0))) {
            break;
        }
        if (S_OK != (hr = Deliver(pSample))) {
            break;
        }
    } while (false);

    return hr;
}

void CBaseSplitterOutputPin::MakeISCRHappy()
{
    CComPtr<IPin> pPinTo = this, pTmp;
    while (pPinTo && SUCCEEDED(pPinTo->ConnectedTo(&pTmp)) && (pPinTo = pTmp)) {
        pTmp = nullptr;

        CComPtr<IBaseFilter> pBF = GetFilterFromPin(pPinTo);

        if (GetCLSID(pBF) == GUIDFromCString(_T("{48025243-2D39-11CE-875D-00608CB78066}"))) { // ISCR
            CAutoPtr<Packet> p(DEBUG_NEW Packet());
            p->TrackNumber = DWORD_ERROR;
            p->rtStart = -1;
            p->rtStop = 0;
            p->bSyncPoint = FALSE;
            p->SetData(" ", 2);
            QueuePacket(p);
            break;
        }

        pPinTo = GetFirstPin(pBF, PINDIR_OUTPUT);
    }
}

HRESULT CBaseSplitterOutputPin::GetDeliveryBuffer(IMediaSample** ppSample, REFERENCE_TIME* pStartTime, REFERENCE_TIME* pEndTime, DWORD dwFlags)
{
    return __super::GetDeliveryBuffer(ppSample, pStartTime, pEndTime, dwFlags);
}

HRESULT CBaseSplitterOutputPin::Deliver(IMediaSample* pSample)
{
    return __super::Deliver(pSample);
}

// IMediaSeeking

STDMETHODIMP CBaseSplitterOutputPin::GetCapabilities(DWORD* pCapabilities)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetCapabilities(pCapabilities);
}

STDMETHODIMP CBaseSplitterOutputPin::CheckCapabilities(DWORD* pCapabilities)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->CheckCapabilities(pCapabilities);
}

STDMETHODIMP CBaseSplitterOutputPin::IsFormatSupported(const GUID* pFormat)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->IsFormatSupported(pFormat);
}

STDMETHODIMP CBaseSplitterOutputPin::QueryPreferredFormat(GUID* pFormat)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->QueryPreferredFormat(pFormat);
}

STDMETHODIMP CBaseSplitterOutputPin::GetTimeFormat(GUID* pFormat)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetTimeFormat(pFormat);
}

STDMETHODIMP CBaseSplitterOutputPin::IsUsingTimeFormat(const GUID* pFormat)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->IsUsingTimeFormat(pFormat);
}

STDMETHODIMP CBaseSplitterOutputPin::SetTimeFormat(const GUID* pFormat)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->SetTimeFormat(pFormat);
}

STDMETHODIMP CBaseSplitterOutputPin::GetDuration(LONGLONG* pDuration)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetDuration(pDuration);
}

STDMETHODIMP CBaseSplitterOutputPin::GetStopPosition(LONGLONG* pStop)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetStopPosition(pStop);
}

STDMETHODIMP CBaseSplitterOutputPin::GetCurrentPosition(LONGLONG* pCurrent)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetCurrentPosition(pCurrent);
}

STDMETHODIMP CBaseSplitterOutputPin::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat);
}

STDMETHODIMP CBaseSplitterOutputPin::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->SetPositionsInternal(this, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

STDMETHODIMP CBaseSplitterOutputPin::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetPositions(pCurrent, pStop);
}

STDMETHODIMP CBaseSplitterOutputPin::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetAvailable(pEarliest, pLatest);
}

STDMETHODIMP CBaseSplitterOutputPin::SetRate(double dRate)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->SetRate(dRate);
}

STDMETHODIMP CBaseSplitterOutputPin::GetRate(double* pdRate)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetRate(pdRate);
}

STDMETHODIMP CBaseSplitterOutputPin::GetPreroll(LONGLONG* pllPreroll)
{
    return (static_cast<CBaseSplitterFilter*>(m_pFilter))->GetPreroll(pllPreroll);
}

//
// CBaseSplitterFilter
//

CBaseSplitterFilter::CBaseSplitterFilter(LPCTSTR pName, LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid, int QueueMaxPackets)
    : CBaseFilter(pName, pUnk, this, clsid)
    , m_nOpenProgress(100)
    , m_fAbort(false)
    , m_rtDuration(0)
    , m_rtStart(0)
    , m_rtStop(0)
    , m_rtCurrent(0)
    , m_rtNewStart(0)
    , m_rtNewStop(0)
    , m_dRate(1.0)
    , m_fFlushing(false)
    , m_priority(THREAD_PRIORITY_NORMAL)
    , m_QueueMaxPackets(QueueMaxPackets)
    , m_rtLastStart(_I64_MIN)
    , m_rtLastStop(_I64_MIN)
{
    if (phr) {
        *phr = S_OK;
    }

    m_pInput.Attach(DEBUG_NEW CBaseSplitterInputPin(NAME("CBaseSplitterInputPin"), this, this, phr));
}

CBaseSplitterFilter::~CBaseSplitterFilter()
{
    CAutoLock cAutoLock(this);

    CAMThread::CallWorker(CMD_EXIT);
    CAMThread::Close();
}

STDMETHODIMP CBaseSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    *ppv = nullptr;

    if (m_pInput && riid == __uuidof(IFileSourceFilter)) {
        return E_NOINTERFACE;
    }

    return
        QI(IFileSourceFilter)
        QI(IMediaSeeking)
        QI(IAMOpenProgress)
        QI2(IAMMediaContent)
        QI2(IAMExtendedSeeking)
        QI(IKeyFrameInfo)
        QI(IBufferInfo)
        QI(IPropertyBag)
        QI(IPropertyBag2)
        QI(IDSMPropertyBag)
        QI(IDSMResourceBag)
        QI(IDSMChapterBag)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

CBaseSplitterOutputPin* CBaseSplitterFilter::GetOutputPin(DWORD TrackNum)
{
    CAutoLock cAutoLock(&m_csPinMap);

    CBaseSplitterOutputPin* pPin = nullptr;
    m_pPinMap.Lookup(TrackNum, pPin);
    return pPin;
}

DWORD CBaseSplitterFilter::GetOutputTrackNum(CBaseSplitterOutputPin* pPin)
{
    CAutoLock cAutoLock(&m_csPinMap);

    POSITION pos = m_pPinMap.GetStartPosition();
    while (pos) {
        DWORD TrackNum;
        CBaseSplitterOutputPin* pPinTmp;
        m_pPinMap.GetNextAssoc(pos, TrackNum, pPinTmp);
        if (pPinTmp == pPin) {
            return TrackNum;
        }
    }

    return DWORD_ERROR;
}

HRESULT CBaseSplitterFilter::RenameOutputPin(DWORD TrackNumSrc, DWORD TrackNumDst, const AM_MEDIA_TYPE* pmt)
{
    CAutoLock cAutoLock(&m_csPinMap);

    CBaseSplitterOutputPin* pPin;
    if (m_pPinMap.Lookup(TrackNumSrc, pPin)) {
        if (CComQIPtr<IPin> pPinTo = pPin->GetConnected()) {
            if (pmt && S_OK != pPinTo->QueryAccept(pmt)) {
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }

        m_pPinMap.RemoveKey(TrackNumSrc);
        m_pPinMap[TrackNumDst] = pPin;

        if (pmt) {
            CAutoLock cAutoLock2(&m_csmtnew);
            m_mtnew[TrackNumDst] = *pmt;
        }

        return S_OK;
    }

    return E_FAIL;
}

HRESULT CBaseSplitterFilter::AddOutputPin(DWORD TrackNum, CAutoPtr<CBaseSplitterOutputPin> pPin)
{
    CAutoLock cAutoLock(&m_csPinMap);

    if (!pPin) {
        return E_INVALIDARG;
    }
    m_pPinMap[TrackNum] = pPin;
    m_pOutputs.AddTail(pPin);
    return S_OK;
}

HRESULT CBaseSplitterFilter::DeleteOutputs()
{
    m_rtDuration = 0;

    m_pRetiredOutputs.RemoveAll();

    CAutoLock cAutoLockF(this);
    if (m_State != State_Stopped) {
        return VFW_E_NOT_STOPPED;
    }

    while (m_pOutputs.GetCount()) {
        CAutoPtr<CBaseSplitterOutputPin> pPin(m_pOutputs.RemoveHead().Detach());
        if (IPin* pPinTo = pPin->GetConnected()) {
            pPinTo->Disconnect();
        }
        pPin->Disconnect();
        // we can't just let it be deleted now, something might have AddRefed on it (graphedit...)
        m_pRetiredOutputs.AddTail(pPin);
    }

    CAutoLock cAutoLockPM(&m_csPinMap);
    m_pPinMap.RemoveAll();

    CAutoLock cAutoLockMT(&m_csmtnew);
    m_mtnew.RemoveAll();

    RemoveAll();
    ResRemoveAll();
    ChapRemoveAll();

    m_fontinst.UninstallFonts();

    m_pSyncReader.Release();

    return S_OK;
}

void CBaseSplitterFilter::DeliverBeginFlush()
{
    m_fFlushing = true;
    POSITION pos = m_pOutputs.GetHeadPosition();
    while (pos) {
        m_pOutputs.GetNext(pos)->DeliverBeginFlush();
    }
}

void CBaseSplitterFilter::DeliverEndFlush()
{
    POSITION pos = m_pOutputs.GetHeadPosition();
    while (pos) {
        m_pOutputs.GetNext(pos)->DeliverEndFlush();
    }
    m_fFlushing = false;
    m_eEndFlush.Set();
}

#pragma warning(push)
#pragma warning(disable: 4702)
DWORD CBaseSplitterFilter::ThreadProc()
{
    if (m_pSyncReader) {
        m_pSyncReader->SetBreakEvent(GetRequestHandle());
    }

    if (!DemuxInit()) {
        for (;;) {
            DWORD cmd = GetRequest();
            if (cmd == CMD_EXIT) {
                CAMThread::m_hThread = nullptr;
            }
            Reply(S_OK);
            if (cmd == CMD_EXIT) {
                return 0;
            }
        }
    }

    m_eEndFlush.Set();
    m_fFlushing = false;

    for (DWORD cmd = DWORD_ERROR; ; cmd = GetRequest()) {
        if (cmd == CMD_EXIT) {
            m_hThread = nullptr;
            Reply(S_OK);
            return 0;
        }

        SetThreadPriority(m_hThread, m_priority = THREAD_PRIORITY_NORMAL);

        m_rtStart = m_rtNewStart;
        m_rtStop = m_rtNewStop;

        DemuxSeek(m_rtStart);

        if (cmd != DWORD_ERROR) {
            Reply(S_OK);
        }

        m_eEndFlush.Wait();

        m_pActivePins.RemoveAll();

        POSITION pos = m_pOutputs.GetHeadPosition();
        while (pos && !m_fFlushing) {
            CBaseSplitterOutputPin* pPin = m_pOutputs.GetNext(pos);
            if (pPin->IsConnected() && pPin->IsActive()) {
                m_pActivePins.AddTail(pPin);
                pPin->DeliverNewSegment(m_rtStart, m_rtStop, m_dRate);
            }
        }

        do {
            m_bDiscontinuitySent.RemoveAll();
        } while (!DemuxLoop());

        pos = m_pActivePins.GetHeadPosition();
        while (pos && !CheckRequest(&cmd)) {
            m_pActivePins.GetNext(pos)->QueueEndOfStream();
        }
    }
    UNREACHABLE_CODE(); // we should only exit via CMD_EXIT
#pragma warning(pop)
}

HRESULT CBaseSplitterFilter::DeliverPacket(CAutoPtr<Packet> p)
{
    HRESULT hr = S_FALSE;

    CBaseSplitterOutputPin* pPin = GetOutputPin(p->TrackNumber);
    if (!pPin || !pPin->IsConnected() || !m_pActivePins.Find(pPin)) {
        return S_FALSE;
    }

    if (p->rtStart != Packet::INVALID_TIME) {
        m_rtCurrent = p->rtStart;

        p->rtStart -= m_rtStart;
        p->rtStop -= m_rtStart;

        ASSERT(p->rtStart <= p->rtStop);
    }

    {
        CAutoLock cAutoLock(&m_csmtnew);

        CMediaType mt;
        if (m_mtnew.Lookup(p->TrackNumber, mt)) {
            p->pmt = CreateMediaType(&mt);
            m_mtnew.RemoveKey(p->TrackNumber);
        }
    }

    if (!m_bDiscontinuitySent.Find(p->TrackNumber)) {
        p->bDiscontinuity = TRUE;
    }

    DWORD TrackNumber = p->TrackNumber;
    BOOL bDiscontinuity = p->bDiscontinuity;

#if defined(_DEBUG) && 0
    TRACE(_T("[%d]: d%d s%d p%d, b=%d, [%20I64d - %20I64d]\n"),
          p->TrackNumber,
          p->bDiscontinuity, p->bSyncPoint, p->rtStart != Packet::INVALID_TIME && p->rtStart < 0,
          p->GetCount(), p->rtStart, p->rtStop);
#endif

    hr = pPin->QueuePacket(p);

    if (S_OK != hr) {
        if (POSITION pos = m_pActivePins.Find(pPin)) {
            m_pActivePins.RemoveAt(pos);
        }

        if (!m_pActivePins.IsEmpty()) { // only die when all pins are down
            hr = S_OK;
        }

        return hr;
    }

    if (bDiscontinuity) {
        m_bDiscontinuitySent.AddTail(TrackNumber);
    }

    return hr;
}

bool CBaseSplitterFilter::IsAnyPinDrying()
{
    int totalcount = 0, totalsize = 0;

    POSITION pos = m_pActivePins.GetHeadPosition();
    while (pos) {
        CBaseSplitterOutputPin* pPin = m_pActivePins.GetNext(pos);
        int count = pPin->QueueCount();
        int size = pPin->QueueSize();
        if (!pPin->IsDiscontinuous() && (count < MINPACKETS || size < MINPACKETSIZE)) {
            //          if (m_priority != THREAD_PRIORITY_ABOVE_NORMAL && (count < MINPACKETS/3 || size < MINPACKETSIZE/3))
            if (m_priority != THREAD_PRIORITY_BELOW_NORMAL && (count < MINPACKETS / 3 || size < MINPACKETSIZE / 3)) {
                // SetThreadPriority(m_hThread, m_priority = THREAD_PRIORITY_ABOVE_NORMAL);
                POSITION pos2 = m_pOutputs.GetHeadPosition();
                while (pos2) {
                    m_pOutputs.GetNext(pos2)->SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
                }
                m_priority = THREAD_PRIORITY_BELOW_NORMAL;
            }
            return true;
        }
        totalcount += count;
        totalsize += size;
    }

    if (m_priority != THREAD_PRIORITY_NORMAL && (totalcount > m_QueueMaxPackets * 2 / 3 || totalsize > MAXPACKETSIZE * 2 / 3)) {
        //      SetThreadPriority(m_hThread, m_priority = THREAD_PRIORITY_NORMAL);
        POSITION pos2 = m_pOutputs.GetHeadPosition();
        while (pos2) {
            m_pOutputs.GetNext(pos2)->SetThreadPriority(THREAD_PRIORITY_NORMAL);
        }
        m_priority = THREAD_PRIORITY_NORMAL;
    }

    if (totalcount < m_QueueMaxPackets && totalsize < MAXPACKETSIZE) {
        return true;
    }

    return false;
}

HRESULT CBaseSplitterFilter::BreakConnect(PIN_DIRECTION dir, CBasePin* pPin)
{
    CheckPointer(pPin, E_POINTER);

    if (dir == PINDIR_INPUT) {
        DeleteOutputs();
    } else if (dir == PINDIR_OUTPUT) {
    } else {
        return E_UNEXPECTED;
    }

    return S_OK;
}

HRESULT CBaseSplitterFilter::CompleteConnect(PIN_DIRECTION dir, CBasePin* pPin)
{
    CheckPointer(pPin, E_POINTER);

    if (dir == PINDIR_INPUT) {
        CBaseSplitterInputPin* pIn = static_cast<CBaseSplitterInputPin*>(pPin);

        HRESULT hr;

        CComPtr<IAsyncReader> pAsyncReader;
        if (FAILED(hr = pIn->GetAsyncReader(&pAsyncReader))
                || FAILED(hr = DeleteOutputs())
                || FAILED(hr = CreateOutputs(pAsyncReader))) {
            return hr;
        }

        ChapSort();

        m_pSyncReader = pAsyncReader;
    } else if (dir == PINDIR_OUTPUT) {
        m_pRetiredOutputs.RemoveAll();
    } else {
        return E_UNEXPECTED;
    }

    return S_OK;
}

int CBaseSplitterFilter::GetPinCount()
{
    return (m_pInput ? 1 : 0) + (int)m_pOutputs.GetCount();
}

CBasePin* CBaseSplitterFilter::GetPin(int n)
{
    CAutoLock cAutoLock(this);

    if (n >= 0 && n < (int)m_pOutputs.GetCount()) {
        if (POSITION pos = m_pOutputs.FindIndex(n)) {
            return m_pOutputs.GetAt(pos);
        }
    }

    if (n == (int)m_pOutputs.GetCount() && m_pInput) {
        return m_pInput;
    }

    return nullptr;
}

STDMETHODIMP CBaseSplitterFilter::Stop()
{
    CAutoLock cAutoLock(this);

    DeliverBeginFlush();
    CallWorker(CMD_EXIT);
    DeliverEndFlush();

    HRESULT hr;
    if (FAILED(hr = __super::Stop())) {
        return hr;
    }

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::Pause()
{
    CAutoLock cAutoLock(this);

    FILTER_STATE fs = m_State;

    HRESULT hr;
    if (FAILED(hr = __super::Pause())) {
        return hr;
    }

    if (fs == State_Stopped) {
        Create();
    }

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cAutoLock(this);

    HRESULT hr;
    if (FAILED(hr = __super::Run(tStart))) {
        return hr;
    }

    return S_OK;
}

// IFileSourceFilter

STDMETHODIMP CBaseSplitterFilter::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    CheckPointer(pszFileName, E_POINTER);

    m_fn = pszFileName;
    HRESULT hr = E_FAIL;
    CComPtr<IAsyncReader> pAsyncReader;
    CAtlList<CHdmvClipInfo::PlaylistItem> Items;
    CAtlList<CHdmvClipInfo::PlaylistChapter> Chapters;

    if (BuildPlaylist(pszFileName, Items)) {
        pAsyncReader = (IAsyncReader*)DEBUG_NEW CAsyncFileReader(Items, hr);
    } else {
        pAsyncReader = (IAsyncReader*)DEBUG_NEW CAsyncFileReader(CString(pszFileName), hr);
    }

    if (FAILED(hr)
            || FAILED(hr = DeleteOutputs())
            || FAILED(hr = CreateOutputs(pAsyncReader))) {
        m_fn.Empty();
        return hr;
    }

    if (BuildChapters(pszFileName, Items, Chapters)) {
        POSITION pos = Chapters.GetHeadPosition();
        int i = 1;
        while (pos) {
            CString str;
            CHdmvClipInfo::PlaylistChapter& chap = Chapters.GetNext(pos);
            if (chap.m_nMarkType == CHdmvClipInfo::EntryMark) {
                str.Format(_T("Chapter %d"), i);
                ChapAppend(chap.m_rtTimestamp, str);
                i++;
            }
        }
    }

    ChapSort();

    m_pSyncReader = pAsyncReader;

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }
    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);
    return S_OK;
}

LPCTSTR CBaseSplitterFilter::GetPartFilename(IAsyncReader* pAsyncReader)
{
    CComQIPtr<IFileHandle>  pFH = pAsyncReader;
    return pFH ? pFH->GetFileName() : (LPCWSTR)m_fn;
}

// IMediaSeeking

STDMETHODIMP CBaseSplitterFilter::GetCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);

    *pCapabilities = AM_SEEKING_CanGetStopPos | AM_SEEKING_CanGetDuration
                     | AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanSeekForwards | AM_SEEKING_CanSeekBackwards;

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::CheckCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);
    if (*pCapabilities == 0) {
        return S_OK;
    }
    DWORD caps;
    GetCapabilities(&caps);
    if ((caps&*pCapabilities) == 0) {
        return E_FAIL;
    }
    if (caps == *pCapabilities) {
        return S_OK;
    }
    return S_FALSE;
}

STDMETHODIMP CBaseSplitterFilter::IsFormatSupported(const GUID* pFormat)
{
    return !pFormat ? E_POINTER : *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

STDMETHODIMP CBaseSplitterFilter::QueryPreferredFormat(GUID* pFormat)
{
    return GetTimeFormat(pFormat);
}

STDMETHODIMP CBaseSplitterFilter::GetTimeFormat(GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);

    *pFormat = TIME_FORMAT_MEDIA_TIME;

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::IsUsingTimeFormat(const GUID* pFormat)
{
    return IsFormatSupported(pFormat);
}

STDMETHODIMP CBaseSplitterFilter::SetTimeFormat(const GUID* pFormat)
{
    return S_OK == IsFormatSupported(pFormat) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CBaseSplitterFilter::GetDuration(LONGLONG* pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    *pDuration = m_rtDuration;
    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::GetStopPosition(LONGLONG* pStop)
{
    return GetDuration(pStop);
}

STDMETHODIMP CBaseSplitterFilter::GetCurrentPosition(LONGLONG* pCurrent)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseSplitterFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseSplitterFilter::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    return SetPositionsInternal(this, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

STDMETHODIMP CBaseSplitterFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    if (pCurrent) {
        *pCurrent = m_rtCurrent;
    }
    if (pStop) {
        *pStop = m_rtStop;
    }
    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
    if (pEarliest) {
        *pEarliest = 0;
    }
    return GetDuration(pLatest);
}

STDMETHODIMP CBaseSplitterFilter::SetRate(double dRate)
{
    HRESULT hr = E_INVALIDARG;

    if (dRate > 0.0) {
        m_dRate = dRate;
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CBaseSplitterFilter::GetRate(double* pdRate)
{
    CheckPointer(pdRate, E_POINTER);

    *pdRate = m_dRate;

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::GetPreroll(LONGLONG* pllPreroll)
{
    CheckPointer(pllPreroll, E_POINTER);

    *pllPreroll = 0;

    return S_OK;
}

HRESULT CBaseSplitterFilter::SetPositionsInternal(void* id, LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    CAutoLock cAutoLock(this);

    if (!pCurrent && !pStop
            || (dwCurrentFlags & AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning
            && (dwStopFlags & AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning) {
        return S_OK;
    }

    REFERENCE_TIME rtCurrent = m_rtCurrent;
    REFERENCE_TIME rtStop = m_rtStop;

    if (pCurrent)
        switch (dwCurrentFlags & AM_SEEKING_PositioningBitsMask) {
            case AM_SEEKING_NoPositioning:
                break;
            case AM_SEEKING_AbsolutePositioning:
                rtCurrent = *pCurrent;
                break;
            case AM_SEEKING_RelativePositioning:
                rtCurrent = rtCurrent + *pCurrent;
                break;
            case AM_SEEKING_IncrementalPositioning:
                rtCurrent = rtCurrent + *pCurrent;
                break;
        }

    if (pStop)
        switch (dwStopFlags & AM_SEEKING_PositioningBitsMask) {
            case AM_SEEKING_NoPositioning:
                break;
            case AM_SEEKING_AbsolutePositioning:
                rtStop = *pStop;
                break;
            case AM_SEEKING_RelativePositioning:
                rtStop += *pStop;
                break;
            case AM_SEEKING_IncrementalPositioning:
                rtStop = rtCurrent + *pStop;
                break;
        }

    if (m_rtCurrent == rtCurrent && m_rtStop == rtStop) {
        return S_OK;
    }

    if (m_rtLastStart == rtCurrent && m_rtLastStop == rtStop && !m_LastSeekers.Find(id)) {
        m_LastSeekers.AddTail(id);
        return S_OK;
    }

    m_rtLastStart = rtCurrent;
    m_rtLastStop = rtStop;
    m_LastSeekers.RemoveAll();
    m_LastSeekers.AddTail(id);

    DbgLog((LOG_TRACE, 0, _T("Seek Started %I64d"), rtCurrent));

    m_rtNewStart = m_rtCurrent = rtCurrent;
    m_rtNewStop = rtStop;

    if (ThreadExists()) {
        DeliverBeginFlush();
        CallWorker(CMD_SEEK);
        DeliverEndFlush();
    }

    DbgLog((LOG_TRACE, 0, _T("Seek Ended")));

    return S_OK;
}

// IAMOpenProgress

STDMETHODIMP CBaseSplitterFilter::QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent)
{
    CheckPointer(pllTotal, E_POINTER);
    CheckPointer(pllCurrent, E_POINTER);

    *pllTotal = 100;
    *pllCurrent = m_nOpenProgress;

    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::AbortOperation()
{
    m_fAbort = true;
    return S_OK;
}

// IAMMediaContent

STDMETHODIMP CBaseSplitterFilter::get_AuthorName(BSTR* pbstrAuthorName)
{
    return GetProperty(L"AUTH", pbstrAuthorName);
}

STDMETHODIMP CBaseSplitterFilter::get_Title(BSTR* pbstrTitle)
{
    return GetProperty(L"TITL", pbstrTitle);
}

STDMETHODIMP CBaseSplitterFilter::get_Rating(BSTR* pbstrRating)
{
    return GetProperty(L"RTNG", pbstrRating);
}

STDMETHODIMP CBaseSplitterFilter::get_Description(BSTR* pbstrDescription)
{
    return GetProperty(L"DESC", pbstrDescription);
}

STDMETHODIMP CBaseSplitterFilter::get_Copyright(BSTR* pbstrCopyright)
{
    return GetProperty(L"CPYR", pbstrCopyright);
}

// IAMExtendedSeeking

STDMETHODIMP CBaseSplitterFilter::get_ExSeekCapabilities(long* pExCapabilities)
{
    CheckPointer(pExCapabilities, E_POINTER);
    *pExCapabilities = AM_EXSEEK_CANSEEK;
    if (ChapGetCount()) {
        *pExCapabilities |= AM_EXSEEK_MARKERSEEK;
    }
    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::get_MarkerCount(long* pMarkerCount)
{
    CheckPointer(pMarkerCount, E_POINTER);
    *pMarkerCount = (long)ChapGetCount();
    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::get_CurrentMarker(long* pCurrentMarker)
{
    CheckPointer(pCurrentMarker, E_POINTER);
    REFERENCE_TIME rt = m_rtCurrent;
    long i = ChapLookup(&rt);
    if (i < 0) {
        return E_FAIL;
    }
    *pCurrentMarker = i + 1;
    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::GetMarkerTime(long MarkerNum, double* pMarkerTime)
{
    CheckPointer(pMarkerTime, E_POINTER);
    REFERENCE_TIME rt;
    if (FAILED(ChapGet((int)MarkerNum - 1, &rt))) {
        return E_FAIL;
    }
    *pMarkerTime = (double)rt / 10000000;
    return S_OK;
}

STDMETHODIMP CBaseSplitterFilter::GetMarkerName(long MarkerNum, BSTR* pbstrMarkerName)
{
    return ChapGet((int)MarkerNum - 1, nullptr, pbstrMarkerName);
}

// IKeyFrameInfo

STDMETHODIMP CBaseSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
    return E_NOTIMPL;
}

// IBufferInfo

STDMETHODIMP_(int) CBaseSplitterFilter::GetCount()
{
    CAutoLock cAutoLock(m_pLock);

    return (int)m_pOutputs.GetCount();
}

STDMETHODIMP CBaseSplitterFilter::GetStatus(int i, int& samples, int& size)
{
    CAutoLock cAutoLock(m_pLock);

    if (POSITION pos = m_pOutputs.FindIndex(i)) {
        CBaseSplitterOutputPin* pPin = m_pOutputs.GetAt(pos);
        samples = pPin->QueueCount();
        size = pPin->QueueSize();
        return pPin->IsConnected() ? S_OK : S_FALSE;
    }

    return E_INVALIDARG;
}

STDMETHODIMP_(DWORD) CBaseSplitterFilter::GetPriority()
{
    return m_priority;
}
