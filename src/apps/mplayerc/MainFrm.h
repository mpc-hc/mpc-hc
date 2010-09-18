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

#pragma once

#include <atlbase.h>

#include "ChildView.h"
#include "PlayerSeekBar.h"
#include "PlayerToolBar.h"
#include "PlayerInfoBar.h"
#include "PlayerStatusBar.h"
#include "PlayerSubresyncBar.h"
#include "PlayerPlaylistBar.h"
#include "PlayerCaptureBar.h"
#include "PlayerNavigationBar.h"
#include "PlayerShaderEditorBar.h"
#include "EditListEditor.h"
#include "PPageSheet.h"
#include "PPageFileInfoSheet.h"
#include "OpenCapDeviceDlg.h"

#include "FileDropTarget.h"

#include "KeyProvider.h"

#include "../../SubPic/ISubPic.h"

#include "IGraphBuilder2.h"

#include "RealMediaGraph.h"
#ifdef _WIN64
// TODOX64 : add QuickTime support when available!
#pragma message ("No support for QuickTime for X64 yet!")
#else /* _WIN64 */
#include "QuicktimeGraph.h"
#endif /* _WIN64 */
#include "ShockwaveGraph.h"

#include <IChapterInfo.h>
#include <IKeyFrameInfo.h>
#include <IBufferInfo.h>

#include "WebServer.h"
#include <D3d9.h>
#include <Vmr9.h>
#include <evr.h>
#include <evr9.h>
#include "VMROSD.h"
#include "LcdSupport.h"
#include "MpcApi.h"
#include "../../filters/renderer/SyncClock/SyncClock.h"

class CFullscreenWnd;

enum {PM_NONE, PM_FILE, PM_DVD, PM_CAPTURE};


class OpenMediaData
{
public:
//	OpenMediaData() {}
	virtual ~OpenMediaData() {} // one virtual funct is needed to enable rtti
	CString title;
	CAtlList<CString> subs;
};

class OpenFileData : public OpenMediaData
{
public:
	OpenFileData() : rtStart(0) {}
	CAtlList<CString> fns;
	REFERENCE_TIME rtStart;
};

class OpenDVDData : public OpenMediaData
{
public:
//	OpenDVDData() {}
	CString path;
	CComPtr<IDvdState> pDvdState;
};

class OpenDeviceData : public OpenMediaData
{
public:
	OpenDeviceData()
	{
		vinput = vchannel = ainput = -1;
	}
	CStringW DisplayName[2];
	int vinput, vchannel, ainput;
};

class TunerScanData
{
public :
	ULONG		FrequencyStart;
	ULONG		FrequencyStop;
	ULONG		Bandwidth;
	HWND		Hwnd;
};

class CMainFrame;

class CGraphThread : public CWinThread
{
	CMainFrame* m_pMainFrame;

	DECLARE_DYNCREATE(CGraphThread);

public:
	CGraphThread() : m_pMainFrame(NULL) {}

	void SetMainFrame(CMainFrame* pMainFrame)
	{
		m_pMainFrame = pMainFrame;
	}

	BOOL InitInstance();
	int ExitInstance();

