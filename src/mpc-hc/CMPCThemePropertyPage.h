#pragma once
#include "CMPCThemeUtil.h"
class CMPCThemePropertyPage : public CPropertyPage, public CMPCThemeUtil {
public:
    CMPCThemePropertyPage(UINT nIDTemplate, UINT nIDCaption);
    virtual ~CMPCThemePropertyPage();


    void fulfillThemeReqs() { CMPCThemeUtil::fulfillThemeReqs((CWnd*)this); };
    DECLARE_DYNAMIC(CMPCThemePropertyPage)

    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
protected:
    virtual BOOL OnInitDialog();

};

