/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef DEC_DTX_H
#define DEC_DTX_H

#include "typedef.h"

#define M               16      /* Order of LP filter */
#define SPEECH          0
#define DTX             1
#define D_DTX_MUTE      2
#define D_DTX_HIST_SIZE 8

typedef struct {
   Word16 mem_isf_buf[M * D_DTX_HIST_SIZE];  /* ISF vector history (8 frames)*/
   Word16 mem_isf[M];            /* ISF vector                               */
   Word16 mem_isf_prev[M];       /* Previous ISF vector                      */
   Word16 mem_log_en_buf[D_DTX_HIST_SIZE];/* logarithmic frame energy history*/
   Word16 mem_true_sid_period_inv;  /* inverse of true SID update rate       */
   Word16 mem_log_en;            /* logarithmic frame energy                 */
   Word16 mem_log_en_prev;       /* previous logarithmic frame energy        */
   Word16 mem_cng_seed;          /* Comfort noise excitation seed            */
   Word16 mem_hist_ptr;          /* index to beginning of LSF history        */
   Word16 mem_dither_seed;       /* comfort noise dithering seed             */
   Word16 mem_cn_dith;           /* background noise stationarity information*/
   Word16 mem_since_last_sid;    /* number of frames since last SID frame    */

   UWord8 mem_dec_ana_elapsed_count;/* counts elapsed speech frames after DTX*/
   UWord8 mem_dtx_global_state;  /* DTX state flags                          */
   UWord8 mem_data_updated;      /* flags CNI updates                        */
   UWord8 mem_dtx_hangover_count;/* counts down in hangover period           */
   UWord8 mem_sid_frame;         /* flags SID frames                         */
   UWord8 mem_valid_data;        /* flags SID frames containing valid data   */
   UWord8 mem_dtx_hangover_added;/* flags hangover period at end of speech   */

} D_DTX_State;

int D_DTX_init(D_DTX_State **st, const Word16 *isf_init);
int D_DTX_reset(D_DTX_State *st, const Word16 *isf_init);
void D_DTX_exit(D_DTX_State **st);
UWord8 D_DTX_rx_handler(D_DTX_State *st, UWord8 frame_type);
void D_DTX_exe(D_DTX_State *st, Word16 *exc2, Word16 new_state,
               Word16 isf[], Word16 **prms);
void D_DTX_activity_update(D_DTX_State *st, Word16 isf[], Word16 exc[]);

#endif
