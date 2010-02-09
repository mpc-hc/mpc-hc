/*
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include <stdint.h>

#define MPEG2_VERSION(a,b,c) (((a)<<16)|((b)<<8)|(c))
#define MPEG2_RELEASE MPEG2_VERSION (0, 3, 2)	/* 0.3.2 */

#define SEQ_FLAG_MPEG2 1
#define SEQ_FLAG_CONSTRAINED_PARAMETERS 2
#define SEQ_FLAG_PROGRESSIVE_SEQUENCE 4
#define SEQ_FLAG_LOW_DELAY 8
#define SEQ_FLAG_COLOUR_DESCRIPTION 16

#define SEQ_MASK_VIDEO_FORMAT 0xe0
#define SEQ_VIDEO_FORMAT_COMPONENT 0
#define SEQ_VIDEO_FORMAT_PAL 0x20
#define SEQ_VIDEO_FORMAT_NTSC 0x40
#define SEQ_VIDEO_FORMAT_SECAM 0x60
#define SEQ_VIDEO_FORMAT_MAC 0x80
#define SEQ_VIDEO_FORMAT_UNSPECIFIED 0xa0

typedef struct {
    unsigned int width, height;
    unsigned int chroma_width, chroma_height;
    unsigned int byte_rate;
    unsigned int vbv_buffer_size;
    uint32_t flags;

    unsigned int picture_width, picture_height;
    unsigned int display_width, display_height;
    unsigned int pixel_width, pixel_height;
    unsigned int frame_period;

    uint8_t profile_level_id;
    uint8_t colour_primaries;
    uint8_t transfer_characteristics;
    uint8_t matrix_coefficients;

	void finalize();
} mpeg2_sequence_t;

#define GOP_FLAG_DROP_FRAME 1
#define GOP_FLAG_BROKEN_LINK 2
#define GOP_FLAG_CLOSED_GOP 4

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t pictures;
    uint32_t flags;
} mpeg2_gop_t;

#define PIC_MASK_CODING_TYPE 7
#define PIC_FLAG_CODING_TYPE_I 1
#define PIC_FLAG_CODING_TYPE_P 2
#define PIC_FLAG_CODING_TYPE_B 3
#define PIC_FLAG_CODING_TYPE_D 4

#define PIC_FLAG_TOP_FIELD_FIRST 8
#define PIC_FLAG_PROGRESSIVE_FRAME 16
#define PIC_FLAG_COMPOSITE_DISPLAY 32
#define PIC_FLAG_SKIP 64
#define PIC_FLAG_PTS 128
#define PIC_FLAG_REPEAT_FIRST_FIELD 256
#define PIC_MASK_COMPOSITE_DISPLAY 0xfffff000

typedef struct {
    unsigned int temporal_reference;
    unsigned int nb_fields;
    uint32_t pts;
    uint32_t flags;
    struct {
	int x, y;
    } display_offset[3];
	__int64 rtStart, rtStop;
	bool fDiscontinuity, fDelivered;
} mpeg2_picture_t;

typedef struct {
    uint8_t* buf[3];
    void* id;
} mpeg2_fbuf_t;

typedef enum {
    STATE_BUFFER = 0,
    STATE_SEQUENCE = 1,
    STATE_SEQUENCE_REPEATED = 2,
    STATE_GOP = 3,
    STATE_PICTURE = 4,
    STATE_SLICE_1ST = 5,
    STATE_PICTURE_2ND = 6,
    STATE_SLICE = 7,
    STATE_END = 8,
    STATE_INVALID = 9,
    STATE_PADDING = 10
} mpeg2_state_t;

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/* macroblock modes */
#define MACROBLOCK_INTRA 1
#define MACROBLOCK_PATTERN 2
#define MACROBLOCK_MOTION_BACKWARD 4
#define MACROBLOCK_MOTION_FORWARD 8
#define MACROBLOCK_QUANT 16
#define DCT_TYPE_INTERLACED 32
/* motion_type */
#define MOTION_TYPE_MASK (3*64)
#define MOTION_TYPE_BASE 64
#define MC_FIELD (1*64)
#define MC_FRAME (2*64)
#define MC_16X8 (2*64)
#define MC_DMV (3*64)

