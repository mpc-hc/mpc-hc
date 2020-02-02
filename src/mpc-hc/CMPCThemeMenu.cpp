#include "stdafx.h"
#include "CMPCThemeMenu.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"
#include <strsafe.h>
#include "AppSettings.h"
#include "PPageAccelTbl.h"
#include "mplayerc.h"

std::map<UINT, CMPCThemeMenu*> CMPCThemeMenu::subMenuIDs;


IMPLEMENT_DYNAMIC(CMPCThemeMenu, CMenu);

bool CMPCThemeMenu::hasDimensions = false;
int CMPCThemeMenu::subMenuPadding;
int CMPCThemeMenu::iconSpacing;
int CMPCThemeMenu::iconPadding;
int CMPCThemeMenu::rowPadding;
int CMPCThemeMenu::separatorPadding;
int CMPCThemeMenu::separatorHeight;
int CMPCThemeMenu::postTextSpacing;
int CMPCThemeMenu::accelSpacing;

CMPCThemeMenu::CMPCThemeMenu()
{
}


CMPCThemeMenu::~CMPCThemeMenu()
{
    std::map<UINT, CMPCThemeMenu*>::iterator itr = subMenuIDs.begin();
    while (itr != subMenuIDs.end()) {
        if (itr->second == this) {
            itr = subMenuIDs.erase(itr);
        } else {
            ++itr;
        }
    }

    for (u_int i = 0; i < allocatedItems.size(); i++) {
        delete allocatedItems[i];
    }
    for (u_int i = 0; i < allocatedMenus.size(); i++) {
        delete allocatedMenus[i];
    }
}

void CMPCThemeMenu::initDimensions()
{
    if (!hasDimensions) {
        DpiHelper dpi = DpiHelper();
        dpi.Override(AfxGetMainWnd()->GetSafeHwnd());

        subMenuPadding = dpi.ScaleX(20);
        iconSpacing = dpi.ScaleX(22);
        iconPadding = dpi.ScaleX(10);
        rowPadding = dpi.ScaleY(4) + 3; //windows 10 explorer has paddings of 7,8,9,9,11--this yields 7,8,9,10,11
        separatorPadding = dpi.ScaleX(8);
        separatorHeight = dpi.ScaleX(7);
        postTextSpacing = dpi.ScaleX(20);
        accelSpacing = dpi.ScaleX(30);
        hasDimensions = true;
    }
}

UINT CMPCThemeMenu::findID(UINT& nPos, bool byCommand)
{
    int iMaxItems = GetMenuItemCount();

    UINT nID;
    if (byCommand) {
        nID = nPos;
        bool found = false;
        for (int j = 0; j < iMaxItems; j++) {
            if (nID == GetMenuItemID(j)) {
                nPos = j;
                found = true;
                break;
            }
        }
        if (!found) {
            return (UINT) - 1;
        }
    } else {
        nID = GetMenuItemID(nPos);
        if (nID == 0xFFFFFFFF) { //submenu, have to find the old-fashioned way
            MENUITEMINFO mii = { sizeof(mii) };
            mii.fMask = MIIM_ID;
            GetMenuItemInfo(nPos, &mii, TRUE);
            nID = mii.wID;
        }
    }
    return nID;
}

void CMPCThemeMenu::cleanupItem(UINT nPosition, UINT nFlags)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        UINT nID = findID(nPosition, 0 == (nFlags & MF_BYPOSITION));

        MENUITEMINFO tInfo = { sizeof(MENUITEMINFO) };
        tInfo.fMask = MIIM_DATA;
        GetMenuItemInfo(nID, &tInfo);
        MenuObject* pObject = (MenuObject*)tInfo.dwItemData;
        if (std::find(allocatedItems.begin(), allocatedItems.end(), pObject) != allocatedItems.end()) {
            allocatedItems.erase(std::remove(allocatedItems.begin(), allocatedItems.end(), pObject), allocatedItems.end());
            delete pObject;
        }
    }
}

