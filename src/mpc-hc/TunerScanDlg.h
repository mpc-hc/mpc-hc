/*
 * (C) 2009-2014 see Authors.txt
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

#include <afxcmn.h>
#include <afxwin.h>
#include "CMPCThemeDialog.h"
#include "CMPCThemeEdit.h"
#include "CMPCThemePlayerListCtrl.h"
#include "CMPCThemeButton.h"


// CTunerScanDlg dialog

class CTunerScanDlg : public CMPCThemeDialog
{
    CMainFrame* m_pMainFrame;

    DECLARE_DYNAMIC(CTunerScanDlg)

public:
    CTunerScanDlg(CMainFrame* pMainFrame);   // standard constructor
    virtual ~CTunerScanDlg();

    // Dialog Data
    enum { IDD = IDD_TUNER_SCAN };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void         SetProgress(bool bState);
    void SaveScanSettings();

    DECLARE_MESSAGE_MAP()
public:
    ULONG m_ulFrequencyStart;
    ULONG m_ulFrequencyEnd;
    ULONG m_ulBandwidth;
    LONG m_lOffset;
    CMPCThemeEdit m_OffsetEditBox;
    BOOL m_bUseOffset;
    BOOL m_bIgnoreEncryptedChannels;
    CProgressCtrl m_Progress;
    CProgressCtrl m_Strength;
    CProgressCtrl m_Quality;
    CMPCThemePlayerListCtrl m_ChannelList;
    bool m_bInProgress;

    afx_msg LRESULT OnScanProgress(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnScanEnd(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnStats(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnNewChannel(WPARAM wParam, LPARAM lParam);

    afx_msg void OnBnClickedCheckOffset();
    afx_msg void OnBnClickedSave();
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
    CMPCThemeButton m_btnStart;
    CMPCThemeButton m_btnSave;
    CMPCThemeButton m_btnCancel;
};
