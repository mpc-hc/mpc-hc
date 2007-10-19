/*
 * MPEG Audio decoder
 * Copyright (c) 2001, 2002 Fabrice Bellard.
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
 * @file mpegaudiodec.c
 * MPEG Audio decoder.
 */

//#define DEBUG
#include "avcodec.h"
#include "bitstream.h"
#include "dsputil.h"

/* define USE_HIGHPRECISION to have a bit exact (but slower) mpeg
   audio decoder */
#ifdef CONFIG_MPEGAUDIO_HP
#   define USE_HIGHPRECISION
#endif

#include "mpegaudio.h"

#include "mathops.h"

#define FIXHR(a) ((int)((a) * (1LL<<32) + 0.5))

/****************/

#include "mpegaudiodata.h"

static DECLARE_ALIGNED_16(MPA_INT, window[512]);

/* tab[i][j] = 1.0 / (2.0 * cos(pi*(2*k+1) / 2^(6 - j))) */

/* cos(i*pi/64) */

#define COS0_0  FIXHR(0.50060299823519630134/2)
#define COS0_1  FIXHR(0.50547095989754365998/2)
#define COS0_2  FIXHR(0.51544730992262454697/2)
#define COS0_3  FIXHR(0.53104259108978417447/2)
#define COS0_4  FIXHR(0.55310389603444452782/2)
#define COS0_5  FIXHR(0.58293496820613387367/2)
#define COS0_6  FIXHR(0.62250412303566481615/2)
#define COS0_7  FIXHR(0.67480834145500574602/2)
#define COS0_8  FIXHR(0.74453627100229844977/2)
#define COS0_9  FIXHR(0.83934964541552703873/2)
#define COS0_10 FIXHR(0.97256823786196069369/2)
#define COS0_11 FIXHR(1.16943993343288495515/4)
#define COS0_12 FIXHR(1.48416461631416627724/4)
#define COS0_13 FIXHR(2.05778100995341155085/8)
#define COS0_14 FIXHR(3.40760841846871878570/8)
#define COS0_15 FIXHR(10.19000812354805681150/32)

#define COS1_0 FIXHR(0.50241928618815570551/2)
#define COS1_1 FIXHR(0.52249861493968888062/2)
#define COS1_2 FIXHR(0.56694403481635770368/2)
#define COS1_3 FIXHR(0.64682178335999012954/2)
#define COS1_4 FIXHR(0.78815462345125022473/2)
#define COS1_5 FIXHR(1.06067768599034747134/4)
#define COS1_6 FIXHR(1.72244709823833392782/4)
#define COS1_7 FIXHR(5.10114861868916385802/16)

#define COS2_0 FIXHR(0.50979557910415916894/2)
#define COS2_1 FIXHR(0.60134488693504528054/2)
#define COS2_2 FIXHR(0.89997622313641570463/2)
#define COS2_3 FIXHR(2.56291544774150617881/8)

#define COS3_0 FIXHR(0.54119610014619698439/2)
#define COS3_1 FIXHR(1.30656296487637652785/4)

#define COS4_0 FIXHR(0.70710678118654752439/2)

/* butterfly operator */
#define BF(a, b, c, s)\
{\
    tmp0 = tab[a] + tab[b];\
    tmp1 = tab[a] - tab[b];\
    tab[a] = tmp0;\
    tab[b] = MULH(tmp1<<(s), c);\
}

#define BF1(a, b, c, d)\
{\
    BF(a, b, COS4_0, 1);\
    BF(c, d,-COS4_0, 1);\
    tab[c] += tab[d];\
}

#define BF2(a, b, c, d)\
{\
    BF(a, b, COS4_0, 1);\
    BF(c, d,-COS4_0, 1);\
    tab[c] += tab[d];\
    tab[a] += tab[c];\
    tab[c] += tab[b];\
    tab[b] += tab[d];\
}

#define ADD(a, b) tab[a] += tab[b]

