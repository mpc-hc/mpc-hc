		section	.rdata, rdata

y_co	dq		0004a004a004a004ah
cr_co_r	dq		000cc00cc00cc00cch
cb_co_b	dq		00081008100810081h		;note: divided by two
cr_co_g	dq		0ff98ff98ff98ff98h
cb_co_g	dq		0ffceffceffceffceh
y_bias	dq		0fb7afb7afb7afb7ah
c_bias	dq		0ff80ff80ff80ff80h
interp	dq		06000400020000000h
rb_mask_555	dq		07c1f7c1f7c1f7c1fh
g_mask_555	dq		003e003e003e003e0h
rb_mask_565	dq		0f81ff81ff81ff81fh
g_mask_565	dq		007e007e007e007e0h

cr_coeff	dq	000003313e5fc0000h
cb_coeff	dq	000000000f377408dh
rgb_bias	dq	000007f2180887eebh

msb_inv	dq		08000800080008000h

		section	.text

;============================================================================

%macro YUV411PLANAR_TO_RGB_PROLOG 0
		push		ebp
		push		edi
		push		esi
		push		ebx

		mov			eax, [esp+4+16]
		mov			ecx, [esp+8+16]
		mov			edx, [esp+12+16]
		mov			ebx, [esp+16+16]
		mov			ebp, [esp+20+16]

		pxor		mm7, mm7
%endmacro

%macro YUV411PLANAR_TO_RGB_CORE_MMX 0
		movd		mm0, dword [ecx]		;mm0 = Y3Y2Y1Y0
		add			ecx, 4
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0
		movq		mm1, mm0
		pmullw		mm0, [y_co]
		paddw		mm1, [y_bias]
		paddsw		mm0, mm0
		paddsw		mm0, mm1

		movzx		esi, word [ebx]
		movzx		edi, word [edx]
		add			ebx, 1
		add			edx, 1

		movd		mm1, esi
		movd		mm2, edi

		punpcklbw	mm1, mm7
		paddw		mm1, [c_bias]
		punpcklwd	mm1, mm1
		movq		mm3, mm1
		punpckldq	mm1, mm1
		punpckhdq	mm3, mm3

		punpcklbw	mm2, mm7
		paddw		mm2, [c_bias]
		punpcklwd	mm2, mm2
		movq		mm4, mm2
		punpckldq	mm2, mm2
		punpckhdq	mm4, mm4

		psubw		mm3, mm1
		psubw		mm4, mm2
		paddw		mm3, mm3
		paddw		mm4, mm4

		pmulhw		mm3, [interp]
		pmulhw		mm4, [interp]

		paddw		mm1, mm3
		paddw		mm2, mm4

		movq		mm3, mm1
		movq		mm4, mm2

		pmullw		mm1, [cr_co_r]
		pmullw		mm2, [cb_co_b]
		pmullw		mm3, [cr_co_g]
		pmullw		mm4, [cb_co_g]

		paddsw		mm2, mm2
		paddsw		mm1, mm0
		paddsw		mm3, mm4
		paddsw		mm2, mm0
		paddsw		mm3, mm0

		psraw		mm1, 7
		psraw		mm2, 7
		psraw		mm3, 7

		packuswb	mm1, mm1
		packuswb	mm2, mm2
		packuswb	mm3, mm3
%endmacro

%macro YUV411PLANAR_TO_RGB_CORE_ISSE 0
		movd		mm0, dword [ecx]		;mm0 = Y3Y2Y1Y0
		add			ecx, 4
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0
		movq		mm1, mm0
		pmullw		mm0, [y_co]
		paddw		mm1, [y_bias]
		paddsw		mm0, mm0
		paddsw		mm0, mm1

		movzx		esi, word [ebx]
		movzx		edi, word [edx]
		add			ebx, 1
		add			edx, 1

		movd		mm1, esi
		movd		mm2, edi

		punpcklbw	mm1, mm7
		paddw		mm1, [c_bias]
		pshufw		mm3, mm1, 01010101b
		pshufw		mm1, mm1, 00000000b

		punpcklbw	mm2, mm7
		paddw		mm2, [c_bias]
		pshufw		mm4, mm2, 01010101b
		pshufw		mm2, mm2, 00000000b

		psubw		mm3, mm1
		psubw		mm4, mm2
		paddw		mm3, mm3
		paddw		mm4, mm4

		pmulhw		mm3, [interp]
		pmulhw		mm4, [interp]

		paddw		mm1, mm3
		paddw		mm2, mm4

		psllw		mm1, 3
		psllw		mm2, 3

		movq		mm3, [cr_co_g]
		movq		mm4, [cb_co_g]

		pmullw		mm3, mm1
		pmullw		mm4, mm2
		pmullw		mm1, [cr_co_r]
		pmullw		mm2, [cb_co_b]

		paddsw		mm2, mm2
		paddsw		mm1, mm0
		paddsw		mm3, mm4
		paddsw		mm2, mm0
		paddsw		mm3, mm0

		psraw		mm1, 7
		psraw		mm2, 7
		psraw		mm3, 7

		packuswb	mm1, mm1
		packuswb	mm2, mm2
		packuswb	mm3, mm3
