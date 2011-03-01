/*
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
#include "MpegSplitterSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"
#include "resource.h"
#include "../../../apps/mplayerc/internal_filter_config.h"

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
	while(pos && !(m_pMSF = pUnks.GetNext(pos))) {
		;
	}

	if(!m_pMSF) {
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

	m_grpDefault.Create (ResStr(IDS_OPTIONS_CAPTION), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  nPosY, 320, nPosY+230), this, (UINT)IDC_STATIC);
	
	nPosY += VERTICAL_SPACING;
	m_cbFastStreamChange.Create (ResStr(IDS_MPEGSPLITTER_FSTREAM_CHANGE), WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 305, nPosY+15), this, IDC_PP_FAST_STREAM_SELECT);
	
	nPosY += VERTICAL_SPACING;
	m_txtAudioLanguageOrder.Create (ResStr(IDS_MPEGSPLITTER_LANG_ORDER), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 200, nPosY+15), this, (UINT)IDC_STATIC);
	nPosY += 15;
	m_edtAudioLanguageOrder.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect (LEFT_SPACING,  nPosY, 305, nPosY+20), this, IDC_PP_AUDIO_LANGUAGE_ORDER);

	nPosY += VERTICAL_SPACING;
	m_txtSubtitlesLanguageOrder.Create (ResStr(IDS_MPEGSPLITTER_SUB_ORDER), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 200, nPosY+15), this, (UINT)IDC_STATIC);
	nPosY += 15;
	m_edtSubtitlesLanguageOrder.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect (LEFT_SPACING,  nPosY, 305, nPosY+20), this, IDC_PP_SUBTITLES_LANGUAGE_ORDER);

	if(m_pMSF) {
		m_cbFastStreamChange.SetCheck(m_pMSF->GetFastStreamChange());
		m_edtAudioLanguageOrder.SetWindowText(m_pMSF->GetAudioLanguageOrder());
		m_edtSubtitlesLanguageOrder.SetWindowText(m_pMSF->GetSubtitlesLanguageOrder());
	}

#ifndef REGISTER_FILTER
	m_edtAudioLanguageOrder.EnableWindow(FALSE);
	m_edtSubtitlesLanguageOrder.EnableWindow(FALSE);
#endif

	for(CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
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

	if(m_pMSF) {
		m_pMSF->SetFastStreamChange(m_cbFastStreamChange.GetCheck());

#ifdef REGISTER_FILTER		
		CString str = _T("");
		m_edtAudioLanguageOrder.GetWindowText(str);
		m_pMSF->SetAudioLanguageOrder(str);
		m_edtSubtitlesLanguageOrder.GetWindowText(str);
		m_pMSF->SetSubtitlesLanguageOrder(str);
#endif
		m_pMSF->Apply();
	}

	return true;
}

BEGIN_MESSAGE_MAP(CMpegSplitterSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
