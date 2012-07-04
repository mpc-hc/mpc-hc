//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDBitmap.cpp
//
// The CLCDBitmap class draws bitmaps onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDBitmap::CLCDBitmap
//
//************************************************************************

CLCDBitmap::CLCDBitmap(void)
{
    m_hBitmap = NULL;
    m_dwROP = SRCCOPY;
    m_fZoom = 1.0f;
    m_bAlpha = TRUE;
}


//************************************************************************
//
// CLCDBitmap::CLCDBitmap
//
//************************************************************************

CLCDBitmap::~CLCDBitmap(void)
{

}


//************************************************************************
//
// CLCDBitmap::SetBitmap
//
//************************************************************************

void CLCDBitmap::SetBitmap(HBITMAP hBitmap)
{
    m_hBitmap = hBitmap;
}


//************************************************************************
//
// CLCDBitmap::GetBitmap
//
//************************************************************************
HBITMAP CLCDBitmap::GetBitmap(void)
{
    return m_hBitmap;
}


//************************************************************************
//
// CLCDBitmap::SetBitmap
//
//************************************************************************

void CLCDBitmap::SetROP(DWORD dwROP)
{
    m_dwROP = dwROP;
}


//************************************************************************
//
// CLCDBitmap::SetZoomLevel
//
//************************************************************************

void CLCDBitmap::SetZoomLevel(float fzoom)
{
    m_fZoom = fzoom; 
}


//************************************************************************
//
// CLCDBitmap::GetZoomLevel
//
//************************************************************************

float CLCDBitmap::GetZoomLevel(void)
{
    return m_fZoom; 
}


//************************************************************************
//
// CLCDBitmap::SetAlpha
//
//************************************************************************

void CLCDBitmap::SetAlpha(BOOL bAlpha)
{
    m_bAlpha = bAlpha;
}


//************************************************************************
//
// CLCDBitmap::OnDraw
//
// The logical size is our 'canvas'.
// The control size is our 'window'. The window size can be smaller than the canvas.
// The assumption is that the bitmap size is the size of the canvas.
//************************************************************************

void CLCDBitmap::OnDraw(CLCDGfxBase &rGfx)
{
    if(m_hBitmap)
    {
        HDC hCompatibleDC = CreateCompatibleDC(rGfx.GetHDC());
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hCompatibleDC, m_hBitmap);
        
        // If monochrome output, don't even bother with alpha blend
        if (LGLCD_BMP_FORMAT_160x43x1 == rGfx.GetLCDScreen()->hdr.Format)
        {
            BitBlt(rGfx.GetHDC(), 0, 0, m_sizeLogical.cx, m_sizeLogical.cy, hCompatibleDC, 0, 0, m_dwROP);
        }
        else
        {
            if(0.001f > fabs(1.0f - m_fZoom))
            {
                BOOL b = FALSE;
                if(m_bAlpha)
                {
                    BLENDFUNCTION opblender = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
                    b = AlphaBlend(rGfx.GetHDC(), 0, 0, m_sizeLogical.cx, m_sizeLogical.cy, hCompatibleDC, 0, 0, m_sizeLogical.cx, m_sizeLogical.cy, opblender);
                }
                else
                {
                    BitBlt(rGfx.GetHDC(), 0, 0, m_sizeLogical.cx, m_sizeLogical.cy, hCompatibleDC, 0, 0, m_dwROP);
                }
            }
            else
            {
                if(m_bAlpha)
                {
                    BLENDFUNCTION opblender = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
                    AlphaBlend(rGfx.GetHDC(), 0, 0, (int)(m_fZoom* m_sizeLogical.cx), (int)(m_fZoom*m_sizeLogical.cy), hCompatibleDC, 0, 0, m_sizeLogical.cx, m_sizeLogical.cy, opblender);
                }
                else
                {
                    BLENDFUNCTION opblender = {AC_SRC_OVER, 0, 255, 0};
                    AlphaBlend(rGfx.GetHDC(), 0, 0, (int)(m_fZoom* m_sizeLogical.cx), (int)(m_fZoom*m_sizeLogical.cy), hCompatibleDC, 0, 0, m_sizeLogical.cx, m_sizeLogical.cy, opblender);
                }
            }
        }
        
        // restores
        SelectObject(hCompatibleDC, hOldBitmap);
        DeleteDC(hCompatibleDC);
    }
}


//** end of LCDBitmap.cpp ************************************************
