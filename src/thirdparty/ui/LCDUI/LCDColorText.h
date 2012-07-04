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
// LCDColorText.h
//
// The CLCDText class draws simple text onto the LCD.
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDCOLORTEXT_H_INCLUDED_
#define _LCDCOLORTEXT_H_INCLUDED_

#include "LCDText.h"

class CLCDColorText : public CLCDText
{
public:
    CLCDColorText();
    virtual ~CLCDColorText();

    // CLCDBase
    virtual void OnDraw(CLCDGfxBase &rGfx);
    virtual void OnUpdate(DWORD timestamp);

    virtual void SetFontColor(COLORREF color);
    virtual void SetBackgroundMode(int nMode, COLORREF color=RGB(255,255,255));
    void SetScrollRate( int pixelspersec );
    void SetAutoScroll(bool b)
    { m_bAutoScroll = b; }

    enum { DEFAULT_DPI = 96, DEFAULT_POINTSIZE = 12 };

private:
    COLORREF m_backColor;

    //For scrolling
    DWORD m_RunningTime;
    int m_StartX;
    int m_LoopX;
    int m_ScrollRate; //pixels per second
    int m_PixelLength;
    int m_ScrollBuffer; //how many pixels between scrolls
    int m_JumpDistance;

    bool m_bAutoScroll; //automatically scroll if text length > draw area
};

#endif // !_LCDCOLORTEXT_H_INCLUDED_

//** end of LCDColorText.h ****************************************************
