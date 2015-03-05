/*
* (C) 2009-2013 see Authors.txt
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
#include "resource.h"
#include "TunerScanDlg.h"
#include "IPTVScanDlg.h"

class CTVToolsThread;

// CTVToolsDlg dialog

class CTVToolsDlg : public CDialog
{
    DECLARE_DYNAMIC(CTVToolsDlg)

public:
    CTVToolsDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CTVToolsDlg() = default;

    // Dialog Data
    enum { IDD = IDD_SCAN };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    DECLARE_MESSAGE_MAP()

    friend class CTVToolsThread;
    ::CEvent m_evCloseFinished;

private:
    enum { SC_NONE = 0, SC_DVB, SC_IPTV };
    int m_Tab_scan[2];
    HRESULT SetTab(int iTabNumber);

    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg LRESULT OnSetChannel(WPARAM wparam, LPARAM lparam);
    afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);

public:

    boolean m_bEnabledDVB;
    boolean m_bEnabledIPTV;
    CTunerScanDlg m_TunerScanDlg;
    CIPTVScanDlg m_IPTVScanDlg;
    CTVToolsThread* m_pTVToolsThread;

    virtual BOOL OnInitDialog();

    CTabCtrl m_TabCtrl;
};

class CTVToolsThread : public CWinThread
{
    DECLARE_DYNCREATE(CTVToolsThread);
public:
    CTVToolsThread() : m_pTVToolsDlg(nullptr) {}

    BOOL InitInstance();
    int ExitInstance();
    void SetTVToolsDlg(CTVToolsDlg* pTVToolsDlg) { m_pTVToolsDlg = pTVToolsDlg; }

    enum {
        TM_EXIT = WM_APP,
        TM_OPEN,
        TM_CLOSE,
        TM_TUNER_SCAN,
        TM_IPTV_DISCOVERY,
        TM_IPTV_SCAN,
        TM_IPTV_SERVICEPROVIDERS
    };

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnClose(WPARAM wParam, LPARAM lParam);
    afx_msg void OnExit(WPARAM wParam, LPARAM lParam);
    afx_msg void OnOpen(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTunerScan(WPARAM wParam, LPARAM lParam);
    afx_msg void OnIPTVDiscovery(WPARAM wParam, LPARAM lParam);
    afx_msg void OnIPTVScan(WPARAM wParam, LPARAM lParam);
    afx_msg void OnIPTVServiceProviders(WPARAM wParam, LPARAM lParam);

private:
    CTVToolsDlg* m_pTVToolsDlg;
};
