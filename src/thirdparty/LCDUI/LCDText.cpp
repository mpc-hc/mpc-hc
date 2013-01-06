//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDText.cpp
//
// The CLCDText class draws simple text onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDText::CLCDText
//
//************************************************************************

CLCDText::CLCDText(void)
:   m_hFont(NULL),
    m_nTextLength(0),
    m_nTextFormat(DT_LEFT | DT_NOPREFIX),
    m_bRecalcExtent(TRUE),
    m_nTextAlignment(DT_LEFT)
{
    ZeroMemory(&m_dtp, sizeof(DRAWTEXTPARAMS));
    m_dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    ZeroMemory(&m_sizeVExtent, sizeof(m_sizeVExtent));
    ZeroMemory(&m_sizeHExtent, sizeof(m_sizeHExtent));
    SetBackgroundMode(TRANSPARENT);
    Initialize();
}


//************************************************************************
//
// CLCDText::~CLCDText
//
//************************************************************************

CLCDText::~CLCDText(void)
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
}


//************************************************************************
//
// CLCDText::Initialize
//
//************************************************************************

HRESULT CLCDText::Initialize()
{
    m_hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if(NULL != m_hFont)
    {
        SetFontPointSize(DEFAULT_POINTSIZE);
    }
    SetForegroundColor(RGB(255, 255, 255));
    return (NULL != m_hFont) ? S_OK : E_OUTOFMEMORY;
}


//************************************************************************
//
// CLCDText::SetFont
//
//************************************************************************

void CLCDText::SetFont(LOGFONT& lf)
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }

    m_hFont = CreateFontIndirect(&lf);
    m_bRecalcExtent = TRUE;
}


//************************************************************************
//
// CLCDText::GetFont
//
//************************************************************************

HFONT CLCDText::GetFont()
{
    return m_hFont;
}


//************************************************************************
//
// CLCDText::SetFontFaceName
//
//************************************************************************

void CLCDText::SetFontFaceName(LPCTSTR szFontName)
{
    // if NULL, uses the default gui font
    if (NULL == szFontName)
    {
        return;
    }

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    LCDUI_tcsncpy(lf.lfFaceName, szFontName, LF_FACESIZE);

    SetFont(lf);
}


//************************************************************************
//
// CLCDText::SetFontPointSize
//
//************************************************************************

void CLCDText::SetFontPointSize(int nPointSize)
{
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    lf.lfHeight = -MulDiv(nPointSize, DEFAULT_DPI, 72);

    SetFont(lf);
}


//************************************************************************
//
// CLCDText::SetFontWeight
//
//************************************************************************

void CLCDText::SetFontWeight(int nWeight)
{
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    lf.lfWeight = nWeight;

    SetFont(lf);
}


//************************************************************************
//
// CLCDText::SetFontColor
//
//************************************************************************

void CLCDText::SetFontColor(COLORREF color)
{
    SetForegroundColor(color);
}


//************************************************************************
//
// CLCDText::SetText
//
//************************************************************************

void CLCDText::SetText(LPCTSTR szText)
{
    LCDUIASSERT(NULL != szText);
    if(szText && _tcscmp(m_sText.c_str(), szText))
    {
        m_sText.assign(szText);
        m_nTextLength = m_sText.size();
        m_dtp.iLeftMargin = 0;
        m_dtp.iRightMargin = 0;
        m_bRecalcExtent = TRUE;
    }
}


//************************************************************************
//
// CLCDText::GetText
//
//************************************************************************

LPCTSTR CLCDText::GetText()
{
    return m_sText.c_str();
}


//************************************************************************
//
// CLCDText::SetWordWrap
//
//************************************************************************

void CLCDText::SetWordWrap(BOOL bEnable)
{
    if (bEnable)
    {
        m_nTextFormat |= DT_WORDBREAK;
    }
    else
    {
        m_nTextFormat &= ~DT_WORDBREAK;
    }
    m_bRecalcExtent = TRUE;
}


//************************************************************************
//
// CLCDText::SetLeftMargin
//
//************************************************************************

void CLCDText::SetLeftMargin(int nLeftMargin)
{
    m_dtp.iLeftMargin = nLeftMargin;
}


//************************************************************************
//
// CLCDText::SetRightMargin
//
//************************************************************************

void CLCDText::SetRightMargin(int nRightMargin)
{
    m_dtp.iRightMargin = nRightMargin;
}


//************************************************************************
//
// CLCDText::GetLeftMargin
//
//************************************************************************

int CLCDText::GetLeftMargin(void)
{
    return m_dtp.iLeftMargin;
}


//************************************************************************
//
// CLCDText::GetRightMargin
//
//************************************************************************

int CLCDText::GetRightMargin(void)
{
    return m_dtp.iRightMargin;
}


//************************************************************************
//
// CLCDText::GetVExtent
//
//************************************************************************

SIZE& CLCDText::GetVExtent()
{
    return m_sizeVExtent;
}


//************************************************************************
//
// CLCDText::GetHExtent
//
//************************************************************************

SIZE& CLCDText::GetHExtent()
{
    return m_sizeHExtent;
}


//************************************************************************
//
// CLCDText::CalculateExtent
//
//************************************************************************

