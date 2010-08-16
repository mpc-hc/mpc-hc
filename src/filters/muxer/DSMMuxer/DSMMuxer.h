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

#include "../BaseMuxer/BaseMuxer.h"
#include <dsm/dsm.h>

class __declspec(uuid("C6590B76-587E-4082-9125-680D0693A97B"))
	CDSMMuxerFilter : public CBaseMuxerFilter
{
	bool m_fAutoChap, m_fAutoRes;

	struct SyncPoint {
		BYTE id;
		REFERENCE_TIME rtStart, rtStop;
		__int64 fp;
	};
	struct IndexedSyncPoint {
		BYTE id;
		REFERENCE_TIME rt, rtfp;
		__int64 fp;
	};
	CAtlList<SyncPoint> m_sps;
	CAtlList<IndexedSyncPoint> m_isps;
	REFERENCE_TIME m_rtPrevSyncPoint;
	void IndexSyncPoint(const MuxerPacket* p, __int64 fp);

	void MuxPacketHeader(IBitStream* pBS, dsmp_t type, UINT64 len);
	void MuxFileInfo(IBitStream* pBS);
	void MuxStreamInfo(IBitStream* pBS, CBaseMuxerInputPin* pPin);

protected:
	void MuxInit();

	void MuxHeader(IBitStream* pBS);
	void MuxPacket(IBitStream* pBS, const MuxerPacket* pPacket);
	void MuxFooter(IBitStream* pBS);

public:
	CDSMMuxerFilter(LPUNKNOWN pUnk, HRESULT* phr, bool fAutoChap = true, bool fAutoRes = true);
	virtual ~CDSMMuxerFilter();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
};
