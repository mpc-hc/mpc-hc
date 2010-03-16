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

#include "stdafx.h"
#include "mplayerc.h"
#include <atlsync.h>
#include <Tlhelp32.h>
#include "MainFrm.h"
#include "../../DSUtil/DSUtil.h"
#include "Struct.h"
#include "FileVersionInfo.h"
#include <psapi.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "Ifo.h"
#include "MiniDump.h"
#include "SettingsDefines.h"

#include "Monitors.h"

#pragma comment(lib, "winmm.lib")



void CorrectComboListWidth(CComboBox& box, CFont* pWndFont)
{
	int cnt = box.GetCount();
	if(cnt <= 0) return;

	CDC* pDC = box.GetDC();
	pDC->SelectObject(pWndFont);

	int maxw = box.GetDroppedWidth();

	for(int i = 0; i < cnt; i++)
	{
		CString str;
		box.GetLBText(i, str);
		int w = pDC->GetTextExtent(str).cx + 22;
		if(maxw < w) maxw = w;
	}

	box.ReleaseDC(pDC);

	box.SetDroppedWidth(maxw);
}

HICON LoadIcon(CString fn, bool fSmall)
{
	if(fn.IsEmpty()) return(NULL);

	CString ext = fn.Left(fn.Find(_T("://"))+1).TrimRight(':');
	if(ext.IsEmpty() || !ext.CompareNoCase(_T("file")))
		ext = _T(".") + fn.Mid(fn.ReverseFind('.')+1);

	CSize size(fSmall?16:32,fSmall?16:32);

	if(!ext.CompareNoCase(_T(".ifo")))
	{
		if(HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DVD), IMAGE_ICON, size.cx, size.cy, 0))
			return(hIcon);
	}

	if(!ext.CompareNoCase(_T(".cda")))
	{
		if(HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_AUDIOCD), IMAGE_ICON, size.cx, size.cy, 0))
			return(hIcon);
	}

	do
	{
		CRegKey key;

		TCHAR buff[256];
		ULONG len;

		if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext + _T("\\DefaultIcon"), KEY_READ))
		{
			if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext, KEY_READ))
				break;

			len = sizeof(buff);
			memset(buff, 0, len);
			if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) || (ext = buff).Trim().IsEmpty())
				break;

			if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext + _T("\\DefaultIcon"), KEY_READ))
				break;
		}

		CString icon;

		len = sizeof(buff);
		memset(buff, 0, len);
		if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) || (icon = buff).Trim().IsEmpty())
			break;

		int i = icon.ReverseFind(',');
		if(i < 0) break;
		
		int id = 0;
		if(_stscanf_s(icon.Mid(i+1), _T("%d"), &id) != 1)
			break;

		icon = icon.Left(i);

		HICON hIcon = NULL;
		UINT cnt = fSmall 
			? ExtractIconEx(icon, id, NULL, &hIcon, 1)
			: ExtractIconEx(icon, id, &hIcon, NULL, 1);
		if(hIcon) return hIcon;
	}
	while(0);

	return((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_UNKNOWN), IMAGE_ICON, size.cx, size.cy, 0));
}

bool LoadType(CString fn, CString& type)
{
	CRegKey key;

	TCHAR buff[256];
	ULONG len;

	if(fn.IsEmpty()) return(NULL);

	CString ext = fn.Left(fn.Find(_T("://"))+1).TrimRight(':');
	if(ext.IsEmpty() || !ext.CompareNoCase(_T("file")))
		ext = _T(".") + fn.Mid(fn.ReverseFind('.')+1);

	if(ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext))
		return(false);

	CString tmp = ext;

    while(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, tmp))
	{
		len = sizeof(buff);
		memset(buff, 0, len);
		if(ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len))
			break;

		CString str(buff);
		str.Trim();

		if(str.IsEmpty() || str == tmp)
			break;

		tmp = str;
	}

	type = tmp;

	return(true);
}

bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype)
{
	str.Empty();
	HRSRC hrsrc = FindResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(resid), restype);
	if(!hrsrc) return(false);
	HGLOBAL hGlobal = LoadResource(AfxGetApp()->m_hInstance, hrsrc);
	if(!hGlobal) return(false);
	DWORD size = SizeofResource(AfxGetApp()->m_hInstance, hrsrc);
	if(!size) return(false);
	memcpy(str.GetBufferSetLength(size), LockResource(hGlobal), size);
	return(true);
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

extern "C" char *GetFfmpegCompiler();

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CString m_appname;
	virtual BOOL OnInitDialog()
	{
		USES_CONVERSION;
		UpdateData();
		m_strBuildNumber = AfxGetMyApp()->m_strVersion;

		#if (_MSC_VER == 1500)
			#if (_MSC_FULL_VER >= 150030729)
				m_MPCCompiler = _T("MSVC 2008 SP1");
			#else
				m_MPCCompiler = _T("MSVC 2008");
			#endif
		#elif (_MSC_VER < 1500)
			#error Compiler is not supported!
		#endif

		m_FfmpegCompiler.Format (A2W(GetFfmpegCompiler()));

		UpdateData(FALSE);
		return TRUE;
	}
	CString m_strBuildNumber;
	CString m_MPCCompiler;
	CString m_FfmpegCompiler;
	afx_msg void OnHomepage(NMHDR *pNMHDR, LRESULT *pResult);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD), m_appname(_T(""))
, m_strBuildNumber(_T(""))
, m_MPCCompiler(_T(""))
, m_FfmpegCompiler(_T(""))
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_STATIC1, m_appname);
	DDX_Text(pDX, IDC_BUILD_NUMBER, m_strBuildNumber);
	DDX_Text(pDX, IDC_MPC_COMPILER, m_MPCCompiler);
	DDX_Text(pDX, IDC_FFMPEG_COMPILER, m_FfmpegCompiler);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CLICK, IDC_SOURCEFORGE_LINK, &CAboutDlg::OnHomepage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp

BEGIN_MESSAGE_MAP(CMPlayerCApp, CWinApp)
	//{{AFX_MSG_MAP(CMPlayerCApp)
	ON_COMMAND(ID_HELP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_SHOWCOMMANDLINESWITCHES, OnHelpShowcommandlineswitches)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp construction

CMPlayerCApp::CMPlayerCApp()
//	: m_hMutexOneInstance(NULL)
{
	CFileVersionInfo	Version;
	TCHAR				strApp [_MAX_PATH];

	GetModuleFileNameEx (GetCurrentProcess(), AfxGetMyApp()->m_hInstance, strApp, MAX_PATH);
	Version.Create (strApp);
	m_strVersion = Version.GetFileVersionEx();

	m_fTearingTest  = false;
	m_fDisplayStats = false;
	m_bResetStats	= false;
	m_hD3DX9Dll		= NULL;
	m_nDXSdkRelease = 0;
	memset (&m_ColorControl, 0, sizeof(m_ColorControl));
	m_ColorControl[0].dwSize		= sizeof (COLORPROPERTY_RANGE);
	m_ColorControl[0].dwProperty	= Brightness;
	m_ColorControl[1].dwSize		= sizeof (COLORPROPERTY_RANGE);
	m_ColorControl[1].dwProperty	= Contrast;
	m_ColorControl[2].dwSize		= sizeof (COLORPROPERTY_RANGE);
	m_ColorControl[2].dwProperty	= Hue;
	m_ColorControl[3].dwSize		= sizeof (COLORPROPERTY_RANGE);
	m_ColorControl[3].dwProperty	= Saturation;

	GetRemoteControlCode = GetRemoteControlCodeMicrosoft;
}

void CMPlayerCApp::ShowCmdlnSwitches()
{
	CString s;

	if(m_s.nCLSwitches&CLSW_UNRECOGNIZEDSWITCH)
	{
		CAtlList<CString> sl;
		for(int i = 0; i < __argc; i++) sl.AddTail(__targv[i]);
		s += ResStr(IDS_UNKNOWN_SWITCH) + Implode(sl, ' ') + _T("\n\n");
	}

	s += ResStr (IDS_USAGE);

	AfxMessageBox(s);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMPlayerCApp object

CMPlayerCApp theApp;

HWND g_hWnd = NULL;

bool CMPlayerCApp::StoreSettingsToIni()
{
	CString ini = GetIniPath();
/*
	FILE* f;
	if(!(f = _tfopen(ini, _T("r+"))) && !(f = _tfopen(ini, _T("w"))))
		return StoreSettingsToRegistry();
	fclose(f);
*/
	if(m_pszRegistryKey) free((void*)m_pszRegistryKey);
	m_pszRegistryKey = NULL;
	if(m_pszProfileName) free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(ini);

	return(true);
}

bool CMPlayerCApp::StoreSettingsToRegistry()
{
	_tremove(GetIniPath());

	if(m_pszRegistryKey) free((void*)m_pszRegistryKey);
	m_pszRegistryKey = NULL;

	SetRegistryKey(_T("Gabest"));

	return(true);
}

CString CMPlayerCApp::GetIniPath()
{
	CString path;
	GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	path = path.Left(path.ReverseFind('.')+1) + _T("ini");
	return(path);
}

bool CMPlayerCApp::IsIniValid()
{
	CFileStatus fs;
	return CFileGetStatus(GetIniPath(), fs) && fs.m_size > 0;
}

bool CMPlayerCApp::GetAppSavePath(CString& path)
{
	path.Empty();

	if(IsIniValid()) // If settings ini file found, store stuff in the same folder as the exe file
	{
		GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(MAX_PATH), MAX_PATH);
		path.ReleaseBuffer();
		path = path.Left(path.ReverseFind('\\'));
	}
	else
	{
		CRegKey key;
		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ))
		{
			ULONG len = MAX_PATH;
			if(ERROR_SUCCESS == key.QueryStringValue(_T("AppData"), path.GetBuffer(MAX_PATH), &len))
				path.ReleaseBufferSetLength(len);
		}

		if(path.IsEmpty())
			return(false);

		CPath p;
		p.Combine(path, _T("Media Player Classic"));
		path = (LPCTSTR)p;
	}

	return(true);
}

void CMPlayerCApp::PreProcessCommandLine()
{
	m_cmdln.RemoveAll();
	for(int i = 1; i < __argc; i++)
	{
		CString str = CString(__targv[i]).Trim(_T(" \""));

		if(str[0] != '/' && str[0] != '-' && str.Find(_T(":")) < 0)
		{
			LPTSTR p = NULL;
			CString str2;
			str2.ReleaseBuffer(GetFullPathName(str, MAX_PATH, str2.GetBuffer(MAX_PATH), &p));
			CFileStatus fs;
			if(!str2.IsEmpty() && CFileGetStatus(str2, fs)) str = str2;
		}

		m_cmdln.AddTail(str);
	}
}

void CMPlayerCApp::SendCommandLine(HWND hWnd)
{
	if(m_cmdln.IsEmpty())
		return;

	int bufflen = sizeof(DWORD);

	POSITION pos = m_cmdln.GetHeadPosition();
	while(pos) bufflen += (m_cmdln.GetNext(pos).GetLength()+1)*sizeof(TCHAR);

	CAutoVectorPtr<BYTE> buff;
	if(!buff.Allocate(bufflen))
		return;

	BYTE* p = buff;

	*(DWORD*)p = m_cmdln.GetCount(); 
	p += sizeof(DWORD);

	pos = m_cmdln.GetHeadPosition();
	while(pos)
	{
		CString s = m_cmdln.GetNext(pos); 
		int len = (s.GetLength()+1)*sizeof(TCHAR);
		memcpy(p, s, len);
		p += len;
	}

	COPYDATASTRUCT cds;
	cds.dwData = 0x6ABE51;
	cds.cbData = bufflen;
	cds.lpData = (void*)(BYTE*)buff;
	SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
}

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp initialization

#include <winddk/ntddcdvd.h>

#include <detours/detours.h>

BOOL (__stdcall * Real_IsDebuggerPresent)(void)
    = IsDebuggerPresent;

LONG (__stdcall * Real_ChangeDisplaySettingsExA)(LPCSTR a0,
                                                 LPDEVMODEA a1,
                                                 HWND a2,
                                                 DWORD a3,
                                                 LPVOID a4)
    = ChangeDisplaySettingsExA;

LONG (__stdcall * Real_ChangeDisplaySettingsExW)(LPCWSTR a0,
                                                 LPDEVMODEW a1,
                                                 HWND a2,
                                                 DWORD a3,
                                                 LPVOID a4)
    = ChangeDisplaySettingsExW;

HANDLE (__stdcall * Real_CreateFileA)(LPCSTR a0,
                                      DWORD a1,
                                      DWORD a2,
                                      LPSECURITY_ATTRIBUTES a3,
                                      DWORD a4,
                                      DWORD a5,
                                      HANDLE a6)
    = CreateFileA;

HANDLE (__stdcall * Real_CreateFileW)(LPCWSTR a0,
                                      DWORD a1,
                                      DWORD a2,
                                      LPSECURITY_ATTRIBUTES a3,
                                      DWORD a4,
                                      DWORD a5,
                                      HANDLE a6)
    = CreateFileW;

BOOL (__stdcall * Real_DeviceIoControl)(HANDLE a0,
                                        DWORD a1,
                                        LPVOID a2,
                                        DWORD a3,
                                        LPVOID a4,
                                        DWORD a5,
                                        LPDWORD a6,
                                        LPOVERLAPPED a7)
    = DeviceIoControl;

MMRESULT  (__stdcall * Real_mixerSetControlDetails)( HMIXEROBJ hmxobj, 
																	LPMIXERCONTROLDETAILS pmxcd, 
																	DWORD fdwDetails)
	= mixerSetControlDetails;


typedef NTSTATUS (WINAPI *FUNC_NTQUERYINFORMATIONPROCESS)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
static FUNC_NTQUERYINFORMATIONPROCESS		Real_NtQueryInformationProcess = NULL;
/*
NTSTATUS (* Real_NtQueryInformationProcess) (HANDLE				ProcessHandle, 
											 PROCESSINFOCLASS	ProcessInformationClass, 
											 PVOID				ProcessInformation, 
											 ULONG				ProcessInformationLength, 
											 PULONG				ReturnLength)
	= NULL;*/


BOOL WINAPI Mine_IsDebuggerPresent()
{
	TRACE(_T("Oops, somebody was trying to be naughty! (called IsDebuggerPresent)\n")); 
	return FALSE;
}


NTSTATUS WINAPI Mine_NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
	NTSTATUS		nRet;

	nRet = Real_NtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

	if (ProcessInformationClass == ProcessBasicInformation)
	{
		PROCESS_BASIC_INFORMATION*		pbi = (PROCESS_BASIC_INFORMATION*)ProcessInformation;
		PEB_NT*							pPEB;
		PEB_NT							PEB;
		
		pPEB = (PEB_NT*)pbi->PebBaseAddress;
		ReadProcessMemory(ProcessHandle, pPEB, &PEB, sizeof(PEB), NULL);
		PEB.BeingDebugged = 0;
		WriteProcessMemory(ProcessHandle, pPEB, &PEB, sizeof(PEB), NULL);
	}
	else if (ProcessInformationClass == 7) // ProcessDebugPort
	{
		BOOL*		pDebugPort = (BOOL*)ProcessInformation;
		*pDebugPort = FALSE;
	}

	return nRet;
}

LONG WINAPI Mine_ChangeDisplaySettingsEx(LONG ret, DWORD dwFlags, LPVOID lParam)
{
	if(dwFlags&CDS_VIDEOPARAMETERS)
	{
		VIDEOPARAMETERS* vp = (VIDEOPARAMETERS*)lParam;

		if(vp->Guid == GUIDFromCString(_T("{02C62061-1097-11d1-920F-00A024DF156E}"))
		&& (vp->dwFlags&VP_FLAGS_COPYPROTECT))
		{
			if(vp->dwCommand == VP_COMMAND_GET)
			{
				if((vp->dwTVStandard&VP_TV_STANDARD_WIN_VGA)
				&& vp->dwTVStandard != VP_TV_STANDARD_WIN_VGA)
				{
					TRACE(_T("Ooops, tv-out enabled? macrovision checks suck..."));
					vp->dwTVStandard = VP_TV_STANDARD_WIN_VGA;
				}
			}
			else if(vp->dwCommand == VP_COMMAND_SET)
			{
				TRACE(_T("Ooops, as I already told ya, no need for any macrovision bs here"));
				return 0;
			}
		}
	}

	return ret;
}
LONG WINAPI Mine_ChangeDisplaySettingsExA(LPCSTR lpszDeviceName, LPDEVMODEA lpDevMode, HWND hwnd, DWORD dwFlags, LPVOID lParam)
{
	return Mine_ChangeDisplaySettingsEx(
		Real_ChangeDisplaySettingsExA(lpszDeviceName, lpDevMode, hwnd, dwFlags, lParam), 
		dwFlags, 
		lParam);
}
LONG WINAPI Mine_ChangeDisplaySettingsExW(LPCWSTR lpszDeviceName, LPDEVMODEW lpDevMode, HWND hwnd, DWORD dwFlags, LPVOID lParam)
{
	return Mine_ChangeDisplaySettingsEx(
		Real_ChangeDisplaySettingsExW(lpszDeviceName, lpDevMode, hwnd, dwFlags, lParam), 
		dwFlags,
		lParam);
}

HANDLE WINAPI Mine_CreateFileA(LPCSTR p1, DWORD p2, DWORD p3, LPSECURITY_ATTRIBUTES p4, DWORD p5, DWORD p6, HANDLE p7)
{
	//CStringA fn(p1);
	//fn.MakeLower();
	//int i = fn.Find(".part");
	//if(i > 0 && i == fn.GetLength() - 5)
		p3 |= FILE_SHARE_WRITE;

	return Real_CreateFileA(p1, p2, p3, p4, p5, p6, p7);
}

