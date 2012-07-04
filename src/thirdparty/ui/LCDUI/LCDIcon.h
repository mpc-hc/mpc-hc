//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDIcon.h
//
// The CLCDIcon class draws icons onto the lcd.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDICON_H_INCLUDED_ 
#define _LCDICON_H_INCLUDED_ 

#include "LCDBase.h"

class CLCDIcon : public CLCDBase
{
public:
    CLCDIcon(void);
    virtual ~CLCDIcon(void);

    void SetIcon(HICON hIcon, int nWidth = 16, int nHeight = 16);
    
    HICON GetIcon(void)
    {
        return m_hIcon;
    }

    // CLCDBase
    virtual void OnDraw(CLCDGfxBase &rGfx);

private:
    HICON m_hIcon;
    int m_nIconWidth;
    int m_nIconHeight;
};


#endif // !_LCDICON_H_INCLUDED_ 

//** end of LCDIcon.h ****************************************************
