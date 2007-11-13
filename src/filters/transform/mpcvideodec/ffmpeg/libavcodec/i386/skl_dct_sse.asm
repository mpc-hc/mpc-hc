;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
; [BITS 32]

%include "skl_nasm.h"

;//////////////////////////////////////////////////////////////////////

%macro cfastcall 1
		global @%1@4
		%define %1 @%1@4
%endmacro

;cfastcall Skl_IDct16_SSE
globl Skl_IDct16_SSE
globl Skl_IDct16_MMX
globl Skl_IDct16_Put_SSE
globl Skl_IDct16_Put_MMX
globl Skl_IDct16_Add_SSE
globl Skl_IDct16_Add_MMX

;//////////////////////////////////////////////////////////////////////
;
;                          -=FDCT=-
;
; Vertical pass is an implementation of the scheme:
;  Loeffler C., Ligtenberg A., and Moschytz C.S.:
;  Practical Fast 1D DCT Algorithm with Eleven Multiplications,
;  Proc. ICASSP 1989, 988-991.
;
; Horizontal pass is a double 4x4 vector/matrix multiplication,
; (see also Intel's Application Note 922:
;  http://developer.intel.com/vtune/cbts/strmsimd/922down.htm
;  Copyright (C) 1999 Intel Corporation)
;
; Notes:
;  * tan(3pi/16) is greater than 0.5, and would use the
;    sign bit when turned into 16b fixed-point precision. So,
;    we use the trick: x*tan3 = x*(tan3-1)+x
;
;  * There's only one SSE-specific instruction (pshufw).
;
;  * There's still 1 or 2 ticks to save in fLLM_PASS, but
;    I prefer having a readable code, instead of a tightly
;    scheduled one...
;
;  * Quantization stage (as well as pre-transposition for the
;    idct way back) can be included in the fTab* constants
;    (with induced loss of precision, somehow)
;
;  * Some more details at: http://skal.planet-d.net/coding/dct.html
;
;
;//////////////////////////////////////////////////////////////////////
;
;  == Mean square errors ==
;   0.000 0.001 0.001 0.002 0.000 0.002 0.001 0.000    [0.001]
;   0.035 0.029 0.032 0.032 0.031 0.032 0.034 0.035    [0.032]
;   0.026 0.028 0.027 0.027 0.025 0.028 0.028 0.025    [0.027]
;   0.037 0.032 0.031 0.030 0.028 0.029 0.026 0.031    [0.030]
;   0.000 0.001 0.001 0.002 0.000 0.002 0.001 0.001    [0.001]
;   0.025 0.024 0.022 0.022 0.022 0.022 0.023 0.023    [0.023]
;   0.026 0.028 0.025 0.028 0.030 0.025 0.026 0.027    [0.027]
;   0.021 0.020 0.020 0.022 0.020 0.022 0.017 0.019    [0.020]
;
;  == Abs Mean errors ==
;   0.000 0.000 0.000 0.000 0.000 0.000 0.000 0.000    [0.000]
;   0.020 0.001 0.003 0.003 0.000 0.004 0.002 0.003    [0.002]
;   0.000 0.001 0.001 0.001 0.001 0.004 0.000 0.000    [0.000]
;   0.027 0.001 0.000 0.002 0.002 0.002 0.001 0.000    [0.003]
;   0.000 0.000 0.000 0.000 0.000 0.001 0.000 0.001    [-0.000]
;   0.001 0.003 0.001 0.001 0.002 0.001 0.000 0.000    [-0.000]
;   0.000 0.002 0.002 0.001 0.001 0.002 0.001 0.000    [-0.000]
;   0.000 0.002 0.001 0.002 0.001 0.002 0.001 0.001    [-0.000]
;
;  =========================
;  Peak error:   1.0000
;  Peak MSE:     0.0365
;  Overall MSE:  0.0201
;  Peak ME:      0.0265
;  Overall ME:   0.0006
;
;//////////////////////////////////////////////////////////////////////
;
;                          -=IDCT=-
;
; A little slower than fdct, because the final stages (butterflies and
; descaling) require some unpairable shifting and packing, all on
; the same CPU unit.
;
;   THIS IDCT IS NOT IEEE-COMPLIANT: IT WILL FAIL THE [-300,300]
;   INPUT RANGE TEST (because of overflow). But the [-256,255] one
;   is OK, and I'm fine with it (for now;)
;
;   If you want ultra-compliant iDCT, just %define IEEE_COMPLIANT.
;   The code will then pass the [-384,383] input test.
;
;  == Mean square errors ==
;   0.007 0.006 0.005 0.007 0.006 0.007 0.005 0.007    [0.006]
;   0.006 0.008 0.007 0.007 0.007 0.008 0.008 0.008    [0.007]
;   0.008 0.008 0.008 0.008 0.007 0.009 0.010 0.007    [0.008]
;   0.007 0.007 0.006 0.007 0.008 0.007 0.006 0.008    [0.007]
;   0.007 0.006 0.006 0.006 0.006 0.005 0.006 0.006    [0.006]
;   0.008 0.007 0.006 0.008 0.007 0.008 0.009 0.009    [0.008]
;   0.008 0.006 0.010 0.008 0.008 0.008 0.007 0.007    [0.008]
;   0.007 0.006 0.006 0.007 0.007 0.006 0.006 0.007    [0.006]
;
;  == Abs Mean errors ==
;   0.001 0.000 0.000 0.001 0.001 0.000 0.000 0.000    [0.000]
;   0.000 0.002 0.002 0.000 0.001 0.001 0.000 0.002    [0.000]
;   0.001 0.002 0.001 0.001 0.001 0.001 0.000 0.001    [-0.001]
;   0.000 0.002 0.000 0.000 0.001 0.000 0.000 0.001    [-0.000]
;   0.000 0.001 0.001 0.001 0.000 0.001 0.000 0.001    [0.000]
;   0.000 0.001 0.001 0.001 0.001 0.000 0.001 0.000    [0.000]
;   0.001 0.001 0.002 0.001 0.001 0.002 0.001 0.001    [0.001]
;   0.000 0.000 0.001 0.000 0.000 0.000 0.000 0.000    [0.000]
;
;  =========================
;
;  Peak error:   1.0000
;  Peak MSE:     0.0092
;  Overall MSE:  0.0071
;  Peak ME:      0.0022
;  Overall ME:  -0.0002
;
;//////////////////////////////////////////////////////////////////////

DATA

