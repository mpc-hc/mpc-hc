		section	.rdata, rdata
		
		align	16

bytemasks	dd		000000ffh, 0000ffffh, 00ffffffh

		section	.text

;============================================================================

	global	_vdasm_pixblt_XRGB8888_to_YUV444Planar_scan_SSE2
_vdasm_pixblt_XRGB8888_to_YUV444Planar_scan_SSE2:
		push		edi
		push		esi
		push		ebx

		mov			eax, [esp+4+12]
		mov			ebx, [esp+8+12]
		mov			ecx, [esp+12+12]
		mov			edx, [esp+16+12]
		mov			esi, [esp+20+12]
		mov			edi, [esp+24+12]

		pcmpeqb		xmm6, xmm6
		psrlw		xmm6, 8				;xmm6 = 00FF x 8
		
		sub			esi, 4
		js			.postcheck
.xloop:
		movdqu		xmm2, [edx]			;xmm0 = X3R3G3B3X2R2G2B2X1R1G1B1X0R0G0B0
		add			edx, 16
		movdqa		xmm5, xmm2
		pand		xmm2, xmm6			;xmm0 =   R3  B3  R2  B2  R1  B1  R0  B0
		psrlw		xmm5, 8				;xmm1 =   X3  G3  X2  G2  X1  G1  X0  G0
		movdqa		xmm0, [edi+0]		;coeff_rb_to_y
		movdqa		xmm1, [edi+16]		;coeff_rb_to_u
		movdqa		xmm3, [edi+32]		;coeff_g_to_y
		movdqa		xmm4, [edi+48]		;coeff_g_to_u
		pmaddwd		xmm0, xmm2
		pmaddwd		xmm1, xmm2
		pmaddwd		xmm2, [edi+64]		;coeff_rb_to_v
		pmaddwd		xmm3, xmm5
		pmaddwd		xmm4, xmm5
		pmaddwd		xmm5, [edi+80]		;coeff_g_to_v
		paddd		xmm0, xmm3
		paddd		xmm1, xmm4
		paddd		xmm2, xmm5
		paddd		xmm0, [edi+96]		;bias_y
		paddd		xmm1, [edi+112]		;bias_c
		paddd		xmm2, [edi+112]		;bias_c
		psrad		xmm0, 15
		psrad		xmm1, 15
		psrad		xmm2, 15
		packssdw	xmm0, xmm0
		packssdw	xmm1, xmm1
		packssdw	xmm2, xmm2
		packuswb	xmm0, xmm0
		packuswb	xmm1, xmm1
		packuswb	xmm2, xmm2
		movd		[eax], xmm0
		movd		[ebx], xmm1
		movd		[ecx], xmm2
		add			eax, 4	
		add			ebx, 4	
		add			ecx, 4	
		sub			esi, 4
		jns			.xloop
.postcheck:
		jmp			dword [.finaltable + esi*4 + 16]
.complete:
		pop			ebx
		pop			esi
		pop			edi
		ret		

.finaltable:
		dd			.complete
		dd			.do1
		dd			.do2
		dd			.do3
		
.finaltable2:
		dd			.fin1
		dd			.fin2
		dd			.fin3

.do1:
		movd		xmm2, [edx]
		jmp			short .dofinal
.do2:
		movq		xmm2, [edx]
		jmp			short .dofinal
.do3:
		movq		xmm2, [edx]
		movd		xmm1, [edx]
		movlhps		xmm2, xmm1
.dofinal:
		movdqa		xmm5, xmm2
		pand		xmm2, xmm6			;xmm0 =   R3  B3  R2  B2  R1  B1  R0  B0
		psrlw		xmm5, 8				;xmm1 =   X3  G3  X2  G2  X1  G1  X0  G0
		movdqa		xmm0, [edi+0]		;coeff_rb_to_y
		movdqa		xmm1, [edi+16]		;coeff_rb_to_u
		movdqa		xmm3, [edi+32]		;coeff_g_to_y
		movdqa		xmm4, [edi+48]		;coeff_g_to_u
		pmaddwd		xmm0, xmm2
		pmaddwd		xmm1, xmm2
		pmaddwd		xmm2, [edi+64]		;coeff_rb_to_v
		pmaddwd		xmm3, xmm5
		pmaddwd		xmm4, xmm5
		pmaddwd		xmm5, [edi+80]		;coeff_g_to_v
		paddd		xmm0, xmm3
		paddd		xmm1, xmm4
		paddd		xmm2, xmm5
		paddd		xmm0, [edi+96]		;bias_y
		paddd		xmm1, [edi+112]		;bias_c
		paddd		xmm2, [edi+112]		;bias_c
		psrad		xmm0, 15
		psrad		xmm1, 15
		psrad		xmm2, 15
		packssdw	xmm0, xmm0
		packssdw	xmm1, xmm1
		packssdw	xmm2, xmm2
		packuswb	xmm0, xmm0
		packuswb	xmm1, xmm1
		movd		xmm7, [bytemasks + esi*4 + 12]
		packuswb	xmm2, xmm2
		
		jmp			dword [.finaltable2 + esi*4 + 12]
		
.fin1:
		movd		edx, xmm0
		mov			[eax], dl
		movd		edx, xmm1
		mov			[ebx], dl
		movd		edx, xmm2
		mov			[ecx], dl
		jmp			.complete
.fin2:
		movd		edx, xmm0
		mov			[eax], dx
		movd		edx, xmm1
		mov			[ebx], dx
		movd		edx, xmm2
		mov			[ecx], dx
		jmp			.complete
.fin3:
		movd		edx, xmm0
		mov			[eax], dx
		shr			edx, 16
		mov			[eax+2], dl
		movd		edx, xmm1
		mov			[ebx], dx
		shr			edx, 16
		mov			[ebx+2], dl
		movd		edx, xmm2
		mov			[ecx], dx
		shr			edx, 16
		mov			[ecx+2], dl
		jmp			.complete

		end
