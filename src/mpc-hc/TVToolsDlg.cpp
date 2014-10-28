/*
* (C) 2009-2014 see Authors.txt
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

// ScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TVToolsDlg.h"
#include "mplayerc.h"
#include "MainFrm.h"

// CTVToolsDlg dialog

IMPLEMENT_DYNAMIC(CTVToolsDlg, CDialog)

CTVToolsDlg::CTVToolsDlg(CWnd* pParent /*=nullptr*/)
    : CDialog(CTVToolsDlg::IDD, pParent)
    , m_TunerScanDlg(this)
    , m_IPTVScanDlg(this)
{

}

CTVToolsDlg::~CTVToolsDlg()
{

}

BOOL CTVToolsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_TabCtrl.InsertItem(0, ResStr(IDS_DTV_DVB_SCAN));
    m_TabCtrl.InsertItem(1, ResStr(IDS_DTV_IPTV_SCAN));

    SetTab(0);
    return TRUE;
}

void CTVToolsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CTVToolsDlg, CDialog)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CTVToolsDlg::OnTcnSelchangeTab1)
    ON_MESSAGE(WM_DTV_SETCHANNEL, OnSetChannel)
END_MESSAGE_MAP()

// CTVToolsDlg message handlers

HRESULT CTVToolsDlg::SetTab(int iTabNumber)
{
    HRESULT hr = S_OK;
    TCITEM TabCtrlItem;
    CRect m_ClientRect;
    CRect m_ItemRect;
    CDialog pCurrentDlg;

    TabCtrlItem.mask = TCIF_STATE;
    m_TabCtrl.GetItem(iTabNumber, &TabCtrlItem);
    TabCtrlItem.mask = TCIF_STATE;
    TabCtrlItem.dwState = TCIS_HIGHLIGHTED;
    TabCtrlItem.dwStateMask = TCIS_HIGHLIGHTED;

    m_TabCtrl.SetItem(iTabNumber, &TabCtrlItem);

    m_TabCtrl.GetClientRect(m_ClientRect);
    m_TabCtrl.GetItemRect(0, m_ItemRect);

    int iPosX0 = m_ItemRect.left + 10;
    int iPosY0 = m_ItemRect.bottom + 12;
    int iPosX1 = m_ClientRect.right - iPosX0;
    int iPosY1 = m_ClientRect.bottom - iPosY0;

    switch (iTabNumber) {
        case 0:
            if (m_IPTVScanDlg) {
                m_IPTVScanDlg.ShowWindow(SW_HIDE);
            }
            if (!m_TunerScanDlg) {
                m_TunerScanDlg.Create(IDD_TUNER_SCAN, this);
            }
            m_TunerScanDlg.ShowWindow(SW_SHOWNORMAL);
            m_TunerScanDlg.MoveWindow(iPosX0, iPosY0, iPosX1, iPosY1);

            break;
        case 1:
            if (m_TunerScanDlg) {
                m_TunerScanDlg.ShowWindow(SW_HIDE);
            }
            if (!m_IPTVScanDlg) {
                m_IPTVScanDlg.Create(IDD_IPTV_SCAN, this);
            }
            m_IPTVScanDlg.ShowWindow(SW_SHOWNORMAL);
            m_IPTVScanDlg.MoveWindow(iPosX0, iPosY0, iPosX1, iPosY1);

            break;

        default:
            hr = E_INVALIDARG;
    }
    return hr;
}

void CTVToolsDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
    SetTab(m_TabCtrl.GetCurSel());

    *pResult = 0;
}

LRESULT CTVToolsDlg::OnSetChannel(WPARAM wParam, LPARAM lParam)
{
    // In the future, this should send a message to CMainFrame
    // instead of calling the SetChannel function directly
    LRESULT hr = E_ABORT;

    hr = ((CMainFrame*)GetParent())->SetChannel((int) wParam);
    return hr;
}
