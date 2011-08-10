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
#include <numeric>
#include "blt_spanutils_x86.h"
#include "resample_stages_x86.h"
#include <vd2/Kasumi/resample_kernels.h>

#ifdef _MSC_VER
	#pragma warning(disable: 4799)		// warning C4799: function 'vdasm_resize_table_row_8_k8_4x_MMX' has no EMMS instruction
#endif

///////////////////////////////////////////////////////////////////////////////

extern "C" void vdasm_resize_table_row_8_k8_4x_SSE41(void *dst, const void *src, uint32 width, const void *kernel);
extern "C" void vdasm_resize_table_row_8_k16_4x_SSE41(void *dst, const void *src, uint32 width, const void *kernel);
extern "C" void vdasm_resize_table_row_8_SSE41(void *dst, const void *src, uint32 width, const void *kernel, uint32 kwidth);
extern "C" void vdasm_resize_table_col_8_k2_SSE41(void *dst, const void *const *srcs, uint32 width, const void *kernel);
extern "C" void vdasm_resize_table_col_8_k4_SSE41(void *dst, const void *const *srcs, uint32 width, const void *kernel);

///////////////////////////////////////////////////////////////////////////////

namespace {
	struct ScaleInfo {
		void *dst;
		uintptr	src;
		uint32	accum;
		uint32	fracinc;
		sint32	intinc;
		uint32	count;
	};

	extern "C" void vdasm_resize_point32(const ScaleInfo *);
}

int VDResamplerSeparablePointRowStageX86::GetWindowSize() const {return 1;}
void VDResamplerSeparablePointRowStageX86::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	ScaleInfo info;

	info.dst = (uint32 *)dst + w;
	info.src = ((uintptr)src >> 2) + (u>>16);
	info.accum = u<<16;
	info.fracinc = dudx << 16;
	info.intinc = (sint32)dudx >> 16;
	info.count = -(sint32)w*4;

	vdasm_resize_point32(&info);
}

///////////////////////////////////////////////////////////////////////////////

void VDResamplerRowStageSeparableLinear8_phaseZeroStepHalf_ISSE::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *src = (const uint8 *)src0;

	nsVDPixmapSpanUtils::horiz_expand2x_coaligned_ISSE(dst, src, w);
}

///////////////////////////////////////////////////////////////////////////////

extern "C" void vdasm_resize_point32_MMX(const ScaleInfo *);
extern "C" void vdasm_resize_interp_row_run_MMX(void *dst, const void *src, uint32 width, sint64 xaccum, sint64 x_inc);
extern "C" void vdasm_resize_interp_col_run_MMX(void *dst, const void *src1, const void *src2, uint32 width, uint32 yaccum);
extern "C" void vdasm_resize_ccint_row_MMX(void *dst, const void *src, uint32 count, uint32 xaccum, sint32 xinc, const void *tbl);
extern "C" void vdasm_resize_ccint_col_MMX(void *dst, const void *src1, const void *src2, const void *src3, const void *src4, uint32 count, const void *tbl);
extern "C" long vdasm_resize_table_col_MMX(uint32 *out, const uint32 *const*in_table, const int *filter, int filter_width, uint32 w, long frac);
extern "C" long vdasm_resize_table_row_MMX(uint32 *out, const uint32 *in, const int *filter, int filter_width, uint32 w, long accum, long frac);

int VDResamplerSeparablePointRowStageMMX::GetWindowSize() const {return 1;}
void VDResamplerSeparablePointRowStageMMX::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	ScaleInfo info;

	info.dst = (uint32 *)dst + w;
	info.src = ((uintptr)src >> 2) + (u>>16);
	info.accum = u<<16;
	info.fracinc = dudx << 16;
	info.intinc = (sint32)dudx >> 16;
	info.count = -(sint32)w*4;

	vdasm_resize_point32_MMX(&info);
}

int VDResamplerSeparableLinearRowStageMMX::GetWindowSize() const {return 2;}
void VDResamplerSeparableLinearRowStageMMX::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_interp_row_run_MMX(dst0, src0, w, (sint64)u << 16, (sint64)dudx << 16);
}

int VDResamplerSeparableLinearColStageMMX::GetWindowSize() const {return 2;}
void VDResamplerSeparableLinearColStageMMX::Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase) {
	vdasm_resize_interp_col_run_MMX(dst0, srcarray[0], srcarray[1], w, phase);
}

VDResamplerSeparableCubicRowStageMMX::VDResamplerSeparableCubicRowStageMMX(double A)
	: mFilterBank(1024)
{
	sint32 *p = mFilterBank.data();
	VDResamplerGenerateTable(p, VDResamplerCubicFilter(1.0, A));
	VDResamplerSwizzleTable(p, 512);
}

int VDResamplerSeparableCubicRowStageMMX::GetWindowSize() const {return 4;}
void VDResamplerSeparableCubicRowStageMMX::Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_ccint_row_MMX(dst0, src0, w, u, dudx, mFilterBank.data());
}

VDResamplerSeparableCubicColStageMMX::VDResamplerSeparableCubicColStageMMX(double A)
	: mFilterBank(1024)
{
	sint32 *p = mFilterBank.data();
	VDResamplerGenerateTable(p, VDResamplerCubicFilter(1.0, A));
	VDResamplerSwizzleTable(p, 512);
}

int VDResamplerSeparableCubicColStageMMX::GetWindowSize() const {return 4;}
void VDResamplerSeparableCubicColStageMMX::Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase) {
	vdasm_resize_ccint_col_MMX(dst0, srcarray[0], srcarray[1], srcarray[2], srcarray[3], w, mFilterBank.data() + ((phase>>6)&0x3fc));
}

