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
#include "IPTVDiscoverySetupDlg.h"

enum ISC_COLUMN {
    ISCC_NUMBER,
    ISCC_NAME,
    ISCC_ADDRESS,
    ISCC_PREFNUM,
    ISCC_VALIDATED,
    ISCC_CHANNEL,
    ISCC_SERVICEID
};

// CIPTVScanDlg dialog

class CIPTVScanDlg : public CDialog
{
    DECLARE_DYNAMIC(CIPTVScanDlg)

public:
    CIPTVScanDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CIPTVScanDlg();
    virtual BOOL OnInitDialog();
    std::unique_ptr<CIPTVDiscoverySetupDlg> m_pIPTVDiscoverySetup;

    // Dialog Data
    enum { IDD = IDD_IPTV_SCAN };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //    void    SetProgress(bool bState);
    void SaveScanSettings();

    DECLARE_MESSAGE_MAP()

private:
    HRESULT ImportFile(LPCTSTR sFileName);
    void AddToList(LPCTSTR strChannelName, LPCTSTR strURL, int nChannelNumber);
    boolean bInterfaceBusy;
    void SetInterfaceBusy(boolean bNewStatusBusy);
    boolean GetInterfaceBusy() { return bInterfaceBusy; }

public:
    CListCtrl m_ChannelList;
    CEdit m_ChannelName;
    CEdit m_IPAddress;
    CEdit m_IPAddress1;
    CEdit m_IPAddress2;
    CEdit m_Port;
    CEdit m_ExpectedTime;
    CButton m_btnSave;
    CButton m_btnCancel;
    CButton m_btnScan;
    CButton m_btnImportList;
    CButton m_btnAddChannel;
    CButton m_btnDiscoverySetup;
    CButton m_btnDiscovery;
    CStatic m_StaticChName;
    CStatic m_Static_IPAdr;
    CStatic m_StaticIP1;
    CStatic m_StaticIP2;
    CStatic m_StaticPort;
    CStatic m_StaticTime;
    BOOL m_bRemoveChannels;
    BOOL m_bSaveOnlyValid;
    BOOL m_bOnlyNewChannels;
    CButton m_chkRemoveChannels;
    CButton m_chkOnlyNewCh;
    CButton m_chkSaveOnlyValid;
    CButton m_rdChAddMethod1;
    CButton m_rdChAddMethod2;
    CButton m_rdChAddMethod3;
    CButton m_rdChAddMethod4;
    int m_iChannelAdditionMethod;
    CString m_strIPAddress1, m_strIPAddress2, m_strPort;

    afx_msg LRESULT OnNewChannel(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEndDiscovery(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUpdateData();
    afx_msg void OnClickedSave();
    afx_msg void OnClickedCancel();
    afx_msg void OnClickedNewChannel();
    afx_msg void OnClickedImportList();
    afx_msg void OnClickedScan();
    afx_msg void OnUpdateAddChannelMethod(UINT nID);
    afx_msg void OnClickedDiscoverySetup();
    afx_msg void OnClickedDiscovery();
    afx_msg void OnUpdateExpectedTime();
};
