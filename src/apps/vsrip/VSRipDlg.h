/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "VSRipFileDlg.h"
#include "VSRipPGCDlg.h"
#include "VSRipIndexingDlg.h"

// CVSRipDlg dialog
class CVSRipDlg : public CDialog
{
// Construction
public:
	CVSRipDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CVSRipDlg();

// Dialog Data
	enum { IDD = IDD_VSRIP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:
	HICON m_hIcon;

	CStatic m_dlgrect;
	CStatic m_hdrline;

	CAutoPtrList<CVSRipPage> m_dlgs;
	POSITION m_dlgpos;
	void ShowNext(), ShowPrev();
	POSITION GetNext(), GetPrev();

	CComPtr<IVSFRipper> m_pVSFRipper;

	// Generated message map functions
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg void OnKickIdle();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPrev();
	afx_msg void OnUpdatePrev(CCmdUI* pCmdUI);
	afx_msg void OnNext();
	afx_msg void OnUpdateNext(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnUpdateClose(CCmdUI* pCmdUI);
	CStatic m_ftrline;
};
