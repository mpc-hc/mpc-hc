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

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "PPagePlayback.h"

#include "Monitors.h"
#include "MultiMonitor.h"


// CPPagePlayback dialog

IMPLEMENT_DYNAMIC(CPPagePlayback, CPPageBase)
CPPagePlayback::CPPagePlayback()
	: CPPageBase(CPPagePlayback::IDD, CPPagePlayback::IDD)
	, m_iLoopForever(0)
	, m_nLoops(0)
	, m_fRewind(FALSE)
	, m_iZoomLevel(0)
	, m_iRememberZoomLevel(FALSE)
	, m_nVolume(0)
	, m_nBalance(0)
	, m_fAutoloadAudio(FALSE)
	, m_fAutoloadSubtitles(FALSE)
	, m_fEnableWorkerThreadForOpening(FALSE)
	, m_fReportFailedPins(FALSE)
	, m_subtitlesLanguageOrder(_T(""))
	, m_audiosLanguageOrder(_T(""))
{
}

CPPagePlayback::~CPPagePlayback()
{
}

void CPPagePlayback::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_volumectrl);
	DDX_Control(pDX, IDC_SLIDER2, m_balancectrl);
	DDX_Slider(pDX, IDC_SLIDER1, m_nVolume);
	DDX_Slider(pDX, IDC_SLIDER2, m_nBalance);
	DDX_Radio(pDX, IDC_RADIO1, m_iLoopForever);
	DDX_Control(pDX, IDC_EDIT1, m_loopnumctrl);
	DDX_Text(pDX, IDC_EDIT1, m_nLoops);
	DDX_Check(pDX, IDC_CHECK1, m_fRewind);
	DDX_CBIndex(pDX, IDC_COMBO1, m_iZoomLevel);
	DDX_Check(pDX, IDC_CHECK5, m_iRememberZoomLevel);
	DDX_Check(pDX, IDC_CHECK2, m_fAutoloadAudio);
	DDX_Check(pDX, IDC_CHECK3, m_fAutoloadSubtitles);
	DDX_Check(pDX, IDC_CHECK7, m_fEnableWorkerThreadForOpening);
	DDX_Check(pDX, IDC_CHECK6, m_fReportFailedPins);
	DDX_Text(pDX, IDC_EDIT2, m_subtitlesLanguageOrder);
	DDX_Text(pDX, IDC_EDIT3, m_audiosLanguageOrder);
}

BEGIN_MESSAGE_MAP(CPPagePlayback, CPPageBase)
	ON_WM_HSCROLL()
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO2, OnBnClickedRadio12)
	ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateLoopNum)
	ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateLoopNum)
	ON_UPDATE_COMMAND_UI(IDC_COMBO1, OnUpdateAutoZoomCombo)

	ON_STN_DBLCLK(IDC_STATIC_BALANCE, OnBalanceTextDblClk)
	ON_NOTIFY_EX(TTN_NEEDTEXTW, 0, OnToolTipNotify)
END_MESSAGE_MAP()


// CPPagePlayback message handlers

