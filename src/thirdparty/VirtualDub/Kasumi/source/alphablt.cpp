//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2009 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>
#include <vd2/system/math.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/Kasumi/pixmapops.h>

void VDPixmapBltAlphaConst8(uint8 *dst, ptrdiff_t dstpitch, const uint8 *src, ptrdiff_t srcpitch, uint32 w, uint32 h, uint32 ialpha);

bool VDPixmapBltAlphaConst(const VDPixmap& dst, const VDPixmap& src, float alpha) {
	if (!(alpha >= 0.0f))
		alpha = 0.0f;
	else if (!(alpha <= 1.0f))
		alpha = 1.0f;

	uint32 ialpha = VDRoundToInt32(alpha * 256.0f);

	// format check
	if (dst.format != src.format || !src.format)
		return false;

	// degenerate case check
	if (!dst.w || !dst.h)
		return false;

	// size check
	if (src.w != dst.w || src.h != dst.h)
		return false;

	// check for formats that are not 8bpp
	switch(src.format) {
		case nsVDPixmap::kPixFormat_Pal1:
		case nsVDPixmap::kPixFormat_Pal2:
		case nsVDPixmap::kPixFormat_Pal4:
		case nsVDPixmap::kPixFormat_Pal8:
		case nsVDPixmap::kPixFormat_RGB565:
		case nsVDPixmap::kPixFormat_XRGB1555:
			return false;
	}

	const VDPixmapFormatInfo& formatInfo = VDPixmapGetInfo(src.format);

	const int qw = -(-dst.w >> formatInfo.qwbits);
	const int qh = -(-dst.h >> formatInfo.qhbits);
	const int auxw = -(-dst.w >> formatInfo.auxwbits);
	const int auxh = -(-dst.h >> formatInfo.auxhbits);

	switch(formatInfo.auxbufs) {
	case 2:
		VDPixmapBltAlphaConst8((uint8 *)dst.data3, dst.pitch3, (const uint8 *)src.data3, src.pitch3, auxw, auxh, ialpha);
	case 1:
		VDPixmapBltAlphaConst8((uint8 *)dst.data2, dst.pitch2, (const uint8 *)src.data2, src.pitch2, auxw, auxh, ialpha);
	case 0:
		VDPixmapBltAlphaConst8((uint8 *)dst.data, dst.pitch, (const uint8 *)src.data, src.pitch, formatInfo.qsize * qw, qh, ialpha);
	}

	return true;
}

void VDPixmapBltAlphaConst8(uint8 *dst, ptrdiff_t dstpitch, const uint8 *src, ptrdiff_t srcpitch, uint32 w, uint32 h, uint32 ialpha) {
	dstpitch -= w;
	srcpitch -= w;
	do {
		uint32 w2 = w;
		do {
			sint32 sc = *src;
			sint32 dc = *dst;

			*dst = dc + (((sc-dc)*ialpha + 128) >> 8);
			++src;
			++dst;
		} while(--w2);

		src += srcpitch;
		dst += dstpitch;
	} while(--h);
}
