/*
 * software RGB to RGB converter
 * pluralize by software PAL8 to RGB converter
 *              software YUV to YUV converter
 *              software YUV to RGB converter
 * Written by Nick Kurshev.
 * palette & YUV & runtime CPU stuff by Michael (michaelni@gmx.at)
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * The C code (not assembly, MMX, ...) of this file can be used
 * under the LGPL license.
 */
#include <inttypes.h>
#include "../config.h"
#include "../libavutil/internal.h"
#include "../libavutil/x86_cpu.h"
#include "../libavutil/bswap.h"
#include "../libvo/fastmemcpy.h"
#include "../cpudetect.h"
#include "rgb2rgb.h"
#include "swscale.h"
#include "swscale_internal.h"

#define FAST_BGR2YV12 // use 7-bit instead of 15-bit coefficients

void (*rgb24to32)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb24to16)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb24to15)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb32to24)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb32to16)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb32to15)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb15to16)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb15to24)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb15to32)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb16to15)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb16to24)(const uint8_t *src,uint8_t *dst,stride_t src_size);
void (*rgb16to32)(const uint8_t *src,uint8_t *dst,stride_t src_size);
//void (*rgb24tobgr32)(const uint8_t *src, uint8_t *dst, stride_t src_size);
void (*rgb24tobgr24)(const uint8_t *src, uint8_t *dst, stride_t src_size);
void (*rgb24tobgr16)(const uint8_t *src, uint8_t *dst, stride_t src_size);
void (*rgb24tobgr15)(const uint8_t *src, uint8_t *dst, stride_t src_size);
void (*rgb32tobgr32)(const uint8_t *src, uint8_t *dst, stride_t src_size);
//void (*rgb32tobgr24)(const uint8_t *src, uint8_t *dst, stride_t src_size);
void (*rgb32tobgr16)(const uint8_t *src, uint8_t *dst, stride_t src_size);
void (*rgb32tobgr15)(const uint8_t *src, uint8_t *dst, stride_t src_size);

void (*yv12toyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t dstStride);
void (*yv12touyvy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t dstStride);
void (*yv12tovyuy)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t dstStride);
void (*yv12toyvyu)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t dstStride);
void (*yuv422ptoyuy2)(const uint8_t *ysrc, const uint8_t *usrc, const uint8_t *vsrc, uint8_t *dst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t dstStride);
void (*yuy2toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t srcStride);
void (*rgb24toyv12)(const uint8_t *src, uint8_t *ydst, uint8_t *udst, uint8_t *vdst,
	long width, long height,
	stride_t lumStride, stride_t chromStride, stride_t srcStride);
void (*planar2x)(const uint8_t *src, uint8_t *dst, long width, long height,
	stride_t srcStride, stride_t dstStride);
void (*interleaveBytes)(uint8_t *src1, uint8_t *src2, uint8_t *dst,
			    long width, long height, stride_t src1Stride,
			    stride_t src2Stride, stride_t dstStride);
void (*vu9_to_vu12)(const uint8_t *src1, const uint8_t *src2,
			uint8_t *dst1, uint8_t *dst2,
			long width, long height,
			stride_t srcStride1, stride_t srcStride2,
			stride_t dstStride1, stride_t dstStride2);
void (*yvu9_to_yuy2)(const uint8_t *src1, const uint8_t *src2, const uint8_t *src3,
			uint8_t *dst,
			long width, long height,
			stride_t srcStride1, stride_t srcStride2,
			stride_t srcStride3, stride_t dstStride);

