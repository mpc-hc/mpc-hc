//deprecated in favor of setWindowCompositionAttribute--undocumented Windows 10 API

#include "stdafx.h"
#include "VersionHelpersInternal.h"
#include "CMPCThemeFrameWnd.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"
#include "mplayerc.h"
#include "SVGImage.h"
#include <gdiplusgraphics.h>
#include <../src/mfc/oleimpl2.h>

IMPLEMENT_DYNAMIC(CMPCThemeFrameWnd, CFrameWnd)

BEGIN_MESSAGE_MAP(CMPCThemeFrameWnd, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_ACTIVATE()
    ON_WM_PAINT()
    ON_WM_NCCALCSIZE()
    ON_WM_SHOWWINDOW()
    ON_WM_NCHITTEST()
    ON_WM_NCMOUSELEAVE()
END_MESSAGE_MAP()

CMPCThemeFrameWnd::CMPCThemeFrameWnd():
    CMPCThemeFrameUtil(this),
    minimizeButton(SC_MINIMIZE),
    maximizeButton(SC_MAXIMIZE),
    closeButton(SC_CLOSE),
    currentFrameState(frameNormal),
    titleBarInfo(
{
    0
}),
drawCustomFrame(false),
titlebarHeight(30) //sane default, should be updated as soon as created
{
}

CMPCThemeFrameWnd::~CMPCThemeFrameWnd()
{
    if (closeButton.m_hWnd) {
        closeButton.DestroyWindow();
    }
    if (minimizeButton.m_hWnd) {
        minimizeButton.DestroyWindow();
    }
    if (maximizeButton.m_hWnd) {
        maximizeButton.DestroyWindow();
    }
}

LRESULT CMPCThemeFrameWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (drawCustomFrame) {
        if (uMsg == WM_WINDOWPOSCHANGING &&
                (WS_THICKFRAME) == (GetStyle() & (WS_THICKFRAME | WS_CAPTION)) &&
                currentFrameState != frameThemedTopBorder) {
            WINDOWPOS* wp = (WINDOWPOS*)lParam;
            if (nullptr != wp) {
                wp->flags |= SWP_NOREDRAW; //prevents corruption of the border when disabling caption
            }
        }
    }
    return __super::WindowProc(uMsg, wParam, lParam);
}

void CMPCThemeFrameWnd::RecalcLayout(BOOL bNotify)
{
    if (drawCustomFrame) {
        recalcTitleBar();
        int clientTop = 0;
        if (currentFrameState == frameThemedCaption) {
            CRect titleBarRect = getTitleBarRect();
            clientTop = titleBarRect.bottom;
            if (GetMenuBarState() == AFX_MBS_VISIBLE) {
                clientTop += GetSystemMetrics(SM_CYMENU);
            }

            CRect sysMenuIconRect = getSysMenuIconRect();
            CRect closeRect, maximizeRect, minimizeRect;
            GetIconRects(titleBarRect, closeRect, maximizeRect, minimizeRect);

            if (IsWindow(closeButton.m_hWnd)) {
                closeButton.MoveWindow(closeRect, TRUE);
                closeButton.ShowWindow(SW_SHOW);
            }
            if (IsWindow(minimizeButton.m_hWnd)) {
                minimizeButton.MoveWindow(minimizeRect, TRUE);
                minimizeButton.ShowWindow(SW_SHOW);
            }
            if (IsWindow(maximizeButton.m_hWnd)) {
                maximizeButton.MoveWindow(maximizeRect, TRUE);
                maximizeButton.ShowWindow(SW_SHOW);
            }
        } else {
            m_rectBorder.top = borders.top;
            if (IsWindow(closeButton.m_hWnd)) {
                closeButton.ShowWindow(SW_HIDE);
            }
            if (IsWindow(minimizeButton.m_hWnd)) {
                minimizeButton.ShowWindow(SW_HIDE);
            }
            if (IsWindow(maximizeButton.m_hWnd)) {
                maximizeButton.ShowWindow(SW_HIDE);
            }
        }

        //begin standard CFrameWnd::RecalcLayout code
        if (m_bInRecalcLayout) {
            return;
        }

        m_bInRecalcLayout = TRUE;
        // clear idle flags for recalc layout if called elsewhere
        if (m_nIdleFlags & idleNotify) {
            bNotify = TRUE;
        }
        m_nIdleFlags &= ~(idleLayout | idleNotify);

        // call the layout hook -- OLE support uses this hook
        if (bNotify && m_pNotifyHook != NULL) {
            m_pNotifyHook->OnRecalcLayout();
        }

        // reposition all the child windows (regardless of ID)
        if (GetStyle() & FWS_SNAPTOBARS) {
            CRect rect(0, 0, 32767, 32767);
            RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery,
                           &rect, &rect, FALSE);
            RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra,
                           &m_rectBorder, &rect, TRUE);
            CalcWindowRect(&rect);
            SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
        } else {
            //begin mpc-hc code to to position inside virtual client rect
            CRect cr;
            GetClientRect(cr);
            cr.top = clientTop;
            //end mpc-hc code to to position inside virtual client rect
            RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra, &m_rectBorder, &cr /* mpc-hc, pass virtual client rect */);
        }
        m_bInRecalcLayout = FALSE;
        //end CFrameWnd::RecalcLayout code


        Invalidate(TRUE);
        RedrawWindow();
    } else {
        __super::RecalcLayout(bNotify);
    }
}

