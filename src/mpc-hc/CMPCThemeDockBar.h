#pragma once
#include <afxpriv.h>
class CMPCThemeDockBar : public CDockBar
{
    DECLARE_DYNAMIC(CMPCThemeDockBar)
public:
    CMPCThemeDockBar();
    virtual ~CMPCThemeDockBar();
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnNcPaint();
};

