/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>

#include "enc_dtx.h"
#include "enc_acelp.h"
#include "enc_lpc.h"
#include "enc_main.h"
#include "enc_gain.h"
#include "enc_util.h"

#ifdef WIN32
#pragma warning( disable : 4310)
#endif

#include "typedef.h"

#define MAX_16       (Word16)0x7fff
#define MIN_16       (Word16)0x8000
#define Q_MAX        8        /* scaling max for signal                 */
#define PREEMPH_FAC  0.68F    /* preemphasis factor                     */
#define GAMMA1       0.92F    /* Weighting factor (numerator)           */
#define TILT_FAC     0.68F    /* tilt factor (denominator)              */
#define PIT_MIN      34       /* Minimum pitch lag with resolution 1/4  */
#define PIT_FR2      128      /* Minimum pitch lag with resolution 1/2  */
#define PIT_FR1_9b   160      /* Minimum pitch lag with resolution 1    */
#define PIT_FR1_8b   92       /* Minimum pitch lag with resolution 1    */
#define PIT_MAX      231      /* Maximum pitch lag                      */
#define L_INTERPOL   (16+1)   /* Length of filter for interpolation     */
#define L_FRAME16k   320      /* Frame size at 16kHz                    */
#define L_SUBFR      64       /* Subframe size                          */
#define NB_SUBFR     4        /* Number of subframe per frame           */
#define L_FILT       12       /* Delay of up-sampling filter            */
#define L_NEXT       64       /* Overhead in LP analysis                */
#define MODE_7k      0        /* modes                                  */
#define MODE_9k      1
#define MODE_12k     2
#define MODE_14k     3
#define MODE_16k     4
#define MODE_18k     5
#define MODE_20k     6
#define MODE_23k     7
#define MODE_24k     8
#define MRDTX        9

extern const Word16 E_ROM_isp[];
extern const Word16 E_ROM_isf[];
extern const Word16 E_ROM_interpol_frac[];

/*
 * E_MAIN_reset
 *
 * Parameters:
 *    st        I/O: pointer to state structure
 *    reset_all   I: perform full reset
 *
 * Function:
 *    Initialisation of variables for the coder section.
 *
 *
 * Returns:
 *    void
 */
void E_MAIN_reset(void *st, Word16 reset_all)
{
   Word32 i;

   Coder_State *cod_state;

   cod_state = (Coder_State *) st;

   memset(cod_state->mem_exc, 0, (PIT_MAX + L_INTERPOL) * sizeof(Word16));
   memset(cod_state->mem_isf_q, 0, M * sizeof(Word16));
   memset(cod_state->mem_syn, 0, M * sizeof(Float32));

   cod_state->mem_w0 = 0.0F;
   cod_state->mem_tilt_code = 0;
   cod_state->mem_first_frame = 1;

   E_GAIN_clip_init(cod_state->mem_gp_clip);

   cod_state->mem_gc_threshold = 0.0F;

   if (reset_all != 0)
   {
      /* Set static vectors to zero */
      memset(cod_state->mem_speech, 0, (L_TOTAL - L_FRAME) * sizeof(Float32));
      memset(cod_state->mem_wsp, 0, (PIT_MAX / OPL_DECIM) * sizeof(Float32));
      memset(cod_state->mem_decim2, 0, 3 * sizeof(Float32));

      /* routines initialization */

      memset(cod_state->mem_decim, 0, 2 * L_FILT16k * sizeof(Float32));
      memset(cod_state->mem_sig_in, 0, 4 * sizeof(Float32));
      E_ACELP_Gain2_Q_init(cod_state->mem_gain_q);
      memset(cod_state->mem_hf_wsp, 0, 8 * sizeof(Float32));

      /* isp initialization */
      for (i = 0; i < M - 1; i++)
      {
         cod_state->mem_isp[i] =
            (Float32)cos(3.141592654 * (Float32)(i + 1) / (Float32)M);
      }

      cod_state->mem_isp[M - 1] = 0.045F;
      memcpy(cod_state->mem_isp_q, E_ROM_isp, M * sizeof(Word16));

      /* variable initialization */
      cod_state->mem_preemph = 0.0F;
      cod_state->mem_wsp_df = 0.0F;
      cod_state->mem_q = Q_MAX;
      cod_state->mem_subfr_q[3] = Q_MAX;
      cod_state->mem_subfr_q[2] = Q_MAX;
      cod_state->mem_subfr_q[1] = Q_MAX;
      cod_state->mem_subfr_q[0] = Q_MAX;
      cod_state->mem_ada_w = 0.0F;
      cod_state->mem_ol_gain = 0.0F;
      cod_state->mem_ol_wght_flg = 0;

      for (i = 0; i < 5; i++)
      {
         cod_state->mem_ol_lag[i] = 40;
      }

      cod_state->mem_T0_med = 40;
      memset(cod_state->mem_hp_wsp, 0,
         ( ( L_FRAME / 2 ) / OPL_DECIM + ( PIT_MAX / OPL_DECIM ) )
         * sizeof(Float32) );

      memset(cod_state->mem_syn_hf, 0, M * sizeof(Float32));
      memset(cod_state->mem_syn2, 0, M * sizeof(Float32));

      memset(cod_state->mem_hp400, 0, 4 * sizeof(Float32));
      memset(cod_state->mem_sig_out, 0, 4 * sizeof(Float32));

      memset(cod_state->mem_hf, 0, 2 * L_FILT16k * sizeof(Float32));
      memset(cod_state->mem_hf2, 0, 2 * L_FILT16k * sizeof(Float32));
      memset(cod_state->mem_hf3, 0, 2 * L_FILT16k * sizeof(Float32));

      memcpy(cod_state->mem_isf, E_ROM_isf, M * sizeof(Float32));

      cod_state->mem_deemph = 0.0F;

      cod_state->mem_seed = 21845;
      cod_state->mem_gain_alpha = 1.0F;
      cod_state->mem_vad_hist = 0;

      E_DTX_reset(cod_state->dtx_encSt);
      E_DTX_vad_reset(cod_state->vadSt);
   }
}