void CMPCThemeFrameWnd::SetMenuBarVisibility(DWORD dwStyle)
{
    __super::SetMenuBarVisibility(dwStyle);
    if (currentFrameState == frameThemedCaption && 0 != (dwStyle & AFX_MBS_VISIBLE)) {
        Invalidate();
        DrawMenuBar();
    }
}

BOOL CMPCThemeFrameWnd::SetMenuBarState(DWORD dwState)
{
    BOOL ret = __super::SetMenuBarState(dwState);
    if (ret && currentFrameState == frameThemedCaption) {
        RecalcLayout();
        DrawMenuBar();
    }
    return ret;
}

CRect CMPCThemeFrameWnd::getTitleBarRect()
{
    CRect cr;
    GetClientRect(cr);
    CRect wr;
    GetClientRect(wr);

    if (IsZoomed()) {
        cr.top += borders.top + 1; //invisible area when maximized
        cr.bottom = cr.top + titlebarHeight;
    } else {
        cr.top += 1; //border
        cr.bottom = cr.top + titlebarHeight;
    }
    return cr;
}

CRect CMPCThemeFrameWnd::getSysMenuIconRect()
{
    CRect sysMenuIconRect, cr;
    cr = getTitleBarRect();

    DpiHelper dpiWindow;
    dpiWindow.Override(AfxGetMainWnd()->GetSafeHwnd());
    int iconsize = MulDiv(dpiWindow.DPIX(), 1, 6);
    sysMenuIconRect.top = cr.top + (cr.Height() - iconsize) / 2 + 1;
    sysMenuIconRect.bottom = sysMenuIconRect.top + iconsize;
    sysMenuIconRect.left = (dpiWindow.ScaleX(30) - iconsize) / 2 + 1;
    sysMenuIconRect.right = sysMenuIconRect.left + iconsize;
    return sysMenuIconRect;
}

bool CMPCThemeFrameWnd::checkFrame(LONG style)
{
    frameState oldState = currentFrameState;
    currentFrameState = frameNormal;
    if (drawCustomFrame) {
        if ((WS_THICKFRAME | WS_CAPTION) == (style & (WS_THICKFRAME | WS_CAPTION))) {
            currentFrameState = frameThemedCaption;
        } else if ((WS_THICKFRAME) == (style & (WS_THICKFRAME | WS_CAPTION))) {
            currentFrameState = frameThemedTopBorder;
        }
    }
    return (oldState == currentFrameState);
}

int CMPCThemeFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (drawCustomFrame) {
        int res = CWnd::OnCreate(lpCreateStruct);

        if (res == -1) {
            return -1;
        }

        RECT r = { 0, 0, 0, 0 };

        closeButton.Create(_T("Close Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, r, this, 1001);
        closeButton.setParentFrame(this);

        minimizeButton.Create(_T("Minimize Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, r, this, 1002);
        minimizeButton.setParentFrame(this);

        maximizeButton.Create(_T("Maximize Button"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, r, this, 1003);
        maximizeButton.setParentFrame(this);

        return res;
    } else {
        return __super::OnCreate(lpCreateStruct);
    }
}

void CMPCThemeFrameWnd::GetIconRects(CRect titlebarRect, CRect& closeRect, CRect& maximizeRect, CRect& minimizeRect)
{
    DpiHelper dpi;
    dpi.Override(AfxGetMainWnd()->GetSafeHwnd());

    int closeRightX;
    if (IsZoomed()) {
        closeRightX = titlebarRect.right - 2;
    } else {
        closeRightX = titlebarRect.right;
    }

    int iconHeight;
    if (IsZoomed()) { //works at 96dpi, 120dpi, 144dpi, 168dpi, 192dpi
        iconHeight = titlebarRect.Height() - 2;
    } else {
        iconHeight = titlebarRect.Height() - 1;
    }
    int buttonWidth = CMPCThemeUtil::getConstantByDPI(this, CMPCTheme::W10TitlebarButtonWidth);
    int buttonSpacing = CMPCThemeUtil::getConstantByDPI(this, CMPCTheme::W10TitlebarButtonSpacing);
    CRect buttonDimRect(0, 0, buttonWidth, iconHeight);
    closeRect = CRect(closeRightX - buttonDimRect.Width(), titlebarRect.top, closeRightX, titlebarRect.top + buttonDimRect.Height());
    maximizeRect = CRect(closeRect.left - buttonSpacing - buttonDimRect.Width(), titlebarRect.top, closeRect.left - buttonSpacing, titlebarRect.top + buttonDimRect.Height());
    minimizeRect = CRect(maximizeRect.left - buttonSpacing - buttonDimRect.Width(), titlebarRect.top, maximizeRect.left - buttonSpacing, titlebarRect.top + buttonDimRect.Height());
}

void CMPCThemeFrameWnd::OnPaint()
{
    if (currentFrameState != frameNormal) {
        CRect closeRect, maximizeRect, minimizeRect;
        CRect titleBarRect = getTitleBarRect();
        GetIconRects(titleBarRect, closeRect, maximizeRect, minimizeRect);

        CPaintDC dc(this);

        CDC dcMem;
        CBitmap bmMem;
        CRect memRect = { 0, 0, titleBarRect.right, titleBarRect.bottom };
        CMPCThemeUtil::initMemDC(&dc, dcMem, bmMem, memRect);

        CRect topBorderRect = { titleBarRect.left, titleBarRect.top - 1, titleBarRect.right, titleBarRect.top };
        dcMem.FillSolidRect(topBorderRect, CMPCTheme::W10DarkThemeWindowBorderColor);

        COLORREF titleBarColor;
        if (IsWindowForeground()) {
            titleBarColor = CMPCTheme::W10DarkThemeTitlebarBGColor;
        } else {
            titleBarColor = CMPCTheme::W10DarkThemeTitlebarInactiveBGColor;
        }


        dcMem.FillSolidRect(titleBarRect, titleBarColor);

        if (currentFrameState == frameThemedCaption) {

            CFont f;
            CMPCThemeUtil::getFontByType(f, &dcMem, CMPCThemeUtil::CaptionFont);
            dcMem.SelectObject(f);

            CRect captionRect = titleBarRect;
            DpiHelper dpi;
            dpi.Override(AfxGetMainWnd()->GetSafeHwnd());

            CRect sysMenuIconRect = getSysMenuIconRect();
            int sysIconDim = sysMenuIconRect.Width();

            captionRect.left += sysMenuIconRect.right + dpi.ScaleX(4);
            captionRect.right = minimizeRect.left - dpi.ScaleX(4);

            CFont font;
            CMPCThemeUtil::getFontByType(font, &dcMem, CMPCThemeUtil::CaptionFont);
            dcMem.SetBkColor(titleBarColor);
            dcMem.SetTextColor(CMPCTheme::W10DarkThemeTitlebarFGColor);
            CString windowText;
            GetWindowText(windowText);
            dcMem.DrawText(windowText, captionRect, DT_LEFT | DT_WORD_ELLIPSIS | DT_VCENTER | DT_SINGLELINE);

            HICON icon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_nIDHelp), IMAGE_ICON, sysIconDim, sysIconDim, LR_SHARED);
            ::DrawIconEx(dcMem.m_hDC, sysMenuIconRect.left, sysMenuIconRect.top, icon, 0, 0, 0, nullptr, DI_NORMAL);
        }

        CMPCThemeUtil::flushMemDC(&dc, dcMem, memRect);

        if (m_pCtrlCont != NULL) {
            m_pCtrlCont->OnPaint(&dc);
        }
        Default();
    } else {
        Default();
    }

}

void CMPCThemeFrameWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
    if (AfxGetAppSettings().bMPCThemeLoaded && IsWindows10OrGreater() && !AfxGetAppSettings().bWindows10AccentColorsEnabled) {
        drawCustomFrame = true;
    } else {
        drawCustomFrame = false;
    }

    if (drawCustomFrame) {
        if (currentFrameState != frameNormal) {
            if (bCalcValidRects) {
                if (currentFrameState == frameThemedCaption) {
                    lpncsp->rgrc[0].left += borders.left + 1;
                    lpncsp->rgrc[0].right -= borders.right + 1;
                    lpncsp->rgrc[0].bottom -= borders.bottom + 1;
                } else {
                    __super::OnNcCalcSize(bCalcValidRects, lpncsp);
                    lpncsp->rgrc[0].top -= 6;
                }
            } else {
                __super::OnNcCalcSize(bCalcValidRects, lpncsp);
            }
        } else {
            __super::OnNcCalcSize(bCalcValidRects, lpncsp);
        }
        recalcFrame(); //framechanged--if necessary we recalculate everything; if done internally this should be a no-op
    } else {
        __super::OnNcCalcSize(bCalcValidRects, lpncsp);
    }
}

void CMPCThemeFrameWnd::recalcFrame()
{
    if (!checkFrame(GetStyle())) {
        borders = { 0, 0, 0, 0 };
        UINT style = GetStyle();
        if (0 != (style & WS_THICKFRAME)) {
            AdjustWindowRectEx(&borders, style & ~WS_CAPTION, FALSE, NULL);
            borders.left = abs(borders.left);
            borders.top = abs(borders.top);
        } else if (0 != (style & WS_BORDER)) {
            borders = { 1, 1, 1, 1 };
        }
        SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }
}

void CMPCThemeFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    if (drawCustomFrame) {
        if (titleBarInfo.cbSize == 0) { //only check this once, as it can be wrong later
            titleBarInfo = { sizeof(TITLEBARINFO) };
            GetTitleBarInfo(&titleBarInfo);
        }
        recalcFrame();
        CWnd::OnActivate(nState, pWndOther, bMinimized);
        Invalidate(TRUE);
    } else {
        __super::OnActivate(nState, pWndOther, bMinimized);
    }
}

