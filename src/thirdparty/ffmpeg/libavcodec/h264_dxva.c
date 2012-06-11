/*
 * (C) 2008-2012 see Authors.txt
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

#include <windows.h>
#include <dxva.h>


static void fill_dxva_slice_long(H264Context *h)
{
    MpegEncContext* const s = &h->s;
    DXVA_Slice_H264_Long* pSlice = &((DXVA_Slice_H264_Long*) h->dxva_slice_long)[h->current_slice - 1];
    unsigned              i, list;

    memset(pSlice, 0, sizeof(DXVA_Slice_H264_Long));

    pSlice->slice_id                     = h->current_slice - 1;
    pSlice->first_mb_in_slice            = h->first_mb_in_slice;
    pSlice->NumMbsForSlice               = 0; // h->s.mb_num;          // TODO : to be checked !
    pSlice->BitOffsetToSliceData         = h->bit_offset_to_slice_data;
    pSlice->slice_type                   = h->raw_slice_type;
    pSlice->luma_log2_weight_denom       = h->luma_log2_weight_denom;
    pSlice->chroma_log2_weight_denom     = h->chroma_log2_weight_denom;
    pSlice->slice_alpha_c0_offset_div2   = h->slice_alpha_c0_offset / 2;
    pSlice->slice_beta_offset_div2       = h->slice_beta_offset / 2;
    pSlice->Reserved8Bits                = 0;

    pSlice->num_ref_idx_l0_active_minus1 = 0;
    pSlice->num_ref_idx_l1_active_minus1 = 0;
    if (h->list_count > 0) {
        pSlice->num_ref_idx_l0_active_minus1 = h->ref_count[0] - 1;
    }
    if (h->list_count > 1) {
        pSlice->num_ref_idx_l1_active_minus1 = h->ref_count[1] - 1;
    }

    // Fill prediction weights
    memset (pSlice->Weights, 0, sizeof(pSlice->Weights));
    for (list = 0; list < 2; list++) {
        for (i = 0; i < 32; i++) {
            if (list < h->list_count && i < h->ref_count[list]) {
                const Picture *r = &h->ref_list[list][i];
                unsigned plane;
                for (plane = 0; plane < 3; plane++) {
                    int w, o;
                    if (plane == 0 && h->luma_weight_flag[list]) {
                        w = h->luma_weight[i][list][0];
                        o = h->luma_weight[i][list][1];
                    } else if (plane >= 1 && h->chroma_weight_flag[list]) {
                        w = h->chroma_weight[i][list][plane - 1][0];
                        o = h->chroma_weight[i][list][plane - 1][1];
                    } else {
                        w = 1 << (plane == 0 ? h->luma_log2_weight_denom :
                                  h->chroma_log2_weight_denom);
                        o = 0;
                    }
                    pSlice->Weights[list][i][plane][0] = w;
                    pSlice->Weights[list][i][plane][1] = o;
                }
            } else {
                unsigned plane;
                for (plane = 0; plane < 3; plane++) {
                    pSlice->Weights[list][i][plane][0] = 0;
                    pSlice->Weights[list][i][plane][1] = 0;
                }
            }
        }
    }

    pSlice->slice_qs_delta                = h->slice_qs_delta;
    pSlice->slice_qp_delta                = h->slice_qp_delta;
    pSlice->redundant_pic_cnt             = h->redundant_pic_count;
    pSlice->direct_spatial_mv_pred_flag   = h->direct_spatial_mv_pred;
    pSlice->cabac_init_idc                = h->cabac_init_idc;
    pSlice->disable_deblocking_filter_idc = h->deblocking_filter;

    for (i = 0; i < 32; i++) {
        pSlice->RefPicList[0][i].AssociatedFlag = 1;
        pSlice->RefPicList[0][i].bPicEntry = 255;
        pSlice->RefPicList[0][i].Index7Bits = 127;
        pSlice->RefPicList[1][i].AssociatedFlag = 1;
        pSlice->RefPicList[1][i].bPicEntry = 255;
        pSlice->RefPicList[1][i].Index7Bits = 127;
    }
}

static void decode_postinit_dxva(H264Context *h, int* nOutPOC, int64_t* rtStartTime)
{
    MpegEncContext *const s = &h->s;
    Picture *out = s->current_picture_ptr;
    Picture *cur = s->current_picture_ptr;
    int i, pics, out_of_order, out_idx;

    s->current_picture_ptr->f.qscale_type = FF_QSCALE_TYPE_H264;
    s->current_picture_ptr->f.pict_type   = s->pict_type;

    if (h->next_output_pic)
        return;

    if (cur->field_poc[0] == INT_MAX || cur->field_poc[1] == INT_MAX) {
        /* FIXME: if we have two PAFF fields in one packet, we can't start
         * the next thread here. If we have one field per packet, we can.
         * The check in decode_nal_units() is not good enough to find this
         * yet, so we assume the worst for now. */
        // if (setup_finished)
        //    ff_thread_finish_setup(s->avctx);
        return;
    }

    cur->f.interlaced_frame = 0;
    cur->f.repeat_pict      = 0;

    /* Signal interlacing information externally. */
    /* Prioritize picture timing SEI information over used
     * decoding process if it exists. */

    if (h->sps.pic_struct_present_flag) {
        switch (h->sei_pic_struct) {
        case SEI_PIC_STRUCT_FRAME:
            break;
        case SEI_PIC_STRUCT_TOP_FIELD:
        case SEI_PIC_STRUCT_BOTTOM_FIELD:
            cur->f.interlaced_frame = 1;
            break;
        case SEI_PIC_STRUCT_TOP_BOTTOM:
        case SEI_PIC_STRUCT_BOTTOM_TOP:
            if (FIELD_OR_MBAFF_PICTURE)
                cur->f.interlaced_frame = 1;
            else
                // try to flag soft telecine progressive
                cur->f.interlaced_frame = h->prev_interlaced_frame;
            break;
        case SEI_PIC_STRUCT_TOP_BOTTOM_TOP:
        case SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM:
            /* Signal the possibility of telecined film externally
             * (pic_struct 5,6). From these hints, let the applications
             * decide if they apply deinterlacing. */
            cur->f.repeat_pict = 1;
            break;
        case SEI_PIC_STRUCT_FRAME_DOUBLING:
            // Force progressive here, doubling interlaced frame is a bad idea.
            cur->f.repeat_pict = 2;
            break;
        case SEI_PIC_STRUCT_FRAME_TRIPLING:
            cur->f.repeat_pict = 4;
            break;
        }

        if ((h->sei_ct_type & 3) &&
                h->sei_pic_struct <= SEI_PIC_STRUCT_BOTTOM_TOP)
            cur->f.interlaced_frame = (h->sei_ct_type & (1 << 1)) != 0;
    } else {
        /* Derive interlacing flag from used decoding process. */
        cur->f.interlaced_frame = FIELD_OR_MBAFF_PICTURE;
    }
    h->prev_interlaced_frame = cur->f.interlaced_frame;

    if (cur->field_poc[0] != cur->field_poc[1]) {
        /* Derive top_field_first from field pocs. */
        cur->f.top_field_first = cur->field_poc[0] < cur->field_poc[1];
    } else {
        if (cur->f.interlaced_frame || h->sps.pic_struct_present_flag) {
            /* Use picture timing SEI information. Even if it is a
             * information of a past frame, better than nothing. */
            if (h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM ||
                    h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM_TOP)
                cur->f.top_field_first = 1;
            else
                cur->f.top_field_first = 0;
        } else {
            /* Most likely progressive */
            cur->f.top_field_first = 0;
        }
    }

    cur->mmco_reset = h->mmco_reset;
    h->mmco_reset = 0;
    // FIXME do something with unavailable reference frames

    /* Sort B-frames into display order */

    if (h->sps.bitstream_restriction_flag &&
            s->avctx->has_b_frames < h->sps.num_reorder_frames) {
        s->avctx->has_b_frames = h->sps.num_reorder_frames;
        s->low_delay           = 0;
    }

    if (s->avctx->strict_std_compliance >= FF_COMPLIANCE_STRICT &&
            !h->sps.bitstream_restriction_flag) {
        s->avctx->has_b_frames = MAX_DELAYED_PIC_COUNT - 1;
        s->low_delay           = 0;
    }

    for (i = 0; 1; i++) {
        if (i == MAX_DELAYED_PIC_COUNT || cur->poc < h->last_pocs[i]) {
            if (i)
                h->last_pocs[i - 1] = cur->poc;
            break;
        } else if (i) {
            h->last_pocs[i - 1] = h->last_pocs[i];
        }
    }
    out_of_order = MAX_DELAYED_PIC_COUNT - i;
    if (   cur->f.pict_type == AV_PICTURE_TYPE_B
            || (h->last_pocs[MAX_DELAYED_PIC_COUNT - 2] > INT_MIN && h->last_pocs[MAX_DELAYED_PIC_COUNT - 1] - h->last_pocs[MAX_DELAYED_PIC_COUNT - 2] > 2))
        out_of_order = FFMAX(out_of_order, 1);
    if (s->avctx->has_b_frames < out_of_order && !h->sps.bitstream_restriction_flag) {
        av_log(s->avctx, AV_LOG_WARNING, "Increasing reorder buffer to %d\n", out_of_order);
        s->avctx->has_b_frames = out_of_order;
        s->low_delay = 0;
    }

    pics = 0;
    while (h->delayed_pic[pics])
        pics++;

    av_assert0(pics <= MAX_DELAYED_PIC_COUNT);

    h->delayed_pic[pics++] = cur;
    if (cur->f.reference == 0)
        cur->f.reference = DELAYED_PIC_REF;

    out = h->delayed_pic[0];
    out_idx = 0;
    for (i = 1; h->delayed_pic[i] &&
            !h->delayed_pic[i]->f.key_frame &&
            !h->delayed_pic[i]->mmco_reset;
            i++)
        if (h->delayed_pic[i]->poc < out->poc) {
            out     = h->delayed_pic[i];
            out_idx = i;
        }
    if (s->avctx->has_b_frames == 0 &&
            (h->delayed_pic[0]->f.key_frame || h->delayed_pic[0]->mmco_reset))
        h->next_outputed_poc = INT_MIN;
    out_of_order = out->poc < h->next_outputed_poc;

    if (out_of_order || pics > s->avctx->has_b_frames) {
        out->f.reference &= ~DELAYED_PIC_REF;
        // for frame threading, the owner must be the second field's thread or
        // else the first thread can release the picture and reuse it unsafely
        out->owner2       = s;
        for (i = out_idx; h->delayed_pic[i]; i++)
            h->delayed_pic[i] = h->delayed_pic[i + 1];
    }
    if (!out_of_order && pics > s->avctx->has_b_frames) {
        h->next_output_pic = out;
        if (out_idx == 0 && h->delayed_pic[0] && (h->delayed_pic[0]->f.key_frame || h->delayed_pic[0]->mmco_reset)) {
            h->next_outputed_poc = INT_MIN;
        } else
            h->next_outputed_poc = out->poc;
        // ==> Start patch MPC DXVA
        if (nOutPOC)     *nOutPOC     = h->next_outputed_poc;
        if (rtStartTime) *rtStartTime = out->f.reordered_opaque;
        // <== End patch MPC DXVA
    } else {
        av_log(s->avctx, AV_LOG_DEBUG, "no picture %s\n", out_of_order ? "ooo" : "");
    }

    if (h->next_output_pic && h->next_output_pic->sync) {
        h->sync |= 2;
    }
}