/*
 * E_MAIN_init
 *
 * Parameters:
 *    spe_state         I/O: pointer to state structure
 *
 * Function:
 *    Initialisation of variables for the coder section.
 *    Memory allocation.
 *
 * Returns:
 *    void
 */
Word16 E_MAIN_init(void **spe_state)
{
   Coder_State *st;

   *spe_state = NULL;

   /* allocate memory */
   if ((st = (Coder_State *) malloc(sizeof(Coder_State))) == NULL)
   {
      return(-1);
   }

   st->vadSt = NULL;
   st->dtx_encSt = NULL;

   E_DTX_init(&(st->dtx_encSt));
   E_DTX_vad_init(&(st->vadSt));

   E_MAIN_reset((void *) st, 1);

   *spe_state = (void*)st;

   return(0);
}

/*
 * E_MAIN_close
 *
 *
 * Parameters:
 *    spe_state   I: pointer to state structure
 *
 * Function:
 *    Free coder memory.
 *
 *
 * Returns:
 *    void
 */
void E_MAIN_close(void **spe_state)
{
   E_DTX_exit(&( ( (Coder_State *)(*spe_state) )->dtx_encSt));
   E_DTX_vad_exit(&( ( (Coder_State *) (*spe_state) )->vadSt));
   free(*spe_state);

   return;
}

/*
 * E_MAIN_parm_store
 *
 * Parameters:
 *    value       I: parameter value
 *    prms        O: output parameters
 *
 * Function:
 *    Store parameter values
 *
 * Returns:
 *    void
 */
static void E_MAIN_parm_store(Word32 value, Word16 **prms)
{
   **prms = (Word16)value;
   (*prms)++;
   return;
}


/*
 * E_MAIN_encode
 *
 * Parameters:
 *    mode        I: used mode
 *    input_sp    I: 320 new speech samples (at 16 kHz)
 *    prms        O: output parameters
 *    spe_state   B: state structure
 *    allow_dtx   I: DTX ON/OFF
 *
 * Function:
 *    Main coder routine.
 *
 * Returns:
 *    void
 */
