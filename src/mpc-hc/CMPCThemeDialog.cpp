#include "stdafx.h"
#include "CMPCThemeDialog.h"
#include "CMPCTheme.h"
#include "mplayerc.h"
#undef SubclassWindow


CMPCThemeDialog::CMPCThemeDialog() {
}

CMPCThemeDialog::CMPCThemeDialog(UINT nIDTemplate, CWnd * pParentWnd) : CDialog(nIDTemplate, pParentWnd) {
}


CMPCThemeDialog::~CMPCThemeDialog() {
}

IMPLEMENT_DYNAMIC(CMPCThemeDialog, CDialog)

BEGIN_MESSAGE_MAP(CMPCThemeDialog, CDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


HBRUSH CMPCThemeDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        return getCtlColor(pDC, pWnd, nCtlColor);
    } else {
        HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
        return hbr;
    }
}
