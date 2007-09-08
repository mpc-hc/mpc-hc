/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

// PlayerShaderEditorBar.cpp : implementation file
//

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
	if(!__super::Create(ResStr(IDS_AG_SHADER_EDITOR), pParentWnd, 0))
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
