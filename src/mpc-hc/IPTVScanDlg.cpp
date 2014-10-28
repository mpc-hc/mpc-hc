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

// IPTVScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "IPTVScanDlg.h"
#include "OpenFileDlg.h"


enum TSC_COLUMN {
    TSCC_NUMBER,
    TSCC_NAME,
    TSCC_FREQUENCY,
    TSCC_ENCRYPTED,
    TSCC_VIDEO_FORMAT,
    TSCC_VIDEO_FPS,
    TSCC_VIDEO_RES,
    TSCC_VIDEO_AR,
    TSCC_CHANNEL
};

// CIPTVScanDlg dialog

IMPLEMENT_DYNAMIC(CIPTVScanDlg, CDialog)

CIPTVScanDlg::CIPTVScanDlg(CWnd* pParent /*=nullptr*/)
    : CDialog(CIPTVScanDlg::IDD, pParent)
    , m_iChannelAdditionMethod(0)
{

}

CIPTVScanDlg::~CIPTVScanDlg()
{
}

BOOL CIPTVScanDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_ChannelList.InsertColumn(TSCC_NUMBER, ResStr(IDS_DVB_CHANNEL_NUMBER), LVCFMT_LEFT, 35);
    m_ChannelList.InsertColumn(TSCC_NAME, ResStr(IDS_DVB_CHANNEL_NAME), LVCFMT_LEFT, 190);
    m_ChannelList.InsertColumn(TSCC_FREQUENCY, ResStr(IDS_DVB_CHANNEL_URL), LVCFMT_LEFT, 150);
    m_ChannelList.InsertColumn(TSCC_ENCRYPTED, ResStr(IDS_DVB_CHANNEL_ENCRYPTION), LVCFMT_LEFT, 55);
    m_ChannelList.InsertColumn(TSCC_VIDEO_FORMAT, ResStr(IDS_DVB_CHANNEL_FORMAT), LVCFMT_LEFT, 55);
    m_ChannelList.InsertColumn(TSCC_VIDEO_FPS, ResStr(IDS_DVB_CHANNEL_FPS), LVCFMT_LEFT, 50);
    m_ChannelList.InsertColumn(TSCC_VIDEO_RES, ResStr(IDS_DVB_CHANNEL_RESOLUTION), LVCFMT_LEFT, 70);
    m_ChannelList.InsertColumn(TSCC_VIDEO_AR, ResStr(IDS_DVB_CHANNEL_ASPECT_RATIO), LVCFMT_LEFT, 50);
    m_ChannelList.InsertColumn(TSCC_CHANNEL, _T("Channel"), LVCFMT_LEFT, 0);

    m_btnSave.EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK_REMOVE_CHANNELS)->EnableWindow(FALSE);
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
    DDX_Check(pDX, IDC_CHECK_REMOVE_CHANNELS, m_bRemoveChannels);
    DDX_Control(pDX, IDC_CHECK_REMOVE_CHANNELS, m_chkRemoveChannels);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
    DDX_Control(pDX, IDC_NEW_CHANNEL, m_btnAddChannel);
    DDX_Radio(pDX, IDC_RADIO1, m_iChannelAdditionMethod);
}

BEGIN_MESSAGE_MAP(CIPTVScanDlg, CDialog)
    ON_BN_CLICKED(ID_SAVE, OnBnClickedSave)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
    ON_BN_CLICKED(IDC_NEW_CHANNEL, OnBnClickedNewChannel)
    ON_BN_CLICKED(IDC_IMPORT_LIST, OnBnClickedImportList)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO4, OnUpdateAddChannelMethod)
END_MESSAGE_MAP()


// CIPTVScanDlg message handlers

void CIPTVScanDlg::OnUpdateAddChannelMethod(UINT nId)
{
    switch (nId) {
        case IDC_RADIO1:
            m_btnImportList.ShowWindow(SW_SHOW);
            m_StaticChName.ShowWindow(SW_HIDE);
            m_ChannelName.ShowWindow(SW_HIDE);
            m_Static_IPAdr.ShowWindow(SW_HIDE);
            m_IPAddress.ShowWindow(SW_HIDE);
            m_btnAddChannel.ShowWindow(SW_HIDE);
            break;
        case IDC_RADIO2:
            m_btnImportList.ShowWindow(SW_HIDE);
            m_StaticChName.ShowWindow(SW_SHOW);
            m_ChannelName.ShowWindow(SW_SHOW);
            m_Static_IPAdr.ShowWindow(SW_SHOW);
            m_IPAddress.ShowWindow(SW_SHOW);
            m_btnAddChannel.ShowWindow(SW_SHOW);
            break;
        case IDC_RADIO3:
            m_btnImportList.ShowWindow(SW_HIDE);
            m_StaticChName.ShowWindow(SW_HIDE);
            m_ChannelName.ShowWindow(SW_HIDE);
            m_Static_IPAdr.ShowWindow(SW_HIDE);
            m_IPAddress.ShowWindow(SW_HIDE);
            m_btnAddChannel.ShowWindow(SW_HIDE);
            break;
        case IDC_RADIO4:
            m_btnImportList.ShowWindow(SW_HIDE);
            m_StaticChName.ShowWindow(SW_HIDE);
            m_ChannelName.ShowWindow(SW_HIDE);
            m_Static_IPAdr.ShowWindow(SW_HIDE);
            m_IPAddress.ShowWindow(SW_HIDE);
            m_btnAddChannel.ShowWindow(SW_HIDE);
            break;
    }
}

