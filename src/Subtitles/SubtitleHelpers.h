/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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

#include <afx.h>
#include <atlcoll.h>

#define MS2RT(t)        (10000i64 * (t))
#define RT2MS(t)        ((t) / 10000)
#define UNITS_FLOAT     (10000000.0)

namespace Subtitle
{
    enum SubType {
        SRT = 0,
        SUB,
        SMI,
        PSB,
        SSA,
        ASS,
        IDX,
        USF,
        XSS,
        TXT,
        RT,
        SUP
    };

    enum HearingImpairedType {
        HI_UNKNOWN = -1,
        HI_NO = 0,
        HI_YES = 1
    };

    LPCTSTR GetSubtitleFileExt(SubType type);
    bool IsTextSubtitleFileName(CString filename);

    struct SubFile {
        CString fn; /*SubType type;*/

        bool operator <(const SubFile& rhs) const {
            return fn.CompareNoCase(rhs.fn) < 0;
        }
    };

    void GetSubFileNames(CString fn, const CAtlArray<CString>& paths, CAtlArray<SubFile>& ret);

    CString GuessSubtitleName(const CString& fn, CString videoName, LCID& lcid, HearingImpairedType& hi);
};
