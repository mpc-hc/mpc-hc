#include "../libswscale/config.h"
#ifdef USE_FASTMEMCPY

/*
  aclib - advanced C library ;)
  This file contains functions which improve and expand standard C-library
  see aclib_template.c ... this file only contains runtime cpu detection and config options stuff
  runtime cpu detection by michael niedermayer (michaelni@gmx.at) is under GPL
*/
#include <string.h>
#include <stddef.h>
#include "../libavutil/x86_cpu.h"
#include "../cpudetect.h"
#include "fastmemcpy.h"
#undef memcpy
#include "ffImgfmt.h"

#define BLOCK_SIZE 4096
#define CONFUSION_FACTOR 0
//Feel free to fine-tune the above 2, it might be possible to get some speedup with them :)

//#define STATISTICS
#if ARCH_X86_32 || ARCH_X86_64
#define CAN_COMPILE_X86_ASM
#endif

//Note: we have MMX, MMX2, 3DNOW version there is no 3DNOW+MMX2 one
//Plain C versions
//#if !HAVE_MMX || defined (RUNTIME_CPUDETECT)
//#define COMPILE_C
//#endif

#ifdef CAN_COMPILE_X86_ASM

#if (HAVE_MMX && !HAVE_AMD3DNOW && !HAVE_MMX2) || defined (RUNTIME_CPUDETECT)
#define COMPILE_MMX
#endif

#if (HAVE_MMX2 && !HAVE_SSE2) || defined (RUNTIME_CPUDETECT)
#define COMPILE_MMX2
#endif

#if (HAVE_AMD3DNOW && !HAVE_MMX2) || defined (RUNTIME_CPUDETECT)
#define COMPILE_3DNOW
#endif

#if HAVE_SSE2 || defined (RUNTIME_CPUDETECT)
#define COMPILE_SSE
#endif

#undef HAVE_MMX
#undef HAVE_MMX2
#undef HAVE_AMD3DNOW
#undef HAVE_SSE
#undef HAVE_SSE2
/*
#ifdef COMPILE_C
#undef HAVE_MMX
#undef HAVE_MMX2
#undef HAVE_AMD3DNOW
#undef ARCH_X86
#define RENAME(a) a ## _C
#include "aclib_template.c"
#endif
*/
//MMX versions
#ifdef COMPILE_MMX
#undef RENAME
#define HAVE_MMX 1
#undef HAVE_MMX2
#undef HAVE_AMD3DNOW
#undef HAVE_SSE
#undef HAVE_SSE2
#define RENAME(a) a ## _MMX
#include "aclib_template.c"
#endif

//MMX2 versions
#ifdef COMPILE_MMX2
#undef RENAME
#define HAVE_MMX 1
#define HAVE_MMX2 1
#undef HAVE_AMD3DNOW
#undef HAVE_SSE
#undef HAVE_SSE2
#define RENAME(a) a ## _MMX2
#include "aclib_template.c"
#endif

//3DNOW versions
#ifdef COMPILE_3DNOW
#undef RENAME
#define HAVE_MMX 1
#undef HAVE_MMX2
#define HAVE_AMD3DNOW 1
#undef HAVE_SSE
#undef HAVE_SSE2
#define RENAME(a) a ## _3DNow
#include "aclib_template.c"
#endif

//SSE versions (only used on SSE2 cpus)
#ifdef COMPILE_SSE
#undef RENAME
#define HAVE_MMX 1
#define HAVE_MMX2 1
#undef HAVE_AMD3DNOW
#define HAVE_SSE
#define HAVE_SSE2
#define RENAME(a) a ## _SSE
#include "aclib_template.c"
#endif

#endif // CAN_COMPILE_X86_ASM

void*(*fast_memcpy)(void * to, const void * from, size_t len) = NULL;

void init_fast_memcpy(void)
{
#ifdef RUNTIME_CPUDETECT
#ifdef CAN_COMPILE_X86_ASM
    // ordered per speed fasterst first
    if(gCpuCaps.hasSSE2)
        fast_memcpy = fast_memcpy_SSE;
    else if(gCpuCaps.hasMMX2)
        fast_memcpy = fast_memcpy_MMX2;
    else if(gCpuCaps.has3DNow)
        fast_memcpy = fast_memcpy_3DNow;
    else if(gCpuCaps.hasMMX)
        fast_memcpy = fast_memcpy_MMX;
    else
#endif //CAN_COMPILE_X86_ASM
        fast_memcpy = memcpy; // prior to mmx we use the standart memcpy
#else
#if HAVE_SSE2
    fast_memcpy = fast_memcpy_SSE;
#elif HAVE_MMX2
    fast_memcpy = fast_memcpy_MMX2;
#elif HAVE_AMD3DNOW
    fast_memcpy = fast_memcpy_3DNow;
#elif HAVE_MMX
    fast_memcpy = fast_memcpy_MMX;
#else
    fast_memcpy = memcpy; // prior to mmx we use the standart memcpy
#endif

#endif //!RUNTIME_CPUDETECT
}

#endif /* use fastmemcpy */

