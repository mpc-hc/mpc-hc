/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2016 see Authors.txt
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

// CPPageLogo dialog

#include "MPCPngImage.h"
#include "CMPCThemePPageBase.h"

class CPPageLogo : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageLogo)

private:
    CList<UINT> m_logoids;
    POSITION m_logoidpos;
    CMPCPngImage m_logo;
    void GetDataFromRes();

public:
    CPPageLogo();
    virtual ~CPPageLogo();

    // Dialog Data
    enum { IDD = IDD_PPAGELOGO };
    int m_intext;
    CString m_logofn;
    CStatic m_logopreview;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedInternalRadio();
    afx_msg void OnBnClickedExternalRadio();
    afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButton2();
    CString m_author;
};
