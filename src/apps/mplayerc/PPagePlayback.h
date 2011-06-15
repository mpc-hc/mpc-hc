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


// CPPagePlayback dialog

class CPPagePlayback : public CPPageBase
{
	DECLARE_DYNAMIC(CPPagePlayback)

private:
	CAtlArray<dispmode> m_dms;
	CStringArray m_MonitorDisplayNames;
public:
	CPPagePlayback();
	virtual ~CPPagePlayback();

	CSliderCtrl m_volumectrl;
	CSliderCtrl m_balancectrl;
	int m_nVolume;
	int m_nBalance;
	int m_iLoopForever;
	CEdit m_loopnumctrl;
	int m_nLoops;
	BOOL m_fRewind;
	int m_iZoomLevel;
	BOOL m_iRememberZoomLevel;
	BOOL m_fAutoloadAudio;
	BOOL m_fAutoloadSubtitles;
	BOOL m_fEnableWorkerThreadForOpening;
	BOOL m_fReportFailedPins;
	CString m_subtitlesLanguageOrder;
	CString m_audiosLanguageOrder;

	// Dialog Data
	enum { IDD = IDD_PPAGEPLAYBACK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedRadio12(UINT nID);
	afx_msg void OnUpdateLoopNum(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAutoZoomCombo(CCmdUI* pCmdUI);

	afx_msg void OnBalanceTextDblClk();
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
	virtual void OnCancel();
};
