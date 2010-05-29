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

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "libmpeg2.h"
#include "../../../DSUtil/vd.h"

// decode

#define SEQ_EXT 2
#define SEQ_DISPLAY_EXT 4
#define QUANT_MATRIX_EXT 8
#define COPYRIGHT_EXT 0x10
#define PIC_DISPLAY_EXT 0x80
#define PIC_CODING_EXT 0x100

/* default intra quant matrix, in zig-zag order */
static const uint8_t default_intra_quantizer_matrix[64] = {
    8,
    16, 16,
    19, 16, 19,
    22, 22, 22, 22,
    22, 22, 26, 24, 26,
    27, 27, 27, 26, 26, 26,
    26, 27, 27, 27, 29, 29, 29,
    34, 34, 34, 29, 29, 29, 27, 27,
    29, 29, 32, 32, 34, 34, 37,
    38, 37, 35, 35, 34, 35,
    38, 38, 40, 40, 40,
    48, 48, 46, 46,
    56, 56, 58,
    69, 69,
    83
};

static uint8_t mpeg2_scan_norm_2[64] = {
    /* Zig-Zag scan pattern */
     0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static uint8_t mpeg2_scan_alt_2[64] = {
    /* Alternate scan pattern */
     0, 8,  16, 24,  1,  9,  2, 10, 17, 25, 32, 40, 48, 56, 57, 49,
    41, 33, 26, 18,  3, 11,  4, 12, 19, 27, 34, 42, 50, 58, 35, 43,
    51, 59, 20, 28,  5, 13,  6, 14, 21, 29, 36, 44, 52, 60, 37, 45,
    53, 61, 22, 30,  7, 15, 23, 31, 38, 46, 54, 62, 39, 47, 55, 63
};

// dummy
extern "C" uint8_t mpeg2_scan_norm[64];
extern "C" uint8_t mpeg2_scan_alt[64];
uint8_t mpeg2_scan_norm[64];
uint8_t mpeg2_scan_alt[64];

// idct (c)

#define W1 2841 /* 2048 * sqrt (2) * cos (1 * pi / 16) */
#define W2 2676 /* 2048 * sqrt (2) * cos (2 * pi / 16) */
#define W3 2408 /* 2048 * sqrt (2) * cos (3 * pi / 16) */
#define W5 1609 /* 2048 * sqrt (2) * cos (5 * pi / 16) */
#define W6 1108 /* 2048 * sqrt (2) * cos (6 * pi / 16) */
#define W7 565  /* 2048 * sqrt (2) * cos (7 * pi / 16) */

/*
 * In legal streams, the IDCT output should be between -384 and +384.
 * In corrupted streams, it is possible to force the IDCT output to go
 * to +-3826 - this is the worst case for a column IDCT where the
 * column inputs are 16-bit values.
 */
static uint8_t mpeg2_clip[3840 * 2 + 256];
#define CLIP(i) ((mpeg2_clip + 3840)[i])

#define BUTTERFLY(t0,t1,W0,W1,d0,d1)	\
{										\
    int tmp = W0 * (d0 + d1);			\
    t0 = tmp + (W1 - W0) * d1;			\
    t1 = tmp - (W1 + W0) * d0;			\
}

static void __inline idct_row(int16_t* block)
{
    int d0, d1, d2, d3;
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int t0, t1, t2, t3;

    /* shortcut */
    if(!(block[1] | ((int32_t *)block)[1] | ((int32_t *)block)[2] | ((int32_t *)block)[3]))
	{
		uint32_t tmp = (uint16_t) (block[0] << 3);
		tmp |= tmp << 16;
		((int32_t *)block)[0] = tmp;
		((int32_t *)block)[1] = tmp;
		((int32_t *)block)[2] = tmp;
		((int32_t *)block)[3] = tmp;
		return;
    }

    d0 = (block[0] << 11) + 128;
    d1 = block[1];
    d2 = block[2] << 11;
    d3 = block[3];
    t0 = d0 + d2;
    t1 = d0 - d2;
    BUTTERFLY(t2, t3, W6, W2, d3, d1);
    a0 = t0 + t2;
    a1 = t1 + t3;
    a2 = t1 - t3;
    a3 = t0 - t2;

    d0 = block[4];
    d1 = block[5];
    d2 = block[6];
    d3 = block[7];
    BUTTERFLY(t0, t1, W7, W1, d3, d0);
    BUTTERFLY(t2, t3, W3, W5, d1, d2);
    b0 = t0 + t2;
    b3 = t1 + t3;
    t0 -= t2;
    t1 -= t3;
    b1 = ((t0 + t1) * 181) >> 8;
    b2 = ((t0 - t1) * 181) >> 8;

    block[0] = (a0 + b0) >> 8;
    block[1] = (a1 + b1) >> 8;
    block[2] = (a2 + b2) >> 8;
    block[3] = (a3 + b3) >> 8;
    block[4] = (a3 - b3) >> 8;
    block[5] = (a2 - b2) >> 8;
    block[6] = (a1 - b1) >> 8;
    block[7] = (a0 - b0) >> 8;
}

static void __inline idct_col(int16_t* block)
{
    int d0, d1, d2, d3;
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int t0, t1, t2, t3;

    d0 = (block[8*0] << 11) + 65536;
    d1 = block[8*1];
    d2 = block[8*2] << 11;
    d3 = block[8*3];
    t0 = d0 + d2;
    t1 = d0 - d2;
    BUTTERFLY(t2, t3, W6, W2, d3, d1);
    a0 = t0 + t2;
    a1 = t1 + t3;
    a2 = t1 - t3;
    a3 = t0 - t2;

    d0 = block[8*4];
    d1 = block[8*5];
    d2 = block[8*6];
    d3 = block[8*7];
    BUTTERFLY(t0, t1, W7, W1, d3, d0);
    BUTTERFLY(t2, t3, W3, W5, d1, d2);
    b0 = t0 + t2;
    b3 = t1 + t3;
    t0 = (t0 - t2) >> 8;
    t1 = (t1 - t3) >> 8;
    b1 = (t0 + t1) * 181;
    b2 = (t0 - t1) * 181;

    block[8*0] = (a0 + b0) >> 17;
    block[8*1] = (a1 + b1) >> 17;
    block[8*2] = (a2 + b2) >> 17;
    block[8*3] = (a3 + b3) >> 17;
    block[8*4] = (a3 - b3) >> 17;
    block[8*5] = (a2 - b2) >> 17;
    block[8*6] = (a1 - b1) >> 17;
    block[8*7] = (a0 - b0) >> 17;
}

static void mpeg2_idct_copy_c(int16_t* block, uint8_t* dest, const int stride)
{
	for(int i = 0; i < 8; i++) idct_row(block + 8 * i);
	for(int i = 0; i < 8; i++) idct_col(block + i);
    for(int i = 0; i < 8; i++)
	{
		dest[0] = CLIP(block[0]);
		dest[1] = CLIP(block[1]);
		dest[2] = CLIP(block[2]);
		dest[3] = CLIP(block[3]);
		dest[4] = CLIP(block[4]);
		dest[5] = CLIP(block[5]);
		dest[6] = CLIP(block[6]);
		dest[7] = CLIP(block[7]);

		block[0] = 0; block[1] = 0; block[2] = 0; block[3] = 0;
		block[4] = 0; block[5] = 0; block[6] = 0; block[7] = 0;

		dest += stride;
		block += 8;
    }
}

static void mpeg2_idct_add_c(const int last, int16_t* block, uint8_t* dest, const int stride)
{
    if(last != 129 || (block[0] & 7) == 4)
	{
		for(int i = 0; i < 8; i++) idct_row(block + 8 * i);
		for(int i = 0; i < 8; i++) idct_col(block + i);
		for(int i = 0; i < 8; i++)
		{
			dest[0] = CLIP(block[0] + dest[0]);
			dest[1] = CLIP(block[1] + dest[1]);
			dest[2] = CLIP(block[2] + dest[2]);
			dest[3] = CLIP(block[3] + dest[3]);
			dest[4] = CLIP(block[4] + dest[4]);
			dest[5] = CLIP(block[5] + dest[5]);
			dest[6] = CLIP(block[6] + dest[6]);
			dest[7] = CLIP(block[7] + dest[7]);

			block[0] = 0;	block[1] = 0;	block[2] = 0;	block[3] = 0;
			block[4] = 0;	block[5] = 0;	block[6] = 0;	block[7] = 0;

			dest += stride;
			block += 8;
		}
    }
	else
	{
		int DC = (block[0] + 4) >> 3;
		block[0] = block[63] = 0;
		for(int i = 0; i < 8; i++)
		{
			dest[0] = CLIP(DC + dest[0]);
			dest[1] = CLIP(DC + dest[1]);
			dest[2] = CLIP(DC + dest[2]);
			dest[3] = CLIP(DC + dest[3]);
			dest[4] = CLIP(DC + dest[4]);
			dest[5] = CLIP(DC + dest[5]);
			dest[6] = CLIP(DC + dest[6]);
			dest[7] = CLIP(DC + dest[7]);
			dest += stride;
		}
    }
}

static void mpeg2_idct_init_c()
{
	for(int i = -3840; i < 3840 + 256; i++)
	{
		CLIP(i) = (i < 0) ? 0 : ((i > 255) ? 255 : i);
	}

	// only needed with idct_c

	for(int i = 0; i < 64; i++)
	{
		mpeg2_scan_norm_2[i] = ((mpeg2_scan_norm_2[i] & 0x36) >> 1) | ((mpeg2_scan_norm_2[i] & 0x09) << 2);
		mpeg2_scan_alt_2[i] = ((mpeg2_scan_alt_2[i] & 0x36) >> 1) | ((mpeg2_scan_alt_2[i] & 0x09) << 2);
	}
}

// mc (c)

#define avg2(a,b) ((a+b+1)>>1)
#define avg4(a,b,c,d) ((a+b+c+d+2)>>2)

#define predict_o(i) (ref[i])
#define predict_x(i) (avg2(ref[i], ref[i+1]))
#define predict_y(i) (avg2(ref[i], (ref+stride)[i]))
#define predict_xy(i) (avg4(ref[i], ref[i+1], (ref+stride)[i], (ref+stride)[i+1]))

#define put(predictor,i) dest[i] = predictor(i)
#define avg(predictor,i) dest[i] = avg2(predictor(i), dest[i])

/* mc function template */

#define MC_FUNC(op,xy)							\
static void MC_##op##_##xy##_16_c (uint8_t* dest, const uint8_t* ref, const int stride, int height)	\
{												\
    do {										\
	op (predict_##xy, 0);						\
	op (predict_##xy, 1);						\
	op (predict_##xy, 2);						\
	op (predict_##xy, 3);						\
	op (predict_##xy, 4);						\
	op (predict_##xy, 5);						\
	op (predict_##xy, 6);						\
	op (predict_##xy, 7);						\
	op (predict_##xy, 8);						\
	op (predict_##xy, 9);						\
	op (predict_##xy, 10);						\
	op (predict_##xy, 11);						\
	op (predict_##xy, 12);						\
	op (predict_##xy, 13);						\
	op (predict_##xy, 14);						\
	op (predict_##xy, 15);						\
	ref += stride;								\
	dest += stride;								\
    } while (--height);							\
}												\
static void MC_##op##_##xy##_8_c (uint8_t * dest, const uint8_t * ref, const int stride, int height)	\
{												\
    do {										\
	op (predict_##xy, 0);						\
	op (predict_##xy, 1);						\
	op (predict_##xy, 2);						\
	op (predict_##xy, 3);						\
	op (predict_##xy, 4);						\
	op (predict_##xy, 5);						\
	op (predict_##xy, 6);						\
	op (predict_##xy, 7);						\
	ref += stride;								\
	dest += stride;								\
    } while (--height);							\
}

/* definitions of the actual mc functions */

MC_FUNC(put,o)
MC_FUNC(avg,o)
MC_FUNC(put,x)
MC_FUNC(avg,x)
MC_FUNC(put,y)
MC_FUNC(avg,y)
MC_FUNC(put,xy)
MC_FUNC(avg,xy)

#define MPEG2_MC_EXTERN(x) mpeg2_mc_t mpeg2_mc_##x = {						\
    {MC_put_o_16_##x, MC_put_x_16_##x, MC_put_y_16_##x, MC_put_xy_16_##x,	\
     MC_put_o_8_##x,  MC_put_x_8_##x,  MC_put_y_8_##x,  MC_put_xy_8_##x},	\
    {MC_avg_o_16_##x, MC_avg_x_16_##x, MC_avg_y_16_##x, MC_avg_xy_16_##x,	\
     MC_avg_o_8_##x,  MC_avg_x_8_##x,  MC_avg_y_8_##x,  MC_avg_xy_8_##x}	\
};

MPEG2_MC_EXTERN(c)

// idct (mmx)

extern "C" void mpeg2_idct_copy_mmx(int16_t* block, uint8_t* dest, const int stride);
extern "C" void mpeg2_idct_add_mmx(const int last, int16_t* block, uint8_t* dest, const int stride);

static void mpeg2_idct_init_mmx()
{
    for(int i = 0; i < 64; i++)
	{
		mpeg2_scan_norm_2[i] = (mpeg2_scan_norm_2[i] & 0x38) | ((mpeg2_scan_norm_2[i] & 6) >> 1) | ((mpeg2_scan_norm_2[i] & 1) << 2);
		mpeg2_scan_alt_2[i] = (mpeg2_scan_alt_2[i] & 0x38) | ((mpeg2_scan_alt_2[i] & 6) >> 1) | ((mpeg2_scan_alt_2[i] & 1) << 2);
    }
}

// mc (mmx)

extern "C" mpeg2_mc_t mpeg2_mc_mmx;

// idct (sse2)

extern void mpeg2_idct_init_sse2();
extern void mpeg2_idct_copy_sse2(int16_t* block, uint8_t* dest, const int stride);
extern void mpeg2_idct_add_sse2(const int last, int16_t* block, uint8_t* dest, const int stride);

// mc (sse2)

extern mpeg2_mc_t mpeg2_mc_sse2;

// idct (c)

static void mpeg2_idct_init_c();
static void mpeg2_idct_copy_c(int16_t* block, uint8_t* dest, const int stride);
static void mpeg2_idct_add_c(const int last, int16_t* block, uint8_t* dest, const int stride);

 // mc (c)

static void MC_c(uint8_t* dest, const uint8_t* ref, const int stride, int height);

extern mpeg2_mc_t mpeg2_mc_c;

//

CMpeg2Dec::CMpeg2Dec()
{
    m_shift = 0;
    m_is_display_initialized = 0;
	m_action = NULL;
    m_state = STATE_BUFFER;
    m_ext_state = 0;

	m_chunk_buffer = m_chunk_start = m_chunk_ptr = NULL;
    m_code = 0;

    m_pts_current = m_pts_previous = 0;
    m_num_pts = m_bytes_since_pts = 0;

    m_first = 0;
    m_alloc_index = 0;
    m_first_decode_slice = m_nb_decode_slices = 0;

    memset(&m_new_sequence, 0, sizeof(m_new_sequence));
    memset(&m_sequence, 0, sizeof(m_sequence));
    memset(&m_gop, 0, sizeof(m_gop));
    memset(&m_pictures, 0, sizeof(m_pictures));
	m_picture = NULL;
    memset(&m_fbuf, 0, sizeof(m_fbuf));
    memset(&m_fbuf_alloc, 0, sizeof(m_fbuf_alloc));

    m_buf_start = m_buf_end = NULL;

    m_display_offset_x = m_display_offset_y = 0;

    m_copy_matrix = 0;
    memset(&m_intra_quantizer_matrix, 0, sizeof(m_intra_quantizer_matrix));
    memset(&m_non_intra_quantizer_matrix, 0, sizeof(m_non_intra_quantizer_matrix));

	//

	mpeg2_init();
}

CMpeg2Dec::~CMpeg2Dec()
{
	mpeg2_close();
}

void CMpeg2Dec::mpeg2_init()
{
	m_chunk_buffer = (uint8_t*)_aligned_malloc(BUFFER_SIZE + 4, 16);
	m_shift = 0xffffff00;
	m_code = 0xb4;
	m_action = &CMpeg2Dec::mpeg2_seek_sequence;
	m_sequence.width = (unsigned)-1;
}

void CMpeg2Dec::mpeg2_close()
{
	/* static uint8_t finalizer[] = {0,0,1,0xb4}; */
	/* mpeg2_decode_data (mpeg2dec, finalizer, finalizer+4); */

	mpeg2_header_state_init();
	_aligned_free(m_chunk_buffer);
}

//

int CMpeg2Dec::skip_chunk(int bytes)
{
	if(!bytes)
		return 0;

	int len = 0;

	uint8_t* current = m_buf_start;
	uint8_t* limit = current + bytes;

	while(current < limit)
	{
		if(m_shift == 0x00000100)
		{
			m_shift = 0xffffff00;
			len = ++current - m_buf_start;
			break;
		}

		m_shift = (m_shift | *current++) << 8;
	}

	m_buf_start = current;

	return len;
}

int CMpeg2Dec::copy_chunk(int bytes)
{
	if(!bytes)
		return 0;

	int len = 0;

	// this assembly gives us a nice speed up
	// 36 sec down to 32 sec decoding the ts.stream.tpr test file
	// (idtc, mc was set to null)
#ifndef _WIN64
	__asm
	{
		mov ebx, this
		mov esi, [ebx].m_buf_start
		mov edi, [ebx].m_chunk_ptr
		mov ecx, bytes
		mov edx, [ebx].m_shift

	copy_chunk_loop:

		cmp edx, 0x00000100
		jne copy_chunk_continue
		mov edx, 0xffffff00

		inc edi
		mov [ebx].m_chunk_ptr, edi

		inc esi
		mov eax, esi
		sub eax, [ebx].m_buf_start
		mov len, eax

		jmp copy_chunk_end

	copy_chunk_continue:

		movzx eax, byte ptr [esi]
		or edx, eax
		shl edx, 8
		mov byte ptr [edi], al
		inc esi
		inc edi
		dec	ecx
		jnz copy_chunk_loop

	copy_chunk_end:

		mov [ebx].m_buf_start, esi
		mov [ebx].m_shift, edx
	}
#else
	uint8_t* chunk_ptr = m_chunk_ptr;
	uint8_t* current = m_buf_start;
	uint8_t* limit = current + bytes;

	while(current < limit)
	{
		if(m_shift == 0x00000100)
		{
			m_shift = 0xffffff00;
			len = ++current - m_buf_start;
			m_chunk_ptr = ++chunk_ptr;
			break;
		}

		m_shift = (m_shift | (*chunk_ptr++ = *current++)) << 8;
	}

	m_buf_start = current;
#endif
	return len;
}

mpeg2_state_t CMpeg2Dec::seek_chunk()
{
	int size = m_buf_end - m_buf_start;

	if(int skipped = skip_chunk(size))
	{
		m_bytes_since_pts += skipped;
		m_code = m_buf_start[-1];
		return (mpeg2_state_t)-1;
	}

	m_bytes_since_pts += size;
	return STATE_BUFFER;
}

mpeg2_state_t CMpeg2Dec::seek_header()
{
	while(m_code != 0xb3 && (m_code != 0xb7 && m_code != 0xb8 && m_code || m_sequence.width == (unsigned)-1))
	{
		if(seek_chunk() == STATE_BUFFER)
			return STATE_BUFFER;
	}

	m_chunk_start = m_chunk_ptr = m_chunk_buffer;

	return m_code 
		? mpeg2_parse_header()
		: mpeg2_header_picture_start();
}

mpeg2_state_t CMpeg2Dec::seek_sequence()
{
	mpeg2_header_state_init();
	m_action = &CMpeg2Dec::seek_header;
	return seek_header();
}

//

void CMpeg2Dec::mpeg2_buffer(uint8_t* start, uint8_t* end)
{
	m_buf_start = start;
	m_buf_end = end;
}

int CMpeg2Dec::mpeg2_getpos()
{
	return m_buf_end - m_buf_start;
}

//

mpeg2_state_t CMpeg2Dec::mpeg2_parse()
{
	if(m_action)
	{
		mpeg2_state_t state = (this->*m_action)();
		if((int)state >= 0)
			return state;
	}

	while(1)
	{
		while((unsigned)(m_code - m_first_decode_slice) < m_nb_decode_slices)
		{
			int size_buffer = m_buf_end - m_buf_start;
			int size_chunk = (m_chunk_buffer + BUFFER_SIZE - m_chunk_ptr);
			int copied;

			if(size_buffer <= size_chunk)
			{
				copied = copy_chunk(size_buffer);
				if(!copied)
				{
					m_bytes_since_pts += size_buffer;
					m_chunk_ptr += size_buffer;
					return STATE_BUFFER;
				}
			}
			else
			{
				copied = copy_chunk(size_chunk);
				if(!copied)
				{
					/* filled the chunk buffer without finding a start code */
					m_bytes_since_pts += size_chunk;
					m_action = &CMpeg2Dec::seek_chunk;
					return STATE_INVALID;
				}
			}

			m_bytes_since_pts += copied;

			m_decoder.mpeg2_slice(m_code, m_chunk_start);
			m_code = m_buf_start[-1];
			m_chunk_ptr = m_chunk_start;
		}

		if((unsigned)(m_code - 1) >= 0xb0 - 1)
			break;
		if(seek_chunk() == STATE_BUFFER)
			return STATE_BUFFER;
	}

	switch(m_code)
	{
	case 0x00:
		m_action = &CMpeg2Dec::mpeg2_header_picture_start;
		return m_state;
	case 0xb7:
		m_action = &CMpeg2Dec::mpeg2_header_end;
		break;
	case 0xb3:
	case 0xb8:
		m_action = &CMpeg2Dec::mpeg2_parse_header;
		break;
	case 0xbe:
		m_action = &CMpeg2Dec::seek_chunk;
		return STATE_PADDING;
	default:
		m_action = &CMpeg2Dec::seek_chunk;
		return STATE_INVALID;
	}

	if(m_state != STATE_SLICE)
		m_state = STATE_INVALID;

	return m_state;
}


void CMpeg2Dec::mpeg2_skip(int skip)
{
	m_first_decode_slice = 1;
	m_nb_decode_slices = skip ? 0 : (0xb0 - 1);
}

void CMpeg2Dec::mpeg2_slice_region(int start, int end)
{
	start = (start < 1) ? 1 : (start > 0xb0) ? 0xb0 : start;
	end = (end < start) ? start : (end > 0xb0) ? 0xb0 : end;
	m_first_decode_slice = start;
	m_nb_decode_slices = end - start;
}

void CMpeg2Dec::mpeg2_pts(uint32_t pts)
{
	m_pts_previous = m_pts_current;
	m_pts_current = pts;
	m_num_pts++;
	m_bytes_since_pts = 0;
}

//

/* decode.c */

#define RECEIVED(code,state) (((state) << 8) + (code))

mpeg2_state_t CMpeg2Dec::mpeg2_seek_sequence()
{
	mpeg2_header_state_init();
	m_action = &CMpeg2Dec::seek_header;
	return seek_header();
}

mpeg2_state_t CMpeg2Dec::mpeg2_parse_header()
{
	static int (CMpeg2Dec::* process_header[]) () =
	{
		&CMpeg2Dec::mpeg2_header_picture, 
		&CMpeg2Dec::mpeg2_header_extension, 
		&CMpeg2Dec::mpeg2_header_user_data,
		&CMpeg2Dec::mpeg2_header_sequence, 
		NULL, NULL, NULL, NULL, 
		&CMpeg2Dec::mpeg2_header_gop
	};

	m_action = &CMpeg2Dec::mpeg2_parse_header;

	while(1)
	{
		int size_buffer = m_buf_end - m_buf_start;
		int size_chunk = (m_chunk_buffer + BUFFER_SIZE - m_chunk_ptr);
		int copied;
		if(size_buffer <= size_chunk)
		{
			copied = copy_chunk(size_buffer);
			if(!copied)
			{
				m_bytes_since_pts += size_buffer;
				m_chunk_ptr += size_buffer;
				return STATE_BUFFER;
			}
		}
		else
		{
			copied = copy_chunk(size_chunk);
			if(!copied)
			{
				/* filled the chunk buffer without finding a start code */
				m_bytes_since_pts += size_chunk;
				m_code = 0xb4;
				m_action = &CMpeg2Dec::seek_header;
				return STATE_INVALID;
			}
		}
		m_bytes_since_pts += copied;

		if((this->*(process_header[m_code & 0x0b]))())
		{
			m_code = m_buf_start[-1];
			m_action = &CMpeg2Dec::seek_header;
			return STATE_INVALID;
		}

		m_code = m_buf_start[-1];

		switch(RECEIVED(m_code, m_state))
		{
		/* state transition after a sequence header */
		case RECEIVED(0x00, STATE_SEQUENCE):
			m_action = &CMpeg2Dec::mpeg2_header_picture_start;
		case RECEIVED(0xb8, STATE_SEQUENCE):
			mpeg2_header_sequence_finalize();
			break;

		/* other legal state transitions */
		case RECEIVED (0x00, STATE_GOP):
			m_action = &CMpeg2Dec::mpeg2_header_picture_start;
			break;
		case RECEIVED (0x01, STATE_PICTURE):	
		case RECEIVED (0x01, STATE_PICTURE_2ND):
			mpeg2_header_matrix_finalize();
			m_action = &CMpeg2Dec::mpeg2_header_slice_start;
			break;

		/* legal headers within a given state */
		case RECEIVED (0xb2, STATE_SEQUENCE):
		case RECEIVED (0xb2, STATE_GOP):
		case RECEIVED (0xb2, STATE_PICTURE):
		case RECEIVED (0xb2, STATE_PICTURE_2ND):
		case RECEIVED (0xb5, STATE_SEQUENCE):
		case RECEIVED (0xb5, STATE_PICTURE):
		case RECEIVED (0xb5, STATE_PICTURE_2ND):
			m_chunk_ptr = m_chunk_start;
			continue;

		default:
			m_action = &CMpeg2Dec::seek_header;
			return STATE_INVALID;
		}

		m_chunk_start = m_chunk_ptr = m_chunk_buffer;
		return m_state;
	}
}

/* header.c */

void CMpeg2Dec::mpeg2_header_state_init()
{
    if(m_sequence.width != (unsigned)-1)
	{
		m_sequence.width = (unsigned)-1;
		for(int i = 0; i < m_alloc_index; i++)
			_aligned_free(m_fbuf_alloc[i].buf[0]);
    }

	m_decoder.m_scan = mpeg2_scan_norm_2;
	m_picture = m_pictures;
	m_fbuf[0] = &m_fbuf_alloc[0];
	m_fbuf[1] = &m_fbuf_alloc[1];
	m_fbuf[2] = &m_fbuf_alloc[2];
	m_first = true;
	m_alloc_index = 0;
	m_first_decode_slice = 1;
	m_nb_decode_slices = 0xb0 - 1;
}

int CMpeg2Dec::mpeg2_header_sequence()
{
    uint8_t* buffer = m_chunk_start;
    mpeg2_sequence_t* sequence = &m_new_sequence;
    static unsigned int frame_period[9] = {0, 1126125, 1125000, 1080000, 900900, 900000, 540000, 450450, 450000};

    if((buffer[6] & 0x20) != 0x20)	/* missing marker_bit */
		return 1;

    int i = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
	sequence->display_width = sequence->picture_width = i >> 12;
    if(!sequence->display_width)
		return 1;
	sequence->display_height = sequence->picture_height = i & 0xfff;
    if(!sequence->display_height)
		return 1;

    sequence->width = (sequence->picture_width + 15) & ~15;
    sequence->height = (sequence->picture_height + 15) & ~15;
    sequence->chroma_width = sequence->width >> 1;
    sequence->chroma_height = sequence->height >> 1;
    sequence->flags = (SEQ_FLAG_PROGRESSIVE_SEQUENCE | SEQ_VIDEO_FORMAT_UNSPECIFIED);
    sequence->pixel_width = buffer[3] >> 4;	/* aspect ratio */
    sequence->frame_period = 0;
    if((buffer[3] & 15) < 9) sequence->frame_period = frame_period[buffer[3] & 15];
    sequence->byte_rate = (buffer[4]<<10) | (buffer[5]<<2) | (buffer[6]>>6);
    sequence->vbv_buffer_size = ((buffer[6]<<16)|(buffer[7]<<8))&0x1ff800;
    if(buffer[7] & 4) sequence->flags |= SEQ_FLAG_CONSTRAINED_PARAMETERS;

    m_copy_matrix = 3;
    if(buffer[7] & 2)
	{
		for(i = 0; i < 64; i++) 
			m_intra_quantizer_matrix[mpeg2_scan_norm_2[i]] = (buffer[i+7] << 7) | (buffer[i+8] >> 1);
		buffer += 64;
    }
	else
	{
		for (i = 0; i < 64; i++)
			m_intra_quantizer_matrix[mpeg2_scan_norm_2[i]] = default_intra_quantizer_matrix[i];
	}

    if(buffer[7] & 1)
	{
		for(i = 0; i < 64; i++)
            m_non_intra_quantizer_matrix[mpeg2_scan_norm_2[i]] = buffer[i+8];
	}
    else
	{
		for(i = 0; i < 64; i++)
			m_non_intra_quantizer_matrix[i] = 16;
	}

    sequence->profile_level_id = 0x80;
    sequence->colour_primaries = 0;
    sequence->transfer_characteristics = 0;
    sequence->matrix_coefficients = 0;

    m_ext_state = SEQ_EXT;
    m_state = STATE_SEQUENCE;
    m_display_offset_x = m_display_offset_y = 0;

    m_info.Reset();
    m_info.m_gop = NULL;

    return 0;
}

int CMpeg2Dec::mpeg2_header_gop()
{
    uint8_t* buffer = m_chunk_start;
	m_info.Reset();
    if(!(buffer[1] & 8)) 
		return 1;
    m_info.m_gop = &m_gop;
    m_gop.hours = (buffer[0] >> 2) & 31;
    m_gop.minutes = ((buffer[0] << 4) | (buffer[1] >> 4)) & 63;
    m_gop.seconds = ((buffer[1] << 3) | (buffer[2] >> 5)) & 63;
    m_gop.pictures = ((buffer[2] << 1) | (buffer[3] >> 7)) & 63;
    m_gop.flags = (buffer[0] >> 7) | ((buffer[3] >> 4) & 6);
    m_state = STATE_GOP;
    return 0;
}

mpeg2_state_t CMpeg2Dec::mpeg2_header_picture_start()
{
	{
		mpeg2_picture_t* picture;

		if(m_state != STATE_SLICE_1ST)
		{
			m_state = STATE_PICTURE;
			picture = m_pictures;
			if((m_decoder.m_coding_type != PIC_FLAG_CODING_TYPE_B) ^ (m_picture >= m_pictures + 2))
				picture += 2;
		}
		else
		{
			m_state = STATE_PICTURE_2ND;
			picture = m_picture + 1;	/* second field picture */
		}

		m_picture = picture;
	}

    m_picture->flags = 0;

    if(m_num_pts)
	{
		if(m_bytes_since_pts >= 4)
		{
			m_num_pts = 0;
			m_picture->pts = m_pts_current;
			m_picture->flags = PIC_FLAG_PTS;
		}
		else if(m_num_pts > 1)
		{
		    m_num_pts = 1;
			m_picture->pts = m_pts_previous;
			m_picture->flags = PIC_FLAG_PTS;
		}
    }

    m_picture->display_offset[0].x = 
	m_picture->display_offset[1].x =
	m_picture->display_offset[2].x = m_display_offset_x;
    m_picture->display_offset[0].y = 
	m_picture->display_offset[1].y =
	m_picture->display_offset[2].y = m_display_offset_y;

    return mpeg2_parse_header();
}

int CMpeg2Dec::mpeg2_header_picture()
{
	uint8_t* buffer = m_chunk_start;
	mpeg2_picture_t* picture = m_picture;
	int type;
	int low_delay;

	type = (buffer [1] >> 3) & 7;
	low_delay = m_sequence.flags & SEQ_FLAG_LOW_DELAY;

	if(m_state == STATE_PICTURE)
	{
		mpeg2_picture_t* other;

		m_decoder.m_second_field = 0;
		other = m_pictures;
		if(other == picture)
			other += 2;
		if(m_decoder.m_coding_type != PIC_FLAG_CODING_TYPE_B)
		{
			m_fbuf[2] = m_fbuf[1];
			m_fbuf[1] = m_fbuf[0];
		}
		m_fbuf[0] = NULL;
		m_info.Reset();
		m_info.m_current_picture = picture;
		m_info.m_display_picture = picture;
		if(type != PIC_FLAG_CODING_TYPE_B)
		{
			if(!low_delay)
			{
				if(m_first) {
					m_info.m_display_picture = NULL;
					m_first = false;
				}
				else
				{
					m_info.m_display_picture = other;
					if(other->nb_fields == 1)
						m_info.m_display_picture_2nd = other + 1;
					m_info.m_display_fbuf = m_fbuf[1];
				}
			}
			
			if(!low_delay + !NULL/*m_convert_start*/)
				m_info.m_discard_fbuf = m_fbuf[!low_delay + !NULL/*m_convert_start*/];
		}

		while(m_alloc_index < 3)
		{
			mpeg2_fbuf_t* fbuf = &m_fbuf_alloc[m_alloc_index++];
			fbuf->id = NULL;

			int size = m_decoder.m_width * m_decoder.m_height;
			fbuf->buf[0] = (uint8_t*)_aligned_malloc(6 * size >> 2, 16);
			fbuf->buf[1] = fbuf->buf[0] + size;
			fbuf->buf[2] = fbuf->buf[1] + (size >> 2);
			memset(fbuf->buf[0], 0x10, size);
			memset(fbuf->buf[1], 0x80, size >> 2);
			memset(fbuf->buf[2], 0x80, size >> 2);
		}
		mpeg2_set_fbuf(type);
	}
	else
	{
		m_decoder.m_second_field = 1;
		m_info.m_current_picture_2nd = picture;
		m_info.m_user_data = NULL; m_info.m_user_data_len = 0;
		if(low_delay || type == PIC_FLAG_CODING_TYPE_B)
			m_info.m_display_picture_2nd = picture;
	}
	m_ext_state = PIC_CODING_EXT;

	picture->temporal_reference = (buffer[0] << 2) | (buffer[1] >> 6);

	m_decoder.m_coding_type = type;
	picture->flags |= type;

	if(type == PIC_FLAG_CODING_TYPE_P || type == PIC_FLAG_CODING_TYPE_B)
	{
		/* forward_f_code and backward_f_code - used in mpeg1 only */
		m_decoder.m_f_motion.f_code[1] = (buffer[3] >> 2) & 1;
		m_decoder.m_f_motion.f_code[0] = (((buffer[3] << 1) | (buffer[4] >> 7)) & 7) - 1;
		m_decoder.m_b_motion.f_code[1] = (buffer[4] >> 6) & 1;
		m_decoder.m_b_motion.f_code[0] = ((buffer[4] >> 3) & 7) - 1;
	}

	/* XXXXXX decode extra_information_picture as well */

	picture->nb_fields = 2;

	m_decoder.m_intra_dc_precision = 0;
	m_decoder.m_frame_pred_frame_dct = 1;
	m_decoder.m_q_scale_type = 0;
	m_decoder.m_concealment_motion_vectors = 0;
	m_decoder.m_scan = mpeg2_scan_norm_2;
	m_decoder.m_picture_structure = FRAME_PICTURE;
	m_copy_matrix = 0;

	return 0;
}

int CMpeg2Dec::mpeg2_header_extension()
{
    static int (CMpeg2Dec::* parser[]) () =
	{
		NULL, 
		&CMpeg2Dec::sequence_ext, 
		&CMpeg2Dec::sequence_display_ext, 
		&CMpeg2Dec::quant_matrix_ext,
		&CMpeg2Dec::copyright_ext, 
		NULL, NULL, 
		&CMpeg2Dec::picture_display_ext, 
		&CMpeg2Dec::picture_coding_ext
    };

    int ext, ext_bit;

    ext = m_chunk_start[0] >> 4;
    ext_bit = 1 << ext;

    if(!(m_ext_state & ext_bit)) /* ignore illegal extensions */
		return 0;
    m_ext_state &= ~ext_bit;
    return (this->*parser[ext])();
}

int CMpeg2Dec::mpeg2_header_user_data()
{
    if(!m_info.m_user_data_len) m_info.m_user_data = m_chunk_start;
    else m_info.m_user_data_len += 3;

	m_info.m_user_data_len += (m_chunk_ptr - 4 - m_chunk_start);
    m_chunk_start = m_chunk_ptr - 1;

    return 0;
}

void CMpeg2Dec::mpeg2_header_matrix_finalize()
{
    if(m_copy_matrix & 1)
		memcpy(m_decoder.m_intra_quantizer_matrix, m_intra_quantizer_matrix, 64);

    if(m_copy_matrix & 2)
		memcpy(m_decoder.m_non_intra_quantizer_matrix, m_non_intra_quantizer_matrix, 64);
}

void mpeg2_sequence_t::finalize()
{
    int w, h;

    byte_rate *= 50;

    if(flags & SEQ_FLAG_MPEG2)
	{
		switch(pixel_width)
		{
		case 1:		/* square pixels */
			pixel_width = pixel_height = 1;	return;
		case 2:		/* 4:3 aspect ratio */
			w = 4; h = 3; break;
		case 3:		/* 16:9 aspect ratio */
			w = 16; h = 9; break;
		case 4:		/* 2.21:1 aspect ratio */
			w = 221; h = 100; break;
		default:	/* illegal */
			pixel_width = pixel_height = 0; return;
		}

		w *= display_height;
		h *= display_width;
    }
	else 
	{
		if(byte_rate == 50 * 0x3ffff) 
			byte_rate = 0;        /* mpeg-1 VBR */ 

		switch(pixel_width)
		{
		case 0:	case 15:	/* illegal */
			pixel_width = pixel_height = 0; return;
		case 1:	/* square pixels */
			pixel_width = pixel_height = 1; return;
		case 3:	/* 720x576 16:9 */
			pixel_width = 64; pixel_height = 45; return;
		case 6:	/* 720x480 16:9 */
			pixel_width = 32; pixel_height = 27; return;
		case 12:	/* 720*480 4:3 */
			pixel_width = 8; pixel_height = 9; return;
		default:
			h = 88 * pixel_width + 1171;
			w = 2000;
		}
    }

    pixel_width = w;
    pixel_height = h;

	/* find greatest common divisor */
    while(w) {int tmp = w; w = h % tmp; h = tmp;}

	pixel_width /= h;
    pixel_height /= h;
}

void CMpeg2Dec::mpeg2_header_sequence_finalize()
{
    mpeg2_sequence_t* sequence = &m_new_sequence;

    sequence->finalize();

    mpeg2_header_matrix_finalize();

    m_decoder.m_mpeg1 = !(sequence->flags & SEQ_FLAG_MPEG2);
    m_decoder.m_width = sequence->width;
    m_decoder.m_height = sequence->height;
    m_decoder.m_vertical_position_extension = (sequence->picture_height > 2800);

    /*
     * according to 6.1.1.6, repeat sequence headers should be
     * identical to the original. However some DVDs dont respect that
     * and have different bitrates in the repeat sequence headers. So
     * we'll ignore that in the comparison and still consider these as
     * repeat sequence headers.
     */

	// EDIT: some dvds will work if we allow the last three fields to vary (which aren't needed anyway)
	// EDIT2: vbv_buffer_size can be ignored as well, not used by libmpeg2

	m_sequence.byte_rate = sequence->byte_rate;
	m_sequence.vbv_buffer_size = sequence->vbv_buffer_size;

    if(!memcmp(&m_sequence, sequence, FIELD_OFFSET(mpeg2_sequence_t, colour_primaries) /*sizeof(mpeg2_sequence_t)*/)) 
	{
		m_state = STATE_SEQUENCE_REPEATED;
	}
    else if(m_sequence.width != (unsigned)-1)
	{
		m_action = &CMpeg2Dec::mpeg2_seek_sequence;
		m_state = STATE_INVALID;	/* XXXX STATE_INVALID_END ? */
		return;
    }

    m_sequence = *sequence;
    m_info.m_sequence = &m_sequence;
}

mpeg2_state_t CMpeg2Dec::mpeg2_header_slice_start()
{
	m_info.m_user_data = NULL; 
	m_info.m_user_data_len = 0;
	m_state = (m_picture->nb_fields > 1 || m_state == STATE_PICTURE_2ND) ? STATE_SLICE : STATE_SLICE_1ST;

	if(!m_nb_decode_slices)
	{
		m_picture->flags |= PIC_FLAG_SKIP;
	}
	else
	{
		int b_type = m_decoder.m_coding_type == B_TYPE;
		m_decoder.mpeg2_init_fbuf(m_fbuf[0]->buf, m_fbuf[b_type + 1]->buf, m_fbuf[b_type]->buf);
	}

	m_action = NULL;

	return (mpeg2_state_t)-1;
}

mpeg2_state_t CMpeg2Dec::mpeg2_header_end()
{
	mpeg2_picture_t* picture = m_pictures;

	int b_type = m_decoder.m_coding_type == B_TYPE;

	if((m_picture >= picture + 2) ^ b_type)
		picture = m_pictures + 2;

	m_state = STATE_END;
	m_info.Reset();
	
	if(!(m_sequence.flags & SEQ_FLAG_LOW_DELAY))
	{
		m_info.m_display_picture = picture;
		if(picture->nb_fields == 1)
			m_info.m_display_picture_2nd = picture + 1;
		m_info.m_display_fbuf = m_fbuf[b_type];
		m_info.m_discard_fbuf = m_fbuf[b_type + 1];
	}
	else
	{
		m_info.m_discard_fbuf = m_fbuf[b_type];
	}

	m_action = &CMpeg2Dec::mpeg2_seek_sequence;
	m_first = true;

	return STATE_END;
}

void CMpeg2Dec::mpeg2_set_fbuf(int coding_type)
{
    for(int i = 0; i < 3; i++)
	{
		if(m_fbuf[1] != &m_fbuf_alloc[i] && m_fbuf[2] != &m_fbuf_alloc[i])
		{
			m_fbuf[0] = &m_fbuf_alloc[i];
			m_info.m_current_fbuf = m_fbuf[0];
			if(coding_type == B_TYPE || (m_sequence.flags & SEQ_FLAG_LOW_DELAY))
			{
				if(coding_type == B_TYPE)
					m_info.m_discard_fbuf = m_fbuf[0];
				m_info.m_display_fbuf = m_fbuf[0];
			}
			break;
		}
	}
}

//

int CMpeg2Dec::sequence_ext()
{
	uint8_t* buffer = m_chunk_start;
	mpeg2_sequence_t* sequence = &m_new_sequence;

	if(!(buffer[3]&1))
		return 1;

	sequence->profile_level_id = (buffer[0] << 4) | (buffer[1] >> 4);

	sequence->display_width = 
		sequence->picture_width += ((buffer[1] << 13) | (buffer[2] << 5)) & 0x3000;
	sequence->display_height = 
		sequence->picture_height += (buffer[2] << 7) & 0x3000;

	sequence->width = (sequence->picture_width + 15) & ~15;
	sequence->height = (sequence->picture_height + 15) & ~15;

	sequence->flags |= SEQ_FLAG_MPEG2;

	if(!(buffer[1] & 8))
	{
		sequence->flags &= ~SEQ_FLAG_PROGRESSIVE_SEQUENCE;
		sequence->width = (sequence->width + 31) & ~31;
		sequence->height = (sequence->height + 31) & ~31;
	}

	if(buffer[5] & 0x80)
	{
		sequence->flags |= SEQ_FLAG_LOW_DELAY;
	}

	sequence->chroma_width = sequence->width;
	sequence->chroma_height = sequence->height;

	switch(buffer[1] & 6)
	{
	case 0:	/* invalid */
		return 1;
	case 2:	/* 4:2:0 */
		sequence->chroma_height >>= 1;
	case 4:	/* 4:2:2 */
		sequence->chroma_width >>= 1;
	}

	sequence->byte_rate += ((buffer[2]<<25) | (buffer[3]<<17)) & 0x3ffc0000;

	sequence->vbv_buffer_size |= buffer[4] << 21;

	sequence->frame_period = 
		sequence->frame_period * ((buffer[5]&31)+1) / (((buffer[5]>>2)&3)+1);

	m_ext_state = SEQ_DISPLAY_EXT;

	return 0;
}

int CMpeg2Dec::sequence_display_ext()
{
    uint8_t* buffer = m_chunk_start;
    mpeg2_sequence_t* sequence = &m_new_sequence;

    uint32_t flags = (sequence->flags & ~SEQ_MASK_VIDEO_FORMAT) | ((buffer[0]<<4) & SEQ_MASK_VIDEO_FORMAT);
    if(buffer[0] & 1)
	{
		flags |= SEQ_FLAG_COLOUR_DESCRIPTION;
		sequence->colour_primaries = buffer[1];
		sequence->transfer_characteristics = buffer[2];
		sequence->matrix_coefficients = buffer[3];
		buffer += 3;
    }

    if(!(buffer[2] & 2))	/* missing marker_bit */
		return 1;

	// ???
//	sequence->flags = flags;

	sequence->display_width = (buffer[1] << 6) | (buffer[2] >> 2);
    sequence->display_height = ((buffer[2]& 1 ) << 13) | (buffer[3] << 5) | (buffer[4] >> 3);

    return 0;
}

int CMpeg2Dec::quant_matrix_ext()
{
    uint8_t* buffer = m_chunk_start;

    if(buffer[0] & 8)
	{
		for(int i = 0; i < 64; i++)
			m_intra_quantizer_matrix[mpeg2_scan_norm_2[i]] = (buffer[i] << 5) | (buffer[i+1] >> 3);
		m_copy_matrix |= 1;
		buffer += 64;
    }

    if(buffer[0] & 4)
	{
		for(int i = 0; i < 64; i++)
			m_non_intra_quantizer_matrix[mpeg2_scan_norm_2[i]] = (buffer[i] << 6) | (buffer[i+1] >> 2);
		m_copy_matrix |= 2;
    }

    return 0;
}

int CMpeg2Dec::copyright_ext()
{
    return 0;
}

int CMpeg2Dec::picture_display_ext()
{
    uint8_t* buffer = m_chunk_start;
    mpeg2_picture_t* picture = m_picture;

    int nb_pos = picture->nb_fields;
    if(m_sequence.flags & SEQ_FLAG_PROGRESSIVE_SEQUENCE)
		nb_pos >>= 1;

	int i = 0;

    for(; i < nb_pos; i++)
	{
		int x = ((buffer[4*i] << 24) | (buffer[4*i+1] << 16) |
			(buffer[4*i+2] << 8) | buffer[4*i+3]) >> (11-2*i);
		int y = ((buffer[4*i+2] << 24) | (buffer[4*i+3] << 16) |
			(buffer[4*i+4] << 8) | buffer[4*i+5]) >> (10-2*i);
		if(!(x&y&1))
			return 1;

		picture->display_offset[i].x = m_display_offset_x = x >> 1;
		picture->display_offset[i].y = m_display_offset_y = y >> 1;
	}

    for(; i < 3; i++)
	{
		picture->display_offset[i].x = m_display_offset_x;
		picture->display_offset[i].y = m_display_offset_y;
    }

    return 0;
}

int CMpeg2Dec::picture_coding_ext()
{
    uint8_t* buffer = m_chunk_start;
    mpeg2_picture_t* picture = m_picture;

    /* pre subtract 1 for use later in compute_motion_vector */
    m_decoder.m_f_motion.f_code[0] = (buffer[0] & 15) - 1;
    m_decoder.m_f_motion.f_code[1] = (buffer[1] >> 4) - 1;
    m_decoder.m_b_motion.f_code[0] = (buffer[1] & 15) - 1;
    m_decoder.m_b_motion.f_code[1] = (buffer[2] >> 4) - 1;

    m_decoder.m_intra_dc_precision = (buffer[2] >> 2) & 3;
    m_decoder.m_picture_structure = buffer[2] & 3;
    switch(m_decoder.m_picture_structure)
	{
    case TOP_FIELD:
		picture->flags |= PIC_FLAG_TOP_FIELD_FIRST;
    case BOTTOM_FIELD:
		picture->nb_fields = 1;
		break;
    case FRAME_PICTURE:
		if(!(m_sequence.flags & SEQ_FLAG_PROGRESSIVE_SEQUENCE))
		{
			picture->nb_fields = (buffer[3] & 2) ? 3 : 2;
			picture->flags |= (buffer[3] & 128) ? PIC_FLAG_TOP_FIELD_FIRST : 0;
		}
		else
		{
			picture->nb_fields = (buffer[3]&2) ? ((buffer[3]&128) ? 6 : 4) : 2;
		}
		break;
    default:
		return 1;
    }

    m_decoder.m_top_field_first = buffer[3] >> 7;
    m_decoder.m_frame_pred_frame_dct = (buffer[3] >> 6) & 1;
    m_decoder.m_concealment_motion_vectors = (buffer[3] >> 5) & 1;
    m_decoder.m_q_scale_type = (buffer[3] >> 4) & 1;
    m_decoder.m_intra_vlc_format = (buffer[3] >> 3) & 1;
    m_decoder.m_scan = (buffer[3] & 4) ? mpeg2_scan_alt_2 : mpeg2_scan_norm_2;

	if(buffer[3] & 2)
		picture->flags |= PIC_FLAG_REPEAT_FIRST_FIELD;

    picture->flags |= (buffer[4] & 0x80) ? PIC_FLAG_PROGRESSIVE_FRAME : 0;
    if(buffer[4] & 0x40)
		picture->flags |= (((buffer[4]<<26) | (buffer[5]<<18) | (buffer[6]<<10)) & PIC_MASK_COMPOSITE_DISPLAY) | PIC_FLAG_COMPOSITE_DISPLAY;

    m_ext_state = PIC_DISPLAY_EXT | COPYRIGHT_EXT | QUANT_MATRIX_EXT;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

// vlc

#define GETWORD(bit_buf,shift,bit_ptr)				\
do {								\
    bit_buf |= ((bit_ptr[0] << 8) | bit_ptr[1]) << (shift);	\
    bit_ptr += 2;						\
} while (0)
/*
void bitstream_init(const uint8_t * start)
{
    m_bitstream_buf =
	(start[0] << 24) | (start[1] << 16) | (start[2] << 8) | start[3];
    m_bitstream_ptr = start + 4;
    m_bitstream_bits = -16;
}
*/
/* make sure that there are at least 16 valid bits in bit_buf */
#define NEEDBITS		\
do {						\
    if (bits > 0) {			\
	GETWORD (bit_buf, bits, bit_ptr);	\
	bits -= 16;				\
    }						\
} while (0)

/* remove num valid bits from bit_buf */
#define DUMPBITS(num)	\
do {					\
    bit_buf <<= (num);			\
    bits += (num);			\
} while (0)

/* take num bits from the high part of bit_buf and zero extend them */
#define UBITS(bit_buf,num) (((uint32_t)(bit_buf)) >> (32 - (num)))

/* take num bits from the high part of bit_buf and sign extend them */
#define SBITS(bit_buf,num) (((int32_t)(bit_buf)) >> (32 - (num)))

typedef struct {
    uint8_t modes;
    uint8_t len;
} MBtab;

typedef struct {
    uint8_t delta;
    uint8_t len;
} MVtab;

typedef struct {
    int8_t dmv;
    uint8_t len;
} DMVtab;

typedef struct {
    uint8_t cbp;
    uint8_t len;
} CBPtab;

typedef struct {
    uint8_t size;
    uint8_t len;
} DCtab;

typedef struct {
    uint8_t run;
    uint8_t level;
    uint8_t len;
} DCTtab;

typedef struct {
    uint8_t mba;
    uint8_t len;
} MBAtab;


#define INTRA MACROBLOCK_INTRA
#define QUANT MACROBLOCK_QUANT

static const MBtab MB_I [] = {
    {INTRA|QUANT, 2}, {INTRA, 1}
};

#define MC MACROBLOCK_MOTION_FORWARD
#define CODED MACROBLOCK_PATTERN

static const MBtab MB_P [] = {
    {INTRA|QUANT, 6}, {CODED|QUANT, 5}, {MC|CODED|QUANT, 5}, {INTRA,    5},
    {MC,          3}, {MC,          3}, {MC,             3}, {MC,       3},
    {CODED,       2}, {CODED,       2}, {CODED,          2}, {CODED,    2},
    {CODED,       2}, {CODED,       2}, {CODED,          2}, {CODED,    2},
    {MC|CODED,    1}, {MC|CODED,    1}, {MC|CODED,       1}, {MC|CODED, 1},
    {MC|CODED,    1}, {MC|CODED,    1}, {MC|CODED,       1}, {MC|CODED, 1},
    {MC|CODED,    1}, {MC|CODED,    1}, {MC|CODED,       1}, {MC|CODED, 1},
    {MC|CODED,    1}, {MC|CODED,    1}, {MC|CODED,       1}, {MC|CODED, 1}
};

#define FWD MACROBLOCK_MOTION_FORWARD
#define BWD MACROBLOCK_MOTION_BACKWARD
#define INTER MACROBLOCK_MOTION_FORWARD|MACROBLOCK_MOTION_BACKWARD

static const MBtab MB_B [] = {
    {0,                 0}, {INTRA|QUANT,       6},
    {BWD|CODED|QUANT,   6}, {FWD|CODED|QUANT,   6},
    {INTER|CODED|QUANT, 5}, {INTER|CODED|QUANT, 5},
					{INTRA,       5}, {INTRA,       5},
    {FWD,         4}, {FWD,         4}, {FWD,         4}, {FWD,         4},
    {FWD|CODED,   4}, {FWD|CODED,   4}, {FWD|CODED,   4}, {FWD|CODED,   4},
    {BWD,         3}, {BWD,         3}, {BWD,         3}, {BWD,         3},
    {BWD,         3}, {BWD,         3}, {BWD,         3}, {BWD,         3},
    {BWD|CODED,   3}, {BWD|CODED,   3}, {BWD|CODED,   3}, {BWD|CODED,   3},
    {BWD|CODED,   3}, {BWD|CODED,   3}, {BWD|CODED,   3}, {BWD|CODED,   3},
    {INTER,       2}, {INTER,       2}, {INTER,       2}, {INTER,       2},
    {INTER,       2}, {INTER,       2}, {INTER,       2}, {INTER,       2},
    {INTER,       2}, {INTER,       2}, {INTER,       2}, {INTER,       2},
    {INTER,       2}, {INTER,       2}, {INTER,       2}, {INTER,       2},
    {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2},
    {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2},
    {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2},
    {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2}, {INTER|CODED, 2}
};

#undef INTRA
#undef QUANT
#undef MC
#undef CODED
#undef FWD
#undef BWD
#undef INTER


static const MVtab MV_4 [] = {
    { 3, 6}, { 2, 4}, { 1, 3}, { 1, 3}, { 0, 2}, { 0, 2}, { 0, 2}, { 0, 2}
};

static const MVtab MV_10 [] = {
    { 0,10}, { 0,10}, { 0,10}, { 0,10}, { 0,10}, { 0,10}, { 0,10}, { 0,10},
    { 0,10}, { 0,10}, { 0,10}, { 0,10}, {15,10}, {14,10}, {13,10}, {12,10},
    {11,10}, {10,10}, { 9, 9}, { 9, 9}, { 8, 9}, { 8, 9}, { 7, 9}, { 7, 9},
    { 6, 7}, { 6, 7}, { 6, 7}, { 6, 7}, { 6, 7}, { 6, 7}, { 6, 7}, { 6, 7},
    { 5, 7}, { 5, 7}, { 5, 7}, { 5, 7}, { 5, 7}, { 5, 7}, { 5, 7}, { 5, 7},
    { 4, 7}, { 4, 7}, { 4, 7}, { 4, 7}, { 4, 7}, { 4, 7}, { 4, 7}, { 4, 7}
};


static const DMVtab DMV_2 [] = {
    { 0, 1}, { 0, 1}, { 1, 2}, {-1, 2}
};


static const CBPtab CBP_7 [] = {
    {0x22, 7}, {0x12, 7}, {0x0a, 7}, {0x06, 7},
    {0x21, 7}, {0x11, 7}, {0x09, 7}, {0x05, 7},
    {0x3f, 6}, {0x3f, 6}, {0x03, 6}, {0x03, 6},
    {0x24, 6}, {0x24, 6}, {0x18, 6}, {0x18, 6},
    {0x3e, 5}, {0x3e, 5}, {0x3e, 5}, {0x3e, 5},
    {0x02, 5}, {0x02, 5}, {0x02, 5}, {0x02, 5},
    {0x3d, 5}, {0x3d, 5}, {0x3d, 5}, {0x3d, 5},
    {0x01, 5}, {0x01, 5}, {0x01, 5}, {0x01, 5},
    {0x38, 5}, {0x38, 5}, {0x38, 5}, {0x38, 5},
    {0x34, 5}, {0x34, 5}, {0x34, 5}, {0x34, 5},
    {0x2c, 5}, {0x2c, 5}, {0x2c, 5}, {0x2c, 5},
    {0x1c, 5}, {0x1c, 5}, {0x1c, 5}, {0x1c, 5},
    {0x28, 5}, {0x28, 5}, {0x28, 5}, {0x28, 5},
    {0x14, 5}, {0x14, 5}, {0x14, 5}, {0x14, 5},
    {0x30, 5}, {0x30, 5}, {0x30, 5}, {0x30, 5},
    {0x0c, 5}, {0x0c, 5}, {0x0c, 5}, {0x0c, 5},
    {0x20, 4}, {0x20, 4}, {0x20, 4}, {0x20, 4},
    {0x20, 4}, {0x20, 4}, {0x20, 4}, {0x20, 4},
    {0x10, 4}, {0x10, 4}, {0x10, 4}, {0x10, 4},
    {0x10, 4}, {0x10, 4}, {0x10, 4}, {0x10, 4},
    {0x08, 4}, {0x08, 4}, {0x08, 4}, {0x08, 4},
    {0x08, 4}, {0x08, 4}, {0x08, 4}, {0x08, 4},
    {0x04, 4}, {0x04, 4}, {0x04, 4}, {0x04, 4},
    {0x04, 4}, {0x04, 4}, {0x04, 4}, {0x04, 4},
    {0x3c, 3}, {0x3c, 3}, {0x3c, 3}, {0x3c, 3},
    {0x3c, 3}, {0x3c, 3}, {0x3c, 3}, {0x3c, 3},
    {0x3c, 3}, {0x3c, 3}, {0x3c, 3}, {0x3c, 3},
    {0x3c, 3}, {0x3c, 3}, {0x3c, 3}, {0x3c, 3}
};

static const CBPtab CBP_9 [] = {
    {0,    0}, {0x00, 9}, {0x27, 9}, {0x1b, 9},
    {0x3b, 9}, {0x37, 9}, {0x2f, 9}, {0x1f, 9},
    {0x3a, 8}, {0x3a, 8}, {0x36, 8}, {0x36, 8},
    {0x2e, 8}, {0x2e, 8}, {0x1e, 8}, {0x1e, 8},
    {0x39, 8}, {0x39, 8}, {0x35, 8}, {0x35, 8},
    {0x2d, 8}, {0x2d, 8}, {0x1d, 8}, {0x1d, 8},
    {0x26, 8}, {0x26, 8}, {0x1a, 8}, {0x1a, 8},
    {0x25, 8}, {0x25, 8}, {0x19, 8}, {0x19, 8},
    {0x2b, 8}, {0x2b, 8}, {0x17, 8}, {0x17, 8},
    {0x33, 8}, {0x33, 8}, {0x0f, 8}, {0x0f, 8},
    {0x2a, 8}, {0x2a, 8}, {0x16, 8}, {0x16, 8},
    {0x32, 8}, {0x32, 8}, {0x0e, 8}, {0x0e, 8},
    {0x29, 8}, {0x29, 8}, {0x15, 8}, {0x15, 8},
    {0x31, 8}, {0x31, 8}, {0x0d, 8}, {0x0d, 8},
    {0x23, 8}, {0x23, 8}, {0x13, 8}, {0x13, 8},
    {0x0b, 8}, {0x0b, 8}, {0x07, 8}, {0x07, 8}
};


static const DCtab DC_lum_5 [] = {
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
    {4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}
};

static const DCtab DC_chrom_5 [] = {
    {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}
};

static const DCtab DC_long [] = {
    {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, { 6, 5}, { 6, 5},
    {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, { 6, 5}, { 6, 5},
    {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, { 7, 6}, { 7, 6},
    {8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10, 9}, {11, 9}
};


static const DCTtab DCT_16 [] = {
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {  2,18, 0}, {  2,17, 0}, {  2,16, 0}, {  2,15, 0},
    {  7, 3, 0}, { 17, 2, 0}, { 16, 2, 0}, { 15, 2, 0},
    { 14, 2, 0}, { 13, 2, 0}, { 12, 2, 0}, { 32, 1, 0},
    { 31, 1, 0}, { 30, 1, 0}, { 29, 1, 0}, { 28, 1, 0}
};

static const DCTtab DCT_15 [] = {
    {  1,40,15}, {  1,39,15}, {  1,38,15}, {  1,37,15},
    {  1,36,15}, {  1,35,15}, {  1,34,15}, {  1,33,15},
    {  1,32,15}, {  2,14,15}, {  2,13,15}, {  2,12,15},
    {  2,11,15}, {  2,10,15}, {  2, 9,15}, {  2, 8,15},
    {  1,31,14}, {  1,31,14}, {  1,30,14}, {  1,30,14},
    {  1,29,14}, {  1,29,14}, {  1,28,14}, {  1,28,14},
    {  1,27,14}, {  1,27,14}, {  1,26,14}, {  1,26,14},
    {  1,25,14}, {  1,25,14}, {  1,24,14}, {  1,24,14},
    {  1,23,14}, {  1,23,14}, {  1,22,14}, {  1,22,14},
    {  1,21,14}, {  1,21,14}, {  1,20,14}, {  1,20,14},
    {  1,19,14}, {  1,19,14}, {  1,18,14}, {  1,18,14},
    {  1,17,14}, {  1,17,14}, {  1,16,14}, {  1,16,14}
};

static const DCTtab DCT_13 [] = {
    { 11, 2,13}, { 10, 2,13}, {  6, 3,13}, {  4, 4,13},
    {  3, 5,13}, {  2, 7,13}, {  2, 6,13}, {  1,15,13},
    {  1,14,13}, {  1,13,13}, {  1,12,13}, { 27, 1,13},
    { 26, 1,13}, { 25, 1,13}, { 24, 1,13}, { 23, 1,13},
    {  1,11,12}, {  1,11,12}, {  9, 2,12}, {  9, 2,12},
    {  5, 3,12}, {  5, 3,12}, {  1,10,12}, {  1,10,12},
    {  3, 4,12}, {  3, 4,12}, {  8, 2,12}, {  8, 2,12},
    { 22, 1,12}, { 22, 1,12}, { 21, 1,12}, { 21, 1,12},
    {  1, 9,12}, {  1, 9,12}, { 20, 1,12}, { 20, 1,12},
    { 19, 1,12}, { 19, 1,12}, {  2, 5,12}, {  2, 5,12},
    {  4, 3,12}, {  4, 3,12}, {  1, 8,12}, {  1, 8,12},
    {  7, 2,12}, {  7, 2,12}, { 18, 1,12}, { 18, 1,12}
};

static const DCTtab DCT_B14_10 [] = {
    { 17, 1,10}, {  6, 2,10}, {  1, 7,10}, {  3, 3,10},
    {  2, 4,10}, { 16, 1,10}, { 15, 1,10}, {  5, 2,10}
};

static const DCTtab DCT_B14_8 [] = {
    { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6},
    {  3, 2, 7}, {  3, 2, 7}, { 10, 1, 7}, { 10, 1, 7},
    {  1, 4, 7}, {  1, 4, 7}, {  9, 1, 7}, {  9, 1, 7},
    {  8, 1, 6}, {  8, 1, 6}, {  8, 1, 6}, {  8, 1, 6},
    {  7, 1, 6}, {  7, 1, 6}, {  7, 1, 6}, {  7, 1, 6},
    {  2, 2, 6}, {  2, 2, 6}, {  2, 2, 6}, {  2, 2, 6},
    {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6},
    { 14, 1, 8}, {  1, 6, 8}, { 13, 1, 8}, { 12, 1, 8},
    {  4, 2, 8}, {  2, 3, 8}, {  1, 5, 8}, { 11, 1, 8}
};

static const DCTtab DCT_B14AC_5 [] = {
		 {  1, 3, 5}, {  5, 1, 5}, {  4, 1, 5},
    {  1, 2, 4}, {  1, 2, 4}, {  3, 1, 4}, {  3, 1, 4},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {129, 0, 2}, {129, 0, 2}, {129, 0, 2}, {129, 0, 2},
    {129, 0, 2}, {129, 0, 2}, {129, 0, 2}, {129, 0, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}
};

static const DCTtab DCT_B14DC_5 [] = {
		 {  1, 3, 5}, {  5, 1, 5}, {  4, 1, 5},
    {  1, 2, 4}, {  1, 2, 4}, {  3, 1, 4}, {  3, 1, 4},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}
};

static const DCTtab DCT_B15_10 [] = {
    {  6, 2, 9}, {  6, 2, 9}, { 15, 1, 9}, { 15, 1, 9},
    {  3, 4,10}, { 17, 1,10}, { 16, 1, 9}, { 16, 1, 9}
};

static const DCTtab DCT_B15_8 [] = {
    { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6},
    {  8, 1, 7}, {  8, 1, 7}, {  9, 1, 7}, {  9, 1, 7},
    {  7, 1, 7}, {  7, 1, 7}, {  3, 2, 7}, {  3, 2, 7},
    {  1, 7, 6}, {  1, 7, 6}, {  1, 7, 6}, {  1, 7, 6},
    {  1, 6, 6}, {  1, 6, 6}, {  1, 6, 6}, {  1, 6, 6},
    {  5, 1, 6}, {  5, 1, 6}, {  5, 1, 6}, {  5, 1, 6},
    {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6},
    {  2, 5, 8}, { 12, 1, 8}, {  1,11, 8}, {  1,10, 8},
    { 14, 1, 8}, { 13, 1, 8}, {  4, 2, 8}, {  2, 4, 8},
    {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5},
    {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5},
    {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5},
    {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5},
    {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5},
    {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5},
    {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5},
    {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5},
    {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5},
    { 10, 1, 7}, { 10, 1, 7}, {  2, 3, 7}, {  2, 3, 7},
    { 11, 1, 7}, { 11, 1, 7}, {  1, 8, 7}, {  1, 8, 7},
    {  1, 9, 7}, {  1, 9, 7}, {  1,12, 8}, {  1,13, 8},
    {  3, 3, 8}, {  5, 2, 8}, {  1,14, 8}, {  1,15, 8}
};


static const MBAtab MBA_5 [] = {
		    {6, 5}, {5, 5}, {4, 4}, {4, 4}, {3, 4}, {3, 4},
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3},
    {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1},
    {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}
};

static const MBAtab MBA_11 [] = {
    {32, 11}, {31, 11}, {30, 11}, {29, 11},
    {28, 11}, {27, 11}, {26, 11}, {25, 11},
    {24, 11}, {23, 11}, {22, 11}, {21, 11},
    {20, 10}, {20, 10}, {19, 10}, {19, 10},
    {18, 10}, {18, 10}, {17, 10}, {17, 10},
    {16, 10}, {16, 10}, {15, 10}, {15, 10},
    {14,  8}, {14,  8}, {14,  8}, {14,  8},
    {14,  8}, {14,  8}, {14,  8}, {14,  8},
    {13,  8}, {13,  8}, {13,  8}, {13,  8},
    {13,  8}, {13,  8}, {13,  8}, {13,  8},
    {12,  8}, {12,  8}, {12,  8}, {12,  8},
    {12,  8}, {12,  8}, {12,  8}, {12,  8},
    {11,  8}, {11,  8}, {11,  8}, {11,  8},
    {11,  8}, {11,  8}, {11,  8}, {11,  8},
    {10,  8}, {10,  8}, {10,  8}, {10,  8},
    {10,  8}, {10,  8}, {10,  8}, {10,  8},
    { 9,  8}, { 9,  8}, { 9,  8}, { 9,  8},
    { 9,  8}, { 9,  8}, { 9,  8}, { 9,  8},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 7,  7}, { 7,  7}, { 7,  7}, { 7,  7},
    { 7,  7}, { 7,  7}, { 7,  7}, { 7,  7},
    { 7,  7}, { 7,  7}, { 7,  7}, { 7,  7},
    { 7,  7}, { 7,  7}, { 7,  7}, { 7,  7}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

static int non_linear_quantizer_scale [] = {
     0,  1,  2,  3,  4,  5,   6,   7,
     8, 10, 12, 14, 16, 18,  20,  22,
    24, 28, 32, 36, 40, 44,  48,  52,
    56, 64, 72, 80, 88, 96, 104, 112
};

bool CMpeg2Decoder::m_idct_initialized = false;

CMpeg2Decoder::CMpeg2Decoder()
{
	memset(&m_b_motion, 0, sizeof(m_b_motion));
	memset(&m_f_motion, 0, sizeof(m_f_motion));

	m_DCTblock = (int16_t*)_aligned_malloc(64*sizeof(int16_t), 16);
	memset(m_DCTblock, 0, 64*sizeof(int16_t));

    m_bitstream_buf = 0;
    m_bitstream_bits = 0;
    m_bitstream_ptr = NULL;
    
	memset(&m_dest, 0, sizeof(m_dest));
	memset(&m_picture_dest, 0, sizeof(m_picture_dest));

    m_offset = m_stride = m_uv_stride = 0;
    m_limit_x = m_limit_y_16 = m_limit_y_8 = m_limit_y = 0;

	memset(&m_dc_dct_pred, 0, sizeof(m_dc_dct_pred));

    m_quantizer_scale = m_dmv_offset = 0;
    m_v_offset = 0;

	memset(&m_intra_quantizer_matrix, 0, sizeof(m_intra_quantizer_matrix));
	memset(&m_non_intra_quantizer_matrix, 0, sizeof(m_non_intra_quantizer_matrix));

    m_width = m_height = 0;
    m_vertical_position_extension = 0;

    m_coding_type = 0;

    m_intra_dc_precision = 0;
    m_picture_structure = 0;
	m_frame_pred_frame_dct = 0;
    m_concealment_motion_vectors = 0;
    m_q_scale_type = 0;
    m_intra_vlc_format = 0;
    m_top_field_first = 0;

	m_scan = NULL;

    m_second_field = 0;

	m_mpeg1 = 0;

	if(g_cpuid.m_flags&CCpuID::sse2)
	{
		m_idct_init = mpeg2_idct_init_sse2;
		m_idct_copy = mpeg2_idct_copy_sse2;
		m_idct_add = mpeg2_idct_add_sse2;
		m_mc = &mpeg2_mc_sse2;
	}
#ifndef _WIN64
	else if(g_cpuid.m_flags&CCpuID::mmx)
	{
		m_idct_init = mpeg2_idct_init_mmx;
		m_idct_copy = mpeg2_idct_copy_mmx;
		m_idct_add = mpeg2_idct_add_mmx;
		m_mc = &mpeg2_mc_mmx;
	}
	else
#endif
	{
		m_idct_init = mpeg2_idct_init_c;
		m_idct_copy = mpeg2_idct_copy_c;
		m_idct_add = mpeg2_idct_add_c;
		m_mc = &mpeg2_mc_c;
	}
	if(!m_idct_initialized)
	{
		m_idct_init();
		m_idct_initialized = true;
	}
}

CMpeg2Decoder::~CMpeg2Decoder()
{
	if(m_DCTblock) _aligned_free(m_DCTblock);
}

#define bit_buf (m_bitstream_buf)
#define bits (m_bitstream_bits)
#define bit_ptr (m_bitstream_ptr)
	
int CMpeg2Decoder::get_macroblock_modes()
{
	int macroblock_modes;
    const MBtab* tab;

    switch(m_coding_type)
	{
	case P_TYPE:
		tab = MB_P + UBITS(bit_buf, 5);
		DUMPBITS(tab->len);
		macroblock_modes = tab->modes;

		if(m_picture_structure != FRAME_PICTURE)
		{
			if(macroblock_modes & MACROBLOCK_MOTION_FORWARD)
			{
				macroblock_modes |= UBITS(bit_buf, 2) * MOTION_TYPE_BASE;
				DUMPBITS(2);
			}

			return macroblock_modes;
		}
		else if(m_frame_pred_frame_dct)
		{
			if(macroblock_modes & MACROBLOCK_MOTION_FORWARD)
				macroblock_modes |= MC_FRAME;

			return macroblock_modes;
		}

		if(macroblock_modes & MACROBLOCK_MOTION_FORWARD)
		{
			macroblock_modes |= UBITS(bit_buf, 2) * MOTION_TYPE_BASE;
			DUMPBITS(2);
		}

		if(macroblock_modes & (MACROBLOCK_INTRA|MACROBLOCK_PATTERN))
		{
			macroblock_modes |= UBITS(bit_buf, 1) * DCT_TYPE_INTERLACED;
			DUMPBITS(1);
		}

		return macroblock_modes;

	case B_TYPE:
		tab = MB_B + UBITS(bit_buf, 6);
		DUMPBITS(tab->len);
		macroblock_modes = tab->modes;

		if(m_picture_structure != FRAME_PICTURE)
		{
			if(!(macroblock_modes & MACROBLOCK_INTRA))
			{
				macroblock_modes |= UBITS(bit_buf, 2) * MOTION_TYPE_BASE;
				DUMPBITS(2);
			}

			return macroblock_modes;
		}
		else if(m_frame_pred_frame_dct)
		{
			// if(!(macroblock_modes & MACROBLOCK_INTRA))
				macroblock_modes |= MC_FRAME;

			return macroblock_modes;
		}
/*
		if(macroblock_modes & MACROBLOCK_INTRA)
			goto intra;

		macroblock_modes |= UBITS(bit_buf, 2) * MOTION_TYPE_BASE;
		DUMPBITS(2);

		if(macroblock_modes & (MACROBLOCK_INTRA|MACROBLOCK_PATTERN))
		{
intra:
			macroblock_modes |= UBITS(bit_buf, 1) * DCT_TYPE_INTERLACED;
			DUMPBITS(1);
		}
*/
		if(!(macroblock_modes & MACROBLOCK_INTRA))
		{
			macroblock_modes |= UBITS(bit_buf, 2) * MOTION_TYPE_BASE;
			DUMPBITS(2);
		}

		if(macroblock_modes & (MACROBLOCK_INTRA|MACROBLOCK_PATTERN))
		{
			macroblock_modes |= UBITS(bit_buf, 1) * DCT_TYPE_INTERLACED;
			DUMPBITS(1);
		}

		return macroblock_modes;

    case I_TYPE:
		tab = MB_I + UBITS(bit_buf, 1);
		DUMPBITS(tab->len);
		macroblock_modes = tab->modes;

		if(!m_frame_pred_frame_dct && m_picture_structure == FRAME_PICTURE)
		{
			macroblock_modes |= UBITS(bit_buf, 1) * DCT_TYPE_INTERLACED;
			DUMPBITS(1);
		}

		return macroblock_modes;

	case D_TYPE:

		DUMPBITS(1);
		return MACROBLOCK_INTRA;
	}
    
	return 0;
}

int CMpeg2Decoder::get_quantizer_scale()
{
    int quantizer_scale_code = UBITS(bit_buf, 5);
    DUMPBITS(5);

    return m_q_scale_type 
		? non_linear_quantizer_scale[quantizer_scale_code]
		: (quantizer_scale_code << 1);
}

int CMpeg2Decoder::get_motion_delta(const int f_code)
{
    int delta;
    int sign;
    const MVtab* tab;

    if(bit_buf & 0x80000000)
	{
		DUMPBITS(1);
		return 0;
    }
	else if(bit_buf >= 0x0c000000)
	{
		tab = MV_4 + UBITS(bit_buf, 4);
		delta = (tab->delta << f_code) + 1;
		bits += tab->len + f_code + 1;
		bit_buf <<= tab->len;

		sign = SBITS(bit_buf, 1);
		bit_buf <<= 1;

		if(f_code)
		{
			delta += UBITS(bit_buf, f_code);
		}

		bit_buf <<= f_code;

		return (delta ^ sign) - sign;
    }
	else
	{
		tab = MV_10 + UBITS(bit_buf, 10);
		delta = (tab->delta << f_code) + 1;
		bits += tab->len + 1;
		bit_buf <<= tab->len;

		sign = SBITS(bit_buf, 1);
		bit_buf <<= 1;

		if(f_code)
		{
		    NEEDBITS;
			delta += UBITS(bit_buf, f_code);
			DUMPBITS(f_code);
		}

		return (delta ^ sign) - sign;
    }
}

int CMpeg2Decoder::bound_motion_vector(const int vector, const int f_code)
{
    return ((int32_t)vector << (27 - f_code)) >> (27 - f_code);
}

int CMpeg2Decoder::get_dmv()
{
    const DMVtab* tab = DMV_2 + UBITS(bit_buf, 2);
    DUMPBITS(tab->len);
    return tab->dmv;
}

int CMpeg2Decoder::get_coded_block_pattern()
{
    const CBPtab* tab;

    NEEDBITS;

    if(bit_buf >= 0x20000000)
	{
		tab = CBP_7 + (UBITS(bit_buf, 7) - 16);
		DUMPBITS(tab->len);
		return tab->cbp;
    }
	else
	{
		tab = CBP_9 + UBITS(bit_buf, 9);
		DUMPBITS(tab->len);
		return tab->cbp;
    }
}

int CMpeg2Decoder::get_luma_dc_dct_diff()
{
    const DCtab* tab;
    int size;
    int dc_diff;

    if(bit_buf < 0xf8000000)
	{
		tab = DC_lum_5 + UBITS(bit_buf, 5);
		size = tab->size;
		if(size)
		{
			bits += tab->len + size;
			bit_buf <<= tab->len;
			dc_diff = UBITS(bit_buf, size) - UBITS(SBITS(~bit_buf, 1), size);
			bit_buf <<= size;
			return dc_diff;
		}
		else
		{
			DUMPBITS(3);
			return 0;
		}
	}
	else
	{
		tab = DC_long + (UBITS(bit_buf, 9) - 0x1e0);
		size = tab->size;
		DUMPBITS(tab->len);
		NEEDBITS;
		dc_diff = UBITS(bit_buf, size) - UBITS(SBITS(~bit_buf, 1), size);
		DUMPBITS(size);
		return dc_diff;
	}
}

int CMpeg2Decoder::get_chroma_dc_dct_diff()
{
    const DCtab* tab;
    int size;
    int dc_diff;

    if(bit_buf < 0xf8000000)
	{
		tab = DC_chrom_5 + UBITS(bit_buf, 5);
		size = tab->size;

		if(size)
		{
			bits += tab->len + size;
			bit_buf <<= tab->len;
			dc_diff = UBITS(bit_buf, size) - UBITS(SBITS(~bit_buf, 1), size);
			bit_buf <<= size;
			return dc_diff;
		}
		else
		{
			DUMPBITS(2);
			return 0;
		}
	}
	else
	{
		tab = DC_long + (UBITS(bit_buf, 10) - 0x3e0);
		size = tab->size;
		DUMPBITS(tab->len + 1);
		NEEDBITS;
		dc_diff = UBITS(bit_buf, size) - UBITS(SBITS(~bit_buf, 1), size);
		DUMPBITS(size);
		return dc_diff;
	}
}

#undef bit_buf
#undef bits
#undef bit_ptr

#define SATURATE(val)					\
do {							\
    if((uint32_t)(val + 2048) > 4095)	\
	val = SBITS(val, 1) ^ 2047;			\
} while (0)

void CMpeg2Decoder::get_intra_block_B14()
{
    int i, j;
    int val;
    const uint8_t* scan = m_scan;
    const uint8_t* quant_matrix = m_intra_quantizer_matrix;
    int quantizer_scale = m_quantizer_scale;
    int mismatch;
    const DCTtab* tab;
    uint32_t bit_buf;
    int bits;
    const uint8_t* bit_ptr;
    int16_t* dest;

    dest = m_DCTblock;
    i = 0;
    mismatch = ~dest[0];

    bit_buf = m_bitstream_buf;
    bits = m_bitstream_bits;
    bit_ptr = m_bitstream_ptr;

    NEEDBITS;

    while(1)
	{
		if(bit_buf >= 0x28000000)
		{
			tab = DCT_B14AC_5 + (UBITS(bit_buf, 5) - 5);

			i += tab->run;
			if(i >= 64)
				break;	/* end of block */

normal_code:
			j = scan[i];
			bit_buf <<= tab->len;
			bits += tab->len + 1;
			val = (tab->level * quantizer_scale * quant_matrix[j]) >> 4;

			// if(bitstream_get (1)) val = -val;
				val = (val ^ SBITS(bit_buf, 1)) - SBITS(bit_buf, 1);

			SATURATE(val);
			dest[j] = val;
			mismatch ^= val;

			bit_buf <<= 1;
			NEEDBITS;

			continue;
		}
		else if(bit_buf >= 0x04000000)
		{
			tab = DCT_B14_8 + (UBITS(bit_buf, 8) - 4);

			i += tab->run;
			if(i < 64)
				goto normal_code;

			/* escape code */

			i += UBITS(bit_buf << 6, 6) - 64;
			if(i >= 64)
				break;	/* illegal, check needed to avoid buffer overflow */

			j = scan[i];

			DUMPBITS(12);
			NEEDBITS;
			val = (SBITS(bit_buf, 12) * quantizer_scale * quant_matrix[j]) / 16;

			SATURATE(val);
			dest[j] = val;
			mismatch ^= val;

			DUMPBITS(12);
			NEEDBITS;

			continue;
		}
		else if(bit_buf >= 0x02000000)
		{
			tab = DCT_B14_10 + (UBITS(bit_buf, 10) - 8);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00800000)
		{
			tab = DCT_13 + (UBITS(bit_buf, 13) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00200000)
		{
			tab = DCT_15 + (UBITS(bit_buf, 15) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else
		{
			tab = DCT_16 + UBITS(bit_buf, 16);
			bit_buf <<= 16;
			GETWORD (bit_buf, bits + 16, bit_ptr);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}

		break;	/* illegal, check needed to avoid buffer overflow */
	}

	dest[63] ^= mismatch & 1;
	DUMPBITS(2);	/* dump end of block code */
	m_bitstream_buf = bit_buf;
	m_bitstream_bits = bits;
	m_bitstream_ptr = bit_ptr;
}

void CMpeg2Decoder::get_intra_block_B15()
{
    int i, j;
    int val;
    const uint8_t* scan = m_scan;
    const uint8_t* quant_matrix = m_intra_quantizer_matrix;
    int quantizer_scale = m_quantizer_scale;
    int mismatch;
    const DCTtab* tab;
    uint32_t bit_buf;
    int bits;
    const uint8_t* bit_ptr;
    int16_t* dest;

    dest = m_DCTblock;
    i = 0;
    mismatch = ~dest[0];

    bit_buf = m_bitstream_buf;
    bits = m_bitstream_bits;
    bit_ptr = m_bitstream_ptr;

    NEEDBITS;

	while(1)
	{
		if(bit_buf >= 0x04000000)
		{
			tab = DCT_B15_8 + (UBITS(bit_buf, 8) - 4);

			i += tab->run;
			if(i < 64)
			{
normal_code:
				j = scan[i];
				bit_buf <<= tab->len;
				bits += tab->len + 1;
				val = (tab->level * quantizer_scale * quant_matrix[j]) >> 4;

				// if(bitstream_get (1)) val = -val;
					val = (val ^ SBITS(bit_buf, 1)) - SBITS(bit_buf, 1);

				SATURATE(val);
				dest[j] = val;
				mismatch ^= val;

				bit_buf <<= 1;
				NEEDBITS;

				continue;
			}
			else
			{
				/* end of block. I commented out this code because if we */
				/* dont exit here we will still exit at the later test :) */

				/* if(i >= 128) break;	*/	/* end of block */

				/* escape code */

				i += UBITS(bit_buf << 6, 6) - 64;
				if(i >= 64)
					break;	/* illegal, check against buffer overflow */

				j = scan[i];

				DUMPBITS(12);
				NEEDBITS;
				val = (SBITS(bit_buf, 12) *
					quantizer_scale * quant_matrix[j]) / 16;

				SATURATE(val);
				dest[j] = val;
				mismatch ^= val;

				DUMPBITS(12);
				NEEDBITS;

				continue;
			}
		}
		else if(bit_buf >= 0x02000000)
		{
			tab = DCT_B15_10 + (UBITS(bit_buf, 10) - 8);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00800000)
		{
			tab = DCT_13 + (UBITS(bit_buf, 13) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00200000)
		{
			tab = DCT_15 + (UBITS(bit_buf, 15) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else
		{
			tab = DCT_16 + UBITS(bit_buf, 16);
			bit_buf <<= 16;
			GETWORD(bit_buf, bits + 16, bit_ptr);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}

		break;	/* illegal, check needed to avoid buffer overflow */
	}

	dest[63] ^= mismatch & 1;
	DUMPBITS(4);	/* dump end of block code */
	m_bitstream_buf = bit_buf;
	m_bitstream_bits = bits;
	m_bitstream_ptr = bit_ptr;
}

int CMpeg2Decoder::get_non_intra_block()
{
    int i, j;
    int val;
    const uint8_t* scan = m_scan;
    const uint8_t* quant_matrix = m_non_intra_quantizer_matrix;
    int quantizer_scale = m_quantizer_scale;
    int mismatch;
    const DCTtab* tab;
    uint32_t bit_buf;
    int bits;
    const uint8_t* bit_ptr;
    int16_t* dest;

    i = -1;
    mismatch = 1;
    dest = m_DCTblock;

    bit_buf = m_bitstream_buf;
    bits = m_bitstream_bits;
    bit_ptr = m_bitstream_ptr;

    NEEDBITS;
    if(bit_buf >= 0x28000000)
	{
		tab = DCT_B14DC_5 + (UBITS(bit_buf, 5) - 5);
		goto entry_1;
    }
	else
	{
		goto entry_2;
	}

    while(1)
	{
		if(bit_buf >= 0x28000000)
		{
			tab = DCT_B14AC_5 + (UBITS(bit_buf, 5) - 5);
entry_1:
			i += tab->run;
			if(i >= 64)
				break;	/* end of block */
normal_code:
			j = scan[i];
			bit_buf <<= tab->len;
			bits += tab->len + 1;
			val = ((2*tab->level+1) * quantizer_scale * quant_matrix[j]) >> 5;

			/* if(bitstream_get (1)) val = -val; */
			val = (val ^ SBITS(bit_buf, 1)) - SBITS(bit_buf, 1);

			SATURATE(val);
			dest[j] = val;
			mismatch ^= val;

			bit_buf <<= 1;
			NEEDBITS;

			continue;
		}

entry_2:
		if(bit_buf >= 0x04000000)
		{
			tab = DCT_B14_8 + (UBITS(bit_buf, 8) - 4);

			i += tab->run;
			if(i < 64)
				goto normal_code;

			/* escape code */

			i += UBITS(bit_buf << 6, 6) - 64;
			if(i >= 64)
				break;	/* illegal, check needed to avoid buffer overflow */

			j = scan[i];

			DUMPBITS(12);
			NEEDBITS;
			val = 2 * (SBITS(bit_buf, 12) + SBITS(bit_buf, 1)) + 1;
			val = (val * quantizer_scale * quant_matrix[j]) / 32;

			SATURATE(val);
			dest[j] = val;
			mismatch ^= val;

			DUMPBITS(12);
			NEEDBITS;

			continue;
		}
		else if(bit_buf >= 0x02000000)
		{
			tab = DCT_B14_10 + (UBITS(bit_buf, 10) - 8);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00800000)
		{
			tab = DCT_13 + (UBITS(bit_buf, 13) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00200000)
		{
			tab = DCT_15 + (UBITS(bit_buf, 15) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else
		{
			tab = DCT_16 + UBITS(bit_buf, 16);
			bit_buf <<= 16;
			GETWORD (bit_buf, bits + 16, bit_ptr);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}

		break;	/* illegal, check needed to avoid buffer overflow */
	}

	dest[63] ^= mismatch & 1;
	DUMPBITS(2);	/* dump end of block code */
	m_bitstream_buf = bit_buf;
	m_bitstream_bits = bits;
	m_bitstream_ptr = bit_ptr;

	return i;
}

void CMpeg2Decoder::get_mpeg1_intra_block()
{
    int i, j;
    int val;
    const uint8_t* scan = m_scan;
    const uint8_t* quant_matrix = m_intra_quantizer_matrix;
    int quantizer_scale = m_quantizer_scale;
    const DCTtab* tab;
    uint32_t bit_buf;
    int bits;
    const uint8_t* bit_ptr;
    int16_t* dest;

    i = 0;
    dest = m_DCTblock;

    bit_buf = m_bitstream_buf;
    bits = m_bitstream_bits;
    bit_ptr = m_bitstream_ptr;

    NEEDBITS;

    while(1)
	{
		if(bit_buf >= 0x28000000)
		{
			tab = DCT_B14AC_5 + (UBITS(bit_buf, 5) - 5);

			i += tab->run;
			if(i >= 64)
				break;	/* end of block */
normal_code:
			j = scan[i];
			bit_buf <<= tab->len;
			bits += tab->len + 1;
			val = (tab->level * quantizer_scale * quant_matrix[j]) >> 4;

			/* oddification */
			val = (val - 1) | 1;

			/* if(bitstream_get (1)) val = -val; */
			val = (val ^ SBITS(bit_buf, 1)) - SBITS(bit_buf, 1);

			SATURATE(val);
			dest[j] = val;

			bit_buf <<= 1;
			NEEDBITS;

			continue;
		}
		else if(bit_buf >= 0x04000000)
		{
			tab = DCT_B14_8 + (UBITS(bit_buf, 8) - 4);

			i += tab->run;
			if(i < 64)
				goto normal_code;

			/* escape code */

			i += UBITS(bit_buf << 6, 6) - 64;
			if(i >= 64)
				break;	/* illegal, check needed to avoid buffer overflow */

			j = scan[i];

			DUMPBITS(12);
			NEEDBITS;
			val = SBITS(bit_buf, 8);
			if(!(val & 0x7f))
			{
				DUMPBITS(8);
				val = UBITS(bit_buf, 8) + 2 * val;
			}
			val = (val * quantizer_scale * quant_matrix[j]) / 16;

			/* oddification */
			val = (val + ~SBITS(val, 1)) | 1;

			SATURATE(val);
			dest[j] = val;

			DUMPBITS(8);
			NEEDBITS;

			continue;
		}
		else if(bit_buf >= 0x02000000)
		{
			tab = DCT_B14_10 + (UBITS(bit_buf, 10) - 8);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00800000)
		{
			tab = DCT_13 + (UBITS(bit_buf, 13) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00200000)
		{
			tab = DCT_15 + (UBITS(bit_buf, 15) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else
		{
			tab = DCT_16 + UBITS(bit_buf, 16);
			bit_buf <<= 16;
			GETWORD(bit_buf, bits + 16, bit_ptr);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}

		break;	/* illegal, check needed to avoid buffer overflow */
	}

	DUMPBITS(2);	/* dump end of block code */
	m_bitstream_buf = bit_buf;
	m_bitstream_bits = bits;
	m_bitstream_ptr = bit_ptr;
}

int CMpeg2Decoder::get_mpeg1_non_intra_block()
{
    int i, j;
    int val;
    const uint8_t* scan = m_scan;
    const uint8_t* quant_matrix = m_non_intra_quantizer_matrix;
    int quantizer_scale = m_quantizer_scale;
    const DCTtab* tab;
    uint32_t bit_buf;
    int bits;
    const uint8_t* bit_ptr;
    int16_t* dest;

    i = -1;
    dest = m_DCTblock;

    bit_buf = m_bitstream_buf;
    bits = m_bitstream_bits;
    bit_ptr = m_bitstream_ptr;

    NEEDBITS;
    if(bit_buf >= 0x28000000)
	{
		tab = DCT_B14DC_5 + (UBITS(bit_buf, 5) - 5);
		goto entry_1;
	}
	else
	{
		goto entry_2;
	}

	while(1)
	{
		if(bit_buf >= 0x28000000)
		{
			tab = DCT_B14AC_5 + (UBITS(bit_buf, 5) - 5);
entry_1:
			i += tab->run;
			if(i >= 64)
				break;	/* end of block */
normal_code:
			j = scan[i];
			bit_buf <<= tab->len;
			bits += tab->len + 1;
			val = ((2*tab->level+1) * quantizer_scale * quant_matrix[j]) >> 5;

			/* oddification */
			val = (val - 1) | 1;

			/* if(bitstream_get (1)) val = -val; */
			val = (val ^ SBITS(bit_buf, 1)) - SBITS(bit_buf, 1);

			SATURATE(val);
			dest[j] = val;

			bit_buf <<= 1;
			NEEDBITS;

			continue;
		}

entry_2:
		if(bit_buf >= 0x04000000)
		{
			tab = DCT_B14_8 + (UBITS(bit_buf, 8) - 4);

			i += tab->run;
			if(i < 64)
				goto normal_code;

			/* escape code */

			i += UBITS(bit_buf << 6, 6) - 64;
			if(i >= 64)
				break;	/* illegal, check needed to avoid buffer overflow */

			j = scan[i];

			DUMPBITS(12);
			NEEDBITS;
			val = SBITS(bit_buf, 8);
			if(!(val & 0x7f))
			{
				DUMPBITS(8);
				val = UBITS(bit_buf, 8) + 2 * val;
			}
			val = 2 * (val + SBITS(val, 1)) + 1;
			val = (val * quantizer_scale * quant_matrix[j]) / 32;

			/* oddification */
			val = (val + ~SBITS(val, 1)) | 1;

			SATURATE(val);
			dest[j] = val;

			DUMPBITS(8);
			NEEDBITS;

			continue;
		}
		else if(bit_buf >= 0x02000000)
		{
			tab = DCT_B14_10 + (UBITS(bit_buf, 10) - 8);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00800000)
		{
			tab = DCT_13 + (UBITS(bit_buf, 13) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else if(bit_buf >= 0x00200000)
		{
			tab = DCT_15 + (UBITS(bit_buf, 15) - 16);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}
		else
		{
			tab = DCT_16 + UBITS(bit_buf, 16);
			bit_buf <<= 16;
			GETWORD (bit_buf, bits + 16, bit_ptr);
			i += tab->run;
			if(i < 64)
				goto normal_code;
		}

		break;	/* illegal, check needed to avoid buffer overflow */
	}

	DUMPBITS(2);	/* dump end of block code */
	m_bitstream_buf = bit_buf;
	m_bitstream_bits = bits;
	m_bitstream_ptr = bit_ptr;

	return i;
}

#define bit_buf (m_bitstream_buf)
#define bits (m_bitstream_bits)
#define bit_ptr (m_bitstream_ptr)

void CMpeg2Decoder::slice_intra_DCT(const int cc, uint8_t* dest, int stride)
{
    NEEDBITS;

    /* Get the intra DC coefficient and inverse quantize it */
	m_dc_dct_pred[cc] += (cc == 0)
		? get_luma_dc_dct_diff()
		: get_chroma_dc_dct_diff();
    
	m_DCTblock[0] = m_dc_dct_pred[cc] << (3 - m_intra_dc_precision);

    if(m_mpeg1)
	{
		if(m_coding_type != D_TYPE)
			get_mpeg1_intra_block();
    }
	else if(m_intra_vlc_format)
	{
		get_intra_block_B15();
	}
    else
	{
		get_intra_block_B14();
	}

    m_idct_copy(m_DCTblock, dest, stride);
}

void CMpeg2Decoder::slice_non_intra_DCT(uint8_t* dest, int stride)
{
    int last = m_mpeg1 
		? get_mpeg1_non_intra_block()
		: get_non_intra_block ();
    
	m_idct_add(last, m_DCTblock, dest, stride);
}

void CMpeg2Decoder::MOTION(
	mpeg2_mc_fct * const * const table, uint8_t** ref, 
	int motion_x, int motion_y, 
	unsigned int size, unsigned int y, unsigned int limit_y)
{
	unsigned int pos_x, pos_y, xy_half, offset, dest_offset;

	pos_x = 2 * m_offset + motion_x;
	pos_y = 2 * m_v_offset + motion_y + 2 * y;

	if(pos_x > m_limit_x)
	{
		pos_x = ((int)pos_x < 0) ? 0 : m_limit_x;
		motion_x = pos_x - 2 * m_offset;
	}

	if(pos_y > limit_y)
	{
		pos_y = ((int)pos_y < 0) ? 0 : limit_y;
		motion_y = pos_y - 2 * m_v_offset - 2 * y;
	}

	xy_half = ((pos_y & 1) << 1) | (pos_x & 1);
	offset = (pos_x >> 1) + (pos_y >> 1) * m_stride;
	table[xy_half] (m_dest[0] + y * m_stride + m_offset, ref[0] + offset, m_stride, size);

	motion_x /= 2;	motion_y /= 2;

	xy_half = ((motion_y & 1) << 1) | (motion_x & 1);
	offset = ((m_offset + motion_x) >> 1) + ((((m_v_offset + motion_y) >> 1) + y/2) * m_uv_stride);
	dest_offset = y/2 * m_uv_stride + (m_offset >> 1);
	table[4+xy_half] (m_dest[1] + dest_offset, ref[1] + offset, m_uv_stride, size/2);
	table[4+xy_half] (m_dest[2] + dest_offset, ref[2] + offset, m_uv_stride, size/2);
}

void CMpeg2Decoder::MOTION_FIELD(
	mpeg2_mc_fct * const * const table, uint8_t** ref, 
	int motion_x, int motion_y, 
	int dest_field, int src_field, unsigned int op)
{
    unsigned int pos_x, pos_y, xy_half, offset, dest_offset;

	pos_x = 2 * m_offset + motion_x;
	pos_y = m_v_offset + motion_y;

	if(pos_x > m_limit_x)
	{
		pos_x = ((int)pos_x < 0) ? 0 : m_limit_x;
		motion_x = pos_x - 2 * m_offset;
	}

	if(pos_y > m_limit_y)
	{
		pos_y = ((int)pos_y < 0) ? 0 : m_limit_y;
		motion_y = pos_y - m_v_offset;
	}

	xy_half = ((pos_y & 1) << 1) | (pos_x & 1);
	offset = (pos_x >> 1) + ((op ? (pos_y | 1) : (pos_y & ~1)) + src_field) * m_stride;
	table[xy_half] (m_dest[0] + dest_field * m_stride + m_offset, ref[0] + offset, 2 * m_stride, 8);
	
	motion_x /= 2;	motion_y /= 2;

	xy_half = ((motion_y & 1) << 1) | (motion_x & 1);
	offset = ((m_offset + motion_x) >> 1) + (((m_v_offset >> 1) + (op ? (motion_y | 1) :(motion_y & ~1)) + src_field) * m_uv_stride);
	dest_offset = dest_field * m_uv_stride + (m_offset >> 1);
	table[4+xy_half] (m_dest[1] + dest_offset, ref[1] + offset, 2 * m_uv_stride, 4);
	table[4+xy_half] (m_dest[2] + dest_offset, ref[2] + offset, 2 * m_uv_stride, 4);
}

void CMpeg2Decoder::motion_mp1(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y;

    NEEDBITS;
    motion_x = motion->pmv[0][0] + (get_motion_delta(motion->f_code[0]) << motion->f_code[1]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0] + motion->f_code[1]);
    motion->pmv[0][0] = motion_x;

    NEEDBITS;
    motion_y = motion->pmv[0][1] + (get_motion_delta(motion->f_code[0]) << motion->f_code[1]);
    motion_y = bound_motion_vector(motion_y, motion->f_code[0] + motion->f_code[1]);
    motion->pmv[0][1] = motion_y;

    MOTION(table, motion->ref[0], motion_x, motion_y, 16, 0, m_limit_y_16);
}

void CMpeg2Decoder::motion_fr_frame(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y;

    NEEDBITS;
    motion_x = motion->pmv[0][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0]);
    motion->pmv[1][0] = motion->pmv[0][0] = motion_x;

    NEEDBITS;
    motion_y = motion->pmv[0][1] + get_motion_delta(motion->f_code[1]);
    motion_y = bound_motion_vector (motion_y, motion->f_code[1]);
    motion->pmv[1][1] = motion->pmv[0][1] = motion_y;

    MOTION(table, motion->ref[0], motion_x, motion_y, 16, 0, m_limit_y_16);
}

void CMpeg2Decoder::motion_fr_field(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y, field;

    NEEDBITS;
    field = UBITS(bit_buf, 1);
    DUMPBITS(1);

    motion_x = motion->pmv[0][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0]);
    motion->pmv[0][0] = motion_x;

    NEEDBITS;
    motion_y = (motion->pmv[0][1] >> 1) + get_motion_delta(motion->f_code[1]);
    /* motion_y = bound_motion_vector(motion_y, motion->f_code[1]); */
    motion->pmv[0][1] = motion_y << 1;

    MOTION_FIELD(table, motion->ref[0], motion_x, motion_y, 0, field, 0);

    NEEDBITS;
    field = UBITS(bit_buf, 1);
    DUMPBITS(1);

    motion_x = motion->pmv[1][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0]);
    motion->pmv[1][0] = motion_x;

    NEEDBITS;
    motion_y = (motion->pmv[1][1] >> 1) + get_motion_delta(motion->f_code[1]);
    /* motion_y = bound_motion_vector(motion_y, motion->f_code[1]); */
    motion->pmv[1][1] = motion_y << 1;

    MOTION_FIELD(table, motion->ref[0], motion_x, motion_y, 1, field, 0);
}

void CMpeg2Decoder::motion_fr_dmv(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y, dmv_x, dmv_y, m, other_x, other_y;

    NEEDBITS;
    motion_x = motion->pmv[0][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0]);
    motion->pmv[1][0] = motion->pmv[0][0] = motion_x;
    NEEDBITS;
    dmv_x = get_dmv();

    motion_y = (motion->pmv[0][1] >> 1) + get_motion_delta(motion->f_code[1]);
    /* motion_y = bound_motion_vector (motion_y, motion->f_code[1]); */
    motion->pmv[1][1] = motion->pmv[0][1] = motion_y << 1;
    dmv_y = get_dmv();

    m = m_top_field_first ? 1 : 3;
    other_x = ((motion_x * m + (motion_x > 0)) >> 1) + dmv_x;
    other_y = ((motion_y * m + (motion_y > 0)) >> 1) + dmv_y - 1;
    MOTION_FIELD(m_mc->put, motion->ref[0], other_x, other_y, 0, 0, 1);

    m = m_top_field_first ? 3 : 1;
    other_x = ((motion_x * m + (motion_x > 0)) >> 1) + dmv_x;
    other_y = ((motion_y * m + (motion_y > 0)) >> 1) + dmv_y + 1;
    MOTION_FIELD(m_mc->put, motion->ref[0], other_x, other_y, 1, 0, 0);

    unsigned int pos_x, pos_y, xy_half, offset;

    pos_x = 2 * m_offset + motion_x;
    pos_y = m_v_offset + motion_y;
    if(pos_x > m_limit_x)
	{
		pos_x = ((int)pos_x < 0) ? 0 : m_limit_x;
		motion_x = pos_x - 2 * m_offset;
    }
    if(pos_y > m_limit_y)
	{
		pos_y = ((int)pos_y < 0) ? 0 : m_limit_y;
		motion_y = pos_y - m_v_offset;
    }

    xy_half = ((pos_y & 1) << 1) | (pos_x & 1);
    offset = (pos_x >> 1) + (pos_y & ~1) * m_stride;
    m_mc->avg[xy_half](m_dest[0] + m_offset, motion->ref[0][0] + offset, 2 * m_stride, 8);
    m_mc->avg[xy_half](m_dest[0] + m_stride + m_offset, motion->ref[0][0] + m_stride + offset, 2 * m_stride, 8);
    motion_x /= 2; 
	motion_y /= 2;
    xy_half = ((motion_y & 1) << 1) | (motion_x & 1);
    offset = ((m_offset + motion_x) >> 1) + ((m_v_offset >> 1) + (motion_y & ~1)) * m_uv_stride;
    m_mc->avg[4+xy_half](m_dest[1] + (m_offset >> 1), motion->ref[0][1] + offset, 2 * m_uv_stride, 4);
    m_mc->avg[4+xy_half](m_dest[1] + m_uv_stride + (m_offset >> 1), motion->ref[0][1] + m_uv_stride + offset, 2 * m_uv_stride, 4);
    m_mc->avg[4+xy_half](m_dest[2] + (m_offset >> 1), motion->ref[0][2] + offset, 2 * m_uv_stride, 4);
    m_mc->avg[4+xy_half](m_dest[2] + m_uv_stride + (m_offset >> 1), motion->ref[0][2] + m_uv_stride + offset, 2 * m_uv_stride, 4);
}

void CMpeg2Decoder::motion_reuse(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y;

    motion_x = motion->pmv[0][0];
    motion_y = motion->pmv[0][1];

    MOTION(table, motion->ref[0], motion_x, motion_y, 16, 0, m_limit_y_16);
}

void CMpeg2Decoder::motion_zero(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    unsigned int offset;

    table[0](m_dest[0] + m_offset, motion->ref[0][0] + m_offset + m_v_offset * m_stride, m_stride, 16);
    offset = (m_offset >> 1) + (m_v_offset >> 1) * m_uv_stride;
    table[4](m_dest[1] + (m_offset >> 1), motion->ref[0][1] + offset, m_uv_stride, 8);
    table[4](m_dest[2] + (m_offset >> 1), motion->ref[0][2] + offset, m_uv_stride, 8);
}

/* like motion_frame, but parsing without actual motion compensation */
void CMpeg2Decoder::motion_fr_conceal()
{
    int tmp;

    NEEDBITS;
    tmp = (m_f_motion.pmv[0][0] + get_motion_delta(m_f_motion.f_code[0]));
    tmp = bound_motion_vector(tmp, m_f_motion.f_code[0]);
    m_f_motion.pmv[1][0] = m_f_motion.pmv[0][0] = tmp;

    NEEDBITS;
    tmp = m_f_motion.pmv[0][1] + get_motion_delta(m_f_motion.f_code[1]);
    tmp = bound_motion_vector(tmp, m_f_motion.f_code[1]);
    m_f_motion.pmv[1][1] = m_f_motion.pmv[0][1] = tmp;

    DUMPBITS(1); /* remove marker_bit */
}

void CMpeg2Decoder::motion_fi_field(motion_t * motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y;
    uint8_t** ref_field;

    NEEDBITS;
    ref_field = motion->ref2[UBITS(bit_buf, 1)];
    DUMPBITS(1);

    motion_x = motion->pmv[0][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector (motion_x, motion->f_code[0]);
    motion->pmv[1][0] = motion->pmv[0][0] = motion_x;

    NEEDBITS;
    motion_y = motion->pmv[0][1] + get_motion_delta(motion->f_code[1]);
    motion_y = bound_motion_vector(motion_y, motion->f_code[1]);
    motion->pmv[1][1] = motion->pmv[0][1] = motion_y;

    MOTION(table, ref_field, motion_x, motion_y, 16, 0, m_limit_y_16);
}

void CMpeg2Decoder::motion_fi_16x8(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y;
    uint8_t** ref_field;

    NEEDBITS;
    ref_field = motion->ref2[UBITS(bit_buf, 1)];
    DUMPBITS(1);

    motion_x = motion->pmv[0][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0]);
    motion->pmv[0][0] = motion_x;

    NEEDBITS;
    motion_y = motion->pmv[0][1] + get_motion_delta(motion->f_code[1]);
    motion_y = bound_motion_vector(motion_y, motion->f_code[1]);
    motion->pmv[0][1] = motion_y;

    MOTION(table, ref_field, motion_x, motion_y, 8, 0, m_limit_y_8);

    NEEDBITS;
    ref_field = motion->ref2[UBITS(bit_buf, 1)];
    DUMPBITS(1);

    motion_x = motion->pmv[1][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector (motion_x, motion->f_code[0]);
    motion->pmv[1][0] = motion_x;

    NEEDBITS;
    motion_y = motion->pmv[1][1] + get_motion_delta(motion->f_code[1]);
    motion_y = bound_motion_vector(motion_y, motion->f_code[1]);
    motion->pmv[1][1] = motion_y;

    MOTION(table, ref_field, motion_x, motion_y, 8, 8, m_limit_y_8);
}

void CMpeg2Decoder::motion_fi_dmv(motion_t* motion, mpeg2_mc_fct * const * const table)
{
    int motion_x, motion_y, other_x, other_y;

    NEEDBITS;
    motion_x = motion->pmv[0][0] + get_motion_delta(motion->f_code[0]);
    motion_x = bound_motion_vector(motion_x, motion->f_code[0]);
    motion->pmv[1][0] = motion->pmv[0][0] = motion_x;
    NEEDBITS;
    other_x = ((motion_x + (motion_x > 0)) >> 1) + get_dmv();

    motion_y = motion->pmv[0][1] + get_motion_delta(motion->f_code[1]);
    motion_y = bound_motion_vector(motion_y, motion->f_code[1]);
    motion->pmv[1][1] = motion->pmv[0][1] = motion_y;
    other_y = ((motion_y + (motion_y > 0)) >> 1) + get_dmv () + m_dmv_offset;

    MOTION(m_mc->put, motion->ref[0], motion_x, motion_y, 16, 0, m_limit_y_16);
    MOTION(m_mc->avg, motion->ref[1], other_x, other_y, 16, 0, m_limit_y_16);
}

void CMpeg2Decoder::motion_fi_conceal()
{
    int tmp;

    NEEDBITS;
    DUMPBITS(1); /* remove field_select */

    tmp = m_f_motion.pmv[0][0] + get_motion_delta (m_f_motion.f_code[0]);
    tmp = bound_motion_vector(tmp, m_f_motion.f_code[0]);
    m_f_motion.pmv[1][0] = m_f_motion.pmv[0][0] = tmp;

    NEEDBITS;
    tmp = m_f_motion.pmv[0][1] + get_motion_delta(m_f_motion.f_code[1]);
    tmp = bound_motion_vector(tmp, m_f_motion.f_code[1]);
    m_f_motion.pmv[1][1] = m_f_motion.pmv[0][1] = tmp;

    DUMPBITS(1); /* remove marker_bit */
}

#define MOTION_CALL(routine, direction)				\
do {								\
    if((direction) & MACROBLOCK_MOTION_FORWARD)		\
	routine(&m_f_motion, m_mc->put);	\
    if((direction) & MACROBLOCK_MOTION_BACKWARD)		\
	routine(&m_b_motion, (direction & MACROBLOCK_MOTION_FORWARD) ? m_mc->avg : m_mc->put);	\
} while (0)

#define NEXT_MACROBLOCK							\
do {									\
    m_offset += 16;						\
    if(m_offset == m_width) {				\
	    m_dest[0] += 16 * m_stride;			\
	    m_dest[1] += 4 * m_stride;			\
	    m_dest[2] += 4 * m_stride;			\
		m_v_offset += 16;					\
		if(m_v_offset > m_limit_y)			\
			return;							\
		m_offset = 0;						\
    }									\
} while (0)

void CMpeg2Decoder::mpeg2_init_fbuf(uint8_t* current_fbuf[3], uint8_t* forward_fbuf[3], uint8_t* backward_fbuf[3])
{
    int offset, stride, height, bottom_field;

    stride = m_width;
    bottom_field = (m_picture_structure == BOTTOM_FIELD);
    offset = bottom_field ? stride : 0;
    height = m_height;

    m_picture_dest[0] = current_fbuf[0] + offset;
    m_picture_dest[1] = current_fbuf[1] + (offset >> 1);
    m_picture_dest[2] = current_fbuf[2] + (offset >> 1);

    m_f_motion.ref[0][0] = forward_fbuf[0] + offset;
    m_f_motion.ref[0][1] = forward_fbuf[1] + (offset >> 1);
    m_f_motion.ref[0][2] = forward_fbuf[2] + (offset >> 1);

    m_b_motion.ref[0][0] = backward_fbuf[0] + offset;
    m_b_motion.ref[0][1] = backward_fbuf[1] + (offset >> 1);
    m_b_motion.ref[0][2] = backward_fbuf[2] + (offset >> 1);

    if(m_picture_structure != FRAME_PICTURE)
	{
		m_dmv_offset = bottom_field ? 1 : -1;
		m_f_motion.ref2[0] = m_f_motion.ref[bottom_field];
		m_f_motion.ref2[1] = m_f_motion.ref[!bottom_field];
		m_b_motion.ref2[0] = m_b_motion.ref[bottom_field];
		m_b_motion.ref2[1] = m_b_motion.ref[!bottom_field];
		offset = stride - offset;

		if(m_second_field && (m_coding_type != B_TYPE))
			forward_fbuf = current_fbuf;

		m_f_motion.ref[1][0] = forward_fbuf[0] + offset;
		m_f_motion.ref[1][1] = forward_fbuf[1] + (offset >> 1);
		m_f_motion.ref[1][2] = forward_fbuf[2] + (offset >> 1);

		m_b_motion.ref[1][0] = backward_fbuf[0] + offset;
		m_b_motion.ref[1][1] = backward_fbuf[1] + (offset >> 1);
		m_b_motion.ref[1][2] = backward_fbuf[2] + (offset >> 1);

		stride <<= 1;
		height >>= 1;
	}

	m_stride = stride;
	m_uv_stride = stride >> 1;
	m_limit_x = 2 * m_width - 32;
	m_limit_y_16 = 2 * height - 32;
	m_limit_y_8 = 2 * height - 16;
	m_limit_y = height - 16;
}

int CMpeg2Decoder::slice_init(int code)
{
    int offset;
    const MBAtab* mba;

    m_dc_dct_pred[0] = m_dc_dct_pred[1] =
	m_dc_dct_pred[2] = 128 << m_intra_dc_precision;

    m_f_motion.pmv[0][0] = m_f_motion.pmv[0][1] = 0;
    m_f_motion.pmv[1][0] = m_f_motion.pmv[1][1] = 0;
    m_b_motion.pmv[0][0] = m_b_motion.pmv[0][1] = 0;
    m_b_motion.pmv[1][0] = m_b_motion.pmv[1][1] = 0;

    if(m_vertical_position_extension)
	{
		code += UBITS(bit_buf, 3) << 7;
		DUMPBITS(3);
    }

    m_v_offset = (code - 1) * 16;
    offset = (code - 1) * m_stride * 4;

    m_dest[0] = m_picture_dest[0] + offset * 4;
    m_dest[1] = m_picture_dest[1] + offset;
    m_dest[2] = m_picture_dest[2] + offset;

    m_quantizer_scale = get_quantizer_scale();

    /* ignore intra_slice and all the extra data */
    while(bit_buf & 0x80000000)
	{
		DUMPBITS(9);
		NEEDBITS;
    }

    /* decode initial macroblock address increment */
    offset = 0;
    while(1)
	{
		if(bit_buf >= 0x08000000)
		{
			mba = MBA_5 + (UBITS(bit_buf, 6) - 2);
			break;
		}
		else if(bit_buf >= 0x01800000)
		{
			mba = MBA_11 + (UBITS(bit_buf, 12) - 24);
			break;
		}
		else 
		{
			switch(UBITS(bit_buf, 12))
			{
			case 8:		/* macroblock_escape */
				offset += 33;
				DUMPBITS(11);
				NEEDBITS;
				continue;
			case 15:	/* macroblock_stuffing (MPEG1 only) */
				bit_buf &= 0xfffff;
				DUMPBITS(11);
				NEEDBITS;
				continue;
			default:	/* error */
				return 1;
			}
		}
	}

	DUMPBITS(mba->len + 1);
	m_offset = (offset + mba->mba) << 4;

	while(m_offset - m_width >= 0)
	{
		m_offset -= m_width;
		m_dest[0] += 16 * m_stride;
		m_dest[1] += 4 * m_stride;
		m_dest[2] += 4 * m_stride;
		m_v_offset += 16;
	}

	if(m_v_offset > m_limit_y)
		return 1;

	return 0;
}

void CMpeg2Decoder::mpeg2_slice(int code, const uint8_t* buffer)
{
    m_bitstream_buf = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
    m_bitstream_ptr = buffer + 4;
    m_bitstream_bits = -16;

    if(slice_init(code))
		return;

	while(1)
	{
		int macroblock_modes;
		int mba_inc;
		const MBAtab * mba;

		NEEDBITS;

		macroblock_modes = get_macroblock_modes();

		/* maybe integrate MACROBLOCK_QUANT test into get_macroblock_modes ? */
		if(macroblock_modes & MACROBLOCK_QUANT)
			m_quantizer_scale = get_quantizer_scale();

		if(macroblock_modes & MACROBLOCK_INTRA)
		{
			int DCT_offset, DCT_stride;
			int offset;
			uint8_t* dest_y;

			if(m_concealment_motion_vectors)
			{
				if(m_picture_structure == FRAME_PICTURE) motion_fr_conceal();
				else motion_fi_conceal();
			}
			else
			{
				m_f_motion.pmv[0][0] = m_f_motion.pmv[0][1] = 0;
				m_f_motion.pmv[1][0] = m_f_motion.pmv[1][1] = 0;
				m_b_motion.pmv[0][0] = m_b_motion.pmv[0][1] = 0;
				m_b_motion.pmv[1][0] = m_b_motion.pmv[1][1] = 0;
			}

			if(macroblock_modes & DCT_TYPE_INTERLACED)
			{
				DCT_offset = m_stride;
				DCT_stride = m_stride * 2;
			}
			else
			{
				DCT_offset = m_stride * 8;
				DCT_stride = m_stride;
			}

			offset = m_offset;
			dest_y = m_dest[0] + offset;
			slice_intra_DCT(0, dest_y, DCT_stride);
			slice_intra_DCT(0, dest_y + 8, DCT_stride);
			slice_intra_DCT(0, dest_y + DCT_offset, DCT_stride);
			slice_intra_DCT(0, dest_y + DCT_offset + 8, DCT_stride);
			slice_intra_DCT(1, m_dest[1] + (offset >> 1), m_uv_stride);
			slice_intra_DCT (2, m_dest[2] + (offset >> 1), m_uv_stride);

			if(m_coding_type == D_TYPE)
			{
				NEEDBITS;
				DUMPBITS(1);
			}
		}
		else
		{
			if(m_picture_structure == FRAME_PICTURE)
			{
				switch((macroblock_modes >> 6) & 3) // macroblock_modes & MOTION_TYPE_MASK
				{
				case 0:
					// non-intra mb without forward mv in a P picture //
					m_f_motion.pmv[0][0] = 0;
					m_f_motion.pmv[0][1] = 0;
					m_f_motion.pmv[1][0] = 0;
					m_f_motion.pmv[1][1] = 0;
					MOTION_CALL(motion_zero, MACROBLOCK_MOTION_FORWARD);
					break;

				case 1: // MC_FIELD:
					MOTION_CALL(motion_fr_field, macroblock_modes);
					break;

				case 2: // MC_FRAME:

					if(m_mpeg1) MOTION_CALL(motion_mp1, macroblock_modes);
					else MOTION_CALL (motion_fr_frame, macroblock_modes);
					break;

				case 3: // MC_DMV:
					MOTION_CALL(motion_fr_dmv, MACROBLOCK_MOTION_FORWARD);
					break;

				default:
					__assume(0);
				}
			}
			else
			{
				switch((macroblock_modes >> 6) & 3) // macroblock_modes & MOTION_TYPE_MASK
				{
				case 0:
					/* non-intra mb without forward mv in a P picture */
					m_f_motion.pmv[0][0] = 0;
					m_f_motion.pmv[0][1] = 0;
					m_f_motion.pmv[1][0] = 0;
					m_f_motion.pmv[1][1] = 0;
					MOTION_CALL(motion_zero, MACROBLOCK_MOTION_FORWARD);
					break;

				case 1: // MC_FIELD
					MOTION_CALL(motion_fi_field, macroblock_modes);
					break;

				case 2: // MC_16X8
					MOTION_CALL(motion_fi_16x8, macroblock_modes);
					break;

				case 3: // MC_DMV
					MOTION_CALL(motion_fi_dmv, MACROBLOCK_MOTION_FORWARD);
					break;

				default:
					__assume(0);
				}
			}

			if(macroblock_modes & MACROBLOCK_PATTERN)
			{
				int coded_block_pattern;
				int DCT_offset, DCT_stride;
				int offset;
				uint8_t* dest_y;

				if(macroblock_modes & DCT_TYPE_INTERLACED)
				{
					DCT_offset = m_stride;
					DCT_stride = m_stride * 2;
				}
				else
				{
					DCT_offset = m_stride * 8;
					DCT_stride = m_stride;
				}

				coded_block_pattern = get_coded_block_pattern();

				offset = m_offset;
				dest_y = m_dest[0] + offset;

				if(coded_block_pattern & 0x20)
					slice_non_intra_DCT(dest_y, DCT_stride);
				if(coded_block_pattern & 0x10)
					slice_non_intra_DCT(dest_y + 8, DCT_stride);
				if(coded_block_pattern & 0x08)
					slice_non_intra_DCT(dest_y + DCT_offset, DCT_stride);
				if(coded_block_pattern & 0x04)
					slice_non_intra_DCT(dest_y + DCT_offset + 8, DCT_stride);
				if(coded_block_pattern & 0x2)
					slice_non_intra_DCT(m_dest[1] + (offset >> 1), m_uv_stride);
				if(coded_block_pattern & 0x1)
					slice_non_intra_DCT(m_dest[2] + (offset >> 1), m_uv_stride);
			}

			m_dc_dct_pred[0] = 
			m_dc_dct_pred[1] = 
			m_dc_dct_pred[2] = 128 << m_intra_dc_precision;
		}

		NEXT_MACROBLOCK;

		NEEDBITS;
		mba_inc = 0;
		while(1)
		{
			if(bit_buf >= 0x10000000)
			{
				mba = MBA_5 + (UBITS(bit_buf, 5) - 2);
				break;
			}
			else if(bit_buf >= 0x03000000)
			{
				mba = MBA_11 + (UBITS(bit_buf, 11) - 24);
				break;
			}
			else
			{
				switch(UBITS(bit_buf, 11))
				{
				case 8:		/* macroblock_escape */
					mba_inc += 33;
					/* pass through */
				case 15:	/* macroblock_stuffing (MPEG1 only) */
					DUMPBITS(11);
					NEEDBITS;
					continue;
				default:	/* end of slice, or error */
					return;
				}
			}
		}

		DUMPBITS(mba->len);
		mba_inc += mba->mba;

		if(mba_inc)
		{
			m_dc_dct_pred[0] = 
			m_dc_dct_pred[1] =
			m_dc_dct_pred[2] = 128 << m_intra_dc_precision;

			if(m_coding_type == P_TYPE)
			{
				m_f_motion.pmv[0][0] = m_f_motion.pmv[0][1] = 0;
				m_f_motion.pmv[1][0] = m_f_motion.pmv[1][1] = 0;

				do {
					MOTION_CALL(motion_zero, MACROBLOCK_MOTION_FORWARD);
					NEXT_MACROBLOCK;
				} while(--mba_inc);
			}
			else
			{
				do {
					MOTION_CALL (motion_reuse, macroblock_modes);
					NEXT_MACROBLOCK;
				} while(--mba_inc);
			}
		}
	}
}

#undef bit_buf
#undef bits
#undef bit_ptr

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

CMpeg2Info::CMpeg2Info()
{
	m_sequence = NULL;
    m_gop = NULL;
	Reset();
}

CMpeg2Info::~CMpeg2Info()
{
}

void CMpeg2Info::Reset()
{
    m_current_picture = m_current_picture_2nd = NULL;
    m_display_picture = m_display_picture_2nd = NULL;
    m_current_fbuf = m_display_fbuf = m_discard_fbuf = NULL;
    m_user_data = NULL;
	m_user_data_len = 0;
}

