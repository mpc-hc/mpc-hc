//  VirtualDub - Video processing and capture application
//  Copyright (C) 1998-2001 Avery Lee
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


void AvgLines8(BYTE* dst, DWORD h, DWORD pitch)
{
    if (h <= 1) {
        return;
    }

    BYTE* s = dst;
    BYTE* d = dst + (h - 2) * pitch;

    for (; s < d; s += pitch * 2) {
        BYTE* tmp = s;

#ifndef _WIN64
        if ((g_cpuid.m_flags & CCpuID::sse2) && !((DWORD)tmp & 0xf) && !((DWORD)pitch & 0xf)) {
            __asm {
                mov     esi, tmp
                mov     ebx, pitch

                mov     ecx, ebx
                shr     ecx, 4

                AvgLines8_sse2_loop:
                movdqa  xmm0, [esi]
                pavgb   xmm0, [esi+ebx*2]
                movdqa  [esi+ebx], xmm0
                add     esi, 16

                dec     ecx
                jnz     AvgLines8_sse2_loop

                mov     tmp, esi
            }

            for (ptrdiff_t i = pitch & 7; i--; tmp++) {
                tmp[pitch] = (tmp[0] + tmp[pitch << 1] + 1) >> 1;
            }
        } else if (g_cpuid.m_flags & CCpuID::mmx) {
            __asm {
                mov     esi, tmp
                mov     ebx, pitch

                mov     ecx, ebx
                shr     ecx, 3

                pxor    mm7, mm7
                AvgLines8_mmx_loop:
                movq    mm0, [esi]
                movq    mm1, mm0

                punpcklbw   mm0, mm7
                punpckhbw   mm1, mm7

                movq    mm2, [esi+ebx*2]
                movq    mm3, mm2

                punpcklbw   mm2, mm7
                punpckhbw   mm3, mm7

                paddw   mm0, mm2
                psrlw   mm0, 1

                paddw   mm1, mm3
                psrlw   mm1, 1

                packuswb    mm0, mm1

                movq    [esi+ebx], mm0

                lea     esi, [esi+8]

                dec     ecx
                jnz     AvgLines8_mmx_loop

                mov     tmp, esi
            }

            for (ptrdiff_t i = pitch & 7; i--; tmp++) {
                tmp[pitch] = (tmp[0] + tmp[pitch << 1] + 1) >> 1;
            }
        } else
#endif
        {
            for (ptrdiff_t i = pitch; i--; tmp++) {
                tmp[pitch] = (tmp[0] + tmp[pitch << 1] + 1) >> 1;
            }
        }
    }

    if (!(h & 1) && h >= 2) {
        dst += (h - 2) * pitch;
        memcpy(dst + pitch, dst, pitch);
    }

#ifndef _WIN64
    __asm emms;
#endif
}

void AvgLines555(BYTE* dst, DWORD h, DWORD pitch)
{
    if (h <= 1) {
        return;
    }

#ifndef _WIN64
    unsigned __int64 __0x03e003e003e003e0 = 0x03e003e003e003e0;
    unsigned __int64 __0x001f001f001f001f = 0x001f001f001f001f;
#endif

    BYTE* s = dst;
    BYTE* d = dst + (h - 2) * pitch;

    for (; s < d; s += pitch * 2) {
        BYTE* tmp = s;

#ifndef _WIN64
        __asm {
            mov     esi, tmp
            mov     ebx, pitch

            mov     ecx, ebx
            shr     ecx, 3

            movq    mm6, __0x03e003e003e003e0
            movq    mm7, __0x001f001f001f001f

            AvgLines555_loop:
            movq    mm0, [esi]
            movq    mm1, mm0
            movq    mm2, mm0

            psrlw   mm0, 10             // red1 bits: mm0 = 001f001f001f001f
            pand    mm1, mm6            // green1 bits: mm1 = 03e003e003e003e0
            pand    mm2, mm7            // blue1 bits: mm2 = 001f001f001f001f

            movq    mm3, [esi+ebx*2]
            movq    mm4, mm3
            movq    mm5, mm3

            psrlw   mm3, 10             // red2 bits: mm3 = 001f001f001f001f
            pand    mm4, mm6            // green2 bits: mm4 = 03e003e003e003e0
            pand    mm5, mm7            // blue2 bits: mm5 = 001f001f001f001f

            paddw   mm0, mm3
            psrlw   mm0, 1              // (red1+red2)/2
            psllw   mm0, 10             // red bits at 7c007c007c007c00

            paddw   mm1, mm4
            psrlw   mm1, 1              // (green1+green2)/2
            pand    mm1, mm6            // green bits at 03e003e003e003e0

            paddw   mm2, mm5
            psrlw   mm2, 1              // (blue1+blue2)/2
            // blue bits at 001f001f001f001f (no need to pand, lower bits were discareded)

            por     mm0, mm1
            por     mm0, mm2

            movq    [esi+ebx], mm0

            lea     esi, [esi+8]

            dec     ecx
            jnz     AvgLines555_loop

            mov     tmp, esi
        }
#endif

        for (ptrdiff_t i = (pitch & 7) >> 1; i--; tmp++) {
            tmp[pitch] =
                ((((*tmp & 0x7c00) + (tmp[pitch << 1] & 0x7c00)) >> 1) & 0x7c00) |
                ((((*tmp & 0x03e0) + (tmp[pitch << 1] & 0x03e0)) >> 1) & 0x03e0) |
                ((((*tmp & 0x001f) + (tmp[pitch << 1] & 0x001f)) >> 1) & 0x001f);
        }
    }

    if (!(h & 1) && h >= 2) {
        dst += (h - 2) * pitch;
        memcpy(dst + pitch, dst, pitch);
    }

#ifndef _WIN64
    __asm emms;
#endif
}

