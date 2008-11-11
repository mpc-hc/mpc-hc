/*
 * This file is part of Media Player Classic HomeCinema.
 *
 * MPC-HC is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "h264.h"
#include "h264data.h"
#include "h264_parser.h"
#include "golomb.h"


typedef unsigned short        USHORT;
typedef unsigned char         UCHAR;
typedef int                   INT;
typedef unsigned int          UINT;
typedef char                  CHAR;
typedef unsigned char         BYTE;
typedef unsigned short        WORD;

#define S_OK                0x00000000L
#define E_FAIL              0x80004005L


static int decode_ref_pic_list_reordering_noframe(H264Context *h){
    MpegEncContext * const s = &h->s;
    int list, index, pic_structure;

    print_short_term(h);
    print_long_term(h);
    if(h->slice_type==FF_I_TYPE || h->slice_type==FF_SI_TYPE) return 0; //FIXME move before func

    for(list=0; list<h->list_count; list++){
        memcpy(h->ref_list[list], h->default_ref_list[list], sizeof(Picture)*h->ref_count[list]);

        if(get_bits1(&s->gb)){
            int pred= h->curr_pic_num;

            for(index=0; ; index++){
                unsigned int reordering_of_pic_nums_idc= get_ue_golomb(&s->gb);
                unsigned int pic_id;
                int i;
                Picture *ref = NULL;

                if(reordering_of_pic_nums_idc==3)
                    break;

                if(index >= h->ref_count[list]){
                    av_log(h->s.avctx, AV_LOG_ERROR, "reference count overflow\n");
                    return -1;
                }

                if(reordering_of_pic_nums_idc<3){
                    if(reordering_of_pic_nums_idc<2){
                        const unsigned int abs_diff_pic_num= get_ue_golomb(&s->gb) + 1;
                        int frame_num;

                        if(abs_diff_pic_num > h->max_pic_num){
                            av_log(h->s.avctx, AV_LOG_ERROR, "abs_diff_pic_num overflow\n");
                            return -1;
                        }

                        if(reordering_of_pic_nums_idc == 0) pred-= abs_diff_pic_num;
                        else                                pred+= abs_diff_pic_num;
                        pred &= h->max_pic_num - 1;

                        frame_num = pic_num_extract(h, pred, &pic_structure);

                        for(i= h->short_ref_count-1; i>=0; i--){
                            ref = h->short_ref[i];
                            assert(ref->reference);
                            assert(!ref->long_ref);
                            if(ref->data[0] != NULL &&
                                   ref->frame_num == frame_num &&
                                   (ref->reference & pic_structure) &&
                                   ref->long_ref == 0) // ignore non existing pictures by testing data[0] pointer
                                break;
                        }
                        if(i>=0)
                            ref->pic_id= pred;
                    }else{
                        int long_idx;
                        pic_id= get_ue_golomb(&s->gb); //long_term_pic_idx

                        long_idx= pic_num_extract(h, pic_id, &pic_structure);

                        if(long_idx>31){
                            av_log(h->s.avctx, AV_LOG_ERROR, "long_term_pic_idx overflow\n");
                            return -1;
                        }
                        ref = h->long_ref[long_idx];
                        assert(!(ref && !ref->reference));
                        if(ref && (ref->reference & pic_structure)){
                            ref->pic_id= pic_id;
                            assert(ref->long_ref);
                            i=0;
                        }else{
                            i=-1;
                        }
                    }

                    if (i < 0) {
                        av_log(h->s.avctx, AV_LOG_ERROR, "reference picture missing during reorder\n");
                        memset(&h->ref_list[list][index], 0, sizeof(Picture)); //FIXME
                    } else {
                        for(i=index; i+1<h->ref_count[list]; i++){
                            if(ref->long_ref == h->ref_list[list][i].long_ref && ref->pic_id == h->ref_list[list][i].pic_id)
                                break;
                        }
                        for(; i > index; i--){
                            h->ref_list[list][i]= h->ref_list[list][i-1];
                        }
                        h->ref_list[list][index]= *ref;
                        if (FIELD_PICTURE){
                            pic_as_field(&h->ref_list[list][index], pic_structure);
                        }
                    }
                }else{
                    av_log(h->s.avctx, AV_LOG_ERROR, "illegal reordering_of_pic_nums_idc\n");
                    return -1;
                }
            }
        }
    }
    for(list=0; list<h->list_count; list++){
        for(index= 0; index < h->ref_count[list]; index++){
            if(!h->ref_list[list][index].data[0])
                h->ref_list[list][index]= s->current_picture;
        }
    }

    //if(h->slice_type==FF_B_TYPE && !h->direct_spatial_mv_pred)
    //    direct_dist_scale_factor(h);
//    direct_ref_list_init(h);
    return 0;
}

static int init_poc_noframe(H264Context *h){
    MpegEncContext * const s = &h->s;
    const int max_frame_num= 1<<h->sps.log2_max_frame_num;
    int field_poc[2];

    if(h->nal_unit_type == NAL_IDR_SLICE){
        h->frame_num_offset= 0;
    }else{
        if(h->frame_num < h->prev_frame_num)
            h->frame_num_offset= h->prev_frame_num_offset + max_frame_num;
        else
            h->frame_num_offset= h->prev_frame_num_offset;
    }

    if(h->sps.poc_type==0){
        const int max_poc_lsb= 1<<h->sps.log2_max_poc_lsb;

        if(h->nal_unit_type == NAL_IDR_SLICE){
             h->prev_poc_msb=
             h->prev_poc_lsb= 0;
        }

        if     (h->poc_lsb < h->prev_poc_lsb && h->prev_poc_lsb - h->poc_lsb >= max_poc_lsb/2)
            h->poc_msb = h->prev_poc_msb + max_poc_lsb;
        else if(h->poc_lsb > h->prev_poc_lsb && h->prev_poc_lsb - h->poc_lsb < -max_poc_lsb/2)
            h->poc_msb = h->prev_poc_msb - max_poc_lsb;
        else
            h->poc_msb = h->prev_poc_msb;
//printf("poc: %d %d\n", h->poc_msb, h->poc_lsb);
        field_poc[0] =
        field_poc[1] = h->poc_msb + h->poc_lsb;
        if(s->picture_structure == PICT_FRAME)
            field_poc[1] += h->delta_poc_bottom;
    }else if(h->sps.poc_type==1){
        int abs_frame_num, expected_delta_per_poc_cycle, expectedpoc;
        int i;

        if(h->sps.poc_cycle_length != 0)
            abs_frame_num = h->frame_num_offset + h->frame_num;
        else
            abs_frame_num = 0;

        if(h->nal_ref_idc==0 && abs_frame_num > 0)
            abs_frame_num--;

        expected_delta_per_poc_cycle = 0;
        for(i=0; i < h->sps.poc_cycle_length; i++)
            expected_delta_per_poc_cycle += h->sps.offset_for_ref_frame[ i ]; //FIXME integrate during sps parse

        if(abs_frame_num > 0){
            int poc_cycle_cnt          = (abs_frame_num - 1) / h->sps.poc_cycle_length;
            int frame_num_in_poc_cycle = (abs_frame_num - 1) % h->sps.poc_cycle_length;

            expectedpoc = poc_cycle_cnt * expected_delta_per_poc_cycle;
            for(i = 0; i <= frame_num_in_poc_cycle; i++)
                expectedpoc = expectedpoc + h->sps.offset_for_ref_frame[ i ];
        } else
            expectedpoc = 0;

        if(h->nal_ref_idc == 0)
            expectedpoc = expectedpoc + h->sps.offset_for_non_ref_pic;

        field_poc[0] = expectedpoc + h->delta_poc[0];
        field_poc[1] = field_poc[0] + h->sps.offset_for_top_to_bottom_field;

        if(s->picture_structure == PICT_FRAME)
            field_poc[1] += h->delta_poc[1];
    }else{
        int poc;
        if(h->nal_unit_type == NAL_IDR_SLICE){
            poc= 0;
        }else{
            if(h->nal_ref_idc) poc= 2*(h->frame_num_offset + h->frame_num);
            else               poc= 2*(h->frame_num_offset + h->frame_num) - 1;
        }
        field_poc[0]= poc;
        field_poc[1]= poc;
    }

	if (!h->sps.pic_struct_present_flag)
	{
		if (field_poc[0] != field_poc[1])
			/* Derive top_field_first from field pocs. */
			s->picture_structure = (field_poc[0] < field_poc[1])?PICT_TOP_FIELD:PICT_BOTTOM_FIELD;
		else
			/* Most likely progressive */
			s->picture_structure = PICT_FRAME;
	}
    //if(s->picture_structure != PICT_BOTTOM_FIELD) {
    //    s->current_picture_ptr->field_poc[0]= field_poc[0];
    //    s->current_picture_ptr->poc = field_poc[0];
    //}
    //if(s->picture_structure != PICT_TOP_FIELD) {
    //    s->current_picture_ptr->field_poc[1]= field_poc[1];
    //    s->current_picture_ptr->poc = field_poc[1];
    //}
    //if(!FIELD_PICTURE || !s->first_field) {
    //    Picture *cur = s->current_picture_ptr;
    //    cur->poc= FFMIN(cur->field_poc[0], cur->field_poc[1]);
    //}

    return 0;
}

