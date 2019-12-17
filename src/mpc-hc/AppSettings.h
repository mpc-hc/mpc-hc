/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2018 see Authors.txt
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
#include "../Subtitles/STS.h"
#include "../filters/switcher/AudioSwitcher/AudioSwitcher.h"
#include "../thirdparty/sanear/sanear/src/Interfaces.h"
#include "DVBChannel.h"
#include "FileAssoc.h"
#include "FilterEnum.h"
#include "MediaFormats.h"
#include "MediaPositionList.h"
#include "../filters/renderer/VideoRenderers/RenderersSettings.h"
#include "SettingsDefines.h"
#include "Shaders.h"

#include <afxadv.h>
#include <afxsock.h>

class FilterOverride;

// flags for CAppSettings::nCS
enum {
    CS_NONE = 0,
    CS_SEEKBAR = 1,
    CS_TOOLBAR = CS_SEEKBAR << 1,
    CS_INFOBAR = CS_TOOLBAR << 1,
    CS_STATSBAR = CS_INFOBAR << 1,
    CS_STATUSBAR = CS_STATSBAR << 1,
    CS_LAST = CS_STATUSBAR
};

enum : UINT64 {
    CLSW_NONE = 0,
    CLSW_OPEN = 1,
    CLSW_PLAY = CLSW_OPEN << 1,
    CLSW_CLOSE = CLSW_PLAY << 1,
    CLSW_STANDBY = CLSW_CLOSE << 1,
    CLSW_HIBERNATE = CLSW_STANDBY << 1,
    CLSW_SHUTDOWN = CLSW_HIBERNATE << 1,
    CLSW_LOGOFF = CLSW_SHUTDOWN << 1,
    CLSW_LOCK = CLSW_LOGOFF << 1,
    CLSW_MONITOROFF = CLSW_LOCK << 1,
    CLSW_PLAYNEXT = CLSW_MONITOROFF << 1,
    CLSW_DONOTHING = CLSW_PLAYNEXT << 1,
    CLSW_AFTERPLAYBACK_MASK = CLSW_CLOSE | CLSW_STANDBY | CLSW_SHUTDOWN | CLSW_HIBERNATE | CLSW_LOGOFF | CLSW_LOCK | CLSW_MONITOROFF | CLSW_PLAYNEXT | CLSW_DONOTHING,
    CLSW_FULLSCREEN = CLSW_DONOTHING << 1,
    CLSW_NEW = CLSW_FULLSCREEN << 1,
    CLSW_HELP = CLSW_NEW << 1,
    CLSW_DVD = CLSW_HELP << 1,
    CLSW_CD = CLSW_DVD << 1,
    CLSW_DEVICE = CLSW_CD << 1,
    CLSW_ADD = CLSW_DEVICE << 1,
    CLSW_RANDOMIZE = CLSW_ADD << 1,
    CLSW_MINIMIZED = CLSW_RANDOMIZE << 1,
    CLSW_REGEXTVID = CLSW_MINIMIZED << 1,
    CLSW_REGEXTAUD = CLSW_REGEXTVID << 1,
    CLSW_REGEXTPL = CLSW_REGEXTAUD << 1,
    CLSW_UNREGEXT = CLSW_REGEXTPL << 1,
    CLSW_ICONSASSOC = CLSW_UNREGEXT << 1,
    CLSW_STARTVALID = CLSW_ICONSASSOC << 1,
    CLSW_NOFOCUS = CLSW_STARTVALID << 1,
    CLSW_FIXEDSIZE = CLSW_NOFOCUS << 1,
    CLSW_MONITOR = CLSW_FIXEDSIZE << 1,
    CLSW_D3DFULLSCREEN = CLSW_MONITOR << 1,
    CLSW_ADMINOPTION = CLSW_D3DFULLSCREEN << 1,
    CLSW_SLAVE = CLSW_ADMINOPTION << 1,
    CLSW_AUDIORENDERER = CLSW_SLAVE << 1,
    CLSW_RESET = CLSW_AUDIORENDERER << 1,
    CLSW_PRESET1 = CLSW_RESET << 1,
    CLSW_PRESET2 = CLSW_PRESET1 << 1,
    CLSW_PRESET3 = CLSW_PRESET2 << 1,
    CLSW_CONFIGLAVSPLITTER = CLSW_PRESET3 << 1,
    CLSW_CONFIGLAVAUDIO = CLSW_CONFIGLAVSPLITTER << 1,
    CLSW_CONFIGLAVVIDEO = CLSW_CONFIGLAVAUDIO << 1,
    CLSW_MUTE = CLSW_CONFIGLAVVIDEO << 1,
    CLSW_VOLUME = CLSW_MUTE << 1,
    CLSW_UNRECOGNIZEDSWITCH = CLSW_VOLUME << 1 // 46
};

