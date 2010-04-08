/*
 * $Id$
 *
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
#include "afxcmn.h"
#include "afxwin.h"


// CTunerScanDlg dialog

class CTunerScanDlg : public CDialog
{
    DECLARE_DYNAMIC(CTunerScanDlg)

public:
    CTunerScanDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CTunerScanDlg();

// Dialog Data
    enum { IDD = IDD_TUNER_SCAN };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void		 SetProgress(bool bState);

    DECLARE_MESSAGE_MAP()
public:
    ULONG m_ulFrequencyStart;
    ULONG m_ulFrequencyEnd;
    ULONG m_ulBandwidth;
    CProgressCtrl m_Progress;
    CProgressCtrl m_Strength;
    CProgressCtrl m_Quality;
    CListCtrl m_ChannelList;
    bool m_bInProgress;

    afx_msg LRESULT OnScanProgress(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnScanEnd(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStats(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNewChannel(WPARAM wParam, LPARAM lParam);

    afx_msg void OnBnClickedSave();
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
    CButton m_btnStart;
    CButton m_btnSave;
    CButton m_btnCancel;
};
