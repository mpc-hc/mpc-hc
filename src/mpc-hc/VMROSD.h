/*
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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
#include <d3d9.h>
#include <evr9.h>
#include <vmr9.h>
#include "madVRAllocatorPresenter.h"


typedef enum {
    OSD_TRANSPARENT,
    OSD_BACKGROUND,
    OSD_BORDER,
    OSD_TEXT,
    OSD_BAR,
    OSD_CURSOR,
    OSD_DEBUGCLR,
    OSD_LAST
} OSD_COLORS;

typedef enum {
    OSD_NOMESSAGE,
    OSD_TOPLEFT,
    OSD_TOPRIGHT,
    OSD_DEBUG,
} OSD_MESSAGEPOS;

class CVMROSD
{
public:
    CVMROSD(CWnd* pMainWindow);
    ~CVMROSD();

    void Start(HWND hWnd, IVMRMixerBitmap9* pVMB);
    void Start(HWND hWnd, IMFVideoMixerBitmap* pVMB);
    void Start(HWND hWnd, IMadVRTextOsd* pMVTO);
    void Stop();
    void BindToWindow(HWND hWnd);

    void DisplayMessage(OSD_MESSAGEPOS nPos, wchar_t const* strMsg, int nDuration = 5000, int FontSize = 0, CStringW OSD_Font = L"");
    void DebugMessage(wchar_t const* strFormat, ...);
    void ClearMessage();
    void EnableShowMessage(bool enabled);
    void SuppressOSD();
    void UnsuppressOSD();

    __int64 GetPos() const;
    void SetPos(__int64 pos);
    void SetRange(__int64 start,  __int64 stop);
    void GetRange(__int64& start, __int64& stop);

    void OnSize(UINT nType, int cx, int cy);
    bool OnMouseMove(UINT nFlags, CPoint point);
    bool OnLButtonDown(UINT nFlags, CPoint point);
    bool OnLButtonUp(UINT nFlags, CPoint point);

    HANDLE m_hTimer;
    bool m_bShowMessage;
private:
    bool m_bCursorMoving, m_bSeekBarVisible;
    bool m_bOSDVisible, m_bOSDSuppressed;

    IVMRMixerBitmap9*    m_pVMB;
    IMFVideoMixerBitmap* m_pMFVMB;
    IMadVRTextOsd*       m_pMVTO;

    CWnd*              m_pMainWindow;
    HWND               m_hWnd;
    CCritSec           m_Lock;
    CDC                m_MemDC;
    VMR9AlphaBitmap    m_VMR9AlphaBitmap;
    MFVideoAlphaBitmap m_MFVideoAlphaBitmap;
    BITMAP             m_BitmapInfo;

    CFont    m_MainFont;
    CPen     m_penBorder;
    CPen     m_penCursor;
    CBrush   m_brushBack;
    CBrush   m_brushBar;
    CPen     m_debugPenBorder;
    CBrush   m_debugBrushBack;
    CStringW m_OSD_Font;
    int      m_FontSize;

    CRect m_rectWnd, m_rectSeekBar, m_rectCursor, m_rectBar;
    __int64 m_s64SeekMin, m_s64SeekMax, m_s64SeekPos;

    // Messages
    CStringW        m_strMessage;
    OSD_MESSAGEPOS  m_nMessagePos;
    CList<CStringW> m_debugMessages;

    void UpdateBitmap();
    void CalcRect();
    void UpdateSeekBarPos(CPoint point);
    void DrawSlider(RECT const* rect, __int64 llMin, __int64 llMax, __int64 llPos);
    void DrawRect(RECT const* rect, CBrush* pBrush = NULL, CPen* pPen = NULL);
    void DrawMessage();
    void DrawDebug();
    void Invalidate();
};
