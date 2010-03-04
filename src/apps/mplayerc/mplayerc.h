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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <afxadv.h>
#include <atlsync.h>
#include "../../subtitles/STS.h"
#include "MediaFormats.h"
#include "FakeFilterMapper2.h"
#include "DVBChannel.h"

#ifdef UNICODE
#define MPC_WND_CLASS_NAME L"MediaPlayerClassicW"
#else
#define MPC_WND_CLASS_NAME "MediaPlayerClassicA"
#endif

enum 
{
	WM_GRAPHNOTIFY = WM_APP+1,
	WM_REARRANGERENDERLESS,
	WM_RESUMEFROMSTATE,
	WM_TUNER_SCAN_PROGRESS,
	WM_TUNER_SCAN_END,
	WM_TUNER_STATS,
	WM_TUNER_NEW_CHANNEL
};

#define WM_MYMOUSELAST WM_XBUTTONDBLCLK

///////////////

extern void CorrectComboListWidth(CComboBox& box, CFont* pWndFont);
extern HICON LoadIcon(CString fn, bool fSmall);
extern bool LoadType(CString fn, CString& type);
extern bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype);
extern CStringA GetContentType(CString fn, CAtlList<CString>* redir = NULL);

/////////////////////////////////////////////////////////////////////////////
// Casimir666
//
typedef enum
{	
	Brightness	= 0x1,
	Contrast	= 0x2,
	Hue			= 0x4,
	Saturation	= 0x8,
} 	ControlType;

typedef struct // _VMR9ProcAmpControlRange
{
	DWORD dwSize;
	DWORD dwProperty;
	float MinValue;
	float MaxValue;
	float DefaultValue;
	float StepSize;
} 	COLORPROPERTY_RANGE;

#define MAX_DVD_POSITION		20
typedef struct
{
	ULONGLONG			llDVDGuid;
	ULONG				lTitle;
	DVD_HMSF_TIMECODE	Timecode;
} DVD_POSITION;

#define MAX_FILE_POSITION		20
typedef struct
{
	CString				strFile;
	LONGLONG			llPosition;
} FILE_POSITION;


/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp:
// See mplayerc.cpp for the implementation of this class
//

// flags for AppSettings::nCS
enum 
{
	CS_NONE=0, 
	CS_SEEKBAR=1, 
	CS_TOOLBAR=CS_SEEKBAR<<1, 
	CS_INFOBAR=CS_TOOLBAR<<1, 
	CS_STATSBAR=CS_INFOBAR<<1, 
	CS_STATUSBAR=CS_STATSBAR<<1, 
	CS_LAST=CS_STATUSBAR
};

enum
{
	CLSW_NONE=0,
	CLSW_OPEN=1,
	CLSW_PLAY=CLSW_OPEN<<1,
	CLSW_CLOSE=CLSW_PLAY<<1,
	CLSW_STANDBY=CLSW_CLOSE<<1,
	CLSW_HIBERNATE=CLSW_STANDBY<<1,
	CLSW_SHUTDOWN=CLSW_HIBERNATE<<1,
	CLSW_LOGOFF=CLSW_SHUTDOWN<<1,
	CLSW_AFTERPLAYBACK_MASK=CLSW_CLOSE|CLSW_STANDBY|CLSW_SHUTDOWN|CLSW_HIBERNATE|CLSW_LOGOFF,
	CLSW_FULLSCREEN=CLSW_LOGOFF<<1,
	CLSW_NEW=CLSW_FULLSCREEN<<1,
	CLSW_HELP=CLSW_NEW<<1,
	CLSW_DVD=CLSW_HELP<<1,
	CLSW_CD=CLSW_DVD<<1,
	CLSW_ADD=CLSW_CD<<1,
	CLSW_MINIMIZED=CLSW_ADD<<1,
	CLSW_REGEXTVID=CLSW_MINIMIZED<<1,		// 16
	CLSW_REGEXTAUD=CLSW_REGEXTVID<<1,
	CLSW_UNREGEXT=CLSW_REGEXTAUD<<1,
	CLSW_STARTVALID=CLSW_UNREGEXT<<2,
	CLSW_NOFOCUS=CLSW_STARTVALID<<1,
	CLSW_FIXEDSIZE=CLSW_NOFOCUS<<1,
	CLSW_MONITOR=CLSW_FIXEDSIZE<<1,
	CLSW_D3DFULLSCREEN=CLSW_MONITOR<<1,
	CLSW_ADMINOPTION=CLSW_D3DFULLSCREEN<<1,
	CLSW_SLAVE=CLSW_ADMINOPTION<<1,
	CLSW_AUDIORENDER=CLSW_SLAVE<<1,
	CLSW_UNRECOGNIZEDSWITCH=CLSW_AUDIORENDER<<1
};

