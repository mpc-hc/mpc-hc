/*
 * (C) 2003-2006 Gabest
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
#include "resource.h"
#include "MpaDecSettingsWnd.h"
#include "../../../DSUtil/DSUtil.h"


// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

//
// CMpaDecSettingsWnd
//

CMpaDecSettingsWnd::CMpaDecSettingsWnd()
{
}

bool CMpaDecSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    ASSERT(!m_pMDF);

    m_pMDF.Release();

    POSITION pos = pUnks.GetHeadPosition();
    while (pos && !(m_pMDF = pUnks.GetNext(pos))) {
        ;
    }

    if (!m_pMDF) {
        return false;
    }

    m_outputformat = m_pMDF->GetSampleFormat();
    m_mixer        = m_pMDF->GetMixer();
    m_mixer_layout = m_pMDF->GetMixerLayout();
    m_drc          = m_pMDF->GetDynamicRangeControl();
    m_spdif_ac3    = m_pMDF->GetSPDIF(IMpaDecFilter::ac3);
    m_spdif_dts    = m_pMDF->GetSPDIF(IMpaDecFilter::dts);

    return true;
}

void CMpaDecSettingsWnd::OnDisconnect()
{
    m_pMDF.Release();
}

bool CMpaDecSettingsWnd::OnActivate()
{
    DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
    CRect r;
    CPoint p(10, 10);

    m_outputformat_static.Create(ResStr(IDS_MPADEC_SAMPLE_FMT), WS_VISIBLE | WS_CHILD, CRect(p, CSize(IPP_SCALE(120), m_fontheight)), this);
    m_outputformat_combo.Create(dwStyle | CBS_DROPDOWNLIST, CRect(p + CPoint(IPP_SCALE(125), -4), CSize(IPP_SCALE(80), 200)), this, IDC_PP_COMBO1);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("PCM 16 Bit")), SF_PCM16);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("PCM 24 Bit")), SF_PCM24);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("PCM 32 Bit")), SF_PCM32);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("IEEE Float")), SF_FLOAT);
    m_outputformat_combo.SetCurSel(0);
    for (int i = 0; i < m_outputformat_combo.GetCount(); i++) {
        if ((int)m_outputformat_combo.GetItemData(i) == m_outputformat) {
            m_outputformat_combo.SetCurSel(i);
        }
    }
    p.y += IPP_SCALE(25);

    m_drc_check.Create(ResStr(IDS_MPADEC_DRC), dwStyle | BS_AUTOCHECKBOX, CRect(p, CSize(IPP_SCALE(205), m_fontheight)), this, IDC_PP_CHECK2);
    m_drc_check.SetCheck(m_drc);
    p.y += IPP_SCALE(30);

    m_mixer_group.Create(_T(""), WS_VISIBLE | WS_CHILD | BS_GROUPBOX, CRect(p + CPoint(-5, 0), CSize(IPP_SCALE(215), IPP_SCALE(45))), this, (UINT)IDC_STATIC);
    m_mixer_check.Create(ResStr(IDS_MPADEC_MIXER), dwStyle | BS_AUTOCHECKBOX, CRect(p, CSize(IPP_SCALE(60), m_fontheight)), this, IDC_PP_CHECK1);
    m_mixer_check.SetCheck(m_mixer);
    p.y += IPP_SCALE(20);
    m_mixer_layout_static.Create(ResStr(IDS_MPADEC_MIX_SPEAKERS), WS_VISIBLE | WS_CHILD, CRect(p, CSize(IPP_SCALE(120), m_fontheight)), this);
    m_mixer_layout_combo.Create(dwStyle | CBS_DROPDOWNLIST, CRect(p + CPoint(IPP_SCALE(125), -4), CSize(IPP_SCALE(80), 200)), this, IDC_PP_COMBO2);
    m_mixer_layout_combo.SetItemData(m_mixer_layout_combo.AddString(ResStr(IDS_MPADEC_MONO)),   SPK_MONO);
    m_mixer_layout_combo.SetItemData(m_mixer_layout_combo.AddString(ResStr(IDS_MPADEC_STEREO)), SPK_STEREO);
    m_mixer_layout_combo.SetItemData(m_mixer_layout_combo.AddString(_T("4.0")), SPK_4_0);
    m_mixer_layout_combo.SetItemData(m_mixer_layout_combo.AddString(_T("5.1")), SPK_5_1);
    m_mixer_layout_combo.SetItemData(m_mixer_layout_combo.AddString(_T("7.1")), SPK_7_1);
    for (int i = 0; i < m_mixer_layout_combo.GetCount(); i++) {
        if ((int)m_mixer_layout_combo.GetItemData(i) == m_mixer_layout) {
            m_mixer_layout_combo.SetCurSel(i);
            break;
        }
    }
    m_mixer_layout_combo.GetWindowRect(r);
    ScreenToClient(r);
    p.y += IPP_SCALE(35);

    m_spdif_group.Create(ResStr(IDS_MPADEC_SPDIF), WS_VISIBLE | WS_CHILD | BS_GROUPBOX, CRect(p + CPoint(-5, 0), CSize(IPP_SCALE(215), IPP_SCALE(40))), this, (UINT)IDC_STATIC);
    p.y += IPP_SCALE(20);
    m_spdif_ac3_check.Create(_T("AC-3"), dwStyle | BS_AUTOCHECKBOX, CRect(p, CSize(IPP_SCALE(45), m_fontheight)), this, IDC_PP_CHECK3);
    m_spdif_ac3_check.SetCheck(m_spdif_ac3);
    m_spdif_dts_check.Create(_T("DTS"), dwStyle | BS_AUTOCHECKBOX, CRect(p + CPoint(IPP_SCALE(50), 0), CSize(IPP_SCALE(45), m_fontheight)), this, IDC_PP_CHECK4);
    m_spdif_dts_check.SetCheck(m_spdif_dts);

    for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
        pWnd->SetFont(&m_font, FALSE);
    }

    return true;
}

void CMpaDecSettingsWnd::OnDeactivate()
{
    m_outputformat = (int)m_outputformat_combo.GetItemData(m_outputformat_combo.GetCurSel());
    m_mixer        = !!m_mixer_check.GetCheck();
    m_mixer_layout = (int)m_mixer_layout_combo.GetItemData(m_mixer_layout_combo.GetCurSel());
    m_drc          = !!m_drc_check.GetCheck();
    m_spdif_ac3    = !!m_spdif_ac3_check.GetCheck();
    m_spdif_dts    = !!m_spdif_dts_check.GetCheck();
}

bool CMpaDecSettingsWnd::OnApply()
{
    OnDeactivate();

    if (m_pMDF) {
        m_pMDF->SetSampleFormat((MPCSampleFormat)m_outputformat);
        m_pMDF->SetMixer(m_mixer);
        m_pMDF->SetMixerLayout(m_mixer_layout);
        m_pMDF->SetDynamicRangeControl(m_drc);
        m_pMDF->SetSPDIF(IMpaDecFilter::ac3, m_spdif_ac3);
        m_pMDF->SetSPDIF(IMpaDecFilter::dts, m_spdif_dts);

        m_pMDF->SaveSettings();
    }

    return true;
}

BEGIN_MESSAGE_MAP(CMpaDecSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
