/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#pragma once

#include <atlbase.h>

#include "ChildView.h"
#include "DebugShadersDlg.h"
#include "PlayerSeekBar.h"
#include "PlayerToolBar.h"
#include "PlayerInfoBar.h"
#include "PlayerStatusBar.h"
#include "PlayerSubresyncBar.h"
#include "PlayerPlaylistBar.h"
#include "PlayerCaptureBar.h"
#include "PlayerNavigationBar.h"
#include "EditListEditor.h"
#include "PPageSheet.h"
#include "PPageFileInfoSheet.h"
#include "FileDropTarget.h"
#include "KeyProvider.h"
#include "GraphThread.h"
#include "TimerWrappers.h"
#include "MainFrmControls.h"

#include "../SubPic/ISubPic.h"

#include "IGraphBuilder2.h"

#include "RealMediaGraph.h"
#ifndef _WIN64
// TODO: add QuickTime support for x64 when available!
#include "QuicktimeGraph.h"
#endif /* _WIN64 */
#include "ShockwaveGraph.h"

#include "IChapterInfo.h"
#include "IKeyFrameInfo.h"
#include "IBufferInfo.h"

#include "WebServer.h"
#include <afxmt.h>
#include <d3d9.h>
#include <vmr9.h>
#include <evr.h>
#include <evr9.h>
#include <Il21dec.h>
#include "VMROSD.h"
#include "LcdSupport.h"
#include "MpcApi.h"
#include "../filters/renderer/SyncClock/SyncClock.h"
#include "sizecbar/scbarg.h"
#include "DSMPropertyBag.h"
#include "SkypeMoodMsgHandler.h"
#include "SubtitleDlDlg.h"
#include "SubtitleUpDlg.h"

#include <memory>
#include <future>


class CFullscreenWnd;

enum class MLS {
    CLOSED,
    LOADING,
    LOADED,
    CLOSING,
    FAILING,
};

enum {
    PM_NONE,
    PM_FILE,
    PM_DVD,
    PM_ANALOG_CAPTURE,
    PM_DIGITAL_CAPTURE
};

interface __declspec(uuid("6E8D4A21-310C-11d0-B79A-00AA003767A7")) // IID_IAMLine21Decoder
IAMLine21Decoder_2 :
public IAMLine21Decoder {};

class OpenMediaData
{
public:
    //  OpenMediaData() {}
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
    //  OpenDVDData() {}
    CString path;
    CComPtr<IDvdState> pDvdState;
};

class OpenDeviceData : public OpenMediaData
{
public:
    OpenDeviceData() {
        vinput = vchannel = ainput = -1;
    }
    CStringW DisplayName[2];
    int vinput, vchannel, ainput;
};

class TunerScanData
{
public:
    ULONG FrequencyStart;
    ULONG FrequencyStop;
    ULONG Bandwidth;
    LONG  Offset;
    HWND  Hwnd;
};

struct SubtitleInput {
    CComQIPtr<ISubStream> pSubStream;
    CComPtr<IBaseFilter> pSourceFilter;

    SubtitleInput() {};
    SubtitleInput(CComQIPtr<ISubStream> pSubStream) : pSubStream(pSubStream) {};
    SubtitleInput(CComQIPtr<ISubStream> pSubStream, CComPtr<IBaseFilter> pSourceFilter)
        : pSubStream(pSubStream), pSourceFilter(pSourceFilter) {};
};

interface ISubClock;

class CMainFrame : public CFrameWnd, public CDropTarget
{
public:
    enum class Timer32HzSubscriber {
        TOOLBARS_HIDER,
        CURSOR_HIDER,
        CURSOR_HIDER_D3DFS,
    };
    OnDemandTimer<Timer32HzSubscriber> m_timer32Hz;

    enum class TimerOneTimeSubscriber {
        TOOLBARS_DELAY_NOTLOADED,
        CHILDVIEW_CURSOR_HACK,
        DELAY_IDLE,
        ACTIVE_SHADER_FILES_CHANGE_COOLDOWN,
        DELAY_PLAYPAUSE_AFTER_AUTOCHANGE_MODE,
        DVBINFO_UPDATE,
        STATUS_ERASE,
        PLACE_FULLSCREEN_UNDER_ACTIVE_WINDOW,
        AUTOFIT_TIMEOUT
    };
    OneTimeTimerPool<TimerOneTimeSubscriber> m_timerOneTime;

private:
    EventClient m_eventc;
    void EventCallback(MpcEvent ev);

