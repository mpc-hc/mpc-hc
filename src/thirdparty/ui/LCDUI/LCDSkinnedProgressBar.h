//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDSkinnedProgressBar.h
//
// The CLCDSkinnedProgressBar class draws a progress bar onto the color LCD.
// Unlike the other progress bars, this one uses a bitmap resource to draw
// the progress bar. This scrollbar cannot be scaled.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef __LCDSKINNEDPROGRESSBAR_H__
#define __LCDSKINNEDPROGRESSBAR_H__

#include "LCDProgressBar.h"

class CLCDSkinnedProgressBar : public CLCDProgressBar
{
public:
    CLCDSkinnedProgressBar(void);
    virtual ~CLCDSkinnedProgressBar(void);

    //See example bitmaps for how to make the images
    void SetBackground(HBITMAP background, int bmpWidth, int bmpHeight);
    void SetFiller(HBITMAP cursor, int bmpWidth, int bmpHeight); //STYLE_FILLED
    void SetCursor(HBITMAP cursor, int bmpWidth, int bmpHeight); //STYLE_CURSOR
    void SetThreePieceCursor(HBITMAP left, int bmpLeftWidth, int bmpLeftHeight,
                             HBITMAP mid, int bmpMidWidth, int bmpMidHeight,
                             HBITMAP right, int bmpRightWidth, int bmpRightHeight );

    //optional -- draw a highlight on top of the progress bar
    //(for example, something like a glass effect)
    void AddHighlight(HBITMAP highlight, int bmpWidth, int bmpHeight); 

    //CLCDBase
    virtual void OnDraw(CLCDGfxBase &rGfx);

private:
    HBITMAP m_hBackground;
    int m_BackgroundHeight;
    int m_BackgroundWidth;

    HBITMAP m_hFiller;
    int m_FillerHeight;
    int m_FillerWidth;

    HBITMAP m_hCursor;
    int m_CursorHeight;
    int m_CursorWidth;

    HBITMAP m_hHighlight;
    int m_HighlightHeight;
    int m_HighlightWidth;

    //3 piece bitmaps, if used
    BOOL m_bUse3P;

    HBITMAP m_h3PCursorLeft;
    int m_3PCursorLeftHeight;
    int m_3PCursorLeftWidth;

    HBITMAP m_h3PCursorMid;
    int m_3PCursorMidHeight;
    int m_3PCursorMidWidth;

    HBITMAP m_h3PCursorRight;
    int m_3PCursorRightHeight;
    int m_3PCursorRightWidth;    
};

#endif

//** end of LCDSkinnedProgressBar.h **************************************
