/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	BOOL m_fRememberWindowPos;
	BOOL m_fRememberWindowSize;
	BOOL m_fSnapToDesktopEdges;
	BOOL m_fUseIni;
	BOOL m_fTrayIcon;
	BOOL m_fKeepHistory;
	BOOL m_fHideCDROMsSubMenu;
	BOOL m_priority;
	BOOL m_fShowOSD;
	BOOL m_fRememberDVDPos;
	BOOL m_fRememberFilePos;

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
	afx_msg void OnUpdatePos(CCmdUI* pCmdUI);
};
