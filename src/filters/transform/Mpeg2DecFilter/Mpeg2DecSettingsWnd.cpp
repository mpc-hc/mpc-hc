/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

#include "stdafx.h"
#include "resource.h"
#include "Mpeg2DecSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"

// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

//
// CMpeg2DecSettingsWnd
//

static TCHAR m_strBrightness[100];
static TCHAR m_strContrast[100];
static TCHAR m_strHue[100];
static TCHAR m_strSaturation[100];

CMpeg2DecSettingsWnd::CMpeg2DecSettingsWnd()
{
	wcscpy (m_strBrightness, ResStr(IDS_MPEG2_BRIGHTNESS));
	wcscpy (m_strContrast,	 ResStr(IDS_MPEG2_CONTRAST));
	wcscpy (m_strHue,		 ResStr(IDS_MPEG2_HUE));
	wcscpy (m_strSaturation, ResStr(IDS_MPEG2_SATURATION));
}

bool CMpeg2DecSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pM2DF);

	m_pM2DF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while(pos && !(m_pM2DF = pUnks.GetNext(pos))) {
		;
	}

	if(!m_pM2DF) {
		return false;
	}

	m_ditype = m_pM2DF->GetDeinterlaceMethod();
	m_procamp[0] = m_pM2DF->GetBrightness();
	m_procamp[1] = m_pM2DF->GetContrast();
	m_procamp[2] = m_pM2DF->GetHue();
	m_procamp[3] = m_pM2DF->GetSaturation();
	m_forcedsubs = m_pM2DF->IsForcedSubtitlesEnabled();
	m_planaryuv = m_pM2DF->IsPlanarYUVEnabled();
	m_interlaced = m_pM2DF->IsInterlacedEnabled();
	m_readARFromStream = m_pM2DF->IsReadARFromStreamEnabled();

	return true;
}

void CMpeg2DecSettingsWnd::OnDisconnect()
{
	m_pM2DF.Release();
}

bool CMpeg2DecSettingsWnd::OnActivate()
{
	DWORD dwStyle = WS_VISIBLE|WS_CHILD|WS_TABSTOP;

	CPoint p(10, 10);

	m_planaryuv_check.Create(ResStr(IDS_MPEG2DECSETTINGSWND_0), dwStyle|BS_AUTOCHECKBOX, CRect(p, CSize(300, m_fontheight)), this, IDC_PP_CHECK1);
	m_planaryuv_check.SetCheck(m_planaryuv ? BST_CHECKED : BST_UNCHECKED);
	p.y += m_fontheight + 5;

	m_interlaced_check.Create(ResStr(IDS_MPEG2DECSETTINGSWND_1), dwStyle|BS_AUTOCHECKBOX, CRect(p, CSize(300, m_fontheight)), this, IDC_PP_CHECK2);
	m_interlaced_check.SetCheck(m_interlaced ? BST_CHECKED : BST_UNCHECKED);
	p.y += m_fontheight + 5;

	m_forcedsubs_check.Create(ResStr(IDS_MPEG2DECSETTINGSWND_2), dwStyle|BS_AUTOCHECKBOX, CRect(p, CSize(300, m_fontheight)), this, IDC_PP_CHECK3);
	m_forcedsubs_check.SetCheck(m_forcedsubs ? BST_CHECKED : BST_UNCHECKED);
	p.y += m_fontheight + 5;

	m_readARFromStream_check.Create(ResStr(IDS_MPEG2DECSETTINGSWND_3), dwStyle|BS_AUTOCHECKBOX, CRect(p, CSize(300, m_fontheight)), this, IDC_PP_CHECK4);
	m_readARFromStream_check.SetCheck(m_readARFromStream ? BST_CHECKED : BST_UNCHECKED);
	p.y += m_fontheight + 5;

	p.y += 10;

	m_ditype_static.Create(ResStr(IDS_MPEG2_DEINTERLACING), dwStyle, CRect(p, CSize(100, m_fontheight)), this);
	m_ditype_combo.Create(dwStyle|CBS_DROPDOWNLIST, CRect(p + CSize(110, -3), CSize(100, 200)), this, IDC_PP_COMBO1);
	m_ditype_combo.SetItemData(m_ditype_combo.AddString(_T("Auto")), (DWORD)DIAuto);
	m_ditype_combo.SetItemData(m_ditype_combo.AddString(_T("Weave")), (DWORD)DIWeave);
	m_ditype_combo.SetItemData(m_ditype_combo.AddString(_T("Blend")), (DWORD)DIBlend);
	m_ditype_combo.SetItemData(m_ditype_combo.AddString(_T("Bob")), (DWORD)DIBob);
	m_ditype_combo.SetItemData(m_ditype_combo.AddString(_T("Field Shift")), (DWORD)DIFieldShift);
	m_ditype_combo.SetItemData(m_ditype_combo.AddString(_T("ELA")), (DWORD)DIELA);
	m_ditype_combo.SetCurSel(0);
	for(int i = 0; i < m_ditype_combo.GetCount(); i++)
		if((int)m_ditype_combo.GetItemData(i) == m_ditype) {
			m_ditype_combo.SetCurSel(i);
		}

	m_ditype_combo.EnableWindow(!IsDlgButtonChecked(m_interlaced_check.GetDlgCtrlID()));

	p.y += m_fontheight + 20;

	for(int i = 0, h = max(20, m_fontheight)+1; i < countof(m_procamp_slider); i++, p.y += h) {
		static const TCHAR* labels[] = {m_strBrightness, m_strContrast,	m_strHue, m_strSaturation};
		m_procamp_static[i].Create(labels[i], dwStyle, CRect(p, CSize(70, h)), this);
		m_procamp_slider[i].Create(dwStyle, CRect(p + CPoint(80, 0), CSize(201, h)), this, IDC_PP_SLIDER1+i);
		m_procamp_value[i].Create(_T(""), dwStyle, CRect(p + CPoint(280, 0), CSize(40, h)), this);
	}

	m_procamp_slider[0].SetRange(0, 2*128);
	m_procamp_slider[0].SetTic(128);
	m_procamp_slider[0].SetPos((int)(m_procamp[0] + (m_procamp[0] >= 0 ? 0.5f : -0.5f)) + 128);
	m_procamp_slider[1].SetRange(0, 200);
	m_procamp_slider[1].SetTic(100);
	m_procamp_slider[1].SetPos((int)(100*m_procamp[1] + 0.5f));
	m_procamp_slider[2].SetRange(0, 2*180);
	m_procamp_slider[2].SetTic(180);
	m_procamp_slider[2].SetPos((int)(m_procamp[2] + (m_procamp[2] >= 0 ? 0.5f : -0.5f)) + 180);
	m_procamp_slider[3].SetRange(0, 200);
	m_procamp_slider[3].SetTic(100);
	m_procamp_slider[3].SetPos((int)(100*m_procamp[3] + 0.5f));

	p.y += 5;

	m_procamp_tv2pc.Create(_T("TV->PC"), dwStyle, CRect(p + CPoint(50 + 200/2 - 55, 0), CSize(80, 20)), this, IDC_PP_BUTTON1);
	m_procamp_reset.Create(ResStr(IDS_MPEG2_RESET), dwStyle, CRect(p + CPoint(80 + 200/2 + 5, 0), CSize(80, 20)), this, IDC_PP_BUTTON2);

	p.y += 30;

	UpdateProcampValues();

	m_note_static.Create(
		ResStr(IDS_MPEG2DECSETTINGSWND_7) +
		ResStr(IDS_MPEG2DECSETTINGSWND_8),
		dwStyle, CRect(p, CSize(320, m_fontheight * 4)), this);

	for(CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
		pWnd->SetFont(&m_font, FALSE);
	}

	return true;
}

