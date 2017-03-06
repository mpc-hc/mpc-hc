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
#include <vd2/system/vdstl.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/system/memory.h>
#include <vd2/system/win32/intrin.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>

#include "blt_spanutils.h"

#ifdef _M_IX86
	#include "blt_spanutils_x86.h"
#endif

using namespace nsVDPixmapSpanUtils;

namespace {
	struct YCbCrToRGB {
		sint16 y_tab[256];
		sint16 r_cr_tab[256];
		sint16 b_cb_tab[256];
		sint16 g_cr_tab[256];
		sint16 g_cb_tab[256];
		uint8 cliptab[277+256+279];
		uint16 cliptab15[277+256+279];
		uint16 cliptab16[277+256+279];

		YCbCrToRGB() {
			int i;

			memset(cliptab, 0, 277);
			memset(cliptab+277+256, 255, 279);

			memset(cliptab15, 0, sizeof cliptab15[0] * 277);
			memset(cliptab16, 0, sizeof cliptab16[0] * 277);
			memset(cliptab15+277+256, 0xff, sizeof cliptab15[0] * 279);
			memset(cliptab16+277+256, 0xff, sizeof cliptab16[0] * 279);

			for(i=0; i<256; ++i) {
				y_tab[i] = (sint16)(((i-16) * 76309 + 32768) >> 16);
				r_cr_tab[i] = (sint16)(((i-128) * 104597 + 32768) >> 16);
				b_cb_tab[i] = (sint16)(((i-128) * 132201 + 32768) >> 16);
				g_cr_tab[i] = (sint16)(((i-128) * -53279 + 32768) >> 16);
				g_cb_tab[i] = (sint16)(((i-128) * -25674 + 32768) >> 16);
				cliptab[i+277] = (uint8)i;
				cliptab15[i+277] = 0x421 * ((unsigned)i>>3);
				cliptab16[i+277] = 0x801 * ((unsigned)i>>3) + 0x20 * ((unsigned)i>>2);
			}
		}
	} static const colorconv;

	struct YCbCrFormatInfo {
		ptrdiff_t	ystep;
		ptrdiff_t	cstep;
		ptrdiff_t	yinc[4];
		ptrdiff_t	cinc[4];
		sint8		ypos[4];
		sint8		cbpos[4];
		sint8		crpos[4];
	};

	YCbCrFormatInfo		g_formatInfo_YUV444_Planar	= { -4, -4, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {0,1,2,3}, {0,1,2,3}, {0,1,2,3}};
	YCbCrFormatInfo		g_formatInfo_YUV422_YUYV	= { -8, -8, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {0,2,4,6}, {1,1,5,5}, {3,3,7,7}};
	YCbCrFormatInfo		g_formatInfo_YUV422_UYVY	= { -8, -8, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {1,3,5,7}, {0,0,4,4}, {2,2,6,6}};
	YCbCrFormatInfo		g_formatInfo_YUV420_YV12	= { -4, -2, {-1,-1,-1,-1}, { 0,-1, 0,-1}, {0,1,2,3}, {0,0,1,1}, {0,0,1,1}};
	YCbCrFormatInfo		g_formatInfo_YUV411_YV12	= { -4, -1, {-1,-1,-1,-1}, {-1,-1,-1,-1}, {0,1,2,3}, {0,0,0,0}, {0,0,0,0}};

	inline uint16 ycbcr_to_1555(uint8 y, uint8 cb0, uint8 cr0) {
		const uint16 *p = &colorconv.cliptab15[277 + colorconv.y_tab[y]];
		uint32 r = 0x7c00 & p[colorconv.r_cr_tab[cr0]];
		uint32 g = 0x03e0 & p[colorconv.g_cr_tab[cr0] + colorconv.g_cb_tab[cb0]];
		uint32 b = 0x001f & p[colorconv.b_cb_tab[cb0]];

		return r + g + b;
	}

	inline uint16 ycbcr_to_565(uint8 y, uint8 cb0, uint8 cr0) {
		const uint16 *p = &colorconv.cliptab16[277 + colorconv.y_tab[y]];
		uint32 r = 0xf800 & p[colorconv.r_cr_tab[cr0]];
		uint32 g = 0x07e0 & p[colorconv.g_cr_tab[cr0] + colorconv.g_cb_tab[cb0]];
		uint32 b = 0x001f & p[colorconv.b_cb_tab[cb0]];

		return r + g + b;
	}

	inline void ycbcr_to_888(uint8 *dst, uint8 y, uint8 cb0, uint8 cr0) {
		const uint8 *p = &colorconv.cliptab[277 + colorconv.y_tab[y]];
		uint8 r = p[colorconv.r_cr_tab[cr0]];
		uint8 g = p[colorconv.g_cr_tab[cr0] + colorconv.g_cb_tab[cb0]];
		uint8 b = p[colorconv.b_cb_tab[cb0]];

		dst[0] = b;
		dst[1] = g;
		dst[2] = r;
	}

	inline uint32 ycbcr_to_8888(uint8 y, uint8 cb0, uint8 cr0) {
		const uint8 *p = &colorconv.cliptab[277 + colorconv.y_tab[y]];
		uint8 r = p[colorconv.r_cr_tab[cr0]];
		uint8 g = p[colorconv.g_cr_tab[cr0] + colorconv.g_cb_tab[cb0]];
		uint8 b = p[colorconv.b_cb_tab[cb0]];

		return (r << 16) + (g << 8) + b;
	}

	void VDYCbCrToXRGB1555Span(void *dst0, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 w) {
		uint16 *dst = (uint16 *)dst0;

		do {
			*dst++ = ycbcr_to_1555(*y++, *cb++, *cr++);
		} while(--w);
	}

	void VDYCbCrToRGB565Span(void *dst0, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 w) {
		uint16 *dst = (uint16 *)dst0;

		do {
			*dst++ = ycbcr_to_565(*y++, *cb++, *cr++);
		} while(--w);
	}

	void VDYCbCrToRGB888Span(void *dst0, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 w) {
		uint8 *dst = (uint8 *)dst0;

		do {
			ycbcr_to_888(dst, *y++, *cb++, *cr++);
			dst += 3;
		} while(--w);
	}

	void VDYCbCrToXRGB8888Span(void *dst0, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 w) {
		uint32 *dst = (uint32 *)dst0;

		do {
			*dst++ = ycbcr_to_8888(*y++, *cb++, *cr++);
		} while(--w);
	}

	void VDYCbCrToUYVYSpan(void *dst0, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 w) {
		uint32 *dst = (uint32 *)dst0;

		if (--w) {
			do {
				*dst++ = (uint32)*cb++ + ((uint32)y[0] << 8) + ((uint32)*cr++ << 16) + ((uint32)y[1] << 24);
				y += 2;
			} while((sint32)(w-=2)>0);
		}

		if (!(w & 1))
			*dst++ = (uint32)*cb + ((uint32)y[0] << 8) + ((uint32)*cr << 16) + ((uint32)y[0] << 24);
	}

