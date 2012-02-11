/*
 *
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
#include "MpegSplitterSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"
#include "resource.h"
#include "../../../apps/mplayerc/InternalFiltersConfig.h"

// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

#define LEFT_SPACING					25
#define VERTICAL_SPACING				25

CMpegSplitterSettingsWnd::CMpegSplitterSettingsWnd(void)
{
}

bool CMpegSplitterSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMSF);

	m_pMSF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while (pos && !(m_pMSF = pUnks.GetNext(pos))) {
		;
	}

	if (!m_pMSF) {
		return false;
	}

	return true;
}

void CMpegSplitterSettingsWnd::OnDisconnect()
{
	m_pMSF.Release();
}

bool CMpegSplitterSettingsWnd::OnActivate()
{
	int		nPosY	= 10;

	m_grpDefault.Create (ResStr(IDS_OPTIONS_CAPTION), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  nPosY, 320, nPosY+285), this, (UINT)IDC_STATIC);

	nPosY += VERTICAL_SPACING;
	m_cbFastStreamChange.Create (ResStr(IDS_MPEGSPLITTER_FSTREAM_CHANGE), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 305, nPosY+15), this, IDC_PP_FAST_STREAM_SELECT);

	nPosY += VERTICAL_SPACING;
	m_cbForcedSub.Create (ResStr(IDS_MPEGSPLITTER_SUB_FORCING), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 305, nPosY+15), this, IDC_PP_SUBTITLE_FORCED);

	nPosY += VERTICAL_SPACING;
	m_cbTrackPriority.Create (ResStr(IDS_MPEGSPLITTER_TRACKS_ORDER), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 305, nPosY+15), this, IDC_PP_TRACK_PRIORITY);

	nPosY += VERTICAL_SPACING;
	m_txtAudioLanguageOrder.Create (ResStr(IDS_MPEGSPLITTER_LANG_ORDER), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 200, nPosY+15), this, (UINT)IDC_STATIC);
	nPosY += 15;
	m_edtAudioLanguageOrder.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect (LEFT_SPACING,  nPosY, 305, nPosY+20), this, IDC_PP_AUDIO_LANGUAGE_ORDER);

	nPosY += VERTICAL_SPACING;
	m_txtSubtitlesLanguageOrder.Create (ResStr(IDS_MPEGSPLITTER_SUB_ORDER), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 200, nPosY+15), this, (UINT)IDC_STATIC);
	nPosY += 15;
	m_edtSubtitlesLanguageOrder.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect (LEFT_SPACING,  nPosY, 305, nPosY+20), this, IDC_PP_SUBTITLES_LANGUAGE_ORDER);

	nPosY += VERTICAL_SPACING;
	m_txtVC1_GuidFlag.Create (ResStr(IDS_MPEGSPLITTER_VC1_GUIDFLAG), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 200, nPosY+15), this, (UINT)IDC_STATIC);
	nPosY += 15;
	m_cbVC1_GuidFlag.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (LEFT_SPACING,  nPosY, 305, nPosY+20), this, IDC_PP_VC1_GUIDFLAG);
	m_cbVC1_GuidFlag.AddString (_T("Default"));
	m_cbVC1_GuidFlag.AddString (_T("Cyberlink VC-1 Decoder"));
	m_cbVC1_GuidFlag.AddString (_T("ArcSoft VC-1 Decoder"));

	SetClassLongPtr(GetDlgItem(IDC_PP_VC1_GUIDFLAG)->m_hWnd, GCLP_HCURSOR, (long) AfxGetApp()->LoadStandardCursor(IDC_HAND));

	nPosY += VERTICAL_SPACING + 5;
	m_grpTrueHD.Create (ResStr(IDS_MPEGSPLITTER_TRUEHD_OUTPUT), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (LEFT_SPACING,  nPosY, 305, nPosY+50), this, (UINT)IDC_STATIC);

	nPosY += VERTICAL_SPACING - 5;
	m_cbTrueHD.Create (_T("TrueHD"), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTORADIOBUTTON|BS_TOP|BS_MULTILINE|WS_GROUP, CRect (LEFT_SPACING + 15, nPosY, LEFT_SPACING + 15 + 80, nPosY+20), this, IDC_PP_TRUEHD);
	m_cbAC3Core.Create (_T("AC-3 core"), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTORADIOBUTTON|BS_TOP|BS_MULTILINE, CRect (LEFT_SPACING + 15 + 85, nPosY, LEFT_SPACING + 15 + 165, nPosY+20), this, IDC_PP_AC3CORE);
	m_cbAsIs.Create (ResStr(IDS_MPEGSPLITTER_THD_NOSPLIT), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTORADIOBUTTON|BS_TOP|BS_MULTILINE, CRect (LEFT_SPACING + 15 + 170, nPosY, LEFT_SPACING + 15 + 260, nPosY+20), this, IDC_PP_ASIS);

	if (m_pMSF) {
		m_cbFastStreamChange.SetCheck(m_pMSF->GetFastStreamChange());
		m_cbForcedSub.SetCheck(m_pMSF->GetForcedSub());
		m_cbTrackPriority.SetCheck(m_pMSF->GetTrackPriority());
		m_edtAudioLanguageOrder.SetWindowText(m_pMSF->GetAudioLanguageOrder());
		m_edtSubtitlesLanguageOrder.SetWindowText(m_pMSF->GetSubtitlesLanguageOrder());
		m_cbVC1_GuidFlag.SetCurSel(m_pMSF->GetVC1_GuidFlag() - 1);
		m_cbTrueHD.SetCheck	(m_pMSF->GetTrueHD() == 0);
		m_cbAC3Core.SetCheck(m_pMSF->GetTrueHD() == 1);
		m_cbAsIs.SetCheck		(!m_cbTrueHD.GetCheck() && !m_cbAC3Core.GetCheck());
	}

#ifndef REGISTER_FILTER
	m_edtAudioLanguageOrder.EnableWindow(FALSE);
	m_edtSubtitlesLanguageOrder.EnableWindow(FALSE);
#endif


	for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
		pWnd->SetFont(&m_font, FALSE);
	}

	return true;
}

void CMpegSplitterSettingsWnd::OnDeactivate()
{
}

bool CMpegSplitterSettingsWnd::OnApply()
{
	OnDeactivate();

	if (m_pMSF) {
		m_pMSF->SetFastStreamChange(m_cbFastStreamChange.GetCheck());
		m_pMSF->SetForcedSub(m_cbForcedSub.GetCheck());
		m_pMSF->SetTrackPriority(m_cbTrackPriority.GetCheck());
		m_pMSF->SetVC1_GuidFlag(m_cbVC1_GuidFlag.GetCurSel() + 1);
		m_pMSF->SetTrueHD(m_cbTrueHD.GetCheck() ? 0 : m_cbAC3Core.GetCheck() ? 1 : 2);

#ifdef REGISTER_FILTER
		CString str = _T("");
		m_edtAudioLanguageOrder.GetWindowText(str);
		m_pMSF->SetAudioLanguageOrder(str.GetBuffer());
		m_edtSubtitlesLanguageOrder.GetWindowText(str);
		m_pMSF->SetSubtitlesLanguageOrder(str.GetBuffer());
#endif
		m_pMSF->Apply();
	}

	return true;
}

BEGIN_MESSAGE_MAP(CMpegSplitterSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
