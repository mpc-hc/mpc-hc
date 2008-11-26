/*
 * DV decoder
 * Copyright (c) 2002 Fabrice Bellard.
 * Copyright (c) 2004 Roman Shaposhnik.
 *
 * DV encoder
 * Copyright (c) 2003 Roman Shaposhnik.
 *
 * 50 Mbps (DVCPRO50) support
 * Copyright (c) 2006 Daniel Maas <dmaas@maasdigital.com>
 *
 * 100 Mbps (DVCPRO HD) support
 * Initial code by Daniel Maas <dmaas@maasdigital.com> (funded by BBC R&D)
 * Final code by Roman Shaposhnik
 *
 * Many thanks to Dan Dennedy <dan@dennedy.org> for providing wealth
 * of DV technical info.
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
 * @file dv.c
 * DV codec.
 */
#define ALT_BITSTREAM_READER
#include "avcodec.h"
#include "dsputil.h"
#include "bitstream.h"
#include "simple_idct.h"
#include "dvdata.h"

//#undef NDEBUG
//#include <assert.h>

typedef struct DVVideoContext {
    const DVprofile *sys;
    AVFrame          picture;
    AVCodecContext  *avctx;
    uint8_t         *buf;

    uint8_t  dv_zigzag[2][64];

    void (*get_pixels)(DCTELEM *block, const uint8_t *pixels, int line_size);
    void (*fdct[2])(DCTELEM *block);
    void (*idct_put[2])(uint8_t *dest, int line_size, DCTELEM *block);
} DVVideoContext;

#define TEX_VLC_BITS 9

#if ENABLE_SMALL
#define DV_VLC_MAP_RUN_SIZE 15
#define DV_VLC_MAP_LEV_SIZE 23
#else
#define DV_VLC_MAP_RUN_SIZE  64
#define DV_VLC_MAP_LEV_SIZE 512 //FIXME sign was removed so this should be /2 but needs check
#endif

/* XXX: also include quantization */
static RL_VLC_ELEM dv_rl_vlc[1184];
/* VLC encoding lookup table */
static struct dv_vlc_pair {
   uint32_t vlc;
   uint8_t  size;
} dv_vlc_map[DV_VLC_MAP_RUN_SIZE][DV_VLC_MAP_LEV_SIZE];

static inline int dv_work_pool_size(const DVprofile *d)
{
    int size = d->n_difchan*d->difseg_size*27;
    if (DV_PROFILE_IS_1080i50(d))
        size -= 3*27;
    if (DV_PROFILE_IS_720p50(d))
        size -= 4*27;
    return size;
}