int decode_slice_header_noexecute (H264Context *h)
{
    MpegEncContext* const s = &h->s;
    H264Context* hx;
    SPS* cur_sps = h->sps_buffers[0];
    PPS* cur_pps = h->pps_buffers[0];
    unsigned int first_mb_in_slice;
    unsigned int pps_id;
    int num_ref_idx_active_override_flag;
    static const uint8_t slice_type_map[5]= {FF_P_TYPE, FF_B_TYPE, FF_I_TYPE, FF_SP_TYPE, FF_SI_TYPE};
    unsigned int slice_type, tmp, i;
    int default_ref_list_done = 0;
    int last_pic_structure;
    int field_pic_flag;

    s->dropable= h->nal_ref_idc == 0;

    //if((s->avctx->flags2 & CODEC_FLAG2_FAST) && !h->nal_ref_idc){
    //    s->me.qpel_put= s->dsp.put_2tap_qpel_pixels_tab;
    //    s->me.qpel_avg= s->dsp.avg_2tap_qpel_pixels_tab;
    //}else{
    //    s->me.qpel_put= s->dsp.put_h264_qpel_pixels_tab;
    //    s->me.qpel_avg= s->dsp.avg_h264_qpel_pixels_tab;
    //}

    //first_mb_in_slice= get_ue_golomb(&s->gb);

    if((s->flags2 & CODEC_FLAG2_CHUNKS) && first_mb_in_slice == 0){
        h->current_slice = 0;
        //if (!s->first_field)
        //    s->current_picture_ptr= NULL;
    }

    h->sp_for_switch_flag = 0;
    hx = h->thread_context[0];

    hx->intra_gb_ptr=
    hx->inter_gb_ptr= &hx->s.gb;
    hx->s.data_partitioning = 0;


    first_mb_in_slice = get_ue_golomb(&s->gb);
    slice_type        = get_ue_golomb(&s->gb);

    if(slice_type > 9)
        return E_FAIL;
    
    if(slice_type > 4)
    {
        slice_type -= 5;
        h->slice_type_fixed=1;
    }else
        h->slice_type_fixed=0;

    slice_type= slice_type_map[ slice_type ];
    if (slice_type == FF_I_TYPE
        || (h->current_slice != 0 && slice_type == h->last_slice_type) ) {
        default_ref_list_done = 1;
    }
    h->slice_type= slice_type;

    //s->pict_type= h->slice_type; // to make a few old func happy, it's wrong though
    //if (s->pict_type == FF_B_TYPE && s0->last_picture_ptr == NULL) {
    //    av_log(h->s.avctx, AV_LOG_ERROR,
    //           "B picture before any references, skipping\n");
    //    return -1;
    //}

    // ffdshow custom code
    if (s->pict_type == FF_I_TYPE){
        h->first_I_slice_detected = 1;
    } else if (s->pict_type == FF_P_TYPE && !h->first_I_slice_detected) {
        av_log(h->s.avctx, AV_LOG_ERROR,
               "P picture before first I picture, skipping\n");
        return -1;
    }

    pps_id= get_ue_golomb(&s->gb);
    if(pps_id>=MAX_PPS_COUNT){
        return E_FAIL;
    }
    if(!h->pps_buffers[pps_id]) {
        return E_FAIL;
    }
    h->pps= *h->pps_buffers[pps_id];

    if(!h->sps_buffers[h->pps.sps_id]) {
        return E_FAIL;
    }
    h->sps = *h->sps_buffers[h->pps.sps_id];

    if(h->dequant_coeff_pps != pps_id){
        h->dequant_coeff_pps = pps_id;
        init_dequant_tables(h);
    }

    s->mb_width= h->sps.mb_width;
    s->mb_height= h->sps.mb_height * (2 - h->sps.frame_mbs_only_flag);

    h->b_stride=  s->mb_width*4;
    h->b8_stride= s->mb_width*2;

    s->width = 16*s->mb_width - 2*FFMIN(h->sps.crop_right, 7);
    if(h->sps.frame_mbs_only_flag)
        s->height= 16*s->mb_height - 2*FFMIN(h->sps.crop_bottom, 7);
    else
        s->height= 16*s->mb_height - 4*FFMIN(h->sps.crop_bottom, 3);

    if (s->context_initialized
        && (   s->width != s->avctx->width || s->height != s->avctx->height)) {
        free_tables(h);
        MPV_common_end(s);
    }
    if (!s->context_initialized) {
        if (MPV_common_init(s) < 0)
            return -1;
        s->first_field = 0;

        init_scan_tables(h);
        alloc_tables(h);

        s->avctx->width = s->width;
        s->avctx->height = s->height;
        s->avctx->sample_aspect_ratio= h->sps.sar;
        if(!s->avctx->sample_aspect_ratio.den)
            s->avctx->sample_aspect_ratio.den = 1;

        if(h->sps.timing_info_present_flag){
            //s->avctx->time_base= (AVRational){h->sps.num_units_in_tick * 2, h->sps.time_scale};
            s->avctx->time_base.num= h->sps.num_units_in_tick * 2;
            s->avctx->time_base.den= h->sps.time_scale;
            if(h->x264_build > 0 && h->x264_build < 44)
                s->avctx->time_base.den *= 2;
            av_reduce(&s->avctx->time_base.num, &s->avctx->time_base.den,
                      s->avctx->time_base.num, s->avctx->time_base.den, 1<<30);
        }
        frame_start(h);
    }

    h->frame_num= get_bits(&s->gb, h->sps.log2_max_frame_num);

    h->mb_mbaff = 0;
    h->mb_aff_frame = 0;
    last_pic_structure = s->picture_structure;
    if(h->sps.frame_mbs_only_flag){
        s->picture_structure= PICT_FRAME;
    }else{
        field_pic_flag = get_bits1(&s->gb);
        if(field_pic_flag) { //field_pic_flag
            s->picture_structure= PICT_TOP_FIELD + get_bits1(&s->gb); //bottom_field_flag
        } else {
            s->picture_structure= PICT_FRAME;
            h->mb_aff_frame = h->sps.mb_aff;
        }
    }

    assert(s->mb_num == s->mb_width * s->mb_height);
    if(first_mb_in_slice << FIELD_OR_MBAFF_PICTURE >= s->mb_num ||
       first_mb_in_slice                    >= s->mb_num){
        av_log(h->s.avctx, AV_LOG_ERROR, "first_mb_in_slice overflow\n");
        return -1;
    }
    s->resync_mb_x = s->mb_x = first_mb_in_slice % s->mb_width;
    s->resync_mb_y = s->mb_y = (first_mb_in_slice / s->mb_width) << FIELD_OR_MBAFF_PICTURE;
    if (s->picture_structure == PICT_BOTTOM_FIELD)
        s->resync_mb_y = s->mb_y = s->mb_y + 1;
    assert(s->mb_y < s->mb_height);

    if(s->picture_structure==PICT_FRAME){
        h->curr_pic_num=   h->frame_num;
        h->max_pic_num= 1<< h->sps.log2_max_frame_num;
    }else{
        h->curr_pic_num= 2*h->frame_num + 1;
        h->max_pic_num= 1<<(h->sps.log2_max_frame_num + 1);
    }

	
    if(h->nal_unit_type == NAL_IDR_SLICE){
        get_ue_golomb(&s->gb); /* idr_pic_id */
    }

    if(h->sps.poc_type==0){
        h->poc_lsb= get_bits(&s->gb, h->sps.log2_max_poc_lsb);

        if(h->pps.pic_order_present==1 && s->picture_structure==PICT_FRAME){
            h->delta_poc_bottom= get_se_golomb(&s->gb);
        }
    }

    if(h->sps.poc_type==1 && !h->sps.delta_pic_order_always_zero_flag){
        h->delta_poc[0]= get_se_golomb(&s->gb);

        if(h->pps.pic_order_present==1 && s->picture_structure==PICT_FRAME)
            h->delta_poc[1]= get_se_golomb(&s->gb);
    }

    init_poc_noframe(h);

    if(h->pps.redundant_pic_cnt_present){
        h->redundant_pic_count= get_ue_golomb(&s->gb);
    }

    //set defaults, might be overriden a few line later
    h->ref_count[0]= h->pps.ref_count[0];
    h->ref_count[1]= h->pps.ref_count[1];

    if(h->slice_type == FF_P_TYPE || h->slice_type == FF_SP_TYPE || h->slice_type == FF_B_TYPE){
        if(h->slice_type == FF_B_TYPE){
            h->direct_spatial_mv_pred= get_bits1(&s->gb);
        }
        num_ref_idx_active_override_flag= get_bits1(&s->gb);

        if(num_ref_idx_active_override_flag){
            h->ref_count[0]= get_ue_golomb(&s->gb) + 1;
            if(h->slice_type==FF_B_TYPE)
                h->ref_count[1]= get_ue_golomb(&s->gb) + 1;

            if(h->ref_count[0]-1 > 32-1 || h->ref_count[1]-1 > 32-1){
                av_log(h->s.avctx, AV_LOG_ERROR, "reference overflow\n");
                h->ref_count[0]= h->ref_count[1]= 1;
                return E_FAIL;
            }
        }
        if(h->slice_type == FF_B_TYPE)
            h->list_count= 2;
        else
            h->list_count= 1;
    }else
        h->list_count= 0;

    if(decode_ref_pic_list_reordering_noframe(h) < 0)
        return E_FAIL;

    if(   (h->pps.weighted_pred          && (h->slice_type == FF_P_TYPE || h->slice_type == FF_SP_TYPE ))
       || (h->pps.weighted_bipred_idc==1 && h->slice_type==FF_B_TYPE ) )
        pred_weight_table(h);

    if(h->nal_ref_idc)
        decode_ref_pic_marking(h, &s->gb);

    if(FRAME_MBAFF)
        fill_mbaff_ref_list(h);

    if( h->slice_type != FF_I_TYPE && h->slice_type != FF_SI_TYPE && h->pps.cabac ){
        tmp = get_ue_golomb(&s->gb);
        if(tmp > 2){
//            av_log(s->avctx, AV_LOG_ERROR, "cabac_init_idc overflow\n");
            return E_FAIL;
        }
        h->cabac_init_idc= tmp;
    }

    h->last_qscale_diff = 0;
    tmp = h->pps.init_qp + get_se_golomb(&s->gb);
    if(tmp>51){
//        av_log(s->avctx, AV_LOG_ERROR, "QP %u out of range\n", tmp);
        return E_FAIL;
    }
    s->qscale= tmp;
    h->chroma_qp[0] = get_chroma_qp(h, 0, s->qscale);
    h->chroma_qp[1] = get_chroma_qp(h, 1, s->qscale);
    //FIXME qscale / qp ... stuff
    if(h->slice_type == FF_SP_TYPE){
        h->sp_for_switch_flag = get_bits1(&s->gb); // sp_for_switch_flag
    }
    if(h->slice_type==FF_SP_TYPE || h->slice_type == FF_SI_TYPE){
        get_se_golomb(&s->gb); // slice_qs_delta
    }

    return S_OK;
}

