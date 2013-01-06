//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// NOTE: This version of ColorLCDUI is pre-release and is subject to 
// change.
//
// CLCDColorText.cpp
//
// The CLCDColorText class draws scrollable color text onto the LCD.
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"

//************************************************************************
//
// CLCDColorText::CLCDColorText
//
//************************************************************************

CLCDColorText::CLCDColorText(void)
{
    m_nTextLength = 0;
    m_hFont = NULL;
    m_sText.erase(m_sText.begin(), m_sText.end());
    SetForegroundColor(RGB(255, 0, 255));
    m_bRecalcExtent = TRUE;
    ZeroMemory(&m_dtp, sizeof(DRAWTEXTPARAMS));
    m_dtp.cbSize = sizeof(DRAWTEXTPARAMS);
    m_nTextFormat = m_nTextAlignment = (DT_LEFT | DT_NOPREFIX);
    m_nTextAlignment = DT_LEFT;
    ZeroMemory(&m_sizeVExtent, sizeof(m_sizeVExtent));
    ZeroMemory(&m_sizeHExtent, sizeof(m_sizeHExtent));

    SetBackgroundMode(TRANSPARENT, RGB(255,255,255));

    //Default font
    LOGFONT lf;
    lf.lfCharSet = 0;
    lf.lfClipPrecision = 2;
    lf.lfEscapement = 0;
    lf.lfHeight = -32;
    lf.lfItalic = 0;
    lf.lfOrientation = 0;
    lf.lfOutPrecision = 3;
    lf.lfPitchAndFamily = 18;
    lf.lfQuality = 1;
    lf.lfStrikeOut = 0;
    lf.lfUnderline = 0;
    lf.lfWeight = 400;
    lf.lfWidth = 0;

    wsprintf(lf.lfFaceName, _T("Times New Roman") );

    m_hFont = CreateFontIndirect(&lf);

    m_StartX = 0;
    m_LoopX = 0;
    m_ScrollRate = 0;
    m_PixelLength = 0;
    m_RunningTime = 0;
    m_ScrollBuffer = 20;
    m_JumpDistance = 0;

    m_bAutoScroll = true;
}



//************************************************************************
//
// CLCDColorText::~CLCDColorText
//
//************************************************************************

CLCDColorText::~CLCDColorText()
{
}


//************************************************************************
//
// CLCDColorText::SetBackgroundMode
//
//************************************************************************

void CLCDColorText::SetBackgroundMode(int nMode, COLORREF color)
{
    m_nBkMode = nMode;
    m_backColor = color;
}


//************************************************************************
//
// CLCDColorText::OnDraw
//
//************************************************************************

void CLCDColorText::OnDraw(CLCDGfxBase &rGfx)
{
    if(GetBackgroundMode() == OPAQUE)
    {
        HBRUSH backbrush = CreateSolidBrush(m_backColor);

        // clear the clipped area
        RECT rcClp = { 0, 0, m_Size.cx, m_Size.cy };
        FillRect(rGfx.GetHDC(), &rcClp, backbrush);

        // clear the logical area
        RECT rcLog = { 0, 0, m_sizeLogical.cx, m_sizeLogical.cy };
        FillRect(rGfx.GetHDC(), &rcLog, backbrush);

        DeleteObject(backbrush);
    }

    if(m_nTextLength)
    {

        // map mode text, with transparency
        int nOldMapMode = SetMapMode(rGfx.GetHDC(), MM_TEXT);

        int nOldBkMode = SetBkMode(rGfx.GetHDC(), TRANSPARENT);

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

            //For scrolling
            m_PixelLength = m_sizeHExtent.cx;
            m_StartX = 0;

            if( m_bAutoScroll )
            {
                if( m_PixelLength > GetWidth() )
                {
                    m_ScrollRate = -1*GetHeight();
                }
                else
                {
                    m_ScrollRate = 0;
                }
            }

            if( m_ScrollRate > 0 )
            {
                if( GetWidth() > m_PixelLength + m_ScrollBuffer )
                {
                    m_JumpDistance = -1 * GetWidth();
                }
                else
                {
                    m_JumpDistance = -1 * (m_PixelLength + m_ScrollBuffer);
                }
            }
            else if( m_ScrollRate < 0 )
            {
                if( GetWidth() > m_PixelLength + m_ScrollBuffer )
                {
                    m_JumpDistance = GetWidth();
                }
                else
                {
                    m_JumpDistance = m_PixelLength + m_ScrollBuffer;
                }
            }

            m_LoopX = m_JumpDistance;
        }

        if( IsVisible() )
        {
            if( m_ScrollRate == 0 )
            {
                RECT rBoundary = { 0, 0, 0 + GetLogicalSize().cx, 0 + GetLogicalSize().cy }; 
                DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rBoundary, m_nTextFormat, &m_dtp);
            }
            else
            {
                RECT rBoundaryFirst = { m_StartX, 0, 0 + GetLogicalSize().cx, 0 + GetLogicalSize().cy }; 
                RECT rBoundarySecond = { m_LoopX, 0, 0 + GetLogicalSize().cx, 0 + GetLogicalSize().cy }; 

                DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), 
                    &rBoundaryFirst, m_nTextFormat, &m_dtp);
                DrawTextEx(rGfx.GetHDC(), (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), 
                    &rBoundarySecond, m_nTextFormat, &m_dtp);
            }
        }

        // restores
        SetMapMode(rGfx.GetHDC(), nOldMapMode);
        SetTextColor(rGfx.GetHDC(), crOldTextColor);
        SetBkMode(rGfx.GetHDC(), nOldBkMode);
        SelectObject(rGfx.GetHDC(), hOldFont);
    }
}


//************************************************************************
//
// CLCDColorText::OnUpdate
//
//************************************************************************

void CLCDColorText::OnUpdate(DWORD timestamp)
{
    if( m_ScrollRate != 0 && m_bRecalcExtent == FALSE )
    {
        //How much time has passed?
        if( m_RunningTime == 0 )
            m_RunningTime = timestamp;

        //Scrollrate is in pixels per second
        DWORD elapsed = timestamp - m_RunningTime; //milliseconds

        //Only update if a full second has passed
        if( elapsed < 1000 )
            return;

        float secs = elapsed / 1000.0f; //to seconds
        int jump = (int)(secs * m_ScrollRate + 0.5f); //to pixels

        m_StartX += jump;
        m_LoopX += jump;

        if( (m_ScrollRate > 0 && m_LoopX >= 0) ||
            (m_ScrollRate < 0 && m_LoopX <= 0) )
        {
            m_StartX = m_LoopX;
            m_LoopX += m_JumpDistance;
        }

        m_RunningTime = timestamp;
    }
}


//************************************************************************
//
// CLCDColorText::SetFontColor
//
//************************************************************************

void CLCDColorText::SetFontColor(COLORREF color)
{
    m_crForegroundColor = color;
}


//************************************************************************
//
// CLCDColorText::SetScrollRate
//
//************************************************************************

void CLCDColorText::SetScrollRate( int pixelspersec )
{
    m_ScrollRate = pixelspersec;

    m_StartX = 0; 
    m_bRecalcExtent = TRUE;
}

//** end of LCDColorText.cpp *********************************************
