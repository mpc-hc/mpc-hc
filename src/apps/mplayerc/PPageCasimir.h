/*
 * $Id$
 *
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
#include "afxcmn.h"


// CPPageCasimir dialog

class CPPageCasimir : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageCasimir)

private:
	float m_dBrightness;
	float m_dContrast;
	float m_dHue;
	float m_dSaturation;

public:
	CPPageCasimir();
	virtual ~CPPageCasimir();

// Dialog Data
	enum { IDD = IDD_PPAGECASIMIR };

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
	virtual void OnCancel();
};
