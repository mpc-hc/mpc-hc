//  VirtualDub - Video processing and capture application
//  Graphics support library
//  Copyright (C) 1998-2007 Avery Lee
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  Notes:
//  - VDPixmapBlt is from VirtualDub
//  - sse2 yv12 to yuy2 conversion by Haali
//  (- vd.cpp/h should be renamed to something more sensible already :)


#include "stdafx.h"
#include "vd.h"
#include "vd_asm.h"
#include <intrin.h>

#include "vd2/system/cpuaccel.h"
#include "vd2/system/memory.h"
#include "vd2/system/vdstl.h"

#include "vd2/Kasumi/pixmap.h"
#include "vd2/Kasumi/pixmaputils.h"
#include "vd2/Kasumi/pixmapops.h"
#include "vd2/Kasumi/resample.h"

#pragma warning(disable : 4799) // no emms... blahblahblah

void VDCPUTest() {
    SYSTEM_INFO si;

    long lEnableFlags = CPUCheckForExtensions();

    GetSystemInfo(&si);

    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        if (si.wProcessorLevel < 4)
            lEnableFlags &= ~CPUF_SUPPORTS_FPU;     // Not strictly true, but very slow anyway

    // Enable FPU support...

    CPUEnableExtensions(lEnableFlags);

    VDFastMemcpyAutodetect();
}

CCpuID g_cpuid;

CCpuID::CCpuID()
{
    VDCPUTest();

    long lEnableFlags = CPUGetEnabledExtensions();

    int flags = 0;
    flags |= !!(lEnableFlags & CPUF_SUPPORTS_MMX)           ? mmx       : 0;            // STD MMX
    flags |= !!(lEnableFlags & CPUF_SUPPORTS_INTEGER_SSE)   ? ssemmx    : 0;            // SSE MMX
    flags |= !!(lEnableFlags & CPUF_SUPPORTS_SSE)           ? ssefpu    : 0;            // STD SSE
    flags |= !!(lEnableFlags & CPUF_SUPPORTS_SSE2)          ? sse2      : 0;            // SSE2
    flags |= !!(lEnableFlags & CPUF_SUPPORTS_3DNOW)         ? _3dnow    : 0;            // 3DNow

    // result
    m_flags = (flag_t)flags;
}

bool BitBltFromI420ToI420(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    VDPixmap srcbm = {0};

    srcbm.data      = srcy;
    srcbm.pitch     = srcpitch;
    srcbm.w         = w;
    srcbm.h         = h;
    srcbm.format    = nsVDPixmap::kPixFormat_YUV420_Planar;
    srcbm.data2     = srcu;
    srcbm.pitch2    = srcpitch / 2;
    srcbm.data3     = srcv;
    srcbm.pitch3    = srcpitch / 2;

    VDPixmap dstpxm = {0};

    dstpxm.data     = dsty;
    dstpxm.pitch    = dstpitch;
    dstpxm.w        = w;
    dstpxm.h        = h;
    dstpxm.format   = nsVDPixmap::kPixFormat_YUV420_Planar;
    dstpxm.data2    = dstu;
    dstpxm.pitch2   = dstpitch / 2;
    dstpxm.data3    = dstv;
    dstpxm.pitch3   = dstpitch / 2;

    return VDPixmapBlt(dstpxm, srcbm);
}

bool BitBltFromYUY2ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* src, int srcpitch)
{
    VDPixmap srcbm = {0};

    srcbm.data      = src;
    srcbm.pitch     = srcpitch;
    srcbm.w         = w;
    srcbm.h         = h;
    srcbm.format    = nsVDPixmap::kPixFormat_YUV422_YUYV;

    VDPixmap dstpxm = {
        dst,
        NULL,
        w,
        h,
        dstpitch
    };

    dstpxm.format = nsVDPixmap::kPixFormat_YUV422_YUYV;

    return VDPixmapBlt(dstpxm, srcbm);
}

