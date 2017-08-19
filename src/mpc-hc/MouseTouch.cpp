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

#include "stdafx.h"
#include "MouseTouch.h"
#include "MainFrm.h"
#include "mplayerc.h"
#include "FullscreenWnd.h"
#include <mvrInterfaces.h>

#define CURSOR_HIDE_TIMEOUT 2000

CMouse::CMouse(CMainFrame* pMainFrm, bool bD3DFS/* = false*/)
    : m_bD3DFS(bD3DFS)
    , m_pMainFrame(pMainFrm)
    , m_dwMouseHiderStartTick(0)
    , m_bLeftDoubleStarted(false)
    , m_leftDoubleStartTime(0)
    , m_popupMenuUninitTime(0)
{
    m_cursors[Cursor::NONE] = nullptr;
    m_cursors[Cursor::ARROW] = LoadCursor(nullptr, IDC_ARROW);
    m_cursors[Cursor::HAND] = LoadCursor(nullptr, IDC_HAND);
    ResetToBlankState();

    EventRouter::EventSelection evs;
    evs.insert(MpcEvent::SWITCHING_TO_FULLSCREEN);
    evs.insert(MpcEvent::SWITCHED_TO_FULLSCREEN);
    evs.insert(MpcEvent::SWITCHING_TO_FULLSCREEN_D3D);
    evs.insert(MpcEvent::SWITCHED_TO_FULLSCREEN_D3D);
    evs.insert(MpcEvent::MEDIA_LOADED);
    evs.insert(MpcEvent::CONTEXT_MENU_POPUP_UNINITIALIZED);
    evs.insert(MpcEvent::SYSTEM_MENU_POPUP_INITIALIZED);
    GetEventd().Connect(m_eventc, evs, std::bind(&CMouse::EventCallback, this, std::placeholders::_1));
}

CMouse::~CMouse()
{
    StopMouseHider();
}

UINT CMouse::GetMouseFlags()
{
    UINT nFlags = 0;
    nFlags |= GetKeyState(VK_CONTROL)  < 0 ? MK_CONTROL  : 0;
    nFlags |= GetKeyState(VK_LBUTTON)  < 0 ? MK_LBUTTON  : 0;
    nFlags |= GetKeyState(VK_MBUTTON)  < 0 ? MK_MBUTTON  : 0;
    nFlags |= GetKeyState(VK_RBUTTON)  < 0 ? MK_RBUTTON  : 0;
    nFlags |= GetKeyState(VK_SHIFT)    < 0 ? MK_SHIFT    : 0;
    nFlags |= GetKeyState(VK_XBUTTON1) < 0 ? MK_XBUTTON1 : 0;
    nFlags |= GetKeyState(VK_XBUTTON2) < 0 ? MK_XBUTTON2 : 0;
    return nFlags;
}

bool CMouse::CursorOnRootWindow(const CPoint& screenPoint, const CFrameWnd& frameWnd)
{
    bool ret = false;

    CWnd* pWnd = CWnd::WindowFromPoint(screenPoint);
    CWnd* pRoot = pWnd ? pWnd->GetAncestor(GA_ROOT) : nullptr;

    // tooltips are special case
    if (pRoot && pRoot == pWnd) {
        CString strClass;
        VERIFY(GetClassName(pRoot->m_hWnd, strClass.GetBuffer(256), 256));
        strClass.ReleaseBuffer();
        if (strClass == _T("tooltips_class32")) {
            CWnd* pTooltipOwner = pWnd->GetParent();
            pRoot = pTooltipOwner ? pTooltipOwner->GetAncestor(GA_ROOT) : nullptr;
        }
    }

    if (pRoot) {
        ret = (pRoot->m_hWnd == frameWnd.m_hWnd);
    } else {
        ASSERT(FALSE);
    }

    return ret;
}
bool CMouse::CursorOnWindow(const CPoint& screenPoint, const CWnd& wnd)
{
    bool ret = false;
    CWnd* pWnd = CWnd::WindowFromPoint(screenPoint);
    if (pWnd) {
        ret = (pWnd->m_hWnd == wnd.m_hWnd);
    } else {
        ASSERT(FALSE);
    }
    return ret;
}

bool CMouse::Dragging()
{
    return m_drag == Drag::DRAGGED;
}