    enum {
        TIMER_STREAMPOSPOLLER = 1,
        TIMER_STREAMPOSPOLLER2,
        TIMER_STATS,
        TIMER_UNLOAD_UNUSED_EXTERNAL_OBJECTS,
        TIMER_32HZ,
        TIMER_ONETIME_START,
        TIMER_ONETIME_END = TIMER_ONETIME_START + 127,
    };
    enum {
        SEEK_DIRECTION_NONE,
        SEEK_DIRECTION_BACKWARD,
        SEEK_DIRECTION_FORWARD
    };
    enum {
        ZOOM_DEFAULT_LEVEL = 0,
        ZOOM_AUTOFIT = -1,
        ZOOM_AUTOFIT_LARGER = -2
    };

    friend class CPPageFileInfoSheet;
    friend class CPPageLogo;
    friend class CMouse;
    friend class CPlayerSeekBar; // for accessing m_controls.ControlChecked()
    friend class CChildView; // for accessing m_controls.DelayShowNotLoaded()
    friend class SubtitlesProvider;

    // TODO: wrap these graph objects into a class to make it look cleaner

    CComPtr<IGraphBuilder2> m_pGB;
    CComQIPtr<IMediaControl> m_pMC;
    CComQIPtr<IMediaEventEx> m_pME;
    CComQIPtr<IVideoWindow> m_pVW;
    CComQIPtr<IBasicVideo> m_pBV;
    CComQIPtr<IBasicAudio> m_pBA;
    CComQIPtr<IMediaSeeking> m_pMS;
    CComQIPtr<IVideoFrameStep> m_pFS;
    CComQIPtr<IFileSourceFilter> m_pFSF;
    CComQIPtr<IQualProp, &IID_IQualProp> m_pQP;
    CComQIPtr<IBufferInfo> m_pBI;
    CComQIPtr<IAMOpenProgress> m_pAMOP;
    CComPtr<IVMRMixerControl9> m_pVMRMC;
    CComPtr<IMFVideoDisplayControl> m_pMFVDC;
    CComPtr<IMFVideoProcessor> m_pMFVP;
    CComPtr<IVMRWindowlessControl9> m_pVMRWC;

    CComPtr<ISubPicAllocatorPresenter> m_pCAP;
    CComPtr<ISubPicAllocatorPresenter2> m_pCAP2;

    CComPtr<IMadVRSettings> m_pMVRS;

    CComQIPtr<IDvdControl2> m_pDVDC;
    CComQIPtr<IDvdInfo2> m_pDVDI;
    CComPtr<IAMLine21Decoder_2> m_pLN21;

    CComPtr<ICaptureGraphBuilder2> m_pCGB;
    CStringW m_VidDispName, m_AudDispName;
    CComPtr<IBaseFilter> m_pVidCap, m_pAudCap;
    CComPtr<IAMVideoCompression> m_pAMVCCap, m_pAMVCPrev;
    CComPtr<IAMStreamConfig> m_pAMVSCCap, m_pAMVSCPrev, m_pAMASC;
    CComPtr<IAMCrossbar> m_pAMXBar;
    CComPtr<IAMTVTuner> m_pAMTuner;
    CComPtr<IAMDroppedFrames> m_pAMDF;

    CComPtr<IUnknown> m_pProv;

    bool m_bUsingDXVA;
    CString m_HWAccelType;
    void UpdateDXVAStatus();

    void SetVolumeBoost(UINT nAudioBoost);
    void SetBalance(int balance);

    // subtitles

    CCritSec m_csSubLock;
    CCritSec m_csSubtitleManagementLock;

    CList<SubtitleInput> m_pSubStreams;
    POSITION m_posFirstExtSub;
    SubtitleInput m_pCurrentSubInput;

    SubtitleInput* GetSubtitleInput(int& i, bool bIsOffset = false);

    friend class CTextPassThruFilter;

    // windowing

    bool m_bDelaySetOutputRect;

    CRect m_lastWindowRect;

    void SetDefaultWindowRect(int iMonitor = 0);
    void SetDefaultFullscreenState();
    void RestoreDefaultWindowRect();
    CSize GetZoomWindowSize(double dScale);
    CRect GetZoomWindowRect(const CSize& size);
    void ZoomVideoWindow(double dScale = ZOOM_DEFAULT_LEVEL);
    double GetZoomAutoFitScale(bool bLargerOnly = false);

