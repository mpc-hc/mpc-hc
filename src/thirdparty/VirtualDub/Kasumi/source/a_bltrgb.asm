		section	.text

		global	_vdasm_pixblt_RGB565_to_XRGB1555
_vdasm_pixblt_RGB565_to_XRGB1555:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-2]
		lea		ecx, [ecx+ebp-2]
		neg		ebp
		mov		[esp+20+16], ebp

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 2
		jbe		.odd

.xloop:
		mov		eax, [ecx+ebp]
		mov		ebx, 0ffc0ffc0h

		and		ebx, eax
		and		eax, 0001f001fh

		shr		ebx, 1

		add		eax, ebx

		mov		[edx+ebp], eax
		add		ebp, 4

		jnc		.xloop
		jnz		.noodd
.odd:
		movzx	eax, word [ecx]
		mov		ebx, 0ffc0ffc0h
		and		ebx, eax
		and		eax, 0001f001fh
		shr		ebx, 1
		add		eax, ebx
		mov		[edx], ax
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret


		global	_vdasm_pixblt_RGB888_to_XRGB1555
_vdasm_pixblt_RGB888_to_XRGB1555:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		esi,[esp+12+16]
		mov		edi,[esp+4+16]

		mov		ebp,[esp+20+16]
		lea		eax,[ebp+ebp]
		lea		ebx,[ebp+eax]
		sub		[esp+8+16],eax
		sub		[esp+16+16],ebx

		mov		edx,[esp+24+16]
.yloop:
		mov		ebp,[esp+20+16]
		push	ebp
		push	edx
		shr		ebp,1
		jz		.checkodd
.xloop:
		mov		eax,[esi+2]		;u
		add		esi,6			;v

		mov		ebx,eax			;u
		mov		ecx,eax			;v
		shr		ebx,11			;u
		and		ecx,00f80000h	;v
		shr		eax,17			;u
		and		ebx,0000001fh	;v
		shr		ecx,14			;u
		and		eax,00007c00h	;v
		or		ebx,ecx			;u
		add		edi,4			;v
		or		ebx,eax			;u

		mov		ecx,[esi-6]		;v
		mov		edx,ebx			;u
		mov		eax,ecx			;v

		shl		edx,16			;u
		mov		ebx,ecx			;v
		shr		ebx,3			;u
		and		ecx,0000f800h	;v
		shr		eax,9			;u
		and		ebx,0000001fh	;v
		shr		ecx,6			;u
		and		eax,00007c00h	;v
		or		eax,ecx			;u
		or		edx,ebx			;v
		or		edx,eax			;u
		sub		ebp,1			;v
		mov		[edi-4],edx		;u
		jne		.xloop			;v
.checkodd:
		pop		edx
		pop		ebp
		and		ebp,1
		jz		.noodd
		movzx	eax,word [esi]
		movzx	ebx,byte [esi+2]
		shl		ebx,16
		add		esi,3
		add		eax,ebx

		mov		ebx,eax
		mov		ecx,eax
		shr		ebx,3
		and		ecx,0000f800h
		shr		eax,9
		and		ebx,0000001fh
		shr		ecx,6
		and		eax,00007c00h
		or		ebx,ecx
		or		ebx,eax
		mov		[edi+0],bl
		mov		[edi+1],bh
		add		edi,2
.noodd:

		add		esi,[esp+16+16]
		add		edi,[esp+ 8+16]

		sub		edx,1
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp

		ret

		global	_vdasm_pixblt_XRGB8888_to_XRGB1555
_vdasm_pixblt_XRGB8888_to_XRGB1555:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebp, [esp+20+16]
		mov		edx, [esp+4+16]
		add		ebp, ebp
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-2]
		lea		ecx, [ecx+ebp*2-4]
		neg		ebp
		mov		[esp+20+16], ebp

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 2
		jbe		.odd

.xloop:
		mov		eax, [ecx+ebp*2]
		mov		ebx, 00f80000h
		and		ebx, eax
		mov		esi, eax
		shr		ebx, 9
		and		esi, 0000f800h
		shr		esi, 6
		and		eax, 000000f8h
		shr		eax, 3
		add		ebx, esi
		mov		esi, [ecx+ebp*2+4]
		add		eax, ebx
		mov		ebx, esi
		and		esi, 00f80000h
		shl		esi, 7
		mov		edi, ebx
		and		edi, 0000f800h
		add		eax, esi
		shl		edi, 10
		and		ebx, 000000f8h
		shl		ebx, 13
		add		eax, edi
		add		eax, ebx
		mov		[edx+ebp], eax
		add		ebp, 4
		jnc		.xloop
		jnz		.noodd
