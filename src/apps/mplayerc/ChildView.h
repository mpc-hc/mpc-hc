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

#pragma once

#include "libpng.h"

class CChildView : public CWnd
{
    CRect m_vrect;

    DWORD m_lastlmdowntime;
    CPoint m_lastlmdownpoint;

    CCritSec m_csLogo;
    CPngImage m_logo;

public:
    CChildView();
    virtual ~CChildView();

    DECLARE_DYNAMIC(CChildView)

public:
    void SetVideoRect(CRect r = CRect(0, 0, 0, 0));
    CRect GetVideoRect()
    {
        return(m_vrect);
    }

    void LoadLogo();
    CSize GetLogoSize();

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnPlayPlayPauseStop(UINT nID);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
};
