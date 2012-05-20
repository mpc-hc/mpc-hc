/*
 * $Id$
 *
 * (C) 2012 see Authors.txt
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
#include "InternalFiltersConfig.h" // needed for HAS_FFMPEG

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

extern "C" char *GetFFmpegCompiler();

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD), m_appname(_T(""))
	, m_strBuildNumber(_T(""))
	, m_MPCCompiler(_T(""))
	, m_FFmpegCompiler(_T(""))
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

BOOL CAboutDlg::OnInitDialog()
{
	USES_CONVERSION;
	UpdateData();

#ifdef _WIN64
	m_appname += _T(" x64");
#endif

	m_strBuildNumber = AfxGetMyApp()->m_strVersion;

#if defined(__INTEL_COMPILER)
#if (__INTEL_COMPILER >= 1200)
	m_MPCCompiler = _T("ICL 12.x");
#else
	#error Compiler is not supported!
#endif
#elif defined(_MSC_VER)
#if (_MSC_VER == 1700)
	m_MPCCompiler = _T("MSVC 2011");
#elif (_MSC_VER == 1600)
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

#if !defined(_M_X64) && defined(_M_IX86_FP)
#if (_M_IX86_FP == 1) // /arch:SSE was used
	m_MPCCompiler += _T(" (SSE)");
#elif (_M_IX86_FP == 2) // /arch:SSE2 was used
	m_MPCCompiler += _T(" (SSE2)");
#endif
#endif // _M_IX86_FP
#ifdef _DEBUG
	m_MPCCompiler += _T(" Debug");
#endif

#if HAS_FFMPEG
	m_FFmpegCompiler.Format(A2W(GetFFmpegCompiler()));
#endif

	// Build the path to Authors.txt
	GetModuleFileName(AfxGetInstanceHandle(), m_AuthorsPath.GetBuffer(_MAX_PATH), _MAX_PATH);
	m_AuthorsPath.ReleaseBuffer();
	m_AuthorsPath = m_AuthorsPath.Left(m_AuthorsPath.ReverseFind('\\') + 1) + _T("Authors.txt");
	// Check if the file exists
	CFileStatus fs;
	if (CFile::GetStatus(m_AuthorsPath, fs)) {
		// If it does, we make the filename clickable
		m_Credits.Replace(_T("Authors.txt"), _T("<a>Authors.txt</a>"));
	}

	UpdateData(FALSE);

	GetDlgItem(IDOK)->SetFocus();

	return FALSE;
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
	DDX_Text(pDX, IDC_AUTHORS_LINK, m_Credits);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CLICK, IDC_SOURCEFORGE_LINK, OnHomepage)
	ON_NOTIFY(NM_CLICK, IDC_AUTHORS_LINK, OnAuthors)
END_MESSAGE_MAP()

void CAboutDlg::OnHomepage(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(m_hWnd, _T("open"), _T("http://mpc-hc.sourceforge.net/"), NULL, NULL, SW_SHOWDEFAULT);
	*pResult = 0;
}

void CAboutDlg::OnAuthors(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(m_hWnd, _T("open"), m_AuthorsPath, NULL, NULL, SW_SHOWDEFAULT);
	*pResult = 0;
}
