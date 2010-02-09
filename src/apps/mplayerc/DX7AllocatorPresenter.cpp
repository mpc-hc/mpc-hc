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
#include "mplayerc.h"
#include <atlbase.h>
#include <atlcoll.h>
#include "../../DSUtil/DSUtil.h"

#include <initguid.h>
#include "DX7AllocatorPresenter.h"
#include <ddraw.h>
#include <d3d.h>
#include "../../SubPic/DX7SubPic.h"
#include <RealMedia/pntypes.h>
#include <RealMedia/pnwintyp.h>
#include <RealMedia/pncom.h>
#include <RealMedia/rmavsurf.h>
#include "IQTVideoSurface.h"

#include "IPinHook.h"


bool IsVMR7InGraph(IFilterGraph* pFG)
{
	BeginEnumFilters(pFG, pEF, pBF)
		if(CComQIPtr<IVMRWindowlessControl>(pBF)) return(true);
	EndEnumFilters
	return(false);
}

namespace DSObjects
{

	class CDX7AllocatorPresenter
		: public ISubPicAllocatorPresenterImpl
	{
	protected:
		CSize m_ScreenSize;

		CComPtr<IDirectDraw7> m_pDD;
		CComQIPtr<IDirect3D7, &IID_IDirect3D7> m_pD3D;
		CComPtr<IDirect3DDevice7> m_pD3DDev;

		CComPtr<IDirectDrawSurface7> m_pPrimary, m_pBackBuffer;
		CComPtr<IDirectDrawSurface7> m_pVideoTexture, m_pVideoSurface;

	    virtual HRESULT CreateDevice();
		virtual HRESULT AllocSurfaces();
		virtual void DeleteSurfaces();

	public:
		CDX7AllocatorPresenter(HWND hWnd, HRESULT& hr);

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(bool) Paint(bool fAll);
		STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size);
	};

	class CVMR7AllocatorPresenter
		: public CDX7AllocatorPresenter
		, public IVMRSurfaceAllocator
		, public IVMRImagePresenter
		, public IVMRWindowlessControl
	{
		CComPtr<IVMRSurfaceAllocatorNotify> m_pIVMRSurfAllocNotify;
		CComPtr<IVMRSurfaceAllocator> m_pSA;

		HRESULT CreateDevice();
		void DeleteSurfaces();

		bool m_fUseInternalTimer;

	public:
		CVMR7AllocatorPresenter(HWND hWnd, HRESULT& hr);

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);

		// IVMRSurfaceAllocator
		STDMETHODIMP AllocateSurface(DWORD_PTR dwUserID, VMRALLOCATIONINFO* lpAllocInfo, DWORD* lpdwBuffer, LPDIRECTDRAWSURFACE7* lplpSurface);
		STDMETHODIMP FreeSurface(DWORD_PTR dwUserID);
		STDMETHODIMP PrepareSurface(DWORD_PTR dwUserID, IDirectDrawSurface7* lpSurface, DWORD dwSurfaceFlags);
		STDMETHODIMP AdviseNotify(IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify);

		// IVMRImagePresenter
		STDMETHODIMP StartPresenting(DWORD_PTR dwUserID);
		STDMETHODIMP StopPresenting(DWORD_PTR dwUserID);
		STDMETHODIMP PresentImage(DWORD_PTR dwUserID, VMRPRESENTATIONINFO* lpPresInfo);

		// IVMRWindowlessControl
		STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight);
		STDMETHODIMP GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight);
		STDMETHODIMP GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight);
		STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect);
		STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect);
		STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode);
		STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode);
		STDMETHODIMP SetVideoClippingWindow(HWND hwnd);
		STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc);
		STDMETHODIMP DisplayModeChanged();
		STDMETHODIMP GetCurrentImage(BYTE** lpDib);
		STDMETHODIMP SetBorderColor(COLORREF Clr);
		STDMETHODIMP GetBorderColor(COLORREF* lpClr);
		STDMETHODIMP SetColorKey(COLORREF Clr);
		STDMETHODIMP GetColorKey(COLORREF* lpClr);
	};

	class CRM7AllocatorPresenter
		: public CDX7AllocatorPresenter
		, public IRMAVideoSurface
	{
		CComPtr<IDirectDrawSurface7> m_pVideoSurfaceOff;
		CComPtr<IDirectDrawSurface7> m_pVideoSurfaceYUY2;

		RMABitmapInfoHeader m_bitmapInfo;
		RMABitmapInfoHeader m_lastBitmapInfo;

	protected:
		HRESULT AllocSurfaces();
		void DeleteSurfaces();

	public:
		CRM7AllocatorPresenter(HWND hWnd, HRESULT& hr);

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		// IRMAVideoSurface
		STDMETHODIMP Blt(UCHAR*	pImageData, RMABitmapInfoHeader* pBitmapInfo, REF(PNxRect) inDestRect, REF(PNxRect) inSrcRect);
		STDMETHODIMP BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo);
		STDMETHODIMP OptimizedBlt(UCHAR* pImageBits, REF(PNxRect) rDestRect, REF(PNxRect) rSrcRect);
		STDMETHODIMP EndOptimizedBlt();
		STDMETHODIMP GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType);
		STDMETHODIMP GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType);
	};

	class CQT7AllocatorPresenter
		: public CDX7AllocatorPresenter
		, public IQTVideoSurface
	{
		CComPtr<IDirectDrawSurface7> m_pVideoSurfaceOff;

	protected:
		HRESULT AllocSurfaces();
		void DeleteSurfaces();

	public:
		CQT7AllocatorPresenter(HWND hWnd, HRESULT& hr);

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		// IQTVideoSurface
		STDMETHODIMP BeginBlt(const BITMAP& bm);
		STDMETHODIMP DoBlt(const BITMAP& bm);
	};

}
using namespace DSObjects;

