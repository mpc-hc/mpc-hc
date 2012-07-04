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
// LCDGfxBase.cpp
//
//
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDGfxBase::CLCDGfxBase
//
//************************************************************************

CLCDGfxBase::CLCDGfxBase(void)
:   m_pLCDScreen(NULL),
    m_nWidth(0),
    m_nHeight(0),
    m_pBitmapInfo(NULL),
    m_hDC(NULL),
    m_hBitmap(NULL),
    m_hPrevBitmap(NULL),
    m_pBitmapBits(NULL)
{
}


//************************************************************************
//
// CLCDGfxBase::~CLCDGfxBase
//
//************************************************************************

CLCDGfxBase::~CLCDGfxBase(void)
{
    Shutdown();
}


//************************************************************************
//
// CLCDGfxBase::Initialize
//
//************************************************************************

HRESULT CLCDGfxBase::Initialize(void)
{
    m_hDC = CreateCompatibleDC(NULL);
    if(NULL == m_hDC)
    {
        LCDUITRACE(_T("CLCDGfxBase::Initialize(): failed to create compatible DC.\n"));
        Shutdown();
        return E_FAIL;
    }

    return S_OK;
}


//************************************************************************
//
// CLCDGfxBase::Shutdown
//
//************************************************************************

void CLCDGfxBase::Shutdown(void)
{
    if(NULL != m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
        m_pBitmapBits = NULL;
    }

    LCDUIASSERT(NULL == m_hPrevBitmap);
    m_hPrevBitmap = NULL;

    if(NULL != m_pBitmapInfo)
    {
        delete [] m_pBitmapInfo;
        m_pBitmapInfo = NULL;
    }

    if(NULL != m_hDC)
    {
        DeleteDC(m_hDC);
        m_hDC = NULL;
    }

    m_nWidth = 0;
    m_nHeight = 0;
}


//************************************************************************
//
// CLCDGfxBase::ClearScreen
//
//************************************************************************

void CLCDGfxBase::ClearScreen(void)
{
    // this means, we're inside BeginDraw()/EndDraw()
    LCDUIASSERT(NULL != m_hPrevBitmap);
    RECT rc = { 0, 0, m_nWidth, m_nHeight };
    FillRect(m_hDC, &rc, (HBRUSH) GetStockObject(BLACK_BRUSH));
}


//************************************************************************
//
// CLCDGfxBase::BeginDraw
//
//************************************************************************

void CLCDGfxBase::BeginDraw(void)
{
    LCDUIASSERT(NULL != m_hBitmap);
    LCDUIASSERT(NULL == m_hPrevBitmap);
    if(NULL == m_hPrevBitmap)
    {
        m_hPrevBitmap = (HBITMAP) SelectObject(m_hDC, m_hBitmap);
        SetTextColor(m_hDC, RGB(255, 255, 255));
        SetBkColor(m_hDC, RGB(0, 0, 0));
    }
}


//************************************************************************
//
// CLCDGfxBase::EndDraw
//
//************************************************************************

void CLCDGfxBase::EndDraw(void)
{
    LCDUIASSERT(NULL != m_hPrevBitmap);
    if(NULL != m_hPrevBitmap)
    {
        GdiFlush();
        m_hPrevBitmap = (HBITMAP) SelectObject(m_hDC, m_hPrevBitmap);
        LCDUIASSERT(m_hPrevBitmap == m_hBitmap);
        m_hPrevBitmap = NULL;
    }
}


//************************************************************************
//
// CLCDGfxBase::GetHDC
//
//************************************************************************

HDC CLCDGfxBase::GetHDC(void)
{
    LCDUIASSERT(NULL != m_hDC);
    return m_hDC;
}


//************************************************************************
//
// CLCDGfxBase::GetLCDScreen
//
//************************************************************************

