/*
 * (C) 2010-2014 see Authors.txt
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
#include "PlayerNavigationDialog.h"
#include "DSUtil.h"
#include "mplayerc.h"
#include "MainFrm.h"

// CPlayerNavigationDialog dialog

// IMPLEMENT_DYNAMIC(CPlayerNavigationDialog, CResizableDialog)
CPlayerNavigationDialog::CPlayerNavigationDialog(CMainFrame* pMainFrame)
    : CResizableDialog(CPlayerNavigationDialog::IDD, nullptr)
    , m_pMainFrame(pMainFrame)
    , m_bTVStations(true)
{
}

BOOL CPlayerNavigationDialog::Create(CWnd* pParent)
{
    if (!__super::Create(IDD, pParent)) {
        return FALSE;
    }

    AddAnchor(IDC_LISTCHANNELS, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_NAVIGATION_INFO, BOTTOM_LEFT);
    AddAnchor(IDC_NAVIGATION_SCAN, BOTTOM_RIGHT);
    AddAnchor(IDC_NAVIGATION_FILTERSTATIONS, BOTTOM_LEFT, BOTTOM_RIGHT);

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
    ON_WM_CONTEXTMENU()
    ON_LBN_SELCHANGE(IDC_LISTCHANNELS, OnChangeChannel)
    ON_BN_CLICKED(IDC_NAVIGATION_INFO, OnButtonInfo)
    ON_BN_CLICKED(IDC_NAVIGATION_SCAN, OnTunerScan)
    ON_BN_CLICKED(IDC_NAVIGATION_FILTERSTATIONS, OnTvRadioStations)
END_MESSAGE_MAP()


// CPlayerNavigationDialog message handlers

BOOL CPlayerNavigationDialog::OnInitDialog()
{
    __super::OnInitDialog();

    if (m_bTVStations) {
        m_ButtonFilterStations.SetWindowText(ResStr(IDS_DVB_TVNAV_SEERADIO));
    } else {
        m_ButtonFilterStations.SetWindowText(ResStr(IDS_DVB_TVNAV_SEETV));
    }

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
    int nItem = m_ChannelList.GetCurSel();
    if (nItem != LB_ERR) {
        UINT nChannelID = (UINT)m_ChannelList.GetItemData(nItem) + ID_NAVIGATE_JUMPTO_SUBITEM_START;
        m_pMainFrame->OnNavigateJumpTo(nChannelID);
    }
}

void CPlayerNavigationDialog::UpdateElementList()
{
    if (m_pMainFrame->GetPlaybackMode() == PM_DIGITAL_CAPTURE) {
        const auto& s = AfxGetAppSettings();
        m_ChannelList.ResetContent();

        for (const auto& channel : s.m_DVBChannels) {
            if (channel.GetAudioCount() && (m_bTVStations && channel.GetVideoPID() || !m_bTVStations && !channel.GetVideoPID())) {
                int nItem = m_ChannelList.AddString(channel.GetName());
                if (nItem != LB_ERR) {
                    m_ChannelList.SetItemData(nItem, (DWORD_PTR)channel.GetPrefNumber());
                    if (s.nDVBLastChannel == channel.GetPrefNumber()) {
                        m_ChannelList.SetCurSel(nItem);
                    }
                }
            }
        }
    }
}

void CPlayerNavigationDialog::UpdatePos(int nID)
{
    for (int i = 0, count = m_ChannelList.GetCount(); i < count; i++) {
        if ((int)m_ChannelList.GetItemData(i) == nID) {
            m_ChannelList.SetCurSel(i);
            break;
        }
    }
}

void CPlayerNavigationDialog::OnTunerScan()
{
    m_pMainFrame->OnTunerScan();
    UpdateElementList();
}

void CPlayerNavigationDialog::OnButtonInfo()
{
    m_pMainFrame->UpdateCurrentChannelInfo(true, true);
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

void CPlayerNavigationDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    auto& s = AfxGetAppSettings();
    CPoint clientPoint = point;
    m_ChannelList.ScreenToClient(&clientPoint);
    BOOL bOutside;
    const UINT nItem = m_ChannelList.ItemFromPoint(clientPoint, bOutside);
    const int curSel = m_ChannelList.GetCurSel();
    const int channelCount = m_ChannelList.GetCount();
    CMenu m;
    m.CreatePopupMenu();

    enum {
        M_WATCH = 1,
        M_MOVE_UP,
        M_MOVE_DOWN,
        M_SORT,
        M_REMOVE,
        M_REMOVE_ALL
    };

    auto findChannelByItemNumber = [this](std::vector<CDVBChannel>& c, int nItem) {
        int nPrefNumber = m_ChannelList.GetItemData(nItem);
        return find_if(c.begin(), c.end(), [&](CDVBChannel const & channel) {
            return channel.GetPrefNumber() == nPrefNumber;
        });
        ASSERT(FALSE);
        return c.end();
    };

    if (!bOutside) {
        m_ChannelList.SetCurSel(nItem);

        m.AppendMenu(MF_STRING | (curSel != nItem ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)), M_WATCH, ResStr(IDS_NAVIGATION_WATCH));
        m.AppendMenu(MF_SEPARATOR);
        m.AppendMenu(MF_STRING | (nItem == 0 ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_MOVE_UP, ResStr(IDS_NAVIGATION_MOVE_UP));
        m.AppendMenu(MF_STRING | (nItem == channelCount - 1 ? (MF_DISABLED | MF_GRAYED) : MF_ENABLED), M_MOVE_DOWN, ResStr(IDS_NAVIGATION_MOVE_DOWN));
    }
    m.AppendMenu(MF_STRING | (channelCount > 1 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)), M_SORT, ResStr(IDS_NAVIGATION_SORT));
    m.AppendMenu(MF_SEPARATOR);
    if (!bOutside) {
        m.AppendMenu(MF_STRING | MF_ENABLED, M_REMOVE, ResStr(IDS_PLAYLIST_REMOVE));
    }
    m.AppendMenu(MF_STRING | (channelCount > 0 ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)), M_REMOVE_ALL, ResStr(IDS_NAVIGATION_REMOVE_ALL));

    int nID = (int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, point.x, point.y, this);

    try {
        switch (nID) {
            case M_WATCH:
                OnChangeChannel();
                break;
            case M_MOVE_UP:
                iter_swap(findChannelByItemNumber(s.m_DVBChannels, nItem), findChannelByItemNumber(s.m_DVBChannels, nItem - 1));
                UpdateElementList();
                break;
            case M_MOVE_DOWN:
                iter_swap(findChannelByItemNumber(s.m_DVBChannels, nItem), findChannelByItemNumber(s.m_DVBChannels, nItem + 1));
                UpdateElementList();
                break;
            case M_SORT:
                sort(s.m_DVBChannels.begin(), s.m_DVBChannels.end());
                UpdateElementList();
                break;
            case M_REMOVE: {
                const auto it = findChannelByItemNumber(s.m_DVBChannels, nItem);
                const int nRemovedPrefNumber = it->GetPrefNumber();
                s.m_DVBChannels.erase(it);
                // Update channels pref number
                for (CDVBChannel& channel : s.m_DVBChannels) {
                    const int nPrefNumber = channel.GetPrefNumber();
                    ASSERT(nPrefNumber != nRemovedPrefNumber);
                    if (nPrefNumber > nRemovedPrefNumber) {
                        channel.SetPrefNumber(nPrefNumber - 1);
                    }
                }
                UpdateElementList();

                UINT newCurSel = (UINT)curSel;
                if ((newCurSel >= nItem) && (channelCount - 1 == nItem || newCurSel > nItem)) {
                    --newCurSel;
                }

                const auto NewChanIt = findChannelByItemNumber(s.m_DVBChannels, newCurSel);
                if (NewChanIt != s.m_DVBChannels.end()) {
                    // Update pref number of the current channel
                    s.nDVBLastChannel = NewChanIt->GetPrefNumber();
                    m_ChannelList.SetCurSel(newCurSel);
                    if (curSel == nItem) {
                        // Set closest channel on list after removing current channel
                        m_pMainFrame->SetChannel(s.nDVBLastChannel);
                    }
                }
            }
            break;
            case M_REMOVE_ALL:
                if (IDYES == AfxMessageBox(IDS_REMOVE_CHANNELS_QUESTION, MB_ICONQUESTION | MB_YESNO, 0)) {
                    s.m_DVBChannels.clear();
                    UpdateElementList();
                }
                break;
            default:
                m_ChannelList.SetCurSel(curSel);
                break;
        }
    } catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        TRACE(_T("Navigation Dialog requested operation failed: \"%s\""), e.what());
        UpdateElementList();
        ASSERT(FALSE);
    }
}

