#include "stdafx.h"
#include "CMPCThemeMiniDockFrameWnd.h"
#include "CMPCThemeUtil.h"

IMPLEMENT_DYNCREATE(CMPCThemeMiniDockFrameWnd, CMiniDockFrameWnd)

BEGIN_MESSAGE_MAP(CMPCThemeMiniDockFrameWnd, CMiniDockFrameWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()


int CMPCThemeMiniDockFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMiniDockFrameWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    CMPCThemeUtil::enableWindows10DarkFrame(this);

    return 0;
}
