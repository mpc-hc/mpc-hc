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

#include "PlayerBar.h"
#include "PlayerListCtrl.h"


class CClip
{
private:
    REFERENCE_TIME m_rtIn;
    REFERENCE_TIME m_rtOut;
    CString m_strName;

public:
    CClip();
    ~CClip();

    bool HaveIn() const { return m_rtIn != _I64_MIN; };
    bool HaveOut() const { return m_rtOut != _I64_MIN; };

    void SetIn(LPCTSTR strVal);
    void SetOut(LPCTSTR strVal);
    void SetIn(REFERENCE_TIME rtVal);
    void SetOut(REFERENCE_TIME rtVal);
    void SetName(LPCTSTR strName) { m_strName = strName; };

    CString GetIn() const;
    CString GetOut() const;
    CString GetName() const { return m_strName; };
};

class CEditListEditor : public CPlayerBar
{
    DECLARE_DYNAMIC(CEditListEditor)

private:
    enum {
        COL_INDEX,
        COL_IN,
        COL_OUT,
        COL_NAME,
        COL_MAX
    };

    CPlayerListCtrl m_list;
    CStatic         m_stUsers;
    CComboBox       m_cbUsers;
    CStatic         m_stHotFolders;
    CComboBox       m_cbHotFolders;
    CImageList      m_fakeImageList;
    POSITION        m_curPos;
    BOOL            m_bDragging;
    int             m_nDragIndex;
    int             m_nDropIndex;
    CPoint          m_ptDropPoint;
    CImageList*     m_pDragImage;

    CString         m_strFileName;
    bool            m_bFileOpen;
    CList<CClip>    m_editList;
    CArray<CString> m_nameList;

    void            SaveEditListToFile();
    void            ResizeListColumn();
    POSITION        InsertClip(POSITION pos, CClip& newClip);
    void            DropItemOnList();
    int             FindIndex(const POSITION pos) const;
    int             FindNameIndex(LPCTSTR strName) const;
    void            FillCombo(LPCTSTR strFileName, CComboBox& combo, bool bAllowNull);
    void            SelectCombo(LPCTSTR strValue, CComboBox& combo);

public:
    CEditListEditor();
    ~CEditListEditor();

    BOOL Create(CWnd* pParentWnd, UINT defDockBarID);

    virtual void ReloadTranslatableResources();

    void CloseFile();
    void OpenFile(LPCTSTR lpFileName);
    void SetIn(REFERENCE_TIME rtIn);
    void SetOut(REFERENCE_TIME rtOut);
    void NewClip(REFERENCE_TIME rtVal);
    void Save();

protected:
    DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLvnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult);
};
