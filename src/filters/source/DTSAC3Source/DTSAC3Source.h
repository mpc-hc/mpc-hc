/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

class __declspec(uuid("B4A7BE85-551D-4594-BDC7-832B09185041"))
	CDTSAC3Source : public CBaseSource<CDTSAC3Stream>
{
public:
	CDTSAC3Source(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CDTSAC3Source();
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
