#pragma once
#include <afxcmn.h>

class CMPCThemeToolTipCtrl;


class CMPCThemeToolTipCtrl : public CToolTipCtrl
{
    class CMPCThemeToolTipCtrlHelper : public CWnd
    {
    private:
        CMPCThemeToolTipCtrl* tt;
    public:
        CMPCThemeToolTipCtrlHelper(CMPCThemeToolTipCtrl* tt);
        virtual ~CMPCThemeToolTipCtrlHelper();
        DECLARE_MESSAGE_MAP()
        afx_msg void OnPaint();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    };


    DECLARE_DYNAMIC(CMPCThemeToolTipCtrl)
private:
    bool useFlickerHelper, basicMode;
    CMPCThemeToolTipCtrlHelper* helper;
    void makeHelper();
public:
    CMPCThemeToolTipCtrl();
    virtual ~CMPCThemeToolTipCtrl();
    void enableFlickerHelper();
    static void drawText(CDC& dc, CMPCThemeToolTipCtrl* tt, CRect& rect, bool calcRect = false);
    static void paintTT(CDC& dc, CMPCThemeToolTipCtrl* tt);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
};

