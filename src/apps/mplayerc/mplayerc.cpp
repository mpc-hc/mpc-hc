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
#include "mplayerc.h"
#include <Tlhelp32.h>
#include "MainFrm.h"
#include "../../DSUtil/DSUtil.h"
#include "Struct.h"
#include "FileVersionInfo.h"
#include <psapi.h>
#include "Ifo.h"
#include "Monitors.h"
#include "Version.h"
#include "WinAPIUtils.h"


extern "C" {
	int mingw_app_type = 1;
}

void CorrectComboListWidth(CComboBox& box, CFont* pWndFont)
{
	int cnt = box.GetCount();
	if (cnt <= 0) {
		return;
	}

	CDC* pDC = box.GetDC();
	pDC->SelectObject(pWndFont);

	int maxw = box.GetDroppedWidth();

	for (int i = 0; i < cnt; i++) {
		CString str;
		box.GetLBText(i, str);
		int w = pDC->GetTextExtent(str).cx + 22;
		if (maxw < w) {
			maxw = w;
		}
	}

	box.ReleaseDC(pDC);

	box.SetDroppedWidth(maxw);
}

HICON LoadIcon(CString fn, bool fSmall)
{
	if (fn.IsEmpty()) {
		return(NULL);
	}

	CString ext = fn.Left(fn.Find(_T("://"))+1).TrimRight(':');
	if (ext.IsEmpty() || !ext.CompareNoCase(_T("file"))) {
		ext = _T(".") + fn.Mid(fn.ReverseFind('.')+1);
	}

	CSize size(fSmall?16:32,fSmall?16:32);

	if (!ext.CompareNoCase(_T(".ifo"))) {
		if (HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DVD), IMAGE_ICON, size.cx, size.cy, 0)) {
			return(hIcon);
		}
	}

	if (!ext.CompareNoCase(_T(".cda"))) {
		if (HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_AUDIOCD), IMAGE_ICON, size.cx, size.cy, 0)) {
			return(hIcon);
		}
	}

	do {
		CRegKey key;

		TCHAR buff[256];
		ULONG len;

		if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext + _T("\\DefaultIcon"), KEY_READ)) {
			if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext, KEY_READ)) {
				break;
			}

			len = sizeof(buff)/sizeof(buff[0]);
			memset(buff, 0, sizeof(buff));
			if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) || (ext = buff).Trim().IsEmpty()) {
				break;
			}

			if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext + _T("\\DefaultIcon"), KEY_READ)) {
				break;
			}
		}

		CString icon;

		len = sizeof(buff)/sizeof(buff[0]);
		memset(buff, 0, sizeof(buff));
		if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) || (icon = buff).Trim().IsEmpty()) {
			break;
		}

		int i = icon.ReverseFind(',');
		if (i < 0) {
			break;
		}

		int id = 0;
		if (_stscanf_s(icon.Mid(i+1), _T("%d"), &id) != 1) {
			break;
		}

		icon = icon.Left(i);

		HICON hIcon = NULL;
		UINT cnt = fSmall
				   ? ExtractIconEx(icon, id, NULL, &hIcon, 1)
				   : ExtractIconEx(icon, id, &hIcon, NULL, 1);
		UNUSED_ALWAYS(cnt);
		if (hIcon) {
			return hIcon;
		}
	} while (0);

	return((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_UNKNOWN), IMAGE_ICON, size.cx, size.cy, 0));
}

bool LoadType(CString fn, CString& type)
{
	CRegKey key;

	TCHAR buff[256];
	ULONG len;

	if (fn.IsEmpty()) {
		return(NULL);
	}

	CString ext = fn.Left(fn.Find(_T("://"))+1).TrimRight(':');
	if (ext.IsEmpty() || !ext.CompareNoCase(_T("file"))) {
		ext = _T(".") + fn.Mid(fn.ReverseFind('.')+1);
	}

	CString tmp = _T("");
	CString mplayerc_ext = _T("mplayerc") + ext;

	if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, mplayerc_ext)) {
		tmp = mplayerc_ext;
	}

	if ((tmp == _T("")) && (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext))) {
		return(false);
	}

	if (tmp == _T("")) {
		tmp = ext;
	}

	while (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, tmp)) {
		len = sizeof(buff)/sizeof(buff[0]);
		memset(buff, 0, sizeof(buff));
		if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len)) {
			break;
		}

		CString str(buff);
		str.Trim();

		if (str.IsEmpty() || str == tmp) {
			break;
		}

		tmp = str;
	}

	type = tmp;

	return(true);
}

bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype)
{
	str.Empty();
	HRSRC hrsrc = FindResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(resid), restype);
	if (!hrsrc) {
		return(false);
	}
	HGLOBAL hGlobal = LoadResource(AfxGetApp()->m_hInstance, hrsrc);
	if (!hGlobal) {
		return(false);
	}
	DWORD size = SizeofResource(AfxGetApp()->m_hInstance, hrsrc);
	if (!size) {
		return(false);
	}
	memcpy(str.GetBufferSetLength(size), LockResource(hGlobal), size);
	return(true);
}

WORD AssignedToCmd(UINT keyOrMouseValue, bool bIsFullScreen, bool bCheckMouse)
{
	WORD assignTo = 0;
	AppSettings& s = AfxGetAppSettings();

	POSITION pos = s.wmcmds.GetHeadPosition();
	while (pos && !assignTo) {
		wmcmd& wc = s.wmcmds.GetNext(pos);

		if (bCheckMouse) {
			if (bIsFullScreen) {
				if (wc.mouseFS == keyOrMouseValue) {
					assignTo = wc.cmd;
				}
			} else if (wc.mouse == keyOrMouseValue) {
				assignTo = wc.cmd;
			}
		} else if (wc.key == keyOrMouseValue) {
			assignTo = wc.cmd;
		}
	}

	return assignTo;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

extern "C" char *GetFFmpegCompiler();

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
	virtual BOOL OnInitDialog() {
		USES_CONVERSION;
		UpdateData();

#ifdef _WIN64
		m_appname += _T(" x64");
#endif

		m_strBuildNumber = AfxGetMyApp()->m_strVersion;

#if defined(__INTEL_COMPILER)
	#if (__INTEL_COMPILER >= 1200)
		m_MPCCompiler = _T("ICL 12.x");
	#elif (__INTEL_COMPILER >= 1100)
		m_MPCCompiler = _T("ICL 11.x");
	#elif (__INTEL_COMPILER >= 1000)
		m_MPCCompiler = _T("ICL 10.x");
	#else
		#error Compiler is not supported!
	#endif
#elif defined(_MSC_VER)
	#if (_MSC_VER == 1600)
		#if (_MSC_FULL_VER >= 160040219)
			m_MPCCompiler = _T("MSVC 2010 SP1");
		#else
			m_MPCCompiler = _T("MSVC 2010");
		#endif
	#elif (_MSC_VER == 1500)
		#if (_MSC_FULL_VER >= 150030729)
			m_MPCCompiler = _T("MSVC 2008 SP1");
		#else
			m_MPCCompiler = _T("MSVC 2008");
		#endif
	#elif (_MSC_VER < 1500)
		#error Compiler is not supported!
	#endif

	// Note: /arch:SSE and /arch:SSE2 are only available when you compile for the x86 platform.
	// Link: http://msdn.microsoft.com/en-us/library/7t5yh4fd.aspx
	// Link: http://msdn.microsoft.com/en-us/library/b0084kay.aspx
	#if !defined(_M_X64) && defined(_M_IX86_FP)
		//#if (_M_IX86_FP == 0) // 0 if /arch was not used.
		//	m_MPCCompiler += _T("");
		#if (_M_IX86_FP == 1) // 1 if /arch:SSE was used.
			m_MPCCompiler += _T(" (SSE)");
		#elif (_M_IX86_FP == 2) // 2 if /arch:SSE2 was used.
			m_MPCCompiler += _T(" (SSE2)");
		#endif
	#endif // _M_IX86_FP
	#ifdef _DEBUG
		m_MPCCompiler += _T(" Debug");
	#endif
#else
	#error Please add support for your compiler
#endif

#if HAS_FFMPEG
		m_FFmpegCompiler.Format (A2W(GetFFmpegCompiler()));
#endif

		UpdateData(FALSE);
		return TRUE;
	}
	CString m_appname;
	CString m_strBuildNumber;
	CString m_MPCCompiler;
	CString m_FFmpegCompiler;
	afx_msg void OnHomepage(NMHDR *pNMHDR, LRESULT *pResult);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD), m_appname(_T(""))
	, m_strBuildNumber(_T(""))
	, m_MPCCompiler(_T(""))
	, m_FFmpegCompiler(_T(""))
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
	DDX_Text(pDX, IDC_FFMPEG_COMPILER, m_FFmpegCompiler);
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

	GetModuleFileNameEx (GetCurrentProcess(), AfxGetMyApp()->m_hInstance, strApp, _MAX_PATH);
	Version.Create (strApp);
	m_strVersion = Version.GetFileVersionEx();

	memset (&m_ColorControl, 0, sizeof(m_ColorControl));
	ResetColorControlRange();

	memset (&m_VMR9ColorControl, 0, sizeof(m_VMR9ColorControl));
	m_VMR9ColorControl[0].dwSize		= sizeof (VMR9ProcAmpControlRange);
	m_VMR9ColorControl[0].dwProperty	= ProcAmpControl9_Brightness;
	m_VMR9ColorControl[1].dwSize		= sizeof (VMR9ProcAmpControlRange);
	m_VMR9ColorControl[1].dwProperty	= ProcAmpControl9_Contrast;
	m_VMR9ColorControl[2].dwSize		= sizeof (VMR9ProcAmpControlRange);
	m_VMR9ColorControl[2].dwProperty	= ProcAmpControl9_Hue;
	m_VMR9ColorControl[3].dwSize		= sizeof (VMR9ProcAmpControlRange);
	m_VMR9ColorControl[3].dwProperty	= ProcAmpControl9_Saturation;

	memset (&m_EVRColorControl, 0, sizeof(m_EVRColorControl));

	GetRemoteControlCode = GetRemoteControlCodeMicrosoft;
}

