/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "RenderersSettings.h"

#include "VMR7AllocatorPresenter.h"
#include "IPinHook.h"

extern bool g_bNoDuration; // Defined in MainFrm.cpp
extern bool g_bExternalSubtitleTime;

using namespace DSObjects;

//
// CVMR7AllocatorPresenter
//

#define MY_USER_ID 0x6ABE51

CVMR7AllocatorPresenter::CVMR7AllocatorPresenter(HWND hWnd, HRESULT& hr)
	: CDX7AllocatorPresenter(hWnd, hr)
	, m_fUseInternalTimer(false)
{
	if(FAILED(hr))
		return;

	if(FAILED(hr = m_pSA.CoCreateInstance(CLSID_AllocPresenter)))
	{
		hr = E_FAIL;
		return;
	}
}

STDMETHODIMP CVMR7AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		QI(IVMRSurfaceAllocator)
		QI(IVMRImagePresenter)
		QI(IVMRWindowlessControl)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVMR7AllocatorPresenter::CreateDevice()
{
	HRESULT hr = __super::CreateDevice();
	if(FAILED(hr)) return hr;

	if(m_pIVMRSurfAllocNotify)
	{
		HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		if(FAILED(hr = m_pIVMRSurfAllocNotify->ChangeDDrawDevice(m_pDD, hMonitor)))
			return(false);
	}

	return hr;
}

void CVMR7AllocatorPresenter::DeleteSurfaces()
{
	CAutoLock cAutoLock(this);

	m_pSA->FreeSurface(MY_USER_ID);

	__super::DeleteSurfaces();
}

// ISubPicAllocatorPresenter

STDMETHODIMP CVMR7AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	CheckPointer(ppRenderer, E_POINTER);

	*ppRenderer = NULL;

	HRESULT hr;

	do
	{
		CComPtr<IBaseFilter> pBF;

		if(FAILED(hr = pBF.CoCreateInstance(CLSID_VideoMixingRenderer)))
			break;

		CComQIPtr<IVMRFilterConfig> pConfig = pBF;
		if(!pConfig)
			break;

		if(FAILED(hr = pConfig->SetRenderingMode(VMRMode_Renderless)))
			break;

		CComQIPtr<IVMRSurfaceAllocatorNotify> pSAN = pBF;
		if(!pSAN)
			break;

		if(FAILED(hr = pSAN->AdviseSurfaceAllocator(MY_USER_ID, static_cast<IVMRSurfaceAllocator*>(this)))
				|| FAILED(hr = AdviseNotify(pSAN)))
			break;

		CComPtr<IPin> pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		m_fUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);

		*ppRenderer = (IUnknown*)pBF.Detach();

		return S_OK;
	}
	while(0);

	return E_FAIL;
}

STDMETHODIMP_(void) CVMR7AllocatorPresenter::SetTime(REFERENCE_TIME rtNow)
{
	__super::SetTime(rtNow);
//	m_fUseInternalTimer = false;
}

// IVMRSurfaceAllocator

STDMETHODIMP CVMR7AllocatorPresenter::AllocateSurface(DWORD_PTR dwUserID, VMRALLOCATIONINFO* lpAllocInfo, DWORD* lpdwBuffer, LPDIRECTDRAWSURFACE7* lplpSurface)
{
	if(!lpAllocInfo || !lpdwBuffer || !lplpSurface)
		return E_POINTER;

	if(!m_pIVMRSurfAllocNotify)
		return E_FAIL;

	HRESULT hr;

	DeleteSurfaces();

	// HACK: yv12 will fail to blt onto the backbuffer anyway, but if we first
	// allocate it and then let our FreeSurface callback call m_pSA->FreeSurface,
	// then that might stall for about 30 seconds because of some unknown buggy code
	// behind <ddraw surface>->Release()

	if(lpAllocInfo->lpHdr->biBitCount < 16)
		return E_FAIL;

	hr = m_pSA->AllocateSurface(dwUserID, lpAllocInfo, lpdwBuffer, lplpSurface);
	if(FAILED(hr))
		return hr;

	m_NativeVideoSize = CSize(abs(lpAllocInfo->lpHdr->biWidth), abs(lpAllocInfo->lpHdr->biHeight));
	m_AspectRatio = m_NativeVideoSize;
	int arx = lpAllocInfo->szAspectRatio.cx, ary = lpAllocInfo->szAspectRatio.cy;
	if(arx > 0 && ary > 0) m_AspectRatio.SetSize(arx, ary);

	if(FAILED(hr = AllocSurfaces()))
		return hr;

	// test if the colorspace is acceptable
	if(FAILED(hr = m_pVideoSurface->Blt(NULL, *lplpSurface, NULL, DDBLT_WAIT, NULL)))
	{
		DeleteSurfaces();
		return hr;
	}

	DDBLTFX fx;
	INITDDSTRUCT(fx);
	fx.dwFillColor = 0;
	m_pVideoSurface->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

	return hr;
}