/* picture structure */
#define TOP_FIELD 1
#define BOTTOM_FIELD 2
#define FRAME_PICTURE 3

/* picture coding type */
#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

typedef void mpeg2_mc_fct(uint8_t*, const uint8_t*, int, int);
typedef struct {mpeg2_mc_fct* put[8]; mpeg2_mc_fct* avg[8];} mpeg2_mc_t;

class CMpeg2Decoder
{
public:
    /* Motion vectors */
    /* The f_ and b_ correspond to the forward and backward motion */
    /* predictors */

	struct motion_t {
		uint8_t* ref[2][3];
		uint8_t** ref2[2];
		int pmv[2][2];
		int f_code[2];
	} m_b_motion, m_f_motion;

private:
	int get_macroblock_modes();
	int get_quantizer_scale();
	int get_motion_delta(const int f_code);
	int bound_motion_vector(const int vector, const int f_code);
	int get_dmv();
	int get_coded_block_pattern();
	int get_luma_dc_dct_diff();
	int get_chroma_dc_dct_diff();
	
	void get_intra_block_B14();
	void get_intra_block_B15();
	int get_non_intra_block();
	void get_mpeg1_intra_block();
	int get_mpeg1_non_intra_block();
	void slice_intra_DCT(const int cc, uint8_t* dest, int stride);
	void slice_non_intra_DCT(uint8_t* dest, int stride);

	void MOTION(mpeg2_mc_fct * const * const table, uint8_t** ref, int motion_x, int motion_y, unsigned int size, unsigned int y, unsigned int limit_y);
	void MOTION_FIELD(mpeg2_mc_fct * const * const table, uint8_t** ref, int motion_x, int motion_y, int dest_field, int src_field, unsigned int op);

