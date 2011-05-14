// RegisterCopyData.h : main header file for the REGISTERCOPYDATA application
//

#pragma once
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
