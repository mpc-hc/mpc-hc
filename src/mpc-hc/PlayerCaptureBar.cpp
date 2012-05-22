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
#include "mplayerc.h"
#include "MainFrm.h"
#include "PlayerCaptureBar.h"


// CPlayerCaptureBar

IMPLEMENT_DYNAMIC(CPlayerCaptureBar, CPlayerBar)
CPlayerCaptureBar::CPlayerCaptureBar()
{
}

CPlayerCaptureBar::~CPlayerCaptureBar()
{
}

BOOL CPlayerCaptureBar::Create(CWnd* pParentWnd, UINT defDockBarID)
{
	if (!__super::Create(ResStr(IDS_CAPTURE_SETTINGS), pParentWnd, ID_VIEW_CAPTURE, defDockBarID, _T("Capture Settings"))) {
		return FALSE;
	}

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
	if (IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (IsDialogMessage(pMsg)) {
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerCaptureBar, CPlayerBar)
END_MESSAGE_MAP()
