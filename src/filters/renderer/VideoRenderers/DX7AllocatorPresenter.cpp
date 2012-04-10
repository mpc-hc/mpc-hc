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
#include "RenderersSettings.h"

#include "DX7AllocatorPresenter.h"
#include "../../../SubPic/DX7SubPic.h"
#include "../../../SubPic/SubPicQueueImpl.h"

using namespace DSObjects;

//

static HRESULT TextureBlt(IDirect3DDevice7* pD3DDev, IDirectDrawSurface7* pTexture, Vector dst[4], CRect src)
{
	if (!pTexture) {
		return E_POINTER;
	}

	ASSERT(pD3DDev);
	HRESULT hr;

	do {
		DDSURFACEDESC2 ddsd;
		INITDDSTRUCT(ddsd);
		if (FAILED(hr = pTexture->GetSurfaceDesc(&ddsd))) {
			break;
		}

		float w = (float)ddsd.dwWidth;
		float h = (float)ddsd.dwHeight;

		struct {
			float x, y, z, rhw;
			float tu, tv;
		}
		pVertices[] = {
			{(float)dst[0].x, (float)dst[0].y, (float)dst[0].z, 1.0f/(float)dst[0].z, (float)src.left / w, (float)src.top / h},
			{(float)dst[1].x, (float)dst[1].y, (float)dst[1].z, 1.0f/(float)dst[1].z, (float)src.right / w, (float)src.top / h},
			{(float)dst[2].x, (float)dst[2].y, (float)dst[2].z, 1.0f/(float)dst[2].z, (float)src.left / w, (float)src.bottom / h},
			{(float)dst[3].x, (float)dst[3].y, (float)dst[3].z, 1.0f/(float)dst[3].z, (float)src.right / w, (float)src.bottom / h},
		};

		for (int i = 0; i < countof(pVertices); i++) {
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

		if (FAILED(hr = pD3DDev->BeginScene())) {
			break;
		}

		hr = pD3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP,
									D3DFVF_XYZRHW | D3DFVF_TEX1,
									pVertices, 4, D3DDP_WAIT);
		pD3DDev->EndScene();

		//

		pD3DDev->SetTexture(0, NULL);

		return S_OK;
	} while (0);

	return E_FAIL;
}

//
// CDX7AllocatorPresenter
//

typedef HRESULT (WINAPI *DirectDrawCreateExPtr)( GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );


CDX7AllocatorPresenter::CDX7AllocatorPresenter(HWND hWnd, HRESULT& hr)
	: CSubPicAllocatorPresenterImpl(hWnd, hr, NULL)
	, m_ScreenSize(0, 0)
{
	if (FAILED(hr)) {
		return;
	}

	DirectDrawCreateExPtr	pDirectDrawCreateEx	= NULL;
	HMODULE					hDDrawLib			= NULL;

	hDDrawLib	= LoadLibrary (_T("ddraw.dll"));
	if (hDDrawLib) {
		pDirectDrawCreateEx = (DirectDrawCreateExPtr)GetProcAddress (hDDrawLib, "DirectDrawCreateEx");
	}
	if (pDirectDrawCreateEx == NULL) {
		hr = E_FAIL;
		return;
	}

	if (FAILED(hr = pDirectDrawCreateEx(NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL))
			|| FAILED(hr = m_pDD->SetCooperativeLevel(AfxGetMainWnd()->GetSafeHwnd(), DDSCL_NORMAL))) {
		return;
	}

	if (!(m_pD3D = m_pDD)) {
		hr = E_NOINTERFACE;
		return;
	}

	hr = CreateDevice();
	if (FAILED(hr)) {
		TRACE("CreateDevice failed: 0x%08x\n", (LONG)hr);
	}
}

