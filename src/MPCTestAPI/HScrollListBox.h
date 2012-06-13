/////////////////////////////////////////////////////////////////////////////
// HScrollListBox.h : header file
//
// Copyright (c) 2002, Nebula Technologies, Inc.
// www.nebutech.com
//
// Nebula Technologies, Inc. grants you a royalty free
// license to use, modify and distribute this code
// provided that this copyright notice appears on all
// copies. This code is provided "AS IS," without a
// warranty of any kind.
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CHScrollListBox window
/////////////////////////////////////////////////////////////////////////////
class CHScrollListBox : public CListBox
{
    // Construction
public:
    CHScrollListBox();

    // Attributes
public:

    // Operations
public:

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHScrollListBox)
protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual ~CHScrollListBox();

    // Generated message map functions
protected:
    //{{AFX_MSG(CHScrollListBox)
    // NOTE - the ClassWizard will add and remove member functions here.
    //}}AFX_MSG

    afx_msg LRESULT OnAddString(WPARAM wParam, LPARAM lParam); // wParam - none, lParam - string, returns - int
    afx_msg LRESULT OnInsertString(WPARAM wParam, LPARAM lParam); // wParam - index, lParam - string, returns - int
    afx_msg LRESULT OnDeleteString(WPARAM wParam, LPARAM lParam); // wParam - index, lParam - none, returns - int
    afx_msg LRESULT OnResetContent(WPARAM wParam, LPARAM lParam); // wParam - none, lParam - none, returns - int
    afx_msg LRESULT OnDir(WPARAM wParam, LPARAM lParam); // wParam - attr, lParam - wildcard, returns - int

    DECLARE_MESSAGE_MAP()

private:
    void ResetHExtent();
    void SetNewHExtent(LPCTSTR lpszNewString);
    int GetTextLen(LPCTSTR lpszText);

};
