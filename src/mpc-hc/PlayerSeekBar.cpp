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
#define TOOLTIP_HIDE_TIMEOUT 3000
#define HOVER_CAPTURED_TIMEOUT 100
#define HOVER_CAPTURED_IGNORE_X_DELTA 1

IMPLEMENT_DYNAMIC(CPlayerSeekBar, CDialogBar)

CPlayerSeekBar::CPlayerSeekBar()
    : m_rtStart(0)
    , m_rtStop(0)
    , m_rtPos(0)
    , m_bEnabled(false)
    , m_bHasDuration(false)
    , m_rtHoverPos(0)
    , m_bHovered(false)
    , m_cursor(AfxGetApp()->LoadStandardCursor(IDC_HAND))
    , m_tooltipState(TOOLTIP_HIDDEN)
    , m_bIgnoreLastTooltipPoint(true)
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

void CPlayerSeekBar::MoveThumb(const CPoint& point)
{
    if (m_bHasDuration) {
        REFERENCE_TIME rtPos = PositionFromClientPoint(point);
        if (AfxGetAppSettings().bFastSeek ^ (GetKeyState(VK_SHIFT) < 0)) {
            rtPos = AfxGetMainFrame()->GetClosestKeyFrame(rtPos);
        }
        SyncThumbToVideo(rtPos);
    }
}

void CPlayerSeekBar::SyncVideoToThumb()
{
    GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_rtPos, SB_THUMBTRACK), (LPARAM)m_hWnd);
}

long CPlayerSeekBar::ChannelPointFromPosition(REFERENCE_TIME rtPos) const
{
    rtPos = min(m_rtStop, max(m_rtStart, rtPos));
    long ret = 0;
    auto w = GetChannelRect().Width();
    if (m_bHasDuration) {
        ret = (long)(w * (rtPos - m_rtStart) / (m_rtStop - m_rtStart));
    }
    if (ret >= w) {
        ret = w - 1;
    }
    return ret;
}

REFERENCE_TIME CPlayerSeekBar::PositionFromClientPoint(const CPoint& point) const
{
    REFERENCE_TIME rtRet = -1;
    if (m_bHasDuration) {
        ASSERT(m_rtStart < m_rtStop);
        const CRect channelRect(GetChannelRect());
        auto channelPointX = (point.x < channelRect.left) ? channelRect.left :
                             (point.x > channelRect.right) ? channelRect.right : point.x;
        ASSERT(channelPointX >= channelRect.left && channelPointX <= channelRect.right);
        rtRet = m_rtStart + (channelPointX - channelRect.left) * (m_rtStop - m_rtStart) / channelRect.Width();
    }
    return rtRet;
}

void CPlayerSeekBar::SyncThumbToVideo(REFERENCE_TIME rtPos)
{
    if (m_rtPos == rtPos) {
        return;
    }

    m_rtPos = rtPos;

    if (m_bHasDuration) {
        CRect newThumbRect(GetThumbRect());
        if (newThumbRect != m_lastThumbRect) {
            InvalidateRect(newThumbRect | m_lastThumbRect);
            auto pFrame = AfxGetMainFrame();
            if (pFrame && AfxGetAppSettings().fUseWin7TaskBar && pFrame->m_pTaskbarList) {
                pFrame->m_pTaskbarList->SetProgressValue(pFrame->m_hWnd, m_rtPos, m_rtStop);
            }
        }
    }
}