/* DCT32 without 1/sqrt(2) coef zero scaling. */
static void dct32(int32_t *out, int32_t *tab)
{
    int tmp0, tmp1;

    /* pass 1 */
    BF( 0, 31, COS0_0 , 1);
    BF(15, 16, COS0_15, 5);
    /* pass 2 */
    BF( 0, 15, COS1_0 , 1);
    BF(16, 31,-COS1_0 , 1);
    /* pass 1 */
    BF( 7, 24, COS0_7 , 1);
    BF( 8, 23, COS0_8 , 1);
    /* pass 2 */
    BF( 7,  8, COS1_7 , 4);
    BF(23, 24,-COS1_7 , 4);
    /* pass 3 */
    BF( 0,  7, COS2_0 , 1);
    BF( 8, 15,-COS2_0 , 1);
    BF(16, 23, COS2_0 , 1);
    BF(24, 31,-COS2_0 , 1);
    /* pass 1 */
    BF( 3, 28, COS0_3 , 1);
    BF(12, 19, COS0_12, 2);
    /* pass 2 */
    BF( 3, 12, COS1_3 , 1);
    BF(19, 28,-COS1_3 , 1);
    /* pass 1 */
    BF( 4, 27, COS0_4 , 1);
    BF(11, 20, COS0_11, 2);
    /* pass 2 */
    BF( 4, 11, COS1_4 , 1);
    BF(20, 27,-COS1_4 , 1);
    /* pass 3 */
    BF( 3,  4, COS2_3 , 3);
    BF(11, 12,-COS2_3 , 3);
    BF(19, 20, COS2_3 , 3);
    BF(27, 28,-COS2_3 , 3);
    /* pass 4 */
    BF( 0,  3, COS3_0 , 1);
    BF( 4,  7,-COS3_0 , 1);
    BF( 8, 11, COS3_0 , 1);
    BF(12, 15,-COS3_0 , 1);
    BF(16, 19, COS3_0 , 1);
    BF(20, 23,-COS3_0 , 1);
    BF(24, 27, COS3_0 , 1);
    BF(28, 31,-COS3_0 , 1);



    /* pass 1 */
    BF( 1, 30, COS0_1 , 1);
    BF(14, 17, COS0_14, 3);
    /* pass 2 */
    BF( 1, 14, COS1_1 , 1);
    BF(17, 30,-COS1_1 , 1);
    /* pass 1 */
    BF( 6, 25, COS0_6 , 1);
    BF( 9, 22, COS0_9 , 1);
    /* pass 2 */
    BF( 6,  9, COS1_6 , 2);
    BF(22, 25,-COS1_6 , 2);
    /* pass 3 */
    BF( 1,  6, COS2_1 , 1);
    BF( 9, 14,-COS2_1 , 1);
    BF(17, 22, COS2_1 , 1);
    BF(25, 30,-COS2_1 , 1);

    /* pass 1 */
    BF( 2, 29, COS0_2 , 1);
    BF(13, 18, COS0_13, 3);
    /* pass 2 */
    BF( 2, 13, COS1_2 , 1);
    BF(18, 29,-COS1_2 , 1);
    /* pass 1 */
    BF( 5, 26, COS0_5 , 1);
    BF(10, 21, COS0_10, 1);
    /* pass 2 */
    BF( 5, 10, COS1_5 , 2);
    BF(21, 26,-COS1_5 , 2);
    /* pass 3 */
    BF( 2,  5, COS2_2 , 1);
    BF(10, 13,-COS2_2 , 1);
    BF(18, 21, COS2_2 , 1);
    BF(26, 29,-COS2_2 , 1);
    /* pass 4 */
    BF( 1,  2, COS3_1 , 2);
    BF( 5,  6,-COS3_1 , 2);
    BF( 9, 10, COS3_1 , 2);
    BF(13, 14,-COS3_1 , 2);
    BF(17, 18, COS3_1 , 2);
    BF(21, 22,-COS3_1 , 2);
    BF(25, 26, COS3_1 , 2);
    BF(29, 30,-COS3_1 , 2);

    /* pass 5 */
    BF1( 0,  1,  2,  3);
    BF2( 4,  5,  6,  7);
    BF1( 8,  9, 10, 11);
    BF2(12, 13, 14, 15);
    BF1(16, 17, 18, 19);
    BF2(20, 21, 22, 23);
    BF1(24, 25, 26, 27);
    BF2(28, 29, 30, 31);

    /* pass 6 */

    ADD( 8, 12);
    ADD(12, 10);
    ADD(10, 14);
    ADD(14,  9);
    ADD( 9, 13);
    ADD(13, 11);
    ADD(11, 15);

    out[ 0] = tab[0];
    out[16] = tab[1];
    out[ 8] = tab[2];
    out[24] = tab[3];
    out[ 4] = tab[4];
    out[20] = tab[5];
    out[12] = tab[6];
    out[28] = tab[7];
    out[ 2] = tab[8];
    out[18] = tab[9];
    out[10] = tab[10];
    out[26] = tab[11];
    out[ 6] = tab[12];
    out[22] = tab[13];
    out[14] = tab[14];
    out[30] = tab[15];

    ADD(24, 28);
    ADD(28, 26);
    ADD(26, 30);
    ADD(30, 25);
    ADD(25, 29);
    ADD(29, 27);
    ADD(27, 31);

    out[ 1] = tab[16] + tab[24];
    out[17] = tab[17] + tab[25];
    out[ 9] = tab[18] + tab[26];
    out[25] = tab[19] + tab[27];
    out[ 5] = tab[20] + tab[28];
    out[21] = tab[21] + tab[29];
    out[13] = tab[22] + tab[30];
    out[29] = tab[23] + tab[31];
    out[ 3] = tab[24] + tab[20];
    out[19] = tab[25] + tab[21];
    out[11] = tab[26] + tab[22];
    out[27] = tab[27] + tab[23];
    out[ 7] = tab[28] + tab[18];
    out[23] = tab[29] + tab[19];
    out[15] = tab[30] + tab[17];
    out[31] = tab[31];
}

