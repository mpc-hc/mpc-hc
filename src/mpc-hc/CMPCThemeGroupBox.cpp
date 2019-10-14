#include "stdafx.h"
#include "CMPCThemeGroupBox.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"

IMPLEMENT_DYNAMIC(CMPCThemeGroupBox, CStatic)

CMPCThemeGroupBox::CMPCThemeGroupBox() {

}


CMPCThemeGroupBox::~CMPCThemeGroupBox() {
}

BEGIN_MESSAGE_MAP(CMPCThemeGroupBox, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()


void CMPCThemeGroupBox::OnPaint() {
    if (AfxGetAppSettings().bMPCThemeLoaded) {

        CPaintDC dc(this);

        CRect r, rborder, rtext;
        GetClientRect(r);
        HDC hDC = ::GetDC(NULL);
        CString text;
        GetWindowText(text);

        CBrush fb;
        fb.CreateSolidBrush(CMPCTheme::GroupBoxBorderColor);
        rborder = r;

        CSize cs = CMPCThemeUtil::GetTextSize(_T("W"), hDC, CMPCThemeUtil::CaptionFont);
        rborder.top += cs.cy / 2;
        dc.FrameRect(rborder, &fb);
        if (!text.IsEmpty()) {

            COLORREF oldClr = dc.SetTextColor(CMPCTheme::TextFGColor);
            COLORREF oldBkClr = dc.SetBkColor(CMPCTheme::WindowBGColor);
            //CFont *font = CMPCThemeUtil::getUIFont(dc.GetSafeHdc(), CMPCThemeUtil::uiTextFont, 8);
            CFont font;
            CMPCThemeUtil::getFontByType(font, &dc, CMPCThemeUtil::CaptionFont);
            CFont* pOldFont = dc.SelectObject(&font);

            rtext = r;
            rtext.left += CMPCTheme::GroupBoxTextIndent;

            text += _T(" "); //seems to be the default behavior
            dc.DrawText(text, rtext, DT_TOP | DT_LEFT | DT_SINGLELINE | DT_EDITCONTROL);

            dc.SelectObject(pOldFont);
            dc.SetTextColor(oldClr);
            dc.SetBkColor(oldBkClr);
        }

    } else {
        __super::OnPaint();
    }
}
