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
 *  @brief Implementation of the CResizableLayout class.
 */

#include "stdafx.h"
#include "ResizableLayout.h"
#include "ResizableVersion.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*!
 *  @internal Constant used to detect clipping and refresh properties
 *
 *  @note In August 2002 Platform SDK, some guy at MS thought it was time
 *        to add the missing symbol BS_TYPEMASK, but forgot its original
 *        meaning and so now he's telling us not to use that symbol because
 *        its value is likely to change in the future SDK releases, including
 *        all the BS_* style bits in the mask, not just the button's type
 *        as the symbol's name suggests.
 *     @n So now we're forced to define another symbol, great!
 */
#define _BS_TYPEMASK 0x0000000FL

/*!
 *  This function adds a new control to the layout manager and sets anchor
 *  points for its top-left and bottom-right corners.
 *
 *  @param hWnd Window handle to the control to be added
 *  @param anchorTopLeft Anchor point for the top-left corner
 *  @param anchorBottomRight Anchor point for the bottom-right corner
 *
 *  @remarks Overlapping controls, like group boxes and the controls inside,
 *           must be added from the outer controls to the inner ones, to let
 *           the clipping routines work correctly.
 *
 *  @sa AddAnchorCallback RemoveAnchor
 */
void CResizableLayout::AddAnchor(HWND hWnd, ANCHOR anchorTopLeft, ANCHOR anchorBottomRight)
{
	CWnd* pParent = GetResizableWnd();

	// child window must be valid
	ASSERT(::IsWindow(hWnd));
	// must be child of parent window
	ASSERT(::IsChild(pParent->GetSafeHwnd(), hWnd));

	// get parent window's rect
	CRect rectParent;
	GetTotalClientRect(&rectParent);
	// and child control's rect
	CRect rectChild;
	::GetWindowRect(hWnd, &rectChild);
	::MapWindowPoints(NULL, pParent->m_hWnd, (LPPOINT)&rectChild, 2);

	// adjust position, if client area has been scrolled
	rectChild.OffsetRect(-rectParent.TopLeft());

	// go calculate margins
	CSize marginTopLeft, marginBottomRight;

	// calculate margin for the top-left corner

	marginTopLeft.cx = rectChild.left - rectParent.Width() * anchorTopLeft.cx / 100;
	marginTopLeft.cy = rectChild.top - rectParent.Height() * anchorTopLeft.cy / 100;

	// calculate margin for the bottom-right corner

	marginBottomRight.cx = rectChild.right - rectParent.Width() * anchorBottomRight.cx / 100;
	marginBottomRight.cy = rectChild.bottom - rectParent.Height() * anchorBottomRight.cy / 100;

	// prepare the structure
	LAYOUTINFO layout(hWnd, anchorTopLeft, marginTopLeft,
		anchorBottomRight, marginBottomRight);

	// get control's window class
	GetClassName(hWnd, layout.sWndClass, MAX_PATH);

	// initialize resize properties (overridable)
	InitResizeProperties(layout);

	// must not be already there!
	// (this is probably due to a duplicate call to AddAnchor)
	POSITION pos;
	ASSERT(!m_mapLayout.Lookup(hWnd, pos));

	// add to the list and the map
	pos = m_listLayout.AddTail(layout);
	m_mapLayout.SetAt(hWnd, pos);
}

/*!
 *  This function adds all the controls not yet added to the layout manager
 *  and sets anchor points for its top-left and bottom-right corners.
 *
 *  @param anchorTopLeft Anchor point for the top-left corner
 *  @param anchorBottomRight Anchor point for the bottom-right corner
 *  @param anchor Anchor point for the top-left and bottom-right corner
 *
 *  @remarks Overlapping controls, like group boxes and the controls inside,
 *           may not be handled correctly. Use individual @ref AddAnchor calls
 *           to solve any issues that may arise with clipping.
 *
 *  @sa AddAnchor
 */
