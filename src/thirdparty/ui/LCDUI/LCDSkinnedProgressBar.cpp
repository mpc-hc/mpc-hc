//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// CLCDSkinnedProgressBar.cpp
//
// Color LCD Skinned Progress Bar class
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDSkinnedProgressBar::CLCDSkinnedProgressBar
//
//************************************************************************

CLCDSkinnedProgressBar::CLCDSkinnedProgressBar(void)
{
    m_hBackground = NULL;
    m_hFiller = NULL;
    m_hCursor = NULL;
    m_hHighlight = NULL;
    m_h3PCursorLeft = NULL;
    m_h3PCursorMid = NULL;
    m_h3PCursorRight = NULL;

    m_BackgroundHeight = 0;
    m_BackgroundWidth = 0;

    m_FillerHeight = 0;
    m_FillerWidth = 0;

    m_CursorHeight = 0;
    m_CursorWidth = 0;

    m_HighlightHeight = 0;
    m_HighlightWidth = 0;

    m_bUse3P = FALSE;
    
    m_3PCursorLeftHeight = 0;
    m_3PCursorLeftWidth = 0;

    m_3PCursorMidHeight = 0;
    m_3PCursorMidWidth = 0;

    m_3PCursorRightHeight = 0;
    m_3PCursorRightWidth = 0; 
}


//************************************************************************
//
// CLCDSkinnedProgressBar::~CLCDSkinnedProgressBar
//
//************************************************************************

CLCDSkinnedProgressBar::~CLCDSkinnedProgressBar(void)
{
}


//************************************************************************
//
// CLCDSkinnedProgressBar::SetBackground
//
//************************************************************************

void CLCDSkinnedProgressBar::SetBackground(HBITMAP background, int bmpWidth, int bmpHeight)
{
    m_hBackground = background;
    m_BackgroundHeight = bmpHeight;
    m_BackgroundWidth = bmpWidth;
    SetSize(bmpWidth, bmpHeight);
}


//************************************************************************
//
// CLCDSkinnedProgressBar::SetFiller
//
//************************************************************************

void CLCDSkinnedProgressBar::SetFiller(HBITMAP cursor, int bmpWidth, int bmpHeight)
{
    m_bUse3P = FALSE;
    m_hFiller = cursor;
    m_FillerHeight = bmpHeight;
    m_FillerWidth = bmpWidth;
    SetSize(bmpWidth, bmpHeight);
}


//************************************************************************
//
// CLCDSkinnedProgressBar::SetCursor
//
//************************************************************************

void CLCDSkinnedProgressBar::SetCursor(HBITMAP cursor, int bmpWidth, int bmpHeight)
{
    m_bUse3P = FALSE;
    m_hCursor = cursor;
    m_CursorHeight = bmpHeight;
    m_CursorWidth = bmpWidth;
    m_nCursorWidth = m_CursorWidth;
}


//************************************************************************
//
// CLCDSkinnedProgressBar::SetThreePieceCursor
//
//************************************************************************

void CLCDSkinnedProgressBar::SetThreePieceCursor(HBITMAP left, int bmpLeftWidth, int bmpLeftHeight,
                             HBITMAP mid, int bmpMidWidth, int bmpMidHeight,
                             HBITMAP right, int bmpRightWidth, int bmpRightHeight )
{
    m_bUse3P = TRUE;

    m_h3PCursorLeft = left;
    m_3PCursorLeftHeight = bmpLeftHeight;
    m_3PCursorLeftWidth = bmpLeftWidth;

    m_h3PCursorMid = mid;
    m_3PCursorMidHeight = bmpMidHeight;
    m_3PCursorMidWidth = bmpMidWidth;

    m_h3PCursorRight = right;
    m_3PCursorRightHeight = bmpRightHeight;
    m_3PCursorRightWidth = bmpRightWidth; 

    m_nCursorWidth = m_3PCursorLeftWidth + m_3PCursorMidWidth + m_3PCursorRightWidth;
}

//************************************************************************
//
// CLCDSkinnedProgressBar::AddHighlight
//
//************************************************************************

void CLCDSkinnedProgressBar::AddHighlight(HBITMAP highlight, int bmpWidth, int bmpHeight)
{
    m_hHighlight = highlight;
    m_HighlightHeight = bmpHeight;
    m_HighlightWidth = bmpWidth;
}

//************************************************************************
//
// CLCDSkinnedProgressBar::OnDraw
//
//************************************************************************

