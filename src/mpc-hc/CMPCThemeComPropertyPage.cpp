#include "stdafx.h"
#include "CMPCThemeComPropertyPage.h"

CMPCThemeComPropertyPage::CMPCThemeComPropertyPage(IPropertyPage* pPage) : CComPropertyPage(pPage) {

}

CMPCThemeComPropertyPage::~CMPCThemeComPropertyPage() {
}

BOOL CMPCThemeComPropertyPage::OnInitDialog() {
    __super::OnInitDialog();
    fulfillThemeReqs();
    return 0;
}

IMPLEMENT_DYNAMIC(CMPCThemeComPropertyPage, CComPropertyPage)
BEGIN_MESSAGE_MAP(CMPCThemeComPropertyPage, CComPropertyPage)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


HBRUSH CMPCThemeComPropertyPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH ret;
    ret = getCtlColor(pDC, pWnd, nCtlColor);
    if (nullptr != ret) {
        return ret;
    } else {
        return __super::OnCtlColor(pDC, pWnd, nCtlColor);
    }
}