%endmacro

%macro YUV411PLANAR_TO_RGB_EPILOG 0
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
%endmacro

	global	_vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_MMX
_vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_MMX:
		YUV411PLANAR_TO_RGB_PROLOG
.xloop:
		YUV411PLANAR_TO_RGB_CORE_MMX

		psrlw		mm1, 1
		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 2
		pand		mm2, [rb_mask_555]
		pand		mm3, [g_mask_555]
		por			mm2, mm3

		movq		[eax], mm2
		add			eax, 8

		sub			ebp, 1
		jne			.xloop

		YUV411PLANAR_TO_RGB_EPILOG

;============================================================================

	global	_vdasm_pixblt_YUV411Planar_to_RGB565_scan_MMX
_vdasm_pixblt_YUV411Planar_to_RGB565_scan_MMX:
		YUV411PLANAR_TO_RGB_PROLOG
.xloop:
		YUV411PLANAR_TO_RGB_CORE_MMX

		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 3
		pand		mm2, [rb_mask_565]
		pand		mm3, [g_mask_565]
		por			mm2, mm3

		movq		[eax], mm2
		add			eax, 8

		sub			ebp, 1
		jne			.xloop

		YUV411PLANAR_TO_RGB_EPILOG

;============================================================================

	global	_vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_MMX
_vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_MMX:
		YUV411PLANAR_TO_RGB_PROLOG
.xloop:
		YUV411PLANAR_TO_RGB_PROLOG

		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		movq		mm1, mm2
		punpcklbw	mm1, mm3
		punpckhbw	mm2, mm3

		movq		[eax], mm1
		movq		[eax+8], mm2
		add			eax, 16

		sub			ebp, 1
		jne			.xloop

		YUV411PLANAR_TO_RGB_EPILOG

;============================================================================

	global	_vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_ISSE
_vdasm_pixblt_YUV411Planar_to_XRGB1555_scan_ISSE:
		YUV411PLANAR_TO_RGB_PROLOG
.xloop:
		YUV411PLANAR_TO_RGB_CORE_ISSE

		psrlw		mm1, 1
		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 2
		pand		mm2, [rb_mask_555]
		pand		mm3, [g_mask_555]
		por			mm2, mm3

		movq		[eax], mm2
		add			eax, 8

		sub			ebp, 1
		jne			.xloop

		YUV411PLANAR_TO_RGB_EPILOG

;============================================================================

	global	_vdasm_pixblt_YUV411Planar_to_RGB565_scan_ISSE
_vdasm_pixblt_YUV411Planar_to_RGB565_scan_ISSE:
		YUV411PLANAR_TO_RGB_PROLOG
.xloop:
		YUV411PLANAR_TO_RGB_CORE_ISSE

		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 3
		pand		mm2, [rb_mask_565]
		pand		mm3, [g_mask_565]
		por			mm2, mm3

		movq		[eax], mm2
		add			eax, 8

		sub			ebp, 1
		jne			.xloop

		YUV411PLANAR_TO_RGB_EPILOG

;============================================================================

	global	_vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_ISSE
_vdasm_pixblt_YUV411Planar_to_XRGB8888_scan_ISSE:
		push		ebp
		push		edi
		push		esi
		push		ebx

		mov			eax, [esp+4+16]
		mov			ecx, [esp+8+16]
		mov			edx, [esp+12+16]
		mov			ebx, [esp+16+16]
		mov			ebp, [esp+20+16]

		pxor		mm7, mm7

		movzx		esi, byte [ebx]
		movzx		edi, byte [edx]
		add			ebx, 1
		add			edx, 1

		movd		mm1, esi
		movd		mm2, edi

		psllw		mm1, 3
		psllw		mm2, 3

		pshufw		mm5, mm1, 0
		pshufw		mm6, mm2, 0

		pmulhw		mm5, [cr_coeff]
		pmulhw		mm6, [cb_coeff]
		paddw		mm6, mm5
		paddw		mm6, [rgb_bias]

