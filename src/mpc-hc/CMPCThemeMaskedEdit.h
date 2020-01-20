#pragma once
#include <afxmaskededit.h>
class CMPCThemeMaskedEdit :
    public CMFCMaskedEdit
{
    DECLARE_DYNAMIC(CMPCThemeMaskedEdit)
public:
    CMPCThemeMaskedEdit();
    virtual ~CMPCThemeMaskedEdit();
    void PreSubclassWindow();
protected:
    CFont font;

    DECLARE_MESSAGE_MAP()
    afx_msg void OnNcPaint();

};

