/* 
 *	Copyright (C) 2003-2005 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <afxtempl.h>
#include "..\..\subtitles\VobSubFileRipper.h"

// CVSRipPage dialog

class CVSRipPage : public CDialog, public IVSFRipperCallbackImpl
{
	DECLARE_DYNAMIC(CVSRipPage)

protected:
	CComPtr<IVSFRipper> m_pVSFRipper;

public:
	CVSRipPage(IVSFRipper* pVSFRipper, UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
	virtual ~CVSRipPage();

//	static bool ParseParamFile(CString fn);

	virtual void OnPrev() {}
	virtual void OnNext() {}
	virtual void OnClose() {}
	virtual bool CanGoPrev() {return(false);}
	virtual bool CanGoNext() {return(false);}
	virtual bool CanClose() {return(true);}
	virtual CString GetPrevText() {return(_T("< &Back"));}
	virtual CString GetNextText() {return(_T("&Next >"));}
	virtual CString GetCloseText() {return(_T("&Cancel"));}
	virtual CString GetHeaderText() {return(_T("Header Text"));}
	virtual CString GetDescText() {return(_T("Hello World"));}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
