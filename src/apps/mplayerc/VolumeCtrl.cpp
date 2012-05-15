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
#include "VolumeCtrl.h"


// CVolumeCtrl

IMPLEMENT_DYNAMIC(CVolumeCtrl, CSliderCtrl)
CVolumeCtrl::CVolumeCtrl(bool fSelfDrawn) : m_fSelfDrawn(fSelfDrawn)
{
}

CVolumeCtrl::~CVolumeCtrl()
{
}

bool CVolumeCtrl::Create(CWnd* pParentWnd)
{
	if (!CSliderCtrl::Create(WS_CHILD|WS_VISIBLE|TBS_NOTICKS|TBS_HORZ|TBS_TOOLTIPS, CRect(0,0,0,0), pParentWnd, IDC_SLIDER1)) {
		return false;
	}
	EnableToolTips(TRUE);
	SetRange(0, 100);
	SetPosInternal(AfxGetAppSettings().nVolume);
	SetPageSize(5);
	SetLineSize(0);

	return true;
}

void CVolumeCtrl::SetPosInternal(int pos)
{
	SetPos(pos);
	GetParent()->PostMessage(WM_HSCROLL, MAKEWPARAM((short)pos, SB_THUMBPOSITION), (LPARAM)m_hWnd); // this will be reflected back on us
}

void CVolumeCtrl::IncreaseVolume()
{
	SetPosInternal(GetPos() + GetPageSize());
}

void CVolumeCtrl::DecreaseVolume()
{
	SetPosInternal(GetPos() - GetPageSize());
}

BEGIN_MESSAGE_MAP(CVolumeCtrl, CSliderCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL_REFLECT()
	ON_WM_SETCURSOR()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()

// CVolumeCtrl message handlers

void CVolumeCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	LRESULT lr = CDRF_DODEFAULT;

	if (m_fSelfDrawn)
		switch (pNMCD->dwDrawStage) {
			case CDDS_PREPAINT:
				lr = CDRF_NOTIFYITEMDRAW;
				break;

			case CDDS_ITEMPREPAINT:
				if (pNMCD->dwItemSpec == TBCD_CHANNEL) {
					CDC dc;
					dc.Attach(pNMCD->hdc);

					CRect r;
					GetClientRect(r);
					r.DeflateRect(8, 4, 10, 6);
					CopyRect(&pNMCD->rc, &r);
					CPen shadow(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
					CPen light(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
					CPen* old = dc.SelectObject(&light);
					dc.MoveTo(pNMCD->rc.right, pNMCD->rc.top);
					dc.LineTo(pNMCD->rc.right, pNMCD->rc.bottom);
					dc.LineTo(pNMCD->rc.left, pNMCD->rc.bottom);
					dc.SelectObject(&shadow);
					dc.LineTo(pNMCD->rc.right, pNMCD->rc.top);
					dc.SelectObject(old);

					dc.Detach();
					lr = CDRF_SKIPDEFAULT;
				} else if (pNMCD->dwItemSpec == TBCD_THUMB) {
					CDC dc;
					dc.Attach(pNMCD->hdc);
					pNMCD->rc.bottom--;
					CRect r(pNMCD->rc);
					r.DeflateRect(0, 0, 1, 0);

					COLORREF shadow = GetSysColor(COLOR_3DSHADOW);
					COLORREF light = GetSysColor(COLOR_3DHILIGHT);
					dc.Draw3dRect(&r, light, 0);
					r.DeflateRect(0, 0, 1, 1);
					dc.Draw3dRect(&r, light, shadow);
					r.DeflateRect(1, 1, 1, 1);
					dc.FillSolidRect(&r, GetSysColor(COLOR_BTNFACE));
					dc.SetPixel(r.left+7, r.top-1, GetSysColor(COLOR_BTNFACE));

					dc.Detach();
					lr = CDRF_SKIPDEFAULT;
				}

				break;
		};

	pNMCD->uItemState &= ~CDIS_FOCUS;

	*pResult = lr;
}

void CVolumeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect r;
	GetChannelRect(&r);

	if (r.left >= r.right) {
		return;
	}

	int start, stop;
	GetRange(start, stop);

	r.left += 3;
	r.right -= 4;

	if (point.x < r.left) {
		SetPos(start);
	} else if (point.x >= r.right) {
		SetPos(stop);
	} else {
		int w = r.right - r.left;
		if (start < stop) {
			SetPosInternal(start + ((stop - start) * (point.x - r.left) + (w/2)) / w);
		}
	}

	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CVolumeCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CSliderCtrl::OnSetFocus(pOldWnd);

	AfxGetMainWnd()->SetFocus(); // don't focus on us, parents will take care of our positioning
}

void CVolumeCtrl::HScroll(UINT nSBCode, UINT nPos)
{
	AfxGetAppSettings().nVolume = GetPos();

	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame && pFrame != GetParent()) {
		pFrame->PostMessage(WM_HSCROLL, MAKEWPARAM((short)nPos, nSBCode), (LPARAM)m_hWnd);
	}
}

BOOL CVolumeCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	return TRUE;
}

BOOL CVolumeCtrl::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXT *pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMHDR);
	CString str;
	str.AppendFormat(_T("%d%%"), GetPos());
	_tcscpy_s(pTTT->szText, str);
	pTTT->hinst = NULL;

	*pResult = 0;

	return TRUE;
}
