/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2016 see Authors.txt
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
#include "FavoriteOrganizeDlg.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include <strsafe.h>
#include "CMPCTheme.h"

// CFavoriteOrganizeDlg dialog

//IMPLEMENT_DYNAMIC(CFavoriteOrganizeDlg, CMPCThemeResizableDialog)
CFavoriteOrganizeDlg::CFavoriteOrganizeDlg(CWnd* pParent /*=nullptr*/)
    : CMPCThemeResizableDialog(CFavoriteOrganizeDlg::IDD, pParent)
{
}

CFavoriteOrganizeDlg::~CFavoriteOrganizeDlg()
{
}

void CFavoriteOrganizeDlg::SetupList(bool fSave)
{
    int i = m_tab.GetCurSel();

    if (fSave) {
        CAtlList<CString> sl;

        for (int j = 0; j < m_list.GetItemCount(); j++) {
            CAtlList<CString> args;
            ExplodeEsc(m_sl[i].GetAt((POSITION)m_list.GetItemData(j)), args, _T(';'));
            args.RemoveHead();
            args.AddHead(m_list.GetItemText(j, 0));
            sl.AddTail(ImplodeEsc(args, _T(';')));
        }

        m_sl[i].RemoveAll();
        m_sl[i].AddTailList(&sl);
    } else {
        m_list.DeleteAllItems();

        POSITION pos = m_sl[i].GetHeadPosition(), tmp;
        while (pos) {
            tmp = pos;

            CAtlList<CString> sl;
            ExplodeEsc(m_sl[i].GetNext(pos), sl, _T(';'), 3);

            int n = m_list.InsertItem(m_list.GetItemCount(), sl.RemoveHead());
            m_list.SetItemData(n, (DWORD_PTR)tmp);

            if (!sl.IsEmpty()) {
                REFERENCE_TIME rt = 0;
                if (1 == _stscanf_s(sl.GetHead(), _T("%I64d"), &rt) && rt > 0) {
                    DVD_HMSF_TIMECODE hmsf = RT2HMSF(rt);

                    CString str;
                    str.Format(_T("[%02u:%02u:%02u]"), hmsf.bHours, hmsf.bMinutes, hmsf.bSeconds);
                    m_list.SetItemText(n, 1, str);
                }
            }
        }

        UpdateColumnsSizes();
    }
}

void CFavoriteOrganizeDlg::UpdateColumnsSizes()
{
    CRect r;
    m_list.GetClientRect(r);
    m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
    m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
    m_list.SetColumnWidth(1, std::max(m_list.GetColumnWidth(1), r.Width() - m_list.GetColumnWidth(0)));
}

void CFavoriteOrganizeDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB1, m_tab);
    DDX_Control(pDX, IDC_LIST2, m_list);
}


BEGIN_MESSAGE_MAP(CFavoriteOrganizeDlg, CMPCThemeResizableDialog)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
    ON_WM_DRAWITEM()
    ON_BN_CLICKED(IDC_BUTTON1, OnRenameBnClicked)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateRenameBn)
    ON_BN_CLICKED(IDC_BUTTON2, OnDeleteBnClicked)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateDeleteBn)
    ON_BN_CLICKED(IDC_BUTTON3, OnUpBnClicked)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON3, OnUpdateUpBn)
    ON_BN_CLICKED(IDC_BUTTON4, OnDownBnClicked)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON4, OnUpdateDownBn)
    ON_NOTIFY(TCN_SELCHANGING, IDC_TAB1, OnTcnSelchangingTab1)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_WM_ACTIVATE()
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST2, OnLvnEndlabeleditList2)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST2, OnPlayFavorite)
    ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, OnKeyPressed)
    ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST2, OnLvnGetInfoTipList)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CFavoriteOrganizeDlg message handlers

BOOL CFavoriteOrganizeDlg::OnInitDialog()
{
    __super::OnInitDialog();

    m_tab.InsertItem(0, ResStr(IDS_FAVFILES));
    m_tab.InsertItem(1, ResStr(IDS_FAVDVDS));
    //  m_tab.InsertItem(2, ResStr(IDS_FAVDEVICES));
    m_tab.SetCurSel(0);

    m_list.InsertColumn(0, _T(""));
    m_list.InsertColumn(1, _T(""));
    m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_INFOTIP);
    m_list.setAdditionalStyles(LVS_EX_FULLROWSELECT);

    const CAppSettings& s = AfxGetAppSettings();
    s.GetFav(FAV_FILE, m_sl[0]);
    s.GetFav(FAV_DVD, m_sl[1]);
    s.GetFav(FAV_DEVICE, m_sl[2]);

    SetupList(false);

    AddAnchor(IDC_TAB1, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_LIST2, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_BUTTON1, TOP_RIGHT);
    AddAnchor(IDC_BUTTON2, TOP_RIGHT);
    AddAnchor(IDC_BUTTON3, TOP_RIGHT);
    AddAnchor(IDC_BUTTON4, TOP_RIGHT);
    AddAnchor(IDOK, BOTTOM_RIGHT);
    EnableSaveRestore(IDS_R_DLG_ORGANIZE_FAV);
    fulfillThemeReqs();
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFavoriteOrganizeDlg::PreTranslateMessage(MSG* pMsg)
{
    // Inhibit default handling for the Enter key when the list has the focus and an item is selected.
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN
            && pMsg->hwnd == m_list.GetSafeHwnd() && m_list.GetSelectedCount() > 0) {
        return FALSE;
    }

    return __super::PreTranslateMessage(pMsg);
}

void CFavoriteOrganizeDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
    SetupList(false);

    m_list.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    *pResult = 0;
}

void CFavoriteOrganizeDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (nIDCtl != IDC_LIST2) {
        return;
    }

    int nItem = lpDrawItemStruct->itemID;
    CRect rcItem = lpDrawItemStruct->rcItem;

    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

    if (!!m_list.GetItemState(nItem, LVIS_SELECTED)) {
        if (AfxGetAppSettings().bMPCThemeLoaded) {
            CBrush b(CMPCTheme::ContentSelectedColor);
            pDC->FillRect(rcItem, &b);
        } else {
            CBrush b1, b2;
            b1.CreateSolidBrush(0xf1dacc);
            pDC->FillRect(rcItem, &b1);
            b2.CreateSolidBrush(0xc56a31);
            pDC->FrameRect(rcItem, &b2);
        }
    } else {
        if (AfxGetAppSettings().bMPCThemeLoaded) {
            CBrush b(CMPCTheme::ContentBGColor);
            pDC->FillRect(rcItem, &b);
        } else {
            CBrush b;
            b.CreateSysColorBrush(COLOR_WINDOW);
            pDC->FillRect(rcItem, &b);
        }
    }

    COLORREF textcolor;
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        textcolor = CMPCTheme::TextFGColor;
    } else {
        textcolor = 0;
    }
    pDC->SetTextColor(textcolor);

    CString str;
    str = m_list.GetItemText(nItem, 0);
    pDC->TextOut(rcItem.left + 3, (rcItem.top + rcItem.bottom - pDC->GetTextExtent(str).cy) / 2, str);
    str = m_list.GetItemText(nItem, 1);
    if (!str.IsEmpty()) {
        pDC->TextOut(rcItem.right - pDC->GetTextExtent(str).cx - 3, (rcItem.top + rcItem.bottom - pDC->GetTextExtent(str).cy) / 2, str);
    }
}

void CFavoriteOrganizeDlg::OnRenameBnClicked()
{
    if (POSITION pos = m_list.GetFirstSelectedItemPosition()) {
        m_list.SetFocus();
        m_list.EditLabel(m_list.GetNextSelectedItem(pos));
    }
}

void CFavoriteOrganizeDlg::OnUpdateRenameBn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_list.GetSelectedCount() == 1);
}

void CFavoriteOrganizeDlg::OnLvnEndlabeleditList2(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    if (pDispInfo->item.iItem >= 0 && pDispInfo->item.pszText) {
        m_list.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);
    }
    UpdateColumnsSizes();

    *pResult = 0;
}

void CFavoriteOrganizeDlg::PlayFavorite(int nItem)
{
    switch (m_tab.GetCurSel()) {
        case 0: // Files
            ((CMainFrame*)GetParentFrame())->PlayFavoriteFile(m_sl[0].GetAt((POSITION)m_list.GetItemData(nItem)));
            break;
        case 1: // DVDs
            ((CMainFrame*)GetParentFrame())->PlayFavoriteDVD(m_sl[1].GetAt((POSITION)m_list.GetItemData(nItem)));
            break;
        case 2: // Devices
            break;
    }
}

void CFavoriteOrganizeDlg::OnPlayFavorite(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    if (pItemActivate->iItem >= 0) {
        PlayFavorite(pItemActivate->iItem);
    }
}

