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
#include <atlcoll.h>
#include "DSMSplitterFile.h"
#include "../BaseSplitter/BaseSplitter.h"

class __declspec(uuid("0912B4DD-A30A-4568-B590-7179EBB420EC"))
CDSMSplitterFilter : public CBaseSplitterFilter
{
protected:
	CAutoPtr<CDSMSplitterFile> m_pFile;
	HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

	bool DemuxInit();
	void DemuxSeek(REFERENCE_TIME rt);
	bool DemuxLoop();

public:
	CDSMSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);
	virtual ~CDSMSplitterFilter();

	// IKeyFrameInfo

	STDMETHODIMP_(HRESULT) GetKeyFrameCount(UINT& nKFs);
	STDMETHODIMP_(HRESULT) GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs);
};

class __declspec(uuid("803E8280-F3CE-4201-982C-8CD8FB512004"))
CDSMSourceFilter : public CDSMSplitterFilter
{
public:
	CDSMSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};
