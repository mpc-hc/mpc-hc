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
    : m_pAVRCxt(nullptr)
    , m_matrix_dbl(nullptr)
    , m_in_avsf(AV_SAMPLE_FMT_NONE)
    , m_in_layout(0)
    , m_out_layout(0)
    , m_matrix_norm(0.0f)
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

void CMixer::Init(AVSampleFormat in_avsf, DWORD in_layout, DWORD out_layout, float matrix_norm, int in_samplerate, int out_samplerate)
{
    // reset parameters
    m_in_avsf        = AV_SAMPLE_FMT_NONE;
    m_in_layout      = 0;
    m_out_layout     = 0;
    m_matrix_norm    = 0.0f;
    m_in_samplerate  = 0;
    m_out_samplerate = 0;
    av_free(m_matrix_dbl);

    // Close Resample Context
    avresample_close(m_pAVRCxt);

    int ret = 0;
    // Set options
    av_opt_set_int(m_pAVRCxt, "in_sample_fmt",      in_avsf,           0);
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
        const double surround_mix_level = 1.0;
        const double lfe_mix_level      = 1.0;
        const int normalize = 0;
        ret = avresample_build_matrix(in_layout, out_layout, center_mix_level, surround_mix_level, lfe_mix_level, normalize, m_matrix_dbl, in_ch, AV_MATRIX_ENCODING_NONE);
        if (ret < 0) {
            TRACE(_T("Mixer: avresample_build_matrix failed\n"));
            av_free(m_matrix_dbl);
            return;
        }

        // if back channels do not have sound, then divide side channels for the back and side
        if (out_layout == AV_CH_LAYOUT_7POINT1) {
            bool back_no_sound = true;
            for (int i = 0; i < in_ch * 2; i++) {
                if (m_matrix_dbl[4 * in_ch + i] != 0.0) {
                    back_no_sound = false;
                }
            }
            if (back_no_sound) {
                for (int i = 0; i < in_ch * 2; i++) {
                    m_matrix_dbl[4 * in_ch + i] = (m_matrix_dbl[6 * in_ch + i] *= M_SQRT1_2);
                }
            }
        }
    }

    if (matrix_norm > 0.0f && matrix_norm <= 1.0f) { // 0.0 - normalize off; 1.0 - full normalize matrix
        double max_peak = 0;
        for (int j = 0; j < out_ch; j++) {
            double peak = 0;
            for (int i = 0; i < in_ch; i++) {
                peak += fabs(m_matrix_dbl[j * in_ch + i]);
            }
            if (peak > max_peak) {
                max_peak = peak;
            }
        }
        if (max_peak > 1.0) {
            double g = ((max_peak - 1.0) * (1.0 - matrix_norm) + 1.0) / max_peak;
            for (int i = 0, n = in_ch * out_ch; i < n; i++) {
                m_matrix_dbl[i] *= g;
            }
        }
    }

#ifdef _DEBUG
    CString matrix_str = _T("Mixer: matrix\n");
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
    m_matrix_norm    = matrix_norm;
    m_in_samplerate  = in_samplerate;
    m_out_samplerate = out_samplerate;
}

void CMixer::Update(AVSampleFormat in_avsf, DWORD in_layout, DWORD out_layout, float matrix_norm, int in_samplerate, int out_samplerate)
{
    if (in_avsf != m_in_avsf ||
            in_layout != m_in_layout ||
            out_layout != m_out_layout ||
            matrix_norm != m_matrix_norm ||
            in_samplerate != m_in_samplerate ||
            out_samplerate != m_out_samplerate) {
        Init(in_avsf, in_layout, out_layout, matrix_norm, in_samplerate, out_samplerate);
    }
}

int CMixer::Mixing(float* pOutput, int out_samples, BYTE* pInput, int in_samples)
{
    int in_ch  = av_popcount(m_in_layout);
    int out_ch = av_popcount(m_out_layout);

    int in_plane_nb   = av_sample_fmt_is_planar(m_in_avsf) ? in_ch : 1;
    int in_plane_size = in_samples * (av_sample_fmt_is_planar(m_in_avsf) ? 1 : in_ch) * av_get_bytes_per_sample(m_in_avsf);
    static BYTE* ppInput[AVRESAMPLE_MAX_CHANNELS];
    for (int i = 0; i < in_plane_nb; i++) {
        ppInput[i] = pInput + i * in_plane_size;
    }

    int out_plane_size = out_samples * out_ch * sizeof(float);

    out_samples = avresample_convert(m_pAVRCxt, (uint8_t**)&pOutput, out_plane_size, out_samples, ppInput, in_plane_size, in_samples);
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
        ret = avresample_set_matrix(m_pAVRCxt, m_matrix_dbl, av_popcount(m_in_layout));
        if (ret < 0) {
            TRACE(_T("Mixer: avresample_set_matrix failed\n"));
            return;
        }
    }
}
