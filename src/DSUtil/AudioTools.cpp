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

#include "stdafx.h"
#include "AudioTools.h"

#define limit(a, x, b) if ((x) < (a)) { x = a; } else if ((x) > (b)) { x = b;}

void gain_uint8(const double factor, const size_t allsamples, uint8_t* pData)
{
    uint8_t* end = pData + allsamples;
    for (; pData < end; ++pData) {
        double d = factor * (int8_t)(*pData ^ 0x80);
        limit(INT8_MIN, d, INT8_MAX);
        *pData = (uint8_t)(int8_t)d ^ 0x80;
    }
}

void gain_int16(const double factor, const size_t allsamples, int16_t* pData)
{
    int16_t* end = pData + allsamples;
    for (; pData < end; ++pData) {
        double d = factor * (*pData);
        limit(INT16_MIN, d, INT16_MAX);
        *pData = (int16_t)d;
    }
}

void gain_int24(const double factor, const size_t allsamples, BYTE* pData)
{
    BYTE* end = pData + allsamples * 3;
    while (pData < end) {
        int32_t i32 = 0;
        BYTE* p = (BYTE*)(&i32);
        p[1] = *(pData);
        p[2] = *(pData + 1);
        p[3] = *(pData + 2);
        double d = factor * i32;
        limit(INT32_MIN, d, INT32_MAX);
        i32 = (int32_t)d;
        *pData++ = p[1];
        *pData++ = p[2];
        *pData++ = p[3];
    }
}

void gain_int32(const double factor, const size_t allsamples, int32_t* pData)
{
    int32_t* end = pData + allsamples;
    for (; pData < end; ++pData) {
        double d = factor * (*pData);
        limit(INT32_MIN, d, INT32_MAX);
        *pData = (int32_t)d;
    }
}

void gain_float(const double factor, const size_t allsamples, float* pData)
{
    float* end = pData + allsamples;
    for (; pData < end; ++pData) {
        double d = factor * (*pData);
        limit(-1.0, d, 1.0);
        *pData = (float)d;
    }
}

void gain_double(const double factor, const size_t allsamples, double* pData)
{
    double* end = pData + allsamples;
    for (; pData < end; ++pData) {
        double d = factor * (*pData);
        limit(-1.0, d, 1.0);
        *pData = d;
    }
}
