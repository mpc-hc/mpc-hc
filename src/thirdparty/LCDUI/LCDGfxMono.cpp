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
// LCDGfxMono.cpp
//
// This Gfx object handles drawing to a 160x43 monochrome display.
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDGfxMono::CLCDGfxMono
//
//************************************************************************

CLCDGfxMono::CLCDGfxMono(void)
{
    m_pLCDScreen = (lgLcdBitmap *) &m_LCDScreen;
    m_pLCDScreen->hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
}


//************************************************************************
//
// CLCDGfxMono::~CLCDGfxMono
//
//************************************************************************

CLCDGfxMono::~CLCDGfxMono(void)
{
}


//************************************************************************
//
// CLCDGfxMono::Initialize
//
//************************************************************************

HRESULT CLCDGfxMono::Initialize(void)
{
    //reset everything
    Shutdown();

    m_nWidth = LGLCD_BW_BMP_WIDTH;
    m_nHeight = LGLCD_BW_BMP_HEIGHT;

    HRESULT hRes = CLCDGfxBase::Initialize();
    if(FAILED(hRes))
    {
        return hRes;
    }

    hRes = CLCDGfxBase::CreateBitmap(8);
    if(FAILED(hRes))
    {
        return hRes;
    }

    return S_OK;
}


//************************************************************************
//
// CLCDGfxMono::GetLCDScreen
//
//************************************************************************

lgLcdBitmap* CLCDGfxMono::GetLCDScreen(void)
{
    LCDUIASSERT(m_pLCDScreen == (lgLcdBitmap *) &m_LCDScreen);
    m_LCDScreen.hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
    memcpy(m_LCDScreen.pixels, m_pBitmapBits, m_nWidth * m_nHeight);
    return m_pLCDScreen;
}


//************************************************************************
//
// CLCDGfxMono::ClearScreen
//
//************************************************************************

void CLCDGfxMono::ClearScreen(void)
{
    memset(m_pBitmapBits, 0, m_nWidth * m_nHeight);

    CLCDGfxBase::ClearScreen();
}

//** end of LCDGfxMono.cpp ***********************************************