static int field_end_noexecute(H264Context *h)
{
    MpegEncContext *const s     = &h->s;
    int err = 0;
    s->mb_y = 0;

    if (!s->dropable) {
        err = ff_h264_execute_ref_pic_marking(h, h->mmco, h->mmco_index);
        h->prev_poc_msb     = h->poc_msb;
        h->prev_poc_lsb     = h->poc_lsb;
    }
    h->prev_frame_num_offset = h->frame_num_offset;
    h->prev_frame_num        = h->frame_num;
    h->outputed_poc = h->next_outputed_poc;

    ff_MPV_frame_end(s);

    h->current_slice = 0;

    return err;
}

int decode_slice_header_noexecute(H264Context *h, H264Context *h0)
{
    MpegEncContext * const s = &h->s;
    MpegEncContext * const s0 = &h0->s;
    unsigned int pps_id;
    int num_ref_idx_active_override_flag;
    unsigned int slice_type, tmp, i, j;
    int default_ref_list_done = 0;
    int last_pic_structure, last_pic_dropable;

    /* FIXME: 2tap qpel isn't implemented for high bit depth. */
    if ((s->avctx->flags2 & CODEC_FLAG2_FAST) &&
            !h->nal_ref_idc && !h->pixel_shift) {
        s->me.qpel_put = s->dsp.put_2tap_qpel_pixels_tab;
        s->me.qpel_avg = s->dsp.avg_2tap_qpel_pixels_tab;
    } else {
        s->me.qpel_put = s->dsp.put_h264_qpel_pixels_tab;
        s->me.qpel_avg = s->dsp.avg_h264_qpel_pixels_tab;
    }

    // ==> Start patch MPC DXVA
    h->first_mb_in_slice = get_ue_golomb_long(&s->gb);
    // <== End patch MPC DXVA

    if (h->first_mb_in_slice == 0) { //FIXME better field boundary detection
        if (h0->current_slice && FIELD_PICTURE) {
            // ==> Start patch MPC DXVA
            field_end_noexecute(h);
            // <== End patch MPC DXVA
        }

        h0->current_slice = 0;
        if (!s0->first_field) {
            if (s->current_picture_ptr && !s->dropable &&
                    s->current_picture_ptr->owner2 == s) {
                ff_thread_report_progress(&s->current_picture_ptr->f, INT_MAX,
                                          s->picture_structure == PICT_BOTTOM_FIELD);
            }
            s->current_picture_ptr = NULL;
        }
    }

    slice_type = get_ue_golomb_31(&s->gb);
    if (slice_type > 9) {
        av_log(h->s.avctx, AV_LOG_ERROR,
               "slice type too large (%d) at %d %d\n",
               h->slice_type, s->mb_x, s->mb_y);
        return -1;
    }
    if (slice_type > 4) {
        slice_type -= 5;
        h->slice_type_fixed = 1;
    } else
        h->slice_type_fixed = 0;

    // ==> Start patch MPC DXVA
    h->raw_slice_type = slice_type;
    // <== End patch MPC DXVA
    slice_type = golomb_to_pict_type[slice_type];
    if (slice_type == AV_PICTURE_TYPE_I ||
            (h0->current_slice != 0 && slice_type == h0->last_slice_type)) {
        default_ref_list_done = 1;
    }
    h->slice_type     = slice_type;
    h->slice_type_nos = slice_type & 3;

    // to make a few old functions happy, it's wrong though
    s->pict_type = h->slice_type;

    pps_id = get_ue_golomb(&s->gb);
    if (pps_id >= MAX_PPS_COUNT) {
        av_log(h->s.avctx, AV_LOG_ERROR, "pps_id %d out of range\n", pps_id);
        return -1;
    }
    if (!h0->pps_buffers[pps_id]) {
        av_log(h->s.avctx, AV_LOG_ERROR,
               "non-existing PPS %u referenced\n",
               pps_id);
        return -1;
    }
    h->pps = *h0->pps_buffers[pps_id];

    if (!h0->sps_buffers[h->pps.sps_id]) {
        av_log(h->s.avctx, AV_LOG_ERROR,
               "non-existing SPS %u referenced\n",
               h->pps.sps_id);
        return -1;
    }
    h->sps = *h0->sps_buffers[h->pps.sps_id];

    s->avctx->profile = ff_h264_get_profile(&h->sps);
    s->avctx->level   = h->sps.level_idc;
    s->avctx->refs    = h->sps.ref_frame_count;

    s->mb_width  = h->sps.mb_width;
    s->mb_height = h->sps.mb_height * (2 - h->sps.frame_mbs_only_flag);

    h->b_stride = s->mb_width * 4;

    s->chroma_y_shift = h->sps.chroma_format_idc <= 1; // 400 uses yuv420p

    s->width  = 16 * s->mb_width;
    s->height = 16 * s->mb_height;


    if (s->context_initialized &&
            (   s->width != s->avctx->coded_width || s->height != s->avctx->coded_height
                || av_cmp_q(h->sps.sar, s->avctx->sample_aspect_ratio))) {
        if (h != h0) {
            av_log_missing_feature(s->avctx, "Width/height changing with threads is", 0);
            return -1;   // width / height changed during parallelized decoding
        }
        free_tables(h, 0);
        flush_dpb(s->avctx);
        ff_MPV_common_end(s);
        h->list_count = 0;
    }
    if (!s->context_initialized) {
        if (h != h0) {
            av_log(h->s.avctx, AV_LOG_ERROR,
                   "Cannot (re-)initialize context during parallel decoding.\n");
            return -1;
        }
        avcodec_set_dimensions(s->avctx, s->width, s->height);
        s->avctx->width  -= (2 >> CHROMA444) * FFMIN(h->sps.crop_right, (8 << CHROMA444) - 1);
        s->avctx->height -= (1 << s->chroma_y_shift) * FFMIN(h->sps.crop_bottom, (16 >> s->chroma_y_shift) - 1) * (2 - h->sps.frame_mbs_only_flag);


        if (h->sps.video_signal_type_present_flag) {
            s->avctx->color_range = h->sps.full_range > 0 ? AVCOL_RANGE_JPEG
                                    : AVCOL_RANGE_MPEG;
            if (h->sps.colour_description_present_flag) {
                s->avctx->color_primaries = h->sps.color_primaries;
                s->avctx->color_trc       = h->sps.color_trc;
                s->avctx->colorspace      = h->sps.colorspace;
            }
        }

        if (h->sps.timing_info_present_flag) {
            int64_t den = h->sps.time_scale;
            if (h->x264_build < 44U)
                den *= 2;
            av_reduce(&s->avctx->time_base.num, &s->avctx->time_base.den,
                      h->sps.num_units_in_tick, den, 1 << 30);
        }

        s->avctx->pix_fmt = PIX_FMT_YUV420P;

        if (ff_MPV_common_init(s) < 0) {
            av_log(h->s.avctx, AV_LOG_ERROR, "ff_MPV_common_init() failed.\n");
            return -1;
        }
        s->first_field = 0;
        h->prev_interlaced_frame = 1;

        init_scan_tables(h);
        if (ff_h264_alloc_tables(h) < 0) {
            av_log(h->s.avctx, AV_LOG_ERROR,
                   "Could not allocate memory for h264\n");
            return AVERROR(ENOMEM);
        }

        if (!HAVE_THREADS || !(s->avctx->active_thread_type & FF_THREAD_SLICE)) {
            if (context_init(h) < 0) {
                av_log(h->s.avctx, AV_LOG_ERROR, "context_init() failed.\n");
                return -1;
            }
        } else {
            for (i = 1; i < s->slice_context_count; i++) {
                H264Context *c;
                c = h->thread_context[i] = av_malloc(sizeof(H264Context));
                memcpy(c, h->s.thread_context[i], sizeof(MpegEncContext));
                memset(&c->s + 1, 0, sizeof(H264Context) - sizeof(MpegEncContext));
                c->h264dsp     = h->h264dsp;
                c->sps         = h->sps;
                c->pps         = h->pps;
                c->pixel_shift = h->pixel_shift;
                c->cur_chroma_format_idc = h->cur_chroma_format_idc;
                init_scan_tables(c);
                clone_tables(c, h, i);
            }

            for (i = 0; i < s->slice_context_count; i++)
                if (context_init(h->thread_context[i]) < 0) {
                    av_log(h->s.avctx, AV_LOG_ERROR,
                           "context_init() failed.\n");
                    return -1;
                }
        }
    }

    s->avctx->sample_aspect_ratio = h->sps.sar;
    av_assert0(s->avctx->sample_aspect_ratio.den);

    if (h == h0 && h->dequant_coeff_pps != pps_id) {
        h->dequant_coeff_pps = pps_id;
        init_dequant_tables(h);
    }

    h->frame_num = get_bits(&s->gb, h->sps.log2_max_frame_num);

    h->mb_mbaff        = 0;
    h->mb_aff_frame    = 0;
    last_pic_structure = s0->picture_structure;
    last_pic_dropable  = s->dropable;
    s->dropable        = h->nal_ref_idc == 0;
    if (h->sps.frame_mbs_only_flag) {
        s->picture_structure = PICT_FRAME;
    } else {
        if (!h->sps.direct_8x8_inference_flag && slice_type == AV_PICTURE_TYPE_B) {
            av_log(h->s.avctx, AV_LOG_ERROR, "This stream was generated by a broken encoder, invalid 8x8 inference\n");
            return -1;
        }
        if (get_bits1(&s->gb)) { // field_pic_flag
            s->picture_structure = PICT_TOP_FIELD + get_bits1(&s->gb); // bottom_field_flag
        } else {
            s->picture_structure = PICT_FRAME;
            h->mb_aff_frame      = h->sps.mb_aff;
        }
    }
    h->mb_field_decoding_flag = s->picture_structure != PICT_FRAME;

    if (h0->current_slice != 0) {
        if (last_pic_structure != s->picture_structure ||
            last_pic_dropable  != s->dropable) {
            av_log(h->s.avctx, AV_LOG_ERROR,
                   "Changing field mode (%d -> %d) between slices is not allowed\n",
                   last_pic_structure, s->picture_structure);
            s->picture_structure = last_pic_structure;
            s->dropable          = last_pic_dropable;
            return AVERROR_INVALIDDATA;
        }
    } else {
        /* Shorten frame num gaps so we don't have to allocate reference
         * frames just to throw them away */
        if (h->frame_num != h->prev_frame_num && h->prev_frame_num >= 0) {
            int unwrap_prev_frame_num = h->prev_frame_num;
            int max_frame_num         = 1 << h->sps.log2_max_frame_num;

            if (unwrap_prev_frame_num > h->frame_num)
                unwrap_prev_frame_num -= max_frame_num;

            if ((h->frame_num - unwrap_prev_frame_num) > h->sps.ref_frame_count) {
                unwrap_prev_frame_num = (h->frame_num - h->sps.ref_frame_count) - 1;
                if (unwrap_prev_frame_num < 0)
                    unwrap_prev_frame_num += max_frame_num;

                h->prev_frame_num = unwrap_prev_frame_num;
            }
        }

        /* See if we have a decoded first field looking for a pair...
         * Here, we're using that to see if we should mark previously
         * decode frames as "finished".
         * We have to do that before the "dummy" in-between frame allocation,
         * since that can modify s->current_picture_ptr. */
        if (s0->first_field) {
            assert(s0->current_picture_ptr);
            assert(s0->current_picture_ptr->f.data[0]);
            assert(s0->current_picture_ptr->f.reference != DELAYED_PIC_REF);

            /* Mark old field/frame as completed */
            if (!last_pic_dropable && s0->current_picture_ptr->owner2 == s0) {
                ff_thread_report_progress(&s0->current_picture_ptr->f, INT_MAX,
                                          last_pic_structure == PICT_BOTTOM_FIELD);
            }

            /* figure out if we have a complementary field pair */
            if (!FIELD_PICTURE || s->picture_structure == last_pic_structure) {
                /* Previous field is unmatched. Don't display it, but let it
                 * remain for reference if marked as such. */
                if (!last_pic_dropable && last_pic_structure != PICT_FRAME) {
                    ff_thread_report_progress(&s0->current_picture_ptr->f, INT_MAX,
                                              last_pic_structure == PICT_TOP_FIELD);
                }
            } else {
                if (s0->current_picture_ptr->frame_num != h->frame_num) {
                    /* This and previous field were reference, but had
                     * different frame_nums. Consider this field first in
                     * pair. Throw away previous field except for reference
                     * purposes. */
                    if (!last_pic_dropable && last_pic_structure != PICT_FRAME) {
                        ff_thread_report_progress(&s0->current_picture_ptr->f, INT_MAX,
                                                  last_pic_structure == PICT_TOP_FIELD);
                    }
                } else {
                    /* Second field in complementary pair */
                    if (!((last_pic_structure   == PICT_TOP_FIELD &&
                           s->picture_structure == PICT_BOTTOM_FIELD) ||
                          (last_pic_structure   == PICT_BOTTOM_FIELD &&
                           s->picture_structure == PICT_TOP_FIELD))) {
                        av_log(s->avctx, AV_LOG_ERROR,
                               "Invalid field mode combination %d/%d\n",
                               last_pic_structure, s->picture_structure);
                        s->picture_structure = last_pic_structure;
                        s->dropable          = last_pic_dropable;
                        return AVERROR_INVALIDDATA;
                    } else if (last_pic_dropable != s->dropable) {
                        av_log(s->avctx, AV_LOG_ERROR,
                               "Cannot combine reference and non-reference fields in the same frame\n");
                        av_log_ask_for_sample(s->avctx, NULL);
                        s->picture_structure = last_pic_structure;
                        s->dropable          = last_pic_dropable;
                        return AVERROR_INVALIDDATA;
                    }

                    /* Take ownership of this buffer. Note that if another thread owned
                     * the first field of this buffer, we're not operating on that pointer,
                     * so the original thread is still responsible for reporting progress
                     * on that first field (or if that was us, we just did that above).
                     * By taking ownership, we assign responsibility to ourselves to
                     * report progress on the second field. */
                    s0->current_picture_ptr->owner2 = s0;
                }
            }
        }

        while (h->frame_num != h->prev_frame_num && h->prev_frame_num >= 0 &&
               h->frame_num != (h->prev_frame_num + 1) % (1 << h->sps.log2_max_frame_num)) {
            Picture *prev = h->short_ref_count ? h->short_ref[0] : NULL;
            av_log(h->s.avctx, AV_LOG_DEBUG, "Frame num gap %d %d\n",
                   h->frame_num, h->prev_frame_num);
            if (ff_h264_frame_start(h) < 0)
                return -1;
            h->prev_frame_num++;
            h->prev_frame_num %= 1 << h->sps.log2_max_frame_num;
            s->current_picture_ptr->frame_num = h->prev_frame_num;
            ff_thread_report_progress(&s->current_picture_ptr->f, INT_MAX, 0);
            ff_thread_report_progress(&s->current_picture_ptr->f, INT_MAX, 1);
            ff_generate_sliding_window_mmcos(h);
            if (ff_h264_execute_ref_pic_marking(h, h->mmco, h->mmco_index) < 0 &&
                    (s->avctx->err_recognition & AV_EF_EXPLODE))
                return AVERROR_INVALIDDATA;
            /* Error concealment: if a ref is missing, copy the previous ref in its place.
             * FIXME: avoiding a memcpy would be nice, but ref handling makes many assumptions
             * about there being no actual duplicates.
             * FIXME: this doesn't copy padding for out-of-frame motion vectors.  Given we're
             * concealing a lost frame, this probably isn't noticeable by comparison, but it should
             * be fixed. */
            if (h->short_ref_count) {
                if (prev) {
                    av_image_copy(h->short_ref[0]->f.data, h->short_ref[0]->f.linesize,
                                  (const uint8_t **)prev->f.data, prev->f.linesize,
                                  s->avctx->pix_fmt, s->mb_width * 16, s->mb_height * 16);
                    h->short_ref[0]->poc = prev->poc + 2;
                }
                h->short_ref[0]->frame_num = h->prev_frame_num;
            }
        }

        /* See if we have a decoded first field looking for a pair...
         * We're using that to see whether to continue decoding in that
         * frame, or to allocate a new one. */
        if (s0->first_field) {
            assert(s0->current_picture_ptr);
            assert(s0->current_picture_ptr->f.data[0]);
            assert(s0->current_picture_ptr->f.reference != DELAYED_PIC_REF);

            /* figure out if we have a complementary field pair */
            if (!FIELD_PICTURE || s->picture_structure == last_pic_structure) {
                /* Previous field is unmatched. Don't display it, but let it
                 * remain for reference if marked as such. */
                s0->current_picture_ptr = NULL;
                s0->first_field         = FIELD_PICTURE;
            } else {
                if (s0->current_picture_ptr->frame_num != h->frame_num) {
                    ff_thread_report_progress((AVFrame*)s0->current_picture_ptr, INT_MAX,
                                              s0->picture_structure == PICT_BOTTOM_FIELD);
                    /* This and the previous field had different frame_nums.
                     * Consider this field first in pair. Throw away previous
                     * one except for reference purposes. */
                    s0->first_field         = 1;
                    s0->current_picture_ptr = NULL;
                } else {
                    /* Second field in complementary pair */
                    s0->first_field = 0;
                }
            }
        } else {
            /* Frame or first field in a potentially complementary pair */
            // ==> Start patch MPC
            // assert(!s0->current_picture_ptr);
            // ==> End patch MPC
            s0->first_field = FIELD_PICTURE;
        }

        if (!FIELD_PICTURE || s0->first_field) {
            if (ff_h264_frame_start(h) < 0) {
                s0->first_field = 0;
                return -1;
            }
        } else {
            ff_release_unused_pictures(s, 0);
        }
    }
    if (h != h0)
        clone_slice(h, h0);

    s->current_picture_ptr->frame_num = h->frame_num; // FIXME frame_num cleanup

    assert(s->mb_num == s->mb_width * s->mb_height);
    if (h->first_mb_in_slice << FIELD_OR_MBAFF_PICTURE >= s->mb_num ||
        h->first_mb_in_slice                    >= s->mb_num){
        av_log(h->s.avctx, AV_LOG_ERROR, "first_mb_in_slice overflow\n");
        return -1;
    }
    s->resync_mb_x = s->mb_x =  h->first_mb_in_slice % s->mb_width;
    s->resync_mb_y = s->mb_y = (h->first_mb_in_slice / s->mb_width) << FIELD_OR_MBAFF_PICTURE;
    if (s->picture_structure == PICT_BOTTOM_FIELD)
        s->resync_mb_y = s->mb_y = s->mb_y + 1;
    assert(s->mb_y < s->mb_height);

    if (s->picture_structure == PICT_FRAME) {
        h->curr_pic_num = h->frame_num;
        h->max_pic_num  = 1 << h->sps.log2_max_frame_num;
    } else {
        h->curr_pic_num = 2 * h->frame_num + 1;
        h->max_pic_num  = 1 << (h->sps.log2_max_frame_num + 1);
    }

    if (h->nal_unit_type == NAL_IDR_SLICE)
        get_ue_golomb(&s->gb); /* idr_pic_id */

    if (h->sps.poc_type == 0) {
        h->poc_lsb = get_bits(&s->gb, h->sps.log2_max_poc_lsb);

        if (h->pps.pic_order_present == 1 && s->picture_structure == PICT_FRAME)
            h->delta_poc_bottom = get_se_golomb(&s->gb);
    }

    if (h->sps.poc_type == 1 && !h->sps.delta_pic_order_always_zero_flag) {
        h->delta_poc[0] = get_se_golomb(&s->gb);

        if (h->pps.pic_order_present == 1 && s->picture_structure == PICT_FRAME)
            h->delta_poc[1] = get_se_golomb(&s->gb);
    }

    init_poc(h);

    if (h->pps.redundant_pic_cnt_present)
        h->redundant_pic_count = get_ue_golomb(&s->gb);

    // set defaults, might be overridden a few lines later
    h->ref_count[0] = h->pps.ref_count[0];
    h->ref_count[1] = h->pps.ref_count[1];

    if (h->slice_type_nos != AV_PICTURE_TYPE_I) {
        unsigned max = s->picture_structure == PICT_FRAME ? 15 : 31;

        if (h->slice_type_nos == AV_PICTURE_TYPE_B)
            h->direct_spatial_mv_pred = get_bits1(&s->gb);
        num_ref_idx_active_override_flag = get_bits1(&s->gb);

        if (num_ref_idx_active_override_flag) {
            h->ref_count[0] = get_ue_golomb(&s->gb) + 1;
            if (h->slice_type_nos == AV_PICTURE_TYPE_B)
                h->ref_count[1] = get_ue_golomb(&s->gb) + 1;
        }

        if (h->ref_count[0] - 1 > max || h->ref_count[1] - 1 > max) {
            av_log(h->s.avctx, AV_LOG_ERROR, "reference overflow\n");
            h->ref_count[0] = h->ref_count[1] = 1;
            return AVERROR_INVALIDDATA;
        }

        if (h->slice_type_nos == AV_PICTURE_TYPE_B)
            h->list_count = 2;
        else
            h->list_count = 1;
    } else
        h->ref_count[1] = h->ref_count[0] = h->list_count = 0;

    if (!default_ref_list_done)
        ff_h264_fill_default_ref_list(h);

    if (h->slice_type_nos != AV_PICTURE_TYPE_I &&
            ff_h264_decode_ref_pic_list_reordering(h) < 0) {
        h->ref_count[1] = h->ref_count[0] = 0;
        return -1;
    }

    if (h->slice_type_nos != AV_PICTURE_TYPE_I) {
        s->last_picture_ptr = &h->ref_list[0][0];
        ff_copy_picture(&s->last_picture, s->last_picture_ptr);
    }
    if (h->slice_type_nos == AV_PICTURE_TYPE_B) {
        s->next_picture_ptr = &h->ref_list[1][0];
        ff_copy_picture(&s->next_picture, s->next_picture_ptr);
    }

    if ((h->pps.weighted_pred && h->slice_type_nos == AV_PICTURE_TYPE_P) ||
        (h->pps.weighted_bipred_idc == 1 &&
         h->slice_type_nos == AV_PICTURE_TYPE_B))
        pred_weight_table(h);
    else if (h->pps.weighted_bipred_idc == 2 &&
             h->slice_type_nos == AV_PICTURE_TYPE_B) {
        implicit_weight_table(h, -1);
    } else {
        h->use_weight = 0;
        for (i = 0; i < 2; i++) {
            h->luma_weight_flag[i]   = 0;
            h->chroma_weight_flag[i] = 0;
        }
    }

    if (h->nal_ref_idc && ff_h264_decode_ref_pic_marking(h0, &s->gb) < 0 &&
        (s->avctx->err_recognition & AV_EF_EXPLODE))
        return AVERROR_INVALIDDATA;

    if (FRAME_MBAFF) {
        ff_h264_fill_mbaff_ref_list(h);

        if (h->pps.weighted_bipred_idc == 2 && h->slice_type_nos == AV_PICTURE_TYPE_B) {
            implicit_weight_table(h, 0);
            implicit_weight_table(h, 1);
        }
    }

    if (h->slice_type_nos == AV_PICTURE_TYPE_B && !h->direct_spatial_mv_pred)
        ff_h264_direct_dist_scale_factor(h);
    ff_h264_direct_ref_list_init(h);

    if (h->slice_type_nos != AV_PICTURE_TYPE_I && h->pps.cabac) {
        tmp = get_ue_golomb_31(&s->gb);
        if (tmp > 2) {
            av_log(s->avctx, AV_LOG_ERROR, "cabac_init_idc overflow\n");
            return -1;
        }
        h->cabac_init_idc = tmp;
    }

    h->last_qscale_diff = 0;
    // ==> Start patch MPC
    h->slice_qp_delta = get_se_golomb(&s->gb);
    tmp = h->pps.init_qp + h->slice_qp_delta;
    // <== End patch MPC
    if (tmp > 51 + 6 * (h->sps.bit_depth_luma - 8)) {
        av_log(s->avctx, AV_LOG_ERROR, "QP %u out of range\n", tmp);
        return -1;
    }
    s->qscale       = tmp;
    h->chroma_qp[0] = get_chroma_qp(h, 0, s->qscale);
    h->chroma_qp[1] = get_chroma_qp(h, 1, s->qscale);
    // FIXME qscale / qp ... stuff
    if (h->slice_type == AV_PICTURE_TYPE_SP)
        get_bits1(&s->gb); /* sp_for_switch_flag */
    if (h->slice_type == AV_PICTURE_TYPE_SP ||
        h->slice_type == AV_PICTURE_TYPE_SI)
        // ==> Start patch MPC
        h->slice_qs_delta = get_se_golomb(&s->gb); /* slice_qs_delta */
        // <== End patch MPC

    h->deblocking_filter     = 1;
    h->slice_alpha_c0_offset = 52;
    h->slice_beta_offset     = 52;
    if (h->pps.deblocking_filter_parameters_present) {
        tmp = get_ue_golomb_31(&s->gb);
        if (tmp > 2) {
            av_log(s->avctx, AV_LOG_ERROR,
                   "deblocking_filter_idc %u out of range\n", tmp);
            return -1;
        }
        h->deblocking_filter = tmp;
        if (h->deblocking_filter < 2)
            h->deblocking_filter ^= 1;  // 1<->0

        if (h->deblocking_filter) {
            h->slice_alpha_c0_offset += get_se_golomb(&s->gb) << 1;
            h->slice_beta_offset     += get_se_golomb(&s->gb) << 1;
            if (h->slice_alpha_c0_offset > 104U ||
                h->slice_beta_offset     > 104U) {
                av_log(s->avctx, AV_LOG_ERROR,
                       "deblocking filter parameters %d %d out of range\n",
                       h->slice_alpha_c0_offset, h->slice_beta_offset);
                return -1;
            }
        }
    }

    if (s->avctx->skip_loop_filter >= AVDISCARD_ALL ||
        (s->avctx->skip_loop_filter >= AVDISCARD_NONKEY &&
         h->slice_type_nos != AV_PICTURE_TYPE_I) ||
        (s->avctx->skip_loop_filter >= AVDISCARD_BIDIR  &&
         h->slice_type_nos == AV_PICTURE_TYPE_B) ||
        (s->avctx->skip_loop_filter >= AVDISCARD_NONREF &&
         h->nal_ref_idc == 0))
        h->deblocking_filter = 0;

    if (h->deblocking_filter == 1 && h0->max_contexts > 1) {
        if (s->avctx->flags2 & CODEC_FLAG2_FAST) {
            /* Cheat slightly for speed:
             * Do not bother to deblock across slices. */
            h->deblocking_filter = 2;
        } else {
            h0->max_contexts = 1;
            if (!h0->single_decode_warning) {
                av_log(s->avctx, AV_LOG_INFO,
                       "Cannot parallelize deblocking type 1, decoding such frames in sequential order\n");
                h0->single_decode_warning = 1;
            }
            if (h != h0) {
                av_log(h->s.avctx, AV_LOG_ERROR,
                       "Deblocking switched inside frame.\n");
                return 1;
            }
        }
    }
    h->qp_thresh = 15 + 52 -
                   FFMIN(h->slice_alpha_c0_offset, h->slice_beta_offset) -
                   FFMAX3(0,
                          h->pps.chroma_qp_index_offset[0],
                          h->pps.chroma_qp_index_offset[1]) +
                   6 * (h->sps.bit_depth_luma - 8);

    // ==> Start patch MPC
    // If entropy_coding_mode, align to 8 bits
    if (h->pps.cabac) align_get_bits(&s->gb);
    h->bit_offset_to_slice_data = s->gb.index;
    // <== End patch MPC

    h0->last_slice_type = slice_type;
    h->slice_num = ++h0->current_slice;

    if (h->slice_num)
        h0->slice_row[(h->slice_num - 1) & (MAX_SLICES - 1)]      = s->resync_mb_y;
    if (   h0->slice_row[h->slice_num    & (MAX_SLICES - 1)] + 3 >= s->resync_mb_y
        && h0->slice_row[h->slice_num    & (MAX_SLICES - 1)]     <= s->resync_mb_y
        && h->slice_num >= MAX_SLICES) {
        //in case of ASO this check needs to be updated depending on how we decide to assign slice numbers in this case
        av_log(s->avctx, AV_LOG_WARNING, "Possibly too many slices (%d >= %d), increase MAX_SLICES and recompile if there are artifacts\n", h->slice_num, MAX_SLICES);
    }

    for (j = 0; j < 2; j++) {
        int id_list[16];
        int *ref2frm = h->ref2frm[h->slice_num & (MAX_SLICES - 1)][j];
        for (i = 0; i < 16; i++) {
            id_list[i] = 60;
            if (h->ref_list[j][i].f.data[0]) {
                int k;
                uint8_t *base = h->ref_list[j][i].f.base[0];
                for (k = 0; k < h->short_ref_count; k++)
                    if (h->short_ref[k]->f.base[0] == base) {
                        id_list[i] = k;
                        break;
                    }
                for (k = 0; k < h->long_ref_count; k++)
                    if (h->long_ref[k] && h->long_ref[k]->f.base[0] == base) {
                        id_list[i] = h->short_ref_count + k;
                        break;
                    }
            }
        }

        ref2frm[0]     =
            ref2frm[1] = -1;
        for (i = 0; i < 16; i++)
            ref2frm[i + 2] = 4 * id_list[i] +
                             (h->ref_list[j][i].f.reference & 3);
        ref2frm[18 + 0]     =
            ref2frm[18 + 1] = -1;
        for (i = 16; i < 48; i++)
            ref2frm[i + 4] = 4 * id_list[(i - 16) >> 1] +
                             (h->ref_list[j][i].f.reference & 3);
    }

    // FIXME: fix draw_edges + PAFF + frame threads
    h->emu_edge_width  = (s->flags & CODEC_FLAG_EMU_EDGE ||
                          (!h->sps.frame_mbs_only_flag &&
                           s->avctx->active_thread_type))
                         ? 0 : 16;
    h->emu_edge_height = (FRAME_MBAFF || FIELD_PICTURE) ? 0 : h->emu_edge_width;

    // ==> Start patch MPC
    fill_dxva_slice_long(h);
    // ==> End patch MPC

    if (s->avctx->debug & FF_DEBUG_PICT_INFO) {
        av_log(h->s.avctx, AV_LOG_DEBUG,
               "slice:%d %s mb:%d %c%s%s pps:%u frame:%d poc:%d/%d ref:%d/%d qp:%d loop:%d:%d:%d weight:%d%s %s\n",
               h->slice_num,
               (s->picture_structure == PICT_FRAME ? "F" : s->picture_structure == PICT_TOP_FIELD ? "T" : "B"),
               h->first_mb_in_slice,
               av_get_picture_type_char(h->slice_type),
               h->slice_type_fixed ? " fix" : "",
               h->nal_unit_type == NAL_IDR_SLICE ? " IDR" : "",
               pps_id, h->frame_num,
               s->current_picture_ptr->field_poc[0],
               s->current_picture_ptr->field_poc[1],
               h->ref_count[0], h->ref_count[1],
               s->qscale,
               h->deblocking_filter,
               h->slice_alpha_c0_offset / 2 - 26, h->slice_beta_offset / 2 - 26,
               h->use_weight,
               h->use_weight == 1 && h->use_weight_chroma ? "c" : "",
               h->slice_type == AV_PICTURE_TYPE_B ? (h->direct_spatial_mv_pred ? "SPAT" : "TEMP") : "");
    }

    return 0;
}


