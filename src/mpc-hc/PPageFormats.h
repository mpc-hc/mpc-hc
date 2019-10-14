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
#include "CMPCThemePPageBase.h"
#include "CMPCThemePlayerListCtrl.h"
#include "CMPCThemeButton.h"
#include "CMPCThemeGroupBox.h"
#include "CMPCThemeRadioOrCheck.h"



// CPPageFormats dialog

class CPPageFormats : public CMPCThemePPageBase
{
    DECLARE_DYNAMIC(CPPageFormats)

private:

    CMPCThemePlayerListCtrl m_list;
    CImageList m_onoff;
    CMPCThemeRadioOrCheck m_fContextDir;
    CMPCThemeRadioOrCheck m_fContextFiles;
    CMPCThemeRadioOrCheck m_fAssociatedWithIcons;
    CMPCThemeGroupBox m_autoplay;
    CMPCThemeRadioOrCheck m_apvideo;
    CMPCThemeRadioOrCheck m_apmusic;
    CMPCThemeRadioOrCheck m_apaudiocd;
    CMPCThemeRadioOrCheck m_apdvd;

    CString m_exts;
    bool m_bInsufficientPrivileges;
    bool m_bFileExtChanged;
    CMediaFormats m_mf;
    int m_iRtspHandler;
    BOOL m_fRtspFileExtFirst;
    bool m_bHaveRegisteredCategory;

    enum { COL_CATEGORY, COL_ENGINE };

    int IsCheckedMediaCategory(int iItem);
    void SetCheckedMediaCategory(int iItem, int fChecked);

    void UpdateMediaCategoryState(int iItem);

    bool IsNeededIconsLib();

    void SetSelectionAllFormats(bool bSelect);

public:
    CPPageFormats();
    virtual ~CPPageFormats();

    // Dialog Data
    enum { IDD = IDD_PPAGEFORMATS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
    void LoadSettings();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnMediaCategoryClicked(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMediaCategoryKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMediaCategorySelected(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginEditMediaCategoryEngine(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEditMediaCategoryEngine(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndEditMediaCategoryEngine(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAssociateAllFormats();
    afx_msg void OnAssociateVideoFormatsOnly();
    afx_msg void OnAssociateAudioFormatsOnly();
    afx_msg void OnClearAllAssociations();
    afx_msg void OnBnRunAsAdmin();
    afx_msg void OnBnWin8SetDefProg();
    afx_msg void OnBnClickedResetExtensionsList();
    afx_msg void OnBnClickedSetExtensionsList();
    afx_msg void OnFilesAssocModified();
    afx_msg void OnUpdateButtonDefault(CCmdUI* pCmdUI);
    afx_msg void OnUpdateButtonSet(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBnWin8SetDefProg(CCmdUI* pCmdUI);
};
