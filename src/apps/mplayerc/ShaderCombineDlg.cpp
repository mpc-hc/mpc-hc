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
#include "ShaderCombineDlg.h"
#include "MainFrm.h"

// CShaderCombineDlg dialog

CShaderCombineDlg::CShaderCombineDlg(CAtlList<CString>& labels1, CAtlList<CString>& labels2, CWnd* pParent)
	: CCmdUIDialog(CShaderCombineDlg::IDD, pParent)
	, m_fcheck1(FALSE)
	, m_fcheck2(FALSE)
	, m_labels1(labels1)
	, m_labels2(labels2)
{
}

CShaderCombineDlg::~CShaderCombineDlg()
{
}

void CShaderCombineDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_fcheck1);
	DDX_Control(pDX, IDC_LIST1, m_list1);

	DDX_Check(pDX, IDC_CHECK2, m_fcheck2);
	DDX_Control(pDX, IDC_LIST2, m_list2);

	DDX_Control(pDX, IDC_COMBO1, m_combo);
}

BEGIN_MESSAGE_MAP(CShaderCombineDlg, CCmdUIDialog)
	ON_BN_CLICKED(IDC_CHECK1, OnUpdateCheck1)
	ON_LBN_SETFOCUS(IDC_LIST1, OnSetFocusList1)

	ON_BN_CLICKED(IDC_CHECK2, OnUpdateCheck2)
	ON_LBN_SETFOCUS(IDC_LIST2, OnSetFocusList2)

	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedDel)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedUp)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedDown)
END_MESSAGE_MAP()

// CShaderCombineDlg message handlers

