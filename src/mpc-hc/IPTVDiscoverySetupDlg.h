/*
* (C) 2014 see Authors.txt
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

enum TSD_COLUMN {
    TSDC_NUMBER,
    TSDC_NAME,
    TSDC_IPADDRESS,
    TSDC_IPPORT,
};


// CIPTVScanDlg dialog

class CIPTVDiscoverySetupDlg : public CDialog
{
    DECLARE_DYNAMIC(CIPTVDiscoverySetupDlg)

public:
    CIPTVDiscoverySetupDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CIPTVDiscoverySetupDlg();
    virtual BOOL OnInitDialog();

    // Dialog Data
    enum { IDD = IDD_IPTV_DISCOVERY };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    HRESULT ImportFile(CString sFileName);
    void AddToList(CString strChannelName, CString strURL, int nChannelNumber);
    CWinThread* m_pTVToolsThread;


public:
    CListCtrl m_SP_List;
    CEdit m_SP_EntryPoint;
    CEdit m_SP_Port;
    CEdit m_IPAddress;
    CEdit m_Port;
    CButton m_btnSave;
    CButton m_btnCancel;
    CString m_strPort, m_strIP;

    afx_msg LRESULT OnNewServiceProvider(WPARAM wParam, LPARAM lParam);
    afx_msg void OnFindServiceProviders();
    afx_msg void OnClickedSave();
    afx_msg void OnClickedCancel();
    afx_msg void OnClickSPList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void onUpdated();
};
