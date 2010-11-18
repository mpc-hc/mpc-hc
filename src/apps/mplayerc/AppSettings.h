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

#include "resource.h"		// main symbols
#include <afxadv.h>
#include <atlsync.h>
#include "RenderersSettings.h"
#include "internal_filter_config.h"

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
	CLSW_AUDIORENDERER=CLSW_SLAVE<<1,
	CLSW_UNRECOGNIZEDSWITCH=CLSW_AUDIORENDERER<<1
};

enum
{
	MODE_SHOWCAPTIONMENU,
	MODE_BORDERLESS,
	MODE_FRAMEONLY,
	MODE_COUNT
}; // flags for Caption & Menu Mode

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
  SOURCE_FILTER,
  DECODER,
  DXVA_DECODER,
  FFMPEG_DECODER
};

enum SOURCE_FILTER
{
#if INTERNAL_SOURCEFILTER_CDDA
	SRC_CDDA,
#endif
#if INTERNAL_SOURCEFILTER_CDXA
	SRC_CDXA,
#endif
#if INTERNAL_SOURCEFILTER_VTS
	SRC_VTS,
#endif
#if INTERNAL_SOURCEFILTER_FLIC
	SRC_FLIC,
#endif
#if INTERNAL_SOURCEFILTER_DVSOURCE
	SRC_D2V,
#endif
#if INTERNAL_SOURCEFILTER_DTSAC3
	SRC_DTSAC3,
#endif
#if INTERNAL_SOURCEFILTER_MATROSKA
	SRC_MATROSKA,
#endif
#if INTERNAL_SOURCEFILTER_SHOUTCAST
	SRC_SHOUTCAST,
#endif
#if INTERNAL_SOURCEFILTER_REALMEDIA
	SRC_REALMEDIA,
#endif
#if INTERNAL_SOURCEFILTER_AVI
	SRC_AVI,
#endif
#if INTERNAL_SOURCEFILTER_ROQ
	SRC_ROQ,
#endif
#if INTERNAL_SOURCEFILTER_OGG
	SRC_OGG,
#endif
#if INTERNAL_SOURCEFILTER_NUT
	SRC_NUT,
#endif
#if INTERNAL_SOURCEFILTER_MPEG
	SRC_MPEG,
#endif
#if INTERNAL_SOURCEFILTER_DIRAC
	SRC_DIRAC,
#endif
#if INTERNAL_SOURCEFILTER_MPEGAUDIO
	SRC_MPA,
#endif
#if INTERNAL_SOURCEFILTER_DSM
	SRC_DSM,
#endif
	SRC_SUBS,
#if INTERNAL_SOURCEFILTER_MP4
	SRC_MP4,
#endif
#if INTERNAL_SOURCEFILTER_FLV
	SRC_FLV,
#endif
#if INTERNAL_SOURCEFILTER_FLAC
	SRC_FLAC,
#endif
	SRC_LAST
};

enum DECODER
{
#if INTERNAL_DECODER_MPEG1
	TRA_MPEG1,
#endif
#if INTERNAL_DECODER_MPEG2
	TRA_MPEG2,
#endif
#if INTERNAL_DECODER_REALVIDEO
	TRA_RV,
#endif
#if INTERNAL_DECODER_REALAUDIO
	TRA_RA,
#endif
#if INTERNAL_DECODER_MPEGAUDIO
	TRA_MPA,
#endif
#if INTERNAL_DECODER_DTS
	TRA_DTS,
	TRA_LPCM,
#endif
#if INTERNAL_DECODER_AC3
	TRA_AC3,
#endif
#if INTERNAL_DECODER_AAC
	TRA_AAC,
#endif
#if INTERNAL_DECODER_PS2AUDIO
	TRA_PS2AUD,
#endif
#if INTERNAL_DECODER_DIRAC
	TRA_DIRAC,
#endif
#if INTERNAL_DECODER_VORBIS
	TRA_VORBIS,
#endif
#if INTERNAL_DECODER_FLAC
	TRA_FLAC,
#endif
#if INTERNAL_DECODER_NELLYMOSER
	TRA_NELLY,
#endif
#if INTERNAL_DECODER_AMR
	TRA_AMR,
#endif
#if INTERNAL_DECODER_PCM
	TRA_PCM,
#endif
	TRA_LAST
};

enum DXVA_DECODER
{
#if INTERNAL_DECODER_H264_DXVA
	DXVA_H264,
#endif
#if INTERNAL_DECODER_VC1_DXVA
	DXVA_VC1,
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
	DXVA_MPEG2,
#endif
	DXVA_LAST
};

