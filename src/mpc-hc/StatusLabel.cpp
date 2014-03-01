/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include "StatusLabel.h"
#include "WinAPIUtils.h"
#include "SysVersion.h"


// CStatusLabel

IMPLEMENT_DYNAMIC(CStatusLabel, CStatic)
CStatusLabel::CStatusLabel(bool fRightAlign, bool fAddEllipses)
    : m_fRightAlign(fRightAlign)
    , m_fAddEllipses(fAddEllipses)
{
    m_font.m_hObject = nullptr;
    if (SysVersion::IsVistaOrLater()) {
        LOGFONT lf;
        GetStatusFont(&lf);
        VERIFY(m_font.CreateFontIndirect(&lf));
    } else {
        HDC hdc = ::GetDC(nullptr);
        double scale = 1.0 * GetDeviceCaps(hdc, LOGPIXELSY) / 96.0;
        ::ReleaseDC(nullptr, hdc);
        VERIFY(m_font.CreateFont(int(14 * scale), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                                 _T("Microsoft Sans Serif")));
    }
}

CStatusLabel::~CStatusLabel()
{
}

BEGIN_MESSAGE_MAP(CStatusLabel, CStatic)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CStatusLabel message handlers

BOOL CStatusLabel::PreTranslateMessage(MSG* pMsg)
{
    // Notify the parent window if the left button of the mouse is pressed so
    // that the user can drag the window by clicking on the information panel.
    if (pMsg->message == WM_LBUTTONDOWN) {
        CPoint p(pMsg->lParam);
        MapWindowPoints(GetParent(), &p, 1);
        GetParent()->SendMessage(pMsg->message, pMsg->wParam, MAKELPARAM(p.x, p.y));
    }

    return __super::PreTranslateMessage(pMsg);
}

void CStatusLabel::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CString str;
    GetWindowText(str);
    CRect clientRect;
    GetClientRect(&clientRect);

    CDC dc;
    dc.Attach(lpDrawItemStruct->hDC);
    dc.SetTextColor(0xffffff);
    dc.SetBkColor(0);
    CFont* old = dc.SelectObject(&m_font);
    const UINT style = DT_SINGLELINE | DT_NOPREFIX | (m_fAddEllipses ? DT_END_ELLIPSIS : 0);
    CRect textRect(clientRect);
    dc.DrawText(str, textRect, style | DT_CALCRECT);
    if (m_fRightAlign) {
        textRect.MoveToX(clientRect.Width() - textRect.Width());
    }
    textRect.MoveToY((clientRect.Height() - textRect.Height()) / 2);
    dc.DrawText(str, textRect, style);
    dc.ExcludeClipRect(textRect);
    dc.FillSolidRect(clientRect, 0);
    dc.SelectObject(&old);
    dc.Detach();
}

BOOL CStatusLabel::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}
