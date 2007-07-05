/* 
 *	Copyright (C) 2007 Casimir666
 *	http://tibrium.neuf.fr
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


class CMainFrame;

class CFullscreenWnd : public CWnd
{
	DECLARE_DYNAMIC(CFullscreenWnd)

public:
	CFullscreenWnd(CMainFrame* pMainFrame);
	virtual ~CFullscreenWnd();

	void	ShowCursor(bool bVisible);
	bool	IsWindow();

protected:
	DECLARE_MESSAGE_MAP()
	
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CMainFrame*			m_pMainFrame;
	HCURSOR				m_hCursor;
	bool				m_bCursorVisible;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};


