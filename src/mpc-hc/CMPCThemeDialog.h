#pragma once
#include <afxwin.h>
#include "CMPCThemeButton.h"
#include "CMPCThemeGroupBox.h"
#include "CMPCThemeLinkCtrl.h"
#include "CMPCThemeUtil.h"

class CMPCThemeDialog :
	public CDialog, public CMPCThemeUtil
{
public:
	CMPCThemeDialog();
    explicit CMPCThemeDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
    virtual ~CMPCThemeDialog();
    void fulfillThemeReqs() { CMPCThemeUtil::fulfillThemeReqs((CWnd*)this); };
    DECLARE_DYNAMIC(CMPCThemeDialog)
    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