void CMouse::ResetToBlankState()
{
    StopMouseHider();
    m_bLeftDown = false;
    m_bTrackingMouseLeave = false;
    m_drag = Drag::NO_DRAG;
    m_cursor = Cursor::ARROW;
    m_switchingToFullscreen.first = false;
}

void CMouse::StartMouseHider(const CPoint& screenPoint)
{
    ASSERT(!m_pMainFrame->IsInteractiveVideo());
    m_mouseHiderStartScreenPoint = screenPoint;
    if (!m_bMouseHiderStarted) {
        // periodic timer is used here intentionally, recreating timer after each mouse move is more expensive
        auto t = m_bD3DFS ? CMainFrame::Timer32HzSubscriber::CURSOR_HIDER_D3DFS : CMainFrame::Timer32HzSubscriber::CURSOR_HIDER;
        m_pMainFrame->m_timer32Hz.Subscribe(t, std::bind(&CMouse::MouseHiderCallback, this));
        m_bMouseHiderStarted = true;
    }
    m_dwMouseHiderStartTick = GetTickCount64();
}
void CMouse::StopMouseHider()
{
    auto t = m_bD3DFS ? CMainFrame::Timer32HzSubscriber::CURSOR_HIDER_D3DFS : CMainFrame::Timer32HzSubscriber::CURSOR_HIDER;
    m_pMainFrame->m_timer32Hz.Unsubscribe(t);
    m_bMouseHiderStarted = false;
}
void CMouse::MouseHiderCallback()
{
    ASSERT(!m_pMainFrame->IsInteractiveVideo());
    if (GetTickCount64() > m_dwMouseHiderStartTick + CURSOR_HIDE_TIMEOUT) {
        StopMouseHider();
        ASSERT(m_cursor != Cursor::NONE);
        m_cursor = Cursor::NONE;
        CPoint screenPoint;
        VERIFY(GetCursorPos(&screenPoint));
        m_hideCursorPoint = screenPoint;
        ::SetCursor(m_cursors[m_cursor]);
    }
}

void CMouse::StartMouseLeaveTracker()
{
    ASSERT(!m_pMainFrame->IsInteractiveVideo());
    TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, GetWnd() };
    if (TrackMouseEvent(&tme)) {
        m_bTrackingMouseLeave = true;
    } else {
        ASSERT(FALSE);
    }
}
void CMouse::StopMouseLeaveTracker()
{
    TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE | TME_CANCEL, GetWnd() };
    TrackMouseEvent(&tme);
    m_bTrackingMouseLeave = false;
}

CPoint CMouse::GetVideoPoint(const CPoint& point) const
{
    return m_bD3DFS ? point : CPoint(point - m_pMainFrame->m_wndView.GetVideoRect().TopLeft());
}

bool CMouse::IsOnFullscreenWindow() const
{
    bool bD3DFSActive = m_pMainFrame->IsD3DFullScreenMode();
    return (m_pMainFrame->m_fFullScreen && !bD3DFSActive) || (m_bD3DFS && bD3DFSActive);
}

bool CMouse::OnButton(UINT id, const CPoint& point, bool bOnFullscreen)
{
    bool ret = false;
    WORD cmd = AssignedToCmd(id, bOnFullscreen);
    if (cmd) {
        m_pMainFrame->PostMessage(WM_COMMAND, cmd);
        ret = true;
    }
    return ret;
}
bool CMouse::OnButton(UINT id, const CPoint& point)
{
    return OnButton(id, point, IsOnFullscreenWindow());
}

void CMouse::EventCallback(MpcEvent ev)
{
    CPoint screenPoint;
    VERIFY(GetCursorPos(&screenPoint));
    switch (ev) {
        case MpcEvent::SWITCHED_TO_FULLSCREEN:
        case MpcEvent::SWITCHED_TO_FULLSCREEN_D3D:
            m_switchingToFullscreen.first = false;
            break;
        case MpcEvent::SWITCHING_TO_FULLSCREEN:
        case MpcEvent::SWITCHING_TO_FULLSCREEN_D3D:
            m_switchingToFullscreen = std::make_pair(true, screenPoint);
        // no break
        case MpcEvent::MEDIA_LOADED:
            if (CursorOnWindow(screenPoint, GetWnd())) {
                SetCursor(screenPoint);
            }
            break;
        case MpcEvent::CONTEXT_MENU_POPUP_UNINITIALIZED:
            m_popupMenuUninitTime = GetMessageTime();
            break;
        case MpcEvent::SYSTEM_MENU_POPUP_INITIALIZED:
            if (!GetCapture() && CursorOnWindow(screenPoint, GetWnd())) {
                ::SetCursor(m_cursors[Cursor::ARROW]);
            }
            break;
        default:
            ASSERT(FALSE);
    }
}

