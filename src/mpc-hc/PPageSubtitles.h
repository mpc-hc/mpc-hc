/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016 see Authors.txt
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

#pragma once

#include "../Subtitles/STS.h"
#include "CMPCThemePPageBase.h"


// CPPageSubtitles dialog

class CPPageSubtitles : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageSubtitles)

private:
    BOOL m_bOverridePlacement;
    int m_nHorPos;
    CMPCThemeSpinButtonCtrl m_horPosCtrl;
    int m_nVerPos;
    CMPCThemeSpinButtonCtrl m_verPosCtrl;
    int m_nSPQSize;
    CMPCThemeSpinButtonCtrl m_SPQSizeCtrl;
    CMPCThemeComboBox m_cbSPQMaxRes;
    BOOL m_bDisableSubtitleAnimation;
    int m_nRenderAtWhenAnimationIsDisabled;
    CMPCThemeSpinButtonCtrl m_renderAtCtrl;
    int m_nAnimationRate;
    CMPCThemeSpinButtonCtrl m_animationRateCtrl;
    BOOL m_bAllowDroppingSubpic;
    int m_nSubDelayStep;
    BOOL m_bSubtitleARCompensation;

public:
    CPPageSubtitles();
    virtual ~CPPageSubtitles();

    // Dialog Data
    enum { IDD = IDD_PPAGESUBTITLES };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnUpdatePosOverride(CCmdUI* pCmdUI);
    afx_msg void OnUpdateRenderAtWhenAnimationIsDisabled(CCmdUI* pCmdUI);
    afx_msg void OnUpdateAnimationRate(CCmdUI* pCmdUI);
    afx_msg void OnUpdateAllowDroppingSubpic(CCmdUI* pCmdUI);
    afx_msg void OnSubDelayStep();
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
};
