/*
 * $Id$
 *
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

// PPageFileMediaInfo.cpp : implementation file


#include "stdafx.h"
#include "mplayerc.h"
#include "PPageFileMediaInfo.h"

#ifdef USE_MEDIAINFO_STATIC
#include <MediaInfo/MediaInfo.h>
using namespace MediaInfoLib;
#else
#include <MediaInfoDLL.h>
using namespace MediaInfoDLL;
#endif


// CPPageFileMediaInfo dialog

IMPLEMENT_DYNAMIC(CPPageFileMediaInfo, CPropertyPage)
CPPageFileMediaInfo::CPPageFileMediaInfo(CString fn, IFilterGraph* pFG)
	: CPropertyPage(CPPageFileMediaInfo::IDD, CPPageFileMediaInfo::IDD)
	, m_fn(fn)
	, m_pFG(pFG)
	, m_pCFont(NULL)
{
}

CPPageFileMediaInfo::~CPPageFileMediaInfo()
{
	delete m_pCFont;
	m_pCFont = NULL;
}

void CPPageFileMediaInfo::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MIEDIT, m_mediainfo);
}


BEGIN_MESSAGE_MAP(CPPageFileMediaInfo, CPropertyPage)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CPPageFileMediaInfo message handlers
static WNDPROC OldControlProc;

static LRESULT CALLBACK ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KEYDOWN) {
		if ((LOWORD(wParam)== 'A' || LOWORD(wParam) == 'a')
				&&(GetKeyState(VK_CONTROL) < 0)) {
			CEdit *pEdit = (CEdit*)CWnd::FromHandle(control);
			pEdit->SetSel(0, pEdit->GetWindowTextLength(), TRUE);
			return 0;
		}
	}

	return CallWindowProc(OldControlProc, control, message, wParam, lParam); // call edit control's own windowproc
}

BOOL CPPageFileMediaInfo::OnInitDialog()
{
	__super::OnInitDialog();

	if (!m_pCFont) {
		m_pCFont = DNew CFont;
	}
	if (!m_pCFont) {
		return TRUE;
	}

	if (m_fn == _T("")) {
		BeginEnumFilters(m_pFG, pEF, pBF) {
			CComQIPtr<IFileSourceFilter> pFSF = pBF;
			if (pFSF) {
				LPOLESTR pFN = NULL;
				AM_MEDIA_TYPE mt;
				if (SUCCEEDED(pFSF->GetCurFile(&pFN, &mt)) && pFN && *pFN) {
					m_fn = CStringW(pFN);
					CoTaskMemFree(pFN);
				}
				break;
			}
		}
		EndEnumFilters
	}

#ifdef USE_MEDIAINFO_STATIC
	MediaInfoLib::String f_name = m_fn;
	MediaInfoLib::MediaInfo MI;
#else
	MediaInfoDLL::String f_name = m_fn;
	MediaInfo MI;
#endif

	MI.Open(f_name);
	MI.Option(_T("Complete"));
	MI.Option(_T("Language"), _T("  Config_Text_ColumnSize;30"));
	MI_Text = MI.Inform().c_str();
	MI.Close();
	if (!MI_Text.Find(_T("Unable to load"))) {
		MI_Text = _T("");
	}

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	// The empty string will fallback to the first font that matches the other specified attributes.
	CString  fonts[] = { _T("Lucida Console"), _T("Courier New"), _T("") };
	// Use a negative value to match the character height instead of the cell height.
	int fonts_size[] = { -10, -11, -11 };
	UINT i = 0;
	BOOL success;
	do {
		lstrcpy(lf.lfFaceName, fonts[i]);
		lf.lfHeight = fonts_size[i];
		success = m_pCFont->CreateFontIndirect(&lf);
		i++;
	} while (!success && i < countof(fonts));
	m_mediainfo.SetFont(m_pCFont);
	m_mediainfo.SetWindowText(MI_Text);

	// subclass the edit control
	OldControlProc = (WNDPROC) SetWindowLongPtr(m_mediainfo.m_hWnd, GWLP_WNDPROC, (LONG_PTR) ControlProc);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPageFileMediaInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);
	if (bShow) {
		GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(SW_SHOW);
	} else {
		GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(SW_HIDE);
	}
}

#ifndef USE_MEDIAINFO_STATIC
bool CPPageFileMediaInfo::HasMediaInfo()
{
	MediaInfo MI;
	return MI.IsReady();
}
#endif
