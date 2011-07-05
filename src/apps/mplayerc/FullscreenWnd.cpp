/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

#include "stdafx.h"
#include "mplayerc.h"
#include "FullscreenWnd.h"
#include "MainFrm.h"


// CFullscreenWnd

IMPLEMENT_DYNAMIC(CFullscreenWnd, CWnd)
CFullscreenWnd::CFullscreenWnd(CMainFrame* pMainFrame)
{
	m_pMainFrame		= pMainFrame;
	m_hCursor			= ::LoadCursor(NULL, IDC_HAND);
	m_bCursorVisible	= false;
}

CFullscreenWnd::~CFullscreenWnd()
{
}


BEGIN_MESSAGE_MAP(CFullscreenWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


LRESULT CFullscreenWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_COMMAND :
			m_pMainFrame->PostMessage(message, wParam, lParam);
			break;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

BOOL CFullscreenWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
		case WM_MOUSEMOVE :
		case WM_SYSKEYDOWN :
		case WM_SYSKEYUP :
		case WM_SYSCHAR :
		case WM_SYSCOMMAND :

		case WM_KEYDOWN :
		case WM_KEYUP :
		case WM_CHAR :

		case WM_LBUTTONDOWN :
		case WM_LBUTTONUP :
		case WM_LBUTTONDBLCLK :
		case WM_MBUTTONDOWN :
		case WM_MBUTTONUP :
		case WM_MBUTTONDBLCLK :
		case WM_RBUTTONDOWN :
		case WM_RBUTTONUP :
		case WM_RBUTTONDBLCLK :

		case WM_MOUSEWHEEL :

			m_pMainFrame->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}


BOOL CFullscreenWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, m_hCursor, HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

// CFullscreenWnd message handlers


BOOL CFullscreenWnd::OnEraseBkgnd(CDC* pDC)
{
	return false;
}

BOOL CFullscreenWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bCursorVisible) {
		::SetCursor(m_hCursor);
	} else {
		::SetCursor(NULL);
	}
	return FALSE;
}


void CFullscreenWnd::ShowCursor(bool bVisible)
{
	if (m_bCursorVisible != bVisible) {
		m_bCursorVisible = bVisible;
		PostMessage (WM_SETCURSOR,0,0);
	}
}

bool CFullscreenWnd::IsWindow()
{
	return (m_hWnd != NULL);
}
