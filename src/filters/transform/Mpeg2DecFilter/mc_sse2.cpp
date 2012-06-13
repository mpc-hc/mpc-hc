/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


//  Based on Intel's AP-942

#include "stdafx.h"
#include <inttypes.h>
#include "libmpeg2.h"
#include "attributes.h"
#include "../../../DSUtil/simd.h"

static const __m128i const_1_16_bytes = _mm_set1_epi16(1);

static void MC_put_o_16_sse2(uint8_t* ecx, const uint8_t* edx, const int eax, int esi)
{
    const int edi = eax + eax;
    const int ebx = edi + eax;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        __m128i xmm0, xmm1, xmm2, xmm3;
        movdqu(xmm0,  edx);
        movdqu(xmm1,  edx + eax);
        movdqu(xmm2,  edx + edi);
        movdqu(xmm3,  edx + ebx);
        movdqa(ecx, xmm0);
        movdqa(ecx + eax, xmm1);
        movdqa(ecx + edi, xmm2);
        movdqa(ecx + ebx, xmm3);
    }
}

static void MC_put_o_8_sse2(uint8_t* ecx, const uint8_t* edx, const int eax, int esi)
{
    const int edi = eax + eax;
    const int ebx = edi + eax;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        __m128d xmm0, xmm1, xmm2, xmm3;
        movlpd(xmm0, edx);
        movlpd(xmm1, edx + eax);
        movlpd(xmm2, edx + edi);
        movlpd(xmm3, edx + ebx);
        movlpd(ecx, xmm0);
        movlpd(ecx + eax, xmm1);
        movlpd(ecx + edi, xmm2);
        movlpd(ecx + ebx, xmm3);
    }
}

static void MC_put_x_16_sse2(uint8_t* ecx, const uint8_t* edx, const int eax, int esi)
{
    const int edi = eax + eax;
    const int ebx = edi + eax;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
        movdqu(xmm0, edx);
        movdqu(xmm1, edx + 1);
        movdqu(xmm2, edx + eax);
        movdqu(xmm3, edx + eax + 1);
        movdqu(xmm4, edx + edi);
        movdqu(xmm5, edx + edi + 1);
        movdqu(xmm6, edx + ebx);
        movdqu(xmm7, edx + ebx + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        pavgb(xmm4, xmm5);
        pavgb(xmm6, xmm7);
        movdqa(ecx, xmm0);
        movdqa(ecx + eax, xmm2);
        movdqa(ecx + edi, xmm4);
        movdqa(ecx + ebx, xmm6);
    }
}

static void MC_put_x_8_sse2(uint8_t* ecx, const uint8_t* edx, const int eax, int esi)
{
    const int edi = eax + eax;
    const int ebx = edi + eax;
    __m128i xmm0, xmm1, xmm2, xmm3;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        movlpd(xmm0, edx);
        movlpd(xmm1, edx + 1);
        movhpd(xmm0, edx + eax);
        movhpd(xmm1, edx + eax + 1);
        movlpd(xmm2, edx + edi);
        movlpd(xmm3, edx + edi + 1);
        movhpd(xmm2, edx + ebx);
        movhpd(xmm3, edx + ebx + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        movlpd(ecx, xmm0);
        movhpd(ecx + eax, xmm0);
        movlpd(ecx + edi, xmm2);
        movhpd(ecx + ebx, xmm2);
    }
}

static void MC_put_y_16_sse2(uint8_t* ecx, const uint8_t* edx, const int eax, int esi)
{
    const int edi = eax + eax;
    const int ebx = edi + eax;
    __m128i xmm0;
    movdqu(xmm0, edx);
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        __m128i xmm1, xmm2, xmm3, xmm4;
        movdqu(xmm1, edx + eax);
        movdqu(xmm2, edx + edi);
        movdqu(xmm3, edx + ebx);
        movdqu(xmm4, edx + edi * 2);
        pavgb(xmm0, xmm1);
        pavgb(xmm1, xmm2);
        pavgb(xmm2, xmm3);
        pavgb(xmm3, xmm4);
        movdqa(ecx, xmm0);
        movdqa(ecx + eax, xmm1);
        movdqa(ecx + edi, xmm2);
        movdqa(ecx + ebx, xmm3);
        movdqa(xmm0, xmm4);
    }
}

static void MC_put_y_8_sse2(uint8_t* ecx, const uint8_t* edx, const int eax, int esi)
{
    const int edi = eax + eax;
    const int ebx = edi + eax;
    __m128i xmm0;
    movlpd(xmm0, edx);

    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        __m128i xmm1, xmm2, xmm3, xmm4;
        movlpd(xmm1, edx + eax);
        movlpd(xmm2, edx + edi);
        movlpd(xmm3, edx + ebx);
        movlpd(xmm4, edx + edi * 2);
        pavgb(xmm0, xmm1);
        pavgb(xmm1, xmm2);
        pavgb(xmm2, xmm3);
        pavgb(xmm3, xmm4);
        movlpd(ecx, xmm0);
        movlpd(ecx + eax, xmm1);
        movlpd(ecx + edi, xmm2);
        movlpd(ecx + ebx, xmm3);
        movdqa(xmm0, xmm4);
    }
}

