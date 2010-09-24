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
#include "../../Subtitles/STS.h"
#include "MediaFormats.h"
#include "FakeFilterMapper2.h"
#include "DVBChannel.h"
#include "RenderersSettings.h"

#include "AppSettings.h"
#define MPC_WND_CLASS_NAME L"MediaPlayerClassicW"

//define the default logo we use
#define DEF_LOGO IDF_LOGO3

enum
{
	WM_GRAPHNOTIFY = WM_RESET_DEVICE+1,
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
extern "C" BOOL	IsVistaOrAbove();

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

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp:
// See mplayerc.cpp for the implementation of this class
//


extern void GetCurDispMode(dispmode& dm, CString& DisplayName);
extern bool GetDispMode(int i, dispmode& dm, CString& DisplayName);
extern void SetDispMode(dispmode& dm, CString& DisplayName);
extern void SetAudioRenderer(int AudioDevNo);

extern void SetHandCursor(HWND m_hWnd, UINT nID);

class CMPlayerCApp : public CWinApp
{
	ATL::CMutex m_mutexOneInstance;

	CAtlList<CString> m_cmdln;
	void PreProcessCommandLine();
	void SendCommandLine(HWND hWnd);
	UINT GetVKFromAppCommand(UINT nAppCommand);

	// Casimir666 : new in CMPlayerCApp
	COLORPROPERTY_RANGE		m_ColorControl[4];

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

	// Casimir666 : new in CMPlayerCApp
	CRenderersData m_Renderers;
	CString		m_strVersion;
	CString		m_AudioRendererDisplayName_CL;

	CAppSettings m_s;

	typedef UINT (*PTR_GetRemoteControlCode)(UINT nInputcode, HRAWINPUT hRawInput);

	PTR_GetRemoteControlCode	GetRemoteControlCode;
	COLORPROPERTY_RANGE*		GetColorControl(ControlType nFlag);
	static void					SetLanguage (int nLanguage);
	static LPCTSTR				GetSatelliteDll(int nLang);
	static int					GetDefLanguage();
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

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileExit();
	afx_msg void OnHelpShowcommandlineswitches();
};

#define AfxGetMyApp() static_cast<CMPlayerCApp*>(AfxGetApp())
#define AfxGetAppSettings() static_cast<CMPlayerCApp*>(AfxGetApp())->m_s
#define AppSettings CAppSettings
