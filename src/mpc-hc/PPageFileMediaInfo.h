/*
 * (C) 2009-2013 see Authors.txt
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

// CPPageFileMediaInfo dialog

class CPPageFileMediaInfo : public CPropertyPage
{
    DECLARE_DYNAMIC(CPPageFileMediaInfo)

private:
    CComPtr<IFilterGraph> m_pFG;
    CComPtr<IFileSourceFilter> m_pFSF;

public:
    CPPageFileMediaInfo(CString path, IFilterGraph* pFG, IFileSourceFilter* pFSF);
    virtual ~CPPageFileMediaInfo();

    // Dialog Data
    enum { IDD = IDD_FILEMEDIAINFO };

    CEdit m_mediainfo;
    CString m_fn, m_path;
    CFont* m_pCFont;
    CString MI_Text;

#if !USE_STATIC_MEDIAINFO
    static bool HasMediaInfo();
#endif
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
