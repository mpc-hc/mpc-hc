;	VirtualDub - Video processing and capture application
;	Copyright (C) 1998-2001 Avery Lee
;
;	This program is free software; you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation; either version 2 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.
;
;	You should have received a copy of the GNU General Public License
;	along with this program; if not, write to the Free Software
;	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	extern _YUV_Y_table: dword
	extern _YUV_U_table: dword
	extern _YUV_V_table: dword
	extern _YUV_clip_table: byte
	extern _YUV_clip_table16: byte

	segment	.rdata, align=16

	align	16

	global	_asm_YUVtoRGB_row_constants_SSE2		
_asm_YUVtoRGB_row_constants_SSE2:
SSE2_80w	dq	00080008000800080h, 00080008000800080h
SSE2_Ublucoeff	dq	00081008100810081h, 00081008100810081h
SSE2_Vredcoeff	dq	00066006600660066h, 00066006600660066h
SSE2_Ugrncoeff	dq	0FFE7FFE7FFE7FFE7h, 0FFE7FFE7FFE7FFE7h
SSE2_Vgrncoeff	dq	0FFCCFFCCFFCCFFCCh, 0FFCCFFCCFFCCFFCCh
SSE2_Ylow	dq	000FF00FF00FF00FFh, 000FF00FF00FF00FFh
SSE2_Ybias	dq	00010001000100010h, 00010001000100010h
SSE2_Ycoeff	dq	0004A004A004A004Ah, 0004A004A004A004Ah

SSE2_Ucoeff0	dq	000810000FFE70081h, 0FFE700810000FFE7h
SSE2_Ucoeff1	dq	00000FFE700810000h, 000810000FFE70081h
SSE2_Ucoeff2	dq	0FFE700810000FFE7h, 00000FFE700810000h
SSE2_Vcoeff0	dq	000000066FFCC0000h, 0FFCC00000066FFCCh
SSE2_Vcoeff1	dq	00066FFCC00000066h, 000000066FFCC0000h
SSE2_Vcoeff2	dq	0FFCC00000066FFCCh, 00066FFCC00000066h

%assign		offs_var_begin		0
%assign		offs_rgb_pitch		offs_var_begin + 0
%assign		offs_y_pitch		offs_var_begin + 4
%assign		offs_uv_pitch		offs_var_begin + 8
%assign		offs_width			offs_var_begin + 12
%assign		offs_height			offs_var_begin + 16

%assign		offs_const_begin	32

%assign		offs_SSE2_80w		offs_const_begin + 0
%assign		offs_SSE2_Ublucoeff	offs_const_begin + 16
%assign		offs_SSE2_Vredcoeff	offs_const_begin + 32
%assign		offs_SSE2_Ugrncoeff	offs_const_begin + 48
%assign		offs_SSE2_Vgrncoeff	offs_const_begin + 64
%assign		offs_SSE2_Ylow		offs_const_begin + 80
%assign		offs_SSE2_Ybias		offs_const_begin + 96
%assign		offs_SSE2_Ycoeff	offs_const_begin + 112

%assign		offs_SSE2_Ucoeff0	offs_const_begin + 128
%assign		offs_SSE2_Ucoeff1	offs_const_begin + 144
%assign		offs_SSE2_Ucoeff2	offs_const_begin + 160
%assign		offs_SSE2_Vcoeff0	offs_const_begin + 176
%assign		offs_SSE2_Vcoeff1	offs_const_begin + 192
%assign		offs_SSE2_Vcoeff2	offs_const_begin + 208


MMX_10w		dq	00010001000100010h
MMX_80w		dq	00080008000800080h
MMX_00FFw	dq	000FF00FF00FF00FFh
MMX_FF00w	dq	0FF00FF00FF00FF00h
MMX_Ublucoeff	dq	00081008100810081h
MMX_Vredcoeff	dq	00066006600660066h
MMX_Ugrncoeff	dq	0FFE7FFE7FFE7FFE7h
MMX_Vgrncoeff	dq	0FFCCFFCCFFCCFFCCh
MMX_Ycoeff	dq	0004A004A004A004Ah
MMX_rbmask	dq	07c1f7c1f7c1f7c1fh
MMX_grnmask	dq	003e003e003e003e0h
MMX_grnmask2	dq	000f800f800f800f8h
MMX_clip	dq	07c007c007c007c00h

MMX_Ucoeff0	dq	000810000FFE70081h
MMX_Ucoeff1	dq	0FFE700810000FFE7h
MMX_Ucoeff2	dq	00000FFE700810000h
MMX_Vcoeff0	dq	000000066FFCC0000h
MMX_Vcoeff1	dq	0FFCC00000066FFCCh
MMX_Vcoeff2	dq	00066FFCC00000066h

	segment	.text

	global _asm_YUVtoRGB32_row
	global _asm_YUVtoRGB32_row_MMX
	global _asm_YUVtoRGB32_row_ISSE
	global _asm_YUVtoRGB32_row_SSE2
	global _asm_YUVtoRGB24_row
	global _asm_YUVtoRGB24_row_MMX
	global _asm_YUVtoRGB24_SSE2
	global _asm_YUVtoRGB16_row
	global _asm_YUVtoRGB16_row_MMX
	global _asm_YUVtoRGB16_row_ISSE

;	asm_YUVtoRGB_row(
;		Pixel *ARGB1_pointer,
;		Pixel *ARGB2_pointer,
;		YUVPixel *Y1_pointer,
;		YUVPixel *Y2_pointer,
;		YUVPixel *U_pointer,
;		YUVPixel *V_pointer,
;		long width
;		);

%define	ARGB1_pointer	[esp+ 4+16]
%define	ARGB2_pointer	[esp+ 8+16]
%define	Y1_pointer		[esp+12+16]
%define	Y2_pointer		[esp+16+16]
%define	U_pointer		[esp+20+16]
%define	V_pointer		[esp+24+16]
%define	count			[esp+28+16]
%define	context_pointer [esp+32+16+8]

_asm_YUVtoRGB32_row:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,3
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer			;[C]
	mov	edi,V_pointer			;[C]
	xor	edx,edx				;[C]
	xor	ecx,ecx				;[C]
	jmp	short col_loop_start

col_loop:
	mov	ch,[_YUV_clip_table+ebx-3f00h]	;[4] edx = [0][0][red][green]
	mov	esi,U_pointer			;[C]
	shl	ecx,8				;[4] edx = [0][red][green][0]
	mov	edi,V_pointer			;[C]
	mov	cl,[_YUV_clip_table+edx-3f00h]	;[4] edx = [0][r][g][b] !!
	xor	edx,edx				;[C]
	mov	[eax+ebp*8-4],ecx		;[4] 
	xor	ecx,ecx				;[C]
col_loop_start:
	mov	cl,[esi + ebp]			;[C] eax = U
	mov	dl,[edi + ebp]			;[C] ebx = V
	mov	eax,Y1_pointer			;[1] 
	xor	ebx,ebx				;[1] 
	mov	esi,[_YUV_U_table + ecx*4]	;[C] eax = [b impact][u-g impact]
	mov	ecx,[_YUV_V_table + edx*4]	;[C] ebx = [r impact][v-g impact]
	mov	edi,esi				;[C]
	mov	bl,[eax + ebp*2]		;[1] ebx = Y1 value
	shr	esi,16				;[C] eax = blue impact
	add	edi,ecx				;[C] edi = [junk][g impact]
	mov	ebx,[_YUV_Y_table + ebx*4]	;[1] ebx = Y impact
	and	ecx,0ffff0000h			;[C]
	mov	edx,ebx				;[1] edx = Y impact
	add	esi,ecx				;[C] eax = [r impact][b impact]
	and	edi,0000ffffh			;[C]
	add	ebx,esi				;[1] ebx = [red][blue]
	mov	ecx,ebx				;[1] edi = [red][blue]
	and	edx,0000ffffh			;[1] ecx = green
	shr	ebx,16				;[1] ebx = red
	and	ecx,0000ffffh			;[1] edi = blue
	mov	dl,[_YUV_clip_table+edx+edi-3f00h]	;[1] edx = [0][0][junk][green]
	mov	eax,Y1_pointer			;[2] 
	mov	dh,[_YUV_clip_table+ebx-3f00h]	;[1] edx = [0][0][red][green]
	xor	ebx,ebx				;[2] 
	shl	edx,8				;[1] edx = [0][red][green][0]
	mov	bl,[eax + ebp*2 + 1]		;[2] ebx = Y1 value
	mov	eax,ARGB1_pointer		;[1] 
	mov	dl,[_YUV_clip_table+ecx-3f00h]	;[1] edx = [0][r][g][b] !!
	mov	ebx,[_YUV_Y_table + ebx*4]	;[2] ebx = Y impact
	mov	ecx,0000ffffh			;[2] 

	and	ecx,ebx				;[2]
	add	ebx,esi				;[2] ebx = [red][blue]

	mov	[eax+ebp*8],edx			;[1] 
	mov	edx,ebx				;[2]

	shr	ebx,16				;[2] ebx = red
	mov	eax,Y2_pointer			;[3] 

	and	edx,0000ffffh			;[2]
	mov	cl,[_YUV_clip_table+ecx+edi-3f00h]	;[2] edx = [0][0][junk][green]	

	mov	al,[eax + ebp*2]		;[3] ebx = Y1 value
	mov	ch,[_YUV_clip_table+ebx-3f00h]	;[2] edx = [0][0][red][green]

	shl	ecx,8				;[2] edx = [0][red][green][0]
	and	eax,000000ffh			;[3] 

	mov	cl,[_YUV_clip_table+edx-3f00h]	;[2] edx = [0][r][g][b] !!
	mov	edx,ARGB1_pointer		;[2] 

	mov	ebx,[_YUV_Y_table + eax*4]	;[3] ebx = Y impact
	mov	eax,0000ffffh

	and	eax,ebx				;[3] edi = [red][blue]
	add	ebx,esi				;[3] ebx = [red][blue]

	mov	[edx+ebp*8+4],ecx		;[2] 
	mov	edx,ebx				;[3]

	shr	ebx,16				;[3] ebx = red
	mov	ecx,Y2_pointer			;[4] 

	and	edx,0000ffffh			;[3] ecx = green
	mov	al,[_YUV_clip_table+eax+edi-3f00h]	;[3] edx = [0][0][junk][green]

	mov	cl,[ecx + ebp*2+1]		;[4] ebx = Y1 value
	mov	ah,[_YUV_clip_table+ebx-3f00h]	;[3] edx = [0][0][red][green]

	shl	eax,8				;[3] edx = [0][red][green][0]
	and	ecx,000000ffh			;[4] 

	mov	al,[_YUV_clip_table+edx-3f00h]	;[3] edx = [0][r][g][b] !!
	mov	edx,ARGB2_pointer		;[3] 

	mov	ebx,[_YUV_Y_table + ecx*4]	;[4] ebx = Y impact
	mov	ecx,0000ffffh			;[4]

	and	ecx,ebx				;[4] ecx = [0][Y-impact]
	add	ebx,esi				;[4] ebx = [red][blue]

	mov	[edx+ebp*8],eax			;[3] 
	mov	edx,ebx				;[4] edx = [red][blue]

	shr	ebx,16				;[4] ebx = red
	mov	cl,[_YUV_clip_table+ecx+edi-3f00h]	;[4] edx = [0][0][junk][green]

	and	edx,0000ffffh			;[4] edx = blue
	mov	eax,ARGB2_pointer		;[4] 

	inc	ebp

	jnz	col_loop

	mov	ch,[_YUV_clip_table+ebx-3f00h]	;[4] edx = [0][0][red][green]
	shl	ecx,8				;[4] edx = [0][red][green][0]
	mov	cl,[_YUV_clip_table+edx-3f00h]	;[4] edx = [0][r][g][b] !!
	mov	[eax+ebp*8-4],ecx		;[4] 

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