BOOL CreateFakeVideoTS(LPCWSTR strIFOPath, LPWSTR strFakeFile, size_t nFakeFileSize)
{
	BOOL		bRet = FALSE;
    WCHAR		szTempPath[_MAX_PATH];
	WCHAR		strFileName[_MAX_PATH];
	WCHAR		strExt[_MAX_EXT];
	CIfo		Ifo;

	if (!GetTempPathW(MAX_PATH, szTempPath)) return FALSE;

	_wsplitpath_s (strIFOPath, NULL, 0, NULL, 0, strFileName, countof(strFileName), strExt, countof(strExt));
	_snwprintf_s  (strFakeFile, nFakeFileSize, _TRUNCATE, L"%sMPC%s%s", szTempPath, strFileName, strExt);

	if (Ifo.OpenFile (strIFOPath) &&
		Ifo.RemoveUOPs()  &&
		Ifo.SaveFile (strFakeFile))
    {
		bRet = TRUE;
	}

	return bRet;
}

HANDLE WINAPI Mine_CreateFileW(LPCWSTR p1, DWORD p2, DWORD p3, LPSECURITY_ATTRIBUTES p4, DWORD p5, DWORD p6, HANDLE p7)
{
	HANDLE	hFile = INVALID_HANDLE_VALUE;
	WCHAR	strFakeFile[_MAX_PATH];
	int		nLen  = wcslen(p1);

	p3 |= FILE_SHARE_WRITE;

	if (nLen>=4 && _wcsicmp (p1 + nLen-4, L".ifo") == 0)
	{
		if (CreateFakeVideoTS(p1, strFakeFile, countof(strFakeFile)))
		{
			hFile = Real_CreateFileW(strFakeFile, p2, p3, p4, p5, p6, p7);
		}
	}
	
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = Real_CreateFileW(p1, p2, p3, p4, p5, p6, p7);

	return hFile;
}


MMRESULT WINAPI Mine_mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
	if(fdwDetails == (MIXER_OBJECTF_HMIXER|MIXER_SETCONTROLDETAILSF_VALUE)) 
		return MMSYSERR_NOERROR; // don't touch the mixer, kthx
	return Real_mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
}

BOOL WINAPI Mine_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
	BOOL ret = Real_DeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

	if(IOCTL_DVD_GET_REGION == dwIoControlCode && lpOutBuffer
	&& lpBytesReturned && *lpBytesReturned == sizeof(DVD_REGION))
	{
		DVD_REGION* pDVDRegion = (DVD_REGION*)lpOutBuffer;
		pDVDRegion->SystemRegion = ~pDVDRegion->RegionData;
	}

	return ret;
}

#include "../../subtitles/SSF.h"
#include "../../subtitles/RTS.h"
#include "../../SubPic/MemSubPic.h"

class ssftest
{
public:
	ssftest()
	{
		Sleep(10000);

		MessageBeep(-1);
// 8; //
		SubPicDesc spd;
		spd.w = 640;
		spd.h = 480;
		spd.bpp = 32;
		spd.pitch = spd.w*spd.bpp>>3;
		spd.type = MSP_RGB32;
		spd.vidrect = CRect(0, 0, spd.w, spd.h);
		spd.bits = DNew BYTE[spd.pitch*spd.h];

		CCritSec csLock;
/*
		CRenderedTextSubtitle s(&csLock);
		s.Open(_T("../../subtitles/libssf/demo/demo.ssa"), 1);

		for(int i = 2*60*1000+2000; i < 2*60*1000+17000; i += 10)
		{
			memsetd(spd.bits, 0xff000000, spd.pitch*spd.h);
			CRect bbox;
			bbox.SetRectEmpty();
			s.Render(spd, 10000i64*i, 25, bbox);
		}
*/
		try
		{
			ssf::CRenderer s(&csLock);
			s.Open(_T("../../subtitles/libssf/demo/demo.ssf"));

			for(int i = 2*60*1000+2000; i < 2*60*1000+17000; i += 40)
			// for(int i = 2*60*1000+2000; i < 2*60*1000+17000; i += 1000)
			//for(int i = 0; i < 5000; i += 40)
			{
				memsetd(spd.bits, 0xff000000, spd.pitch*spd.h);
				CRect bbox;
				bbox.SetRectEmpty();
				s.Render(spd, 10000i64*i, 25, bbox);
			}
		}
		catch(ssf::Exception& e)
		{
			UNREFERENCED_PARAMETER(e);
			TRACE(_T("%s\n"), e.ToString());
			ASSERT(0);
		}

		delete [] spd.bits;

		::ExitProcess(0);
	}
};


BOOL CMPlayerCApp::InitInstance()
{
	long		lError;

	#ifdef GOTHTRACE
	// Used for tracing when debugger can't be used, e.g. when using some commercial decoders
	// Print traces usint _tprintf()
	if (AllocConsole())
	{
		FILE * foo; // Not used
		freopen_s(&foo, "conin$", "r", stdin); // Redirect stdin etc. to console
		freopen_s(&foo, "conout$", "w", stdout);
		freopen_s(&foo, "conout$", "w", stderr);
	}
	else
		AfxMessageBox(_T("Could not create console"));
	#endif

	DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)Real_IsDebuggerPresent, (PVOID)Mine_IsDebuggerPresent);
	DetourAttach(&(PVOID&)Real_ChangeDisplaySettingsExA, (PVOID)Mine_ChangeDisplaySettingsExA);
	DetourAttach(&(PVOID&)Real_ChangeDisplaySettingsExW, (PVOID)Mine_ChangeDisplaySettingsExW);
	DetourAttach(&(PVOID&)Real_CreateFileA, (PVOID)Mine_CreateFileA);
	DetourAttach(&(PVOID&)Real_CreateFileW, (PVOID)Mine_CreateFileW);
	DetourAttach(&(PVOID&)Real_mixerSetControlDetails, (PVOID)Mine_mixerSetControlDetails);
	DetourAttach(&(PVOID&)Real_DeviceIoControl, (PVOID)Mine_DeviceIoControl);

	HMODULE hNTDLL	=	LoadLibrary (_T("ntdll.dll"));
#ifndef _DEBUG	// Disable NtQueryInformationProcess in debug (prevent VS debugger to stop on crash address)
	if (hNTDLL)
	{		
		Real_NtQueryInformationProcess = (FUNC_NTQUERYINFORMATIONPROCESS)GetProcAddress (hNTDLL, "NtQueryInformationProcess");

		if (Real_NtQueryInformationProcess)
			DetourAttach(&(PVOID&)Real_NtQueryInformationProcess, (PVOID)Mine_NtQueryInformationProcess);
	}
#endif

	CFilterMapper2::Init();

    lError = DetourTransactionCommit();
	ASSERT (lError == NOERROR);

	HRESULT hr;
    if(FAILED(hr = OleInitialize(0)))
	{
        AfxMessageBox(_T("OleInitialize failed!"));
		return FALSE;
	}

    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(WNDCLASS));
    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = ::DefWindowProc; 
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = LoadIcon(IDR_MAINFRAME);
    wndcls.hCursor = LoadCursor(IDC_ARROW);
    wndcls.hbrBackground = 0;//(HBRUSH)(COLOR_WINDOW + 1); // no bkg brush, the view and the bars should always fill the whole client area
    wndcls.lpszMenuName = NULL;
    wndcls.lpszClassName = MPC_WND_CLASS_NAME;

	if(!AfxRegisterClass(&wndcls))
    {
		AfxMessageBox(_T("MainFrm class registration failed!"));
		return FALSE;
    }

	if(!AfxSocketInit(NULL))
	{
        AfxMessageBox(_T("AfxSocketInit failed!"));
		return FALSE;
	}

	PreProcessCommandLine();

	if(IsIniValid())
		StoreSettingsToIni();
	else
	{
		StoreSettingsToRegistry();

		// Only create a folder when using registry to store settings
		CString AppSavePath;
		if(GetAppSavePath(AppSavePath))
			CreateDirectory(AppSavePath, NULL);
	}

	m_s.ParseCommandLine(m_cmdln);

	if(m_s.nCLSwitches&(CLSW_HELP|CLSW_UNRECOGNIZEDSWITCH))
	{
		m_s.UpdateData(false);
		ShowCmdlnSwitches();
		return FALSE;
	}

	if((m_s.nCLSwitches&CLSW_CLOSE) && m_s.slFiles.IsEmpty())
	{
		return FALSE;
	}

	m_s.UpdateData(false);

	if((m_s.nCLSwitches&CLSW_REGEXTVID) || (m_s.nCLSwitches&CLSW_REGEXTAUD))
	{
		CMediaFormats& mf = m_s.Formats;

		for(int i = 0; i < (int)mf.GetCount(); i++)
		{
			if(!mf[i].GetLabel().CompareNoCase(ResStr(IDS_AG_PLAYLIST_FILE))) continue;
				
			bool fAudioOnly = mf[i].IsAudioOnly();

			int j = 0;
			CString str = mf[i].GetExtsWithPeriod();
			for(CString ext = str.Tokenize(_T(" "), j); !ext.IsEmpty(); ext = str.Tokenize(_T(" "), j))
			{
				if(((m_s.nCLSwitches&CLSW_REGEXTVID) && !fAudioOnly) || ((m_s.nCLSwitches&CLSW_REGEXTAUD) && fAudioOnly)) {
					CPPageFormats::RegisterExt(ext, mf[i].GetLabel(), true);
				}
			}
		}
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

		return FALSE;
	}
	
	if((m_s.nCLSwitches&CLSW_UNREGEXT))
	{
		CMediaFormats& mf = m_s.Formats;

		for(int i = 0; i < (int)mf.GetCount(); i++)
		{
			int j = 0;
			CString str = mf[i].GetExtsWithPeriod();
			for(CString ext = str.Tokenize(_T(" "), j); !ext.IsEmpty(); ext = str.Tokenize(_T(" "), j))
			{
				CPPageFormats::RegisterExt(ext, mf[i].GetLabel(), false);
			}
		}
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

		return FALSE;
	}

	// Enable to open options with administrator privilege (for Vista UAC)
	if (m_s.nCLSwitches & CLSW_ADMINOPTION)
	{
		switch (m_s.iAdminOption)
		{
		case CPPageFormats::IDD :
			{
				CPPageSheet options(ResStr(IDS_OPTIONS_CAPTION), NULL, NULL, m_s.iAdminOption);
				options.LockPage();
				options.DoModal();
			}
			break;

		default :
			ASSERT (FALSE);
		}
		return FALSE;
	}

	m_mutexOneInstance.Create(NULL, TRUE, MPC_WND_CLASS_NAME);

	if(GetLastError() == ERROR_ALREADY_EXISTS
	&& (!(m_s.fAllowMultipleInst || (m_s.nCLSwitches&CLSW_NEW) || m_cmdln.IsEmpty())
		|| (m_s.nCLSwitches&CLSW_ADD)))
	{
		int wait_count = 0;
		HWND hWnd = ::FindWindow(MPC_WND_CLASS_NAME, NULL);
		while(!hWnd && (wait_count++<200))
		{
			Sleep(100);
			hWnd = ::FindWindow(MPC_WND_CLASS_NAME, NULL);
		}
		if(hWnd && (wait_count<200))
		{
			SetForegroundWindow(hWnd);

			if(!(m_s.nCLSwitches&CLSW_MINIMIZED) && IsIconic(hWnd))
				ShowWindow(hWnd, SW_RESTORE);

			SendCommandLine(hWnd);

			return FALSE;
		} 
	}

	if(!__super::InitInstance())
	{
		AfxMessageBox(_T("InitInstance failed!"));
		return FALSE;
	}

	CRegKey key;
	if(ERROR_SUCCESS == key.Create(HKEY_LOCAL_MACHINE, _T("Software\\Gabest\\Media Player Classic")))
	{
		CString path;
		GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(MAX_PATH), MAX_PATH);
		path.ReleaseBuffer();
		key.SetStringValue(_T("ExePath"), path);
	}

	AfxEnableControlContainer();

	CMainFrame* pFrame = DNew CMainFrame;
	m_pMainWnd = pFrame;
	if ( !pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW|FWS_ADDTOTITLE, NULL, NULL) )
	{
		AfxMessageBox(_T("CMainFrame::LoadFrame failed!"));
		return FALSE;
	}
	pFrame->SetDefaultWindowRect((m_s.nCLSwitches&CLSW_MONITOR)?m_s.iMonitor:0);
	pFrame->RestoreFloatingControlBars();
	pFrame->SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
	pFrame->DragAcceptFiles();
	pFrame->ShowWindow((m_s.nCLSwitches&CLSW_MINIMIZED)?SW_SHOWMINIMIZED:SW_SHOW);
	pFrame->UpdateWindow();
	pFrame->m_hAccelTable = m_s.hAccel;
	m_s.WinLircClient.SetHWND(m_pMainWnd->m_hWnd);
	if(m_s.fWinLirc) m_s.WinLircClient.Connect(m_s.WinLircAddr);
	m_s.UIceClient.SetHWND(m_pMainWnd->m_hWnd);
	if(m_s.fUIce) m_s.UIceClient.Connect(m_s.UIceAddr);

	SendCommandLine(m_pMainWnd->m_hWnd);
	RegisterHotkeys();

	pFrame->SetFocus();

	return TRUE;
}

