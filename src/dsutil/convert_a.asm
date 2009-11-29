; Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
; http://www.avisynth.org
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
; http://www.gnu.org/copyleft/gpl.html .
;
; Linking Avisynth statically or dynamically with other modules is making a
; combined work based on Avisynth.  Thus, the terms and conditions of the GNU
; General Public License cover the whole combination.
;
; As a special exception, the copyright holders of Avisynth give you
; permission to link Avisynth with independent modules that communicate with
; Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
; terms of these independent modules, and to copy and distribute the
; resulting combined work under terms of your choice, provided that
; every copy of the combined work is accompanied by a complete copy of
; the source code of Avisynth (the version of Avisynth used to produce the
; combined work), being distributed under the terms of the GNU General
; Public License plus this exception.  An independent module is a module
; which is not derived from or based on Avisynth, such as 3rd-party filters,
; import and export plugins, or graphical user interfaces.

	.586
	.mmx
	.model	flat

; alignment has to be 'page' so that I can use 'align 32' below

_TEXT64	segment	page public use32 'CODE'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	align	8

yuv2rgb_constants_rec601:

x0000_0000_0010_0010	dq	00000000000100010h		;    16
x0080_0080_0080_0080	dq	00080008000800080h		;   128
x00FF_00FF_00FF_00FF	dq	000FF00FF00FF00FFh
x00002000_00002000		dq	00000200000002000h		;  8192        = (0.5)<<14
xFF000000_FF000000		dq	0FF000000FF000000h
cy						dq	000004A8500004A85h		; 19077        = (255./219.)<<14+0.5
crv						dq	03313000033130000h		; 13075        = ((1-0.299)*255./112.)<<13+0.5
cgu_cgv					dq	0E5FCF377E5FCF377h		; -6660, -3209 = ((K-1)*K/0.587*255./112.)<<13-0.5, K=(0.299, 0.114)
cbu						dq	00000408D0000408Dh		;        16525 = ((1-0.114)*255./112.)<<13+0.5

yuv2rgb_constants_PC_601:

						dq	00000000000000000h		;     0       
						dq	00080008000800080h		;   128       
						dq	000FF00FF00FF00FFh                    
						dq	00000200000002000h		;  8192        = (0.5)<<14
						dq	0FF000000FF000000h                    
						dq	00000400000004000h		; 16384        = (1.)<<14+0.5                                
						dq	02D0B00002D0B0000h		; 11531        = ((1-0.299)*255./127.)<<13+0.5                      
						dq	0E90FF4F2E90FF4F2h		; -5873, -2830 = (((K-1)*K/0.587)*255./127.)<<13-0.5, K=(0.299, 0.114)
						dq	0000038ED000038EDh		;        14573 = ((1-0.114)*255./127.)<<13+0.5                      

yuv2rgb_constants_rec709:

						dq	00000000000100010h		;    16       
						dq	00080008000800080h		;   128       
						dq	000FF00FF00FF00FFh                    
						dq	00000200000002000h		;  8192        = (0.5)<<14
						dq	0FF000000FF000000h                    
						dq	000004A8500004A85h		; 19077        = (255./219.)<<14+0.5
						dq	0395E0000395E0000h		; 14686        = ((1-0.2126)*255./112.)<<13+0.5
						dq	0EEF2F92DEEF2F92Dh		; -4366, -1747 = ((K-1)*K/0.7152*255./112.)<<13-0.5, K=(0.2126, 0.0722)
						dq	00000439900004399h		;        17305        = ((1-0.0722)*255./112.)<<13+0.5       

yuv2rgb_constants_PC_709:

						dq	00000000000000000h		;     0       
						dq	00080008000800080h		;   128       
						dq	000FF00FF00FF00FFh                    
						dq	00000200000002000h		;  8192        = (0.5)<<14
						dq	0FF000000FF000000h                    
						dq	00000400000004000h		; 16384        = (1.)<<14+0.5                                
						dq	03298000032980000h		; 12952        = ((1-0.2126)*255./127.)<<13+0.5                      
						dq	0F0F6F9FBF0F6F9FBh		; -3850, -1541 = (((K-1)*K/0.7152)*255./127.)<<13-0.5, K=(0.2126, 0.0722)
						dq	000003B9D00003B9Dh		;        15261 = ((1-0.0722)*255./127.)<<13+0.5                      

