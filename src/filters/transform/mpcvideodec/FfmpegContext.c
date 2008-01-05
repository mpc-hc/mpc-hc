/* 
 * $Id: FfmpegContext.c 249 2007-09-26 11:07:22Z casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#define HAVE_AV_CONFIG_H
#define CONFIG_H264_PARSER
#define CONFIG_VORBIS_DECODER
#define CONFIG_H264_DECODER
#define H264_MERGE_TESTING

#include <windows.h>
#include <winnt.h>
#include "FfmpegContext.h"
#include "dsputil.h"
#include "avcodec.h"
#include "mpegvideo.h"
#include "golomb.h"

#include "h264.h"
#include "h264data.h"
#include "vc1.h"



void FFH264UpdatePictureParam (int bInit, DXVA_PicParams_H264* pDXVAPicParams, DXVA_Qmatrix_H264* pDXVAScalingMatrix, struct AVCodecContext* pAVCtx)
{
	H264Context*	pContext	= (H264Context*) pAVCtx->priv_data;
	SPS*			cur_sps		= pContext->sps_buffers[0];
	PPS*			cur_pps		= pContext->pps_buffers[0];
	int				i;

	if (bInit)
	{
		memset (pDXVAPicParams, 0, sizeof (DXVA_PicParams_H264));
		pDXVAPicParams->MbsConsecutiveFlag					= 1;
		pDXVAPicParams->Reserved16Bits						= 0;
		pDXVAPicParams->ContinuationFlag					= 1;
		pDXVAPicParams->Reserved8BitsA						= 0;
		pDXVAPicParams->Reserved8BitsB						= 0;
		pDXVAPicParams->MinLumaBipredSize8x8Flag			= 1;	// Improve accelerator performances
		pDXVAPicParams->StatusReportFeedbackNumber			= 0;	// Use to report status

		for (i =0; i<16; i++)
		{
			pDXVAPicParams->RefFrameList[i].AssociatedFlag	= 1;
			pDXVAPicParams->RefFrameList[i].bPicEntry		= 255;
			pDXVAPicParams->RefFrameList[i].Index7Bits		= 127;
		}
	}

	pDXVAPicParams->wFrameWidthInMbsMinus1			= cur_sps->mb_width  - 1;		// pic_width_in_mbs_minus1;
	pDXVAPicParams->wFrameHeightInMbsMinus1			= cur_sps->mb_height - 1;		// pic_height_in_map_units_minus1;
	pDXVAPicParams->num_ref_frames					= cur_sps->ref_frame_count;		// num_ref_frames;
//	pDXVAPicParams->field_pic_flag					= SET IN DecodeFrame;
//	pDXVAPicParams->MbaffFrameFlag					= SET IN DecodeFrame;
	pDXVAPicParams->residual_colour_transform_flag	= cur_sps->residual_colour_transform_flag;
//	pDXVAPicParams->sp_for_switch_flag				= SET IN DecodeFrame;
	pDXVAPicParams->chroma_format_idc				= cur_sps->chroma_format_idc;
//	pDXVAPicParams->RefPicFlag						= SET IN DecodeFrame;
	pDXVAPicParams->constrained_intra_pred_flag		= cur_pps->constrained_intra_pred;
	pDXVAPicParams->weighted_pred_flag				= cur_pps->weighted_pred;
	pDXVAPicParams->weighted_bipred_idc				= cur_pps->weighted_bipred_idc;
	pDXVAPicParams->frame_mbs_only_flag				= cur_sps->frame_mbs_only_flag;
	pDXVAPicParams->transform_8x8_mode_flag			= cur_pps->transform_8x8_mode;
//	pDXVAPicParams->IntraPicFlag					= SET IN DecodeFrame;

	pDXVAPicParams->bit_depth_luma_minus8			= cur_sps->bit_depth_luma   - 8;	// bit_depth_luma_minus8
	pDXVAPicParams->bit_depth_chroma_minus8			= cur_sps->bit_depth_chroma - 8;	// bit_depth_chroma_minus8
//	pDXVAPicParams->StatusReportFeedbackNumber		= SET IN DecodeFrame;


//	pDXVAPicParams->CurrFieldOrderCnt						= SET IN UpdateRefFramesList;
//	pDXVAPicParams->FieldOrderCntList						= SET IN UpdateRefFramesList;
//	pDXVAPicParams->FrameNumList							= SET IN UpdateRefFramesList;
//	pDXVAPicParams->UsedForReferenceFlags					= SET IN UpdateRefFramesList;
//	pDXVAPicParams->NonExistingFrameFlags
//	pDXVAPicParams->SliceGroupMap


	pDXVAPicParams->log2_max_frame_num_minus4				= cur_sps->log2_max_frame_num - 4;					// log2_max_frame_num_minus4;
	pDXVAPicParams->pic_order_cnt_type						= cur_sps->poc_type;								// pic_order_cnt_type;
	pDXVAPicParams->log2_max_pic_order_cnt_lsb_minus4		= cur_sps->log2_max_poc_lsb - 4;					// log2_max_pic_order_cnt_lsb_minus4;
	pDXVAPicParams->delta_pic_order_always_zero_flag		= cur_sps->delta_pic_order_always_zero_flag;
	pDXVAPicParams->direct_8x8_inference_flag				= cur_sps->direct_8x8_inference_flag;
	pDXVAPicParams->entropy_coding_mode_flag				= cur_pps->cabac;									// entropy_coding_mode_flag;
	pDXVAPicParams->pic_order_present_flag					= cur_pps->pic_order_present;						// pic_order_present_flag;
	pDXVAPicParams->num_slice_groups_minus1					= cur_pps->slice_group_count - 1;					// num_slice_groups_minus1;
	pDXVAPicParams->slice_group_map_type					= cur_pps->mb_slice_group_map_type;					// slice_group_map_type;
	pDXVAPicParams->deblocking_filter_control_present_flag	= cur_pps->deblocking_filter_parameters_present;	// deblocking_filter_control_present_flag;
	pDXVAPicParams->redundant_pic_cnt_present_flag			= cur_pps->redundant_pic_cnt_present;				// redundant_pic_cnt_present_flag;
	pDXVAPicParams->slice_group_change_rate_minus1			= cur_pps->slice_group_change_rate_minus1;

	pDXVAPicParams->pic_init_qp_minus26						= cur_pps->init_qp - 26;							// pic_init_qp_minus26;
	pDXVAPicParams->chroma_qp_index_offset					= cur_pps->chroma_qp_index_offset[0];
	pDXVAPicParams->second_chroma_qp_index_offset			= cur_pps->chroma_qp_index_offset[1];
	pDXVAPicParams->num_ref_idx_l0_active_minus1			= cur_pps->ref_count[0]-1;							// num_ref_idx_l0_active_minus1;
	pDXVAPicParams->num_ref_idx_l1_active_minus1			= cur_pps->ref_count[1]-1;							// num_ref_idx_l1_active_minus1;

	memcpy (pDXVAScalingMatrix, cur_pps->scaling_matrix4, sizeof (DXVA_Qmatrix_H264));
}


static int decode_ref_pic_list_reordering(H264Context *h){
    MpegEncContext * const s = &h->s;
    int list, index;

    if(h->slice_type==I_TYPE || h->slice_type==SI_TYPE) return 0; //FIXME move before func

    for(list=0; list<h->list_count; list++){
        //memcpy(h->ref_list[list], h->default_ref_list[list], sizeof(Picture)*h->ref_count[list]);

        if(get_bits1(&s->gb)){
            int pred= h->curr_pic_num;

            for(index=0; ; index++){
                unsigned int reordering_of_pic_nums_idc= get_ue_golomb(&s->gb);
                unsigned int pic_id;
                Picture *ref = NULL;

                if(reordering_of_pic_nums_idc==3)
                    break;

                if(reordering_of_pic_nums_idc<3){
                    if(reordering_of_pic_nums_idc<2){
                        const unsigned int abs_diff_pic_num= get_ue_golomb(&s->gb) + 1;

                    }else
					{
                        pic_id= get_ue_golomb(&s->gb); //long_term_pic_idx
                    }

                }else{
                    return E_FAIL;
                }
            }
        }
    }

    return 0;
}


static int pred_weight_table(H264Context *h){
    MpegEncContext * const s = &h->s;
    int list, i;
    int luma_def, chroma_def;

    h->use_weight= 0;
    h->use_weight_chroma= 0;
    h->luma_log2_weight_denom= get_ue_golomb(&s->gb);
    h->chroma_log2_weight_denom= get_ue_golomb(&s->gb);
    luma_def = 1<<h->luma_log2_weight_denom;
    chroma_def = 1<<h->chroma_log2_weight_denom;

    for(list=0; list<2; list++){
        for(i=0; i<h->ref_count[list]; i++){
            int luma_weight_flag, chroma_weight_flag;

            luma_weight_flag= get_bits1(&s->gb);
            if(luma_weight_flag){
                h->luma_weight[list][i]= get_se_golomb(&s->gb);
                h->luma_offset[list][i]= get_se_golomb(&s->gb);
                if(   h->luma_weight[list][i] != luma_def
                   || h->luma_offset[list][i] != 0)
                    h->use_weight= 1;
            }else{
                h->luma_weight[list][i]= luma_def;
                h->luma_offset[list][i]= 0;
            }

            chroma_weight_flag= get_bits1(&s->gb);
            if(chroma_weight_flag){
                int j;
                for(j=0; j<2; j++){
                    h->chroma_weight[list][i][j]= get_se_golomb(&s->gb);
                    h->chroma_offset[list][i][j]= get_se_golomb(&s->gb);
                    if(   h->chroma_weight[list][i][j] != chroma_def
                       || h->chroma_offset[list][i][j] != 0)
                        h->use_weight_chroma= 1;
                }
            }else{
                int j;
                for(j=0; j<2; j++){
                    h->chroma_weight[list][i][j]= chroma_def;
                    h->chroma_offset[list][i][j]= 0;
                }
            }
        }
        if(h->slice_type != B_TYPE) break;
    }
    h->use_weight= h->use_weight || h->use_weight_chroma;
    return 0;
}

static int decode_ref_pic_marking(H264Context *h, GetBitContext *gb){
    MpegEncContext * const s = &h->s;
    int i;

    if(h->nal_unit_type == NAL_IDR_SLICE){ //FIXME fields
        s->broken_link= get_bits1(gb) -1;
        h->mmco[0].long_arg= get_bits1(gb) - 1; // current_long_term_idx
        if(h->mmco[0].long_arg == -1)
            h->mmco_index= 0;
        else{
            h->mmco[0].opcode= MMCO_LONG;
            h->mmco_index= 1;
        }
    }else{
        if(get_bits1(gb)){ // adaptive_ref_pic_marking_mode_flag
            for(i= 0; i<MAX_MMCO_COUNT; i++) {
                MMCOOpcode opcode= get_ue_golomb(gb);

                h->mmco[i].opcode= opcode;
                if(opcode==MMCO_SHORT2UNUSED || opcode==MMCO_SHORT2LONG){
                    h->mmco[i].short_pic_num= (h->curr_pic_num - get_ue_golomb(gb) - 1) & (h->max_pic_num - 1);
/*                    if(h->mmco[i].short_pic_num >= h->short_ref_count || h->short_ref[ h->mmco[i].short_pic_num ] == NULL){
                        av_log(s->avctx, AV_LOG_ERROR, "illegal short ref in memory management control operation %d\n", mmco);
                        return -1;
                    }*/
                }
                if(opcode==MMCO_SHORT2LONG || opcode==MMCO_LONG2UNUSED || opcode==MMCO_LONG || opcode==MMCO_SET_MAX_LONG){
                    unsigned int long_arg= get_ue_golomb(gb);
                    if(long_arg >= 32 || (long_arg >= 16 && !(opcode == MMCO_LONG2UNUSED && FIELD_PICTURE))){
                        av_log(h->s.avctx, AV_LOG_ERROR, "illegal long ref in memory management control operation %d\n", opcode);
                        return -1;
                    }
                    h->mmco[i].long_arg= long_arg;
                }

                if(opcode > (unsigned)MMCO_LONG){
                    av_log(h->s.avctx, AV_LOG_ERROR, "illegal memory management control operation %d\n", opcode);
                    return -1;
                }
                if(opcode == MMCO_END)
                    break;
            }
            h->mmco_index= i;
        }else{
            assert(h->long_ref_count + h->short_ref_count <= h->sps.ref_frame_count);

            if(h->short_ref_count && h->long_ref_count + h->short_ref_count == h->sps.ref_frame_count &&
                    !(FIELD_PICTURE && !s->first_field && s->current_picture_ptr->reference)) {
                h->mmco[0].opcode= MMCO_SHORT2UNUSED;
                h->mmco[0].short_pic_num= h->short_ref[ h->short_ref_count - 1 ]->frame_num;
                h->mmco_index= 1;
                if (FIELD_PICTURE) {
                    h->mmco[0].short_pic_num *= 2;
                    h->mmco[1].opcode= MMCO_SHORT2UNUSED;
                    h->mmco[1].short_pic_num= h->mmco[0].short_pic_num + 1;
                    h->mmco_index= 2;
                }
            }else
                h->mmco_index= 0;
        }
    }

    return 0;
}


