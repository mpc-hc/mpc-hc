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
 *  @brief Interface for the CResizableState class.
 */

#if !defined(AFX_RESIZABLESTATE_H__INCLUDED_)
#define AFX_RESIZABLESTATE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*! @addtogroup CoreComponents
 *  @{
 */

//! @brief Provides basic persisting capabilities
/*!
 *  Derive from this class to persist user interface settings, or anything
 *  suitable. The base implementation uses the application profile, which can
 *  be set to either the Registry or an INI File. Other storing methods
 *  can be implemented in derived classes.
 */
class CResizableState
{
	static LPCTSTR m_sDefaultStorePath;
	CString m_sStorePath;

protected:

	//! @brief Get default path where state is stored
	static LPCTSTR GetDefaultStateStore();

	//! @brief Set default path where state is stored
	static void SetDefaultStateStore(LPCTSTR szPath);

	//! @brief Get current path where state is stored
	LPCTSTR GetStateStore() const;

	//! @brief Set current path where state is stored
	void SetStateStore(LPCTSTR szPath);

	//! @name Overridables
	//@{

	//! @brief Read state information
	virtual BOOL ReadState(LPCTSTR szId, LPCTSTR szValue, CString& rsState);

	//! @brief Write state information
	virtual BOOL WriteState(LPCTSTR szId, LPCTSTR szValue, LPCTSTR szState);

	//@}

public:
	CResizableState();
	virtual ~CResizableState();
};

// @}
#endif // !defined(AFX_RESIZABLESTATE_H__INCLUDED_)
