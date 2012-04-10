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

#define DTSAC3SourceName   L"MPC DTS/AC3/DD+ Source"

class CDTSAC3Stream;

class __declspec(uuid("B4A7BE85-551D-4594-BDC7-832B09185041"))
	CDTSAC3Source : public CBaseSource<CDTSAC3Stream>
{
public:
	CDTSAC3Source(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CDTSAC3Source();

	// CBaseFilter
	STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);
};

class CDTSAC3Stream : public CBaseStream
{
	CFile m_file;
	LONGLONG m_dataOffset;

	GUID m_subtype;
	WORD m_wFormatTag;
	int  m_channels;       // number of channels
	int  m_samplerate;     // samples per second
	int  m_bitrate;        // bits per second
	int  m_framesize;      // bytes per frame
	bool m_fixedframesize; // constant frame size
	int  m_framelength;    // sample per frame
	WORD m_bitdepth;       // bits per sample
	BYTE m_streamtype;

	bool CheckDTS(const CMediaType* pmt);
	bool CheckDTS2(const CMediaType* pmt);
	bool CheckAC3(const CMediaType* pmt);
	bool CheckSPDIFAC3(const CMediaType* pmt);
	//	bool CheckTrueHD(const CMediaType* pmt);

public:
	CDTSAC3Stream(const WCHAR* wfn, CSource* pParent, HRESULT* phr);
	virtual ~CDTSAC3Stream();

	HRESULT FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len);

	HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT CheckMediaType(const CMediaType* pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);
};
