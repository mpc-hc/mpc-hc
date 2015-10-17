/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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
#include "FocusThread.h"

IMPLEMENT_DYNCREATE(CFocusThread, CWinThread)

LRESULT CALLBACK FocusWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCACTIVATE) {
        if (wp) {
            AfxGetMainWnd()->SetForegroundWindow();
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

CFocusThread::CFocusThread()
    : m_hWnd(nullptr)
    , m_hEvtInit(nullptr)
{
    WNDCLASS wndclass {
        CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE,
        FocusWndProc,
        0,
        0,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        _T("D3DFocusClass")
    };

    if (!RegisterClass(&wndclass)) {
        TRACE("Registering focus window failed");
    }

    m_hEvtInit = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

CFocusThread::~CFocusThread()
{
    SAFE_CLOSE_HANDLE(m_hEvtInit);
    UnregisterClass(_T("D3DFocusClass"), nullptr);
}

BOOL CFocusThread::InitInstance()
{
    SetThreadName(DWORD(-1), "FocusThread");
    m_hWnd = CreateWindow(_T("D3DFocusClass"), _T("D3D Focus Window"), WS_OVERLAPPED, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
    SetEvent(m_hEvtInit);
    if (!m_hWnd) {
        TRACE("Creating focus window failed");
        return FALSE;
    }
    return TRUE;
}

int CFocusThread::ExitInstance()
{
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;

        CWnd* pMainWnd = AfxGetApp()->GetMainWnd();
        if (pMainWnd) {
            pMainWnd->SetActiveWindow();
        }
    }
    return __super::ExitInstance();
}

HWND CFocusThread::GetFocusWindow()
{
    if (!m_hWnd) {
        WaitForSingleObject(m_hEvtInit, 10000);
    }
    return m_hWnd;
}
