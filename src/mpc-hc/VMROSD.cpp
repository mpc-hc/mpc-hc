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

#include "stdafx.h"
#include "VMROSD.h"
#include "mplayerc.h"

#define SEEKBAR_HEIGHT          40
#define SLIDER_BAR_HEIGHT       10
#define SLIDER_CURSOR_HEIGHT    30
#define SLIDER_CURSOR_WIDTH     10

static COLORREF const m_OSDColor[OSD_LAST] = {
    RGB(0,   0,   0),  // OSD_TRANSPARENT
    RGB(32,  40,  48), // OSD_BACKGROUND
    RGB(48,  56,  62), // OSD_BORDER
    RGB(224, 224, 224),// OSD_TEXT
    RGB(64,  72,  80), // OSD_BAR
    RGB(192, 200, 208),// OSD_CURSOR
    RGB(128, 136, 144) // OSD_DEBUGCLR
};

// WAITORTIMERCALLBACK implementation
static void CALLBACK TimerFunc(__in PVOID lpParameter, __in BOOLEAN TimerOrWaitFired)
{
    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    TRACE(L"OSD TimerFunc\n");
    CVMROSD* pVMROSD = reinterpret_cast<CVMROSD*>(lpParameter);
    ASSERT(pVMROSD->m_hTimer);
    pVMROSD->m_hTimer = NULL;// before the call to WAITORTIMERCALLBACK(), the handle is invalidated for one-time timers
    pVMROSD->ClearMessage();
}

CVMROSD::CVMROSD(CWnd* pMainWindow)
    : m_pMainWindow(pMainWindow)
    , m_s64SeekMin(0)
    , m_s64SeekMax(0)
    , m_s64SeekPos(0)
    , m_hTimer(NULL)
    , m_pVMB(NULL)
    , m_pMFVMB(NULL)
    , m_pMVTO(NULL)
    , m_hWnd(NULL)
    , m_FontSize(0)
    , m_nMessagePos(OSD_NOMESSAGE)
    , m_bCursorMoving(false)
    , m_bSeekBarVisible(false)
    , m_bOSDVisible(false)
    , m_bOSDSuppressed(false)
    , m_bShowMessage(true)
{
    ZeroMemory(&m_BitmapInfo, sizeof(m_BitmapInfo));
    ASSERT(m_pMainWindow);

    BOOL b = m_penBorder.CreatePen(PS_SOLID, 1, m_OSDColor[OSD_BORDER]);
    ASSERT(b);
    b = m_penCursor.CreatePen(PS_SOLID, 4, m_OSDColor[OSD_CURSOR]);
    ASSERT(b);
    b = m_brushBack.CreateSolidBrush(m_OSDColor[OSD_BACKGROUND]);
    ASSERT(b);
    b = m_brushBar.CreateSolidBrush(m_OSDColor[OSD_BAR]);
    ASSERT(b);
    b = m_debugBrushBack.CreateSolidBrush(m_OSDColor[OSD_DEBUGCLR]);
    ASSERT(b);
    b = m_debugPenBorder.CreatePen(PS_SOLID, 1, m_OSDColor[OSD_BORDER]);
    ASSERT(b);
}

CVMROSD::~CVMROSD()
{
    if (m_hTimer) {
        BOOL b = DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        ASSERT(b);
    }
    if (m_pVMB) {
        m_pVMB->Release();
    } else if (m_pMFVMB) {
        m_pMFVMB->Release();
    } else if (m_pMVTO) {
        m_pMVTO->Release();
    }
    m_MemDC.DeleteDC();// no ASSERT here, m_MemDC can be empty already
}

void CVMROSD::Start(HWND hWnd, IVMRMixerBitmap9* pVMB)
{
    ASSERT(hWnd);
    ASSERT(pVMB);
    ASSERT(!m_pVMB);
    ASSERT(!m_pMFVMB);
    ASSERT(!m_pMVTO);
    TRACE(L"OSD Start, IVMRMixerBitmap9\n");
    m_hWnd = hWnd;
    m_pVMB = pVMB;
    pVMB->AddRef();
    UpdateBitmap();
}

