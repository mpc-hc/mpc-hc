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
 *  @brief Interface for the CResizableLayout class.
 */

#if !defined(AFX_RESIZABLELAYOUT_H__INCLUDED_)
#define AFX_RESIZABLELAYOUT_H__INCLUDED_

//#include <afxtempl.h>
#include "ResizableMsgSupport.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*! @addtogroup CoreComponents
 *  @{
 */

//! @brief Special type for layout alignment
/*!
 *  Implements anchor points as a percentage of the parent window client area.
 *  Control corners are always kept at a fixed distance from their anchor points,
 *  thus allowing a control to resize proportionally as the parent window is resized.
 */
typedef struct tagANCHOR
{
	int cx; //!< horizontal component, in percent
	int cy; //!< vertical component, in percent

	tagANCHOR() : cx(0), cy(0) {}

	tagANCHOR(int x, int y)
	{
		cx = x;
		cy = y;
	}

} ANCHOR, *PANCHOR, *LPANCHOR;

/*! @defgroup ConstAnchors Alignment Constants
 *  Define common layout alignment constants for anchor points.
 *  @{
 */
	//! Anchor to the top-left corner
	const ANCHOR TOP_LEFT(0, 0);
	//! Anchor to the top edge and center horizontally
	const ANCHOR TOP_CENTER(50, 0);
	//! Anchor to the top-right corner
	const ANCHOR TOP_RIGHT(100, 0);
	//! Anchor to the left edge and center vertically
	const ANCHOR MIDDLE_LEFT(0, 50);
	//! Anchor to the center
	const ANCHOR MIDDLE_CENTER(50, 50);
	//! Anchor to the right edge and center vertically
	const ANCHOR MIDDLE_RIGHT(100, 50);
	//! Anchor to the bottom-left corner
	const ANCHOR BOTTOM_LEFT(0, 100);
	//! Anchor to the bottom edge and center horizontally
	const ANCHOR BOTTOM_CENTER(50, 100);
	//! Anchor to the bottom-right corner
	const ANCHOR BOTTOM_RIGHT(100, 100);
// @}

//! @brief Holds a control layout settings
/*!
 *  Layout settings specify how a control must be moved and resized with respect to
 *  the parent window and how it reacts to dynamic changes to its size when painting
 *  its client area, with special care for flickering.
 */
typedef struct tagLAYOUTINFO
{
	//! Handle of the window the layout of which is being defined
	HWND hWnd;
	//! Identification number assigned to the callback slot
	LRESULT nCallbackID;

	//! Window class name to identify standard controls
	TCHAR sWndClass[MAX_PATH];

	//! Anchor point for the top-left corner
	ANCHOR anchorTopLeft;
	//! Fixed distance for the top-left corner
	SIZE marginTopLeft;

	//! Anchor point for the bottom-right corner
	ANCHOR anchorBottomRight;
	//! Fixed distance for the bottom-right corner
	SIZE marginBottomRight;

	//! Flag that enables support for custom windows
	BOOL bMsgSupport;
	//! Redraw settings for anti-flickering and proper painting
	RESIZEPROPERTIES properties;

	tagLAYOUTINFO() : hWnd(NULL), nCallbackID(0)
		, marginTopLeft(), marginBottomRight(), bMsgSupport(FALSE)
	{
		sWndClass[0] = 0;
	}

	tagLAYOUTINFO(HWND hwnd, ANCHOR tl_type, SIZE tl_margin,
		ANCHOR br_type, SIZE br_margin)
		:
		hWnd(hwnd), nCallbackID(0),
		anchorTopLeft(tl_type), marginTopLeft(tl_margin),
		anchorBottomRight(br_type), marginBottomRight(br_margin), bMsgSupport(FALSE)
	{
		sWndClass[0] = 0;
	}

} LAYOUTINFO, *PLAYOUTINFO, *LPLAYOUTINFO;

//! @brief Layout manager implementation
/*!
 *  Derive from this class to implement resizable windows, adding the ability
 *  to dinamically resize and reposition child controls.
 *	Special care is taken to ensure a smooth animation during the resize
 *  operations performed by the users, without annoying flickering effects.
 */
class CResizableLayout
{
private:
	//@{
	//! @brief Collection of layout settings for each control
	CMap<HWND, HWND, POSITION, POSITION> m_mapLayout;
	CList<LAYOUTINFO, LAYOUTINFO&> m_listLayout;
	CList<LAYOUTINFO, LAYOUTINFO&> m_listLayoutCB;
	//@}

	//@{
	//! @brief Used for clipping implementation
	HRGN m_hOldClipRgn;
	int m_nOldClipRgn;
	//@}

	//@{
	//! @brief Used for advanced anti-flickering
	RECT m_rectClientBefore;
	BOOL m_bNoRecursion;
	//@}

	//! @brief Apply clipping settings for the specified control
	void ClipChildWindow(const LAYOUTINFO &layout, CRgn* pRegion) const;

	//! @brief Helper function to calculate new layout
	void CalcNewChildPosition(const LAYOUTINFO &layout,
		const CRect &rectParent, CRect &rectChild, UINT *lpFlags) const;

protected:
	//! @brief Override to initialize resize properties (clipping, refresh)
	virtual void InitResizeProperties(LAYOUTINFO& layout) const;

	//! @brief Override to specify clipping for unsupported windows
	virtual BOOL LikesClipping(const LAYOUTINFO &layout) const;

	//! @brief Override to specify refresh for unsupported windows
	virtual BOOL NeedsRefresh(const LAYOUTINFO &layout,
		const CRect &rectOld, const CRect &rectNew) const;

	//! @brief Clip controls in the layout out of the specified device context
	BOOL ClipChildren(const CDC* pDC, BOOL bUndo);

	//! @brief Get the layout clipping region
	void GetClippingRegion(CRgn* pRegion) const;

	//! @brief Override for scrollable or expanding parent windows
	virtual void GetTotalClientRect(LPRECT lpRect) const;

	//@{
	//! @brief Add anchor points for the specified control to the layout
	void AddAnchor(HWND hWnd, ANCHOR anchorTopLeft, ANCHOR anchorBottomRight);

	void AddAnchor(HWND hWnd, ANCHOR anchorTopLeft)
	{
		AddAnchor(hWnd, anchorTopLeft, anchorTopLeft);
	}

	void AddAnchor(UINT nID, ANCHOR anchorTopLeft, ANCHOR anchorBottomRight)
	{
		AddAnchor(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			anchorTopLeft, anchorBottomRight);
	}

	void AddAnchor(UINT nID, ANCHOR anchorTopLeft)
	{
		AddAnchor(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			anchorTopLeft, anchorTopLeft);
	}
	//@}

	//@{
	//! @brief Add anchor points for all the remaining controls to the layout
	void AddAllOtherAnchors(ANCHOR anchorTopLeft, ANCHOR anchorBottomRight);
	
	void AddAllOtherAnchors(ANCHOR anchor)
	{
		AddAllOtherAnchors(anchor, anchor);
	}

	void AddAllOtherAnchors()
	{
		AddAllOtherAnchors(TOP_LEFT);
	}
	//@}

	//! @brief Add a callback slot to the layout for dynamic controls or anchor points
	LRESULT AddAnchorCallback();

	//@{
	//! @brief Get position and size of a control in the layout from the parent's client area
	BOOL GetAnchorPosition(HWND hWnd, const CRect &rectParent,
		CRect &rectChild, UINT* lpFlags = NULL) const
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			return FALSE;

		CalcNewChildPosition(m_listLayout.GetAt(pos), rectParent, rectChild, lpFlags);
		return TRUE;
	}

	BOOL GetAnchorPosition(UINT nID, const CRect &rectParent,
		CRect &rectChild, UINT* lpFlags = NULL) const
	{
		return GetAnchorPosition(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			rectParent, rectChild, lpFlags);
	}
	//@}

	//@{
	//! @brief Get margins surrounding a control in the layout with the given size
	BOOL GetAnchorMargins(HWND hWnd, const CSize &sizeChild, CRect &rectMargins) const;

	BOOL GetAnchorMargins(UINT nID, const CSize &sizeChild, CRect &rectMargins) const
	{
		return GetAnchorMargins(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			sizeChild, rectMargins);
	}
	//@}

	//@{
	//! @brief Remove a control from the layout
	BOOL RemoveAnchor(HWND hWnd)
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			return FALSE;

		m_listLayout.RemoveAt(pos);
		return m_mapLayout.RemoveKey(hWnd);
	}

	BOOL RemoveAnchor(UINT nID)
	{
		return RemoveAnchor(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID));
	}
	//@}

	//! @brief Reset the layout content
	void RemoveAllAnchors()
	{
		m_mapLayout.RemoveAll();
		m_listLayout.RemoveAll();
		m_listLayoutCB.RemoveAll();
	}

	//! @brief Reposition and size all the controls in the layout
	void ArrangeLayout() const;

	//! @brief Override to provide dynamic control's layout info
	virtual BOOL ArrangeLayoutCallback(LAYOUTINFO& layout) const;

	//! @brief Override to provide the parent window
	virtual CWnd* GetResizableWnd() const = 0;

	//! @brief Enhance anti-flickering
	void HandleNcCalcSize(BOOL bAfterDefault, LPNCCALCSIZE_PARAMS lpncsp, LRESULT& lResult);

	//! @brief Enable resizable style for top level parent windows
	void MakeResizable(LPCREATESTRUCT lpCreateStruct) const;

public:
	CResizableLayout()
		: m_rectClientBefore()
	{
		m_bNoRecursion = FALSE;
		m_hOldClipRgn = ::CreateRectRgn(0,0,0,0);
		m_nOldClipRgn = 0;
	}

	virtual ~CResizableLayout()
	{
		::DeleteObject(m_hOldClipRgn);
		// just for safety
		RemoveAllAnchors();
	}
};

// @}
#endif // !defined(AFX_RESIZABLELAYOUT_H__INCLUDED_)
