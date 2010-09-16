// ResizableDialog.cpp : implementation file
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

#include "stdafx.h"
#include "ResizableDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CResizableDialog

inline void CResizableDialog::PrivateConstruct()
{
	m_bEnableSaveRestore = FALSE;
	m_dwGripTempState = 1;
}

CResizableDialog::CResizableDialog()
{
	PrivateConstruct();
}

CResizableDialog::CResizableDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CCmdUIDialog(nIDTemplate, pParentWnd)
{
	PrivateConstruct();
}

CResizableDialog::CResizableDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
	: CCmdUIDialog(lpszTemplateName, pParentWnd)
{
	PrivateConstruct();
}

CResizableDialog::~CResizableDialog()
{
}


BEGIN_MESSAGE_MAP(CResizableDialog, CCmdUIDialog)
	//{{AFX_MSG_MAP(CResizableDialog)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CResizableDialog message handlers

int CResizableDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	// child dialogs don't want resizable border or size grip,
	// nor they can handle the min/max size constraints
	BOOL bChild = GetStyle() & WS_CHILD;

	if (!bChild)
	{
		// keep client area
		CRect rect;
		GetClientRect(&rect);
		// set resizable style
		ModifyStyle(DS_MODALFRAME, WS_POPUP | WS_THICKFRAME);
		// adjust size to reflect new style
		::AdjustWindowRectEx(&rect, GetStyle(),
			::IsMenu(GetMenu()->GetSafeHmenu()), GetExStyle());
		SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_FRAMECHANGED|
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREPOSITION);

		// set the initial size as the min track size
		SetMinTrackSize(rect.Size());
	}

	// create and init the size-grip
	if (!CreateSizeGrip(!bChild))
		return -1;

	return 0;
}

void CResizableDialog::OnDestroy() 
{
	if (m_bEnableSaveRestore)
		SaveWindowRect(m_sSection, m_bRectOnly);

	// remove child windows
	RemoveAllAnchors();

	__super::OnDestroy();
}

void CResizableDialog::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (nType == SIZE_MAXHIDE || nType == SIZE_MAXSHOW)
		return;		// arrangement not needed

	if (nType == SIZE_MAXIMIZED)
		HideSizeGrip(&m_dwGripTempState);
	else
		ShowSizeGrip(&m_dwGripTempState);

	// update grip and layout
	UpdateSizeGrip();
	ArrangeLayout();
}

void CResizableDialog::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	MinMaxInfo(lpMMI);
}

// NOTE: this must be called after setting the layout
//       to have the dialog and its controls displayed properly
void CResizableDialog::EnableSaveRestore(LPCTSTR pszSection, BOOL bRectOnly)
{
	m_sSection = pszSection;

	m_bEnableSaveRestore = TRUE;
	m_bRectOnly = bRectOnly;

	// restore immediately
	LoadWindowRect(pszSection, bRectOnly);
}


BOOL CResizableDialog::OnEraseBkgnd(CDC* pDC) 
{
	// Windows XP doesn't like clipping regions ...try this!
	EraseBackground(pDC);
	return TRUE;

/*	ClipChildren(pDC);	// old-method (for safety)

	return CDialog::OnEraseBkgnd(pDC);
*/
}
