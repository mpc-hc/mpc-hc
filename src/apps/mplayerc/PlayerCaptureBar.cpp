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

// PlayerCaptureBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "mainfrm.h"
#include "PlayerCaptureBar.h"


// CPlayerCaptureBar

IMPLEMENT_DYNAMIC(CPlayerCaptureBar, baseCPlayerCaptureBar)
CPlayerCaptureBar::CPlayerCaptureBar()
{
}

CPlayerCaptureBar::~CPlayerCaptureBar()
{
}

BOOL CPlayerCaptureBar::Create(CWnd* pParentWnd)
{
	if(!baseCPlayerCaptureBar::Create(_T("Capture Settings"), pParentWnd, 0))
		return FALSE;

	m_capdlg.Create(this);
	m_capdlg.ShowWindow(SW_SHOWNORMAL);

	CRect r;
	m_capdlg.GetWindowRect(r);
	m_szMinVert = m_szVert = r.Size();
	m_szMinHorz = m_szHorz = r.Size();
	m_szMinFloat = m_szFloat = r.Size();
	m_bFixedFloat = true;
	m_szFixedFloat = r.Size();

	return TRUE;
}

BOOL CPlayerCaptureBar::PreTranslateMessage(MSG* pMsg)
{
	if(IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		if(IsDialogMessage(pMsg))
			return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerCaptureBar, baseCPlayerCaptureBar)
END_MESSAGE_MAP()

// CPlayerCaptureBar message handlers
