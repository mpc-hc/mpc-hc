/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#pragma once

#include <atlbase.h>

#define WavDestName L"MPC WavDest"

class CWavDestOutputPin : public CTransformOutputPin
{
public:
    CWavDestOutputPin(CTransformFilter* pFilter, HRESULT* phr);

    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes** ppEnum);
    HRESULT CheckMediaType(const CMediaType* pmt);
};

class __declspec(uuid("8685214E-4D32-4058-BE04-D01104F00B0C"))
    CWavDestFilter : public CTransformFilter
{
public:
    CWavDestFilter(LPUNKNOWN pUnk, HRESULT* pHr);
    ~CWavDestFilter();

    DECLARE_IUNKNOWN;

    HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
    HRESULT Receive(IMediaSample* pSample);

    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
    HRESULT GetMediaType(int iPosition, CMediaType* pMediaType);

    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);

    HRESULT StartStreaming();
    HRESULT StopStreaming();

private:

    HRESULT Copy(IMediaSample* pSource, IMediaSample* pDest) const;
    HRESULT Transform(IMediaSample* pMediaSample);
    HRESULT Transform(AM_MEDIA_TYPE* pType, const signed char ContrastLevel) const;

    ULONG m_cbWavData;
    ULONG m_cbHeader;
};