static int decode_nal_units_noexecute(H264Context *h, const uint8_t *buf, int buf_size)
{
    MpegEncContext *const s     = &h->s;
    AVCodecContext *const avctx = s->avctx;
    int buf_index = 0;
    H264Context *hx; ///< thread context
    int context_count = 0;
    int next_avc = h->is_avc ? 0 : buf_size;

    h->max_contexts = 1;
    if (!(s->flags2 & CODEC_FLAG2_CHUNKS)) {
        h->current_slice = 0;
        if (!s->first_field)
            s->current_picture_ptr = NULL;
        ff_h264_reset_sei(h);
    }

    for (;;) {
        int consumed;
        int dst_length;
        int bit_length;
        const uint8_t *ptr;
        int i, nalsize = 0;
        int err;

        if (buf_index >= next_avc) {
            if (buf_index >= buf_size - h->nal_length_size) break;
            nalsize = 0;
            for (i = 0; i < h->nal_length_size; i++)
                nalsize = (nalsize << 8) | buf[buf_index++];
            if (nalsize <= 0 || nalsize > buf_size - buf_index) {
                av_log(h->s.avctx, AV_LOG_ERROR, "AVC: nal size %d\n", nalsize);
                break;
            }
            next_avc = buf_index + nalsize;
        } else {
            // start code prefix search
            for (; buf_index + 3 < next_avc; buf_index++) {
                // This should always succeed in the first iteration.
                if (buf[buf_index] == 0 && buf[buf_index + 1] == 0 && buf[buf_index + 2] == 1)
                    break;
            }

            if (buf_index + 3 >= buf_size) break;

            buf_index += 3;
            if (buf_index >= next_avc) continue;
        }

        hx = h->thread_context[context_count];

        ptr = ff_h264_decode_nal(hx, buf + buf_index, &dst_length, &consumed, next_avc - buf_index);
        if (ptr == NULL || dst_length < 0) {
            return -1;
        }
        i = buf_index + consumed;
        if ((s->workaround_bugs & FF_BUG_AUTODETECT) && i + 3 < next_avc &&
                buf[i] == 0x00 && buf[i + 1] == 0x00 && buf[i + 2] == 0x01 && buf[i + 3] == 0xE0)
            s->workaround_bugs |= FF_BUG_TRUNCATED;

        if (!(s->workaround_bugs & FF_BUG_TRUNCATED)) {
            while (dst_length > 0 && ptr[dst_length - 1] == 0)
                dst_length--;
        }
        bit_length = !dst_length ? 0 : (8 * dst_length - decode_rbsp_trailing(h, ptr + dst_length - 1));

        if (s->avctx->debug & FF_DEBUG_STARTCODE) {
            av_log(h->s.avctx, AV_LOG_DEBUG, "NAL %d/%d at %d/%d length %d\n", hx->nal_unit_type, hx->nal_ref_idc, buf_index, buf_size, dst_length);
        }

        if (h->is_avc && (nalsize != consumed) && nalsize) {
            av_log(h->s.avctx, AV_LOG_DEBUG, "AVC: Consumed only %d bytes instead of %d\n", consumed, nalsize);
        }

        buf_index += consumed;

        //FIXME do not discard SEI id
        if (avctx->skip_frame >= AVDISCARD_NONREF && h->nal_ref_idc  == 0)
            continue;

again:
        err = 0;
        switch (hx->nal_unit_type) {
        case NAL_IDR_SLICE:
            if (h->nal_unit_type != NAL_IDR_SLICE) {
                av_log(h->s.avctx, AV_LOG_ERROR, "Invalid mix of idr and non-idr slices");
                return -1;
            }
            idr(h); //FIXME ensure we don't loose some frames if there is reordering
        case NAL_SLICE:
            init_get_bits(&hx->s.gb, ptr, bit_length);
            hx->intra_gb_ptr =
            hx->inter_gb_ptr = &hx->s.gb;
            hx->s.data_partitioning = 0;

            // ==> Start patch MPC DXVA
            hx->ref_pic_flag = (h->nal_ref_idc != 0);
            if ((err = decode_slice_header_noexecute(hx, h)))
                break;
            // <== End patch MPC DXVA

            s->current_picture_ptr->f.key_frame |=
                (hx->nal_unit_type == NAL_IDR_SLICE) ||
                (h->sei_recovery_frame_cnt >= 0);
            if (hx->redundant_pic_count == 0
                    && (avctx->skip_frame < AVDISCARD_NONREF || hx->nal_ref_idc)
                    && (avctx->skip_frame < AVDISCARD_BIDIR  || hx->slice_type_nos != AV_PICTURE_TYPE_B)
                    && (avctx->skip_frame < AVDISCARD_NONKEY || hx->slice_type_nos == AV_PICTURE_TYPE_I)
                    && avctx->skip_frame < AVDISCARD_ALL) {
                context_count++;
            }
            break;
        case NAL_DPA:
            init_get_bits(&hx->s.gb, ptr, bit_length);
            hx->intra_gb_ptr =
            hx->inter_gb_ptr = NULL;
            // ==> Start patch MPC DXVA
            if ((err = decode_slice_header_noexecute(hx, h)) < 0)
                break;
            // <== End patch MPC DXVA

            hx->s.data_partitioning = 1;

            break;
        case NAL_DPB:
            init_get_bits(&hx->intra_gb, ptr, bit_length);
            hx->intra_gb_ptr = &hx->intra_gb;
            break;
        case NAL_DPC:
            init_get_bits(&hx->inter_gb, ptr, bit_length);
            hx->inter_gb_ptr = &hx->inter_gb;

            if (hx->redundant_pic_count == 0 && hx->intra_gb_ptr && hx->s.data_partitioning
                    && s->context_initialized
                    && (avctx->skip_frame < AVDISCARD_NONREF || hx->nal_ref_idc)
                    && (avctx->skip_frame < AVDISCARD_BIDIR  || hx->slice_type_nos != AV_PICTURE_TYPE_B)
                    && (avctx->skip_frame < AVDISCARD_NONKEY || hx->slice_type_nos == AV_PICTURE_TYPE_I)
                    && avctx->skip_frame < AVDISCARD_ALL)
                context_count++;
            break;
        case NAL_SEI:
            init_get_bits(&s->gb, ptr, bit_length);
            ff_h264_decode_sei(h);
            break;
        case NAL_SPS:
            init_get_bits(&s->gb, ptr, bit_length);
            if (ff_h264_decode_seq_parameter_set(h) < 0 &&
                    h->is_avc && (nalsize != consumed) && nalsize) {
                av_log(h->s.avctx, AV_LOG_DEBUG, "SPS decoding failure, "
                       "try parsing the coomplete NAL\n");
                init_get_bits(&s->gb, buf + buf_index + 1 - consumed,
                              8 * (nalsize - 1));
                ff_h264_decode_seq_parameter_set(h);
            }

            if (s->flags & CODEC_FLAG_LOW_DELAY ||
                    (h->sps.bitstream_restriction_flag && !h->sps.num_reorder_frames))
                s->low_delay = 1;

            if (avctx->has_b_frames < 2)
                avctx->has_b_frames = !s->low_delay;
            break;
        case NAL_PPS:
            init_get_bits(&s->gb, ptr, bit_length);

            ff_h264_decode_picture_parameter_set(h, bit_length);

            break;
        case NAL_AUD:
        case NAL_END_SEQUENCE:
        case NAL_END_STREAM:
        case NAL_FILLER_DATA:
        case NAL_SPS_EXT:
        case NAL_AUXILIARY_SLICE:
            break;
        default:
            av_log(avctx, AV_LOG_DEBUG, "Unknown NAL code: %d (%d bits)\n", hx->nal_unit_type, bit_length);
        }

        if (context_count == h->max_contexts) {
            // ==> Start patch MPC DXVA
            // execute_decode_slices(h, context_count);
            // <== End patch MPC DXVA
            context_count = 0;
        }

        if (err < 0)
            av_log(h->s.avctx, AV_LOG_ERROR, "decode_slice_header error\n");
        else if (err == 1) {
            /* Slice could not be decoded in parallel mode, copy down
             * NAL unit stuff to context 0 and restart. Note that
             * rbsp_buffer is not transferred, but since we no longer
             * run in parallel mode this should not be an issue. */
            h->nal_unit_type = hx->nal_unit_type;
            h->nal_ref_idc   = hx->nal_ref_idc;
            hx = h;
            goto again;
        }
    }
    // ==> Start patch MPC DXVA
    //if(context_count)
    //    execute_decode_slices(h, context_count);
    // <== End patch MPC DXVA
    return buf_index;
}