enum
{
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

enum
{
	VIDRNDT_RM_DEFAULT,
	VIDRNDT_RM_DX7,
	VIDRNDT_RM_DX9,
};

enum
{
	VIDRNDT_QT_DEFAULT,
	VIDRNDT_QT_DX7,
	VIDRNDT_QT_DX9,
};

enum
{
	VIDRNDT_AP_SURFACE,
	VIDRNDT_AP_TEXTURE2D,
	VIDRNDT_AP_TEXTURE3D,
};


// Enumeration for MCE remotecontrol (careful : add 0x010000 for all keys!)
enum MCE_RAW_INPUT
{
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

enum
{
	SRC_CDDA      = 1, 
	SRC_CDXA      = SRC_CDDA<<1,
	SRC_VTS       = SRC_CDXA<<1,
	SRC_FLIC      = SRC_VTS<<1,
	SRC_D2V       = SRC_FLIC<<1,
	SRC_DTSAC3    = SRC_D2V<<1,
	SRC_MATROSKA  = SRC_DTSAC3<<1,
	SRC_SHOUTCAST = SRC_MATROSKA<<1,
	SRC_REALMEDIA = SRC_SHOUTCAST<<1,
	SRC_AVI       = SRC_REALMEDIA<<1,
	SRC_RADGT     = SRC_AVI<<1,
	SRC_ROQ       = SRC_RADGT<<1,
	SRC_OGG       = SRC_ROQ<<1,
	SRC_NUT       = SRC_OGG<<1,
	SRC_MPEG      = SRC_NUT<<1,
	SRC_DIRAC     = SRC_MPEG<<1,
	SRC_MPA       = SRC_DIRAC<<1,
	SRC_DSM       = SRC_MPA<<1,
	SRC_SUBS      = SRC_DSM<<1,
	SRC_MP4       = SRC_SUBS<<1,
	SRC_FLV       = SRC_MP4<<1,	
	SRC_FLAC      = SRC_FLV<<1,
	SRC_LAST      = SRC_FLAC<<1
};

enum
{
	TRA_MPEG1  = 1, 
	TRA_MPEG2  = TRA_MPEG1<<1,
	TRA_RV     = TRA_MPEG2<<1,
	TRA_RA     = TRA_RV<<1,
	TRA_MPA    = TRA_RA<<1,
	TRA_LPCM   = TRA_MPA<<1,
	TRA_AC3    = TRA_LPCM<<1,
	TRA_DTS    = TRA_AC3<<1,
	TRA_AAC    = TRA_DTS<<1,
	TRA_PS2AUD = TRA_AAC<<1,
	TRA_DIRAC  = TRA_PS2AUD<<1,
	TRA_VORBIS = TRA_DIRAC<<1,
	TRA_FLAC   = TRA_VORBIS<<1,
	TRA_NELLY  = TRA_FLAC<<1,
	TRA_AMR    = TRA_NELLY<<1,
	TRA_LAST   = TRA_AMR<<1
};

enum
{
	DXVA_H264  = 1,
	DXVA_VC1   = DXVA_H264<<1,
	DXVA_MPEG2 = DXVA_VC1<<1,
	DXVA_LAST  = DXVA_MPEG2<<1
};

enum
{
	FFM_H264    = 1,
	FFM_VC1     = FFM_H264<<1,	
	FFM_FLV4    = FFM_VC1<<1,
	FFM_VP62    = FFM_FLV4<<1,
	FFM_XVID    = FFM_VP62<<1,
	FFM_DIVX    = FFM_XVID<<1,
	FFM_MSMPEG4 = FFM_DIVX<<1,
	FFM_WMV     = FFM_MSMPEG4<<1,
	FFM_SVQ3    = FFM_WMV<<1,
	FFM_H263    = FFM_SVQ3<<1,
	FFM_THEORA  = FFM_H263<<1,
	FFM_AMVV    = FFM_THEORA<<1,	
	FFM_LAST    = FFM_AMVV<<1
};

enum
{
	DVS_HALF, 
	DVS_NORMAL, 
	DVS_DOUBLE, 
	DVS_STRETCH, 
	DVS_FROMINSIDE, 
	DVS_FROMOUTSIDE
};

typedef enum 
{
	FAV_FILE,
	FAV_DVD,
	FAV_DEVICE
} favtype;

#pragma pack(push, 1)
typedef struct
{
	bool fValid;
	CSize size;
	int bpp, freq;
	DWORD dmDisplayFlags;
} dispmode;
typedef struct
{
	bool bEnabled;
	dispmode dmFullscreenRes24Hz;
	dispmode dmFullscreenRes25Hz;
	dispmode dmFullscreenRes30Hz;
	dispmode dmFullscreenResOther;
	bool bApplyDefault;
	dispmode dmFullscreenRes23d976Hz;
	dispmode dmFullscreenRes29d97Hz;
}	AChFR; //AutoChangeFullscrRes

class wmcmd : public ACCEL
{
	ACCEL backup;
	UINT appcmdorg;
	UINT mouseorg;
public:
	DWORD dwname;
	UINT appcmd;
	enum {NONE,LDOWN,LUP,LDBLCLK,MDOWN,MUP,MDBLCLK,RDOWN,RUP,RDBLCLK,X1DOWN,X1UP,X1DBLCLK,X2DOWN,X2UP,X2DBLCLK,WUP,WDOWN,LAST};
	UINT mouse;
	CStringA rmcmd;
	int rmrepcnt;
	wmcmd(WORD cmd = 0) {this->cmd = cmd;}
	wmcmd(WORD cmd, WORD key, BYTE fVirt, DWORD dwname, UINT appcmd = 0, UINT mouse = NONE, LPCSTR rmcmd = "", int rmrepcnt = 5)
	{
		this->cmd = cmd;
		this->key = key;
		this->fVirt = fVirt;
		this->appcmd = appcmdorg = appcmd;
		this->dwname = dwname;
		this->mouse = mouseorg = mouse;
		this->rmcmd = rmcmd;
		this->rmrepcnt = rmrepcnt;
		backup = *this;
	}
	bool operator == (const wmcmd& wc) const
	{
		return(cmd > 0 && cmd == wc.cmd);
	}

