/*
 * $Id$
 *
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
#include "mplayerc.h"
#include "mainfrm.h"
#include "PlayerNavigationBar.h"
#include "afxwin.h"


// CPlayerCaptureBar

IMPLEMENT_DYNAMIC(CPlayerNavigationBar, baseCPlayerNavigationBar)
CPlayerNavigationBar::CPlayerNavigationBar()
{
}

CPlayerNavigationBar::~CPlayerNavigationBar()
{
}

BOOL CPlayerNavigationBar::Create(CWnd* pParentWnd)
{
    if(!baseCPlayerNavigationBar::Create(_T("Navigation bar"), pParentWnd, 0))
        return FALSE;

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
    if(IsWindow(pMsg->hwnd) && IsVisible() && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
    {
        if(IsDialogMessage(pMsg))
            return TRUE;
    }

    return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPlayerNavigationBar, baseCPlayerNavigationBar)
    ON_WM_SIZE()
END_MESSAGE_MAP()

// CPlayerShaderEditorBar message handlers

void CPlayerNavigationBar::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy);

    if(::IsWindow(m_navdlg.m_hWnd))
    {
        CRect r;
        GetClientRect(r);
        m_navdlg.MoveWindow(r);
        r.DeflateRect(8,8,8,40);
        m_navdlg.m_ChannelList.MoveWindow(r);

        m_navdlg.m_ComboAudio.SetWindowPos(NULL, r.left,r.bottom +5, 0,0, SWP_NOSIZE | SWP_NOZORDER);
        m_navdlg.m_ButtonInfo.SetWindowPos(NULL, r.left+90,r.bottom +5, 0,0, SWP_NOSIZE | SWP_NOZORDER);
        m_navdlg.m_ButtonScan.SetWindowPos(NULL, r.left+145,r.bottom +5, 0,0, SWP_NOSIZE | SWP_NOZORDER);
    }


    /*
    	if (cy > 300)
    		m_navdlg.m_ChannelList.Size = System::Drawing::Size( cx - 20, cy - 85 );


    	if(::IsWindow(m_dlg.m_hWnd))
    	{
    		CRect r;
    		GetClientRect(r);
    		m_dlg.MoveWindow(r);
    	}
    */
}

void CPlayerNavigationBar::ShowControls(CWnd* pMainfrm, bool bShow)
{
	int hbefore = 0, hafter = 0;

	CSize s = this->CalcFixedLayout(FALSE, TRUE);
	hafter += s.cx;
	((CMainFrame*) pMainfrm) ->ShowControlBar(this, bShow, TRUE);

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);

	((CMainFrame*) pMainfrm)->RecalcLayout();
}
