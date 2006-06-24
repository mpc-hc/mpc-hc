/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

// CPlayerSeekBar

class CPlayerSeekBar : public CDialogBar
{
	DECLARE_DYNAMIC(CPlayerSeekBar)

private:
	__int64 m_start, m_stop, m_pos, m_posreal;
	bool m_fEnabled;
	
	void MoveThumb(CPoint point);
	void SetPosInternal(__int64 pos);

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


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerSeekBar)
	virtual BOOL Create(CWnd* pParentWnd);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CPlayerSeekBar)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnPlayStop(UINT nID);
};