void CVMROSD::Start(HWND hWnd, IMFVideoMixerBitmap* pMFVMB)
{
    ASSERT(hWnd);
    ASSERT(pMFVMB);
    ASSERT(!m_pVMB);
    ASSERT(!m_pMFVMB);
    ASSERT(!m_pMVTO);
    TRACE(L"OSD Start, IMFVideoMixerBitmap\n");
    m_hWnd = hWnd;
    m_pMFVMB = pMFVMB;
    pMFVMB->AddRef();
    UpdateBitmap();
}

void CVMROSD::Start(HWND hWnd, IMadVRTextOsd* pMVTO)
{
    ASSERT(hWnd);
    ASSERT(pMVTO);
    ASSERT(!m_pVMB);
    ASSERT(!m_pMFVMB);
    ASSERT(!m_pMVTO);
    TRACE(L"OSD Start, IMadVRTextOsd\n");
    m_hWnd = hWnd;
    m_pMVTO = pMVTO;
    pMVTO->AddRef();
}

void CVMROSD::Stop()
{
    CAutoLock Lock(&m_Lock);
    TRACE(L"OSD Stop\n");

    if (m_hTimer) {
        BOOL b = DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        ASSERT(b);
        m_hTimer = NULL;
    }
    m_bSeekBarVisible = false;// force the clearing
    ClearMessage();// even required if none of the OSD interface pointers is set

    if (m_pVMB) {
        m_pVMB->Release();
        m_pVMB = NULL;
    } else if (m_pMFVMB) {
        m_pMFVMB->Release();
        m_pMFVMB = NULL;
    } else if (m_pMVTO) {
        m_pMVTO->Release();
        m_pMVTO = NULL;
    }
    m_hWnd = NULL;
}

void CVMROSD::BindToWindow(HWND hWnd)
{
    CAutoLock Lock(&m_Lock);
    TRACE(L"OSD BindToWindow\n");
    m_hWnd = hWnd;
    ClearMessage();
}

void CVMROSD::DisplayMessage(OSD_MESSAGEPOS nPos, wchar_t const* strMsg, int nDuration, int FontSize, CStringW OSD_Font)
{
    CAutoLock Lock(&m_Lock);
    TRACE(L"OSD DisplayMessage: %s, duration: %d\n", strMsg, nDuration);
    if (!m_bShowMessage) {
        TRACE(L"OSD DisplayMessage suppressed\n");
        return;
    }

    if (m_pVMB || m_pMFVMB) {
        if (m_hTimer) {
            BOOL b = DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
            ASSERT(b);
            m_hTimer = NULL;
        }

        if (nPos != OSD_DEBUG) {
            m_nMessagePos = nPos;
            m_strMessage = strMsg;
        } else {
            m_debugMessages.AddTail(strMsg);
            if (m_debugMessages.GetCount() > 20) {
                m_debugMessages.RemoveHead();
            }
            nDuration = -1;
        }

        int temp_m_FontSize = m_FontSize;
        CStringW temp_m_OSD_Font = m_OSD_Font;

        if (FontSize == 0) {
            m_FontSize = AfxGetAppSettings().nOSDSize;
        } else {
            m_FontSize = FontSize;
        }
        if (m_FontSize < 10 || m_FontSize > 26) {
            m_FontSize = 20;
        }
        if (OSD_Font.IsEmpty()) {
            m_OSD_Font = AfxGetAppSettings().strOSDFont;
        } else {
            m_OSD_Font = OSD_Font;
        }

        if ((temp_m_FontSize != m_FontSize) || (temp_m_OSD_Font != m_OSD_Font)) {
            if (m_MainFont.GetSafeHandle()) {
                BOOL b = m_MainFont.DeleteObject();
                ASSERT(b);
            }

            BOOL b = m_MainFont.CreatePointFont(m_FontSize * 10, m_OSD_Font);
            ASSERT(b);
            m_MemDC.SelectObject(m_MainFont);
        }

        Invalidate();

        if (nDuration != -1) {// -1 signifies that the message should not be automatically cleared, disable the timer if present
            BOOL b = CreateTimerQueueTimer(&m_hTimer, NULL, TimerFunc, this, nDuration, 0, WT_EXECUTEINTIMERTHREAD);
            ASSERT(b);
        }
    } else if (m_pMVTO) {
        HRESULT hr = m_pMVTO->OsdDisplayMessage(strMsg, nDuration);
        ASSERT(SUCCEEDED(hr));
    }
}

