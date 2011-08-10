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
#include "blt_spanutils_x86.h"

#ifdef _MSC_VER
	#pragma warning(disable: 4799)		// warning C4799: function 'nsVDPixmapSpanUtils::vdasm_horiz_expand2x_coaligned_ISSE' has no EMMS instruction
#endif

extern "C" void __cdecl vdasm_horiz_expand2x_coaligned_ISSE(void *dst, const void *src, uint32 count);
extern "C" void __cdecl vdasm_horiz_expand4x_coaligned_MMX(void *dst, const void *src, uint32 count);
extern "C" void __cdecl vdasm_vert_average_13_ISSE(void *dst, const void *src1, const void *src3, uint32 count);
extern "C" void __cdecl vdasm_vert_average_17_ISSE(void *dst, const void *src1, const void *src3, uint32 count);
extern "C" void __cdecl vdasm_vert_average_35_ISSE(void *dst, const void *src1, const void *src3, uint32 count);

namespace nsVDPixmapSpanUtils {

	void horiz_expand2x_coaligned_ISSE(uint8 *dst, const uint8 *src, sint32 w) {
		if (w >= 17) {
			uint32 fastcount = (w - 1) & ~15;

			vdasm_horiz_expand2x_coaligned_ISSE(dst, src, fastcount);
			dst += fastcount;
			src += fastcount >> 1;
			w -= fastcount;
		}

		w = -w;
		if ((w+=2) < 0) {
			do {
				dst[0] = src[0];
				dst[1] = (uint8)((src[0] + src[1] + 1)>>1);
				dst += 2;
				++src;
			} while((w+=2)<0);
		}

		w -= 2;
		while(w < 0) {
			++w;
			*dst++ = src[0];
		}
	}

	void horiz_expand4x_coaligned_MMX(uint8 *dst, const uint8 *src, sint32 w) {
		if (w >= 17) {
			uint32 fastcount = (w - 1) >> 4;

			vdasm_horiz_expand4x_coaligned_MMX(dst, src, fastcount);
			dst += fastcount << 4;
			src += fastcount << 2;
			w -= fastcount << 4;
		}

		w = -w;
		if ((w+=4) < 0) {
			do {
				dst[0] = src[0];
				dst[1] = (uint8)((3*src[0] + src[1] + 2)>>2);
				dst[2] = (uint8)((src[0] + src[1] + 1)>>1);
				dst[3] = (uint8)((src[0] + 3*src[1] + 2)>>2);
				dst += 4;
				++src;
			} while((w+=4)<0);
		}

		w -= 4;
		while(w < 0) {
			++w;
			*dst++ = src[0];
		}
	}

	void vert_expand2x_centered_ISSE(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase) {
		const uint8 *src3 = srcs[0];
		const uint8 *src1 = srcs[1];

		if (phase >= 128)
			std::swap(src1, src3);

		uint32 fastcount = w & ~15;

		if (fastcount) {
			vdasm_vert_average_13_ISSE(dst, src1, src3, fastcount);
			dst += fastcount;
			src1 += fastcount;
			src3 += fastcount;
			w -= fastcount;
		}

		if (w) {
			do {
				*dst++ = (uint8)((*src1++ + 3**src3++ + 2) >> 2);
			} while(--w);
		}
	}

	void vert_average_1_7_ISSE(uint8 *dst, const uint8 *src7, const uint8 *src1, sint32 w) {
		uint32 fastcount = w & ~7;

		if (fastcount) {
			vdasm_vert_average_17_ISSE(dst, src1, src7, fastcount);
			dst += fastcount;
			src1 += fastcount;
			src7 += fastcount;
			w -= fastcount;
		}

		if (w) {
			do {
				*dst++ = (uint8)((*src1++ + 7**src7++ + 4) >> 3);
			} while(--w);
		}
	}

	void vert_average_3_5_ISSE(uint8 *dst, const uint8 *src7, const uint8 *src1, sint32 w) {
		uint32 fastcount = w & ~7;

		if (fastcount) {
			vdasm_vert_average_35_ISSE(dst, src1, src7, fastcount);
			dst += fastcount;
			src1 += fastcount;
			src7 += fastcount;
			w -= fastcount;
		}

		if (w) {
			do {
				*dst++ = (uint8)((3**src1++ + 5**src7++ + 4) >> 3);
			} while(--w);
		}
	}

	void vert_expand4x_centered_ISSE(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase) {
		const uint8 *src1 = srcs[0];
		const uint8 *src2 = srcs[1];

		switch(phase & 0xc0) {
		case 0x00:
			vert_average_1_7_ISSE(dst, src2, src1, w);
			break;
		case 0x40:
			vert_average_3_5_ISSE(dst, src2, src1, w);
			break;
		case 0x80:
			vert_average_3_5_ISSE(dst, src1, src2, w);
			break;
		case 0xc0:
			vert_average_1_7_ISSE(dst, src1, src2, w);
			break;
		default:
			VDNEVERHERE;
		}
	}
}
