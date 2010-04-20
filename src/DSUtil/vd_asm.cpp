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
//
//  Notes: 
//  - BitBltFromI420ToRGB is from VirtualDub
//  - sse2 yv12 to yuy2 conversion by Haali
//	(- vd.cpp/h should be renamed to something more sensible already :)


#include "stdafx.h"
#include "vd_asm.h"

#pragma warning(disable : 4799) // no emms... blahblahblah

#ifndef _WIN64
static void __declspec(naked) yuvtoyuy2row_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width)
{
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width

		shr		ecx, 3

yuvtoyuy2row_loop:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		dec		ecx
		jnz		yuvtoyuy2row_loop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) yuvtoyuy2row_avg_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
	static const __int64 mask = 0x7f7f7f7f7f7f7f7fi64;

	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		movq	mm7, mask

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width
		mov		eax, [esp+40] // pitchuv

		shr		ecx, 3

yuvtoyuy2row_avg_loop:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]
		movq		mm1, mm0

		movd		mm2, [ebx + eax]
		punpcklbw	mm2, [esi + eax]
		movq		mm3, mm2

		// (x+y)>>1 == (x&y)+((x^y)>>1)

		pand		mm0, mm2
		pxor		mm1, mm3
		psrlq		mm1, 1
		pand		mm1, mm7
		paddb		mm0, mm1

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		dec		ecx
		jnz		yuvtoyuy2row_avg_loop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) yv12_yuy2_row_sse2() {
  __asm {
    // ebx - Y
    // edx - U
    // esi - V
    // edi - dest
    // ecx - halfwidth
    xor     eax, eax

one:
    movdqa  xmm0, [ebx + eax*2]    // YYYYYYYY
    movdqa  xmm1, [ebx + eax*2 + 16]    // YYYYYYYY

    movdqa  xmm2, [edx + eax]      // UUUUUUUU
    movdqa  xmm3, [esi + eax]      // VVVVVVVV

    movdqa  xmm4, xmm2
    movdqa  xmm5, xmm0
    movdqa  xmm6, xmm1
    punpcklbw xmm2, xmm3          // VUVUVUVU
    punpckhbw xmm4, xmm3          // VUVUVUVU

    punpcklbw xmm0, xmm2          // VYUYVYUY
    punpcklbw xmm1, xmm4
    punpckhbw xmm5, xmm2
    punpckhbw xmm6, xmm4

    movntdq [edi + eax*4], xmm0
    movntdq [edi + eax*4 + 16], xmm5
    movntdq [edi + eax*4 + 32], xmm1
    movntdq [edi + eax*4 + 48], xmm6

    add     eax, 16
    cmp     eax, ecx

    jb      one

    ret
  };
}

static void __declspec(naked) yv12_yuy2_row_sse2_linear() {
  __asm {
    // ebx - Y
    // edx - U
    // esi - V
    // edi - dest
    // ecx - width
    // ebp - uv_stride
    xor     eax, eax

one:
    movdqa  xmm0, [ebx + eax*2]    // YYYYYYYY
    movdqa  xmm1, [ebx + eax*2 + 16]    // YYYYYYYY

    movdqa  xmm2, [edx]
    movdqa  xmm3, [esi]
    pavgb   xmm2, [edx + ebp]      // UUUUUUUU
    pavgb   xmm3, [esi + ebp]      // VVVVVVVV

    movdqa  xmm4, xmm2
    movdqa  xmm5, xmm0
    movdqa  xmm6, xmm1
    punpcklbw xmm2, xmm3          // VUVUVUVU
    punpckhbw xmm4, xmm3          // VUVUVUVU

    punpcklbw xmm0, xmm2          // VYUYVYUY
    punpcklbw xmm1, xmm4
    punpckhbw xmm5, xmm2
    punpckhbw xmm6, xmm4

    movntdq [edi + eax*4], xmm0
    movntdq [edi + eax*4 + 16], xmm5
    movntdq [edi + eax*4 + 32], xmm1
    movntdq [edi + eax*4 + 48], xmm6

    add     eax, 16
    add     edx, 16
    add     esi, 16
    cmp     eax, ecx

    jb      one

    ret
  };
}

static void __declspec(naked) yv12_yuy2_row_sse2_linear_interlaced() {
  __asm {
    // ebx - Y
    // edx - U
    // esi - V
    // edi - dest
    // ecx - width
    // ebp - uv_stride
    xor     eax, eax

one:
    movdqa  xmm0, [ebx + eax*2]    // YYYYYYYY
    movdqa  xmm1, [ebx + eax*2 + 16]    // YYYYYYYY

    movdqa  xmm2, [edx]
    movdqa  xmm3, [esi]
    pavgb   xmm2, [edx + ebp*2]      // UUUUUUUU
    pavgb   xmm3, [esi + ebp*2]      // VVVVVVVV

    movdqa  xmm4, xmm2
    movdqa  xmm5, xmm0
    movdqa  xmm6, xmm1
    punpcklbw xmm2, xmm3          // VUVUVUVU
    punpckhbw xmm4, xmm3          // VUVUVUVU

    punpcklbw xmm0, xmm2          // VYUYVYUY
    punpcklbw xmm1, xmm4
    punpckhbw xmm5, xmm2
    punpckhbw xmm6, xmm4

    movntdq [edi + eax*4], xmm0
    movntdq [edi + eax*4 + 16], xmm5
    movntdq [edi + eax*4 + 32], xmm1
    movntdq [edi + eax*4 + 48], xmm6

    add     eax, 16
    add     edx, 16
    add     esi, 16
    cmp     eax, ecx

    jb      one

    ret
  };
}

