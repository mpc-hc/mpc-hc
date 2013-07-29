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

#pragma once

#include "DSMPropertyBag.h"

class CPlayerSeekBar : public CDialogBar
{
    DECLARE_DYNAMIC(CPlayerSeekBar)
public:
    CPlayerSeekBar();
    virtual ~CPlayerSeekBar();
    virtual BOOL Create(CWnd* pParentWnd);

private:
    enum { TIMER_SHOWHIDE_TOOLTIP = 1, TIMER_HOVER_CAPTURED };

    __int64 m_start, m_stop, m_pos, m_posreal;
    bool m_bEnabled;
    bool m_bSeekable;
    bool m_bHovered;
    CPoint m_hoverPoint;
    HCURSOR m_cursor;

    CToolTipCtrl m_tooltip;
    enum { TOOLTIP_HIDDEN, TOOLTIP_TRIGGERED, TOOLTIP_VISIBLE } m_tooltipState;
    TOOLINFO m_ti;
    CPoint m_tooltipPoint;
    bool m_bIgnoreLastTooltipPoint;
    CString m_tooltipText;

    CComPtr<IDSMChapterBag> m_pChapterBag;
    CCritSec m_csChapterBag; // Graph thread sets the chapter bag

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    void MoveThumb(CPoint point);
    void SyncVideoToThumb();
    __int64 CalculatePosition(REFERENCE_TIME rt);
    __int64 CalculatePosition(CPoint point);
    void SyncThumbToVideo(__int64 pos);

    CRect GetChannelRect() const;
    CRect GetThumbRect() const;
    CRect GetInnerThumbRect() const;

    void UpdateTooltip(CPoint point);
    void UpdateToolTipPosition();
    void UpdateToolTipText();

public:
    void Enable(bool bEnable);
    void HideToolTip();

    void GetRange(__int64& start, __int64& stop) const;
    void SetRange(__int64 start, __int64 stop);
    __int64 GetPos() const;
    __int64 GetPosReal() const;
    void SetPos(__int64 pos);

    void SetChapterBag(CComPtr<IDSMChapterBag>& pCB);
    void RemoveChapters();

private:
    DECLARE_MESSAGE_MAP()

    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMouseLeave();

    BOOL OnPlayStop(UINT nID);
};