#if ARCH_X86 && CONFIG_GPL
DECLARE_ASM_CONST(8, uint64_t, mmx_null)     = 0x0000000000000000ULL;
DECLARE_ASM_CONST(8, uint64_t, mmx_one)      = 0xFFFFFFFFFFFFFFFFULL;
DECLARE_ASM_CONST(8, uint64_t, mask32b)      = 0x000000FF000000FFULL;
DECLARE_ASM_CONST(8, uint64_t, mask32g)      = 0x0000FF000000FF00ULL;
DECLARE_ASM_CONST(8, uint64_t, mask32r)      = 0x00FF000000FF0000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask32a)      = 0xFF000000FF000000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask32)       = 0x00FFFFFF00FFFFFFULL;
DECLARE_ASM_CONST(8, uint64_t, mask3216br)   = 0x00F800F800F800F8ULL;
DECLARE_ASM_CONST(8, uint64_t, mask3216g)    = 0x0000FC000000FC00ULL;
DECLARE_ASM_CONST(8, uint64_t, mask3215g)    = 0x0000F8000000F800ULL;
DECLARE_ASM_CONST(8, uint64_t, mul3216)      = 0x2000000420000004ULL;
DECLARE_ASM_CONST(8, uint64_t, mul3215)      = 0x2000000820000008ULL;
DECLARE_ASM_CONST(8, uint64_t, mask24b)      = 0x00FF0000FF0000FFULL;
DECLARE_ASM_CONST(8, uint64_t, mask24g)      = 0xFF0000FF0000FF00ULL;
DECLARE_ASM_CONST(8, uint64_t, mask24r)      = 0x0000FF0000FF0000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask24l)      = 0x0000000000FFFFFFULL;
DECLARE_ASM_CONST(8, uint64_t, mask24h)      = 0x0000FFFFFF000000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask24hh)     = 0xffff000000000000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask24hhh)    = 0xffffffff00000000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask24hhhh)   = 0xffffffffffff0000ULL;
DECLARE_ASM_CONST(8, uint64_t, mask15b)      = 0x001F001F001F001FULL; /* 00000000 00011111  xxB */
DECLARE_ASM_CONST(8, uint64_t, mask15rg)     = 0x7FE07FE07FE07FE0ULL; /* 01111111 11100000  RGx */
DECLARE_ASM_CONST(8, uint64_t, mask15s)      = 0xFFE0FFE0FFE0FFE0ULL;
DECLARE_ASM_CONST(8, uint64_t, mask15g)      = 0x03E003E003E003E0ULL;
DECLARE_ASM_CONST(8, uint64_t, mask15r)      = 0x7C007C007C007C00ULL;
#define mask16b mask15b
DECLARE_ASM_CONST(8, uint64_t, mask16g)      = 0x07E007E007E007E0ULL;
DECLARE_ASM_CONST(8, uint64_t, mask16r)      = 0xF800F800F800F800ULL;
DECLARE_ASM_CONST(8, uint64_t, red_16mask)   = 0x0000f8000000f800ULL;
DECLARE_ASM_CONST(8, uint64_t, green_16mask) = 0x000007e0000007e0ULL;
DECLARE_ASM_CONST(8, uint64_t, blue_16mask)  = 0x0000001f0000001fULL;
DECLARE_ASM_CONST(8, uint64_t, red_15mask)   = 0x00007c0000007c00ULL;
DECLARE_ASM_CONST(8, uint64_t, green_15mask) = 0x000003e0000003e0ULL;
DECLARE_ASM_CONST(8, uint64_t, blue_15mask)  = 0x0000001f0000001fULL;

#ifdef FAST_BGR2YV12
static const uint64_t bgr2YCoeff  attribute_used __attribute__((aligned(8))) = 0x000000210041000DULL;
static const uint64_t bgr2UCoeff  attribute_used __attribute__((aligned(8))) = 0x0000FFEEFFDC0038ULL;
static const uint64_t bgr2VCoeff  attribute_used __attribute__((aligned(8))) = 0x00000038FFD2FFF8ULL;
#else
static const uint64_t bgr2YCoeff  attribute_used __attribute__((aligned(8))) = 0x000020E540830C8BULL;
static const uint64_t bgr2UCoeff  attribute_used __attribute__((aligned(8))) = 0x0000ED0FDAC23831ULL;
static const uint64_t bgr2VCoeff  attribute_used __attribute__((aligned(8))) = 0x00003831D0E6F6EAULL;
#endif
static const uint64_t bgr2YOffset attribute_used __attribute__((aligned(8))) = 0x1010101010101010ULL;
static const uint64_t bgr2UVOffset attribute_used __attribute__((aligned(8)))= 0x8080808080808080ULL;
static const uint64_t w1111       attribute_used __attribute__((aligned(8))) = 0x0001000100010001ULL;