UINT CMPlayerCApp::GetRemoteControlCodeMicrosoft(UINT nInputcode, HRAWINPUT hRawInput)
{
	UINT		dwSize		= 0;
	BYTE*		pRawBuffer	= NULL;
	UINT		nMceCmd		= 0;

	// Support for MCE remote control
	GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	if (dwSize > 0)
	{
		pRawBuffer = DNew BYTE[dwSize];
		if (GetRawInputData(hRawInput, RID_INPUT, pRawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != -1)
		{
			RAWINPUT*	raw = (RAWINPUT*) pRawBuffer;
			if(raw->header.dwType == RIM_TYPEHID)
			{
				nMceCmd = 0x10000 + (raw->data.hid.bRawData[1] | raw->data.hid.bRawData[2] << 8);
			}
		}
		delete [] pRawBuffer;
	}

	return nMceCmd;
}

UINT CMPlayerCApp::GetRemoteControlCodeSRM7500(UINT nInputcode, HRAWINPUT hRawInput)
{
	UINT		dwSize		= 0;
	BYTE*		pRawBuffer	= NULL;
	UINT		nMceCmd		= 0;

	GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	if (dwSize > 21)
	{
		pRawBuffer = DNew BYTE[dwSize];
		if (GetRawInputData(hRawInput, RID_INPUT, pRawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != -1)
		{
			RAWINPUT*	raw = (RAWINPUT*) pRawBuffer;

			// data.hid.bRawData[21] set to one when key is pressed
			if(raw->header.dwType == RIM_TYPEHID && raw->data.hid.bRawData[21] == 1)
			{
				// data.hid.bRawData[21] has keycode
				switch (raw->data.hid.bRawData[20])
				{
				case 0x0033 : nMceCmd = MCE_DETAILS; break;
				case 0x0022 : nMceCmd = MCE_GUIDE; break;
				case 0x0036 : nMceCmd = MCE_MYTV; break;
				case 0x0026 : nMceCmd = MCE_RECORDEDTV; break;
				case 0x0005 : nMceCmd = MCE_RED; break;
				case 0x0002 : nMceCmd = MCE_GREEN; break;
				case 0x0045 : nMceCmd = MCE_YELLOW; break;
				case 0x0046 : nMceCmd = MCE_BLUE; break;
				case 0x000A : nMceCmd = MCE_MEDIA_PREVIOUSTRACK; break;
				case 0x004A : nMceCmd = MCE_MEDIA_NEXTTRACK; break;
				}
			}
		}
		delete [] pRawBuffer;
	}

	return nMceCmd;
}

void CMPlayerCApp::RegisterHotkeys()
{
	RAWINPUTDEVICELIST	InputDeviceList[50];
	UINT				nInputDeviceCount = countof(InputDeviceList);
	RID_DEVICE_INFO		DevInfo;
	RAWINPUTDEVICE		MCEInputDevice[] = 
	{
		//	usUsagePage		usUsage			dwFlags		hwndTarget
		{	0xFFBC,			0x88,				0,		NULL},
		{	0x000C,			0x01,				0,		NULL},
		{	0x000C,			0x80,				0,		NULL}
	};

	// Register MCE Remote Control raw input
	for (int i=0; i<countof(MCEInputDevice); i++)
		MCEInputDevice[i].hwndTarget = m_pMainWnd->m_hWnd;

	nInputDeviceCount = GetRawInputDeviceList (InputDeviceList, &nInputDeviceCount, sizeof(RAWINPUTDEVICELIST));
	for (int i=0; i<nInputDeviceCount; i++)
	{
		UINT	nTemp = sizeof(DevInfo);
		if (GetRawInputDeviceInfo (InputDeviceList[i].hDevice, RIDI_DEVICEINFO, &DevInfo, &nTemp)>0)
		{
			if (DevInfo.hid.dwVendorId == 0x00000471 &&		// Philips HID vendor id
				DevInfo.hid.dwProductId == 0x00000617)		// IEEE802.15.4 RF Dongle (SRM 7500)
			{
				MCEInputDevice[0].usUsagePage	= DevInfo.hid.usUsagePage;
				MCEInputDevice[0].usUsage		= DevInfo.hid.usUsage;
				GetRemoteControlCode = GetRemoteControlCodeSRM7500;
			}
		}
	}


	RegisterRawInputDevices (MCEInputDevice, countof(MCEInputDevice), sizeof(RAWINPUTDEVICE));


	if (m_s.fGlobalMedia)
	{
		POSITION pos = m_s.wmcmds.GetHeadPosition();

		while(pos)
		{
			wmcmd& wc = m_s.wmcmds.GetNext(pos);
			if (wc.appcmd != 0) 
				RegisterHotKey(m_pMainWnd->m_hWnd, wc.appcmd, 0, GetVKFromAppCommand (wc.appcmd));
		}
	}
}

void CMPlayerCApp::UnregisterHotkeys()
{
	if (m_s.fGlobalMedia)
	{
		POSITION pos = m_s.wmcmds.GetHeadPosition();

		while(pos)
		{
			wmcmd& wc = m_s.wmcmds.GetNext(pos);
			if (wc.appcmd != 0) 
				UnregisterHotKey(m_pMainWnd->m_hWnd, wc.appcmd);
		}
	}
}

UINT CMPlayerCApp::GetVKFromAppCommand(UINT nAppCommand)
{
	switch (nAppCommand)
	{
		case APPCOMMAND_BROWSER_BACKWARD	: return VK_BROWSER_BACK;
		case APPCOMMAND_BROWSER_FORWARD		: return VK_BROWSER_FORWARD;
		case APPCOMMAND_BROWSER_REFRESH		: return VK_BROWSER_REFRESH;
		case APPCOMMAND_BROWSER_STOP		: return VK_BROWSER_STOP;
		case APPCOMMAND_BROWSER_SEARCH		: return VK_BROWSER_SEARCH;
		case APPCOMMAND_BROWSER_FAVORITES	: return VK_BROWSER_FAVORITES;
		case APPCOMMAND_BROWSER_HOME		: return VK_BROWSER_HOME;
		case APPCOMMAND_VOLUME_MUTE			: return VK_VOLUME_MUTE;
		case APPCOMMAND_VOLUME_DOWN			: return VK_VOLUME_DOWN;
		case APPCOMMAND_VOLUME_UP			: return VK_VOLUME_UP;
		case APPCOMMAND_MEDIA_NEXTTRACK		: return VK_MEDIA_NEXT_TRACK;
		case APPCOMMAND_MEDIA_PREVIOUSTRACK	: return VK_MEDIA_PREV_TRACK;
		case APPCOMMAND_MEDIA_STOP			: return VK_MEDIA_STOP;
		case APPCOMMAND_MEDIA_PLAY_PAUSE	: return VK_MEDIA_PLAY_PAUSE;
		case APPCOMMAND_LAUNCH_MAIL			: return VK_LAUNCH_MAIL;
		case APPCOMMAND_LAUNCH_MEDIA_SELECT	: return VK_LAUNCH_MEDIA_SELECT;
		case APPCOMMAND_LAUNCH_APP1			: return VK_LAUNCH_APP1;
		case APPCOMMAND_LAUNCH_APP2			: return VK_LAUNCH_APP2;
	}

	return 0;}

int CMPlayerCApp::ExitInstance()
{
	m_s.UpdateData(true);

	OleUninitialize();

	return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp message handlers
// App command to run the dialog

void CMPlayerCApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CMPlayerCApp::OnFileExit()
{
	OnAppExit();
}

// CRemoteCtrlClient

CRemoteCtrlClient::CRemoteCtrlClient() 
	: m_pWnd(NULL)
	, m_nStatus(DISCONNECTED)
{
}

void CRemoteCtrlClient::SetHWND(HWND hWnd)
{
	CAutoLock cAutoLock(&m_csLock);

	m_pWnd = CWnd::FromHandle(hWnd);
}

void CRemoteCtrlClient::Connect(CString addr)
{
	CAutoLock cAutoLock(&m_csLock);

	if(m_nStatus == CONNECTING && m_addr == addr)
	{
		TRACE(_T("CRemoteCtrlClient (Connect): already connecting to %s\n"), addr);
		return;
	}

	if(m_nStatus == CONNECTED && m_addr == addr)
	{
		TRACE(_T("CRemoteCtrlClient (Connect): already connected to %s\n"), addr);
		return;
	}

	m_nStatus = CONNECTING;

	TRACE(_T("CRemoteCtrlClient (Connect): connecting to %s\n"), addr);

	Close();

	Create();

	CString ip = addr.Left(addr.Find(':')+1).TrimRight(':');
	int port = _tcstol(addr.Mid(addr.Find(':')+1), NULL, 10);

	__super::Connect(ip, port);

	m_addr = addr;
}

void CRemoteCtrlClient::OnConnect(int nErrorCode)
{
	CAutoLock cAutoLock(&m_csLock);

	m_nStatus = (nErrorCode == 0 ? CONNECTED : DISCONNECTED);

	TRACE(_T("CRemoteCtrlClient (OnConnect): %d\n"), nErrorCode);
}

void CRemoteCtrlClient::OnClose(int nErrorCode)
{
	CAutoLock cAutoLock(&m_csLock);

	if(m_hSocket != INVALID_SOCKET && m_nStatus == CONNECTED)
	{
		TRACE(_T("CRemoteCtrlClient (OnClose): connection lost\n"));
	}

	m_nStatus = DISCONNECTED;

	TRACE(_T("CRemoteCtrlClient (OnClose): %d\n"), nErrorCode);
}

void CRemoteCtrlClient::OnReceive(int nErrorCode)
{
	if(nErrorCode != 0 || !m_pWnd) return;

	CStringA str;
	int ret = Receive(str.GetBuffer(256), 255, 0);
	if(ret <= 0) return;
	str.ReleaseBuffer(ret);

	TRACE(_T("CRemoteCtrlClient (OnReceive): %s\n"), CString(str));

	OnCommand(str);

	__super::OnReceive(nErrorCode);
}

void CRemoteCtrlClient::ExecuteCommand(CStringA cmd, int repcnt)
{
	cmd.Trim();
	if(cmd.IsEmpty()) return;
	cmd.Replace(' ', '_');

	AppSettings& s = AfxGetAppSettings();

	POSITION pos = s.wmcmds.GetHeadPosition();
	while(pos)
	{
		wmcmd wc = s.wmcmds.GetNext(pos);
		CStringA name = TToA(wc.GetName());
		name.Replace(' ', '_');
		if((repcnt == 0 && wc.rmrepcnt == 0 || wc.rmrepcnt > 0 && (repcnt%wc.rmrepcnt) == 0)
		&& (!name.CompareNoCase(cmd) || !wc.rmcmd.CompareNoCase(cmd) || wc.cmd == (WORD)strtol(cmd, NULL, 10)))
		{
			CAutoLock cAutoLock(&m_csLock);
			TRACE(_T("CRemoteCtrlClient (calling command): %s\n"), wc.GetName());
			m_pWnd->SendMessage(WM_COMMAND, wc.cmd);
			break;
		}
	}
}

// CWinLircClient

CWinLircClient::CWinLircClient()
{
}

void CWinLircClient::OnCommand(CStringA str)
{
	TRACE(_T("CWinLircClient (OnCommand): %s\n"), CString(str));

	int i = 0, j = 0, repcnt = 0;
	for(CStringA token = str.Tokenize(" ", i); 
		!token.IsEmpty();
		token = str.Tokenize(" ", i), j++)
	{
		if(j == 1)
			repcnt = strtol(token, NULL, 16);
		else if(j == 2)
			ExecuteCommand(token, repcnt);
	}
}

// CUIceClient

CUIceClient::CUIceClient()
{
}

void CUIceClient::OnCommand(CStringA str)
{
	TRACE(_T("CUIceClient (OnCommand): %s\n"), CString(str));

	CStringA cmd;
	int i = 0, j = 0;
	for(CStringA token = str.Tokenize("|", i); 
		!token.IsEmpty(); 
		token = str.Tokenize("|", i), j++)
	{
		if(j == 0)
			cmd = token;
		else if(j == 1)
			ExecuteCommand(cmd, strtol(token, NULL, 16));
	}
}

// CMPlayerCApp::Settings

CMPlayerCApp::Settings::Settings() 
	: fInitialized(false)
	, MRU(0, _T("Recent File List"), _T("File%d"), 20)
	, MRUDub(0, _T("Recent Dub List"), _T("Dub%d"), 20)
	, hAccel(NULL)
{
}



void CMPlayerCApp::Settings::CreateCommands()
{
#define ADDCMD(cmd) wmcmds.AddTail(wmcmd##cmd)
	ADDCMD((ID_FILE_OPENQUICK,					'Q', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_0));
	ADDCMD((ID_FILE_OPENMEDIA,					'O', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_FILE));
	ADDCMD((ID_FILE_OPENDVD,					'D', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_DVD));
	ADDCMD((ID_FILE_OPENDEVICE,					'V', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_OPEN_DEVICE));
	ADDCMD((ID_FILE_SAVE_COPY,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_SAVE_AS));
	ADDCMD((ID_FILE_SAVE_IMAGE,					'I', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_SAVE_IMAGE));
	ADDCMD((ID_FILE_SAVE_IMAGE_AUTO,		  VK_F5, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_6));
	ADDCMD((ID_FILE_LOAD_SUBTITLE,				'L', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_LOAD_SUBTITLE));
	ADDCMD((ID_FILE_SAVE_SUBTITLE,				'S', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_SAVE_SUBTITLE));
	ADDCMD((ID_FILE_CLOSEPLAYLIST,				'C', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_CLOSE));
	ADDCMD((ID_FILE_PROPERTIES,				 VK_F10, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PROPERTIES));
	ADDCMD((ID_FILE_EXIT,						'X', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_EXIT));
	ADDCMD((ID_PLAY_PLAYPAUSE,			   VK_SPACE, FVIRTKEY|FNOINVERT,				IDS_AG_PLAYPAUSE,	APPCOMMAND_MEDIA_PLAY_PAUSE, wmcmd::LDOWN));
	ADDCMD((ID_PLAY_PLAY,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_PLAY,		APPCOMMAND_MEDIA_PLAY));
	ADDCMD((ID_PLAY_PAUSE,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_PAUSE,		APPCOMMAND_MEDIA_PAUSE));
	ADDCMD((ID_PLAY_STOP,			  VK_OEM_PERIOD, FVIRTKEY|FNOINVERT,				IDS_AG_STOP,		APPCOMMAND_MEDIA_STOP));
	ADDCMD((ID_PLAY_FRAMESTEP,			   VK_RIGHT, FVIRTKEY|FNOINVERT,				IDS_AG_FRAMESTEP));
	ADDCMD((ID_PLAY_FRAMESTEPCANCEL,		VK_LEFT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_16));
	ADDCMD((ID_PLAY_GOTO,						'G', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_GO_TO));
	ADDCMD((ID_PLAY_INCRATE,				  VK_UP, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_INCREASE_RATE));
	ADDCMD((ID_PLAY_DECRATE,				VK_DOWN, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_DECREASE_RATE));
	ADDCMD((ID_PLAY_RESETRATE,					'R', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_RESET_RATE));
	ADDCMD((ID_PLAY_INCAUDDELAY,			 VK_ADD, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_21));
	ADDCMD((ID_PLAY_DECAUDDELAY,		VK_SUBTRACT, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_22));
	ADDCMD((ID_PLAY_SEEKFORWARDSMALL,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_23));
	ADDCMD((ID_PLAY_SEEKBACKWARDSMALL,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_24));
	ADDCMD((ID_PLAY_SEEKFORWARDMED,		   VK_RIGHT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_25));
	ADDCMD((ID_PLAY_SEEKBACKWARDMED,	    VK_LEFT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_26));
	ADDCMD((ID_PLAY_SEEKFORWARDLARGE,		      0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_27));
	ADDCMD((ID_PLAY_SEEKBACKWARDLARGE,		      0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_28));
	ADDCMD((ID_PLAY_SEEKKEYFORWARD,		   VK_RIGHT, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_MPLAYERC_29));
	ADDCMD((ID_PLAY_SEEKKEYBACKWARD,	    VK_LEFT, FVIRTKEY|FSHIFT|FNOINVERT,			IDS_MPLAYERC_30));
	ADDCMD((ID_NAVIGATE_SKIPFORWARD,	    VK_NEXT, FVIRTKEY|FNOINVERT,				IDS_AG_NEXT, APPCOMMAND_MEDIA_NEXTTRACK, wmcmd::X2DOWN));
	ADDCMD((ID_NAVIGATE_SKIPBACK,		   VK_PRIOR, FVIRTKEY|FNOINVERT,				IDS_AG_PREVIOUS, APPCOMMAND_MEDIA_PREVIOUSTRACK, wmcmd::X1DOWN));
	ADDCMD((ID_NAVIGATE_SKIPFORWARDPLITEM,  VK_NEXT, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_33));
	ADDCMD((ID_NAVIGATE_SKIPBACKPLITEM,	   VK_PRIOR, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_34));
	ADDCMD((ID_NAVIGATE_TUNERSCAN,	            'T', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_NAVIGATE_TUNERSCAN));
	ADDCMD((ID_FAVORITES_QUICKADDFAVORITE,	    'Q', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_FAVORITES_QUICKADDFAVORITE));
	ADDCMD((ID_VIEW_CAPTIONMENU,				'0', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CAPTION));
	ADDCMD((ID_VIEW_SEEKER,						'1', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SEEKER));
	ADDCMD((ID_VIEW_CONTROLS,					'2', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CONTROLS));
	ADDCMD((ID_VIEW_INFORMATION,				'3', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_INFO));
	ADDCMD((ID_VIEW_STATISTICS,					'4', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_STATS));
	ADDCMD((ID_VIEW_STATUS,						'5', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_STATUS));
	ADDCMD((ID_VIEW_SUBRESYNC,					'6', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SUBRESYNC));
	ADDCMD((ID_VIEW_PLAYLIST,					'7', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_PLAYLIST));
	ADDCMD((ID_VIEW_CAPTURE,					'8', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_CAPTURE));
	ADDCMD((ID_VIEW_SHADEREDITOR,				'9', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TOGGLE_SHADER));
	ADDCMD((ID_VIEW_PRESETS_MINIMAL,			'1', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_MINIMAL));
	ADDCMD((ID_VIEW_PRESETS_COMPACT,			'2', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_COMPACT));
	ADDCMD((ID_VIEW_PRESETS_NORMAL,				'3', FVIRTKEY|FNOINVERT,				IDS_AG_VIEW_NORMAL));
	ADDCMD((ID_VIEW_FULLSCREEN,			  VK_RETURN, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_FULLSCREEN, 0, wmcmd::LDBLCLK));
	ADDCMD((ID_VIEW_FULLSCREEN_SECONDARY, VK_RETURN, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_39));
	ADDCMD((ID_VIEW_ZOOM_50,					'1', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_50));
	ADDCMD((ID_VIEW_ZOOM_100,					'2', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_100));
	ADDCMD((ID_VIEW_ZOOM_200,					'3', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_200));
	ADDCMD((ID_VIEW_ZOOM_AUTOFIT,				'4', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_ZOOM_AUTO_FIT));	
	ADDCMD((ID_ASPECTRATIO_NEXT,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_AR_PRESET));
	ADDCMD((ID_VIEW_VF_HALF,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_HALF));
	ADDCMD((ID_VIEW_VF_NORMAL,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_NORMAL));
	ADDCMD((ID_VIEW_VF_DOUBLE,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_DOUBLE));
	ADDCMD((ID_VIEW_VF_STRETCH,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_STRETCH));
	ADDCMD((ID_VIEW_VF_FROMINSIDE,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_INSIDE));
	ADDCMD((ID_VIEW_VF_FROMOUTSIDE,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_VIDFRM_OUTSIDE));
	ADDCMD((ID_ONTOP_ALWAYS,					'A', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_ALWAYS_ON_TOP));
	ADDCMD((ID_VIEW_RESET,				 VK_NUMPAD5, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_RESET));
	ADDCMD((ID_VIEW_INCSIZE,			 VK_NUMPAD9, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_INC_SIZE));
	ADDCMD((ID_VIEW_INCWIDTH,			 VK_NUMPAD6, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_INC_WIDTH));
	ADDCMD((ID_VIEW_INCHEIGHT,			 VK_NUMPAD8, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_47));
	ADDCMD((ID_VIEW_DECSIZE,			 VK_NUMPAD1, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_DEC_SIZE));
	ADDCMD((ID_VIEW_DECWIDTH,			 VK_NUMPAD4, FVIRTKEY|FNOINVERT,				IDS_AG_PNS_DEC_WIDTH));
	ADDCMD((ID_VIEW_DECHEIGHT,			 VK_NUMPAD2, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_50));
	ADDCMD((ID_PANSCAN_CENTER,			 VK_NUMPAD5, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_CENTER));
	ADDCMD((ID_PANSCAN_MOVELEFT,		 VK_NUMPAD4, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_LEFT));
	ADDCMD((ID_PANSCAN_MOVERIGHT,		 VK_NUMPAD6, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_RIGHT));
	ADDCMD((ID_PANSCAN_MOVEUP,			 VK_NUMPAD8, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UP));
	ADDCMD((ID_PANSCAN_MOVEDOWN,		 VK_NUMPAD2, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_DOWN));
	ADDCMD((ID_PANSCAN_MOVEUPLEFT,		 VK_NUMPAD7, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UPLEFT));
	ADDCMD((ID_PANSCAN_MOVEUPRIGHT,		 VK_NUMPAD9, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_UPRIGHT));
	ADDCMD((ID_PANSCAN_MOVEDOWNLEFT,	 VK_NUMPAD1, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_PNS_DOWNLEFT));
	ADDCMD((ID_PANSCAN_MOVEDOWNRIGHT,	 VK_NUMPAD3, FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_59));
	ADDCMD((ID_PANSCAN_ROTATEXP,		 VK_NUMPAD8, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEX_P));
	ADDCMD((ID_PANSCAN_ROTATEXM,		 VK_NUMPAD2, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEX_M));
	ADDCMD((ID_PANSCAN_ROTATEYP,		 VK_NUMPAD4, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEY_P));
	ADDCMD((ID_PANSCAN_ROTATEYM,		 VK_NUMPAD6, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEY_M));
	ADDCMD((ID_PANSCAN_ROTATEZP,		 VK_NUMPAD1, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEZ_P));
	ADDCMD((ID_PANSCAN_ROTATEZM,		 VK_NUMPAD3, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_PNS_ROTATEZ_M));
	ADDCMD((ID_VOLUME_UP,					  VK_UP, FVIRTKEY|FNOINVERT,				IDS_AG_VOLUME_UP,	APPCOMMAND_VOLUME_UP, wmcmd::WUP));
	ADDCMD((ID_VOLUME_DOWN,				    VK_DOWN, FVIRTKEY|FNOINVERT,				IDS_AG_VOLUME_DOWN, APPCOMMAND_VOLUME_DOWN, wmcmd::WDOWN));
	ADDCMD((ID_VOLUME_MUTE,						'M', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_VOLUME_MUTE, APPCOMMAND_VOLUME_MUTE));
	ADDCMD((ID_VOLUME_BOOST_INC,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_INC));
	ADDCMD((ID_VOLUME_BOOST_DEC,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_DEC));
	ADDCMD((ID_VOLUME_BOOST_MIN,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_MIN));
	ADDCMD((ID_VOLUME_BOOST_MAX,				  0, FVIRTKEY|FNOINVERT,				IDS_VOLUME_BOOST_MAX));
	ADDCMD((ID_NAVIGATE_TITLEMENU,				'T', FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_63));
	ADDCMD((ID_NAVIGATE_ROOTMENU,				'R', FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_ROOT_MENU));
	ADDCMD((ID_NAVIGATE_SUBPICTUREMENU,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_65));
	ADDCMD((ID_NAVIGATE_AUDIOMENU,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_66));
	ADDCMD((ID_NAVIGATE_ANGLEMENU,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_67));
	ADDCMD((ID_NAVIGATE_CHAPTERMENU,			  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_68));
	ADDCMD((ID_NAVIGATE_MENU_LEFT,		    VK_LEFT, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_LEFT));
	ADDCMD((ID_NAVIGATE_MENU_RIGHT,		   VK_RIGHT, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_70));
	ADDCMD((ID_NAVIGATE_MENU_UP,			  VK_UP, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_UP));
	ADDCMD((ID_NAVIGATE_MENU_DOWN,		    VK_DOWN, FVIRTKEY|FALT|FNOINVERT,			IDS_AG_DVD_MENU_DOWN));
	ADDCMD((ID_NAVIGATE_MENU_ACTIVATE,	   VK_SPACE, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_73));
	ADDCMD((ID_NAVIGATE_MENU_BACK,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_DVD_MENU_BACK));
	ADDCMD((ID_NAVIGATE_MENU_LEAVE,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_75));
	ADDCMD((ID_BOSS,							'B', FVIRTKEY|FNOINVERT,				IDS_AG_BOSS_KEY));
	ADDCMD((ID_MENU_PLAYER_SHORT,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_77, 0, wmcmd::RUP));
	ADDCMD((ID_MENU_PLAYER_LONG,				  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_78));
	ADDCMD((ID_MENU_FILTERS,					  0, FVIRTKEY|FNOINVERT,				IDS_AG_FILTERS_MENU));
	ADDCMD((ID_VIEW_OPTIONS,					'O', FVIRTKEY|FNOINVERT,				IDS_AG_OPTIONS));
	ADDCMD((ID_STREAM_AUDIO_NEXT,				'A', FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_AUDIO));
	ADDCMD((ID_STREAM_AUDIO_PREV,				'A', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PREV_AUDIO));
	ADDCMD((ID_STREAM_SUB_NEXT,					'S', FVIRTKEY|FNOINVERT,				IDS_AG_NEXT_SUBTITLE));
	ADDCMD((ID_STREAM_SUB_PREV,					'S', FVIRTKEY|FSHIFT|FNOINVERT,			IDS_AG_PREV_SUBTITLE));
	ADDCMD((ID_STREAM_SUB_ONOFF,				'W', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_85));
	ADDCMD((ID_SUBTITLES_SUBITEM_START+2,		  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_86));
	ADDCMD((ID_OGM_AUDIO_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_87));
	ADDCMD((ID_OGM_AUDIO_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_88));
	ADDCMD((ID_OGM_SUB_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_89));
	ADDCMD((ID_OGM_SUB_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_90));
	ADDCMD((ID_DVD_ANGLE_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_91));
	ADDCMD((ID_DVD_ANGLE_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_92));
	ADDCMD((ID_DVD_AUDIO_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_93));
	ADDCMD((ID_DVD_AUDIO_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_94));
	ADDCMD((ID_DVD_SUB_NEXT,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_95));
	ADDCMD((ID_DVD_SUB_PREV,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_96));
	ADDCMD((ID_DVD_SUB_ONOFF,					  0, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_97));
	ADDCMD((ID_VIEW_TEARING_TEST,				'T', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_TEARING_TEST));
	ADDCMD((ID_VIEW_REMAINING_TIME,				'I', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_98));
	ADDCMD((ID_SHADER_TOGGLE,					'P', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AT_TOGGLE_SHADER));
	ADDCMD((ID_SHADER_TOGGLESCREENSPACE,		'P', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AT_TOGGLE_SHADERSCREENSPACE));
	ADDCMD((ID_D3DFULLSCREEN_TOGGLE,			'F', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_MPLAYERC_99));
	ADDCMD((ID_GOTO_PREV_SUB,					'Y', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_100, APPCOMMAND_BROWSER_BACKWARD));
	ADDCMD((ID_GOTO_NEXT_SUB,					'U', FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_101,  APPCOMMAND_BROWSER_FORWARD));
	ADDCMD((ID_SHIFT_SUB_DOWN,				VK_NEXT, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_102));
	ADDCMD((ID_SHIFT_SUB_UP,			   VK_PRIOR, FVIRTKEY|FALT|FNOINVERT,			IDS_MPLAYERC_103));
	ADDCMD((ID_VIEW_DISPLAYSTATS,				'J', FVIRTKEY|FCONTROL|FNOINVERT,		IDS_AG_DISPLAY_STATS));
	ADDCMD((ID_VIEW_RESETSTATS,					'R', FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_RESET_STATS));
	ADDCMD((ID_VIEW_VSYNC,						'V', FVIRTKEY|FNOINVERT,				IDS_AG_VSYNC));
	ADDCMD((ID_VIEW_ENABLEFRAMETIMECORRECTION,  'C', FVIRTKEY|FNOINVERT,				IDS_AG_ENABLEFRAMETIMECORRECTION));
	ADDCMD((ID_VIEW_VSYNCACCURATE,				'V', FVIRTKEY|FCONTROL|FALT|FNOINVERT,			IDS_AG_VSYNCACCURATE));
	ADDCMD((ID_VIEW_VSYNCOFFSET_DECREASE,    VK_UP, FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCOFFSET_DECREASE));
	ADDCMD((ID_VIEW_VSYNCOFFSET_INCREASE,	VK_DOWN, FVIRTKEY|FCONTROL|FALT|FNOINVERT,	IDS_AG_VSYNCOFFSET_INCREASE));
	ADDCMD((ID_SUB_DELAY_DOWN,				  VK_F1, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_104));
	ADDCMD((ID_SUB_DELAY_UP,				  VK_F2, FVIRTKEY|FNOINVERT,				IDS_MPLAYERC_105));
	ADDCMD((ID_FILE_SAVE_THUMBNAILS,			  0, FVIRTKEY|FNOINVERT,				IDS_FILE_SAVE_THUMBNAILS));

	ADDCMD((ID_VIEW_EDITLISTEDITOR,				  0, FVIRTKEY|FNOINVERT,				IDS_AG_TOGGLE_EDITLISTEDITOR));
	ADDCMD((ID_EDL_IN,							  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_IN));
	ADDCMD((ID_EDL_OUT,							  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDL_OUT));
	ADDCMD((ID_EDL_NEWCLIP,						  0, FVIRTKEY|FNOINVERT,				IDS_AG_EDO_NEW_CLIP));

	ResetPositions();

#undef ADDCMD
}

CMPlayerCApp::Settings::~Settings()
{
	if(hAccel)
		DestroyAcceleratorTable(hAccel);
}

bool CMPlayerCApp::Settings::IsD3DFullscreen()
{
	if(nCLSwitches&CLSW_D3DFULLSCREEN)
		return true;
	else
		return fD3DFullscreen;
}
CString CMPlayerCApp::Settings::SelectedAudioRender()
{
	CString	strResult;
	if	(nCLSwitches&CLSW_AUDIORENDER)
		strResult = AfxGetMyApp()->m_AudioRendererDisplayName_CL;
	else
		strResult = AfxGetAppSettings().AudioRendererDisplayName;
	return strResult;
}

void CMPlayerCApp::Settings::ResetPositions()
{
	nCurrentDvdPosition		= -1;
	nCurrentFilePosition	= -1;
}


DVD_POSITION* CMPlayerCApp::Settings::CurrentDVDPosition()
{
	if (nCurrentDvdPosition != -1)
		return &DvdPosition[nCurrentDvdPosition];
	else
		return NULL;
}

bool CMPlayerCApp::Settings::NewDvd(ULONGLONG llDVDGuid)
{
	int			i;

	// Recherche si la position du DVD est connue
	for (i=0; i<MAX_DVD_POSITION; i++)
	{
		if (DvdPosition[i].llDVDGuid == llDVDGuid)
		{
			nCurrentDvdPosition = i;
			return false;
		}
	}

	// Si DVD inconnu, le mettre en premier
	for (int i=MAX_DVD_POSITION-1; i>0; i--)
		memcpy (&DvdPosition[i], &DvdPosition[i-1], sizeof(DVD_POSITION));
	DvdPosition[0].llDVDGuid	= llDVDGuid;
	nCurrentDvdPosition			= 0;
	return true;
}

FILE_POSITION* CMPlayerCApp::Settings::CurrentFilePosition()
{
	if (nCurrentFilePosition != -1)
		return &FilePosition[nCurrentFilePosition];
	else
		return NULL;
}

bool CMPlayerCApp::Settings::NewFile(LPCTSTR strFileName)
{
	int			i;

	// Recherche si la position du fichier est connue
	for (i=0; i<MAX_FILE_POSITION; i++)
	{
		if (FilePosition[i].strFile == strFileName)
		{
			nCurrentFilePosition = i;
			return false;
		}
	}

	// Si fichier inconnu, le mettre en premier
	for (int i=MAX_FILE_POSITION-1; i>0; i--)
	{
		FilePosition[i].strFile		= FilePosition[i-1].strFile;
		FilePosition[i].llPosition	= FilePosition[i-1].llPosition;
	}
	FilePosition[0].strFile		= strFileName;
	FilePosition[0].llPosition	= 0;
	nCurrentFilePosition		= 0;
	return true;
}


void CMPlayerCApp::Settings::DeserializeHex (LPCTSTR strVal, BYTE* pBuffer, int nBufSize)
{
	long		lRes;

	for (int i=0; i<nBufSize; i++)
	{
		_stscanf_s (strVal+(i*2), _T("%02x"), &lRes);
		pBuffer[i] = (BYTE)lRes;
	}
}

CString CMPlayerCApp::Settings::SerializeHex (BYTE* pBuffer, int nBufSize)
{
	CString		strTemp;
	CString		strResult;

	for (int i=0; i<nBufSize; i++)
	{
		strTemp.Format (_T("%02x"), pBuffer[i]);
		strResult += strTemp;
	}

	return strResult;
}

void CMPlayerCApp::Settings::UpdateData(bool fSave)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp);

	UINT len;
	BYTE* ptr = NULL;

	if(fSave)
	{
		if(!fInitialized) return;

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, fHideCaptionMenu);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, fHideNavigation);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, nCS);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, iDefaultVideoSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, fKeepAspectRatio);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, fCompMonDeskARDiff);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, nVolume);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, nBalance);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, fMute);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, nLoops);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, fLoopForever);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, fRewind);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, iZoomLevel);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, fAllowMultipleInst);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, iTitleBarTextStyle);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, fTitleBarTextTitle);		
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, iOnTop);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, fTrayIcon);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, fRememberZoomLevel);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, fShowBarsWhenFullScreen);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, nShowBarsWhenFullScreenTimeOut);
		pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, (BYTE*)&AutoChangeFullscrRes, sizeof(AutoChangeFullscrRes));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, fExitFullScreenAtTheEnd);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("RestoreResAfterExit"), fRestoreResAfterExit);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, fRememberWindowPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, fRememberWindowSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, fSnapToDesktopEdges);		
		pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, (BYTE*)&rcLastWindowPos, sizeof(rcLastWindowPos));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, lastWindowType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, AspectRatio.cx);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, AspectRatio.cy);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, fKeepHistory);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, iDSVideoRendererType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, iRMVideoRendererType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, iQTVideoRendererType);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, iAPSurfaceUsage);
