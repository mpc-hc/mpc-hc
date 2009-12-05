#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CTunerScanDlg dialog

class CTunerScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CTunerScanDlg)

public:
	CTunerScanDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTunerScanDlg();

// Dialog Data
	enum { IDD = IDD_TUNER_SCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void		 SetProgress (bool bState);

	DECLARE_MESSAGE_MAP()
public:
	ULONG m_ulFrequencyStart;
	ULONG m_ulFrequencyEnd;
	ULONG m_ulBandwidth;
	CProgressCtrl m_Progress;
	CProgressCtrl m_Strength;
	CProgressCtrl m_Quality;
	CListCtrl m_ChannelList;
	bool m_bInProgress;

	afx_msg LRESULT OnScanProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScanEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStats(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewChannel(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	CButton m_btnStart;
	CButton m_btnSave;
	CButton m_btnCancel;
};