enum FFMPEG_DECODER
{
#if INTERNAL_DECODER_H264
	FFM_H264,
#endif
#if INTERNAL_DECODER_VC1
	FFM_VC1,
#endif
#if INTERNAL_DECODER_FLV
	FFM_FLV4,
#endif
#if INTERNAL_DECODER_VP6
	FFM_VP62,
#endif
#if INTERNAL_DECODER_VP8
	FFM_VP8,
#endif
#if INTERNAL_DECODER_XVID
	FFM_XVID,
#endif
#if INTERNAL_DECODER_DIVX
	FFM_DIVX,
#endif
#if INTERNAL_DECODER_MSMPEG4
	FFM_MSMPEG4,
#endif
#if INTERNAL_DECODER_WMV
	FFM_WMV,
#endif
#if INTERNAL_DECODER_SVQ
	FFM_SVQ3,
#endif
#if INTERNAL_DECODER_H263
	FFM_H263,
#endif
#if INTERNAL_DECODER_THEORA
	FFM_THEORA,
#endif
#if INTERNAL_DECODER_AMVV
	FFM_AMVV,
#endif
	FFM_LAST
};

typedef enum
{
	DVS_HALF,
	DVS_NORMAL,
	DVS_DOUBLE,
	DVS_STRETCH,
	DVS_FROMINSIDE,
	DVS_FROMOUTSIDE,
	DVS_ZOOM1,
	DVS_ZOOM2
} dvstype;

