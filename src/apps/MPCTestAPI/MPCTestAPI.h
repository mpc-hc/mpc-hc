// RegisterCopyData.h : main header file for the REGISTERCOPYDATA application
//

#if !defined(AFX_REGISTERCOPYDATA_H__6D924C21_1ACC_4896_A34C_BC3906D8B012__INCLUDED_)
#define AFX_REGISTERCOPYDATA_H__6D924C21_1ACC_4896_A34C_BC3906D8B012__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRegisterCopyDataApp:
// See RegisterCopyData.cpp for the implementation of this class
//

class CRegisterCopyDataApp : public CWinApp
{
public:
	CRegisterCopyDataApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegisterCopyDataApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRegisterCopyDataApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGISTERCOPYDATA_H__6D924C21_1ACC_4896_A34C_BC3906D8B012__INCLUDED_)