BOOL CShaderCombineDlg::OnInitDialog()
{
	__super::OnInitDialog();

	//AddAnchor(IDOK, TOP_RIGHT);
	//AddAnchor(IDCANCEL, TOP_RIGHT);

	AppSettings& s = AfxGetAppSettings();

	// remember the initial state
	m_fcheck1 = m_oldcheck1 = ((CMainFrame*)AfxGetMainWnd())->m_bToggleShader;
	m_fcheck2 = m_oldcheck2 = ((CMainFrame*)AfxGetMainWnd())->m_bToggleShaderScreenSpace;
	m_oldlabels1.AddTailList(&m_labels1);
	m_oldlabels2.AddTailList(&m_labels2);

	POSITION pos;

	pos = m_labels1.GetHeadPosition();
	while (pos) {
		m_list1.AddString(m_labels1.GetNext(pos));
	}
	m_list1.AddString(_T(""));

	pos = m_labels2.GetHeadPosition();
	while (pos) {
		m_list2.AddString(m_labels2.GetNext(pos));
	}
	m_list2.AddString(_T(""));

	pos = s.m_shaders.GetHeadPosition();
	CString str;
	while (pos) {
		str = s.m_shaders.GetNext(pos).label;
		m_combo.AddString(str);
	}

	if (m_combo.GetCount()) {
		m_combo.SetCurSel(0);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CShaderCombineDlg::OnOK()
{
	__super::OnOK();
}

void CShaderCombineDlg::OnCancel()
{
	m_labels1.RemoveAll();
	m_labels1.AddTailList(&m_oldlabels1);
	((CMainFrame*)AfxGetMainWnd())->EnableShaders1(m_oldcheck1);

	m_labels2.RemoveAll();
	m_labels2.AddTailList(&m_oldlabels2);
	((CMainFrame*)AfxGetMainWnd())->EnableShaders2(m_oldcheck2);

	__super::OnCancel();
}

void CShaderCombineDlg::OnUpdateCheck1()
{
	UpdateData();

	((CMainFrame*)AfxGetMainWnd())->EnableShaders1(!!m_fcheck1);
}

void CShaderCombineDlg::OnUpdateCheck2()
{
	UpdateData();

	((CMainFrame*)AfxGetMainWnd())->EnableShaders2(!!m_fcheck2);
}

void CShaderCombineDlg::OnSetFocusList1()
{
	m_list2.SetCurSel(-1);
	if (m_list1.GetCurSel() < 0) {
		m_list1.SetCurSel(m_list1.GetCount()-1);
	}
}

void CShaderCombineDlg::OnSetFocusList2()
{
	m_list1.SetCurSel(-1);
	if (m_list2.GetCurSel() < 0) {
		m_list2.SetCurSel(m_list2.GetCount()-1);
	}
}

void CShaderCombineDlg::OnBnClickedAdd()
{
	int i = m_combo.GetCurSel();
	if (i < 0) {
		return;
	}
	CString label;
	m_combo.GetLBText(i, label);

	i = m_list1.GetCurSel();
	if (i >= 0) {
		m_list1.InsertString(i, label);
		UpdateShaders(SHAIDER1);
		return;
	}

	i = m_list2.GetCurSel();
	if (i >= 0) {
		m_list2.InsertString(i, label);
		UpdateShaders(SHAIDER2);
		//return;
	}
}

void CShaderCombineDlg::OnBnClickedDel()
{
	int i = m_list1.GetCurSel();
	if (i >= 0 && i < m_list1.GetCount()-1) {
		m_list1.DeleteString(i);
		if (i == m_list1.GetCount()-1 && i > 0) {
			i--;
		}
		m_list1.SetCurSel(i);

		UpdateShaders(SHAIDER1);
		return;
	}

	i = m_list2.GetCurSel();
	if (i >= 0 && i < m_list2.GetCount()-1) {
		m_list2.DeleteString(i);
		if (i == m_list2.GetCount()-1 && i > 0) {
			i--;
		}
		m_list2.SetCurSel(i);

		UpdateShaders(SHAIDER2);
		//return;
	}
}

void CShaderCombineDlg::OnBnClickedUp()
{
	int i = m_list1.GetCurSel();
	if (i >= 1 && i < m_list1.GetCount()-1) {
		CString label;
		m_list1.GetText(i, label);
		m_list1.DeleteString(i);
		i--;
		m_list1.InsertString(i, label);
		m_list1.SetCurSel(i);

		UpdateShaders(SHAIDER1);
		return;
	}

	i = m_list2.GetCurSel();
	if (i >= 1 && i < m_list2.GetCount()-1) {
		CString label;
		m_list2.GetText(i, label);
		m_list2.DeleteString(i);
		i--;
		m_list2.InsertString(i, label);
		m_list2.SetCurSel(i);

		UpdateShaders(SHAIDER2);
		//return;
	}
}

void CShaderCombineDlg::OnBnClickedDown()
{
	int i = m_list1.GetCurSel();
	if (i >= 0 && i < m_list1.GetCount()-2) {
		CString label;
		m_list1.GetText(i, label);
		m_list1.DeleteString(i);
		i++;
		m_list1.InsertString(i, label);
		m_list1.SetCurSel(i);

		UpdateShaders(SHAIDER1);
		return;
	}

	i = m_list2.GetCurSel();
	if (i >= 0 && i < m_list2.GetCount()-2) {
		CString label;
		m_list2.GetText(i, label);
		m_list2.DeleteString(i);
		i++;
		m_list2.InsertString(i, label);
		m_list2.SetCurSel(i);

		UpdateShaders(SHAIDER2);
		//return;
	}
}

// Update shaders

void CShaderCombineDlg::UpdateShaders(unsigned char type)
{
	if (type & SHAIDER1) {
		m_labels1.RemoveAll();
		for (int i = 0, j = m_list1.GetCount()-1; i < j; i++) {
			CString label;
			m_list1.GetText(i, label);
			m_labels1.AddTail(label);
		}

		((CMainFrame*)AfxGetMainWnd())->EnableShaders1(!!m_fcheck1);
	}

	if (type & SHAIDER2) {
		m_labels2.RemoveAll();
		for (int m = 0, n = m_list2.GetCount()-1; m < n; m++) {
			CString label;
			m_list2.GetText(m, label);
			m_labels2.AddTail(label);
		}

		((CMainFrame*)AfxGetMainWnd())->EnableShaders2(!!m_fcheck2);
	}
}
