// RegisterCopyDataDlg.h : header file
//

#if !defined(AFX_REGISTERCOPYDATADLG_H__8FBF82CB_6629_4707_AC99_F6E3CA6AD9A6__INCLUDED_)
#define AFX_REGISTERCOPYDATADLG_H__8FBF82CB_6629_4707_AC99_F6E3CA6AD9A6__INCLUDED_

#pragma once

#include "../mplayerc/MpcApi.h"


/////////////////////////////////////////////////////////////////////////////
// CRegisterCopyDataDlg dialog

class CRegisterCopyDataDlg : public CDialog
{
// Construction
public:
	HWND m_RemoteWindow;
	CRegisterCopyDataDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CRegisterCopyDataDlg)
	enum { IDD = IDD_REGISTERCOPYDATA_DIALOG };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegisterCopyDataDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON	m_hIcon;
	HWND	m_hWndMPC;

	// Generated message map functions
	//{{AFX_MSG(CRegisterCopyDataDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonFindwindow();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CString m_strMPCPath;
	CListBox m_lbLog;
	CString m_txtCommand;
	int m_nCommandType;
	afx_msg void OnBnClickedButtonSendcommand();
	void Senddata(MPCAPI_COMMAND nCmd, LPCTSTR strCommand);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGISTERCOPYDATADLG_H__8FBF82CB_6629_4707_AC99_F6E3CA6AD9A6__INCLUDED_)