static inline void dv_calc_mb_coordinates(const DVprofile *d, int chan, int seq, int slot,
                                          uint16_t *tbl)
{
    const static uint8_t off[] = { 2, 6, 8, 0, 4 };
    const static uint8_t shuf1[] = { 36, 18, 54, 0, 72 };
    const static uint8_t shuf2[] = { 24, 12, 36, 0, 48 };
    const static uint8_t shuf3[] = { 18, 9, 27, 0, 36 };

    const static uint8_t l_start[] = {0, 4, 9, 13, 18, 22, 27, 31, 36, 40};
    const static uint8_t l_start_shuffled[] = { 9, 4, 13, 0, 18 };

    const static uint8_t serpent1[] = {0, 1, 2, 2, 1, 0,
                                       0, 1, 2, 2, 1, 0,
                                       0, 1, 2, 2, 1, 0,
                                       0, 1, 2, 2, 1, 0,
                                       0, 1, 2};
    const static uint8_t serpent2[] = {0, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1, 0,
                                       0, 1, 2, 3, 4, 5, 5, 4, 3, 2, 1, 0,
                                       0, 1, 2, 3, 4, 5};

    const static uint8_t remap[][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, /* dummy */
                                       { 0, 0}, { 0, 1}, { 0, 2}, { 0, 3}, {10, 0},
                                       {10, 1}, {10, 2}, {10, 3}, {20, 0}, {20, 1},
                                       {20, 2}, {20, 3}, {30, 0}, {30, 1}, {30, 2},
                                       {30, 3}, {40, 0}, {40, 1}, {40, 2}, {40, 3},
                                       {50, 0}, {50, 1}, {50, 2}, {50, 3}, {60, 0},
                                       {60, 1}, {60, 2}, {60, 3}, {70, 0}, {70, 1},
                                       {70, 2}, {70, 3}, { 0,64}, { 0,65}, { 0,66},
                                       {10,64}, {10,65}, {10,66}, {20,64}, {20,65},
                                       {20,66}, {30,64}, {30,65}, {30,66}, {40,64},
                                       {40,65}, {40,66}, {50,64}, {50,65}, {50,66},
                                       {60,64}, {60,65}, {60,66}, {70,64}, {70,65},
                                       {70,66}, { 0,67}, {20,67}, {40,67}, {60,67}};

    int i, k, m;
    int x, y, blk;

    for (m=0; m<5; m++) {
         switch (d->width) {
         case 1440:
              blk = (chan*11+seq)*27+slot;

              if (chan == 0 && seq == 11) {
                  x = m*27+slot;
                  if (x<90) {
                      y = 0;
                  } else {
                      x = (x - 90)*2;
                      y = 67;
                  }
              } else {
                  i = (4*chan + blk + off[m])%11;
                  k = (blk/11)%27;

                  x = shuf1[m] + (chan&1)*9 + k%9;
                  y = (i*3+k/9)*2 + (chan>>1) + 1;
              }
              tbl[m] = (x<<1)|(y<<9);
              break;
         case 1280:
              blk = (chan*10+seq)*27+slot;

              i = (4*chan + (seq/5) + 2*blk + off[m])%10;
              k = (blk/5)%27;

              x = shuf1[m]+(chan&1)*9 + k%9;
              y = (i*3+k/9)*2 + (chan>>1) + 4;

              if (x >= 80) {
                  x = remap[y][0]+((x-80)<<(y>59));
                  y = remap[y][1];
              }
              tbl[m] = (x<<1)|(y<<9);
              break;
       case 960:
              blk = (chan*10+seq)*27+slot;

              i = (4*chan + (seq/5) + 2*blk + off[m])%10;
              k = (blk/5)%27 + (i&1)*3;

              x = shuf2[m] + k%6 + 6*(chan&1);
              y = l_start[i] + k/6 + 45*(chan>>1);
              tbl[m] = (x<<1)|(y<<9);
              break;
        case 720:
              switch (d->pix_fmt) {
              case PIX_FMT_YUV422P:
                   x = shuf3[m] + slot/3;
                   y = serpent1[slot] +
                       ((((seq + off[m]) % d->difseg_size)<<1) + chan)*3;
                   tbl[m] = (x<<1)|(y<<8);
                   break;
              case PIX_FMT_YUV420P:
                   x = shuf3[m] + slot/3;
                   y = serpent1[slot] +
                       ((seq + off[m]) % d->difseg_size)*3;
                   tbl[m] = (x<<1)|(y<<9);
                   break;
              case PIX_FMT_YUV411P:
                   i = (seq + off[m]) % d->difseg_size;
                   k = slot + ((m==1||m==2)?3:0);

                   x = l_start_shuffled[m] + k/6;
                   y = serpent2[k] + i*6;
                   if (x>21)
                       y = y*2 - i*6;
                   tbl[m] = (x<<2)|(y<<8);
                   break;
              }
        default:
              break;
        }
    }
}

