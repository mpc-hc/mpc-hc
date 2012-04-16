/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#pragma once

#include <afxwin.h>
#include <ResizableLib/ResizableDialog.h>


// CShaderCombineDlg dialog

class CShaderCombineDlg : public CCmdUIDialog
{
#define SHADER1 1
#define SHADER2 2
#define SHADERS (SHADER1 | SHADER2)

	CListBox m_list1, m_list2;
	CComboBox m_combo;

	BOOL m_fcheck1, m_fcheck2;
	CAtlList<CString>& m_labels1;
	CAtlList<CString>& m_labels2;

	bool m_oldcheck1, m_oldcheck2;
	CAtlList<CString> m_oldlabels1;
	CAtlList<CString> m_oldlabels2;

	void UpdateShaders(unsigned char type = SHADERS);

public:
	CShaderCombineDlg(CAtlList<CString>& labels1, CAtlList<CString>& labels2, CWnd* pParent);   // standard constructor
	virtual ~CShaderCombineDlg();

	// Dialog Data
	enum { IDD = IDD_SHADERCOMBINE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

protected:
	virtual void OnOK();
	virtual void OnCancel();

public:
	afx_msg void OnUpdateCheck1();
	afx_msg void OnSetFocusList1();
	afx_msg void OnUpdateCheck2();
	afx_msg void OnSetFocusList2();

	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDel();
	afx_msg void OnBnClickedUp();
	afx_msg void OnBnClickedDown();
};