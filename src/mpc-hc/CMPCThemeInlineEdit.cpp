#include "stdafx.h"
#include "CMPCThemeInlineEdit.h"
#include "CMPCTheme.h"

CMPCThemeInlineEdit::CMPCThemeInlineEdit() {
    m_brBkgnd.CreateSolidBrush(CMPCTheme::InlineEditBorderColor);
}


CMPCThemeInlineEdit::~CMPCThemeInlineEdit() {
}
BEGIN_MESSAGE_MAP(CMPCThemeInlineEdit, CEdit)
    ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


HBRUSH CMPCThemeInlineEdit::CtlColor(CDC* pDC, UINT /*nCtlColor*/) {
    pDC->SetTextColor(CMPCTheme::TextFGColor);
    pDC->SetBkColor(CMPCTheme::ContentBGColor);
    return m_brBkgnd;
}
