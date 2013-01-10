/*
 * (C) 2012 see Authors.txt
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
#include "Mixer.h"

#pragma warning(disable: 4005)
extern "C" {
#include "ffmpeg/libavresample/avresample.h"
#include "ffmpeg/libavutil/samplefmt.h"
#include "ffmpeg/libavutil/opt.h"
}
#pragma warning(default: 4005)
#include "AudioHelper.h"

CMixer::CMixer()
    : m_pAVRCxt(NULL)
    , m_matrix_dbl(NULL)
    , m_in_avsf(AV_SAMPLE_FMT_NONE)
    , m_in_avsf_used(AV_SAMPLE_FMT_NONE)
    , m_in_layout(0)
    , m_out_layout(0)
    , m_in_samplerate(0)
    , m_out_samplerate(0)
{
    // Allocate Resample Context
    m_pAVRCxt = avresample_alloc_context();
}

CMixer::~CMixer()
{
    avresample_free(&m_pAVRCxt);
    av_free(m_matrix_dbl);
}

void CMixer::Init(AVSampleFormat in_avsf, DWORD in_layout, DWORD out_layout, int in_samplerate, int out_samplerate)
{
    // reset parameters
    m_in_avsf        = AV_SAMPLE_FMT_NONE;
    m_in_layout      = 0;
    m_out_layout     = 0;
    m_in_samplerate  = 0;
    m_out_samplerate = 0;
    av_free(m_matrix_dbl);

    // Close Resample Context
    avresample_close(m_pAVRCxt);

    if (in_avsf >= AV_SAMPLE_FMT_U8P && in_avsf <= AV_SAMPLE_FMT_DBLP) { // planar audio is not supported (ffmpeg crashed)
        m_in_avsf_used = AV_SAMPLE_FMT_FLT; // convert to float
    } else {
        m_in_avsf_used = in_avsf;
    }

    int ret = 0;
    // Set options
    av_opt_set_int(m_pAVRCxt, "in_sample_fmt",      m_in_avsf_used,    0);
    av_opt_set_int(m_pAVRCxt, "out_sample_fmt",     AV_SAMPLE_FMT_FLT, 0); // forced float output
    av_opt_set_int(m_pAVRCxt, "in_channel_layout",  in_layout,         0);
    av_opt_set_int(m_pAVRCxt, "out_channel_layout", out_layout,        0);
    av_opt_set_int(m_pAVRCxt, "in_sample_rate",     in_samplerate,     0);
    av_opt_set_int(m_pAVRCxt, "out_sample_rate",    out_samplerate,    0);

    // Open Resample Context
    ret = avresample_open(m_pAVRCxt);
    if (ret < 0) {
        TRACE(_T("Mixer: avresample_open failed\n"));
        return;
    }

    // Create Matrix
    int in_ch  = av_popcount(in_layout);
    int out_ch = av_popcount(out_layout);
    m_matrix_dbl = (double*)av_mallocz(in_ch * out_ch * sizeof(*m_matrix_dbl));
    // expand stereo
    if (in_layout == AV_CH_LAYOUT_STEREO && (out_layout == AV_CH_LAYOUT_QUAD || out_layout == AV_CH_LAYOUT_5POINT1 || out_layout == AV_CH_LAYOUT_7POINT1)) {
        m_matrix_dbl[0] = 1.0;
        m_matrix_dbl[1] = 0.0;
        m_matrix_dbl[2] = 0.0;
        m_matrix_dbl[3] = 1.0;
        if (out_layout == AV_CH_LAYOUT_QUAD) {
            m_matrix_dbl[4] = 0.6666;
            m_matrix_dbl[5] = (-0.2222);
            m_matrix_dbl[6] = (-0.2222);
            m_matrix_dbl[7] = 0.6666;
        } else if (out_layout == AV_CH_LAYOUT_5POINT1 || out_layout == AV_CH_LAYOUT_7POINT1) {
            m_matrix_dbl[4] = 0.5;
            m_matrix_dbl[5] = 0.5;
            m_matrix_dbl[6] = 0.0;
            m_matrix_dbl[7] = 0.0;
            m_matrix_dbl[8] =  0.6666;
            m_matrix_dbl[9] = (-0.2222);
            m_matrix_dbl[10] = (-0.2222);
            m_matrix_dbl[11] = 0.6666;
            if (out_layout == AV_CH_LAYOUT_7POINT1) {
                m_matrix_dbl[12] = 0.6666;
                m_matrix_dbl[13] = (-0.2222);
                m_matrix_dbl[14] = (-0.2222);
                m_matrix_dbl[15] = 0.6666;
            }
        }
    } else {
        const double center_mix_level   = M_SQRT1_2;
        const double surround_mix_level = M_SQRT1_2;
        const double lfe_mix_level      = M_SQRT1_2;
        const int normalize = 0;
        ret = avresample_build_matrix(in_layout, out_layout, center_mix_level, surround_mix_level, lfe_mix_level, normalize, m_matrix_dbl, in_ch, AV_MATRIX_ENCODING_NONE);
        if (ret < 0) {
            TRACE(_T("Mixer: avresample_build_matrix failed\n"));
            av_free(m_matrix_dbl);
            return;
        }
    }

#ifdef _DEBUG
    CString matrix_str;
    for (int j = 0; j < out_ch; j++) {
        matrix_str.AppendFormat(_T("%d:"), j + 1);
        for (int i = 0; i < in_ch; i++) {
            double k = m_matrix_dbl[j * in_ch + i];
            matrix_str.AppendFormat(_T(" %.4f"), k);
        }
        matrix_str += _T("\n");
    }
    TRACE(matrix_str);
#endif

    // Set Matrix on the context
    ret = avresample_set_matrix(m_pAVRCxt, m_matrix_dbl, in_ch);
    if (ret < 0) {
        TRACE(_T("Mixer: avresample_set_matrix failed\n"));
        av_free(m_matrix_dbl);
        return;
    }

    m_in_avsf        = in_avsf;
    m_in_layout      = in_layout;
    m_out_layout     = out_layout;
    m_in_samplerate  = in_samplerate;
    m_out_samplerate = out_samplerate;
}

void CMixer::Update(AVSampleFormat in_avsf, DWORD in_layout, DWORD out_layout, int in_samplerate, int out_samplerate)
{
    if (in_avsf != m_in_avsf ||
            in_layout != m_in_layout ||
            out_layout != m_out_layout ||
            in_samplerate != m_in_samplerate ||
            out_samplerate != m_out_samplerate) {
        Init(in_avsf, in_layout, out_layout, in_samplerate, out_samplerate);
    }
}

int CMixer::Mixing(float* pOutput, int out_samples, BYTE* pInput, int in_samples)
{
    int in_ch  = av_popcount(m_in_layout);
    int out_ch = av_popcount(m_out_layout);

    float* buf  = NULL;
    if (m_in_avsf != m_in_avsf_used) { // need convert
        buf = new float[in_samples * in_ch];
        convert_to_float(m_in_avsf, (WORD)in_ch, in_samples, pInput, buf); // convert to float
        pInput = (BYTE*)buf;
    }

    // int in_plane_size  = in_samples * (av_sample_fmt_is_planar(in_avsf) ? 1 : in_ch) * av_get_bytes_per_sample(in_avsf);
    int in_plane_size  = in_samples * in_ch * av_get_bytes_per_sample(m_in_avsf_used);
    int out_plane_size = out_samples * out_ch * sizeof(float);

    out_samples = avresample_convert(m_pAVRCxt, (uint8_t**)&pOutput, out_plane_size, out_samples, (uint8_t**)&pInput, in_plane_size, in_samples);
    if (buf) {
        delete [] buf;
    }
    if (out_samples < 0) {
        TRACE(_T("Mixer: avresample_convert failed\n"));
        return 0;
    }

    return out_samples;
}

int CMixer::CalcOutSamples(int in_samples)
{
    if (m_in_samplerate == m_out_samplerate) {
        return in_samples;
    } else {
        return avresample_available(m_pAVRCxt) + (int)((__int64)(avresample_get_delay(m_pAVRCxt) + in_samples) * m_out_samplerate / m_in_samplerate);
    }
}

void CMixer::FlushBuffers()
{
    if (m_in_samplerate != m_out_samplerate) {
        // Close Resample Context
        avresample_close(m_pAVRCxt);

        int ret = 0;
        // Open Resample Context
        ret = avresample_open(m_pAVRCxt);
        if (ret < 0) {
            TRACE(_T("Mixer: avresample_open failed\n"));
            return;
        }

        // Set Matrix on the context
        ret = avresample_set_matrix(m_pAVRCxt, m_matrix_dbl, m_in_layout);
        if (ret < 0) {
            TRACE(_T("Mixer: avresample_set_matrix failed\n"));
            return;
        }
    }
}
