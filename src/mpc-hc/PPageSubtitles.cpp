/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "PPageSubtitles.h"
#include "PPageAccelTbl.h"


// CPPageSubtitles dialog

IMPLEMENT_DYNAMIC(CPPageSubtitles, CMPCThemePPageBase)
CPPageSubtitles::CPPageSubtitles()
    : CMPCThemePPageBase(CPPageSubtitles::IDD, CPPageSubtitles::IDD)
    , m_bOverridePlacement(FALSE)
    , m_nHorPos(0)
    , m_nVerPos(0)
    , m_nSPQSize(10)
    , m_bDisableSubtitleAnimation(FALSE)
    , m_nRenderAtWhenAnimationIsDisabled(50)
    , m_nAnimationRate(100)
    , m_bAllowDroppingSubpic(TRUE)
    , m_nSubDelayStep(500)
    , m_bSubtitleARCompensation(TRUE)
{
}

CPPageSubtitles::~CPPageSubtitles()
{
}

void CPPageSubtitles::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK3, m_bOverridePlacement);
    DDX_Text(pDX, IDC_EDIT2, m_nHorPos);
    DDX_Control(pDX, IDC_SPIN2, m_horPosCtrl);
    DDX_Text(pDX, IDC_EDIT3, m_nVerPos);
    DDX_Control(pDX, IDC_SPIN3, m_verPosCtrl);
    DDX_Text(pDX, IDC_SUBPIC_TO_BUFFER, m_nSPQSize);
    DDX_Control(pDX, IDC_SPIN1, m_SPQSizeCtrl);
    DDX_Control(pDX, IDC_COMBO1, m_cbSPQMaxRes);
    DDX_Check(pDX, IDC_CHECK_NO_SUB_ANIM, m_bDisableSubtitleAnimation);
    DDX_Text(pDX, IDC_EDIT5, m_nRenderAtWhenAnimationIsDisabled);
    DDX_Control(pDX, IDC_SPIN5, m_renderAtCtrl);
    DDX_Text(pDX, IDC_EDIT6, m_nAnimationRate);
    DDX_Control(pDX, IDC_SPIN6, m_animationRateCtrl);
    DDX_Check(pDX, IDC_CHECK_ALLOW_DROPPING_SUBPIC, m_bAllowDroppingSubpic);
    DDX_Text(pDX, IDC_EDIT4, m_nSubDelayStep);
    DDX_Check(pDX, IDC_CHECK_SUB_AR_COMPENSATION, m_bSubtitleARCompensation);
}


BEGIN_MESSAGE_MAP(CPPageSubtitles, CMPCThemePPageBase)
    ON_UPDATE_COMMAND_UI(IDC_EDIT2, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_SPIN2, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_EDIT3, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_SPIN3, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC4, OnUpdatePosOverride)
    ON_EN_CHANGE(IDC_EDIT4, OnSubDelayStep)
    ON_UPDATE_COMMAND_UI(IDC_STATIC5, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_EDIT5, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_SPIN5, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_STATIC6, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_STATIC7, OnUpdateAnimationRate)
    ON_UPDATE_COMMAND_UI(IDC_EDIT6, OnUpdateAnimationRate)
    ON_UPDATE_COMMAND_UI(IDC_SPIN6, OnUpdateAnimationRate)
    ON_UPDATE_COMMAND_UI(IDC_STATIC8, OnUpdateAnimationRate)
    ON_UPDATE_COMMAND_UI(IDC_CHECK_ALLOW_DROPPING_SUBPIC, OnUpdateAllowDroppingSubpic)
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()


// CPPageSubtitles message handlers

static int TranslateResSettingToUIPos(int nResSetting)
{
    switch (nResSetting) {
        default:
            ASSERT(FALSE);
        // no break
        case 0:
            return 0;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return nResSetting + 5;
        case 6:
        case 7:
        case 8:
        case 9:
            return nResSetting - 4;
        case 10:
            return 1;
    }
}

static int TranslateResUIPosToSetting(int nResPos)
{
    switch (nResPos) {
        default:
            ASSERT(FALSE);
        // no break
        case 0:
            return 0;
        case 1:
            return 10;
        case 2:
        case 3:
        case 4:
        case 5:
            return nResPos + 4;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            return nResPos - 5;
    }
}

