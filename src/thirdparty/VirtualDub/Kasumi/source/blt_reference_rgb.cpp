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
#include <vd2/system/vdtypes.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>

#define DECLARE_RGB(x, y) void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)

///////////////////////////////////////////////////////////////////////////
//
//	RGB blitters: -> XRGB1555
//
///////////////////////////////////////////////////////////////////////////

DECLARE_RGB(RGB565, XRGB1555) {
	const uint16 *src = (const uint16 *)src0;
	uint16 *dst = (uint16 *)dst0;

	srcpitch -= 2*w;
	dstpitch -= 2*w;

	do {
		int wt = w;

		do {
			const uint32 px = *src++;
			*dst++ = (px&0x001f) + ((px&0xffc0)>>1);
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(RGB888, XRGB1555) {
	const uint8 *src = (const uint8 *)src0;
	uint16 *dst = (uint16 *)dst0;

	srcpitch -= 3*w;
	dstpitch -= 2*w;

	do {
		int wt = w;

		do {
			const uint32 r = ((uint32)src[2] & 0xf8) << 7;
			const uint32 g = ((uint32)src[1] & 0xf8) << 2;
			const uint32 b = (uint32)src[0] >> 3;
			src += 3;

			*dst++ = (uint16)(r + g + b);
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(XRGB8888, XRGB1555) {
	const uint8 *src = (const uint8 *)src0;
	uint16 *dst = (uint16 *)dst0;

	srcpitch -= 4*w;
	dstpitch -= 2*w;

	do {
		int wt = w;

		do {
			const uint32 r = ((uint32)src[2] & 0xf8) << 7;
			const uint32 g = ((uint32)src[1] & 0xf8) << 2;
			const uint32 b = (uint32)src[0] >> 3;
			src += 4;

			*dst++ = (uint16)(r + g + b);
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

///////////////////////////////////////////////////////////////////////////
//
//	RGB blitters: -> RGB565
//
///////////////////////////////////////////////////////////////////////////

DECLARE_RGB(XRGB1555, RGB565) {
	const uint16 *src = (const uint16 *)src0;
	uint16 *dst = (uint16 *)dst0;

	srcpitch -= 2*w;
	dstpitch -= 2*w;

	do {
		int wt = w;

		do {
			const uint32 px = *src++;
			*dst++ = (uint16)(px + (px&0xffe0) + ((px&0x0200)>>4));
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(RGB888, RGB565) {
	const uint8 *src = (const uint8 *)src0;
	uint16 *dst = (uint16 *)dst0;

	srcpitch -= 3*w;
	dstpitch -= 2*w;

	do {
		int wt = w;

		do {
			const uint32 r = ((uint32)src[2] & 0xf8) << 8;
			const uint32 g = ((uint32)src[1] & 0xfc) << 3;
			const uint32 b = (uint32)src[0] >> 3;
			src += 3;

			*dst++ = (uint16)(r + g + b);
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(XRGB8888, RGB565) {
	const uint8 *src = (const uint8 *)src0;
	uint16 *dst = (uint16 *)dst0;

	srcpitch -= 4*w;
	dstpitch -= 2*w;

	do {
		int wt = w;

		do {
			const uint32 r = ((uint32)src[2] & 0xf8) << 8;
			const uint32 g = ((uint32)src[1] & 0xfc) << 3;
			const uint32 b = (uint32)src[0] >> 3;
			src += 4;

			*dst++ = (uint16)(r + g + b);
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

///////////////////////////////////////////////////////////////////////////
//
//	RGB blitters: -> RGB888
//
///////////////////////////////////////////////////////////////////////////

DECLARE_RGB(XRGB1555, RGB888) {
	const uint16 *src = (const uint16 *)src0;
	uint8 *dst = (uint8 *)dst0;

	srcpitch -= 2*w;
	dstpitch -= 3*w;

	do {
		int wt = w;

		do {
			const uint32 px = *src++;
			uint32 rb = px & 0x7c1f;
			uint32 g = px & 0x03e0;

			rb += rb<<5;
			g += g<<5;

			dst[0] = (uint8)(rb>>2);
			dst[1] = (uint8)(g>>7);
			dst[2] = (uint8)(rb>>12);
			dst += 3;
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(RGB565, RGB888) {
	const uint16 *src = (const uint16 *)src0;
	uint8 *dst = (uint8 *)dst0;

	srcpitch -= 2*w;
	dstpitch -= 3*w;

	do {
		int wt = w;

		do {
			const uint32 px = *src++;
			uint32 rb = px & 0xf81f;
			uint32 g = px & 0x07e0;

			rb += rb<<5;
			g += g<<6;

			dst[0] = (uint8)(rb>>2);
			dst[1] = (uint8)(g>>9);
			dst[2] = (uint8)(rb>>13);
			dst += 3;
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(XRGB8888, RGB888) {
	const uint8 *src = (const uint8 *)src0;
	uint8 *dst = (uint8 *)dst0;

	srcpitch -= 4*w;
	dstpitch -= 3*w;

	do {
		int wt = w;

		do {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst += 3;
			src += 4;
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

///////////////////////////////////////////////////////////////////////////
//
//	RGB blitters: -> XRGB8888
//
///////////////////////////////////////////////////////////////////////////

DECLARE_RGB(XRGB1555, XRGB8888) {
	const uint16 *src = (const uint16 *)src0;
	uint32 *dst = (uint32 *)dst0;

	srcpitch -= 2*w;
	dstpitch -= 4*w;

	do {
		int wt = w;

		do {
			const uint32 px = *src++;
			const uint32 rgb = ((px & 0x7c00) << 9) + ((px & 0x03e0) << 6) + ((px & 0x001f) << 3);

			*dst++ = rgb + ((rgb & 0xe0e0e0)>>5);
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(RGB565, XRGB8888) {
	const uint16 *src = (const uint16 *)src0;
	uint32 *dst = (uint32 *)dst0;

	srcpitch -= 2*w;
	dstpitch -= 4*w;

	do {
		int wt = w;

		do {
			const uint32 px = *src++;
			const uint32 rb = ((px & 0xf800) << 8) + ((px & 0x001f) << 3);
			const uint32 g = ((px & 0x07e0) << 5) + (px & 0x0300);

			*dst++ = rb + ((rb & 0xe000e0)>>5) + g;
		} while(--wt);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_RGB(RGB888, XRGB8888) {
	const uint8 *src = (const uint8 *)src0;
	uint32 *dst = (uint32 *)dst0;

	srcpitch -= 3*w;
	dstpitch -= 4*w;

	do {
		int wt = w;

		do {
			*dst++ = (uint32)src[0] + ((uint32)src[1]<<8) + ((uint32)src[2]<<16);
			src += 3;
		} while(--wt);
		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}