static void MC_put_xy_16_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref ;
    uint8_t*  ecx = dest;
    int eax = stride;
    int esi = height;
    int edi = eax + eax;
    __m128i xmm7, xmm0, xmm1, xmm4, xmm5, xmm2, xmm3;
    movdqa(xmm7, const_1_16_bytes);
    movdqu(xmm0, edx);
    movdqu(xmm1, edx + 1);
    for (; esi; edx += edi, ecx += edi, esi -= 2) {
        movdqu(xmm2, edx + eax);
        movdqu(xmm3, edx + eax + 1);
        movdqu(xmm4, edx + edi);
        movdqu(xmm5, edx + edi + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        movdqa(xmm1, xmm5);
        pavgb(xmm5, xmm4);
        psubusb(xmm2, xmm7);
        pavgb(xmm0, xmm2);
        pavgb(xmm2, xmm5);
        movdqa(ecx, xmm0);
        movdqa(xmm0, xmm4);
        movdqa(ecx + eax, xmm2);
    }
}

static void MC_put_xy_8_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int eax = stride;
    int esi = height;
    int edi = eax + eax;
    __m128i xmm7, xmm0, xmm2, xmm1, xmm3, xmm4, xmm5;
    movdqa(xmm7, const_1_16_bytes);
    movlpd(xmm0, edx);
    movlpd(xmm1, edx + 1);
    for (; esi; edx += edi, ecx += edi, esi -= 2) {
        movlpd(xmm2, edx + eax);
        movlpd(xmm3, edx + eax + 1);
        movlpd(xmm4, edx + edi);
        movlpd(xmm5, edx + edi + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        movdqa(xmm1, xmm5);
        pavgb(xmm5, xmm4);
        psubusb(xmm2, xmm7);
        pavgb(xmm0, xmm2);
        pavgb(xmm2, xmm5);
        movlpd(ecx, xmm0);
        movdqa(xmm0, xmm4);
        movlpd(ecx + eax, xmm2);
    }
}

static void MC_avg_o_16_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    int ebx = edi + eax;

    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        __m128i xmm0, xmm1, xmm2, xmm3;
        movdqu(xmm0, edx);
        movdqu(xmm1, edx + eax);
        movdqu(xmm2, edx + edi);
        movdqu(xmm3, edx + ebx);
        pavgb(xmm0, ecx);
        pavgb(xmm1, ecx + eax);
        pavgb(xmm2, ecx + edi);
        pavgb(xmm3, ecx + ebx);
        movdqa(ecx, xmm0);
        movdqa(ecx + eax, xmm1);
        movdqa(ecx + edi, xmm2);
        movdqa(ecx + ebx, xmm3);
    }
}

static void MC_avg_o_8_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    int ebx = edi + eax;

    __m128i xmm0, xmm1, xmm2, xmm3;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        movlpd(xmm0, edx);
        movhpd(xmm0, edx + eax);
        movlpd(xmm2, edx + edi);
        movhpd(xmm2, edx + ebx);
        movlpd(xmm1, ecx);
        movhpd(xmm1, ecx + eax);
        movlpd(xmm3, ecx + edi);
        movhpd(xmm3, ecx + ebx);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        movlpd(ecx, xmm0);
        movhpd(ecx + eax, xmm0);
        movlpd(ecx + edi, xmm2);
        movhpd(ecx + ebx, xmm2);
    }
}

static void MC_avg_x_16_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    int ebx = edi + eax;

    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        movdqu(xmm0, edx);
        movdqu(xmm1, edx + 1);
        movdqu(xmm2, edx + eax);
        movdqu(xmm3, edx + eax + 1);
        movdqu(xmm4, edx + edi);
        movdqu(xmm5, edx + edi + 1);
        movdqu(xmm6, edx + ebx);
        movdqu(xmm7, edx + ebx + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        pavgb(xmm4, xmm5);
        pavgb(xmm6, xmm7);
        pavgb(xmm0, ecx);
        pavgb(xmm2, ecx + eax);
        pavgb(xmm4, ecx + edi);
        pavgb(xmm6, ecx + ebx);
        movdqa(ecx, xmm0);
        movdqa(ecx + eax, xmm2);
        movdqa(ecx + edi, xmm4);
        movdqa(ecx + ebx, xmm6);
    }
}

static void MC_avg_x_8_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    int ebx = edi + eax;

    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        movlpd(xmm0, edx);
        movlpd(xmm1, edx + 1);
        movhpd(xmm0, edx + eax);
        movhpd(xmm1, edx + eax + 1);
        movlpd(xmm2, edx + edi);
        movlpd(xmm3, edx + edi + 1);
        movhpd(xmm2, edx + ebx);
        movhpd(xmm3, edx + ebx + 1);
        movlpd(xmm4, ecx);
        movhpd(xmm4, ecx + eax);
        movlpd(xmm5, ecx + edi);
        movhpd(xmm5, ecx + ebx);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        pavgb(xmm0, xmm4);
        pavgb(xmm2, xmm5);
        movlpd(ecx, xmm0);
        movhpd(ecx + eax, xmm0);
        movlpd(ecx + edi, xmm2);
        movhpd(ecx + ebx, xmm2);
    }
}