void CVMROSD::DebugMessage(wchar_t const* strFormat, ...)
{
    CAutoLock Lock(&m_Lock);
    CStringW tmp;
    va_list argList;
    va_start(argList, strFormat);
    tmp.FormatV(strFormat, argList);
    va_end(argList);

    TRACE(L"OSD DebugMessage: %s\n", tmp);
    DisplayMessage(OSD_DEBUG, tmp);
}

void CVMROSD::ClearMessage()
{
    CAutoLock Lock(&m_Lock);
    TRACE(L"OSD ClearMessage\n");
    if (m_hTimer) {
        BOOL b = DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
        ASSERT(b);
        m_hTimer = NULL;
    }
    m_nMessagePos = OSD_NOMESSAGE;

    if (m_bSeekBarVisible) {
        TRACE(L"OSD ClearMessage suppressed because of seekbar\n");
        return;
    }

    if (m_bOSDVisible) {
        m_bOSDVisible = false;
        HRESULT hr;
        if (m_pVMB) {
            DWORD dwBackup = m_VMR9AlphaBitmap.dwFlags | VMRBITMAP_DISABLE;
            m_VMR9AlphaBitmap.dwFlags = VMRBITMAP_DISABLE;
            hr = m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
            ASSERT(SUCCEEDED(hr));
            m_VMR9AlphaBitmap.dwFlags = dwBackup;
        } else if (m_pMFVMB) {
            hr = m_pMFVMB->ClearAlphaBitmap();
            ASSERT(SUCCEEDED(hr));
        } else if (m_pMVTO) {
            hr = m_pMVTO->OsdClearMessage();
            ASSERT(SUCCEEDED(hr));
        }
    }
}

void CVMROSD::EnableShowMessage(bool enabled)
{
    TRACE(L"OSD EnableShowMessage: %hu\n", enabled);
    m_bShowMessage = enabled;
}

void CVMROSD::SuppressOSD()
{
    CAutoLock Lock(&m_Lock);
    TRACE(L"OSD SuppressOSD\n");
    m_bOSDSuppressed = true;
    m_bSeekBarVisible = false;// force the clearing
    ClearMessage();
}

void CVMROSD::UnsuppressOSD()
{
    CAutoLock Lock(&m_Lock);
    TRACE(L"OSD UnsuppressOSD\n");
    m_bOSDSuppressed = false;
    Invalidate();
}

__int64 CVMROSD::GetPos() const
{
    return m_s64SeekPos;
}

void CVMROSD::SetPos(__int64 pos)
{
    m_s64SeekPos = pos;
}

void CVMROSD::SetRange(__int64 start,  __int64 stop)
{
    m_s64SeekMin = start;
    m_s64SeekMax = stop;
}

void CVMROSD::GetRange(__int64& start, __int64& stop)
{
    start = m_s64SeekMin;
    stop = m_s64SeekMax;
}

void CVMROSD::OnSize(UINT nType, int cx, int cy)
{
    if (m_pVMB || m_pMFVMB) {
        CAutoLock Lock(&m_Lock);
        UpdateBitmap();
        m_bCursorMoving = false;
        m_bSeekBarVisible = false;
        Invalidate();
    }
}

