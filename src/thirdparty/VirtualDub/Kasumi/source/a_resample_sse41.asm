		segment	.rdata, align=16

round		dq		0000000000002000h
colround	dq		0000200000002000h

		segment	.text
		
		global		_vdasm_resize_table_row_8_k8_4x_SSE41
_vdasm_resize_table_row_8_k8_4x_SSE41:
		push		ebp
		push		edi
		push		esi
		push		ebx

		movq		xmm6, [round]
		pshufd		xmm6, xmm6, 0

		mov			ebp, [esp +  4 + 16]		;ebp = dst
		mov			esi, [esp + 12 + 16]		;esi = width
		mov			edi, [esp + 16 + 16]		;edi = kernel
.yloop:
		;eax = temp
		;ebx = temp
		;ecx = temp
		;edx = temp
		;esi = horiz counter
		;edi = filter list
		;ebp = destination

		mov			eax, [edi+0]
		mov			ebx, [edi+4]
		mov			ecx, [edi+8]
		mov			edx, [esp+8+16]
		add			eax, edx
		add			ebx, edx
		add			ecx, edx
		add			edx, [edi+12]

		pmovzxbw	xmm0, [eax]
		pmaddwd		xmm0, [edi+10h]
		pmovzxbw	xmm1, [ebx]
		pmaddwd		xmm1, [edi+20h]
		pmovzxbw	xmm2, [ecx]
		pmaddwd		xmm2, [edi+30h]
		pmovzxbw	xmm3, [edx]
		pmaddwd		xmm3, [edi+40h]
		add			edi, 50h
		phaddd		xmm0, xmm1
		phaddd		xmm2, xmm3
		phaddd		xmm0, xmm2
		paddd		xmm0, xmm6
		psrad		xmm0, 14
		packssdw	xmm0, xmm0
		packuswb	xmm0, xmm0
		movd		[ebp], xmm0

		add			ebp, 4
		sub			esi, 1
		jne			.yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

		global		_vdasm_resize_table_row_8_k16_4x_SSE41
_vdasm_resize_table_row_8_k16_4x_SSE41:
		push		ebp
		push		edi
		push		esi
		push		ebx

		movq		xmm6, [round]
		pshufd		xmm6, xmm6, 0

		mov			ebp, [esp +  4 + 16]		;ebp = dst
		mov			esi, [esp + 12 + 16]		;esi = width
		mov			edi, [esp + 16 + 16]		;edi = kernel
.yloop:
		;eax = temp
		;ebx = temp
		;ecx = temp
		;edx = temp
		;esi = horiz counter
		;edi = filter list
		;ebp = destination

		mov			eax, [edi+0]
		mov			ebx, [edi+4]
		mov			ecx, [edi+8]
		mov			edx, [esp+8+16]
		add			eax, edx
		add			ebx, edx
		add			ecx, edx
		add			edx, [edi+12]

		pmovzxbw	xmm0, [eax]
		pmaddwd		xmm0, [edi+10h]
		pmovzxbw	xmm1, [ebx]
		pmaddwd		xmm1, [edi+20h]
		pmovzxbw	xmm2, [ecx]
		pmaddwd		xmm2, [edi+30h]
		pmovzxbw	xmm3, [edx]
		pmaddwd		xmm3, [edi+40h]
		pmovzxbw	xmm4, [eax+8]
		pmaddwd		xmm4, [edi+50h]
		pmovzxbw	xmm5, [ebx+8]
		pmaddwd		xmm5, [edi+60h]
		paddd		xmm0, xmm4
		pmovzxbw	xmm4, [ecx+8]
		pmaddwd		xmm4, [edi+70h]
		paddd		xmm1, xmm5
		pmovzxbw	xmm5, [edx+8]
		pmaddwd		xmm5, [edi+80h]
		paddd		xmm2, xmm4
		paddd		xmm3, xmm5
		add			edi, 90h
		phaddd		xmm0, xmm1
		phaddd		xmm2, xmm3
		phaddd		xmm0, xmm2
		paddd		xmm0, xmm6
		psrad		xmm0, 14
		packssdw	xmm0, xmm0
		packuswb	xmm0, xmm0
		movd		[ebp], xmm0

		add			ebp, 4
		sub			esi, 1
		jne			.yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

		global		_vdasm_resize_table_row_8_SSE41
_vdasm_resize_table_row_8_SSE41:
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		xmm7, xmm7
		movq		xmm6, [round]

		mov			edi, [esp +  4 + 16]		;edi = dst
		mov			ebx, [esp +  8 + 16]		;ebx = src
		mov			ebp, [esp + 12 + 16]		;ebp = width
		mov			edx, [esp + 16 + 16]		;edx = kernel
.yloop:
		;eax = temp
		;ebx = source base address
		;ecx = (temp) source
		;edx = filter list
		;esi = (temp) kernel width
		;edi = destination
		;ebp = horiz counter

		mov			eax, [edx]
		add			edx, 16
		lea			ecx, [ebx + eax]
		mov			esi, [esp + 20 + 16]		;esi = kernel width

		movq		xmm2, xmm6