void CMPlayerCApp::ShowCmdlnSwitches() const
{
	CString s;

	if (m_s.nCLSwitches&CLSW_UNRECOGNIZEDSWITCH) {
		CAtlList<CString> sl;
		for (int i = 0; i < __argc; i++) {
			sl.AddTail(__targv[i]);
		}
		s += ResStr(IDS_UNKNOWN_SWITCH) + Implode(sl, ' ') + _T("\n\n");
	}

	s += ResStr(IDS_USAGE);

	AfxMessageBox(s, MB_ICONINFORMATION | MB_OK);
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
		if (!(f = _tfopen(ini, _T("r+"))) && !(f = _tfopen(ini, _T("w"))))
			return StoreSettingsToRegistry();
		fclose(f);
	*/
	free((void*)m_pszRegistryKey);
	m_pszRegistryKey = NULL;
	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(ini);

	// We can only use UTF16-LE for unicode ini files in windows. UTF8/UTF16-BE do not work.
	// So to ensure we have correct encoding for ini files, create a file with right BOM first,
	// then add some comments in first line to make sure it's not empty.

	// If you want to try unicode ini, uncomment following code block.
	/*
	if (!::PathFileExists(m_pszProfileName)) // don't overwrite existing ini file
	{
		LPTSTR pszComments = _T("; Media Player Classic - Home Cinema");
		WORD wBOM = 0xFEFF;// UTF16-LE BOM(FFFE)
		DWORD nBytes;

		HANDLE hFile = ::CreateFile(m_pszProfileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::WriteFile(hFile, &wBOM, sizeof(WORD), &nBytes, NULL);
			::WriteFile(hFile, pszComments, (_tcslen(pszComments)+1)*(sizeof(TCHAR)), &nBytes, NULL);
			::CloseHandle(hFile);
		}
	}
	*/

	return(true);
}

bool CMPlayerCApp::StoreSettingsToRegistry()
{
	_tremove(GetIniPath());

	free((void*)m_pszRegistryKey);
	m_pszRegistryKey = NULL;

	SetRegistryKey(_T("Gabest"));

	return(true);
}

CString CMPlayerCApp::GetIniPath() const
{
	CString path;
	GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(_MAX_PATH), _MAX_PATH);
	path.ReleaseBuffer();
	path = path.Left(path.ReverseFind('.')+1) + _T("ini");
	return(path);
}

bool CMPlayerCApp::IsIniValid() const
{
	CFileStatus fs;
	return !!CFile::GetStatus(GetIniPath(), fs);
}

bool CMPlayerCApp::GetAppSavePath(CString& path)
{
	path.Empty();

	if (IsIniValid()) { // If settings ini file found, store stuff in the same folder as the exe file
		GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(_MAX_PATH), _MAX_PATH);
		path.ReleaseBuffer();
		path = path.Left(path.ReverseFind('\\'));
	} else {
		CRegKey key;
		if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), KEY_READ)) {
			ULONG len = _MAX_PATH;
			if (ERROR_SUCCESS == key.QueryStringValue(_T("AppData"), path.GetBuffer(_MAX_PATH), &len)) {
				path.ReleaseBufferSetLength(len);
			}
		}

		if (path.IsEmpty()) {
			return(false);
		}

		CPath p;
		p.Combine(path, _T("Media Player Classic"));

		path = (LPCTSTR)p;
	}

	return(true);
}

bool CMPlayerCApp::ChangeSettingsLocation(bool useIni)
{
	bool success;

	// Load favorites so that they can be correctly saved to the new location
	CAtlList<CString> filesFav, DVDsFav, devicesFav;
	AfxGetAppSettings().GetFav(FAV_FILE, filesFav);
	AfxGetAppSettings().GetFav(FAV_DVD, DVDsFav);
	AfxGetAppSettings().GetFav(FAV_DEVICE, devicesFav);

	if (useIni) {
		success = StoreSettingsToIni();
	} else {
		success = StoreSettingsToRegistry();
	}

	// Save favorites to the new location
	AfxGetAppSettings().SetFav(FAV_FILE, filesFav);
	AfxGetAppSettings().SetFav(FAV_DVD, DVDsFav);
	AfxGetAppSettings().SetFav(FAV_DEVICE, devicesFav);

	return success;
}

void CMPlayerCApp::ExportSettings()
{
	CString ext = IsIniValid() ? _T("ini") : _T("reg");
	CFileDialog fileSaveDialog(FALSE, ext, _T("mpc-hc-settings.")+ext);

	if (fileSaveDialog.DoModal() == IDOK) {
		CString savePath = fileSaveDialog.GetPathName();
		bool success;

		AfxGetAppSettings().UpdateData(true);

		if (IsIniValid()) {
			success = !!CopyFile(GetIniPath(), savePath, FALSE);
		} else {
			CString regKey;
			regKey.Format(_T("Software\\%s\\%s"), m_pszRegistryKey, m_pszProfileName);

			FILE* fStream;
			errno_t error = _tfopen_s(&fStream, savePath, _T("wt,ccs=UNICODE"));
			CStdioFile file(fStream);
			file.WriteString(_T("Windows Registry Editor Version 5.00\n\n"));

			success = !error && ExportRegistryKey(file, HKEY_CURRENT_USER, regKey);

			file.Close();
		}

		if (success) {
			MessageBox(GetMainWnd()->m_hWnd, ResStr(IDS_EXPORT_SETTINGS_SUCCESS), ResStr(IDS_EXPORT_SETTINGS), MB_ICONINFORMATION | MB_OK);
		} else {
			MessageBox(GetMainWnd()->m_hWnd, ResStr(IDS_EXPORT_SETTINGS_FAILED), ResStr(IDS_EXPORT_SETTINGS), MB_ICONERROR | MB_OK);
		}
	}
}

void CMPlayerCApp::PreProcessCommandLine()
{
	m_cmdln.RemoveAll();
	for (int i = 1; i < __argc; i++) {
		CString str = CString(__targv[i]).Trim(_T(" \""));

		if (str[0] != '/' && str[0] != '-' && str.Find(_T(":")) < 0) {
			LPTSTR p = NULL;
			CString str2;
			str2.ReleaseBuffer(GetFullPathName(str, _MAX_PATH, str2.GetBuffer(_MAX_PATH), &p));
			CFileStatus fs;
			if (!str2.IsEmpty() && CFileGetStatus(str2, fs)) {
				str = str2;
			}
		}

		m_cmdln.AddTail(str);
	}
}

