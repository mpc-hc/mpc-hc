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

#include "PPageBase.h"
#include "afxcmn.h"

// CPPageCasimir dialog

class CPPageCasimir : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageCasimir)

private:
	CStringArray m_AudioRendererDisplayNames;

	float		m_dBrightness;
	float		m_dContrast;
	float		m_dHue;
	float		m_dSaturation;

public:
	CPPageCasimir();
	virtual ~CPPageCasimir();

// Dialog Data
	enum { IDD = IDD_PPAGECASIMIR };
	BOOL m_fMonitorAutoRefreshRate;
	BOOL m_fD3DFullscreen;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	CSliderCtrl m_SliContrast;
	CSliderCtrl m_SliBrightness;
	CSliderCtrl m_SliHue;
	CSliderCtrl m_SliSaturation;
	afx_msg void OnBnClickedReset();
	BOOL m_fRememberDVDPos;
	BOOL m_fRememberFilePos;
	virtual void OnCancel();
};
