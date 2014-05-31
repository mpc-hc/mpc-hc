/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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


// CPPageSubtitles dialog

IMPLEMENT_DYNAMIC(CPPageSubtitles, CPPageBase)
CPPageSubtitles::CPPageSubtitles()
    : CPPageBase(CPPageSubtitles::IDD, CPPageSubtitles::IDD)
    , m_fOverridePlacement(FALSE)
    , m_nHorPos(0)
    , m_nVerPos(0)
    , m_nSPCSize(0)
    , m_fSPCPow2Tex(FALSE)
    , m_bDisableSubtitleAnimation(FALSE)
    , m_nRenderAtWhenAnimationIsDisabled(50)
    , m_nSubDelayInterval(0)
    , m_bSubtitleARCompensation(TRUE)
{
}

CPPageSubtitles::~CPPageSubtitles()
{
}

void CPPageSubtitles::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK3, m_fOverridePlacement);
    DDX_Text(pDX, IDC_EDIT2, m_nHorPos);
    DDX_Control(pDX, IDC_SPIN2, m_nHorPosCtrl);
    DDX_Text(pDX, IDC_EDIT3, m_nVerPos);
    DDX_Control(pDX, IDC_SPIN3, m_nVerPosCtrl);
    DDX_Text(pDX, IDC_EDIT1, m_nSPCSize);
    DDX_Control(pDX, IDC_SPIN1, m_nSPCSizeCtrl);
    DDX_Control(pDX, IDC_COMBO1, m_spmaxres);
    DDX_Control(pDX, IDC_EDIT2, m_nHorPosEdit);
    DDX_Control(pDX, IDC_EDIT3, m_nVerPosEdit);
    DDX_Check(pDX, IDC_CHECK_SPCPOW2TEX, m_fSPCPow2Tex);
    DDX_Check(pDX, IDC_CHECK_NO_SUB_ANIM, m_bDisableSubtitleAnimation);
    DDX_Text(pDX, IDC_EDIT5, m_nRenderAtWhenAnimationIsDisabled);
    DDX_Control(pDX, IDC_SPIN5, m_renderAtCtrl);
    DDX_Text(pDX, IDC_EDIT4, m_nSubDelayInterval);
    DDX_Check(pDX, IDC_CHECK_SUB_AR_COMPENSATION, m_bSubtitleARCompensation);
}


BEGIN_MESSAGE_MAP(CPPageSubtitles, CPPageBase)
    ON_UPDATE_COMMAND_UI(IDC_EDIT2, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_SPIN2, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_EDIT3, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_SPIN3, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdatePosOverride)
    ON_UPDATE_COMMAND_UI(IDC_STATIC4, OnUpdatePosOverride)
    ON_EN_CHANGE(IDC_EDIT4, OnSubDelayInterval)
    ON_UPDATE_COMMAND_UI(IDC_STATIC5, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_EDIT5, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_SPIN5, OnUpdateRenderAtWhenAnimationIsDisabled)
    ON_UPDATE_COMMAND_UI(IDC_STATIC6, OnUpdateRenderAtWhenAnimationIsDisabled)
END_MESSAGE_MAP()


// CPPageSubtitles message handlers

int TranslateResIn(int _In)
{
    switch (_In) {
        case 0:
            return 0;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return _In + 5;
        case 6:
        case 7:
        case 8:
        case 9:
            return _In - 4;
        case 10:
            return 1;
    }
    return _In;
}

int TranslateResOut(int _In)
{
    switch (_In) {
        case 0:
            return 0;
        case 1:
            return 10;
        case 2:
        case 3:
        case 4:
        case 5:
            return _In + 4;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            return _In - 5;
    }
    return _In;
}

BOOL CPPageSubtitles::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_COMBO1);

    const CAppSettings& s = AfxGetAppSettings();
    const CRenderersSettings& r = GetRenderersSettings();

    m_fOverridePlacement = s.fOverridePlacement;
    m_nHorPos = s.nHorPos;
    m_nHorPosCtrl.SetRange32(-40, 140);
    m_nVerPos = s.nVerPos;
    m_nVerPosCtrl.SetRange32(140, -40);
    m_nSPCSize = r.subPicQueueSettings.nSize;
    m_nSPCSizeCtrl.SetRange32(0, 60);
    m_spmaxres.AddString(_T("Desktop"));
    m_spmaxres.AddString(_T("Video"));
    m_spmaxres.AddString(_T("2560x1600"));
    m_spmaxres.AddString(_T("1920x1080"));
    m_spmaxres.AddString(_T("1320x900"));
    m_spmaxres.AddString(_T("1280x720"));
    m_spmaxres.AddString(_T("1024x768"));
    m_spmaxres.AddString(_T("800x600"));
    m_spmaxres.AddString(_T("640x480"));
    m_spmaxres.AddString(_T("512x384"));
    m_spmaxres.AddString(_T("384x288"));
    m_spmaxres.SetCurSel(TranslateResIn(r.subPicQueueSettings.nMaxRes));
    m_fSPCPow2Tex = r.subPicQueueSettings.bPow2Tex;
    m_bDisableSubtitleAnimation = r.subPicQueueSettings.bDisableSubtitleAnimation;
    m_nRenderAtWhenAnimationIsDisabled = r.subPicQueueSettings.nRenderAtWhenAnimationIsDisabled;
    m_renderAtCtrl.SetRange32(0, 100);
    m_nSubDelayInterval = s.nSubDelayInterval;
    m_bSubtitleARCompensation = s.bSubtitleARCompensation;

    UpdateData(FALSE);

    CreateToolTip();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageSubtitles::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();
    CRenderersSettings& r = GetRenderersSettings();

    r.subPicQueueSettings.nSize = m_nSPCSize;
    s.nSubDelayInterval = m_nSubDelayInterval;
    r.subPicQueueSettings.nMaxRes = TranslateResOut(m_spmaxres.GetCurSel());
    r.subPicQueueSettings.bPow2Tex = !!m_fSPCPow2Tex;
    r.subPicQueueSettings.bDisableSubtitleAnimation = !!m_bDisableSubtitleAnimation;
    r.subPicQueueSettings.nRenderAtWhenAnimationIsDisabled = std::max(0, std::min(m_nRenderAtWhenAnimationIsDisabled, 100));

    if (s.bSubtitleARCompensation != !!m_bSubtitleARCompensation) {
        s.bSubtitleARCompensation = !!m_bSubtitleARCompensation;
        if (auto pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd())) {
            pMainFrame->UpdateSubAspectRatioCompensation();
        }
    }

    if (s.fOverridePlacement != !!m_fOverridePlacement
            || s.nHorPos != m_nHorPos
            || s.nVerPos != m_nVerPos) {
        s.fOverridePlacement = !!m_fOverridePlacement;
        s.nHorPos = m_nHorPos;
        s.nVerPos = m_nVerPos;
        if (auto pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd())) {
            pMainFrame->UpdateSubOverridePlacement();
        }
    }

    return __super::OnApply();
}

void CPPageSubtitles::OnUpdatePosOverride(CCmdUI* pCmdUI)
{
    UpdateData();
    pCmdUI->Enable(m_fOverridePlacement);
}

void CPPageSubtitles::OnSubDelayInterval()
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