//		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMRSYNCFIX, fVMRSyncFix);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, iDX9Resizer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, fVMR9MixerMode);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERYUV, fVMR9MixerYUV);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), m_RenderSettings.fVMR9AlterativeVSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), m_RenderSettings.iVMR9VSyncOffset);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), m_RenderSettings.iVMR9VSyncAccurate);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), m_RenderSettings.iVMR9FullscreenGUISupport);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), m_RenderSettings.iVMR9VSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), m_RenderSettings.iVMRDisableDesktopComposition);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), m_RenderSettings.iEVROutputRange);		
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), m_RenderSettings.iEVRHighColorResolution);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), m_RenderSettings.iEVREnableFrameTimeCorrection);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), m_RenderSettings.iVMRFlushGPUBeforeVSync);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), m_RenderSettings.iVMRFlushGPUAfterPresent);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), m_RenderSettings.iVMRFlushGPUWait);

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), m_RenderSettings.bSynchronizeVideo);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), m_RenderSettings.bSynchronizeDisplay);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), m_RenderSettings.bSynchronizeNearest);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LineDelta"), m_RenderSettings.iLineDelta);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), m_RenderSettings.iColumnDelta);

		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE)&(m_RenderSettings.fCycleDelta), sizeof(m_RenderSettings.fCycleDelta));
		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE)&(m_RenderSettings.fTargetSyncOffset), sizeof(m_RenderSettings.fTargetSyncOffset));
		pApp->WriteProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE)&(m_RenderSettings.fControlLimit), sizeof(m_RenderSettings.fControlLimit));

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), fResetDevice);
		
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, CString(AudioRendererDisplayName));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, fAutoloadAudio);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, fAutoloadSubtitles);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, CString(m_subtitlesLanguageOrder));
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, CString(m_audiosLanguageOrder));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, fBlockVSFilter);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, fEnableWorkerThreadForOpening);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, fReportFailedPins);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, sDVDPath);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, fUseDVDPath);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, idMenuLang);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, idAudioLang);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, idSubtitlesLang);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOSPEAKERCONF, fAutoSpeakerConf);
		CString style;
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, style <<= subdefstyle);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, fOverridePlacement);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, nHorPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, nVerPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, nSPCSize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, nSPCMaxRes);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, nSubDelayInterval);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, fSPCPow2Tex);
		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), fSPCAllowAnimationWhenBuffering);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, fEnableSubtitles);		
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, fUseDefaultSubtitlesStyle);		
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, fEnableAudioSwitcher);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, fAudioTimeShift);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, tAudioTimeShift);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, fDownSampleTo441);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, fCustomChannelMapping);
		pApp->WriteProfileBinary(IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, (BYTE*)pSpeakerToChannelMap, sizeof(pSpeakerToChannelMap));
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, fAudioNormalize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, fAudioNormalizeRecover);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, (int)AudioBoost);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, fnChannels);

		// Multi-monitor code
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, CString(f_hmonitor));
		// Prevent Minimize when in Fullscreen mode on non default monitor
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, m_fPreventMinimize);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, m_fUseWin7TaskBar);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, m_fExitAfterPlayback);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, m_fNextInDirAfterPlayback);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, m_fDontUseSearchInFolder);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, nOSD_Size);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, m_OSD_Font);

		// Associated types with icon or not...
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, fAssociatedWithIcons);
		// Last Open Dir
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, f_lastOpenDir);


		// CASIMIR666 : nouveau settings
		CString		strTemp;
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, fD3DFullscreen);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, fMonitorAutoRefreshRate);

		strTemp.Format (_T("%f"), dBrightness);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS, strTemp);
		strTemp.Format (_T("%f"), dContrast);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST, strTemp);
		strTemp.Format (_T("%f"), dHue);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_HUE, strTemp);
		strTemp.Format (_T("%f"), dSaturation);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION, strTemp);
		
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, strShaderList);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, strShaderListScreenSpace);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, (int)m_bToggleShader);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, (int)m_bToggleShaderScreenSpace);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, iEvrBuffers);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEOSD, (int)fDisableOSD);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, (int)fEnableEDLEditor);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, (int)iLanguage);

		// Save analog capture settings
		pApp->WriteProfileInt   (IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, iDefaultCaptureDevice);
		pApp->WriteProfileString(IDS_RS_CAPTURE, IDS_RS_VIDEO_DISP_NAME, strAnalogVideo);
		pApp->WriteProfileString(IDS_RS_CAPTURE, IDS_RS_AUDIO_DISP_NAME, strAnalogAudio);
		pApp->WriteProfileInt   (IDS_RS_CAPTURE, IDS_RS_COUNTRY,		 iAnalogCountry);

		// Save digital capture settings (BDA)
		pApp->WriteProfileString(IDS_RS_DVB, IDS_RS_BDA_NETWORKPROVIDER, BDANetworkProvider);
		pApp->WriteProfileString(IDS_RS_DVB, IDS_RS_BDA_TUNER, BDATuner);
		pApp->WriteProfileString(IDS_RS_DVB, IDS_RS_BDA_RECEIVER, BDAReceiver);
		pApp->WriteProfileInt(IDS_RS_DVB, IDS_RS_DVB_LAST_CHANNEL, DVBLastChannel);

		int			iChannel = 0;
		POSITION	pos = DVBChannels.GetHeadPosition();
		while (pos)
		{
			CString			strTemp;
			CString			strChannel;
			CDVBChannel&	Channel = DVBChannels.GetNext(pos);
			strTemp.Format(_T("%d"), iChannel);
			pApp->WriteProfileString(IDS_RS_DVB, strTemp, Channel.ToString());
			iChannel++;
		}


		// Position de lecture des derniers DVD's
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, (int)fRememberDVDPos);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, (int)fRememberFilePos);
		if (fKeepHistory)
		{
			for (int i=0; i<MAX_DVD_POSITION; i++)
			{
				CString		strDVDPos;
				CString		strValue;

				strDVDPos.Format (_T("DVD Position %d"), i);
				strValue = SerializeHex((BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
				pApp->WriteProfileString(IDS_R_SETTINGS, strDVDPos, strValue);
			}

			// Position de lecture des derniers fichiers
			for (int i=0; i<MAX_FILE_POSITION; i++)
			{
				CString		strFilePos;
				CString		strValue;

				strFilePos.Format (_T("File Name %d"), i);
				pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, FilePosition[i].strFile);
				strFilePos.Format (_T("File Position %d"), i);
				strValue.Format (_T("%I64d"), FilePosition[i].llPosition);
				pApp->WriteProfileString(IDS_R_SETTINGS, strFilePos, strValue);
			}
		}

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, (int)fLastFullScreen);
		// CASIMIR666 : fin nouveaux settings

		{
			for(int i = 0; ; i++)
			{
				CString key;
				key.Format(_T("%s\\%04d"), IDS_R_FILTERS, i);
				int j = pApp->GetProfileInt(key, _T("Enabled"), -1); 
				pApp->WriteProfileString(key, NULL, NULL);
				if(j < 0) break;
			}
			pApp->WriteProfileString(IDS_R_FILTERS, NULL, NULL);

			POSITION pos = filters.GetHeadPosition();
			for(int i = 0; pos; i++)
			{
				FilterOverride* f = filters.GetNext(pos);

				if(f->fTemporary)
					continue;

				CString key;
				key.Format(_T("%s\\%04d"), IDS_R_FILTERS, i);

				pApp->WriteProfileInt(key, _T("SourceType"), (int)f->type);
				pApp->WriteProfileInt(key, _T("Enabled"), (int)!f->fDisabled);
				if(f->type == FilterOverride::REGISTERED)
				{
					pApp->WriteProfileString(key, _T("DisplayName"), CString(f->dispname));
					pApp->WriteProfileString(key, _T("Name"), f->name);
				}
				else if(f->type == FilterOverride::EXTERNAL)
				{
					pApp->WriteProfileString(key, _T("Path"), f->path);
					pApp->WriteProfileString(key, _T("Name"), f->name);
					pApp->WriteProfileString(key, _T("CLSID"), CStringFromGUID(f->clsid));
				}
				POSITION pos2 = f->backup.GetHeadPosition();
				for(int i = 0; pos2; i++)
				{
					CString val;
					val.Format(_T("org%04d"), i);
					pApp->WriteProfileString(key, val, CStringFromGUID(f->backup.GetNext(pos2)));
				}
				pos2 = f->guids.GetHeadPosition();
				for(int i = 0; pos2; i++)
				{
					CString val;
					val.Format(_T("mod%04d"), i);
					pApp->WriteProfileString(key, val, CStringFromGUID(f->guids.GetNext(pos2)));
				}
				pApp->WriteProfileInt(key, _T("LoadType"), f->iLoadType);
				pApp->WriteProfileInt(key, _T("Merit"), f->dwMerit);
			}
		}

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, fIntRealMedia);
		// pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, fRealMediaRenderless);
		// pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, iQuickTimeRenderer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&RealMediaQuickTimeFPS));

		pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, NULL, NULL);
		for(int i = 0, j = m_pnspresets.GetCount(); i < j; i++)
		{
			CString str;
			str.Format(_T("Preset%d"), i);
			pApp->WriteProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, m_pnspresets[i]);
		}

		pApp->WriteProfileString(IDS_R_COMMANDS, NULL, NULL);
		pos = wmcmds.GetHeadPosition();
		for(int i = 0; pos; )
		{
			wmcmd& wc = wmcmds.GetNext(pos);
			if(wc.IsModified())
			{
				CString str;
				str.Format(_T("CommandMod%d"), i);
				CString str2;
				str2.Format(_T("%d %x %x %s %d %d %d"), 
					wc.cmd, wc.fVirt, wc.key, 
					_T("\"") + CString(wc.rmcmd) +  _T("\""), wc.rmrepcnt,
					wc.mouse, wc.appcmd);
				pApp->WriteProfileString(IDS_R_COMMANDS, str, str2);
				i++;
			}
		}

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, fWinLirc);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, WinLircAddr);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, fUIce);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, UIceAddr);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, fGlobalMedia);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEXPTOOLBARS, fDisableXPToolbars);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_USEWMASFREADER, fUseWMASFReader);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, nJumpDistS);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, nJumpDistM);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, nJumpDistL);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, fLimitWindowProportions);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYMSN, fNotifyMSN);		
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYGTSDLL, fNotifyGTSdll);

		Formats.UpdateData(true);

		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_SRCFILTERS, SrcFilters|~(SRC_LAST-1));
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_TRAFILTERS, TraFilters|~(TRA_LAST-1));
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_DXVAFILTERS, DXVAFilters|~(DXVA_LAST-1));
		pApp->WriteProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_FFMPEGFILTERS, FFmpegFilters|~(FFM_LAST-1));
		
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, logofn);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, logoid);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, logoext);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, fHideCDROMsSubMenu);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, priority);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, launchfullscreen);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, fEnableWebServer);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, nWebServerPort);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, fWebServerPrintDebugInfo);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, fWebServerUseCompression);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, fWebServerLocalhostOnly);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, WebRoot);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, WebDefIndex);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, WebServerCGI);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, SnapShotPath);
		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, SnapShotExt);

		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, ThumbRows);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, ThumbCols);
		pApp->WriteProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, ThumbWidth);		

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, ISDb);

		pApp->WriteProfileString(IDS_R_SETTINGS, IDS_D3D9RENDERDEVICE, D3D9RenderDevice);

		pApp->WriteProfileString(_T("Shaders"), NULL, NULL);
		pApp->WriteProfileInt(_T("Shaders"), _T("Initialized"), 1);
		pApp->WriteProfileString(_T("Shaders"), _T("Combine"), m_shadercombine);
		pApp->WriteProfileString(_T("Shaders"), _T("CombineScreenSpace"), m_shadercombineScreenSpace);
		

		pos = m_shaders.GetHeadPosition();
		for(int i = 0; pos; i++)
		{
			const Shader& s = m_shaders.GetNext(pos);

			if(!s.label.IsEmpty())
			{
				CString index;
				index.Format(_T("%d"), i);
				CString srcdata = s.srcdata;
				srcdata.Replace(_T("\r"), _T(""));
				srcdata.Replace(_T("\n"), _T("\\n"));
				srcdata.Replace(_T("\t"), _T("\\t"));
				AfxGetApp()->WriteProfileString(_T("Shaders"), index, s.label + _T("|") + s.target + _T("|") + srcdata);
			}
		}

		if(pApp->m_pszRegistryKey)
		{
			// WINBUG: on win2k this would crash WritePrivateProfileString
			pApp->WriteProfileInt(_T(""), _T(""), pApp->GetProfileInt(_T(""), _T(""), 0)?0:1);
		}
	}
	else
	{
		if(fInitialized) return;

		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		GetVersionEx(&vi);
		fXpOrBetter = (vi.dwMajorVersion >= 5 && vi.dwMinorVersion >= 1 || vi.dwMajorVersion >= 6);

		iDXVer = 0;
		CRegKey dxver;
		if(ERROR_SUCCESS == dxver.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DirectX"), KEY_READ))
		{
			CString str;
			ULONG len = 64;
			if(ERROR_SUCCESS == dxver.QueryStringValue(_T("Version"), str.GetBuffer(len), &len))
			{
				str.ReleaseBuffer(len);
				int ver[4];
				_stscanf_s(str, _T("%d.%d.%d.%d"), ver+0, ver+1, ver+2, ver+3);
				iDXVer = ver[1];
			}
		}

		// Set interface language first!
		iLanguage  = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LANGUAGE, 0);
		if (iLanguage != 0) SetLanguage(iLanguage);
		CreateCommands();

		fHideCaptionMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECAPTIONMENU, 0);
		fHideNavigation = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDENAVIGATION, 0);
		nCS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CONTROLSTATE, CS_SEEKBAR|CS_TOOLBAR|CS_STATUSBAR);
		iDefaultVideoSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULTVIDEOFRAME, DVS_FROMINSIDE);
		fKeepAspectRatio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPASPECTRATIO, TRUE);
		fCompMonDeskARDiff = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_COMPMONDESKARDIFF, FALSE);
		nVolume = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VOLUME, 100);
		nBalance = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BALANCE, 0);
		fMute = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MUTE, 0);
		nLoops = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOPNUM, 1);
		fLoopForever = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOOP, 0);
		fRewind = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REWIND, FALSE);
		iZoomLevel = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ZOOM, 1);
		iDSVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DSVIDEORENDERERTYPE, (IsVistaOrAbove() ? (HasEVR() ? VIDRNDT_DS_EVR_CUSTOM : VIDRNDT_DS_DEFAULT) : VIDRNDT_DS_OVERLAYMIXER) );
		iRMVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_RMVIDEORENDERERTYPE, VIDRNDT_RM_DEFAULT);
		iQTVideoRendererType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QTVIDEORENDERERTYPE, VIDRNDT_QT_DEFAULT);
		iAPSurfaceUsage = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_APSURACEFUSAGE, (IsVistaOrAbove() ? VIDRNDT_AP_TEXTURE3D : VIDRNDT_AP_TEXTURE2D));
