//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDPaginateText.h
//
// The CLCDPaginateText class draws text onto the LCD in pages.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#include "LCDUI.h"


//************************************************************************
//
// CLCDPaginateText::CLCDPaginateText
//
//************************************************************************

CLCDPaginateText::CLCDPaginateText(void)
:   m_linePerPage(0),
    m_totalPageNum(0),
    m_iCurPageNum(0)
{
    ZeroMemory(&m_origSize, sizeof(m_origSize));
}


//************************************************************************
//
// CLCDPaginateText::~CLCDPaginateText
//
//************************************************************************

CLCDPaginateText::~CLCDPaginateText(void)
{
}


//************************************************************************
//
// CLCDPaginateText::SetSize
//
//************************************************************************

void CLCDPaginateText::SetSize(SIZE& size)
{
    CLCDBase::SetSize(size);
    m_origSize = size;
    m_bRecalcExtent = true;
}


//************************************************************************
//
// CLCDPaginateText::SetSize
//
//************************************************************************

void CLCDPaginateText::SetSize(int nCX, int nCY)
{
    CLCDBase::SetSize(nCX, nCY);
    m_origSize = m_Size;
    m_bRecalcExtent = true;
}


//************************************************************************
//
// CLCDPaginateText::DoPaginate
//
// Force the re-pagination, will set the first page to be the current page
//
//************************************************************************

void CLCDPaginateText::DoPaginate(void)
{
    // if m_bRecalcExtent is false, nothing affects pagination changes, don't do anything
    if(!m_bRecalcExtent)
    {
        return;
    }
    // if the page size is 0, set current page, total number of page, and line of text per page as 0
    if(m_origSize.cx == 0 || m_origSize.cy == 0)
    {
        m_iCurPageNum = 0;
        m_totalPageNum = 0;
        m_linePerPage = 0;
    }
    else
    {
        HDC hdc = CreateCompatibleDC(NULL);

        int nOldMapMode = SetMapMode(hdc, MM_TEXT);
        int nOldBkMode = SetBkMode(hdc, GetBackgroundMode()); 
        // select current font
        HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);   

        RECT rExtent;

        // calculate horizontal extent w/ single line, we can get the line height
        int nTextFormat = (m_nTextFormat | DT_SINGLELINE | DT_CALCRECT);
        rExtent.left = rExtent.top = 0;
        rExtent.right = m_origSize.cx;
        rExtent.bottom = m_origSize.cy;
        DrawTextEx(hdc, (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rExtent, nTextFormat, &m_dtp);
        m_sizeHExtent.cx = rExtent.right;
        m_sizeHExtent.cy = rExtent.bottom;

        // calculate vertical extent with word wrap
        nTextFormat = (m_nTextFormat | DT_WORDBREAK | DT_CALCRECT);
        rExtent.left = rExtent.top = 0;
        rExtent.right = m_origSize.cx;
        rExtent.bottom = m_origSize.cy;
        DrawTextEx(hdc, (LPTSTR)m_sText.c_str(), static_cast<int>(m_nTextLength), &rExtent, nTextFormat, &m_dtp);
        m_sizeVExtent.cx = rExtent.right;
        m_sizeVExtent.cy = rExtent.bottom;

        CLCDText::SetLogicalSize(m_sizeVExtent.cx, m_sizeVExtent.cy);

        // restores
        SetMapMode(hdc, nOldMapMode);
        SetBkMode(hdc, nOldBkMode);
        SelectObject(hdc, hOldFont);

        DeleteDC(hdc);

        m_linePerPage = (int)(m_origSize.cy / m_sizeHExtent.cy);
        // we re-set the m_Size.cy to show m_linePerPage line of text exactly (no clipping text)
        m_Size.cy = m_linePerPage * m_sizeHExtent.cy;

        // if the control size is too small to fit one line of text, the m_Size.cy is set to 0, 
        // and we also set total number of pages to be 0
        if(m_Size.cy == 0)
        {
            m_iCurPageNum = 0;
            m_totalPageNum = 0;
            m_linePerPage = 0;
        }
        else
        {
            m_totalPageNum = (int)(m_sizeVExtent.cy / m_Size.cy);
            m_totalPageNum += ((m_sizeVExtent.cy % m_Size.cy) > 0) ? 1 : 0;

            m_totalPageNum = (m_totalPageNum > 0) ? m_totalPageNum : 1;

            // after paginate, always set first page to be the current page
            m_iCurPageNum = 0;
        }
    }
    // set m_bRecalcExtent to be false to avoid another calculation at Draw time
    m_bRecalcExtent = false;
    SetLogicalOrigin(0, (int)((-1) * m_iCurPageNum * m_Size.cy));
}


//************************************************************************
//
// CLCDPaginateText::GetTotalPages
//
//************************************************************************

int CLCDPaginateText::GetTotalPages(void)
{
    DoPaginate();
    return m_totalPageNum;
}


//************************************************************************
//
// CLCDPaginateText::SetCurPage
//
//************************************************************************

void CLCDPaginateText::SetCurPage(int iPageNum)
{
    DoPaginate();

    if(iPageNum > m_totalPageNum - 1)
    {
        iPageNum = m_totalPageNum - 1;
    }
    if(iPageNum < 0) 
    {
        iPageNum = 0;
    }

    m_iCurPageNum = iPageNum;

    SetLogicalOrigin(0, (int)((-1) * m_iCurPageNum * m_Size.cy));
}


//************************************************************************
//
// CLCDPaginateText::GetCurPage
//
//************************************************************************

int CLCDPaginateText::GetCurPage(void)
{
    DoPaginate();
    return m_iCurPageNum;
}


//************************************************************************
//
// CLCDPaginateText::GotoPrevPage
//
// Go to previous page, return false if it is already at the first page.
// Otherwise, return true;
//
//************************************************************************

bool CLCDPaginateText::GotoPrevPage(void)
{
    DoPaginate();
    if(m_iCurPageNum > 0)
    {
        SetCurPage(--m_iCurPageNum);
        return true;
    }

    return false;
}


//************************************************************************
//
// CLCDPaginateText::GotoNextPage
//
// Go to next page, return false if it is already at the last page.
// Otherwise, return true;
//
//************************************************************************

bool CLCDPaginateText::GotoNextPage(void)
{
    DoPaginate();
    if(m_iCurPageNum < m_totalPageNum -1)
    {
        SetCurPage(++m_iCurPageNum);
        return true;
    }

    return false;
}

//************************************************************************
//
// CLCDPaginateText::OnDraw
//
// Before drawing, make sure we have a updated pagination
//
//************************************************************************

void CLCDPaginateText::OnDraw(CLCDGfxBase &rGfx)
{
    DoPaginate();
    CLCDText::OnDraw(rGfx);
}

//** end of LCDPaginateText.cpp ******************************************
