/*
* $Id: SyncRenderer.h 1292 2009-10-03 23:20:26Z ar-jar $
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
#include "..\..\SubPic\ISubPic.h"
#include "SyncAllocatorPresenter.h"

#define VMRBITMAP_UPDATE 0x80000000
#define MAX_PICTURE_SLOTS (60+2) // Last 2 for pixels shader!
#define NB_JITTER 126

extern bool g_bNoDuration; // Defined in MainFrm.cpp
extern bool g_bExternalSubtitleTime;

// Possible messages to the PowerStrip API. PowerStrip is used to control
// the display frequency in one of the video - display synchronization modes.
// Powerstrip can also through a CGenlock object give very accurate timing data
// (given) that the gfx board is supported by PS.
#define UM_SETCUSTOMTIMING (WM_USER+200) 
#define UM_SETREFRESHRATE (WM_USER+201) 
#define UM_SETPOLARITY (WM_USER+202) 
#define UM_REMOTECONTROL (WM_USER+210) 
#define UM_SETGAMMARAMP (WM_USER+203) 
#define UM_CREATERESOLUTION (WM_USER+204)
#define UM_GETTIMING (WM_USER+205)
#define UM_SETCUSTOMTIMINGFAST (WM_USER+211) // Sets timing without writing to file. Faster

#define PositiveHorizontalPolarity 0x00 
#define PositiveVerticalPolarity 0x00 
#define NegativeHorizontalPolarity 0x02 
#define NegativeVerticalPolarity 0x04 
#define HideTrayIcon 0x00 
#define ShowTrayIcon 0x01 
#define ClosePowerStrip 0x63

#define HACTIVE 0
#define HFRONTPORCH 1
#define HSYNCWIDTH 2
#define HBACKPORCH 3
#define VACTIVE 4
#define VFRONTPORCH 5
#define VSYNCWIDTH 6
#define VBACKPORCH 7
#define PIXELCLOCK 8
#define UNKNOWN 9

#define MAX_FIFO_SIZE 1024

// Manages synchronization of video and display.
namespace GothSync
{
	class CGenlock;

	// Base allocator-presenter
	class CBaseAP: public ISubPicAllocatorPresenterImpl
	{
	protected:
		CMPlayerCApp::Settings::CRendererSettingsEVR m_LastRendererSettings;

		HRESULT (__stdcall * m_pDwmIsCompositionEnabled)(__out BOOL* pfEnabled);
		HRESULT (__stdcall * m_pDwmEnableComposition)(UINT uCompositionAction);
		HMODULE m_hDWMAPI;
		HRESULT (__stdcall * m_pDirect3DCreate9Ex)(UINT SDKVersion, IDirect3D9Ex**);
		HMODULE m_hD3D9;

		CCritSec m_allocatorLock;
		CComPtr<IDirect3D9Ex> m_pD3DEx;
		CComPtr<IDirect3D9> m_pD3D;
		CComPtr<IDirect3DDevice9Ex> m_pD3DDevEx;
		CComPtr<IDirect3DDevice9> m_pD3DDev;
		CString m_D3DDevExError;

		CComPtr<IDirect3DTexture9> m_pVideoTexture[MAX_PICTURE_SLOTS];
		CComPtr<IDirect3DSurface9> m_pVideoSurface[MAX_PICTURE_SLOTS];
		CComPtr<IDirect3DTexture9> m_pOSDTexture;
		CComPtr<IDirect3DSurface9> m_pOSDSurface;
		CComPtr<ID3DXLine> m_pLine;
		CComPtr<ID3DXFont> m_pFont;
		CComPtr<ID3DXSprite> m_pSprite;

		class CExternalPixelShader
		{
		public:
			CComPtr<IDirect3DPixelShader9> m_pPixelShader;
			CStringA m_SourceData;
			CStringA m_SourceTarget;
			HRESULT Compile(CPixelShaderCompiler *pCompiler)
			{
				HRESULT hr = pCompiler->CompileShader(m_SourceData, "main", m_SourceTarget, 0, &m_pPixelShader);
				if(FAILED(hr)) return hr;
				return S_OK;
			}
		};

		CAutoPtr<CPixelShaderCompiler> m_pPSC;
		CAtlList<CExternalPixelShader> m_pPixelShaders;
		CAtlList<CExternalPixelShader> m_pPixelShadersScreenSpace;
		CComPtr<IDirect3DPixelShader9> m_pResizerPixelShader[4]; // bl, bc1, bc2_1, bc2_2
		CComPtr<IDirect3DTexture9> m_pScreenSizeTemporaryTexture[2];

		D3DFORMAT m_SurfaceType;
		D3DFORMAT m_BackbufferType;
		D3DFORMAT m_DisplayType;
		D3DTEXTUREFILTERTYPE m_filter;
		D3DCAPS9 m_caps;

		bool SettingsNeedResetDevice();
		virtual HRESULT CreateDevice(CString &_Error);
		virtual HRESULT AllocSurfaces(D3DFORMAT Format = D3DFMT_A8R8G8B8);
		virtual void DeleteSurfaces();

		HANDLE m_hEvtQuit; // Stop rendering thread event
		UINT GetAdapter(IDirect3D9 *pD3D);

		float m_bicubicA;
		HRESULT InitResizers(float bicubicA, bool bNeedScreenSizeTexture);

		// Functions to trace timing performance
		void SyncStats(LONGLONG syncTime);
		void SyncOffsetStats(LONGLONG syncOffset);
		void DrawText(const RECT &rc, const CString &strText, int _Priority);
		void DrawStats();

		HRESULT DrawRect(DWORD _Color, DWORD _Alpha, const CRect &_Rect);
		HRESULT TextureCopy(CComPtr<IDirect3DTexture9> pTexture);
		HRESULT TextureResize(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], D3DTEXTUREFILTERTYPE filter, const CRect &SrcRect);
		HRESULT TextureResizeBilinear(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect);
		HRESULT TextureResizeBicubic1pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect);
		HRESULT TextureResizeBicubic2pass(CComPtr<IDirect3DTexture9> pTexture, Vector dst[4], const CRect &SrcRect);

		typedef HRESULT (WINAPI * D3DXLoadSurfaceFromMemoryPtr)(
			LPDIRECT3DSURFACE9 pDestSurface,
			CONST PALETTEENTRY* pDestPalette,
			CONST RECT* pDestRect,
			LPCVOID pSrcMemory,
			D3DFORMAT SrcFormat,
			UINT SrcPitch,
			CONST PALETTEENTRY*	pSrcPalette,
			CONST RECT* pSrcRect,
			DWORD Filter,
			D3DCOLOR ColorKey);

		typedef HRESULT (WINAPI* D3DXCreateLinePtr)
			(LPDIRECT3DDEVICE9 pDevice,
			LPD3DXLINE* ppLine);

		typedef HRESULT (WINAPI* D3DXCreateFontPtr)(
			LPDIRECT3DDEVICE9 pDevice,
			int Height,
			UINT Width,
			UINT Weight,
			UINT MipLevels,
			bool Italic,
			DWORD CharSet,
			DWORD OutputPrecision,
			DWORD Quality,
			DWORD PitchAndFamily,
			LPCWSTR pFaceName,
			LPD3DXFONT* ppFont);

		HRESULT AlphaBlt(RECT* pSrc, RECT* pDst, CComPtr<IDirect3DTexture9> pTexture);

		virtual void OnResetDevice() {};
		virtual bool ResetDevice();

		int m_nTearingPos;
		VMR9AlphaBitmap m_VMR9AlphaBitmap;
		CAutoVectorPtr<BYTE> m_VMR9AlphaBitmapData;
		CRect m_VMR9AlphaBitmapRect;
		int m_VMR9AlphaBitmapWidthBytes;

		D3DXLoadSurfaceFromMemoryPtr m_pD3DXLoadSurfaceFromMemory;
		D3DXCreateLinePtr m_pD3DXCreateLine;
		D3DXCreateFontPtr m_pD3DXCreateFont;
		HRESULT (__stdcall *m_pD3DXCreateSprite)(LPDIRECT3DDEVICE9 pDevice, LPD3DXSPRITE * ppSprite);

		int m_nDXSurface; // Total number of DX Surfaces
		int m_nVMR9Surfaces;
		int m_iVMR9Surface;
		int m_nCurSurface; // Surface currently displayed
		long m_nUsedBuffer;
		bool m_bNeedPendingResetDevice;
		bool m_bPendingResetDevice;

		LONG m_lNextSampleWait; // Waiting time for next sample in EVR
		bool m_bSnapToVSync; // True if framerate is low enough so that snap to vsync makes sense

		UINT m_uScanLineEnteringPaint; // The active scan line when entering Paint()
		REFERENCE_TIME m_llEstVBlankTime; // Next vblank start time in reference clock "coordinates"

		double m_fAvrFps; // Estimate the true FPS as given by the distance between vsyncs when a frame has been presented
		double m_fJitterStdDev; // VSync estimate std dev
		double m_fJitterMean; // Mean time between two syncpulses when a frame has been presented (i.e. when Paint() has been called

		double m_fSyncOffsetAvr; // Mean time between the call of Paint() and vsync. To avoid tearing this should be several ms at least
		double m_fSyncOffsetStdDev; // The std dev of the above

		bool m_bHighColorResolution;
		bool m_bCompositionEnabled;
		bool m_bDesktopCompositionDisabled;
		bool m_bIsFullscreen;
		bool m_bNeedCheckSample;
		DWORD m_dMainThreadId;

		CSize m_ScreenSize;

		// Display and frame rates and cycles
		double m_dDetectedScanlineTime; // Time for one (horizontal) scan line. Extracted at stream start and used to calculate vsync time
		UINT m_uD3DRefreshRate; // As got when creating the d3d device
		double m_dD3DRefreshCycle; // Display refresh cycle ms
		double m_dEstRefreshCycle; // As estimated from scan lines
		double m_dFrameCycle; // Average sample time, extracted from the samples themselves
		// double m_fps is defined in ISubPic.h
		double m_dOptimumDisplayCycle; // The display cycle that is closest to the frame rate. A multiple of the actual display cycle
		double m_dCycleDifference; // Difference in video and display cycle time relative to the video cycle time

		UINT m_pcFramesDropped;
		UINT m_pcFramesDuplicated;
		UINT m_pcFramesDrawn;

		LONGLONG m_pllJitter [NB_JITTER]; // Vertical sync time stats
		LONGLONG m_pllSyncOffset [NB_JITTER]; // Sync offset time stats
		int m_nNextJitter;
		int m_nNextSyncOffset;
		LONGLONG m_JitterStdDev;

		LONGLONG m_llLastSyncTime;

		LONGLONG m_MaxJitter;
		LONGLONG m_MinJitter;
		LONGLONG m_MaxSyncOffset;
		LONGLONG m_MinSyncOffset;
		UINT m_uSyncGlitches;

		LONGLONG m_llSampleTime, m_llLastSampleTime; // Present time for the current sample
		LONG m_lSampleLatency, m_lLastSampleLatency; // Time between intended and actual presentation time
		LONG m_lMinSampleLatency, m_lLastMinSampleLatency;
		LONGLONG m_llHysteresis;
		LONG m_lHysteresis;
		LONG m_lShiftToNearest, m_lShiftToNearestPrev;
		bool m_bVideoSlowerThanDisplay;

		int m_bInterlaced;
		double m_TextScale;
		CString	 m_strStatsMsg[10];

		CGenlock *m_pGenlock; // The video - display synchronizer class
		CComPtr<IReferenceClock> m_pRefClock; // The reference clock. Used in Paint()
		CComPtr<IAMAudioRendererStats> m_pAudioStats; // Audio statistics from audio renderer. To check so that audio is in sync
		DWORD m_lAudioLag; // Time difference between audio and video when the audio renderer is matching rate to the external reference clock
		long m_lAudioLagMin, m_lAudioLagMax; // The accumulated difference between the audio renderer and the master clock
		DWORD m_lAudioSlaveMode; // To check whether the audio renderer matches rate with SyncClock (returns the value 4 if it does)

		double GetRefreshRate(); // Get the best estimate of the display refresh rate in Hz
		double GetDisplayCycle(); // Get the best estimate of the display cycle time in milliseconds
		double GetCycleDifference(); // Get the difference in video and display cycle times.
		void EstimateRefreshTimings(); // Estimate the times for one scan line and one frame respectively from the actual refresh data
		bool ExtractInterlaced(const AM_MEDIA_TYPE* pmt);

	public:
		CBaseAP(HWND hWnd, HRESULT& hr, CString &_Error);
		~CBaseAP();

		CCritSec m_VMR9AlphaBitmapLock;
		void UpdateAlphaBitmap();
		void ResetStats();

		// ISubPicAllocatorPresenter
		STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
		STDMETHODIMP_(bool) Paint(bool fAll);
		STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size);
		STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget);
		STDMETHODIMP SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace);
	};

	class CGenlock
	{
	public:
		class MovingAverage
		{
		public:
			MovingAverage(INT size):
			  fifoSize(size),
				  oldestSample(0),
				  sum(0)
			  {
				  if (fifoSize > MAX_FIFO_SIZE)
				  {
					  fifoSize = MAX_FIFO_SIZE;
				  }
				  for (INT i = 0; i < MAX_FIFO_SIZE; i++)
					  fifo[i] = 0;
			  }

			  ~MovingAverage()
			  {
			  }

			  double Average(double sample)
			  {
				  sum = sum + sample - fifo[oldestSample];
				  fifo[oldestSample] = sample;
				  oldestSample++;
				  if (oldestSample == fifoSize)
					  oldestSample = 0;
				  return sum / fifoSize;
			  }

		private:
			INT fifoSize;
			double fifo[MAX_FIFO_SIZE];
			INT oldestSample;
			double sum;
		};

		CGenlock(DOUBLE target, DOUBLE limit, INT rowD, INT colD, DOUBLE clockD, UINT mon);
		~CGenlock();

		BOOL PowerstripRunning(); // TRUE if PowerStrip is running
		HRESULT GetTiming(); // Get the string representing the display's current timing parameters
		HRESULT ResetTiming(); // Reset timing to what was last registered by GetTiming()
		HRESULT ResetClock(); // Reset reference clock speed to nominal
		HRESULT SetTargetSyncOffset(DOUBLE targetD);
		HRESULT GetTargetSyncOffset(DOUBLE *targetD);
		HRESULT SetControlLimit(DOUBLE cL);
		HRESULT GetControlLimit(DOUBLE *cL);
		HRESULT SetDisplayResolution(UINT columns, UINT lines);
		HRESULT AdviseSyncClock(CComPtr<ISyncClock> sC);
		HRESULT SetMonitor(UINT mon); // Set the number of the monitor to synchronize
		HRESULT ResetStats(); // Reset timing statistics

		HRESULT ControlDisplay(double syncOffset, double frameCycle); // Adjust the frequency of the display if needed
		HRESULT ControlClock(double syncOffset, double frameCycle); // Adjust the frequency of the clock if needed
		HRESULT UpdateStats(double syncOffset, double frameCycle); // Don't adjust anything, just update the syncOffset stats

		BOOL powerstripTimingExists; // TRUE if display timing has been got through Powerstrip
		BOOL liveSource; // TRUE if live source -> display sync is the only option
		INT adjDelta; // -1 for display slower in relation to video, 0 for keep, 1 for faster 
		INT lineDelta; // The number of rows added or subtracted when adjusting display fps
		INT columnDelta; // The number of colums added or subtracted when adjusting display fps
		DOUBLE cycleDelta; // Adjustment factor for cycle time as fraction of nominal value
		UINT displayAdjustmentsMade; // The number of adjustments made to display refresh rate
		UINT clockAdjustmentsMade; // The number of adjustments made to clock frequency

		UINT totalLines, totalColumns; // Including the porches and sync widths
		UINT visibleLines, visibleColumns; // The nominal resolution
		MovingAverage *syncOffsetFifo;
		MovingAverage *frameCycleFifo;
		DOUBLE minSyncOffset, maxSyncOffset;
		DOUBLE syncOffsetAvg; // Average of the above
		DOUBLE minFrameCycle, maxFrameCycle;
		DOUBLE frameCycleAvg;

		UINT pixelClock; // In pixels/s
		DOUBLE displayFreqCruise;  // Nominal display frequency in frames/s
		DOUBLE displayFreqSlower;
		DOUBLE displayFreqFaster;
		DOUBLE curDisplayFreq; // Current (adjusted) display frequency
		DOUBLE controlLimit; // How much the sync offset is allowed to drift from target sync offset
		WPARAM monitor; // The monitor to be controlled. 0-based.
		CComPtr<ISyncClock> syncClock; // Interface to an adjustable reference clock

	private:
		HWND psWnd; // PowerStrip window
		const static INT TIMING_PARAM_CNT = 10;
		const static INT MAX_LOADSTRING = 100;
		UINT displayTiming[TIMING_PARAM_CNT]; // Display timing parameters
		UINT displayTimingSave[TIMING_PARAM_CNT]; // So that we can reset the display at exit
		TCHAR faster[MAX_LOADSTRING]; // String corresponding to faster display frequency
		TCHAR cruise[MAX_LOADSTRING]; // String corresponding to nominal display frequency
		TCHAR slower[MAX_LOADSTRING]; // String corresponding to slower display frequency
		TCHAR savedTiming[MAX_LOADSTRING]; // String version of saved timing (to be restored upon exit)
		DOUBLE lowSyncOffset; // The closest we want to let the scheduled render time to get to the next vsync. In % of the frame time
		DOUBLE targetSyncOffset; // Where we want the scheduled render time to be in relation to the next vsync
		DOUBLE highSyncOffset; // The furthers we want to let the scheduled render time to get to the next vsync
		CCritSec csGenlockLock;
	};
}
