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
#include "PPageBase.h"


struct filter_t {
    LPCTSTR label;
    int type;
    int flag;
    UINT nHintID;
    CUnknown* (WINAPI* CreateInstance)(LPUNKNOWN lpunk, HRESULT* phr);
};

class CPPageInternalFiltersListBox : public CCheckListBox
{
    DECLARE_DYNAMIC(CPPageInternalFiltersListBox)

public:
    CPPageInternalFiltersListBox(int n);

protected:
    virtual void PreSubclassWindow();
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

    CFont m_bold;
    int m_n;
    unsigned int m_nbFiltersPerType[FILTER_TYPE_NB];
    unsigned int m_nbChecked[FILTER_TYPE_NB];

public:
    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
    virtual int AddFilter(filter_t* filter, bool checked);
    virtual void UpdateCheckState();
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
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
    afx_msg void OnSelChange();
    afx_msg void OnCheckBoxChange();
};
