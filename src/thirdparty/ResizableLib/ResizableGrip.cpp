/////////////////////////////////////////////////////////////////////////////
//
// This file is part of ResizableLib
// https://github.com/ppescher/resizablelib
//
// Copyright (C) 2000-2015 by Paolo Messina
// mailto:ppescher@hotmail.com
//
// The contents of this file are subject to the Artistic License 2.0
// http://opensource.org/licenses/Artistic-2.0
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

/*!
 *  @file
 *  @brief Implementation of the CResizableGrip class.
 */

#include "stdafx.h"
#include "ResizableGrip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableGrip::CResizableGrip()
{
	m_nShowCount = 0;
}

CResizableGrip::~CResizableGrip()
{

}

void CResizableGrip::UpdateSizeGrip()
{
	if (!::IsWindow(m_wndGrip.m_hWnd))
		return;

	// size-grip goes bottom right in the client area
	// (any right-to-left adjustment should go here)

	RECT rect;
	GetResizableWnd()->GetClientRect(&rect);

	rect.left = rect.right - m_wndGrip.m_size.cx;
	rect.top = rect.bottom - m_wndGrip.m_size.cy;

	// must stay below other children
	m_wndGrip.SetWindowPos(&CWnd::wndBottom, rect.left, rect.top, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION
		| (IsSizeGripVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}

// pStatus points to a variable, maintained by the caller, that
// holds its visibility status. Initialize the variable with 1
// to allow to temporarily hide the grip, 0 to allow to
// temporarily show the grip (with respect to the dwMask bit).

// NB: visibility is effective only after an update

void CResizableGrip::ShowSizeGrip(DWORD* pStatus, DWORD dwMask /*= 1*/)
{
	ASSERT(pStatus != NULL);

	if (!(*pStatus & dwMask))
	{
		m_nShowCount++;
		(*pStatus) |= dwMask;
	}
}

void CResizableGrip::HideSizeGrip(DWORD* pStatus, DWORD dwMask /*= 1*/)
{
	ASSERT(pStatus != NULL);

	if (*pStatus & dwMask)
	{
		m_nShowCount--;
		(*pStatus) &= ~dwMask;
	}
}

BOOL CResizableGrip::IsSizeGripVisible() const
{
	// NB: visibility is effective only after an update
	return (m_nShowCount > 0);
}

void CResizableGrip::SetSizeGripVisibility(BOOL bVisible)
{
	if (bVisible)
		m_nShowCount = 1;
	else
		m_nShowCount = 0;
}

BOOL CResizableGrip::SetSizeGripBkMode(int nBkMode)
{
	if (::IsWindow(m_wndGrip.m_hWnd))
	{
		if (nBkMode == OPAQUE)
			m_wndGrip.SetTransparency(FALSE);
		else if (nBkMode == TRANSPARENT)
			m_wndGrip.SetTransparency(TRUE);
		else
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

void CResizableGrip::SetSizeGripShape(BOOL bTriangular)
{
	if (::IsWindow(m_wndGrip.m_hWnd))
		m_wndGrip.SetTriangularShape(bTriangular);
}

BOOL CResizableGrip::CreateSizeGrip(BOOL bVisible /*= TRUE*/,
		BOOL bTriangular /*= TRUE*/, BOOL bTransparent /*= FALSE*/)
{
	// create grip
	CRect rect(0 , 0, m_wndGrip.m_size.cx, m_wndGrip.m_size.cy);
	BOOL bRet = m_wndGrip.Create(WS_CHILD | WS_CLIPSIBLINGS
		| SBS_SIZEGRIP, rect, GetResizableWnd(), 0);

	if (bRet)
	{
		// set options
		m_wndGrip.SetTriangularShape(bTriangular);
		m_wndGrip.SetTransparency(bTransparent);
		SetSizeGripVisibility(bVisible);

		// update position
		UpdateSizeGrip();
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CSizeGrip implementation

BOOL CResizableGrip::CSizeGrip::IsRTL()
{
	return GetExStyle() & WS_EX_LAYOUTRTL;
}

BOOL CResizableGrip::CSizeGrip::PreCreateWindow(CREATESTRUCT& cs)
{
	// set window size
	m_size.cx = GetSystemMetrics(SM_CXVSCROLL);
	m_size.cy = GetSystemMetrics(SM_CYHSCROLL);

	cs.cx = m_size.cx;
	cs.cy = m_size.cy;

	return CScrollBar::PreCreateWindow(cs);
}

LRESULT CResizableGrip::CSizeGrip::WindowProc(UINT message,
											  WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_GETDLGCODE:
		// fix to prevent the control to gain focus, using arrow keys
		// (standard grip returns DLGC_WANTARROWS, like any standard scrollbar)
		return DLGC_STATIC;

	case WM_SETFOCUS:
		// fix to prevent the control to gain focus, if set directly
		// (for example when it's the only one control in a dialog)
		return 0;

	case WM_NCHITTEST:
		// choose proper cursor shape
		return IsRTL() ? HTBOTTOMLEFT : HTBOTTOMRIGHT;

	case WM_SETTINGCHANGE:
		{
			// update grip's size
			CSize sizeOld = m_size;
			m_size.cx = GetSystemMetrics(SM_CXVSCROLL);
			m_size.cy = GetSystemMetrics(SM_CYHSCROLL);

			// resize transparency bitmaps
			if (m_bTransparent)
			{
				CClientDC dc(this);

				// destroy bitmaps
				m_bmGrip.DeleteObject();
				m_bmMask.DeleteObject();

				// re-create bitmaps
				m_bmGrip.CreateCompatibleBitmap(&dc, m_size.cx, m_size.cy);
				m_bmMask.CreateBitmap(m_size.cx, m_size.cy, 1, 1, NULL);
			}

			// re-calc shape
			if (m_bTriangular)
				SetTriangularShape(m_bTriangular);

			// reposition the grip
			CRect rect;
			GetWindowRect(rect);
			rect.InflateRect(m_size.cx - sizeOld.cx, m_size.cy - sizeOld.cy, 0, 0);
			::MapWindowPoints(NULL, GetParent()->GetSafeHwnd(), (LPPOINT)&rect, 2);
			MoveWindow(rect, TRUE);
		}
		break;

	case WM_DESTROY:
		// perform clean up
		if (m_bTransparent)
			SetTransparency(FALSE);
		break;

	case WM_PAINT:
	case WM_PRINTCLIENT:
		if (m_bTransparent)
		{
			PAINTSTRUCT ps;
			CDC* pDC = (message == WM_PAINT && wParam == 0) ?
				BeginPaint(&ps) : CDC::FromHandle((HDC)wParam);

			// select bitmaps
			CBitmap *pOldGrip, *pOldMask;

			pOldGrip = m_dcGrip.SelectObject(&m_bmGrip);
			pOldMask = m_dcMask.SelectObject(&m_bmMask);

			// obtain original grip bitmap, make the mask and prepare masked bitmap
			CScrollBar::WindowProc(message, (WPARAM)m_dcGrip.GetSafeHdc(), lParam);
			m_dcGrip.SetBkColor(m_dcGrip.GetPixel(0, 0));
			m_dcMask.BitBlt(0, 0, m_size.cx, m_size.cy, &m_dcGrip, 0, 0, SRCCOPY);
			m_dcGrip.BitBlt(0, 0, m_size.cx, m_size.cy, &m_dcMask, 0, 0, 0x00220326);

			// draw transparently
			pDC->BitBlt(0, 0, m_size.cx, m_size.cy, &m_dcMask, 0, 0, SRCAND);
			pDC->BitBlt(0, 0, m_size.cx, m_size.cy, &m_dcGrip, 0, 0, SRCPAINT);

			// unselect bitmaps
			m_dcGrip.SelectObject(pOldGrip);
			m_dcMask.SelectObject(pOldMask);

			if (message == WM_PAINT && wParam == 0)
				EndPaint(&ps);
			return 0;
		}
	}

	return CScrollBar::WindowProc(message, wParam, lParam);
}

void CResizableGrip::CSizeGrip::SetTransparency(BOOL bActivate)
{
	// creates or deletes DCs and Bitmaps used for
	// implementing a transparent size grip

	if (bActivate && !m_bTransparent)
	{
		m_bTransparent = TRUE;

		CClientDC dc(this);

		// create memory DCs and bitmaps
		m_dcGrip.CreateCompatibleDC(&dc);
		m_bmGrip.CreateCompatibleBitmap(&dc, m_size.cx, m_size.cy);

		m_dcMask.CreateCompatibleDC(&dc);
		m_bmMask.CreateBitmap(m_size.cx, m_size.cy, 1, 1, NULL);
	}
	else if (!bActivate && m_bTransparent)
	{
		m_bTransparent = FALSE;

		// destroy memory DCs and bitmaps
		m_dcGrip.DeleteDC();
		m_bmGrip.DeleteObject();

		m_dcMask.DeleteDC();
		m_bmMask.DeleteObject();
	}
}

void CResizableGrip::CSizeGrip::SetTriangularShape(BOOL bEnable)
{
	m_bTriangular = bEnable;

	if (bEnable)
	{
		// set a triangular window region
		CRect rect;
		GetWindowRect(rect);
		rect.OffsetRect(-rect.TopLeft());
		POINT arrPoints[] =
		{
			{ rect.left, rect.bottom },
			{ rect.right, rect.bottom },
			{ rect.right, rect.top }
		};
		CRgn rgnGrip;
		rgnGrip.CreatePolygonRgn(arrPoints, 3, WINDING);
		SetWindowRgn((HRGN)rgnGrip.Detach(), IsWindowVisible());
	}
	else
	{
		SetWindowRgn((HRGN)NULL, IsWindowVisible());
	}
}