#endif /* ARCH_X86 */

#define RGB2YUV_SHIFT 8
#define BY ((int)( 0.098*(1<<RGB2YUV_SHIFT)+0.5))
#define BV ((int)(-0.071*(1<<RGB2YUV_SHIFT)+0.5))
#define BU ((int)( 0.439*(1<<RGB2YUV_SHIFT)+0.5))
#define GY ((int)( 0.504*(1<<RGB2YUV_SHIFT)+0.5))
#define GV ((int)(-0.368*(1<<RGB2YUV_SHIFT)+0.5))
#define GU ((int)(-0.291*(1<<RGB2YUV_SHIFT)+0.5))
#define RY ((int)( 0.257*(1<<RGB2YUV_SHIFT)+0.5))
#define RV ((int)( 0.439*(1<<RGB2YUV_SHIFT)+0.5))
#define RU ((int)(-0.148*(1<<RGB2YUV_SHIFT)+0.5))

//Note: We have C, MMX, MMX2, 3DNOW versions, there is no 3DNOW + MMX2 one.
//plain C versions
#undef HAVE_MMX
#undef HAVE_MMX2
#undef HAVE_AMD3DNOW
#undef HAVE_SSE2
#define RENAME(a) a ## _C
#include "rgb2rgb_template.c"

#if ARCH_X86 && CONFIG_GPL

//MMX versions
#undef RENAME
#define HAVE_MMX 1
#undef HAVE_MMX2
#undef HAVE_AMD3DNOW
#undef HAVE_SSE2
#define RENAME(a) a ## _MMX
#include "rgb2rgb_template.c"

//MMX2 versions
#undef RENAME
#define HAVE_MMX 1
#define HAVE_MMX2 1
#undef HAVE_AMD3DNOW
#undef HAVE_SSE2
#define RENAME(a) a ## _MMX2
#include "rgb2rgb_template.c"

//3DNOW versions
#undef RENAME
#define HAVE_MMX 1
#undef HAVE_MMX2
#define HAVE_AMD3DNOW 1
#undef HAVE_SSE2
#define RENAME(a) a ## _3DNOW
#include "rgb2rgb_template.c"

#endif //ARCH_X86_32 || ARCH_X86_64

/*
 RGB15->RGB16 original by Strepto/Astral
 ported to gcc & bugfixed : A'rpi
 MMX2, 3DNOW optimization by Nick Kurshev
 32-bit C version, and and&add trick by Michael Niedermayer
*/

