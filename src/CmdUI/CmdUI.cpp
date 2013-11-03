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

// CmdUI.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>
#include "CmdUI.h"

// CCmdUIDialog dialog

IMPLEMENT_DYNAMIC(CCmdUIDialog, CDialog)

CCmdUIDialog::CCmdUIDialog()
{
}

CCmdUIDialog::CCmdUIDialog(UINT nIDTemplate, CWnd* pParent /*=nullptr*/)
    : CDialog(nIDTemplate, pParent)
{
}

CCmdUIDialog::CCmdUIDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
    : CDialog(lpszTemplateName, pParentWnd)
{
}

CCmdUIDialog::~CCmdUIDialog()
{
}

LRESULT CCmdUIDialog::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = __super::DefWindowProc(message, wParam, lParam);

    if (message == WM_INITDIALOG) {
        SendMessage(WM_KICKIDLE);
    }

    return ret;
}

BEGIN_MESSAGE_MAP(CCmdUIDialog, CDialog)
    ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
    ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()


// CCmdUIDialog message handlers

void CCmdUIDialog::OnKickIdle()
{
    UpdateDialogControls(this, false);

    // TODO: maybe we should send this call to modeless child cdialogs too
}

// Q242577

void CCmdUIDialog::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
    ASSERT(pPopupMenu != nullptr);
    // Check the enabled state of various menu items.

    CCmdUI state;
    state.m_pMenu = pPopupMenu;
    ASSERT(state.m_pOther == nullptr);
    ASSERT(state.m_pParentMenu == nullptr);

    // Determine if menu is popup in top-level menu and set m_pOther to
    // it if so (m_pParentMenu == nullptr) indicates that it is secondary popup.
    if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu) {
        state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
    } else if (::GetMenu(m_hWnd) != nullptr) {
        HMENU hParentMenu;
        CWnd* pParent = this;
        // Child windows don't have menus--need to go to the top!
        if (pParent != nullptr &&
                (hParentMenu = ::GetMenu(pParent->m_hWnd)) != nullptr) {
            int nIndexMax = ::GetMenuItemCount(hParentMenu);
            for (int nIndex = 0; nIndex < nIndexMax; nIndex++) {
                if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu) {
                    // When popup is found, m_pParentMenu is containing menu.
                    state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
                    break;
                }
            }
        }
    }

    state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
            state.m_nIndex++) {
        state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0) {
            continue;    // Menu separator or invalid cmd - ignore it.
        }

        ASSERT(state.m_pOther == nullptr);
        ASSERT(state.m_pMenu != nullptr);
        if (state.m_nID == UINT(-1)) {
            // Possibly a popup menu, route to first item of that popup.
            state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
            if (state.m_pSubMenu == nullptr ||
                    (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                    state.m_nID == UINT(-1)) {
                continue;       // First item of popup can't be routed to.
            }
            state.DoUpdate(this, TRUE);     // Popups are never auto disabled.
        } else {
            // Normal menu item.
            // Auto enable/disable if frame window has m_bAutoMenuEnable
            // set and command is _not_ a system command.
            state.m_pSubMenu = nullptr;
            state.DoUpdate(this, FALSE);
        }

        // Adjust for menu deletions and additions.
        UINT nCount = pPopupMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax) {
            state.m_nIndex -= (state.m_nIndexMax - nCount);
            while (state.m_nIndex < nCount &&
                    pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID) {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}

// CCmdUIPropertyPage

IMPLEMENT_DYNAMIC(CCmdUIPropertyPage, CPropertyPage)
CCmdUIPropertyPage::CCmdUIPropertyPage(UINT nIDTemplate, UINT nIDCaption)
    : CPropertyPage(nIDTemplate, nIDCaption)
{
}

CCmdUIPropertyPage::~CCmdUIPropertyPage()
{
}

LRESULT CCmdUIPropertyPage::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_COMMAND) {
        switch (HIWORD(wParam)) {
            case BN_CLICKED:
            case CBN_SELCHANGE:
            case EN_CHANGE:
                SetModified();
            default:
                ;
        }
    }

    LRESULT ret = __super::DefWindowProc(message, wParam, lParam);

    if (message == WM_INITDIALOG) {
        SendMessage(WM_KICKIDLE);
    }

    return ret;
}

BEGIN_MESSAGE_MAP(CCmdUIPropertyPage, CPropertyPage)
    ON_MESSAGE_VOID(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


// CCmdUIPropertyPage message handlers

void CCmdUIPropertyPage::OnKickIdle()
{
    UpdateDialogControls(this, false);

    // TODO: maybe we should send this call to modeless child cPropertyPages too
}
