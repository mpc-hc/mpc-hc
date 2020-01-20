#pragma once
#include <afxcmn.h>
class CMPCThemeSpinButtonCtrl : public CSpinButtonCtrl
{
public:
    CMPCThemeSpinButtonCtrl();
    virtual ~CMPCThemeSpinButtonCtrl();
    DECLARE_DYNAMIC(CMPCThemeSpinButtonCtrl)
    enum arrowOrientation {
        arrowLeft,
        arrowRight,
        arrowTop,
        arrowBottom
    };


    DECLARE_MESSAGE_MAP()
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
    CPoint downPos;
    void drawSpinArrow(CDC& dc, COLORREF arrowClr, CRect arrowRect, arrowOrientation orientation);
    void OnPaint();
};

