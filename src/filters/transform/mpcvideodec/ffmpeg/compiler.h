#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifndef STRINGIFY
 #define STRINGIFY(s) TOSTRING(s)
 #define TOSTRING(s) #s
#endif

#if defined(__INTEL_COMPILER)
  #if __INTEL_COMPILER  >= 1000
    #define COMPILER "icl 10"
  #elif __INTEL_COMPILER  >= 900
    #define COMPILER "icl 9"
  #elif __INTEL_COMPILER  >= 800
    #define COMPILER "icl 8"
  #else
    #define COMPILER "icl"
  #endif
#elif defined(_MSC_VER)
  #if _MSC_VER>=1400
    #define COMPILER "msvc 2005"
  #elif _MSC_VER>=1300
    #define COMPILER "msvc 2003"
  #else
    #define COMPILER "msvc 6"
  #endif
#elif defined(__GNUC__)
  #ifdef __SSE__
    #define COMPILER_SSE " sse"
    #ifdef __SSE2__
      #define COMPILER_SSE2 ",sse2"
    #else
      #define COMPILER_SSE2 ""
    #endif
  #else
    #define COMPILER_SSE ""
    #define COMPILER_SSE2 ""
  #endif
  #define COMPILER "gcc "STRINGIFY(__GNUC__)"."STRINGIFY(__GNUC_MINOR__)"."STRINGIFY(__GNUC_PATCHLEVEL__) COMPILER_SSE COMPILER_SSE2
#else
  #define COMPILER "unknown"
#endif

#ifdef WIN64
  #define COMPILER_X64 ", x64"
#else
  #define COMPILER_X64 ", x86"
#endif

#endif
