//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDPage.cpp
//
// Collection of items representing a page.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDPage::CLCDPage
//
//************************************************************************

CLCDPage::CLCDPage(void)
:   m_dwStartTime(0),
    m_dwEllapsedTime(0),
    m_dwExpirationTime(0)
{
    SetExpiration(INFINITE);
    m_bUseBitmapBackground = FALSE;
    m_bUseColorBackground = FALSE;
}


//************************************************************************
//
// CLCDPage::~CLCDPage
//
//************************************************************************

CLCDPage::~CLCDPage(void)
{
}


//************************************************************************
//
// CLCDPage::OnDraw
//
//************************************************************************

void CLCDPage::OnDraw(CLCDGfxBase &rGfx)
{
    if(!IsVisible())
    {
        return;
    }

    //Draw the background first
    if(m_bUseBitmapBackground)
    {
        m_Background.OnDraw(rGfx);
    }
    else if(m_bUseColorBackground)
    {
        HBRUSH hBackBrush = CreateSolidBrush(m_BackgroundColor);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(rGfx.GetHDC(), hBackBrush);
        Rectangle(rGfx.GetHDC(), 0, 0, GetWidth(), GetHeight());
        SelectObject(rGfx.GetHDC(), hOldBrush);
        DeleteObject(hBackBrush);
    }


    //iterate through your objects and draw them
    LCD_OBJECT_LIST::iterator it = m_Objects.begin();
    while(it != m_Objects.end())
    {
        CLCDBase *pObject = *it++;
        LCDUIASSERT(NULL != pObject);

        if (pObject->IsVisible())
        {
            // create the clip region
            // Note that pages can now be added to pages (GetOrigin of the page is now factored in)
            HRGN hRgn = CreateRectRgn(GetOrigin().x + pObject->GetOrigin().x, GetOrigin().y + pObject->GetOrigin().y,
                GetOrigin().x + pObject->GetOrigin().x + pObject->GetWidth(),
                GetOrigin().y + pObject->GetOrigin().y + pObject->GetHeight());

            // ensure that controls only draw within their specified region
            SelectClipRgn(rGfx.GetHDC(), hRgn);

            // free the region (a copy is used in the call above)
            DeleteObject(hRgn);

            // offset the control at its origin so controls use (0,0)
            POINT ptPrevViewportOrg = { 0, 0 };
            SetViewportOrgEx(rGfx.GetHDC(),
                GetOrigin().x + pObject->GetOrigin().x,
                GetOrigin().y + pObject->GetOrigin().y,
                &ptPrevViewportOrg);

            // allow controls to supply additional translation
            // this allows controls to move freely within the confined viewport
            OffsetViewportOrgEx(rGfx.GetHDC(),
                pObject->GetLogicalOrigin().x,
                pObject->GetLogicalOrigin().y,
                NULL);

            pObject->OnDraw(rGfx);

            // set the clipping region to nothing
            SelectClipRgn(rGfx.GetHDC(), NULL);

            // restore the viewport origin
            SetViewportOrgEx(rGfx.GetHDC(),
                ptPrevViewportOrg.x,
                ptPrevViewportOrg.y,
                NULL);

            // restore the viewport origin offset
            OffsetViewportOrgEx(rGfx.GetHDC(), 0, 0, NULL);
        }
    }
}


//************************************************************************
//
// CLCDPage::OnUpdate
//
//************************************************************************

void CLCDPage::OnUpdate(DWORD dwTimestamp)
{
    m_dwEllapsedTime = (dwTimestamp - m_dwStartTime);

    CLCDCollection::OnUpdate(dwTimestamp);
}


//************************************************************************
//
// CLCDPage::OnLCDButtonDown
//
//************************************************************************

void CLCDPage::OnLCDButtonDown(int nButton)
{
    UNREFERENCED_PARAMETER(nButton);
}


//************************************************************************
//
// CLCDPage::OnLCDButtonUp
//
//************************************************************************

void CLCDPage::OnLCDButtonUp(int nButton)
{
    UNREFERENCED_PARAMETER(nButton);
}


//************************************************************************
//
// CLCDPage::SetExpiration
//
//************************************************************************

void CLCDPage::SetExpiration(DWORD dwMilliseconds)
{
    m_dwStartTime = GetTickCount();
    m_dwEllapsedTime = 0;
    m_dwExpirationTime = dwMilliseconds;
}


//************************************************************************
//
// CLCDPage::HasExpired
//
//************************************************************************

BOOL CLCDPage::HasExpired(void)
{
    return (!m_dwStartTime || !m_dwExpirationTime || (m_dwEllapsedTime >= m_dwExpirationTime));
}


//************************************************************************
//
// CLCDPage::SetBackground (using a bitmap)
//
//************************************************************************

void CLCDPage::SetBackground(HBITMAP hBitmap)
{
    m_Background.SetBitmap(hBitmap);
    m_Background.SetSize(320, 240);
    m_bUseBitmapBackground = TRUE;
    m_bUseColorBackground = FALSE;
}


//************************************************************************
//
// CLCDPage::SetBackground (using a color)
//
//************************************************************************

void CLCDPage::SetBackground(COLORREF Color)
{
    m_BackgroundColor = Color;
    m_bUseColorBackground = TRUE;
    m_bUseBitmapBackground = FALSE;
}

//** end of LCDPage.cpp **************************************************
