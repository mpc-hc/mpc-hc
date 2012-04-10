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
#include <afxcmn.h>
#include "../../DSUtil/DSMPropertyBag.h"
#include "PPageBase.h"


// CPPageFileInfoRes dialog

class CPPageFileInfoRes : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageFileInfoRes)

private:
	CComPtr<IFilterGraph> m_pFG;
	HICON m_hIcon;
	CAtlList<CDSMResource> m_res;

public:
	CPPageFileInfoRes(CString fn, IFilterGraph* pFG);   // standard constructor
	virtual ~CPPageFileInfoRes();

	// Dialog Data
	enum { IDD = IDD_FILEPROPRES };

	CStatic m_icon;
	CString m_fn;
	CListCtrl m_list;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSaveAs();
	afx_msg void OnUpdateSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
};
