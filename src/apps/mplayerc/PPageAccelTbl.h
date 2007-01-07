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

#include "PPageBase.h"
#include "PlayerListCtrl.h"
#include "StaticLink.h"

// CPPageAccelTbl dialog

class CPPageAccelTbl : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageAccelTbl)

private:
	enum {COL_CMD, COL_MOD, COL_KEY, COL_TYPE, COL_ID, COL_MOUSE, COL_APPCMD, COL_RMCMD, COL_RMREPCNT};
	CList<wmcmd> m_wmcmds;

	void SetupList();

	int m_counter;

public:
	CPPageAccelTbl();
	virtual ~CPPageAccelTbl();

	static CString MakeAccelModLabel(BYTE fVirt);
	static CString MakeAccelVkeyLabel(WORD key, bool fVirtKey);
	static CString MakeAccelShortcutLabel(UINT id);
	static CString MakeAccelShortcutLabel(ACCEL& a);
	static CString MakeMouseButtonLabel(UINT mouse);
	static CString MakeAppCommandLabel(UINT id);

	enum {APPCOMMAND_LAST=APPCOMMAND_MEDIA_CHANNEL_DOWN};

// Dialog Data
	enum { IDD = IDD_PPAGEACCELTBL };
	CPlayerListCtrl m_list;
	BOOL m_fWinLirc;
	CString m_WinLircAddr;
	CEdit m_WinLircEdit;
	CStaticLink m_WinLircLink;
	BOOL m_fUIce;
	CString m_UIceAddr;
	CEdit m_UIceEdit;
	CStaticLink m_UIceLink;
	BOOL m_fPN31;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
};