HRESULT CDX7AllocatorPresenter::CreateDevice()
{
	m_pD3DDev = NULL;

	m_pPrimary = NULL;
	m_pBackBuffer = NULL;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	if (FAILED(m_pDD->GetDisplayMode(&ddsd)) ||
			ddsd.ddpfPixelFormat.dwRGBBitCount <= 8) {
		return DDERR_INVALIDMODE;
	}

	m_ScreenSize.SetSize(ddsd.dwWidth, ddsd.dwHeight);

	HRESULT hr;

	// m_pPrimary

	INITDDSTRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pPrimary, NULL))) {
		return hr;
	}

	CComPtr<IDirectDrawClipper> pcClipper;
	if (FAILED(hr = m_pDD->CreateClipper(0, &pcClipper, NULL))) {
		return hr;
	}
	pcClipper->SetHWnd(0, m_hWnd);
	m_pPrimary->SetClipper(pcClipper);

	// m_pBackBuffer

	INITDDSTRUCT(ddsd);
	ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = /*DDSCAPS_OFFSCREENPLAIN |*/ DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
	ddsd.dwWidth = m_ScreenSize.cx;
	ddsd.dwHeight = m_ScreenSize.cy;
	if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pBackBuffer, NULL))) {
		return hr;
	}

	pcClipper = NULL;
	if (FAILED(hr = m_pDD->CreateClipper(0, &pcClipper, NULL))) {
		return hr;
	}
	BYTE rgnDataBuffer[1024];
	HRGN hrgn = CreateRectRgn(0, 0, ddsd.dwWidth, ddsd.dwHeight);
	GetRegionData(hrgn, sizeof(rgnDataBuffer), (RGNDATA*)rgnDataBuffer);
	DeleteObject(hrgn);
	pcClipper->SetClipList((RGNDATA*)rgnDataBuffer, 0);
	m_pBackBuffer->SetClipper(pcClipper);

	// m_pD3DDev

	if (FAILED(hr = m_pD3D->CreateDevice(IID_IDirect3DHALDevice, m_pBackBuffer, &m_pD3DDev))) { // this seems to fail if the desktop size is too large (width or height >2048)
		return hr;
	}

	//

	CComPtr<ISubPicProvider> pSubPicProvider;
	if (m_pSubPicQueue) {
		m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);
	}

	CSize size;
	switch (GetRenderersSettings().nSPCMaxRes) {
		case 0:
		default:
			size = m_ScreenSize;
			break;
		case 1:
			size.SetSize(1024, 768);
			break;
		case 2:
			size.SetSize(800, 600);
			break;
		case 3:
			size.SetSize(640, 480);
			break;
		case 4:
			size.SetSize(512, 384);
			break;
		case 5:
			size.SetSize(384, 288);
			break;
		case 6:
			size.SetSize(2560, 1600);
			break;
		case 7:
			size.SetSize(1920, 1080);
			break;
		case 8:
			size.SetSize(1320, 900);
			break;
		case 9:
			size.SetSize(1280, 720);
			break;
	}

	if (m_pAllocator) {
		m_pAllocator->ChangeDevice(m_pD3DDev);
	} else {
		m_pAllocator = DNew CDX7SubPicAllocator(m_pD3DDev, size, GetRenderersSettings().fSPCPow2Tex);
		if (!m_pAllocator || FAILED(hr)) {
			return E_FAIL;
		}
	}

	hr = S_OK;
	m_pSubPicQueue = GetRenderersSettings().nSPCSize > 0
					 ? (ISubPicQueue*)DNew CSubPicQueue(GetRenderersSettings().nSPCSize, !GetRenderersSettings().fSPCAllowAnimationWhenBuffering, m_pAllocator, &hr)
					 : (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);
	if (!m_pSubPicQueue || FAILED(hr)) {
		return E_FAIL;
	}

	if (pSubPicProvider) {
		m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);
	}

	return S_OK;
}

