/*
 * $Id$
 *
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

#include "Rasterizer.h"
#include "../SubPic/SubPicProviderImpl.h"
#include "HdmvSub.h"
#include "BaseSub.h"


class __declspec(uuid("FCA68599-C83E-4ea5-94A3-C2E1B0E326B9"))
	CRenderedHdmvSubtitle : public CSubPicProviderImpl, public ISubStream
{
public:
	CRenderedHdmvSubtitle(CCritSec* pLock, SUBTITLE_TYPE nType, const CString& name, LCID lcid);
	~CRenderedHdmvSubtitle(void);

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// ISubPicProvider
	STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps);
	STDMETHODIMP_(POSITION) GetNext(POSITION pos);
	STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
	STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
	STDMETHODIMP_(bool) IsAnimated(POSITION pos);
	STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
	STDMETHODIMP GetTextureSize (POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft);

	// IPersist
	STDMETHODIMP GetClassID(CLSID* pClassID);

	// ISubStream
	STDMETHODIMP_(int) GetStreamCount();
	STDMETHODIMP GetStreamInfo(int i, WCHAR** ppName, LCID* pLCID);
	STDMETHODIMP_(int) GetStream();
	STDMETHODIMP SetStream(int iStream);
	STDMETHODIMP Reload();

	HRESULT ParseSample (IMediaSample* pSample);
	HRESULT	NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

private :
	CString			m_name;
	LCID			m_lcid;
	REFERENCE_TIME	m_rtStart;

	CBaseSub*		m_pSub;
	CCritSec		m_csCritSec;
};
