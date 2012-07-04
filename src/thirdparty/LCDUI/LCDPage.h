//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDPage.h
//
// Collection of items representing a page.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDPAGE_H_INCLUDED_ 
#define _LCDPAGE_H_INCLUDED_ 

#include "LCDCollection.h"
#include "LCDBitmap.h"

class CLCDPage : public CLCDCollection
{
public:
    CLCDPage(void);
    virtual ~CLCDPage(void);

    virtual void SetExpiration(DWORD dwMilliseconds);
    virtual BOOL HasExpired(void);
    virtual void OnLCDButtonDown(int nButton);
    virtual void OnLCDButtonUp(int nButton);

    // CLCDCollection
    virtual void OnDraw(CLCDGfxBase &rGfx);
    virtual void OnUpdate(DWORD dwTimestamp);

    void SetBackground(HBITMAP hBitmap);
    void SetBackground(COLORREF Color);

private:
    DWORD m_dwStartTime;
    DWORD m_dwEllapsedTime;
    DWORD m_dwExpirationTime;

    //Background data
    BOOL m_bUseBitmapBackground;
    CLCDBitmap m_Background;
    BOOL m_bUseColorBackground;
    COLORREF m_BackgroundColor;
};

#endif

//** end of LCDPage.h ****************************************************
