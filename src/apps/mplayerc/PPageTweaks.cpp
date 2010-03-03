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
#include "PPageTweaks.h"
#include "MainFrm.h"


// CPPageTweaks dialog

IMPLEMENT_DYNAMIC(CPPageTweaks, CPPageBase)
CPPageTweaks::CPPageTweaks()
	: CPPageBase(CPPageTweaks::IDD, CPPageTweaks::IDD)
	, m_fDisableXPToolbars(FALSE)
	, m_fUseWMASFReader(FALSE)
	, m_nJumpDistS(0)
	, m_nJumpDistM(0)
	, m_nJumpDistL(0)
	, m_fNotifyMSN(TRUE)
	, m_fNotifyGTSdll(FALSE)
	, m_GTSdllLink(_T("https://sourceforge.net/project/showfiles.php?group_id=82303&package_id=169521&release_id=371114"))
	, m_fPreventMinimize(FALSE)
	, m_fUseWin7TaskBar(TRUE)
{
	m_fWMASFReader = SUCCEEDED(CComPtr<IBaseFilter>().CoCreateInstance(
		GUIDFromCString(_T("{187463A0-5BB7-11D3-ACBE-0080C75E246E}")))); // WM ASF Reader
}

CPPageTweaks::~CPPageTweaks()
{
}

void CPPageTweaks::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK3, m_fDisableXPToolbars);
	DDX_Control(pDX, IDC_CHECK3, m_fDisableXPToolbarsCtrl);
	DDX_Check(pDX, IDC_CHECK2, m_fUseWMASFReader);
	DDX_Control(pDX, IDC_CHECK2, m_fUseWMASFReaderCtrl);
	DDX_Text(pDX, IDC_EDIT1, m_nJumpDistS);
	DDX_Text(pDX, IDC_EDIT2, m_nJumpDistM);
	DDX_Text(pDX, IDC_EDIT3, m_nJumpDistL);
	DDX_Check(pDX, IDC_CHECK4, m_fNotifyMSN);
	DDX_Check(pDX, IDC_CHECK5, m_fNotifyGTSdll);
	DDX_Control(pDX, IDC_STATICLINKGTS, m_GTSdllLink);
	DDX_Check(pDX, IDC_CHECK6, m_fPreventMinimize);
	DDX_Check(pDX, IDC_CHECK_WIN7, m_fUseWin7TaskBar);
}

BOOL CPPageTweaks::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_fDisableXPToolbars = s.fDisableXPToolbars;
	m_fUseWMASFReader = s.fUseWMASFReader;
	m_nJumpDistS = s.nJumpDistS;
	m_nJumpDistM = s.nJumpDistM;
	m_nJumpDistL = s.nJumpDistL;
	m_fNotifyMSN = s.fNotifyMSN;
	m_fNotifyGTSdll = s.fNotifyGTSdll;

	m_fPreventMinimize = s.m_fPreventMinimize;
	m_fUseWin7TaskBar = s.m_fUseWin7TaskBar;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageTweaks::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fDisableXPToolbars = !!m_fDisableXPToolbars;
	s.fUseWMASFReader = !!m_fUseWMASFReader;
	s.nJumpDistS = m_nJumpDistS;
	s.nJumpDistM = m_nJumpDistM;
	s.nJumpDistL = m_nJumpDistL;
	s.fNotifyMSN = !!m_fNotifyMSN;
	s.fNotifyGTSdll = !!m_fNotifyGTSdll;

	s.m_fPreventMinimize = m_fPreventMinimize;
	s.m_fUseWin7TaskBar = m_fUseWin7TaskBar;

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	if(m_fUseWin7TaskBar) pFrame->CreateThumbnailToolbar();
	pFrame->UpdateThumbarButton();

	return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageTweaks, CPPageBase)
	ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateCheck3)
	ON_UPDATE_COMMAND_UI(IDC_CHECK2, OnUpdateCheck2)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// CPPageTweaks message handlers

void CPPageTweaks::OnUpdateCheck3(CCmdUI* pCmdUI)
{
	if(!AfxGetAppSettings().fXpOrBetter)
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetCheck(TRUE);
	}
}

void CPPageTweaks::OnUpdateCheck2(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_fWMASFReader);
}

void CPPageTweaks::OnBnClickedButton1()
{
	m_nJumpDistS = 1000;
	m_nJumpDistM = 5000;
	m_nJumpDistL = 20000;

	UpdateData(FALSE);
}
