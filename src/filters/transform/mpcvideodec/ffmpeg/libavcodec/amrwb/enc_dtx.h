/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_F_DTX_H
#define ENC_F_DTX_H

#include "typedef.h"

#define M               16       /* Order of LP filter                  */
#define DTX_HIST_SIZE   8        /* 8 frames                            */
#define COMPLEN         12       /* Number of sub-bands used by VAD     */
#define F_5TH_CNT       5        /* Number of 5th order filters in VAD  */
#define F_3TH_CNT       6        /* Number of 3th order filters in VAD  */

typedef struct {
   Float32 mem_isf[M * DTX_HIST_SIZE]; /* LSP history                        */
   Float32 mem_distance[28];           /* ISF history distance matrix        */
   Float32 mem_distance_sum[DTX_HIST_SIZE];/* Sum of ISF history distances   */
   Float32 mem_log_en[DTX_HIST_SIZE];  /* logarithmic frame energy history   */

   Word16 mem_hist_ptr;             /* pointer to the cyclic history vectors */
   Word16 mem_log_en_index;         /* Index for logarithmic energy          */
   Word16 mem_cng_seed;             /* Comfort noise excitation seed         */
   Word16 mem_dtx_hangover_count;   /* DTX hangover period                   */
   Word16 mem_dec_ana_elapsed_count;/* counter for elapsed speech frames     */
                                    /* in DTX                                */
} E_DTX_State;


typedef struct {

   Float64 mem_pow_sum;             /* power of previous frame               */

   Float32 mem_bckr_est[COMPLEN];   /* background noise estimate             */
   Float32 mem_ave_level[COMPLEN];  /* averaged input components             */
                                    /* for stationary estimation             */
   Float32 mem_level[COMPLEN];      /* input levels of the previous frame    */
   Float32 mem_sub_level[COMPLEN];  /* input levels calculated at the end of */
                                    /* a frame (lookahead)                   */
   Float32 mem_a_data5[F_5TH_CNT][2];/* memory for the filter bank           */
   Float32 mem_a_data3[F_3TH_CNT];  /* memory for the filter bank            */

   Float32 mem_sp_max;              /* maximum level                         */
   Float32 mem_speech_level;        /* estimated speech level                */

   Word16 mem_burst_count;          /* counts length of a speech burst       */
   Word16 mem_hang_count;           /* hangover counter                      */
   Word16 mem_stat_count;           /* stationary counter                    */
   Word16 mem_vadreg;               /* flags for intermediate VAD decisions  */
   Word16 mem_pitch_tone;           /* flags for pitch and tone detection    */
   Word16 mem_sp_est_cnt;           /* counter for speech level estimation   */
   Word16 mem_sp_max_cnt;           /* counts frames that contains speech    */

} E_DTX_Vad_State;

Word32 E_DTX_init(E_DTX_State **st);
Word32 E_DTX_reset(E_DTX_State *st);
void E_DTX_exit(E_DTX_State **st);
void E_DTX_tx_handler(E_DTX_State *st, Word32 vad_flag, Word16 *usedMode);
void E_DTX_buffer(E_DTX_State *st, Float32 isf_new[], Float32 enr,
                  Word16 codec_mode);
void E_DTX_exe(E_DTX_State *st, Float32 *exc2, Word16 **pt_prms);
Word32 E_DTX_vad_init(E_DTX_Vad_State **st);
Word32 E_DTX_vad_reset(E_DTX_Vad_State *st);
void E_DTX_vad_exit(E_DTX_Vad_State **st);
void E_DTX_pitch_tone_detection(E_DTX_Vad_State *st, Float32 p_gain);
Word16 E_DTX_vad(E_DTX_Vad_State *st, Float32 in_buf[]);

#endif

