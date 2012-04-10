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

#include "../BaseSplitter/BaseSplitter.h"

class CMpaSplitterFile : public CBaseSplitterFileEx
{
	CMediaType m_mt;
	REFERENCE_TIME m_rtDuration;

	enum {none, mpa, mp4a} m_mode;

	mpahdr m_mpahdr;
	aachdr m_aachdr;
	__int64 m_startpos, m_endpos;

	__int64 m_totalbps;
	CRBMap<__int64, int> m_pos2bps;

	HRESULT Init();
	void AdjustDuration(int nBytesPerSec);

	bool m_bIsVBR;

public:
	CMpaSplitterFile(IAsyncReader* pAsyncReader, HRESULT& hr);

	CAtlMap<DWORD, CStringW> m_tags;

	const CMediaType& GetMediaType() {
		return m_mt;
	}
	REFERENCE_TIME GetDuration() {
		return IsRandomAccess() ? m_rtDuration : 0;
	}

	__int64 GetStartPos() {
		return m_startpos;
	}
	__int64 GetEndPos() {
		return m_endpos;
	}

	bool Sync(int limit = 0x2000);
	bool Sync(int& FrameSize, REFERENCE_TIME& rtDuration, int limit = 0x2000);
};