void CResizableLayout::AddAllOtherAnchors(ANCHOR anchorTopLeft, ANCHOR anchorBottomRight)
{
	HWND hParent = GetResizableWnd()->GetSafeHwnd();
	ASSERT(::IsWindow(hParent));

	HWND hWnd = ::GetWindow(hParent, GW_CHILD);
	while (hWnd != NULL)
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			AddAnchor(hWnd, anchorTopLeft, anchorBottomRight);

		hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
	}
}

/*!
 *  This function adds a placeholder to the layout manager, that will be
 *  dinamically set by a callback function whenever required.
 *
 *  @return The return value is an integer used to distinguish between
 *          different placeholders in the callback implementation.
 *
 *  @remarks You must override @ref ArrangeLayoutCallback to provide layout
 *           information.
 *
 *  @sa AddAnchor ArrangeLayoutCallback ArrangeLayout
 */
LRESULT CResizableLayout::AddAnchorCallback()
{
	// one callback control cannot rely upon another callback control's
	// size and/or position (they're updated all together at the end)
	// it can however use a non-callback control, calling GetAnchorPosition()

	// add to the list
	LAYOUTINFO layout;
	layout.nCallbackID = m_listLayoutCB.GetCount() + 1;
	m_listLayoutCB.AddTail(layout);
	return layout.nCallbackID;
}

/*!
 *  This function is called for each placeholder added to the layout manager
 *  and must be overridden to provide the necessary layout information.
 *
 *  @param layout Reference to a LAYOUTINFO structure to be filled with
 *         layout information for the specified placeholder.
 *         On input, nCallbackID is the identification number
 *         returned by AddAnchorCallback. On output, anchor points and
 *         the window handle must be set and valid.
 *
 *  @return The return value is @c TRUE if the layout information has been
 *          provided successfully, @c FALSE to skip this placeholder.
 *
 *  @remarks When implementing this function, unknown placeholders should be
 *           passed to the base class. Unhandled cases will fire an assertion
 *           in the debug version.
 *
 *  @sa AddAnchorCallback ArrangeLayout LAYOUTINFO
 */
BOOL CResizableLayout::ArrangeLayoutCallback(LAYOUTINFO& layout) const
{
	UNREFERENCED_PARAMETER(layout);

	ASSERT(FALSE); // must be overridden, if callback is used

	return FALSE; // no useful output data
}

/*!
 *  This function should be called in resizable window classes whenever the
 *  controls layout should be updated, usually after a resize operation.
 *
 *  @remarks All the controls added to the layout are moved and resized at
 *           once for performace reasons, so all the controls are in their
 *           old position when AddAnchorCallback is called.
 *           To know where a control will be placed use GetAnchorPosition.
 *
 *  @sa AddAnchor AddAnchorCallback ArrangeLayoutCallback GetAnchorPosition
 */
void CResizableLayout::ArrangeLayout() const
{
	// common vars
	UINT uFlags;
	CRect rectParent, rectChild;
	const INT_PTR count = m_listLayout.GetCount() + m_listLayoutCB.GetCount();

	if (count <= 0)
		return;

	// get parent window's rect
	GetTotalClientRect(&rectParent);

	// reposition child windows
	HDWP hdwp = ::BeginDeferWindowPos(static_cast<int>(count));

	for (POSITION pos = m_listLayout.GetHeadPosition(); pos != NULL;)
	{
		// get layout info
		const LAYOUTINFO layout = m_listLayout.GetNext(pos);

		// calculate new child's position, size and flags for SetWindowPos
		CalcNewChildPosition(layout, rectParent, rectChild, &uFlags);

		// only if size or position changed
		if ((uFlags & (SWP_NOMOVE|SWP_NOSIZE)) != (SWP_NOMOVE|SWP_NOSIZE))
		{
			hdwp = ::DeferWindowPos(hdwp, layout.hWnd, NULL, rectChild.left,
				rectChild.top, rectChild.Width(), rectChild.Height(), uFlags);
		}
	}

	// for callback items you may use GetAnchorPosition to know the
	// new position and size of a non-callback item after resizing

	for (POSITION pos = m_listLayoutCB.GetHeadPosition(); pos != NULL;)
	{
		// get layout info
		LAYOUTINFO layout = m_listLayoutCB.GetNext(pos);
		// request layout data
		if (!ArrangeLayoutCallback(layout))
			continue;

		// calculate new child's position, size and flags for SetWindowPos
		CalcNewChildPosition(layout, rectParent, rectChild, &uFlags);

		// only if size or position changed
		if ((uFlags & (SWP_NOMOVE|SWP_NOSIZE)) != (SWP_NOMOVE|SWP_NOSIZE))
		{
			hdwp = ::DeferWindowPos(hdwp, layout.hWnd, NULL, rectChild.left,
				rectChild.top, rectChild.Width(), rectChild.Height(), uFlags);
		}
	}

	// finally move all the windows at once
	::EndDeferWindowPos(hdwp);
}