static int dv_init_dynamic_tables(const DVprofile *d)
{
    int j,i,c,s,p;
    uint32_t *factor1, *factor2;
    const int *iweight1, *iweight2;

    if (!d->work_chunks[dv_work_pool_size(d)-1].buf_offset) {
        p = i = 0;
        for (c=0; c<d->n_difchan; c++) {
            for (s=0; s<d->difseg_size; s++) {
                p += 6;
                for (j=0; j<27; j++) {
                    p += !(j%3);
                    if (!(DV_PROFILE_IS_1080i50(d) && c != 0 && s == 11) &&
                        !(DV_PROFILE_IS_720p50(d) && s > 9)) {
                          dv_calc_mb_coordinates(d, c, s, j, &d->work_chunks[i].mb_coordinates[0]);
                          d->work_chunks[i++].buf_offset = p;
                    }
                    p += 5;
                }
            }
        }
    }

    if (!d->idct_factor[DV_PROFILE_IS_HD(d)?8191:5631]) {
        factor1 = &d->idct_factor[0];
        factor2 = &d->idct_factor[DV_PROFILE_IS_HD(d)?4096:2816];
        if (d->height == 720) {
            iweight1 = &dv_iweight_720_y[0];
            iweight2 = &dv_iweight_720_c[0];
        } else {
            iweight1 = &dv_iweight_1080_y[0];
            iweight2 = &dv_iweight_1080_c[0];
            }
        if (DV_PROFILE_IS_HD(d)) {
            for (c = 0; c < 4; c++) {
                for (s = 0; s < 16; s++) {
                    for (i = 0; i < 64; i++) {
                        *factor1++ = (dv100_qstep[s] << (c + 9)) * iweight1[i];
                        *factor2++ = (dv100_qstep[s] << (c + 9)) * iweight2[i];
                    }
                }
            }
        } else {
            iweight1 = &dv_iweight_88[0];
            for (j = 0; j < 2; j++, iweight1 = &dv_iweight_248[0]) {
                for (s = 0; s < 22; s++) {
                    for (i = c = 0; c < 4; c++) {
                        for (; i < dv_quant_areas[c]; i++) {
                            *factor1   = iweight1[i] << (dv_quant_shifts[s][c] + 1);
                            *factor2++ = (*factor1++) << 1;
        }
    }
            }
        }
    }
}

    return 0;
}

