#pragma once


// CPPageFileMediaInfo dialog

class CPPageFileMediaInfo : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPageFileMediaInfo)

public:
	CPPageFileMediaInfo(CString fn);   // standard constructor
	virtual ~CPPageFileMediaInfo();

// Dialog Data
	enum { IDD = IDD_FILEMEDIAINFO };

	CEdit m_mediainfo;	
	CString m_fn;
	CFont* m_pCFont;

	CString MI_Text;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
