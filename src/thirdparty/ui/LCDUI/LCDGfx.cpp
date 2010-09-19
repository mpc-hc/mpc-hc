//************************************************************************
//
// LCDGfx.cpp
//
// The CLCDGfx class abstracts GDI/bitmap details. It is used in the
// OnDraw event.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "../../../DSUtil/SharedInclude.h"
#include <Afx.h>
#include "LCDGfx.h"


//************************************************************************
//
// CLCDGfx::CLCDGfx
//
//************************************************************************

CLCDGfx::CLCDGfx(void)
:   m_nWidth(0),
    m_nHeight(0),
    m_pLCDScreen(NULL),
    m_pBitmapInfo(NULL),
    m_hDC(NULL),
    m_hBitmap(NULL),
    m_hPrevBitmap(NULL),
    m_pBitmapBits(NULL)
{
}


//************************************************************************
//
// CLCDGfx::~CLCDGfx
//
//************************************************************************

CLCDGfx::~CLCDGfx(void)
{
    Shutdown();
}


//************************************************************************
//
// CLCDGfx::Initialize
//
//************************************************************************

HRESULT CLCDGfx::Initialize(int nWidth, int nHeight)
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;

    m_hDC = CreateCompatibleDC(NULL);
    if(NULL == m_hDC)
    {
        LCDUITRACE(_T("CLCDGfx::Initialize(): failed to create compatible DC.\n"));
        Shutdown();
        return E_FAIL;
    }
    
    int nBMISize = sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD);
    m_pBitmapInfo = (BITMAPINFO *) DNew BYTE [nBMISize];
    if(NULL == m_pBitmapInfo)
    {
        LCDUITRACE(_T("CLCDGfx::Initialize(): failed to allocate bitmap info.\n"));
        Shutdown();
        return E_OUTOFMEMORY;
    }
    
    ZeroMemory(m_pBitmapInfo, nBMISize);
    m_pBitmapInfo->bmiHeader.biSize = sizeof(m_pBitmapInfo->bmiHeader);
    m_pBitmapInfo->bmiHeader.biWidth = m_nWidth;
    m_pBitmapInfo->bmiHeader.biHeight = -m_nHeight;
    m_pBitmapInfo->bmiHeader.biPlanes = 1;
    m_pBitmapInfo->bmiHeader.biBitCount = 8;
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
    
    m_hBitmap = CreateDIBSection(m_hDC, m_pBitmapInfo, DIB_RGB_COLORS, (PVOID *) &m_pBitmapBits, NULL, 0);
    if(NULL == m_hBitmap)
    {
        LCDUITRACE(_T("CLCDGfx::Initialize(): failed to create bitmap.\n"));
        Shutdown();
        return E_FAIL;
    }
    
    m_pLCDScreen = DNew lgLcdBitmap160x43x1;
    if(NULL == m_pLCDScreen)
    {
        LCDUITRACE(_T("CLCDGfx::Initialize(): failed to allocate the lcd screen structure.\n"));
        Shutdown();
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


//************************************************************************
//
// CLCDGfx::Shutdown
//
//************************************************************************

void CLCDGfx::Shutdown(void)
{
    if(NULL != m_pLCDScreen)
    {
        delete m_pLCDScreen;
        m_pLCDScreen = NULL;
    }
    
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
// CLCDGfx::BeginDraw
//
//************************************************************************

void CLCDGfx::BeginDraw(void)
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
// CLCDGfx::ClearScreen
//
//************************************************************************

void CLCDGfx::ClearScreen(void)
{
    // this means, we're inside BeginDraw()/EndDraw()
    LCDUIASSERT(NULL != m_hPrevBitmap);
    RECT rc = { 0, 0, m_nWidth, m_nHeight };
    FillRect(m_hDC, &rc, (HBRUSH) GetStockObject(BLACK_BRUSH));
}


//************************************************************************
//
// CLCDGfx::SetPixel
//
//************************************************************************

void CLCDGfx::SetPixel(int nX, int nY, BYTE bValue)
{
    // this means, we're inside BeginDraw()/EndDraw()
    LCDUIASSERT(NULL != m_hPrevBitmap);
    ::SetPixel(m_hDC, nX, nY, bValue ? RGB(255, 255, 255) : RGB(0, 0, 0));
}


//************************************************************************
//
// CLCDGfx::DrawLine
//
//************************************************************************

void CLCDGfx::DrawLine(int nX1, int nY1, int nX2, int nY2)
{
    // this means, we're inside BeginDraw()/EndDraw()
    LCDUIASSERT(NULL != m_hPrevBitmap);

    HPEN hPrevPen = (HPEN) SelectObject(m_hDC, GetStockObject(WHITE_PEN));
    ::MoveToEx(m_hDC, nX1, nY1, NULL);
    ::LineTo(m_hDC, nX2, nY2);
    SelectObject(m_hDC, hPrevPen);
}


//************************************************************************
//
// CLCDGfx::DrawFilledRect
//
//************************************************************************

void CLCDGfx::DrawFilledRect(int nX, int nY, int nWidth, int nHeight)
{
    // this means, we're inside BeginDraw()/EndDraw()
    LCDUIASSERT(NULL != m_hPrevBitmap);

    HBRUSH hPrevBrush = (HBRUSH) SelectObject(m_hDC, GetStockObject(WHITE_BRUSH));
    RECT r = { nX, nY, nX + nWidth, nY + nHeight };
    ::FillRect(m_hDC, &r, hPrevBrush);
    SelectObject(m_hDC, hPrevBrush);
}


//************************************************************************
//
// CLCDGfx::DrawText
//
//************************************************************************

void CLCDGfx::DrawText(int nX, int nY, LPCTSTR sText)
{
    // map mode text, with transparency
    int nOldMapMode = SetMapMode(m_hDC, MM_TEXT);
    int nOldBkMode = SetBkMode(m_hDC, TRANSPARENT); 
    
    ::TextOut(m_hDC, nX, nY, sText, (int)_tcslen(sText));
    
    // restores
    SetMapMode(m_hDC, nOldMapMode);
    SetBkMode(m_hDC, nOldBkMode);

}


//************************************************************************
//
// CLCDGfx::EndDraw
//
//************************************************************************

void CLCDGfx::EndDraw(void)
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
// CLCDGfx::GetHDC
//
//************************************************************************

HDC CLCDGfx::GetHDC(void)
{
    LCDUIASSERT(NULL != m_hDC);
    return m_hDC;
}


//************************************************************************
//
// CLCDGfx::GetLCDScreen
//
//************************************************************************

lgLcdBitmap160x43x1 *CLCDGfx::GetLCDScreen(void)
{
    LCDUIASSERT(NULL != m_pLCDScreen);
    if(NULL != m_pLCDScreen)
    {
        m_pLCDScreen->hdr.Format = LGLCD_BMP_FORMAT_160x43x1;
        memcpy(m_pLCDScreen->pixels, m_pBitmapBits, m_nWidth * m_nHeight);
    }
    
    return m_pLCDScreen;
}


//************************************************************************
//
// CLCDGfx::GetBitmapInfo
//
//************************************************************************

BITMAPINFO *CLCDGfx::GetBitmapInfo(void)
{
    LCDUIASSERT(NULL != m_pBitmapInfo);
    return m_pBitmapInfo;
}


//************************************************************************
//
// CLCDGfx::GetHBITMAP
//
//************************************************************************

HBITMAP CLCDGfx::GetHBITMAP(void)
{
    LCDUIASSERT(NULL != m_hBitmap);
    return m_hBitmap;
}


//** end of LCDGfx.cpp ***************************************************
