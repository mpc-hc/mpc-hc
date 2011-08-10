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

#ifndef f_VD2_KASUMI_PIXEL_H
#define f_VD2_KASUMI_PIXEL_H

#ifndef f_VD2_SYSTEM_VDTYPES_H
	#include <vd2/system/vdtypes.h>
#endif

struct VDPixmap;

uint32 VDPixmapSample(const VDPixmap& px, sint32 x, sint32 y);
uint32 VDPixmapInterpolateSampleRGB24(const VDPixmap& px, sint32 x, sint32 y);

inline uint8 VDPixmapSample8(const void *data, ptrdiff_t pitch, sint32 x, sint32 y) {
	return ((const uint8 *)data)[pitch*y + x];
}

uint8 VDPixmapInterpolateSample8(const void *data, ptrdiff_t pitch, uint32 w, uint32 h, sint32 x_256, sint32 y_256);
uint32 VDConvertYCbCrToRGB(uint8 y, uint8 cb, uint8 cr, bool use709, bool useFullRange);
uint32 VDConvertRGBToYCbCr(uint32 c);
uint32 VDConvertRGBToYCbCr(uint8 r, uint8 g, uint8 b, bool use709, bool useFullRange);

#endif