static void MC_avg_y_16_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    int ebx = edi + eax;
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4;

    movdqu(xmm0, edx);
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        movdqu(xmm1, edx + eax);
        movdqu(xmm2, edx + edi);
        movdqu(xmm3, edx + ebx);
        movdqu(xmm4, edx + edi * 2);
        pavgb(xmm0, xmm1);
        pavgb(xmm1, xmm2);
        pavgb(xmm2, xmm3);
        pavgb(xmm3, xmm4);
        pavgb(xmm0, ecx);
        pavgb(xmm1, ecx + eax);
        pavgb(xmm2, ecx + edi);
        pavgb(xmm3, ecx + ebx);
        movdqa(ecx, xmm0);
        movdqa(ecx + eax, xmm1);
        movdqa(ecx + edi, xmm2);
        movdqa(ecx + ebx, xmm3);
        movdqa(xmm0, xmm4);
    }
}

static void MC_avg_y_8_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    int ebx = edi + eax;
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
    movhpd(xmm0, edx);
    movlpd(xmm0, edx + eax);
    for (; esi; edx += edi * 2, ecx += edi * 2, esi -= 4) {
        movlhps(xmm1, xmm0);
        movlpd(xmm1, edx + edi);
        movlhps(xmm2, xmm1);
        movlpd(xmm2, edx + ebx);
        movlhps(xmm3, xmm2);
        movlpd(xmm3, edx + edi * 2);
        movhpd(xmm4, ecx);
        movlpd(xmm4, ecx + eax);
        movhpd(xmm5, ecx + edi);
        movlpd(xmm5, ecx + ebx);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        pavgb(xmm0, xmm4);
        pavgb(xmm2, xmm5);
        movhpd(ecx, xmm0);
        movlpd(ecx + eax, xmm0);
        movhpd(ecx + edi, xmm2);
        movlpd(ecx + ebx, xmm2);
        movdqa(xmm0, xmm3);
    }
}

static void MC_avg_xy_16_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    __m128i xmm7, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;
    movdqa(xmm7, const_1_16_bytes);
    movdqu(xmm0, edx);
    movdqu(xmm1, edx + 1);
    for (; esi; edx += edi, ecx += edi, esi -= 2) {
        movdqu(xmm2, edx + eax);
        movdqu(xmm3, edx + eax + 1);
        movdqu(xmm4, edx + edi);
        movdqu(xmm5, edx + edi + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        movdqa(xmm1, xmm5);
        pavgb(xmm5, xmm4);
        psubusb(xmm2, xmm7);
        pavgb(xmm0, xmm2);
        pavgb(xmm2, xmm5);
        pavgb(xmm0, ecx);
        pavgb(xmm2, ecx + eax);
        movdqa(ecx, xmm0);
        movdqa(xmm0, xmm4);
        movdqa(ecx + eax, xmm2);
    }
}

static void MC_avg_xy_8_sse2(uint8_t* dest, const uint8_t* ref, const int stride, int height)
{
    const uint8_t* edx = ref;
    uint8_t* ecx = dest;
    int esi = height;
    int eax = stride;
    int edi = eax + eax;
    __m128i xmm7, xmm0, xmm2, xmm1, xmm3, xmm4;
    movdqa(xmm7, const_1_16_bytes);
    movhpd(xmm0, edx);
    movlpd(xmm0, edx + eax);
    movhpd(xmm2, edx + 1);
    movlpd(xmm2, edx + eax + 1);
    for (; esi; edx += edi, ecx += edi, esi -= 2) {

        movhpd(xmm1, edx + eax);
        movlpd(xmm1, edx + edi);
        movhpd(xmm3, edx + eax + 1);
        movlpd(xmm3, edx + edi + 1);
        pavgb(xmm0, xmm1);
        pavgb(xmm2, xmm3);
        psubusb(xmm0, xmm7);
        pavgb(xmm0, xmm2);
        movhpd(xmm4, ecx);
        movlpd(xmm4, ecx + eax);
        pavgb(xmm0, xmm4);
        movhpd(ecx, xmm0);
        movlpd(ecx + eax, xmm0);
        movdqa(xmm0, xmm1);
        movdqa(xmm2, xmm3);
    }
}

mpeg2_mc_t mpeg2_mc_sse2 = {
    {
        MC_put_o_16_sse2, MC_put_x_16_sse2, MC_put_y_16_sse2, MC_put_xy_16_sse2,
        MC_put_o_8_sse2,  MC_put_x_8_sse2,  MC_put_y_8_sse2,  MC_put_xy_8_sse2
    },
    {
        MC_avg_o_16_sse2, MC_avg_x_16_sse2, MC_avg_y_16_sse2, MC_avg_xy_16_sse2,
        MC_avg_o_8_sse2,  MC_avg_x_8_sse2,  MC_avg_y_8_sse2,  MC_avg_xy_8_sse2
    }
};
