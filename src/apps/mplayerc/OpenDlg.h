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

#pragma once
#include "resource.h"

// COpenDlg dialog

class COpenDlg : public CResizableDialog
{
//	DECLARE_DYNAMIC(COpenDlg)

public:
	COpenDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COpenDlg();

	bool m_fMultipleFiles;
	CAtlList<CString> m_fns;

// Dialog Data
	enum { IDD = IDD_OPEN_DLG };
	CComboBox m_mrucombo;
	CString m_path;
	CComboBox m_mrucombo2;
	CString m_path2;
	CStatic m_label2;
	BOOL m_fAppendPlaylist;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBrowsebutton();
	afx_msg void OnBnClickedBrowsebutton2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnUpdateDub(CCmdUI* pCmdUI);
};