enum MpcCaptionState {
    MODE_SHOWCAPTIONMENU,
    MODE_HIDEMENU,
    MODE_FRAMEONLY,
    MODE_BORDERLESS,
    MODE_COUNT
}; // flags for Caption & Menu Mode

enum {
    VIDRNDT_DS_DEFAULT,
    VIDRNDT_DS_OLDRENDERER,
    VIDRNDT_DS_OVERLAYMIXER,
    VIDRNDT_DS_VMR9WINDOWED = 4,
    VIDRNDT_DS_VMR9RENDERLESS = 6,
    VIDRNDT_DS_DXR,
    VIDRNDT_DS_NULL_COMP,
    VIDRNDT_DS_NULL_UNCOMP,
    VIDRNDT_DS_EVR,
    VIDRNDT_DS_EVR_CUSTOM,
    VIDRNDT_DS_MADVR,
    VIDRNDT_DS_SYNC,
    VIDRNDT_DS_MPCVR,
};

// Enumeration for MCE remote control (careful : add 0x010000 for all keys!)
enum MCE_RAW_INPUT {
    MCE_DETAILS             = 0x010209,
    MCE_GUIDE               = 0x01008D,
    MCE_TVJUMP              = 0x010025,
    MCE_STANDBY             = 0x010082,
    MCE_OEM1                = 0x010080,
    MCE_OEM2                = 0x010081,
    MCE_MYTV                = 0x010046,
    MCE_MYVIDEOS            = 0x01004A,
    MCE_MYPICTURES          = 0x010049,
    MCE_MYMUSIC             = 0x010047,
    MCE_RECORDEDTV          = 0x010048,
    MCE_DVDANGLE            = 0x01004B,
    MCE_DVDAUDIO            = 0x01004C,
    MCE_DVDMENU             = 0x010024,
    MCE_DVDSUBTITLE         = 0x01004D,
    MCE_RED                 = 0x01005B,
    MCE_GREEN               = 0x01005C,
    MCE_YELLOW              = 0x01005D,
    MCE_BLUE                = 0x01005E,
    MCE_MEDIA_NEXTTRACK     = 0x0100B5,
    MCE_MEDIA_PREVIOUSTRACK = 0x0100B6
};

#define AUDRNDT_NULL_COMP       _T("Null Audio Renderer (Any)")
#define AUDRNDT_NULL_UNCOMP     _T("Null Audio Renderer (Uncompressed)")
#define AUDRNDT_INTERNAL        _T("Internal Audio Renderer")

#define DEFAULT_SUBTITLE_PATHS  _T(".;.\\subtitles;.\\subs")
#define DEFAULT_JUMPDISTANCE_1  1000
#define DEFAULT_JUMPDISTANCE_2  5000
#define DEFAULT_JUMPDISTANCE_3  20000


enum dvstype {
    DVS_HALF,
    DVS_NORMAL,
    DVS_DOUBLE,
    DVS_STRETCH,
    DVS_FROMINSIDE,
    DVS_FROMOUTSIDE,
    DVS_ZOOM1,
    DVS_ZOOM2
};

enum favtype {
    FAV_FILE,
    FAV_DVD,
    FAV_DEVICE
};

enum {
    TIME_TOOLTIP_ABOVE_SEEKBAR,
    TIME_TOOLTIP_BELOW_SEEKBAR
};

enum DVB_RebuildFilterGraph {
    DVB_REBUILD_FG_NEVER = 0,
    DVB_REBUILD_FG_WHEN_SWITCHING,
    DVB_REBUILD_FG_ALWAYS
};

