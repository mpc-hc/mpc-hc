/*
 * $Id$
 *
 * (C) 2006-2011 see AUTHORS
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
#include "VMR9AllocatorPresenter.h"
#include "OuterVMR.h"
#include "IPinHook.h"
#include "MacrovisionKicker.h"

// ISubPicAllocatorPresenter


using namespace DSObjects;

//
// CVMR9AllocatorPresenter
//

#define MY_USER_ID 0x6ABE51

CVMR9AllocatorPresenter::CVMR9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, CString &_Error)
	: CDX9AllocatorPresenter(hWnd, bFullscreen, hr, false, _Error)
	, m_fUseInternalTimer(false)
	, m_rtPrevStart(-1)
{
}

STDMETHODIMP CVMR9AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		QI(IVMRSurfaceAllocator9)
		QI(IVMRImagePresenter9)
		QI(IVMRWindowlessControl9)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CVMR9AllocatorPresenter::CreateDevice(CString &_Error)
{
	HRESULT hr = __super::CreateDevice(_Error);
	if (FAILED(hr)) {
		return hr;
	}

	if (m_pIVMRSurfAllocNotify) {
		HMONITOR hMonitor = m_pD3D->GetAdapterMonitor(m_CurrentAdapter);
		if (FAILED(hr = m_pIVMRSurfAllocNotify->ChangeD3DDevice(m_pD3DDev, hMonitor))) {
			_Error += L"m_pIVMRSurfAllocNotify->ChangeD3DDevice failed";
			return hr; //return false;
		}
	}

	return hr;
}

void CVMR9AllocatorPresenter::DeleteSurfaces()
{
	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	m_pSurfaces.RemoveAll();

	return __super::DeleteSurfaces();
}

STDMETHODIMP CVMR9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	CheckPointer(ppRenderer, E_POINTER);

	*ppRenderer = NULL;

	HRESULT hr;

	do {
		CMacrovisionKicker* pMK = DNew CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
		CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;

		COuterVMR9 *pOuter = DNew COuterVMR9(NAME("COuterVMR9"), pUnk, &m_VMR9AlphaBitmap, this);


		pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)pOuter);
		CComQIPtr<IBaseFilter> pBF = pUnk;

		CComPtr<IPin> pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		m_fUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);

		if (CComQIPtr<IAMVideoAccelerator> pAMVA = pPin) {
			HookAMVideoAccelerator((IAMVideoAcceleratorC*)(IAMVideoAccelerator*)pAMVA);
		}

		CComQIPtr<IVMRFilterConfig9> pConfig = pBF;
		if (!pConfig) {
			break;
		}

		CRenderersSettings& s = GetRenderersSettings();

		if (s.fVMR9MixerMode) {
			if (FAILED(hr = pConfig->SetNumberOfStreams(1))) {
				break;
			}

			if (CComQIPtr<IVMRMixerControl9> pMC = pBF) {
				DWORD dwPrefs;
				pMC->GetMixingPrefs(&dwPrefs);

				// See http://msdn.microsoft.com/en-us/library/dd390928(VS.85).aspx
				dwPrefs |= MixerPref9_NonSquareMixing;
				dwPrefs |= MixerPref9_NoDecimation;
				if (s.fVMR9MixerYUV && !IsVistaOrAbove()) {
					dwPrefs &= ~MixerPref9_RenderTargetMask;
					dwPrefs |= MixerPref9_RenderTargetYUV;
				}
				pMC->SetMixingPrefs(dwPrefs);
			}
		}

		if (FAILED(hr = pConfig->SetRenderingMode(VMR9Mode_Renderless))) {
			break;
		}

		CComQIPtr<IVMRSurfaceAllocatorNotify9> pSAN = pBF;
		if (!pSAN) {
			break;
		}

		if (FAILED(hr = pSAN->AdviseSurfaceAllocator(MY_USER_ID, static_cast<IVMRSurfaceAllocator9*>(this)))
				|| FAILED(hr = AdviseNotify(pSAN))) {
			break;
		}

		*ppRenderer = (IUnknown*)pBF.Detach();

		return S_OK;
	} while (0);

	return E_FAIL;
}

STDMETHODIMP_(void) CVMR9AllocatorPresenter::SetTime(REFERENCE_TIME rtNow)
{
	__super::SetTime(rtNow);
	//m_fUseInternalTimer = false;
}

// IVMRSurfaceAllocator9

STDMETHODIMP CVMR9AllocatorPresenter::InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers)
{
	CAutoLock lock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	if (!lpAllocInfo || !lpNumBuffers) {
		return E_POINTER;
	}

	if (!m_pIVMRSurfAllocNotify) {
		return E_FAIL;
	}

	// WTF: Is this some kind of forgotten debug code ?
	if ((GetAsyncKeyState(VK_CONTROL)&0x80000000))
		if (lpAllocInfo->Format == '21VY' || lpAllocInfo->Format == '024I') {
			return E_FAIL;
		}

	// The surfaces should already be free when InitializeDevice is called
	DeleteSurfaces();

	DWORD nOriginal = *lpNumBuffers;

	if (*lpNumBuffers == 1) {
		*lpNumBuffers = 4;
		m_nVMR9Surfaces = 4;
	} else {
		m_nVMR9Surfaces = 0;
	}
	m_pSurfaces.SetCount(*lpNumBuffers);

	int w = lpAllocInfo->dwWidth;
	int h = abs((int)lpAllocInfo->dwHeight);

	HRESULT hr;

	if (lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget) {
		lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;
	}

	hr = m_pIVMRSurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &m_pSurfaces[0]);
	if (FAILED(hr)) {
		return hr;
	}

	m_pSurfaces.SetCount(*lpNumBuffers);

	m_bNeedCheckSample = true;
	m_NativeVideoSize = CSize(w, h);
	CSize VideoSize = GetVisibleVideoSize();
	int arx = lpAllocInfo->szAspectRatio.cx;
	int ary = lpAllocInfo->szAspectRatio.cy;
	if (arx > 0 && ary > 0) {
		arx = arx / ((float) m_NativeVideoSize.cx / VideoSize.cx);
		ary = ary / ((float) m_NativeVideoSize.cy / VideoSize.cy);
		m_AspectRatio.SetSize(arx, ary);
	} else {
		m_AspectRatio = VideoSize;
	}

	if (FAILED(hr = AllocSurfaces())) {
		return hr;
	}

	if (!(lpAllocInfo->dwFlags & VMR9AllocFlag_TextureSurface)) {
		// test if the colorspace is acceptable
		if (FAILED(hr = m_pD3DDev->StretchRect(m_pSurfaces[0], NULL, m_pVideoSurface[m_nCurSurface], NULL, D3DTEXF_NONE))) {
			DeleteSurfaces();
			return E_FAIL;
		}
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], NULL, 0);

	if (m_nVMR9Surfaces && m_nVMR9Surfaces != (int)*lpNumBuffers) {
		m_nVMR9Surfaces = *lpNumBuffers;
	}
	*lpNumBuffers = min(nOriginal, *lpNumBuffers);
	m_iVMR9Surface = 0;

	return hr;
}

STDMETHODIMP CVMR9AllocatorPresenter::TerminateDevice(DWORD_PTR dwUserID)
{
	// We should not free the surfaces until we are told to !
	// Thats what TerminateDevice is for
	DeleteSurfaces();
	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface)
{
	if (!lplpSurface) {
		return E_POINTER;
	}

	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	/*
	SurfaceIndex = 0
	m_pSurfaces.GetCount() = 0

	Scenario:
	Thread 1:
		Wait on m_RenderLock in this function
	Thread 2:
		Have m_RenderLock and removes all m_pSurfaces
		(Happens by calling ex CDX9AllocatorPresenter::ResetDevice)

	When thread 2 releases the lock thread 1 gets it and boom!

	Possible solution: Adding object lock and moving m_RenderLock to try to fix this threading issue.
	This problem occurs when moving the window from display a to display b.

	NOTE: This is just a workaround.
	CDX9AllocatorPresenter doesn't follow the rules which is why this happened.
	And it is used by EVR custom (which it really shouldn't) so i can't easily fix it without breaking EVR custom.
	*/
	if (SurfaceIndex >= m_pSurfaces.GetCount()) {
		return E_FAIL;
	}

	if (m_nVMR9Surfaces) {
		++m_iVMR9Surface;
		m_iVMR9Surface = m_iVMR9Surface % m_nVMR9Surfaces;
		*lplpSurface = m_pSurfaces[m_iVMR9Surface + SurfaceIndex];
		(*lplpSurface)->AddRef();
	} else {
		m_iVMR9Surface = SurfaceIndex;
		*lplpSurface = m_pSurfaces[SurfaceIndex];
		(*lplpSurface)->AddRef();
	}

	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
{
	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	m_pIVMRSurfAllocNotify = lpIVMRSurfAllocNotify;

	HRESULT hr;
	HMONITOR hMonitor = m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D));
	if (FAILED(hr = m_pIVMRSurfAllocNotify->SetD3DDevice(m_pD3DDev, hMonitor))) {
		return hr;
	}

	return S_OK;
}