;MMX_test	dq	7060504030201000h

_asm_YUVtoRGB32_row_MMX:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,3
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_MMX:
	movd	mm0,dword [esi+ebp]	;U (byte)
	pxor	mm7,mm7

	movd	mm1,dword [edi+ebp]	;V (byte)
	punpcklbw mm0,mm7		;U (word)

	psubw	mm0,[MMX_80w]
	punpcklbw mm1,mm7		;V (word)

	psubw	mm1,[MMX_80w]
	movq	mm2,mm0

	pmullw	mm2,[MMX_Ugrncoeff]
	movq	mm3,mm1

	pmullw	mm3,[MMX_Vgrncoeff]
	pmullw	mm0,[MMX_Ublucoeff]
	pmullw	mm1,[MMX_Vredcoeff]
	paddw	mm2,mm3

	;mm0: blue
	;mm1: red
	;mm2: green

	movq	mm6,[ecx+ebp*2]		;Y
	pand	mm6,[MMX_00FFw]
	psubw	mm6,[MMX_10w]
	pmullw	mm6,[MMX_Ycoeff]
	movq	mm4,mm6
	paddw	mm6,mm0			;mm6: <B3><B2><B1><B0>
	movq	mm5,mm4
	paddw	mm4,mm1			;mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;mm5: <G3><G2><G1><G0>
	psraw	mm6,6
	psraw	mm4,6
	packuswb mm6,mm6		;mm6: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm4,mm4		;mm4: R3R2R1R0R3R2R1R0
	punpcklbw mm6,mm4		;mm6: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm4,mm6
	punpcklbw mm6,mm5		;mm6: G1R1G1B2G0R0G0B0
	punpckhbw mm4,mm5		;mm4: G3R3G3B3G2R2G2B2

	movq	mm7,[ecx+ebp*2]		;Y
	psrlw	mm7,8
	psubw	mm7,[MMX_10w]
	pmullw	mm7,[MMX_Ycoeff]
	movq	mm3,mm7
	paddw	mm7,mm0			;mm7: final blue
	movq	mm5,mm3
	paddw	mm3,mm1			;mm3: final red
	paddw	mm5,mm2			;mm5: final green
	psraw	mm7,6
	psraw	mm3,6
	packuswb mm7,mm7		;mm7: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm3,mm3		;mm3: R3R2R1R0R3R2R1R0
	punpcklbw mm7,mm3		;mm7: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm3,mm7
	punpcklbw mm7,mm5		;mm7: G1R1G1B2G0R0G0B0
	punpckhbw mm3,mm5		;mm3: G3R3G3B3G2R2G2B2

	;mm3	P7:P5
	;mm4	P6:P4
	;mm6	P2:P0
	;mm7	P3:P1

	movq	mm5,mm6
	punpckldq mm5,mm7		;P1:P0
	punpckhdq mm6,mm7		;P3:P2
	movq	mm7,mm4
	punpckldq mm4,mm3		;P5:P4
	punpckhdq mm7,mm3		;P7:P6

	movq	[eax+ebp*8],mm5
	movq	[eax+ebp*8+8],mm6
	movq	[eax+ebp*8+16],mm4
	movq	[eax+ebp*8+24],mm7

	movq	mm6,[edx+ebp*2]		;Y
	pand	mm6,[MMX_00FFw]
	psubw	mm6,[MMX_10w]
	pmullw	mm6,[MMX_Ycoeff]
	movq	mm4,mm6
	paddw	mm6,mm0			;mm6: <B3><B2><B1><B0>
	movq	mm5,mm4
	paddw	mm4,mm1			;mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;mm5: <G3><G2><G1><G0>
	psraw	mm6,6
	psraw	mm4,6
	packuswb mm6,mm6		;mm6: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm4,mm4		;mm4: R3R2R1R0R3R2R1R0
	punpcklbw mm6,mm4		;mm6: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm4,mm6
	punpcklbw mm6,mm5		;mm6: G1R1G1B2G0R0G0B0
	punpckhbw mm4,mm5		;mm4: G3R3G3B3G2R2G2B2

	movq	mm7,[edx+ebp*2]		;Y
	psrlw	mm7,8
	psubw	mm7,[MMX_10w]
	pmullw	mm7,[MMX_Ycoeff]
	movq	mm3,mm7
	paddw	mm7,mm0			;mm7: final blue
	movq	mm5,mm3
	paddw	mm3,mm1			;mm3: final red
	paddw	mm5,mm2			;mm5: final green
	psraw	mm7,6
	psraw	mm3,6
	packuswb mm7,mm7		;mm7: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm3,mm3		;mm3: R3R2R1R0R3R2R1R0
	punpcklbw mm7,mm3		;mm7: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm3,mm7
	punpcklbw mm7,mm5		;mm7: G1R1G1B2G0R0G0B0
	punpckhbw mm3,mm5		;mm3: G3R3G3B3G2R2G2B2

	;mm3	P7:P5
	;mm4	P6:P4
	;mm6	P2:P0
	;mm7	P3:P1

	movq	mm5,mm6
	punpckldq mm5,mm7		;P1:P0
	punpckhdq mm6,mm7		;P3:P2
	movq	mm7,mm4
	punpckldq mm4,mm3		;P5:P4
	punpckhdq mm7,mm3		;P7:P6

	movq	[ebx+ebp*8   ],mm5
	movq	[ebx+ebp*8+ 8],mm6

	movq	[ebx+ebp*8+16],mm4
	movq	[ebx+ebp*8+24],mm7

	add	ebp,4

	jnz	col_loop_MMX

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

;**************************************************************************
;
;	asm_YUVtoRGB24_row(
;		Pixel *ARGB1_pointer,
;		Pixel *ARGB2_pointer,
;		YUVPixel *Y1_pointer,
;		YUVPixel *Y2_pointer,
;		YUVPixel *U_pointer,
;		YUVPixel *V_pointer,
;		long width
;		);

%define	ARGB1_pointer	[esp+ 4+16]
%define	ARGB2_pointer	[esp+ 8+16]
%define	Y1_pointer		[esp+12+16]
%define	Y2_pointer		[esp+16+16]
%define	U_pointer		[esp+20+16]
%define	V_pointer		[esp+24+16]
%define	count			[esp+28+16]

_asm_YUVtoRGB24_row:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	add	eax,eax
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer			;[C]
	mov	edi,V_pointer			;[C]
	xor	edx,edx				;[C]
	xor	ecx,ecx				;[C]

col_loop24:
	mov	esi,U_pointer
	mov	edi,V_pointer
	xor	eax,eax
	xor	ebx,ebx
	mov	al,[esi + ebp]			;eax = U
	mov	bl,[edi + ebp]			;ebx = V
	mov	eax,[_YUV_U_table + eax*4]	;eax = [b impact][u-g impact]
	mov	edi,[_YUV_V_table + ebx*4]	;edi = [r impact][v-g impact]

	mov	ecx,eax				;[C]
	mov	esi,Y1_pointer			;[1]

	mov	edx,edi				;[C]
	xor	ebx,ebx				;[1]

	shr	eax,16				;[C] eax = blue impact
	mov	bl,[esi + ebp*2]		;[1] ebx = Y1 value

	and	edi,0ffff0000h			;[C] edi = [r impact][0]
	add	ecx,edx				;[C] ecx = [junk][g impact]

	add	eax,edi				;[C] eax = [r impact][b impact]
	mov	ebx,[_YUV_Y_table + ebx*4]	;[1] ebx = Y impact

	;eax = [r][b]
	;ecx = [g]

	mov	esi,ebx				;[1]
	add	ebx,eax				;[1] ebx = [red][blue]

	add	esi,ecx				;[1] edx = [junk][green]
	mov	edi,ebx				;[1] edi = [red][blue]

	shr	ebx,16				;[1] ebx = red
	and	esi,0000ffffh			;[1] ecx = green

	and	edi,0000ffffh			;edi = blue
	xor	edx,edx

	mov	bh,[_YUV_clip_table+ebx-3f00h]	;bh = red
	mov	dl,[_YUV_clip_table+esi-3f00h]	;dl = green

	mov	esi,Y1_pointer			;[2]
	mov	bl,[_YUV_clip_table+edi-3f00h]	;bl = blue

	mov	edi,ARGB1_pointer		;[1]
	mov	[edi+2],bh			;[1]

	mov	[edi+0],bl			;[1]
	xor	ebx,ebx				;[2]

	mov	[edi+1],dl			;[1]

	mov	bl,[esi + ebp*2 + 1]		;[2] ebx = Y1 value
	mov	esi,ecx				;[2]

	mov	ebx,[_YUV_Y_table + ebx*4]	;[2] ebx = Y impact
	mov	edi,0000ffffh			;[2]

	add	esi,ebx				;[2] edx = [junk][green]
	add	ebx,eax				;[2] ebx = [red][blue]

	and	edi,ebx				;[2] edi = blue
	and	esi,0000ffffh			;[2] ecx = green

	shr	ebx,16				;ebx = red
	xor	edx,edx

	mov	bh,[_YUV_clip_table+ebx-3f00h]	;bh = red
	mov	dl,[_YUV_clip_table+esi-3f00h]	;dl = green

	mov	esi,Y2_pointer			;[3]
	mov	bl,[_YUV_clip_table+edi-3f00h]	;bl = blue

	mov	edi,ARGB1_pointer		;[2]
	mov	[edi+5],bh			;[2]

	mov	[edi+4],dl			;[2]
	mov	[edi+3],bl			;[2]

	xor	ebx,ebx				;[3]

	mov	bl,[esi + ebp*2]		;[3] ebx = Y1 value
	mov	edi,ecx				;[2]

	mov	ebx,[_YUV_Y_table + ebx*4]	;[3] ebx = Y impact
	mov	esi,0000ffffh			;[3]

	add	edi,ebx				;[3] edx = [junk][green]
	add	ebx,eax				;[3] ebx = [red][blue]

	and	esi,ebx				;[3] edi = blue
	and	edi,0000ffffh			;ecx = green

	shr	ebx,16				;ebx = red
	xor	edx,edx

	mov	dl,[_YUV_clip_table+edi-3f00h]	;dl = green
	mov	edi,ARGB2_pointer		;[3]

	mov	bh,[_YUV_clip_table+ebx-3f00h]	;bh = red
	mov	bl,[_YUV_clip_table+esi-3f00h]	;bl = blue

	mov	esi,Y2_pointer			;[4]
	mov	[edi+2],bh

	mov	[edi+0],bl
	xor	ebx,ebx				;[4]

	mov	[edi+1],dl
	mov	bl,[esi + ebp*2 + 1]		;[4] ebx = Y1 value

	mov	edi,0000ffffh			;[4]

	mov	ebx,[_YUV_Y_table + ebx*4]	;[4] ebx = Y impact
	xor	edx,edx

	add	ecx,ebx				;[4] ecx = [junk][green]
	add	ebx,eax				;ebx = [red][blue]

	and	edi,ebx				;edi = blue
	and	ecx,0000ffffh			;ecx = green

	shr	ebx,16				;ebx = red
	mov	esi,ARGB2_pointer

	mov	bl,[_YUV_clip_table+ebx-3f00h]	;bh = red
	mov	dl,[_YUV_clip_table+ecx-3f00h]	;dl = green

	mov	al,[_YUV_clip_table+edi-3f00h]	;bl = blue
	mov	[esi+5],bl

	mov	[esi+4],dl
	mov	ecx,ARGB1_pointer

	mov	[esi+3],al
	add	esi,6

	mov	ARGB2_pointer,esi
	add	ecx,6

	mov	ARGB1_pointer,ecx

	inc	ebp
	jnz	col_loop24

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

