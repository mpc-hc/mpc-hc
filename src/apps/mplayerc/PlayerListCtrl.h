/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#define LVN_DOLABELEDIT (LVN_FIRST+1)


class CInPlaceEdit : public CEdit
{
private:
    int m_iItem;
    int m_iSubItem;
    CString m_sInitText;
    BOOL m_bESC; // To indicate whether ESC key was pressed

public:
    CInPlaceEdit(int iItem, int iSubItem, CString sInitText);
    virtual ~CInPlaceEdit();

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

class CInPlaceComboBox : public CComboBox
{
private:
    int m_iItem;
    int m_iSubItem;
    CAtlList<CString> m_lstItems;
    int m_nSel;
    BOOL m_bESC; // To indicate whether ESC key was pressed

public:
    CInPlaceComboBox(int iItem, int iSubItem, CAtlList<CString>& plstItems, int nSel);
    virtual ~CInPlaceComboBox();

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnCloseup();
};

class CInPlaceListBox : public CListBox
{
private:
    int m_iItem;
    int m_iSubItem;
    CAtlList<CString> m_lstItems;
    int m_nSel;
    BOOL m_bESC; // To indicate whether ESC key was pressed

public:
    CInPlaceListBox(int iItem, int iSubItem, CAtlList<CString>& plstItems, int nSel);
    virtual ~CInPlaceListBox();

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

// CPlayerListCtrl

class CPlayerListCtrl : public CListCtrl
{
    DECLARE_DYNAMIC(CPlayerListCtrl)

private:
    int m_nItemClicked, m_nSubItemClicked;
    int m_tStartEditingDelay;

    bool PrepareInPlaceControl(int nRow, int nCol, CRect& rect);

public:
    CPlayerListCtrl(int tStartEditingDelay = 500);
    virtual ~CPlayerListCtrl();

    int HitTestEx(CPoint& point, int* col) const;
    CImageList* CreateDragImageEx(LPPOINT lpPoint);

    int GetBottomIndex() const;

    CEdit* ShowInPlaceEdit(int nItem, int nCol);
    CComboBox* ShowInPlaceComboBox(int nItem, int nCol, CAtlList<CString>& lstItems, int nSel, bool bShowDropDown = false);
    CListBox* ShowInPlaceListBox(int nItem, int nCol, CAtlList<CString>& lstItems, int nSel);

    bool m_fInPlaceDirty;

protected:
    virtual void PreSubclassWindow();
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLvnMarqueeBegin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnInsertitem(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEnChangeEdit1();
    afx_msg void OnCbnDropdownCombo1();
    afx_msg void OnCbnSelendokCombo1();
    afx_msg void OnLbnSelChangeList1();
    afx_msg BOOL OnHdnItemchanging(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
};