VDResamplerSeparableTableRowStage8MMX::VDResamplerSeparableTableRowStage8MMX(const IVDResamplerFilter& filter)
	: VDResamplerRowStageSeparableTable32(filter)
	, mLastSrcWidth(0)
	, mLastDstWidth(0)
	, mLastU(0)
	, mLastDUDX(0)
{
	mAlignedKernelWidth = (GetWindowSize() + 6) & ~3;
	mAlignedKernelSize = mAlignedKernelWidth + 4;
}

void VDResamplerSeparableTableRowStage8MMX::Init(const VDResamplerAxis& axis, uint32 srcw) {
	uint32 w = axis.dx_preclip + axis.dx_active + axis.dx_postclip + axis.dx_dualclip;

	if (mLastSrcWidth != srcw || mLastDstWidth != w || mLastU != axis.u || mLastDUDX != axis.dudx) {
		mLastSrcWidth	= srcw;
		mLastDstWidth	= w;
		mLastU			= axis.u;
		mLastDUDX		= axis.dudx;

		RedoRowFilters(axis, w, srcw);
	}
}

void VDResamplerSeparableTableRowStage8MMX::RedoRowFilters(const VDResamplerAxis& axis, uint32 w, uint32 srcw) {
	int kstride = mFilterBank.size() >> 8;
	int ksize = mAlignedKernelWidth;
	int kesize = mAlignedKernelSize;

	mRowKernels.clear();
	mRowKernelSize = w * kesize;

	mRowKernels.resize(mRowKernelSize * 4, 0);

	for(int byteOffset = 0; byteOffset < 4; ++byteOffset) {
		sint16 *dst = mRowKernels.data() + mRowKernelSize * byteOffset;
		int ksizeThisOffset = std::min<int>(ksize, (byteOffset + srcw + 3) & ~3);

		mKernelSizeByOffset[byteOffset] = ksizeThisOffset;

		sint32 u = axis.u;
		sint32 uoffmin = -byteOffset;
		sint32 uoffmax = ((srcw + byteOffset + 3) & ~3) - byteOffset - ksizeThisOffset;
		for(uint32 i=0; i<w; ++i) {
			sint32 uoffset = u >> 16;
			sint32 uoffset2 = ((uoffset + byteOffset) & ~3) - byteOffset;

			if (uoffset2 < uoffmin)
				uoffset2 = uoffmin;

			if (uoffset2 > uoffmax)
				uoffset2 = uoffmax;

			VDASSERT(uoffset2 + ksizeThisOffset <= (((sint32)srcw + byteOffset + 3) & ~3));

			*(sint32 *)dst = uoffset2;
			dst += 2;
			*dst++ = 0;
			*dst++ = 0;

			uint32 phase = (u >> 8) & 255;
			const sint32 *src = &mFilterBank[kstride * phase];

			sint32 start = 0;
			sint32 end = kstride;

			int dstoffset = uoffset - uoffset2;

			// check for filter kernel overlapping left source boundary
			if (uoffset < 0)
				start = -uoffset;

			// check for filter kernel overlapping right source boundary
			if (uoffset + end > (sint32)srcw)
				end = srcw - uoffset;

			VDASSERT(dstoffset + start >= 0);
			VDASSERT(dstoffset + end <= ksizeThisOffset);

			sint16 *dst2 = dst + dstoffset;
			dst += ksizeThisOffset;

			for(int j=start; j<end; ++j)
				dst2[j] = src[j];

			if (start > 0)
				dst2[start] = std::accumulate(src, src+start, dst2[start]);

			if (end < kstride)
				dst2[end - 1] = std::accumulate(src+end, src+kstride, dst2[end - 1]);

			u += axis.dudx;
		}
	}

	// swizzle rows where optimization is possible
	vdfastvector<sint16> temp;

	int quads = w >> 2;
	int quadRemainder = w & 3;

	for(int byteOffset = 0; byteOffset < 4; ++byteOffset) {
		int ksizeThisOffset = mKernelSizeByOffset[byteOffset];
		int kpairs = ksizeThisOffset >> 2;

		if (ksizeThisOffset < 8 || ksizeThisOffset > 12) {
			mbQuadOptimizationEnabled[byteOffset] = false;
		} else {
			ptrdiff_t unswizzledStride = (ksizeThisOffset >> 1) + 2;

			mbQuadOptimizationEnabled[byteOffset] = true;
			mTailOffset[byteOffset] = quads * (8 + ksizeThisOffset*4);

			uint32 *dst = (uint32 *)&mRowKernels[mRowKernelSize * byteOffset];
			temp.resize(mRowKernelSize);
			memcpy(temp.data(), dst, mRowKernelSize*2);

			const uint32 *src0 = (const uint32 *)temp.data();
			const uint32 *src1 = src0 + unswizzledStride;
			const uint32 *src2 = src1 + unswizzledStride;
			const uint32 *src3 = src2 + unswizzledStride;
			ptrdiff_t srcskip = unswizzledStride * 3;

			for(int q = 0; q < quads; ++q) {
				dst[0] = src0[0];
				dst[1] = src1[0];
				dst[2] = src2[0];
				dst[3] = src3[0];
				src0 += 2;
				src1 += 2;
				src2 += 2;
				src3 += 2;
				dst += 4;

				for(int p = 0; p < kpairs; ++p) {
					dst[0] = src0[0];
					dst[1] = src0[1];
					dst[2] = src1[0];
					dst[3] = src1[1];
					dst[4] = src2[0];
					dst[5] = src2[1];
					dst[6] = src3[0];
					dst[7] = src3[1];
					dst += 8;
					src0 += 2;
					src1 += 2;
					src2 += 2;
					src3 += 2;
				}

				src0 += srcskip;
				src1 += srcskip;
				src2 += srcskip;
				src3 += srcskip;
			}

			memcpy(dst, src0, unswizzledStride * 4 * quadRemainder);

			VDASSERT(dst + unswizzledStride * quadRemainder <= (void *)(mRowKernels.data() + (mRowKernelSize * (byteOffset + 1))));
		}
	}
}