.odd:
		mov		eax, [ecx]
		mov		ebx, 00f80000h
		and		ebx, eax
		mov		esi, eax
		shr		ebx, 9
		and		esi, 0000f800h
		shr		esi, 6
		and		eax, 000000f8h
		shr		eax, 3
		add		ebx, esi
		add		eax, ebx
		mov		[edx], ax
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		dword [esp+24+16]
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret

		global	_vdasm_pixblt_XRGB1555_to_RGB565
_vdasm_pixblt_XRGB1555_to_RGB565:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebp, [esp+20+16]
		mov		edi, [esp+24+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-2]
		lea		ecx, [ecx+ebp-2]
		neg		ebp
		mov		[esp+20+16], ebp

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 2
		jbe		.odd

.xloop:
		mov		eax, [ecx+ebp]
		mov		ebx, 02000200h

		mov		esi, eax
		and		ebx, eax

		shr		ebx, 4
		and		esi, 0ffe0ffe0h

		add		eax, esi

		add		eax, ebx

		mov		[edx+ebp], eax
		add		ebp, 4

		jnc		.xloop
		jnz		.noodd
.odd:
		movzx	eax, word [ecx]
		mov		ebx, 02000200h
		mov		esi, eax
		and		ebx, eax
		shr		ebx, 4
		and		esi, 0ffe0ffe0h
		add		eax, esi
		add		eax, ebx
		mov		[edx], ax
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		edi
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret

		global	_vdasm_pixblt_RGB888_to_RGB565
_vdasm_pixblt_RGB888_to_RGB565:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		esi,[esp+12+16]
		mov		edi,[esp+4+16]

		mov		ebp,[esp+20+16]
		lea		eax,[ebp+ebp]
		lea		ebx,[ebp+eax]
		sub		[esp+8+16],eax
		sub		[esp+16+16],ebx

		mov		edx,[esp+24+16]
.yloop:
		mov		ebp,[esp+20+16]
		push	ebp
		push	edx
		shr		ebp,1
		jz		.checkodd
.xloop:
		mov		eax,[esi+2]		;u
		add		esi,6			;v

		mov		ebx,eax			;u
		mov		ecx,eax			;v
		shr		ebx,11			;u
		and		ecx,00fc0000h	;v
		shr		eax,16			;u
		and		ebx,0000001fh	;v
		shr		ecx,13			;u
		and		eax,0000f800h	;v
		or		ebx,ecx			;u
		add		edi,4			;v
		or		ebx,eax			;u

		mov		ecx,[esi-6]		;v
		mov		edx,ebx			;u
		mov		eax,ecx			;v

		shl		edx,16			;u
		mov		ebx,ecx			;v
		shr		ebx,3			;u
		and		ecx,0000fc00h	;v
		shr		eax,8			;u
		and		ebx,0000001fh	;v
		shr		ecx,5			;u
		and		eax,0000f800h	;v
		or		eax,ecx			;u
		or		edx,ebx			;v
		or		edx,eax			;u
		sub		ebp,1			;v
		mov		[edi-4],edx		;u
		jne		.xloop			;v
.checkodd:
		pop		edx
		pop		ebp
		and		ebp,1
		jz		.noodd
		movzx	eax,word [esi]
		movzx	ebx,byte [esi+2]
		shl		ebx,16
		add		esi,3
		add		eax,ebx

		mov		ebx,eax
		mov		ecx,eax
		shr		ebx,3
		and		ecx,0000fc00h
		shr		eax,8
		and		ebx,0000001fh
		shr		ecx,5
		and		eax,0000f800h
		or		ebx,ecx
		or		ebx,eax
		mov		[edi+0],bl
		mov		[edi+1],bh
		add		edi,2
.noodd:

		add		esi,[esp+16+16]
		add		edi,[esp+ 8+16]

		sub		edx,1
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp

		ret

		global	_vdasm_pixblt_XRGB8888_to_RGB565
_vdasm_pixblt_XRGB8888_to_RGB565:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebp, [esp+20+16]
		mov		edx, [esp+4+16]
		add		ebp, ebp
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp-2]
		lea		ecx, [ecx+ebp*2-4]
		neg		ebp
		mov		[esp+20+16], ebp

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 2
		jbe		.odd