	CString GetName()
	{
		return ResStr (dwname);
	}
	void Restore() {*(ACCEL*)this = backup; appcmd = appcmdorg; mouse = mouseorg; rmcmd.Empty(); rmrepcnt = 5;}
	bool IsModified() {return(memcmp((const ACCEL*)this, &backup, sizeof(ACCEL)) || appcmd != appcmdorg || mouse != mouseorg || !rmcmd.IsEmpty() || rmrepcnt != 5);}
};
#pragma pack(pop)

#include <afxsock.h>

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
	int GetStatus() {return(m_nStatus);}
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

extern void GetCurDispMode(dispmode& dm, CString& DisplayName);
extern bool GetDispMode(int i, dispmode& dm, CString& DisplayName);
extern void SetDispMode(dispmode& dm, CString& DisplayName);
extern void SetAudioRender(int AudioDevNo);

extern void SetHandCursor(HWND m_hWnd, UINT nID);

class CMPlayerCApp : public CWinApp
{
	ATL::CMutex m_mutexOneInstance;

	CAtlList<CString> m_cmdln;
	void PreProcessCommandLine();
	void SendCommandLine(HWND hWnd);
	UINT GetVKFromAppCommand(UINT nAppCommand);

	// === CASIMIR666 : Ajout CMPlayerCApp
	COLORPROPERTY_RANGE		m_ColorControl[4];
	HINSTANCE				m_hD3DX9Dll;
	int						m_nDXSdkRelease;

	static UINT	GetRemoteControlCodeMicrosoft(UINT nInputcode, HRAWINPUT hRawInput);
	static UINT	GetRemoteControlCodeSRM7500(UINT nInputcode, HRAWINPUT hRawInput);

public:
	CMPlayerCApp();

