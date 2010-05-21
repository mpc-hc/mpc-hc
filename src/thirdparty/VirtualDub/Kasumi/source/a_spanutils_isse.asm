		section	.rdata, rdata, align=16

xfefefefefefefefe	dq	0fefefefefefefefeh
xe0e0e0e0e0e0e0e0	dq	0e0e0e0e0e0e0e0e0h
x0002000200020002	dq	00002000200020002h

		section	.text

;==============================================================================
		global _vdasm_horiz_expand2x_coaligned_ISSE
_vdasm_horiz_expand2x_coaligned_ISSE:
		mov			ecx, [esp+8]
		mov			edx, [esp+4]
		mov			eax, [esp+12]
.xloop:
		movq		mm0, [ecx]
		movq		mm1, mm0
		pavgb		mm0, [ecx+1]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edx], mm1
		movq		[edx+8], mm2
		add			edx, 16
		add			ecx, 8

		sub			eax, 16
		jne			.xloop
		ret

;==============================================================================
		global	_vdasm_vert_average_13_ISSE
_vdasm_vert_average_13_ISSE:
		push	ebx
		mov		ebx, [esp+12+4]
		mov		ecx, [esp+8+4]
		mov		edx, [esp+4+4]
		mov		eax, [esp+16+4]

		add		ebx, eax
		add		ecx, eax
		add		edx, eax
		neg		eax

		pcmpeqb	mm7, mm7
.xloop:
		movq	mm0, [ebx+eax]
		movq	mm1, [ecx+eax]
		movq	mm2, mm0

		movq	mm3, [ebx+eax+8]
		pxor	mm0, mm7
		pxor	mm1, mm7

		movq	mm4, [ecx+eax+8]
		movq	mm5, mm3
		pxor	mm3, mm7

		pxor	mm4, mm7
		pavgb	mm0, mm1
		pavgb	mm3, mm4

		pxor	mm0, mm7
		pxor	mm3, mm7
		pavgb	mm0, mm2

		movq	[edx+eax], mm0
		pavgb	mm3, mm5

		movq	[edx+eax+8], mm3
		add		eax, 16
		jne		.xloop

		pop		ebx
		ret

;==============================================================================
		global	_vdasm_vert_average_17_ISSE
_vdasm_vert_average_17_ISSE:
		push	ebx
		mov		ebx, [esp+12+4]
		mov		ecx, [esp+8+4]
		mov		edx, [esp+4+4]
		mov		eax, [esp+16+4]

		add		ebx, eax
		add		ecx, eax
		add		edx, eax
		neg		eax

		;r = avgup(avgdown(avgdown(a, b), a), a)
		;  = pavgb(~pavgb(pavgb(~a, ~b), ~a), a)
		
		pcmpeqb		mm7, mm7
.xloop:
		movq		mm0, [ecx+eax]
		movq		mm1, [ebx+eax]
		movq		mm2, mm0
		pxor		mm0, mm7			;~a
		pxor		mm1, mm7			;~b
		pavgb		mm1, mm0			;pavgb(~a, ~b) = ~avgdown(a, b)
		pavgb		mm1, mm0			;pavgb(~avgdown(a, b), ~a) = ~avgdown(avgdown(a, b), a)
		pxor		mm1, mm7			;avgdown(avgdown(a, b), a)
		pavgb		mm1, mm2			;pavgb(avgdown(avgdown(a, b), a), a) = round((7*a + b)/8)
		movq		[edx+eax], mm1
		
		add		eax, 8
		jne		.xloop

		pop		ebx
		ret

;==============================================================================
		global	_vdasm_vert_average_35_ISSE
_vdasm_vert_average_35_ISSE:
		push	ebx
		mov		ebx, [esp+12+4]
		mov		ecx, [esp+8+4]
		mov		edx, [esp+4+4]
		mov		eax, [esp+16+4]

		add		ebx, eax
		add		ecx, eax
		add		edx, eax
		neg		eax

		;r = avgup(avgdown(avgdown(a, b), b), a)
		;  = pavgb(~pavgb(pavgb(~a, ~b), ~b), a)
		
		pcmpeqb		mm7, mm7
.xloop:
		movq		mm0, [ecx+eax]
		movq		mm1, [ebx+eax]
		movq		mm2, mm0
		pxor		mm0, mm7		;~a
		pxor		mm1, mm7		;~b
		pavgb		mm0, mm1		;avgup(~a, ~b) = ~avgdown(a, b)
		pavgb		mm0, mm1		;avgup(~avgdown(a, b), ~b) = ~avgdown(avgdown(a, b), b)
		pxor		mm0, mm7		;avgdown(avgdown(a, b), b)
		pavgb		mm0, mm2		;avgup(avgdown(avgdown(a, b), b), a) = round((5*a + 3*b) / 8)
		movq		[edx+eax], mm0
		
		add		eax, 8
		jne		.xloop

		pop		ebx
		ret

;==============================================================================
		global	_vdasm_horiz_expand4x_coaligned_MMX
_vdasm_horiz_expand4x_coaligned_MMX:
		mov			edx, [esp+4]
		mov			ecx, [esp+8]
		mov			eax, [esp+12]
		movq		mm6, qword [x0002000200020002]
		pxor		mm7, mm7
.xloop:
		movd		mm0, [ecx]
		movd		mm1, [ecx+1]
		add			ecx, 4
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		psubw		mm1, mm0		;x1
		movq		mm2, mm1
		paddw		mm1, mm6		;x1 + 2
		movq		mm3, mm1
		paddw		mm2, mm2		;x2
		paddw		mm3, mm2		;x3 + 2
		paddw		mm2, mm6		;x2 + 2
		psraw		mm1, 2			;x1/4
		psraw		mm2, 2			;x2/4
		psraw		mm3, 2			;x3/4
		paddw		mm1, mm0
		paddw		mm2, mm0
		paddw		mm3, mm0
		movd		mm0, [ecx-4]
		packuswb	mm1, mm1
		packuswb	mm2, mm2
		packuswb	mm3, mm3
		punpcklbw	mm0, mm1
		punpcklbw	mm2, mm3
		movq		mm1, mm0
		punpcklwd	mm0, mm2
		punpckhwd	mm1, mm2
		
		movq		[edx], mm0
		movq		[edx+8], mm1
		add			edx, 16
		sub			eax, 1
		jne			.xloop
		
		ret
