;/********************************************************
; * Some code. Copyright (C) 2003 by Pascal Massimino.   *
; * All Rights Reserved.      (http://skal.planet-d.net) *
; * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
; ********************************************************/
; [BITS 32]

%include "skl_nasm.h"

%define IEEE_COMPLIANT

;//////////////////////////////////////////////////////////////////////

globl Skl_IDct16_SSE2
globl Skl_IDct16_Add_SSE2
globl Skl_IDct16_Put_SSE2

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
;  Peak MSE:     0.0096
;  Overall MSE:  0.0070
;  Peak ME:      0.0024
;  Overall ME:   0.0001
;
;//////////////////////////////////////////////////////////////////////

DATA

align 16
tan1:    times 8 dw 0x32ec    ; tan( pi/16)
tan2:    times 8 dw 0x6a0a    ; tan(2pi/16)  (=sqrt(2)-1)
tan3:    times 8 dw 0xab0e    ; tan(3pi/16)-1
sqrt2:   times 8 dw 0x5a82    ; 0.5/sqrt(2)

;//////////////////////////////////////////////////////////////////////

align 16
iTab1:
  dw 0x4000, 0x539f, 0x4000, 0x22a3
  dw 0x4000, 0xdd5d, 0x4000, 0xac61
  dw 0x4000, 0x22a3, 0xc000, 0xac61
  dw 0xc000, 0x539f, 0x4000, 0xdd5d
  dw 0x58c5, 0x4b42, 0x4b42, 0xee58
  dw 0x3249, 0xa73b, 0x11a8, 0xcdb7
  dw 0x3249, 0x11a8, 0xa73b, 0xcdb7
  dw 0x11a8, 0x4b42, 0x4b42, 0xa73b

iTab2:
  dw 0x58c5, 0x73fc, 0x58c5, 0x300b
  dw 0x58c5, 0xcff5, 0x58c5, 0x8c04
  dw 0x58c5, 0x300b, 0xa73b, 0x8c04
  dw 0xa73b, 0x73fc, 0x58c5, 0xcff5
  dw 0x7b21, 0x6862, 0x6862, 0xe782
  dw 0x45bf, 0x84df, 0x187e, 0xba41
  dw 0x45bf, 0x187e, 0x84df, 0xba41
  dw 0x187e, 0x6862, 0x6862, 0x84df

iTab3:
  dw 0x539f, 0x6d41, 0x539f, 0x2d41
  dw 0x539f, 0xd2bf, 0x539f, 0x92bf
  dw 0x539f, 0x2d41, 0xac61, 0x92bf
  dw 0xac61, 0x6d41, 0x539f, 0xd2bf
  dw 0x73fc, 0x6254, 0x6254, 0xe8ee
  dw 0x41b3, 0x8c04, 0x1712, 0xbe4d
  dw 0x41b3, 0x1712, 0x8c04, 0xbe4d
  dw 0x1712, 0x6254, 0x6254, 0x8c04

iTab4:
  dw 0x4b42, 0x6254, 0x4b42, 0x28ba
  dw 0x4b42, 0xd746, 0x4b42, 0x9dac
  dw 0x4b42, 0x28ba, 0xb4be, 0x9dac
  dw 0xb4be, 0x6254, 0x4b42, 0xd746
  dw 0x6862, 0x587e, 0x587e, 0xeb3d
  dw 0x3b21, 0x979e, 0x14c3, 0xc4df
  dw 0x3b21, 0x14c3, 0x979e, 0xc4df
  dw 0x14c3, 0x587e, 0x587e, 0x979e

  ; the original rounding trick is by
  ; Michel Lespinasse (hi Walken!) <walken@zoy.org>

align 16
Idct_Rnd0      dd  65535, 65535, 65535, 65535
Idct_Rnd1      dd   3612,  3612,  3612,  3612
Idct_Rnd2      dd   2271,  2271,  2271,  2271
Idct_Rnd3      dd   1203,  1203,  1203,  1203
Idct_Rnd4      dd   1023,  1023,  1023,  1023
Idct_Rnd5      dd    102,   102,   102,   102
Idct_Rnd6      dd    398,   398,   398,   398
Idct_Rnd7      dd    469,   469,   469,   469

