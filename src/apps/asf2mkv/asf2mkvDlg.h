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

// asf2mkvDlg.h : header file
//

#pragma once

#include <afxole.h>

#define WM_OPENURL WM_APP

class CUrlDropTarget : public COleDropTarget
{
public:
	CUrlDropTarget() {}

	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	void OnDragLeave(CWnd* pWnd);
	DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);

	DECLARE_MESSAGE_MAP()
};

// Casf2mkvDlg dialog
class Casf2mkvDlg : public CResizableDialog
{
	CComPtr<IGraphBuilder> pGB;
	CComQIPtr<IMediaControl> pMC;
	CComQIPtr<IMediaEventEx> pME;
	CComQIPtr<IMediaSeeking> pMS;
	CComQIPtr<IVideoWindow> pVW;
	CComQIPtr<IBasicVideo> pBV;

	bool m_fRecording;
	CString m_dst;

	class CRecentFileAndURLList : public CRecentFileList
	{
	public:
		CRecentFileAndURLList(UINT nStart, LPCTSTR lpszSection,
			LPCTSTR lpszEntryFormat, int nSize,
			int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

		virtual void Add(LPCTSTR lpszPathName); // we have to override CRecentFileList::Add because the original version can't handle URLs
	};

	CRecentFileAndURLList m_mru;
	void SetupCombo();

	void SetVideoRect();

	CUrlDropTarget m_urlDropTarget;

// Construction
public:
	Casf2mkvDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ASF2MKV_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	CComboBox m_combo;
	CStatic m_video;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRecord();
	afx_msg void OnUpdateRecord(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSettings(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButton2();
	afx_msg LRESULT OnUrlOpen(WPARAM wParam, LPARAM lParam);
};
