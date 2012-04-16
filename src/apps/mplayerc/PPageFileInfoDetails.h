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

#include "../../SubPic/ISubPic.h"
#include <afxwin.h>


// CPPageFileInfoDetails dialog

class CPPageFileInfoDetails : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPageFileInfoDetails)

private:
	CComPtr<IFilterGraph> m_pFG;
	CComPtr<ISubPicAllocatorPresenter> m_pCAP;

	HICON m_hIcon;

	void InitEncoding();

public:
	CPPageFileInfoDetails(CString fn, IFilterGraph* pFG, ISubPicAllocatorPresenter* pCAP);
	virtual ~CPPageFileInfoDetails();

	// Dialog Data
	enum { IDD = IDD_FILEPROPDETAILS };

	CStatic m_icon;
	CString m_fn;
	CString m_type;
	CString m_size;
	CString m_time;
	CString m_res;
	CString m_created;
	CEdit   m_encoding;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual LRESULT OnSetPageFocus(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
};
