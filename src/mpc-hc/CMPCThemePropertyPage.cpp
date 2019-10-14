#include "stdafx.h"
#include "CMPCThemePropertyPage.h"

CMPCThemePropertyPage::CMPCThemePropertyPage(UINT nIDTemplate, UINT nIDCaption)
    : CPropertyPage(nIDTemplate, nIDCaption) {

}

CMPCThemePropertyPage::~CMPCThemePropertyPage() {
}

BOOL CMPCThemePropertyPage::OnInitDialog() {
    __super::OnInitDialog();
    fulfillThemeReqs();
    return 0;
}

IMPLEMENT_DYNAMIC(CMPCThemePropertyPage, CPropertyPage)
BEGIN_MESSAGE_MAP(CMPCThemePropertyPage, CPropertyPage)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


HBRUSH CMPCThemePropertyPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH ret;
    ret = getCtlColor(pDC, pWnd, nCtlColor);
    if (nullptr != ret) {
        return ret;
    } else {
        return __super::OnCtlColor(pDC, pWnd, nCtlColor);
    }
}
