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

#include "DpiHelper.h"

// CStatusLabel

class CStatusLabel : public CStatic
{
    DECLARE_DYNAMIC(CStatusLabel)

private:
    bool m_fRightAlign, m_fAddEllipses;
    CFont m_font;

public:
    CStatusLabel(const DpiHelper& dpiHelper, bool fRightAlign, bool fAddEllipses);
    virtual ~CStatusLabel();

    void ScaleFont(const DpiHelper& dpiHelper);
    CFont& GetFont() { return m_font; }

    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    DECLARE_MESSAGE_MAP()

    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
