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
#include <math.h>
#include <atlbase.h>
#include <afxpriv.h>
#include "MPCPngImage.h"
#include "PlayerToolBar.h"
#include "MainFrm.h"

// CPlayerToolBar

IMPLEMENT_DYNAMIC(CPlayerToolBar, CToolBar)
CPlayerToolBar::CPlayerToolBar()
	: m_nButtonHeight(16), m_pButtonsImages(NULL)
{
}

CPlayerToolBar::~CPlayerToolBar()
{
	if (NULL != m_pButtonsImages) {
		delete m_pButtonsImages;
		m_pButtonsImages = NULL;
	}
}

void CPlayerToolBar::LoadExternalToolBar(CImage* image)
{
	CString path;
	GetModuleFileName(AfxGetInstanceHandle(), path.GetBuffer(_MAX_PATH), _MAX_PATH);
	path.ReleaseBuffer();
	path = path.Left(path.ReverseFind('\\') + 1);

	if (SUCCEEDED(image->Load(path + _T("toolbar.png")))) {
		;
	} else { // toolbar.bmp
		HBITMAP hBmp = (HBITMAP)LoadImage(NULL, path + _T("toolbar.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		if (!!hBmp) {
			image->Attach(hBmp);
		}
	}
}

BOOL CPlayerToolBar::Create(CWnd* pParentWnd)
{
	VERIFY(__super::CreateEx(pParentWnd,
							 TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|TBSTYLE_AUTOSIZE|TBSTYLE_CUSTOMERASE,
							 WS_CHILD|WS_VISIBLE|CBRS_BOTTOM|CBRS_TOOLTIPS, CRect(2,2,0,1)));

	VERIFY(LoadToolBar(IDB_PLAYERTOOLBAR));

	// Should never be RTLed
	ModifyStyleEx(WS_EX_LAYOUTRTL, WS_EX_NOINHERITLAYOUT);

	CToolBarCtrl& tb = GetToolBarCtrl();
	tb.DeleteButton(tb.GetButtonCount()-1);
	tb.DeleteButton(tb.GetButtonCount()-1);

	SetMute(AfxGetAppSettings().fMute);

	UINT styles[] = {
		TBBS_CHECKGROUP, TBBS_CHECKGROUP, TBBS_CHECKGROUP,
		TBBS_SEPARATOR,
		TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON, TBBS_BUTTON,
		TBBS_SEPARATOR,
		TBBS_BUTTON,
		TBBS_SEPARATOR,
		TBBS_SEPARATOR,
		TBBS_CHECKBOX,
	};

	for (int i = 0; i < _countof(styles); ++i) {
		SetButtonStyle(i, styles[i] | TBBS_DISABLED);
	}

	m_volctrl.Create(this);
	m_volctrl.SetRange(0, 100);

	m_nButtonHeight = 16; //reset m_nButtonHeight
	CImage image;
	LoadExternalToolBar(&image);
	if (!image.IsNull()) {
		CBitmap *bmp = CBitmap::FromHandle((HBITMAP)image);
		int width = image.GetWidth();
		int height = image.GetHeight();
		int bpp = image.GetBPP();
		if (width == height * 15) {
			// the manual specifies that sizeButton should be sizeImage inflated by (7, 6)
			SetSizes(CSize(height + 7, height + 6), CSize(height, height));

			m_pButtonsImages = DNew CImageList();
			if (bpp == 32) {
				m_pButtonsImages->Create(height, height, ILC_COLOR32 | ILC_MASK, 1, 0);
				m_pButtonsImages->Add(bmp, static_cast<CBitmap*>(0));	// alpha is the mask
			} else {
				m_pButtonsImages->Create(height, height, ILC_COLOR24 | ILC_MASK, 1, 0);
				m_pButtonsImages->Add(bmp, RGB(255, 0, 255));
			}
			m_nButtonHeight = height;
			GetToolBarCtrl().SetImageList(m_pButtonsImages);
		}
		image.Destroy();
	}

	return TRUE;
}

void CPlayerToolBar::ArrangeControls()
{
	if (!::IsWindow(m_volctrl.m_hWnd)) {
		return;
	}

	CRect r;
	GetClientRect(&r);

	CRect br = GetBorders();

	CRect r10;
	GetItemRect(10, &r10);

	CRect vr;
	m_volctrl.GetClientRect(&vr);
	CRect vr2(r.right + br.right - 60, r.bottom - 25, r.right + br.right + 6, r.bottom);
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
	bi.iImage = fMute ? 13 : 12;
	tb.SetButtonInfo(ID_VOLUME_MUTE, &bi);

	AfxGetAppSettings().fMute = fMute;
}

bool CPlayerToolBar::IsMuted() const
{
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_VOLUME_MUTE, &bi);
	return (bi.iImage == 13);
}

