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

#pragma once

typedef long(*AudioPointSampler)(void*, void*, long, long, long);
typedef long(*AudioDownSampler)(void*, void*, long*, int, long, long, long);

class AudioStreamResampler
{
private:
    AudioPointSampler ptsampleRout;
    AudioDownSampler dnsampleRout;
    long samp_frac;
    long accum;
    int holdover;
    long* filter_bank;
    int filter_width;

    enum { BUFFER_SIZE = 512 };
    BYTE cbuffer[4 * BUFFER_SIZE];
    int bps;

public:
    AudioStreamResampler(int bps, long orig_rate, long new_rate, bool fHighQuality);
    ~AudioStreamResampler();

    AudioStreamResampler(const AudioStreamResampler&) = delete;
    AudioStreamResampler& operator=(const AudioStreamResampler&) = delete;

    long Downsample(void* input, long samplesIn, void* output, long samplesOut);
};
