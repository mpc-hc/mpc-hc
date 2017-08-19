/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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
#include "BufferFilter.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
    {&MEDIATYPE_NULL, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_NULL, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
    {const_cast<LPWSTR>(L"Input"), FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesIn), sudPinTypesIn},
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CBufferFilter), BufferFilterName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CBufferFilter>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CBufferFilter
//

CBufferFilter::CBufferFilter(LPUNKNOWN lpunk, HRESULT* phr)
    : CTransformFilter(NAME("CBufferFilter"), lpunk, __uuidof(this))
    , m_nSamplesToBuffer(2)
{
    HRESULT hr = S_OK;

    do {
        m_pInput = DEBUG_NEW CTransformInputPin(NAME("Transform input pin"), this, &hr, L"In");
        if (!m_pInput) {
            hr = E_OUTOFMEMORY;
        }
        if (FAILED(hr)) {
            break;
        }

        m_pOutput = DEBUG_NEW CBufferFilterOutputPin(this, &hr);
        if (!m_pOutput) {
            hr = E_OUTOFMEMORY;
        }
        if (FAILED(hr)) {
            delete m_pInput;
            m_pInput = nullptr;
            break;
        }
    } while (false);

    if (phr) {
        *phr = hr;
    }
}

CBufferFilter::~CBufferFilter()
{
}

STDMETHODIMP CBufferFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(IBufferFilter)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IBufferFilter

STDMETHODIMP CBufferFilter::SetBuffers(int nBuffers)
{
    if (!m_pOutput) {
        return E_FAIL;
    }

    if (m_pOutput->IsConnected()) { // TODO: allow "on-the-fly" changes
        return VFW_E_ALREADY_CONNECTED;
    }

    m_nSamplesToBuffer = nBuffers;

    return S_OK;
}

STDMETHODIMP_(int) CBufferFilter::GetBuffers()
{
    return m_nSamplesToBuffer;
}

STDMETHODIMP_(int) CBufferFilter::GetFreeBuffers()
{
    CBufferFilterOutputPin* pPin = static_cast<CBufferFilterOutputPin*>(m_pOutput);
    return (pPin && pPin->m_pOutputQueue ? (m_nSamplesToBuffer - pPin->m_pOutputQueue->GetQueueCount()) : 0);
}

STDMETHODIMP CBufferFilter::SetPriority(DWORD dwPriority)
{
    CBufferFilterOutputPin* pPin = static_cast<CBufferFilterOutputPin*>(m_pOutput);
    return (pPin && pPin->m_pOutputQueue ? (pPin->m_pOutputQueue->SetPriority(dwPriority) ? S_OK : E_FAIL) : E_UNEXPECTED);
}

//

HRESULT CBufferFilter::Receive(IMediaSample* pSample)
{
    ASSERT(pSample);
    CheckPointer(pSample, E_POINTER);
    ASSERT(m_pOutput);
    CheckPointer(m_pOutput, E_POINTER);

    /*  Check for other streams and pass them on */
    AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
    if (pProps->dwStreamId != AM_STREAM_MEDIA) {
        return m_pOutput->Deliver(pSample);
    }

    HRESULT hr;
    IMediaSample* pOutSample;

    // Set up the output sample
    hr = InitializeOutputSample(pSample, &pOutSample);

    if (FAILED(hr)) {
        return hr;
    }

    // Start timing the transform (if PERF is defined)
    MSR_START(m_idTransform);

    // have the derived class transform the data

    hr = Transform(pSample, pOutSample);

    // Stop the clock and log it (if PERF is defined)
    MSR_STOP(m_idTransform);

    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, 1, _T("Error from transform")));
    } else {
        // the Transform() function can return S_FALSE to indicate that the
        // sample should not be delivered; we only deliver the sample if it's
        // really S_OK (same as NOERROR, of course.)
        if (hr == NOERROR) {
            hr = m_pOutput->Deliver(pOutSample);
            m_bSampleSkipped = FALSE;   // last thing no longer dropped
        } else {
            // S_FALSE returned from Transform is a PRIVATE agreement
            // We should return NOERROR from Receive() in this cause because returning S_FALSE
            // from Receive() means that this is the end of the stream and no more data should
            // be sent.
            if (S_FALSE == hr) {

                //  Release the sample before calling notify to avoid
                //  deadlocks if the sample holds a lock on the system
                //  such as DirectDraw buffers do
                pOutSample->Release();
                m_bSampleSkipped = TRUE;
                if (!m_bQualityChanged) {
                    NotifyEvent(EC_QUALITY_CHANGE, 0, 0);
                    m_bQualityChanged = TRUE;
                }
                return NOERROR;
            }
        }
    }

    // release the output buffer. If the connected pin still needs it,
    // it will have addrefed it itself.
    pOutSample->Release();

    return hr;
}