int CPlayerToolBar::GetVolume() const
{
	int volume = m_volctrl.GetPos(); // [0..100]
	if (IsMuted() || volume <= 0) {
		volume = -10000;
	} else {
		volume = min((int)(4000 * log10(volume / 100.0f)), 0); // 4000=2.0*100*20, where 2.0 is a special factor
	}

	return volume;
}

int CPlayerToolBar::GetMinWidth() const
{
	return m_nButtonHeight * 9 + 155;
}

void CPlayerToolBar::SetVolume(int volume)
{
	m_volctrl.SetPosInternal(volume);
}

BEGIN_MESSAGE_MAP(CPlayerToolBar, CToolBar)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_COMMAND_EX(ID_VOLUME_MUTE, OnVolumeMute)
	ON_UPDATE_COMMAND_UI(ID_VOLUME_MUTE, OnUpdateVolumeMute)
	ON_COMMAND_EX(ID_VOLUME_UP, OnVolumeUp)
	ON_COMMAND_EX(ID_VOLUME_DOWN, OnVolumeDown)
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CPlayerToolBar message handlers

void CPlayerToolBar::OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTBCUSTOMDRAW pTBCD = reinterpret_cast<LPNMTBCUSTOMDRAW>(pNMHDR);
	LRESULT lr = CDRF_DODEFAULT;
	switch (pTBCD->nmcd.dwDrawStage) {
		case CDDS_PREERASE:
			m_volctrl.Invalidate();
			lr = CDRF_DODEFAULT;
			break;
		case CDDS_PREPAINT: {
			// paint the control background, this is needed for XP
			CDC dc;
			dc.Attach(pTBCD->nmcd.hdc);
			RECT r;
			GetClientRect(&r);
			dc.FillSolidRect(&r, ::GetSysColor(COLOR_BTNFACE));
			dc.Detach();
		}
		lr |= CDRF_NOTIFYITEMDRAW;
		break;
		case CDDS_ITEMPREPAINT:
			// notify we want to paint after the system's paint cycle
			lr |= CDRF_NOTIFYPOSTPAINT;
			lr |= CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPOSTPAINT:
			// paint over the duplicated separator
			CDC dc;
			dc.Attach(pTBCD->nmcd.hdc);
			RECT r;
			GetItemRect(11, &r);
			dc.FillSolidRect(&r, GetSysColor(COLOR_BTNFACE));
			dc.Detach();
			lr |= CDRF_SKIPDEFAULT;
			break;
	}

	*pResult = lr;
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

void CPlayerToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	int i = getHitButtonIdx(point);

	if (i == -1 || (GetButtonStyle(i) & (TBBS_SEPARATOR | TBBS_DISABLED))) {
		;
	} else {
		if (i != 10 && i != 11) {
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
		}
	}
	__super::OnMouseMove(nFlags, point);
}

void CPlayerToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int i = getHitButtonIdx(point);
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	if (i == -1 || (GetButtonStyle(i) & (TBBS_SEPARATOR | TBBS_DISABLED))) {
		if (!pFrame->m_fFullScreen) {
			MapWindowPoints(pFrame, &point, 1);
			pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
		}
	} else {
		if (i != 10 && i != 11) {
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
		}

		__super::OnLButtonDown(nFlags, point);
	}
}

int CPlayerToolBar::getHitButtonIdx(CPoint point)
{
	int hit = -1; // -1 means not on any buttons, mute button is 12/13, others < 10, 11 is empty space between
	CRect r;

	for (int i = 0, j = GetToolBarCtrl().GetButtonCount(); i < j; i++) {
		GetItemRect(i, r);

		if (r.PtInRect(point)) {
			hit = i;
			break;
		}
	}

	return hit;
}
