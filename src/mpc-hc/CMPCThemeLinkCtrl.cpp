#include "stdafx.h"
#include "CMPCThemeLinkCtrl.h"
#include "CMPCTheme.h"


CMPCThemeLinkCtrl::CMPCThemeLinkCtrl()
{

}


CMPCThemeLinkCtrl::~CMPCThemeLinkCtrl()
{
}
BEGIN_MESSAGE_MAP(CMPCThemeLinkCtrl, CLinkCtrl)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


HBRUSH CMPCThemeLinkCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CLinkCtrl::OnCtlColor(pDC, pWnd, nCtlColor);

    pDC->SetTextColor(CMPCTheme::TextFGColor);
    return hbr;
}

void CMPCThemeLinkCtrl::PreSubclassWindow()
{
    LITEM item = { 0 };
    item.mask = LIF_ITEMINDEX | LIF_STATE;
    item.state = LIS_DEFAULTCOLORS;
    item.stateMask = LIS_DEFAULTCOLORS;
    SetItem(&item);
}
