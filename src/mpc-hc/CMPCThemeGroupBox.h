#pragma once
#include <afxwin.h>
class CMPCThemeGroupBox :
	public CStatic
{
    DECLARE_DYNAMIC(CMPCThemeGroupBox)
public:
	CMPCThemeGroupBox();
	virtual ~CMPCThemeGroupBox();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
};

