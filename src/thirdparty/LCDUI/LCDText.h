//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDText.h
//
// The CLCDText class draws simple text onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDTEXT_H_INCLUDED_ 
#define _LCDTEXT_H_INCLUDED_ 


#include "LCDBase.h"
#include <string>

class CLCDText : public CLCDBase
{
public:
#ifdef UNICODE
    typedef std::wstring lcdstring;
#else
    typedef std::string lcdstring;
#endif

public:
    CLCDText(void);
    virtual ~CLCDText(void);

    virtual HRESULT Initialize(void);
    
    virtual void SetFont(LOGFONT& lf);
    virtual void SetFontFaceName(LPCTSTR szFontName);
    virtual void SetFontPointSize(int nPointSize);
    virtual void SetFontWeight(int nWeight);
    virtual void SetFontColor(COLORREF color);

    virtual HFONT GetFont(void);
    virtual void SetText(LPCTSTR szText);
    virtual LPCTSTR GetText(void);
    virtual void SetWordWrap(BOOL bEnable);
    virtual SIZE& GetVExtent(void);
    virtual SIZE& GetHExtent(void);
    virtual void CalculateExtent(BOOL bSingleLine);
    virtual void SetLeftMargin(int nLeftMargin);
    virtual int GetLeftMargin(void);
    virtual void SetRightMargin(int nRightMargin);
    virtual int GetRightMargin(void);
    virtual void SetAlignment(int nAlignment = DT_LEFT);

    // CLCDBase
    virtual void OnDraw(CLCDGfxBase &rGfx);

    enum { DEFAULT_DPI = 96, DEFAULT_POINTSIZE = 8 };

protected:
    void DrawText(CLCDGfxBase &rGfx);

    lcdstring m_sText;
    HFONT m_hFont;
    lcdstring::size_type m_nTextLength;
    UINT m_nTextFormat;
    BOOL m_bRecalcExtent;
    DRAWTEXTPARAMS m_dtp;
    int m_nTextAlignment;
    SIZE m_sizeVExtent, m_sizeHExtent;
};


#endif // !_LCDTEXT_H_INCLUDED_ 

//** end of LCDText.h ****************************************************