BOOL CMPCThemeMenu::DeleteMenu(UINT nPosition, UINT nFlags)
{
    cleanupItem(nPosition, nFlags);
    return CMenu::DeleteMenu(nPosition, nFlags);
}

BOOL CMPCThemeMenu::RemoveMenu(UINT nPosition, UINT nFlags)
{
    cleanupItem(nPosition, nFlags);
    return CMenu::RemoveMenu(nPosition, nFlags);
}

BOOL CMPCThemeMenu::AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem)
{
    BOOL ret = CMenu::AppendMenu(nFlags, nIDNewItem, lpszNewItem);
    fulfillThemeReqsItem((UINT)nIDNewItem);
    return ret;
}

void CMPCThemeMenu::fulfillThemeReqs(bool isMenubar)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        MENUINFO oldInfo = { sizeof(MENUINFO) };
        oldInfo.fMask = MIM_STYLE;
        GetMenuInfo(&oldInfo);

        MENUINFO MenuInfo = { 0 };
        MenuInfo.cbSize = sizeof(MENUINFO);
        MenuInfo.fMask = MIM_BACKGROUND | MIM_STYLE | MIM_APPLYTOSUBMENUS;
        MenuInfo.dwStyle = oldInfo.dwStyle;
        MenuInfo.hbrBack = ::CreateSolidBrush(CMPCTheme::MenuBGColor);
        SetMenuInfo(&MenuInfo);

        int iMaxItems = GetMenuItemCount();
        for (int i = 0; i < iMaxItems; i++) {
            CString nameHolder;
            MenuObject* pObject = new MenuObject;
            allocatedItems.push_back(pObject);
            pObject->m_hIcon = NULL;
            pObject->isMenubar = isMenubar;
            if (i == 0) {
                pObject->isFirstMenuInMenuBar = true;
            }

            GetMenuString(i, pObject->m_strCaption, MF_BYPOSITION);

            UINT nID = GetMenuItemID(i);
            pObject->m_strAccel = CPPageAccelTbl::MakeAccelShortcutLabel(nID);

            subMenuIDs[nID] = this;

            MENUITEMINFO tInfo;
            ZeroMemory(&tInfo, sizeof(MENUITEMINFO));
            tInfo.fMask = MIIM_FTYPE;
            tInfo.cbSize = sizeof(MENUITEMINFO);
            GetMenuItemInfo(i, &tInfo, true);

            if (tInfo.fType & MFT_SEPARATOR) {
                pObject->isSeparator = true;
            }

            MENUITEMINFO mInfo;
            ZeroMemory(&mInfo, sizeof(MENUITEMINFO));

            mInfo.fMask = MIIM_FTYPE | MIIM_DATA;
            mInfo.fType = MFT_OWNERDRAW | tInfo.fType;
            mInfo.cbSize = sizeof(MENUITEMINFO);
            mInfo.dwItemData = (ULONG_PTR)pObject;
            SetMenuItemInfo(i, &mInfo, true);

            CMenu* t = GetSubMenu(i);
            if (nullptr != t) {
                CMPCThemeMenu* pSubMenu = new CMPCThemeMenu;
                allocatedMenus.push_back(pSubMenu);
                pSubMenu->Attach(t->GetSafeHmenu());
                pSubMenu->fulfillThemeReqs();
            }
        }
    }
}

