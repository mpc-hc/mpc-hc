//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDIcon.cpp
//
// The CLCDIcon class draws icons onto the lcd.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDIcon::CLCDIcon
//
//************************************************************************

CLCDIcon::CLCDIcon(void)
:   m_hIcon(NULL),
    m_nIconWidth(16),
    m_nIconHeight(16)
{
}


//************************************************************************
//
// CLCDIcon::CLCDIcon
//
//************************************************************************

CLCDIcon::~CLCDIcon(void)
{
}


//************************************************************************
//
// CLCDIcon::SetIcon
//
//************************************************************************

void CLCDIcon::SetIcon(HICON hIcon, int nWidth /* = 16 */, int nHeight /* = 16 */)
{
    m_hIcon = hIcon;
    m_nIconWidth = nWidth;
    m_nIconHeight = nHeight;
}


//************************************************************************
//
// CLCDIcon::OnDraw
//
//************************************************************************

void CLCDIcon::OnDraw(CLCDGfxBase &rGfx)
{
    if (m_hIcon)
    {
        int nOldBkMode = SetBkMode(rGfx.GetHDC(), TRANSPARENT);

        DrawIconEx(rGfx.GetHDC(), 0, 0, m_hIcon,
                   m_nIconWidth, m_nIconHeight, 0, NULL, DI_NORMAL);
        if (m_bInverted)
        {
            RECT rBoundary = { 0, 0, m_nIconWidth, m_nIconHeight};
            InvertRect(rGfx.GetHDC(), &rBoundary);
        }
        SetBkMode(rGfx.GetHDC(), nOldBkMode);
    }
}


//** end of LCDIcon.cpp **************************************************
