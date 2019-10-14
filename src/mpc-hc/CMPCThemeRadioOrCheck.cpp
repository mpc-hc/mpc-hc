#include "stdafx.h"
#include "CMPCThemeRadioOrCheck.h"
#include "CMPCTheme.h"
#include "CMPCThemeButton.h"
#include "CMPCThemeUtil.h"

CMPCThemeRadioOrCheck::CMPCThemeRadioOrCheck() {
    isHover = false;
    buttonType = unknownType;
}


CMPCThemeRadioOrCheck::~CMPCThemeRadioOrCheck() {
}

void CMPCThemeRadioOrCheck::PreSubclassWindow() {
    DWORD winButtonType = (GetButtonStyle() & BS_TYPEMASK);

    if (BS_RADIOBUTTON == winButtonType || BS_AUTORADIOBUTTON == winButtonType) {
        buttonType = radioType;
        isAuto = BS_AUTORADIOBUTTON == winButtonType;
    } else if (BS_3STATE == winButtonType || BS_AUTO3STATE == winButtonType) {
        buttonType = threeStateType;
        isAuto = BS_AUTO3STATE == winButtonType;
    } else if (BS_CHECKBOX == winButtonType || BS_AUTOCHECKBOX == winButtonType) {
        buttonType = checkType;
        isAuto = BS_AUTOCHECKBOX == winButtonType;
    }
    ASSERT(buttonType != unknownType);

    buttonStyle = GetWindowLongPtr(GetSafeHwnd(), GWL_STYLE);
    if (nullptr == font.m_hObject) {
        CMPCThemeUtil::getFontByType(font, GetWindowDC(), CMPCThemeUtil::DialogFont);
    }
    SetFont(&font); //DSUtil checks metrics and resizes.  if our font is a bit different, things can look funny
    CButton::PreSubclassWindow();
}

IMPLEMENT_DYNAMIC(CMPCThemeRadioOrCheck, CButton)
BEGIN_MESSAGE_MAP(CMPCThemeRadioOrCheck, CButton)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_PAINT()
    ON_WM_ENABLE()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CMPCThemeRadioOrCheck::OnPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CPaintDC dc(this);
        CRect   rectItem;
        GetClientRect(rectItem);

        COLORREF oldBkColor = dc.GetBkColor();
        COLORREF oldTextColor = dc.GetTextColor();

        bool isDisabled = !IsWindowEnabled();
        bool isFocused = (GetFocus() == this);

        LRESULT checkState = SendMessage(BM_GETCHECK);

        CString sTitle;
        GetWindowText(sTitle);


        if (0 != (buttonStyle & BS_PUSHLIKE)) {
            CFont *oFont;
            oFont = dc.SelectObject(&font);
            CMPCThemeButton::drawButtonBase(&dc, rectItem, sTitle, checkState != BST_UNCHECKED, isHover, isFocused, checkState == BST_INDETERMINATE, false);
            dc.SelectObject(oFont);
        } else {
            CRect rectCheck;
            int cbWidth = GetSystemMetrics(SM_CXMENUCHECK);
            int cbHeight = GetSystemMetrics(SM_CYMENUCHECK);

            if (buttonStyle & BS_LEFTTEXT) {
                rectCheck.left = rectItem.right - cbWidth;
                rectCheck.right = rectCheck.left + cbWidth;
                rectItem.right = rectCheck.left - 2;
            } else {
                rectCheck.left = rectItem.left;
                rectCheck.right = rectCheck.left + cbWidth;
                rectItem.left = rectCheck.right + 2;
            }

            rectCheck.top = (rectItem.Height() - cbHeight) / 2;
            rectCheck.bottom = rectCheck.top + cbHeight;

            if (buttonType == checkType) {
                CMPCThemeUtil::drawCheckBox(checkState, isHover, true, rectCheck, &dc);
            } else if (buttonType == threeStateType) {
                CMPCThemeUtil::drawCheckBox(checkState, isHover, true, rectCheck, &dc);
            } else if (buttonType == radioType) {
                CMPCThemeUtil::drawCheckBox(checkState, isHover, true, rectCheck, &dc, true);
            }

            if (!sTitle.IsEmpty()) {
                CRect centerRect = rectItem;
                CFont* pOldFont = dc.SelectObject(&font);

                UINT uFormat = 0;
                if (buttonStyle & BS_MULTILINE) {
                    uFormat |= DT_WORDBREAK;
                } else {
                    uFormat |= DT_SINGLELINE;
                }

                if (buttonStyle & BS_VCENTER) {
                    uFormat |= DT_VCENTER;
                }

                if ((buttonStyle & BS_CENTER) == BS_CENTER) {
                    uFormat |= DT_CENTER;
                    dc.DrawText(sTitle, -1, &rectItem, uFormat | DT_CALCRECT);
                    rectItem.OffsetRect((centerRect.Width() - rectItem.Width()) / 2,
                        (centerRect.Height() - rectItem.Height()) / 2);
                } else if ((buttonStyle & BS_RIGHT) == BS_RIGHT) {
                    uFormat |= DT_RIGHT;
                    dc.DrawText(sTitle, -1, &rectItem, uFormat | DT_CALCRECT);
                    rectItem.OffsetRect(centerRect.Width() - rectItem.Width(),
                        (centerRect.Height() - rectItem.Height()) / 2);
                } else { // if ((buttonStyle & BS_LEFT) == BS_LEFT) {
                    uFormat |= DT_LEFT;
                    dc.DrawText(sTitle, -1, &rectItem, uFormat | DT_CALCRECT);
                    rectItem.OffsetRect(0, (centerRect.Height() - rectItem.Height()) / 2);
                }

                dc.SetBkColor(CMPCTheme::WindowBGColor);
                if (isDisabled) {
                    dc.SetTextColor(CMPCTheme::ButtonDisabledFGColor);
                    dc.DrawText(sTitle, -1, &rectItem, uFormat);
                } else {
                    dc.SetTextColor(CMPCTheme::TextFGColor);
                    dc.DrawText(sTitle, -1, &rectItem, uFormat);
                }
                dc.SelectObject(pOldFont);

                if (isFocused) {
                    CRect focusRect = rectItem;
                    focusRect.InflateRect(0, 0);
                    dc.SetTextColor(CMPCTheme::ButtonBorderKBFocusColor); //no example of this in explorer, but white seems too harsh
                    CBrush *dotted = dc.GetHalftoneBrush();
                    dc.FrameRect(focusRect, dotted);
                    DeleteObject(dotted);
                }

            }
        }

        dc.SetBkColor(oldBkColor);
        dc.SetTextColor(oldTextColor);
    } else {
        CButton::OnPaint();
    }
}