void sws_rgb2rgb_init(SwsParams *params){
		yv12toyvyu= yv12toyvyu_C;
		yv12tovyuy= yv12tovyuy_C;
#if ARCH_X86_32 || ARCH_X86_64
	if(params->cpu & SWS_CPU_CAPS_MMX2){
		rgb15to16= rgb15to16_MMX2;
		rgb15to24= rgb15to24_MMX2;
		rgb15to32= rgb15to32_MMX2;
		rgb16to24= rgb16to24_MMX2;
		rgb16to32= rgb16to32_MMX2;
		rgb16to15= rgb16to15_MMX2;
		rgb24to16= rgb24to16_MMX2;
		rgb24to15= rgb24to15_MMX2;
		rgb24to32= rgb24to32_MMX2;
		rgb32to16= rgb32to16_MMX2;
		rgb32to15= rgb32to15_MMX2;
		rgb32to24= rgb32to24_MMX2;
		rgb24tobgr15= rgb24tobgr15_MMX2;
		rgb24tobgr16= rgb24tobgr16_MMX2;
		rgb24tobgr24= rgb24tobgr24_MMX2;
		rgb32tobgr32= rgb32tobgr32_MMX2;
		rgb32tobgr16= rgb32tobgr16_MMX2;
		rgb32tobgr15= rgb32tobgr15_MMX2;
		yv12toyuy2= yv12toyuy2_MMX2;
		yv12touyvy= yv12touyvy_MMX2;
		yuv422ptoyuy2= yuv422ptoyuy2_MMX2;
		yuy2toyv12= yuy2toyv12_MMX2;
//		uyvytoyv12= uyvytoyv12_MMX2;
//		yvu9toyv12= yvu9toyv12_MMX2;
		planar2x= planar2x_MMX2;
		rgb24toyv12= rgb24toyv12_MMX2;
		interleaveBytes= interleaveBytes_MMX2;
		vu9_to_vu12= vu9_to_vu12_MMX2;
		yvu9_to_yuy2= yvu9_to_yuy2_MMX2;
	}else if(params->cpu & SWS_CPU_CAPS_3DNOW){
		rgb15to16= rgb15to16_3DNOW;
		rgb15to24= rgb15to24_3DNOW;
		rgb15to32= rgb15to32_3DNOW;
		rgb16to24= rgb16to24_3DNOW;
		rgb16to32= rgb16to32_3DNOW;
		rgb16to15= rgb16to15_3DNOW;
		rgb24to16= rgb24to16_3DNOW;
		rgb24to15= rgb24to15_3DNOW;
		rgb24to32= rgb24to32_3DNOW;
		rgb32to16= rgb32to16_3DNOW;
		rgb32to15= rgb32to15_3DNOW;
		rgb32to24= rgb32to24_3DNOW;
		rgb24tobgr15= rgb24tobgr15_3DNOW;
		rgb24tobgr16= rgb24tobgr16_3DNOW;
		rgb24tobgr24= rgb24tobgr24_3DNOW;
		rgb32tobgr32= rgb32tobgr32_3DNOW;
		rgb32tobgr16= rgb32tobgr16_3DNOW;
		rgb32tobgr15= rgb32tobgr15_3DNOW;
		yv12toyuy2= yv12toyuy2_3DNOW;
		yv12touyvy= yv12touyvy_3DNOW;
		yuv422ptoyuy2= yuv422ptoyuy2_3DNOW;
		yuy2toyv12= yuy2toyv12_3DNOW;
//		uyvytoyv12= uyvytoyv12_3DNOW;
//		yvu9toyv12= yvu9toyv12_3DNOW;
		planar2x= planar2x_3DNOW;
		rgb24toyv12= rgb24toyv12_3DNOW;
		interleaveBytes= interleaveBytes_3DNOW;
		vu9_to_vu12= vu9_to_vu12_3DNOW;
		yvu9_to_yuy2= yvu9_to_yuy2_3DNOW;
	}else if(params->cpu & SWS_CPU_CAPS_MMX){
		rgb15to16= rgb15to16_MMX;
		rgb15to24= rgb15to24_MMX;
		rgb15to32= rgb15to32_MMX;
		rgb16to24= rgb16to24_MMX;
		rgb16to32= rgb16to32_MMX;
		rgb16to15= rgb16to15_MMX;
		rgb24to16= rgb24to16_MMX;
		rgb24to15= rgb24to15_MMX;
		rgb24to32= rgb24to32_MMX;
		rgb32to16= rgb32to16_MMX;
		rgb32to15= rgb32to15_MMX;
		rgb32to24= rgb32to24_MMX;
		rgb24tobgr15= rgb24tobgr15_MMX;
		rgb24tobgr16= rgb24tobgr16_MMX;
		rgb24tobgr24= rgb24tobgr24_MMX;
		rgb32tobgr32= rgb32tobgr32_MMX;
		rgb32tobgr16= rgb32tobgr16_MMX;
		rgb32tobgr15= rgb32tobgr15_MMX;
		yv12toyuy2= yv12toyuy2_MMX;
		yv12touyvy= yv12touyvy_MMX;
		yuv422ptoyuy2= yuv422ptoyuy2_MMX;
		yuy2toyv12= yuy2toyv12_MMX;
//		uyvytoyv12= uyvytoyv12_MMX;
//		yvu9toyv12= yvu9toyv12_MMX;
		planar2x= planar2x_MMX;
		rgb24toyv12= rgb24toyv12_MMX;
		interleaveBytes= interleaveBytes_MMX;
		vu9_to_vu12= vu9_to_vu12_MMX;
		yvu9_to_yuy2= yvu9_to_yuy2_MMX;
	}else
#endif /* ARCH_X86_32 || ARCH_X86_64 */
	{
		rgb15to16= rgb15to16_C;
		rgb15to24= rgb15to24_C;
		rgb15to32= rgb15to32_C;
		rgb16to24= rgb16to24_C;
		rgb16to32= rgb16to32_C;
		rgb16to15= rgb16to15_C;
		rgb24to16= rgb24to16_C;
		rgb24to15= rgb24to15_C;
		rgb24to32= rgb24to32_C;
		rgb32to16= rgb32to16_C;
		rgb32to15= rgb32to15_C;
		rgb32to24= rgb32to24_C;
		rgb24tobgr15= rgb24tobgr15_C;
		rgb24tobgr16= rgb24tobgr16_C;
		rgb24tobgr24= rgb24tobgr24_C;
		rgb32tobgr32= rgb32tobgr32_C;
		rgb32tobgr16= rgb32tobgr16_C;
		rgb32tobgr15= rgb32tobgr15_C;
		yv12toyuy2= yv12toyuy2_C;
		yv12touyvy= yv12touyvy_C;
		yuv422ptoyuy2= yuv422ptoyuy2_C;
		yuy2toyv12= yuy2toyv12_C;
//		uyvytoyv12= uyvytoyv12_C;
//		yvu9toyv12= yvu9toyv12_C;
		planar2x= planar2x_C;
		rgb24toyv12= rgb24toyv12_C;
		interleaveBytes= interleaveBytes_C;
		vu9_to_vu12= vu9_to_vu12_C;
		yvu9_to_yuy2= yvu9_to_yuy2_C;
	}
}

