#if defined(__GNUC__)
  #define HAVE_MMX 1
  #define HAVE_SSSE3 1

  #define ARCH_X86 1

  #ifndef ARCH_X86_64
    #define ARCH_X86_32 1
  #endif

  #ifdef ARCH_X86_64
    #define HAVE_FAST_64BIT 1
  #endif
#endif

#define HAVE_MALLOC_H 1
#define HAVE_LRINTF 1
#define SIMPLE_IDCT 1
#define CONFIG_ZLIB 1
#define HAVE_W32THREADS 1
#define HAVE_THREADS 1
#define HAVE_MEMALIGN 1
#define ASMALIGN(ZEROBITS) ".align 1<<" #ZEROBITS "\n\t"
#define HAVE_BSWAP 1

#define CONFIG_DECODERS 1

// == LibAVFormat
#define CONFIG_DEMUXERS 1
#define CONFIG_AC3_DEMUXER 1
// ==

#ifndef __GNUC__
  #define EMULATE_FAST_INT
#endif

#define HAVE_7REGS
#define HAVE_EBX_AVAILABLE

/* CPU specific */
//#define HAVE_FAST_CMOV