void __declspec(naked) vdasm_resize_table_row_8_k8_4x_MMX(void *dst, const void *src, uint32 width, const void *kernel) {
	static const __declspec(align(8)) __int64 kRound = 0x0000000000002000;
	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm7, mm7
		movq		mm6, kRound

		mov			ebp, [esp +  4 + 16]		;ebp = dst
		mov			esi, [esp + 12 + 16]		;esi = width
		mov			edi, [esp + 16 + 16]		;edi = kernel
yloop:
		;eax = temp
		;ebx = temp
		;ecx = temp
		;edx = temp
		;esi = horiz counter
		;edi = filter list
		;ebp = destination

		mov			eax, [edi+0]
		mov			ebx, [edi+4]
		mov			ecx, [edi+8]
		mov			edx, [esp+8+16]
		add			eax, edx
		add			ebx, edx
		add			ecx, edx
		add			edx, [edi+12]

		movd		mm0, [eax]
		punpcklbw	mm0, mm7

		pmaddwd		mm0, [edi+16]
		movd		mm1, [ebx]
		punpcklbw	mm1, mm7

		pmaddwd		mm1, [edi+24]
		movd		mm2, [ecx]
		punpcklbw	mm2, mm7

		pmaddwd		mm2, [edi+32]
		movd		mm3, [edx]
		punpcklbw	mm3, mm7

		pmaddwd		mm3, [edi+40]
		movd		mm4, [eax+4]
		paddd		mm0, mm6

		movd		mm5, [ebx+4]
		punpcklbw	mm4, mm7
		paddd		mm1, mm6

		pmaddwd		mm4, [edi+48]
		punpcklbw	mm5, mm7
		paddd		mm2, mm6

		pmaddwd		mm5, [edi+56]
		paddd		mm3, mm6
		paddd		mm0, mm4

		paddd		mm1, mm5
		movd		mm4, [ecx+4]
		punpcklbw	mm4, mm7

		pmaddwd		mm4, [edi+64]
		movd		mm5, [edx+4]
		punpcklbw	mm5, mm7

		pmaddwd		mm5, [edi+72]
		paddd		mm2, mm4
		paddd		mm3, mm5

		movq		mm4, mm0
		punpckldq	mm0, mm1
		movq		mm5, mm2
		punpckldq	mm2, mm3
		punpckhdq	mm4, mm1
		punpckhdq	mm5, mm3
		paddd		mm0, mm4
		paddd		mm2, mm5
		psrad		mm0, 14
		psrad		mm2, 14

		packssdw	mm0, mm2
		packuswb	mm0, mm0

		add			edi, 80

		movd		[ebp], mm0
		add			ebp, 4
		sub			esi, 1
		jne			yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
	}
}

void __declspec(naked) vdasm_resize_table_row_8_k12_4x_MMX(void *dst, const void *src, uint32 width, const void *kernel) {
	static const __declspec(align(8)) __int64 kRound = 0x0000200000002000;
	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm7, mm7
		movq		mm6, kRound

		mov			ebp, [esp +  4 + 16]		;ebp = dst
		mov			esi, [esp + 12 + 16]		;esi = width
		mov			edi, [esp + 16 + 16]		;edi = kernel
yloop:
		;eax = temp
		;ebx = temp
		;ecx = temp
		;edx = temp
		;esi = horiz counter
		;edi = filter list
		;ebp = destination

		mov			eax, [edi+0]
		mov			ebx, [edi+4]
		mov			ecx, [edi+8]
		mov			edx, [esp+8+16]
		add			eax, edx
		add			ebx, edx
		add			ecx, edx
		add			edx, [edi+12]

		movd		mm0, [eax]
		punpcklbw	mm0, mm7

		pmaddwd		mm0, [edi+16]
		movd		mm1, [ebx]
		punpcklbw	mm1, mm7

		pmaddwd		mm1, [edi+24]
		movd		mm2, [ecx]
		punpcklbw	mm2, mm7

		pmaddwd		mm2, [edi+32]
		movd		mm3, [edx]
		punpcklbw	mm3, mm7

		pmaddwd		mm3, [edi+40]
		movd		mm4, [eax+4]
		punpcklbw	mm4, mm7

		pmaddwd		mm4, [edi+48]
		movd		mm5, [ebx+4]
		punpcklbw	mm5, mm7

		pmaddwd		mm5, [edi+56]
		paddd		mm0, mm4
		paddd		mm1, mm5

		movd		mm4, [ecx+4]
		punpcklbw	mm4, mm7
		movd		mm5, [edx+4]

		pmaddwd		mm4, [edi+64]
		punpcklbw	mm5, mm7
		paddd		mm2, mm4

		pmaddwd		mm5, [edi+72]
		movd		mm4, [eax+8]
		punpcklbw	mm4, mm7

		paddd		mm3, mm5
		movd		mm5, [ebx+8]
		punpcklbw	mm5, mm7

		pmaddwd		mm4, [edi+80]
		paddd		mm0, mm4
		movd		mm4, [ecx+8]

		pmaddwd		mm5, [edi+88]
		paddd		mm1, mm5
		punpcklbw	mm4, mm7

		pmaddwd		mm4, [edi+96]
		movd		mm5, [edx+8]
		punpcklbw	mm5, mm7

		pmaddwd		mm5, [edi+104]
		paddd		mm2, mm4
		paddd		mm3, mm5

		movq		mm4, mm0
		punpckldq	mm0, mm1
		movq		mm5, mm2
		punpckldq	mm2, mm3
		punpckhdq	mm4, mm1
		punpckhdq	mm5, mm3
		paddd		mm0, mm4
		paddd		mm2, mm5
		paddd		mm0, mm6
		paddd		mm2, mm6
		psrad		mm0, 14
		psrad		mm2, 14

		packssdw	mm0, mm2
		packuswb	mm0, mm0

		add			edi, 112

		movd		[ebp], mm0
		add			ebp, 4
		sub			esi, 1
		jne			yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
	}
}

