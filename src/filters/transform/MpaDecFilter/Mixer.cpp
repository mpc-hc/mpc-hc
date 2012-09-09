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
#include "ffmpeg/libavutil/samplefmt.h"
#include "ffmpeg/libavutil/opt.h"
#include "ffmpeg/libavresample/avresample.h"
}
#pragma warning(default: 4005)

CMixer::CMixer()
    : m_pAVRCxt(NULL)
{
}

void CMixer::Reset()
{
    avresample_free(&m_pAVRCxt);
}

HRESULT CMixer::Mixing(float* pOutput, WORD out_ch, DWORD out_layout, float* pInput, int samples, WORD in_ch, DWORD in_layout)
{
    if (in_layout == out_layout) {
        return S_FALSE;
    }
    int ret = 0;

    if (!m_pAVRCxt) {
        // Allocate Resample Context and set options.
        m_pAVRCxt = avresample_alloc_context();
        av_opt_set_int(m_pAVRCxt, "in_channel_layout", in_layout, 0);
        av_opt_set_int(m_pAVRCxt, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
        av_opt_set_int(m_pAVRCxt, "out_channel_layout", out_layout, 0);
        av_opt_set_int(m_pAVRCxt, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

        // Open Resample Context
        ret = avresample_open(m_pAVRCxt);
        if (ret < 0) {
            TRACE(_T("avresample_open failed\n"));
            avresample_free(&m_pAVRCxt);
            return S_FALSE;
        }

        // Create Matrix
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
                TRACE(_T("avresample_build_matrix failed\n"));
                av_free(matrix_dbl);
                avresample_free(&m_pAVRCxt);
                return S_FALSE;
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
            TRACE(_T("avresample_set_matrix failed\n"));
            avresample_free(&m_pAVRCxt);
            return S_FALSE;
        }
    }

    if (m_pAVRCxt) {
        ret = avresample_convert(m_pAVRCxt, (void**)&pOutput, samples * out_ch, samples, (void**)&pInput, samples * in_ch, samples);
        if (ret < 0) {
            TRACE(_T("avresample_convert failed\n"));
            return S_FALSE;
        }
    }

    return S_OK;
}
