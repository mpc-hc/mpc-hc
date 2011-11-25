#ifndef _SIMD_H_
#define _SIMD_H_

#include "simd_common.h"

#pragma warning(push)
#pragma warning(disable:4799)
#pragma warning(disable:4309)
#pragma warning(disable:4700)

#define MMX_INSTRUCTION(instruction,function) \
 static __forceinline void instruction(__m64 &dst,const __m64   &src) {dst=function(dst,          src);} \
 static __forceinline void instruction(__m64 &dst,const void    *src) {dst=function(dst,*(__m64*) src);} \
 static __forceinline void instruction(__m64 &dst,const __int64 &src) {dst=function(dst,*(__m64*)&src);}

#define SSE2I_INSTRUCTION(instruction,function) \
 static __forceinline void instruction(__m128i &dst,const __m128i &src) {dst=function(dst,           src);} \
 static __forceinline void instruction(__m128i &dst,const void    *src) {dst=function(dst,*(__m128i*)src);}

#include "simd_instructions.h"

#undef MMX_INSTRUCTION
#undef SSE2I_INSTRUCTION

static __forceinline void movq(__m64 &dst,const __m64 &src)
{
    dst=src;
}
static __forceinline void movq(__m64 &dst,const void *src)
{
    dst=*(__m64*)src;
}
static __forceinline void movq(__m64 &dst,const __int64 &src)
{
    dst=*(__m64*)&src;
}
static __forceinline void movq(void *dst,const __m64 &src)
{
    *(__m64*)dst=src;
}
static __forceinline void movntq(void *dst,const __m64 &src)
{
    _mm_stream_pi((__m64*)dst,src);
}

static __forceinline void movdqu(__m64 &dst,const void *src)
{
    dst=*(__m64*)src;
}

static __forceinline void movd(__m64 &dst,int src)
{
    dst=_mm_cvtsi32_si64(src);
}
static __forceinline void movd(int &dst,const __m64 &src)
{
    dst=_mm_cvtsi64_si32(src);
}
static __forceinline void movd(__m64 &dst,const void *src)
{
    dst=_mm_cvtsi32_si64(*(const int*)src);
}
static __forceinline void movd(void *dst,const __m64 &src)
{
    *(int*)dst=_mm_cvtsi64_si32(src);
}

static __forceinline void psllq(__m64 &dst,int i)
{
    dst=_mm_slli_si64(dst,i);
}
static __forceinline void pslld(__m64 &dst,int i)
{
    dst=_mm_slli_pi32(dst,i);
}
static __forceinline void psllw(__m64 &dst,int i)
{
    dst=_mm_slli_pi16(dst,i);
}
static __forceinline void psrlq(__m64 &dst,int i)
{
    dst=_mm_srli_si64(dst,i);
}
static __forceinline void psrld(__m64 &dst,int i)
{
    dst=_mm_srli_pi32(dst,i);
}
static __forceinline void psrlw(__m64 &dst,int i)
{
    dst=_mm_srli_pi16(dst,i);
}
static __forceinline void psraw(__m64 &dst,int i)
{
    dst=_mm_srai_pi16(dst,i);
}
static __forceinline void psraw(__m64 &dst,const __m64 &src)
{
    dst=_mm_sra_pi16(dst,src);
}
static __forceinline void psrad(__m64 &dst,int i)
{
    dst=_mm_srai_pi32(dst,i);
}

// load the same width as the register aligned
static __forceinline void movVqa(__m64 &dst,const void *ptr)
{
    dst = *(__m64*)ptr;
}

// load the same width as the register un-aligned
static __forceinline void movVqu(__m64 &dst,const void *ptr)
{
    dst = *(__m64*)ptr;
}

// store the same width as the register un-aligned
static __forceinline void movVqu(void *ptr,const __m64 &m)
{
    *(__m64*)ptr=m;
}

// load half width of the register (4 bytes for MMX/MMXEXT)
static __forceinline void movHalf(__m64 &dst, const void *ptr)
{
    dst = _mm_cvtsi32_si64(*(int*)ptr);
}

// load quarter width of the register (2 bytes for MMX/MMXEXT)
static __forceinline void movQuarter(__m64 &m, const void *ptr)
{
    uint16_t d = *(uint16_t*)ptr;
    m = _mm_cvtsi32_si64(int(d));
}

static __forceinline void prefetcht0(const void *a)
{
    _mm_prefetch((char*)a,_MM_HINT_T0);
}

