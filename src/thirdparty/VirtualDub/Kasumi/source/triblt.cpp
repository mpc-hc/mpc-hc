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
#include <math.h>
#include <vector>
#include <vd2/system/math.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/system/vdalloc.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/Kasumi/pixmapops.h>
#include <vd2/Kasumi/resample.h>
#include <vd2/Kasumi/tables.h>
#include <vd2/Kasumi/triblt.h>

namespace {
	uint32 lerp_RGB888(sint32 a, sint32 b, sint32 x) {
		sint32 a_rb	= a & 0xff00ff;
		sint32 a_g	= a & 0x00ff00;
		sint32 b_rb	= b & 0xff00ff;
		sint32 b_g	= b & 0x00ff00;

		const uint32 top_rb = (a_rb + (((b_rb - a_rb)*x + 0x00800080) >> 8)) & 0xff00ff;
		const uint32 top_g  = (a_g  + (((b_g  - a_g )*x + 0x00008000) >> 8)) & 0x00ff00;

		return top_rb + top_g;
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

	uint32 bicubic_RGB888(const uint32 *src0, const uint32 *src1, const uint32 *src2, const uint32 *src3, sint32 x, sint32 y) {
		const uint32 p00 = src0[0];
		const uint32 p01 = src0[1];
		const uint32 p02 = src0[2];
		const uint32 p03 = src0[3];
		const uint32 p10 = src1[0];
		const uint32 p11 = src1[1];
		const uint32 p12 = src1[2];
		const uint32 p13 = src1[3];
		const uint32 p20 = src2[0];
		const uint32 p21 = src2[1];
		const uint32 p22 = src2[2];
		const uint32 p23 = src2[3];
		const uint32 p30 = src3[0];
		const uint32 p31 = src3[1];
		const uint32 p32 = src3[2];
		const uint32 p33 = src3[3];

		const sint32 *htab = kVDCubicInterpTableFX14_075[x];
		const sint32 *vtab = kVDCubicInterpTableFX14_075[y];

		const int ch0 = htab[0];
		const int ch1 = htab[1];
		const int ch2 = htab[2];
		const int ch3 = htab[3];
		const int cv0 = vtab[0];
		const int cv1 = vtab[1];
		const int cv2 = vtab[2];
		const int cv3 = vtab[3];

		int r0 = ((int)((p00>>16)&0xff) * ch0 + (int)((p01>>16)&0xff) * ch1 + (int)((p02>>16)&0xff) * ch2 + (int)((p03>>16)&0xff) * ch3 + 128) >> 8;
		int g0 = ((int)((p00>> 8)&0xff) * ch0 + (int)((p01>> 8)&0xff) * ch1 + (int)((p02>> 8)&0xff) * ch2 + (int)((p03>> 8)&0xff) * ch3 + 128) >> 8;
		int b0 = ((int)((p00    )&0xff) * ch0 + (int)((p01    )&0xff) * ch1 + (int)((p02    )&0xff) * ch2 + (int)((p03    )&0xff) * ch3 + 128) >> 8;
		int r1 = ((int)((p10>>16)&0xff) * ch0 + (int)((p11>>16)&0xff) * ch1 + (int)((p12>>16)&0xff) * ch2 + (int)((p13>>16)&0xff) * ch3 + 128) >> 8;
		int g1 = ((int)((p10>> 8)&0xff) * ch0 + (int)((p11>> 8)&0xff) * ch1 + (int)((p12>> 8)&0xff) * ch2 + (int)((p13>> 8)&0xff) * ch3 + 128) >> 8;
		int b1 = ((int)((p10    )&0xff) * ch0 + (int)((p11    )&0xff) * ch1 + (int)((p12    )&0xff) * ch2 + (int)((p13    )&0xff) * ch3 + 128) >> 8;
		int r2 = ((int)((p20>>16)&0xff) * ch0 + (int)((p21>>16)&0xff) * ch1 + (int)((p22>>16)&0xff) * ch2 + (int)((p23>>16)&0xff) * ch3 + 128) >> 8;
		int g2 = ((int)((p20>> 8)&0xff) * ch0 + (int)((p21>> 8)&0xff) * ch1 + (int)((p22>> 8)&0xff) * ch2 + (int)((p23>> 8)&0xff) * ch3 + 128) >> 8;
		int b2 = ((int)((p20    )&0xff) * ch0 + (int)((p21    )&0xff) * ch1 + (int)((p22    )&0xff) * ch2 + (int)((p23    )&0xff) * ch3 + 128) >> 8;
		int r3 = ((int)((p30>>16)&0xff) * ch0 + (int)((p31>>16)&0xff) * ch1 + (int)((p32>>16)&0xff) * ch2 + (int)((p33>>16)&0xff) * ch3 + 128) >> 8;
		int g3 = ((int)((p30>> 8)&0xff) * ch0 + (int)((p31>> 8)&0xff) * ch1 + (int)((p32>> 8)&0xff) * ch2 + (int)((p33>> 8)&0xff) * ch3 + 128) >> 8;
		int b3 = ((int)((p30    )&0xff) * ch0 + (int)((p31    )&0xff) * ch1 + (int)((p32    )&0xff) * ch2 + (int)((p33    )&0xff) * ch3 + 128) >> 8;

		int r = (r0 * cv0 + r1 * cv1 + r2 * cv2 + r3 * cv3 + (1<<19)) >> 20;
		int g = (g0 * cv0 + g1 * cv1 + g2 * cv2 + g3 * cv3 + (1<<19)) >> 20;
		int b = (b0 * cv0 + b1 * cv1 + b2 * cv2 + b3 * cv3 + (1<<19)) >> 20;

		if (r<0) r=0; else if (r>255) r=255;
		if (g<0) g=0; else if (g>255) g=255;
		if (b<0) b=0; else if (b>255) b=255;

		return (r<<16) + (g<<8) + b;
	}
}

namespace {
	enum {
		kTop = 1,
		kBottom = 2,
		kLeft = 4,
		kRight = 8,
		kNear = 16,
		kFar = 32
	};

	struct VDTriBltMipInfo {
		const uint32 *mip;
		ptrdiff_t pitch;
		uint32 uvmul, _pad;
	};

	struct VDTriBltInfo {
		VDTriBltMipInfo mips[16];
		uint32 *dst;
		const uint32 *src;
		sint32 width;
		const int *cubictab;
	};

	struct VDTriBltGenInfo {
		float	u;
		float	v;
		float	rhw;
		float	dudx;
		float	dvdx;
		float	drhwdx;
	};

	typedef void (*VDTriBltSpanFunction)(const VDTriBltInfo *);
	typedef void (*VDTriBltGenFunction)(const VDTriBltGenInfo *);

	void vd_triblt_span_point(const VDTriBltInfo *pInfo) {
		sint32 w = -pInfo->width;
		uint32 *dst = pInfo->dst + pInfo->width;
		const uint32 *src = pInfo->src;
		const uint32 *texture = pInfo->mips[0].mip;
		const ptrdiff_t texpitch = pInfo->mips[0].pitch;

		do {
			dst[w] = vdptroffset(texture, texpitch * src[1])[src[0]];
			src += 2;
		} while(++w);
	}

	void vd_triblt_span_bilinear(const VDTriBltInfo *pInfo) {
		sint32 w = -pInfo->width;
		uint32 *dst = pInfo->dst + pInfo->width;
		const uint32 *src = pInfo->src;
		const uint32 *texture = pInfo->mips[0].mip;
		const ptrdiff_t texpitch = pInfo->mips[0].pitch;

		do {
			const sint32 u = src[0];
			const sint32 v = src[1];
			src += 2;
			const uint32 *src1 = vdptroffset(texture, texpitch * (v>>8)) + (u>>8);
			const uint32 *src2 = vdptroffset(src1, texpitch);

			dst[w] = bilerp_RGB888(src1[0], src1[1], src2[0], src2[1], u&255, v&255);
		} while(++w);
	}