void __declspec(naked) yv12_yuy2_sse2(const BYTE *Y, const BYTE *U, const BYTE *V,
    int halfstride, unsigned halfwidth, unsigned height,
    BYTE *YUY2, int d_stride)
{
  __asm {
    push    ebx
    push    esi
    push    edi
    push    ebp

    mov     ebx, [esp + 20] // Y
    mov     edx, [esp + 24] // U
    mov     esi, [esp + 28] // V
    mov     edi, [esp + 44] // D
    mov     ebp, [esp + 32] // uv_stride
    mov     ecx, [esp + 36] // uv_width

    mov     eax, ecx
    add     eax, 15
    and     eax, 0xfffffff0
    sub     [esp + 32], eax

    cmp     dword ptr [esp + 40], 2
    jbe     last2

row:
    sub     dword ptr [esp + 40], 2
    call    yv12_yuy2_row_sse2

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    call    yv12_yuy2_row_sse2_linear

    add     edx, [esp + 32]
    add     esi, [esp + 32]

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    cmp     dword ptr [esp + 40], 2
    ja      row

last2:
    call    yv12_yuy2_row_sse2

    dec     dword ptr [esp + 40]
    jz      done

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]
    call    yv12_yuy2_row_sse2
done:

    pop     ebp
    pop     edi
    pop     esi
    pop     ebx

    ret
  };
}

void __declspec(naked) yv12_yuy2_sse2_interlaced(const BYTE *Y, const BYTE *U, const BYTE *V,
    int halfstride, unsigned halfwidth, unsigned height,
    BYTE *YUY2, int d_stride)
{
  __asm {
    push    ebx
    push    esi
    push    edi
    push    ebp

    mov     ebx, [esp + 20] // Y
    mov     edx, [esp + 24] // U
    mov     esi, [esp + 28] // V
    mov     edi, [esp + 44] // D
    mov     ebp, [esp + 32] // uv_stride
    mov     ecx, [esp + 36] // uv_width

    mov     eax, ecx
    add     eax, 15
    and     eax, 0xfffffff0
    sub     [esp + 32], eax

    cmp     dword ptr [esp + 40], 4
    jbe     last4

row:
    sub     dword ptr [esp + 40], 4
    call    yv12_yuy2_row_sse2	// first row, first field

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    add	    edx, ebp
    add	    esi, ebp

    call    yv12_yuy2_row_sse2	// first row, second field

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    sub	    edx, ebp
    sub	    esi, ebp

    call    yv12_yuy2_row_sse2_linear_interlaced // second row, first field

    add     edx, [esp + 32]
    add     esi, [esp + 32]

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    call    yv12_yuy2_row_sse2_linear_interlaced // second row, second field

    add     edx, [esp + 32]
    add     esi, [esp + 32]

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    cmp     dword ptr [esp + 40], 4
    ja      row

last4:
    call    yv12_yuy2_row_sse2

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    add     edx, ebp
    add     esi, ebp

    call    yv12_yuy2_row_sse2

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    sub     edx, ebp
    sub     esi, ebp

    call    yv12_yuy2_row_sse2

    lea     ebx, [ebx + ebp*2]
    add     edi, [esp + 48]

    add     edx, ebp
    add     esi, ebp

    call    yv12_yuy2_row_sse2

    pop     ebp
    pop     edi
    pop     esi
    pop     ebx

    ret
  };
}

