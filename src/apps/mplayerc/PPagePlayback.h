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

// CPPagePlayback dialog

class CPPagePlayback : public CPPageBase
{
	DECLARE_DYNAMIC(CPPagePlayback)

private:
	CAtlArray<dispmode> m_dms;

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
	BOOL m_fSetFullscreenRes;
	CComboBox m_dispmodecombo;
	BOOL m_fAutoloadAudio;
	BOOL m_fAutoloadSubtitles;
	BOOL m_fEnableWorkerThreadForOpening;
	BOOL m_fReportFailedPins;

// Dialog Data
	enum { IDD = IDD_PPAGEPLAYBACK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedRadio12(UINT nID);
	afx_msg void OnUpdateLoopNum(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAutoZoomCombo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDispModeCombo(CCmdUI* pCmdUI);
};