Idct_Sparse_Rnd0  times 8 dw  (65535>>11)
Idct_Sparse_Rnd1  times 8 dw  ( 3612>>11)
Idct_Sparse_Rnd2  times 8 dw  ( 2271>>11)
  ; other rounders are zero...

;//////////////////////////////////////////////////////////////////////

align 16
fTab1:
  dw 0x4000, 0x4000, 0x58c5, 0x4b42,
  dw 0xdd5d, 0xac61, 0xa73b, 0xcdb7,
  dw 0x4000, 0x4000, 0x3249, 0x11a8,
  dw 0x539f, 0x22a3, 0x4b42, 0xee58,
  dw 0x4000, 0xc000, 0x3249, 0xa73b,
  dw 0x539f, 0xdd5d, 0x4b42, 0xa73b,
  dw 0xc000, 0x4000, 0x11a8, 0x4b42,
  dw 0x22a3, 0xac61, 0x11a8, 0xcdb7

fTab2:
  dw 0x58c5, 0x58c5, 0x7b21, 0x6862,
  dw 0xcff5, 0x8c04, 0x84df, 0xba41,
  dw 0x58c5, 0x58c5, 0x45bf, 0x187e,
  dw 0x73fc, 0x300b, 0x6862, 0xe782,
  dw 0x58c5, 0xa73b, 0x45bf, 0x84df,
  dw 0x73fc, 0xcff5, 0x6862, 0x84df,
  dw 0xa73b, 0x58c5, 0x187e, 0x6862,
  dw 0x300b, 0x8c04, 0x187e, 0xba41

fTab3:
  dw 0x539f, 0x539f, 0x73fc, 0x6254,
  dw 0xd2bf, 0x92bf, 0x8c04, 0xbe4d,
  dw 0x539f, 0x539f, 0x41b3, 0x1712,
  dw 0x6d41, 0x2d41, 0x6254, 0xe8ee,
  dw 0x539f, 0xac61, 0x41b3, 0x8c04,
  dw 0x6d41, 0xd2bf, 0x6254, 0x8c04,
  dw 0xac61, 0x539f, 0x1712, 0x6254,
  dw 0x2d41, 0x92bf, 0x1712, 0xbe4d

fTab4:
  dw 0x4b42, 0x4b42, 0x6862, 0x587e,
  dw 0xd746, 0x9dac, 0x979e, 0xc4df,
  dw 0x4b42, 0x4b42, 0x3b21, 0x14c3,
  dw 0x6254, 0x28ba, 0x587e, 0xeb3d,
  dw 0x4b42, 0xb4be, 0x3b21, 0x979e,
  dw 0x6254, 0xd746, 0x587e, 0x979e,
  dw 0xb4be, 0x4b42, 0x14c3, 0x587e,
  dw 0x28ba, 0x9dac, 0x14c3, 0xc4df


align 16
Fdct_Rnd0: dw  6,8,8,8, 6,8,8,8
Fdct_Rnd1: dw  8,8,8,8, 8,8,8,8
Fdct_Rnd2: dw 10,8,8,8, 8,8,8,8
Rounder1:  dw  1,1,1,1, 1,1,1,1

;//////////////////////////////////////////////////////////////////////

TEXT

;//////////////////////////////////////////////////////////////////////
; iMTX_MULT
;//////////////////////////////////////////////////////////////////////