//		fVMRSyncFix = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMRSYNCFIX, FALSE);
		iDX9Resizer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DX9_RESIZER, 1);
		fVMR9MixerMode = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERMODE, TRUE);
		fVMR9MixerYUV = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_VMR9MIXERYUV, FALSE);
		CRendererSettingsEVR DefaultSettings;
		m_RenderSettings.fVMR9AlterativeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRAlternateVSync"), DefaultSettings.fVMR9AlterativeVSync);
		m_RenderSettings.iVMR9VSyncOffset = pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncOffset"), DefaultSettings.iVMR9VSyncOffset);
		m_RenderSettings.iVMR9VSyncAccurate = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSyncAccurate2"), DefaultSettings.iVMR9VSyncAccurate);
		m_RenderSettings.iVMR9FullscreenGUISupport = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFullscreenGUISupport"), DefaultSettings.iVMR9FullscreenGUISupport);
		m_RenderSettings.iEVRHighColorResolution = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVRHighColorRes"), DefaultSettings.iEVRHighColorResolution);
		m_RenderSettings.iEVREnableFrameTimeCorrection = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVREnableFrameTimeCorrection"), DefaultSettings.iEVREnableFrameTimeCorrection);
		m_RenderSettings.iVMR9VSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRVSync"), DefaultSettings.iVMR9VSync);
		m_RenderSettings.iVMRDisableDesktopComposition = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRDisableDesktopComposition"), DefaultSettings.iVMRDisableDesktopComposition);
		
		m_RenderSettings.iEVROutputRange = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("EVROutputRange"), DefaultSettings.iEVROutputRange);

		m_RenderSettings.iVMRFlushGPUBeforeVSync = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUBeforeVSync"), DefaultSettings.iVMRFlushGPUBeforeVSync);
		m_RenderSettings.iVMRFlushGPUAfterPresent = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUAfterPresent"), DefaultSettings.iVMRFlushGPUAfterPresent);
		m_RenderSettings.iVMRFlushGPUWait = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("VMRFlushGPUWait"), DefaultSettings.iVMRFlushGPUWait);

		m_RenderSettings.bSynchronizeVideo = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeClock"), DefaultSettings.bSynchronizeVideo);
		m_RenderSettings.bSynchronizeDisplay = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeDisplay"), DefaultSettings.bSynchronizeDisplay);
		m_RenderSettings.bSynchronizeNearest = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SynchronizeNearest"), DefaultSettings.bSynchronizeNearest);
		m_RenderSettings.iLineDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("LineDelta"), DefaultSettings.iLineDelta);
		m_RenderSettings.iColumnDelta = pApp->GetProfileInt(IDS_R_SETTINGS, _T("ColumnDelta"), DefaultSettings.iColumnDelta);

		double *dPtr;
		UINT dSize;
		if(pApp->GetProfileBinary(IDS_R_SETTINGS, _T("CycleDelta"), (LPBYTE*)&dPtr, &dSize))
		{
			m_RenderSettings.fCycleDelta = *dPtr;
			delete [] dPtr;
		}

		if(pApp->GetProfileBinary(IDS_R_SETTINGS, _T("TargetSyncOffset"), (LPBYTE*)&dPtr, &dSize))
		{
			m_RenderSettings.fTargetSyncOffset = *dPtr;
			delete [] dPtr;
		}
		if(pApp->GetProfileBinary(IDS_R_SETTINGS, _T("ControlLimit"), (LPBYTE*)&dPtr, &dSize))
		{
			m_RenderSettings.fControlLimit = *dPtr;
			delete [] dPtr;
		}

		fResetDevice = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("ResetDevice"), TRUE);

		AudioRendererDisplayName = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIORENDERERTYPE, _T(""));
		fAutoloadAudio = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADAUDIO, TRUE);
		fAutoloadSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOLOADSUBTITLES, !IsVSFilterInstalled() || (IsVistaOrAbove() && HasEVR()) );
		m_subtitlesLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANGORDER, _T(""));
		m_audiosLanguageOrder = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_AUDIOSLANGORDER, _T(""));
		fBlockVSFilter = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_BLOCKVSFILTER, TRUE);
		fEnableWorkerThreadForOpening = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWORKERTHREADFOROPENING, TRUE);
		fReportFailedPins = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REPORTFAILEDPINS, TRUE);
		fAllowMultipleInst = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MULTIINST, 0);
		iTitleBarTextStyle = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTSTYLE, 1);
		fTitleBarTextTitle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TITLEBARTEXTTITLE, FALSE);
		iOnTop = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ONTOP, 0);
		fTrayIcon = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TRAYICON, 0);
		fRememberZoomLevel = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOZOOM, 1);
		fShowBarsWhenFullScreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLS, 1);
		nShowBarsWhenFullScreenTimeOut = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FULLSCREENCTRLSTIMEOUT, 0);
		
		//Multi-monitor code
		f_hmonitor = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_FULLSCREENMONITOR, _T(""));
		// Prevent Minimize when in Fullscreen mode on non default monitor
		m_fPreventMinimize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_PREVENT_MINIMIZE, 0);
		m_fUseWin7TaskBar = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_WIN7TASKBAR, 1);
		m_fExitAfterPlayback = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_EXIT_AFTER_PB, 0);
		m_fNextInDirAfterPlayback = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NEXT_AFTER_PB, 0);
		m_fDontUseSearchInFolder   = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_NO_SEARCH_IN_FOLDER, 0);
		nOSD_Size = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MPC_OSD_SIZE, 20);
		m_OSD_Font= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_MPC_OSD_FONT, _T("Arial"));

		// Associated types with icon or not...
		fAssociatedWithIcons = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASSOCIATED_WITH_ICON, 1);
		// Last Open Dir
		f_lastOpenDir = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LAST_OPEN_DIR, _T("C:\\"));

		if ( pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_FULLSCREENRES, &ptr, &len) )
		{
			if ( len == sizeof(AChFR) )
				memcpy( &AutoChangeFullscrRes, ptr, sizeof(AChFR) );
			else
				AutoChangeFullscrRes.bEnabled = false;
			delete [] ptr;
		}
		else
		{
			AutoChangeFullscrRes.bEnabled = false;
		}

		fExitFullScreenAtTheEnd = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EXITFULLSCREENATTHEEND, 1);
		fRestoreResAfterExit = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("RestoreResAfterExit"), 1);
		fRememberWindowPos = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWPOS, 0);
		fRememberWindowSize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REMEMBERWINDOWSIZE, 0);
		fSnapToDesktopEdges = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SNAPTODESKTOPEDGES, 0);
		AspectRatio.cx = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_X, 0);
		AspectRatio.cy = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ASPECTRATIO_Y, 0);
		fKeepHistory = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_KEEPHISTORY, 1);
		if ( pApp->GetProfileBinary(IDS_R_SETTINGS, IDS_RS_LASTWINDOWRECT, &ptr, &len) )
		{
			if ( len == sizeof(CRect) )
				memcpy( &rcLastWindowPos, ptr, sizeof(CRect) );
			else
				fRememberWindowPos = false;
			delete [] ptr;
		}
		else
		{
			fRememberWindowPos = false;
		}
		lastWindowType = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTWINDOWTYPE, SIZE_RESTORED);
		sDVDPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_DVDPATH, _T(""));
		fUseDVDPath = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDVDPATH, 0);
		idMenuLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MENULANG, ::GetUserDefaultLCID());
		idAudioLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOLANG, ::GetUserDefaultLCID());
		idSubtitlesLang = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBTITLESLANG, ::GetUserDefaultLCID());
		fAutoSpeakerConf = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUTOSPEAKERCONF, 1);
		// TODO: rename subdefstyle -> defStyle, IDS_RS_SPLOGFONT -> IDS_RS_SPSTYLE
		subdefstyle <<= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SPLOGFONT, _T(""));
		fOverridePlacement = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPOVERRIDEPLACEMENT, 0);
		nHorPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPHORPOS, 50);
		nVerPos = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPVERPOS, 90);
		nSPCSize = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCSIZE, 3);
		nSPCMaxRes = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPCMAXRES, 2);
		nSubDelayInterval = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SUBDELAYINTERVAL, 500);
		fSPCPow2Tex = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_POW2TEX, TRUE);
		
		bool bAllowAnimationWhenBuffering = true;
		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);
		if (SysInfo.dwNumberOfProcessors < 3)
			bAllowAnimationWhenBuffering = false;
		

		fSPCAllowAnimationWhenBuffering = !!pApp->GetProfileInt(IDS_R_SETTINGS, _T("SPCAllowAnimationWhenBuffering"), bAllowAnimationWhenBuffering);
		fEnableSubtitles = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLESUBTITLES, TRUE);
		fUseDefaultSubtitlesStyle = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEDEFAULTSUBTITLESSTYLE, FALSE);
		fEnableAudioSwitcher = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOSWITCHER, TRUE);
		fAudioTimeShift = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEAUDIOTIMESHIFT, 0);
		tAudioTimeShift = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOTIMESHIFT, 0);
		fDownSampleTo441 = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DOWNSAMPLETO441, 0);
		fCustomChannelMapping = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_CUSTOMCHANNELMAPPING, 0);
		
		BOOL bResult = pApp->GetProfileBinary( IDS_R_SETTINGS, IDS_RS_SPEAKERTOCHANNELMAPPING, &ptr, &len );
		if ( bResult && len == sizeof(pSpeakerToChannelMap) )
		{
			memcpy( pSpeakerToChannelMap, ptr, sizeof(pSpeakerToChannelMap) );
		}
		else
		{
			memset(pSpeakerToChannelMap, 0, sizeof(pSpeakerToChannelMap));
			for(int j = 0; j < 18; j++)
				for(int i = 0; i <= j; i++)
					pSpeakerToChannelMap[j][i] = 1<<i;

			pSpeakerToChannelMap[0][0] = 1<<0;
			pSpeakerToChannelMap[0][1] = 1<<0;

			pSpeakerToChannelMap[3][0] = 1<<0;
			pSpeakerToChannelMap[3][1] = 1<<1;
			pSpeakerToChannelMap[3][2] = 0;
			pSpeakerToChannelMap[3][3] = 0;
			pSpeakerToChannelMap[3][4] = 1<<2;
			pSpeakerToChannelMap[3][5] = 1<<3;

			pSpeakerToChannelMap[4][0] = 1<<0;
			pSpeakerToChannelMap[4][1] = 1<<1;
			pSpeakerToChannelMap[4][2] = 1<<2;
			pSpeakerToChannelMap[4][3] = 0;
			pSpeakerToChannelMap[4][4] = 1<<3;
			pSpeakerToChannelMap[4][5] = 1<<4;
		}
		if ( bResult )
			delete [] ptr;

		fAudioNormalize = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZE, FALSE);
		fAudioNormalizeRecover = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIONORMALIZERECOVER, TRUE);
		AudioBoost = (float)pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_AUDIOBOOST, 1);
		fnChannels = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_SPEAKERCHANNELS, 2);

		{
			for(int i = 0; ; i++)
			{
				CString key;
				key.Format(_T("%s\\%04d"), IDS_R_FILTERS, i);

				CAutoPtr<FilterOverride> f(DNew FilterOverride);

				f->fDisabled = !pApp->GetProfileInt(key, _T("Enabled"), 0);

				UINT j = pApp->GetProfileInt(key, _T("SourceType"), -1);
				if(j == 0)
				{
					f->type = FilterOverride::REGISTERED;
					f->dispname = CStringW(pApp->GetProfileString(key, _T("DisplayName"), _T("")));
					f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
				}
				else if(j == 1)
				{
					f->type = FilterOverride::EXTERNAL;
					f->path = pApp->GetProfileString(key, _T("Path"), _T(""));
					f->name = pApp->GetProfileString(key, _T("Name"), _T(""));
					f->clsid = GUIDFromCString(pApp->GetProfileString(key, _T("CLSID"), _T("")));
				}
				else
				{
					pApp->WriteProfileString(key, NULL, 0);
					break;
				}

				f->backup.RemoveAll();
				for(int i = 0; ; i++)
				{
					CString val;
					val.Format(_T("org%04d"), i);
					CString guid = pApp->GetProfileString(key, val, _T(""));
					if(guid.IsEmpty()) break;
					f->backup.AddTail(GUIDFromCString(guid));
				}

				f->guids.RemoveAll();
				for(int i = 0; ; i++)
				{
					CString val;
					val.Format(_T("mod%04d"), i);
					CString guid = pApp->GetProfileString(key, val, _T(""));
					if(guid.IsEmpty()) break;
					f->guids.AddTail(GUIDFromCString(guid));
				}

				f->iLoadType = (int)pApp->GetProfileInt(key, _T("LoadType"), -1);
				if(f->iLoadType < 0) break;

				f->dwMerit = pApp->GetProfileInt(key, _T("Merit"), MERIT_DO_NOT_USE+1);

				filters.AddTail(f);
			}
		}

		fIntRealMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_INTREALMEDIA, 0);
		//fRealMediaRenderless = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIARENDERLESS, 0);
		//iQuickTimeRenderer = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_QUICKTIMERENDERER, 2);
		RealMediaQuickTimeFPS = 25.0;
		*((DWORD*)&RealMediaQuickTimeFPS) = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_REALMEDIAFPS, *((DWORD*)&RealMediaQuickTimeFPS));

		m_pnspresets.RemoveAll();
		for(int i = 0; i < (ID_PANNSCAN_PRESETS_END - ID_PANNSCAN_PRESETS_START); i++)
		{
			CString str;
			str.Format(_T("Preset%d"), i);
			str = pApp->GetProfileString(IDS_R_SETTINGS _T("\\") IDS_RS_PNSPRESETS, str, _T(""));
			if(str.IsEmpty()) break;
			m_pnspresets.Add(str);
		}
		if(m_pnspresets.IsEmpty())
		{
			double _4p3 = 4.0/3.0;
			double _16p9 = 16.0/9.0;
			double _185p1 = 1.85/1.0;
			double _235p1 = 2.35/1.0;

			CString str;
			str.Format(ResStr(IDS_SCALE_16_9), 0.5, 0.5, _4p3/_4p3, _16p9/_4p3);
			m_pnspresets.Add(str);
			str.Format(ResStr(IDS_SCALE_WIDESCREEN), 0.5, 0.5, _16p9/_4p3, _16p9/_4p3);
			m_pnspresets.Add(str);
			str.Format(ResStr(IDS_SCALE_ULTRAWIDE), 0.5, 0.5, _235p1/_4p3, _235p1/_4p3);
			m_pnspresets.Add(str);
		}

		for(int i = 0; i < wmcmds.GetCount(); i++)
		{
			CString str;
			str.Format(_T("CommandMod%d"), i);
			str = pApp->GetProfileString(IDS_R_COMMANDS, str, _T(""));
			if(str.IsEmpty()) break;
			int cmd, fVirt, key, repcnt, mouse, appcmd;
			TCHAR buff[128];
			int n;
			if(5 > (n = _stscanf_s(str, _T("%d %x %x %s %d %d %d"), &cmd, &fVirt, &key, buff, countof(buff), &repcnt, &mouse, &appcmd)))
				break;
			if(POSITION pos = wmcmds.Find(cmd))
			{
				wmcmd& wc = wmcmds.GetAt(pos);
				wc.cmd = cmd;
				wc.fVirt = fVirt;
				wc.key = key;
				if(n >= 6) wc.mouse = (UINT)mouse;
				if(n >= 7) wc.appcmd = (UINT)appcmd;
				wc.rmcmd = CStringA(buff).Trim('\"');
				wc.rmrepcnt = repcnt;
			}
		}

		CAtlArray<ACCEL> pAccel;
		pAccel.SetCount(wmcmds.GetCount());
		POSITION pos = wmcmds.GetHeadPosition();
		for(int i = 0; pos; i++) pAccel[i] = wmcmds.GetNext(pos);
		hAccel = CreateAcceleratorTable(pAccel.GetData(), pAccel.GetCount());

		WinLircAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WINLIRCADDR, _T("127.0.0.1:8765"));
		fWinLirc = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WINLIRC, 0);
		UIceAddr = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_UICEADDR, _T("127.0.0.1:1234"));
		fUIce = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_UICE, 0);
		fGlobalMedia = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_GLOBALMEDIA, 0);

		fDisableXPToolbars = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEXPTOOLBARS, 0);
		fUseWMASFReader = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_USEWMASFREADER, FALSE);
		nJumpDistS = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTS, 1000);
		nJumpDistM = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTM, 5000);
		nJumpDistL = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_JUMPDISTL, 20000);
		fLimitWindowProportions = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LIMITWINDOWPROPORTIONS, FALSE);
		fNotifyMSN = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYMSN, FALSE);
		fNotifyGTSdll = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_NOTIFYGTSDLL, FALSE);

		Formats.UpdateData(false);

		SrcFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_SRCFILTERS, ~0);
		TraFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_TRAFILTERS, ~0);
		DXVAFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_DXVAFILTERS, ~0);
		FFmpegFilters = pApp->GetProfileInt(IDS_R_INTERNAL_FILTERS, IDS_RS_FFMPEGFILTERS, ~0);

		logofn = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_LOGOFILE, _T(""));
		logoid = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOID, IDF_LOGO4);
		logoext = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LOGOEXT, 0);

		fHideCDROMsSubMenu = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_HIDECDROMSSUBMENU, 0);		

		priority = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_PRIORITY, NORMAL_PRIORITY_CLASS);
		::SetPriorityClass(::GetCurrentProcess(), priority);
		launchfullscreen = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LAUNCHFULLSCREEN, FALSE);

		fEnableWebServer = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEWEBSERVER, FALSE);
		nWebServerPort = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPORT, 13579);
		fWebServerPrintDebugInfo = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERPRINTDEBUGINFO, FALSE);
		fWebServerUseCompression = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERUSECOMPRESSION, TRUE);
		fWebServerLocalhostOnly = !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_WEBSERVERLOCALHOSTONLY, FALSE);
		WebRoot = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBROOT, _T("*./webroot"));
		WebDefIndex = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBDEFINDEX, _T("index.html;index.php"));
		WebServerCGI = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_WEBSERVERCGI, _T(""));

		CString MyPictures;

		CRegKey key;
		// grrrrr
		// if(!SHGetSpecialFolderPath(NULL, MyPictures.GetBufferSetLength(MAX_PATH), CSIDL_MYPICTURES, TRUE)) MyPictures.Empty();
		// else MyPictures.ReleaseBuffer();
		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ))
		{
			ULONG len = MAX_PATH;
			if(ERROR_SUCCESS == key.QueryStringValue(_T("My Pictures"), MyPictures.GetBuffer(MAX_PATH), &len)) MyPictures.ReleaseBufferSetLength(len);
			else MyPictures.Empty();
		}
		SnapShotPath = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTPATH, MyPictures);
		SnapShotExt = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SNAPSHOTEXT, _T(".jpg"));

		ThumbRows = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBROWS, 4);
		ThumbCols = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBCOLS, 4);
		ThumbWidth = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_THUMBWIDTH, 1024);

		ISDb = pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_ISDB, _T("www.opensubtitles.org/isdb"));

		D3D9RenderDevice = pApp->GetProfileString(IDS_R_SETTINGS, IDS_D3D9RENDERDEVICE, _T(""));

		pApp->WriteProfileInt(IDS_R_SETTINGS, _T("LastUsedPage"), 0);

		//

		m_shaders.RemoveAll();

		CAtlStringMap<UINT> shaders;

		shaders[_T("16-235 -> 0-255  [SD][HD]")] = IDF_SHADER_LEVELS;
		shaders[_T("16-235 -> 0-255  [SD]")] = IDF_SHADER_LEVELS2;
		shaders[_T("0-255 -> 16-235")] = IDF_SHADER_LEVELS3;
		shaders[_T("BT.601 -> BT.709")] = IDF_SHADER_BT601_BT709;
		shaders[_T("contour")] = IDF_SHADER_CONTOUR;
		shaders[_T("deinterlace (blend)")] = IDF_SHADER_DEINTERLACE;
		shaders[_T("edge sharpen")] = IDF_SHADER_EDGE_SHARPEN;
		shaders[_T("emboss")] = IDF_SHADER_EMBOSS;
		shaders[_T("grayscale")] = IDF_SHADER_GRAYSCALE;
		shaders[_T("invert")] = IDF_SHADER_INVERT;
		shaders[_T("letterbox")] = IDF_SHADER_LETTERBOX;
		shaders[_T("nightvision")] = IDF_SHADER_NIGHTVISION;
		shaders[_T("procamp")] = IDF_SHADER_PROCAMP;
		shaders[_T("sharpen")] = IDF_SHADER_SHARPEN;
		shaders[_T("sharpen complex")] = IDF_SHADER_SHARPEN_COMPLEX;
		shaders[_T("sharpen complex 2")] = IDF_SHADER_SHARPEN_COMPLEX2;
		shaders[_T("sphere")] = IDF_SHADER_SPHERE;
		shaders[_T("spotlight")] = IDF_SHADER_SPOTLIGHT;
		shaders[_T("wave")] = IDF_SHADER_WAVE;
		shaders[_T("denoise")] = IDF_SHADER_DENOISE;
		shaders[_T("YV12 Chroma Upsampling")] = IDF_SHADER_YV12CHROMAUP;

		int iShader = 0;

		for(; ; iShader++)
		{
			CString str;
			str.Format(_T("%d"), iShader);
			str = pApp->GetProfileString(_T("Shaders"), str);

			CAtlList<CString> sl;
			CString label = Explode(str, sl, '|');
			if(label.IsEmpty()) break;
			if(sl.GetCount() < 3) continue;

			Shader s;
			s.label = sl.RemoveHead();
			s.target = sl.RemoveHead();
			s.srcdata = sl.RemoveHead();
			s.srcdata.Replace(_T("\\n"), _T("\n"));
			s.srcdata.Replace(_T("\\t"), _T("\t"));
			m_shaders.AddTail(s);

			shaders.RemoveKey(s.label);
		}

		pos = shaders.GetStartPosition();
		for(; pos; iShader++)
		{
			CAtlStringMap<UINT>::CPair* pPair = shaders.GetNext(pos);

			CStringA srcdata;
			if(LoadResource(pPair->m_value, srcdata, _T("FILE")))
			{
				Shader s;
				s.label = pPair->m_key;

				// Select minimum version for each shader!
				switch (pPair->m_value)
				{
				case IDF_SHADER_DENOISE :
					s.target = _T("ps_3_0");
					break;
				case IDF_SHADER_SHARPEN_COMPLEX2 :
					s.target = _T("ps_2_a");
					break;
				default :
					s.target = _T("ps_2_0");
					break;
				}
				s.srcdata = CString(srcdata);
				m_shaders.AddTail(s);
			}
		}

		// CASIMIR666 : nouveaux settings
		fD3DFullscreen			= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_D3DFULLSCREEN, FALSE);
		fMonitorAutoRefreshRate	= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_MONITOR_AUTOREFRESHRATE, FALSE);

		dBrightness		= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_BRIGHTNESS,	_T("1")));
		dContrast		= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_CONTRAST,		_T("1")));
		dHue			= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_HUE,			_T("0")));
		dSaturation		= (float)_tstof(pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_COLOR_SATURATION,	_T("1")));
		strShaderList	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLIST, _T(""));
		strShaderListScreenSpace	= pApp->GetProfileString(IDS_R_SETTINGS, IDS_RS_SHADERLISTSCREENSPACE, _T(""));
		m_bToggleShader = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADER, 0);
		m_bToggleShaderScreenSpace = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_TOGGLESHADERSSCREENSPACE, 0);

		iEvrBuffers		= pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_EVR_BUFFERS, 5);
		fDisableOSD		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DISABLEOSD, 0);
		fEnableEDLEditor= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_ENABLEEDLEDITOR, FALSE);

		// Save analog capture settings
		iDefaultCaptureDevice = pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DEFAULT_CAPTURE, 0);
		strAnalogVideo		= pApp->GetProfileString(IDS_RS_CAPTURE, IDS_RS_VIDEO_DISP_NAME, _T(""));
		strAnalogAudio		= pApp->GetProfileString(IDS_RS_CAPTURE, IDS_RS_AUDIO_DISP_NAME, _T(""));
		iAnalogCountry		= pApp->GetProfileInt(IDS_RS_CAPTURE, IDS_RS_COUNTRY, 1);

		BDANetworkProvider	= pApp->GetProfileString(IDS_RS_DVB, IDS_RS_BDA_NETWORKPROVIDER, _T(""));
		BDATuner			= pApp->GetProfileString(IDS_RS_DVB, IDS_RS_BDA_TUNER, _T(""));
		BDAReceiver			= pApp->GetProfileString(IDS_RS_DVB, IDS_RS_BDA_RECEIVER, _T(""));
		DVBLastChannel		= pApp->GetProfileInt(IDS_RS_DVB, IDS_RS_DVB_LAST_CHANNEL, 1);

		for(int iChannel = 0; ; iChannel++)
		{
			CString		strTemp;
			CString		strChannel;
			CDVBChannel	Channel;
			strTemp.Format(_T("%d"), iChannel);
			strChannel = pApp->GetProfileString(IDS_RS_DVB, strTemp, _T(""));
			if (strChannel.IsEmpty()) break;
			Channel.FromString(strChannel);
			DVBChannels.AddTail (Channel);
		}

		// Position de lecture des derniers DVD's
		fRememberDVDPos		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_DVDPOS, 0);
		nCurrentDvdPosition = -1;
		memset (DvdPosition, 0, sizeof(DvdPosition));
		for (int i=0; i<MAX_DVD_POSITION; i++)
		{
			CString		strDVDPos;
			CString		strValue;

			strDVDPos.Format (_T("DVD Position %d"), i);
			strValue = pApp->GetProfileString(IDS_R_SETTINGS, strDVDPos, _T(""));
			if (strValue.GetLength()/2 == sizeof(DVD_POSITION))
			{
				DeserializeHex(strValue, (BYTE*)&DvdPosition[i], sizeof(DVD_POSITION));
			}
		}

		// Position de lecture des derniers fichiers
		fRememberFilePos		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_FILEPOS, 0);
		nCurrentFilePosition = -1;
		for (int i=0; i<MAX_FILE_POSITION; i++)
		{
			CString		strFilePos;
			CString		strValue;

			strFilePos.Format (_T("File Name %d"), i);
			FilePosition[i].strFile = pApp->GetProfileString(IDS_R_SETTINGS, strFilePos, _T(""));

			strFilePos.Format (_T("File Position %d"), i);
			strValue = pApp->GetProfileString(IDS_R_SETTINGS, strFilePos, _T(""));
			FilePosition[i].llPosition = _tstoi64 (strValue);
		}

		fLastFullScreen		= !!pApp->GetProfileInt(IDS_R_SETTINGS, IDS_RS_LASTFULLSCREEN, 0);

		// CASIMIR666 : fin nouveaux settings


		// TODO: sort shaders by label

		m_shadercombine = pApp->GetProfileString(_T("Shaders"), _T("Combine"), _T(""));
		m_shadercombineScreenSpace = pApp->GetProfileString(_T("Shaders"), _T("CombineScreenSpace"), _T(""));

		if(launchfullscreen) nCLSwitches |= CLSW_FULLSCREEN;

		fInitialized = true;
	}
}

