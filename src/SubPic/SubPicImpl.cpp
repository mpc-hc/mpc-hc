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

#include "stdafx.h"
#include "SubPicImpl.h"
#include "../DSUtil/DSUtil.h"

//
// CSubPicImpl
//

CSubPicImpl::CSubPicImpl()
	: CUnknown(NAME("CSubPicImpl"), NULL)
	, m_rtStart(0), m_rtStop(0)
	, m_rtSegmentStart(0), m_rtSegmentStop(0)
	, m_rcDirty(0, 0, 0, 0), m_maxsize(0, 0), m_size(0, 0), m_vidrect(0, 0, 0, 0)
	, m_VirtualTextureSize(0, 0), m_VirtualTextureTopLeft (0, 0)
{
}

STDMETHODIMP CSubPicImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(ISubPic)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPic

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetStart()
{
	return(m_rtStart);
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetStop()
{
	return(m_rtStop);
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetSegmentStart()
{
	if (m_rtSegmentStart) {
		return(m_rtSegmentStart);
	}
	return(m_rtStart);
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetSegmentStop()
{
	if (m_rtSegmentStop) {
		return(m_rtSegmentStop);
	}
	return(m_rtStop);
}

STDMETHODIMP_(void) CSubPicImpl::SetSegmentStart(REFERENCE_TIME rtStart)
{
	m_rtSegmentStart = rtStart;
}

STDMETHODIMP_(void) CSubPicImpl::SetSegmentStop(REFERENCE_TIME rtStop)
{
	m_rtSegmentStop = rtStop;
}



STDMETHODIMP_(void) CSubPicImpl::SetStart(REFERENCE_TIME rtStart)
{
	m_rtStart = rtStart;
}

STDMETHODIMP_(void) CSubPicImpl::SetStop(REFERENCE_TIME rtStop)
{
	m_rtStop = rtStop;
}

STDMETHODIMP CSubPicImpl::CopyTo(ISubPic* pSubPic)
{
	if (!pSubPic) {
		return E_POINTER;
	}

	pSubPic->SetStart(m_rtStart);
	pSubPic->SetStop(m_rtStop);
	pSubPic->SetSegmentStart(m_rtSegmentStart);
	pSubPic->SetSegmentStop(m_rtSegmentStop);
	pSubPic->SetDirtyRect(m_rcDirty);
	pSubPic->SetSize(m_size, m_vidrect);
	pSubPic->SetVirtualTextureSize(m_VirtualTextureSize, m_VirtualTextureTopLeft);

	return S_OK;
}

STDMETHODIMP CSubPicImpl::GetDirtyRect(RECT* pDirtyRect)
{
	return pDirtyRect ? *pDirtyRect = m_rcDirty, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::GetSourceAndDest(SIZE* pSize, RECT* pRcSource, RECT* pRcDest)
{
	CheckPointer (pRcSource, E_POINTER);
	CheckPointer (pRcDest,	 E_POINTER);

	if (m_size.cx > 0 && m_size.cy > 0) {
		CRect		rcTemp = m_rcDirty;

		// FIXME
		rcTemp.DeflateRect(1, 1);

		*pRcSource = rcTemp;

		rcTemp.OffsetRect (m_VirtualTextureTopLeft);
		*pRcDest = CRect (rcTemp.left   * pSize->cx / m_VirtualTextureSize.cx,
						  rcTemp.top    * pSize->cy / m_VirtualTextureSize.cy,
						  rcTemp.right  * pSize->cx / m_VirtualTextureSize.cx,
						  rcTemp.bottom * pSize->cy / m_VirtualTextureSize.cy);

		return S_OK;
	} else {
		return E_INVALIDARG;
	}
}

STDMETHODIMP CSubPicImpl::SetDirtyRect(RECT* pDirtyRect)
{
	return pDirtyRect ? m_rcDirty = *pDirtyRect, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::GetMaxSize(SIZE* pMaxSize)
{
	return pMaxSize ? *pMaxSize = m_maxsize, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::SetSize(SIZE size, RECT vidrect)
{
	m_size = size;
	m_vidrect = vidrect;

	if (m_size.cx > m_maxsize.cx) {
		m_size.cy = MulDiv(m_size.cy, m_maxsize.cx, m_size.cx);
		m_size.cx = m_maxsize.cx;
	}

	if (m_size.cy > m_maxsize.cy) {
		m_size.cx = MulDiv(m_size.cx, m_maxsize.cy, m_size.cy);
		m_size.cy = m_maxsize.cy;
	}

	if (m_size.cx != size.cx || m_size.cy != size.cy) {
		m_vidrect.top = MulDiv(m_vidrect.top, m_size.cx, size.cx);
		m_vidrect.bottom = MulDiv(m_vidrect.bottom, m_size.cx, size.cx);
		m_vidrect.left = MulDiv(m_vidrect.left, m_size.cy, size.cy);
		m_vidrect.right = MulDiv(m_vidrect.right, m_size.cy, size.cy);
	}
	m_VirtualTextureSize = m_size;

	return S_OK;
}

STDMETHODIMP CSubPicImpl::SetVirtualTextureSize (const SIZE pSize, const POINT pTopLeft)
{
	m_VirtualTextureSize.SetSize (pSize.cx, pSize.cy);
	m_VirtualTextureTopLeft.SetPoint (pTopLeft.x, pTopLeft.y);

	return S_OK;
}

//
// ISubPicAllocatorImpl
//

CSubPicAllocatorImpl::CSubPicAllocatorImpl(SIZE cursize, bool fDynamicWriteOnly, bool fPow2Textures)
	: CUnknown(NAME("ISubPicAllocatorImpl"), NULL)
	, m_cursize(cursize)
	, m_fDynamicWriteOnly(fDynamicWriteOnly)
	, m_fPow2Textures(fPow2Textures)
{
	m_curvidrect = CRect(CPoint(0,0), m_cursize);
}

STDMETHODIMP CSubPicAllocatorImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(ISubPicAllocator)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPicAllocator

STDMETHODIMP CSubPicAllocatorImpl::SetCurSize(SIZE cursize)
{
	m_cursize = cursize;
	return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::SetCurVidRect(RECT curvidrect)
{
	m_curvidrect = curvidrect;
	return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::GetStatic(ISubPic** ppSubPic)
{
	if (!ppSubPic) {
		return E_POINTER;
	}

	if (!m_pStatic) {
		if (!Alloc(true, &m_pStatic) || !m_pStatic) {
			return E_OUTOFMEMORY;
		}
	}

	m_pStatic->SetSize(m_cursize, m_curvidrect);

	(*ppSubPic = m_pStatic)->AddRef();

	return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::AllocDynamic(ISubPic** ppSubPic)
{
	if (!ppSubPic) {
		return E_POINTER;
	}

	if (!Alloc(false, ppSubPic) || !*ppSubPic) {
		return E_OUTOFMEMORY;
	}

	(*ppSubPic)->SetSize(m_cursize, m_curvidrect);

	return S_OK;
}

STDMETHODIMP_(bool) CSubPicAllocatorImpl::IsDynamicWriteOnly()
{
	return(m_fDynamicWriteOnly);
}

STDMETHODIMP CSubPicAllocatorImpl::ChangeDevice(IUnknown* pDev)
{
	m_pStatic = NULL;
	return S_OK;
}
