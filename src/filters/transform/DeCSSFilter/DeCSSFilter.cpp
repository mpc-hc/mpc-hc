/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2016-2017 see Authors.txt
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
#include <atlbase.h>
#include <algorithm>
#include "DeCSSFilter.h"
#include "../../../DeCSS/DeCSSInputPin.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

#include <initguid.h>
#include "moreuuids.h"

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
    {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_NULL},
    {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
    {const_cast<LPWSTR>(L"Input"), FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesIn), sudPinTypesIn},
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CDeCSSFilter), DeCSSFilterName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CDeCSSFilter>, nullptr, &sudFilter[0]},
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
// CDeCSSFilter
//

class CKsPSInputPin : public CDeCSSInputPin
{
public:
    CKsPSInputPin(LPCTSTR pObjectName, CTransformFilter* pFilter, HRESULT* phr, LPCWSTR pName)
        : CDeCSSInputPin(pObjectName, pFilter, phr, pName) {
    }

    // IKsPropertySet
    STDMETHODIMP Set(REFGUID PropSet, ULONG Id, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength) {
        if (CComQIPtr<IKsPropertySet> pKsPS = (static_cast<CDeCSSFilter*>(m_pFilter))->m_pOutput->GetConnected()) {
            return pKsPS->Set(PropSet, Id, InstanceData, InstanceLength, PropertyData, DataLength);
        }
        return E_NOTIMPL;
    }
    STDMETHODIMP Get(REFGUID PropSet, ULONG Id, LPVOID InstanceData, ULONG InstanceLength, LPVOID PropertyData, ULONG DataLength, ULONG* pBytesReturned) {
        if (CComQIPtr<IKsPropertySet> pKsPS = (static_cast<CDeCSSFilter*>(m_pFilter))->m_pOutput->GetConnected()) {
            return pKsPS->Get(PropSet, Id, InstanceData, InstanceLength, PropertyData, DataLength, pBytesReturned);
        }
        return E_NOTIMPL;
    }
    STDMETHODIMP QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport) {
        if (CComQIPtr<IKsPropertySet> pKsPS = (static_cast<CDeCSSFilter*>(m_pFilter))->m_pOutput->GetConnected()) {
            return pKsPS->QuerySupported(PropSet, Id, pTypeSupport);
        }
        return E_NOTIMPL;
    }
};

CDeCSSFilter::CDeCSSFilter(LPUNKNOWN lpunk, HRESULT* phr)
    : CTransformFilter(NAME("CDeCSSFilter"), lpunk, __uuidof(this))
{
    HRESULT hr;
    if (!phr) {
        phr = &hr;
    }
    *phr = S_OK;

    m_pInput = DEBUG_NEW CKsPSInputPin(NAME("CKsPSInputPin"), this, phr, L"In");
    if (!m_pInput) {
        *phr = E_OUTOFMEMORY;
    }
    if (FAILED(*phr)) {
        return;
    }

    m_pOutput = DEBUG_NEW CTransformOutputPin(NAME("CTransformOutputPin"), this, phr, L"Out");
    if (!m_pOutput) {
        *phr = E_OUTOFMEMORY;
    }
    if (FAILED(*phr))  {
        delete m_pInput;
        m_pInput = nullptr;
        return;
    }
}

CDeCSSFilter::~CDeCSSFilter()
{
}

HRESULT CDeCSSFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    AM_MEDIA_TYPE* pmt;
    if (SUCCEEDED(pIn->GetMediaType(&pmt)) && pmt) {
        CMediaType mt = *pmt;
        m_pInput->SetMediaType(&mt);
        mt.majortype = m_pOutput->CurrentMediaType().majortype;
        m_pOutput->SetMediaType(&mt);
        pOut->SetMediaType(&mt);
        DeleteMediaType(pmt);
    }

    BYTE* pDataIn = nullptr;
    BYTE* pDataOut = nullptr;

    HRESULT hr;
    if (FAILED(hr = pIn->GetPointer(&pDataIn)) || FAILED(hr = pOut->GetPointer(&pDataOut))) {
        return hr;
    }

    long len = pIn->GetActualDataLength();
    long size = pOut->GetSize();

    if (len == 0 || pDataIn == nullptr) { // format changes do not carry any data
        pOut->SetActualDataLength(0);
        return S_OK;
    }

    if (m_pOutput->CurrentMediaType().majortype == MEDIATYPE_MPEG2_PES) {
        if (*(DWORD*)pDataIn == 0xBA010000) {
            len -= 14;
            pDataIn += 14;
            if (int stuffing = (pDataIn[-1] & 7)) {
                len -= stuffing;
                pDataIn += stuffing;
            }
        }
        if (len <= 0) {
            return S_FALSE;
        }
        if (*(DWORD*)pDataIn == 0xBB010000) {
            len -= 4;
            pDataIn += 4;
            int hdrlen = ((pDataIn[0] << 8) | pDataIn[1]) + 2;
            len -= hdrlen;
            pDataIn += hdrlen;
        }
        if (len <= 0) {
            return S_FALSE;
        }
    }

    if (!pDataIn || !pDataOut || len > size || len < 0) {
        return S_FALSE;
    }

    memcpy(pDataOut, pDataIn, std::min(len, size));
    pOut->SetActualDataLength(std::min(len, size));

    return S_OK;
}

HRESULT CDeCSSFilter::CheckInputType(const CMediaType* mtIn)
{
    return mtIn->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK
           ? S_OK
           : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CDeCSSFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    return SUCCEEDED(CheckInputType(mtIn))
           && mtOut->majortype == MEDIATYPE_MPEG2_PACK || mtOut->majortype == MEDIATYPE_MPEG2_PES
           ? S_OK
           : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CDeCSSFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    pProperties->cbAlign = 1;
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = 2048;
    pProperties->cbPrefix = 0;

    HRESULT hr;
    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    return (pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
            ? E_FAIL
            : NOERROR);
}

HRESULT CDeCSSFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 1) {
        return VFW_S_NO_MORE_ITEMS;
    }

    CopyMediaType(pmt, &m_pInput->CurrentMediaType());
    if (iPosition == 0) {
        pmt->majortype = MEDIATYPE_MPEG2_PACK;
    }
    if (iPosition == 1) {
        pmt->majortype = MEDIATYPE_MPEG2_PES;
    }

    return S_OK;
}