void CMpeg2DecSettingsWnd::OnDeactivate()
{
	m_ditype = (ditype)m_ditype_combo.GetItemData(m_ditype_combo.GetCurSel());
	m_procamp[0] = (float)m_procamp_slider[0].GetPos() - 128;
	m_procamp[1] = (float)m_procamp_slider[1].GetPos() / 100;
	m_procamp[2] = (float)m_procamp_slider[2].GetPos() - 180;
	m_procamp[3] = (float)m_procamp_slider[3].GetPos() / 100;
	m_planaryuv = !!IsDlgButtonChecked(m_planaryuv_check.GetDlgCtrlID());
	m_interlaced = !!IsDlgButtonChecked(m_interlaced_check.GetDlgCtrlID());
	m_forcedsubs = !!IsDlgButtonChecked(m_forcedsubs_check.GetDlgCtrlID());
	m_readARFromStream = !!IsDlgButtonChecked(m_readARFromStream_check.GetDlgCtrlID());
}

bool CMpeg2DecSettingsWnd::OnApply()
{
	OnDeactivate();

	if(m_pM2DF) {
		m_pM2DF->SetDeinterlaceMethod(m_ditype);
		m_pM2DF->SetBrightness(m_procamp[0]);
		m_pM2DF->SetContrast(m_procamp[1]);
		m_pM2DF->SetHue(m_procamp[2]);
		m_pM2DF->SetSaturation(m_procamp[3]);
		m_pM2DF->EnableForcedSubtitles(m_forcedsubs);
		m_pM2DF->EnablePlanarYUV(m_planaryuv);
		m_pM2DF->EnableInterlaced(m_interlaced);
		m_pM2DF->EnableReadARFromStream(m_readARFromStream);
		m_pM2DF->Apply();
	}

	return true;
}

void CMpeg2DecSettingsWnd::UpdateProcampValues()
{
	CString str;

	str.Format(_T("%d"), m_procamp_slider[0].GetPos() - 128);
	m_procamp_value[0].SetWindowText(str);
	str.Format(_T("%d%%"), m_procamp_slider[1].GetPos());
	m_procamp_value[1].SetWindowText(str);
	str.Format(_T("%d"), m_procamp_slider[2].GetPos() - 180);
	m_procamp_value[2].SetWindowText(str);
	str.Format(_T("%d%%"), m_procamp_slider[3].GetPos());
	m_procamp_value[3].SetWindowText(str);
}

BEGIN_MESSAGE_MAP(CMpeg2DecSettingsWnd, CInternalPropertyPageWnd)
	ON_BN_CLICKED(IDC_PP_BUTTON1, OnButtonProcampPc2Tv)
	ON_BN_CLICKED(IDC_PP_BUTTON2, OnButtonProcampReset)
	ON_BN_CLICKED(IDC_PP_CHECK2, OnButtonInterlaced)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

void CMpeg2DecSettingsWnd::OnButtonProcampPc2Tv()
{
	m_procamp_slider[0].SetPos(128 - 16);
	m_procamp_slider[1].SetPos(100 * 255/(235-16));

	UpdateProcampValues();
}

void CMpeg2DecSettingsWnd::OnButtonProcampReset()
{
	m_procamp_slider[0].SetPos(128);
	m_procamp_slider[1].SetPos(100);
	m_procamp_slider[2].SetPos(180);
	m_procamp_slider[3].SetPos(100);

	UpdateProcampValues();
}

void CMpeg2DecSettingsWnd::OnButtonInterlaced()
{
	m_ditype_combo.EnableWindow(!IsDlgButtonChecked(m_interlaced_check.GetDlgCtrlID()));
}

void CMpeg2DecSettingsWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	UpdateProcampValues();

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}
