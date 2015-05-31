/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef ZenLib_MemoryUtilsH
#define ZenLib_MemoryUtilsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
#include "ZenLib/Conf.h"
//---------------------------------------------------------------------------

#include <cstring>
#ifdef ZENLIB_MEMUTILS_SSE2
    #include <emmintrin.h>
#endif //ZENLIB_MEMUTILS_SSE2

namespace ZenLib
{

#ifndef ZENLIB_MEMUTILS_SSE2
    //-----------------------------------------------------------------------
    // Memory alloc/free
    #define malloc_Aligned128 (size) \
            malloc (size)
    #define free_Aligned128 (ptr) \
            free (ptr)

    //-----------------------------------------------------------------------
    // Arbitrary size - To Unaligned
    #define memcpy_Unaligned_Unaligned memcpy
    #define memcpy_Aligned128_Unaligned memcpy

    //-----------------------------------------------------------------------
    // Arbitrary size - To Aligned 128 bits (16 bytes)
    #define memcpy_Unaligned_Aligned128 memcpy
    #define memcpy_Aligned128_Aligned128 memcpy

    //-----------------------------------------------------------------------
    // 128 bits - To Unaligned
    #define memcpy_Unaligned_Unaligned_Once128 memcpy

    //-----------------------------------------------------------------------
    // 128 bits - To Aligned 128 bits (16 bytes)
    #define memcpy_Aligned128_Aligned128_Once128 memcpy

    //-----------------------------------------------------------------------
    // 1024 bits - To Unaligned
    #define memcpy_Unaligned_Unaligned_Once1024 memcpy

    //-----------------------------------------------------------------------
    // 1024 bits - To Aligned 128 bits (16 bytes)
    #define memcpy_Aligned128_Aligned128_Once1024 memcpy

    //-----------------------------------------------------------------------
    // 128-bit multiple - To Aligned 128 bits (16 bytes)
    #define memcpy_Unaligned_Aligned128_Size128 memcpy
    #define memcpy_Aligned128_Aligned128_Size128 memcpy

#else // ZENLIB_MEMUTILS_SSE2

    //-----------------------------------------------------------------------
    // Memory alloc/free

    inline void*    malloc_Aligned128 (size_t size)
    {
        return _aligned_malloc (size, 16); //aligned_alloc in C11
    }

    inline void     free_Aligned128 ( void *ptr )
    {
        _aligned_free (ptr); //free in C11
    }

    //-----------------------------------------------------------------------
    // Arbitrary size - To Unaligned

    inline void memcpy_Unaligned_Unaligned (void* destination, const void* source, size_t num)
    {
        size_t extra=num&0xF;
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(const __m128i*)source;

        num>>=4;
        while (num--)
            _mm_storeu_si128 (destination16++, _mm_loadu_si128(source16++));

        char* destination1=(char*)destination16;
        char* source1=(char*)source16;
        while (extra--)
            *destination1++=*source1++;
    }

    inline void memcpy_Aligned128_Unaligned (void* destination, const void* source, size_t num)
    {
        size_t extra=num&0xF;
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(const __m128i*)source;

        num>>=4;
        while (num--)
            _mm_storeu_si128 (destination16++, _mm_load_si128(source16++));

        char* destination1=(char*)destination16;
        char* source1=(char*)source16;
        while (extra--)
            *destination1++=*source1++;
    }

    //-----------------------------------------------------------------------
    // Arbitrary size - To Aligned 128 bits (16 bytes)

    inline void memcpy_Unaligned_Aligned128 (void* destination, const void* source, size_t num)
    {
        size_t extra=num&0xF;
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(const __m128i*)source;

        num>>=4;
        while (num--)
            _mm_stream_si128 (destination16++, _mm_loadu_si128(source16++));

        char* destination1=(char*)destination16;
        char* source1=(char*)source16;
        while (extra--)
            *destination1++=*source1++;
    }

    //-----------------------------------------------------------------------
    // 128 bits - To Unaligned

    inline void memcpy_Unaligned_Unaligned_Once128 (void* destination, const void* source)
    {
        _mm_storeu_si128 ((__m128i*)destination, _mm_loadu_si128((const __m128i*)source));
    }

    //-----------------------------------------------------------------------
    // 128 bits - To Aligned 128 bits (16 bytes)

    inline void memcpy_Aligned128_Aligned128 (void* destination, const void* source, size_t num)
    {
        size_t extra=num&0xF;
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(const __m128i*)source;

        num>>=4;
        while (num--)
            _mm_stream_si128 (destination16++, _mm_load_si128(source16++));

        char* destination1=(char*)destination16;
        char* source1=(char*)source16;
        while (extra--)
            *destination1++=*source1++;
    }

    inline void memcpy_Aligned128_Aligned128_Size128 (void* destination, const void* source, size_t num)
    {
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(__m128i*)source;

        num>>=4;
        while (num--)
            _mm_stream_si128 (destination16++, _mm_load_si128(source16++));
    }

    //-----------------------------------------------------------------------
    // 1024 bits - To Unaligned

    inline void memcpy_Unaligned_Unaligned_Once1024 (void* destination, const void* source, size_t)
    {
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(__m128i*)source;

        size_t num=8;
        while (num--)
            _mm_storeu_si128 (destination16++, _mm_loadu_si128(source16++));
    }

    //-----------------------------------------------------------------------
    // 1024 bits - To Aligned 128 bits (16 bytes)

    inline void memcpy_Aligned128_Aligned128_Once128 (void* destination, const void* source)
    {
        _mm_stream_si128 ((__m128i*)destination, _mm_load_si128((const __m128i*)source));
    }

    //-----------------------------------------------------------------------
    // 128-bit multiple - To Unaligned (16 bytes)

    inline void memcpy_Unaligned_Unaligned_Size128 (void* destination, const void* source, size_t num)
    {
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(const __m128i*)source;

        num>>=4;
        while (num--)
            _mm_storeu_si128 (destination16++, _mm_loadu_si128(source16++));
    }

    inline void memcpy_Aligned128_Unaligned_Size128 (void* destination, const void* source, size_t num)
    {
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(__m128i*)source;

        num>>=4;
        while (num--)
            _mm_storeu_si128 (destination16++, _mm_load_si128(source16++));
    }

    //-----------------------------------------------------------------------
    // 128-bit multiple - To Aligned 128 bits (16 bytes)

    inline void memcpy_Unaligned_Aligned128_Size128 (void* destination, const void* source, size_t num)
    {
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(__m128i*)source;

        num>>=4;
        while (num--)
            _mm_stream_si128 (destination16++, _mm_loadu_si128(source16++));
    }


    /* Slower
    inline void memcpy_Aligned128_Aligned128_Once1024 (void* destination, const void* source)
    {
        __m128i* destination16=(__m128i*)destination;
        const __m128i* source16=(__m128i*)source;

        size_t num=8;
        while (num--)
            _mm_stream_si128 (destination16++, _mm_load_si128(source16++));
    }
    */

    /*
    inline void memcpy_Aligned256_Aligned256 (void* destination, const void* source, size_t num) //with AVX, actually slower
    {
        size_t extra=num&0x1F;
        __m256i* destination16=(__m256i*)destination;
        const __m256i* source16=(const __m256i*)source;

        num>>=5;
        while (num--)
            _mm256_storeu_si256 (destination16++, _mm256_loadu_si256(source16++));

        char* destination1=(char*)destination16;
        char* source1=(char*)source16;
        while (extra--)
            *destination1++=*source1++;
    }
    */

#endif // ZENLIB_MEMUTILS_SSE2

} //NameSpace

#endif