// madVR compatibility layer for exclusive mode seekbar
bool CMouse::UsingMVR() const
{
    return !!m_pMainFrame->m_pMVRSR;
}
void CMouse::MVRMove(UINT nFlags, const CPoint& point)
{
    if (UsingMVR()) {
        WPARAM wp = nFlags;
        LPARAM lp = MAKELPARAM(point.x, point.y);
        LRESULT lr = 0;
        m_pMainFrame->m_pMVRSR->ParentWindowProc(GetWnd(), WM_MOUSEMOVE, &wp, &lp, &lr);
    }
}
bool CMouse::MVRDown(UINT nFlags, const CPoint& point)
{
    bool ret = false;
    if (UsingMVR()) {
        WPARAM wp = nFlags;
        LPARAM lp = MAKELPARAM(point.x, point.y);
        LRESULT lr = 0;
        ret = !!m_pMainFrame->m_pMVRSR->ParentWindowProc(GetWnd(), WM_LBUTTONDOWN, &wp, &lp, &lr);
    }
    return ret;
}
bool CMouse::MVRUp(UINT nFlags, const CPoint& point)
{
    bool ret = false;
    if (UsingMVR()) {
        WPARAM wp = nFlags;
        LPARAM lp = MAKELPARAM(point.x, point.y);
        LRESULT lr = 0;
        ret = !!m_pMainFrame->m_pMVRSR->ParentWindowProc(GetWnd(), WM_LBUTTONUP, &wp, &lp, &lr);
    }
    return ret;
}

// Left button
void CMouse::InternalOnLButtonDown(UINT nFlags, const CPoint& point)
{
    GetWnd().SetFocus();
    m_bLeftDown = false;
    SetCursor(nFlags, point);
    if (MVRDown(nFlags, point)) {
        return;
    }
    bool bIsOnFS = IsOnFullscreenWindow();
    if ((!m_bD3DFS || !bIsOnFS) && (abs(GetMessageTime() - m_popupMenuUninitTime) < 2)) {
        return;
    }
    if (m_pMainFrame->GetLoadState() == MLS::LOADED && m_pMainFrame->GetPlaybackMode() == PM_DVD &&
            (m_pMainFrame->IsD3DFullScreenMode() ^ m_bD3DFS) == 0 &&
            (m_pMainFrame->m_pDVDC->ActivateAtPosition(GetVideoPoint(point)) == S_OK)) {
        return;
    }
    if (m_bD3DFS && bIsOnFS && m_pMainFrame->m_OSD.OnLButtonDown(nFlags, point)) {
        return;
    }
    m_bLeftDown = true;
    bool bDouble = false;
    if (m_bLeftDoubleStarted &&
            GetMessageTime() - m_leftDoubleStartTime < (int)GetDoubleClickTime() &&
            CMouse::PointEqualsImprecise(m_leftDoubleStartPoint, point,
                                         GetSystemMetrics(SM_CXDOUBLECLK) / 2, GetSystemMetrics(SM_CYDOUBLECLK) / 2)) {
        m_bLeftDoubleStarted = false;
        bDouble = true;
    } else {
        m_bLeftDoubleStarted = true;
        m_leftDoubleStartTime = GetMessageTime();
        m_leftDoubleStartPoint = point;
    }
    auto onButton = [&]() {
        GetWnd().SetCapture();
        bool ret = false;
        if (bIsOnFS || !m_pMainFrame->IsCaptionHidden()) {
            ret = OnButton(wmcmd::LDOWN, point, bIsOnFS);
        }
        if (bDouble) {
            ret = OnButton(wmcmd::LDBLCLK, point, bIsOnFS) || ret;
        }
        if (!ret) {
            ReleaseCapture();
        }
        return ret;
    };
    m_drag = (!onButton() && !bIsOnFS) ? Drag::BEGIN_DRAG : Drag::NO_DRAG;
    if (m_drag == Drag::BEGIN_DRAG) {
        GetWnd().SetCapture();
        m_beginDragPoint = point;
        GetWnd().ClientToScreen(&m_beginDragPoint);
    }
}
void CMouse::InternalOnLButtonUp(UINT nFlags, const CPoint& point)
{
    ReleaseCapture();
    if (!MVRUp(nFlags, point)) {
        bool bIsOnFS = IsOnFullscreenWindow();
        if (!(m_bD3DFS && bIsOnFS && m_pMainFrame->m_OSD.OnLButtonUp(nFlags, point)) && m_bLeftDown) {
            OnButton(wmcmd::LUP, point, bIsOnFS);
        }
    }
    m_drag = Drag::NO_DRAG;
    m_bLeftDown = false;
    SetCursor(nFlags, point);
}

