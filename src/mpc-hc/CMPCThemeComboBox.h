#pragma once
#include <afxwin.h>
class CMPCThemeComboBox :
    public CComboBox
{
    DECLARE_DYNAMIC(CMPCThemeComboBox)
private:
    bool isHover;
    bool isThemedDropDown;
    CBrush bgBrush;
public:
    CMPCThemeComboBox();
    void doDraw(CDC& dc, CString strText, CRect r, COLORREF bkColor, COLORREF fgColor, bool drawDotted);
    virtual ~CMPCThemeComboBox();
    void themeDropDown();
    void PreSubclassWindow();
    void drawComboArrow(CDC& dc, COLORREF arrowClr, CRect arrowRect);
    void checkHover(UINT nFlags, CPoint point, bool invalidate = true);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