void __declspec(naked) vdasm_resize_table_row_8_MMX(void *dst, const void *src, uint32 width, const void *kernel, uint32 kwidth) {
	static const __declspec(align(8)) __int64 kRound = 0x0000000000002000;
	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm7, mm7
		movq		mm6, kRound

		mov			edi, [esp +  4 + 16]		;edi = dst
		mov			ebx, [esp +  8 + 16]		;ebx = src
		mov			ebp, [esp + 12 + 16]		;ebp = width
		mov			edx, [esp + 16 + 16]		;edx = kernel
yloop:
		;eax = temp
		;ebx = source base address
		;ecx = (temp) source
		;edx = filter list
		;esi = (temp) kernel width
		;edi = destination
		;ebp = horiz counter

		mov			eax, [edx]
		add			edx, 8
		lea			ecx, [ebx + eax]
		mov			esi, [esp + 20 + 16]		;esi = kernel width

		movq		mm2, mm6
xloop:
		movd		mm0, [ecx]
		punpcklbw	mm0, mm7
		add			ecx, 4
		pmaddwd		mm0, [edx]
		paddd		mm2, mm0
		add			edx, 8
		sub			esi, 4
		jne			xloop

		punpckldq	mm0, mm2
		paddd		mm0, mm2
		psrad		mm0, 14
		psrlq		mm0, 32
		packssdw	mm0, mm0
		packuswb	mm0, mm0
		movd		eax, mm0
		mov			[edi], al
		add			edi, 1
		sub			ebp, 1
		jne			yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
	}
}

void VDResamplerSeparableTableRowStage8MMX::Process(void *dst, const void *src, uint32 w) {
	int byteOffset = (int)(ptrdiff_t)src & 3;
	const sint16 *ksrc = mRowKernels.data() + (mRowKernelSize * byteOffset);
#if 0
	int kwidth = mAlignedKernelWidth;
	uint8 *dst2 = (uint8 *)dst;

	do {
		int offset = ksrc[0];
		ksrc += 4;
		
		const uint8 *src2 = (const uint8 *)src + offset;
		sint32 accum = 0x8000;
		for(int i=0; i<kwidth; ++i) {
			accum += (sint32)src2[i] * (*ksrc++);
		}

		accum >>= 14;

		accum &= ~(accum >> 31);
		accum |= (255 - accum) >> 31;

		*dst2++ = (uint8)accum;

	} while(--w);
#else
	int ksize = mKernelSizeByOffset[byteOffset];
	if (mbQuadOptimizationEnabled[byteOffset]) {
		if (w >= 4) {
			if (ksize == 12) {
				vdasm_resize_table_row_8_k12_4x_MMX(dst, src, w >> 2, ksrc);

#if 0
				int w4 = w >> 2;
				uint8 *dst2 = (uint8 *)dst;
				const uint8 *src2 = (const uint8 *)src;
				const sint16 *ksrc2 = ksrc;

				do {
					int off0 = ksrc2[0];
					int off1 = ksrc2[2];
					int off2 = ksrc2[4];
					int off3 = ksrc2[6];
					const uint8 *d0 = src2 + off0;
					const uint8 *d1 = src2 + off1;
					const uint8 *d2 = src2 + off2;
					const uint8 *d3 = src2 + off3;

					int acc0 = 0;
					int acc1 = 0;
					int acc2 = 0;
					int acc3 = 0;

					acc0 += d0[ 0]*ksrc2[   8]
						  + d0[ 1]*ksrc2[   9]
						  + d0[ 2]*ksrc2[  10]
						  + d0[ 3]*ksrc2[  11]
						  + d0[ 4]*ksrc2[  24]
						  + d0[ 5]*ksrc2[  25]
						  + d0[ 6]*ksrc2[  26]
						  + d0[ 7]*ksrc2[  27]
						  + d0[ 8]*ksrc2[  40]
						  + d0[ 9]*ksrc2[  41]
						  + d0[10]*ksrc2[  42]
						  + d0[11]*ksrc2[  43];

					acc0 = (acc0 + 0x2000) >> 14;
					if (acc0 < 0) acc0 = 0; else if (acc0 > 255) acc0 = 255;

					acc1 += d1[ 0]*ksrc2[  12]
						  + d1[ 1]*ksrc2[  13]
						  + d1[ 2]*ksrc2[  14]
						  + d1[ 3]*ksrc2[  15]
						  + d1[ 4]*ksrc2[  28]
						  + d1[ 5]*ksrc2[  29]
						  + d1[ 6]*ksrc2[  30]
						  + d1[ 7]*ksrc2[  31]
						  + d1[ 8]*ksrc2[  44]
						  + d1[ 9]*ksrc2[  45]
						  + d1[10]*ksrc2[  46]
						  + d1[11]*ksrc2[  47];

					acc1 = (acc1 + 0x2000) >> 14;
					if (acc1 < 0) acc1 = 0; else if (acc1 > 255) acc1 = 255;

					acc2 += d2[ 0]*ksrc2[  16]
						  + d2[ 1]*ksrc2[  17]
						  + d2[ 2]*ksrc2[  18]
						  + d2[ 3]*ksrc2[  19]
						  + d2[ 4]*ksrc2[  32]
						  + d2[ 5]*ksrc2[  33]
						  + d2[ 6]*ksrc2[  34]
						  + d2[ 7]*ksrc2[  35]
						  + d2[ 8]*ksrc2[  48]
						  + d2[ 9]*ksrc2[  49]
						  + d2[10]*ksrc2[  50]
						  + d2[11]*ksrc2[  51];

					acc2 = (acc2 + 0x2000) >> 14;
					if (acc2 < 0) acc2 = 0; else if (acc2 > 255) acc2 = 255;

					acc3 += d3[ 0]*ksrc2[  20]
						  + d3[ 1]*ksrc2[  21]
						  + d3[ 2]*ksrc2[  22]
						  + d3[ 3]*ksrc2[  23]
						  + d3[ 4]*ksrc2[  36]
						  + d3[ 5]*ksrc2[  37]
						  + d3[ 6]*ksrc2[  38]
						  + d3[ 7]*ksrc2[  39]
						  + d3[ 8]*ksrc2[  52]
						  + d3[ 9]*ksrc2[  53]
						  + d3[10]*ksrc2[  54]
						  + d3[11]*ksrc2[  55];

					acc3 = (acc3 + 0x2000) >> 14;
					if (acc3 < 0) acc3 = 0; else if (acc3 > 255) acc3 = 255;

					ksrc2 += 56;

					dst2[0] = (uint8)acc0;
					dst2[1] = (uint8)acc1;
					dst2[2] = (uint8)acc2;
					dst2[3] = (uint8)acc3;
					dst2 += 4;
				} while(--w4);
#endif
			} else
				vdasm_resize_table_row_8_k8_4x_MMX(dst, src, w >> 2, ksrc);
		}

		if (w & 3)
			vdasm_resize_table_row_8_MMX((char *)dst + (w & ~3), src, w & 3, ksrc + mTailOffset[byteOffset], ksize);
	} else if (w) {
		vdasm_resize_table_row_8_MMX(dst, src, w, ksrc, ksize);
	}
#endif
}

