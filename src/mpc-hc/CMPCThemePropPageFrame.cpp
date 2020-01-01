#include "stdafx.h"
#include "CMPCThemePropPageFrame.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"
#include "TreePropSheet/PropPageFrameDefault.h"
#include "../DSUtil/WinAPIUtils.h"

CBrush CMPCThemePropPageFrame::mpcThemeBorderBrush = CBrush();

CMPCThemePropPageFrame::CMPCThemePropPageFrame() : CPropPageFrameDefault() {
    if (nullptr == mpcThemeBorderBrush.m_hObject) {
        mpcThemeBorderBrush.CreateSolidBrush(CMPCTheme::WindowBorderColorLight);
    }
}


CMPCThemePropPageFrame::~CMPCThemePropPageFrame() {
}

BEGIN_MESSAGE_MAP(CMPCThemePropPageFrame, CPropPageFrameDefault)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


BOOL CMPCThemePropPageFrame::Create(DWORD dwWindowStyle, const RECT & rect, CWnd * pwndParent, UINT nID) {
    return CWnd::Create(
        AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0),
        _T("MPCTheme Page Frame"),
        dwWindowStyle, rect, pwndParent, nID);
}

CWnd * CMPCThemePropPageFrame::GetWnd() {
    return static_cast<CWnd*>(this);
}

void CMPCThemePropPageFrame::DrawCaption(CDC *pDC, CRect rect, LPCTSTR lpszCaption, HICON hIcon) {
    COLORREF    clrLeft = CMPCTheme::ContentSelectedColor;
    COLORREF    clrRight = CMPCTheme::ContentBGColor;
    FillGradientRectH(pDC, rect, clrLeft, clrRight);

    rect.left += 2;

    COLORREF clrPrev = pDC->SetTextColor(CMPCTheme::TextFGColor);
    int nBkStyle = pDC->SetBkMode(TRANSPARENT);

    LOGFONT lf;
    GetMessageFont(&lf);
    lf.lfHeight = -.8f * rect.Height();
    lf.lfWeight = FW_BOLD;
    CFont f;
    f.CreateFontIndirect(&lf);
    CFont * oldFont = pDC->SelectObject(&f);

    pDC->DrawText(lpszCaption, rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    pDC->SetTextColor(clrPrev);
    pDC->SelectObject(oldFont);
    pDC->SetBkMode(nBkStyle);
}

void CMPCThemePropPageFrame::OnPaint() {
    CPaintDC dc(this);
    Draw(&dc);
}

BOOL CMPCThemePropPageFrame::OnEraseBkgnd(CDC * pDC) {
    bool ret = CMPCThemeUtil::MPCThemeEraseBkgnd(pDC, this, CTLCOLOR_DLG);
    if (ret) {
        CRect rect;
        GetClientRect(rect);
        pDC->FrameRect(rect, &mpcThemeBorderBrush);
        return ret;
    } else {
        return __super::OnEraseBkgnd(pDC);
    }
}