// Middle button
void CMouse::InternalOnMButtonDown(UINT nFlags, const CPoint& point)
{
    SetCursor(nFlags, point);
    OnButton(wmcmd::MDOWN, point);
}
void CMouse::InternalOnMButtonUp(UINT nFlags, const CPoint& point)
{
    OnButton(wmcmd::MUP, point);
    SetCursor(nFlags, point);
}
void CMouse::InternalOnMButtonDblClk(UINT nFlags, const CPoint& point)
{
    SetCursor(nFlags, point);
    OnButton(wmcmd::MDOWN, point);
    OnButton(wmcmd::MDBLCLK, point);
}

// Right button
void CMouse::InternalOnRButtonDown(UINT nFlags, const CPoint& point)
{
    SetCursor(nFlags, point);
    OnButton(wmcmd::RDOWN, point);
}
void CMouse::InternalOnRButtonUp(UINT nFlags, const CPoint& point)
{
    OnButton(wmcmd::RUP, point);
    SetCursor(nFlags, point);
}
void CMouse::InternalOnRButtonDblClk(UINT nFlags, const CPoint& point)
{
    SetCursor(nFlags, point);
    OnButton(wmcmd::RDOWN, point);
    OnButton(wmcmd::RDBLCLK, point);
}

// Navigation buttons
bool CMouse::InternalOnXButtonDown(UINT nFlags, UINT nButton, const CPoint& point)
{
    SetCursor(nFlags, point);
    return OnButton(nButton == XBUTTON1 ? wmcmd::X1DOWN : nButton == XBUTTON2 ? wmcmd::X2DOWN : wmcmd::NONE, point);
}
bool CMouse::InternalOnXButtonUp(UINT nFlags, UINT nButton, const CPoint& point)
{
    bool ret = OnButton(nButton == XBUTTON1 ? wmcmd::X1UP : nButton == XBUTTON2 ? wmcmd::X2UP : wmcmd::NONE, point);
    SetCursor(nFlags, point);
    return ret;
}
bool CMouse::InternalOnXButtonDblClk(UINT nFlags, UINT nButton, const CPoint& point)
{
    InternalOnXButtonDown(nFlags, nButton, point);
    return OnButton(nButton == XBUTTON1 ? wmcmd::X1DBLCLK : nButton == XBUTTON2 ? wmcmd::X2DBLCLK : wmcmd::NONE, point);
}

BOOL CMouse::InternalOnMouseWheel(UINT nFlags, short zDelta, const CPoint& point)
{
    return zDelta > 0 ? OnButton(wmcmd::WUP, point) :
           zDelta < 0 ? OnButton(wmcmd::WDOWN, point) :
           FALSE;
}