    void SetAlwaysOnTop(int iOnTop);

    // dynamic menus

    void CreateDynamicMenus();
    void DestroyDynamicMenus();
    void SetupOpenCDSubMenu();
    void SetupFiltersSubMenu();
    void SetupAudioSubMenu();
    void SetupSubtitlesSubMenu();
    void SetupVideoStreamsSubMenu();
    void SetupJumpToSubMenus(CMenu* parentMenu = nullptr, int iInsertPos = -1);
    void SetupFavoritesSubMenu();
    void SetupShadersSubMenu();
    void SetupRecentFilesSubMenu();

    IBaseFilter* FindSourceSelectableFilter();
    void SetupNavStreamSelectSubMenu(CMenu& subMenu, UINT id, DWORD dwSelGroup);
    void OnNavStreamSelectSubMenu(UINT id, DWORD dwSelGroup);

    CMenu m_mainPopupMenu, m_popupMenu;
    CMenu m_openCDsMenu;
    CMenu m_filtersMenu, m_subtitlesMenu, m_audiosMenu, m_videoStreamsMenu;
    CMenu m_chaptersMenu, m_titlesMenu, m_playlistMenu, m_BDPlaylistMenu, m_channelsMenu;
    CMenu m_favoritesMenu;
    CMenu m_shadersMenu;
    CMenu m_recentFilesMenu;

    UINT m_nJumpToSubMenusCount;

    CInterfaceArray<IUnknown, &IID_IUnknown> m_pparray;
    CInterfaceArray<IAMStreamSelect> m_ssarray;

    // chapters (file mode)
    CComPtr<IDSMChapterBag> m_pCB;
    void SetupChapters();

    // chapters (DVD mode)
    void SetupDVDChapters();

    bool SeekToFileChapter(int iChapter, bool bRelative = false);
    bool SeekToDVDChapter(int iChapter, bool bRelative = false);

    void AddTextPassThruFilter();

    int m_nLoops;
    UINT m_nLastSkipDirection;

    bool m_fCustomGraph;
    bool m_fRealMediaGraph, m_fShockwaveGraph, m_fQuicktimeGraph;

    CComPtr<ISubClock> m_pSubClock;

    bool m_fFrameSteppingActive;
    int m_nStepForwardCount;
    REFERENCE_TIME m_rtStepForwardStart;
    int m_nVolumeBeforeFrameStepping;

    bool m_fEndOfStream;

    bool m_bRememberFilePos;

    DWORD m_dwLastRun;

    bool m_bBuffering;

    bool m_fLiveWM;

    void SendStatusMessage(CString msg, int nTimeOut);
    CString m_playingmsg, m_closingmsg;

    REFERENCE_TIME m_rtDurationOverride;

    void CleanGraph();

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

    CString m_currentCoverAuthor;
    CString m_currentCoverPath;

    CAutoPtr<SkypeMoodMsgHandler> m_pSkypeMoodMsgHandler;
    void SendNowPlayingToSkype();

    MLS m_eMediaLoadState;

    REFTIME GetAvgTimePerFrame() const;

public:
    void StartWebServer(int nPort);
    void StopWebServer();

    int GetPlaybackMode() const { return m_iPlaybackMode; }
    bool IsPlaybackCaptureMode() const { return GetPlaybackMode() == PM_ANALOG_CAPTURE || GetPlaybackMode() == PM_DIGITAL_CAPTURE; }
    void SetPlaybackMode(int iNewStatus);
    bool IsMuted() { return m_wndToolBar.GetVolume() == -10000; }
    int GetVolume() { return m_wndToolBar.m_volctrl.GetPos(); }

public:
    CMainFrame();
    DECLARE_DYNAMIC(CMainFrame)

    // Attributes
public:
    bool m_fFullScreen;
    bool m_fFirstFSAfterLaunchOnFS;
    bool m_fStartInD3DFullscreen;

    CComPtr<IBaseFilter> m_pRefClock; // Adjustable reference clock. GothSync
    CComPtr<ISyncClock> m_pSyncClock;

