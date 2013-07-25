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
#include "PlayerSeekBar.h"
#include "MainFrm.h"

#define TOOLTIP_SHOW_DELAY 100
#define HOVER_CAPTURED_TIMEOUT 100
#define HOVER_CAPTURED_IGNORE_X_DELTA 1

IMPLEMENT_DYNAMIC(CPlayerSeekBar, CDialogBar)

CPlayerSeekBar::CPlayerSeekBar()
    : m_start(0)
    , m_stop(100)
    , m_pos(0)
    , m_posreal(0)
    , m_bEnabled(false)
    , m_bSeekable(false)
    , m_bHovered(false)
    , m_cursor(AfxGetApp()->LoadStandardCursor(IDC_HAND))
    , m_tooltipPos(0)
    , m_tooltipState(TOOLTIP_HIDDEN)
    , m_tooltipLastPos(-1)
{
    ZeroMemory(&m_ti, sizeof(m_ti));
    m_ti.cbSize = sizeof(m_ti);
}

CPlayerSeekBar::~CPlayerSeekBar()
{
}

BOOL CPlayerSeekBar::Create(CWnd* pParentWnd)
{
    if (!__super::Create(pParentWnd,
                         IDD_PLAYERSEEKBAR, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_BOTTOM, IDD_PLAYERSEEKBAR)) {
        return FALSE;
    }

    // Should never be RTLed
    ModifyStyleEx(WS_EX_LAYOUTRTL, WS_EX_NOINHERITLAYOUT);

    m_tooltip.Create(this, TTS_NOPREFIX | TTS_ALWAYSTIP);
    m_tooltip.SetMaxTipWidth(SHRT_MAX);

    m_ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
    m_ti.hwnd = m_hWnd;
    m_ti.uId = (UINT_PTR)m_hWnd;
    m_ti.hinst = AfxGetInstanceHandle();
    m_ti.lpszText = nullptr;

    m_tooltip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&m_ti);

    return TRUE;
}

BOOL CPlayerSeekBar::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!__super::PreCreateWindow(cs)) {
        return FALSE;
    }

    m_dwStyle &= ~CBRS_BORDER_TOP;
    m_dwStyle &= ~CBRS_BORDER_BOTTOM;
    m_dwStyle |= CBRS_SIZE_FIXED;

    return TRUE;
}

void CPlayerSeekBar::MoveThumb(CPoint point)
{
    __int64 pos = CalculatePosition(point);

    if (pos >= 0) {
        SyncThumbToVideo(pos);
    }
}

void CPlayerSeekBar::SyncVideoToThumb()
{
    GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_pos, SB_THUMBTRACK), (LPARAM)m_hWnd);
}

__int64 CPlayerSeekBar::CalculatePosition(REFERENCE_TIME rt)
{
    if (rt >= m_start && rt < m_stop) {
        return (__int64)(GetChannelRect().Width() * ((double)(rt) / m_stop) + 1);
    } else {
        return -1;
    }
}

__int64 CPlayerSeekBar::CalculatePosition(CPoint point)
{
    CRect r(GetChannelRect());
    __int64 pos = -1;

    if (r.left >= r.right) {
        pos = -1;
    } else if (point.x < r.left) {
        pos = m_start;
    } else if (point.x >= r.right) {
        pos = m_stop;
    } else {
        __int64 w = r.right - r.left;
        if (m_start < m_stop) {
            pos = m_start + ((m_stop - m_start) * (point.x - r.left) + (w / 2)) / w;
        }
    }

    return pos;
}

void CPlayerSeekBar::SyncThumbToVideo(__int64 pos)
{
    if (m_pos == pos) {
        return;
    }

    CRect before(GetThumbRect());
    m_pos = min(max(pos, m_start), m_stop);
    m_posreal = pos;
    CRect after(GetThumbRect());

    if (before != after) {
        InvalidateRect(before | after);

        auto pFrame = AfxGetMainFrame();
        if (pFrame && (AfxGetAppSettings().fUseWin7TaskBar && pFrame->m_pTaskbarList)) {
            pFrame->m_pTaskbarList->SetProgressValue(pFrame->m_hWnd, pos, m_stop);
        }
    }
}

