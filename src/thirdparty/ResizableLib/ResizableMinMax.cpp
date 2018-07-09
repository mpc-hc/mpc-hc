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
 *  @brief Implementation of the CResizableMinMax class.
 */

#include "stdafx.h"
#include "ResizableMinMax.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableMinMax::CResizableMinMax()
	: m_ptMinTrackSize(), m_ptMaxTrackSize(), m_ptMaxPos(), m_ptMaxSize()
{
	m_bUseMinTrack = FALSE;
	m_bUseMaxTrack = FALSE;
	m_bUseMaxRect = FALSE;
}

CResizableMinMax::~CResizableMinMax()
{

}

void CResizableMinMax::ApplyMinMaxTrackSize(LPMINMAXINFO lpMMI)
{
	lpMMI->ptMaxSize.x = __max(lpMMI->ptMaxSize.x, lpMMI->ptMinTrackSize.x);
	lpMMI->ptMaxSize.y = __max(lpMMI->ptMaxSize.y, lpMMI->ptMinTrackSize.y);

	lpMMI->ptMaxSize.x = __min(lpMMI->ptMaxSize.x, lpMMI->ptMaxTrackSize.x);
	lpMMI->ptMaxSize.y = __min(lpMMI->ptMaxSize.y, lpMMI->ptMaxTrackSize.y);
}

void CResizableMinMax::MinMaxInfo(LPMINMAXINFO lpMMI) const
{
	if (m_bUseMinTrack)
		lpMMI->ptMinTrackSize = m_ptMinTrackSize;

	if (m_bUseMaxTrack)
		lpMMI->ptMaxTrackSize = m_ptMaxTrackSize;

	if (m_bUseMaxRect)
	{
		lpMMI->ptMaxPosition = m_ptMaxPos;
		lpMMI->ptMaxSize = m_ptMaxSize;
	}

	ApplyMinMaxTrackSize(lpMMI);
}

void CResizableMinMax::ChainMinMaxInfo(LPMINMAXINFO lpMMI, CWnd* pParentFrame, const CWnd* pWnd)
{
	// get the extra size from child to parent
	CRect rectClient, rectWnd;
	if ((pParentFrame->GetStyle() & WS_CHILD) && pParentFrame->IsZoomed())
		pParentFrame->GetClientRect(rectWnd);
	else
		pParentFrame->GetWindowRect(rectWnd);
	pParentFrame->RepositionBars(0, 0xFFFF,
		AFX_IDW_PANE_FIRST, CWnd::reposQuery, rectClient);
	const CSize sizeExtra = rectWnd.Size() - rectClient.Size();

	ChainMinMaxInfo(lpMMI, pWnd->GetSafeHwnd(), sizeExtra);
}

void CResizableMinMax::ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndChild, const CSize& sizeExtra)
{
	// ask the child window for track size
	MINMAXINFO mmiChild = *lpMMI;
	::SendMessage(hWndChild, WM_GETMINMAXINFO, 0, (LPARAM)&mmiChild);
	BOOL bRetMax = (lpMMI->ptMaxTrackSize.x != mmiChild.ptMaxTrackSize.x
		|| lpMMI->ptMaxTrackSize.y != mmiChild.ptMaxTrackSize.y);
	BOOL bRetMin = (lpMMI->ptMinTrackSize.x != mmiChild.ptMinTrackSize.x
		|| lpMMI->ptMinTrackSize.y != mmiChild.ptMinTrackSize.y);

	// add static extra size
	mmiChild.ptMaxTrackSize = sizeExtra + mmiChild.ptMaxTrackSize;
	mmiChild.ptMinTrackSize = sizeExtra + mmiChild.ptMinTrackSize;

	// min size is the largest
	if (bRetMin)
	{
		lpMMI->ptMinTrackSize.x = __max(lpMMI->ptMinTrackSize.x,
			mmiChild.ptMinTrackSize.x);
		lpMMI->ptMinTrackSize.y = __max(lpMMI->ptMinTrackSize.y,
			mmiChild.ptMinTrackSize.y);
	}
	// max size is the shortest
	if (bRetMax)
	{
		lpMMI->ptMaxTrackSize.x = __min(lpMMI->ptMaxTrackSize.x,
			mmiChild.ptMaxTrackSize.x);
		lpMMI->ptMaxTrackSize.y = __min(lpMMI->ptMaxTrackSize.y,
			mmiChild.ptMaxTrackSize.y);
	}

	ApplyMinMaxTrackSize(lpMMI);
}