//

HRESULT CreateAP7(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP)
{
	CheckPointer(ppAP, E_POINTER);

	*ppAP = NULL;

	HRESULT hr;
	if(clsid == CLSID_VMR7AllocatorPresenter && !(*ppAP = DNew CVMR7AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_RM7AllocatorPresenter && !(*ppAP = DNew CRM7AllocatorPresenter(hWnd, hr))
	|| clsid == CLSID_QT7AllocatorPresenter && !(*ppAP = DNew CQT7AllocatorPresenter(hWnd, hr)))
		return E_OUTOFMEMORY;

	if(*ppAP == NULL)
		return E_FAIL;

	(*ppAP)->AddRef();

	if(FAILED(hr))
	{
		(*ppAP)->Release();
		*ppAP = NULL;
	}

	return hr;
}

//

static HRESULT TextureBlt(CComPtr<IDirect3DDevice7> pD3DDev, CComPtr<IDirectDrawSurface7> pTexture, Vector dst[4], CRect src)
{
	if(!pTexture)
		return E_POINTER;

	HRESULT hr;

    do
	{
		DDSURFACEDESC2 ddsd;
		INITDDSTRUCT(ddsd);
		if(FAILED(hr = pTexture->GetSurfaceDesc(&ddsd)))
			break;

		float w = (float)ddsd.dwWidth;
		float h = (float)ddsd.dwHeight;

		struct
		{
			float x, y, z, rhw;
			float tu, tv;
		}
		pVertices[] =
		{
			{(float)dst[0].x, (float)dst[0].y, (float)dst[0].z, 1.0f/(float)dst[0].z, (float)src.left / w, (float)src.top / h},
			{(float)dst[1].x, (float)dst[1].y, (float)dst[1].z, 1.0f/(float)dst[1].z, (float)src.right / w, (float)src.top / h},
			{(float)dst[2].x, (float)dst[2].y, (float)dst[2].z, 1.0f/(float)dst[2].z, (float)src.left / w, (float)src.bottom / h},
			{(float)dst[3].x, (float)dst[3].y, (float)dst[3].z, 1.0f/(float)dst[3].z, (float)src.right / w, (float)src.bottom / h},
		};

		for(int i = 0; i < countof(pVertices); i++)
		{
			pVertices[i].x -= 0.5;
			pVertices[i].y -= 0.5;
		}

		hr = pD3DDev->SetTexture(0, pTexture);

		pD3DDev->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
		pD3DDev->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
		pD3DDev->SetRenderState(D3DRENDERSTATE_BLENDENABLE, FALSE);
		pD3DDev->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE); 

		pD3DDev->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		pD3DDev->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		pD3DDev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

		pD3DDev->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);

		//

		if(FAILED(hr = pD3DDev->BeginScene()))
			break;

		hr = pD3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,
									D3DFVF_XYZRHW | D3DFVF_TEX1,
									pVertices, 4, D3DDP_WAIT);
		pD3DDev->EndScene();

        //

		pD3DDev->SetTexture(0, NULL);

		return S_OK;
	}
	while(0);

	return E_FAIL;
}

