/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_MAIN_H
#define ENC_MAIN_H

#include "typedef.h"
#include "enc_dtx.h"

#define L_FRAME      256     /* Frame size                                 */
#define L_TOTAL      384     /* Total size of speech buffer.               */
#define L_FILT16k    15      /* Delay of down-sampling filter              */
#define PIT_MAX      231     /* Maximum pitch lag                          */
#define OPL_DECIM    2       /* Decimation in open-loop pitch analysis     */
#define L_INTERPOL   (16+1)  /* Length of filter for interpolation         */

typedef struct
{
   /* Float32 */
   Float32 mem_speech[L_TOTAL];        /* old speech vector at 12.8kHz       */
   Float32 mem_wsp[L_FRAME + PIT_MAX / OPL_DECIM]; /* old decimated weighted */
                                                   /* speech vector          */
   Float32 mem_hp_wsp[L_FRAME / OPL_DECIM + (PIT_MAX / OPL_DECIM)];
   Float32 mem_decim[2 * L_FILT16k];   /* speech decimated filter memory     */
   Float32 mem_hf[2 * L_FILT16k];      /* HF band-pass filter memory         */
   Float32 mem_hf2[2 * L_FILT16k];     /* HF band-pass filter memory         */
   Float32 mem_hf3[2 * L_FILT16k];     /* HF band-pass filter memory         */
   Float32 mem_isp[M];                 /* old isp (immittance spectral pairs)*/
   Float32 mem_syn[M];                 /* synthesis memory                   */
   Float32 mem_syn2[M];                /* modified synthesis memory          */
   Float32 mem_syn_hf[M];              /* HF synthesis memory                */
   Float32 mem_isf[M];                 /* old isf (frequency domain)         */
   Float32 mem_hf_wsp[9];              /* Open-loop lag gain filter memory   */
   Float32 mem_sig_in[4];              /* hp50 filter memory                 */
   Float32 mem_sig_out[4];             /* hp50 filter memory for synthesis   */
   Float32 mem_hp400[4];               /* hp400 filter memory for synthesis  */
   Float32 mem_decim2[3];              /* wsp decimation filter memory       */
   Float32 mem_gp_clip[2];             /* gain of pitch clipping memory      */
   Float32 mem_preemph;                /* speech preemph filter memory       */
   Float32 mem_deemph;                 /* speech deemph filter memory        */
   Float32 mem_wsp_df;                 /* Open-loop LTP deemph filter memory */
   Float32 mem_w0;                     /* target vector memory               */
   Float32 mem_ol_gain;                /* Open-loop gain                     */
   Float32 mem_ada_w;                  /* weighting level                    */
   Float32 mem_gc_threshold;           /* threshold for noise enhancer       */
   Float32 mem_gain_alpha;             /* Higher band gain weighting factor  */

   /* Word32 */
   Word32 mem_ol_lag[5];               /* Open loop lag history              */
   Word32 mem_T0_med;                  /* weighted open loop pitch lag       */

   /* Word16 */
   Word16 mem_exc[(L_FRAME + 1) + PIT_MAX + L_INTERPOL];
                                       /* old excitation vector              */
   Word16 mem_isp_q[M];                /* quantized old isp                  */
   Word16 mem_isf_q[M];                /* past isf quantizer                 */
   Word16 mem_gain_q[4];               /* gain quantizer memory              */
   Word16 mem_subfr_q[4];              /* old maximum scaling factor         */
   Word16 mem_tilt_code;               /* tilt of code                       */
   Word16 mem_q;                       /* old scaling factor                 */
   Word16 mem_seed;                    /* random memory for HF generation    */

   /* other */
   E_DTX_Vad_State *vadSt;
   E_DTX_State *dtx_encSt;
   UWord8 mem_first_frame;             /* First frame indicator              */
   UWord8 mem_ol_wght_flg;             /* switches lag weighting on and off  */
   UWord8 mem_vad_hist;                /* VAD history                        */

} Coder_State;

#endif