enum DVB_StopFilterGraph {
    DVB_STOP_FG_NEVER = 0,
    DVB_STOP_FG_WHEN_SWITCHING,
    DVB_STOP_FG_ALWAYS
};

struct DisplayMode {
    bool  bValid = false;
    CSize size;
    int   bpp = 0, freq = 0;
    DWORD dwDisplayFlags = 0;

    bool operator == (const DisplayMode& dm) const {
        return (bValid == dm.bValid && size == dm.size && bpp == dm.bpp && freq == dm.freq && dwDisplayFlags == dm.dwDisplayFlags);
    };

    bool operator < (const DisplayMode& dm) const {
        bool bRet = false;

        // Ignore bValid when sorting
        if (size.cx < dm.size.cx) {
            bRet = true;
        } else if (size.cx == dm.size.cx) {
            if (size.cy < dm.size.cy) {
                bRet = true;
            } else if (size.cy == dm.size.cy) {
                if (freq < dm.freq) {
                    bRet = true;
                } else if (freq == dm.freq) {
                    if (bpp < dm.bpp) {
                        bRet = true;
                    } else if (bpp == dm.bpp) {
                        bRet = (dwDisplayFlags & DM_INTERLACED) && !(dm.dwDisplayFlags & DM_INTERLACED);
                    }
                }
            }
        }

        return bRet;
    };
};

struct AutoChangeMode {
    AutoChangeMode(bool _bChecked, double _dFrameRateStart, double _dFrameRateStop, int _msAudioDelay, DisplayMode _dm)
        : bChecked(_bChecked)
        , dFrameRateStart(_dFrameRateStart)
        , dFrameRateStop(_dFrameRateStop)
        , msAudioDelay(_msAudioDelay)
        , dm(std::move(_dm)) {
    }

    bool        bChecked;
    double      dFrameRateStart;
    double      dFrameRateStop;
    int         msAudioDelay;
    DisplayMode dm;
};

struct AutoChangeFullscreenMode {
    bool                        bEnabled = false;
    std::vector<AutoChangeMode> modes;
    bool                        bApplyDefaultModeAtFSExit = true;
    bool                        bRestoreResAfterProgExit = true;
    unsigned                    uDelay = 0u;
};

struct wmcmd_base : public ACCEL {
    BYTE mouse;
    BYTE mouseFS;
    DWORD dwname;
    UINT appcmd;

    enum : BYTE {
        NONE,
        LDOWN,
        LUP,
        LDBLCLK,
        MDOWN,
        MUP,
        MDBLCLK,
        RDOWN,
        RUP,
        RDBLCLK,
        X1DOWN,
        X1UP,
        X1DBLCLK,
        X2DOWN,
        X2UP,
        X2DBLCLK,
        WUP,
        WDOWN,
        LAST
    };

    wmcmd_base()
        : ACCEL( {
        0, 0, 0
    })
    , mouse(NONE)
    , mouseFS(NONE)
    , dwname(0)
    , appcmd(0) {}

    constexpr wmcmd_base(WORD _cmd, WORD _key, BYTE _fVirt, DWORD _dwname, UINT _appcmd = 0, BYTE _mouse = NONE, BYTE _mouseFS = NONE)
        : ACCEL{ _fVirt, _key, _cmd }
        , mouse(_mouse)
        , mouseFS(_mouseFS)
        , dwname(_dwname)
        , appcmd(_appcmd) {}

    constexpr wmcmd_base(const wmcmd_base&) = default;
    constexpr wmcmd_base(wmcmd_base&&) = default;
    wmcmd_base& operator=(const wmcmd_base&) = default;
    wmcmd_base& operator=(wmcmd_base&&) = default;
};

class wmcmd : public wmcmd_base
{
    const wmcmd_base* default_cmd = nullptr;

public:
    CStringA rmcmd;
    int rmrepcnt = 5;

    wmcmd() = default;
    wmcmd& operator=(const wmcmd&) = default;
    wmcmd& operator=(wmcmd&&) = default;

    explicit wmcmd(const wmcmd_base& cmd)
        : wmcmd_base(cmd)
        , default_cmd(&cmd)
        , rmrepcnt(5) {
    }