void CMPCThemeMenu::fulfillThemeReqsItem(UINT i, bool byCommand)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        MENUITEMINFO tInfo = { sizeof(MENUITEMINFO) };
        tInfo.fMask = MIIM_DATA;
        GetMenuItemInfo(i, &tInfo, !byCommand);
        if (NULL == tInfo.dwItemData) {
            CString nameHolder;
            MenuObject* pObject = new MenuObject;
            allocatedItems.push_back(pObject);
            pObject->m_hIcon = NULL;

            UINT posOrCmd = byCommand ? MF_BYCOMMAND : MF_BYPOSITION;

            GetMenuString(i, pObject->m_strCaption, posOrCmd);

            UINT nPos = i;
            UINT nID = findID(nPos, byCommand);
            if (nID == -1) {
                return;
            }

            pObject->m_strAccel = CPPageAccelTbl::MakeAccelShortcutLabel(nID);

            subMenuIDs[nID] = this;

            ZeroMemory(&tInfo, sizeof(MENUITEMINFO));
            tInfo.fMask = MIIM_FTYPE;
            tInfo.cbSize = sizeof(MENUITEMINFO);
            GetMenuItemInfo(nPos, &tInfo, true);

            if (tInfo.fType & MFT_SEPARATOR) {
                pObject->isSeparator = true;
            }

            MENUITEMINFO mInfo;
            ZeroMemory(&mInfo, sizeof(MENUITEMINFO));

            mInfo.fMask = MIIM_FTYPE | MIIM_DATA;
            mInfo.fType = MFT_OWNERDRAW | tInfo.fType;
            mInfo.cbSize = sizeof(MENUITEMINFO);
            mInfo.dwItemData = (ULONG_PTR)pObject;
            SetMenuItemInfo(nPos, &mInfo, true);

            CMenu* t = GetSubMenu(nPos);
            if (nullptr != t) {
                CMPCThemeMenu* pSubMenu = new CMPCThemeMenu;
                allocatedMenus.push_back(pSubMenu);
                pSubMenu->Attach(t->GetSafeHmenu());
                pSubMenu->fulfillThemeReqs();
            }
        }
    }
}

void CMPCThemeMenu::fulfillThemeReqsItem(CMenu* parent, UINT i, bool byCommand)
{
    CMPCThemeMenu* t;
    if ((t = DYNAMIC_DOWNCAST(CMPCThemeMenu, parent)) != nullptr) {
        t->fulfillThemeReqsItem(i, byCommand);
    }
}

UINT CMPCThemeMenu::getPosFromID(CMenu* parent, UINT nID)
{
    int iMaxItems = parent->GetMenuItemCount();
    for (int j = 0; j < iMaxItems; j++) {
        if (nID == parent->GetMenuItemID(j)) {
            return j;
        }
    }
    return (UINT) - 1;
}

CMPCThemeMenu* CMPCThemeMenu::getParentMenu(UINT itemID)
{
    if (subMenuIDs.count(itemID) == 1) {
        CMPCThemeMenu* m = subMenuIDs.at(itemID);
        /* // checks if submenu for overriding of onmeasureitem (win32 limitation).
           // but mpc-hc doesn't set up some submenus until later
           // which is too late for measureitem to take place
           // so we return all items for measuring
        MENUITEMINFO mInfo;
        ZeroMemory(&mInfo, sizeof(MENUITEMINFO));
        mInfo.fMask = MIIM_SUBMENU;
        mInfo.cbSize = sizeof(MENUITEMINFO);
        m->GetMenuItemInfo(itemID, &mInfo);
        if (mInfo.hSubMenu)      //  */
        return m;
    }

    return nullptr;
}

void CMPCThemeMenu::GetRects(RECT rcItem, CRect& rectFull, CRect& rectM, CRect& rectIcon, CRect& rectText, CRect& rectArrow)
{
    rectFull.CopyRect(&rcItem);
    rectM = rectFull;
    rectIcon.SetRect(rectM.left, rectM.top, rectM.left + iconSpacing, rectM.bottom);
    rectText.SetRect(rectM.left + iconSpacing + iconPadding, rectM.top, rectM.right - subMenuPadding, rectM.bottom);
    rectArrow.SetRect(rectM.right - subMenuPadding, rectM.top, rectM.right, rectM.bottom);
}