.xloop:
		mov		eax, [ecx+ebp*2]
		mov		ebx, 00f80000h
		and		ebx, eax
		mov		esi, eax
		shr		ebx, 8
		and		esi, 0000fc00h
		shr		esi, 5
		and		eax, 000000f8h
		shr		eax, 3
		add		ebx, esi
		mov		esi, [ecx+ebp*2+4]
		add		eax, ebx
		mov		ebx, esi
		and		esi, 00f80000h
		shl		esi, 8
		mov		edi, ebx
		and		edi, 0000fc00h
		add		eax, esi
		shl		edi, 11
		and		ebx, 000000f8h
		shl		ebx, 13
		add		eax, edi
		add		eax, ebx
		mov		[edx+ebp], eax
		add		ebp, 4
		jnc		.xloop
		jnz		.noodd
.odd:
		mov		eax, [ecx]
		mov		ebx, 00f80000h
		and		ebx, eax
		mov		esi, eax
		shr		ebx, 8
		and		esi, 0000fc00h
		shr		esi, 5
		and		eax, 000000f8h
		shr		eax, 3
		add		ebx, esi
		add		eax, ebx
		mov		[edx], ax
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		dword [esp+24+16]
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret


		global	_vdasm_pixblt_XRGB8888_to_RGB888
_vdasm_pixblt_XRGB8888_to_RGB888:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		esi,[esp+12+16]
		mov		edi,[esp+4+16]

		mov		ecx,[esp+20+16]
		lea		eax,[ecx+ecx*2]
		lea		ebx,[ecx*4]
		sub		[esp+8+16],eax
		sub		[esp+16+16],ebx

		mov		edx,[esp+24+16]
.yloop:
		mov		ecx,[esp+20+16]
		push	ecx
		push	edx
		shr		ecx,2
		jz		.checkodd
.xloop:
		mov		eax,[esi]		;EAX = xxr0g0b0
		mov		ebx,[esi+4]		;EBX = xxr1g1b1
		mov		edx,ebx			;EDX = xxr1g1b1
		mov		ebp,[esi+8]		;EBP = xxr2g2b2
		shl		ebx,24			;EBX = b1000000
		and		eax,00ffffffh		;EAX = 00r0g0b0
		shr		edx,8			;EDX = 00xxr1g1
		or		eax,ebx			;EAX = b1r0g0b0
		mov		[edi],eax
		mov		ebx,ebp			;EBX = xxr2g2b2
		shl		ebp,16			;EBP = g2b20000
		and		edx,0000ffffh		;EDX = 0000r1g1
		or		ebp,edx			;EBP = g2b2r1g1
		mov		eax,[esi+12]		;EAX = xxr3g3b3
		shr		ebx,16			;EBX = 0000xxr2
		add		edi,12
		shl		eax,8			;EAX = r3g3b300
		and		ebx,000000ffh		;EBX = 000000r2
		or		eax,ebx			;EAX = r3g3b3r2
		mov		[edi+4-12],ebp
		add		esi,16
		mov		[edi+8-12],eax
		sub		ecx,1
		jne		.xloop
.checkodd:
		pop		edx
		pop		ecx
		and		ecx,3
		jz		.noodd
.oddloop:
		mov		eax,[esi]
		add		esi,4
		mov		[edi],ax
		shr		eax,16
		mov		[edi+2],al
		add		edi,3
		sub		ecx,1
		jnz		.oddloop
.noodd:
		add		esi,[esp+16+16]
		add		edi,[esp+ 8+16]

		sub		edx,1
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret

		global	_vdasm_pixblt_XRGB1555_to_XRGB8888
_vdasm_pixblt_XRGB1555_to_XRGB8888:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebp, [esp+20+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp*2-4]
		lea		ecx, [ecx+ebp-2]
		neg		ebp
		mov		[esp+20+16], ebp

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 2
		jbe		.odd