.xloop:
		pmovzxbw	xmm0, [ecx]
		add			ecx, 8
		pmaddwd		xmm0, [edx]
		paddd		xmm2, xmm0
		add			edx, 16
		sub			esi, 8
		jne			.xloop

		phaddd		xmm2, xmm2
		phaddd		xmm2, xmm2
		psrad		xmm2, 14
		packssdw	xmm2, xmm2
		packuswb	xmm2, xmm2
		movd		eax, xmm2
		mov			[edi], al
		add			edi, 1
		sub			ebp, 1
		jne			.yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret
		

		global		_vdasm_resize_table_col_8_k2_SSE41
_vdasm_resize_table_col_8_k2_SSE41:
		push		ebp
		push		edi
		push		esi
		push		ebx

		movq		xmm6, [colround]
		pshufd		xmm6, xmm6, 0

		mov			esi, [esp +  4 + 16]		;esi = dst
		mov			edi, [esp + 16 + 16]		;edi = kernel
		mov			ebp, [esp + 12 + 16]		;ebp = width

		movq		xmm7, [edi]
		pshufd		xmm7, xmm7, 0

		mov			edx, [esp +  8 + 16]		;ebx = srcs
		mov			eax, [edx+0]
		mov			ebx, [edx+4]
		add			eax, ebp
		add			ebx, ebp
		neg			ebp
		
.yloop:
		;eax = row0
		;ebx = row1
		;ecx =
		;edx =
		;edi = kernel
		;esi = dest
		;ebp = width counter

		movd		xmm0, [eax+ebp]
		movd		xmm2, [ebx+ebp]
		punpcklbw	xmm0, xmm2
		pmovzxbw	xmm0, xmm0
		pmaddwd		xmm0, xmm7

		paddd		xmm0, xmm6

		psrad		xmm0, 14
		packssdw	xmm0, xmm0
		packuswb	xmm0, xmm0
		movd		[esi], xmm0
		add			esi, 4
		add			ebp, 4
		jnz			.yloop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

		global		_vdasm_resize_table_col_8_k4_SSE41
_vdasm_resize_table_col_8_k4_SSE41:
		push		ebp
		push		edi
		push		esi
		push		ebx

		movq		xmm7, [colround]
		pshufd		xmm7, xmm7, 0

		mov			esi, [esp +  4 + 16]		;esi = dst
		mov			edi, [esp + 16 + 16]		;edi = kernel

		movdqu		xmm6, [edi]
		pshufd		xmm5, xmm6, 0
		pshufd		xmm6, xmm6, 0aah

		mov			edx, [esp +  8 + 16]		;ebx = srcs
		mov			ebp, [esp + 12 + 16]
		mov			eax, [edx+0]
		mov			ebx, [edx+4]
		mov			ecx, [edx+8]
		mov			edx, [edx+12]
		lea			eax, [eax+ebp-4]
		lea			ebx, [ebx+ebp-4]
		lea			ecx, [ecx+ebp-4]
		lea			edx, [edx+ebp-4]
		lea			esi, [esi+ebp-4]
		neg			ebp
		add			ebp,4
		jz			.odd
.yloop:
		;eax = row0
		;ebx = row1
		;ecx = row2
		;edx = row3
		;edi = kernel
		;esi = dest
		;ebp = width counter

		movd		xmm0, [eax+ebp]
		movd		xmm1, [ebx+ebp]
		punpcklbw	xmm0, xmm1

		movd		xmm1, [ecx+ebp]
		movd		xmm2, [edx+ebp]
		punpcklbw	xmm1, xmm2

		movd		xmm2, [eax+ebp+4]
		movd		xmm3, [ebx+ebp+4]
		punpcklbw	xmm2, xmm3
		
		movd		xmm3, [ecx+ebp+4]
		movd		xmm4, [edx+ebp+4]
		punpcklbw	xmm3, xmm4
		
		pmovzxbw	xmm0, xmm0
		pmaddwd		xmm0, xmm5
		
		pmovzxbw	xmm1, xmm1
		pmaddwd		xmm1, xmm6
		
		pmovzxbw	xmm2, xmm2
		pmaddwd		xmm2, xmm5
		
		pmovzxbw	xmm3, xmm3
		pmaddwd		xmm3, xmm6

		paddd		xmm0, xmm1
		paddd		xmm2, xmm3

		paddd		xmm0, xmm7
		paddd		xmm2, xmm7

		psrad		xmm0, 14
		psrad		xmm2, 14
		
		packssdw	xmm0, xmm2
		packuswb	xmm0, xmm0
		movq		[esi+ebp], xmm0
		add			ebp, 8
		js			.yloop
		jnz			.noodd

.odd:
		movd		xmm0, [eax]
		movd		xmm1, [ebx]
		movd		xmm2, [ecx]
		movd		xmm3, [edx]
		punpcklbw	xmm0, xmm1
		punpcklbw	xmm2, xmm3
		pmovzxbw	xmm0, xmm0
		pmovzxbw	xmm2, xmm2
		pmaddwd		xmm0, xmm5
		pmaddwd		xmm2, xmm6
		paddd		xmm0, xmm2
		paddd		xmm0, xmm7
		psrad		xmm0, 14
		packssdw	xmm0, xmm0
		packuswb	xmm0, xmm0
		movd		[esi], xmm0
.noodd:

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

		end
