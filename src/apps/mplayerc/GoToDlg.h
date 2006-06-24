/* 
 *	Copyright (C) 2003-2006 Gabest
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
#include "afxwin.h"

// CGoToDlg dialog

class CGoToDlg : public CDialog
{
	DECLARE_DYNAMIC(CGoToDlg)

public:
	CGoToDlg(int time = -1, float fps = 0, CWnd* pParent = NULL);   // standard constructor
	virtual ~CGoToDlg();

	CString m_timestr;
	CString m_framestr;
	CEdit m_timeedit;
	CEdit m_frameedit;

	int m_time;
	float m_fps;

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
};
