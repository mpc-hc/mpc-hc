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
#include "VSRipPage.h"
#include "afxwin.h"


// CVSRipPGCDlg dialog

class CVSRipPGCDlg : public CVSRipPage
{
	DECLARE_DYNAMIC(CVSRipPGCDlg)

private:
	VSFRipperData m_rd;
	void SetupPGCList();
	void SetupAngleList();
	void SetupVCList();
	void SetupLangList();

public:
	CVSRipPGCDlg(IVSFRipper* pVSFRipper, CWnd* pParent = NULL);   // standard constructor
	virtual ~CVSRipPGCDlg();

	virtual void OnPrev();
	virtual void OnNext();
	virtual bool CanGoPrev() {return(true);}
	virtual bool CanGoNext();
	virtual CString GetHeaderText() {return(_T("Extraction settings"));}
	virtual CString GetDescText() {return(_T("Select the program chain and angle you did or ")
										_T("will do in the dvd ripper. Optionally, remove any not ")
										_T("needed language streams and vob/cell ids."));}

// Dialog Data
	enum { IDD = IDD_DIALOG_PGC };
	CListBox m_pgclist;
	CListBox m_anglelist;
	CListBox m_vclist;
	CListBox m_langlist;
	BOOL m_bResetTime;
	BOOL m_bClosedCaption;
	BOOL m_bForcedOnly;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnLbnSelchangeList2();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
