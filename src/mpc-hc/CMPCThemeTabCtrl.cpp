#include "stdafx.h"
#include "CMPCThemeTabCtrl.h"
#include "mplayerc.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"


CMPCThemeTabCtrl::CMPCThemeTabCtrl():CTabCtrl() {
}

CMPCThemeTabCtrl::~CMPCThemeTabCtrl() {
}

void CMPCThemeTabCtrl::PreSubclassWindow() {
}

IMPLEMENT_DYNAMIC(CMPCThemeTabCtrl, CTabCtrl)
BEGIN_MESSAGE_MAP(CMPCThemeTabCtrl, CTabCtrl)
    ON_WM_CTLCOLOR()
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
END_MESSAGE_MAP()


HBRUSH CMPCThemeTabCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        return getCtlColor(pDC, pWnd, nCtlColor);
    } else {
        HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
        return hbr;
    }
}

void CMPCThemeTabCtrl::doDrawItem(int nItem, CRect rText, bool isSelected, CDC* pDC) {
    if (nItem != -1) {
        TCITEM tcitem = { 0 };
        tcitem.mask = TCIF_TEXT | TCIF_STATE;
        const int c_cchBuffer = 1024;
        TCHAR  lpBuffer[c_cchBuffer];
        tcitem.pszText = lpBuffer;
        tcitem.cchTextMax = c_cchBuffer;

        GetItem(nItem, &tcitem);

        COLORREF oldTextColor = pDC->GetTextColor();
        COLORREF oldBkColor = pDC->GetBkColor();

        COLORREF textColor = CMPCTheme::TextFGColor;
        COLORREF bgColor;

        CRect rBorder;
        rBorder = rText;

        int leftY, rightY;
        if (!isSelected) {
            bgColor = CMPCTheme::TabCtrlInactiveColor;
            rightY = rBorder.bottom;
        } else {
            bgColor = CMPCTheme::WindowBGColor;
            rightY = rBorder.bottom - 1;
        }

        if (nItem != 0) {
            leftY = rBorder.bottom - 2; //starts above the horizontal border
        } else {
            leftY = rBorder.bottom - 1; //starts on the border to connect with the main border
        }

        pDC->FillSolidRect(rBorder, bgColor);

        CPen borderPen, *oldPen;
        borderPen.CreatePen(PS_SOLID, 1, CMPCTheme::TabCtrlBorderColor);
        oldPen = pDC->SelectObject(&borderPen);
        pDC->MoveTo(rBorder.left, leftY);
        pDC->LineTo(rBorder.left, rBorder.top);
        pDC->LineTo(rBorder.right, rBorder.top);
        pDC->LineTo(rBorder.right, rightY); //non-inclusive

        pDC->SelectObject(oldPen);

        CPoint ptCursor;
        ::GetCursorPos(&ptCursor);
        ScreenToClient(&ptCursor);

        UINT textFormat = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
        textFormat |= DT_LEFT;
        rText.left += 6;

        CString text = tcitem.pszText;
        pDC->SetTextColor(textColor);
        pDC->SetBkColor(bgColor);

        CMPCThemeUtil::DrawBufferedText(pDC, text, rText, textFormat);

        pDC->SetTextColor(oldTextColor);
        pDC->SetBkColor(oldBkColor);
    }

}

void CMPCThemeTabCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
    CDC dc;
    dc.Attach(lpDrawItemStruct->hDC);
    doDrawItem(lpDrawItemStruct->itemID, lpDrawItemStruct->rcItem, 0 != (lpDrawItemStruct->itemState & ODS_SELECTED), &dc);
    dc.Detach();
}

BOOL CMPCThemeTabCtrl::OnEraseBkgnd(CDC* pDC) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CRect r;
        GetClientRect(r);
        CMPCThemeUtil::drawParentDialogBGClr(this, pDC, r);
    }
    return TRUE;
}



void CMPCThemeTabCtrl::OnPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CPaintDC dc(this); // device context for painting
        int oldDC = dc.SaveDC();

        dc.SelectObject(GetFont());

        DRAWITEMSTRUCT dItemStruct;
        dItemStruct.CtlType = ODT_TAB;
        dItemStruct.CtlID = GetDlgCtrlID();
        dItemStruct.hwndItem = GetSafeHwnd();
        dItemStruct.hDC = dc.GetSafeHdc();
        dItemStruct.itemAction = ODA_DRAWENTIRE;

        CRect rClient, rContent;
        GetClientRect(&dItemStruct.rcItem);
        rContent = dItemStruct.rcItem;
        AdjustRect(FALSE, rContent);
        dItemStruct.rcItem.top = rContent.top - 2;

        COLORREF oldTextColor = dc.GetTextColor();
        COLORREF oldBkColor = dc.GetBkColor();

        CBrush contentFrameBrush;
        contentFrameBrush.CreateSolidBrush(CMPCTheme::TabCtrlBorderColor);
        rContent.InflateRect(1, 1);
        dc.FrameRect(rContent, &CMPCThemeUtil::windowBrush);
        rContent.InflateRect(1, 1);
        dc.FrameRect(rContent, &contentFrameBrush);

        dc.SetTextColor(oldTextColor);
        dc.SetBkColor(oldBkColor);


        int nTab = GetItemCount();
        int nSel = GetCurSel();

        if (!nTab) return;

        while (nTab--) {
            if (nTab != nSel) {
                dItemStruct.itemID = nTab;
                dItemStruct.itemState = 0;

                VERIFY(GetItemRect(nTab, &dItemStruct.rcItem));
                DrawItem(&dItemStruct);
            }
        }

        dItemStruct.itemID = nSel;
        dItemStruct.itemState = ODS_SELECTED;

        VERIFY(GetItemRect(nSel, &dItemStruct.rcItem));

        dItemStruct.rcItem.bottom += 2;
        dItemStruct.rcItem.top -= 2;
        DrawItem(&dItemStruct);

        dc.RestoreDC(oldDC);
    } else {
        __super::OnPaint();
    }
}

