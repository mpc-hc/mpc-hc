// SniffUSB.h : main header file for the SNIFFUSB application
//

#if !defined(AFX_SNIFFUSB_H__3ACD35A9_E49A_11D3_A755_00A0C9971EFC__INCLUDED_)
#define AFX_SNIFFUSB_H__3ACD35A9_E49A_11D3_A755_00A0C9971EFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSniffUSBApp:
// See SniffUSB.cpp for the implementation of this class
//

class CSniffUSBApp : public CWinApp
{
public:
	CSniffUSBApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSniffUSBApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSniffUSBApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNIFFUSB_H__3ACD35A9_E49A_11D3_A755_00A0C9971EFC__INCLUDED_)
