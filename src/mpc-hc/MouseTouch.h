/*
 * (C) 2013-2015, 2017 see Authors.txt
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

#include <map>
#include <unordered_set>
#include <VersionHelpersInternal.h>

#include "EventDispatcher.h"

// TODO: handle touch gestures

class CMainFrame;

class CMouse
{
public:
    CMouse(CMainFrame* pMainFrm, bool bD3DFS = false);
    CMouse(const CMouse&) = delete;

    virtual ~CMouse();

    static inline bool PointEqualsImprecise(long a, long b, long lDelta) {
        return abs(a - b) <= abs(lDelta);
    }
    static inline bool PointEqualsImprecise(const CPoint& a, const CPoint& b, long xDelta = 1, long yDelta = 1) {
        return PointEqualsImprecise(a.x, b.x, xDelta) && PointEqualsImprecise(a.y, b.y, yDelta);
    }

    static UINT GetMouseFlags();

    static bool CursorOnRootWindow(const CPoint& screenPoint, const CFrameWnd& frameWnd);
    static bool CursorOnWindow(const CPoint& screenPoint, const CWnd& wnd);

    bool Dragging();

    CMouse& operator=(const CMouse&) = delete;

protected:
    const bool m_bD3DFS;

private:
    CMainFrame* m_pMainFrame;
    bool m_bMouseHiderStarted;
    CPoint m_mouseHiderStartScreenPoint;
    ULONGLONG m_dwMouseHiderStartTick;
    bool m_bTrackingMouseLeave;
    enum class Drag { NO_DRAG, BEGIN_DRAG, DRAGGED } m_drag;
    enum class Cursor { NONE, ARROW, HAND };
    std::map<Cursor, HCURSOR> m_cursors;
    Cursor m_cursor;
    CPoint m_beginDragPoint;
    CPoint m_hideCursorPoint;
    bool m_bLeftDown;
    bool m_bLeftDoubleStarted;
    CPoint m_leftDoubleStartPoint;
    int m_leftDoubleStartTime;
    int m_popupMenuUninitTime;

    std::pair<bool, CPoint> m_switchingToFullscreen;

    virtual CWnd& GetWnd() = 0;

    void ResetToBlankState();
    void StartMouseHider(const CPoint& screenPoint);
    void StopMouseHider();
    void MouseHiderCallback();
    void StartMouseLeaveTracker();
    void StopMouseLeaveTracker();

    CPoint GetVideoPoint(const CPoint& point) const;
    bool IsOnFullscreenWindow() const;
    bool OnButton(UINT id, const CPoint& point, bool bOnFullscreen);
    bool OnButton(UINT id, const CPoint& point);
    bool SelectCursor(const CPoint& screenPoint, const CPoint& clientPoint, UINT nFlags);
    void SetCursor(UINT nFlags, const CPoint& screenPoint, const CPoint& clientPoint);
    void SetCursor(UINT nFlags, const CPoint& clientPoint);
    void SetCursor(const CPoint& screenPoint);
    bool TestDrag(const CPoint& screenPoint);

    EventClient m_eventc;
    void EventCallback(MpcEvent ev);

    bool UsingMVR() const;
    void MVRMove(UINT nFlags, const CPoint& point);
    bool MVRDown(UINT nFlags, const CPoint& point);
    bool MVRUp(UINT nFlags, const CPoint& point);

protected:
    void InternalOnLButtonDown(UINT nFlags, const CPoint& point);
    void InternalOnLButtonUp(UINT nFlags, const CPoint& point);
    void InternalOnMButtonDown(UINT nFlags, const CPoint& point);
    void InternalOnMButtonUp(UINT nFlags, const CPoint& point);
    void InternalOnMButtonDblClk(UINT nFlags, const CPoint& point);
    void InternalOnRButtonDown(UINT nFlags, const CPoint& point);
    void InternalOnRButtonUp(UINT nFlags, const CPoint& point);
    void InternalOnRButtonDblClk(UINT nFlags, const CPoint& point);
    bool InternalOnXButtonDown(UINT nFlags, UINT nButton, const CPoint& point);
    bool InternalOnXButtonUp(UINT nFlags, UINT nButton, const CPoint& point);
    bool InternalOnXButtonDblClk(UINT nFlags, UINT nButton, const CPoint& point);
    BOOL InternalOnMouseWheel(UINT nFlags, short zDelta, const CPoint& point);
    BOOL InternalOnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    void InternalOnMouseMove(UINT nFlags, const CPoint& point);
    void InternalOnMouseLeave();
    void InternalOnDestroy();
};

class CMouseWnd : public CWnd, public CMouse
{
public:
    CMouseWnd(CMainFrame* pMainFrm, bool bD3DFS = false);

private:
    DECLARE_DYNAMIC(CMouseWnd)
    DECLARE_MESSAGE_MAP()

    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonUp(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    void OnMButtonDown(UINT nFlags, CPoint point);
    void OnMButtonUp(UINT nFlags, CPoint point);
    void OnMButtonDblClk(UINT nFlags, CPoint point);

    void OnRButtonDown(UINT nFlags, CPoint point);
    void OnRButtonUp(UINT nFlags, CPoint point);
    void OnRButtonDblClk(UINT nFlags, CPoint point);

    void OnXButtonDown(UINT nFlags, UINT nButton, CPoint point);
    void OnXButtonUp(UINT nFlags, UINT nButton, CPoint point);
    void OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point);

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);

    BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    void OnMouseMove(UINT nFlags, CPoint point);

    void OnMouseLeave();

    void OnDestroy();

    virtual CWnd& GetWnd() override final {
        return *this;
    }

    virtual ULONG GetGestureStatus(CPoint) override {
        return m_bD3DFS ? TABLET_DISABLE_PRESSANDHOLD : 0;
    }
};

template <class T>
class CMouseWheelHook
{
    HHOOK m_hHook = NULL;

    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION && wParam == WM_MOUSEWHEEL) {
            const auto& msex = *reinterpret_cast<MOUSEHOOKSTRUCTEX*>(lParam);
            if (const CWnd* pFocus = CWnd::FromHandlePermanent(msex.hwnd)) {
                if (const CWnd* pFocusRoot = pFocus->GetAncestor(GA_ROOT)) {
                    // only intercept messages to focused windows that have white-listed root windows
                    if (T::GetRoots().count(pFocusRoot)) {
                        if (const CWnd* pUnder = CWnd::WindowFromPoint(msex.pt)) {
                            if (pFocusRoot == pUnder->GetAncestor(GA_ROOT) &&
                                    GetCurrentThreadId() == GetWindowThreadProcessId(pUnder->m_hWnd, nullptr)) {
                                MSG msg = {
                                    NULL,
                                    static_cast<UINT>(wParam),
                                    CMouse::GetMouseFlags() | msex.mouseData,
                                    MAKELPARAM(msex.pt.x, msex.pt.y),
                                    static_cast<DWORD>(GetMessageTime()),
                                    msex.pt
                                };

                                for (const CWnd* pTarget : { pUnder, pFocusRoot }) {
                                    msg.hwnd = pTarget->m_hWnd;
                                    if (!msg.hwnd) {
                                        ASSERT(FALSE);
                                        continue;
                                    }

                                    // walk through pre-translate
                                    if (CWnd::WalkPreTranslateTree(pFocusRoot->m_hWnd, &msg)) {
                                        // the message shouldn't be dispatched
                                        continue;
                                    }

                                    if (DispatchMessage(&msg)) {
                                        return TRUE;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

public:
    CMouseWheelHook() {
        if (IsWindows10OrGreater()) {
            m_hHook = SetWindowsHookEx(WH_MOUSE, MouseProc, nullptr, GetCurrentThreadId());
            ASSERT(m_hHook);
        }
    }

    virtual ~CMouseWheelHook() {
        if (m_hHook) {
            VERIFY(UnhookWindowsHookEx(m_hHook));
        }
    }
};

struct CMainFrameMouseHook : CMouseWheelHook<CMainFrameMouseHook> {
    static std::unordered_set<const CWnd*> GetRoots();
};
