#pragma once
#include "ComPropertyPage.h"
#include "CMPCThemeUtil.h"
class CMPCThemeComPropertyPage : public CComPropertyPage, public CMPCThemeUtil {
public:
    CMPCThemeComPropertyPage(IPropertyPage* pPage);
    virtual ~CMPCThemeComPropertyPage();


    void fulfillThemeReqs() { CMPCThemeUtil::fulfillThemeReqs((CWnd*)this); };
    DECLARE_DYNAMIC(CMPCThemeComPropertyPage)

    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
protected:
    virtual BOOL OnInitDialog();

};

