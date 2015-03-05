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
#include "mplayerc.h"
#include "MainFrm.h"
#include "IPTVScanDlg.h"
#include "TVToolsDlg.h"
#include "OpenFileDlg.h"


// CIPTVScanDlg dialog

IMPLEMENT_DYNAMIC(CIPTVScanDlg, CDialog)

CIPTVScanDlg::CIPTVScanDlg(CWnd* pParent /*=nullptr*/)
    : CDialog(CIPTVScanDlg::IDD, pParent)
    , m_bInProgress(false)
    , m_bStopRequested(false)
    , m_iChannelAdditionMethod(0)
    , m_bRemoveChannels(FALSE)
    , m_bOnlyNewChannels(FALSE)
    , m_bSaveOnlyValid(FALSE)
    , m_pIPTVDiscoverySetup(nullptr)
    , m_pTVToolsThread(nullptr)
{
    m_pParent = pParent;
}

CIPTVScanDlg::~CIPTVScanDlg()
{
}

BOOL CIPTVScanDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_pTVToolsThread = dynamic_cast<CTVToolsDlg*> (m_pParent)->m_pTVToolsThread;
    if (!m_pTVToolsThread) {
        ASSERT(FALSE);
    }

    m_ChannelList.InsertColumn(ISCC_NUMBER, ResStr(IDS_DVB_CHANNEL_NUMBER), LVCFMT_LEFT, 35);
    m_ChannelList.InsertColumn(ISCC_NAME, ResStr(IDS_DVB_CHANNEL_NAME), LVCFMT_LEFT, 190);
    m_ChannelList.InsertColumn(ISCC_ADDRESS, _T("URL"), LVCFMT_LEFT, 150);
    m_ChannelList.InsertColumn(ISCC_PREFNUM, _T("Channel number"), LVCFMT_LEFT, 85);
    m_ChannelList.InsertColumn(ISCC_VALIDATED, _T("Validated"), LVCFMT_LEFT, 60);
    m_ChannelList.InsertColumn(ISCC_CHANNEL, _T("Channel"), LVCFMT_LEFT, 0);

    m_btnSave.EnableWindow(FALSE);
    m_btnScan.EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK_REMOVE_CHANNELS)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK_SAVE_ONLY_VALID)->EnableWindow(FALSE);
    OnUpdateAddChannelMethod(IDC_RADIO1);
    return TRUE;
}

void CIPTVScanDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHANNELNAME, m_ChannelName);
    DDX_Control(pDX, IDC_IPADDRESS, m_IPAddress);
    DDX_Control(pDX, IDC_CHANNEL_LIST, m_ChannelList);
    DDX_Control(pDX, IDC_IMPORT_LIST, m_btnImportList);
    DDX_Control(pDX, IDC_STATIC1, m_StaticChName);
    DDX_Control(pDX, IDC_STATIC2, m_Static_IPAdr);
    DDX_Control(pDX, ID_SAVE, m_btnSave);
    DDX_Control(pDX, IDC_DISCOVERY_SETUP, m_btnDiscoverySetup);
    DDX_Control(pDX, IDC_DISCOVERY, m_btnDiscovery);

    DDX_Check(pDX, IDC_CHECK_SAVE_ONLY_VALID, m_bSaveOnlyValid);
    DDX_Control(pDX, IDC_CHECK_SAVE_ONLY_VALID, m_chkSaveOnlyValid);
    DDX_Check(pDX, IDC_CHECK_REMOVE_CHANNELS, m_bRemoveChannels);
    DDX_Control(pDX, IDC_CHECK_REMOVE_CHANNELS, m_chkRemoveChannels);
    DDX_Check(pDX, IDC_CHECK_ONLY_NEW, m_bOnlyNewChannels);
    DDX_Control(pDX, IDC_CHECK_ONLY_NEW, m_chkOnlyNewCh);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
    DDX_Control(pDX, IDC_NEW_CHANNEL, m_btnAddChannel);
    DDX_Control(pDX, IDC_RADIO1, m_rdChAddMethod1);
    DDX_Control(pDX, IDC_RADIO2, m_rdChAddMethod2);
    DDX_Control(pDX, IDC_RADIO3, m_rdChAddMethod3);
    DDX_Control(pDX, IDC_RADIO4, m_rdChAddMethod4);
    DDX_Radio(pDX, IDC_RADIO1, m_iChannelAdditionMethod);
    DDX_Control(pDX, IDC_IPADDRESS1, m_IPAddress1);
    DDX_Control(pDX, IDC_IPADDRESS2, m_IPAddress2);
    DDX_Control(pDX, IDC_PORT, m_Port);
    DDX_Control(pDX, IDC_EXPECTEDTIME, m_ExpectedTime);
    DDX_Control(pDX, IDC_STATIC3, m_StaticIP1);
    DDX_Control(pDX, IDC_STATIC4, m_StaticIP2);
    DDX_Control(pDX, IDC_STATIC5, m_StaticPort);
    DDX_Control(pDX, IDC_STATIC6, m_StaticTime);
    DDX_Control(pDX, IDC_SCAN, m_btnScan);
}