_asm_YUVtoRGB24_row_MMX:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	add	eax,eax
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_MMX24:
	movd		mm0,dword [esi+ebp]	;U (byte)
	pxor		mm7,mm7

	movd		mm1,dword [edi+ebp]	;V (byte)
	punpcklbw mm0,mm7		;U (word)

	movd		mm2,dword [ecx+ebp*2]	;Y low
	punpcklbw mm1,mm7		;V (word)

	movd		mm3,dword [edx+ebp*2]	;Y high
	punpcklbw mm2,mm7		;Y1 (word)

	psubw		mm2,[MMX_10w]
	punpcklbw mm3,mm7		;Y2 (word)

	psubw		mm3,[MMX_10w]

	psubw		mm0,[MMX_80w]
	psubw		mm1,[MMX_80w]

	;group 1

	pmullw		mm2,[MMX_Ycoeff]	;[lazy]
	movq		mm6,mm0
	pmullw		mm3,[MMX_Ycoeff]	;[lazy]
	movq		mm7,mm1
	punpcklwd	mm6,mm6		;mm6 = U1U1U0U0
	movq		mm4,mm2		;mm4 = Y3Y2Y1Y0		[high]
	punpckldq	mm6,mm6		;mm6 = U0U0U0U0
	movq		mm5,mm3		;mm3 = Y3Y2Y1Y0		[low]
	punpcklwd	mm7,mm7		;mm7 = V1V1V0V0
	punpckldq	mm7,mm7		;mm7 = V0V0V0V0

	pmullw		mm6,[MMX_Ucoeff0]
	punpcklwd	mm4,mm4		;mm4 = Y1Y1Y0Y0		[high]
	pmullw		mm7,[MMX_Vcoeff0]
	punpcklwd	mm5,mm5		;mm5 = Y1Y1Y0Y0		[low]

	punpcklwd	mm4,mm2		;mm4 = Y1Y0Y0Y0
	punpcklwd	mm5,mm3		;mm5 = Y1Y0Y0Y0

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	packuswb	mm4,mm4
	packuswb	mm5,mm5

	;group 2

	movd		dword [eax],mm4	;[lazy write]
	movq		mm4,mm0
	movd		dword [ebx],mm5	;[lazy write]
	movq		mm5,mm1

	punpcklwd	mm4,mm4		;mm6 = U1U1U0U0
	movq		mm6,mm2		;mm4 = Y3Y2Y1Y0		[high]
	punpcklwd	mm5,mm5		;mm6 = V1V1V0V0
	movq		mm7,mm3		;mm3 = Y3Y2Y1Y0		[low]

	pmullw		mm4,[MMX_Ucoeff1]
	psrlq		mm6,16		;mm4 = 00Y3Y2Y1		[high]
	pmullw		mm5,[MMX_Vcoeff1]
	psrlq		mm7,16		;mm4 = 00Y3Y2Y1		[low]

	punpcklwd	mm6,mm6		;mm4 = Y2Y2Y1Y1		[high]
	punpcklwd	mm7,mm7		;mm5 = Y2Y2Y1Y1		[high]

	paddw		mm6,mm4
	paddw		mm7,mm4
	paddw		mm6,mm5
	paddw		mm7,mm5

	psraw		mm6,6
	psraw		mm7,6

	packuswb	mm6,mm6
	packuswb	mm7,mm7

	;group 3

	movd		dword [eax+4],mm6	;[lazy write]
	movq		mm6,mm0
	movd		dword [ebx+4],mm7	;[lazy write]
	movq		mm7,mm1

	movq		mm4,mm2		;mm4 = Y3Y2Y1Y0		[high]
	punpcklwd	mm6,mm6		;mm6 = U1U1U0U0
	movq		mm5,mm3		;mm3 = Y3Y2Y1Y0		[low]
	punpckhdq	mm6,mm6		;mm6 = U1U1U1U1
	punpcklwd	mm7,mm7		;mm7 = V1V1V0V0
	punpckhdq	mm7,mm7		;mm7 = V1V1V1V1

	pmullw		mm6,[MMX_Ucoeff2]
	punpckhwd	mm2,mm2		;mm2 = Y3Y3Y2Y2		[high]
	pmullw		mm7,[MMX_Vcoeff2]
	punpckhwd	mm3,mm3		;mm3 = Y3Y3Y2Y2		[low]

	punpckhdq	mm4,mm2		;mm4 = Y3Y3Y3Y2		[high]
	punpckhdq	mm5,mm3		;mm5 = Y3Y3Y3Y2		[low]

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	;next 3 groups

	movd		mm2,dword [ecx+ebp*2+4]	;Y low
	packuswb	mm4,mm4		;[lazy]

	movd		mm3,dword [edx+ebp*2+4]	;Y high
	packuswb	mm5,mm5		;[lazy]

	movd		dword [eax+8],mm4	;[lazy write]
	pxor		mm7,mm7

	movd		dword [ebx+8],mm5	;[lazy write]
	punpcklbw	mm2,mm7		;U (word)


	psubw		mm2,[MMX_10w]
	punpcklbw	mm3,mm7		;V (word)

	psubw		mm3,[MMX_10w]


	;group 1

	pmullw		mm2,[MMX_Ycoeff]	;[init]
	movq		mm6,mm0

	pmullw		mm3,[MMX_Ycoeff]	;[init]
	punpckhwd	mm6,mm6		;mm6 = U3U3U2U2

	movq		mm7,mm1
	punpckldq	mm6,mm6		;mm6 = U2U2U2U2
	movq		mm4,mm2		;mm4 = Y3Y2Y1Y0		[high]
	punpckhwd	mm7,mm7		;mm7 = V3V3V2V2
	movq		mm5,mm3		;mm3 = Y3Y2Y1Y0		[low]
	punpckldq	mm7,mm7		;mm7 = V2V2V2V2

	pmullw		mm6,[MMX_Ucoeff0]
	punpcklwd	mm4,mm4		;mm4 = Y1Y1Y0Y0		[high]
	pmullw		mm7,[MMX_Vcoeff0]
	punpcklwd	mm5,mm5		;mm5 = Y1Y1Y0Y0		[low]

	punpcklwd	mm4,mm2		;mm4 = Y1Y0Y0Y0
	punpcklwd	mm5,mm3		;mm5 = Y1Y0Y0Y0

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	packuswb	mm4,mm4
	packuswb	mm5,mm5

	;group 2

	movd		dword [eax+12],mm4
	movq		mm6,mm0
	movd		dword [ebx+12],mm5
	movq		mm7,mm1

	punpckhwd	mm6,mm6		;mm6 = U3U3U2U2
	movq		mm4,mm2		;mm4 = Y3Y2Y1Y0		[high]
	punpckhwd	mm7,mm7		;mm6 = V3V3V2V2
	movq		mm5,mm3		;mm3 = Y3Y2Y1Y0		[low]

	pmullw		mm6,[MMX_Ucoeff1]
	psrlq		mm4,16		;mm4 = 00Y3Y2Y1		[high]
	pmullw		mm7,[MMX_Vcoeff1]
	psrlq		mm5,16		;mm4 = 00Y3Y2Y1		[low]

	punpcklwd	mm4,mm4		;mm4 = Y2Y2Y1Y1		[high]
	punpcklwd	mm5,mm5		;mm5 = Y2Y2Y1Y1		[high]

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	packuswb	mm4,mm4
	packuswb	mm5,mm5

	;group 3

	movq		mm6,mm2		;mm4 = Y3Y2Y1Y0		[high]
	punpckhwd	mm0,mm0		;mm6 = U3U3U2U2

	movq		mm7,mm3		;mm3 = Y3Y2Y1Y0		[low]
	punpckhdq	mm0,mm0		;mm6 = U3U3U3U3

	movd		dword [eax+16],mm4	;[lazy write]
	punpckhwd	mm1,mm1		;mm7 = V3V3V2V2

	movd		dword [ebx+16],mm5	;[lazy write]
	punpckhdq	mm1,mm1		;mm7 = V3V3V3V3

	pmullw		mm0,[MMX_Ucoeff2]
	punpckhwd	mm2,mm2		;mm2 = Y3Y3Y2Y2		[high]
	pmullw		mm1,[MMX_Vcoeff2]
	punpckhwd	mm3,mm3		;mm3 = Y3Y3Y2Y2		[low]

	punpckhdq	mm6,mm2		;mm4 = Y3Y3Y3Y2		[high]
	punpckhdq	mm7,mm3		;mm5 = Y3Y3Y3Y2		[low]

	paddw		mm6,mm0
	paddw		mm7,mm0
	paddw		mm6,mm1
	paddw		mm7,mm1

	psraw		mm6,6
	psraw		mm7,6

	packuswb	mm6,mm6
	packuswb	mm7,mm7

	movd		dword [eax+20],mm6
	add	eax,24
	movd		dword [ebx+20],mm7
	add	ebx,24

	;done

	add	ebp,4
	jnz	col_loop_MMX24

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

;**************************************************************************

_asm_YUVtoRGB16_row:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,2
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	eax,eax
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer			;[C]
	mov	edi,V_pointer			;[C]
	xor	edx,edx				;[C]
	xor	ecx,ecx				;[C]

