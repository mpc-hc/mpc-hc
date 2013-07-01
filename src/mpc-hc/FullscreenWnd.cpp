/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "mplayerc.h"
#include "FullscreenWnd.h"
#include "MainFrm.h"

IMPLEMENT_DYNAMIC(CFullscreenWnd, CWnd)
CFullscreenWnd::CFullscreenWnd(CMainFrame* pMainFrame)
    : CMouseWnd(pMainFrame, true)
{
}

CFullscreenWnd::~CFullscreenWnd()
{
}

bool CFullscreenWnd::IsWindow()
{
    return (m_hWnd != nullptr);
}

BOOL CFullscreenWnd::PreTranslateMessage(MSG* pMsg)
{
    switch (pMsg->message) {
        case WM_KEYDOWN:
        case WM_KEYUP:
            AfxGetMainFrame()->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
            return TRUE;
    }
    return CWnd::PreTranslateMessage(pMsg);
}

BOOL CFullscreenWnd::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs)) {
        return FALSE;
    }

    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       ::LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr);

    return TRUE;
}

BEGIN_MESSAGE_MAP(CFullscreenWnd, CMouseWnd)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CFullscreenWnd::OnEraseBkgnd(CDC* pDC)
{
    return FALSE;
}