    bool IsFrameLessWindow() const {
        return (m_fFullScreen || AfxGetAppSettings().eCaptionMenuMode == MODE_BORDERLESS);
    }
    bool IsCaptionHidden() const {//If no caption, there is no menu bar. But if is no menu bar, then the caption can be.
        return (!m_fFullScreen && AfxGetAppSettings().eCaptionMenuMode > MODE_HIDEMENU); //!=MODE_SHOWCAPTIONMENU && !=MODE_HIDEMENU
    }
    bool IsMenuHidden() const {
        return (!m_fFullScreen && AfxGetAppSettings().eCaptionMenuMode != MODE_SHOWCAPTIONMENU);
    }
    bool IsPlaylistEmpty() const {
        return (m_wndPlaylistBar.GetCount() == 0);
    }
    bool IsInteractiveVideo() const {
        return (AfxGetAppSettings().fIntRealMedia && m_fRealMediaGraph || m_fShockwaveGraph);
    }
    bool IsD3DFullScreenMode() const;

    CControlBar* m_pLastBar;

protected:
    bool m_bFirstPlay;
    bool m_bOpeningInAutochangedMonitorMode;
    bool m_bPausedForAutochangeMonitorMode;

    bool m_fAudioOnly;
    CString m_LastOpenBDPath;
    CAutoPtr<OpenMediaData> m_lastOMD;
    HMONITOR m_LastWindow_HM;

    DVD_DOMAIN m_iDVDDomain;
    DWORD m_iDVDTitle;
    double m_dSpeedRate;
    double m_ZoomX, m_ZoomY, m_PosX, m_PosY;
    int m_AngleX, m_AngleY, m_AngleZ;

    // Operations
    bool OpenMediaPrivate(CAutoPtr<OpenMediaData> pOMD);
    void CloseMediaPrivate();
    void DoTunerScan(TunerScanData* pTSD);

    CWnd* GetModalParent();

    void OpenCreateGraphObject(OpenMediaData* pOMD);
    void OpenFile(OpenFileData* pOFD);
    void OpenDVD(OpenDVDData* pODD);
    void OpenCapture(OpenDeviceData* pODD);
    HRESULT OpenBDAGraph();
    void OpenCustomizeGraph();
    void OpenSetupVideo();
    void OpenSetupAudio();
    void OpenSetupInfoBar(bool bClear = true);
    void UpdateChapterInInfoBar();
    void OpenSetupStatsBar();
    void OpenSetupStatusBar();
    void OpenSetupCaptureBar();
    void OpenSetupWindowTitle(bool reset = false);

public:
    static bool GetCurDispMode(const CString& displayName, DisplayMode& dm);
    static bool GetDispMode(CString displayName, int i, DisplayMode& dm);

protected:
    void SetDispMode(CString displayName, const DisplayMode& dm);
    void AutoChangeMonitorMode();

    void GraphEventComplete();

    friend class CGraphThread;
    CGraphThread* m_pGraphThread;
    bool m_bOpenedThroughThread;
    ::CEvent m_evOpenPrivateFinished;
    ::CEvent m_evClosePrivateFinished;

    void LoadKeyFrames();
    std::vector<REFERENCE_TIME> m_kfs;

    bool m_fOpeningAborted;
    bool m_bWasSnapped;

protected:
    friend class CSubtitleDlDlg;
    CSubtitleDlDlg m_wndSubtitlesDownloadDialog;
    friend class CSubtitleUpDlg;
    CSubtitleUpDlg m_wndSubtitlesUploadDialog;
    friend class CPPageSubMisc;

    friend class SubtitlesProviders;
    SubtitlesProviders* m_pSubtitlesProviders;
    friend struct SubtitlesInfo;
    friend class SubtitlesThread;

public:
    void OpenCurPlaylistItem(REFERENCE_TIME rtStart = 0);
    void OpenMedia(CAutoPtr<OpenMediaData> pOMD);
    void PlayFavoriteFile(CString fav);
    void PlayFavoriteDVD(CString fav);
    bool ResetDevice();
    bool DisplayChange();
    void CloseMedia(bool bNextIsQueued = false);
    void StartTunerScan(CAutoPtr<TunerScanData> pTSD);
    void StopTunerScan();
    HRESULT SetChannel(int nChannel);

    void AddCurDevToPlaylist();

    bool m_bTrayIcon;
    void ShowTrayIcon(bool bShow);
    void SetTrayTip(CString str);

    CSize GetVideoSize() const;
    void ToggleFullscreen(bool fToNearest, bool fSwitchScreenResWhenHasTo);
    void ToggleD3DFullscreen(bool fSwitchScreenResWhenHasTo);
    void MoveVideoWindow(bool fShowStats = false, bool bSetStoppedVideoRect = false);
    void RepaintVideo();
    void HideVideoWindow(bool fHide);

