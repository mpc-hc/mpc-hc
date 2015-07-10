//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDStreamingText.h
//
// The CLCDStreamingText class draws streaming text onto the LCD.
// Streaming text is a single line of text that is repeatedly streamed
// horizontally across the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDSTREAMINGTEXT_H_INCLUDED_ 
#define _LCDSTREAMINGTEXT_H_INCLUDED_ 

#include "LCDBase.h"
#include "LCDCollection.h"
#include "LCDText.h"

#include <string>

class CLCDStreamingText: public CLCDCollection
{

public:
    CLCDStreamingText();
    virtual ~CLCDStreamingText();

    // CLCDBase
    virtual HRESULT Initialize(void);
    virtual void ResetUpdate(void);
    virtual void Show(BOOL bShow);
    virtual void SetOrigin(POINT pt);
    virtual void SetOrigin(int nX, int nY);
    virtual void SetSize(SIZE& size);
    virtual void SetSize(int nCX, int nCY);
    virtual void SetBackgroundMode(int nMode);
    virtual void SetForegroundColor(COLORREF crForeground);
    virtual void SetBackgroundColor(COLORREF crBackground);

    void SetText(LPCTSTR szText);
    void SetGapText(LPCTSTR szGapText);
    void SetStartDelay(DWORD dwMilliseconds);
    void SetSpeed(DWORD dwSpeed);
	void SetScrollingStep(DWORD dwStepInPixels);
    void SetAlignment(int nAlignment = DT_LEFT);
    void SetFont(LOGFONT& lf);
    void SetFontFaceName(LPCTSTR szFontName);
    void SetFontPointSize(int nSize);
    void SetFontWeight(int nPointSize);
    void SetFontColor(COLORREF color);
    HFONT GetFont(void);

    enum { DEFAULT_DPI = 96, DEFAULT_POINTSIZE = 8 };

protected:
    virtual void OnUpdate(DWORD dwTimestamp);
    virtual void OnDraw(CLCDGfxBase &rGfx);

private:
    int AddText(LPCTSTR szText);
    void RemoveText(int nIndex);
    void RemoveAllText();

private:
    BOOL RecalcTextBoxes(CLCDGfxBase &rGfx);
    void RecalcTextBoxOrigins();
    void ApplyOrigins(int nOffset);

    enum eSCROLL_STATES { STATE_DELAY, STATE_SCROLL};

    // ellapsed time in state
    DWORD m_dwEllapsedTime; 

    // milliseconds
    DWORD m_dwStartDelay;

    // pixels/second
    DWORD m_dwSpeed;

    // Number of pixels to shift
	DWORD m_dwStepInPixels;

    // milliseconds
    DWORD m_dwLastUpdate;   

    eSCROLL_STATES m_eState;
    BOOL m_bRecalcExtent;

    CLCDText *m_pQueueHead;

    COLORREF m_crColor;
    BOOL m_bRedoColors;

#ifdef UNICODE
    std::wstring m_sText;
    std::wstring m_sGapText;
#else
    std::string m_sText;
    std::string m_sGapText;
#endif

    HFONT m_hFont;
    int m_nTextAlignment;
    float m_fFractDistance;
};


#endif // !_LCDSTREAMINGTEXT_H_INCLUDED_ 

//** end of LCDStreamingText.h *******************************************
