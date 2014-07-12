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

#pragma once

#include "resource.h"
#include <afxwin.h>
#include <afxcmn.h>
#include "../../../mpc-hc/ColorButton.h"
#include "../../../Subtitles/STS.h"

// CStyleEditorDialog dialog

class CStyleEditorDialog : public CDialog
{
    DECLARE_DYNAMIC(CStyleEditorDialog)

private:
    CString m_title;
    STSStyle m_stss;

    CButton m_font;
    int m_iCharset;
    CComboBox m_cbCharset;
    int m_spacing;
    CSpinButtonCtrl m_spacingSpin;
    int m_angle;
    CSpinButtonCtrl m_angleSpin;
    int m_scalex;
    CSpinButtonCtrl m_scalexSpin;
    int m_scaley;
    CSpinButtonCtrl m_scaleySpin;
    int m_borderStyle;
    int m_borderWidth;
    CSpinButtonCtrl m_borderWidthSpin;
    int m_shadowDepth;
    CSpinButtonCtrl m_shadowDepthSpin;
    int m_screenAlignment;
    CRect m_margin;
    CSpinButtonCtrl m_marginLeftSpin;
    CSpinButtonCtrl m_marginRightSpin;
    CSpinButtonCtrl m_marginTopSpin;
    CSpinButtonCtrl m_marginBottomSpin;
    std::array<CColorButton, 4> m_color;
    std::array<int, 4> m_alpha;
    std::array<CSliderCtrl, 4> m_alphaSliders;
    BOOL m_bLinkAlphaSliders;

    void AskColor(int i);

public:
    CStyleEditorDialog(CString title, STSStyle* pstss, CWnd* pParent = nullptr);
    virtual ~CStyleEditorDialog();

    void GetStyle(STSStyle& stss) const { stss = m_stss; }

    // Dialog Data
    enum { IDD = IDD_STYLEDIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnChooseFont();
    afx_msg void OnChoosePrimaryColor();
    afx_msg void OnChooseSecondaryColor();
    afx_msg void OnChooseOutlineColor();
    afx_msg void OnChooseShadowColor();
    afx_msg void OnLinkAlphaSlidersChanged();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
