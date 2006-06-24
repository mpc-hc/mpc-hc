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

#include <atlcoll.h>
#include "StatusLabel.h"

// CPlayerInfoBar

class CPlayerInfoBar : public CDialogBar
{
	DECLARE_DYNAMIC(CPlayerInfoBar)

private:
	CAutoPtrArray<CStatusLabel> m_label;
	CAutoPtrArray<CStatusLabel> m_info;

	int m_nFirstColWidth;

	void Relayout();

public:
	CPlayerInfoBar(int nFirstColWidth = 100);
	virtual ~CPlayerInfoBar();

	BOOL Create(CWnd* pParentWnd);

	void SetLine(CString label, CString info);
	void GetLine(CString label, CString& info);
	void RemoveLine(CString label);
	void RemoveAllLines();

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