BEGIN_MESSAGE_MAP(CIPTVScanDlg, CDialog)
    ON_BN_CLICKED(ID_SAVE, OnClickedSave)
    ON_BN_CLICKED(IDCANCEL, OnClickedCancel)
    ON_BN_CLICKED(IDC_NEW_CHANNEL, OnClickedNewChannel)
    ON_BN_CLICKED(IDC_IMPORT_LIST, OnClickedImportList)
    ON_BN_CLICKED(IDC_DISCOVERY_SETUP, OnClickedDiscoverySetup)
    ON_BN_CLICKED(IDC_DISCOVERY, OnClickedDiscovery)
    ON_BN_CLICKED(IDC_CHECK_REMOVE_CHANNELS, OnUpdateData)
    ON_BN_CLICKED(IDC_CHECK_SAVE_ONLY_VALID, OnUpdateData)
    ON_BN_CLICKED(IDC_CHECK_ONLY_NEW, OnUpdateData)
    ON_BN_CLICKED(IDC_SCAN, OnClickedScan)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO4, OnUpdateAddChannelMethod)
    ON_EN_CHANGE(IDC_IPADDRESS1, OnUpdateExpectedTime)
    ON_EN_CHANGE(IDC_IPADDRESS2, OnUpdateExpectedTime)
    ON_EN_CHANGE(IDC_PORT, OnUpdateExpectedTime)
    ON_MESSAGE(WM_IPTV_NEW_CHANNEL, OnNewChannel)
    ON_MESSAGE(WM_IPTV_END_DISCOVERY, OnEndDiscovery)
END_MESSAGE_MAP()


// CIPTVScanDlg message handlers

void CIPTVScanDlg::OnUpdateData()
{
    UpdateData(true);
}

void CIPTVScanDlg::SetInterfaceBusy(boolean bNewStatusBusy)
{
    if (bNewStatusBusy != GetInterfaceBusy()) {
        bInterfaceBusy = bNewStatusBusy;
        m_rdChAddMethod1.EnableWindow(!bNewStatusBusy);
        m_rdChAddMethod2.EnableWindow(!bNewStatusBusy);
        m_rdChAddMethod3.EnableWindow(!bNewStatusBusy);
        m_rdChAddMethod4.EnableWindow(!bNewStatusBusy);
        m_btnImportList.EnableWindow(!bNewStatusBusy);
        m_StaticChName.EnableWindow(!bNewStatusBusy);
        m_ChannelName.EnableWindow(!bNewStatusBusy);
        m_Static_IPAdr.EnableWindow(!bNewStatusBusy);
        m_IPAddress.EnableWindow(!bNewStatusBusy);
        m_btnAddChannel.EnableWindow(!bNewStatusBusy);
        m_btnDiscoverySetup.EnableWindow(!bNewStatusBusy);
        m_btnDiscovery.EnableWindow(!bNewStatusBusy);
        m_StaticIP1.EnableWindow(!bNewStatusBusy);
        m_StaticIP2.EnableWindow(!bNewStatusBusy);
        m_StaticPort.EnableWindow(!bNewStatusBusy);
        m_StaticTime.EnableWindow(!bNewStatusBusy);
        m_IPAddress1.EnableWindow(!bNewStatusBusy);
        m_IPAddress2.EnableWindow(!bNewStatusBusy);
        m_Port.EnableWindow(!bNewStatusBusy);
        m_chkOnlyNewCh.EnableWindow(!bNewStatusBusy);
        m_ExpectedTime.EnableWindow(!bNewStatusBusy);
        m_btnScan.EnableWindow(!bNewStatusBusy);
        if (bNewStatusBusy) {
            m_btnSave.EnableWindow(FALSE);
            m_btnScan.EnableWindow(FALSE);
            GetDlgItem(IDC_CHECK_REMOVE_CHANNELS)->EnableWindow(FALSE);
            GetDlgItem(IDC_CHECK_SAVE_ONLY_VALID)->EnableWindow(FALSE);
        }
        auto pParentWnd = dynamic_cast<CTVToolsDlg*>(GetParent());
        if (pParentWnd->m_TabCtrl) {
            pParentWnd->m_TabCtrl.EnableWindow(!bNewStatusBusy);
        }
    }
}