bool CMouse::SelectCursor(const CPoint& screenPoint, const CPoint& clientPoint, UINT nFlags)
{
    const auto& s = AfxGetAppSettings();

    if (m_bD3DFS && m_pMainFrame->m_OSD.OnMouseMove(nFlags, clientPoint)) {
        StopMouseHider();
        m_cursor = Cursor::HAND;
        return true;
    }

    if (m_pMainFrame->GetLoadState() == MLS::LOADED && m_pMainFrame->GetPlaybackMode() == PM_DVD &&
            (m_pMainFrame->IsD3DFullScreenMode() ^ m_bD3DFS) == 0 &&
            (m_pMainFrame->m_pDVDC->SelectAtPosition(GetVideoPoint(clientPoint)) == S_OK)) {
        StopMouseHider();
        m_cursor = Cursor::HAND;
        return true;
    }

    bool bMouseButtonDown = !!(nFlags & ~(MK_CONTROL | MK_SHIFT));
    bool bHidden = (m_cursor == Cursor::NONE);
    bool bHiddenAndMoved = bHidden && !PointEqualsImprecise(screenPoint, m_hideCursorPoint);
    bool bCanHide = !bMouseButtonDown &&
                    (m_pMainFrame->GetLoadState() == MLS::LOADED || m_pMainFrame->m_controls.DelayShowNotLoaded()) &&
                    !m_pMainFrame->IsInteractiveVideo() &&
                    (m_switchingToFullscreen.first || IsOnFullscreenWindow() ||
                     (s.bHideWindowedMousePointer && !(m_pMainFrame->IsD3DFullScreenMode() ^ m_bD3DFS)));

    if (m_switchingToFullscreen.first) {
        if (bCanHide && PointEqualsImprecise(screenPoint, m_switchingToFullscreen.second)) {
            StopMouseHider();
            m_cursor = Cursor::NONE;
            m_hideCursorPoint = screenPoint;
            return true;
        }
    }

    if (!bHidden || bHiddenAndMoved || !bCanHide) {
        m_cursor = Cursor::ARROW;
        if (bCanHide) {
            if (!m_bMouseHiderStarted || screenPoint != m_mouseHiderStartScreenPoint) {
                StartMouseHider(screenPoint);
            }
        } else if (m_bMouseHiderStarted) {
            StopMouseHider();
        }
    }

    return true;
}

void CMouse::SetCursor(UINT nFlags, const CPoint& screenPoint, const CPoint& clientPoint)
{
    if (SelectCursor(screenPoint, clientPoint, nFlags) && !m_pMainFrame->IsInteractiveVideo()) {
        ::SetCursor(m_cursors[m_cursor]);
    }
}
void CMouse::SetCursor(UINT nFlags, const CPoint& clientPoint)
{
    CPoint screenPoint(clientPoint);
    GetWnd().ClientToScreen(&screenPoint);
    SetCursor(nFlags, screenPoint, clientPoint);
}

void CMouse::SetCursor(const CPoint& screenPoint)
{
    ASSERT(CursorOnWindow(screenPoint, GetWnd()));
    CPoint clientPoint(screenPoint);
    GetWnd().ScreenToClient(&clientPoint);
    SetCursor(GetMouseFlags(), screenPoint, clientPoint);
}

bool CMouse::TestDrag(const CPoint& screenPoint)
{
    bool ret = false;
    if (m_drag == Drag::BEGIN_DRAG) {
        ASSERT(!IsOnFullscreenWindow());
        bool bUpAssigned = !!AssignedToCmd(wmcmd::LUP, false);
        if ((!bUpAssigned && screenPoint != m_beginDragPoint) ||
                (bUpAssigned && !PointEqualsImprecise(screenPoint, m_beginDragPoint,
                                                      GetSystemMetrics(SM_CXDRAG), GetSystemMetrics(SM_CYDRAG)))) {
            VERIFY(ReleaseCapture());
            m_pMainFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(m_beginDragPoint.x, m_beginDragPoint.y));
            m_drag = Drag::DRAGGED;
            m_bLeftDown = false;
            ret = true;
        }
    } else {
        m_drag = Drag::NO_DRAG;
    }
    return ret;
}

BOOL CMouse::InternalOnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    return nHitTest == HTCLIENT;
}

void CMouse::InternalOnMouseMove(UINT nFlags, const CPoint& point)
{
    CPoint screenPoint(point);
    GetWnd().ClientToScreen(&screenPoint);

    if (!TestDrag(screenPoint) && !m_pMainFrame->IsInteractiveVideo()) {
        if (!m_bTrackingMouseLeave) {
            StartMouseLeaveTracker();
        }
        SetCursor(nFlags, screenPoint, point);
        MVRMove(nFlags, point);
    }

    m_pMainFrame->UpdateControlState(CMainFrame::UPDATE_CONTROLS_VISIBILITY);
}