BOOL CPPageSubtitles::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_COMBO1);

    const CAppSettings& s = AfxGetAppSettings();
    const CRenderersSettings& r = GetRenderersSettings();

    m_bOverridePlacement = s.fOverridePlacement;
    m_nHorPos = s.nHorPos;
    m_horPosCtrl.SetRange32(-40, 140);
    m_nVerPos = s.nVerPos;
    m_verPosCtrl.SetRange32(140, -40);
    m_nSPQSize = r.subPicQueueSettings.nSize;
    m_SPQSizeCtrl.SetRange32(0, 120);
    m_cbSPQMaxRes.AddString(_T("Desktop"));
    m_cbSPQMaxRes.AddString(_T("Video"));
    m_cbSPQMaxRes.AddString(_T("2560x1600"));
    m_cbSPQMaxRes.AddString(_T("1920x1080"));
    m_cbSPQMaxRes.AddString(_T("1320x900"));
    m_cbSPQMaxRes.AddString(_T("1280x720"));
    m_cbSPQMaxRes.AddString(_T("1024x768"));
    m_cbSPQMaxRes.AddString(_T("800x600"));
    m_cbSPQMaxRes.AddString(_T("640x480"));
    m_cbSPQMaxRes.AddString(_T("512x384"));
    m_cbSPQMaxRes.AddString(_T("384x288"));
    m_cbSPQMaxRes.SetCurSel(TranslateResSettingToUIPos(r.subPicQueueSettings.nMaxRes));
    m_bDisableSubtitleAnimation = r.subPicQueueSettings.bDisableSubtitleAnimation;
    m_nRenderAtWhenAnimationIsDisabled = r.subPicQueueSettings.nRenderAtWhenAnimationIsDisabled;
    m_bAllowDroppingSubpic = r.subPicQueueSettings.bAllowDroppingSubpic;
    m_renderAtCtrl.SetRange32(0, 100);
    m_nAnimationRate = r.subPicQueueSettings.nAnimationRate;
    m_animationRateCtrl.SetRange32(10, 100);
    UDACCEL accels[] = { { 0, 5 }, { 2, 10 }, { 5, 20 } };
    m_animationRateCtrl.SetAccel(_countof(accels), accels);
    m_nSubDelayStep = s.nSubDelayStep;
    m_bSubtitleARCompensation = s.bSubtitleARCompensation;

    CorrectComboBoxHeaderWidth(GetDlgItem(IDC_CHECK3));

    UpdateData(FALSE);

    if (AfxGetAppSettings().bMPCThemeLoaded) {
        themedToolTip.Create(this, TTS_NOPREFIX | TTS_ALWAYSTIP);
        themedToolTip.Activate(TRUE);
        themedToolTip.SetDelayTime(TTDT_AUTOPOP, 10000);
        //must add manually the ones we support.
        themedToolTip.AddTool(GetDlgItem(IDC_EDIT4), LPSTR_TEXTCALLBACK);
    } else {
        EnableToolTips(TRUE);
    }
    CreateToolTip();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageSubtitles::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();
    CRenderersSettings& r = GetRenderersSettings();

    r.subPicQueueSettings.nSize = m_nSPQSize;
    s.nSubDelayStep = m_nSubDelayStep;
    r.subPicQueueSettings.nMaxRes = TranslateResUIPosToSetting(m_cbSPQMaxRes.GetCurSel());
    r.subPicQueueSettings.bDisableSubtitleAnimation = !!m_bDisableSubtitleAnimation;
    r.subPicQueueSettings.nRenderAtWhenAnimationIsDisabled = std::max(0, std::min(m_nRenderAtWhenAnimationIsDisabled, 100));
    r.subPicQueueSettings.nAnimationRate = std::max(10, std::min(m_nAnimationRate, 100));
    r.subPicQueueSettings.bAllowDroppingSubpic = !!m_bAllowDroppingSubpic;

    if (s.bSubtitleARCompensation != !!m_bSubtitleARCompensation) {
        s.bSubtitleARCompensation = !!m_bSubtitleARCompensation;
        if (CMainFrame* pMainFrame = AfxGetMainFrame()) {
            pMainFrame->UpdateSubAspectRatioCompensation();
        }
    }

    if (s.fOverridePlacement != !!m_bOverridePlacement
            || s.nHorPos != m_nHorPos
            || s.nVerPos != m_nVerPos) {
        s.fOverridePlacement = !!m_bOverridePlacement;
        s.nHorPos = m_nHorPos;
        s.nVerPos = m_nVerPos;
        if (CMainFrame* pMainFrame = AfxGetMainFrame()) {
            pMainFrame->UpdateSubOverridePlacement();
        }
    }

    return __super::OnApply();
}

void CPPageSubtitles::OnUpdatePosOverride(CCmdUI* pCmdUI)
{
    UpdateData();
    pCmdUI->Enable(m_bOverridePlacement);
}

void CPPageSubtitles::OnSubDelayStep()
{
    // If incorrect number, revert modifications
    if (!UpdateData()) {
        UpdateData(FALSE);
    }

    SetModified();
}

void CPPageSubtitles::OnUpdateRenderAtWhenAnimationIsDisabled(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK_NO_SUB_ANIM));
}

void CPPageSubtitles::OnUpdateAnimationRate(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsDlgButtonChecked(IDC_CHECK_NO_SUB_ANIM));
}

void CPPageSubtitles::OnUpdateAllowDroppingSubpic(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetDlgItemInt(IDC_SUBPIC_TO_BUFFER));
}

BOOL CPPageSubtitles::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    LPTOOLTIPTEXT pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMHDR);

    UINT_PTR nID = pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND) {
        nID = ::GetDlgCtrlID((HWND)nID);
    }

    BOOL bRet = FALSE;

    static CString m_strToolTip;

    switch (nID) {
        case IDC_EDIT4:
            auto substituteEmpty = [](CString && hotkey) -> CString && {
                if (hotkey.IsEmpty())
                {
                    hotkey.LoadString(IDS_HOTKEY_NOT_DEFINED);
                }
                return std::move(hotkey);
            };
            ::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 320);
            m_strToolTip.Format(IDS_SUBTITLE_DELAY_STEP_TOOLTIP,
                                substituteEmpty(CPPageAccelTbl::MakeAccelShortcutLabel(ID_SUB_DELAY_DOWN)).GetString(),
                                substituteEmpty(CPPageAccelTbl::MakeAccelShortcutLabel(ID_SUB_DELAY_UP)).GetString());
            pTTT->lpszText = (LPTSTR)(LPCTSTR)m_strToolTip;
            bRet = true;
            break;
    }

    return bRet;
}

BOOL CPPageSubtitles::PreTranslateMessage(MSG* pMsg)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (IsWindow(themedToolTip)) {
            themedToolTip.RelayEvent(pMsg);
        }
    }
    return __super::PreTranslateMessage(pMsg);
}
