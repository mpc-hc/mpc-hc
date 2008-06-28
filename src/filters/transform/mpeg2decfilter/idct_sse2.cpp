#include "stdafx.h"
#include "libmpeg2.h"

// Intel's SSE2 implementation of iDCT
// AP-945
// http://cache-www.intel.com/cd/00/00/01/76/17680_w_idct.pdf

#define BITS_INV_ACC 4 // 4 or 5 for IEEE
#define SHIFT_INV_ROW 16 - BITS_INV_ACC
#define SHIFT_INV_COL 1 + BITS_INV_ACC
const short RND_INV_ROW = 1024 * (6 - BITS_INV_ACC); //1 << (SHIFT_INV_ROW-1)
const short RND_INV_COL = 16 * (BITS_INV_ACC - 3); // 1 << (SHIFT_INV_COL-1)
const short RND_INV_CORR = RND_INV_COL - 1; // correction -1.0 and round

__declspec(align(16)) short M128_one_corr[8] = {1,1,1,1,1,1,1,1};
__declspec(align(16)) short M128_round_inv_row[8] = {RND_INV_ROW, 0, RND_INV_ROW, 0, RND_INV_ROW, 0, RND_INV_ROW, 0};
__declspec(align(16)) short M128_round_inv_col[8] = {RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL};
__declspec(align(16)) short M128_round_inv_corr[8]= {RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR};
__declspec(align(16)) short M128_tg_1_16[8] = {13036, 13036, 13036, 13036, 13036, 13036, 13036, 13036}; // tg * (2<<16) + 0.5
__declspec(align(16)) short M128_tg_2_16[8] = {27146, 27146, 27146, 27146, 27146, 27146, 27146, 27146}; // tg * (2<<16) + 0.5
__declspec(align(16)) short M128_tg_3_16[8] = {-21746, -21746, -21746, -21746, -21746, -21746, -21746, -21746}; // tg * (2<<16) + 0.5
__declspec(align(16)) short M128_cos_4_16[8] = {-19195, -19195, -19195, -19195, -19195, -19195, -19195, -19195};// cos * (2<<16) + 0.5

//-----------------------------------------------------------------------------
// Table for rows 0,4 - constants are multiplied on cos_4_16
//movq -> w13 w12 w09 w08 w05 w04 w01 w00
// w15 w14 w11 w10 w07 w06 w03 w02
// w29 w28 w25 w24 w21 w20 w17 w16
// w31 w30 w27 w26 w23 w22 w19 w18

__declspec(align(16)) short M128_tab_i_04[] = 
{
	16384, 21407, 16384, 8867, //movq -> w05 w04 w01 w00
	16384, -8867, 16384, -21407, // w13 w12 w09 w08
	16384, 8867, -16384, -21407, // w07 w06 w03 w02
	-16384, 21407, 16384, -8867, // w15 w14 w11 w10
	22725, 19266, 19266, -4520, // w21 w20 w17 w16
	12873, -22725, 4520, -12873, // w29 w28 w25 w24
	12873, 4520, -22725, -12873, // w23 w22 w19 w18
	4520, 19266, 19266, -22725  // w31 w30 w27 w26
};

// Table for rows 1,7 - constants are multiplied on cos_1_16

__declspec(align(16)) short M128_tab_i_17[] =
{
	22725, 29692, 22725, 12299, //movq -> w05 w04 w01 w00
	22725, -12299, 22725, -29692, // w13 w12 w09 w08
	22725, 12299, -22725, -29692, // w07 w06 w03 w02
	-22725, 29692, 22725, -12299, // w15 w14 w11 w10
	31521, 26722, 26722, -6270, // w21 w20 w17 w16
	17855, -31521, 6270, -17855, // w29 w28 w25 w24
	17855, 6270, -31521, -17855, // w23 w22 w19 w18
	6270, 26722, 26722, -31521 // w31 w30 w27 w26
};

// Table for rows 2,6 - constants are multiplied on cos_2_16