	void vd_triblt_span_trilinear(const VDTriBltInfo *pInfo) {
		sint32 w = -pInfo->width;
		uint32 *dst = pInfo->dst + pInfo->width;
		const uint32 *src = pInfo->src;

		do {
			sint32 u = src[0];
			sint32 v = src[1];
			const sint32 lambda = src[2];
			src += 3;

			const sint32 lod = lambda >> 8;

			const uint32 *texture1 = pInfo->mips[lod].mip;
			const ptrdiff_t texpitch1 = pInfo->mips[lod].pitch;
			const uint32 *texture2 = pInfo->mips[lod+1].mip;
			const ptrdiff_t texpitch2 = pInfo->mips[lod+1].pitch;

			u >>= lod;
			v >>= lod;

			u += 128;
			v += 128;

			const uint32 *src1 = vdptroffset(texture1, texpitch1 * (v>>8)) + (u>>8);
			const uint32 *src2 = vdptroffset(src1, texpitch1);
			const uint32 p1 = bilerp_RGB888(src1[0], src1[1], src2[0], src2[1], u&255, v&255);

			u += 128;
			v += 128;

			const uint32 *src3 = vdptroffset(texture2, texpitch2 * (v>>9)) + (u>>9);
			const uint32 *src4 = vdptroffset(src3, texpitch2);
			const uint32 p2 = bilerp_RGB888(src3[0], src3[1], src4[0], src4[1], (u>>1)&255, (v>>1)&255);

			dst[w] = lerp_RGB888(p1, p2, lambda & 255);
		} while(++w);
	}

	void vd_triblt_span_bicubic_mip_linear(const VDTriBltInfo *pInfo) {
		sint32 w = -pInfo->width;
		uint32 *dst = pInfo->dst + pInfo->width;
		const uint32 *src = pInfo->src;

		do {
			sint32 u = src[0];
			sint32 v = src[1];
			const sint32 lambda = src[2];
			src += 3;

			const sint32 lod = lambda >> 8;

			const uint32 *texture1 = pInfo->mips[lod].mip;
			const ptrdiff_t texpitch1 = pInfo->mips[lod].pitch;
			const uint32 *texture2 = pInfo->mips[lod+1].mip;
			const ptrdiff_t texpitch2 = pInfo->mips[lod+1].pitch;

			u >>= lod;
			v >>= lod;

			u += 128;
			v += 128;

			const uint32 *src1 = vdptroffset(texture1, texpitch1 * (v>>8)) + (u>>8);
			const uint32 *src2 = vdptroffset(src1, texpitch1);
			const uint32 *src3 = vdptroffset(src2, texpitch1);
			const uint32 *src4 = vdptroffset(src3, texpitch1);
			const uint32 p1 = bicubic_RGB888(src1, src2, src3, src4, u&255, v&255);

			u += 128;
			v += 128;

			const uint32 *src5 = vdptroffset(texture2, texpitch2 * (v>>9)) + (u>>9);
			const uint32 *src6 = vdptroffset(src5, texpitch2);
			const uint32 *src7 = vdptroffset(src6, texpitch2);
			const uint32 *src8 = vdptroffset(src7, texpitch2);
			const uint32 p2 = bicubic_RGB888(src5, src6, src7, src8, (u>>1)&255, (v>>1)&255);

			dst[w] = lerp_RGB888(p1, p2, lambda & 255);
		} while(++w);
	}

#ifdef _M_IX86
	extern "C" void vdasm_triblt_span_bilinear_mmx(const VDTriBltInfo *pInfo);
	extern "C" void vdasm_triblt_span_trilinear_mmx(const VDTriBltInfo *pInfo);
	extern "C" void vdasm_triblt_span_bicubic_mip_linear_mmx(const VDTriBltInfo *pInfo);
	extern "C" void vdasm_triblt_span_bicubic_mip_linear_sse2(const VDTriBltInfo *pInfo);
	extern "C" void vdasm_triblt_span_point(const VDTriBltInfo *pInfo);
#endif

	struct VDTriBltTransformedVertex {
		float x, y, z;
		union {
			float w;
			float rhw;
		};
		float r, g, b, a;
		float u, v;
		int outcode;

		void interp(const VDTriBltTransformedVertex *v1, const VDTriBltTransformedVertex *v2, float alpha) {
			x = v1->x + alpha * (v2->x - v1->x);
			y = v1->y + alpha * (v2->y - v1->y);
			z = v1->z + alpha * (v2->z - v1->z);
			w = v1->w + alpha * (v2->w - v1->w);

			r = v1->r + alpha * (v2->r - v1->r);
			g = v1->g + alpha * (v2->g - v1->g);
			b = v1->b + alpha * (v2->b - v1->b);
			a = v1->a + alpha * (v2->a - v1->a);

			u = v1->u + alpha * (v2->u - v1->u);
			v = v1->v + alpha * (v2->v - v1->v);

			outcode	= (x < -w ? kLeft : 0)
					+ (x > +w ? kRight : 0)
					+ (y < -w ? kTop : 0)
					+ (y > +w ? kBottom : 0)
					+ (z < -w ? kNear : 0)
					+ (z > +w ? kFar : 0);
		}
	};

	void TransformVerts(VDTriBltTransformedVertex *dst, const VDTriBltVertex *src, int nVerts, const float xform[16]) {
		const float xflocal[16]={
			xform[ 0],	xform[ 1],	xform[ 2],	xform[ 3],
			xform[ 4],	xform[ 5],	xform[ 6],	xform[ 7],
			xform[ 8],	xform[ 9],	xform[10],	xform[11],
			xform[12],	xform[13],	xform[14],	xform[15],
		};

		if (nVerts <= 0)
			return;

		do {
			const float x0 = src->x;
			const float y0 = src->y;
			const float z0 = src->z;

			const float w	= x0*xflocal[12] + y0*xflocal[13] + z0*xflocal[14] + xflocal[15];
			const float x   = x0*xflocal[ 0] + y0*xflocal[ 1] + z0*xflocal[ 2] + xflocal[ 3];
			const float y   = x0*xflocal[ 4] + y0*xflocal[ 5] + z0*xflocal[ 6] + xflocal[ 7];
			const float z   = x0*xflocal[ 8] + y0*xflocal[ 9] + z0*xflocal[10] + xflocal[11];

			int outcode = 0;

			if (x < -w)		outcode += kLeft;
			if (x > w)		outcode += kRight;
			if (y < -w)		outcode += kTop;
			if (y > w)		outcode += kBottom;
			if (z < -w)		outcode += kNear;
			if (z > w)		outcode += kFar;

			dst->x = x;
			dst->y = y;
			dst->z = z;
			dst->w = w;
			dst->u = src->u;
			dst->v = src->v;
			dst->r = 1.0f;
			dst->g = 1.0f;
			dst->b = 1.0f;
			dst->a = 1.0f;
			dst->outcode = outcode;

			++src;
			++dst;
		} while(--nVerts);
	}

	void TransformVerts(VDTriBltTransformedVertex *dst, const VDTriColorVertex *src, int nVerts, const float xform[16]) {
		const float xflocal[16]={
			xform[ 0],	xform[ 1],	xform[ 2],	xform[ 3],
			xform[ 4],	xform[ 5],	xform[ 6],	xform[ 7],
			xform[ 8],	xform[ 9],	xform[10],	xform[11],
			xform[12],	xform[13],	xform[14],	xform[15],
		};

		if (nVerts <= 0)
			return;

		do {
			const float x0 = src->x;
			const float y0 = src->y;
			const float z0 = src->z;

			const float w	= x0*xflocal[12] + y0*xflocal[13] + z0*xflocal[14] + xflocal[15];
			const float x   = x0*xflocal[ 0] + y0*xflocal[ 1] + z0*xflocal[ 2] + xflocal[ 3];
			const float y   = x0*xflocal[ 4] + y0*xflocal[ 5] + z0*xflocal[ 6] + xflocal[ 7];
			const float z   = x0*xflocal[ 8] + y0*xflocal[ 9] + z0*xflocal[10] + xflocal[11];

			int outcode = 0;

			if (x < -w)		outcode += kLeft;
			if (x > w)		outcode += kRight;
			if (y < -w)		outcode += kTop;
			if (y > w)		outcode += kBottom;
			if (z < -w)		outcode += kNear;
			if (z > w)		outcode += kFar;

			dst->x = x;
			dst->y = y;
			dst->z = z;
			dst->w = w;
			dst->u = 0.0f;
			dst->v = 0.0f;
			dst->r = src->r;
			dst->g = src->g;
			dst->b = src->b;
			dst->a = src->a;
			dst->outcode = outcode;

			++src;
			++dst;
		} while(--nVerts);
	}

