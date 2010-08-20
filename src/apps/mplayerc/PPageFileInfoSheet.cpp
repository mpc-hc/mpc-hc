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
#include "MainFrm.h"
#include "PPageFileInfoSheet.h"

// CPPageFileInfoSheet

IMPLEMENT_DYNAMIC(CPPageFileInfoSheet, CPropertySheet)
CPPageFileInfoSheet::CPPageFileInfoSheet(CString fn, CMainFrame* pMainFrame, CWnd* pParentWnd)
	: CPropertySheet(ResStr(IDS_PROPSHEET_PROPERTIES), pParentWnd, 0)
	, m_clip(fn, pMainFrame->pGB)
	, m_details(fn, pMainFrame->pGB, pMainFrame->m_pCAP)
	, m_res(fn, pMainFrame->pGB)
	, m_mi(fn)
	, m_fn(fn)
{
	AddPage(&m_clip);
	AddPage(&m_details);

	BeginEnumFilters(pMainFrame->pGB, pEF, pBF)
	{
		if(CComQIPtr<IDSMResourceBag> pRB = pBF)
			if(pRB && pRB->ResGetCount() > 0)
			{
				AddPage(&m_res);
				break;
			}
	}
	EndEnumFilters;

	if (CPPageFileMediaInfo::HasMediaInfo())
		AddPage(&m_mi);
}

CPPageFileInfoSheet::~CPPageFileInfoSheet()
{
}


BEGIN_MESSAGE_MAP(CPPageFileInfoSheet, CPropertySheet)
	ON_BN_CLICKED(IDC_BUTTON_MI, OnSaveAs)
END_MESSAGE_MAP()

// CPPageFileInfoSheet message handlers

BOOL CPPageFileInfoSheet::OnInitDialog()
{
	BOOL fRet = __super::OnInitDialog();

	m_fn.TrimRight('/');
	int i = max(m_fn.ReverseFind('\\'), m_fn.ReverseFind('/'));
	if(i >= 0 && i < m_fn.GetLength()-1)
		m_fn = m_fn.Mid(i+1);
	m_fn = m_fn+_T(".MediaInfo.txt");

	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
	GetDlgItem(IDOK)->SetWindowText(ResStr(IDS_AG_CLOSE));

	CRect r;
	GetDlgItem(ID_APPLY_NOW)->GetWindowRect(&r);
	ScreenToClient(r);
	GetDlgItem(IDOK)->MoveWindow(r);

	r.MoveToX(5);
	r.right += 10;
	m_Button_MI.Create(ResStr(IDS_AG_SAVE_AS), WS_CHILD|BS_PUSHBUTTON|WS_VISIBLE, r, this, IDC_BUTTON_MI);
	m_Button_MI.SetFont(GetFont());
	m_Button_MI.ShowWindow(SW_HIDE);

	return fRet;
}

void CPPageFileInfoSheet::OnSaveAs()
{
	CFileDialog filedlg (FALSE, _T("*.txt"), m_fn,
						 OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,
						 _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"), NULL);

	if (filedlg.DoModal() == IDOK) // user has chosen a file, so
	{
		_TCHAR bom = (_TCHAR)0xFEFF;
		CFile mFile;
		if(mFile.Open(filedlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			mFile.Write(&bom, sizeof(_TCHAR));
			mFile.Write(LPCTSTR(m_mi.MI_Text), m_mi.MI_Text.GetLength()*sizeof(_TCHAR));
			mFile.Close();
		}
	}
}
