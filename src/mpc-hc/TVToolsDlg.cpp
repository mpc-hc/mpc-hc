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
    , m_bEnabledDVB(true)
    , m_bEnabledIPTV(true)
    , m_evCloseFinished(FALSE, TRUE)
{
    m_pTVToolsThread = (CTVToolsThread*)AfxBeginThread(RUNTIME_CLASS(CTVToolsThread));
    m_pTVToolsThread->SetTVToolsDlg(this);
}

void CTVToolsDlg::OnDestroy()
{
    if (m_pTVToolsThread) {
        CAMMsgEvent e;
        m_pTVToolsThread->PostThreadMessage(CTVToolsThread::TM_EXIT, 0, (LPARAM)&e);
        if (!e.Wait(5000)) {
            TRACE(_T("ERROR: Must call TerminateThread() on CTVToolsDlg::m_pTVToolsThread->m_hThread\n"));
            TerminateThread(m_pTVToolsThread->m_hThread, DWORD_ERROR);
        }
    }
    __super::OnDestroy();
}

void CTVToolsDlg::OnClose()
{
    VERIFY(m_pTVToolsThread->PostThreadMessage(CTVToolsThread::TM_CLOSE, 0, 0));
    ASSERT(WaitForSingleObject(m_evCloseFinished, 0) == WAIT_TIMEOUT);
    __super::OnClose();
}

BOOL CTVToolsDlg::OnInitDialog()
{
    const auto& s = AfxGetAppSettings();
    CDialog::OnInitDialog();

    m_Tab_scan[0] = SC_NONE;
    m_Tab_scan[1] = SC_NONE;
    int i = 0;

    if (m_bEnabledDVB && !s.strBDATuner.IsEmpty()) {
        m_TabCtrl.InsertItem(i, ResStr(IDS_DTV_DVB_SCAN));
        m_Tab_scan[i] = SC_DVB;
        i++;
    }

    if (m_bEnabledIPTV) {
        m_TabCtrl.InsertItem(i, ResStr(IDS_DTV_IPTV_SCAN));
        m_Tab_scan[i] = SC_IPTV;
        i++;
    }

    SetTab(0);
    return TRUE;
}

void CTVToolsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CTVToolsDlg, CDialog)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
    ON_MESSAGE(WM_DTV_SETCHANNEL, OnSetChannel)
    ON_WM_CLOSE()
    ON_WM_DESTROY()
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

    CRect tabRect;
    m_TabCtrl.GetWindowRect(tabRect);
    m_TabCtrl.AdjustRect(FALSE, tabRect);
    ScreenToClient(tabRect);

    switch (iTabNumber) {
        case 0:
            if (m_Tab_scan[iTabNumber] == SC_DVB) {
                if (m_IPTVScanDlg) {
                    m_IPTVScanDlg.ShowWindow(SW_HIDE);
                }
                if (!m_TunerScanDlg) {
                    m_TunerScanDlg.Create(IDD_TUNER_SCAN, this);
                }
                m_TunerScanDlg.ShowWindow(SW_SHOWNORMAL);
                m_TunerScanDlg.MoveWindow(tabRect);
            } else if (m_Tab_scan[iTabNumber] == SC_IPTV) {
                if (m_TunerScanDlg) {
                    m_TunerScanDlg.ShowWindow(SW_HIDE);
                }
                if (!m_IPTVScanDlg) {
                    m_IPTVScanDlg.Create(IDD_IPTV_SCAN, this);
                }
                m_IPTVScanDlg.ShowWindow(SW_SHOWNORMAL);
                m_IPTVScanDlg.MoveWindow(tabRect);
            }

            break;
        case 1:
            if (m_Tab_scan[iTabNumber] == SC_IPTV) {
                if (m_TunerScanDlg) {
                    m_TunerScanDlg.ShowWindow(SW_HIDE);
                }
                if (!m_IPTVScanDlg) {
                    m_IPTVScanDlg.Create(IDD_IPTV_SCAN, this);
                }
                m_IPTVScanDlg.ShowWindow(SW_SHOWNORMAL);
                m_IPTVScanDlg.MoveWindow(tabRect);
            } else if (m_Tab_scan[iTabNumber] == SC_DVB) {
                if (m_IPTVScanDlg) {
                    m_IPTVScanDlg.ShowWindow(SW_HIDE);
                }
                if (!m_TunerScanDlg) {
                    m_TunerScanDlg.Create(IDD_TUNER_SCAN, this);
                }
                m_TunerScanDlg.ShowWindow(SW_SHOWNORMAL);
                m_TunerScanDlg.MoveWindow(tabRect);
            }

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
    auto pMFrm = dynamic_cast<CMainFrame*>(GetParent());

    if (pMFrm) {
        hr = pMFrm->SetChannel((int)wParam);
    }
    return hr;
}

// CTVToolsThread

IMPLEMENT_DYNCREATE(CTVToolsThread, CWinThread)

BOOL CTVToolsThread::InitInstance()
{
    SetThreadName(DWORD(-1), "TVToolsThread");
    AfxSocketInit();
    return SUCCEEDED(CoInitialize(nullptr)) ? TRUE : FALSE;
}

int CTVToolsThread::ExitInstance()
{
    CoUninitialize();
    return __super::ExitInstance();
}