/*!
 *  @internal This function adds or removes a control window region
 *  to or from the specified clipping region, according to its layout
 *  properties.
 */
void CResizableLayout::ClipChildWindow(const LAYOUTINFO& layout,
									   CRgn* pRegion) const
{
	// obtain window position
	CRect rect;
	::GetWindowRect(layout.hWnd, &rect);
#if (_WIN32_WINNT >= 0x0501)
	//! @todo decide when to clip client only or non-client too (themes?)
	//! (leave disabled meanwhile, until I find a good solution)
	//! @note wizard97 with watermark bitmap and themes won't look good!
	// if (real_WIN32_WINNT >= 0x501)
	//	::SendMessage(layout.hWnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rect);
#endif
	::MapWindowPoints(NULL, GetResizableWnd()->m_hWnd, (LPPOINT)&rect, 2);

	// use window region if any
	CRgn rgn;
	rgn.CreateRectRgn(0,0,0,0);
	switch (::GetWindowRgn(layout.hWnd, rgn))
	{
	case COMPLEXREGION:
	case SIMPLEREGION:
		rgn.OffsetRgn(rect.TopLeft());
		break;

	default:
		rgn.SetRectRgn(&rect);
	}

	// get the clipping property
	const BOOL bClipping = layout.properties.bAskClipping ?
		LikesClipping(layout) : layout.properties.bCachedLikesClipping;

	// modify region accordingly
	if (bClipping)
		pRegion->CombineRgn(pRegion, &rgn, RGN_DIFF);
	else
		pRegion->CombineRgn(pRegion, &rgn, RGN_OR);
}

/*!
 *  This function retrieves the clipping region for the current layout.
 *  It can be used to draw directly inside the region, without applying
 *  clipping as the ClipChildren function does.
 *
 *  @param pRegion Pointer to a CRegion object that holds the
 *         calculated clipping region upon return
 *
 *  @deprecated For anti-flickering ClipChildren should be preferred
 *              as it is more complete for platform compatibility.
 *              It will probably become a private function.
 */
void CResizableLayout::GetClippingRegion(CRgn* pRegion) const
{
	const CWnd* pWnd = GetResizableWnd();

	// System's default clipping area is screen's size,
	// not enough for max track size, for example:
	// if screen is 1024 x 768 and resizing border is 4 pixels,
	// maximized size is 1024+4*2=1032 x 768+4*2=776,
	// but max track size is 4 pixels bigger 1036 x 780 (don't ask me why!)
	// So, if you resize the window to maximum size, the last 4 pixels
	// are clipped out by the default clipping region, that gets created
	// as soon as you call clipping functions (my guess).

	// reset clipping region to the whole client area
	CRect rect;
	pWnd->GetClientRect(&rect);
	pRegion->CreateRectRgnIndirect(&rect);

	// clip only anchored controls

	for (POSITION pos = m_listLayout.GetHeadPosition(); pos != NULL;)
	{
		// get layout info
		LAYOUTINFO layout = m_listLayout.GetNext(pos);

		if (::IsWindowVisible(layout.hWnd))
			ClipChildWindow(layout, pRegion);
	}
	
	for (POSITION pos = m_listLayoutCB.GetHeadPosition(); pos != NULL;)
	{
		// get layout info
		LAYOUTINFO layout = m_listLayoutCB.GetNext(pos);
		// request data
		if (ArrangeLayoutCallback(layout) && ::IsWindowVisible(layout.hWnd))
			ClipChildWindow(layout, pRegion);
	}
//! @todo Has XP changed this??? It doesn't seem correct anymore!
/*
	// fix for RTL layouts (1 pixel of horz offset)
	if (pWnd->GetExStyle() & WS_EX_LAYOUTRTL)
		pRegion->OffsetRgn(-1,0);
*/
}

