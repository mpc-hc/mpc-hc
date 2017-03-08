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
#include <algorithm>
#include <MMReg.h>
#include "BaseMuxer.h"
#include "../../../DSUtil/DSUtil.h"
#include "moreuuids.h"

#define MAXQUEUESIZE 100

//
// CBaseMuxerInputPin
//

CBaseMuxerInputPin::CBaseMuxerInputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
    : CBaseInputPin(NAME("CBaseMuxerInputPin"), pFilter, pLock, phr, pName)
    , m_rtMaxStart(_I64_MIN)
    , m_rtDuration(0)
    , m_fEOS(false)
    , m_iPacketIndex(0)
    , m_evAcceptPacket(TRUE)
{
    static int s_iID = 0;
    m_iID = s_iID++;
}

CBaseMuxerInputPin::~CBaseMuxerInputPin()
{
}

STDMETHODIMP CBaseMuxerInputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IBaseMuxerRelatedPin)
        QI(IPropertyBag)
        QI(IPropertyBag2)
        QI(IDSMPropertyBag)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

bool CBaseMuxerInputPin::IsSubtitleStream()
{
    return m_mt.majortype == MEDIATYPE_Subtitle || m_mt.majortype == MEDIATYPE_Text;
}

void CBaseMuxerInputPin::PushPacket(CAutoPtr<MuxerPacket> pPacket)
{
    for (int i = 0; m_pFilter->IsActive() && !m_bFlushing
            && !m_evAcceptPacket.Wait(1)
            && i < 1000;
            i++) {
        ;
    }

    if (!m_pFilter->IsActive() || m_bFlushing) {
        return;
    }

    CAutoLock cAutoLock(&m_csQueue);

    m_queue.AddTail(pPacket);

    if (m_queue.GetCount() >= MAXQUEUESIZE) {
        m_evAcceptPacket.Reset();
    }
}

CAutoPtr<MuxerPacket> CBaseMuxerInputPin::PopPacket()
{
    CAutoPtr<MuxerPacket> pPacket;

    CAutoLock cAutoLock(&m_csQueue);

    if (m_queue.GetCount()) {
        pPacket.Attach(m_queue.RemoveHead().Detach());
    }

    if (m_queue.GetCount() < MAXQUEUESIZE) {
        m_evAcceptPacket.Set();
    }

    return pPacket;
}

