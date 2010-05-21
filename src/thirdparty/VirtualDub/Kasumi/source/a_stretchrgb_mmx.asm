		segment	.rdata, align=16

x0020w			dq	00020002000200020h
rb_mask_555		dq	07c1f7c1f7c1f7c1fh
g_mask_555		dq	003e003e003e003e0h
rb_mask_888		dq	000ff00ff00ff00ffh
g_mask_888		dq	00000ff000000ff00h

		segment	.text

		struc	VDPixmapReferenceStretchBltBilinearParameters
.dst		resd	1
.src		resd	1
.u			resd	1
.uinc		resd	1
.dudx		resd	1

.xprepos	resd	1
.xpostpos	resd	1
.xprecopy	resd	1
.xpostcopy	resd	1
.xmidsize	resd	1
		endstruc



		global	_vdasm_stretchbltV_XRGB1555_to_XRGB1555_MMX
_vdasm_stretchbltV_XRGB1555_to_XRGB1555_MMX:
		push		ebp
		push		edi
		push		esi
		push		ebx

		mov			eax, [esp+20+16]
		and			eax, 0f8000000h
		mov			ebx, [esp+8+16]
		mov			ecx, [esp+12+16]
		jz			.noreverse
		xchg		ebx, ecx
		js			.noreverse
		neg			eax
		xchg		ebx, ecx
.noreverse:
		shr			eax, 16
		mov			[esp+20+16], eax
		mov			edx, [esp+4+16]
		mov			eax, [esp+16+16]
		add			eax, eax
		lea			ebx, [ebx+eax-6]
		lea			ecx, [ecx+eax-6]
		lea			edx, [edx+eax-6]
		neg			eax

		movd		mm4, dword [esp+20+16]
		punpcklwd	mm4, mm4
		punpckldq	mm4, mm4

		movq		mm6, [rb_mask_555]
		movq		mm7, [g_mask_555]

.xstart:
		add			eax, 6
		jbe			.doodd
.xloop:
		movq		mm0, [ebx+eax]
		movq		mm1, [ecx+eax]
		movq		mm2, mm7
		movq		mm3, mm7

		pand		mm2, mm0
		pand		mm3, mm1
		pand		mm0, mm6
		pand		mm1, mm6

		psubw		mm3, mm2
		psubw		mm1, mm0

		pmulhw		mm3, mm4
		pmulhw		mm1, mm4

		psubw		mm0, mm1
		psubw		mm2, mm3

		pand		mm0, mm6
		pand		mm2, mm7

		paddw		mm0, mm2

		movq		[edx+eax], mm0
		add			eax, 8
		jnc			.xloop

.doodd:
		sub			eax, 6
		jz			.noodd
.odd:
		movzx		esi, word [ebx+eax+6]
		movd		mm0, esi
		movzx		esi, word [ecx+eax+6]
		movd		mm1, esi
		movq		mm2, mm7
		movq		mm3, mm7

		pand		mm2, mm0
		pand		mm3, mm1
		pand		mm0, mm6
		pand		mm1, mm6

		psubw		mm3, mm2
		psubw		mm1, mm0

		pmulhw		mm3, mm4
		pmulhw		mm1, mm4

		psubw		mm0, mm1
		psubw		mm2, mm3

		pand		mm0, mm6
		pand		mm2, mm7

		paddw		mm0, mm2

		movd		esi, mm0
		mov			[edx+eax+6], si
		add			eax,2
		jne			.odd

.noodd:
		emms
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret


	global	_vdasm_stretchbltH_XRGB8888_to_XRGB8888_MMX
_vdasm_stretchbltH_XRGB8888_to_XRGB8888_MMX:
		push		ebp
		push		edi
		push		esi
		push		ebx

		mov			edx, [esp+4+16]

		mov			ebx, [edx+VDPixmapReferenceStretchBltBilinearParameters.src]
		mov			edi, [edx+VDPixmapReferenceStretchBltBilinearParameters.dst]

		mov			ecx, [edx+VDPixmapReferenceStretchBltBilinearParameters.xprecopy]
		or			ecx, ecx
		jz			.noprecopy
		mov			eax, [edx+VDPixmapReferenceStretchBltBilinearParameters.xprepos]
		mov			eax, [ebx+eax]
		lea			ebp, [ecx*4]
		sub			edi, ebp
		rep			stosd
