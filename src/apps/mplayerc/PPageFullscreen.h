/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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

// CPPageFullscreen dialog

class CPPageFullscreen : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageFullscreen)

	//	private:
	CAtlArray<dispmode> m_dms;
	CStringArray m_MonitorDisplayNames;

public:
	CPPageFullscreen();
	virtual ~CPPageFullscreen();

	BOOL m_launchfullscreen;
	BOOL m_fSetFullscreenRes;
	BOOL m_fSetDefault;
	CComboBox m_dispmode24combo;
	CComboBox m_dispmode25combo;
	CComboBox m_dispmode30combo;
	CComboBox m_dispmodeOthercombo;
	CComboBox m_dispmode23d976combo;
	CComboBox m_dispmode29d97combo;

	AChFR m_AutoChangeFullscrRes;
	CStringW m_f_hmonitor;
	int m_iMonitorType;
	CComboBox m_iMonitorTypeCtrl;

	BOOL m_iShowBarsWhenFullScreen;
	int m_nShowBarsWhenFullScreenTimeOut;
	BOOL m_fExitFullScreenAtTheEnd;
	CSpinButtonCtrl m_nTimeOutCtrl;
	BOOL m_fRestoreResAfterExit;

	// Dialog Data
	enum { IDD = IDD_PPAGEFULLSCREEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnUpdateDispMode24Combo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDispMode25Combo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDispMode30Combo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDispModeOtherCombo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDispMode23d976Combo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDispMode29d97Combo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateApplyDefault(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFullScrCombo();
	afx_msg void OnUpdateTimeout(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRestoreRes(CCmdUI* pCmdUI);
	void ModesUpdate();
};
