/////////////////////////////////////////////////////////////////////////////
//
// This file is part of ResizableLib
// http://sourceforge.net/projects/resizablelib
//
// Copyright (C) 2000-2004 by Paolo Messina
// http://www.geocities.com/ppescher - mailto:ppescher@hotmail.com
//
// The contents of this file are subject to the Artistic License (the "License").
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.opensource.org/licenses/artistic-license.html
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

/*!
 *  @file
 *  @brief Interface for the CResizableWndState class.
 */

#if !defined(AFX_RESIZABLEWNDSTATE_H__INCLUDED_)
#define AFX_RESIZABLEWNDSTATE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ResizableState.h"

/*! @addtogroup CoreComponents
 *  @{
 */

//! @brief Persists window position, size and state
/*!
 *  Derive from this class when you want to persist the size, position and
 *  minimized/maximized state of top level windows.
 *  This class is used in the provided resizable counterparts of
 *  the standard MFC window and dialog classes.
 */
class CResizableWndState : public CResizableState  
{
protected:

	//! @brief Load and set the window position and size
	BOOL LoadWindowRect(LPCTSTR pszName, BOOL bRectOnly);

	//! @brief Save the current window position and size
	BOOL SaveWindowRect(LPCTSTR pszName, BOOL bRectOnly);

	//! @brief Override to provide the parent window
	virtual CWnd* GetResizableWnd() const = 0;

public:
	CResizableWndState();
	virtual ~CResizableWndState();
};

// @}
#endif // !defined(AFX_RESIZABLEWNDSTATE_H__INCLUDED_)