	void ShowCmdlnSwitches();

	bool StoreSettingsToIni();
	bool StoreSettingsToRegistry();
	CString GetIniPath();
	bool IsIniValid();

	bool GetAppSavePath(CString& path);

	// === CASIMIR666 : Ajout CMPlayerCApp
	bool		m_fTearingTest;
	int			m_fDisplayStats;
	bool		m_bResetStats; // Set to reset the presentation statistics
	CString		m_strVersion;
	CString		m_strD3DX9Version;
	CString		m_AudioRendererDisplayName_CL;

	typedef UINT (*PTR_GetRemoteControlCode)(UINT nInputcode, HRAWINPUT hRawInput);

	PTR_GetRemoteControlCode	GetRemoteControlCode;
	LONGLONG					GetPerfCounter();
	COLORPROPERTY_RANGE*		GetColorControl(ControlType nFlag);
	HINSTANCE					GetD3X9Dll();
	int							GetDXSdkRelease() { return m_nDXSdkRelease; };
	static void					SetLanguage (int nLanguage);
	static LPCTSTR				GetSatelliteDll(int nLang);
	static bool					IsVistaOrAbove();
	static bool					IsVSFilterInstalled();
	static bool					HasEVR();
	static HRESULT				GetElevationType(TOKEN_ELEVATION_TYPE* ptet);
	static void					RunAsAdministrator(LPCTSTR strCommand, LPCTSTR strArgs, bool bWaitProcess);