	void VDYCbCrToYUYVSpan(void *dst0, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 w) {
		uint32 *dst = (uint32 *)dst0;

		if (--w) {
			do {
				*dst++ = (uint32)y[0] + ((uint32)*cb++ << 8) + ((uint32)y[1] << 16) + ((uint32)*cr++ << 24);
				y += 2;
			} while((sint32)(w-=2)>0);
		}

		if (!(w & 1))
			*dst++ = (uint32)y[0] + ((uint32)*cb << 8) + ((uint32)y[0] << 16) + ((uint32)*cr << 24);
	}

	void VDYCbCrToRGB1555Generic(void *dst, ptrdiff_t dststride, const void *yrow, ptrdiff_t ystride, const void *cbrow, ptrdiff_t cbstride, const void *crrow, ptrdiff_t crstride, int w, int h, const YCbCrFormatInfo& formatinfo) {
		const ptrdiff_t ystep	= formatinfo.ystep;
		const ptrdiff_t cstep	= formatinfo.cstep;
		const ptrdiff_t ypos0	= formatinfo.ypos[0];
		const ptrdiff_t ypos1	= formatinfo.ypos[1];
		const ptrdiff_t ypos2	= formatinfo.ypos[2];
		const ptrdiff_t ypos3	= formatinfo.ypos[3];
		const ptrdiff_t crpos0	= formatinfo.crpos[0];
		const ptrdiff_t crpos1	= formatinfo.crpos[1];
		const ptrdiff_t crpos2	= formatinfo.crpos[2];
		const ptrdiff_t crpos3	= formatinfo.crpos[3];
		const ptrdiff_t cbpos0	= formatinfo.cbpos[0];
		const ptrdiff_t cbpos1	= formatinfo.cbpos[1];
		const ptrdiff_t cbpos2	= formatinfo.cbpos[2];
		const ptrdiff_t cbpos3	= formatinfo.cbpos[3];

		yrow	= (char *)yrow - ystep * ((w-1) >> 2);
		crrow	= (char *)crrow - cstep * ((w-1) >> 2);
		cbrow	= (char *)cbrow - cstep * ((w-1) >> 2);
		dst		= (char *)dst + 2*((w-1) & ~3);

		int y = 0;
		do {
			const uint8 *ysrc	= (const uint8 *)yrow;
			const uint8 *crsrc	= (const uint8 *)crrow;
			const uint8 *cbsrc	= (const uint8 *)cbrow;
			uint16 *out = (uint16 *)dst;
			int w2 = -w;

			switch(w2 & 3) {
				do {
			case 0:	out[3] = ycbcr_to_1555(ysrc[ypos3], cbsrc[cbpos3], crsrc[crpos3]);
			case 1:	out[2] = ycbcr_to_1555(ysrc[ypos2], cbsrc[cbpos2], crsrc[crpos2]);
			case 2:	out[1] = ycbcr_to_1555(ysrc[ypos1], cbsrc[cbpos1], crsrc[crpos1]);
			case 3:	out[0] = ycbcr_to_1555(ysrc[ypos0], cbsrc[cbpos0], crsrc[crpos0]);
					out -= 4;
					ysrc += ystep;
					crsrc += cstep;
					cbsrc += cstep;
				} while((w2 += 4) < 0);
			}

			dst		= (char *)dst + dststride;
			yrow	= (const char *)yrow + (ystride & formatinfo.yinc[y & 3]);
			cbrow	= (const char *)cbrow + (cbstride & formatinfo.cinc[y & 3]);
			crrow	= (const char *)crrow + (crstride & formatinfo.cinc[y & 3]);
		} while(++y < h);
	}

	void VDYCbCrToRGB565Generic(void *dst, ptrdiff_t dststride, const void *yrow, ptrdiff_t ystride, const void *cbrow, ptrdiff_t cbstride, const void *crrow, ptrdiff_t crstride, int w, int h, const YCbCrFormatInfo& formatinfo) {
		const ptrdiff_t ystep	= formatinfo.ystep;
		const ptrdiff_t cstep	= formatinfo.cstep;
		const ptrdiff_t ypos0	= formatinfo.ypos[0];
		const ptrdiff_t ypos1	= formatinfo.ypos[1];
		const ptrdiff_t ypos2	= formatinfo.ypos[2];
		const ptrdiff_t ypos3	= formatinfo.ypos[3];
		const ptrdiff_t crpos0	= formatinfo.crpos[0];
		const ptrdiff_t crpos1	= formatinfo.crpos[1];
		const ptrdiff_t crpos2	= formatinfo.crpos[2];
		const ptrdiff_t crpos3	= formatinfo.crpos[3];
		const ptrdiff_t cbpos0	= formatinfo.cbpos[0];
		const ptrdiff_t cbpos1	= formatinfo.cbpos[1];
		const ptrdiff_t cbpos2	= formatinfo.cbpos[2];
		const ptrdiff_t cbpos3	= formatinfo.cbpos[3];

		yrow	= (char *)yrow - ystep * ((w-1) >> 2);
		crrow	= (char *)crrow - cstep * ((w-1) >> 2);
		cbrow	= (char *)cbrow - cstep * ((w-1) >> 2);
		dst		= (char *)dst + 2*((w-1) & ~3);

		int y = 0;
		do {
			const uint8 *ysrc = (const uint8 *)yrow;
			const uint8 *crsrc = (const uint8 *)crrow;
			const uint8 *cbsrc = (const uint8 *)cbrow;
			uint16 *out = (uint16 *)dst;
			int w2 = -w;

			switch(w2 & 3) {
				do {
			case 0:	out[3] = ycbcr_to_565(ysrc[ypos3], cbsrc[cbpos3], crsrc[crpos3]);
			case 1:	out[2] = ycbcr_to_565(ysrc[ypos2], cbsrc[cbpos2], crsrc[crpos2]);
			case 2:	out[1] = ycbcr_to_565(ysrc[ypos1], cbsrc[cbpos1], crsrc[crpos1]);
			case 3:	out[0] = ycbcr_to_565(ysrc[ypos0], cbsrc[cbpos0], crsrc[crpos0]);
					out -= 4;
					ysrc += ystep;
					crsrc += cstep;
					cbsrc += cstep;
				} while((w2 += 4) < 0);
			}

			dst		= (char *)dst + dststride;
			yrow	= (const char *)yrow + (ystride & formatinfo.yinc[y & 3]);
			cbrow	= (const char *)cbrow + (cbstride & formatinfo.cinc[y & 3]);
			crrow	= (const char *)crrow + (crstride & formatinfo.cinc[y & 3]);
		} while(++y < h);
	}

