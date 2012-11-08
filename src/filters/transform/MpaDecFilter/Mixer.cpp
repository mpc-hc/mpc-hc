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

CMixer::CMixer()
    : m_pAVRCxt(NULL)
    , last_in_layout(0)
    , last_out_layout(0)
    , last_in_avsf(AV_SAMPLE_FMT_NONE)
{
}

CMixer::~CMixer()
{
    avresample_free(&m_pAVRCxt);
}

void CMixer::Init(DWORD out_layout, DWORD in_layout, enum AVSampleFormat in_avsf)
{
    avresample_free(&m_pAVRCxt);

    int ret = 0;
    // Allocate Resample Context and set options.
    m_pAVRCxt = avresample_alloc_context();
    av_opt_set_int(m_pAVRCxt, "in_channel_layout", in_layout, 0);
    av_opt_set_int(m_pAVRCxt, "in_sample_fmt", in_avsf, 0);
    av_opt_set_int(m_pAVRCxt, "out_channel_layout", out_layout, 0);
    av_opt_set_int(m_pAVRCxt, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0); // forced float output

    // Open Resample Context
    ret = avresample_open(m_pAVRCxt);
    if (ret < 0) {
        TRACE(_T("Mixer: avresample_open failed\n"));
        avresample_free(&m_pAVRCxt);
        return;
    }

    // Create Matrix
    int in_ch  = av_popcount(in_layout);
    int out_ch = av_popcount(out_layout);
    double* matrix_dbl = (double*)av_mallocz(in_ch * out_ch * sizeof(*matrix_dbl));
    // expand stereo
    if (in_layout == AV_CH_LAYOUT_STEREO && (out_layout == AV_CH_LAYOUT_QUAD || out_layout == AV_CH_LAYOUT_5POINT1 || out_layout == AV_CH_LAYOUT_7POINT1)) {
        matrix_dbl[0] = 1.0;
        matrix_dbl[1] = 0.0;
        matrix_dbl[2] = 0.0;
        matrix_dbl[3] = 1.0;
        if (out_layout == AV_CH_LAYOUT_QUAD) {
            matrix_dbl[4] = 0.5;
            matrix_dbl[5] = (-0.5);
            matrix_dbl[6] = (-0.5);
            matrix_dbl[7] = 0.5;
        } else if (out_layout == AV_CH_LAYOUT_5POINT1 || out_layout == AV_CH_LAYOUT_7POINT1) {
            matrix_dbl[4] = 0.5;
            matrix_dbl[5] = 0.5;
            matrix_dbl[6] = 0.0;
            matrix_dbl[7] = 0.0;
            matrix_dbl[8] = 0.5;
            matrix_dbl[9] = (-0.5);
            matrix_dbl[10] = (-0.5);
            matrix_dbl[11] = 0.5;
            if (out_layout == AV_CH_LAYOUT_7POINT1) {
                matrix_dbl[12] = 0.5;
                matrix_dbl[13] = (-0.5);
                matrix_dbl[14] = (-0.5);
                matrix_dbl[15] = 0.5;
            }
        }
    } else {
        const double center_mix_level   = M_SQRT1_2;
        const double surround_mix_level = M_SQRT1_2;
        const double lfe_mix_level      = M_SQRT1_2;
        const int normalize = 0;
        ret = avresample_build_matrix(in_layout, out_layout, center_mix_level, surround_mix_level, lfe_mix_level, normalize, matrix_dbl, in_ch, AV_MATRIX_ENCODING_NONE);
        if (ret < 0) {
            TRACE(_T("Mixer: avresample_build_matrix failed\n"));
            av_free(matrix_dbl);
            avresample_free(&m_pAVRCxt);
            return;
        }
    }

#ifdef _DEBUG
    CString matrix_str;
    for (int j = 0; j < out_ch; j++) {
        matrix_str.AppendFormat(_T("%d:"), j + 1);
        for (int i = 0; i < in_ch; i++) {
            double k = matrix_dbl[j * in_ch + i];
            matrix_str.AppendFormat(_T(" %.4f"), k);
        }
        matrix_str += _T("\n");
    }
    TRACE(matrix_str);
#endif

    // Set Matrix on the context
    ret = avresample_set_matrix(m_pAVRCxt, matrix_dbl, in_ch);
    av_free(matrix_dbl);
    if (ret < 0) {
        TRACE(_T("Mixer: avresample_set_matrix failed\n"));
        avresample_free(&m_pAVRCxt);
        return;
    }

    last_in_layout  = in_layout;
    last_out_layout = out_layout;
    last_in_avsf      = in_avsf;
}

HRESULT CMixer::Mixing(float* pOutput, int out_samples, DWORD out_layout, BYTE* pInput, int in_samples, DWORD in_layout, enum AVSampleFormat in_avsf)
{
    if (in_layout == out_layout) {
        return E_ABORT; // do nothing
    }
    if (in_avsf < AV_SAMPLE_FMT_U8 || in_avsf > AV_SAMPLE_FMT_DBL) { // planar audio is not supported (ffmprg crashed)
        return E_INVALIDARG;
    }

    if (!m_pAVRCxt || in_layout != last_in_layout || out_layout != last_out_layout || in_avsf != last_in_avsf) {
        Init(out_layout, in_layout, in_avsf);
    }
    if (!m_pAVRCxt) {
        return E_FAIL;
    }

    int in_plane_size  = in_samples * (av_sample_fmt_is_planar(in_avsf) ? 1 : av_popcount(in_layout)) * av_get_bytes_per_sample(in_avsf);
    int out_plane_size = out_samples * av_popcount(out_layout) * sizeof(float);

    int ret = avresample_convert(m_pAVRCxt, (uint8_t**)&pOutput, in_plane_size, out_samples, (uint8_t**)&pInput, out_plane_size, in_samples);
    if (ret < 0) {
        TRACE(_T("Mixer: avresample_convert failed\n"));
        return E_FAIL;
    }

    return S_OK;
}
