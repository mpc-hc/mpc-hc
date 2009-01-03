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
#include "..\BaseSource\BaseSource.h"

class CFlacStream;

[uuid("1930D8FF-4739-4e42-9199-3B2EDEAA3BF2")]
class CFlacSource : public CBaseSource<CFlacStream>
{
public:
	CFlacSource(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CFlacSource();
};

class CFlacStream : public CBaseStream
{
	CFile m_file;
	int m_nFileOffset, m_nBytesPerFrame, m_nAvgBytesPerSec, m_nSamplesPerSec, m_nChannels, m_nMaxFrameSize;
	GUID m_subtype;
	WORD m_wFormatTag;
	BYTE m_streamid;
	WORD m_wBitsPerSample;

	unsigned long charArrToULong(unsigned char* inCharArray);

public:
    CFlacStream(const WCHAR* wfn, CSource* pParent, HRESULT* phr);
	virtual ~CFlacStream();

    HRESULT FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len);
    
	HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
    HRESULT CheckMediaType(const CMediaType* pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
};