//
// CDX7AllocatorPresenter
//

CDX7AllocatorPresenter::CDX7AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: ISubPicAllocatorPresenterImpl(hWnd, hr, NULL)
	, m_ScreenSize(0, 0)
{
	if(FAILED(hr)) return;

	if(FAILED(hr = DirectDrawCreateEx(NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL))
	|| FAILED(hr = m_pDD->SetCooperativeLevel(AfxGetMainWnd()->GetSafeHwnd(), DDSCL_NORMAL)))
		return;

	if(!(m_pD3D = m_pDD)) {hr = E_NOINTERFACE; return;}

	hr = CreateDevice();
}

HRESULT CDX7AllocatorPresenter::CreateDevice()
{
	m_pD3DDev = NULL;

	m_pPrimary = NULL;
	m_pBackBuffer = NULL;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	if(FAILED(m_pDD->GetDisplayMode(&ddsd)) ||
	   ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
		return DDERR_INVALIDMODE;

	m_ScreenSize.SetSize(ddsd.dwWidth, ddsd.dwHeight);

	HRESULT hr;

	// m_pPrimary

	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if(FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pPrimary, NULL)))
		return hr;

	CComPtr<IDirectDrawClipper> pcClipper;
	if(FAILED(hr = m_pDD->CreateClipper(0, &pcClipper, NULL)))
		return hr;
	pcClipper->SetHWnd(0, m_hWnd);
	m_pPrimary->SetClipper(pcClipper);

	// m_pBackBuffer

	INITDDSTRUCT(ddsd);
	ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = /*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
	ddsd.dwWidth = m_ScreenSize.cx;
	ddsd.dwHeight = m_ScreenSize.cy;
	if(FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pBackBuffer, NULL)))
      	  return hr;

	pcClipper = NULL;
	if(FAILED(hr = m_pDD->CreateClipper(0, &pcClipper, NULL)))
		return hr;
	BYTE rgnDataBuffer[1024];
	HRGN hrgn = CreateRectRgn(0, 0, ddsd.dwWidth, ddsd.dwHeight);
	GetRegionData(hrgn, sizeof(rgnDataBuffer), (RGNDATA*)rgnDataBuffer);
	DeleteObject(hrgn);
	pcClipper->SetClipList((RGNDATA*)rgnDataBuffer, 0);
	m_pBackBuffer->SetClipper(pcClipper);

	// m_pD3DDev

	if(FAILED(hr = m_pD3D->CreateDevice(IID_IDirect3DHALDevice, m_pBackBuffer, &m_pD3DDev))) // this seems to fail if the desktop size is too large (width or height >2048)
		return hr;

	//

	CComPtr<ISubPicProvider> pSubPicProvider;
	if(m_pSubPicQueue) m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);

	CSize size;
	switch(AfxGetAppSettings().nSPCMaxRes)
	{
	case 0: default: size = m_ScreenSize; break;
	case 1: size.SetSize(1024, 768); break;
	case 2: size.SetSize(800, 600); break;
	case 3: size.SetSize(640, 480); break;
	case 4: size.SetSize(512, 384); break;
	case 5: size.SetSize(384, 288); break;
	case 6: size.SetSize(2560, 1600); break;
	case 7: size.SetSize(1920, 1080); break;
	case 8: size.SetSize(1320, 900); break;
	case 9: size.SetSize(1280, 720); break;
	}

	if(m_pAllocator)
	{
		m_pAllocator->ChangeDevice(m_pD3DDev);
	}
	else
	{
		m_pAllocator = DNew CDX7SubPicAllocator(m_pD3DDev, size, AfxGetAppSettings().fSPCPow2Tex);
		if(!m_pAllocator || FAILED(hr))
			return E_FAIL;
	}

	hr = S_OK;
	m_pSubPicQueue = AfxGetAppSettings().nSPCSize > 0 
		? (ISubPicQueue*)DNew CSubPicQueue(AfxGetAppSettings().nSPCSize, !AfxGetAppSettings().fSPCAllowAnimationWhenBuffering, m_pAllocator, &hr)
		: (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);
	if(!m_pSubPicQueue || FAILED(hr))
		return E_FAIL;

	if(pSubPicProvider) m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);

	return S_OK;
}

HRESULT CDX7AllocatorPresenter::AllocSurfaces()
{
	CAutoLock cAutoLock(this);

	AppSettings& s = AfxGetAppSettings();

	m_pVideoTexture = NULL;
	m_pVideoSurface = NULL;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = m_NativeVideoSize.cx;
	ddsd.dwHeight = m_NativeVideoSize.cy;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount	= 32;
	ddsd.ddpfPixelFormat.dwRBitMask		= 0x00FF0000;
	ddsd.ddpfPixelFormat.dwGBitMask		= 0x0000FF00;
	ddsd.ddpfPixelFormat.dwBBitMask		= 0x000000FF;

	if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
	{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
//		ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
//		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= 0xFF000000;
	}

	HRESULT hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurface, NULL);
	if(FAILED(hr))
	{
		// FIXME: eh, dx9 has no problem creating a 32bpp surface under a 16bpp desktop, but dx7 fails here (textures are ok)
		DDSURFACEDESC2 ddsd2;
		INITDDSTRUCT(ddsd2);
		if(!(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
		&& SUCCEEDED(m_pDD->GetDisplayMode(&ddsd2))
		&& ddsd2.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			ddsd.ddpfPixelFormat.dwRGBBitCount	= 16;
			ddsd.ddpfPixelFormat.dwRBitMask		= 0x0000F800;
			ddsd.ddpfPixelFormat.dwGBitMask		= 0x000007E0;
			ddsd.ddpfPixelFormat.dwBBitMask		= 0x0000001F;
			hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurface, NULL);
		}

		if(FAILED(hr))
			return hr;
	}

	if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
		m_pVideoTexture = m_pVideoSurface;

	DDBLTFX fx;
	INITDDSTRUCT(fx);
	fx.dwFillColor = 0;
	hr = m_pVideoSurface->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

	return S_OK;
}

void CDX7AllocatorPresenter::DeleteSurfaces()
{
	CAutoLock cAutoLock(this);

	m_pVideoTexture = NULL;
	m_pVideoSurface = NULL;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CDX7AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(bool) CDX7AllocatorPresenter::Paint(bool fAll)
{
	CAutoLock cAutoLock(this);

	if(m_WindowRect.right <= m_WindowRect.left || m_WindowRect.bottom <= m_WindowRect.top
	|| m_NativeVideoSize.cx <= 0 || m_NativeVideoSize.cy <= 0
	|| !m_pPrimary || !m_pBackBuffer || !m_pVideoSurface)
		return(false);

	HRESULT hr;

	CRect rSrcVid(CPoint(0, 0), m_NativeVideoSize);
	CRect rDstVid(m_VideoRect);

	CRect rSrcPri(CPoint(0, 0), m_WindowRect.Size());
	CRect rDstPri(m_WindowRect);
	MapWindowRect(m_hWnd, HWND_DESKTOP, &rDstPri);

	if(fAll)
	{
		// clear the backbuffer

		CRect rl(0, 0, rDstVid.left, rSrcPri.bottom);
		CRect rr(rDstVid.right, 0, rSrcPri.right, rSrcPri.bottom);
		CRect rt(0, 0, rSrcPri.right, rDstVid.top);
		CRect rb(0, rDstVid.bottom, rSrcPri.right, rSrcPri.bottom);

		DDBLTFX fx;
		INITDDSTRUCT(fx);
		fx.dwFillColor = 0;
		hr = m_pBackBuffer->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

		// paint the video on the backbuffer

		if(!rDstVid.IsRectEmpty())
		{
			if(m_pVideoTexture)
			{
				Vector v[4];
				Transform(rDstVid, v);
				hr = TextureBlt(m_pD3DDev, m_pVideoTexture, v, rSrcVid);
			}
			else
			{
				hr = m_pBackBuffer->Blt(rDstVid, m_pVideoSurface, rSrcVid, DDBLT_WAIT, NULL);
			}
		}

		// paint the text on the backbuffer

		AlphaBltSubPic(rSrcPri.Size());
	}

	// wait vsync

	m_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);

	// blt to the primary surface

	hr = m_pPrimary->Blt(rDstPri, m_pBackBuffer, rSrcPri, DDBLT_WAIT, NULL);

	if(hr == DDERR_SURFACELOST)
	{
		HRESULT hr = DDERR_WRONGMODE; // m_pDD->TestCooperativeLevel();

		if(hr == DDERR_WRONGMODE) 
		{
			DeleteSurfaces();
			if(SUCCEEDED(CreateDevice()) || FAILED(hr = AllocSurfaces()))
				return(true);
		}

		hr = S_OK;
	}

	return(true);
}

STDMETHODIMP CDX7AllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	CheckPointer(size, E_POINTER);

	HRESULT hr;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	if(FAILED(m_pVideoSurface->GetSurfaceDesc(&ddsd)))
		return E_FAIL;

	if(ddsd.ddpfPixelFormat.dwRGBBitCount != 16 && ddsd.ddpfPixelFormat.dwRGBBitCount != 32)
		return E_FAIL;

	DWORD required = sizeof(BITMAPINFOHEADER) + (ddsd.dwWidth*ddsd.dwHeight*32>>3);
	if(!lpDib) {*size = required; return S_OK;}
	if(*size < required) return E_OUTOFMEMORY;
	*size = required;

	INITDDSTRUCT(ddsd);
	if(FAILED(hr = m_pVideoSurface->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_NOSYSLOCK, NULL)))
	{
		// TODO
		return hr;
	}

	BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)lpDib;
	memset(bih, 0, sizeof(BITMAPINFOHEADER));
	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = ddsd.dwWidth;
	bih->biHeight = ddsd.dwHeight;
	bih->biBitCount = 32;
	bih->biPlanes = 1;
	bih->biSizeImage = bih->biWidth*bih->biHeight*bih->biBitCount>>3;

	BitBltFromRGBToRGB(
		bih->biWidth, bih->biHeight, 
		(BYTE*)(bih + 1), bih->biWidth*bih->biBitCount>>3, bih->biBitCount,
		(BYTE*)ddsd.lpSurface + ddsd.lPitch*(ddsd.dwHeight-1), -(int)ddsd.lPitch, ddsd.ddpfPixelFormat.dwRGBBitCount);

	m_pVideoSurface->Unlock(NULL);

/*
			BitBltFromRGBToRGB(
				w, h, 
				(BYTE*)ddsd.lpSurface, ddsd.lPitch, ddsd.ddpfPixelFormat.dwRGBBitCount,
				(BYTE*)bm.bmBits, bm.bmWidthBytes, bm.bmBitsPixel);
			m_pVideoSurfaceOff->Unlock(NULL);
			fOk = true;
		}
*/

	return S_OK;
}

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
	CAutoLock cAutoLock(this);

	ASSERT(m_pD3DDev);

	return m_pD3DDev ? S_OK : E_FAIL;
}