    bool operator == (const wmcmd& wc) const {
        return cmd > 0 && cmd == wc.cmd;
    }

    CString GetName() const {
        return ResStr(dwname);
    }

    void Restore() {
        ASSERT(default_cmd);
        *static_cast<ACCEL*>(this) = *static_cast<const ACCEL*>(default_cmd);
        appcmd = default_cmd->appcmd;
        mouse = default_cmd->mouse;
        mouseFS = default_cmd->mouseFS;
        rmcmd.Empty();
        rmrepcnt = 5;
    }

    bool IsModified() const {
        ASSERT(default_cmd);
        return memcmp(static_cast<const ACCEL*>(this), static_cast<const ACCEL*>(default_cmd), sizeof(ACCEL)) ||
               appcmd != default_cmd->appcmd ||
               mouse != default_cmd->mouse ||
               mouseFS != default_cmd->mouseFS ||
               !rmcmd.IsEmpty() ||
               rmrepcnt != 5;
    }
};

class CRemoteCtrlClient : public CAsyncSocket
{
protected:
    CCritSec m_csLock;
    CWnd* m_pWnd;
    enum {
        DISCONNECTED,
        CONNECTED,
        CONNECTING
    } m_nStatus;
    CString m_addr;

    virtual void OnConnect(int nErrorCode);
    virtual void OnClose(int nErrorCode);
    virtual void OnReceive(int nErrorCode);

    virtual void OnCommand(CStringA str) = 0;

    void ExecuteCommand(CStringA cmd, int repcnt);

public:
    CRemoteCtrlClient();
    void SetHWND(HWND hWnd);
    void Connect(CString addr);
    void DisConnect();
    int GetStatus() const {
        return m_nStatus;
    }
};

class CWinLircClient : public CRemoteCtrlClient
{
protected:
    virtual void OnCommand(CStringA str);

public:
    CWinLircClient();
};

class CUIceClient : public CRemoteCtrlClient
{
protected:
    virtual void OnCommand(CStringA str);

public:
    CUIceClient();
};

#define APPSETTINGS_VERSION 8

class CAppSettings
{
    bool bInitialized;

    class CRecentFileAndURLList : public CRecentFileList
    {
    public:
        CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
                              LPCTSTR lpszEntryFormat, int nSize,
                              int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

        virtual void Add(LPCTSTR lpszPathName); // we have to override CRecentFileList::Add because the original version can't handle URLs

        void SetSize(int nSize);
    };

public:
    // cmdline params
    UINT64 nCLSwitches;
    CAtlList<CString>   slFiles, slDubs, slSubs, slFilters;

    // Initial position (used by command line flags)
    REFERENCE_TIME      rtShift;
    REFERENCE_TIME      rtStart;
    ULONG               lDVDTitle;
    ULONG               lDVDChapter;
    DVD_HMSF_TIMECODE   DVDPosition;

    CSize sizeFixedWindow;
    bool HasFixedWindowSize() const {
        return sizeFixedWindow.cx > 0 || sizeFixedWindow.cy > 0;
    }
    //int           iFixedWidth, iFixedHeight;
    int             iMonitor;

    CString         ParseFileName(CString const& param);
    void            ParseCommandLine(CAtlList<CString>& cmdln);

    // Added a Debug display to the screen (/debug option)
    bool            fShowDebugInfo;
    int             iAdminOption;


    // Player
    bool            fAllowMultipleInst;
    bool            fTrayIcon;
    bool            fShowOSD;
    bool            fLimitWindowProportions;
    bool            fSnapToDesktopEdges;
    bool            fHideCDROMsSubMenu;
    DWORD           dwPriority;
    int             iTitleBarTextStyle;
    bool            fTitleBarTextTitle;
    bool            fKeepHistory;
    int             iRecentFilesNumber;
    CRecentFileAndURLList MRU;
    CRecentFileAndURLList MRUDub;
    CFilePositionList filePositions;
    CDVDPositionList  dvdPositions;
    bool            fRememberDVDPos;
    bool            fRememberFilePos;
    int             iRememberPosForLongerThan;
    bool            bRememberPosForAudioFiles;
    bool            bRememberPlaylistItems;
    bool            fRememberWindowPos;
    CRect           rcLastWindowPos;
    bool            fRememberWindowSize;
    bool            fSavePnSZoom;
    double          dZoomX;
    double          dZoomY;

