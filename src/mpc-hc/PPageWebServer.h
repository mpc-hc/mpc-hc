/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2015 see Authors.txt
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

#include "CMPCThemePPageBase.h"
#include "CMPCThemeStaticLink.h"
#include "FloatEdit.h"

// CPPageWebServer dialog

class CPPageWebServer : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageWebServer)

private:
    CString GetMPCDir();
    CString GetCurWebRoot();
    bool PickDir(CString& dir);

public:
    CPPageWebServer();
    virtual ~CPPageWebServer();

    // Dialog Data
    enum { IDD = IDD_PPAGEWEBSERVER };
    BOOL m_fEnableWebServer;
    int m_nWebServerPort;
    CMPCThemeIntEdit m_nWebServerPortCtrl;
    CMPCThemeStaticLink m_launch;
    BOOL m_fWebServerUseCompression;
    BOOL m_fWebServerLocalhostOnly;
    BOOL m_fWebServerPrintDebugInfo;
    BOOL m_bWebUIEnablePreview;
    BOOL m_fWebRoot;
    CString m_WebRoot;
    CString m_WebServerCGI;
    CString m_WebDefIndex;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnEnChangeEdit1();
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnUpdateButton2(CCmdUI* pCmdUI);
    afx_msg void OnEnablePreviewChecked();
};
