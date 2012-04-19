// ResizableState.cpp: implementation of the CResizableState class.
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
#include "ResizableState.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableState::CResizableState()
{

}

CResizableState::~CResizableState()
{

}


// used to save/restore window's size and position
// either in the registry or a private .INI file
// depending on your application settings

#define PLACEMENT_ENT	_T("WindowPlacement")
#define PLACEMENT_FMT 	_T("%d,%d,%d,%d,%d,%d")

BOOL CResizableState::SaveWindowRect(LPCTSTR pszSection, BOOL bRectOnly)
{
	CString data;
	WINDOWPLACEMENT wp;

	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);
	if (!GetResizableWnd()->GetWindowPlacement(&wp))
		return FALSE;
	
	RECT& rc = wp.rcNormalPosition;	// alias

	if (bRectOnly)	// save size/pos only (normal state)
	{
		// use screen coordinates
		GetResizableWnd()->GetWindowRect(&rc);

		data.Format(PLACEMENT_FMT, rc.left, rc.top,
			rc.right, rc.bottom, SW_NORMAL, 0);
	}
	else	// save also min/max state
	{
		// use workspace coordinates
		data.Format(PLACEMENT_FMT, rc.left, rc.top,
			rc.right, rc.bottom, wp.showCmd, wp.flags);
	}

	return AfxGetApp()->WriteProfileString(pszSection, PLACEMENT_ENT, data);
}

BOOL CResizableState::LoadWindowRect(LPCTSTR pszSection, BOOL bRectOnly)
{
	CString data;
	WINDOWPLACEMENT wp;

	data = AfxGetApp()->GetProfileString(pszSection, PLACEMENT_ENT);
	
	if (data.IsEmpty())	// never saved before
		return FALSE;
	
	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);
	if (!GetResizableWnd()->GetWindowPlacement(&wp))
		return FALSE;

	RECT& rc = wp.rcNormalPosition;	// alias

	if (_stscanf_s(data, PLACEMENT_FMT, &rc.left, &rc.top,
		&rc.right, &rc.bottom, &wp.showCmd, &wp.flags) == 6)
	{
		if (bRectOnly)	// restore size/pos only
		{
			CRect rect(rc);
			return GetResizableWnd()->SetWindowPos(NULL, rect.left, rect.top,
				rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER |
				SWP_NOREPOSITION);
		}
		else	// restore also min/max state
		{
			return GetResizableWnd()->SetWindowPlacement(&wp);
		}
	}
	return FALSE;
}
