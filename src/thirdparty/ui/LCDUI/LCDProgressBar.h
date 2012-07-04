//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

//************************************************************************
//
// LCDProgressBar.h
//
// The CLCDProgressBar class draws a progress bar onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2010 Logitech Inc.
//************************************************************************

#ifndef _LCDPROGRESSBAR_H_INCLUDED_ 
#define _LCDPROGRESSBAR_H_INCLUDED_ 

#include "LCDBase.h"

typedef struct RANGE
{
    int nMin;
    int nMax;

}RANGE, *LPRANGE;

class CLCDProgressBar : public CLCDBase
{
public:    
    enum ePROGRESS_STYLE { STYLE_FILLED, STYLE_CURSOR, STYLE_DASHED_CURSOR };

    CLCDProgressBar(void);
    virtual ~CLCDProgressBar(void);

    // CLCDBase
    virtual HRESULT Initialize(void);
    virtual void OnDraw(CLCDGfxBase &rGfx);
    virtual void ResetUpdate(void);
    
    // CLCDProgressBar
    virtual void SetRange(int nMin, int nMax);
    virtual void SetRange(RANGE& Range);
    virtual RANGE& GetRange(void);
    virtual float SetPos(float fPos);
    virtual float GetPos(void);
    virtual void EnableCursor(BOOL bEnable);
    virtual void SetProgressStyle(ePROGRESS_STYLE eStyle);

protected:
    float Scalef(float fFromMin, float fFromMax,
                 float fToMin, float fToMax, float fFromValue);
    int Scale(int nFromMin, int nFromMax,
              int nToMin, int nToMax, int nFromValue);

protected:
    RANGE m_Range;
    float m_fPos;
    ePROGRESS_STYLE m_eStyle;
    HBRUSH m_hBrush;
    HPEN m_hPen;
    int m_nCursorWidth;
};


#endif // !_LCDPROGRESSBAR_H_INCLUDED_ 

//** end of LCDProgressBar.h *********************************************
