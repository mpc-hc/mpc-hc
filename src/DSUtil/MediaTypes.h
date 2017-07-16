/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#include <dvdmedia.h>

#pragma pack(push, 1)
struct VIH {
    VIDEOINFOHEADER vih;
    UINT mask[3];
    int size;
    const GUID* subtype;
};

struct VIH2 {
    VIDEOINFOHEADER2 vih;
    UINT mask[3];
    int size;
    const GUID* subtype;
};
#pragma pack(pop)

extern const VIH vihs[];
extern const VIH2 vih2s[];

extern const UINT VIHSIZE;

extern CString VIH2String(int i), Subtype2String(const GUID& subtype);
extern void CorrectMediaType(AM_MEDIA_TYPE* pmt);