#if FRAC_BITS <= 15

static inline int round_sample(int *sum)
{
    int sum1;
    sum1 = (*sum) >> OUT_SHIFT;
    *sum &= (1<<OUT_SHIFT)-1;
    if (sum1 < OUT_MIN)
        sum1 = OUT_MIN;
    else if (sum1 > OUT_MAX)
        sum1 = OUT_MAX;
    return sum1;
}

/* signed 16x16 -> 32 multiply add accumulate */
#define MACS(rt, ra, rb) MAC16(rt, ra, rb)

/* signed 16x16 -> 32 multiply */
#define MULS(ra, rb) MUL16(ra, rb)

#else

static inline int round_sample(int64_t *sum)
{
    int sum1;
    sum1 = (int)((*sum) >> OUT_SHIFT);
    *sum &= (1<<OUT_SHIFT)-1;
    if (sum1 < OUT_MIN)
        sum1 = OUT_MIN;
    else if (sum1 > OUT_MAX)
        sum1 = OUT_MAX;
    return sum1;
}

#   define MULS(ra, rb) MUL64(ra, rb)
#endif

#define SUM8(sum, op, w, p) \
{                                               \
    sum op MULS((w)[0 * 64], p[0 * 64]);\
    sum op MULS((w)[1 * 64], p[1 * 64]);\
    sum op MULS((w)[2 * 64], p[2 * 64]);\
    sum op MULS((w)[3 * 64], p[3 * 64]);\
    sum op MULS((w)[4 * 64], p[4 * 64]);\
    sum op MULS((w)[5 * 64], p[5 * 64]);\
    sum op MULS((w)[6 * 64], p[6 * 64]);\
    sum op MULS((w)[7 * 64], p[7 * 64]);\
}

