/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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

#include "STS.h"

class CCDecoder
{
    CSimpleTextSubtitle m_sts;
    CString m_fn, m_rawfn;
    __int64 m_time;
    bool m_fEndOfCaption;
    WCHAR m_buff[16][33], m_disp[16][33];
    CPoint m_cursor;

    void SaveDisp(__int64 time);
    void MoveCursor(int x, int y);
    void OffsetCursor(int x, int y);
    void PutChar(WCHAR c);

public:
    CCDecoder(CString fn = _T(""), CString rawfn = _T(""));
    virtual ~CCDecoder();
    void DecodeCC(const BYTE* buff, int len, __int64 time);
    void ExtractCC(BYTE* buff, int len, __int64 time);
    CSimpleTextSubtitle& GetSTS() { return m_sts; }
};