	void VDYCbCrToRGB888Generic(void *dst, ptrdiff_t dststride, const void *yrow, ptrdiff_t ystride, const void *cbrow, ptrdiff_t cbstride, const void *crrow, ptrdiff_t crstride, int w, int h, const YCbCrFormatInfo& formatinfo) {
		const ptrdiff_t ystep	= formatinfo.ystep;
		const ptrdiff_t cstep	= formatinfo.cstep;
		const ptrdiff_t ypos0	= formatinfo.ypos[0];
		const ptrdiff_t ypos1	= formatinfo.ypos[1];
		const ptrdiff_t ypos2	= formatinfo.ypos[2];
		const ptrdiff_t ypos3	= formatinfo.ypos[3];
		const ptrdiff_t crpos0	= formatinfo.crpos[0];
		const ptrdiff_t crpos1	= formatinfo.crpos[1];
		const ptrdiff_t crpos2	= formatinfo.crpos[2];
		const ptrdiff_t crpos3	= formatinfo.crpos[3];
		const ptrdiff_t cbpos0	= formatinfo.cbpos[0];
		const ptrdiff_t cbpos1	= formatinfo.cbpos[1];
		const ptrdiff_t cbpos2	= formatinfo.cbpos[2];
		const ptrdiff_t cbpos3	= formatinfo.cbpos[3];

		yrow	= (char *)yrow - ystep * ((w-1) >> 2);
		crrow	= (char *)crrow - cstep * ((w-1) >> 2);
		cbrow	= (char *)cbrow - cstep * ((w-1) >> 2);
		dst		= (char *)dst + 3*((w-1) & ~3);

		int y = 0;
		do {
			const uint8 *ysrc	= (const uint8 *)yrow;
			const uint8 *crsrc	= (const uint8 *)crrow;
			const uint8 *cbsrc	= (const uint8 *)cbrow;
			uint8 *out = (uint8 *)dst;
			int w2 = -w;

			switch(w2 & 3) {
				do {
			case 0:	ycbcr_to_888(out+9, ysrc[ypos3], cbsrc[cbpos3], crsrc[crpos3]);
			case 1:	ycbcr_to_888(out+6, ysrc[ypos2], cbsrc[cbpos2], crsrc[crpos2]);
			case 2:	ycbcr_to_888(out+3, ysrc[ypos1], cbsrc[cbpos1], crsrc[crpos1]);
			case 3:	ycbcr_to_888(out, ysrc[ypos0], cbsrc[cbpos0], crsrc[crpos0]);
					out -= 12;
					ysrc += ystep;
					crsrc += cstep;
					cbsrc += cstep;
				} while((w2 += 4) < 0);
			}

			dst		= (char *)dst + dststride;
			yrow	= (const char *)yrow + (ystride & formatinfo.yinc[y & 3]);
			cbrow	= (const char *)cbrow + (cbstride & formatinfo.cinc[y & 3]);
			crrow	= (const char *)crrow + (crstride & formatinfo.cinc[y & 3]);
		} while(++y < h);
	}

	void VDYCbCrToRGB8888Generic(void *dst, ptrdiff_t dststride, const void *yrow, ptrdiff_t ystride, const void *cbrow, ptrdiff_t cbstride, const void *crrow, ptrdiff_t crstride, int w, int h, const YCbCrFormatInfo& formatinfo) {
		const ptrdiff_t ystep	= formatinfo.ystep;
		const ptrdiff_t cstep	= formatinfo.cstep;
		const ptrdiff_t ypos0	= formatinfo.ypos[0];
		const ptrdiff_t ypos1	= formatinfo.ypos[1];
		const ptrdiff_t ypos2	= formatinfo.ypos[2];
		const ptrdiff_t ypos3	= formatinfo.ypos[3];
		const ptrdiff_t crpos0	= formatinfo.crpos[0];
		const ptrdiff_t crpos1	= formatinfo.crpos[1];
		const ptrdiff_t crpos2	= formatinfo.crpos[2];
		const ptrdiff_t crpos3	= formatinfo.crpos[3];
		const ptrdiff_t cbpos0	= formatinfo.cbpos[0];
		const ptrdiff_t cbpos1	= formatinfo.cbpos[1];
		const ptrdiff_t cbpos2	= formatinfo.cbpos[2];
		const ptrdiff_t cbpos3	= formatinfo.cbpos[3];

		yrow	= (char *)yrow - ystep * ((w-1) >> 2);
		crrow	= (char *)crrow - cstep * ((w-1) >> 2);
		cbrow	= (char *)cbrow - cstep * ((w-1) >> 2);
		dst		= (char *)dst + 4*((w-1) & ~3);

		int y = 0;
		do {
			const uint8 *ysrc	= (const uint8 *)yrow;
			const uint8 *crsrc	= (const uint8 *)crrow;
			const uint8 *cbsrc	= (const uint8 *)cbrow;
			uint32 *out = (uint32 *)dst;
			int w2 = -w;

			switch(w2 & 3) {
				do {
			case 0:	out[3] = ycbcr_to_8888(ysrc[ypos3], cbsrc[cbpos3], crsrc[crpos3]);
			case 1:	out[2] = ycbcr_to_8888(ysrc[ypos2], cbsrc[cbpos2], crsrc[crpos2]);
			case 2:	out[1] = ycbcr_to_8888(ysrc[ypos1], cbsrc[cbpos1], crsrc[crpos1]);
			case 3:	out[0] = ycbcr_to_8888(ysrc[ypos0], cbsrc[cbpos0], crsrc[crpos0]);
					out -= 4;
					ysrc += ystep;
					crsrc += cstep;
					cbsrc += cstep;
				} while((w2 += 4) < 0);
			}

			dst		= (char *)dst + dststride;
			yrow	= (const char *)yrow + (ystride & formatinfo.yinc[y & 3]);
			cbrow	= (const char *)cbrow + (cbstride & formatinfo.cinc[y & 3]);
			crrow	= (const char *)crrow + (crstride & formatinfo.cinc[y & 3]);
		} while(++y < h);
	}
}

#define DECLARE_YUV(x, y) void VDPixmapBlt_##x##_to_##y##_reference(void *dst0, ptrdiff_t dstpitch, const void *src0, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h)

