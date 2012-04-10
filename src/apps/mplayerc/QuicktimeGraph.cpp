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
#include <math.h>
#include "QuicktimeGraph.h"
#include <IQTVideoSurface.h>
#include "mplayerc.h"
#include "MainFrm.h"
#include "../../DSUtil/DSUtil.h"

using namespace DSObjects;


//
// CQuicktimeGraph
//

using namespace QT;

CQuicktimeGraph::CQuicktimeGraph(HWND hWndParent, HRESULT& hr)
	: CBaseGraph()
	, m_wndDestFrame(this)
	, m_fQtInitialized(false)
{
	hr = S_OK;

	DWORD dwStyle = WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;

	AppSettings& s = AfxGetAppSettings();

	if (s.iQTVideoRendererType == VIDRNDT_QT_DX7) {
		if (SUCCEEDED(CreateAP7(CLSID_QT7AllocatorPresenter, hWndParent, &m_pQTAP))) {
			dwStyle &= ~WS_VISIBLE;
		}
	} else if (s.iQTVideoRendererType == VIDRNDT_QT_DX9) {
		bool bFullscreen = (AfxGetApp()->m_pMainWnd != NULL) && (((CMainFrame*)AfxGetApp()->m_pMainWnd)->IsD3DFullScreenMode());
		if (SUCCEEDED(CreateAP9(CLSID_QT9AllocatorPresenter, hWndParent, bFullscreen, &m_pQTAP))) {
			dwStyle &= ~WS_VISIBLE;
		}
	}

	m_fQtInitialized = false;
	if (InitializeQTML(0) != 0) {
		hr = E_FAIL;
		return;
	}
	if (EnterMovies() != 0) {
		TerminateQTML();
		hr = E_FAIL;
		return;
	}
	m_fQtInitialized = true;

	if (!m_wndWindowFrame.CreateEx(WS_EX_NOPARENTNOTIFY, NULL, NULL, dwStyle, CRect(0, 0, 0, 0), CWnd::FromHandle(hWndParent), 0)) {
		hr = E_FAIL;
		return;
	}

	if (!m_wndDestFrame.Create(NULL, NULL, dwStyle, CRect(0, 0, 0, 0), &m_wndWindowFrame, 0)) {
		hr = E_FAIL;
		return;
	}
}

CQuicktimeGraph::~CQuicktimeGraph()
{
	m_wndDestFrame.DestroyWindow();
	m_wndWindowFrame.DestroyWindow();

	if (m_fQtInitialized) {
		ExitMovies();
		TerminateQTML();
	}
}

STDMETHODIMP CQuicktimeGraph::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		QI(IVideoFrameStep)
		(m_pQTAP && (riid == __uuidof(ISubPicAllocatorPresenter) || riid == __uuidof(IQTVideoSurface))) ? m_pQTAP->QueryInterface(riid, ppv) :
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// IGraphBuilder
STDMETHODIMP CQuicktimeGraph::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
	bool fRet = m_wndDestFrame.OpenMovie(CString(lpcwstrFile));

	if (fRet) {
		for (int i = 1, cnt = GetMovieTrackCount(m_wndDestFrame.theMovie); i <= cnt; i++) {
			Track aTrack = GetMovieIndTrack(m_wndDestFrame.theMovie, i);
			Media aMedia = GetTrackMedia(aTrack);

			OSType aTrackType;
			GetMediaHandlerDescription(aMedia, &aTrackType, 0, 0);
			if (aTrackType == SoundMediaType) {
				SampleDescriptionHandle aDesc = (SampleDescriptionHandle)NewHandle(sizeof(aDesc));
				GetMediaSampleDescription(aMedia, 1, aDesc);
				if (GetMoviesError() == noErr) {
					SoundDescription& desc = **(SoundDescriptionHandle)aDesc;
					NotifyEvent(EC_BG_AUDIO_CHANGED, desc.numChannels, 0);
					i = cnt;
				}
				DisposeHandle((Handle)aDesc);
			}
		}
	}

	return fRet ? S_OK : E_FAIL;
}

