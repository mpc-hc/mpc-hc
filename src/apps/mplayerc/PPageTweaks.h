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
#include "StaticLink.h"

// CPPageTweaks dialog

class CPPageTweaks : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageTweaks)

private:
	bool m_fWMASFReader;

public:
	CPPageTweaks();
	virtual ~CPPageTweaks();

	BOOL m_fDisableXPToolbars;
	CButton m_fDisableXPToolbarsCtrl;
	BOOL m_fUseWMASFReader;
	CButton m_fUseWMASFReaderCtrl;

	// Dialog Data
	enum { IDD = IDD_PPAGETWEAKS };
	int m_nJumpDistS;
	int m_nJumpDistM;
	int m_nJumpDistL;
	BOOL m_fNotifyMSN;
	BOOL m_fNotifyGTSdll;
	CStaticLink m_GTSdllLink;

	BOOL m_fPreventMinimize;
	BOOL m_fUseWin7TaskBar;
	BOOL m_fDontUseSearchInFolder;
	CComboBox m_FontSize;
	CComboBox m_FontType;
	int m_OSD_Size;
	CString	m_OSD_Font;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnUpdateCheck3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCheck2(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnChngOSDCombo();
};
