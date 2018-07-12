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
 *  @brief Implementation of the CResizableWndState class.
 */

#include "stdafx.h"
#include "ResizableWndState.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableWndState::CResizableWndState()
{

}

CResizableWndState::~CResizableWndState()
{

}

// used to save/restore window's size and position
// either in the registry or a private .INI file
// depending on your application settings

#define PLACEMENT_ENT	_T("WindowPlacement")
#define PLACEMENT_FMT 	_T("%ld,%ld,%ld,%ld,%u,%u,%ld,%ld")

/*!
 *  This function saves the current window position and size using the base
 *  class persist method. Minimized and maximized state is also optionally
 *  preserved.
 *  @sa CResizableState::WriteState
 *  @note Window coordinates are in the form used by the system functions
 *  GetWindowPlacement and SetWindowPlacement.
 *
 *  @param pszName String that identifies stored settings
 *  @param bRectOnly Flag that specifies wether to ignore min/max state
 *
 *  @return Returns @a TRUE if successful, @a FALSE otherwise
 */
BOOL CResizableWndState::SaveWindowRect(LPCTSTR pszName, BOOL bRectOnly)
{
	WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};

	if (!GetResizableWnd()->GetWindowPlacement(&wp))
		return FALSE;

	// use workspace coordinates
	const RECT& rc = wp.rcNormalPosition;

	CString data;
	if (bRectOnly)	// save size/pos only (normal state)
	{
		data.Format(PLACEMENT_FMT, rc.left, rc.top,
			rc.right, rc.bottom, SW_SHOWNORMAL, 0U, 0L, 0L);
	}
	else	// save also min/max state
	{
		data.Format(PLACEMENT_FMT, rc.left, rc.top,
			rc.right, rc.bottom, wp.showCmd, wp.flags,
			wp.ptMinPosition.x, wp.ptMinPosition.y);
	}

	// MPC-HC custom code
	return WriteState(CString(pszName), PLACEMENT_ENT, data);
}

/*!
 *  This function loads and set the current window position and size using
 *  the base class persist method. Minimized and maximized state is also
 *  optionally preserved.
 *  @sa CResizableState::WriteState
 *  @note Window coordinates are in the form used by the system functions
 *  GetWindowPlacement and SetWindowPlacement.
 *
 *  @param pszName String that identifies stored settings
 *  @param bRectOnly Flag that specifies wether to ignore min/max state
 *
 *  @return Returns @a TRUE if successful, @a FALSE otherwise
 */
BOOL CResizableWndState::LoadWindowRect(LPCTSTR pszName, BOOL bRectOnly)
{
	CString data;
	WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};

	// MPC-HC custom code
	if (!ReadState(CString(pszName), PLACEMENT_ENT, data))	// never saved before
		return FALSE;

	if (!GetResizableWnd()->GetWindowPlacement(&wp))
		return FALSE;

	// use workspace coordinates
	RECT& rc = wp.rcNormalPosition;

	if (_stscanf_s(data, PLACEMENT_FMT, &rc.left, &rc.top,
		&rc.right, &rc.bottom, &wp.showCmd, &wp.flags,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y) == 8)
	{
		if (bRectOnly)	// restore size/pos only
		{
			// MPC-HC custom code
			wp.showCmd = SW_HIDE;
			wp.flags = 0;
		}
		// restore also min/max state
		return GetResizableWnd()->SetWindowPlacement(&wp);
	}
	return FALSE;
}