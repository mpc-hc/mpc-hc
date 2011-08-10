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
#include <vd2/system/memory.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>

namespace {
	struct VDPixmapReferenceStretchBltParameters {
		void *dst;
		ptrdiff_t	dstpitch;
		const void *src;
		ptrdiff_t	srcpitch;
		ptrdiff_t	srcinc;
		sint32		dx;
		sint32		dy;
		uint32		u;
		uint32		uinc;
		uint32		dudx;
		uint32		v;
		uint32		dvdy;
		sint32		xprecopy;
		sint32		xpostcopy;
		ptrdiff_t	xprepos;
		ptrdiff_t	xpostpos;

		void advance() {
			dst = (char *)dst + dstpitch;
			src = (char *)src + srcinc;

			uint32 vt = v + dvdy;

			if (vt < v)
				src = (char *)src + srcpitch;

			v = vt;
		}
	};
}

void VDPixmapStretchBlt_Any8_nearest_reference(VDPixmapReferenceStretchBltParameters params) {
	do {
		uint8 *dstp = (uint8 *)params.dst;
		const uint8 *srcp = (const uint8 *)params.src;
		uint32 u = params.u;

		if (params.xprecopy) {
			VDMemset8(dstp, *(const uint8 *)((const char *)params.src + params.xprepos), params.xprecopy);
			dstp += params.xprecopy;
		}

		sint32 wt = params.dx;

		if (wt > 0)
			do {
				*dstp++ = *srcp;

				uint32 ut = u + params.dudx;
				srcp += ut<u;
				srcp += params.uinc;
				u = ut;
			} while(--wt);

		if (params.xpostcopy)
			VDMemset8(dstp, *(const uint8 *)((const char *)params.src + params.xpostpos), params.xpostcopy);

		params.advance();
	} while(--params.dy);
}

void VDPixmapStretchBlt_Any16_nearest_reference(VDPixmapReferenceStretchBltParameters params) {
	do {
		uint16 *dstp = (uint16 *)params.dst;
		const uint16 *srcp = (const uint16 *)params.src;
		uint32 u = params.u;

		if (params.xprecopy) {
			VDMemset16(dstp, *(const uint16 *)((const char *)params.src + params.xprepos), params.xprecopy);
			dstp += params.xprecopy;
		}

		sint32 wt = params.dx;

		if (wt > 0)
			do {
				*dstp++ = *srcp;

				uint32 ut = u + params.dudx;
				srcp += ut<u;
				srcp += params.uinc;
				u = ut;
			} while(--wt);

		if (params.xpostcopy)
			VDMemset16(dstp, *(const uint16 *)((const char *)params.src + params.xpostpos), params.xpostcopy);

		params.advance();
	} while(--params.dy);
}

void VDPixmapStretchBlt_Any24_nearest_reference(VDPixmapReferenceStretchBltParameters params) {
	do {
		uint8 *dstp = (uint8 *)params.dst;
		const uint8 *srcp = (const uint8 *)params.src;
		uint32 u = params.u;

		if (params.xprecopy) {
			const uint8 *repsrc = (const uint8 *)params.src + params.xprepos;
			const uint8 p0 = repsrc[0];
			const uint8 p1 = repsrc[1];
			const uint8 p2 = repsrc[2];

			for(sint32 i=0; i<params.xprecopy; ++i) {
				dstp[0] = p0;
				dstp[1] = p1;
				dstp[2] = p2;
				dstp += 3;
			}
		}

		sint32 wt = params.dx;

		if (wt > 0)
			do {
				dstp[0] = srcp[0];
				dstp[1] = srcp[1];
				dstp[2] = srcp[2];
				dstp += 3;

				uint32 ut = u + params.dudx;
				srcp += (ut<u)*3;
				srcp += params.uinc*3;
				u = ut;
			} while(--wt);

		if (params.xpostcopy) {
			const uint8 *repsrc = (const uint8 *)params.src + params.xpostpos;
			const uint8 p0 = repsrc[0];
			const uint8 p1 = repsrc[1];
			const uint8 p2 = repsrc[2];

			for(sint32 i=0; i<params.xpostcopy; ++i) {
				dstp[0] = p0;
				dstp[1] = p1;
				dstp[2] = p2;
				dstp += 3;
			}
		}

		params.advance();
	} while(--params.dy);
}

