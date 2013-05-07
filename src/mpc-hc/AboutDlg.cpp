/*
 * (C) 2012-2013 see Authors.txt
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
#include "InternalFiltersConfig.h" // needed for HAS_FFMPEG
#endif
#include "mplayerc.h"
#include "version.h"
#include "WinAPIUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#if defined(HAS_FFMPEG) && !defined(MPCHC_LITE)
extern "C" char* GetFFmpegCompiler();
#endif

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
    , m_appname(_T(""))
    , m_strBuildNumber(_T(""))
    , m_MPCCompiler(_T(""))
#ifndef MPCHC_LITE
    , m_FFmpegCompiler(_T(""))
#endif
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

BOOL CAboutDlg::OnInitDialog()
{
    // Get the default text before it is overwritten by the call to __super::OnInitDialog()
    GetDlgItem(IDC_STATIC1)->GetWindowText(m_appname);
    GetDlgItem(IDC_AUTHORS_LINK)->GetWindowText(m_credits);

    __super::OnInitDialog();

    // Because we set LR_SHARED, there is no need to explicitly destroy the icon
    m_icon.SetIcon((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));

#if MPC_BETA_RELEASE || _WIN64
    m_appname += _T(" (");
#endif

#if MPC_BETA_RELEASE
    m_appname += MPC_VERSION_BETA;
#endif

#if MPC_BETA_RELEASE && _WIN64
    m_appname += _T(", ");
#endif

#ifdef _WIN64
    m_appname += _T("64-bit");
#endif

#if MPC_BETA_RELEASE || _WIN64
    m_appname += _T(")");
#endif

#ifdef MPCHC_LITE
    m_appname += _T(" Lite");
#endif

    m_strBuildNumber = MPC_VERSION_STR_FULL;

#if defined(__INTEL_COMPILER)
#if (__INTEL_COMPILER >= 1210)
    m_MPCCompiler = _T("ICL ") MAKE_STR(__INTEL_COMPILER) _T(" Build ") MAKE_STR(__INTEL_COMPILER_BUILD_DATE);
#else
#error Compiler is not supported!
#endif
#elif defined(_MSC_VER)
#if (_MSC_VER == 1700) // 2012
#if (_MSC_FULL_VER == 170060315)
    m_MPCCompiler = _T("MSVC 2012 Update 2");
#elif (_MSC_FULL_VER == 170051106)
    m_MPCCompiler = _T("MSVC 2012 Update 1");
#elif (_MSC_FULL_VER < 170050727)
    m_MPCCompiler = _T("MSVC 2012 Beta/RC/PR");
#else
    m_MPCCompiler = _T("MSVC 2012");
#endif
#elif (_MSC_VER == 1600) // 2010
#if (_MSC_FULL_VER >= 160040219)
    m_MPCCompiler = _T("MSVC 2010 SP1");
#else
    m_MPCCompiler = _T("MSVC 2010");
#endif
#elif (_MSC_VER < 1600)
#error Compiler is not supported!
#endif
#else
#error Please add support for your compiler
#endif

#if (__AVX__)
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

#if defined(HAS_FFMPEG) && !defined(MPCHC_LITE)
    m_FFmpegCompiler.Format(CA2CT(GetFFmpegCompiler()));
#else
    GetDlgItem(IDC_FFMPEG_TEXT)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_FFMPEG_COMPILER)->ShowWindow(SW_HIDE);
#endif

    // Build the path to Authors.txt
    m_AuthorsPath = GetProgramPath() + _T("Authors.txt");
    // Check if the file exists
    if (FileExists(m_AuthorsPath)) {
        // If it does, we make the filename clickable
        m_credits.Replace(_T("Authors.txt"), _T("<a>Authors.txt</a>"));
    }

    m_homepage.Format(_T("<a>%s</a>"), WEBSITE_URL);

    UpdateData(FALSE);

    GetDlgItem(IDOK)->SetFocus();

    return FALSE;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDR_MAINFRAME, m_icon);
    DDX_Text(pDX, IDC_STATIC1, m_appname);
    DDX_Text(pDX, IDC_BUILD_NUMBER, m_strBuildNumber);
    DDX_Text(pDX, IDC_MPC_COMPILER, m_MPCCompiler);
#ifndef MPCHC_LITE
    DDX_Text(pDX, IDC_FFMPEG_COMPILER, m_FFmpegCompiler);
#endif
    DDX_Text(pDX, IDC_AUTHORS_LINK, m_credits);
    DDX_Text(pDX, IDC_HOMEPAGE_LINK, m_homepage);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    // No message handlers
    //}}AFX_MSG_MAP
    ON_NOTIFY(NM_CLICK, IDC_HOMEPAGE_LINK, OnHomepage)
    ON_NOTIFY(NM_CLICK, IDC_AUTHORS_LINK, OnAuthors)
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