static av_cold int dvvideo_init(AVCodecContext *avctx)
{
    DVVideoContext *s = avctx->priv_data;
    DSPContext dsp;
    static int done = 0;
    int i, j;

    if (!done) {
        VLC dv_vlc;
        uint16_t new_dv_vlc_bits[NB_DV_VLC*2];
        uint8_t  new_dv_vlc_len[NB_DV_VLC*2];
        uint8_t  new_dv_vlc_run[NB_DV_VLC*2];
        int16_t  new_dv_vlc_level[NB_DV_VLC*2];

        done = 1;

        /* it's faster to include sign bit in a generic VLC parsing scheme */
        for (i = 0, j = 0; i < NB_DV_VLC; i++, j++) {
            new_dv_vlc_bits[j]  = dv_vlc_bits[i];
            new_dv_vlc_len[j]   = dv_vlc_len[i];
            new_dv_vlc_run[j]   = dv_vlc_run[i];
            new_dv_vlc_level[j] = dv_vlc_level[i];

            if (dv_vlc_level[i]) {
                new_dv_vlc_bits[j] <<= 1;
                new_dv_vlc_len[j]++;

                j++;
                new_dv_vlc_bits[j]  = (dv_vlc_bits[i] << 1) | 1;
                new_dv_vlc_len[j]   =  dv_vlc_len[i] + 1;
                new_dv_vlc_run[j]   =  dv_vlc_run[i];
                new_dv_vlc_level[j] = -dv_vlc_level[i];
            }
        }

        /* NOTE: as a trick, we use the fact the no codes are unused
           to accelerate the parsing of partial codes */
        init_vlc(&dv_vlc, TEX_VLC_BITS, j,
                 new_dv_vlc_len, 1, 1, new_dv_vlc_bits, 2, 2, 0);
        assert(dv_vlc.table_size == 1184);

        for (i = 0; i < dv_vlc.table_size; i++){
            int code = dv_vlc.table[i][0];
            int len  = dv_vlc.table[i][1];
            int level, run;

            if (len < 0){ //more bits needed
                run   = 0;
                level = code;
            } else {
                run   = new_dv_vlc_run  [code] + 1;
                level = new_dv_vlc_level[code];
            }
            dv_rl_vlc[i].len   = len;
            dv_rl_vlc[i].level = level;
            dv_rl_vlc[i].run   = run;
        }
        free_vlc(&dv_vlc);

        for (i = 0; i < NB_DV_VLC - 1; i++) {
           if (dv_vlc_run[i] >= DV_VLC_MAP_RUN_SIZE)
               continue;
#if ENABLE_SMALL
           if (dv_vlc_level[i] >= DV_VLC_MAP_LEV_SIZE)
               continue;
#endif

           if (dv_vlc_map[dv_vlc_run[i]][dv_vlc_level[i]].size != 0)
               continue;

           dv_vlc_map[dv_vlc_run[i]][dv_vlc_level[i]].vlc  =
               dv_vlc_bits[i] << (!!dv_vlc_level[i]);
           dv_vlc_map[dv_vlc_run[i]][dv_vlc_level[i]].size =
               dv_vlc_len[i] + (!!dv_vlc_level[i]);
        }
        for (i = 0; i < DV_VLC_MAP_RUN_SIZE; i++) {
#if ENABLE_SMALL
           for (j = 1; j < DV_VLC_MAP_LEV_SIZE; j++) {
              if (dv_vlc_map[i][j].size == 0) {
                  dv_vlc_map[i][j].vlc = dv_vlc_map[0][j].vlc |
                            (dv_vlc_map[i-1][0].vlc << (dv_vlc_map[0][j].size));
                  dv_vlc_map[i][j].size = dv_vlc_map[i-1][0].size +
                                          dv_vlc_map[0][j].size;
              }
           }
#else
           for (j = 1; j < DV_VLC_MAP_LEV_SIZE/2; j++) {
              if (dv_vlc_map[i][j].size == 0) {
                  dv_vlc_map[i][j].vlc = dv_vlc_map[0][j].vlc |
                            (dv_vlc_map[i-1][0].vlc << (dv_vlc_map[0][j].size));
                  dv_vlc_map[i][j].size = dv_vlc_map[i-1][0].size +
                                          dv_vlc_map[0][j].size;
              }
              dv_vlc_map[i][((uint16_t)(-j))&0x1ff].vlc =
                                            dv_vlc_map[i][j].vlc | 1;
              dv_vlc_map[i][((uint16_t)(-j))&0x1ff].size =
                                            dv_vlc_map[i][j].size;
           }
#endif
        }
    }

    /* Generic DSP setup */
    dsputil_init(&dsp, avctx);
    s->get_pixels = dsp.get_pixels;

    /* 88DCT setup */
    s->fdct[0]     = dsp.fdct;
    s->idct_put[0] = dsp.idct_put;
    for (i = 0; i < 64; i++)
       s->dv_zigzag[0][i] = dsp.idct_permutation[ff_zigzag_direct[i]];

    /* 248DCT setup */
    s->fdct[1]     = dsp.fdct248;
    s->idct_put[1] = ff_simple_idct248_put;  // FIXME: need to add it to DSP
    if (avctx->lowres){
        for (i = 0; i < 64; i++){
            int j = ff_zigzag248_direct[i];
            s->dv_zigzag[1][i] = dsp.idct_permutation[(j & 7) + (j & 8) * 4 + (j & 48) / 2];
        }
    }else
        memcpy(s->dv_zigzag[1], ff_zigzag248_direct, 64);

    avctx->coded_frame = &s->picture;
    s->avctx = avctx;

    return 0;
}

// #define VLC_DEBUG
// #define printf(...) av_log(NULL, AV_LOG_ERROR, __VA_ARGS__)

