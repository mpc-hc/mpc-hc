;/****************************************************************************
; *
; *  XVID MPEG-4 VIDEO CODEC
; *  - SSE2 forward discrete cosine transform -
; *
; *  Copyright(C) 2003 Pascal Massimino <skal@planet-d.net>
; *
; *  This program is free software; you can redistribute it and/or modify it
; *  under the terms of the GNU General Public License as published by
; *  the Free Software Foundation; either version 2 of the License, or
; *  (at your option) any later version.
; *
; *  This program is distributed in the hope that it will be useful,
; *  but WITHOUT ANY WARRANTY; without even the implied warranty of
; *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; *  GNU General Public License for more details.
; *
; *  You should have received a copy of the GNU General Public License
; *  along with this program; if not, write to the Free Software
; *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
; *
; ***************************************************************************/

%include "skl_nasm.h"

;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
;
;                          -=IDCT=-
;
; A little slower than fdct, because the final stages (butterflies and
; descaling) require some unpairable shifting and packing, all on
; the same CPU unit.
;
;-----------------------------------------------------------------------------

;=============================================================================
; Read only data
;=============================================================================

%ifdef FORMAT_COFF
SECTION .data data
%else
SECTION .data data align=16
%endif

ALIGN 16
tan1:    times 8 dw 0x32ec    ; tan( pi/16)
tan2:    times 8 dw 0x6a0a    ; tan(2pi/16)  (=sqrt(2)-1)
tan3:    times 8 dw 0xab0e    ; tan(3pi/16)-1
sqrt2:   times 8 dw 0x5a82    ; 0.5/sqrt(2)

;-----------------------------------------------------------------------------
; Inverse DCT tables
;-----------------------------------------------------------------------------

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

align 16
Walken_Idct_Rounders:
  dd  65536, 65536, 65536, 65536
  dd   3597,  3597,  3597,  3597
  dd   2260,  2260,  2260,  2260
  dd   1203,  1203,  1203,  1203
  dd      0,     0,     0,     0
  dd    120,   120,   120,   120
  dd    512,   512,   512,   512
  dd    512,   512,   512,   512

  times 8 dw  (65536>>11)
  times 8 dw  ( 3597>>11)
  times 8 dw  ( 2260>>11)
  ; other rounders are zero...

;-----------------------------------------------------------------------------
; Forward DCT tables
;-----------------------------------------------------------------------------

ALIGN 16
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


ALIGN 16
Fdct_Rnd0: dw  6,8,8,8, 6,8,8,8
Fdct_Rnd1: dw  8,8,8,8, 8,8,8,8
Fdct_Rnd2: dw 10,8,8,8, 8,8,8,8
Rounder1:  dw  1,1,1,1, 1,1,1,1

;=============================================================================
; Code
;=============================================================================

SECTION .text

globl idct_sse2_skal
globl fdct_sse2_skal

;-----------------------------------------------------------------------------
; Helper macro iMTX_MULT
;-----------------------------------------------------------------------------

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

;-----------------------------------------------------------------------------
; Helper macro iLLM_PASS
;-----------------------------------------------------------------------------

%macro iLLM_PASS 1  ; %1: src/dst

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

  movdqa [%1   ], xmm2  ; we spill 1 reg to perform safe butterflies

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

  psraw  xmm5, 6      ; out6
  psraw  xmm3, 6      ; out5
  psraw  xmm0, 6      ; out1
  psraw  xmm4, 6      ; out2

  movdqa [%1+6*16], xmm5
  movdqa [%1+5*16], xmm3
  movdqa [%1+1*16], xmm0
  movdqa [%1+2*16], xmm4

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

    ; store result

  movdqa [%1+0*16], xmm1
  movdqa [%1+3*16], xmm2
  movdqa [%1+4*16], xmm6
  movdqa [%1+7*16], xmm7

%endmacro

;-----------------------------------------------------------------------------
; Helper macro TEST_ROW (test a null row)
;-----------------------------------------------------------------------------

%macro TEST_ROW 2     ; %1:src,  %2:label x8
  mov eax, [%1   ]
  mov edx, [%1+ 8]
  or  eax, [%1+ 4]
  or  edx, [%1+12]
  or  eax, edx
  jz near %2
%endmacro

;-----------------------------------------------------------------------------
; Function idct (this one skips null rows)
;-----------------------------------------------------------------------------
; IEEE1180 and Walken compatible version

align 16
idct_sse2_skal:

  mov ecx, [esp+ 4]  ; Src

  TEST_ROW ecx, .Row0_Round
  iMTX_MULT  0, iTab1, Walken_Idct_Rounders + 16*0, 11
  jmp .Row1
.Row0_Round
  movdqa xmm0, [Walken_Idct_Rounders + 16*8 + 8*0]
  movdqa [ecx  ], xmm0

.Row1
  TEST_ROW ecx+16, .Row1_Round
  iMTX_MULT  1, iTab2, Walken_Idct_Rounders + 16*1, 11
  jmp .Row2
