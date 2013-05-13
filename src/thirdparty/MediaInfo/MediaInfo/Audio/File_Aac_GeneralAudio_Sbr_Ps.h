/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Aac_GeneralAudio_Sbr_PsH
#define MediaInfo_File_Aac_GeneralAudio_Sbr_PsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aac.h"


//---------------------------------------------------------------------------

namespace MediaInfoLib
{

typedef float complex_t[2];
typedef complex_t qmf_t;
#define MAX_PS_ENVELOPES 5
#define NO_ALLPASS_LINKS 3

struct hyb_info
{
    int8u  frame_len;
    int8u  resolution20[3];
    int8u  resolution34[5];

    qmf_t *work;
    qmf_t **buffer;
    qmf_t **temp;

    hyb_info(int8u numTimeSlotsRate);
    ~hyb_info();
};

struct ps_info
{
    //ps_data
    bool   enable_ps_header;
    bool   enable_iid;
    bool   enable_icc;
    bool   enable_ext;
    int8u  iid_mode;
    int8u  nr_iid_par;
    int8u  nr_ipdopd_par;
    int8u  icc_mode;
    int8u  nr_icc_par;
    bool   frame_class;
    int8u  num_env;

    int8u  border_position[MAX_PS_ENVELOPES+1];

    int8u  iid_dt[MAX_PS_ENVELOPES];
    int8u  icc_dt[MAX_PS_ENVELOPES];

    int8u  enable_ipdopd;
    int8u  ipd_mode;
    int8u  ipd_dt[MAX_PS_ENVELOPES];
    int8u  opd_dt[MAX_PS_ENVELOPES];

    /* indices */
    int8s  iid_index_prev[34];
    int8s  icc_index_prev[34];
    int8s  ipd_index_prev[17];
    int8s  opd_index_prev[17];
    int8s  iid_index[MAX_PS_ENVELOPES][34];
    int8s  icc_index[MAX_PS_ENVELOPES][34];
    int8s  ipd_index[MAX_PS_ENVELOPES][17];
    int8s  opd_index[MAX_PS_ENVELOPES][17];

    int8s  ipd_index_1[17];
    int8s  opd_index_1[17];
    int8s  ipd_index_2[17];
    int8s  opd_index_2[17];

    /* ps data was correctly read */
    int8u  ps_data_available;

    /* a header has been read */
    int8u  header_read;

    /* hybrid filterbank parameters */
    hyb_info *hyb;
    int8u  use34hybrid_bands;
    int8u  numTimeSlotsRate;

    /**/
    int8u  num_groups;
    int8u  num_hybrid_groups;
    int8u  nr_par_bands;
    int8u  nr_allpass_bands;
    int8u  decay_cutoff;

    int8u  *group_border;
    int16u *map_group2bk;

    /* filter delay handling */
    int8u  saved_delay;
    int8u  delay_buf_index_ser[NO_ALLPASS_LINKS];
    int8u  num_sample_delay_ser[NO_ALLPASS_LINKS];
    int8u  delay_D[64];
    int8u  delay_buf_index_delay[64];

    complex_t delay_Qmf[14][64]; /* 14 samples delay max, 64 QMF channels */
    complex_t delay_SubQmf[2][32]; /* 2 samples delay max (SubQmf is always allpass filtered) */
    complex_t delay_Qmf_ser[NO_ALLPASS_LINKS][5][64]; /* 5 samples delay max (table 8.34), 64 QMF channels */
    complex_t delay_SubQmf_ser[NO_ALLPASS_LINKS][5][32]; /* 5 samples delay max (table 8.34) */

    /* transients */
    float alpha_decay;
    float alpha_smooth;

    float P_PeakDecayNrg[34];
    float P_prev[34];
    float P_SmoothPeakDecayDiffNrg_prev[34];

    /* mixing and phase */
    complex_t h11_prev[50];
    complex_t h12_prev[50];
    complex_t h21_prev[50];
    complex_t h22_prev[50];
    int8u  phase_hist;
    complex_t ipd_prev[20][2];
    complex_t opd_prev[20][2];

    ps_info(int8u sr_index, int8u numTimeSlotsRate);
    ~ps_info();
};

} //NameSpace

#endif