__int64 CMPlayerCApp::Settings::ConvertTimeToMSec(CString& time)
{
	__int64 Sec = 0;
	__int64 mSec = 0;
	__int64 mult = 1;

	int pos = time.GetLength() - 1;
	if (pos < 3) return 0;

	while (pos >= 0) {
		TCHAR ch = time[pos];
		if (ch == '.') {
			mSec = Sec * 1000 / mult;
			Sec = 0;
			mult = 1;
		} else if (ch == ':') {
			mult = mult * 6 / 10;
		} else if (ch >= '0' && ch <= '9') {
			Sec += (ch - '0') * mult;
			mult *= 10;
		} else {
			mSec = Sec = 0;
			break;
		}
		pos--;
	}
	return Sec*1000 + mSec;
}

void CMPlayerCApp::Settings::ExtractDVDStartPos(CString& strParam)
{
	int i = 0, j = 0;
	for(CString token = strParam.Tokenize(_T("#"), i); 
		j < 3 && !token.IsEmpty(); 
		token = strParam.Tokenize(_T("#"), i), j++)
	{
		switch (j)
		{
		case 0 :
			lDVDTitle = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
			break;
		case 1 :
			if (token.Find(':') >0)
				_stscanf_s(token, _T("%02d:%02d:%02d.%03d"), &DVDPosition.bHours, &DVDPosition.bMinutes, &DVDPosition.bSeconds, &DVDPosition.bFrames);
			else
				lDVDChapter = token.IsEmpty() ? 0 : (ULONG)_wtol(token);
			break;
		}
	}
}

