// RegisterCopyDataDlg.h : header file
//

#pragma once
#include "../mplayerc/MpcApi.h"
#include "HScrollListBox.h"


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
	CString		m_strMPCPath;
	CHScrollListBox m_listBox;
	CString		m_txtCommand;
	int			m_nCommandType;
	afx_msg void OnBnClickedButtonSendcommand();
	void		Senddata(MPCAPI_COMMAND nCmd, LPCTSTR strCommand);
};