align 16
tan1:    dw  0x32ec,0x32ec,0x32ec,0x32ec    ; tan( pi/16)
tan2:    dw  0x6a0a,0x6a0a,0x6a0a,0x6a0a    ; tan(2pi/16)  (=sqrt(2)-1)
tan3:    dw  0xab0e,0xab0e,0xab0e,0xab0e    ; tan(3pi/16)-1
sqrt2:   dw  0x5a82,0x5a82,0x5a82,0x5a82    ; 0.5/sqrt(2)

;//////////////////////////////////////////////////////////////////////

align 16
iTab1:
  dw 0x4000, 0x539f, 0x4000, 0x22a3,
  dw 0x4000, 0x22a3, 0xc000, 0xac61,
  dw 0x4000, 0xdd5d, 0x4000, 0xac61,
  dw 0xc000, 0x539f, 0x4000, 0xdd5d,
  dw 0x58c5, 0x4b42, 0x4b42, 0xee58,
  dw 0x3249, 0x11a8, 0xa73b, 0xcdb7,
  dw 0x3249, 0xa73b, 0x11a8, 0xcdb7,
  dw 0x11a8, 0x4b42, 0x4b42, 0xa73b

iTab2:
  dw 0x58c5, 0x73fc, 0x58c5, 0x300b,
  dw 0x58c5, 0x300b, 0xa73b, 0x8c04,
  dw 0x58c5, 0xcff5, 0x58c5, 0x8c04,
  dw 0xa73b, 0x73fc, 0x58c5, 0xcff5,
  dw 0x7b21, 0x6862, 0x6862, 0xe782,
  dw 0x45bf, 0x187e, 0x84df, 0xba41,
  dw 0x45bf, 0x84df, 0x187e, 0xba41,
  dw 0x187e, 0x6862, 0x6862, 0x84df

iTab3:
  dw 0x539f, 0x6d41, 0x539f, 0x2d41,
  dw 0x539f, 0x2d41, 0xac61, 0x92bf,
  dw 0x539f, 0xd2bf, 0x539f, 0x92bf,
  dw 0xac61, 0x6d41, 0x539f, 0xd2bf,
  dw 0x73fc, 0x6254, 0x6254, 0xe8ee,
  dw 0x41b3, 0x1712, 0x8c04, 0xbe4d,
  dw 0x41b3, 0x8c04, 0x1712, 0xbe4d,
  dw 0x1712, 0x6254, 0x6254, 0x8c04

iTab4:
  dw 0x4b42, 0x6254, 0x4b42, 0x28ba,
  dw 0x4b42, 0x28ba, 0xb4be, 0x9dac,
  dw 0x4b42, 0xd746, 0x4b42, 0x9dac,
  dw 0xb4be, 0x6254, 0x4b42, 0xd746,
  dw 0x6862, 0x587e, 0x587e, 0xeb3d,
  dw 0x3b21, 0x14c3, 0x979e, 0xc4df,
  dw 0x3b21, 0x979e, 0x14c3, 0xc4df,
  dw 0x14c3, 0x587e, 0x587e, 0x979e

align 16
iTab1_MMX:
  dw 0x4000, 0x4000, 0x4000, 0xc000
  dw 0x539f, 0x22a3, 0x22a3, 0xac61
  dw 0x4000, 0xc000, 0x4000, 0x4000
  dw 0xdd5d, 0x539f, 0xac61, 0xdd5d
  dw 0x58c5, 0x3249, 0x4b42, 0xa73b
  dw 0x4b42, 0x11a8, 0xee58, 0xcdb7
  dw 0x3249, 0x11a8, 0x11a8, 0x4b42
  dw 0xa73b, 0x4b42, 0xcdb7, 0xa73b

iTab2_MMX:
  dw 0x58c5, 0x58c5, 0x58c5, 0xa73b
  dw 0x73fc, 0x300b, 0x300b, 0x8c04
  dw 0x58c5, 0xa73b, 0x58c5, 0x58c5
  dw 0xcff5, 0x73fc, 0x8c04, 0xcff5
  dw 0x7b21, 0x45bf, 0x6862, 0x84df
  dw 0x6862, 0x187e, 0xe782, 0xba41
  dw 0x45bf, 0x187e, 0x187e, 0x6862
  dw 0x84df, 0x6862, 0xba41, 0x84df

iTab3_MMX:
  dw 0x539f, 0x539f, 0x539f, 0xac61
  dw 0x6d41, 0x2d41, 0x2d41, 0x92bf
  dw 0x539f, 0xac61, 0x539f, 0x539f
  dw 0xd2bf, 0x6d41, 0x92bf, 0xd2bf
  dw 0x73fc, 0x41b3, 0x6254, 0x8c04
  dw 0x6254, 0x1712, 0xe8ee, 0xbe4d
  dw 0x41b3, 0x1712, 0x1712, 0x6254
  dw 0x8c04, 0x6254, 0xbe4d, 0x8c04

iTab4_MMX:
  dw 0x4b42, 0x4b42, 0x4b42, 0xb4be
  dw 0x6254, 0x28ba, 0x28ba, 0x9dac
  dw 0x4b42, 0xb4be, 0x4b42, 0x4b42
  dw 0xd746, 0x6254, 0x9dac, 0xd746
  dw 0x6862, 0x3b21, 0x587e, 0x979e
  dw 0x587e, 0x14c3, 0xeb3d, 0xc4df
  dw 0x3b21, 0x14c3, 0x14c3, 0x587e
  dw 0x979e, 0x587e, 0xc4df, 0x979e

  ; the original rounding trick is by
  ; Michel Lespinasse (hi Walken!) <walken@zoy.org>

align 16
Idct_Rnd0      dd  65535, 65535
Idct_Rnd1      dd   3612,  3612
Idct_Rnd2      dd   2271,  2271
Idct_Rnd3      dd   1203,  1203
Idct_Rnd4      dd   1023,  1023
Idct_Rnd5      dd    102,   102
Idct_Rnd6      dd    398,   398
Idct_Rnd7      dd    469,   469

Idct_Sparse_Rnd0  times 4 dw  (65535>>11)
Idct_Sparse_Rnd1  times 4 dw  ( 3612>>11)
Idct_Sparse_Rnd2  times 4 dw  ( 2271>>11)
  ; other rounders are zero...

;//////////////////////////////////////////////////////////////////////

align 16
fTab1:
  dw 0x4000, 0x4000, 0x58c5, 0x4b42,
  dw 0x4000, 0x4000, 0x3249, 0x11a8,
  dw 0x539f, 0x22a3, 0x4b42, 0xee58,
  dw 0xdd5d, 0xac61, 0xa73b, 0xcdb7,
  dw 0x4000, 0xc000, 0x3249, 0xa73b,
  dw 0xc000, 0x4000, 0x11a8, 0x4b42,
  dw 0x22a3, 0xac61, 0x11a8, 0xcdb7,
  dw 0x539f, 0xdd5d, 0x4b42, 0xa73b

