#pragma once
#include <afxpriv.h>

class CMPCThemeMiniDockFrameWnd:
    CMiniDockFrameWnd
{
    DECLARE_DYNCREATE(CMPCThemeMiniDockFrameWnd)
public:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

