//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDAnimatedBitmap.cpp
//
// The CLCDAnimatedBitmap class draws animated bitmaps onto the LCD.
// An animated bitmap consists of a tiled bitmap representing the
// animation. The tile size is set with the SetSubpicWidth.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDAnimatedBitmap::CLCDAnimatedBitmap
//
//************************************************************************

CLCDAnimatedBitmap::CLCDAnimatedBitmap(void)
{
    m_dwCurrSubpic = 0;
    m_dwTotalSubpics = 0;
}


//************************************************************************
//
// CLCDAnimatedBitmap::CLCDAnimatedBitmap
//
//************************************************************************

CLCDAnimatedBitmap::~CLCDAnimatedBitmap(void)
{

}


//************************************************************************
//
// CLCDAnimatedBitmap::Initialize
//
//************************************************************************

HRESULT CLCDAnimatedBitmap::Initialize(void)
{
    m_dwRate        = 250;
    m_dwElapsedTime = 0;
    m_dwLastUpdate = GetTickCount();

    return S_OK;
}


//************************************************************************
//
// CLCDAnimatedBitmap::ResetUpdate
//
//************************************************************************

void CLCDAnimatedBitmap::ResetUpdate(void)
{
    m_dwCurrSubpic = 0;
    m_dwLastUpdate = GetTickCount();
}


//************************************************************************
//
// CLCDAnimatedBitmap::SetSubpicWidth
//
//************************************************************************

void CLCDAnimatedBitmap::SetSubpicWidth(DWORD dwWidth)
{
    m_dwSubpicWidth = dwWidth;
    LCDUIASSERT(NULL != m_hBitmap);
    LCDUIASSERT(0 != dwWidth);
    if((NULL != m_hBitmap) && (0 != dwWidth))
    {
        // figure out how many tiles we have
        BITMAP bitmap;
        if(GetObject(m_hBitmap, sizeof(bitmap), &bitmap))
        {
            m_dwTotalSubpics = bitmap.bmWidth / dwWidth;
            SetLogicalSize(bitmap.bmWidth, bitmap.bmHeight);
        }
        else
        {
            m_dwTotalSubpics = 0;
        }
    }
    else
    {
        m_dwTotalSubpics = 0;
    }
}

//************************************************************************
//
// CLCDAnimatedBitmap::SetAnimationRate
//
//************************************************************************

void CLCDAnimatedBitmap::SetAnimationRate(DWORD dwRate)
{
    m_dwRate = dwRate;
}


//************************************************************************
//
// CLCDAnimatedBitmap::OnUpdate
//
//************************************************************************

void CLCDAnimatedBitmap::OnUpdate(DWORD dwTimestamp)
{
    m_dwElapsedTime = (dwTimestamp - m_dwLastUpdate);

    // Just update the logical origin
    SetLogicalOrigin(-1 * m_dwSubpicWidth * m_dwCurrSubpic, 0);

    DWORD increment = m_dwElapsedTime / m_dwRate;
    if(increment > 0)
    {
        m_dwCurrSubpic += increment;
        m_dwCurrSubpic %= m_dwTotalSubpics;
        m_dwElapsedTime %= m_dwRate;
        m_dwLastUpdate = GetTickCount();
    }
}


//** end of LCDAnimatedBitmap.cpp ****************************************