Word16 E_MAIN_encode(Word16 * mode, Word16 speech16k[], Word16 prms[],
                    void *spe_state, Word16 allow_dtx)
{

   /* Float32 */
   Float32 f_speech16k[L_FRAME16k];    /* Speech vector                          */
   Float32 f_old_exc[(L_FRAME + 1) + PIT_MAX + L_INTERPOL]; /* Excitation vector */
   Float32 f_exc2[L_FRAME];            /* excitation vector                      */
   Float32 error[M + L_SUBFR];         /* error of quantization                  */
   Float32 A[NB_SUBFR * (M + 1)];      /* A(z) unquantized for the 4 subframes   */
   Float32 Aq[NB_SUBFR * (M + 1)];     /* A(z)   quantized for the 4 subframes   */
   Float32 xn[L_SUBFR];                /* Target vector for pitch search         */
   Float32 xn2[L_SUBFR];               /* Target vector for codebook search      */
   Float32 dn[L_SUBFR];                /* Correlation between xn2 and h1         */
   Float32 cn[L_SUBFR];                /* Target vector in residual domain       */
   Float32 h1[L_SUBFR];                /* Impulse response vector                */
   Float32 f_code[L_SUBFR];            /* Fixed codebook excitation              */
   Float32 y1[L_SUBFR];                /* Filtered adaptive excitation           */
   Float32 y2[L_SUBFR];                /* Filtered adaptive excitation           */
   Float32 synth[L_SUBFR];             /* 12.8kHz synthesis vector               */
   Float32 r[M + 1];                   /* Autocorrelations of windowed speech    */
   Float32 Ap[M + 1];                  /* A(z) with spectral expansion           */
   Float32 ispnew[M];                  /* immittance spectral pairs at 4nd sfr   */
   Float32 isf[M];                     /* ISF (frequency domain) at 4nd sfr      */
   Float32 g_coeff[5], g_coeff2[2];    /* Correlations                           */
   Float32 gain_pit;
   Float32 f_tmp, gain1, gain2;
   Float32 stab_fac = 0.0F, fac;
   Float32 *new_speech, *speech;       /* Speech vector                          */
   Float32 *wsp;                       /* Weighted speech vector                 */
   Float32 *f_exc;                     /* Excitation vector                      */
   Float32 *p_A, *p_Aq;                /* ptr to A(z) for the 4 subframes        */
   Float32 *f_pt_tmp;

   /* Word32 */
   Word32 indice[8];                   /* quantization indices                   */
   Word32 vad_flag, clip_gain;
   Word32 T_op, T_op2, T0, T0_frac;
   Word32 T0_min, T0_max;
   Word32 voice_fac, Q_new = 0;
   Word32 L_gain_code, l_tmp;
   Word32 i, i_subfr, pit_flag;

   /* Word16 */
   Word16 exc2[L_FRAME];               /* excitation vector                      */
   Word16 s_Aq[NB_SUBFR * (M + 1)];    /* A(z) quantized for the 4 subframes     */
   Word16 s_code[L_SUBFR];             /* Fixed codebook excitation              */
   Word16 ispnew_q[M];                 /* quantized ISPs at 4nd subframe         */
   Word16 isfq[M];                     /* quantized ISPs                         */
   Word16 select, codec_mode;
   Word16 index;
   Word16 s_gain_pit, gain_code;
   Word16 s_tmp, s_max;
   Word16 corr_gain;
   Word16 *exc;                        /* Excitation vector                      */

   /* Other */
   Coder_State *st;                    /* Coder states                           */

   st = (Coder_State *)spe_state;
   codec_mode = *mode;

   /*
    * Initialize pointers to speech vector.
    *
    *
    *         |-------|-------|-------|-------|-------|-------|
    *          past sp   sf1     sf2     sf3     sf4    L_NEXT
    *         <-------  Total speech buffer (L_TOTAL)   ------>
    *   old_speech
    *         <-------  LPC analysis window (L_WINDOW)  ------>
    *                 <-- present frame (L_FRAME) ---->
    *                 |       <----- new speech (L_FRAME) ---->
    *                 |       |
    *               speech    |
    *                      new_speech
    */

   new_speech = st->mem_speech + L_TOTAL - L_FRAME - L_FILT;     /* New speech     */
   speech     = st->mem_speech + L_TOTAL - L_FRAME - L_NEXT;     /* Present frame  */

   exc = st->mem_exc + PIT_MAX + L_INTERPOL;
   f_exc = f_old_exc + PIT_MAX + L_INTERPOL;
   wsp = st->mem_wsp + (PIT_MAX / OPL_DECIM);

   for(i = 0; i < L_FRAME16k; i++)
   {
      f_speech16k[i] = (Float32)speech16k[i];
   }

   Q_new = -st->mem_q;

   for(i = 0; i < (PIT_MAX + L_INTERPOL); i++)
   {
      f_old_exc[i] = (Float32)(st->mem_exc[i] * pow(2, Q_new));
   }

   /*
    * Down sampling signal from 16kHz to 12.8kHz
    */
   E_UTIL_decim_12k8(f_speech16k, L_FRAME16k, new_speech, st->mem_decim);

   /* decimate with zero-padding to avoid delay of filter */
   memcpy(f_code, st->mem_decim, 2 * L_FILT16k * sizeof(Float32));
   memset(error, 0, L_FILT16k * sizeof(Float32));

   E_UTIL_decim_12k8(error, L_FILT16k, new_speech + L_FRAME, f_code);

   /*
    * Perform 50Hz HP filtering of input signal.
    * Perform fixed preemphasis through 1 - g z^-1
    */
   E_UTIL_hp50_12k8(new_speech, L_FRAME, st->mem_sig_in);

   memcpy(f_code, st->mem_sig_in,  4 * sizeof(Float32) );

   E_UTIL_hp50_12k8(new_speech + L_FRAME, L_FILT, f_code);

   E_UTIL_f_preemph(new_speech, PREEMPH_FAC, L_FRAME, &(st->mem_preemph));

   /* last L_FILT samples for autocorrelation window */
   f_tmp = st->mem_preemph;
   E_UTIL_f_preemph(new_speech + L_FRAME, PREEMPH_FAC, L_FILT, &f_tmp);

   /*
    *  Call VAD
    *  Preemphesis scale down signal in low frequency and keep dynamic in HF.
    *  Vad work slightly in futur (new_speech = speech + L_NEXT - L_FILT).
    */

   vad_flag = E_DTX_vad(st->vadSt, new_speech);

   if (vad_flag == 0)
   {
      st->mem_vad_hist = 1;
   }
   else
   {
      st->mem_vad_hist = 0;
   }

   /* DTX processing */
   if (allow_dtx)
   {
      /* Note that mode may change here */
      E_DTX_tx_handler(st->dtx_encSt, vad_flag, mode);
   }
   else
   {
      E_DTX_reset(st->dtx_encSt);
   }

   if(*mode != MRDTX)
   {
      E_MAIN_parm_store(vad_flag, &prms);
   }

   /*
    *  Perform LPC analysis
    *  --------------------
    *   - autocorrelation + lag windowing
    *   - Levinson-durbin algorithm to find a[]
    *   - convert a[] to isp[]
    *   - convert isp[] to isf[] for quantization
    *   - quantize and code the isf[]
    *   - convert isf[] to isp[] for interpolation
    *   - find the interpolated isps and convert to a[] for the 4 subframes
    */

   /* LP analysis centered at 3nd subframe */
   E_UTIL_autocorr(st->mem_speech, r);
   E_LPC_lag_wind(r + 1, M);  /* Lag windowing    */

   E_LPC_lev_dur(A, r, M);

   E_LPC_a_isp_conversion(A, ispnew, st->mem_isp, M);  /* From A(z) to isp */

   /* Find the interpolated isps and convert to a[] for all subframes */
   E_LPC_f_int_isp_find(st->mem_isp, ispnew, A, NB_SUBFR, M);

   /* update isp memory for the next frame */
   memcpy(st->mem_isp, ispnew, M * sizeof(Float32));

   /* Convert isps to frequency domain 0..6400 */
   E_LPC_isp_isf_conversion(ispnew, isf, M);

   /* check resonance for pitch clipping algorithm */
   E_GAIN_clip_isf_test(isf, st->mem_gp_clip);


   /*
    *  Perform PITCH_OL analysis
    *  -------------------------
    * - Find the residual res[] for the whole speech frame
    * - Find the weighted input speech wsp[] for the whole speech frame
    * - Find the 2 open-loop pitch estimate
    * - Set the range for searching closed-loop pitch in 1st subframe
    */

   p_A = A;

   for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
   {
      E_LPC_a_weight(p_A, Ap, GAMMA1, M);

      E_UTIL_residu(Ap, &speech[i_subfr], &wsp[i_subfr], L_SUBFR);

      p_A += (M + 1);
   }

   E_UTIL_deemph(wsp, TILT_FAC, L_FRAME, &(st->mem_wsp_df));

   /* decimation of wsp[] to search pitch in LF and to reduce complexity */

   E_GAIN_lp_decim2(wsp, L_FRAME, st->mem_decim2);

   /* Find open loop pitch lag for whole speech frame */

   if (*mode == MODE_7k)
   {
      /* Find open loop pitch lag for whole speech frame */

      T_op = E_GAIN_open_loop_search(wsp, PIT_MIN / OPL_DECIM,
         PIT_MAX / OPL_DECIM, L_FRAME / OPL_DECIM, st->mem_T0_med,
         &(st->mem_ol_gain), st->mem_hf_wsp, st->mem_hp_wsp,
         st->mem_ol_wght_flg);
   }
   else
   {
      /* Find open loop pitch lag for first 1/2 frame */

      T_op = E_GAIN_open_loop_search(wsp, PIT_MIN / OPL_DECIM,
         PIT_MAX / OPL_DECIM, (L_FRAME / 2) / OPL_DECIM, st->mem_T0_med,
         &(st->mem_ol_gain), st->mem_hf_wsp, st->mem_hp_wsp,
         st->mem_ol_wght_flg);
   }

   if (st->mem_ol_gain > 0.6)
   {
      st->mem_T0_med = E_GAIN_olag_median(T_op, st->mem_ol_lag);
      st->mem_ada_w = 1.0F;
   }
   else
   {
      st->mem_ada_w = st->mem_ada_w * 0.9F;
   }

   if (st->mem_ada_w < 0.8)
   {
      st->mem_ol_wght_flg = 0;
   }
   else
   {
      st->mem_ol_wght_flg = 1;
   }

   E_DTX_pitch_tone_detection(st->vadSt, st->mem_ol_gain);

   T_op *= OPL_DECIM;

   if (*mode != MODE_7k)
   {
      /* Find open loop pitch lag for second 1/2 frame */
      T_op2 = E_GAIN_open_loop_search(wsp + ((L_FRAME / 2) / OPL_DECIM),
         PIT_MIN / OPL_DECIM, PIT_MAX / OPL_DECIM, (L_FRAME / 2) / OPL_DECIM,
         st->mem_T0_med, &st->mem_ol_gain, st->mem_hf_wsp, st->mem_hp_wsp,
         st->mem_ol_wght_flg);

      if (st->mem_ol_gain > 0.6)
      {
          st->mem_T0_med = E_GAIN_olag_median(T_op2, st->mem_ol_lag);
          st->mem_ada_w = 1.0F;
      }
      else
      {
          st->mem_ada_w = st->mem_ada_w * 0.9F;
      }

      if (st->mem_ada_w < 0.8)
      {
         st->mem_ol_wght_flg = 0;
      }
      else
      {
         st->mem_ol_wght_flg = 1;
      }

      E_DTX_pitch_tone_detection(st->vadSt, st->mem_ol_gain);

      T_op2 *= OPL_DECIM;
   }
   else
   {
      T_op2 = T_op;
   }

   /*
    * DTX-CNG
    */
   if(*mode == MRDTX)
   {
      /* Buffer isf's and energy */
      E_UTIL_residu(&A[3 * (M + 1)], speech, f_exc, L_FRAME);
      f_tmp = 0.0;

      for(i = 0; i < L_FRAME; i++)
      {
         f_tmp += f_exc[i] * f_exc[i];
      }

      E_DTX_buffer(st->dtx_encSt, isf, f_tmp, codec_mode);

      /* Quantize and code the isfs */
      E_DTX_exe(st->dtx_encSt, f_exc2, &prms);

      /* reset speech coder memories */
      E_MAIN_reset(st, 0);

      /*
       * Update signal for next frame.
       * -> save past of speech[] and wsp[].
       */
      memcpy(st->mem_speech, &st->mem_speech[L_FRAME],
         (L_TOTAL - L_FRAME) * sizeof(Float32));
      memcpy(st->mem_wsp, &st->mem_wsp[L_FRAME / OPL_DECIM],
         (PIT_MAX / OPL_DECIM) * sizeof(Float32));

      return(0);
   }

   /*
    *   ACELP
    */

   /* Quantize and code the isfs */

   if (*mode <= MODE_7k)
   {
      E_LPC_isf_2s3s_quantise(isf, isfq, st->mem_isf_q, indice, 4);
      E_MAIN_parm_store((Word16)indice[0], &prms);
      E_MAIN_parm_store((Word16)indice[1], &prms);
      E_MAIN_parm_store((Word16)indice[2], &prms);
      E_MAIN_parm_store((Word16)indice[3], &prms);
      E_MAIN_parm_store((Word16)indice[4], &prms);
   }
   else
   {
      E_LPC_isf_2s5s_quantise(isf, isfq, st->mem_isf_q, indice, 4);
      E_MAIN_parm_store((Word16)indice[0], &prms);
      E_MAIN_parm_store((Word16)indice[1], &prms);
      E_MAIN_parm_store((Word16)indice[2], &prms);
      E_MAIN_parm_store((Word16)indice[3], &prms);
      E_MAIN_parm_store((Word16)indice[4], &prms);
      E_MAIN_parm_store((Word16)indice[5], &prms);
      E_MAIN_parm_store((Word16)indice[6], &prms);
   }

   /* Convert isfs to the cosine domain */
   E_LPC_isf_isp_conversion(isfq, ispnew_q, M);

   if (*mode == MODE_24k)
   {
      /* Check stability on isf : distance between old isf and current isf */

      f_tmp = 0.0F;
      f_pt_tmp = st->mem_isf;

      for (i=0; i < M - 1; i++)
      {
         f_tmp += (isf[i] - f_pt_tmp[i]) * (isf[i] - f_pt_tmp[i]);
      }

      stab_fac = (Float32)(1.25F - (f_tmp / 400000.0F));

      if (stab_fac > 1.0F)
      {
         stab_fac = 1.0F;
      }

      if (stab_fac < 0.0F)
      {
         stab_fac = 0.0F;
      }

      memcpy(f_pt_tmp, isf, M * sizeof(Float32));
   }


   if (st->mem_first_frame == 1)
   {
      st->mem_first_frame = 0;
      memcpy(st->mem_isp_q, ispnew_q, M * sizeof(Word16));
   }

   /* Find the interpolated isps and convert to a[] for all subframes */
   E_LPC_int_isp_find(st->mem_isp_q, ispnew_q, E_ROM_interpol_frac, s_Aq);


   for (i = 0; i < (NB_SUBFR * (M + 1)); i++)
   {
      Aq[i] = s_Aq[i] * 0.000244140625F; /* 1/4096 */
   }

   /* update isp memory for the next frame */
   memcpy(st->mem_isp_q, ispnew_q, M * sizeof(Word16));

   /*
    * Find the best interpolation for quantized ISPs
    */

   p_Aq = Aq;

   for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
   {
      E_UTIL_residu(p_Aq, &speech[i_subfr], &f_exc[i_subfr], L_SUBFR);
      p_Aq += (M + 1);
   }


   /* Buffer isf's and energy for dtx on non-speech frame */

   if(vad_flag == 0)
   {
      f_tmp = 0.0F;

      for(i = 0; i < L_FRAME; i++)
      {
         f_tmp += f_exc[i] * f_exc[i];
      }

      E_DTX_buffer(st->dtx_encSt, isf, f_tmp, codec_mode);
   }

   /* range for closed loop pitch search in 1st subframe */

   T0_min = T_op - 8;

   if (T0_min < PIT_MIN)
   {
      T0_min = PIT_MIN;
   }

   T0_max = T0_min + 15;

   if (T0_max > PIT_MAX)
   {
      T0_max = PIT_MAX;
      T0_min = T0_max - 15;
   }

   /*
    *          Loop for every subframe in the analysis frame
    *          ---------------------------------------------
    *  To find the pitch and innovation parameters. The subframe size is
    *  L_SUBFR and the loop is repeated L_FRAME/L_SUBFR times.
    *     - compute the target signal for pitch search
    *     - compute impulse response of weighted synthesis filter (h1[])
    *     - find the closed-loop pitch parameters
    *     - encode the pitch dealy
    *     - find 2 lt prediction (with / without LP filter for lt pred)
    *     - find 2 pitch gains and choose the best lt prediction.
    *     - find target vector for codebook search
    *     - update the impulse response h1[] for codebook search
    *     - correlation between target vector and impulse response
    *     - codebook search and encoding
    *     - VQ of pitch and codebook gains
    *     - find voicing factor and tilt of code for next subframe.
    *     - update states of weighting filter
    *     - find excitation and synthesis speech
    */

   p_A = A;
   p_Aq = Aq;

   for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
   {
      pit_flag = i_subfr;

      if ((i_subfr == (2 * L_SUBFR)) & (*mode > MODE_7k))
      {
         pit_flag = 0;

         /* range for closed loop pitch search in 3rd subframe */

         T0_min = T_op2 - 8;

         if (T0_min < PIT_MIN)
         {
            T0_min = PIT_MIN;
         }

         T0_max = T0_min + 15;

         if (T0_max > PIT_MAX)
         {
            T0_max = PIT_MAX;
            T0_min = T0_max - 15;
         }

      }

      /*
       *
       *        Find the target vector for pitch search:
       *        ---------------------------------------
       *
       *             |------|  res[n]
       * speech[n]---| A(z) |--------
       *             |------|       |   |--------| error[n]  |------|
       *                   zero -- (-)--| 1/A(z) |-----------| W(z) |-- target
       *                   exc          |--------|           |------|
       *
       * Instead of subtracting the zero-input response of filters from
       * the weighted input speech, the above configuration is used to
       * compute the target vector.
       *
       */

      for (i = 0; i < M; i++)
      {
         error[i] = (Float32)(speech[i + i_subfr - 16] - st->mem_syn[i]);
      }

      E_UTIL_residu(p_Aq, &speech[i_subfr], &f_exc[i_subfr], L_SUBFR);

      E_UTIL_synthesis(p_Aq, &f_exc[i_subfr], error + M, L_SUBFR, error, 0);

      E_LPC_a_weight(p_A, Ap, GAMMA1, M);

      E_UTIL_residu(Ap, error + M, xn, L_SUBFR);

      E_UTIL_deemph(xn, TILT_FAC, L_SUBFR, &(st->mem_w0));

      /*
       * Find target in residual domain (cn[]) for innovation search.
       */

      /* first half: xn[] --> cn[] */
      memset(f_code, 0, M * sizeof(Float32));

      memcpy(f_code + M, xn, (L_SUBFR / 2) * sizeof(Float32));

      f_tmp = 0.0F;

      E_UTIL_f_preemph(f_code + M, TILT_FAC, L_SUBFR / 2, &f_tmp);

      E_LPC_a_weight(p_A, Ap, GAMMA1, M);

      E_UTIL_synthesis(Ap, f_code + M, f_code + M, L_SUBFR / 2, f_code, 0);

      E_UTIL_residu(p_Aq, f_code + M, cn, L_SUBFR / 2);

      /* second half: res[] --> cn[] (approximated and faster) */
      for(i = (L_SUBFR / 2); i < L_SUBFR; i++)
      {
         cn[i] = f_exc[i_subfr + i];
      }

      /*
       * Compute impulse response, h1[], of weighted synthesis filter
       */

      E_LPC_a_weight(p_A, Ap, GAMMA1, M);

      memset(h1, 0, L_SUBFR * sizeof(Float32));
      memcpy(h1, Ap, (M + 1) * sizeof(Float32));
      E_UTIL_synthesis(p_Aq, h1, h1, L_SUBFR, h1 + (M + 1), 0);

      f_tmp = 0.0;
      E_UTIL_deemph(h1, TILT_FAC, L_SUBFR, &f_tmp);

      /*
       * Closed-loop fractional pitch search
       */

      /* find closed loop fractional pitch  lag */

      if (*mode <= MODE_9k)
      {
         T0 = E_GAIN_closed_loop_search(&f_exc[i_subfr], xn, h1,
            T0_min, T0_max, &T0_frac,
            pit_flag, PIT_MIN, PIT_FR1_8b);

         /* encode pitch lag */

         if (pit_flag == 0)   /* if 1st/3rd subframe */
         {

            /*
             * The pitch range for the 1st/3rd subframe is encoded with
             * 8 bits and is divided as follows:
             *   PIT_MIN to PIT_FR1-1  resolution 1/2 (frac = 0 or 2)
             *   PIT_FR1 to PIT_MAX    resolution 1   (frac = 0)
             */
            if (T0 < PIT_FR1_8b)
            {
               index = (Word16)(T0 * 2 + (T0_frac >> 1) - (PIT_MIN * 2));
            }
            else
            {
               index = (Word16)(T0 - PIT_FR1_8b + ((PIT_FR1_8b - PIT_MIN) * 2));
            }

            E_MAIN_parm_store(index, &prms);

            /* find T0_min and T0_max for subframe 2 and 4 */

            T0_min = T0 - 8;

            if (T0_min < PIT_MIN)
            {
               T0_min = PIT_MIN;
            }

            T0_max = T0_min + 15;

            if (T0_max > PIT_MAX)
            {
               T0_max = PIT_MAX;
               T0_min = T0_max - 15;
            }
         }
         else  /* if subframe 2 or 4 */
         {
            /*
             * The pitch range for subframe 2 or 4 is encoded with 6 bits:
             *   T0_min  to T0_max     resolution 1/2 (frac = 0 or 2)
             */
            i = T0 - T0_min;
            index = (Word16)(i * 2 + (T0_frac >> 1));

            E_MAIN_parm_store(index, &prms);
         }
      }
      else
      {
         T0 = E_GAIN_closed_loop_search(&f_exc[i_subfr], xn, h1,
            T0_min, T0_max, &T0_frac,
            pit_flag, PIT_FR2, PIT_FR1_9b);

         /* encode pitch lag */

         if (pit_flag == 0)   /* if 1st/3rd subframe */
         {
            /*
             * The pitch range for the 1st/3rd subframe is encoded with
             * 9 bits and is divided as follows:
             *   PIT_MIN to PIT_FR2-1  resolution 1/4 (frac = 0,1,2 or 3)
             *   PIT_FR2 to PIT_FR1-1  resolution 1/2 (frac = 0 or 2)
             *   PIT_FR1 to PIT_MAX    resolution 1   (frac = 0)
             */
            if (T0 < PIT_FR2)
            {
               index = (Word16)(T0 * 4 + T0_frac - (PIT_MIN * 4));
            }
            else if (T0 < PIT_FR1_9b)
            {
               index = (Word16)(T0 * 2 + (T0_frac >> 1) - (PIT_FR2 * 2) + ((PIT_FR2 - PIT_MIN) * 4));
            }
            else
            {
               index = (Word16)(T0 - PIT_FR1_9b + ((PIT_FR2 - PIT_MIN) * 4) + ((PIT_FR1_9b - PIT_FR2) * 2));
            }

            E_MAIN_parm_store(index, &prms);

            /* find T0_min and T0_max for subframe 2 and 4 */

            T0_min = T0 - 8;

            if (T0_min < PIT_MIN)
            {
               T0_min = PIT_MIN;
            }

            T0_max = T0_min + 15;

            if (T0_max > PIT_MAX)
            {
               T0_max = PIT_MAX;
               T0_min = T0_max - 15;
            }
         }
         else      /* if subframe 2 or 4 */
         {
            /*
             * The pitch range for subframe 2 or 4 is encoded with 6 bits:
             *   T0_min  to T0_max     resolution 1/4 (frac = 0,1,2 or 3)
             */
            i = T0 - T0_min;
            index = (Word16)(i * 4 + T0_frac);

            E_MAIN_parm_store(index, &prms);
         }
      }

      /*
       * Gain clipping test to avoid unstable synthesis on frame erasure
       */

      clip_gain = E_GAIN_clip_test(st->mem_gp_clip);

      /*
       * - find unity gain pitch excitation (adaptive codebook entry)
       *   with fractional interpolation.
       * - find filtered pitch exc. y1[]=exc[] convolved with h1[])
       * - compute pitch gain1
       */

      /* find pitch exitation */
      E_GAIN_adaptive_codebook_excitation(&exc[i_subfr], (Word16)T0, T0_frac, L_SUBFR + 1);

      if(*mode > MODE_9k)
      {

         E_UTIL_convolve(&exc[i_subfr], st->mem_q, h1, y1);

         gain1 = E_ACELP_xy1_corr(xn, y1, g_coeff);

         /* clip gain if necessary to avoid problem at decoder */
        if (clip_gain && (gain1 > 0.95))
        {
           gain1 = 0.95f;
        }

         /* find energy of new target xn2[] */
         E_ACELP_codebook_target_update(xn, dn, y1, gain1);
      }
      else
      {
         gain1 = 0.0F;
      }

      /*
       * - find pitch excitation filtered by 1st order LP filter.
       * - find filtered pitch exc. y2[]=exc[] convolved with h1[])
       * - compute pitch gain2
       */

      /* find pitch excitation with lp filter */
      for (i = 0; i < L_SUBFR; i++)
      {
         l_tmp = 5898 * exc[i - 1 + i_subfr];
         l_tmp += 20972 * exc[i + i_subfr];
         l_tmp += 5898 * exc[i + 1 + i_subfr];
         s_code[i] = (Word16)((l_tmp + 0x4000) >> 15);
      }

      E_UTIL_convolve(s_code, st->mem_q, h1, y2);

      gain2 = E_ACELP_xy1_corr(xn, y2, g_coeff2);

      /* clip gain if necessary to avoid problem at decoder */
      if (clip_gain && (gain2 > 0.95))
      {
         gain2 = 0.95F;
      }

      /* find energy of new target xn2[] */
      E_ACELP_codebook_target_update(xn, xn2, y2, gain2);

      /*
       * use the best prediction (minimise quadratic error).
       */

      select = 0;

      if (*mode > MODE_9k)
      {
         f_tmp = 0.0;
         for (i = 0; i < L_SUBFR; i++)
         {
            f_tmp += dn[i] * dn[i];
            f_tmp -= xn2[i] * xn2[i];
         }

         if (f_tmp < 0.1)
         {
            select = 1;
         }

         E_MAIN_parm_store(select, &prms);
      }

      if (select == 0)
      {
         /* use the lp filter for pitch excitation prediction */
         memcpy(&exc[i_subfr], s_code, L_SUBFR * sizeof(Word16));
         memcpy(y1, y2, L_SUBFR * sizeof(Float32));
         gain_pit = gain2;
         g_coeff[0] = g_coeff2[0];
         g_coeff[1] = g_coeff2[1];
      }
      else
      {
         /* no filter used for pitch excitation prediction */
         gain_pit = gain1;
         memcpy(xn2, dn, L_SUBFR * sizeof(Float32));        /* target vector for codebook search */
      }


      /*
       * - update target vector for codebook search
       * - scaling of cn[] to limit dynamic at 12 bits
       */
      for (i = 0; i < L_SUBFR; i ++)
      {
         cn[i] = (Float32)(cn[i] - gain_pit * exc[i_subfr + i] * pow(2, Q_new));
      }

      /*
       * - include fixed-gain pitch contribution into impulse resp. h1[]
       */
      f_tmp = 0.0F;
      E_UTIL_f_preemph(h1, (Float32)(st->mem_tilt_code / 32768.0), L_SUBFR, &f_tmp);

      if (T0_frac > 2)
      {
         T0++;
      }

      E_GAIN_f_pitch_sharpening(h1, T0);

      /*
       * - Correlation between target xn2[] and impulse response h1[]
       * - Innovative codebook search
       */
      E_ACELP_xh_corr(xn2, dn, h1);

      switch(*mode)
      {
      case MODE_7k:
         E_ACELP_2t(dn, cn, h1, s_code, y2, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         break;
      case MODE_9k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 20, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);

         break;
      case MODE_12k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 36, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);

         break;
      case MODE_14k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 44, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);

         break;
      case MODE_16k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 52, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);

         break;
      case MODE_18k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 64, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);
         E_MAIN_parm_store((Word16)indice[4], &prms);
         E_MAIN_parm_store((Word16)indice[5], &prms);
         E_MAIN_parm_store((Word16)indice[6], &prms);
         E_MAIN_parm_store((Word16)indice[7], &prms);

         break;
      case MODE_20k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 72, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);
         E_MAIN_parm_store((Word16)indice[4], &prms);
         E_MAIN_parm_store((Word16)indice[5], &prms);
         E_MAIN_parm_store((Word16)indice[6], &prms);
         E_MAIN_parm_store((Word16)indice[7], &prms);

         break;
      case MODE_23k:
      case MODE_24k:
         E_ACELP_4t(dn, cn, h1, s_code, y2, 88, *mode, indice);
         E_MAIN_parm_store((Word16)indice[0], &prms);
         E_MAIN_parm_store((Word16)indice[1], &prms);
         E_MAIN_parm_store((Word16)indice[2], &prms);
         E_MAIN_parm_store((Word16)indice[3], &prms);
         E_MAIN_parm_store((Word16)indice[4], &prms);
         E_MAIN_parm_store((Word16)indice[5], &prms);
         E_MAIN_parm_store((Word16)indice[6], &prms);
         E_MAIN_parm_store((Word16)indice[7], &prms);
         break;
      default:
         return -1;
      }

      /*
       * - Add the fixed-gain pitch contribution to code[].
       */
      s_tmp = 0;
      E_UTIL_preemph(s_code, st->mem_tilt_code, L_SUBFR, &s_tmp);

      E_GAIN_pitch_sharpening(s_code, (Word16)T0);

      E_ACELP_xy2_corr(xn, y1, y2, g_coeff);

      /*
       *  - Compute the fixed codebook gain
       *  - quantize fixed codebook gain
       */
      if (*mode <= MODE_9k)
      {
          index = (Word16)E_ACELP_gains_quantise(s_code, 6, gain_pit,
             &s_gain_pit, &L_gain_code, g_coeff, clip_gain, st->mem_gain_q);

         E_MAIN_parm_store(index, &prms);
      }
      else
      {
         index = (Word16)E_ACELP_gains_quantise(s_code, 7, gain_pit,
            &s_gain_pit, &L_gain_code, g_coeff, clip_gain, st->mem_gain_q);
         E_MAIN_parm_store(index, &prms);
      }

      /* find best scaling to perform on excitation (Q_new) */
      s_tmp = st->mem_subfr_q[0];
      for (i = 1; i < 4; i++)
      {
         if (st->mem_subfr_q[i] < s_tmp)
         {
            s_tmp = st->mem_subfr_q[i];
         }
      }

      /* limit scaling (Q_new) to Q_MAX */
      if (s_tmp > Q_MAX)
      {
         s_tmp = Q_MAX;
      }

      Q_new = 0;
      l_tmp = L_gain_code;                 /* L_gain_code in Q16 */

      while ((l_tmp < 0x08000000L) && (Q_new < s_tmp))
      {
         l_tmp = (l_tmp << 1);
         Q_new = Q_new + 1;
      }

      if (l_tmp < 0x7FFF7FFF)
      {
         /* scaled gain_code with Qnew */
         gain_code = (Word16)((l_tmp + 0x8000) >> 16);
      }
      else
      {
         gain_code = 32767;
      }

      if (Q_new > st->mem_q)
      {
         E_UTIL_signal_up_scale(exc + i_subfr - (PIT_MAX + L_INTERPOL),
            (Word16)(Q_new - st->mem_q));
      }
      else
      {
         E_UTIL_signal_down_scale(exc + i_subfr - (PIT_MAX + L_INTERPOL),
            PIT_MAX + L_INTERPOL + L_SUBFR, (Word16)(st->mem_q - Q_new));
      }
      st->mem_q = (Word16)Q_new;

      /* test quantized gain of pitch for pitch clipping algorithm */
      E_GAIN_clip_pit_test((Float32)(s_gain_pit * pow(2, -14)),
         st->mem_gp_clip);

      /*
       * Update parameters for the next subframe.
       * - tilt of code: 0.0 (unvoiced) to 0.5 (voiced)
       */

      /* find voice factor in Q15 (1=voiced, -1=unvoiced) */
      memcpy(exc2, &exc[i_subfr], L_SUBFR * sizeof(Word16));

      E_UTIL_signal_down_scale(exc2, L_SUBFR, 3);

      voice_fac = E_GAIN_voice_factor(exc2, -3, s_gain_pit, s_code, gain_code);

      /* tilt of code for next subframe: 0.5=voiced, 0=unvoiced */
      st->mem_tilt_code = (Word16)((voice_fac >> 2) + 8192);

      /*
       * - Update filter's memory "mem_w0" for finding the
       *   target vector in the next subframe.
       * - Find the total excitation
       * - Find synthesis speech to update mem_syn[].
       */
      memcpy(exc2, &exc[i_subfr], L_SUBFR * sizeof(Word16));

      st->mem_w0 = (Float32)((xn[L_SUBFR - 1] -
         ((s_gain_pit / 16384.0F) * y1[L_SUBFR - 1])) -
         (gain_code * pow(2, -st->mem_q) * y2[L_SUBFR - 1]));

      if (*mode == MODE_24k)
      {
         Q_new = -st->mem_q;

         for (i = 0; i < L_SUBFR; i++)
         {
            f_exc2[i_subfr + i] = (Float32)(exc[i_subfr + i] * pow(2, Q_new) * (s_gain_pit / 16384.0F));
         }
      }

      s_max = 1;
      for (i = 0; i < L_SUBFR; i++)
      {
         /* code in Q9, gain_pit in Q14 */
         l_tmp = gain_code * s_code[i];
         l_tmp = l_tmp << 5;
         l_tmp += exc[i + i_subfr] * s_gain_pit; /* gain_pit Q14 */
         l_tmp = (l_tmp + 0x2000) >> 14;

         if ((l_tmp > MIN_16) & (l_tmp < 32768))
         {
            exc[i + i_subfr] = (Word16)l_tmp;
            s_tmp = (Word16)abs(l_tmp);

            if (s_tmp > s_max)
            {
               s_max = s_tmp;
            }
         }
         else if (l_tmp > MAX_16)
         {
            exc[i + i_subfr] = MAX_16;
            s_max = MAX_16;
         }
         else
         {
            exc[i + i_subfr] = MIN_16;
            s_max = MAX_16;
         }
      }

      /* tmp = scaling possible according to max value of excitation */
      s_tmp = (Word16)((E_UTIL_norm_s(s_max) + st->mem_q) - 1);

      st->mem_subfr_q[3] = st->mem_subfr_q[2];
      st->mem_subfr_q[2] = st->mem_subfr_q[1];
      st->mem_subfr_q[1] = st->mem_subfr_q[0];
      st->mem_subfr_q[0] = s_tmp;

      Q_new = -st->mem_q;

      for (i = 0; i < L_SUBFR; i++)
      {
         f_exc[i + i_subfr] = (Float32)(exc[i + i_subfr] * pow(2, Q_new));
      }

      E_UTIL_synthesis(p_Aq, &f_exc[i_subfr], synth, L_SUBFR, st->mem_syn, 1);

      if(*mode >= MODE_24k)
      {
         /*
          * noise enhancer
          * --------------
          * - Enhance excitation on noise. (modify gain of code)
          *   If signal is noisy and LPC filter is stable, move gain
          *   of code 1.5 dB toward gain of code threshold.
          *   This decrease by 3 dB noise energy variation.
          */

         /* 1=unvoiced, 0=voiced */
         f_tmp = (Float32)(0.5 * (1.0 - (voice_fac / 32768.0)));
         fac = stab_fac * f_tmp;
         f_tmp = (Float32)(gain_code * pow(2, -st->mem_q));

         if(f_tmp < st->mem_gc_threshold)
         {
            f_tmp = (Float32)(f_tmp * 1.19);

            if(f_tmp > st->mem_gc_threshold)
            {
               f_tmp = st->mem_gc_threshold;
            }
         }
         else
         {
            f_tmp = (Float32)(f_tmp / 1.19);

            if(f_tmp < st->mem_gc_threshold)
            {
               f_tmp = st->mem_gc_threshold;
            }
         }

         st->mem_gc_threshold = f_tmp;
         f_tmp = (Float32)(((fac * f_tmp) + ((1.0 - fac) *
            (gain_code * pow(2, -st->mem_q)))) * 0.001953125F);

         for(i = 0; i < L_SUBFR; i++)
         {
            f_code[i] = (Float32)(s_code[i] * f_tmp);
         }

         /*
          * pitch enhancer
          * --------------
          * - Enhance excitation on voice. (HP filtering of code)
          *   On voiced signal, filtering of code by a smooth fir HP
          *   filter to decrease energy of code in low frequency.
          */

         /* 0.25=voiced, 0=unvoiced */
         f_tmp = (Float32)(0.125F * (1.0F + (voice_fac / 32768.0)));

         f_exc2[i_subfr] += f_code[0] - (f_tmp * f_code[1]);

         for(i = 1; i < L_SUBFR - 1; i++)
         {
            f_exc2[i + i_subfr] +=
               f_code[i] - (f_tmp * f_code[i - 1]) - (f_tmp * f_code[i + 1]);
         }

         f_exc2[i_subfr + L_SUBFR - 1] +=
            f_code[L_SUBFR - 1] - (f_tmp * f_code[L_SUBFR - 2]);

         corr_gain = (Word16)E_UTIL_enc_synthesis(p_Aq, &f_exc2[i_subfr],
            &f_speech16k[i_subfr * 5 /4], st);

         E_MAIN_parm_store(corr_gain, &prms);
      }

      p_A += (M + 1);
      p_Aq += (M + 1);
  }   /* end of subframe loop */

   /*
    * Update signal for next frame.
    * -> save past of speech[], wsp[] and exc[].
    */

   memmove(st->mem_speech, &st->mem_speech[L_FRAME], (L_TOTAL - L_FRAME) * sizeof(Float32));
   memmove(st->mem_wsp, &st->mem_wsp[L_FRAME / OPL_DECIM], (PIT_MAX / OPL_DECIM) * sizeof(Float32));
   memmove(st->mem_exc, &st->mem_exc[L_FRAME], (PIT_MAX + L_INTERPOL) * sizeof(Word16));

   return 0;
}