void CFavoriteOrganizeDlg::OnKeyPressed(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

    switch (pLVKeyDow->wVKey) {
        case VK_DELETE:
        case VK_BACK:
            OnDeleteBnClicked();
            *pResult = 1;
            break;
        case VK_RETURN:
            if (POSITION pos = m_list.GetFirstSelectedItemPosition()) {
                int nItem = m_list.GetNextSelectedItem(pos);
                if (nItem >= 0 && nItem < m_list.GetItemCount()) {
                    PlayFavorite(nItem);
                }
            }
            *pResult = 1;
            break;
        case 'A':
            if (GetKeyState(VK_CONTROL) < 0) { // If the high-order bit is 1, the key is down; otherwise, it is up.
                m_list.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
            }
            *pResult = 1;
            break;
        case 'I':
            if (GetKeyState(VK_CONTROL) < 0) { // If the high-order bit is 1, the key is down; otherwise, it is up.
                for (int nItem = 0; nItem < m_list.GetItemCount(); nItem++) {
                    m_list.SetItemState(nItem, ~m_list.GetItemState(nItem, LVIS_SELECTED), LVIS_SELECTED);
                }
            }
            *pResult = 1;
            break;
        default:
            *pResult = 0;
    }
}

void CFavoriteOrganizeDlg::OnDeleteBnClicked()
{
    POSITION pos;
    int nItem = -1;

    while ((pos = m_list.GetFirstSelectedItemPosition()) != nullptr) {
        nItem = m_list.GetNextSelectedItem(pos);
        if (nItem < 0 || nItem >= m_list.GetItemCount()) {
            return;
        }

        m_list.DeleteItem(nItem);
    }

    nItem = std::min(nItem, m_list.GetItemCount() - 1);
    m_list.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
}

void CFavoriteOrganizeDlg::OnUpdateDeleteBn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_list.GetSelectedCount() > 0);
}

void CFavoriteOrganizeDlg::MoveItem(int nItem, int offset)
{
    DWORD_PTR data = m_list.GetItemData(nItem);
    CString strName = m_list.GetItemText(nItem, 0);
    CString strPos = m_list.GetItemText(nItem, 1);

    m_list.DeleteItem(nItem);

    nItem += offset;

    m_list.InsertItem(nItem, strName);
    m_list.SetItemData(nItem, data);
    m_list.SetItemText(nItem, 1, strPos);
    m_list.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
}

void CFavoriteOrganizeDlg::OnUpBnClicked()
{
    POSITION pos = m_list.GetFirstSelectedItemPosition();

    while (pos) {
        int nItem = m_list.GetNextSelectedItem(pos);
        if (nItem <= 0 || nItem >= m_list.GetItemCount()) {
            return;
        }

        MoveItem(nItem, -1);
    }
}

void CFavoriteOrganizeDlg::OnUpdateUpBn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_list.GetSelectedCount() > 0 && !m_list.GetItemState(0, LVIS_SELECTED));
}

void CFavoriteOrganizeDlg::OnDownBnClicked()
{
    CArray<int> selectedItems;
    POSITION pos = m_list.GetFirstSelectedItemPosition();

    while (pos) {
        int nItem = m_list.GetNextSelectedItem(pos);
        if (nItem < 0 || nItem >= m_list.GetItemCount() - 1) {
            return;
        }

        selectedItems.Add(nItem);
    }

    for (INT_PTR i = selectedItems.GetSize() - 1; i >= 0; i--) {
        MoveItem(selectedItems[i], +1);
    }
}

void CFavoriteOrganizeDlg::OnUpdateDownBn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_list.GetSelectedCount() > 0 && !m_list.GetItemState(m_list.GetItemCount() - 1, LVIS_SELECTED));
}

void CFavoriteOrganizeDlg::OnTcnSelchangingTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
    SetupList(true);

    *pResult = 0;
}

void CFavoriteOrganizeDlg::OnBnClickedOk()
{
    SetupList(true);

    CAppSettings& s = AfxGetAppSettings();
    s.SetFav(FAV_FILE, m_sl[0]);
    s.SetFav(FAV_DVD, m_sl[1]);
    s.SetFav(FAV_DEVICE, m_sl[2]);

    OnOK();
}

void CFavoriteOrganizeDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    __super::OnActivate(nState, pWndOther, bMinimized);

    if (nState == WA_ACTIVE) {
        m_list.SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void CFavoriteOrganizeDlg::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);

    if (IsWindow(m_list)) {
        m_list.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
    }
}

void CFavoriteOrganizeDlg::OnLvnGetInfoTipList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);

    CAtlList<CString> args;
    ExplodeEsc(m_sl[m_tab.GetCurSel()].GetAt((POSITION)m_list.GetItemData(pGetInfoTip->iItem)), args, _T(';'));
    CString path = args.RemoveTail();
    // Relative to drive value is always third. If less args are available that means it is not included.
    int rootLength = (args.GetCount() == 3 && args.RemoveTail() != _T("0")) ? CPath(path).SkipRoot() : 0;

    StringCchCopy(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, path.Mid(rootLength));

    *pResult = 0;
}
