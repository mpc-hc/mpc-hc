#pragma once
#include <afxwin.h>
class CMPCThemeStatic :
    public CStatic
{
    DECLARE_DYNAMIC(CMPCThemeStatic)
public:

    CMPCThemeStatic();
    virtual ~CMPCThemeStatic();
    void setFileDialogChild(bool set) { isFileDialogChild = set; };
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnNcPaint();
    afx_msg void OnEnable(BOOL bEnable);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
    bool isFileDialogChild;
};