static __forceinline void movaps(__m128 &dst,const __m128 &src)
{
    dst=src;
}
static __forceinline void movaps(void *dst,const __m128 &src)
{
    *(__m128*)dst=src;
}
static __forceinline void movups(__m128 &dst,const void *src)
{
    dst=_mm_loadu_ps((float*)src);
}
static __forceinline void movups(void *dst,const __m128 &src)
{
    _mm_storeu_ps((float*)dst,src);
}
static __forceinline void movss(__m128 &dst,const void *src)
{
    dst=_mm_load_ss((float*)src);
}
static __forceinline void movss(void *dst,const __m128 &src)
{
    _mm_store_ss((float*)dst,src);
}
static __forceinline void movhlps(__m128 &dst,const __m128 &src)
{
    dst=_mm_movehl_ps(dst,src);
}
static __forceinline void movlhps(__m128 &dst,const __m128 &src)
{
    dst=_mm_movelh_ps(dst,src);
}
static __forceinline void movlps(__m128 &dst,const void *src)
{
    dst=_mm_loadl_pi(dst,(const __m64*)src);
}
static __forceinline void movlps(void *dst,const __m128 &src)
{
    _mm_storel_pi((__m64*)dst,src);
}
static __forceinline void movhps(__m128 &dst,const void *src)
{
    dst=_mm_loadh_pi(dst,(const __m64*)src);
}
static __forceinline void movhps(void *dst,const __m128 &src)
{
    _mm_storeh_pi((__m64*)dst,src);
}

static __forceinline void xorps(__m128 &dst,const __m128 &src)
{
    dst=_mm_xor_ps(dst,src);
}
static __forceinline void addps(__m128 &dst,const __m128 &src)
{
    dst=_mm_add_ps(dst,src);
}
static __forceinline void addss(__m128 &dst,const __m128 &src)
{
    dst=_mm_add_ss(dst,src);
}
static __forceinline void mulps(__m128 &dst,const __m128 &src)
{
    dst=_mm_mul_ps(dst,src);
}
static __forceinline void mulss(__m128 &dst,const __m128 &src)
{
    dst=_mm_mul_ss(dst,src);
}
static __forceinline void minps(__m128 &dst,const __m128 &src)
{
    dst=_mm_min_ps(dst,src);
}
static __forceinline void cvtps2pi(__m64 &dst,const __m128 &src)
{
    dst=_mm_cvtps_pi32(src);
}
static __forceinline void cmpnltps(__m128 &dst,const __m128 &src)
{
    dst=_mm_cmpnlt_ps(dst,src);
}
static __forceinline void cvtpi2ps(__m128 &dst,const __m64 &src)
{
    dst=_mm_cvtpi32_ps(dst,src);
}

#ifdef __SSE2__
static __forceinline void movq(__m128i &dst,const __m128i &src)
{
    dst=src;
}
static __forceinline void movq(__m128i &dst,const void *src)
{
    dst=*(__m128i*)src;
}
static __forceinline void movq(const void *dst,__m128i &src)
{
    *(__m128i*)dst=src;
}
static __forceinline void movd(__m128i &dst,const void *src)
{
    dst=_mm_loadl_epi64((__m128i*)src);
}
static __forceinline void movd(void *dst,const __m128i &src)
{
    _mm_storel_epi64((__m128i*)dst,src);
}

static __forceinline void movdqu(__m128i &dst,const void *src)
{
    dst=_mm_loadu_si128((__m128i*)src);
}
static __forceinline void movdqu(__m128i &dst,const __m128i &src)
{
    dst=_mm_loadu_si128(&src);
}
static __forceinline void movdqa(__m128i &dst,const __m128i &src)
{
    dst=src;
}
static __forceinline void movdqa(__m128i &dst,const void * src)
{
    dst=_mm_load_si128((__m128i*)src);
}
static __forceinline void movdqa(void *dst,const __m128i &src)
{
    _mm_store_si128((__m128i*)dst,src);
}
static __forceinline void movntdq(void *dst,const __m128i &src)
{
    _mm_stream_si128((__m128i*)dst,src);
}
static __forceinline void movdq2q(__m64 &dst,const __m128i &src)
{
    dst=_mm_movepi64_pi64(src);
}

static __forceinline void psrlw(__m128i &dst,int i)
{
    dst=_mm_srli_epi16(dst,i);
}
static __forceinline void psrlq(__m128i &dst,int i)
{
    dst=_mm_srli_epi64(dst,i);
}
static __forceinline void psrad(__m128i &dst,int i)
{
    dst=_mm_srai_epi32(dst,i);
}
static __forceinline void psraw(__m128i &dst,int i)
{
    dst=_mm_srai_epi16(dst,i);
}
static __forceinline void psraw(__m128i &dst,const __m128i &src)
{
    dst=_mm_sra_epi16(dst,src);
}
static __forceinline void psllw(__m128i &dst,int i)
{
    dst=_mm_slli_epi16(dst,i);
}
static __forceinline void pslld(__m128i &dst,int i)
{
    dst=_mm_slli_epi32(dst,i);
}
static __forceinline void psllq(__m128i &dst,int i)
{
    dst=_mm_slli_epi64(dst,i);
}

//static __forceinline void pshufd(__m128i &dst,const __m128i &src,const int i) {dst=_mm_shuffle_epi32(src,i);}
//static __forceinline void pshuflw(__m128i &dst,const __m128i &src,const int i) {dst=_mm_shufflelo_epi16(src,i);}
//static __forceinline void pshufhw(__m128i &dst,const __m128i &src,const int i) {dst=_mm_shufflehi_epi16(src,i);}