void CMPlayerCApp::Settings::ParseCommandLine(CAtlList<CString>& cmdln)
{
	nCLSwitches = 0;
	slFiles.RemoveAll();
	slDubs.RemoveAll();
	slSubs.RemoveAll();
	slFilters.RemoveAll();
	rtStart = 0;
	rtShift = 0;
	lDVDTitle = 0;
	lDVDChapter = 0;
	memset (&DVDPosition, 0, sizeof(DVDPosition));
	iAdminOption=0;
	fixedWindowSize.SetSize(0, 0);
	iMonitor = 0;
	hMasterWnd = 0;
	sPnSPreset.Empty();

	POSITION pos = cmdln.GetHeadPosition();
	while(pos)
	{
		CString param = cmdln.GetNext(pos);
		if(param.IsEmpty()) continue;

		if((param[0] == '-' || param[0] == '/') && param.GetLength() > 1)
		{
			CString sw = param.Mid(1).MakeLower();
			if(sw == _T("open")) nCLSwitches |= CLSW_OPEN;
			else if(sw == _T("play")) nCLSwitches |= CLSW_PLAY;
			else if(sw == _T("fullscreen")) nCLSwitches |= CLSW_FULLSCREEN;
			else if(sw == _T("minimized")) nCLSwitches |= CLSW_MINIMIZED;
			else if(sw == _T("new")) nCLSwitches |= CLSW_NEW;
			else if(sw == _T("help") || sw == _T("h") || sw == _T("?")) nCLSwitches |= CLSW_HELP;
			else if(sw == _T("dub") && pos) slDubs.AddTail(cmdln.GetNext(pos));
			else if(sw == _T("dubdelay") && pos)
			{
				CString		strFile = cmdln.GetNext(pos);
				int			nPos  = strFile.Find (_T("DELAY"));
				if (nPos != -1)
					rtShift = 10000 * _tstol(strFile.Mid(nPos + 6));
				slDubs.AddTail(strFile);
			}
			else if(sw == _T("sub") && pos) slSubs.AddTail(cmdln.GetNext(pos));
			else if(sw == _T("filter") && pos) slFilters.AddTail(cmdln.GetNext(pos));
			else if(sw == _T("dvd")) nCLSwitches |= CLSW_DVD;
			else if(sw == _T("dvdpos")) ExtractDVDStartPos(cmdln.GetNext(pos));
			else if(sw == _T("cd")) nCLSwitches |= CLSW_CD;
			else if(sw == _T("add")) nCLSwitches |= CLSW_ADD;
			else if(sw == _T("regvid")) nCLSwitches |= CLSW_REGEXTVID;
			else if(sw == _T("regaud")) nCLSwitches |= CLSW_REGEXTAUD;
			else if(sw == _T("unregall")) nCLSwitches |= CLSW_UNREGEXT;
			else if(sw == _T("unregvid")) nCLSwitches |= CLSW_UNREGEXT; /* keep for compatibility with old versions */
			else if(sw == _T("unregaud")) nCLSwitches |= CLSW_UNREGEXT; /* keep for compatibility with old versions */
			else if(sw == _T("start") && pos) {rtStart = 10000i64*_tcstol(cmdln.GetNext(pos), NULL, 10); nCLSwitches |= CLSW_STARTVALID;}
			else if(sw == _T("startpos") && pos) {rtStart = 10000i64 * ConvertTimeToMSec(cmdln.GetNext(pos)); nCLSwitches |= CLSW_STARTVALID;}
			else if(sw == _T("nofocus")) nCLSwitches |= CLSW_NOFOCUS;
			else if(sw == _T("close")) nCLSwitches |= CLSW_CLOSE;
			else if(sw == _T("standby")) nCLSwitches |= CLSW_STANDBY;
			else if(sw == _T("hibernate")) nCLSwitches |= CLSW_HIBERNATE;
			else if(sw == _T("shutdown")) nCLSwitches |= CLSW_SHUTDOWN;
			else if(sw == _T("logoff")) nCLSwitches |= CLSW_LOGOFF;
			else if(sw == _T("d3dfs")) nCLSwitches |= CLSW_D3DFULLSCREEN;
			else if(sw == _T("adminoption")) { nCLSwitches |= CLSW_ADMINOPTION; iAdminOption = _ttoi (cmdln.GetNext(pos)); }
			else if(sw == _T("slave"))
			{
				nCLSwitches |= CLSW_SLAVE;
				hMasterWnd = (HWND)_ttol (cmdln.GetNext(pos));
			}
			else if(sw == _T("fixedsize") && pos)
			{
				CAtlList<CString> sl;
				Explode(cmdln.GetNext(pos), sl, ',', 2);
				if(sl.GetCount() == 2)
				{
					fixedWindowSize.SetSize(_ttol(sl.GetHead()), _ttol(sl.GetTail()));
					if(fixedWindowSize.cx > 0 && fixedWindowSize.cy > 0)
						nCLSwitches |= CLSW_FIXEDSIZE;
				}
			}
			else if(sw == _T("monitor") && pos) {iMonitor = _tcstol(cmdln.GetNext(pos), NULL, 10); nCLSwitches |= CLSW_MONITOR;}
			else if(sw == _T("minidump")) { CMiniDump::Enable(); }
			else if(sw == _T("pns")) sPnSPreset = cmdln.GetNext(pos);
			else if(sw == _T("audiorender") && pos) 
			{	
				SetAudioRender(_ttoi(cmdln.GetNext(pos)));
			}
			else nCLSwitches |= CLSW_HELP|CLSW_UNRECOGNIZEDSWITCH;
		}
		else
		{
			slFiles.AddTail(param);
		}
	}
}

void CMPlayerCApp::Settings::GetFav(favtype ft, CAtlList<CString>& sl)
{
	sl.RemoveAll();

	CString root;

	switch(ft)
	{
	case FAV_FILE: root = IDS_R_FAVFILES; break;
	case FAV_DVD: root = IDS_R_FAVDVDS; break;
	case FAV_DEVICE: root = IDS_R_FAVDEVICES; break;
	default: return;
	}

	for(int i = 0; ; i++)
	{
		CString s;
		s.Format(_T("Name%d"), i);
		s = AfxGetApp()->GetProfileString(root, s, NULL);
		if(s.IsEmpty()) break;
		sl.AddTail(s);
	}
}

void CMPlayerCApp::Settings::SetFav(favtype ft, CAtlList<CString>& sl)
{
	CString root;

	switch(ft)
	{
	case FAV_FILE: root = IDS_R_FAVFILES; break;
	case FAV_DVD: root = IDS_R_FAVDVDS; break;
	case FAV_DEVICE: root = IDS_R_FAVDEVICES; break;
	default: return;
	}

	AfxGetApp()->WriteProfileString(root, NULL, NULL);

	int i = 0;
	POSITION pos = sl.GetHeadPosition();
	while(pos)
	{
		CString s;
		s.Format(_T("Name%d"), i++);
		AfxGetApp()->WriteProfileString(root, s, sl.GetNext(pos));
	}
}

void CMPlayerCApp::Settings::AddFav(favtype ft, CString s)
{
	CAtlList<CString> sl;
	GetFav(ft, sl);
	if(sl.Find(s)) return;
	sl.AddTail(s);
	SetFav(ft, sl);
}

CDVBChannel* CMPlayerCApp::Settings::FindChannelByPref(int nPrefNumber)
{
	POSITION	pos = DVBChannels.GetHeadPosition();
	while (pos)
	{
		CDVBChannel&	Channel = DVBChannels.GetNext (pos);
		if (Channel.GetPrefNumber() == nPrefNumber)
		{
			return &Channel;
		}
	}

	return NULL;
}


// CMPlayerCApp::Settings::CRecentFileAndURLList

CMPlayerCApp::Settings::CRecentFileAndURLList::CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
															LPCTSTR lpszEntryFormat, int nSize,	
															int nMaxDispLen) 
	: CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen)	
{
}

//#include <afximpl.h>
extern BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
extern BOOL AFXAPI AfxComparePath(LPCTSTR lpszPath1, LPCTSTR lpszPath2);

void CMPlayerCApp::Settings::CRecentFileAndURLList::Add(LPCTSTR lpszPathName)
{
	ASSERT(m_arrNames != NULL);
	ASSERT(lpszPathName != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	if(CString(lpszPathName).MakeLower().Find(_T("@device:")) >= 0)
		return;

	bool fURL = (CString(lpszPathName).Find(_T("://")) >= 0);

	// fully qualify the path name
	TCHAR szTemp[1024];
	if(fURL) _tcscpy_s(szTemp, lpszPathName);
	else AfxFullPath(szTemp, lpszPathName);

	// update the MRU list, if an existing MRU string matches file name
	int iMRU;
	for (iMRU = 0; iMRU < m_nSize-1; iMRU++)
	{
		if((fURL && !_tcscmp(m_arrNames[iMRU], szTemp))
		|| AfxComparePath(m_arrNames[iMRU], szTemp))
			break;      // iMRU will point to matching entry
	}
	// move MRU strings before this one down
	for (; iMRU > 0; iMRU--)
	{
		ASSERT(iMRU > 0);
		ASSERT(iMRU < m_nSize);
		m_arrNames[iMRU] = m_arrNames[iMRU-1];
	}
	// place this one at the beginning
	m_arrNames[0] = szTemp;
}


void CMPlayerCApp::OnHelpShowcommandlineswitches()
{
	ShowCmdlnSwitches();
}

//
void GetCurDispMode(dispmode& dm, CString& DisplayName)
{  
	HDC hDC;
	CString DisplayName1 = DisplayName;
	if ((DisplayName == _T("Current")) || (DisplayName == _T("")))
	{
		CMonitor monitor;
		CMonitors monitors;
		monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
		monitor.GetName(DisplayName1);
	}
	if(hDC = CreateDC(DisplayName1, NULL, NULL, NULL))
	{
		dm.fValid = true;
		dm.size = CSize(GetDeviceCaps(hDC, HORZRES), GetDeviceCaps(hDC, VERTRES));
		dm.bpp = GetDeviceCaps(hDC, BITSPIXEL);
		dm.freq = GetDeviceCaps(hDC, VREFRESH);
		DeleteDC(hDC);
	}
}    

bool GetDispMode(int i, dispmode& dm, CString& DisplayName)
{
	DEVMODE devmode;
	CString DisplayName1 = DisplayName;
	devmode.dmSize = sizeof(DEVMODE);
	if ((DisplayName == _T("Current")) || (DisplayName == _T("")))
	{ 
		CMonitor monitor;
		CMonitors monitors;
		monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
		monitor.GetName(DisplayName1);
	}
	if(!EnumDisplaySettings(DisplayName1, i, &devmode))
	return(false);
	dm.fValid = true;
	dm.size = CSize(devmode.dmPelsWidth, devmode.dmPelsHeight);
	dm.bpp = devmode.dmBitsPerPel;
	dm.freq = devmode.dmDisplayFrequency;
	dm.dmDisplayFlags = devmode.dmDisplayFlags;
	return(true);
}

void SetDispMode(dispmode& dm, CString& DisplayName)
{
	dispmode dm1;
	GetCurDispMode(dm1, DisplayName);
	if ((dm.size == dm1.size) && (dm.bpp == dm1.bpp) && (dm.freq == dm1.freq)) return; 

	if(!dm.fValid) return;
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = dm.size.cx;
	dmScreenSettings.dmPelsHeight = dm.size.cy;
	dmScreenSettings.dmBitsPerPel = dm.bpp;
	dmScreenSettings.dmDisplayFrequency = dm.freq;
	dmScreenSettings.dmDisplayFlags = dm.dmDisplayFlags;
	dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY  | DM_DISPLAYFLAGS;
	CString DisplayName1 = DisplayName;
	if ((DisplayName == _T("Current")) || (DisplayName == _T("")))
	{ 
		CMonitor monitor;
		CMonitors monitors;
		monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
		monitor.GetName(DisplayName1);
	}
	if(AfxGetAppSettings().fRestoreResAfterExit)
		ChangeDisplaySettingsEx(DisplayName1, &dmScreenSettings, NULL, CDS_FULLSCREEN, NULL);
	else	
		ChangeDisplaySettingsEx(DisplayName1, &dmScreenSettings, NULL, NULL, NULL);
}

void SetAudioRender(int AudioDevNo)
{
	CStringArray m_AudioRendererDisplayNames;
	m_AudioRendererDisplayNames.Add(_T(""));
	int i=2;

	BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker)
	{
		LPOLESTR olestr = NULL;
		if(FAILED(pMoniker->GetDisplayName(0, 0, &olestr)))
			continue;
		CStringW str(olestr);
		CoTaskMemFree(olestr);
		m_AudioRendererDisplayNames.Add(CString(str));
		i++;
	}
	EndEnumSysDev

	m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_COMP);
	m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_UNCOMP);
	m_AudioRendererDisplayNames.Add(AUDRNDT_MPC);
	i+=3;
	if (AudioDevNo>=1 && AudioDevNo<=i) 
	{	
		AfxGetMyApp()->m_AudioRendererDisplayName_CL = m_AudioRendererDisplayNames[AudioDevNo-1];
		AfxGetAppSettings().nCLSwitches |= CLSW_AUDIORENDER;	
	}
}

void SetHandCursor(HWND m_hWnd, UINT nID)
{
	SetClassLongPtr(GetDlgItem(m_hWnd, nID), GCLP_HCURSOR, (long) AfxGetApp()->LoadStandardCursor(IDC_HAND));
}

#include <afxsock.h>
#include <atlsync.h>
#include <atlutil.h> // put this before the first detours macro above to see an ICE with vc71 :)
#include <atlrx.h>

typedef CAtlRegExp<CAtlRECharTraits> CAtlRegExpT;
typedef CAtlREMatchContext<CAtlRECharTraits> CAtlREMatchContextT;

