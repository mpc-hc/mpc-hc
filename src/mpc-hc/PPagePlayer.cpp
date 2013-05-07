/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "FileAssoc.h"
#include "PPagePlayer.h"


// CPPagePlayer dialog

IMPLEMENT_DYNAMIC(CPPagePlayer, CPPageBase)
CPPagePlayer::CPPagePlayer()
    : CPPageBase(CPPagePlayer::IDD, CPPagePlayer::IDD)
    , m_iAllowMultipleInst(0)
    , m_iAlwaysOnTop(FALSE)
    , m_fTrayIcon(FALSE)
    , m_iTitleBarTextStyle(0)
    , m_bTitleBarTextTitle(0)
    , m_fRememberWindowPos(FALSE)
    , m_fRememberWindowSize(FALSE)
    , m_fSavePnSZoom(FALSE)
    , m_fSnapToDesktopEdges(FALSE)
    , m_fUseIni(FALSE)
    , m_fKeepHistory(FALSE)
    , m_fHideCDROMsSubMenu(FALSE)
    , m_priority(FALSE)
    , m_fShowOSD(FALSE)
    , m_fLimitWindowProportions(TRUE)
    , m_fRememberDVDPos(FALSE)
    , m_fRememberFilePos(FALSE)
    , m_bRememberPlaylistItems(TRUE)
{
}

CPPagePlayer::~CPPagePlayer()
{
}

void CPPagePlayer::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDI_SINGLE, m_iconSingle);
    DDX_Control(pDX, IDI_MULTI, m_iconMulti);
    DDX_Radio(pDX, IDC_RADIO1, m_iAllowMultipleInst);
    DDX_Radio(pDX, IDC_RADIO3, m_iTitleBarTextStyle);
    DDX_Check(pDX, IDC_CHECK13, m_bTitleBarTextTitle);
    //DDX_Check(pDX, IDC_CHECK2, m_iAlwaysOnTop);
    DDX_Check(pDX, IDC_CHECK3, m_fTrayIcon);
    DDX_Check(pDX, IDC_CHECK6, m_fRememberWindowPos);
    DDX_Check(pDX, IDC_CHECK7, m_fRememberWindowSize);
    DDX_Check(pDX, IDC_CHECK11, m_fSavePnSZoom);
    DDX_Check(pDX, IDC_CHECK12, m_fSnapToDesktopEdges);
    DDX_Check(pDX, IDC_CHECK8, m_fUseIni);
    DDX_Check(pDX, IDC_CHECK1, m_fKeepHistory);
    DDX_Check(pDX, IDC_CHECK10, m_fHideCDROMsSubMenu);
    DDX_Check(pDX, IDC_CHECK9, m_priority);
    DDX_Check(pDX, IDC_SHOW_OSD, m_fShowOSD);
    DDX_Check(pDX, IDC_CHECK4, m_fLimitWindowProportions);
    DDX_Check(pDX, IDC_DVD_POS, m_fRememberDVDPos);
    DDX_Check(pDX, IDC_FILE_POS, m_fRememberFilePos);
    DDX_Check(pDX, IDC_CHECK2, m_bRememberPlaylistItems);
}

BEGIN_MESSAGE_MAP(CPPagePlayer, CPPageBase)
    ON_UPDATE_COMMAND_UI(IDC_CHECK13, OnUpdateCheck13)
    ON_UPDATE_COMMAND_UI(IDC_DVD_POS, OnUpdatePos)
    ON_UPDATE_COMMAND_UI(IDC_FILE_POS, OnUpdatePos)
END_MESSAGE_MAP()

// CPPagePlayer message handlers