static __forceinline void cvtps2dq(__m128i &dst,const __m128 &src)
{
    dst=_mm_cvtps_epi32(src);
}
static __forceinline void cvtdq2ps(__m128 &dst,const __m128i &src)
{
    dst=_mm_cvtepi32_ps(src);
}

static __forceinline void movlpd(__m128d &dst,const void *src)
{
    dst=_mm_loadl_pd(dst,(double*)src);
}
static __forceinline void movhpd(__m128d &dst,const void *src)
{
    dst=_mm_loadh_pd(dst,(double*)src);
}
static __forceinline void movlpd(void *dst,const __m128d &src)
{
    _mm_storel_pd((double*)dst,src);
}
static __forceinline void movhpd(void *dst,const __m128d &src)
{
    _mm_storeh_pd((double*)dst,src);
}

// load the same width as the register aligned
static __forceinline void movVqa(__m128i &dst, const void *ptr)
{
    dst = _mm_load_si128((const __m128i*)ptr);
}

// load the same width as the register un-aligned
static __forceinline void movVqu(__m128i &dst, const void *ptr)
{
    dst = _mm_loadu_si128((const __m128i*)ptr);
}

// store the same width as the register un-aligned
static __forceinline void movVqu(void *ptr,const __m128i &m)
{
    _mm_storeu_si128((__m128i*)ptr,m);
}

// load half width of the register (8 bytes for SSE2)
static __forceinline void movHalf(__m128i &dst, const void *ptr)
{
    dst = _mm_loadl_epi64((const __m128i*)ptr);
}

// load quarter width of the register (4 bytes for SSE2)
static __forceinline void movQuarter(__m128i &dst, const void *ptr)
{
    dst = _mm_cvtsi32_si128(*(int*)ptr);
}

#if defined(__INTEL_COMPILER) || (defined(__GNUC__) && __GNUC__>=4)
static __forceinline void movlpd(__m128i &dst,const void *src)
{
    dst=_mm_castpd_si128(_mm_loadl_pd(_mm_castsi128_pd(dst),(double*)src));
}
static __forceinline void movhpd(__m128i &dst,const void *src)
{
    dst=_mm_castpd_si128(_mm_loadh_pd(_mm_castsi128_pd(dst),(double*)src));
}
static __forceinline void movlpd(void *dst,const __m128i &src)
{
    _mm_storel_pd((double*)dst,_mm_castsi128_pd(src));
}
static __forceinline void movhpd(void *dst,const __m128i &src)
{
    _mm_storeh_pd((double*)dst,_mm_castsi128_pd(src));
}

static __forceinline void movlps(__m128i &dst,const void *src)
{
    dst=_mm_castps_si128(_mm_loadl_pi(_mm_castsi128_ps(dst),(const __m64*)src));
}
static __forceinline void movlps(void *dst,const __m128i &src)
{
    _mm_storel_pi((__m64*)dst,_mm_castsi128_ps(src));
}
static __forceinline void movhps(__m128i &dst,const void *src)
{
    dst=_mm_castps_si128(_mm_loadh_pi(_mm_castsi128_ps(dst),(const __m64*)src));
}
static __forceinline void movhps(void *dst,const __m128i &src)
{
    _mm_storeh_pi((__m64*)dst,_mm_castsi128_ps(src));
}

static __forceinline void movlhps(__m128i &dst,const __m128i &src)
{
    dst=_mm_castps_si128(_mm_movelh_ps(_mm_castsi128_ps(dst),_mm_castsi128_ps(src)));
}
#else
static __forceinline __m128i _mm_castps_si128(__m128 &src)
{
    return (__m128i&)src;
}
static __forceinline void movlpd(__m128i &dst,const void *src)
{
    (__m128d&)dst=_mm_loadl_pd((__m128d&)dst,(double*)src);
}
static __forceinline void movhpd(__m128i &dst,const void *src)
{
    (__m128d&)dst=_mm_loadh_pd((__m128d&)dst,(double*)src);
}
static __forceinline void movlpd(void *dst,const __m128i &src)
{
    _mm_storel_pd((double*)dst,(const __m128d&)src);
}
static __forceinline void movhpd(void *dst,const __m128i &src)
{
    _mm_storeh_pd((double*)dst,(const __m128d&)src);
}

static __forceinline void movlps(__m128i &dst,const void *src)
{
    (__m128&)dst=_mm_loadl_pi((__m128&)dst,(const __m64*)src);
}
static __forceinline void movlps(void *dst,const __m128i &src)
{
    _mm_storel_pi((__m64*)dst,(const __m128&)src);
}
static __forceinline void movhps(__m128i &dst,const void *src)
{
    (__m128&)dst=_mm_loadh_pi((__m128&)dst,(const __m64*)src);
}
static __forceinline void movhps(void *dst,const __m128i &src)
{
    _mm_storeh_pi((__m64*)dst,(const __m128&)src);
}

static __forceinline void movlhps(__m128i &dst,const __m128i &src)
{
    (__m128&)dst=_mm_movelh_ps((__m128&)dst,(const __m128&)src);
}
#endif

#endif //__SSE2__

