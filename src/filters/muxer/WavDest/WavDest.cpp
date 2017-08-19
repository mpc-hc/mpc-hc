/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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
#include "BaseClasses/streams.h"
#include <aviriff.h>
#include "WavDest.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
    {&MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_WAVE},
};

const AMOVIESETUP_PIN sudpPins[] = {
    {const_cast<LPWSTR>(L"Input"), FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesIn), sudPinTypesIn},
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CWavDestFilter), WavDestName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {L"WavDest", &__uuidof(CWavDestFilter), CreateInstance<CWavDestFilter>, nullptr, &sudFilter[0]}
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
// CWavDestFilter
//

CWavDestFilter::CWavDestFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CTransformFilter(NAME("WavDest filter"), pUnk, __uuidof(this))
    , m_cbWavData(0)
    , m_cbHeader(0)
{
    if (CWavDestOutputPin* pOut = DEBUG_NEW CWavDestOutputPin(this, phr)) {
        if (SUCCEEDED(*phr)) {
            m_pOutput = pOut;
        } else {
            delete pOut;
        }
    } else {
        *phr = E_OUTOFMEMORY;
        return;
    }

    if (CTransformInputPin* pIn = DEBUG_NEW CTransformInputPin(NAME("Transform input pin"), this, phr, L"In")) {
        if (SUCCEEDED(*phr)) {
            m_pInput = pIn;
        } else {
            delete pIn;
        }
    } else {
        *phr = E_OUTOFMEMORY;
        return;
    }
}

CWavDestFilter::~CWavDestFilter()
{
}

HRESULT CWavDestFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    return CheckInputType(mtIn);
}

HRESULT CWavDestFilter::Receive(IMediaSample* pSample)
{
    ULONG cbOld = m_cbWavData;
    HRESULT hr = CTransformFilter::Receive(pSample);

    // don't update the count if Deliver() downstream fails.
    if (hr != S_OK) {
        m_cbWavData = cbOld;
    }

    return hr;
}

HRESULT CWavDestFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    REFERENCE_TIME rtStart, rtEnd;

    HRESULT hr = Copy(pIn, pOut);
    if (FAILED(hr)) {
        return hr;
    }

    // Prepare it for writing
    LONG lActual = pOut->GetActualDataLength();

    if (m_cbWavData + m_cbHeader + lActual < m_cbWavData + m_cbHeader) {  // overflow
        return E_FAIL;
    }

    rtStart = m_cbWavData + m_cbHeader;
    rtEnd = rtStart + lActual;
    m_cbWavData += lActual;

    EXECUTE_ASSERT(pOut->SetTime(&rtStart, &rtEnd) == S_OK);

    return S_OK;
}

HRESULT CWavDestFilter::Copy(IMediaSample* pSource, IMediaSample* pDest) const
{
    BYTE* pSourceBuffer, * pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();

#ifdef _DEBUG
    long lDestSize = pDest->GetSize();
    ASSERT(lDestSize >= lSourceSize);
#endif

    if (FAILED(pSource->GetPointer(&pSourceBuffer)) || !pSourceBuffer) {
        return E_FAIL;
    }
    if (FAILED(pDest->GetPointer(&pDestBuffer)) || !pDestBuffer) {
        return E_FAIL;
    }

    CopyMemory((PVOID)pDestBuffer, (PVOID)pSourceBuffer, lSourceSize);

    // Copy the sample times

    REFERENCE_TIME rtTimeStart, rtTimeEnd;
    if (SUCCEEDED(pSource->GetTime(&rtTimeStart, &rtTimeEnd))) {
        pDest->SetTime(&rtTimeStart, &rtTimeEnd);
    }

    LONGLONG rtMediaStart, rtMediaEnd;
    if (SUCCEEDED(pSource->GetMediaTime(&rtMediaStart, &rtMediaEnd))) {
        pDest->SetMediaTime(&rtMediaStart, &rtMediaEnd);
    }

    // Copy the media type
    AM_MEDIA_TYPE* pMediaType;
    if (SUCCEEDED(pSource->GetMediaType(&pMediaType)) && pMediaType) {
        pDest->SetMediaType(pMediaType);
        DeleteMediaType(pMediaType);
    }

    // Copy the actual data length
    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;
}

HRESULT CWavDestFilter::CheckInputType(const CMediaType* mtIn)
{
    return mtIn->formattype == FORMAT_WaveFormatEx ? S_OK : S_FALSE;
}

