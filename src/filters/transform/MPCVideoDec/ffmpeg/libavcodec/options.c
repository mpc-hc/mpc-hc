/*
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file libavcodec/options.c
 * Options definition for AVCodecContext.
 */

#include "avcodec.h"

static const char* context_to_name(void* ptr) {
    AVCodecContext *avc= ptr;

    if(avc && avc->codec && avc->codec->name)
        return avc->codec->name;
    else
        return "NULL";
}

static AVClass av_codec_context_class = { "AVCodecContext", context_to_name };

void avcodec_get_context_defaults(AVCodecContext *s){
    memset(s, 0, sizeof(AVCodecContext));

    s->av_class= &av_codec_context_class;

    s->time_base.num=0; s->time_base.den=1;

    s->get_buffer= avcodec_default_get_buffer;
    s->release_buffer= avcodec_default_release_buffer;
    s->get_format= avcodec_default_get_format;
    s->execute= avcodec_default_execute;
    s->execute2= avcodec_default_execute2;
    s->sample_aspect_ratio.num=0; s->sample_aspect_ratio.den=1;
    s->pix_fmt= PIX_FMT_NONE;
    s->sample_fmt= SAMPLE_FMT_S16; // FIXME: set to NONE

    s->palctrl = NULL;
    s->reget_buffer= avcodec_default_reget_buffer;
    s->reordered_opaque= AV_NOPTS_VALUE;
    
    s->bit_rate= 800*1000;
    s->bit_rate_tolerance= s->bit_rate*10;
    s->qmin= 2;
    s->qmax= 31;
    s->mb_lmin= FF_QP2LAMBDA * 2;
    s->mb_lmax= FF_QP2LAMBDA * 31;
    s->cqp = -1;
    s->refs = 1;
    s->directpred = 2;
    s->qcompress= 0.5;
    s->complexityblur = 20.0;
    s->keyint_min = 25;
    s->flags2 = CODEC_FLAG2_FASTPSKIP;
    s->max_qdiff= 3;
    s->b_quant_factor=1.25;
    s->b_quant_offset=1.25;
    s->i_quant_factor=-0.8;
    s->i_quant_offset=0.0;
    s->error_concealment= 3;
    s->error_recognition= 1;
    s->workaround_bugs= FF_BUG_AUTODETECT;
    s->gop_size= 50;
    s->me_method= ME_EPZS;   
    s->thread_count=1;
    s->me_subpel_quality=8;
    s->lmin= FF_QP2LAMBDA * s->qmin;
    s->lmax= FF_QP2LAMBDA * s->qmax;
    s->ildct_cmp= FF_CMP_VSAD;
    s->profile= FF_PROFILE_UNKNOWN;
    s->level= FF_LEVEL_UNKNOWN;
    s->me_penalty_compensation= 256;
    s->frame_skip_cmp= FF_CMP_DCTMAX;
    s->nsse_weight= 8;
    s->mv0_threshold= 256;
    s->b_sensitivity= 40;
    s->compression_level = FF_COMPRESSION_DEFAULT;
    s->use_lpc = -1;
    s->min_prediction_order = -1;
    s->max_prediction_order = -1;
    s->prediction_order_method = -1;
    s->min_partition_order = -1;
    s->max_partition_order = -1;
    s->intra_quant_bias= FF_DEFAULT_QUANT_BIAS;
    s->inter_quant_bias= FF_DEFAULT_QUANT_BIAS;
    s->rc_max_available_vbv_use = 1.0/3;
    s->rc_min_vbv_overflow_use = 3;
    s->bidir_refine = 1;
}

AVCodecContext *avcodec_alloc_context(void){
    AVCodecContext *avctx= av_malloc(sizeof(AVCodecContext));

    if(avctx==NULL) return NULL;

    avcodec_get_context_defaults(avctx);

    return avctx;
}