STDMETHODIMP CVMR7AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
	return S_OK;
}

extern bool g_bNoDuration;
extern bool g_bExternalSubtitleTime;

STDMETHODIMP CVMR7AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMRPRESENTATIONINFO* lpPresInfo)
{
	if(!lpPresInfo || !lpPresInfo->lpSurf)
		return E_POINTER;

	CAutoLock cAutoLock(this);

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

STDMETHODIMP CVMR7AllocatorPresenter::GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) {return E_NOTIMPL;} // we have our own method for this

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

STDMETHODIMP CVMR7AllocatorPresenter::SetAspectRatioMode(DWORD AspectRatioMode) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::SetVideoClippingWindow(HWND hwnd) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::RepaintVideo(HWND hwnd, HDC hdc) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::DisplayModeChanged() {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::GetCurrentImage(BYTE** lpDib) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::SetBorderColor(COLORREF Clr) {return E_NOTIMPL;}

STDMETHODIMP CVMR7AllocatorPresenter::GetBorderColor(COLORREF* lpClr)
{
	if(lpClr) *lpClr = 0;
	return S_OK;
}

STDMETHODIMP CVMR7AllocatorPresenter::SetColorKey(COLORREF Clr) {return E_NOTIMPL;}
STDMETHODIMP CVMR7AllocatorPresenter::GetColorKey(COLORREF* lpClr) {return E_NOTIMPL;}

//

static HRESULT AllocDX7Surface(IDirectDraw7* pDD, CSize size, DWORD compression, int bpp, IDirectDrawSurface7** pSurface)
{
	if(!pDD || !pSurface || size.cx <= 0 || size.cy <= 0)
		return E_POINTER;

	*pSurface = NULL;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = size.cx;
	ddsd.dwHeight = size.cy;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	if(compression >= 0x1000)
	{
		ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		ddsd.ddpfPixelFormat.dwFourCC = compression;
	}
	else if((compression == 0 || compression == 3) && (bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32))
	{
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = max(bpp, 16);
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= (bpp == 16) ? 0x0000 : (bpp == 15) ? 0x8000 : 0xFF000000;
		ddsd.ddpfPixelFormat.dwRBitMask			= (bpp == 16) ? 0xf800 : (bpp == 15) ? 0x7c00 : 0x00FF0000;
		ddsd.ddpfPixelFormat.dwGBitMask			= (bpp == 16) ? 0x07e0 : (bpp == 15) ? 0x03e0 : 0x0000FF00;
		ddsd.ddpfPixelFormat.dwBBitMask			= (bpp == 16) ? 0x001F : (bpp == 15) ? 0x001F : 0x000000FF;
	}

	return pDD->CreateSurface(&ddsd, pSurface, NULL);
}

//
// CRM7AllocatorPresenter
//

CRM7AllocatorPresenter::CRM7AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX7AllocatorPresenter(hWnd, hr)
{
}

STDMETHODIMP CRM7AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return 
		QI2(IRMAVideoSurface)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CRM7AllocatorPresenter::AllocSurfaces()
{
	CAutoLock cAutoLock(this);

	m_pVideoSurfaceOff = NULL;
	m_pVideoSurfaceYUY2 = NULL;

	DDSURFACEDESC2 ddsd;
	DDBLTFX fx;

	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = m_NativeVideoSize.cx;
	ddsd.dwHeight = m_NativeVideoSize.cy;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= 0xFF000000;
	ddsd.ddpfPixelFormat.dwRBitMask			= 0x00FF0000;
	ddsd.ddpfPixelFormat.dwGBitMask			= 0x0000FF00;
	ddsd.ddpfPixelFormat.dwBBitMask			= 0x000000FF;

	HRESULT hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurfaceOff, NULL);
	if(FAILED(hr)) return E_FAIL;

	INITDDSTRUCT(fx);
	fx.dwFillColor = 0;
	m_pVideoSurfaceOff->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = m_NativeVideoSize.cx;
	ddsd.dwHeight = m_NativeVideoSize.cy;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
	ddsd.ddpfPixelFormat.dwYUVBitCount = 16;
	ddsd.ddpfPixelFormat.dwFourCC = '2YUY';

	hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurfaceYUY2, NULL);

	if(FAILED(m_pVideoSurfaceOff->Blt(NULL, m_pVideoSurfaceYUY2, NULL, DDBLT_WAIT, NULL)))
		m_pVideoSurfaceYUY2 = NULL;

	if(m_pVideoSurfaceYUY2)
	{
		INITDDSTRUCT(fx);
		fx.dwFillColor = 0x80108010;
		m_pVideoSurfaceYUY2->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);
	}

	return __super::AllocSurfaces();
}