	enum {TM_EXIT=WM_APP, TM_OPEN, TM_CLOSE, TM_RESET, TM_TUNER_SCAN};
	DECLARE_MESSAGE_MAP()
	afx_msg void OnExit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOpen(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose(WPARAM wParam, LPARAM lParam);
	afx_msg void OnReset(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTunerScan(WPARAM wParam, LPARAM lParam);
};
/*
class CKeyFrameFinderThread : public CWinThread, public CCritSec
{
	DECLARE_DYNCREATE(CKeyFrameFinderThread);

public:
	CKeyFrameFinderThread() {}

	CUIntArray m_kfs; // protected by (CCritSec*)this

	BOOL InitInstance();
	int ExitInstance();

	enum {TM_EXIT=WM_APP, TM_INDEX, TM_BREAK};
	DECLARE_MESSAGE_MAP()
	afx_msg void OnExit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIndex(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBreak(WPARAM wParam, LPARAM lParam);
};
*/
interface ISubClock;

class CMainFrame : public CFrameWnd, public CDropTarget
{
	enum
	{
		TIMER_STREAMPOSPOLLER = 1,
		TIMER_STREAMPOSPOLLER2,
		TIMER_FULLSCREENCONTROLBARHIDER,
		TIMER_FULLSCREENMOUSEHIDER,
		TIMER_STATS,
		TIMER_LEFTCLICK,
		TIMER_STATUSERASER
	};

	friend class CPPageFileInfoSheet;
	friend class CPPageLogo;

	// TODO: wrap these graph objects into a class to make it look cleaner

	DWORD m_dwRegister;

	CComPtr<IGraphBuilder2> pGB;
	CComQIPtr<IMediaControl> pMC;
	CComQIPtr<IMediaEventEx> pME;
	CComQIPtr<IVideoWindow> pVW;
	CComQIPtr<IBasicVideo> pBV;
	CComQIPtr<IBasicAudio> pBA;
	CComQIPtr<IMediaSeeking> pMS;
	CComQIPtr<IVideoFrameStep> pFS;
	CComQIPtr<IQualProp, &IID_IQualProp> pQP;
	CComQIPtr<IBufferInfo> pBI;
	CComQIPtr<IAMOpenProgress> pAMOP;

	CComQIPtr<IDvdControl2> pDVDC;
	CComQIPtr<IDvdInfo2> pDVDI;

	CComPtr<ICaptureGraphBuilder2> pCGB;
	CStringW m_VidDispName, m_AudDispName;
	CComPtr<IBaseFilter> pVidCap, pAudCap;
	CComPtr<IAMVideoCompression> pAMVCCap, pAMVCPrev;
	CComPtr<IAMStreamConfig> pAMVSCCap, pAMVSCPrev, pAMASC;
	CComPtr<IAMCrossbar> pAMXBar;
	CComPtr<IAMTVTuner> pAMTuner;
	CComPtr<IAMDroppedFrames> pAMDF;

	CComPtr<ISubPicAllocatorPresenter> m_pCAP;
	CComPtr<ISubPicAllocatorPresenter2> m_pCAP2;

	void SetBalance(int balance);

	// subtitles

	CCritSec m_csSubLock;
	CInterfaceList<ISubStream> m_pSubStreams;
	CAtlList<int> m_iAudioStreams; // foxX uses this to keep a mapping of audio streams, in which they're ordered based by language user preference
	int m_iSubtitleSel; // if(m_iSubtitleSel&(1<<31)): disabled
	DWORD_PTR m_nSubtitleId;

	friend class CTextPassThruFilter;

	// windowing

	CRect m_lastWindowRect;
	CPoint m_lastMouseMove;

	CRect m_rcDesktop;

	void ShowControls(int nCS, bool fSave = true);

	void SetDefaultWindowRect(int iMonitor = 0);
	void RestoreDefaultWindowRect();
	void ZoomVideoWindow(double scale = -1);
	double GetZoomAutoFitScale();

	void SetAlwaysOnTop(int i);

	// dynamic menus

	void SetupOpenCDSubMenu();
	void SetupFiltersSubMenu();
	void SetupAudioSwitcherSubMenu();
	void SetupSubtitlesSubMenu();
	void SetupNavAudioSubMenu();
	void SetupNavSubtitleSubMenu();
	void SetupNavAngleSubMenu();
	void SetupNavChaptersSubMenu();
	void SetupFavoritesSubMenu();
	void SetupShadersSubMenu();
	void SetupRecentFilesSubMenu();

	IBaseFilter* FindSourceSelectableFilter();
	void SetupNavStreamSelectSubMenu(CMenu* pSub, UINT id, DWORD dwSelGroup);
	void OnNavStreamSelectSubMenu(UINT id, DWORD dwSelGroup);

	CMenu m_popupmain, m_popup;
	CMenu m_opencds;
	CMenu m_filters, m_subtitles, m_audios;
	CAutoPtrArray<CMenu> m_filterpopups;
	CMenu m_navangle;
	CMenu m_navchapters;
	CMenu m_favorites;
	CMenu m_shaders;
	CMenu m_recentfiles;

	CInterfaceArray<IUnknown, &IID_IUnknown> m_pparray;
	CInterfaceArray<IAMStreamSelect> m_ssarray;

	// chapters (file mode)
	CComPtr<IDSMChapterBag> m_pCB;
	void SetupChapters();

	//

	void SetupIViAudReg();

	void AddTextPassThruFilter();

	int m_nLoops;

	bool m_fCustomGraph;
	bool m_fRealMediaGraph, m_fShockwaveGraph, m_fQuicktimeGraph;

	CComPtr<ISubClock> m_pSubClock;

	int m_fFrameSteppingActive;
	int m_nStepForwardCount;
	REFERENCE_TIME m_rtStepForwardStart;
	int m_VolumeBeforeFrameStepping;

	bool m_fEndOfStream;

	LARGE_INTEGER m_LastSaveTime;

	bool m_fBuffering;

	bool m_fLiveWM;

	bool m_fUpdateInfoBar;

	void SendStatusMessage(CString msg, int nTimeOut);
	CString m_playingmsg, m_closingmsg;

	REFERENCE_TIME m_rtDurationOverride;

	CComPtr<IUnknown> m_pProv;

	void CleanGraph();

	CComPtr<IBaseFilter> pAudioDubSrc;

	void ShowOptions(int idPage = 0);

	bool GetDIB(BYTE** ppData, long& size, bool fSilent = false);
	void SaveDIB(LPCTSTR fn, BYTE* pData, long size);
	BOOL IsRendererCompatibleWithSaveImage();
	void SaveImage(LPCTSTR fn);
	void SaveThumbnails(LPCTSTR fn);

	//

	friend class CWebClientSocket;
	friend class CWebServer;
	CAutoPtr<CWebServer> m_pWebServer;
	int m_iPlaybackMode;
	ULONG m_lCurrentChapter;
	ULONG m_lChapterStartTime;

public:
	void StartWebServer(int nPort);
	void StopWebServer();

	CString GetStatusMessage();
	int GetPlaybackMode()
	{
		return m_iPlaybackMode;
	}
	void SetPlaybackMode(int iNewStatus);
	bool IsMuted()
	{
		return m_wndToolBar.GetVolume() == -10000;
	}
	int GetVolume()
	{
		return m_wndToolBar.m_volctrl.GetPos();
	}

public:
	CMainFrame();

	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	bool m_fFullScreen;
	bool m_fFirstFSAfterLaunchOnFS;
	bool m_fHideCursor;
	CMenu m_navaudio, m_navsubtitle;

	CComPtr<IBaseFilter> m_pRefClock; // Adjustable reference clock. GothSync
	CComPtr<ISyncClock> m_pSyncClock;

	bool IsFrameLessWindow()
	{
		return(m_fFullScreen || AfxGetAppSettings().fHideCaptionMenu);
	}
	bool IsCaptionMenuHidden()
	{
		return(!m_fFullScreen && AfxGetAppSettings().fHideCaptionMenu);
	}
	bool IsSomethingLoaded()
	{
		return((m_iMediaLoadState == MLS_LOADING || m_iMediaLoadState == MLS_LOADED) && !IsD3DFullScreenMode());
	}
	bool IsPlaylistEmpty()
	{
		return(m_wndPlaylistBar.GetCount() == 0);
	}
	bool IsInteractiveVideo()
	{
		return(AfxGetAppSettings().fIntRealMedia && m_fRealMediaGraph || m_fShockwaveGraph);
	}
	bool IsD3DFullScreenMode();

	CControlBar* m_pLastBar;

protected:
	int m_iMediaLoadState;

	bool m_fAudioOnly;
	dispmode m_dmBeforeFullscreen;
	CString m_LastOpenFile;
	HMONITOR m_LastWindow_HM;

	DVD_DOMAIN m_iDVDDomain;
	DWORD m_iDVDTitle;
	int m_iSpeedLevel;

	double m_ZoomX, m_ZoomY, m_PosX, m_PosY;
	int m_AngleX, m_AngleY, m_AngleZ;

// Operations
	bool OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD);
	void CloseMediaPrivate();
	void DoTunerScan(TunerScanData* pTSD);

	void SendNowPlayingToMSN();
	void SendNowPlayingTomIRC();

	CWnd *GetModalParent();

	void OpenCreateGraphObject(OpenMediaData* pOMD);
	void OpenFile(OpenFileData* pOFD);
	void OpenDVD(OpenDVDData* pODD);
	void OpenCapture(OpenDeviceData* pODD);
	HRESULT OpenBDAGraph();
	void OpenCustomizeGraph();
	void OpenSetupVideo();
	void OpenSetupAudio();
	void OpenSetupInfoBar();
	void OpenSetupStatsBar();
	void OpenSetupStatusBar();
	// void OpenSetupToolBar();
	void OpenSetupCaptureBar();
	void OpenSetupWindowTitle(CString fn = _T(""));
	void AutoChangeMonitorMode();

	bool GraphEventComplete();

	friend class CGraphThread;
	CGraphThread* m_pGraphThread;
	bool m_bOpenedThruThread;

	CAtlArray<REFERENCE_TIME> m_kfs;

	bool m_fOpeningAborted;

public:
	void OpenCurPlaylistItem(REFERENCE_TIME rtStart = 0);
	void OpenMedia(CAutoPtr<OpenMediaData> pOMD);
	bool ResetDevice();
	void CloseMedia();
	void StartTunerScan(CAutoPtr<TunerScanData> pTSD);
	void StopTunerScan();

	void AddCurDevToPlaylist();

	bool m_fTrayIcon;
	void ShowTrayIcon(bool fShow);
	void SetTrayTip(CString str);

	CSize GetVideoSize();
	void ToggleFullscreen(bool fToNearest, bool fSwitchScreenResWhenHasTo);
	void MoveVideoWindow(bool fShowStats = false);
	void RepaintVideo();
	void HideVideoWindow(bool fHide);

	OAFilterState GetMediaState();
	REFERENCE_TIME GetPos(), GetDur();
	void SeekTo(REFERENCE_TIME rt, bool fSeekToKeyFrame = false);

	// audio streams order functions
	void InsertAudioStream(const CComQIPtr<IAMStreamSelect> &pSS, int i);
	void SetupAudioStreams();
	// subtitle streams order function
	bool LoadSubtitle(CString fn, ISubStream **actualStream = NULL);

	void UpdateSubtitle(bool fDisplayMessage = false, bool fApplyDefStyle = false);
	void SetSubtitle(ISubStream* pSubStream, bool fApplyDefStyle = false);
	void ReplaceSubtitle(ISubStream* pSubStreamOld, ISubStream* pSubStreamNew);
	void InvalidateSubtitle(DWORD_PTR nSubtitleId = -1, REFERENCE_TIME rtInvalidate = -1);
	void ReloadSubtitle();

	void SetAudioTrackIdx(int index);
	void SetSubtitleTrackIdx(int index);

	// shaders
	CAtlList<CString> m_shaderlabels;
	CAtlList<CString> m_shaderlabelsScreenSpace;
	void SetShaders();
	void UpdateShaders(CString label);

	// capturing
	bool m_fCapturing;
	HRESULT BuildCapture(IPin* pPin, IBaseFilter* pBF[3], const GUID& majortype, AM_MEDIA_TYPE* pmt); // pBF: 0 buff, 1 enc, 2 mux, pmt is for 1 enc
	bool BuildToCapturePreviewPin(
		IBaseFilter* pVidCap, IPin** pVidCapPin, IPin** pVidPrevPin,
		IBaseFilter* pAudCap, IPin** pAudCapPin, IPin** pAudPrevPin);
	bool BuildGraphVideoAudio(int fVPreview, bool fVCapture, int fAPreview, bool fACapture);
	bool DoCapture(), StartCapture(), StopCapture();

	bool DoAfterPlaybackEvent();
	int SearchInDir(bool DirForward);

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

	// Dvb capture
	void DisplayCurrentChannelOSD();

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members

	CChildView m_wndView;

	CPlayerSeekBar m_wndSeekBar;
	CPlayerToolBar m_wndToolBar;
	CPlayerInfoBar m_wndInfoBar;
	CPlayerInfoBar m_wndStatsBar;
	CPlayerStatusBar m_wndStatusBar;
	CList<CControlBar*> m_bars;

	CPlayerSubresyncBar m_wndSubresyncBar;
	CPlayerPlaylistBar m_wndPlaylistBar;
	CPlayerCaptureBar m_wndCaptureBar;
	CPlayerNavigationBar m_wndNavigationBar;
	CPlayerShaderEditorBar m_wndShaderEditorBar;
	CEditListEditor m_wndEditListEditor;
	CList<CSizingControlBar*> m_dockingbars;

	CFileDropTarget m_fileDropTarget;
	// TODO
	DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	DROPEFFECT OnDropEx(COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	void OnDragLeave();
	DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);

	LPCTSTR GetRecentFile();

	friend class CPPagePlayback; // TODO
	friend class CMPlayerCApp; // TODO

	void LoadControlBar(CControlBar* pBar, UINT defDockBarID);
	void RestoreFloatingControlBars();
	void SaveControlBars();

// Generated message map functions

	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	afx_msg LRESULT OnTaskBarRestart(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyIcon(WPARAM, LPARAM);
	afx_msg LRESULT OnTaskBarThumbnailsCreate(WPARAM, LPARAM);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnDisplayChange();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRawInput(UINT nInputcode, HRAWINPUT hRawInput);

	afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnResetDevice(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRepaintRenderLess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnResumeFromState(WPARAM wParam, LPARAM lParam);

	BOOL OnButton(UINT id, UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnXButtonDown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnXButtonUp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnXButtonDblClk(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg LRESULT OnNcHitTest(CPoint point);

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

	BOOL OnMenu(CMenu* pMenu);
	afx_msg void OnMenuPlayerShort();
	afx_msg void OnMenuPlayerLong();
	afx_msg void OnMenuFilters();

	afx_msg void OnUpdatePlayerStatus(CCmdUI* pCmdUI);

	afx_msg void OnFilePostOpenmedia();
	afx_msg void OnUpdateFilePostOpenmedia(CCmdUI* pCmdUI);
	afx_msg void OnFilePostClosemedia();
	afx_msg void OnUpdateFilePostClosemedia(CCmdUI* pCmdUI);

	afx_msg void OnBossKey();

	afx_msg void OnStreamAudio(UINT nID);
	afx_msg void OnStreamSub(UINT nID);
	afx_msg void OnStreamSubOnOff();
	afx_msg void OnOgmAudio(UINT nID);
	afx_msg void OnOgmSub(UINT nID);
	afx_msg void OnDvdAngle(UINT nID);
	afx_msg void OnDvdAudio(UINT nID);
	afx_msg void OnDvdSub(UINT nID);
	afx_msg void OnDvdSubOnOff();


	// menu item handlers

	afx_msg void OnFileOpenQuick();
	afx_msg void OnFileOpenmedia();
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnFileOpendvd();
	afx_msg void OnFileOpendevice();
	afx_msg void OnFileOpenCD(UINT nID);
	afx_msg void OnFileReopen();
	afx_msg void OnDropFiles(HDROP hDropInfo); // no menu item
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveImage();
	afx_msg void OnFileSaveImageAuto();
	afx_msg void OnUpdateFileSaveImage(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveThumbnails();
	afx_msg void OnUpdateFileSaveThumbnails(CCmdUI* pCmdUI);
	afx_msg void OnFileLoadsubtitle();
	afx_msg void OnUpdateFileLoadsubtitle(CCmdUI* pCmdUI);
	afx_msg void OnFileSavesubtitle();
	afx_msg void OnUpdateFileSavesubtitle(CCmdUI* pCmdUI);
	afx_msg void OnFileISDBSearch();
	afx_msg void OnUpdateFileISDBSearch(CCmdUI* pCmdUI);
	afx_msg void OnFileISDBUpload();
	afx_msg void OnUpdateFileISDBUpload(CCmdUI* pCmdUI);
	afx_msg void OnFileISDBDownload();
	afx_msg void OnUpdateFileISDBDownload(CCmdUI* pCmdUI);
	afx_msg void OnFileProperties();
	afx_msg void OnUpdateFileProperties(CCmdUI* pCmdUI);
	afx_msg void OnFileClosePlaylist();
	afx_msg void OnFileCloseMedia(); // no menu item
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);

	afx_msg void OnViewCaptionmenu();
	afx_msg void OnViewNavigation();
	afx_msg void OnUpdateViewCaptionmenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewNavigation(CCmdUI* pCmdUI);
	afx_msg void OnViewControlBar(UINT nID);
	afx_msg void OnUpdateViewControlBar(CCmdUI* pCmdUI);
	afx_msg void OnViewSubresync();
	afx_msg void OnUpdateViewSubresync(CCmdUI* pCmdUI);
	afx_msg void OnViewPlaylist();
	afx_msg void OnUpdateViewPlaylist(CCmdUI* pCmdUI);
	afx_msg void OnViewEditListEditor();
	afx_msg void OnEDLIn();
	afx_msg void OnUpdateEDLIn(CCmdUI* pCmdUI);
	afx_msg void OnEDLOut();
	afx_msg void OnUpdateEDLOut(CCmdUI* pCmdUI);
	afx_msg void OnEDLNewClip();
	afx_msg void OnUpdateEDLNewClip(CCmdUI* pCmdUI);
	afx_msg void OnEDLSave();
	afx_msg void OnUpdateEDLSave(CCmdUI* pCmdUI);
	afx_msg void OnViewCapture();
	afx_msg void OnUpdateViewCapture(CCmdUI* pCmdUI);
	afx_msg void OnViewShaderEditor();
	afx_msg void OnUpdateViewShaderEditor(CCmdUI* pCmdUI);
	afx_msg void OnViewMinimal();
	afx_msg void OnUpdateViewMinimal(CCmdUI* pCmdUI);
	afx_msg void OnViewCompact();
	afx_msg void OnUpdateViewCompact(CCmdUI* pCmdUI);
	afx_msg void OnViewNormal();
	afx_msg void OnUpdateViewNormal(CCmdUI* pCmdUI);
	afx_msg void OnViewFullscreen();
	afx_msg void OnViewFullscreenSecondary();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom(UINT nID);
	afx_msg void OnUpdateViewZoom(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomAutoFit();
	afx_msg void OnViewDefaultVideoFrame(UINT nID);
	afx_msg void OnUpdateViewDefaultVideoFrame(CCmdUI* pCmdUI);
	afx_msg void OnViewSwitchVideoFrame();
	afx_msg void OnUpdateViewSwitchVideoFrame(CCmdUI* pCmdUI);
	afx_msg void OnViewKeepaspectratio();
	afx_msg void OnUpdateViewKeepaspectratio(CCmdUI* pCmdUI);
	afx_msg void OnViewCompMonDeskARDiff();
	afx_msg void OnUpdateViewCompMonDeskARDiff(CCmdUI* pCmdUI);
	afx_msg void OnViewPanNScan(UINT nID);
	afx_msg void OnUpdateViewPanNScan(CCmdUI* pCmdUI);
	afx_msg void OnViewPanNScanPresets(UINT nID);
	afx_msg void OnUpdateViewPanNScanPresets(CCmdUI* pCmdUI);
	afx_msg void OnViewRotate(UINT nID);
	afx_msg void OnUpdateViewRotate(CCmdUI* pCmdUI);
	afx_msg void OnViewAspectRatio(UINT nID);
	afx_msg void OnUpdateViewAspectRatio(CCmdUI* pCmdUI);
	afx_msg void OnViewAspectRatioNext();
	afx_msg void OnViewOntop(UINT nID);
	afx_msg void OnUpdateViewOntop(CCmdUI* pCmdUI);
	afx_msg void OnViewOptions();
	afx_msg void OnUpdateViewTearingTest(CCmdUI* pCmdUI);
	afx_msg void OnViewTearingTest();
	afx_msg void OnUpdateViewDisplayStats(CCmdUI* pCmdUI);
	afx_msg void OnViewResetStats();
	afx_msg void OnViewDisplayStatsSC();
	afx_msg void OnUpdateViewVSync(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewVSyncOffset(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewVSyncAccurate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFlushGPU(CCmdUI* pCmdUI);

	afx_msg void OnUpdateViewSynchronizeVideo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSynchronizeDisplay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSynchronizeNearest(CCmdUI* pCmdUI);

	afx_msg void OnUpdateViewD3DFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDisableDesktopComposition(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewAlternativeVSync(CCmdUI* pCmdUI);

	afx_msg void OnUpdateViewColorManagementEnable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewColorManagementInput(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewColorManagementAmbientLight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewColorManagementIntent(CCmdUI* pCmdUI);

	afx_msg void OnUpdateViewEVROutputRange(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFullscreenGUISupport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHighColorResolution(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewForceInputHighColorResolution(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFullFloatingPointProcessing(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewEnableFrameTimeCorrection(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewVSyncOffsetIncrease(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewVSyncOffsetDecrease(CCmdUI* pCmdUI);
	afx_msg void OnViewVSync();
	afx_msg void OnViewVSyncAccurate();

	afx_msg void OnViewSynchronizeVideo();
	afx_msg void OnViewSynchronizeDisplay();
	afx_msg void OnViewSynchronizeNearest();

	afx_msg void OnViewColorManagementEnable();
	afx_msg void OnViewColorManagementInputAuto();
	afx_msg void OnViewColorManagementInputHDTV();
	afx_msg void OnViewColorManagementInputSDTV_NTSC();
	afx_msg void OnViewColorManagementInputSDTV_PAL();
	afx_msg void OnViewColorManagementAmbientLightBright();
	afx_msg void OnViewColorManagementAmbientLightDim();
	afx_msg void OnViewColorManagementAmbientLightDark();
	afx_msg void OnViewColorManagementIntentPerceptual();
	afx_msg void OnViewColorManagementIntentRelativeColorimetric();
	afx_msg void OnViewColorManagementIntentSaturation();
	afx_msg void OnViewColorManagementIntentAbsoluteColorimetric();

	afx_msg void OnViewEVROutputRange_0_255();
	afx_msg void OnViewEVROutputRange_16_235();

	afx_msg void OnViewFlushGPUBeforeVSync();
	afx_msg void OnViewFlushGPUAfterVSync();
	afx_msg void OnViewFlushGPUWait();

	afx_msg void OnViewD3DFullScreen();
	afx_msg void OnViewDisableDesktopComposition();
	afx_msg void OnViewAlternativeVSync();
	afx_msg void OnViewResetDefault();
	afx_msg void OnViewResetOptimal();

	afx_msg void OnViewFullscreenGUISupport();
	afx_msg void OnViewHighColorResolution();
	afx_msg void OnViewForceInputHighColorResolution();
	afx_msg void OnViewFullFloatingPointProcessing();
	afx_msg void OnViewEnableFrameTimeCorrection();
	afx_msg void OnViewVSyncOffsetIncrease();
	afx_msg void OnViewVSyncOffsetDecrease();
	afx_msg void OnUpdateShaderToggle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShaderToggleScreenSpace(CCmdUI* pCmdUI);
	afx_msg void OnShaderToggle();
	afx_msg void OnShaderToggleScreenSpace();
	afx_msg void OnUpdateViewRemainingTime(CCmdUI* pCmdUI);
	afx_msg void OnViewRemainingTime();
	afx_msg void OnD3DFullscreenToggle();
	afx_msg void OnGotoSubtitle(UINT nID);
	afx_msg void OnShiftSubtitle(UINT nID);
	afx_msg void OnSubtitleDelay(UINT nID);


	afx_msg void OnPlayPlay();
	afx_msg void OnPlayPause();
	afx_msg void OnPlayPauseI();
	afx_msg void OnPlayPlaypause();
	afx_msg void OnPlayStop();
	afx_msg void OnUpdatePlayPauseStop(CCmdUI* pCmdUI);
	afx_msg void OnPlayFramestep(UINT nID);
	afx_msg void OnUpdatePlayFramestep(CCmdUI* pCmdUI);
	afx_msg void OnPlaySeek(UINT nID);
	afx_msg void OnPlaySeekKey(UINT nID); // no menu item
	afx_msg void OnUpdatePlaySeek(CCmdUI* pCmdUI);
	afx_msg void OnPlayGoto();
	afx_msg void OnUpdateGoto(CCmdUI* pCmdUI);
	afx_msg void OnPlayChangeRate(UINT nID);
	afx_msg void OnUpdatePlayChangeRate(CCmdUI* pCmdUI);
	afx_msg void OnPlayResetRate();
	afx_msg void OnUpdatePlayResetRate(CCmdUI* pCmdUI);
	afx_msg void OnPlayChangeAudDelay(UINT nID);
	afx_msg void OnUpdatePlayChangeAudDelay(CCmdUI* pCmdUI);
	afx_msg void OnPlayFilters(UINT nID);
	afx_msg void OnUpdatePlayFilters(CCmdUI* pCmdUI);
	afx_msg void OnPlayShaders(UINT nID);
	afx_msg void OnUpdatePlayShaders(CCmdUI* pCmdUI);
	afx_msg void OnPlayAudio(UINT nID);
	afx_msg void OnUpdatePlayAudio(CCmdUI* pCmdUI);
	afx_msg void OnPlaySubtitles(UINT nID);
	afx_msg void OnUpdatePlaySubtitles(CCmdUI* pCmdUI);
	afx_msg void OnPlayLanguage(UINT nID);
	afx_msg void OnUpdatePlayLanguage(CCmdUI* pCmdUI);
	afx_msg void OnPlayVolume(UINT nID);
	afx_msg void OnPlayVolumeBoost(UINT nID);
	afx_msg void OnUpdatePlayVolumeBoost(CCmdUI* pCmdUI);
	afx_msg void OnAfterplayback(UINT nID);
	afx_msg void OnUpdateAfterplayback(CCmdUI* pCmdUI);

	afx_msg void OnNavigateSkip(UINT nID);
	afx_msg void OnUpdateNavigateSkip(CCmdUI* pCmdUI);
	afx_msg void OnNavigateSkipPlaylistItem(UINT nID);
	afx_msg void OnUpdateNavigateSkipPlaylistItem(CCmdUI* pCmdUI);
	afx_msg void OnNavigateMenu(UINT nID);
	afx_msg void OnUpdateNavigateMenu(CCmdUI* pCmdUI);
	afx_msg void OnNavigateAudio(UINT nID);
	afx_msg void OnNavigateSubpic(UINT nID);
	afx_msg void OnNavigateAngle(UINT nID);
	afx_msg void OnNavigateChapters(UINT nID);
	afx_msg void OnNavigateMenuItem(UINT nID);
	afx_msg void OnUpdateNavigateMenuItem(CCmdUI* pCmdUI);
	afx_msg void OnTunerScan();
	afx_msg void OnUpdateTunerScan(CCmdUI* pCmdUI);

	afx_msg void OnFavoritesAdd();
	afx_msg void OnUpdateFavoritesAdd(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesQuickAddFavorite();
	afx_msg void OnFavoritesOrganize();
	afx_msg void OnUpdateFavoritesOrganize(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesFile(UINT nID);
	afx_msg void OnUpdateFavoritesFile(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesDVD(UINT nID);
	afx_msg void OnUpdateFavoritesDVD(CCmdUI* pCmdUI);
	afx_msg void OnFavoritesDevice(UINT nID);
	afx_msg void OnUpdateFavoritesDevice(CCmdUI* pCmdUI);
	afx_msg void OnRecentFileClear();
	afx_msg void OnUpdateRecentFileClear(CCmdUI* pCmdUI);
	afx_msg void OnRecentFile(UINT nID);
	afx_msg void OnUpdateRecentFile(CCmdUI* pCmdUI);


	afx_msg void OnHelpHomepage();
	afx_msg void OnHelpDocumentation();
	afx_msg void OnHelpDonate();

	afx_msg void OnClose();

	afx_msg void OnLanguage(UINT nID);
	afx_msg void OnUpdateLanguage(CCmdUI* pCmdUI);


	CMPC_Lcd			m_Lcd;

	// ==== Added by CASIMIR666
	CWnd*				m_pVideoWnd;			// Current Video (main display screen or 2nd)
	SIZE				m_fullWndSize;
	CFullscreenWnd*		m_pFullscreenWnd;
	CComPtr<IVMRMixerControl9>	m_pMC;
	CComPtr<IMFVideoDisplayControl>	m_pMFVDC;
	CVMROSD				m_OSD;
	bool				m_OpenFile;
	bool				m_bRemainingTime;
	int					m_nCurSubtitle;
	long				m_lSubtitleShift;
	__int64				m_rtCurSubPos;
	CString				m_strTitle;
	bool				m_bToggleShader;
	bool				m_bToggleShaderScreenSpace;
	bool				m_bInOptions;
	bool				m_bStopTunerScan;

	void		SetLoadState(MPC_LOADSTATE iState);
	void		SetPlayState(MPC_PLAYSTATE iState);
	bool		CreateFullScreenWindow();
	void		SetVMR9ColorControl(float Brightness, float Contrast, float Hue, float Saturation);
	LPCTSTR		GetDVDAudioFormatName (DVD_AudioAttributes& ATR);
	void		SetAudioDelay(REFERENCE_TIME rtShift);
	void		SetSubtitleDelay(int delay_ms);
//	void		AutoSelectTracks();
	bool		IsRealEngineCompatible(CString strFilename);
	void		SetTimersPlay();
	void		KillTimersStop();


	// MPC API functions
	void		ProcessAPICommand(COPYDATASTRUCT* pCDS);
	void		SendAPICommand (MPCAPI_COMMAND nCommand, LPCWSTR fmt, ...);
	void		SendNowPlayingToApi();
	void		SendSubtitleTracksToApi();
	void		SendAudioTracksToApi();
	void		SendPlaylistToApi();
	afx_msg void OnFileOpendirectory();

	void		SendCurrentPositionToApi(bool fNotifySeek = false);
	void		ShowOSDCustomMessageApi(MPC_OSDDATA *osdData);
	void		JumpOfNSeconds(int seconds);

	CString GetVidPos();

	ITaskbarList3* m_pTaskbarList;
	HRESULT CreateThumbnailToolbar();
	HRESULT UpdateThumbarButton();
	HRESULT UpdateThumbnailClip();

protected:
	// GDI+
	ULONG_PTR m_gdiplusToken;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, UINT nEventData);
};
