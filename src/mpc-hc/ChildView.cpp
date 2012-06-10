/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "ChildView.h"
#include "MainFrm.h"

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
	if (!CWnd::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
									   ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST) {
		CWnd* pParent = GetParent();
		CPoint p(pMsg->lParam);
		::MapWindowPoints(pMsg->hwnd, pParent->m_hWnd, &p, 1);

		bool fDblClick = false;

		bool fInteractiveVideo = ((CMainFrame*)AfxGetMainWnd())->IsInteractiveVideo();
		/*
					if (fInteractiveVideo)
					{
						if (pMsg->message == WM_LBUTTONDOWN)
						{
							if ((pMsg->time - m_lastlmdowntime) <= GetDoubleClickTime()
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
						else if (pMsg->message == WM_LBUTTONDBLCLK)
						{
							m_lastlmdowntime = pMsg->time;
							m_lastlmdownpoint = pMsg->pt;
						}
					}
		*/
		if ((pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP || pMsg->message == WM_MOUSEMOVE)
				&& fInteractiveVideo) {
			if (pMsg->message == WM_MOUSEMOVE) {
				pParent->PostMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
			}

			if (fDblClick) {
				pParent->PostMessage(WM_LBUTTONDOWN, pMsg->wParam, MAKELPARAM(p.x, p.y));
				pParent->PostMessage(WM_LBUTTONDBLCLK, pMsg->wParam, MAKELPARAM(p.x, p.y));
			}
		} else {
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

	m_logo.Detach();

	if (s.fLogoExternal) {
		bHaveLogo = !!m_logo.LoadFromFile(s.strLogoFileName);
	}

	if (!bHaveLogo) {
		s.fLogoExternal = false; // use the built-in logo instead
		s.strLogoFileName = ""; // clear logo file name

		if (!m_logo.Load(s.nLogoId)) { // try the latest selected build-in logo
			m_logo.Load(s.nLogoId = DEF_LOGO);    // if fail then use the default logo, should and must never fail
		}
	}

	if (m_hWnd) {
		Invalidate();
	}
}

CSize CChildView::GetLogoSize() const
{
	return m_logo.GetBitmapDimension();
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

	CImage img;
	img.Attach(m_logo);

	if (((CMainFrame*)GetParentFrame())->IsSomethingLoaded()) {
		pDC->ExcludeClipRect(m_vrect);
	} else if (!img.IsNull()) {
		GetClientRect(r);
		int w = min(img.GetWidth(), r.Width());
		int h = min(img.GetHeight(), r.Height());
		int x = (r.Width() - w) / 2;
		int y = (r.Height() - h) / 2;
		r = CRect(CPoint(x, y), CSize(w, h));

		int oldmode = pDC->SetStretchBltMode(STRETCH_HALFTONE);
		img.StretchBlt(*pDC, r, CRect(0, 0, img.GetWidth(), img.GetHeight()));
		pDC->SetStretchBltMode(oldmode);
		pDC->ExcludeClipRect(r);
	}
	img.Detach();

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
	if (nID == ID_PLAY_STOP) {
		SetVideoRect();
	}
	CString osd = ResStr(nID);
	int i = osd.Find(_T("\n"));
	if (i > 0) {
		osd.Delete(i, osd.GetLength()-i);
	}

	CRect r1;
	((CMainFrame*)AfxGetMainWnd())->GetClientRect(&r1);
	if ((!r1.Width()) || (!r1.Height())) {
		return FALSE;
	}

	if (!(((CMainFrame*)AfxGetMainWnd())->m_OpenFile)) {
		((CMainFrame*)AfxGetMainWnd())->m_OSD.DisplayMessage(OSD_TOPLEFT, osd, 1500);
	}
	return FALSE;
}

BOOL CChildView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (((CMainFrame*)GetParentFrame())->m_fHideCursor) {
		SetCursor(NULL);
		return TRUE;
	}
	if (((CMainFrame*)GetParentFrame())->IsSomethingLoaded() && (nHitTest == HTCLIENT)) {
		if (((CMainFrame*)GetParentFrame())->GetPlaybackMode() == PM_DVD) {
			return FALSE;
		}
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CChildView::OnNcHitTest(CPoint point)
{
	LRESULT nHitTest = CWnd::OnNcHitTest(point);

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	bool fLeftMouseBtnUnassigned = !AssignedToCmd(wmcmd::LDOWN);
	if (!pFrame->m_fFullScreen && (pFrame->IsCaptionHidden() || fLeftMouseBtnUnassigned)) {
		CRect rcClient, rcFrame;
		GetWindowRect(&rcFrame);
		rcClient = rcFrame;

		CSize sizeBorder(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));

		rcClient.InflateRect(-(5 * sizeBorder.cx), -(5 * sizeBorder.cy));
		rcFrame.InflateRect(sizeBorder.cx, sizeBorder.cy);

		if (rcFrame.PtInRect(point)) {
			if (point.x > rcClient.right) {
				if (point.y < rcClient.top) {
					nHitTest = HTTOPRIGHT;
				} else if (point.y > rcClient.bottom) {
					nHitTest = HTBOTTOMRIGHT;
				} else {
					nHitTest = HTRIGHT;
				}
			} else if (point.x < rcClient.left) {
				if (point.y < rcClient.top) {
					nHitTest = HTTOPLEFT;
				} else if (point.y > rcClient.bottom) {
					nHitTest = HTBOTTOMLEFT;
				} else {
					nHitTest = HTLEFT;
				}
			} else if (point.y < rcClient.top) {
				nHitTest = HTTOP;
			} else if (point.y > rcClient.bottom) {
				nHitTest = HTBOTTOM;
			}
		}
	}
	return nHitTest;
}

void CChildView::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	bool fLeftMouseBtnUnassigned = !AssignedToCmd(wmcmd::LDOWN);
	if (!pFrame->m_fFullScreen && (pFrame->IsCaptionHidden() || fLeftMouseBtnUnassigned)) {
		BYTE bFlag = 0;
		switch (nHitTest) {
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
		if (bFlag) {
			pFrame->PostMessage(WM_SYSCOMMAND, (SC_SIZE | bFlag), (LPARAM)POINTTOPOINTS(point));
		} else {
			CWnd::OnNcLButtonDown(nHitTest, point);
		}
	}
}
