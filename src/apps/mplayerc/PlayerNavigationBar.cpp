/*
 * $Id$
 *
 * (C) 2006-2012 see AUTHORS
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
#include "mplayerc.h"
#include "MainFrm.h"
#include "PlayerNavigationBar.h"
#include <afxwin.h>


// CPlayerCaptureBar

IMPLEMENT_DYNAMIC(CPlayerNavigationBar, baseCPlayerNavigationBar)
CPlayerNavigationBar::CPlayerNavigationBar()
{
}

CPlayerNavigationBar::~CPlayerNavigationBar()
{
}

BOOL CPlayerNavigationBar::Create(CWnd* pParentWnd, UINT defDockBarID)
{
	if (!baseCPlayerNavigationBar::Create(ResStr(IDS_NAVIGATION_BAR), pParentWnd, ID_VIEW_NAVIGATION, defDockBarID, _T("Navigation Bar"))) {
		return FALSE;
	}

	m_pParent = pParentWnd;
	m_navdlg.Create(this);
	m_navdlg.ShowWindow(SW_SHOWNORMAL);

	CRect r;
	m_navdlg.GetWindowRect(r);
	m_szMinVert = m_szVert = r.Size();
	m_szMinHorz = m_szHorz = r.Size();
	m_szMinFloat = m_szFloat = r.Size();
	m_bFixedFloat = true;
	m_szFixedFloat = r.Size();

	return TRUE;
}

BOOL CPlayerNavigationBar::PreTranslateMessage(MSG* pMsg)
{
	if (IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {
		if (IsDialogMessage(pMsg)) {
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerNavigationBar, baseCPlayerNavigationBar)
	ON_WM_SIZE()
	ON_WM_NCLBUTTONUP()
END_MESSAGE_MAP()

// CPlayerShaderEditorBar message handlers

void CPlayerNavigationBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	if (::IsWindow(m_navdlg.m_hWnd)) {
		CRect r, rectComboAudio, rectButtonInfo, rectButtonScan;
		LONG totalsize, separation, sizeComboAudio, sizeButtonInfo, sizeButtonScan;
		GetClientRect(r);
		m_navdlg.MoveWindow(r);
		r.DeflateRect(8,8,8,50);
		m_navdlg.m_ChannelList.MoveWindow(r);

		m_navdlg.m_ComboAudio.GetClientRect(rectComboAudio);
		m_navdlg.m_ButtonInfo.GetClientRect(rectButtonInfo);
		m_navdlg.m_ButtonScan.GetClientRect(rectButtonScan);
		sizeComboAudio = rectComboAudio.right - rectComboAudio.left;
		sizeButtonInfo = rectButtonInfo.right - rectButtonInfo.left;
		sizeButtonScan = rectButtonScan.right - rectButtonScan.left;
		totalsize = r.right - r.left;
		separation = (totalsize - sizeComboAudio - sizeButtonInfo - sizeButtonScan) / 2;
		if (separation < 0) {
			separation = 0;
		}
		m_navdlg.m_ComboAudio.SetWindowPos(NULL, r.left, r.bottom+6, 0,0, SWP_NOSIZE | SWP_NOZORDER);
		m_navdlg.m_ButtonInfo.SetWindowPos(NULL, r.left + sizeComboAudio + separation, r.bottom +5, 0,0, SWP_NOSIZE | SWP_NOZORDER);
		m_navdlg.m_ButtonScan.SetWindowPos(NULL, r.left + sizeComboAudio + sizeButtonInfo + 2 * separation, r.bottom +5, 0,0, SWP_NOSIZE | SWP_NOZORDER);
		m_navdlg.m_ButtonFilterStations.SetWindowPos(NULL, r.left,r.bottom +30, totalsize, 20, SWP_NOZORDER);
	}

}

void CPlayerNavigationBar::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	__super::OnNcLButtonUp(nHitTest, point);

	if (nHitTest == HTCLOSE) {
		AfxGetAppSettings().fHideNavigation = true;
	}
}


void CPlayerNavigationBar::ShowControls(CWnd* pMainfrm, bool bShow)
{
	CSize s = this->CalcFixedLayout(FALSE, TRUE);
	((CMainFrame*) pMainfrm) ->ShowControlBar(this, bShow, TRUE);

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);

	((CMainFrame*) pMainfrm)->RecalcLayout();
}
