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

#include "../../subtitles/STS.h"


// CPPageSubtitles dialog

class CPPageSubtitles : public CPPageBase
{
	DECLARE_DYNAMIC(CPPageSubtitles)

public:
	CPPageSubtitles();
	virtual ~CPPageSubtitles();

	BOOL m_fOverridePlacement;
	int m_nHorPos;
	CEdit m_nHorPosEdit;
	CSpinButtonCtrl m_nHorPosCtrl;
	int m_nVerPos;
	CEdit m_nVerPosEdit;
	CSpinButtonCtrl m_nVerPosCtrl;
	int m_nSPCSize;
	CSpinButtonCtrl m_nSPCSizeCtrl;
	CComboBox m_spmaxres;
	BOOL m_fSPCPow2Tex;
	BOOL m_fSPCAllowAnimationWhenBuffering;
	int m_nSubDelayInterval;

// Dialog Data
	enum { IDD = IDD_PPAGESUBTITLES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	void	OnSubDelayInterval();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnUpdatePosOverride(CCmdUI* pCmdUI);
};