void CMPCThemeMenu::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{

    MenuObject* menuObject = (MenuObject*)lpDrawItemStruct->itemData;

    MENUITEMINFO mInfo;
    ZeroMemory(&mInfo, sizeof(MENUITEMINFO));

    mInfo.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
    mInfo.cbSize = sizeof(MENUITEMINFO);
    GetMenuItemInfo(lpDrawItemStruct->itemID, &mInfo);

    CRect rectFull;
    CRect rectM;
    CRect rectIcon;
    CRect rectText;
    CRect rectArrow;

    GetRects(lpDrawItemStruct->rcItem, rectFull, rectM, rectIcon, rectText, rectArrow);

    UINT captionAlign = DT_LEFT;


    COLORREF ArrowColor = CMPCTheme::SubmenuColor;
    COLORREF TextFGColor;
    COLORREF TextBGColor = CMPCTheme::MenuBGColor;
    //TextBGColor = R255; //test
    COLORREF TextSelectColor = CMPCTheme::MenuSelectedColor;

    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

    if ((lpDrawItemStruct->itemState & ODS_DISABLED)) {
        TextFGColor = CMPCTheme::MenuItemDisabledColor;
        ArrowColor = CMPCTheme::MenuItemDisabledColor;
    } else {
        TextFGColor = CMPCTheme::TextFGColor;
    }

    int oldBKMode = pDC->SetBkMode(TRANSPARENT);
    pDC->FillSolidRect(&rectM, TextBGColor);

    if (menuObject->isMenubar) {
        if (menuObject->isFirstMenuInMenuBar) { //clean up white borders
            CRect wndSize;
            ::GetClientRect(AfxGetMainWnd()->m_hWnd, &wndSize);

            CRect rectBorder(rectM.left, rectM.bottom, rectM.left + wndSize.Width(), rectM.bottom + 1);
            pDC->FillSolidRect(&rectBorder, CMPCTheme::MenuItemDisabledColor);
            ExcludeClipRect(lpDrawItemStruct->hDC, rectBorder.left, rectBorder.top, rectBorder.right, rectBorder.bottom);
        }
        rectM = rectFull;
        rectText = rectFull;
        captionAlign = DT_CENTER;
    }

    if (mInfo.fType & MFT_SEPARATOR) {
        int centerOffset = (separatorHeight - 1) / 2;
        CRect rectSeparator(rectM.left + separatorPadding, rectM.top + centerOffset, rectM.right - separatorPadding, rectM.top + centerOffset + 1);
        pDC->FillSolidRect(&rectSeparator, CMPCTheme::MenuSeparatorColor);
    } else {



        COLORREF oldTextFGColor = pDC->SetTextColor(TextFGColor);

        CFont* pOldFont = pDC->GetCurrentFont();
        CFont font;
        if (CMPCThemeUtil::getFontByType(font, pDC, CMPCThemeUtil::MenuFont)) {
            pDC->SelectObject(&font);
        }


        if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
            pDC->FillSolidRect(&rectM, TextSelectColor);
        }
        CString left, right;
        GetStrings(menuObject, left, right);

        if (lpDrawItemStruct->itemState & ODS_NOACCEL) { //removing single &s before drawtext
            left.Replace(TEXT("&&"), TEXT("{{amp}}"));
            left.Remove(TEXT('&'));
            left.Replace(TEXT("{{amp}}"), TEXT("&&"));

            pDC->DrawText(left, rectText, DT_VCENTER | captionAlign | DT_SINGLELINE);
        } else {
            pDC->DrawText(left, rectText, DT_VCENTER | captionAlign | DT_SINGLELINE);
        }

        if (!menuObject->isMenubar) {

            if (right.GetLength() > 0) {
                pDC->DrawText(right, rectText, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);
            }


            if (mInfo.hSubMenu) {
                CFont sfont;
                if (CMPCThemeUtil::getFontByFace(sfont, pDC, CMPCTheme::uiSymbolFont, 14, FW_BOLD)) {
                    pDC->SelectObject(&sfont);
                }
                pDC->SetTextColor(ArrowColor);
                pDC->DrawText(TEXT(">"), rectArrow, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
            }

            if (mInfo.fState & MFS_CHECKED) {
                CString check;
                int size;
                if (mInfo.fType & MFT_RADIOCHECK) {
                    check = TEXT("\u25CF"); //bullet
                    size = 6;
                } else {
                    check = TEXT("\u2714"); //checkmark
                    size = 10;
                }
                CFont bFont;
                if (CMPCThemeUtil::getFontByFace(bFont, pDC, CMPCTheme::uiSymbolFont, size, FW_REGULAR)) {
                    pDC->SelectObject(&bFont);
                }
                pDC->SetTextColor(TextFGColor);
                pDC->DrawText(check, rectIcon, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
            }
        }

        pDC->SetBkMode(oldBKMode);
        pDC->SetTextColor(oldTextFGColor);
        pDC->SelectObject(pOldFont);
    }
    ExcludeClipRect(lpDrawItemStruct->hDC, rectFull.left, rectFull.top, rectFull.right, rectFull.bottom);
}

void CMPCThemeMenu::GetStrings(MenuObject* mo, CString& left, CString& right)
{
    if (mo->m_strAccel.GetLength() > 0) {
        left = mo->m_strCaption;
        right = mo->m_strAccel;
    } else {
        CString text = mo->m_strCaption;
        if (!AfxExtractSubString(left, text, 0, _T('\t'))) {
            left = _T("");
        }
        if (!AfxExtractSubString(right, text, 1, _T('\t'))) {
            right = _T("");
        }
    }
}

void CMPCThemeMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    initDimensions();

    HDC hDC = ::GetDC(AfxGetMainWnd()->GetSafeHwnd());
    MenuObject* mo = (MenuObject*)lpMeasureItemStruct->itemData;

    if (mo->isSeparator) {
        lpMeasureItemStruct->itemWidth = 0;
        lpMeasureItemStruct->itemHeight = separatorHeight;
    } else {
        CSize height = CMPCThemeUtil::GetTextSize(_T("W"), hDC, CMPCThemeUtil::MenuFont);
        if (mo->isMenubar) {
            CSize cs = CMPCThemeUtil::GetTextSize(mo->m_strCaption, hDC, CMPCThemeUtil::MenuFont);
            lpMeasureItemStruct->itemWidth = cs.cx;
            lpMeasureItemStruct->itemHeight = height.cy + rowPadding;
        } else {
            CString left, right;
            GetStrings(mo, left, right);
            CSize cs = CMPCThemeUtil::GetTextSize(left, hDC, CMPCThemeUtil::MenuFont);
            lpMeasureItemStruct->itemHeight = height.cy + rowPadding;
            lpMeasureItemStruct->itemWidth = iconSpacing + postTextSpacing + subMenuPadding + cs.cx;
            if (right.GetLength() > 0) {
                CSize csAccel = CMPCThemeUtil::GetTextSize(right, hDC, CMPCThemeUtil::MenuFont);
                lpMeasureItemStruct->itemWidth += accelSpacing + csAccel.cx;
            }
        }
    }
}

CMPCThemeMenu* CMPCThemeMenu::GetSubMenu(int nPos)
{
    return (CMPCThemeMenu*) CMenu::GetSubMenu(nPos);
}

void CMPCThemeMenu::updateItem(CCmdUI* pCmdUI)
{
    CMenu* cm = pCmdUI->m_pMenu;

    if (DYNAMIC_DOWNCAST(CMPCThemeMenu, cm)) {
        MENUITEMINFO mInfo;
        ZeroMemory(&mInfo, sizeof(MENUITEMINFO));

        mInfo.fMask = MIIM_DATA;
        mInfo.cbSize = sizeof(MENUITEMINFO);
        VERIFY(cm->GetMenuItemInfo(pCmdUI->m_nID, &mInfo));

        MenuObject* menuObject = (MenuObject*)mInfo.dwItemData;
        cm->GetMenuString(pCmdUI->m_nID, menuObject->m_strCaption, MF_BYCOMMAND);
    }
}

