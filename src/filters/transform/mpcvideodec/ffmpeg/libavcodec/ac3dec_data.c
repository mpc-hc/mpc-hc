/*
 * AC-3 and E-AC-3 decoder tables
 * Copyright (c) 2007 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
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
 * @file libavcodec/ac3dec_data.c
 * Tables taken directly from the AC-3 spec.
 */

#include "ac3dec_data.h"
#include "ac3.h"

/**
 * Table used to ungroup 3 values stored in 5 bits.
 * Used by bap=1 mantissas and GAQ.
 * ff_ac3_ungroup_3_in_5_bits_tab[i] = { i/9, (i%9)/3, (i%9)%3 }
 */
const uint8_t ff_ac3_ungroup_3_in_5_bits_tab[32][3] = {
    { 0, 0, 0 }, { 0, 0, 1 }, { 0, 0, 2 }, { 0, 1, 0 },
    { 0, 1, 1 }, { 0, 1, 2 }, { 0, 2, 0 }, { 0, 2, 1 },
    { 0, 2, 2 }, { 1, 0, 0 }, { 1, 0, 1 }, { 1, 0, 2 },
    { 1, 1, 0 }, { 1, 1, 1 }, { 1, 1, 2 }, { 1, 2, 0 },
    { 1, 2, 1 }, { 1, 2, 2 }, { 2, 0, 0 }, { 2, 0, 1 },
    { 2, 0, 2 }, { 2, 1, 0 }, { 2, 1, 1 }, { 2, 1, 2 },
    { 2, 2, 0 }, { 2, 2, 1 }, { 2, 2, 2 }, { 3, 0, 0 },
    { 3, 0, 1 }, { 3, 0, 2 }, { 3, 1, 0 }, { 3, 1, 1 }
};

/**
 * Table of bin locations for rematrixing bands
 * reference: Section 7.5.2 Rematrixing : Frequency Band Definitions
 */
const uint8_t ff_ac3_rematrix_band_tab[5] = { 13, 25, 37, 61, 253 };

const uint8_t ff_eac3_hebap_tab[64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
    8, 8, 9, 9, 9, 10, 10, 10, 10, 11,
    11, 11, 11, 12, 12, 12, 12, 13, 13, 13,
    13, 14, 14, 14, 14, 15, 15, 15, 15, 16,
    16, 16, 16, 17, 17, 17, 17, 18, 18, 18,
    18, 18, 18, 18, 18, 19, 19, 19, 19, 19,
    19, 19, 19, 19,
};

/**
 * Table E2.16 Default Coupling Banding Structure
 */
const uint8_t ff_eac3_default_cpl_band_struct[18] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1 };

/**
 * Table E2.15 Default Spectral Extension Banding Structure
 */
const uint8_t ff_eac3_default_spx_band_struct[17] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 };

/**
 * Table E.25: Spectral Extension Attenuation Table
 * ff_eac3_spx_atten_tab[code][bin]=pow(2.0,(bin+1)*(code+1)/-15.0);
 */
const float ff_eac3_spx_atten_tab[32][3] = {
    { 0.954841603910416503f, 0.911722488558216804f, 0.870550563296124125f },
    { 0.911722488558216804f, 0.831237896142787758f, 0.757858283255198995f },
    { 0.870550563296124125f, 0.757858283255198995f, 0.659753955386447100f },
    { 0.831237896142787758f, 0.690956439983888004f, 0.574349177498517438f },
    { 0.793700525984099792f, 0.629960524947436595f, 0.500000000000000000f },
    { 0.757858283255198995f, 0.574349177498517438f, 0.435275281648062062f },
    { 0.723634618720189082f, 0.523647061410313364f, 0.378929141627599553f },
    { 0.690956439983888004f, 0.477420801955208307f, 0.329876977693223550f },
    { 0.659753955386447100f, 0.435275281648062062f, 0.287174588749258719f },
    { 0.629960524947436595f, 0.396850262992049896f, 0.250000000000000000f },
    { 0.601512518041058319f, 0.361817309360094541f, 0.217637640824031003f },
    { 0.574349177498517438f, 0.329876977693223550f, 0.189464570813799776f },
    { 0.548412489847312945f, 0.300756259020529160f, 0.164938488846611775f },
    { 0.523647061410313364f, 0.274206244923656473f, 0.143587294374629387f },
    { 0.500000000000000000f, 0.250000000000000000f, 0.125000000000000000f },
    { 0.477420801955208307f, 0.227930622139554201f, 0.108818820412015502f },
    { 0.455861244279108402f, 0.207809474035696939f, 0.094732285406899888f },
    { 0.435275281648062062f, 0.189464570813799776f, 0.082469244423305887f },
    { 0.415618948071393879f, 0.172739109995972029f, 0.071793647187314694f },
    { 0.396850262992049896f, 0.157490131236859149f, 0.062500000000000000f },
    { 0.378929141627599553f, 0.143587294374629387f, 0.054409410206007751f },
    { 0.361817309360094541f, 0.130911765352578369f, 0.047366142703449930f },
    { 0.345478219991944002f, 0.119355200488802049f, 0.041234622211652958f },
    { 0.329876977693223550f, 0.108818820412015502f, 0.035896823593657347f },
    { 0.314980262473718298f, 0.099212565748012460f, 0.031250000000000000f },
    { 0.300756259020529160f, 0.090454327340023621f, 0.027204705103003875f },
    { 0.287174588749258719f, 0.082469244423305887f, 0.023683071351724965f },
    { 0.274206244923656473f, 0.075189064755132290f, 0.020617311105826479f },
    { 0.261823530705156682f, 0.068551561230914118f, 0.017948411796828673f },
    { 0.250000000000000000f, 0.062500000000000000f, 0.015625000000000000f },
    { 0.238710400977604098f, 0.056982655534888536f, 0.013602352551501938f },
    { 0.227930622139554201f, 0.051952368508924235f, 0.011841535675862483f }
};