fTab2:
  dw 0x58c5, 0x58c5, 0x7b21, 0x6862,
  dw 0x58c5, 0x58c5, 0x45bf, 0x187e,
  dw 0x73fc, 0x300b, 0x6862, 0xe782,
  dw 0xcff5, 0x8c04, 0x84df, 0xba41,
  dw 0x58c5, 0xa73b, 0x45bf, 0x84df,
  dw 0xa73b, 0x58c5, 0x187e, 0x6862,
  dw 0x300b, 0x8c04, 0x187e, 0xba41,
  dw 0x73fc, 0xcff5, 0x6862, 0x84df

fTab3:
  dw 0x539f, 0x539f, 0x73fc, 0x6254,
  dw 0x539f, 0x539f, 0x41b3, 0x1712,
  dw 0x6d41, 0x2d41, 0x6254, 0xe8ee,
  dw 0xd2bf, 0x92bf, 0x8c04, 0xbe4d,
  dw 0x539f, 0xac61, 0x41b3, 0x8c04,
  dw 0xac61, 0x539f, 0x1712, 0x6254,
  dw 0x2d41, 0x92bf, 0x1712, 0xbe4d,
  dw 0x6d41, 0xd2bf, 0x6254, 0x8c04

fTab4:
  dw 0x4b42, 0x4b42, 0x6862, 0x587e,
  dw 0x4b42, 0x4b42, 0x3b21, 0x14c3,
  dw 0x6254, 0x28ba, 0x587e, 0xeb3d,
  dw 0xd746, 0x9dac, 0x979e, 0xc4df,
  dw 0x4b42, 0xb4be, 0x3b21, 0x979e,
  dw 0xb4be, 0x4b42, 0x14c3, 0x587e,
  dw 0x28ba, 0x9dac, 0x14c3, 0xc4df,
  dw 0x6254, 0xd746, 0x587e, 0x979e

align 16
Fdct_Rnd0: dw  6,8,8,8
Fdct_Rnd1: dw  8,8,8,8
Fdct_Rnd2: dw 10,8,8,8
MMX_One:   dw  1,1,1,1

;//////////////////////////////////////////////////////////////////////

TEXT

;//////////////////////////////////////////////////////////////////////
; iMTX_MULT (~24c)
;//////////////////////////////////////////////////////////////////////

%macro iMTX_MULT 4   ; %1=src, %2 = Table to use, %3=rounder, %4=Shift
  movq mm0, [ecx+%1*16+0]     ; mm0 = [0123]
  movq mm1, [ecx+%1*16+8]     ; mm1 = [4567]

  movq    mm3, [%2+0]   ;  [   M00    M01      M04    M05]
  pshufw  mm2, mm0, 11011101b ;  [1313]
  movq    mm4, [%2+8]   ;  [   M02    M03      M06    M07]
  pshufw  mm0, mm0, 10001000b ;  [0202]
  movq    mm6, [%2+32]  ;  [   M16    M17      M20    M21]
  pshufw  mm5, mm1, 11011101b ;  [5757]
  movq    mm7, [%2+40]  ;  [   M18    M19      M22    M23]
  pshufw  mm1, mm1, 10001000b ;  [4646]
  pmaddwd mm3, mm0      ;  [i0.M00+i2.M01 | i0.M04+i2.M05]
  pmaddwd mm6, mm2      ;  [i1.M16+i3.M17 | i1.M20+i3.M21]
  pmaddwd mm4, mm1      ;  [i4.M02+i6.M03 | i4.M06+i6.M07]
  pmaddwd mm7, mm5      ;  [i5.M18+i7.M19 | i5.M22+i7.M23]
  pmaddwd mm2, [%2+48]  ;  [i1.M24+i3.M25 | i1.M28+i3.M29]
  pmaddwd mm5, [%2+56]  ;  [i5.M26+i7.M27 | i5.M30+i7.M31]
  pmaddwd mm0, [%2+16]  ;  [i0.M08+i2.M09 | i0.M12+i2.M13]
  pmaddwd mm1, [%2+24]  ;  [i4.M10+i6.M11 | i4.M14+i6.M15]

  paddd   mm3, [%3]     ;  Round
  paddd   mm6, mm7      ;  => b0 | b1
  paddd   mm0, [%3]     ;  Round
  paddd   mm2, mm5      ;  => b2 | b3
  paddd   mm3, mm4      ;  => a0 | a1
  paddd   mm0, mm1      ;  => a2 | a3

  movq    mm4, mm3      ;     a0 | a1
  movq    mm7, mm0      ;     a2 | a3
  paddd   mm3, mm6      ;  a0+b0 | a1+b1
  psubd   mm4, mm6      ;  a0-b0 | a1-b1
  psubd   mm7, mm2      ;  a2-b2 | a3-b3
  paddd   mm0, mm2      ;  a2+b2 | a3+b3

  psrad   mm3, %4       ;  => out0 | out1
  psrad   mm4, %4       ;  => out7 | out6
  psrad   mm0, %4       ;  => out2 | out3
  psrad   mm7, %4       ;  => out5 | out4

  packssdw  mm3, mm0            ;  [0123]
  packssdw  mm7, mm4            ;  [5476]

  movq    [ecx+%1*16+0], mm3
  pshufw  mm7, mm7, 10110001b   ;  [4567]

  movq    [ecx+%1*16+8], mm7
%endmacro

%macro iMTX_MULT_03 4   ; %1=src, %2 = Table to use, %3=rounder, %4=Shift
    ; this version assume [4567] coeffs are zero...
  movq mm0, [ecx+%1*16+0]     ; mm0 = [0123]

  movq    mm3, [%2+0]   ;  [   M00    M01      M04    M05]
  pshufw  mm2, mm0, 11011101b ;  [1313]
  movq    mm6, [%2+32]  ;  [   M16    M17      M20    M21]
  pshufw  mm0, mm0, 10001000b ;  [0202]

  pmaddwd mm6, mm2      ;  [i1.M16+i3.M17 | i1.M20+i3.M21]
  pmaddwd mm3, mm0      ;  [i0.M00+i2.M01 | i0.M04+i2.M05]
  pmaddwd mm2, [%2+48]  ;  [i1.M24+i3.M25 | i1.M28+i3.M29]
  pmaddwd mm0, [%2+16]  ;  [i0.M08+i2.M09 | i0.M12+i2.M13]

    ; mm2=b2|b3  mm6  = b0|b1
  paddd   mm3, [%3]     ;  Round
  paddd   mm0, [%3]     ;  Round

  movq    mm4, mm3      ;     a0 | a1
  movq    mm7, mm0      ;     a2 | a3
  paddd   mm3, mm6      ;  a0+b0 | a1+b1
  psubd   mm4, mm6      ;  a0-b0 | a1-b1
  psubd   mm7, mm2      ;  a2-b2 | a3-b3
  paddd   mm0, mm2      ;  a2+b2 | a3+b3

  psrad   mm3, %4       ;  => out0 | out1
  psrad   mm4, %4       ;  => out7 | out6
  psrad   mm0, %4       ;  => out2 | out3
  psrad   mm7, %4       ;  => out5 | out4

  packssdw  mm3, mm0            ;  [0123]
  packssdw  mm7, mm4            ;  [5476]

  movq    [ecx+%1*16+0], mm3
  pshufw  mm7, mm7, 10110001b   ;  [4567]

  movq    [ecx+%1*16+8], mm7
