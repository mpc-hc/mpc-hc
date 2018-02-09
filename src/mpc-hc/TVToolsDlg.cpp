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
#include "FGManagerBDA.h"

#define THREAD_EXIT_TIMEOUT 20000
#define PROCESS_FINISH_TIMEOUT 30000

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
    m_pTVToolsThread = dynamic_cast<CTVToolsThread*>(AfxBeginThread(RUNTIME_CLASS(CTVToolsThread)));
    if (m_pTVToolsThread) {
        m_pTVToolsThread->SetTVToolsDlg(this);
    } else {
        ASSERT(FALSE);
    }
}

void CTVToolsDlg::OnDestroy()
{
    if (m_pTVToolsThread) {
        if (m_TunerScanDlg.m_bInProgress) {
            m_TunerScanDlg.m_bStopRequested = true;
            DWORD RetValue = WaitForSingleObject(m_pTVToolsThread->m_evProcessFinished, PROCESS_FINISH_TIMEOUT);
            switch (RetValue) {
            case WAIT_TIMEOUT:
                TRACE(_T("Timeout when trying to stop the tuner scan process.\n"));
                break;
            case WAIT_ABANDONED:
                TRACE(_T("Wait abandoned trying to stop the tuner scan process.\n"));
                break;
            case WAIT_OBJECT_0:
                TRACE(_T("Tuner scan process stopped successfully.\n"));
                break;
            case WAIT_FAILED:
                DWORD LErr = GetLastError();
                TRACE(_T("Error %lu trying to stop the tuner scan process.\n"), LErr);
                break;
            }
        }
        else if (m_IPTVScanDlg.m_bInProgress) {
            m_IPTVScanDlg.m_bStopRequested = true;
            DWORD RetValue = WaitForSingleObject(m_pTVToolsThread->m_evProcessFinished, PROCESS_FINISH_TIMEOUT);
            switch (RetValue) {
            case WAIT_TIMEOUT:
                TRACE(_T("Timeout when trying to stop the IPTV scan/discovery process.\n"));
                break;
            case WAIT_ABANDONED:
                TRACE(_T("Wait abandoned trying to stop the IPTV scan/discovery process.\n"));
                break;
            case WAIT_OBJECT_0:
                TRACE(_T("IPTV scan/discovery process stopped successfully.\n"));
                break;
            case WAIT_FAILED:
                DWORD LErr = GetLastError();
                TRACE(_T("Error %lu trying to stop the IPTV scan/discovery process.\n"), LErr);
                break;
            }
        }

        VERIFY(m_evCloseFinished.Set());

        CAMMsgEvent e;
        VERIFY(m_pTVToolsThread->PostThreadMessage(CTVToolsThread::TM_EXIT, 0, (LPARAM)&e));
        if (!e.Wait(THREAD_EXIT_TIMEOUT)) {
             TRACE(_T("ERROR: Must call TerminateThread() on CTVToolsDlg::m_pTVToolsThread->m_hThread\n"));
             TerminateThread(m_pTVToolsThread->m_hThread, DWORD_ERROR);
        }
        m_pTVToolsThread = nullptr;
    }

    __super::OnDestroy();
}

void CTVToolsDlg::OnClose()
{
    if (m_TunerScanDlg.m_bInProgress || m_IPTVScanDlg.m_bInProgress) {
        m_TunerScanDlg.m_bStopRequested = true;
    }
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
    if (m_pTVToolsDlg->m_TunerScanDlg.m_bInProgress) {
        m_pTVToolsDlg->m_TunerScanDlg.m_bStopRequested = true;
        ASSERT(WaitForSingleObject(m_evProcessFinished, 0) == WAIT_TIMEOUT);
    } else if (m_pTVToolsDlg->m_IPTVScanDlg.m_bInProgress) {
        m_pTVToolsDlg->m_IPTVScanDlg.m_bStopRequested = true;
        ASSERT(WaitForSingleObject(m_evProcessFinished, 0) == WAIT_TIMEOUT);
    }

    if (m_bMediaClosed) {
        AfxGetMyApp()->GetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_OPENDIGITALTV);
        m_bMediaClosed = false;
    }
}

void CTVToolsThread::OnExit(WPARAM wParam, LPARAM lParam)
{
    ASSERT(m_pTVToolsDlg);
    if (m_bMediaClosed) {
        AfxGetMyApp()->GetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_OPENDIGITALTV);
        m_bMediaClosed = false;
    }

    PostQuitMessage(0);
    if (CAMEvent* e = (CAMEvent*)lParam) {
        e->Set();
    }
    TRACE(_T("Thread OnExit: PostQuitMessage(0)./n"));
}