    OAFilterState GetMediaState() const;
    REFERENCE_TIME GetPos() const;
    REFERENCE_TIME GetDur() const;
    bool GetNeighbouringKeyFrames(REFERENCE_TIME rtTarget, std::pair<REFERENCE_TIME, REFERENCE_TIME>& keyframes) const;
    REFERENCE_TIME GetClosestKeyFrame(REFERENCE_TIME rtTarget) const;
    void SeekTo(REFERENCE_TIME rt, bool bShowOSD = true);
    void SetPlayingRate(double rate);

    int SetupAudioStreams();
    int SetupSubtitleStreams();

    bool LoadSubtitle(CString fn, SubtitleInput* pSubInput = nullptr, bool bAutoLoad = false);
    bool SetSubtitle(int i, bool bIsOffset = false, bool bDisplayMessage = false);
    void SetSubtitle(const SubtitleInput& subInput);
    void ToggleSubtitleOnOff(bool bDisplayMessage = false);
    void ReplaceSubtitle(const ISubStream* pSubStreamOld, ISubStream* pSubStreamNew);
    void InvalidateSubtitle(DWORD_PTR nSubtitleId = DWORD_PTR_MAX, REFERENCE_TIME rtInvalidate = -1);
    void ReloadSubtitle();
    void UpdateSubOverridePlacement();
    void UpdateSubDefaultStyle();
    void UpdateSubAspectRatioCompensation();
    HRESULT InsertTextPassThruFilter(IBaseFilter* pBF, IPin* pPin, IPin* pPinto);

    void SetAudioTrackIdx(int index);
    void SetSubtitleTrackIdx(int index);

    void AddFavorite(bool fDisplayMessage = false, bool fShowDialog = true);

    CString GetFileName();
    CString GetCaptureTitle();

    // shaders
    void SetShaders(bool bSetPreResize = true, bool bSetPostResize = true);

    // capturing
    bool m_fCapturing;
    HRESULT BuildCapture(IPin* pPin, IBaseFilter* pBF[3], const GUID& majortype, AM_MEDIA_TYPE* pmt); // pBF: 0 buff, 1 enc, 2 mux, pmt is for 1 enc
    bool BuildToCapturePreviewPin(IBaseFilter* pVidCap, IPin** pVidCapPin, IPin** pVidPrevPin,
                                  IBaseFilter* pAudCap, IPin** pAudCapPin, IPin** pAudPrevPin);
    bool BuildGraphVideoAudio(int fVPreview, bool fVCapture, int fAPreview, bool fACapture);
    bool DoCapture(), StartCapture(), StopCapture();

    void DoAfterPlaybackEvent();
    void ParseDirs(CAtlList<CString>& sl);
    bool SearchInDir(bool bDirForward, bool bLoop = false);

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    virtual void RecalcLayout(BOOL bNotify = TRUE);

    // DVB capture
    void UpdateCurrentChannelInfo(bool bShowOSD = true, bool bShowInfoBar = false);
    LRESULT OnCurrentChannelInfoUpdated(WPARAM wParam, LPARAM lParam);

    struct DVBState {
        struct EITData {
            HRESULT hr        = E_FAIL;
            EventDescriptor NowNext;
            bool bShowOSD     = true;
            bool bShowInfoBar = false;
        };

        CString         sChannelName;                // Current channel name
        CDVBChannel*    pChannel          = nullptr; // Pointer to current channel object
        EventDescriptor NowNext;                     // Current channel EIT
        bool            bActive           = false;   // True when channel is active
        bool            bSetChannelActive = false;   // True when channel change is in progress
        bool            bInfoActive       = false;   // True when EIT data update is in progress
        bool            bAbortInfo        = true;    // True when aborting current EIT update
        std::future<DVBState::EITData> infoData;

        void Reset() {
            sChannelName.Empty();
            pChannel          = nullptr;
            NowNext           = EventDescriptor();
            bActive           = false;
            bSetChannelActive = false;
            bInfoActive       = false;
            bAbortInfo        = true;
        }

        ~DVBState() {
            bAbortInfo = true;
        }
    };

    std::unique_ptr<DVBState> m_pDVBState = nullptr;

    // Implementation
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
    friend class CMainFrameControls;
    CMainFrameControls m_controls;
    friend class CPlayerBar; // it notifies m_controls of panel re-dock

    CChildView m_wndView;