%endmacro

;//////////////////////////////////////////////////////////////////////
; iMTX_MULT_MMX (~27c)
;//////////////////////////////////////////////////////////////////////

%macro iMTX_MULT_MMX 4   ; %1=src, %2 = Table to use, %3=rounder, %4=Shift
  movq   mm0, [ecx+%1*16+0] ; [0123]

  movq   mm1, [ecx+%1*16+8] ; [4567]
  movq   mm2, mm0

  punpcklwd mm0, mm1  ; [0415]
  punpckhwd mm2, mm1  ; [2637]

  movq mm1, mm0
  movq mm3, mm2

  punpckldq mm0, mm0  ; [0404]
  punpckldq mm2, mm2  ; [2626]
  punpckhdq mm1, mm1  ; [1515]
  punpckhdq mm3, mm3  ; [3737]

  movq    mm4, [%2+ 0]  ;  [   M00    M02      M04    M06]
  movq    mm6, [%2+ 8]  ;  [   M01    M03      M05    M07]
  pmaddwd mm4, mm0      ;  [i0.M00+i4.M02 | i0.M04+i4.M06]
  movq    mm5, [%2+32]  ;  [   M16    M18      M20    M22]
  movq    mm7, [%2+40]  ;  [   M17    M19      M21    M23]
  pmaddwd mm6, mm2      ;  [i2.M01+i6.M03 | i2.M05+i6.M07]
  pmaddwd mm5, mm1      ;  [i1.M16+i5.M18 | i1.M20+i5.M22]
  pmaddwd mm7, mm3      ;  [i3.M17+i7.M19 | i3.M21+i7.M23]
  pmaddwd mm0, [%2+16]  ;  [i0.M08+i4.M10 | i0.M12+i4.M14]
  pmaddwd mm2, [%2+24]  ;  [i2.M09+i6.M11 | i2.M13+i6.M15]
  pmaddwd mm1, [%2+48]  ;  [i1.M24+i5.M26 | i1.M28+i5.M30]
  pmaddwd mm3, [%2+56]  ;  [i3.M25+i7.M27 | i3.M29+i7.M31]

  paddd   mm0, [%3]     ;  Round
  paddd   mm1, mm3      ;  => b2 | b3
  paddd   mm4, [%3]     ;  Round
  paddd   mm5, mm7      ;  => b0 | b1
  paddd   mm4, mm6      ;  => a0 | a1
  movq    mm6, mm4      ;     a0 | a1
  paddd   mm0, mm2      ;  => a2 | a3
  movq    mm2, mm0      ;     a2 | a3
  paddd   mm4, mm5      ;  a0+b0 | a1+b1
  psrad   mm4, %4       ;  => out0 | out1
  paddd   mm0, mm1      ;  a2+b2 | a3+b3
  psrad   mm0, %4       ;  => out2 | out3

  psubd   mm6, mm5      ;  a0-b0 | a1-b1
  psubd   mm2, mm1      ;  a2-b2 | a3-b3
  psrad   mm6, %4       ;  => out7 | out6
  psrad   mm2, %4       ;  => out5 | out4


  packssdw  mm2, mm6    ;  [5476]
  packssdw  mm4, mm0    ;  [0123]

  movq  mm7, mm2
  psrld mm2, 16         ;  [4-6-]
  pslld mm7, 16         ;  [-5-7]

  movq    [ecx+%1*16+0], mm4
  por   mm2, mm7
  movq    [ecx+%1*16+8], mm2
%endmacro

;//////////////////////////////////////////////////////////////////////
;// iLLM_PASS (~42c)
;//////////////////////////////////////////////////////////////////////

%macro ADD_TO_DST 4   ; %1:src1   %2:dst1   %3:src2   %4:dst2
                      ; trashes mm0,mm4
  punpcklbw mm0, [%2]
  punpcklbw mm4, [%4]
  psrlw     mm0, 8   ; will zero the high words
  psrlw     mm4, 8
  paddsw    mm0, %1
  paddsw    mm4, %3
  packuswb  mm0, mm0
  packuswb  mm4, mm4
  movd      [%2], mm0
  movd      [%4], mm4
%endmacro

%macro iLLM_PASS 3  ; %1: src/dst, %2: combine func (0:none, 1:Put,  2:Add)
                    ; %3: dst offset (only for Add)

  movq   mm0, [tan3]    ; t3-1
  movq   mm3, [%1+16*3] ; x3
  movq   mm1, mm0       ; t3-1
  movq   mm5, [%1+16*5] ; x5

  movq   mm4, [tan1]    ; t1
  movq   mm6, [%1+16*1] ; x1
  movq   mm7, [%1+16*7] ; x7
  movq   mm2, mm4       ; t1

  pmulhw mm0, mm3       ; x3*(t3-1)
  pmulhw mm1, mm5       ; x5*(t3-1)
  paddsw mm0, mm3       ; x3*t3
  paddsw mm1, mm5       ; x5*t3
  psubsw mm0, mm5       ; x3*t3-x5 = tm35
  paddsw mm1, mm3       ; x3+x5*t3 = tp35

  pmulhw mm4, mm7       ; x7*t1
  pmulhw mm2, mm6       ; x1*t1
  paddsw mm4, mm6       ; x1+t1*x7 = tp17
  psubsw mm2, mm7       ; x1*t1-x7 = tm17


  movq   mm3, [sqrt2]
  movq   mm7, mm4
  movq   mm6, mm2
  psubsw mm4, mm1       ; tp17-tp35 = t1
  psubsw mm2, mm0       ; tm17-tm35 = b3
  paddsw mm1, mm7       ; tp17+tp35 = b0
  paddsw mm0, mm6       ; tm17+tm35 = t2

    ; mm1 = b0, mm2 = b3. preserved

  movq   mm6, mm4
  psubsw mm4, mm0       ; t1-t2
  paddsw mm0, mm6       ; t1+t2

  pmulhw mm4, mm3       ; (t1-t2)/(2.sqrt2)
  pmulhw mm0, mm3       ; (t1+t2)/(2.sqrt2)

  paddsw mm0, mm0       ; 2.(t1+t2) = b1
  paddsw mm4, mm4       ; 2.(t1-t2) = b2

  movq   mm7, [tan2]    ; t2
  movq   mm3, [%1+2*16] ; x2
  movq   mm6, [%1+6*16] ; x6
  movq   mm5, mm7       ; t2

  pmulhw mm7, mm6       ; x6*t2
  pmulhw mm5, mm3       ; x2*t2

  paddsw mm7, mm3       ; x2+x6*t2 = tp26
  psubsw mm5, mm6       ; x2*t2-x6 = tm26

    ; use:mm3,mm5,mm6,mm7   frozen: mm0,mm4,mm1,mm2

  movq   mm3, [%1+0*16] ; x0
  movq   mm6, [%1+4*16] ; x4