void CPlayerSeekBar::CreateThumb(bool bEnabled, CDC& parentDC)
{
    auto& pThumb = bEnabled ? m_pEnabledThumb : m_pDisabledThumb;
    pThumb = std::unique_ptr<CDC>(new CDC());

    if (pThumb->CreateCompatibleDC(&parentDC)) {
        COLORREF
        white  = GetSysColor(COLOR_WINDOW),
        shadow = GetSysColor(COLOR_3DSHADOW),
        light  = GetSysColor(COLOR_3DHILIGHT),
        bkg    = GetSysColor(COLOR_BTNFACE);

        CRect r(GetThumbRect());
        r.MoveToXY(0, 0);
        CRect ri(GetInnerThumbRect(bEnabled, r));

        CBitmap bmp;
        VERIFY(bmp.CreateCompatibleBitmap(&parentDC, r.Width(), r.Height()));
        VERIFY(pThumb->SelectObject(bmp));

        pThumb->Draw3dRect(&r, light, 0);
        r.DeflateRect(0, 0, 1, 1);
        pThumb->Draw3dRect(&r, light, shadow);
        r.DeflateRect(1, 1, 1, 1);

        CBrush b(bkg);

        pThumb->FrameRect(&r, &b);
        r.DeflateRect(0, 1, 0, 1);
        pThumb->FrameRect(&r, &b);

        r.DeflateRect(1, 1, 0, 0);
        pThumb->Draw3dRect(&r, shadow, bkg);

        if (bEnabled) {
            r.DeflateRect(1, 1, 1, 2);
            CPen whitePen(PS_INSIDEFRAME, 1, white);
            CPen* old = pThumb->SelectObject(&whitePen);
            pThumb->MoveTo(r.left, r.top);
            pThumb->LineTo(r.right, r.top);
            pThumb->MoveTo(r.left, r.bottom);
            pThumb->LineTo(r.right, r.bottom);
            pThumb->SelectObject(old);
            pThumb->SetPixel(r.CenterPoint().x, r.top, 0);
            pThumb->SetPixel(r.CenterPoint().x, r.bottom, 0);
        }
    } else {
        ASSERT(FALSE);
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
    const CRect channelRect(GetChannelRect());
    long x = channelRect.left + ChannelPointFromPosition(m_rtPos);
    long y = channelRect.CenterPoint().y;
    return CRect(x - 6, y - 7, x + 7, y + 8);
}

CRect CPlayerSeekBar::GetInnerThumbRect(bool bEnabled, const CRect& thumbRect) const
{
    CRect r(thumbRect);
    r.DeflateRect(3, bEnabled ? 5 : 4, 3, bEnabled ? 5 : 4);
    return r;
}

void CPlayerSeekBar::UpdateTooltip(const CPoint& point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    if (!m_bHasDuration || !clientRect.PtInRect(point)) {
        HideToolTip();
        return;
    }

    switch (m_tooltipState) {
        case TOOLTIP_HIDDEN: {
            // If mouse moved or the tooltip wasn't hidden by timeout
            if (point != m_tooltipPoint || m_bIgnoreLastTooltipPoint) {
                m_bIgnoreLastTooltipPoint = false;
                // Start show tooltip countdown
                m_tooltipState = TOOLTIP_TRIGGERED;
                VERIFY(SetTimer(TIMER_SHOWHIDE_TOOLTIP, TOOLTIP_SHOW_DELAY, nullptr));
                // Track mouse leave
                TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, m_hWnd };
                VERIFY(TrackMouseEvent(&tme));
            }
        }
        break;
        case TOOLTIP_TRIGGERED:
            // Do nothing until tooltip is shown
            break;
        case TOOLTIP_VISIBLE:
            // Update the tooltip if needed
            ASSERT(!m_bIgnoreLastTooltipPoint);
            if (point != m_tooltipPoint) {
                m_tooltipPoint = point;
                UpdateToolTipPosition();
                UpdateToolTipText();
                VERIFY(SetTimer(TIMER_SHOWHIDE_TOOLTIP, TOOLTIP_HIDE_TIMEOUT, nullptr));
            }
            break;
        default:
            ASSERT(FALSE);
    }
}

void CPlayerSeekBar::UpdateToolTipPosition()
{
    CSize bubbleSize(m_tooltip.GetBubbleSize(&m_ti));
    CRect windowRect;
    GetWindowRect(windowRect);
    CPoint point(m_tooltipPoint);

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
}

