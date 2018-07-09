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
 *  @brief Implementation of the CResizableState class.
 */

#include "stdafx.h"
#include "ResizableState.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR CResizableState::m_sDefaultStorePath = _T("ResizableState");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizableState::CResizableState()
{
	m_sStorePath = m_sDefaultStorePath;
}

CResizableState::~CResizableState()
{

}

/*!
 *  Static function to set the default path used to store state information.
 *  This path is used to initialize all the instances of this class.
 *  @sa GetDefaultStateStore GetStateStore SetStateStore
 *
 *  @param szPath String that specifies the new path to be set
 */
void CResizableState::SetDefaultStateStore(LPCTSTR szPath)
{
	m_sDefaultStorePath = szPath;
}

/*!
 *  Static function to retrieve the default path used to store state
 *  information.
 *  This path is used to initialize all the instances of this class.
 *  @sa SetDefaultStateStore GetStateStore SetStateStore
 *
 *  @return The return value is a string that specifies the current path
 */
LPCTSTR CResizableState::GetDefaultStateStore()
{
	return m_sDefaultStorePath;
}

/*!
 *  This function sets the path used to store state information by
 *  the current instance of the class.
 *  @sa GetStateStore GetDefaultStateStore SetDefaultStateStore
 *
 *  @param szPath String that specifies the new path to be set
 */
void CResizableState::SetStateStore(LPCTSTR szPath)
{
	m_sStorePath = szPath;
}

/*!
 *  This function retrieves the path used to store state information by
 *  the current instance of the class.
 *  @sa SetStateStore GetDefaultStateStore SetDefaultStateStore
 *
 *  @return The return value is a string that specifies the current path
 */
LPCTSTR CResizableState::GetStateStore() const
{
	return m_sStorePath;
}

/*!
 *  This function writes state information and associates it with some
 *  identification text for later retrieval.
 *  The base implementation uses the application profile to persist state
 *  information, but this function can be overridden to implement
 *  different methods.
 *
 *  @param szId String that identifies the stored settings
 *  @param szState String that represents the state information to store
 *
 *  @return The return value is @a TRUE if settings have been successfully
 *          stored, @a FALSE otherwise.
 */
BOOL CResizableState::WriteState(LPCTSTR szId, LPCTSTR szValue, LPCTSTR szState)
{
	// MPC-HC custom code
	return AfxGetApp()->WriteProfileString(szId, szValue, szState);
}

/*!
 *  This function reads state information previously associated with some
 *  identification text.
 *  The base implementation uses the application profile to persist state
 *  information, but this function can be overridden to implement
 *  different methods.
 *
 *  @param szId String that identifies the stored settings
 *  @param rsState String to be filled with the retrieved state information
 *
 *  @return The return value is @a TRUE if settings have been successfully
 *          retrieved, @a FALSE otherwise.
 */
BOOL CResizableState::ReadState(LPCTSTR szId, LPCTSTR szValue, CString &rsState)
{
	// MPC-HC custom code
	rsState = AfxGetApp()->GetProfileString(szId, szValue);
	return !rsState.IsEmpty();
}