%ifndef IEEE_COMPLIANT

  psubsw mm3, mm6   ; x0-x4 = tm04
  paddsw mm6, mm6
  paddsw mm6, mm3   ; x0+x4 = tp04

  psubsw mm3, mm5   ; tm04-tm26 = a2
  psubsw mm6, mm7   ; tp04-tp26 = a3
  paddsw mm5, mm5   ; 2.tm26
  paddsw mm7, mm7   ; 2.tp26
  paddsw mm5, mm3   ; tm04+tm26 = a1
  paddsw mm7, mm6   ; tp04+tp26 = a0

  psubsw mm5, mm0   ; a1-b1
  psubsw mm3, mm4   ; a2-b2
  paddsw mm0, mm0   ; 2.b1
  paddsw mm4, mm4   ; 2.b2
  paddsw mm0, mm5   ; a1+b1
  paddsw mm4, mm3   ; a2+b2

%else

    ; we spill 1 reg to perform safe butterflies
  movq [%1   ], mm2

  movq   mm2, mm3
  psubsw mm3, mm6   ; x0-x4 = tm04
  paddsw mm6, mm2   ; x0+x4 = tp04

  movq   mm2, mm6
  psubsw mm6, mm7
  paddsw mm7, mm2
  movq   mm2, mm3
  psubsw mm3, mm5
  paddsw mm5, mm2

  movq   mm2, mm5
  psubsw mm5, mm0
  paddsw mm0, mm2
  movq   mm2, mm3
  psubsw mm3, mm4
  paddsw mm4, mm2

  movq mm2, [%1]

%endif

  psraw  mm5, 6     ; out6
  psraw  mm3, 6     ; out5
  psraw  mm0, 6     ; out1
  psraw  mm4, 6     ; out2

%if (%2==0)

  movq   [%1+5*16], mm3
  movq   [%1+6*16], mm5
  movq   [%1+1*16], mm0
  movq   [%1+2*16], mm4

%elif (%2==2)
  movq [%1   ], mm0   ; spill
  movq [%1+16], mm4   ; spill
  ADD_TO_DST [%1], eax+  edx+%3, [%1+16], eax+2*edx+%3    ; #1 - #2
%else
  packuswb mm0,[%1+1*16+8]
  packuswb mm4,[%1+2*16+8]
  movq [eax+  edx], mm0   ; #1
  movq [eax+2*edx], mm4   ; #2
%endif

    ; reminder: mm1=b0, mm2=b3, mm7=a0, mm6=a3

  movq   mm0, mm7
  movq   mm4, mm6
  psubsw mm7, mm1   ; a0-b0
  psubsw mm6, mm2   ; a3-b3
  paddsw mm1, mm0   ; a0+b0
  paddsw mm2, mm4   ; a3+b3

  psraw  mm1, 6     ; out0
  psraw  mm7, 6     ; out7
  psraw  mm2, 6     ; out3
  psraw  mm6, 6     ; out4

%if (%2==0)

  movq   [%1+0*16], mm1
  movq   [%1+7*16], mm7
  movq   [%1+3*16], mm2
  movq   [%1+4*16], mm6

%elif (%2==2)
  ADD_TO_DST mm1, eax      +%3, mm6, eax+4*edx+%3  ; #0 - #4
  lea eax, [eax+2*edx]                             ; -> #2
  ADD_TO_DST mm2, eax+  edx+%3, mm5, eax+4*edx+%3  ; #3 - #6
  lea eax, [eax+  edx]                             ; -> #3
  ADD_TO_DST mm3, eax+2*edx+%3, mm7, eax+4*edx+%3  ; #5 - #7
%else
  packuswb mm1,[%1+0*16+8]
  packuswb mm6,[%1+4*16+8]
  packuswb mm2,[%1+3*16+8]
  packuswb mm5,[%1+6*16+8]
  packuswb mm3,[%1+5*16+8]
  packuswb mm7,[%1+7*16+8]
  movq [eax      ], mm1   ; #0
  movq [eax+4*edx], mm6   ; #4
  lea eax, [eax+2*edx]    ; -> #2
  movq [eax+  edx], mm2   ; #3
  movq [eax+4*edx], mm5   ; #6
  lea eax, [eax+  edx]    ; -> #3
  movq [eax+2*edx], mm3   ; #5
  movq [eax+4*edx], mm7   ; #7
%endif

%endmacro

%macro iLLM_PASS_03 3  ; %1: src/dst, %2: combine func (0:none, 1:Put,  2:Add)
                       ; %3: dst offset (only for Add)

  movq   mm0, [tan3]    ; t3-1
  movq   mm1, [%1+16*3] ; x3
  movq   mm2, [%1+16*1] ; x1

  pmulhw mm0, mm1       ; x3*(t3-1)
  movq   mm4, mm2
  pmulhw mm2, [tan1]    ; x1*t1 => mm2:tm17, mm4:tp17
  paddsw mm0, mm1       ; x3*t3 => mm0:tm35, mm1:tp35

  movq   mm3, [sqrt2]
  movq   mm7, mm4
  movq   mm6, mm2
  psubsw mm4, mm1       ; tp17-tp35 = t1
  psubsw mm2, mm0       ; tm17-tm35 = b3
  paddsw mm1, mm7       ; tp17+tp35 = b0
  paddsw mm0, mm6       ; tm17+tm35 = t2

    ; mm1 = b0, mm2 = b3. preserved

  movq   mm6, mm4
  psubsw mm4, mm0       ; t1-t2
  paddsw mm0, mm6       ; t1+t2

  pmulhw mm4, mm3       ; (t1-t2)/(2.sqrt2)
  pmulhw mm0, mm3       ; (t1+t2)/(2.sqrt2)

  paddsw mm0, mm0       ; 2.(t1+t2) = b1
  paddsw mm4, mm4       ; 2.(t1-t2) = b2

  movq   mm5, [tan2]    ; t2
  movq   mm3, [%1+0*16] ; x0 => mm3:tm04
  movq   mm7, [%1+2*16] ; x2 => mm7:tp26

  pmulhw mm5, mm7       ; x2*t2 => mm5:tm26

  movq   mm6, mm3   ; mm6:tp04

