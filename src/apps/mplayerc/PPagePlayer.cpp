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

#include "stdafx.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "PPagePlayer.h"


// CPPagePlayer dialog

IMPLEMENT_DYNAMIC(CPPagePlayer, CPPageBase)
CPPagePlayer::CPPagePlayer()
	: CPPageBase(CPPagePlayer::IDD, CPPagePlayer::IDD)
	, m_iAllowMultipleInst(0)
	, m_iAlwaysOnTop(FALSE)
	, m_fTrayIcon(FALSE)
	, m_iShowBarsWhenFullScreen(FALSE)
	, m_nShowBarsWhenFullScreenTimeOut(0)
	, m_iTitleBarTextStyle(0)
	, m_bTitleBarTextTitle(0)
	, m_fExitFullScreenAtTheEnd(FALSE)
	, m_fRememberWindowPos(FALSE)
	, m_fRememberWindowSize(FALSE)
	, m_fSnapToDesktopEdges(FALSE)
	, m_fUseIni(FALSE)
	, m_fKeepHistory(FALSE)
	, m_fHideCDROMsSubMenu(FALSE)
	, m_priority(FALSE)
	, m_launchfullscreen(FALSE)
	, m_fShowOSD(FALSE)
{
}

CPPagePlayer::~CPPagePlayer()
{
}

void CPPagePlayer::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_iAllowMultipleInst);
	DDX_Radio(pDX, IDC_RADIO3, m_iTitleBarTextStyle);
	DDX_Check(pDX, IDC_CHECK13, m_bTitleBarTextTitle);
	DDX_Check(pDX, IDC_CHECK2, m_iAlwaysOnTop);
	DDX_Check(pDX, IDC_CHECK3, m_fTrayIcon);
	DDX_Check(pDX, IDC_CHECK4, m_iShowBarsWhenFullScreen);
	DDX_Text(pDX, IDC_EDIT1, m_nShowBarsWhenFullScreenTimeOut);
	DDX_Check(pDX, IDC_CHECK5, m_fExitFullScreenAtTheEnd);
	DDX_Check(pDX, IDC_CHECK6, m_fRememberWindowPos);
	DDX_Check(pDX, IDC_CHECK7, m_fRememberWindowSize);
	DDX_Check(pDX, IDC_CHECK12, m_fSnapToDesktopEdges);	
	DDX_Check(pDX, IDC_CHECK8, m_fUseIni);
	DDX_Control(pDX, IDC_SPIN1, m_nTimeOutCtrl);
	DDX_Check(pDX, IDC_CHECK1, m_fKeepHistory);
	DDX_Check(pDX, IDC_CHECK10, m_fHideCDROMsSubMenu);
	DDX_Check(pDX, IDC_CHECK9, m_priority);
	DDX_Check(pDX, IDC_CHECK11, m_launchfullscreen);
	DDX_Check(pDX, IDC_SHOW_OSD, m_fShowOSD);
}

BEGIN_MESSAGE_MAP(CPPagePlayer, CPPageBase)
	ON_BN_CLICKED(IDC_CHECK8, OnBnClickedCheck8)
	ON_UPDATE_COMMAND_UI(IDC_SPIN1, OnUpdateTimeout)
	ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateTimeout)
	ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateTimeout)
	ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdateTimeout)
	ON_UPDATE_COMMAND_UI(IDC_CHECK13, OnUpdateCheck13)
END_MESSAGE_MAP()

// CPPagePlayer message handlers

BOOL CPPagePlayer::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_iAllowMultipleInst = s.fAllowMultipleInst;
	m_iTitleBarTextStyle = s.iTitleBarTextStyle;
	m_bTitleBarTextTitle = s.fTitleBarTextTitle;
	m_iAlwaysOnTop = s.iOnTop;
	m_fTrayIcon = s.fTrayIcon;
	m_iShowBarsWhenFullScreen = s.fShowBarsWhenFullScreen;
	m_nShowBarsWhenFullScreenTimeOut = s.nShowBarsWhenFullScreenTimeOut;
	m_nTimeOutCtrl.SetRange(-1, 10);
	m_fExitFullScreenAtTheEnd = s.fExitFullScreenAtTheEnd;
	m_fRememberWindowPos = s.fRememberWindowPos;
	m_fRememberWindowSize = s.fRememberWindowSize;
	m_fSnapToDesktopEdges = s.fSnapToDesktopEdges;
	m_fUseIni = ((CMPlayerCApp*)AfxGetApp())->IsIniValid();
	m_fKeepHistory = s.fKeepHistory;
	m_fHideCDROMsSubMenu = s.fHideCDROMsSubMenu;
	m_priority = s.priority != NORMAL_PRIORITY_CLASS;
	m_launchfullscreen = s.launchfullscreen;
	m_fShowOSD = s.fShowOSD;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPagePlayer::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fAllowMultipleInst = !!m_iAllowMultipleInst;
	s.iTitleBarTextStyle = m_iTitleBarTextStyle;
	s.fTitleBarTextTitle = !!m_bTitleBarTextTitle;
	s.iOnTop = m_iAlwaysOnTop;
	s.fTrayIcon = !!m_fTrayIcon;
	s.fShowBarsWhenFullScreen = !!m_iShowBarsWhenFullScreen;
	s.nShowBarsWhenFullScreenTimeOut = m_nShowBarsWhenFullScreenTimeOut;
	s.fExitFullScreenAtTheEnd = !!m_fExitFullScreenAtTheEnd;
	s.fRememberWindowPos = !!m_fRememberWindowPos;
	s.fRememberWindowSize = !!m_fRememberWindowSize;
	s.fSnapToDesktopEdges = !!m_fSnapToDesktopEdges;
	s.fKeepHistory = !!m_fKeepHistory;
	s.fHideCDROMsSubMenu = !!m_fHideCDROMsSubMenu;
	s.priority = !m_priority ? NORMAL_PRIORITY_CLASS : GetVersion() < 0 ? HIGH_PRIORITY_CLASS : ABOVE_NORMAL_PRIORITY_CLASS;
	s.launchfullscreen = !!m_launchfullscreen;
	s.fShowOSD = !!m_fShowOSD;

	if(!m_fKeepHistory)
	{
		for(int i = 0; i < s.MRU.GetSize(); i++) s.MRU.Remove(i);
		for(int i = 0; i < s.MRUDub.GetSize(); i++) s.MRUDub.Remove(i);
		s.MRU.WriteList();
		s.MRUDub.WriteList();
	}

	((CMainFrame*)AfxGetMainWnd())->ShowTrayIcon(s.fTrayIcon);

	::SetPriorityClass(::GetCurrentProcess(), s.priority);

	return __super::OnApply();
}

void CPPagePlayer::OnBnClickedCheck8()
{
	UpdateData();

	if(m_fUseIni) ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToIni();
	else ((CMPlayerCApp*)AfxGetApp())->StoreSettingsToRegistry();

	SetModified();
}

void CPPagePlayer::OnUpdateTimeout(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(m_iShowBarsWhenFullScreen);
}

void CPPagePlayer::OnUpdateCheck13(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(m_iTitleBarTextStyle == 1);
}
