		section	.rdata, rdata

x07b		dq		00707070707070707h
x0200w		dq		00200020002000200h
x001fw		dq		0001f001f001f001fh
xffc0w		dq		0ffc0ffc0ffc0ffc0h
xffe0w		dq		0ffe0ffe0ffe0ffe0h
x2080w		dq		02080208020802080h
x4200w		dq		04200420042004200h
rb_mask5	dq		000f800f800f800f8h
g_mask5		dq		00000f8000000f800h
g_mask6		dq		00000fc000000fc00h
rb_mul_565	dq		02000000420000004h
rb_mul_555	dq		02000000820000008h
r_mask_555	dq		07c007c007c007c00h
g_mask_555	dq		003e003e003e003e0h
b_mask_555	dq		0001f001f001f001fh
r_mask_565	dq		0f800f800f800f800h
g_mask_565	dq		007e007e007e007e0h
b_mask_565	dq		0001f001f001f001fh

%macro prologue 1
			push	ebx
			push	esi
			push	edi
			push	ebp
			;.fpo	(0,%1,4,4,1,0)
%endmacro

%macro epilogue 0
			pop		ebp
			pop		edi
			pop		esi
			pop		ebx
%endmacro

		section	.text

	global	_vdasm_pixblt_RGB565_to_XRGB1555_MMX
_vdasm_pixblt_RGB565_to_XRGB1555_MMX:
		prologue	6

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-6]
		lea		ecx, [ecx+ebp-6]
		neg		ebp
		mov		[esp+20+16], ebp

		movq	mm5, [x001fw]
		movq	mm4, [xffc0w]

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 6
		jbe		.odd

.xloop:
		movq	mm0, [ecx+ebp]
		movq	mm1, mm5
		pand	mm1, mm0
		pand	mm0, mm4
		psrlq	mm0, 1
		paddw	mm0, mm1
		movq	[edx+ebp], mm0
		add		ebp, 8
		jnc		.xloop

		sub		ebp, 6
		jz		.noodd
.odd:
		movzx	eax, word [ecx+ebp+6]
		mov		ebx, 0001f001fh
		and		ebx, eax
		and		eax, 0ffc0ffc0h
		shr		eax, 1
		add		eax, ebx
		mov		[edx+ebp+6], ax
		add		ebp, 2
		jnz		.odd
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		emms
		epilogue
		ret

	global	_vdasm_pixblt_XRGB8888_to_XRGB1555_MMX
_vdasm_pixblt_XRGB8888_to_XRGB1555_MMX:
		prologue	6

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-14]
		lea		ecx, [ecx+ebp*2-28]
		neg		ebp
		mov		[esp+20+16], ebp

		movq		mm5,[rb_mul_555]
		movq		mm6,[rb_mask5]
		movq		mm7,[g_mask5]

.yloop:
		mov			ebp, [esp+20+16]
		add			ebp, 14
		jbe			.odd

		;This code uses the "pmaddwd" trick for 32->16 conversions from Intel's MMX
		;Application Notes.

		movq		mm0,[ecx+ebp*2]		;allocate 0	(0123)
		movq		mm2,mm0				;allocate 2	(0 23)

		movq		mm1,[ecx+ebp*2+8]	;allocate 1	(0123)
		movq		mm3,mm1				;allocate 3	(0123)
		pand		mm0,mm6
		pmaddwd		mm0,mm5
		pand		mm1,mm6
		pmaddwd		mm1,mm5
		pand		mm2,mm7
		pand		mm3,mm7
		jmp			.xloopstart

		align 16
.xloop:
		movq		mm0,[ecx+ebp*2]		;allocate 0	(01234)
		por			mm4,mm2				;free 2		(01 34)

		por			mm3,mm1				;free 3		(01 34)
		movq		mm2,mm0				;allocate 2	(0 234)

		movq		mm1,[ecx+ebp*2+8]	;allocate 1	(01234)
		psrld		mm4,6

		psrld		mm3,6
		pand		mm0,mm6

		packssdw	mm4,mm3				;free 3		(012 4)
		movq		mm3,mm1				;allocate 3	(01234)

		pmaddwd		mm0,mm5
		pand		mm1,mm6

		pmaddwd		mm1,mm5
		pand		mm2,mm7

		movq		[edx+ebp-8],mm4		;free 4		(0123 )
		pand		mm3,mm7

