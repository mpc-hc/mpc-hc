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

#include "afxcmn.h"
#include "afxwin.h"

// CSaveDlg dialog

class CSaveDlg : public CCmdUIDialog
{
	DECLARE_DYNAMIC(CSaveDlg)

private:
	CString m_in, m_out;
	CComPtr<IGraphBuilder> pGB;
	CComQIPtr<IMediaControl> pMC;
	CComQIPtr<IMediaEventEx> pME;
	CComQIPtr<IMediaSeeking> pMS;
	UINT_PTR m_nIDTimerEvent;

public:
	CSaveDlg(CString in, CString out, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSaveDlg();

// Dialog Data
	enum { IDD = IDD_SAVE_DLG };
	CAnimateCtrl m_anim;
	CProgressCtrl m_progress;
	CStatic m_report;
	CStatic m_fromto;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
};
