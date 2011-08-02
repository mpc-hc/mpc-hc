//************************************************************************
//
// LCDText.cpp
//
// The CLCDText class draws simple text onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "stdafx.h"
#include "LCDText.h"


//************************************************************************
//
// CLCDText::CLCDText
//
//************************************************************************

CLCDText::CLCDText()
{
    m_nTextLength = 0;
    m_hFont = NULL;
    m_sText.erase(m_sText.begin(), m_sText.end());
    m_crColor = RGB(255, 255, 255); // white
    m_bRecalcExtent = TRUE;
    ZeroMemory(&m_dtp, sizeof(DRAWTEXTPARAMS));
    m_dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    m_nTextFormat = m_nTextAlignment = (DT_LEFT | DT_NOPREFIX);
    m_nTextAlignment = DT_LEFT;
    ZeroMemory(&m_sizeVExtent, sizeof(m_sizeVExtent));
    ZeroMemory(&m_sizeHExtent, sizeof(m_sizeHExtent));
}


//************************************************************************
//
// CLCDText::~CLCDText
//
//************************************************************************

CLCDText::~CLCDText()
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
        return;

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    GetObject(m_hFont, sizeof(LOGFONT), &lf);

    //_tcsncpy(lf.lfFaceName, szFontName, LF_FACESIZE);
    //_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, szFontName, LF_FACESIZE);
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

void CLCDText::DrawText(CLCDGfx &rGfx)
{
    // draw the text
    RECT rBoundary = { 0, 0,0 + GetLogicalSize().cx, 0 + GetLogicalSize().cy }; 
    DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rBoundary, m_nTextFormat, &m_dtp);
    
//    LCDUITRACE(_T("Drawing %s at (%d,%d)-(%d-%d) lmargin=%d, rmargin=%d\n"),
//         m_sText.c_str(), m_Origin.x, m_Origin.y, GetWidth(), GetHeight(),
//         m_dtp.iLeftMargin, m_dtp.iRightMargin);

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

void CLCDText::OnDraw(CLCDGfx &rGfx)
{

    if (GetBackgroundMode() == OPAQUE)
    {
        // clear the clipped area
        RECT rcClp = { 0, 0, m_Size.cx, m_Size.cy };
        FillRect(rGfx.GetHDC(), &rcClp, (HBRUSH) GetStockObject(BLACK_BRUSH));
    
        // clear the logical area
        RECT rcLog = { 0, 0, m_sizeLogical.cx, m_sizeLogical.cy };
        FillRect(rGfx.GetHDC(), &rcLog, (HBRUSH) GetStockObject(BLACK_BRUSH));
    }
    
    if (m_nTextLength)
    {

        // map mode text, with transparency
        int nOldMapMode = SetMapMode(rGfx.GetHDC(), MM_TEXT);
        int nOldBkMode = SetBkMode(rGfx.GetHDC(), GetBackgroundMode()); 

        // select current font
        HFONT hOldFont = (HFONT)SelectObject(rGfx.GetHDC(), m_hFont);   

        // select color
        COLORREF crOldTextColor = SetTextColor(rGfx.GetHDC(), m_crColor);
        
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
        SelectObject(rGfx.GetHDC(), hOldFont);
    }
}

//** end of LCDText.cpp **************************************************

