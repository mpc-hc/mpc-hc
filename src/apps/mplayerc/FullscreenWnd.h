#pragma once


// CFullscreenWnd

class CMainFrame;

class CFullscreenWnd : public CWnd
{
	DECLARE_DYNAMIC(CFullscreenWnd)

public:
	CFullscreenWnd(CMainFrame* pMainFrame);
	virtual ~CFullscreenWnd();

	void	ShowCursor(bool bVisible);

protected:
	DECLARE_MESSAGE_MAP()
	
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CMainFrame*			m_pMainFrame;
	HCURSOR				m_hCursor;
	bool				m_bCursorVisible;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};


