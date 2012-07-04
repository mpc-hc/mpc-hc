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
// LCDGfxMono.h
//
// This Gfx object handles drawing to a 160x43 monochrome display.
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef __LCDGFXMONO_H__
#define __LCDGFXMONO_H__

#include "LCDGfxBase.h"

class CLCDGfxMono : public CLCDGfxBase
{
public:
    CLCDGfxMono(void);
    virtual ~CLCDGfxMono(void);

    virtual HRESULT Initialize(void);
    virtual lgLcdBitmap *GetLCDScreen(void);
    virtual void ClearScreen(void);

    virtual DWORD GetFamily(void)
    {
        return LGLCD_DEVICE_FAMILY_KEYBOARD_G15;
    }

private:
    lgLcdBitmap160x43x1 m_LCDScreen;
};

#endif

//** end of LCDGfxMono.h *************************************************