ofs_x0000_0000_0010_0010 = 0
ofs_x0080_0080_0080_0080 = 8
ofs_x00FF_00FF_00FF_00FF = 16
ofs_x00002000_00002000 = 24
ofs_xFF000000_FF000000 = 32
ofs_cy = 40
ofs_crv = 48
ofs_cgu_cgv = 56
ofs_cbu = 64

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

GET_Y	MACRO	mma,uyvy
IF &uyvy
	psrlw		mma,8
ELSE
	pand		mma,[edx+ofs_x00FF_00FF_00FF_00FF]
ENDIF
	ENDM

GET_UV	MACRO	mma,uyvy
	GET_Y		mma,1-uyvy
	ENDM

YUV2RGB_INNER_LOOP	MACRO	uyvy,rgb32,no_next_pixel

;; This YUV422->RGB conversion code uses only four MMX registers per
;; source dword, so I convert two dwords in parallel.  Lines corresponding
;; to the "second pipe" are indented an extra space.  There's almost no
;; overlap, except at the end and in the three lines marked ***.
;; revised 4july,2002 to properly set alpha in rgb32 to default "on" & other small memory optimizations

	movd		mm0, DWORD PTR [esi] ; DWORD PTR for compatibility woth masm8
	 movd		 mm5, DWORD PTR [esi+4]
	movq		mm1,mm0
	GET_Y		mm0,&uyvy	; mm0 = __________Y1__Y0
	 movq		 mm4,mm5
	GET_UV		mm1,&uyvy	; mm1 = __________V0__U0
	 GET_Y		 mm4,&uyvy	; mm4 = __________Y3__Y2
	movq		mm2,mm5		; *** avoid reload from [esi+4]
	 GET_UV		 mm5,&uyvy	; mm5 = __________V2__U2
	psubw		mm0,[edx+ofs_x0000_0000_0010_0010]	; (Y-16)
	 movd		 mm6, DWORD PTR [esi+8-4*(no_next_pixel)]
	GET_UV		mm2,&uyvy	; mm2 = __________V2__U2
	 psubw		 mm4,[edx+ofs_x0000_0000_0010_0010]	; (Y-16)
	paddw		mm2,mm1		; 2*UV1=UV0+UV2
	 GET_UV		 mm6,&uyvy	; mm6 = __________V4__U4
	psubw		mm1,[edx+ofs_x0080_0080_0080_0080]	; (UV-128)
	 paddw		 mm6,mm5	; 2*UV3=UV2+UV4
	psllq		mm2,32
	 psubw		 mm5,[edx+ofs_x0080_0080_0080_0080]	; (UV-128)
	punpcklwd	mm0,mm2		; mm0 = ______Y1______Y0
	 psllq		 mm6,32
	pmaddwd		mm0,[edx+ofs_cy]	; (Y-16)*(255./219.)<<14
	 punpcklwd	 mm4,mm6
	paddw		mm1,mm1		; 2*UV0=UV0+UV0
	 pmaddwd	 mm4,[edx+ofs_cy]
	 paddw		 mm5,mm5	; 2*UV2=UV2+UV2
	paddw		mm1,mm2		; mm1 = __V1__U1__V0__U0 * 2
	paddd		mm0,[edx+ofs_x00002000_00002000]	; +=0.5<<14
	 paddw		 mm5,mm6	; mm5 = __V3__U3__V2__U2 * 2
	movq		mm2,mm1
	 paddd		 mm4,[edx+ofs_x00002000_00002000]	; +=0.5<<14
	movq		mm3,mm1
	 movq		 mm6,mm5
	pmaddwd		mm1,[edx+ofs_crv]
	 movq		 mm7,mm5
	paddd		mm1,mm0
	 pmaddwd	 mm5,[edx+ofs_crv]
	psrad		mm1,14		; mm1 = RRRRRRRRrrrrrrrr
	 paddd		 mm5,mm4
	pmaddwd		mm2,[edx+ofs_cgu_cgv]
	 psrad		 mm5,14
	paddd		mm2,mm0
	 pmaddwd	 mm6,[edx+ofs_cgu_cgv]
	psrad		mm2,14		; mm2 = GGGGGGGGgggggggg
	 paddd		 mm6,mm4
	pmaddwd		mm3,[edx+ofs_cbu]
	 psrad		 mm6,14
	paddd		mm3,mm0
	 pmaddwd	 mm7,[edx+ofs_cbu]
       add	       esi,8
       add	       edi,12+4*rgb32