	void						RegisterHotkeys();
	void						UnregisterHotkeys();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMPlayerCApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	class Settings
	{
		friend class CMPlayerCApp;

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
		int nCLSwitches;
		CAtlList<CString>	slFiles, slDubs, slSubs, slFilters;
		
		// Initial position (used by command line flags)
		__int64				rtShift;
		__int64				rtStart;
		ULONG				lDVDTitle;
		ULONG				lDVDChapter;
		DVD_HMSF_TIMECODE	DVDPosition;

		CSize fixedWindowSize;
		bool HasFixedWindowSize() const {return fixedWindowSize.cx > 0 || fixedWindowSize.cy > 0;}
		// int iFixedWidth, iFixedHeight;
		int iMonitor;

		CString sPnSPreset;

		void ParseCommandLine(CAtlList<CString>& cmdln);

		bool fXpOrBetter;
		int iDXVer;
		int iAdminOption;

		int nCS;
		bool fHideCaptionMenu;
		bool fHideNavigation;
		int iDefaultVideoSize;
		bool fKeepAspectRatio;
		bool fCompMonDeskARDiff;

		CRecentFileAndURLList MRU;
		CRecentFileAndURLList MRUDub;

		CAutoPtrList<FilterOverride> filters;

		bool fResetDevice;

		class CRendererSettingsShared
		{
		public:
			CRendererSettingsShared()
			{
				SetDefault();
			}
			bool fVMR9AlterativeVSync;
			int iVMR9VSyncOffset;
			bool iVMR9VSyncAccurate;
			bool iVMR9FullscreenGUISupport;
			bool iVMR9VSync;
			bool iVMRDisableDesktopComposition;
			int iVMRFlushGPUBeforeVSync;
			int iVMRFlushGPUAfterPresent;
			int iVMRFlushGPUWait;

			// SyncRenderer settings
			int bSynchronizeVideo;
			int bSynchronizeDisplay;
			int bSynchronizeNearest;
			int iLineDelta;
			int iColumnDelta;
			double fCycleDelta;
			double fTargetSyncOffset;
			double fControlLimit;

			void SetDefault()
			{
				fVMR9AlterativeVSync = 0;
				iVMR9VSyncOffset = 0;
				iVMR9VSyncAccurate = 1;
				iVMR9FullscreenGUISupport = 0;
				iVMR9VSync = 1;
				iVMRDisableDesktopComposition = 0;
				iVMRFlushGPUBeforeVSync = 1;
				iVMRFlushGPUAfterPresent = 1;
				iVMRFlushGPUWait = 0;
				bSynchronizeVideo = 0;
				bSynchronizeDisplay = 0;
				bSynchronizeNearest = 1;
				iLineDelta = 0;
				iColumnDelta = 0;
				fCycleDelta = 0.0012;
				fTargetSyncOffset = 12.0;
				fControlLimit = 2.0;
			}
			void SetOptimal()
			{
				fVMR9AlterativeVSync = 1;
				iVMR9VSyncAccurate = 1;
				iVMR9VSync = 1;
				iVMRDisableDesktopComposition = 1;
				iVMRFlushGPUBeforeVSync = 1;
				iVMRFlushGPUAfterPresent = 1;
				iVMRFlushGPUWait = 0;
				bSynchronizeVideo = 0;
				bSynchronizeDisplay = 0;
				bSynchronizeNearest = 1;
				iLineDelta = 0;
				iColumnDelta = 0;
				fCycleDelta = 0.0012;
				fTargetSyncOffset = 12.0;
				fControlLimit = 2.0;
			}
		};
		class CRendererSettingsEVR : public CRendererSettingsShared
		{
		public:
			bool iEVRHighColorResolution;
			bool iEVREnableFrameTimeCorrection;
			int iEVROutputRange;

			CRendererSettingsEVR()
			{
				SetDefault();
			}
			void SetDefault()
			{
				CRendererSettingsShared::SetDefault();
				iEVRHighColorResolution = 0;
				iEVREnableFrameTimeCorrection = 0;
				iEVROutputRange = 0;
			}
			void SetOptimal()
			{
				CRendererSettingsShared::SetOptimal();
				iEVRHighColorResolution = 0;
			}
		};

		CRendererSettingsEVR m_RenderSettings;

		int iDSVideoRendererType;
		int iRMVideoRendererType;
		int iQTVideoRendererType;
		int iAPSurfaceUsage;
//		bool fVMRSyncFix;
		int iDX9Resizer;
		bool fVMR9MixerMode;
		bool fVMR9MixerYUV;

		int nVolume;
		int nBalance;
		bool fMute;
		int nLoops;
		bool fLoopForever;
		bool fRewind;
		int iZoomLevel;
		// int iVideoRendererType; 
		CStringW AudioRendererDisplayName;
		bool fAutoloadAudio;
		bool fAutoloadSubtitles;
		bool fBlockVSFilter;
		bool fEnableWorkerThreadForOpening;
		bool fReportFailedPins;

		CStringW f_hmonitor;
		bool fAssociatedWithIcons;
		CStringW f_lastOpenDir;

		bool fAllowMultipleInst;
		int iTitleBarTextStyle;
		bool fTitleBarTextTitle;
		int iOnTop;
		bool fTrayIcon;
		bool fRememberZoomLevel;
		bool fShowBarsWhenFullScreen;
		int nShowBarsWhenFullScreenTimeOut;
		AChFR AutoChangeFullscrRes;
		bool fExitFullScreenAtTheEnd;
		bool fRestoreResAfterExit;
		bool fRememberWindowPos;
		bool fRememberWindowSize;
		bool fSnapToDesktopEdges;
		CRect rcLastWindowPos;
		UINT lastWindowType;
		CSize AspectRatio;
		bool fKeepHistory;

		CString sDVDPath;
		bool fUseDVDPath;
		LCID idMenuLang, idAudioLang, idSubtitlesLang;
		bool fAutoSpeakerConf;

		STSStyle subdefstyle;
		bool fOverridePlacement;
		int nHorPos, nVerPos;
		int nSPCSize;
		int nSPCMaxRes;
		int nSubDelayInterval;
		bool fSPCPow2Tex;
		bool fSPCAllowAnimationWhenBuffering;
		bool fEnableSubtitles;
		bool fUseDefaultSubtitlesStyle;

		bool fDisableXPToolbars;
		bool fUseWMASFReader;
		int nJumpDistS;
		int nJumpDistM;
		int nJumpDistL;
		bool fFreeWindowResizing;
		bool fNotifyMSN;
		bool fNotifyGTSdll;

		bool fEnableAudioSwitcher;
		bool fDownSampleTo441;
		bool fAudioTimeShift;
		int tAudioTimeShift;
		bool fCustomChannelMapping;
		DWORD pSpeakerToChannelMap[18][18];
		bool fAudioNormalize;
		bool fAudioNormalizeRecover;
		float AudioBoost;

		bool fIntRealMedia;
		// bool fRealMediaRenderless;
		int iQuickTimeRenderer;
		float RealMediaQuickTimeFPS;

		CStringArray m_pnspresets;

		CList<wmcmd> wmcmds;
		HACCEL hAccel;

		bool fWinLirc;
		CString WinLircAddr;
		CWinLircClient WinLircClient;
		bool fUIce;
		CString UIceAddr;
		CUIceClient UIceClient;
		bool fGlobalMedia;

		CMediaFormats Formats;

		UINT SrcFilters, TraFilters, DXVAFilters, FFmpegFilters;

		CString logofn;
		UINT logoid;
		bool logoext;

		bool fHideCDROMsSubMenu;

		DWORD priority;
		bool launchfullscreen;

		BOOL fEnableWebServer;
		int nWebServerPort;
		bool fWebServerPrintDebugInfo;
		bool fWebServerUseCompression;
		bool fWebServerLocalhostOnly;
		CString WebRoot, WebDefIndex;
		CString WebServerCGI;

		CString SnapShotPath, SnapShotExt;
		int ThumbRows, ThumbCols, ThumbWidth;

		CString ISDb;

		struct Shader 
		{
			CString label;
			CString target;
			CString srcdata;
		};
		CAtlList<Shader> m_shaders;
		CString m_shadercombine;
		CString m_shadercombineScreenSpace;

		// === CASIMIR666 : nouveau settings
		bool			fD3DFullscreen;
		bool			fMonitorAutoRefreshRate;
		bool			fLastFullScreen;
		bool			fEnableEDLEditor;
		float			dBrightness;
		float			dContrast;
		float			dHue;
		float			dSaturation;
		CString			strShaderList;
		CString			strShaderListScreenSpace;
		bool			m_bToggleShader;
		bool			m_bToggleShaderScreenSpace;

		bool			fRememberDVDPos;
		bool			fRememberFilePos;
		bool			fShowOSD;
		int				iEvrBuffers;
		int				iLanguage;

		// BDA configuration
		int				iDefaultCaptureDevice;		// Default capture device (analog=0, 1=digital)
		CString			strAnalogVideo;
		CString			strAnalogAudio;
		int				iAnalogCountry;
		CString			BDANetworkProvider;
		CString			BDATuner;
		CString			BDAReceiver;
		int				DVBLastChannel;
		CAtlList<CDVBChannel>	DVBChannels;


		HWND			hMasterWnd;

		bool			IsD3DFullscreen();
		CString			SelectedAudioRender();
		void			ResetPositions();
		DVD_POSITION*	CurrentDVDPosition();
		bool			NewDvd(ULONGLONG llDVDGuid);
		FILE_POSITION*	CurrentFilePosition();
		bool			NewFile(LPCTSTR strFileName);

		void			DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize);
		CString			SerializeHex (BYTE* pBuffer, int nBufSize);

