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

// StyleEditorDialog.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include <afxdlgs.h>
#include "StyleEditorDialog.h"

IMPLEMENT_DYNAMIC(CColorStatic, CStatic)

BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
END_MESSAGE_MAP()


// CStyleEditorDialog dialog

IMPLEMENT_DYNAMIC(CStyleEditorDialog, CDialog)
CStyleEditorDialog::CStyleEditorDialog(CString title, STSStyle* pstss, CWnd* pParent /*=NULL*/)
	: CDialog(CStyleEditorDialog::IDD, pParent)
	, m_title(title)
	, m_stss(*pstss)
	, m_pParent(pParent)
	, m_iCharset(0)
	, m_spacing(0)
	, m_angle(0)
	, m_scalex(0)
	, m_scaley(0)
	, m_borderstyle(0)
	, m_borderwidth(0)
	, m_shadowdepth(0)
	, m_screenalignment(0)
	, m_margin(0,0,0,0)
	, m_linkalphasliders(FALSE)
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
	DDX_Control(pDX, IDC_COMBO1, m_charset);
	DDX_Text(pDX, IDC_EDIT3, m_spacing);
	DDX_Control(pDX, IDC_SPIN3, m_spacingspin);
	DDX_Text(pDX, IDC_EDIT11, m_angle);
	DDX_Control(pDX, IDC_SPIN10, m_anglespin);
	DDX_Text(pDX, IDC_EDIT5, m_scalex);
	DDX_Control(pDX, IDC_SPIN4, m_scalexspin);
	DDX_Text(pDX, IDC_EDIT6, m_scaley);
	DDX_Control(pDX, IDC_SPIN5, m_scaleyspin);
	DDX_Radio(pDX, IDC_RADIO1, m_borderstyle);
	DDX_Text(pDX, IDC_EDIT1, m_borderwidth);
	DDX_Control(pDX, IDC_SPIN1, m_borderwidthspin);
	DDX_Text(pDX, IDC_EDIT2, m_shadowdepth);
	DDX_Control(pDX, IDC_SPIN2, m_shadowdepthspin);
	DDX_Radio(pDX, IDC_RADIO3, m_screenalignment);
	DDX_Text(pDX, IDC_EDIT7, m_margin.left);
	DDX_Control(pDX, IDC_SPIN6, m_marginleftspin);
	DDX_Text(pDX, IDC_EDIT8, m_margin.right);
	DDX_Control(pDX, IDC_SPIN7, m_marginrightspin);
	DDX_Text(pDX, IDC_EDIT9, m_margin.top);
	DDX_Control(pDX, IDC_SPIN8, m_margintopspin);
	DDX_Text(pDX, IDC_EDIT10, m_margin.bottom);
	DDX_Control(pDX, IDC_SPIN9, m_marginbottomspin);
	DDX_Control(pDX, IDC_COLORPRI, m_color[0]);
	DDX_Control(pDX, IDC_COLORSEC, m_color[1]);
	DDX_Control(pDX, IDC_COLOROUTL, m_color[2]);
	DDX_Control(pDX, IDC_COLORSHAD, m_color[3]);
	DDX_Slider(pDX, IDC_SLIDER2, m_alpha[0]);
	DDX_Slider(pDX, IDC_SLIDER3, m_alpha[1]);
	DDX_Slider(pDX, IDC_SLIDER5, m_alpha[2]);
	DDX_Slider(pDX, IDC_SLIDER6, m_alpha[3]);
	DDX_Control(pDX, IDC_SLIDER2, m_alphasliders[0]);
	DDX_Control(pDX, IDC_SLIDER3, m_alphasliders[1]);
	DDX_Control(pDX, IDC_SLIDER5, m_alphasliders[2]);
	DDX_Control(pDX, IDC_SLIDER6, m_alphasliders[3]);
	DDX_Check(pDX, IDC_CHECK1, m_linkalphasliders);
}