%macro iMTX_MULT 4   ; %1=src, %2 = Table to use, %3=rounder, %4=Shift

  movdqa  xmm0, [ecx+%1*16]     ; xmm0 = [01234567]

  pshuflw xmm0, xmm0, 11011000b ; [02134567]  ; these two shufflings could be
  pshufhw xmm0, xmm0, 11011000b ; [02134657]  ; integrated in zig-zag orders

  pshufd  xmm4, xmm0, 00000000b ; [02020202]
  pshufd  xmm5, xmm0, 10101010b ; [46464646]
  pshufd  xmm6, xmm0, 01010101b ; [13131313]
  pshufd  xmm7, xmm0, 11111111b ; [57575757]

  pmaddwd xmm4, [%2+ 0]   ; dot [M00,M01][M04,M05][M08,M09][M12,M13]
  pmaddwd xmm5, [%2+16]   ; dot [M02,M03][M06,M07][M10,M11][M14,M15]
  pmaddwd xmm6, [%2+32]   ; dot [M16,M17][M20,M21][M24,M25][M28,M29]
  pmaddwd xmm7, [%2+48]   ; dot [M18,M19][M22,M23][M26,M27][M30,M31]
  paddd   xmm4, [%3]      ; Round

  paddd   xmm6, xmm7      ; [b0|b1|b2|b3]
  paddd   xmm4, xmm5      ; [a0|a1|a2|a3]

  movdqa  xmm7, xmm6
  paddd   xmm6, xmm4      ; mm6=a+b
  psubd   xmm4, xmm7      ; mm4=a-b
  psrad   xmm6, %4        ; => out [0123]
  psrad   xmm4, %4        ; => out [7654]

  packssdw xmm6, xmm4     ; [01237654]

  pshufhw xmm6, xmm6, 00011011b ; [01234567]

  movdqa  [ecx+%1*16], xmm6

%endmacro

;//////////////////////////////////////////////////////////////////////
;// iLLM_PASS
;//////////////////////////////////////////////////////////////////////

%macro ADD_TO_DST 4   ; %1:src1   %2:dst1   %3:src2   %4:dst2
                      ; trashes xmm0,xmm4
  movdqu    xmm0, [%2]
  movdqu    xmm4, [%4]
  punpcklbw xmm0, xmm0
  psrlw     xmm0, 8   ; will zero the high words
  punpcklbw xmm4, xmm4
  psrlw     xmm4, 8
  paddsw    xmm0, %1
  paddsw    xmm4, %3
  packuswb  xmm0, xmm0
  packuswb  xmm4, xmm4
  movq      [%2], xmm0
  movq      [%4], xmm4
%endmacro

%macro iLLM_PASS 2  ; %1: src/dst   %2: combine func (0:store, 1:add,  2:put)

  movdqa xmm0, [tan3]     ; t3-1
  movdqa xmm3, [%1+16*3]  ; x3
  movdqa xmm1, xmm0       ; t3-1
  movdqa xmm5, [%1+16*5]  ; x5

  movdqa xmm4, [tan1]     ; t1
  movdqa xmm6, [%1+16*1]  ; x1
  movdqa xmm7, [%1+16*7]  ; x7
  movdqa xmm2, xmm4       ; t1

  pmulhw xmm0, xmm3       ; x3*(t3-1)
  pmulhw xmm1, xmm5       ; x5*(t3-1)
  paddsw xmm0, xmm3       ; x3*t3
  paddsw xmm1, xmm5       ; x5*t3
  psubsw xmm0, xmm5       ; x3*t3-x5 = tm35
  paddsw xmm1, xmm3       ; x3+x5*t3 = tp35

  pmulhw xmm4, xmm7       ; x7*t1
  pmulhw xmm2, xmm6       ; x1*t1
  paddsw xmm4, xmm6       ; x1+t1*x7 = tp17
  psubsw xmm2, xmm7       ; x1*t1-x7 = tm17


  movdqa xmm3, [sqrt2]
  movdqa xmm7, xmm4
  movdqa xmm6, xmm2
  psubsw xmm4, xmm1       ; tp17-tp35 = t1
  psubsw xmm2, xmm0       ; tm17-tm35 = b3
  paddsw xmm1, xmm7       ; tp17+tp35 = b0
  paddsw xmm0, xmm6       ; tm17+tm35 = t2

    ; xmm1 = b0, xmm2 = b3. preserved

  movdqa xmm6, xmm4
  psubsw xmm4, xmm0       ; t1-t2
  paddsw xmm0, xmm6       ; t1+t2

  pmulhw xmm4, xmm3       ; (t1-t2)/(2.sqrt2)
  pmulhw xmm0, xmm3       ; (t1+t2)/(2.sqrt2)

  paddsw xmm0, xmm0       ; 2.(t1+t2) = b1
  paddsw xmm4, xmm4       ; 2.(t1-t2) = b2

  movdqa xmm7, [tan2]     ; t2
  movdqa xmm3, [%1+2*16]  ; x2
  movdqa xmm6, [%1+6*16]  ; x6
  movdqa xmm5, xmm7       ; t2

  pmulhw xmm7, xmm6       ; x6*t2
  pmulhw xmm5, xmm3       ; x2*t2

  paddsw xmm7, xmm3       ; x2+x6*t2 = tp26
  psubsw xmm5, xmm6       ; x2*t2-x6 = tm26


   ; use:xmm3,xmm5,xmm6,xmm7   frozen: xmm0,xmm4,xmm1,xmm2

  movdqa xmm3, [%1+0*16] ; x0
  movdqa xmm6, [%1+4*16] ; x4