//! @internal @brief Implements GetAncestor(pWnd->GetSafeHwnd(), GA_ROOT)
inline CWnd* GetRootParentWnd(CWnd* pWnd)
{
	// GetAncestor API not present, emulate
	if (!(pWnd->GetStyle() & WS_CHILD))
		return NULL;
	while (pWnd->GetStyle() & WS_CHILD)
		pWnd = pWnd->GetParent();
	return pWnd;
}

/*!
 *  This function enables or restores clipping on the specified DC when
 *  appropriate. It should be called whenever drawing on the window client
 *  area to avoid flickering.
 *
 *  @param pDC Pointer to the target device context
 *  @param bUndo Flag that specifies wether to restore the clipping region
 *
 *  @return The return value is @c TRUE if the clipping region has been
 *          modified, @c FALSE otherwise
 *
 *  @remarks For anti-flickering to work, you should wrap your
 *           @c WM_ERASEBKGND message handler inside a pair of calls to
 *           this function, with the last parameter set to @c FALSE first
 *           and to @c TRUE at the end.
 */
BOOL CResizableLayout::ClipChildren(const CDC* pDC, BOOL bUndo)
{
#if (_WIN32_WINNT >= 0x0501 && !defined(RSZLIB_NO_XP_DOUBLE_BUFFER))
	// clipping not necessary when double-buffering enabled
	if (real_WIN32_WINNT >= 0x0501)
	{
		CWnd *pWnd = GetRootParentWnd(GetResizableWnd());
		if (pWnd == NULL)
			pWnd = GetResizableWnd();
		if (pWnd->GetExStyle() & WS_EX_COMPOSITED)
			return FALSE;
	}
#endif

	const HDC hDC = pDC->GetSafeHdc();
	const HWND hWnd = GetResizableWnd()->GetSafeHwnd();

	// Some controls (such as transparent toolbars and standard controls
	// with XP theme enabled) send a WM_ERASEBKGND msg to the parent
	// to draw themselves, in which case we must not enable clipping.

	// We check that the window associated with the DC is the
	// resizable window and not a child control.

	if (!bUndo)
	{
		if (hWnd != ::WindowFromDC(hDC))
			m_nOldClipRgn = -1; // invalid region
		else
		{
			// save old DC clipping region
			m_nOldClipRgn = ::GetClipRgn(hDC, m_hOldClipRgn);
			// clip out supported child windows
			CRgn rgnClip;
			GetClippingRegion(&rgnClip);
			::ExtSelectClipRgn(hDC, rgnClip, RGN_AND);
		}
		return (m_nOldClipRgn >= 0) ? TRUE : FALSE;
	}

	if (m_nOldClipRgn >= 0) // restore old clipping region, only if modified and valid
	{
		::SelectClipRgn(hDC, (m_nOldClipRgn>0 ? m_hOldClipRgn : NULL));
		return TRUE;
	}

	return FALSE;
}

/*!
 *  This function is used by this class, and should be used by derived
 *  classes too, in place of the standard GetClientRect. It can be useful
 *  for windows with scrollbars or expanding windows, to provide the true
 *  client area, including even those parts which are not visible.
 *
 *  @param lpRect Pointer to the RECT structure that holds the result
 *
 *  @remarks Override this function to provide the client area the class uses
 *           to perform layout calculations, both when adding controls and
 *           when rearranging the layout.
 *        @n The base implementation simply calls @c GetClientRect
 */
void CResizableLayout::GetTotalClientRect(LPRECT lpRect) const
{
	GetResizableWnd()->GetClientRect(lpRect);
}

