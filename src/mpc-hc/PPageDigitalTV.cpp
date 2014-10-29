/*
* (C) 2009-2013 see Authors.txt
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

// PPageCapture.cpp : implementation file
//

#include "stdafx.h"
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdaiface.h>

#include "mplayerc.h"
#include "PPageDigitalTV.h"
#include "DSUtil.h"


// CPPageCapture dialog

IMPLEMENT_DYNAMIC(CPPageDigitalTV, CPPageBase)

CPPageDigitalTV::CPPageDigitalTV()
    : CPPageBase(CPPageDigitalTV::IDD, CPPageDigitalTV::IDD)
{
}

CPPageDigitalTV::~CPPageDigitalTV()
{
}

void CPPageDigitalTV::DoDataExchange(CDataExchange* pDX)
{
    CPPageBase::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK1, m_bEnableDVB);
    DDX_Control(pDX, IDC_COMBO4, m_cbDigitalNetworkProvider);
    DDX_Control(pDX, IDC_COMBO5, m_cbDigitalTuner);
    DDX_Control(pDX, IDC_COMBO3, m_cbDigitalReceiver);
    DDX_Control(pDX, IDC_COMBO6, m_cbRebuildFilterGraph);
    DDX_Control(pDX, IDC_COMBO7, m_cbStopFilterGraph);
    DDX_Check(pDX, IDC_CHECK2, m_bEnableIPTV);
    DDX_Check(pDX, IDC_CHECK3, m_bUseIGMPMembership);
}


BEGIN_MESSAGE_MAP(CPPageDigitalTV, CPPageBase)
    ON_UPDATE_COMMAND_UI(IDC_COMBO4, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_COMBO5, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_STATIC4, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_STATIC5, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_COMBO3, OnUpdateDigitalReciver)
    ON_UPDATE_COMMAND_UI(IDC_STATIC6, OnUpdateDigitalReciver)
    ON_UPDATE_COMMAND_UI(IDC_COMBO6, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_COMBO7, OnUpdateDigitalStopFilterGraph)
    ON_UPDATE_COMMAND_UI(IDC_STATIC, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_PPAGECAPTURE_ST10, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_PPAGECAPTURE_ST11, OnUpdateDVB)
    ON_UPDATE_COMMAND_UI(IDC_PPAGECAPTURE_ST12, OnUpdateDigitalStopFilterGraph)
    ON_UPDATE_COMMAND_UI(IDC_PPAGECAPTURE_DESC1, OnUpdateDVB)
    ON_CBN_SELCHANGE(IDC_COMBO6, OnSelchangeRebuildFilterGraph)
    ON_CBN_SELCHANGE(IDC_COMBO7, OnSelchangeStopFilterGraph)
    ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateIPTV)
    ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdateIPTV)
    ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateIPTV)
END_MESSAGE_MAP()


// CPPageCapture message handlers

BOOL CPPageDigitalTV::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_COMBO4);

    const CAppSettings& s = AfxGetAppSettings();

    FindDigitalDevices();

    m_bEnableDVB = s.bEnabledDVB;

    m_cbRebuildFilterGraph.AddString(ResStr(IDS_PPAGE_CAPTURE_FG0));
    m_cbRebuildFilterGraph.AddString(ResStr(IDS_PPAGE_CAPTURE_FG1));
    m_cbRebuildFilterGraph.AddString(ResStr(IDS_PPAGE_CAPTURE_FG2));
    m_cbRebuildFilterGraph.SetCurSel(s.nDVBRebuildFilterGraph);
    CorrectComboListWidth(m_cbRebuildFilterGraph);

    m_cbStopFilterGraph.AddString(ResStr(IDS_PPAGE_CAPTURE_SFG0));
    m_cbStopFilterGraph.AddString(ResStr(IDS_PPAGE_CAPTURE_SFG1));
    m_cbStopFilterGraph.AddString(ResStr(IDS_PPAGE_CAPTURE_SFG2));
    m_cbStopFilterGraph.SetCurSel(s.nDVBStopFilterGraph);
    CorrectComboListWidth(m_cbStopFilterGraph);
    OnSelchangeRebuildFilterGraph();
    OnSelchangeStopFilterGraph();

    m_bEnableIPTV = s.bEnabledIPTV;
    m_bUseIGMPMembership = s.bUseIGMPMembership;
    GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);

    UpdateData(FALSE);
    SaveFoundDevices(); // Save (new) devices to ensure that comboboxes reflect actual settings.

    EnableToolTips(TRUE);

    return TRUE;
}

BOOL CPPageDigitalTV::OnApply()
{
    UpdateData();
    SaveFoundDevices();
    AfxGetMainWnd()->PostMessageW(WM_DTV_REFRESHSETTINGS);
    return __super::OnApply();
}

void CPPageDigitalTV::OnUpdateDVB(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK1) && m_cbDigitalTuner.GetCount());
}

void CPPageDigitalTV::OnUpdateIPTV(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK2));
}

void CPPageDigitalTV::OnUpdateDigitalReciver(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK1) && m_cbDigitalReceiver.GetCount());
}

void CPPageDigitalTV::OnUpdateDigitalStopFilterGraph(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK1) && m_cbDigitalTuner.GetCount() &&
                   (m_cbRebuildFilterGraph.GetCurSel() != 2));
}

void CPPageDigitalTV::OnSelchangeRebuildFilterGraph()
{
    if (m_cbRebuildFilterGraph.GetCurSel() == 0) {
        GetDlgItem(IDC_PPAGECAPTURE_DESC1)->SetWindowText(ResStr(IDS_PPAGE_CAPTURE_FGDESC0));
    } else if (m_cbRebuildFilterGraph.GetCurSel() == 1) {
        GetDlgItem(IDC_PPAGECAPTURE_DESC1)->SetWindowText(ResStr(IDS_PPAGE_CAPTURE_FGDESC1));
    } else if (m_cbRebuildFilterGraph.GetCurSel() == 2) {
        GetDlgItem(IDC_PPAGECAPTURE_DESC1)->SetWindowText(ResStr(IDS_PPAGE_CAPTURE_FGDESC2));
    } else {
        GetDlgItem(IDC_PPAGECAPTURE_DESC1)->SetWindowText(_T(""));
    }
    SetModified();
}


void CPPageDigitalTV::OnSelchangeStopFilterGraph()
{
    SetModified();
}

BOOL CPPageDigitalTV::OnToolTipNotify(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
    LPTOOLTIPTEXT pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMH);

    UINT_PTR nID = pNMH->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND) {
        nID = ::GetDlgCtrlID((HWND)nID);
    }

    BOOL bRet = FALSE;

    switch (nID) {
        case IDC_COMBO4:
            bRet = FillComboToolTip(m_cbDigitalNetworkProvider, pTTT);
            break;
        case IDC_COMBO5:
            bRet = FillComboToolTip(m_cbDigitalTuner, pTTT);
            break;
        case IDC_COMBO3:
            bRet = FillComboToolTip(m_cbDigitalReceiver, pTTT);
            break;
        case IDC_COMBO6:
            bRet = FillComboToolTip(m_cbRebuildFilterGraph, pTTT);
            break;
        case IDC_COMBO7:
            bRet = FillComboToolTip(m_cbStopFilterGraph, pTTT);
            break;
    }

    return bRet;
}

void CPPageDigitalTV::FindDigitalDevices()
{
    const CAppSettings& s = AfxGetAppSettings();
    int iSel = 0;
    bool bFound = false;

    BeginEnumSysDev(KSCATEGORY_BDA_NETWORK_PROVIDER, pMoniker) {
        CComPtr<IPropertyBag> pPB;
        pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPB));

        CComVariant var;
        if (SUCCEEDED(pPB->Read(_T("FriendlyName"), &var, nullptr))) {
            int i = m_cbDigitalNetworkProvider.AddString(CString(var.bstrVal));

            CComHeapPtr<OLECHAR> strName;
            if (SUCCEEDED(pMoniker->GetDisplayName(nullptr, nullptr, &strName))) {
                m_providernames.Add(CString(strName));
                if (s.strBDANetworkProvider == CString(strName)) {
                    iSel = i;
                    bFound = true;
                } else if (!bFound && CString(var.bstrVal) == _T("Microsoft Network Provider")) {
                    // Select Microsoft Network Provider by default, other network providers are deprecated.
                    iSel = i;
                }
            }
        }
    }
    EndEnumSysDev;
    if (m_cbDigitalNetworkProvider.GetCount()) {
        m_cbDigitalNetworkProvider.SetCurSel(iSel);
    } else {
        return;
    }


    iSel = 0;
    BeginEnumSysDev(KSCATEGORY_BDA_NETWORK_TUNER, pMoniker) {
        CComPtr<IPropertyBag> pPB;
        pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPB));

        CComVariant var;
        if (SUCCEEDED(pPB->Read(_T("FriendlyName"), &var, nullptr))) {
            int i = m_cbDigitalTuner.AddString(CString(var.bstrVal));

            CComHeapPtr<OLECHAR> strName;
            if (SUCCEEDED(pMoniker->GetDisplayName(nullptr, nullptr, &strName))) {
                m_tunernames.Add(CString(strName));
                if (s.strBDATuner == CString(strName)) {
                    iSel = i;
                }
            }
        }
    }
    EndEnumSysDev;
    if (m_cbDigitalTuner.GetCount()) {
        m_cbDigitalTuner.SetCurSel(iSel);
    } else {
        return;
    }

    iSel = 0;
    BeginEnumSysDev(KSCATEGORY_BDA_RECEIVER_COMPONENT, pMoniker) {
        CComPtr<IPropertyBag> pPB;
        pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPB));

        CComVariant var;
        if (SUCCEEDED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
            int i = m_cbDigitalReceiver.AddString(CString(var.bstrVal));

            LPOLESTR strName = nullptr;
            if (SUCCEEDED(pMoniker->GetDisplayName(nullptr, nullptr, &strName))) {
                m_receivernames.Add(CString(strName));
                if (s.strBDAReceiver == CString(strName)) {
                    iSel = i;
                }
                CoTaskMemFree(strName);
            }
        }
    }
    EndEnumSysDev;
    if (m_cbDigitalReceiver.GetCount()) {
        m_cbDigitalReceiver.SetCurSel(iSel);
    }
}

void CPPageDigitalTV::SaveFoundDevices()
{
    CAppSettings& s = AfxGetAppSettings();

    s.bEnabledDVB = !!m_bEnableDVB;
    if (m_cbDigitalNetworkProvider.GetCurSel() >= 0) {
        s.strBDANetworkProvider = m_providernames[m_cbDigitalNetworkProvider.GetCurSel()];
    }
    if (m_cbDigitalTuner.GetCurSel() >= 0) {
        s.strBDATuner = m_tunernames[m_cbDigitalTuner.GetCurSel()];
    }
    if (m_cbDigitalReceiver.GetCurSel() >= 0) {
        s.strBDAReceiver = m_receivernames[m_cbDigitalReceiver.GetCurSel()];
    }
    s.nDVBRebuildFilterGraph = (DVB_RebuildFilterGraph)m_cbRebuildFilterGraph.GetCurSel();
    s.nDVBStopFilterGraph = (DVB_StopFilterGraph)m_cbStopFilterGraph.GetCurSel();

    s.bEnabledIPTV = !!m_bEnableIPTV;
    s.bUseIGMPMembership = !!m_bUseIGMPMembership;

}
