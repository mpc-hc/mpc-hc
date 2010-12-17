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

#pragma once

#include "afxwin.h"


// CShaderCombineDlg dialog

class CShaderCombineDlg : public CResizableDialog
{
	CAtlList<CString>& m_labels;
	bool m_bScreenSpace;

public:
	CShaderCombineDlg(CAtlList<CString>& labels, CWnd* pParent, bool bScreenSpace);   // standard constructor
	virtual ~CShaderCombineDlg();

	// Dialog Data
	enum { IDD = IDD_SHADERCOMBINE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	CListBox m_list;
	CComboBox m_combo;
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton11();
};
