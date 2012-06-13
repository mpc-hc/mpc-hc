//  VirtualDub - Video processing and capture application
//  Graphics support library
//  Copyright (C) 1998-2007 Avery Lee
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  Notes:
//  - VDPixmapBlt is from VirtualDub
//  (- vd.cpp/h should be renamed to something more sensible already :)

#pragma once

#ifndef _WIN64
void yuvtoyuy2row_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width);
void yuvtoyuy2row_avg_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv);

void yv12_yuy2_row_sse2();
void yv12_yuy2_row_sse2_linear();
void yv12_yuy2_row_sse2_linear_interlaced();
void yv12_yuy2_sse2(const BYTE *Y, const BYTE *U, const BYTE *V, int halfstride, unsigned halfwidth, unsigned height, BYTE *YUY2, int d_stride);
void yv12_yuy2_sse2_interlaced(const BYTE *Y, const BYTE *U, const BYTE *V, int halfstride, unsigned halfwidth, unsigned height, BYTE *YUY2, int d_stride);
#endif
