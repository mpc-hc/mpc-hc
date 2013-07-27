/*
* (C) 2007 Niels Martin Hansen
* (C) 2013-2014 see Authors.txt
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

#include <math.h>


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
    }

    inline ~GaussianKernel() {
        delete [] kernel;
    }
};
