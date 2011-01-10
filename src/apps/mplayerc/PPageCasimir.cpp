/*
 * $Id$
 *
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
#include "PPageOutput.h"
#include <moreuuids.h>
#include "PPageCasimir.h"
#include <psapi.h>


// CPPageCasimir dialog

IMPLEMENT_DYNAMIC(CPPageCasimir, CPPageBase)
CPPageCasimir::CPPageCasimir()
	: CPPageBase(CPPageCasimir::IDD, CPPageCasimir::IDD)
{
}

CPPageCasimir::~CPPageCasimir()
{
}

void CPPageCasimir::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLI_CONTRAST, m_SliContrast);
	DDX_Control(pDX, IDC_SLI_BRIGHTNESS, m_SliBrightness);
	DDX_Control(pDX, IDC_SLI_HUE, m_SliHue);
	DDX_Control(pDX, IDC_SLI_SATURATION, m_SliSaturation);
}


BEGIN_MESSAGE_MAP(CPPageCasimir, CPPageBase)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnBnClickedReset)
END_MESSAGE_MAP()


// CPPageCasimir message handlers

BOOL CPPageCasimir::OnInitDialog()
{
	COLORPROPERTY_RANGE*	ControlRange;
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	UpdateData(FALSE);

	CreateToolTip();

	ControlRange = AfxGetMyApp()->GetColorControl (Contrast);
	if (ControlRange) {
		m_dContrast		= s.dContrast;
		m_SliContrast.EnableWindow (TRUE);
		m_SliContrast.SetRange		((int)ControlRange->MinValue*100, (int)ControlRange->MaxValue*100);
		m_SliContrast.SetTicFreq	((int)(ControlRange->MaxValue - ControlRange->MinValue) * 10);
		m_SliContrast.SetPos		((int)(m_dContrast*100));
	}

	ControlRange = AfxGetMyApp()->GetColorControl (Brightness);
	if (ControlRange) {
		m_dBrightness = s.dBrightness;
		m_SliBrightness.EnableWindow (TRUE);
		m_SliBrightness.SetRange	((int)ControlRange->MinValue, (int)ControlRange->MaxValue);
		m_SliBrightness.SetTicFreq	((int)(ControlRange->MaxValue - ControlRange->MinValue) / 10);
		m_SliBrightness.SetPos		((int)m_dBrightness);
	}

	ControlRange = AfxGetMyApp()->GetColorControl (Hue);
	if (ControlRange) {
		m_dHue		= s.dHue;
		m_SliHue.EnableWindow (TRUE);
		m_SliHue.SetRange	((int)ControlRange->MinValue, (int)ControlRange->MaxValue);
		m_SliHue.SetTicFreq	((int)(ControlRange->MaxValue - ControlRange->MinValue) / 10);
		m_SliHue.SetPos		((int)m_dHue);
	}

	ControlRange = AfxGetMyApp()->GetColorControl (Saturation);
	if (ControlRange) {
		m_dSaturation	= s.dSaturation;
		m_SliSaturation.EnableWindow (TRUE);
		m_SliSaturation.SetRange	((int)ControlRange->MinValue*100, (int)ControlRange->MaxValue*100);
		m_SliSaturation.SetTicFreq	((int)(ControlRange->MaxValue - ControlRange->MinValue) * 10);
		m_SliSaturation.SetPos		((int)(m_dSaturation*100));
	}

	return TRUE;
}

BOOL CPPageCasimir::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.dBrightness				= m_dBrightness;
	s.dContrast					= m_dContrast;
	s.dHue						= m_dHue;
	s.dSaturation				= m_dSaturation;

	return __super::OnApply();
}


void CPPageCasimir::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(*pScrollBar == m_SliContrast) {
		UpdateData();
		m_dContrast = (float)(m_SliContrast.GetPos()/100.0);
	} else if(*pScrollBar == m_SliBrightness) {
		UpdateData();
		m_dBrightness = (float)m_SliBrightness.GetPos();
	} else if(*pScrollBar == m_SliHue) {
		UpdateData();
		m_dHue = (float)m_SliHue.GetPos();
	} else if(*pScrollBar == m_SliSaturation) {
		UpdateData();
		m_dSaturation = (float)(m_SliSaturation.GetPos()/100.0);
	}

	SetModified();

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetVMR9ColorControl(m_dBrightness, m_dContrast, m_dHue, m_dSaturation);

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPageCasimir::OnBnClickedReset()
{
	UpdateData(FALSE);

	m_dContrast		= AfxGetMyApp()->GetColorControl (Contrast)->DefaultValue;
	m_dBrightness	= AfxGetMyApp()->GetColorControl (Brightness)->DefaultValue;
	m_dHue			= AfxGetMyApp()->GetColorControl (Hue)->DefaultValue;
	m_dSaturation	= AfxGetMyApp()->GetColorControl (Saturation)->DefaultValue;

	m_SliContrast.SetPos	((int)m_dContrast*100);
	m_SliBrightness.SetPos	((int)m_dBrightness);
	m_SliHue.SetPos			((int)m_dHue);
	m_SliSaturation.SetPos	((int)m_dSaturation*100);

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetVMR9ColorControl(m_dBrightness, m_dContrast, m_dHue, m_dSaturation);
}

void CPPageCasimir::OnCancel()
{
	AppSettings& s = AfxGetAppSettings();

	((CMainFrame*)AfxGetMyApp()->GetMainWnd())->SetVMR9ColorControl(s.dBrightness, s.dContrast, s.dHue, s.dSaturation);
	__super::OnCancel();
}
