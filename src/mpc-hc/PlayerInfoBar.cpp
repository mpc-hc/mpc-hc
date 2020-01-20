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
#include "PlayerInfoBar.h"
#include "MainFrm.h"
#include "CMPCTheme.h"


// CPlayerInfoBar

IMPLEMENT_DYNAMIC(CPlayerInfoBar, CDialogBar)
CPlayerInfoBar::CPlayerInfoBar(CMainFrame* pMainFrame)
    : m_pMainFrame(pMainFrame)
{
    GetEventd().Connect(m_eventc, {
        MpcEvent::DPI_CHANGED,
    }, std::bind(&CPlayerInfoBar::EventCallback, this, std::placeholders::_1));
}

CPlayerInfoBar::~CPlayerInfoBar()
{
}

bool CPlayerInfoBar::SetLine(CString label, CString info)
{
    const CAppSettings& s = AfxGetAppSettings();
    info.Trim();
    if (info.IsEmpty()) {
        return RemoveLine(label);
    }

    for (size_t idx = 0; idx < m_label.GetCount(); idx++) {
        CString tmp;
        m_label[idx]->GetWindowText(tmp);
        if (label == tmp) {
            m_info[idx]->GetWindowText(tmp);
            if (info != tmp) {
                m_info[idx]->SetWindowText(info);
                if (s.bMPCThemeLoaded) {
                    themedToolTip.UpdateTipText(info, m_info[idx]);
                } else {
                    m_tooltip.UpdateTipText(info, m_info[idx]);
                }
            }
            return false;
        }
    }

    CAutoPtr<CStatusLabel> l(DEBUG_NEW CStatusLabel(m_pMainFrame->m_dpi, true, false));
    l->Create(label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_OWNERDRAW, CRect(0, 0, 0, 0), this);
    m_label.Add(l);

    CAutoPtr<CStatusLabel> i(DEBUG_NEW CStatusLabel(m_pMainFrame->m_dpi, false, true));
    i->Create(info, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_OWNERDRAW | SS_NOTIFY, CRect(0, 0, 0, 0), this);
    if (s.bMPCThemeLoaded) {
        themedToolTip.AddTool(i, info);
    } else {
        m_tooltip.AddTool(i, info);
    }
    m_info.Add(i);

    Relayout();

    return true;
}

void CPlayerInfoBar::GetLine(CString label, CString& info)
{
    info.Empty();

    for (size_t idx = 0; idx < m_label.GetCount(); idx++) {
        CString tmp;
        m_label[idx]->GetWindowText(tmp);
        if (label == tmp) {
            m_info[idx]->GetWindowText(tmp);
            info = tmp;
            return;
        }
    }
}

bool CPlayerInfoBar::RemoveLine(CString label)
{
    const CAppSettings& s = AfxGetAppSettings();
    for (size_t i = 0; i < m_label.GetCount(); i++) {
        CString tmp;
        m_label[i]->GetWindowText(tmp);
        if (label == tmp) {
            if (s.bMPCThemeLoaded) {
                themedToolTip.DelTool(m_info[i]);
            } else {
                m_tooltip.DelTool(m_info[i]);
            }
            m_label.RemoveAt(i);
            m_info.RemoveAt(i);
            Relayout();
            return true;
        }
    }
    return false;
}

void CPlayerInfoBar::RemoveAllLines()
{
    m_label.RemoveAll();
    m_info.RemoveAll();

    // invalidate and redraw bypassing message queue
    Invalidate();
    UpdateWindow();
}

BOOL CPlayerInfoBar::Create(CWnd* pParentWnd)
{
    BOOL res = CDialogBar::Create(pParentWnd, IDD_PLAYERINFOBAR, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_BOTTOM, IDD_PLAYERINFOBAR);

    if (AfxGetAppSettings().bMPCThemeLoaded) {
        themedToolTip.Create(this, TTS_NOPREFIX);
        themedToolTip.Activate(TRUE);
        themedToolTip.SetMaxTipWidth(m_pMainFrame->m_dpi.ScaleX(500));
        themedToolTip.SetDelayTime(TTDT_AUTOPOP, 10000);
    } else {
        m_tooltip.Create(this, TTS_NOPREFIX);
        m_tooltip.Activate(TRUE);
        m_tooltip.SetMaxTipWidth(m_pMainFrame->m_dpi.ScaleX(500));
        m_tooltip.SetDelayTime(TTDT_AUTOPOP, 10000);
    }

    return res;
}