typedef struct BlockInfo {
    const uint32_t *factor_table;
    const uint8_t *scan_table;
    uint8_t pos; /* position in block */
    void (*idct_put)(uint8_t *dest, int line_size, DCTELEM *block);
    uint8_t partial_bit_count;
    uint16_t partial_bit_buffer;
    int shift_offset;
} BlockInfo;

/* bit budget for AC only in 5 MBs */
static const int vs_total_ac_bits = (100 * 4 + 68*2) * 5;
/* see dv_88_areas and dv_248_areas for details */
static const int mb_area_start[5] = { 1, 6, 21, 43, 64 };

static inline int get_bits_left(GetBitContext *s)
{
    return s->size_in_bits - get_bits_count(s);
}

static inline int put_bits_left(PutBitContext* s)
{
    return (s->buf_end - s->buf) * 8 - put_bits_count(s);
}

/* decode ac coefficients */
static void dv_decode_ac(GetBitContext *gb, BlockInfo *mb, DCTELEM *block)
{
    int last_index = gb->size_in_bits;
    const uint8_t  *scan_table   = mb->scan_table;
    const uint32_t *factor_table = mb->factor_table;
    int pos               = mb->pos;
    int partial_bit_count = mb->partial_bit_count;
    int level, run, vlc_len, index;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);

    /* if we must parse a partial vlc, we do it here */
    if (partial_bit_count > 0) {
        re_cache = ((unsigned)re_cache >> partial_bit_count) |
                   (mb->partial_bit_buffer << (sizeof(re_cache) * 8 - partial_bit_count));
        re_index -= partial_bit_count;
        mb->partial_bit_count = 0;
    }

    /* get the AC coefficients until last_index is reached */
    for (;;) {
#ifdef VLC_DEBUG
        printf("%2d: bits=%04x index=%d\n", pos, SHOW_UBITS(re, gb, 16), re_index);
#endif
        /* our own optimized GET_RL_VLC */
        index   = NEG_USR32(re_cache, TEX_VLC_BITS);
        vlc_len = dv_rl_vlc[index].len;
        if (vlc_len < 0) {
            index = NEG_USR32((unsigned)re_cache << TEX_VLC_BITS, -vlc_len) + dv_rl_vlc[index].level;
            vlc_len = TEX_VLC_BITS - vlc_len;
        }
        level = dv_rl_vlc[index].level;
        run   = dv_rl_vlc[index].run;

        /* gotta check if we're still within gb boundaries */
        if (re_index + vlc_len > last_index) {
            /* should be < 16 bits otherwise a codeword could have been parsed */
            mb->partial_bit_count = last_index - re_index;
            mb->partial_bit_buffer = NEG_USR32(re_cache, mb->partial_bit_count);
            re_index = last_index;
            break;
        }
        re_index += vlc_len;

#ifdef VLC_DEBUG
        printf("run=%d level=%d\n", run, level);
#endif
        pos += run;
        if (pos >= 64)
            break;

        level = (level * factor_table[pos] + (1 << (dv_iweight_bits - 1))) >> dv_iweight_bits;
        block[scan_table[pos]] = level;

        UPDATE_CACHE(re, gb);
    }
    CLOSE_READER(re, gb);
    mb->pos = pos;
}

static inline void bit_copy(PutBitContext *pb, GetBitContext *gb)
{
    int bits_left = get_bits_left(gb);
    while (bits_left >= MIN_CACHE_BITS) {
        put_bits(pb, MIN_CACHE_BITS, get_bits(gb, MIN_CACHE_BITS));
        bits_left -= MIN_CACHE_BITS;
    }
    if (bits_left > 0) {
        put_bits(pb, bits_left, get_bits(gb, bits_left));
    }
}

