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
#include "resource.h"
#include "MpcAudioRendererSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"

#include "../../../apps/mplayerc/internal_filter_config.h"

// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

#define LEFT_SPACING					25
#define VERTICAL_SPACING				25


CMpcAudioRendererSettingsWnd::CMpcAudioRendererSettingsWnd(void)
{
}


bool CMpcAudioRendererSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMAR);

	m_pMAR.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while(pos && !(m_pMAR = pUnks.GetNext(pos))) {
		;
	}

	if(!m_pMAR) {
		return false;
	}

	return true;
}

void CMpcAudioRendererSettingsWnd::OnDisconnect()
{
	m_pMAR.Release();
}

bool CMpcAudioRendererSettingsWnd::OnActivate()
{
	int		nPosY	= 10;
	GUID*	DxvaGui = NULL;

	m_grpDefault.Create (_T(""), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  nPosY, 350, nPosY+300), this, (UINT)IDC_STATIC);
	nPosY += VERTICAL_SPACING;
	m_cbWasapiMode.Create (ResStr (IDS_ARS_WASAPI_MODE), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 315, nPosY+15), this, IDC_PP_WASAPI_MODE);
	nPosY += VERTICAL_SPACING;
	m_cbMuteFastForward.Create (ResStr (IDS_ARS_MUTE_FAST_FORWARD), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|BS_LEFTTEXT, CRect (LEFT_SPACING,  nPosY, 315, nPosY+15), this, IDC_PP_MUTE_FAST_FORWARD);

	m_cbWasapiMode.SetCheck(m_pMAR->GetWasapiMode());
	m_cbMuteFastForward.SetCheck(m_pMAR->GetMuteFastForward());


	for(CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
		pWnd->SetFont(&m_font, FALSE);
	}

	return true;
}

void CMpcAudioRendererSettingsWnd::OnDeactivate()
{
}

bool CMpcAudioRendererSettingsWnd::OnApply()
{
	OnDeactivate();

	if(m_pMAR) {
		m_pMAR->SetWasapiMode(m_cbWasapiMode.GetCheck());
		m_pMAR->SetMuteFastForward(m_cbMuteFastForward.GetCheck());
		m_pMAR->Apply();
	}

	return true;
}


BEGIN_MESSAGE_MAP(CMpcAudioRendererSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()


