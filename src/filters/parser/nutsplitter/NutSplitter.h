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

#include "..\BaseSplitter\BaseSplitter.h"
#include "NutFile.h"

// {5EB7173E-AA71-4a54-BDD1-1CA57D20405F}
DEFINE_GUID(MEDIASUBTYPE_Nut, 
0x5eb7173e, 0xaa71, 0x4a54, 0xbd, 0xd1, 0x1c, 0xa5, 0x7d, 0x20, 0x40, 0x5f);

[uuid("90514D6A-76B7-4405-88A8-B4B1EF6061C6")]
class CNutSplitterFilter : public CBaseSplitterFilter
{
	CAutoPtr<CNutFile> m_pFile;

protected:
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CNutSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

	// IMediaSeeking
	STDMETHODIMP GetDuration(LONGLONG* pDuration);
};

[uuid("918B5A9F-DFED-4532-83A9-9B16D83ED73F")]
class CNutSourceFilter : public CNutSplitterFilter
{
public:
	CNutSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};
