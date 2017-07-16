/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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
#include <algorithm>
#include <afxdlgs.h>
#include "StyleEditorDialog.h"

// CStyleEditorDialog dialog

IMPLEMENT_DYNAMIC(CStyleEditorDialog, CDialog)
CStyleEditorDialog::CStyleEditorDialog(CString title, STSStyle* pstss, CWnd* pParent /*=nullptr*/)
    : CDialog(CStyleEditorDialog::IDD, pParent)
    , m_title(title)
    , m_stss(*pstss)
    , m_iCharset(0)
    , m_spacing(0)
    , m_angle(0)
    , m_scalex(0)
    , m_scaley(0)
    , m_borderStyle(0)
    , m_borderWidth(0)
    , m_shadowDepth(0)
    , m_screenAlignment(0)
    , m_margin(0, 0, 0, 0)
    , m_bLinkAlphaSliders(FALSE)
{
}

CStyleEditorDialog::~CStyleEditorDialog()
{
}

void CStyleEditorDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON1, m_font);
    DDX_CBIndex(pDX, IDC_COMBO1, m_iCharset);
    DDX_Control(pDX, IDC_COMBO1, m_cbCharset);
    DDX_Text(pDX, IDC_EDIT3, m_spacing);
    DDX_Control(pDX, IDC_SPIN3, m_spacingSpin);
    DDX_Text(pDX, IDC_EDIT11, m_angle);
    DDX_Control(pDX, IDC_SPIN10, m_angleSpin);
    DDX_Text(pDX, IDC_EDIT5, m_scalex);
    DDX_Control(pDX, IDC_SPIN4, m_scalexSpin);
    DDX_Text(pDX, IDC_EDIT6, m_scaley);
    DDX_Control(pDX, IDC_SPIN5, m_scaleySpin);
    DDX_Radio(pDX, IDC_RADIO1, m_borderStyle);
    DDX_Text(pDX, IDC_EDIT1, m_borderWidth);
    DDX_Control(pDX, IDC_SPIN1, m_borderWidthSpin);
    DDX_Text(pDX, IDC_EDIT2, m_shadowDepth);
    DDX_Control(pDX, IDC_SPIN2, m_shadowDepthSpin);
    DDX_Radio(pDX, IDC_RADIO3, m_screenAlignment);
    DDX_Text(pDX, IDC_EDIT7, m_margin.left);
    DDX_Control(pDX, IDC_SPIN6, m_marginLeftSpin);
    DDX_Text(pDX, IDC_EDIT8, m_margin.right);
    DDX_Control(pDX, IDC_SPIN7, m_marginRightSpin);
    DDX_Text(pDX, IDC_EDIT9, m_margin.top);
    DDX_Control(pDX, IDC_SPIN8, m_marginTopSpin);
    DDX_Text(pDX, IDC_EDIT10, m_margin.bottom);
    DDX_Control(pDX, IDC_SPIN9, m_marginBottomSpin);
    DDX_Control(pDX, IDC_COLORPRI, m_color[0]);
    DDX_Control(pDX, IDC_COLORSEC, m_color[1]);
    DDX_Control(pDX, IDC_COLOROUTL, m_color[2]);
    DDX_Control(pDX, IDC_COLORSHAD, m_color[3]);
    DDX_Slider(pDX, IDC_SLIDER2, m_alpha[0]);
    DDX_Slider(pDX, IDC_SLIDER3, m_alpha[1]);
    DDX_Slider(pDX, IDC_SLIDER5, m_alpha[2]);
    DDX_Slider(pDX, IDC_SLIDER6, m_alpha[3]);
    DDX_Control(pDX, IDC_SLIDER2, m_alphaSliders[0]);
    DDX_Control(pDX, IDC_SLIDER3, m_alphaSliders[1]);
    DDX_Control(pDX, IDC_SLIDER5, m_alphaSliders[2]);
    DDX_Control(pDX, IDC_SLIDER6, m_alphaSliders[3]);
    DDX_Check(pDX, IDC_CHECK1, m_bLinkAlphaSliders);
}

void CStyleEditorDialog::AskColor(int i)
{
    CColorDialog dlg(m_stss.colors[i]);
    dlg.m_cc.Flags |= CC_FULLOPEN;
    if (dlg.DoModal() == IDOK) {
        m_stss.colors[i] = dlg.m_cc.rgbResult;
        m_color[i].SetColor(dlg.m_cc.rgbResult);
    }
}

BEGIN_MESSAGE_MAP(CStyleEditorDialog, CDialog)
    ON_BN_CLICKED(IDC_BUTTON1, OnChooseFont)
    ON_BN_CLICKED(IDC_COLORPRI, OnChoosePrimaryColor)
    ON_BN_CLICKED(IDC_COLORSEC, OnChooseSecondaryColor)
    ON_BN_CLICKED(IDC_COLOROUTL, OnChooseOutlineColor)
    ON_BN_CLICKED(IDC_COLORSHAD, OnChooseShadowColor)
    ON_BN_CLICKED(IDC_CHECK1, OnLinkAlphaSlidersChanged)
    ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CStyleEditorDialog message handlers