%ifndef IEEE_COMPLIANT

  psubsw mm3, mm5   ; tm04-tm26 = a2
  psubsw mm6, mm7   ; tp04-tp26 = a3
  paddsw mm5, mm5   ; 2.tm26
  paddsw mm7, mm7   ; 2.tp26
  paddsw mm5, mm3   ; tm04+tm26 = a1
  paddsw mm7, mm6   ; tp04+tp26 = a0

  psubsw mm5, mm0   ; a1-b1
  psubsw mm3, mm4   ; a2-b2
  paddsw mm0, mm0   ; 2.b1
  paddsw mm4, mm4   ; 2.b2
  paddsw mm0, mm5   ; a1+b1
  paddsw mm4, mm3   ; a2+b2

%else

    ; we spill 1 reg to perform safe butterflies
  movq [%1   ], mm2

  movq   mm2, mm6
  psubsw mm6, mm7
  paddsw mm7, mm2
  movq   mm2, mm3
  psubsw mm3, mm5
  paddsw mm5, mm2

  movq   mm2, mm5
  psubsw mm5, mm0
  paddsw mm0, mm2
  movq   mm2, mm3
  psubsw mm3, mm4
  paddsw mm4, mm2

  movq mm2, [%1]

%endif

  psraw  mm5, 6     ; out6
  psraw  mm3, 6     ; out5
  psraw  mm0, 6     ; out1
  psraw  mm4, 6     ; out2

%if (%2==0)

  movq   [%1+5*16], mm3
  movq   [%1+6*16], mm5
  movq   [%1+1*16], mm0
  movq   [%1+2*16], mm4

%elif (%2==2)
  movq [%1   ], mm0   ; spill
  movq [%1+16], mm4   ; spill
  ADD_TO_DST [%1], eax+  edx+%3, [%1+16], eax+2*edx+%3    ; #1 - #2
%else
  packuswb mm0,[%1+1*16+8]
  packuswb mm4,[%1+2*16+8]
  movq [eax+  edx], mm0   ; #1
  movq [eax+2*edx], mm4   ; #2
%endif

    ; reminder: mm1=b0, mm2=b3, mm7=a0, mm6=a3

  movq   mm0, mm7
  movq   mm4, mm6
  psubsw mm7, mm1   ; a0-b0
  psubsw mm6, mm2   ; a3-b3
  paddsw mm1, mm0   ; a0+b0
  paddsw mm2, mm4   ; a3+b3

  psraw  mm1, 6     ; out0
  psraw  mm7, 6     ; out7
  psraw  mm2, 6     ; out3
  psraw  mm6, 6     ; out4

%if (%2==0)

  movq   [%1+0*16], mm1
  movq   [%1+7*16], mm7
  movq   [%1+3*16], mm2
  movq   [%1+4*16], mm6

%elif (%2==2)
  ADD_TO_DST mm1, eax      +%3, mm6, eax+4*edx+%3  ; #0 - #4
  lea eax, [eax+2*edx]                             ; -> #2
  ADD_TO_DST mm2, eax+  edx+%3, mm5, eax+4*edx+%3  ; #3 - #6
  lea eax, [eax+  edx]                             ; -> #3
  ADD_TO_DST mm3, eax+2*edx+%3, mm7, eax+4*edx+%3  ; #5 - #7
%else
  packuswb mm1,[%1+0*16+8]
  packuswb mm6,[%1+4*16+8]
  packuswb mm2,[%1+3*16+8]
  packuswb mm5,[%1+6*16+8]
  packuswb mm3,[%1+5*16+8]
  packuswb mm7,[%1+7*16+8]
  movq [eax      ], mm1   ; #0
  movq [eax+4*edx], mm6   ; #4
  lea eax, [eax+2*edx]    ; -> #2
  movq [eax+  edx], mm2   ; #3
  movq [eax+4*edx], mm5   ; #6
  lea eax, [eax+  edx]    ; -> #3
  movq [eax+2*edx], mm3   ; #5
  movq [eax+4*edx], mm7   ; #7
%endif

%endmacro

;//////////////////////////////////////////////////////////////////////
;// basic IDCTs...
; Nic - Changed to fastcall convention
align 16
Skl_IDct16_SSE:  ; 249c
  mov ecx, [esp+4]  ; Old cdecl way, ecx is the pointer anyway with
  iMTX_MULT  0, iTab1, Idct_Rnd0, 11
  iMTX_MULT  1, iTab2, Idct_Rnd1, 11
  iMTX_MULT  2, iTab3, Idct_Rnd2, 11
  iMTX_MULT  3, iTab4, Idct_Rnd3, 11
  iMTX_MULT  4, iTab1, Idct_Rnd4, 11
  iMTX_MULT  5, iTab4, Idct_Rnd5, 11
  iMTX_MULT  6, iTab3, Idct_Rnd6, 11
  iMTX_MULT  7, iTab2, Idct_Rnd7, 11
  iLLM_PASS ecx+0, 0,0
  iLLM_PASS ecx+8, 0,0
  ret

align 16
Skl_IDct16_MMX:  ; 288c
  mov ecx, [esp+4]
  iMTX_MULT_MMX  0, iTab1_MMX, Idct_Rnd0, 11
  iMTX_MULT_MMX  1, iTab2_MMX, Idct_Rnd1, 11
  iMTX_MULT_MMX  2, iTab3_MMX, Idct_Rnd2, 11
  iMTX_MULT_MMX  3, iTab4_MMX, Idct_Rnd3, 11
  iMTX_MULT_MMX  4, iTab1_MMX, Idct_Rnd4, 11
  iMTX_MULT_MMX  5, iTab4_MMX, Idct_Rnd5, 11
  iMTX_MULT_MMX  6, iTab3_MMX, Idct_Rnd6, 11
  iMTX_MULT_MMX  7, iTab2_MMX, Idct_Rnd7, 11
  iLLM_PASS ecx+0, 0,0
  iLLM_PASS ecx+8, 0,0
  ret

;//////////////////////////////////////////////////////////////////////
;// Optimized Sparse/Put/Add MMX/SSE IDCTs

