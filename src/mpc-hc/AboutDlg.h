/*
 * (C) 2012-2013 see Authors.txt
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

#include <afxwin.h>
#include "resource.h"
#include "CMPCThemeDialog.h"

class CAboutDlg : public CMPCThemeDialog
{
    CStatic m_icon;

    CString m_appname;
    CString m_credits;
    CString m_AuthorsPath;
    CString m_homepage;

    CString m_strBuildNumber;
    CString m_MPCCompiler;
    CString m_LAVFilters;
#ifndef MPCHC_LITE
    CString m_LAVFiltersVersion;
#endif
    CString m_buildDate;

    CString m_OSName;
    CString m_OSVersion;

public:
    CAboutDlg();
    virtual ~CAboutDlg();

    virtual BOOL OnInitDialog();

    afx_msg void OnHomepage(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAuthors(NMHDR* pNMHDR, LRESULT* pResult);

    // Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    // No message handlers
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    afx_msg void OnCopyToClipboard();
};
