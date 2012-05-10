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

#pragma once

#include "SettingsDefines.h"
#include "FilterEnum.h"
#include "RenderersSettings.h"
#include "../../Subtitles/STS.h"
#include "MediaFormats.h"
#include "DVBChannel.h"

#include <afxsock.h>

// flags for AppSettings::nCS
enum {
	CS_NONE=0,
	CS_SEEKBAR=1,
	CS_TOOLBAR=CS_SEEKBAR<<1,
	CS_INFOBAR=CS_TOOLBAR<<1,
	CS_STATSBAR=CS_INFOBAR<<1,
	CS_STATUSBAR=CS_STATSBAR<<1,
	CS_LAST=CS_STATUSBAR
};

enum {
	CLSW_NONE=0,
	CLSW_OPEN=1,
	CLSW_PLAY=CLSW_OPEN<<1,
	CLSW_CLOSE=CLSW_PLAY<<1,
	CLSW_STANDBY=CLSW_CLOSE<<1,
	CLSW_HIBERNATE=CLSW_STANDBY<<1,
	CLSW_SHUTDOWN=CLSW_HIBERNATE<<1,
	CLSW_LOGOFF=CLSW_SHUTDOWN<<1,
	CLSW_LOCK=CLSW_LOGOFF<<1,
	CLSW_AFTERPLAYBACK_MASK=CLSW_CLOSE|CLSW_STANDBY|CLSW_SHUTDOWN|CLSW_HIBERNATE|CLSW_LOGOFF|CLSW_LOCK,
	CLSW_FULLSCREEN=CLSW_LOCK<<1,
	CLSW_NEW=CLSW_FULLSCREEN<<1,
	CLSW_HELP=CLSW_NEW<<1,
	CLSW_DVD=CLSW_HELP<<1,
	CLSW_CD=CLSW_DVD<<1,
	CLSW_ADD=CLSW_CD<<1,
	CLSW_MINIMIZED=CLSW_ADD<<1,
	CLSW_REGEXTVID=CLSW_MINIMIZED<<1,		// 16
	CLSW_REGEXTAUD=CLSW_REGEXTVID<<1,
	CLSW_REGEXTPL=CLSW_REGEXTAUD<<1,
	CLSW_UNREGEXT=CLSW_REGEXTPL<<1,
	CLSW_STARTVALID=CLSW_UNREGEXT<<2,
	CLSW_NOFOCUS=CLSW_STARTVALID<<1,
	CLSW_FIXEDSIZE=CLSW_NOFOCUS<<1,
	CLSW_MONITOR=CLSW_FIXEDSIZE<<1,
	CLSW_D3DFULLSCREEN=CLSW_MONITOR<<1,
	CLSW_ADMINOPTION=CLSW_D3DFULLSCREEN<<1,
	CLSW_SLAVE=CLSW_ADMINOPTION<<1,
	CLSW_AUDIORENDERER=CLSW_SLAVE<<1,
	CLSW_RESET=CLSW_AUDIORENDERER<<1,
	CLSW_UNRECOGNIZEDSWITCH=CLSW_RESET<<1 // 30
};

enum {
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
	VIDRNDT_DS_VMR7WINDOWED,
	VIDRNDT_DS_VMR9WINDOWED,
	VIDRNDT_DS_VMR7RENDERLESS,
	VIDRNDT_DS_VMR9RENDERLESS,
	VIDRNDT_DS_DXR,
	VIDRNDT_DS_NULL_COMP,
	VIDRNDT_DS_NULL_UNCOMP,
	VIDRNDT_DS_EVR,
	VIDRNDT_DS_EVR_CUSTOM,
	VIDRNDT_DS_MADVR,
	VIDRNDT_DS_SYNC
};