void CTVToolsThread::OnOpen(WPARAM wParam, LPARAM lParam)
{
    TRACE(_T("--> CTVToolsThread::OnOpen on thread: %lu\n"), GetCurrentThreadId());
    ASSERT(m_pTVToolsDlg);
    VERIFY(m_pTVToolsDlg->m_evCloseFinished.Reset());
}

void CTVToolsThread::OnTunerScan(WPARAM wParam, LPARAM lParam)
{
    m_pTVToolsDlg->m_TabCtrl.EnableWindow(false);
    m_evProcessFinished.Reset();
    CAutoPtr<TunerScanData> pTSD((TunerScanData*) lParam);
    CFGManagerBDA* pFGMBDA = DEBUG_NEW CFGManagerBDA(_T("CFGManagerBDA"), nullptr, AfxGetMyApp()->GetMainWnd()->m_hWnd);
    pGB = pFGMBDA;
    pGB->AddToROT();
    CMainFrame* pMfrm = dynamic_cast<CMainFrame*>(AfxGetMyApp()->GetMainWnd());

    // When watching DTV, close the current graph before opening new graph for tuner scan
    if (pMfrm && (pMfrm->GetLoadState() != MLS::CLOSED) && (pMfrm->GetPlaybackMode() == PM_DIGITAL_TV)) {
        pMfrm->SendMessage(WM_COMMAND, ID_FILE_CLOSEMEDIA);
        ASSERT(pMfrm->GetLoadState() == MLS::CLOSED);
        m_bMediaClosed = true;
    }

    pGB->RenderFile(P_SCAN, (LPCWSTR)&m_pTVToolsDlg->m_TunerScanDlg.m_bStopRequested);

    CComQIPtr<IBDATuner> pTun = pGB;
    BOOLEAN bPresent;
    BOOLEAN bLocked;
    LONG lDbStrength;
    LONG lPercentQuality;
    int nOffset = pTSD->Offset ? 3 : 1;
    LONG lOffsets[3] = { 0, pTSD->Offset, -pTSD->Offset };
    pTun->Scan(0, 0, NULL);  // Clear maps

    bool bStopRequested = false;
    for (ULONG ulFrequency = pTSD->FrequencyStart; ulFrequency <= pTSD->FrequencyStop; ulFrequency += pTSD->Bandwidth) {
        bool bSucceeded = false;
        for (int nOffsetPos = 0; nOffsetPos < nOffset && !bSucceeded; nOffsetPos++) {
            if (SUCCEEDED(pTun->SetFrequency(ulFrequency + lOffsets[nOffsetPos], pTSD->Bandwidth))) {
                // Let the tuner some time to detect the signal (originally Sleep(200))
                int iWaitTime = 20;
                while (iWaitTime > 0 && !bStopRequested) {
                    Sleep(10);
                    bStopRequested = m_pTVToolsDlg->m_TunerScanDlg.m_bStopRequested;
                    iWaitTime--;
                }

                if (bStopRequested) {
                    break;
                }
                if (SUCCEEDED(pTun->GetStats(bPresent, bLocked, lDbStrength, lPercentQuality)) && bPresent) {
                    ::PostMessage(pTSD->Hwnd, WM_TUNER_STATS, lDbStrength, lPercentQuality);
                    if (pTun->Scan(ulFrequency + lOffsets[nOffsetPos], pTSD->Bandwidth, pTSD->Hwnd) == S_FALSE) {
                        bStopRequested = true;
                        break;
                    }
                    bSucceeded = true;
                }
            }
            if (!bStopRequested) {
                bStopRequested = m_pTVToolsDlg->m_TunerScanDlg.m_bStopRequested;
                if (bStopRequested) {
                    break;
                }
            }
        }

        if (bStopRequested) {
            break;
        }

        int nProgress = MulDiv(ulFrequency - pTSD->FrequencyStart, 100, pTSD->FrequencyStop - pTSD->FrequencyStart);
        ::PostMessage(pTSD->Hwnd, WM_TUNER_SCAN_PROGRESS, nProgress, 0);
        ::PostMessage(pTSD->Hwnd, WM_TUNER_STATS, lDbStrength, lPercentQuality);
    }

    if (pGB) {
        pGB->RemoveFromROT();
        pGB.Release();
    }
    m_evProcessFinished.Set();
    ::PostMessage(pTSD->Hwnd, WM_TUNER_SCAN_END, 0, 0);
}


