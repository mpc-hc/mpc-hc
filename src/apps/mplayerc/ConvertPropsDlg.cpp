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
#include "ConvertPropsDlg.h"


// CConvertPropsDlg dialog

CConvertPropsDlg::CConvertPropsDlg(bool fPin, CWnd* pParent /*=NULL*/)
	: CResizableDialog(CConvertPropsDlg::IDD, pParent)
	, m_fPin(fPin)
{
}

CConvertPropsDlg::~CConvertPropsDlg()
{
}

void CConvertPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_fcc);
	DDX_Control(pDX, IDC_EDIT1, m_text);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CConvertPropsDlg, CResizableDialog)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateButton1)
	ON_CBN_EDITCHANGE(IDC_COMBO1, OnCbnEditchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST1, OnLvnKeydownList1)
END_MESSAGE_MAP()


// CConvertPropsDlg message handlers

BOOL CConvertPropsDlg::OnInitDialog()
{
	__super::OnInitDialog();

	AddAnchor(IDC_COMBO1, TOP_LEFT);
	AddAnchor(IDC_EDIT1, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_BUTTON1, TOP_RIGHT);
	AddAnchor(IDC_LIST1, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_CENTER);
	AddAnchor(IDCANCEL, BOTTOM_CENTER);

	if(m_fPin)
	{
		m_fcc.AddString(_T("NAME"));
		m_fcc.AddString(_T("LANG"));
		m_fcc.AddString(_T("DESC"));
		m_fcc.AddString(_T("SGRP"));
	}
	else
	{
		m_fcc.AddString(_T("TITL"));
		m_fcc.AddString(_T("AUTH"));
		m_fcc.AddString(_T("RTNG"));
		m_fcc.AddString(_T("CPYR"));
		m_fcc.AddString(_T("DESC"));
	}

	m_list.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 75);
	m_list.InsertColumn(1, _T("Text"), LVCFMT_LEFT, 280);

	m_list.SetExtendedStyle(m_list.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	POSITION pos = m_props.GetStartPosition();
	while(pos)
	{
		CString key, value;
		m_props.GetNextAssoc(pos, key, value);
		SetItem(key, value);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConvertPropsDlg::SetItem(CString key, CString value)
{
	LVFINDINFO fi;
	fi.flags = LVFI_STRING;
	fi.psz = key;

	int i = m_list.FindItem(&fi);
	if(i < 0) i = m_list.InsertItem(m_list.GetItemCount(), _T(""));

	key.Trim();
	value.Trim();
	
	if(value.IsEmpty())
	{
		m_list.DeleteItem(i);
		return;
	}

	if(key == _T("LANG") && value.GetLength() != 3)
	{
		m_list.DeleteItem(i);
		AfxMessageBox(_T("LANG has to be a three letter ISO 639-2 language code."), MB_OK);
		return;
	}

	m_list.SetItemText(i, 0, key);
	m_list.SetItemText(i, 1, value);
}

void CConvertPropsDlg::OnOK()
{
	m_props.RemoveAll();

	for(int i = 0; i < m_list.GetItemCount(); i++)
		m_props[m_list.GetItemText(i, 0)] = m_list.GetItemText(i, 1);

	__super::OnOK();
}

void CConvertPropsDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem >= 0)
	{
		m_fcc.SetWindowText(m_list.GetItemText(lpnmlv->iItem, 0));
		m_text.SetWindowText(m_list.GetItemText(lpnmlv->iItem, 1));
	}

	*pResult = 0;
}

void CConvertPropsDlg::OnBnClickedButton1()
{
	CString key, value;
	m_fcc.GetWindowText(key);
	m_text.GetWindowText(value);
	if(key.GetLength() != 4) {AfxMessageBox(_T("ID must be 4 characters long!"), MB_OK); return;}
	SetItem(key, value);
}

void CConvertPropsDlg::OnUpdateButton1(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDlgItem(IDC_EDIT1)->GetWindowTextLength() > 0);
}

void CConvertPropsDlg::OnCbnEditchangeCombo1()
{
	int i = m_fcc.GetCurSel();
	if(i < 0) return;

	CString key;
	m_fcc.GetLBText(i, key);

	LVFINDINFO fi;
	fi.flags = LVFI_STRING;
	fi.psz = key;

	i = m_list.FindItem(&fi);
	if(i > 0) m_text.SetWindowText(m_list.GetItemText(i, 1));
}

void CConvertPropsDlg::OnCbnSelchangeCombo1()
{
	OnCbnEditchangeCombo1();
}

void CConvertPropsDlg::OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	int i = m_fcc.GetCurSel();
	if(i >= 0) m_list.DeleteItem(i);

	*pResult = 0;
}
