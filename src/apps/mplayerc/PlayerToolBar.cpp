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
#include "mplayerc.h"
#include <math.h>
#include <atlbase.h>
#include <afxpriv.h>
#include "PlayerToolBar.h"
#include "MainFrm.h"

typedef HRESULT (__stdcall * SetWindowThemeFunct)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);


// CPlayerToolBar

IMPLEMENT_DYNAMIC(CPlayerToolBar, CToolBar)
CPlayerToolBar::CPlayerToolBar()
{
}

CPlayerToolBar::~CPlayerToolBar()
{
}

BOOL CPlayerToolBar::Create(CWnd* pParentWnd)
{
	if(!__super::CreateEx(pParentWnd,
		TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|TBSTYLE_AUTOSIZE,
		WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM|CBRS_TOOLTIPS, CRect(2,2,0,3)) 
	|| !LoadToolBar(IDB_PLAYERTOOLBAR))
		return FALSE;

	GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	CToolBarCtrl& tb = GetToolBarCtrl();
	tb.DeleteButton(tb.GetButtonCount()-1);
	tb.DeleteButton(tb.GetButtonCount()-1);

	SetMute(AfxGetAppSettings().fMute);

	UINT styles[] = 
	{
		TBBS_CHECKGROUP, TBBS_CHECKGROUP, TBBS_CHECKGROUP, 
		TBBS_SEPARATOR,
		TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, 
		TBBS_SEPARATOR,
		TBBS_BUTTON/*|TBSTYLE_DROPDOWN*/, 
		TBBS_SEPARATOR,
		TBBS_SEPARATOR,
		TBBS_CHECKBOX, 
		/*TBBS_SEPARATOR,*/
	};

	for(int i = 0; i < countof(styles); i++)
		SetButtonStyle(i, styles[i]|TBBS_DISABLED);

	m_volctrl.Create(this);

	if(AfxGetAppSettings().fDisableXPToolbars)
	{
		if(HMODULE h = LoadLibrary(_T("uxtheme.dll")))
		{
			SetWindowThemeFunct f = (SetWindowThemeFunct)GetProcAddress(h, "SetWindowTheme");
			if(f) f(m_hWnd, L" ", L" ");
			FreeLibrary(h);
		}
	}

	// quick and dirty code from foxx1337; will leak, but don't care yet
	m_nButtonHeight = 16;	// hardcoded from MainFrm.cpp - DEFCLIENTW; min width should be 9 * button width + 60 + 91
	HBITMAP hBmp = static_cast<HBITMAP>(::LoadImage(NULL, _T("toolbar.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
	if(NULL != hBmp)
	{
		CBitmap *bmp = new CBitmap();
		bmp->Attach(hBmp);
		BITMAP bitmapBmp;
		bmp->GetBitmap(&bitmapBmp);
		if(bitmapBmp.bmWidth == bitmapBmp.bmHeight * 15)
		{
			// the manual specifies that sizeButton should be sizeImage inflated by (7, 6)
			SetSizes(CSize(bitmapBmp.bmHeight + 7, bitmapBmp.bmHeight + 6), CSize(bitmapBmp.bmHeight, bitmapBmp.bmHeight));

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			BITMAPINFOHEADER bih;
			bih.biSize = sizeof(BITMAPINFOHEADER);
			GetDIBits(dc, hBmp, 0, 0, NULL, reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS);
			int fileDepth = bih.biBitCount;
			CImageList *imageList = new CImageList();
			if(32 == fileDepth)
			{
				imageList->Create(bitmapBmp.bmHeight, bitmapBmp.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0); 
				imageList->Add(bmp, static_cast<CBitmap*>(0));	// alpha is the mask
			}
			else
			{
				imageList->Create(bitmapBmp.bmHeight, bitmapBmp.bmHeight, ILC_COLOR24 | ILC_MASK, 1, 0);
				imageList->Add(bmp, RGB(255, 0, 255));
			}
			m_nButtonHeight = bitmapBmp.bmHeight;
			GetToolBarCtrl().SetImageList(imageList);
		}
		delete bmp;
		DeleteObject(hBmp);
	}

	return TRUE;
}

BOOL CPlayerToolBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!__super::PreCreateWindow(cs))
		return FALSE;

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;
//	m_dwStyle |= CBRS_SIZE_FIXED;

	return TRUE;
}

void CPlayerToolBar::ArrangeControls()
{
	if(!::IsWindow(m_volctrl.m_hWnd)) return;

	CRect r;
	GetClientRect(&r);

	CRect br = GetBorders();

	CRect r10;
	GetItemRect(10, &r10);

	CRect vr;
	m_volctrl.GetClientRect(&vr);
	CRect vr2(r.right + br.right - 60, r.bottom - 25, r.right +br.right + 6, r.bottom);
	m_volctrl.MoveWindow(vr2);

	UINT nID;
	UINT nStyle;
	int iImage;
	GetButtonInfo(12, nID, nStyle, iImage);
	SetButtonInfo(11, GetItemID(11), TBBS_SEPARATOR, vr2.left - iImage - r10.right - (r10.bottom - r10.top) + 11);
}

void CPlayerToolBar::SetMute(bool fMute)
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	bi.iImage = fMute?13:12;
	tb.SetButtonInfo(ID_VOLUME_MUTE, &bi);

	AfxGetAppSettings().fMute = fMute;
}

