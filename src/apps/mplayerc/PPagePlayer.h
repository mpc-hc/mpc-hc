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

// CPPagePlayer dialog

class CPPagePlayer : public CPPageBase
{
	DECLARE_DYNAMIC(CPPagePlayer)

public:
	CPPagePlayer();
	virtual ~CPPagePlayer();

	int m_iAllowMultipleInst;
	int m_iTitleBarTextStyle;
	BOOL m_bTitleBarTextTitle;
	BOOL m_iAlwaysOnTop;
	BOOL m_iShowBarsWhenFullScreen;
	int m_nShowBarsWhenFullScreenTimeOut;
	BOOL m_fExitFullScreenAtTheEnd;
	BOOL m_fRememberWindowPos;
	BOOL m_fRememberWindowSize;
	BOOL m_fSnapToDesktopEdges;
	BOOL m_fUseIni;
	CSpinButtonCtrl m_nTimeOutCtrl;
	BOOL m_fTrayIcon;
	BOOL m_fKeepHistory;
	BOOL m_fHideCDROMsSubMenu;
	BOOL m_priority;
	BOOL m_launchfullscreen;

// Dialog Data
	enum { IDD = IDD_PPAGEPLAYER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCheck8();
	afx_msg void OnUpdateTimeout(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCheck13(CCmdUI* pCmdUI);
};
