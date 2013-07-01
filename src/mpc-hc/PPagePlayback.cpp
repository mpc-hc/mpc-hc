/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "mplayerc.h"
#include "MainFrm.h"
#include "PPagePlayback.h"
#include "Monitors.h"
#include "MultiMonitor.h"


// CPPagePlayback dialog

IMPLEMENT_DYNAMIC(CPPagePlayback, CPPageBase)
CPPagePlayback::CPPagePlayback()
    : CPPageBase(CPPagePlayback::IDD, CPPagePlayback::IDD)
    , m_iLoopForever(0)
    , m_nLoops(0)
    , m_fRewind(FALSE)
    , m_iZoomLevel(0)
    , m_iRememberZoomLevel(FALSE)
    , m_nAutoFitFactor(75)
    , m_nVolume(0)
    , m_oldVolume(0)
    , m_nBalance(0)
    , m_fAutoloadAudio(FALSE)
    , m_fAutoloadSubtitles(FALSE)
    , m_fEnableWorkerThreadForOpening(FALSE)
    , m_fReportFailedPins(FALSE)
    , m_subtitlesLanguageOrder(_T(""))
    , m_audiosLanguageOrder(_T(""))
    , m_nSpeedStep(0)
    , m_nVolumeStep(0)
{
}

CPPagePlayback::~CPPagePlayback()
{
}

void CPPagePlayback::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER1, m_volumectrl);
    DDX_Control(pDX, IDC_SLIDER2, m_balancectrl);
    DDX_Control(pDX, IDC_COMBO1, m_zoomlevelctrl);
    DDX_Slider(pDX, IDC_SLIDER1, m_nVolume);
    DDX_Slider(pDX, IDC_SLIDER2, m_nBalance);
    DDX_Radio(pDX, IDC_RADIO1, m_iLoopForever);
    DDX_Control(pDX, IDC_EDIT1, m_loopnumctrl);
    DDX_Text(pDX, IDC_EDIT1, m_nLoops);
    DDX_Check(pDX, IDC_CHECK1, m_fRewind);
    DDX_CBIndex(pDX, IDC_COMBO1, m_iZoomLevel);
    DDX_Check(pDX, IDC_CHECK5, m_iRememberZoomLevel);
    DDX_Check(pDX, IDC_CHECK2, m_fAutoloadAudio);
    DDX_Check(pDX, IDC_CHECK3, m_fAutoloadSubtitles);
    DDX_Check(pDX, IDC_CHECK7, m_fEnableWorkerThreadForOpening);
    DDX_Check(pDX, IDC_CHECK6, m_fReportFailedPins);
    DDX_Text(pDX, IDC_EDIT2, m_subtitlesLanguageOrder);
    DDX_Text(pDX, IDC_EDIT3, m_audiosLanguageOrder);
    DDX_Text(pDX, IDC_VOLUMESTEP, m_nVolumeStep);
    DDX_Text(pDX, IDC_EDIT4, m_nAutoFitFactor);
    DDX_Control(pDX, IDC_VOLUMESTEP_SPIN, m_VolumeStepCtrl);
    DDX_Control(pDX, IDC_SPEEDSTEP_SPIN, m_SpeedStepCtrl);
    DDX_Control(pDX, IDC_SPIN1, m_AutoFitFactorCtrl);
}

BEGIN_MESSAGE_MAP(CPPagePlayback, CPPageBase)
    ON_WM_HSCROLL()
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO2, OnBnClickedRadio12)
    ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateLoopNum)
    ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateLoopNum)
    ON_UPDATE_COMMAND_UI(IDC_COMBO1, OnUpdateAutoZoomCombo)
    ON_UPDATE_COMMAND_UI(IDC_SPEEDSTEP_SPIN, OnUpdateSpeedStep)
    ON_UPDATE_COMMAND_UI(IDC_EDIT4, OnUpdateAutoZoomFactor)
    ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdateAutoZoomFactor)
    ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdateAutoZoomFactor)

    ON_STN_DBLCLK(IDC_STATIC_BALANCE, OnBalanceTextDblClk)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// CPPagePlayback message handlers

