// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#pragma once

#include <afxwin.h>

#if 0
#include "afxcontrolbarutil.h"
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCMaskedEdit window

class CMFCMaskedEdit : public CEdit
{
	DECLARE_DYNAMIC(CMFCMaskedEdit)

// Construction
public:
	CMFCMaskedEdit();
	~CMFCMaskedEdit();

// Implementation
public:
	void EnableMask(LPCTSTR lpszMask, LPCTSTR lpszInputTemplate, TCHAR chMaskInputTemplate = _T('_'), LPCTSTR lpszValid = NULL);
	void DisableMask();

	void SetValidChars(LPCTSTR lpszValid = NULL);
	void EnableGetMaskedCharsOnly(BOOL bEnable = TRUE) { m_bGetMaskedCharsOnly = bEnable; }
	void EnableSetMaskedCharsOnly(BOOL bEnable = TRUE) { m_bSetMaskedCharsOnly = bEnable; }
	void EnableSelectByGroup(BOOL bEnable = TRUE) { m_bSelectByGroup = bEnable; }

	void SetWindowText(LPCTSTR lpszString);
	int GetWindowText(_Out_z_cap_post_count_(nMaxCount, return + 1) LPTSTR lpszStringBuf, _In_ int nMaxCount) const;
	void GetWindowText(CString& rstrString) const;

protected:
	virtual BOOL IsMaskedChar(TCHAR chChar, TCHAR chMaskChar) const;

	const CString GetValue() const { return m_str;}
	const CString GetMaskedValue(BOOL bWithSpaces = TRUE) const;
	BOOL SetValue(LPCTSTR lpszString, BOOL bWithDelimiters = TRUE);

private:
	BOOL CheckChar(TCHAR chChar, int nPos);
	void OnCharPrintchar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCharBackspace(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCharDelete(UINT nChar, UINT nRepCnt, UINT nFlags);
	void GetGroupBounds(int &nBegin, int &nEnd, int nStartPos=0, BOOL bForward=TRUE);
	BOOL DoUpdate(BOOL bRestoreLastGood = TRUE, int nBeginOld = -1, int nEndOld = -1);

// Attributes
private:
	CString m_str;                  // Initial value
	CString m_strMask;              // The mask string
	CString m_strInputTemplate;     // "_" char = character entry
	TCHAR   m_chMaskInputTemplate;  // Default char
	CString m_strValid;             // Valid string characters
	BOOL    m_bGetMaskedCharsOnly;
	BOOL    m_bSetMaskedCharsOnly;
	BOOL    m_bSelectByGroup;
	BOOL    m_bMaskKeyInProgress;
	BOOL    m_bPasteProcessing;
	BOOL    m_bSetTextProcessing;

protected:
	//{{AFX_MSG(CMFCMaskedEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnUpdateR();
	afx_msg void OnSetFocusR();
	afx_msg LRESULT OnCut(WPARAM, LPARAM);
	afx_msg LRESULT OnClear(WPARAM, LPARAM);
	afx_msg LRESULT OnPaste(WPARAM, LPARAM);
	afx_msg LRESULT OnSetText(WPARAM, LPARAM);
	afx_msg LRESULT OnGetText(WPARAM, LPARAM);
	afx_msg LRESULT OnGetTextLength(WPARAM, LPARAM);
	afx_msg LRESULT OnInitControl(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