//======================================= MMX ======================================
#define MMX_INSTRUCTIONS \
 static __forceinline __m setzero_si64(void) {return _mm_setzero_si64();} \
 static __forceinline __m set_pi8(char b7,char b6,char b5,char b4,char b3,char b2,char b1,char b0) {return _mm_set_pi8(b7,b6,b5,b4,b3,b2,b1,b0);} \
 static __forceinline __m set_pi32(int i1,int i0) {return _mm_set_pi32(i1,i0);} \
 static __forceinline __m set1_pi8(char b) {return _mm_set1_pi8(b);} \
 static __forceinline __m set1_pi16(short s) {return _mm_set1_pi16(s);} \
 static __forceinline __m set1_pi64(int64_t s) {return *(__m64*)&s;} \
 static __forceinline __m packs_pu16(const __m &m1,const __m &m2) {return _mm_packs_pu16(m1,m2);} \
 static __forceinline __m slli_pi16(const __m &m,int count) {return _mm_slli_pi16(m,count);} \
 static __forceinline __m srli_pi16(const __m &m,int count) {return _mm_srli_pi16(m,count);} \
 static __forceinline __m srli_si64(const __m &m,int count) {return _mm_srli_si64(m,count);} \
 static __forceinline __m srai_pi16(const __m &m,int count) {return _mm_srai_pi16(m,count);} \
 static __forceinline __m madd_pi16(const __m &m1,const __m &m2) {return _mm_madd_pi16(m1,m2);} \
 static __forceinline __m add_pi16(const __m &m1,const __m &m2) {return _mm_add_pi16(m1,m2);} \
 static __forceinline __m adds_pi16(const __m &m1,const __m &m2) {return _mm_adds_pi16(m1,m2);} \
 static __forceinline __m adds_pu16(const __m &m1,const __m &m2) {return _mm_adds_pu16(m1,m2);} \
 static __forceinline __m adds_pu8(const __m &m1,const __m &m2) {return _mm_adds_pu8(m1,m2);} \
 static __forceinline __m sub_pi16(const __m &m1,const __m &m2) {return _mm_sub_pi16(m1,m2);} \
 static __forceinline __m subs_pi16(const __m &m1,const __m &m2) {return _mm_subs_pi16(m1,m2);} \
 static __forceinline __m subs_pu16(const __m &m1,const __m &m2) {return _mm_subs_pu16(m1,m2);} \
 static __forceinline __m subs_pu8(const __m &m1,const __m &m2) {return _mm_subs_pu8(m1,m2);} \
 static __forceinline __m or_si64(const __m &m1,const __m &m2) {return _mm_or_si64(m1,m2);} \
 static __forceinline __m xor_si64(const __m &m1,const __m &m2) {return _mm_xor_si64(m1,m2);} \
 static __forceinline __m and_si64(const __m &m1,const __m &m2) {return _mm_and_si64(m1,m2);} \
 static __forceinline __m andnot_si64(const __m &m1,const __m &m2) {return _mm_andnot_si64(m1,m2);} \
 static __forceinline __m mullo_pi16(const __m &m1,const __m &m2) {return _mm_mullo_pi16(m1,m2);} \
 static __forceinline __m mulhi_pi16(const __m &m1,const __m &m2) {return _mm_mulhi_pi16(m1,m2);} \
 static __forceinline __m unpacklo_pi8(const __m &m1,const __m &m2) {return _mm_unpacklo_pi8(m1,m2);} \
 static __forceinline __m unpackhi_pi8(const __m &m1,const __m &m2) {return _mm_unpackhi_pi8(m1,m2);} \
 static __forceinline __m cmpgt_pi16(const __m &m1,const __m &m2) {return _mm_cmpgt_pi16(m1,m2);} \
 static __forceinline __m cmpeq_pi16(const __m &m1,const __m &m2) {return _mm_cmpeq_pi16(m1,m2);} \
 static __forceinline __m cmpeq_pi8(const __m &m1,const __m &m2) {return _mm_cmpeq_pi8(m1,m2);} \
 static __forceinline __m shuffle_pi16_0(__m64 mm3) {return _mm_shuffle_pi16_0(mm3);} \
 static __forceinline void store2(void *ptr,const __m &m) {*(int2*)ptr=_mm_cvtsi64_si32(m);} \
 static __forceinline __m load2(const void *ptr) {return _mm_cvtsi32_si64(*(int2*)ptr);} \
 static __forceinline void empty(void) {_mm_empty();}

struct Tmmx {
    typedef __m64 __m;
    typedef int32_t int2;
    typedef int32_t integer2_t;
    static const size_t size=sizeof(__m);
    static const int align=0;
    typedef Tmmx T64;