void CStyleEditorDialog::UpdateControlData(bool fSave)
{
	if(fSave)
	{
		UpdateData();

		if(m_iCharset >= 0) m_stss.charSet = m_charset.GetItemData(m_iCharset);
		m_stss.fontSpacing = m_spacing;
		m_stss.fontAngleZ = m_angle;
		m_stss.fontScaleX = m_scalex;
		m_stss.fontScaleY = m_scaley;

		m_stss.borderStyle = m_borderstyle;
		m_stss.outlineWidth = m_borderwidth;
		m_stss.shadowDepth = m_shadowdepth;

		m_stss.scrAlignment = m_screenalignment+1;
		m_stss.marginRect = m_margin;

		for(int i = 0; i < 4; i++) m_stss.alpha[i] = 255-m_alpha[i];
	}
	else
	{
		m_font.SetWindowText(m_stss.fontName);
		m_iCharset = -1;
		for(int i = 0; i < CharSetLen; i++)
		{
			CString str;
			str.Format(_T("%s (%d)"), CharSetNames[i], CharSetList[i]);
			m_charset.AddString(str);
			m_charset.SetItemData(i, CharSetList[i]);
			if(m_stss.charSet == CharSetList[i]) m_iCharset = i;
		}
		// TODO: allow floats in these edit boxes
		m_spacing = m_stss.fontSpacing;
		m_spacingspin.SetRange32(-10000, 10000);
		while(m_stss.fontAngleZ < 0) m_stss.fontAngleZ += 360;
		m_angle = fmod(m_stss.fontAngleZ, 360);
		m_anglespin.SetRange32(0, 359);
		m_scalex = m_stss.fontScaleX;
		m_scalexspin.SetRange32(-10000, 10000);
		m_scaley = m_stss.fontScaleY;
		m_scaleyspin.SetRange32(-10000, 10000);

		m_borderstyle = m_stss.borderStyle;
		m_borderwidth = m_stss.outlineWidth;
		m_borderwidthspin.SetRange32(0, 10000);
		m_shadowdepth = m_stss.shadowDepth;
		m_shadowdepthspin.SetRange32(0, 10000);

		m_screenalignment = m_stss.scrAlignment-1;
		m_margin = m_stss.marginRect;
		m_marginleftspin.SetRange32(-10000, 10000);
		m_marginrightspin.SetRange32(-10000, 10000);
		m_margintopspin.SetRange32(-10000, 10000);
		m_marginbottomspin.SetRange32(-10000, 10000);

		for(int i = 0; i < 4; i++)
		{
			m_color[i].SetColorPtr(&m_stss.colors[i]);
			m_alpha[i] = 255-m_stss.alpha[i];
			m_alphasliders[i].SetRange(0, 255);
		}
		
		m_linkalphasliders = FALSE;

		UpdateData(FALSE);
	}
}

void CStyleEditorDialog::AskColor(int i)
{
	CColorDialog dlg(m_stss.colors[i]);
	dlg.m_cc.Flags |= CC_FULLOPEN;
	if(dlg.DoModal() == IDOK)
	{
		m_stss.colors[i] = dlg.m_cc.rgbResult;
		m_color[i].Invalidate();
	}
}

BEGIN_MESSAGE_MAP(CStyleEditorDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_STN_CLICKED(IDC_COLORPRI, OnStnClickedColorpri)
	ON_STN_CLICKED(IDC_COLORSEC, OnStnClickedColorsec)
	ON_STN_CLICKED(IDC_COLOROUTL, OnStnClickedColoroutl)
	ON_STN_CLICKED(IDC_COLORSHAD, OnStnClickedColorshad)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CStyleEditorDialog message handlers

BOOL CStyleEditorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(_T("Style Editor - \"") + m_title + _T("\""));

	UpdateControlData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CStyleEditorDialog::OnOK()
{
	UpdateControlData(true);

	CDialog::OnOK();
}

void CStyleEditorDialog::OnBnClickedButton1()
{
	LOGFONT lf;
	lf <<= m_stss;

	CFontDialog dlg(&lf, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_FORCEFONTEXIST|CF_SCALABLEONLY|CF_EFFECTS);
	if(dlg.DoModal() == IDOK)
	{
		CString str(lf.lfFaceName);
		if(str.GetLength() > 16) str = str.Left(14) + _T("...");
		m_font.SetWindowText(str);

		for(int i = 0, j = m_charset.GetCount(); i < j; i++)
		{
			if(m_charset.GetItemData(i) == lf.lfCharSet)
			{
				m_charset.SetCurSel(i);
				break;
			}
		}

		m_stss = lf;
	}
}

void CStyleEditorDialog::OnStnClickedColorpri()
{
	AskColor(0);
}

void CStyleEditorDialog::OnStnClickedColorsec()
{
	AskColor(1);
}

void CStyleEditorDialog::OnStnClickedColoroutl()
{
	AskColor(2);
}

void CStyleEditorDialog::OnStnClickedColorshad()
{
	AskColor(3);
}

void CStyleEditorDialog::OnBnClickedCheck1()
{
	UpdateData();

	int avg = 0;
	for(int i = 0; i < 4; i++) avg += m_alphasliders[i].GetPos();
	avg /= 4;
	for(int i = 0; i < 4; i++) m_alphasliders[i].SetPos(avg);
}

void CStyleEditorDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(m_linkalphasliders && pScrollBar)
	{
		int pos = ((CSliderCtrl*)pScrollBar)->GetPos();
		for(int i = 0; i < 4; i++) m_alphasliders[i].SetPos(pos);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