/**
 * Pallete is assumed to contain bgr32
 */
void palette8torgb32(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
	long i;

/*
        for(i=0; i<num_pixels; i++)
                ((unsigned *)dst)[i] = ((unsigned *)palette)[ src[i] ];
*/

	for(i=0; i<num_pixels; i++)
	{
		#ifdef WORDS_BIGENDIAN
		dst[3]= palette[ src[i]*4+2 ];
		dst[2]= palette[ src[i]*4+1 ];
		dst[1]= palette[ src[i]*4+0 ];
		#else
		//FIXME slow?
		dst[0]= palette[ src[i]*4+2 ];
		dst[1]= palette[ src[i]*4+1 ];
		dst[2]= palette[ src[i]*4+0 ];
//		dst[3]= 0; /* do we need this cleansing? */
		#endif
		dst+= 4;
	}
}

void palette8tobgr32(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
	long i;
	for(i=0; i<num_pixels; i++)
	{
		#ifdef WORDS_BIGENDIAN
		dst[3]= palette[ src[i]*4+0 ];
		dst[2]= palette[ src[i]*4+1 ];
		dst[1]= palette[ src[i]*4+2 ];
		#else
		//FIXME slow?
		dst[0]= palette[ src[i]*4+0 ];
		dst[1]= palette[ src[i]*4+1 ];
		dst[2]= palette[ src[i]*4+2 ];
//		dst[3]= 0; /* do we need this cleansing? */
		#endif

		dst+= 4;
	}
}

