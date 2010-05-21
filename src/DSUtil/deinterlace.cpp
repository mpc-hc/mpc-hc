//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2001 Avery Lee
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

#include "stdafx.h"
#include <emmintrin.h>
#include <vd2/system/cpuaccel.h>

#define uint8	unsigned char
#define uint32	unsigned int
#define uint64	unsigned __int64

#ifdef _M_IX86
#define VD_CPU_X86
#endif

#ifdef _M_X64
#define VD_CPU_AMD64
#endif

///////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 4799)		// warning C4799: function has no EMMS instruction

///////////////////////////////////////////////////////////////////////////

#ifdef _M_IX86
static void __declspec(naked) asm_blend_row_clipped(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi,[esp+20]
		mov		esi,[esp+24]
		sub		edi,esi
		mov		ebp,[esp+28]
		mov		edx,[esp+32]

xloop:
		mov		ecx,[esi]
		mov		eax,0fefefefeh

		mov		ebx,[esi+edx]
		and		eax,ecx

		shr		eax,1
		and		ebx,0fefefefeh

		shr		ebx,1
		add		esi,4

		add		eax,ebx
		dec		ebp

		mov		[edi+esi-4],eax
		jnz		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) asm_blend_row(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi,[esp+20]
		mov		esi,[esp+24]
		sub		edi,esi
		mov		ebp,[esp+28]
		mov		edx,[esp+32]

xloop:
		mov		ecx,[esi]
		mov		eax,0fcfcfcfch

		mov		ebx,[esi+edx]
		and		eax,ecx

		shr		ebx,1
		mov		ecx,[esi+edx*2]

		shr		ecx,2
		and		ebx,07f7f7f7fh

		shr		eax,2
		and		ecx,03f3f3f3fh

		add		eax,ebx
		add		esi,4

		add		eax,ecx
		dec		ebp

		mov		[edi+esi-4],eax
		jnz		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) asm_blend_row_MMX(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
	static const __declspec(align(8)) __int64 mask0 = 0xfcfcfcfcfcfcfcfci64;
	static const __declspec(align(8)) __int64 mask1 = 0x7f7f7f7f7f7f7f7fi64;
	static const __declspec(align(8)) __int64 mask2 = 0x3f3f3f3f3f3f3f3fi64;
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi,[esp+20]
		mov		esi,[esp+24]
		sub		edi,esi
		mov		ebp,[esp+28]
		mov		edx,[esp+32]

		movq	mm5,mask0
		movq	mm6,mask1
		movq	mm7,mask2
		inc		ebp
		shr		ebp,1
xloop:
		movq	mm2,[esi]
		movq	mm0,mm5

		movq	mm1,[esi+edx]
		pand	mm0,mm2

		psrlq	mm1,1
		movq	mm2,[esi+edx*2]

		psrlq	mm2,2
		pand	mm1,mm6

		psrlq	mm0,2
		pand	mm2,mm7

		paddb	mm0,mm1
		add		esi,8

		paddb	mm0,mm2
		dec		ebp

		movq	[edi+esi-8],mm0
		jne		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) asm_blend_row_ISSE(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi,[esp+20]
		mov		esi,[esp+24]
		sub		edi,esi
		mov		ebp,[esp+28]
		mov		edx,[esp+32]

		inc		ebp
		shr		ebp,1
		pcmpeqb	mm7, mm7

		align	16
xloop:
		movq	mm0, [esi]
		movq	mm2, mm7
		pxor	mm0, mm7

		pxor	mm2, [esi+edx*2]
		pavgb	mm0, mm2
		pxor	mm0, mm7

		pavgb	mm0, [esi+edx]
		add		esi,8

		movq	[edi+esi-8],mm0
		dec		ebp
		jne		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}
#else
static void asm_blend_row_clipped(void *dst0, const void *src0, uint32 w, ptrdiff_t srcpitch) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;
	const uint32 *src2 = (const uint32 *)((const char *)src + srcpitch);

	do {
		const uint32 x = *src++;
		const uint32 y = *src2++;

		*dst++ = (x|y) - (((x^y)&0xfefefefe)>>1);
	} while(--w);
}

static void asm_blend_row(void *dst0, const void *src0, uint32 w, ptrdiff_t srcpitch) {
	uint32 *dst = (uint32 *)dst0;
	const uint32 *src = (const uint32 *)src0;
	const uint32 *src2 = (const uint32 *)((const char *)src + srcpitch);
	const uint32 *src3 = (const uint32 *)((const char *)src2 + srcpitch);

	do {
		const uint32 a = *src++;
		const uint32 b = *src2++;
		const uint32 c = *src3++;
		const uint32 hi = (a & 0xfcfcfc) + 2*(b & 0xfcfcfc) + (c & 0xfcfcfc);
		const uint32 lo = (a & 0x030303) + 2*(b & 0x030303) + (c & 0x030303) + 0x020202;

		*dst++ = (hi + (lo & 0x0c0c0c))>>2;
	} while(--w);
}
#endif

