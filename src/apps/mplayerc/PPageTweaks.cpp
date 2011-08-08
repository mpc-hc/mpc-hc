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
	, m_OSD_Size(0)
	, m_fNotifyMSN(TRUE)
	, m_fNotifyGTSdll(FALSE)
	, m_GTSdllLink(_T("http://www.gts-stuff.com/index.php/topic,1664.0.html"))
	, m_fPreventMinimize(FALSE)
	, m_fUseWin7TaskBar(TRUE)
	, m_fDontUseSearchInFolder(FALSE)
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
	DDX_Check(pDX, IDC_CHECK7, m_fDontUseSearchInFolder);
	DDX_Check(pDX, IDC_CHECK8, m_fUseTimeTooltip);
	DDX_Control(pDX, IDC_COMBO3, m_TimeTooltipPosition);
	DDX_Control(pDX, IDC_COMBO1, m_FontType);
	DDX_Control(pDX, IDC_COMBO2, m_FontSize);
	DDX_Check(pDX, IDC_CHECK1, m_fFastSeek);
}

int CALLBACK EnumFontProc(ENUMLOGFONT FAR* lf, NEWTEXTMETRIC FAR* tm, int FontType, LPARAM dwData)
{
	CAtlArray<CString>* fntl = (CAtlArray<CString>*)dwData;
	if (FontType == TRUETYPE_FONTTYPE) {
		fntl->Add(lf->elfFullName);
	}
	return true;
}

BOOL CPPageTweaks::OnInitDialog()
{
	__super::OnInitDialog();

	SetHandCursor(m_hWnd, IDC_COMBO1);

	AppSettings& s = AfxGetAppSettings();

	m_fDisableXPToolbars = s.fDisableXPToolbars;
	m_fUseWMASFReader = s.fUseWMASFReader;
	m_nJumpDistS = s.nJumpDistS;
	m_nJumpDistM = s.nJumpDistM;
	m_nJumpDistL = s.nJumpDistL;
	m_fNotifyMSN = s.fNotifyMSN;
	m_fNotifyGTSdll = s.fNotifyGTSdll;

	m_fPreventMinimize = s.fPreventMinimize;
	m_fUseWin7TaskBar = s.fUseWin7TaskBar;
	m_fDontUseSearchInFolder =s.fDontUseSearchInFolder;

	m_fUseTimeTooltip = s.fUseTimeTooltip;
	m_TimeTooltipPosition.AddString(ResStr(IDS_TIME_TOOLTIP_ABOVE));
	m_TimeTooltipPosition.AddString(ResStr(IDS_TIME_TOOLTIP_BELOW));
	m_TimeTooltipPosition.SetCurSel(s.nTimeTooltipPosition);
	m_TimeTooltipPosition.EnableWindow(m_fUseTimeTooltip);

	m_OSD_Size = s.nOSDSize;
	m_OSD_Font = s.strOSDFont;

	m_fFastSeek = s.fFastSeek;

	CString str;
	int iSel = 0;
	m_FontType.Clear();
	m_FontSize.Clear();
	HDC dc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	CAtlArray<CString> fntl;
	EnumFontFamilies(dc, NULL,(FONTENUMPROC)EnumFontProc, (LPARAM)&fntl);
	DeleteDC(dc);
	for (int i=0; i< fntl.GetCount(); i++) {
		if (i>0 && fntl[i-1] == fntl[i]) {
			continue;
		}
		m_FontType.AddString(fntl[i]);
	}
	for (int i=0; i< m_FontType.GetCount(); i++) {
		m_FontType.GetLBText(i,str);
		if (m_OSD_Font == str) {
			iSel=i;
		}
	}
	m_FontType.SetCurSel(iSel);

	for (int i=10; i<26; i++) {
		str.Format(_T("%d"), i);
		m_FontSize.AddString(str);
		if (m_OSD_Size == i) {
			iSel=i;
		}
	}
	m_FontSize.SetCurSel(iSel-10);

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

	s.fPreventMinimize = !!m_fPreventMinimize;
	s.fUseWin7TaskBar = !!m_fUseWin7TaskBar;
	s.fDontUseSearchInFolder = !!m_fDontUseSearchInFolder;
	s.fUseTimeTooltip = !!m_fUseTimeTooltip;
	s.nTimeTooltipPosition = m_TimeTooltipPosition.GetCurSel();
	s.nOSDSize = m_OSD_Size;
	m_FontType.GetLBText(m_FontType.GetCurSel(),s.strOSDFont);

	s.fFastSeek = !!m_fFastSeek;

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	if (m_fUseWin7TaskBar) {
		pFrame->CreateThumbnailToolbar();
	}
	pFrame->UpdateThumbarButton();

	return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageTweaks, CPPageBase)
	ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateCheck3)
	ON_UPDATE_COMMAND_UI(IDC_CHECK2, OnUpdateCheck2)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK8, OnUseTimeTooltipClicked)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnChngOSDCombo)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnChngOSDCombo)
END_MESSAGE_MAP()


// CPPageTweaks message handlers

void CPPageTweaks::OnUpdateCheck3(CCmdUI* pCmdUI)
{
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

void CPPageTweaks::OnChngOSDCombo()
{
	CString str;
	m_OSD_Size = m_FontSize.GetCurSel()+10;
	m_FontType.GetLBText(m_FontType.GetCurSel(),str);
	((CMainFrame*)AfxGetMainWnd())->m_OSD.DisplayMessage(OSD_TOPLEFT, _T("Test"), 2000, m_OSD_Size, str);
	SetModified();
}

void CPPageTweaks::OnUseTimeTooltipClicked()
{
	m_TimeTooltipPosition.EnableWindow(IsDlgButtonChecked(IDC_CHECK8));
}