void CPlayerSeekBar::UpdateToolTipText()
{
    ASSERT(m_bHasDuration);
    REFERENCE_TIME rtNow = PositionFromClientPoint(m_tooltipPoint);

    CString time;
    GUID timeFormat = AfxGetMainFrame()->GetTimeFormat();
    if (timeFormat == TIME_FORMAT_MEDIA_TIME) {
        DVD_HMSF_TIMECODE tcNow = RT2HMS_r(rtNow);
        if (tcNow.bHours > 0) {
            time.Format(_T("%02u:%02u:%02u"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
        } else {
            time.Format(_T("%02u:%02u"), tcNow.bMinutes, tcNow.bSeconds);
        }
    } else if (timeFormat == TIME_FORMAT_FRAME) {
        time.Format(_T("%I64d"), rtNow);
    } else {
        ASSERT(FALSE);
    }

    CComBSTR chapterName;
    {
        CAutoLock lock(&m_csChapterBag);
        if (m_pChapterBag) {
            REFERENCE_TIME rt = rtNow;
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
        KillTimer(TIMER_SHOWHIDE_TOOLTIP);
        m_tooltip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
        m_tooltipState = TOOLTIP_HIDDEN;
    }
}

void CPlayerSeekBar::GetRange(REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop) const
{
    rtStart = m_rtStart;
    rtStop = m_rtStop;
}

void CPlayerSeekBar::SetRange(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
    if (rtStart < rtStop) {
        m_rtStart = rtStart;
        m_rtStop = rtStop;
        if (!m_bHasDuration) {
            m_bHasDuration = true;
            Invalidate();
        }
    } else {
        m_rtStart = 0;
        m_rtStop = 0;
        if (m_bHasDuration) {
            m_bHasDuration = false;
            HideToolTip();
            if (GetCapture() == this) {
                ReleaseCapture();
                KillTimer(TIMER_HOVER_CAPTURED);
            }
            Invalidate();
        }
    }
}

REFERENCE_TIME CPlayerSeekBar::GetPos() const
{
    return m_rtPos;
}

void CPlayerSeekBar::SetPos(REFERENCE_TIME rtPos)
{
    if (GetCapture() == this) {
        return;
    }

    SyncThumbToVideo(rtPos);
}

bool CPlayerSeekBar::HasDuration() const
{
    return m_bHasDuration;
}

void CPlayerSeekBar::SetChapterBag(IDSMChapterBag* pCB)
{
    CAutoLock lock(&m_csChapterBag);
    m_pChapterBag = pCB;
}

void CPlayerSeekBar::RemoveChapters()
{
    SetChapterBag(nullptr);
}

BEGIN_MESSAGE_MAP(CPlayerSeekBar, CDialogBar)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SETCURSOR()
    ON_WM_TIMER()
    ON_WM_MOUSELEAVE()
    ON_WM_THEMECHANGED()
END_MESSAGE_MAP()

void CPlayerSeekBar::OnPaint()
{
    CPaintDC dc(this);

    COLORREF
    dark   = GetSysColor(COLOR_GRAYTEXT),
    white  = GetSysColor(COLOR_WINDOW),
    shadow = GetSysColor(COLOR_3DSHADOW),
    light  = GetSysColor(COLOR_3DHILIGHT),
    bkg    = GetSysColor(COLOR_BTNFACE);

    // Thumb
    {
        auto& pThumb = m_bEnabled ? m_pEnabledThumb : m_pDisabledThumb;
        if (!pThumb) {
            CreateThumb(m_bEnabled, dc);
            ASSERT(pThumb);
        }
        CRect r(GetThumbRect());
        CRect ri(GetInnerThumbRect(m_bEnabled, r));

        CRgn rg, rgi;
        VERIFY(rg.CreateRectRgnIndirect(&r));
        VERIFY(rgi.CreateRectRgnIndirect(&ri));

        ExtSelectClipRgn(dc, rgi, RGN_DIFF);
        VERIFY(dc.BitBlt(r.TopLeft().x, r.TopLeft().y, r.Width(), r.Height(), pThumb.get(), 0, 0, SRCCOPY));
        ExtSelectClipRgn(dc, rg, RGN_XOR);

        m_lastThumbRect = r;
    }

    const CRect channelRect(GetChannelRect());

    // Chapters
    if (m_bHasDuration) {
        CAutoLock lock(&m_csChapterBag);
        if (m_pChapterBag) {
            for (DWORD i = 0; i < m_pChapterBag->ChapGetCount(); i++) {
                REFERENCE_TIME rtChap;
                if (SUCCEEDED(m_pChapterBag->ChapGet(i, &rtChap, nullptr))) {
                    long chanPos = channelRect.left + ChannelPointFromPosition(rtChap);
                    CRect r(chanPos, channelRect.top, chanPos + 1, channelRect.bottom);
                    if (r.right < channelRect.right) {
                        r.right++;
                    }
                    ASSERT(r.right <= channelRect.right);
                    dc.FillSolidRect(&r, dark);
                    dc.ExcludeClipRect(&r);
                } else {
                    ASSERT(FALSE);
                }
            }
        }
    }

    // Channel
    {
        dc.FillSolidRect(&channelRect, m_bEnabled ? white : bkg);
        CRect r(channelRect);
        r.InflateRect(1, 1);
        dc.Draw3dRect(&r, shadow, light);
        dc.ExcludeClipRect(&r);
    }

    // Background
    {
        CRect r;
        GetClientRect(&r);
        dc.FillSolidRect(&r, bkg);
    }
}

void CPlayerSeekBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    if (m_bEnabled && m_bHasDuration && clientRect.PtInRect(point)) {
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

void CPlayerSeekBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    OnLButtonDown(nFlags, point);
}

void CPlayerSeekBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (GetCapture() == this) {
        ReleaseCapture();
        KillTimer(TIMER_HOVER_CAPTURED);
        if (!m_bHovered || (abs(point.x - m_hoverPoint.x) > HOVER_CAPTURED_IGNORE_X_DELTA &&
                            m_rtPos != m_rtHoverPos)) {
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
    if (m_bEnabled && m_bHasDuration) {
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
        case TIMER_SHOWHIDE_TOOLTIP:
            if (m_tooltipState == TOOLTIP_TRIGGERED && m_bHasDuration) {
                m_tooltipPoint = point;
                UpdateToolTipText();
                m_tooltip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
                UpdateToolTipPosition();
                m_tooltipState = TOOLTIP_VISIBLE;
                VERIFY(SetTimer(TIMER_SHOWHIDE_TOOLTIP, TOOLTIP_HIDE_TIMEOUT, nullptr));
            } else if (m_tooltipState == TOOLTIP_VISIBLE) {
                HideToolTip();
                ASSERT(!m_bIgnoreLastTooltipPoint);
                KillTimer(TIMER_SHOWHIDE_TOOLTIP);
            } else {
                KillTimer(TIMER_SHOWHIDE_TOOLTIP);
            }
            break;
        case TIMER_HOVER_CAPTURED:
            if (GetCapture() == this && (!m_bHovered || m_rtHoverPos != m_rtPos)) {
                m_bHovered = true;
                m_rtHoverPos = m_rtPos;
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
    m_bIgnoreLastTooltipPoint = true;
}

LRESULT CPlayerSeekBar::OnThemeChanged()
{
    m_pEnabledThumb.release();
    m_pDisabledThumb.release();
    return __super::OnThemeChanged();
}
