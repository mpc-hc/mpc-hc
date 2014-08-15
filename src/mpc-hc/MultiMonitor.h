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

/*
 * David Campbell's article
 * How to Exploit Multiple Monitor Support in Memphis and Windows NT 5.0
 * is very helpful for multimonitor api calls
 * http://www.microsoft.com/msj/defaultframe.asp?page=/msj/0697/monitor/monitor.htm&nav=/msj/0697/newnav.htm
*/

// CMonitor

#pragma once

class CMonitor : public CObject
{
public:
    //construction destruction
    CMonitor();
    CMonitor(const CMonitor& monitor);
    virtual ~CMonitor();

    //operations
    void Attach(const HMONITOR hMonitor);
    HMONITOR Detach();

    void ClipRectToMonitor(LPRECT lprc, const BOOL UseWorkAreaRect = FALSE) const;
    void CenterRectToMonitor(LPRECT lprc, const BOOL UseWorkAreaRect = FALSE) const;
    void CenterWindowToMonitor(CWnd* const pWnd, const BOOL UseWorkAreaRect = FALSE) const;

    HDC CreateDC() const;

    //properties
    void GetMonitorRect(LPRECT lprc) const;
    void GetWorkAreaRect(LPRECT lprc) const;

    void GetName(CString& string) const;

    int GetBitsPerPixel() const;

    BOOL IsOnMonitor(const POINT& pt) const;
    BOOL IsOnMonitor(const CWnd* pWnd) const;
    BOOL IsOnMonitor(const LPRECT lprc) const;

    BOOL IsPrimaryMonitor() const;
    BOOL IsMonitor() const;

    //operators
    operator HMONITOR() const {
        return this == nullptr ? nullptr : m_hMonitor;
    }

    BOOL operator ==(const CMonitor& monitor) const {
        return m_hMonitor == (HMONITOR)monitor;
    }

    BOOL operator !=(const CMonitor& monitor) const {
        return !(*this == monitor);
    }

    CMonitor& operator =(const CMonitor& monitor) {
        m_hMonitor = (HMONITOR)monitor;
        return *this;
    }

private:
    HMONITOR m_hMonitor;

};
