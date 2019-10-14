/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "CMPCThemeEdit.h"
// CFloatEdit

class CMPCThemeFloatEdit : public CMPCThemeEdit
{
public:
    bool GetFloat(float& f);
    double operator = (double d);
    operator double();

    DECLARE_DYNAMIC(CMPCThemeFloatEdit)
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// CIntEdit

class CMPCThemeIntEdit : public CMPCThemeEdit
{
public:
    DECLARE_DYNAMIC(CMPCThemeIntEdit)
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

// CHexEdit

class CMPCThemeHexEdit : public CMPCThemeEdit
{
public:
    bool GetDWORD(DWORD& dw);
    DWORD operator = (DWORD dw);
    operator DWORD();

    DECLARE_DYNAMIC(CMPCThemeHexEdit)
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};