.xloopstart:
		movq		mm4,[ecx+ebp*2+16]	;allocate 4	(01234)
		por			mm0,mm2				;free 2		(01 34)

		por			mm1,mm3				;free 3		(01  4)
		psrld		mm0,6

		movq		mm3,[ecx+ebp*2+24]	;allocate 3	(01 34)
		movq		mm2,mm4				;allocate 2	(01234)

		psrld		mm1,6
		pand		mm4,mm6

		packssdw	mm0,mm1				;free 1		(0 234)
		movq		mm1,mm3				;allocate 1	(01234)

		movq		[edx+ebp],mm0		;free 0		( 1234)
		pand		mm3,mm6

		pmaddwd		mm4,mm5
		add			ebp,16

		pmaddwd		mm3,mm5
		pand		mm2,mm7

		pand		mm1,mm7
		jnc			.xloop

		por			mm4,mm2				;free 2		(01 34)
		por			mm3,mm1				;free 3		(01 34)
		psrld		mm4,6
		psrld		mm3,6
		packssdw	mm4,mm3				;free 3		(012 4)
		movq		[edx+ebp-8],mm4		;free 4		(0123 )

.odd:
		sub			ebp, 14
		jz			.noodd
.oddloop:
		mov			eax, [ecx+ebp*2+28]
		mov			ebx, 00f80000h
		mov			esi, eax
		and			ebx, eax
		shr			ebx, 9
		and			esi, 0000f800h
		shr			esi, 6
		and			eax, 000000f8h
		shr			eax, 3
		add			esi, ebx
		add			eax, esi
		mov			[edx+ebp+14], ax
		add			ebp, 2
		jnz			.oddloop
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		emms
		epilogue
		ret

	global	_vdasm_pixblt_XRGB1555_to_RGB565_MMX
_vdasm_pixblt_XRGB1555_to_RGB565_MMX:
		prologue	6

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-6]
		lea		ecx, [ecx+ebp-6]
		neg		ebp
		mov		[esp+20+16], ebp

		movq	mm5, [x0200w]
		movq	mm4, [xffe0w]

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 6
		jbe		.odd

.xloop:
		movq	mm0, [ecx+ebp]
		movq	mm1, mm4
		movq	mm2, mm0
		pand	mm1, mm0
		pand	mm0, mm5
		paddw	mm1, mm2
		psrlq	mm0, 4
		paddw	mm0, mm1
		movq	[edx+ebp], mm0
		add		ebp, 8
		jnc		.xloop

.odd:
		sub		ebp, 6
		jz		.noodd
.oddloop:
		movzx	eax, word [ecx+ebp+6]
		mov		ebx, 02000200h
		mov		esi, eax
		and		ebx, eax
		shr		ebx, 4
		and		esi, 0ffe0ffe0h
		add		eax, esi
		add		eax, ebx
		mov		[edx+ebp+6], ax
		add		ebp, 2
		jnz		.oddloop
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		emms
		epilogue
		ret


	global	_vdasm_pixblt_XRGB8888_to_RGB565_MMX
_vdasm_pixblt_XRGB8888_to_RGB565_MMX:
		prologue	6

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-14]
		lea		ecx, [ecx+ebp*2-28]
		neg		ebp
		mov		[esp+20+16], ebp

		movq		mm5,[rb_mul_565]
		movq		mm6,[rb_mask5]
		movq		mm7,[g_mask6]

.yloop:
		mov			ebp, [esp+20+16]
		add			ebp, 14
		jbe			.odd

		;This code uses the "pmaddwd" trick for 32->16 conversions from Intel's MMX
		;Application Notes.

		movq		mm0,[ecx+ebp*2]		;allocate 0	(0123)
		movq		mm2,mm0				;allocate 2	(0 23)

		movq		mm1,[ecx+ebp*2+8]	;allocate 1	(0123)
		movq		mm3,mm1				;allocate 3	(0123)
		pand		mm0,mm6
		pmaddwd		mm0,mm5
		pand		mm1,mm6
		pmaddwd		mm1,mm5
		pand		mm2,mm7
		pand		mm3,mm7
		jmp			.xloopstart

		align 16
.xloop:
		movq		mm0,[ecx+ebp*2]		;allocate 0	(01234)
		por			mm4,mm2				;free 2		(01 34)

		por			mm3,mm1				;free 3		(01 34)
		pslld		mm4,16-5

		pslld		mm3,16-5
		movq		mm2,mm0				;allocate 2	(0 234)

		movq		mm1,[ecx+ebp*2+8]	;allocate 1	(01234)
		psrad		mm4,16

		psrad		mm3,16
		pand		mm0,mm6

		packssdw	mm4,mm3				;free 3		(012 4)
		movq		mm3,mm1				;allocate 3	(01234)

		pmaddwd		mm0,mm5
		pand		mm1,mm6

		pmaddwd		mm1,mm5
		pand		mm2,mm7

		movq		[edx+ebp-8],mm4		;free 4		(0123 )
		pand		mm3,mm7