    static __forceinline void pmaxub(__m64 &mmr1,const __m64 &mmr2) {
        mmr1=_mm_subs_pu8(mmr1,mmr2);
        mmr1=_mm_adds_pu8(mmr1,mmr2);
    }
    static __forceinline void pmaxub(__m64 &mmr1,const void *mmr2) {
        pmaxub(mmr1,*(__m64*)mmr2);
    }
    static __forceinline void pminub(__m64 &mmr1,const __m64 &mmr2) {
        __m64 mmrw;
        pcmpeqb (mmrw,mmrw );
        psubusb (mmrw,mmr2 );
        paddusb (mmr1, mmrw);
        psubusb (mmr1, mmrw);
    }
    static __forceinline void pminub(__m64 &mmr1,const void *mmr2) {
        pminub(mmr1,*(__m64*)mmr2);
    }
    static __forceinline void pmaxsw(__m64 &a,const __m64 &b) {
        psubusw(a,b);
        paddw(a,b);
    }
    static __forceinline void pminsw(__m64 &mm4,const __m64 &mm0) {
        __m64 mm2;
        movq   (mm2,mm4);
        psubusw(mm2,mm0);
        psubw  (mm4,mm2);
    }
    static __forceinline void pavgb(__m64 &rega,__m64 regb) {
        __m64 regr;
        static const __int64 regfe=0xfefefefefefefefeULL;//_mm_set1_pi8(/*0xfe*/-2);
        movq    (regr,rega);
        por     (regr,regb);
        pxor    (regb,rega);
        pand    (regb,regfe);
        psrlq   (regb,1);
        psubb   (regr,regb);
        rega=regr;
    }
    static __forceinline void pavgb(__m64 &rega,const void *regb) {
        pavgb(rega,*(__m64*)regb);
    }
    static __forceinline void v_pavgb(__m64 &mmr1,const __m64 &mmr2,__m64 &mmrw,const __int64 &smask) {
        movq( mmrw,mmr2 );
        pand( mmrw, smask );
        psrlw( mmrw,1 );
        pand( mmr1,smask );
        psrlw( mmr1,1 );
        paddusb( mmr1,mmrw );
    }
    static __forceinline void v_pavgb(__m64 &mmr1,const void *mmr2,__m64 &mmrw,const __int64 &smask) {
        v_pavgb(mmr1,*(__m64*)mmr2,mmrw,smask);
    }
    static __forceinline void sfence(void) {
    }
    static __forceinline void movntq(void *dst,const __m64 &src) {
        movq(dst,src);
    }
    static __forceinline void v_pminub(__m64 &mmr1,const __m64 &mmr2,__m64 &mmrw) {
        pcmpeqb (mmrw,mmrw );
        psubusb (mmrw,mmr2 );
        paddusb (mmr1, mmrw);
        psubusb (mmr1, mmrw);
    }
    static __forceinline void v_pminub(__m64 &mmr1,const __int64 &mmr2,__m64 &mmrw) {
        v_pminub(mmr1,*(const __m64*)&mmr2,mmrw);
    }
    static __forceinline void pmulhuw(__m64 &mm3,const __m64 &mm2) {
        __m64 mm5;
        movq   ( mm5, mm2);
        psraw  ( mm5, 15 );
        pand   ( mm5, mm3);
        pmulhw ( mm3, mm2);
        paddw  ( mm3, mm5);
    }
    static __forceinline void prefetchnta(const void*) {
    }
    static __forceinline void prefetcht0(const void*) {
    }
    static __forceinline __m64 _mm_shuffle_pi16_0(__m64 mm3) {
        __m64 mm2;
        static const __int64 qwLowWord=0x000000000000FFFF;
        pand    (mm3, qwLowWord);        // mm3 = same limited to low word
        movq    (mm2, mm3);              // mm2 = same
        psllq   (mm3, 16 );              // mm3 = moved to second word
        por     (mm2, mm3);              // mm2 = copied to first and second words
        movq    (mm3, mm2);              // mm3 = same
        psllq   (mm3, 32 );              // mm3 = moved to third and fourth words
        por     (mm2, mm3);              // mm2 = low word copied to all four words
        return mm2;
    }
    static __forceinline __m64 _mm_shuffle_pi16_1(const __m64 &src) {
        static const __int64 const1=0x00000000FFFF0000LL;
        static const __int64 const2=0x000000000000FFFFLL;
        __m64 w0=_mm_srli_si64(_mm_and_si64(src,*(__m64*)&const1),16);
        __m64 w1=_mm_and_si64(src,*(__m64*)&const2);
        return _mm_or_si64(_mm_or_si64(_mm_or_si64(_mm_slli_si64(w1,48),_mm_slli_si64(w1,32)),_mm_slli_si64(w1,16)),w0);
    }
    static __forceinline __m64 _mm_shuffle_pi16_14(const __m64 &src) {
        static const __int64 const1=0x000000000000FFFFLL;
        static const __int64 const2=0xffffffff00000000ULL;
        __m64 w34=_mm_and_si64(src,*(__m64*)&const1);
        __m64 w12=_mm_srli_si64(_mm_and_si64(src,*(__m64*)&const2),32);
        return _mm_or_si64(w12,_mm_or_si64(_mm_slli_si64(w34,32),_mm_slli_si64(w34,48)));
    }
    static __forceinline __m64 _mm_shuffle_pi16_x50(const __m64 &src) {
        static const __int64 const1=0x00000000ffff0000LL;
        static const __int64 const2=0x000000000000ffffLL;
        __m64 w3=_mm_and_si64(src,*(__m64*)&const1);
        __m64 w4=_mm_and_si64(src,*(__m64*)&const2);
        return _mm_or_si64( _mm_or_si64(_mm_slli_si64(w3,32),_mm_slli_si64(w3,16)) , _mm_or_si64(_mm_slli_si64(w4,16),w4));
    }
    static __forceinline void psadbw(__m64 &mm0,const __m64 &SourceMM) {
        __m64 mm1;
        movq (mm1, SourceMM);
        __m64 mm4;
        movq (mm4, mm0);
        psubusb (mm0, mm1);
        psubusb (mm1, mm4);
        por (mm0, mm1);
        __m64 mm7=_mm_setzero_si64();
        movq (mm1,mm0);
        punpcklbw (mm0,mm7);
        punpckhbw (mm1,mm7);
        paddusw (mm0,mm1);
        static const __int64 mmx_one=0x0001000100010001LL;
        pmaddwd (mm0, mmx_one);
        movq (mm7, mm0);
        psrlq (mm7, 32);
        paddd (mm0, mm7);
        static const __int64 mmx_ffff=0x00000000000fffffLL;
        pand (mm0, mmx_ffff);
    }
    static __forceinline __m64 min_pu8(const __m64 &mm1,const __m64 &mm2) {
        __m64 mm0=mm1;
        pminub(mm0,mm2);
        return mm0;
    }
    static __forceinline __m64 max_pu8(const __m64 &mm1,const __m64 &mm2) {
        __m64 mm0=mm1;
        pmaxub(mm0,mm2);
        return mm0;
    }
    static __forceinline __m64 min_pi16(const __m64 &mm1,const __m64 &mm2) {
        __m64 mm0=mm1;
        pminsw(mm0,mm2);
        return mm0;
    }
    static __forceinline __m64 max_pi16(const __m64 &mm1,const __m64 &mm2) {
        __m64 mm0=mm1;
        pmaxsw(mm0,mm2);
        return mm0;
    }
    // store the same width as the register without polluting chache (if supported)
    static __forceinline void movntVq(void *ptr,const __m64 &m)
    {
        *(__m64*)ptr=m;
    }

