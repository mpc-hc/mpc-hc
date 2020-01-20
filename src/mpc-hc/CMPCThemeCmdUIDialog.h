#pragma once
#include "..\CmdUI\CmdUI.h"
#include "CMPCThemeButton.h"
#include "CMPCThemeGroupBox.h"
#include "CMPCThemeLinkCtrl.h"
#include "CMPCThemeUtil.h"
class CMPCThemeCmdUIDialog : public CCmdUIDialog, public CMPCThemeUtil
{
public:
    CMPCThemeCmdUIDialog();
    CMPCThemeCmdUIDialog(UINT nIDTemplate, CWnd* pParent = nullptr);
    CMPCThemeCmdUIDialog(LPCTSTR lpszTemplateName, CWnd* pParent = nullptr);
    virtual ~CMPCThemeCmdUIDialog();
    void fulfillThemeReqs() { CMPCThemeUtil::fulfillThemeReqs((CWnd*)this); };
    BOOL OnInitDialog();
    DECLARE_DYNAMIC(CMPCThemeCmdUIDialog)
    DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