.xloopstart:
		movq		mm4,[ecx+ebp*2+16]	;allocate 4	(01234)
		por			mm0,mm2				;free 2		(01 34)

		por			mm1,mm3				;free 3		(01  4)
		pslld		mm0,16-5

		movq		mm3,[ecx+ebp*2+24]	;allocate 3	(01 34)
		pslld		mm1,16-5

		psrad		mm0,16
		movq		mm2,mm4				;allocate 2	(01234)

		psrad		mm1,16
		pand		mm4,mm6

		packssdw	mm0,mm1				;free 1		(0 234)
		movq		mm1,mm3				;allocate 1	(01234)

		movq		[edx+ebp],mm0		;free 0		( 1234)
		pand		mm3,mm6

		pmaddwd		mm4,mm5
		add			ebp,16

		pmaddwd		mm3,mm5
		pand		mm2,mm7

		pand		mm1,mm7
		jnc			.xloop

		por			mm4,mm2				;free 2		(01 34)
		por			mm3,mm1				;free 3		(01 34)
		psllq		mm4,16-5
		psllq		mm3,16-5
		psrad		mm4,16
		psrad		mm3,16
		packssdw	mm4,mm3				;free 3		(012 4)
		movq		[edx+ebp-8],mm4		;free 4		(0123 )

.odd:
		sub			ebp, 14
		jz			.noodd
.oddloop:
		mov			eax, [ecx+ebp*2+28]
		mov			ebx, 00f80000h
		mov			esi, eax
		and			ebx, eax
		and			eax, 000000f8h
		shr			eax, 3
		and			esi, 0000fc00h
		shr			ebx, 8
		shr			esi, 5
		add			eax, ebx
		add			eax, esi
		mov			[edx+ebp+14], ax
		add			ebp, 2
		jnz			.oddloop
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		emms
		epilogue
		ret

	global	_vdasm_pixblt_XRGB8888_to_RGB888_MMX
_vdasm_pixblt_XRGB8888_to_RGB888_MMX:
		prologue	6

		mov			esi,[esp+12+16]
		mov			edi,[esp+4+16]

		mov			ecx,[esp+20+16]
		lea			eax,[ecx+ecx*2]
		lea			ebx,[ecx*4]
		sub			[esp+8+16],eax
		sub			[esp+16+16],ebx
		
		pcmpeqb		mm7,mm7
		psrld		mm7,8
		movq		mm6,mm7
		psllq		mm7,32			;mm7 = high rgb mask
		psrlq		mm6,32			;mm6 = low rgb mask
		
		mov			ebp,[esp+20+16]
		mov			edx,[esp+24+16]
		mov			eax,[esp+16+16]
		mov			ebx,[esp+ 8+16]
.yloop:
		mov			ecx,ebp
		shr			ecx,3
		jz			.checkodd
.xloop:
		movq		mm0,[esi]		;mm0 = a1r1g1b1a0r0g0b0
		movq		mm1,mm6
		
		movq		mm2,[esi+8]		;mm2 = a3r3g3b3a2r2g2b2
		pand		mm1,mm0			;mm1 = ----------r0g0b0
		
		movq		mm3,mm6
		pand		mm0,mm7			;mm0 = --r1g1b1--------
		
		movq		mm4,mm2
		pand		mm3,mm2			;mm3 = ----------r2g2b2
		
		psrlq		mm0,8			;mm0 = ----r1g1b1------
		pand		mm2,mm7			;mm2 = --r3g3b3--------
		
		movq		mm5,[esi+16]	;mm5 = a5r5g5b5a4r4g4b4
		psllq		mm4,48			;mm4 = g2b2------------
		
		por			mm0,mm1			;mm0 = ----r1g1b1r0g0b0
		psrlq		mm3,16			;mm3 = --------------r2
		
		por			mm0,mm4			;mm0 = g2b2r1g1b1r0g0b0
		movq		mm1,mm6
		
		pand		mm1,mm5			;mm1 = ----------r4g4b4
		psrlq		mm2,24			;mm2 = --------r3g3b3--
		
		movq		[edi],mm0
		pand		mm5,mm7			;mm5 = --r5g5b5--------
		
		psllq		mm1,32			;mm1 = --r4g4b4--------
		movq		mm4,mm5			;mm4 = --r5g5b5--------
		
		por			mm2,mm3			;mm2 = --------r3g3b3r2
		psllq		mm5,24			;mm5 = b5--------------
		
		movq		mm3,[esi+24]	;mm3 = a7r7g7b7a6r6g6b6
		por			mm2,mm1			;mm2 = --r4g4b4r3g3b3r2
		
		movq		mm1,mm6
		por			mm2,mm5			;mm2 = b5r4g4b4r3g3b3r2
		
		psrlq		mm4,40			;mm4 = ------------r5g5
		pand		mm1,mm3			;mm1 = ----------r6g6b6
		
		psllq		mm1,16			;mm1 = ------r6g6b6----	
		pand		mm3,mm7			;mm3 = --r7g7b7--------
		
		por			mm4,mm1			;mm4 = ------r6g6b6r5g5
		psllq		mm3,8			;mm3 = r7g7b7----------
		
		movq		[edi+8],mm2
		por			mm4,mm3			;mm4 = r7g7b7r6g6b6r5g5
		
		add			esi,32
		sub			ecx,1
		
		movq		[edi+16],mm4	;mm3

		lea			edi,[edi+24]
		jne			.xloop
	