/**
 * Pallete is assumed to contain bgr32
 */
void palette8torgb24(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
	long i;
/*
        writes 1 byte o much and might cause alignment issues on some architectures?
        for(i=0; i<num_pixels; i++)
                ((unsigned *)(&dst[i*3])) = ((unsigned *)palette)[ src[i] ];
*/
        for(i=0; i<num_pixels; i++)
        {
                //FIXME slow?
		dst[0]= palette[ src[i]*4+2 ];
		dst[1]= palette[ src[i]*4+1 ];
		dst[2]= palette[ src[i]*4+0 ];
		dst+= 3;
	}
}

void palette8tobgr24(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
	long i;
/*
	writes 1 byte o much and might cause alignment issues on some architectures?
	for(i=0; i<num_pixels; i++)
		((unsigned *)(&dst[i*3])) = ((unsigned *)palette)[ src[i] ];
*/
	for(i=0; i<num_pixels; i++)
	{
		//FIXME slow?
                dst[0]= palette[ src[i]*4+0 ];
                dst[1]= palette[ src[i]*4+1 ];
                dst[2]= palette[ src[i]*4+2 ];
                dst+= 3;
        }
}

/**
 * Palette is assumed to contain BGR16, see rgb32to16 to convert the palette.
 */
void palette8torgb16(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = ((const uint16_t *)palette)[src[i]];
}
void palette8tobgr16(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = bswap_16(((const uint16_t *)palette)[src[i]]);
}

/**
 * Palette is assumed to contain BGR15, see rgb32to15 to convert the palette.
 */
void palette8torgb15(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = ((const uint16_t *)palette)[src[i]];
}
void palette8tobgr15(const uint8_t *src, uint8_t *dst, long num_pixels, const uint8_t *palette)
{
    long i;
    for (i=0; i<num_pixels; i++)
        ((uint16_t *)dst)[i] = bswap_16(((const uint16_t *)palette)[src[i]]);
}

void rgb32tobgr24(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    long i;
    stride_t num_pixels = src_size >> 2;
    for (i=0; i<num_pixels; i++)
    {
        #ifdef WORDS_BIGENDIAN
            /* RGB32 (= A,B,G,R) -> BGR24 (= B,G,R) */
            dst[3*i + 0] = src[4*i + 1];
            dst[3*i + 1] = src[4*i + 2];
            dst[3*i + 2] = src[4*i + 3];
        #else
            dst[3*i + 0] = src[4*i + 2];
            dst[3*i + 1] = src[4*i + 1];
            dst[3*i + 2] = src[4*i + 0];
        #endif
    }
}

void rgb24tobgr32(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    long i;
    for (i=0; 3*i<src_size; i++)
    {
        #ifdef WORDS_BIGENDIAN
            /* RGB24 (= R,G,B) -> BGR32 (= A,R,G,B) */
            dst[4*i + 0] = 255;
            dst[4*i + 1] = src[3*i + 0];
            dst[4*i + 2] = src[3*i + 1];
            dst[4*i + 3] = src[3*i + 2];
        #else
            dst[4*i + 0] = src[3*i + 2];
            dst[4*i + 1] = src[3*i + 1];
            dst[4*i + 2] = src[3*i + 0];
            dst[4*i + 3] = 255;
        #endif
    }
}

void rgb16tobgr32(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        #ifdef WORDS_BIGENDIAN
            *d++ = 255;
            *d++ = (bgr&0x1F)<<3;
            *d++ = (bgr&0x7E0)>>3;
            *d++ = (bgr&0xF800)>>8;
        #else
            *d++ = (bgr&0xF800)>>8;
            *d++ = (bgr&0x7E0)>>3;
            *d++ = (bgr&0x1F)<<3;
            *d++ = 255;
        #endif
    }
}