// IMediaControl
STDMETHODIMP CQuicktimeGraph::Run()
{
	m_wndDestFrame.Run();
	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::Pause()
{
	m_wndDestFrame.Pause();
	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::Stop()
{
	m_wndDestFrame.Stop();
	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::GetState(LONG msTimeout, OAFilterState* pfs)
{
	// TODO: this seems to deadlock when opening from the net
	return pfs ? *pfs = m_wndDestFrame.GetState(), S_OK : E_POINTER;
}

// IMediaSeeking
STDMETHODIMP CQuicktimeGraph::GetDuration(LONGLONG* pDuration)
{
	CheckPointer(pDuration, E_POINTER);

	*pDuration = 0;

	if (!m_wndDestFrame.theMovie) {
		return E_UNEXPECTED;
	}

	TimeScale ts = GetMovieTimeScale(m_wndDestFrame.theMovie);
	if (ts == 0) {
		return E_FAIL;
	}

	*pDuration = 10000i64*GetMovieDuration(m_wndDestFrame.theMovie)/ts*1000;

	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::GetCurrentPosition(LONGLONG* pCurrent)
{
	CheckPointer(pCurrent, E_POINTER);

	*pCurrent = 0;

	if (!m_wndDestFrame.theMovie) {
		return E_UNEXPECTED;
	}

	TimeScale ts = GetMovieTimeScale(m_wndDestFrame.theMovie);
	if (ts == 0) {
		return E_FAIL;
	}

	TimeRecord tr;
	*pCurrent = 10000i64*GetMovieTime(m_wndDestFrame.theMovie, &tr)/ts*1000;

	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
	CheckPointer(pCurrent, E_POINTER);

	if (!(dwCurrentFlags&AM_SEEKING_AbsolutePositioning)) {
		return E_INVALIDARG;
	}

	if (!m_wndDestFrame.theMovie) {
		return E_UNEXPECTED;
	}

	TimeScale ts = GetMovieTimeScale(m_wndDestFrame.theMovie);
	if (ts == 0) {
		return E_FAIL;
	}

	SetMovieTimeValue(m_wndDestFrame.theMovie, (TimeValue)(*pCurrent*ts/1000/10000i64));

	if (!m_wndDestFrame.theMC) {
		UpdateMovie(m_wndDestFrame.theMovie);
		MoviesTask(m_wndDestFrame.theMovie, 0L);
	}

	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::SetRate(double dRate)
{
	return m_wndDestFrame.theMovie ? SetMovieRate(m_wndDestFrame.theMovie, (Fixed)(dRate * 0x10000)), S_OK : E_UNEXPECTED;
}

STDMETHODIMP CQuicktimeGraph::GetRate(double* pdRate)
{
	CheckPointer(pdRate, E_POINTER);
	*pdRate = 1.0;
	return m_wndDestFrame.theMovie ? *pdRate = (double)GetMovieRate(m_wndDestFrame.theMovie) / 0x10000, S_OK : E_UNEXPECTED;
}

// IVideoWindow
STDMETHODIMP CQuicktimeGraph::SetWindowPosition(long Left, long Top, long Width, long Height)
{
	if (IsWindow(m_wndWindowFrame.m_hWnd)) {
		m_wndWindowFrame.MoveWindow(Left, Top, Width, Height);
	}

	return S_OK;
}

// IBasicVideo
STDMETHODIMP CQuicktimeGraph::SetDestinationPosition(long Left, long Top, long Width, long Height)// {return E_NOTIMPL;}
{
	if (!m_pQTAP && IsWindow(m_wndDestFrame.m_hWnd)) {
		m_wndDestFrame.MoveWindow(Left, Top, Width, Height);

		if (m_wndDestFrame.theMC) {
			Rect bounds = {0,0,(short)Height,(short)Width};
			MCPositionController(m_wndDestFrame.theMC, &bounds, NULL, mcTopLeftMovie|mcScaleMovieToFit);
		}
	}

	return S_OK;
}

STDMETHODIMP CQuicktimeGraph::GetVideoSize(long* pWidth, long* pHeight)
{
	if (!pWidth || !pHeight) {
		return E_POINTER;
	}

	*pWidth = m_wndDestFrame.m_size.cx;
	*pHeight = m_wndDestFrame.m_size.cy;

	return S_OK;
}

// IBasicAudio
STDMETHODIMP CQuicktimeGraph::put_Volume(long lVolume)
{
	if (m_wndDestFrame.theMovie) {
		short volume = (lVolume <= -10000) ? 0 : (short)(pow(10.0, lVolume/4000.0)*256);
		volume = max(min(volume, 256), 0);
		SetMovieVolume(m_wndDestFrame.theMovie, volume);
		return S_OK;
	}

	return E_UNEXPECTED;
}

STDMETHODIMP CQuicktimeGraph::get_Volume(long* plVolume)
{
	CheckPointer(plVolume, E_POINTER);

	if (m_wndDestFrame.theMovie) {
		*plVolume = (long)GetMovieVolume(m_wndDestFrame.theMovie); // [?..256]
		if (*plVolume > 0) {
			*plVolume = min((long)(4000*log10(*plVolume/256.0f)), 0);
		} else {
			*plVolume = -10000;
		}
		return S_OK;
	}

	return E_UNEXPECTED;
}

// IVideoFrameStep
STDMETHODIMP CQuicktimeGraph::Step(DWORD dwFrames, IUnknown* pStepObject)
{
	if (pStepObject) {
		return E_INVALIDARG;
	}
	if (dwFrames == 0) {
		return S_OK;
	}
	if (!m_wndDestFrame.theMovie) {
		return E_UNEXPECTED;
	}

	// w/o m_wndDestFrame.theMC

	OSType myTypes[] = {VisualMediaCharacteristic};
	TimeValue myCurrTime = GetMovieTime(m_wndDestFrame.theMovie, NULL);
	Fixed theRate = (int)dwFrames > 0 ? 0x00010000 : 0xffff0000;

	for (int nSteps = abs((int)dwFrames); nSteps > 0; nSteps--) {
		TimeValue myNextTime;
		GetMovieNextInterestingTime(m_wndDestFrame.theMovie, nextTimeStep, 1, myTypes, myCurrTime, theRate, &myNextTime, NULL);
		if (GetMoviesError() != noErr) {
			return E_FAIL;
		}
		myCurrTime = myNextTime;
	}

	if (myCurrTime >= 0 && myCurrTime < GetMovieDuration(m_wndDestFrame.theMovie)) {
		SetMovieTimeValue(m_wndDestFrame.theMovie, myCurrTime);
		if (GetMoviesError() != noErr) {
			return E_FAIL;
		}
		// the rest is not needed when we also have m_wndDestFrame.theMC:
		UpdateMovie(m_wndDestFrame.theMovie);
		if (GetMoviesError() != noErr) {
			return E_FAIL;
		}
		MoviesTask(m_wndDestFrame.theMovie, 0L);
	}

	NotifyEvent(EC_STEP_COMPLETE);

	return S_OK;

	/*
		// w/ m_wndDestFrame.theMC

		short myStep = (short)(long)dwFrames;
		return noErr == MCDoAction(m_wndDestFrame.theMC, mcActionStep, (Ptr)myStep)
			? NotifyEvent(EC_STEP_COMPLETE), S_OK : E_FAIL;
	*/
}

STDMETHODIMP CQuicktimeGraph::CanStep(long bMultiple, IUnknown* pStepObject)
{
	return m_wndDestFrame.theMovie ? S_OK : S_FALSE;
}

STDMETHODIMP CQuicktimeGraph::CancelStep()
{
	return E_NOTIMPL;
}

// IGraphEngine
STDMETHODIMP_(engine_t) CQuicktimeGraph::GetEngine()
{
	return QuickTime;
}

//
// CQuicktimeWindow
//

CQuicktimeWindow::CQuicktimeWindow(CQuicktimeGraph* pGraph)
	: m_pGraph(pGraph)
	, theMovie(NULL)
	, theMC(NULL)
	, m_size(0, 0)
	, m_idEndPoller(0)
	, m_fs(State_Stopped)
	, m_offscreenGWorld(NULL)
{
}

void CQuicktimeWindow::ProcessMovieEvent(unsigned int message, unsigned int wParam, long lParam)
{
	if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST
			|| message >= WM_KEYFIRST && message <= WM_KEYLAST) {
		return;
	}

	// Convert the Windows event to a QTML event
	MSG				theMsg;
	EventRecord		macEvent;
	LONG			thePoints = GetMessagePos();

	theMsg.hwnd = m_hWnd;
	theMsg.message = message;
	theMsg.wParam = wParam;
	theMsg.lParam = lParam;
	theMsg.time = GetMessageTime();
	theMsg.pt.x = LOWORD(thePoints);
	theMsg.pt.y = HIWORD(thePoints);

	// tranlate a windows event to a mac event
	WinEventToMacEvent(&theMsg, &macEvent);

	// Pump messages as mac event
	MCIsPlayerEvent(theMC, (const EventRecord*)&macEvent);
}

LRESULT CQuicktimeWindow::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_ERASEBKGND) {
		LRESULT theResult = __super::WindowProc(message, wParam, lParam);
		ProcessMovieEvent(message, wParam, lParam);
		return theResult;
	} else {
		ProcessMovieEvent(message, wParam, lParam);
		return __super::WindowProc(message, wParam, lParam);
	}
}

OSErr CQuicktimeWindow::MyMovieDrawingCompleteProc(Movie theMovie, long refCon)
{
	CQuicktimeWindow* pQW = (CQuicktimeWindow*)refCon;
	if (!pQW) {
		return noErr;
	}

	CQuicktimeGraph* pGraph = pQW->m_pGraph;
	if (!pGraph) {
		return noErr;
	}

	if (CComQIPtr<IQTVideoSurface> pQTVS = (IUnknown*)(INonDelegatingUnknown*)pGraph) {
		BITMAP bm;
		pQW->m_bm.GetObject(sizeof(bm), &bm);
		bm.bmWidth = pQW->m_size.cx;
		pQTVS->DoBlt(bm);
	}
	/*
	else
	{
		pQW->Invalidate();
	}*/

	return(noErr);
}

bool CQuicktimeWindow::OpenMovie(CString fn)
{
	CloseMovie();

	CComQIPtr<IQTVideoSurface> pQTVS = (IUnknown*)(INonDelegatingUnknown*)m_pGraph;

	if (!pQTVS) {
		// Set the port
		SetGWorld((CGrafPtr)GetHWNDPort(m_hWnd), NULL);
	}

	if (fn.Find(_T("://")) > 0) {
		Handle myHandle = NULL;
		Size mySize = fn.GetLength()+1;
		myHandle = NewHandleClear(mySize);
		if (!myHandle) {
			return false;
		}

		BlockMove((LPSTR)(LPCSTR)CStringA(fn), *myHandle, mySize);

		OSErr err = NewMovieFromDataRef(&theMovie, newMovieActive, NULL, myHandle, URLDataHandlerSubType);

		DisposeHandle(myHandle);

		if (err != noErr) {
			return false;
		}
	} else {
		if (!(fn.GetLength() > 0 && fn.GetLength() < 255)) {
			return false;
		}

		CHAR buff[_MAX_PATH] = {0, 0};
#ifdef UNICODE
		WideCharToMultiByte(GetACP(), 0, fn, -1, buff+1, _MAX_PATH-1, 0, 0);
#else
		strcpy(buff+1, fn);
#endif
		buff[0] = strlen(buff+1);

		// Make a FSSpec with a pascal string filename
		FSSpec sfFile;
		FSMakeFSSpec(0, 0L, (BYTE*)buff, &sfFile);

		// Open the movie file
		short movieResFile;
		OSErr err = OpenMovieFile(&sfFile, &movieResFile, fsRdPerm);
		if (err == noErr) {
			err = NewMovieFromFile(&theMovie, movieResFile, 0, 0, newMovieActive, 0);
			CloseMovieFile(movieResFile);
		}
		if (err != noErr) {
			return false;
		}
	}

	Rect rect;
	GetMovieBox(theMovie, &rect);
	MacOffsetRect(&rect, -rect.left, -rect.top);
	SetMovieBox(theMovie, &rect);
	m_size.SetSize(rect.right - rect.left, rect.bottom - rect.top);

	Rect nrect;
	GetMovieNaturalBoundsRect(theMovie, &nrect);

	if (!pQTVS) {
		theMC = NewMovieController(theMovie, &rect, mcTopLeftMovie|mcNotVisible);
	} else if (m_size.cx > 0 && m_size.cy > 0) {
		SetMovieDrawingCompleteProc(theMovie,
									movieDrawingCallWhenChanged,//|movieDrawingCallAlways,
									MyMovieDrawingCompleteProc, (long)this);

		if (CDC* pDC = GetDC()) {
			m_dc.CreateCompatibleDC(pDC);
			ReleaseDC(pDC);

			struct {
				BITMAPINFOHEADER bmiHeader;
				long bmiColors[256];
			} bmi;

			memset(&bmi, 0, sizeof(bmi));

			//int bpp = m_dc.GetDeviceCaps(BITSPIXEL);
			bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biCompression	= BI_BITFIELDS/*BI_RGB*/;
			LONG pitch = (m_size.cx + 0xF) & ~0xF;
			bmi.bmiHeader.biWidth		= pitch;
			bmi.bmiHeader.biHeight		= -m_size.cy;
			bmi.bmiHeader.biPlanes		= 1;
			bmi.bmiHeader.biBitCount	= 32/*bpp*/;

			bmi.bmiColors[0] = /*bpp == 16 ? 0xf800 :*/ 0xff0000;
			bmi.bmiColors[1] = /*bpp == 16 ? 0x07e0 :*/ 0x00ff00;
			bmi.bmiColors[2] = /*bpp == 16 ? 0x001f :*/ 0x0000ff;

			void* bits;
			m_bm.Attach(CreateDIBSection(m_dc, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &bits, NULL, 0));

			QDErr err = NewGWorldFromHBITMAP(&m_offscreenGWorld, NULL, NULL, 0, m_bm.m_hObject, m_dc.m_hDC);
			UNUSED_ALWAYS(err);

			SetMovieGWorld(theMovie, m_offscreenGWorld, GetGWorldDevice(m_offscreenGWorld));

			BITMAP bm;
			m_bm.GetObject(sizeof(bm), &bm);
			bm.bmWidth = m_size.cx;
			pQTVS->BeginBlt(bm);
		}
	}

	return(theMovie != NULL);
}

void CQuicktimeWindow::CloseMovie()
{
	if (theMC) {
		DisposeMovieController(theMC), theMC = NULL;
	}
	if (theMovie) {
		DisposeMovie(theMovie), theMovie = NULL;
	}
	m_size.SetSize(0, 0);
	m_fs = State_Stopped;

	if (m_offscreenGWorld) {
		DisposeGWorld(m_offscreenGWorld), m_offscreenGWorld = NULL;
	}
	m_dc.DeleteDC();
	m_bm.DeleteObject();
}

void CQuicktimeWindow::Run()
{
	if (theMovie) {
		StartMovie(theMovie);
		if (!m_idEndPoller) {
			m_idEndPoller = SetTimer(1, 10, NULL);    // 10ms -> 100fps max
		}
	}

	m_fs = State_Running;
}

void CQuicktimeWindow::Pause()
{
	if (theMovie) {
		StopMovie(theMovie);
		if (m_idEndPoller) {
			KillTimer(m_idEndPoller), m_idEndPoller = 0;
		}
	}

	m_fs = State_Paused;
}

void CQuicktimeWindow::Stop()
{
	if (theMovie) {
		StopMovie(theMovie);
		GoToBeginningOfMovie(theMovie);
		if (m_idEndPoller) {
			KillTimer(m_idEndPoller), m_idEndPoller = 0;
		}
	}

	m_fs = State_Stopped;
}

FILTER_STATE CQuicktimeWindow::GetState()
{
	return m_fs;
}

BEGIN_MESSAGE_MAP(CQuicktimeWindow, CPlayerWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

int CQuicktimeWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	CComQIPtr<IQTVideoSurface> pQTVS = (IUnknown*)(INonDelegatingUnknown*)m_pGraph;

	if (!pQTVS) {
		// Create GrafPort <-> HWND association
		CreatePortAssociation(m_hWnd, NULL, 0);
	}

	return 0;
}

void CQuicktimeWindow::OnDestroy()
{
	CPlayerWindow::OnDestroy();

	// close any movies	before destroying PortAssocation
	CloseMovie();

	CComQIPtr<IQTVideoSurface> pQTVS = (IUnknown*)(INonDelegatingUnknown*)m_pGraph;

	if (!pQTVS) {
		// Destroy the view's GrafPort <-> HWND association
		if (m_hWnd)
			if (CGrafPtr windowPort = (CGrafPtr)GetHWNDPort(m_hWnd)) {
				DestroyPortAssociation(windowPort);
			}
	}
}

BOOL CQuicktimeWindow::OnEraseBkgnd(CDC* pDC)
{
	return m_fs != State_Stopped && theMovie ? TRUE : __super::OnEraseBkgnd(pDC);
}

void CQuicktimeWindow::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_idEndPoller && theMovie) {
		if (IsMovieDone(theMovie)) {
			Pause();
			m_pGraph->NotifyEvent(EC_COMPLETE);
		} else if (CComQIPtr<IQTVideoSurface> pQTVS = (IUnknown*)(INonDelegatingUnknown*)m_pGraph) {
			MoviesTask(theMovie, 0);
			/*
						long duration = 0, scale = 1000;
						OSErr err = QTGetTimeUntilNextTask(&duration, scale);

						// err is 0 but still doesn't seem to work... returns duration=0 always
						TRACE(_T("%d\n"), duration);
						KillTimer(m_idEndPoller);
						m_idEndPoller = SetTimer(m_idEndPoller, duration, NULL);
			*/
		}
	}

	__super::OnTimer(nIDEvent);
}