.checkodd:
		mov			ecx,ebp
		and			ecx,7
		jz			.noodd
		movd		mm0,eax
.oddloop:
		mov			eax,[esi]
		add			esi,4
		mov			[edi],ax
		shr			eax,16
		mov			[edi+2],al
		add			edi,3
		sub			ecx,1
		jnz			.oddloop
		movd		eax,mm0
.noodd:
		add			esi,eax
		add			edi,ebx

		sub			edx,1
		jne			.yloop

		emms

		epilogue
		ret

	global	_vdasm_pixblt_XRGB1555_to_XRGB8888_MMX
_vdasm_pixblt_XRGB1555_to_XRGB8888_MMX:
		prologue	6

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp*2-12]
		lea		ecx, [ecx+ebp-6]
		neg		ebp
		mov		[esp+20+16], ebp

		movq	mm5, [r_mask_555]
		movq	mm6, [g_mask_555]
		movq	mm7, [b_mask_555]

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 6
		jbe		.odd

.xloop:
		movq		mm0, [ecx+ebp]
		movq		mm1, mm6
		movq		mm2, mm7
		pand		mm1, mm0
		pand		mm2, mm0
		pand		mm0, mm5

		paddw		mm0, mm0
		pmulhw		mm1, [x4200w]
		psllq		mm2, 3
		paddw		mm0, mm2
		movq		mm2, mm0
		psrlw		mm0, 5
		pand		mm0, [x07b]
		paddw		mm0, mm2
		movq		mm2, mm0
		punpcklbw	mm0, mm1
		punpckhbw	mm2, mm1

		movq	[edx+ebp*2], mm0
		movq	[edx+ebp*2+8], mm2
		add		ebp, 8
		jnc		.xloop
.odd:
		sub		ebp, 6
		jz		.noodd
.oddloop:
		movzx	eax, word [ecx+ebp+6]
		mov		ebx, 03e0h
		mov		esi, 001fh
		and		ebx, eax
		and		esi, eax
		and		eax, 07c00h
		shl		esi, 3
		shl		ebx, 6
		shl		eax, 9
		add		ebx, esi
		add		eax, ebx
		mov		ebx, eax
		shr		eax, 5
		and		eax, 070707h
		add		eax, ebx
		mov		[edx+ebp*2+12], eax
		add		ebp, 2
		jnz		.oddloop
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		emms
		epilogue
		ret


	global	_vdasm_pixblt_RGB565_to_XRGB8888_MMX
_vdasm_pixblt_RGB565_to_XRGB8888_MMX:
		prologue	6

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp*2-12]
		lea		ecx, [ecx+ebp-6]
		neg		ebp
		mov		[esp+20+16], ebp

		movq	mm5, [r_mask_565]
		movq	mm6, [g_mask_565]
		movq	mm7, [b_mask_565]

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 6
		jbe		.odd

.xloop:
		movq		mm0, [ecx+ebp]
		movq		mm1, mm6
		movq		mm2, mm7
		pand		mm1, mm0
		pand		mm2, mm0
		pand		mm0, mm5

		pmulhw		mm1, [x2080w]
		psllq		mm2, 3
		paddw		mm0, mm2
		movq		mm2, mm0
		psrlw		mm0, 5
		pand		mm0, [x07b]
		paddw		mm0, mm2
		movq		mm2, mm0
		punpcklbw	mm0, mm1
		punpckhbw	mm2, mm1

		movq	[edx+ebp*2], mm0
		movq	[edx+ebp*2+8], mm2
		add		ebp, 8
		jnc		.xloop

