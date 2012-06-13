/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "PlayerShaderEditorBar.h"


// CPlayerShaderEditorBar

IMPLEMENT_DYNAMIC(CPlayerShaderEditorBar, CPlayerBar)
CPlayerShaderEditorBar::CPlayerShaderEditorBar()
{
}

CPlayerShaderEditorBar::~CPlayerShaderEditorBar()
{
}

BOOL CPlayerShaderEditorBar::Create(CWnd* pParentWnd, UINT defDockBarID)
{
    if (!__super::Create(ResStr(IDS_SHADER_EDITOR), pParentWnd, ID_VIEW_SHADEREDITOR, defDockBarID, _T("Shader Editor"))) {
        return FALSE;
    }

    m_dlg.Create(this);
    m_dlg.ShowWindow(SW_SHOWNORMAL);

    CRect r;
    m_dlg.GetWindowRect(r);
    m_szMinVert = m_szVert = r.Size();
    m_szMinHorz = m_szHorz = r.Size();
    m_szMinFloat = m_szFloat = r.Size();
    m_bFixedFloat = false;

    return TRUE;
}

BOOL CPlayerShaderEditorBar::PreTranslateMessage(MSG* pMsg)
{
    if (IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
        if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
            GetParentFrame()->ShowControlBar(this, FALSE, TRUE);
            return TRUE;
        }

        if (IsDialogMessage(pMsg)) {
            return TRUE;
        }
    }

    return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerShaderEditorBar, CPlayerBar)
    ON_WM_SIZE()
END_MESSAGE_MAP()

// CPlayerShaderEditorBar message handlers

void CPlayerShaderEditorBar::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);

    if (::IsWindow(m_dlg.m_hWnd)) {
        CRect r;
        GetClientRect(r);
        m_dlg.MoveWindow(r);
    }
}
