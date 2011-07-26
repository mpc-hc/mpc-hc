#ifndef _CSIMD_H_
#define _CSIMD_H_

#include "simd_common.h"

#pragma warning(push)
#pragma warning(disable:4799)
#pragma warning(disable:4309)
#pragma warning(disable:4700)

namespace csimd
{

#define MMX_INSTRUCTION(instruction,function) \
 static __forceinline void instruction(const __m64     &src,__m64 &dst) {dst=function(dst,          src);} \
 static __forceinline void instruction(const void      *src,__m64 &dst) {dst=function(dst,*(__m64*) src);} \
 static __forceinline void instruction(const long long &src,__m64 &dst) {dst=function(dst,*(__m64*)&src);}

#define SSE2I_INSTRUCTION(instruction,function) \
 static __forceinline void instruction(const __m128i &src,__m128i &dst) {dst=function(dst,           src);} \
 static __forceinline void instruction(const void    *src,__m128i &dst) {dst=function(dst,*(__m128i*)src);}

#include "simd_instructions.h"

#undef MMX_INSTRUCTION
#undef SSE2I_INSTRUCTION

    static __forceinline void movq(const __m64 &src,__m64 &dst)
    {
        dst=src;
    }
    static __forceinline void movq(const void *src ,__m64 &dst)
    {
        dst=*(__m64*)src;
    }
    static __forceinline void movq(const long long &src ,__m64 &dst)
    {
        dst=*(__m64*)&src;
    }
    static __forceinline void movq(const __m64 &src,void *dst )
    {
        *(__m64*)dst=src;
    }

    static __forceinline void movd(int src         ,__m64 &dst)
    {
        dst=_mm_cvtsi32_si64(src);
    }
    static __forceinline void movd(const __m64 &src,int &dst  )
    {
        dst=_mm_cvtsi64_si32(src);
    }
    static __forceinline void movd(const void *src ,__m64 &dst)
    {
        dst=_mm_cvtsi32_si64(*(const int*)src);
    }
    static __forceinline void movd(const __m64 &src,void *dst )
    {
        *(int*)dst=_mm_cvtsi64_si32(src);
    }

    static __forceinline void psllw(int i,__m64 &dst)
    {
        dst=_mm_slli_pi16(dst,i);
    }
    static __forceinline void psrlw(int i,__m64 &dst)
    {
        dst=_mm_srli_pi16(dst,i);
    }
    static __forceinline void pslld(int i,__m64 &dst)
    {
        dst=_mm_slli_pi32(dst,i);
    }
    static __forceinline void psrld(int i,__m64 &dst)
    {
        dst=_mm_srli_pi32(dst,i);
    }
    static __forceinline void psrad(int i,__m64 &dst)
    {
        dst=_mm_srai_pi32(dst,i);
    }
    static __forceinline void psraw(int i,__m64 &dst)
    {
        dst=_mm_srai_pi16(dst,i);
    }
    static __forceinline void psraw(const __m64 &src,__m64 &dst)
    {
        dst=_mm_sra_pi16(dst,src);
    }
    static __forceinline void psrlq(int i,__m64 &dst)
    {
        dst=_mm_srli_si64(dst,i);
    }

    static __forceinline void movlps(const void *src,__m128 &dst)
    {
        dst=_mm_loadl_pi(dst,(__m64*)src);
    }
    static __forceinline void movhps(const void *src,__m128 &dst)
    {
        dst=_mm_loadh_pi(dst,(__m64*)src);
    }
    static __forceinline void movlps(const __m128 &src,void *dst)
    {
        _mm_storel_pi((__m64*)dst,src);
    }
    static __forceinline void movhps(const __m128 &src,void *dst)
    {
        _mm_storeh_pi((__m64*)dst,src);
    }
    static __forceinline void movaps(const void *src,__m128 &dst)
    {
        dst=_mm_load_ps((const float*)src);
    }
    static __forceinline void movaps(const __m128 &src,void *dst)
    {
        _mm_store_ps((float*)dst,src);
    }
    static __forceinline void movaps(const __m128 &src,__m128 &dst)
    {
        dst=src;
    }
    static __forceinline void mulps(const __m128 &src,__m128 &dst)
    {
        dst=_mm_mul_ps(dst,src);
    }
    static __forceinline void mulps(const void *src,__m128 &dst)
    {
        dst=_mm_mul_ps(dst,*(__m128*)src);
    }
    static __forceinline void addps(const __m128 &src,__m128 &dst)
    {
        dst=_mm_add_ps(dst,src);
    }
    static __forceinline void addps(const void *src,__m128 &dst)
    {
        dst=_mm_add_ps(dst,*(__m128*)src);
    }
    static __forceinline void subps(const __m128 &src,__m128 &dst)
    {
        dst=_mm_sub_ps(dst,src);
    }
    static __forceinline void xorps(const __m128 &src,__m128 &dst)
    {
        dst=_mm_xor_ps(dst,src);
    }
    static __forceinline void movss(const float &src,__m128 &dst)
    {
        dst=_mm_load_ss(&src);
    }

#ifdef __SSE2__
    static __forceinline void movdqu(const void *src,__m128i &dst)
    {
        dst=_mm_loadu_si128((__m128i*)src);
    }
    static __forceinline void movdqu(const __m128i &src,__m128i &dst)
    {
        dst=_mm_loadu_si128(&src);
    }
    static __forceinline void movdqu(const __m128i &src,void *dst)
    {
        _mm_storeu_si128((__m128i*)dst,src);
    }
    static __forceinline void psraw(int i,__m128i &dst)
    {
        dst=_mm_srai_epi16(dst,i);
    }
    //static __forceinline void pslldq(int i,__m128i &dst) {dst=_mm_slli_si128(dst,i);}
    //static __forceinline void psrldq(int i,__m128i &dst) {dst=_mm_srli_si128(dst,i);}
#endif

}

#pragma warning(pop)

#endif
