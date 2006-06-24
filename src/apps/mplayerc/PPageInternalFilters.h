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

#include "PPageBase.h"
#include "afxwin.h"

class CPPageInternalFiltersListBox : public CCheckListBox
{
	DECLARE_DYNAMIC(CPPageInternalFiltersListBox)

public:
	CPPageInternalFiltersListBox();

	CFont m_bold;

protected:
	virtual void PreSubclassWindow();
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};

// CPPageInternalFilters dialog

class CPPageInternalFilters : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageInternalFilters)

public:
	CPPageInternalFilters();
	virtual ~CPPageInternalFilters();

// Dialog Data
	enum { IDD = IDD_PPAGEINTERNALFILTERS };
	CPPageInternalFiltersListBox m_listSrc;
	CPPageInternalFiltersListBox m_listTra;

	void ShowPPage(CPPageInternalFiltersListBox& l);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnLbnDblclkList1();
	afx_msg void OnLbnDblclkList2();
};
