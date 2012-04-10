/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include "LineNumberEdit.h"
#include "ShaderAutoCompleteDlg.h"
#include "mplayerc.h"
#include <ResizableLib/ResizableDialog.h>


// Q174667

class CShaderLabelComboBox : public CComboBox
{
public:
	CEdit m_edit;

	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
};

class CShaderEdit : public CLineNumberEdit
{
	int m_nEndChar;
	UINT_PTR m_nIDEvent;

public:
	CShaderEdit();
	~CShaderEdit();

	CShaderAutoCompleteDlg m_acdlg;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnUpdate();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

// CShaderEditorDlg dialog

class CPixelShaderCompiler;

class CShaderEditorDlg : public CResizableDialog
{
private:
	UINT m_nIDEventShader;

	bool m_fSplitterGrabbed;
	bool HitTestSplitter(CPoint p);

	CPixelShaderCompiler* m_pPSC;
	AppSettings::Shader* m_pShader;

public:
	CShaderEditorDlg();   // standard constructor
	virtual ~CShaderEditorDlg();

	BOOL Create(CWnd* pParent = NULL);

	// Dialog Data
	enum { IDD = IDD_SHADEREDITOR_DLG };
	CShaderLabelComboBox m_labels;
	CComboBox m_targets;
	CShaderEdit m_srcdata;
	CEdit m_output;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK() {}
	virtual void OnCancel() {}

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};
