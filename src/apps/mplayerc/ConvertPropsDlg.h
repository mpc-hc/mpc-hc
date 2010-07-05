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

#include "../../DSUtil/DSMPropertyBag.h"
#include "afxwin.h"
#include "afxcmn.h"


// CConvertPropsDlg dialog

class CConvertPropsDlg : public CResizableDialog
{
private:
	bool m_fPin;
	void SetItem(CString key, CString value);

public:
	CConvertPropsDlg(bool fPin, CWnd* pParent = NULL);   // standard constructor
	virtual ~CConvertPropsDlg();

	CAtlStringMap<> m_props;

// Dialog Data
	enum { IDD = IDD_CONVERTPROPS_DLG };
	CComboBox m_fcc;
	CEdit m_text;
	CListCtrl m_list;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnUpdateButton1(CCmdUI* pCmdUI);
	afx_msg void OnCbnEditchangeCombo1();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnLvnKeydownList1(NMHDR *pNMHDR, LRESULT *pResult);
};