typedef enum
{
	FAV_FILE,
	FAV_DVD,
	FAV_DEVICE
} favtype;

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
#pragma pack(pop)

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
	wmcmd(WORD cmd = 0)
	{
		this->cmd = cmd;
	}
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
	void Restore()
	{
		*(ACCEL*)this = backup;
		appcmd = appcmdorg;
		mouse = mouseorg;
		rmcmd.Empty();
		rmrepcnt = 5;
	}
	bool IsModified()
	{
		return(memcmp((const ACCEL*)this, &backup, sizeof(ACCEL)) || appcmd != appcmdorg || mouse != mouseorg || !rmcmd.IsEmpty() || rmrepcnt != 5);
	}
};

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
	int GetStatus()
	{
		return(m_nStatus);
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
	int nCLSwitches;
	CAtlList<CString>	slFiles, slDubs, slSubs, slFilters;

	// Initial position (used by command line flags)
	__int64				rtShift;
	__int64				rtStart;
	ULONG				lDVDTitle;
	ULONG				lDVDChapter;
	DVD_HMSF_TIMECODE	DVDPosition;

	CSize fixedWindowSize;
	bool HasFixedWindowSize() const
	{
		return fixedWindowSize.cx > 0 || fixedWindowSize.cy > 0;
	}
	//int			iFixedWidth, iFixedHeight;
	int				iMonitor;

	CString			sPnSPreset;

	void			ParseCommandLine(CAtlList<CString>& cmdln);

	// Added a Debug display to the screen (/debug option)
	bool			ShowDebugInfo;

	int				iDXVer;
	int				iAdminOption;

	int				nCS;
	int				fCaptionMenuMode;
	bool			fHideNavigation;
	int				iDefaultVideoSize;
	bool			fKeepAspectRatio;
	bool			fCompMonDeskARDiff;

	CRecentFileAndURLList MRU;
	CRecentFileAndURLList MRUDub;

	CAutoPtrList<FilterOverride> filters;

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
	CStringW		AudioRendererDisplayName;
	bool			fAutoloadAudio;
	bool			fAutoloadSubtitles;
	bool			fBlockVSFilter;
	bool			fEnableWorkerThreadForOpening;
	bool			fReportFailedPins;

	CStringW		f_hmonitor;
	bool			fAssociatedWithIcons;
	CStringW		f_lastOpenDir;

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
	bool			fSnapToDesktopEdges;
	CRect			rcLastWindowPos;
	UINT			lastWindowType;
	CSize			AspectRatio;
	bool			fKeepHistory;
	UINT			iLastUsedPage;

	CString			sDVDPath;
	bool			fUseDVDPath;
	LCID			idMenuLang, idAudioLang, idSubtitlesLang;
	bool			fAutoSpeakerConf;

	STSStyle		subdefstyle;
	bool			fOverridePlacement;
	int				nHorPos, nVerPos;
	int				nSubDelayInterval;
	bool			fEnableSubtitles;
	bool			fUseDefaultSubtitlesStyle;
	bool			fPrioritizeExternalSubtitles;
	bool			fDisableInternalSubtitles;
	CString			szSubtitlePaths;

	bool			fDisableXPToolbars;
	bool			fUseWMASFReader;
	int				nJumpDistS;
	int				nJumpDistM;
	int				nJumpDistL;
	bool			fLimitWindowProportions;
	bool			fNotifyMSN;
	bool			fNotifyGTSdll;

	bool			fEnableAudioSwitcher;
	bool			fDownSampleTo441;
	bool			fAudioTimeShift;
	int				tAudioTimeShift;
	bool			fCustomChannelMapping;
	DWORD			pSpeakerToChannelMap[18][18];
	bool			fAudioNormalize;
	bool			fAudioNormalizeRecover;
	float			AudioBoost;

	bool			fIntRealMedia;
	//bool			fRealMediaRenderless;
	int				iQuickTimeRenderer;
	float			RealMediaQuickTimeFPS;

	CStringArray	m_pnspresets;

	CList<wmcmd>	wmcmds;
	HACCEL			hAccel;

	bool			fWinLirc;
	CString			WinLircAddr;
	CWinLircClient	WinLircClient;
	bool			fUIce;
	CString			UIceAddr;
	CUIceClient		UIceClient;
	bool			fGlobalMedia;

	CMediaFormats	Formats;

	bool			SrcFilters[SRC_LAST];
	bool			TraFilters[TRA_LAST];
	bool			DXVAFilters[DXVA_LAST];
	bool			FFmpegFilters[FFM_LAST];

	CString			logofn;
	UINT			logoid;
	bool			logoext;

	bool			fHideCDROMsSubMenu;

	DWORD			priority;
	bool			launchfullscreen;

	BOOL			fEnableWebServer;
	int				nWebServerPort;
	int				nCmdlnWebServerPort;
	bool			fWebServerPrintDebugInfo;
	bool			fWebServerUseCompression;
	bool			fWebServerLocalhostOnly;
	CString			WebRoot, WebDefIndex;
	CString			WebServerCGI;

	CString			SnapShotPath, SnapShotExt;
	int				ThumbRows, ThumbCols, ThumbWidth;

	CString			ISDb;

	struct Shader
	{
		CString		label;
		CString		target;
		CString		srcdata;
	};
	CAtlList<Shader> m_shaders;
	CString			m_shadercombine;
	CString			m_shadercombineScreenSpace;

	// Casimir666 : new settings
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
	int				iLanguage;

	// BDA configuration
	int				iDefaultCaptureDevice;		// Default capture device (analog=0, 1=digital)
	CString			strAnalogVideo;
	CString			strAnalogAudio;
	int				iAnalogCountry;
	CString			BDANetworkProvider;
	CString			BDATuner;
	CString			BDAReceiver;
	CString			BDAStandard;
	int				BDAScanFreqStart;
	int				BDAScanFreqEnd;
	int				BDABandwidth;
	bool			BDAUseOffset;
	int				BDAOffset;
	bool			BDAIgnoreEncryptedChannels;
	int				DVBLastChannel;
	CAtlList<CDVBChannel>	DVBChannels;

	HWND			hMasterWnd;

	bool			IsD3DFullscreen();
	CString			SelectedAudioRenderer();
	void			ResetPositions();
	DVD_POSITION*	CurrentDVDPosition();
	bool			NewDvd(ULONGLONG llDVDGuid);
	FILE_POSITION*	CurrentFilePosition();
	bool			NewFile(LPCTSTR strFileName);

	void			SaveCurrentDVDPosition();
	void			SaveCurrentFilePosition();

	void			DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize);
	CString			SerializeHex (BYTE* pBuffer, int nBufSize);

private :
	DVD_POSITION	DvdPosition[MAX_DVD_POSITION];
	int				nCurrentDvdPosition;
	FILE_POSITION	FilePosition[MAX_FILE_POSITION];
	int				nCurrentFilePosition;

	CString		SrcFiltersKeys[SRC_LAST];
	CString		TraFiltersKeys[TRA_LAST];
	CString		DXVAFiltersKeys[DXVA_LAST];
	CString		FFMFiltersKeys[FFM_LAST];

	__int64			ConvertTimeToMSec(CString& time);
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

	bool			m_fPreventMinimize;
	bool			m_fUseWin7TaskBar;
	bool			m_fExitAfterPlayback;
	bool			m_fNextInDirAfterPlayback;
	bool			m_fDontUseSearchInFolder;
	int				nOSD_Size;
	CString			m_OSD_Font;
	CStringW		m_subtitlesLanguageOrder;
	CStringW		m_audiosLanguageOrder;

	int				fnChannels;
private:
	void			UpdateRenderersData(bool fSave);
	friend	void	CRenderersSettings::UpdateData(bool bSave);
};
