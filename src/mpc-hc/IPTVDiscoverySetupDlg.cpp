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

// IPTVScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IPTVDiscoverySetupDlg.h"
#include "IPTVMcastTools.h"
#include "TVToolsDlg.h"
#include "mplayerc.h"


// CIPTVDiscoverySetupDlg dialog

IMPLEMENT_DYNAMIC(CIPTVDiscoverySetupDlg, CDialog)

CIPTVDiscoverySetupDlg::CIPTVDiscoverySetupDlg(CWnd* pParent /*=nullptr*/)
    : CDialog(CIPTVDiscoverySetupDlg::IDD, pParent)

{
    m_pTVToolsThread = dynamic_cast<CTVToolsDlg*> (pParent)->m_pTVToolsThread;
    if (!m_pTVToolsThread) {
        ASSERT(FALSE);
    }
}

CIPTVDiscoverySetupDlg::~CIPTVDiscoverySetupDlg()
{
}

BOOL CIPTVDiscoverySetupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    const CAppSettings& s = AfxGetAppSettings();

    m_SP_EntryPoint.SetWindowTextW(s.strServicesDiscoveryEntry);
    m_SP_Port.SetWindowTextW(s.strServicesDiscoveryPort);
    m_IPAddress.SetWindowTextW(s.strServiceProvider_IP);
    m_Port.SetWindowTextW(s.strServicesProvider_Port);

    m_SP_List.InsertColumn(TSDC_NUMBER, ResStr(IDS_DTV_IPTV_NUMBER), LVCFMT_LEFT, 35);
    m_SP_List.InsertColumn(TSDC_NAME, ResStr(IDS_DTV_IPTV_LOCALIZATION), LVCFMT_LEFT, 190);
    m_SP_List.InsertColumn(TSDC_IPADDRESS, ResStr(IDS_DTV_IPTV_IP), LVCFMT_LEFT, 150);
    m_SP_List.InsertColumn(TSDC_IPPORT, ResStr(IDS_DTV_IPTV_PORT), LVCFMT_LEFT, 55);

    m_SP_List.SetExtendedStyle(m_SP_List.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    m_btnSave.EnableWindow(FALSE);

    return TRUE;
}

void CIPTVDiscoverySetupDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SP_ENTRY_POINT, m_SP_EntryPoint);
    DDX_Control(pDX, IDC_SP_PORT, m_SP_Port);
    DDX_Control(pDX, IDC_IPADDRESS, m_IPAddress);
    DDX_Control(pDX, IDC_PORT, m_Port);
    DDX_Control(pDX, IDC_SP_LIST, m_SP_List);
    DDX_Control(pDX, ID_SAVE, m_btnSave);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);

}

BEGIN_MESSAGE_MAP(CIPTVDiscoverySetupDlg, CDialog)

    ON_BN_CLICKED(IDC_FIND, OnFindServiceProviders)
    ON_BN_CLICKED(ID_SAVE, OnClickedSave)
    ON_BN_CLICKED(IDCANCEL, OnClickedCancel)
    ON_NOTIFY(NM_CLICK, IDC_SP_LIST, OnClickSPList)
    ON_EN_CHANGE(IDC_SP_ENTRY_POINT, onUpdated)
    ON_EN_CHANGE(IDC_SP_PORT, onUpdated)
    ON_EN_CHANGE(IDC_IPADDRESS, onUpdated)
    ON_EN_CHANGE(IDC_PORT, onUpdated)
    ON_MESSAGE(WM_IPTV_NEW_SERVICEPROVIDER, OnNewServiceProvider)
END_MESSAGE_MAP()

void CIPTVDiscoverySetupDlg::OnClickSPList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

    if (lpnmlv->iItem >= 0) {
        UpdateData();
        CString strIP = m_SP_List.GetItemText(lpnmlv->iItem, TSDC_IPADDRESS);
        m_IPAddress.SetWindowTextW(strIP);
        CString strPort = m_SP_List.GetItemText(lpnmlv->iItem, TSDC_IPPORT);
        m_Port.SetWindowTextW(strPort);
    }

    *pResult = 0;
}

void CIPTVDiscoverySetupDlg::OnClickedSave()
{
    CAppSettings& s = AfxGetAppSettings();

    m_SP_EntryPoint.GetWindowTextW(s.strServicesDiscoveryEntry);
    m_SP_Port.GetWindowTextW(s.strServicesDiscoveryPort);
    m_IPAddress.GetWindowTextW(s.strServiceProvider_IP);
    m_Port.GetWindowTextW(s.strServicesProvider_Port);

    EndDialog(IDOK);
}


void CIPTVDiscoverySetupDlg::OnClickedCancel()
{

    OnCancel();
}

void CIPTVDiscoverySetupDlg::OnFindServiceProviders()
{
    m_SP_EntryPoint.GetWindowTextW(m_strIP);
    m_SP_Port.GetWindowTextW(m_strPort);

    m_pTVToolsThread->PostThreadMessage(CTVToolsThread::TM_IPTV_SERVICEPROVIDERS, (WPARAM)(LPCTSTR) m_strIP, (LPARAM)(LPCTSTR) m_strPort);
}

LRESULT CIPTVDiscoverySetupDlg::OnNewServiceProvider(WPARAM wParam, LPARAM lParam)
{
    try {
        strServiceProvider* pSP((strServiceProvider*)lParam);
        int nItem = m_SP_List.GetItemCount();
        CString strTemp;
        strTemp.Format(_T("%d"), nItem);
        nItem = m_SP_List.InsertItem(nItem, strTemp);
        m_SP_List.SetItemText(nItem, TSDC_NUMBER, strTemp);
        m_SP_List.SetItemText(nItem, TSDC_NAME, pSP->DomainName);
        m_SP_List.SetItemText(nItem, TSDC_IPADDRESS, pSP->IP_addr);
        m_SP_List.SetItemText(nItem, TSDC_IPPORT, pSP->port);
    } catch (CException* e) {
        // Assigning lParam to a strServiceProvider type can fail if lParam was invalid
        TRACE(_T("Failed to assign the Service Provider structure from lParam \"%s\""), (LPCTSTR)lParam);
        ASSERT(FALSE);
        e->Delete();
        return FALSE;
    }
    return TRUE;
}

void CIPTVDiscoverySetupDlg::onUpdated()
{
    GetDlgItem(ID_SAVE)->EnableWindow(TRUE);
}
