//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDPopup.cpp
//
// Color LCD Popup class
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"
#include "LCDPopup.h"

// This uses GDI+
using namespace Gdiplus;


//************************************************************************
//
// CLCDPopupBackground::CLCDPopupBackground
//
//************************************************************************

CLCDPopupBackground::CLCDPopupBackground(void)
{
    m_bUseGradient = FALSE;
    m_cAlphaStart = 0x9f;
    m_cAlphaEnd = m_cAlphaStart/2;
    m_cfColor = 0;
    m_nRectRadius = 15;
    m_pGraphicsPath = NULL;
}


//************************************************************************
//
// CLCDPopupBackground::~CLCDPopupBackground
//
//************************************************************************

CLCDPopupBackground::~CLCDPopupBackground(void)
{
    if (m_pGraphicsPath)
    {
        delete m_pGraphicsPath;
        m_pGraphicsPath = NULL;
    }
}


//************************************************************************
//
// CLCDPopupBackground::SetSize
//
//************************************************************************

void CLCDPopupBackground::SetSize(int nCX, int nCY)
{
    CLCDBase::SetSize(nCX, nCY);
    RecalcRoundedRectangle();
}


//************************************************************************
//
// CLCDPopupBackground::OnDraw
//
//************************************************************************

void CLCDPopupBackground::OnDraw(CLCDGfxBase &rGfx)
{
    if (NULL == m_pGraphicsPath)
    {
        // Size has not been set
        return;
    }

    // Draw to the extent of the control
    Rect r(0, 0, GetWidth(), GetHeight());

    Graphics gfx(rGfx.GetHDC());

    if (m_bUseGradient)
    {
        LinearGradientBrush brush(
            r,
            Color(Color::MakeARGB(m_cAlphaStart, GetRValue(m_cfColor), GetGValue(m_cfColor), GetBValue(m_cfColor))),
            Color(Color::MakeARGB(m_cAlphaEnd, GetRValue(m_cfColor), GetGValue(m_cfColor), GetBValue(m_cfColor))),
            LinearGradientModeVertical
            );  // blue
        gfx.FillPath(&brush, m_pGraphicsPath);
    }
    else
    {
        SolidBrush brush(Color::MakeARGB(m_cAlphaStart, GetRValue(m_cfColor), GetGValue(m_cfColor), GetBValue(m_cfColor)));
        gfx.FillPath(&brush, m_pGraphicsPath);
    }
}


//************************************************************************
//
// CLCDPopupBackground::SetAlphaLevel
//
//************************************************************************

void CLCDPopupBackground::SetAlphaLevel(BYTE cAlphaStart, BYTE cAlphaEnd)
{
    m_cAlphaStart = cAlphaStart;
    m_cAlphaEnd = cAlphaEnd; 
}


//************************************************************************
//
// CLCDPopupBackground::SetGradientMode
//
//************************************************************************

void CLCDPopupBackground::SetGradientMode(BOOL bGradient)
{
    m_bUseGradient = bGradient;
}


//************************************************************************
//
// CLCDPopupBackground::SetColor
//
//************************************************************************

void CLCDPopupBackground::SetColor(COLORREF cfColor)
{
    m_cfColor = cfColor;
}


//************************************************************************
//
// CLCDPopupBackground::SetRoundedRecteRadius
//
//************************************************************************

void CLCDPopupBackground::SetRoundedRecteRadius(int nRadius)
{
    m_nRectRadius = nRadius;
    RecalcRoundedRectangle();
}


//************************************************************************
//
// CLCDPopupBackground::RecalcRoundedRectangle
//
//************************************************************************

void CLCDPopupBackground::RecalcRoundedRectangle(void)
{
    if (m_pGraphicsPath)
    {
        delete m_pGraphicsPath;
        m_pGraphicsPath = NULL;
    }

    m_pGraphicsPath = new GraphicsPath();
    int l = 0;
    int t = 0;
    int w = GetWidth();
    int h = GetHeight();
    int d = m_nRectRadius << 1;
    m_pGraphicsPath->AddArc(l, t, d, d, 180, 90);
    m_pGraphicsPath->AddLine(l + m_nRectRadius, t, l + w - m_nRectRadius, t);
    m_pGraphicsPath->AddArc(l + w - d, t, d, d, 270, 90);
    m_pGraphicsPath->AddLine(l + w, t + m_nRectRadius, l + w, t + h - m_nRectRadius);
    m_pGraphicsPath->AddArc(l + w - d, t + h - d, d, d, 0, 90);
    m_pGraphicsPath->AddLine(l + w - m_nRectRadius, t + h, l + m_nRectRadius, t + h);
    m_pGraphicsPath->AddArc(l, t + h - d, d, d, 90, 90);
    m_pGraphicsPath->AddLine(l, t + h - m_nRectRadius, l, t + m_nRectRadius);
    m_pGraphicsPath->CloseFigure();
}






