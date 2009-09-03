/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef DEC_MAIN_H
#define DEC_MAIN_H

#include "typedef.h"
#include "dec_dtx.h"

#define L_FRAME      256      /* Frame size                          */
#define PIT_MAX      231      /* Maximum pitch lag                   */
#define L_INTERPOL   (16 + 1) /* Length of filter for interpolation  */
#define L_MEANBUF    3
#define L_FILT       12       /* Delay of up-sampling filter         */
#define L_FILT16k    15       /* Delay of down-sampling filter       */
#define M16k         20       /* Order of LP filter                  */

typedef struct
{
   Word32 mem_gc_thres;             /* threshold for noise enhancer        */
   Word16 mem_exc[(L_FRAME + 1) + PIT_MAX + L_INTERPOL];/* old excitation vector */
   Word16 mem_isf_buf[L_MEANBUF * M];/* isf buffer(frequency domain)        */
   Word16 mem_hf[2 * L_FILT16k];    /* HF band-pass filter memory          */
   Word16 mem_hf2[2 * L_FILT16k];   /* HF band-pass filter memory          */
   Word16 mem_hf3[2 * L_FILT16k];   /* HF band-pass filter memory          */
   Word16 mem_oversamp[2 * L_FILT]; /* synthesis oversampled filter memory */
   Word16 mem_gain[23];             /* gain decoder memory                 */
   Word16 mem_syn_hf[M16k];         /* HF synthesis memory                 */
   Word16 mem_isp[M];               /* old isp (immittance spectral pairs) */
   Word16 mem_isf[M];               /* old isf (frequency domain)          */
   Word16 mem_isf_q[M];             /* past isf quantizer                  */
   Word16 mem_syn_hi[M];            /* modified synthesis memory (MSB)     */
   Word16 mem_syn_lo[M];            /* modified synthesis memory (LSB)     */
   Word16 mem_ph_disp[8];           /* phase dispersion memory             */
   Word16 mem_sig_out[6];           /* hp50 filter memory for synthesis    */
   Word16 mem_hp400[6];             /* hp400 filter memory for synthesis   */
   Word16 mem_lag[5];               /* LTP lag history                     */
   Word16 mem_subfr_q[4];           /* old maximum scaling factor          */
   Word16 mem_tilt_code;            /* tilt of code                        */
   Word16 mem_q;                    /* old scaling factor                  */
   Word16 mem_deemph;               /* speech deemph filter memory         */
   Word16 mem_seed;                 /* random memory for frame erasure     */
   Word16 mem_seed2;                /* random memory for HF generation     */
   Word16 mem_seed3;                /* random memory for lag concealment   */
   Word16 mem_T0;                   /* old pitch lag                       */
   Word16 mem_T0_frac;              /* old pitch fraction lag              */
   UWord16 mem_vad_hist;            /* VAD history                         */
   D_DTX_State *dtx_decSt;
   UWord8 mem_bfi;                  /* Previous BFI                        */
   UWord8 mem_state;                /* BGH state machine memory            */
   UWord8 mem_first_frame;          /* First frame indicator               */

} Decoder_State;

#endif

