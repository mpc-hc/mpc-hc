/*
 *  $Id$
 *
 *  (C) 2003-2006 Gabest
 *  (C) 2006-2010 see AUTHORS
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

#include "stdafx.h"
#include "SubPicAllocatorPresenterImpl.h"
#include "../DSUtil/DSUtil.h"

CSubPicAllocatorPresenterImpl::CSubPicAllocatorPresenterImpl(HWND hWnd, HRESULT& hr, CString *_pError)
	: CUnknown(NAME("CSubPicAllocatorPresenterImpl"), NULL)
	, m_hWnd(hWnd)
	, m_NativeVideoSize(0, 0), m_AspectRatio(0, 0)
	, m_VideoRect(0, 0, 0, 0), m_WindowRect(0, 0, 0, 0)
	, m_fps(25.0)
	, m_rtSubtitleDelay(0)
	, m_bDeviceResetRequested(false)
	, m_bPendingResetDevice(false)
{
	if (!IsWindow(m_hWnd)) {
		hr = E_INVALIDARG;
		if (_pError) {
			*_pError += "Invalid window handle in ISubPicAllocatorPresenterImpl\n";
		}
		return;
	}
	GetWindowRect(m_hWnd, &m_WindowRect);
	SetVideoAngle(Vector(), false);
	hr = S_OK;
}

CSubPicAllocatorPresenterImpl::~CSubPicAllocatorPresenterImpl()
{
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{

	return
		QI(ISubPicAllocatorPresenter)
		QI(ISubPicAllocatorPresenter2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

void CSubPicAllocatorPresenterImpl::AlphaBltSubPic(CSize size, SubPicDesc* pTarget)
{
	CComPtr<ISubPic> pSubPic;
	if (m_pSubPicQueue->LookupSubPic(m_rtNow, pSubPic)) {
		CRect rcSource, rcDest;
		if (SUCCEEDED (pSubPic->GetSourceAndDest(&size, rcSource, rcDest))) {
			pSubPic->AlphaBlt(rcSource, rcDest, pTarget);
		}
		/*		SubPicDesc spd;
				pSubPic->GetDesc(spd);

				if(spd.w > 0 && spd.h > 0)
				{
					CRect r;
					pSubPic->GetDirtyRect(r);

					// FIXME
					r.DeflateRect(1, 1);

					CRect rDstText(
						r.left * size.cx / spd.w,
						r.top * size.cy / spd.h,
						r.right * size.cx / spd.w,
						r.bottom * size.cy / spd.h);

					pSubPic->AlphaBlt(r, rDstText, pTarget);
				}
		*/
	}
}

// ISubPicAllocatorPresenter

STDMETHODIMP_(SIZE) CSubPicAllocatorPresenterImpl::GetVideoSize(bool fCorrectAR)
{
	CSize VideoSize(GetVisibleVideoSize());

	if (fCorrectAR && m_AspectRatio.cx > 0 && m_AspectRatio.cy > 0) {
		VideoSize.cx = (LONGLONG(VideoSize.cy)*LONGLONG(m_AspectRatio.cx))/LONGLONG(m_AspectRatio.cy);
	}

	return(VideoSize);
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetPosition(RECT w, RECT v)
{
	bool fWindowPosChanged = !!(m_WindowRect != w);
	bool fWindowSizeChanged = !!(m_WindowRect.Size() != CRect(w).Size());

	m_WindowRect = w;

	bool fVideoRectChanged = !!(m_VideoRect != v);

	m_VideoRect = v;

	if (fWindowSizeChanged || fVideoRectChanged) {
		if (m_pAllocator) {
			m_pAllocator->SetCurSize(m_WindowRect.Size());
			m_pAllocator->SetCurVidRect(m_VideoRect);
		}

		if (m_pSubPicQueue) {
			m_pSubPicQueue->Invalidate();
		}
	}

	if (fWindowPosChanged || fVideoRectChanged) {
		Paint(fWindowSizeChanged || fVideoRectChanged);
	}
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetTime(REFERENCE_TIME rtNow)
{
	/*
		if(m_rtNow <= rtNow && rtNow <= m_rtNow + 1000000)
			return;
	*/
	m_rtNow = rtNow - m_rtSubtitleDelay;

	if (m_pSubPicQueue) {
		m_pSubPicQueue->SetTime(m_rtNow);
	}
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetSubtitleDelay(int delay_ms)
{
	m_rtSubtitleDelay = delay_ms*10000i64;
}

STDMETHODIMP_(int) CSubPicAllocatorPresenterImpl::GetSubtitleDelay()
{
	return (m_rtSubtitleDelay/10000);
}

STDMETHODIMP_(double) CSubPicAllocatorPresenterImpl::GetFPS()
{
	return(m_fps);
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetSubPicProvider(ISubPicProvider* pSubPicProvider)
{
	m_SubPicProvider = pSubPicProvider;

	if (m_pSubPicQueue) {
		m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);
	}
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::Invalidate(REFERENCE_TIME rtInvalidate)
{
	if (m_pSubPicQueue) {
		m_pSubPicQueue->Invalidate(rtInvalidate);
	}
}

#include <math.h>

void CSubPicAllocatorPresenterImpl::Transform(CRect r, Vector v[4])
{
	v[0] = Vector(r.left, r.top, 0);
	v[1] = Vector(r.right, r.top, 0);
	v[2] = Vector(r.left, r.bottom, 0);
	v[3] = Vector(r.right, r.bottom, 0);

	Vector center(r.CenterPoint().x, r.CenterPoint().y, 0);
	int l = (int)(Vector(r.Size().cx, r.Size().cy, 0).Length()*1.5f)+1;

	for (ptrdiff_t i = 0; i < 4; i++) {
		v[i] = m_xform << (v[i] - center);
		v[i].z = v[i].z / l + 0.5f;
		v[i].x /= v[i].z*2;
		v[i].y /= v[i].z*2;
		v[i] += center;
	}
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetVideoAngle(Vector v, bool fRepaint)
{
	m_xform = XForm(Ray(Vector(0, 0, 0), v), Vector(1, 1, 1), false);
	if (fRepaint) {
		Paint(true);
	}
	return S_OK;
}