BOOL CPPagePlayback::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_COMBO1);

    const CAppSettings& s = AfxGetAppSettings();

    m_volumectrl.SetRange(0, 100);
    m_volumectrl.SetTicFreq(10);
    m_balancectrl.SetRange(-100, 100);
    m_balancectrl.SetTicFreq(20);
    m_nVolume = m_oldVolume = s.nVolume;
    m_nBalance = s.nBalance;
    m_nVolumeStep = s.nVolumeStep;
    m_VolumeStepCtrl.SetRange(1, 25);
    m_nSpeedStep = s.nSpeedStep;
    m_SpeedStepCtrl.SetPos(m_nSpeedStep);
    m_SpeedStepCtrl.SetRange(0, 100);
    m_iLoopForever = s.fLoopForever ? 1 : 0;
    m_nLoops = s.nLoops;
    m_fRewind = s.fRewind;
    m_iZoomLevel = s.iZoomLevel;
    m_iRememberZoomLevel = s.fRememberZoomLevel;
    m_nAutoFitFactor = s.nAutoFitFactor;
    m_AutoFitFactorCtrl.SetPos(m_nAutoFitFactor);
    m_AutoFitFactorCtrl.SetRange(25, 100);
    m_fAutoloadAudio = s.fAutoloadAudio;
    m_fAutoloadSubtitles = s.fAutoloadSubtitles;
    m_fEnableWorkerThreadForOpening = s.fEnableWorkerThreadForOpening;
    m_fReportFailedPins = s.fReportFailedPins;
    m_subtitlesLanguageOrder = s.strSubtitlesLanguageOrder;
    m_audiosLanguageOrder = s.strAudiosLanguageOrder;

    m_zoomlevelctrl.AddString(ResStr(IDS_ZOOM_50));
    m_zoomlevelctrl.AddString(ResStr(IDS_ZOOM_100));
    m_zoomlevelctrl.AddString(ResStr(IDS_ZOOM_200));
    m_zoomlevelctrl.AddString(ResStr(IDS_ZOOM_AUTOFIT));
    m_zoomlevelctrl.AddString(ResStr(IDS_ZOOM_AUTOFIT_LARGER));
    CorrectComboListWidth(m_zoomlevelctrl);

    // set the spinner acceleration value
    UDACCEL accel = { 0, 10 };
    m_SpeedStepCtrl.SetAccel(1, &accel);

    EnableToolTips(TRUE);
    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPagePlayback::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    s.nVolume = m_oldVolume = m_nVolume;
    s.nBalance = m_nBalance;
    s.nVolumeStep = min(max(m_nVolumeStep, 1), 100);
    s.nSpeedStep = m_nSpeedStep;
    s.fLoopForever = !!m_iLoopForever;
    s.nLoops = m_nLoops;
    s.fRewind = !!m_fRewind;
    s.iZoomLevel = m_iZoomLevel;
    s.fRememberZoomLevel = !!m_iRememberZoomLevel;
    s.nAutoFitFactor = m_nAutoFitFactor = min(max(m_nAutoFitFactor, 25), 100);
    s.fAutoloadAudio = !!m_fAutoloadAudio;
    s.fAutoloadSubtitles = !!m_fAutoloadSubtitles;
    s.fEnableWorkerThreadForOpening = !!m_fEnableWorkerThreadForOpening;
    s.fReportFailedPins = !!m_fReportFailedPins;
    s.strSubtitlesLanguageOrder = m_subtitlesLanguageOrder;
    s.strAudiosLanguageOrder = m_audiosLanguageOrder;

    AfxGetMainFrame()->UpdateControlState(CMainFrame::UPDATE_VOLUME_STEP);

    return __super::OnApply();
}

void CPPagePlayback::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (*pScrollBar == m_volumectrl) {
        UpdateData();
        ((CMainFrame*)GetParentFrame())->m_wndToolBar.Volume = m_nVolume; // nice shortcut...
    } else if (*pScrollBar == m_balancectrl) {
        UpdateData();
        ((CMainFrame*)GetParentFrame())->SetBalance(m_nBalance); // see prev note...
    }

    SetModified();

    __super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPagePlayback::OnBnClickedRadio12(UINT nID)
{
    SetModified();
}

void CPPagePlayback::OnUpdateLoopNum(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!!IsDlgButtonChecked(IDC_RADIO1));
}

void CPPagePlayback::OnUpdateAutoZoomCombo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!!IsDlgButtonChecked(IDC_CHECK5));
}

void CPPagePlayback::OnUpdateAutoZoomFactor(CCmdUI* pCmdUI)
{
    int iZoomLevel = m_zoomlevelctrl.GetCurSel();
    pCmdUI->Enable(!!IsDlgButtonChecked(IDC_CHECK5) && (iZoomLevel == 3 || iZoomLevel == 4));
}

void CPPagePlayback::OnUpdateSpeedStep(CCmdUI* pCmdUI)
{
    // if there is an error retrieving the position, assume the speedstep is set to auto
    BOOL bError = FALSE;
    int iPos = m_SpeedStepCtrl.GetPos32(&bError);
    m_nSpeedStep = bError ? 0 : iPos;

    // set the edit box text to auto if the position is zero
    if (!bError && iPos == 0) {
        SetDlgItemText(IDC_SPEEDSTEP, ResStr(IDS_SPEEDSTEP_AUTO));
    }
}

void CPPagePlayback::OnBalanceTextDblClk()
{
    // double click on text "Balance" resets the balance to zero
    m_nBalance = 0;
    m_balancectrl.SetPos(m_nBalance);
    ((CMainFrame*)GetParentFrame())->SetBalance(m_nBalance);
    SetModified();
}

BOOL CPPagePlayback::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    LPTOOLTIPTEXT pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMHDR);

    UINT_PTR nID = pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND) {
        nID = ::GetDlgCtrlID((HWND)nID);
    }

    if (nID == 0) {
        return FALSE;
    }

    CString strTipText;

    if (nID == IDC_SLIDER1) {
        strTipText.Format(_T("%d%%"), m_nVolume);
    } else if (nID == IDC_SLIDER2) {
        if (m_nBalance > 0) {
            strTipText.Format(_T("R +%d%%"), m_nBalance);
        } else if (m_nBalance < 0) {
            strTipText.Format(_T("L +%d%%"), -m_nBalance);
        } else { //if (m_nBalance == 0)
            strTipText = _T("L = R");
        }
    } else if (nID == IDC_COMBO1) {
        int i = m_zoomlevelctrl.GetCurSel();
        m_zoomlevelctrl.GetLBText(i, strTipText);
    } else {
        return FALSE;
    }

    _tcscpy_s(pTTT->szText, strTipText.Left(_countof(pTTT->szText)));

    *pResult = 0;

    return TRUE;    // message was handled
}

void CPPagePlayback::OnCancel()
{
    const CAppSettings& s = AfxGetAppSettings();

    if (m_nVolume != m_oldVolume) {
        ((CMainFrame*)GetParentFrame())->m_wndToolBar.Volume = m_oldVolume;    //not very nice solution
    }
    if (m_nBalance != s.nBalance) {
        ((CMainFrame*)GetParentFrame())->SetBalance(s.nBalance);
    }

    __super::OnCancel();
}