    // Formats
    CMediaFormats   m_Formats;
    bool            fAssociatedWithIcons;

    // Keys
    CList<wmcmd>    wmcmds;
    HACCEL          hAccel;
    bool            fWinLirc;
    CString         strWinLircAddr;
    CWinLircClient  WinLircClient;
    bool            fUIce;
    CString         strUIceAddr;
    CUIceClient     UIceClient;
    bool            fGlobalMedia;

    // Logo
    UINT            nLogoId;
    bool            fLogoExternal;
    CString         strLogoFileName;

    // Web Inteface
    bool            fEnableWebServer;
    int             nWebServerPort;
    int             nCmdlnWebServerPort;
    bool            fWebServerUseCompression;
    bool            fWebServerLocalhostOnly;
    bool            bWebUIEnablePreview;
    bool            fWebServerPrintDebugInfo;
    CString         strWebRoot, strWebDefIndex;
    CString         strWebServerCGI;

    // Playback
    int             nVolume;
    bool            fMute;
    int             nBalance;
    int             nLoops;
    bool            fLoopForever;

    enum class LoopMode {
        FILE,
        PLAYLIST
    } eLoopMode;

    bool            fRememberZoomLevel;
    int             nAutoFitFactor;
    int             iZoomLevel;
    CStringW        strAudiosLanguageOrder;
    CStringW        strSubtitlesLanguageOrder;
    bool            fEnableWorkerThreadForOpening;
    bool            fReportFailedPins;
    bool            fAutoloadAudio;
    bool            fBlockVSFilter;
    UINT            nVolumeStep;
    UINT            nSpeedStep;
    int             nDefaultToolbarSize;
    bool            bSaveImagePosition;
    bool            bSaveImageCurrentTime;
    bool            bAllowInaccurateFastseek;
    bool            bLoopFolderOnPlayNextFile;

    enum class AfterPlayback {
        DO_NOTHING,
        PLAY_NEXT,
        REWIND,
        MONITOROFF,
        CLOSE,
        EXIT
    } eAfterPlayback;

    // DVD/OGM
    bool            fUseDVDPath;
    CString         strDVDPath;
    LCID            idMenuLang, idAudioLang, idSubtitlesLang;
    bool            fClosedCaptions;

    // Output
    CRenderersSettings m_RenderersSettings;
    int             iDSVideoRendererType;
    int             iRMVideoRendererType;
    int             iQTVideoRendererType;

    CStringW        strAudioRendererDisplayName;
    bool            fD3DFullscreen;

    // Fullscreen
    bool            fLaunchfullscreen;
    bool            bHideFullscreenControls;
    enum class HideFullscreenControlsPolicy {
        SHOW_NEVER,
        SHOW_WHEN_HOVERED,
        SHOW_WHEN_CURSOR_MOVED,
    } eHideFullscreenControlsPolicy;
    unsigned        uHideFullscreenControlsDelay;
    bool            bHideFullscreenDockedPanels;
    bool            fExitFullScreenAtTheEnd;
    CStringW        strFullScreenMonitor;
    AutoChangeFullscreenMode autoChangeFSMode;

    // Sync Renderer Settings

    // Capture (BDA configuration)
    int             iDefaultCaptureDevice;      // Default capture device (analog=0, 1=digital)
    CString         strAnalogVideo;
    CString         strAnalogAudio;
    int             iAnalogCountry;
    CString         strBDANetworkProvider;
    CString         strBDATuner;
    CString         strBDAReceiver;
    //CString           strBDAStandard;
    int             iBDAScanFreqStart;
    int             iBDAScanFreqEnd;
    int             iBDABandwidth;
    bool            fBDAUseOffset;
    int             iBDAOffset;
    bool            fBDAIgnoreEncryptedChannels;
    int             nDVBLastChannel;
    std::vector<CDVBChannel> m_DVBChannels;
    DVB_RebuildFilterGraph nDVBRebuildFilterGraph;
    DVB_StopFilterGraph nDVBStopFilterGraph;

