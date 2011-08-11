/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
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
#include "RenderersSettings.h"
#include "DX9AllocatorPresenter.h"
#include <InitGuid.h>
#include <utility>
#include "../../../SubPic/DX9SubPic.h"
#include "../../../SubPic/SubPicQueueImpl.h"
#include "IPinHook.h"
#include <Version.h>

CCritSec g_ffdshowReceive;
bool queue_ffdshow_support = false;

// only for debugging
//#define DISABLE_USING_D3D9EX

#define FRAMERATE_MAX_DELTA			3000
//#pragma optimize("", off)
//#pragma inline_depth(0)

using namespace DSObjects;
using namespace std;

// CDX9AllocatorPresenter

CDX9AllocatorPresenter::CDX9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, bool bIsEVR, CString &_Error)
	: CDX9RenderingEngine(hWnd, hr, &_Error)
	, m_RefreshRate(0)
	, m_nTearingPos(0)
	, m_nVMR9Surfaces(0)
	, m_iVMR9Surface(0)
	, m_rtTimePerFrame(0)
	, m_bInterlaced(false)
	, m_nUsedBuffer(0)
	, m_OrderedPaint(0)
	, m_bCorrectedFrameTime(0)
	, m_FrameTimeCorrection(0)
	, m_LastSampleTime(0)
	, m_LastFrameDuration(0)
	, m_bAlternativeVSync(0)
	, m_bIsEVR(bIsEVR)
	, m_VSyncMode(0)
	, m_TextScale(1.0)
	, m_MainThreadId(0)
	, m_bNeedCheckSample(true)
	, m_pDirectDraw(NULL)
	, m_hVSyncThread(NULL)
	, m_hEvtQuit(NULL)
	, m_bIsFullscreen(bFullscreen)
	, m_Decoder(_T(""))
{
	HINSTANCE		hDll;

	if(FAILED(hr)) {
		_Error += L"ISubPicAllocatorPresenterImpl failed\n";
		return;
	}

	m_pD3DXLoadSurfaceFromMemory	= NULL;
	m_pD3DXLoadSurfaceFromSurface = NULL;
	m_pD3DXCreateLine				= NULL;
	m_pD3DXCreateFont				= NULL;
	m_pD3DXCreateSprite				= NULL;
	hDll							= GetRenderersData()->GetD3X9Dll();
	if(hDll) {
		(FARPROC&)m_pD3DXLoadSurfaceFromMemory	= GetProcAddress(hDll, "D3DXLoadSurfaceFromMemory");
		(FARPROC&)m_pD3DXLoadSurfaceFromSurface = GetProcAddress(hDll, "D3DXLoadSurfaceFromSurface");
		(FARPROC&)m_pD3DXCreateLine				= GetProcAddress(hDll, "D3DXCreateLine");
		(FARPROC&)m_pD3DXCreateFont				= GetProcAddress(hDll, "D3DXCreateFontW");
		(FARPROC&)m_pD3DXCreateSprite			= GetProcAddress(hDll, "D3DXCreateSprite");
	} else {
		_Error += L"The installed DirectX End-User Runtime is outdated. Please download and install the ";
		_Error += DIRECTX_SDK_DATE;
		_Error += L" release or newer in order for MPC-HC to function properly.\n";
	}

	m_pDwmIsCompositionEnabled = NULL;
	m_pDwmEnableComposition = NULL;
	m_hDWMAPI = LoadLibrary(L"dwmapi.dll");
	if (m_hDWMAPI) {
		(FARPROC &)m_pDwmIsCompositionEnabled = GetProcAddress(m_hDWMAPI, "DwmIsCompositionEnabled");
		(FARPROC &)m_pDwmEnableComposition = GetProcAddress(m_hDWMAPI, "DwmEnableComposition");
	}

	m_pDirect3DCreate9Ex = NULL;
	m_hD3D9 = LoadLibrary(L"d3d9.dll");
#ifndef DISABLE_USING_D3D9EX
	if (m_hD3D9) {
		(FARPROC &)m_pDirect3DCreate9Ex = GetProcAddress(m_hD3D9, "Direct3DCreate9Ex");
	}
#endif

	if (m_pDirect3DCreate9Ex) {
		m_pDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
		if(!m_pD3DEx) {
			m_pDirect3DCreate9Ex(D3D9b_SDK_VERSION, &m_pD3DEx);
		}
	}
	if(!m_pD3DEx) {
		m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
		if(!m_pD3D) {
			m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));
		}
	} else {
		m_pD3D = m_pD3DEx;
	}

	m_DetectedFrameRate = 0.0;
	m_DetectedFrameTime = 0.0;
	m_DetectedFrameTimeStdDev = 0.0;
	m_DetectedLock = false;
	ZeroMemory(m_DetectedFrameTimeHistory, sizeof(m_DetectedFrameTimeHistory));
	ZeroMemory(m_DetectedFrameTimeHistoryHistory, sizeof(m_DetectedFrameTimeHistoryHistory));
	m_DetectedFrameTimePos = 0;
	ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap));

	ZeroMemory(m_ldDetectedRefreshRateList, sizeof(m_ldDetectedRefreshRateList));
	ZeroMemory(m_ldDetectedScanlineRateList, sizeof(m_ldDetectedScanlineRateList));
	m_DetectedRefreshRatePos = 0;
	m_DetectedRefreshTimePrim = 0;
	m_DetectedScanlineTime = 0;
	m_DetectedScanlineTimePrim = 0;
	m_DetectedRefreshRate = 0;
	CRenderersSettings& s = GetRenderersSettings();

	if (s.m_RenderSettings.iVMRDisableDesktopComposition) {
		m_bDesktopCompositionDisabled = true;
		if (m_pDwmEnableComposition) {
			m_pDwmEnableComposition(0);
		}
	} else {
		m_bDesktopCompositionDisabled = false;
	}

	hr = CreateDevice(_Error);

	memset (m_pllJitter, 0, sizeof(m_pllJitter));
	memset (m_pllSyncOffset, 0, sizeof(m_pllSyncOffset));
	m_nNextJitter		= 0;
	m_nNextSyncOffset = 0;
	m_llLastPerf		= 0;
	m_fAvrFps			= 0.0;
	m_fJitterStdDev		= 0.0;
	m_fSyncOffsetStdDev = 0.0;
	m_fSyncOffsetAvr	= 0.0;
	m_bSyncStatsAvailable = false;
}

CDX9AllocatorPresenter::~CDX9AllocatorPresenter()
{
	if (m_bDesktopCompositionDisabled) {
		m_bDesktopCompositionDisabled = false;
		if (m_pDwmEnableComposition) {
			m_pDwmEnableComposition(1);
		}
	}

	StopWorkerThreads();
	m_pFont		= NULL;
	m_pLine		= NULL;
	m_pD3DDev	= NULL;
	m_pD3DDevEx = NULL;

	CleanupRenderingEngine();

	m_pD3D		= NULL;
	m_pD3DEx	= NULL;
	if (m_hDWMAPI) {
		FreeLibrary(m_hDWMAPI);
		m_hDWMAPI = NULL;
	}
	if (m_hD3D9) {
		FreeLibrary(m_hD3D9);
		m_hD3D9 = NULL;
	}
}

void ModerateFloat(double& Value, double Target, double& ValuePrim, double ChangeSpeed);

#if 0
class CRandom31
{
public:

	CRandom31() {
		m_Seed = 12164;
	}

	void f_SetSeed(int32 _Seed) {
		m_Seed = _Seed;
	}
	int32 f_GetSeed() {
		return m_Seed;
	}
	/*
	Park and Miller's psuedo-random number generator.
	*/
	int32 m_Seed;
	int32 f_Get() {
		static const int32 A = 16807;
		static const int32 M = 2147483647;		// 2^31 - 1
		static const int32 q = M / A;			// M / A
		static const int32 r = M % A;			// M % A
		m_Seed = A * (m_Seed % q) - r * (m_Seed / q);
		if (m_Seed < 0) {
			m_Seed += M;
		}
		return m_Seed;
	}

	static int32 fs_Max() {
		return 2147483646;
	}

	double f_GetFloat() {
		return double(f_Get()) * (1.0 / double(fs_Max()));
	}
};

class CVSyncEstimation
{
private:
	class CHistoryEntry
	{
	public:
		CHistoryEntry() {
			m_Time = 0;
			m_ScanLine = -1;
		}
		LONGLONG m_Time;
		int m_ScanLine;
	};

	class CSolution
	{
	public:
		CSolution() {
			m_ScanLines = 1000;
			m_ScanLinesPerSecond = m_ScanLines * 100;
		}
		int m_ScanLines;
		double m_ScanLinesPerSecond;
		double m_SqrSum;

		void f_Mutate(double _Amount, CRandom31 &_Random, int _MinScans) {
			int ToDo = _Random.f_Get() % 10;
			if (ToDo == 0) {
				m_ScanLines = m_ScanLines / 2;
			} else if (ToDo == 1) {
				m_ScanLines = m_ScanLines * 2;
			}

			m_ScanLines = m_ScanLines * (1.0 + (_Random.f_GetFloat() * _Amount) - _Amount * 0.5);
			m_ScanLines = max(m_ScanLines, _MinScans);

			if (ToDo == 2) {
				m_ScanLinesPerSecond /= (_Random.f_Get() % 4) + 1;
			} else if (ToDo == 3) {
				m_ScanLinesPerSecond *= (_Random.f_Get() % 4) + 1;
			}

			m_ScanLinesPerSecond *= 1.0 + (_Random.f_GetFloat() * _Amount) - _Amount * 0.5;
		}

		void f_SpawnInto(CSolution &_Other, CRandom31 &_Random, int _MinScans) {
			_Other = *this;
			_Other.f_Mutate(_Random.f_GetFloat() * 0.1, _Random, _MinScans);
		}

		static int fs_Compare(const void *_pFirst, const void *_pSecond) {
			const CSolution *pFirst = (const CSolution *)_pFirst;
			const CSolution *pSecond = (const CSolution *)_pSecond;
			if (pFirst->m_SqrSum < pSecond->m_SqrSum) {
				return -1;
			} else if (pFirst->m_SqrSum > pSecond->m_SqrSum) {
				return 1;
			}
			return 0;
		}


	};

	enum {
		ENumHistory = 128
	};

	CHistoryEntry m_History[ENumHistory];
	int m_iHistory;
	CSolution m_OldSolutions[2];

	CRandom31 m_Random;


	double fp_GetSquareSum(double _ScansPerSecond, double _ScanLines) {
		double SquareSum = 0;
		int nHistory = min(m_nHistory, ENumHistory);
		int iHistory = m_iHistory - nHistory;
		if (iHistory < 0) {
			iHistory += ENumHistory;
		}
		for (int i = 1; i < nHistory; ++i) {
			int iHistory0 = iHistory + i - 1;
			int iHistory1 = iHistory + i;
			if (iHistory0 < 0) {
				iHistory0 += ENumHistory;
			}
			iHistory0 = iHistory0 % ENumHistory;
			iHistory1 = iHistory1 % ENumHistory;
			ASSERT(m_History[iHistory0].m_Time != 0);
			ASSERT(m_History[iHistory1].m_Time != 0);

			double DeltaTime = (m_History[iHistory1].m_Time - m_History[iHistory0].m_Time)/10000000.0;
			double PredictedScanLine = m_History[iHistory0].m_ScanLine + DeltaTime * _ScansPerSecond;
			PredictedScanLine = fmod(PredictedScanLine, _ScanLines);
			double Delta = (m_History[iHistory1].m_ScanLine - PredictedScanLine);
			double DeltaSqr = Delta * Delta;
			SquareSum += DeltaSqr;
		}
		return SquareSum;
	}

	int m_nHistory;
public:

	CVSyncEstimation() {
		m_iHistory = 0;
		m_nHistory = 0;
	}

	void f_AddSample(int _ScanLine, LONGLONG _Time) {
		m_History[m_iHistory].m_ScanLine = _ScanLine;
		m_History[m_iHistory].m_Time = _Time;
		++m_nHistory;
		m_iHistory = (m_iHistory + 1) % ENumHistory;
	}

	void f_GetEstimation(double &_RefreshRate, int &_ScanLines, int _ScreenSizeY, int _WindowsRefreshRate) {
		_RefreshRate = 0;
		_ScanLines = 0;

		int iHistory = m_iHistory;
		// We have a full history
		if (m_nHistory > 10) {
			for (int l = 0; l < 5; ++l) {
				const int nSol = 3+5+5+3;
				CSolution Solutions[nSol];

				Solutions[0] = m_OldSolutions[0];
				Solutions[1] = m_OldSolutions[1];
				Solutions[2].m_ScanLines = _ScreenSizeY;
				Solutions[2].m_ScanLinesPerSecond = _ScreenSizeY * _WindowsRefreshRate;

				int iStart = 3;
				for (int i = iStart; i < iStart + 5; ++i) {
					Solutions[0].f_SpawnInto(Solutions[i], m_Random, _ScreenSizeY);
				}
				iStart += 5;
				for (int i = iStart; i < iStart + 5; ++i) {
					Solutions[1].f_SpawnInto(Solutions[i], m_Random, _ScreenSizeY);
				}
				iStart += 5;
				for (int i = iStart; i < iStart + 3; ++i) {
					Solutions[2].f_SpawnInto(Solutions[i], m_Random, _ScreenSizeY);
				}

				int Start = 2;
				if (l == 0) {
					Start = 0;
				}
				for (int i = Start; i < nSol; ++i) {
					Solutions[i].m_SqrSum = fp_GetSquareSum(Solutions[i].m_ScanLinesPerSecond, Solutions[i].m_ScanLines);
				}

				qsort(Solutions, nSol, sizeof(Solutions[0]), &CSolution::fs_Compare);
				for (int i = 0; i < 2; ++i) {
					m_OldSolutions[i] = Solutions[i];
				}
			}

			_ScanLines = m_OldSolutions[0].m_ScanLines + 0.5;
			_RefreshRate = 1.0 / (m_OldSolutions[0].m_ScanLines / m_OldSolutions[0].m_ScanLinesPerSecond);
		} else {
			m_OldSolutions[0].m_ScanLines = _ScreenSizeY;
			m_OldSolutions[1].m_ScanLines = _ScreenSizeY;
		}
	}
};
#endif

void CDX9AllocatorPresenter::VSyncThread()
{
	HANDLE				hEvts[]		= { m_hEvtQuit};
	bool				bQuit		= false;
	TIMECAPS			tc;
	DWORD				dwResolution;
	DWORD				dwUser		= 0;

	//DWORD				dwTaskIndex	= 0;
	//// Tell Vista Multimedia Class Scheduler we are a playback thread (increase priority)
	//if (pfAvSetMmThreadCharacteristicsW)
	//	hAvrt = pfAvSetMmThreadCharacteristicsW (L"Playback", &dwTaskIndex);
	//if (pfAvSetMmThreadPriority)
	//	pfAvSetMmThreadPriority (hAvrt, AVRT_PRIORITY_HIGH /*AVRT_PRIORITY_CRITICAL*/);

	//Sleep(2000);	// Remove ugly patch : create a 2s delay on opening files with Win7!

	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	dwResolution	= min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
	dwUser			= timeBeginPeriod(dwResolution);
	CRenderersData *pApp = GetRenderersData();
	CRenderersSettings& s = GetRenderersSettings();

	while (!bQuit) {

		DWORD dwObject = WaitForMultipleObjects (countof(hEvts), hEvts, FALSE, 1);
		switch (dwObject) {
			case WAIT_OBJECT_0 :
				bQuit = true;
				break;
			case WAIT_TIMEOUT : {
				// Do our stuff
				if (m_pD3DDev && s.m_RenderSettings.iVMR9VSync) {

					int VSyncPos = GetVBlackPos();
					int WaitRange = max(m_ScreenSize.cy / 40, 5);
					int MinRange = max(min(int(0.003 * double(m_ScreenSize.cy) * double(m_RefreshRate) + 0.5), m_ScreenSize.cy/3), 5); // 1.8  ms or max 33 % of Time

					VSyncPos += MinRange + WaitRange;

					VSyncPos = VSyncPos % m_ScreenSize.cy;
					if (VSyncPos < 0) {
						VSyncPos += m_ScreenSize.cy;
					}

					int ScanLine = 0;
					int StartScanLine = ScanLine;
					UNUSED_ALWAYS(StartScanLine);
					int LastPos = ScanLine;
					UNUSED_ALWAYS(LastPos);
					ScanLine = (VSyncPos + 1) % m_ScreenSize.cy;
					if (ScanLine < 0) {
						ScanLine += m_ScreenSize.cy;
					}
					int ScanLineMiddle = ScanLine + m_ScreenSize.cy/2;
					ScanLineMiddle = ScanLineMiddle % m_ScreenSize.cy;
					if (ScanLineMiddle < 0) {
						ScanLineMiddle += m_ScreenSize.cy;
					}

					int ScanlineStart = ScanLine;
					bool bTakenLock;
					WaitForVBlankRange(ScanlineStart, 5, true, true, false, bTakenLock);
					LONGLONG TimeStart = pApp->GetPerfCounter();

					WaitForVBlankRange(ScanLineMiddle, 5, true, true, false, bTakenLock);
					LONGLONG TimeMiddle = pApp->GetPerfCounter();

					int ScanlineEnd = ScanLine;
					WaitForVBlankRange(ScanlineEnd, 5, true, true, false, bTakenLock);
					LONGLONG TimeEnd = pApp->GetPerfCounter();

					double nSeconds = double(TimeEnd - TimeStart) / 10000000.0;
					LONGLONG DiffMiddle = TimeMiddle - TimeStart;
					LONGLONG DiffEnd = TimeEnd - TimeMiddle;
					double DiffDiff;
					if (DiffEnd > DiffMiddle) {
						DiffDiff = double(DiffEnd) / double(DiffMiddle);
					} else {
						DiffDiff = double(DiffMiddle) / double(DiffEnd);
					}
					if (nSeconds > 0.003 && DiffDiff < 1.3) {
						double ScanLineSeconds;
						double nScanLines;
						if (ScanLineMiddle > ScanlineEnd) {
							ScanLineSeconds = double(TimeMiddle - TimeStart) / 10000000.0;
							nScanLines = ScanLineMiddle - ScanlineStart;
						} else {
							ScanLineSeconds = double(TimeEnd - TimeMiddle) / 10000000.0;
							nScanLines = ScanlineEnd - ScanLineMiddle;
						}

						double ScanLineTime = ScanLineSeconds / nScanLines;

						int iPos = m_DetectedRefreshRatePos	% 100;
						m_ldDetectedScanlineRateList[iPos] = ScanLineTime;
						if (m_DetectedScanlineTime && ScanlineStart != ScanlineEnd) {
							int Diff = ScanlineEnd - ScanlineStart;
							nSeconds -= double(Diff) * m_DetectedScanlineTime;
						}
						m_ldDetectedRefreshRateList[iPos] = nSeconds;
						double Average = 0;
						double AverageScanline = 0;
						int nPos = min(iPos + 1, 100);
						for (int i = 0; i < nPos; ++i) {
							Average += m_ldDetectedRefreshRateList[i];
							AverageScanline += m_ldDetectedScanlineRateList[i];
						}

						if (nPos) {
							Average /= double(nPos);
							AverageScanline /= double(nPos);
						} else {
							Average = 0;
							AverageScanline = 0;
						}

						double ThisValue = Average;

						if (Average > 0.0 && AverageScanline > 0.0) {
							CAutoLock Lock(&m_RefreshRateLock);
							++m_DetectedRefreshRatePos;
							if (m_DetectedRefreshTime == 0 || m_DetectedRefreshTime / ThisValue > 1.01 || m_DetectedRefreshTime / ThisValue < 0.99) {
								m_DetectedRefreshTime = ThisValue;
								m_DetectedRefreshTimePrim = 0;
							}
							if(_isnan(m_DetectedRefreshTime)) {m_DetectedRefreshTime = 0.0;}
							if(_isnan(m_DetectedRefreshTimePrim)) {m_DetectedRefreshTimePrim = 0.0;}

							ModerateFloat(m_DetectedRefreshTime, ThisValue, m_DetectedRefreshTimePrim, 1.5);
							if (m_DetectedRefreshTime > 0.0) {
								m_DetectedRefreshRate = 1.0/m_DetectedRefreshTime;
							} else {
								m_DetectedRefreshRate = 0.0;
							}

							if (m_DetectedScanlineTime == 0 || m_DetectedScanlineTime / AverageScanline > 1.01 || m_DetectedScanlineTime / AverageScanline < 0.99) {
								m_DetectedScanlineTime = AverageScanline;
								m_DetectedScanlineTimePrim = 0;
							}
							ModerateFloat(m_DetectedScanlineTime, AverageScanline, m_DetectedScanlineTimePrim, 1.5);
							if (m_DetectedScanlineTime > 0.0) {
								m_DetectedScanlinesPerFrame = m_DetectedRefreshTime / m_DetectedScanlineTime;
							} else {
								m_DetectedScanlinesPerFrame = 0;
							}
						}
						//TRACE("Refresh: %f\n", RefreshRate);
					}
				} else {
					m_DetectedRefreshRate = 0.0;
					m_DetectedScanlinesPerFrame = 0.0;
				}
			}
			break;
		}
	}

	timeEndPeriod (dwResolution);
	//	if (pfAvRevertMmThreadCharacteristics) pfAvRevertMmThreadCharacteristics (hAvrt);
}

DWORD WINAPI CDX9AllocatorPresenter::VSyncThreadStatic(LPVOID lpParam)
{
	SetThreadName((DWORD)-1, "CDX9Presenter::VSyncThread");
	CDX9AllocatorPresenter*		pThis = (CDX9AllocatorPresenter*) lpParam;
	pThis->VSyncThread();
	return 0;
}

void CDX9AllocatorPresenter::StartWorkerThreads()
{
	DWORD dwThreadId;

	if ( m_bIsEVR ) {
		m_hEvtQuit = CreateEvent( NULL, TRUE, FALSE, NULL );
		if ( m_hEvtQuit != NULL ) { // Don't create a thread with no stop switch
			m_hVSyncThread = ::CreateThread( NULL, 0, VSyncThreadStatic, (LPVOID)this, 0, &dwThreadId );
			if ( m_hVSyncThread != NULL ) {
				SetThreadPriority( m_hVSyncThread, THREAD_PRIORITY_HIGHEST );
			}
		}
	}
}

void CDX9AllocatorPresenter::StopWorkerThreads()
{
	if ( m_bIsEVR ) {
		if ( m_hEvtQuit != NULL ) {
			SetEvent( m_hEvtQuit );

			if ( m_hVSyncThread != NULL ) {
				if ( WaitForSingleObject(m_hVSyncThread, 10000) == WAIT_TIMEOUT ) {
					ASSERT(FALSE);
					TerminateThread( m_hVSyncThread, 0xDEAD );
				}

				CloseHandle( m_hVSyncThread );
				m_hVSyncThread = NULL;
			}

			CloseHandle( m_hEvtQuit );
			m_hEvtQuit = NULL;
		}
	}
}