__declspec(align(16)) short M128_tab_i_26[] =
{
	21407, 27969, 21407, 11585, //movq -> w05 w04 w01 w00
	21407, -11585, 21407, -27969, // w13 w12 w09 w08
	21407, 11585, -21407, -27969, // w07 w06 w03 w02
	-21407, 27969, 21407, -11585, // w15 w14 w11 w10
	29692, 25172, 25172, -5906, // w21 w20 w17 w16
	16819, -29692, 5906, -16819, // w29 w28 w25 w24
	16819, 5906, -29692, -16819, // w23 w22 w19 w18
	5906, 25172, 25172, -29692 // w31 w30 w27 w26
};

// Table for rows 3,5 - constants are multiplied on cos_3_16

__declspec(align(16)) short M128_tab_i_35[] = 
{
	19266, 25172, 19266, 10426, //movq -> w05 w04 w01 w00
	19266, -10426, 19266, -25172, // w13 w12 w09 w08
	19266, 10426, -19266, -25172, // w07 w06 w03 w02
	-19266, 25172, 19266, -10426, // w15 w14 w11 w10
	26722, 22654, 22654, -5315, // w21 w20 w17 w16
	15137, -26722, 5315, -15137, // w29 w28 w25 w24
	15137, 5315, -26722, -15137, // w23 w22 w19 w18
	5315, 22654, 22654, -26722 // w31 w30 w27 w26
};