.odd:
		sub		ebp, 6
		jz		.noodd
		push	edi
.oddloop:
		movzx	eax, word [ecx+ebp+6]
		mov		ebx, 0000f800h
		and		ebx, eax
		mov		esi, eax
		shl		ebx, 8
		mov		edi, eax
		shl		eax, 3
		and		esi, 000007e0h
		and		eax, 000000f8h
		add		ebx, eax
		shl		esi, 5
		mov		eax, ebx
		shr		ebx, 5
		and		edi, 00000600h
		shr		edi, 1
		and		ebx, 00070007h
		add		esi, edi
		add		eax, ebx
		add		eax, esi
		mov		[edx+ebp*2+12], eax
		add		ebp, 2
		jnz		.oddloop
		pop		edi
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		emms
		epilogue
		ret


	global	_vdasm_pixblt_RGB888_to_XRGB8888_MMX
_vdasm_pixblt_RGB888_to_XRGB8888_MMX:
		prologue	6

		mov		esi,[esp+12+16]
		mov		edi,[esp+4+16]

		mov		ecx,[esp+20+16]
		lea		eax,[ecx+ecx*2]
		lea		ebx,[ecx*4]
		sub		[esp+8+16],ebx
		sub		[esp+16+16],eax

		mov		edx,[esp+24+16]
		mov		ebx,[esp+20+16]
		mov		ecx,[esp+16+16]
		mov		eax,[esp+ 8+16]
		
		;ebx	horizontal count backup
		;ecx	source modulo
		;edx	vertical count
		;esi	source
		;edi	destination
		;ebp	horizontal count
	
.yloop:
		mov	ebp,ebx
		shr	ebp,3
		jz	.checkodd
.xloop:
		movq		mm0,[esi]		;mm0: g2b2r1g1b1r0g0b0
		movq		mm1,mm0			;
		
		psrlq		mm1,24			;mm1: ------g2b2r1g1b1
		movq		mm2,mm0			;
		
		movq		mm3,[esi+8]		;mm3: b5r4g4b4r3g3b3r2
		punpckldq	mm0,mm1			;mm0: b2r1g1b1b1r0g0b0	[qword 0 ready]
		
		movq		mm4,mm3			;mm4: b5r4g4b4r3g3b3r2
		psllq		mm3,48			;mm3: b3r2------------
		
		movq		mm5,mm4			;mm5: b5r4g4b4r3g3b3r2
		psrlq		mm2,16			;mm2: ----g2b2--------
		
		movq		mm1,[esi+16]	;mm1: r7g7b7r6g6b6r5g5
		por			mm2,mm3			;mm2: b3r2g2b2--------
		
		movq		[edi],mm0		;
		psllq		mm4,24			;mm4: b4r3g3b3r2------
		
		movq		mm3,mm5			;mm3: b5r4g4b4r3g3b3r2
		psrlq		mm5,24			;mm5: ------b5r4g4b4r3
		
		movq		mm0,mm1			;mm0: r7g7b7r6g6b6r5g5
		psllq		mm1,40			;mm1: b6r5g5----------
		
		punpckhdq	mm2,mm4			;mm2: b4r3g3b3b3r2g2b2 [qword 1 ready]
		por			mm1,mm5			;mm1: b6r5g5b5r4g4b4r3
		
		movq		mm4,mm0			;mm4: r7g7b7r6g6b6r5g5
		punpckhdq	mm3,mm1			;mm3: b6r5g5b5b5r4g4b4 [qword 2 ready]
		
		movq		[edi+8],mm2
		psrlq		mm0,16			;mm0: ----r7g7b7r6g6b6
		
		movq		[edi+16],mm3
		psrlq		mm4,40			;mm4: ----------r7g7b7
		
		punpckldq	mm0,mm4			;mm0: --r7g7b7b7r6g6b6 [qword 3 ready]
		add			esi,24
		
		movq		[edi+24],mm0
			
		add			edi,32
		sub			ebp,1
		jne			.xloop

.checkodd:
		mov			ebp,ebx
		and			ebp,7
		jz			.noodd
		movd		mm7,eax
.oddloop:
		mov			ax,[esi]
		mov			[edi],ax
		mov			al,[esi+2]
		mov			[edi+2],al
		add			esi,3
		add			edi,4
		sub			ebp,1
		jne			.oddloop
		
		movd		eax,mm7
.noodd:
		add			esi,ecx
		add			edi,eax

		sub			edx,1
		jne			.yloop
		emms
		epilogue
		ret

		end