void VDResamplerSeparableTableRowStage8MMX::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_table_row_MMX((uint32 *)dst, (const uint32 *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, u, dudx);
}

VDResamplerSeparableTableRowStageMMX::VDResamplerSeparableTableRowStageMMX(const IVDResamplerFilter& filter)
	: VDResamplerRowStageSeparableTable32(filter)
{
	VDResamplerSwizzleTable(mFilterBank.data(), (unsigned)mFilterBank.size() >> 1);
}

void VDResamplerSeparableTableRowStageMMX::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_table_row_MMX((uint32 *)dst, (const uint32 *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, u, dudx);
}

///////////////////////////////////////////////////////////////////////////

VDResamplerSeparableTableColStage8MMX::VDResamplerSeparableTableColStage8MMX(const IVDResamplerFilter& filter)
	: VDResamplerColStageSeparableTable8(filter)
{
	VDResamplerSwizzleTable(mFilterBank.data(), (unsigned)mFilterBank.size() >> 1);
}

void __declspec(naked) vdasm_resize_table_col_8_k2_MMX(void *dst, const void *const *srcs, uint32 width, const void *kernel) {
	static const __declspec(align(8)) __int64 kRound = 0x0000200000002000;

	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm7, mm7
		movq		mm6, kRound

		mov			esi, [esp +  4 + 16]		;esi = dst
		mov			edi, [esp + 16 + 16]		;edi = kernel
		mov			ebp, [esp + 12 + 16]		;ebp = width

		movq		mm5, [edi]

		mov			edx, [esp +  8 + 16]		;ebx = srcs
		mov			eax, [edx+0]
		mov			ebx, [edx+4]
		add			eax, ebp
		add			ebx, ebp
		neg			ebp
yloop:
		;eax = row0
		;ebx = row1
		;ecx =
		;edx =
		;edi = kernel
		;esi = dest
		;ebp = width counter

		movd		mm0, [eax+ebp]
		punpcklbw	mm0, mm7
		movd		mm2, [ebx+ebp]
		punpcklbw	mm2, mm7
		movq		mm1, mm0
		punpcklwd	mm0, mm2
		punpckhwd	mm1, mm2
		pmaddwd		mm0, mm5
		pmaddwd		mm1, mm5

		paddd		mm0, mm6
		paddd		mm1, mm6

		psrad		mm0, 14
		psrad		mm1, 14
		packssdw	mm0, mm1
		packuswb	mm0, mm0
		movd		[esi], mm0
		add			esi, 4
		add			ebp, 4
		jne			yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
	}
}

