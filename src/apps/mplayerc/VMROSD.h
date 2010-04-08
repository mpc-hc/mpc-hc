/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <atlbase.h>
#include <D3d9.h>
#include <Vmr9.h>
#include <evr9.h>


typedef enum
{
    OSD_TRANSPARENT,
    OSD_BACKGROUND,
    OSD_BORDER,
    OSD_TEXT,
    OSD_BAR,
    OSD_CURSOR,
    OSD_LAST
} OSD_COLORS;

typedef enum
{
    OSD_NOMESSAGE,
    OSD_TOPLEFT,
    OSD_TOPRIGHT,
} OSD_MESSAGEPOS;


class CVMROSD
{
public:
    CVMROSD(void);
    ~CVMROSD(void);

    void Start(CWnd* pWnd, CComPtr<IVMRMixerBitmap9> pVMB);
    void Start(CWnd* pWnd, CComPtr<IMFVideoMixerBitmap> pVMB);
    void Stop();

    void DisplayMessage(OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration = 5000, int FontSize = 0, CString OSD_Font = _T(""));
    void ClearMessage();

    __int64 GetPos();
    void SetPos(__int64 pos);
    void SetRange(__int64 start,  __int64 stop);
    void GetRange(__int64& start, __int64& stop);

    void OnSize(UINT nType, int cx, int cy);
    bool OnMouseMove(UINT nFlags, CPoint point);
    bool OnLButtonDown(UINT nFlags, CPoint point);
    bool OnLButtonUp(UINT nFlags, CPoint point);

private :
    CComPtr<IVMRMixerBitmap9>	m_pVMB;
    CComPtr<IMFVideoMixerBitmap>	m_pMFVMB;
    CWnd*				m_pWnd;

    CCritSec	m_Lock;
    CDC			m_MemDC;
    VMR9AlphaBitmap		m_VMR9AlphaBitmap;
    MFVideoAlphaBitmap	m_MFVideoAlphaBitmap;
    BITMAP			m_BitmapInfo;

    CFont	m_MainFont;
    CPen	m_penBorder;
    CPen	m_penCursor;
    CBrush	m_brushBack;
    CBrush	m_brushBar;
    int		m_FontSize;
    CString m_OSD_Font;

    CRect		m_rectWnd;
    COLORREF	m_Color[OSD_LAST];

    // Curseur de calage
    CRect	m_rectSeekBar;
    CRect	m_rectCursor;
    CRect	m_rectBar;
    bool	m_bCursorMoving;
    bool	m_bSeekBarVisible;
    __int64	m_llSeekMin;
    __int64	m_llSeekMax;
    __int64	m_llSeekPos;

    // Messages
    CString		m_strMessage;
    OSD_MESSAGEPOS	m_nMessagePos;

    void UpdateBitmap();
    void CalcRect();
    void UpdateSeekBarPos(CPoint point);
    void DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos);
    void DrawRect(CRect* rect, CBrush* pBrush = NULL, CPen* pPen = NULL);
    void Invalidate();
    void DrawMessage();

    static void CALLBACK TimerFunc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);

};
