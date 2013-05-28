/*
 * (C) 2013 see Authors.txt
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

// TODO: remove this when it's fixed in MSVC
// Work around warning C4005: 'XXXX' : macro redefinition
#pragma warning(push)
#pragma warning(disable: 4005)
#include <stdint.h>
#pragma warning(pop)

#define INT24_MAX       8388607
#define INT24_MIN     (-8388608)

#define INT8_PEAK       128
#define INT16_PEAK      32768
#define INT24_PEAK      8388608
#define INT32_PEAK      2147483648

void gain_uint8(const double factor, const size_t allsamples, uint8_t* pData);
void gain_int16(const double factor, const size_t allsamples, int16_t* pData);
void gain_int24(const double factor, const size_t allsamples, BYTE*    pData);
void gain_int32(const double factor, const size_t allsamples, int32_t* pData);
void gain_float(const double factor, const size_t allsamples, float*   pData);
void gain_double(const double factor, const size_t allsamples, double* pData);