.Row1_Round
  movdqa xmm0, [Walken_Idct_Rounders + 16*8 + 16*1]
  movdqa [ecx+16  ], xmm0

.Row2
  TEST_ROW ecx+32, .Row2_Round
  iMTX_MULT  2, iTab3, Walken_Idct_Rounders + 16*2, 11
  jmp .Row3
.Row2_Round
  movdqa xmm0, [Walken_Idct_Rounders + 16*8 + 16*2]
  movdqa [ecx+32  ], xmm0

.Row3
  TEST_ROW ecx+48, .Row4
  iMTX_MULT  3, iTab4, Walken_Idct_Rounders + 16*3, 11

.Row4
  TEST_ROW ecx+64, .Row5
  iMTX_MULT  4, iTab1, Walken_Idct_Rounders + 16*4, 11

.Row5
  TEST_ROW ecx+80, .Row6
  iMTX_MULT  5, iTab4, Walken_Idct_Rounders + 16*5, 11

.Row6
  TEST_ROW ecx+96, .Row7
  iMTX_MULT  6, iTab3, Walken_Idct_Rounders + 16*6, 11

.Row7
  TEST_ROW ecx+112, .End
  iMTX_MULT  7, iTab2, Walken_Idct_Rounders + 16*7, 11
.End

  iLLM_PASS ecx

  ret
.endfunc

;-----------------------------------------------------------------------------
; Helper macro fLLM_PASS
;-----------------------------------------------------------------------------

%macro fLLM_PASS 2  ; %1: src/dst, %2:Shift

  movdqa xmm0, [%1+0*16]   ; In0
  movdqa xmm2, [%1+2*16]   ; In2
  movdqa xmm3, xmm0
  movdqa xmm4, xmm2
  movdqa xmm7, [%1+7*16]   ; In7
  movdqa xmm5, [%1+5*16]   ; In5

  psubsw xmm0, xmm7         ; t7 = In0-In7
  paddsw xmm7, xmm3         ; t0 = In0+In7
  psubsw xmm2, xmm5         ; t5 = In2-In5
  paddsw xmm5, xmm4         ; t2 = In2+In5

  movdqa xmm3, [%1+3*16]   ; In3
  movdqa xmm4, [%1+4*16]   ; In4
  movdqa xmm1, xmm3
  psubsw xmm3, xmm4         ; t4 = In3-In4
  paddsw xmm4, xmm1         ; t3 = In3+In4
  movdqa xmm6, [%1+6*16]   ; In6
  movdqa xmm1, [%1+1*16]   ; In1
  psubsw xmm1, xmm6         ; t6 = In1-In6
  paddsw xmm6, [%1+1*16]   ; t1 = In1+In6

  psubsw xmm7, xmm4         ; tm03 = t0-t3
  psubsw xmm6, xmm5         ; tm12 = t1-t2
  paddsw xmm4, xmm4         ; 2.t3
  paddsw xmm5, xmm5         ; 2.t2
  paddsw xmm4, xmm7         ; tp03 = t0+t3
  paddsw xmm5, xmm6         ; tp12 = t1+t2

  psllw  xmm2, %2+1        ; shift t5 (shift +1 to..
  psllw  xmm1, %2+1        ; shift t6  ..compensate cos4/2)
  psllw  xmm4, %2          ; shift t3
  psllw  xmm5, %2          ; shift t2
  psllw  xmm7, %2          ; shift t0
  psllw  xmm6, %2          ; shift t1
  psllw  xmm3, %2          ; shift t4
  psllw  xmm0, %2          ; shift t7

  psubsw xmm4, xmm5         ; out4 = tp03-tp12
  psubsw xmm1, xmm2         ; xmm1: t6-t5
  paddsw xmm5, xmm5
  paddsw xmm2, xmm2
  paddsw xmm5, xmm4         ; out0 = tp03+tp12
  movdqa [%1+4*16], xmm4   ; => out4
  paddsw xmm2, xmm1         ; xmm2: t6+t5
  movdqa [%1+0*16], xmm5   ; => out0

  movdqa xmm4, [tan2]      ; xmm4 <= tan2
  pmulhw xmm4, xmm7         ; tm03*tan2
  movdqa xmm5, [tan2]      ; xmm5 <= tan2
  psubsw xmm4, xmm6         ; out6 = tm03*tan2 - tm12
  pmulhw xmm5, xmm6         ; tm12*tan2
  paddsw xmm5, xmm7         ; out2 = tm12*tan2 + tm03

  movdqa xmm6, [sqrt2]
  movdqa xmm7, [Rounder1]

  pmulhw xmm2, xmm6         ; xmm2: tp65 = (t6 + t5)*cos4
  por    xmm5, xmm7         ; correct out2
  por    xmm4, xmm7         ; correct out6
  pmulhw xmm1, xmm6         ; xmm1: tm65 = (t6 - t5)*cos4
  por    xmm2, xmm7         ; correct tp65

  movdqa [%1+2*16], xmm5   ; => out2
  movdqa xmm5, xmm3         ; save t4
  movdqa [%1+6*16], xmm4   ; => out6
  movdqa xmm4, xmm0         ; save t7

  psubsw xmm3, xmm1         ; xmm3: tm465 = t4 - tm65
  psubsw xmm0, xmm2         ; xmm0: tm765 = t7 - tp65
  paddsw xmm2, xmm4         ; xmm2: tp765 = t7 + tp65
  paddsw xmm1, xmm5         ; xmm1: tp465 = t4 + tm65

  movdqa xmm4, [tan3]      ; tan3 - 1
  movdqa xmm5, [tan1]      ; tan1

  movdqa xmm7, xmm3         ; save tm465
  pmulhw xmm3, xmm4         ; tm465*(tan3-1)
  movdqa xmm6, xmm1         ; save tp465
  pmulhw xmm1, xmm5         ; tp465*tan1

  paddsw xmm3, xmm7         ; tm465*tan3
  pmulhw xmm4, xmm0         ; tm765*(tan3-1)
  paddsw xmm4, xmm0         ; tm765*tan3
  pmulhw xmm5, xmm2         ; tp765*tan1

  paddsw xmm1, xmm2         ; out1 = tp765 + tp465*tan1
  psubsw xmm0, xmm3         ; out3 = tm765 - tm465*tan3
  paddsw xmm7, xmm4         ; out5 = tm465 + tm765*tan3
  psubsw xmm5, xmm6         ; out7 =-tp465 + tp765*tan1

  movdqa [%1+1*16], xmm1   ; => out1
  movdqa [%1+3*16], xmm0   ; => out3
  movdqa [%1+5*16], xmm7   ; => out5
  movdqa [%1+7*16], xmm5   ; => out7