lgLcdBitmap* CLCDGfxBase::GetLCDScreen(void)
{
    LCDUIASSERT(NULL != m_pLCDScreen);
    if(NULL != m_pLCDScreen)
    {
        // monochrome, as well as color pixels start at the same offset...
        memcpy(m_pLCDScreen->bmp_mono.pixels, m_pBitmapBits, m_nWidth * m_nHeight * 4);
    }

    return m_pLCDScreen;
}


//************************************************************************
//
// CLCDGfxBase::GetBitmapInfo
//
//************************************************************************

BITMAPINFO* CLCDGfxBase::GetBitmapInfo(void)
{
    LCDUIASSERT(NULL != m_pBitmapInfo);
    return m_pBitmapInfo;
}


//************************************************************************
//
// CLCDGfxBase::GetHBITMAP
//
//************************************************************************

HBITMAP CLCDGfxBase::GetHBITMAP(void)
{
    LCDUIASSERT(NULL != m_hBitmap);
    return m_hBitmap;
}


//************************************************************************
//
// CLCDGfxBase::GetWidth
//
//************************************************************************

int CLCDGfxBase::GetWidth(void)
{
    return m_nWidth;
}


//************************************************************************
//
// CLCDGfxBase::GetHeight
//
//************************************************************************

int CLCDGfxBase::GetHeight(void)
{
    return m_nHeight;
}


//************************************************************************
//
// CLCDGfxBase::CreateBitmap
//
//************************************************************************

HRESULT CLCDGfxBase::CreateBitmap(WORD wBitCount)
{
    int nBMISize = sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD);
    m_pBitmapInfo = (BITMAPINFO *) new BYTE [nBMISize];
    if(NULL == m_pBitmapInfo)
    {
        LCDUITRACE(_T("CLCDGfxBase::CreateBitmap(): failed to allocate bitmap info.\n"));
        Shutdown();
        return E_OUTOFMEMORY;
    }

    ZeroMemory(m_pBitmapInfo, nBMISize);
    m_pBitmapInfo->bmiHeader.biSize = sizeof(m_pBitmapInfo->bmiHeader);
    m_pBitmapInfo->bmiHeader.biWidth = m_nWidth;
    m_pBitmapInfo->bmiHeader.biHeight = -m_nHeight;
    m_pBitmapInfo->bmiHeader.biPlanes = 1;
    m_pBitmapInfo->bmiHeader.biBitCount = wBitCount;
    m_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
    m_pBitmapInfo->bmiHeader.biSizeImage = 
        (m_nWidth * 
        m_nHeight * 
        m_pBitmapInfo->bmiHeader.biBitCount) / 8;
    m_pBitmapInfo->bmiHeader.biXPelsPerMeter = 3200;
    m_pBitmapInfo->bmiHeader.biYPelsPerMeter = 3200;
    m_pBitmapInfo->bmiHeader.biClrUsed = 256;
    m_pBitmapInfo->bmiHeader.biClrImportant = 256;

    for(int nColor = 0; nColor < 256; ++nColor)
    {
        m_pBitmapInfo->bmiColors[nColor].rgbRed = (BYTE)((nColor > 128) ? 255 : 0);
        m_pBitmapInfo->bmiColors[nColor].rgbGreen = (BYTE)((nColor > 128) ? 255 : 0);
        m_pBitmapInfo->bmiColors[nColor].rgbBlue = (BYTE)((nColor > 128) ? 255 : 0);
        m_pBitmapInfo->bmiColors[nColor].rgbReserved = 0;
    }

    m_hBitmap = CreateDIBSection(m_hDC, m_pBitmapInfo, DIB_RGB_COLORS,
        (PVOID *) &m_pBitmapBits, NULL, 0);
    if(NULL == m_hBitmap)
    {
        LCDUITRACE(_T("CLCDGfxBase::CreateBitmap(): failed to create bitmap.\n"));
        Shutdown();
        return E_FAIL;
    }

    return S_OK;
}


//** end of LCDGfxBase.cpp ***********************************************
