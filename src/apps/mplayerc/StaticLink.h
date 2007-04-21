/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

// CHyperlink

class CHyperlink : public CString { 
public: 
    CHyperlink(LPCTSTR lpLink = NULL) : CString(lpLink) { } 
    ~CHyperlink() { } 
    const CHyperlink& operator=(LPCTSTR lpsz) { 
        CString::operator=(lpsz); 
        return *this; 
    } 
    operator LPCTSTR() { 
        return CString::operator LPCTSTR();  
    } 
    virtual HINSTANCE Navigate() { 
        return IsEmpty() ? NULL : 
            ShellExecute(0, _T("open"), *this, 0, 0, SW_SHOWNORMAL); 
    } 
};

// CStaticLink

class CStaticLink : public CStatic
{
public: 
    DECLARE_DYNAMIC(CStaticLink) 
    CStaticLink(LPCTSTR lpText = NULL, BOOL bDeleteOnDestroy=FALSE); 
    ~CStaticLink() { } 

    // Hyperlink contains URL/filename. If NULL, I will use the window text. 
    // (GetWindowText) to get the target. 
    CHyperlink    m_link; 
    COLORREF        m_color; 

    // Default colors you can change 
    // These are global, so they're the same for all links. 
    static COLORREF g_colorUnvisited; 
    static COLORREF g_colorVisited; 

    // Cursor used when mouse is on a link--you can set, or 
    // it will default to the standard hand with pointing finger. 
    // This is global, so it's the same for all links. 
    static HCURSOR     g_hCursorLink; 

protected: 
    CFont            m_font;                    // underline font for text control 
    BOOL            m_bDeleteOnDestroy;    // delete object when window destroyed? 

    virtual void PostNcDestroy(); 

    // message handlers 
    DECLARE_MESSAGE_MAP() 
    afx_msg LRESULT    OnNcHitTest(CPoint point); 
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor); 
    afx_msg void    OnLButtonDown(UINT nFlags, CPoint point); 
    afx_msg BOOL    OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); 
};


