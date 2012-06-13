/*
 * $Id$
 *
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
#include "../BaseSource/BaseSource.h"

#define D2VSourceName   L"MPC D2V Source"

class CD2VStream;

class __declspec(uuid("47CE0591-C4D5-4b41-BED7-28F59AD76228"))
    CD2VSource : public CBaseSource<CD2VStream>
{
public:
    CD2VSource(LPUNKNOWN lpunk, HRESULT* phr);
    virtual ~CD2VSource();

    // CBaseFilter
    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);
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
