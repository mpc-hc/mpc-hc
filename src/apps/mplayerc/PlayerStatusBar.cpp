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
#include "PlayerStatusBar.h"
#include "MainFrm.h"
#include "../../DSUtil/DSUtil.h"


// CPlayerStatusBar

IMPLEMENT_DYNAMIC(CPlayerStatusBar, CDialogBar)

CPlayerStatusBar::CPlayerStatusBar()
	: m_status(false, false)
	, m_time(true, false)
	, m_bmid(0)
	, m_hIcon(0)
{
}

CPlayerStatusBar::~CPlayerStatusBar()
{
	if (m_hIcon) {
		DestroyIcon(m_hIcon);
	}
}

BOOL CPlayerStatusBar::Create(CWnd* pParentWnd)
{
	return CDialogBar::Create(pParentWnd, IDD_PLAYERSTATUSBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM, IDD_PLAYERSTATUSBAR);
}

BOOL CPlayerStatusBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CDialogBar::PreCreateWindow(cs)) {
		return FALSE;
	}

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;

	return TRUE;
}

int CPlayerStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	CRect r;
	r.SetRectEmpty();

	m_type.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_ICON,
				  r, this, IDC_STATIC1);

	m_status.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_OWNERDRAW,
					r, this, IDC_PLAYERSTATUS);

	m_time.Create(_T(""), WS_CHILD|WS_VISIBLE|SS_OWNERDRAW,
				  r, this, IDC_PLAYERTIME);

	m_status.SetWindowPos(&m_time, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	Relayout();

	return 0;
}

void CPlayerStatusBar::Relayout()
{
	BITMAP bm;
	memset(&bm, 0, sizeof(bm));
	if (m_bm.m_hObject) {
		m_bm.GetBitmap(&bm);
	}

	CRect r, r2;
	GetClientRect(r);

	r.DeflateRect(27, 5, bm.bmWidth + 8, 4);
	int div = r.right - (m_time.IsWindowVisible() ? 140 : 0);

	CString str;
	m_time.GetWindowText(str);
	if (CDC* pDC = m_time.GetDC()) {
		CFont* pOld = pDC->SelectObject(&m_time.GetFont());
		div = r.right - pDC->GetTextExtent(str).cx;
		pDC->SelectObject(pOld);
		m_time.ReleaseDC(pDC);
	}

	r2 = r;
	r2.right = div - 2;
	m_status.MoveWindow(&r2);

	r2 = r;
	r2.left = div;
	m_time.MoveWindow(&r2);

	GetClientRect(r);
	r.SetRect(6, r.top+4, 22, r.bottom-4);
	m_type.MoveWindow(r);

	Invalidate();
}

void CPlayerStatusBar::Clear()
{
	m_status.SetWindowText(_T(""));
	m_time.SetWindowText(_T(""));
	SetStatusBitmap(0);
	SetStatusTypeIcon(0);

	Relayout();
}

void CPlayerStatusBar::SetStatusBitmap(UINT id)
{
	if (m_bmid == id) {
		return;
	}

	if (m_bm.m_hObject) {
		m_bm.DeleteObject();
	}
	if (id) {
		m_bm.LoadBitmap(id);
	}
	m_bmid = id;

	Relayout();
}

void CPlayerStatusBar::SetStatusTypeIcon(HICON hIcon)
{
	if (m_hIcon == hIcon) {
		return;
	}

	if (m_hIcon) {
		DestroyIcon(m_hIcon);
	}
	m_type.SetIcon(m_hIcon = hIcon);

	Relayout();
}

void CPlayerStatusBar::SetStatusMessage(CString str)
{
	str.Trim();
	m_status.SetWindowText(str);
}

CString CPlayerStatusBar::GetStatusTimer()
{
	CString		strResult;

	m_time.GetWindowText(strResult);

	return strResult;
}

void CPlayerStatusBar::SetStatusTimer(CString str)
{
	CString tmp;
	m_time.GetWindowText(tmp);
	if (tmp == str) {
		return;
	}

	str.Trim();
	m_time.SetWindowText(str);

	Relayout();
}