%macro TEST_ROW 3     ; %1:src,  %2:label x8, %3: label x4
  mov eax, [%1   ]
  mov edx, [%1+ 8]
  or  eax, [%1+ 4]
  or  edx, [%1+12]
  or  eax, edx
  jz near %2
  test edx, 0xffffffff
  jz near %3
%endmacro

%macro IDCT_IMPL  4   ; %1: combine func (0:none, 1:Put,  2:Add)
                      ; %2:HPASS macro, %3:HPASS-03 macro, %4:VPASS macro
  mov ecx, [esp+12]  ; Src

  TEST_ROW ecx, .Row0_Round, .Row0_4
  %2  0, ITAB1, Idct_Rnd0, 11
  jmp .Row1
.Row0_4
  %3  0, ITAB1, Idct_Rnd0, 11
  jmp .Row1
.Row0_Round
  movq mm0, [Idct_Sparse_Rnd0]
  movq [ecx  ], mm0
  movq [ecx+8], mm0

.Row1
  TEST_ROW ecx+16, .Row1_Round, .Row1_4
  %2  1, ITAB2, Idct_Rnd1, 11
  jmp .Row2
.Row1_4
  %3  1, ITAB2, Idct_Rnd1, 11
  jmp .Row2
.Row1_Round
  movq mm0, [Idct_Sparse_Rnd1]
  movq [ecx+16  ], mm0
  movq [ecx+16+8], mm0

.Row2
  TEST_ROW ecx+32, .Row2_Round, .Row2_4
  %2  2, ITAB3, Idct_Rnd2, 11
  jmp .Row3
.Row2_4
  %3  2, ITAB3, Idct_Rnd2, 11
  jmp .Row3
.Row2_Round
  movq mm0, [Idct_Sparse_Rnd2]
  movq [ecx+32  ], mm0
  movq [ecx+32+8], mm0

.Row3
  TEST_ROW ecx+48, .Row4, .Row3_4
  %2  3, ITAB4, Idct_Rnd3, 11
  jmp .Row4
.Row3_4
  %3  3, ITAB4, Idct_Rnd3, 11

.Row4
  TEST_ROW ecx+64, .Row5, .Row4_4
  %2  4, ITAB1, Idct_Rnd4, 11
  jmp .Row5
.Row4_4:
  %3  4, ITAB1, Idct_Rnd4, 11

.Row5
  TEST_ROW ecx+80, .Row6, .Row5_4
  %2  5, ITAB4, Idct_Rnd5, 11
  jmp .Row6
.Row5_4
  %3  5, ITAB4, Idct_Rnd5, 11

.Row6
  TEST_ROW ecx+96, .Row7, .Row6_4
  %2  6, ITAB3, Idct_Rnd6, 11
  jmp .Row7
.Row6_4
  %3  6, ITAB3, Idct_Rnd6, 11
.Row7
  TEST_ROW ecx+112, .End, .Row7_4
  %2  7, ITAB2, Idct_Rnd7, 11
  jmp .End
.Row7_4
  %3  7, ITAB2, Idct_Rnd7, 11
.End

%if (%1==0)
  %4 ecx+0, 0,0
  %4 ecx+8, 0,0
%elif (%1==1)
  mov eax, [esp+ 4]  ; Dst
  mov edx, [esp+ 8]  ; BpS
  %4 ecx+8, 0,0
  %4 ecx+0, 1,0
%else
  mov eax, [esp+ 4]  ; Dst
  mov edx, [esp+ 8]  ; BpS
  %4 ecx+0, 2,0
  mov eax, [esp+ 4]  ; reload Dst
  %4 ecx+8, 2,4
%endif

%endmacro

;//////////////////////////////////////////////////////////////////////

%define ITAB1 iTab1
%define ITAB2 iTab2
%define ITAB3 iTab3
%define ITAB4 iTab4

align 16
Skl_IDct16_Put_SSE:
  IDCT_IMPL 1, iMTX_MULT, iMTX_MULT_03, iLLM_PASS
  ret

align 16
Skl_IDct16_Add_SSE:
  IDCT_IMPL 2, iMTX_MULT, iMTX_MULT_03, iLLM_PASS
  ret

;//////////////////////////////////////////////////////////////////////

%define ITAB1 iTab1_MMX
%define ITAB2 iTab2_MMX
%define ITAB3 iTab3_MMX
%define ITAB4 iTab4_MMX

align 16
Skl_IDct16_Put_MMX:
  IDCT_IMPL 1, iMTX_MULT_MMX, iMTX_MULT_MMX, iLLM_PASS
  ret

align 16
Skl_IDct16_Add_MMX:
  IDCT_IMPL 2, iMTX_MULT_MMX, iMTX_MULT_MMX, iLLM_PASS
  ret

;//////////////////////////////////////////////////////////////////////
;// fLLM_PASS (~39c)
;//////////////////////////////////////////////////////////////////////