col_loop16:
	mov	esi,U_pointer
	mov	edi,V_pointer
	xor	eax,eax
	xor	ebx,ebx
	mov	al,[esi + ebp]			;eax = U
	mov	bl,[edi + ebp]			;ebx = V
	mov	eax,[_YUV_U_table + eax*4]	;eax = [b impact][u-g impact]
	mov	edi,[_YUV_V_table + ebx*4]	;edi = [r impact][v-g impact]

	mov	ecx,eax				;[C]
	mov	esi,Y1_pointer			;[1]

	mov	edx,edi				;[C]
	xor	ebx,ebx				;[1]

	shr	eax,16				;[C] eax = blue impact
	mov	bl,[esi + ebp*2]		;[1] ebx = Y1 value

	and	edi,0ffff0000h			;[C] edi = [r impact][0]
	add	ecx,edx				;[C] ecx = [junk][g impact]

	add	eax,edi				;[C] eax = [r impact][b impact]
	mov	ebx,[_YUV_Y_table + ebx*4]	;[1] ebx = Y impact

	;eax = [r][b]
	;ecx = [g]

	mov	esi,ebx				;[1]
	add	ebx,eax				;[1] ebx = [red][blue]

	add	esi,ecx				;[1] edx = [junk][green]
	mov	edi,ebx				;[1] edi = [red][blue]

	shr	ebx,16				;[1] ebx = red
	and	esi,0000ffffh			;[1] ecx = green

	and	edi,0000ffffh			;edi = blue
	xor	edx,edx

	mov	bh,[_YUV_clip_table16+ebx-3f00h]	;bh = red
	mov	dl,[_YUV_clip_table16+esi-3f00h]	;dl = green

	mov	bl,[_YUV_clip_table16+edi-3f00h]	;bl = blue
	xor	dh,dh				;[1]

	shl	bh,2				;[1]
	mov	edi,ARGB1_pointer		;[1]

	shl	edx,5				;[1]
	mov	esi,Y1_pointer			;[2]

	add	edx,ebx				;[1]
	xor	ebx,ebx				;[2]

	mov	[edi+ebp*4+0],dl		;[1]
	mov	bl,[esi + ebp*2 + 1]		;[2] ebx = Y1 value

	mov	[edi+ebp*4+1],dh		;[1]
	mov	esi,ecx				;[2]

	mov	ebx,[_YUV_Y_table + ebx*4]	;[2] ebx = Y impact
	mov	edi,0000ffffh			;[2]

	add	esi,ebx				;[2] edx = [junk][green]
	add	ebx,eax				;[2] ebx = [red][blue]

	and	edi,ebx				;[2] edi = blue
	and	esi,0000ffffh			;[2] ecx = green

	shr	ebx,16				;ebx = red
	xor	edx,edx

	mov	bh,[_YUV_clip_table16+ebx-3f00h]	;bh = red

	mov	dl,[_YUV_clip_table16+esi-3f00h]	;dl = green
	mov	bl,[_YUV_clip_table16+edi-3f00h]	;bl = blue

	shl	edx,5				;[2]
	mov	edi,ARGB1_pointer		;[2]

	shl	bh,2				;[2]
	mov	esi,Y2_pointer			;[3]

	add	edx,ebx				;[2]
	xor	ebx,ebx				;[3]

	mov	[edi+ebp*4+2],dl		;[2]
	mov	bl,[esi + ebp*2]		;[3] ebx = Y1 value

	mov	[edi+ebp*4+3],dh		;[2]
	mov	edi,ecx				;[2]

	mov	ebx,[_YUV_Y_table + ebx*4]	;[3] ebx = Y impact
	mov	esi,0000ffffh			;[3]

	add	edi,ebx				;[3] edx = [junk][green]
	add	ebx,eax				;[3] ebx = [red][blue]

	and	esi,ebx				;[3] edi = blue
	and	edi,0000ffffh			;ecx = green

	shr	ebx,16				;ebx = red
	xor	edx,edx

	mov	dl,[_YUV_clip_table16+edi-3f00h]	;dl = green
	mov	edi,ARGB2_pointer		;[3]

	shl	edx,5
	mov	bh,[_YUV_clip_table16+ebx-3f00h]	;bh = red

	mov	bl,[_YUV_clip_table16+esi-3f00h]	;bl = blue
	mov	esi,Y2_pointer			;[4]

	shl	bh,2				;[3]
	nop

	add	edx,ebx				;[3]
	xor	ebx,ebx				;[4]

	mov	[edi+ebp*4+0],dl		;[3]
	mov	bl,[esi + ebp*2 + 1]		;[4] ebx = Y1 value

	mov	[edi+ebp*4+1],dh		;[3]
	mov	edi,0000ffffh			;[4]

	mov	ebx,[_YUV_Y_table + ebx*4]	;[4] ebx = Y impact
	xor	edx,edx

	add	ecx,ebx				;[4] ecx = [junk][green]
	add	ebx,eax				;ebx = [red][blue]

	and	edi,ebx				;edi = blue
	and	ecx,0000ffffh			;ecx = green

	shr	ebx,16				;ebx = red
	mov	esi,ARGB2_pointer

	mov	dl,[_YUV_clip_table16+ecx-3f00h]	;dl = green
	mov	al,[_YUV_clip_table16+edi-3f00h]	;bl = blue

	shl	edx,5
	mov	ah,[_YUV_clip_table16+ebx-3f00h]	;bh = red

	shl	ah,2

	add	eax,edx

	mov	[esi+ebp*4+2],al
	mov	[esi+ebp*4+3],ah

	inc	ebp
	jnz	col_loop16

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret



_asm_YUVtoRGB16_row_MMX:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,2
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_MMX16:
	movd	mm0,dword [esi+ebp]		;[0       ] U (byte)
	pxor	mm7,mm7			;[0      7] 

	movd	mm1,dword [edi+ebp]		;[01     7] V (byte)
	punpcklbw mm0,mm7		;[01     7] U (word)

	psubw	mm0,[MMX_80w]		;[01     7] 
	punpcklbw mm1,mm7		;[01     7] V (word)

	psubw	mm1,[MMX_80w]		;[01      ] 
	movq	mm2,mm0			;[012     ] 

	pmullw	mm2,[MMX_Ugrncoeff]	;[012     ] 
	movq	mm3,mm1			;[0123    ] 

	;mm0: blue
	;mm1: red
	;mm2: green

	movq	mm6,[ecx+ebp*2]		;[0123  6 ] [1] Y
	;<-->

	pmullw	mm3,[MMX_Vgrncoeff]	;[0123    ] 
	movq	mm7,mm6			;[012   67] [2] Y

	pmullw	mm0,[MMX_Ublucoeff]	;[0123    ] 
	psrlw	mm7,8			;[012   67] [2]

	pmullw	mm1,[MMX_Vredcoeff]	;[0123    ] 
	;<-->

	pand	mm6,[MMX_00FFw]		;[012   67] [1]
	paddw	mm2,mm3			;[012   6 ] [C]

	psubw	mm6,[MMX_10w]		;[012   67] [1]

	pmullw	mm6,[MMX_Ycoeff]		;[012   67] [1]

	psubw	mm7,[MMX_10w]		;[012   67] [2]
	movq	mm4,mm6			;[012 4 67] [1]

	pmullw	mm7,[MMX_Ycoeff]		;[012   67] [2]
	movq	mm5,mm6			;[012 4567] [1]

	paddw	mm6,mm0			;[012 4 67] [1] mm6: <B3><B2><B1><B0>
	paddw	mm4,mm1			;[012 4567] [1] mm4: <R3><R2><R1><R0>

	paddw	mm5,mm2			;[012 4567] [1] mm5: <G3><G2><G1><G0>
	psraw	mm4,6			;[012 4567] [1]

	movq	mm3,mm7			;[01234567] [2]
	psraw	mm5,4			;[01234567] [1]

	paddw	mm7,mm0			;[01234567] [2] mm6: <B3><B2><B1><B0>
	psraw	mm6,6			;[01234567] [1]

	paddsw	mm5,[MMX_clip]
	packuswb mm6,mm6		;[01234567] [1] mm6: B3B2B1B0B3B2B1B0

	psubusw	mm5,[MMX_clip]
	packuswb mm4,mm4		;[01234567] [1] mm4: R3R2R1R0R3R2R1R0

	pand	mm5,[MMX_grnmask]		;[01234567] [1] mm7: <G3><G2><G1><G0>
	psrlq	mm6,2			;[01234567] [1]

	punpcklbw mm6,mm4		;[0123 567] [1] mm4: R3B3R2B2R1B1R0B0

	movq	mm4,[edx+ebp*2]		;[01234567] [3] Y
	psrlw	mm6,1			;[01234567] [1]

	pand	mm6,[MMX_rbmask]		;[01234567] [1] mm6: <RB3><RB2><RB1><RB0>

	por	mm6,mm5			;[01234 67] [1] mm6: P6P4P2P0
	movq	mm5,mm3			;[01234567] [2]

	paddw	mm3,mm1			;[01234567] [2] mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;[01234567] [2] mm5: <G3><G2><G1><G0>

	pand	mm4,[MMX_00FFw]		;[01234567] [3]
	psraw	mm3,6			;[01234567] [2]	

	psubw	mm4,[MMX_10w]		;[01234567] [3]
	psraw	mm5,4			;[01234567] [2]

	pmullw	mm4,[MMX_Ycoeff]		;[01234567] [3]
	psraw	mm7,6			;[01234567] [2]

	paddsw	mm5,[MMX_clip]
	packuswb mm3,mm3		;[01234567] [2] mm4: R3R2R1R0R3R2R1R0

	psubusw	mm5,[MMX_clip]
	packuswb mm7,mm7		;[01234567] [2] mm6: B3B2B1B0B3B2B1B0

	pand	mm5,[MMX_grnmask]		;[012 4567] [2] mm7: <G3><G2><G1><G0>
	psrlq	mm7,2			;[01234567] [2]

	punpcklbw mm7,mm3		;[012 4567] [2] mm6: R3B3R2B2R1B1R0B0

	movq	mm3,[edx+ebp*2]		;[01234567] [4] Y
	psrlw	mm7,1			;[01234567] [2]

	pand	mm7,[MMX_rbmask]		;[01234567] [2] mm6: <RB3><RB2><RB1><RB0>
	psrlw	mm3,8			;[01234567] [4]

	por	mm7,mm5			;[01234567] [2] mm7: P7P5P3P1
	movq	mm5,mm6			;[01234567] [A]

	psubw	mm3,[MMX_10w]		;[01234567] [4]
	punpcklwd mm6,mm7		;[01234567] [A] mm4: P3P2P1P0

	pmullw	mm3,[MMX_Ycoeff]		;[0123456 ] [4]
	punpckhwd mm5,mm7		;[0123456 ] [A} mm5: P7P6P5P4

	movq	[eax+ebp*4   ],mm6	;[012345  ] [A]
	movq	mm6,mm4			;[0123456 ] [3]

	movq	[eax+ebp*4+ 8],mm5	;[0123456 ] [A]
	paddw	mm6,mm0			;[01234 6 ] [3] mm6: <B3><B2><B1><B0>

	movq	mm5,mm4			;[0123456 ] [3]
	paddw	mm4,mm1			;[0123456 ] [3] mm4: <R3><R2><R1><R0>

	paddw	mm5,mm2			;[0123456 ] [3] mm5: <G3><G2><G1><G0>
	psraw	mm4,6			;[0123456 ] [3]

	movq	mm7,mm3			;[01234567] [4]
	psraw	mm5,4			;[01234567] [3]

	paddw	mm7,mm0			;[01234567] [4] mm6: <B3><B2><B1><B0>
	psraw	mm6,6			;[01234567] [3]

	movq	mm0,mm3			;[01234567] [4]
	packuswb mm4,mm4		;[01234567] [3] mm4: R3R2R1R0R3R2R1R0


	packuswb mm6,mm6		;[01 34567] [3] mm6: B3B2B1B0B3B2B1B0
	paddw	mm3,mm1			;[01234567] [4] mm4: <R3><R2><R1><R0>

	psrlq	mm6,2
	paddw	mm0,mm2			;[01 34567] [4] mm5: <G3><G2><G1><G0>

	paddsw	mm5,[MMX_clip]
	punpcklbw mm6,mm4		;[01 3 567] [3] mm6: B3B3B2B2B1B1B0B0

	psubusw	mm5,[MMX_clip]
	psrlw	mm6,1			;[01 3 567] [3]

	pand	mm6,[MMX_rbmask]		;[01 3 567] [3] mm6: <B3><B2><B1><B0>
	psraw	mm3,6			;[01 3 567] [4]

	pand	mm5,[MMX_grnmask]		;[01 3 567] [3] mm7: <G3><G2><G1><G0>
	psraw	mm0,4			;[01 3 567] [4]

	por	mm6,mm5			;[01 3  67] [3] mm4: P6P4P2P0	
	psraw	mm7,6			;[01 3  67] [4]

	paddsw	mm0,[MMX_clip]
	packuswb mm3,mm3		;[01 3  67] [4] mm4: R3R2R1R0R3R2R1R0

	psubusw	mm0,[MMX_clip]
	packuswb mm7,mm7		;[01 3  67] mm6: B3B2B1B0B3B2B1B0

	pand	mm0,[MMX_grnmask]		;[01    67] mm7: <G3><G2><G1><G0>
	psrlq	mm7,2

	punpcklbw mm7,mm3		;[01    67] mm6: R3B3R2B2R1B1R0B0
	movq	mm1,mm6

	psrlw	mm7,1
	add	ebp,4

	pand	mm7,[MMX_rbmask]		;[01    67] mm6: <B3><B2><B1><B0>

	por	mm0,mm7			;[01    67] mm0: P7P5P3P1

	punpcklwd mm6,mm0		;[01    6 ] mm4: P3P2P1P0

	punpckhwd mm1,mm0		;[ 1    6 ] mm5: P7P6P5P4
	movq	[ebx+ebp*4-16],mm6

	movq	[ebx+ebp*4- 8],mm1
	jnz	col_loop_MMX16

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