void CPlayerStatusBar::SetStatusTimer(REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, bool fHighPrecision, const GUID* pTimeFormat)
{
	ASSERT(pTimeFormat);
	ASSERT(rtNow <= rtDur);

	CString str;
	CString posstr, durstr;

	if (*pTimeFormat == TIME_FORMAT_MEDIA_TIME) {
		DVD_HMSF_TIMECODE tcNow = RT2HMSF(rtNow+4990000);
		DVD_HMSF_TIMECODE tcDur = RT2HMSF(rtDur+4990000);

		if (tcDur.bHours > 0 || (rtNow >= rtDur && tcNow.bHours > 0)) {
			posstr.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
		} else {
			posstr.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);
		}

		if (tcDur.bHours > 0) {
			durstr.Format(_T("%02d:%02d:%02d"), tcDur.bHours, tcDur.bMinutes, tcDur.bSeconds);
		} else {
			durstr.Format(_T("%02d:%02d"), tcDur.bMinutes, tcDur.bSeconds);
		}

		if (fHighPrecision) {
			str.Format(_T("%s.%03d"), posstr, (rtNow/10000)%1000);
			posstr = str;
			str.Format(_T("%s.%03d"), durstr, (rtDur/10000)%1000);
			durstr = str;
			str.Empty();
		}
	} else if (*pTimeFormat == TIME_FORMAT_FRAME) {
		posstr.Format(_T("%I64d"), rtNow);
		durstr.Format(_T("%I64d"), rtDur);
	}

	str = (/*start <= 0 &&*/ rtDur <= 0) ? posstr : posstr + _T(" / ") + durstr;

	SetStatusTimer(str);
}

void CPlayerStatusBar::ShowTimer(bool fShow)
{
	m_time.ShowWindow(fShow ? SW_SHOW : SW_HIDE);

	Relayout();
}

BEGIN_MESSAGE_MAP(CPlayerStatusBar, CDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPlayerStatusBar message handlers


BOOL CPlayerStatusBar::OnEraseBkgnd(CDC* pDC)
{
	for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow()) {
		if (!pChild->IsWindowVisible()) {
			continue;
		}

		CRect r;
		pChild->GetClientRect(&r);
		pChild->MapWindowPoints(this, &r);
		pDC->ExcludeClipRect(&r);
	}

	CRect r;
	GetClientRect(&r);

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	if (pFrame->m_pLastBar != this || pFrame->m_fFullScreen) {
		r.InflateRect(0, 0, 0, 1);
	}

	if (pFrame->m_fFullScreen) {
		r.InflateRect(1, 0, 1, 0);
	}

	pDC->Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

	r.DeflateRect(1, 1);

	pDC->FillSolidRect(&r, 0);

	return TRUE;
}

void CPlayerStatusBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect r;

	if (m_bm.m_hObject) {
		BITMAP bm;
		m_bm.GetBitmap(&bm);
		CDC memdc;
		memdc.CreateCompatibleDC(&dc);
		memdc.SelectObject(&m_bm);
		GetClientRect(&r);
		dc.BitBlt(r.right-bm.bmWidth-1, (r.Height() - bm.bmHeight)/2, bm.bmWidth, bm.bmHeight, &memdc, 0, 0, SRCCOPY);

		//
	}
	/*
		if (m_hIcon)
		{
			GetClientRect(&r);
			r.SetRect(6, r.top+4, 22-1, r.bottom-4-1);
			DrawIconEx(dc, r.left, r.top, m_hIcon, r.Width(), r.Height(), 0, NULL, DI_NORMAL|DI_COMPAT);
		}
	*/
	// Do not call CDialogBar::OnPaint() for painting messages
}

void CPlayerStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	Relayout();
}

void CPlayerStatusBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	pFrame->GetWindowPlacement(&wp);

	if (!pFrame->m_fFullScreen && wp.showCmd != SW_SHOWMAXIMIZED) {
		CRect r;
		GetClientRect(r);
		CPoint p = point;

		MapWindowPoints(pFrame, &point, 1);
		pFrame->PostMessage(WM_NCLBUTTONDOWN,
							//			(p.x+p.y >= r.Width()) ? HTBOTTOMRIGHT : HTCAPTION,
							(p.x >= r.Width()-r.Height() && !pFrame->IsCaptionHidden()) ? HTBOTTOMRIGHT :
							HTCAPTION,
							MAKELPARAM(point.x, point.y));
	}
}

BOOL CPlayerStatusBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	pFrame->GetWindowPlacement(&wp);

	if (!pFrame->m_fFullScreen && wp.showCmd != SW_SHOWMAXIMIZED) {
		CRect r;
		GetClientRect(r);
		CPoint p;
		GetCursorPos(&p);
		ScreenToClient(&p);
		//		if (p.x+p.y >= r.Width())
		if (p.x >= r.Width()-r.Height() && !pFrame->IsCaptionHidden()) {
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			return TRUE;
		}
	}

	return CDialogBar::OnSetCursor(pWnd, nHitTest, message);
}

HBRUSH CPlayerStatusBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogBar::OnCtlColor(pDC, pWnd, nCtlColor);

	if (*pWnd == m_type) {
		hbr = GetStockBrush(BLACK_BRUSH);
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
