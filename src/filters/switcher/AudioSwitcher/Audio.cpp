/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
// originally from virtualdub

#include "stdafx.h"
#include <algorithm>
#include <MMReg.h>
#include "Audio.h"

static long audio_pointsample_8(void* dst, void* src, long accum, long samp_frac, long cnt)
{
    unsigned char* d = (unsigned char*)dst;
    unsigned char* s = (unsigned char*)src;

    do {
        *d++ = s[accum >> 19];
        accum += samp_frac;
    } while (--cnt);

    return accum;
}

static long audio_pointsample_16(void* dst, void* src, long accum, long samp_frac, long cnt)
{
    unsigned short* d = (unsigned short*)dst;
    unsigned short* s = (unsigned short*)src;

    do {
        *d++ = s[accum >> 19];
        accum += samp_frac;
    } while (--cnt);

    return accum;
}

static long audio_pointsample_32(void* dst, void* src, long accum, long samp_frac, long cnt)
{
    unsigned long* d = (unsigned long*)dst;
    unsigned long* s = (unsigned long*)src;

    do {
        *d++ = s[accum >> 19];
        accum += samp_frac;
    } while (--cnt);

    return accum;
}

static long audio_downsample_mono8(void* dst, void* src, long* filter_bank, int filter_width, long accum, long samp_frac, long cnt)
{
    unsigned char* d = (unsigned char*)dst;
    unsigned char* s = (unsigned char*)src;

    do {
        long sum = 0;
        int w;
        long* fb_ptr;
        unsigned char* s_ptr;

        w = filter_width;
        fb_ptr = filter_bank + filter_width * ((accum >> 11) & 0xff);
        s_ptr = s + (accum >> 19);
        do {
            sum += *fb_ptr++ * (int) * s_ptr++;
        } while (--w);

        if (sum < 0) {
            *d++ = 0;
        } else if (sum > 0x3fffff) {
            *d++ = 0xff;
        } else {
            *d++ = (unsigned char)((sum + 0x2000) >> 14);
        }

        accum += samp_frac;
    } while (--cnt);

    return accum;
}

static long audio_downsample_mono16(void* dst, void* src, long* filter_bank, int filter_width, long accum, long samp_frac, long cnt)
{
    signed short* d = (signed short*)dst;
    signed short* s = (signed short*)src;

    do {
        long sum = 0;
        int w;
        long* fb_ptr;
        signed short* s_ptr;

        w = filter_width;
        fb_ptr = filter_bank + filter_width * ((accum >> 11) & 0xff);
        s_ptr = s + (accum >> 19);
        do {
            sum += *fb_ptr++ * (int) * s_ptr++;
        } while (--w);

        if (sum < -0x20000000) {
            *d++ = -0x8000;
        } else if (sum > 0x1fffffff) {
            *d++ = 0x7fff;
        } else {
            *d++ = (signed short)((sum + 0x2000) >> 14);
        }

        accum += samp_frac;
    } while (--cnt);

    return accum;
}

static int permute_index(int a, int b)
{
    return (b - (a >> 8) - 1) + (a & 255) * b;
}

static void make_downsample_filter(long* filter_bank, int filter_width, long samp_frac)
{
    int i, j;
    double filt_max;
    double filtwidth_frac;

    filtwidth_frac = samp_frac / 2048.0;

    filter_bank[filter_width - 1] = 0;

    filt_max = (16384.0 * 524288.0) / samp_frac;

    for (i = 0; i < 128 * filter_width; i++) {
        int y = 0;
        double d = i / filtwidth_frac;

        if (d < 1.0) {
            y = (int)(0.5 + filt_max * (1.0 - d));
        }

        filter_bank[permute_index(128 * filter_width + i, filter_width)]
            = filter_bank[permute_index(128 * filter_width - i, filter_width)]
              = y;
    }

    // Normalize the filter to correct for integer roundoff errors

    for (i = 0; i < 256 * filter_width; i += filter_width) {
        int v = 0;
        for (j = 0; j < filter_width; j++) {
            v += filter_bank[i + j];
        }

        //_RPT2(0,"error[%02x] = %04x\n", i/filter_width, 0x4000 - v);

        v = (0x4000 - v) / filter_width;
        for (j = 0; j < filter_width; j++) {
            filter_bank[i + j] += v;
        }
    }

    //  _CrtCheckMemory();
}

