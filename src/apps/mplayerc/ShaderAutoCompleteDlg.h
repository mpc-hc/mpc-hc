#pragma once
#include "resource.h"


// CShaderAutoCompleteDlg dialog

class CShaderAutoCompleteDlg : public CResizableDialog
{
    TOOLINFO m_ti;
	HWND m_hToolTipWnd;
	TCHAR m_text[1024];

public:
	CShaderAutoCompleteDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShaderAutoCompleteDlg();

	CMap<CString, LPCTSTR, CString, CString> m_inst;

// Dialog Data
	enum { IDD = IDD_SHADERAUTOCOMPLETE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CListBox m_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