BEGIN_MESSAGE_MAP(CTVToolsThread, CWinThread)
    ON_THREAD_MESSAGE(TM_CLOSE, OnClose)
    ON_THREAD_MESSAGE(TM_EXIT, OnExit)
    ON_THREAD_MESSAGE(TM_OPEN, OnOpen)
    ON_THREAD_MESSAGE(TM_TUNER_SCAN, OnTunerScan)
    ON_THREAD_MESSAGE(TM_IPTV_DISCOVERY, OnIPTVDiscovery)
    ON_THREAD_MESSAGE(TM_IPTV_SCAN, OnIPTVScan)
    ON_THREAD_MESSAGE(TM_IPTV_SERVICEPROVIDERS, OnIPTVServiceProviders)
END_MESSAGE_MAP()

void CTVToolsThread::OnClose(WPARAM wParam, LPARAM lParam)
{
    ASSERT(m_pTVToolsDlg);
    VERIFY(m_pTVToolsDlg->m_evCloseFinished.Set());
}

void CTVToolsThread::OnExit(WPARAM wParam, LPARAM lParam)
{
    PostQuitMessage(0);
    if (CAMEvent* e = (CAMEvent*)lParam) {
        e->Set();
    }
}

void CTVToolsThread::OnOpen(WPARAM wParam, LPARAM lParam)
{
    TRACE(_T("--> CTVToolsThread::OnOpen on thread: %lu\n"), GetCurrentThreadId());
    ASSERT(m_pTVToolsDlg);
    VERIFY(m_pTVToolsDlg->m_evCloseFinished.Reset());
}

void CTVToolsThread::OnTunerScan(WPARAM wParam, LPARAM lParam)
{
    if (m_pTVToolsDlg) {
        CAutoPtr<TunerScanData> pTSD((TunerScanData*)lParam);
        //        m_pTVToolsDlg->DoTunerScan(pTSD);
    }
}

void CTVToolsThread::OnIPTVDiscovery(WPARAM wParam, LPARAM lParam)
{
    m_pTVToolsDlg->m_TabCtrl.EnableWindow(false);
    std::unique_ptr<CIPTVMcastTools> pMulticastTools;
    pMulticastTools = std::make_unique<CIPTVMcastTools>();
    const CAppSettings& s = AfxGetAppSettings();
    boolean bValidDiscoverySetupWindow = true;

    pMulticastTools->BroadcastChannelsDiscover(s.strServiceProvider_IP, s.strServicesProvider_Port, (HWND)m_pTVToolsDlg->m_IPTVScanDlg);

    for (int nItem = 0; nItem < m_pTVToolsDlg->m_IPTVScanDlg.m_ChannelList.GetItemCount(); nItem++) {
        CDVBChannel channel(m_pTVToolsDlg->m_IPTVScanDlg.m_ChannelList.GetItemText(nItem, ISCC_CHANNEL));
        HRESULT hr = pMulticastTools->VerifyChannel(channel.GetUrl());
        CString strVerify = _T("No");
        if (hr == S_OK) {
            strVerify = _T("Yes");
        }
        if (m_pTVToolsDlg && ::IsWindow(m_pTVToolsDlg->m_IPTVScanDlg.GetSafeHwnd())) {
            m_pTVToolsDlg->m_IPTVScanDlg.m_ChannelList.SetItemText(nItem, ISCC_VALIDATED, strVerify);
        } else {
            // The window has been closed and doesn't exist anymore
            bValidDiscoverySetupWindow = false;
            break;
        }
    }

    pMulticastTools = nullptr;
    if (bValidDiscoverySetupWindow) {
        m_pTVToolsDlg->m_TabCtrl.EnableWindow(true);
        m_pTVToolsDlg->m_IPTVScanDlg.SendMessage(WM_IPTV_END_DISCOVERY);
    }
}

void CTVToolsThread::OnIPTVScan(WPARAM wParam, LPARAM lParam)
{
    m_pTVToolsDlg->m_TabCtrl.EnableWindow(false);
    CString strIP1, strIPAddress1, strIPAddress2, strPort;
    strIP1 = (LPCTSTR)wParam;

    int iPos1 = strIP1.ReverseFind(_T(':'));
    if (iPos1 < 0) {
        iPos1 = strIP1.GetLength() - 1;
    }
    strIPAddress1 = strIP1.Left(iPos1);
    strIPAddress2 = (LPCTSTR)lParam;
    strPort = strIP1.Right(strIP1.GetLength() - iPos1 - 1);

    std::unique_ptr<CIPTVMcastTools> pMulticastTools;
    pMulticastTools = std::make_unique<CIPTVMcastTools>();
    pMulticastTools->ScanRangeIPs(strIPAddress1, strIPAddress2, strPort, (HWND)m_pTVToolsDlg->m_IPTVScanDlg);

    pMulticastTools = nullptr;
    m_pTVToolsDlg->m_TabCtrl.EnableWindow(true);
    m_pTVToolsDlg->m_IPTVScanDlg.SendMessage(WM_IPTV_END_DISCOVERY);
}

void CTVToolsThread::OnIPTVServiceProviders(WPARAM wParam, LPARAM lParam)
{
    std::unique_ptr<CIPTVMcastTools> pMulticastTools;
    pMulticastTools = std::make_unique<CIPTVMcastTools>();
    CString strIP = (LPCTSTR)wParam;
    CString strPort = (LPCTSTR)lParam;
    pMulticastTools->GetServiceProviders(strIP, strPort, m_pTVToolsDlg->m_IPTVScanDlg.m_pIPTVDiscoverySetup->GetSafeHwnd());

    pMulticastTools = nullptr;

}