/*!
 *  This function is used to determine if a control needs to be painted when
 *  it is moved or resized by the layout manager.
 *
 *  @param layout Reference to a @c LAYOUTINFO structure for the control
 *  @param rectOld Reference to a @c RECT structure that holds the control
 *         position and size before the layout update
 *  @param rectNew Reference to a @c RECT structure that holds the control
 *         position and size after the layout update
 *
 *  @return The return value is @c TRUE if the control should be freshly
 *          painted after a layout update, @c FALSE if not necessary.
 *
 *  @remarks The default implementation tries to identify windows that
 *           need refresh by their class name and window style.
 *        @n Override this function if you need a different behavior or if
 *           you have custom controls that fail to be identified.
 *
 *  @sa LikesClipping InitResizeProperties
 */
BOOL CResizableLayout::NeedsRefresh(const LAYOUTINFO& layout,
								const CRect& rectOld, const CRect& rectNew) const
{
	if (layout.bMsgSupport)
	{
		REFRESHPROPERTY refresh;
		refresh.rcOld = rectOld;
		refresh.rcNew = rectNew;
		if (Send_NeedsRefresh(layout.hWnd, &refresh))
			return refresh.bNeedsRefresh;
	}

	const int nDiffWidth = (rectNew.Width() - rectOld.Width());
	const int nDiffHeight = (rectNew.Height() - rectOld.Height());

	// is the same size?
	if (nDiffWidth == 0 && nDiffHeight == 0)
		return FALSE;

	// optimistic, no need to refresh
	BOOL bRefresh = FALSE;

	// window classes that need refresh when resized
	if (0 == lstrcmp(layout.sWndClass, WC_STATIC))
	{
		LONG_PTR style = ::GetWindowLongPtr(layout.hWnd, GWL_STYLE);

		switch (style & SS_TYPEMASK)
		{
		case SS_LEFT:
		case SS_CENTER:
		case SS_RIGHT:
			// word-wrapped text
			bRefresh = bRefresh || (nDiffWidth != 0);
			// vertically centered text
			if (style & SS_CENTERIMAGE)
				bRefresh = bRefresh || (nDiffHeight != 0);
			break;

		case SS_LEFTNOWORDWRAP:
			// text with ellipsis
			if (style & SS_ELLIPSISMASK)
				bRefresh = bRefresh || (nDiffWidth != 0);
			// vertically centered text
			if (style & SS_CENTERIMAGE)
				bRefresh = bRefresh || (nDiffHeight != 0);
			break;

		case SS_ENHMETAFILE:
		case SS_BITMAP:
		case SS_ICON:
			// images
		case SS_BLACKFRAME:
		case SS_GRAYFRAME:
		case SS_WHITEFRAME:
		case SS_ETCHEDFRAME:
			// and frames
			bRefresh = TRUE;
			break;
		}
		return bRefresh;
	}

	// window classes that don't redraw client area correctly
	// when the hor scroll pos changes due to a resizing
	const BOOL bHScroll = (0 == lstrcmp(layout.sWndClass, WC_LISTBOX));

	// fix for horizontally scrollable windows, if wider
	if (bHScroll && (nDiffWidth > 0))
	{
		// get max scroll position
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		if (::GetScrollInfo(layout.hWnd, SB_HORZ, &info) && info.nPage > 1) //fix for unsigned subtraction
		{
			// subtract the page size
			info.nMax -= info.nPage-1; //should not use __max() macro
		}

		// resizing will cause the text to scroll on the right
		// because the scrollbar is going beyond the right limit
		if ((info.nMax > 0) && (info.nPos + nDiffWidth > info.nMax))
		{
			// needs repainting, due to horiz scrolling
			bRefresh = TRUE;
		}
	}

	return bRefresh;
}

/*!
 *  This function is used to determine if a control can be safely clipped
 *  out of the parent window client area when it is repainted, usually
 *  after a resize operation.
 *
 *  @param layout Reference to a @c LAYOUTINFO structure for the control
 *
 *  @return The return value is @c TRUE if clipping is supported by the
 *          control, @c FALSE otherwise.
 *
 *  @remarks The default implementation tries to identify @a clippable
 *           windows by their class name and window style.
 *        @n Override this function if you need a different behavior or if
 *           you have custom controls that fail to be identified.
 *
 *  @sa NeedsRefresh InitResizeProperties
 */
