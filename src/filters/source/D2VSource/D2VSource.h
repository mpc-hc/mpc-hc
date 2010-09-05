/* 
 *  Copyright (C) 2003-2006 Gabest
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
 *
 */

#pragma once
#include <atlbase.h>
#include "../BaseSource/BaseSource.h"

class CD2VStream;

class __declspec(uuid("47CE0591-C4D5-4b41-BED7-28F59AD76228"))
CD2VSource : public CBaseSource<CD2VStream>
{
public:
	CD2VSource(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CD2VSource();
};

class CMPEG2Dec;

class CD2VStream : public CBaseStream
{
private:
	CAutoPtr<CMPEG2Dec> m_pDecoder;
	CAutoVectorPtr<BYTE> m_pFrameBuffer;

	bool GetDim(int& w, int& h, int& bpp);

public:
    CD2VStream(const WCHAR* fn, CSource* pParent, HRESULT* phr);
	virtual ~CD2VStream();

    HRESULT FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len /*in+out*/);

    HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT CheckMediaType(const CMediaType* pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    HRESULT SetMediaType(const CMediaType* pmt);

	STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
};