HRESULT CDX7AllocatorPresenter::AllocSurfaces()
{
	CAutoLock cAutoLock(this);

	CRenderersSettings& s = GetRenderersSettings();

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

	if (s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D) {
		ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
		//		ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		//		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= 0xFF000000;
	}

	HRESULT hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurface, NULL);
	if (FAILED(hr)) {
		// FIXME: eh, dx9 has no problem creating a 32bpp surface under a 16bpp desktop, but dx7 fails here (textures are ok)
		DDSURFACEDESC2 ddsd2;
		INITDDSTRUCT(ddsd2);
		if (!(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
				&& SUCCEEDED(m_pDD->GetDisplayMode(&ddsd2))
				&& ddsd2.ddpfPixelFormat.dwRGBBitCount == 16) {
			ddsd.ddpfPixelFormat.dwRGBBitCount	= 16;
			ddsd.ddpfPixelFormat.dwRBitMask		= 0x0000F800;
			ddsd.ddpfPixelFormat.dwGBitMask		= 0x000007E0;
			ddsd.ddpfPixelFormat.dwBBitMask		= 0x0000001F;
			hr = m_pDD->CreateSurface(&ddsd, &m_pVideoSurface, NULL);
		}

		if (FAILED(hr)) {
			return hr;
		}
	}

	if (s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D) {
		m_pVideoTexture = m_pVideoSurface;
	}

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
	if (m_bPendingResetDevice) {
		SendResetRequest();
		return false;
	}

	CAutoLock cAutoLock(this);

	if (m_WindowRect.right <= m_WindowRect.left || m_WindowRect.bottom <= m_WindowRect.top
			|| m_NativeVideoSize.cx <= 0 || m_NativeVideoSize.cy <= 0
			|| !m_pPrimary || !m_pBackBuffer || !m_pVideoSurface) {
		return false;
	}

	HRESULT hr;

	CRect rSrcVid(CPoint(0, 0), m_NativeVideoSize);
	CRect rDstVid(m_VideoRect);

	CRect rSrcPri(CPoint(0, 0), m_WindowRect.Size());
	CRect rDstPri(m_WindowRect);
	MapWindowRect(m_hWnd, HWND_DESKTOP, &rDstPri);

	if (fAll) {
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

		if (!rDstVid.IsRectEmpty()) {
			if (m_pVideoTexture) {
				Vector v[4];
				Transform(rDstVid, v);
				hr = TextureBlt(m_pD3DDev, m_pVideoTexture, v, rSrcVid);
			} else {
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

	if (hr == DDERR_SURFACELOST) {
		m_bPendingResetDevice = true;
		SendResetRequest();
		return false;
	}

	return true;
}

void CDX7AllocatorPresenter::SendResetRequest()
{
	if (!m_bDeviceResetRequested) {
		m_bDeviceResetRequested = true;
		AfxGetApp()->m_pMainWnd->PostMessage(WM_RESET_DEVICE);
	}
}

STDMETHODIMP_(bool) CDX7AllocatorPresenter::ResetDevice()
{
	HRESULT hr;
	DeleteSurfaces();
	if (FAILED(hr = CreateDevice()) || FAILED(hr = AllocSurfaces())) {
		//DDERR_UNSUPPORTEDMODE - 0x8876024e
		TRACE("ResetDevice failed: 0x%08x\n", (LONG)hr);
		m_bDeviceResetRequested = false;
		return false;
	}
	TRACE("ResetDevice\n");
	m_bPendingResetDevice = false;
	m_bDeviceResetRequested = false;
	return true;
}

STDMETHODIMP_(bool) CDX7AllocatorPresenter::DisplayChange()
{
	return true;
}

STDMETHODIMP CDX7AllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	CheckPointer(size, E_POINTER);

	HRESULT hr;

	DDSURFACEDESC2 ddsd;
	INITDDSTRUCT(ddsd);
	if (FAILED(m_pVideoSurface->GetSurfaceDesc(&ddsd))) {
		return E_FAIL;
	}

	if (ddsd.ddpfPixelFormat.dwRGBBitCount != 16 && ddsd.ddpfPixelFormat.dwRGBBitCount != 32) {
		return E_FAIL;
	}

	DWORD required = sizeof(BITMAPINFOHEADER) + (ddsd.dwWidth*ddsd.dwHeight*32>>3);
	if (!lpDib) {
		*size = required;
		return S_OK;
	}
	if (*size < required) {
		return E_OUTOFMEMORY;
	}
	*size = required;

	INITDDSTRUCT(ddsd);
	if (FAILED(hr = m_pVideoSurface->Lock(NULL, &ddsd, DDLOCK_WAIT|DDLOCK_SURFACEMEMORYPTR|DDLOCK_READONLY|DDLOCK_NOSYSLOCK, NULL))) {
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
