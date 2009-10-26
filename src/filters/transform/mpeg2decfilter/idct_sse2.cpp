/* 
 *    Copyright (C) 2003-2006 Gabest
 *    http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  Based on Intel's AP-942
 *
 */

#include "stdafx.h"
#include <inttypes.h>
#include "libmpeg2.h"
#include "attributes.h"
#include "..\..\..\DSUtil\simd.h"

// Intel's SSE2 implementation of iDCT
// AP-945
// http://cache-www.intel.com/cd/00/00/01/76/17680_w_idct.pdf

static const int BITS_INV_ACC=4;
static const int SHIFT_INV_ROW=16-BITS_INV_ACC;
static const int SHIFT_INV_COL=1+BITS_INV_ACC;
static const int RND_INV_ROW  =1024*(6-BITS_INV_ACC);
static const int RND_INV_COL  =16*(BITS_INV_ACC-3);
static const int RND_INV_CORR =RND_INV_COL-1;

static __align16(const short,M128_round_inv_row[8]) = {RND_INV_ROW, 0, RND_INV_ROW, 0, RND_INV_ROW, 0, RND_INV_ROW, 0};
static __align16(const short,M128_one_corr[8]) = {1,1,1,1,1,1,1,1};
static __align16(const short,M128_round_inv_col[8]) = {RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL, RND_INV_COL};
static __align16(const short,M128_round_inv_corr[8])= {RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR, RND_INV_CORR};
static __align16(const short,M128_tg_1_16[8]) = {13036, 13036, 13036, 13036, 13036, 13036, 13036, 13036}; // tg * (2<<16) + 0.5
static __align16(const short,M128_tg_2_16[8]) = {27146, 27146, 27146, 27146, 27146, 27146, 27146, 27146}; // tg * (2<<16) + 0.5
static __align16(const short,M128_tg_3_16[8]) = {-21746, -21746, -21746, -21746, -21746, -21746, -21746, -21746}; // tg * (2<<16) + 0.5
static __align16(const short,M128_cos_4_16[8]) = {-19195, -19195, -19195, -19195, -19195, -19195, -19195, -19195};// cos * (2<<16) + 0.5

static __align16(const int16_t,M128_tab_i_04[])={16384, 21407, 16384,  8867, 16384,  -8867, 16384, -21407, 16384,  8867, -16384, -21407, -16384, 21407, 16384,  -8867, 22725, 19266, 19266, -4520, 12873, -22725, 4520, -12873, 12873, 4520, -22725, -12873, 4520, 19266, 19266, -22725};
static __align16(const int16_t,M128_tab_i_17[])={22725, 29692, 22725, 12299, 22725, -12299, 22725, -29692, 22725, 12299, -22725, -29692, -22725, 29692, 22725, -12299, 31521, 26722, 26722, -6270, 17855, -31521, 6270, -17855, 17855, 6270, -31521, -17855, 6270, 26722, 26722, -31521};
static __align16(const int16_t,M128_tab_i_26[])={21407, 27969, 21407, 11585, 21407, -11585, 21407, -27969, 21407, 11585, -21407, -27969, -21407, 27969, 21407, -11585, 29692, 25172, 25172, -5906, 16819, -29692, 5906, -16819, 16819, 5906, -29692, -16819, 5906, 25172, 25172, -29692};
static __align16(const int16_t,M128_tab_i_35[])={19266, 25172, 19266, 10426, 19266, -10426, 19266, -25172, 19266, 10426, -19266, -25172, -19266, 25172, 19266, -10426, 26722, 22654, 22654, -5315, 15137, -26722, 5315, -15137, 15137, 5315, -26722, -15137, 5315, 22654, 22654, -26722};

//#ifdef _WIN64		// Temporary patch : full intrinsic version didn't works in Win32 (to be fixed later...) 
#if 1				// (Spec-Chum: Works now, will keep intel code in place though...)

