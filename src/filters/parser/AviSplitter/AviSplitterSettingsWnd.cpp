/*
 * (C) 2012 see Authors.txt
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
#include "AviSplitterSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"
#include "resource.h"
#include "../../../mpc-hc/InternalFiltersConfig.h"


CAviSplitterSettingsWnd::CAviSplitterSettingsWnd()
{
}

bool CAviSplitterSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    ASSERT(!m_pASF);

    m_pASF.Release();

    POSITION pos = pUnks.GetHeadPosition();
    while (pos && !(m_pASF = pUnks.GetNext(pos))) {
        ;
    }

    if (!m_pASF) {
        return false;
    }

    return true;
}

void CAviSplitterSettingsWnd::OnDisconnect()
{
    m_pASF.Release();
}

bool CAviSplitterSettingsWnd::OnActivate()
{
    ASSERT(IPP_FONTSIZE == 13);
    const int h20 = IPP_SCALE(20);
    DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
    CPoint p(10, 10);

    m_cbNonInterleavedFilesSupport.Create(ResStr(IDS_AVISPLITTER_NON_INTERLEAVED), dwStyle | BS_AUTOCHECKBOX | BS_LEFTTEXT, CRect(p, CSize(IPP_SCALE(260), m_fontheight)), this, IDC_PP_NON_INTERLEAVED_FILES_SUPPORT);
    p.y += h20;

    if (m_pASF) {
        m_cbNonInterleavedFilesSupport.SetCheck(m_pASF->GetNonInterleavedFilesSupport());
    }
    m_cbNonInterleavedFilesSupport.EnableWindow(FALSE);

    for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
        pWnd->SetFont(&m_font, FALSE);
    }

    return true;
}

void CAviSplitterSettingsWnd::OnDeactivate()
{
}

bool CAviSplitterSettingsWnd::OnApply()
{
    OnDeactivate();

    if (m_pASF) {
        m_pASF->SetNonInterleavedFilesSupport(m_cbNonInterleavedFilesSupport.GetCheck());

        m_pASF->Apply();
    }

    return true;
}

BEGIN_MESSAGE_MAP(CAviSplitterSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
