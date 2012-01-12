/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see AUTHORS
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
#include "GoToDlg.h"

#include <atlrx.h>
#include "SettingsDefines.h"


// CGoToDlg dialog

IMPLEMENT_DYNAMIC(CGoToDlg, CDialog)
CGoToDlg::CGoToDlg(int time, float fps, CWnd* pParent /*=NULL*/)
	: CDialog(CGoToDlg::IDD, pParent)
	, m_timestr(_T(""))
	, m_framestr(_T(""))
	, m_time(time)
	, m_fps(fps)
{
	if (m_fps == 0) {
		CString str = AfxGetApp()->GetProfileString(IDS_R_SETTINGS, _T("fps"), _T("0"));
		if (_stscanf_s(str, _T("%f"), &m_fps) != 1) {
			m_fps = 0;
		}
	}
}

CGoToDlg::~CGoToDlg()
{
}

void CGoToDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_timestr);
	DDX_Text(pDX, IDC_EDIT2, m_framestr);
	DDX_Control(pDX, IDC_EDIT1, m_timeedit);
	DDX_Control(pDX, IDC_EDIT2, m_frameedit);
}

BOOL CGoToDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_time >= 0) {
		m_timestr.Format(_T("%02d:%02d:%02d.%03d"),
						 (m_time/(1000*60*60))%60, (m_time/(1000*60))%60, (m_time/1000)%60, m_time%1000);

		if (m_fps > 0) {
			m_framestr.Format(_T("%d, %.3f"), (int)(m_fps*m_time/1000), m_fps);
		}

		UpdateData(FALSE);

		switch (AfxGetApp()->GetProfileInt(IDS_R_SETTINGS, _T("gotoluf"), 0)) {
			default:
			case 0:
				m_timeedit.SetFocus();
				m_timeedit.SetSel(0, 0);
				break;
			case 1:
				m_frameedit.SetFocus();
				m_frameedit.SetSel(0, m_framestr.Find(','));
				break;
		}

	}

	return FALSE;

	//	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CGoToDlg, CDialog)
	ON_BN_CLICKED(IDC_OK1, OnBnClickedOk1)
	ON_BN_CLICKED(IDC_OK2, OnBnClickedOk2)
END_MESSAGE_MAP()


// CGoToDlg message handlers

void CGoToDlg::OnBnClickedOk1()
{
	UpdateData();

	unsigned int hh = 0;
	unsigned int mm = 0;
	float ss = 0.0;
	wchar_t c1 = L':'; // delimiter character
	wchar_t c2 = L':'; // delimiter character
	wchar_t c3[2]; // unnecessary character

	if ((swscanf_s(m_timestr, L"%f%1s", &ss, &c3, 2) == 1 || // sss[.ms]
		 swscanf_s(m_timestr, L"%u%c%f%1s", &mm, &c2, 1, &ss, &c3, 2) == 3 && ss < 60 || // mmm:ss[.ms]
		 swscanf_s(m_timestr, L"%u%c%u%c%f%1s", &hh, &c1, 1, &mm, &c2, 1, &ss, &c3, 2) == 5 && mm < 60  && ss < 60) && // hhh:mm:ss[.ms]
		 c1 == L':' && c2 == L':' && ss >=0) {
		
		m_time = (int)(1000*((hh*60+mm)*60+ss)+0.5);
		OnOK();
	} else {
		AfxMessageBox(ResStr(IDS_GOTO_ERROR_PARSING_TIME), MB_ICONEXCLAMATION | MB_OK);
	}
}

void CGoToDlg::OnBnClickedOk2()
{
	UpdateData();

	unsigned int frame;
	float fps;
	wchar_t c1[2]; // delimiter character
	wchar_t c2[2]; // unnecessary character

	int result = swscanf_s(m_framestr, L"%u%1s%f%1s", &frame, &c1, 2, &fps, &c2, 2);
	if (result == 1) {
		m_time = (int)(1000.0*frame/m_fps+0.5);
		OnOK();
	} else if (result == 3 && c1[0] == L',') {
		m_time = (int)(1000.0*frame/fps+0.5);
		OnOK();
	} else if (result == 0 || c1[0] != L',') {
		AfxMessageBox(ResStr(IDS_GOTO_ERROR_PARSING_TEXT), MB_ICONEXCLAMATION | MB_OK);
	} else {
		AfxMessageBox(ResStr(IDS_GOTO_ERROR_PARSING_FPS), MB_ICONEXCLAMATION | MB_OK);
	}
}

BOOL CGoToDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) {
		if (*GetFocus() == m_timeedit) {
			OnBnClickedOk1();
		} else if (*GetFocus() == m_frameedit) {
			OnBnClickedOk2();
		}

		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}
