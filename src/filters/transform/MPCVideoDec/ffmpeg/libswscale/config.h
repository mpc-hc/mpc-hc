#ifdef __GNUC__
  #define HAVE_MMX 1
  #define HAVE_MMX2 1
  #define HAVE_SSE 1
  #define HAVE_SSE2 1
  #define HAVE_SSSE3 1
  #define HAVE_AMD3DNOW 1
  #define HAVE_AMD3DNOWEX 1	
	
  #define ARCH_X86 1
  #ifdef ARCH_X86_64
    #define HAVE_FAST_64BIT 1
    #define HAVE_CMOV 1
    #define HAVE_FAST_CMOV 1
  #else
    #define ARCH_X86_32 1
  #endif
#else
	#define HAVE_INLINE_ASM 0
	#define HAVE_MMX 0
	#define HAVE_MMX2 0
	#define HAVE_SSE 0
  #define HAVE_SSE2 0
	#define HAVE_SSSE3 0
	#define HAVE_AMD3DNOW 0
	#define HAVE_AMD3DNOWEXT 0
	#define ARCH_X86 0
	#define ARCH_X86_32 0
	#define ARCH_X86_64 0
	#define HAVE_FAST_64BIT 0
  #define HAVE_CMOV 0
  #define HAVE_FAST_CMOV 0
#endif

#define HAVE_TEN_OPERANDS 1
#define HAVE_EBP_AVAILABLE 1
#define HAVE_EBX_AVAILABLE 1
#define NAMED_ASM_ARGS 1

#define HAVE_ALLOCA_H 1
#define HAVE_BSWAP 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMALIGN 1
#define HAVE_THREADS 1
#define RUNTIME_CPUDETECT 1
#define USE_FASTMEMCPY 1
#define CONFIG_SWSCALE_ALPHA 1

#define ASMALIGN(ZEROBITS) ".align 1 << " #ZEROBITS "\n\t"

#define CONFIG_GPL 1

/* Toggles debugging informations */
#undef MP_DEBUG

#ifndef __GNUC__
 #define inline __inline
 #ifndef __attribute__
  #define __attribute__(x) /**/
 #endif
 #pragma warning (disable:4002)
 #include <malloc.h>
 #define memalign(a,b) _aligned_malloc(b,a)
#else
 #define memalign(a,b) __mingw_aligned_malloc(b,a)
#endif
