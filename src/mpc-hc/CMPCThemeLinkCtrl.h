#pragma once
#include <afxcmn.h>
class CMPCThemeLinkCtrl :
    public CLinkCtrl
{
public:
    CMPCThemeLinkCtrl();
    virtual ~CMPCThemeLinkCtrl();
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    virtual void PreSubclassWindow();
};