//-----------------------------------------------------------------------------
/*
;=============================================================================
;=============================================================================
;=============================================================================
;
; Inverse DCT
;
;-----------------------------------------------------------------------------
;
; This implementation calculates iDCT-2D by a row-column method.
; On the first stage the iDCT-1D is calculated for each row with use
; direct algorithm, on the second stage the calculation is executed
; at once for four columns with use of scaled iDCT-1D algorithm.
; Base R&Y algorithm for iDCT-1D is modified for second stage.
;
;=============================================================================
;-----------------------------------------------------------------------------
;
; The first stage - inverse DCTs of rows
;
;-----------------------------------------------------------------------------
; The 8-point inverse DCT direct algorithm
;-----------------------------------------------------------------------------
;
; static const short w[32] = {
; FIX(cos_4_16), FIX(cos_2_16), FIX(cos_4_16), FIX(cos_6_16),
; FIX(cos_4_16), FIX(cos_6_16), -FIX(cos_4_16), -FIX(cos_2_16),
; FIX(cos_4_16), -FIX(cos_6_16), -FIX(cos_4_16), FIX(cos_2_16),
; FIX(cos_4_16), -FIX(cos_2_16), FIX(cos_4_16), -FIX(cos_6_16),
; FIX(cos_1_16), FIX(cos_3_16), FIX(cos_5_16), FIX(cos_7_16),
; FIX(cos_3_16), -FIX(cos_7_16), -FIX(cos_1_16), -FIX(cos_5_16),
; FIX(cos_5_16), -FIX(cos_1_16), FIX(cos_7_16), FIX(cos_3_16),
; FIX(cos_7_16), -FIX(cos_5_16), FIX(cos_3_16), -FIX(cos_1_16) };
;
; #define DCT_8_INV_ROW(x, y)
; {
; int a0, a1, a2, a3, b0, b1, b2, b3;
;
; a0 = x[0] * w[ 0] + x[2] * w[ 1] + x[4] * w[ 2] + x[6] * w[ 3];
; a1 = x[0] * w[ 4] + x[2] * w[ 5] + x[4] * w[ 6] + x[6] * w[ 7];
; a2 = x[0] * w[ 8] + x[2] * w[ 9] + x[4] * w[10] + x[6] * w[11];
; a3 = x[0] * w[12] + x[2] * w[13] + x[4] * w[14] + x[6] * w[15];
; b0 = x[1] * w[16] + x[3] * w[17] + x[5] * w[18] + x[7] * w[19];
; b1 = x[1] * w[20] + x[3] * w[21] + x[5] * w[22] + x[7] * w[23];
; b2 = x[1] * w[24] + x[3] * w[25] + x[5] * w[26] + x[7] * w[27];
; b3 = x[1] * w[28] + x[3] * w[29] + x[5] * w[30] + x[7] * w[31];
;
; y[0] = SHIFT_ROUND ( a0 + b0 );
; y[1] = SHIFT_ROUND ( a1 + b1 );
; y[2] = SHIFT_ROUND ( a2 + b2 );
; y[3] = SHIFT_ROUND ( a3 + b3 );
; y[4] = SHIFT_ROUND ( a3 - b3 );
; y[5] = SHIFT_ROUND ( a2 - b2 );
; y[6] = SHIFT_ROUND ( a1 - b1 );
; y[7] = SHIFT_ROUND ( a0 - b0 );
; }
;
;-----------------------------------------------------------------------------
;
; In this implementation the outputs of the iDCT-1D are multiplied
; for rows 0,4 - on cos_4_16,
; for rows 1,7 - on cos_1_16,
; for rows 2,6 - on cos_2_16,
; for rows 3,5 - on cos_3_16
; and are shifted to the left for rise of accuracy
;
; For used constants
; FIX(float_const) = (short) (float_const * (1<<15) + 0.5)
;
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
;
; The second stage - inverse DCTs of columns
;
; The inputs are multiplied
; for rows 0,4 - on cos_4_16,
; for rows 1,7 - on cos_1_16,
; for rows 2,6 - on cos_2_16,
; for rows 3,5 - on cos_3_16
; and are shifted to the left for rise of accuracy
;
;-----------------------------------------------------------------------------
;
; The 8-point scaled inverse DCT algorithm (26a8m)
;
;-----------------------------------------------------------------------------
;
; #define DCT_8_INV_COL(x, y)
; {
; short t0, t1, t2, t3, t4, t5, t6, t7;
; short tp03, tm03, tp12, tm12, tp65, tm65;
; short tp465, tm465, tp765, tm765;
;
; tp765 = x[1] + x[7] * tg_1_16;
; tp465 = x[1] * tg_1_16 - x[7];
; tm765 = x[5] * tg_3_16 + x[3];
; tm465 = x[5] - x[3] * tg_3_16;
;
; t7 = tp765 + tm765;
; tp65 = tp765 - tm765;
; t4 = tp465 + tm465;
; tm65 = tp465 - tm465;
;
; t6 = ( tp65 + tm65 ) * cos_4_16;
; t5 = ( tp65 - tm65 ) * cos_4_16;
;
; tp03 = x[0] + x[4];
; tp12 = x[0] - x[4];
;
; tm03 = x[2] + x[6] * tg_2_16;
; tm12 = x[2] * tg_2_16 - x[6];
;
; t0 = tp03 + tm03;
; t3 = tp03 - tm03;
; t1 = tp12 + tm12;
; t2 = tp12 - tm12;
;
; y[0] = SHIFT_ROUND ( t0 + t7 );
; y[7] = SHIFT_ROUND ( t0 - t7 );
; y[1] = SHIFT_ROUND ( t1 + t6 );
; y[6] = SHIFT_ROUND ( t1 - t6 );
; y[2] = SHIFT_ROUND ( t2 + t5 );
; y[5] = SHIFT_ROUND ( t2 - t5 );
; y[3] = SHIFT_ROUND ( t3 + t4 );
; y[4] = SHIFT_ROUND ( t3 - t4 );
; }
;
;-----------------------------------------------------------------------------
*/
//xmm7 = round_inv_row
#define DCT_8_INV_ROW __asm{ \
	__asm pshuflw xmm0, xmm0, 0xD8 \
	__asm pshufd xmm1, xmm0, 0 \
	__asm pmaddwd xmm1, [esi] \
	__asm pshufd xmm3, xmm0, 0x55 \
	__asm pshufhw xmm0, xmm0, 0xD8 \
	__asm pmaddwd xmm3, [esi+32] \
	__asm pshufd xmm2, xmm0, 0xAA \
	__asm pshufd xmm0, xmm0, 0xFF \
	__asm pmaddwd xmm2, [esi+16] \
	__asm pshufhw xmm4, xmm4, 0xD8 \
	__asm paddd xmm1, M128_round_inv_row \
	__asm pshuflw xmm4, xmm4, 0xD8 \
	__asm pmaddwd xmm0, [esi+48] \
	__asm pshufd xmm5, xmm4, 0 \
	__asm pshufd xmm6, xmm4, 0xAA \
	__asm pmaddwd xmm5, [ecx] \
	__asm paddd xmm1, xmm2 \
	__asm movdqa xmm2, xmm1 \
	__asm pshufd xmm7, xmm4, 0x55 \
	__asm pmaddwd xmm6, [ecx+16] \
	__asm paddd xmm0, xmm3 \
	__asm pshufd xmm4, xmm4, 0xFF \
	__asm psubd xmm2, xmm0 \
	__asm pmaddwd xmm7, [ecx+32] \
	__asm paddd xmm0, xmm1 \
	__asm psrad xmm2, 12 \
	__asm paddd xmm5, M128_round_inv_row \
	__asm pmaddwd xmm4, [ecx+48] \
	__asm paddd xmm5, xmm6 \
	__asm movdqa xmm6, xmm5 \
	__asm psrad xmm0, 12 \
	__asm pshufd xmm2, xmm2, 0x1B \
	__asm packssdw xmm0, xmm2 \
	__asm paddd xmm4, xmm7 \
	__asm psubd xmm6, xmm4 \
	__asm paddd xmm4, xmm5 \
	__asm psrad xmm6, 12 \
	__asm psrad xmm4, 12 \
	__asm pshufd xmm6, xmm6, 0x1B \
	__asm packssdw xmm4, xmm6 \
}
#define DCT_8_INV_COL_8 __asm{ \
	__asm movdqa xmm1, XMMWORD PTR M128_tg_3_16 \
	__asm movdqa xmm2, xmm0 \
	__asm movdqa xmm3, XMMWORD PTR [edx+3*16] \
	__asm pmulhw xmm0, xmm1 \
	__asm pmulhw xmm1, xmm3 \
	__asm movdqa xmm5, XMMWORD PTR M128_tg_1_16 \
	__asm movdqa xmm6, xmm4 \
	__asm pmulhw xmm4, xmm5 \
	__asm paddsw xmm0, xmm2 \
	__asm pmulhw xmm5, [edx+1*16] \
	__asm paddsw xmm1, xmm3 \
	__asm movdqa xmm7, XMMWORD PTR [edx+6*16] \
	__asm paddsw xmm0, xmm3 \
	__asm movdqa xmm3, XMMWORD PTR M128_tg_2_16 \
	__asm psubsw xmm2, xmm1 \
	__asm pmulhw xmm7, xmm3 \
	__asm movdqa xmm1, xmm0 \
	__asm pmulhw xmm3, [edx+2*16] \
	__asm psubsw xmm5, xmm6 \
	__asm paddsw xmm4, [edx+1*16] \
	__asm paddsw xmm0, xmm4 \
	__asm paddsw xmm0, XMMWORD PTR M128_one_corr \
	__asm psubsw xmm4, xmm1 \
	__asm movdqa xmm6, xmm5 \
	__asm psubsw xmm5, xmm2 \
	__asm paddsw xmm5, XMMWORD PTR M128_one_corr \
	__asm paddsw xmm6, xmm2 \
	__asm movdqa [edx+7*16], xmm0 \
	__asm movdqa xmm1, xmm4 \
	__asm movdqa xmm0, XMMWORD PTR M128_cos_4_16 \
	__asm paddsw xmm4, xmm5 \
	__asm movdqa xmm2, XMMWORD PTR M128_cos_4_16 \
	__asm pmulhw xmm2, xmm4 \
	__asm movdqa [edx+3*16], xmm6 \
	__asm psubsw xmm1, xmm5 \
	__asm paddsw xmm7, [edx+2*16] \
	__asm psubsw xmm3, [edx+6*16] \
	__asm movdqa xmm6, [edx] \
	__asm pmulhw xmm0, xmm1 \
	__asm movdqa xmm5, [edx+4*16] \
	__asm paddsw xmm5, xmm6 \
	__asm psubsw xmm6, [edx+4*16] \
	__asm paddsw xmm4, xmm2 \
	__asm por xmm4, XMMWORD PTR M128_one_corr \
	__asm paddsw xmm0, xmm1 \
	__asm por xmm0, XMMWORD PTR M128_one_corr \
	__asm movdqa xmm2, xmm5 \
	__asm paddsw xmm5, xmm7 \
	__asm movdqa xmm1, xmm6 \
	__asm paddsw xmm5, XMMWORD PTR M128_round_inv_col \
	__asm psubsw xmm2, xmm7 \
	__asm movdqa xmm7, [edx+7*16] \
	__asm paddsw xmm6, xmm3 \
	__asm paddsw xmm6, XMMWORD PTR M128_round_inv_col \
	__asm paddsw xmm7, xmm5 \
	__asm psraw xmm7, SHIFT_INV_COL \
	__asm psubsw xmm1, xmm3 \
	__asm paddsw xmm1, XMMWORD PTR M128_round_inv_corr \
	__asm movdqa xmm3, xmm6 \
	__asm paddsw xmm2, XMMWORD PTR M128_round_inv_corr \
	__asm paddsw xmm6, xmm4 \
	__asm movdqa [edx], xmm7 \
	__asm psraw xmm6, SHIFT_INV_COL \
	__asm movdqa xmm7, xmm1 \
	__asm paddsw xmm1, xmm0 \
	__asm movdqa [edx+1*16], xmm6 \
	__asm psraw xmm1, SHIFT_INV_COL \
	__asm movdqa xmm6, [edx+3*16] \
	__asm psubsw xmm7, xmm0 \
	__asm psraw xmm7, SHIFT_INV_COL \
	__asm movdqa [edx+2*16], xmm1 \
	__asm psubsw xmm5, [edx+7*16] \
	__asm psraw xmm5, SHIFT_INV_COL \
	__asm movdqa [edx+7*16], xmm5 \
	__asm psubsw xmm3, xmm4 \
	__asm paddsw xmm6, xmm2 \
	__asm psubsw xmm2, [edx+3*16] \
	__asm psraw xmm6, SHIFT_INV_COL \
	__asm psraw xmm2, SHIFT_INV_COL \
	__asm movdqa [edx+3*16], xmm6 \
	__asm psraw xmm3, SHIFT_INV_COL \
	__asm movdqa [edx+4*16], xmm2 \
	__asm movdqa [edx+5*16], xmm7 \
	__asm movdqa [edx+6*16], xmm3 \
}

