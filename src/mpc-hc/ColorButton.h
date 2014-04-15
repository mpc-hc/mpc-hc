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

#pragma once

class CColorButton : public CButton
{
public:
    CColorButton();
    void SetColor(COLORREF color);

protected:
    void Initialize();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;

    DECLARE_MESSAGE_MAP()

    BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

    bool m_bInitialized = false;
    HCURSOR m_cursor = LoadCursor(nullptr, IDC_HAND);
    COLORREF m_color = 0;
    CPen m_penInside, m_penBorder, m_penBorderFocus;
};