BOOL CResizableLayout::LikesClipping(const LAYOUTINFO& layout) const
{
	if (layout.bMsgSupport)
	{
		CLIPPINGPROPERTY clipping;
		if (Send_LikesClipping(layout.hWnd, &clipping))
			return clipping.bLikesClipping;
	}

	LONG_PTR style = ::GetWindowLongPtr(layout.hWnd, GWL_STYLE);

	// skip windows that wants background repainted
	if (0 == lstrcmp(layout.sWndClass, WC_BUTTON))
	{
		CRect rect;
		switch (style & _BS_TYPEMASK)
		{
		case BS_GROUPBOX:
			return FALSE;

		case BS_OWNERDRAW:
			// ownerdraw buttons must return correct hittest code
			// to notify their transparency to the system and this library
			// or they could use the registered message (more reliable)
			::GetWindowRect(layout.hWnd, &rect);
			::SendMessage(layout.hWnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rect);
			if ( HTTRANSPARENT == ::SendMessage(layout.hWnd,
				WM_NCHITTEST, 0, MAKELPARAM(rect.left, rect.top)) )
				return FALSE;
			break;
		}
		return TRUE;
	}
	else if (0 == lstrcmp(layout.sWndClass, WC_STATIC))
	{
		switch (style & SS_TYPEMASK)
		{
		case SS_LEFT:
		case SS_CENTER:
		case SS_RIGHT:
		case SS_LEFTNOWORDWRAP:
			// text
		case SS_BLACKRECT:
		case SS_GRAYRECT:
		case SS_WHITERECT:
			// filled rects
		case SS_ETCHEDHORZ:
		case SS_ETCHEDVERT:
			// etched lines
		case SS_BITMAP:
			// bitmaps
			return TRUE;

		case SS_ICON:
		case SS_ENHMETAFILE:
			if (style & SS_CENTERIMAGE)
				return FALSE;
			return TRUE;

		default:
			return FALSE;
		}
	}

	// assume the others like clipping
	return TRUE;
}


/*!
 *  @internal This function calculates the new size and position of a
 *  control in the layout and flags for @c SetWindowPos
 */
void CResizableLayout::CalcNewChildPosition(const LAYOUTINFO& layout,
						const CRect &rectParent, CRect &rectChild, UINT *lpFlags) const
{
	const CWnd* pParent = GetResizableWnd();

	::GetWindowRect(layout.hWnd, &rectChild);
	::MapWindowPoints(NULL, pParent->m_hWnd, (LPPOINT)&rectChild, 2);

	CRect rectNew;

	// calculate new top-left corner
	rectNew.left = layout.marginTopLeft.cx + rectParent.Width() * layout.anchorTopLeft.cx / 100;
	rectNew.top = layout.marginTopLeft.cy + rectParent.Height() * layout.anchorTopLeft.cy / 100;

	// calculate new bottom-right corner
	rectNew.right = layout.marginBottomRight.cx + rectParent.Width() * layout.anchorBottomRight.cx / 100;
	rectNew.bottom = layout.marginBottomRight.cy + rectParent.Height() * layout.anchorBottomRight.cy / 100;

	// adjust position, if client area has been scrolled
	rectNew.OffsetRect(rectParent.TopLeft());

	// get the refresh property
	BOOL bRefresh = layout.properties.bAskRefresh ?
		NeedsRefresh(layout, rectChild, rectNew) : layout.properties.bCachedNeedsRefresh;

	// set flags
	if (lpFlags) {
		*lpFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREPOSITION;
		if (bRefresh)
			*lpFlags |= SWP_NOCOPYBITS;
		if (rectNew.TopLeft() == rectChild.TopLeft())
			*lpFlags |= SWP_NOMOVE;
		if (rectNew.Size() == rectChild.Size())
			*lpFlags |= SWP_NOSIZE;
	}

	// update rect
	rectChild = rectNew;
}

