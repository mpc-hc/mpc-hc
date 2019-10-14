/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"

CChildView::CChildView(CMainFrame* pMainFrame)
    : m_vrect(0, 0, 0, 0)
    , CMouseWnd(pMainFrame)
    , m_pMainFrame(pMainFrame)
    , m_bSwitchingFullscreen(false)
    , m_bFirstMedia(true)
{
    LoadImg();
    GetEventd().Connect(m_eventc, {
        MpcEvent::SWITCHING_TO_FULLSCREEN,
        MpcEvent::SWITCHED_TO_FULLSCREEN,
        MpcEvent::SWITCHING_FROM_FULLSCREEN,
        MpcEvent::SWITCHED_FROM_FULLSCREEN,
        MpcEvent::MEDIA_LOADED,
    }, std::bind(&CChildView::EventCallback, this, std::placeholders::_1));
}

CChildView::~CChildView()
{
}

void CChildView::EventCallback(MpcEvent ev)
{
    switch (ev) {
        case MpcEvent::SWITCHING_TO_FULLSCREEN:
        case MpcEvent::SWITCHING_FROM_FULLSCREEN:
            m_bSwitchingFullscreen = true;
            break;
        case MpcEvent::SWITCHED_TO_FULLSCREEN:
        case MpcEvent::SWITCHED_FROM_FULLSCREEN:
            m_bSwitchingFullscreen = false;
            break;
        case MpcEvent::MEDIA_LOADED:
            m_bFirstMedia = false;
            break;
        default:
            ASSERT(FALSE);
    }
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
            m_pMainFrame->IsInteractiveVideo()) {
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

void CChildView::SetVideoRect(const CRect& r)
{
    m_vrect = r;

    Invalidate();
}

void CChildView::LoadImg(const CString& imagePath)
{
    CMPCPngImage img;

    if (!imagePath.IsEmpty()) {
        img.LoadFromFile(imagePath);
    }

    LoadImgInternal(img.Detach());
}

void CChildView::LoadImg(std::vector<BYTE> buffer)
{
    CMPCPngImage img;

    if (!buffer.empty()) {
        img.LoadFromBuffer(buffer.data(), (UINT)buffer.size());
    }

    LoadImgInternal(img.Detach());
}

void CChildView::LoadImgInternal(HGDIOBJ hImg)
{
    CAppSettings& s = AfxGetAppSettings();
    bool bHaveLogo = false;

    m_img.DeleteObject();
    m_resizedImg.Destroy();
    m_bCustomImgLoaded = !!m_img.Attach(hImg);

    if (!m_bCustomImgLoaded && s.fLogoExternal) {
        bHaveLogo = !!m_img.LoadFromFile(s.strLogoFileName);
    }

    if (!bHaveLogo && !m_bCustomImgLoaded) {
        s.fLogoExternal = false;               // use the built-in logo instead
        s.strLogoFileName.Empty();             // clear logo file name
        UINT useLogoId = s.nLogoId;
        if ((UINT)-1 == useLogoId) { //if the user has never chosen a logo, we can try loading a theme default logo
            if (s.bMPCThemeLoaded) {
                useLogoId = CMPCThemeUtil::defaultLogo();
            } else {
                useLogoId = DEF_LOGO;
            }
        }
        if (!m_img.Load(useLogoId)) {          // try the latest selected build-in logo
            s.nLogoId = (UINT)-1; //-1 == never selected, will default
            m_img.Load(DEF_LOGO);  // if fail then use the default logo, should and must never fail
        }
    }

    if (m_hWnd) {
        Invalidate();
    }
}

CSize CChildView::GetLogoSize()
{
    return m_img.GetSize();
}

IMPLEMENT_DYNAMIC(CChildView, CMouseWnd)

BEGIN_MESSAGE_MAP(CChildView, CMouseWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_NCHITTEST()
    ON_WM_NCLBUTTONDOWN()
END_MESSAGE_MAP()

void CChildView::OnPaint()
{
    CPaintDC dc(this);
    m_pMainFrame->RepaintVideo();
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
    CRect r;

    CImage img;
    img.Attach(m_img);

    if ((m_pMainFrame->GetLoadState() != MLS::CLOSED || (!m_bFirstMedia && m_pMainFrame->m_controls.DelayShowNotLoaded())) &&
            !m_pMainFrame->IsD3DFullScreenMode() && !m_pMainFrame->m_fAudioOnly) {
        pDC->ExcludeClipRect(m_vrect);
    } else if (!img.IsNull()) {
        const double dImageAR = double(img.GetWidth()) / img.GetHeight();

        GetClientRect(r);
        int width = r.Width();
        int height = r.Height();
        if (!m_bCustomImgLoaded) {
            // Limit logo size
            // TODO: Use vector logo to preserve quality and remove limit.
            width = std::min(img.GetWidth(), width);
            height = std::min(img.GetHeight(), height);
        }

        double dImgWidth = height * dImageAR;
        double dImgHeight;
        if (width < dImgWidth) {
            dImgWidth = width;
            dImgHeight = dImgWidth / dImageAR;
        } else {
            dImgHeight = height;
        }

        int x = std::lround((r.Width() - dImgWidth) / 2.0);
        int y = std::lround((r.Height() - dImgHeight) / 2.0);

        r = CRect(CPoint(x, y), CSize(std::lround(dImgWidth), std::lround(dImgHeight)));

        if (!r.IsRectEmpty()) {
            if (m_resizedImg.IsNull() || r.Width() != m_resizedImg.GetWidth() || r.Height() != m_resizedImg.GetHeight() || img.GetBPP() != m_resizedImg.GetBPP()) {
                m_resizedImg.Destroy();
                m_resizedImg.Create(r.Width(), r.Height(), std::max(img.GetBPP(), 24));

                HDC hDC = m_resizedImg.GetDC();
                SetStretchBltMode(hDC, STRETCH_HALFTONE);
                img.StretchBlt(hDC, 0, 0, r.Width(), r.Height(), SRCCOPY);
                m_resizedImg.ReleaseDC();
            }

            m_resizedImg.BitBlt(*pDC, r.TopLeft());
            pDC->ExcludeClipRect(r);
        }
    }
    img.Detach();

    GetClientRect(r);
    pDC->FillSolidRect(r, 0);

    return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);
    if (!m_bSwitchingFullscreen) {
        m_pMainFrame->MoveVideoWindow();
    }
    m_pMainFrame->UpdateThumbnailClip();
}

LRESULT CChildView::OnNcHitTest(CPoint point)
{
    LRESULT ret = CWnd::OnNcHitTest(point);
    if (!m_pMainFrame->m_fFullScreen && m_pMainFrame->IsFrameLessWindow()) {
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
        m_pMainFrame->SendMessage(WM_SYSCOMMAND, SC_SIZE | flag, MAKELPARAM(point.x, point.y));
    }
}
