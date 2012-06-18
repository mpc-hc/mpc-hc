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

#include <afxwin.h>
#include "PPageBase.h"
#include "PlayerListCtrl.h"


// CPPageFormats dialog

class CPPageFormats : public CPPageBase
{
    DECLARE_DYNAMIC(CPPageFormats)

private:
    CImageList m_onoff;
    bool m_bInsufficientPrivileges;
    bool m_bFileExtChanged;

    int GetChecked(int iItem);
    void SetChecked(int iItem, int fChecked);

    void SetListItemState(int nItem);

    bool IsNeededIconsLib();

public:
    CPPageFormats();
    virtual ~CPPageFormats();

    enum { COL_CATEGORY, COL_ENGINE };
    CPlayerListCtrl m_list;
    CString m_exts;
    CStatic m_autoplay;
    CButton m_apvideo;
    CButton m_apmusic;
    CButton m_apaudiocd;
    CButton m_apdvd;
    int m_iRtspHandler;
    BOOL m_fRtspFileExtFirst;

    // Dialog Data
    enum { IDD = IDD_PPAGEFORMATS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton14();
    afx_msg void OnBnClickedButton13();
    afx_msg void OnBnClickedButton12();
    afx_msg void OnBnClickedButton11();
    afx_msg void OnBnVistaModify();
    afx_msg void OnFilesAssocModified();
    afx_msg void OnUpdateButtonDefault(CCmdUI* pCmdUI);
    afx_msg void OnUpdateButtonSet(CCmdUI* pCmdUI);
    CButton m_fContextDir;
    CButton m_fContextFiles;
    CButton m_fAssociatedWithIcons;
};
