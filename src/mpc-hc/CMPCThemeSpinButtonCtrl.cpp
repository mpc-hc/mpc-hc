#include "stdafx.h"
#include "CMPCThemeSpinButtonCtrl.h"
#include "CMPCTheme.h"
#include "CMPCThemeEdit.h"

CMPCThemeSpinButtonCtrl::CMPCThemeSpinButtonCtrl() {
}


CMPCThemeSpinButtonCtrl::~CMPCThemeSpinButtonCtrl() {
}

IMPLEMENT_DYNAMIC(CMPCThemeSpinButtonCtrl, CSpinButtonCtrl)
BEGIN_MESSAGE_MAP(CMPCThemeSpinButtonCtrl, CSpinButtonCtrl)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CMPCThemeSpinButtonCtrl::OnPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CWnd *buddy = GetBuddy();
        bool hasBuddy = false;
        CMPCThemeEdit* buddyEdit;
        if (nullptr != buddy && nullptr != (buddyEdit = DYNAMIC_DOWNCAST(CMPCThemeEdit, buddy))) {
            buddyEdit->setBuddy(this); //we need to know about the buddy spin ctrl to clip it in ncpaint :-/
            hasBuddy = true;
        }

        CPaintDC dc(this);
        CRect   rectItem;
        GetClientRect(rectItem);

        COLORREF oldBkColor = dc.GetBkColor();
        COLORREF oldTextColor = dc.GetTextColor();

        COLORREF bgClr = CMPCTheme::ContentBGColor;


        CBrush borderBrush(CMPCTheme::EditBorderColor);
        CBrush butBorderBrush(CMPCTheme::ButtonBorderInnerColor);

        CBitmap arrowBMP;
        CDC dcArrowBMP;
        dcArrowBMP.CreateCompatibleDC(&dc);
        const BYTE* bits;
        int arrowLeft=0, arrowTop=0, arrowWidth, arrowHeight;

        dc.FillSolidRect(rectItem, bgClr);

        bool horz = 0 != (GetStyle() & UDS_HORZ);
        if (horz) {
            arrowWidth = CMPCTheme::SpinnerArrowHeight;
            arrowHeight = CMPCTheme::SpinnerArrowWidth;
            arrowTop = rectItem.top + (rectItem.Height() - arrowHeight) / 2;
            bits = CMPCTheme::SpinnerArrowBitsH;
            if (hasBuddy) {
                dc.ExcludeClipRect(1, 0, rectItem.Width() - 1, 1); //don't get top edge of rect
                dc.FrameRect(rectItem, &borderBrush);
            }
        } else {
            arrowWidth = CMPCTheme::SpinnerArrowWidth;
            arrowHeight = CMPCTheme::SpinnerArrowHeight;
            arrowLeft = rectItem.left + (rectItem.Width() - arrowWidth) / 2;
            bits = CMPCTheme::SpinnerArrowBitsV;
            if (hasBuddy) {
                dc.ExcludeClipRect(0, 1, 1, rectItem.Height() - 1); //don't get left edge of rect
                dc.FrameRect(rectItem, &borderBrush);
            }
        }
        arrowBMP.CreateBitmap(arrowWidth, arrowHeight, 1, 1, bits);
        dcArrowBMP.SelectObject(&arrowBMP);

        int buddySpacing = hasBuddy ? 1 : 0;
        for (int firstOrSecond = 0; firstOrSecond < 2; firstOrSecond++) {
            CRect butRect = rectItem;
            if (horz) {
                butRect.DeflateRect(1, 1, 1, 1 + buddySpacing);
                if (0 == firstOrSecond) {//left or top
                    butRect.right -= butRect.Width() / 2;
                } else {
                    butRect.left += butRect.Width() / 2;
                }
                butRect.DeflateRect(1, 0);
            } else {
                butRect.DeflateRect(1, 1, 1 + buddySpacing, 1);
                if (0 == firstOrSecond) {//left or top
                    butRect.bottom -= butRect.Height() / 2;
                } else {
                    butRect.top += butRect.Height() / 2;
                }
                butRect.DeflateRect(0, 1);
            }


            if (butRect.PtInRect(downPos)) {
                bgClr = CMPCTheme::ButtonFillSelectedColor;
            } else {
                bgClr = CMPCTheme::ButtonFillColor;
            }

            if (horz) {
                arrowLeft = butRect.left + (butRect.Width() - arrowWidth) / 2;
            } else {
                arrowTop = butRect.top + (butRect.Height() - arrowHeight) / 2;
            }

            dc.FillSolidRect(butRect, bgClr);
            dc.FrameRect(butRect, &butBorderBrush);
            dc.SetBkColor(CMPCTheme::TextFGColor);
            dc.SetTextColor(bgClr);

            if (0 == firstOrSecond) { //left or top
                dc.BitBlt(arrowLeft, arrowTop, arrowWidth, arrowHeight, &dcArrowBMP, 0, 0, SRCCOPY);
            } else {
                if (horz) {
                    dc.StretchBlt(arrowLeft, arrowTop, arrowWidth, arrowHeight, &dcArrowBMP, arrowWidth - 1, 0, -arrowWidth, arrowHeight, SRCCOPY);
                } else {
                    dc.StretchBlt(arrowLeft, arrowTop, arrowWidth, arrowHeight, &dcArrowBMP, 0, arrowHeight - 1, arrowWidth, -arrowHeight, SRCCOPY);
                }
            }
        }

        dc.SetBkColor(oldBkColor);
        dc.SetTextColor(oldTextColor);
    } else {
        __super::OnPaint();
    }

}


void CMPCThemeSpinButtonCtrl::OnMouseMove(UINT nFlags, CPoint point) {
    CSpinButtonCtrl::OnMouseMove(nFlags, point);
    if (MK_LBUTTON & nFlags) {
        downPos = point;
    } else {
        downPos = CPoint(-1, -1);
    }
}


void CMPCThemeSpinButtonCtrl::OnLButtonDown(UINT nFlags, CPoint point) {
    CSpinButtonCtrl::OnLButtonDown(nFlags, point);
    downPos = point;
}


void CMPCThemeSpinButtonCtrl::OnLButtonUp(UINT nFlags, CPoint point) {
    CSpinButtonCtrl::OnLButtonUp(nFlags, point);
    downPos = CPoint(-1, -1);
}


BOOL CMPCThemeSpinButtonCtrl::OnEraseBkgnd(CDC* pDC) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        return TRUE;
    } else {
        return __super::OnEraseBkgnd(pDC);
    }
}