void CMPlayerCApp::SendCommandLine(HWND hWnd)
{
	if (m_cmdln.IsEmpty()) {
		return;
	}

	int bufflen = sizeof(DWORD);

	POSITION pos = m_cmdln.GetHeadPosition();
	while (pos) {
		bufflen += (m_cmdln.GetNext(pos).GetLength()+1)*sizeof(TCHAR);
	}

	CAutoVectorPtr<BYTE> buff;
	if (!buff.Allocate(bufflen)) {
		return;
	}

	BYTE* p = buff;

	*(DWORD*)p = m_cmdln.GetCount();
	p += sizeof(DWORD);

	pos = m_cmdln.GetHeadPosition();
	while (pos) {
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
	= NULL;
*/


BOOL WINAPI Mine_IsDebuggerPresent()
{
	TRACE(_T("Oops, somebody was trying to be naughty! (called IsDebuggerPresent)\n"));
	return FALSE;
}


NTSTATUS WINAPI Mine_NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
	NTSTATUS		nRet;

	nRet = Real_NtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

	if (ProcessInformationClass == ProcessBasicInformation) {
		PROCESS_BASIC_INFORMATION*		pbi = (PROCESS_BASIC_INFORMATION*)ProcessInformation;
		PEB_NT*							pPEB;
		PEB_NT							PEB;

		pPEB = (PEB_NT*)pbi->PebBaseAddress;
		ReadProcessMemory(ProcessHandle, pPEB, &PEB, sizeof(PEB), NULL);
		PEB.BeingDebugged = 0;
		WriteProcessMemory(ProcessHandle, pPEB, &PEB, sizeof(PEB), NULL);
	} else if (ProcessInformationClass == 7) { // ProcessDebugPort
		BOOL*		pDebugPort = (BOOL*)ProcessInformation;
		*pDebugPort = FALSE;
	}

	return nRet;
}

LONG WINAPI Mine_ChangeDisplaySettingsEx(LONG ret, DWORD dwFlags, LPVOID lParam)
{
	if (dwFlags&CDS_VIDEOPARAMETERS) {
		VIDEOPARAMETERS* vp = (VIDEOPARAMETERS*)lParam;

		if (vp->Guid == GUIDFromCString(_T("{02C62061-1097-11d1-920F-00A024DF156E}"))
				&& (vp->dwFlags&VP_FLAGS_COPYPROTECT)) {
			if (vp->dwCommand == VP_COMMAND_GET) {
				if ((vp->dwTVStandard&VP_TV_STANDARD_WIN_VGA)
						&& vp->dwTVStandard != VP_TV_STANDARD_WIN_VGA) {
					TRACE(_T("Ooops, tv-out enabled? macrovision checks suck..."));
					vp->dwTVStandard = VP_TV_STANDARD_WIN_VGA;
				}
			} else if (vp->dwCommand == VP_COMMAND_SET) {
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
	//if (i > 0 && i == fn.GetLength() - 5)
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

	if (!GetTempPathW(_MAX_PATH, szTempPath)) {
		return FALSE;
	}

	_wsplitpath_s (strIFOPath, NULL, 0, NULL, 0, strFileName, countof(strFileName), strExt, countof(strExt));
	_snwprintf_s  (strFakeFile, nFakeFileSize, _TRUNCATE, L"%sMPC%s%s", szTempPath, strFileName, strExt);

	if (Ifo.OpenFile (strIFOPath) &&
			Ifo.RemoveUOPs()  &&
			Ifo.SaveFile (strFakeFile)) {
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

	if (nLen>=4 && _wcsicmp (p1 + nLen-4, L".ifo") == 0) {
		if (CreateFakeVideoTS(p1, strFakeFile, countof(strFakeFile))) {
			hFile = Real_CreateFileW(strFakeFile, p2, p3, p4, p5, p6, p7);
		}
	}

	if (hFile == INVALID_HANDLE_VALUE) {
		hFile = Real_CreateFileW(p1, p2, p3, p4, p5, p6, p7);
	}

	return hFile;
}

MMRESULT WINAPI Mine_mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
	if (fdwDetails == (MIXER_OBJECTF_HMIXER|MIXER_SETCONTROLDETAILSF_VALUE)) {
		return MMSYSERR_NOERROR;    // don't touch the mixer, kthx
	}
	return Real_mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
}

BOOL WINAPI Mine_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
	BOOL ret = Real_DeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

	if (IOCTL_DVD_GET_REGION == dwIoControlCode && lpOutBuffer
			&& lpBytesReturned && *lpBytesReturned == sizeof(DVD_REGION)) {
		DVD_REGION* pDVDRegion = (DVD_REGION*)lpOutBuffer;
		pDVDRegion->SystemRegion = ~pDVDRegion->RegionData;
	}

	return ret;
}

// Class ssftest is nowhere to see except here. Right now remove it from release build, should we just remove it completely?
#ifdef _DEBUG
#include "../../Subtitles/SSF.h"
#include "../../Subtitles/RTS.h"
#include "../../SubPic/MemSubPic.h"

class ssftest
{
public:
	ssftest() {
		Sleep(10000);

		MessageBeep((UINT)-1);
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
				s.Open(_T("../../Subtitles/libssf/demo/demo.ssa"), 1);

				for (int i = 2*60*1000+2000; i < 2*60*1000+17000; i += 10)
				{
					memsetd(spd.bits, 0xff000000, spd.pitch*spd.h);
					CRect bbox;
					bbox.SetRectEmpty();
					s.Render(spd, 10000i64*i, 25, bbox);
				}
		*/
		try {
			ssf::CRenderer s(&csLock);
			s.Open(_T("../../Subtitles/libssf/demo/demo.ssf"));

			for (int i = 2*60*1000+2000; i < 2*60*1000+17000; i += 40)
				//for (int i = 2*60*1000+2000; i < 2*60*1000+17000; i += 1000)
				//for (int i = 0; i < 5000; i += 40)
			{
				memsetd(spd.bits, 0xff000000, spd.pitch*spd.h);
				CRect bbox;
				bbox.SetRectEmpty();
				s.Render(spd, 10000i64*i, 25, bbox);
			}
		} catch (ssf::Exception& e) {
			UNREFERENCED_PARAMETER(e);
			TRACE(_T("%s\n"), e.ToString());
			ASSERT(0);
		}

		delete [] spd.bits;

		::ExitProcess(0);
	}
};
#endif

BOOL SetHeapOptions()
{
	HMODULE hLib = LoadLibrary(L"kernel32.dll");
	if (hLib == NULL) {
		return FALSE;
	}

	typedef BOOL (WINAPI *HSI)
	(HANDLE, HEAP_INFORMATION_CLASS ,PVOID, SIZE_T);
	HSI pHsi = (HSI)GetProcAddress(hLib,"HeapSetInformation");
	if (!pHsi) {
		FreeLibrary(hLib);
		return FALSE;
	}

#ifndef HeapEnableTerminationOnCorruption
#	define HeapEnableTerminationOnCorruption (HEAP_INFORMATION_CLASS)1
#endif

	BOOL fRet = (pHsi)(NULL,HeapEnableTerminationOnCorruption,NULL,0)
				? TRUE
				: FALSE;
	if (hLib) {
		FreeLibrary(hLib);
	}

	return fRet;
}

BOOL CMPlayerCApp::InitInstance()
{
	// Remove the working directory from the search path to work around the DLL preloading vulnerability
	SetDllDirectory(_T(""));

	long		lError;

#ifdef GOTHTRACE
	// Used for tracing when debugger can't be used, e.g. when using some commercial decoders
	// Print traces usint _tprintf()
	if (AllocConsole()) {
		FILE * foo; // Not used
		freopen_s(&foo, "conin$", "r", stdin); // Redirect stdin etc. to console
		freopen_s(&foo, "conout$", "w", stdout);
		freopen_s(&foo, "conout$", "w", stderr);
	} else {
		AfxMessageBox(_T("Could not create console"));
	}
#endif

	if (SetHeapOptions()) {
		TRACE(_T("Terminate on corruption enabled\n"));
	} else {
		CString heap_err;
		heap_err.Format(_T("Terminate on corruption error = %d\n"), GetLastError());
		TRACE(heap_err);
	}

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
	if (hNTDLL) {
		Real_NtQueryInformationProcess = (FUNC_NTQUERYINFORMATIONPROCESS)GetProcAddress (hNTDLL, "NtQueryInformationProcess");

		if (Real_NtQueryInformationProcess) {
			DetourAttach(&(PVOID&)Real_NtQueryInformationProcess, (PVOID)Mine_NtQueryInformationProcess);
		}
	}
#endif

	CFilterMapper2::Init();

	lError = DetourTransactionCommit();
	ASSERT (lError == NOERROR);

	HRESULT hr;
	if (FAILED(hr = OleInitialize(0))) {
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

	if (!AfxRegisterClass(&wndcls)) {
		AfxMessageBox(_T("MainFrm class registration failed!"));
		return FALSE;
	}

	if (!AfxSocketInit(NULL)) {
		AfxMessageBox(_T("AfxSocketInit failed!"));
		return FALSE;
	}

	PreProcessCommandLine();

	if (IsIniValid()) {
		StoreSettingsToIni();
	} else {
		StoreSettingsToRegistry();
	}

	m_s.ParseCommandLine(m_cmdln);

	if (m_s.nCLSwitches&(CLSW_HELP|CLSW_UNRECOGNIZEDSWITCH)) {
		m_s.UpdateData(false);
		ShowCmdlnSwitches();
		return FALSE;
	}

	if (m_s.nCLSwitches & CLSW_RESET) {
		// We want the other instances to be closed before resetting the settings.
		HWND hWnd = FindWindow(MPC_WND_CLASS_NAME, NULL);

		while (hWnd) {
			Sleep(500);

			hWnd = FindWindow(MPC_WND_CLASS_NAME, NULL);

			if (hWnd && MessageBox(NULL, ResStr(IDS_RESET_SETTINGS_MUTEX), ResStr(IDS_RESET_SETTINGS), MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDCANCEL) {
				return FALSE;
			}
		}

		// Remove the settings
		if (IsIniValid()) {
			CFile::Remove(GetIniPath());
		} else {
			HKEY reg = GetAppRegistryKey();
			SHDeleteKey(reg, _T(""));
			RegCloseKey(reg);
		}

		// Remove the current playlist if it exists
		CString strSavePath;
		if (AfxGetMyApp()->GetAppSavePath(strSavePath)) {
			CPath playlistPath;
			playlistPath.Combine(strSavePath, _T("default.mpcpl"));

			CFileStatus status;
			if (CFile::GetStatus(playlistPath, status)) {
				CFile::Remove(playlistPath);
			}
		}
	}

	if ((m_s.nCLSwitches&CLSW_CLOSE) && m_s.slFiles.IsEmpty()) {
		return FALSE;
	}

	m_s.UpdateData(false);

	if (m_s.nCLSwitches & (CLSW_REGEXTVID | CLSW_REGEXTAUD | CLSW_REGEXTPL)) {
		CMediaFormats& mf = m_s.m_Formats;

		bool bAudioOnly, bPlaylist;

		for (size_t i = 0; i < mf.GetCount(); i++) {
			bPlaylist = !mf[i].GetLabel().CompareNoCase(_T("pls"));

			if (bPlaylist && !(m_s.nCLSwitches & CLSW_REGEXTPL)) {
				continue;
			}

			bAudioOnly = mf[i].IsAudioOnly();

			int j = 0;
			CString str = mf[i].GetExtsWithPeriod();
			for (CString ext = str.Tokenize(_T(" "), j); !ext.IsEmpty(); ext = str.Tokenize(_T(" "), j)) {
				if (((m_s.nCLSwitches & CLSW_REGEXTVID) && !bAudioOnly) ||
					((m_s.nCLSwitches & CLSW_REGEXTAUD) && bAudioOnly) ||
					((m_s.nCLSwitches & CLSW_REGEXTPL) && bPlaylist)) {
					CPPageFormats::RegisterExt(ext, mf[i].GetDescription(), true);
				}
			}
		}
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

		return FALSE;
	}

	if ((m_s.nCLSwitches&CLSW_UNREGEXT)) {
		CMediaFormats& mf = m_s.m_Formats;

		for (int i = 0; i < (int)mf.GetCount(); i++) {
			int j = 0;
			CString str = mf[i].GetExtsWithPeriod();
			for (CString ext = str.Tokenize(_T(" "), j); !ext.IsEmpty(); ext = str.Tokenize(_T(" "), j)) {
				CPPageFormats::RegisterExt(ext, mf[i].GetDescription(), false);
			}
		}
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

		return FALSE;
	}

	// Enable to open options with administrator privilege (for Vista UAC)
	if (m_s.nCLSwitches & CLSW_ADMINOPTION) {
		switch (m_s.iAdminOption) {
			case CPPageFormats::IDD : {
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

	if (GetLastError() == ERROR_ALREADY_EXISTS
			&& (!(m_s.fAllowMultipleInst || (m_s.nCLSwitches&CLSW_NEW) || m_cmdln.IsEmpty())
				|| (m_s.nCLSwitches&CLSW_ADD))) {
		int wait_count = 0;
		HWND hWnd = ::FindWindow(MPC_WND_CLASS_NAME, NULL);
		while (!hWnd && (wait_count++<200)) {
			Sleep(100);
			hWnd = ::FindWindow(MPC_WND_CLASS_NAME, NULL);
		}
		if (hWnd && (wait_count<200)) {
			SetForegroundWindow(hWnd);

			if (!(m_s.nCLSwitches&CLSW_MINIMIZED) && IsIconic(hWnd)) {
				ShowWindow(hWnd, SW_RESTORE);
			}

			SendCommandLine(hWnd);

			return FALSE;
		}
	}

	AfxGetMyApp()->m_AudioRendererDisplayName_CL = _T("");

	if (!__super::InitInstance()) {
		AfxMessageBox(_T("InitInstance failed!"));
		return FALSE;
	}

	CRegKey key;
	if (ERROR_SUCCESS == key.Create(HKEY_LOCAL_MACHINE, _T("Software\\Gabest\\Media Player Classic"))) {
		CString path;
		GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(_MAX_PATH), _MAX_PATH);
		path.ReleaseBuffer();
		key.SetStringValue(_T("ExePath"), path);
	}

	AfxEnableControlContainer();

	CMainFrame* pFrame = DNew CMainFrame;
	m_pMainWnd = pFrame;
	if ( !pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW|FWS_ADDTOTITLE, NULL, NULL) ) {
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
	if (m_s.fWinLirc) {
		m_s.WinLircClient.Connect(m_s.strWinLircAddr);
	}
	m_s.UIceClient.SetHWND(m_pMainWnd->m_hWnd);
	if (m_s.fUIce) {
		m_s.UIceClient.Connect(m_s.strUIceAddr);
	}

	SendCommandLine(m_pMainWnd->m_hWnd);
	RegisterHotkeys();

	pFrame->SetFocus();

	// set HIGH I/O Priority for better playback perfomance
	if (hNTDLL) {
		typedef NTSTATUS (WINAPI *FUNC_NTSETINFORMATIONPROCESS)(HANDLE, ULONG, PVOID, ULONG);
		FUNC_NTSETINFORMATIONPROCESS NtSetInformationProcess = (FUNC_NTSETINFORMATIONPROCESS)GetProcAddress (hNTDLL, "NtSetInformationProcess");

		if (NtSetInformationProcess && SetPrivilege(SE_INC_BASE_PRIORITY_NAME)) {
			ULONG IoPriority = 3;
			ULONG ProcessIoPriority = 0x21;
			NTSTATUS NtStatus = NtSetInformationProcess(GetCurrentProcess(), ProcessIoPriority, &IoPriority, sizeof(ULONG));
			TRACE(_T("Set I/O Priority - %d\n"), NtStatus);
		}

		FreeLibrary( hNTDLL );
		hNTDLL = NULL;
	}

	return TRUE;
}

UINT CMPlayerCApp::GetRemoteControlCodeMicrosoft(UINT nInputcode, HRAWINPUT hRawInput)
{
	UINT		dwSize		= 0;
	BYTE*		pRawBuffer	= NULL;
	UINT		nMceCmd		= 0;

	// Support for MCE remote control
	GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	if (dwSize > 0) {
		pRawBuffer = DNew BYTE[dwSize];
		if (GetRawInputData(hRawInput, RID_INPUT, pRawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != -1) {
			RAWINPUT*	raw = (RAWINPUT*) pRawBuffer;
			if (raw->header.dwType == RIM_TYPEHID) {
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
	if (dwSize > 21) {
		pRawBuffer = DNew BYTE[dwSize];
		if (GetRawInputData(hRawInput, RID_INPUT, pRawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != -1) {
			RAWINPUT*	raw = (RAWINPUT*) pRawBuffer;

			// data.hid.bRawData[21] set to one when key is pressed
			if (raw->header.dwType == RIM_TYPEHID && raw->data.hid.bRawData[21] == 1) {
				// data.hid.bRawData[21] has keycode
				switch (raw->data.hid.bRawData[20]) {
					case 0x0033 :
						nMceCmd = MCE_DETAILS;
						break;
					case 0x0022 :
						nMceCmd = MCE_GUIDE;
						break;
					case 0x0036 :
						nMceCmd = MCE_MYTV;
						break;
					case 0x0026 :
						nMceCmd = MCE_RECORDEDTV;
						break;
					case 0x0005 :
						nMceCmd = MCE_RED;
						break;
					case 0x0002 :
						nMceCmd = MCE_GREEN;
						break;
					case 0x0045 :
						nMceCmd = MCE_YELLOW;
						break;
					case 0x0046 :
						nMceCmd = MCE_BLUE;
						break;
					case 0x000A :
						nMceCmd = MCE_MEDIA_PREVIOUSTRACK;
						break;
					case 0x004A :
						nMceCmd = MCE_MEDIA_NEXTTRACK;
						break;
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
	RAWINPUTDEVICE		MCEInputDevice[] = {
		//	usUsagePage		usUsage			dwFlags		hwndTarget
		{	0xFFBC,			0x88,				0,		NULL},
		{	0x000C,			0x01,				0,		NULL},
		{	0x000C,			0x80,				0,		NULL}
	};

	// Register MCE Remote Control raw input
	for (int i=0; i<countof(MCEInputDevice); i++) {
		MCEInputDevice[i].hwndTarget = m_pMainWnd->m_hWnd;
	}

	nInputDeviceCount = GetRawInputDeviceList (InputDeviceList, &nInputDeviceCount, sizeof(RAWINPUTDEVICELIST));
	for (int i=0; i<nInputDeviceCount; i++) {
		UINT	nTemp = sizeof(DevInfo);

		if (GetRawInputDeviceInfo (InputDeviceList[i].hDevice, RIDI_DEVICEINFO, &DevInfo, &nTemp)>0) {
			if (DevInfo.hid.dwVendorId == 0x00000471 &&			// Philips HID vendor id
					DevInfo.hid.dwProductId == 0x00000617) {	// IEEE802.15.4 RF Dongle (SRM 7500)
				MCEInputDevice[0].usUsagePage	= DevInfo.hid.usUsagePage;
				MCEInputDevice[0].usUsage		= DevInfo.hid.usUsage;
				GetRemoteControlCode = GetRemoteControlCodeSRM7500;
			}
		}
	}


	RegisterRawInputDevices (MCEInputDevice, countof(MCEInputDevice), sizeof(RAWINPUTDEVICE));


	if (m_s.fGlobalMedia) {
		POSITION pos = m_s.wmcmds.GetHeadPosition();

		while (pos) {
			wmcmd& wc = m_s.wmcmds.GetNext(pos);
			if (wc.appcmd != 0) {
				RegisterHotKey(m_pMainWnd->m_hWnd, wc.appcmd, 0, GetVKFromAppCommand (wc.appcmd));
			}
		}
	}
}

void CMPlayerCApp::UnregisterHotkeys()
{
	if (m_s.fGlobalMedia) {
		POSITION pos = m_s.wmcmds.GetHeadPosition();

		while (pos) {
			wmcmd& wc = m_s.wmcmds.GetNext(pos);
			if (wc.appcmd != 0) {
				UnregisterHotKey(m_pMainWnd->m_hWnd, wc.appcmd);
			}
		}
	}
}

UINT CMPlayerCApp::GetVKFromAppCommand(UINT nAppCommand)
{
	switch (nAppCommand) {
		case APPCOMMAND_BROWSER_BACKWARD	:
			return VK_BROWSER_BACK;
		case APPCOMMAND_BROWSER_FORWARD		:
			return VK_BROWSER_FORWARD;
		case APPCOMMAND_BROWSER_REFRESH		:
			return VK_BROWSER_REFRESH;
		case APPCOMMAND_BROWSER_STOP		:
			return VK_BROWSER_STOP;
		case APPCOMMAND_BROWSER_SEARCH		:
			return VK_BROWSER_SEARCH;
		case APPCOMMAND_BROWSER_FAVORITES	:
			return VK_BROWSER_FAVORITES;
		case APPCOMMAND_BROWSER_HOME		:
			return VK_BROWSER_HOME;
		case APPCOMMAND_VOLUME_MUTE			:
			return VK_VOLUME_MUTE;
		case APPCOMMAND_VOLUME_DOWN			:
			return VK_VOLUME_DOWN;
		case APPCOMMAND_VOLUME_UP			:
			return VK_VOLUME_UP;
		case APPCOMMAND_MEDIA_NEXTTRACK		:
			return VK_MEDIA_NEXT_TRACK;
		case APPCOMMAND_MEDIA_PREVIOUSTRACK	:
			return VK_MEDIA_PREV_TRACK;
		case APPCOMMAND_MEDIA_STOP			:
			return VK_MEDIA_STOP;
		case APPCOMMAND_MEDIA_PLAY_PAUSE	:
			return VK_MEDIA_PLAY_PAUSE;
		case APPCOMMAND_LAUNCH_MAIL			:
			return VK_LAUNCH_MAIL;
		case APPCOMMAND_LAUNCH_MEDIA_SELECT	:
			return VK_LAUNCH_MEDIA_SELECT;
		case APPCOMMAND_LAUNCH_APP1			:
			return VK_LAUNCH_APP1;
		case APPCOMMAND_LAUNCH_APP2			:
			return VK_LAUNCH_APP2;
	}

	return 0;
}

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

	if (m_nStatus == CONNECTING && m_addr == addr) {
		TRACE(_T("CRemoteCtrlClient (Connect): already connecting to %s\n"), addr);
		return;
	}

	if (m_nStatus == CONNECTED && m_addr == addr) {
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

void CRemoteCtrlClient::DisConnect()
{
	CAutoLock cAutoLock(&m_csLock);

	ShutDown(2);
	Close();
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

	if (m_hSocket != INVALID_SOCKET && m_nStatus == CONNECTED) {
		TRACE(_T("CRemoteCtrlClient (OnClose): connection lost\n"));
	}

	m_nStatus = DISCONNECTED;

	TRACE(_T("CRemoteCtrlClient (OnClose): %d\n"), nErrorCode);
}

void CRemoteCtrlClient::OnReceive(int nErrorCode)
{
	if (nErrorCode != 0 || !m_pWnd) {
		return;
	}

	CStringA str;
	int ret = Receive(str.GetBuffer(256), 255, 0);
	if (ret <= 0) {
		return;
	}
	str.ReleaseBuffer(ret);

	TRACE(_T("CRemoteCtrlClient (OnReceive): %s\n"), CString(str));

	OnCommand(str);

	__super::OnReceive(nErrorCode);
}

void CRemoteCtrlClient::ExecuteCommand(CStringA cmd, int repcnt)
{
	cmd.Trim();
	if (cmd.IsEmpty()) {
		return;
	}
	cmd.Replace(' ', '_');

	CAppSettings& s = AfxGetAppSettings();

	POSITION pos = s.wmcmds.GetHeadPosition();
	while (pos) {
		wmcmd wc = s.wmcmds.GetNext(pos);
		CStringA name = TToA(wc.GetName());
		name.Replace(' ', '_');
		if ((repcnt == 0 && wc.rmrepcnt == 0 || wc.rmrepcnt > 0 && (repcnt%wc.rmrepcnt) == 0)
				&& (!name.CompareNoCase(cmd) || !wc.rmcmd.CompareNoCase(cmd) || wc.cmd == (WORD)strtol(cmd, NULL, 10))) {
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
	for (CStringA token = str.Tokenize(" ", i);
			!token.IsEmpty();
			token = str.Tokenize(" ", i), j++) {
		if (j == 1) {
			repcnt = strtol(token, NULL, 16);
		} else if (j == 2) {
			ExecuteCommand(token, repcnt);
		}
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
	for (CStringA token = str.Tokenize("|", i);
			!token.IsEmpty();
			token = str.Tokenize("|", i), j++) {
		if (j == 0) {
			cmd = token;
		} else if (j == 1) {
			ExecuteCommand(cmd, strtol(token, NULL, 16));
		}
	}
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
	if ((DisplayName == _T("Current")) || (DisplayName == _T(""))) {
		CMonitor monitor;
		CMonitors monitors;
		monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
		monitor.GetName(DisplayName1);
	}
	hDC = CreateDC(DisplayName1, NULL, NULL, NULL);
	if (hDC) {
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
	if ((DisplayName == _T("Current")) || (DisplayName == _T(""))) {
		CMonitor monitor;
		CMonitors monitors;
		monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
		monitor.GetName(DisplayName1);
	}
	if (!EnumDisplaySettings(DisplayName1, i, &devmode)) {
		return(false);
	}
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
	if ((dm.size == dm1.size) && (dm.bpp == dm1.bpp) && (dm.freq == dm1.freq)) {
		return;
	}

	if (!dm.fValid) {
		return;
	}
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
	if ((DisplayName == _T("Current")) || (DisplayName == _T(""))) {
		CMonitor monitor;
		CMonitors monitors;
		monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
		monitor.GetName(DisplayName1);
	}
	if (AfxGetAppSettings().fRestoreResAfterExit) {
		ChangeDisplaySettingsEx(DisplayName1, &dmScreenSettings, NULL, CDS_FULLSCREEN, NULL);
	} else {
		ChangeDisplaySettingsEx(DisplayName1, &dmScreenSettings, NULL, NULL, NULL);
	}
}

void SetAudioRenderer(int AudioDevNo)
{
	CStringArray m_AudioRendererDisplayNames;
	AfxGetMyApp()->m_AudioRendererDisplayName_CL = _T("");
	m_AudioRendererDisplayNames.Add(_T(""));
	int i=2;

	BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker) {
		LPOLESTR olestr = NULL;
		if (FAILED(pMoniker->GetDisplayName(0, 0, &olestr))) {
			continue;
		}
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
	if (AudioDevNo>=1 && AudioDevNo<=i) {
		AfxGetMyApp()->m_AudioRendererDisplayName_CL = m_AudioRendererDisplayNames[AudioDevNo-1];
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
	while (pos) {
		CAtlRegExpT* re = res.GetNext(pos);

		CAtlREMatchContextT mc;
		const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
		const CAtlREMatchContextT::RECHAR* e = NULL;
		for (; s && re->Match(s, &mc, &e); s = e) {
			const CAtlREMatchContextT::RECHAR* szStart = 0;
			const CAtlREMatchContextT::RECHAR* szEnd = 0;
			mc.GetMatch(0, &szStart, &szEnd);

			CString url;
			url.Format(_T("%.*s"), szEnd - szStart, szStart);
			url.Trim();

			if (url.CompareNoCase(_T("asf path")) == 0) {
				continue;
			}

			CUrl dst;
			dst.CrackUrl(CString(url));
			if (_tcsicmp(src.GetSchemeName(), dst.GetSchemeName())
					|| _tcsicmp(src.GetHostName(), dst.GetHostName())
					|| _tcsicmp(src.GetUrlPath(), dst.GetUrlPath())) {
				urls.AddTail(url);
			} else {
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
	if (f.Open(fn)) for (CString tmp; f.ReadString(tmp); body += tmp + '\n') {
			;
		}

	CString dir = fn.Left(max(fn.ReverseFind('/'), fn.ReverseFind('\\'))+1); // "ReverseFindOneOf"

	POSITION pos = res.GetHeadPosition();
	while (pos) {
		CAtlRegExpT* re = res.GetNext(pos);

		CAtlREMatchContextT mc;
		const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
		const CAtlREMatchContextT::RECHAR* e = NULL;
		for (; s && re->Match(s, &mc, &e); s = e) {
			const CAtlREMatchContextT::RECHAR* szStart = 0;
			const CAtlREMatchContextT::RECHAR* szEnd = 0;
			mc.GetMatch(0, &szStart, &szEnd);

			CString fn2;
			fn2.Format(_T("%.*s"), szEnd - szStart, szStart);
			fn2.Trim();

			if (!fn2.CompareNoCase(_T("asf path"))) {
				continue;
			}
			if (fn2.Find(_T("EXTM3U")) == 0 || fn2.Find(_T("#EXTINF")) == 0) {
				continue;
			}

			if (fn2.Find(_T(":")) < 0 && fn2.Find(_T("\\\\")) != 0 && fn2.Find(_T("//")) != 0) {
				CPath p;
				p.Combine(dir, fn2);
				fn2 = (LPCTSTR)p;
			}

			if (!fn2.CompareNoCase(fn)) {
				continue;
			}

			fns.AddTail(fn2);
		}
	}

	return fns.GetCount() > 0;
}

CStringA GetContentType(CString fn, CAtlList<CString>* redir)
{
	CUrl url;
	CString ct, body;

	if (fn.Find(_T("://")) >= 0) {
		url.CrackUrl(fn);

		if (_tcsicmp(url.GetSchemeName(), _T("pnm")) == 0) {
			return "audio/x-pn-realaudio";
		}

		if (_tcsicmp(url.GetSchemeName(), _T("mms")) == 0) {
			return "video/x-ms-asf";
		}

		if (_tcsicmp(url.GetSchemeName(), _T("http")) != 0) {
			return "";
		}

		DWORD ProxyEnable = 0;
		CString ProxyServer;
		DWORD ProxyPort = 0;

		ULONG len = 256+1;
		CRegKey key;
		if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
				&& ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
				&& ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer.GetBufferSetLength(256), &len)) {
			ProxyServer.ReleaseBufferSetLength(len);

			CAtlList<CString> sl;
			ProxyServer = Explode(ProxyServer, sl, ';');
			if (sl.GetCount() > 1) {
				POSITION pos = sl.GetHeadPosition();
				while (pos) {
					CAtlList<CString> sl2;
					if (!Explode(sl.GetNext(pos), sl2, '=', 2).CompareNoCase(_T("http"))
							&& sl2.GetCount() == 2) {
						ProxyServer = sl2.GetTail();
						break;
					}
				}
			}

			ProxyServer = Explode(ProxyServer, sl, ':');
			if (sl.GetCount() > 1) {
				ProxyPort = _tcstol(sl.GetTail(), NULL, 10);
			}
		}

		CSocket s;
		s.Create();
		if (s.Connect(
					ProxyEnable ? ProxyServer : url.GetHostName(),
					ProxyEnable ? ProxyPort : url.GetPortNumber())) {
			CStringA host = CStringA(url.GetHostName());
			CStringA path = CStringA(url.GetUrlPath()) + CStringA(url.GetExtraInfo());

			if (ProxyEnable) {
				path = "http://" + host + path;
			}

			CStringA hdr;
			hdr.Format(
				"GET %s HTTP/1.0\r\n"
				"User-Agent: Media Player Classic\r\n"
				"Host: %s\r\n"
				"Accept: */*\r\n"
				"\r\n", path, host);

			// MessageBox(NULL, CString(hdr), _T("Sending..."), MB_OK);

			if (s.Send((LPCSTR)hdr, hdr.GetLength()) < hdr.GetLength()) {
				return "";
			}

			hdr.Empty();
			while (1) {
				CStringA str;
				str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
				if (str.IsEmpty()) {
					break;
				}
				hdr += str;
				int hdrend = hdr.Find("\r\n\r\n");
				if (hdrend >= 0) {
					body = hdr.Mid(hdrend+4);
					hdr = hdr.Left(hdrend);
					break;
				}
			}

			// MessageBox(NULL, CString(hdr), _T("Received..."), MB_OK);

			CAtlList<CStringA> sl;
			Explode(hdr, sl, '\n');
			POSITION pos = sl.GetHeadPosition();
			while (pos) {
				CStringA& hdrline = sl.GetNext(pos);
				CAtlList<CStringA> sl2;
				Explode(hdrline, sl2, ':', 2);
				CStringA field = sl2.RemoveHead().MakeLower();
				if (field == "location" && !sl2.IsEmpty()) {
					return GetContentType(CString(sl2.GetHead()), redir);
				}
				if (field == "content-type" && !sl2.IsEmpty()) {
					ct = sl2.GetHead();
				}
			}

			while (body.GetLength() < 256) {
				CStringA str;
				str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
				if (str.IsEmpty()) {
					break;
				}
				body += str;
			}

			if (body.GetLength() >= 8) {
				CStringA str = TToA(body);
				if (!strncmp((LPCSTR)str, ".ra", 3)) {
					return "audio/x-pn-realaudio";
				}
				if (!strncmp((LPCSTR)str, ".RMF", 4)) {
					return "audio/x-pn-realaudio";
				}
				if (*(DWORD*)(LPCSTR)str == 0x75b22630) {
					return "video/x-ms-wmv";
				}
				if (!strncmp((LPCSTR)str+4, "moov", 4)) {
					return "video/quicktime";
				}
			}

			if (redir && (ct == _T("audio/x-scpls") || ct == _T("audio/x-mpegurl"))) {
				while (body.GetLength() < 4*1024) { // should be enough for a playlist...
					CStringA str;
					str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
					if (str.IsEmpty()) {
						break;
					}
					body += str;
				}
			}
		}
	} else if (!fn.IsEmpty()) {
		CPath p(fn);
		CString ext = p.GetExtension().MakeLower();
		if (ext == _T(".asx")) {
			ct = _T("video/x-ms-asf");
		} else if (ext == _T(".pls")) {
			ct = _T("audio/x-scpls");
		} else if (ext == _T(".m3u") || ext == _T(".m3u8")) {
			ct = _T("audio/x-mpegurl");
		} else if (ext == _T(".qtl")) {
			ct = _T("application/x-quicktimeplayer");
		} else if (ext == _T(".mpcpl")) {
			ct = _T("application/x-mpc-playlist");
		} else if (ext == _T(".bdmv")) {
			ct = _T("application/x-bdmv-playlist");
		}

		if (FILE* f = _tfopen(fn, _T("rb"))) {
			CStringA str;
			str.ReleaseBufferSetLength(fread(str.GetBuffer(10240), 1, 10240, f));
			body = AToT(str);
			fclose(f);
		}
	}

	if (body.GetLength() >= 4) { // here only those which cannot be opened through dshow
		CStringA str = TToA(body);
		if (!strncmp((LPCSTR)str, ".ra", 3)) {
			return "audio/x-pn-realaudio";
		}
		if (!strncmp((LPCSTR)str, "FWS", 3)) {
			return "application/x-shockwave-flash";
		}

	}

	if (redir && !ct.IsEmpty()) {
		CAutoPtrList<CAtlRegExpT> res;
		CAutoPtr<CAtlRegExpT> re;

		if (ct == _T("video/x-ms-asf")) {
			// ...://..."/>
			re.Attach(DNew CAtlRegExpT());
			if (re && REPARSE_ERROR_OK == re->Parse(_T("{[a-zA-Z]+://[^\n\">]*}"), FALSE)) {
				res.AddTail(re);
			}
			// Ref#n= ...://...\n
			re.Attach(DNew CAtlRegExpT());
			if (re && REPARSE_ERROR_OK == re->Parse(_T("Ref\\z\\b*=\\b*[\"]*{([a-zA-Z]+://[^\n\"]+}"), FALSE)) {
				res.AddTail(re);
			}
		} else if (ct == _T("audio/x-scpls")) {
			// File1=...\n
			re.Attach(DNew CAtlRegExp<>());
			if (re && REPARSE_ERROR_OK == re->Parse(_T("file\\z\\b*=\\b*[\"]*{[^\n\"]+}"), FALSE)) {
				res.AddTail(re);
			}
		} else if (ct == _T("audio/x-mpegurl")) {
			// #comment
			// ...
			re.Attach(DNew CAtlRegExp<>());
			if (re && REPARSE_ERROR_OK == re->Parse(_T("{[^#][^\n]+}"), FALSE)) {
				res.AddTail(re);
			}
		} else if (ct == _T("audio/x-pn-realaudio")) {
			// rtsp://...
			re.Attach(DNew CAtlRegExp<>());
			if (re && REPARSE_ERROR_OK == re->Parse(_T("{rtsp://[^\n]+}"), FALSE)) {
				res.AddTail(re);
			}
		}

		if (!body.IsEmpty()) {
			if (fn.Find(_T("://")) >= 0) {
				FindRedir(url, ct, body, *redir, res);
			} else {
				FindRedir(fn, ct, *redir, res);
			}
		}
	}

	return TToA(ct);
}

COLORPROPERTY_RANGE* CMPlayerCApp::GetColorControl(ControlType nFlag)
{
	switch (nFlag) {
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

void CMPlayerCApp::ResetColorControlRange()
{
	m_ColorControl[0].dwProperty	= Brightness;
	m_ColorControl[0].MinValue		= -100;
	m_ColorControl[0].MaxValue		= 100;
	m_ColorControl[0].DefaultValue	= 0;
	m_ColorControl[0].StepSize		= 1;
	m_ColorControl[1].dwProperty	= Contrast;
	m_ColorControl[1].MinValue		= 0;
	m_ColorControl[1].MaxValue		= 200;
	m_ColorControl[1].DefaultValue	= 100;
	m_ColorControl[1].StepSize		= 1;
	m_ColorControl[2].dwProperty	= Hue;
	m_ColorControl[2].MinValue		= -180;
	m_ColorControl[2].MaxValue		= 180;
	m_ColorControl[2].DefaultValue	= 0;
	m_ColorControl[2].StepSize		= 1;
	m_ColorControl[3].dwProperty	= Saturation;
	m_ColorControl[3].MinValue		= 0;
	m_ColorControl[3].MaxValue		= 200;
	m_ColorControl[3].DefaultValue	= 100;
	m_ColorControl[3].StepSize		= 1;
}

void CMPlayerCApp::UpdateColorControlRange(bool isEVR)
{
	if (isEVR) {
		// Brightness
		m_ColorControl[0].MinValue		= FixedToInt(m_EVRColorControl[0].MinValue);
		m_ColorControl[0].MaxValue		= FixedToInt(m_EVRColorControl[0].MaxValue);
		m_ColorControl[0].DefaultValue	= FixedToInt(m_EVRColorControl[0].DefaultValue);
		m_ColorControl[0].StepSize		= max(1, FixedToInt(m_EVRColorControl[0].StepSize));
		// Contrast
		m_ColorControl[1].MinValue		= FixedToInt(m_EVRColorControl[1].MinValue,100);
		m_ColorControl[1].MaxValue		= FixedToInt(m_EVRColorControl[1].MaxValue,100);
		m_ColorControl[1].DefaultValue	= FixedToInt(m_EVRColorControl[1].DefaultValue,100);
		m_ColorControl[1].StepSize		= max(1, FixedToInt(m_EVRColorControl[1].StepSize,100));
		// Hue
		m_ColorControl[2].MinValue		= FixedToInt(m_EVRColorControl[2].MinValue);
		m_ColorControl[2].MaxValue		= FixedToInt(m_EVRColorControl[2].MaxValue);
		m_ColorControl[2].DefaultValue	= FixedToInt(m_EVRColorControl[2].DefaultValue);
		m_ColorControl[2].StepSize		= max(1, FixedToInt(m_EVRColorControl[2].StepSize));
		// Saturation
		m_ColorControl[3].MinValue		= FixedToInt(m_EVRColorControl[3].MinValue,100);
		m_ColorControl[3].MaxValue		= FixedToInt(m_EVRColorControl[3].MaxValue,100);
		m_ColorControl[3].DefaultValue	= FixedToInt(m_EVRColorControl[3].DefaultValue,100);
		m_ColorControl[3].StepSize		= max(1, FixedToInt(m_EVRColorControl[3].StepSize,100));
	}
	else {
		// Brightness
		m_ColorControl[0].MinValue		= (int)floor(m_VMR9ColorControl[0].MinValue+0.5);
		m_ColorControl[0].MaxValue		= (int)floor(m_VMR9ColorControl[0].MaxValue+0.5);
		m_ColorControl[0].DefaultValue	= (int)floor(m_VMR9ColorControl[0].DefaultValue+0.5);
		m_ColorControl[0].StepSize		= max(1,(int)(m_VMR9ColorControl[0].StepSize+0.5));
		// Contrast
		//if(m_VMR9ColorControl[1].MinValue == 0.0999908447265625) m_VMR9ColorControl[1].MinValue = 0.11; //fix nvidia bug
		if(*(int*)&m_VMR9ColorControl[1].MinValue == 1036830720) m_VMR9ColorControl[1].MinValue = 0.11f; //fix nvidia bug
		m_ColorControl[1].MinValue		= (int)floor(m_VMR9ColorControl[1].MinValue*100+0.5);
		m_ColorControl[1].MaxValue		= (int)floor(m_VMR9ColorControl[1].MaxValue*100+0.5);
		m_ColorControl[1].DefaultValue	= (int)floor(m_VMR9ColorControl[1].DefaultValue*100+0.5);
		m_ColorControl[1].StepSize		= max(1, (int)(m_VMR9ColorControl[1].StepSize*100+0.5));
		// Hue
		m_ColorControl[2].MinValue		= (int)floor(m_VMR9ColorControl[2].MinValue+0.5);
		m_ColorControl[2].MaxValue		= (int)floor(m_VMR9ColorControl[2].MaxValue+0.5);
		m_ColorControl[2].DefaultValue	= (int)floor(m_VMR9ColorControl[2].DefaultValue+0.5);
		m_ColorControl[2].StepSize		= max(1,(int)(m_VMR9ColorControl[2].StepSize+0.5));
		// Saturation
		m_ColorControl[3].MinValue		= (int)floor(m_VMR9ColorControl[3].MinValue*100+0.5);
		m_ColorControl[3].MaxValue		= (int)floor(m_VMR9ColorControl[3].MaxValue*100+0.5);
		m_ColorControl[3].DefaultValue	= (int)floor(m_VMR9ColorControl[3].DefaultValue*100+0.5);
		m_ColorControl[3].StepSize		= max(1, (int)(m_VMR9ColorControl[3].StepSize*100+0.5));
	}
}

VMR9ProcAmpControlRange* CMPlayerCApp::GetVMR9ColorControl(ControlType nFlag)
{
	switch (nFlag) {
		case Brightness :
			return &m_VMR9ColorControl[0];
		case Contrast :
			return &m_VMR9ColorControl[1];
		case Hue :
			return &m_VMR9ColorControl[2];
		case Saturation :
			return &m_VMR9ColorControl[3];
	}
	return NULL;
}

DXVA2_ValueRange* CMPlayerCApp::GetEVRColorControl(ControlType nFlag)
{
	switch (nFlag) {
		case Brightness :
			return &m_EVRColorControl[0];
		case Contrast :
			return &m_EVRColorControl[1];
		case Hue :
			return &m_EVRColorControl[2];
		case Saturation :
			return &m_EVRColorControl[3];
	}
	return NULL;
}

LPCTSTR CMPlayerCApp::GetSatelliteDll(int nLanguage)
{
	switch (nLanguage) {
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
		case 13 :	// Chinese (Simplified)
			return _T("mpcresources.sc.dll");
		case 14 :	// Chinese (Traditional)
			return _T("mpcresources.tc.dll");
		case 15 :	// Belarusian
			return _T("mpcresources.by.dll");
		case 16 :	// Swedish
			return _T("mpcresources.sv.dll");
		case 17 :	// Portuguese (Brasil)
			return _T("mpcresources.br.dll");
		case 18 :	// Dutch
			return _T("mpcresources.nl.dll");
		case 19 :	// Catalan
			return _T("mpcresources.ca.dll");
		case 20 :	// Japanese
			return _T("mpcresources.ja.dll");
		case 21 :	// Armenian
			return _T("mpcresources.hy.dll");
		case 22 :	// Hebrew
			return _T("mpcresources.he.dll");
	}
	return NULL;
}

int CMPlayerCApp::GetDefLanguage()
{
	switch (GetUserDefaultUILanguage()) {
		case 1036:	// French
			return 1;
		case 1031:	// German
			return 2;
		case 1049:	// Russian
			return 3;
		case 1055:	// Turkish
			return 4;
		case 1029:	// Czech
			return 5;
		case 1034:	// Spanish
			return 6;
		case 1038:	// Hungarian
			return 7;
		case 1042:	// Korean
			return 8;
		case 1045:	// Polish
			return 9;
		case 1058:	// Ukrainian
			return 10;
		case 1040:	// Italian
			return 11;
		case 1051 :	// Slovak
			return 12;
		case 2052 :	// Chinese (simplified)
			return 13;
		case 3076 :	// Chinese (traditional)
			return 14;
		case 1059 :	// Belarusian
			return 15;
		case 1053 :	// Swedish
			return 16;
		case 1046 :	// Portuguese (brasil)
			return 17;
		case 1043 :	// Dutch
			return 18;
		case 1027 :	// Catalan
			return 19;
		case 1041 :	// Japanese
			return 20;
		case 1067 : // Armenian
			return 21;
		case 1037 : // Hebrew
			return 22;
		default:
			return 0;
	}
}

LRESULT CALLBACK RTLWindowsLayoutCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HCBT_CREATEWND)
	{
		//LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;

		//if ((lpcs->style & WS_CHILD) == 0)
		//	lpcs->dwExStyle |= WS_EX_LAYOUTRTL;	// doesn't seem to have any effect, but shouldn't hurt

		HWND hWnd = (HWND)wParam;
		if ((GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) == 0) {
			SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

void CMPlayerCApp::SetLanguage (int nLanguage)
{
	AppSettings&	s = AfxGetAppSettings();
	HMODULE			hMod = NULL;
	LPCTSTR			strSatellite;

	strSatellite = GetSatelliteDll( nLanguage );
	if ( strSatellite ) {
		CFileVersionInfo	Version;
		CString				strSatVersion;

		if ( Version.Create(strSatellite) ) {
			strSatVersion = Version.GetFileVersionEx();

			CString needVersion = L"";
			needVersion += MAKE_STR(MPC_VERSION_MAJOR);
			needVersion += L".";
			needVersion += MAKE_STR(MPC_VERSION_MINOR);
			needVersion += L".";
			needVersion += MAKE_STR(MPC_VERSION_PATCH);
			needVersion += L".0";

			if ( strSatVersion == needVersion ) {
				hMod = LoadLibrary( strSatellite );
				s.iLanguage = nLanguage;
			} else {
				// This message should stay in English!
				MessageBox(NULL, _T("Your language pack will not work with this version. Please download a compatible one from the MPC-HC homepage."),
						   _T("Media Player Classic - Home Cinema"), MB_OK);
			}
		}
	}

	if ( hMod == NULL ) {
		hMod = AfxGetApp()->m_hInstance;
		s.iLanguage = 0;
	}
	if (AfxGetResourceHandle() != AfxGetApp()->m_hInstance) {
		FreeLibrary(AfxGetResourceHandle());
	}
	AfxSetResourceHandle( hMod );

	// Hebrew needs the RTL flag.
	SetProcessDefaultLayout((nLanguage == 22) ? LAYOUT_RTL : LAYOUT_LTR);
	/*
	// Something like this is needed to have the options dialog RTLed
	// but it currently totally breaks the layout ...
	if (nLanguage == 22) {
		SetWindowsHookEx(WH_CBT, RTLWindowsLayoutCbtFilterHook, NULL, GetCurrentThreadId());
	}
	*/
}

bool CMPlayerCApp::IsVSFilterInstalled()
{
	bool result = false;
	CRegKey key;
	if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\{9852A670-F845-491B-9BE6-EBD841B8A613}"), KEY_READ)) {
		result = true;
	}

	return result;
}

bool CMPlayerCApp::HasEVR()
{
	bool result = false;
	CRegKey key;
	if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\{FA10746C-9B63-4B6C-BC49-FC300EA5F256}"), KEY_READ)) {
		result = true;
	}

	return result;
}

HRESULT CMPlayerCApp::GetElevationType(TOKEN_ELEVATION_TYPE* ptet )
{
	ASSERT( IsWinVistaOrLater() );
	ASSERT( ptet );

	HRESULT hResult = E_FAIL; // assume an error occurred
	HANDLE hToken	= NULL;

	if ( !::OpenProcessToken(
				::GetCurrentProcess(),
				TOKEN_QUERY,
				&hToken ) ) {
		ASSERT( FALSE );
		return hResult;
	}

	DWORD dwReturnLength = 0;

	if ( !::GetTokenInformation(
				hToken,
				TokenElevationType,
				ptet,
				sizeof( *ptet ),
				&dwReturnLength ) ) {
		ASSERT( FALSE );
	} else {
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

	if (bWaitProcess) {
		WaitForSingleObject(execinfo.hProcess, INFINITE);
	}
}

void CAboutDlg::OnHomepage(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(m_hWnd, _T("open"), _T("http://mpc-hc.sourceforge.net/"), NULL, NULL, SW_SHOWDEFAULT);
	*pResult = 0;
}

CRenderersData*		GetRenderersData()
{
	return &AfxGetMyApp()->m_Renderers;
}

CRenderersSettings&	GetRenderersSettings()
{
	return AfxGetAppSettings().m_RenderersSettings;
}