.xloop:
		movd		mm0, dword [ecx];mm0 = Y3Y2Y1Y0
		add			ecx, 4
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0
		psllw		mm0, 3
		pmulhw		mm0, [y_co]
		pxor		mm0, [msb_inv]

		movzx		esi, byte [ebx]
		movzx		edi, byte [edx]
		add			ebx, 1
		add			edx, 1

		movd		mm1, esi
		movd		mm2, edi

		psllw		mm1, 3
		psllw		mm2, 3

		pshufw		mm1, mm1, 0
		pshufw		mm2, mm2, 0

		pmulhw		mm1, [cr_coeff]
		pmulhw		mm2, [cb_coeff]
		paddw		mm1, mm2
		paddw		mm1, [rgb_bias]

		movq		mm2, mm1
		pavgw		mm2, mm6			;mm2 = 1/2
		pshufw		mm3, mm0, 00000000b
		paddw		mm3, mm6
		pavgw		mm6, mm2			;mm1 = 1/4
		pshufw		mm4, mm0, 01010101b
		paddw		mm4, mm6
		packuswb	mm3, mm4
		movq		[eax], mm3

		pshufw		mm3, mm0, 10101010b
		paddw		mm3, mm2
		pshufw		mm0, mm0, 11111111b
		pavgw		mm2, mm1			;mm2 = 3/4
		paddw		mm2, mm0
		packuswb	mm3, mm2
		movq		[eax+8], mm3

		movq		mm6, mm1

		add			eax, 16

		sub			ebp, 1
		jne			.xloop

		YUV411PLANAR_TO_RGB_EPILOG

;==========================================================================

%macro YUV444PLANAR_TO_RGB_PROLOG 0
		push		ebp
		push		edi
		push		esi
		push		ebx

		mov			eax, [esp+4+16]
		mov			ecx, [esp+8+16]
		mov			edx, [esp+12+16]
		mov			ebx, [esp+16+16]
		mov			ebp, [esp+20+16]
%endmacro

%macro YUV444PLANAR_TO_RGB_CORE 0
		movq		mm3, mm0
		pmullw		mm0, [y_co]
		paddw		mm1, [c_bias]
		paddw		mm2, [c_bias]
		paddw		mm0, [y_bias]
		paddsw		mm0, mm0
		paddsw		mm0, mm3

		movq		mm3, [cr_co_g]
		movq		mm4, [cb_co_g]

		pmullw		mm3, mm1
		pmullw		mm4, mm2
		pmullw		mm1, [cr_co_r]
		pmullw		mm2, [cb_co_b]

		paddsw		mm2, mm2
		paddsw		mm1, mm0
		paddsw		mm3, mm4
		paddsw		mm2, mm0
		paddsw		mm3, mm0

		psraw		mm1, 7
		psraw		mm2, 7
		psraw		mm3, 7

		packuswb	mm1, mm1
		packuswb	mm2, mm2
		packuswb	mm3, mm3
%endmacro

%macro YUV444PLANAR_TO_RGB_EPILOG 0
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
%endmacro

;==========================================================================

	global	_vdasm_pixblt_YUV444Planar_to_XRGB1555_scan_MMX
_vdasm_pixblt_YUV444Planar_to_XRGB1555_scan_MMX:
		YUV444PLANAR_TO_RGB_PROLOG

		pxor		mm7, mm7
		movq		mm5, [rb_mask_555]
		movq		mm6, [g_mask_555]

		sub			ebp, 3
		jbe			.oddcheck
.xloop4:
		movd		mm0, dword [ecx];mm0 = Y3Y2Y1Y0
		movd		mm1, dword [ebx]
		movd		mm2, dword [edx]
		add			ecx, 4
		add			ebx, 4
		add			edx, 4
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0
		punpcklbw	mm1, mm7
		punpcklbw	mm2, mm7

		YUV444PLANAR_TO_RGB_CORE

		psrlw		mm1, 1
		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 2
		pand		mm2, mm5
		pand		mm3, mm6
		por			mm2, mm3

		movq		[eax], mm2
		add			eax, 8

		sub			ebp, 4
		ja			.xloop4
.oddcheck:
		add			ebp, 3
		jz			.noodd