void CIPTVScanDlg::OnUpdateAddChannelMethod(UINT nId)
{
    const CAppSettings& s = AfxGetAppSettings();
    switch (nId) {
        case IDC_RADIO1:
            m_btnImportList.ShowWindow(SW_SHOW);
            m_StaticChName.ShowWindow(SW_HIDE);
            m_ChannelName.ShowWindow(SW_HIDE);
            m_Static_IPAdr.ShowWindow(SW_HIDE);
            m_IPAddress.ShowWindow(SW_HIDE);
            m_btnAddChannel.ShowWindow(SW_HIDE);
            m_btnDiscoverySetup.ShowWindow(SW_HIDE);
            m_btnDiscovery.ShowWindow(SW_HIDE);
            m_StaticIP1.ShowWindow(SW_HIDE);
            m_StaticIP2.ShowWindow(SW_HIDE);
            m_StaticPort.ShowWindow(SW_HIDE);
            m_StaticTime.ShowWindow(SW_HIDE);
            m_IPAddress1.ShowWindow(SW_HIDE);
            m_IPAddress2.ShowWindow(SW_HIDE);
            m_Port.ShowWindow(SW_HIDE);
            m_chkOnlyNewCh.ShowWindow(SW_HIDE);
            m_ExpectedTime.ShowWindow(SW_HIDE);
            m_btnScan.ShowWindow(SW_HIDE);
            break;
        case IDC_RADIO2:
            m_btnImportList.ShowWindow(SW_HIDE);
            m_StaticChName.ShowWindow(SW_SHOW);
            m_ChannelName.ShowWindow(SW_SHOW);
            m_Static_IPAdr.ShowWindow(SW_SHOW);
            m_IPAddress.ShowWindow(SW_SHOW);
            m_btnAddChannel.ShowWindow(SW_SHOW);
            m_btnDiscoverySetup.ShowWindow(SW_HIDE);
            m_btnDiscovery.ShowWindow(SW_HIDE);
            m_StaticIP1.ShowWindow(SW_HIDE);
            m_StaticIP2.ShowWindow(SW_HIDE);
            m_StaticPort.ShowWindow(SW_HIDE);
            m_StaticTime.ShowWindow(SW_HIDE);
            m_IPAddress1.ShowWindow(SW_HIDE);
            m_IPAddress2.ShowWindow(SW_HIDE);
            m_Port.ShowWindow(SW_HIDE);
            m_ExpectedTime.ShowWindow(SW_HIDE);
            m_chkOnlyNewCh.ShowWindow(SW_HIDE);
            m_btnScan.ShowWindow(SW_HIDE);
            m_chkSaveOnlyValid.ShowWindow(SW_HIDE);
            break;
        case IDC_RADIO3:
            m_btnImportList.ShowWindow(SW_HIDE);
            m_StaticChName.ShowWindow(SW_HIDE);
            m_ChannelName.ShowWindow(SW_HIDE);
            m_Static_IPAdr.ShowWindow(SW_HIDE);
            m_IPAddress.ShowWindow(SW_HIDE);
            m_btnAddChannel.ShowWindow(SW_HIDE);
            m_btnDiscoverySetup.ShowWindow(SW_HIDE);
            m_btnDiscovery.ShowWindow(SW_HIDE);
            m_StaticIP1.ShowWindow(SW_SHOW);
            m_StaticIP2.ShowWindow(SW_SHOW);
            m_StaticPort.ShowWindow(SW_SHOW);
            m_StaticTime.ShowWindow(SW_SHOW);
            m_IPAddress1.ShowWindow(SW_SHOW);
            m_IPAddress2.ShowWindow(SW_SHOW);
            m_Port.ShowWindow(SW_SHOW);
            m_ExpectedTime.ShowWindow(SW_SHOW);
            m_chkOnlyNewCh.ShowWindow(SW_SHOW);
            m_btnScan.ShowWindow(SW_SHOW);
            m_chkSaveOnlyValid.ShowWindow(SW_HIDE);
            break;
        case IDC_RADIO4:
            m_btnImportList.ShowWindow(SW_HIDE);
            m_StaticChName.ShowWindow(SW_HIDE);
            m_ChannelName.ShowWindow(SW_HIDE);
            m_Static_IPAdr.ShowWindow(SW_HIDE);
            m_IPAddress.ShowWindow(SW_HIDE);
            m_btnAddChannel.ShowWindow(SW_HIDE);
            m_btnDiscoverySetup.ShowWindow(SW_SHOW);
            m_btnDiscovery.ShowWindow(SW_SHOW);
            m_btnDiscovery.EnableWindow(!s.strServiceProvider_IP.IsEmpty() && !s.strServicesProvider_Port.IsEmpty());
            m_StaticIP1.ShowWindow(SW_HIDE);
            m_StaticIP2.ShowWindow(SW_HIDE);
            m_StaticPort.ShowWindow(SW_HIDE);
            m_StaticTime.ShowWindow(SW_HIDE);
            m_IPAddress1.ShowWindow(SW_HIDE);
            m_IPAddress2.ShowWindow(SW_HIDE);
            m_Port.ShowWindow(SW_HIDE);
            m_ExpectedTime.ShowWindow(SW_HIDE);
            m_chkOnlyNewCh.ShowWindow(SW_HIDE);
            m_btnScan.ShowWindow(SW_HIDE);
            m_chkSaveOnlyValid.ShowWindow(SW_SHOW);
            break;
    }
}