//assumes src and destination are aligned on a 16-byte boundary

static void idct_M128ASM(short* src) 
{
	ASSERT(((DWORD)src & 0xf) == 0); //aligned on 16-byte boundary

	__asm mov edx, src

	__asm movdqa xmm0, XMMWORD PTR[edx] //row 1
	__asm lea esi, M128_tab_i_04
	__asm movdqa xmm4, XMMWORD PTR[edx+16*2] //row 3
	__asm lea ecx, M128_tab_i_26
	DCT_8_INV_ROW; //Row 1, tab_i_04 and Row 3, tab_i_26
	__asm movdqa XMMWORD PTR[edx], xmm0
	__asm movdqa XMMWORD PTR[edx+16*2], xmm4

	__asm movdqa xmm0, XMMWORD PTR[edx+16*4] //row 5
	//__asm lea esi, M128_tab_i_04
	__asm movdqa xmm4, XMMWORD PTR[edx+16*6] //row 7
	//__asm lea ecx, M128_tab_i_26
	DCT_8_INV_ROW; //Row 5, tab_i_04 and Row 7, tab_i_26
	__asm movdqa XMMWORD PTR[edx+16*4], xmm0
	__asm movdqa XMMWORD PTR[edx+16*6], xmm4

	__asm movdqa xmm0, XMMWORD PTR[edx+16*3] //row 4
	__asm lea esi, M128_tab_i_35
	__asm movdqa xmm4, XMMWORD PTR[edx+16*1] //row 2
	__asm lea ecx, M128_tab_i_17
	DCT_8_INV_ROW; //Row 4, tab_i_35 and Row 2, tab_i_17
	__asm movdqa XMMWORD PTR[edx+16*3], xmm0
	__asm movdqa XMMWORD PTR[edx+16*1], xmm4

	__asm movdqa xmm0, XMMWORD PTR[edx+16*5] //row 6
	//__asm lea esi, M128_tab_i_35
	__asm movdqa xmm4, XMMWORD PTR[edx+16*7] //row 8
	//__asm lea ecx, M128_tab_i_17
	DCT_8_INV_ROW; //Row 6, tab_i_35 and Row 8, tab_i_17
	//__asm movdqa XMMWORD PTR[edx+16*5], xmm0
	//__asm movdqa xmm0, XMMWORD PTR [edx+16*5] /* 0 /* x5 */
	//__asm movdqa XMMWORD PTR[edx+16*7], xmm4
	//__asm movdqa xmm4, XMMWORD PTR [edx+7*16]/* 4 ; x7 */
	DCT_8_INV_COL_8
	// __asm emms
}