void CTVToolsThread::OnIPTVDiscovery(WPARAM wParam, LPARAM lParam)
{
    bool bStopRequested;
    std::unique_ptr<CIPTVMcastTools> pMulticastTools;
    pMulticastTools = std::make_unique<CIPTVMcastTools>();
    const CAppSettings& s = AfxGetAppSettings();
    boolean bValidDiscoverySetupWindow = true;
    m_evProcessFinished.Reset();

    pMulticastTools->BroadcastChannelsDiscover(s.strServiceProvider_IP, s.strServicesProvider_Port, (HWND)m_pTVToolsDlg->m_IPTVScanDlg, std::ref(m_pTVToolsDlg->m_IPTVScanDlg.m_bStopRequested));
    bStopRequested = m_pTVToolsDlg->m_IPTVScanDlg.m_bStopRequested;
    if (!bStopRequested) {

        for (int nItem = 0; nItem < m_pTVToolsDlg->m_IPTVScanDlg.m_ChannelList.GetItemCount(); nItem++) {
            CDVBChannel channel(m_pTVToolsDlg->m_IPTVScanDlg.m_ChannelList.GetItemText(nItem, ISCC_CHANNEL));
            HRESULT hr = pMulticastTools->VerifyChannel(channel.GetUrl());
            bStopRequested = m_pTVToolsDlg->m_IPTVScanDlg.m_bStopRequested;
            if (bStopRequested) {
                break;
            }
            CString strVerify = _T("No");
            if (hr == S_OK) {
                strVerify = _T("Yes");
            }
            if (m_pTVToolsDlg && ::IsWindow(m_pTVToolsDlg->m_IPTVScanDlg.GetSafeHwnd())) {
                m_pTVToolsDlg->m_IPTVScanDlg.m_ChannelList.SetItemText(nItem, ISCC_VALIDATED, strVerify);
            } else {
                // The window has been closed and doesn't exist anymore
                bValidDiscoverySetupWindow = false;
                TRACE(_T("The window has been closed and doesn't exist anymore\n"));
                break;
            }
            bStopRequested = m_pTVToolsDlg->m_IPTVScanDlg.m_bStopRequested;
            if (bStopRequested) {
                break;
            }

        }
    }
    else {
        TRACE(_T("Stop requested.\n"));
    }

    pMulticastTools = nullptr;
    m_evProcessFinished.Set();
    if (bValidDiscoverySetupWindow) {
        m_pTVToolsDlg->m_IPTVScanDlg.PostMessage(WM_IPTV_END_DISCOVERY);
    }
}

void CTVToolsThread::OnIPTVScan(WPARAM wParam, LPARAM lParam)
{
    CString strIP1, strIPAddress1, strIPAddress2, strPort;
    strIP1 = (LPCTSTR)wParam;

    m_evProcessFinished.Reset();

    int iPos1 = strIP1.ReverseFind(_T(':'));
    if (iPos1 < 0) {
        iPos1 = strIP1.GetLength() - 1;
    }
    strIPAddress1 = strIP1.Left(iPos1);
    strIPAddress2 = (LPCTSTR)lParam;
    strPort = strIP1.Right(strIP1.GetLength() - iPos1 - 1);

    std::unique_ptr<CIPTVMcastTools> pMulticastTools;
    pMulticastTools = std::make_unique<CIPTVMcastTools>();
    pMulticastTools->ScanRangeIPs(strIPAddress1, strIPAddress2, strPort, (HWND)m_pTVToolsDlg->m_IPTVScanDlg, std::ref(m_pTVToolsDlg->m_IPTVScanDlg.m_bStopRequested));

    pMulticastTools = nullptr;
    m_evProcessFinished.Set();
    m_pTVToolsDlg->m_IPTVScanDlg.PostMessage(WM_IPTV_END_DISCOVERY);
}

void CTVToolsThread::OnIPTVServiceProviders(WPARAM wParam, LPARAM lParam)
{
    std::unique_ptr<CIPTVMcastTools> pMulticastTools;
    m_evProcessFinished.Reset();
    pMulticastTools = std::make_unique<CIPTVMcastTools>();
    CString strIP = (LPCTSTR)wParam;
    CString strPort = (LPCTSTR)lParam;
    pMulticastTools->GetServiceProviders(strIP, strPort, m_pTVToolsDlg->m_IPTVScanDlg.m_pIPTVDiscoverySetup->GetSafeHwnd());

    pMulticastTools = nullptr;
    m_evProcessFinished.Set();

}