    MMX_INSTRUCTIONS
};

//===================================== MMXEXT =====================================
struct Tmmxext {
    typedef Tmmx::__m __m;
    typedef Tmmx::int2 int2;
    static const size_t size=Tmmx::size;
    static const int align=Tmmx::align;
    typedef Tmmxext T64;

    static __forceinline void pmaxub(__m64 &mmr1,const __m64 &mmr2) {
        mmr1=_mm_max_pu8(mmr1,mmr2);
    }
    static __forceinline void pmaxub(__m64 &mmr1,const void *mmr2) {
        pmaxub(mmr1,*(__m64*)mmr2);
    }
    static __forceinline void pminub(__m64 &mmr1,const __m64 &mmr2) {
        mmr1=_mm_min_pu8(mmr1,mmr2);
    }
    static __forceinline void pminub(__m64 &mmr1,const void *mmr2) {
        pminub(mmr1,*(__m64*)mmr2);
    }
    static __forceinline void pminsw(__m64 &mmr1,const __m64 &mmr2) {
        mmr1=_mm_min_pi16(mmr1,mmr2);
    }
    static __forceinline void pavgb(__m64 &mmr1,const __m64 &mmr2) {
        mmr1=_mm_avg_pu8(mmr1,mmr2);
    }
    static __forceinline void pavgb(__m64 &mmr1,const void *mmr2) {
        mmr1=_mm_avg_pu8(mmr1,*(__m64*)mmr2);
    }
    static __forceinline void v_pavgb(__m64 &mmr1,const __m64 &mmr2,__m64,__int64) {
        mmr1=_mm_avg_pu8(mmr1,mmr2);
    }
    static __forceinline void v_pavgb(__m64 &mmr1,const void *mmr2,__m64,__int64) {
        mmr1=_mm_avg_pu8(mmr1,*(__m64*)mmr2);
    }
    static __forceinline void sfence(void) {
        _mm_sfence();
    }
    static __forceinline void movntq(void *dst,const __m64 &src) {
        _mm_stream_pi((__m64*)dst,src);
    }
    static __forceinline void v_pminub(__m64 &mmr1,const __m64 &mmr2,__m64) {
        mmr1=_mm_min_pu8(mmr1,mmr2);
    }
    static __forceinline void v_pminub(__m64 &mmr1,const __int64 &mmr2,__m64 &mmrw) {
        v_pminub(mmr1,*(const __m64*)&mmr2,mmrw);
    }
    static __forceinline void pmulhuw(__m64 &mmr1,const __m64 &mmr2) {
        mmr1=_mm_mulhi_pu16(mmr1,mmr2);
    }
    static __forceinline void prefetchnta(const void *ptr) {
        _mm_prefetch((const char*)ptr,_MM_HINT_NTA);
    }
    static __forceinline void prefetcht0(const void *ptr) {
        _mm_prefetch((const char*)ptr,_MM_HINT_T0);
    }
    static __forceinline __m64 _mm_shuffle_pi16_0(const __m64 &src) {
        return _mm_shuffle_pi16(src,0);
    }
    static __forceinline __m64 _mm_shuffle_pi16_1(const __m64 &src) {
        return _mm_shuffle_pi16(src,1);
    }
    static __forceinline __m64 _mm_shuffle_pi16_14(const __m64 &src) {
        return _mm_shuffle_pi16(src,(3 << 2) + 2);
    }
    static __forceinline __m64 _mm_shuffle_pi16_x50(const __m64 &src) {
        return _mm_shuffle_pi16(src,0x50);
    }
    static __forceinline void psadbw(__m64 &mm3,const __m64 &mm2) {
        mm3=_mm_sad_pu8(mm3,mm2);
    }
    static __forceinline __m64 min_pu8(const __m64 &mm1,const __m64 &mm2) {
        return _mm_min_pu8(mm1,mm2);
    }
    static __forceinline __m64 max_pu8(const __m64 &mm1,const __m64 &mm2) {
        return _mm_max_pu8(mm1,mm2);
    }
    static __forceinline __m64 min_pi16(const __m64 &mm1,const __m64 &mm2) {
        return _mm_min_pi16(mm1,mm2);
    }
    static __forceinline __m64 max_pi16(const __m64 &mm1,const __m64 &mm2) {
        return _mm_max_pi16(mm1,mm2);
    }
    static __forceinline void pmaxsw(__m64 &dst,const __m64 &src) {
        dst=_mm_max_pi16(dst,src);
    }
    // store the same width as the register without poluting chache (if supported)
    static __forceinline void movntVq(void *ptr,const __m64 &m)
    {
        _mm_stream_pi((__m64 *)ptr, m);
    }