CRect CPlayerSeekBar::GetChannelRect() const
{
    CRect r;
    GetClientRect(&r);
    r.DeflateRect(8, 9, 9, 0);
    r.bottom = r.top + 5;
    return r;
}

CRect CPlayerSeekBar::GetThumbRect() const
{
    CRect r(GetChannelRect());

    int x = r.left + (int)((m_start < m_stop) ? (__int64)r.Width() * (m_pos - m_start) / (m_stop - m_start) : 0);
    int y = r.CenterPoint().y;

    r.SetRect(x, y, x, y);
    r.InflateRect(6, 7, 7, 8);

    return r;
}

CRect CPlayerSeekBar::GetInnerThumbRect() const
{
    CRect r(GetThumbRect());

    bool bEnabled = m_bEnabled && m_start < m_stop;
    r.DeflateRect(3, bEnabled ? 5 : 4, 3, bEnabled ? 5 : 4);

    return r;
}

void CPlayerSeekBar::UpdateTooltip(CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    if (!m_bEnabled || !m_bSeekable || !clientRect.PtInRect(point)) {
        HideToolTip();
        return;
    }

    m_tooltipPos = CalculatePosition(point);

    switch (m_tooltipState) {
        case TOOLTIP_HIDDEN: {
            // Start show tooltip countdown
            m_tooltipState = TOOLTIP_TRIGGERED;
            VERIFY(SetTimer(TIMER_SHOW_TOOLTIP, TOOLTIP_SHOW_DELAY, nullptr));
            // Track mouse leave
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd };
            VERIFY(TrackMouseEvent(&tme));
        }
        break;
        case TOOLTIP_TRIGGERED:
            // Do nothing until tooltip is shown
            break;
        case TOOLTIP_VISIBLE:
            // Update the tooltip if needed
            if (m_tooltipPos != m_tooltipLastPos) {
                UpdateToolTipText();
                UpdateToolTipPosition(point);
            }
            break;
        default:
            ASSERT(FALSE);
    }
}

void CPlayerSeekBar::UpdateToolTipPosition(CPoint& point)
{
    CSize bubbleSize(m_tooltip.GetBubbleSize(&m_ti));
    CRect windowRect;
    GetWindowRect(windowRect);

    if (AfxGetAppSettings().nTimeTooltipPosition == TIME_TOOLTIP_ABOVE_SEEKBAR) {
        point.x -= bubbleSize.cx / 2 - 2;
        point.y = GetChannelRect().TopLeft().y - (bubbleSize.cy + 13);
    } else {
        point.x += 10;
        point.y += 20;
    }
    point.x = max(0, min(point.x, windowRect.Width() - bubbleSize.cx));
    ClientToScreen(&point);

    m_tooltip.SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(point.x, point.y));
    m_tooltipLastPos = m_tooltipPos;
}