BOOL CResizableMinMax::CalcSizeExtra(HWND /*hWndChild*/, const CSize& /*sizeChild*/, CSize& /*sizeExtra*/)
{
	// should be overridden if you use ChainMinMaxInfoCB
	ASSERT(FALSE);
	return FALSE;
}

void CResizableMinMax::ChainMinMaxInfoCB(LPMINMAXINFO lpMMI, HWND hWndChild)
{
	// ask the child window for track size
	MINMAXINFO mmiChild = *lpMMI;
	::SendMessage(hWndChild, WM_GETMINMAXINFO, 0, (LPARAM)&mmiChild);
	BOOL bRetMax = (lpMMI->ptMaxTrackSize.x != mmiChild.ptMaxTrackSize.x
		|| lpMMI->ptMaxTrackSize.y != mmiChild.ptMaxTrackSize.y);
	BOOL bRetMin = (lpMMI->ptMinTrackSize.x != mmiChild.ptMinTrackSize.x
		|| lpMMI->ptMinTrackSize.y != mmiChild.ptMinTrackSize.y);

	// use a callback to determine extra size
	CSize sizeExtra(0, 0);
	bRetMax = bRetMax && CalcSizeExtra(hWndChild, mmiChild.ptMaxTrackSize, sizeExtra);
	mmiChild.ptMaxTrackSize = sizeExtra + mmiChild.ptMaxTrackSize;
	bRetMin = bRetMin && CalcSizeExtra(hWndChild, mmiChild.ptMinTrackSize, sizeExtra);
	mmiChild.ptMinTrackSize = sizeExtra + mmiChild.ptMinTrackSize;

	// min size is the largest
	if (bRetMin)
	{
		lpMMI->ptMinTrackSize.x = __max(lpMMI->ptMinTrackSize.x,
			mmiChild.ptMinTrackSize.x);
		lpMMI->ptMinTrackSize.y = __max(lpMMI->ptMinTrackSize.y,
			mmiChild.ptMinTrackSize.y);
	}
	// max size is the shortest
	if (bRetMax)
	{
		lpMMI->ptMaxTrackSize.x = __min(lpMMI->ptMaxTrackSize.x,
			mmiChild.ptMaxTrackSize.x);
		lpMMI->ptMaxTrackSize.y = __min(lpMMI->ptMaxTrackSize.y,
			mmiChild.ptMaxTrackSize.y);
	}

	ApplyMinMaxTrackSize(lpMMI);
}

void CResizableMinMax::ResetAllRects()
{
	m_bUseMinTrack = FALSE;
	m_bUseMaxTrack = FALSE;
	m_bUseMaxRect = FALSE;
}

void CResizableMinMax::SetMaximizedRect(const CRect& rc)
{
	m_bUseMaxRect = TRUE;

	m_ptMaxPos = rc.TopLeft();
	m_ptMaxSize.x = rc.Width();
	m_ptMaxSize.y = rc.Height();
}

void CResizableMinMax::ResetMaximizedRect()
{
	m_bUseMaxRect = FALSE;
}

void CResizableMinMax::SetMinTrackSize(const CSize& size)
{
	m_bUseMinTrack = TRUE;

	m_ptMinTrackSize.x = size.cx;
	m_ptMinTrackSize.y = size.cy;
}

void CResizableMinMax::ResetMinTrackSize()
{
	m_bUseMinTrack = FALSE;
}

void CResizableMinMax::SetMaxTrackSize(const CSize& size)
{
	m_bUseMaxTrack = TRUE;

	m_ptMaxTrackSize.x = size.cx;
	m_ptMaxTrackSize.y = size.cy;
}

void CResizableMinMax::ResetMaxTrackSize()
{
	m_bUseMaxTrack = FALSE;
}