HRESULT FFH264ReadSlideHeader (DXVA_PicParams_H264* pDXVAPicParams, struct AVCodecContext* pAVCtx, BYTE* pBuffer, UINT nSize)
{
	H264Context*			h			= (H264Context*) pAVCtx->priv_data;
    MpegEncContext* const	s = &h->s;
	H264Context*			hx;
	SPS*					cur_sps		= h->sps_buffers[0];
	PPS*					cur_pps		= h->pps_buffers[0];
	int						tmp;
    unsigned int			first_mb_in_slice;
	unsigned int			pps_id;
    static const uint8_t	slice_type_map[5]= {P_TYPE, B_TYPE, I_TYPE, SP_TYPE, SI_TYPE};
    unsigned int			slice_type;
    int						num_ref_idx_active_override_flag;
	int						sp_for_switch_flag = 0;
	int						field_pic_flag;

	hx					= h->thread_context[0];
    h->nal_ref_idc		= pBuffer[0]>>5;
    h->nal_unit_type	= pBuffer[0]&0x1F;

    init_get_bits(&hx->s.gb, pBuffer+1, nSize-1);
    hx->intra_gb_ptr=
    hx->inter_gb_ptr= &hx->s.gb;
    hx->s.data_partitioning = 0;


    first_mb_in_slice	= get_ue_golomb(&s->gb);
    slice_type			= get_ue_golomb(&s->gb);

    if(slice_type > 9)
        return E_FAIL;
    
    if(slice_type > 4)
	{
        slice_type -= 5;
        h->slice_type_fixed=1;
    }
	else
        h->slice_type_fixed=0;

    h->slice_type= slice_type_map[ slice_type ];
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


	s->mb_width= h->sps.mb_width;
    s->mb_height= h->sps.mb_height * (2 - h->sps.frame_mbs_only_flag);

    h->frame_num= get_bits(&s->gb, h->sps.log2_max_frame_num);

    h->mb_mbaff = 0;
    h->mb_aff_frame = 0;
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

//    init_poc(h);

    if(h->pps.redundant_pic_cnt_present){
        h->redundant_pic_count= get_ue_golomb(&s->gb);
    }

    //set defaults, might be overriden a few line later
    h->ref_count[0]= h->pps.ref_count[0];
    h->ref_count[1]= h->pps.ref_count[1];

    if(h->slice_type == P_TYPE || h->slice_type == SP_TYPE || h->slice_type == B_TYPE){
        if(h->slice_type == B_TYPE){
            h->direct_spatial_mv_pred= get_bits1(&s->gb);
            //if(FIELD_OR_MBAFF_PICTURE && h->direct_spatial_mv_pred)
            //    av_log(h->s.avctx, AV_LOG_ERROR, "Interlaced pictures + spatial direct mode is not implemented\n");
        }
        num_ref_idx_active_override_flag= get_bits1(&s->gb);

        if(num_ref_idx_active_override_flag){
            h->ref_count[0]= get_ue_golomb(&s->gb) + 1;
            if(h->slice_type==B_TYPE)
                h->ref_count[1]= get_ue_golomb(&s->gb) + 1;

            if(h->ref_count[0]-1 > 32-1 || h->ref_count[1]-1 > 32-1){
                //av_log(h->s.avctx, AV_LOG_ERROR, "reference overflow\n");
                h->ref_count[0]= h->ref_count[1]= 1;
                return E_FAIL;
            }
        }
        if(h->slice_type == B_TYPE)
            h->list_count= 2;
        else
            h->list_count= 1;
    }else
        h->list_count= 0;

    if(decode_ref_pic_list_reordering(h) < 0)
        return E_FAIL;

    if(   (h->pps.weighted_pred          && (h->slice_type == P_TYPE || h->slice_type == SP_TYPE ))
       || (h->pps.weighted_bipred_idc==1 && h->slice_type==B_TYPE ) )
        pred_weight_table(h);

    if(h->nal_ref_idc)
        decode_ref_pic_marking(h, &s->gb);

    if( h->slice_type != I_TYPE && h->slice_type != SI_TYPE && h->pps.cabac ){
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

	if(h->slice_type == SP_TYPE){
        sp_for_switch_flag = get_bits1(&s->gb); // sp_for_switch_flag
    }
    if(h->slice_type==SP_TYPE || h->slice_type == SI_TYPE){
        get_se_golomb(&s->gb); // slice_qs_delta
    }
	


	field_pic_flag = (h->s.picture_structure != PICT_FRAME);
	pDXVAPicParams->wFrameWidthInMbsMinus1		= s->mb_width  - 1;		// pic_width_in_mbs_minus1;
	pDXVAPicParams->wFrameHeightInMbsMinus1		= s->mb_height - 1;		// pic_height_in_map_units_minus1;

	pDXVAPicParams->field_pic_flag				= field_pic_flag;
	pDXVAPicParams->RefPicFlag					= (h->nal_ref_idc != 0);
	pDXVAPicParams->IntraPicFlag				= (h->slice_type == I_TYPE );
	pDXVAPicParams->MbaffFrameFlag				= (h->sps.mb_aff && (field_pic_flag==0));
	pDXVAPicParams->frame_num					= h->frame_num;
	pDXVAPicParams->sp_for_switch_flag			= sp_for_switch_flag;

	if (field_pic_flag)
		pDXVAPicParams->CurrPic.AssociatedFlag	= h->delta_poc[0];
	pDXVAPicParams->CurrFieldOrderCnt[0] = h->poc_lsb;		// m_Slice.pic_order_cnt_lsb;
	pDXVAPicParams->CurrFieldOrderCnt[1] = h->poc_lsb;		// m_Slice.pic_order_cnt_lsb;

	return S_OK;
}



/*
void FF_H264_ReadSliceHeader(DXVA_PicParams_H264* pDXVAPicParams, struct AVCodecContext* pAVCtx, BYTE* pBuffer, UINT nLength)
{
	H264Context*	h			= (VC1Context*) pAVCtx->priv_data;
	H264Context*	hx;
	int				field_pic_flag;

	hx = h->thread_context[0];

    h->nal_ref_idc		= pBuffer[0]>>5;
    h->nal_unit_type	= pBuffer[0]&0x1F;

    init_get_bits(&hx->s.gb, pBuffer+1, nLength-1);
    hx->intra_gb_ptr=
    hx->inter_gb_ptr= &hx->s.gb;
    hx->s.data_partitioning = 0;

    decode_slice_header(hx, h);

	field_pic_flag = (h->s.picture_structure != PICT_FRAME);
	pDXVAPicParams->field_pic_flag		= field_pic_flag;
	pDXVAPicParams->RefPicFlag			= (h->nal_ref_idc != 0);
	pDXVAPicParams->IntraPicFlag		= (h->slice_type == 2 );
	pDXVAPicParams->MbaffFrameFlag		= (h->sps.mb_aff && (field_pic_flag==0));
	pDXVAPicParams->frame_num			= h->frame_num;
//	pDXVAPicParams->sp_for_switch_flag	= h->sp_for_switch_flag;
}
*/


void FillVC1Context (DXVA_PictureParameters* pPicParams, AVCodecContext* pAVCtx)
{
	VC1Context*		vc1 = (VC1Context*) pAVCtx->priv_data;


	//   Ok     Todo    Todo    Todo    Ok
	// iWMV9 - i9IRU - iOHIT - iINSO - iWMVA		| Section 3.2.5
	pPicParams->bBidirectionalAveragingMode	= ((vc1->profile == PROFILE_ADVANCED)<<3) | (1 << 7);

	// Section 3.2.20.3
	pPicParams->bPicSpatialResid8	= (vc1->panscanflag   << 7) | (vc1->refdist  << 6) |
									  (vc1->s.loop_filter << 5) | (vc1->fastuvmc << 4) | 
									  (vc1->extended_mv   << 3) | (vc1->dquant   << 1) | 
									  (vc1->vstransform);

	// Section 3.2.20.4
	pPicParams->bPicOverflowBlocks  = (vc1->quantizer_mode  << 6) | (vc1->multires << 5) |
									  (vc1->s.resync_marker << 4) | (vc1->rangered << 3) |
									  (vc1->s.max_b_frames);

	//				TODO section 3.2.20.6
	//pPicParams->bPicDeblocked		= 
}