HRESULT CBufferFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    BYTE* pDataIn  = nullptr;
    BYTE* pDataOut = nullptr;

    long len  = pIn->GetActualDataLength();
    long size = pOut->GetSize();

    if (FAILED(pIn->GetPointer(&pDataIn)) || !pDataIn || FAILED(pOut->GetPointer(&pDataOut)) || !pDataOut
            || len > size || len <= 0) {
        return S_FALSE;
    }

    memcpy(pDataOut, pDataIn, std::min(len, size));

    pOut->SetActualDataLength(std::min(len, size));

    return S_OK;
}

HRESULT CBufferFilter::CheckInputType(const CMediaType* mtIn)
{
    return S_OK;
}

HRESULT CBufferFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    return mtIn->MatchesPartial(mtOut) ? S_OK : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CBufferFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    CComPtr<IMemAllocator> pAllocatorIn;
    m_pInput->GetAllocator(&pAllocatorIn);
    if (!pAllocatorIn) {
        return E_UNEXPECTED;
    }

    pAllocatorIn->GetProperties(pProperties);

    pProperties->cBuffers = std::max<long>(m_nSamplesToBuffer, pProperties->cBuffers);

    HRESULT hr;
    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    return (pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
            ? E_FAIL
            : NOERROR);
}

HRESULT CBufferFilter::GetMediaType(int iPosition, CMediaType* pMediaType)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    // TODO: offer all input types from upstream and allow reconnection at least in stopped state
    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    CopyMediaType(pMediaType, &m_pInput->CurrentMediaType());

    return S_OK;
}

HRESULT CBufferFilter::StopStreaming()
{
    CBufferFilterOutputPin* pPin = static_cast<CBufferFilterOutputPin*>(m_pOutput);
    if (m_pInput && pPin && pPin->m_pOutputQueue) {
        while (!m_pInput->IsFlushing() && pPin->m_pOutputQueue->GetQueueCount() > 0) {
            Sleep(50);
        }
    }

    return __super::StopStreaming();
}

//
// CBufferFilterOutputPin
//

CBufferFilterOutputPin::CBufferFilterOutputPin(CTransformFilter* pFilter, HRESULT* phr)
    : CTransformOutputPin(NAME("CBufferFilterOutputPin"), pFilter, phr, L"Out")
{
}

HRESULT CBufferFilterOutputPin::Active()
{
    CAutoLock lock_it(m_pLock);

    if (m_Connected && !m_pOutputQueue) {
        HRESULT hr = NOERROR;

        m_pOutputQueue.Attach(DEBUG_NEW CBufferFilterOutputQueue(m_Connected, &hr));
        if (!m_pOutputQueue) {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr)) {
            m_pOutputQueue.Free();
            return hr;
        }
    }

    return __super::Active();
}

HRESULT CBufferFilterOutputPin::Inactive()
{
    CAutoLock lock_it(m_pLock);
    m_pOutputQueue.Free();
    return __super::Inactive();
}

HRESULT CBufferFilterOutputPin::Deliver(IMediaSample* pMediaSample)
{
    if (!m_pOutputQueue) {
        return NOERROR;
    }
    pMediaSample->AddRef();
    return m_pOutputQueue->Receive(pMediaSample);
}

HRESULT CBufferFilterOutputPin::DeliverEndOfStream()
{
    CallQueue(EOS());
}

HRESULT CBufferFilterOutputPin::DeliverBeginFlush()
{
    CallQueue(BeginFlush());
}

HRESULT CBufferFilterOutputPin::DeliverEndFlush()
{
    CallQueue(EndFlush());
}

HRESULT CBufferFilterOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    CallQueue(NewSegment(tStart, tStop, dRate));
}