bool CVMROSD::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_pVMB || m_pMFVMB) {
        CAutoLock Lock(&m_Lock);
        if (m_bCursorMoving) {
            UpdateSeekBarPos(point);
            Invalidate();
        } else if (!m_bSeekBarVisible && m_rectSeekBar.PtInRect(point) && AfxGetAppSettings().IsD3DFullscreen()) {
            m_bSeekBarVisible = true;
            Invalidate();
            // Add new timer for removing any messages
            if (m_hTimer) {
                BOOL b = DeleteTimerQueueTimer(NULL, m_hTimer, NULL);
                ASSERT(b);
            }
            BOOL b = CreateTimerQueueTimer(&m_hTimer, NULL, TimerFunc, this, 1000, 0, WT_EXECUTEINTIMERTHREAD);
            ASSERT(b);
        } else if (m_bSeekBarVisible && !m_rectSeekBar.PtInRect(point)) {
            m_bSeekBarVisible = false;
            Invalidate();
        }
    }

    return false;
}

bool CVMROSD::OnLButtonDown(UINT nFlags, CPoint point)
{
    bool bRet = false;
    if (m_pVMB || m_pMFVMB) {
        CAutoLock Lock(&m_Lock);
        if (m_rectCursor.PtInRect(point)) {
            m_bCursorMoving = true;
            bRet = true;
        } else if (m_rectSeekBar.PtInRect(point)) {
            UpdateSeekBarPos(point);
            Invalidate();
            bRet = true;
        }
    }

    return bRet;
}

bool CVMROSD::OnLButtonUp(UINT nFlags, CPoint point)
{
    bool bRet = false;
    if (m_pVMB || m_pMFVMB) {
        CAutoLock Lock(&m_Lock);
        m_bCursorMoving = false;
        bRet = m_rectCursor.PtInRect(point) || m_rectSeekBar.PtInRect(point);
    }

    return bRet;
}

void CVMROSD::UpdateBitmap()
{
    TRACE(L"OSD UpdateBitmap\n");
    CalcRect();// for the seek bar

    ZeroMemory(&m_BitmapInfo, sizeof(m_BitmapInfo));
    m_MemDC.DeleteDC();// remove the old one

    HDC hDCWnd = GetWindowDC(m_hWnd);
    ASSERT(hDCWnd);
    HDC hDCutil = CreateCompatibleDC(hDCWnd);// always make a copy
    ASSERT(hDCutil);

    if (m_MemDC.Attach(hDCutil)) {// if for some reason CreateCompatibleDC() returned something invalid, this provides safety
        BITMAPINFO bmi;
        *reinterpret_cast<unsigned __int32*>(bmi.bmiColors) = 0;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_rectWnd.Width();
        bmi.bmiHeader.biHeight = - m_rectWnd.Height(); // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        bmi.bmiHeader.biXPelsPerMeter = 0;
        bmi.bmiHeader.biYPelsPerMeter = 0;
        bmi.bmiHeader.biClrUsed = 0;
        bmi.bmiHeader.biClrImportant = 0;

        HBITMAP hbmpRender = CreateDIBSection(m_MemDC, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL);
        m_MemDC.SelectObject(hbmpRender);

        if (GetObjectW(hbmpRender, sizeof(BITMAP), &m_BitmapInfo) != 0) {
            // Configure the VMR's bitmap structure
            if (m_pVMB) {
                m_VMR9AlphaBitmap.dwFlags      = VMRBITMAP_HDC | VMRBITMAP_SRCCOLORKEY;
                m_VMR9AlphaBitmap.hdc          = m_MemDC;
                m_VMR9AlphaBitmap.pDDS         = NULL;
                m_VMR9AlphaBitmap.rSrc         = m_rectWnd;
                m_VMR9AlphaBitmap.rDest.left   = 0.0f;
                m_VMR9AlphaBitmap.rDest.top    = 0.0f;
                m_VMR9AlphaBitmap.rDest.right  = 1.0f;
                m_VMR9AlphaBitmap.rDest.bottom = 1.0f;
                m_VMR9AlphaBitmap.fAlpha       = 1.0f;
                m_VMR9AlphaBitmap.clrSrcKey    = m_OSDColor[OSD_TRANSPARENT];
                m_VMR9AlphaBitmap.dwFilterMode = 0;
            } else if (m_pMFVMB) {
                // Configure the MF type bitmap structure
                m_MFVideoAlphaBitmap.GetBitmapFromDC       = TRUE;
                m_MFVideoAlphaBitmap.bitmap.hdc            = m_MemDC;
                m_MFVideoAlphaBitmap.params.dwFlags        = MFVideoAlphaBitmap_SrcColorKey;
                m_MFVideoAlphaBitmap.params.clrSrcKey      = m_OSDColor[OSD_TRANSPARENT];
                m_MFVideoAlphaBitmap.params.rcSrc          = m_rectWnd;
                m_MFVideoAlphaBitmap.params.nrcDest.left   = 0.0f;
                m_MFVideoAlphaBitmap.params.nrcDest.top    = 0.0f;
                m_MFVideoAlphaBitmap.params.nrcDest.right  = 1.0f;
                m_MFVideoAlphaBitmap.params.nrcDest.bottom = 1.0f;
                m_MFVideoAlphaBitmap.params.fAlpha         = 1.0f;
                m_MFVideoAlphaBitmap.params.dwFilterMode   = 0;
            }
            m_MemDC.SetTextColor(m_OSDColor[OSD_TEXT]);
            m_MemDC.SetBkMode(TRANSPARENT);
        }

        if (m_MainFont.GetSafeHandle()) {
            m_MemDC.SelectObject(m_MainFont);
        }

        DeleteObject(hbmpRender);
    } else { ASSERT(0); }
}

