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
#include "../BaseSource/BaseSource.h"

class CDTSAC3Stream;

[uuid("B4A7BE85-551D-4594-BDC7-832B09185041")]
class CDTSAC3Source : public CBaseSource<CDTSAC3Stream>
{
public:
    CDTSAC3Source(LPUNKNOWN lpunk, HRESULT* phr);
    virtual ~CDTSAC3Source();
};

class CDTSAC3Stream : public CBaseStream
{
    CFile m_file;
    int m_nFileOffset, m_nBytesPerFrame, m_nAvgBytesPerSec, m_nSamplesPerSec;
    GUID m_subtype;
    WORD m_wFormatTag;
    BYTE m_streamid;

    bool CheckDTS(const CMediaType* pmt);
    bool CheckWAVEDTS(const CMediaType* pmt);
    bool CheckAC3(const CMediaType* pmt);
    bool CheckWAVEAC3(const CMediaType* pmt);

public:
    CDTSAC3Stream(const WCHAR* wfn, CSource* pParent, HRESULT* phr);
    virtual ~CDTSAC3Stream();

    HRESULT FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len);

    HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT CheckMediaType(const CMediaType* pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
};