int av_h264_decode_frame(struct AVCodecContext* avctx, int* nOutPOC, int64_t* rtStartTime, const uint8_t *buf, int buf_size)
{
    H264Context *h     = avctx->priv_data;
    MpegEncContext *s  = &h->s;
    int buf_index      = 0;
    Picture *out;
    int i, out_idx;

    // ==> Start patch MPC DXVA
    if (nOutPOC) *nOutPOC = INT_MIN;
    // <== End patch MPC DXVA

    s->flags  = avctx->flags;
    s->flags2 = avctx->flags2;

    /* end of stream, output what is still in the buffers */
    if (buf_size == 0) {
out:

        s->current_picture_ptr = NULL;

        // FIXME factorize this with the output code below
        out     = h->delayed_pic[0];
        out_idx = 0;
        for (i = 1;
                h->delayed_pic[i] &&
                !h->delayed_pic[i]->f.key_frame &&
                !h->delayed_pic[i]->mmco_reset;
                i++)
            if (h->delayed_pic[i]->poc < out->poc) {
                out     = h->delayed_pic[i];
                out_idx = i;
            }

        for (i = out_idx; h->delayed_pic[i]; i++)
            h->delayed_pic[i] = h->delayed_pic[i + 1];

        if (out) {
            // ==> Start patch MPC DXVA
            //*data_size = sizeof(AVFrame);
            //*pict      = out->f;
            if (nOutPOC) *nOutPOC = out->poc;
            if (rtStartTime) *rtStartTime = out->f.reordered_opaque;
            // <== End patch MPC DXVA
        }

        return buf_index;
    }

    // ==> Start patch MPC DXVA
    buf_index = decode_nal_units_noexecute(h, buf, buf_size);
    // <== End patch MPC DXVA
    if (buf_index < 0)
        return -1;

    if (!s->current_picture_ptr && h->nal_unit_type == NAL_END_SEQUENCE) {
        av_assert0(buf_index <= buf_size);
        goto out;
    }

    if (!(s->flags2 & CODEC_FLAG2_CHUNKS) && !s->current_picture_ptr) {
        if (avctx->skip_frame >= AVDISCARD_NONREF ||
                buf_size >= 4 && !memcmp("Q264", buf, 4))
            return buf_size;
        av_log(avctx, AV_LOG_ERROR, "no frame!\n");
        return -1;
    }

    if (!(s->flags2 & CODEC_FLAG2_CHUNKS) ||
            (s->mb_y >= s->mb_height && s->mb_height)) {
        // ==> Start patch MPC DXVA
        decode_postinit_dxva(h, nOutPOC, rtStartTime);
        field_end_noexecute(h);
        // <== End patch MPC DXVA
    }

    if ((nOutPOC) && *nOutPOC == INT_MIN && h->next_output_pic) {
        *nOutPOC = h->next_output_pic->poc;
    }

    return get_consumed_bytes(s, buf_index, buf_size);
}
