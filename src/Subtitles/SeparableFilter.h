/*
* (C) 2007 Niels Martin Hansen
* (C) 2013-2017 see Authors.txt
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

#include <cmath>

#define LIBDIVIDE_USE_SSE2 1
#pragma warning(push)
#pragma warning(disable: 4244 4456 4702)
#include "libdivide.h"
#pragma warning(pop)

// Filter an image in horizontal direction with a one-dimensional filter
// PixelWidth is the distance in bytes between pixels
template<ptrdiff_t PixelDist>
void SeparableFilterX(unsigned char* src, unsigned char* dst, int width, int height, ptrdiff_t stride,
                      short* kernel, int kernel_size, int divisor)
{
    int* tmp = DEBUG_NEW int[width];

    for (int y = 0; y < height; y++) {
        ZeroMemory(tmp, width * sizeof(int));

        const unsigned char* in = src + y * stride;
        unsigned char* out = dst + y * stride;

        for (int k = 0; k < kernel_size; k++) {
            int xOffset = k - kernel_size / 2;
            int xStart = 0;
            int xEnd = width;
            if (xOffset < 0) {
                xEnd += xOffset;
            } else if (xOffset > 0) {
                xStart += xOffset;
            }
            for (int x = xStart; x < xEnd; x++) {
                tmp[x - xOffset] += (int)(in[x * PixelDist] * kernel[k]);
            }
        }
        for (int x = 0; x < width; x++) {
            int accum = tmp[x] / divisor;
            if (accum > 255) {
                accum = 255;
            } else if (accum < 0) {
                accum = 0;
            }
            out[x * PixelDist] = (unsigned char)accum;
        }
    }

    delete [] tmp;
}


// Filter an image in vertical direction with a one-dimensional filter
// PixelWidth is the distance in bytes between pixels
template<ptrdiff_t PixelDist>
void SeparableFilterY(unsigned char* src, unsigned char* dst, int width, int height, ptrdiff_t stride,
                      short* kernel, int kernel_size, int divisor)
{
    int* tmp = DEBUG_NEW int[width];

    for (int y = 0; y < height; y++) {
        ZeroMemory(tmp, width * sizeof(int));

        const unsigned char* in = src + y * stride;
        unsigned char* out = dst + y * stride;

        int kOffset = kernel_size / 2;
        int kStart = 0;
        int kEnd = kernel_size;
        if (y < kOffset) { // 0 > y - kOffset
            kStart += kOffset - y;
        } else if (height <= y + kOffset) {
            kEnd -= kOffset + y + 1 - height;
        }
        for (int k = kStart; k < kEnd; k++) {
            for (int x = 0; x < width; x++) {
                tmp[x] += (int)(in[(k - kOffset) * stride + x * PixelDist] * kernel[k]);
            }
        }
        for (int x = 0; x < width; x++) {
            int accum = tmp[x] / divisor;
            if (accum > 255) {
                accum = 255;
            } else if (accum < 0) {
                accum = 0;
            }
            out[x * PixelDist] = (unsigned char)accum;
        }
    }

    delete [] tmp;
}


// Filter an image in horizontal direction with a one-dimensional filter
void SeparableFilterX_SSE2(unsigned char* src, unsigned char* dst, int width, int height, ptrdiff_t stride,
                           short* kernel, int kernel_size, int divisor)
{
    int width16 = width & ~15;
    int* tmp = (int*)_aligned_malloc(stride * sizeof(int), 16);
    libdivide::divider<int> divisorLibdivide(divisor);

    for (int y = 0; y < height; y++) {
        ZeroMemory(tmp, stride * sizeof(int));

        const unsigned char* in = src + y * stride;
        unsigned char* out = dst + y * stride;

        for (int k = 0; k < kernel_size; k++) {
            int xOffset = k - kernel_size / 2;
            int xStart = 0;
            int xEnd = width;
            if (xOffset < 0) {
                xEnd += xOffset;
            } else if (xOffset > 0) {
                xStart += xOffset;
            }
            int xStart16 = (xStart + 15) & ~15;
            int xEnd16 = xEnd & ~15;
            if (xStart16 >= xEnd16) { // Don't use SSE2 at all
                xStart16 = xEnd16 = xEnd;
            }
            for (int x = xStart; x < xStart16; x++) {
                tmp[x - xOffset] += (int)(in[x] * kernel[k]);
            }
            __m128i coeff = _mm_set1_epi16(kernel[k]);
            for (int x = xStart16; x < xEnd16; x += 16) {
                // Load 16 values
                __m128i data16 = _mm_load_si128((__m128i*)&in[x]);

                // Multiply the first 8 values by the coefficient to get 8 32-bit integers
                __m128i data8 = _mm_unpacklo_epi8(data16, _mm_setzero_si128());
                __m128i resLo = _mm_mullo_epi16(data8, coeff);
                __m128i resHi = _mm_mulhi_epi16(data8, coeff);
                __m128i res32bitLo = _mm_unpacklo_epi16(resLo, resHi);
                __m128i res32bitHi = _mm_unpackhi_epi16(resLo, resHi);

                // Load the 4 32-bit integers values, add the values we computed and store them back
                __m128i res = _mm_loadu_si128((__m128i*)&tmp[x - xOffset]);
                res = _mm_add_epi32(res, res32bitLo);
                _mm_storeu_si128((__m128i*)&tmp[x - xOffset], res);
                // Repeat the same operation for the next 4 values
                res = _mm_loadu_si128((__m128i*)&tmp[x - xOffset + 4]);
                res = _mm_add_epi32(res, res32bitHi);
                _mm_storeu_si128((__m128i*)&tmp[x - xOffset + 4], res);

                // Multiply the next 8 values by the coefficient to get 8 32-bit integers
                data8 = _mm_unpackhi_epi8(data16, _mm_setzero_si128());
                resLo = _mm_mullo_epi16(data8, coeff);
                resHi = _mm_mulhi_epi16(data8, coeff);
                res32bitLo = _mm_unpacklo_epi16(resLo, resHi);
                res32bitHi = _mm_unpackhi_epi16(resLo, resHi);

                // Load the 4 32-bit integers values, add the values we computed and store them back
                res = _mm_loadu_si128((__m128i*)&tmp[x - xOffset + 8]);
                res = _mm_add_epi32(res, res32bitLo);
                _mm_storeu_si128((__m128i*)&tmp[x - xOffset + 8], res);
                // Repeat the same operation for the next 4 values
                res = _mm_loadu_si128((__m128i*)&tmp[x - xOffset + 12]);
                res = _mm_add_epi32(res, res32bitHi);
                _mm_storeu_si128((__m128i*)&tmp[x - xOffset + 12], res);
            }
            for (int x = xEnd16; x < xEnd; x++) {
                tmp[x - xOffset] += (int)(in[x] * kernel[k]);
            }
        }
        for (int x = 0; x < width16; x += 16) {
            // Load 4 32-bit integer values and divide them
            __m128i accum1 = _mm_load_si128((__m128i*)&tmp[x]);
            accum1 = accum1 / divisorLibdivide;
            // Repeat the same operation on the next 4 32-bit integer values
            __m128i accum2 = _mm_load_si128((__m128i*)&tmp[x + 4]);
            accum2 = accum2 / divisorLibdivide;
            // Pack the 8 32-bit integers into 8 16-bit integers
            accum1 = _mm_packs_epi32(accum1, accum2);

            // Load 4 32-bit integer values and divide them
            __m128i accum3 = _mm_load_si128((__m128i*)&tmp[x + 8]);
            accum3 = accum3 / divisorLibdivide;
            // Repeat the same operation on the next 4 32-bit integer values
            __m128i accum4 = _mm_load_si128((__m128i*)&tmp[x + 12]);
            accum4 = accum4 / divisorLibdivide;
            // Pack the 8 32-bit integers into 8 16-bit integers
            accum3 = _mm_packs_epi32(accum3, accum4);

            // Pack the 16 16-bit integers into 16 8-bit unsigned integers
            accum1 = _mm_packus_epi16(accum1, accum3);

            // Store the 16 8-bit unsigned integers
            _mm_store_si128((__m128i*)&out[x], accum1);
        }
        for (int x = width16; x < width; x++) {
            int accum = tmp[x] / divisor;
            if (accum > 255) {
                accum = 255;
            } else if (accum < 0) {
                accum = 0;
            }
            out[x] = (unsigned char)accum;
        }
    }

    _aligned_free(tmp);
}


// Filter an image in vertical direction with a one-dimensional filter
void SeparableFilterY_SSE2(unsigned char* src, unsigned char* dst, int width, int height, ptrdiff_t stride,
                           short* kernel, int kernel_size, int divisor)
{
    int width16 = width & ~15;
    int* tmp = (int*)_aligned_malloc(stride * sizeof(int), 16);
    libdivide::divider<int> divisorLibdivide(divisor);

#ifdef _OPENMP
    #pragma omp parallel for
#endif
    for (int y = 0; y < height; y++) {
        ZeroMemory(tmp, stride * sizeof(int));

        const unsigned char* in = src + y * stride;
        unsigned char* out = dst + y * stride;

        int kOffset = kernel_size / 2;
        int kStart = 0;
        int kEnd = kernel_size;
        if (y < kOffset) { // 0 > y - kOffset
            kStart += kOffset - y;
        } else if (height <= y + kOffset) {
            kEnd -= kOffset + y + 1 - height;
        }
        for (int k = kStart; k < kEnd; k++) {
            __m128i coeff = _mm_set1_epi16(kernel[k]);
            for (int x = 0; x < width16; x += 16) {
                // Load 16 values
                __m128i data16 = _mm_load_si128((__m128i*)&in[(k - kOffset) * stride + x]);

                // Multiply the first 8 values by the coefficient to get 8 32-bit integers
                __m128i data8 = _mm_unpacklo_epi8(data16, _mm_setzero_si128());
                __m128i resLo = _mm_mullo_epi16(data8, coeff);
                __m128i resHi = _mm_mulhi_epi16(data8, coeff);
                __m128i res32bitLo = _mm_unpacklo_epi16(resLo, resHi);
                __m128i res32bitHi = _mm_unpackhi_epi16(resLo, resHi);

                // Load the 4 32-bit integers values, add the values we computed and store them back
                __m128i res = _mm_load_si128((__m128i*)&tmp[x]);
                res = _mm_add_epi32(res, res32bitLo);
                _mm_store_si128((__m128i*)&tmp[x], res);
                // Repeat the same operation for the next 4 values
                res = _mm_load_si128((__m128i*)&tmp[x + 4]);
                res = _mm_add_epi32(res, res32bitHi);
                _mm_store_si128((__m128i*)&tmp[x + 4], res);

                // Multiply the next 8 values by the coefficient to get 8 32-bit integers
                data8 = _mm_unpackhi_epi8(data16, _mm_setzero_si128());
                resLo = _mm_mullo_epi16(data8, coeff);
                resHi = _mm_mulhi_epi16(data8, coeff);
                res32bitLo = _mm_unpacklo_epi16(resLo, resHi);
                res32bitHi = _mm_unpackhi_epi16(resLo, resHi);

                // Load the 4 32-bit integers values, add the values we computed and store them back
                res = _mm_load_si128((__m128i*)&tmp[x + 8]);
                res = _mm_add_epi32(res, res32bitLo);
                _mm_store_si128((__m128i*)&tmp[x + 8], res);
                // Repeat the same operation for the next 4 values
                res = _mm_load_si128((__m128i*)&tmp[x + 12]);
                res = _mm_add_epi32(res, res32bitHi);
                _mm_store_si128((__m128i*)&tmp[x + 12], res);
            }
            for (int x = width16; x < width; x++) {
                tmp[x] += (int)(in[(k - kOffset) * stride + x] * kernel[k]);
            }
        }
        for (int x = 0; x < width16; x += 16) {
            // Load 4 32-bit integer values and divide them
            __m128i accum1 = _mm_load_si128((__m128i*)&tmp[x]);
            accum1 = accum1 / divisorLibdivide;
            // Repeat the same operation on the next 4 32-bit integer values
            __m128i accum2 = _mm_load_si128((__m128i*)&tmp[x + 4]);
            accum2 = accum2 / divisorLibdivide;
            // Pack the 8 32-bit integers into 8 16-bit integers
            accum1 = _mm_packs_epi32(accum1, accum2);

            // Load 4 32-bit integer values and divide them
            __m128i accum3 = _mm_load_si128((__m128i*)&tmp[x + 8]);
            accum3 = accum3 / divisorLibdivide;
            // Repeat the same operation on the next 4 32-bit integer values
            __m128i accum4 = _mm_load_si128((__m128i*)&tmp[x + 12]);
            accum4 = accum4 / divisorLibdivide;
            // Pack the 8 32-bit integers into 8 16-bit integers
            accum3 = _mm_packs_epi32(accum3, accum4);

            // Pack the 16 16-bit integers into 16 8-bit unsigned integers
            accum1 = _mm_packus_epi16(accum1, accum3);

            // Store the 16 8-bit unsigned integers
            _mm_store_si128((__m128i*)&out[x], accum1);
        }
        for (int x = width16; x < width; x++) {
            int accum = tmp[x] / divisor;
            if (accum > 255) {
                accum = 255;
            } else if (accum < 0) {
                accum = 0;
            }
            out[x] = (unsigned char)accum;
        }
    }

    _aligned_free(tmp);
}



static inline double NormalDist(double sigma, double x)
{
    if (sigma <= 0.0 && x == 0.0) {
        return 1.0;
    } else if (sigma <= 0.0) {
        return 0.0;
    } else {
        return exp(-(x * x) / (2 * sigma * sigma)) / (sigma * sqrt(2 * M_PI));
    }
}


struct GaussianKernel {
    short* kernel;
    int width;
    int divisor;

    inline GaussianKernel(double sigma) {
        width = (int)(sigma * 3.0 + 0.5) | 1; // binary-or with 1 to make sure the number is odd
        if (width < 3) {
            width = 3;
        }
        kernel = DEBUG_NEW short[width];
        kernel[width / 2] = (short)(NormalDist(sigma, 0.0) * 255);
        divisor = kernel[width / 2];
        for (int x = width / 2 - 1; x >= 0; x--) {
            short val = (short)(NormalDist(sigma, width / 2 - x) * 255 + 0.5);
            divisor += val * 2;
            kernel[x] = val;
            kernel[width - x - 1] = val;
        }
        if (divisor == 0) { divisor = 1; } // workaround to prevent crash
    }

    inline ~GaussianKernel() {
        delete [] kernel;
    }

    GaussianKernel(const GaussianKernel&) = delete;
    GaussianKernel& operator=(const GaussianKernel&) = delete;
};
