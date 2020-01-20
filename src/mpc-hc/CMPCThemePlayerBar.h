#pragma once
#include "PlayerBar.h"
class CMPCThemePlayerBar :  public CPlayerBar
{
public:
    CMPCThemePlayerBar();
    virtual ~CMPCThemePlayerBar();
    DECLARE_DYNAMIC(CMPCThemePlayerBar)

    void NcPaintGripper(CDC* pDC, CRect rcClient);
    void mpc_fillNcBG(CDC* mdc, CRect rcDraw);
    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