void CMPCThemeRadioOrCheck::OnSetFocus(CWnd* pOldWnd) {
    CButton::OnSetFocus(pOldWnd);
    Invalidate();
}

void CMPCThemeRadioOrCheck::checkHover(UINT nFlags, CPoint point, bool invalidate) {
    CRect r;
    GetClientRect(r);
    bool oldHover = isHover;
    CPoint ptScreen = point;
    ClientToScreen(&ptScreen);

    if (r.PtInRect(point) && WindowFromPoint(ptScreen)->GetSafeHwnd() == GetSafeHwnd()) {
        isHover = true;
    } else {
        isHover = false;
    }
    if (isHover != oldHover && invalidate) {
        Invalidate();
    }

}

void CMPCThemeRadioOrCheck::OnMouseMove(UINT nFlags, CPoint point) {
    checkHover(nFlags, point);
    CButton::OnMouseMove(nFlags, point);
}


void CMPCThemeRadioOrCheck::OnMouseLeave() {
    checkHover(0, CPoint(-1, -1));
    CButton::OnMouseLeave();
}


void CMPCThemeRadioOrCheck::OnLButtonUp(UINT nFlags, CPoint point) {
    checkHover(nFlags, point, false);
    CButton::OnLButtonUp(nFlags, point);
}


void CMPCThemeRadioOrCheck::OnLButtonDown(UINT nFlags, CPoint point) {
    checkHover(nFlags, point);
    CButton::OnLButtonDown(nFlags, point);
}



void CMPCThemeRadioOrCheck::OnEnable(BOOL bEnable) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        SetRedraw(FALSE);
        __super::OnEnable(bEnable);
        SetRedraw(TRUE);
        Invalidate(); //WM_PAINT not handled when enabling/disabling
    } else {
        __super::OnEnable(bEnable);
    }
}


BOOL CMPCThemeRadioOrCheck::OnEraseBkgnd(CDC* pDC) {
    CRect r;
    GetClientRect(r);
    pDC->FillSolidRect(r, CMPCTheme::CMPCTheme::WindowBGColor);
    return TRUE;
}
