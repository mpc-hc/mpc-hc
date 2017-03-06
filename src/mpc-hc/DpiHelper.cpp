/*
 * (C) 2015-2016 see Authors.txt
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
#include "DpiHelper.h"

#include "WinapiFunc.h"

namespace
{
    typedef enum MONITOR_DPI_TYPE {
        MDT_EFFECTIVE_DPI = 0,
        MDT_ANGULAR_DPI = 1,
        MDT_RAW_DPI = 2,
        MDT_DEFAULT = MDT_EFFECTIVE_DPI
    } MONITOR_DPI_TYPE;

    HRESULT WINAPI GetDpiForMonitor(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
}

DpiHelper::DpiHelper()
{
    HDC hDC = ::GetDC(nullptr);
    m_sdpix = GetDeviceCaps(hDC, LOGPIXELSX);
    m_sdpiy = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(nullptr, hDC);
    m_dpix = m_sdpix;
    m_dpiy = m_sdpiy;
}

void DpiHelper::Override(HWND hWindow)
{
    const WinapiFunc<decltype(GetDpiForMonitor)>
    fnGetDpiForMonitor = { _T("Shcore.dll"), "GetDpiForMonitor" };

    if (hWindow && fnGetDpiForMonitor) {
        if (fnGetDpiForMonitor(MonitorFromWindow(hWindow, MONITOR_DEFAULTTONULL),
                               MDT_EFFECTIVE_DPI, (UINT*)&m_dpix, (UINT*)&m_dpiy) != S_OK) {
            m_dpix = m_sdpix;
            m_dpiy = m_sdpiy;
        }
    }
}

void DpiHelper::Override(int dpix, int dpiy)
{
    m_dpix = dpix;
    m_dpiy = dpiy;
}