bool BitBltFromI420ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    VDPixmap srcbm = {0};

    srcbm.data      = srcy;
    srcbm.pitch     = srcpitch;
    srcbm.w         = w;
    srcbm.h         = h;
    srcbm.format    = nsVDPixmap::kPixFormat_YUV420_Planar;
    srcbm.data2     = srcu;
    srcbm.pitch2    = srcpitch/2;
    srcbm.data3     = srcv;
    srcbm.pitch3    = srcpitch/2;

    VDPixmap dstpxm = {
        (char *)dst + dstpitch * (h - 1),
        NULL,
        w,
        h,
        -dstpitch
    };

    switch(dbpp) {
    case 16:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB565;
        break;
    case 24:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB888;
        break;
    case 32:
        dstpxm.format = nsVDPixmap::kPixFormat_XRGB8888;
        break;
    default:
        VDASSERT(false);
    }

    return VDPixmapBlt(dstpxm, srcbm);
}

bool BitBltFromI420ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if (srcpitch == 0) srcpitch = w;

#ifndef _WIN64
    if ((g_cpuid.m_flags & CCpuID::sse2)
        && !((DWORD_PTR)srcy&15) && !((DWORD_PTR)srcu&15) && !((DWORD_PTR)srcv&15) && !(srcpitch&31)
        && !((DWORD_PTR)dst&15) && !(dstpitch&15))
    {
        if (w<=0 || h<=0 || (w&1) || (h&1))
            return false;

        yv12_yuy2_sse2(srcy, srcu, srcv, srcpitch/2, w/2, h, dst, dstpitch);
        return true;
    }
#endif

    VDPixmap srcbm = {0};

    srcbm.data      = srcy;
    srcbm.pitch     = srcpitch;
    srcbm.w         = w;
    srcbm.h         = h;
    srcbm.format    = nsVDPixmap::kPixFormat_YUV420_Planar;
    srcbm.data2     = srcu;
    srcbm.pitch2    = srcpitch/2;
    srcbm.data3     = srcv;
    srcbm.pitch3    = srcpitch/2;

    VDPixmap dstpxm = {
        dst,
        NULL,
        w,
        h,
        dstpitch
    };

    dstpxm.format = nsVDPixmap::kPixFormat_YUV422_YUYV;

    return VDPixmapBlt(dstpxm, srcbm);
}

bool BitBltFromRGBToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* src, int srcpitch, int sbpp)
{
    VDPixmap srcbm = {
        (char *)src + srcpitch * (h - 1),
        NULL,
        w,
        h,
        -srcpitch
    };

    switch(sbpp) {
    case 8:
        srcbm.format = nsVDPixmap::kPixFormat_Pal8;
        break;
    case 16:
        srcbm.format = nsVDPixmap::kPixFormat_RGB565;
        break;
    case 24:
        srcbm.format = nsVDPixmap::kPixFormat_RGB888;
        break;
    case 32:
        srcbm.format = nsVDPixmap::kPixFormat_XRGB8888;
        break;
    default:
        VDASSERT(false);
    }

    VDPixmap dstpxm = {
        (char *)dst + dstpitch * (h - 1),
        NULL,
        w,
        h,
        -dstpitch
    };

    switch(dbpp) {
    case 8:
        dstpxm.format = nsVDPixmap::kPixFormat_Pal8;
        break;
    case 16:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB565;
        break;
    case 24:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB888;
        break;
    case 32:
        dstpxm.format = nsVDPixmap::kPixFormat_XRGB8888;
        break;
    default:
        VDASSERT(false);
    }

    return VDPixmapBlt(dstpxm, srcbm);
}

bool BitBltFromRGBToRGBStretch(int dstw, int dsth, BYTE* dst, int dstpitch, int dbpp, int srcw, int srch, BYTE* src, int srcpitch, int sbpp)
{
    VDPixmap srcbm = {
        src + srcpitch * (srch - 1),
        nullptr,
        srcw,
        srch,
        -srcpitch
    };

    switch (sbpp) {
    case 8:
        srcbm.format = nsVDPixmap::kPixFormat_Pal8;
        break;
    case 16:
        srcbm.format = nsVDPixmap::kPixFormat_RGB565;
        break;
    case 24:
        srcbm.format = nsVDPixmap::kPixFormat_RGB888;
        break;
    case 32:
        srcbm.format = nsVDPixmap::kPixFormat_XRGB8888;
        break;
    default:
        VDASSERT(false);
    }

    VDPixmap dstpxm = {
        dst + dstpitch * (dsth - 1),
        nullptr,
        dstw,
        dsth,
        -dstpitch
    };

    switch (dbpp) {
    case 8:
        dstpxm.format = nsVDPixmap::kPixFormat_Pal8;
        break;
    case 16:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB565;
        break;
    case 24:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB888;
        break;
    case 32:
        dstpxm.format = nsVDPixmap::kPixFormat_XRGB8888;
        break;
    default:
        VDASSERT(false);
    }

    return VDPixmapResample(dstpxm, srcbm, IVDPixmapResampler::kFilterCubic);
}