	struct VDTriangleSetupInfo {
		const VDTriBltTransformedVertex *pt, *pr, *pl;
		VDTriBltTransformedVertex tmp0, tmp1, tmp2;
	};

	void SetupTri(
			VDTriangleSetupInfo& setup,
			VDPixmap& dst,
			const VDTriBltTransformedVertex *vx0,
			const VDTriBltTransformedVertex *vx1,
			const VDTriBltTransformedVertex *vx2,
			const VDTriBltFilterMode *filterMode
			)
	{
		setup.tmp0 = *vx0;
		setup.tmp1 = *vx1;
		setup.tmp2 = *vx2;

		// adjust UVs for filter mode
		if (filterMode) {
			switch(*filterMode) {
			case kTriBltFilterBilinear:
				setup.tmp0.u += 0.5f;
				setup.tmp0.v += 0.5f;
				setup.tmp1.u += 0.5f;
				setup.tmp1.v += 0.5f;
				setup.tmp2.u += 0.5f;
				setup.tmp2.v += 0.5f;
			case kTriBltFilterTrilinear:
			case kTriBltFilterBicubicMipLinear:
				setup.tmp0.u *= 256.0f;
				setup.tmp0.v *= 256.0f;
				setup.tmp1.u *= 256.0f;
				setup.tmp1.v *= 256.0f;
				setup.tmp2.u *= 256.0f;
				setup.tmp2.v *= 256.0f;
				break;
			case kTriBltFilterPoint:
				setup.tmp0.u += 1.0f;
				setup.tmp0.v += 1.0f;
				setup.tmp1.u += 1.0f;
				setup.tmp1.v += 1.0f;
				setup.tmp2.u += 1.0f;
				setup.tmp2.v += 1.0f;
				break;
			}
		}

		// do perspective divide and NDC space conversion
		const float xscale = dst.w * 0.5f;
		const float yscale = dst.h * 0.5f;

		setup.tmp0.rhw = 1.0f / setup.tmp0.w;
		setup.tmp0.x = (1.0f+setup.tmp0.x*setup.tmp0.rhw)*xscale;
		setup.tmp0.y = (1.0f+setup.tmp0.y*setup.tmp0.rhw)*yscale;
		setup.tmp0.u *= setup.tmp0.rhw;
		setup.tmp0.v *= setup.tmp0.rhw;
		setup.tmp0.r *= setup.tmp0.rhw;
		setup.tmp0.g *= setup.tmp0.rhw;
		setup.tmp0.b *= setup.tmp0.rhw;
		setup.tmp0.a *= setup.tmp0.rhw;
		setup.tmp1.rhw = 1.0f / setup.tmp1.w;
		setup.tmp1.x = (1.0f+setup.tmp1.x*setup.tmp1.rhw)*xscale;
		setup.tmp1.y = (1.0f+setup.tmp1.y*setup.tmp1.rhw)*yscale;
		setup.tmp1.u *= setup.tmp1.rhw;
		setup.tmp1.v *= setup.tmp1.rhw;
		setup.tmp1.r *= setup.tmp1.rhw;
		setup.tmp1.g *= setup.tmp1.rhw;
		setup.tmp1.b *= setup.tmp1.rhw;
		setup.tmp1.a *= setup.tmp1.rhw;
		setup.tmp2.rhw = 1.0f / setup.tmp2.w;
		setup.tmp2.x = (1.0f+setup.tmp2.x*setup.tmp2.rhw)*xscale;
		setup.tmp2.y = (1.0f+setup.tmp2.y*setup.tmp2.rhw)*yscale;
		setup.tmp2.u *= setup.tmp2.rhw;
		setup.tmp2.v *= setup.tmp2.rhw;
		setup.tmp2.r *= setup.tmp2.rhw;
		setup.tmp2.g *= setup.tmp2.rhw;
		setup.tmp2.b *= setup.tmp2.rhw;
		setup.tmp2.a *= setup.tmp2.rhw;

		// verify clipping
		VDASSERT(setup.tmp0.x >= 0 && setup.tmp0.x <= dst.w);
		VDASSERT(setup.tmp1.x >= 0 && setup.tmp1.x <= dst.w);
		VDASSERT(setup.tmp2.x >= 0 && setup.tmp2.x <= dst.w);
		VDASSERT(setup.tmp0.y >= 0 && setup.tmp0.y <= dst.h);
		VDASSERT(setup.tmp1.y >= 0 && setup.tmp1.y <= dst.h);
		VDASSERT(setup.tmp2.y >= 0 && setup.tmp2.y <= dst.h);

		vx0 = &setup.tmp0;
		vx1 = &setup.tmp1;
		vx2 = &setup.tmp2;

		const VDTriBltTransformedVertex *pt, *pl, *pr;

		// sort points
		if (vx0->y < vx1->y)		// 1 < 2
			if (vx0->y < vx2->y) {	// 1 < 2,3
				pt = vx0;
				pr = vx1;
				pl = vx2;
			} else {				// 3 < 1 < 2
				pt = vx2;
				pr = vx0;
				pl = vx1;
			}
		else						// 2 < 1
			if (vx1->y < vx2->y) {	// 2 < 1,3
				pt = vx1;
				pr = vx2;
				pl = vx0;
			} else {				// 3 < 2 < 1
				pt = vx2;
				pr = vx0;
				pl = vx1;
			}

		setup.pl = pl;
		setup.pt = pt;
		setup.pr = pr;
	}

