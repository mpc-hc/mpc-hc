/*
 * (C) 2012-2017 see Authors.txt
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
#include "AboutDlg.h"
#include "mpc-hc_config.h"
#ifndef MPCHC_LITE
#include "FGFilterLAV.h"
#endif
#include "mplayerc.h"
#include "FileVersionInfo.h"
#include "PathUtils.h"
#include "VersionInfo.h"
#include "WinapiFunc.h"
#include <afxole.h>

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

CAboutDlg::CAboutDlg() : CMPCThemeDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

CAboutDlg::~CAboutDlg() {
}

BOOL CAboutDlg::OnInitDialog()
{
    // Get the default text before it is overwritten by the call to __super::OnInitDialog()
    GetDlgItem(IDC_AUTHORS_LINK)->GetWindowText(m_credits);
#ifndef MPCHC_LITE
    GetDlgItem(IDC_LAVFILTERS_VERSION)->GetWindowText(m_LAVFiltersVersion);
#endif

    __super::OnInitDialog();

    // Because we set LR_SHARED, there is no need to explicitly destroy the icon
    m_icon.SetIcon((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 48, 48, LR_SHARED));

    m_appname = _T("MPC-HC");
    if (VersionInfo::IsNightly() || VersionInfo::Is64Bit()) {
        m_appname += _T(" (");
    }
    if (VersionInfo::IsNightly()) {
        m_appname += VersionInfo::GetNightlyWord();
    }
    if (VersionInfo::IsNightly() && VersionInfo::Is64Bit()) {
        m_appname += _T(", ");
    }
    if (VersionInfo::Is64Bit()) {
        m_appname += _T("64-bit");
    }
    if (VersionInfo::IsNightly() || VersionInfo::Is64Bit()) {
        m_appname += _T(")");
    }

#ifdef MPCHC_LITE
    m_appname += _T(" Lite");
#endif

    // Build the path to Authors.txt
    m_AuthorsPath = PathUtils::CombinePaths(PathUtils::GetProgramPath(), _T("Authors.txt"));
    // Check if the file exists
    if (PathUtils::Exists(m_AuthorsPath)) {
        // If it does, we make the filename clickable
        m_credits.Replace(_T("Authors.txt"), _T("<a>Authors.txt</a>"));
    }

    m_homepage.Format(_T("<a>%s</a>"), WEBSITE_URL);

    m_strBuildNumber = VersionInfo::GetFullVersionString();

#if defined(__INTEL_COMPILER)
#if (__INTEL_COMPILER >= 1210)
    m_MPCCompiler = _T("ICL ") MAKE_STR(__INTEL_COMPILER) _T(" Build ") MAKE_STR(__INTEL_COMPILER_BUILD_DATE);
#else
#error Compiler is not supported!
#endif
#elif defined(_MSC_VER)
#if (_MSC_VER >= 1910)
    m_MPCCompiler.Format(_T("MSVC v%.2d.%.2d.%.5d"), _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000);
#if _MSC_BUILD
    m_MPCCompiler.AppendFormat(_T(".%.2d"), _MSC_BUILD);
#endif
#elif (_MSC_VER == 1900)                // 2015
#if (_MSC_FULL_VER >= 190024210 && _MSC_FULL_VER <= 190024218)
    m_MPCCompiler = _T("MSVC 2015 Update 3");
#elif (_MSC_FULL_VER == 190023918)
    m_MPCCompiler = _T("MSVC 2015 Update 2");
#elif (_MSC_FULL_VER == 190023506)
    m_MPCCompiler = _T("MSVC 2015 Update 1");
#elif (_MSC_FULL_VER == 190023026)
    m_MPCCompiler = _T("MSVC 2015");
#else
    m_MPCCompiler.Format(_T("MSVC v%.2d.%.2d.%.5d"), _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000);
#if _MSC_BUILD
    m_MPCCompiler.AppendFormat(_T(".%.2d"), _MSC_BUILD);
#endif
#endif
#elif (_MSC_VER <= 1800)
#error Compiler is not supported!
#endif
#else
#error Please add support for your compiler
#endif

#if (__AVX2__)
    m_MPCCompiler += _T(" (AVX2)");
#elif (__AVX__)
    m_MPCCompiler += _T(" (AVX)");
#elif (__SSSE3__)
    m_MPCCompiler += _T(" (SSSE3)");
#elif (__SSE3__)
    m_MPCCompiler += _T(" (SSE3)");
#elif !defined(_M_X64) && defined(_M_IX86_FP)
#if (_M_IX86_FP == 2)   // /arch:SSE2 was used
    m_MPCCompiler += _T(" (SSE2)");
#elif (_M_IX86_FP == 1) // /arch:SSE was used
    m_MPCCompiler += _T(" (SSE)");
#endif
#endif

#ifdef _DEBUG
    m_MPCCompiler += _T(" Debug");
#endif

    m_LAVFilters.Format(IDS_STRING_COLON, _T("LAV Filters"));
#ifndef MPCHC_LITE
    CString LAVFiltersVersion = CFGFilterLAV::GetVersion();
    if (!LAVFiltersVersion.IsEmpty()) {
        m_LAVFiltersVersion = LAVFiltersVersion;
    }
#endif

    m_buildDate = VersionInfo::GetBuildDateString();

#pragma warning(push)
#pragma warning(disable: 4996)
    OSVERSIONINFOEX osVersion = { sizeof(OSVERSIONINFOEX) };
    GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&osVersion));
#pragma warning(pop)

    m_OSName.Format(_T("Windows NT %1u.%1u (build %u"),
                    osVersion.dwMajorVersion, osVersion.dwMinorVersion, osVersion.dwBuildNumber);
    if (osVersion.szCSDVersion[0]) {
        m_OSName.AppendFormat(_T(", %s)"), osVersion.szCSDVersion);
    } else {
        m_OSName += _T(")");
    }
    m_OSVersion.Format(_T("%1u.%1u"), osVersion.dwMajorVersion, osVersion.dwMinorVersion);

#if !defined(_WIN64)
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = FALSE;
    if (IsWow64Process(GetCurrentProcess(), &f64) && f64)
#endif
    {
        m_OSVersion += _T(" (64-bit)");
    }

    UpdateData(FALSE);

    GetDlgItem(IDOK)->SetFocus();
    fulfillThemeReqs();

    return FALSE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDR_MAINFRAME, m_icon);
    DDX_Text(pDX, IDC_STATIC1, m_appname);
    DDX_Text(pDX, IDC_AUTHORS_LINK, m_credits);
    DDX_Text(pDX, IDC_HOMEPAGE_LINK, m_homepage);
    DDX_Text(pDX, IDC_VERSION, m_strBuildNumber);
    DDX_Text(pDX, IDC_MPC_COMPILER, m_MPCCompiler);
    DDX_Text(pDX, IDC_STATIC5, m_LAVFilters);
#ifndef MPCHC_LITE
    DDX_Text(pDX, IDC_LAVFILTERS_VERSION, m_LAVFiltersVersion);
#endif
    DDX_Text(pDX, IDC_STATIC2, m_buildDate);
    DDX_Text(pDX, IDC_STATIC3, m_OSName);
    DDX_Text(pDX, IDC_STATIC4, m_OSVersion);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CMPCThemeDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    // No message handlers
    //}}AFX_MSG_MAP
    ON_NOTIFY(NM_CLICK, IDC_HOMEPAGE_LINK, OnHomepage)
    ON_NOTIFY(NM_CLICK, IDC_AUTHORS_LINK, OnAuthors)
    ON_BN_CLICKED(IDC_BUTTON1, OnCopyToClipboard)
END_MESSAGE_MAP()

void CAboutDlg::OnHomepage(NMHDR* pNMHDR, LRESULT* pResult)
{
    ShellExecute(m_hWnd, _T("open"), WEBSITE_URL, nullptr, nullptr, SW_SHOWDEFAULT);
    *pResult = 0;
}

void CAboutDlg::OnAuthors(NMHDR* pNMHDR, LRESULT* pResult)
{
    ShellExecute(m_hWnd, _T("open"), m_AuthorsPath, nullptr, nullptr, SW_SHOWDEFAULT);
    *pResult = 0;
}

void CAboutDlg::OnCopyToClipboard()
{
    CStringW info = m_appname + _T("\r\n");
    info += CString(_T('-'), m_appname.GetLength()) + _T("\r\n\r\n");
    info += _T("Build information:\r\n");
    info += _T("    Version:            ") + m_strBuildNumber + _T("\r\n");
    info += _T("    Compiler:           ") + m_MPCCompiler + _T("\r\n");
    info += _T("    Build date:         ") + m_buildDate + _T("\r\n\r\n");
#ifndef MPCHC_LITE
    info += _T("LAV Filters:\r\n");
    info += _T("    LAV Splitter:       ") + CFGFilterLAV::GetVersion(CFGFilterLAV::SPLITTER) + _T("\r\n");
    info += _T("    LAV Video:          ") + CFGFilterLAV::GetVersion(CFGFilterLAV::VIDEO_DECODER) + _T("\r\n");
    info += _T("    LAV Audio:          ") + CFGFilterLAV::GetVersion(CFGFilterLAV::AUDIO_DECODER) + _T("\r\n");
    info += _T("    FFmpeg compiler:    ") + VersionInfo::GetGCCVersion() + _T("\r\n\r\n");
#endif
    info += _T("Operating system:\r\n");
    info += _T("    Name:               ") + m_OSName + _T("\r\n");
    info += _T("    Version:            ") + m_OSVersion + _T("\r\n\r\n");

    info += _T("Hardware:\r\n");

    CRegKey key;
    if (key.Open(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), KEY_READ) == ERROR_SUCCESS) {
        ULONG nChars = 0;
        if (key.QueryStringValue(_T("ProcessorNameString"), nullptr, &nChars) == ERROR_SUCCESS) {
            CString cpuName;
            if (key.QueryStringValue(_T("ProcessorNameString"), cpuName.GetBuffer(nChars), &nChars) == ERROR_SUCCESS) {
                cpuName.ReleaseBuffer(nChars);
                cpuName.Trim();
                info.AppendFormat(_T("    CPU:                %s\r\n"), cpuName.GetString());
            }
        }
    }

    IDirect3D9* pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (pD3D9) {
        for (UINT adapter = 0, adapterCount = pD3D9->GetAdapterCount(); adapter < adapterCount; adapter++) {
            D3DADAPTER_IDENTIFIER9 adapterIdentifier;
            if (pD3D9->GetAdapterIdentifier(adapter, 0, &adapterIdentifier) == D3D_OK) {
                CString deviceName = adapterIdentifier.Description;
                deviceName.Trim();

                if (adapterCount > 1) {
                    info.AppendFormat(_T("    GPU%u:               %s"), adapter + 1, deviceName.GetString());
                } else {
                    info.AppendFormat(_T("    GPU:                %s"), deviceName.GetString());
                }
                if (adapterIdentifier.DriverVersion.QuadPart) {
                    info.AppendFormat(_T(" (driver version: %s)"),
                                      FileVersionInfo::FormatVersionString(adapterIdentifier.DriverVersion.LowPart, adapterIdentifier.DriverVersion.HighPart).GetString());
                }
                info += _T("\r\n");
            }
        }
        pD3D9->Release();
    }

    // Allocate a global memory object for the text
    int len = info.GetLength() + 1;
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(WCHAR));
    if (hGlob) {
        // Lock the handle and copy the text to the buffer
        LPVOID pData = GlobalLock(hGlob);
        if (pData) {
            wcscpy_s((WCHAR*)pData, len, (LPCWSTR)info);
            GlobalUnlock(hGlob);

            if (GetParent()->OpenClipboard()) {
                // Place the handle on the clipboard, if the call succeeds
                // the system will take care of the allocated memory
                if (::EmptyClipboard() && ::SetClipboardData(CF_UNICODETEXT, hGlob)) {
                    hGlob = nullptr;
                }

                ::CloseClipboard();
            }
        }

        if (hGlob) {
            GlobalFree(hGlob);
        }
    }
}


