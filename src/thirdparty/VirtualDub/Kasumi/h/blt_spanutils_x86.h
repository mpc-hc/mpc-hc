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

#ifndef f_VD2_KASUMI_BLT_SPANUTILS_X86_H
#define f_VD2_KASUMI_BLT_SPANUTILS_X86_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>

namespace nsVDPixmapSpanUtils {
	void horiz_expand2x_coaligned_ISSE(uint8 *dst, const uint8 *src, sint32 w);
	void horiz_expand4x_coaligned_MMX(uint8 *dst, const uint8 *src, sint32 w);
	void vert_expand2x_centered_ISSE(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase);
	void vert_expand4x_centered_ISSE(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase);
}

#endif