// Define the maximum popup width as 1/2 of the QVGA width
#define MAX_POPUP_WIDTH     (LGLCD_QVGA_BMP_WIDTH / 2)
#define POPUP_MARGIN        (12)
#define POPUP_FONT_PT       (12)
#define POPUP_FONT_WT       FW_BOLD
#define POPUP_FONT_FACE     _T("Arial")

//************************************************************************
//
// CLCDPopup::CLCDPopup
//
//************************************************************************

CLCDPopup::CLCDPopup(void)
{
    m_nMaxPopupWidth = MAX_POPUP_WIDTH;
    m_pbType = PB_TEXT;
}


//************************************************************************
//
// CLCDPopup::Initialize
//
//************************************************************************

HRESULT CLCDPopup::Initialize(int nMaxPopupWidth /* = 0 */)
{
    if(0 == nMaxPopupWidth)
    {
        m_nMaxPopupWidth = MAX_POPUP_WIDTH;
    }
    else
    {
        m_nMaxPopupWidth = nMaxPopupWidth;
    }
    // The text gets offset by the margin
    m_Background.Initialize();
    m_Background.SetOrigin(0, 0);

    m_MessageText.SetOrigin(POPUP_MARGIN, POPUP_MARGIN);
    m_MessageText.SetFontFaceName(POPUP_FONT_FACE);
    m_MessageText.SetFontPointSize(POPUP_FONT_PT);
    m_MessageText.SetFontWeight(POPUP_FONT_WT);
    m_MessageText.SetWordWrap(TRUE);
    m_MessageText.SetSize(LGLCD_QVGA_BMP_WIDTH, LGLCD_QVGA_BMP_HEIGHT);

    m_OKText.SetFontFaceName(_T("Arial"));
    m_OKText.SetFontPointSize(10);
    m_OKText.SetFontWeight(FW_NORMAL);
    m_OKText.SetWordWrap(TRUE);
    m_OKText.Show(FALSE);

    m_CancelText.SetFontFaceName(_T("Arial"));
    m_CancelText.SetFontPointSize(10);
    m_CancelText.SetFontWeight(FW_NORMAL);
    m_CancelText.SetWordWrap(TRUE);
    m_CancelText.Show(FALSE);

    AddObject(&m_Background);
    AddObject(&m_MessageText);
    AddObject(&m_OKBitmap);
    AddObject(&m_CancelBitmap);
    AddObject(&m_OKText);
    AddObject(&m_CancelText);

    return CLCDBase::Initialize();
}


//************************************************************************
//
// CLCDPopup::SetPopupType
//
//************************************************************************

void CLCDPopup::SetPopupType(PB_TYPE pbType)
{
    m_pbType = pbType;
    RecalcLayout();
}


//************************************************************************
//
// CLCDPopup::SetText
//
//************************************************************************

void CLCDPopup::SetText(LPCTSTR szMessage, LPCTSTR szOK, LPCTSTR szCancel)
{
    if (NULL != szMessage)
    {
        m_MessageText.SetText(szMessage);
    }
    if (NULL != szOK)
    {
        m_OKText.SetText(szOK);
    }
    if (NULL != szCancel)
    {
        m_CancelText.SetText(szCancel);
    }

    RecalcLayout();
}


//************************************************************************
//
// CLCDPopup::SetSize
//
//************************************************************************

void CLCDPopup::SetSize(int nCX, int nCY)
{
    CLCDPage::SetSize(nCX, nCY);
}


//************************************************************************
//
// CLCDPopup::RecalcLayout
//
//************************************************************************