bool CDX9AllocatorPresenter::SettingsNeedResetDevice()
{
	CRenderersSettings& s = GetRenderersSettings();
	CRenderersSettings::CRendererSettingsEVR & New = GetRenderersSettings().m_RenderSettings;
	CRenderersSettings::CRendererSettingsEVR & Current = m_LastRendererSettings;

	bool bRet = false;

	bRet = bRet || New.fVMR9AlterativeVSync != Current.fVMR9AlterativeVSync;
	bRet = bRet || New.iVMR9VSyncAccurate != Current.iVMR9VSyncAccurate;

	if (m_bIsFullscreen) {
		bRet = bRet || New.iVMR9FullscreenGUISupport != Current.iVMR9FullscreenGUISupport;
	} else {
		if (Current.iVMRDisableDesktopComposition) {
			if (!m_bDesktopCompositionDisabled) {
				m_bDesktopCompositionDisabled = true;
				if (m_pDwmEnableComposition) {
					m_pDwmEnableComposition(0);
				}
			}
		} else {
			if (m_bDesktopCompositionDisabled) {
				m_bDesktopCompositionDisabled = false;
				if (m_pDwmEnableComposition) {
					m_pDwmEnableComposition(1);
				}
			}
		}
	}

	if (m_bIsEVR) {
		bRet = bRet || New.iEVRHighColorResolution != Current.iEVRHighColorResolution;
		bRet = bRet || New.iEVRForceInputHighColorResolution != Current.iEVRForceInputHighColorResolution;
	}

	m_LastRendererSettings = s.m_RenderSettings;

	return bRet;
}

HRESULT CDX9AllocatorPresenter::CreateDevice(CString &_Error)
{
	StopWorkerThreads();

	CRenderersSettings& s = GetRenderersSettings();
	CRenderersData* renderersData = GetRenderersData();

	m_VBlankEndWait = 0;
	m_VBlankMin = 300000;
	m_VBlankMinCalc = 300000;
	m_VBlankMax = 0;
	m_VBlankStartWait = 0;
	m_VBlankWaitTime = 0;
	m_VBlankLockTime = 0;
	m_PresentWaitTime = 0;
	m_PresentWaitTimeMin = 3000000000;
	m_PresentWaitTimeMax = 0;

	m_LastRendererSettings = s.m_RenderSettings;

	m_VBlankEndPresent = -100000;
	m_VBlankStartMeasureTime = 0;
	m_VBlankStartMeasure = 0;

	m_PaintTime = 0;
	m_PaintTimeMin = 3000000000;
	m_PaintTimeMax = 0;

	m_RasterStatusWaitTime = 0;
	m_RasterStatusWaitTimeMin = 3000000000;
	m_RasterStatusWaitTimeMax = 0;
	m_RasterStatusWaitTimeMaxCalc = 0;

	m_ClockDiff = 0.0;
	m_ClockDiffPrim = 0.0;
	m_ClockDiffCalc = 0.0;

	m_ModeratedTimeSpeed = 1.0;
	m_ModeratedTimeSpeedDiff = 0.0;
	m_ModeratedTimeSpeedPrim = 0;
	ZeroMemory(m_TimeChangeHistory, sizeof(m_TimeChangeHistory));
	ZeroMemory(m_ClockChangeHistory, sizeof(m_ClockChangeHistory));
	m_ClockTimeChangeHistoryPos = 0;

	m_pD3DDev = NULL;
	m_pD3DDevEx = NULL;
	m_pDirectDraw = NULL;

	CleanupRenderingEngine();

	if(!m_pD3D) {
		_Error += L"Failed to create D3D9\n";
		return E_UNEXPECTED;
	}

	HRESULT hr = S_OK;
	m_CurrentAdapter = GetAdapter(m_pD3D);

	/*		// TODO : add nVidia PerfHUD !!!

	// Set default settings
	UINT AdapterToUse=D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType=D3DDEVTYPE_HAL;

	#if SHIPPING_VERSION
	// When building a shipping version, disable PerfHUD (opt-out)
	#else
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for (UINT Adapter=0;Adapter<g_pD3D->GetAdapterCount();Adapter++)
	{
	  D3DADAPTER_IDENTIFIER9  Identifier;
	  HRESULT       Res;

	Res = g_pD3D->GetAdapterIdentifier(Adapter,0,&Identifier);
	  if (strstr(Identifier.Description,"PerfHUD") != 0)
	 {
	  AdapterToUse=Adapter;
	  DeviceType=D3DDEVTYPE_REF;
	  break;
	 }
	}
	#endif

	if (FAILED(g_pD3D->CreateDevice( AdapterToUse, DeviceType, hWnd,
	  D3DCREATE_HARDWARE_VERTEXPROCESSING,
	&d3dpp, &g_pd3dDevice) ) )
	{
	 return E_FAIL;
	}
	*/


	//#define ENABLE_DDRAWSYNC
#ifdef ENABLE_DDRAWSYNC
	hr = DirectDrawCreate(NULL, &m_pDirectDraw, NULL) ;
	if (hr == S_OK) {
		hr = m_pDirectDraw->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL) ;
	}
#endif

	D3DPRESENT_PARAMETERS pp;
	ZeroMemory(&pp, sizeof(pp));

	BOOL bCompositionEnabled = false;
	if (m_pDwmIsCompositionEnabled) {
		m_pDwmIsCompositionEnabled(&bCompositionEnabled);
	}

	m_bCompositionEnabled = !!bCompositionEnabled;
	m_bAlternativeVSync = s.m_RenderSettings.fVMR9AlterativeVSync;

	// detect FP textures support
	renderersData->m_bFP16Support = SUCCEEDED(m_pD3D->CheckDeviceFormat(m_CurrentAdapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_FILTER, D3DRTYPE_VOLUMETEXTURE, D3DFMT_A32B32G32R32F));

	// detect 10-bit textures support
	renderersData->m_b10bitSupport = SUCCEEDED(m_pD3D->CheckDeviceFormat(m_CurrentAdapter, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, D3DFMT_A2R10G10B10));

	// set settings that depend on hardware feature support
	m_bForceInputHighColorResolution = s.m_RenderSettings.iEVRForceInputHighColorResolution && m_bIsEVR && renderersData->m_b10bitSupport;
	m_bHighColorResolution = s.m_RenderSettings.iEVRHighColorResolution && m_bIsEVR && renderersData->m_b10bitSupport;
	m_bFullFloatingPointProcessing = s.m_RenderSettings.iVMR9FullFloatingPointProcessing && renderersData->m_bFP16Support;
	m_bHalfFloatingPointProcessing = s.m_RenderSettings.iVMR9HalfFloatingPointProcessing && renderersData->m_bFP16Support && !m_bFullFloatingPointProcessing;


	// set color formats
	if (m_bFullFloatingPointProcessing) {
		m_SurfaceType = D3DFMT_A32B32G32R32F;
	} else if (m_bHalfFloatingPointProcessing) {
		m_SurfaceType = D3DFMT_A16B16G16R16F;
	} else if (m_bForceInputHighColorResolution || m_bHighColorResolution) {
		m_SurfaceType = D3DFMT_A2R10G10B10;
	} else {
		if(m_nPCIVendor == PCIV_ATI)
			m_SurfaceType = D3DFMT_X8R8G8B8;
		else
			m_SurfaceType = D3DFMT_A8R8G8B8;
	}

	D3DDISPLAYMODEEX DisplayMode;
	ZeroMemory(&DisplayMode, sizeof(DisplayMode));
	DisplayMode.Size = sizeof(DisplayMode);
	D3DDISPLAYMODE d3ddm;
	ZeroMemory(&d3ddm, sizeof(d3ddm));

	if (m_bIsFullscreen) {
		if (m_bHighColorResolution) {
			pp.BackBufferFormat = D3DFMT_A2R10G10B10;
		} else {
			pp.BackBufferFormat = D3DFMT_X8R8G8B8;	
		}
		pp.Windowed = false;
		pp.BackBufferCount = 3;
		pp.SwapEffect = D3DSWAPEFFECT_FLIP;
		// there's no Desktop composition to take care of alternative vSync in exclusive mode, alternative vSync is therefore unused
		pp.hDeviceWindow = m_hWnd;
		pp.Flags = D3DPRESENTFLAG_VIDEO;
		if (s.m_RenderSettings.iVMR9FullscreenGUISupport && !m_bHighColorResolution) {
			pp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		}
		m_D3DDevExError = L"No m_pD3DEx";
		if (m_pD3DEx) {
			m_pD3DEx->GetAdapterDisplayModeEx(m_CurrentAdapter, &DisplayMode, NULL);

			DisplayMode.Format = pp.BackBufferFormat;
			m_ScreenSize.SetSize(DisplayMode.Width, DisplayMode.Height);
			pp.FullScreen_RefreshRateInHz = m_RefreshRate = DisplayMode.RefreshRate;
			pp.BackBufferWidth = m_ScreenSize.cx;
			pp.BackBufferHeight = m_ScreenSize.cy;

			hr = m_pD3DEx->CreateDeviceEx(
					 m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd,
					 GetVertexProcessing() | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED | D3DCREATE_ENABLE_PRESENTSTATS, //D3DCREATE_MANAGED
					 &pp, &DisplayMode, &m_pD3DDevEx);

			m_D3DDevExError = GetWindowsErrorMessage(hr, m_hD3D9);
			if (m_pD3DDevEx) {
				m_pD3DDev = m_pD3DDevEx;
				m_BackbufferType = pp.BackBufferFormat;
				m_DisplayType = DisplayMode.Format;
			}
		}
		if (!m_pD3DDev) {
			m_pD3D->GetAdapterDisplayMode(m_CurrentAdapter, &d3ddm);
			d3ddm.Format = pp.BackBufferFormat;
			m_ScreenSize.SetSize(d3ddm.Width, d3ddm.Height);
			pp.FullScreen_RefreshRateInHz = m_RefreshRate = d3ddm.RefreshRate;
			pp.BackBufferWidth = m_ScreenSize.cx;
			pp.BackBufferHeight = m_ScreenSize.cy;

			hr = m_pD3D->CreateDevice(
					 m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd,
					 GetVertexProcessing() | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, //D3DCREATE_MANAGED
					 &pp, &m_pD3DDev);
			m_DisplayType = d3ddm.Format;
			m_BackbufferType = pp.BackBufferFormat;
		}
		if (m_pD3DDev && s.m_RenderSettings.iVMR9FullscreenGUISupport && !m_bHighColorResolution) {
			m_pD3DDev->SetDialogBoxMode(true);
			//if (m_pD3DDev->SetDialogBoxMode(true) != S_OK)
			//	ExitProcess(0);
		}
	} else {
		pp.Windowed = TRUE;
		pp.hDeviceWindow = m_hWnd;
		pp.SwapEffect = D3DSWAPEFFECT_COPY;
		pp.Flags = D3DPRESENTFLAG_VIDEO;
		pp.BackBufferCount = 1;
		if (bCompositionEnabled || m_bAlternativeVSync) {
			// Desktop composition takes care of the VSYNC
			pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		//		if(m_fVMRSyncFix = GetRenderersData()->m_s.fVMRSyncFix)
		//			pp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

		if (m_pD3DEx) {
			m_pD3DEx->GetAdapterDisplayModeEx(m_CurrentAdapter, &DisplayMode, NULL);
			m_ScreenSize.SetSize(DisplayMode.Width, DisplayMode.Height);
			m_RefreshRate = DisplayMode.RefreshRate;
			pp.BackBufferWidth = m_ScreenSize.cx;
			pp.BackBufferHeight = m_ScreenSize.cy;

			// We can get 0x8876086a here when switching from two displays to one display using Win + P (Windows 7)
			// Cause: We might not reinitialize dx correctly during the switch
			hr = m_pD3DEx->CreateDeviceEx(
					 m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd,
					 GetVertexProcessing() | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED | D3DCREATE_ENABLE_PRESENTSTATS, //D3DCREATE_MANAGED
					 &pp, NULL, &m_pD3DDevEx);
			if (m_pD3DDevEx) {
				m_pD3DDev = m_pD3DDevEx;
				m_DisplayType = DisplayMode.Format;
			}
		}
		if (!m_pD3DDev) {
			m_pD3D->GetAdapterDisplayMode(m_CurrentAdapter, &d3ddm);
			m_ScreenSize.SetSize(d3ddm.Width, d3ddm.Height);
			m_RefreshRate = d3ddm.RefreshRate;
			pp.BackBufferWidth = m_ScreenSize.cx;
			pp.BackBufferHeight = m_ScreenSize.cy;

			hr = m_pD3D->CreateDevice(
					 m_CurrentAdapter, D3DDEVTYPE_HAL, m_hWnd,
					 GetVertexProcessing() | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, //D3DCREATE_MANAGED
					 &pp, &m_pD3DDev);
			m_DisplayType = d3ddm.Format;
		}
		m_BackbufferType = pp.BackBufferFormat;
	}

	while(hr == D3DERR_DEVICELOST) {
		TRACE("D3DERR_DEVICELOST. Trying to Reset.\n");
		hr = m_pD3DDev->TestCooperativeLevel();
	}
	if (hr == D3DERR_DEVICENOTRESET) {
		TRACE("D3DERR_DEVICENOTRESET\n");
		hr = m_pD3DDev->Reset(&pp);
	}

	TRACE("CreateDevice: %d\n", (LONG)hr);
	ASSERT (SUCCEEDED (hr));

	m_MainThreadId = GetCurrentThreadId();

	if (m_pD3DDevEx) {
		m_pD3DDevEx->SetGPUThreadPriority(7);
	}

	if(FAILED(hr)) {
		_Error += L"CreateDevice failed\n";
		CStringW str;
		str.Format(L"Error code: 0x%X\n", hr);
		_Error += str;

		return hr;
	}

	// Get the device caps
	ZeroMemory(&m_Caps, sizeof(m_Caps));
	m_pD3DDev->GetDeviceCaps(&m_Caps);

	// Initialize the rendering engine
	InitRenderingEngine();

	CComPtr<ISubPicProvider> pSubPicProvider;
	if(m_pSubPicQueue) {
		m_pSubPicQueue->GetSubPicProvider(&pSubPicProvider);
	}

	CSize size;
	switch(GetRenderersSettings().nSPCMaxRes) {
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

	if(m_pAllocator) {
		m_pAllocator->ChangeDevice(m_pD3DDev);
	} else {
		m_pAllocator = DNew CDX9SubPicAllocator(m_pD3DDev, size, GetRenderersSettings().fSPCPow2Tex, false);
		if(!m_pAllocator) {
			_Error += L"CDX9SubPicAllocator failed\n";

			return E_FAIL;
		}
	}

	hr = S_OK;
	m_pSubPicQueue = GetRenderersSettings().nSPCSize > 0
					 ? (ISubPicQueue*)DNew CSubPicQueue(GetRenderersSettings().nSPCSize, !GetRenderersSettings().fSPCAllowAnimationWhenBuffering, m_pAllocator, &hr)
					 : (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);
	if(!m_pSubPicQueue || FAILED(hr)) {
		_Error += L"m_pSubPicQueue failed\n";

		return E_FAIL;
	}

	if(pSubPicProvider) {
		m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);
	}

	m_pFont = NULL;
	if (m_pD3DXCreateFont) {
		int MinSize = 1600;
		int CurrentSize = min(m_ScreenSize.cx, MinSize);
		double Scale = double(CurrentSize) / double(MinSize);
		m_TextScale = Scale;
		m_pD3DXCreateFont( m_pD3DDev,					// D3D device
						   -24.0*Scale,					// Height
						   -11.0*Scale,					// Width
						   CurrentSize < 800 ? FW_NORMAL : FW_BOLD,		// Weight
						   0,							// MipLevels, 0 = autogen mipmaps
						   FALSE,						// Italic
						   DEFAULT_CHARSET,				// CharSet
						   OUT_DEFAULT_PRECIS,			// OutputPrecision
						   ANTIALIASED_QUALITY,			// Quality
						   FIXED_PITCH | FF_DONTCARE,	// PitchAndFamily
						   L"Lucida Console",			// pFaceName
						   &m_pFont);					// ppFont
	}


	m_pSprite = NULL;

	if (m_pD3DXCreateSprite) {
		m_pD3DXCreateSprite( m_pD3DDev,					// D3D device
							 &m_pSprite);
	}

	m_pLine = NULL;
	if (m_pD3DXCreateLine) {
		m_pD3DXCreateLine (m_pD3DDev, &m_pLine);
	}

	m_LastAdapterCheck = GetRenderersData()->GetPerfCounter();

	StartWorkerThreads();

	return S_OK;
}

