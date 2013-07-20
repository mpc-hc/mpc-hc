/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "mplayerc.h"
#include "ChildView.h"
#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView(CMainFrame* pMainFrm)
    : m_vrect(0, 0, 0, 0)
    , CMouseWnd(pMainFrm)
    , m_pMainFrm(pMainFrm)
{
    LoadLogo();
}

CChildView::~CChildView()
{
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs)) {
        return FALSE;
    }

    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       ::LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr);

    return TRUE;
}

BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
    // filter interactive video controls mouse messages
    if (pMsg->hwnd != m_hWnd &&
            pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST &&
            m_pMainFrm->IsInteractiveVideo()) {
        switch (pMsg->message) {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                // let them through, interactive video controls will handle those
                break;
            case WM_MOUSEMOVE: {
                // duplicate those
                CPoint point(pMsg->lParam);
                ::MapWindowPoints(pMsg->hwnd, m_hWnd, &point, 1);
                VERIFY(PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(point.x, point.y)));
                break;
            }
            default: {
                // and handle others in this class
                CPoint point(pMsg->lParam);
                ::MapWindowPoints(pMsg->hwnd, m_hWnd, &point, 1);
                pMsg->lParam = MAKELPARAM(point.x, point.y);
                pMsg->hwnd = m_hWnd;
            }
        }
    }
    return CWnd::PreTranslateMessage(pMsg);
}

LRESULT CChildView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;
    bool bCallOurProc = true;
    if (m_pMainFrm->m_pMVRSR) {
        // call madVR window proc directly when the interface is available
        switch (message) {
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                // CMouseWnd will call madVR window proc
                break;
            default:
                bCallOurProc = !m_pMainFrm->m_pMVRSR->ParentWindowProc(m_hWnd, message, &wParam, &lParam, &ret);
        }
    }
    if (bCallOurProc) {
        ret = __super::WindowProc(message, wParam, lParam);
    }
    return ret;
}

void CChildView::SetVideoRect(const CRect& r)
{
    m_vrect = r;

    Invalidate();
}

void CChildView::LoadLogo()
{
    CAppSettings& s = AfxGetAppSettings();
    bool bHaveLogo = false;

    m_logo.DeleteObject();

    if (s.fLogoExternal) {
        bHaveLogo = !!m_logo.LoadFromFile(s.strLogoFileName);
    }

    if (!bHaveLogo) {
        s.fLogoExternal = false;                // use the built-in logo instead
        s.strLogoFileName = "";                 // clear logo file name

        if (!m_logo.Load(s.nLogoId)) {          // try the latest selected build-in logo
            m_logo.Load(s.nLogoId = DEF_LOGO);  // if fail then use the default logo, should and must never fail
        }
    }

    if (m_hWnd) {
        Invalidate();
    }
}

CSize CChildView::GetLogoSize()
{
    BITMAP bitmap;
    ZeroMemory(&bitmap, sizeof(BITMAP));
    m_logo.GetBitmap(&bitmap);
    return CSize(bitmap.bmWidth, bitmap.bmHeight);
}

IMPLEMENT_DYNAMIC(CChildView, CMouseWnd)

BEGIN_MESSAGE_MAP(CChildView, CMouseWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_NCHITTEST()
    ON_WM_NCLBUTTONDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

void CChildView::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    ((CMainFrame*)GetParentFrame())->RepaintVideo();

    // Do not call CWnd::OnPaint() for painting messages
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
    CRect r;

    CImage img;
    img.Attach(m_logo);

    if (m_pMainFrm->GetLoadState() != MLS_CLOSED && !m_pMainFrm->IsD3DFullScreenMode()) {
        pDC->ExcludeClipRect(m_vrect);
    } else if (!img.IsNull()) {
        GetClientRect(r);
        int w = min(img.GetWidth(), r.Width());
        int h = min(img.GetHeight(), r.Height());
        int x = (r.Width() - w) / 2;
        int y = (r.Height() - h) / 2;
        r = CRect(CPoint(x, y), CSize(w, h));

        int oldmode = pDC->SetStretchBltMode(STRETCH_HALFTONE);
        img.StretchBlt(*pDC, r, CRect(0, 0, img.GetWidth(), img.GetHeight()));
        pDC->SetStretchBltMode(oldmode);
        pDC->ExcludeClipRect(r);
    }
    img.Detach();

    GetClientRect(r);
    pDC->FillSolidRect(r, 0);

    return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    ((CMainFrame*)GetParentFrame())->MoveVideoWindow();
}

LRESULT CChildView::OnNcHitTest(CPoint point)
{
    LRESULT ret = CWnd::OnNcHitTest(point);
    const auto pFrame = AfxGetMainFrame();
    if (!pFrame->m_fFullScreen && pFrame->IsFrameLessWindow()) {
        CRect rcFrame;
        GetWindowRect(&rcFrame);
        CRect rcClient(rcFrame);
        rcClient.InflateRect(-GetSystemMetrics(SM_CXSIZEFRAME), -GetSystemMetrics(SM_CYSIZEFRAME));

        if (rcFrame.PtInRect(point)) {
            if (point.x > rcClient.right) {
                if (point.y < rcClient.top) {
                    ret = HTTOPRIGHT;
                } else if (point.y > rcClient.bottom) {
                    ret = HTBOTTOMRIGHT;
                } else {
                    ret = HTRIGHT;
                }
            } else if (point.x < rcClient.left) {
                if (point.y < rcClient.top) {
                    ret = HTTOPLEFT;
                } else if (point.y > rcClient.bottom) {
                    ret = HTBOTTOMLEFT;
                } else {
                    ret = HTLEFT;
                }
            } else if (point.y < rcClient.top) {
                ret = HTTOP;
            } else if (point.y > rcClient.bottom) {
                ret = HTBOTTOM;
            }
        }
    }
    return ret;
}

void CChildView::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
    BYTE flag = 0;
    switch (nHitTest) {
        case HTTOP:
            flag = WMSZ_TOP;
            break;
        case HTTOPLEFT:
            flag = WMSZ_TOPLEFT;
            break;
        case HTTOPRIGHT:
            flag = WMSZ_TOPRIGHT;
            break;
        case HTLEFT:
            flag = WMSZ_LEFT;
            break;
        case HTRIGHT:
            flag = WMSZ_RIGHT;
            break;
        case HTBOTTOM:
            flag = WMSZ_BOTTOM;
            break;
        case HTBOTTOMLEFT:
            flag = WMSZ_BOTTOMLEFT;
            break;
        case HTBOTTOMRIGHT:
            flag = WMSZ_BOTTOMRIGHT;
            break;
    }
    if (flag) {
        AfxGetMainFrame()->SendMessage(WM_SYSCOMMAND, SC_SIZE | flag, MAKELPARAM(point.x, point.y));
    }
}