%ifndef IEEE_COMPLIANT

  psubsw xmm3, xmm6   ; x0-x4 = tm04
  paddsw xmm6, xmm6   ; 2.x4
  paddsw xmm6, xmm3   ; x0+x4 = tp04

  psubsw xmm3, xmm5   ; tm04-tm26 = a2
  psubsw xmm6, xmm7   ; tp04-tp26 = a3
  paddsw xmm5, xmm5   ; 2.tm26
  paddsw xmm7, xmm7   ; 2.tp26
  paddsw xmm5, xmm3   ; tm04+tm26 = a1
  paddsw xmm7, xmm6   ; tp04+tp26 = a0

  psubsw xmm5, xmm0   ; a1-b1
  psubsw xmm3, xmm4   ; a2-b2
  paddsw xmm0, xmm0   ; 2.b1
  paddsw xmm4, xmm4   ; 2.b2
  paddsw xmm0, xmm5   ; a1+b1
  paddsw xmm4, xmm3   ; a2+b2

%else

    ; we spill 1 reg to perform safe butterflies
  movdqa [%1   ], xmm2

  movdqa xmm2, xmm3
  psubsw xmm3, xmm6   ; x0-x4 = tm04
  paddsw xmm6, xmm2   ; x0+x4 = tp04

  movdqa xmm2, xmm6
  psubsw xmm6, xmm7
  paddsw xmm7, xmm2
  movdqa xmm2, xmm3
  psubsw xmm3, xmm5
  paddsw xmm5, xmm2

  movdqa xmm2, xmm5
  psubsw xmm5, xmm0
  paddsw xmm0, xmm2
  movdqa xmm2, xmm3
  psubsw xmm3, xmm4
  paddsw xmm4, xmm2

  movdqa xmm2, [%1]

%endif

  psraw  xmm5, 6      ; out6
  psraw  xmm3, 6      ; out5
  psraw  xmm0, 6      ; out1
  psraw  xmm4, 6      ; out2

%if (%2==0)
  movdqa [%1+6*16], xmm5
  movdqa [%1+5*16], xmm3
  movdqa [%1+1*16], xmm0
  movdqa [%1+2*16], xmm4
%elif (%2==2)
  movdqa [%1   ], xmm0   ; spill
  movdqa [%1+16], xmm4   ; spill
  ADD_TO_DST [%1], eax+  edx, [%1+16], eax+2*edx    ; #1 - #2
%else
  packuswb xmm0,xmm0
  packuswb xmm4,xmm4
  packuswb xmm3,xmm3
  packuswb xmm5,xmm5
  movq [eax+  edx], xmm0   ; #1
  movq [eax+2*edx], xmm4   ; #2
    ; keep xmm3 and xmm5 for later
%endif

    ; reminder: xmm1=b0, xmm2=b3, xmm7=a0, xmm6=a3

  movdqa xmm0, xmm7
  movdqa xmm4, xmm6
  psubsw xmm7, xmm1   ; a0-b0
  psubsw xmm6, xmm2   ; a3-b3
  paddsw xmm1, xmm0   ; a0+b0
  paddsw xmm2, xmm4   ; a3+b3

  psraw  xmm1, 6      ; out0
  psraw  xmm7, 6      ; out7
  psraw  xmm2, 6      ; out3
  psraw  xmm6, 6      ; out4

    ; combine result
%if (%2==0)
  movdqa [%1+0*16], xmm1
  movdqa [%1+3*16], xmm2
  movdqa [%1+4*16], xmm6
  movdqa [%1+7*16], xmm7
