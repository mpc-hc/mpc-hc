//************************************************************************
//
// LCDAnimatedBitmap.h
//
// The CLCDAnimatedBitmap class draws animated bitmaps onto the LCD.
// An animated bitmap consists of a tiled bitmap representing the
// animation. The tile size is set with the SetSubpicWidth.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDANIMATEDBITMAP_H_INCLUDED_ 
#define _LCDANIMATEDBITMAP_H_INCLUDED_ 

#include "LCDBase.h"
#include "LCDBitmap.h"

class CLCDAnimatedBitmap : public CLCDBitmap
{
public:
    CLCDAnimatedBitmap();
    virtual ~CLCDAnimatedBitmap();

    virtual HRESULT Initialize(void);
    virtual void ResetUpdate(void);

    void SetSubpicWidth(DWORD dwWidth);
    void SetAnimationRate(DWORD dwRate);    // milliseconds/subpicture

protected:
    virtual void OnUpdate(DWORD dwTimestamp);
    virtual void OnDraw(CLCDGfx &rGfx);

private:
    DWORD m_dwElapsedTime;  // elapsed time in state
    DWORD m_dwRate;         // milliseconds per subpicture
    DWORD m_dwLastUpdate;   // milliseconds

    DWORD m_dwSubpicWidth;
    DWORD m_dwCurrSubpic;
    DWORD m_dwTotalSubpics;
};


#endif // !_LCDANIMATEDBITMAP_H_INCLUDED_ 

//** end of LCDBitmap.h **************************************************