void CRM7AllocatorPresenter::DeleteSurfaces()
{
	CAutoLock cAutoLock(this);

	m_pVideoSurfaceOff = NULL;
	m_pVideoSurfaceYUY2 = NULL;

	__super::DeleteSurfaces();
}

// IRMAVideoSurface

STDMETHODIMP CRM7AllocatorPresenter::Blt(UCHAR* pImageData, RMABitmapInfoHeader* pBitmapInfo, REF(PNxRect) inDestRect, REF(PNxRect) inSrcRect)
{
	if(!m_pVideoSurface || !m_pVideoSurfaceOff)
		return E_FAIL;

	bool fRGB = false;
	bool fYUY2 = false;

	CRect src((RECT*)&inSrcRect), dst((RECT*)&inDestRect), src2(CPoint(0,0), src.Size());
	if(src.Width() > dst.Width() || src.Height() > dst.Height())
		return E_FAIL;

	DDSURFACEDESC2 ddsd;

	if(pBitmapInfo->biCompression == '024I')
	{
		DWORD pitch = pBitmapInfo->biWidth;
		DWORD size = pitch*abs(pBitmapInfo->biHeight);

		BYTE* y = pImageData					+ src.top*pitch + src.left;
		BYTE* u = pImageData + size				+ src.top*(pitch/2) + src.left/2;
		BYTE* v = pImageData + size + size/4	+ src.top*(pitch/2) + src.left/2;

		if(m_pVideoSurfaceYUY2)
		{
			INITDDSTRUCT(ddsd);
			if(SUCCEEDED(m_pVideoSurfaceYUY2->Lock(src2, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL)))
			{
				BitBltFromI420ToYUY2(src.Width(), src.Height(), (BYTE*)ddsd.lpSurface, ddsd.lPitch, y, u, v, pitch);
				m_pVideoSurfaceYUY2->Unlock(src2);
				fYUY2 = true;
			}
		}
		else
		{
			INITDDSTRUCT(ddsd);
			if(SUCCEEDED(m_pVideoSurfaceOff->Lock(src2, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL)))
			{
				BitBltFromI420ToRGB(src.Width(), src.Height(), (BYTE*)ddsd.lpSurface, ddsd.lPitch, ddsd.ddpfPixelFormat.dwRGBBitCount, y, u, v, pitch);
				m_pVideoSurfaceOff->Unlock(src2);
				fRGB = true;
			}
		}
	}
	else if(pBitmapInfo->biCompression == '2YUY')
	{
		DWORD w = pBitmapInfo->biWidth;
		DWORD h = abs(pBitmapInfo->biHeight);
		DWORD pitch = pBitmapInfo->biWidth*2;

		BYTE* yvyu = pImageData + src.top*pitch + src.left*2;

		if(m_pVideoSurfaceYUY2)
		{
			INITDDSTRUCT(ddsd);
			if(SUCCEEDED(m_pVideoSurfaceYUY2->Lock(src2, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL)))
			{
				BitBltFromYUY2ToYUY2(src.Width(), src.Height(), (BYTE*)ddsd.lpSurface, ddsd.lPitch, yvyu, pitch);
				m_pVideoSurfaceYUY2->Unlock(src2);
				fYUY2 = true;
			}
		}
		else
		{
			INITDDSTRUCT(ddsd);
			if(SUCCEEDED(m_pVideoSurfaceOff->Lock(src2, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL)))
			{
				BitBltFromYUY2ToRGB(src.Width(), src.Height(), (BYTE*)ddsd.lpSurface, ddsd.lPitch, ddsd.ddpfPixelFormat.dwRGBBitCount, yvyu, pitch);
				m_pVideoSurfaceOff->Unlock(src2);
				fRGB = true;
			}
		}
	}
	else if(pBitmapInfo->biCompression == 0 || pBitmapInfo->biCompression == 3
		 || pBitmapInfo->biCompression == 'BGRA')
	{
		DWORD w = pBitmapInfo->biWidth;
		DWORD h = abs(pBitmapInfo->biHeight);
		DWORD pitch = pBitmapInfo->biWidth*pBitmapInfo->biBitCount>>3;

		BYTE* rgb = pImageData + src.top*pitch + src.left*(pBitmapInfo->biBitCount>>3);

		INITDDSTRUCT(ddsd);
		if(SUCCEEDED(m_pVideoSurfaceOff->Lock(src2, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL)))
		{
			BYTE* lpSurface = (BYTE*)ddsd.lpSurface;
			if(pBitmapInfo->biHeight > 0) {lpSurface += ddsd.lPitch*(src.Height()-1); ddsd.lPitch = -ddsd.lPitch;}
			BitBltFromRGBToRGB(src.Width(), src.Height(), lpSurface, ddsd.lPitch, ddsd.ddpfPixelFormat.dwRGBBitCount, rgb, pitch, pBitmapInfo->biBitCount);
			fRGB = true;
			m_pVideoSurfaceOff->Unlock(src2);
		}
	}

	if(!fRGB && !fYUY2)
	{
		DDBLTFX fx;
		INITDDSTRUCT(fx);
		fx.dwFillColor = 0;
		m_pVideoSurfaceOff->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

		HDC hDC;
		if(SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC)))
		{
			CString str;
			str.Format(_T("Sorry, this format is not supported"));

			SetBkColor(hDC, 0);
			SetTextColor(hDC, 0x404040);
			TextOut(hDC, 10, 10, str, str.GetLength());

			m_pVideoSurfaceOff->ReleaseDC(hDC);

			fRGB = true;
		}
	}


	HRESULT hr;

	if(fRGB)
		hr = m_pVideoSurface->Blt(dst, m_pVideoSurfaceOff, src2, DDBLT_WAIT, NULL);
	if(fYUY2)
		hr = m_pVideoSurface->Blt(dst, m_pVideoSurfaceYUY2, src2, DDBLT_WAIT, NULL);
    
	Paint(true);

	return PNR_OK;
}

