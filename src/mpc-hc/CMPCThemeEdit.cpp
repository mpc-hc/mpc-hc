#include "stdafx.h"
#include "CMPCThemeEdit.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"

CMPCThemeEdit::CMPCThemeEdit()
{
    buddy = nullptr;
    themedSBHelper = nullptr;
    isFileDialogChild = false;
    //horizontal scrollbar broken for CEdit, we must theme ourselves
    //    if (!CMPCThemeUtil::canUseWin10DarkTheme()()) {
    themedSBHelper = DEBUG_NEW CMPCThemeScrollBarHelper(this);
    //    }
}


CMPCThemeEdit::~CMPCThemeEdit()
{
    if (nullptr != themedSBHelper) {
        delete themedSBHelper;
    }
}

IMPLEMENT_DYNAMIC(CMPCThemeEdit, CEdit)
BEGIN_MESSAGE_MAP(CMPCThemeEdit, CEdit)
    ON_WM_NCPAINT()
    ON_WM_MOUSEWHEEL()
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()


void CMPCThemeEdit::PreSubclassWindow()
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (WS_EX_CLIENTEDGE == (GetStyle() & WS_EX_CLIENTEDGE)) {
            ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
        }
        CRect r;
        GetClientRect(r);
        r.DeflateRect(2, 2); //some default padding for those spaceless fonts
        SetRect(r);
        if (CMPCThemeUtil::canUseWin10DarkTheme()) {
            SetWindowTheme(GetSafeHwnd(), L"DarkMode_Explorer", NULL);
        } else {
            SetWindowTheme(GetSafeHwnd(), L"", NULL);
        }
    } else {
        __super::PreSubclassWindow();
    }
}

void CMPCThemeEdit::OnNcPaint()
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (0 != (GetStyle() & (WS_VSCROLL | WS_HSCROLL))) {  //scrollable edit will be treated like a window, not a field
            if (nullptr != themedSBHelper) {
                themedSBHelper->themedNcPaintWithSB();
            } else {
                CMPCThemeScrollBarHelper::themedNcPaint(this, this);
            }
        } else {
            CWindowDC dc(this);

            CRect rect;
            GetWindowRect(&rect);
            rect.OffsetRect(-rect.left, -rect.top);

            CBrush brush;
            if (isFileDialogChild) {//special case for edits injected to file dialog
                brush.CreateSolidBrush(CMPCTheme::W10DarkThemeFileDialogInjectedEditBorderColor);
            } else {
                brush.CreateSolidBrush(CMPCTheme::EditBorderColor);
            }

            dc.FrameRect(&rect, &brush);

            //added code to draw the inner rect for the border.  we shrunk the draw rect for border spacing earlier
            //normally, the bg of the dialog is sufficient, but in the case of ResizableDialog, it clips the anchored
            //windows, which leaves unpainted area just inside our border
            rect.DeflateRect(1, 1);
            CMPCThemeUtil::drawParentDialogBGClr(this, &dc, rect, false);

            if (nullptr != buddy) {
                buddy->Invalidate();
            }
        }

    } else {
        __super::OnNcPaint();
    }
}

void CMPCThemeEdit::SetFixedWidthFont(CFont& f)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CWindowDC dc(this);
        if (CMPCThemeUtil::getFixedFont(font, &dc)) {
            SetFont(&font);
        } else {
            SetFont(&f);
        }
    } else {
        SetFont(&f);
    }
}

BOOL CMPCThemeEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    __super::OnMouseWheel(nFlags, zDelta, pt);
    if (nullptr != themedSBHelper) {
        themedSBHelper->updateScrollInfo();
    }
    ScreenToClient(&pt);
    return TRUE;
}

void CMPCThemeEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    __super::OnVScroll(nSBCode, nPos, pScrollBar);
    if (nullptr != themedSBHelper) {
        themedSBHelper->updateScrollInfo();
    }
}

void CMPCThemeEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    __super::OnHScroll(nSBCode, nPos, pScrollBar);
    if (nullptr != themedSBHelper) {
        themedSBHelper->updateScrollInfo();
    }
}


void CMPCThemeEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    __super::OnKeyDown(nChar, nRepCnt, nFlags);
    if (nullptr != themedSBHelper) {
        themedSBHelper->updateScrollInfo();
    }
}