DECLARE_YUV(UYVY, XRGB1555) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint16 *dst = (uint16 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint16 *y;

		cb = src[0];
		cr = src[2];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab15[277 + colorconv.y_tab[src[1]]];
		*dst++ = (y[rc1] & 0x7c00) + (y[gc1] & 0x3e0) + (y[bc1] & 0x001f);

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[4];
				cr = src[6];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab15[277 + colorconv.y_tab[src[3]]];
				dst[0] = (y[(rc0+rc1+1)>>1] & 0x7c00) + (y[(gc0+gc1+1)>>1] & 0x3e0) + (y[(bc0+bc1+1)>>1] & 0x001f);

				y = &colorconv.cliptab15[277 + colorconv.y_tab[src[5]]];
				dst[1] = (y[rc1] & 0x7c00) + (y[gc1] & 0x3e0) + (y[bc1] & 0x001f);

				dst += 2;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab15[277 + colorconv.y_tab[src[3]]];
			*dst = (y[rc1] & 0x7c00) + (y[gc1] & 0x3e0) + (y[bc1] & 0x001f);
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(UYVY, RGB565) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint16 *dst = (uint16 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint16 *y;

		cb = src[0];
		cr = src[2];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab16[277 + colorconv.y_tab[src[1]]];
		*dst++ = (y[rc1] & 0xf800) + (y[gc1] & 0x7e0) + (y[bc1] & 0x001f);

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[4];
				cr = src[6];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab16[277 + colorconv.y_tab[src[3]]];
				dst[0] = (y[(rc0+rc1+1)>>1] & 0xf800) + (y[(gc0+gc1+1)>>1] & 0x7e0) + (y[(bc0+bc1+1)>>1] & 0x001f);

				y = &colorconv.cliptab16[277 + colorconv.y_tab[src[5]]];
				dst[1] = (y[rc1] & 0xf800) + (y[gc1] & 0x7e0) + (y[bc1] & 0x001f);

				dst += 2;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab16[277 + colorconv.y_tab[src[3]]];
			*dst = (y[rc1] & 0xf800) + (y[gc1] & 0x7e0) + (y[bc1] & 0x001f);
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(UYVY, RGB888) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint8 *dst = (uint8 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint8 *y;

		cb = src[0];
		cr = src[2];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab[277 + colorconv.y_tab[src[1]]];
		dst[0] = y[bc1];
		dst[1] = y[gc1];
		dst[2] = y[rc1];
		dst += 3;

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[4];
				cr = src[6];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[3]]];
				dst[0] = y[(bc0+bc1+1)>>1];
				dst[1] = y[(gc0+gc1+1)>>1];
				dst[2] = y[(rc0+rc1+1)>>1];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[5]]];
				dst[3] = y[bc1];
				dst[4] = y[gc1];
				dst[5] = y[rc1];

				dst += 6;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab[277 + colorconv.y_tab[src[3]]];
			dst[0] = y[bc1];
			dst[1] = y[gc1];
			dst[2] = y[rc1];
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(UYVY, XRGB8888) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint8 *dst = (uint8 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint8 *y;

		cb = src[0];
		cr = src[2];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab[277 + colorconv.y_tab[src[1]]];
		dst[0] = y[bc1];
		dst[1] = y[gc1];
		dst[2] = y[rc1];
		dst += 4;

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[4];
				cr = src[6];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[3]]];
				dst[0] = y[(bc0+bc1+1)>>1];
				dst[1] = y[(gc0+gc1+1)>>1];
				dst[2] = y[(rc0+rc1+1)>>1];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[5]]];
				dst[4] = y[bc1];
				dst[5] = y[gc1];
				dst[6] = y[rc1];

				dst += 8;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab[277 + colorconv.y_tab[src[3]]];
			dst[0] = y[bc1];
			dst[1] = y[gc1];
			dst[2] = y[rc1];
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(YUYV, XRGB1555) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint16 *dst = (uint16 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint16 *y;

		cb = src[1];
		cr = src[3];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab15[277 + colorconv.y_tab[src[0]]];
		*dst++ = (y[rc1] & 0x7c00) + (y[gc1] & 0x3e0) + (y[bc1] & 0x001f);

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[5];
				cr = src[7];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab15[277 + colorconv.y_tab[src[2]]];
				dst[0] = (y[(rc0+rc1+1)>>1] & 0x7c00) + (y[(gc0+gc1+1)>>1] & 0x3e0) + (y[(bc0+bc1+1)>>1] & 0x001f);

				y = &colorconv.cliptab15[277 + colorconv.y_tab[src[4]]];
				dst[1] = (y[rc1] & 0x7c00) + (y[gc1] & 0x3e0) + (y[bc1] & 0x001f);

				dst += 2;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab15[277 + colorconv.y_tab[src[2]]];
			*dst = (y[rc1] & 0x7c00) + (y[gc1] & 0x3e0) + (y[bc1] & 0x001f);
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(YUYV, RGB565) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint16 *dst = (uint16 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint16 *y;

		cb = src[1];
		cr = src[3];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab16[277 + colorconv.y_tab[src[0]]];
		*dst++ = (y[rc1] & 0xf800) + (y[gc1] & 0x7e0) + (y[bc1] & 0x001f);

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[5];
				cr = src[7];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab16[277 + colorconv.y_tab[src[2]]];
				dst[0] = (y[(rc0+rc1+1)>>1] & 0xf800) + (y[(gc0+gc1+1)>>1] & 0x7e0) + (y[(bc0+bc1+1)>>1] & 0x001f);

				y = &colorconv.cliptab16[277 + colorconv.y_tab[src[4]]];
				dst[1] = (y[rc1] & 0xf800) + (y[gc1] & 0x7e0) + (y[bc1] & 0x001f);

				dst += 2;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab16[277 + colorconv.y_tab[src[2]]];
			*dst = (y[rc1] & 0xf800) + (y[gc1] & 0x7e0) + (y[bc1] & 0x001f);
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(YUYV, RGB888) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint8 *dst = (uint8 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint8 *y;

		cb = src[1];
		cr = src[3];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab[277 + colorconv.y_tab[src[0]]];
		dst[0] = y[bc1];
		dst[1] = y[gc1];
		dst[2] = y[rc1];
		dst += 3;

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[5];
				cr = src[7];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[2]]];
				dst[0] = y[(bc0+bc1+1)>>1];
				dst[1] = y[(gc0+gc1+1)>>1];
				dst[2] = y[(rc0+rc1+1)>>1];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[4]]];
				dst[3] = y[bc1];
				dst[4] = y[gc1];
				dst[5] = y[rc1];

				dst += 6;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab[277 + colorconv.y_tab[src[2]]];
			dst[0] = y[bc1];
			dst[1] = y[gc1];
			dst[2] = y[rc1];
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(YUYV, XRGB8888) {
	do {
		const uint8 *src = (const uint8 *)src0;
		uint8 *dst = (uint8 *)dst0;

		// convert first pixel
		int cb, cr;
		int rc0, gc0, bc0, rc1, gc1, bc1;
		const uint8 *y;

		cb = src[1];
		cr = src[3];
		rc1 = colorconv.r_cr_tab[cr];
		gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
		bc1 = colorconv.b_cb_tab[cb];

		y = &colorconv.cliptab[277 + colorconv.y_tab[src[0]]];
		dst[0] = y[bc1];
		dst[1] = y[gc1];
		dst[2] = y[rc1];
		dst += 4;

		// convert pairs of pixels
		int w2 = w;

		if ((w2 -= 2) > 0) {
			do {
				rc0 = rc1;
				gc0 = gc1;
				bc0 = bc1;

				cb = src[5];
				cr = src[7];
				rc1 = colorconv.r_cr_tab[cr];
				gc1 = colorconv.g_cr_tab[cr] + colorconv.g_cb_tab[cb];
				bc1 = colorconv.b_cb_tab[cb];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[2]]];
				dst[0] = y[(bc0+bc1+1)>>1];
				dst[1] = y[(gc0+gc1+1)>>1];
				dst[2] = y[(rc0+rc1+1)>>1];

				y = &colorconv.cliptab[277 + colorconv.y_tab[src[4]]];
				dst[4] = y[bc1];
				dst[5] = y[gc1];
				dst[6] = y[rc1];

				dst += 8;
				src += 4;
			} while((w2 -= 2) > 0);
		}

		// handle oddballs
		if (!(w2 & 1)) {
			y = &colorconv.cliptab[277 + colorconv.y_tab[src[2]]];
			dst[0] = y[bc1];
			dst[1] = y[gc1];
			dst[2] = y[rc1];
		}

		vdptrstep(src0, srcpitch);
		vdptrstep(dst0, dstpitch);
	} while(--h);
}