void CIPTVScanDlg::OnBnClickedSave()
{
    auto& DVBChannels = AfxGetAppSettings().m_DVBChannels;
    const int maxChannelsNum = ID_NAVIGATE_JUMPTO_SUBITEM_END - ID_NAVIGATE_JUMPTO_SUBITEM_START + 1;
    CAppSettings& s = AfxGetAppSettings();
    int iChannel = 0;

    UpdateData();
    if (m_bRemoveChannels) {
        // Remove only IPTV Channels
        auto it = std::remove_if(std::begin(DVBChannels), std::end(DVBChannels), IsChannelIPTV);
    }

    for (int i = 0; i < m_ChannelList.GetItemCount(); i++) {
        try {
            CDVBChannel channel(m_ChannelList.GetItemText(i, TSCC_CHANNEL));
            bool bItemUpdated = false;
            auto it = DVBChannels.begin();
            while (it != DVBChannels.end() && !bItemUpdated) {
                if (channel.IsIPTV()) {
                    if (it->GetUrl() == channel.GetUrl()) {
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
                    UINT nNextChannelID = s.nNextChannelCount;
                    while (s.FindChannelByPref(nNextChannelID)) {
                        nNextChannelID++;
                    }
                    channel.SetPrefNumber(nNextChannelID);
                    s.nNextChannelCount = nNextChannelID + 1;
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
            TRACE(_T("Failed to parse a IPTV channel from string \"%s\""), m_ChannelList.GetItemText(i, TSCC_CHANNEL));
            ASSERT(FALSE);
            e->Delete();
        }
    }
    // Set the new channel and close the dialog
    GetParent()->SendMessage(WM_DTV_SETCHANNEL, (WPARAM)iChannel);
    GetParent()->SendMessage(WM_CLOSE);

}


void CIPTVScanDlg::OnBnClickedCancel()
{
    // Set the current channel and close the dialog
    GetParent()->SendMessage(WM_DTV_SETCHANNEL, (WPARAM)AfxGetAppSettings().nDVBLastChannel);
    GetParent()->SendMessage(WM_CLOSE);
}


void CIPTVScanDlg::OnBnClickedNewChannel()
{
    CString strChannelName;
    CString strURL;

    m_ChannelName.GetWindowText(strChannelName);
    m_IPAddress.GetWindowText(strURL);

    if (!strChannelName.IsEmpty() && !strURL.IsEmpty()) {
        AddToList(strChannelName, strURL, 0);
    }

}


void CIPTVScanDlg::OnBnClickedImportList()
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


HRESULT CIPTVScanDlg::ImportFile(CString strFilePath)
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
                    if ((strLine.Find(_T("rtp")) != -1) || (strLine.Find(_T("RTP")) != -1) ||
                            (strLine.Find(_T("udp")) != -1) || (strLine.Find(_T("UDP")) != -1) ||
                            (strLine.Find(_T("html")) != -1) || (strLine.Find(_T("HTML")) != -1) ||
                            (strLine.Find(_T("rtsp")) != -1) || (strLine.Find(_T("RTSP")) != -1) ||
                            (strLine.Find(_T("mms")) != -1) || (strLine.Find(_T("MMS")) != -1)) {
                        strURL = strLine;
                    } else {
                        strURL = ResStr(IDS_DTV_ERRORPARSING);
                    }
                    if (strChannelName && strURL) {
                        AddToList(strChannelName, strURL, nChannelNumber);
                        strChannelName = _T("");
                        strURL = _T("");
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



void CIPTVScanDlg::AddToList(CString strChannelName, CString strURL, int nChannelNumber)
{
    CDVBChannel channel;
    CString strTemp;

    if (!strChannelName.IsEmpty() && !strURL.IsEmpty()) {
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

        m_ChannelList.SetItemData(nItem, channel.GetOriginNumber());

        m_ChannelList.SetItemText(nItem, TSCC_NAME, channel.GetName());

        m_ChannelList.SetItemText(nItem, TSCC_FREQUENCY, channel.GetUrl());

        m_ChannelList.SetItemText(nItem, TSCC_ENCRYPTED, ResStr(channel.IsEncrypted() ? IDS_YES : IDS_NO));
        if (channel.GetVideoType() == DVB_H264) {
            strTemp = _T(" H.264");
        } else if (channel.GetVideoPID()) {
            strTemp = _T("MPEG-2");
        } else {
            strTemp = _T("   -  ");
        }
        m_ChannelList.SetItemText(nItem, TSCC_VIDEO_FORMAT, strTemp);
        strTemp = channel.GetVideoFpsDesc();
        m_ChannelList.SetItemText(nItem, TSCC_VIDEO_FPS, strTemp);
        if (channel.GetVideoWidth() || channel.GetVideoHeight()) {
            strTemp.Format(_T("%lux%lu"), channel.GetVideoWidth(), channel.GetVideoHeight());
        } else {
            strTemp = _T("   -   ");
        }
        m_ChannelList.SetItemText(nItem, TSCC_VIDEO_RES, strTemp);
        strTemp.Format(_T("%lu/%lu"), channel.GetVideoARy(), channel.GetVideoARx());
        m_ChannelList.SetItemText(nItem, TSCC_VIDEO_AR, strTemp);
        strTemp = channel.ToString();
        m_ChannelList.SetItemText(nItem, TSCC_CHANNEL, strTemp);
        m_btnSave.EnableWindow(TRUE);
        m_chkRemoveChannels.EnableWindow(TRUE);
    }
}