%endmacro

;-----------------------------------------------------------------------------
;Helper macro fMTX_MULT
;-----------------------------------------------------------------------------

%macro fMTX_MULT 3   ; %1=src, %2 = Coeffs, %3=rounders

  movdqa   xmm0, [ecx+%1*16+0]   ; xmm0 = [0123][4567]
  pshufhw  xmm1, xmm0, 00011011b ; xmm1 = [----][7654]
  pshufd   xmm0, xmm0, 01000100b
  pshufd   xmm1, xmm1, 11101110b

  movdqa   xmm2, xmm0
  paddsw  xmm0, xmm1              ; xmm0 = [a0 a1 a2 a3]
  psubsw  xmm2, xmm1              ; xmm2 = [b0 b1 b2 b3]

  punpckldq xmm0, xmm2            ; xmm0 = [a0 a1 b0 b1][a2 a3 b2 b3]
  pshufd    xmm2, xmm0, 01001110b ; xmm2 = [a2 a3 b2 b3][a0 a1 b0 b1]

    ;  [M00 M01    M16 M17] [M06 M07    M22 M23]  x mm0 = [0 /1 /2'/3']
    ;  [M02 M03    M18 M19] [M04 M05    M20 M21]  x mm2 = [0'/1'/2 /3 ]
    ;  [M08 M09    M24 M25] [M14 M15    M30 M31]  x mm0 = [4 /5 /6'/7']
    ;  [M10 M11    M26 M27] [M12 M13    M28 M29]  x mm2 = [4'/5'/6 /7 ]

  movdqa  xmm1, [%2+16]
  movdqa  xmm3, [%2+32]
  pmaddwd xmm1, xmm2
  pmaddwd xmm3, xmm0
  pmaddwd xmm2, [%2+48]
  pmaddwd xmm0, [%2+ 0]

  paddd   xmm0, xmm1             ;  [ out0 | out1 ][ out2 | out3 ]
  paddd   xmm2, xmm3             ;  [ out4 | out5 ][ out6 | out7 ]
  psrad   xmm0, 16
  psrad   xmm2, 16

  packssdw xmm0, xmm2            ;  [ out0 .. out7 ]
  paddsw   xmm0, [%3]            ;  Round

  psraw    xmm0, 4               ; => [-2048, 2047]

  movdqa  [ecx+%1*16+0], xmm0
%endmacro

;-----------------------------------------------------------------------------
; Function Forward DCT
;-----------------------------------------------------------------------------

ALIGN 16
fdct_sse2_skal:
  mov ecx, [esp+4]
  fLLM_PASS ecx+0, 3
  fMTX_MULT  0, fTab1, Fdct_Rnd0
  fMTX_MULT  1, fTab2, Fdct_Rnd2
  fMTX_MULT  2, fTab3, Fdct_Rnd1
  fMTX_MULT  3, fTab4, Fdct_Rnd1
  fMTX_MULT  4, fTab1, Fdct_Rnd0
  fMTX_MULT  5, fTab4, Fdct_Rnd1
  fMTX_MULT  6, fTab3, Fdct_Rnd1
  fMTX_MULT  7, fTab2, Fdct_Rnd1
  ret
.endfunc





