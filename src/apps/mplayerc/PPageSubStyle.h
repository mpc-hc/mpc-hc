/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "PPageBase.h"
#include "../../Subtitles/STS.h"


class CColorStatic : public CStatic
{
	//	DECLARE_DYNAMIC(CColorStatic)

	COLORREF* m_pColor;

public:
	CColorStatic(CWnd* pParent = NULL) : m_pColor(NULL) {}
	virtual ~CColorStatic() {}

	void SetColorPtr(COLORREF* pColor) {
		m_pColor = pColor;
	}

	//	DECLARE_MESSAGE_MAP()

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
		CRect r;
		GetClientRect(r);
		CDC::FromHandle(lpDrawItemStruct->hDC)->FillSolidRect(r, m_pColor ? *m_pColor : ::GetSysColor(COLOR_BTNFACE));
	}
};

// CPPageSubStyle dialog

class CPPageSubStyle : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageSubStyle)

private:
	CString m_title;
	STSStyle m_stss;
	bool m_fUseDefaultStyle;

	void AskColor(int i);

public:
	CPPageSubStyle();
	virtual ~CPPageSubStyle();

	void InitStyle(CString title, STSStyle& stss);
	void GetStyle(STSStyle& stss) {
		stss = m_stss;
	}

	// Dialog Data
	enum { IDD = IDD_PPAGESUBSTYLE };
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
	BOOL m_relativeTo;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButton1();
	afx_msg void OnStnClickedColorpri();
	afx_msg void OnStnClickedColorsec();
	afx_msg void OnStnClickedColoroutl();
	afx_msg void OnStnClickedColorshad();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
public:
};
