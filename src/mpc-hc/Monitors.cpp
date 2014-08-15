/*
 * Author: Donald Kackman
 * Email: don@itsEngineering.com
 * Copyright 2002, Donald Kackman
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
#include "Monitors.h"


// CMonitors

CMonitors::CMonitors()
{
    // WARNING : GetSystemMetrics(SM_CMONITORS) return only visible display monitors, and  EnumDisplayMonitors
    // enumerate visible and pseudo invisible monitors !!!
    //  m_MonitorArray.SetSize( GetMonitorCount() );

    ADDMONITOR addMonitor;
    addMonitor.pMonitors = &m_MonitorArray;
    addMonitor.currentIndex = 0;

    ::EnumDisplayMonitors(nullptr, nullptr, AddMonitorsCallBack, (LPARAM)&addMonitor);
}

CMonitors::~CMonitors()
{
    for (int i = 0; i < m_MonitorArray.GetSize(); i++) {
        delete m_MonitorArray.GetAt(i);
    }
}


// CMonitors member functions

BOOL CALLBACK CMonitors::AddMonitorsCallBack(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    LPADDMONITOR pAddMonitor = (LPADDMONITOR)dwData;

    CMonitor* pMonitor = DEBUG_NEW CMonitor;
    pMonitor->Attach(hMonitor);

    pAddMonitor->pMonitors->Add(pMonitor);
    pAddMonitor->currentIndex++;

    return TRUE;
}
//
// returns the primary monitor
CMonitor CMonitors::GetPrimaryMonitor()
{
    //the primary monitor always has its origin at 0,0
    HMONITOR hMonitor = ::MonitorFromPoint(CPoint(0, 0), MONITOR_DEFAULTTOPRIMARY);
    ASSERT(IsMonitor(hMonitor));

    CMonitor monitor;
    monitor.Attach(hMonitor);
    ASSERT(monitor.IsPrimaryMonitor());

    return monitor;
}

//
// is the given handle a valid monitor handle
BOOL CMonitors::IsMonitor(const HMONITOR hMonitor)
{
    if (hMonitor == nullptr) {
        return FALSE;
    }

    MATCHMONITOR match;
    match.target = hMonitor;
    match.foundMatch = FALSE;

    ::EnumDisplayMonitors(nullptr, nullptr, FindMatchingMonitorHandle, (LPARAM)&match);

    return match.foundMatch;
}

//this is the callback method that gets called via IsMontior
BOOL CALLBACK CMonitors::FindMatchingMonitorHandle(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    LPMATCHMONITOR pMatch = (LPMATCHMONITOR)dwData;

    if (hMonitor == pMatch->target) {
        //found a monitor with the same handle we are looking for
        pMatch->foundMatch = TRUE;
        return FALSE; //stop enumerating
    }

    //haven't found a match yet
    pMatch->foundMatch = FALSE;
    return TRUE;    //keep enumerating
}

BOOL CMonitors::AllMonitorsShareDisplayFormat()
{
    return ::GetSystemMetrics(SM_SAMEDISPLAYFORMAT);
}

//
// the number of monitors on the system
int CMonitors::GetMonitorCount()
{
    return ::GetSystemMetrics(SM_CMONITORS);
}

CMonitor CMonitors::GetMonitor(const int index) const
{
    ASSERT(index >= 0 && index < m_MonitorArray.GetCount());

    CMonitor* pMonitor = static_cast<CMonitor*>(m_MonitorArray.GetAt(index));

    return *pMonitor;
}

//
// returns the rectangle that is the union of all active monitors
void CMonitors::GetVirtualDesktopRect(LPRECT lprc)
{
    ::SetRect(lprc,
              ::GetSystemMetrics(SM_XVIRTUALSCREEN),
              ::GetSystemMetrics(SM_YVIRTUALSCREEN),
              ::GetSystemMetrics(SM_CXVIRTUALSCREEN),
              ::GetSystemMetrics(SM_CYVIRTUALSCREEN));

}

//
// these methods determine whether the given item is
// visible on any monitor
BOOL CMonitors::IsOnScreen(const LPRECT lprc)
{
    return ::MonitorFromRect(lprc, MONITOR_DEFAULTTONULL) != nullptr;
}

BOOL CMonitors::IsOnScreen(const POINT& pt)
{
    return ::MonitorFromPoint(pt, MONITOR_DEFAULTTONULL) != nullptr;
}

BOOL CMonitors::IsOnScreen(const CWnd* pWnd)
{
    return ::MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONULL) != nullptr;
}

CMonitor CMonitors::GetNearestMonitor(const LPRECT lprc)
{
    CMonitor monitor;
    monitor.Attach(::MonitorFromRect(lprc, MONITOR_DEFAULTTONEAREST));

    return monitor;

}

CMonitor CMonitors::GetNearestMonitor(const POINT& pt)
{
    CMonitor monitor;
    monitor.Attach(::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST));

    return monitor;
}

CMonitor CMonitors::GetNearestMonitor(const CWnd* pWnd)
{
    ASSERT(pWnd);
    ASSERT(::IsWindow(pWnd->m_hWnd));

    CMonitor monitor;
    monitor.Attach(::MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST));

    return monitor;
}
