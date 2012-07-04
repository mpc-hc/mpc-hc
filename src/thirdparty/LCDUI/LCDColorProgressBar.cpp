//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDColorProgressBar.cpp
//
// The CLCDColorProgressBar class draws a progress bar onto the color LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDColorProgressBar::CLCDColorProgressBar
//
//************************************************************************

CLCDColorProgressBar::CLCDColorProgressBar(void)
    : CLCDProgressBar()
{
    SetProgressStyle(STYLE_FILLED);
    SetCursorWidth(25);
    SetBackgroundColor(RGB(0, 255, 100));
    SetBorderThickness(3);

    // Show a border by default
    EnableBorder(FALSE);
    SetBorderColor(RGB(0, 100, 150));
}


//************************************************************************
//
// CLCDColorProgressBar::~CLCDColorProgressBar
//
//************************************************************************

CLCDColorProgressBar::~CLCDColorProgressBar(void)
{
}


//************************************************************************
//
// CLCDColorProgressBar::OnDraw
//
//************************************************************************

void CLCDColorProgressBar::OnDraw(CLCDGfxBase &rGfx)
{
    HBRUSH hCursorBrush = CreateSolidBrush(m_crForegroundColor);
    HBRUSH hBackBrush = CreateSolidBrush(m_crBackgroundColor);

    RECT rBoundary = { 0, 0, GetWidth(), GetHeight() };

    // Draw the background/border or just background
    if (m_bBorderOn && (m_nBorderThickness > 0))
    {
        HBRUSH hOldBrush = (HBRUSH)SelectObject(rGfx.GetHDC(), hBackBrush);
        HPEN hBorderPen = CreatePen(PS_INSIDEFRAME, m_nBorderThickness, m_crBorderColor);
        HPEN hOldPen = (HPEN)SelectObject(rGfx.GetHDC(), hBorderPen);
        Rectangle(rGfx.GetHDC(), rBoundary.left, rBoundary.top, rBoundary.right, rBoundary.bottom);
        SelectObject(rGfx.GetHDC(), hOldPen);
        SelectObject(rGfx.GetHDC(), hOldBrush);
        DeleteObject(hBorderPen);

        // adjust the progress boundary by the thickness
        rBoundary.top += m_nBorderThickness;
        rBoundary.bottom -= m_nBorderThickness;
        rBoundary.left += m_nBorderThickness;
        rBoundary.right -= m_nBorderThickness;
    }
    else
    {
        FillRect(rGfx.GetHDC(), &rBoundary, hBackBrush);
    }

    //Drawing the cursor
    switch(m_eStyle)
    {      
    case STYLE_CURSOR:
        {
            RECT r = rBoundary;
            int nCursorPos = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                (float)rBoundary.left, (float)(rBoundary.right - m_nCursorWidth),
                m_fPos);
            r.left = nCursorPos;
            r.right = nCursorPos + m_nCursorWidth;
            FillRect(rGfx.GetHDC(), &r, hCursorBrush);

            break;
        }
    case STYLE_FILLED:
        {
            RECT r = rBoundary;
            int nBarWidth = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                (float)rBoundary.left, (float)(rBoundary.right),
                m_fPos);
            r.right = nBarWidth ? nBarWidth : r.left;
            FillRect(rGfx.GetHDC(), &r, hCursorBrush);

            break;
        }
    case STYLE_DASHED_CURSOR:
        {
            RECT r = rBoundary;
            int nCursorPos = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                (float)rBoundary.left, (float)(rBoundary.right - m_nCursorWidth),
                m_fPos);
            r.left = nCursorPos;
            r.right = nCursorPos + m_nCursorWidth;
            FillRect(rGfx.GetHDC(), &r, hCursorBrush);
            HPEN hCursorPen = ::CreatePen(PS_DOT, 1, m_crForegroundColor);
            HPEN hOldPen = (HPEN)::SelectObject(rGfx.GetHDC(), hCursorPen);

            ::MoveToEx(rGfx.GetHDC(), rBoundary.left, (r.bottom - r.top)/2, NULL);
            ::LineTo(rGfx.GetHDC(), nCursorPos, (r.bottom - r.top)/2);

            DeleteObject(hCursorPen);
            ::SelectObject(rGfx.GetHDC(), hOldPen);
            break;
        }

    default:
        break;
    }

    DeleteObject(hBackBrush);
    DeleteObject(hCursorBrush);
}


//************************************************************************
//
// CLCDColorProgressBar::SetCursorColor
//
//************************************************************************

void CLCDColorProgressBar::SetCursorColor(COLORREF color)
{
    SetForegroundColor(color);
}


//************************************************************************
//
// CLCDColorProgressBar::EnableBorder
//
//************************************************************************

void CLCDColorProgressBar::EnableBorder(BOOL bEnable)
{
    m_bBorderOn = bEnable;
}


//************************************************************************
//
// CLCDColorProgressBar::SetBorderColor
//
//************************************************************************

void CLCDColorProgressBar::SetBorderColor(COLORREF color)
{
    m_crBorderColor = color;
}


//************************************************************************
//
// CLCDColorProgressBar::SetBorderThickness
//
//************************************************************************

void CLCDColorProgressBar::SetBorderThickness(int thickness)
{
    m_nBorderThickness = thickness;
}


//************************************************************************
//
// CLCDColorProgressBar::SetCursorWidth
//
//************************************************************************

void CLCDColorProgressBar::SetCursorWidth(int width)
{
    m_nCursorWidth = width;
}

//** end of LCDColorProgressBar.cpp **************************************
