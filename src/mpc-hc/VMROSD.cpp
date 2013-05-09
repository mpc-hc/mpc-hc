/*
 * (C) 2006-2013 see Authors.txt
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

#include "stdafx.h"
#include "VMROSD.h"
#include "mplayerc.h"

#define SEEKBAR_MARGIN       10
#define SEEKBAR_HEIGHT       60
#define SLIDER_BAR_MARGIN    10
#define SLIDER_BAR_HEIGHT    10
#define SLIDER_CURSOR_HEIGHT 30
#define SLIDER_CURSOR_WIDTH  15


CVMROSD::CVMROSD()
    : m_pWnd(nullptr)
    , m_llSeekMin(0)
    , m_llSeekMax(0)
    , m_llSeekPos(0)
    , m_nMessagePos(OSD_NOMESSAGE)
    , m_bShowSeekBar(false)
    , m_bSeekBarVisible(false)
    , m_bCursorMoving(false)
    , m_pMFVMB(nullptr)
    , m_pVMB(nullptr)
    , m_pMVTO(nullptr)
    , m_iFontSize(0)
    , m_fontName(_T(""))
    , m_bShowMessage(true)
{
    m_colors[OSD_TRANSPARENT] = RGB(0,     0,   0);
    m_colors[OSD_BACKGROUND]  = RGB(32,   40,  48);
    m_colors[OSD_BORDER]      = RGB(48,   56,  62);
    m_colors[OSD_TEXT]        = RGB(224, 224, 224);
    m_colors[OSD_BAR]         = RGB(64,   72,  80);
    m_colors[OSD_CURSOR]      = RGB(192, 200, 208);
    m_colors[OSD_DEBUGCLR]    = RGB(128, 136, 144);

    m_penBorder.CreatePen(PS_SOLID, 1, m_colors[OSD_BORDER]);
    m_penCursor.CreatePen(PS_SOLID, 4, m_colors[OSD_CURSOR]);
    m_brushBack.CreateSolidBrush(m_colors[OSD_BACKGROUND]);
    m_brushBar.CreateSolidBrush(m_colors[OSD_BAR]);
    m_debugBrushBack.CreateSolidBrush(m_colors[OSD_DEBUGCLR]);
    m_debugPenBorder.CreatePen(PS_SOLID, 1, m_colors[OSD_BORDER]);

    memset(&m_bitmapInfo, 0, sizeof(m_bitmapInfo));
}

CVMROSD::~CVMROSD()
{
    Stop();
    m_memDC.DeleteDC();
}

void CVMROSD::SetSize(CRect wndRect, CRect videoRect)
{
    if (m_pWnd && (m_pVMB || m_pMFVMB)) {
        if (m_bSeekBarVisible) {
            m_bCursorMoving   = false;
            m_bSeekBarVisible = false;
            Invalidate();
        }

        // Vanilla VMR9/EVR renderers draw the OSD relative to the video frame
        const CAppSettings& s = AfxGetAppSettings();
        m_rectWnd = (s.iDSVideoRendererType != VIDRNDT_DS_VMR9WINDOWED
                     && s.iDSVideoRendererType != VIDRNDT_DS_EVR) ? wndRect : videoRect;
        m_rectWnd.MoveToXY(0, 0);

        m_rectSeekBar.left   = m_rectWnd.left    + SEEKBAR_MARGIN;
        m_rectSeekBar.right  = m_rectWnd.right   - SEEKBAR_MARGIN;
        m_rectSeekBar.top    = m_rectWnd.bottom  - SEEKBAR_HEIGHT;
        m_rectSeekBar.bottom = m_rectSeekBar.top + SEEKBAR_HEIGHT;

        UpdateBitmap();
    }
}

void CVMROSD::UpdateBitmap()
{
    CAutoLock lock(&m_csLock);
    CRect rc;
    CWindowDC dc(m_pWnd);

    m_memDC.DeleteDC();
    memset(&m_bitmapInfo, 0, sizeof(m_bitmapInfo));

    if (m_memDC.CreateCompatibleDC(&dc)) {
        BITMAPINFO bmi;
        HBITMAP    hbmpRender;

        ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_rectWnd.Width();
        bmi.bmiHeader.biHeight = - m_rectWnd.Height(); // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        hbmpRender = CreateDIBSection(m_memDC, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
        m_memDC.SelectObject(hbmpRender);

        if (::GetObject(hbmpRender, sizeof(BITMAP), &m_bitmapInfo) != 0) {
            // Configure the VMR's bitmap structure
            if (m_pVMB) {
                ZeroMemory(&m_VMR9AlphaBitmap, sizeof(m_VMR9AlphaBitmap));
                m_VMR9AlphaBitmap.dwFlags      = VMRBITMAP_HDC | VMRBITMAP_SRCCOLORKEY;
                m_VMR9AlphaBitmap.hdc          = m_memDC;
                m_VMR9AlphaBitmap.rSrc         = m_rectWnd;
                m_VMR9AlphaBitmap.rDest.left   = 0;
                m_VMR9AlphaBitmap.rDest.top    = 0;
                m_VMR9AlphaBitmap.rDest.right  = 1.0;
                m_VMR9AlphaBitmap.rDest.bottom = 1.0;
                m_VMR9AlphaBitmap.fAlpha       = 1.0;
                m_VMR9AlphaBitmap.clrSrcKey    = m_colors[OSD_TRANSPARENT];
            } else if (m_pMFVMB) {
                ZeroMemory(&m_MFVideoAlphaBitmap, sizeof(m_MFVideoAlphaBitmap));
                m_MFVideoAlphaBitmap.params.dwFlags        = MFVideoAlphaBitmap_SrcColorKey;
                m_MFVideoAlphaBitmap.params.clrSrcKey      = m_colors[OSD_TRANSPARENT];
                m_MFVideoAlphaBitmap.params.rcSrc          = m_rectWnd;
                m_MFVideoAlphaBitmap.params.nrcDest.right  = 1;
                m_MFVideoAlphaBitmap.params.nrcDest.bottom = 1;
                m_MFVideoAlphaBitmap.GetBitmapFromDC       = TRUE;
                m_MFVideoAlphaBitmap.bitmap.hdc            = m_memDC;
            }
            m_memDC.SetTextColor(m_colors[OSD_TEXT]);
            m_memDC.SetBkMode(TRANSPARENT);
        }

        if (m_mainFont.GetSafeHandle()) {
            m_memDC.SelectObject(m_mainFont);
        }

        DeleteObject(hbmpRender);
    }
}

void CVMROSD::Start(CWnd* pWnd, IVMRMixerBitmap9* pVMB, bool bShowSeekBar)
{
    m_pVMB   = pVMB;
    m_pMFVMB = nullptr;
    m_pMVTO  = nullptr;
    m_pWnd   = pWnd;
    m_bShowSeekBar = bShowSeekBar;
    UpdateBitmap();
}

void CVMROSD::Start(CWnd* pWnd, IMFVideoMixerBitmap* pMFVMB, bool bShowSeekBar)
{
    m_pMFVMB = pMFVMB;
    m_pVMB   = nullptr;
    m_pMVTO  = nullptr;
    m_pWnd   = pWnd;
    m_bShowSeekBar = bShowSeekBar;
    UpdateBitmap();
}

void CVMROSD::Start(CWnd* pWnd, IMadVRTextOsd* pMVTO)
{
    m_pMFVMB = nullptr;
    m_pVMB   = nullptr;
    m_pMVTO  = pMVTO;
    m_pWnd   = pWnd;
}

void CVMROSD::Stop()
{
    m_pVMB.Release();
    m_pMFVMB.Release();
    m_pMVTO.Release();
    if (m_pWnd) {
        m_pWnd->KillTimer((UINT_PTR)this);
        m_pWnd = nullptr;
    }
}

void CVMROSD::DrawRect(CRect* rect, CBrush* pBrush, CPen* pPen)
{
    if (pPen) {
        m_memDC.SelectObject(pPen);
    } else {
        m_memDC.SelectStockObject(NULL_PEN);
    }

    if (pBrush) {
        m_memDC.SelectObject(pBrush);
    } else {
        m_memDC.SelectStockObject(HOLLOW_BRUSH);
    }

    m_memDC.Rectangle(rect);
}

void CVMROSD::DrawSlider(CRect* rect, __int64 llMin, __int64 llMax, __int64 llPos)
{
    m_rectBar.left   = rect->left    + SLIDER_BAR_MARGIN;
    m_rectBar.right  = rect->right   - SLIDER_BAR_MARGIN;
    m_rectBar.top    = rect->top     + (rect->Height() - SLIDER_BAR_HEIGHT) / 2;
    m_rectBar.bottom = m_rectBar.top + SLIDER_BAR_HEIGHT;

    if (llMax == llMin) {
        m_rectCursor.left = m_rectBar.left;
    } else {
        m_rectCursor.left = m_rectBar.left + (long)((m_rectBar.Width() - SLIDER_CURSOR_WIDTH) * llPos / (llMax - llMin));
    }
    m_rectCursor.right  = m_rectCursor.left + SLIDER_CURSOR_WIDTH;
    m_rectCursor.top    = rect->top + (rect->Height() - SLIDER_CURSOR_HEIGHT) / 2;
    m_rectCursor.bottom = m_rectCursor.top + SLIDER_CURSOR_HEIGHT;

    DrawRect(rect, &m_brushBack, &m_penBorder);
    DrawRect(&m_rectBar, &m_brushBar);
    DrawRect(&m_rectCursor, nullptr, &m_penCursor);
}

void CVMROSD::DrawMessage()
{
    if (!m_bitmapInfo.bmWidth || !m_bitmapInfo.bmHeight || !m_bitmapInfo.bmBitsPixel) {
        return;
    }
    if (m_nMessagePos != OSD_NOMESSAGE) {
        CRect rectText(0, 0, 0, 0);
        CRect rectMessages;

        m_memDC.DrawText(m_strMessage, &rectText, DT_CALCRECT);
        rectText.InflateRect(20, 10);
        switch (m_nMessagePos) {
            case OSD_TOPLEFT:
                rectMessages = CRect(10, 10, min((rectText.right + 10), (m_rectWnd.right - 10)), (rectText.bottom + 10));
                break;
            case OSD_TOPRIGHT:
            default:
                rectMessages = CRect(max(10, m_rectWnd.right - 10 - rectText.Width()), 10, m_rectWnd.right - 10, rectText.bottom + 10);
                break;
        }
        DrawRect(&rectMessages, &m_brushBack, &m_penBorder);
        DWORD uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX;
        if (rectText.right > (m_rectWnd.right - 20)) {
            m_strMessage = _T(" ") + m_strMessage;
            uFormat = uFormat | DT_END_ELLIPSIS;
        }
        m_memDC.DrawText(m_strMessage, &rectMessages, uFormat);
    }
}

void CVMROSD::DrawDebug()
{
    if (!m_debugMessages.IsEmpty()) {
        CString msg, tmp;
        POSITION pos;
        pos = m_debugMessages.GetHeadPosition();
        msg.Format(_T("%s"), m_debugMessages.GetNext(pos));

        while (pos) {
            tmp = m_debugMessages.GetNext(pos);
            if (!tmp.IsEmpty()) {
                msg.AppendFormat(_T("\r\n%s"), tmp);
            }
        }

        CRect rectText(0, 0, 0, 0);
        CRect rectMessages;
        m_memDC.DrawText(msg, &rectText, DT_CALCRECT);
        rectText.InflateRect(20, 10);

        int l, r, t, b;
        l = (m_rectWnd.Width()  >> 1) - (rectText.Width()  >> 1) - 10;
        r = (m_rectWnd.Width()  >> 1) + (rectText.Width()  >> 1) + 10;
        t = (m_rectWnd.Height() >> 1) - (rectText.Height() >> 1) - 10;
        b = (m_rectWnd.Height() >> 1) + (rectText.Height() >> 1) + 10;
        rectMessages = CRect(l, t, r, b);
        DrawRect(&rectMessages, &m_debugBrushBack, &m_debugPenBorder);
        m_memDC.DrawText(msg, &rectMessages, DT_CENTER | DT_VCENTER);
    }
}

void CVMROSD::Invalidate()
{
    CAutoLock lock(&m_csLock);
    if (!m_bitmapInfo.bmWidth || !m_bitmapInfo.bmHeight || !m_bitmapInfo.bmBitsPixel) {
        return;
    }
    memsetd(m_bitmapInfo.bmBits, 0xff000000, m_bitmapInfo.bmWidth * m_bitmapInfo.bmHeight * (m_bitmapInfo.bmBitsPixel / 8));

    if (m_bSeekBarVisible) {
        DrawSlider(&m_rectSeekBar, m_llSeekMin, m_llSeekMax, m_llSeekPos);
    }
    DrawMessage();
    DrawDebug();

    if (m_pVMB) {
        m_VMR9AlphaBitmap.dwFlags &= ~VMRBITMAP_DISABLE;
        m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
    } else if (m_pMFVMB) {
        m_pMFVMB->SetAlphaBitmap(&m_MFVideoAlphaBitmap);
    }
}

void CVMROSD::UpdateSeekBarPos(CPoint point)
{
    m_llSeekPos = (point.x - m_rectBar.left) * (m_llSeekMax - m_llSeekMin) / (m_rectBar.Width() - SLIDER_CURSOR_WIDTH);
    m_llSeekPos = max(m_llSeekPos, m_llSeekMin);
    m_llSeekPos = min(m_llSeekPos, m_llSeekMax);

    if (m_pWnd) {
        AfxGetApp()->GetMainWnd()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)m_llSeekPos, SB_THUMBTRACK), (LPARAM)m_pWnd->m_hWnd);
    }
}

bool CVMROSD::OnMouseMove(UINT nFlags, CPoint point)
{
    bool bRet = false;

    if (m_pVMB || m_pMFVMB) {
        if (m_bCursorMoving) {
            UpdateSeekBarPos(point);
            Invalidate();
        } else if (!m_bSeekBarVisible && m_bShowSeekBar && m_rectSeekBar.PtInRect(point)) {
            m_bSeekBarVisible = true;
            Invalidate();
        } else if (m_bSeekBarVisible && !m_rectSeekBar.PtInRect(point)) {
            m_bSeekBarVisible = false;
            // Add new timer for removing any messages
            if (m_pWnd) {
                m_pWnd->KillTimer((UINT_PTR)this);
                m_pWnd->SetTimer((UINT_PTR)this, 1000, (TIMERPROC)TimerFunc);
            }
            Invalidate();
        } else {
            bRet = false;
        }
    }

    return bRet;
}

bool CVMROSD::OnLButtonDown(UINT nFlags, CPoint point)
{
    bool bRet = false;
    if (m_pVMB || m_pMFVMB) {
        if (m_rectCursor.PtInRect(point)) {
            m_bCursorMoving = true;
            bRet = true;
        } else if (m_rectSeekBar.PtInRect(point)) {
            bRet = true;
            UpdateSeekBarPos(point);
            Invalidate();
        }
    }

    return bRet;
}

bool CVMROSD::OnLButtonUp(UINT nFlags, CPoint point)
{
    bool bRet = false;

    if (m_pVMB || m_pMFVMB) {
        m_bCursorMoving = false;

        bRet = (m_rectCursor.PtInRect(point) || m_rectSeekBar.PtInRect(point));
    }
    return bRet;
}

__int64 CVMROSD::GetPos() const
{
    return m_llSeekPos;
}

void CVMROSD::SetPos(__int64 pos)
{
    m_llSeekPos = pos;
}

void CVMROSD::SetRange(__int64 start,  __int64 stop)
{
    m_llSeekMin = start;
    m_llSeekMax = stop;
}

void CVMROSD::GetRange(__int64& start, __int64& stop)
{
    start = m_llSeekMin;
    stop  = m_llSeekMax;
}

void CVMROSD::TimerFunc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
    CVMROSD* pVMROSD = (CVMROSD*)nIDEvent;
    if (pVMROSD) {
        pVMROSD->ClearMessage();
    }
    KillTimer(hWnd, nIDEvent);
}

void CVMROSD::ClearMessage(bool hide)
{
    CAutoLock lock(&m_csLock);
    if (m_bSeekBarVisible) {
        return;
    }

    if (!hide) {
        m_nMessagePos = OSD_NOMESSAGE;
    }

    if (m_pVMB) {
        DWORD dwBackup = (m_VMR9AlphaBitmap.dwFlags | VMRBITMAP_DISABLE);
        m_VMR9AlphaBitmap.dwFlags = VMRBITMAP_DISABLE;
        m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
        m_VMR9AlphaBitmap.dwFlags = dwBackup;
    } else if (m_pMFVMB) {
        m_pMFVMB->ClearAlphaBitmap();
    } else if (m_pMVTO) {
        m_pMVTO->OsdClearMessage();
    }
}

void CVMROSD::DisplayMessage(OSD_MESSAGEPOS nPos, LPCTSTR strMsg, int nDuration, int iFontSize, CString fontName)
{
    if (!m_bShowMessage) {
        return;
    }

    if (m_pVMB || m_pMFVMB) {
        if (nPos != OSD_DEBUG) {
            m_nMessagePos = nPos;
            m_strMessage  = strMsg;
        } else {
            m_debugMessages.AddTail(strMsg);
            if (m_debugMessages.GetCount() > 20) {
                m_debugMessages.RemoveHead();
            }
            nDuration = -1;
        }

        int iOldFontSize = m_iFontSize;
        CString oldFontName = m_fontName;
        const CAppSettings& s = AfxGetAppSettings();

        if (iFontSize == 0) {
            m_iFontSize = s.nOSDSize;
        } else {
            m_iFontSize = iFontSize;
        }
        if (m_iFontSize < 10 || m_iFontSize > 26) {
            m_iFontSize = 20;
        }
        if (fontName.IsEmpty()) {
            m_fontName = s.strOSDFont;
        } else {
            m_fontName = fontName;
        }

        if (iOldFontSize != m_iFontSize || oldFontName != m_fontName) {
            if (m_mainFont.GetSafeHandle()) {
                m_mainFont.DeleteObject();
            }

            m_mainFont.CreatePointFont(m_iFontSize * 10, m_fontName);
            m_memDC.SelectObject(m_mainFont);
        }

        if (m_pWnd) {
            m_pWnd->KillTimer((UINT_PTR)this);
            if (nDuration != -1) {
                m_pWnd->SetTimer((UINT_PTR)this, nDuration, (TIMERPROC)TimerFunc);
            }
        }
        Invalidate();
    } else if (m_pMVTO) {
        m_pMVTO->OsdDisplayMessage(strMsg, nDuration);
    }
}

void CVMROSD::DebugMessage(LPCTSTR format, ...)
{
    CString msg;
    va_list argList;
    va_start(argList, format);
    msg.FormatV(format, argList);
    va_end(argList);

    DisplayMessage(OSD_DEBUG, msg);
}

void CVMROSD::HideMessage(bool hide)
{
    if (m_pVMB || m_pMFVMB) {
        if (hide) {
            ClearMessage(true);
        } else {
            Invalidate();
        }
    }
}

void CVMROSD::EnableShowMessage(bool enabled)
{
    m_bShowMessage = enabled;
}
