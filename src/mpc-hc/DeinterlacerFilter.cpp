/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2016 see Authors.txt
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
#include "DeinterlacerFilter.h"
#include "MediaTypes.h"
#include "moreuuids.h"


CDeinterlacerFilter::CDeinterlacerFilter(LPUNKNOWN punk, HRESULT* phr)
    : CTransformFilter(NAME("CDeinterlacerFilter"), punk, __uuidof(CDeinterlacerFilter))
{
    if (phr) {
        *phr = S_OK;
    }
}

HRESULT CDeinterlacerFilter::CheckConnect(PIN_DIRECTION dir, IPin* pPin)
{
    return GetCLSID(pPin) == __uuidof(*this) ? E_FAIL : S_OK;
}

HRESULT CDeinterlacerFilter::CheckInputType(const CMediaType* mtIn)
{
    BITMAPINFOHEADER bih;
    if (!ExtractBIH(mtIn, &bih) /*|| bih.biHeight <= 0*/ || bih.biHeight <= 288) {
        return E_FAIL;
    }

    return mtIn->subtype == MEDIASUBTYPE_YUY2 || mtIn->subtype == MEDIASUBTYPE_UYVY
           || mtIn->subtype == MEDIASUBTYPE_I420 || mtIn->subtype == MEDIASUBTYPE_YV12 || mtIn->subtype == MEDIASUBTYPE_IYUV
           ? S_OK
           : E_FAIL;
}

HRESULT CDeinterlacerFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    return mtIn->subtype == mtOut->subtype ? S_OK : E_FAIL;
}

HRESULT CDeinterlacerFilter::Transform(IMediaSample* pIn, IMediaSample* pOut)
{
    HRESULT hr;

    AM_MEDIA_TYPE* pmt = nullptr;
    if (SUCCEEDED(pOut->GetMediaType(&pmt)) && pmt) {
        CMediaType mt = *pmt;
        m_pOutput->SetMediaType(&mt);
        DeleteMediaType(pmt);
    }

    BYTE* pDataIn = nullptr;
    if (FAILED(pIn->GetPointer(&pDataIn)) || !pDataIn) {
        return S_FALSE;
    }

    BYTE* pDataOut = nullptr;
    if (FAILED(hr = pOut->GetPointer(&pDataOut)) || !pDataOut) {
        return hr;
    }

    const CMediaType& mtIn = m_pInput->CurrentMediaType();
    const CMediaType& mtOut = m_pOutput->CurrentMediaType();

    BITMAPINFOHEADER bihIn, bihOut;
    ExtractBIH(&mtIn, &bihIn);
    ExtractBIH(&mtOut, &bihOut);

    bool fInputFlipped = bihIn.biHeight >= 0 && bihIn.biCompression <= 3;
    bool fOutputFlipped = bihOut.biHeight >= 0 && bihOut.biCompression <= 3;
    bool fFlip = fInputFlipped != fOutputFlipped;

    int bppIn = !(bihIn.biBitCount & 7) ? bihIn.biBitCount : 8;
    int bppOut = !(bihOut.biBitCount & 7) ? bihOut.biBitCount : 8;
    int pitchIn = bihIn.biWidth * bppIn >> 3;
    int pitchOut = bihOut.biWidth * bppOut >> 3;

    if (fFlip) {
        pitchOut = -pitchOut;
    }

    if (mtIn.subtype == MEDIASUBTYPE_YUY2 || mtIn.subtype == MEDIASUBTYPE_UYVY) {
        DeinterlaceBlend(pDataOut, pDataIn, pitchIn, bihIn.biHeight, pitchOut, pitchIn);
    } else if (mtIn.subtype == MEDIASUBTYPE_I420 || mtIn.subtype == MEDIASUBTYPE_YV12 || mtIn.subtype == MEDIASUBTYPE_IYUV) {
        DeinterlaceBlend(pDataOut, pDataIn, pitchIn, bihIn.biHeight, pitchOut, pitchIn);

        int sizeIn = bihIn.biHeight * pitchIn, sizeOut = abs(bihOut.biHeight) * pitchOut;
        pitchIn /= 2;
        pitchOut /= 2;
        bihIn.biHeight /= 2;
        pDataIn += sizeIn;
        pDataOut += sizeOut;
        DeinterlaceBlend(pDataOut, pDataIn, pitchIn, bihIn.biHeight, pitchOut, pitchIn);

        pDataIn += sizeIn / 4;
        pDataOut += sizeOut / 4;
        DeinterlaceBlend(pDataOut, pDataIn, pitchIn, bihIn.biHeight, pitchOut, pitchIn);
    }

    return S_OK;
}

HRESULT CDeinterlacerFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    BITMAPINFOHEADER bih;
    ExtractBIH(&m_pOutput->CurrentMediaType(), &bih);

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = bih.biSizeImage;
    pProperties->cbAlign = 1;
    pProperties->cbPrefix = 0;

    HRESULT hr;
    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
           ? E_FAIL
           : NOERROR;
}

HRESULT CDeinterlacerFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }
    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }
    *pmt = m_pInput->CurrentMediaType();
    CorrectMediaType(pmt);
    return S_OK;
}