HRESULT CDX9AllocatorPresenter::AllocSurfaces()
{
	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	return CreateVideoSurfaces();
}

void CDX9AllocatorPresenter::DeleteSurfaces()
{
	CAutoLock cAutoLock(this);
	CAutoLock cRenderLock(&m_RenderLock);

	FreeVideoSurfaces();
}

UINT CDX9AllocatorPresenter::GetAdapter(IDirect3D9* pD3D, bool bGetAdapter)
{
	if(m_hWnd == NULL || pD3D == NULL) {
		return D3DADAPTER_DEFAULT;
	}

	m_D3D9Device = _T("");
	m_nPCIVendor = 0;

	CRenderersSettings& s = GetRenderersSettings();
	if(bGetAdapter && (pD3D->GetAdapterCount()>1) && (s.D3D9RenderDevice != _T(""))) {
		TCHAR		strGUID[50];
		D3DADAPTER_IDENTIFIER9 adapterIdentifier;

		for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp) {
			if (pD3D->GetAdapterIdentifier(adp, 0, &adapterIdentifier) == S_OK) {
				if ((::StringFromGUID2(adapterIdentifier.DeviceIdentifier, strGUID, 50) > 0) && (s.D3D9RenderDevice == strGUID)) {
					m_D3D9Device = adapterIdentifier.Description;
					m_nPCIVendor = adapterIdentifier.VendorId;
					return	adp;
				}
			}
		}
	}

	HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	if(hMonitor == NULL) {
		return D3DADAPTER_DEFAULT;
	}

	for(UINT adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp) {
		HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
		if(hAdpMon == hMonitor) {
			if(bGetAdapter) {
				D3DADAPTER_IDENTIFIER9 adapterIdentifier;
				if (pD3D->GetAdapterIdentifier(adp, 0, &adapterIdentifier) == S_OK) {
					m_D3D9Device = adapterIdentifier.Description;
					m_nPCIVendor = adapterIdentifier.VendorId;
				}
			}
			return adp;
		}
	}

	return D3DADAPTER_DEFAULT;
}

