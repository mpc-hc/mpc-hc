/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

#include "CMPCThemePPageBase.h"
#include "CMPCThemeListBox.h"
#include "CMPCThemeButton.h"
#include "CMPCThemeEdit.h"

// CPPageDVD dialog

class CPPageDVD : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageDVD)

private:
    void UpdateLCIDList();

public:
    CPPageDVD();
    virtual ~CPPageDVD();

    CMPCThemeListBox m_lcids;
    CString m_dvdpath;
    CMPCThemeEdit m_dvdpathctrl;
    CMPCThemeButton m_dvdpathselctrl;
    int m_iDVDLocation;
    int m_iDVDLangType;

    LCID m_idMenuLang;
    LCID m_idAudioLang;
    LCID m_idSubtitlesLang;

    BOOL m_fClosedCaptions;

    // Dialog Data
    enum { IDD = IDD_PPAGEDVD};

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedLangradio123(UINT nID);
    afx_msg void OnLbnSelchangeList1();
    afx_msg void OnUpdateDVDPath(CCmdUI* pCmdUI);
};
