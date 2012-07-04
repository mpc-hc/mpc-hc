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
// LCDGfxColor.cpp
//
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDGfxColor::CLCDGfxColor
//
//************************************************************************

CLCDGfxColor::CLCDGfxColor(void)
{
    m_pLCDScreen = (lgLcdBitmap *) &m_LCDScreen;
    m_pLCDScreen->hdr.Format = LGLCD_BMP_FORMAT_QVGAx32;
}


//************************************************************************
//
// CLCDGfxColor::~CLCDGfxColor
//
//************************************************************************

CLCDGfxColor::~CLCDGfxColor(void)
{
}


//************************************************************************
//
// CLCDGfxColor::Initialize
//
//************************************************************************

HRESULT CLCDGfxColor::Initialize(void)
{
    //reset everything
    Shutdown();

    m_nWidth = LGLCD_QVGA_BMP_WIDTH;
    m_nHeight = LGLCD_QVGA_BMP_HEIGHT;

    HRESULT hRes = CLCDGfxBase::Initialize();
    if(FAILED(hRes))
    {
        return hRes;
    }

    hRes = CLCDGfxBase::CreateBitmap(32);
    if(FAILED(hRes))
    {
        return hRes;
    }

    return S_OK;
}


//************************************************************************
//
// CLCDGfxColor::GetLCDScreen
//
//************************************************************************

lgLcdBitmap* CLCDGfxColor::GetLCDScreen(void)
{
    LCDUIASSERT(m_pLCDScreen == (lgLcdBitmap *) &m_LCDScreen);
    m_LCDScreen.hdr.Format = LGLCD_BMP_FORMAT_QVGAx32;
    memcpy(m_LCDScreen.pixels, m_pBitmapBits, m_nWidth * m_nHeight * 4);
    return m_pLCDScreen;
}


//** end of LCDGfxColor.cpp **********************************************
