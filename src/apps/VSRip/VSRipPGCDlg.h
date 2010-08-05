/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include <afxwin.h>
#include "VSRipPage.h"


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
	virtual bool CanGoPrev()
	{
		return(true);
	}
	virtual bool CanGoNext();
	virtual CString GetHeaderText()
	{
		return(_T("Extraction settings"));
	}
	virtual CString GetDescText()
	{
		return(_T("Select the program chain and angle you did or ")
			   _T("will do in the dvd ripper. Optionally, remove any not ")
			   _T("needed language streams and vob/cell ids."));
	}

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
