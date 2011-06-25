/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#define SHOW_DELAY 100
#define AUTOPOP_DELAY 1000

// CPlayerSeekBar

class CPlayerSeekBar : public CDialogBar
{
	DECLARE_DYNAMIC(CPlayerSeekBar)

private:
	enum tooltip_state_t { TOOLTIP_HIDDEN, TOOLTIP_TRIGGERED, TOOLTIP_VISIBLE };

	__int64 m_start, m_stop, m_pos, m_posreal;
	bool m_fEnabled;
	CToolTipCtrl m_tooltip;
	TOOLINFO m_ti;
	tooltip_state_t m_tooltipState;
	__int64 m_tooltipPos, m_tooltipLastPos;
	CString m_tooltipText;
	UINT_PTR m_tooltipTimer;

	void MoveThumb(CPoint point);
	__int64 CalculatePosition(CPoint point);
	void SetPosInternal(__int64 pos);

	void UpdateTooltip(CPoint point);

	CRect GetChannelRect();
	CRect GetThumbRect();
	CRect GetInnerThumbRect();

public:
	CPlayerSeekBar();
	virtual ~CPlayerSeekBar();

	void Enable(bool fEnable);

	void GetRange(__int64& start, __int64& stop);
	void SetRange(__int64 start, __int64 stop);
	__int64 GetPos(), GetPosReal();
	void SetPos(__int64 pos);

	void HideToolTip();
	void UpdateToolTipPosition(CPoint& point);
	void UpdateToolTipText();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerSeekBar)
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerSeekBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnPlayStop(UINT nID);
};
