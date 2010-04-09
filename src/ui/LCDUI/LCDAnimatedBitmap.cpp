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
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "LCDAnimatedBitmap.h"


//************************************************************************
//
// CLCDAnimatedBitmap::CLCDAnimatedBitmap
//
//************************************************************************

CLCDAnimatedBitmap::CLCDAnimatedBitmap()
{
    m_dwCurrSubpic = m_dwTotalSubpics = 0;
}


//************************************************************************
//
// CLCDAnimatedBitmap::CLCDAnimatedBitmap
//
//************************************************************************

CLCDAnimatedBitmap::~CLCDAnimatedBitmap()
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

    return CLCDBitmap::Initialize();
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
}


//************************************************************************
//
// CLCDAnimatedBitmap::OnDraw
//
//************************************************************************

void CLCDAnimatedBitmap::OnDraw(CLCDGfx &rGfx)
{
    if(m_dwTotalSubpics > 0)
    {
        int xoffs = m_dwCurrSubpic * m_dwSubpicWidth;

        DWORD increment = m_dwElapsedTime / m_dwRate;
        if(increment > 0)
        {
            m_dwCurrSubpic += increment;
            m_dwCurrSubpic %= m_dwTotalSubpics;
            m_dwElapsedTime %= m_dwRate;
            m_dwLastUpdate = GetTickCount();
        }
        
        // stolen from: CLCDBitmap::OnDraw(rGfx);
        if(m_hBitmap)
        {
            HDC hCompatibleDC = CreateCompatibleDC(rGfx.GetHDC());
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, m_hBitmap);
            
            // get
            BitBlt(rGfx.GetHDC(), 0, 0, m_Size.cx, m_Size.cy, hCompatibleDC, xoffs, 0, m_dwROP);
            
            // restores
            SelectObject(hCompatibleDC, hOldBitmap);
            DeleteDC(hCompatibleDC);
        }
    }
}


//** end of LCDAnimatedBitmap.cpp ****************************************