void CVMROSD::CalcRect()
{
    if (m_hWnd) {
        BOOL b = GetClientRect(m_hWnd, &m_rectWnd);
        ASSERT(b);

        m_rectSeekBar.left      = m_rectWnd.left;
        m_rectSeekBar.right     = m_rectWnd.right;
        m_rectSeekBar.top       = m_rectWnd.bottom  - SEEKBAR_HEIGHT;
        m_rectSeekBar.bottom    = m_rectSeekBar.top + SEEKBAR_HEIGHT;
    }
}

void CVMROSD::UpdateSeekBarPos(CPoint point)
{
    m_s64SeekPos = (point.x - m_rectBar.left) * (m_s64SeekMax - m_s64SeekMin) / (m_rectBar.Width() - SLIDER_CURSOR_WIDTH);
    if (m_s64SeekPos < m_s64SeekMin) { m_s64SeekPos = m_s64SeekMin; }
    else if (m_s64SeekPos > m_s64SeekMax) { m_s64SeekPos = m_s64SeekMax; }

    if (m_hWnd) {
        BOOL b = m_pMainWindow->PostMessage(WM_HSCROLL, (m_s64SeekPos & 0xffff) | (SB_THUMBTRACK << 16), reinterpret_cast<LPARAM>(m_hWnd));
        ASSERT(b);
    }
}

void CVMROSD::DrawSlider(RECT const* rect, __int64 llMin, __int64 llMax, __int64 llPos)
{
    TRACE(L"OSD DrawSlider\n");
    ASSERT(rect->bottom);
    ASSERT(rect->right);

    LONG lHalf = rect->top + ((rect->bottom - rect->top) >> 1);
    m_rectBar.left      = rect->left  + 1;
    m_rectBar.right     = rect->right - 1;
    m_rectBar.top       = lHalf - (SLIDER_BAR_HEIGHT >> 1);
    m_rectBar.bottom    = m_rectBar.top + SLIDER_BAR_HEIGHT;

    if (llMax == llMin) {
        m_rectCursor.left   = m_rectBar.left;
    } else {
        m_rectCursor.left   = m_rectBar.left + LONG((m_rectBar.Width() - SLIDER_CURSOR_WIDTH) * llPos / (llMax - llMin));
    }
    m_rectCursor.right      = m_rectCursor.left + SLIDER_CURSOR_WIDTH;
    m_rectCursor.top        = lHalf - (SLIDER_CURSOR_HEIGHT >> 1);
    m_rectCursor.bottom     = m_rectCursor.top + SLIDER_CURSOR_HEIGHT;

    DrawRect(rect, &m_brushBack, &m_penBorder);
    DrawRect(&m_rectBar, &m_brushBar);
    DrawRect(&m_rectCursor, NULL, &m_penCursor);
}