	void motion_mp1(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_fr_frame(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_fr_field(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_fr_dmv(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_reuse(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_zero(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_fr_conceal();
	void motion_fi_field(motion_t * motion, mpeg2_mc_fct * const * const table);
	void motion_fi_16x8(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_fi_dmv(motion_t* motion, mpeg2_mc_fct * const * const table);
	void motion_fi_conceal();

	int slice_init(int code);

	static bool m_idct_initialized;
	void (*m_idct_init)();
	void (*m_idct_copy)(int16_t* block, uint8_t* dest, const int stride);
	void (*m_idct_add)(const int last, int16_t* block, uint8_t* dest, const int stride);
	mpeg2_mc_t* m_mc;

public:
	CMpeg2Decoder();
	virtual ~CMpeg2Decoder();

	void mpeg2_init_fbuf(uint8_t* current_fbuf[3], uint8_t* forward_fbuf[3], uint8_t* backward_fbuf[3]);
	void mpeg2_slice(int code, const uint8_t* buffer);

	int16_t* m_DCTblock;

    /* bit parsing stuff */
    uint32_t m_bitstream_buf;		/* current 32 bit working set */
    int m_bitstream_bits;			/* used bits in working set */
    const uint8_t* m_bitstream_ptr;	/* buffer with stream data */

    uint8_t* m_dest[3];
    uint8_t* m_picture_dest[3];

    int m_offset, m_stride, m_uv_stride;
    unsigned int m_limit_x, m_limit_y_16, m_limit_y_8, m_limit_y;

    /* predictor for DC coefficients in intra blocks */
    int16_t m_dc_dct_pred[3];

    int m_quantizer_scale;	/* remove */
    int m_dmv_offset;		/* remove */
    unsigned int m_v_offset;	/* remove */

    /* now non-slice-specific information */

    /* sequence header stuff */
    uint8_t m_intra_quantizer_matrix[64];
    uint8_t m_non_intra_quantizer_matrix[64];

    /* The width and height of the picture snapped to macroblock units */
    int m_width, m_height;
    int m_vertical_position_extension;

    /* picture header stuff */

    /* what type of picture this is (I, P, B, D) */
    int m_coding_type;

    /* picture coding extension stuff */

    /* quantization factor for intra dc coefficients */
    int m_intra_dc_precision;
    /* top/bottom/both fields */
    int m_picture_structure;
    /* bool to indicate all predictions are frame based */
    int m_frame_pred_frame_dct;
    /* bool to indicate whether intra blocks have motion vectors */
    /* (for concealment) */
    int m_concealment_motion_vectors;
    /* bit to indicate which quantization table to use */
    int m_q_scale_type;
    /* bool to use different vlc tables */
    int m_intra_vlc_format;
    /* used for DMV MC */
    int m_top_field_first;

    /* stuff derived from bitstream */

    /* pointer to the zigzag scan we're supposed to be using */
    const uint8_t* m_scan;

    int m_second_field;

    int m_mpeg1;
};

class CMpeg2Info
{
public:
	CMpeg2Info();
	virtual ~CMpeg2Info();

	void Reset();

	mpeg2_sequence_t* m_sequence;
    mpeg2_gop_t* m_gop;
    mpeg2_picture_t* m_current_picture;
    mpeg2_picture_t* m_current_picture_2nd;
    mpeg2_fbuf_t* m_current_fbuf;
    mpeg2_picture_t* m_display_picture;
    mpeg2_picture_t* m_display_picture_2nd;
    mpeg2_fbuf_t* m_display_fbuf;
    mpeg2_fbuf_t* m_discard_fbuf;
    const uint8_t* m_user_data;
    int m_user_data_len;
};

class CMpeg2Dec
{
	int skip_chunk(int bytes);
	int copy_chunk(int bytes);
	mpeg2_state_t seek_chunk(), seek_header(), seek_sequence();

	int sequence_ext();
	int sequence_display_ext();
	int quant_matrix_ext();
	int copyright_ext();
	int picture_display_ext();
	int picture_coding_ext();

public:
	CMpeg2Dec();
	virtual ~CMpeg2Dec();

	void mpeg2_init();
	void mpeg2_close();

	void mpeg2_buffer(uint8_t* start, uint8_t* end);
	int mpeg2_getpos();
	mpeg2_state_t mpeg2_parse();

	void mpeg2_skip(int skip);
	void mpeg2_slice_region(int start, int end);

	void mpeg2_pts(uint32_t pts);

	/* decode.c */
	mpeg2_state_t mpeg2_seek_sequence();
	mpeg2_state_t mpeg2_parse_header();

	/* header.c */
	void mpeg2_header_state_init();
	int mpeg2_header_sequence();
	int mpeg2_header_gop();
	mpeg2_state_t mpeg2_header_picture_start();
	int mpeg2_header_picture();
	int mpeg2_header_extension();
	int mpeg2_header_user_data();
	void mpeg2_header_matrix_finalize();
	void mpeg2_header_sequence_finalize();
	mpeg2_state_t mpeg2_header_slice_start();
	mpeg2_state_t mpeg2_header_end();
	void mpeg2_set_fbuf(int coding_type);

	enum {BUFFER_SIZE = 1194 * 1024};


	CMpeg2Decoder m_decoder;
    CMpeg2Info m_info;

    uint32_t m_shift;
    int m_is_display_initialized;
	mpeg2_state_t (CMpeg2Dec::* m_action)();
    mpeg2_state_t m_state;
    uint32_t m_ext_state;

    /* allocated in init - gcc has problems allocating such big structures */
    uint8_t* m_chunk_buffer;
    /* pointer to start of the current chunk */
    uint8_t* m_chunk_start;
    /* pointer to current position in chunk_buffer */
    uint8_t* m_chunk_ptr;
    /* last start code ? */
    uint8_t m_code;

    /* PTS */
    uint32_t m_pts_current, m_pts_previous;
    int m_num_pts;
    int m_bytes_since_pts;

    bool m_first;
    int m_alloc_index;
    uint8_t m_first_decode_slice;
    uint8_t m_nb_decode_slices;

    mpeg2_sequence_t m_new_sequence;
    mpeg2_sequence_t m_sequence;
    mpeg2_gop_t m_gop;
    mpeg2_picture_t m_pictures[4];
    mpeg2_picture_t* m_picture;
    /*const*/ mpeg2_fbuf_t* m_fbuf[3];	/* 0: current fbuf, 1-2: prediction fbufs */

	mpeg2_fbuf_t m_fbuf_alloc[3];

    uint8_t* m_buf_start;
    uint8_t* m_buf_end;

    int16_t m_display_offset_x, m_display_offset_y;

    int m_copy_matrix;
    uint8_t m_intra_quantizer_matrix[64];
    uint8_t m_non_intra_quantizer_matrix[64];
};
