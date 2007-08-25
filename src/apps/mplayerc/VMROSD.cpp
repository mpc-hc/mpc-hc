/* 
 *	Copyright (C) 2007 Casimir666
 *	http://tibrium.neuf.fr
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


#include "stdafx.h"
#include ".\vmrosd.h"
#include "mplayerc.h"


#define SEEKBAR_HEIGHT			60
#define SLIDER_BAR_HEIGHT		10
#define SLIDER_CURSOR_HEIGHT	30
#define SLIDER_CURSOR_WIDTH		15	


CVMROSD::CVMROSD(void)
{
	m_Color[OSD_TRANSPARENT]	= RGB(  0,   0,   0);
	m_Color[OSD_BACKGROUND]		= RGB(  0,  96, 183);
	m_Color[OSD_BORDER]			= RGB(255, 255, 255);
	m_Color[OSD_TEXT]			= RGB(255, 255, 255);
	m_Color[OSD_BAR]			= RGB(  4, 200,  12);
	m_Color[OSD_CURSOR]			= RGB( 23,  50, 247);

	m_penBorder.CreatePen(PS_SOLID, 1, m_Color[OSD_BORDER]);
	m_penCursor.CreatePen(PS_SOLID, 4, m_Color[OSD_CURSOR]);
	m_brushBack.CreateSolidBrush(m_Color[OSD_BACKGROUND]);
	m_brushBar.CreateSolidBrush (m_Color[OSD_BAR]);
	m_MainFont.CreatePointFont (200, _T("Arial"));

	m_nMessagePos		= OSD_NOMESSAGE;
	m_bSeekBarVisible	= false;
	m_bCursorMoving		= false;
}

CVMROSD::~CVMROSD(void)
{
}


void CVMROSD::OnSize(UINT nType, int cx, int cy)
{
	if (m_pWnd && (m_pVMB || m_pMFVMB))
	{
		if (m_bSeekBarVisible)
		{
			m_bCursorMoving		= false;
			m_bSeekBarVisible	= false;
			Invalidate();
		}
		UpdateVMRBitmap();
	}
}


void CVMROSD::UpdateVMRBitmap()
{
	CRect				rc;
	CWindowDC			dc (m_pWnd);

	CalcRect();

	m_MemDC.DeleteDC();
	m_Bitmap.DeleteObject();

	if (m_MemDC.CreateCompatibleDC (&dc))
	{
		BITMAPINFO	bmi = {0};
		HBITMAP		hbmpRender;
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_rectWnd.Width();
		bmi.bmiHeader.biHeight = - (int) m_rectWnd.Height(); // top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		hbmpRender = CreateDIBSection( m_MemDC, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL );
		m_MemDC.SelectObject (hbmpRender);
		
		if (::GetObject(hbmpRender, sizeof(BITMAP), &m_BitmapInfo) != 0)
		{
			// Configure the VMR's bitmap structure
			ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap) );
			m_VMR9AlphaBitmap.dwFlags		= VMRBITMAP_HDC | VMRBITMAP_SRCCOLORKEY;
			m_VMR9AlphaBitmap.hdc			= m_MemDC;
			m_VMR9AlphaBitmap.rSrc			= m_rectWnd;
			m_VMR9AlphaBitmap.rDest.left	= 0;
			m_VMR9AlphaBitmap.rDest.top		= 0;
			m_VMR9AlphaBitmap.rDest.right	= 1.0;
			m_VMR9AlphaBitmap.rDest.bottom	= 1.0;
			m_VMR9AlphaBitmap.fAlpha		= 1.0;
			m_VMR9AlphaBitmap.clrSrcKey		= m_Color[OSD_TRANSPARENT];

			m_MemDC.SelectObject(m_MainFont);
			m_MemDC.SetTextColor(RGB(255, 255, 255));
			m_MemDC.SetBkMode(TRANSPARENT);
		}
	}

}


void CVMROSD::UpdateMFBitmap()
{
	CRect				rc;
	CWindowDC			dc (m_pWnd);

	CalcRect();

	m_MemDC.DeleteDC();
	m_Bitmap.DeleteObject();

	if (m_MemDC.CreateCompatibleDC (&dc))
	{
		BITMAPINFO	bmi = {0};
		HBITMAP		hbmpRender;
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = m_rectWnd.Width();
		bmi.bmiHeader.biHeight = - (int) m_rectWnd.Height(); // top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		hbmpRender = CreateDIBSection( m_MemDC, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL );
		m_MemDC.SelectObject (hbmpRender);
		
		if (::GetObject(hbmpRender, sizeof(BITMAP), &m_BitmapInfo) != 0)
		{
			// Configure the VMR's bitmap structure
			ZeroMemory(&m_MFVideoAlphaBitmap, sizeof(m_MFVideoAlphaBitmap) );
			m_MFVideoAlphaBitmap.params.dwFlags		= MFVideoAlphaBitmap_SrcColorKey;
			m_MFVideoAlphaBitmap.params.clrSrcKey	= m_Color[OSD_TRANSPARENT];
			m_MFVideoAlphaBitmap.params.rcSrc		= m_rectWnd;
			m_MFVideoAlphaBitmap.params.nrcDest.right	= 1;
			m_MFVideoAlphaBitmap.params.nrcDest.bottom  = 1;

			m_MFVideoAlphaBitmap.GetBitmapFromDC	= TRUE;
			m_MFVideoAlphaBitmap.bitmap.hdc			= m_MemDC;

			m_MemDC.SelectObject(m_MainFont);
			m_MemDC.SetTextColor(RGB(255, 255, 255));
			m_MemDC.SetBkMode(TRANSPARENT);
		}
	}

}
void CVMROSD::Start (CWnd* pWnd, CComPtr<IVMRMixerBitmap9> pVMB)
{
	m_pVMB				= pVMB;
	m_pWnd				= pWnd;
	UpdateVMRBitmap();
}


void CVMROSD::Start (CWnd* pWnd, CComPtr<IMFVideoMixerBitmap> pMFVMB)
{
	m_pMFVMB			= pMFVMB;
	m_pWnd				= pWnd;
	UpdateMFBitmap();
}


void CVMROSD::Stop()
{
	m_pVMB.Release();
	m_pWnd  = NULL;
}

void CVMROSD::CalcRect()
{
	if (m_pWnd)
	{
		m_pWnd->GetClientRect(&m_rectWnd);

		m_rectSeekBar.left		= m_rectWnd.left	+ 10;
		m_rectSeekBar.right		= m_rectWnd.right	- 10;
		m_rectSeekBar.top		= m_rectWnd.bottom	- SEEKBAR_HEIGHT;
		m_rectSeekBar.bottom	= m_rectSeekBar.top	+ SEEKBAR_HEIGHT;

		m_rectSeekBar.left		= m_rectSeekBar.left;
		m_rectSeekBar.right		= m_rectSeekBar.right;
		m_rectSeekBar.top		= m_rectSeekBar.top;
		m_rectSeekBar.bottom	= m_rectSeekBar.bottom;
	}
}

void CVMROSD::DrawRect(CRect* rect, CBrush* pBrush, CPen* pPen)
{
	if (pPen)
		m_MemDC.SelectObject (pPen);
	else
		m_MemDC.SelectStockObject(NULL_PEN);

	if (pBrush)
		m_MemDC.SelectObject (pBrush);
	else
		m_MemDC.SelectStockObject(HOLLOW_BRUSH);

	m_MemDC.Rectangle	 (rect);
}

void CVMROSD::DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos)
{
	m_rectBar.left		= rect->left  + 10;
	m_rectBar.right		= rect->right - 10;
	m_rectBar.top		= rect->top   + (rect->Height() - SLIDER_BAR_HEIGHT) / 2;
	m_rectBar.bottom	= m_rectBar.top + SLIDER_BAR_HEIGHT;

	if (llMax == llMin)
		m_rectCursor.left	= m_rectBar.left;
	else
		m_rectCursor.left	= m_rectBar.left + (long)((m_rectBar.Width() - SLIDER_CURSOR_WIDTH) * llPos / (llMax-llMin));
	m_rectCursor.right	= m_rectCursor.left + SLIDER_CURSOR_WIDTH;
	m_rectCursor.top	= rect->top   + (rect->Height() - SLIDER_CURSOR_HEIGHT) / 2;
	m_rectCursor.bottom	= m_rectCursor.top + SLIDER_CURSOR_HEIGHT;

	DrawRect (rect,	&m_brushBack, &m_penBorder);
	DrawRect (&m_rectBar, &m_brushBar);
	DrawRect (&m_rectCursor, NULL, &m_penCursor);
}


void CVMROSD::DrawMessage()
{
 	if (m_nMessagePos != OSD_NOMESSAGE)
	{
		CRect		rectText (0,0,0,0);
		CRect		rectMessages;

		m_MemDC.DrawText (m_strMessage, &rectText, DT_CALCRECT);
		rectText.InflateRect(20, 10);
		switch (m_nMessagePos)
		{
		case OSD_TOPLEFT :
			rectMessages = CRect  (10, 10, rectText.right + 10, rectText.bottom + 10);
			break;
		case OSD_TOPRIGHT :
		default :
			rectMessages = CRect  (m_rectWnd.right-10-rectText.Width(), 10, m_rectWnd.right-10, rectText.bottom + 10);
			break;
		}
		DrawRect (&rectMessages,	&m_brushBack, &m_penBorder);
		m_MemDC.DrawText (m_strMessage, &rectMessages, DT_SINGLELINE |DT_CENTER|DT_VCENTER);
	}
}

void CVMROSD::Invalidate()
{
	memsetd(m_BitmapInfo.bmBits, 0xff000000, m_BitmapInfo.bmWidth*m_BitmapInfo.bmHeight*(m_BitmapInfo.bmBitsPixel/8));
	
	if (m_bSeekBarVisible) DrawSlider(&m_rectSeekBar, m_llSeekMin, m_llSeekMax, m_llSeekPos);
	DrawMessage();

	if (m_pVMB)
	{
		m_VMR9AlphaBitmap.dwFlags &= ~VMRBITMAP_DISABLE;
		m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
	}
	else if (m_pMFVMB)
	{
		m_pMFVMB->SetAlphaBitmap (&m_MFVideoAlphaBitmap);
	}
}

void CVMROSD::UpdateSeekBarPos(CPoint point)
{
	m_llSeekPos = (point.x - m_rectBar.left) * (m_llSeekMax-m_llSeekMin) / (m_rectBar.Width() - SLIDER_CURSOR_WIDTH);
	m_llSeekPos = max (m_llSeekPos, m_llSeekMin);
	m_llSeekPos = min (m_llSeekPos, m_llSeekMax);
	
	if (m_pWnd)
		AfxGetApp()->GetMainWnd()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_llSeekPos, SB_THUMBTRACK), (LPARAM)m_pWnd->m_hWnd);
}

bool CVMROSD::OnMouseMove(UINT nFlags, CPoint point)
{
	bool		bRet = false;

	if (m_pVMB || m_pMFVMB)
	{
		if (m_bCursorMoving)
		{
			UpdateSeekBarPos(point);
			Invalidate();
		}
		else if (!m_bSeekBarVisible && AfxGetAppSettings().IsD3DFullscreen() && m_rectSeekBar.PtInRect(point))
		{
			m_bSeekBarVisible = true;
			Invalidate();
		}
		else if (m_bSeekBarVisible && !m_rectSeekBar.PtInRect(point))
		{
			m_bSeekBarVisible = false;
			Invalidate();
		}
		else
			bRet = false;
	}

	return bRet;
}

bool CVMROSD::OnLButtonDown(UINT nFlags, CPoint point)
{
	bool		bRet = false;
	if (m_pVMB || m_pMFVMB)		
	{
		if (m_rectCursor.PtInRect (point))
		{
			m_bCursorMoving = true;
			bRet			= true;
		}
		else if (m_rectSeekBar.PtInRect(point))
		{
			bRet			= true;
			UpdateSeekBarPos(point);
			Invalidate();
		}
	}

	return bRet;
}

bool CVMROSD::OnLButtonUp(UINT nFlags, CPoint point)
{
	bool		bRet = false;

	if (m_pVMB || m_pMFVMB)
	{
		m_bCursorMoving = false;
		
		bRet = (m_rectCursor.PtInRect (point) || m_rectSeekBar.PtInRect(point));
	}
	return bRet;
}

__int64 CVMROSD::GetPos()
{
	return m_llSeekPos;
}

void CVMROSD::SetPos(__int64 pos)
{
	m_llSeekPos = pos;
}

void CVMROSD::SetRange(__int64 start,  __int64 stop)
{
	m_llSeekMin	= start;
	m_llSeekMax = stop;
}

void CVMROSD::GetRange(__int64& start, __int64& stop)
{
	start	= m_llSeekMin;
	stop	= m_llSeekMax;
}

void CVMROSD::TimerFunc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	CVMROSD*	pVMROSD = (CVMROSD*) nIDEvent;
	if (pVMROSD)
	{
		pVMROSD->ClearMessage();
	}
}

void CVMROSD::ClearMessage()
{
	if (m_pVMB) 
	{
		m_VMR9AlphaBitmap.dwFlags  |= VMRBITMAP_DISABLE;
		m_nMessagePos				= OSD_NOMESSAGE;
		m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
	}

	if (m_pMFVMB)
	{
		m_pMFVMB->ClearAlphaBitmap();
	}
}

void CVMROSD::DisplayMessage (OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration)
{
	if (m_pVMB || m_pMFVMB)
	{
		m_nMessagePos	= nPos;
		m_strMessage	= strMsg;

		if (m_pWnd)
		{
			KillTimer(m_pWnd->m_hWnd, (long)this);
			if (nDuration != -1) SetTimer(m_pWnd->m_hWnd, (long)this, nDuration, TimerFunc);
		}
		Invalidate();
	}
}