void CLCDPopup::RecalcLayout(void)
{
    // First get the size of the text
    // If the width > MAX_WIDTH, snap it to the max width
    int nTextWidthWithMargin = 2 * POPUP_MARGIN;
    int nTextHeightWithMargin = 2 * POPUP_MARGIN;
    if(0 < _tcslen(m_MessageText.GetText()))
    {
        m_MessageText.CalculateExtent(FALSE);
        nTextWidthWithMargin = m_MessageText.GetVExtent().cx + 2*POPUP_MARGIN;
        nTextHeightWithMargin = m_MessageText.GetVExtent().cy + 2*POPUP_MARGIN;
        if(nTextWidthWithMargin > m_nMaxPopupWidth)
        {
            // Resize the text using a max height
            m_MessageText.SetSize(m_nMaxPopupWidth, LGLCD_QVGA_BMP_HEIGHT);
            m_MessageText.CalculateExtent(FALSE);
            nTextWidthWithMargin = m_MessageText.GetVExtent().cx + 2*POPUP_MARGIN;
            nTextHeightWithMargin = m_MessageText.GetVExtent().cy + 2*POPUP_MARGIN;
        }
        m_MessageText.SetSize(m_MessageText.GetVExtent().cx, m_MessageText.GetVExtent().cy);
    }
    else
    {
        nTextHeightWithMargin = POPUP_MARGIN;
        nTextWidthWithMargin = 0;
        m_MessageText.SetSize(0, 0);
    }

    // Determine the size of the popup including..
    // Margin
    // Text
    // OK Bitmap
    // Cancel Bitmap
    int nPopupWidth = nTextWidthWithMargin;
    int nPopupHeight = nTextHeightWithMargin;

    // Bitmap operations
    BITMAP bmOK, bmCancel;
    memset(&bmOK, 0, sizeof(bmOK));
    memset(&bmCancel, 0, sizeof(bmCancel));

    if (NULL != m_OKBitmap.GetBitmap())
    {
        GetObject( m_OKBitmap.GetBitmap(), sizeof(BITMAP), &bmOK);
        m_OKBitmap.SetSize(bmOK.bmWidth, bmOK.bmHeight);
    }
    if (NULL != m_CancelBitmap.GetBitmap())
    {
        GetObject( m_CancelBitmap.GetBitmap(), sizeof(BITMAP), &bmCancel);
        m_CancelBitmap.SetSize(bmCancel.bmWidth, bmCancel.bmHeight);
    }

    switch(m_pbType)
    {
    // Main text block only
    case PB_TEXT:
        m_OKBitmap.Show(FALSE);
        m_CancelBitmap.Show(FALSE);
        break;

    // OK is centered against the main text block
    case PB_OK:
        if (bmOK.bmWidth)
        {
            // Just factor in a bitmap
            // Add the bitmap w/ margin on the bottom
            nPopupHeight += bmOK.bmHeight;
            nPopupHeight += POPUP_MARGIN;

            // Position the bitmap centered, and vertically after the text+margin
            m_OKBitmap.SetOrigin((nPopupWidth-bmOK.bmWidth)/2, nTextHeightWithMargin);
            
            m_OKBitmap.Show(TRUE);
        }
        break;  

    // OK, Cancel Buttons are aligned on the same row below the text block (like a dialog)
    case PB_OKCANCEL:
        if (bmOK.bmWidth && bmCancel.bmWidth)
        {
            // Add the bitmap w/ margin on the bottom
            nPopupHeight += bmOK.bmHeight;
            nPopupHeight += POPUP_MARGIN;

            // OK is right-aligned with the text
            m_OKBitmap.SetOrigin(m_MessageText.GetOrigin().x + m_MessageText.GetSize().cx - bmOK.bmWidth,
                nTextHeightWithMargin);
            // Cancel is right-aligned with OK (with margin)
            m_CancelBitmap.SetOrigin(m_OKBitmap.GetOrigin().x - bmCancel.bmWidth - POPUP_MARGIN,
                nTextHeightWithMargin);

            m_OKBitmap.Show(TRUE);
            m_CancelBitmap.Show(TRUE);
        }
        break;

    // OK, Cancel Buttons are on separate rows centered against their own text blocks
    case PB_OKCANCEL_TEXT:
        {
            int nOKTextWithBitmapHeight = 0;
            int nMaxOKTextWidth = 0;
            m_OKText.SetOrigin(POPUP_MARGIN, nTextHeightWithMargin - POPUP_MARGIN);

            if (bmOK.bmWidth)
            {
                // OK is left-aligned with the text
                m_OKBitmap.SetOrigin(m_MessageText.GetOrigin().x, nTextHeightWithMargin);

                // OK text height is left-aligned with OK bmp and right-aligned with text
                // OK bmp is vertically centered against the OK text
                nMaxOKTextWidth = m_MessageText.GetWidth() - bmOK.bmWidth - POPUP_MARGIN;
                if (nMaxOKTextWidth < 0)
                {
                    nMaxOKTextWidth = LGLCD_QVGA_BMP_WIDTH;
                }
                m_OKText.SetOrigin(m_OKBitmap.GetOrigin().x + bmOK.bmWidth + POPUP_MARGIN, nTextHeightWithMargin);
                m_OKText.SetSize(nMaxOKTextWidth, LGLCD_QVGA_BMP_HEIGHT);
                m_OKText.CalculateExtent(FALSE);
                m_OKText.SetSize(m_OKText.GetVExtent().cx, m_OKText.GetVExtent().cy);                

                // if no main text, figure out required max popup width
                int nCalcPopupWidth = POPUP_MARGIN*3  + bmOK.bmWidth + m_OKText.GetSize().cx;
                if (nCalcPopupWidth > nPopupWidth)
                {
                    nPopupWidth = nCalcPopupWidth;
                }

                // Add the bitmap w/ margin on the bottom
                nOKTextWithBitmapHeight = max(bmOK.bmHeight, m_OKText.GetHeight());
                nPopupHeight += nOKTextWithBitmapHeight;
                nPopupHeight += POPUP_MARGIN;

                m_OKText.Show(TRUE);
                m_OKBitmap.Show(TRUE);
                m_OKBitmap.SetOrigin(m_OKBitmap.GetOrigin().x, m_OKText.GetOrigin().y + (m_OKText.GetHeight() - bmOK.bmHeight)/2);
            }

            if (bmCancel.bmWidth)
            {
                // Cancel is left-aligned with the text
                m_CancelBitmap.SetOrigin(m_MessageText.GetOrigin().x, m_OKText.GetOrigin().y + nOKTextWithBitmapHeight + POPUP_MARGIN);

                int nMaxCancelTextWidth = m_MessageText.GetWidth() - bmCancel.bmWidth - POPUP_MARGIN;
                if (nMaxCancelTextWidth < 0)
                {
                    nMaxCancelTextWidth = LGLCD_QVGA_BMP_WIDTH;
                }
                m_CancelText.SetOrigin(m_CancelBitmap.GetOrigin().x + bmCancel.bmWidth + POPUP_MARGIN, m_CancelBitmap.GetOrigin().y);
                m_CancelText.SetSize(nMaxCancelTextWidth, LGLCD_QVGA_BMP_HEIGHT);
                m_CancelText.CalculateExtent(FALSE);
                m_CancelText.SetSize(m_CancelText.GetVExtent().cx, m_CancelText.GetVExtent().cy);

                // if no main text, make the
                int nCalcPopupWidth = POPUP_MARGIN*3  + bmOK.bmWidth + m_CancelText.GetSize().cx;
                if (nCalcPopupWidth > nPopupWidth)
                {
                    nPopupWidth = nCalcPopupWidth;
                }

                nPopupHeight += max(bmCancel.bmHeight, m_CancelText.GetHeight());
                nPopupHeight += POPUP_MARGIN;

                // Vertically center the OK and CANCEL bitmaps against their respective texts
                m_CancelText.Show(TRUE);
                m_CancelBitmap.Show(TRUE);
                m_CancelBitmap.SetOrigin(m_CancelBitmap.GetOrigin().x, m_CancelText.GetOrigin().y + (m_CancelText.GetHeight() - bmCancel.bmHeight)/2);
            }
        }

        break;
    }

    // Resize ourself
    SetSize(nPopupWidth, nPopupHeight);

    // Center the popup in the middle of the screen
    SetOrigin( (LGLCD_QVGA_BMP_WIDTH - nPopupWidth)/2, (LGLCD_QVGA_BMP_HEIGHT - nPopupHeight)/2);

    // Resize the background
    m_Background.SetSize(GetWidth(), GetHeight());
}


//************************************************************************
//
// CLCDPopup::SetBitmaps
//
//************************************************************************

void CLCDPopup::SetBitmaps(HBITMAP hbmOK, HBITMAP hbmCancel)
{
    m_OKBitmap.SetBitmap(hbmOK);
    m_CancelBitmap.SetBitmap(hbmCancel);
    
    RecalcLayout();
}


//************************************************************************
//
// CLCDPopup::SetAlpha
//
//************************************************************************

void CLCDPopup::SetAlpha(BYTE bAlphaStart, BYTE bAlphaEnd)
{
    m_Background.SetAlphaLevel(bAlphaStart, bAlphaEnd);
}


//************************************************************************
//
// CLCDPopup::SetGradientMode
//
//************************************************************************

void CLCDPopup::SetGradientMode(BOOL bGradient)
{
    m_Background.SetGradientMode(bGradient);
}


//************************************************************************
//
// CLCDPopup::SetColor
//
//************************************************************************

void CLCDPopup::SetColor(COLORREF cfColor)
{
    m_Background.SetColor(cfColor);
}


//** end of LCDPopup.cpp *************************************************