%elif (%2==2)
  ADD_TO_DST xmm1, eax,       xmm6, eax+4*edx  ; #0 - #4
  lea eax, [eax+2*edx]                         ; -> #2
  ADD_TO_DST xmm2, eax+  edx, xmm5, eax+4*edx  ; #3 - #6
  lea eax, [eax+  edx]                         ; -> #3
  ADD_TO_DST xmm3, eax+2*edx, xmm7, eax+4*edx  ; #5 - #7
%else
  packuswb xmm1,xmm1
  packuswb xmm2,xmm2
  packuswb xmm6,xmm6
  packuswb xmm7,xmm7
  movq [eax      ], xmm1   ; #0
  movq [eax+4*edx], xmm6   ; #4
  lea  eax, [eax+2*edx]    ; -> #2
  movq [eax+  edx], xmm2   ; #3
  movq [eax+4*edx], xmm5   ; #6
  lea  eax, [eax+  edx]    ; -> #3
  movq [eax+2*edx], xmm3   ; #5
  movq [eax+4*edx], xmm7   ; #7
%endif

%endmacro

;//////////////////////////////////////////////////////////////////////

align 16
Skl_IDct16_SSE2:
  mov ecx, [esp+4]
  iMTX_MULT  0, iTab1, Idct_Rnd0, 11
  iMTX_MULT  1, iTab2, Idct_Rnd1, 11
  iMTX_MULT  2, iTab3, Idct_Rnd2, 11
  iMTX_MULT  3, iTab4, Idct_Rnd3, 11
  iMTX_MULT  4, iTab1, Idct_Rnd4, 11
  iMTX_MULT  5, iTab4, Idct_Rnd5, 11
  iMTX_MULT  6, iTab3, Idct_Rnd6, 11
  iMTX_MULT  7, iTab2, Idct_Rnd7, 11
  iLLM_PASS ecx+0, 0
  ret

;//////////////////////////////////////////////////////////////////////
;// Sparseness-testing version

%macro TEST_ROW 2     ; %1:src,  %2:label x8
  mov eax, [%1   ]
  mov edx, [%1+ 8]
  or  eax, [%1+ 4]
  or  edx, [%1+12]
  or  eax, edx
  jz near %2
%endmacro

%macro IDCT_IMPL 1    ; %1: 0 = 16b store     1:put   2:add
  mov ecx, [esp+ 12]  ; Src

  TEST_ROW ecx, .Row0_Round
  iMTX_MULT  0, iTab1, Idct_Rnd0, 11
  jmp .Row1
.Row0_Round
  movdqa xmm0, [Idct_Sparse_Rnd0]
  movdqa [ecx  ], xmm0

.Row1
  TEST_ROW ecx+16, .Row1_Round
  iMTX_MULT  1, iTab2, Idct_Rnd1, 11
  jmp .Row2
.Row1_Round
  movdqa xmm0, [Idct_Sparse_Rnd1]
  movdqa [ecx+16  ], xmm0

.Row2
  TEST_ROW ecx+32, .Row2_Round
  iMTX_MULT  2, iTab3, Idct_Rnd2, 11
  jmp .Row3
.Row2_Round
  movdqa xmm0, [Idct_Sparse_Rnd2]
  movdqa [ecx+32  ], xmm0

.Row3
  TEST_ROW ecx+48, .Row4
  iMTX_MULT  3, iTab4, Idct_Rnd3, 11

.Row4
  TEST_ROW ecx+64, .Row5
  iMTX_MULT  4, iTab1, Idct_Rnd4, 11

.Row5
  TEST_ROW ecx+80, .Row6
  iMTX_MULT  5, iTab4, Idct_Rnd5, 11

.Row6
  TEST_ROW ecx+96, .Row7
  iMTX_MULT  6, iTab3, Idct_Rnd6, 11

.Row7
  TEST_ROW ecx+112, .End
  iMTX_MULT  7, iTab2, Idct_Rnd7, 11
.End

%if (%1!=0)
  mov eax, [esp+ 4]  ; Dst
  mov edx, [esp+ 8]  ; BpS
%endif

  iLLM_PASS ecx, %1

%endmacro

align 16
Skl_IDct16_Put_SSE2:
  IDCT_IMPL 1
  ret

align 16
Skl_IDct16_Add_SSE2:
  IDCT_IMPL 2
  ret
