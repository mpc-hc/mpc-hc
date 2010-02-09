/**
 * @file libavutil/timer.h
 * high precision timer, useful to profile code
 *
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
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

#ifndef AVUTIL_TIMER_H
#define AVUTIL_TIMER_H

#include <stdlib.h>
#ifdef __GNUC__
#include <stdint.h>
#endif
#include "config.h"

#if ARCH_X86
#   include "x86/timer.h"
#endif

#if !defined(AV_READ_TIME) && HAVE_GETHRTIME
#   define AV_READ_TIME gethrtime
#endif

#define START_TIMER
#define STOP_TIMER(id) {}

#endif /* AVUTIL_TIMER_H */