static inline void dv_calculate_mb_xy(DVVideoContext *s, DVwork_chunk *work_chunk, int m, int *mb_x, int *mb_y)
{
     *mb_x = work_chunk->mb_coordinates[m] & 0xff;
     *mb_y = work_chunk->mb_coordinates[m] >> 8;

     /* We work with 720p frames split in half. The odd half-frame (chan==2,3) is displaced :-( */
     if (s->sys->height == 720 && !(s->buf[1]&0x0C)) {
         *mb_y -= (*mb_y>17)?18:-72; /* shifting the Y coordinate down by 72/2 macro blocks */
     }
}

/* mb_x and mb_y are in units of 8 pixels */
static int dv_decode_video_segment(AVCodecContext *avctx, DVwork_chunk *work_chunk)
{
    DVVideoContext *s = avctx->priv_data;
    int quant, dc, dct_mode, class1, j;
    int mb_index, mb_x, mb_y, last_index;
    int y_stride, linesize;
    DCTELEM *block, *block1;
    int c_offset;
    uint8_t *y_ptr;
    const uint8_t *buf_ptr;
    PutBitContext pb, vs_pb;
    GetBitContext gb;
    BlockInfo mb_data[5 * DV_MAX_BPM], *mb, *mb1;
    DECLARE_ALIGNED_16(DCTELEM, sblock[5*DV_MAX_BPM][64]);
    DECLARE_ALIGNED_8(uint8_t, mb_bit_buffer[80 + 4]); /* allow some slack */
    DECLARE_ALIGNED_8(uint8_t, vs_bit_buffer[5 * 80 + 4]); /* allow some slack */
    const int log2_blocksize = 3-s->avctx->lowres;
    int is_field_mode[5];

    assert((((int)mb_bit_buffer) & 7) == 0);
    assert((((int)vs_bit_buffer) & 7) == 0);

    memset(sblock, 0, sizeof(sblock));

    /* pass 1 : read DC and AC coefficients in blocks */
    buf_ptr = &s->buf[work_chunk->buf_offset*80];
    block1  = &sblock[0][0];
    mb1     = mb_data;
    init_put_bits(&vs_pb, vs_bit_buffer, 5 * 80);
    for (mb_index = 0; mb_index < 5; mb_index++, mb1 += s->sys->bpm, block1 += s->sys->bpm * 64) {
        /* skip header */
        quant = buf_ptr[3] & 0x0f;
        buf_ptr += 4;
        init_put_bits(&pb, mb_bit_buffer, 80);
        mb    = mb1;
        block = block1;
        is_field_mode[mb_index] = 0;
        for (j = 0; j < s->sys->bpm; j++) {
            last_index = s->sys->block_sizes[j];
            init_get_bits(&gb, buf_ptr, last_index);

            /* get the dc */
            dc       = get_sbits(&gb, 9);
            dct_mode = get_bits1(&gb);
            class1   = get_bits(&gb, 2);
            if (DV_PROFILE_IS_HD(s->sys)) {
                mb->idct_put     = s->idct_put[0];
                mb->scan_table   = s->dv_zigzag[0];
                mb->factor_table = &s->sys->idct_factor[(j >= 4)*4*16*64 + class1*16*64 + quant*64];
                is_field_mode[mb_index] |= !j && dct_mode;
            } else {
                mb->idct_put     = s->idct_put[dct_mode && log2_blocksize == 3];
                mb->scan_table   = s->dv_zigzag[dct_mode];
                mb->factor_table = &s->sys->idct_factor[(class1 == 3)*2*22*64 + dct_mode*22*64 +
                                                        (quant + dv_quant_offset[class1])*64];
            }
            dc = dc << 2;
            /* convert to unsigned because 128 is not added in the
               standard IDCT */
            dc += 1024;
            block[0] = dc;
            buf_ptr += last_index >> 3;
            mb->pos               = 0;
            mb->partial_bit_count = 0;

#ifdef VLC_DEBUG
            printf("MB block: %d, %d ", mb_index, j);
#endif
            dv_decode_ac(&gb, mb, block);

            /* write the remaining bits  in a new buffer only if the
               block is finished */
            if (mb->pos >= 64)
                bit_copy(&pb, &gb);

            block += 64;
            mb++;
        }

        /* pass 2 : we can do it just after */
#ifdef VLC_DEBUG
        printf("***pass 2 size=%d MB#=%d\n", put_bits_count(&pb), mb_index);
#endif
        block = block1;
        mb    = mb1;
        init_get_bits(&gb, mb_bit_buffer, put_bits_count(&pb));
        flush_put_bits(&pb);
        for (j = 0; j < s->sys->bpm; j++, block += 64, mb++) {
            if (mb->pos < 64 && get_bits_left(&gb) > 0) {
                dv_decode_ac(&gb, mb, block);
                /* if still not finished, no need to parse other blocks */
                if (mb->pos < 64)
                    break;
            }
        }
        /* all blocks are finished, so the extra bytes can be used at
           the video segment level */
        if (j >= s->sys->bpm)
            bit_copy(&vs_pb, &gb);
    }

    /* we need a pass other the whole video segment */
#ifdef VLC_DEBUG
    printf("***pass 3 size=%d\n", put_bits_count(&vs_pb));
#endif
    block = &sblock[0][0];
    mb    = mb_data;
    init_get_bits(&gb, vs_bit_buffer, put_bits_count(&vs_pb));
    flush_put_bits(&vs_pb);
    for (mb_index = 0; mb_index < 5; mb_index++) {
        for (j = 0; j < s->sys->bpm; j++) {
            if (mb->pos < 64) {
#ifdef VLC_DEBUG
                printf("start %d:%d\n", mb_index, j);
#endif
                dv_decode_ac(&gb, mb, block);
            }
            if (mb->pos >= 64 && mb->pos < 127)
                av_log(NULL, AV_LOG_ERROR, "AC EOB marker is absent pos=%d\n", mb->pos);
            block += 64;
            mb++;
        }
    }

    /* compute idct and place blocks */
    block = &sblock[0][0];
    mb    = mb_data;
    for (mb_index = 0; mb_index < 5; mb_index++) {
        dv_calculate_mb_xy(s, work_chunk, mb_index, &mb_x, &mb_y);

        /* idct_put'ting luminance */
        if ((s->sys->pix_fmt == PIX_FMT_YUV420P) ||
            (s->sys->pix_fmt == PIX_FMT_YUV411P && mb_x >= (704 / 8)) ||
            (s->sys->height >= 720 && mb_y != 134)) {
            y_stride = (s->picture.linesize[0] << ((!is_field_mode[mb_index]) * log2_blocksize));
        } else {
            y_stride = (2 << log2_blocksize);
        }
        y_ptr = s->picture.data[0] + ((mb_y * s->picture.linesize[0] + mb_x) << log2_blocksize);
        linesize = s->picture.linesize[0] << is_field_mode[mb_index];
        mb[0]    .idct_put(y_ptr                                   , linesize, block + 0*64);
        if (s->sys->video_stype == 4) { /* SD 422 */
            mb[2].idct_put(y_ptr + (1 << log2_blocksize)           , linesize, block + 2*64);
        } else {
            mb[1].idct_put(y_ptr + (1 << log2_blocksize)           , linesize, block + 1*64);
            mb[2].idct_put(y_ptr                         + y_stride, linesize, block + 2*64);
            mb[3].idct_put(y_ptr + (1 << log2_blocksize) + y_stride, linesize, block + 3*64);
        }
        mb += 4;
        block += 4*64;

        /* idct_put'ting chrominance */
        c_offset = (((mb_y >>  (s->sys->pix_fmt == PIX_FMT_YUV420P)) * s->picture.linesize[1] +
                     (mb_x >> ((s->sys->pix_fmt == PIX_FMT_YUV411P) ? 2 : 1))) << log2_blocksize);
        for (j = 2; j; j--) {
            uint8_t *c_ptr = s->picture.data[j] + c_offset;
            if (s->sys->pix_fmt == PIX_FMT_YUV411P && mb_x >= (704 / 8)) {
                  uint64_t aligned_pixels[64/8];
                  uint8_t *pixels = (uint8_t*)aligned_pixels;
                  uint8_t *c_ptr1, *ptr1;
                  int x, y;
                  mb->idct_put(pixels, 8, block);
                  for (y = 0; y < (1 << log2_blocksize); y++, c_ptr += s->picture.linesize[j], pixels += 8) {
                      ptr1   = pixels + (1 << (log2_blocksize - 1));
                      c_ptr1 = c_ptr + (s->picture.linesize[j] << log2_blocksize);
                      for (x = 0; x < (1 << (log2_blocksize - 1)); x++) {
                          c_ptr[x]  = pixels[x];
                          c_ptr1[x] = ptr1[x];
                      }
                  }
                  block += 64; mb++;
            } else {
                  y_stride = (mb_y == 134) ? (1 << log2_blocksize) :
                                             s->picture.linesize[j] << ((!is_field_mode[mb_index]) * log2_blocksize);
                  linesize = s->picture.linesize[j] << is_field_mode[mb_index];
                  (mb++)->    idct_put(c_ptr           , linesize, block); block += 64;
                  if (s->sys->bpm == 8) {
                      (mb++)->idct_put(c_ptr + y_stride, linesize, block); block += 64;
                  }
            }
        }
    }
    return 0;
}

