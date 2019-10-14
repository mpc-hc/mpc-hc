#pragma once
#include <afxcmn.h>
class CMPCThemeSpinButtonCtrl : public CSpinButtonCtrl {
public:
	CMPCThemeSpinButtonCtrl();
	virtual ~CMPCThemeSpinButtonCtrl();
    DECLARE_DYNAMIC(CMPCThemeSpinButtonCtrl)
    DECLARE_MESSAGE_MAP()
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
    CPoint downPos;
    void OnPaint();
};