HRESULT CWavDestFilter::GetMediaType(int iPosition, CMediaType* pMediaType)
{
    ASSERT(iPosition == 0 || iPosition == 1);

    if (iPosition == 0) {
        pMediaType->SetType(&MEDIATYPE_Stream);
        pMediaType->SetSubtype(&MEDIASUBTYPE_WAVE);
        return S_OK;
    }

    return VFW_S_NO_MORE_ITEMS;
}

HRESULT CWavDestFilter::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbAlign = 1;

    CComPtr<IMemAllocator> pInAlloc;
    ALLOCATOR_PROPERTIES InProps;
    if (SUCCEEDED(hr = m_pInput->GetAllocator(&pInAlloc))
            && SUCCEEDED(hr = pInAlloc->GetProperties(&InProps))) {
        pProperties->cbBuffer = InProps.cbBuffer;
    } else {
        return hr;
    }

    ASSERT(pProperties->cbBuffer);

    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    ASSERT(Actual.cBuffers == 1);

    if (pProperties->cBuffers > Actual.cBuffers
            || pProperties->cbBuffer > Actual.cbBuffer) {
        return E_FAIL;
    }

    return NOERROR;
}

// Compute the header size to allow space for us to write it at the end.
//
// 00000000    RIFF (00568BFE) 'WAVE'
// 0000000C        fmt  (00000010)
// 00000024        data (00568700)
// 0056872C
//

HRESULT CWavDestFilter::StartStreaming()
{
    // leave space for the header
    m_cbHeader = sizeof(RIFFLIST) +
                 sizeof(RIFFCHUNK) +
                 m_pInput->CurrentMediaType().FormatLength() +
                 sizeof(RIFFCHUNK);

    m_cbWavData = 0;

    return S_OK;
}

HRESULT CWavDestFilter::StopStreaming()
{
    IStream* pStream;
    if (m_pOutput->IsConnected() == FALSE) {
        return E_FAIL;
    }

    IPin* pDwnstrmInputPin = m_pOutput->GetConnected();

    if (!pDwnstrmInputPin) {
        return E_FAIL;
    }

    HRESULT hr = ((IMemInputPin*) pDwnstrmInputPin)->QueryInterface(IID_PPV_ARGS(&pStream));
    if (SUCCEEDED(hr)) {
        BYTE* pb = (BYTE*)_alloca(m_cbHeader);

        RIFFLIST* pRiffWave = (RIFFLIST*)pb;
        RIFFCHUNK* pRiffFmt = (RIFFCHUNK*)(pRiffWave + 1);
        RIFFCHUNK* pRiffData = (RIFFCHUNK*)(((BYTE*)(pRiffFmt + 1)) + m_pInput->CurrentMediaType().FormatLength());

        pRiffData->fcc = FCC('data');
        pRiffData->cb = m_cbWavData;

        pRiffFmt->fcc = FCC('fmt ');
        pRiffFmt->cb = m_pInput->CurrentMediaType().FormatLength();
        CopyMemory(pRiffFmt + 1, m_pInput->CurrentMediaType().Format(), pRiffFmt->cb);

        pRiffWave->fcc = FCC('RIFF');
        pRiffWave->cb = m_cbWavData + m_cbHeader - sizeof(RIFFCHUNK);
        pRiffWave->fccListType = FCC('WAVE');

        LARGE_INTEGER li;
        ZeroMemory(&li, sizeof(li));

        hr = pStream->Seek(li, STREAM_SEEK_SET, 0);
        if (SUCCEEDED(hr)) {
            hr = pStream->Write(pb, m_cbHeader, 0);
        }
        pStream->Release();
    }

    return hr;
}

CWavDestOutputPin::CWavDestOutputPin(CTransformFilter* pFilter, HRESULT* phr)
    : CTransformOutputPin(NAME("WavDest output pin"), pFilter, phr, L"Out")
{
}

STDMETHODIMP CWavDestOutputPin::EnumMediaTypes(IEnumMediaTypes** ppEnum)
{
    return CBaseOutputPin::EnumMediaTypes(ppEnum);
}

HRESULT CWavDestOutputPin::CheckMediaType(const CMediaType* pmt)
{
    if (pmt->majortype == MEDIATYPE_Stream && pmt->subtype == MEDIASUBTYPE_WAVE) {
        return S_OK;
    } else {
        return S_FALSE;
    }
}
