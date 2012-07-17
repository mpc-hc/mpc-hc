/*
 * $Id$
 *
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

static TCHAR m_strDecodeToSpeaker[50];

CMpaDecSettingsWnd::CMpaDecSettingsWnd()
{
    wcscpy_s(m_strDecodeToSpeaker, ResStr(IDS_MPADECSETTINGSWND_5));
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
    m_spkcfg_ac3 = m_pMDF->GetSpeakerConfig(IMpaDecFilter::ac3);
    m_drc_ac3 = m_pMDF->GetDynamicRangeControl(IMpaDecFilter::ac3);
    m_spkcfg_dts = m_pMDF->GetSpeakerConfig(IMpaDecFilter::dts);
    m_drc_dts = m_pMDF->GetDynamicRangeControl(IMpaDecFilter::dts);
    m_spdif_ac3 = m_pMDF->GetSPDIF(IMpaDecFilter::ac3);
    m_spdif_dts = m_pMDF->GetSPDIF(IMpaDecFilter::dts);

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

    m_outputformat_static.Create(ResStr(IDS_MPADECSETTINGSWND_0), dwStyle, CRect(p, CSize(120, m_fontheight)), this);

    p.y += m_fontheight + 5;

    m_outputformat_combo.Create(dwStyle | CBS_DROPDOWNLIST, CRect(p + CSize(10, 0), CSize(100, 200)), this, IDC_PP_COMBO1);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("PCM 16 Bit")), SF_PCM16);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("PCM 24 Bit")), SF_PCM24);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("PCM 32 Bit")), SF_PCM32);
    m_outputformat_combo.SetItemData(m_outputformat_combo.AddString(_T("IEEE Float")), SF_FLOAT32);
    m_outputformat_combo.SetCurSel(0);

    for (int i = 0; i < m_outputformat_combo.GetCount(); i++)
        if ((int)m_outputformat_combo.GetItemData(i) == m_outputformat) {
            m_outputformat_combo.SetCurSel(i);
        }

    p.y += 30;

    m_ac3spkcfg_static.Create(ResStr(IDS_MPADECSETTINGSWND_1), dwStyle, CRect(p, CSize(220, m_fontheight)), this);

    p.y += m_fontheight + 5;

    m_ac3spkcfg_combo.Create(dwStyle | CBS_DROPDOWNLIST, CRect(p + CSize(150, 0), CSize(100, 200)), this, IDC_PP_COMBO2);
    m_ac3spkcfg_combo.SetItemData(m_ac3spkcfg_combo.AddString(_T("As is")),  SPK_ASIS);
    m_ac3spkcfg_combo.SetItemData(m_ac3spkcfg_combo.AddString(_T("Mono")),   SPK_MONO);
    m_ac3spkcfg_combo.SetItemData(m_ac3spkcfg_combo.AddString(_T("Stereo")), SPK_STEREO);

    for (int i = 0, sel = m_spkcfg_ac3 & SPK_MASK; i < m_ac3spkcfg_combo.GetCount(); i++)
        if ((int)m_ac3spkcfg_combo.GetItemData(i) == sel) {
            m_ac3spkcfg_combo.SetCurSel(i);
            break;
        }

    m_ac3spkcfg_combo.GetWindowRect(r);
    ScreenToClient(r);

    for (int i = 0, h = max(20, m_fontheight) + 1; i < _countof(m_ac3spkcfg_radio); i++, p.y += h) {
        static const TCHAR* labels[] = {m_strDecodeToSpeaker, _T("SPDIF")};
        m_ac3spkcfg_radio[i].Create(labels[i], dwStyle | BS_AUTORADIOBUTTON | (i == 0 ? WS_GROUP : 0), CRect(p + CPoint(10, 0), CSize(140, h)), this, IDC_PP_RADIO1 + i);
    }

    CheckRadioButton(IDC_PP_RADIO1, IDC_PP_RADIO2, m_spdif_ac3 ? IDC_PP_RADIO2 : IDC_PP_RADIO1);

    p.y += 5;

    m_ac3spkcfg_check.Create(ResStr(IDS_MPA_DYNRANGE), dwStyle | BS_AUTOCHECKBOX, CRect(p + CPoint(10, 0), CSize(205, m_fontheight)), this, IDC_PP_CHECK2);
    m_ac3spkcfg_check.SetCheck(m_drc_ac3);

    p.y += m_fontheight + 10;

    m_dtsspkcfg_static.Create(ResStr(IDS_MPADECSETTINGSWND_7), dwStyle, CRect(p, CSize(120, m_fontheight)), this);

    p.y += m_fontheight + 5;

    m_dtsspkcfg_combo.Create(dwStyle | CBS_DROPDOWNLIST, CRect(p + CSize(150, 0), CSize(100, 200)), this, IDC_PP_COMBO3);
    m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(_T("As is")),           SPK_ASIS);
    //m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(_T("Mono")),            SPK_MONO);
    m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(_T("Stereo")),          SPK_STEREO);
    //m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(ResStr(IDS_MPA_3F)),    SPK_3F);
    //m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(ResStr(IDS_MPA_2F_1R)), SPK_2F1R);
    //m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(ResStr(IDS_MPA_3F_1R)), SPK_3F1R);
    //m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(ResStr(IDS_MPA_2F_2R)), SPK_2F2R);
    //m_dtsspkcfg_combo.SetItemData(m_dtsspkcfg_combo.AddString(ResStr(IDS_MPA_3F_2R)), SPK_3F2R);

    for (int i = 0, sel = m_spkcfg_dts & SPK_MASK; i < m_dtsspkcfg_combo.GetCount(); i++)
        if ((int)m_dtsspkcfg_combo.GetItemData(i) == sel) {
            m_dtsspkcfg_combo.SetCurSel(i);
            break;
        }

    m_dtsspkcfg_combo.GetWindowRect(r);
    ScreenToClient(r);

    //m_dtslfe_check.Create(_T("LFE"), dwStyle | BS_AUTOCHECKBOX, CRect(CPoint(r.left, r.bottom + 3), CSize(50, m_fontheight)), this, IDC_PP_CHECK3);
    //m_dtslfe_check.SetCheck(!!(m_spkcfg_dts & SPK_LFE));

    for (int i = 0, h = max(20, m_fontheight) + 1; i < _countof(m_dtsspkcfg_radio); i++, p.y += h) {
        static const TCHAR* labels[] = {m_strDecodeToSpeaker, _T("SPDIF")};
        m_dtsspkcfg_radio[i].Create(labels[i], dwStyle | BS_AUTORADIOBUTTON | (i == 0 ? WS_GROUP : 0), CRect(p + CPoint(10, 0), CSize(140, h)), this, IDC_PP_RADIO3 + i);
    }

    CheckRadioButton(IDC_PP_RADIO3, IDC_PP_RADIO4, m_spdif_dts ? IDC_PP_RADIO4 : IDC_PP_RADIO3);

    p.y += 5;

    m_dtsspkcfg_check.Create(ResStr(IDS_MPA_DYNRANGE), dwStyle | WS_DISABLED | BS_AUTOCHECKBOX, CRect(p + CPoint(10, 0), CSize(205, m_fontheight)), this, IDC_PP_CHECK4);
    m_dtsspkcfg_check.SetCheck(m_drc_dts);

    for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
        pWnd->SetFont(&m_font, FALSE);
    }

    return true;
}

void CMpaDecSettingsWnd::OnDeactivate()
{
    m_outputformat = (int)m_outputformat_combo.GetItemData(m_outputformat_combo.GetCurSel());
    m_spkcfg_ac3 = (int)m_ac3spkcfg_combo.GetItemData(m_ac3spkcfg_combo.GetCurSel());
    m_drc_ac3 = !!m_ac3spkcfg_check.GetCheck();
    m_spkcfg_dts = (int)m_dtsspkcfg_combo.GetItemData(m_dtsspkcfg_combo.GetCurSel());
    //if (!!m_dtslfe_check.GetCheck()) {
    //    m_spkcfg_dts |= SPK_LFE;
    //}
    m_drc_dts = !!m_dtsspkcfg_check.GetCheck();
    m_spdif_ac3 = !!IsDlgButtonChecked(IDC_PP_RADIO2);
    m_spdif_dts = !!IsDlgButtonChecked(IDC_PP_RADIO4);
}

bool CMpaDecSettingsWnd::OnApply()
{
    OnDeactivate();

    if (m_pMDF) {
        m_pMDF->SetSampleFormat((MPCSampleFormat)m_outputformat);
        m_pMDF->SetSpeakerConfig(IMpaDecFilter::ac3, m_spkcfg_ac3);
        m_pMDF->SetDynamicRangeControl(IMpaDecFilter::ac3, m_drc_ac3);
        m_pMDF->SetSpeakerConfig(IMpaDecFilter::dts, m_spkcfg_dts);
        m_pMDF->SetDynamicRangeControl(IMpaDecFilter::dts, m_drc_dts);
        m_pMDF->SetSPDIF(IMpaDecFilter::ac3, m_spdif_ac3);
        m_pMDF->SetSPDIF(IMpaDecFilter::dts, m_spdif_dts);

        m_pMDF->SaveSettings();
    }

    return true;
}

BEGIN_MESSAGE_MAP(CMpaDecSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