%macro fLLM_PASS 2  ; %1: src/dst, %2:Shift

  movq   mm0, [%1+0*16]   ; In0
  movq   mm2, [%1+2*16]   ; In2
  movq   mm3, mm0
  movq   mm4, mm2
  movq   mm7, [%1+7*16]   ; In7
  movq   mm5, [%1+5*16]   ; In5

  psubsw mm0, mm7         ; t7 = In0-In7
  paddsw mm7, mm3         ; t0 = In0+In7
  psubsw mm2, mm5         ; t5 = In2-In5
  paddsw mm5, mm4         ; t2 = In2+In5

  movq   mm3, [%1+3*16]   ; In3
  movq   mm4, [%1+4*16]   ; In4
  movq   mm1, mm3
  psubsw mm3, mm4         ; t4 = In3-In4
  paddsw mm4, mm1         ; t3 = In3+In4
  movq   mm6, [%1+6*16]   ; In6
  movq   mm1, [%1+1*16]   ; In1
  psubsw mm1, mm6         ; t6 = In1-In6
  paddsw mm6, [%1+1*16]   ; t1 = In1+In6

  psubsw mm7, mm4         ; tm03 = t0-t3
  psubsw mm6, mm5         ; tm12 = t1-t2
  paddsw mm4, mm4         ; 2.t3
  paddsw mm5, mm5         ; 2.t2
  paddsw mm4, mm7         ; tp03 = t0+t3
  paddsw mm5, mm6         ; tp12 = t1+t2

  psllw  mm2, %2+1        ; shift t5 (shift +1 to..
  psllw  mm1, %2+1        ; shift t6  ..compensate cos4/2)
  psllw  mm4, %2          ; shift t3
  psllw  mm5, %2          ; shift t2
  psllw  mm7, %2          ; shift t0
  psllw  mm6, %2          ; shift t1
  psllw  mm3, %2          ; shift t4
  psllw  mm0, %2          ; shift t7

  psubsw mm4, mm5         ; out4 = tp03-tp12
  psubsw mm1, mm2         ; mm1: t6-t5
  paddsw mm5, mm5
  paddsw mm2, mm2
  paddsw mm5, mm4         ; out0 = tp03+tp12
  movq   [%1+4*16], mm4   ; => out4
  paddsw mm2, mm1         ; mm2: t6+t5
  movq   [%1+0*16], mm5   ; => out0

  movq   mm4, [tan2]      ; mm4 <= tan2
  pmulhw mm4, mm7         ; tm03*tan2
  movq   mm5, [tan2]      ; mm5 <= tan2
  psubsw mm4, mm6         ; out6 = tm03*tan2 - tm12
  pmulhw mm5, mm6         ; tm12*tan2
  paddsw mm5, mm7         ; out2 = tm12*tan2 + tm03

  movq   mm6, [sqrt2]
  movq   mm7, [MMX_One]

  pmulhw mm2, mm6         ; mm2: tp65 = (t6 + t5)*cos4
  por    mm5, mm7         ; correct out2
  por    mm4, mm7         ; correct out6
  pmulhw mm1, mm6         ; mm1: tm65 = (t6 - t5)*cos4
  por    mm2, mm7         ; correct tp65

  movq   [%1+2*16], mm5   ; => out2
  movq   mm5, mm3         ; save t4
  movq   [%1+6*16], mm4   ; => out6
  movq   mm4, mm0         ; save t7

  psubsw mm3, mm1         ; mm3: tm465 = t4 - tm65
  psubsw mm0, mm2         ; mm0: tm765 = t7 - tp65
  paddsw mm2, mm4         ; mm2: tp765 = t7 + tp65
  paddsw mm1, mm5         ; mm1: tp465 = t4 + tm65

  movq   mm4, [tan3]      ; tan3 - 1
  movq   mm5, [tan1]      ; tan1

  movq   mm7, mm3         ; save tm465
  pmulhw mm3, mm4         ; tm465*(tan3-1)
  movq   mm6, mm1         ; save tp465
  pmulhw mm1, mm5         ; tp465*tan1

  paddsw mm3, mm7         ; tm465*tan3
  pmulhw mm4, mm0         ; tm765*(tan3-1)
  paddsw mm4, mm0         ; tm765*tan3
  pmulhw mm5, mm2         ; tp765*tan1

  paddsw mm1, mm2         ; out1 = tp765 + tp465*tan1
  psubsw mm0, mm3         ; out3 = tm765 - tm465*tan3
  paddsw mm7, mm4         ; out5 = tm465 + tm765*tan3
  psubsw mm5, mm6         ; out7 =-tp465 + tp765*tan1

  movq   [%1+1*16], mm1   ; => out1
  movq   [%1+3*16], mm0   ; => out3
  movq   [%1+5*16], mm7   ; => out5
  movq   [%1+7*16], mm5   ; => out7

%endmacro

;//////////////////////////////////////////////////////////////////////
;// fMTX_MULT (~20c)  (~26c for MMX)
;//////////////////////////////////////////////////////////////////////

%macro fMTX_MULT 5   ; %1=src, %2 = Coeffs, %3/%4=rounders, %5=MMX-Only

%if %5==0

      ; SSE version ('pshufw')

  movq    mm0, [ecx+%1*16+0]  ; mm0 = [0123]

  pshufw  mm1, [ecx+%1*16+8], 00011011b ; mm1 = [7654]
  movq    mm7, mm0

%else

      ; MMX-only version (~10% slower overall)

  movd    mm1, [ecx+%1*16+8+4]  ; [67..]
  movq    mm0, [ecx+%1*16+0]    ; mm0 = [0123]
  movq    mm7, mm0
  punpcklwd mm1, [ecx+%1*16+8]  ; [6475]
  movq    mm2, mm1
  psrlq   mm1, 32               ; [75..]
  punpcklwd mm1,mm2             ; [7654]

%endif

  paddsw  mm0, mm1      ; mm0 = [a0 a1 a2 a3]
  psubsw  mm7, mm1      ; mm7 = [b0 b1 b2 b3]

  movq      mm1, mm0
  punpckldq mm0, mm7    ; mm0 = [a0 a1 b0 b1]
  punpckhdq mm1, mm7    ; mm1 = [a2 a3 b2 b3]

  movq    mm2, [%2+ 0]  ;  [   M00    M01      M16    M17]
  movq    mm3, [%2+ 8]  ;  [   M02    M03      M18    M19]
  pmaddwd mm2, mm0      ;  [a0.M00+a1.M01 | b0.M16+b1.M17]
  movq    mm4, [%2+16]  ;  [   M04    M05      M20    M21]
  pmaddwd mm3, mm1      ;  [a2.M02+a3.M03 | b2.M18+b3.M19]
  movq    mm5, [%2+24]  ;  [   M06    M07      M22    M23]
  pmaddwd mm4, mm0      ;  [a0.M04+a1.M05 | b0.M20+b1.M21]
  movq    mm6, [%2+32]  ;  [   M08    M09      M24    M25]
  pmaddwd mm5, mm1      ;  [a2.M06+a3.M07 | b2.M22+b3.M23]
  movq    mm7, [%2+40]  ;  [   M10    M11      M26    M27]
  pmaddwd mm6, mm0      ;  [a0.M08+a1.M09 | b0.M24+b1.M25]
  paddd   mm2, mm3      ;  [ out0 | out1 ]
  pmaddwd mm7, mm1      ;  [a0.M10+a1.M11 | b0.M26+b1.M27]
  psrad   mm2, 16
  pmaddwd mm0, [%2+48]  ;  [a0.M12+a1.M13 | b0.M28+b1.M29]
  paddd   mm4, mm5      ;  [ out2 | out3 ]
  pmaddwd mm1, [%2+56]  ;  [a0.M14+a1.M15 | b0.M30+b1.M31]
  psrad   mm4, 16

  paddd   mm6, mm7            ;  [ out4 | out5 ]
  psrad   mm6, 16
  paddd   mm0, mm1            ;  [ out6 | out7 ]
  psrad   mm0, 16

  packssdw mm2, mm4           ;  [ out0|out1|out2|out3 ]
  paddsw   mm2, [%3]          ;  Round
  packssdw mm6, mm0           ;  [ out4|out5|out6|out7 ]
  paddsw   mm6, [%4]          ;  Round

  psraw   mm2, 4              ; => [-2048, 2047]
  psraw   mm6, 4

  movq    [ecx+%1*16+0], mm2
  movq    [ecx+%1*16+8], mm6

%endmacro

;//////////////////////////////////////////////////////////////////////
