/*
 * (C) 2014 see Authors.txt
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
#include "ColorButton.h"
#include "CMPCTheme.h"

CColorButton::CColorButton()
{
    Initialize();
}

void CColorButton::SetColor(COLORREF color)
{
    if (m_color != color) {
        m_color = color;
        Invalidate();
    }
}

void CColorButton::Initialize()
{
    if (m_bInitialized) {
        m_penInside.DeleteObject();
        m_penBorder.DeleteObject();
        m_penBorderFocus.DeleteObject();
    } else {
        m_bInitialized = true;
    }

    if (AfxGetAppSettings().bMPCThemeLoaded) {
        VERIFY(m_penInside.CreatePen(PS_INSIDEFRAME, 1, CMPCTheme::NoBorderColor));
        VERIFY(m_penBorder.CreatePen(PS_INSIDEFRAME, 1, CMPCTheme::ButtonBorderOuterColor));
        VERIFY(m_penBorderFocus.CreatePen(PS_INSIDEFRAME, 1, CMPCTheme::ButtonBorderInnerFocusedColor));
    } else {
        VERIFY(m_penInside.CreatePen(PS_INSIDEFRAME, 1, GetSysColor(COLOR_BTNFACE)));
        VERIFY(m_penBorder.CreatePen(PS_INSIDEFRAME, 1, GetSysColor(COLOR_BTNSHADOW)));
        VERIFY(m_penBorderFocus.CreatePen(PS_INSIDEFRAME, 1, GetSysColor(COLOR_HIGHLIGHT)));
    }
}

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    if (pDC) {
        CRect rect;
        GetClientRect(rect);
        CPen& borderPen = (lpDrawItemStruct->itemState & ODS_FOCUS) ? m_penBorderFocus : m_penBorder;
        CPen* pOldPen = pDC->SelectObject(&borderPen);
        pDC->Rectangle(rect);
        pDC->SelectObject(&m_penInside);
        rect.DeflateRect(1, 1);
        pDC->Rectangle(rect);
        rect.DeflateRect(1, 1);
        pDC->FillSolidRect(rect, m_color);
        pDC->SelectObject(pOldPen);
    } else {
        ASSERT(FALSE);
    }
}

BEGIN_MESSAGE_MAP(CColorButton, CButton)
    ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CColorButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    UNREFERENCED_PARAMETER(pWnd);
    UNREFERENCED_PARAMETER(nHitTest);
    UNREFERENCED_PARAMETER(message);

    if (IsWindowEnabled()) {
        ::SetCursor(m_cursor);
        return TRUE;
    }

    return FALSE;
}