BOOL CPPagePlayback::OnInitDialog()
{
	__super::OnInitDialog();

	SetHandCursor(m_hWnd, IDC_COMBO1);

	AppSettings& s = AfxGetAppSettings();

	m_volumectrl.SetRange(0, 100);
	m_volumectrl.SetTicFreq(10);
	m_balancectrl.SetRange(-100, 100);
	m_balancectrl.SetLineSize(2);
	m_balancectrl.SetPageSize(2);
	m_balancectrl.SetTicFreq(20);
	m_nVolume = s.nVolume;
	m_nBalance = s.nBalance;
	m_iLoopForever = s.fLoopForever?1:0;
	m_nLoops = s.nLoops;
	m_fRewind = s.fRewind;
	m_iZoomLevel = s.iZoomLevel;
	m_iRememberZoomLevel = s.fRememberZoomLevel;
	m_fAutoloadAudio = s.fAutoloadAudio;
	m_fAutoloadSubtitles = s.fAutoloadSubtitles;
	m_fEnableWorkerThreadForOpening = s.fEnableWorkerThreadForOpening;
	m_fReportFailedPins = s.fReportFailedPins;
	m_subtitlesLanguageOrder = s.strSubtitlesLanguageOrder;
	m_audiosLanguageOrder = s.strAudiosLanguageOrder;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPagePlayback::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.nVolume = m_nVolume;
	s.nBalance = m_nBalance;
	s.fLoopForever = !!m_iLoopForever;
	s.nLoops = m_nLoops;
	s.fRewind = !!m_fRewind;
	s.iZoomLevel = m_iZoomLevel;
	s.fRememberZoomLevel = !!m_iRememberZoomLevel;
	s.fAutoloadAudio = !!m_fAutoloadAudio;
	s.fAutoloadSubtitles = !!m_fAutoloadSubtitles;
	s.fEnableWorkerThreadForOpening = !!m_fEnableWorkerThreadForOpening;
	s.fReportFailedPins = !!m_fReportFailedPins;
	s.strSubtitlesLanguageOrder = m_subtitlesLanguageOrder;
	s.strAudiosLanguageOrder = m_audiosLanguageOrder;

	return __super::OnApply();
}

LRESULT CPPagePlayback::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_HSCROLL || message == WM_VSCROLL) {
		SetModified();
	}

	return __super::DefWindowProc(message, wParam, lParam);
}

void CPPagePlayback::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (*pScrollBar == m_volumectrl) {
		UpdateData();
		((CMainFrame*)GetParentFrame())->m_wndToolBar.Volume = m_nVolume; // nice shortcut...
	} else if (*pScrollBar == m_balancectrl) {
		UpdateData();
		((CMainFrame*)GetParentFrame())->SetBalance(m_nBalance); // see prev note...
	}

	SetModified();

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPagePlayback::OnBnClickedRadio12(UINT nID)
{
	SetModified();
}

void CPPagePlayback::OnUpdateLoopNum(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsDlgButtonChecked(IDC_RADIO1));
}

void CPPagePlayback::OnUpdateAutoZoomCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!!IsDlgButtonChecked(IDC_CHECK5));
}

void CPPagePlayback::OnBalanceTextDblClk()
{
	// double click on text "Balance" resets the balance to zero
	m_balancectrl.SetPos(0);
	((CMainFrame*)GetParentFrame())->SetBalance(0);
	SetModified();
}

BOOL CPPagePlayback::OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	UINT nID = pNMHDR->idFrom;

	if (pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {
		nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID == 0) {
		return FALSE;
	}

	static CStringW m_strTipTextW;

	if (nID == IDC_SLIDER1) {
		m_strTipTextW.Format(L"%d%%", m_nVolume);
	} else if (nID == IDC_SLIDER2) {
		if (m_nBalance == 0) {
			m_strTipTextW = L"L = R";
		} else if (m_nBalance < 0) {
			m_strTipTextW.Format(L"L +%d%%", -m_nBalance);
		} else { //if (m_nBalance > 0)
			m_strTipTextW.Format(L"R +%d%%", m_nBalance);
		}
	} else {
		return FALSE;
	}

	if (pNMHDR->code == TTN_NEEDTEXTW) { //?possible check is not needed
		pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_strTipTextW;
	}

	*pResult = 0;

	return TRUE;    // message was handled
}

BOOL CPPagePlayback::OnSetActive()
{
	AppSettings& s = AfxGetAppSettings();
	m_iRememberZoomLevel = !!s.fRememberZoomLevel;
	UpdateData(FALSE);

	return __super::OnSetActive();
}

BOOL CPPagePlayback::OnKillActive()
{
	AppSettings& s = AfxGetAppSettings();
	if (s.fRememberZoomLevel) {
		s.fRememberWindowSize = false;
	}

	return __super::OnKillActive();
}