void __declspec(naked) vdasm_resize_table_col_8_k4_MMX(void *dst, const void *const *srcs, uint32 width, const void *kernel) {
	static const __declspec(align(8)) __int64 kRound = 0x0000200000002000;

	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm7, mm7
		movq		mm6, kRound

		mov			esi, [esp +  4 + 16]		;esi = dst
		mov			edi, [esp + 16 + 16]		;edi = kernel
		xor			ebp, ebp

		mov			edx, [esp +  8 + 16]		;ebx = srcs
		mov			eax, [edx+0]
		mov			ebx, [edx+4]
		mov			ecx, [edx+8]
		mov			edx, [edx+12]
yloop:
		;eax = row0
		;ebx = row1
		;ecx = row2
		;edx = row3
		;edi = kernel
		;esi = dest
		;ebp = width counter

		movd		mm0, [eax+ebp]
		punpcklbw	mm0, mm7
		movd		mm2, [ebx+ebp]
		punpcklbw	mm2, mm7
		movq		mm1, mm0
		punpcklwd	mm0, mm2
		movq		mm5, [edi]
		punpckhwd	mm1, mm2
		pmaddwd		mm0, mm5
		pmaddwd		mm1, mm5

		paddd		mm0, mm6
		paddd		mm1, mm6

		movd		mm3, [ecx+ebp]
		punpcklbw	mm3, mm7
		movd		mm2, [edx+ebp]
		punpcklbw	mm2, mm7
		movq		mm4, mm3
		punpcklwd	mm3, mm2
		movq		mm5, [edi+8]
		punpckhwd	mm4, mm2
		pmaddwd		mm3, mm5
		pmaddwd		mm4, mm5

		paddd		mm0, mm3
		paddd		mm1, mm4

		psrad		mm0, 14
		psrad		mm1, 14
		packssdw	mm0, mm1
		packuswb	mm0, mm0
		add			ebp, 4
		movd		[esi], mm0
		add			esi, 4
		cmp			ebp, [esp + 12 + 16]
		jb			yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
	}
}

void __declspec(naked) vdasm_resize_table_col_8_MMX(void *dst, const void *const *srcs, uint32 width, const void *kernel, uint32 kwidth) {
	static const __declspec(align(8)) __int64 kRound = 0x0000200000002000;

	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm7, mm7
		movq		mm6, kRound

		mov			edi, [esp +  4 + 16]		;edi = dst
		xor			ebp, ebp
yloop:
		mov			edx, [esp + 16 + 16]		;edx = kernel
		mov			ebx, [esp +  8 + 16]		;ebx = srcs
		mov			esi, [esp + 20 + 16]		;esi = kwidth
		movq		mm3, mm6
		movq		mm4, mm6
xloop:
		mov			ecx, [ebx]
		movd		mm0, [ecx+ebp]
		punpcklbw	mm0, mm7
		mov			ecx, [ebx+4]
		movd		mm2, [ecx+ebp]
		punpcklbw	mm2, mm7
		movq		mm1, mm0
		punpcklwd	mm0, mm2
		punpckhwd	mm1, mm2
		movq		mm5, [edx]
		pmaddwd		mm0, mm5
		pmaddwd		mm1, mm5

		paddd		mm3, mm0
		paddd		mm4, mm1
		add			ebx, 8
		add			edx, 8
		sub			esi, 2
		jne			xloop

		psrad		mm3, 14
		psrad		mm4, 14
		packssdw	mm3, mm4
		packuswb	mm3, mm3
		movd		[edi], mm3
		add			edi, 4
		add			ebp, 4
		cmp			ebp, [esp + 12 + 16]
		jb			yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
	}
}

void VDResamplerSeparableTableColStage8MMX::Process(void *dst0, const void *const *src0, uint32 w, sint32 phase) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *const *src = (const uint8 *const *)src0;
	const unsigned ksize = (unsigned)mFilterBank.size() >> 8;
	const sint16 *filter = (const sint16 *)&mFilterBank[((phase>>8)&0xff) * ksize];

	int w4 = w & ~3;

	if (w4) {
		switch(ksize) {
			case 2:
				vdasm_resize_table_col_8_k2_MMX(dst, (const void *const *)src, w4, filter);
				break;

			case 4:
				vdasm_resize_table_col_8_k4_MMX(dst, (const void *const *)src, w4, filter);
				break;

			default:
				vdasm_resize_table_col_8_MMX(dst, (const void *const *)src, w4, filter, ksize);
				break;
		}
	}

	for(uint32 i=w4; i<w; ++i) {
		int b = 0x2000;
		const sint16 *filter2 = filter;
		const uint8 *const *src2 = src;

		for(unsigned j = ksize; j; j -= 2) {
			sint32 p0 = (*src2++)[i];
			sint32 p1 = (*src2++)[i];
			sint32 coeff0 = filter2[0];
			sint32 coeff1 = filter2[1];
			filter2 += 4;

			b += p0*coeff0;
			b += p1*coeff1;
		}

		b >>= 14;

		if ((uint32)b >= 0x00000100)
			b = ~b >> 31;

		dst[i] = (uint8)b;
	}
}

///////////////////////////////////////////////////////////////////////////

VDResamplerSeparableTableColStageMMX::VDResamplerSeparableTableColStageMMX(const IVDResamplerFilter& filter)
	: VDResamplerColStageSeparableTable32(filter)
{
	VDResamplerSwizzleTable(mFilterBank.data(), (unsigned)mFilterBank.size() >> 1);
}

