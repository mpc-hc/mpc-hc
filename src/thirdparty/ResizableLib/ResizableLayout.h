// ResizableLayout.h: interface for the CResizableLayout class.
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000-2002 by Paolo Messina
// (http://www.geocities.com/ppescher - ppescher@yahoo.com)
//
// The contents of this file are subject to the Artistic License (the "License").
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.opensource.org/licenses/artistic-license.html
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESIZABLELAYOUT_H__INCLUDED_)
#define AFX_RESIZABLELAYOUT_H__INCLUDED_

#include <afxtempl.h>
#include "ResizableMsgSupport.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// useful compatibility constants (the only one required is NOANCHOR)

const CSize NOANCHOR(-1,-1),
	TOP_LEFT(0,0), TOP_CENTER(50,0), TOP_RIGHT(100,0),
	MIDDLE_LEFT(0,50), MIDDLE_CENTER(50,50), MIDDLE_RIGHT(100,50),
	BOTTOM_LEFT(0,100), BOTTOM_CENTER(50,100), BOTTOM_RIGHT(100,100);


class CResizableLayout
{
protected:
	class LayoutInfo
	{
	public:
		HWND hWnd;
		UINT nCallbackID;

		CString sWndClass;

		// upper-left corner
		SIZE sizeTypeTL;
		SIZE sizeMarginTL;
		
		// bottom-right corner
		SIZE sizeTypeBR;
		SIZE sizeMarginBR;

		// custom window support
		BOOL bMsgSupport;
		RESIZEPROPERTIES properties;
	
	public:
		LayoutInfo() : hWnd(NULL), nCallbackID(0), bMsgSupport(FALSE)
		{ 
			sizeTypeTL.cx = 0;
			sizeTypeTL.cy = 0;
			sizeMarginTL.cx = 0;
			sizeMarginTL.cy = 0;
			sizeTypeBR.cx = 0;
			sizeTypeBR.cy = 0;
			sizeMarginBR.cx = 0;
			sizeMarginBR.cy = 0;
			memset(&properties, 0, sizeof properties);
		}

		LayoutInfo(HWND hwnd, SIZE tl_t, SIZE tl_m, 
			SIZE br_t, SIZE br_m, CString classname)
			: hWnd(hwnd), nCallbackID(0),
			sWndClass(classname), bMsgSupport(FALSE),
			sizeTypeTL(tl_t), sizeMarginTL(tl_m),
			sizeTypeBR(br_t), sizeMarginBR(br_m)
		{ 
			memset(&properties, 0, sizeof properties);
		}
	};

private:
	// list of repositionable controls
	CMap<HWND, HWND, POSITION, POSITION> m_mapLayout;
	CList<LayoutInfo, LayoutInfo&> m_listLayout;
	CList<LayoutInfo, LayoutInfo&> m_listLayoutCB;

	void ClipChildWindow(const CResizableLayout::LayoutInfo &layout, CRgn* pRegion);

	void CalcNewChildPosition(const CResizableLayout::LayoutInfo &layout,
		const CRect &rectParent, CRect &rectChild, UINT& uFlags);

protected:
	// override to initialize resize properties (clipping, refresh)
	virtual void InitResizeProperties(CResizableLayout::LayoutInfo& layout);

	// override to specify clipping for unsupported windows
	virtual BOOL LikesClipping(const CResizableLayout::LayoutInfo &layout);

	// override to specify refresh for unsupported windows
	virtual BOOL NeedsRefresh(const CResizableLayout::LayoutInfo &layout,
		const CRect &rectOld, const CRect &rectNew);

	// paint the background on the given DC (for XP theme's compatibility)
	void EraseBackground(CDC* pDC);

	// clip out child windows from the given DC (support old code)
	void ClipChildren(CDC* pDC);

	// get the clipping region (without clipped child windows)
	void GetClippingRegion(CRgn* pRegion);
	
	// override for scrollable or expanding parent windows
	virtual void GetTotalClientRect(LPRECT lpRect);

	// add anchors to a control, given its HWND
	void AddAnchor(HWND hWnd, CSize sizeTypeTL, CSize sizeTypeBR = NOANCHOR);

	// add anchors to a control, given its ID
	void AddAnchor(UINT nID, CSize sizeTypeTL, CSize sizeTypeBR = NOANCHOR)
	{
		AddAnchor(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			sizeTypeTL, sizeTypeBR);
	}

	// add a callback (control ID or HWND is unknown or may change)
	void AddAnchorCallback(UINT nCallbackID);

	// get rect of an anchored window, given the parent's client area
	BOOL GetAnchorPosition(HWND hWnd, const CRect &rectParent,
		CRect &rectChild, UINT* lpFlags = NULL)
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			return FALSE;

		UINT uTmpFlags = 0;
		CalcNewChildPosition(m_listLayout.GetAt(pos), rectParent, rectChild,
			(lpFlags != NULL) ? (*lpFlags) : uTmpFlags);
		return TRUE;
	}

	// get rect of an anchored window, given the parent's client area
	BOOL GetAnchorPosition(UINT nID, const CRect &rectParent,
		CRect &rectChild, UINT* lpFlags = NULL)
	{
		return GetAnchorPosition(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			rectParent, rectChild, lpFlags);
	}

	// remove an anchored control from the layout, given its HWND
	BOOL RemoveAnchor(HWND hWnd)
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			return FALSE;

		m_listLayout.RemoveAt(pos);
		return m_mapLayout.RemoveKey(hWnd);
	}

	// remove an anchored control from the layout, given its HWND
	BOOL RemoveAnchor(UINT nID)
	{
		return RemoveAnchor(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID));
	}

	// reset layout content
	void RemoveAllAnchors()
	{
		m_mapLayout.RemoveAll();
		m_listLayout.RemoveAll();
		m_listLayoutCB.RemoveAll();
	}

	// adjust children's layout, when parent's size changes
	void ArrangeLayout();

	// override to provide dynamic control's layout info
	virtual BOOL ArrangeLayoutCallback(CResizableLayout::LayoutInfo& layout);

	// override to provide the parent window
	virtual CWnd* GetResizableWnd() = 0;

public:
	CResizableLayout() { }

	virtual ~CResizableLayout()
	{
		// just for safety
		RemoveAllAnchors();
	}
};

#endif // !defined(AFX_RESIZABLELAYOUT_H__INCLUDED_)