void CLCDSkinnedProgressBar::OnDraw(CLCDGfxBase &rGfx)
{
    RECT rBoundary = { 0, 0, GetWidth(), GetHeight() };

    HDC hdcMem = CreateCompatibleDC(rGfx.GetHDC());
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, m_hBackground);

    //Draw the background
    //BitBlt the background onto the screen
    BLENDFUNCTION opblender = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    AlphaBlend(rGfx.GetHDC(), 0, 0, GetWidth(), GetHeight(), hdcMem, 0, 0, GetWidth(), GetHeight(), opblender);

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);

    //Drawing the cursor
    switch(m_eStyle)
    {
    case STYLE_FILLED:
        {
            

            HDC hdcMemCursor = CreateCompatibleDC(rGfx.GetHDC());

            if(m_bUse3P)
            {
                RECT r = rBoundary;
                r.left = 0;
                r.right = GetWidth() - m_3PCursorRightWidth - m_3PCursorLeftWidth;
                int nBarWidth = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                    (float)r.left, (float)r.right,
                    m_fPos);

                int midstart, midwidth;
                midstart = m_3PCursorLeftWidth;
                midwidth = nBarWidth;

                //Left
                hbmOld = (HBITMAP)SelectObject(hdcMemCursor, m_h3PCursorLeft);

                AlphaBlend(rGfx.GetHDC(), 0, 0, m_3PCursorLeftWidth, GetHeight(), 
                    hdcMemCursor, 0, 0, m_3PCursorLeftWidth, m_3PCursorLeftHeight, opblender);

                //Mid
                SelectObject(hdcMemCursor, m_h3PCursorMid);

                AlphaBlend(rGfx.GetHDC(), midstart, 0, midwidth, GetHeight(), 
                    hdcMemCursor, 0, 0, m_3PCursorMidWidth, m_3PCursorMidHeight, opblender);

                //Right
                SelectObject(hdcMemCursor, m_h3PCursorRight);

                AlphaBlend(rGfx.GetHDC(), midstart+midwidth, 0, m_3PCursorRightWidth, GetHeight(), 
                    hdcMemCursor, 0, 0, m_3PCursorRightWidth, m_3PCursorRightHeight, opblender);

                // restore previous bitmap
                SelectObject(hdcMemCursor, hbmOld);
                
            }
            else
            {     
                RECT r = rBoundary;
                int nBarWidth = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                    (float)rBoundary.left, (float)(rBoundary.right),
                    m_fPos);
                r.right = nBarWidth ? nBarWidth : r.left;

                HBITMAP hbmOldCursor = (HBITMAP)SelectObject(hdcMemCursor, m_hFiller);

                BitBlt(rGfx.GetHDC(), 0, 0, nBarWidth, GetHeight(), hdcMemCursor, 0, 0, SRCCOPY);

                SelectObject(hdcMemCursor, hbmOldCursor);
            }

            DeleteDC(hdcMemCursor);

            break;
        }
        //These two cases will be the same
    case STYLE_CURSOR:
    case STYLE_DASHED_CURSOR:
        {
            HDC hdcMemCursor = CreateCompatibleDC(rGfx.GetHDC());

            if(m_bUse3P)
            {
                RECT r = rBoundary;
                int nCursorPos = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                    (float)rBoundary.left, (float)(rBoundary.right - m_nCursorWidth),
                    m_fPos);
                r.left = nCursorPos;
                r.right = nCursorPos + m_nCursorWidth;

                int midstart, midwidth;
                midstart = r.left+m_3PCursorLeftWidth;
                midwidth = m_3PCursorMidWidth;
                
                //Left
                hbmOld = (HBITMAP)SelectObject(hdcMemCursor, m_h3PCursorLeft);

                AlphaBlend(rGfx.GetHDC(), r.left, 0, m_3PCursorLeftWidth, GetHeight(), 
                    hdcMemCursor, 0, 0, m_3PCursorLeftWidth, m_3PCursorLeftHeight, opblender);

                //Mid
                SelectObject(hdcMemCursor, m_h3PCursorMid);

                AlphaBlend(rGfx.GetHDC(), midstart, 0, midwidth, GetHeight(), 
                    hdcMemCursor, 0, 0, m_3PCursorMidWidth, m_3PCursorMidHeight, opblender);

                //Right                
                SelectObject(hdcMemCursor, m_h3PCursorRight);

                AlphaBlend(rGfx.GetHDC(), midstart+midwidth, 0, m_3PCursorRightWidth, GetHeight(), 
                    hdcMemCursor, 0, 0, m_3PCursorRightWidth, m_3PCursorRightHeight, opblender);

                // restore old bitmap
                SelectObject(hdcMemCursor, hbmOld);
            }
            else
            {
                RECT r = rBoundary;
                int nCursorPos = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                    (float)rBoundary.left, (float)(rBoundary.right - m_nCursorWidth),
                    m_fPos);
                r.left = nCursorPos;
                r.right = nCursorPos + m_nCursorWidth;
                
                HBITMAP hbmOldCursor = (HBITMAP)SelectObject(hdcMemCursor, m_hCursor);

                BitBlt(rGfx.GetHDC(), r.left, 0, m_nCursorWidth, GetHeight(), hdcMemCursor, 0, 0, SRCCOPY);

                SelectObject(hdcMemCursor, hbmOldCursor);
            }

            DeleteDC(hdcMemCursor);

            break;
        }
    default:
        break;
    }

    if( NULL != m_hHighlight )
    {
        HDC hdcMemHighlight = CreateCompatibleDC(rGfx.GetHDC());
        HBITMAP hbmOldHighlight = (HBITMAP)SelectObject(hdcMemHighlight, m_hHighlight);

        AlphaBlend(rGfx.GetHDC(), 0, 0, GetWidth(), GetHeight(), hdcMemHighlight, 0, 0, m_HighlightWidth, m_HighlightHeight, opblender);

        SelectObject(hdcMemHighlight, hbmOldHighlight);
        DeleteDC(hdcMemHighlight);
    }
}


//** end of LCDSkinnedProgressBar.cpp ************************************
