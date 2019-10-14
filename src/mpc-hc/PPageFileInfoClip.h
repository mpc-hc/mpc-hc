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
#include "CMPCThemePropertyPage.h"
#include "CMPCThemeToolTipCtrl.h"
#include "CMPCThemeEdit.h"

// CPPageFileInfoClip dialog

class CPPageFileInfoClip : public CMPCThemePropertyPage
{
    DECLARE_DYNAMIC(CPPageFileInfoClip)

private:
    HICON m_hIcon;
    CMPCThemeToolTipCtrl m_tooltip;
    CMPCThemeEdit m_locationCtrl;

    CStatic m_icon;
    CString m_fn, m_path;
    CString m_clip;
    CString m_author;
    CString m_copyright;
    CString m_rating;
    CString m_location;
    CString m_desc;

public:
    CPPageFileInfoClip(CString path, IFilterGraph* pFG, IFileSourceFilter* pFSF, IDvdInfo2* pDVDI);
    virtual ~CPPageFileInfoClip();

    // Dialog Data
    enum { IDD = IDD_FILEPROPCLIP };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();

    DECLARE_MESSAGE_MAP()

    bool OnDoubleClickLocation();
};
