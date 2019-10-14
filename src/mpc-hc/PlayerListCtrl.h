/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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

#include "WinHotkeyCtrl.h"
#include "CMPCThemeComboBox.h"

#define LVN_DOLABELEDIT (LVN_FIRST+1)


class CInPlaceWinHotkey : public CWinHotkeyCtrl
{
private:
    int m_iItem;
    int m_iSubItem;
    CString m_sInitText;
    BOOL m_bESC; // To indicate whether ESC key was pressed

public:
    CInPlaceWinHotkey(int iItem, int iSubItem, CString sInitText);
    virtual ~CInPlaceWinHotkey();

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnNcDestroy();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

class CInPlaceEdit : public CEdit
{
protected:
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

class CInPlaceFloatEdit : public CInPlaceEdit
{
public:
    CInPlaceFloatEdit(int iItem, int iSubItem, CString sInitText);
    virtual ~CInPlaceFloatEdit();

protected:
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class CInPlaceComboBox : public CMPCThemeComboBox
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
    UINT_PTR m_nTimerID;

    bool PrepareInPlaceControl(int nRow, int nCol, CRect& rect);

public:
    CPlayerListCtrl(int tStartEditingDelay = 500);
    virtual ~CPlayerListCtrl();

    int HitTestEx(const CPoint& point, int* col) const;
    CImageList* CreateDragImageEx(LPPOINT lpPoint);

    int GetBottomIndex() const;

    CWinHotkeyCtrl* ShowInPlaceWinHotkey(int nItem, int nCol);
    CEdit* ShowInPlaceEdit(int nItem, int nCol);
    CEdit* ShowInPlaceFloatEdit(int nItem, int nCol);
    CComboBox* ShowInPlaceComboBox(int nItem, int nCol, CAtlList<CString>& lstItems, int nSel, bool bShowDropDown = false);
    CListBox* ShowInPlaceListBox(int nItem, int nCol, CAtlList<CString>& lstItems, int nSel);

    bool m_fInPlaceDirty;

protected:
    virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
    virtual ULONG GetGestureStatus(CPoint) override { return 0; };

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
    afx_msg void OnEnChangeWinHotkey1();
    afx_msg void OnCbnDropdownCombo1();
    afx_msg void OnCbnSelendokCombo1();
    afx_msg void OnLbnSelChangeList1();
    afx_msg BOOL OnHdnItemchanging(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point);
    afx_msg void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point);
};
