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
 *  @brief Interface for the CResizableMinMax class.
 */

#if !defined(AFX_RESIZABLEMINMAX_H__INCLUDED_)
#define AFX_RESIZABLEMINMAX_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*! @addtogroup CoreComponents
 *  @{
 */

//! @brief brief_description
/*!
 *  long_description
 */
class CResizableMinMax
{
// Attributes
private:
	// flags
	BOOL m_bUseMaxTrack;
	BOOL m_bUseMinTrack;
	BOOL m_bUseMaxRect;

	POINT m_ptMinTrackSize;		// min tracking size
	POINT m_ptMaxTrackSize;		// max tracking size
	POINT m_ptMaxPos;			// maximized position
	POINT m_ptMaxSize;			// maximized size

public:
	CResizableMinMax();
	virtual ~CResizableMinMax();

protected:
	static void ApplyMinMaxTrackSize(LPMINMAXINFO lpMMI);

	void MinMaxInfo(LPMINMAXINFO lpMMI) const;
	static void ChainMinMaxInfo(LPMINMAXINFO lpMMI, CWnd* pParentFrame, const CWnd* pWnd);

	static void ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndChild, const CSize& sizeExtra);

	static void ChainMinMaxInfo(LPMINMAXINFO lpMMI, const CWnd* pParentWnd, UINT nID, const CSize& sizeExtra)
	{
		ChainMinMaxInfo(lpMMI,
			::GetDlgItem(pParentWnd->GetSafeHwnd(), nID), sizeExtra);
	}

	void ChainMinMaxInfoCB(LPMINMAXINFO lpMMI, HWND hWndChild);
	virtual BOOL CalcSizeExtra(HWND hWndChild, const CSize& sizeChild, CSize& sizeExtra);

	void ResetAllRects();

	void SetMaximizedRect(const CRect& rc);		// set window rect when maximized
	void ResetMaximizedRect();					// reset to default maximized rect
	void SetMinTrackSize(const CSize& size);	// set minimum tracking size
	void ResetMinTrackSize();					// reset to default minimum tracking size
	void SetMaxTrackSize(const CSize& size);	// set maximum tracking size
	void ResetMaxTrackSize();					// reset to default maximum tracking size
};

// @}
#endif // !defined(AFX_RESIZABLEMINMAX_H__INCLUDED_)