DECLARE_YUV(Y8, XRGB1555) {
	uint16 *dst = (uint16 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= 2*w;
	srcpitch -= w;

	do {
		vdpixsize w2 = w;

		do {
			*dst++ = colorconv.cliptab15[colorconv.y_tab[*src++] + 277];
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(Y8, RGB565) {
	uint16 *dst = (uint16 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= 2*w;
	srcpitch -= w;

	do {
		vdpixsize w2 = w;

		do {
			*dst++ = colorconv.cliptab16[colorconv.y_tab[*src++] + 277];
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(Y8, RGB888) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= 3*w;
	srcpitch -= w;

	do {
		vdpixsize w2 = w;

		do {
			dst[0] = dst[1] = dst[2] = colorconv.cliptab[colorconv.y_tab[*src++] + 277];
			dst += 3;
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

DECLARE_YUV(Y8, XRGB8888) {
	uint32 *dst = (uint32 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	dstpitch -= 4*w;
	srcpitch -= w;

	do {
		vdpixsize w2 = w;

		do {
			*dst++ = 0x010101 * colorconv.cliptab[colorconv.y_tab[*src++] + 277];
		} while(--w2);

		vdptrstep(src, srcpitch);
		vdptrstep(dst, dstpitch);
	} while(--h);
}

#define DECLARE_YUV_PLANAR(x, y) void VDPixmapBlt_##x##_to_##y##_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h)


namespace {
	typedef void (*tpYUVPlanarFinalDecoder)(void *, const uint8 *, const uint8 *, const uint8 *, uint32);
	typedef void (*tpYUVPlanarHorizDecoder)(uint8 *dst, const uint8 *src, sint32 w);
	typedef void (*tpYUVPlanarVertDecoder)(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase);
}

#ifdef _M_IX86
	extern "C" void __cdecl vdasm_pixblt_YUV444Planar_to_XRGB1555_scan_MMX(void *dst, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 count);
	extern "C" void __cdecl vdasm_pixblt_YUV444Planar_to_RGB565_scan_MMX(void *dst, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 count);
	extern "C" void __cdecl vdasm_pixblt_YUV444Planar_to_XRGB8888_scan_MMX(void *dst, const uint8 *y, const uint8 *cb, const uint8 *cr, uint32 count);
#endif


void VDPixmapBlt_YUVPlanar_decode_reference(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h) {
	const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(src.format);
	int hbits = srcinfo.auxwbits;
	int vbits = srcinfo.auxhbits;

	if (src.format == nsVDPixmap::kPixFormat_YUV422_UYVY || src.format == nsVDPixmap::kPixFormat_YUV422_YUYV)
		hbits = 1;

	bool h_coaligned = true;
	bool v_coaligned = false;

	if (src.format == nsVDPixmap::kPixFormat_YUV422_Planar_Centered ||
		src.format == nsVDPixmap::kPixFormat_YUV420_Planar_Centered) {
		h_coaligned = false;
	}

	tpYUVPlanarVertDecoder vfunc = NULL;
	tpYUVPlanarHorizDecoder hfunc = NULL;
	uint32 horiz_buffer_size = 0;
	uint32 vert_buffer_size = 0;
	uint32 horiz_count = 0;
	sint32 yaccum = 8;
	sint32 yinc = 8;
	uint32 yleft = h;

	switch(vbits*2+v_coaligned) {
	case 0:		// 4:4:4, 4:2:2
	case 1:
		break;
	case 2:		// 4:2:0 (centered) 
		vfunc = vert_expand2x_centered;
		vert_buffer_size = w>>1;
		yaccum = 6;
		yinc = 4;
		yleft >>= 1;
		break;
	case 4:		// 4:1:0 (centered)
		vfunc = vert_expand4x_centered;
		vert_buffer_size = w>>2;
		yaccum = 5;
		yinc = 2;
		yleft >>= 2;
		break;
	default:
		VDNEVERHERE;
		return;
	}

	--yleft;

	tpYUVPlanarFinalDecoder dfunc = NULL;

#ifdef _M_IX86
	uint32 cpuflags = CPUGetEnabledExtensions();

	if (cpuflags & CPUF_SUPPORTS_MMX) {
		if (cpuflags & CPUF_SUPPORTS_INTEGER_SSE) {
			if (vfunc == vert_expand2x_centered)
				vfunc = vert_expand2x_centered_ISSE;
		}

		switch(dst.format) {
		case nsVDPixmap::kPixFormat_XRGB1555:	dfunc = vdasm_pixblt_YUV444Planar_to_XRGB1555_scan_MMX;	break;
		case nsVDPixmap::kPixFormat_RGB565:		dfunc = vdasm_pixblt_YUV444Planar_to_RGB565_scan_MMX;	break;
		case nsVDPixmap::kPixFormat_XRGB8888:	dfunc = vdasm_pixblt_YUV444Planar_to_XRGB8888_scan_MMX;	break;
		}
	}
#endif

	bool halfchroma = false;

	if (!dfunc) {
		switch(dst.format) {
		case nsVDPixmap::kPixFormat_XRGB1555:		dfunc = VDYCbCrToXRGB1555Span;	break;
		case nsVDPixmap::kPixFormat_RGB565:			dfunc = VDYCbCrToRGB565Span;	break;
		case nsVDPixmap::kPixFormat_RGB888:			dfunc = VDYCbCrToRGB888Span;	break;
		case nsVDPixmap::kPixFormat_XRGB8888:		dfunc = VDYCbCrToXRGB8888Span;	break;
		case nsVDPixmap::kPixFormat_YUV422_UYVY:	dfunc = VDYCbCrToUYVYSpan;		halfchroma = true;	break;
		case nsVDPixmap::kPixFormat_YUV422_YUYV:	dfunc = VDYCbCrToYUYVSpan;		halfchroma = true;	break;
		default:
			VDNEVERHERE;
			return;
		}
	}

	switch(hbits*2+h_coaligned) {
	case 0:		// 4:4:4
	case 1:
		if (halfchroma) {
			hfunc = horiz_compress2x_coaligned;
			horiz_buffer_size = (w + 1) >> 1;
			horiz_count = w;
		}
		break;
	case 2:		// 4:2:0 MPEG-1 (centered)
		if (halfchroma) {
			hfunc = horiz_realign_to_coaligned;
			horiz_buffer_size = (w + 1) >> 1;
			horiz_count = (w + 1) >> 1;
		} else {
			hfunc = horiz_expand2x_centered;
			horiz_buffer_size = w;
			horiz_count = w;
		}
		break;
	case 3:		// 4:2:0/4:2:2 MPEG-2 (coaligned)
		if (!halfchroma) {
			hfunc = horiz_expand2x_coaligned;
			horiz_buffer_size = w;
			horiz_count = w;
		}
		break;
	case 5:		// 4:1:1 (coaligned)
		if (halfchroma) {
			hfunc = horiz_expand2x_coaligned;
			horiz_buffer_size = (w + 1) >> 1;
			horiz_count = (w + 1) >> 1;
		} else {
			hfunc = horiz_expand4x_coaligned;
			horiz_buffer_size = w;
			horiz_count = w;
		}
		break;

	default:
		VDNEVERHERE;
		return;
	}

#ifdef _M_IX86
	if (cpuflags & CPUF_SUPPORTS_INTEGER_SSE) {
		if (hfunc == horiz_expand2x_coaligned)
			hfunc = horiz_expand2x_coaligned_ISSE;
	}
#endif

	uint32 chroma_srcwidth = -(-w >> srcinfo.auxwbits);
	horiz_buffer_size = (horiz_buffer_size + 15) & ~15;
	vert_buffer_size = (vert_buffer_size + 15) & ~15;

	// allocate buffers

	vdblock<uint8> tempbuf((horiz_buffer_size + vert_buffer_size)*2 + 1);

	uint8 *const crbufh = tempbuf.data();
	uint8 *const crbufv = crbufh + horiz_buffer_size;
	uint8 *const cbbufh = crbufv + vert_buffer_size;
	uint8 *const cbbufv = cbbufh + horiz_buffer_size;

	const uint8 *cb0 = (const uint8*)src.data2;
	const uint8 *cr0 = (const uint8*)src.data3;
	const uint8 *cb1  = cb0;
	const uint8 *cr1  = cr0;
	const uint8 *y = (const uint8 *)src.data;
	const ptrdiff_t ypitch = src.pitch;
	const ptrdiff_t cbpitch = src.pitch2;
	const ptrdiff_t crpitch = src.pitch3;

	void *out = dst.data;
	ptrdiff_t outpitch = dst.pitch;

	for(;;) {
		if (yaccum >= 8) {
			yaccum &= 7;

			cb0 = cb1;
			cr0 = cr1;

			if (yleft > 0) {
				--yleft;
				vdptrstep(cb1, cbpitch);
				vdptrstep(cr1, crpitch);
			}
		}

		const uint8 *cr = cr0;
		const uint8 *cb = cb0;

		// vertical interpolation: cr
		if(yaccum & 7) {
			const uint8 *const srcs[2]={cr0, cr1};
			vfunc(crbufv, srcs, chroma_srcwidth, (yaccum & 7) << 5);
			cr = crbufv;
		}

		// horizontal interpolation: cr
		if (hfunc) {
			hfunc(crbufh, cr, horiz_count);
			cr = crbufh;
		}

		// vertical interpolation: cb
		if(yaccum & 7) {
			const uint8 *const srcs[2]={cb0, cb1};
			vfunc(cbbufv, srcs, chroma_srcwidth, (yaccum & 7) << 5);
			cb = cbbufv;
		}

		// horizontal interpolation: cb
		if (hfunc) {
			hfunc(cbbufh, cb, horiz_count);
			cb = cbbufh;
		}

		dfunc(out, y, cb, cr, w);
		vdptrstep(out, outpitch);
		vdptrstep(y, ypitch);

		if (!--h)
			break;

		yaccum += yinc;
	}

#ifdef _M_IX86
	if (cpuflags & CPUF_SUPPORTS_MMX) {
		_mm_empty();
	}
#endif
}

namespace {
	typedef void (*tpUVBltHorizDecoder)(uint8 *dst, const uint8 *src, sint32 w);
	typedef void (*tpUVBltVertDecoder)(uint8 *dst, const uint8 *const *srcs, sint32 w, uint8 phase);

	void uvplaneblt(uint8 *dst, ptrdiff_t dstpitch, int dstformat, const uint8 *src, ptrdiff_t srcpitch, int srcformat, vdpixsize w, vdpixsize h) {
		const VDPixmapFormatInfo& srcinfo = VDPixmapGetInfo(srcformat);
		const VDPixmapFormatInfo& dstinfo = VDPixmapGetInfo(dstformat);

		int xshift = srcinfo.auxwbits - dstinfo.auxwbits;
		int yshift = srcinfo.auxhbits - dstinfo.auxhbits;

		tpUVBltHorizDecoder		hfunc = NULL;
		tpUVBltVertDecoder		vfunc = NULL;

		switch(xshift) {
		case +2:
			hfunc = horiz_expand4x_coaligned;
			break;
		case +1:
			hfunc = horiz_expand2x_coaligned;
			break;
		case  0:
			break;
		case -1:
			hfunc = horiz_compress2x_coaligned;
			break;
		case -2:
			hfunc = horiz_compress4x_coaligned;
			break;
		default:
			VDNEVERHERE;
			return;
		}

#ifdef _M_IX86
		uint32 cpuflags = CPUGetEnabledExtensions();

		if (cpuflags & CPUF_SUPPORTS_INTEGER_SSE) {
			if (hfunc == horiz_expand2x_coaligned)
				hfunc = horiz_expand2x_coaligned_ISSE;
		}
#endif

		int winsize, winposnext, winstep;

		switch(yshift) {
		case +2:
			vfunc = vert_expand4x_centered;
			winsize = 2;
			winposnext = 0xa0;
			winstep = 0x40;
			break;
		case +1:
			vfunc = vert_expand2x_centered;
			winsize = 2;
			winposnext = 0xc0;
			winstep = 0x80;
			break;
		case  0:
			winsize = 1;
			winposnext = 0;
			winstep = 0x100;
			break;
		case -1:
			vfunc = vert_compress2x_centered;
			winsize = 4;
			winposnext = 0x200;
			winstep = 0x200;
			break;
		case -2:
			vfunc = vert_compress4x_centered;
			winsize = 8;
			winposnext = 0x500;
			winstep = 0x400;
			break;
		default:
			VDNEVERHERE;
			return;
		}

#ifdef _M_IX86
		if (cpuflags & CPUF_SUPPORTS_INTEGER_SSE) {
			if (vfunc == vert_expand2x_centered)
				vfunc = vert_expand2x_centered_ISSE;
		}
#endif

		int dsth = -(-h >> dstinfo.auxhbits);
		int srch = -(-h >> srcinfo.auxhbits);
		int dstw = -(-w >> dstinfo.auxwbits);
		int w2 = -(-w >> std::min<int>(dstinfo.auxwbits, srcinfo.auxwbits));

		int winpos = (winposnext>>8) - winsize;

		const uint8 *window[16];

		vdblock<uint8> tmpbuf;
		ptrdiff_t tmppitch = (w+15) & ~15;

		if (vfunc && hfunc)
			tmpbuf.resize(tmppitch * winsize);

		do {
			int desiredpos = winposnext >> 8;

			while(winpos < desiredpos) {
				const uint8 *srcrow = vdptroffset(src, srcpitch * std::max<int>(0, std::min<int>(srch-1, ++winpos)));
				int winoffset = (winpos-1) & (winsize-1);

				if (hfunc) {
					uint8 *dstrow = vfunc ? tmpbuf.data() + tmppitch * winoffset : dst;
					hfunc(dstrow, srcrow, w2);
					srcrow = dstrow;
				}

				window[winoffset] = window[winoffset + winsize] = srcrow;
			}

			if (vfunc)
				vfunc(dst, window + (winpos & (winsize-1)), dstw, winposnext & 255);
			else if (!hfunc)
				memcpy(dst, window[winpos & (winsize-1)], dstw);

			winposnext += winstep;
			vdptrstep(dst, dstpitch);
		} while(--dsth);

#ifdef _M_IX86
		if (cpuflags & CPUF_SUPPORTS_MMX) {
			_mm_empty();
		}
#endif
	}
}

void VDPixmapBlt_YUVPlanar_convert_reference(const VDPixmap& dstpm, const VDPixmap& srcpm, vdpixsize w, vdpixsize h) {
	VDMemcpyRect(dstpm.data, dstpm.pitch, srcpm.data, srcpm.pitch, dstpm.w, dstpm.h);

	if (srcpm.format != nsVDPixmap::kPixFormat_Y8) {
		if (dstpm.format != nsVDPixmap::kPixFormat_Y8) {
			// YCbCr -> YCbCr
			uvplaneblt((uint8 *)dstpm.data2, dstpm.pitch2, dstpm.format, (uint8 *)srcpm.data2, srcpm.pitch2, srcpm.format, w, h);
			uvplaneblt((uint8 *)dstpm.data3, dstpm.pitch3, dstpm.format, (uint8 *)srcpm.data3, srcpm.pitch3, srcpm.format, w, h);
		}
	} else {
		if (dstpm.format != nsVDPixmap::kPixFormat_Y8) {
			const VDPixmapFormatInfo& info = VDPixmapGetInfo(dstpm.format);
			VDMemset8Rect(dstpm.data2, dstpm.pitch2, 0x80, -(-w >> info.auxwbits), -(-h >> info.auxhbits));
			VDMemset8Rect(dstpm.data3, dstpm.pitch3, 0x80, -(-w >> info.auxwbits), -(-h >> info.auxhbits));
		}
	}
}

extern "C" void vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_MMX(void *dst, const void *y, const void *cb, const void *cr, unsigned count);
extern "C" void vdasm_pixblt_YUV411Planar_to_RGB565_scan_MMX(void *dst, const void *y, const void *cb, const void *cr, unsigned count);
extern "C" void vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_MMX(void *dst, const void *y, const void *cb, const void *cr, unsigned count);
extern "C" void vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_ISSE(void *dst, const void *y, const void *cb, const void *cr, unsigned count);
extern "C" void vdasm_pixblt_YUV411Planar_to_RGB565_scan_ISSE(void *dst, const void *y, const void *cb, const void *cr, unsigned count);
extern "C" void vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_ISSE(void *dst, const void *y, const void *cb, const void *cr, unsigned count);

DECLARE_YUV_PLANAR(YUV411, XRGB1555) {
	uint16			*out	= (uint16 *)dst.data;
	const ptrdiff_t	opitch	= dst.pitch;
	const uint8		*yrow	= (const uint8 *)src.data;
	const uint8		*cbrow	= (const uint8 *)src.data2;
	const uint8		*crrow	= (const uint8 *)src.data3;
	const ptrdiff_t	ypitch	= src.pitch;
	const ptrdiff_t	cbpitch	= src.pitch2;
	const ptrdiff_t	crpitch	= src.pitch3;

	vdpixsize wpairs = (w-1)>>2;
	vdpixsize wleft = w - (wpairs<<2);

	do {
		uint16 *p = out;
		const uint8 *y = yrow;
		const uint8 *cb = cbrow;
		const uint8 *cr = crrow;
		vdpixsize wt;

		if (wpairs > 0) {
#if !defined(VD_CPU_X86)
			wt = wpairs;

			do {
				const unsigned cb0 = cb[0];
				const unsigned cb1 = cb[1];
				const unsigned cr0 = cr[0];
				const unsigned cr1 = cr[1];

				p[0] = ycbcr_to_1555(y[0], cb0, cr0);
				p[1] = ycbcr_to_1555(y[1], (3*cb0+cb1+2)>>2, (3*cr0+cr1+2)>>2);
				p[2] = ycbcr_to_1555(y[2], (cb0+cb1+1)>>1, (cr0+cr1+1)>>1);
				p[3] = ycbcr_to_1555(y[3], (cb0+3*cb1+2)>>2, (cr0+3*cr1+2)>>2);

				y += 4;
				p += 4;
				++cb;
				++cr;
			} while(--wt);
#else
			vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_ISSE(p, y, cb, cr, wpairs);
			y += 4*wpairs;
			cr += wpairs;
			cb += wpairs;
			p += 4*wpairs;
#endif
		}

		if (wleft > 0) {
			wt = wleft;

			const uint8 cr0 = *cr;
			const uint8 cb0 = *cb;

			do {
				*p++ = ycbcr_to_1555(*y++, cb0, cr0);
			} while(--wt);
		}

		vdptrstep(out, opitch);
		vdptrstep(yrow, ypitch);
		vdptrstep(cbrow, cbpitch);
		vdptrstep(crrow, crpitch);
	} while(--h);

#ifdef VD_CPU_X86
	_mm_empty();
#endif
}

DECLARE_YUV_PLANAR(YUV411, RGB565) {
	uint16			*out	= (uint16 *)dst.data;
	const ptrdiff_t	opitch	= dst.pitch;
	const uint8		*yrow	= (const uint8 *)src.data;
	const uint8		*cbrow	= (const uint8 *)src.data2;
	const uint8		*crrow	= (const uint8 *)src.data3;
	const ptrdiff_t	ypitch	= src.pitch;
	const ptrdiff_t	cbpitch	= src.pitch2;
	const ptrdiff_t	crpitch	= src.pitch3;

	vdpixsize wpairs = (w-1)>>2;
	vdpixsize wleft = w - (wpairs<<2);

	do {
		uint16 *p = out;
		const uint8 *y = yrow;
		const uint8 *cb = cbrow;
		const uint8 *cr = crrow;
		vdpixsize wt;

		if (wpairs > 0) {
#if !defined(VD_CPU_X86)
			wt = wpairs;

			do {
				const unsigned cb0 = cb[0];
				const unsigned cb1 = cb[1];
				const unsigned cr0 = cr[0];
				const unsigned cr1 = cr[1];

				p[0] = ycbcr_to_565(y[0], cb0, cr0);
				p[1] = ycbcr_to_565(y[1], (3*cb0+cb1+2)>>2, (3*cr0+cr1+2)>>2);
				p[2] = ycbcr_to_565(y[2], (cb0+cb1+1)>>1, (cr0+cr1+1)>>1);
				p[3] = ycbcr_to_565(y[3], (cb0+3*cb1+2)>>2, (cr0+3*cr1+2)>>2);

				y += 4;
				p += 4;
				++cb;
				++cr;
			} while(--wt);
#else
			vdasm_pixblt_YUV411Planar_to_RGB565_scan_ISSE(p, y, cb, cr, wpairs);
#endif
		}

		if (wleft > 0) {
			wt = wleft;

			const uint8 cr0 = *cr;
			const uint8 cb0 = *cb;

			do {
				*p++ = ycbcr_to_565(*y++, cb0, cr0);
			} while(--wt);
		}

		vdptrstep(out, opitch);
		vdptrstep(yrow, ypitch);
		vdptrstep(cbrow, cbpitch);
		vdptrstep(crrow, crpitch);
	} while(--h);

#ifdef VD_CPU_X86
	_mm_empty();
#endif
}

DECLARE_YUV_PLANAR(YUV411, RGB888) {
	uint8			*out	= (uint8 *)dst.data;
	const ptrdiff_t	opitch	= dst.pitch;
	const uint8		*yrow	= (const uint8 *)src.data;
	const uint8		*cbrow	= (const uint8 *)src.data2;
	const uint8		*crrow	= (const uint8 *)src.data3;
	const ptrdiff_t	ypitch	= src.pitch;
	const ptrdiff_t	cbpitch	= src.pitch2;
	const ptrdiff_t	crpitch	= src.pitch3;

	vdpixsize wpairs = (w-1)>>2;
	vdpixsize wleft = w - (wpairs<<2);

	do {
		uint8 *p = out;
		const uint8 *y = yrow;
		const uint8 *cb = cbrow;
		const uint8 *cr = crrow;
		vdpixsize wt;

		if (wpairs > 0) {
			wt = wpairs;

			do {
				const unsigned cb0 = cb[0];
				const unsigned cb1 = cb[1];
				const unsigned cr0 = cr[0];
				const unsigned cr1 = cr[1];

				ycbcr_to_888(p+0, y[0], cb0, cr0);
				ycbcr_to_888(p+3, y[1], (3*cb0+cb1+2)>>2, (3*cr0+cr1+2)>>2);
				ycbcr_to_888(p+6, y[2], (cb0+cb1+1)>>1, (cr0+cr1+1)>>1);
				ycbcr_to_888(p+9, y[3], (cb0+3*cb1+2)>>2, (cr0+3*cr1+2)>>2);

				y += 4;
				p += 12;
				++cb;
				++cr;
			} while(--wt);
		}

		if (wleft > 0) {
			wt = wleft;

			const uint8 cr0 = *cr;
			const uint8 cb0 = *cb;

			do {
				ycbcr_to_888(p, *y++, cb0, cr0);
				p += 4;
			} while(--wt);
		}

		vdptrstep(out, opitch);
		vdptrstep(yrow, ypitch);
		vdptrstep(cbrow, cbpitch);
		vdptrstep(crrow, crpitch);
	} while(--h);
}

DECLARE_YUV_PLANAR(YUV411, XRGB8888) {
	uint32			*out	= (uint32 *)dst.data;
	const ptrdiff_t	opitch	= dst.pitch;
	const uint8		*yrow	= (const uint8 *)src.data;
	const uint8		*cbrow	= (const uint8 *)src.data2;
	const uint8		*crrow	= (const uint8 *)src.data3;
	const ptrdiff_t	ypitch	= src.pitch;
	const ptrdiff_t	cbpitch	= src.pitch2;
	const ptrdiff_t	crpitch	= src.pitch3;

	vdpixsize wpairs = (w-1)>>2;
	vdpixsize wleft = w - (wpairs<<2);

	do {
		uint32 *p = out;
		const uint8 *y = yrow;
		const uint8 *cb = cbrow;
		const uint8 *cr = crrow;
		vdpixsize wt;

		if (wpairs > 0) {
#if !defined(VD_CPU_X86)
			wt = wpairs;

			do {
				const unsigned cb0 = cb[0];
				const unsigned cb1 = cb[1];
				const unsigned cr0 = cr[0];
				const unsigned cr1 = cr[1];

				p[0] = ycbcr_to_8888(y[0], cb0, cr0);
				p[1] = ycbcr_to_8888(y[1], (3*cb0+cb1+2)>>2, (3*cr0+cr1+2)>>2);
				p[2] = ycbcr_to_8888(y[2], (cb0+cb1+1)>>1, (cr0+cr1+1)>>1);
				p[3] = ycbcr_to_8888(y[3], (cb0+3*cb1+2)>>2, (cr0+3*cr1+2)>>2);

				y += 4;
				p += 4;
				++cb;
				++cr;
			} while(--wt);
#else
			vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_MMX(p, y, cb, cr, wpairs);
			y += 4*wpairs;
			cr += wpairs;
			cb += wpairs;
			p += 4*wpairs;
#endif
		}

		if (wleft > 0) {
			wt = wleft;

			const uint8 cr0 = *cr;
			const uint8 cb0 = *cb;

			do {
				*p++ = ycbcr_to_8888(*y++, cb0, cr0);
			} while(--wt);
		}

		vdptrstep(out, opitch);
		vdptrstep(yrow, ypitch);
		vdptrstep(cbrow, cbpitch);
		vdptrstep(crrow, crpitch);
	} while(--h);

#ifdef VD_CPU_X86
	_mm_empty();
#endif
}