    CPlayerSeekBar m_wndSeekBar;
    CPlayerToolBar m_wndToolBar;
    CPlayerInfoBar m_wndInfoBar;
    CPlayerInfoBar m_wndStatsBar;
    CPlayerStatusBar m_wndStatusBar;

    CPlayerSubresyncBar m_wndSubresyncBar;
    CPlayerPlaylistBar m_wndPlaylistBar;
    CPlayerCaptureBar m_wndCaptureBar;
    CPlayerNavigationBar m_wndNavigationBar;
    CEditListEditor m_wndEditListEditor;

    std::unique_ptr<CDebugShadersDlg> m_pDebugShaders;

    CFileDropTarget m_fileDropTarget;
    // TODO
    DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
    BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
    DROPEFFECT OnDropEx(COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
    void OnDragLeave();
    DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);

    LPCTSTR GetRecentFile() const;

    friend class CPPagePlayback; // TODO
    friend class CPPageAudioSwitcher; // TODO
    friend class CMPlayerCApp; // TODO

    // Generated message map functions

    DECLARE_MESSAGE_MAP()

public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();

    afx_msg LRESULT OnTaskBarRestart(WPARAM, LPARAM);
    afx_msg LRESULT OnNotifyIcon(WPARAM, LPARAM);
    afx_msg LRESULT OnTaskBarThumbnailsCreate(WPARAM, LPARAM);