void CPlayerSeekBar::UpdateToolTipText()
{
    DVD_HMSF_TIMECODE tcNow = RT2HMS_r(m_tooltipPos);

    CString time;
    if (tcNow.bHours > 0) {
        time.Format(_T("%02u:%02u:%02u"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
    } else {
        time.Format(_T("%02u:%02u"), tcNow.bMinutes, tcNow.bSeconds);
    }

    CComBSTR chapterName;
    {
        CAutoLock lock(&m_csChapterBag);
        if (m_pChapterBag) {
            REFERENCE_TIME rt = m_tooltipPos;
            m_pChapterBag->ChapLookup(&rt, &chapterName);
        }
    }

    if (chapterName.Length() == 0) {
        m_tooltipText = time;
    } else {
        m_tooltipText.Format(_T("%s - %s"), time, chapterName);
    }

    m_ti.lpszText = (LPTSTR)(LPCTSTR)m_tooltipText;
    m_tooltip.SetToolInfo(&m_ti);
}

void CPlayerSeekBar::Enable(bool bEnable)
{
    if (bEnable != m_bEnabled) {
        m_bEnabled = bEnable;
        Invalidate();
    }
}

void CPlayerSeekBar::HideToolTip()
{
    if (m_tooltipState != TOOLTIP_HIDDEN) {
        KillTimer(TIMER_SHOW_TOOLTIP);
        m_tooltip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
        m_tooltipState = TOOLTIP_HIDDEN;
    }
}

void CPlayerSeekBar::GetRange(__int64& start, __int64& stop) const
{
    start = m_start;
    stop = m_stop;
}

void CPlayerSeekBar::SetRange(__int64 start, __int64 stop)
{
    if (start < stop) {
        m_start = start;
        m_stop = stop;
        m_bSeekable = true;
    } else {
        m_start = 0;
        m_stop = 0;
        if (m_bSeekable) {
            m_bSeekable = false;
            HideToolTip();
            if (GetCapture() == this) {
                ReleaseCapture();
                KillTimer(TIMER_HOVER_CAPTURED);
            }
        }
    }
}

__int64 CPlayerSeekBar::GetPos() const
{
    return m_pos;
}

__int64 CPlayerSeekBar::GetPosReal() const
{
    return m_posreal;
}

void CPlayerSeekBar::SetPos(__int64 pos)
{
    if (GetCapture() == this) {
        return;
    }
    SyncThumbToVideo(pos);
}

void CPlayerSeekBar::SetChapterBag(CComPtr<IDSMChapterBag>& pCB)
{
    CAutoLock lock(&m_csChapterBag);
    m_pChapterBag.Release();
    if (pCB) {
        pCB.CopyTo(&m_pChapterBag);
    }
}

void CPlayerSeekBar::RemoveChapters()
{
    CAutoLock lock(&m_csChapterBag);
    m_pChapterBag.Release();
}

BEGIN_MESSAGE_MAP(CPlayerSeekBar, CDialogBar)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SETCURSOR()
    ON_WM_TIMER()
    ON_WM_MOUSELEAVE()
    ON_COMMAND_EX(ID_PLAY_STOP, OnPlayStop)
END_MESSAGE_MAP()

void CPlayerSeekBar::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    bool bEnabled = m_bEnabled && m_bSeekable;

    COLORREF
    dark   = GetSysColor(COLOR_GRAYTEXT),
    white  = GetSysColor(COLOR_WINDOW),
    shadow = GetSysColor(COLOR_3DSHADOW),
    light  = GetSysColor(COLOR_3DHILIGHT),
    bkg    = GetSysColor(COLOR_BTNFACE);

    // Thumb
    {
        CRect r = GetThumbRect(), r2 = GetInnerThumbRect();
        CRect rt = r, rit = r2;

        dc.Draw3dRect(&r, light, 0);
        r.DeflateRect(0, 0, 1, 1);
        dc.Draw3dRect(&r, light, shadow);
        r.DeflateRect(1, 1, 1, 1);

        CBrush b(bkg);

        dc.FrameRect(&r, &b);
        r.DeflateRect(0, 1, 0, 1);
        dc.FrameRect(&r, &b);

        r.DeflateRect(1, 1, 0, 0);
        dc.Draw3dRect(&r, shadow, bkg);

        if (bEnabled) {
            r.DeflateRect(1, 1, 1, 2);
            CPen white(PS_INSIDEFRAME, 1, white);
            CPen* old = dc.SelectObject(&white);
            dc.MoveTo(r.left, r.top);
            dc.LineTo(r.right, r.top);
            dc.MoveTo(r.left, r.bottom);
            dc.LineTo(r.right, r.bottom);
            dc.SelectObject(old);
            dc.SetPixel(r.CenterPoint().x, r.top, 0);
            dc.SetPixel(r.CenterPoint().x, r.bottom, 0);
        }

        dc.SetPixel(r.CenterPoint().x + 5, r.top - 4, bkg);

        {
            CRgn rgn1, rgn2;
            rgn1.CreateRectRgnIndirect(&rt);
            rgn2.CreateRectRgnIndirect(&rit);
            ExtSelectClipRgn(dc, rgn1, RGN_DIFF);
            ExtSelectClipRgn(dc, rgn2, RGN_OR);
        }
    }

    // Chapters
    {
        CAutoLock lock(&m_csChapterBag);

        if (m_pChapterBag && m_pChapterBag->ChapGetCount() > 1) {
            CRect cr = GetChannelRect();
            REFERENCE_TIME rt;
            for (DWORD i = 0; i < m_pChapterBag->ChapGetCount(); ++i) {
                if (SUCCEEDED(m_pChapterBag->ChapGet(i, &rt, nullptr))) {
                    __int64 pos = CalculatePosition(rt);
                    if (pos < 0) {
                        continue;
                    }

                    LONG chanPos = cr.left + (LONG)pos;
                    CRect chan = GetChannelRect();
                    CRect r;
                    if (chanPos >= chan.right) {
                        r = CRect(chanPos - 1, cr.top, chanPos, cr.bottom); // 1 px width
                    } else {
                        r = CRect(chanPos - 1, cr.top, chanPos + 1, cr.bottom); // 2 px width
                    }

                    dc.FillSolidRect(&r, dark);
                    dc.ExcludeClipRect(&r);
                }
            }
        }
    }

    // Channel
    {
        CRect r = GetChannelRect();

        dc.FillSolidRect(&r, bEnabled ? white : bkg);
        r.InflateRect(1, 1);
        dc.Draw3dRect(&r, shadow, light);
        dc.ExcludeClipRect(&r);
    }

    // Background
    {
        CRect r;
        GetClientRect(&r);
        CBrush b(bkg);
        dc.FillRect(&r, &b);
    }
}

void CPlayerSeekBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    if (m_bEnabled && m_bSeekable && clientRect.PtInRect(point)) {
        m_bHovered = false;
        SetCapture();
        MoveThumb(point);
        VERIFY(SetTimer(TIMER_HOVER_CAPTURED, HOVER_CAPTURED_TIMEOUT, nullptr));
    } else {
        auto pFrame = AfxGetMainFrame();
        if (!pFrame->m_fFullScreen) {
            MapWindowPoints(pFrame, &point, 1);
            pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
        }
    }
}

void CPlayerSeekBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (GetCapture() == this) {
        ReleaseCapture();
        KillTimer(TIMER_HOVER_CAPTURED);
        if (!m_bHovered || abs(point.x - m_hoverPoint.x) > HOVER_CAPTURED_IGNORE_X_DELTA) {
            SyncVideoToThumb();
        }
    }
}

void CPlayerSeekBar::OnMouseMove(UINT nFlags, CPoint point)
{
    if (GetCapture() == this && (nFlags & MK_LBUTTON)) {
        MoveThumb(point);
        VERIFY(SetTimer(TIMER_HOVER_CAPTURED, HOVER_CAPTURED_TIMEOUT, nullptr));
    }
    if (AfxGetAppSettings().fUseTimeTooltip) {
        UpdateTooltip(point);
    }
}

BOOL CPlayerSeekBar::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

BOOL CPlayerSeekBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    BOOL ret = TRUE;
    if (m_bEnabled && m_bSeekable) {
        ::SetCursor(m_cursor);
    } else {
        ret = __super::OnSetCursor(pWnd, nHitTest, message);
    }
    return ret;
}

void CPlayerSeekBar::OnTimer(UINT_PTR nIDEvent)
{
    CPoint point;
    VERIFY(GetCursorPos(&point));
    ScreenToClient(&point);
    switch (nIDEvent) {
        case TIMER_SHOW_TOOLTIP:
            if (m_tooltipState == TOOLTIP_TRIGGERED && m_bEnabled && m_bSeekable) {
                m_tooltipPos = CalculatePosition(point);
                UpdateToolTipText();
                m_tooltip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
                UpdateToolTipPosition(point);
                m_tooltipState = TOOLTIP_VISIBLE;
            }
            KillTimer(TIMER_SHOW_TOOLTIP);
            break;
        case TIMER_HOVER_CAPTURED:
            if (GetCapture() == this) {
                m_bHovered = true;
                m_hoverPoint = point;
                SyncVideoToThumb();
            }
            KillTimer(TIMER_HOVER_CAPTURED);
            break;
        default:
            ASSERT(FALSE);
    }
}

void CPlayerSeekBar::OnMouseLeave()
{
    HideToolTip();
}

BOOL CPlayerSeekBar::OnPlayStop(UINT nID)
{
    SetPos(0);
    return FALSE;
}