static int decode_nal_units_noexecute(H264Context *h, uint8_t *buf, int buf_size)
{
    MpegEncContext * const s = &h->s;
    AVCodecContext * const avctx= s->avctx;
    int buf_index=0;
    H264Context *hx; ///< thread context
    int context_count = 0;

    h->max_contexts = avctx->thread_count;
#if 0
    int i;
    for(i=0; i<50; i++){
        av_log(NULL, AV_LOG_ERROR,"%02X ", buf[i]);
    }
#endif
    if(!(s->flags2 & CODEC_FLAG2_CHUNKS)){
        h->current_slice = 0;
        //if (!s->first_field)
        //    s->current_picture_ptr= NULL;
    }

    for(;;){
        int consumed;
        int dst_length;
        int bit_length;
        const uint8_t *ptr;
        int i, nalsize = 0;
        int err;

        if(h->is_avc) {
            if(buf_index >= buf_size) break;
            nalsize = 0;
            for(i = 0; i < h->nal_length_size; i++)
                nalsize = (nalsize << 8) | buf[buf_index++];
            if(nalsize <= 1 || (nalsize+buf_index > buf_size)){
                if(nalsize == 1){
                    buf_index++;
                    continue;
                }else{
                    av_log(h->s.avctx, AV_LOG_ERROR, "AVC: nal size %d\n", nalsize);
                    break;
                }
            }
        } else {
            // start code prefix search
            for(; buf_index + 3 < buf_size; buf_index++){
                // This should always succeed in the first iteration.
                if(buf[buf_index] == 0 && buf[buf_index+1] == 0 && buf[buf_index+2] == 1)
                    break;
            }

            if(buf_index+3 >= buf_size) break;

            buf_index+=3;
        }

        hx = h->thread_context[context_count];

        ptr= decode_nal(hx, buf + buf_index, &dst_length, &consumed, h->is_avc ? nalsize : buf_size - buf_index);
        if (ptr==NULL || dst_length < 0){
            return -1;
        }
        while(ptr[dst_length - 1] == 0 && dst_length > 0)
            dst_length--;
        bit_length= !dst_length ? 0 : (8*dst_length - decode_rbsp_trailing(h, ptr + dst_length - 1));

        if(s->avctx->debug&FF_DEBUG_STARTCODE){
            av_log(h->s.avctx, AV_LOG_DEBUG, "NAL %d at %d/%d length %d\n", hx->nal_unit_type, buf_index, buf_size, dst_length);
        }

        if (h->is_avc && (nalsize != consumed)){
            av_log(h->s.avctx, AV_LOG_ERROR, "AVC: Consumed only %d bytes instead of %d\n", consumed, nalsize);
            consumed= nalsize;
        }

        buf_index += consumed;

        if(  (s->hurry_up == 1 && h->nal_ref_idc  == 0) //FIXME do not discard SEI id
           ||(avctx->skip_frame >= AVDISCARD_NONREF && h->nal_ref_idc  == 0))
            continue;

      again:
        err = 0;
        switch(hx->nal_unit_type){
        case NAL_IDR_SLICE:
            if (h->nal_unit_type != NAL_IDR_SLICE) {
                av_log(h->s.avctx, AV_LOG_ERROR, "Invalid mix of idr and non-idr slices");
                return -1;
            }
            idr(h); //FIXME ensure we don't loose some frames if there is reordering
        case NAL_SLICE:
            init_get_bits(&hx->s.gb, ptr, bit_length);
            hx->intra_gb_ptr=
            hx->inter_gb_ptr= &hx->s.gb;
            hx->s.data_partitioning = 0;

            if((err = decode_slice_header_noexecute(hx)))
               break;

            //s->current_picture_ptr->key_frame|= (hx->nal_unit_type == NAL_IDR_SLICE);
            //if(hx->redundant_pic_count==0 && hx->s.hurry_up < 5
            //   && (avctx->skip_frame < AVDISCARD_NONREF || hx->nal_ref_idc)
            //   && (avctx->skip_frame < AVDISCARD_BIDIR  || hx->slice_type!=FF_B_TYPE)
            //   && (avctx->skip_frame < AVDISCARD_NONKEY || hx->slice_type==FF_I_TYPE)
            //   && avctx->skip_frame < AVDISCARD_ALL)
            //    context_count++;
            break;
        case NAL_DPA:
            init_get_bits(&hx->s.gb, ptr, bit_length);
            hx->intra_gb_ptr=
            hx->inter_gb_ptr= NULL;
            hx->s.data_partitioning = 1;

            err = decode_slice_header_noexecute(hx);
            break;
        case NAL_DPB:
            //init_get_bits(&hx->intra_gb, ptr, bit_length);
            //hx->intra_gb_ptr= &hx->intra_gb;
            //break;
        case NAL_DPC:
            //init_get_bits(&hx->inter_gb, ptr, bit_length);
            //hx->inter_gb_ptr= &hx->inter_gb;

            //if(hx->redundant_pic_count==0 && hx->intra_gb_ptr && hx->s.data_partitioning
            //   && s->context_initialized
            //   && s->hurry_up < 5
            //   && (avctx->skip_frame < AVDISCARD_NONREF || hx->nal_ref_idc)
            //   && (avctx->skip_frame < AVDISCARD_BIDIR  || hx->slice_type!=FF_B_TYPE)
            //   && (avctx->skip_frame < AVDISCARD_NONKEY || hx->slice_type==FF_I_TYPE)
            //   && avctx->skip_frame < AVDISCARD_ALL)
            //    context_count++;
            break;
        case NAL_SEI:
            init_get_bits(&s->gb, ptr, bit_length);
            decode_sei(h);
            break;
        case NAL_SPS:
            init_get_bits(&s->gb, ptr, bit_length);
            decode_seq_parameter_set(h);

            if(s->flags& CODEC_FLAG_LOW_DELAY)
                s->low_delay=1;

            if(avctx->has_b_frames < 2)
                avctx->has_b_frames= !s->low_delay;
            break;
        case NAL_PPS:
            init_get_bits(&s->gb, ptr, bit_length);

            decode_picture_parameter_set(h, bit_length);

            break;
        case NAL_AUD:
        case NAL_END_SEQUENCE:
        case NAL_END_STREAM:
        case NAL_FILLER_DATA:
        case NAL_SPS_EXT:
        case NAL_AUXILIARY_SLICE:
            break;
        default:
            av_log(avctx, AV_LOG_DEBUG, "Unknown NAL code: %d (%d bits)\n", h->nal_unit_type, bit_length);
        }

        if(context_count == h->max_contexts) {
      // execute_decode_slices(h, context_count);
            context_count = 0;
        }

        if (err < 0)
            av_log(h->s.avctx, AV_LOG_ERROR, "decode_slice_header_noexecute error\n");
        else if(err == 1) {
            /* Slice could not be decoded in parallel mode, copy down
             * NAL unit stuff to context 0 and restart. Note that
             * rbsp_buffer is not transfered, but since we no longer
             * run in parallel mode this should not be an issue. */
            h->nal_unit_type = hx->nal_unit_type;
            h->nal_ref_idc   = hx->nal_ref_idc;
            hx = h;
            goto again;
        }
    }
    //if(context_count)
    //execute_decode_slices(h, context_count);
    return buf_index;
}