void CIPTVScanDlg::OnClickedScan()
{
    SetInterfaceBusy(true);
    m_bInProgress = true;
    m_IPAddress1.GetWindowTextW(m_strIPAddress1);
    m_IPAddress2.GetWindowTextW(m_strIPAddress2);
    m_Port.GetWindowTextW(m_strPort);
    m_strIPAddress1.Append(_T(":") + m_strPort);
    if (m_pTVToolsThread) {
        m_pTVToolsThread->PostThreadMessage(CTVToolsThread::TM_IPTV_SCAN, (WPARAM)(LPCTSTR)m_strIPAddress1, (LPARAM)(LPCTSTR)m_strIPAddress2);
    }
    else {
        TRACE(_T("m_pTVToolsThread thread not found."));
        ASSERT(FALSE);
    }

}

void CIPTVScanDlg::OnUpdateExpectedTime()
{
    CString strIPAddress1, strIPAddress2;
    m_IPAddress1.GetWindowTextW(strIPAddress1);
    m_IPAddress2.GetWindowTextW(strIPAddress2);

    CT2CA pszConvertedAnsiString1(strIPAddress1);
    char* sIPAddr1 = pszConvertedAnsiString1;
    UINT32 uIP1 = inet_addr(sIPAddr1);
    CT2CA pszConvertedAnsiString2(strIPAddress2);
    char* sIPAddr2 = pszConvertedAnsiString2;
    UINT32 uIP2 = inet_addr(sIPAddr2);
    uIP1 = ntohl(uIP1);
    uIP2 = ntohl(uIP2);
    int iTime = (int)(uIP2 - uIP1) * 3 / 60;
    CString strTime;
    if (iTime >= 0) {
        strTime.Format(_T("%.d minutes"), iTime);
    } else {
        strTime = _T(" - ");
    }
    m_ExpectedTime.SetWindowTextW(strTime);
    m_Port.GetWindowTextW(m_strPort);
    if ((iTime > 0 && iTime < 4320) && (!m_strPort.IsEmpty())) {
        m_btnScan.EnableWindow(TRUE);
    } else {
        m_btnScan.EnableWindow(FALSE);
    }
    UpdateData(TRUE);
}