bool FindRedir(CUrl& src, CString ct, CString& body, CAtlList<CString>& urls, CAutoPtrList<CAtlRegExpT>& res)
{
	POSITION pos = res.GetHeadPosition();
	while(pos)
	{
		CAtlRegExpT* re = res.GetNext(pos);

		CAtlREMatchContextT mc;
		const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
		const CAtlREMatchContextT::RECHAR* e = NULL;
		for(; s && re->Match(s, &mc, &e); s = e)
		{
			const CAtlREMatchContextT::RECHAR* szStart = 0;
			const CAtlREMatchContextT::RECHAR* szEnd = 0;
			mc.GetMatch(0, &szStart, &szEnd);

			CString url;
			url.Format(_T("%.*s"), szEnd - szStart, szStart);
			url.Trim();

			if(url.CompareNoCase(_T("asf path")) == 0) continue;

			CUrl dst;
			dst.CrackUrl(CString(url));
			if(_tcsicmp(src.GetSchemeName(), dst.GetSchemeName())
			|| _tcsicmp(src.GetHostName(), dst.GetHostName())
			|| _tcsicmp(src.GetUrlPath(), dst.GetUrlPath()))
			{
				urls.AddTail(url);
			}
			else
			{
				// recursive
				urls.RemoveAll();
				break;
			}
		}
	}

	return urls.GetCount() > 0;
}

bool FindRedir(CString& fn, CString ct, CAtlList<CString>& fns, CAutoPtrList<CAtlRegExpT>& res)
{
	CString body;

	CTextFile f(CTextFile::ANSI);
	if(f.Open(fn)) for(CString tmp; f.ReadString(tmp); body += tmp + '\n');

	CString dir = fn.Left(max(fn.ReverseFind('/'), fn.ReverseFind('\\'))+1); // "ReverseFindOneOf"

	POSITION pos = res.GetHeadPosition();
	while(pos)
	{
		CAtlRegExpT* re = res.GetNext(pos);

		CAtlREMatchContextT mc;
		const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
		const CAtlREMatchContextT::RECHAR* e = NULL;
		for(; s && re->Match(s, &mc, &e); s = e)
		{
			const CAtlREMatchContextT::RECHAR* szStart = 0;
			const CAtlREMatchContextT::RECHAR* szEnd = 0;
			mc.GetMatch(0, &szStart, &szEnd);

			CString fn2;
			fn2.Format(_T("%.*s"), szEnd - szStart, szStart);
			fn2.Trim();

			if(!fn2.CompareNoCase(_T("asf path"))) continue;
			if(fn2.Find(_T("EXTM3U")) == 0 || fn2.Find(_T("#EXTINF")) == 0) continue;

			if(fn2.Find(_T(":")) < 0 && fn2.Find(_T("\\\\")) != 0 && fn2.Find(_T("//")) != 0)
			{
				CPath p;
				p.Combine(dir, fn2);
				fn2 = (LPCTSTR)p;
			}

			if(!fn2.CompareNoCase(fn))
				continue;

			fns.AddTail(fn2);
		}
	}

	return fns.GetCount() > 0;
}

CStringA GetContentType(CString fn, CAtlList<CString>* redir)
{
	CUrl url;
	CString ct, body;

	if(fn.Find(_T("://")) >= 0)
	{
		url.CrackUrl(fn);

		if(_tcsicmp(url.GetSchemeName(), _T("pnm")) == 0)
			return "audio/x-pn-realaudio";

		if(_tcsicmp(url.GetSchemeName(), _T("mms")) == 0)
			return "video/x-ms-asf";

		if(_tcsicmp(url.GetSchemeName(), _T("http")) != 0)
			return "";

		DWORD ProxyEnable = 0;
		CString ProxyServer;
		DWORD ProxyPort = 0;

		ULONG len = 256+1;
		CRegKey key;
		if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
		&& ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
		&& ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer.GetBufferSetLength(256), &len))
		{
			ProxyServer.ReleaseBufferSetLength(len);

			CAtlList<CString> sl;
			ProxyServer = Explode(ProxyServer, sl, ';');
			if(sl.GetCount() > 1)
			{
				POSITION pos = sl.GetHeadPosition();
				while(pos)
				{
					CAtlList<CString> sl2;
					if(!Explode(sl.GetNext(pos), sl2, '=', 2).CompareNoCase(_T("http"))
					&& sl2.GetCount() == 2)
					{
						ProxyServer = sl2.GetTail();
						break;
					}
				}
			}

			ProxyServer = Explode(ProxyServer, sl, ':');
			if(sl.GetCount() > 1) ProxyPort = _tcstol(sl.GetTail(), NULL, 10);
		}

		CSocket s;
		s.Create();
		if(s.Connect(
			ProxyEnable ? ProxyServer : url.GetHostName(), 
			ProxyEnable ? ProxyPort : url.GetPortNumber()))
		{
			CStringA host = CStringA(url.GetHostName());
			CStringA path = CStringA(url.GetUrlPath()) + CStringA(url.GetExtraInfo());

			if(ProxyEnable) path = "http://" + host + path;

			CStringA hdr;
			hdr.Format(
				"GET %s HTTP/1.0\r\n"
				"User-Agent: Media Player Classic\r\n"
				"Host: %s\r\n"
				"Accept: */*\r\n"
				"\r\n", path, host);

// MessageBox(NULL, CString(hdr), _T("Sending..."), MB_OK);

			if(s.Send((LPCSTR)hdr, hdr.GetLength()) < hdr.GetLength()) return "";

			hdr.Empty();
			while(1)
			{
				CStringA str;
				str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
				if(str.IsEmpty()) break;
				hdr += str;
				int hdrend = hdr.Find("\r\n\r\n");
				if(hdrend >= 0) {body = hdr.Mid(hdrend+4); hdr = hdr.Left(hdrend); break;}
			}

// MessageBox(NULL, CString(hdr), _T("Received..."), MB_OK);

			CAtlList<CStringA> sl;
			Explode(hdr, sl, '\n');
			POSITION pos = sl.GetHeadPosition();
			while(pos)
			{
				CStringA& hdrline = sl.GetNext(pos);
				CAtlList<CStringA> sl2;
				Explode(hdrline, sl2, ':', 2);
				CStringA field = sl2.RemoveHead().MakeLower();
				if(field == "location" && !sl2.IsEmpty())
					return GetContentType(CString(sl2.GetHead()), redir);
				if(field == "content-type" && !sl2.IsEmpty())
					ct = sl2.GetHead();
			}

			while(body.GetLength() < 256)
			{
				CStringA str;
				str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
				if(str.IsEmpty()) break;
				body += str;
			}

			if(body.GetLength() >= 8)
			{
				CStringA str = TToA(body);
				if(!strncmp((LPCSTR)str, ".ra", 3))
					return "audio/x-pn-realaudio";
				if(!strncmp((LPCSTR)str, ".RMF", 4))
					return "audio/x-pn-realaudio";
				if(*(DWORD*)(LPCSTR)str == 0x75b22630)
					return "video/x-ms-wmv";
				if(!strncmp((LPCSTR)str+4, "moov", 4))
					return "video/quicktime";
			}

			if(redir && (ct == _T("audio/x-scpls") || ct == _T("audio/x-mpegurl")))
			{
				while(body.GetLength() < 4*1024) // should be enough for a playlist...
				{
					CStringA str;
					str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
					if(str.IsEmpty()) break;
					body += str;
				}
			}
		}
	}
	else if(!fn.IsEmpty())
	{
		CPath p(fn);
		CString ext = p.GetExtension().MakeLower();
		if(ext == _T(".asx")) ct = _T("video/x-ms-asf");
		else if(ext == _T(".pls")) ct = _T("audio/x-scpls");
		else if(ext == _T(".m3u")) ct = _T("audio/x-mpegurl");
		else if(ext == _T(".qtl")) ct = _T("application/x-quicktimeplayer");
		else if(ext == _T(".mpcpl")) ct = _T("application/x-mpc-playlist");
		else if(ext == _T(".bdmv")) ct = _T("application/x-bdmv-playlist");

		if(FILE* f = _tfopen(fn, _T("rb")))
		{
			CStringA str;
			str.ReleaseBufferSetLength(fread(str.GetBuffer(10240), 1, 10240, f));
			body = AToT(str);
			fclose(f);
		}
	}

	if(body.GetLength() >= 4) // here only those which cannot be opened through dshow
	{
		CStringA str = TToA(body);
		if(!strncmp((LPCSTR)str, ".ra", 3))
			return "audio/x-pn-realaudio";
		if(!strncmp((LPCSTR)str, "FWS", 3))
			return "application/x-shockwave-flash";

	}

	if(redir && !ct.IsEmpty())
	{
		CAutoPtrList<CAtlRegExpT> res;
		CAutoPtr<CAtlRegExpT> re;

		if(ct == _T("video/x-ms-asf"))
		{
			// ...://..."/>
			re.Attach(DNew CAtlRegExpT());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("{[a-zA-Z]+://[^\n\">]*}"), FALSE))
				res.AddTail(re);
			// Ref#n= ...://...\n
			re.Attach(DNew CAtlRegExpT());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("Ref\\z\\b*=\\b*[\"]*{([a-zA-Z]+://[^\n\"]+}"), FALSE))
				res.AddTail(re);
		}
		else if(ct == _T("audio/x-scpls"))
		{
			// File1=...\n
			re.Attach(DNew CAtlRegExp<>());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("file\\z\\b*=\\b*[\"]*{[^\n\"]+}"), FALSE))
				res.AddTail(re);
		}
		else if(ct == _T("audio/x-mpegurl"))
		{
			// #comment
			// ...
			re.Attach(DNew CAtlRegExp<>());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("{[^#][^\n]+}"), FALSE))
				res.AddTail(re);
		}
		else if(ct == _T("audio/x-pn-realaudio"))
		{
			// rtsp://...
			re.Attach(DNew CAtlRegExp<>());
			if(re && REPARSE_ERROR_OK == re->Parse(_T("{rtsp://[^\n]+}"), FALSE))
				res.AddTail(re);
		}

		if(!body.IsEmpty())
		{
			if(fn.Find(_T("://")) >= 0) FindRedir(url, ct, body, *redir, res);
			else FindRedir(fn, ct, *redir, res);
		}
	}

	return TToA(ct);
}


LONGLONG CMPlayerCApp::GetPerfCounter()
{
	LARGE_INTEGER		i64Ticks100ns;
	LARGE_INTEGER		llPerfFrequency;

	QueryPerformanceFrequency (&llPerfFrequency);
	if (llPerfFrequency.QuadPart != 0)
	{
		QueryPerformanceCounter (&i64Ticks100ns);
		return llMulDiv (i64Ticks100ns.QuadPart, 10000000, llPerfFrequency.QuadPart, 0);
	}
	else
	{
		// ms to 100ns units
		return timeGetTime() * 10000; 
	}
}

COLORPROPERTY_RANGE* CMPlayerCApp::GetColorControl(ControlType nFlag)
{
	switch (nFlag)
	{
	case Brightness :
		return &m_ColorControl[0];
	case Contrast :
		return &m_ColorControl[1];
	case Hue :
		return &m_ColorControl[2];
	case Saturation :
		return &m_ColorControl[3];
	}
	return NULL;
}


HINSTANCE CMPlayerCApp::GetD3X9Dll()
{
	if (m_hD3DX9Dll == NULL)
	{
		int min_ver = D3DX_SDK_VERSION;
		int max_ver = D3DX_SDK_VERSION;
		
		m_nDXSdkRelease = 0;

		if(D3DX_SDK_VERSION >= 42) {
			// August 2009 SDK (v42) is not compatible with older versions
			min_ver = 42;			
		} else {
			if(D3DX_SDK_VERSION > 33) {
				// versions between 34 and 41 have no known compatibility issues
				min_ver = 34;
			}	else {		
				// The minimum version that supports the functionality required by MPC is 24
				min_ver = 24;
				
				if(D3DX_SDK_VERSION == 33) {
					// The April 2007 SDK (v33) should not be used (crash sometimes during shader compilation)
					max_ver = 32;		
				}				
			}
		}
		
		// load latest compatible version of the DLL that is available
		for (int i=max_ver; i>=min_ver; i--)
		{
			m_strD3DX9Version.Format(_T("d3dx9_%d.dll"), i);
			m_hD3DX9Dll = LoadLibrary (m_strD3DX9Version);
			if (m_hD3DX9Dll) 
			{
				m_nDXSdkRelease = i;
				break;
			}
		}
	}

	return m_hD3DX9Dll;
}

LPCTSTR CMPlayerCApp::GetSatelliteDll(int nLanguage)
{
	switch (nLanguage)
	{
	case 1:		// French
		return _T("mpcresources.fr.dll");
	case 2:		// German
		return _T("mpcresources.de.dll");
	case 3:		// Russian
		return _T("mpcresources.ru.dll");
	case 4:		// Turkish
		return _T("mpcresources.tr.dll");
	case 5:		// Czech
		return _T("mpcresources.cz.dll");
	case 6:		// Spanish
		return _T("mpcresources.es.dll");
	case 7:		// Hungarian
		return _T("mpcresources.hu.dll");
	case 8:		// Korean
		return _T("mpcresources.kr.dll");
	case 9:		// Polish
		return _T("mpcresources.pl.dll");
	case 10:	// Ukrainian
		return _T("mpcresources.ua.dll");
	case 11:	// Italian
		return _T("mpcresources.it.dll");
	case 12 :	// Slovak
		return _T("mpcresources.sk.dll");
	case 13 :	// Chinese (simplified)
		return _T("mpcresources.sc.dll");
	case 14 :	// Chinese (traditional)
		return _T("mpcresources.tc.dll");
	case 15 :	// Belarusian
		return _T("mpcresources.by.dll");
	case 16 :	// Sweedish
		return _T("mpcresources.sv.dll");
	case 17 :	// Portuguese (brasil)
		return _T("mpcresources.br.dll");
	case 18 :	// Dutch
		return _T("mpcresources.nl.dll");
	}
	return NULL;
}


void CMPlayerCApp::SetLanguage (int nLanguage)
{
	AppSettings&	s = AfxGetAppSettings();
	HMODULE		hMod = NULL;
	LPCTSTR		strSatellite;

	strSatellite = GetSatelliteDll( nLanguage );
	if ( strSatellite )
	{
		CFileVersionInfo	Version;
		CString				strSatVersion;

		if ( Version.Create(strSatellite) )
		{
			strSatVersion = Version.GetFileVersionEx();

			if ( strSatVersion == _T("1.3.0.0") )
			{
				hMod = LoadLibrary( strSatellite );
				s.iLanguage = nLanguage;
			}
			else
			{
				// This message should stay in english!
				MessageBox(NULL, _T("Your language pack will not work with this version. Please download a compatible one from the MPC-HC homepage."), 
								 _T("Media Player Classic - Home Cinema"), MB_OK);
			}
		}
	}

	if ( hMod == NULL )
	{
		hMod = AfxGetApp()->m_hInstance;
		s.iLanguage = 0;
	}

	AfxSetResourceHandle( hMod );
}


bool CMPlayerCApp::IsVistaOrAbove()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	
	if (	::GetVersionEx( &osver ) && 
			osver.dwPlatformId == VER_PLATFORM_WIN32_NT && 
			(osver.dwMajorVersion >= 6 ) )
		return TRUE;

	return FALSE;
}

bool CMPlayerCApp::IsVSFilterInstalled()
{
	bool result = false;
	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\{9852A670-F845-491B-9BE6-EBD841B8A613}"), KEY_READ)) {
		result = true;
	}
	
	return result;
}

bool CMPlayerCApp::HasEVR()
{
	bool result = false;
	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\{FA10746C-9B63-4B6C-BC49-FC300EA5F256}"), KEY_READ)) {
		result = true;
	}
	
	return result;
}

HRESULT CMPlayerCApp::GetElevationType(TOKEN_ELEVATION_TYPE* ptet )
{
	ASSERT( IsVistaOrAbove() );
	ASSERT( ptet );

	HRESULT hResult = E_FAIL; // assume an error occured
	HANDLE hToken	= NULL;

	if ( !::OpenProcessToken( 
				::GetCurrentProcess(), 
				TOKEN_QUERY, 
				&hToken ) )
	{
		ASSERT( FALSE );
		return hResult;
	}

	DWORD dwReturnLength = 0;

	if ( !::GetTokenInformation(
				hToken,
				TokenElevationType,
				ptet,
				sizeof( *ptet ),
				&dwReturnLength ) )
	{
		ASSERT( FALSE );
	}
	else
	{
		ASSERT( dwReturnLength == sizeof( *ptet ) );
		hResult = S_OK;
	}

	::CloseHandle( hToken );

	return hResult;
}

void CMPlayerCApp::RunAsAdministrator(LPCTSTR strCommand, LPCTSTR strArgs, bool bWaitProcess)
{
	SHELLEXECUTEINFO execinfo;
	memset(&execinfo, 0, sizeof(execinfo));
	execinfo.lpFile			= strCommand;
	execinfo.cbSize			= sizeof(execinfo);
	execinfo.lpVerb			= _T("runas");
	execinfo.fMask			= SEE_MASK_NOCLOSEPROCESS;
	execinfo.nShow			= SW_SHOWDEFAULT;
	execinfo.lpParameters	= strArgs;
		
	ShellExecuteEx(&execinfo);

	if (bWaitProcess)
		WaitForSingleObject(execinfo.hProcess, INFINITE);
}

void CAboutDlg::OnHomepage(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(m_hWnd, _T("open"), _T("http://mpc-hc.sourceforge.net/about-homepage.html"), NULL, NULL, SW_SHOWDEFAULT);	
	*pResult = 0;
}