    MMX_INSTRUCTIONS
};

static __forceinline __m64 _mm_absdif_u8(__m64 mm1,__m64 mm2)
{
    __m64 mm7=mm1;
    mm1=_mm_subs_pu8(mm1,mm2);
    mm2=_mm_subs_pu8(mm2,mm7);
    return _mm_or_si64(mm2,mm1);
}

static __forceinline void memadd(unsigned char *dst,const unsigned char *src,unsigned int len)
{
    __m64 *dst8=(__m64*)dst;
    const __m64 *src8=(__m64*)src;
    for (unsigned int i=0; i<len/8; i++,src8++,dst8++) {
        *dst8=_mm_adds_pu8(*src8,*dst8);
    }
}

//====================================== SSE2 ======================================
#ifdef __SSE2__
struct Tsse2 {
    typedef __m128i __m;
    typedef __m64 int2;
    typedef int64_t integer2_t;
    static const size_t size=sizeof(__m);
    static const int align=16;
    typedef Tmmxext T64;
    static __forceinline __m setzero_si64(void) {
        return _mm_setzero_si128();
    }
    static __forceinline __m set_pi8(char b7,char b6,char b5,char b4,char b3,char b2,char b1,char b0) {
        return _mm_set_epi8(b7,b6,b5,b4,b3,b2,b1,b0,b7,b6,b5,b4,b3,b2,b1,b0);
    }
    static __forceinline __m set_pi32(int i1,int i0) {
        return _mm_set_epi32(i1,i0,i1,i0);
    }
    static __forceinline __m set1_pi8(char b) {
        return _mm_set1_epi8(b);
    }
    static __forceinline __m set1_pi16(short s) {
        return _mm_set1_epi16(s);
    }
    static __forceinline __m set1_pi64(int64_t s) {
        __align16(int64_t,x[])= {s,s};    //__m128i _mm_set1_epi64(*(__m64*)&s); TODO: _mm_set1_epi64x
        return *(__m*)x;
    }
    static __forceinline __m packs_pu16(const __m &m1,const __m &m2) {
        return _mm_packus_epi16(m1,m2);
    }
    static __forceinline __m slli_pi16(const __m &m,int count) {
        return _mm_slli_epi16(m,count);
    }
    static __forceinline __m srli_pi16(const __m &m,int count) {
        return _mm_srli_epi16(m,count);
    }
    static __forceinline __m srli_si64(const __m &m,int count) {
        return _mm_srli_epi64(m,count);
    }
    static __forceinline __m srai_pi16(const __m &m,int count) {
        return _mm_srai_epi16(m,count);
    }
    static __forceinline __m madd_pi16(const __m &m1,const __m &m2) {
        return _mm_madd_epi16(m1,m2);
    }
    static __forceinline __m add_pi16(const __m &m1,const __m &m2) {
        return _mm_add_epi16(m1,m2);
    }
    static __forceinline __m adds_pi16(const __m &m1,const __m &m2) {
        return _mm_adds_epi16(m1,m2);
    }
    static __forceinline __m adds_pu16(const __m &m1,const __m &m2) {
        return _mm_adds_epu16(m1,m2);
    }
    static __forceinline __m adds_pu8(const __m &m1,const __m &m2) {
        return _mm_adds_epu8(m1,m2);
    }
    static __forceinline __m sub_pi16(const __m &m1,const __m &m2) {
        return _mm_sub_epi16(m1,m2);
    }
    static __forceinline __m subs_pi16(const __m &m1,const __m &m2) {
        return _mm_subs_epi16(m1,m2);
    }
    static __forceinline __m subs_pu16(const __m &m1,const __m &m2) {
        return _mm_subs_epu16(m1,m2);
    }
    static __forceinline __m subs_pu8(const __m &m1,const __m &m2) {
        return _mm_subs_epu8(m1,m2);
    }
    static __forceinline __m or_si64(const __m &m1,const __m &m2) {
        return _mm_or_si128(m1,m2);
    }
    static __forceinline __m xor_si64(const __m &m1,const __m &m2) {
        return _mm_xor_si128(m1,m2);
    }
    static __forceinline __m and_si64(const __m &m1,const __m &m2) {
        return _mm_and_si128(m1,m2);
    }
    static __forceinline __m andnot_si64(const __m &m1,const __m &m2) {
        return _mm_andnot_si128(m1,m2);
    }
    static __forceinline __m mullo_pi16(const __m &m1,const __m &m2) {
        return _mm_mullo_epi16(m1,m2);
    }
    static __forceinline __m mulhi_pi16(const __m &m1,const __m &m2) {
        return _mm_mulhi_epi16(m1,m2);
    }
    static __forceinline __m unpacklo_pi8(const __m &m1,const __m &m2) {
        return _mm_unpacklo_epi8(m1,m2);
    }
    static __forceinline __m unpackhi_pi8(const __m &m1,const __m &m2) {
        return _mm_unpackhi_epi8(m1,m2);
    }
    static __forceinline __m cmpgt_pi16(const __m &m1,const __m &m2) {
        return _mm_cmpgt_epi16(m1,m2);
    }
    static __forceinline __m cmpeq_pi16(const __m &m1,const __m &m2) {
        return _mm_cmpeq_epi16(m1,m2);
    }
    static __forceinline __m cmpeq_pi8(const __m &m1,const __m &m2) {
        return _mm_cmpeq_epi8(m1,m2);
    }
    static __forceinline __m min_pi16(const __m &mm1,const __m &mm2) {
        return _mm_min_epi16(mm1,mm2);
    }
    static __forceinline __m max_pi16(const __m &mm1,const __m &mm2) {
        return _mm_max_epi16(mm1,mm2);
    }
    static __forceinline __m load2(const void *ptr) {
        return _mm_loadl_epi64((const __m128i*)ptr);
    }
    static __forceinline void store2(void *ptr,const __m &m) {
        _mm_storel_epi64((__m128i*)ptr,m);
    }
    static __forceinline void empty(void) {
        /*_mm_empty();*/
    }