void VDPixmapStretchBlt_Any32_nearest_reference(VDPixmapReferenceStretchBltParameters params) {
	do {
		uint32 *dstp = (uint32 *)params.dst;
		const uint32 *srcp = (const uint32 *)params.src;
		uint32 u = params.u;

		if (params.xprecopy) {
			VDMemset32(dstp, *(const uint32 *)((const char *)params.src + params.xprepos), params.xprecopy);
			dstp += params.xprecopy;
		}

		sint32 wt = params.dx;
		if (wt > 0)
			do {
				*dstp++ = *srcp;

				uint32 ut = u + params.dudx;
				srcp += ut<u;
				srcp += params.uinc;
				u = ut;
			} while(--wt);

		if (params.xpostcopy)
			VDMemset32(dstp, *(const uint32 *)((const char *)params.src + params.xpostpos), params.xpostcopy);

		params.advance();
	} while(--params.dy);
}

///////////////////////////////////////////////////////////////////////////

namespace {
	void VDSetupNearestSamplingParameters(sint64& u64, sint64 dudx, sint32 dx, sint32 du, sint32& xprecopy, sint32& xprepos, sint32& xmain, sint32& xpostcopy, sint32& xpostpos) {
		sint64 ulo = u64;
		sint64 uhi = u64 + dudx * (dx - 1);
		sint64 tdudx = dudx;
		const sint64 ulimit = ((sint64)du << 32);

		xprepos = 0;
		xpostpos = du-1;

		if (!tdudx) {
			if (u64 < 0)
				xprecopy = dx;
			else if (u64 >= ulimit)
				xprecopy = dx;
			else
				xmain = dx;
		} else {
			if (tdudx < 0) {
				std::swap(ulo, uhi);
				tdudx = -tdudx;
			}

			if (ulo < 0) {
				if (uhi < 0)
					xprecopy = dx;
				else
					xprecopy = (sint32)((-ulo-1) / tdudx) + 1;

				VDASSERT(xprecopy <= 0 || (uint64)ulo >= (uint64)ulimit);
				VDASSERT(xprecopy <= 0 || (uint64)(ulo + tdudx * (xprecopy-1)) >= (uint64)ulimit);
			}

			if (uhi >= ulimit) {
				if (ulo >= ulimit)
					xpostcopy = dx;
				else
					xpostcopy = (sint32)((uhi - ulimit) / tdudx) + 1;

				VDASSERT(xpostcopy <= 0 || (uint64)uhi >= (uint64)ulimit);
				VDASSERT(xpostcopy <= 0 || (uint64)(uhi - tdudx * (xpostcopy - 1)) >= (uint64)ulimit);
			}

			if (dudx < 0) {
				std::swap(xprecopy, xpostcopy);
				std::swap(xprepos, xpostpos);
			}

			xmain = dx - (xprecopy + xpostcopy);
		}

		// sanity-check parameters

		VDASSERT(xprecopy>=0 && xprecopy <= dx);
		VDASSERT(xpostcopy>=0 && xpostcopy <= dx);
		VDASSERT(xmain>=0 && xmain <= dx);

		VDASSERT(xprecopy <= 0 || (uint64)u64 >= (uint64)ulimit);
		VDASSERT(xprecopy <= 0 || (uint64)(u64 + dudx * (xprecopy-1)) >= (uint64)ulimit);
		VDASSERT(xmain <= 0 || (uint64)(u64 + dudx * xprecopy) < (uint64)ulimit);
		VDASSERT(xmain <= 0 || (uint64)(u64 + dudx * (xprecopy+xmain-1)) < (uint64)ulimit);
		VDASSERT(xpostcopy <= 0 || (uint64)(u64 + dudx * (xprecopy + xmain)) >= (uint64)ulimit);
		VDASSERT(xpostcopy <= 0 || (uint64)(u64 + dudx * (xprecopy + xmain + xpostcopy - 1)) >= (uint64)ulimit);

		u64 += dudx * xprecopy;
	}
}