/////////////

#define CLIP(x) (x < 0 ? 0 : x > 255 ? 255 : x)

#include <xmmintrin.h>
#include <emmintrin.h>

void mpeg2_idct_copy_sse2(int16_t* block, uint8_t* dest, const int stride)
{
	idct_M128ASM(block);
/*
    for(int i = 0; i < 8; i++)
	{
		dest[0] = CLIP(block[0]);
		dest[1] = CLIP(block[1]);
		dest[2] = CLIP(block[2]);
		dest[3] = CLIP(block[3]);
		dest[4] = CLIP(block[4]);
		dest[5] = CLIP(block[5]);
		dest[6] = CLIP(block[6]);
		dest[7] = CLIP(block[7]);

		memset(block, 0, sizeof(short)*8);

		dest += stride;
		block += 8;
    }
*/
	__m128i* src = (__m128i*)block;
	__m128i zero = _mm_setzero_si128();

	__m128i r0 = _mm_packus_epi16(_mm_load_si128(&src[0]), _mm_load_si128(&src[1]));
	__m128i r1 = _mm_packus_epi16(_mm_load_si128(&src[2]), _mm_load_si128(&src[3]));
	__m128i r2 = _mm_packus_epi16(_mm_load_si128(&src[4]), _mm_load_si128(&src[5]));
	__m128i r3 = _mm_packus_epi16(_mm_load_si128(&src[6]), _mm_load_si128(&src[7]));

	_mm_storel_pi((__m64*)&dest[0*stride], *(__m128*)&r0);
	_mm_storeh_pi((__m64*)&dest[1*stride], *(__m128*)&r0);
	_mm_storel_pi((__m64*)&dest[2*stride], *(__m128*)&r1);
	_mm_storeh_pi((__m64*)&dest[3*stride], *(__m128*)&r1);
	_mm_storel_pi((__m64*)&dest[4*stride], *(__m128*)&r2);
	_mm_storeh_pi((__m64*)&dest[5*stride], *(__m128*)&r2);
	_mm_storel_pi((__m64*)&dest[6*stride], *(__m128*)&r3);
	_mm_storeh_pi((__m64*)&dest[7*stride], *(__m128*)&r3);

	_mm_store_si128(&src[0], zero);
	_mm_store_si128(&src[1], zero);
	_mm_store_si128(&src[2], zero);
	_mm_store_si128(&src[3], zero);
	_mm_store_si128(&src[4], zero);
	_mm_store_si128(&src[5], zero);
	_mm_store_si128(&src[6], zero);
	_mm_store_si128(&src[7], zero);
}

