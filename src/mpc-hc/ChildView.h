/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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

#include "MPCPngImage.h"
#include "MouseTouch.h"

class CChildView : public CMouseWnd
{
    CRect m_vrect;

    CMPCPngImage m_img;
    CImage m_resizedImg;

    CMainFrame* m_pMainFrame;

    bool m_bSwitchingFullscreen;
    bool m_bFirstMedia;
    bool m_bCustomImgLoaded;

    EventClient m_eventc;

    void EventCallback(MpcEvent ev);

    void LoadImgInternal(HGDIOBJ hImg);

public:
    CChildView(CMainFrame* pMainFrm);
    virtual ~CChildView();

    DECLARE_DYNAMIC(CChildView)

    void SetVideoRect(const CRect& r = CRect(0, 0, 0, 0));
    CRect GetVideoRect() const { return m_vrect; }

    void LoadImg(const CString& imagePath = _T(""));
    void LoadImg(std::vector<BYTE> buffer);
    CSize GetLogoSize();
    bool IsCustomImgLoaded() const { return m_bCustomImgLoaded; };

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
};