bool VDPixmapStretchBltNearest_reference(const VDPixmap& dst, sint32 x1, sint32 y1, sint32 x2, sint32 y2, const VDPixmap& src, sint32 u1, sint32 v1, sint32 u2, sint32 v2) {
	// we don't support format conversion
	if (dst.format != src.format)
		return false;

	void (*pBlitter)(VDPixmapReferenceStretchBltParameters);
	int bpp;

	switch(src.format) {
	case nsVDPixmap::kPixFormat_Pal8:
		pBlitter = VDPixmapStretchBlt_Any8_nearest_reference;
		bpp = 1;
		break;
	case nsVDPixmap::kPixFormat_XRGB1555:
	case nsVDPixmap::kPixFormat_RGB565:
		pBlitter = VDPixmapStretchBlt_Any16_nearest_reference;
		bpp = 2;
		break;
	case nsVDPixmap::kPixFormat_RGB888:
		pBlitter = VDPixmapStretchBlt_Any24_nearest_reference;
		bpp = 3;
		break;
	case nsVDPixmap::kPixFormat_XRGB8888:
		pBlitter = VDPixmapStretchBlt_Any32_nearest_reference;
		bpp = 4;
		break;
	default:
		return false;
	}

	// preemptive clip to prevent gradient calculations from crashing
	if (x2 == x1 || y2 == y1)
		return true;

	// translate destination flips into source flips
	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(u1, u2);
	}

	if (y1 > y2) {
		std::swap(y1, y2);
		std::swap(v1, v2);
	}

	// compute gradients
	sint32 dx	= x2 - x1;
	sint32 dy	= y2 - y1;
	sint32 du	= u2 - u1;
	sint32 dv	= v2 - v1;
	sint64 dudx = ((sint64)du << 32) / dx;		// must truncate toward zero to prevent overflow
	sint64 dvdy = ((sint64)dv << 32) / dy;

	// prestep top-left point to pixel center and convert destination coordinates to integer
	sint64 u64 = (sint64)u1 << 16;
	sint64 v64 = (sint64)v1 << 16;
	sint32 prestepx = (0x8000 - x1) & 0xffff;
	sint32 prestepy = (0x8000 - y1) & 0xffff;

	u64 += (dudx * prestepx) >> 16;
	v64 += (dvdy * prestepy) >> 16;

	sint32 x1i = (x1 + 0x8000) >> 16;
	sint32 y1i = (y1 + 0x8000) >> 16;
	sint32 x2i = (x2 + 0x8000) >> 16;
	sint32 y2i = (y2 + 0x8000) >> 16;

	// destination clipping
	if (x1i < 0) {
		u64 -= dudx * x1i;
		x1i = 0;
	}

	if (y1i < 0) {
		v64 -= dvdy * y1i;
		y1i = 0;
	}

	if (x2i > dst.w)
		x2i = dst.w;

	if (y2i > dst.h)
		y2i = dst.h;

	if (x1i >= x2i || y1i >= y2i)
		return true;

	// Calculate horizontal clip parameters
	sint32 xprecopy = 0, xpostcopy = 0;
	int xprepos = 0;
	int xpostpos = src.w-1;
	int xmain = 0;

	VDSetupNearestSamplingParameters(u64, dudx, x2i-x1i, src.w, xprecopy, xprepos, xmain, xpostcopy, xpostpos);

	// Calculate vertical clip parameters
	sint32 yprecopy = 0, ypostcopy = 0;
	int yprepos = 0;
	int ypostpos = src.h-1;
	int ymain = 0;

	VDSetupNearestSamplingParameters(v64, dvdy, y2i-y1i, src.h, yprecopy, yprepos, ymain, ypostcopy, ypostpos);

	// set up parameter block
	VDPixmapReferenceStretchBltParameters params;

	char *srcbase = (char *)src.data + (sint32)(u64 >> 32) * bpp;

	params.dst			= (char *)dst.data + y1i * dst.pitch + x1i * bpp;
	params.dstpitch		= dst.pitch;
	params.src			= srcbase + (sint32)(v64 >> 32) * src.pitch;
	params.srcpitch		= src.pitch;
	params.srcinc		= (sint32)(dvdy >> 32) * src.pitch;
	params.dx			= xmain;
	params.dy			= ymain;
	params.u			= (uint32)u64;
	params.uinc			= (uint32)(dudx >> 32);
	params.dudx			= (uint32)dudx;
	params.v			= (uint32)v64;
	params.dvdy			= (uint32)dvdy;
	params.xprecopy		= xprecopy;
	params.xprepos		= (xprepos - (sint32)(u64 >> 32)) * bpp;
	params.xpostcopy	= xpostcopy;
	params.xpostpos		= (xpostpos - (sint32)(u64 >> 32)) * bpp;

	if (yprecopy > 0) {
		VDPixmapReferenceStretchBltParameters preparams(params);

		preparams.src		= srcbase + yprepos * src.pitch;
		preparams.srcinc	= 0;
		preparams.dy		= yprecopy;
		preparams.v			= 0;
		preparams.dvdy		= 0;

		pBlitter(preparams);

		params.dst		= (char *)params.dst + params.dstpitch * yprecopy;
	}

	if (ymain > 0)
		pBlitter(params);

	if (ypostcopy > 0) {
		VDPixmapReferenceStretchBltParameters postparams(params);

		postparams.dst		= (char *)params.dst + params.dstpitch * params.dy;
		postparams.src		= srcbase + ypostpos * src.pitch;
		postparams.srcpitch	= 0;
		postparams.srcinc	= 0;
		postparams.dy		= ypostcopy;
		postparams.v		= 0;
		postparams.dvdy		= 0;

		pBlitter(postparams);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace {
	uint32 lerp_XRGB1555(sint32 a, sint32 b, sint32 f) {
		sint32 a_rb	= a & 0x7c1f;
		sint32 a_g	= a & 0x03e0;
		sint32 b_rb	= b & 0x7c1f;
		sint32 b_g	= b & 0x03e0;

		const sint32 rb = (a_rb + (((b_rb - a_rb)*f + 0x4010) >> 5)) & 0x7c1f;
		const sint32 g  = (a_g  + (((b_g  - a_g )*f + 0x0200) >> 5)) & 0x03e0;

		return rb + g;
	}

	uint32 lerp_XRGB8888(sint32 a, sint32 b, sint32 f) {
		sint32 a_rb	= a & 0xff00ff;
		sint32 a_g	= a & 0x00ff00;
		sint32 b_rb	= b & 0xff00ff;
		sint32 b_g	= b & 0x00ff00;

		const uint32 rb = (a_rb + (((b_rb - a_rb)*f + 0x00800080) >> 8)) & 0xff00ff;
		const uint32 g  = (a_g  + (((b_g  - a_g )*f + 0x00008000) >> 8)) & 0x00ff00;

		return rb + g;
	}

	uint32 bilerp_RGB888(sint32 a, sint32 b, sint32 c, sint32 d, sint32 x, sint32 y) {
		sint32 a_rb	= a & 0xff00ff;
		sint32 a_g	= a & 0x00ff00;
		sint32 b_rb	= b & 0xff00ff;
		sint32 b_g	= b & 0x00ff00;
		sint32 c_rb	= c & 0xff00ff;
		sint32 c_g	= c & 0x00ff00;
		sint32 d_rb	= d & 0xff00ff;
		sint32 d_g	= d & 0x00ff00;

		const uint32 top_rb = (a_rb + (((b_rb - a_rb)*x + 0x00800080) >> 8)) & 0xff00ff;
		const uint32 top_g  = (a_g  + (((b_g  - a_g )*x + 0x00008000) >> 8)) & 0x00ff00;
		const uint32 bot_rb = (c_rb + (((d_rb - c_rb)*x + 0x00800080) >> 8)) & 0xff00ff;
		const uint32 bot_g  = (c_g  + (((d_g  - c_g )*x + 0x00008000) >> 8)) & 0x00ff00;

		const uint32 final_rb = (top_rb + (((bot_rb - top_rb)*y) >> 8)) & 0xff00ff;
		const uint32 final_g  = (top_g  + (((bot_g  - top_g )*y) >> 8)) & 0x00ff00;

		return final_rb + final_g;
	}

	uint32 bilerp_XRGB1555(sint32 a, sint32 b, sint32 c, sint32 d, sint32 x, sint32 y) {
		sint32 a_rb	= a & 0x7c1f;
		sint32 a_g	= a & 0x03e0;
		sint32 b_rb	= b & 0x7c1f;
		sint32 b_g	= b & 0x03e0;
		sint32 c_rb	= c & 0x7c1f;
		sint32 c_g	= c & 0x03e0;
		sint32 d_rb	= d & 0x7c1f;
		sint32 d_g	= d & 0x03e0;

		const sint32 top_rb = (a_rb + (((b_rb - a_rb)*x + 0x4010) >> 5)) & 0x7c1f;
		const sint32 top_g  = (a_g  + (((b_g  - a_g )*x + 0x0200) >> 5)) & 0x03e0;
		const sint32 bot_rb = (c_rb + (((d_rb - c_rb)*x + 0x4010) >> 5)) & 0x7c1f;
		const sint32 bot_g  = (c_g  + (((d_g  - c_g )*x + 0x0200) >> 5)) & 0x03e0;

		const sint32 final_rb = (top_rb + (((bot_rb - top_rb)*y + 0x4010) >> 5)) & 0x7c1f;
		const sint32 final_g  = (top_g  + (((bot_g  - top_g )*y + 0x0200) >> 5)) & 0x03e0;

		return final_rb + final_g;
	}

	uint32 bilerp_RGB565(sint32 a, sint32 b, sint32 c, sint32 d, sint32 x, sint32 y) {
		sint32 a_rb	= a & 0xf81f;
		sint32 a_g	= a & 0x07e0;
		sint32 b_rb	= b & 0xf81f;
		sint32 b_g	= b & 0x07e0;
		sint32 c_rb	= c & 0xf81f;
		sint32 c_g	= c & 0x07e0;
		sint32 d_rb	= d & 0xf81f;
		sint32 d_g	= d & 0x07e0;

		const sint32 top_rb = (a_rb + (((b_rb - a_rb)*x + 0x8010) >> 6)) & 0xf81f;
		const sint32 top_g  = (a_g  + (((b_g  - a_g )*x + 0x0400) >> 6)) & 0x07e0;
		const sint32 bot_rb = (c_rb + (((d_rb - c_rb)*x + 0x8010) >> 6)) & 0xf81f;
		const sint32 bot_g  = (c_g  + (((d_g  - c_g )*x + 0x0400) >> 6)) & 0x07e0;

		const sint32 final_rb = (top_rb + (((bot_rb - top_rb)*y + 0x8010) >> 6)) & 0xf81f;
		const sint32 final_g  = (top_g  + (((bot_g  - top_g )*y + 0x0400) >> 6)) & 0x07e0;

		return final_rb + final_g;
	}
}

///////////////////////////////////////////////////////////////////////////

namespace {
	struct VDPixmapReferenceStretchBltBilinearParameters {
		void		*dst;
		const void	*src;
		uint32		u;
		uint32		uinc;
		uint32		dudx;

		ptrdiff_t	xprepos;
		ptrdiff_t	xpostpos;
		sint32		xprecopy;
		sint32		xpostcopy;
		sint32		xmidsize;
	};

	void VDPixmapStretchBiH_XRGB1555_to_XRGB1555(const VDPixmapReferenceStretchBltBilinearParameters& params) {
		uint16 *dst = (uint16 *)params.dst;
		const uint16 *src = (const uint16 *)params.src;

		if (params.xprecopy)
			VDMemset16(dst - params.xprecopy, *(const uint16 *)((const char *)params.src + params.xprepos), params.xprecopy);

		if (params.xmidsize) {
			sint32 w = params.xmidsize;
			uint32 u = params.u;
			const uint32 dudx = params.dudx;
			const ptrdiff_t uinc = params.uinc;

			do {
				*dst++ = lerp_XRGB1555(src[0], src[1], u >> 27);

				const uint32 ut = u + dudx;
				src += uinc + (ut < u);
				u = ut;
			} while(--w);
		}

		if (params.xpostcopy)
			VDMemset16(dst, *(const uint16 *)((const char *)params.src + params.xpostpos), params.xpostcopy);
	}

	void VDPixmapStretchBiH_XRGB8888_to_XRGB8888(const VDPixmapReferenceStretchBltBilinearParameters& params) {
		uint32 *dst = (uint32 *)params.dst;
		const uint32 *src = (const uint32 *)params.src;

		if (params.xprecopy)
			VDMemset32(dst - params.xprecopy, *(const uint32 *)((const char *)params.src + params.xprepos), params.xprecopy);

		if (params.xmidsize) {
			sint32 w = params.xmidsize;
			uint32 u = params.u;
			const uint32 dudx = params.dudx;
			const ptrdiff_t uinc = params.uinc;

			do {
				*dst++ = lerp_XRGB8888(src[0], src[1], u >> 24);

				const uint32 ut = u + dudx;
				src += uinc + (ut < u);
				u = ut;
			} while(--w);
		}

		if (params.xpostcopy)
			VDMemset32(dst, *(const uint32 *)((const char *)params.src + params.xpostpos), params.xpostcopy);
	}

	void VDPixmapStretchBiV_XRGB1555_to_XRGB1555(void *dstv, const void *src1v, const void *src2v, sint32 w, uint32 f) {
		uint16 *dst = (uint16 *)dstv;
		const uint16 *src1 = (const uint16 *)src1v;
		const uint16 *src2 = (const uint16 *)src2v;

		f >>= 27;

		do {
			*dst++ = lerp_XRGB1555(*src1++, *src2++, f);
		} while(--w);
	}

	void VDPixmapStretchBiV_XRGB8888_to_XRGB8888(void *dstv, const void *src1v, const void *src2v, sint32 w, uint32 f) {
		uint32 *dst = (uint32 *)dstv;
		const uint32 *src1 = (const uint32 *)src1v;
		const uint32 *src2 = (const uint32 *)src2v;

		f >>= 24;

		do {
			*dst++ = lerp_XRGB8888(*src1++, *src2++, f);
		} while(--w);
	}
}

#ifdef _M_IX86
extern "C" void vdasm_stretchbltH_XRGB8888_to_XRGB8888_MMX(const VDPixmapReferenceStretchBltBilinearParameters&);

extern "C" void vdasm_stretchbltV_XRGB1555_to_XRGB1555_MMX(void *dstv, const void *src1v, const void *src2v, sint32 w, uint32 f);
extern "C" void vdasm_stretchbltV_XRGB8888_to_XRGB8888_MMX(void *dstv, const void *src1v, const void *src2v, sint32 w, uint32 f);
#endif

bool VDPixmapStretchBltBilinear_reference(const VDPixmap& dst, sint32 x1, sint32 y1, sint32 x2, sint32 y2, const VDPixmap& src, sint32 u1, sint32 v1, sint32 u2, sint32 v2) {
	// preemptive clip to prevent gradient calculations from crashing
	if (x2 == x1 || y2 == y1)
		return true;

	// we don't support source clipping
	if ((uint32)u1 > (uint32)(src.w << 16) || (uint32)v1 > (uint32)(src.h << 16))
		return false;

	if ((uint32)u2 > (uint32)(src.w << 16) || (uint32)v2 > (uint32)(src.h << 16))
		return false;

	// we don't support format changes (yet)
	if (dst.format != src.format)
		return false;

	// format determination
	void (*pHorizontalFilter)(const VDPixmapReferenceStretchBltBilinearParameters& params);
	void (*pVerticalFilter)(void *dstv, const void *src1v, const void *src2v, sint32 w, uint32 f);
	int bpp;

#pragma vdpragma_TODO("fixme this is b0rken")
	switch(src.format) {
	case nsVDPixmap::kPixFormat_XRGB1555:
		pHorizontalFilter = VDPixmapStretchBiH_XRGB1555_to_XRGB1555;
#ifdef _M_IX86
		if (CPUGetEnabledExtensions() & CPUF_SUPPORTS_MMX)
			pVerticalFilter = vdasm_stretchbltV_XRGB1555_to_XRGB1555_MMX;
		else
#endif
			pVerticalFilter = VDPixmapStretchBiV_XRGB1555_to_XRGB1555;
		bpp = 2;
		break;
	case nsVDPixmap::kPixFormat_XRGB8888:
#ifdef _M_IX86
		if (CPUGetEnabledExtensions() & CPUF_SUPPORTS_MMX) {
			pHorizontalFilter = vdasm_stretchbltH_XRGB8888_to_XRGB8888_MMX;
			pVerticalFilter = vdasm_stretchbltV_XRGB8888_to_XRGB8888_MMX;
		} else
#endif
		{
			pHorizontalFilter = VDPixmapStretchBiH_XRGB8888_to_XRGB8888;
			pVerticalFilter = VDPixmapStretchBiV_XRGB8888_to_XRGB8888;
		}
		bpp = 4;
		break;
	default:
		return false;
	}

	// translate destination flips into source flips
	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(u1, u2);
	}

	if (y1 > y2) {
		std::swap(y1, y2);
		std::swap(v1, v2);
	}

	// compute gradients
	sint32 dx	= x2 - x1;
	sint32 dy	= y2 - y1;
	sint32 du	= u2 - u1;
	sint32 dv	= v2 - v1;
	sint64 dudx = ((sint64)du << 32) / dx;		// must truncate toward zero to prevent overflow
	sint64 dvdy = ((sint64)dv << 32) / dy;

	// prestep top-left point to pixel center and convert destination coordinates to integer
	sint64 u64 = (sint64)u1 << 16;
	sint64 v64 = (sint64)v1 << 16;
	sint32 prestepx = (0x8000 - x1) & 0xffff;
	sint32 prestepy = (0x8000 - y1) & 0xffff;

	u64 += (dudx * prestepx) >> 16;
	v64 += (dvdy * prestepy) >> 16;

	sint32 x1i = (x1 + 0x8000) >> 16;
	sint32 y1i = (y1 + 0x8000) >> 16;
	sint32 x2i = (x2 + 0x8000) >> 16;
	sint32 y2i = (y2 + 0x8000) >> 16;

	// destination clipping
	if (x1i < 0) {
		u64 -= dudx * x1i;
		x1i = 0;
	}

	if (y1i < 0) {
		v64 -= dvdy * y1i;
		y1i = 0;
	}

	if (x2i > dst.w)
		x2i = dst.w;

	if (y2i > dst.h)
		y2i = dst.h;

	if (x1i >= x2i || y1i >= y2i)
		return true;

	u64 -= 0x80000000;
	v64 -= 0x80000000;

	int xprepos = 0;
	int xpostpos = src.w-1;

	sint64 ulo = u64;
	sint64 uhi = u64 + dudx * (x2i - x1i - 1);
	sint64 tdudx = dudx;

	if (ulo > uhi) {
		std::swap(ulo, uhi);
		tdudx = -tdudx;
	}

	int xprecopy = 0;
	int xpostcopy = 0;

	if (ulo < 0) {
		xprecopy = (int)((1 - ulo) / tdudx) + 1;
	}

	const sint64 ulimit = ((sint64)(src.w-1) << 32);

	if (uhi >= ulimit)
		xpostcopy = (int)((uhi - ulimit - 1) / tdudx) + 1;

	if (dudx < 0) {
		std::swap(xprecopy, xpostcopy);
		std::swap(xprepos, xpostpos);
	}

	u64 += dudx * xprecopy;
	const int xtotal	= x2i - x1i;
	int xmidcopy = (x2i - x1i) - (xprecopy + xpostcopy);
	const sint32 ui = (sint32)(u64 >> 32);

	// set up parameter block

	VDPixmapReferenceStretchBltBilinearParameters params;

	params.u			= (uint32)u64;
	params.uinc			= (sint32)(dudx >> 32);
	params.dudx			= (sint32)dudx;
	params.xprecopy		= xprecopy;
	params.xprepos		= (xprepos - ui) * bpp;
	params.xpostcopy	= xpostcopy;
	params.xpostpos		= (xpostpos - ui) * bpp;
	params.xmidsize		= xmidcopy;

	void *dstp			= (char *)dst.data + y1i * dst.pitch + x1i * bpp;
	const void *srcp	= (char *)src.data + ui * bpp;

	VDPixmapBuffer		window(xtotal, 2, src.format);

	void *pTempRow1 = window.data;
	void *pTempRow2 = (char *)window.data + window.pitch;
	int windowbottom = dvdy > 0 ? -0x7fffffff : 0x7fffffff;

	do {
		sint32 iv = (sint32)(v64 >> 32);
		sint32 iv_bottom = iv + 1;

		if (iv < 0)
			iv = iv_bottom = 0;

		if (iv >= src.h-1)
			iv = iv_bottom = src.h-1;

		if (dvdy < 0) {
			if (windowbottom > iv_bottom+1)
				windowbottom = iv_bottom+1;

			while(windowbottom > iv) {
				std::swap(pTempRow1, pTempRow2);

				--windowbottom;

				params.dst		= (char *)pTempRow1 + bpp * params.xprecopy;
				params.src		= vdptroffset(srcp, windowbottom * src.pitch);

				pHorizontalFilter(params);
			}
		} else {
			if (windowbottom < iv-1)
				windowbottom = iv-1;

			while(windowbottom < iv_bottom) {
				std::swap(pTempRow1, pTempRow2);

				++windowbottom;

				params.dst		= (char *)pTempRow2 + bpp * params.xprecopy;
				params.src		= vdptroffset(srcp, windowbottom * src.pitch);

				pHorizontalFilter(params);
			}
		}

		if (iv == iv_bottom)
			if (dvdy < 0)
				pVerticalFilter(dstp, pTempRow1, pTempRow1, xtotal, 0);
			else
				pVerticalFilter(dstp, pTempRow2, pTempRow2, xtotal, 0);
		else
			pVerticalFilter(dstp, pTempRow1, pTempRow2, xtotal, (uint32)v64);

		v64 += dvdy;
		dstp = (char *)dstp + dst.pitch;
	} while(++y1i < y2i);

	return true;
}