static __forceinline void DCT_8_INV_ROW(const uint8_t * const ecx,const uint8_t * const esi,__m128i &xmm0,__m128i &xmm1,__m128i &xmm2,__m128i &xmm3,__m128i &xmm4,__m128i &xmm5,__m128i &xmm6,__m128i &xmm7)
{
     xmm0=_mm_shufflelo_epi16(xmm0, 0xD8 );
     xmm1=_mm_shuffle_epi32( xmm0, 0 );
     pmaddwd (xmm1, esi);
     xmm3=_mm_shuffle_epi32( xmm0, 0x55);
     xmm0=_mm_shufflehi_epi16( xmm0, 0xD8 );
     pmaddwd( xmm3, esi+32 );
     xmm2=_mm_shuffle_epi32( xmm0, 0xAA );
     xmm0=_mm_shuffle_epi32( xmm0, 0xFF );
     pmaddwd( xmm2, esi+16 );
     xmm4=_mm_shufflehi_epi16( xmm4, 0xD8 );
     paddd (xmm1, M128_round_inv_row);
     xmm4=_mm_shufflelo_epi16 (xmm4, 0xD8 );
     pmaddwd (xmm0, esi+48 );
     xmm5=_mm_shuffle_epi32( xmm4, 0 );
     xmm6=_mm_shuffle_epi32( xmm4, 0xAA );
     pmaddwd (xmm5, ecx );
     paddd (xmm1, xmm2 );
     movdqa (xmm2, xmm1 );
     xmm7=_mm_shuffle_epi32( xmm4, 0x55 );
     pmaddwd (xmm6, ecx+16 );
     paddd (xmm0, xmm3 );
     xmm4=_mm_shuffle_epi32( xmm4, 0xFF );
     psubd (xmm2, xmm0 );
     pmaddwd (xmm7, ecx+32 );
     paddd (xmm0, xmm1 );
     psrad (xmm2, 12 );
     paddd (xmm5, M128_round_inv_row);
     pmaddwd (xmm4, ecx+48 );
     paddd (xmm5, xmm6 );
     movdqa (xmm6, xmm5 );
     psrad (xmm0, 12 );
     xmm2=_mm_shuffle_epi32( xmm2, 0x1B );
     packssdw (xmm0, xmm2 );
     paddd (xmm4, xmm7 );
     psubd (xmm6, xmm4 );
     paddd (xmm4, xmm5 );
     psrad (xmm6, 12 );
     psrad (xmm4, 12 );
     xmm6=_mm_shuffle_epi32( xmm6, 0x1B );
     packssdw (xmm4, xmm6 );
}
static __forceinline void DCT_8_INV_COL_8(__m128i &src0,__m128i &src1,__m128i &src2,__m128i &src3,__m128i &src4,__m128i &src5,__m128i &src6,__m128i &src7,
                                          __m128i &xmm0,__m128i &xmm1,__m128i &xmm2,__m128i &xmm3,__m128i &xmm4,__m128i &xmm5,__m128i &xmm6,__m128i &xmm7)
{
    movdqa( xmm1,  M128_tg_3_16  );
    movdqa( xmm2, xmm0           );
    movdqa( xmm3,  src3      );
    pmulhw( xmm0, xmm1           );
    pmulhw( xmm1, xmm3           );
    movdqa( xmm5,  M128_tg_1_16  );
    movdqa( xmm6, xmm4           );
    pmulhw( xmm4, xmm5           );
    paddsw( xmm0, xmm2           );
    pmulhw( xmm5, src1       );
    paddsw( xmm1, xmm3           );
    movdqa( xmm7,  src6      );
    paddsw( xmm0, xmm3           );
    movdqa( xmm3,  M128_tg_2_16  );
    psubsw( xmm2, xmm1           );
    pmulhw( xmm7, xmm3           );
    movdqa( xmm1, xmm0           );
    pmulhw( xmm3, src2       );
    psubsw( xmm5, xmm6           );
    paddsw( xmm4, src1       );
    paddsw( xmm0, xmm4           );
    paddsw( xmm0,  M128_one_corr );
    psubsw( xmm4, xmm1           );
    movdqa( xmm6, xmm5           );
    psubsw( xmm5, xmm2           );
    paddsw( xmm5,  M128_one_corr );
    paddsw( xmm6, xmm2           );
    movdqa( src7, xmm0       );
    movdqa( xmm1, xmm4           );
    movdqa( xmm0,  M128_cos_4_16 );
    paddsw( xmm4, xmm5           );
    movdqa( xmm2,  M128_cos_4_16 );
    pmulhw( xmm2, xmm4           );
    movdqa( src3, xmm6       );
    psubsw( xmm1, xmm5           );
    paddsw( xmm7, src2       );
    psubsw( xmm3, src6       );
    movdqa( xmm6, src0           );
    pmulhw( xmm0, xmm1           );
    movdqa( xmm5, src4       );
    paddsw( xmm5, xmm6          );
    psubsw( xmm6, src4       );
    paddsw( xmm4, xmm2           );
    por   (  xmm4,  M128_one_corr     );
    paddsw(  xmm0, xmm1                 );
    por   (  xmm0,  M128_one_corr     );
    movdqa( xmm2, xmm5                  );
    paddsw( xmm5, xmm7                  );
    movdqa( xmm1, xmm6                  );
    paddsw( xmm5,  M128_round_inv_col );
    psubsw( xmm2, xmm7                  );
    movdqa( xmm7, src7            );
    paddsw( xmm6, xmm3                  );
    paddsw( xmm6,  M128_round_inv_col );
    paddsw( xmm7, xmm5                  );
    psraw ( xmm7, SHIFT_INV_COL           );
    psubsw( xmm1, xmm3                   );
    paddsw( xmm1,  M128_round_inv_corr );
    movdqa( xmm3, xmm6                   );
    paddsw( xmm2,  M128_round_inv_corr );
    paddsw( xmm6, xmm4                   );
    movdqa( src0,xmm7                  );
    psraw (xmm6, SHIFT_INV_COL           );
    movdqa( xmm7, xmm1                   );
    paddsw( xmm1, xmm0                   );
    movdqa( src1, xmm6             );
    psraw (xmm1, SHIFT_INV_COL           );
    movdqa( xmm6, src3             );
    psubsw( xmm7, xmm0                   );
    psraw (xmm7, SHIFT_INV_COL           );
    movdqa( src2, xmm1             );
    psubsw( xmm5, src7             );
    psraw (xmm5, SHIFT_INV_COL           );
    movdqa( src7, xmm5             );
    psubsw( xmm3, xmm4                   );
    paddsw( xmm6, xmm2                   );
    psubsw( xmm2, src3             );
    psraw (xmm6, SHIFT_INV_COL           );
    psraw (xmm2, SHIFT_INV_COL           );
    movdqa( src3, xmm6             );
    psraw (xmm3, SHIFT_INV_COL           );
    movdqa( src4, xmm2             );
    movdqa( src5, xmm7             );
    movdqa( src6, xmm3             );
}