#if defined(VD_CPU_X86) || defined(VD_CPU_AMD64)
	static void asm_blend_row_SSE2(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
		__m128i zero = _mm_setzero_si128();
		__m128i inv = _mm_cmpeq_epi8(zero, zero);

		w = (w + 3) >> 2;

		const __m128i *src1 = (const __m128i *)src;
		const __m128i *src2 = (const __m128i *)((const char *)src + srcpitch);
		const __m128i *src3 = (const __m128i *)((const char *)src + srcpitch*2);
		__m128i *dstrow = (__m128i *)dst;
		do {
			__m128i a = *src1++;
			__m128i b = *src2++;
			__m128i c = *src3++;

			*dstrow++ = _mm_avg_epu8(_mm_xor_si128(_mm_avg_epu8(_mm_xor_si128(a, inv), _mm_xor_si128(c, inv)), inv), b);
		} while(--w);
	}

#endif

namespace {

	void Average_scalar(void *dst, ptrdiff_t dstPitch, const void *src1, const void *src2, ptrdiff_t srcPitch, uint32 w16, uint32 h) {
		uint32 w4 = w16 << 2;
		do {
			uint32 *dstv = (uint32 *)dst;
			uint32 *src1v = (uint32 *)src1;
			uint32 *src2v = (uint32 *)src2;

			for(uint32 i=0; i<w4; ++i) {
				uint32 a = src1v[i];
				uint32 b = src2v[i];

				dstv[i] = (a|b) - (((a^b) & 0xfefefefe) >> 1);
			}

			dst = (char *)dst + dstPitch;
			src1 = (char *)src1 + srcPitch;
			src2 = (char *)src2 + srcPitch;
		} while(--h);
	}

#if defined(VD_CPU_X86)
	void __declspec(naked) __cdecl Average_MMX(void *dst, ptrdiff_t dstPitch, const void *src1, const void *src2, ptrdiff_t srcPitch, uint32 w16, uint32 h) {
		static const __declspec(align(8)) uint64 x7fb = 0x7f7f7f7f7f7f7f7f;
		static const __declspec(align(8)) uint64 xfeb = 0xfefefefefefefefe;

		__asm {
			push		ebp
			push		edi
			push		esi
			push		ebx

			mov			esi, [esp+24+16]
			mov			eax, [esp+4+16]
			shl			esi, 4
			mov			ecx, [esp+12+16]
			mov			edx, [esp+16+16]
			mov			ebp, [esp+20+16]
			mov			edi, [esp+8+16]
			sub			edi, esi
			sub			ebp, esi

			movq		mm6, x7fb
			movq		mm7, xfeb

			mov			esi, [esp+28+16]
yloop:
			mov			ebx, [esp+24+16]
mainRowLoop:
			movq		mm0, [ecx]
			movq		mm3, [ecx + 8]
			movq		mm1, mm0
			movq		mm2, [edx]
			movq		mm4, mm3
			movq		mm5, [edx + 8]
			por			mm1, mm2
			pxor		mm0, mm2
			por			mm4, mm5
			pxor		mm3, mm5
			psrlq		mm0, 1
			pand		mm3, mm7
			pand		mm0, mm6
			psrlq		mm3, 1
			psubb		mm1, mm0
			psubb		mm4, mm3
			add			ecx, 16
			movq		[eax], mm1
			movq		[eax+8], mm4
			add			edx, 16
			add			eax, 16
			dec			ebx
			jne			mainRowLoop

			add			eax, edi
			add			ecx, ebp
			add			edx, ebp
			dec			esi
			jne			yloop

			emms
			pop			ebx
			pop			esi
			pop			edi
			pop			ebp
			ret	
		}
	}