AudioStreamResampler::AudioStreamResampler(int bps, long orig_rate, long new_rate, bool fHighQuality)
    : ptsampleRout(audio_pointsample_16)
    , dnsampleRout(audio_downsample_mono16)
    , samp_frac(0x80000)
    , accum(0)
    , holdover(0)
    , filter_bank(nullptr)
    , filter_width(1)
    , bps(bps)
{
    if (bps == 1) {
        ptsampleRout = audio_pointsample_8;
        dnsampleRout = audio_downsample_mono8;
    } else if (bps >= 2) {
        ptsampleRout = audio_pointsample_16;
        dnsampleRout = audio_downsample_mono16;
    } else {
        return;
    }

    // orig_rate > new_rate!
    samp_frac = MulDiv(orig_rate, 0x80000, new_rate);

    // If this is a high-quality downsample, allocate memory for the filter bank
    if (fHighQuality) {
        if (samp_frac > 0x80000) {
            // HQ downsample: allocate filter bank

            filter_width = ((samp_frac + 0x7ffff) >> 19) << 1 << 1;

            filter_bank = DEBUG_NEW long[filter_width * 256];
            if (!filter_bank) {
                filter_width = 1;
                return;
            }

            make_downsample_filter(filter_bank, filter_width, samp_frac);

            // Clear lower samples

            memset(cbuffer, bps >= 2 ? 0 : 0x80, bps * filter_width);

            holdover = filter_width / 2;
        }
    }
}

AudioStreamResampler::~AudioStreamResampler()
{
    delete [] filter_bank;
}

long AudioStreamResampler::Downsample(void* input, long samplesIn, void* output, long samplesOut)
{
    long lActualSamples = 0;

    // Downsampling is even worse because we have overlap to the left and to the
    // right of the interpolated point.
    //
    // We need (n/2) points to the left and (n/2-1) points to the right.

    while (samplesIn > 0 && samplesOut > 0) {
        long srcSamples, dstSamples;
        int nhold;

        // Figure out how many source samples we need.
        //
        // To do this, compute the highest fixed-point accumulator we'll reach.
        // Truncate that, and add the filter width.  Then subtract however many
        // samples are sitting at the bottom of the buffer.

        srcSamples = (long)(((__int64)samp_frac * (samplesOut - 1) + accum) >> 19) + filter_width - holdover;

        // Don't exceed the buffer (BUFFER_SIZE - holdover).

        if (srcSamples > BUFFER_SIZE - holdover) {
            srcSamples = BUFFER_SIZE - holdover;
        }

        // Read into buffer.

        srcSamples = std::min(srcSamples, samplesIn);
        if (!srcSamples) {
            break;
        }

        memcpy((char*)cbuffer + holdover * bps, (char*)input, srcSamples * bps);
        input = (void*)((char*)input + srcSamples * bps);

        // Figure out how many destination samples we'll get out of what we
        // read.  We'll have (srcSamples+holdover) bytes, so the maximum
        // fixed-pt accumulator we can hit is
        // (srcSamples+holdover-filter_width)<<16 + 0xffff.

        dstSamples = (((__int64)(srcSamples + holdover - filter_width) << 19) + 0x7ffff - accum) / samp_frac + 1;

        if (dstSamples > samplesOut) {
            dstSamples = samplesOut;
        }

        if (dstSamples >= 1) {
            if (filter_bank) {
                accum = dnsampleRout(output, cbuffer, filter_bank, filter_width, accum, samp_frac, dstSamples);
            } else {
                accum = ptsampleRout(output, cbuffer, accum, samp_frac, dstSamples);
            }

            output = (void*)((char*)output + bps * dstSamples);
            lActualSamples += dstSamples;
            samplesOut -= dstSamples;
        }

        // We're "shifting" the new samples down to the bottom by discarding
        // all the samples in the buffer, so adjust the fixed-pt accum
        // accordingly.

        accum -= ((srcSamples + holdover) << 19);

        // Oops, did we need some of those?
        //
        // If accum=0, we need (n/2) samples back.  accum>=0x10000 is fewer,
        // accum<0 is more.

        nhold = - (accum >> 19);

        //ASSERT(nhold <= (filter_width / 2));

        if (nhold > 0) {
            memmove(cbuffer, (char*)cbuffer + bps * (srcSamples + holdover - nhold), bps * nhold);
            holdover = nhold;
            accum += nhold << 19;
        } else {
            holdover = 0;
        }

        //ASSERT(accum >= 0);
    }

    int Bytes = lActualSamples * bps;
    UNREFERENCED_PARAMETER(Bytes);

    return lActualSamples;
}
