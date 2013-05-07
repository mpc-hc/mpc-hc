/*
 * (C) 2010-2013 see Authors.txt
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

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "PlayerNavigationDialog.h"
#include "DSUtil.h"
#include "moreuuids.h"


// CPlayerNavigationDialog dialog

// IMPLEMENT_DYNAMIC(CPlayerNavigationDialog, CResizableDialog)
CPlayerNavigationDialog::CPlayerNavigationDialog()
    : CResizableDialog(CPlayerNavigationDialog::IDD, nullptr)
    , m_bTVStations(true)
    , m_pParent(nullptr)
{
}

CPlayerNavigationDialog::~CPlayerNavigationDialog()
{
}

BOOL CPlayerNavigationDialog::Create(CWnd* pParent)
{
    if (!__super::Create(IDD, pParent)) {
        return FALSE;
    }
    m_pParent = pParent;
    return TRUE;
}

void CPlayerNavigationDialog::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LISTCHANNELS, m_ChannelList);
    DDX_Control(pDX, IDC_NAVIGATION_INFO, m_ButtonInfo);
    DDX_Control(pDX, IDC_NAVIGATION_SCAN, m_ButtonScan);
    DDX_Control(pDX, IDC_NAVIGATION_FILTERSTATIONS, m_ButtonFilterStations);
}

BOOL CPlayerNavigationDialog::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN) {
        if (pMsg->wParam == VK_RETURN) {
            CWnd* pFocused = GetFocus();
            if (pFocused && pFocused->m_hWnd == m_ChannelList.m_hWnd) {
                return TRUE;
            }
        }
    }
    return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerNavigationDialog, CResizableDialog)
    ON_WM_DESTROY()
    ON_LBN_SELCHANGE(IDC_LISTCHANNELS, OnChangeChannel)
    ON_BN_CLICKED(IDC_NAVIGATION_INFO, OnButtonInfo)
    ON_BN_CLICKED(IDC_NAVIGATION_SCAN, OnTunerScan)
    ON_BN_CLICKED(IDC_NAVIGATION_FILTERSTATIONS, OnTvRadioStations)

END_MESSAGE_MAP()


// CPlayerNavigationDialog message handlers

BOOL CPlayerNavigationDialog::OnInitDialog()
{
    __super::OnInitDialog();
    m_bTVStations = true;
    m_ButtonFilterStations.SetWindowText(ResStr(IDS_DVB_TVNAV_SEERADIO));
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPlayerNavigationDialog::OnDestroy()
{
    m_ChannelList.ResetContent();
    __super::OnDestroy();
}

void CPlayerNavigationDialog::OnChangeChannel()
{
    CWnd* TempWnd;
    int nItem;

    TempWnd = static_cast<CPlayerNavigationBar*>(m_pParent)->m_pParent;
    nItem = p_nItems[m_ChannelList.GetCurSel()] + ID_NAVIGATE_CHAP_SUBITEM_START;
    static_cast<CMainFrame*>(TempWnd)->OnNavigateChapters(nItem);
}

void CPlayerNavigationDialog::UpdateElementList()
{
    const CAppSettings& s = AfxGetAppSettings();

    if (s.iDefaultCaptureDevice == 1) {
        m_ChannelList.ResetContent();

        int nCurrentChannel = s.nDVBLastChannel;
        POSITION pos = s.m_DVBChannels.GetHeadPosition();

        while (pos) {
            const CDVBChannel& channel = s.m_DVBChannels.GetNext(pos);
            if (((m_bTVStations && channel.GetVideoPID() != 0) ||
                    (!m_bTVStations && channel.GetVideoPID() == 0)) && channel.GetAudioCount() > 0) {
                int nItem = m_ChannelList.AddString(channel.GetName());
                if (nItem < MAX_CHANNELS_ALLOWED) {
                    p_nItems [nItem] = channel.GetPrefNumber();
                }
                if (nCurrentChannel == channel.GetPrefNumber()) {
                    m_ChannelList.SetCurSel(nItem);
                }
            }
        }
    }

}

void CPlayerNavigationDialog::UpdatePos(int nID)
{
    for (int i = 0; i < MAX_CHANNELS_ALLOWED; i++) {
        if (p_nItems [i] == nID) {
            m_ChannelList.SetCurSel(i);
            break;
        }

    }
}

void CPlayerNavigationDialog::OnTunerScan()
{
    CWnd* TempWnd;
    TempWnd = static_cast<CPlayerNavigationBar*>(m_pParent)->m_pParent;
    static_cast<CMainFrame*>(TempWnd)->OnTunerScan();
    UpdateElementList();
}

void CPlayerNavigationDialog::OnButtonInfo()
{
    CWnd* TempWnd;
    TempWnd = static_cast<CPlayerNavigationBar*>(m_pParent)->m_pParent;
    static_cast<CMainFrame*>(TempWnd)->ShowCurrentChannelInfo(true, true);
}

void CPlayerNavigationDialog::OnTvRadioStations()
{
    m_bTVStations = !m_bTVStations;
    UpdateElementList();

    if (m_bTVStations) {
        m_ButtonFilterStations.SetWindowText(ResStr(IDS_DVB_TVNAV_SEERADIO));
    } else {
        m_ButtonFilterStations.SetWindowText(ResStr(IDS_DVB_TVNAV_SEETV));
    }
}
