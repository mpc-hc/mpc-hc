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

#pragma once

// CHyperlink

class CHyperlink : public CString
{
public:
    CHyperlink(LPCTSTR lpLink = nullptr) : CString(lpLink) {}
    ~CHyperlink() {}
    const CHyperlink& operator=(LPCTSTR lpsz) {
        CString::operator=(lpsz);
        return *this;
    }
    operator LPCTSTR() {
        return CString::operator LPCTSTR();
    }
    /*virtual*/ HINSTANCE Navigate() {
        return IsEmpty() ? nullptr : ShellExecute(0, _T("open"), *this, 0, 0, SW_SHOWNORMAL);
    }
};

// CStaticLink

class CStaticLink : public CStatic
{
public:
    DECLARE_DYNAMIC(CStaticLink)
    CStaticLink(LPCTSTR lpText = nullptr, bool bDeleteOnDestroy = false);
    ~CStaticLink();

    // Hyperlink contains URL/filename. If nullptr, I will use the window text.
    // (GetWindowText) to get the target.
    CHyperlink m_link;
    COLORREF   m_color;

    // Default colors you can change
    // These are global, so they're the same for all links.
    static COLORREF g_colorUnvisited;
    static COLORREF g_colorVisited;

protected:
    CFont m_font;             // underline font for text control
    bool  m_bDeleteOnDestroy; // delete object when window destroyed?

    virtual void PostNcDestroy();

    // message handlers
    DECLARE_MESSAGE_MAP()
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg HBRUSH  CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg void    OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL    OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
