/*
 * (C) 2009-2014 see Authors.txt
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

#include <future>
#include "mpc-hc_config.h"

// CPPageFileMediaInfo dialog

class CPPageFileMediaInfo : public CPropertyPage
{
    DECLARE_DYNAMIC(CPPageFileMediaInfo)

private:
    CEdit m_mediainfo;
    CFont m_font;

    CString m_fn, m_path;
    std::shared_future<CString> m_futureMIText;
    std::thread m_threadSetText;

public:
    CPPageFileMediaInfo(CString path, IFileSourceFilter* pFSF);
    virtual ~CPPageFileMediaInfo();

    // Dialog Data
    enum { IDD = IDD_FILEMEDIAINFO };

#if !USE_STATIC_MEDIAINFO
    static bool HasMediaInfo() {
        MediaInfo MI;
        return MI.IsReady();
    };
#endif

    void OnSaveAs();

protected:
    enum {
        WM_REFRESH_TEXT = WM_APP + 1
    };

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnDestroy();
    afx_msg void OnRefreshText();

    bool OnKeyDownInEdit(MSG* pMsg);
};