    // Internal Filters
    bool            SrcFilters[SRC_LAST + !SRC_LAST];
    bool            TraFilters[TRA_LAST + !TRA_LAST];

    // Audio Switcher
    bool            fEnableAudioSwitcher;
    bool            fAudioNormalize;
    UINT            nAudioMaxNormFactor;
    bool            fAudioNormalizeRecover;
    UINT            nAudioBoost;
    bool            fDownSampleTo441;
    bool            fAudioTimeShift;
    int             iAudioTimeShift;
    bool            fCustomChannelMapping;
    int             nSpeakerChannels;
    DWORD           pSpeakerToChannelMap[AS_MAX_CHANNELS][AS_MAX_CHANNELS];

    // External Filters
    CAutoPtrList<FilterOverride> m_filters;

    // Subtitles
    bool            fOverridePlacement;
    int             nHorPos, nVerPos;
    bool            bSubtitleARCompensation;
    int             nSubDelayStep;

    // Default Style
    STSStyle        subtitlesDefStyle;

    // Misc
    bool            bPreferDefaultForcedSubtitles;
    bool            fPrioritizeExternalSubtitles;
    bool            fDisableInternalSubtitles;
    bool            bAllowOverridingExternalSplitterChoice;
    bool            bAutoDownloadSubtitles;
    int             nAutoDownloadScoreMovies;
    int             nAutoDownloadScoreSeries;
    CString         strAutoDownloadSubtitlesExclude;
    bool            bAutoUploadSubtitles;
    bool            bPreferHearingImpairedSubtitles;
    bool            bMPCThemeLoaded;
    bool            bMPCTheme;
    bool            bWindows10DarkThemeActive;
    bool            bWindows10AccentColorsEnabled;
    bool            bModernSeekbar;
    int             iModernSeekbarHeight;

    enum class verticalAlignVideoType {
        ALIGN_MIDDLE,
        ALIGN_TOP,
        ALIGN_BOTTOM
    } eVerticalAlignVideoType;
    verticalAlignVideoType iVerticalAlignVideo;

    CString         strSubtitlesProviders;
    CString         strSubtitlePaths;

    // Tweaks
    int             nJumpDistS;
    int             nJumpDistM;
    int             nJumpDistL;
    bool            bFastSeek;
    enum { FASTSEEK_LATEST_KEYFRAME, FASTSEEK_NEAREST_KEYFRAME } eFastSeekMethod;
    bool            fShowChapters;
    bool            bNotifySkype;
    bool            fPreventMinimize;
    bool            bUseEnhancedTaskBar;
    bool            fLCDSupport;
    bool            fUseSearchInFolder;
    bool            fUseTimeTooltip;
    int             nTimeTooltipPosition;
    CString         strOSDFont;
    int             nOSDSize;
    bool            bHideWindowedMousePointer;

    // Miscellaneous
    int             iBrightness;
    int             iContrast;
    int             iHue;
    int             iSaturation;
    int             nUpdaterAutoCheck;
    int             nUpdaterDelay;

    // MENUS
    // View
    MpcCaptionState eCaptionMenuMode;
    bool            fHideNavigation;
    UINT            nCS; // Control state for toolbars
    // Language
    LANGID          language;
    // Subtitles menu
    bool            fEnableSubtitles;
    bool            fUseDefaultSubtitlesStyle;
    // Video Frame
    int             iDefaultVideoSize;
    bool            fKeepAspectRatio;
    bool            fCompMonDeskARDiff;
    // Pan&Scan
    CString         strPnSPreset;
    CStringArray    m_pnspresets;
    // On top menu
    int             iOnTop;

    // WINDOWS
    // Add Favorite
    bool            bFavRememberPos;
    bool            bFavRelativeDrive;
    // Save Image...
    CString         strSnapshotPath, strSnapshotExt;
    // Save Thumbnails...
    int             iThumbRows, iThumbCols, iThumbWidth;
    // Save Subtitle
    bool            bSubSaveExternalStyleFile;
    // Shaders
    ShaderList      m_ShadersExtraList;
    ShaderSelection m_Shaders;
    // Playlist (contex menu)
    bool            bShufflePlaylistItems;
    bool            bHidePlaylistFullScreen;