    afx_msg LRESULT OnSkypeAttach(WPARAM wParam, LPARAM lParam);

    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnEnterSizeMove();
    afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
    afx_msg void OnExitSizeMove();
    afx_msg void OnDisplayChange();
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
    afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);
    afx_msg void OnRawInput(UINT nInputcode, HRAWINPUT hRawInput);

    afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);

    afx_msg void OnTimer(UINT_PTR nIDEvent);

    afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResetDevice(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRepaintRenderLess(WPARAM wParam, LPARAM lParam);

    afx_msg void SaveAppSettings();

    afx_msg LRESULT OnNcHitTest(CPoint point);

    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

    afx_msg void OnInitMenu(CMenu* pMenu);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg void OnUnInitMenuPopup(CMenu* pPopupMenu, UINT nFlags);
    afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);

    afx_msg BOOL OnQueryEndSession();
    afx_msg void OnEndSession(BOOL bEnding);

    BOOL OnMenu(CMenu* pMenu);
    afx_msg void OnMenuPlayerShort();
    afx_msg void OnMenuPlayerLong();
    afx_msg void OnMenuFilters();

    afx_msg void OnUpdatePlayerStatus(CCmdUI* pCmdUI);

    afx_msg LRESULT OnFilePostOpenmedia(WPARAM wParam, LPARAM lparam);
    afx_msg LRESULT OnOpenMediaFailed(WPARAM wParam, LPARAM lParam);
    void OnFilePostClosemedia(bool bNextIsQueued = false);

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
    afx_msg void OnFileOpenOpticalDisk(UINT nID);
    afx_msg void OnFileReopen();
    afx_msg void OnFileRecycle();
    afx_msg void OnDropFiles(HDROP hDropInfo); // no menu item
    afx_msg void OnFileSaveAs();
    afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
    afx_msg void OnFileSaveImage();
    afx_msg void OnFileSaveImageAuto();
    afx_msg void OnUpdateFileSaveImage(CCmdUI* pCmdUI);
    afx_msg void OnFileSaveThumbnails();
    afx_msg void OnUpdateFileSaveThumbnails(CCmdUI* pCmdUI);
    afx_msg void OnFileSubtitlesLoad();
    afx_msg void OnUpdateFileSubtitlesLoad(CCmdUI* pCmdUI);
    afx_msg void OnFileSubtitlesSave();
    afx_msg void OnUpdateFileSubtitlesSave(CCmdUI* pCmdUI);
    afx_msg void OnFileSubtitlesUpload();
    afx_msg void OnUpdateFileSubtitlesUpload(CCmdUI* pCmdUI);
    afx_msg void OnFileSubtitlesDownload();
    afx_msg void OnUpdateFileSubtitlesDownload(CCmdUI* pCmdUI);
    afx_msg void OnFileProperties();
    afx_msg void OnUpdateFileProperties(CCmdUI* pCmdUI);
    afx_msg void OnFileCloseAndRestore();
    afx_msg void OnFileCloseMedia(); // no menu item
    afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);

    void SetCaptionState(MpcCaptionState eState);
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
    afx_msg void OnViewDebugShaders();
    afx_msg void OnUpdateViewDebugShaders(CCmdUI* pCmdUI);
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
    afx_msg void OnViewZoomAutoFitLarger();
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
    afx_msg void OnUpdateViewHalfFloatingPointProcessing(CCmdUI* pCmdUI);
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
    afx_msg void OnViewHalfFloatingPointProcessing();
    afx_msg void OnViewEnableFrameTimeCorrection();
    afx_msg void OnViewVSyncOffsetIncrease();
    afx_msg void OnViewVSyncOffsetDecrease();
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
    afx_msg void OnApiPlay();
    afx_msg void OnApiPause();
    afx_msg void OnPlayStop();
    afx_msg void OnUpdatePlayPauseStop(CCmdUI* pCmdUI);
    afx_msg void OnPlayFramestep(UINT nID);
    afx_msg void OnUpdatePlayFramestep(CCmdUI* pCmdUI);
    afx_msg void OnPlaySeek(UINT nID);
    afx_msg void OnPlaySeekSet();
    afx_msg void OnPlaySeekKey(UINT nID); // no menu item
    afx_msg void OnUpdatePlaySeek(CCmdUI* pCmdUI);
    afx_msg void OnPlayChangeRate(UINT nID);
    afx_msg void OnUpdatePlayChangeRate(CCmdUI* pCmdUI);
    afx_msg void OnPlayResetRate();
    afx_msg void OnUpdatePlayResetRate(CCmdUI* pCmdUI);
    afx_msg void OnPlayChangeAudDelay(UINT nID);
    afx_msg void OnUpdatePlayChangeAudDelay(CCmdUI* pCmdUI);
    afx_msg void OnPlayFiltersCopyToClipboard();
    afx_msg void OnPlayFilters(UINT nID);
    afx_msg void OnUpdatePlayFilters(CCmdUI* pCmdUI);
    afx_msg void OnPlayShadersSelect();
    afx_msg void OnPlayShadersPresetNext();
    afx_msg void OnPlayShadersPresetPrev();
    afx_msg void OnPlayShadersPresets(UINT nID);
    afx_msg void OnPlayAudio(UINT nID);
    afx_msg void OnPlaySubtitles(UINT nID);
    afx_msg void OnPlayVideoStreams(UINT nID);
    afx_msg void OnPlayFiltersStreams(UINT nID);
    afx_msg void OnPlayVolume(UINT nID);
    afx_msg void OnPlayVolumeBoost(UINT nID);
    afx_msg void OnUpdatePlayVolumeBoost(CCmdUI* pCmdUI);
    afx_msg void OnCustomChannelMapping();
    afx_msg void OnUpdateCustomChannelMapping(CCmdUI* pCmdUI);
    afx_msg void OnNormalizeRegainVolume(UINT nID);
    afx_msg void OnUpdateNormalizeRegainVolume(CCmdUI* pCmdUI);
    afx_msg void OnPlayColor(UINT nID);
    afx_msg void OnAfterplayback(UINT nID);
    afx_msg void OnUpdateAfterplayback(CCmdUI* pCmdUI);

    afx_msg void OnNavigateSkip(UINT nID);
    afx_msg void OnUpdateNavigateSkip(CCmdUI* pCmdUI);
    afx_msg void OnNavigateSkipFile(UINT nID);
    afx_msg void OnUpdateNavigateSkipFile(CCmdUI* pCmdUI);
    afx_msg void OnNavigateGoto();
    afx_msg void OnUpdateNavigateGoto(CCmdUI* pCmdUI);
    afx_msg void OnNavigateMenu(UINT nID);
    afx_msg void OnUpdateNavigateMenu(CCmdUI* pCmdUI);
    afx_msg void OnNavigateJumpTo(UINT nID);
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
    afx_msg void OnHelpCheckForUpdate();
    afx_msg void OnHelpToolbarImages();
    afx_msg void OnHelpDonate();

    afx_msg void OnClose();

    CMPC_Lcd m_Lcd;

    // ==== Added by CASIMIR666
    CWnd*           m_pVideoWnd;            // Current Video (main display screen or 2nd)
    CFullscreenWnd* m_pFullscreenWnd;
    CVMROSD     m_OSD;
    bool        m_bRemainingTime;
    int         m_nCurSubtitle;
    long        m_lSubtitleShift;
    REFERENCE_TIME m_rtCurSubPos;
    bool        m_bScanDlgOpened;
    bool        m_bStopTunerScan;
    bool        m_bLockedZoomVideoWindow;
    int         m_nLockedZoomVideoWindow;

    void        SetLoadState(MLS eState);
    MLS         GetLoadState() const;
    void        SetPlayState(MPC_PLAYSTATE iState);
    bool        CreateFullScreenWindow();
    void        SetupEVRColorControl();
    void        SetupVMR9ColorControl();
    void        SetColorControl(DWORD flags, int& brightness, int& contrast, int& hue, int& saturation);
    void        SetClosedCaptions(bool enable);
    LPCTSTR     GetDVDAudioFormatName(const DVD_AudioAttributes& ATR) const;
    void        SetAudioDelay(REFERENCE_TIME rtShift);
    void        SetSubtitleDelay(int delay_ms);
    //void      AutoSelectTracks();
    bool        IsRealEngineCompatible(CString strFilename) const;
    void        SetTimersPlay();
    void        KillTimersStop();


    // MPC API functions
    void        ProcessAPICommand(COPYDATASTRUCT* pCDS);
    void        SendAPICommand(MPCAPI_COMMAND nCommand, LPCWSTR fmt, ...);
    void        SendNowPlayingToApi();
    void        SendSubtitleTracksToApi();
    void        SendAudioTracksToApi();
    void        SendPlaylistToApi();
    afx_msg void OnFileOpendirectory();

    void        SendCurrentPositionToApi(bool fNotifySeek = false);
    void        ShowOSDCustomMessageApi(const MPC_OSDDATA* osdData);
    void        JumpOfNSeconds(int seconds);

    CString GetVidPos() const;

    ITaskbarList3* m_pTaskbarList;
    HRESULT CreateThumbnailToolbar();
    HRESULT UpdateThumbarButton();
    HRESULT UpdateThumbarButton(MPC_PLAYSTATE iPlayState);
    HRESULT UpdateThumbnailClip();

