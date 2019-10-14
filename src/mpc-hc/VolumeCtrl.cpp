/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2016 see Authors.txt
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
#include "VolumeCtrl.h"
#include "AppSettings.h"
#include "CMPCTheme.h"
#undef SubclassWindow


// CVolumeCtrl

IMPLEMENT_DYNAMIC(CVolumeCtrl, CSliderCtrl)
CVolumeCtrl::CVolumeCtrl(bool fSelfDrawn)
    : m_fSelfDrawn(fSelfDrawn)
    ,m_bDrag(false)
    ,m_bHover(false)
{
}

CVolumeCtrl::~CVolumeCtrl()
{
}

bool CVolumeCtrl::Create(CWnd* pParentWnd)
{
    if (!CSliderCtrl::Create(WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_HORZ | TBS_TOOLTIPS, CRect(0, 0, 0, 0), pParentWnd, IDC_SLIDER1)) {
        return false;
    }

    const CAppSettings& s = AfxGetAppSettings();
    EnableToolTips(TRUE);
    SetRange(0, 100);
    SetPos(s.nVolume);
    SetPageSize(s.nVolumeStep);
    SetLineSize(0);

    if (s.bMPCThemeLoaded) {
        CToolTipCtrl* pTip = GetToolTips();
        if (NULL != pTip) {
            themedToolTip.SubclassWindow(pTip->m_hWnd);
        }
    }

    return true;
}

void CVolumeCtrl::SetPosInternal(int pos)
{
    SetPos(pos);
    GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM(static_cast<WORD>(pos), SB_THUMBPOSITION), reinterpret_cast<LPARAM>(m_hWnd)); // this will be reflected back on us
    m_bDrag = true;
}

void CVolumeCtrl::IncreaseVolume()
{
    // align volume up to step. recommend using steps 1, 2, 5 and 10
    SetPosInternal(GetPos() + GetPageSize() - GetPos() % GetPageSize());

}

void CVolumeCtrl::DecreaseVolume()
{
    // align volume down to step. recommend using steps 1, 2, 5 and 10
    int m = GetPos() % GetPageSize();
    SetPosInternal(GetPos() - (m ? m : GetPageSize()));
}

BEGIN_MESSAGE_MAP(CVolumeCtrl, CSliderCtrl)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
    ON_WM_LBUTTONDOWN()
    ON_WM_SETFOCUS()
    ON_WM_HSCROLL_REFLECT()
    ON_WM_SETCURSOR()
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
    ON_WM_MOUSEWHEEL()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

// CVolumeCtrl message handlers

void CVolumeCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

    LRESULT lr = CDRF_DODEFAULT;

    const CAppSettings& s = AfxGetAppSettings();
    if (m_fSelfDrawn)
        switch (pNMCD->dwDrawStage) {
            case CDDS_PREPAINT:
                lr = CDRF_NOTIFYITEMDRAW;
                break;

            case CDDS_ITEMPREPAINT:

                if (pNMCD->dwItemSpec == TBCD_CHANNEL) {
                    CDC dc;
                    dc.Attach(pNMCD->hdc);

                    if (s.bMPCThemeLoaded) {
                        CRect rect;
                        GetClientRect(rect);
                        dc.FillSolidRect(&rect, CMPCTheme::PlayerBGColor);
                    }

                    CRect channelRect;
                    GetChannelRect(channelRect);
                    CRect thumbRect;
                    GetThumbRect(thumbRect);

                    CopyRect(&pNMCD->rc, CRect(channelRect.left, thumbRect.top + 2, channelRect.right - 2, thumbRect.bottom - 2));

                    CPen shadow;
                    CPen light;
                    if (s.bMPCThemeLoaded) {
                        shadow.CreatePen(PS_SOLID, 1, CMPCTheme::ShadowColor);
                        light.CreatePen(PS_SOLID, 1, CMPCTheme::LightColor);
                        CRect r(pNMCD->rc);
                        r.DeflateRect(0, 6, 0, 6);
                        dc.FillSolidRect(r, CMPCTheme::ScrollBGColor);
                        CBrush fb;
                        fb.CreateSolidBrush(CMPCTheme::NoBorderColor);
                        dc.FrameRect(r, &fb);
                    } else {
                        shadow.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
                        light.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
                        CPen* old = dc.SelectObject(&light);
                        dc.MoveTo(pNMCD->rc.right, pNMCD->rc.top);
                        dc.LineTo(pNMCD->rc.right, pNMCD->rc.bottom);
                        dc.LineTo(pNMCD->rc.left, pNMCD->rc.bottom);
                        dc.SelectObject(&shadow);
                        dc.LineTo(pNMCD->rc.right, pNMCD->rc.top);
                        dc.SelectObject(old);
                    }

                    dc.Detach();
                    lr = CDRF_SKIPDEFAULT;
                } else if (pNMCD->dwItemSpec == TBCD_THUMB) {
                    CDC dc;
                    dc.Attach(pNMCD->hdc);
                    pNMCD->rc.bottom--;
                    CRect r(pNMCD->rc);
                    r.DeflateRect(0, 0, 1, 0);

                    COLORREF shadow = GetSysColor(COLOR_3DSHADOW);
                    COLORREF light = GetSysColor(COLOR_3DHILIGHT);
                    if (s.bMPCThemeLoaded) {
                        CBrush fb;
                        if (m_bDrag) {
                            dc.FillSolidRect(r, CMPCTheme::ScrollThumbDragColor);
                        } else if(m_bHover) {
                            dc.FillSolidRect(r, CMPCTheme::ScrollThumbHoverColor);
                        } else {
                            dc.FillSolidRect(r, CMPCTheme::ScrollThumbColor);
                        }
                        fb.CreateSolidBrush(CMPCTheme::NoBorderColor);
                        dc.FrameRect(r, &fb);
                    } else {

                        dc.Draw3dRect(&r, light, 0);
                        r.DeflateRect(0, 0, 1, 1);
                        dc.Draw3dRect(&r, light, shadow);
                        r.DeflateRect(1, 1, 1, 1);
                        dc.FillSolidRect(&r, GetSysColor(COLOR_BTNFACE));
                        dc.SetPixel(r.left + 7, r.top - 1, GetSysColor(COLOR_BTNFACE));
                    }

                    dc.Detach();
                    lr = CDRF_SKIPDEFAULT;
                }

                break;
        };

    pNMCD->uItemState &= ~CDIS_FOCUS;

    *pResult = lr;
}

void CVolumeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRect r;
    GetChannelRect(&r);

    if (r.left >= r.right) {
        return;
    }

    int start, stop;
    GetRange(start, stop);

    r.left += 3;
    r.right -= 4;

    if (point.x < r.left) {
        SetPosInternal(start);
    } else if (point.x >= r.right) {
        SetPosInternal(stop);
    } else {
        int w = r.right - r.left;
        if (start < stop) {
            SetPosInternal(start + ((stop - start) * (point.x - r.left) + (w / 2)) / w);
        }
    }
    m_bDrag = true;
    invalidateThumb();
    CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CVolumeCtrl::OnSetFocus(CWnd* pOldWnd)
{
    CSliderCtrl::OnSetFocus(pOldWnd);

    AfxGetMainWnd()->SetFocus(); // don't focus on us, parents will take care of our positioning
}

void CVolumeCtrl::HScroll(UINT nSBCode, UINT nPos)
{
    AfxGetAppSettings().nVolume = GetPos();

    CFrameWnd* pFrame = GetParentFrame();
    if (pFrame && pFrame != GetParent()) {
        pFrame->PostMessage(WM_HSCROLL, MAKEWPARAM(static_cast<WORD>(nPos), static_cast<WORD>(nSBCode)), reinterpret_cast<LPARAM>(m_hWnd));
    }
}

BOOL CVolumeCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
    return TRUE;
}

BOOL CVolumeCtrl::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    TOOLTIPTEXT* pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMHDR);
    CString str;
    str.Format(IDS_VOLUME, GetPos());
    _tcscpy_s(pTTT->szText, str);
    pTTT->hinst = nullptr;

    *pResult = 0;

    return TRUE;
}

BOOL CVolumeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
    if (zDelta > 0) {
        IncreaseVolume();
    } else if (zDelta < 0) {
        DecreaseVolume();
    } else {
        return FALSE;
    }
    return TRUE;
}

void CVolumeCtrl::invalidateThumb() {
    SetRangeMax(100, TRUE);
}


void CVolumeCtrl::checkHover(CPoint point) {
    CRect thumbRect;
    GetThumbRect(thumbRect);
    bool oldHover = m_bHover;
    m_bHover = false;
    if (thumbRect.PtInRect(point)) {
        m_bHover = true;
    }

    if (m_bHover != oldHover)
        invalidateThumb();
}

void CVolumeCtrl::OnMouseMove(UINT nFlags, CPoint point) {
    checkHover(point);
    CSliderCtrl::OnMouseMove(nFlags, point);
}


void CVolumeCtrl::OnLButtonUp(UINT nFlags, CPoint point) {
    m_bDrag = false;
    invalidateThumb();
    checkHover(point);
    CSliderCtrl::OnLButtonUp(nFlags, point);
}


void CVolumeCtrl::OnMouseLeave() {
    checkHover(CPoint(-1 - 1));
    CSliderCtrl::OnMouseLeave();
}