	void RenderTri(VDPixmap& dst, const VDPixmap *const *pSources, int nMipmaps,
							const VDTriBltTransformedVertex *vx0,
							const VDTriBltTransformedVertex *vx1,
							const VDTriBltTransformedVertex *vx2,
							VDTriBltFilterMode filterMode,
							float mipMapLODBias)
	{
		VDTriangleSetupInfo setup;

		SetupTri(setup, dst, vx0, vx1, vx2, &filterMode);

		const VDTriBltTransformedVertex *pt = setup.pt, *pl = setup.pl, *pr = setup.pr;

		const float x10 = pl->x - pt->x;
		const float x20 = pr->x - pt->x;
		const float y10 = pl->y - pt->y;
		const float y20 = pr->y - pt->y;
		const float A = x20*y10 - x10*y20;

		if (A <= 0.f)
			return;

		float invA = 0.f;
		if (A >= 1e-5f)
			invA = 1.0f / A;

		float x10_A = x10 * invA;
		float x20_A = x20 * invA;
		float y10_A = y10 * invA;
		float y20_A = y20 * invA;

		float u10 = pl->u - pt->u;
		float u20 = pr->u - pt->u;
		float v10 = pl->v - pt->v;
		float v20 = pr->v - pt->v;
		float rhw10 = pl->rhw - pt->rhw;
		float rhw20 = pr->rhw - pt->rhw;

		float dudx = u20*y10_A - u10*y20_A;
		float dudy = u10*x20_A - u20*x10_A;
		float dvdx = v20*y10_A - v10*y20_A;
		float dvdy = v10*x20_A - v20*x10_A;
		float drhwdx = rhw20*y10_A - rhw10*y20_A;
		float drhwdy = rhw10*x20_A - rhw20*x10_A;

		// Compute edge walking parameters

		float dxl1=0, dxr1=0, dul1=0, dvl1=0, drhwl1=0;
		float dxl2=0, dxr2=0, dul2=0, dvl2=0, drhwl2=0;

		// Compute left-edge interpolation parameters for first half.

		if (pl->y != pt->y) {
			dxl1 = (pl->x - pt->x) / (pl->y - pt->y);

			dul1 = dudy + dxl1 * dudx;
			dvl1 = dvdy + dxl1 * dvdx;
			drhwl1 = drhwdy + dxl1 * drhwdx;
		}

		// Compute right-edge interpolation parameters for first half.

		if (pr->y != pt->y) {
			dxr1 = (pr->x - pt->x) / (pr->y - pt->y);
		}

		// Compute third-edge interpolation parameters.

		if (pr->y != pl->y) {
			dxl2 = (pr->x - pl->x) / (pr->y - pl->y);

			dul2 = dudy + dxl2 * dudx;
			dvl2 = dvdy + dxl2 * dvdx;
			drhwl2 = drhwdy + dxl2 * drhwdx;

			dxr2 = dxl2;
		}

		// Initialize parameters for first half.
		//
		// We place pixel centers at (x+0.5, y+0.5).

		double xl, xr, ul, vl, rhwl, yf;
		int y, y1, y2;

		// y_start < y+0.5 to include pixel y.

		y = (int)floor(pt->y + 0.5);
		yf = (y+0.5) - pt->y;

		xl = pt->x + dxl1 * yf;
		xr = pt->x + dxr1 * yf;
		ul = pt->u + dul1 * yf;
		vl = pt->v + dvl1 * yf;
		rhwl = pt->rhw + drhwl1 * yf;

		// Initialize parameters for second half.

		double xl2, xr2, ul2, vl2, rhwl2;

		if (pl->y > pr->y) {		// Left edge is long side
			dxl2 = dxl1;
			dul2 = dul1;
			dvl2 = dvl1;
			drhwl2 = drhwl1;

			y1 = (int)floor(pr->y + 0.5);
			y2 = (int)floor(pl->y + 0.5);

			yf = (y1+0.5) - pr->y;

			// Step left edge.

			xl2 = xl + dxl1 * (y1 - y);
			ul2 = ul + dul1 * (y1 - y);
			vl2 = vl + dvl1 * (y1 - y);
			rhwl2 = rhwl + drhwl1 * (y1 - y);

			// Prestep right edge.

			xr2 = pr->x + dxr2 * yf;
		} else {					// Right edge is long side
			dxr2 = dxr1;

			y1 = (int)floor(pl->y + 0.5);
			y2 = (int)floor(pr->y + 0.5);

			yf = (y1+0.5) - pl->y;

			// Prestep left edge.

			xl2 = pl->x + dxl2 * yf;
			ul2 = pl->u + dul2 * yf;
			vl2 = pl->v + dvl2 * yf;
			rhwl2 = pl->rhw + drhwl2 * yf;

			// Step right edge.

			xr2 = xr + dxr1 * (y1 - y);
		}

		// rasterize
		const ptrdiff_t dstpitch = dst.pitch;
		uint32 *dstp = (uint32 *)((char *)dst.data + dstpitch * y);

		VDTriBltInfo texinfo;
		VDTriBltSpanFunction drawSpan;
		uint32 cpuflags = CPUGetEnabledExtensions();

		bool triBlt16 = false;

		switch(filterMode) {
		case kTriBltFilterBicubicMipLinear:
#ifdef _M_IX86
			if (cpuflags & CPUF_SUPPORTS_SSE2) {
				drawSpan = vdasm_triblt_span_bicubic_mip_linear_sse2;
				triBlt16 = true;
			} else if (cpuflags & CPUF_SUPPORTS_MMX) {
				drawSpan = vdasm_triblt_span_bicubic_mip_linear_mmx;
				triBlt16 = true;
			} else
#endif
				drawSpan = vd_triblt_span_bicubic_mip_linear;
			break;
		case kTriBltFilterTrilinear:
#ifdef _M_IX86
			if (cpuflags & CPUF_SUPPORTS_MMX) {
				drawSpan = vdasm_triblt_span_trilinear_mmx;
				triBlt16 = true;
			} else
#endif
				drawSpan = vd_triblt_span_trilinear;
			break;
		case kTriBltFilterBilinear:
#ifdef _M_IX86
			if (cpuflags & CPUF_SUPPORTS_MMX) {
				drawSpan = vdasm_triblt_span_bilinear_mmx;
				triBlt16 = true;
			} else
#endif
				drawSpan = vd_triblt_span_bilinear;
			break;
		case kTriBltFilterPoint:
			drawSpan = vd_triblt_span_point;
			break;
		}

		float rhobase = sqrtf(std::max<float>(dudx*dudx + dvdx*dvdx, dudy*dudy + dvdy*dvdy) * (1.0f / 65536.0f)) * powf(2.0f, mipMapLODBias);

		if (triBlt16) {
			ul *= 256.0f;
			vl *= 256.0f;
			ul2 *= 256.0f;
			vl2 *= 256.0f;
			dul1 *= 256.0f;
			dvl1 *= 256.0f;
			dul2 *= 256.0f;
			dvl2 *= 256.0f;
			dudx *= 256.0f;
			dvdx *= 256.0f;
			dudy *= 256.0f;
			dvdy *= 256.0f;
		}

		int minx1 = (int)floor(std::min<float>(std::min<float>(pl->x, pr->x), pt->x) + 0.5);
		int maxx2 = (int)floor(std::max<float>(std::max<float>(pl->x, pr->x), pt->x) + 0.5);

		uint32 *const spanptr = new uint32[3 * (maxx2 - minx1)];

		while(y < y2) {
			if (y == y1) {
				xl = xl2;
				xr = xr2;
				ul = ul2;
				vl = vl2;
				rhwl = rhwl2;
				dxl1 = dxl2;
				dxr1 = dxr2;
				dul1 = dul2;
				dvl1 = dvl2;
				drhwl1 = drhwl2;
			}

			int x1, x2;
			double xf;
			double u, v, rhw;

			// x_left must be less than (x+0.5) to include pixel x.

			x1		= (int)floor(xl + 0.5);
			x2		= (int)floor(xr + 0.5);
			xf		= (x1+0.5) - xl;
			
			u		= ul + xf * dudx;
			v		= vl + xf * dvdx;
			rhw		= rhwl + xf * drhwdx;

			int x = x1;
			uint32 *spanp = spanptr;

			float w = 1.0f / (float)rhw;

			if (x < x2) {
				if (filterMode >= kTriBltFilterTrilinear) {
					do {
						int utexel = VDRoundToIntFastFullRange(u * w);
						int vtexel = VDRoundToIntFastFullRange(v * w);
						union{ float f; sint32 i; } rho = {rhobase * w};

						int lambda = ((rho.i - 0x3F800000) >> (23-8));
						if (lambda < 0)
							lambda = 0;
						if (lambda >= (nMipmaps<<8)-256)
							lambda = (nMipmaps<<8)-257;

						spanp[0] = utexel;
						spanp[1] = vtexel;
						spanp[2] = lambda;
						spanp += 3;

						u += dudx;
						v += dvdx;
						rhw += drhwdx;

						w *= (2.0f - w*(float)rhw);
					} while(++x < x2);
				} else {
					do {
						int utexel = VDFloorToInt(u * w);
						int vtexel = VDFloorToInt(v * w);

						spanp[0] = utexel;
						spanp[1] = vtexel;
						spanp += 2;

						u += dudx;
						v += dvdx;
						rhw += drhwdx;

						w *= (2.0f - w*(float)rhw);
					} while(++x < x2);
				}
			}

			for(int i=0; i<nMipmaps; ++i) {
				texinfo.mips[i].mip		= (const uint32 *)pSources[i]->data;
				texinfo.mips[i].pitch	= pSources[i]->pitch;
				texinfo.mips[i].uvmul	= (pSources[i]->pitch << 16) + 4;
			}
			texinfo.dst = dstp+x1;
			texinfo.src = spanptr;
			texinfo.width = x2-x1;

			if (texinfo.width>0)
				drawSpan(&texinfo);

			dstp = vdptroffset(dstp, dstpitch);
			xl += dxl1;
			xr += dxr1;
			ul += dul1;
			vl += dvl1;
			rhwl += drhwl1;

			++y;
		}

		delete[] spanptr;
	}

