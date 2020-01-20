#include "stdafx.h"
#include "CMPCThemeDockBar.h"
#include "CMPCTheme.h"
#include "mplayerc.h"

IMPLEMENT_DYNAMIC(CMPCThemeDockBar, CDockBar)
BEGIN_MESSAGE_MAP(CMPCThemeDockBar, CDockBar)
    ON_WM_ERASEBKGND()
    ON_WM_NCPAINT()
END_MESSAGE_MAP()

CMPCThemeDockBar::CMPCThemeDockBar()
{
}


CMPCThemeDockBar::~CMPCThemeDockBar()
{
}


BOOL CMPCThemeDockBar::OnEraseBkgnd(CDC* pDC)
{
    const CAppSettings& s = AfxGetAppSettings();
    if (!s.bMPCThemeLoaded) {
        return __super::OnEraseBkgnd(pDC);
    }

    CBrush backBrush(CMPCTheme::WindowBGColor);

    CBrush* pOldBrush = pDC->SelectObject(&backBrush);
    CRect rect;
    pDC->GetClipBox(&rect);

    pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOldBrush);

    return TRUE;
}


void CMPCThemeDockBar::OnNcPaint()
{
    const CAppSettings& s = AfxGetAppSettings();
    if (!s.bMPCThemeLoaded) {
        __super::OnNcPaint();
        return;
    }

    CWindowDC dc(this); // the HDC will be released by the destructor

    CRect rcClient, rcWindow;
    GetClientRect(rcClient);
    GetWindowRect(rcWindow);
    ScreenToClient(rcWindow);
    rcClient.OffsetRect(-rcWindow.TopLeft());
    rcWindow.OffsetRect(-rcWindow.TopLeft());

    CRect rcDraw = rcWindow;

    dc.IntersectClipRect(rcWindow);
    dc.ExcludeClipRect(rcClient);
    dc.FillSolidRect(rcDraw, CMPCTheme::WindowBGColor);
}