void CIPTVScanDlg::OnClickedSave()
{
    auto& DVBChannels = AfxGetAppSettings().m_DVBChannels;
    const int maxChannelsNum = ID_NAVIGATE_JUMPTO_SUBITEM_END - ID_NAVIGATE_JUMPTO_SUBITEM_START + 1;
    CAppSettings& s = AfxGetAppSettings();
    int iChannel = 0;

    if (m_bRemoveChannels) {
        // Remove only IPTV Channels
        auto new_end = std::remove_if(std::begin(DVBChannels), std::end(DVBChannels), [](const CDVBChannel & c) { return c.IsIPTV(); });
        DVBChannels.erase(new_end, DVBChannels.end());
        s.uNextChannelCount = 0;
    }

    for (int i = 0; i < m_ChannelList.GetItemCount(); i++) {
        if (m_ChannelList.GetItemText(i, ISCC_VALIDATED) == _T("Yes") || !m_bSaveOnlyValid || !m_chkSaveOnlyValid.IsWindowVisible()) {
            try {
                CDVBChannel channel(m_ChannelList.GetItemText(i, ISCC_CHANNEL));
                bool bItemUpdated = false;
                auto it = DVBChannels.begin();
                while (it != DVBChannels.end() && !bItemUpdated) {
                    if (channel.IsIPTV()) {
                        if (((CString)it->GetUrl()).Compare(channel.GetUrl()) == 0) {
                            // Update existing channel
                            channel.SetPrefNumber(it->GetPrefNumber());
                            *it = channel;
                            iChannel = channel.GetPrefNumber();
                            bItemUpdated = true;
                        }
                    }
                    if (!bItemUpdated) {
                        *it++;
                    }
                }
                if (!bItemUpdated) {
                    // Add new channel to the end
                    const size_t size = DVBChannels.size();
                    if (size < maxChannelsNum) {
                        UINT nNextChannelID = s.uNextChannelCount;
                        while (s.FindChannelByPref(nNextChannelID)) {
                            nNextChannelID++;
                        }
                        channel.SetPrefNumber(nNextChannelID);
                        s.uNextChannelCount = nNextChannelID + 1;
                        DVBChannels.push_back(channel);
                        iChannel = channel.GetPrefNumber();
                    } else {
                        // Just to be safe. We have 600 channels limit, but we never know what user might load there ;)
                        CString msg;
                        msg.Format(_T("Unable to add new channel \"%s\" to the list. Channels list is full. Please notify developers about the problem."), channel.GetName());
                        AfxMessageBox(msg, MB_OK | MB_ICONERROR);

                    }
                }
            } catch (CException* e) {
                // The tokenisation can fail if the input string was invalid
                TRACE(_T("Failed to parse a IPTV channel from string \"%s\""), m_ChannelList.GetItemText(i, ISCC_CHANNEL));
                ASSERT(FALSE);
                e->Delete();
            }
        }
    }
    // Update the preferred numbers
    int nPrefNumber = 0;
    int nDVBLastChannel = s.nDVBLastChannel;
    for (auto& channel : s.m_DVBChannels) {
        // Make sure the current channel number will be updated
        if (channel.GetPrefNumber() == s.nDVBLastChannel) {
            nDVBLastChannel = nPrefNumber;
        }
        channel.SetPrefNumber(nPrefNumber++);
    }
    s.nDVBLastChannel = nDVBLastChannel;

    // Set the new channel and close the dialog
    GetParent()->SendMessage(WM_DTV_SETCHANNEL, (WPARAM)iChannel);
    GetParent()->SendMessage(WM_CLOSE);

}


void CIPTVScanDlg::OnClickedCancel()
{
    m_btnCancel.EnableWindow(false);
    m_bStopRequested = true;

    // Set the current channel and close the dialog
    GetParent()->SendMessage(WM_DTV_SETCHANNEL, (WPARAM)AfxGetAppSettings().nDVBLastChannel);
    GetParent()->SendMessage(WM_CLOSE);
    GetParent()->SendMessage(WM_DESTROY);

}


void CIPTVScanDlg::OnClickedNewChannel()
{
    CString strChannelName;
    CString strURL;

    m_ChannelName.GetWindowText(strChannelName);
    m_IPAddress.GetWindowText(strURL);

    if (!strChannelName.IsEmpty() && !strURL.IsEmpty()) {
        AddToList(strChannelName, strURL, 0);
    }

}


void CIPTVScanDlg::OnClickedImportList()
{
    CString sFilename;
    // Call file selection (currently m3u format only)
    CString filter = _T("List Files (.m3u)|*.m3u");
    CAtlArray<CString> mask;

    DWORD dwFlags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

    COpenFileDlg fd(mask, true, nullptr, nullptr, dwFlags, filter, GetParent());
    if (fd.DoModal() != IDOK) {
        return;
    }

    sFilename = fd.GetPathName();
    ImportFile(sFilename);
}

