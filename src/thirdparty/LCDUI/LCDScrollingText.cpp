//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDScrollingText.cpp
//
// The CLCDScrollingText class draws scrolling text onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDScrollingText::CLCDScrollingText
//
//************************************************************************

CLCDScrollingText::CLCDScrollingText(void)
{
    m_eState = STATE_START_DELAY;
    m_eScrollDir = SCROLL_HORZ;
    m_bRepeat = TRUE;
}


//************************************************************************
//
// CLCDScrollingText::~CLCDScrollingText
//
//************************************************************************

CLCDScrollingText::~CLCDScrollingText(void)
{
}


//************************************************************************
//
// CLCDScrollingText::Initialize
//
//************************************************************************

HRESULT CLCDScrollingText::Initialize(void)
{
    m_dwStartDelay  = 1000;
    m_dwSpeed       = 20;
    m_nScrollingDistance = -1;
    m_dwLastUpdate = 0;
    m_dwEllapsedTime = 0;
    m_dwLastUpdate = GetTickCount();
    m_fTotalDistance = 0;
    m_eScrollDir = SCROLL_HORZ;
    m_dwEndDelay = 1000;
    m_bRepeat = TRUE;

    return CLCDText::Initialize();
}


//************************************************************************
//
// CLCDScrollingText::ResetUpdate
//
//************************************************************************

void CLCDScrollingText::ResetUpdate(void)
{
    m_eState = STATE_START_DELAY;
    m_dwEllapsedTime = 0;
    m_dwLastUpdate = GetTickCount();
    m_nScrollingDistance = -1;
    m_fTotalDistance = 0;
    SetLeftMargin(0);
    SetLogicalOrigin(0, 0);
    
    CLCDText::ResetUpdate();
}


//************************************************************************
//
// CLCDScrollingText::SetStartDelay
//
//************************************************************************

void CLCDScrollingText::SetStartDelay(DWORD dwMilliseconds)
{
    m_dwStartDelay = dwMilliseconds;
}


//************************************************************************
//
// CLCDScrollingText::SetEndDelay
//
//************************************************************************

void CLCDScrollingText::SetEndDelay(DWORD dwMilliseconds)
{
    m_dwEndDelay = dwMilliseconds;
}


//************************************************************************
//
// CLCDScrollingText::SetSpeed
//
//************************************************************************

void CLCDScrollingText::SetSpeed(DWORD dwSpeed)
{
    m_dwSpeed = dwSpeed;
}


//************************************************************************
//
// CLCDScrollingText::SetScrollDirection
//
//************************************************************************

void CLCDScrollingText::SetScrollDirection(eSCROLL_DIR eScrollDir)
{
    m_eScrollDir = eScrollDir;
    SetWordWrap(eScrollDir == SCROLL_VERT);
    ResetUpdate();
}


//************************************************************************
//
// CLCDScrollingText::GetScrollDirection
//
//************************************************************************

eSCROLL_DIR CLCDScrollingText::GetScrollDirection()
{
    return m_eScrollDir;
}


//************************************************************************
//
// CLCDScrollingText::SetText
//
//************************************************************************

void CLCDScrollingText::SetText(LPCTSTR szText)
{
    if (_tcscmp(szText, m_sText.c_str()))
    {
        ResetUpdate();
    }

    CLCDText::SetText(szText);
}


//************************************************************************
//
// CLCDScrollingText::IsScrollingDone
//
//************************************************************************

BOOL CLCDScrollingText::IsScrollingDone()
{
    return (STATE_DONE == m_eState);
}


//************************************************************************
//
// CLCDScrollingText::EnableRepeat
//
//************************************************************************

void CLCDScrollingText::EnableRepeat(BOOL bEnable)
{
    m_bRepeat = bEnable;
}


//************************************************************************
//
// CLCDScrollingText::OnUpdate
//
//************************************************************************

void CLCDScrollingText::OnUpdate(DWORD dwTimestamp)
{
    m_dwEllapsedTime = (dwTimestamp - m_dwLastUpdate);
}


//************************************************************************
//
// CLCDScrollingText::OnDraw
//
//************************************************************************

void CLCDScrollingText::OnDraw(CLCDGfxBase &rGfx)
{
    if (!m_nTextLength)
    {
        return;
    }

    // calculate the scrolling distance
    if (-1 == m_nScrollingDistance)
    {
        CLCDText::OnDraw(rGfx);

        if (SCROLL_VERT == m_eScrollDir)
        { 
            // determine how far we have to travel until scrolling stops
            m_nScrollingDistance = ((GetHeight()) >= GetVExtent().cy) ?
                0 : (GetVExtent().cy - GetHeight());
            SetLogicalSize(GetVExtent().cx, GetVExtent().cy);
        }
        else
        {
            // determine how far we have to travel until scrolling stops
            m_nScrollingDistance = ((GetWidth()) >= GetHExtent().cx) ?
                0 : (GetHExtent().cx - GetWidth());
            SetLogicalSize(max(GetSize().cx, GetHExtent().cx), GetHExtent().cy);
        }
    }

    switch(m_eState)
    {
    case STATE_START_DELAY:
        if (m_dwEllapsedTime > m_dwStartDelay)
        {
            m_eState = STATE_SCROLL;
            m_dwEllapsedTime = 0;
            m_dwLastUpdate = GetTickCount();
        }
        break;

    case STATE_END_DELAY:
        if (m_dwEllapsedTime > m_dwEndDelay)
        {
            if (m_bRepeat)
            {
                ResetUpdate();
                break;
            }
            m_dwEllapsedTime = 0;
            m_dwLastUpdate = GetTickCount();
            m_eState = STATE_DONE;
        }
        break;

    case STATE_SCROLL:
        {
            // TODO: add some anti-aliasing on the movement

            // how much time has ellapsed?
            // given the speed, what is the total displacement?
            float fDistance = (float)(m_dwSpeed * m_dwEllapsedTime) / 1000.0f;
            m_fTotalDistance += fDistance;

            // we dont want the total distnace exceed our scrolling distance
            int nTotalOffset = min((int)m_fTotalDistance, m_nScrollingDistance);
            
            if (SCROLL_VERT == m_eScrollDir)
            {
                SetLogicalOrigin(GetLogicalOrigin().x, -1 * nTotalOffset);
            }
            else
            {
                SetLogicalOrigin(-1 * nTotalOffset, GetLogicalOrigin().y);
            }
            
            m_dwLastUpdate = GetTickCount();

            if (nTotalOffset == m_nScrollingDistance)
            {
                m_eState = STATE_END_DELAY;
            }
        }
        break;

    case STATE_DONE:
        break;

    default:
        break;
    }

    CLCDText::OnDraw(rGfx);
}


//** end of LCDScrollingText.cpp *****************************************