void AvgLines565(BYTE* dst, DWORD h, DWORD pitch)
{
    if (h <= 1) {
        return;
    }

#ifndef _WIN64
    unsigned __int64 __0x07e007e007e007e0 = 0x07e007e007e007e0;
    unsigned __int64 __0x001f001f001f001f = 0x001f001f001f001f;
#endif

    BYTE* s = dst;
    BYTE* d = dst + (h - 2) * pitch;

    for (; s < d; s += pitch * 2) {
        WORD* tmp = (WORD*)s;

#ifndef _WIN64
        __asm {
            mov     esi, tmp
            mov     ebx, pitch

            mov     ecx, ebx
            shr     ecx, 3

            movq    mm6, __0x07e007e007e007e0
            movq    mm7, __0x001f001f001f001f

            AvgLines565_loop:
            movq    mm0, [esi]
            movq    mm1, mm0
            movq    mm2, mm0

            psrlw   mm0, 11             // red1 bits: mm0 = 001f001f001f001f
            pand    mm1, mm6            // green1 bits: mm1 = 07e007e007e007e0
            pand    mm2, mm7            // blue1 bits: mm2 = 001f001f001f001f

            movq    mm3, [esi+ebx*2]
            movq    mm4, mm3
            movq    mm5, mm3

            psrlw   mm3, 11             // red2 bits: mm3 = 001f001f001f001f
            pand    mm4, mm6            // green2 bits: mm4 = 07e007e007e007e0
            pand    mm5, mm7            // blue2 bits: mm5 = 001f001f001f001f

            paddw   mm0, mm3
            psrlw   mm0, 1              // (red1+red2)/2
            psllw   mm0, 11             // red bits at f800f800f800f800

            paddw   mm1, mm4
            psrlw   mm1, 1              // (green1+green2)/2
            pand    mm1, mm6            // green bits at 03e003e003e003e0

            paddw   mm2, mm5
            psrlw   mm2, 1              // (blue1+blue2)/2
            // blue bits at 001f001f001f001f (no need to pand, lower bits were discareded)

            por     mm0, mm1
            por     mm0, mm2

            movq    [esi+ebx], mm0

            lea     esi, [esi+8]

            dec     ecx
            jnz     AvgLines565_loop

            mov     tmp, esi
        }
#else
        for (ptrdiff_t wd = (pitch >> 3); wd--; tmp++) {
            tmp[0] =
                ((((*tmp & 0xf800) + (tmp[pitch << 1] & 0xf800)) >> 1) & 0xf800) |
                ((((*tmp & 0x07e0) + (tmp[pitch << 1] & 0x07e0)) >> 1) & 0x07e0) |
                ((((*tmp & 0x001f) + (tmp[pitch << 1] & 0x001f)) >> 1) & 0x001f);
        }
#endif

        for (ptrdiff_t i = (pitch & 7) >> 1; i--; tmp++) {
            tmp[pitch] =
                ((((*tmp & 0xf800) + (tmp[pitch << 1] & 0xf800)) >> 1) & 0xf800) |
                ((((*tmp & 0x07e0) + (tmp[pitch << 1] & 0x07e0)) >> 1) & 0x07e0) |
                ((((*tmp & 0x001f) + (tmp[pitch << 1] & 0x001f)) >> 1) & 0x001f);
        }
    }

    if (!(h & 1) && h >= 2) {
        dst += (h - 2) * pitch;
        memcpy(dst + pitch, dst, pitch);
    }

#ifndef _WIN64
    __asm emms;
#endif
}
