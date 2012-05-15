/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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
#include "PPageOutput.h"
#include <moreuuids.h>
#include "PPageMisc.h"
#include <psapi.h>


// CPPageMisc dialog

IMPLEMENT_DYNAMIC(CPPageMisc, CPPageBase)
CPPageMisc::CPPageMisc()
	: CPPageBase(CPPageMisc::IDD, CPPageMisc::IDD)
{
}

CPPageMisc::~CPPageMisc()
{
}

void CPPageMisc::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLI_BRIGHTNESS, m_SliBrightness);
	DDX_Control(pDX, IDC_SLI_CONTRAST, m_SliContrast);
	DDX_Control(pDX, IDC_SLI_HUE, m_SliHue);
	DDX_Control(pDX, IDC_SLI_SATURATION, m_SliSaturation);
	DDX_Check(pDX, IDC_CHECK1, m_nUpdaterAutoCheck);
	DDX_Text(pDX, IDC_EDIT1, m_nUpdaterDelay);
	DDX_Control(pDX, IDC_SPIN1, m_updaterDelaySpin);
}


BEGIN_MESSAGE_MAP(CPPageMisc, CPPageBase)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnBnClickedReset)
	ON_BN_CLICKED(IDC_RESET_SETTINGS, OnResetSettings)
	ON_BN_CLICKED(IDC_EXPORT_SETTINGS, OnExportSettings)
	ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateDelayEditBox)
	ON_UPDATE_COMMAND_UI(IDC_SPIN1, OnUpdateDelayEditBox)
END_MESSAGE_MAP()


// CPPageMisc message handlers

BOOL CPPageMisc::OnInitDialog()
{
	COLORPROPERTY_RANGE*	ControlRange;
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	CreateToolTip();

	ControlRange = AfxGetMyApp()->GetColorControl (Brightness);
	if (ControlRange) {
		m_iBrightness = s.iBrightness;
		m_SliBrightness.EnableWindow	(TRUE);
		m_SliBrightness.SetRange		(ControlRange->MinValue, ControlRange->MaxValue, true);
		m_SliBrightness.SetTic			(ControlRange->DefaultValue);
		m_SliBrightness.SetPos			(m_iBrightness);
	}

	ControlRange = AfxGetMyApp()->GetColorControl (Contrast);
	if (ControlRange) {
		m_iContrast = s.iContrast;
		m_SliContrast.EnableWindow		(TRUE);
		m_SliContrast.SetRange			(ControlRange->MinValue, ControlRange->MaxValue, true);
		m_SliContrast.SetTic			(ControlRange->DefaultValue);
		m_SliContrast.SetPos			(m_iContrast);
	}

	ControlRange = AfxGetMyApp()->GetColorControl (Hue);
	if (ControlRange) {
		m_iHue = s.iHue;
		m_SliHue.EnableWindow			(TRUE);
		m_SliHue.SetRange				(ControlRange->MinValue, ControlRange->MaxValue, true);
		m_SliHue.SetTic					(ControlRange->DefaultValue);
		m_SliHue.SetPos					(m_iHue);
	}

	ControlRange = AfxGetMyApp()->GetColorControl (Saturation);
	if (ControlRange) {
		m_iSaturation	= s.iSaturation;
		m_SliSaturation.EnableWindow	(TRUE);
		m_SliSaturation.SetRange		(ControlRange->MinValue, ControlRange->MaxValue, true);
		m_SliSaturation.SetTic			(ControlRange->DefaultValue);
		m_SliSaturation.SetPos			(m_iSaturation);
	}

	m_nUpdaterAutoCheck = s.nUpdaterAutoCheck;
	m_nUpdaterDelay = s.nUpdaterDelay;
	m_updaterDelaySpin.SetRange32(1, 365);

	UpdateData(FALSE);

	return TRUE;
}

BOOL CPPageMisc::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.iBrightness				= m_iBrightness;
	s.iContrast					= m_iContrast;
	s.iHue						= m_iHue;
	s.iSaturation				= m_iSaturation;

	s.nUpdaterAutoCheck = m_nUpdaterAutoCheck;
	s.nUpdaterDelay = m_nUpdaterDelay;

	return __super::OnApply();
}

void CPPageMisc::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (*pScrollBar == m_SliBrightness) {
		UpdateData();
		m_iBrightness = m_SliBrightness.GetPos();
	} else if (*pScrollBar == m_SliContrast) {
		UpdateData();
		m_iContrast = m_SliContrast.GetPos();
	} else if (*pScrollBar == m_SliHue) {
		UpdateData();
		m_iHue = m_SliHue.GetPos();
	} else if (*pScrollBar == m_SliSaturation) {
		UpdateData();
		m_iSaturation = m_SliSaturation.GetPos();
	}

	SetModified();

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(m_iBrightness, m_iContrast, m_iHue, m_iSaturation);

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPageMisc::OnBnClickedReset()
{
	UpdateData(FALSE);

	m_iBrightness	= AfxGetMyApp()->GetColorControl (Brightness)->DefaultValue;
	m_iContrast		= AfxGetMyApp()->GetColorControl (Contrast)->DefaultValue;
	m_iHue			= AfxGetMyApp()->GetColorControl (Hue)->DefaultValue;
	m_iSaturation	= AfxGetMyApp()->GetColorControl (Saturation)->DefaultValue;

	m_SliBrightness.SetPos	(m_iBrightness);
	m_SliContrast.SetPos	(m_iContrast);
	m_SliHue.SetPos			(m_iHue);
	m_SliSaturation.SetPos	(m_iSaturation);

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(m_iBrightness, m_iContrast, m_iHue, m_iSaturation);

	SetModified();
}

void CPPageMisc::OnUpdateDelayEditBox(CCmdUI* pCmdUI)
{
	UpdateData();

	pCmdUI->Enable(m_nUpdaterAutoCheck);
}

void CPPageMisc::OnResetSettings()
{
	if (MessageBox(ResStr(IDS_RESET_SETTINGS_WARNING), ResStr(IDS_RESET_SETTINGS), MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SendMessage(WM_CLOSE);

		CString strAppPath;
		GetModuleFileName(NULL, strAppPath.GetBuffer(MAX_PATH), MAX_PATH);
		ShellExecute(NULL, _T("open"), strAppPath, _T("/reset"), NULL, SW_SHOWNORMAL) ;
	}
}

void CPPageMisc::OnExportSettings()
{
	if (GetParent()->GetDlgItem(ID_APPLY_NOW)->IsWindowEnabled()) {
		int ret = MessageBox(ResStr(IDS_EXPORT_SETTINGS_WARNING), ResStr(IDS_EXPORT_SETTINGS), MB_ICONEXCLAMATION | MB_YESNOCANCEL);

		if (ret == IDCANCEL) {
			return;
		} else if (ret == IDYES) {
			GetParent()->PostMessage(PSM_APPLY);
		}
	}

	AfxGetMyApp()->ExportSettings();
}

void CPPageMisc::OnCancel()
{
	AppSettings& s = AfxGetAppSettings();

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetColorControl(s.iBrightness, s.iContrast, s.iHue, s.iSaturation);
	__super::OnCancel();
}