DWORD CDX9AllocatorPresenter::GetVertexProcessing()
{
	HRESULT hr;
	D3DCAPS9 caps;

	hr = m_pD3D->GetDeviceCaps(m_CurrentAdapter, D3DDEVTYPE_HAL, &caps);
	if (FAILED(hr)) {
		return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
			caps.VertexShaderVersion < D3DVS_VERSION(2, 0)) {
		return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	return D3DCREATE_HARDWARE_VERTEXPROCESSING;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CDX9AllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	return E_NOTIMPL;
}

void CDX9AllocatorPresenter::CalculateJitter(LONGLONG PerfCounter)
{
	// Calculate the jitter!
	LONGLONG	llPerf = PerfCounter;
	if ((m_rtTimePerFrame != 0) && (labs ((long)(llPerf - m_llLastPerf)) < m_rtTimePerFrame*3) ) {
		m_nNextJitter = (m_nNextJitter+1) % NB_JITTER;
		m_pllJitter[m_nNextJitter] = llPerf - m_llLastPerf;

		m_MaxJitter = MINLONG64;
		m_MinJitter = MAXLONG64;

		// Calculate the real FPS
		LONGLONG		llJitterSum = 0;
		LONGLONG		llJitterSumAvg = 0;
		for (int i=0; i<NB_JITTER; i++) {
			LONGLONG Jitter = m_pllJitter[i];
			llJitterSum += Jitter;
			llJitterSumAvg += Jitter;
		}
		double FrameTimeMean = double(llJitterSumAvg)/NB_JITTER;
		m_fJitterMean = FrameTimeMean;
		double DeviationSum = 0;
		for (int i=0; i<NB_JITTER; i++) {
			LONGLONG DevInt = m_pllJitter[i] - FrameTimeMean;
			double Deviation = DevInt;
			DeviationSum += Deviation*Deviation;
			m_MaxJitter = max(m_MaxJitter, DevInt);
			m_MinJitter = min(m_MinJitter, DevInt);
		}
		double StdDev = sqrt(DeviationSum/NB_JITTER);

		m_fJitterStdDev = StdDev;

		m_fAvrFps = 10000000.0/(double(llJitterSum)/NB_JITTER);
	}

	m_llLastPerf = llPerf;
}

bool CDX9AllocatorPresenter::GetVBlank(int &_ScanLine, int &_bInVBlank, bool _bMeasureTime)
{
	LONGLONG llPerf = 0;
	if (_bMeasureTime) {
		llPerf = GetRenderersData()->GetPerfCounter();
	}

	int ScanLine = 0;
	_ScanLine = 0;
	_bInVBlank = 0;
	
	if (m_pDirectDraw) {
		DWORD ScanLineGet = 0;
		m_pDirectDraw->GetScanLine(&ScanLineGet);
		BOOL InVBlank;
		if (m_pDirectDraw->GetVerticalBlankStatus (&InVBlank) != S_OK) {
			return false;
		}
		ScanLine = ScanLineGet;
		_bInVBlank = InVBlank;
		if (InVBlank) {
			ScanLine = 0;
		}
	} else {
		D3DRASTER_STATUS RasterStatus;
		if (m_pD3DDev->GetRasterStatus(0, &RasterStatus) != S_OK) {
			return false;
		}
		ScanLine = RasterStatus.ScanLine;
		_bInVBlank = RasterStatus.InVBlank;
	}
	if (_bMeasureTime) {
		m_VBlankMax = max(m_VBlankMax, ScanLine);
		if (ScanLine != 0 && !_bInVBlank) {
			m_VBlankMinCalc = min(m_VBlankMinCalc, ScanLine);
		}
		m_VBlankMin = m_VBlankMax - m_ScreenSize.cy;
	}
	if (_bInVBlank) {
		_ScanLine = 0;
	} else if (m_VBlankMin != 300000) {
		_ScanLine = ScanLine - m_VBlankMin;
	} else {
		_ScanLine = ScanLine;
	}

	if (_bMeasureTime) {
		LONGLONG Time = GetRenderersData()->GetPerfCounter() - llPerf;
		if (Time > 5000000) { // 0.5 sec
			TRACE("GetVBlank too long (%f sec)\n", Time / 10000000.0);
		}
		m_RasterStatusWaitTimeMaxCalc = max(m_RasterStatusWaitTimeMaxCalc, Time);
	}

	return true;
}

bool CDX9AllocatorPresenter::WaitForVBlankRange(int &_RasterStart, int _RasterSize, bool _bWaitIfInside, bool _bNeedAccurate, bool _bMeasure, bool &_bTakenLock)
{
	if (_bMeasure) {
		m_RasterStatusWaitTimeMaxCalc = 0;
	}
	bool bWaited = false;
	int ScanLine = 0;
	int InVBlank = 0;
	LONGLONG llPerf = 0;
	if (_bMeasure) {
		llPerf = GetRenderersData()->GetPerfCounter();
	}
	GetVBlank(ScanLine, InVBlank, _bMeasure);
	if (_bMeasure) {
		m_VBlankStartWait = ScanLine;
	}

	static bool bOneWait = true;
	if (bOneWait && _bMeasure) {
		bOneWait = false;
		// If we are already in the wanted interval we need to wait until we aren't, this improves sync when for example you are playing 23.976 Hz material on a 24 Hz refresh rate
		int nInVBlank = 0;
		while (1) {
			if (!GetVBlank(ScanLine, InVBlank, _bMeasure)) {
				break;
			}

			if (InVBlank && nInVBlank == 0) {
				nInVBlank = 1;
			} else if (!InVBlank && nInVBlank == 1) {
				nInVBlank = 2;
			} else if (InVBlank && nInVBlank == 2) {
				nInVBlank = 3;
			} else if (!InVBlank && nInVBlank == 3) {
				break;
			}
		}
	}
	if (_bWaitIfInside) {
		int ScanLineDiff = long(ScanLine) - _RasterStart;
		if (ScanLineDiff > m_ScreenSize.cy / 2) {
			ScanLineDiff -= m_ScreenSize.cy;
		} else if (ScanLineDiff < -m_ScreenSize.cy / 2) {
			ScanLineDiff += m_ScreenSize.cy;
		}

		if (ScanLineDiff >= 0 && ScanLineDiff <= _RasterSize) {
			bWaited = true;
			// If we are already in the wanted interval we need to wait until we aren't, this improves sync when for example you are playing 23.976 Hz material on a 24 Hz refresh rate
			int LastLineDiff = ScanLineDiff;
			while (1) {
				if (!GetVBlank(ScanLine, InVBlank, _bMeasure)) {
					break;
				}
				int ScanLineDiff = long(ScanLine) - _RasterStart;
				if (ScanLineDiff > m_ScreenSize.cy / 2) {
					ScanLineDiff -= m_ScreenSize.cy;
				} else if (ScanLineDiff < -m_ScreenSize.cy / 2) {
					ScanLineDiff += m_ScreenSize.cy;
				}
				if (!(ScanLineDiff >= 0 && ScanLineDiff <= _RasterSize) || (LastLineDiff < 0 && ScanLineDiff > 0)) {
					break;
				}
				LastLineDiff = ScanLineDiff;
				Sleep(1); // Just sleep
			}
		}
	}
	double RefreshRate = GetRefreshRate();
	LONG ScanLines = GetScanLines();
	int MinRange = max(min(int(0.0015 * double(ScanLines) * RefreshRate + 0.5), ScanLines/3), 5); // 1.5 ms or max 33 % of Time
	int NoSleepStart = _RasterStart - MinRange;
	int NoSleepRange = MinRange;
	if (NoSleepStart < 0) {
		NoSleepStart += m_ScreenSize.cy;
	}

	int MinRange2 = max(min(int(0.0050 * double(ScanLines) * RefreshRate + 0.5), ScanLines/3), 5); // 5 ms or max 33 % of Time
	int D3DDevLockStart = _RasterStart - MinRange2;
	int D3DDevLockRange = MinRange2;
	if (D3DDevLockStart < 0) {
		D3DDevLockStart += m_ScreenSize.cy;
	}

	int ScanLineDiff = ScanLine - _RasterStart;
	if (ScanLineDiff > m_ScreenSize.cy / 2) {
		ScanLineDiff -= m_ScreenSize.cy;
	} else if (ScanLineDiff < -m_ScreenSize.cy / 2) {
		ScanLineDiff += m_ScreenSize.cy;
	}
	int LastLineDiff = ScanLineDiff;


	int ScanLineDiffSleep = long(ScanLine) - NoSleepStart;
	if (ScanLineDiffSleep > m_ScreenSize.cy / 2) {
		ScanLineDiffSleep -= m_ScreenSize.cy;
	} else if (ScanLineDiffSleep < -m_ScreenSize.cy / 2) {
		ScanLineDiffSleep += m_ScreenSize.cy;
	}
	int LastLineDiffSleep = ScanLineDiffSleep;


	int ScanLineDiffLock = long(ScanLine) - D3DDevLockStart;
	if (ScanLineDiffLock > m_ScreenSize.cy / 2) {
		ScanLineDiffLock -= m_ScreenSize.cy;
	} else if (ScanLineDiffLock < -m_ScreenSize.cy / 2) {
		ScanLineDiffLock += m_ScreenSize.cy;
	}
	int LastLineDiffLock = ScanLineDiffLock;

	LONGLONG llPerfLock = 0;

	while (1) {
		if (!GetVBlank(ScanLine, InVBlank, _bMeasure)) {
			break;
		}
		int ScanLineDiff = long(ScanLine) - _RasterStart;
		if (ScanLineDiff > m_ScreenSize.cy / 2) {
			ScanLineDiff -= m_ScreenSize.cy;
		} else if (ScanLineDiff < -m_ScreenSize.cy / 2) {
			ScanLineDiff += m_ScreenSize.cy;
		}
		if ((ScanLineDiff >= 0 && ScanLineDiff <= _RasterSize) || (LastLineDiff < 0 && ScanLineDiff > 0)) {
			break;
		}

		LastLineDiff = ScanLineDiff;

		bWaited = true;

		int ScanLineDiffLock = long(ScanLine) - D3DDevLockStart;
		if (ScanLineDiffLock > m_ScreenSize.cy / 2) {
			ScanLineDiffLock -= m_ScreenSize.cy;
		} else if (ScanLineDiffLock < -m_ScreenSize.cy / 2) {
			ScanLineDiffLock += m_ScreenSize.cy;
		}

		if (((ScanLineDiffLock >= 0 && ScanLineDiffLock <= D3DDevLockRange) || (LastLineDiffLock < 0 && ScanLineDiffLock > 0))) {
			if (!_bTakenLock && _bMeasure) {
				_bTakenLock = true;
				llPerfLock = GetRenderersData()->GetPerfCounter();
				LockD3DDevice();
			}
		}
		LastLineDiffLock = ScanLineDiffLock;


		int ScanLineDiffSleep = long(ScanLine) - NoSleepStart;
		if (ScanLineDiffSleep > m_ScreenSize.cy / 2) {
			ScanLineDiffSleep -= m_ScreenSize.cy;
		} else if (ScanLineDiffSleep < -m_ScreenSize.cy / 2) {
			ScanLineDiffSleep += m_ScreenSize.cy;
		}

		if (!((ScanLineDiffSleep >= 0 && ScanLineDiffSleep <= NoSleepRange) || (LastLineDiffSleep < 0 && ScanLineDiffSleep > 0)) || !_bNeedAccurate) {
			//TRACE("%d\n", RasterStatus.ScanLine);
			Sleep(1); // Don't sleep for the last 1.5 ms scan lines, so we get maximum precision
		}
		LastLineDiffSleep = ScanLineDiffSleep;
	}
	_RasterStart = ScanLine;
	if (_bMeasure) {
		m_VBlankEndWait = ScanLine;
		m_VBlankWaitTime = GetRenderersData()->GetPerfCounter() - llPerf;

		if (_bTakenLock) {
			m_VBlankLockTime = GetRenderersData()->GetPerfCounter() - llPerfLock;
		} else {
			m_VBlankLockTime = 0;
		}

		m_RasterStatusWaitTime = m_RasterStatusWaitTimeMaxCalc;
		m_RasterStatusWaitTimeMin = min(m_RasterStatusWaitTimeMin, m_RasterStatusWaitTime);
		m_RasterStatusWaitTimeMax = max(m_RasterStatusWaitTimeMax, m_RasterStatusWaitTime);
	}

	return bWaited;
}

int CDX9AllocatorPresenter::GetVBlackPos()
{
	CRenderersSettings& s = GetRenderersSettings();
	BOOL bCompositionEnabled = m_bCompositionEnabled;

	int WaitRange = max(m_ScreenSize.cy / 40, 5);
	if (!bCompositionEnabled) {
		if (m_bAlternativeVSync) {
			return s.m_RenderSettings.iVMR9VSyncOffset;
		} else {
			int MinRange = max(min(int(0.005 * double(m_ScreenSize.cy) * GetRefreshRate() + 0.5), m_ScreenSize.cy/3), 5); // 5  ms or max 33 % of Time
			int WaitFor = m_ScreenSize.cy - (MinRange + WaitRange);
			return WaitFor;
		}
	} else {
		int WaitFor = m_ScreenSize.cy / 2;
		return WaitFor;
	}
}

bool CDX9AllocatorPresenter::WaitForVBlank(bool &_Waited, bool &_bTakenLock)
{
	CRenderersSettings& s = GetRenderersSettings();
	if (!s.m_RenderSettings.iVMR9VSync) {
		_Waited = true;
		m_VBlankWaitTime = 0;
		m_VBlankLockTime = 0;
		m_VBlankEndWait = 0;
		m_VBlankStartWait = 0;
		return true;
	}
	//	_Waited = true;
	//	return false;

	BOOL bCompositionEnabled = m_bCompositionEnabled;
	int WaitFor = GetVBlackPos();

	if (!bCompositionEnabled) {
		if (m_bAlternativeVSync) {
			_Waited = WaitForVBlankRange(WaitFor, 0, false, true, true, _bTakenLock);
			return false;
		} else {
			_Waited = WaitForVBlankRange(WaitFor, 0, false, s.m_RenderSettings.iVMR9VSyncAccurate, true, _bTakenLock);
			return true;
		}
	} else {
		// Instead we wait for VBlack after the present, this seems to fix the stuttering problem. It's also possible to fix by removing the Sleep above, but that isn't an option.
		WaitForVBlankRange(WaitFor, 0, false, s.m_RenderSettings.iVMR9VSyncAccurate, true, _bTakenLock);

		return false;
	}
}

void CDX9AllocatorPresenter::UpdateAlphaBitmap()
{
	m_VMR9AlphaBitmapData.Free();

	if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0) {
		HBITMAP			hBitmap = (HBITMAP)GetCurrentObject (m_VMR9AlphaBitmap.hdc, OBJ_BITMAP);
		if (!hBitmap) {
			return;
		}
		DIBSECTION		info = {0};
		if (!::GetObject(hBitmap, sizeof( DIBSECTION ), &info )) {
			return;
		}

		m_VMR9AlphaBitmapRect = CRect(0, 0, info.dsBm.bmWidth, info.dsBm.bmHeight);
		m_VMR9AlphaBitmapWidthBytes = info.dsBm.bmWidthBytes;

		if (m_VMR9AlphaBitmapData.Allocate(info.dsBm.bmWidthBytes * info.dsBm.bmHeight)) {
			memcpy((BYTE *)m_VMR9AlphaBitmapData, info.dsBm.bmBits, info.dsBm.bmWidthBytes * info.dsBm.bmHeight);
		}
	}
}

