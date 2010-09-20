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

#include "stdafx.h"
#include "mplayerc.h"
#include "ChildView.h"
#include "MainFrm.h"
#include "libpng.h"


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
									   ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

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
	bool bHaveLogo = false;

	CAutoLock cAutoLock(&m_csLogo);

	m_logo.Destroy();

	if(s.logoext)
		bHaveLogo = SUCCEEDED(m_logo.Load(s.logofn));

	if(!bHaveLogo)
	{
		s.logoext = 0; // use the built-in logo instead
		s.logofn = ""; // clear logo file name

		if (!m_logo.LoadFromResource(s.logoid)) // try the latest selected build-in logo
			m_logo.LoadFromResource(s.logoid=DEF_LOGO); // if fail then use the default logo, should never fail
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
	ON_COMMAND_EX(ID_PLAY_PLAYPAUSE, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_PLAY, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_PAUSE, OnPlayPlayPauseStop)
	ON_COMMAND_EX(ID_PLAY_STOP, OnPlayPlayPauseStop)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	//	ON_WM_NCHITTEST()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
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

BOOL CChildView::OnPlayPlayPauseStop(UINT nID)
{
	if(nID == ID_PLAY_STOP) SetVideoRect();
	CString osd = ResStr(nID);
	int i = osd.Find(_T("\n"));
	if(i > 0) osd.Delete(i, osd.GetLength()-i);

	CRect r1;
	((CMainFrame*)AfxGetMainWnd())->GetClientRect(&r1);
	if((!r1.Width()) || (!r1.Height())) return FALSE;

	if(!(((CMainFrame*)AfxGetMainWnd())->m_OpenFile))
		((CMainFrame*)AfxGetMainWnd())->m_OSD.DisplayMessage(OSD_TOPLEFT, osd, 1500);
	return FALSE;
}

BOOL CChildView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(((CMainFrame*)GetParentFrame())->m_fHideCursor)
	{
		SetCursor(NULL);
		return TRUE;
	}
	if(((CMainFrame*)GetParentFrame())->IsSomethingLoaded()) 
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
        return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CChildView::OnNcHitTest(CPoint point)
{
	UINT nHitTest = CWnd::OnNcHitTest(point);

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	bool fLeftMouseBtnUnassigned = true;
	AppSettings& s = AfxGetAppSettings();
	POSITION pos = s.wmcmds.GetHeadPosition();
	while(pos && fLeftMouseBtnUnassigned)
		if(s.wmcmds.GetNext(pos).mouse == wmcmd::LDOWN)
			fLeftMouseBtnUnassigned = false;
	if(!pFrame->m_fFullScreen && (pFrame->IsCaptionMenuHidden() || fLeftMouseBtnUnassigned))
	{
		CRect rcClient, rcFrame;
		GetWindowRect(&rcFrame);
		rcClient = rcFrame;

		CSize sizeBorder(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));

		rcClient.InflateRect(-(5 * sizeBorder.cx), -(5 * sizeBorder.cy));
		rcFrame.InflateRect(sizeBorder.cx, sizeBorder.cy);

		if(rcFrame.PtInRect(point))
		{
			if(point.x > rcClient.right)
			{
				if(point.y < rcClient.top)
				{
					nHitTest = HTTOPRIGHT;
				}
				else if(point.y > rcClient.bottom)
				{
					nHitTest = HTBOTTOMRIGHT;
				}
				else
				{
					nHitTest = HTRIGHT;
				}
			}
			else if(point.x < rcClient.left)
			{
				if(point.y < rcClient.top)
				{
					nHitTest = HTTOPLEFT;
				}
				else if(point.y > rcClient.bottom)
				{
					nHitTest = HTBOTTOMLEFT;
				}
				else
				{
					nHitTest = HTLEFT;
				}
			}
			else if(point.y < rcClient.top)
			{
				nHitTest = HTTOP;
			}
			else if(point.y > rcClient.bottom)
			{
				nHitTest = HTBOTTOM;
			}
		}
	}
	return nHitTest;
}

void CChildView::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	bool fLeftMouseBtnUnassigned = true;
	AppSettings& s = AfxGetAppSettings();
	POSITION pos = s.wmcmds.GetHeadPosition();
	while(pos && fLeftMouseBtnUnassigned)
		if(s.wmcmds.GetNext(pos).mouse == wmcmd::LDOWN)
			fLeftMouseBtnUnassigned = false;
	if(!pFrame->m_fFullScreen && (pFrame->IsCaptionMenuHidden() || fLeftMouseBtnUnassigned))
	{
		BYTE bFlag = 0;
		switch(nHitTest)
		{
		case HTTOP:
			bFlag = WMSZ_TOP;
			break;
		case HTTOPLEFT:
			bFlag = WMSZ_TOPLEFT;
			break;
		case HTTOPRIGHT:
			bFlag = WMSZ_TOPRIGHT;
			break;
		case HTLEFT:
			bFlag = WMSZ_LEFT;
			break;
		case HTRIGHT:
			bFlag = WMSZ_RIGHT;
			break;
		case HTBOTTOM:
			bFlag = WMSZ_BOTTOM;
			break;
		case HTBOTTOMLEFT:
			bFlag = WMSZ_BOTTOMLEFT;
			break;
		case HTBOTTOMRIGHT:
			bFlag = WMSZ_BOTTOMRIGHT;
			break;
		}
		if(bFlag)
		{
			pFrame->PostMessage(WM_SYSCOMMAND, (SC_SIZE | bFlag), (LPARAM)POINTTOPOINTS(point));
		}
		else
		{
			CWnd::OnNcLButtonDown(nHitTest, point);
		}
	}
}
