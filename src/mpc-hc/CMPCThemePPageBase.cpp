#include "stdafx.h"
#include "CMPCThemePPageBase.h"
#include "CMPCTheme.h"
#include "ImageGrayer.h"

IMPLEMENT_DYNAMIC(CMPCThemePPageBase, CPPageBase)

CMPCThemePPageBase::CMPCThemePPageBase(UINT nIDTemplate, UINT nIDCaption)
    : CPPageBase(nIDTemplate, nIDCaption)
{
}


CMPCThemePPageBase::~CMPCThemePPageBase()
{
}

BOOL CMPCThemePPageBase::OnInitDialog()
{
    __super::OnInitDialog();
    fulfillThemeReqs();
    return 0;
}

void CMPCThemePPageBase::SetMPCThemeButtonIcon(UINT nIDButton, UINT nIDIcon, ImageGrayer::mpcColorStyle colorStyle)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (!m_buttonIcons.count(nIDIcon)) {
            CImage img, imgEnabled, imgDisabled;
            img.LoadFromResource(AfxGetInstanceHandle(), nIDIcon);

            ImageGrayer::UpdateColor(img, imgEnabled, false, colorStyle);
            ImageGrayer::UpdateColor(img, imgDisabled, true, colorStyle);

            CImageList& imageList = m_buttonIcons[nIDIcon];
            imageList.Create(img.GetWidth(), img.GetHeight(), ILC_COLOR32, 2, 0);
            imageList.Add(CBitmap::FromHandle(imgEnabled), nullptr);
            imageList.Add(CBitmap::FromHandle(imgDisabled), nullptr);
        }

        BUTTON_IMAGELIST buttonImageList;
        buttonImageList.himl = m_buttonIcons[nIDIcon];
        buttonImageList.margin = { 0, 0, 0, 0 };
        buttonImageList.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
        static_cast<CButton*>(GetDlgItem(nIDButton))->SetImageList(&buttonImageList);
    } else {
        CPPageBase::SetButtonIcon(nIDButton, nIDIcon);

    }
}


BEGIN_MESSAGE_MAP(CMPCThemePPageBase, CPPageBase)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


HBRUSH CMPCThemePPageBase::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH ret;
    ret = getCtlColor(pDC, pWnd, nCtlColor);
    if (nullptr != ret) {
        return ret;
    } else {
        return __super::OnCtlColor(pDC, pWnd, nCtlColor);
    }
}
