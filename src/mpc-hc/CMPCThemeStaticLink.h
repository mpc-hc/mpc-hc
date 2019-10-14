#pragma once
#include "StaticLink.h"
#include "CMPCThemeUtil.h"
class CMPCThemeStaticLink :
	public CStaticLink, public CMPCThemeUtil
{
protected:
    CBrush bgBrush;
public:
    DECLARE_DYNAMIC(CMPCThemeStaticLink)
    CMPCThemeStaticLink(LPCTSTR lpText = nullptr, bool bDeleteOnDestroy = false);
	virtual ~CMPCThemeStaticLink();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg void OnEnable(BOOL bEnable);
};