/*!
 *  This function calculates the top, left, bottom, right margins for a
 *  given size of the specified control.
 *
 *  @param hWnd Window handle to a control in the layout
 *  @param sizeChild Size of the control to use in calculations
 *  @param rectMargins Holds the calculated margins
 *
 *  @return The return value is @c TRUE if successful, @c FALSE otherwise
 *
 *  @remarks This function can be used to infer the parent window size
 *           from the size of one of its child controls.
 *           It is used to implement cascading of size constraints.
 */
BOOL CResizableLayout::GetAnchorMargins(HWND hWnd, const CSize &sizeChild, CRect &rectMargins) const
{
	POSITION pos;
	if (!m_mapLayout.Lookup(hWnd, pos))
		return FALSE;

	const LAYOUTINFO& layout = m_listLayout.GetAt(pos);

	// augmented size, relative to anchor points
	CSize size = sizeChild + layout.marginTopLeft - layout.marginBottomRight;

	// percent of parent size occupied by this control
	CSize percent(layout.anchorBottomRight.cx - layout.anchorTopLeft.cx,
		layout.anchorBottomRight.cy - layout.anchorTopLeft.cy);

	// calculate total margins
	rectMargins.left = size.cx * layout.anchorTopLeft.cx / percent.cx + layout.marginTopLeft.cx;
	rectMargins.top = size.cy * layout.anchorTopLeft.cy / percent.cy + layout.marginTopLeft.cy;
	rectMargins.right = size.cx * (100 - layout.anchorBottomRight.cx) / percent.cx - layout.marginBottomRight.cx;
	rectMargins.bottom = size.cy * (100 - layout.anchorBottomRight.cy) / percent.cy - layout.marginBottomRight.cy;

	return TRUE;
}

/*!
 *  This function is used to set the initial resize properties of a control
 *  in the layout, that are stored in the @c properties member of the
 *  related @c LAYOUTINFO structure.
 *
 *  @param layout Reference to the @c LAYOUTINFO structure to be set
 *
 *  @remarks The various flags are used to specify whether the resize
 *           properties (clipping, refresh) can change at run-time, and a new
 *           call to the property querying functions is needed at every
 *           layout update, or they are static properties, and the cached
 *           value is used whenever necessary.
 *        @n The default implementation sends a registered message to the
 *           control, giving it the opportunity to specify its resize
 *           properties, which takes precedence if the message is supported.
 *           It then sets the @a clipping property as static, calling
 *           @c LikesClipping only once, and the @a refresh property as
 *           dynamic, causing @c NeedsRefresh to be called every time.
 *        @n This should be right for most situations, as the need for
 *           @a refresh usually depends on the size fo a control, while the
 *           support for @a clipping is usually linked to the specific type
 *           of control, which is unlikely to change at run-time, but you can
 *           still override this function if a different beahvior is needed.
 *
 *  @sa LikesClipping NeedsRefresh LAYOUTINFO RESIZEPROPERTIES
 */
void CResizableLayout::InitResizeProperties(LAYOUTINFO &layout) const
{
	// check if custom window supports this library
	// (properties must be correctly set by the window)
	layout.bMsgSupport = Send_QueryProperties(layout.hWnd, &layout.properties);

	// default properties
	if (!layout.bMsgSupport)
	{
		// clipping property is assumed as static
		layout.properties.bAskClipping = FALSE;
		layout.properties.bCachedLikesClipping = LikesClipping(layout);
		// refresh property is assumed as dynamic
		layout.properties.bAskRefresh = TRUE;
	}
}

/*!
 *  This function modifies a window to enable resizing functionality.
 *  This affects the window style, size, system menu and appearance.
 *
 *  @param lpCreateStruct Pointer to a @c CREATESTRUCT structure, usually
 *         passed by the system to the window procedure in a @c WM_CREATE
 *         or @c WM_NCCREATE
 *
 *  @remarks The function is intended to be called only inside a @c WM_CREATE
 *           or @c WM_NCCREATE message handler.
 */
