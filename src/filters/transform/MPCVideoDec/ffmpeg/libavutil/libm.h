/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file libavutil/libm.h
 * Replacements for frequently missing libm functions
 */

#ifndef AVUTIL_LIBM_H
#define AVUTIL_LIBM_H

#include <math.h>
#include "config.h"
#include "attributes.h"

#ifndef exp2
#define exp2(x) exp((x) * 0.693147180559945)
#endif

#ifndef exp2f
#define exp2f(x) ((float)exp2(x))
#endif

#ifndef rint
#define rint(x) (int)(x+0.5)
#endif

#ifndef llrint
#define llrint(x) ((long long)rint(x))
#endif

#ifndef log2
#define log2(x) (log(x) * 1.44269504088896340736)
#endif

#ifndef log2f
#define log2f(x) ((float)log2(x))
#endif

#ifndef lrint
static av_always_inline av_const long int lrint(double x)
{
    return rint(x);
}
#endif

#ifndef lrintf
static av_always_inline av_const long int lrintf(float x)
{
    return (int)(rint(x));
}
#endif

#ifndef __GNUC__

#ifndef round
static av_always_inline av_const double round(double x)
{
    return (x > 0) ? floor(x + 0.5) : ceil(x - 0.5);
}
#endif

#ifndef roundf
static av_always_inline av_const float roundf(float x)
{
    return (x > 0) ? floor(x + 0.5) : ceil(x - 0.5);
}
#endif

#ifndef truncf
static av_always_inline av_const float truncf(float x)
{
    return (x > 0) ? floor(x) : ceil(x);
}
#endif

#ifndef cbrtf
static float cbrtf(float x)
{
    return pow((float)x, (float)1.0/3);
}
#endif

#endif /* __GNUC__ */

#endif /* AVUTIL_LIBM_H */