STDMETHODIMP_(bool) CDX9AllocatorPresenter::Paint(bool fAll)
{
	if (m_bPendingResetDevice) {
		SendResetRequest();
		return false;
	}

	CRenderersSettings& s = GetRenderersSettings();

	//	TRACE("Thread: %d\n", (LONG)((CRITICAL_SECTION &)m_RenderLock).OwningThread);

#if 0
	if (TryEnterCriticalSection (&(CRITICAL_SECTION &)(*((CCritSec *)this)))) {
		LeaveCriticalSection((&(CRITICAL_SECTION &)(*((CCritSec *)this))));
	} else {
		__debugbreak();
	}
#endif

	CRenderersData * pApp = GetRenderersData();

	LONGLONG StartPaint = pApp->GetPerfCounter();
	CAutoLock cRenderLock(&m_RenderLock);

	if(m_WindowRect.right <= m_WindowRect.left || m_WindowRect.bottom <= m_WindowRect.top
			|| m_NativeVideoSize.cx <= 0 || m_NativeVideoSize.cy <= 0
			|| !m_pVideoSurface) {
		if (m_OrderedPaint) {
			--m_OrderedPaint;
		} else {
			//			TRACE("UNORDERED PAINT!!!!!!\n");
		}


		return false;
	}

	HRESULT hr;

	m_pD3DDev->BeginScene();

	CComPtr<IDirect3DSurface9> pBackBuffer;
	m_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

	// Clear the backbuffer
	m_pD3DDev->SetRenderTarget(0, pBackBuffer);
	hr = m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);

	CRect rSrcVid(CPoint(0, 0), GetVisibleVideoSize());
	CRect rDstVid(m_VideoRect);

	CRect rSrcPri(CPoint(0, 0), m_WindowRect.Size());
	CRect rDstPri(m_WindowRect);

	// Render the current video frame
	hr = RenderVideo(pBackBuffer, rSrcVid, rDstVid);

	if (FAILED(hr)) {
		if (m_RenderingPath == RENDERING_PATH_STRETCHRECT) {
			// Support ffdshow queueing
			// m_pD3DDev->StretchRect may fail if ffdshow is using queue output samples.
			// Here we don't want to show the black buffer.
			if (m_OrderedPaint) {
				--m_OrderedPaint;
			} else {
				//				TRACE("UNORDERED PAINT!!!!!!\n");
			}

			return false;
		}
	}

	// paint the text on the backbuffer
	AlphaBltSubPic(rSrcPri.Size());

	// Casimir666 : show OSD
	if (m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_UPDATE) {
		CAutoLock BitMapLock(&m_VMR9AlphaBitmapLock);
		CRect		rcSrc (m_VMR9AlphaBitmap.rSrc);
		m_pOSDTexture	= NULL;
		m_pOSDSurface	= NULL;
		if ((m_VMR9AlphaBitmap.dwFlags & VMRBITMAP_DISABLE) == 0 && (BYTE *)m_VMR9AlphaBitmapData) {
			if( (m_pD3DXLoadSurfaceFromMemory != NULL) &&
					SUCCEEDED(hr = m_pD3DDev->CreateTexture(rcSrc.Width(), rcSrc.Height(), 1,
								   D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
								   D3DPOOL_DEFAULT, &m_pOSDTexture, NULL)) ) {
				if (SUCCEEDED (hr = m_pOSDTexture->GetSurfaceLevel(0, &m_pOSDSurface))) {
					hr = m_pD3DXLoadSurfaceFromMemory (m_pOSDSurface,
													   NULL,
													   NULL,
													   (BYTE *)m_VMR9AlphaBitmapData,
													   D3DFMT_A8R8G8B8,
													   m_VMR9AlphaBitmapWidthBytes,
													   NULL,
													   &m_VMR9AlphaBitmapRect,
													   D3DX_FILTER_NONE,
													   m_VMR9AlphaBitmap.clrSrcKey);
				}
				if (FAILED (hr)) {
					m_pOSDTexture	= NULL;
					m_pOSDSurface	= NULL;
				}
			}
		}
		m_VMR9AlphaBitmap.dwFlags ^= VMRBITMAP_UPDATE;

	}

	if (pApp->m_fDisplayStats) {
		DrawStats();
	}

	if (m_pOSDTexture) {
		AlphaBlt(rSrcPri, rDstPri, m_pOSDTexture);
	}

	m_pD3DDev->EndScene();

	BOOL bCompositionEnabled = m_bCompositionEnabled;

	bool bDoVSyncInPresent = (!bCompositionEnabled && !m_bAlternativeVSync) || !s.m_RenderSettings.iVMR9VSync;

	LONGLONG PresentWaitTime = 0;
	/*
		if(fAll && m_fVMRSyncFix && bDoVSyncInPresent)
			{
				LONGLONG llPerf = pApp->GetPerfCounter();
				D3DLOCKED_RECT lr;
				if(SUCCEEDED(pBackBuffer->LockRect(&lr, NULL, 0)))
					pBackBuffer->UnlockRect();
				PresentWaitTime = pApp->GetPerfCounter() - llPerf;
			}
	*/

	CComPtr<IDirect3DQuery9> pEventQuery;

	m_pD3DDev->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);
	if (pEventQuery) {
		pEventQuery->Issue(D3DISSUE_END);
	}

	if (s.m_RenderSettings.iVMRFlushGPUBeforeVSync && pEventQuery) {
		LONGLONG llPerf = pApp->GetPerfCounter();
		BOOL Data;
		//Sleep(5);
		LONGLONG FlushStartTime = pApp->GetPerfCounter();
		while(S_FALSE == pEventQuery->GetData( &Data, sizeof(Data), D3DGETDATA_FLUSH )) {
			if (!s.m_RenderSettings.iVMRFlushGPUWait) {
				break;
			}
			Sleep(1);
			if (pApp->GetPerfCounter() - FlushStartTime > 500000) {
				break;    // timeout after 50 ms
			}
		}
		if (s.m_RenderSettings.iVMRFlushGPUWait) {
			m_WaitForGPUTime = pApp->GetPerfCounter() - llPerf;
		} else {
			m_WaitForGPUTime = 0;
		}
	} else {
		m_WaitForGPUTime = 0;
	}

	if (fAll) {
		m_PaintTime = (GetRenderersData()->GetPerfCounter() - StartPaint);
		m_PaintTimeMin = min(m_PaintTimeMin, m_PaintTime);
		m_PaintTimeMax = max(m_PaintTimeMax, m_PaintTime);
	}

	bool bWaited = false;
	bool bTakenLock = false;
	if (fAll) {
		// Only sync to refresh when redrawing all
		bool bTest = WaitForVBlank(bWaited, bTakenLock);
		ASSERT(bTest == bDoVSyncInPresent);
		if (!bDoVSyncInPresent) {
			LONGLONG Time = pApp->GetPerfCounter();
			OnVBlankFinished(fAll, Time);
			if (!m_bIsEVR || m_OrderedPaint) {
				CalculateJitter(Time);
			}
		}
	}

	// Create a device pointer m_pd3dDevice

	// Create a query object
	{
		CComPtr<IDirect3DQuery9> pEventQuery;
		m_pD3DDev->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery);

		LONGLONG llPerf = pApp->GetPerfCounter();
		if (m_pD3DDevEx) {
			if (m_bIsFullscreen) {
				hr = m_pD3DDevEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
			} else {
				hr = m_pD3DDevEx->PresentEx(rSrcPri, rDstPri, NULL, NULL, NULL);
			}
		} else {
			if (m_bIsFullscreen) {
				hr = m_pD3DDev->Present(NULL, NULL, NULL, NULL);
			} else {
				hr = m_pD3DDev->Present(rSrcPri, rDstPri, NULL, NULL);
			}
		}
		// Issue an End event
		if (pEventQuery) {
			pEventQuery->Issue(D3DISSUE_END);
		}

		BOOL Data;

		if (s.m_RenderSettings.iVMRFlushGPUAfterPresent && pEventQuery) {
			LONGLONG FlushStartTime = pApp->GetPerfCounter();
			while (S_FALSE == pEventQuery->GetData( &Data, sizeof(Data), D3DGETDATA_FLUSH )) {
				if (!s.m_RenderSettings.iVMRFlushGPUWait) {
					break;
				}
				if (pApp->GetPerfCounter() - FlushStartTime > 500000) {
					break;    // timeout after 50 ms
				}
			}
		}

		int ScanLine;
		int bInVBlank;
		GetVBlank(ScanLine, bInVBlank, false);

		if (fAll && (!m_bIsEVR || m_OrderedPaint)) {
			m_VBlankEndPresent = ScanLine;
		}

		while (ScanLine == 0 || bInVBlank) {
			GetVBlank(ScanLine, bInVBlank, false);

		}
		m_VBlankStartMeasureTime = pApp->GetPerfCounter();
		m_VBlankStartMeasure = ScanLine;

		if (fAll && bDoVSyncInPresent) {
			m_PresentWaitTime = (pApp->GetPerfCounter() - llPerf) + PresentWaitTime;
			m_PresentWaitTimeMin = min(m_PresentWaitTimeMin, m_PresentWaitTime);
			m_PresentWaitTimeMax = max(m_PresentWaitTimeMax, m_PresentWaitTime);
		} else {
			m_PresentWaitTime = 0;
			m_PresentWaitTimeMin = min(m_PresentWaitTimeMin, m_PresentWaitTime);
			m_PresentWaitTimeMax = max(m_PresentWaitTimeMax, m_PresentWaitTime);
		}
	}

	if (bDoVSyncInPresent) {
		LONGLONG Time = pApp->GetPerfCounter();
		if (!m_bIsEVR || m_OrderedPaint) {
			CalculateJitter(Time);
		}
		OnVBlankFinished(fAll, Time);
	}

	if (bTakenLock) {
		UnlockD3DDevice();
	}

	/*if (!bWaited)
	{
		bWaited = true;
		WaitForVBlank(bWaited);
		TRACE("Double VBlank\n");
		ASSERT(bWaited);
		if (!bDoVSyncInPresent)
		{
			CalculateJitter();
			OnVBlankFinished(fAll);
		}
	}*/

	if (!m_bPendingResetDevice) {
		bool fResetDevice = false;

		if(hr == D3DERR_DEVICELOST && m_pD3DDev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
			TRACE("Reset Device: D3D Device Lost\n");
			fResetDevice = true;
		}

		//if(hr == S_PRESENT_MODE_CHANGED)
		//{
		//	TRACE("Reset Device: D3D Device mode changed\n");
		//	fResetDevice = true;
		//}

		if (SettingsNeedResetDevice()) {
			TRACE("Reset Device: settings changed\n");
			fResetDevice = true;
		}

		bCompositionEnabled = false;
		if (m_pDwmIsCompositionEnabled) {
			m_pDwmIsCompositionEnabled(&bCompositionEnabled);
		}
		if ((bCompositionEnabled != 0) != m_bCompositionEnabled) {
			if (m_bIsFullscreen) {
				m_bCompositionEnabled = (bCompositionEnabled != 0);
			} else {
				TRACE("Reset Device: DWM composition changed\n");
				fResetDevice = true;
			}
		}

		if(s.fResetDevice) {
			LONGLONG time = GetRenderersData()->GetPerfCounter();
			if (time > m_LastAdapterCheck + 20000000) { // check every 2 sec.
				m_LastAdapterCheck = time;
#ifdef _DEBUG
				D3DDEVICE_CREATION_PARAMETERS Parameters;
				if(SUCCEEDED(m_pD3DDev->GetCreationParameters(&Parameters))) {
					ASSERT(Parameters.AdapterOrdinal == m_CurrentAdapter);
				}
#endif
				if(m_CurrentAdapter != GetAdapter(m_pD3D)) {
					TRACE("Reset Device: D3D adapter changed\n");
					fResetDevice = true;
				}
#ifdef _DEBUG
				else {
					ASSERT(m_pD3D->GetAdapterMonitor(m_CurrentAdapter) == m_pD3D->GetAdapterMonitor(GetAdapter(m_pD3D)));
				}
#endif
			}
		}

		if(fResetDevice) {
			m_bPendingResetDevice = true;
			SendResetRequest();
		}
	}

	if (m_OrderedPaint) {
		--m_OrderedPaint;
	} else {
		//if (m_bIsEVR)
		//	TRACE("UNORDERED PAINT!!!!!!\n");
	}
	return true;
}

double CDX9AllocatorPresenter::GetFrameTime()
{
	if (m_DetectedLock) {
		return m_DetectedFrameTime;
	}

	return m_rtTimePerFrame / 10000000.0;
}

double CDX9AllocatorPresenter::GetFrameRate()
{
	if (m_DetectedLock) {
		return m_DetectedFrameRate;
	}

	return 10000000.0 / m_rtTimePerFrame;
}

