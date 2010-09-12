/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
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

IMPLEMENT_DYNAMIC(CPlayerShaderEditorBar, baseCPlayerShaderEditorBar)
CPlayerShaderEditorBar::CPlayerShaderEditorBar()
{
}

CPlayerShaderEditorBar::~CPlayerShaderEditorBar()
{
}

BOOL CPlayerShaderEditorBar::Create(CWnd* pParentWnd)
{
	if(!__super::Create(_T("Shader Editor"), pParentWnd, ID_VIEW_SHADEREDITOR))
		return FALSE;

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
	if(IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		if(IsDialogMessage(pMsg))
			return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerShaderEditorBar, baseCPlayerShaderEditorBar)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CPlayerShaderEditorBar message handlers

void CPlayerShaderEditorBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if(::IsWindow(m_dlg.m_hWnd))
	{
		CRect r;
		GetClientRect(r);
		m_dlg.MoveWindow(r);
	}
}