// Enumeration for MCE remote control (careful : add 0x010000 for all keys!)
enum MCE_RAW_INPUT {
	MCE_DETAILS				= 0x010209,
	MCE_GUIDE				= 0x01008D,
	MCE_TVJUMP				= 0x010025,
	MCE_STANDBY				= 0x010082,
	MCE_OEM1				= 0x010080,
	MCE_OEM2				= 0x010081,
	MCE_MYTV				= 0x010046,
	MCE_MYVIDEOS			= 0x01004A,
	MCE_MYPICTURES			= 0x010049,
	MCE_MYMUSIC				= 0x010047,
	MCE_RECORDEDTV			= 0x010048,
	MCE_DVDANGLE			= 0x01004B,
	MCE_DVDAUDIO			= 0x01004C,
	MCE_DVDMENU				= 0x010024,
	MCE_DVDSUBTITLE			= 0x01004D,
	MCE_RED					= 0x01005B,
	MCE_GREEN				= 0x01005C,
	MCE_YELLOW				= 0x01005D,
	MCE_BLUE				= 0x01005E,
	MCE_MEDIA_NEXTTRACK		= 0x0100B5,
	MCE_MEDIA_PREVIOUSTRACK	= 0x0100B6,
};

#define AUDRNDT_NULL_COMP _T("Null Audio Renderer (Any)")
#define AUDRNDT_NULL_UNCOMP _T("Null Audio Renderer (Uncompressed)")
#define AUDRNDT_MPC _T("MPC Audio Renderer")

#define DEFAULT_SUBTITLE_PATHS _T(".;.\\subtitles;.\\subs")
#define DEFAULT_JUMPDISTANCE_1  1000
#define DEFAULT_JUMPDISTANCE_2  5000
#define DEFAULT_JUMPDISTANCE_3 20000


typedef enum {
	DVS_HALF,
	DVS_NORMAL,
	DVS_DOUBLE,
	DVS_STRETCH,
	DVS_FROMINSIDE,
	DVS_FROMOUTSIDE,
	DVS_ZOOM1,
	DVS_ZOOM2
} dvstype;

typedef enum {
	FAV_FILE,
	FAV_DVD,
	FAV_DEVICE
} favtype;

#define MAX_DVD_POSITION 20
typedef struct {
	ULONGLONG			llDVDGuid;
	ULONG				lTitle;
	DVD_HMSF_TIMECODE	Timecode;
} DVD_POSITION;

#define MAX_FILE_POSITION 20
typedef struct {
	CString				strFile;
	LONGLONG			llPosition;
} FILE_POSITION;

enum {
	TIME_TOOLTIP_ABOVE_SEEKBAR,
	TIME_TOOLTIP_BELOW_SEEKBAR
};

#pragma pack(push, 1)
typedef struct {
	bool fValid;
	CSize size;
	int bpp, freq;
	DWORD dmDisplayFlags;
} dispmode;

typedef struct {
	double vfr_from;
	double vfr_to;
	bool fChecked;
	dispmode dmFSRes;
	bool fIsData;
} fpsmode;

#define MaxFpsCount 30
typedef struct {
	bool bEnabled;
	fpsmode dmFullscreenRes[MaxFpsCount];
	bool bApplyDefault;
}	AChFR; //AutoChangeFullscrRes
#pragma pack(pop)

class wmcmd : public ACCEL
{
	ACCEL backup;
	UINT appcmdorg;
	UINT mouseorg;
	UINT mouseFSorg;

public:
	DWORD dwname;
	UINT appcmd;
	enum {NONE,LDOWN,LUP,LDBLCLK,MDOWN,MUP,MDBLCLK,RDOWN,RUP,RDBLCLK,X1DOWN,X1UP,X1DBLCLK,X2DOWN,X2UP,X2DBLCLK,WUP,WDOWN,LAST};
	UINT mouse;
	UINT mouseFS;
	CStringA rmcmd;
	int rmrepcnt;

	wmcmd(WORD cmd = 0) {
		this->cmd = cmd;
	}