void CDX9AllocatorPresenter::SendResetRequest()
{
	if (!m_bDeviceResetRequested) {
		m_bDeviceResetRequested = true;
		AfxGetApp()->m_pMainWnd->PostMessage(WM_RESET_DEVICE);
	}
}

STDMETHODIMP_(bool) CDX9AllocatorPresenter::ResetDevice()
{
	TRACE("ResetDevice\n");
	_ASSERT(m_MainThreadId == GetCurrentThreadId());
	StopWorkerThreads();
	
	// In VMR-9 deleting the surfaces before we are told to is bad !
	// Can't comment out this because CDX9AllocatorPresenter is used by EVR Custom
	// Why is EVR using a presenter for DX9 anyway ?!
	DeleteSurfaces();

	HRESULT hr;
	CString Error;
	// TODO: Report error messages here

	// In VMR-9 'AllocSurfaces' call is redundant afaik because
	// 'CreateDevice' calls 'm_pIVMRSurfAllocNotify->ChangeD3DDevice' which in turn calls
	// 'CVMR9AllocatorPresenter::InitializeDevice' which calls 'AllocSurfaces'
	if(FAILED(hr = CreateDevice(Error)) || FAILED(hr = AllocSurfaces())) {
		// TODO: We should probably pause player
#ifdef _DEBUG
		Error += GetWindowsErrorMessage(hr, NULL);
		TRACE("D3D Reset Error\n%ws\n\n", Error.GetBuffer());
#endif
		m_bDeviceResetRequested = false;
		return false;
	}
	OnResetDevice();
	m_bDeviceResetRequested = false;
	m_bPendingResetDevice = false;

	return true;
}

STDMETHODIMP_(bool) CDX9AllocatorPresenter::DisplayChange()
{
	SendResetRequest();
	return true;
}

void CDX9AllocatorPresenter::DrawText(const RECT &rc, const CString &strText, int _Priority)
{
	if (_Priority < 1) {
		return;
	}
	int Quality = 1;
	D3DXCOLOR Color1( 1.0f, 0.2f, 0.2f, 1.0f );
	D3DXCOLOR Color0( 0.0f, 0.0f, 0.0f, 1.0f );
	RECT Rect1 = rc;
	RECT Rect2 = rc;
	if (Quality == 1) {
		OffsetRect (&Rect2 , 2, 2);
	} else {
		OffsetRect (&Rect2 , -1, -1);
	}
	if (Quality > 0) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , 1, 0);
	if (Quality > 3) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , 1, 0);
	if (Quality > 2) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , 0, 1);
	if (Quality > 3) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , 0, 1);
	if (Quality > 1) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , -1, 0);
	if (Quality > 3) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , -1, 0);
	if (Quality > 2) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	OffsetRect (&Rect2 , 0, -1);
	if (Quality > 3) {
		m_pFont->DrawText( m_pSprite, strText, -1, &Rect2, DT_NOCLIP, Color0);
	}
	m_pFont->DrawText( m_pSprite, strText, -1, &Rect1, DT_NOCLIP, Color1);
}

