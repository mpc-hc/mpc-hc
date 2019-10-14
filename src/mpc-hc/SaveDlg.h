/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

#include <afxcmn.h>
#include <afxwin.h>
#include "CMPCThemeCmdUIDialog.h"
#include "CMPCThemeStatic.h"

// CSaveDlg dialog

class CSaveDlg : public CMPCThemeCmdUIDialog
{
    DECLARE_DYNAMIC(CSaveDlg)

private:
    CString m_in, m_out;
    CComPtr<IGraphBuilder> pGB;
    CComQIPtr<IMediaControl> pMC;
    CComQIPtr<IMediaEventEx> pME;
    CComQIPtr<IMediaSeeking> pMS;
    UINT_PTR m_nIDTimerEvent;

public:
    CSaveDlg(CString in, CString out, CWnd* pParent = nullptr);   // standard constructor
    virtual ~CSaveDlg();

    // Dialog Data
    enum { IDD = IDD_SAVE_DLG };
    CAnimateCtrl m_anim;
    CProgressCtrl m_progress;
    CMPCThemeStatic m_report;
    CMPCThemeStatic m_fromto;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg LRESULT OnGraphNotify(WPARAM wParam, LPARAM lParam);
};