;--------------------------------------------------------------------------

_asm_YUVtoRGB32_row_ISSE:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,3
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_SSE:
	prefetchnta [esi+ebp+32]
	prefetchnta [edi+ebp+32]
	prefetchnta [ecx+ebp*2+32]
	prefetchnta [edx+ebp*2+32]

	movd	mm0,dword [esi+ebp]		;U (byte)
	pxor	mm7,mm7

	movd	mm1,dword [edi+ebp]		;V (byte)
	punpcklbw mm0,mm7		;U (word)

	psubw	mm0,[MMX_80w]
	punpcklbw mm1,mm7		;V (word)

	psubw	mm1,[MMX_80w]
	movq	mm2,mm0

	pmullw	mm2,[MMX_Ugrncoeff]
	movq	mm3,mm1

	pmullw	mm3,[MMX_Vgrncoeff]
	pmullw	mm0,[MMX_Ublucoeff]
	pmullw	mm1,[MMX_Vredcoeff]
	paddw	mm2,mm3

	;mm0: blue
	;mm1: red
	;mm2: green

	movq	mm6,[ecx+ebp*2]		;Y
	pand	mm6,[MMX_00FFw]
	psubw	mm6,[MMX_10w]
	pmullw	mm6,[MMX_Ycoeff]
	movq	mm4,mm6
	paddw	mm6,mm0			;mm6: <B3><B2><B1><B0>
	movq	mm5,mm4
	paddw	mm4,mm1			;mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;mm5: <G3><G2><G1><G0>
	psraw	mm6,6
	psraw	mm4,6
	packuswb mm6,mm6		;mm6: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm4,mm4		;mm4: R3R2R1R0R3R2R1R0
	punpcklbw mm6,mm4		;mm6: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm4,mm6
	punpcklbw mm6,mm5		;mm6: G1R1G1B2G0R0G0B0
	punpckhbw mm4,mm5		;mm4: G3R3G3B3G2R2G2B2

	movq	mm7,[ecx+ebp*2]		;Y
	psrlw	mm7,8
	psubw	mm7,[MMX_10w]
	pmullw	mm7,[MMX_Ycoeff]
	movq	mm3,mm7
	paddw	mm7,mm0			;mm7: final blue
	movq	mm5,mm3
	paddw	mm3,mm1			;mm3: final red
	paddw	mm5,mm2			;mm5: final green
	psraw	mm7,6
	psraw	mm3,6
	packuswb mm7,mm7		;mm7: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm3,mm3		;mm3: R3R2R1R0R3R2R1R0
	punpcklbw mm7,mm3		;mm7: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm3,mm7
	punpcklbw mm7,mm5		;mm7: G1R1G1B2G0R0G0B0
	punpckhbw mm3,mm5		;mm3: G3R3G3B3G2R2G2B2

	;mm3	P7:P5
	;mm4	P6:P4
	;mm6	P2:P0
	;mm7	P3:P1

	movq	mm5,mm6
	punpckldq mm5,mm7		;P1:P0
	punpckhdq mm6,mm7		;P3:P2
	movq	mm7,mm4
	punpckldq mm4,mm3		;P5:P4
	punpckhdq mm7,mm3		;P7:P6

	movntq	[eax+ebp*8],mm5
	movntq	[eax+ebp*8+8],mm6
	movntq	[eax+ebp*8+16],mm4
	movntq	[eax+ebp*8+24],mm7

	movq	mm6,[edx+ebp*2]		;Y
	pand	mm6,[MMX_00FFw]
	psubw	mm6,[MMX_10w]
	pmullw	mm6,[MMX_Ycoeff]
	movq	mm4,mm6
	paddw	mm6,mm0			;mm6: <B3><B2><B1><B0>
	movq	mm5,mm4
	paddw	mm4,mm1			;mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;mm5: <G3><G2><G1><G0>
	psraw	mm6,6
	psraw	mm4,6
	packuswb mm6,mm6		;mm6: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm4,mm4		;mm4: R3R2R1R0R3R2R1R0
	punpcklbw mm6,mm4		;mm6: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm4,mm6
	punpcklbw mm6,mm5		;mm6: G1R1G1B2G0R0G0B0
	punpckhbw mm4,mm5		;mm4: G3R3G3B3G2R2G2B2

	movq	mm7,[edx+ebp*2]		;Y
	psrlw	mm7,8
	psubw	mm7,[MMX_10w]
	pmullw	mm7,[MMX_Ycoeff]
	movq	mm3,mm7
	paddw	mm7,mm0			;mm7: final blue
	movq	mm5,mm3
	paddw	mm3,mm1			;mm3: final red
	paddw	mm5,mm2			;mm5: final green
	psraw	mm7,6
	psraw	mm3,6
	packuswb mm7,mm7		;mm7: B3B2B1B0B3B2B1B0
	psraw	mm5,6
	packuswb mm3,mm3		;mm3: R3R2R1R0R3R2R1R0
	punpcklbw mm7,mm3		;mm7: R3B3R2B2R1B1R0B0
	packuswb mm5,mm5		;mm5: G3G2G1G0G3G2G1G0
	punpcklbw mm5,mm5		;mm5: G3G3G2G2G1G1G0G0
	movq	mm3,mm7
	punpcklbw mm7,mm5		;mm7: G1R1G1B2G0R0G0B0
	punpckhbw mm3,mm5		;mm3: G3R3G3B3G2R2G2B2

	;mm3	P7:P5
	;mm4	P6:P4
	;mm6	P2:P0
	;mm7	P3:P1

	movq	mm5,mm6
	punpckldq mm5,mm7		;P1:P0
	punpckhdq mm6,mm7		;P3:P2
	movq	mm7,mm4
	punpckldq mm4,mm3		;P5:P4
	punpckhdq mm7,mm3		;P7:P6

	movntq	[ebx+ebp*8   ],mm5
	movntq	[ebx+ebp*8+ 8],mm6

	movntq	[ebx+ebp*8+16],mm4
	movntq	[ebx+ebp*8+24],mm7

	add	ebp,4

	jnz	col_loop_SSE

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

	global	_asm_YUVtoRGB24_row_ISSE
_asm_YUVtoRGB24_row_ISSE:
	;.FPO (7, 9, 0, 0, 0, 0)
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	add	eax,eax
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

	movd	mm0,esp
	sub	esp,20
	and	esp,-8
	movd	dword [esp+16],mm0

col_loop_ISSE24:
	prefetchnta	[esi+ebp+32]
	prefetchnta [edi+ebp+32]
	prefetchnta [ecx+ebp*2+32]
	prefetchnta [edx+ebp*2+32]

	movd		mm0,dword [esi+ebp]	;U (byte)
	pxor		mm7,mm7

	movd		mm1,dword [edi+ebp]	;V (byte)
	punpcklbw mm0,mm7		;U (word)

	movd		mm2,dword [ecx+ebp*2]	;Y low
	punpcklbw mm1,mm7		;V (word)

	movd		mm3,dword [edx+ebp*2]	;Y high
	punpcklbw mm2,mm7		;Y1 (word)

	psubw		mm2,[MMX_10w]
	punpcklbw mm3,mm7		;Y2 (word)

	psubw		mm3,[MMX_10w]

	psubw		mm0,[MMX_80w]
	psubw		mm1,[MMX_80w]

	movq		[esp+0],mm0
	movq		[esp+8],mm1

	;group 1

	pmullw		mm2,[MMX_Ycoeff]	;[lazy]
	pmullw		mm3,[MMX_Ycoeff]	;[lazy]

	pshufw		mm6,mm0,00000000b	;mm6 = U0U0U0U0
	pshufw		mm7,mm1,00000000b	;mm7 = V0V0V0V0

	pmullw		mm6,[MMX_Ucoeff0]
	pshufw		mm4,mm2,01000000b	;mm4 = Y1Y0Y0Y0 [high]
	pmullw		mm7,[MMX_Vcoeff0]
	pshufw		mm5,mm3,01000000b	;mm4 = Y1Y0Y0Y0 [low]

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	;group 2

	pshufw		mm6,[esp+0],01010000b	;mm6 = U1U1U0U0
	pshufw		mm7,[esp+8],01010000b	;mm7 = V1V1V0V0

	pmullw		mm6,[MMX_Ucoeff1]
	pshufw		mm0,mm2,10100101b	;mm0 = Y2Y2Y1Y1		[high]
	pmullw		mm7,[MMX_Vcoeff1]
	pshufw		mm1,mm3,10100101b	;mm1 = Y2Y2Y1Y1		[low]

	paddw		mm0,mm6
	paddw		mm1,mm6
	paddw		mm0,mm7
	paddw		mm1,mm7

	psraw		mm0,6
	psraw		mm1,6

	packuswb	mm4,mm0
	packuswb	mm5,mm1

	;group 3

	pshufw		mm6,[esp+0],01010101b	;mm6 = U1U1U1U1
	pshufw		mm7,[esp+8],01010101b	;mm7 = V1V1V1V1

	movntq		[eax],mm4	;[lazy write]
	movntq		[ebx],mm5	;[lazy write]

	pmullw		mm6,[MMX_Ucoeff2]
	pshufw		mm4,mm2,11111110b	;mm4 = Y3Y3Y3Y2		[high]
	pmullw		mm7,[MMX_Vcoeff2]
	pshufw		mm5,mm3,11111110b	;mm5 = Y3Y3Y3Y2		[low]

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	;next 3 groups

	movd		mm2,dword [ecx+ebp*2+4]	;Y low
	pxor		mm7,mm7

	movd		mm3,dword [edx+ebp*2+4]	;Y high
	punpcklbw	mm2,mm7		;U (word)

	psubw		mm2,[MMX_10w]
	punpcklbw	mm3,mm7		;V (word)

	psubw		mm3,[MMX_10w]


	;group 1

	pmullw		mm2,[MMX_Ycoeff]	;[init]
	pmullw		mm3,[MMX_Ycoeff]	;[init]

	pshufw		mm6,[esp+0],10101010b	;mm6 = U2U2U2U2
	pshufw		mm7,[esp+8],10101010b	;mm7 = V2V2V2V2

	pmullw		mm6,[MMX_Ucoeff0]
	pshufw		mm0,mm2,01000000b	;mm0 = Y1Y0Y0Y0 [high]
	pmullw		mm7,[MMX_Vcoeff0]
	pshufw		mm1,mm3,01000000b	;mm1 = Y1Y0Y0Y0 [low]

	paddw		mm0,mm6
	paddw		mm1,mm6
	paddw		mm0,mm7
	paddw		mm1,mm7

	psraw		mm0,6
	psraw		mm1,6

	packuswb	mm4,mm0
	packuswb	mm5,mm1

	;group 2

	pshufw		mm6,[esp+0],11111010b	;mm6 = U3U3U2U2
	pshufw		mm7,[esp+8],11111010b	;mm7 = V3V3V2V2

	movntq		[eax+8],mm4
	movntq		[ebx+8],mm5

	pmullw		mm6,[MMX_Ucoeff1]
	pshufw		mm4,mm2,10100101b	;mm4 = Y2Y2Y1Y1		[high]
	pmullw		mm7,[MMX_Vcoeff1]
	pshufw		mm5,mm3,10100101b	;mm5 = Y2Y2Y1Y1		[low]

	paddw		mm4,mm6
	paddw		mm5,mm6
	paddw		mm4,mm7
	paddw		mm5,mm7

	psraw		mm4,6
	psraw		mm5,6

	;group 3

	pshufw		mm0,[esp+0],11111111b	;mm6 = U3U3U3U3
	pshufw		mm1,[esp+8],11111111b	;mm7 = V3V3V3V3

	pmullw		mm0,[MMX_Ucoeff2]
	pshufw		mm2,mm2,11111110b	;mm6 = Y3Y3Y3Y2		[high]
	pmullw		mm1,[MMX_Vcoeff2]
	pshufw		mm3,mm3,11111110b	;mm7 = Y3Y3Y3Y2		[low]

	paddw		mm2,mm0
	paddw		mm3,mm0
	paddw		mm2,mm1
	paddw		mm3,mm1

	psraw		mm2,6
	psraw		mm3,6

	packuswb	mm4,mm2
	packuswb	mm5,mm3

	movntq		[eax+16],mm4
	add	eax,24
	movntq		[ebx+16],mm5
	add	ebx,24

	;done

	add	ebp,4
	jnz	col_loop_ISSE24

	mov	esp,[esp+16]

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

