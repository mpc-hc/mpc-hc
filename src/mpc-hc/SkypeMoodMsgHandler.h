/*
 * (C) 2013 see Authors.txt
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

#include <afxwin.h>

class SkypeMoodMsgHandler
{
public:
    static const UINT uSkypeControlAPIDiscover;
    static const UINT uSkypeControlAPIAttach;

    SkypeMoodMsgHandler();

    void Connect(HWND hWnd);

    LRESULT HandleAttach(WPARAM wParam, LPARAM lParam);
    bool HandleMessage(HWND hWnd, COPYDATASTRUCT* pCDS);

    bool SendMoodMessage(CString msg);

private:
    enum {
        SKYPECONTROLAPI_ATTACH_SUCCESS = 0,
        SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION,
        SKYPECONTROLAPI_ATTACH_REFUSED,
        SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE,
        SKYPECONTROLAPI_ATTACH_API_AVAILABLE = 0x8001
    };

    HWND m_hWnd;
    HWND m_hWndSkype;

    CString m_lastMsg;

    bool SendMessage(CString msg);
};
