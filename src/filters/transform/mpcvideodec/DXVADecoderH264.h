/* 
 * $Id: DXVADecoderH264.h 249 2007-09-26 11:07:22Z casimir666 $
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


#pragma once

#include <dxva.h>
#include "DXVADecoder.h"
#include "H264QuantizationMatrix.h"



class CDXVADecoderH264 : public CDXVADecoder
{
public:
	CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IAMVideoAccelerator*  pAMVideoAccelerator, DXVAMode nMode, int nPicEntryNumber);
	CDXVADecoderH264 (CMPCVideoDecFilter* pFilter, IDirectXVideoDecoder* pDirectXVideoDec, DXVAMode nMode, int nPicEntryNumber);
	virtual ~CDXVADecoderH264();

	virtual HRESULT DecodeFrame   (BYTE* pDataIn, UINT nSize, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, BOOL bDiscontinuity);
	virtual void	SetExtraData  (BYTE* pDataIn, UINT nSize);
	virtual void	CopyBitstream (BYTE* pDXVABuffer, BYTE* pBuffer, UINT& nSize);
	virtual void	Flush();

	static QMatrixH264Type			g_nH264QuantMatrix;			// Index for inverse quantization matrix

protected :


private:

	typedef enum
	{
		P_FRAME = 0,
		B_FRAME,
		I_FRAME,
		SP_FRAME,
		SI_FRAME
	} FRAME_TYPE; 

	// === H264 bitstream structures (based on JM / reference decoder)
	#pragma region

	//! Syntaxelement
	typedef struct syntaxelement
	{
		int           type;                  //!< type of syntax element for data part.
		int           value1;                //!< numerical value of syntax element
		int           value2;                //!< for blocked symbols, e.g. run/level
		int           len;                   //!< length of code
		int           inf;                   //!< info part of UVLC code
		unsigned int  bitpattern;            //!< UVLC bitpattern
		int           context;               //!< CABAC context
		int           k;                     //!< CABAC context for coeff_count,uv


		//! for mapping of UVLC to syntaxElement
		void    (*mapping)(int len, int info, int *value1, int *value2);
		//! used for CABAC: refers to actual coding method of each individual syntax element type
//		void  (*reading)(struct syntaxelement *, struct img_par *, DecodingEnvironmentPtr);
	} SyntaxElement;

	typedef enum
	{
		NALU_TYPE_SLICE    = 1,
		NALU_TYPE_DPA      = 2,
		NALU_TYPE_DPB      = 3,
		NALU_TYPE_DPC      = 4,
		NALU_TYPE_IDR      = 5,
		NALU_TYPE_SEI      = 6,
		NALU_TYPE_SPS      = 7,
		NALU_TYPE_PPS      = 8,
		NALU_TYPE_AUD      = 9,
		NALU_TYPE_EOSEQ    = 10,
		NALU_TYPE_EOSTREAM = 11,
		NALU_TYPE_FILL     = 12
	} NALU_TYPE;

	//FREXT Profile IDC definitions
	typedef enum
	{
		FREXT_HP        = 100,      //!< YUV 4:2:0/8 "High"
		FREXT_Hi10P     = 110,      //!< YUV 4:2:0/10 "High 10"
		FREXT_Hi422     = 122,      //!< YUV 4:2:2/10 "High 4:2:2"
		FREXT_Hi444     = 244,      //!< YUV 4:4:4/14 "High 4:4:4"
		FREXT_CAVLC444  =  44,      //!< YUV 4:4:4/14 "CAVLC 4:4:4"
	} FREXT_PROFILE;


	typedef enum
	{
		YUV400 = 0,
		YUV420 = 1,
		YUV422 = 2,
		YUV444 = 3
	} YUV_TYPE;


	typedef struct
	{
	  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	  unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	  unsigned max_size;            //! Nal Unit Buffer size
	  int forbidden_bit;            //! should be always FALSE
	  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	  int nal_unit_type;            //! NALU_TYPE_xxxx    
	  byte *buf;                    //! contains the first byte followed by the EBSP
	} NALU;

	#define MAXnum_slice_groups_minus1  8
	typedef struct
	{
		bool				Valid;												// indicates the parameter set is valid
		unsigned  int		pic_parameter_set_id;								// ue(v)
		unsigned  int		seq_parameter_set_id;								// ue(v)
		bool				entropy_coding_mode_flag;							// u(1)

		bool				transform_8x8_mode_flag;							// u(1)

		bool				pic_scaling_matrix_present_flag;					// u(1)

		// if( pic_order_cnt_type < 2 )  in the sequence parameter set
		bool				pic_order_present_flag;								// u(1)
		unsigned  int		num_slice_groups_minus1;							// ue(v)
		unsigned  int		slice_group_map_type;								// ue(v)
		// if( slice_group_map_type = = 0 )
		unsigned  int		run_length_minus1[MAXnum_slice_groups_minus1];		// ue(v)
		// else if( slice_group_map_type = = 2 )
		unsigned  int		top_left[MAXnum_slice_groups_minus1];				// ue(v)
		unsigned  int		bottom_right[MAXnum_slice_groups_minus1];			// ue(v)
		// else if( slice_group_map_type = = 3 || 4 || 5
		bool				slice_group_change_direction_flag;					// u(1)
		unsigned  int		slice_group_change_rate_minus1;						// ue(v)
		// else if( slice_group_map_type = = 6 )
		unsigned  int		num_slice_group_map_units_minus1;					// ue(v)
		unsigned  int*		slice_group_id;										// complete MBAmap u(v)
		unsigned  int		num_ref_idx_l0_active_minus1;						 // ue(v)
		unsigned  int		num_ref_idx_l1_active_minus1;						// ue(v)
		bool				weighted_pred_flag;									// u(1)
		unsigned  int		weighted_bipred_idc;								// u(2)
		int					pic_init_qp_minus26;								// se(v)
		int					pic_init_qs_minus26;								// se(v)
		int					chroma_qp_index_offset;								// se(v)

		int					second_chroma_qp_index_offset;						// se(v)

		bool				deblocking_filter_control_present_flag;				// u(1)
		bool				constrained_intra_pred_flag;						// u(1)
		bool				redundant_pic_cnt_present_flag;						// u(1)
	} PIC_PARAMETER_SET_RBSP;



	#define MAXnum_ref_frames_in_pic_order_cnt_cycle  256
	typedef struct
	{
		bool				Valid;                  // indicates the parameter set is valid

		unsigned  int		profile_idc;										// u(8)
		bool				constrained_set0_flag;								// u(1)
		bool				constrained_set1_flag;								// u(1)
		bool				constrained_set2_flag;								// u(1)
		bool				constrained_set3_flag;								// u(1)
		unsigned  int		level_idc;											// u(8)
		unsigned  int		seq_parameter_set_id;								// ue(v)
		unsigned  int		chroma_format_idc;									// ue(v)

		bool				seq_scaling_matrix_present_flag;					// u(1)
		int					seq_scaling_list_present_flag[12];					// u(1)
		int					ScalingList4x4[6][16];								// se(v)
		int					ScalingList8x8[6][64];								// se(v)
		bool				UseDefaultScalingMatrix4x4Flag[6];
		bool				UseDefaultScalingMatrix8x8Flag[6];

		unsigned int		bit_depth_luma_minus8;								// ue(v)
		unsigned int		bit_depth_chroma_minus8;							// ue(v)

		unsigned int		log2_max_frame_num_minus4;							// ue(v)
		unsigned int		pic_order_cnt_type;
		// if( pic_order_cnt_type == 0 )
		unsigned			log2_max_pic_order_cnt_lsb_minus4;					// ue(v)
		// else if( pic_order_cnt_type == 1 )
		bool				delta_pic_order_always_zero_flag;					// u(1)
		int					offset_for_non_ref_pic;								// se(v)
		int					offset_for_top_to_bottom_field;						// se(v)
		unsigned int		num_ref_frames_in_pic_order_cnt_cycle;				// ue(v)
		// for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
		int					offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)
		unsigned int		num_ref_frames;										// ue(v)
		bool				gaps_in_frame_num_value_allowed_flag;				// u(1)
		unsigned int		pic_width_in_mbs_minus1;							// ue(v)
		unsigned int		pic_height_in_map_units_minus1;						// ue(v)
		bool				frame_mbs_only_flag;								// u(1)
		// if( !frame_mbs_only_flag )
		bool				mb_adaptive_frame_field_flag;						// u(1)
		bool				direct_8x8_inference_flag;							// u(1)
		bool				frame_cropping_flag;								// u(1)
		unsigned int		frame_cropping_rect_left_offset;					// ue(v)
		unsigned int		frame_cropping_rect_right_offset;					// ue(v)
		unsigned int		frame_cropping_rect_top_offset;						// ue(v)
		unsigned int		frame_cropping_rect_bottom_offset;					// ue(v)
		bool				vui_parameters_present_flag;						// u(1)
		//		VUI_SEQ_PARAMETERS vui_seq_parameters;							// vui_seq_parameters_t
		unsigned			residual_colour_transform_flag;                     // u(1)	separate_colour_plane_flag
	} SEQ_PARAMETER_SET_RBSP;


	// image parameters
	typedef struct
	{
		int                 first_mb_in_slice;
		int                 pic_parameter_set_id;   //!<the ID of the picture parameter set the slice is reffering to
		FRAME_TYPE			slice_type;
		UINT				frame_num;
		int					colour_plane_id;
		unsigned int		field_pic_flag;
		unsigned int		bottom_field_flag;
		int					idr_flag;
		int					nal_reference_idc;                       //!< nal_reference_idc from NAL unit
		int					idr_pic_id;

		//the following is for slice header syntax elements of poc
		// for poc mode 0.
		unsigned int		pic_order_cnt_lsb;
		int					delta_pic_order_cnt_bottom;
		// for poc mode 1.
		int					delta_pic_order_cnt[3];

		int					toppoc;      //poc for this top field // POC200301
		int					bottompoc;   //poc of bottom field of frame
		int					framepoc;    //poc of this frame // POC200301

		// ////////////////////////
		// for POC mode 0:
		signed int			PrevPicOrderCntMsb;
		unsigned int		PrevPicOrderCntLsb;
		signed int			PicOrderCntMsb;

		// for POC mode 1:
		unsigned int		AbsFrameNum;
		signed int			ExpectedPicOrderCnt, PicOrderCntCycleCnt, FrameNumInPicOrderCntCycle;
		unsigned int		PreviousFrameNum, FrameNumOffset;
		int					ExpectedDeltaPerPicOrderCntCycle;
		int					PreviousPOC, ThisPOC;
		int					PreviousFrameNumOffset;
	} SLICE_PARAMETER;

	#pragma endregion


	DXVA_PicParams_H264		m_DXVAPicParams;
	PIC_PARAMETER_SET_RBSP	m_PicParam;
	SEQ_PARAMETER_SET_RBSP	m_SeqParam;
	SLICE_PARAMETER			m_Slice;

	// Variables for RefFrameList fields
	UINT					m_nCurRefFrame;		// First free RefFrameList position
	
	DXVA_Slice_H264_Short	m_SliceShort;

	// Private functions
	void					Init();

	// DXVA functions
	void					InitPictureParams();
	void					UpdatePictureParams (int nFrameNum, bool bRefFrame);

	// Bitstream parsing functions from JM Ref h264 decoder (http://iphome.hhi.de/suehring/tml/)
	int						GetBits (byte buffer[],int totbitoffset,int *info, int bytecount, int numbits);
	int						GetVLCSymbol (BYTE* buffer,int totbitoffset,int *info, int bytecount);
	int						ReadSyntaxElement_VLC(SyntaxElement *sym, BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset);
	int						ReadSyntaxElement_FLC(SyntaxElement *sym, BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset);
	int						ue_v (BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset);
	int						se_v (BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset);
	int						u_v (int LenInBits, BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset);
	bool					u_1 (BYTE* pBuffer, UINT nBufferLength, UINT& nBitOffset);
	static void				LInfo_Ue(int len, int info, int *value1, int *dummy);
	static void				LInfo_Se(int len,  int info, int *value1, int *dummy);
	void					ReadSliceHeader(SLICE_PARAMETER* pSlice, BYTE* pBuffer, UINT nBufferLength);
	void					DecodePOC(SLICE_PARAMETER* pSlice);

	int						MoreRBSPData (BYTE* buffer,int totbitoffset,int bytecount);
	void					ReadNalu (NALU* Nalu, BYTE* pDataIn, UINT nSize);
	void					ReadPPS(PIC_PARAMETER_SET_RBSP* pps, BYTE* pDataIn, UINT nSize);
	void					ReadSPS(SEQ_PARAMETER_SET_RBSP* sps, BYTE* pBuffer, UINT nBufferLength);
};
