/*
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

#include "PPageBase.h"

class SubtitlesProviders;

// CPPageSubMisc dialog

class CPPageSubMisc : public CPPageBase
{
    DECLARE_DYNAMIC(CPPageSubMisc)

private:
    enum {
        COL_PROVIDER,
        COL_USERNAME,
        COL_LANGUAGES,
        COL_TOTAL_COLUMNS,
    };

public:
    CPPageSubMisc();
    virtual ~CPPageSubMisc();

    // Dialog Data
    enum { IDD = IDD_PPAGESUBMISC };

protected:
    BOOL m_fPreferDefaultForcedSubtitles;
    BOOL m_fPrioritizeExternalSubtitles;
    BOOL m_fDisableInternalSubtitles;
    BOOL m_bAutoDownloadSubtitles;
    CString m_strAutoDownloadSubtitlesExclude;
    BOOL m_bAutoUploadSubtitles;
    BOOL m_bPreferHearingImpairedSubtitles;
    CString m_strSubtitlesProviders;
    CString m_strSubtitlesLanguageOrder;
    CString m_strAutoloadPaths;
    CListCtrl m_list;

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    static int CALLBACK SortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

    DECLARE_MESSAGE_MAP()

    afx_msg void OnBnClickedResetSubsPath();
    afx_msg void OnAutoDownloadSubtitlesClicked();
    afx_msg void OnRightClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
    SubtitlesProviders& m_pSubtitlesProviders;
};