IFE &no_next_pixel
       cmp	       esi,ecx
ENDIF
	psrad		mm3,14		; mm3 = BBBBBBBBbbbbbbbb
	 paddd		 mm7,mm4
	pxor		mm0,mm0
	 psrad		 mm7,14
	packssdw	mm3,mm2	; mm3 = GGGGggggBBBBbbbb
	 packssdw	 mm7,mm6
	packssdw	mm1,mm0	; mm1 = ________RRRRrrrr
	 packssdw	 mm5,mm0	; *** avoid pxor mm4,mm4
	movq		mm2,mm3
	 movq		 mm6,mm7
	punpcklwd	mm2,mm1	; mm2 = RRRRBBBBrrrrbbbb
	 punpcklwd	 mm6,mm5
	punpckhwd	mm3,mm1	; mm3 = ____GGGG____gggg
	 punpckhwd	 mm7,mm5
	movq		mm0,mm2
	 movq		 mm4,mm6
	punpcklwd	mm0,mm3	; mm0 = ____rrrrggggbbbb
	 punpcklwd	 mm4,mm7
IFE &rgb32
	psllq		mm0,16
	 psllq		 mm4,16
ENDIF
	punpckhwd	mm2,mm3	; mm2 = ____RRRRGGGGBBBB
	 punpckhwd	 mm6,mm7
	packuswb	mm0,mm2	; mm0 = __RRGGBB__rrggbb <- ta dah!
	 packuswb	 mm4,mm6

IF &rgb32
	por mm0, [edx+ofs_xFF000000_FF000000]	 ; set alpha channels "on"
	 por mm4, [edx+ofs_xFF000000_FF000000]
	movq	[edi-16],mm0	; store the quadwords independently
	 movq	 [edi-8],mm4
ELSE
	psrlq	mm0,8		; pack the two quadwords into 12 bytes
	psllq	mm4,8		; (note: the two shifts above leave
	movd	DWORD PTR [edi-12],mm0	; mm0,4 = __RRGGBBrrggbb__)
	psrlq	mm0,32
	por	mm4,mm0
	movd	DWORD PTR [edi-8],mm4
	psrlq	mm4,32
	movd	DWORD PTR [edi-4],mm4
ENDIF

	ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

YUV2RGB_PROC	MACRO	procname,uyvy,rgb32

	PUBLIC	C _&procname

;;void __cdecl procname(
;;	[esp+ 4] const BYTE* src,
;;	[esp+ 8] BYTE* dst,
;;	[esp+12] const BYTE* src_end,
;;	[esp+16] int src_pitch,
;;	[esp+20] int row_size,
;;	[esp+24] rec709 matrix);  0=rec601, 1=rec709, 3=PC_601, 7=PC_709

_&procname	PROC

	push	esi
	push	edi
	push	ebx

	mov	eax,[esp+16+12]		; src_pitch
	mov	esi,[esp+12+12]		; src_end - read source bottom-up
	mov	edi,[esp+8+12]		; dstp
	mov	ebx,[esp+20+12]		; row_size
	mov	edx,offset yuv2rgb_constants_rec601
	test	byte ptr [esp+24+12],1
	jz	loop0
	mov	edx,offset yuv2rgb_constants_rec709

	test	byte ptr [esp+24+12],2
	jz	loop0
	mov	edx,offset yuv2rgb_constants_PC_601

	test	byte ptr [esp+24+12],4
	jz	loop0
	mov	edx,offset yuv2rgb_constants_PC_709

loop0:
	sub	esi,eax
	lea	ecx,[esi+ebx-8]

	align 32
loop1:
	YUV2RGB_INNER_LOOP	uyvy,rgb32,0
	jb	loop1

	YUV2RGB_INNER_LOOP	uyvy,rgb32,1

	sub	esi,ebx
	cmp	esi,[esp+4+12]		; src
	ja	loop0

	emms
	pop	ebx
	pop	edi
	pop	esi
	retn

_&procname	ENDP

	ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

YUV2RGB_PROC	mmx_YUY2toRGB24,0,0
YUV2RGB_PROC	mmx_YUY2toRGB32,0,1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	END

