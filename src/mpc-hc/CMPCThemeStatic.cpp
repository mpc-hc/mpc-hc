#include "stdafx.h"
#include "CMPCThemeStatic.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"

CMPCThemeStatic::CMPCThemeStatic() {
    isFileDialogChild = false;
}


CMPCThemeStatic::~CMPCThemeStatic() {
}
IMPLEMENT_DYNAMIC(CMPCThemeStatic, CStatic)
BEGIN_MESSAGE_MAP(CMPCThemeStatic, CStatic)
    ON_WM_PAINT()
    ON_WM_NCPAINT()
    ON_WM_ENABLE()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CMPCThemeStatic::OnPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CPaintDC dc(this);

        CString sTitle;
        GetWindowText(sTitle);
        CRect rectItem;
        GetClientRect(rectItem);
        dc.SetBkMode(TRANSPARENT);

        COLORREF oldBkColor = dc.GetBkColor();
        COLORREF oldTextColor = dc.GetTextColor();

        bool isDisabled = !IsWindowEnabled();
        UINT style = GetStyle();

        if (!sTitle.IsEmpty()) {
            CFont *font= GetFont();
            CFont* pOldFont = dc.SelectObject(font);

            UINT uFormat = 0;
            if (style & SS_LEFTNOWORDWRAP) {
                uFormat |= DT_SINGLELINE;
            } else {
                uFormat |= DT_WORDBREAK;
            }

            if (0 != (style & SS_CENTERIMAGE) && sTitle.Find(_T("\n")) == -1) {
                //If the static control contains a single line of text, the text is centered vertically in the client area of the control. msdn
                uFormat |= DT_SINGLELINE;
                uFormat |= DT_VCENTER; 
            } else {
                uFormat |= DT_TOP;
            }

            if ((style & SS_CENTER) == SS_CENTER) {
                uFormat |= DT_CENTER;
            } else if ((style & SS_RIGHT) == SS_RIGHT) {
                uFormat |= DT_RIGHT;
            } else { // if ((style & SS_LEFT) == SS_LEFT || (style & SS_LEFTNOWORDWRAP) == SS_LEFTNOWORDWRAP) {
                uFormat |= DT_LEFT;
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
            dc.SetBkColor(oldBkColor);
            dc.SetTextColor(oldTextColor);
        }
    } else {
        __super::OnPaint();
    }
}


void CMPCThemeStatic::OnNcPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CDC* pDC = GetWindowDC();

        CRect rect;
        GetWindowRect(&rect);
        rect.OffsetRect(-rect.left, -rect.top);
        DWORD type = GetStyle() & SS_TYPEMASK;


        if (SS_ETCHEDHORZ == type || SS_ETCHEDVERT == type) { //etched lines assumed
            rect.DeflateRect(0, 0, 1, 1); //make it thinner
            CBrush brush(CMPCTheme::StaticEtchedColor);
            pDC->FillSolidRect(rect, CMPCTheme::StaticEtchedColor);
        } else if (SS_ETCHEDFRAME == type) { //etched border
            CBrush brush(CMPCTheme::StaticEtchedColor);
            pDC->FrameRect(rect, &brush);
        } else { //not supported yet
        }
    } else {
        CStatic::OnNcPaint();
    }
}

void CMPCThemeStatic::OnEnable(BOOL bEnable) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        SetRedraw(FALSE);
        __super::OnEnable(bEnable);
        SetRedraw(TRUE);
        CWnd *parent = GetParent();
        if (nullptr != parent) {
            CRect wr;
            GetWindowRect(wr);
            parent->ScreenToClient(wr);
            parent->InvalidateRect(wr, TRUE);
        } else {
            Invalidate();
        }
    } else {
        __super::OnEnable(bEnable);
    }
}

BOOL CMPCThemeStatic::OnEraseBkgnd(CDC* pDC) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CRect r;
        GetClientRect(r);
        if (isFileDialogChild) {
            HBRUSH hBrush=CMPCThemeUtil::getCtlColorFileDialog(pDC->GetSafeHdc(), CTLCOLOR_STATIC);
            ::FillRect(pDC->GetSafeHdc(), r, hBrush);
        } else {
            CMPCThemeUtil::drawParentDialogBGClr(this, pDC, r);
        }
        return TRUE;
    } else {
        return CStatic::OnEraseBkgnd(pDC);
    }
}
