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

namespace DSObjects
{

	class CDX9AllocatorPresenter
		: public ISubPicAllocatorPresenterImpl
	{
	protected:
		CSize m_ScreenSize;
		bool m_fVMRSyncFix;

		CComPtr<IDirect3D9> m_pD3D;
		CComPtr<IDirect3DDevice9> m_pD3DDev;
		CComPtr<IDirect3DTexture9> m_pVideoTexture[4];
		CComPtr<IDirect3DSurface9> m_pVideoSurface[4];
		int						   m_nCurPicture;
		CComPtr<IDirect3DTexture9> m_pOSDTexture;
		CComPtr<IDirect3DSurface9> m_pOSDSurface;
		CInterfaceList<IDirect3DPixelShader9> m_pPixelShaders;
		CComPtr<IDirect3DPixelShader9> m_pResizerPixelShader[3]; // bl, bc1, bc2
		CComPtr<IDirect3DTexture9> m_pResizerBicubic1stPass;
		D3DTEXTUREFILTERTYPE m_filter;
		D3DCAPS9 m_caps;

		CAutoPtr<CPixelShaderCompiler> m_pPSC;

		virtual HRESULT CreateDevice();
		virtual HRESULT AllocSurfaces();
		virtual void DeleteSurfaces();

		UINT GetAdapter(IDirect3D9 *pD3D);

		float m_bicubicA;
		HRESULT InitResizers(float bicubicA);

		HRESULT TextureCopy(CComPtr<IDirect3DTexture9> pTexture);
		HRESULT TextureResize(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter);
		HRESULT TextureResizeBilinear(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4]);
		HRESULT TextureResizeBicubic1pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4]);
		HRESULT TextureResizeBicubic2pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4]);

		// Casimir666
		typedef HRESULT (WINAPI * D3DXLoadSurfaceFromMemoryPtr)(
				LPDIRECT3DSURFACE9        pDestSurface,
				CONST PALETTEENTRY*       pDestPalette,
				CONST RECT*               pDestRect,
				LPCVOID                   pSrcMemory,
				D3DFORMAT                 SrcFormat,
				UINT                      SrcPitch,
				CONST PALETTEENTRY*       pSrcPalette,
				CONST RECT*               pSrcRect,
				DWORD                     Filter,
				D3DCOLOR                  ColorKey);

		int					m_nTearingPos;
		VMR9AlphaBitmap		m_VMR9AlphaBitmap;
		HRESULT				AlphaBlt(RECT* pSrc, RECT* pDst, CComPtr<IDirect3DTexture9> pTexture);
		HINSTANCE			m_hDll;
		D3DXLoadSurfaceFromMemoryPtr	m_pD3DXLoadSurfaceFromMemory;

	public:
		CDX9AllocatorPresenter(HWND hWnd, HRESULT& hr);
		~CDX9AllocatorPresenter();

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(bool) Paint(bool fAll);
		STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size);
		STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget);
	};

	class CVMR9AllocatorPresenter
		: public CDX9AllocatorPresenter
		, public IVMRSurfaceAllocator9
		, public IVMRImagePresenter9
		, public IVMRWindowlessControl9
	{
	protected:
		CComPtr<IVMRSurfaceAllocatorNotify9> m_pIVMRSurfAllocNotify;
		CInterfaceArray<IDirect3DSurface9> m_pSurfaces;

		HRESULT CreateDevice();
		void DeleteSurfaces();

		bool m_fUseInternalTimer;
		REFERENCE_TIME m_rtPrevStart;

	public:
		CVMR9AllocatorPresenter(HWND hWnd, HRESULT& hr);

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);
		
		// IVMRSurfaceAllocator9
		STDMETHODIMP InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers);
		STDMETHODIMP TerminateDevice(DWORD_PTR dwID);
		STDMETHODIMP GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface);
		STDMETHODIMP AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify);

		// IVMRImagePresenter9
		STDMETHODIMP StartPresenting(DWORD_PTR dwUserID);
		STDMETHODIMP StopPresenting(DWORD_PTR dwUserID);
		STDMETHODIMP PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo);

		// IVMRWindowlessControl9
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
	};

	class CRM9AllocatorPresenter
		: public CDX9AllocatorPresenter
		, public IRMAVideoSurface
	{
		CComPtr<IDirect3DSurface9> m_pVideoSurfaceOff;
		CComPtr<IDirect3DSurface9> m_pVideoSurfaceYUY2;

		RMABitmapInfoHeader m_bitmapInfo;
		RMABitmapInfoHeader m_lastBitmapInfo;

	protected:
		HRESULT AllocSurfaces();
		void DeleteSurfaces();

	public:
		CRM9AllocatorPresenter(HWND hWnd, HRESULT& hr);

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

	class CQT9AllocatorPresenter
		: public CDX9AllocatorPresenter
		, public IQTVideoSurface
	{
		CComPtr<IDirect3DSurface9> m_pVideoSurfaceOff;

	protected:
		 HRESULT AllocSurfaces();
		 void DeleteSurfaces();

	public:
		CQT9AllocatorPresenter(HWND hWnd, HRESULT& hr);

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		// IQTVideoSurface
		STDMETHODIMP BeginBlt(const BITMAP& bm);
		STDMETHODIMP DoBlt(const BITMAP& bm);
	};

	class CDXRAllocatorPresenter
		: public ISubPicAllocatorPresenterImpl
	{
		class CSubRenderCallback : public CUnknown, public ISubRenderCallback, public CCritSec
		{
			CDXRAllocatorPresenter* m_pDXRAP;

		public:
			CSubRenderCallback(CDXRAllocatorPresenter* pDXRAP)
				: CUnknown(_T("CSubRender"), NULL)
				, m_pDXRAP(pDXRAP)
			{
			}

			DECLARE_IUNKNOWN
			STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
			{
				return 
					QI(ISubRenderCallback)
					__super::NonDelegatingQueryInterface(riid, ppv);
			}

			void SetDXRAP(CDXRAllocatorPresenter* pDXRAP)
			{
				CAutoLock cAutoLock(this);
				m_pDXRAP = pDXRAP;
			}

			// ISubRenderCallback

			STDMETHODIMP SetDevice(IDirect3DDevice9* pD3DDev)
			{
				CAutoLock cAutoLock(this);
				return m_pDXRAP ? m_pDXRAP->SetDevice(pD3DDev) : E_UNEXPECTED;
			}

			STDMETHODIMP Render(REFERENCE_TIME rtStart, int left, int top, int right, int bottom, int width, int height)
			{
				CAutoLock cAutoLock(this);
				return m_pDXRAP ? m_pDXRAP->Render(rtStart, 0, 0, left, top, right, bottom, width, height) : E_UNEXPECTED;
			}

			// ISubRendererCallback2

			STDMETHODIMP RenderEx(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME AvgTimePerFrame, int left, int top, int right, int bottom, int width, int height)
			{
				CAutoLock cAutoLock(this);
				return m_pDXRAP ? m_pDXRAP->Render(rtStart, rtStop, AvgTimePerFrame, left, top, right, bottom, width, height) : E_UNEXPECTED;
			}
		};

		CComPtr<IUnknown> m_pDXR;
		CComPtr<ISubRenderCallback> m_pSRCB;

	public:
		CDXRAllocatorPresenter(HWND hWnd, HRESULT& hr);
		virtual ~CDXRAllocatorPresenter();

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		HRESULT SetDevice(IDirect3DDevice9* pD3DDev);
		HRESULT Render(
			REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME atpf,
			int left, int top, int bottom, int right, int width, int height);

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(void) SetPosition(RECT w, RECT v);
		STDMETHODIMP_(SIZE) GetVideoSize(bool fCorrectAR);
		STDMETHODIMP_(bool) Paint(bool fAll);
		STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size);
		STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget);
	};

}
