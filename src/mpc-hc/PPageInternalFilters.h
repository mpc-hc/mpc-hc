/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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
#include "CMPCThemePPageBase.h"
#include "CMPCThemePlayerListCtrl.h"

struct filter_t {
    LPCTSTR label;
    int type;
    int flag;
    UINT nHintID;

    filter_t()
        : label(nullptr)
        , type(0)
        , flag(0)
        , nHintID(0) {
    };

    filter_t(LPCTSTR label, int type, int flag, UINT nHintID)
        : label(label)
        , type(type)
        , flag(flag)
        , nHintID(nHintID) {
    };
};

class CPPageInternalFiltersListBox : public CMPCThemePlayerListCtrl
{
    DECLARE_DYNAMIC(CPPageInternalFiltersListBox)

    const CArray<filter_t>& m_filters;

public:
    CPPageInternalFiltersListBox(int n, const CArray<filter_t>& filters);

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
    virtual int AddFilter(filter_t* filter, bool checked);
    virtual void UpdateCheckState();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
};

// CPPageInternalFilters dialog

class CPPageInternalFilters : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageInternalFilters)

    CPPageInternalFiltersListBox m_listSrc;
    CPPageInternalFiltersListBox m_listTra;

    CArray<filter_t> m_filters;

    void InitFiltersList();

public:
    CPPageInternalFilters();
    virtual ~CPPageInternalFilters();

    // Dialog Data
    enum { IDD = IDD_PPAGEINTERNALFILTERS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

    void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedSplitterConf();
    afx_msg void OnBnClickedVideoDecConf();
    afx_msg void OnBnClickedAudioDecConf();
};
