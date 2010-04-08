//************************************************************************
//
// LCDBitmap.h
//
// The CLCDBitmap class draws bitmaps onto the LCD.
//
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDBITMAP_H_INCLUDED_
#define _LCDBITMAP_H_INCLUDED_

#include "LCDBase.h"

class CLCDBitmap : public CLCDBase
{
public:
    CLCDBitmap();
    virtual ~CLCDBitmap();

    void SetBitmap(HBITMAP hBitmap);
    void SetROP(DWORD dwROP);

protected:
    virtual void OnDraw(CLCDGfx &rGfx);
    HBITMAP m_hBitmap;
    DWORD m_dwROP;

private:
};


#endif // !_LCDBITMAP_H_INCLUDED_ 

//** end of LCDBitmap.h **************************************************
