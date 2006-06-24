#pragma once
#include "afxwin.h"


// CShaderCombineDlg dialog

class CShaderCombineDlg : public CResizableDialog
{
	CAtlList<CString>& m_labels;

public:
	CShaderCombineDlg(CAtlList<CString>& labels, CWnd* pParent = NULL);   // standard constructor
	virtual ~CShaderCombineDlg();

// Dialog Data
	enum { IDD = IDD_SHADERCOMBINE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	CListBox m_list;
public:
	CComboBox m_combo;
public:
	afx_msg void OnBnClickedButton12();
public:
	afx_msg void OnBnClickedButton13();
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnBnClickedButton11();
};
