/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <atlbase.h>

class CWavDestOutputPin : public CTransformOutputPin
{
public:
    CWavDestOutputPin(CTransformFilter* pFilter, HRESULT* phr);

    STDMETHODIMP EnumMediaTypes(IEnumMediaTypes** ppEnum);
    HRESULT CheckMediaType(const CMediaType* pmt);
};

[uuid("8685214E-4D32-4058-BE04-D01104F00B0C")]
class CWavDestFilter : public CTransformFilter
{
public:
    CWavDestFilter(LPUNKNOWN pUnk, HRESULT* pHr);
    ~CWavDestFilter();

	DECLARE_IUNKNOWN;

    HRESULT Transform(IMediaSample* pIn, IMediaSample* pOut);
    HRESULT Receive(IMediaSample* pSample);

    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType*mtIn, const CMediaType* mtOut);
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
