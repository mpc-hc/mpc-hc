#pragma once
#include <afxdlgs.h>
#include "CMPCThemeUtil.h"


class CMPCThemePropertySheet :
	public CPropertySheet
    , public CMPCThemeUtil
{
    DECLARE_DYNAMIC(CMPCThemePropertySheet)

public:
    CMPCThemePropertySheet(UINT nIDCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
    CMPCThemePropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
    virtual ~CMPCThemePropertySheet();

    virtual BOOL OnInitDialog();
    void fulfillThemeReqs();
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

};

