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


// CIPTVScanDlg dialog

class CIPTVScanDlg : public CDialog
{
    DECLARE_DYNAMIC(CIPTVScanDlg)

public:
    CIPTVScanDlg(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CIPTVScanDlg();
    virtual BOOL OnInitDialog();

    // Dialog Data
    enum { IDD = IDD_IPTV_SCAN };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //    void    SetProgress(bool bState);
    void SaveScanSettings();

    DECLARE_MESSAGE_MAP()

private:
    HRESULT ImportFile(CString sFileName);
    void AddToList(CString strChannelName, CString strURL, int nChannelNumber);


public:
    CListCtrl m_ChannelList;
    CEdit m_ChannelName;
    CEdit m_IPAddress;
    CButton m_btnSave;
    CButton m_btnCancel;
    CButton m_btnImportList;
    CButton m_btnAddChannel;
    CStatic m_StaticChName;
    CStatic m_Static_IPAdr;
    BOOL m_bRemoveChannels;
    CButton m_chkRemoveChannels;
    int m_iChannelAdditionMethod;

    afx_msg LRESULT OnNewChannel(WPARAM wParam, LPARAM lParam);

    afx_msg void OnBnClickedSave();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedNewChannel();
    afx_msg void OnBnClickedImportList();
    afx_msg void OnUpdateAddChannelMethod(UINT nID);
};