BOOL CPPagePlayer::OnInitDialog()
{
    __super::OnInitDialog();

    m_iconSingle.SetIcon((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SINGLE), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
    m_iconMulti.SetIcon((HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_MULTI), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));

    const CAppSettings& s = AfxGetAppSettings();

    m_iAllowMultipleInst = s.fAllowMultipleInst;
    m_iTitleBarTextStyle = s.iTitleBarTextStyle;
    m_bTitleBarTextTitle = s.fTitleBarTextTitle;
    m_iAlwaysOnTop = s.iOnTop;
    m_fTrayIcon = s.fTrayIcon;
    m_fRememberWindowPos = s.fRememberWindowPos;
    m_fRememberWindowSize = s.fRememberWindowSize;
    m_fSavePnSZoom = s.fSavePnSZoom;
    m_fSnapToDesktopEdges = s.fSnapToDesktopEdges;
    m_fUseIni = AfxGetMyApp()->IsIniValid();
    m_fKeepHistory = s.fKeepHistory;
    m_fHideCDROMsSubMenu = s.fHideCDROMsSubMenu;
    m_priority = s.dwPriority != NORMAL_PRIORITY_CLASS;
    m_fShowOSD = s.fShowOSD;
    m_fRememberDVDPos = s.fRememberDVDPos;
    m_fRememberFilePos = s.fRememberFilePos;
    m_fLimitWindowProportions = s.fLimitWindowProportions;
    m_bRememberPlaylistItems = s.bRememberPlaylistItems;

    UpdateData(FALSE);

    GetDlgItem(IDC_FILE_POS)->EnableWindow(s.fKeepHistory);
    GetDlgItem(IDC_DVD_POS)->EnableWindow(s.fKeepHistory);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPagePlayer::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    s.fAllowMultipleInst = !!m_iAllowMultipleInst;
    s.iTitleBarTextStyle = m_iTitleBarTextStyle;
    s.fTitleBarTextTitle = !!m_bTitleBarTextTitle;
    s.iOnTop = m_iAlwaysOnTop;
    s.fTrayIcon = !!m_fTrayIcon;
    s.fRememberWindowPos = !!m_fRememberWindowPos;
    s.fRememberWindowSize = !!m_fRememberWindowSize;
    s.fSavePnSZoom = !!m_fSavePnSZoom;
    s.fSnapToDesktopEdges = !!m_fSnapToDesktopEdges;
    s.fKeepHistory = !!m_fKeepHistory;
    s.fHideCDROMsSubMenu = !!m_fHideCDROMsSubMenu;
    s.dwPriority = m_priority ? ABOVE_NORMAL_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
    s.fShowOSD = !!m_fShowOSD;
    s.fLimitWindowProportions = !!m_fLimitWindowProportions;
    s.fRememberDVDPos = !!m_fRememberDVDPos;
    s.fRememberFilePos = !!m_fRememberFilePos;
    s.bRememberPlaylistItems = !!m_bRememberPlaylistItems;

    if (!m_fKeepHistory) {
        // Empty MPC-HC's recent menu (iterating reverse because the indexes change)
        for (int i = s.MRU.GetSize() - 1; i >= 0; i--) {
            s.MRU.Remove(i);
        }
        for (int i = s.MRUDub.GetSize() - 1; i >= 0; i--) {
            s.MRUDub.Remove(i);
        }
        s.MRU.WriteList();
        s.MRUDub.WriteList();

        // Empty the "Recent" jump list
        CComPtr<IApplicationDestinations> pDests;
        HRESULT hr = pDests.CoCreateInstance(CLSID_ApplicationDestinations, nullptr, CLSCTX_INPROC_SERVER);
        if (SUCCEEDED(hr)) {
            pDests->RemoveAllDestinations();
        }

        // Ensure no new items are added in Windows recent menu and in the "Recent" jump list
        CFileAssoc::SetNoRecentDocs(true, true);
    } else {
        // Re-enable Windows recent menu and the "Recent" jump list if needed
        CFileAssoc::SetNoRecentDocs(false, true);
    }
    if (!m_fKeepHistory || !m_fRememberFilePos) {
        s.filePositions.Empty();
    }
    if (!m_fKeepHistory || !m_fRememberDVDPos) {
        s.dvdPositions.Empty();
    }

    // Check if the settings location needs to be changed
    if (AfxGetMyApp()->IsIniValid() != !!m_fUseIni) {
        AfxGetMyApp()->ChangeSettingsLocation(!!m_fUseIni);
    }

    ((CMainFrame*)AfxGetMainWnd())->ShowTrayIcon(s.fTrayIcon);

    ::SetPriorityClass(::GetCurrentProcess(), s.dwPriority);

    GetDlgItem(IDC_FILE_POS)->EnableWindow(s.fKeepHistory);
    GetDlgItem(IDC_DVD_POS)->EnableWindow(s.fKeepHistory);

    return __super::OnApply();
}

void CPPagePlayer::OnUpdateCheck13(CCmdUI* pCmdUI)
{
    UpdateData();

    pCmdUI->Enable(m_iTitleBarTextStyle == 1);
}

void CPPagePlayer::OnUpdatePos(CCmdUI* pCmdUI)
{
    UpdateData();

    pCmdUI->Enable(!!m_fKeepHistory);
}