void CDX9AllocatorPresenter::DrawStats()
{
	CRenderersSettings& s = GetRenderersSettings();
	CRenderersData * pApp = GetRenderersData();
	int bDetailedStats = 2;
	switch (pApp->m_fDisplayStats) {
		case 1:
			bDetailedStats = 2;
			break;
		case 2:
			bDetailedStats = 1;
			break;
		case 3:
			bDetailedStats = 0;
			break;
	}

	LONGLONG		llMaxJitter = m_MaxJitter;
	LONGLONG		llMinJitter = m_MinJitter;
	LONGLONG		llMaxSyncOffset = m_MaxSyncOffset;
	LONGLONG		llMinSyncOffset = m_MinSyncOffset;
	RECT			rc = {40, 40, 0, 0 };
	if (m_pFont && m_pSprite) {
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		CString		strText;
		int TextHeight = 25.0*m_TextScale + 0.5;
		//		strText.Format(L"Frame rate   : %7.03f   (%7.3f ms = %.03f, %s)   (%7.3f ms = %.03f%s)    Clock: %7.3f ms %+1.4f %%  %+1.9f  %+1.9f", m_fAvrFps, double(m_rtTimePerFrame) / 10000.0, 10000000.0 / (double)(m_rtTimePerFrame), m_bInterlaced ? L"I" : L"P", GetFrameTime() * 1000.0, GetFrameRate(), m_DetectedLock ? L" L" : L"", m_ClockDiff/10000.0, m_ModeratedTimeSpeed*100.0 - 100.0, m_ModeratedTimeSpeedDiff, m_ClockDiffCalc/10000.0);
		if (bDetailedStats > 1) {
			if (m_bIsEVR) {
				strText.Format(L"Frame rate   : %7.03f   (%7.3f ms = %.03f, %s)   (%7.3f ms = %.03f%s, %2.03f StdDev)  Clock: %1.4f %%", m_fAvrFps, double(m_rtTimePerFrame) / 10000.0, 10000000.0 / (double)(m_rtTimePerFrame), m_bInterlaced ? L"I" : L"P", GetFrameTime() * 1000.0, GetFrameRate(), m_DetectedLock ? L" L" : L"", m_DetectedFrameTimeStdDev / 10000.0, m_ModeratedTimeSpeed*100.0);
			} else {
				strText.Format(L"Frame rate   : %7.03f   (%7.3f ms = %.03f, %s)", m_fAvrFps, double(m_rtTimePerFrame) / 10000.0, 10000000.0 / (double)(m_rtTimePerFrame), m_bInterlaced ? L"I" : L"P");
			}
		}
		//			strText.Format(L"Frame rate   : %7.03f   (%7.3f ms = %.03f, %s)   (%7.3f ms = %.03f%s, %2.03f StdDev)", m_fAvrFps, double(m_rtTimePerFrame) / 10000.0, 10000000.0 / (double)(m_rtTimePerFrame), m_bInterlaced ? L"I" : L"P", GetFrameTime() * 1000.0, GetFrameRate(), m_DetectedLock ? L" L" : L"", m_DetectedFrameTimeStdDev / 10000.0);
		else {
			strText.Format(L"Frame rate   : %7.03f   (%.03f%s)", m_fAvrFps, GetFrameRate(), m_DetectedLock ? L" L" : L"");
		}
		DrawText(rc, strText, 1);
		OffsetRect (&rc, 0, TextHeight);

		if (bDetailedStats > 1) {
			strText.Format(L"Settings     : ");

			if (m_bIsEVR) {
				strText += "EVR ";
			} else {
				strText += "VMR9 ";
			}

			if (m_bIsFullscreen) {
				strText += "FS ";
			}
			if (s.m_RenderSettings.iVMR9FullscreenGUISupport) {
				strText += "FSGui ";
			}

			if (s.m_RenderSettings.iVMRDisableDesktopComposition) {
				strText += "DisDC ";
			}

			if (m_bColorManagement) {
				strText += "ColorMan ";
			}

			if (s.m_RenderSettings.iVMRFlushGPUBeforeVSync) {
				strText += "GPUFlushBV ";
			}
			if (s.m_RenderSettings.iVMRFlushGPUAfterPresent) {
				strText += "GPUFlushAP ";
			}

			if (s.m_RenderSettings.iVMRFlushGPUWait) {
				strText += "GPUFlushWt ";
			}

			if (s.m_RenderSettings.iVMR9VSync) {
				strText += "VS ";
			}
			if (s.m_RenderSettings.fVMR9AlterativeVSync) {
				strText += "AltVS ";
			}
			if (s.m_RenderSettings.iVMR9VSyncAccurate) {
				strText += "AccVS ";
			}
			if (s.m_RenderSettings.iVMR9VSyncOffset) {
				strText.AppendFormat(L"VSOfst(%d)", s.m_RenderSettings.iVMR9VSyncOffset);
			}

			if (m_bFullFloatingPointProcessing) {
				strText += "FullFP ";
			}

			if (m_bHalfFloatingPointProcessing) {
				strText += "HalfFP ";
			}

			if (m_bIsEVR) {
				if (m_bHighColorResolution) {
					strText += "10bitOut ";
				}
				if (m_bForceInputHighColorResolution) {
					strText += "For10bitIn ";
				}
				if (s.m_RenderSettings.iEVREnableFrameTimeCorrection) {
					strText += "FTC ";
				}
				if (s.m_RenderSettings.iEVROutputRange == 0) {
					strText += "0-255 ";
				} else if (s.m_RenderSettings.iEVROutputRange == 1) {
					strText += "16-235 ";
				}
			}


			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);

		}

		if (bDetailedStats > 1) {
			strText.Format(L"Formats      : Surface %s    Backbuffer %s    Display %s     Device %s      D3DExError: %s", GetD3DFormatStr(m_SurfaceType), GetD3DFormatStr(m_BackbufferType), GetD3DFormatStr(m_DisplayType), m_pD3DDevEx ? L"D3DDevEx" : L"D3DDev", m_D3DDevExError.GetString());
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);

			if (m_bIsEVR) {
				strText.Format(L"Refresh rate : %.05f Hz    SL: %4d     (%3d Hz)      Last Duration: %10.6f      Corrected Frame Time: %s", m_DetectedRefreshRate, int(m_DetectedScanlinesPerFrame + 0.5), m_RefreshRate, double(m_LastFrameDuration)/10000.0, m_bCorrectedFrameTime?L"Yes":L"No");
				DrawText(rc, strText, 1);
				OffsetRect (&rc, 0, TextHeight);
			}
		}

		if (m_bSyncStatsAvailable) {
			if (bDetailedStats > 1) {
				strText.Format(L"Sync offset  : Min = %+8.3f ms, Max = %+8.3f ms, StdDev = %7.3f ms, Avr = %7.3f ms, Mode = %d", (double(llMinSyncOffset)/10000.0), (double(llMaxSyncOffset)/10000.0), m_fSyncOffsetStdDev/10000.0, m_fSyncOffsetAvr/10000.0, m_VSyncMode);
			} else {
				strText.Format(L"Sync offset  : Mode = %d", m_VSyncMode);
			}
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
		}

		if (bDetailedStats > 1) {
			strText.Format(L"Jitter       : Min = %+8.3f ms, Max = %+8.3f ms, StdDev = %7.3f ms", (double(llMinJitter)/10000.0), (double(llMaxJitter)/10000.0), m_fJitterStdDev/10000.0);
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
		}

		if (m_pAllocator && bDetailedStats > 1) {
			CDX9SubPicAllocator *pAlloc = (CDX9SubPicAllocator *)m_pAllocator.p;
			int nFree = 0;
			int nAlloc = 0;
			int nSubPic = 0;
			REFERENCE_TIME QueueStart = 0;
			REFERENCE_TIME QueueEnd = 0;

			if (m_pSubPicQueue) {
				REFERENCE_TIME QueueNow = 0;
				m_pSubPicQueue->GetStats(nSubPic, QueueNow, QueueStart, QueueEnd);

				if (QueueStart) {
					QueueStart -= QueueNow;
				}

				if (QueueEnd) {
					QueueEnd -= QueueNow;
				}
			}

			pAlloc->GetStats(nFree, nAlloc);
			strText.Format(L"Subtitles    : Free %d     Allocated %d     Buffered %d     QueueStart %7.3f     QueueEnd %7.3f", nFree, nAlloc, nSubPic, (double(QueueStart)/10000000.0), (double(QueueEnd)/10000000.0));
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
		}

		if (bDetailedStats > 1) {
			if (m_VBlankEndPresent == -100000) {
				strText.Format(L"VBlank Wait  : Start %4d   End %4d   Wait %7.3f ms   Lock %7.3f ms   Offset %4d   Max %4d", m_VBlankStartWait, m_VBlankEndWait, (double(m_VBlankWaitTime)/10000.0), (double(m_VBlankLockTime)/10000.0), m_VBlankMin, m_VBlankMax - m_VBlankMin);
			} else {
				strText.Format(L"VBlank Wait  : Start %4d   End %4d   Wait %7.3f ms   Lock %7.3f ms   Offset %4d   Max %4d   EndPresent %4d", m_VBlankStartWait, m_VBlankEndWait, (double(m_VBlankWaitTime)/10000.0), (double(m_VBlankLockTime)/10000.0), m_VBlankMin, m_VBlankMax - m_VBlankMin, m_VBlankEndPresent);
			}
		} else {
			if (m_VBlankEndPresent == -100000) {
				strText.Format(L"VBlank Wait  : Start %4d   End %4d", m_VBlankStartWait, m_VBlankEndWait);
			} else {
				strText.Format(L"VBlank Wait  : Start %4d   End %4d   EP %4d", m_VBlankStartWait, m_VBlankEndWait, m_VBlankEndPresent);
			}
		}
		DrawText(rc, strText, 1);
		OffsetRect (&rc, 0, TextHeight);

		BOOL bCompositionEnabled = m_bCompositionEnabled;

		bool bDoVSyncInPresent = (!bCompositionEnabled && !m_bAlternativeVSync) || !s.m_RenderSettings.iVMR9VSync;

		if (bDetailedStats > 1 && bDoVSyncInPresent) {
			strText.Format(L"Present Wait : Wait %7.3f ms   Min %7.3f ms   Max %7.3f ms", (double(m_PresentWaitTime)/10000.0), (double(m_PresentWaitTimeMin)/10000.0), (double(m_PresentWaitTimeMax)/10000.0));
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
		}

		if (bDetailedStats > 1) {
			if (m_WaitForGPUTime) {
				strText.Format(L"Paint Time   : Draw %7.3f ms   Min %7.3f ms   Max %7.3f ms   GPU %7.3f ms", (double(m_PaintTime-m_WaitForGPUTime)/10000.0), (double(m_PaintTimeMin)/10000.0), (double(m_PaintTimeMax)/10000.0), (double(m_WaitForGPUTime)/10000.0));
			} else {
				strText.Format(L"Paint Time   : Draw %7.3f ms   Min %7.3f ms   Max %7.3f ms", (double(m_PaintTime-m_WaitForGPUTime)/10000.0), (double(m_PaintTimeMin)/10000.0), (double(m_PaintTimeMax)/10000.0));
			}
		} else {
			if (m_WaitForGPUTime) {
				strText.Format(L"Paint Time   : Draw %7.3f ms   GPU %7.3f ms", (double(m_PaintTime - m_WaitForGPUTime)/10000.0), (double(m_WaitForGPUTime)/10000.0));
			} else {
				strText.Format(L"Paint Time   : Draw %7.3f ms", (double(m_PaintTime - m_WaitForGPUTime)/10000.0));
			}
		}
		DrawText(rc, strText, 2);
		OffsetRect (&rc, 0, TextHeight);

		if (bDetailedStats > 1) {
			strText.Format(L"Raster Status: Wait %7.3f ms   Min %7.3f ms   Max %7.3f ms", (double(m_RasterStatusWaitTime)/10000.0), (double(m_RasterStatusWaitTimeMin)/10000.0), (double(m_RasterStatusWaitTimeMax)/10000.0));
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
		}

		if (bDetailedStats > 1) {
			if (m_bIsEVR) {
				strText.Format(L"Buffering    : Buffered %3d    Free %3d    Current Surface %3d", m_nUsedBuffer, m_nNbDXSurface - m_nUsedBuffer, m_nCurSurface, m_nVMR9Surfaces, m_iVMR9Surface);
			} else {
				strText.Format(L"Buffering    : VMR9Surfaces %3d   VMR9Surface %3d", m_nVMR9Surfaces, m_iVMR9Surface);
			}
		} else {
			strText.Format(L"Buffered     : %3d", m_nUsedBuffer);
		}
		DrawText(rc, strText, 1);
		OffsetRect (&rc, 0, TextHeight);

		if (bDetailedStats > 1) {
			strText.Format(L"Video size   : %d x %d  (AR = %d : %d)", m_NativeVideoSize.cx, m_NativeVideoSize.cy, m_AspectRatio.cx, m_AspectRatio.cy);
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
			if (m_pVideoTexture[0] || m_pVideoSurface[0]) {
				D3DSURFACE_DESC desc;
				if (m_pVideoTexture[0]) {
					m_pVideoTexture[0]->GetLevelDesc(0, &desc);
				} else if (m_pVideoSurface[0]) {
					m_pVideoSurface[0]->GetDesc(&desc);
				}

				if (desc.Width != (UINT)m_NativeVideoSize.cx || desc.Height != (UINT)m_NativeVideoSize.cy) {
					strText.Format(L"Texture size : %d x %d", desc.Width, desc.Height);
					DrawText(rc, strText, 1);
					OffsetRect (&rc, 0, TextHeight);
				}
			}


			strText.Format(L"%-13s: %s", GetDXVAVersion(), GetDXVADecoderDescription());
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);

			strText.Format(L"DirectX SDK  : %d", GetRenderersData()->GetDXSdkRelease());
			DrawText(rc, strText, 1);
			OffsetRect (&rc, 0, TextHeight);
			
			if(m_D3D9Device != _T("")) {
				strText = "Render device: " + m_D3D9Device;
				DrawText(rc, strText, 1);
				OffsetRect (&rc, 0, TextHeight);
			}

			if(m_Decoder != _T("")) {
				strText = "Decoder      : " + m_Decoder;
				DrawText(rc, strText, 1);
				OffsetRect (&rc, 0, TextHeight);
			}

			for (int i=0; i<6; i++) {
				if (m_strStatsMsg[i][0]) {
					DrawText(rc, m_strStatsMsg[i], 1);
					OffsetRect (&rc, 0, TextHeight);
				}
			}
		}
		m_pSprite->End();
		OffsetRect(&rc, 0, TextHeight); // Extra "line feed"
	}

	if (m_pLine && bDetailedStats) {
		D3DXVECTOR2		Points[NB_JITTER];
		int				nIndex;

		int StartX = 0;
		int StartY = 0;
		int ScaleX = 1;
		int ScaleY = 1;
		int DrawWidth = 625 * ScaleX + 50;
		int DrawHeight = 250 * ScaleY;
		int Alpha = 80;
		StartX = m_WindowRect.Width() - (DrawWidth + 20);
		StartY = m_WindowRect.Height() - (DrawHeight + 20);

		DrawRect(RGB(0,0,0), Alpha, CRect(StartX, StartY, StartX + DrawWidth, StartY + DrawHeight));
		// === Jitter Graduation
		//		m_pLine->SetWidth(2.2);          // Width
		//		m_pLine->SetAntialias(1);
		m_pLine->SetWidth(2.5);          // Width
		m_pLine->SetAntialias(1);
		//		m_pLine->SetGLLines(1);
		m_pLine->Begin();

		for (int i=10; i<250*ScaleY; i+= 10*ScaleY) {
			Points[0].x = (FLOAT)StartX;
			Points[0].y = (FLOAT)(StartY + i);
			Points[1].x = (FLOAT)(StartX + ((i-10)%40 ? 50 : 625 * ScaleX));
			Points[1].y = (FLOAT)(StartY + i);
			if (i == 130) {
				Points[1].x += 50;
			}
			m_pLine->Draw (Points, 2, D3DCOLOR_XRGB(100,100,255));
		}

		// === Jitter curve
		if (m_rtTimePerFrame) {
			for (int i=0; i<NB_JITTER; i++) {
				nIndex = (m_nNextJitter+1+i) % NB_JITTER;
				if (nIndex < 0) {
					nIndex += NB_JITTER;
				}
				double Jitter = m_pllJitter[nIndex] - m_fJitterMean;
				Points[i].x  = (FLOAT)(StartX + (i*5*ScaleX+5));
				Points[i].y  = (FLOAT)(StartY + ((Jitter*ScaleY)/5000.0 + 125.0* ScaleY));
			}
			m_pLine->Draw (Points, NB_JITTER, D3DCOLOR_XRGB(255,100,100));

			if (m_bSyncStatsAvailable) {
				for (int i=0; i<NB_JITTER; i++) {
					nIndex = (m_nNextSyncOffset+1+i) % NB_JITTER;
					if (nIndex < 0) {
						nIndex += NB_JITTER;
					}
					Points[i].x  = (FLOAT)(StartX + (i*5*ScaleX+5));
					Points[i].y  = (FLOAT)(StartY + ((m_pllSyncOffset[nIndex]*ScaleY)/5000 + 125*ScaleY));
				}
				m_pLine->Draw (Points, NB_JITTER, D3DCOLOR_XRGB(100,200,100));
			}
		}
		m_pLine->End();
	}

	// === Text

}

STDMETHODIMP CDX9AllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
	CheckPointer(size, E_POINTER);

	HRESULT hr;

	D3DSURFACE_DESC desc;
	memset(&desc, 0, sizeof(desc));
	m_pVideoSurface[m_nCurSurface]->GetDesc(&desc);

	DWORD required = sizeof(BITMAPINFOHEADER) + (desc.Width * desc.Height * 32 >> 3);
	if(!lpDib) {
		*size = required;
		return S_OK;
	}
	if(*size < required) {
		return E_OUTOFMEMORY;
	}
	*size = required;

	D3DLOCKED_RECT r;
	CComPtr<IDirect3DSurface9> pSurface;
	if(m_bFullFloatingPointProcessing || m_bHalfFloatingPointProcessing || m_bHighColorResolution) {
		CComPtr<IDirect3DSurface9> fSurface = m_pVideoSurface[m_nCurSurface];
		if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &fSurface, NULL))
			|| FAILED(hr = m_pD3DXLoadSurfaceFromSurface(fSurface, NULL, NULL, m_pVideoSurface[m_nCurSurface], NULL, NULL, D3DX_DEFAULT, 0))) return hr;
		pSurface = fSurface;
		if(FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY))) {
			pSurface = NULL;
			if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(desc.Width, desc.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL))
				|| FAILED(hr = m_pD3DDev->GetRenderTargetData(fSurface, pSurface))
				|| FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY))) return hr;}}
	else {
		pSurface = m_pVideoSurface[m_nCurSurface];
		if(FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY))) {
			pSurface = NULL;
			if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL))
				|| FAILED(hr = m_pD3DDev->GetRenderTargetData(m_pVideoSurface[m_nCurSurface], pSurface))
				|| FAILED(hr = pSurface->LockRect(&r, NULL, D3DLOCK_READONLY))) return hr;}}

	BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)lpDib;
	memset(bih, 0, sizeof(BITMAPINFOHEADER));
	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = desc.Width;
	bih->biHeight = desc.Height;
	bih->biBitCount = 32;
	bih->biPlanes = 1;
	bih->biSizeImage = bih->biWidth * bih->biHeight * bih->biBitCount >> 3;

	BitBltFromRGBToRGB(
		bih->biWidth, bih->biHeight,
		(BYTE*)(bih + 1), bih->biWidth*bih->biBitCount>>3, bih->biBitCount,
		(BYTE*)r.pBits + r.Pitch*(desc.Height-1), -(int)r.Pitch, 32);

	pSurface->UnlockRect();

	return S_OK;
}

STDMETHODIMP CDX9AllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
	return SetPixelShader2(pSrcData, pTarget, false);
}

STDMETHODIMP CDX9AllocatorPresenter::SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace)
{
	CAutoLock cRenderLock(&m_RenderLock);

	return SetCustomPixelShader(pSrcData, pTarget, bScreenSpace);
}
