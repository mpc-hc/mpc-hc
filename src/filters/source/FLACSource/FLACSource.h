/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

class CFLACStream;

class __declspec(uuid("1930D8FF-4739-4e42-9199-3B2EDEAA3BF2"))
	CFLACSource : public CBaseSource<CFLACStream>
{
public:
	CFLACSource(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CFLACSource();
};


class CGolombBuffer;

class CFLACStream : public CBaseStream
{
	CFile		m_file;
	void*		m_pDecoder;

	int			m_nMaxFrameSize;
	int			m_nSamplesPerSec;
	int			m_nChannels;
	WORD		m_wBitsPerSample;
	__int64		m_i64TotalNumSamples;
	int			m_nAvgBytesPerSec;

	ULONGLONG	m_llOffset;				// Position of first frame in file
	ULONGLONG	m_llFileSize;			// Size of the file

public:
	CFLACStream(const WCHAR* wfn, CSource* pParent, HRESULT* phr);
	virtual ~CFLACStream();

	HRESULT			FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len);

	HRESULT			DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT			CheckMediaType(const CMediaType* pMediaType);
	HRESULT			GetMediaType(int iPosition, CMediaType* pmt);

	void			UpdateFromMetadata (void* pBuffer);
	inline CFile*	GetFile() {
		return &m_file;
	};

	bool			m_bIsEOF;
};