    static __forceinline void psadbw(__m &mm3,const __m &mm2) {
        mm3=_mm_sad_epu8(mm3,mm2);
    }
    static __forceinline void prefetchnta(const void *ptr) {
        _mm_prefetch((const char*)ptr,_MM_HINT_NTA);
    }
    static __forceinline __m shuffle_pi16_0(const __m &mm0) {
        return _mm_shufflehi_epi16(_mm_shufflelo_epi16(mm0,0),0);
    }
    static __forceinline void pmaxub(__m &mmr1,const __m &mmr2) {
        mmr1=_mm_max_epu8(mmr1,mmr2);
    }
    static __forceinline void pmulhuw(__m &mmr1,const __m &mmr2) {
        mmr1=_mm_mulhi_epu16(mmr1,mmr2);
    }
    static __forceinline void movntq(void *dst,const __m &src) {
        _mm_stream_si128((__m128i*)dst,src);
    }
    static __forceinline void pavgb(__m &mmr1,const __m &mmr2) {
        mmr1=_mm_avg_epu8(mmr1,mmr2);
    }
    static __forceinline void pavgb(__m &mmr1,const void *mmr2) {
        mmr1=_mm_avg_epu8(mmr1,*(__m*)mmr2);
    }
    static __forceinline void sfence(void) {
        _mm_sfence();
    }
    // store the same width as the register without polluting the cache
    static __forceinline void movntVq(void *ptr,const __m128i &m)
    {
        _mm_stream_si128((__m128i*)ptr,m);
    }
};
#endif //__SSE2__

template<class _mm> static __forceinline typename _mm::__m abs_16(const typename _mm::__m &mm0)
{
    typename _mm::__m mm6=_mm::srai_pi16(mm0,15);
    return _mm::sub_pi16(_mm::xor_si64(mm0,mm6),mm6);
}
template<class _mm> static __forceinline typename _mm::__m absdif_s16(typename _mm::__m mm0,typename _mm::__m mm1)
{
    typename _mm::__m mm2=mm0;
    mm0=_mm::cmpgt_pi16(mm0,mm1);
    typename _mm::__m mm4=mm2;
    mm2=_mm::xor_si64(mm2,mm1);
    mm2=_mm::and_si64(mm2,mm0);
    typename _mm::__m mm3=mm2;
    mm4=_mm::xor_si64(mm4,mm2);
    mm1=_mm::xor_si64(mm1,mm3);
    return _mm::sub_pi16(mm1,mm4);
}

#pragma warning(pop)

#endif