void CVMROSD::DrawRect(RECT const* rect, CBrush* pBrush, CPen* pPen)
{
    ASSERT(rect->bottom);
    ASSERT(rect->right);

    if (pPen) {
        m_MemDC.SelectObject(pPen);
    } else {
        m_MemDC.SelectStockObject(NULL_PEN);
    }

    if (pBrush) {
        m_MemDC.SelectObject(pBrush);
    } else {
        m_MemDC.SelectStockObject(HOLLOW_BRUSH);
    }

    BOOL b = m_MemDC.Rectangle(rect);
    ASSERT(b);
}

void CVMROSD::DrawMessage()
{
    ASSERT(m_nMessagePos != OSD_NOMESSAGE);
    ASSERT(!m_strMessage.IsEmpty());
    TRACE(L"OSD DrawMessage: %s\n", m_strMessage);

    RECT rectText;
    ZeroMemory(&rectText, sizeof(RECT));
    m_MemDC.DrawTextW(m_strMessage, &rectText, DT_CALCRECT);

    LONG lTh = rectText.bottom + 10;
    if (lTh > m_rectWnd.bottom) lTh = m_rectWnd.bottom;
    rectText.bottom = lTh;// messages always go on top

    LONG lTw = rectText.right + 20;
    DWORD dwFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX;
    if (lTw > m_rectWnd.right) {
        m_strMessage = L' ' + m_strMessage;
        dwFormat |= DT_END_ELLIPSIS;        
        lTw = m_rectWnd.right;
    }
    switch (m_nMessagePos) {
        case OSD_TOPLEFT :
            rectText.right = lTw;
            break;

        // case OSD_TOPRIGHT :
        default :
            rectText.right = m_rectWnd.right;
            rectText.left  = m_rectWnd.right - lTw;
            break;
    }

    DrawRect(&rectText, &m_brushBack, &m_penBorder);
    m_MemDC.DrawTextW(m_strMessage, &rectText, dwFormat);
}

void CVMROSD::DrawDebug()
{
    ASSERT(!m_debugMessages.IsEmpty());
    TRACE(L"OSD DrawDebug: %s\n", m_debugMessages);

    POSITION pos = m_debugMessages.GetHeadPosition();
    CStringW msg(m_debugMessages.GetNext(pos));
    while (pos) {
        CStringW const &tmp = m_debugMessages.GetNext(pos);
        if (!tmp.IsEmpty()) {
            msg += L"\r\n";
            msg.Append(tmp);
        }
    }

    RECT rectText;
    ZeroMemory(&rectText, sizeof(RECT));
    m_MemDC.DrawTextW(msg, &rectText, DT_CALCRECT);

    LONG cWw = m_rectWnd.Width() >> 1,
         cWh = m_rectWnd.Height() >> 1,
         cTw = ((rectText.right - rectText.left) >> 1) + 20,
         cTh = ((rectText.bottom - rectText.top) >> 1) + 10;

    rectText.left   = cWw - cTw - 10;
    rectText.top    = cWh - cTh - 10,
    rectText.right  = cWw + cTw + 10,
    rectText.bottom = cWh + cTh + 10;

    DrawRect(&rectText, &m_debugBrushBack, &m_debugPenBorder);
    m_MemDC.DrawTextW(msg, &rectText, DT_CENTER | DT_VCENTER);
}