void CIPTVScanDlg::OnClickedDiscoverySetup()
{
    const CAppSettings& s = AfxGetAppSettings();
    if (!m_pIPTVDiscoverySetup) {
        m_pIPTVDiscoverySetup = std::make_unique<CIPTVDiscoverySetupDlg>(GetParent());
    }
    if (m_pIPTVDiscoverySetup->DoModal() != IDOK) {
        return;
    }
    GetDlgItem(IDC_DISCOVERY)->EnableWindow(!s.strServiceProvider_IP.IsEmpty() && !s.strServicesProvider_Port.IsEmpty());

}

void CIPTVScanDlg::OnClickedDiscovery()
{
    SetInterfaceBusy(true);
    m_bInProgress = true;
    if (m_pTVToolsThread) {
        m_pTVToolsThread->PostThreadMessage(CTVToolsThread::TM_IPTV_DISCOVERY, 0, 0);
    }
    else {
        TRACE(_T("m_pTVToolsThread thread not found."));
        ASSERT(FALSE);
    }
}

LPARAM CIPTVScanDlg::OnEndDiscovery(WPARAM wParam, LPARAM lParam)
{
    SetInterfaceBusy(false);

    m_btnSave.EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_REMOVE_CHANNELS)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK_SAVE_ONLY_VALID)->EnableWindow(TRUE);
    m_bInProgress = false;
    m_bStopRequested = false;
    return TRUE;
}

HRESULT CIPTVScanDlg::ImportFile(LPCTSTR strFilePath)
{
    HRESULT hr = S_OK;
    CStdioFile readFile;
    CFileException fileException;
    CString strChannelName;
    CString strURL;
    int nChannelNumber = 0;

    try {
        if (readFile.Open(strFilePath, CFile::modeRead, &fileException)) {
            CString strLine;
            while (readFile.ReadString(strLine)) {
                if (strLine.Find(_T("#EXTM3U")) == 0) {
                    continue;
                }

                if (strLine.Find(_T("#EXTINF")) == 0) {
                    int nPos0 = strLine.Find(_T("["));
                    int nPos1 = strLine.Find(_T("]"));
                    if ((nPos0 == -1) || (nPos1 == -1) || (nPos1 < nPos0)) {
                        nPos0 = strLine.Find(_T(","));
                        nPos1 = strLine.Find(_T("-"));
                    }
                    if ((nPos0 != -1) && (nPos1 != -1) && (nPos1 > nPos0 + 1)) {
                        nChannelNumber = _tstol(strLine.Mid(nPos0 + 1, nPos1 - (nPos0 + 1)));
                        strChannelName = strLine.Mid(nPos1 + 1, strLine.GetLength() - nPos1 - 1);
                        strChannelName.Trim();
                    } else {
                        nChannelNumber = 0;
                        ResStr(IDS_DTV_ERRORPARSING);
                    }
                } else {
                    if (IsValidUrl(strLine)) {
                        strURL = strLine;
                    } else {
                        strURL = ResStr(IDS_DTV_ERRORPARSING);
                    }
                    if (strChannelName && strURL) {
                        AddToList(strChannelName, strURL, nChannelNumber);
                        strChannelName.Empty();
                        strURL.Empty();
                    }
                }
            }
        } else {
            hr = E_FAIL;
        }
    } catch (CException* e) {
        hr = E_INVALIDARG;
        e->Delete();
    }

    return hr;
}