.noprecopy:
		mov			ebp, [edx+VDPixmapReferenceStretchBltBilinearParameters.xmidsize]
		add			ebp, ebp
		add			ebp, ebp
		add			edi, ebp
		neg			ebp

		mov			esi, [edx+VDPixmapReferenceStretchBltBilinearParameters.u]
		mov			eax, [edx+VDPixmapReferenceStretchBltBilinearParameters.dudx]
		mov			edx, [edx+VDPixmapReferenceStretchBltBilinearParameters.uinc]
		movd		mm2, esi
		movd		mm3, eax
		shr			ebx, 2

		movq		mm5, mm2
		punpcklwd	mm5, mm5
		punpckhdq	mm5, mm5
		movq		mm4, mm5
		psraw		mm4, 15

.xloop:
		movd		mm0, dword [ebx*4]
		pxor		mm7, mm7
		movd		mm1, dword [ebx*4+4]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		psubw		mm1, mm0
		pand		mm4, mm1
		pmulhw		mm1, mm5
		paddw		mm1, mm4
		paddw		mm0, mm1
		packuswb	mm0, mm0
		movd		dword [edi+ebp], mm0

		add			esi, eax
		adc			ebx, edx

		paddd		mm2, mm3
		movq		mm5, mm2
		punpcklwd	mm5, mm5
		punpckhdq	mm5, mm5
		movq		mm4, mm5
		psraw		mm4, 15
		add			ebp, 4
		jnz			.xloop

		mov			edx, [esp+4+16]
		mov			ecx, [edx+VDPixmapReferenceStretchBltBilinearParameters.xpostcopy]
		or			ecx, ecx
		jz			.nopostcopy
		mov			eax, [edx+VDPixmapReferenceStretchBltBilinearParameters.xpostpos]
		add			eax, [edx+VDPixmapReferenceStretchBltBilinearParameters.src]
		mov			eax, [eax]
		rep			stosd
.nopostcopy:

		emms
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

	global	_vdasm_stretchbltV_XRGB8888_to_XRGB8888_MMX
_vdasm_stretchbltV_XRGB8888_to_XRGB8888_MMX:
		push		ebp
		push		edi
		push		esi
		push		ebx

		mov			eax, [esp+20+16]
		and			eax, 0ff000000h
		mov			ebx, [esp+8+16]
		mov			ecx, [esp+12+16]
		jz			.noreverse
		xchg		ebx, ecx
		js			.noreverse
		neg			eax
		xchg		ebx, ecx
.noreverse:
		shr			eax, 16
		mov			[esp+20+16], eax
		mov			edx, [esp+4+16]
		mov			eax, [esp+16+16]
		add			eax, eax
		add			eax, eax
		lea			ebx, [ebx+eax-4]
		lea			ecx, [ecx+eax-4]
		lea			edx, [edx+eax-4]
		neg			eax

		movd		mm4, dword [esp+20+16]
		punpcklwd	mm4, mm4
		punpckldq	mm4, mm4

		movq		mm6, [rb_mask_888]
		movq		mm7, [g_mask_888]

.xstart:
		add			eax, 4
		jbe			.doodd
.xloop:
		movq		mm0, [ebx+eax]
		movq		mm1, [ecx+eax]
		movq		mm2, mm0
		movq		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		pand		mm0, mm6
		pand		mm1, mm6

		psubw		mm3, mm2
		psubw		mm1, mm0

		pmulhw		mm3, mm4
		pmulhw		mm1, mm4

		psubw		mm0, mm1
		psubw		mm2, mm3

		pand		mm0, mm6

		psllw		mm2, 8

		paddw		mm0, mm2

		movq		qword [edx+eax], mm0
		add			eax, 8
		jnc			.xloop

.doodd:
		sub			eax, 4
		jz			.noodd
.odd:
		movd		mm0, dword [ebx]
		movd		mm1, dword [ecx]
		movq		mm2, mm0
		movq		mm3, mm1
		psrlw		mm2, 8
		psrlw		mm3, 8
		pand		mm0, mm6
		pand		mm1, mm6

		psubw		mm3, mm2
		psubw		mm1, mm0

		pmulhw		mm3, mm4
		pmulhw		mm1, mm4

		psubw		mm0, mm1
		psubw		mm2, mm3

		pand		mm0, mm6

		psllw		mm2, 8

		paddw		mm0, mm2

		movd		dword [edx], mm0

.noodd:
		emms
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret


		end