void VDResamplerSeparableTableColStageMMX::Process(void *dst, const void *const *src, uint32 w, sint32 phase) {
	vdasm_resize_table_col_MMX((uint32*)dst, (const uint32 *const *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, (phase >> 8) & 0xff);
}

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (SSE2, x86)
//
///////////////////////////////////////////////////////////////////////////

extern "C" long vdasm_resize_table_col_SSE2(uint32 *out, const uint32 *const*in_table, const int *filter, int filter_width, uint32 w, long frac);
extern "C" long vdasm_resize_table_row_SSE2(uint32 *out, const uint32 *in, const int *filter, int filter_width, uint32 w, long accum, long frac);
extern "C" void vdasm_resize_ccint_col_SSE2(void *dst, const void *src1, const void *src2, const void *src3, const void *src4, uint32 count, const void *tbl);

VDResamplerSeparableCubicColStageSSE2::VDResamplerSeparableCubicColStageSSE2(double A)
	: VDResamplerSeparableCubicColStageMMX(A)
{
}

void VDResamplerSeparableCubicColStageSSE2::Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase) {
	vdasm_resize_ccint_col_SSE2(dst0, srcarray[0], srcarray[1], srcarray[2], srcarray[3], w, mFilterBank.data() + ((phase>>6)&0x3fc));
}

VDResamplerSeparableTableRowStageSSE2::VDResamplerSeparableTableRowStageSSE2(const IVDResamplerFilter& filter)
	: VDResamplerSeparableTableRowStageMMX(filter)
{
}

void VDResamplerSeparableTableRowStageSSE2::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_table_row_MMX((uint32 *)dst, (const uint32 *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, u, dudx);
}

VDResamplerSeparableTableColStageSSE2::VDResamplerSeparableTableColStageSSE2(const IVDResamplerFilter& filter)
	: VDResamplerSeparableTableColStageMMX(filter)
{
}

