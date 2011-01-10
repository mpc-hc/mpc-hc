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
#include <atlcoll.h>
#include "OggFile.h"
#include "../BaseSplitter/BaseSplitter.h"

class OggPacket : public Packet
{
public:
	OggPacket() {
		fSkip = false;
	}
	bool fSkip;
};

class COggSplitterOutputPin : public CBaseSplitterOutputPin
{
	class CComment
	{
	public:
		CStringW m_key, m_value;
		CComment(CStringW key, CStringW value) : m_key(key), m_value(value) {
			m_key.MakeUpper();
		}
	};

	CAutoPtrList<CComment> m_pComments;

protected:
	CCritSec m_csPackets;
	CAutoPtrList<OggPacket> m_packets;
	CAutoPtr<OggPacket> m_lastpacket;
	int m_lastseqnum;
	REFERENCE_TIME m_rtLast;
	bool m_fSkip;

	void ResetState(DWORD seqnum = -1);

public:
	COggSplitterOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);

	void AddComment(BYTE* p, int len);
	CStringW GetComment(CStringW key);

	HRESULT UnpackPage(OggPage& page);
	virtual HRESULT UnpackPacket(CAutoPtr<OggPacket>& p, BYTE* pData, int len) = 0;
	virtual REFERENCE_TIME GetRefTime(__int64 granule_position) = 0;
	CAutoPtr<OggPacket> GetPacket();

	HRESULT DeliverEndFlush();
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
};

class COggVorbisOutputPin : public COggSplitterOutputPin
{
	CAutoPtrList<OggPacket> m_initpackets;

	DWORD m_audio_sample_rate;
	DWORD m_blocksize[2], m_lastblocksize;
	CAtlArray<bool> m_blockflags;

	virtual HRESULT UnpackPacket(CAutoPtr<OggPacket>& p, BYTE* pData, int len);
	virtual REFERENCE_TIME GetRefTime(__int64 granule_position);

	HRESULT DeliverPacket(CAutoPtr<OggPacket> p);
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

public:
	COggVorbisOutputPin(OggVorbisIdHeader* h, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);

	HRESULT UnpackInitPage(OggPage& page);
	bool IsInitialized() {
		return m_initpackets.GetCount() >= 3;
	}
};

class COggFlacOutputPin : public COggSplitterOutputPin
{
	CAutoPtrList<OggPacket> m_initpackets;

	int			m_nSamplesPerSec;
	int			m_nChannels;
	WORD		m_wBitsPerSample;
	int			m_nAvgBytesPerSec;

	DWORD m_blocksize[2], m_lastblocksize;
	CAtlArray<bool> m_blockflags;

	virtual HRESULT UnpackPacket(CAutoPtr<OggPacket>& p, BYTE* pData, int len);
	virtual REFERENCE_TIME GetRefTime(__int64 granule_position);

	HRESULT DeliverPacket(CAutoPtr<OggPacket> p);
	HRESULT DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

public:
	COggFlacOutputPin(BYTE* h, int nCount, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);

	bool IsInitialized() {
		return m_initpackets.GetCount() >= 3;
	}
};

class COggDirectShowOutputPin : public COggSplitterOutputPin
{
	virtual HRESULT UnpackPacket(CAutoPtr<OggPacket>& p, BYTE* pData, int len);
	virtual REFERENCE_TIME GetRefTime(__int64 granule_position);

public:
	COggDirectShowOutputPin(AM_MEDIA_TYPE* pmt, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
};

class COggStreamOutputPin : public COggSplitterOutputPin
{
	__int64 m_time_unit, m_samples_per_unit;
	DWORD m_default_len;

	virtual HRESULT UnpackPacket(CAutoPtr<OggPacket>& p, BYTE* pData, int len);
	virtual REFERENCE_TIME GetRefTime(__int64 granule_position);

public:
	COggStreamOutputPin(OggStreamHeader* h, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
};

class COggVideoOutputPin : public COggStreamOutputPin
{
public:
	COggVideoOutputPin(OggStreamHeader* h, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
};

class COggAudioOutputPin : public COggStreamOutputPin
{
public:
	COggAudioOutputPin(OggStreamHeader* h, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
};

class COggTextOutputPin : public COggStreamOutputPin
{
public:
	COggTextOutputPin(OggStreamHeader* h, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
};

class COggTheoraOutputPin : public COggSplitterOutputPin
{
	CAutoPtrList<OggPacket> m_initpackets;
	LONGLONG				m_KfgShift;
	int						m_nIndexOffset;
	int						m_nFpsNum;
	int						m_nFpsDenum;
	REFERENCE_TIME			m_rtAvgTimePerFrame;

	virtual HRESULT UnpackPacket(CAutoPtr<OggPacket>& p, BYTE* pData, int len);
	virtual REFERENCE_TIME GetRefTime(__int64 granule_position);

public:
	COggTheoraOutputPin(BYTE* p, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);

	HRESULT UnpackInitPage(OggPage& page);
	bool IsInitialized() {
		return m_initpackets.GetCount() >= 3;
	}
};

class __declspec(uuid("9FF48807-E133-40AA-826F-9B2959E5232D"))
	COggSplitterFilter : public CBaseSplitterFilter
{
protected:
	CAutoPtr<COggFile> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	COggSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
	virtual ~COggSplitterFilter();
};

class __declspec(uuid("6D3688CE-3E9D-42F4-92CA-8A11119D25CD"))
	COggSourceFilter : public COggSplitterFilter
{
public:
	COggSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};
