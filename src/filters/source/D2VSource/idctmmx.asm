;
; idct8x8_xmm.asm
;
; Originally provided by Intel at AP-922
; http://developer.intel.com/vtune/cbts/strmsimd/922down.htm
; (See more app notes at http://developer.intel.com/vtune/cbts/strmsimd/appnotes.htm)
; but in a limited edition.
; New macro implements a column part for precise iDCT
; The routine precision now satisfies IEEE standard 1180-1990. 
;
; Copyright (c) 2000-2001 Peter Gubanov <peter@elecard.net.ru>
; Rounding trick Copyright (c) 2000 Michel Lespinasse <walken@zoy.org>
;
; http://www.elecard.com/peter/idct.html
; http://www.linuxvideo.org/mpeg2dec/
;
;=============================================================================
;
; These examples contain code fragments for first stage iDCT 8x8
; (for rows) and first stage DCT 8x8 (for columns)
;
;=============================================================================
mword 	typedef qword
mptr 	equ mword ptr

BITS_INV_ACC = 5 			; 4 or 5 for IEEE
SHIFT_INV_ROW = 16 - BITS_INV_ACC
SHIFT_INV_COL = 1 + BITS_INV_ACC
RND_INV_ROW = 1024 * (6 - BITS_INV_ACC) ; 1 << (SHIFT_INV_ROW-1)
RND_INV_COL = 16 * (BITS_INV_ACC - 3) 	; 1 << (SHIFT_INV_COL-1)
RND_INV_CORR = RND_INV_COL - 1 		; correction -1.0 and round

BITS_FRW_ACC = 3 			; 2 or 3 for accuracy
SHIFT_FRW_COL = BITS_FRW_ACC
SHIFT_FRW_ROW = BITS_FRW_ACC + 17
RND_FRW_ROW = 262144 * (BITS_FRW_ACC - 1)	; 1 << (SHIFT_FRW_ROW-1)

_MMX = 1

.nolist

.586

if @version GE 612
.mmx
;mmword	TEXTEQU	<QWORD>
else
include IAMMX.INC
endif

if @version GE 614
.xmm
;mm2word	TEXTEQU	<QWORD>			; needed for Streaming SIMD Extensions macros
else
include iaxmm.inc				; Streaming SIMD Extensions Emulator Macros
endif

	.list
	.model flat

_DATA SEGMENT PARA PUBLIC USE32 'DATA'

one_corr 	sword            1,            1,            1,            1
round_inv_row	dword  RND_INV_ROW,  RND_INV_ROW
round_inv_col	sword  RND_INV_COL,  RND_INV_COL,  RND_INV_COL, RND_INV_COL
round_inv_corr	sword RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR
round_frw_row	dword  RND_FRW_ROW,  RND_FRW_ROW
  tg_1_16	sword  13036,  13036,  13036,  13036	; tg * (2<<16) + 0.5
  tg_2_16	sword  27146,  27146,  27146,  27146 	; tg * (2<<16) + 0.5
  tg_3_16	sword -21746, -21746, -21746, -21746 	; tg * (2<<16) + 0.5
 cos_4_16	sword -19195, -19195, -19195, -19195 	; cos * (2<<16) + 0.5
ocos_4_16	sword  23170,  23170,  23170,  23170 	; cos * (2<<15) + 0.5

 otg_3_16	sword  21895, 21895, 21895, 21895 	; tg * (2<<16) + 0.5

; assume SHIFT_INV_ROW == 12
;rounder_0	dword  65536, 65536
;rounder_4       dword      0,     0
;rounder_1       dword   7195,  7195
;rounder_7	dword   1024,  1024
;rounder_2	dword   4520,  4520
;rounder_6	dword   1024,  1024
;rounder_3	dword   2407,  2407
;rounder_5	dword    240,   240

; assume SHIFT_INV_ROW == 11
rounder_0	dword  65536, 65536
rounder_4       dword      0,     0
rounder_1       dword   3597,  3597
rounder_7	dword    512,   512
rounder_2	dword   2260,  2260
rounder_6	dword    512,   512
rounder_3	dword   1203,  1203
rounder_5	dword    120,   120

;=============================================================================
;
; The first stage iDCT 8x8 - inverse DCTs of rows
;
;-----------------------------------------------------------------------------
; The 8-point inverse DCT direct algorithm
;-----------------------------------------------------------------------------
;
; static const short w[32] = {
;	FIX(cos_4_16),  FIX(cos_2_16),  FIX(cos_4_16),  FIX(cos_6_16),
;	FIX(cos_4_16),  FIX(cos_6_16), -FIX(cos_4_16), -FIX(cos_2_16),
;	FIX(cos_4_16), -FIX(cos_6_16), -FIX(cos_4_16),  FIX(cos_2_16),
;	FIX(cos_4_16), -FIX(cos_2_16),  FIX(cos_4_16), -FIX(cos_6_16),
;	FIX(cos_1_16),  FIX(cos_3_16),  FIX(cos_5_16),  FIX(cos_7_16),
;	FIX(cos_3_16), -FIX(cos_7_16), -FIX(cos_1_16), -FIX(cos_5_16),
;	FIX(cos_5_16), -FIX(cos_1_16),  FIX(cos_7_16),  FIX(cos_3_16),
;	FIX(cos_7_16), -FIX(cos_5_16),  FIX(cos_3_16), -FIX(cos_1_16) };
;
; #define DCT_8_INV_ROW(x, y)
; {
; 	int a0, a1, a2, a3, b0, b1, b2, b3;
;
; 	a0 =x[0]*w[0]+x[2]*w[1]+x[4]*w[2]+x[6]*w[3];
; 	a1 =x[0]*w[4]+x[2]*w[5]+x[4]*w[6]+x[6]*w[7];
; 	a2 = x[0] * w[ 8] + x[2] * w[ 9] + x[4] * w[10] + x[6] * w[11];
; 	a3 = x[0] * w[12] + x[2] * w[13] + x[4] * w[14] + x[6] * w[15];
; 	b0 = x[1] * w[16] + x[3] * w[17] + x[5] * w[18] + x[7] * w[19];
; 	b1 = x[1] * w[20] + x[3] * w[21] + x[5] * w[22] + x[7] * w[23];
; 	b2 = x[1] * w[24] + x[3] * w[25] + x[5] * w[26] + x[7] * w[27];
; 	b3 = x[1] * w[28] + x[3] * w[29] + x[5] * w[30] + x[7] * w[31];
;
; 	y[0] = SHIFT_ROUND ( a0 + b0 );
; 	y[1] = SHIFT_ROUND ( a1 + b1 );
; 	y[2] = SHIFT_ROUND ( a2 + b2 );
; 	y[3] = SHIFT_ROUND ( a3 + b3 );
; 	y[4] = SHIFT_ROUND ( a3 - b3 );
; 	y[5] = SHIFT_ROUND ( a2 - b2 );
; 	y[6] = SHIFT_ROUND ( a1 - b1 );
; 	y[7] = SHIFT_ROUND ( a0 - b0 );
; }
;
;-----------------------------------------------------------------------------
;
; In this implementation the outputs of the iDCT-1D are multiplied
; 	for rows 0,4 - by cos_4_16,
; 	for rows 1,7 - by cos_1_16,
; 	for rows 2,6 - by cos_2_16,
; 	for rows 3,5 - by cos_3_16
; and are shifted to the left for better accuracy
;
; For the constants used,
; 	FIX(float_const) = (short) (float_const * (1<<15) + 0.5)
;
;=============================================================================

;=============================================================================
; MMX code
;=============================================================================

; Table for rows 0,4 - constants are multiplied by cos_4_16

tab_i_04 	sword  16384,  16384,  16384, -16384	; movq-> w06 w04 w02 w00
		sword  21407,   8867,   8867, -21407	; w07 w05 w03 w01
		sword  16384, -16384,  16384,  16384	; w14 w12 w10 w08
		sword  -8867,  21407, -21407,  -8867	; w15 w13 w11 w09
		sword  22725,  12873,  19266, -22725	; w22 w20 w18 w16
		sword  19266,   4520,  -4520, -12873	; w23 w21 w19 w17
		sword  12873,   4520,   4520,  19266	; w30 w28 w26 w24
		sword -22725,  19266, -12873, -22725	; w31 w29 w27 w25

; Table for rows 1,7 - constants are multiplied by cos_1_16

tab_i_17	sword  22725,  22725,  22725, -22725	; movq-> w06 w04 w02 w00
		sword  29692,  12299,  12299, -29692	; w07 w05 w03 w01
		sword  22725, -22725,  22725,  22725	; w14 w12 w10 w08
		sword -12299,  29692, -29692, -12299	; w15 w13 w11 w09
		sword  31521,  17855,  26722, -31521	; w22 w20 w18 w16
		sword  26722,   6270,  -6270, -17855	; w23 w21 w19 w17
		sword  17855,   6270,   6270,  26722	; w30 w28 w26 w24
		sword -31521,  26722, -17855, -31521	; w31 w29 w27 w25

; Table for rows 2,6 - constants are multiplied by cos_2_16

tab_i_26	sword  21407,  21407,  21407, -21407	; movq-> w06 w04 w02 w00
		sword  27969,  11585,  11585, -27969	; w07 w05 w03 w01
		sword  21407, -21407,  21407,  21407	; w14 w12 w10 w08
		sword -11585,  27969, -27969, -11585	; w15 w13 w11 w09
		sword  29692,  16819,  25172, -29692	; w22 w20 w18 w16
		sword  25172,   5906,  -5906, -16819	; w23 w21 w19 w17
		sword  16819,   5906,   5906,  25172	; w30 w28 w26 w24
		sword -29692,  25172, -16819, -29692	; w31 w29 w27 w25

; Table for rows 3,5 - constants are multiplied by cos_3_16

tab_i_35	sword  19266,  19266,  19266, -19266	; movq-> w06 w04 w02 w00
		sword  25172,  10426,  10426, -25172	; w07 w05 w03 w01
		sword  19266, -19266,  19266,  19266	; w14 w12 w10 w08
		sword -10426,  25172, -25172, -10426	; w15 w13 w11 w09
		sword  26722,  15137,  22654, -26722	; w22 w20 w18 w16
		sword  22654,   5315,  -5315, -15137	; w23 w21 w19 w17
		sword  15137,   5315,   5315,  22654	; w30 w28 w26 w24
		sword -26722,  22654, -15137, -26722	; w31 w29 w27 w25

;-----------------------------------------------------------------------------

DCT_8_INV_ROW_1 MACRO INP:REQ, OUT:REQ, TABLE:REQ, ROUNDER:REQ

	movq mm0, mptr [INP] 		; 0	; x3 x2 x1 x0

	movq mm1, mptr [INP+8]		; 1	; x7 x6 x5 x4
	movq mm2, mm0 			; 2	; x3 x2 x1 x0

	movq mm3, mptr [TABLE]		; 3	; w06 w04 w02 w00
	punpcklwd mm0, mm1 			; x5 x1 x4 x0

	movq mm5, mm0 			; 5	; x5 x1 x4 x0
	punpckldq mm0, mm0 			; x4 x0 x4 x0

	movq mm4, mptr [TABLE+8] 	; 4	; w07 w05 w03 w01
	punpckhwd mm2, mm1		; 1	; x7 x3 x6 x2

	pmaddwd mm3, mm0 			; x4*w06+x0*w04 x4*w02+x0*w00
	movq mm6, mm2 			; 6 	; x7 x3 x6 x2

	movq mm1, mptr [TABLE+32] 	; 1 	; w22 w20 w18 w16
	punpckldq mm2, mm2 			; x6 x2 x6 x2

	pmaddwd mm4, mm2 			; x6*w07+x2*w05 x6*w03+x2*w01
	punpckhdq mm5, mm5 			; x5 x1 x5 x1

	pmaddwd mm0, mptr [TABLE+16] 		; x4*w14+x0*w12 x4*w10+x0*w08
	punpckhdq mm6, mm6 			; x7 x3 x7 x3

	movq mm7, mptr [TABLE+40] 	; 7 	; w23 w21 w19 w17
	pmaddwd mm1, mm5 			; x5*w22+x1*w20 x5*w18+x1*w16

	paddd mm3, mptr [ROUNDER] 		; +rounder
	pmaddwd mm7, mm6 			; x7*w23+x3*w21 x7*w19+x3*w17

	pmaddwd mm2, mptr [TABLE+24] 		; x6*w15+x2*w13 x6*w11+x2*w09
	paddd mm3, mm4 			; 4 	; a1=sum(even1) a0=sum(even0)

	pmaddwd mm5, mptr [TABLE+48] 		; x5*w30+x1*w28 x5*w26+x1*w24
	movq mm4, mm3 			; 4 	; a1 a0

	pmaddwd mm6, mptr [TABLE+56] 		; x7*w31+x3*w29 x7*w27+x3*w25
	paddd mm1, mm7 			; 7 	; b1=sum(odd1) b0=sum(odd0)

	paddd mm0, mptr [ROUNDER]		; +rounder
	psubd mm3, mm1 				; a1-b1 a0-b0

	psrad mm3, SHIFT_INV_ROW 		; y6=a1-b1 y7=a0-b0
	paddd mm1, mm4 			; 4 	; a1+b1 a0+b0

	paddd mm0, mm2 			; 2 	; a3=sum(even3) a2=sum(even2)
	psrad mm1, SHIFT_INV_ROW 		; y1=a1+b1 y0=a0+b0

	paddd mm5, mm6 			; 6 	; b3=sum(odd3) b2=sum(odd2)
	movq mm4, mm0 			; 4 	; a3 a2

	paddd mm0, mm5 				; a3+b3 a2+b2
	psubd mm4, mm5 			; 5 	; a3-b3 a2-b2

	psrad mm0, SHIFT_INV_ROW 		; y3=a3+b3 y2=a2+b2
	psrad mm4, SHIFT_INV_ROW 		; y4=a3-b3 y5=a2-b2

	packssdw mm1, mm0 		; 0 	; y3 y2 y1 y0
	packssdw mm4, mm3 		; 3 	; y6 y7 y4 y5

	movq mm7, mm4 			; 7 	; y6 y7 y4 y5
	psrld mm4, 16 				; 0 y6 0 y4

	pslld mm7, 16 				; y7 0 y5 0
	movq mptr [OUT], mm1 		; 1 	; save y3 y2 y1 y0
                             	
	por mm7, mm4 			; 4 	; y7 y6 y5 y4
	movq mptr [OUT+8], mm7 		; 7 	; save y7 y6 y5 y4
ENDM

;=============================================================================
; code for Pentium III
;=============================================================================

; Table for rows 0,4 - constants are multiplied by cos_4_16

tab_i_04_s 	sword 16384, 21407, 16384, 8867 ; movq-> w05 w04 w01 w00
		sword 16384, 8867, -16384, -21407 ; w07 w06 w03 w02
		sword 16384, -8867, 16384, -21407 ; w13 w12 w09 w08
		sword -16384, 21407, 16384, -8867 ; w15 w14 w11 w10
		sword 22725, 19266, 19266, -4520 ; w21 w20 w17 w16
		sword 12873, 4520, -22725, -12873 ; w23 w22 w19 w18
		sword 12873, -22725, 4520, -12873 ; w29 w28 w25 w24
		sword 4520, 19266, 19266, -22725 ; w31 w30 w27 w26

; Table for rows 1,7 - constants are multiplied by cos_1_16

tab_i_17_s	sword 22725, 29692, 22725, 12299 ; movq-> w05 w04 w01 w00
		sword 22725, 12299, -22725, -29692 ; w07 w06 w03 w02
		sword 22725, -12299, 22725, -29692 ; w13 w12 w09 w08
		sword -22725, 29692, 22725, -12299 ; w15 w14 w11 w10
		sword 31521, 26722, 26722, -6270 ; w21 w20 w17 w16
		sword 17855, 6270, -31521, -17855 ; w23 w22 w19 w18
		sword 17855, -31521, 6270, -17855 ; w29 w28 w25 w24
		sword 6270, 26722, 26722, -31521 ; w31 w30 w27 w26

; Table for rows 2,6 - constants are multiplied by cos_2_16

tab_i_26_s	sword 21407, 27969, 21407, 11585 ; movq-> w05 w04 w01 w00
		sword 21407, 11585, -21407, -27969 ; w07 w06 w03 w02
		sword 21407, -11585, 21407, -27969 ; w13 w12 w09 w08
		sword -21407, 27969, 21407, -11585 ; w15 w14 w11 w10
		sword 29692, 25172, 25172, -5906 ; w21 w20 w17 w16
		sword 16819, 5906, -29692, -16819 ; w23 w22 w19 w18
		sword 16819, -29692, 5906, -16819 ; w29 w28 w25 w24
		sword 5906, 25172, 25172, -29692 ; w31 w30 w27 w26

; Table for rows 3,5 - constants are multiplied by cos_3_16

tab_i_35_s	sword 19266, 25172, 19266, 10426 ; movq-> w05 w04 w01 w00
		sword 19266, 10426, -19266, -25172 ; w07 w06 w03 w02
		sword 19266, -10426, 19266, -25172 ; w13 w12 w09 w08
		sword -19266, 25172, 19266, -10426 ; w15 w14 w11 w10
		sword 26722, 22654, 22654, -5315 ; w21 w20 w17 w16
		sword 15137, 5315, -26722, -15137 ; w23 w22 w19 w18
		sword 15137, -26722, 5315, -15137 ; w29 w28 w25 w24
		sword 5315, 22654, 22654, -26722 ; w31 w30 w27 w26

;-----------------------------------------------------------------------------

DCT_8_INV_ROW_1_s MACRO INP:REQ, OUT:REQ, TABLE:REQ, ROUNDER:REQ

	movq 	mm0, mptr [INP] 	; 0 	; x3 x2 x1 x0

	movq 	mm1, mptr [INP+8]	; 1 	; x7 x6 x5 x4
	movq 	mm2, mm0 		; 2 	; x3 x2 x1 x0

	movq 	mm3, mptr [TABLE] 	; 3 	; w05 w04 w01 w00
	pshufw	mm0, mm0, 10001000b 	; x2 x0 x2 x0

	movq 	mm4, mptr [TABLE+8] 	; 4 	; w07 w06 w03 w02
	movq 	mm5, mm1		; 5 	; x7 x6 x5 x4
	pmaddwd mm3, mm0 		; x2*w05+x0*w04 x2*w01+x0*w00

	movq 	mm6, mptr [TABLE+32] 	; 6 	; w21 w20 w17 w16
	pshufw 	mm1, mm1, 10001000b 		; x6 x4 x6 x4
	pmaddwd mm4, mm1 			; x6*w07+x4*w06 x6*w03+x4*w02

	movq 	mm7, mptr [TABLE+40] 	; 7 	; w23 w22 w19 w18
	pshufw 	mm2, mm2, 11011101b 		; x3 x1 x3 x1
	pmaddwd mm6, mm2 			; x3*w21+x1*w20 x3*w17+x1*w16

	pshufw 	mm5, mm5, 11011101b 		; x7 x5 x7 x5
	pmaddwd mm7, mm5 			; x7*w23+x5*w22 x7*w19+x5*w18

	paddd 	mm3, mptr [ROUNDER] 		; +rounder

	pmaddwd mm0, mptr [TABLE+16] 		; x2*w13+x0*w12 x2*w09+x0*w08
	paddd 	mm3, mm4 		; 4 	; a1=sum(even1) a0=sum(even0)

	pmaddwd mm1, mptr [TABLE+24] 		; x6*w15+x4*w14 x6*w11+x4*w10
	movq 	mm4, mm3 		; 4 	; a1 a0

	pmaddwd mm2, mptr [TABLE+48] 		; x3*w29+x1*w28 x3*w25+x1*w24
	paddd 	mm6, mm7 		; 7 	; b1=sum(odd1) b0=sum(odd0)

	pmaddwd mm5, mptr [TABLE+56] 		; x7*w31+x5*w30 x7*w27+x5*w26
	paddd mm3, mm6 				; a1+b1 a0+b0

	paddd mm0, mptr [ROUNDER] 		; +rounder
	psrad mm3, SHIFT_INV_ROW 		; y1=a1+b1 y0=a0+b0

	paddd mm0, mm1 			; 1 	; a3=sum(even3) a2=sum(even2)
	psubd mm4, mm6 			; 6 	; a1-b1 a0-b0

	movq mm7, mm0 			; 7 	; a3 a2
	paddd mm2, mm5 			; 5 	; b3=sum(odd3) b2=sum(odd2)

	paddd mm0, mm2 				; a3+b3 a2+b2
	psrad mm4, SHIFT_INV_ROW 		; y6=a1-b1 y7=a0-b0

	psubd mm7, mm2 			; 2 	; a3-b3 a2-b2
	psrad mm0, SHIFT_INV_ROW 		; y3=a3+b3 y2=a2+b2

	psrad mm7, SHIFT_INV_ROW 		; y4=a3-b3 y5=a2-b2

	packssdw mm3, mm0 		; 0 	; y3 y2 y1 y0

	packssdw mm7, mm4 		; 4 	; y6 y7 y4 y5

	movq mptr [OUT], mm3 		; 3 	; save y3 y2 y1 y0
	pshufw mm7, mm7, 10110001b 		; y7 y6 y5 y4

	movq mptr [OUT+8], mm7 		; 7 	; save y7 y6 y5 y4
ENDM

;=============================================================================
;
;=============================================================================

;=============================================================================
;
; The first stage DCT 8x8 - forward DCTs of columns
;
; The outputs are multiplied
; for rows 0,4 - on cos_4_16,
; for rows 1,7 - on cos_1_16,
; for rows 2,6 - on cos_2_16,
; for rows 3,5 - on cos_3_16
; and are shifted to the left for rise of accuracy
;
;-----------------------------------------------------------------------------
;
; The 8-point scaled forward DCT algorithm (26a8m)
;
;-----------------------------------------------------------------------------
;
; #define DCT_8_FRW_COL(x, y)
;{
; short t0, t1, t2, t3, t4, t5, t6, t7;
; short tp03, tm03, tp12, tm12, tp65, tm65;
; short tp465, tm465, tp765, tm765;
;
; t0 = LEFT_SHIFT ( x[0] + x[7] );
; t1 = LEFT_SHIFT ( x[1] + x[6] );
; t2 = LEFT_SHIFT ( x[2] + x[5] );
; t3 = LEFT_SHIFT ( x[3] + x[4] );
; t4 = LEFT_SHIFT ( x[3] - x[4] );
; t5 = LEFT_SHIFT ( x[2] - x[5] );
; t6 = LEFT_SHIFT ( x[1] - x[6] );
; t7 = LEFT_SHIFT ( x[0] - x[7] );
;
; tp03 = t0 + t3;
; tm03 = t0 - t3;
; tp12 = t1 + t2;
; tm12 = t1 - t2;
;
; y[0] = tp03 + tp12;
; y[4] = tp03 - tp12;
;
; y[2] = tm03 + tm12 * tg_2_16;
; y[6] = tm03 * tg_2_16 - tm12;
;
; tp65 =(t6 +t5 )*cos_4_16;
; tm65 =(t6 -t5 )*cos_4_16;
;
; tp765 = t7 + tp65;
; tm765 = t7 - tp65;
; tp465 = t4 + tm65;
; tm465 = t4 - tm65;
;
; y[1] = tp765 + tp465 * tg_1_16;
; y[7] = tp765 * tg_1_16 - tp465;
; y[5] = tm765 * tg_3_16 + tm465;
; y[3] = tm765 - tm465 * tg_3_16;
;}
;
;=============================================================================
DCT_8_FRW_COL_4 MACRO INP:REQ, OUT:REQ
LOCAL x0, x1, x2, x3, x4, x5, x6, x7
LOCAL y0, y1, y2, y3, y4, y5, y6, y7
x0 equ [INP + 0*16]
x1 equ [INP + 1*16]
x2 equ [INP + 2*16]
x3 equ [INP + 3*16]
x4 equ [INP + 4*16]
x5 equ [INP + 5*16]
x6 equ [INP + 6*16]
x7 equ [INP + 7*16]
y0 equ [OUT + 0*16]
y1 equ [OUT + 1*16]
y2 equ [OUT + 2*16]
y3 equ [OUT + 3*16]
y4 equ [OUT + 4*16]
y5 equ [OUT + 5*16]
y6 equ [OUT + 6*16]
y7 equ [OUT + 7*16]
movq mm0, x1 ; 0 ; x1
movq mm1, x6 ; 1 ; x6
movq mm2, mm0 ; 2 ; x1
movq mm3, x2 ; 3 ; x2
paddsw mm0, mm1 ; t1 = x[1] + x[6]
movq mm4, x5 ; 4 ; x5
psllw mm0, SHIFT_FRW_COL ; t1
movq mm5, x0 ; 5 ; x0
paddsw mm4, mm3 ; t2 = x[2] + x[5]
paddsw mm5, x7 ; t0 = x[0] + x[7]
psllw mm4, SHIFT_FRW_COL ; t2
movq mm6, mm0 ; 6 ; t1
psubsw mm2, mm1 ; 1 ; t6 = x[1] - x[6]
movq mm1, mptr tg_2_16 ; 1 ; tg_2_16
psubsw mm0, mm4 ; tm12 = t1 - t2
movq mm7, x3 ; 7 ; x3
pmulhw mm1, mm0 ; tm12*tg_2_16
paddsw mm7, x4 ; t3 = x[3] + x[4]
psllw mm5, SHIFT_FRW_COL ; t0
paddsw mm6, mm4 ; 4 ; tp12 = t1 + t2
psllw mm7, SHIFT_FRW_COL ; t3
movq mm4, mm5 ; 4 ; t0
psubsw mm5, mm7 ; tm03 = t0 - t3
paddsw mm1, mm5 ; y2 = tm03 + tm12*tg_2_16
paddsw mm4, mm7 ; 7 ; tp03 = t0 + t3
por mm1, mptr one_corr ; correction y2 +0.5
psllw mm2, SHIFT_FRW_COL+1 ; t6
pmulhw mm5, mptr tg_2_16 ; tm03*tg_2_16
movq mm7, mm4 ; 7 ; tp03
psubsw mm3, x5 ; t5 = x[2] - x[5]
psubsw mm4, mm6 ; y4 = tp03 - tp12
movq y2, mm1 ; 1 ; save y2
paddsw mm7, mm6 ; 6 ; y0 = tp03 + tp12
movq mm1, x3 ; 1 ; x3
psllw mm3, SHIFT_FRW_COL+1 ; t5
psubsw mm1, x4 ; t4 = x[3] - x[4]
movq mm6, mm2 ; 6 ; t6
movq y4, mm4 ; 4 ; save y4
paddsw mm2, mm3 ; t6 + t5
pmulhw mm2, mptr ocos_4_16 ; tp65 = (t6 + t5)*cos_4_16
psubsw mm6, mm3 ; 3 ; t6 - t5
pmulhw mm6, mptr ocos_4_16 ; tm65 = (t6 - t5)*cos_4_16
psubsw mm5, mm0 ; 0 ; y6 = tm03*tg_2_16 - tm12
por mm5, mptr one_corr ; correction y6 +0.5
psllw mm1, SHIFT_FRW_COL ; t4
por mm2, mptr one_corr ; correction tp65 +0.5
movq mm4, mm1 ; 4 ; t4
movq mm3, x0 ; 3 ; x0
paddsw mm1, mm6 ; tp465 = t4 + tm65
psubsw mm3, x7 ; t7 = x[0] - x[7]
psubsw mm4, mm6 ; 6 ; tm465 = t4 - tm65
movq mm0, mptr tg_1_16 ; 0 ; tg_1_16
psllw mm3, SHIFT_FRW_COL ; t7
movq mm6, mptr tg_3_16 ; 6 ; tg_3_16
pmulhw mm0, mm1 ; tp465*tg_1_16
movq y0, mm7 ; 7 ; save y0
pmulhw mm6, mm4 ; tm465*tg_3_16
movq y6, mm5 ; 5 ; save y6
movq mm7, mm3 ; 7 ; t7
movq mm5, mptr tg_3_16 ; 5 ; tg_3_16
psubsw mm7, mm2 ; tm765 = t7 - tp65
paddsw mm3, mm2 ; 2 ; tp765 = t7 + tp65
pmulhw mm5, mm7 ; tm765*tg_3_16
paddsw mm0, mm3 ; y1 = tp765 + tp465*tg_1_16
paddsw mm6, mm4 ; tm465*tg_3_16
pmulhw mm3, mptr tg_1_16 ; tp765*tg_1_16
por mm0, mptr one_corr ; correction y1 +0.5
paddsw mm5, mm7 ; tm765*tg_3_16
psubsw mm7, mm6 ; 6 ; y3 = tm765 - tm465*tg_3_16
movq y1, mm0 ; 0 ; save y1
paddsw mm5, mm4 ; 4 ; y5 = tm765*tg_3_16 + tm465
movq y3, mm7 ; 7 ; save y3
psubsw mm3, mm1 ; 1 ; y7 = tp765*tg_1_16 - tp465
movq y5, mm5 ; 5 ; save y5
movq y7, mm3 ; 3 ; save y7
ENDM

DCT_8_INV_COL_4 MACRO INP:REQ, OUT:REQ
	movq	mm0, qword ptr tg_3_16

	movq	mm3, qword ptr [INP+16*3]
	movq	mm1, mm0			; tg_3_16

	movq	mm5, qword ptr [INP+16*5]
	pmulhw	mm0, mm3			; x3*(tg_3_16-1)

	movq	mm4, qword ptr tg_1_16
	pmulhw	mm1, mm5			; x5*(tg_3_16-1)

	movq	mm7, qword ptr [INP+16*7]
	movq	mm2, mm4			; tg_1_16

	movq	mm6, qword ptr [INP+16*1]
	pmulhw	mm4, mm7			; x7*tg_1_16

	paddsw	mm0, mm3			; x3*tg_3_16
	pmulhw	mm2, mm6			; x1*tg_1_16

	paddsw	mm1, mm3			; x3+x5*(tg_3_16-1)
	psubsw	mm0, mm5			; x3*tg_3_16-x5 = tm35

	movq	mm3, qword ptr ocos_4_16
	paddsw	mm1, mm5			; x3+x5*tg_3_16 = tp35

	paddsw	mm4, mm6			; x1+tg_1_16*x7 = tp17
	psubsw	mm2, mm7			; x1*tg_1_16-x7 = tm17

	movq	mm5, mm4			; tp17
	movq	mm6, mm2			; tm17

	paddsw	mm5, mm1			; tp17+tp35 = b0
	psubsw	mm6, mm0			; tm17-tm35 = b3

	psubsw	mm4, mm1			; tp17-tp35 = t1
	paddsw	mm2, mm0			; tm17+tm35 = t2

	movq	mm7, qword ptr tg_2_16
	movq	mm1, mm4			; t1

;	movq	qword ptr [SCRATCH+0], mm5	; save b0
	movq	qword ptr [OUT+3*16], mm5	; save b0
	paddsw	mm1, mm2			; t1+t2

;	movq	qword ptr [SCRATCH+8], mm6	; save b3
	movq	qword ptr [OUT+5*16], mm6	; save b3
	psubsw	mm4, mm2			; t1-t2

	movq	mm5, qword ptr [INP+2*16]
	movq	mm0, mm7			; tg_2_16

	movq	mm6, qword ptr [INP+6*16]
	pmulhw	mm0, mm5			; x2*tg_2_16

	pmulhw	mm7, mm6			; x6*tg_2_16
; slot
	pmulhw	mm1, mm3			; ocos_4_16*(t1+t2) = b1/2
; slot
	movq	mm2, qword ptr [INP+0*16]
	pmulhw	mm4, mm3			; ocos_4_16*(t1-t2) = b2/2

	psubsw	mm0, mm6			; t2*tg_2_16-x6 = tm26
	movq	mm3, mm2			; x0

	movq	mm6, qword ptr [INP+4*16]
	paddsw	mm7, mm5			; x2+x6*tg_2_16 = tp26

	paddsw	mm2, mm6			; x0+x4 = tp04
	psubsw	mm3, mm6			; x0-x4 = tm04

	movq	mm5, mm2			; tp04
	movq	mm6, mm3			; tm04

	psubsw	mm2, mm7			; tp04-tp26 = a3
	paddsw	mm3, mm0			; tm04+tm26 = a1

	paddsw mm1, mm1				; b1
	paddsw mm4, mm4				; b2

	paddsw	mm5, mm7			; tp04+tp26 = a0
	psubsw	mm6, mm0			; tm04-tm26 = a2

	movq	mm7, mm3			; a1
	movq	mm0, mm6			; a2

	paddsw	mm3, mm1			; a1+b1
	paddsw	mm6, mm4			; a2+b2

	psraw	mm3, SHIFT_INV_COL		; dst1
	psubsw	mm7, mm1			; a1-b1

	psraw	mm6, SHIFT_INV_COL		; dst2
	psubsw	mm0, mm4			; a2-b2

;	movq	mm1, qword ptr [SCRATCH+0]	; load b0
	movq	mm1, qword ptr [OUT+3*16]	; load b0
	psraw	mm7, SHIFT_INV_COL		; dst6

	movq	mm4, mm5			; a0
	psraw	mm0, SHIFT_INV_COL		; dst5

	movq	qword ptr [OUT+1*16], mm3
	paddsw	mm5, mm1			; a0+b0

	movq	qword ptr [OUT+2*16], mm6
	psubsw	mm4, mm1			; a0-b0

;	movq	mm3, qword ptr [SCRATCH+8]	; load b3
	movq	mm3, qword ptr [OUT+5*16]	; load b3
	psraw	mm5, SHIFT_INV_COL		; dst0

	movq	mm6, mm2			; a3
	psraw	mm4, SHIFT_INV_COL		; dst7

	movq	qword ptr [OUT+5*16], mm0
	paddsw	mm2, mm3			; a3+b3

	movq	qword ptr [OUT+6*16], mm7
	psubsw	mm6, mm3			; a3-b3

	movq	qword ptr [OUT+0*16], mm5
	psraw	mm2, SHIFT_INV_COL		; dst3

	movq	qword ptr [OUT+7*16], mm4
	psraw	mm6, SHIFT_INV_COL		; dst4

	movq	qword ptr [OUT+3*16], mm2

	movq	qword ptr [OUT+4*16], mm6
ENDM

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'

;
; extern "C" __fastcall void idct8x8_mmx (short *src_result);
;
public  @MMX_IDCT@4

@MMX_IDCT@4 proc near
	mov     eax, ecx          ; source

	DCT_8_INV_ROW_1	[eax+0], [eax+0], tab_i_04, rounder_0
	DCT_8_INV_ROW_1	[eax+16], [eax+16], tab_i_17, rounder_1
	DCT_8_INV_ROW_1	[eax+32], [eax+32], tab_i_26, rounder_2
	DCT_8_INV_ROW_1	[eax+48], [eax+48], tab_i_35, rounder_3
	DCT_8_INV_ROW_1	[eax+64], [eax+64], tab_i_04, rounder_4
	DCT_8_INV_ROW_1	[eax+80], [eax+80], tab_i_35, rounder_5
	DCT_8_INV_ROW_1	[eax+96], [eax+96], tab_i_26, rounder_6
	DCT_8_INV_ROW_1	[eax+112], [eax+112], tab_i_17, rounder_7

	DCT_8_INV_COL_4 [eax+0],[eax+0]
	DCT_8_INV_COL_4 [eax+8],[eax+8]

	ret    

@MMX_IDCT@4 ENDP

_TEXT ENDS

_TEXT SEGMENT PARA PUBLIC USE32 'CODE'

;
; extern "C" __fastcall void idct8x8_sse (short *src_result);
;
public  @SSEMMX_IDCT@4

@SSEMMX_IDCT@4 proc near
	mov     eax, ecx          ; source

	DCT_8_INV_ROW_1_s [eax+0], [eax+0], tab_i_04_s, rounder_0
	DCT_8_INV_ROW_1_s [eax+16], [eax+16], tab_i_17_s, rounder_1
	DCT_8_INV_ROW_1_s [eax+32], [eax+32], tab_i_26_s, rounder_2
	DCT_8_INV_ROW_1_s [eax+48], [eax+48], tab_i_35_s, rounder_3
	DCT_8_INV_ROW_1_s [eax+64], [eax+64], tab_i_04_s, rounder_4
	DCT_8_INV_ROW_1_s [eax+80], [eax+80], tab_i_35_s, rounder_5
	DCT_8_INV_ROW_1_s [eax+96], [eax+96], tab_i_26_s, rounder_6
	DCT_8_INV_ROW_1_s [eax+112], [eax+112], tab_i_17_s, rounder_7

	DCT_8_INV_COL_4 [eax+0],[eax+0]
	DCT_8_INV_COL_4 [eax+8],[eax+8]

	ret

@SSEMMX_IDCT@4 ENDP

_TEXT ENDS

END