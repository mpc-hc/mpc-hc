#pragma once
#include <afxwin.h>
#include "CMPCTheme.h"
#include "CMPCThemeFrameUtil.h"
#include "CMPCThemeTitleBarControlButton.h"

class CMPCThemeFrameWnd :
    public CFrameWnd,
    public CMPCThemeFrameUtil
{
public:
    CMPCThemeFrameWnd();
protected:
    DECLARE_DYNAMIC(CMPCThemeFrameWnd)
public:
    virtual ~CMPCThemeFrameWnd();
    LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void RecalcLayout(BOOL bNotify = TRUE);
    virtual void SetMenuBarVisibility(DWORD dwStyle);
    BOOL SetMenuBarState(DWORD dwState);
    CRect getTitleBarRect();
    CRect getSysMenuIconRect();
protected:
    CRect   borders;
    int     titlebarHeight;
    void recalcTitleBar();
    CMPCThemeTitleBarControlButton minimizeButton, maximizeButton, closeButton;
    void GetIconRects(CRect titlebarRect, CRect& closeRect, CRect& maximizeRect, CRect& minimizeRect);
    bool checkFrame(LONG style);
    void recalcFrame();
    enum frameState {
        frameNormal,
        frameThemedCaption,
        frameThemedTopBorder,
    };
private:
    TITLEBARINFO titleBarInfo;
    frameState currentFrameState;
    bool drawCustomFrame;
public:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnPaint();
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnNcMouseLeave();
};