void mpeg2_idct_add_sse2(const int last, int16_t* block, uint8_t* dest, const int stride)
{
	idct_M128ASM(block);

	/*
	for(int i = 0; i < 8; i++)
	{
		dest[0] = CLIP(block[0] + dest[0]);
		dest[1] = CLIP(block[1] + dest[1]);
		dest[2] = CLIP(block[2] + dest[2]);
		dest[3] = CLIP(block[3] + dest[3]);
		dest[4] = CLIP(block[4] + dest[4]);
		dest[5] = CLIP(block[5] + dest[5]);
		dest[6] = CLIP(block[6] + dest[6]);
		dest[7] = CLIP(block[7] + dest[7]);

		memset(block, 0, sizeof(short)*8);

		dest += stride;
		block += 8;
	}
	*/

	__m128i* src = (__m128i*)block;
	__m128i zero = _mm_setzero_si128();

	__m128i r0 = _mm_load_si128(&src[0]);
	__m128i r1 = _mm_load_si128(&src[1]);
	__m128i r2 = _mm_load_si128(&src[2]);
	__m128i r3 = _mm_load_si128(&src[3]);
	__m128i r4 = _mm_load_si128(&src[4]);
	__m128i r5 = _mm_load_si128(&src[5]);
	__m128i r6 = _mm_load_si128(&src[6]);
	__m128i r7 = _mm_load_si128(&src[7]);

	__m128 q0 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[0*stride]);
	__m128 q1 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[1*stride]);
	__m128 q2 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[2*stride]);
	__m128 q3 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[3*stride]);
	__m128 q4 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[4*stride]);
	__m128 q5 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[5*stride]);
	__m128 q6 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[6*stride]);
	__m128 q7 = _mm_loadl_pi(*(__m128*)&zero, (__m64*)&dest[7*stride]);

	r0 = _mm_adds_epi16(r0, _mm_unpacklo_epi8(*(__m128i*)&q0, zero));
	r1 = _mm_adds_epi16(r1, _mm_unpacklo_epi8(*(__m128i*)&q1, zero));
	r2 = _mm_adds_epi16(r2, _mm_unpacklo_epi8(*(__m128i*)&q2, zero));
	r3 = _mm_adds_epi16(r3, _mm_unpacklo_epi8(*(__m128i*)&q3, zero));
	r4 = _mm_adds_epi16(r4, _mm_unpacklo_epi8(*(__m128i*)&q4, zero));
	r5 = _mm_adds_epi16(r5, _mm_unpacklo_epi8(*(__m128i*)&q5, zero));
	r6 = _mm_adds_epi16(r6, _mm_unpacklo_epi8(*(__m128i*)&q6, zero));
	r7 = _mm_adds_epi16(r7, _mm_unpacklo_epi8(*(__m128i*)&q7, zero));

	r0 = _mm_packus_epi16(r0, r1);
	r1 = _mm_packus_epi16(r2, r3);
	r2 = _mm_packus_epi16(r4, r5);
	r3 = _mm_packus_epi16(r6, r7);

	_mm_storel_pi((__m64*)&dest[0*stride], *(__m128*)&r0);
	_mm_storeh_pi((__m64*)&dest[1*stride], *(__m128*)&r0);
	_mm_storel_pi((__m64*)&dest[2*stride], *(__m128*)&r1);
	_mm_storeh_pi((__m64*)&dest[3*stride], *(__m128*)&r1);
	_mm_storel_pi((__m64*)&dest[4*stride], *(__m128*)&r2);
	_mm_storeh_pi((__m64*)&dest[5*stride], *(__m128*)&r2);
	_mm_storel_pi((__m64*)&dest[6*stride], *(__m128*)&r3);
	_mm_storeh_pi((__m64*)&dest[7*stride], *(__m128*)&r3);

	_mm_store_si128(&src[0], zero);
	_mm_store_si128(&src[1], zero);
	_mm_store_si128(&src[2], zero);
	_mm_store_si128(&src[3], zero);
	_mm_store_si128(&src[4], zero);
	_mm_store_si128(&src[5], zero);
	_mm_store_si128(&src[6], zero);
	_mm_store_si128(&src[7], zero);
}

void mpeg2_idct_init_sse2()
{
}
