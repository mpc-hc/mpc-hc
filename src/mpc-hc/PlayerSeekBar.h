/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2018 see Authors.txt
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

#include "EventDispatcher.h"
#include "DSMPropertyBag.h"
#include <memory>
#include "CMPCThemeToolTipCtrl.h"

class CMainFrame;

class CPlayerSeekBar : public CDialogBar
{
    DECLARE_DYNAMIC(CPlayerSeekBar)

public:
    CPlayerSeekBar(CMainFrame* pMainFrame);
    virtual ~CPlayerSeekBar();
    virtual BOOL Create(CWnd* pParentWnd);

private:
    enum { TIMER_SHOWHIDE_TOOLTIP = 1 };

    CMainFrame* m_pMainFrame;
    REFERENCE_TIME m_rtStart, m_rtStop, m_rtPos;
    bool m_bEnabled;
    bool m_bHasDuration;
    REFERENCE_TIME m_rtHoverPos;
    CPoint m_hoverPoint;
    HCURSOR m_cursor;
    bool m_bDraggingThumb, m_bHoverThumb;

    EventClient m_eventc;
    void EventCallback(MpcEvent ev);

    CMPCThemeToolTipCtrl m_tooltip;
    enum { TOOLTIP_HIDDEN, TOOLTIP_TRIGGERED, TOOLTIP_VISIBLE } m_tooltipState;
    CFont mpcThemeFont;
    TOOLINFO m_ti;
    CPoint m_tooltipPoint;
    bool m_bIgnoreLastTooltipPoint;
    CString m_tooltipText;

    CComPtr<IDSMChapterBag> m_pChapterBag;
    CCritSec m_csChapterBag; // Graph thread sets the chapter bag

    std::unique_ptr<CDC> m_pEnabledThumb;
    std::unique_ptr<CDC> m_pDisabledThumb;
    CRect m_lastThumbRect;

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz) override;

    void MoveThumb(const CPoint& point);
    void SyncVideoToThumb();
    void checkHover(CPoint point);
    void invalidateThumb();
    void CheckScrollDistance(CPoint point, REFERENCE_TIME minimum_time_change);
    long ChannelPointFromPosition(REFERENCE_TIME rtPos) const;
    REFERENCE_TIME PositionFromClientPoint(const CPoint& point) const;
    void SyncThumbToVideo(REFERENCE_TIME rtPos);

    void CreateThumb(bool bEnabled, CDC& parentDC);
    CRect GetChannelRect() const;
    CRect GetThumbRect() const;
    CRect GetInnerThumbRect(bool bEnabled, const CRect& thumbRect) const;

    void UpdateTooltip(const CPoint& point);
    void UpdateToolTipPosition();
    void UpdateToolTipText();

public:
    void Enable(bool bEnable);
    void HideToolTip();

    void GetRange(REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop) const;
    void SetRange(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
    REFERENCE_TIME GetPos() const;
    void SetPos(REFERENCE_TIME rtPos);
    bool HasDuration() const;

    void SetChapterBag(IDSMChapterBag* pCB);
    void RemoveChapters();

    bool DraggingThumb();

private:
    DECLARE_MESSAGE_MAP()

    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMouseLeave();
    afx_msg LRESULT OnThemeChanged();
    afx_msg void OnCaptureChanged(CWnd* pWnd);
};
