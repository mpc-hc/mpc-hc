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
#include "blt_spanutils.h"
#include "bitutils.h"

using namespace nsVDPixmapBitUtils;

namespace nsVDPixmapSpanUtils {
	void horiz_expand2x_centered(uint8 *dst, const uint8 *src, sint32 w) {
		w = -w;

		*dst++ = *src;

		if (++w) {
			if (++w) {
				do {
					dst[0] = (uint8)((3*src[0] + src[1] + 2)>>2);
					dst[1] = (uint8)((src[0] + 3*src[1] + 2)>>2);
					dst += 2;
					++src;
				} while((w+=2)<0);
			}

			if (!(w & 1)) {
				*dst = src[0];
			}
		}
	}

	void horiz_expand2x_coaligned(uint8 *dst, const uint8 *src, sint32 w) {
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

	void horiz_expand4x_coaligned(uint8 *dst, const uint8 *src, sint32 w) {
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

	void horiz_compress2x_coaligned(uint8 *dst, const uint8 *src, sint32 w) {
		if (w == 1) {
			*dst = *src;
			return;
		}

		*dst++ = (uint8)((3*src[0] + src[1] + 2) >> 2);
		++src;
		--w;

		while(w >= 3) {
			w -= 2;
			*dst++ = (uint8)((src[0] + 2*src[1] + src[2] + 2) >> 2);
			src += 2;
		}

		if (w >= 2)
			*dst++ = (uint8)((src[0] + 3*src[1] + 2) >> 2);
	}

	void horiz_compress2x_centered(uint8 *dst, const uint8 *src, sint32 w) {
		if (w == 1) {
			*dst = *src;
			return;
		}

		if (w == 2) {
			*dst = (uint8)((src[0] + src[1] + 1) >> 1);
			return;
		}

		*dst++ = (uint8)((4*src[0] + 3*src[1] + src[2] + 4) >> 3);
		--w;
		++src;

		while(w >= 4) {
			w -= 2;
			*dst++ = (uint8)(((src[0] + src[3]) + 3*(src[1] + src[2]) + 4) >> 3);
			src += 2;
		}

		switch(w) {
		case 3:
			*dst++ = (uint8)((src[0] + 3*src[1] + 4*src[2] + 4) >> 3);
			break;
		case 2:
			*dst++ = (uint8)((src[0] + 7*src[1] + 4) >> 3);
			break;
		}
	}

	void horiz_compress4x_coaligned(uint8 *dst, const uint8 *src, sint32 w) {
		if (w == 1) {
			*dst = *src;
			return;
		}

		if (w == 2) {
			*dst++ = (uint8)((11*src[0] + 5*src[1] + 8) >> 4);
			return;
		}

		*dst++ = (uint8)((11*src[0] + 4*src[1] + src[2] + 8) >> 4);
		src += 2;
		w -= 2;

		while(w >= 5) {
			w -= 4;
			*dst++ = (uint8)(((src[0] + src[4]) + 4*(src[1] + src[3]) + 6*src[2] + 8) >> 4);
			src += 4;
		}

		switch(w) {
		case 4:
			*dst = (uint8)((src[0] + 4*src[1] + 6*src[2] + 5*src[3] + 8) >> 4);
			break;
		case 3:
			*dst = (uint8)((src[0] + 4*src[1] + 11*src[2] + 8) >> 4);
			break;
		}
	}

	void horiz_compress4x_centered(uint8 *dst, const uint8 *src, sint32 w) {

		switch(w) {
		case 1:
			*dst = *src;
			return;
		case 2:		// 29 99
			*dst = (uint8)((29*src[0] + 99*src[1] + 64) >> 7);
			return;
		case 3:		// 29 35 64
			*dst = (uint8)((29*src[0] + 35*src[1] + 64*src[1] + 64) >> 7);
			return;
		case 4:		// 29 35 35 29
			*dst = (uint8)((29*src[0] + 35*(src[1] + src[2]) + 29*src[3] + 64) >> 7);
			return;
		case 5:		// 29 35 35 21 8
					//        1 7 120
			dst[0] = (uint8)((29*src[0] + 35*(src[1] + src[2]) + 21*src[3] + 8*src[4] + 64) >> 7);
			dst[1] = (uint8)((src[2] + 7*src[3] + 120*src[4] + 64) >> 7);
			return;
		}

		*dst++ = (uint8)((29*src[0] + 35*(src[1] + src[2]) + 21*src[3] + 7*src[4] + src[5] + 64) >> 7);
		src += 2;
		w -= 2;

		while(w >= 8) {
			w -= 4;
			*dst++ = (uint8)(((src[0] + src[7]) + 7*(src[1] + src[6]) + 21*(src[2] + src[5]) + 35*(src[3] + src[4]) + 64) >> 7);
			src += 4;
		}

		switch(w) {
		case 4:		// 1 7 21 99
			*dst = (uint8)((src[0] + 7*src[1] + 21*src[2] + 99*src[3] + 64) >> 7);
			break;
		case 5:		// 1 7 21 35 64
			*dst = (uint8)((src[0] + 7*src[1] + 21*src[2] + 35*src[3] + 64*src[4] + 64) >> 7);
			break;
		case 6:		// 1 7 21 35 35 29
			*dst = (uint8)((src[0] + 7*src[1] + 21*src[2] + 29*src[5] + 35*(src[3] + src[4]) + 64) >> 7);
			break;
		case 7:		// 1 7 21 35 35 21 8
					//            1 7 120
			dst[0] = (uint8)((src[0] + 7*src[1] + 8*src[6] + 21*(src[2] + src[5]) + 35*(src[3] + src[4]) + 64) >> 7);
			dst[1] = (uint8)((src[4] + 7*src[5] + 120*src[6] + 64) >> 7);
			break;
		}
	}

	void horiz_realign_to_centered(uint8 *dst, const uint8 *src, sint32 w) {
		// luma samples:	Y		Y		Y		Y		Y
		// coaligned:		C				C				C
		// centered:			C				C
		//
		// To realign coaligned samples to centered, we need to shift them
		// right by a quarter sample in chroma space. This can be done via
		// a [3 1]/4 filter.

		for(sint32 i=1; i<w; ++i) {
			dst[0] = (uint8)((3*(uint32)src[0] + (uint32)src[1] + 2) >> 2);
			++dst;
			++src;
		}

		*dst++ = *src++;
	}

	void horiz_realign_to_coaligned(uint8 *dst, const uint8 *src, sint32 w) {
		// luma samples:	Y		Y		Y		Y		Y
		// coaligned:		C				C				C
		// centered:			C				C
		//
		// To realign centered samples to coaligned, we need to shift them
		// left by a quarter sample in chroma space. This can be done via
		// a [1 3]/4 filter.

		*dst++ = *src++;

		for(sint32 i=1; i<w; ++i) {
			dst[0] = (uint8)(((uint32)src[-1] + 3*(uint32)src[0] + 2) >> 2);
			++dst;
			++src;
		}
	}

	void vert_expand2x_centered(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase) {
		const uint8 *src3 = srcs[0];
		const uint8 *src1 = srcs[1];

		if (phase >= 128)
			std::swap(src1, src3);

		sint32 w4 = w>>2;
		w &= 3;

		if (w4) {
			const uint32 *src34 = (const uint32 *)src3;
			const uint32 *src14 = (const uint32 *)src1;
			      uint32 *dst4  = (      uint32 *)dst;

			do {
				const uint32 a = *src34++;
				const uint32 b = *src14++;
				const uint32 ab = (a&b) + (((a^b)&0xfefefefe)>>1);

				*dst4++ = (a|ab) - (((a^ab)&0xfefefefe)>>1);
			} while(--w4);

			src3 = (const uint8 *)src34;
			src1 = (const uint8 *)src14;
			dst  = (      uint8 *)dst4;
		}

		if (w) {
			do {
				*dst++ = (uint8)((*src1++ + 3**src3++ + 2) >> 2);
			} while(--w);
		}
	}

	void vert_expand4x_centered(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase) {
		const uint8 *src3 = srcs[0];
		const uint8 *src1 = srcs[1];

		switch(phase & 0xc0) {
		case 0x00:
			do {
				*dst++ = (uint8)((1**src1++ + 7**src3++ + 4) >> 3);
			} while(--w);
			break;
		case 0x40:
			do {
				*dst++ = (uint8)((3**src1++ + 5**src3++ + 4) >> 3);
			} while(--w);
			break;
		case 0x80:
			do {
				*dst++ = (uint8)((5**src1++ + 3**src3++ + 4) >> 3);
			} while(--w);
			break;
		case 0xc0:
			do {
				*dst++ = (uint8)((7**src1++ + 1**src3++ + 4) >> 3);
			} while(--w);
			break;
		default:
			VDNEVERHERE;
		}
	}

	void vert_compress2x_centered_fast(uint8 *dst, const uint8 *const *srcarray, sint32 w, uint8 phase) {
		const uint8 *src1 = srcarray[0];
		const uint8 *src2 = srcarray[1];

		w = -w;
		w += 3;

		while(w < 0) {
			*(uint32 *)dst = avg_8888_11(*(uint32 *)src1, *(uint32 *)src2);
			dst += 4;
			src1 += 4;
			src2 += 4;
			w += 4;
		}

		w -= 3;

		while(w < 0) {
			*dst = (uint8)((*src1 + *src2 + 1)>>1);
			++dst;
			++src1;
			++src2;
			++w;
		}
	}

	void vert_compress2x_centered(uint8 *dst, const uint8 *const *srcarray, sint32 w, uint8 phase) {
		const uint8 *src1 = srcarray[0];
		const uint8 *src2 = srcarray[1];
		const uint8 *src3 = srcarray[2];
		const uint8 *src4 = srcarray[3];

		w = -w;

		while(w < 0) {
			*dst++ = (uint8)(((*src1++ + *src4++) + 3*(*src2++ + *src3++) + 4)>>3);
			++w;
		}
	}

	void vert_compress4x_centered(uint8 *dst, const uint8 *const *srcarray, sint32 w, uint8 phase) {
		const uint8 *src1 = srcarray[0];
		const uint8 *src2 = srcarray[1];
		const uint8 *src3 = srcarray[2];
		const uint8 *src4 = srcarray[3];
		const uint8 *src5 = srcarray[4];
		const uint8 *src6 = srcarray[5];
		const uint8 *src7 = srcarray[6];
		const uint8 *src8 = srcarray[7];

		w = -w;

		while(w < 0) {
			int sum18 = *src1++ + *src8++;
			int sum27 = *src2++ + *src7++;
			int sum36 = *src3++ + *src6++;
			int sum45 = *src4++ + *src5++;

			*dst++ = (uint8)((sum18 + 7*sum27 + 21*sum36 + 35*sum45 + 64) >> 7);

			++w;
		}
	}
}

