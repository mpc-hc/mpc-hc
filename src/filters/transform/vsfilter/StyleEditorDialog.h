/*
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "../../../subtitles/STS.h"

// CColorStatic dialog

class CColorStatic : public CStatic
{
    DECLARE_DYNAMIC(CColorStatic)

    COLORREF* m_pColor;

public:
    CColorStatic(CWnd* pParent = NULL) : m_pColor(NULL) {}
    virtual ~CColorStatic() {}

    void SetColorPtr(COLORREF* pColor)
    {
        m_pColor = pColor;
    }

    DECLARE_MESSAGE_MAP()

protected:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
    {
        CRect r;
        GetClientRect(r);
        CDC::FromHandle(lpDrawItemStruct->hDC)->FillSolidRect(r, m_pColor ? *m_pColor : ::GetSysColor(COLOR_BTNFACE));
    }
};

// CStyleEditorDialog dialog

class CStyleEditorDialog : public CDialog
{
    DECLARE_DYNAMIC(CStyleEditorDialog)

    CString m_title;
    CWnd* m_pParent;

    void UpdateControlData(bool fSave);
    void AskColor(int i);

public:
    CStyleEditorDialog(CString title, STSStyle* pstss, CWnd* pParent = NULL);   // standard constructor
    virtual ~CStyleEditorDialog();

// Dialog Data
    enum { IDD = IDD_STYLEDIALOG };

    STSStyle m_stss;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

public:
    CButton m_font;
    int m_iCharset;
    CComboBox m_charset;
    int m_spacing;
    CSpinButtonCtrl m_spacingspin;
    int m_angle;
    CSpinButtonCtrl m_anglespin;
    int m_scalex;
    CSpinButtonCtrl m_scalexspin;
    int m_scaley;
    CSpinButtonCtrl m_scaleyspin;
    int m_borderstyle;
    int m_borderwidth;
    CSpinButtonCtrl m_borderwidthspin;
    int m_shadowdepth;
    CSpinButtonCtrl m_shadowdepthspin;
    int m_screenalignment;
    CRect m_margin;
    CSpinButtonCtrl m_marginleftspin;
    CSpinButtonCtrl m_marginrightspin;
    CSpinButtonCtrl m_margintopspin;
    CSpinButtonCtrl m_marginbottomspin;
    CColorStatic m_color[4];
    int m_alpha[4];
    CSliderCtrl m_alphasliders[4];
    BOOL m_linkalphasliders;

    afx_msg void OnBnClickedButton1();
    afx_msg void OnStnClickedColorpri();
    afx_msg void OnStnClickedColorsec();
    afx_msg void OnStnClickedColoroutl();
    afx_msg void OnStnClickedColorshad();
    afx_msg void OnBnClickedCheck1();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