static __forceinline void idct_M128ASM(__m128i &src0,__m128i &src1,__m128i &src2,__m128i &src3,__m128i &src4,__m128i &src5,__m128i &src6,__m128i &src7)
{
     /*src0=_mm_srai_epi16(src0,4);
     src1=_mm_srai_epi16(src1,4);
     src2=_mm_srai_epi16(src2,4);
     src3=_mm_srai_epi16(src3,4);
     src4=_mm_srai_epi16(src4,4);
     src5=_mm_srai_epi16(src5,4);
     src6=_mm_srai_epi16(src6,4);
     src7=_mm_srai_epi16(src7,4); // No idea why they needed to be shifted?  ...and it makes the picture all green...*/

__m128i xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
     movdqa (xmm0, src0);
     uint8_t *esi=(uint8_t*)M128_tab_i_04;
     movdqa (xmm4, src2);
     uint8_t *ecx=(uint8_t*)M128_tab_i_26;
    DCT_8_INV_ROW(ecx,esi,xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7);
     movdqa (src0, xmm0);
     movdqa (src2, xmm4);

     movdqa (xmm0, src4);
     movdqa (xmm4, src6);
    DCT_8_INV_ROW(ecx,esi,xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7);
     movdqa (src4, xmm0);
     movdqa (src6, xmm4);

     movdqa (xmm0, src3);
     esi=(uint8_t*)M128_tab_i_35;
     movdqa (xmm4, src1);
     ecx=(uint8_t*)M128_tab_i_17;
    DCT_8_INV_ROW(ecx,esi,xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7);
     movdqa (src3, xmm0);
     movdqa (src1, xmm4);

     movdqa (xmm0, src5);
     movdqa (xmm4, src7);
    DCT_8_INV_ROW(ecx,esi,xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7);
    DCT_8_INV_COL_8(src0,src1,src2,src3,src4,src5,src6,src7,
                    xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7);
}