//static int decode_frame(AVCodecContext *avctx,
//                             void *data, int *data_size,
//                             uint8_t *buf, int buf_size)
int av_h264_decode_frame(struct AVCodecContext* avctx, uint8_t *buf, int buf_size)
{
    H264Context *h = avctx->priv_data;
    MpegEncContext *s = &h->s;
//    AVFrame *pict = data;
    int buf_index;

    s->flags= avctx->flags;
    s->flags2= avctx->flags2;

   /* no supplementary picture */
//    if (buf_size == 0) {
//        Picture *out;
//        int i, out_idx;
//
////FIXME factorize this with the output code below
//        out = h->delayed_pic[0];
//        out_idx = 0;
//        for(i=1; h->delayed_pic[i] && !h->delayed_pic[i]->key_frame; i++)
//            if(h->delayed_pic[i]->poc < out->poc){
//                out = h->delayed_pic[i];
//                out_idx = i;
//            }
//
//        for(i=out_idx; h->delayed_pic[i]; i++)
//            h->delayed_pic[i] = h->delayed_pic[i+1];
//
//        if(out){
//            *data_size = sizeof(AVFrame);
//            *pict= *(AVFrame*)out;
//        }
//
//        return 0;
//    }

    if(s->flags&CODEC_FLAG_TRUNCATED){
        int next= ff_h264_find_frame_end(h, buf, buf_size);

        if( ff_combine_frame(&s->parse_context, next, (const uint8_t **)&buf, &buf_size) < 0 )
            return buf_size;
//printf("next:%d buf_size:%d last_index:%d\n", next, buf_size, s->parse_context.last_index);
    }

    if(h->is_avc && !h->got_avcC) {
        int i, cnt, nalsize;
        unsigned char *p = avctx->extradata, *pend=p+avctx->extradata_size;
#if 0
        if(avctx->extradata_size < 7) {
            av_log(avctx, AV_LOG_ERROR, "avcC too short\n");
            return -1;
        }
        if(*p != 1) {
            av_log(avctx, AV_LOG_ERROR, "Unknown avcC version %d\n", *p);
            return -1;
        }
        /* sps and pps in the avcC always have length coded with 2 bytes,
           so put a fake nal_length_size = 2 while parsing them */
        h->nal_length_size = 2;
        // Decode sps from avcC
        cnt = *(p+5) & 0x1f; // Number of sps
        p += 6;
#else
        h->nal_length_size = 2;
        cnt = 1;
#endif
        for (i = 0; i < cnt; i++) {
            nalsize = AV_RB16(p) + 2;
            if(decode_nal_units_noexecute(h, p, nalsize)  != nalsize) {
                av_log(avctx, AV_LOG_ERROR, "Decoding sps %d from avcC failed\n", i);
                return -1;
            }
            p += nalsize;
        }
        // Decode pps from avcC
        //cnt = *(p++); // Number of pps
        for (i = 0; p<pend-2; i++) {
            nalsize = AV_RB16(p) + 2;
            if(decode_nal_units_noexecute(h, p, nalsize)  != nalsize) {
                av_log(avctx, AV_LOG_ERROR, "Decoding pps %d from avcC failed\n", i);
                return -1;
            }
            p += nalsize;
        }
        // Now store right nal length size, that will be use to parse all other nals
        h->nal_length_size = avctx->nal_length_size?avctx->nal_length_size:4;//((*(((char*)(avctx->extradata))+4))&0x03)+1;
        // Do not reparse avcC
        h->got_avcC = 1;
    }

    if(avctx->frame_number==0 && !h->is_avc && s->avctx->extradata_size){
        if(decode_nal_units_noexecute(h, s->avctx->extradata, s->avctx->extradata_size) < 0)
            return -1;
        s->picture_number++;
    }

    buf_index=decode_nal_units_noexecute(h, buf, buf_size);
    if(buf_index < 0)
        return -1;

    //if(!(s->flags2 & CODEC_FLAG2_CHUNKS) && !s->current_picture_ptr){
    //    if (avctx->skip_frame >= AVDISCARD_NONREF || s->hurry_up) return 0;
    //    av_log(avctx, AV_LOG_ERROR, "no frame!\n");
    //    return -1;
    //}

    if(!(s->flags2 & CODEC_FLAG2_CHUNKS) || (s->mb_y >= s->mb_height && s->mb_height)){
        //Picture *out = s->current_picture_ptr;
        //Picture *cur = s->current_picture_ptr;
        //Picture *prev = h->delayed_output_pic;
        int i, pics, cross_idr, out_of_order, out_idx;

        s->mb_y= 0;

        //s->current_picture_ptr->qscale_type= FF_QSCALE_TYPE_H264;
        //s->current_picture_ptr->pict_type= s->pict_type;

        h->prev_frame_num_offset= h->frame_num_offset;
        h->prev_frame_num= h->frame_num;
        if(!s->dropable) {
            h->prev_poc_msb= h->poc_msb;
            h->prev_poc_lsb= h->poc_lsb;
//            execute_ref_pic_marking(h, h->mmco, h->mmco_index);
        }
    }

    //assert(pict->data[0] || !*data_size);
    //ff_print_debug_info(s, pict);

    return get_consumed_bytes(s, buf_index, buf_size);
}
