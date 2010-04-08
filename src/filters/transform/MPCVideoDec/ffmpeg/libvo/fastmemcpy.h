#ifndef __MPLAYER_MEMCPY
#define __MPLAYER_MEMCPY

#include <stddef.h>
#include "../libswscale/config.h"
#include "../libavutil/internal.h"

#if defined(USE_FASTMEMCPY) && (HAVE_MMX || HAVE_MMX2 || HAVE_AMD3DNOW || HAVE_SSE || HAVE_SSE2)
extern void*(*fast_memcpy)(void * to, const void * from, size_t len);
#define memcpy(a,b,c) fast_memcpy(a,b,c)
#endif
void init_fast_memcpy(void);

static inline void * memcpy_pic(unsigned char * dst, unsigned char * src, int bytesPerLine, int height, int dstStride, int srcStride)
{
    int i;
    void *retval = dst;

    if(dstStride == srcStride)
    {
        if(srcStride < 0)
        {
            src += (height - 1) * srcStride;
            dst += (height - 1) * dstStride;
            srcStride = -srcStride;
        }

        memcpy(dst, src, srcStride * height);
    }
    else
    {
        for(i = 0; i < height; i++)
        {
            memcpy(dst, src, bytesPerLine);
            src += srcStride;
            dst += dstStride;
        }
    }

    return retval;
}

#endif