bool CPlayerToolBar::IsMuted()
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_VOLUME_MUTE, &bi);
	return(bi.iImage==13);
}

int CPlayerToolBar::GetVolume()
{
	int volume = m_volctrl.GetPos();
	volume = (int)((log10(1.0*volume)-2)*5000);
	volume = max(min(volume, 0), -10000);
	return(IsMuted() ? -10000 : volume);
}

int CPlayerToolBar::GetMinWidth()
{
	return m_nButtonHeight * 9 + 151;
}

void CPlayerToolBar::SetVolume(int volume)
{
/*
	volume = (int)pow(10, ((double)volume)/5000+2);
	volume = max(min(volume, 100), 1);
*/
	m_volctrl.SetPosInternal(volume);
}

BEGIN_MESSAGE_MAP(CPlayerToolBar, CToolBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_COMMAND_EX(ID_VOLUME_MUTE, OnVolumeMute)
	ON_UPDATE_COMMAND_UI(ID_VOLUME_MUTE, OnUpdateVolumeMute)
	ON_COMMAND_EX(ID_VOLUME_UP, OnVolumeUp)
	ON_COMMAND_EX(ID_VOLUME_DOWN, OnVolumeDown)
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CPlayerToolBar message handlers

void CPlayerToolBar::OnPaint()
{
	if(m_bDelayedButtonLayout)
		Layout();

	CPaintDC dc(this); // device context for painting

	DefWindowProc(WM_PAINT, WPARAM(dc.m_hDC), 0);

	{
		UINT nID;
		UINT nStyle = 0;
		int iImage = 0;
		GetButtonInfo(11, nID, nStyle, iImage);
		CRect ItemRect;
		GetItemRect(11, ItemRect);
		dc.FillSolidRect(ItemRect, GetSysColor(COLOR_BTNFACE));
	}
}

void CPlayerToolBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	ArrangeControls();
}

void CPlayerToolBar::OnInitialUpdate()
{
	ArrangeControls();
}

BOOL CPlayerToolBar::OnVolumeMute(UINT nID)
{
	SetMute(!IsMuted());
	return FALSE;
}

void CPlayerToolBar::OnUpdateVolumeMute(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(IsMuted());
}

BOOL CPlayerToolBar::OnVolumeUp(UINT nID)
{
	m_volctrl.IncreaseVolume();
	return FALSE;
}

BOOL CPlayerToolBar::OnVolumeDown(UINT nID)
{
	m_volctrl.DecreaseVolume();
	return FALSE;
}

void CPlayerToolBar::OnNcPaint() // when using XP styles the NC area isn't drawn for our toolbar...
{
	CRect wr, cr;

	CWindowDC dc(this);
	GetClientRect(&cr);
	ClientToScreen(&cr);
	GetWindowRect(&wr);
	cr.OffsetRect(-wr.left, -wr.top);
	wr.OffsetRect(-wr.left, -wr.top);
	dc.ExcludeClipRect(&cr);
	dc.FillSolidRect(wr, GetSysColor(COLOR_BTNFACE));

	// Do not call CToolBar::OnNcPaint() for painting messages
}

void CPlayerToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	for(int i = 0, j = GetToolBarCtrl().GetButtonCount(); i < j; i++)
	{
		if(GetButtonStyle(i)&(TBBS_SEPARATOR|TBBS_DISABLED))
			continue;

		CRect r;
		GetItemRect(i, r);
		if(r.PtInRect(point))
		{
			__super::OnLButtonDown(nFlags, point);
			return;
		}
	}

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	if(!pFrame->m_fFullScreen)
	{
		MapWindowPoints(pFrame, &point, 1);
		pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}
}