	void FillTri(VDPixmap& dst, uint32 c,
					const VDTriBltTransformedVertex *vx0,
					const VDTriBltTransformedVertex *vx1,
					const VDTriBltTransformedVertex *vx2
					)
	{

		VDTriangleSetupInfo setup;

		SetupTri(setup, dst, vx0, vx1, vx2, NULL);

		const VDTriBltTransformedVertex *pt = setup.pt, *pl = setup.pl, *pr = setup.pr;

		// Compute edge walking parameters
		float dxl1=0, dxr1=0;
		float dxl2=0, dxr2=0;

		float x_lt = pl->x - pt->x;
		float x_rt = pr->x - pt->x;
		float x_rl = pr->x - pl->x;
		float y_lt = pl->y - pt->y;
		float y_rt = pr->y - pt->y;
		float y_rl = pr->y - pl->y;

		// reject backfaces
		if (x_lt*y_rt >= x_rt*y_lt)
			return;

		// Compute left-edge interpolation parameters for first half.
		if (pl->y != pt->y)
			dxl1 = x_lt / y_lt;

		// Compute right-edge interpolation parameters for first half.
		if (pr->y != pt->y)
			dxr1 = x_rt / y_rt;

		// Compute third-edge interpolation parameters.
		if (pr->y != pl->y) {
			dxl2 = x_rl / y_rl;

			dxr2 = dxl2;
		}

		// Initialize parameters for first half.
		//
		// We place pixel centers at (x+0.5, y+0.5).

		double xl, xr, yf;
		int y, y1, y2;

		// y_start < y+0.5 to include pixel y.

		y = (int)floor(pt->y + 0.5);
		yf = (y+0.5) - pt->y;

		xl = pt->x + dxl1 * yf;
		xr = pt->x + dxr1 * yf;

		// Initialize parameters for second half.
		double xl2, xr2;

		if (pl->y > pr->y) {		// Left edge is long side
			dxl2 = dxl1;

			y1 = (int)floor(pr->y + 0.5);
			y2 = (int)floor(pl->y + 0.5);

			yf = (y1+0.5) - pr->y;

			// Prestep right edge.
			xr2 = pr->x + dxr2 * yf;

			// Step left edge.
			xl2 = xl + dxl1 * (y1 - y);
		} else {					// Right edge is long side
			dxr2 = dxr1;

			y1 = (int)floor(pl->y + 0.5);
			y2 = (int)floor(pr->y + 0.5);

			yf = (y1+0.5) - pl->y;

			// Prestep left edge.
			xl2 = pl->x + dxl2 * yf;

			// Step right edge.
			xr2 = xr + dxr1 * (y1 - y);
		}

		// rasterize
		const ptrdiff_t dstpitch = dst.pitch;
		uint32 *dstp = (uint32 *)((char *)dst.data + dstpitch * y);

		while(y < y2) {
			if (y == y1) {
				xl = xl2;
				xr = xr2;
				dxl1 = dxl2;
				dxr1 = dxr2;
			}

			int x1, x2;
			double xf;

			// x_left must be less than (x+0.5) to include pixel x.

			x1		= (int)floor(xl + 0.5);
			x2		= (int)floor(xr + 0.5);
			xf		= (x1+0.5) - xl;
			
			while(x1 < x2)
				dstp[x1++] = c;

			dstp = vdptroffset(dstp, dstpitch);
			xl += dxl1;
			xr += dxr1;
			++y;
		}
	}

	void FillTriGrad(VDPixmap& dst,
					const VDTriBltTransformedVertex *vx0,
					const VDTriBltTransformedVertex *vx1,
					const VDTriBltTransformedVertex *vx2
					)
	{

		VDTriangleSetupInfo setup;

		SetupTri(setup, dst, vx0, vx1, vx2, NULL);

		const VDTriBltTransformedVertex *pt = setup.pt, *pl = setup.pl, *pr = setup.pr;
		const float x10 = pl->x - pt->x;
		const float x20 = pr->x - pt->x;
		const float y10 = pl->y - pt->y;
		const float y20 = pr->y - pt->y;
		const float A = x20*y10 - x10*y20;

		if (A <= 0.f)
			return;

		float invA = 0.f;
		if (A >= 1e-5f)
			invA = 1.0f / A;

		float x10_A = x10 * invA;
		float x20_A = x20 * invA;
		float y10_A = y10 * invA;
		float y20_A = y20 * invA;

		float r10 = pl->r - pt->r;
		float r20 = pr->r - pt->r;
		float g10 = pl->g - pt->g;
		float g20 = pr->g - pt->g;
		float b10 = pl->b - pt->b;
		float b20 = pr->b - pt->b;
		float a10 = pl->a - pt->a;
		float a20 = pr->a - pt->a;
		float rhw10 = pl->rhw - pt->rhw;
		float rhw20 = pr->rhw - pt->rhw;

		float drdx = r20*y10_A - r10*y20_A;
		float drdy = r10*x20_A - r20*x10_A;
		float dgdx = g20*y10_A - g10*y20_A;
		float dgdy = g10*x20_A - g20*x10_A;
		float dbdx = b20*y10_A - b10*y20_A;
		float dbdy = b10*x20_A - b20*x10_A;
		float dadx = a20*y10_A - a10*y20_A;
		float dady = a10*x20_A - a20*x10_A;
		float drhwdx = rhw20*y10_A - rhw10*y20_A;
		float drhwdy = rhw10*x20_A - rhw20*x10_A;

		// Compute edge walking parameters
		float dxl1=0;
		float drl1=0;
		float dgl1=0;
		float dbl1=0;
		float dal1=0;
		float drhwl1=0;
		float dxr1=0;
		float dxl2=0;
		float drl2=0;
		float dgl2=0;
		float dbl2=0;
		float dal2=0;
		float drhwl2=0;
		float dxr2=0;

		float x_lt = pl->x - pt->x;
		float x_rt = pr->x - pt->x;
		float x_rl = pr->x - pl->x;
		float y_lt = pl->y - pt->y;
		float y_rt = pr->y - pt->y;
		float y_rl = pr->y - pl->y;

		// Compute left-edge interpolation parameters for first half.
		if (pl->y != pt->y) {
			dxl1 = x_lt / y_lt;
			drl1 = drdy + dxl1 * drdx;
			dgl1 = dgdy + dxl1 * dgdx;
			dbl1 = dbdy + dxl1 * dbdx;
			dal1 = dady + dxl1 * dadx;
			drhwl1 = drhwdy + dxl1 * drhwdx;
		}

		// Compute right-edge interpolation parameters for first half.
		if (pr->y != pt->y)
			dxr1 = x_rt / y_rt;

		// Compute third-edge interpolation parameters.
		if (pr->y != pl->y) {
			dxl2 = x_rl / y_rl;

			drl2 = drdy + dxl2 * drdx;
			dgl2 = dgdy + dxl2 * dgdx;
			dbl2 = dbdy + dxl2 * dbdx;
			dal2 = dady + dxl2 * dadx;
			drhwl2 = drhwdy + dxl2 * drhwdx;

			dxr2 = dxl2;
		}

		// Initialize parameters for first half.
		//
		// We place pixel centers at (x+0.5, y+0.5).

		double xl, xr, yf;
		double rl, gl, bl, al, rhwl;
		double rl2, gl2, bl2, al2, rhwl2;
		int y, y1, y2;

		// y_start < y+0.5 to include pixel y.

		y = (int)floor(pt->y + 0.5);
		yf = (y+0.5) - pt->y;

		xl = pt->x + dxl1 * yf;
		xr = pt->x + dxr1 * yf;
		rl = pt->r + drl1 * yf;
		gl = pt->g + dgl1 * yf;
		bl = pt->b + dbl1 * yf;
		al = pt->a + dal1 * yf;
		rhwl = pt->rhw + drhwl1 * yf;

		// Initialize parameters for second half.
		double xl2, xr2;

		if (pl->y > pr->y) {		// Left edge is long side
			dxl2 = dxl1;
			drl2 = drl1;
			dgl2 = dgl1;
			dbl2 = dbl1;
			dal2 = dal1;
			drhwl2 = drhwl1;

			y1 = (int)floor(pr->y + 0.5);
			y2 = (int)floor(pl->y + 0.5);

			yf = (y1+0.5) - pr->y;

			// Step left edge.
			xl2 = xl + dxl1 * (y1 - y);
			rl2 = rl + drl1 * (y1 - y);
			gl2 = gl + dgl1 * (y1 - y);
			bl2 = bl + dbl1 * (y1 - y);
			al2 = al + dal1 * (y1 - y);
			rhwl2 = rhwl + drhwl1 * (y1 - y);

			// Prestep right edge.
			xr2 = pr->x + dxr2 * yf;
		} else {					// Right edge is long side
			dxr2 = dxr1;

			y1 = (int)floor(pl->y + 0.5);
			y2 = (int)floor(pr->y + 0.5);

			yf = (y1+0.5) - pl->y;

			// Prestep left edge.
			xl2 = pl->x + dxl2 * yf;
			rl2 = pl->r + drl2 * yf;
			gl2 = pl->g + dgl2 * yf;
			bl2 = pl->b + dbl2 * yf;
			al2 = pl->a + dal2 * yf;
			rhwl2 = pl->rhw + drhwl2 * yf;

			// Step right edge.
			xr2 = xr + dxr2 * (y1 - y);
		}

		// rasterize
		const ptrdiff_t dstpitch = dst.pitch;
		char *dstp0 = (char *)dst.data + dstpitch * y;

		while(y < y2) {
			if (y == y1) {
				xl = xl2;
				xr = xr2;
				rl = rl2;
				gl = gl2;
				bl = bl2;
				al = al2;
				rhwl = rhwl2;
				dxl1 = dxl2;
				drl1 = drl2;
				dgl1 = dgl2;
				dbl1 = dbl2;
				dal1 = dal2;
				drhwl1 = drhwl2;
				dxr1 = dxr2;
			}

			int x1, x2;
			double xf;
			double r, g, b, a, rhw;

			// x_left must be less than (x+0.5) to include pixel x.

			x1		= (int)floor(xl + 0.5);
			x2		= (int)floor(xr + 0.5);
			xf		= (x1+0.5) - xl;
			
			r		= rl + xf * drdx;
			g		= gl + xf * dgdx;
			b		= bl + xf * dbdx;
			a		= al + xf * dadx;
			rhw		= rhwl + xf * drhwdx;

			float w = 1.0f / (float)rhw;

			if (x1 < x2) {
				if (dst.format == nsVDPixmap::kPixFormat_XRGB8888) {
					uint32 *dstp = (uint32 *)dstp0;

					do {
						float sr = (float)(r * w);
						float sg = (float)(g * w);
						float sb = (float)(b * w);
						float sa = (float)(a * w);

						uint8 ir = VDClampedRoundFixedToUint8Fast(sr);
						uint8 ig = VDClampedRoundFixedToUint8Fast(sg);
						uint8 ib = VDClampedRoundFixedToUint8Fast(sb);
						uint8 ia = VDClampedRoundFixedToUint8Fast(sa);

						dstp[x1] = ((uint32)ia << 24) + ((uint32)ir << 16) + ((uint32)ig << 8) + ib;

						r += drdx;
						g += dgdx;
						b += dbdx;
						a += dadx;
						rhw += drhwdx;

						w *= (2.0f - w*(float)rhw);
					} while(++x1 < x2);
				} else {
					uint8 *dstp = (uint8 *)dstp0;

					do {
						float sg = (float)(g * w);

						uint8 ig = VDClampedRoundFixedToUint8Fast(sg);

						dstp[x1] = ig;

						g += dgdx;
						rhw += drhwdx;

						w *= (2.0f - w*(float)rhw);
					} while(++x1 < x2);
				}
			}

			dstp0 = vdptroffset(dstp0, dstpitch);
			xl += dxl1;
			rl += drl1;
			gl += dgl1;
			bl += dbl1;
			al += dal1;
			rhwl += drhwl1;
			xr += dxr1;
			++y;
		}
	}

