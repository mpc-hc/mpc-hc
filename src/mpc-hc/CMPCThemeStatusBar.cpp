#include "stdafx.h"
#include "CMPCThemeStatusBar.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"

CMPCThemeStatusBar::CMPCThemeStatusBar()
{
}


CMPCThemeStatusBar::~CMPCThemeStatusBar()
{
}

void CMPCThemeStatusBar::PreSubclassWindow()
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        ModifyStyleEx(WS_BORDER, WS_EX_STATICEDGE, 0);
    } else {
        __super::PreSubclassWindow();
    }
}


BEGIN_MESSAGE_MAP(CMPCThemeStatusBar, CStatusBar)
    ON_WM_NCPAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CMPCThemeStatusBar::SetText(LPCTSTR lpszText, int nPane, int nType)
{
    CStatusBarCtrl& ctrl = GetStatusBarCtrl();

    if (AfxGetAppSettings().bMPCThemeLoaded) {
        ctrl.SetText(_T(""), nPane, SBT_OWNERDRAW);
        texts[nPane] = lpszText;
        Invalidate();
    } else {
        ctrl.SetText(lpszText, nPane, nType);
    }
}

BOOL CMPCThemeStatusBar::SetParts(int nParts, int* pWidths)
{
    CStatusBarCtrl& ctrl = GetStatusBarCtrl();
    numParts = nParts;
    return ctrl.SetParts(nParts, pWidths);
}

int CMPCThemeStatusBar::GetParts(int nParts, int* pParts)
{
    CStatusBarCtrl& ctrl = GetStatusBarCtrl();
    return ctrl.GetParts(nParts, pParts);
}

BOOL CMPCThemeStatusBar::GetRect(int nPane, LPRECT lpRect)
{
    CStatusBarCtrl& ctrl = GetStatusBarCtrl();
    return ctrl.GetRect(nPane, lpRect);
}


void CMPCThemeStatusBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC dc;
    dc.Attach(lpDrawItemStruct->hDC);
    CRect rect(&lpDrawItemStruct->rcItem);
    int item = lpDrawItemStruct->itemID;
    rect.left += 4;
    dc.SetBkColor(CMPCTheme::StatusBarBGColor);
    dc.SetTextColor(CMPCTheme::TextFGColor);
    CFont font;
    if (CMPCThemeUtil::getFontByType(font, &dc, CMPCThemeUtil::MessageFont)) {
        dc.SelectObject(&font);
    }
    dc.FillSolidRect(rect, CMPCTheme::StatusBarBGColor);
    dc.DrawText(texts[item], rect, 0);
    if (item < numParts - 1) { //draw a separator
        CRect separator(rect.right, rect.top, rect.right + 1, rect.bottom);
        dc.OffsetClipRgn(1, 0); //separator is 1 pixel beyond our rect
        dc.FillSolidRect(separator, CMPCTheme::StatusBarSeparatorColor);
    }
    dc.Detach();
}


void CMPCThemeStatusBar::OnNcPaint()
{
    if (!AfxGetAppSettings().bMPCThemeLoaded) {
        return __super::OnNcPaint();
    } else {
        CWindowDC dc(this);

        CRect rcWindow;
        GetWindowRect(rcWindow);
        ScreenToClient(rcWindow);
        rcWindow.OffsetRect(-rcWindow.TopLeft());
        dc.FillSolidRect(rcWindow, CMPCTheme::StatusBarBGColor);
    }
}


BOOL CMPCThemeStatusBar::OnEraseBkgnd(CDC* pDC)
{
    if (!AfxGetAppSettings().bMPCThemeLoaded) {
        return __super::OnEraseBkgnd(pDC);
    } else {
        return TRUE;
    }
}