void CIPTVScanDlg::AddToList(LPCTSTR strChannelName, LPCTSTR strURL, int nChannelNumber)
{
    CDVBChannel channel;
    CString strTemp;

    if (!((CString)strChannelName).IsEmpty() && !((CString)strURL).IsEmpty()) {
        int nItem;

        channel.SetName(strChannelName);
        channel.SetUrl(strURL);
        channel.SetVideoPID(1);  // Radio channels not considered here
        channel.SetDefaultAudio(-1); // Info not relevant for iptv
        channel.SetDefaultSubtitle(0); // Info not relevant for iptv
        channel.SetOriginNumber(nChannelNumber);

        if (channel.GetOriginNumber() != 0) { // LCN is available
            nChannelNumber = channel.GetOriginNumber();
            // Insert new channel so that channels are sorted by their logical number
            for (nItem = 0; nItem < m_ChannelList.GetItemCount(); nItem++) {
                if ((int)m_ChannelList.GetItemData(nItem) > nChannelNumber || (int)m_ChannelList.GetItemData(nItem) == 0) {
                    break;
                }
            }
        } else {
            nChannelNumber = 0;
            nItem = m_ChannelList.GetItemCount();
        }

        strTemp.Format(_T("%d"), nChannelNumber);
        nItem = m_ChannelList.InsertItem(nItem, strTemp);
        m_ChannelList.EnsureVisible(m_ChannelList.GetItemCount() - 1, false); // Scroll down to the bottom

        m_ChannelList.SetItemData(nItem, channel.GetOriginNumber());

        m_ChannelList.SetItemText(nItem, ISCC_NAME, channel.GetName());

        m_ChannelList.SetItemText(nItem, ISCC_ADDRESS, channel.GetUrl());

        m_ChannelList.SetItemText(nItem, ISCC_PREFNUM, _T("0"));
        m_ChannelList.SetItemText(nItem, ISCC_VALIDATED, _T(" - "));
        strTemp = channel.ToString();
        m_ChannelList.SetItemText(nItem, ISCC_CHANNEL, strTemp);
        m_btnSave.EnableWindow(TRUE);
        m_chkRemoveChannels.EnableWindow(TRUE);
    }
}

LRESULT CIPTVScanDlg::OnNewChannel(WPARAM wParam, LPARAM lParam)
{
    try {
        CDVBChannel channel((LPCTSTR)lParam);
        CString strTemp;
        int nItem, nChannelNumber;
        bool bItemUpdated = false;

        if (channel.GetOriginNumber() != 0) { // LCN is available
            nChannelNumber = channel.GetOriginNumber();
            // Insert new channel so that channels are sorted by their logical number
            for (nItem = 0; nItem < m_ChannelList.GetItemCount(); nItem++) {
                if ((int)m_ChannelList.GetItemData(nItem) > nChannelNumber || (int)m_ChannelList.GetItemData(nItem) == 0) {
                    break;
                }
            }
        } else {
            nChannelNumber = 0;
            nItem = m_ChannelList.GetItemCount();
        }
        if (((CString)channel.GetName()) == _T(".")) {
            auto& DVBChannels = AfxGetAppSettings().m_DVBChannels;

            auto it = DVBChannels.begin();
            while (it != DVBChannels.end() && !bItemUpdated) {
                if (channel.IsIPTV()) {
                    if (((CString)it->GetUrl()).Compare(channel.GetUrl()) == 0) {
                        // Update existing channel
                        channel.SetPrefNumber(it->GetPrefNumber());
                        channel.SetName(it->GetName());
                        bItemUpdated = true;
                    }
                }
                if (!bItemUpdated) {
                    *it++;
                }
            }
            if (!bItemUpdated) {
                channel.SetName(channel.GetUrl());
            }
        }

        if (!bItemUpdated || (!m_bOnlyNewChannels)) {
            strTemp.Format(_T("%d"), nChannelNumber);
            nItem = m_ChannelList.InsertItem(nItem, strTemp);
            m_ChannelList.EnsureVisible(m_ChannelList.GetItemCount() - 1, false); // Scroll down to the bottom
            m_ChannelList.SetItemData(nItem, channel.GetOriginNumber());

            // Insert new channel in the list
            m_ChannelList.SetItemText(nItem, ISCC_NUMBER, strTemp);
            m_ChannelList.SetItemText(nItem, ISCC_NAME, channel.GetName());
            m_ChannelList.SetItemText(nItem, ISCC_ADDRESS, channel.GetUrl());
            strTemp.Format(_T("%d"), channel.GetOriginNumber());
            m_ChannelList.SetItemText(nItem, ISCC_PREFNUM, strTemp);
            strTemp.Format(_T("%d"), channel.GetONID());
            m_ChannelList.SetItemText(nItem, ISCC_SERVICEID, strTemp);
            strTemp = channel.ToString();

            m_ChannelList.SetItemText(nItem, ISCC_CHANNEL, strTemp);
        }
    } catch (CException* e) {
        // The tokenisation can fail if the input string was invalid
        TRACE(_T("Failed to parse an IPTV channel from string \"%s\""), (LPCTSTR)lParam);
        ASSERT(FALSE);
        e->Delete();
        return FALSE;
    }

    return TRUE;
}
