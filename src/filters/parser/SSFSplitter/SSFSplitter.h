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
#include "../../../Subtitles/libssf/SubtitleFile.h"

class __declspec(uuid("95C3F9F6-1E05-4C34-8122-504476EACB51"))
	CSSFSplitterFilter : public CBaseSplitterFilter
{
	CAutoPtr<CBaseSplitterFile> m_pFile;

	ssf::SubtitleFile m_ssf;

	struct SegmentItemEx : public ssf::SubtitleFile::SegmentItem {
		static int Compare(const void* a, const void* b);
	};

	CAtlList<SegmentItemEx> m_subs;

protected:
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CSSFSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
};

class __declspec(uuid("57F46A2A-6DC9-4A9F-B5FA-DFDD62B8BAFB"))
	CSSFSourceFilter : public CSSFSplitterFilter
{
public:
	CSSFSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};
