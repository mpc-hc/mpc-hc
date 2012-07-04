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
// LCDGfxBase.h
//
// Abstract class for different display types.
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef __LCDGFXBASE_H__
#define __LCDGFXBASE_H__

#include <lglcd/lglcd.h>


#if _MSC_VER >= 1400  
#define LCDUI_tcsncpy(x, y, z)       _tcsncpy_s(x, _countof(x), y, z)
#define LCDUI_tcscpy(x, y)           _tcscpy_s(x, _countof(x), y)
#else
#define LCDUI_tcsncpy(x, y, z)       _tcsncpy(x, y, z)
#define LCDUI_tcscpy(x, y)           _tcscpy(x, y)
#endif

#ifndef LCDUITRACE
    // .NET compiler uses __noop intrinsic
    #if _MSC_VER > 1300
        #define LCDUITRACE __noop
    #else
        #define LCDUITRACE (void)0
    #endif
#endif

#ifndef LCDUIASSERT
    // .NET compiler uses __noop intrinsic
    #if _MSC_VER > 1300
        #define LCDUIASSERT __noop
    #else
        #define LCDUIASSERT (void)0
    #endif
#endif


class CLCDGfxBase
{
public:
    CLCDGfxBase(void);
    virtual ~CLCDGfxBase(void);

    virtual HRESULT Initialize(void);
    virtual void Shutdown(void);
    virtual void ClearScreen(void);
    virtual void BeginDraw(void);
    virtual void EndDraw(void);

    virtual HDC GetHDC(void);
    virtual lgLcdBitmap *GetLCDScreen(void);
    virtual BITMAPINFO *GetBitmapInfo(void);
    virtual HBITMAP GetHBITMAP(void);

    virtual DWORD GetFamily(void) = 0;

    virtual int GetWidth(void);
    virtual int GetHeight(void);

protected:
    HRESULT CreateBitmap(WORD wBitCount);

protected:
    lgLcdBitmap *m_pLCDScreen;
    int m_nWidth;
    int m_nHeight;
    BITMAPINFO *m_pBitmapInfo;
    HDC m_hDC;
    HBITMAP m_hBitmap;
    HBITMAP m_hPrevBitmap;
    PBYTE m_pBitmapBits;

};

#endif

//** end of LCDGfxBase.h *************************************************