STDMETHODIMP CVMR7AllocatorPresenter::FreeSurface(DWORD_PTR dwUserID)
{
	DeleteSurfaces();
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::PrepareSurface(DWORD_PTR dwUserID, IDirectDrawSurface7* lpSurface, DWORD dwSurfaceFlags)
{
	SetThreadName((DWORD)-1, "CVMR7AllocatorPresenter");

	if(!lpSurface)
		return E_POINTER;

	// FIXME: sometimes the msmpeg4/divx3/wmv decoder wants to reuse our
	// surface (expects it to point to the same mem every time), and to avoid
	// problems we can't call m_pSA->PrepareSurface (flips? clears?).
	return S_OK;
	/*
		return m_pSA->PrepareSurface(dwUserID, lpSurface, dwSurfaceFlags);
	*/
}

STDMETHODIMP CVMR7AllocatorPresenter::AdviseNotify(IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify)
{
	CAutoLock cAutoLock(this);

	m_pIVMRSurfAllocNotify = lpIVMRSurfAllocNotify;

	HRESULT hr;
	HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if(FAILED(hr = m_pIVMRSurfAllocNotify->SetDDrawDevice(m_pDD, hMonitor)))
		return hr;

	return m_pSA->AdviseNotify(lpIVMRSurfAllocNotify);
}

// IVMRImagePresenter

STDMETHODIMP CVMR7AllocatorPresenter::StartPresenting(DWORD_PTR dwUserID)
{
	if (!m_bPendingResetDevice)
	{
		ASSERT(m_pD3DDev);
	}

	CAutoLock cAutoLock(this);

	return m_pD3DDev ? S_OK : E_FAIL;
}

STDMETHODIMP CVMR7AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMRPRESENTATIONINFO* lpPresInfo)
{
	if(!lpPresInfo || !lpPresInfo->lpSurf)
		return E_POINTER;

	CAutoLock cAutoLock(this);

	if (!m_bPendingResetDevice)
		m_pVideoSurface->Blt(NULL, lpPresInfo->lpSurf, NULL, DDBLT_WAIT, NULL);

	if(lpPresInfo->rtEnd > lpPresInfo->rtStart)
	{
		REFERENCE_TIME rtTimePerFrame = lpPresInfo->rtEnd - lpPresInfo->rtStart;
		m_fps = 10000000.0 / rtTimePerFrame;

		if(m_pSubPicQueue)
		{
			m_pSubPicQueue->SetFPS(m_fps);

			if(m_fUseInternalTimer && !g_bExternalSubtitleTime)
			{
				__super::SetTime(g_tSegmentStart + g_tSampleStart);
			}
		}
	}

	CSize VideoSize = m_NativeVideoSize;
	int arx = lpPresInfo->szAspectRatio.cx, ary = lpPresInfo->szAspectRatio.cy;
	if(arx > 0 && ary > 0) VideoSize.cx = VideoSize.cy*arx/ary;
	if(VideoSize != GetVideoSize())
	{
		m_AspectRatio.SetSize(arx, ary);
		AfxGetApp()->m_pMainWnd->PostMessage(WM_REARRANGERENDERLESS);
	}

	Paint(true);

	return S_OK;
}

// IVMRWindowlessControl
//
// It is only implemented (partially) for the dvd navigator's
// menu handling, which needs to know a few things about the
// location of our window.

STDMETHODIMP CVMR7AllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	CSize vs = m_NativeVideoSize, ar = m_AspectRatio;
	// DVD Nav. bug workaround fix
	vs.cx = vs.cy * ar.cx / ar.cy;
	if(lpWidth) *lpWidth = vs.cx;
	if(lpHeight) *lpHeight = vs.cy;
	if(lpARWidth) *lpARWidth = ar.cx;
	if(lpARHeight) *lpARHeight = ar.cy;
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect)
{
	return E_NOTIMPL;   // we have our own method for this
}

STDMETHODIMP CVMR7AllocatorPresenter::GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
{
	CopyRect(lpSRCRect, CRect(CPoint(0, 0), m_NativeVideoSize));
	CopyRect(lpDSTRect, &m_VideoRect);
	// DVD Nav. bug workaround fix
	GetNativeVideoSize(&lpSRCRect->right, &lpSRCRect->bottom, NULL, NULL);
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::GetAspectRatioMode(DWORD* lpAspectRatioMode)
{
	if(lpAspectRatioMode) *lpAspectRatioMode = AM_ARMODE_STRETCHED;
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::SetAspectRatioMode(DWORD AspectRatioMode)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::SetVideoClippingWindow(HWND hwnd)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::RepaintVideo(HWND hwnd, HDC hdc)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::DisplayModeChanged()
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::GetCurrentImage(BYTE** lpDib)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::SetBorderColor(COLORREF Clr)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR7AllocatorPresenter::GetBorderColor(COLORREF* lpClr)
{
	if(lpClr) *lpClr = 0;
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::SetColorKey(COLORREF Clr)
{
	return E_NOTIMPL;
}
STDMETHODIMP CVMR7AllocatorPresenter::GetColorKey(COLORREF* lpClr)
{
	return E_NOTIMPL;
}