	wmcmd(WORD cmd, WORD key, BYTE fVirt, DWORD dwname, UINT appcmd = 0, UINT mouse = NONE, UINT mouseFS = NONE, LPCSTR rmcmd = "", int rmrepcnt = 5) {
		this->cmd = cmd;
		this->key = key;
		this->fVirt = fVirt;
		this->appcmd = appcmdorg = appcmd;
		this->dwname = dwname;
		this->mouse = mouseorg = mouse;
		this->mouseFS = mouseFSorg = mouseFS;
		this->rmcmd = rmcmd;
		this->rmrepcnt = rmrepcnt;
		backup = *this;
	}

	bool operator == (const wmcmd& wc) const {
		return(cmd > 0 && cmd == wc.cmd);
	}

	CString GetName() const {
		return ResStr (dwname);
	}

	void Restore() {
		*(ACCEL*)this = backup;
		appcmd = appcmdorg;
		mouse = mouseorg;
		mouseFS = mouseFSorg;
		rmcmd.Empty();
		rmrepcnt = 5;
	}

	bool IsModified() const {
		return (memcmp((const ACCEL*)this, &backup, sizeof(ACCEL)) || appcmd != appcmdorg || mouse != mouseorg || mouseFS != mouseFSorg || !rmcmd.IsEmpty() || rmrepcnt != 5);
	}
};

class CRemoteCtrlClient : public CAsyncSocket
{
protected:
	CCritSec m_csLock;
	CWnd* m_pWnd;
	enum {DISCONNECTED, CONNECTED, CONNECTING} m_nStatus;
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
	int GetStatus() const { return(m_nStatus); }
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

class CAppSettings
{
	bool fInitialized;

	class CRecentFileAndURLList : public CRecentFileList
	{
	public:
		CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
							  LPCTSTR lpszEntryFormat, int nSize,
							  int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

		virtual void Add(LPCTSTR lpszPathName); // we have to override CRecentFileList::Add because the original version can't handle URLs
	};

public:
	// cmdline params
	UINT nCLSwitches;
	CAtlList<CString>	slFiles, slDubs, slSubs, slFilters;

	// Initial position (used by command line flags)
	__int64				rtShift;
	__int64				rtStart;
	ULONG				lDVDTitle;
	ULONG				lDVDChapter;
	DVD_HMSF_TIMECODE	DVDPosition;

	CSize sizeFixedWindow;
	bool HasFixedWindowSize() const { return sizeFixedWindow.cx > 0 || sizeFixedWindow.cy > 0; }
	//int			iFixedWidth, iFixedHeight;
	int				iMonitor;

	CString			strPnSPreset;

	CString			ParseFileName(CString const& param);
	void			ParseCommandLine(CAtlList<CString>& cmdln);

	// Added a Debug display to the screen (/debug option)
	bool			fShowDebugInfo;

	int				iDXVer;
	int				iAdminOption;

	UINT			nCS; // Control state for toolbars
	int				iCaptionMenuMode; // normal -> hidemenu -> frameonly -> borderless
	bool			fHideNavigation;
	int				iDefaultVideoSize;
	bool			fKeepAspectRatio;
	bool			fCompMonDeskARDiff;

	CRecentFileAndURLList MRU;
	CRecentFileAndURLList MRUDub;

	CAutoPtrList<FilterOverride> m_filters;

	CRenderersSettings m_RenderersSettings;

	int				iDSVideoRendererType;
	int				iRMVideoRendererType;
	int				iQTVideoRendererType;

	int				nVolume;
	int				nBalance;
	bool			fMute;
	int				nLoops;
	bool			fLoopForever;
	bool			fRewind;
	int				iZoomLevel;
	//int			iVideoRendererType;
	CStringW		strAudioRendererDisplayName;
	bool			fAutoloadAudio;
	bool			fAutoloadSubtitles;
	bool			fBlockVSFilter;
	bool			fEnableWorkerThreadForOpening;
	bool			fReportFailedPins;

	CStringW		strFullScreenMonitor;
	bool			fAssociatedWithIcons;
	CStringW		strLastOpenDir;

