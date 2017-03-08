/*
 * (C) 2006-2014, 2016 see Authors.txt
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

#include <evr9.h>
#include <vmr9.h>


enum OSD_COLORS {
    OSD_TRANSPARENT,
    OSD_BACKGROUND,
    OSD_BORDER,
    OSD_TEXT,
    OSD_BAR,
    OSD_CURSOR,
    OSD_DEBUGCLR,
    OSD_LAST
};

enum OSD_MESSAGEPOS {
    OSD_NOMESSAGE,
    OSD_TOPLEFT,
    OSD_TOPRIGHT,
    OSD_DEBUG
};

interface IMadVRTextOsd;
interface IDSMChapterBag;
class CMainFrame;

class CVMROSD
{
public:
    CVMROSD(CMainFrame* pMainFrame);
    ~CVMROSD();

    void Start(CWnd* pWnd, IVMRMixerBitmap9* pVMB, bool bShowSeekBar);
    void Start(CWnd* pWnd, IMFVideoMixerBitmap* pVMB, bool bShowSeekBar);
    void Start(CWnd* pWnd, IMadVRTextOsd* pMVTO);
    void Stop();

    void DisplayMessage(OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration = 5000, int iFontSize = 0, CString fontName = _T(""));
    void DebugMessage(LPCTSTR format, ...);
    void ClearMessage(bool hide = false);
    void HideMessage(bool hide);
    void EnableShowMessage(bool enabled = true);

    __int64 GetPos() const;
    void SetPos(__int64 pos);
    void SetRange(__int64 start,  __int64 stop);
    void GetRange(__int64& start, __int64& stop);

    void SetSize(const CRect& wndRect, const CRect& videoRect);
    bool OnMouseMove(UINT nFlags, CPoint point);
    void OnMouseLeave();
    bool OnLButtonDown(UINT nFlags, CPoint point);
    bool OnLButtonUp(UINT nFlags, CPoint point);

    void EnableShowSeekBar(bool enabled = true);
    void SetVideoWindow(CWnd* pWnd);

    void SetChapterBag(IDSMChapterBag* pCB);
    void RemoveChapters();
private:
    CComPtr<IVMRMixerBitmap9>    m_pVMB;
    CComPtr<IMFVideoMixerBitmap> m_pMFVMB;
    CComPtr<IMadVRTextOsd>       m_pMVTO;
    CComPtr<IDSMChapterBag>      m_pCB;

    CMainFrame* m_pMainFrame;
    CWnd* m_pWnd;

    CCritSec           m_csLock;
    CDC                m_memDC;
    VMR9AlphaBitmap    m_VMR9AlphaBitmap;
    MFVideoAlphaBitmap m_MFVideoAlphaBitmap;
    BITMAP             m_bitmapInfo;

    CFont   m_mainFont;
    CPen    m_penBorder;
    CPen    m_penCursor;
    CBrush  m_brushBack;
    CBrush  m_brushBar;
    CBrush  m_brushChapter;
    CPen    m_debugPenBorder;
    CBrush  m_debugBrushBack;
    int     m_iFontSize;
    CString m_fontName;

    CRect    m_rectWnd;
    COLORREF m_colors[OSD_LAST];

    // Seekbar
    CRect   m_rectSeekBar;
    CRect   m_rectCursor;
    CRect   m_rectBar;
    bool    m_bCursorMoving;
    bool    m_bShowSeekBar;
    bool    m_bSeekBarVisible;
    __int64 m_llSeekMin;
    __int64 m_llSeekMax;
    __int64 m_llSeekPos;

    bool    m_bShowMessage;

    // Messages
    CString        m_strMessage;
    OSD_MESSAGEPOS m_nMessagePos;
    CList<CString> m_debugMessages;

    void UpdateBitmap();
    void UpdateSeekBarPos(CPoint point);
    void DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos);
    void DrawRect(const CRect* rect, CBrush* pBrush = nullptr, CPen* pPen = nullptr);
    void Invalidate();
    void DrawMessage();
    void DrawDebug();

    static void CALLBACK TimerFunc(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime);
};
