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

#define LEFT_SPACING					25
#define VERTICAL_SPACING				25

CMpegSplitterSettingsWnd::CMpegSplitterSettingsWnd(void)
{
}

bool CMpegSplitterSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMS);

	m_pMS.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while(pos && !(m_pMS = pUnks.GetNext(pos))) {
		;
	}

	if(!m_pMS) {
		return false;
	}

	return true;
}

void CMpegSplitterSettingsWnd::OnDisconnect()
{
	m_pMS.Release();
}

bool CMpegSplitterSettingsWnd::OnActivate()
{
	int		nPosY	= 10;

	m_grpDefault.Create (ResStr(IDS_OPTIONS_CAPTION), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  nPosY, 350, nPosY+300), this, (UINT)IDC_STATIC);
	nPosY += VERTICAL_SPACING;

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

	if(m_pMS) {
		//m_pMS->Apply();
	}

	return true;
}


BEGIN_MESSAGE_MAP(CMpegSplitterSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
