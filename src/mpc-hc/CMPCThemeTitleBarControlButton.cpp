#include "stdafx.h"
#include "CMPCThemeTitleBarControlButton.h"
#include "CMPCThemeUtil.h"

BEGIN_MESSAGE_MAP(CMPCThemeTitleBarControlButton, CMFCButton)
    ON_WM_PAINT()
    ON_CONTROL_REFLECT(BN_CLICKED, &CMPCThemeTitleBarControlButton::OnBnClicked)
END_MESSAGE_MAP()

CMPCThemeTitleBarControlButton::CMPCThemeTitleBarControlButton(WPARAM _buttonType) : CMFCButton()
    , parent(nullptr)
{
    this->buttonType = _buttonType;
    switch (buttonType) {
        case SC_CLOSE:
            hoverColor = CMPCTheme::CloseHoverColor;
            pushedColor = CMPCTheme::ClosePushColor;
            hoverInactiveColor = CMPCTheme::CloseHoverColor;
            break;
        case SC_MINIMIZE:
        case SC_MAXIMIZE:
        default:
            hoverColor = CMPCTheme::W10DarkThemeTitlebarControlHoverBGColor;
            pushedColor = CMPCTheme::W10DarkThemeTitlebarControlPushedBGColor;
            hoverInactiveColor = CMPCTheme::W10DarkThemeTitlebarInactiveControlHoverBGColor;
            break;
    }
}


void CMPCThemeTitleBarControlButton::setParentFrame(CMPCThemeFrameUtil* _parent)
{
    this->parent = _parent;
}

WPARAM CMPCThemeTitleBarControlButton::getButtonType()
{
    if (buttonType == SC_MAXIMIZE && parent->IsWindowZoomed()) {
        return SC_RESTORE;
    }
    return buttonType;
}

void CMPCThemeTitleBarControlButton::drawTitleBarButton(CDC* pDC, CRect iconRect, std::vector<CMPCTheme::pathPoint> icon, double dpiScaling, bool antiAlias)
{
    int iconWidth = CMPCThemeUtil::getConstantByDPI(this, CMPCTheme::W10TitlebarIconPathWidth);
    int iconHeight = CMPCThemeUtil::getConstantByDPI(this, CMPCTheme::W10TitlebarIconPathHeight);
    float penThickness = CMPCThemeUtil::getConstantFByDPI(this, CMPCTheme::W10TitlebarIconPathThickness);
    CRect pathRect = {
        iconRect.left + (iconRect.Width() - iconWidth) / 2,
        iconRect.top + (iconRect.Height() - iconHeight) / 2,
        iconWidth,
        iconHeight
    };

    Gdiplus::Graphics gfx(pDC->m_hDC);
    if (antiAlias) {
        gfx.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x8);
    }
    Gdiplus::Color lineClr;
    lineClr.SetFromCOLORREF(CMPCTheme::W10DarkThemeTitlebarIconPenColor);
    Gdiplus::Pen iPen(lineClr, penThickness);
    if (penThickness >= 2) {
        iPen.SetLineCap(Gdiplus::LineCapSquare, Gdiplus::LineCapSquare, Gdiplus::DashCapFlat);
    }
    Gdiplus::REAL lastX = 0, lastY = 0;
    for (u_int i = 0; i < icon.size(); i++) {
        CMPCTheme::pathPoint p = icon[i];
        Gdiplus::REAL x = (Gdiplus::REAL)(pathRect.left + p.x);
        Gdiplus::REAL y = (Gdiplus::REAL)(pathRect.top + p.y);
        if (p.state == CMPCTheme::newPath) {
            lastX = x;
            lastY = y;
        } else if ((p.state == CMPCTheme::linePath || p.state == CMPCTheme::closePath) && i > 0) {
            gfx.DrawLine(&iPen, lastX, lastY, x, y);
            if (antiAlias && penThickness < 2) {
                gfx.DrawLine(&iPen, lastX, lastY, x, y); //draw again to brighten the AA diagonals
            }
            lastX = x;
            lastY = y;
        }
    }
}


void CMPCThemeTitleBarControlButton::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    CRect cr;
    GetClientRect(cr);
    if (IsPushed()) {
        dc.FillSolidRect(cr, pushedColor);
    } else if (IsHighlighted()) {
        if (parent->IsWindowForeground()) {
            dc.FillSolidRect(cr, hoverColor);
        } else {
            dc.FillSolidRect(cr, hoverInactiveColor);
        }
    } else {
        if (nullptr != parent) {
            COLORREF tbColor;
            if (parent->IsWindowForeground()) {
                tbColor = CMPCTheme::W10DarkThemeTitlebarBGColor;
            } else {
                tbColor = CMPCTheme::W10DarkThemeTitlebarInactiveBGColor;
            }
            dc.FillSolidRect(cr, tbColor);
        }
    }
    DpiHelper dpiWindow;
    dpiWindow.Override(AfxGetMainWnd()->GetSafeHwnd());
    drawTitleBarButton(&dc, cr, CMPCThemeUtil::getIconPathByDPI(this), dpiWindow.ScaleFactorX(), true);
}

void CMPCThemeTitleBarControlButton::OnBnClicked()
{
    switch (buttonType) {
        case SC_CLOSE:
            parent->PostWindowMessage(WM_CLOSE, 0, 0);
            break;
        case SC_MINIMIZE:
            parent->PostWindowMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
            break;
        case SC_MAXIMIZE:
            if (parent->IsWindowZoomed()) {
                parent->PostWindowMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
            } else {
                parent->PostWindowMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            }
            break;
    }
}