BOOL CPlayerInfoBar::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CDialogBar::PreCreateWindow(cs)) {
        return FALSE;
    }

    m_dwStyle &= ~CBRS_BORDER_TOP;
    m_dwStyle &= ~CBRS_BORDER_BOTTOM;

    return TRUE;
}

CSize CPlayerInfoBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    CRect r;
    GetParent()->GetClientRect(&r);
    r.bottom = r.top + (LONG)m_label.GetCount() * m_pMainFrame->m_dpi.ScaleY(17) +
               (m_label.GetCount() ? m_pMainFrame->m_dpi.ScaleY(2) * 2 : 0);
    return r.Size();
}

void CPlayerInfoBar::EventCallback(MpcEvent ev)
{
    switch (ev) {
        case MpcEvent::DPI_CHANGED:
            for (size_t i = 0; i < m_label.GetCount(); i++) {
                m_label[i]->ScaleFont(m_pMainFrame->m_dpi);
            }
            for (size_t i = 0; i < m_info.GetCount(); i++) {
                m_info[i]->ScaleFont(m_pMainFrame->m_dpi);
            }
            break;

        default:
            ASSERT(FALSE);
    }
}

void CPlayerInfoBar::Relayout()
{
    CRect r;
    GetParent()->GetClientRect(&r);

    int w = m_pMainFrame->m_dpi.ScaleX(100);
    const int h = m_pMainFrame->m_dpi.ScaleY(17);
    int y = m_pMainFrame->m_dpi.ScaleY(2);

    for (size_t i = 0; i < m_label.GetCount(); i++) {
        CDC* pDC = m_label[i]->GetDC();
        CString str;
        m_label[i]->GetWindowText(str);
        w = std::max<int>(w, pDC->GetTextExtent(str).cx);
        m_label[i]->ReleaseDC(pDC);
    }

    const int sep = m_pMainFrame->m_dpi.ScaleX(10);
    for (size_t i = 0; i < m_label.GetCount(); i++, y += h) {
        m_label[i]->MoveWindow(1, y, w - sep, h);
        m_info[i]->MoveWindow(w + sep, y, r.Width() - w - sep - 1, h);
    }
}

BEGIN_MESSAGE_MAP(CPlayerInfoBar, CDialogBar)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CPlayerInfoBar message handlers

BOOL CPlayerInfoBar::PreTranslateMessage(MSG* pMsg)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (IsWindow(themedToolTip)) {
            themedToolTip.RelayEvent(pMsg);
        }
    } else {
        if (IsWindow(m_tooltip)) {
            m_tooltip.RelayEvent(pMsg);
        }
    }
    return __super::PreTranslateMessage(pMsg);
}

BOOL CPlayerInfoBar::OnEraseBkgnd(CDC* pDC)
{
    for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow()) {
        CRect r;
        pChild->GetClientRect(&r);
        pChild->MapWindowPoints(this, &r);
        pDC->ExcludeClipRect(&r);
    }

    CRect r;
    GetClientRect(&r);

    CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

    if (pFrame->m_pLastBar != this || pFrame->m_fFullScreen) {
        r.InflateRect(0, 0, 0, 1);
    }

    if (pFrame->m_fFullScreen) {
        r.InflateRect(1, 0, 1, 0);
    }

    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        pDC->FillSolidRect(&r, CMPCTheme::NoBorderColor);
    } else {
        pDC->Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
    }

    r.DeflateRect(1, 1);

    pDC->FillSolidRect(&r, 0);

    return TRUE;
}

void CPlayerInfoBar::OnSize(UINT nType, int cx, int cy)
{
    CDialogBar::OnSize(nType, cx, cy);

    Relayout();

    Invalidate();
}

void CPlayerInfoBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
    if (!pFrame->m_fFullScreen) {
        MapWindowPoints(pFrame, &point, 1);
        pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
    }
}
