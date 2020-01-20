#pragma once
#include <afxwin.h>
class CMPCThemeInlineEdit :
    public CEdit
{
public:
    CMPCThemeInlineEdit();
    virtual ~CMPCThemeInlineEdit();
    CBrush m_brBkgnd;
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};

