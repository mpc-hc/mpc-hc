/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2007 see AUTHORS
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
#include "ChildView.h"
#include "MainFrm.h"
#include "libpng.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView() : m_vrect(0,0,0,0)
{
	m_lastlmdowntime = 0;
	m_lastlmdownpoint.SetPoint(0, 0);

	LoadLogo();
}

CChildView::~CChildView()
{
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_HAND), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST)
	{
		CWnd* pParent = GetParent();
		CPoint p(pMsg->lParam);
		::MapWindowPoints(pMsg->hwnd, pParent->m_hWnd, &p, 1);

		bool fDblClick = false;

		bool fInteractiveVideo = ((CMainFrame*)AfxGetMainWnd())->IsInteractiveVideo();
/*
		if(fInteractiveVideo)
		{
			if(pMsg->message == WM_LBUTTONDOWN)
			{
				if((pMsg->time - m_lastlmdowntime) <= GetDoubleClickTime()
				&& abs(pMsg->pt.x - m_lastlmdownpoint.x) <= GetSystemMetrics(SM_CXDOUBLECLK)
				&& abs(pMsg->pt.y - m_lastlmdownpoint.y) <= GetSystemMetrics(SM_CYDOUBLECLK))
				{
					fDblClick = true;
					m_lastlmdowntime = 0;
					m_lastlmdownpoint.SetPoint(0, 0);
				}
				else
				{
					m_lastlmdowntime = pMsg->time;
					m_lastlmdownpoint = pMsg->pt;
				}
			}
			else if(pMsg->message == WM_LBUTTONDBLCLK)
			{
				m_lastlmdowntime = pMsg->time;
				m_lastlmdownpoint = pMsg->pt;
			}
		}
*/
		if((pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
		&& fInteractiveVideo)
		{
			if(pMsg->message == WM_MOUSEMOVE)
			{
				pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
			}

			if(fDblClick)
			{
				pParent->PostMessage(WM_LBUTTONDOWN, pMsg->wParam, MAKELPARAM(p.x, p.y));
				pParent->PostMessage(WM_LBUTTONDBLCLK, pMsg->wParam, MAKELPARAM(p.x, p.y));
			}
		}
		else
		{
			pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
            return TRUE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CChildView::SetVideoRect(CRect r)
{
	m_vrect = r;

	Invalidate();
}

void CChildView::LoadLogo()
{
	AppSettings& s = AfxGetAppSettings();

	CAutoLock cAutoLock(&m_csLogo);

	m_logo.Destroy();

	if(s.logoext)
	{
		if(AfxGetAppSettings().fXpOrBetter)
			m_logo.Load(s.logofn);
		else if(HANDLE h = LoadImage(NULL, s.logofn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
			m_logo.Attach((HBITMAP)h); // win9x bug: Inside Attach GetObject() will return all zeros in DIBSECTION and silly CImage uses that to init width, height, bpp, ... so we can't use CImage::Draw later
	}

	if(m_logo.IsNull())
	{
		m_logo.LoadFromResource(s.logoid);
		// m_logo.LoadFromResource(AfxGetInstanceHandle(), s.logoid);
	}

	if(m_hWnd) Invalidate();
}

CSize CChildView::GetLogoSize()
{
	CSize ret(0,0);
	if(!m_logo.IsNull())
		ret.SetSize(m_logo.GetWidth(), m_logo.GetHeight());
	return ret;
}

IMPLEMENT_DYNAMIC(CChildView, CWnd)

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGED()
	ON_COMMAND_EX(ID_PLAY_PLAYPAUSE, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_PLAY, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_PAUSE, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_STOP, OnPlayPlayPauseStop)
	ON_WM_SETCURSOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	((CMainFrame*)GetParentFrame())->RepaintVideo();

	// Do not call CWnd::OnPaint() for painting messages
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	CRect r;

	CAutoLock cAutoLock(&m_csLogo);

	if(((CMainFrame*)GetParentFrame())->IsSomethingLoaded())
	{
		pDC->ExcludeClipRect(m_vrect);
	}
	else if(!m_logo.IsNull() /*&& ((CMainFrame*)GetParentFrame())->IsPlaylistEmpty()*/)
	{
		BITMAP bm;
		GetObject(m_logo, sizeof(bm), &bm);

		GetClientRect(r);
		int w = min(bm.bmWidth, r.Width());
		int h = min(abs(bm.bmHeight), r.Height());
//		int w = min(m_logo.GetWidth(), r.Width());
//		int h = min(m_logo.GetHeight(), r.Height());
		int x = (r.Width() - w) / 2;
		int y = (r.Height() - h) / 2;
		r = CRect(CPoint(x, y), CSize(w, h));

		int oldmode = pDC->SetStretchBltMode(STRETCH_HALFTONE);
		m_logo.StretchBlt(*pDC, r, CRect(0,0,bm.bmWidth,abs(bm.bmHeight)));
//		m_logo.Draw(*pDC, r);
		pDC->SetStretchBltMode(oldmode);

		pDC->ExcludeClipRect(r);
	}

	GetClientRect(r);
	pDC->FillSolidRect(r, 0);

	return TRUE;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	((CMainFrame*)GetParentFrame())->MoveVideoWindow();
}

void CChildView::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);

	((CMainFrame*)GetParentFrame())->MoveVideoWindow();
}

BOOL CChildView::OnPlayPlayPauseStop(UINT nID)
{
	if(nID == ID_PLAY_STOP) SetVideoRect();
	return FALSE;
}

BOOL CChildView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(((CMainFrame*)GetParentFrame())->m_fHideCursor)
	{
		SetCursor(NULL);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CChildView::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	if(!((CMainFrame*)GetParentFrame())->IsFrameLessWindow()) 
	{
		InflateRect(&lpncsp->rgrc[0], -1, -1);
	}

	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CChildView::OnNcPaint()
{
	if(!((CMainFrame*)GetParentFrame())->IsFrameLessWindow()) 
	{
		CRect r, c;
		GetWindowRect(r);
		r.OffsetRect(-r.left, -r.top);
		c = r;
		c.DeflateRect (1,1);

		CWindowDC		dc(this);
		dc.ExcludeClipRect(c);		// Casimir666 : prevent flashing when resizing
		dc.Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT)); 
	}
}
