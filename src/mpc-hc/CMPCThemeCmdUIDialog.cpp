#include "stdafx.h"
#include "CMPCThemeCmdUIDialog.h"
#include "CMPCTheme.h"
#include "mplayerc.h"


CMPCThemeCmdUIDialog::CMPCThemeCmdUIDialog()
{
}

CMPCThemeCmdUIDialog::CMPCThemeCmdUIDialog(UINT nIDTemplate, CWnd* pParent): CCmdUIDialog(nIDTemplate, pParent)
{
}

CMPCThemeCmdUIDialog::CMPCThemeCmdUIDialog(LPCTSTR lpszTemplateName, CWnd* pParent): CCmdUIDialog(lpszTemplateName, pParent)
{
}


CMPCThemeCmdUIDialog::~CMPCThemeCmdUIDialog()
{
}

IMPLEMENT_DYNAMIC(CMPCThemeCmdUIDialog, CCmdUIDialog)
BEGIN_MESSAGE_MAP(CMPCThemeCmdUIDialog, CCmdUIDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CMPCThemeCmdUIDialog::OnInitDialog()
{
    BOOL ret = __super::OnInitDialog();
    CMPCThemeUtil::enableWindows10DarkFrame(this);
    return ret;
}


HBRUSH CMPCThemeCmdUIDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        return getCtlColor(pDC, pWnd, nCtlColor);
    } else {
        HBRUSH hbr = __super::OnCtlColor(pDC, pWnd, nCtlColor);
        return hbr;
    }
}
