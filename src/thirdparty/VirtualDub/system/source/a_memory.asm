;	VirtualDub - Video processing and capture application
;	System library component
;	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
;
;	Beginning with 1.6.0, the VirtualDub system library is licensed
;	differently than the remainder of VirtualDub.  This particular file is
;	thus licensed as follows (the "zlib" license):
;
;	This software is provided 'as-is', without any express or implied
;	warranty.  In no event will the authors be held liable for any
;	damages arising from the use of this software.
;
;	Permission is granted to anyone to use this software for any purpose,
;	including commercial applications, and to alter it and redistribute it
;	freely, subject to the following restrictions:
;
;	1.	The origin of this software must not be misrepresented; you must
;		not claim that you wrote the original software. If you use this
;		software in a product, an acknowledgment in the product
;		documentation would be appreciated but is not required.
;	2.	Altered source versions must be plainly marked as such, and must
;		not be misrepresented as being the original software.
;	3.	This notice may not be removed or altered from any source
;		distribution.

		segment	.text

	global	_VDFastMemcpyPartialScalarAligned8
_VDFastMemcpyPartialScalarAligned8:
		mov		eax, [esp+12]
		mov		edx, [esp+4]
		mov		ecx, [esp+8]
		add		ecx, eax
		add		edx, eax
		neg		eax
		jz		.nobytes
		add		eax, 8
		jz		.doodd
		jmp		short .xloop
		align	16
.xloop:
		fild	qword [ecx+eax-8]
		fild	qword [ecx+eax]
		fxch
		fistp	qword [edx+eax-8]
		fistp	qword [edx+eax]
		add		eax,16
		jnc		.xloop
		jnz		.nobytes
.doodd:
		fild	qword [ecx-8]
		fistp	qword [edx-8]
.nobytes:
		ret

	global	_VDFastMemcpyPartialMMX
_VDFastMemcpyPartialMMX:
		push	edi
		push	esi

		mov		edi, [esp+4+8]
		mov		esi, [esp+8+8]
		mov		ecx, [esp+12+8]
		mov		edx, ecx
		shr		ecx, 2
		and		edx, 3
		rep		movsd
		mov		ecx, edx
		rep		movsb
		pop		esi
		pop		edi
		ret

; MPC custom code start
	global	_VDFastMemcpyPartialSSE2
_VDFastMemcpyPartialSSE2:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ecx, [esp+12+16]
		cmp		ecx, 128
		jb		_VDFastMemcpyPartialMMX2.MMX2
		mov		edi, [esp+4+16]
		mov		esi, [esp+8+16]
		mov		eax, edi
		or		eax, esi
		test	al, 15
		jne		SHORT _VDFastMemcpyPartialMMX2.MMX2

		shr		ecx, 7
.loop128:
		prefetchnta	[esi+16*8]
		movaps		xmm0, [esi]
		movaps		xmm1, [esi+16*1]
		movaps		xmm2, [esi+16*2]
		movaps		xmm3, [esi+16*3]
		movaps		xmm4, [esi+16*4]
		movaps		xmm5, [esi+16*5]
		movaps		xmm6, [esi+16*6]
		movaps		xmm7, [esi+16*7]
		movntps		[edi], xmm0
		movntps		[edi+16*1], xmm1
		movntps		[edi+16*2], xmm2
		movntps		[edi+16*3], xmm3
		movntps		[edi+16*4], xmm4
		movntps		[edi+16*5], xmm5
		movntps		[edi+16*6], xmm6
		movntps		[edi+16*7], xmm7
		add			esi, 128
		add			edi, 128
		dec			ecx
		jne			.loop128
.skiploop128:
		mov		ecx, [esp+12+16]
		and		ecx, 127
		cmp		ecx, 0
		je		.nooddballs
.loop:
		mov		dl, [esi]
		mov		[edi], dl
		inc		esi
		inc		edi
		dec		ecx
		jne		.loop
.nooddballs:
		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
; MPC custom code end

	global	_VDFastMemcpyPartialMMX2
_VDFastMemcpyPartialMMX2:
		push	ebp
		push	edi
		push	esi
		push	ebx

.MMX2 ; MPC custom code
		mov		ebx, [esp+4+16]
		mov		edx, [esp+8+16]
		mov		eax, [esp+12+16]
		neg		eax
		add		eax, 63
		jbe		.skipblastloop
.blastloop:
		movq	mm0, [edx]
		movq	mm1, [edx+8]
		movq	mm2, [edx+16]
		movq	mm3, [edx+24]
		movq	mm4, [edx+32]
		movq	mm5, [edx+40]
		movq	mm6, [edx+48]
		movq	mm7, [edx+56]
		movntq	[ebx], mm0
		movntq	[ebx+8], mm1
		movntq	[ebx+16], mm2
		movntq	[ebx+24], mm3
		movntq	[ebx+32], mm4
		movntq	[ebx+40], mm5
		movntq	[ebx+48], mm6
		movntq	[ebx+56], mm7
		add		ebx, 64
		add		edx, 64
		add		eax, 64
		jnc		.blastloop
.skipblastloop:
		sub		eax, 63-7
		jns		.noextras
.quadloop:
		movq	mm0, [edx]
		movntq	[ebx], mm0
		add		edx, 8
		add		ebx, 8
		add		eax, 8
		jnc		.quadloop
.noextras:
		sub		eax, 7
		jz		.nooddballs
		mov		ecx, eax
		neg		ecx
		mov		esi, edx
		mov		edi, ebx
		rep		movsb
.nooddballs:
		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret


		end

