/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2007 see AUTHORS
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

#pragma once

#define VMRBITMAP_UPDATE            0x80000000
#define MAX_PICTURE_SLOTS			(60+2)				// Last 2 for pixels shader!

#define NB_JITTER					126

extern bool g_bNoDuration;
extern bool g_bExternalSubtitleTime;

namespace DSObjects
{

	class CDX9AllocatorPresenter
		: public ISubPicAllocatorPresenterImpl
	{
	public:
		CCritSec				m_VMR9AlphaBitmapLock;
		void					UpdateAlphaBitmap();
	protected:
		CSize	m_ScreenSize;
		UINT	m_RefreshRate;

//		bool	m_fVMRSyncFix;
		bool	m_bAlternativeVSync;
		bool	m_bHighColorResolution;
		bool	m_bCompositionEnabled;
		bool	m_bIsEVR;
		int		m_OrderedPaint;
		int		m_VSyncMode;
		bool	m_bDesktopCompositionDisabled;
		bool	m_bIsFullscreen;
		bool	m_bNeedCheckSample;
		DWORD	m_MainThreadId;

		CMPlayerCApp::Settings::CRendererSettingsEVR m_LastRendererSettings;

		HRESULT (__stdcall * m_pDwmIsCompositionEnabled)(__out BOOL* pfEnabled);
		HRESULT (__stdcall * m_pDwmEnableComposition)(UINT uCompositionAction);

		HMODULE m_hDWMAPI;

		HRESULT (__stdcall * m_pDirect3DCreate9Ex)(UINT SDKVersion, IDirect3D9Ex**);
		HMODULE m_hD3D9;

		CCritSec					m_RenderLock;
		CComPtr<IDirectDraw>		m_pDirectDraw;

		CComPtr<IDirect3D9Ex>			m_pD3DEx;
		CComPtr<IDirect3D9>			m_pD3D;
		CComPtr<IDirect3DDevice9Ex>		m_pD3DDevEx;

		void LockD3DDevice()
		{
			if (m_pD3DDev)
			{
				_RTL_CRITICAL_SECTION *pCritSec = (_RTL_CRITICAL_SECTION *)((size_t)m_pD3DDev.p + sizeof(size_t));

				if (!IsBadReadPtr(pCritSec, sizeof(*pCritSec)) && !IsBadWritePtr(pCritSec, sizeof(*pCritSec))
					&& !IsBadReadPtr(pCritSec->DebugInfo, sizeof(*(pCritSec->DebugInfo))) && !IsBadWritePtr(pCritSec->DebugInfo, sizeof(*(pCritSec->DebugInfo))))			
				{
					if (pCritSec->DebugInfo->CriticalSection == pCritSec)
						EnterCriticalSection(pCritSec);
				}
			}
		}

		void UnlockD3DDevice()
		{
			if (m_pD3DDev)
			{
				_RTL_CRITICAL_SECTION *pCritSec = (_RTL_CRITICAL_SECTION *)((size_t)m_pD3DDev.p + sizeof(size_t));

				if (!IsBadReadPtr(pCritSec, sizeof(*pCritSec)) && !IsBadWritePtr(pCritSec, sizeof(*pCritSec))
					&& !IsBadReadPtr(pCritSec->DebugInfo, sizeof(*(pCritSec->DebugInfo))) && !IsBadWritePtr(pCritSec->DebugInfo, sizeof(*(pCritSec->DebugInfo))))			
				{
					if (pCritSec->DebugInfo->CriticalSection == pCritSec)
						LeaveCriticalSection(pCritSec);
				}
			}
		}
		CString m_D3DDevExError;
		CComPtr<IDirect3DDevice9>		m_pD3DDev;
		CComPtr<IDirect3DTexture9>		m_pVideoTexture[MAX_PICTURE_SLOTS];
		CComPtr<IDirect3DSurface9>		m_pVideoSurface[MAX_PICTURE_SLOTS];
		CComPtr<IDirect3DTexture9>		m_pOSDTexture;
		CComPtr<IDirect3DSurface9>		m_pOSDSurface;
		CComPtr<ID3DXLine>			m_pLine;
		CComPtr<ID3DXFont>			m_pFont;
		CComPtr<ID3DXSprite>		m_pSprite;
		class CExternalPixelShader
		{
		public:
			CComPtr<IDirect3DPixelShader9> m_pPixelShader;
			CStringA m_SourceData;
			CStringA m_SourceTarget;
			HRESULT Compile(CPixelShaderCompiler *pCompiler)
			{
				HRESULT hr = pCompiler->CompileShader(m_SourceData, "main", m_SourceTarget, 0, &m_pPixelShader);
				if(FAILED(hr)) 
					return hr;

				return S_OK;
			}
		};
		CAtlList<CExternalPixelShader>	m_pPixelShaders;
		CAtlList<CExternalPixelShader>	m_pPixelShadersScreenSpace;
		CComPtr<IDirect3DPixelShader9>		m_pResizerPixelShader[3]; // bl, bc1, bc2
		CComPtr<IDirect3DTexture9>		m_pScreenSizeTemporaryTexture[2];
		D3DFORMAT						m_SurfaceType;
		D3DFORMAT						m_BackbufferType;
		D3DFORMAT						m_DisplayType;
		D3DTEXTUREFILTERTYPE			m_filter;
		D3DCAPS9				m_caps;

		CAutoPtr<CPixelShaderCompiler>		m_pPSC;

		bool SettingsNeedResetDevice();

		virtual HRESULT CreateDevice(CString &_Error);
//		virtual HRESULT AllocSurfaces(D3DFORMAT Format = D3DFMT_A2B10G10R10);
		virtual HRESULT AllocSurfaces(D3DFORMAT Format = D3DFMT_A8R8G8B8);
		virtual void DeleteSurfaces();

		// Thread stuff
		HANDLE			m_hEvtQuit;			// Stop rendering thread event
		HANDLE			m_hVSyncThread;
		static DWORD WINAPI VSyncThreadStatic(LPVOID lpParam);
		void VSyncThread();
		void StartWorkerThreads();
		void StopWorkerThreads();

		UINT GetAdapter(IDirect3D9 *pD3D);

		float m_bicubicA;
		HRESULT InitResizers(float bicubicA, bool bNeedScreenSizeTexture);

		bool GetVBlank(int &_ScanLine, int &_bInVBlank, bool _bMeasureTime);
		bool WaitForVBlankRange(int &_RasterStart, int _RasterEnd, bool _bWaitIfInside, bool _bNeedAccurate, bool _bMeasure, bool &_bTakenLock);
		bool WaitForVBlank(bool &_Waited, bool &_bTakenLock);
		int GetVBlackPos();
		void CalculateJitter(LONGLONG PerformanceCounter);
		virtual void OnVBlankFinished(bool fAll, LONGLONG PerformanceCounter){}

		HRESULT DrawRect(DWORD _Color, DWORD _Alpha, const CRect &_Rect);
		HRESULT TextureCopy(CComPtr<IDirect3DTexture9> pTexture);
		HRESULT TextureResize(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter, const CRect &SrcRect);
		HRESULT TextureResizeBilinear(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect);
		HRESULT TextureResizeBicubic1pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect);
		HRESULT TextureResizeBicubic2pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect);

		// Casimir666
		typedef HRESULT (WINAPI * D3DXLoadSurfaceFromMemoryPtr)(
				LPDIRECT3DSURFACE9	pDestSurface,
				CONST PALETTEENTRY*	pDestPalette,
				CONST RECT*		pDestRect,
				LPCVOID			pSrcMemory,
				D3DFORMAT		SrcFormat,
				UINT			SrcPitch,
				CONST PALETTEENTRY*	pSrcPalette,
				CONST RECT*		pSrcRect,
				DWORD			Filter,
				D3DCOLOR		ColorKey);

		typedef HRESULT (WINAPI* D3DXCreateLinePtr) (LPDIRECT3DDEVICE9   pDevice, LPD3DXLINE* ppLine);

		typedef HRESULT (WINAPI* D3DXCreateFontPtr)(
										LPDIRECT3DDEVICE9	pDevice,  
										int			Height,
										UINT			Width,
										UINT			Weight,
										UINT			MipLevels,
										bool			Italic,
										DWORD			CharSet,
										DWORD			OutputPrecision,
										DWORD			Quality,
										DWORD			PitchAndFamily,
										LPCWSTR			pFaceName,
										LPD3DXFONT*		ppFont);


		void				DrawText(const RECT &rc, const CString &strText, int _Priority);
		void				DrawStats();
		HRESULT				AlphaBlt(RECT* pSrc, RECT* pDst, CComPtr<IDirect3DTexture9> pTexture);
		virtual void		OnResetDevice() {};
		virtual bool		ResetDevice();

		double GetFrameTime();
		double GetFrameRate();


		int						m_nTearingPos;
		VMR9AlphaBitmap			m_VMR9AlphaBitmap;
		CAutoVectorPtr<BYTE>	m_VMR9AlphaBitmapData;
		CRect					m_VMR9AlphaBitmapRect;
		int						m_VMR9AlphaBitmapWidthBytes;

		D3DXLoadSurfaceFromMemoryPtr	m_pD3DXLoadSurfaceFromMemory;
		D3DXCreateLinePtr		m_pD3DXCreateLine;
		D3DXCreateFontPtr		m_pD3DXCreateFont;
		HRESULT (__stdcall *m_pD3DXCreateSprite)(LPDIRECT3DDEVICE9 pDevice, LPD3DXSPRITE * ppSprite);



		int						m_nNbDXSurface;					// Total number of DX Surfaces
		int						m_nVMR9Surfaces;					// Total number of DX Surfaces
		int						m_iVMR9Surface;
		int						m_nCurSurface;					// Surface currently displayed
		long					m_nUsedBuffer;
		bool					m_bNeedPendingResetDevice;
		bool					m_bPendingResetDevice;

		double					m_fAvrFps;						// Estimate the real FPS
		double					m_fJitterStdDev;				// Estimate the Jitter std dev
		double					m_fJitterMean;
		double					m_fSyncOffsetStdDev;
		double					m_fSyncOffsetAvr;
		double					m_DetectedRefreshRate;

		CCritSec				m_RefreshRateLock;
		double					m_DetectedRefreshTime;
		double					m_DetectedRefreshTimePrim;
		double					m_DetectedScanlineTime;
		double					m_DetectedScanlineTimePrim;
		double					m_DetectedScanlinesPerFrame;

		double GetRefreshRate()
		{
			if (m_DetectedRefreshRate)
				return m_DetectedRefreshRate;
			return m_RefreshRate;
		}

		LONG GetScanLines()
		{
			if (m_DetectedRefreshRate)
				return m_DetectedScanlinesPerFrame;
			return m_ScreenSize.cy;
		}

		double					m_ldDetectedRefreshRateList[100];
		double					m_ldDetectedScanlineRateList[100];
		int						m_DetectedRefreshRatePos;
		bool					m_bSyncStatsAvailable;						
		LONGLONG				m_pllJitter [NB_JITTER];		// Jitter buffer for stats
		LONGLONG				m_pllSyncOffset [NB_JITTER];		// Jitter buffer for stats
		LONGLONG				m_llLastPerf;
		LONGLONG				m_JitterStdDev;
		LONGLONG				m_MaxJitter;
		LONGLONG				m_MinJitter;
		LONGLONG				m_MaxSyncOffset;
		LONGLONG				m_MinSyncOffset;
		int						m_nNextJitter;
		int						m_nNextSyncOffset;
		REFERENCE_TIME			m_rtTimePerFrame;
		double					m_DetectedFrameRate;
		double					m_DetectedFrameTime;
		double					m_DetectedFrameTimeStdDev;
		bool					m_DetectedLock;
		LONGLONG				m_DetectedFrameTimeHistory[60];
		double					m_DetectedFrameTimeHistoryHisotry[500];
		int						m_DetectedFrameTimePos;
		int						m_bInterlaced;

		double					m_TextScale;

		int						m_VBlankEndWait;
		int						m_VBlankStartWait;
		LONGLONG				m_VBlankWaitTime;
		LONGLONG				m_VBlankLockTime;
		int						m_VBlankMin;
		int						m_VBlankMinCalc;
		int						m_VBlankMax;
		int						m_VBlankEndPresent;
		LONGLONG				m_VBlankStartMeasureTime;
		int						m_VBlankStartMeasure;

		LONGLONG				m_PresentWaitTime;
		LONGLONG				m_PresentWaitTimeMin;
		LONGLONG				m_PresentWaitTimeMax;

		LONGLONG				m_PaintTime;
		LONGLONG				m_PaintTimeMin;
		LONGLONG				m_PaintTimeMax;

		LONGLONG				m_WaitForGPUTime;

		LONGLONG				m_RasterStatusWaitTime;
		LONGLONG				m_RasterStatusWaitTimeMin;
		LONGLONG				m_RasterStatusWaitTimeMax;
		LONGLONG				m_RasterStatusWaitTimeMaxCalc;

		double					m_ClockDiffCalc;
		double					m_ClockDiffPrim;
		double					m_ClockDiff;

		double					m_TimeChangeHisotry[100];
		double					m_ClockChangeHisotry[100];
		int						m_ClockTimeChangeHistoryPos;
		double					m_ModeratedTimeSpeed;
		double					m_ModeratedTimeSpeedPrim;
		double					m_ModeratedTimeSpeedDiff;

		bool					m_bCorrectedFrameTime;
		int						m_FrameTimeCorrection;
		LONGLONG				m_LastFrameDuration;
		LONGLONG				m_LastSampleTime;

		CString					m_strStatsMsg[10];

	public:
		CDX9AllocatorPresenter(HWND hWnd, HRESULT& hr, bool bIsEVR, CString &_Error);
		~CDX9AllocatorPresenter();

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(bool) Paint(bool fAll);
		STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size);
		STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget);
		STDMETHODIMP SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace);
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

		HRESULT CreateDevice(CString &_Error);
		void DeleteSurfaces();

		bool m_fUseInternalTimer;
		REFERENCE_TIME m_rtPrevStart;

	public:
		CVMR9AllocatorPresenter(HWND hWnd, HRESULT& hr, CString &_Error);

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
		CRM9AllocatorPresenter(HWND hWnd, HRESULT& hr, CString &_Error);

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
		CQT9AllocatorPresenter(HWND hWnd, HRESULT& hr, CString &_Error);

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
		CSize	m_ScreenSize;

	public:
		CDXRAllocatorPresenter(HWND hWnd, HRESULT& hr, CString &_Error);
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
