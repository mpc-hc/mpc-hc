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

#include "BitStream.h"
#include "BaseMuxerInputPin.h"
#include "BaseMuxerRelatedPin.h"
#include "../../../Subtitles/libssf/SubtitleFile.h"

class CBaseMuxerOutputPin : public CBaseOutputPin
{
	CComPtr<IBitStream> m_pBitStream;

public:
	CBaseMuxerOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
	virtual ~CBaseMuxerOutputPin() {}

	IBitStream* GetBitStream();

	HRESULT BreakConnect();

	HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);

	HRESULT CheckMediaType(const CMediaType* pmt);
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);

	HRESULT DeliverEndOfStream();

	STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
};

class CBaseMuxerRawOutputPin : public CBaseMuxerOutputPin, public CBaseMuxerRelatedPin
{
	struct idx_t {
		REFERENCE_TIME rt;
		__int64 fp;
	};
	CAtlList<idx_t> m_idx;

	ssf::SubtitleFile m_ssf;

public:
	CBaseMuxerRawOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
	virtual ~CBaseMuxerRawOutputPin() {}

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	virtual void MuxHeader(const CMediaType& mt);
	virtual void MuxPacket(const CMediaType& mt, const MuxerPacket* pPacket);
	virtual void MuxFooter(const CMediaType& mt);
};
