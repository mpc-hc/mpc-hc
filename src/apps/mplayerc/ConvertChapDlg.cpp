/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// ConvertChapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "ConvertChapDlg.h"

// CConvertChapDlg dialog

CConvertChapDlg::CConvertChapDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CConvertChapDlg::IDD, pParent)
	, m_time(_T(""))
	, m_name(_T(""))
{
}

CConvertChapDlg::~CConvertChapDlg()
{
}

void CConvertChapDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_time);
	DDX_Text(pDX, IDC_EDIT2, m_name);
}

BEGIN_MESSAGE_MAP(CConvertChapDlg, CResizableDialog)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOK)
END_MESSAGE_MAP()


// CConvertChapDlg message handlers

BOOL CConvertChapDlg::OnInitDialog()
{
	__super::OnInitDialog();

	AddAnchor(IDC_EDIT1, TOP_LEFT);
	AddAnchor(IDC_EDIT2, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDOK, BOTTOM_CENTER);
	AddAnchor(IDCANCEL, BOTTOM_CENTER);

	CRect r;
	GetWindowRect(r);
	CSize s = r.Size();
	SetMinTrackSize(s);
	s.cx = 1000;
	SetMaxTrackSize(s);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConvertChapDlg::OnOK()
{
	UpdateData();

	__super::OnOK();
}

void CConvertChapDlg::OnUpdateOK(CCmdUI* pCmdUI)
{
	CString str;
	GetDlgItem(IDC_EDIT1)->GetWindowText(str);
	int i;
	pCmdUI->Enable(3 == _stscanf(str, _T("%d:%d:%d"), &i, &i, &i)
		&& GetDlgItem(IDC_EDIT2)->GetWindowTextLength() > 0);
}
