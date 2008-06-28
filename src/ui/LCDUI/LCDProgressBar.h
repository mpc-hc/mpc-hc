//************************************************************************
//
// LCDProgressBar.h
//
// The CLCDProgressBar class draws a progress bar onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#ifndef _LCDPROGRESSBAR_H_INCLUDED_ 
#define _LCDPROGRESSBAR_H_INCLUDED_ 

#include "LCDBase.h"

typedef struct RANGE
{
    __int64 nMin;
    __int64 nMax;

}RANGE, *LPRANGE;

enum ePROGRESS_STYLE { STYLE_FILLED_H, STYLE_FILLED_V, STYLE_CURSOR, STYLE_DASHED_CURSOR };

class CLCDProgressBar : public CLCDBase
{
public:
	enum ePROGRESS_STYLE { STYLE_FILLED_H, STYLE_FILLED_V, STYLE_CURSOR, STYLE_DASHED_CURSOR };
    
    CLCDProgressBar();
    virtual ~CLCDProgressBar();

    // CLCDBase
    virtual HRESULT Initialize(void);
    virtual void OnDraw(CLCDGfx &rGfx);
    virtual void ResetUpdate(void);
    
    // CLCDProgressBar
    virtual void SetRange(__int64 nMin, __int64 nMax);
    virtual void SetRange(RANGE& Range);
    virtual RANGE& GetRange(void);
    virtual __int64 SetPos(__int64 fPos);
    virtual __int64 GetPos(void);
    virtual void EnableCursor(BOOL bEnable);
	virtual void SetProgressStyle(ePROGRESS_STYLE eStyle);

protected:
    float Scalef(float fFromMin, float fFromMax,
                 float fToMin, float fToMax, __int64 fFromValue);
    int Scale(int nFromMin, int nFromMax,
              int nToMin, int nToMax, __int64 nFromValue);

private:
    RANGE m_Range;
    __int64 m_Pos;
    ePROGRESS_STYLE m_eStyle;
    HBRUSH m_hBrush;
	HPEN m_hPen;
    int m_nCursorWidth;
};


#endif // !_LCDPROGRESSBAR_H_INCLUDED_ 

//** end of LCDProgressBar.h *********************************************
