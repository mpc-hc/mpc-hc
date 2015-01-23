//  VirtualDub - Video processing and capture application
//  Copyright (C) 1998-2007 Avery Lee
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include <emmintrin.h>
#include "vd2/system/memory.h"
#include "vd2/system/cpuaccel.h"
#include "vd2/system/vdstl.h"

#pragma warning(disable: 4799)      // warning C4799: function has no EMMS instruction

///////////////////////////////////////////////////////////////////////////

#ifdef _M_IX86
static void __declspec(naked) asm_blend_row_clipped(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     edi,[esp+20]
        mov     esi,[esp+24]
        sub     edi,esi
        mov     ebp,[esp+28]
        mov     edx,[esp+32]

xloop:
        mov     ecx,[esi]
        mov     eax,0fefefefeh

        mov     ebx,[esi+edx]
        and     eax,ecx

        shr     eax,1
        and     ebx,0fefefefeh

        shr     ebx,1
        add     esi,4

        add     eax,ebx
        dec     ebp

        mov     [edi+esi-4],eax
        jnz     xloop

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
    };
}

static void __declspec(naked) asm_blend_row(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     edi,[esp+20]
        mov     esi,[esp+24]
        sub     edi,esi
        mov     ebp,[esp+28]
        mov     edx,[esp+32]

xloop:
        mov     ecx,[esi]
        mov     eax,0fcfcfcfch

        mov     ebx,[esi+edx]
        and     eax,ecx

        shr     ebx,1
        mov     ecx,[esi+edx*2]

        shr     ecx,2
        and     ebx,07f7f7f7fh

        shr     eax,2
        and     ecx,03f3f3f3fh

        add     eax,ebx
        add     esi,4

        add     eax,ecx
        dec     ebp

        mov     [edi+esi-4],eax
        jnz     xloop

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
    };
}

static void __declspec(naked) asm_blend_row_MMX(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
    static const __declspec(align(8)) __int64 mask0 = 0xfcfcfcfcfcfcfcfci64;
    static const __declspec(align(8)) __int64 mask1 = 0x7f7f7f7f7f7f7f7fi64;
    static const __declspec(align(8)) __int64 mask2 = 0x3f3f3f3f3f3f3f3fi64;
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     edi,[esp+20]
        mov     esi,[esp+24]
        sub     edi,esi
        mov     ebp,[esp+28]
        mov     edx,[esp+32]

        movq    mm5,mask0
        movq    mm6,mask1
        movq    mm7,mask2
        inc     ebp
        shr     ebp,1
xloop:
        movq    mm2,[esi]
        movq    mm0,mm5

        movq    mm1,[esi+edx]
        pand    mm0,mm2

        psrlq   mm1,1
        movq    mm2,[esi+edx*2]

        psrlq   mm2,2
        pand    mm1,mm6

        psrlq   mm0,2
        pand    mm2,mm7

        paddb   mm0,mm1
        add     esi,8

        paddb   mm0,mm2
        dec     ebp

        movq    [edi+esi-8],mm0
        jne     xloop

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret
    };
}