	bool			fAllowMultipleInst;
	int				iTitleBarTextStyle;
	bool			fTitleBarTextTitle;
	int				iOnTop;
	bool			fTrayIcon;
	bool			fRememberZoomLevel;
	bool			fShowBarsWhenFullScreen;
	int				nShowBarsWhenFullScreenTimeOut;
	AChFR			AutoChangeFullscrRes;
	bool			fExitFullScreenAtTheEnd;
	bool			fRestoreResAfterExit;
	bool			fRememberWindowPos;
	bool			fRememberWindowSize;
	bool			fSavePnSZoom;
	float			dZoomX;
	float			dZoomY;
	bool			fSnapToDesktopEdges;
	CRect			rcLastWindowPos;
	UINT			nLastWindowType;
	CSize			sizeAspectRatio;
	bool			fKeepHistory;
	UINT			nLastUsedPage;

	bool			bShufflePlaylistItems;
	bool			bRememberPlaylistItems;
	bool			bHidePlaylistFullScreen;
	bool			bFavRememberPos;
	bool			bFavRelativeDrive;

	CString			strDVDPath;
	bool			fUseDVDPath;
	LCID			idMenuLang, idAudioLang, idSubtitlesLang;
	bool			fAutoSpeakerConf;
	bool			fClosedCaptions;

	STSStyle		subdefstyle;
	bool			fOverridePlacement;
	int				nHorPos, nVerPos;
	int				nSubDelayInterval;
	bool			fEnableSubtitles;
	bool			fUseDefaultSubtitlesStyle;
	bool			fPrioritizeExternalSubtitles;
	bool			fDisableInternalSubtitles;
	CString			strSubtitlePaths;

	int				nJumpDistS;
	int				nJumpDistM;
	int				nJumpDistL;
	bool			fLimitWindowProportions;
	bool			fNotifyMSN;

	bool			fEnableAudioSwitcher;
	bool			fDownSampleTo441;
	bool			fAudioTimeShift;
	int				iAudioTimeShift;
	bool			fCustomChannelMapping;
	DWORD			pSpeakerToChannelMap[18][18];
	bool			fAudioNormalize;
	bool			fAudioNormalizeRecover;
	float			dAudioBoost_dB;

	bool			fIntRealMedia;
	//bool			fRealMediaRenderless;
	int				iQuickTimeRenderer;
	//float			dRealMediaQuickTimeFPS;

	CStringArray	m_pnspresets;

	CList<wmcmd>	wmcmds;
	HACCEL			hAccel;

	bool			fWinLirc;
	CString			strWinLircAddr;
	CWinLircClient	WinLircClient;
	bool			fUIce;
	CString			strUIceAddr;
	CUIceClient		UIceClient;
	bool			fGlobalMedia;

	CMediaFormats	m_Formats;

	bool			SrcFilters[SRC_LAST + !SRC_LAST];
	bool			TraFilters[TRA_LAST + !TRA_LAST];
	bool			DXVAFilters[TRA_DXVA_LAST + !TRA_DXVA_LAST];
	bool			FFmpegFilters[FFM_LAST + !FFM_LAST];

	CString			strLogoFileName;
	UINT			nLogoId;
	bool			fLogoExternal;

	bool			fHideCDROMsSubMenu;

	DWORD			dwPriority;
	bool			fLaunchfullscreen;

	BOOL			fEnableWebServer;
	int				nWebServerPort;
	int				nCmdlnWebServerPort;
	bool			fWebServerPrintDebugInfo;
	bool			fWebServerUseCompression;
	bool			fWebServerLocalhostOnly;
	CString			strWebRoot, strWebDefIndex;
	CString			strWebServerCGI;

	CString			strSnapShotPath, strSnapShotExt;
	int				iThumbRows, iThumbCols, iThumbWidth;

	CString			strISDb;