LRESULT CMPCThemeFrameWnd::OnNcHitTest(CPoint point)
{
    if (currentFrameState == frameThemedCaption) {
        LRESULT result = 0;

        result = CWnd::OnNcHitTest(point);
        if (result == HTCLIENT) {
            ScreenToClient(&point);
            if (point.y < borders.top) {
                return HTTOP;
            } else if (point.y < titlebarHeight) {
                CRect sysMenuIconRect = getSysMenuIconRect();
                CRect closeRect, maximizeRect, minimizeRect;
                CRect titleBarRect = getTitleBarRect();
                GetIconRects(titleBarRect, closeRect, maximizeRect, minimizeRect);

                if (sysMenuIconRect.PtInRect(point)) {
                    return HTSYSMENU;
                } else if (closeRect.PtInRect(point) || minimizeRect.PtInRect(point) || maximizeRect.PtInRect(point)) {
                    return HTNOWHERE;
                } else {
                    return HTCAPTION;
                }
            } else if (point.y < titlebarHeight + GetSystemMetrics(SM_CYMENU)) {
                return HTMENU;
            }
        }
        return result;
    } else {
        return __super::OnNcHitTest(point);
    }
}

void CMPCThemeFrameWnd::OnNcMouseLeave()
{
    if (currentFrameState == frameThemedCaption) {
        CWnd::OnNcMouseLeave();
    } else {
        __super::OnNcMouseLeave();
    }
}

void CMPCThemeFrameWnd::recalcTitleBar()
{
    titlebarHeight = CRect(titleBarInfo.rcTitleBar).Height() + borders.top;
    if (IsZoomed()) {
        titlebarHeight -= borders.top;
    }
}