void CLCDText::CalculateExtent(BOOL bSingleLine)
{
    HDC hdc = CreateCompatibleDC(NULL);

    int nOldMapMode = SetMapMode(hdc, MM_TEXT);
    int nOldBkMode = SetBkMode(hdc, GetBackgroundMode());
    // select current font
    HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);

    int nTextFormat;
    RECT rExtent;

    if(bSingleLine)
    {
        // calculate horizontal extent w/ single line, we can get the line height
        nTextFormat = (m_nTextFormat | DT_SINGLELINE | DT_CALCRECT);
        rExtent.left = rExtent.top = 0;
        rExtent.right = m_Size.cx;
        rExtent.bottom = m_Size.cy;
        DrawTextEx(hdc, (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rExtent, nTextFormat, &m_dtp);
        m_sizeHExtent.cx = rExtent.right;
        m_sizeHExtent.cy = rExtent.bottom;
    }
    else
    {
        // calculate vertical extent with word wrap
        nTextFormat = (m_nTextFormat | DT_WORDBREAK | DT_CALCRECT);
        rExtent.left = rExtent.top = 0;
        rExtent.right = m_Size.cx;
        rExtent.bottom = m_Size.cy;
        DrawTextEx(hdc, (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rExtent, nTextFormat, &m_dtp);
        m_sizeVExtent.cx = rExtent.right;
        m_sizeVExtent.cy = rExtent.bottom;
    }

    // restores
    SetMapMode(hdc, nOldMapMode);
    SetBkMode(hdc, nOldBkMode);
    SelectObject(hdc, hOldFont);

    DeleteDC(hdc);
}


//************************************************************************
//
// CLCDText::SetAlignment
//
//************************************************************************

void CLCDText::SetAlignment(int nAlignment)
{
    m_nTextFormat &= ~m_nTextAlignment;
    m_nTextFormat |= nAlignment;
    m_nTextAlignment = nAlignment;
}


//************************************************************************
//
// CLCDText::DrawText
//
//************************************************************************

void CLCDText::DrawText(CLCDGfxBase &rGfx)
{
    // draw the text
    RECT rBoundary = { 0, 0,0 + GetLogicalSize().cx, 0 + GetLogicalSize().cy }; 
    DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rBoundary, m_nTextFormat, &m_dtp);

    if (m_bInverted)
    {
        InvertRect(rGfx.GetHDC(), &rBoundary);
    }
}


//************************************************************************
//
// CLCDText::OnDraw
//
//************************************************************************

void CLCDText::OnDraw(CLCDGfxBase &rGfx)
{
    if (GetBackgroundMode() == OPAQUE)
    {
        HBRUSH hBackBrush = CreateSolidBrush(m_crBackgroundColor);

        // clear the clipped area
        RECT rcClp = { 0, 0, m_Size.cx, m_Size.cy };
        FillRect(rGfx.GetHDC(), &rcClp, (HBRUSH) GetStockObject(BLACK_BRUSH));
    
        // clear the logical area
        RECT rcLog = { 0, 0, m_sizeLogical.cx, m_sizeLogical.cy };
        FillRect(rGfx.GetHDC(), &rcLog, hBackBrush);

        DeleteObject(hBackBrush);
    }
    
    if (m_nTextLength)
    {

        // map mode text, with transparency
        int nOldMapMode = SetMapMode(rGfx.GetHDC(), MM_TEXT);
        int nOldBkMode = SetBkMode(rGfx.GetHDC(), GetBackgroundMode()); 
        COLORREF nOldBkColor = SetBkColor(rGfx.GetHDC(), m_crBackgroundColor);

        // select current font
        HFONT hOldFont = (HFONT)SelectObject(rGfx.GetHDC(), m_hFont);   

        // select color
        COLORREF crOldTextColor = SetTextColor(rGfx.GetHDC(), m_crForegroundColor);
        
        if (m_bRecalcExtent)
        {
            int nTextFormat;

            RECT rExtent;
            // calculate vertical extent with word wrap
            nTextFormat = (m_nTextFormat | DT_WORDBREAK | DT_CALCRECT);
            rExtent.left = rExtent.top = 0;
            rExtent.right = GetWidth();
            rExtent.bottom = GetHeight();
            DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rExtent, nTextFormat, &m_dtp);
            m_sizeVExtent.cx = rExtent.right;
            m_sizeVExtent.cy = rExtent.bottom;

            // calculate horizontal extent w/o word wrap
            nTextFormat = (m_nTextFormat | DT_CALCRECT);
            rExtent.left = rExtent.top = 0;
            rExtent.right = GetWidth();
            rExtent.bottom = GetHeight();
            DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rExtent, nTextFormat, &m_dtp);
            m_sizeHExtent.cx = rExtent.right;
            m_sizeHExtent.cy = rExtent.bottom;

            m_bRecalcExtent = FALSE;
        }

        if (IsVisible())
        {
            DrawText(rGfx);
        }

        // restores
        SetMapMode(rGfx.GetHDC(), nOldMapMode);
        SetTextColor(rGfx.GetHDC(), crOldTextColor);
        SetBkMode(rGfx.GetHDC(), nOldBkMode);
        SetBkColor(rGfx.GetHDC(), nOldBkColor);
        SelectObject(rGfx.GetHDC(), hOldFont);
    }
}

//** end of LCDText.cpp **************************************************