static void __declspec(naked) asm_blend_row_ISSE(void *dst, const void *src, uint32 w, ptrdiff_t srcpitch) {
    __asm {
        push    ebp
        push    edi
        push    esi
        push    ebx

        mov     edi,[esp+20]
        mov     esi,[esp+24]
        sub     edi,esi
        mov     ebp,[esp+28]
        mov     edx,[esp+32]

        inc     ebp
        shr     ebp,1
        pcmpeqb mm7, mm7

        align   16
xloop:
        movq    mm0, [esi]
        movq    mm2, mm7
        pxor    mm0, mm7

        pxor    mm2, [esi+edx*2]
        pavgb   mm0, mm2
        pxor    mm0, mm7

        pavgb   mm0, [esi+edx]
        add     esi,8

        movq    [edi+esi-8],mm0
        dec     ebp
        jne     xloop

        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
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


    void ela_L8_SSE2(__m128i *dst, const __m128i *srcat, const __m128i *srcab, int w16) {
        do {
            __m128i top0 = srcat[0];
            __m128i top1 = srcat[1];
            __m128i top2 = srcat[2];
            __m128i bot0 = srcab[0];
            __m128i bot1 = srcab[1];
            __m128i bot2 = srcab[2];
            ++srcat;
            ++srcab;

            __m128i topl2 = _mm_or_si128(_mm_srli_si128(top0, 16 - 3), _mm_slli_si128(top1, 3));
            __m128i topl1 = _mm_or_si128(_mm_srli_si128(top0, 16 - 2), _mm_slli_si128(top1, 2));
            __m128i topc0 = _mm_or_si128(_mm_srli_si128(top0, 16 - 1), _mm_slli_si128(top1, 1));
            __m128i topr1 = top1;
            __m128i topr2 = _mm_or_si128(_mm_srli_si128(top1, 1), _mm_slli_si128(top2, 16 - 1));
            __m128i topr3 = _mm_or_si128(_mm_srli_si128(top1, 2), _mm_slli_si128(top2, 16 - 2));

            __m128i botl2 = _mm_or_si128(_mm_srli_si128(bot0, 16 - 3), _mm_slli_si128(bot1, 3));
            __m128i botl1 = _mm_or_si128(_mm_srli_si128(bot0, 16 - 2), _mm_slli_si128(bot1, 2));
            __m128i botc0 = _mm_or_si128(_mm_srli_si128(bot0, 16 - 1), _mm_slli_si128(bot1, 1));
            __m128i botr1 = bot1;
            __m128i botr2 = _mm_or_si128(_mm_srli_si128(bot1, 1), _mm_slli_si128(bot2, 16 - 1));
            __m128i botr3 = _mm_or_si128(_mm_srli_si128(bot1, 2), _mm_slli_si128(bot2, 16 - 2));

            __m128i rawscorec0 = _mm_or_si128(_mm_subs_epu8(topc0, botc0), _mm_subs_epu8(botc0, topc0));
            __m128i rawscorel1 = _mm_or_si128(_mm_subs_epu8(topl1, botr1), _mm_subs_epu8(botr1, topl1));
            __m128i rawscorel2 = _mm_or_si128(_mm_subs_epu8(topl2, botr2), _mm_subs_epu8(botr2, topl2));
            __m128i rawscorer1 = _mm_or_si128(_mm_subs_epu8(topr1, botl1), _mm_subs_epu8(botl1, topr1));
            __m128i rawscorer2 = _mm_or_si128(_mm_subs_epu8(topr2, botl2), _mm_subs_epu8(botl2, topr2));

            dst[0] = rawscorec0;
            dst[1] = rawscorel1;
            dst[2] = rawscorel2;
            dst[3] = rawscorer1;
            dst[4] = rawscorer2;
            dst[5] = _mm_avg_epu8(topr1, botr1);
            dst[6] = _mm_avg_epu8(topc0, botr2);
            dst[7] = _mm_avg_epu8(topl1, botr3);
            dst[8] = _mm_avg_epu8(topr2, botc0);
            dst[9] = _mm_avg_epu8(topr3, botl1);
            dst += 10;
        } while(--w16);
    }

    void nela_L8_SSE2(__m128i *dst, const __m128i *elabuf, int w16) {
        __m128i zero = _mm_setzero_si128();
        __m128i x80b = _mm_set1_epi8((unsigned char)0x80);

        do {
            __m128i x0, x1, x2, y;

            x0 = elabuf[0];
            y = elabuf[10];
            x1 = _mm_or_si128(_mm_srli_si128(x0, 1), _mm_slli_si128(y, 15));
            x2 = _mm_or_si128(_mm_srli_si128(x0, 2), _mm_slli_si128(y, 14));
            __m128i scorec0 = _mm_avg_epu8(_mm_avg_epu8(x0, x2), x1);

            x0 = elabuf[1];
            y = elabuf[11];
            x1 = _mm_or_si128(_mm_srli_si128(x0, 1), _mm_slli_si128(y, 15));
            x2 = _mm_or_si128(_mm_srli_si128(x0, 2), _mm_slli_si128(y, 14));
            __m128i scorel1 = _mm_avg_epu8(_mm_avg_epu8(x0, x2), x1);

            x0 = elabuf[2];
            y = elabuf[12];
            x1 = _mm_or_si128(_mm_srli_si128(x0, 1), _mm_slli_si128(y, 15));
            x2 = _mm_or_si128(_mm_srli_si128(x0, 2), _mm_slli_si128(y, 14));
            __m128i scorel2 = _mm_avg_epu8(_mm_avg_epu8(x0, x2), x1);

            x0 = elabuf[3];
            y = elabuf[13];
            x1 = _mm_or_si128(_mm_srli_si128(x0, 1), _mm_slli_si128(y, 15));
            x2 = _mm_or_si128(_mm_srli_si128(x0, 2), _mm_slli_si128(y, 14));
            __m128i scorer1 = _mm_avg_epu8(_mm_avg_epu8(x0, x2), x1);

            x0 = elabuf[4];
            y = elabuf[14];
            x1 = _mm_or_si128(_mm_srli_si128(x0, 1), _mm_slli_si128(y, 15));
            x2 = _mm_or_si128(_mm_srli_si128(x0, 2), _mm_slli_si128(y, 14));
            __m128i scorer2 = _mm_avg_epu8(_mm_avg_epu8(x0, x2), x1);

            scorec0 = _mm_xor_si128(scorec0, x80b);
            scorel1 = _mm_xor_si128(scorel1, x80b);
            scorel2 = _mm_xor_si128(scorel2, x80b);
            scorer1 = _mm_xor_si128(scorer1, x80b);
            scorer2 = _mm_xor_si128(scorer2, x80b);

            // result = (scorel1 < scorec0) ? (scorel2 < scorel1 ? l2 : l1) : (scorer1 < scorec0) ? (scorer2 < scorer1 ? r2 : r1) : c0

            __m128i cmplt_l1_c0 = _mm_cmplt_epi8(scorel1, scorec0);
            __m128i cmplt_r1_c0 = _mm_cmplt_epi8(scorer1, scorec0);
            __m128i cmplt_l1_r1 = _mm_cmplt_epi8(scorel1, scorer1);

            __m128i is_l1 = _mm_and_si128(cmplt_l1_r1, cmplt_l1_c0);
            __m128i is_r1 = _mm_andnot_si128(cmplt_l1_r1, cmplt_r1_c0);
            __m128i is_c0_inv = _mm_or_si128(cmplt_l1_c0, cmplt_r1_c0);
            __m128i is_c0 = _mm_andnot_si128(is_c0_inv, _mm_cmpeq_epi8(zero, zero));

            __m128i is_l2 = _mm_and_si128(is_l1, _mm_cmplt_epi8(scorel2, scorel1));
            __m128i is_r2 = _mm_and_si128(is_r1, _mm_cmplt_epi8(scorer2, scorer1));

            is_l1 = _mm_andnot_si128(is_l2, is_l1);
            is_r1 = _mm_andnot_si128(is_r2, is_r1);

            __m128i mask_c0 = is_c0;
            __m128i mask_l1 = is_l1;
            __m128i mask_l2 = is_l2;
            __m128i mask_r1 = is_r1;
            __m128i mask_r2 = is_r2;

            __m128i result_c0 = _mm_and_si128(elabuf[5], mask_c0);
            __m128i result_l1 = _mm_and_si128(elabuf[6], mask_l1);
            __m128i result_l2 = _mm_and_si128(elabuf[7], mask_l2);
            __m128i result_r1 = _mm_and_si128(elabuf[8], mask_r1);
            __m128i result_r2 = _mm_and_si128(elabuf[9], mask_r2);

            elabuf += 10;

            __m128i pred = _mm_or_si128(_mm_or_si128(_mm_or_si128(result_l1, result_l2), _mm_or_si128(result_r1, result_r2)), result_c0);

            *dst++ = pred;
        } while(--w16);
    }
#endif

#if defined(VD_CPU_X86)
    void __declspec(naked) __stdcall ela_L8_MMX(void *dst, const void *srcat, const void *srcab, int w16) {
        static const __declspec(align(8)) uint64 xFEb = 0xfefefefefefefefe;
        __asm {
            push        ebx
            mov         ebx, [esp + 4 + 4]
            mov         ecx, [esp + 8 + 4]
            mov         edx, [esp + 12 + 4]
            mov         eax, [esp + 16 + 4]
            movq        mm7, xFEb

xloop:
            movq        mm0, [ecx + 15]
            movq        mm1, [edx + 15]
            movq        mm4, mm0
            movq        mm2, [ecx + 15 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 15 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx], mm0
            por         mm2, mm3
            movq        [ebx + 8], mm2

            movq        mm0, [ecx + 14]
            movq        mm1, [edx + 16]
            movq        mm4, mm0
            movq        mm2, [ecx + 14 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 16 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 16], mm0
            por         mm2, mm3
            movq        [ebx + 24], mm2

            movq        mm0, [ecx + 13]
            movq        mm1, [edx + 17]
            movq        mm4, mm0
            movq        mm2, [ecx + 13 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 17 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 32], mm0
            por         mm2, mm3
            movq        [ebx + 40], mm2

            movq        mm0, [ecx + 16]
            movq        mm1, [edx + 14]
            movq        mm4, mm0
            movq        mm2, [ecx + 16 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 14 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 48], mm0
            por         mm2, mm3
            movq        [ebx + 56], mm2

            movq        mm0, [ecx + 17]
            movq        mm1, [edx + 13]
            movq        mm4, mm0
            movq        mm2, [ecx + 17 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 13 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 64], mm0
            por         mm2, mm3
            movq        [ebx + 72], mm2

            movq        mm0, [ecx + 16]
            movq        mm1, [edx + 16]
            movq        mm2, [ecx + 16 + 8]
            movq        mm3, [edx + 16 + 8]
            movq        mm4, mm0
            movq        mm5, mm2
            pxor        mm0, mm1
            pxor        mm2, mm3
            por         mm1, mm4
            por         mm3, mm5
            pand        mm0, mm7
            pand        mm2, mm7
            psrlw       mm0, 1
            psrlw       mm2, 1
            psubb       mm1, mm0
            psubb       mm3, mm2
            movq        [ebx + 80], mm1
            movq        [ebx + 88], mm3

            movq        mm0, [ecx + 15]
            movq        mm1, [edx + 17]
            movq        mm2, [ecx + 15 + 8]
            movq        mm3, [edx + 17 + 8]
            movq        mm4, mm0
            movq        mm5, mm2
            pxor        mm0, mm1
            pxor        mm2, mm3
            por         mm1, mm4
            por         mm3, mm5
            pand        mm0, mm7
            pand        mm2, mm7
            psrlw       mm0, 1
            psrlw       mm2, 1
            psubb       mm1, mm0
            psubb       mm3, mm2
            movq        [ebx + 96], mm1
            movq        [ebx + 104], mm3

            movq        mm0, [ecx + 14]
            movq        mm1, [edx + 18]
            movq        mm2, [ecx + 14 + 8]
            movq        mm3, [edx + 18 + 8]
            movq        mm4, mm0
            movq        mm5, mm2
            pxor        mm0, mm1
            pxor        mm2, mm3
            por         mm1, mm4
            por         mm3, mm5
            pand        mm0, mm7
            pand        mm2, mm7
            psrlw       mm0, 1
            psrlw       mm2, 1
            psubb       mm1, mm0
            psubb       mm3, mm2
            movq        [ebx + 112], mm1
            movq        [ebx + 120], mm3

            movq        mm0, [ecx + 17]
            movq        mm1, [edx + 15]
            movq        mm2, [ecx + 17 + 8]
            movq        mm3, [edx + 15 + 8]
            movq        mm4, mm0
            movq        mm5, mm2
            pxor        mm0, mm1
            pxor        mm2, mm3
            por         mm1, mm4
            por         mm3, mm5
            pand        mm0, mm7
            pand        mm2, mm7
            psrlw       mm0, 1
            psrlw       mm2, 1
            psubb       mm1, mm0
            psubb       mm3, mm2
            movq        [ebx + 128], mm1
            movq        [ebx + 136], mm3

            movq        mm0, [ecx + 18]
            movq        mm1, [edx + 14]
            movq        mm2, [ecx + 18 + 8]
            movq        mm3, [edx + 14 + 8]
            movq        mm4, mm0
            movq        mm5, mm2
            pxor        mm0, mm1
            pxor        mm2, mm3
            por         mm1, mm4
            por         mm3, mm5
            pand        mm0, mm7
            pand        mm2, mm7
            psrlw       mm0, 1
            psrlw       mm2, 1
            psubb       mm1, mm0
            psubb       mm3, mm2
            movq        [ebx + 144], mm1
            movq        [ebx + 152], mm3

            add         ebx, 160
            add         ecx, 16
            add         edx, 16
            dec         eax
            jne         xloop

            emms
            pop         ebx
            ret         16
        }
    }

    void __declspec(naked) __stdcall ela_L8_ISSE(void *dst, const void *srcat, const void *srcab, int w16) {
        static const __declspec(align(8)) uint64 xFEb = 0xfefefefefefefefe;
        __asm {
            push        ebx
            mov         ebx, [esp + 4 + 4]
            mov         ecx, [esp + 8 + 4]
            mov         edx, [esp + 12 + 4]
            mov         eax, [esp + 16 + 4]
            movq        mm7, xFEb

xloop:
            movq        mm0, [ecx + 15]
            movq        mm1, [edx + 15]
            movq        mm4, mm0
            movq        mm2, [ecx + 15 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 15 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx], mm0
            por         mm2, mm3
            movq        [ebx + 8], mm2

            movq        mm0, [ecx + 14]
            movq        mm1, [edx + 16]
            movq        mm4, mm0
            movq        mm2, [ecx + 14 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 16 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 16], mm0
            por         mm2, mm3
            movq        [ebx + 24], mm2

            movq        mm0, [ecx + 13]
            movq        mm1, [edx + 17]
            movq        mm4, mm0
            movq        mm2, [ecx + 13 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 17 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 32], mm0
            por         mm2, mm3
            movq        [ebx + 40], mm2

            movq        mm0, [ecx + 16]
            movq        mm1, [edx + 14]
            movq        mm4, mm0
            movq        mm2, [ecx + 16 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 14 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 48], mm0
            por         mm2, mm3
            movq        [ebx + 56], mm2

            movq        mm0, [ecx + 17]
            movq        mm1, [edx + 13]
            movq        mm4, mm0
            movq        mm2, [ecx + 17 + 8]
            psubusb     mm0, mm1
            movq        mm3, [edx + 13 + 8]
            movq        mm5, mm2
            psubusb     mm2, mm3
            psubusb     mm1, mm4
            psubusb     mm3, mm5
            por         mm0, mm1
            movq        [ebx + 64], mm0
            por         mm2, mm3
            movq        [ebx + 72], mm2

            movq        mm0, [ecx + 16]
            movq        mm1, [edx + 16]
            movq        mm2, [ecx + 16 + 8]
            movq        mm3, [edx + 16 + 8]
            pavgb       mm1, mm0
            pavgb       mm3, mm2
            movq        [ebx + 80], mm1
            movq        [ebx + 88], mm3

            movq        mm0, [ecx + 15]
            movq        mm1, [edx + 17]
            movq        mm2, [ecx + 15 + 8]
            movq        mm3, [edx + 17 + 8]
            pavgb       mm1, mm0
            pavgb       mm3, mm2
            movq        [ebx + 96], mm1
            movq        [ebx + 104], mm3

            movq        mm0, [ecx + 14]
            movq        mm1, [edx + 18]
            movq        mm2, [ecx + 14 + 8]
            movq        mm3, [edx + 18 + 8]
            pavgb       mm1, mm0
            pavgb       mm3, mm2
            movq        [ebx + 112], mm1
            movq        [ebx + 120], mm3

            movq        mm0, [ecx + 17]
            movq        mm1, [edx + 15]
            movq        mm2, [ecx + 17 + 8]
            movq        mm3, [edx + 15 + 8]
            pavgb       mm1, mm0
            pavgb       mm3, mm2
            movq        [ebx + 128], mm1
            movq        [ebx + 136], mm3

            movq        mm0, [ecx + 18]
            movq        mm1, [edx + 14]
            movq        mm2, [ecx + 18 + 8]
            movq        mm3, [edx + 14 + 8]
            pavgb       mm1, mm0
            pavgb       mm3, mm2
            movq        [ebx + 144], mm1
            movq        [ebx + 152], mm3

            add         ebx, 160
            add         ecx, 16
            add         edx, 16
            dec         eax
            jne         xloop

            emms
            pop         ebx
            ret         16
        }
    }

    void __declspec(naked) __stdcall nela_L8_ISSE(void *dst, const void *elabuf, int w16) {
        static const __declspec(align(8)) uint64 x7fb = 0x7f7f7f7f7f7f7f7f;
        __asm {
            mov     edx, [esp+4]
            mov     ecx, [esp+8]
            mov     eax, [esp+12]
xloop:
            movq    mm0, [ecx+000h]
            pavgb   mm0, [ecx+002h]
            pavgb   mm0, [ecx+001h]

            movq    mm1, [ecx+010h]
            pavgb   mm1, [ecx+012h]
            pavgb   mm1, [ecx+011h]

            movq    mm2, [ecx+020h]
            pavgb   mm2, [ecx+022h]
            pavgb   mm2, [ecx+021h]

            movq    mm3, [ecx+030h]
            pavgb   mm3, [ecx+032h]
            pavgb   mm3, [ecx+031h]

            movq    mm4, [ecx+040h]
            pavgb   mm4, [ecx+042h]
            pavgb   mm4, [ecx+041h]

            movq    mm5, x7fb
            pxor    mm0, mm5
            pxor    mm1, mm5
            pxor    mm2, mm5
            pxor    mm3, mm5
            pxor    mm4, mm5

            ;mm0 = scorec0
            ;mm1 = scorel1
            ;mm2 = scorel2
            ;mm3 = scorer1
            ;mm4 = scorer2

            movq    mm5, mm3
            pcmpgtb mm5, mm1        ;(scorer1 > scorel1) == (scorel1 < scorer1)

            pcmpgtb mm4, mm3        ;scorer2 > scorer1
            pcmpgtb mm2, mm1        ;scorel2 > scorel1

            pcmpgtb mm1, mm0        ;scorel1 > scorec0
            pcmpgtb mm3, mm0        ;scorer1 > scorec0

            movq    mm6, mm1
            pcmpeqb mm0, mm0
            por     mm6, mm3        ;scorel1 > scorec0 || scorer1 > scorec0
            pxor    mm0, mm6        ;mask_c0

            pand    mm3, mm5        ;scorer1 > scorec0 && scorer1 > scorel1
            pandn   mm5, mm1        ;scorel1 > scorec0 && scorel1 >= scorer1

            pand    mm4, mm3        ;mask_r2
            pand    mm2, mm5        ;mask_l2

            pxor    mm3, mm4        ;mask_r1
            pxor    mm5, mm2        ;mask_l1

            pand    mm0, [ecx+050h]
            pand    mm5, [ecx+060h]
            pand    mm2, [ecx+070h]
            pand    mm3, [ecx+080h]
            pand    mm4, [ecx+090h]
            por     mm0, mm5
            por     mm2, mm3
            por     mm0, mm4
            por     mm0, mm2
            movq    [edx], mm0

            movq    mm0, [ecx+008h]
            movq    mm5, [ecx+0a0h]
            movq    mm6, mm0
            psrlq   mm0, 16
            movq    mm7, mm5
            psllq   mm5, 48
            por     mm0, mm5
            psrlq   mm6, 8
            psllq   mm7, 56
            por     mm6, mm7
            pavgb   mm0, [ecx+008h]
            pavgb   mm0, mm6

            movq    mm1, [ecx+018h]
            movq    mm5, [ecx+0b0h]
            movq    mm6, mm1
            psrlq   mm1, 16
            movq    mm7, mm5
            psllq   mm5, 48
            por     mm1, mm5
            psrlq   mm6, 8
            psllq   mm7, 56
            por     mm6, mm7
            pavgb   mm1, [ecx+018h]
            pavgb   mm1, mm6

            movq    mm2, [ecx+028h]
            movq    mm5, [ecx+0c0h]
            movq    mm6, mm2
            psrlq   mm2, 16
            movq    mm7, mm5
            psllq   mm5, 48
            por     mm2, mm5
            psrlq   mm6, 8
            psllq   mm7, 56
            por     mm6, mm7
            pavgb   mm2, [ecx+028h]
            pavgb   mm2, mm6

            movq    mm3, [ecx+038h]
            movq    mm5, [ecx+0d0h]
            movq    mm6, mm3
            psrlq   mm3, 16
            movq    mm7, mm5
            psllq   mm5, 48
            por     mm3, mm5
            psrlq   mm6, 8
            psllq   mm7, 56
            por     mm6, mm7
            pavgb   mm3, [ecx+038h]
            pavgb   mm3, mm6

            movq    mm4, [ecx+048h]
            movq    mm5, [ecx+0e0h]
            movq    mm6, mm4
            psrlq   mm4, 16
            movq    mm7, mm5
            psllq   mm5, 48
            por     mm4, mm5
            psrlq   mm6, 8
            psllq   mm7, 56
            por     mm6, mm7
            pavgb   mm4, [ecx+048h]
            pavgb   mm4, mm6

            movq    mm5, x7fb
            pxor    mm0, mm5
            pxor    mm1, mm5
            pxor    mm2, mm5
            pxor    mm3, mm5
            pxor    mm4, mm5

            ;mm0 = scorec0
            ;mm1 = scorel1
            ;mm2 = scorel2
            ;mm3 = scorer1
            ;mm4 = scorer2

            movq    mm5, mm3
            pcmpgtb mm5, mm1        ;(scorer1 > scorel1) == (scorel1 < scorer1)

            pcmpgtb mm4, mm3        ;scorer2 > scorer1
            pcmpgtb mm2, mm1        ;scorel2 > scorel1

            pcmpgtb mm1, mm0        ;scorel1 > scorec0
            pcmpgtb mm3, mm0        ;scorer1 > scorec0

            movq    mm6, mm1
            pcmpeqb mm0, mm0
            por     mm6, mm3        ;scorel1 > scorec0 || scorer1 > scorec0
            pxor    mm0, mm6        ;mask_c0

            pand    mm3, mm5        ;scorer1 > scorec0 && scorer1 > scorel1
            pandn   mm5, mm1        ;scorel1 > scorec0 && scorel1 >= scorer1

            pand    mm4, mm3        ;mask_r2
            pand    mm2, mm5        ;mask_l2

            pxor    mm3, mm4        ;mask_r1
            pxor    mm5, mm2        ;mask_l1

            pand    mm0, [ecx+058h]
            pand    mm5, [ecx+068h]
            pand    mm2, [ecx+078h]
            pand    mm3, [ecx+088h]
            pand    mm4, [ecx+098h]
            por     mm0, mm5
            por     mm2, mm3
            por     mm0, mm4
            por     mm0, mm2
            movq    [edx+8], mm0

            add     ecx, 0a0h
            add     edx, 16

            dec     eax
            jnz     xloop

            emms
            ret     12
        }
    }

    void __declspec(naked) __stdcall nela_L8_MMX(void *dst, const void *elabuf, int w16) {
        static const __declspec(align(8)) uint64 x7fb = 0x7f7f7f7f7f7f7f7f;

        __asm {
            mov     edx, [esp+4]
            mov     ecx, [esp+8]
            mov     eax, [esp+12]
xloop:
            movq    mm0, [ecx+000h]
            movq    mm5, [ecx+002h]
            movq    mm6, mm0
            pxor    mm0, mm5
            por     mm6, mm5
            movq    mm7, [ecx+001h]
            psrlq   mm0, 1
            pand    mm0, x7fb
            psubb   mm6, mm0
            movq    mm0, mm6
            pxor    mm6, mm7
            por     mm0, mm7
            psrlq   mm6, 1
            pand    mm6, x7fb
            psubb   mm0, mm6

            movq    mm1, [ecx+010h]
            movq    mm5, [ecx+012h]
            movq    mm6, mm1
            pxor    mm1, mm5
            por     mm6, mm5
            movq    mm7, [ecx+011h]
            psrlq   mm1, 1
            pand    mm1, x7fb
            psubb   mm6, mm1
            movq    mm1, mm6
            pxor    mm6, mm7
            por     mm1, mm7
            psrlq   mm6, 1
            pand    mm6, x7fb
            psubb   mm1, mm6

            movq    mm2, [ecx+020h]
            movq    mm5, [ecx+022h]
            movq    mm6, mm2
            pxor    mm2, mm5
            por     mm6, mm5
            movq    mm7, [ecx+021h]
            psrlq   mm2, 1
            pand    mm2, x7fb
            psubb   mm6, mm2
            movq    mm2, mm6
            pxor    mm6, mm7
            por     mm2, mm7
            psrlq   mm6, 1
            pand    mm6, x7fb
            psubb   mm2, mm6

            movq    mm3, [ecx+030h]
            movq    mm5, [ecx+032h]
            movq    mm6, mm3
            pxor    mm3, mm5
            por     mm6, mm5
            movq    mm7, [ecx+031h]
            psrlq   mm3, 1
            pand    mm3, x7fb
            psubb   mm6, mm3
            movq    mm3, mm6
            pxor    mm6, mm7
            por     mm3, mm7
            psrlq   mm6, 1
            pand    mm6, x7fb
            psubb   mm3, mm6

            movq    mm4, [ecx+040h]
            movq    mm5, [ecx+042h]
            movq    mm6, mm4
            pxor    mm4, mm5
            por     mm6, mm5
            movq    mm7, [ecx+041h]
            psrlq   mm4, 1
            pand    mm4, x7fb
            psubb   mm6, mm4
            movq    mm4, mm6
            pxor    mm6, mm7
            por     mm4, mm7
            psrlq   mm6, 1
            pand    mm6, x7fb
            psubb   mm4, mm6


            movq    mm5, x7fb
            pxor    mm0, mm5
            pxor    mm1, mm5
            pxor    mm2, mm5
            pxor    mm3, mm5
            pxor    mm4, mm5

            ;mm0 = scorec0
            ;mm1 = scorel1
            ;mm2 = scorel2
            ;mm3 = scorer1
            ;mm4 = scorer2

            movq    mm5, mm3
            pcmpgtb mm5, mm1        ;(scorer1 > scorel1) == (scorel1 < scorer1)

            pcmpgtb mm4, mm3        ;scorer2 > scorer1
            pcmpgtb mm2, mm1        ;scorel2 > scorel1

            pcmpgtb mm1, mm0        ;scorel1 > scorec0
            pcmpgtb mm3, mm0        ;scorer1 > scorec0

            movq    mm6, mm1
            pcmpeqb mm0, mm0
            por     mm6, mm3        ;scorel1 > scorec0 || scorer1 > scorec0
            pxor    mm0, mm6        ;mask_c0

            pand    mm3, mm5        ;scorer1 > scorec0 && scorer1 > scorel1
            pandn   mm5, mm1        ;scorel1 > scorec0 && scorel1 >= scorer1

            pand    mm4, mm3        ;mask_r2
            pand    mm2, mm5        ;mask_l2

            pxor    mm3, mm4        ;mask_r1
            pxor    mm5, mm2        ;mask_l1

            pand    mm0, [ecx+050h]
            pand    mm5, [ecx+060h]
            pand    mm2, [ecx+070h]
            pand    mm3, [ecx+080h]
            pand    mm4, [ecx+090h]
            por     mm0, mm5
            por     mm2, mm3
            por     mm0, mm4
            por     mm0, mm2

            movq    [edx], mm0

            movq    mm0, [ecx+008h]     ;mm0 = x0
            movq    mm5, [ecx+0a0h]     ;mm5 = x1
            psrlq   mm0, 16             ;mm0 = (x0 >> 16)
            movq    mm7, [ecx+008h]     ;mm7 = y0 = x0
            psllq   mm5, 48             ;mm5 = (x1 << 48)
            movq    mm6, mm7            ;mm6 = y0 = x0
            por     mm0, mm5            ;mm0 = y2 = (x0 >> 16) | (x1 << 48)
            pxor    mm6, mm0            ;mm6 = y0 ^ y2
            por     mm7, mm0            ;mm7 = y0 | y2
            movq    mm5, [ecx+008h]     ;mm5 = x0
            psrlq   mm6, 1              ;mm6 = (y0 ^ y2) >> 1
            movq    mm0, [ecx+0a0h]     ;mm0 = x1
            psrlq   mm5, 8              ;mm5 = x0 >> 8
            pand    mm6, x7fb           ;mm6 = ((y0 ^ y2) >> 1) & 0x7f7f7f7f7f7f7f7f
            psllq   mm0, 56             ;mm0 = x1 << 56
            psubb   mm7, mm6            ;mm7 = t = (y0 | y2) - (((y0 ^ y2) >> 1) & 0x7f7f7f7f7f7f7f7f) = avgb(y0, y2)
            por     mm0, mm5            ;mm0 = y1 = (x0 >> 8) | (x1 << 56)
            movq    mm6, mm7            ;mm6 = t
            pxor    mm7, mm0            ;mm7 = t ^ y1
            por     mm0, mm6            ;mm0 = t | y1
            psrlq   mm7, 1              ;mm7 = (t ^ y1) >> 1
            pand    mm7, x7fb           ;mm7 = ((t ^ y1) >> 1) & 0x7f7f7f7f7f7f7f7f
            psubb   mm0, mm7            ;mm0 = (t | y1) - (((t ^ y1) >> 1) & 0x7f7f7f7f7f7f7f7f) = avgb(y1, avgb(y0, y2))

            movq    mm1, [ecx+018h]
            movq    mm5, [ecx+0b0h]
            psrlq   mm1, 16
            movq    mm7, [ecx+018h]
            psllq   mm5, 48
            movq    mm6, mm7
            por     mm1, mm5
            pxor    mm6, mm1
            por     mm7, mm1
            movq    mm5, [ecx+018h]
            psrlq   mm6, 1
            movq    mm1, [ecx+0b0h]
            psrlq   mm5, 8
            pand    mm6, x7fb
            psllq   mm1, 56
            psubb   mm7, mm6
            por     mm1, mm5
            movq    mm6, mm7
            pxor    mm7, mm1
            por     mm1, mm6
            psrlq   mm7, 1
            pand    mm7, x7fb
            psubb   mm1, mm7

            movq    mm2, [ecx+028h]
            movq    mm5, [ecx+0c0h]
            psrlq   mm2, 16
            movq    mm7, [ecx+028h]
            psllq   mm5, 48
            movq    mm6, mm7
            por     mm2, mm5
            pxor    mm6, mm2
            por     mm7, mm2
            movq    mm5, [ecx+028h]
            psrlq   mm6, 1
            movq    mm2, [ecx+0c0h]
            psrlq   mm5, 8
            pand    mm6, x7fb
            psllq   mm2, 56
            psubb   mm7, mm6
            por     mm2, mm5
            movq    mm6, mm7
            pxor    mm7, mm2
            por     mm2, mm6
            psrlq   mm7, 1
            pand    mm7, x7fb
            psubb   mm2, mm7

            movq    mm3, [ecx+038h]
            movq    mm5, [ecx+0d0h]
            psrlq   mm3, 16
            movq    mm7, [ecx+038h]
            psllq   mm5, 48
            movq    mm6, mm7
            por     mm3, mm5
            pxor    mm6, mm3
            por     mm7, mm3
            movq    mm5, [ecx+038h]
            psrlq   mm6, 1
            movq    mm3, [ecx+0d0h]
            psrlq   mm5, 8
            pand    mm6, x7fb
            psllq   mm3, 56
            psubb   mm7, mm6
            por     mm3, mm5
            movq    mm6, mm7
            pxor    mm7, mm3
            por     mm3, mm6
            psrlq   mm7, 1
            pand    mm7, x7fb
            psubb   mm3, mm7

            movq    mm4, [ecx+048h]
            movq    mm5, [ecx+0e0h]
            psrlq   mm4, 16
            movq    mm7, [ecx+048h]
            psllq   mm5, 48
            movq    mm6, mm7
            por     mm4, mm5
            pxor    mm6, mm4
            por     mm7, mm4
            movq    mm5, [ecx+048h]
            psrlq   mm6, 1
            movq    mm4, [ecx+0e0h]
            psrlq   mm5, 8
            pand    mm6, x7fb
            psllq   mm4, 56
            psubb   mm7, mm6
            por     mm4, mm5
            movq    mm6, mm7
            pxor    mm7, mm4
            por     mm4, mm6
            psrlq   mm7, 1
            pand    mm7, x7fb
            psubb   mm4, mm7

            movq    mm5, x7fb
            pxor    mm0, mm5
            pxor    mm1, mm5
            pxor    mm2, mm5
            pxor    mm3, mm5
            pxor    mm4, mm5

            ;mm0 = scorec0
            ;mm1 = scorel1
            ;mm2 = scorel2
            ;mm3 = scorer1
            ;mm4 = scorer2

            movq    mm5, mm3
            pcmpgtb mm5, mm1        ;(scorer1 > scorel1) == (scorel1 < scorer1)

            pcmpgtb mm4, mm3        ;scorer2 > scorer1
            pcmpgtb mm2, mm1        ;scorel2 > scorel1

            pcmpgtb mm1, mm0        ;scorel1 > scorec0
            pcmpgtb mm3, mm0        ;scorer1 > scorec0

            movq    mm6, mm1
            pcmpeqb mm0, mm0
            por     mm6, mm3        ;scorel1 > scorec0 || scorer1 > scorec0
            pxor    mm0, mm6        ;mask_c0

            pand    mm3, mm5        ;scorer1 > scorec0 && scorer1 > scorel1
            pandn   mm5, mm1        ;scorel1 > scorec0 && scorel1 >= scorer1

            pand    mm4, mm3        ;mask_r2
            pand    mm2, mm5        ;mask_l2

            pxor    mm3, mm4        ;mask_r1
            pxor    mm5, mm2        ;mask_l1

            pand    mm0, [ecx+058h]
            pand    mm5, [ecx+068h]
            pand    mm2, [ecx+078h]
            pand    mm3, [ecx+088h]
            pand    mm4, [ecx+098h]
            por     mm0, mm5
            por     mm2, mm3
            por     mm0, mm4
            por     mm0, mm2
            movq    [edx+8], mm0

            add     ecx, 0a0h
            add     edx, 16

            dec     eax
            jnz     xloop

            emms
            ret     12
        }
    }
#endif

namespace {
    void ela_L8_scalar(uint8 *dst, const uint8 *srcat, const uint8 *srcab, int w16) {
        int w = w16 << 4;

        srcat += 16;
        srcab += 16;
        do {
            int topl2 = srcat[-3];
            int topl1 = srcat[-2];
            int topc0 = srcat[-1];
            int topr1 = srcat[0];
            int topr2 = srcat[1];
            int topr3 = srcat[2];

            int botl2 = srcab[-3];
            int botl1 = srcab[-2];
            int botc0 = srcab[-1];
            int botr1 = srcab[0];
            int botr2 = srcab[1];
            int botr3 = srcab[2];
            ++srcat;
            ++srcab;

            int rawscorec0 = abs(topc0 - botc0);
            int rawscorel1 = abs(topl1 - botr1);
            int rawscorel2 = abs(topl2 - botr2);
            int rawscorer1 = abs(topr1 - botl1);
            int rawscorer2 = abs(topr2 - botl2);

            dst[0] = (uint8)rawscorec0;
            dst[1] = (uint8)rawscorel1;
            dst[2] = (uint8)rawscorel2;
            dst[3] = (uint8)rawscorer1;
            dst[4] = (uint8)rawscorer2;
            dst[5] = (uint8)((topr1 + botr1 + 1) >> 1);
            dst[6] = (uint8)((topc0 + botr2 + 1) >> 1);
            dst[7] = (uint8)((topl1 + botr3 + 1) >> 1);
            dst[8] = (uint8)((topr2 + botc0 + 1) >> 1);
            dst[9] = (uint8)((topr3 + botl1 + 1) >> 1);
            dst += 10;
        } while(--w);
    }

    void nela_L8_scalar(uint8 *dst, const uint8 *elabuf, int w16) {
        int w = w16 << 4;

        do {
            int scorec0 = elabuf[10]*2 + (elabuf[0] + elabuf[20]);
            int result = elabuf[5];

            int scorel1 = elabuf[11]*2 + (elabuf[1] + elabuf[21]);
            if (scorel1 < scorec0) {
                result = elabuf[6];
                scorec0 = scorel1;

                int scorel2 = elabuf[12]*2 + (elabuf[2] + elabuf[22]);
                if (scorel2 < scorec0) {
                    result = elabuf[7];
                    scorec0 = scorel2;
                }
            }

            int scorer1 = elabuf[13]*2 + (elabuf[3] + elabuf[23]);
            if (scorer1 < scorec0) {
                result = elabuf[8];
                scorec0 = scorer1;

                int scorer2 = elabuf[14]*2 + (elabuf[4] + elabuf[24]);
                if (scorer2 < scorec0)
                    result = elabuf[9];
            }

            elabuf += 10;

            *dst++ = (uint8)result;
        } while(--w);
    }

    void BlendScanLine_NELA_scalar(void *dst, const void *srcT, const void *srcB, uint32 w, uint8 *tempBuf) {
        const uint8 *srcat = (const uint8 *)srcT;
        const uint8 *srcab = (const uint8 *)srcB;
        uint32 w16 = (w + 15) >> 4;
        uint32 wr = w16 << 4;

        uint8 *elabuf = tempBuf;
        uint8 *topbuf = elabuf + 10*wr;
        uint8 *botbuf = topbuf + wr + 32;

        uint32 woffset = w & 15;
        topbuf[13] = topbuf[14] = topbuf[15] = srcat[0];
        botbuf[13] = botbuf[14] = botbuf[15] = srcab[0];

        for(uint32 x=0; x<wr; ++x) {
            topbuf[x+16] = srcat[x];
            botbuf[x+16] = srcab[x];
        }

        if (woffset) {
            uint8 *topfinal = &topbuf[w+16];
            uint8 *botfinal = &botbuf[w+16];
            const uint8 tv = topfinal[-1];
            const uint8 bv = botfinal[-1];

            for(uint32 i = woffset; i < 16; ++i) {
                *topfinal++ = tv;
                *botfinal++ = bv;
            }
        }

        topbuf[wr+16] = topbuf[wr+17] = topbuf[wr+18] = topbuf[wr+15];
        botbuf[wr+16] = botbuf[wr+17] = botbuf[wr+18] = botbuf[wr+15];

        ela_L8_scalar(elabuf, topbuf, botbuf, w16);
        nela_L8_scalar((uint8 *)dst, elabuf, w16);
    }

    void ela_X8R8G8B8_scalar(uint32 *dst, const uint8 *srcat, const uint8 *srcab, int w4) {
        srcat += 4;
        srcab += 4;
        do {
            const uint8 *src1 = srcat;
            const uint8 *src2 = srcab + 16;

            for(int i=0; i<5; ++i) {
                int er = abs((int)src1[2] - (int)src2[2]);
                int eg = abs((int)src1[1] - (int)src2[1]);
                int eb = abs((int)src1[0] - (int)src2[0]);
                *dst++ = er*54 + eg*183 + eb*19;
                src1 += 4;
                src2 -= 4;
            }

            srcat += 4;
            srcab += 4;
        } while(--w4);
    }

#if defined(VD_CPU_X86)
    void __declspec(naked) __cdecl ela_X8R8G8B8_MMX(uint32 *dst, const uint8 *srcat, const uint8 *srcab, int w4) {
        static const __declspec(align(16)) uint64 kCoeff = 0x00003600b70013ull;

        __asm {
            push        ebp
                push        edi
                push        esi
                push        ebx

                mov         ebx, [esp+4+16]
            mov         ecx, [esp+8+16]
            mov         edx, [esp+12+16]
            add         ecx, 4
                add         edx, 4
                mov         esi, [esp+16+16]
            movq        mm6, qword ptr [kCoeff]
            pxor        mm7, mm7

                align   16
xloop:
            movd        mm0, [ecx]
            movd        mm2, [edx + 16]
            movq        mm1, mm0
                psubusb     mm0, mm2
                psubusb     mm2, mm1
                por         mm0, mm2
                punpcklbw   mm0, mm7
                pmaddwd     mm0, mm6
                movq        mm1, mm0
                psrlq       mm0, 32
                paddd       mm0, mm1
                movd        [ebx], mm0

                movd        mm0, [ecx + 4]
            movd        mm2, [edx + 12]
            movq        mm1, mm0
                psubusb     mm0, mm2
                psubusb     mm2, mm1
                por         mm0, mm2
                punpcklbw   mm0, mm7
                pmaddwd     mm0, mm6
                movq        mm1, mm0
                psrlq       mm0, 32
                paddd       mm0, mm1
                movd        [ebx + 4], mm0

                movd        mm0, [ecx + 8]
            movd        mm2, [edx + 8]
            movq        mm1, mm0
                psubusb     mm0, mm2
                psubusb     mm2, mm1
                por         mm0, mm2
                punpcklbw   mm0, mm7
                pmaddwd     mm0, mm6
                movq        mm1, mm0
                psrlq       mm0, 32
                paddd       mm0, mm1
                movd        [ebx + 8], mm0

                movd        mm0, [ecx + 12]
            movd        mm2, [edx + 4]
            movq        mm1, mm0
                psubusb     mm0, mm2
                psubusb     mm2, mm1
                por         mm0, mm2
                punpcklbw   mm0, mm7
                pmaddwd     mm0, mm6
                movq        mm1, mm0
                psrlq       mm0, 32
                paddd       mm0, mm1
                movd        [ebx + 12], mm0

                movd        mm0, [ecx + 16]
            movd        mm2, [edx]
            movq        mm1, mm0
                psubusb     mm0, mm2
                psubusb     mm2, mm1
                por         mm0, mm2
                punpcklbw   mm0, mm7
                pmaddwd     mm0, mm6
                movq        mm1, mm0
                psrlq       mm0, 32
                paddd       mm0, mm1
                movd        [ebx + 16], mm0

                add         ebx, 20
                add         ecx, 4
                add         edx, 4
                dec         esi
                jne         xloop

                emms
                pop         ebx
                pop         esi
                pop         edi
                pop         ebp
                ret
        }
    }
#endif

    void nela_X8R8G8B8_scalar(uint32 *dst, const uint32 *elabuf, const uint8 *srca, const uint8 *srcb, int w4) {
        do {
            int scorec0 = elabuf[7]*2 + (elabuf[2] + elabuf[12]);
            int offset = 0;

            int scorel1 = elabuf[6]*2 + (elabuf[1] + elabuf[11]);
            if (scorel1 < scorec0) {
                offset = -4;
                scorec0 = scorel1;

                int scorel2 = elabuf[5]*2 + (elabuf[0] + elabuf[10]);
                if (scorel2 < scorec0) {
                    offset = -8;
                    scorec0 = scorel2;
                }
            }

            int scorer1 = elabuf[8]*2 + (elabuf[3] + elabuf[13]);
            if (scorer1 < scorec0) {
                offset = 4;
                scorec0 = scorer1;

                int scorer2 = elabuf[9]*2 + (elabuf[4] + elabuf[14]);
                if (scorer2 < scorec0)
                    offset = 8;
            }

            elabuf += 5;

            const uint32 a = *(const uint32 *)(srca + offset);
            const uint32 b = *(const uint32 *)(srcb - offset);
            *dst++ = (a|b) - (((a^b) & 0xfefefefe) >> 1);
            srca += 4;
            srcb += 4;
        } while(--w4);
    }

    void BlendScanLine_NELA_X8R8G8B8_scalar(void *dst, const void *srcT, const void *srcB, uint32 w, void *tempBuf) {
        const uint32 *srcat = (const uint32 *)srcT;
        const uint32 *srcab = (const uint32 *)srcB;
        uint32 w4 = (w + 3) >> 2;
        uint32 *elabuf = (uint32 *)tempBuf;
        uint32 *topbuf = elabuf + 5*w4;
        uint32 *botbuf = topbuf + w4 + 8;

        topbuf[0] = topbuf[1] = topbuf[2] = topbuf[3] = srcat[0];
        botbuf[0] = botbuf[1] = botbuf[2] = botbuf[3] = srcab[0];

        for(uint32 x=0; x<w4; ++x) {
            topbuf[x+4] = srcat[x];
            botbuf[x+4] = srcab[x];
        }

        topbuf[w4+4] = topbuf[w4+5] = topbuf[w4+6] = topbuf[w4+7] = topbuf[w4+3];
        botbuf[w4+4] = botbuf[w4+5] = botbuf[w4+6] = botbuf[w4+7] = botbuf[w4+3];

        ela_X8R8G8B8_scalar(elabuf, (const uint8 *)topbuf, (const uint8 *)botbuf, w4);
        nela_X8R8G8B8_scalar((uint32 *)dst, elabuf, (const uint8 *)(topbuf + 4), (const uint8 *)(botbuf + 4), w4);
    }

#if defined(VD_CPU_X86)
    void BlendScanLine_NELA_X8R8G8B8_MMX(void *dst, const void *srcT, const void *srcB, uint32 w, void *tempBuf) {
        const uint32 *srcat = (const uint32 *)srcT;
        const uint32 *srcab = (const uint32 *)srcB;
        uint32 w4 = (w + 3) >> 2;
        uint32 *elabuf = (uint32 *)tempBuf;
        uint32 *topbuf = elabuf + 5*w4;
        uint32 *botbuf = topbuf + w4 + 8;

        topbuf[0] = topbuf[1] = topbuf[2] = topbuf[3] = srcat[0];
        botbuf[0] = botbuf[1] = botbuf[2] = botbuf[3] = srcab[0];

        for(uint32 x=0; x<w4; ++x) {
            topbuf[x+4] = srcat[x];
            botbuf[x+4] = srcab[x];
        }

        topbuf[w4+4] = topbuf[w4+5] = topbuf[w4+6] = topbuf[w4+7] = topbuf[w4+3];
        botbuf[w4+4] = botbuf[w4+5] = botbuf[w4+6] = botbuf[w4+7] = botbuf[w4+3];

        ela_X8R8G8B8_MMX(elabuf, (const uint8 *)topbuf, (const uint8 *)botbuf, w4);
        nela_X8R8G8B8_scalar((uint32 *)dst, elabuf, (const uint8 *)(topbuf + 4), (const uint8 *)(botbuf + 4), w4);
    }

    void BlendScanLine_NELA_MMX_ISSE(void *dst, const void *srcT, const void *srcB, uint32 w, void *tempBuf) {
        const uint32 *srcat = (const uint32 *)srcT;
        const uint32 *srcab = (const uint32 *)srcB;
        uint32 w16 = (w + 15) >> 4;
        uint32 w4 = w16 * 4;
        uint32 *elabuf = (uint32 *)tempBuf;
        uint32 *topbuf = elabuf + 40*w16;
        uint32 *botbuf = topbuf + w4 + 8;

        uint32 woffset = w & 15;
        topbuf[0] = topbuf[1] = topbuf[2] = topbuf[3] = (srcat[0] & 0xff) * 0x01010101;
        botbuf[0] = botbuf[1] = botbuf[2] = botbuf[3] = (srcab[0] & 0xff) * 0x01010101;

        for(uint32 x=0; x<w4; ++x) {
            topbuf[x+4] = srcat[x];
            botbuf[x+4] = srcab[x];
        }

        if (woffset) {
            uint8 *topfinal = (uint8 *)&topbuf[w4] + woffset;
            uint8 *botfinal = (uint8 *)&botbuf[w4] + woffset;
            const uint8 tv = topfinal[-1];
            const uint8 bv = botfinal[-1];

            for(uint32 i = woffset; i < 16; ++i) {
                *topfinal++ = tv;
                *botfinal++ = bv;
            }
        }

        topbuf[w4+4] = topbuf[w4+5] = topbuf[w4+6] = topbuf[w4+7] = topbuf[w4+3];
        botbuf[w4+4] = topbuf[w4+5] = topbuf[w4+6] = topbuf[w4+7] = botbuf[w4+3];

        if (ISSE_enabled) {
            ela_L8_ISSE(elabuf, topbuf, botbuf, w16);
            nela_L8_ISSE(dst, elabuf, w16);
        } else {
            ela_L8_MMX(elabuf, topbuf, botbuf, w16);
            nela_L8_MMX(dst, elabuf, w16);
        }
    }
#endif

    void BlendScanLine_NELA_SSE2(void *dst, const void *srcT, const void *srcB, uint32 w, __m128i *tempBuf) {
        const __m128i *srcat = (const __m128i *)srcT;
        const __m128i *srcab = (const __m128i *)srcB;
        uint32 w16 = (w + 15) >> 4;
        __m128i *elabuf = tempBuf;
        __m128i *topbuf = elabuf + 10*w16;
        __m128i *botbuf = topbuf + w16 + 2;

        uint32 woffset = w & 15;
        topbuf[0] = srcat[0];
        botbuf[0] = srcab[0];

        for(uint32 x=0; x<w16; ++x) {
            topbuf[x+1] = srcat[x];
            botbuf[x+1] = srcab[x];
        }

        if (woffset) {
            uint8 *topfinal = (uint8 *)&topbuf[w16] + woffset;
            uint8 *botfinal = (uint8 *)&botbuf[w16] + woffset;
            const uint8 tv = topfinal[-1];
            const uint8 bv = botfinal[-1];

            for(uint32 i = woffset; i < 16; ++i) {
                *topfinal++ = tv;
                *botfinal++ = bv;
            }
        }

        topbuf[w16+1] = topbuf[w16];
        botbuf[w16+1] = botbuf[w16];

        ela_L8_SSE2(elabuf, topbuf, botbuf, w16);
        nela_L8_SSE2((__m128i *)dst, elabuf, w16);
    }

    void InterpPlane_NELA_X8R8G8B8(void *dst, ptrdiff_t dstpitch, const void *src, ptrdiff_t srcpitch, uint32 w, uint32 h, bool interpField2) {
        uint32 w16 = (w + 15) >> 4;
        vdfastvector<uint8, vdaligned_alloc<uint8> > tempbuf((12 * w16 + 4) * 16);
        void *elabuf = tempbuf.data();

        if (!interpField2)
            memcpy(dst, src, w16 << 4);

        int y0 = interpField2 ? 1 : 2;
        for(uint32 y = y0; y < h - 1; y += 2) {
            const __m128i *srcat = (const __m128i *)((const char *)src + srcpitch * (y-1));
            const __m128i *srcab = (const __m128i *)((const char *)src + srcpitch * (y+1));

#if defined(VD_CPU_X86)
            if (MMX_enabled)
                BlendScanLine_NELA_X8R8G8B8_MMX((char *)dst + dstpitch*y, srcat, srcab, w, (uint8 *)elabuf);
            else
#endif
                BlendScanLine_NELA_X8R8G8B8_scalar((char *)dst + dstpitch*y, srcat, srcab, w, (uint8 *)elabuf);
        }

        if (interpField2)
            memcpy((char *)dst + dstpitch*(h - 1), (const char *)src + srcpitch*(h - 1), w16 << 4);
    }

    void InterpPlane_NELA(void *dst, ptrdiff_t dstpitch, const void *src, ptrdiff_t srcpitch, uint32 w, uint32 h, bool interpField2) {
        uint32 w16 = (w + 15) >> 4;
        vdfastvector<uint8, vdaligned_alloc<uint8> > tempbuf((12 * w16 + 4) * 16);
        void *elabuf = tempbuf.data();

        if (!interpField2)
            memcpy(dst, src, w16 << 4);

        int y0 = interpField2 ? 1 : 2;
        if (SSE2_enabled) {
            for(uint32 y = y0; y < h - 1; y += 2) {
                const __m128i *srcat = (const __m128i *)((const char *)src + srcpitch * (y-1));
                const __m128i *srcab = (const __m128i *)((const char *)src + srcpitch * (y+1));

                BlendScanLine_NELA_SSE2((char *)dst + dstpitch*y, srcat, srcab, w, (__m128i *)elabuf);
            }
        }
#if defined(VD_CPU_X86)
        else if (MMX_enabled || ISSE_enabled) {
            for(uint32 y = y0; y < h - 1; y += 2) {
                const __m128i *srcat = (const __m128i *)((const char *)src + srcpitch * (y-1));
                const __m128i *srcab = (const __m128i *)((const char *)src + srcpitch * (y+1));

                BlendScanLine_NELA_MMX_ISSE((char *)dst + dstpitch*y, srcat, srcab, w, (uint8 *)elabuf);
            }
        }
#endif
        else {
            for(uint32 y = y0; y < h - 1; y += 2) {
                const __m128i *srcat = (const __m128i *)((const char *)src + srcpitch * (y-1));
                const __m128i *srcab = (const __m128i *)((const char *)src + srcpitch * (y+1));

                BlendScanLine_NELA_scalar((char *)dst + dstpitch*y, srcat, srcab, w, (uint8 *)elabuf);
            }
        }

        if (interpField2)
            memcpy((char *)dst + dstpitch*(h - 1), (const char *)src + srcpitch*(h - 1), w16 << 4);
    }

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
            push        ebp
            push        edi
            push        esi
            push        ebx

            mov         esi, [esp+24+16]
            mov         eax, [esp+4+16]
            shl         esi, 4
            mov         ecx, [esp+12+16]
            mov         edx, [esp+16+16]
            mov         ebp, [esp+20+16]
            mov         edi, [esp+8+16]
            sub         edi, esi
            sub         ebp, esi

            movq        mm6, x7fb
            movq        mm7, xfeb

            mov         esi, [esp+28+16]
yloop:
            mov         ebx, [esp+24+16]
mainRowLoop:
            movq        mm0, [ecx]
            movq        mm3, [ecx + 8]
            movq        mm1, mm0
            movq        mm2, [edx]
            movq        mm4, mm3
            movq        mm5, [edx + 8]
            por         mm1, mm2
            pxor        mm0, mm2
            por         mm4, mm5
            pxor        mm3, mm5
            psrlq       mm0, 1
            pand        mm3, mm7
            pand        mm0, mm6
            psrlq       mm3, 1
            psubb       mm1, mm0
            psubb       mm4, mm3
            add         ecx, 16
            movq        [eax], mm1
            movq        [eax+8], mm4
            add         edx, 16
            add         eax, 16
            dec         ebx
            jne         mainRowLoop

            add         eax, edi
            add         ecx, ebp
            add         edx, ebp
            dec         esi
            jne         yloop

            emms
            pop         ebx
            pop         esi
            pop         edi
            pop         ebp
            ret
        }
    }

    void __declspec(naked) __cdecl Average_ISSE(void *dst, ptrdiff_t dstPitch, const void *src1, const void *src2, ptrdiff_t srcPitch, uint32 w16, uint32 h) {
        static const __declspec(align(8)) uint64 x7fb = 0x7f7f7f7f7f7f7f7f;
        static const __declspec(align(8)) uint64 xfeb = 0xfefefefefefefefe;

        __asm {
            push        ebp
            push        edi
            push        esi
            push        ebx

            mov         esi, [esp+24+16]
            mov         eax, [esp+4+16]
            shl         esi, 4
            mov         ecx, [esp+12+16]
            mov         edx, [esp+16+16]
            mov         ebp, [esp+20+16]
            mov         edi, [esp+8+16]
            sub         edi, esi
            sub         ebp, esi

            movq        mm6, x7fb
            movq        mm7, xfeb

            mov         esi, [esp+28+16]
yloop:
            mov         ebx, [esp+24+16]
mainRowLoop:
            movq        mm0, [ecx]
            movq        mm1, [ecx + 8]
            movq        mm2, [edx]
            movq        mm3, [edx + 8]
            pavgb       mm0, mm2
            pavgb       mm1, mm3
            movq        [eax], mm0
            add         ecx, 16
            add         edx, 16
            movq        [eax+8], mm1
            add         eax, 16
            dec         ebx
            jne         mainRowLoop

            add         eax, edi
            add         ecx, ebp
            add         edx, ebp
            dec         esi
            jne         yloop

            emms
            pop         ebx
            pop         esi
            pop         edi
            pop         ebp
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

        uint32 y0 = interpField2 ? 1 : 2;

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
        if (SSE2_enabled && !(srcpitch % 16))
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

void DeinterlaceELA_X8R8G8B8(BYTE* dst, BYTE* src, DWORD w, DWORD h, DWORD dstpitch, DWORD srcpitch, bool topfield)
{
    topfield = !topfield;

    InterpPlane_NELA_X8R8G8B8(dst, dstpitch, src, srcpitch, w, h, topfield);
}

void DeinterlaceELA(BYTE* dst, BYTE* src, DWORD w, DWORD h, DWORD dstpitch, DWORD srcpitch, bool topfield)
{
    topfield = !topfield;

    InterpPlane_NELA(dst, dstpitch, src, srcpitch, w, h, topfield);
}

void DeinterlaceBob(BYTE* dst, BYTE* src, DWORD w, DWORD h, DWORD dstpitch, DWORD srcpitch, bool topfield)
{
    topfield = !topfield;

    InterpPlane_Bob(dst, dstpitch, src, srcpitch, w, h, topfield);
}

void DeinterlaceBlend(BYTE* dst, BYTE* src, DWORD w, DWORD h, DWORD dstpitch, DWORD srcpitch)
{
    BlendPlane(dst, dstpitch, src, srcpitch, w, h);
}
