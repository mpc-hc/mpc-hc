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
#include "StaticLink.h"


// CStaticLink

COLORREF CStaticLink::g_colorUnvisited = RGB(0, 0, 255);        // blue
COLORREF CStaticLink::g_colorVisited   = RGB(128, 0, 128);      // purple

IMPLEMENT_DYNAMIC(CStaticLink, CStatic)

BEGIN_MESSAGE_MAP(CStaticLink, CStatic)
    ON_WM_NCHITTEST()
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_LBUTTONDOWN()
    ON_WM_SETCURSOR()
END_MESSAGE_MAP()

///////////////////
// Constructor sets default colors = blue/purple.
// bDeleteOnDestroy is used internally by PixieLib in CPixieDlg.
//
CStaticLink::CStaticLink(LPCTSTR lpText, bool bDeleteOnDestroy)
    : m_link(lpText)                        // link text (nullptr ==> window text)
    , m_color(g_colorUnvisited)             // not visited yet
    , m_bDeleteOnDestroy(bDeleteOnDestroy)  // delete object with window?
{
}

CStaticLink::~CStaticLink()
{
}

//////////////////
// Normally, a static control does not get mouse events unless it has
// SS_NOTIFY. This achieves the same effect as SS_NOTIFY, but it's fewer
// lines of code and more reliable than turning on SS_NOTIFY in OnCtlColor
// because Windows doesn't send WM_CTLCOLOR to bitmap static controls.
//
LRESULT CStaticLink::OnNcHitTest(CPoint point)
{
    return HTCLIENT;
}

//////////////////
// Handle reflected WM_CTLCOLOR to set custom control color.
// For a text control, use visited/unvisited colors and underline font.
// For non-text controls, do nothing. Also ensures SS_NOTIFY is on.
//
HBRUSH CStaticLink::CtlColor(CDC* pDC, UINT nCtlColor)
{
    ASSERT(nCtlColor == CTLCOLOR_STATIC);
    DWORD dwStyle = GetStyle();

    HBRUSH hbr = nullptr;
    if ((dwStyle & 0xFF) <= SS_RIGHT) {

        // this is a text control: set up font and colors
        if (!(HFONT)m_font) {
            // first time init: create font
            LOGFONT lf;
            GetFont()->GetObject(sizeof(lf), &lf);
            lf.lfUnderline = TRUE;
            m_font.CreateFontIndirect(&lf);
        }

        // use underline font and visited/unvisited colors
        pDC->SelectObject(&m_font);
        pDC->SetTextColor(m_color);
        pDC->SetBkMode(TRANSPARENT);

        // return hollow brush to preserve parent background color
        hbr = (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
    }
    return hbr;
}

/////////////////
// Handle mouse click: navigate link
//
void CStaticLink::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_link.IsEmpty()) {
        // no link: try to load from resource string or window text
        if (!m_link.LoadString(GetDlgCtrlID())) {
            GetWindowText(m_link);
        }
        if (m_link.IsEmpty()) {
            return;
        }
    }

    // Call ShellExecute to run the file.
    // For an URL, this means opening it in the browser.
    //
    HINSTANCE h = m_link.Navigate();
    if ((UINT_PTR)h > 32) {                     // success!
        m_color = g_colorVisited;               // change color
        Invalidate();                           // repaint
    } else {
        MessageBeep(0);                         // unable to execute file!
        TRACE(_T("*** WARNING: CStaticLink: unable to navigate link %s\n"),
              (LPCTSTR)m_link);
    }
}

//////////////////
// Set "hand" cursor to cue user that this is a link.
//
BOOL CStaticLink::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
    return TRUE;
}

//////////////////
// Normally, a control class is not destroyed when the window is;
// however, CPixieDlg creates static controls with "new" instead of
// as class members, so it's convenient to allow the option of destroying
// object with window. In applications where you want the object to be
// destroyed along with the window, you can call constructor with
// bDeleteOnDestroy=TRUE.
//
void CStaticLink::PostNcDestroy()
{
    if (m_bDeleteOnDestroy) {
        delete this;
    }
}