	struct Shader {
		CString		label;
		CString		target;
		CString		srcdata;
	};
	CAtlList<Shader> m_shaders;
	CString			strShadercombine;
	CString			strShadercombineScreenSpace;

	// Casimir666 : new settings
	bool			fD3DFullscreen;
	bool			fMonitorAutoRefreshRate;
	bool			fLastFullScreen;
	bool			fEnableEDLEditor;
	int				iBrightness;
	int				iContrast;
	int				iHue;
	int				iSaturation;
	CString			strShaderList;
	CString			strShaderListScreenSpace;
	bool			fToggleShader;
	bool			fToggleShaderScreenSpace;

	bool			fRememberDVDPos;
	bool			fRememberFilePos;
	bool			fShowOSD;
	int				iLanguage;
	bool			fFastSeek;

	// BDA configuration
	int				iDefaultCaptureDevice;		// Default capture device (analog=0, 1=digital)
	CString			strAnalogVideo;
	CString			strAnalogAudio;
	int				iAnalogCountry;
	CString			strBDANetworkProvider;
	CString			strBDATuner;
	CString			strBDAReceiver;
	//CString			strBDAStandard;
	int				iBDAScanFreqStart;
	int				iBDAScanFreqEnd;
	int				iBDABandwidth;
	bool			fBDAUseOffset;
	int				iBDAOffset;
	bool			fBDAIgnoreEncryptedChannels;
	UINT			nDVBLastChannel;
	CAtlList<CDVBChannel> m_DVBChannels;

	HWND			hMasterWnd;

	bool			IsD3DFullscreen() const;
	CString			SelectedAudioRenderer() const;
	void			ResetPositions();
	DVD_POSITION*	CurrentDVDPosition();
	bool			NewDvd(ULONGLONG llDVDGuid);
	FILE_POSITION*	CurrentFilePosition();
	bool			NewFile(LPCTSTR strFileName);

	void			SaveCurrentDVDPosition();
	void			ClearDVDPositions();
	void			SaveCurrentFilePosition();
	void			ClearFilePositions();

	void			DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize) const;
	CString			SerializeHex (BYTE* pBuffer, int nBufSize) const;

private :
	DVD_POSITION	DvdPosition[MAX_DVD_POSITION];
	int				nCurrentDvdPosition;
	FILE_POSITION	FilePosition[MAX_FILE_POSITION];
	int				nCurrentFilePosition;

	CString		SrcFiltersKeys[SRC_LAST + !SRC_LAST];
	CString		TraFiltersKeys[TRA_LAST + !TRA_LAST];
	CString		DXVAFiltersKeys[TRA_DXVA_LAST + !TRA_DXVA_LAST];
	CString		FFMFiltersKeys[FFM_LAST + !FFM_LAST];

	__int64			ConvertTimeToMSec(CString& time) const;
	void			ExtractDVDStartPos(CString& strParam);

	void			CreateCommands();

public:
	CAppSettings();
	virtual ~CAppSettings();
	void			UpdateData(bool fSave);

	void			GetFav(favtype ft, CAtlList<CString>& sl);
	void			SetFav(favtype ft, CAtlList<CString>& sl);
	void			AddFav(favtype ft, CString s);
	CDVBChannel*	FindChannelByPref(int nPrefNumber);

	bool			fPreventMinimize;
	bool			fUseWin7TaskBar;
	bool			fExitAfterPlayback;
	bool			fNextInDirAfterPlayback;
	bool			fUseSearchInFolder;
	bool			fUseTimeTooltip;
	int				nTimeTooltipPosition;
	int				nOSDSize;
	CString			strOSDFont;
	CStringW		strSubtitlesLanguageOrder;
	CStringW		strAudiosLanguageOrder;

	int				nSpeakerChannels;

	bool			fRemainingTime;

	bool			GetAllowMultiInst();
private:
	void			UpdateRenderersData(bool fSave);
	friend	void	CRenderersSettings::UpdateData(bool bSave);
};