STDMETHODIMP CRM7AllocatorPresenter::BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo)
{
	CAutoLock cAutoLock(this);
	DeleteSurfaces();
	m_NativeVideoSize = m_AspectRatio = CSize(pBitmapInfo->biWidth, abs(pBitmapInfo->biHeight));
	if(FAILED(AllocSurfaces())) return E_FAIL;
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM7AllocatorPresenter::OptimizedBlt(UCHAR* pImageBits, REF(PNxRect) rDestRect, REF(PNxRect) rSrcRect)
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM7AllocatorPresenter::EndOptimizedBlt()
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM7AllocatorPresenter::GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	return PNR_NOTIMPL;
}

STDMETHODIMP CRM7AllocatorPresenter::GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	ulType = RMA_I420;
	return PNR_OK;
}

//
// CQT7AllocatorPresenter
//

CQT7AllocatorPresenter::CQT7AllocatorPresenter(HWND hWnd, HRESULT& hr) 
	: CDX7AllocatorPresenter(hWnd, hr)
{
}

STDMETHODIMP CQT7AllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return 
		QI(IQTVideoSurface)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CQT7AllocatorPresenter::AllocSurfaces()
{
	CAutoLock cAutoLock(this);

	m_pVideoSurfaceOff = NULL;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = m_NativeVideoSize.cx;
	ddsd.dwHeight = m_NativeVideoSize.cy;
	ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= 0xFF000000;
	ddsd.ddpfPixelFormat.dwRBitMask			= 0x00FF0000;
	ddsd.ddpfPixelFormat.dwGBitMask			= 0x0000FF00;
	ddsd.ddpfPixelFormat.dwBBitMask			= 0x000000FF;

	HRESULT hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurfaceOff, NULL);
	if(FAILED(hr)) return E_FAIL;

	DDBLTFX fx;
	INITDDSTRUCT(fx);
	fx.dwFillColor = 0;
	m_pVideoSurfaceOff->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

	return __super::AllocSurfaces();
}

