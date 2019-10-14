/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include <afxcmn.h>
#include "DSMPropertyBag.h"
#include "CMPCThemePPageBase.h"
#include "CMPCThemePlayerListCtrl.h"


// CPPageFileInfoRes dialog

class CPPageFileInfoRes : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageFileInfoRes)

private:
    HICON m_hIcon;
    CStatic m_icon;
    CMPCThemePlayerListCtrl m_list;

    CString m_fn;
    std::vector<CDSMResource> m_res;

public:
    CPPageFileInfoRes(CString path, IFilterGraph* pFG, IFileSourceFilter* pFSF);
    virtual ~CPPageFileInfoRes();

    // Dialog Data
    enum { IDD = IDD_FILEPROPRES };

    bool HasResources() const { return !m_res.empty(); };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnSaveAs();
    afx_msg void OnUpdateSaveAs(CCmdUI* pCmdUI);
    afx_msg void OnOpenEmbeddedResInBrowser(NMHDR* pNMHDR, LRESULT* pResult);
};