    // OTHER STATES
    CStringW        strLastOpenDir;
    UINT            nLastWindowType;
    WORD            nLastUsedPage;
    bool            fRemainingTime;
    bool            bHighPrecisionTimer;
    bool            fLastFullScreen;

    bool            fIntRealMedia;
    bool            fEnableEDLEditor;

    HWND            hMasterWnd;

    bool            bHideWindowedControls;

    int             nJpegQuality;

    bool            bEnableCoverArt;
    int             nCoverArtSizeLimit;

    bool            bEnableLogging;
    bool            bUseLegacyToolbar;

    bool            IsD3DFullscreen() const;
    CString         SelectedAudioRenderer() const;
    bool            IsISRAutoLoadEnabled() const;
    bool            IsInitialized() const;
    static bool     IsVideoRendererAvailable(int iVideoRendererType);

    CFileAssoc      fileAssoc;

    CComPtr<SaneAudioRenderer::ISettings> sanear;

    DWORD           iLAVGPUDevice;
    unsigned        nCmdVolume;

    enum class SubtitleRenderer {
        INTERNAL,
        VS_FILTER,
        XY_SUB_FILTER,
        ASS_FILTER,
    };

    SubtitleRenderer GetSubtitleRenderer() const;
    void  SetSubtitleRenderer(SubtitleRenderer renderer) {
        eSubtitleRenderer = renderer;
    }

    static bool IsSubtitleRendererRegistered(SubtitleRenderer eSubtitleRenderer);

    static bool IsSubtitleRendererSupported(SubtitleRenderer eSubtitleRenderer, int videoRenderer);

    CSize GetAspectRatioOverride() const {
        ASSERT(fKeepAspectRatio && "Keep Aspect Ratio option have to be enabled if override value is used.");
        return sizeAspectRatio;
    };
    void SetAspectRatioOverride(const CSize& ar) {
        sizeAspectRatio = ar;
    }

    // YoutubeDL settings
    bool bUseYDL;
    int iYDLMaxHeight;
    int iYDLVideoFormat;
    bool bYDLAudioOnly;
    CString sYDLCommandLine;

private:
    struct FilterKey {
        CString name;
        bool bDefault;

        FilterKey()
            : name()
            , bDefault(false) {
        }

        FilterKey(CString name, bool bDefault)
            : name(name)
            , bDefault(bDefault) {
        }
    };

    FilterKey       SrcFiltersKeys[SRC_LAST + !SRC_LAST];
    FilterKey       TraFiltersKeys[TRA_LAST + !TRA_LAST];

    __int64         ConvertTimeToMSec(const CString& time) const;
    void            ExtractDVDStartPos(CString& strParam);

    void            CreateCommands();

    void            SaveExternalFilters(CAutoPtrList<FilterOverride>& filters, LPCTSTR baseKey = IDS_R_EXTERNAL_FILTERS);
    void            LoadExternalFilters(CAutoPtrList<FilterOverride>& filters, LPCTSTR baseKey = IDS_R_EXTERNAL_FILTERS);
    void            ConvertOldExternalFiltersList();

    void            SaveSettingsAutoChangeFullScreenMode();

    void            UpdateRenderersData(bool fSave);
    friend void     CRenderersSettings::UpdateData(bool bSave);

    SubtitleRenderer eSubtitleRenderer;
    CSize            sizeAspectRatio;

public:
    CAppSettings();
    CAppSettings(const CAppSettings&) = delete;
    ~CAppSettings();

    CAppSettings& operator = (const CAppSettings&) = delete;

    void            SaveSettings();
    void            LoadSettings();
    void            SaveExternalFilters() {
        if (bInitialized) {
            SaveExternalFilters(m_filters);
        }
    };
    void            UpdateSettings();

    void            SetAsUninitialized() {
        bInitialized = false;
    };

    void            GetFav(favtype ft, CAtlList<CString>& sl) const;
    void            SetFav(favtype ft, CAtlList<CString>& sl);
    void            AddFav(favtype ft, CString s);

    CDVBChannel*    FindChannelByPref(int nPrefNumber);

    bool            GetAllowMultiInst() const;

    static bool     IsVSFilterInstalled();
};
