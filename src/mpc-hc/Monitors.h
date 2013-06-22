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

#pragma once

#include "MultiMonitor.h"

// CMonitors command target

class CMonitors : public CObject
{
public:
    CMonitors();
    virtual ~CMonitors();

    CMonitor GetMonitor(const int index) const;


    int GetCount() const {
        return (int)m_MonitorArray.GetCount();
    }

    //static members
    static CMonitor GetNearestMonitor(const LPRECT lprc);
    static CMonitor GetNearestMonitor(const POINT& pt);
    static CMonitor GetNearestMonitor(const CWnd* pWnd);

    static BOOL IsOnScreen(const POINT& pt);
    static BOOL IsOnScreen(const CWnd* pWnd);
    static BOOL IsOnScreen(const LPRECT lprc);

    static void GetVirtualDesktopRect(LPRECT lprc);

    static BOOL IsMonitor(const HMONITOR hMonitor);

    static CMonitor GetPrimaryMonitor();
    static BOOL AllMonitorsShareDisplayFormat();

    static int GetMonitorCount();

private:
    CObArray m_MonitorArray;

    typedef struct tagMATCHMONITOR {
        HMONITOR target;
        BOOL foundMatch;
    } MATCHMONITOR, *LPMATCHMONITOR;

    static BOOL CALLBACK FindMatchingMonitorHandle(
        HMONITOR hMonitor,  // handle to display monitor
        HDC hdcMonitor,     // handle to monitor DC
        LPRECT lprcMonitor, // monitor intersection rectangle
        LPARAM dwData       // data
    );


    typedef struct tagADDMONITOR {
        CObArray* pMonitors;
        int currentIndex;
    } ADDMONITOR, *LPADDMONITOR;

    static BOOL CALLBACK AddMonitorsCallBack(
        HMONITOR hMonitor,  // handle to display monitor
        HDC hdcMonitor,     // handle to monitor DC
        LPRECT lprcMonitor, // monitor intersection rectangle
        LPARAM dwData       // data
    );

};