static void __declspec(naked) asm_blend_row_clipped_MMX(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch)
{
	static const __int64 _x0001000100010001 = 0x0001000100010001;

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

		shr		ebp, 3

		movq	mm6, _x0001000100010001
		pxor	mm7, mm7

xloop:
		movq		mm0, [esi]
		movq		mm3, mm0
		punpcklbw	mm0, mm7
		punpckhbw	mm3, mm7

		movq		mm1, [esi+edx]
		movq		mm4, mm1
		punpcklbw	mm1, mm7
		punpckhbw	mm4, mm7

		paddw		mm1, mm0
		paddw		mm1, mm6
		psrlw		mm1, 1

		paddw		mm4, mm3
		paddw		mm4, mm6
		psrlw		mm4, 1

		add			esi, 8
		packuswb	mm1, mm4
		movq		[edi+esi-8], mm1

		dec		ebp
		jne		xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) asm_blend_row_MMX(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch)
{
	static const __int64 mask0 = 0xfcfcfcfcfcfcfcfci64;
	static const __int64 mask1 = 0x7f7f7f7f7f7f7f7fi64;
	static const __int64 mask2 = 0x3f3f3f3f3f3f3f3fi64;
	static const __int64 _x0002000200020002 = 0x0002000200020002;

	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi, [esp+20]
		mov		esi, [esp+24]
		sub		edi, esi
		mov		ebp, [esp+28]
		mov		edx, [esp+32]

		shr		ebp, 3

		movq	mm6, _x0002000200020002
		pxor	mm7, mm7

xloop:
		movq		mm0, [esi]
		movq		mm3, mm0
		punpcklbw	mm0, mm7
		punpckhbw	mm3, mm7

		movq		mm1, [esi+edx]
		movq		mm4, mm1
		punpcklbw	mm1, mm7
		punpckhbw	mm4, mm7

		movq		mm2, [esi+edx*2]
		movq		mm5, mm2
		punpcklbw	mm2, mm7
		punpckhbw	mm5, mm7

		psllw		mm1, 1
		paddw		mm1, mm0
		paddw		mm1, mm2
		paddw		mm1, mm6
		psrlw		mm1, 2

		psllw		mm4, 1
		paddw		mm4, mm3
		paddw		mm4, mm5
		paddw		mm4, mm6
		psrlw		mm4, 2

		add			esi, 8
		packuswb	mm1, mm4
		movq		[edi+esi-8], mm1

		dec		ebp
		jne		xloop

		// sadly the original code makes a lot of visible banding artifacts on yuv
		// (it seems those shiftings without rounding introduce too much error)
/*
		mov		edi,[esp+20]
		mov		esi,[esp+24]
		sub		edi,esi
		mov		ebp,[esp+28]
		mov		edx,[esp+32]

		movq	mm5,mask0
		movq	mm6,mask1
		movq	mm7,mask2
		shr		ebp,1
		jz		oddpart

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

oddpart:
		test	byte ptr [esp+28],1
		jz		nooddpart

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
		add		eax,ecx
		mov		[edi+esi],eax

nooddpart:
*/
		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

__declspec(align(16)) static BYTE const_1_16_bytes[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

static void asm_blend_row_SSE2(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch)
{
	__asm
	{
		mov edx, srcpitch
		mov esi, src
		mov edi, dst
		sub edi, esi
		mov ecx, w
		mov ebx, ecx
		shr ecx, 4
		and ebx, 15

		movdqa xmm7, [const_1_16_bytes] 

asm_blend_row_SSE2_loop:
		movdqa xmm0, [esi]
		movdqa xmm1, [esi+edx]
		movdqa xmm2, [esi+edx*2]
		pavgb xmm0, xmm1
		pavgb xmm2, xmm1
		psubusb xmm0, xmm7
		pavgb xmm0, xmm2
		movdqa [esi+edi], xmm0
		add esi, 16
		dec	ecx
		jnz asm_blend_row_SSE2_loop

		test ebx,15
		jz asm_blend_row_SSE2_end

		mov ecx, ebx
		xor ax, ax
		xor bx, bx
		xor dx, dx
asm_blend_row_SSE2_loop2:
		mov al, [esi]
		mov bl, [esi+edx]
		mov dl, [esi+edx*2]
		add ax, bx
		inc ax
		shr ax, 1
		add dx, bx
		inc dx
		shr dx, 1
		add ax, dx
		shr ax, 1
		mov [esi+edi], al
		inc esi
		dec	ecx
		jnz asm_blend_row_SSE2_loop2

asm_blend_row_SSE2_end:
	}
}

static void asm_blend_row_clipped_SSE2(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch)
{
	__asm
	{
		mov edx, srcpitch
		mov esi, src
		mov edi, dst
		sub edi, esi
		mov ecx, w
		mov ebx, ecx
		shr ecx, 4
		and ebx, 15

		movdqa xmm7, [const_1_16_bytes] 

asm_blend_row_clipped_SSE2_loop:
		movdqa xmm0, [esi]
		movdqa xmm1, [esi+edx]
		pavgb xmm0, xmm1
		movdqa [esi+edi], xmm0
		add esi, 16
		dec	ecx
		jnz asm_blend_row_clipped_SSE2_loop

		test ebx,15
		jz asm_blend_row_clipped_SSE2_end

		mov ecx, ebx
		xor ax, ax
		xor bx, bx
asm_blend_row_clipped_SSE2_loop2:
		mov al, [esi]
		mov bl, [esi+edx]
		add ax, bx
		inc ax
		shr ax, 1
		mov [esi+edi], al
		inc esi
		dec	ecx
		jnz asm_blend_row_clipped_SSE2_loop2

asm_blend_row_clipped_SSE2_end:
	}
}
#endif