void VDResamplerSeparableTableColStageSSE2::Process(void *dst, const void *const *src, uint32 w, sint32 phase) {
	vdasm_resize_table_col_SSE2((uint32*)dst, (const uint32 *const *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, (phase >> 8) & 0xff);
}

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (SSE4.1, x86)
//
///////////////////////////////////////////////////////////////////////////

VDResamplerSeparableTableRowStage8SSE41::VDResamplerSeparableTableRowStage8SSE41(const IVDResamplerFilter& filter)
	: VDResamplerRowStageSeparableTable32(filter)
	, mLastSrcWidth(0)
	, mLastDstWidth(0)
	, mLastU(0)
	, mLastDUDX(0)
{
	mAlignedKernelWidth = (GetWindowSize() + 15) & ~7;
	mAlignedKernelSize = mAlignedKernelWidth + 16;
}

void VDResamplerSeparableTableRowStage8SSE41::Init(const VDResamplerAxis& axis, uint32 srcw) {
	uint32 w = axis.dx_preclip + axis.dx_active + axis.dx_postclip + axis.dx_dualclip;

	if (mLastSrcWidth != srcw || mLastDstWidth != w || mLastU != axis.u || mLastDUDX != axis.dudx) {
		mLastSrcWidth	= srcw;
		mLastDstWidth	= w;
		mLastU			= axis.u;
		mLastDUDX		= axis.dudx;

		RedoRowFilters(axis, w, srcw);
	}
}

void VDResamplerSeparableTableRowStage8SSE41::RedoRowFilters(const VDResamplerAxis& axis, uint32 w, uint32 srcw) {
	int kstride = mFilterBank.size() >> 8;
	int ksize = mAlignedKernelWidth;
	int kesize = mAlignedKernelSize;

	mRowKernels.clear();
	mRowKernelSize = w * kesize;

	mRowKernels.resize(mRowKernelSize * 8, 0);

	for(int byteOffset = 0; byteOffset < 8; ++byteOffset) {
		sint16 *dst = mRowKernels.data() + mRowKernelSize * byteOffset;
		int ksizeThisOffset = std::min<int>(ksize, (byteOffset + srcw + 7) & ~7);

		mKernelSizeByOffset[byteOffset] = ksizeThisOffset;

		sint32 u = axis.u;
		sint32 uoffmin = -byteOffset;
		sint32 uoffmax = ((srcw + byteOffset + 7) & ~7) - byteOffset - ksizeThisOffset;
		for(uint32 i=0; i<w; ++i) {
			sint32 uoffset = u >> 16;
			sint32 uoffset2 = ((uoffset + byteOffset) & ~7) - byteOffset;

			if (uoffset2 < uoffmin)
				uoffset2 = uoffmin;

			if (uoffset2 > uoffmax)
				uoffset2 = uoffmax;

			*(sint32 *)dst = uoffset2;
			dst += 2;
			*dst++ = 0;
			*dst++ = 0;
			*dst++ = 0;
			*dst++ = 0;
			*dst++ = 0;
			*dst++ = 0;

			uint32 phase = (u >> 8) & 255;
			const sint32 *src = &mFilterBank[kstride * phase];

			sint32 start = 0;
			sint32 end = kstride;

			int dstoffset = uoffset - uoffset2;

			// check for filter kernel overlapping left source boundary
			if (uoffset < 0)
				start = -uoffset;

			// check for filter kernel overlapping right source boundary
			if (uoffset + end > (sint32)srcw)
				end = srcw - uoffset;

			VDASSERT(dstoffset + start >= 0);
			VDASSERT(dstoffset + end <= ksizeThisOffset);

			sint16 *dst2 = dst + dstoffset;
			dst += ksizeThisOffset;

			for(int j=start; j<end; ++j)
				dst2[j] = src[j];

			if (start > 0)
				dst2[start] = std::accumulate(src, src+start, dst2[start]);

			if (end < kstride)
				dst2[end - 1] = std::accumulate(src+end, src+kstride, dst2[end - 1]);

			u += axis.dudx;
		}
	}

	// swizzle rows where optimization is possible
	vdfastvector<sint16> temp;

	int quads = w >> 2;
	int quadRemainder = w & 3;

	for(int byteOffset = 0; byteOffset < 8; ++byteOffset) {
		int ksizeThisOffset = mKernelSizeByOffset[byteOffset];
		int kpairs = ksizeThisOffset >> 3;

		if (ksizeThisOffset < 8 || ksizeThisOffset > 16) {
			mbQuadOptimizationEnabled[byteOffset] = false;
		} else {
			ptrdiff_t unswizzledStride = (ksizeThisOffset >> 1) + 4;

			mbQuadOptimizationEnabled[byteOffset] = true;
			mTailOffset[byteOffset] = quads * (8 + ksizeThisOffset*4);

			uint32 *dst = (uint32 *)&mRowKernels[mRowKernelSize * byteOffset];
			temp.resize(mRowKernelSize);
			memcpy(temp.data(), dst, mRowKernelSize*2);

			const uint32 *src0 = (const uint32 *)temp.data();
			const uint32 *src1 = src0 + unswizzledStride;
			const uint32 *src2 = src1 + unswizzledStride;
			const uint32 *src3 = src2 + unswizzledStride;
			ptrdiff_t srcskip = unswizzledStride * 3;

			for(int q = 0; q < quads; ++q) {
				dst[0] = src0[0];
				dst[1] = src1[0];
				dst[2] = src2[0];
				dst[3] = src3[0];
				src0 += 4;
				src1 += 4;
				src2 += 4;
				src3 += 4;
				dst += 4;

				for(int p = 0; p < kpairs; ++p) {
					dst[ 0] = src0[0];
					dst[ 1] = src0[1];
					dst[ 2] = src0[2];
					dst[ 3] = src0[3];
					dst[ 4] = src1[0];
					dst[ 5] = src1[1];
					dst[ 6] = src1[2];
					dst[ 7] = src1[3];
					dst[ 8] = src2[0];
					dst[ 9] = src2[1];
					dst[10] = src2[2];
					dst[11] = src2[3];
					dst[12] = src3[0];
					dst[13] = src3[1];
					dst[14] = src3[2];
					dst[15] = src3[3];
					dst += 16;
					src0 += 4;
					src1 += 4;
					src2 += 4;
					src3 += 4;
				}

				src0 += srcskip;
				src1 += srcskip;
				src2 += srcskip;
				src3 += srcskip;
			}

			memcpy(dst, src0, unswizzledStride * 4 * quadRemainder);
		}
	}
}

void VDResamplerSeparableTableRowStage8SSE41::Process(void *dst, const void *src, uint32 w) {
	int byteOffset = (int)(ptrdiff_t)src & 7;
	const sint16 *ksrc = mRowKernels.data() + (mRowKernelSize * byteOffset);

	int ksize = mKernelSizeByOffset[byteOffset];
	if (mbQuadOptimizationEnabled[byteOffset]) {
		if (w >= 4) {
			if (ksize == 16)
				vdasm_resize_table_row_8_k16_4x_SSE41(dst, src, w >> 2, ksrc);
			else
				vdasm_resize_table_row_8_k8_4x_SSE41(dst, src, w >> 2, ksrc);
		}

		if (w & 3)
			vdasm_resize_table_row_8_SSE41((char *)dst + (w & ~3), src, w & 3, ksrc + mTailOffset[byteOffset], ksize);
	} else if (w) {
		vdasm_resize_table_row_8_SSE41(dst, src, w, ksrc, ksize);
	}
}

void VDResamplerSeparableTableRowStage8SSE41::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_table_row_MMX((uint32 *)dst, (const uint32 *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, u, dudx);
}

///////////////////////////////////////////////////////////////////////////

VDResamplerSeparableTableColStage8SSE41::VDResamplerSeparableTableColStage8SSE41(const IVDResamplerFilter& filter)
	: VDResamplerColStageSeparableTable8(filter)
{
	VDResamplerSwizzleTable(mFilterBank.data(), (unsigned)mFilterBank.size() >> 1);
}

void VDResamplerSeparableTableColStage8SSE41::Process(void *dst0, const void *const *src0, uint32 w, sint32 phase) {
	uint8 *dst = (uint8 *)dst0;
	const uint8 *const *src = (const uint8 *const *)src0;
	const unsigned ksize = (unsigned)mFilterBank.size() >> 8;
	const sint16 *filter = (const sint16 *)&mFilterBank[((phase>>8)&0xff) * ksize];

	int w4 = w & ~3;

	if (w4) {
		switch(ksize) {
			case 2:
				vdasm_resize_table_col_8_k2_SSE41(dst, (const void *const *)src, w4, filter);
				break;

			case 4:
				vdasm_resize_table_col_8_k4_SSE41(dst, (const void *const *)src, w4, filter);
				break;

			default:
				vdasm_resize_table_col_8_MMX(dst, (const void *const *)src, w4, filter, ksize);
				break;
		}
	}

	for(uint32 i=w4; i<w; ++i) {
		int b = 0x2000;
		const sint16 *filter2 = filter;
		const uint8 *const *src2 = src;

		for(unsigned j = ksize; j; j -= 2) {
			sint32 p0 = (*src2++)[i];
			sint32 p1 = (*src2++)[i];
			sint32 coeff0 = filter2[0];
			sint32 coeff1 = filter2[1];
			filter2 += 4;

			b += p0*coeff0;
			b += p1*coeff1;
		}

		b >>= 14;

		if ((uint32)b >= 0x00000100)
			b = ~b >> 31;

		dst[i] = (uint8)b;
	}
}