protected:
    // GDI+
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    void WTSRegisterSessionNotification();
    void WTSUnRegisterSessionNotification();

    CMenu* m_pActiveContextMenu;
    CMenu* m_pActiveSystemMenu;

    void UpdateSkypeHandler();
    void UpdateSeekbarChapterBag();
    void UpdateAudioSwitcher();

    void UpdateUILanguage();

    bool m_bAltDownClean;
    bool m_bShowingFloatingMenubar;
    virtual void OnShowMenuBar() override {
        m_bShowingFloatingMenubar = (GetMenuBarVisibility() != AFX_MBV_KEEPVISIBLE);
    };
    virtual void OnHideMenuBar() override {
        m_bShowingFloatingMenubar = false;
    };
    virtual void SetMenuBarVisibility(DWORD dwStyle) override {
        __super::SetMenuBarVisibility(dwStyle);
        if (dwStyle & AFX_MBV_KEEPVISIBLE) {
            m_bShowingFloatingMenubar = false;
        }
    };

    bool IsAeroSnapped();

    CPoint m_snapStartPoint;
    CRect m_snapStartRect;

    bool m_bAllowWindowZoom;
    double m_dLastVideoScaleFactor;
    int m_nLastVideoWidth;

    bool m_bExtOnTop; // 'true' if the "on top" flag was set by an external tool

public:
    afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
    afx_msg void OnSessionChange(UINT nSessionState, UINT nId);

    enum UpdateControlTarget {
        UPDATE_VOLUME_STEP,
        UPDATE_LOGO,
        UPDATE_SKYPE,
        UPDATE_SEEKBAR_CHAPTERS,
        UPDATE_WINDOW_TITLE,
        UPDATE_AUDIO_SWITCHER,
        UPDATE_CONTROLS_VISIBILITY,
        UPDATE_CHILDVIEW_CURSOR_HACK,
    };

    void UpdateControlState(UpdateControlTarget target);

    // TODO: refactor it outside of MainFrm
    GUID GetTimeFormat();

    CAtlList<CHdmvClipInfo::PlaylistItem> m_MPLSPlaylist;
    bool m_bIsBDPlay;
    bool OpenBD(CString Path);

    bool GetDecoderType(CString& type) const;

    DPI m_dpi;
protected:
    afx_msg LRESULT OnLoadSubtitles(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetSubtitles(WPARAM wParam, LPARAM lParam);
};