_asm_YUVtoRGB16_row_ISSE:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,2
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_ISSE16:
	prefetchnta [esi+ebp+32]
	prefetchnta [edi+ebp+32]

	movd	mm0,dword [esi+ebp]		;[0       ] U (byte)
	pxor	mm7,mm7			;[0      7] 

	movd	mm1,dword [edi+ebp]		;[01     7] V (byte)
	punpcklbw mm0,mm7		;[01     7] U (word)

	psubw	mm0,[MMX_80w]		;[01     7] 
	punpcklbw mm1,mm7		;[01     7] V (word)

	psubw	mm1,[MMX_80w]		;[01      ] 
	movq	mm2,mm0			;[012     ] 

	pmullw	mm2,[MMX_Ugrncoeff]	;[012     ] 
	movq	mm3,mm1			;[0123    ] 

	;mm0: blue
	;mm1: red
	;mm2: green

	prefetchnta [ecx+ebp*2+32]
	prefetchnta [edx+ebp*2+32]

	movq	mm6,[ecx+ebp*2]		;[0123  6 ] [1] Y
	;<-->

	pmullw	mm3,[MMX_Vgrncoeff]	;[0123    ] 
	movq	mm7,mm6			;[012   67] [2] Y

	pmullw	mm0,[MMX_Ublucoeff]	;[0123    ] 
	psrlw	mm7,8			;[012   67] [2]

	pmullw	mm1,[MMX_Vredcoeff]	;[0123    ] 
	;<-->

	pand	mm6,[MMX_00FFw]		;[012   67] [1]
	paddw	mm2,mm3			;[012   6 ] [C]

	psubw	mm6,[MMX_10w]		;[012   67] [1]

	pmullw	mm6,[MMX_Ycoeff]		;[012   67] [1]

	psubw	mm7,[MMX_10w]		;[012   67] [2]
	movq	mm4,mm6			;[012 4 67] [1]

	pmullw	mm7,[MMX_Ycoeff]		;[012   67] [2]
	movq	mm5,mm6			;[012 4567] [1]

	paddw	mm6,mm0			;[012 4 67] [1] mm6: <B3><B2><B1><B0>
	paddw	mm4,mm1			;[012 4567] [1] mm4: <R3><R2><R1><R0>

	paddw	mm5,mm2			;[012 4567] [1] mm5: <G3><G2><G1><G0>
	psraw	mm4,6			;[012 4567] [1]

	movq	mm3,mm7			;[01234567] [2]
	psraw	mm5,4			;[01234567] [1]

	paddw	mm7,mm0			;[01234567] [2] mm6: <B3><B2><B1><B0>
	psraw	mm6,6			;[01234567] [1]

	paddsw	mm5,[MMX_clip]
	packuswb mm6,mm6		;[01234567] [1] mm6: B3B2B1B0B3B2B1B0

	psubusw	mm5,[MMX_clip]
	packuswb mm4,mm4		;[01234567] [1] mm4: R3R2R1R0R3R2R1R0

	pand	mm5,[MMX_grnmask]		;[01234567] [1] mm7: <G3><G2><G1><G0>
	psrlq	mm6,2			;[01234567] [1]

	punpcklbw mm6,mm4		;[0123 567] [1] mm4: R3B3R2B2R1B1R0B0

	movq	mm4,[edx+ebp*2]		;[01234567] [3] Y
	psrlw	mm6,1			;[01234567] [1]

	pand	mm6,[MMX_rbmask]		;[01234567] [1] mm6: <RB3><RB2><RB1><RB0>

	por	mm6,mm5			;[01234 67] [1] mm6: P6P4P2P0
	movq	mm5,mm3			;[01234567] [2]

	paddw	mm3,mm1			;[01234567] [2] mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;[01234567] [2] mm5: <G3><G2><G1><G0>

	pand	mm4,[MMX_00FFw]		;[01234567] [3]
	psraw	mm3,6			;[01234567] [2]	

	psubw	mm4,[MMX_10w]		;[01234567] [3]
	psraw	mm5,4			;[01234567] [2]

	pmullw	mm4,[MMX_Ycoeff]		;[01234567] [3]
	psraw	mm7,6			;[01234567] [2]

	paddsw	mm5,[MMX_clip]
	packuswb mm3,mm3		;[01234567] [2] mm4: R3R2R1R0R3R2R1R0

	psubusw	mm5,[MMX_clip]
	packuswb mm7,mm7		;[01234567] [2] mm6: B3B2B1B0B3B2B1B0

	pand	mm5,[MMX_grnmask]		;[012 4567] [2] mm7: <G3><G2><G1><G0>
	psrlq	mm7,2			;[01234567] [2]

	punpcklbw mm7,mm3		;[012 4567] [2] mm6: R3B3R2B2R1B1R0B0

	movq	mm3,[edx+ebp*2]		;[01234567] [4] Y
	psrlw	mm7,1			;[01234567] [2]

	pand	mm7,[MMX_rbmask]		;[01234567] [2] mm6: <RB3><RB2><RB1><RB0>
	psrlw	mm3,8			;[01234567] [4]

	por	mm7,mm5			;[01234567] [2] mm7: P7P5P3P1
	movq	mm5,mm6			;[01234567] [A]

	psubw	mm3,[MMX_10w]		;[01234567] [4]
	punpcklwd mm6,mm7		;[01234567] [A] mm4: P3P2P1P0

	pmullw	mm3,[MMX_Ycoeff]		;[0123456 ] [4]
	punpckhwd mm5,mm7		;[0123456 ] [A} mm5: P7P6P5P4

	movntq	[eax+ebp*4   ],mm6	;[012345  ] [A]
	movq	mm6,mm4			;[0123456 ] [3]

	movntq	[eax+ebp*4+ 8],mm5	;[0123456 ] [A]
	paddw	mm6,mm0			;[01234 6 ] [3] mm6: <B3><B2><B1><B0>

	movq	mm5,mm4			;[0123456 ] [3]
	paddw	mm4,mm1			;[0123456 ] [3] mm4: <R3><R2><R1><R0>

	paddw	mm5,mm2			;[0123456 ] [3] mm5: <G3><G2><G1><G0>
	psraw	mm4,6			;[0123456 ] [3]

	movq	mm7,mm3			;[01234567] [4]
	psraw	mm5,4			;[01234567] [3]

	paddw	mm7,mm0			;[01234567] [4] mm6: <B3><B2><B1><B0>
	psraw	mm6,6			;[01234567] [3]

	movq	mm0,mm3			;[01234567] [4]
	packuswb mm4,mm4		;[01234567] [3] mm4: R3R2R1R0R3R2R1R0


	packuswb mm6,mm6		;[01 34567] [3] mm6: B3B2B1B0B3B2B1B0
	paddw	mm3,mm1			;[01234567] [4] mm4: <R3><R2><R1><R0>

	psrlq	mm6,2
	paddw	mm0,mm2			;[01 34567] [4] mm5: <G3><G2><G1><G0>

	paddsw	mm5,[MMX_clip]
	punpcklbw mm6,mm4		;[01 3 567] [3] mm6: B3B3B2B2B1B1B0B0

	psubusw	mm5,[MMX_clip]
	psrlw	mm6,1			;[01 3 567] [3]

	pand	mm6,[MMX_rbmask]		;[01 3 567] [3] mm6: <B3><B2><B1><B0>
	psraw	mm3,6			;[01 3 567] [4]

	pand	mm5,[MMX_grnmask]		;[01 3 567] [3] mm7: <G3><G2><G1><G0>
	psraw	mm0,4			;[01 3 567] [4]

	por	mm6,mm5			;[01 3  67] [3] mm4: P6P4P2P0	
	psraw	mm7,6			;[01 3  67] [4]

	paddsw	mm0,[MMX_clip]
	packuswb mm3,mm3		;[01 3  67] [4] mm4: R3R2R1R0R3R2R1R0

	psubusw	mm0,[MMX_clip]
	packuswb mm7,mm7		;[01 3  67] mm6: B3B2B1B0B3B2B1B0

	pand	mm0,[MMX_grnmask]		;[01    67] mm7: <G3><G2><G1><G0>
	psrlq	mm7,2

	punpcklbw mm7,mm3		;[01    67] mm6: R3B3R2B2R1B1R0B0
	movq	mm1,mm6

	psrlw	mm7,1
	add	ebp,4

	pand	mm7,[MMX_rbmask]		;[01    67] mm6: <B3><B2><B1><B0>

	por	mm0,mm7			;[01    67] mm0: P7P5P3P1

	punpcklwd mm6,mm0		;[01    6 ] mm4: P3P2P1P0

	punpckhwd mm1,mm0		;[ 1    6 ] mm5: P7P6P5P4
	movntq	[ebx+ebp*4-16],mm6

	movntq	[ebx+ebp*4- 8],mm1
	jnz	col_loop_ISSE16

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

;==========================================================================
;
;	SSE2 (Pentium 4) implementation
;
;==========================================================================

