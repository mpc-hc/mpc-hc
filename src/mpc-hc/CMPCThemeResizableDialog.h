#pragma once
#include "CMPCThemeResizableDialog.h"
#include "CMPCThemeButton.h"
#include "CMPCThemeGroupBox.h"
#include "CMPCThemeLinkCtrl.h"
#include "CMPCThemeUtil.h"
class CMPCThemeResizableDialog : public CResizableDialog, public CMPCThemeUtil
{
public:
    CMPCThemeResizableDialog();
    CMPCThemeResizableDialog(UINT nIDTemplate, CWnd* pParent = nullptr);
    CMPCThemeResizableDialog(LPCTSTR lpszTemplateName, CWnd* pParent = nullptr);
    virtual ~CMPCThemeResizableDialog();
    void fulfillThemeReqs();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