// IVMRImagePresenter9

STDMETHODIMP CVMR9AllocatorPresenter::StartPresenting(DWORD_PTR dwUserID)
{
	if (!m_bPendingResetDevice) {
		ASSERT(m_pD3DDev);
	}

	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	return m_pD3DDev ? S_OK : E_FAIL;
}

STDMETHODIMP CVMR9AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
{
	SetThreadName((DWORD)-1, "CVMR9AllocatorPresenter");
	CheckPointer(m_pIVMRSurfAllocNotify, E_UNEXPECTED);

	if (m_rtTimePerFrame == 0 || m_bNeedCheckSample) {
		m_bNeedCheckSample		= false;
		CComPtr<IBaseFilter>	pVMR9;
		CComPtr<IPin>			pPin;
		CMediaType				mt;

		if (SUCCEEDED (m_pIVMRSurfAllocNotify->QueryInterface (__uuidof(IBaseFilter), (void**)&pVMR9)) &&
				SUCCEEDED (pVMR9->FindPin(L"VMR Input0", &pPin)) &&
				SUCCEEDED (pPin->ConnectionMediaType(&mt)) ) {
			ExtractAvgTimePerFrame (&mt, m_rtTimePerFrame);

			CSize NativeVideoSize = m_NativeVideoSize;
			CSize AspectRatio = m_AspectRatio;
			if (mt.formattype==FORMAT_VideoInfo || mt.formattype==FORMAT_MPEGVideo) {
				VIDEOINFOHEADER *vh = (VIDEOINFOHEADER*)mt.pbFormat;

				NativeVideoSize = CSize(vh->bmiHeader.biWidth, abs(vh->bmiHeader.biHeight));
				if (vh->rcTarget.right - vh->rcTarget.left > 0) {
					NativeVideoSize.cx = vh->rcTarget.right - vh->rcTarget.left;
				} else if (vh->rcSource.right - vh->rcSource.left > 0) {
					NativeVideoSize.cx = vh->rcSource.right - vh->rcSource.left;
				}

				if (vh->rcTarget.bottom - vh->rcTarget.top > 0) {
					NativeVideoSize.cy = vh->rcTarget.bottom - vh->rcTarget.top;
				} else if (vh->rcSource.bottom - vh->rcSource.top > 0) {
					NativeVideoSize.cy = vh->rcSource.bottom - vh->rcSource.top;
				}
			} else if (mt.formattype==FORMAT_VideoInfo2 || mt.formattype==FORMAT_MPEG2Video) {
				VIDEOINFOHEADER2 *vh = (VIDEOINFOHEADER2*)mt.pbFormat;

				if (vh->dwPictAspectRatioX && vh->dwPictAspectRatioY) {
					AspectRatio = CSize(vh->dwPictAspectRatioX, vh->dwPictAspectRatioY);
				}

				NativeVideoSize = CSize(vh->bmiHeader.biWidth, abs(vh->bmiHeader.biHeight));
				if (vh->rcTarget.right - vh->rcTarget.left > 0) {
					NativeVideoSize.cx = vh->rcTarget.right - vh->rcTarget.left;
				} else if (vh->rcSource.right - vh->rcSource.left > 0) {
					NativeVideoSize.cx = vh->rcSource.right - vh->rcSource.left;
				}

				if (vh->rcTarget.bottom - vh->rcTarget.top > 0) {
					NativeVideoSize.cy = vh->rcTarget.bottom - vh->rcTarget.top;
				} else if (vh->rcSource.bottom - vh->rcSource.top > 0) {
					NativeVideoSize.cy = vh->rcSource.bottom - vh->rcSource.top;
				}
			}
			if (m_NativeVideoSize != NativeVideoSize || m_AspectRatio != AspectRatio) {
				m_NativeVideoSize = NativeVideoSize;
				m_AspectRatio = AspectRatio;
				AfxGetApp()->m_pMainWnd->PostMessage(WM_REARRANGERENDERLESS);
			}
		}
		// If framerate not set by Video Decoder choose 23.97...
		if (m_rtTimePerFrame == 0) {
			m_rtTimePerFrame = 417166;
		}

		m_fps = 10000000.0 / m_rtTimePerFrame;
	}

	HRESULT hr;

	if (!lpPresInfo || !lpPresInfo->lpSurf) {
		return E_POINTER;
	}

	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	if (lpPresInfo->rtEnd > lpPresInfo->rtStart) {
		if (m_pSubPicQueue) {
			m_pSubPicQueue->SetFPS(m_fps);

			if (m_fUseInternalTimer && !g_bExternalSubtitleTime) {
				__super::SetTime(g_tSegmentStart + g_tSampleStart);
			}
		}
	}

	CSize VideoSize = GetVisibleVideoSize();
	int arx = lpPresInfo->szAspectRatio.cx;
	int ary = lpPresInfo->szAspectRatio.cy;
	if (arx > 0 && ary > 0) {
		arx = arx / ((float) m_NativeVideoSize.cx / VideoSize.cx);
		ary = ary / ((float) m_NativeVideoSize.cy / VideoSize.cy);
		VideoSize.cx = VideoSize.cy*arx/ary;
	}
	if (VideoSize != GetVideoSize()) {
		m_AspectRatio.SetSize(arx, ary);
		AfxGetApp()->m_pMainWnd->PostMessage(WM_REARRANGERENDERLESS);
	}

	if (!m_bPendingResetDevice) {
		CComPtr<IDirect3DTexture9> pTexture;
		lpPresInfo->lpSurf->GetContainer(IID_IDirect3DTexture9, (void**)&pTexture);

		if (pTexture) {
			m_pVideoSurface[m_nCurSurface] = lpPresInfo->lpSurf;
			if (m_pVideoTexture[m_nCurSurface]) {
				m_pVideoTexture[m_nCurSurface] = pTexture;
			}
		} else {
			hr = m_pD3DDev->StretchRect(lpPresInfo->lpSurf, NULL, m_pVideoSurface[m_nCurSurface], NULL, D3DTEXF_NONE);
		}

		// Tear test bars
		if (GetRenderersData()->m_fTearingTest) {
			RECT		rcTearing;

			rcTearing.left		= m_nTearingPos;
			rcTearing.top		= 0;
			rcTearing.right		= rcTearing.left + 4;
			rcTearing.bottom	= m_NativeVideoSize.cy;
			m_pD3DDev->ColorFill (m_pVideoSurface[m_nCurSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

			rcTearing.left	= (rcTearing.right + 15) % m_NativeVideoSize.cx;
			rcTearing.right	= rcTearing.left + 4;
			m_pD3DDev->ColorFill (m_pVideoSurface[m_nCurSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

			m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
		}
	}

	Paint(true);

	return S_OK;
}

// IVMRWindowlessControl9
//
// It is only implemented (partially) for the dvd navigator's
// menu handling, which needs to know a few things about the
// location of our window.

STDMETHODIMP CVMR9AllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	if (lpWidth) {
		*lpWidth = m_NativeVideoSize.cx;
	}
	if (lpHeight) {
		*lpHeight = m_NativeVideoSize.cy;
	}
	if (lpARWidth) {
		*lpARWidth = m_AspectRatio.cx;
	}
	if (lpARHeight) {
		*lpARHeight = m_AspectRatio.cy;
	}
	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect)
{
	return E_NOTIMPL;   // we have our own method for this
}

STDMETHODIMP CVMR9AllocatorPresenter::GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect)
{
	CopyRect(lpSRCRect, CRect(CPoint(0, 0), GetVisibleVideoSize()));
	CopyRect(lpDSTRect, &m_VideoRect);
	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetAspectRatioMode(DWORD* lpAspectRatioMode)
{
	if (lpAspectRatioMode) {
		*lpAspectRatioMode = AM_ARMODE_STRETCHED;
	}
	return S_OK;
}

STDMETHODIMP CVMR9AllocatorPresenter::SetAspectRatioMode(DWORD AspectRatioMode)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::SetVideoClippingWindow(HWND hwnd)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::RepaintVideo(HWND hwnd, HDC hdc)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::DisplayModeChanged()
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetCurrentImage(BYTE** lpDib)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::SetBorderColor(COLORREF Clr)
{
	return E_NOTIMPL;
}

STDMETHODIMP CVMR9AllocatorPresenter::GetBorderColor(COLORREF* lpClr)
{
	if (lpClr) {
		*lpClr = 0;
	}
	return S_OK;
}