_asm_YUVtoRGB32_row_SSE2:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,3
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_SSE2:
	prefetchnta [esi+ebp+32]
	prefetchnta [edi+ebp+32]
	prefetchnta [ecx+ebp*2+32]
	prefetchnta [edx+ebp*2+32]

	movq		xmm0,qword [esi+ebp];xmm0 = U7|U6|U5|U4|U3|U2|U1|U0
	pxor		xmm7,xmm7

	movq		xmm1,qword [edi+ebp];xmm1 = V7|V6|V5|V4|V3|V2|V1|V0

	punpcklbw	xmm0,xmm7
	punpcklbw	xmm1,xmm7
	
	psubw		xmm0, [SSE2_80w]		;xmm0 = U3|U2|U1|U0
	psubw		xmm1, [SSE2_80w]		;xmm1 = V3|V2|V1|V0
	
	movdqa		xmm2,xmm0
	pmullw		xmm0, [SSE2_Ugrncoeff]
	pmullw		xmm2, [SSE2_Ublucoeff]
	
	movdqa		xmm3,xmm1
	pmullw		xmm1, [SSE2_Vredcoeff]
	pmullw		xmm3, [SSE2_Vgrncoeff]
	
	paddw		xmm0,xmm1		;xmm0 = cG7|cG6|cG5|cG4|cG3|cG2|cG1|cG0
	
	movdqu		xmm3,[ecx+ebp*2]	;xmm4 = YF|YE|YD|YC|YB|YA|Y9|Y8|Y7|Y6|Y5|Y4|Y3|Y2|Y1|Y0
	movq		xmm4,xmm4		;xmm5 = YF|YE|YD|YC|YB|YA|Y9|Y8|Y7|Y6|Y5|Y4|Y3|Y2|Y1|Y0
	pand		xmm3, [SSE2_Ylow]		;xmm4 = YE|YC|YA|Y8|Y6|Y4|Y2|Y0
	psrlw		xmm4,8			;xmm5 = YF|YD|YB|Y9|Y7|Y5|Y3|Y1
	
	psubw		xmm3, [SSE2_Ybias]
	pmullw		xmm3, [SSE2_Ycoeff]
	psubw		xmm4, [SSE2_Ybias]
	pmullw		xmm4, [SSE2_Ycoeff]
	
	;register layout at this point:
	;xmm0:	chroma green
	;xmm1:	chroma red
	;xmm2:	chroma blue
	;xmm3:	Y low
	;xmm4:	Y high

	movdqa		xmm5,xmm4
	movdqa		xmm6,xmm4
	paddw		xmm4,xmm0		;xmm4 = green high
	paddw		xmm5,xmm1		;xmm5 = red high
	paddw		xmm6,xmm2		;xmm6 = blue high
	paddw		xmm0,xmm3		;xmm0 = green low
	paddw		xmm1,xmm3		;xmm1 = red low
	paddw		xmm2,xmm3		;xmm2 = blue low
	
	psraw		xmm0,6
	psraw		xmm1,6
	psraw		xmm2,6
	psraw		xmm4,6
	psraw		xmm5,6
	psraw		xmm6,6
	
	packuswb	xmm0,xmm0
	packuswb	xmm1,xmm1
	packuswb	xmm2,xmm2
	packuswb	xmm4,xmm4
	packuswb	xmm5,xmm5
	packuswb	xmm6,xmm6
		
	punpcklbw	xmm0,xmm0		;xmm3 = GE|GE|GC|GC|GA|GA|G8|G8|G6|G6|G4|G4|G2|G2|G0|G0
	punpcklbw	xmm4,xmm4		;xmm4 = GF|GF|GD|GD|GB|GB|G9|G9|G7|G7|G5|G5|G3|G3|G1|G1
	punpcklbw	xmm2,xmm1		;xmm2 = RE|BE|RC|BC|RA|BA|R8|B8|R6|B6|R4|B4|R2|B2|R0|B0
	punpcklbw	xmm6,xmm5		;xmm6 = RF|BF|RD|BD|RB|BB|R9|B9|R7|B7|R5|B5|R3|B3|B1|B1

	movdqa		xmm1,xmm2
	movdqa		xmm5,xmm6
	
	punpcklbw	xmm1,xmm0		;xmm1 = p6|p4|p2|p0
	punpckhbw	xmm2,xmm0		;xmm2 = pE|pC|pA|p8
	punpcklbw	xmm5,xmm4		;xmm5 = p7|p5|p3|p1
	punpckhbw	xmm6,xmm4		;xmm6 = pF|pD|pB|p9
	
	movdqa		xmm0,xmm1
	punpckldq	xmm0,xmm5		;xmm0 = p3|p2|p1|p0
	punpckhdq	xmm1,xmm5		;xmm1 = p7|p6|p5|p4
	movdqa		xmm3,xmm2
	punpckldq	xmm2,xmm6		;xmm2 = pB|pA|p9|p8
	punpckhdq	xmm3,xmm6		;xmm3 = pF|pE|pD|pC

	movdqu	[ebx+ebp*8   ],xmm0
	movdqu	[ebx+ebp*8+ 8],xmm1

	movdqu	[ebx+ebp*8+16],xmm2
	movdqu	[ebx+ebp*8+24],xmm3

	add	ebp,4

	jnz	col_loop_SSE2

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

_asm_YUVtoRGB24_SSE2:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	add	eax,eax
	mov	esi,U_pointer
	mov	edi,V_pointer
	add	esi,ebp
	add	edi,ebp
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	add	ecx,eax
	add	edx,eax
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer
	neg	ebp
	
	;store esp in the SEH chain and set esp=constant_struct
	push	0
	push	dword [fs:0]
	mov		dword [fs:0],esp
	mov		esp, context_pointer
	
	;---- we have no stack at this point!
	
	mov	[esp+offs_width], ebp
	
row_loop_SSE2_24:
	mov	ebp, [esp+offs_width]

col_loop_SSE2_24:
	prefetchnta	[esi+ebp+128]
	prefetchnta	[edi+ebp+128]
	prefetchnta	[ecx+ebp*2+128]
	prefetchnta	[edx+ebp*2+128]

	;U1|U1|U0|U0|U0|U0|U0|U0
	;U2|U2|U2|U2|U1|U1|U1|U1
	;U3|U3|U3|U3|U3|U3|U2|U2

	movd		xmm0,dword [esi+ebp];xmm0 = U3|U2|U1|U0
	pxor		xmm7,xmm7
	punpcklbw	xmm0,xmm7		;xmm0 = U3|U2|U1|U0
	psubw		xmm0,[esp+offs_SSE2_80w]
	punpcklwd	xmm0,xmm0		;xmm0 = U3|U3|U2|U2|U1|U1|U0|U0
	pshufd		xmm2,xmm0,11111110b	;xmm2 = U3|U3|U3|U3|U3|U3|U2|U2
	pshufd		xmm1,xmm0,10100101b	;xmm1 = U2|U2|U2|U2|U1|U1|U1|U1
	pshufd		xmm0,xmm0,01000000b	;xmm0 = U1|U1|U0|U0|U0|U0|U0|U0
		
	pmullw		xmm0,[esp+offs_SSE2_Ucoeff0]
	pmullw		xmm1,[esp+offs_SSE2_Ucoeff1]
	pmullw		xmm2,[esp+offs_SSE2_Ucoeff2]

	movd		xmm3,dword [edi+ebp];xmm3 = V3|V2|V1|V0
	punpcklbw	xmm3,xmm7		;xmm3 = V3|V2|V1|V0
	psubw		xmm3,[esp+offs_SSE2_80w]
	punpcklwd	xmm3,xmm3
	pshufd		xmm5,xmm3,11111110b	;xmm5 = V7|V6|V7|V6|V7|V6|V5|V4
	pshufd		xmm4,xmm3,10100101b	;xmm4 = V5|V4|V5|V4|V3|V2|V3|V2
	pshufd		xmm3,xmm3,01000000b	;xmm3 = V3|V2|V1|V0|V1|V0|V1|V0
		
	pmullw		xmm3,[esp+offs_SSE2_Vcoeff0]
	pmullw		xmm4,[esp+offs_SSE2_Vcoeff1]
	pmullw		xmm5,[esp+offs_SSE2_Vcoeff2]
	
	paddw		xmm0,xmm3
	paddw		xmm1,xmm4
	paddw		xmm2,xmm5
	
	movq		xmm3,qword [ecx+ebp*2];xmm3 = Y7 | Y6 | Y5 | Y4 | Y3 | Y2 | Y1 | Y0
	punpcklbw	xmm3,xmm7
	psubw		xmm3,[esp+offs_SSE2_Ybias]
	pmullw		xmm3,[esp+offs_SSE2_Ycoeff]
	pshufd		xmm5,xmm3,11111110b	;xmm5 = Y7|Y6|Y7|Y6|Y7|Y6|Y5|Y4
	pshufd		xmm4,xmm3,10100101b	;xmm4 = Y5|Y4|Y5|Y4|Y3|Y2|Y3|Y2
	pshufd		xmm3,xmm3,01000000b	;xmm3 = Y3|Y2|Y1|Y0|Y1|Y0|Y1|Y0
	pshufhw		xmm5,xmm5,11111110b	;xmm5 = Y7|Y7|Y7|Y6|Y7|Y6|Y5|Y4
	pshuflw		xmm5,xmm5,10100101b	;xmm5 = Y7|Y7|Y7|Y6|Y6|Y6|Y5|Y5
	pshufhw		xmm4,xmm4,01000000b	;xmm4 = Y5|Y4|Y4|Y4|Y3|Y2|Y3|Y2
	pshuflw		xmm4,xmm4,11111110b	;xmm4 = Y5|Y4|Y4|Y4|Y3|Y3|Y3|Y2
	pshufhw		xmm3,xmm3,10100101b	;xmm3 = Y2|Y2|Y1|Y1|Y1|Y0|Y1|Y0
	pshuflw		xmm3,xmm3,01000000b	;xmm3 = Y2|Y2|Y1|Y1|Y1|Y0|Y0|Y0
	
	paddw		xmm3,xmm0
	paddw		xmm4,xmm1
	paddw		xmm5,xmm2
	
	psraw		xmm3,6
	psraw		xmm4,6
	psraw		xmm5,6
	
	packuswb	xmm3,xmm3
	packuswb	xmm4,xmm4
	packuswb	xmm5,xmm5

	movdq2q		mm0,xmm3
	movdq2q		mm1,xmm4
	movdq2q		mm2,xmm5
	
	movq		xmm3,qword [edx+ebp*2]	;xmm3 = Y7 | Y6 | Y5 | Y4 | Y3 | Y2 | Y1 | Y0
	punpcklbw	xmm3,xmm7
	psubw		xmm3,[esp+offs_SSE2_Ybias]
	pmullw		xmm3,[esp+offs_SSE2_Ycoeff]
	pshufd		xmm5,xmm3,11111110b	;xmm5 = Y7|Y6|Y7|Y6|Y7|Y6|Y5|Y4
	pshufd		xmm4,xmm3,10100101b	;xmm4 = Y5|Y4|Y5|Y4|Y3|Y2|Y3|Y2
	pshufd		xmm3,xmm3,01000000b	;xmm3 = Y3|Y2|Y1|Y0|Y1|Y0|Y1|Y0
	pshufhw		xmm5,xmm5,11111110b	;xmm5 = Y7|Y7|Y7|Y6|Y7|Y6|Y5|Y4
	pshuflw		xmm5,xmm5,10100101b	;xmm5 = Y7|Y7|Y7|Y6|Y6|Y6|Y5|Y5
	pshufhw		xmm4,xmm4,01000000b	;xmm4 = Y5|Y4|Y4|Y4|Y3|Y2|Y3|Y2
	pshuflw		xmm4,xmm4,11111110b	;xmm4 = Y5|Y4|Y4|Y4|Y3|Y3|Y3|Y2
	pshufhw		xmm3,xmm3,10100101b	;xmm3 = Y2|Y2|Y1|Y1|Y1|Y0|Y1|Y0
	pshuflw		xmm3,xmm3,01000000b	;xmm3 = Y2|Y2|Y1|Y1|Y1|Y0|Y0|Y0
	
	paddw		xmm3,xmm0
	paddw		xmm4,xmm1
	paddw		xmm5,xmm2
	
	psraw		xmm3,6
	psraw		xmm4,6
	psraw		xmm5,6
	
	packuswb	xmm3,xmm3
	packuswb	xmm4,xmm4
	packuswb	xmm5,xmm5

	movdq2q		mm3,xmm3
	movdq2q		mm4,xmm4
	movdq2q		mm5,xmm5
	
	movntq		[eax],mm0
	movntq		[eax+8],mm1
	movntq		[eax+16],mm2
	movntq		[ebx],mm3
	movntq		[ebx+8],mm4
	movntq		[ebx+16],mm5	
	add		eax,24
	add		ebx,24

	;done

	add	ebp,4
	jnz	col_loop_SSE2_24

	mov	ebp, [esp+offs_rgb_pitch]	
	add	eax, ebp
	add	ebx, ebp
	mov	ebp, [esp+offs_y_pitch]
	add	ecx, ebp
	add	edx, ebp
	mov	ebp, [esp+offs_uv_pitch]
	add	esi, ebp
	add	edi, ebp
	
	dec	dword [esp+offs_height]
	jnz	row_loop_SSE2_24

	;restore esp from SEH chain
	mov	esp, dword [fs:0]
	pop	dword [fs:0]
	pop	eax

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

