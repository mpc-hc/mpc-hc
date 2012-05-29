/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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
#include "afxmaskededit.h"
#include "resource.h"


// CGoToDlg dialog

class CGoToDlg : public CDialog
{
	DECLARE_DYNAMIC(CGoToDlg)

public:
	CGoToDlg(REFERENCE_TIME time = -1, REFERENCE_TIME maxTime = -1, double fps = 0, CWnd* pParent = NULL);   // standard constructor
	virtual ~CGoToDlg();

	CString m_timestr;
	CString m_framestr;
	CMFCMaskedEdit m_timeedit;
	CEdit m_frameedit;

	REFERENCE_TIME m_time;
	REFERENCE_TIME m_maxTime;
	double m_fps;

	// Dialog Data
	enum { IDD = IDD_GOTO_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk1();
	afx_msg void OnBnClickedOk2();
	virtual void OnOK();
};
