//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDPopup.h
//
// Color LCD Popup class
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDPOPUP_H_INCLUDED_ 
#define _LCDPOPUP_H_INCLUDED_ 

#include "LCDPage.h"
#include "LCDText.h"
#include "LCDBitmap.h"
#include <gdiplus.h>


//************************************************************************
//
// CLCDPopupBackground
//
//************************************************************************

class CLCDPopupBackground : public CLCDBase
{
public:
    CLCDPopupBackground(void);
    virtual ~CLCDPopupBackground(void);

    void SetAlphaLevel(BYTE cAlphaStart, BYTE cAlphaEnd = 0xff);
    void SetGradientMode(BOOL bGradient);
    void SetColor(COLORREF cfColor);
    void SetRoundedRecteRadius(int nRadius);

public:
    virtual void OnDraw(CLCDGfxBase &rGfx);
    virtual void SetSize(int nCX, int nCY);

private:
    void RecalcRoundedRectangle(void);
    BYTE m_cAlphaStart, m_cAlphaEnd;
    COLORREF m_cfColor;
    BOOL m_bUseGradient;
    int m_nRectRadius;
    Gdiplus::GraphicsPath* m_pGraphicsPath;
};



//************************************************************************
//
// CLCDPopup
//
//************************************************************************

class CLCDPopup : public CLCDPage
{
public:
    enum PB_TYPE { PB_TEXT, PB_OK, PB_OKCANCEL, PB_OKCANCEL_TEXT };

public:
    CLCDPopup(void);

    virtual HRESULT Initialize(int nMaxPopupWidth = 0);

    void SetText(LPCTSTR szMessage, LPCTSTR szOK = NULL, LPCTSTR szCancel = NULL);
    void SetPopupType(PB_TYPE pbType);
    void SetBitmaps(HBITMAP hbmOK, HBITMAP hbmCancel = NULL);
    void SetAlpha(BYTE cAlphaStart, BYTE cAlphaEnd = 0xff);
    void SetGradientMode(BOOL bGradient);
    void SetColor(COLORREF cfColor);

protected:
    void RecalcLayout(void);
    // Resizes automatically, so this is now private
    virtual void SetSize(int nCX, int nCY);

    CLCDText m_MessageText, m_OKText, m_CancelText;
    CLCDPopupBackground m_Background;
    int m_nMaxPopupWidth;
private:

    CLCDBitmap m_OKBitmap, m_CancelBitmap;
    PB_TYPE m_pbType;
};

#endif

//** end of LCDPopup.h ***************************************************