bool BitBltFromYUY2ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* src, int srcpitch)
{
    if (srcpitch == 0) srcpitch = w;

    VDPixmap srcbm = {0};

    srcbm.data      = src;
    srcbm.pitch     = srcpitch;
    srcbm.w         = w;
    srcbm.h         = h;
    srcbm.format    = nsVDPixmap::kPixFormat_YUV422_YUYV;

    VDPixmap dstpxm = {
        (char *)dst + dstpitch * (h - 1),
        NULL,
        w,
        h,
        -dstpitch
    };

    switch(dbpp) {
    case 16:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB565;
        break;
    case 24:
        dstpxm.format = nsVDPixmap::kPixFormat_RGB888;
        break;
    case 32:
        dstpxm.format = nsVDPixmap::kPixFormat_XRGB8888;
        break;
    default:
        VDASSERT(false);
    }

    return VDPixmapBlt(dstpxm, srcbm);
}

static void yuvtoyuy2row_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width)
{
    WORD* dstw = (WORD*)dst;
    for (; width > 1; width -= 2)
    {
        *dstw++ = (*srcu++<<8)|*srcy++;
        *dstw++ = (*srcv++<<8)|*srcy++;
    }
}

static void yuvtoyuy2row_avg_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
    WORD* dstw = (WORD*)dst;
    for (; width > 1; width -= 2, srcu++, srcv++)
    {
        *dstw++ = (((srcu[0]+srcu[pitchuv])>>1)<<8)|*srcy++;
        *dstw++ = (((srcv[0]+srcv[pitchuv])>>1)<<8)|*srcy++;
    }
}

bool BitBltFromI420ToYUY2Interlaced(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if (w<=0 || h<=0 || (w&1) || (h&1))
        return false;

    if (srcpitch == 0) srcpitch = w;

    void (*yuvtoyuy2row)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width) = NULL;
    void (*yuvtoyuy2row_avg)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv) = NULL;

#ifndef _WIN64
    if ((g_cpuid.m_flags & CCpuID::sse2)
        && !((DWORD_PTR)srcy&15) && !((DWORD_PTR)srcu&15) && !((DWORD_PTR)srcv&15) && !(srcpitch&31)
        && !((DWORD_PTR)dst&15) && !(dstpitch&15))
    {
        yv12_yuy2_sse2_interlaced(srcy, srcu, srcv, srcpitch/2, w/2, h, dst, dstpitch);
        return true;
    }

    if ((g_cpuid.m_flags & CCpuID::mmx) && !(w&7))
    {
        yuvtoyuy2row = yuvtoyuy2row_MMX;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_MMX;
    }
    else
#endif
    {
        yuvtoyuy2row = yuvtoyuy2row_c;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_c;
    }

    if (!yuvtoyuy2row)
        return false;

    int halfsrcpitch = srcpitch/2;
    do
    {
        yuvtoyuy2row(dst, srcy, srcu, srcv, w);
        yuvtoyuy2row_avg(dst + dstpitch, srcy + srcpitch, srcu, srcv, w, halfsrcpitch);

        dst += 2*dstpitch;
        srcy += 2*srcpitch;
        srcu += halfsrcpitch;
        srcv += halfsrcpitch;
    }
    while ((h -= 2) > 2);

    yuvtoyuy2row(dst, srcy, srcu, srcv, w);
    yuvtoyuy2row(dst + dstpitch, srcy + srcpitch, srcu, srcv, w);

#ifndef _WIN64
    if (g_cpuid.m_flags & CCpuID::mmx)
        __asm emms
#endif

    return true;
}
