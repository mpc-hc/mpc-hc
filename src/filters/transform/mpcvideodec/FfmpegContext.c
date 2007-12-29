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
#include "h264.h"


void FillH264Context (DXVA_PicParams_H264* pDXVAPicParams, void* pPrivData)
{
	H264Context*	pContext = (H264Context*) pPrivData;
	int				i;

	pDXVAPicParams->wBitFields						= 0;
	pDXVAPicParams->wFrameWidthInMbsMinus1			= pContext->sps.mb_width  - 1;		// pic_width_in_mbs_minus1;
	pDXVAPicParams->wFrameHeightInMbsMinus1			= pContext->sps.mb_height - 1;		// pic_height_in_map_units_minus1;
	pDXVAPicParams->num_ref_frames					= pContext->sps.ref_frame_count;	// num_ref_frames;
//	pDXVAPicParams->field_pic_flag					= SET IN DecodeFrame;
//	pDXVAPicParams->MbaffFrameFlag					= SET IN DecodeFrame;
//	pDXVAPicParams->sp_for_switch_flag
//	pDXVAPicParams->RefPicFlag						= SET IN DecodeFrame;
	pDXVAPicParams->constrained_intra_pred_flag		= pContext->pps.constrained_intra_pred;
	pDXVAPicParams->weighted_pred_flag				= pContext->pps.weighted_pred;
	pDXVAPicParams->weighted_bipred_idc				= pContext->pps.weighted_bipred_idc;
	pDXVAPicParams->MbsConsecutiveFlag				= 1;	// TODO : always activate ??
	pDXVAPicParams->frame_mbs_only_flag				= pContext->sps.frame_mbs_only_flag;
	pDXVAPicParams->transform_8x8_mode_flag			= pContext->pps.transform_8x8_mode;
	pDXVAPicParams->MinLumaBipredSize8x8Flag		= 1;	// TODO : always activate ???
//	pDXVAPicParams->IntraPicFlag

	pDXVAPicParams->Reserved16Bits					= 0;
//	pDXVAPicParams->StatusReportFeedbackNumber		= SET IN DecodeFrame;

	for (i =0; i<16; i++)
	{
		pDXVAPicParams->RefFrameList[i].AssociatedFlag	= 1;
		pDXVAPicParams->RefFrameList[i].bPicEntry		= 255;
		pDXVAPicParams->RefFrameList[i].Index7Bits		= 127;
	}

//	pDXVAPicParams->CurrFieldOrderCnt						= SET IN UpdatePictureParams;
//	pDXVAPicParams->FieldOrderCntList						= SET IN UpdatePictureParams;
	pDXVAPicParams->ContinuationFlag						= 1;
	pDXVAPicParams->Reserved8BitsA							= 0;
//	pDXVAPicParams->FrameNumList							= SET IN UpdatePictureParams;
//	pDXVAPicParams->UsedForReferenceFlags					= SET IN UpdatePictureParams;
//	pDXVAPicParams->NonExistingFrameFlags
	pDXVAPicParams->Reserved8BitsB							= 0;
//	pDXVAPicParams->SliceGroupMap


	pDXVAPicParams->log2_max_frame_num_minus4				= pContext->sps.log2_max_frame_num - 4;					// log2_max_frame_num_minus4;
	pDXVAPicParams->pic_order_cnt_type						= pContext->sps.poc_type;								// pic_order_cnt_type;
	pDXVAPicParams->log2_max_pic_order_cnt_lsb_minus4		= pContext->sps.log2_max_poc_lsb - 4;						// log2_max_pic_order_cnt_lsb_minus4;
	pDXVAPicParams->delta_pic_order_always_zero_flag		= pContext->sps.delta_pic_order_always_zero_flag;
	pDXVAPicParams->direct_8x8_inference_flag				= pContext->sps.direct_8x8_inference_flag;
	pDXVAPicParams->entropy_coding_mode_flag				= pContext->pps.cabac;									// entropy_coding_mode_flag;
	pDXVAPicParams->pic_order_present_flag					= pContext->pps.pic_order_present;						// pic_order_present_flag;
	pDXVAPicParams->num_slice_groups_minus1					= pContext->pps.slice_group_count - 1;					// num_slice_groups_minus1;
	pDXVAPicParams->slice_group_map_type					= pContext->pps.mb_slice_group_map_type;				// slice_group_map_type;
	pDXVAPicParams->deblocking_filter_control_present_flag	= pContext->pps.deblocking_filter_parameters_present;	// deblocking_filter_control_present_flag;
	pDXVAPicParams->redundant_pic_cnt_present_flag			= pContext->pps.redundant_pic_cnt_present;				// redundant_pic_cnt_present_flag;
	pDXVAPicParams->slice_group_change_rate_minus1			= 0;// TODO :	pContext->pps.slice_group_change_rate_minus1;

	pDXVAPicParams->pic_init_qp_minus26						= pContext->pps.init_qp - 26;							// pic_init_qp_minus26;
	pDXVAPicParams->num_ref_idx_l0_active_minus1			= pContext->pps.ref_count[0]-1;							// num_ref_idx_l0_active_minus1;
	pDXVAPicParams->num_ref_idx_l1_active_minus1			= pContext->pps.ref_count[1]-1;							// num_ref_idx_l1_active_minus1;

	// ==> Patch for ffmpeg!
	/*
	pDXVAPicParams->chroma_format_idc				= pContext->sps.chroma_format_idc;
	pDXVAPicParams->residual_colour_transform_flag	= pContext->sps.residual_colour_transform_flag;
	pDXVAPicParams->bit_depth_luma_minus8			= pContext->sps.bit_depth_luma   - 8;	// bit_depth_luma_minus8
	pDXVAPicParams->bit_depth_chroma_minus8			= pContext->sps.bit_depth_chroma - 8;	// bit_depth_chroma_minus8
	*/
}