void CQT7AllocatorPresenter::DeleteSurfaces()
{
	CAutoLock cAutoLock(this);

	m_pVideoSurfaceOff = NULL;

	__super::DeleteSurfaces();
}

// IQTVideoSurface

STDMETHODIMP CQT7AllocatorPresenter::BeginBlt(const BITMAP& bm)
{
	CAutoLock cAutoLock(this);

	DeleteSurfaces();

	m_NativeVideoSize = m_AspectRatio = CSize(bm.bmWidth, abs(bm.bmHeight));

	HRESULT hr;
	if(FAILED(hr = AllocSurfaces()))
		return hr;

	return S_OK;
}

STDMETHODIMP CQT7AllocatorPresenter::DoBlt(const BITMAP& bm)
{
	if(!m_pVideoSurface || !m_pVideoSurfaceOff)
		return E_FAIL;

	bool fOk = false;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	if(FAILED(m_pVideoSurfaceOff->GetSurfaceDesc(&ddsd)))
		return E_FAIL;

	int w = bm.bmWidth;
	int h = abs(bm.bmHeight);
	int bpp = bm.bmBitsPixel;

	if((bpp == 16 || bpp == 24 || bpp == 32) && w == ddsd.dwWidth && h == ddsd.dwHeight)
	{
		INITDDSTRUCT(ddsd);
		if(SUCCEEDED(m_pVideoSurfaceOff->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_WRITEONLY, NULL)))
		{
			BitBltFromRGBToRGB(
				w, h, 
				(BYTE*)ddsd.lpSurface, ddsd.lPitch, ddsd.ddpfPixelFormat.dwRGBBitCount,
				(BYTE*)bm.bmBits, bm.bmWidthBytes, bm.bmBitsPixel);
			m_pVideoSurfaceOff->Unlock(NULL);
			fOk = true;
		}
	}

	if(!fOk)
	{
		DDBLTFX fx;
		INITDDSTRUCT(fx);
		fx.dwFillColor = 0;
		m_pVideoSurfaceOff->Blt(NULL, NULL, NULL, DDBLT_WAIT|DDBLT_COLORFILL, &fx);

		HDC hDC;
		if(SUCCEEDED(m_pVideoSurfaceOff->GetDC(&hDC)))
		{
			CString str;
			str.Format(_T("Sorry, this format is not supported"));

			SetBkColor(hDC, 0);
			SetTextColor(hDC, 0x404040);
			TextOut(hDC, 10, 10, str, str.GetLength());

			m_pVideoSurfaceOff->ReleaseDC(hDC);
		}
	}

	m_pVideoSurface->Blt(NULL, m_pVideoSurfaceOff, NULL, DDBLT_WAIT, NULL);

	Paint(true);

	return S_OK;
}