BOOL CStyleEditorDialog::OnInitDialog()
{
    __super::OnInitDialog();

    SetWindowText(_T("Style Editor - \"") + m_title + _T("\""));

    m_font.SetWindowText(m_stss.fontName);
    m_iCharset = -1;
    for (int i = 0; i < CharSetLen; i++) {
        CString str;
        str.Format(_T("%s (%u)"), CharSetNames[i], CharSetList[i]);
        m_cbCharset.AddString(str);
        m_cbCharset.SetItemData(i, CharSetList[i]);
        if (m_stss.charSet == CharSetList[i]) {
            m_iCharset = i;
        }
    }

    // TODO: allow floats in these edit boxes
    m_spacing = (int)m_stss.fontSpacing;
    m_spacingSpin.SetRange32(-10000, 10000);
    while (m_stss.fontAngleZ < 0) {
        m_stss.fontAngleZ += 360;
    }
    m_angle = (int)std::fmod(m_stss.fontAngleZ, 360);
    m_angleSpin.SetRange32(0, 359);
    m_scalex = (int)m_stss.fontScaleX;
    m_scalexSpin.SetRange32(-10000, 10000);
    m_scaley = (int)m_stss.fontScaleY;
    m_scaleySpin.SetRange32(-10000, 10000);

    m_borderStyle = m_stss.borderStyle;
    m_borderWidth = (int)std::min(m_stss.outlineWidthX, m_stss.outlineWidthY);
    m_borderWidthSpin.SetRange32(0, 10000);
    m_shadowDepth = (int)std::min(m_stss.shadowDepthX, m_stss.shadowDepthY);
    m_shadowDepthSpin.SetRange32(0, 10000);

    m_screenAlignment = m_stss.scrAlignment - 1;
    m_margin = m_stss.marginRect;
    m_marginLeftSpin.SetRange32(-10000, 10000);
    m_marginRightSpin.SetRange32(-10000, 10000);
    m_marginTopSpin.SetRange32(-10000, 10000);
    m_marginBottomSpin.SetRange32(-10000, 10000);

    for (size_t i = 0; i < m_color.size(); i++) {
        m_color[i].SetColor(m_stss.colors[i]);
        m_alpha[i] = BYTE_MAX - m_stss.alpha[i];
        m_alphaSliders[i].SetRange(0, BYTE_MAX);
    }

    m_bLinkAlphaSliders = FALSE;

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CStyleEditorDialog::OnOK()
{
    UpdateData();

    if (m_iCharset >= 0) {
        m_stss.charSet = (int)m_cbCharset.GetItemData(m_iCharset);
    }
    m_stss.fontSpacing = m_spacing;
    m_stss.fontAngleZ = m_angle;
    m_stss.fontScaleX = m_scalex;
    m_stss.fontScaleY = m_scaley;

    m_stss.borderStyle = m_borderStyle;
    m_stss.outlineWidthX = m_stss.outlineWidthY = m_borderWidth;
    m_stss.shadowDepthX = m_stss.shadowDepthY = m_shadowDepth;

    m_stss.scrAlignment = m_screenAlignment + 1;
    m_stss.marginRect = m_margin;

    for (size_t i = 0; i < m_alpha.size(); i++) {
        ASSERT(m_alpha[i] <= BYTE_MAX);
        m_stss.alpha[i] = BYTE_MAX - BYTE(m_alpha[i]);
    }

    __super::OnOK();
}

void CStyleEditorDialog::OnChooseFont()
{
    UpdateData();

    LOGFONT lf;
    lf <<= m_stss;
    if (m_iCharset >= 0) {
        lf.lfCharSet = (BYTE)m_cbCharset.GetItemData(m_iCharset);
    }

    CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST | CF_SCALABLEONLY);
    if (dlg.DoModal() == IDOK) {
        CString str(lf.lfFaceName);
        if (str.GetLength() > 16) {
            str = str.Left(14) + _T("...");
        }
        m_font.SetWindowText(str);

        for (int i = 0, count = m_cbCharset.GetCount(); i < count; i++) {
            if (m_cbCharset.GetItemData(i) == lf.lfCharSet) {
                m_cbCharset.SetCurSel(i);
                break;
            }
        }

        m_stss = lf;
    }
}

void CStyleEditorDialog::OnChoosePrimaryColor()
{
    AskColor(0);
}

void CStyleEditorDialog::OnChooseSecondaryColor()
{
    AskColor(1);
}

void CStyleEditorDialog::OnChooseOutlineColor()
{
    AskColor(2);
}

void CStyleEditorDialog::OnChooseShadowColor()
{
    AskColor(3);
}

void CStyleEditorDialog::OnLinkAlphaSlidersChanged()
{
    UpdateData();

    if (m_bLinkAlphaSliders) {
        int avgAlpha = 0;
        for (const auto& alphaSlider : m_alphaSliders) {
            avgAlpha += alphaSlider.GetPos();
        }
        avgAlpha /= 4;
        for (auto& alphaSlider : m_alphaSliders) {
            alphaSlider.SetPos(avgAlpha);
        }
    }
}

void CStyleEditorDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if (m_bLinkAlphaSliders && pScrollBar) {
        int pos = ((CSliderCtrl*)pScrollBar)->GetPos();
        for (auto& alphaSlider : m_alphaSliders) {
            alphaSlider.SetPos(pos);
        }
    }

    __super::OnHScroll(nSBCode, nPos, pScrollBar);
}