	struct VDTriClipWorkspace {
		VDTriBltTransformedVertex *vxheapptr[2][19];
		VDTriBltTransformedVertex vxheap[21];
	};

	VDTriBltTransformedVertex **VDClipTriangle(VDTriClipWorkspace& ws,
						const VDTriBltTransformedVertex *vx0,
						const VDTriBltTransformedVertex *vx1,
						const VDTriBltTransformedVertex *vx2,
						int orflags) {
		// Each line segment can intersect all six planes, meaning the maximum bound is
		// 18 vertices.  Add 3 for the original.

		VDTriBltTransformedVertex *vxheapnext;
		VDTriBltTransformedVertex **vxlastheap = ws.vxheapptr[0], **vxnextheap = ws.vxheapptr[1];

		ws.vxheap[0]	= *vx0;
		ws.vxheap[1]	= *vx1;
		ws.vxheap[2]	= *vx2;

		vxlastheap[0] = &ws.vxheap[0];
		vxlastheap[1] = &ws.vxheap[1];
		vxlastheap[2] = &ws.vxheap[2];
		vxlastheap[3] = NULL;

		vxheapnext = ws.vxheap + 3;

		//	Current		Next		Action
		//	-------		----		------
		//	Unclipped	Unclipped	Copy vertex
		//	Unclipped	Clipped		Copy vertex and add intersection
		//	Clipped		Unclipped	Add intersection
		//	Clipped		Clipped		No action

#define	DOCLIP(cliptype, _sign_, cliparg)				\
		if (orflags & k##cliptype) {					\
			VDTriBltTransformedVertex **src = vxlastheap;		\
			VDTriBltTransformedVertex **dst = vxnextheap;		\
														\
			while(*src) {								\
				VDTriBltTransformedVertex *cur = *src;			\
				VDTriBltTransformedVertex *next = src[1];		\
														\
				if (!next)								\
					next = vxlastheap[0];				\
														\
				if (!(cur->outcode & k##cliptype))	\
					*dst++ = cur;						\
														\
				if ((cur->outcode ^ next->outcode) & k##cliptype) {	\
					double alpha = (cur->w _sign_ cur->cliparg) / ((cur->w _sign_ cur->cliparg) - (next->w _sign_ next->cliparg));	\
														\
					if (alpha >= 0.0 && alpha <= 1.0) {	\
						vxheapnext->interp(cur, next, (float)alpha);	\
						vxheapnext->cliparg = -(_sign_ vxheapnext->w);	\
						*dst++ = vxheapnext++;			\
					}									\
				}										\
				++src;									\
			}											\
			*dst = NULL;								\
			if (dst < vxnextheap+3) return NULL;		\
			src = vxlastheap; vxlastheap = vxnextheap; vxnextheap = src;	\
		}


		DOCLIP(Far, -, z);
		DOCLIP(Near, +, z);
		DOCLIP(Bottom, -, y);
		DOCLIP(Top, +, y);
		DOCLIP(Right, -, x);
		DOCLIP(Left, +, x);

#undef DOCLIP

		return vxlastheap;
	}

	void RenderClippedTri(VDPixmap& dst, const VDPixmap *const *pSources, int nMipmaps,
							const VDTriBltTransformedVertex *vx0,
							const VDTriBltTransformedVertex *vx1,
							const VDTriBltTransformedVertex *vx2,
							VDTriBltFilterMode filterMode,
							float mipMapLODBias,
							int orflags)
	{

		VDTriBltTransformedVertex *vxheapnext;
		VDTriBltTransformedVertex vxheap[21];

		VDTriBltTransformedVertex *vxheapptr[2][19];
		VDTriBltTransformedVertex **vxlastheap = vxheapptr[0], **vxnextheap = vxheapptr[1];

		vxheap[0]	= *vx0;
		vxheap[1]	= *vx1;
		vxheap[2]	= *vx2;

		vxlastheap[0] = &vxheap[0];
		vxlastheap[1] = &vxheap[1];
		vxlastheap[2] = &vxheap[2];
		vxlastheap[3] = NULL;

		vxheapnext = vxheap + 3;

		//	Current		Next		Action
		//	-------		----		------
		//	Unclipped	Unclipped	Copy vertex
		//	Unclipped	Clipped		Copy vertex and add intersection
		//	Clipped		Unclipped	Add intersection
		//	Clipped		Clipped		No action

#define	DOCLIP(cliptype, _sign_, cliparg)				\
		if (orflags & k##cliptype) {					\
			VDTriBltTransformedVertex **src = vxlastheap;		\
			VDTriBltTransformedVertex **dst = vxnextheap;		\
														\
			while(*src) {								\
				VDTriBltTransformedVertex *cur = *src;			\
				VDTriBltTransformedVertex *next = src[1];		\
														\
				if (!next)								\
					next = vxlastheap[0];				\
														\
				if (!(cur->outcode & k##cliptype))	\
					*dst++ = cur;						\
														\
				if ((cur->outcode ^ next->outcode) & k##cliptype) {	\
					double alpha = (cur->w _sign_ cur->cliparg) / ((cur->w _sign_ cur->cliparg) - (next->w _sign_ next->cliparg));	\
														\
					if (alpha >= 0.0 && alpha <= 1.0) {	\
						vxheapnext->interp(cur, next, (float)alpha);	\
						vxheapnext->cliparg = -(_sign_ vxheapnext->w);	\
						*dst++ = vxheapnext++;			\
					}									\
				}										\
				++src;									\
			}											\
			*dst = NULL;								\
			if (dst < vxnextheap+3) return;				\
			src = vxlastheap; vxlastheap = vxnextheap; vxnextheap = src;	\
		}


		DOCLIP(Far, -, z);
		DOCLIP(Near, +, z);
		DOCLIP(Bottom, -, y);
		DOCLIP(Top, +, y);
		DOCLIP(Right, -, x);
		DOCLIP(Left, +, x);

#undef DOCLIP

		VDTriBltTransformedVertex **src = vxlastheap+1;

		while(src[1]) {
			RenderTri(dst, pSources, nMipmaps, vxlastheap[0], src[0], src[1], filterMode, mipMapLODBias);
			++src;
		}
	}

}

bool VDPixmapTriFill(VDPixmap& dst, const uint32 c, const VDTriBltVertex *pVertices, int nVertices, const int *pIndices, int nIndices, const float pTransform[16]) {
	if (dst.format != nsVDPixmap::kPixFormat_XRGB8888)
		return false;

	static const float xf_ident[16]={1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f};
	vdfastvector<VDTriBltTransformedVertex>	xverts(nVertices);

	if (!pTransform)
		pTransform = xf_ident;

	TransformVerts(xverts.data(), pVertices, nVertices, pTransform);

	const VDTriBltTransformedVertex *xsrc = xverts.data();

	VDTriClipWorkspace clipws;

	while(nIndices >= 3) {
		const int idx0 = pIndices[0];
		const int idx1 = pIndices[1];
		const int idx2 = pIndices[2];
		const VDTriBltTransformedVertex *xv0 = &xsrc[idx0];
		const VDTriBltTransformedVertex *xv1 = &xsrc[idx1];
		const VDTriBltTransformedVertex *xv2 = &xsrc[idx2];
		const int kode0 = xv0->outcode;
		const int kode1 = xv1->outcode;
		const int kode2 = xv2->outcode;

		if (!(kode0 & kode1 & kode2)) {
			if (int orflags = kode0 | kode1 | kode2) {
				VDTriBltTransformedVertex **src = VDClipTriangle(clipws, xv0, xv1, xv2, orflags);

				if (src) {
					VDTriBltTransformedVertex *src0 = *src++;

					// fan out triangles
					while(src[1]) {
						FillTri(dst, c, src0, src[0], src[1]);
						++src;
					}
				}
			} else
				FillTri(dst, c, xv0, xv1, xv2);
		}

		pIndices += 3;
		nIndices -= 3;
	}

	return true;
}

bool VDPixmapTriFill(VDPixmap& dst, const VDTriColorVertex *pVertices, int nVertices, const int *pIndices, int nIndices, const float pTransform[16], const float *chroma_yoffset) {
	VDPixmap pxY;
	VDPixmap pxCb;
	VDPixmap pxCr;
	bool ycbcr = false;
	float ycbcr_xoffset = 0;

	switch(dst.format) {
	case nsVDPixmap::kPixFormat_XRGB8888:
	case nsVDPixmap::kPixFormat_Y8:
		break;
	case nsVDPixmap::kPixFormat_YUV444_Planar:
	case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV444_Planar_709:
	case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
	case nsVDPixmap::kPixFormat_YUV422_Planar:
	case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV422_Planar_709:
	case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
	case nsVDPixmap::kPixFormat_YUV420_Planar:
	case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV420_Planar_709:
	case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
	case nsVDPixmap::kPixFormat_YUV410_Planar:
	case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
	case nsVDPixmap::kPixFormat_YUV410_Planar_709:
	case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
		pxY.format = nsVDPixmap::kPixFormat_Y8;
		pxY.data = dst.data;
		pxY.pitch = dst.pitch;
		pxY.w = dst.w;
		pxY.h = dst.h;

		pxCb.format = nsVDPixmap::kPixFormat_Y8;
		pxCb.data = dst.data2;
		pxCb.pitch = dst.pitch2;
		pxCb.w = dst.w;
		pxCb.h = dst.h;

		pxCr.format = nsVDPixmap::kPixFormat_Y8;
		pxCr.data = dst.data3;
		pxCr.pitch = dst.pitch3;
		pxCr.w = dst.w;
		pxCr.h = dst.h;

		switch(dst.format) {
			case nsVDPixmap::kPixFormat_YUV410_Planar:
			case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV410_Planar_709:
			case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w >> 2;
				pxCr.h = pxCb.h = dst.h >> 2;
				ycbcr_xoffset = 0.75f / (float)pxCr.w;
				break;

			case nsVDPixmap::kPixFormat_YUV420_Planar:
			case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV420_Planar_709:
			case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w >> 1;
				pxCr.h = pxCb.h = dst.h >> 1;
				ycbcr_xoffset = 0.5f / (float)pxCr.w;
				break;

			case nsVDPixmap::kPixFormat_YUV422_Planar:
			case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV422_Planar_709:
			case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w >> 1;
				ycbcr_xoffset = 0.5f / (float)pxCr.w;
				break;

			case nsVDPixmap::kPixFormat_YUV444_Planar:
			case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
			case nsVDPixmap::kPixFormat_YUV444_Planar_709:
			case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
				pxCr.w = pxCb.w = dst.w;
				ycbcr_xoffset = 0.0f;
				break;
		}

		ycbcr = true;
		break;
	default:
		return false;
	}

	VDTriBltTransformedVertex fastxverts[64];
	vdfastvector<VDTriBltTransformedVertex>	xverts;

	VDTriBltTransformedVertex *xsrc;
	if (nVertices <= 64) {
		xsrc = fastxverts;
	} else {
		xverts.resize(nVertices);
		xsrc = xverts.data();
	}

	static const float xf_ident[16]={1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f};
	if (!pTransform)
		pTransform = xf_ident;

	VDTriClipWorkspace clipws;
	for(int plane=0; plane<(ycbcr?3:1); ++plane) {
		VDPixmap& pxPlane = ycbcr ? plane == 0 ? pxY : plane == 1 ? pxCb : pxCr : dst;

		if (ycbcr && plane) {
			float xf_ycbcr[16];
			memcpy(xf_ycbcr, pTransform, sizeof(float) * 16);

			// translate in x by ycbcr_xoffset
			xf_ycbcr[0] += xf_ycbcr[12]*ycbcr_xoffset;
			xf_ycbcr[1] += xf_ycbcr[13]*ycbcr_xoffset;
			xf_ycbcr[2] += xf_ycbcr[14]*ycbcr_xoffset;
			xf_ycbcr[3] += xf_ycbcr[15]*ycbcr_xoffset;

			// translate in y by chroma_yoffset
			if (chroma_yoffset) {
				xf_ycbcr[4] += xf_ycbcr[12]*(*chroma_yoffset);
				xf_ycbcr[5] += xf_ycbcr[13]*(*chroma_yoffset);
				xf_ycbcr[6] += xf_ycbcr[14]*(*chroma_yoffset);
				xf_ycbcr[7] += xf_ycbcr[15]*(*chroma_yoffset);
			}

			TransformVerts(xsrc, pVertices, nVertices, xf_ycbcr);

			switch(plane) {
				case 1:
					for(int i=0; i<nVertices; ++i)
						xsrc[i].g = xsrc[i].b;
					break;
				case 2:
					for(int i=0; i<nVertices; ++i)
						xsrc[i].g = xsrc[i].r;
					break;
			}
		} else {
			TransformVerts(xsrc, pVertices, nVertices, pTransform);
		}

		const int *nextIndex = pIndices;
		int indicesLeft = nIndices;
		while(indicesLeft >= 3) {
			const int idx0 = nextIndex[0];
			const int idx1 = nextIndex[1];
			const int idx2 = nextIndex[2];
			const VDTriBltTransformedVertex *xv0 = &xsrc[idx0];
			const VDTriBltTransformedVertex *xv1 = &xsrc[idx1];
			const VDTriBltTransformedVertex *xv2 = &xsrc[idx2];
			const int kode0 = xv0->outcode;
			const int kode1 = xv1->outcode;
			const int kode2 = xv2->outcode;

			if (!(kode0 & kode1 & kode2)) {
				if (int orflags = kode0 | kode1 | kode2) {
					VDTriBltTransformedVertex **src = VDClipTriangle(clipws, xv0, xv1, xv2, orflags);

					if (src) {
						VDTriBltTransformedVertex *src0 = *src++;

						// fan out triangles
						while(src[1]) {
							FillTriGrad(pxPlane, src0, src[0], src[1]);
							++src;
						}
					}
				} else {
					FillTriGrad(pxPlane, xv0, xv1, xv2);
				}
			}

			nextIndex += 3;
			indicesLeft -= 3;
		}
	}

	return true;
}

bool VDPixmapTriBlt(VDPixmap& dst, const VDPixmap *const *pSources, int nMipmaps,
					const VDTriBltVertex *pVertices, int nVertices,
					const int *pIndices, int nIndices,
					VDTriBltFilterMode filterMode,
					float mipMapLODBias,
					const float pTransform[16])
{
	if (dst.format != nsVDPixmap::kPixFormat_XRGB8888)
		return false;

	static const float xf_ident[16]={1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f};
	vdfastvector<VDTriBltTransformedVertex>	xverts(nVertices);

	if (!pTransform)
		pTransform = xf_ident;

	TransformVerts(xverts.data(), pVertices, nVertices, pTransform);

	const VDTriBltTransformedVertex *xsrc = xverts.data();

	VDTriClipWorkspace clipws;

	while(nIndices >= 3) {
		const int idx0 = pIndices[0];
		const int idx1 = pIndices[1];
		const int idx2 = pIndices[2];
		const VDTriBltTransformedVertex *xv0 = &xsrc[idx0];
		const VDTriBltTransformedVertex *xv1 = &xsrc[idx1];
		const VDTriBltTransformedVertex *xv2 = &xsrc[idx2];
		const int kode0 = xv0->outcode;
		const int kode1 = xv1->outcode;
		const int kode2 = xv2->outcode;

		if (!(kode0 & kode1 & kode2)) {
			if (int orflags = kode0 | kode1 | kode2) {
				VDTriBltTransformedVertex **src = VDClipTriangle(clipws, xv0, xv1, xv2, orflags);

				if (src) {
					VDTriBltTransformedVertex *src0 = *src++;

					// fan out triangles
					while(src[1]) {
						RenderTri(dst, pSources, nMipmaps, src0, src[0], src[1], filterMode, mipMapLODBias);
						++src;
					}
				}
			} else
				RenderTri(dst, pSources, nMipmaps, xv0, xv1, xv2, filterMode, mipMapLODBias);
		}

		pIndices += 3;
		nIndices -= 3;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////

void VDPixmapSetTextureBorders(VDPixmap& px, bool wrap) {
	const int w = px.w;
	const int h = px.h;

	VDPixmapBlt(px, 0,   1,   px, wrap ? w-2 : 1, 1,              1, h-2);
	VDPixmapBlt(px, w-1, 1,   px, wrap ? 1 : w-2, 1,              1, h-2);

	VDPixmapBlt(px, 0,   0,   px, 0,              wrap ? h-2 : 1, w, 1);
	VDPixmapBlt(px, 0,   h-1, px, 0,              wrap ? 1 : h-2, w, 1);
}

void VDPixmapSetTextureBordersCubic(VDPixmap& px) {
	const int w = px.w;
	const int h = px.h;

	VDPixmapBlt(px, 0,   1, px, 2, 1, 1, h-2);
	VDPixmapBlt(px, 1,   1, px, 2, 1, 1, h-2);
	VDPixmapBlt(px, w-2, 1, px, w-3, 1, 1, h-2);
	VDPixmapBlt(px, w-1, 1, px, w-3, 1, 1, h-2);

	VDPixmapBlt(px, 0, 0,   px, 0, 2, w, 1);
	VDPixmapBlt(px, 0, 1,   px, 0, 2, w, 1);
	VDPixmapBlt(px, 0, h-2, px, 0, h-3, w, 1);
	VDPixmapBlt(px, 0, h-1, px, 0, h-3, w, 1);
}

///////////////////////////////////////////////////////////////////////////

VDPixmapTextureMipmapChain::VDPixmapTextureMipmapChain(const VDPixmap& src, bool wrap, bool cubic, int maxlevels) {
	int w = src.w;
	int h = src.h;
	int mipcount = 0;

	while((w>1 || h>1) && maxlevels--) {
		++mipcount;
		w >>= 1;
		h >>= 1;
	}

	mBuffers.resize(mipcount);
	mMipMaps.resize(mipcount);

	vdautoptr<IVDPixmapResampler> r(VDCreatePixmapResampler());
	r->SetFilters(IVDPixmapResampler::kFilterLinear, IVDPixmapResampler::kFilterLinear, false);

	float fw = (float)src.w;
	float fh = (float)src.h;
	for(int mip=0; mip<mipcount; ++mip) {
		const int mipw = VDCeilToInt(fw);
		const int miph = VDCeilToInt(fh);

		mMipMaps[mip] = &mBuffers[mip];

		if (cubic) {
			mBuffers[mip].init(mipw+4, miph+4, nsVDPixmap::kPixFormat_XRGB8888);

			if (!mip) {
				VDPixmapBlt(mBuffers[0], 2, 2, src, 0, 0, src.w, src.h);
				VDPixmapSetTextureBordersCubic(mBuffers[0]);
			} else {
				const VDPixmap& curmip = mBuffers[mip];
				const VDPixmap& prevmip = mBuffers[mip-1];

				vdrect32f rdst( 0.0f,  0.0f,      (float)curmip.w       ,      (float)curmip.h       );
				vdrect32f rsrc(-2.0f, -2.0f, 2.0f*(float)curmip.w - 2.0f, 2.0f*(float)curmip.h - 2.0f);
				r->Init(rdst, curmip.w, curmip.h, curmip.format, rsrc, prevmip.w, prevmip.h, prevmip.format);
				r->Process(curmip, prevmip);
			}
		} else {
			mBuffers[mip].init(mipw+2, miph+2, nsVDPixmap::kPixFormat_XRGB8888);

			if (!mip) {
				VDPixmapBlt(mBuffers[0], 1, 1, src, 0, 0, src.w, src.h);
				VDPixmapSetTextureBorders(mBuffers[0], wrap);
			} else {
				const VDPixmap& curmip = mBuffers[mip];
				const VDPixmap& prevmip = mBuffers[mip-1];

				vdrect32f rdst( 0.0f,  0.0f,      (float)curmip.w       ,      (float)curmip.h       );
				vdrect32f rsrc(-1.0f, -1.0f, 2.0f*(float)curmip.w - 1.0f, 2.0f*(float)curmip.h - 1.0f);
				r->Init(rdst, curmip.w, curmip.h, curmip.format, rsrc, prevmip.w, prevmip.h, prevmip.format);
				r->Process(curmip, prevmip);
			}
		}

		fw *= 0.5f;
		fh *= 0.5f;
	}
}

