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
 *  @brief Implementation of the CResizableSheetState class.
 */

#include "stdafx.h"
#include "ResizableSheetState.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableSheetState::CResizableSheetState()
{

}

CResizableSheetState::~CResizableSheetState()
{

}

// used to save/restore active page
// either in the registry or a private .INI file
// depending on your application settings

#define ACTIVEPAGE_ENT 	_T("ActivePage")

/*!
 *  This function saves the current property sheet active page using the base
 *  class persist method.
 *  @sa CResizableState::WriteState
 *  
 *  @param pszName String that identifies stored settings
 *  
 *  @return Returns @a TRUE if successful, @a FALSE otherwise
 */
BOOL CResizableSheetState::SavePage(LPCTSTR pszName)
{
	// saves active page index, or the initial page if problems
	// cannot use GetActivePage, because it always fails

	const CPropertySheet* pSheet = DYNAMIC_DOWNCAST(CPropertySheet, GetResizableWnd());
	if (pSheet == NULL)
		return FALSE;

	int page = pSheet->m_psh.nStartPage;
	const CTabCtrl *pTab = pSheet->GetTabControl();
	if (pTab != NULL) 
		page = pTab->GetCurSel();
	if (page < 0)
		page = pSheet->m_psh.nStartPage;

	CString data;
	data.Format(_T("%i"), page);

	// MPC-HC custom code
	return WriteState(CString(pszName), ACTIVEPAGE_ENT, data);
}

/*!
 *  This function loads the active page using the base class persist method.
 *  @sa CResizableState::ReadState
 *  
 *  @param pszName String that identifies stored settings
 *  
 *  @return Returns @a TRUE if successful, @a FALSE otherwise
 */
BOOL CResizableSheetState::LoadPage(LPCTSTR pszName)
{
	// restore active page, zero (the first) if not found

	CString data;
	// MPC-HC custom code
	if (!ReadState(CString(pszName), ACTIVEPAGE_ENT, data))
		return FALSE;
	
	CPropertySheet* pSheet = DYNAMIC_DOWNCAST(CPropertySheet, GetResizableWnd());

	return (pSheet != NULL) && pSheet->SetActivePage(_ttoi(data));
}