_asm_YUVtoRGB16_row_SSE2:
	push	ebx
	push	esi
	push	edi
	push	ebp

	mov	eax,count
	mov	ebp,eax
	mov	ebx,eax
	shl	ebx,2
	add	eax,eax
	add	ARGB1_pointer,ebx
	add	ARGB2_pointer,ebx
	add	Y1_pointer,eax
	add	Y2_pointer,eax
	add	U_pointer,ebp
	add	V_pointer,ebp
	neg	ebp

	mov	esi,U_pointer
	mov	edi,V_pointer
	mov	ecx,Y1_pointer
	mov	edx,Y2_pointer
	mov	eax,ARGB1_pointer
	mov	ebx,ARGB2_pointer

col_loop_SSE2_16:
	prefetchnta [esi+ebp+32]
	prefetchnta [edi+ebp+32]

	movd	mm0,dword [esi+ebp]	;[0       ] U (byte)
	pxor	mm7,mm7			;[0      7] 

	movd	mm1,dword [edi+ebp]	;[01     7] V (byte)
	punpcklbw mm0,mm7		;[01     7] U (word)

	psubw	mm0,[MMX_80w]		;[01     7] 
	punpcklbw mm1,mm7		;[01     7] V (word)

	psubw	mm1,[MMX_80w]		;[01      ] 
	movq	mm2,mm0			;[012     ] 

	pmullw	mm2,[MMX_Ugrncoeff]	;[012     ] 
	movq	mm3,mm1			;[0123    ] 

	;mm0: blue
	;mm1: red
	;mm2: green

	prefetchnta [ecx+ebp*2+32]
	prefetchnta [edx+ebp*2+32]

	movq	mm6,[ecx+ebp*2]		;[0123  6 ] [1] Y
	;<-->

	pmullw	mm3,[MMX_Vgrncoeff]	;[0123    ] 
	movq	mm7,mm6			;[012   67] [2] Y

	pmullw	mm0,[MMX_Ublucoeff]	;[0123    ] 
	psrlw	mm7,8			;[012   67] [2]

	pmullw	mm1,[MMX_Vredcoeff]	;[0123    ] 
	;<-->

	pand	mm6,[MMX_00FFw]		;[012   67] [1]
	paddw	mm2,mm3			;[012   6 ] [C]

	psubw	mm6,[MMX_10w]		;[012   67] [1]

	pmullw	mm6,[MMX_Ycoeff]		;[012   67] [1]

	psubw	mm7,[MMX_10w]		;[012   67] [2]
	movq	mm4,mm6			;[012 4 67] [1]

	pmullw	mm7,[MMX_Ycoeff]		;[012   67] [2]
	movq	mm5,mm6			;[012 4567] [1]

	paddw	mm6,mm0			;[012 4 67] [1] mm6: <B3><B2><B1><B0>
	paddw	mm4,mm1			;[012 4567] [1] mm4: <R3><R2><R1><R0>

	paddw	mm5,mm2			;[012 4567] [1] mm5: <G3><G2><G1><G0>
	psraw	mm4,6			;[012 4567] [1]

	movq	mm3,mm7			;[01234567] [2]
	psraw	mm5,4			;[01234567] [1]

	paddw	mm7,mm0			;[01234567] [2] mm6: <B3><B2><B1><B0>
	psraw	mm6,6			;[01234567] [1]

	paddsw	mm5,[MMX_clip]
	packuswb mm6,mm6		;[01234567] [1] mm6: B3B2B1B0B3B2B1B0

	psubusw	mm5,[MMX_clip]
	packuswb mm4,mm4		;[01234567] [1] mm4: R3R2R1R0R3R2R1R0

	pand	mm5,[MMX_grnmask]		;[01234567] [1] mm7: <G3><G2><G1><G0>
	psrlq	mm6,2			;[01234567] [1]

	punpcklbw mm6,mm4		;[0123 567] [1] mm4: R3B3R2B2R1B1R0B0

	movq	mm4,[edx+ebp*2]		;[01234567] [3] Y
	psrlw	mm6,1			;[01234567] [1]

	pand	mm6,[MMX_rbmask]		;[01234567] [1] mm6: <RB3><RB2><RB1><RB0>

	por	mm6,mm5			;[01234 67] [1] mm6: P6P4P2P0
	movq	mm5,mm3			;[01234567] [2]

	paddw	mm3,mm1			;[01234567] [2] mm4: <R3><R2><R1><R0>
	paddw	mm5,mm2			;[01234567] [2] mm5: <G3><G2><G1><G0>

	pand	mm4,[MMX_00FFw]		;[01234567] [3]
	psraw	mm3,6			;[01234567] [2]	

	psubw	mm4,[MMX_10w]		;[01234567] [3]
	psraw	mm5,4			;[01234567] [2]

	pmullw	mm4,[MMX_Ycoeff]		;[01234567] [3]
	psraw	mm7,6			;[01234567] [2]

	paddsw	mm5,[MMX_clip]
	packuswb mm3,mm3		;[01234567] [2] mm4: R3R2R1R0R3R2R1R0

	psubusw	mm5,[MMX_clip]
	packuswb mm7,mm7		;[01234567] [2] mm6: B3B2B1B0B3B2B1B0

	pand	mm5,[MMX_grnmask]		;[012 4567] [2] mm7: <G3><G2><G1><G0>
	psrlq	mm7,2			;[01234567] [2]

	punpcklbw mm7,mm3		;[012 4567] [2] mm6: R3B3R2B2R1B1R0B0

	movq	mm3,[edx+ebp*2]		;[01234567] [4] Y
	psrlw	mm7,1			;[01234567] [2]

	pand	mm7,[MMX_rbmask]		;[01234567] [2] mm6: <RB3><RB2><RB1><RB0>
	psrlw	mm3,8			;[01234567] [4]

	por	mm7,mm5			;[01234567] [2] mm7: P7P5P3P1
	movq	mm5,mm6			;[01234567] [A]

	psubw	mm3,[MMX_10w]		;[01234567] [4]
	punpcklwd mm6,mm7		;[01234567] [A] mm4: P3P2P1P0

	pmullw	mm3,[MMX_Ycoeff]		;[0123456 ] [4]
	punpckhwd mm5,mm7		;[0123456 ] [A} mm5: P7P6P5P4

	movntq	[eax+ebp*4   ],mm6	;[012345  ] [A]
	movq	mm6,mm4			;[0123456 ] [3]

	movntq	[eax+ebp*4+ 8],mm5	;[0123456 ] [A]
	paddw	mm6,mm0			;[01234 6 ] [3] mm6: <B3><B2><B1><B0>

	movq	mm5,mm4			;[0123456 ] [3]
	paddw	mm4,mm1			;[0123456 ] [3] mm4: <R3><R2><R1><R0>

	paddw	mm5,mm2			;[0123456 ] [3] mm5: <G3><G2><G1><G0>
	psraw	mm4,6			;[0123456 ] [3]

	movq	mm7,mm3			;[01234567] [4]
	psraw	mm5,4			;[01234567] [3]

	paddw	mm7,mm0			;[01234567] [4] mm6: <B3><B2><B1><B0>
	psraw	mm6,6			;[01234567] [3]

	movq	mm0,mm3			;[01234567] [4]
	packuswb mm4,mm4		;[01234567] [3] mm4: R3R2R1R0R3R2R1R0


	packuswb mm6,mm6		;[01 34567] [3] mm6: B3B2B1B0B3B2B1B0
	paddw	mm3,mm1			;[01234567] [4] mm4: <R3><R2><R1><R0>

	psrlq	mm6,2
	paddw	mm0,mm2			;[01 34567] [4] mm5: <G3><G2><G1><G0>

	paddsw	mm5,[MMX_clip]
	punpcklbw mm6,mm4		;[01 3 567] [3] mm6: B3B3B2B2B1B1B0B0

	psubusw	mm5,[MMX_clip]
	psrlw	mm6,1			;[01 3 567] [3]

	pand	mm6,[MMX_rbmask]		;[01 3 567] [3] mm6: <B3><B2><B1><B0>
	psraw	mm3,6			;[01 3 567] [4]

	pand	mm5,[MMX_grnmask]		;[01 3 567] [3] mm7: <G3><G2><G1><G0>
	psraw	mm0,4			;[01 3 567] [4]

	por	mm6,mm5			;[01 3  67] [3] mm4: P6P4P2P0	
	psraw	mm7,6			;[01 3  67] [4]

	paddsw	mm0,[MMX_clip]
	packuswb mm3,mm3		;[01 3  67] [4] mm4: R3R2R1R0R3R2R1R0

	psubusw	mm0,[MMX_clip]
	packuswb mm7,mm7		;[01 3  67] mm6: B3B2B1B0B3B2B1B0

	pand	mm0,[MMX_grnmask]		;[01    67] mm7: <G3><G2><G1><G0>
	psrlq	mm7,2

	punpcklbw mm7,mm3		;[01    67] mm6: R3B3R2B2R1B1R0B0
	movq	mm1,mm6

	psrlw	mm7,1
	add	ebp,4

	pand	mm7,[MMX_rbmask]		;[01    67] mm6: <B3><B2><B1><B0>

	por	mm0,mm7			;[01    67] mm0: P7P5P3P1

	punpcklwd mm6,mm0		;[01    6 ] mm4: P3P2P1P0

	punpckhwd mm1,mm0		;[ 1    6 ] mm5: P7P6P5P4
	movntq	[ebx+ebp*4-16],mm6

	movntq	[ebx+ebp*4- 8],mm1
	jnz	col_loop_SSE2_16

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

	end