void mpeg2_idct_copy_sse2(int16_t* block, uint8_t* dest, const int stride)
{
 __m128i &src0=*(__m128i*)(block+0*16/2);
 __m128i &src1=*(__m128i*)(block+1*16/2);
 __m128i &src2=*(__m128i*)(block+2*16/2);
 __m128i &src3=*(__m128i*)(block+3*16/2);
 __m128i &src4=*(__m128i*)(block+4*16/2);
 __m128i &src5=*(__m128i*)(block+5*16/2);
 __m128i &src6=*(__m128i*)(block+6*16/2);
 __m128i &src7=*(__m128i*)(block+7*16/2);
            idct_M128ASM (src0,src1,src2,src3,src4,src5,src6,src7);

    __m128i zero = _mm_setzero_si128();

    __m128i r0 = _mm_packus_epi16(_mm_load_si128(&src0), _mm_load_si128(&src1));
    __m128i r1 = _mm_packus_epi16(_mm_load_si128(&src2), _mm_load_si128(&src3));
    __m128i r2 = _mm_packus_epi16(_mm_load_si128(&src4), _mm_load_si128(&src5));
    __m128i r3 = _mm_packus_epi16(_mm_load_si128(&src6), _mm_load_si128(&src7));

    _mm_storel_pi((__m64*)&dest[0*stride], *(__m128*)&r0);
    _mm_storeh_pi((__m64*)&dest[1*stride], *(__m128*)&r0);
    _mm_storel_pi((__m64*)&dest[2*stride], *(__m128*)&r1);
    _mm_storeh_pi((__m64*)&dest[3*stride], *(__m128*)&r1);
    _mm_storel_pi((__m64*)&dest[4*stride], *(__m128*)&r2);
    _mm_storeh_pi((__m64*)&dest[5*stride], *(__m128*)&r2);
    _mm_storel_pi((__m64*)&dest[6*stride], *(__m128*)&r3);
    _mm_storeh_pi((__m64*)&dest[7*stride], *(__m128*)&r3);

    _mm_store_si128(&src0, zero);
    _mm_store_si128(&src1, zero);
    _mm_store_si128(&src2, zero);
    _mm_store_si128(&src3, zero);
    _mm_store_si128(&src4, zero);
    _mm_store_si128(&src5, zero);
    _mm_store_si128(&src6, zero);
    _mm_store_si128(&src7, zero);
}

void mpeg2_idct_add_sse2(int,int16_t* block, uint8_t* dest, const int stride)
{
 __m128i &src0=*(__m128i*)(block+0*16/2);
 __m128i &src1=*(__m128i*)(block+1*16/2);
 __m128i &src2=*(__m128i*)(block+2*16/2);
 __m128i &src3=*(__m128i*)(block+3*16/2);
 __m128i &src4=*(__m128i*)(block+4*16/2);
 __m128i &src5=*(__m128i*)(block+5*16/2);
 __m128i &src6=*(__m128i*)(block+6*16/2);
 __m128i &src7=*(__m128i*)(block+7*16/2);
            idct_M128ASM (src0,src1,src2,src3,src4,src5,src6,src7);

    __m128i zero = _mm_setzero_si128();

    __m128i r0 = _mm_load_si128(&src0);
    __m128i r1 = _mm_load_si128(&src1);
    __m128i r2 = _mm_load_si128(&src2);
    __m128i r3 = _mm_load_si128(&src3);
    __m128i r4 = _mm_load_si128(&src4);
    __m128i r5 = _mm_load_si128(&src5);
    __m128i r6 = _mm_load_si128(&src6);
    __m128i r7 = _mm_load_si128(&src7);

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

    _mm_store_si128(&src0, zero);
    _mm_store_si128(&src1, zero);
    _mm_store_si128(&src2, zero);
    _mm_store_si128(&src3, zero);
    _mm_store_si128(&src4, zero);
    _mm_store_si128(&src5, zero);
    _mm_store_si128(&src6, zero);
    _mm_store_si128(&src7, zero);
}

void mpeg2_idct_init_sse2()
{
}

#else	// Temporary patch : full intrinsic version didn't works in Win32 (to be fixed later...)

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

#endif