void CVMROSD::Invalidate()
{
    if (!m_bOSDSuppressed && m_BitmapInfo.bmWidth && m_BitmapInfo.bmHeight && (m_BitmapInfo.bmBitsPixel&~7)) {// x86 asm 'test' will test the higher bits of bmBitsPixel this way, discarding the interval [0, 7]
        if (m_bSeekBarVisible || ((m_nMessagePos != OSD_NOMESSAGE) && !m_strMessage.IsEmpty()) || !m_debugMessages.IsEmpty()) {// test for activity

            m_bOSDVisible = true;
            // clear the image to transparent
            __declspec(align(16)) static __int32 const iFillVal[4] = {0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000};
            __m128 mFillVal = _mm_load_ps(reinterpret_cast<float const*>(iFillVal));
#ifdef _M_X64// also pre-load the smaller fill values in gpr registers
            __int64 sFillValPad = 0xFF000000FF000000;
#elif _M_IX86_FP != 1// SSE2 code, don't use on SSE builds, works correctly for AVX
            __int32 sFillValPad = 0xFF000000;
#endif
            uintptr_t pDst = reinterpret_cast<uintptr_t>(m_BitmapInfo.bmBits);
            ULONG ulCount = (m_BitmapInfo.bmWidth * m_BitmapInfo.bmHeight * m_BitmapInfo.bmBitsPixel) >> 5;// expression in 4-byte units
            // the GDI functions will align its surfaces at system granulatity
            ASSERT(!(pDst & 15)); // if not 16-byte aligned, _mm_stream_ps will fail

            // excludes the last the last 3 optional values (in the bit shift), as the next function only targets 128-bit fills
            if (ULONG i = ulCount >> 2) do {
                    _mm_stream_ps(reinterpret_cast<float*>(pDst), mFillVal);
                    pDst += 16;
                } while (--i);

            if (ulCount & 2) { // finalize the last 3 optional values, sorted for aligned access
#ifdef _M_X64
                _mm_stream_si64x(reinterpret_cast<__int64*>(pDst), sFillValPad);
#elif _M_IX86_FP != 1// SSE2 code, don't use on SSE builds, works correctly for AVX
                _mm_stream_si32(reinterpret_cast<__int32*>(pDst), sFillValPad);
                _mm_stream_si32(reinterpret_cast<__int32*>(pDst + 4), sFillValPad);
#else
                _mm_storel_pi(reinterpret_cast<__m64*>(pDst), mFillVal); // not related to MMX
#endif
                pDst += 8;
            }
            if (ulCount & 1) { // no address increment for the last possible value
#if _M_IX86_FP != 1// SSE2 code, don't use on SSE builds, works correctly for x64 and AVX
                _mm_stream_si32(reinterpret_cast<__int32*>(pDst), sFillValPad);// x64: copies lower bytes in the register
#else
                _mm_store_ss(reinterpret_cast<float*>(pDst), mFillVal);
#endif
            }

            if (m_bSeekBarVisible) {
                DrawSlider(&m_rectSeekBar, m_s64SeekMin, m_s64SeekMax, m_s64SeekPos);
            }
            if ((m_nMessagePos != OSD_NOMESSAGE) && !m_strMessage.IsEmpty()) {
                DrawMessage();
            }
            if (!m_debugMessages.IsEmpty()) {
                DrawDebug();
            }

            if (m_pVMB) {
                m_VMR9AlphaBitmap.dwFlags &= ~VMRBITMAP_DISABLE;
                m_pVMB->SetAlphaBitmap(&m_VMR9AlphaBitmap);
            } else if (m_pMFVMB) {
                m_pMFVMB->SetAlphaBitmap(&m_MFVideoAlphaBitmap);
            }
        } else {// for in case the working set is empty
            ClearMessage();
        }
    }
}
