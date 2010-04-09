//************************************************************************
//
// LCDText.h
//
// The CLCDText class draws simple text onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDTEXT_H_INCLUDED_ 
#define _LCDTEXT_H_INCLUDED_ 

#include "LCDBase.h"

#include <string>
using namespace std;

class CLCDText : public CLCDBase
{

public:
    CLCDText();
    virtual ~CLCDText();

    virtual HRESULT Initialize(void);
    
    virtual void SetFont(LOGFONT& lf);
    virtual void SetFontFaceName(LPCTSTR szFontName);
    virtual void SetFontPointSize(int nPointSize);
    virtual void SetFontWeight(int nWeight);

    virtual HFONT GetFont();
    virtual void SetText(LPCTSTR szText);
    virtual LPCTSTR GetText();
    virtual void SetWordWrap(BOOL bEnable);
    virtual SIZE& GetVExtent();
    virtual SIZE& GetHExtent();
    virtual void SetLeftMargin(int nLeftMargin);
    virtual int GetLeftMargin(void);
    virtual void SetRightMargin(int nRightMargin);
    virtual int GetRightMargin(void);
    virtual void SetAlignment(int nAlignment = DT_LEFT);

    virtual void OnDraw(CLCDGfx &rGfx);

    enum { DEFAULT_DPI = 96, DEFAULT_POINTSIZE = 8 };

protected:
    void DrawText(CLCDGfx &rGfx);

#ifdef UNICODE
    std::wstring m_sText;
#else
    std::string m_sText;
#endif
    HFONT m_hFont;
    COLORREF m_crColor;
    basic_string <TCHAR>::size_type m_nTextLength;
    UINT m_nTextFormat;
    BOOL m_bRecalcExtent;
    DRAWTEXTPARAMS m_dtp;
    int m_nTextAlignment;
    SIZE m_sizeVExtent, m_sizeHExtent;
};


#endif // !_LCDTEXT_H_INCLUDED_ 

//** end of LCDText.h ****************************************************