#ifdef CONFIG_DVVIDEO_DECODER
/* NOTE: exactly one frame must be given (120000 bytes for NTSC,
   144000 bytes for PAL - or twice those for 50Mbps) */
static int dvvideo_decode_frame(AVCodecContext *avctx,
                                 void *data, int *data_size,
                                 const uint8_t *buf, int buf_size)
{
    DVVideoContext *s = avctx->priv_data;

    s->sys = dv_frame_profile(buf);
    if (!s->sys || buf_size < s->sys->frame_size || dv_init_dynamic_tables(s->sys))
        return -1; /* NOTE: we only accept several full frames */

    if (s->picture.data[0])
        avctx->release_buffer(avctx, &s->picture);

    s->picture.reference = 0;
    s->picture.key_frame = 1;
    s->picture.pict_type = FF_I_TYPE;
    avctx->pix_fmt   = s->sys->pix_fmt;
    avctx->time_base = s->sys->time_base;
    avcodec_set_dimensions(avctx, s->sys->width, s->sys->height);
    if (avctx->get_buffer(avctx, &s->picture) < 0) {
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return -1;
    }
    s->picture.interlaced_frame = 1;
    s->picture.top_field_first  = 0;

    s->buf = buf;
    avctx->execute(avctx, dv_decode_video_segment, s->sys->work_chunks, NULL,
                   dv_work_pool_size(s->sys), sizeof(DVwork_chunk));

    emms_c();

    /* return image */
    *data_size = sizeof(AVFrame);
    *(AVFrame*)data = s->picture;

    return s->sys->frame_size;
}
#endif /* CONFIG_DVVIDEO_DECODER */

static int dvvideo_close(AVCodecContext *c)
{
    DVVideoContext *s = c->priv_data;

    if (s->picture.data[0])
        c->release_buffer(c, &s->picture);

    return 0;
}

#ifdef CONFIG_DVVIDEO_DECODER
AVCodec dvvideo_decoder = {
    "dvvideo",
    CODEC_TYPE_VIDEO,
    CODEC_ID_DVVIDEO,
    sizeof(DVVideoContext),
    /*.init=*/dvvideo_init,
    /*.encode=*/NULL,
    /*.close=*/dvvideo_close,
    /*.decode=*/dvvideo_decode_frame,
    /*.capabilities=*/CODEC_CAP_DR1,
    /*.next=*/NULL,
    /*.flush=*/NULL,
    /*.supported_framerates = */NULL,
    /*.pix_fmts = */NULL,
    /*.long_name = */NULL_IF_CONFIG_SMALL("DV (Digital Video)"),
};
#endif