.xloop:
		movzx		edi, byte [ecx]			;mm0 = Y3Y2Y1Y0
		movd		mm0, edi
		movzx		edi, byte [ebx]
		movd		mm1, edi
		movzx		edi, byte [edx]
		movd		mm2, edi
		add			ecx, 1
		add			ebx, 1
		add			edx, 1

		YUV444PLANAR_TO_RGB_CORE

		psrlw		mm1, 1
		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 2
		pand		mm2, mm5
		pand		mm3, mm6
		por			mm2, mm3

		movd		edi, mm2
		mov			[eax], di
		add			eax, 2

		sub			ebp, 1
		jnz			.xloop
.noodd:
		YUV444PLANAR_TO_RGB_EPILOG

;==========================================================================

	global	_vdasm_pixblt_YUV444Planar_to_RGB565_scan_MMX
_vdasm_pixblt_YUV444Planar_to_RGB565_scan_MMX:
		YUV444PLANAR_TO_RGB_PROLOG

		pxor		mm7, mm7
		movq		mm5, [rb_mask_565]
		movq		mm6, [g_mask_565]

		sub			ebp, 3
		jbe			.oddcheck
.xloop4:
		movd		mm0, dword [ecx];mm0 = Y3Y2Y1Y0
		movd		mm1, dword [ebx]
		movd		mm2, dword [edx]
		add			ecx, 4
		add			ebx, 4
		add			edx, 4
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0
		punpcklbw	mm1, mm7
		punpcklbw	mm2, mm7

		YUV444PLANAR_TO_RGB_CORE

		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 3
		pand		mm2, mm5
		pand		mm3, mm6
		por			mm2, mm3

		movq		[eax], mm2
		add			eax, 8

		sub			ebp, 4
		ja			.xloop4
.oddcheck:
		add			ebp, 3
		jz			.noodd
.xloop:
		movzx		edi, byte [ecx]			;mm0 = Y3Y2Y1Y0
		movd		mm0, edi
		movzx		edi, byte [ebx]
		movd		mm1, edi
		movzx		edi, byte [edx]
		movd		mm2, edi
		add			ecx, 1
		add			ebx, 1
		add			edx, 1

		YUV444PLANAR_TO_RGB_CORE

		psrlw		mm2, 3
		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		psllw		mm3, 3
		pand		mm2, mm5
		pand		mm3, mm6
		por			mm2, mm3

		movd		edi, mm2
		mov			[eax], di
		add			eax, 2

		sub			ebp, 1
		jnz			.xloop
.noodd:
		YUV444PLANAR_TO_RGB_EPILOG

;==========================================================================

	global	_vdasm_pixblt_YUV444Planar_to_XRGB8888_scan_MMX
_vdasm_pixblt_YUV444Planar_to_XRGB8888_scan_MMX:
		YUV444PLANAR_TO_RGB_PROLOG

		pxor		mm7, mm7

		sub			ebp, 3
		jbe			.oddcheck
.xloop4:
		movd		mm0, dword [ecx];mm0 = Y3Y2Y1Y0
		movd		mm1, dword [ebx]
		movd		mm2, dword [edx]
		add			ecx, 4
		add			ebx, 4
		add			edx, 4
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0
		punpcklbw	mm1, mm7
		punpcklbw	mm2, mm7

		YUV444PLANAR_TO_RGB_CORE

		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		movq		mm1, mm2
		punpcklbw	mm1, mm3
		punpckhbw	mm2, mm3

		movq		[eax], mm1
		movq		[eax+8], mm2
		add			eax, 16

		sub			ebp, 4
		ja			.xloop4
.oddcheck:
		add			ebp, 3
		jz			.noodd
.xloop:
		movzx		edi, byte [ecx]			;mm0 = Y3Y2Y1Y0
		movd		mm0, edi
		movzx		edi, byte [ebx]
		movd		mm1, edi
		movzx		edi, byte [edx]
		movd		mm2, edi
		add			ecx, 1
		add			ebx, 1
		add			edx, 1
		punpcklbw	mm0, mm7			;mm0 = Y3 | Y2 | Y1 | Y0

		YUV444PLANAR_TO_RGB_CORE

		punpcklbw	mm2, mm1
		punpcklbw	mm3, mm3
		punpcklbw	mm2, mm3

		movd		dword [eax], mm2
		add			eax, 4

		sub			ebp, 1
		jnz			.xloop
.noodd:
		YUV444PLANAR_TO_RGB_EPILOG

		end