void CMouse::InternalOnMouseLeave()
{
    StopMouseHider();
    m_bTrackingMouseLeave = false;
    m_cursor = Cursor::ARROW;
    if (m_bD3DFS) {
        m_pMainFrame->m_OSD.OnMouseLeave();
    }
}

void CMouse::InternalOnDestroy()
{
    ResetToBlankState();
}

CMouseWnd::CMouseWnd(CMainFrame* pMainFrm, bool bD3DFS/* = false*/)
    : CMouse(pMainFrm, bD3DFS)
{
}

IMPLEMENT_DYNAMIC(CMouseWnd, CWnd)
BEGIN_MESSAGE_MAP(CMouseWnd, CWnd)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_MBUTTONDOWN()
    ON_WM_MBUTTONUP()
    ON_WM_MBUTTONDBLCLK()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_RBUTTONDBLCLK()
    ON_WM_XBUTTONDOWN()
    ON_WM_XBUTTONUP()
    ON_WM_XBUTTONDBLCLK()
    ON_WM_MOUSEWHEEL()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

void CMouseWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    CMouse::InternalOnLButtonDown(nFlags, point);
}
void CMouseWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
    CMouse::InternalOnLButtonUp(nFlags, point);
}
void CMouseWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CMouse::InternalOnLButtonDown(nFlags, point);
}

void CMouseWnd::OnMButtonDown(UINT nFlags, CPoint point)
{
    CMouse::InternalOnMButtonDown(nFlags, point);
}
void CMouseWnd::OnMButtonUp(UINT nFlags, CPoint point)
{
    CMouse::InternalOnMButtonUp(nFlags, point);
}
void CMouseWnd::OnMButtonDblClk(UINT nFlags, CPoint point)
{
    CMouse::InternalOnMButtonDblClk(nFlags, point);
}

void CMouseWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
    CMouse::InternalOnRButtonDown(nFlags, point);
}
void CMouseWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
    CMouse::InternalOnRButtonUp(nFlags, point);
}
void CMouseWnd::OnRButtonDblClk(UINT nFlags, CPoint point)
{
    CMouse::InternalOnRButtonDblClk(nFlags, point);
}

void CMouseWnd::OnXButtonDown(UINT nFlags, UINT nButton, CPoint point)
{
    if (!CMouse::InternalOnXButtonDown(nFlags, nButton, point)) {
        CWnd::OnXButtonDown(nFlags, nButton, point);
    }
}
void CMouseWnd::OnXButtonUp(UINT nFlags, UINT nButton, CPoint point)
{
    if (!CMouse::InternalOnXButtonUp(nFlags, nButton, point)) {
        CWnd::OnXButtonUp(nFlags, nButton, point);
    }
}
void CMouseWnd::OnXButtonDblClk(UINT nFlags, UINT nButton, CPoint point)
{
    if (!CMouse::InternalOnXButtonDblClk(nFlags, nButton, point)) {
        CWnd::OnXButtonDblClk(nFlags, nButton, point);
    }
}

BOOL CMouseWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
    return CMouse::InternalOnMouseWheel(nFlags, zDelta, point);
}

BOOL CMouseWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    return CMouse::InternalOnSetCursor(pWnd, nHitTest, message) ||
           CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CMouseWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    CMouse::InternalOnMouseMove(nFlags, point);
    CWnd::OnMouseMove(nFlags, point);
}

void CMouseWnd::OnMouseLeave()
{
    CMouse::InternalOnMouseLeave();
    CWnd::OnMouseLeave();
}

void CMouseWnd::OnDestroy()
{
    CMouse::InternalOnDestroy();
    CWnd::OnDestroy();
}

std::unordered_set<const CWnd*> CMainFrameMouseHook::GetRoots()
{
    std::unordered_set<const CWnd*> ret;
    const CMainFrame* pMainFrame = AfxGetMainFrame();
    ASSERT(pMainFrame);
    if (pMainFrame) {
        ret.emplace(pMainFrame);
        if (pMainFrame->IsD3DFullScreenMode()) {
            ret.emplace(pMainFrame->m_pFullscreenWnd);
        }
    }
    return ret;
}