HRESULT CBaseMuxerInputPin::CheckMediaType(const CMediaType* pmt)
{
    if (pmt->formattype == FORMAT_WaveFormatEx) {
        WORD wFormatTag = ((WAVEFORMATEX*)pmt->pbFormat)->wFormatTag;
        if ((wFormatTag == WAVE_FORMAT_PCM
                || wFormatTag == WAVE_FORMAT_EXTENSIBLE
                || wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
                && pmt->subtype != FOURCCMap(wFormatTag)
                && !(pmt->subtype == MEDIASUBTYPE_PCM && wFormatTag == WAVE_FORMAT_EXTENSIBLE)
                && !(pmt->subtype == MEDIASUBTYPE_PCM && wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
                && pmt->subtype != MEDIASUBTYPE_DVD_LPCM_AUDIO
                && pmt->subtype != MEDIASUBTYPE_DOLBY_AC3
                && pmt->subtype != MEDIASUBTYPE_DTS) {
            return E_INVALIDARG;
        }
    }

    return pmt->majortype == MEDIATYPE_Video
           || pmt->majortype == MEDIATYPE_Audio && pmt->formattype != FORMAT_VorbisFormat
           || pmt->majortype == MEDIATYPE_Text && pmt->subtype == MEDIASUBTYPE_NULL && pmt->formattype == FORMAT_None
           || pmt->majortype == MEDIATYPE_Subtitle
           ? S_OK
           : E_INVALIDARG;
}

HRESULT CBaseMuxerInputPin::BreakConnect()
{
    HRESULT hr = __super::BreakConnect();
    if (FAILED(hr)) {
        return hr;
    }

    RemoveAll();

    // TODO: remove extra disconnected pins, leave one

    return hr;
}

HRESULT CBaseMuxerInputPin::CompleteConnect(IPin* pReceivePin)
{
    HRESULT hr = __super::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

    // duration

    m_rtDuration = 0;
    CComQIPtr<IMediaSeeking> pMS;
    if ((pMS = GetFilterFromPin(pReceivePin)) || (pMS = pReceivePin)) {
        pMS->GetDuration(&m_rtDuration);
    }

    // properties

    for (CComPtr<IPin> pPin = pReceivePin; pPin; pPin = GetUpStreamPin(GetFilterFromPin(pPin))) {
        if (CComQIPtr<IDSMPropertyBag> pPB = pPin) {
            ULONG cProperties = 0;
            if (SUCCEEDED(pPB->CountProperties(&cProperties)) && cProperties > 0) {
                for (ULONG iProperty = 0; iProperty < cProperties; iProperty++) {
                    PROPBAG2 PropBag;
                    ZeroMemory(&PropBag, sizeof(PropBag));
                    ULONG cPropertiesReturned = 0;
                    if (FAILED(pPB->GetPropertyInfo(iProperty, 1, &PropBag, &cPropertiesReturned))) {
                        continue;
                    }

                    HRESULT hr2;
                    CComVariant var;
                    if (SUCCEEDED(pPB->Read(1, &PropBag, nullptr, &var, &hr2)) && SUCCEEDED(hr2)) {
                        SetProperty(PropBag.pstrName, &var);
                    }

                    CoTaskMemFree(PropBag.pstrName);
                }
            }
        }
    }

    (static_cast<CBaseMuxerFilter*>(m_pFilter))->AddInput();

    return S_OK;
}

HRESULT CBaseMuxerInputPin::Active()
{
    m_rtMaxStart = _I64_MIN;
    m_fEOS = false;
    m_iPacketIndex = 0;
    m_evAcceptPacket.Set();
    return __super::Active();
}

HRESULT CBaseMuxerInputPin::Inactive()
{
    CAutoLock cAutoLock(&m_csQueue);
    m_queue.RemoveAll();
    return __super::Inactive();
}

STDMETHODIMP CBaseMuxerInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    CAutoLock cAutoLock(&m_csReceive);

    return __super::NewSegment(tStart, tStop, dRate);
}

STDMETHODIMP CBaseMuxerInputPin::Receive(IMediaSample* pSample)
{
    CAutoLock cAutoLock(&m_csReceive);

    HRESULT hr = __super::Receive(pSample);
    if (FAILED(hr)) {
        return hr;
    }

    CAutoPtr<MuxerPacket> pPacket(DEBUG_NEW MuxerPacket(this));

    long len = pSample->GetActualDataLength();

    BYTE* pData = nullptr;
    if (FAILED(pSample->GetPointer(&pData)) || !pData) {
        return S_OK;
    }

    pPacket->pData.SetCount(len);
    memcpy(pPacket->pData.GetData(), pData, len);

    if (S_OK == pSample->IsSyncPoint() || m_mt.majortype == MEDIATYPE_Audio && !m_mt.bTemporalCompression) {
        pPacket->flags |= MuxerPacket::syncpoint;
    }

    if (S_OK == pSample->GetTime(&pPacket->rtStart, &pPacket->rtStop)) {
        pPacket->flags |= MuxerPacket::timevalid;

        pPacket->rtStart += m_tStart;
        pPacket->rtStop += m_tStart;

        if ((pPacket->flags & MuxerPacket::syncpoint) && pPacket->rtStart < m_rtMaxStart) {
            pPacket->flags &= ~MuxerPacket::syncpoint;
            pPacket->flags |= MuxerPacket::bogus;
        }

        m_rtMaxStart = std::max(m_rtMaxStart, pPacket->rtStart);
    } else if (pPacket->flags & MuxerPacket::syncpoint) {
        pPacket->flags &= ~MuxerPacket::syncpoint;
        pPacket->flags |= MuxerPacket::bogus;
    }

    if (S_OK == pSample->IsDiscontinuity()) {
        pPacket->flags |= MuxerPacket::discontinuity;
    }

    pPacket->index = m_iPacketIndex++;

    PushPacket(pPacket);

    return S_OK;
}

STDMETHODIMP CBaseMuxerInputPin::EndOfStream()
{
    CAutoLock cAutoLock(&m_csReceive);

    HRESULT hr = __super::EndOfStream();
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT(!m_fEOS);

    CAutoPtr<MuxerPacket> pPacket(DEBUG_NEW MuxerPacket(this));
    pPacket->flags |= MuxerPacket::eos;
    PushPacket(pPacket);

    m_fEOS = true;

    return hr;
}