	void __declspec(naked) __cdecl Average_ISSE(void *dst, ptrdiff_t dstPitch, const void *src1, const void *src2, ptrdiff_t srcPitch, uint32 w16, uint32 h) {
		static const __declspec(align(8)) uint64 x7fb = 0x7f7f7f7f7f7f7f7f;
		static const __declspec(align(8)) uint64 xfeb = 0xfefefefefefefefe;

		__asm {
			push		ebp
			push		edi
			push		esi
			push		ebx

			mov			esi, [esp+24+16]
			mov			eax, [esp+4+16]
			shl			esi, 4
			mov			ecx, [esp+12+16]
			mov			edx, [esp+16+16]
			mov			ebp, [esp+20+16]
			mov			edi, [esp+8+16]
			sub			edi, esi
			sub			ebp, esi

			movq		mm6, x7fb
			movq		mm7, xfeb

			mov			esi, [esp+28+16]
yloop:
			mov			ebx, [esp+24+16]
mainRowLoop:
			movq		mm0, [ecx]
			movq		mm1, [ecx + 8]
			movq		mm2, [edx]
			movq		mm3, [edx + 8]
			pavgb		mm0, mm2
			pavgb		mm1, mm3
			movq		[eax], mm0
			add			ecx, 16
			add			edx, 16
			movq		[eax+8], mm1
			add			eax, 16
			dec			ebx
			jne			mainRowLoop

			add			eax, edi
			add			ecx, ebp
			add			edx, ebp
			dec			esi
			jne			yloop

			emms
			pop			ebx
			pop			esi
			pop			edi
			pop			ebp
			ret	
		}
	}
#endif

#if defined(VD_CPU_X86) || defined(VD_CPU_AMD64)
	void Average_SSE2(void *dst, ptrdiff_t dstPitch, const void *src1, const void *src2, ptrdiff_t srcPitch, uint32 w16, uint32 h) {
		do {
			__m128i *dstv = (__m128i *)dst;
			__m128i *src1v = (__m128i *)src1;
			__m128i *src2v = (__m128i *)src2;

			for(uint32 i=0; i<w16; ++i)
				dstv[i] = _mm_avg_epu8(src1v[i], src2v[i]);

			dst = (char *)dst + dstPitch;
			src1 = (char *)src1 + srcPitch;
			src2 = (char *)src2 + srcPitch;
		} while(--h);
	}
#endif

	void InterpPlane_Bob(void *dst, ptrdiff_t dstpitch, const void *src, ptrdiff_t srcpitch, uint32 w, uint32 h, bool interpField2) {
		void (*blend_func)(void *dst, ptrdiff_t dstPitch, const void *src1, const void *src2, ptrdiff_t srcPitch, uint32 w16, uint32 h);
#if defined(VD_CPU_X86)
		if (SSE2_enabled)
			blend_func = Average_SSE2;
		else if (ISSE_enabled)
			blend_func = Average_ISSE;
		else if (MMX_enabled)
			blend_func = Average_MMX;
		else
			blend_func = Average_scalar;
#else
		blend_func = Average_SSE2;
#endif

		w = (w + 3) >> 2;

		int y0 = interpField2 ? 1 : 2;

		if (!interpField2)
			memcpy(dst, src, w * 4);

		if (h > y0) {
			ASSERT(((UINT_PTR)dst & 0xF) == 0);
			ASSERT((dstpitch & 0xF) == 0);
			ASSERT(((UINT_PTR)src & 0xF) == 0);
			ASSERT((srcpitch*(y0 - 1) & 0xF) == 0);
			blend_func((char *)dst + dstpitch*y0,
				dstpitch*2,
				(const char *)src + srcpitch*(y0 - 1),
				(const char *)src + srcpitch*(y0 + 1),
				srcpitch*2,
				(w + 3) >> 2,
				(h - y0) >> 1);
		}

		if (interpField2)
			memcpy((char *)dst + dstpitch*(h - 1), (const char *)src + srcpitch*(h - 1), w*4);

#ifdef _M_IX86
		if (MMX_enabled)
			__asm emms
#endif
	}

	void BlendPlane(void *dst, ptrdiff_t dstpitch, const void *src, ptrdiff_t srcpitch, uint32 w, uint32 h) {
		void (*blend_func)(void *, const void *, uint32, ptrdiff_t);
#if defined(VD_CPU_X86)
		if (SSE2_enabled)
			blend_func = asm_blend_row_SSE2;
		else
			blend_func = ISSE_enabled ? asm_blend_row_ISSE : MMX_enabled ? asm_blend_row_MMX : asm_blend_row;
#else
		blend_func = asm_blend_row_SSE2;
#endif

		w = (w + 3) >> 2;

		asm_blend_row_clipped(dst, src, w, srcpitch);
		if (h-=2)
			do {
				dst = ((char *)dst + dstpitch);

				blend_func(dst, src, w, srcpitch);

				src = ((char *)src + srcpitch);
			} while(--h);

		asm_blend_row_clipped((char *)dst + dstpitch, src, w, srcpitch);

#ifdef _M_IX86
		if (MMX_enabled)
			__asm emms
#endif
	}
}

void DeinterlaceBlend(BYTE* dst, BYTE* src, DWORD w, DWORD h, DWORD dstpitch, DWORD srcpitch)
{
	BlendPlane(dst, dstpitch, src, srcpitch, w, h);
}

void DeinterlaceBob(BYTE* dst, BYTE* src, DWORD w, DWORD h, DWORD dstpitch, DWORD srcpitch, bool topfield)
{
	topfield = !topfield;

	InterpPlane_Bob(dst, dstpitch, src, srcpitch, w, h, topfield);
}