#define SUM8P2(sum1, op1, sum2, op2, w1, w2, p) \
{                                               \
    int tmp;\
    tmp = p[0 * 64];\
    sum1 op1 MULS((w1)[0 * 64], tmp);\
    sum2 op2 MULS((w2)[0 * 64], tmp);\
    tmp = p[1 * 64];\
    sum1 op1 MULS((w1)[1 * 64], tmp);\
    sum2 op2 MULS((w2)[1 * 64], tmp);\
    tmp = p[2 * 64];\
    sum1 op1 MULS((w1)[2 * 64], tmp);\
    sum2 op2 MULS((w2)[2 * 64], tmp);\
    tmp = p[3 * 64];\
    sum1 op1 MULS((w1)[3 * 64], tmp);\
    sum2 op2 MULS((w2)[3 * 64], tmp);\
    tmp = p[4 * 64];\
    sum1 op1 MULS((w1)[4 * 64], tmp);\
    sum2 op2 MULS((w2)[4 * 64], tmp);\
    tmp = p[5 * 64];\
    sum1 op1 MULS((w1)[5 * 64], tmp);\
    sum2 op2 MULS((w2)[5 * 64], tmp);\
    tmp = p[6 * 64];\
    sum1 op1 MULS((w1)[6 * 64], tmp);\
    sum2 op2 MULS((w2)[6 * 64], tmp);\
    tmp = p[7 * 64];\
    sum1 op1 MULS((w1)[7 * 64], tmp);\
    sum2 op2 MULS((w2)[7 * 64], tmp);\
}

void ff_mpa_synth_init(MPA_INT *window)
{
    int i;

    /* max = 18760, max sum over all 16 coefs : 44736 */
    for(i=0;i<257;i++) {
        int v;
        v = ff_mpa_enwindow[i];
#if WFRAC_BITS < 16
        v = (v + (1 << (16 - WFRAC_BITS - 1))) >> (16 - WFRAC_BITS);
#endif
        window[i] = v;
        if ((i & 63) != 0)
            v = -v;
        if (i != 0)
            window[512 - i] = v;
    }
}

/* 32 sub band synthesis filter. Input: 32 sub band samples, Output:
   32 samples. */
/* XXX: optimize by avoiding ring buffer usage */
void ff_mpa_synth_filter(MPA_INT *synth_buf_ptr, int *synth_buf_offset,
                         MPA_INT *window, int *dither_state,
                         OUT_INT *samples, int incr,
                         int32_t sb_samples[SBLIMIT])
{
    int32_t tmp[32];
    register MPA_INT *synth_buf;
    register const MPA_INT *w, *w2, *p;
    int j, offset, v;
    OUT_INT *samples2;
#if FRAC_BITS <= 15
    int sum, sum2;
#else
    int64_t sum, sum2;
#endif

    dct32(tmp, sb_samples);

    offset = *synth_buf_offset;
    synth_buf = synth_buf_ptr + offset;

    for(j=0;j<32;j++) {
        v = tmp[j];
#if FRAC_BITS <= 15
        /* NOTE: can cause a loss in precision if very high amplitude
           sound */
        if (v > 32767)
            v = 32767;
        else if (v < -32768)
            v = -32768;
#endif
        synth_buf[j] = v;
    }
    /* copy to avoid wrap */
    memcpy(synth_buf + 512, synth_buf, 32 * sizeof(MPA_INT));

    samples2 = samples + 31 * incr;
    w = window;
    w2 = window + 31;

    sum = *dither_state;
    p = synth_buf + 16;
    SUM8(sum, +=, w, p);
    p = synth_buf + 48;
    SUM8(sum, -=, w + 32, p);
    *samples = round_sample(&sum);
    samples += incr;
    w++;

    /* we calculate two samples at the same time to avoid one memory
       access per two sample */
    for(j=1;j<16;j++) {
        sum2 = 0;
        p = synth_buf + 16 + j;
        SUM8P2(sum, +=, sum2, -=, w, w2, p);
        p = synth_buf + 48 - j;
        SUM8P2(sum, -=, sum2, -=, w + 32, w2 + 32, p);

        *samples = round_sample(&sum);
        samples += incr;
        sum += sum2;
        *samples2 = round_sample(&sum);
        samples2 -= incr;
        w++;
        w2--;
    }

    p = synth_buf + 32;
    SUM8(sum, -=, w + 32, p);
    *samples = round_sample(&sum);
    *dither_state= sum;

    offset = (offset - 32) & 511;
    *synth_buf_offset = offset;
}
