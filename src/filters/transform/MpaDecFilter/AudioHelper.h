/*
 * (C) 2012-2013 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#pragma warning(disable: 4005 4244)
extern "C" {
#include "ffmpeg/libavutil/samplefmt.h"
}
#pragma warning(default: 4005 4244)

#ifdef _MSC_VER
#define bswap_16(x) _byteswap_ushort((unsigned short)(x))
#define bswap_32(x) _byteswap_ulong ((unsigned long)(x))
#define bswap_64(x) _byteswap_uint64((unsigned __int64)(x))
#else
#define bswap_16(x) ((uint16_t)(x) >> 8 | (uint16_t)(x) << 8)
#define bswap_32(x) ((uint32_t)(x) >> 24              | \
                    ((uint32_t)(x) & 0x00ff0000) >> 8 | \
                    ((uint32_t)(x) & 0x0000ff00) << 8 | \
                     (uint32_t)(x) << 24)
#define bswap_64(x) ((uint64_t)(x) >> 56                       | \
                    ((uint64_t)(x) & 0x00FF000000000000) >> 40 | \
                    ((uint64_t)(x) & 0x0000FF0000000000) >> 24 | \
                    ((uint64_t)(x) & 0x000000FF00000000) >>  8 | \
                    ((uint64_t)(x) & 0x00000000FF000000) <<  8 | \
                    ((uint64_t)(x) & 0x0000000000FF0000) << 24 | \
                    ((uint64_t)(x) & 0x000000000000FF00) << 40 | \
                     (uint64_t)(x) << 56)
#endif

HRESULT convert_to_int16(enum AVSampleFormat avsf, WORD nChannels, DWORD nSamples, BYTE* pIn, int16_t* pOut);
HRESULT convert_to_int24(enum AVSampleFormat avsf, WORD nChannels, DWORD nSamples, BYTE* pIn, BYTE* pOut);
HRESULT convert_to_int32(enum AVSampleFormat avsf, WORD nChannels, DWORD nSamples, BYTE* pIn, int32_t* pOut);
HRESULT convert_to_float(enum AVSampleFormat avsf, WORD nChannels, DWORD nSamples, BYTE* pIn, float* pOut);

HRESULT convert_to_planar_float(enum AVSampleFormat avsf, WORD nChannels, DWORD nSamples, BYTE* pIn, float* pOut);
