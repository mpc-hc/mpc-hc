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

// CFloatEdit

class CFloatEdit : public CEdit
{
public:
    bool GetFloat(float& f);
    double operator = (double d);
    operator double();

    DECLARE_DYNAMIC(CFloatEdit)
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// CIntEdit

class CIntEdit : public CEdit
{
public:
    DECLARE_DYNAMIC(CIntEdit)
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// CHexEdit

class CHexEdit : public CEdit
{
public:
    bool GetDWORD(DWORD& dw);
    DWORD operator = (DWORD dw);
    operator DWORD();

    DECLARE_DYNAMIC(CHexEdit)
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};
