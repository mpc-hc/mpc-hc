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

#ifndef _LCDPAGINATETEXT_H_INCLUDED_ 
#define _LCDPAGINATETEXT_H_INCLUDED_ 

#include "LCDText.h"

class CLCDPaginateText: public CLCDText
{

public:
    CLCDPaginateText(void);
    virtual ~CLCDPaginateText(void);

    // CLCDText
    virtual void SetSize(SIZE& size);
    virtual void SetSize(int nCX, int nCY);
    virtual void OnDraw(CLCDGfxBase &rGfx);

    // force the re-pagination, will set the first page to be the current page
    void    DoPaginate(void);
    int     GetTotalPages(void);
    int     GetCurPage(void);
    void    SetCurPage(int iPageNum);
    // go to previous page, return false if it is already at the first page. Otherwise, return true;
    bool    GotoPrevPage(void);
    // go to next page, return false if it is already at the last page. Otherwise, return true;
    bool    GotoNextPage(void);

private:
    // this is the original size for the control. As we try to squeeze text into
    // the rectangle, we squeeze the m_Size to show unclipped text. but if the 
    // user change settings where a recalc is needed to figure out the total page,
    // we need the original size for that calculation
    SIZE    m_origSize;

    // using the current setting, number of lines fit to one page (w/o clipping)
    int     m_linePerPage;
    int     m_totalPageNum;
    int     m_iCurPageNum;
};


#endif // !_LCDPAGINATETEXT_H_INCLUDED_ 

//** end of LCDPaginateText.h *******************************************