.xloop:
		mov		eax, [ecx+ebp]
		mov		ebx, 00007c00h
		and		ebx, eax
		mov		esi, eax
		shl		ebx, 9
		and		esi, 000003e0h
		shl		esi, 6
		mov		edi, eax
		and		eax, 0000001fh
		add		ebx, esi
		shl		eax, 3
		mov		esi, edi
		shr		edi, 7
		add		eax, ebx
		and		edi, 00f80000h
		mov		ebx, esi
		shr		esi, 13
		and		ebx, 03e00000h
		shr		ebx, 10
		and		esi, 000000f8h
		add		ebx, edi
		add		ebx, esi
		mov		edi, eax
		and		eax, 00e0e0e0h
		shr		eax, 5
		mov		esi, ebx
		shr		ebx, 5
		add		eax, edi
		and		ebx, 00070707h
		add		ebx, esi
		mov		[edx+ebp*2], eax
		mov		[edx+ebp*2+4], ebx
		add		ebp, 4
		jnc		.xloop
		jnz		.noodd
.odd:
		movzx	eax, word [ecx]
		mov		ebx, 00007c00h
		and		ebx, eax
		mov		esi, eax
		shl		ebx, 9
		and		esi, 000003e0h
		shl		esi, 6
		and		eax, 0000001fh
		shl		eax, 3
		add		ebx, esi
		add		eax, ebx
		mov		ebx, 00e0e0e0h
		and		ebx, eax
		shr		ebx, 5
		add		eax, ebx
		mov		[edx], eax
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		dword [esp+24+16]
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret


		global	_vdasm_pixblt_RGB565_to_XRGB8888
_vdasm_pixblt_RGB565_to_XRGB8888:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebp, [esp+20+16]
		add		ebp, ebp
		mov		edx, [esp+4+16]
		mov		ecx, [esp+12+16]
		lea		edx, [edx+ebp*2-4]
		lea		ecx, [ecx+ebp-2]
		neg		ebp
		mov		[esp+20+16], ebp

.yloop:
		mov		ebp, [esp+20+16]
		add		ebp, 2
		jbe		.odd

.xloop:
		movzx	eax, word [ecx+ebp]
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
		mov		[edx+ebp*2], eax

		movzx	eax, word [ecx+ebp+2]
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
		mov		[edx+ebp*2+4], eax

		add		ebp, 4

		jnc		.xloop
		jnz		.noodd
.odd:
		movzx	eax, word [ecx]
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
		mov		[edx], eax
.noodd:
		add		ecx, [esp+16+16]
		add		edx, [esp+8+16]
		dec		dword [esp+24+16]
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret


		global	_vdasm_pixblt_RGB888_to_XRGB8888
_vdasm_pixblt_RGB888_to_XRGB8888:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		esi,[esp+12+16]
		mov		edi,[esp+4+16]

		mov		ecx,[esp+20+16]
		lea		eax,[ecx+ecx*2]
		lea		ebx,[ecx*4]
		sub		[esp+8+16],ebx
		sub		[esp+16+16],eax

		mov		edx,[esp+24+16]
.yloop:
		mov		ebp,[esp+20+16]
		shr		ebp,2
		push	edx
		jz		.checkodd
.xloop:
		mov		eax,[esi]			;EAX: b1r0g0b0
		mov		ebx,[esi+4]			;EBX: g2b2r1g1

		mov		[edi],eax
		mov		ecx,ebx				;ECX: g2b2r1g1

		shr		eax,24				;EAX: ------b1
		mov		edx,[esi+8]			;EDX: r3g3b3r2

		shr		ecx,16				;ECX: ----g2b2
		add		edi,16

		shl		ebx,8				;EBX: b2r1g1--
		add		esi,12

		or		eax,ebx				;EAX: b2r1g1b1
		mov		ebx,edx				;EBX: r3g3b3r2

		shr		ebx,8				;EBX: --r3g3b3
		mov		[edi+4-16],eax

		shl		edx,16				;EDX: b3r2----
		mov		[edi+12-16],ebx

		or		edx,ecx				;EDX: b3r2g2b2
		sub		ebp,1

		mov		[edi+8-16],edx
		jne		.xloop

.checkodd:
		pop		edx
		mov		ebx,[esp+20+16]
		and		ebx,3
		jz		.noodd
.oddloop:
		mov		ax,[esi]
		mov		cl,[esi+2]
		mov		[edi],ax
		mov		[edi+2],cl
		add		esi,3
		add		edi,4
		sub		ebx,1
		jne		.oddloop
.noodd:

		add		esi,[esp+16+16]
		add		edi,[esp+ 8+16]

		sub		edx,1
		jne		.yloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp

		ret

		end