void rgb16tobgr24(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = (bgr&0xF800)>>8;
        *d++ = (bgr&0x7E0)>>3;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb16tobgr16(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
	long i;
	stride_t num_pixels = src_size >> 1;

	for(i=0; i<num_pixels; i++)
	{
	    unsigned b,g,r;
	    register uint16_t rgb;
	    rgb = ((uint16_t*)src)[i];
	    r = rgb&0x1F;
	    g = (rgb&0x7E0)>>5;
	    b = (rgb&0xF800)>>11;
	    ((uint16_t*)dst)[i] = (b&0x1F) | ((g&0x3F)<<5) | ((r&0x1F)<<11);
	}
}

void rgb16tobgr15(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
	long i;
	stride_t num_pixels = src_size >> 1;

	for(i=0; i<num_pixels; i++)
	{
	    unsigned b,g,r;
	    register uint16_t rgb;
	    rgb = ((uint16_t*)src)[i];
	    r = rgb&0x1F;
	    g = (rgb&0x7E0)>>5;
	    b = (rgb&0xF800)>>11;
	    ((uint16_t*)dst)[i] = (b&0x1F) | ((g&0x1F)<<5) | ((r&0x1F)<<10);
	}
}

void rgb15tobgr32(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        #ifdef WORDS_BIGENDIAN
            *d++ = 255;
            *d++ = (bgr&0x1F)<<3;
            *d++ = (bgr&0x3E0)>>2;
            *d++ = (bgr&0x7C00)>>7;
        #else
            *d++ = (bgr&0x7C00)>>7;
            *d++ = (bgr&0x3E0)>>2;
            *d++ = (bgr&0x1F)<<3;
            *d++ = 255;
        #endif
    }
}

void rgb15tobgr24(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    const uint16_t *end;
    uint8_t *d = dst;
    const uint16_t *s = (const uint16_t *)src;
    end = s + src_size/2;
    while (s < end)
    {
        register uint16_t bgr;
        bgr = *s++;
        *d++ = (bgr&0x7C00)>>7;
        *d++ = (bgr&0x3E0)>>2;
        *d++ = (bgr&0x1F)<<3;
    }
}

void rgb15tobgr16(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
	long i;
	stride_t num_pixels = src_size >> 1;

	for(i=0; i<num_pixels; i++)
	{
	    unsigned b,g,r;
	    register uint16_t rgb;
	    rgb = ((uint16_t*)src)[i];
	    r = rgb&0x1F;
	    g = (rgb&0x3E0)>>5;
	    b = (rgb&0x7C00)>>10;
	    ((uint16_t*)dst)[i] = (b&0x1F) | ((g&0x3F)<<5) | ((r&0x1F)<<11);
	}
}

void rgb15tobgr15(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
	long i;
	stride_t num_pixels = src_size >> 1;

	for(i=0; i<num_pixels; i++)
	{
	    unsigned b,g,r;
	    register uint16_t rgb;
	    rgb = ((uint16_t*)src)[i];
	    r = rgb&0x1F;
	    g = (rgb&0x3E0)>>5;
	    b = (rgb&0x7C00)>>10;
	    ((uint16_t*)dst)[i] = (b&0x1F) | ((g&0x1F)<<5) | ((r&0x1F)<<10);
	}
}

void rgb8tobgr8(const uint8_t *src, uint8_t *dst, stride_t src_size)
{
    long i;
    stride_t num_pixels = src_size;
    for (i=0; i<num_pixels; i++)
    {
        unsigned b,g,r;
        register uint8_t rgb;
        rgb = src[i];
        r = (rgb&0x07);
        g = (rgb&0x38)>>3;
        b = (rgb&0xC0)>>6;
        dst[i] = ((b<<1)&0x07) | ((g&0x07)<<3) | ((r&0x03)<<6);
    }
}
