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
#include "StatusLabel.h"


// CStatusLabel

IMPLEMENT_DYNAMIC(CStatusLabel, CStatic)
CStatusLabel::CStatusLabel(bool fRightAlign, bool fAddEllipses)
	: m_fRightAlign(fRightAlign)
	, m_fAddEllipses(fAddEllipses)
{
	HDC hdc = ::GetDC(NULL);
	double scale = 1.0*GetDeviceCaps(hdc, LOGPIXELSY) / 96.0;
	::ReleaseDC(0, hdc);

	m_font.m_hObject = NULL;

	if(!(::GetVersion()&0x80000000))
		m_font.CreateFont(int(14.0 * scale), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE,
						  _T("Microsoft Sans Serif"));
	if(!m_font.m_hObject)
		m_font.CreateFont(int(14.0 * scale), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
						  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE,
						  _T("MS Sans Serif"));
}

CStatusLabel::~CStatusLabel()
{
}

BEGIN_MESSAGE_MAP(CStatusLabel, CStatic)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CStatusLabel message handlers

void CStatusLabel::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	CString str;
	GetWindowText(str);
	CRect r;
	GetClientRect(&r);
	CFont* old = dc.SelectObject(&m_font);
	dc.SetTextColor(0xffffff);
	dc.SetBkColor(0);
	CSize size = dc.GetTextExtent(str);
	CPoint p = CPoint(m_fRightAlign ? r.Width() - size.cx : 0, (r.Height()-size.cy)/2);

	if(m_fAddEllipses)
		while(size.cx > r.Width()-3 && str.GetLength() > 3) {
			str = str.Left(str.GetLength()-4) + _T("...");
			size = dc.GetTextExtent(str);
		}

	dc.TextOut(p.x, p.y, str);
	dc.ExcludeClipRect(CRect(p, size));
	dc.SelectObject(&old);
	dc.FillSolidRect(&r, 0);
	dc.Detach();
}

BOOL CStatusLabel::OnEraseBkgnd(CDC* pDC)
{
	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r, 0);
	return TRUE;
}
