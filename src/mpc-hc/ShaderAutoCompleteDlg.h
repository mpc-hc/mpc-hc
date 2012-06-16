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

#include "resource.h"
#include "ui/ResizableLib/ResizableDialog.h"


// CShaderAutoCompleteDlg dialog

class CShaderAutoCompleteDlg : public CResizableDialog
{
    TOOLINFO m_ti;
    HWND m_hToolTipWnd;
    TCHAR m_text[1024];

public:
    CShaderAutoCompleteDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CShaderAutoCompleteDlg();

    CMap<CString, LPCTSTR, CString, CString> m_inst;

    // Dialog Data
    enum { IDD = IDD_SHADERAUTOCOMPLETE_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CListBox m_list;
    virtual BOOL OnInitDialog();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnLbnSelchangeList1();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
