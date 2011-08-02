//************************************************************************
//
// LCDProgressBar.cpp
//
// The CLCDProgressBar class draws a progress bar onto the LCD.
// 
// Logitech LCD SDK
//
// Copyright 2005 Logitech Inc.
//************************************************************************

#include "stdafx.h"
#include "LCDProgressBar.h"

//************************************************************************
//
// CLCDProgressBar::CLCDProgressBar
//
//************************************************************************

CLCDProgressBar::CLCDProgressBar()
{
    m_Pos = 0;
    m_eStyle = STYLE_CURSOR;
    m_Range.nMin = 0;
    m_Range.nMax = 100;
    m_nCursorWidth = 5;
	m_hPen = NULL;
}


//************************************************************************
//
// CLCDProgressBar::~CLCDProgressBar
//
//************************************************************************

CLCDProgressBar::~CLCDProgressBar()
{
	if (m_hPen != NULL)
	{
		::DeleteObject(m_hPen);
		m_hPen = NULL;
	}
}


//************************************************************************
//
// CLCDProgressBar:Initialize
//
//************************************************************************

HRESULT CLCDProgressBar::Initialize()
{

    m_hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	m_hPen = ::CreatePen(PS_DOT, 1, RGB(255, 255, 255));

    return CLCDBase::Initialize();
}


//************************************************************************
//
// CLCDProgressBar::OnDraw
//
//************************************************************************

void CLCDProgressBar::OnDraw(CLCDGfx &rGfx)
{  
	HPEN	hOldPen;

	rGfx.ClearScreen();

	// draw the border
	RECT r = { 0, 0, GetWidth(), GetHeight() };
    
	FrameRect(rGfx.GetHDC(), &r, m_hBrush);

	// draw the progress
	switch(m_eStyle)
	{
	case STYLE_CURSOR:
	{
		int nCursorPos = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                                   (float)1, (float)(GetWidth() - m_nCursorWidth-1),
                                   m_Pos);
		r.left = nCursorPos;
		r.right = r.left + m_nCursorWidth;
		FillRect(rGfx.GetHDC(), &r, m_hBrush);
	}
		break;
	case STYLE_FILLED_V:
	case STYLE_FILLED_H:
	{
		int nBar = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                                  0.0f, (m_eStyle == STYLE_FILLED_H ? (float)GetWidth() : (float)GetHeight())-4,
                                  m_Pos);
		r.left   = r.left+2;
		r.bottom = r.bottom-2;
		if (m_eStyle == STYLE_FILLED_H)
		{
			r.right = nBar+2;
			r.top   = r.top+2;
		}
		else
		{
			r.right = r.right-2;
			r.top   = r.bottom-nBar;
		}

		FillRect(rGfx.GetHDC(), &r, m_hBrush);
	}
		break;
	case STYLE_DASHED_CURSOR:
	{
		int nCursorPos = (int)Scalef((float)m_Range.nMin, (float)m_Range.nMax,
                                   (float)1, (float)(GetWidth() - m_nCursorWidth-1),
                                   m_Pos);
		r.left = nCursorPos;
		r.right = r.left + m_nCursorWidth;
		FillRect(rGfx.GetHDC(), &r, m_hBrush);
		hOldPen = (HPEN)::SelectObject(rGfx.GetHDC(), m_hPen);

		::MoveToEx(rGfx.GetHDC(), 0, (r.bottom - r.top)/2, NULL);
		::LineTo(rGfx.GetHDC(), nCursorPos, (r.bottom - r.top)/2);
		::SelectObject(rGfx.GetHDC(), hOldPen);
	}
		break;
	default:
		break;
	}
}


//************************************************************************
//
// CLCDProgressBar::ResetUpdate
//
//************************************************************************

void CLCDProgressBar::ResetUpdate()
{

}


//************************************************************************
//
// CLCDProgressBar::SetRange
//
//************************************************************************

void CLCDProgressBar::SetRange(__int64 nMin, __int64 nMax)
{
    m_Range.nMin = nMin;
    m_Range.nMax = nMax;
}


//************************************************************************
//
// CLCDProgressBar::SetRange
//
//************************************************************************

void CLCDProgressBar::SetRange(RANGE& Range)
{
    m_Range = Range;
}


//************************************************************************
//
// CLCDProgressBar::GetRange
//
//************************************************************************

RANGE& CLCDProgressBar::GetRange()
{
    return m_Range;
}


//************************************************************************
//
// CLCDProgressBar::SetPos
//
//************************************************************************

__int64 CLCDProgressBar::SetPos(__int64 Pos)
{
    return ( m_Pos = max(m_Range.nMin, min(Pos, m_Range.nMax)) );
}


//************************************************************************
//
// CLCDProgressBar::GetPos
//
//************************************************************************

__int64 CLCDProgressBar::GetPos()
{
    return m_Pos;
}


//************************************************************************
//
// CLCDProgressBar::EnableCursor
//
//************************************************************************

void CLCDProgressBar::EnableCursor(BOOL bEnable)
{
    m_eStyle = bEnable ? STYLE_CURSOR : STYLE_FILLED_H;
}

//************************************************************************
//
// CLCDProgressBar::SetProgressStyle
//
//************************************************************************

void CLCDProgressBar::SetProgressStyle(ePROGRESS_STYLE eStyle)
{
	m_eStyle = eStyle;
}

//************************************************************************
//
// CLCDProgressBar::Scalef
//
//************************************************************************

float CLCDProgressBar::Scalef(float fFromMin, float fFromMax,
                             float fToMin, float fToMax, __int64 fFromValue)
{

    // normalize the input
    float fFromValueN = (fFromValue - fFromMin) / (fFromMax - fFromMin);

    // now scale to the output
    float fToRange = fToMax - fToMin;

    return ( fToMin + (fFromValueN * fToRange) );
}


//************************************************************************
//
// CLCDProgressBar::Scale
//
//************************************************************************

int CLCDProgressBar::Scale(int nFromMin, int nFromMax,
                           int nToMin, int nToMax, __int64 nFromValue)
{
    return (int)Scalef(
        (float)nFromMin,
        (float)nFromMax,
        (float)nToMin,
        (float)nToMax,
        nFromValue
        );
}


//** end of LCDProgressBar.cpp *******************************************
