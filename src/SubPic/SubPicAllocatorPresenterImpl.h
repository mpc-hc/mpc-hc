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

#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include "ISubPic.h"
#include "CoordGeom.h"

class CSubPicAllocatorPresenterImpl
	: public CUnknown
	, public CCritSec
	, public ISubPicAllocatorPresenter2
{
protected:
	HWND m_hWnd;
	CSize m_spMaxSize; // TODO:
	int m_spMaxQueued; // TODO:
	REFERENCE_TIME m_rtSubtitleDelay;

	CSize m_NativeVideoSize, m_AspectRatio;
	CRect m_VideoRect, m_WindowRect;

	REFERENCE_TIME m_rtNow;
	double m_fps;

	CComPtr<ISubPicProvider> m_SubPicProvider;
	CComPtr<ISubPicAllocator> m_pAllocator;
	CComPtr<ISubPicQueue> m_pSubPicQueue;

	bool m_bDeviceResetRequested;
	bool m_bPendingResetDevice;

	void AlphaBltSubPic(CSize size, SubPicDesc* pTarget = NULL);

	XForm m_xform;
	void Transform(CRect r, Vector v[4]);

public:
	CSubPicAllocatorPresenterImpl(HWND hWnd, HRESULT& hr, CString *_pError);
	virtual ~CSubPicAllocatorPresenterImpl();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// ISubPicAllocatorPresenter

	STDMETHODIMP CreateRenderer(IUnknown** ppRenderer) = 0;

	STDMETHODIMP_(SIZE) GetVideoSize(bool fCorrectAR = true);
	STDMETHODIMP_(SIZE) GetVisibleVideoSize() {
		return m_NativeVideoSize;
	};
	STDMETHODIMP_(void) SetPosition(RECT w, RECT v);
	STDMETHODIMP_(bool) Paint(bool fAll) = 0;

	STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);
	STDMETHODIMP_(void) SetSubtitleDelay(int delay_ms);
	STDMETHODIMP_(int) GetSubtitleDelay();
	STDMETHODIMP_(double) GetFPS();

	STDMETHODIMP_(void) SetSubPicProvider(ISubPicProvider* pSubPicProvider);
	STDMETHODIMP_(void) Invalidate(REFERENCE_TIME rtInvalidate = -1);

	STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size) {
		return E_NOTIMPL;
	}

	STDMETHODIMP_(bool) ResetDevice() {
		return E_NOTIMPL;
	}

	STDMETHODIMP_(bool) DisplayChange() {
		return E_NOTIMPL;
	}

	STDMETHODIMP SetVideoAngle(Vector v, bool fRepaint = true);
	STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget) {
		return E_NOTIMPL;
	}
	STDMETHODIMP SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace) {
		if (!bScreenSpace) {
			return SetPixelShader(pSrcData, pTarget);
		}
		return E_NOTIMPL;
	}
};

