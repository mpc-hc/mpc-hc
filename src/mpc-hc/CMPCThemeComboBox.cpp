#include "stdafx.h"
#include "CMPCThemeComboBox.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"
#include "mplayerc.h"

IMPLEMENT_DYNAMIC(CMPCThemeComboBox, CComboBox)

BEGIN_MESSAGE_MAP(CMPCThemeComboBox, CComboBox)
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
END_MESSAGE_MAP()

CMPCThemeComboBox::CMPCThemeComboBox() {
    isHover = false;
    isThemedDropDown = false;
}

void CMPCThemeComboBox::doDraw(CDC& dc, CString strText, CRect rText, COLORREF bkColor, COLORREF fgColor, bool drawDotted) {
    COLORREF crOldTextColor = dc.GetTextColor();
    COLORREF crOldBkColor = dc.GetBkColor();

    dc.SetBkColor(bkColor);
    dc.SetTextColor(fgColor);

    CRect textRect = rText;
    //textRect.left += 3;

    CFont *font = GetFont();
    CFont* pOldFont = dc.SelectObject(font);
    dc.DrawText(strText, &textRect, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    dc.SelectObject(pOldFont);

    if (drawDotted) {
        dc.SetTextColor(bkColor ^ 0xffffff);
        CBrush *dotted = dc.GetHalftoneBrush();
        dc.FrameRect(rText, dotted);
        DeleteObject(dotted);
    }

    dc.SetTextColor(crOldTextColor);
    dc.SetBkColor(crOldBkColor);
}

CMPCThemeComboBox::~CMPCThemeComboBox() {
}

void CMPCThemeComboBox::themeDropDown() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (CMPCThemeUtil::canUseWin10DarkTheme() && false == isThemedDropDown) {
            COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
            if (GetComboBoxInfo(&info)) {
                SetWindowTheme(info.hwndList, L"DarkMode_Explorer", NULL);
                isThemedDropDown = true;
            }
        }
    }
}

void CMPCThemeComboBox::PreSubclassWindow() {
    themeDropDown();
}


void CMPCThemeComboBox::drawComboArrow(CDC &dc, COLORREF arrowClr, CRect arrowRect) {
    DpiHelper dpiWindow;
    dpiWindow.Override(GetSafeHwnd());

    Gdiplus::Color clr;
    clr.SetFromCOLORREF(arrowClr);

    int dpi = dpiWindow.DPIX();
    float steps;

    if (dpi < 120) steps = 3.5;
    else if (dpi < 144) steps = 4;
    else if (dpi < 168) steps = 5;
    else if (dpi < 192) steps = 5;
    else steps = 6;

    int xPos = arrowRect.left + (arrowRect.Width() - (steps * 2 + 1)) / 2;
    int yPos = arrowRect.top + (arrowRect.Height() - (steps + 1)) / 2;

    Gdiplus::Graphics gfx(dc.m_hDC);
    gfx.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x4);
    Gdiplus::Pen pen(clr, 1);
    for (int i = 0; i < 2; i++) {
        Gdiplus::GraphicsPath path;
        Gdiplus::PointF vertices[3];

        vertices[0] = Gdiplus::PointF(xPos, yPos);
        vertices[1] = Gdiplus::PointF(steps + xPos, yPos + steps);
        vertices[2] = Gdiplus::PointF(steps * 2 + xPos, yPos);

        path.AddLines(vertices, 3);
        gfx.DrawPath(&pen, &path);
    }
}


void CMPCThemeComboBox::OnPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CPaintDC dc(this);
        CRect r, rBorder, rText, rBG, rSelect, rDownArrow;
        GetClientRect(r);
        CString strText;
        GetWindowText(strText);

        CBrush fb;
        bool isFocused = (GetFocus() == this);
        bool drawDotted = false;
        if (isFocused) {
            //unless edit exists, no inner border -- fixme if edit combobox needed
            //fb.CreateSolidBrush(CMPCTheme::ButtonBorderInnerFocusedColor);
            fb.CreateSolidBrush(CMPCTheme::ButtonBorderInnerColor);
            drawDotted = true;
        } else {
            fb.CreateSolidBrush(CMPCTheme::ButtonBorderInnerColor);
        }
        COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
        GetComboBoxInfo(&info);

        COLORREF bkColor, fgColor = CMPCTheme::TextFGColor, arrowColor = CMPCTheme::ComboboxArrowColor;
        if ((nullptr != info.hwndList && ::IsWindowVisible(info.hwndList)) || info.stateButton == STATE_SYSTEM_PRESSED) { //always looks the same once the list is open
            bkColor = CMPCTheme::ButtonFillSelectedColor;
            drawDotted = false;
        } else if (info.stateButton == 0 && isHover) {  //not pressed and hovered
            bkColor = CMPCTheme::ButtonFillHoverColor;
        } else if (!IsWindowEnabled()) {
            bkColor = CMPCTheme::ButtonFillColor;
            fgColor = CMPCTheme::ButtonDisabledFGColor;
            arrowColor = CMPCTheme::ComboboxArrowColorDisabled;
        } else {
            bkColor = CMPCTheme::ButtonFillColor;
        }

        rBG = r;
        rBG.DeflateRect(1, 1);
        dc.FillSolidRect(rBG, bkColor);

        rText = r;
        rText.right = info.rcItem.right;
        rText.DeflateRect(3, 3);
        doDraw(dc, strText, rText, bkColor, fgColor, drawDotted);

        rDownArrow = info.rcButton;
        drawComboArrow(dc, arrowColor, rDownArrow);

        rBorder = r;
        dc.FrameRect(rBorder, &fb);
    } else {
        CComboBox::OnPaint();
    }
}


void CMPCThemeComboBox::OnSetFocus(CWnd* pOldWnd) {
    CComboBox::OnSetFocus(pOldWnd);
    //Invalidate();
}

void CMPCThemeComboBox::checkHover(UINT nFlags, CPoint point, bool invalidate) {
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

void CMPCThemeComboBox::OnMouseMove(UINT nFlags, CPoint point) {
    checkHover(nFlags, point);
    CComboBox::OnMouseMove(nFlags, point);
}


void CMPCThemeComboBox::OnMouseLeave() {
    checkHover(0, CPoint(-1,-1));
    CComboBox::OnMouseLeave();
}


void CMPCThemeComboBox::OnLButtonUp(UINT nFlags, CPoint point) {
    checkHover(nFlags, point, false);
    CComboBox::OnLButtonUp(nFlags, point);
}


void CMPCThemeComboBox::OnLButtonDown(UINT nFlags, CPoint point) {
    checkHover(nFlags, point);
    CComboBox::OnLButtonDown(nFlags, point);
}


int CMPCThemeComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;

    themeDropDown();

    return 0;
}
