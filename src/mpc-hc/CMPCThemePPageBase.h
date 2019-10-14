#pragma once
#include "PPageBase.h"
#include "CMPCThemeButton.h"
#include "CMPCThemeUtil.h"
#include "ImageGrayer.h"
class CMPCThemePPageBase :
	public CPPageBase, public CMPCThemeUtil
{
    DECLARE_DYNAMIC(CMPCThemePPageBase)
public:
    CMPCThemePPageBase(UINT nIDTemplate, UINT nIDCaption);
    virtual ~CMPCThemePPageBase();
    void fulfillThemeReqs() { CMPCThemeUtil::fulfillThemeReqs((CWnd*)this); };

    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
protected:
    virtual BOOL OnInitDialog();
    void SetMPCThemeButtonIcon(UINT nIDButton, UINT nIDIcon, ImageGrayer::mpcColorStyle colorStyle = ImageGrayer::mpcMono);
};

