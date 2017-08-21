/*
 * (C) 2013, 2015, 2017 see Authors.txt
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
#include "SkypeMoodMsgHandler.h"
#include "DSUtil.h"

const UINT SkypeMoodMsgHandler::uSkypeControlAPIDiscover = RegisterWindowMessage(_T("SkypeControlAPIDiscover"));

const UINT SkypeMoodMsgHandler::uSkypeControlAPIAttach = RegisterWindowMessage(_T("SkypeControlAPIAttach"));

SkypeMoodMsgHandler::SkypeMoodMsgHandler()
    : m_hWnd(nullptr)
    , m_hWndSkype(nullptr)
{
}

void SkypeMoodMsgHandler::Connect(HWND hWnd)
{
    m_hWnd = hWnd;
    TRACE(_T("SkypeMoodMsgHandler::Connect --> hWnd = %p\n"), hWnd);
    ::SendNotifyMessage(HWND_BROADCAST, uSkypeControlAPIDiscover, (WPARAM)hWnd, 0);
}

LRESULT SkypeMoodMsgHandler::HandleAttach(WPARAM wParam, LPARAM lParam)
{
    m_hWndSkype = nullptr;

    switch (lParam) {
        case SKYPECONTROLAPI_ATTACH_SUCCESS:
            m_hWndSkype = (HWND)wParam;
            TRACE(_T("SkypeMoodMsgHandler::HandleSkypeAttach --> Success: m_hWndSkype = %p\n"), m_hWndSkype);
            if (!m_lastMsg.IsEmpty()) {
                SendMoodMessage(m_lastMsg);
            }
            break;
        case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
            TRACE(_T("SkypeMoodMsgHandler::HandleSkypeAttach --> Pending authorization\n"));
            break;
        case SKYPECONTROLAPI_ATTACH_REFUSED:
            TRACE(_T("SkypeMoodMsgHandler::HandleSkypeAttach --> Attach refused\n"));
            break;
        case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
            TRACE(_T("SkypeMoodMsgHandler::HandleSkypeAttach --> API not available\n"));
            break;
        case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:
            TRACE(_T("SkypeMoodMsgHandler::HandleSkypeAttach --> API available\n"));
            Connect(m_hWnd);
            break;
        default:
            TRACE(_T("SkypeMoodMsgHandler::HandleSkypeAttach --> Unknown message\n"));
            break;
    }

    return TRUE;
}

bool SkypeMoodMsgHandler::HandleMessage(HWND hWnd, COPYDATASTRUCT* pCDS)
{
    bool bMsgHandled = false;

    if (hWnd && hWnd == m_hWndSkype) {
        bMsgHandled = true;

        CString msg = UTF8To16((char*)pCDS->lpData);

        TRACE(_T("SkypeMoodMsgHandler::HandleMessage --> %s\n"), msg.GetString());
    }

    return bMsgHandled;
}

bool SkypeMoodMsgHandler::SendMessage(CString msg)
{
    bool bSuccess = false;

    TRACE(_T("SkypeMoodMsgHandler::SendMessage --> Sending \"%s\"\n"), msg.GetString());

    if (!m_hWndSkype) {
        TRACE(_T("SkypeMoodMsgHandler::SendMessage --> not connected\n"));
    } else {
        CStringA utf8Msg = UTF16To8(msg);

        COPYDATASTRUCT cds;
        cds.dwData = 0;
        cds.lpData = (PVOID)(LPCSTR)utf8Msg;
        cds.cbData = utf8Msg.GetLength() + 1;

        bSuccess = !!::SendMessage(m_hWndSkype, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cds);
        TRACE(_T("SkypeMoodMsgHandler::SendMessage --> %s\n"), bSuccess ? _T("success") : _T("failed"));
    }

    return bSuccess;
}

bool SkypeMoodMsgHandler::SendMoodMessage(CString msg)
{
    m_lastMsg = msg;
    return SendMessage(_T("SET PROFILE RICH_MOOD_TEXT ") + msg);
}