	private :
		DVD_POSITION	DvdPosition[MAX_DVD_POSITION];
		int				nCurrentDvdPosition;
		FILE_POSITION	FilePosition[MAX_FILE_POSITION];
		int				nCurrentFilePosition;

		__int64			ConvertTimeToMSec(CString& time);
		void			ExtractDVDStartPos(CString& strParam);

		void			CreateCommands();
	public:
		Settings();
		virtual ~Settings();
		void UpdateData(bool fSave);

		void GetFav(favtype ft, CAtlList<CString>& sl);
		void SetFav(favtype ft, CAtlList<CString>& sl);
		void AddFav(favtype ft, CString s);
		CDVBChannel* FindChannelByPref(int nPrefNumber);

		bool m_fPreventMinimize;
		bool m_fUseWin7TaskBar;
		bool m_fExitAfterPlayback;
		bool m_fNextInDirAfterPlayback;
		CStringW m_subtitlesLanguageOrder;
		CStringW m_audiosLanguageOrder;

		int fnChannels;

		CString D3D9RenderDevice;
	} m_s;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileExit();
	afx_msg void OnHelpShowcommandlineswitches();
};

#define AfxGetMyApp() static_cast<CMPlayerCApp*>(AfxGetApp())
#define AfxGetAppSettings() static_cast<CMPlayerCApp*>(AfxGetApp())->m_s
#define AppSettings CMPlayerCApp::Settings
