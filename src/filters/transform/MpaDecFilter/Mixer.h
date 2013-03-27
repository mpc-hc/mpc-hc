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

struct AVAudioResampleContext;

class CMixer
{
protected:
    AVAudioResampleContext* m_pAVRCxt;
    double* m_matrix_dbl;

    enum AVSampleFormat m_in_avsf;
    DWORD   m_in_layout;
    DWORD   m_out_layout;
    float   m_matrix_norm;
    int     m_in_samplerate;
    int     m_out_samplerate;

    void Init(AVSampleFormat in_avsf, DWORD in_layout, DWORD out_layout, float matrix_norm = 0.0f, int in_samplerate = 48000, int out_samplerate = 48000);

public:
    CMixer();
    ~CMixer();

    void Update(AVSampleFormat in_avsf, DWORD in_layout, DWORD out_layout, float matrix_norm = 0.0f, int in_samplerate = 48000, int out_samplerate = 48000);
    int  Mixing(float* pOutput, int out_samples, BYTE* pInput, int in_samples);

    int  CalcOutSamples(int in_samples); // needed when using resampling
    void FlushBuffers(); // needed when using resampling
};