void CResizableLayout::MakeResizable(LPCREATESTRUCT lpCreateStruct) const
{
	if (lpCreateStruct->style & WS_CHILD)
		return;

	InitThemeSettings(); //! @todo move theme check in more appropriate place

	CWnd* pWnd = GetResizableWnd();

#if (_WIN32_WINNT >= 0x0501 && !defined(RSZLIB_NO_XP_DOUBLE_BUFFER))
	// enable double-buffering on supported platforms
	pWnd->ModifyStyleEx(0, WS_EX_COMPOSITED);
#endif

	if (!(lpCreateStruct->style & WS_THICKFRAME))
	{
		// keep client area
		CRect rect(CPoint(lpCreateStruct->x, lpCreateStruct->y),
			CSize(lpCreateStruct->cx, lpCreateStruct->cy));
		pWnd->SendMessage(WM_NCCALCSIZE, FALSE, (LPARAM)&rect);
		// set resizable style
		pWnd->ModifyStyle(DS_MODALFRAME, WS_THICKFRAME);
		// adjust size to reflect new style
		::AdjustWindowRectEx(&rect, pWnd->GetStyle(),
			::IsMenu(pWnd->GetMenu()->GetSafeHmenu()), pWnd->GetExStyle());
		pWnd->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
			SWP_NOSENDCHANGING|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREPOSITION);
		// update dimensions
		lpCreateStruct->cx = rect.Width();
		lpCreateStruct->cy = rect.Height();
	}
}

/*!
 *  This function should be called inside the parent window @c WM_NCCALCSIZE
 *  message handler to help eliminate flickering.
 *
 *  @param bAfterDefault Flag that specifies wether the call is made before
 *         or after the default handler
 *  @param lpncsp Pointer to the @c NCCALCSIZE_PARAMS structure that is
 *         passed to the message handler
 *  @param lResult Reference to the result of the message handler.
 *         It contains the default handler result on input and the value to
 *         return from the window procedure on output.
 *
 *  @remarks This function fixes the annoying flickering effect that is
 *           visible when resizing the top or left edges of the window
 *           (at least on a "left to right" Windows localized version).
 */
void CResizableLayout::HandleNcCalcSize(BOOL bAfterDefault, LPNCCALCSIZE_PARAMS lpncsp, LRESULT &lResult)
{
	// prevent useless complication when size is not changing
	// prevent recursion when resetting the window region (see below)
	if ((lpncsp->lppos->flags & SWP_NOSIZE)
#if (_WIN32_WINNT >= 0x0501)
		|| m_bNoRecursion
#endif
		)
		return;

	if (!bAfterDefault)
	{
		// save a copy before default handler gets called
		m_rectClientBefore = lpncsp->rgrc[2];
	}
	else // after default WM_NCCALCSIZE msg processing
	{
		if (lResult != 0)
		{
			// default handler already uses an advanced validation policy, give up
			return;
		}
		// default calculated client rect
		RECT &rectClientAfter = lpncsp->rgrc[0];

		// intersection between old and new client area is to be preserved
		// set source and destination rects to this intersection
		RECT &rectPreserve = lpncsp->rgrc[1];
		::IntersectRect(&rectPreserve, &rectClientAfter, &m_rectClientBefore);
		lpncsp->rgrc[2] = rectPreserve;

		lResult = WVR_VALIDRECTS;

		// FIX: window region must be updated before the result of the
		//		WM_NCCALCSIZE message gets processed by the system,
		//		otherwise the old window region will clip the client
		//		area during the preservation process.
		//		This is especially evident on WinXP when the non-client
		//		area is rendered with Visual Styles enabled and the
		//		windows have a non rectangular region.
		// NOTE: Implementers of skin systems that modify the window region
		//      should not rely on this fix and should handle non-client
		//      window messages themselves, to avoid flickering
#if (_WIN32_WINNT >= 0x0501)
		if ((real_WIN32_WINNT >= 0x0501)
			&& (real_ThemeSettings & STAP_ALLOW_NONCLIENT))
		{
			CWnd* pWnd = GetResizableWnd();
			DWORD dwStyle = pWnd->GetStyle();
			if ((dwStyle & (WS_CAPTION|WS_MAXIMIZE)) == WS_CAPTION)
			{
				m_bNoRecursion = TRUE;
				pWnd->SetWindowRgn(NULL, FALSE);
				m_bNoRecursion = FALSE;
			}
		}
#endif
	}
}
