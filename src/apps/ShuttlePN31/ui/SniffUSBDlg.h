// SniffUSBDlg.h : header file
//

#if !defined(AFX_SNIFFUSBDLG_H__3ACD35AB_E49A_11D3_A755_00A0C9971EFC__INCLUDED_)
#define AFX_SNIFFUSBDLG_H__3ACD35AB_E49A_11D3_A755_00A0C9971EFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// for us :-)

#define SERVICE	"pn31snoop"

/////////////////////////////////////////////////////////////////////////////
// CSniffUSBDlg dialog

class CSniffUSBDlg : public CDialog
{
// Construction
public:
	void CheckLogFile();
	void CheckDriver();
	void CheckService();
	CSniffUSBDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSniffUSBDlg)
	enum { IDD = IDD_SNIFFUSB_DIALOG };
	CListCtrl	m_cDevs;
	CString	m_LogFileName;
	int		m_LogSize;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSniffUSBDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    CString m_sFilterName;
    CString m_sLowerFilters;
    
    BOOL IsThereAFilter(LPCTSTR szVidPid);

	// Generated message map functions
	//{{AFX_MSG(CSniffUSBDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRefresh();
	afx_msg void OnInstall();
	afx_msg void OnUninstall();
	afx_msg void OnRclickUsbdevs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSnoopusbInstall();
	afx_msg void OnSnoopusbUninstall();
	afx_msg void OnReplug();
	afx_msg void OnSnoopusbReplug();
	afx_msg void OnStartService();
	afx_msg void OnStopService();
	afx_msg void OnCreateService();
	afx_msg void OnDeleteService();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLogView();
	afx_msg void OnLogDelete();
	afx_msg void OnTest();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int ComputePosition(int align, int top, int saveTop, int saveBottom);
	BOOL GetControlRect(UINT nID, RECT *lpRect);
	void SaveControlPosition();
	CStatusBar m_StatusBar;
	BOOL GetSelectedDriver(CString& sDriver);
	BOOL m_bIsWin2K;

	CRect wholeRect;
	HANDLE	m_hSniffer;
public:
	afx_msg void OnBnClickedTest2();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNIFFUSBDLG_H__3ACD35AB_E49A_11D3_A755_00A0C9971EFC__INCLUDED_)
