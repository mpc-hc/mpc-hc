//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDScrollingText.h
//
// The CLCDScrollingText class draws scrolling text onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************


#ifndef _LCDSCROLLINGTEXT_H_INCLUDED_ 
#define _LCDSCROLLINGTEXT_H_INCLUDED_ 

#include "LCDBase.h"
#include "LCDText.h"

enum eSCROLL_DIR { SCROLL_HORZ, SCROLL_VERT };

class CLCDScrollingText : public CLCDText
{
public:
    CLCDScrollingText();
    virtual ~CLCDScrollingText();

    // CLCDBase
    virtual HRESULT Initialize(void);
    virtual void ResetUpdate(void);
    
    // CLCDText
    virtual void SetText(LPCTSTR szText);

    void SetStartDelay(DWORD dwMilliseconds);
    void SetEndDelay(DWORD dwMilliseconds);
    void EnableRepeat(BOOL bEnable);
    void SetSpeed(DWORD dwSpeed);

    void SetScrollDirection(eSCROLL_DIR eScrollDir);
    eSCROLL_DIR GetScrollDirection(void);
    BOOL IsScrollingDone(void);

protected:
    virtual void OnUpdate(DWORD dwTimestamp);
    virtual void OnDraw(CLCDGfxBase &rGfx);

private:
    enum eSCROLL_STATES { STATE_START_DELAY, STATE_SCROLL, STATE_END_DELAY, STATE_DONE};

    DWORD m_dwEllapsedTime; // ellapsed time in state
    DWORD m_dwStartDelay;   // milliseconds
    DWORD m_dwEndDelay;     // milliseconds
    DWORD m_dwSpeed;        // pixels/second
    DWORD m_dwLastUpdate;   // milliseconds
    BOOL  m_bRepeat;        // repeat

    int m_nScrollingDistance;
    float m_fTotalDistance;

    eSCROLL_DIR m_eScrollDir;
    eSCROLL_STATES m_eState;
};


#endif // !_LCDSCROLLINGTEXT_H_INCLUDED_ 

//** end of LCDScrollingText.h *******************************************
