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
#include "PlayerInfoBar.h"
#include "MainFrm.h"


// CPlayerInfoBar

IMPLEMENT_DYNAMIC(CPlayerInfoBar, CDialogBar)
CPlayerInfoBar::CPlayerInfoBar(int nFirstColWidth) : m_nFirstColWidth(nFirstColWidth)
{
}

CPlayerInfoBar::~CPlayerInfoBar()
{
}

void CPlayerInfoBar::SetLine(CString label, CString info)
{
	if(info.IsEmpty())
	{
		RemoveLine(label);
		return;
	}

	for(size_t idx = 0; idx < m_label.GetCount(); idx++)
	{
		CString tmp;
		m_label[idx]->GetWindowText(tmp);
		if(label == tmp)
		{
			m_info[idx]->GetWindowText(tmp);
			if(info != tmp) m_info[idx]->SetWindowText(info);
			return;
		}
	}

	CAutoPtr<CStatusLabel> l(DNew CStatusLabel(true, false));
	l->Create(label, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|SS_OWNERDRAW, CRect(0,0,0,0), this);
	m_label.Add(l);

	CAutoPtr<CStatusLabel> i(DNew CStatusLabel(false, true));
	i->Create(info, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|SS_OWNERDRAW, CRect(0,0,0,0), this);
	m_info.Add(i);

	Relayout();
}

void CPlayerInfoBar::GetLine(CString label, CString& info)
{
	info.Empty();

	for(size_t idx = 0; idx < m_label.GetCount(); idx++)
	{
		CString tmp;
		m_label[idx]->GetWindowText(tmp);
		if(label == tmp)
		{
			m_info[idx]->GetWindowText(tmp);
			info = tmp;
			return;
		}
	}
}

void CPlayerInfoBar::RemoveLine(CString label)
{
	for(size_t i = 0; i < m_label.GetCount(); i++)
	{
		CString tmp;
		m_label[i]->GetWindowText(tmp);
		if(label == tmp)
		{
			m_label.RemoveAt(i);
			m_info.RemoveAt(i);
			break;
		}
	}

	Relayout();
}

void CPlayerInfoBar::RemoveAllLines()
{
	m_label.RemoveAll();
	m_info.RemoveAll();

	Relayout();
}

BOOL CPlayerInfoBar::Create(CWnd* pParentWnd)
{
	return CDialogBar::Create(pParentWnd, IDD_PLAYERINFOBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM, IDD_PLAYERINFOBAR);
}

BOOL CPlayerInfoBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!CDialogBar::PreCreateWindow(cs))
		return FALSE;

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;

	return TRUE;
}

CSize CPlayerInfoBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CRect r;
	GetParent()->GetClientRect(&r);
	r.bottom = r.top + m_label.GetCount() * 17 + (m_label.GetCount() ? 4 : 0);
	return r.Size();
}

void CPlayerInfoBar::Relayout()
{
	CRect r;
	GetParent()->GetClientRect(&r);

	int w = m_nFirstColWidth, h = 17, y = 2;

	for(size_t i = 0; i < m_label.GetCount(); i++)
	{
		CDC* pDC = m_label[i]->GetDC();
		CString str;
		m_label[i]->GetWindowText(str);
		w = max(w, pDC->GetTextExtent(str).cx);
		m_label[i]->ReleaseDC(pDC);
	}

	for(size_t i = 0; i < m_label.GetCount(); i++, y += h)
	{
		m_label[i]->MoveWindow(1, y, w - 10, h);
		m_info[i]->MoveWindow(w + 10, y, r.Width()-(w+10)-1, h);
	}
}

BEGIN_MESSAGE_MAP(CPlayerInfoBar, CDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CPlayerInfoBar message handlers

BOOL CPlayerInfoBar::OnEraseBkgnd(CDC* pDC)
{
	for(CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow())
	{
		CRect r;
		pChild->GetClientRect(&r);
		pChild->MapWindowPoints(this, &r);
		pDC->ExcludeClipRect(&r);
	}

	CRect r;
	GetClientRect(&r);

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());

	if(pFrame->m_pLastBar != this || pFrame->m_fFullScreen)
		r.InflateRect(0, 0, 0, 1);

	if(pFrame->m_fFullScreen)
		r.InflateRect(1, 0, 1, 0);

	pDC->Draw3dRect(&r, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

	r.DeflateRect(1, 1);

	pDC->FillSolidRect(&r, 0);

	return TRUE;
}

void CPlayerInfoBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	Relayout();

	Invalidate();
}

void CPlayerInfoBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	if(!pFrame->m_fFullScreen)
	{
		MapWindowPoints(pFrame, &point, 1);
		pFrame->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}
}
