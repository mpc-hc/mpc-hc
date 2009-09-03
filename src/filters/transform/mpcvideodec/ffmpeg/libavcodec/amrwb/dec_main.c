/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include "typedef.h"
#include "dec_main.h"
#include "dec_dtx.h"
#include "dec_acelp.h"
#include "dec_gain.h"
#include "dec_lpc.h"
#include "dec_util.h"

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

#define L_FRAME            256   /* Frame size                             */
#define NB_SUBFR           4     /* Number of subframe per frame           */
#define L_SUBFR            64    /* Subframe size                          */
#define MODE_7k            0     /* modes                                  */
#define MODE_9k            1
#define MODE_12k           2
#define MODE_14k           3
#define MODE_16k           4
#define MODE_18k           5
#define MODE_20k           6
#define MODE_23k           7
#define MODE_24k           8
#define RX_SPEECH_PROBABLY_DEGRADED 1  /* rx types                         */
#define RX_SPEECH_LOST     2
#define RX_SPEECH_BAD      3
#define RX_NO_DATA         7
#define Q_MAX              8     /* scaling max for signal                 */
#define PIT_SHARP          27853 /* pitch sharpening factor = 0.85 Q15     */
#define PIT_MIN            34    /* Minimum pitch lag with resolution 1/4  */
#define PIT_FR2            128   /* Minimum pitch lag with resolution 1/2  */
#define PIT_FR1_9b         160   /* Minimum pitch lag with resolution 1    */
#define PIT_FR1_8b         92    /* Minimum pitch lag with resolution 1    */

extern const Word16 D_ROM_isp[];
extern const Word16 D_ROM_isf[];
extern const Word16 D_ROM_interpol_frac[];

#ifdef WIN32
#pragma warning( disable : 4310)
#endif

/*
 * Decoder_reset
 *
 * Parameters:
 *    st        I/O: pointer to state structure
 *    reset_all   I: perform full reset
 *
 * Function:
 *    Initialisation of variables for the decoder section.
 *
 *
 * Returns:
 *    void
 */
void D_MAIN_reset(void *st, Word16 reset_all)
{
   Word32 i;

   Decoder_State *dec_state;

   dec_state = (Decoder_State*)st;
   memset(dec_state->mem_exc, 0, (PIT_MAX + L_INTERPOL) * sizeof(Word16));
   memset(dec_state->mem_isf_q, 0, M * sizeof(Word16));
   dec_state->mem_T0_frac = 0;   /* old pitch value = 64.0 */
   dec_state->mem_T0 = 64;
   dec_state->mem_first_frame = 1;
   dec_state->mem_gc_thres = 0;
   dec_state->mem_tilt_code = 0;
   memset(dec_state->mem_ph_disp, 0, 8 * sizeof(Word16));

   /* scaling memories for excitation */
   dec_state->mem_q = Q_MAX;
   dec_state->mem_subfr_q[3] = Q_MAX;
   dec_state->mem_subfr_q[2] = Q_MAX;
   dec_state->mem_subfr_q[1] = Q_MAX;
   dec_state->mem_subfr_q[0] = Q_MAX;

   if(reset_all != 0)
   {
      /* routines initialization */
      D_GAIN_init(dec_state->mem_gain);
      memset(dec_state->mem_oversamp, 0, (2 * 12) * sizeof(Word16));
      memset(dec_state->mem_sig_out, 0, 6 * sizeof(Word16));
      memset(dec_state->mem_hf, 0, (31 - 1) * sizeof(Word16));
      memset(dec_state->mem_hf3, 0, (31 - 1) * sizeof(Word16));
      memset(dec_state->mem_hp400, 0, 6 * sizeof(Word16));
      D_GAIN_lag_concealment_init(dec_state->mem_lag);

      /* isp initialization */
      memcpy(dec_state->mem_isp, D_ROM_isp, M * sizeof(Word16));
      memcpy(dec_state->mem_isf, D_ROM_isf, M * sizeof(Word16));

      for(i = 0; i < L_MEANBUF; i++)
      {
         memcpy(&dec_state->mem_isf_buf[i * M], D_ROM_isf, M * sizeof(Word16));
      }

      /* variable initialization */
      dec_state->mem_deemph = 0;
      dec_state->mem_seed = 21845;   /* init random with 21845 */
      dec_state->mem_seed2 = 21845;
      dec_state->mem_seed3 = 21845;
      dec_state->mem_state = 0;
      dec_state->mem_bfi = 0;

      /* Static vectors to zero */
      memset(dec_state->mem_syn_hf, 0, M16k * sizeof(Word16));
      memset(dec_state->mem_syn_hi, 0, M * sizeof(Word16));
      memset(dec_state->mem_syn_lo, 0, M * sizeof(Word16));
      D_DTX_reset(dec_state->dtx_decSt, D_ROM_isf);
      dec_state->mem_vad_hist = 0;
   }

   return;
}


/*
 * Decoder_init
 *
 * Parameters:
 *    spd_state         O: pointer to state structure
 *
 * Function:
 *    Initialization of variables for the decoder section.
 *    Memory allocation.
 *
 * Returns:
 *    return zero if succesful
 */
Word32 D_MAIN_init(void **spd_state)
{
   /* Decoder states */
   Decoder_State *st;

   *spd_state = NULL;

   /*
    * Memory allocation for coder state.
    */
   if((st = (Decoder_State*)malloc(sizeof(Decoder_State))) == NULL)
   {
      return(-1);
   }

   st->dtx_decSt = NULL;
   D_DTX_init(&st->dtx_decSt, D_ROM_isf);
   D_MAIN_reset((void *)st, 1);
   *spd_state = (void *)st;

   return(0);
}


/*
 * Decoder_close
 *
 * Parameters:
 *    spd_state   I: pointer to state structure
 *
 * Function:
 *    Free coder memory.
 *
 * Returns:
 *    void
 */
void D_MAIN_close(void **spd_state)
{
   D_DTX_exit(&(((Decoder_State *)(*spd_state))->dtx_decSt));
   free(*spd_state);

   return;
}


/*
 * Decoder_exe
 *
 * Parameters:
 *    mode           I: used mode
 *    prms           I: parameter vector
 *    synth_out      O: synthesis speech
 *    spe_state      B: state structure
 *    frame_type     I: received frame type
 *
 * Function:
 *    Main decoder routine.
 *
 * Returns:
 *    0 if successful
 */
Word32 D_MAIN_decode(Word16 mode, Word16 prms[], Word16 synth16k[],
                     void *spd_state, UWord8 frame_type)
{

   Word32 code2[L_SUBFR];           /* algebraic codevector                */
   Word32 L_tmp, L_tmp2, L_gain_code, L_stab_fac;
   Word32 i, j, i_subfr, pit_flag;
   Word32 T0, T0_frac, T0_max, select, T0_min = 0;

   Word16 exc2[L_FRAME];            /* excitation vector                   */
   Word16 Aq[NB_SUBFR * (M + 1)];   /* A(z) quantized for the 4 subframes  */
   Word16 code[L_SUBFR];            /* algebraic codevector                */
   Word16 excp[L_SUBFR];            /* excitation vector                   */
   Word16 HfIsf[M16k];
   Word16 ispnew[M];                /* immittance spectral pairs at 4nd sfr*/
   Word16 isf[M];                   /* ISF (frequency domain) at 4nd sfr   */
   Word16 isf_tmp[M];               /* ISF tmp                             */
   Word16 ind[8];                   /* quantization indices                */

   Word16 index, fac, voice_fac, max, Q_new = 0;
   Word16 gain_pit, gain_code, gain_code_lo, tmp;
   Word16 corr_gain = 0;
   UWord16 pit_sharp = 0;

   Word16 *exc;                     /* Excitation vector                   */
   Word16 *p_Aq;                    /* ptr to A(z) for the 4 subframes     */
   Word16 *p_isf;                   /* prt to isf                          */

   Decoder_State *st;   /* Decoder states */
   UWord8 newDTXState, bfi, unusable_frame;
   UWord8 vad_flag;

   st = (Decoder_State*)spd_state;

   /* find the new  DTX state  SPEECH OR DTX */
   newDTXState = D_DTX_rx_handler(st->dtx_decSt, frame_type);

   if(newDTXState != SPEECH)
   {
      D_DTX_exe(st->dtx_decSt, exc2, newDTXState, isf, &prms);
   }

   /* SPEECH action state machine  */
   if((frame_type == RX_SPEECH_BAD) |
      (frame_type == RX_SPEECH_PROBABLY_DEGRADED))
   {
      /* bfi for all index, bits are not usable */
      bfi = 1;
      unusable_frame = 0;
   }
   else if((frame_type == RX_NO_DATA) | (frame_type == RX_SPEECH_LOST))
   {
      /* bfi only for lsf, gains and pitch period */
      bfi = 1;
      unusable_frame = 1;
   }
   else
   {
      bfi = 0;
      unusable_frame = 0;
   }

   if(bfi != 0)
   {
      st->mem_state = (UWord8)(st->mem_state + 1);

      if(st->mem_state > 6)
      {
         st->mem_state = 6;
      }
   }
   else
   {
      st->mem_state = (UWord8)(st->mem_state >> 1);
   }

   /*
    * If this frame is the first speech frame after CNI period,
    * set the BFH state machine to an appropriate state depending
    * on whether there was DTX muting before start of speech or not
    * If there was DTX muting, the first speech frame is muted.
    * If there was no DTX muting, the first speech frame is not
    * muted. The BFH state machine starts from state 5, however, to
    * keep the audible noise resulting from a SID frame which is
    * erroneously interpreted as a good speech frame as small as
    * possible (the decoder output in this case is quickly muted)
    */

   if(st->dtx_decSt->mem_dtx_global_state == DTX)
   {
      st->mem_state = 5;
      st->mem_bfi = 0;
   }
   else if(st->dtx_decSt->mem_dtx_global_state == D_DTX_MUTE)
   {
      st->mem_state = 5;
      st->mem_bfi = 1;
   }

   if(newDTXState == SPEECH)
   {
      vad_flag = (UWord8)(*prms++);

      if(bfi == 0)
      {
         if(vad_flag == 0)
         {
            st->mem_vad_hist = (Word16)(st->mem_vad_hist + 1);

            if(st->mem_vad_hist > 32767)
            {
               st->mem_vad_hist = 32767;
            }
         }
         else
         {
            st->mem_vad_hist = 0;
         }
      }
   }

   /*
    * DTX-CNG
    */
   if(newDTXState != SPEECH) /* CNG mode */
   {
      /*
       * increase slightly energy of noise below 200 Hz
       * Convert ISFs to the cosine domain
       */
      D_LPC_isf_isp_conversion(isf, ispnew, M);
      D_LPC_isp_a_conversion(ispnew, Aq, 1, M);
      memcpy(isf_tmp, st->mem_isf, M * sizeof(Word16));

      for(i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
      {
         j = (i_subfr >> 6);

         for(i = 0; i < M; i++)
         {
            L_tmp = (isf_tmp[i] * (32767 - D_ROM_interpol_frac[j])) << 1;
            L_tmp = L_tmp + ((isf[i] * D_ROM_interpol_frac[j]) << 1);
            HfIsf[i] = (Word16)((L_tmp + 0x8000) >> 16);
         }

         D_UTIL_dec_synthesis(Aq, &exc2[i_subfr], 0, &synth16k[i_subfr * 5 /4],
            (Word16) 1, HfIsf, mode, newDTXState, bfi, st);
      }

      /* reset speech coder memories */
      D_MAIN_reset(st, 0);
      memcpy(st->mem_isf, isf, M * sizeof(Word16));
      st->mem_bfi = bfi;
      st->dtx_decSt->mem_dtx_global_state = (UWord8)newDTXState;

      return(0);
   }

   /*
    * ACELP
    */

   exc = st->mem_exc + PIT_MAX + L_INTERPOL;

   /* Decode the ISFs */
   if(mode <= MODE_7k)
   {
      ind[0] = *prms++;
      ind[1] = *prms++;
      ind[2] = *prms++;
      ind[3] = *prms++;
      ind[4] = *prms++;
      D_LPC_isf_2s3s_decode(ind, isf, st->mem_isf_q, st->mem_isf,
         st->mem_isf_buf, bfi);
   }
   else
   {
      ind[0] = *prms++;
      ind[1] = *prms++;
      ind[2] = *prms++;
      ind[3] = *prms++;
      ind[4] = *prms++;
      ind[5] = *prms++;
      ind[6] = *prms++;
      D_LPC_isf_2s5s_decode(ind, isf, st->mem_isf_q, st->mem_isf,
         st->mem_isf_buf, bfi);
   }

   /* Convert ISFs to the cosine domain */
   D_LPC_isf_isp_conversion(isf, ispnew, M);

   if(st->mem_first_frame != 0)
   {
      st->mem_first_frame = 0;
      memcpy(st->mem_isp, ispnew, M * sizeof(Word16));
   }

   /* Find the interpolated ISPs and convert to a[] for all subframes */
   D_LPC_int_isp_find(st->mem_isp, ispnew, D_ROM_interpol_frac, Aq);

   /* update isp memory for the next frame */
   memcpy(st->mem_isp, ispnew, M * sizeof(Word16));

   /* Check stability on isf : distance between old isf and current isf */
   L_tmp = 0;
   p_isf = st->mem_isf;

   for(i = 0; i < M - 1; i++)
   {
      tmp = (Word16)((isf[i] - p_isf[i]));
      L_tmp = L_tmp + (tmp * tmp);
   }

   if(L_tmp < 3276928)
   {
      L_tmp = L_tmp >> 7;
      L_tmp = (L_tmp * 26214) >> 15;   /* tmp = L_tmp*0.8/256        */
      L_tmp = 20480 - L_tmp;           /* 1.25 - tmp                 */
      L_stab_fac = L_tmp << 1;         /* Q14 -> Q15 with saturation */

      if(L_stab_fac > 0x7FFF)
      {
         L_stab_fac = 0x7FFF;
      }
   }
   else
   {
      L_stab_fac = 0x0;
   }

   memcpy(isf_tmp, st->mem_isf, M * sizeof(Word16));
   memcpy(st->mem_isf, isf, M * sizeof(Word16));

   /*
    * Loop for every subframe in the analysis frame
    *
    * The subframe size is L_SUBFR and the loop is repeated L_FRAME/L_SUBFR
    * times
    *   - decode the pitch delay and filter mode
    *   - decode algebraic code
    *   - decode pitch and codebook gains
    *   - find voicing factor and tilt of code for next subframe
    *   - find the excitation and compute synthesis speech
    */

   p_Aq = Aq;   /* pointer to interpolated LPC parameters */

   for(i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
   {
      pit_flag = i_subfr;

      if((i_subfr == (2 * L_SUBFR)) & (mode > MODE_7k))
      {
         pit_flag = 0;
      }

      /*
       * - Decode pitch lag
       * Lag indeces received also in case of BFI,
       * so that the parameter pointer stays in sync.
       */

      if(pit_flag == 0)
      {
         if(mode <= MODE_9k)
         {
            index = *prms++;

            if(index < ((PIT_FR1_8b - PIT_MIN) * 2))
            {
               T0 = (PIT_MIN + (index >> 1));
               T0_frac = (index - ((T0 - PIT_MIN) << 1));
               T0_frac = (T0_frac << 1);
            }
            else
            {
               T0 = index + (PIT_FR1_8b - ((PIT_FR1_8b - PIT_MIN) * 2));
               T0_frac = 0;
            }
         }
         else
         {
            index = *prms++;

            if(index < ((PIT_FR2 - PIT_MIN) * 4))
            {
               T0 = PIT_MIN + (index >> 2);
               T0_frac = index - ((T0 - PIT_MIN) << 2);
            }
            else if(index <
               ((((PIT_FR2 - PIT_MIN) * 4) + ((PIT_FR1_9b - PIT_FR2) * 2))))
            {
               index = (Word16)((index - ((PIT_FR2 - PIT_MIN) * 4)));
               T0 = PIT_FR2 + (index >> 1);
               T0_frac = index - ((T0 - PIT_FR2) << 1);
               T0_frac = T0_frac << 1;
            }
            else
            {
               T0 = index + (PIT_FR1_9b - ((PIT_FR2 - PIT_MIN) * 4) -
                  ((PIT_FR1_9b - PIT_FR2) * 2));
               T0_frac = 0;
            }
         }

         /* find T0_min and T0_max for subframe 2 and 4 */
         T0_min = T0 - 8;

         if(T0_min < PIT_MIN)
         {
            T0_min = PIT_MIN;
         }

         T0_max = T0_min + 15;

         if(T0_max > PIT_MAX)
         {
            T0_max = PIT_MAX;
            T0_min = T0_max - 15;
         }
      }
      else
      {   /* if subframe 2 or 4 */

         if(mode <= MODE_9k)
         {
            index = *prms++;
            T0 = T0_min + (index >> 1);
            T0_frac = index - ((T0 - T0_min) << 1);
            T0_frac = T0_frac << 1;
         }
         else
         {
            index = *prms++;
            T0 = T0_min + (index >> 2);
            T0_frac = index - ((T0 - T0_min) << 2);
         }
      }

      /* check BFI after pitch lag decoding */
      if(bfi != 0) /* if frame erasure */
      {
         D_GAIN_lag_concealment(&(st->mem_gain[17]), st->mem_lag, &T0,
            &(st->mem_T0), &(st->mem_seed3), unusable_frame);
         T0_frac = 0;
      }

      /*
       * Find the pitch gain, the interpolation filter
       * and the adaptive codebook vector.
       */

      D_GAIN_adaptive_codebook_excitation(&exc[i_subfr], T0, T0_frac);

      if(unusable_frame)
      {
         select = 1;
      }
      else
      {
         if(mode <= MODE_9k)
         {
            select = 0;
         }
         else
         {
            select = *prms++;
         }
      }

      if(select == 0)
      {
         /* find pitch excitation with lp filter */
         for(i = 0; i < L_SUBFR; i++)
         {
            L_tmp = 2949 * exc[i - 1 + i_subfr];
            L_tmp = L_tmp + (10486 * exc[i + i_subfr]);
            L_tmp = L_tmp + (2949 * exc[i + 1 + i_subfr]);
            code[i] = (Word16)((L_tmp + 0x2000) >> 14);
         }

         memcpy(&exc[i_subfr], code, L_SUBFR * sizeof(Word16));
      }

      /*
       * Decode innovative codebook.
       * Add the fixed-gain pitch contribution to code[].
       */

      if(unusable_frame != 0)
      {
         /* the innovative code doesn't need to be scaled (see Q_gain2) */
         for(i = 0; i < L_SUBFR; i++)
         {
            code[i] = (Word16)(D_UTIL_random(&(st->mem_seed)) >> 3);
         }
      }
      else if(mode <= MODE_7k)
      {
         ind[0] = *prms++;
         D_ACELP_decode_2t(ind[0], code);
      }
      else if(mode <= MODE_9k)
      {
         memcpy(ind, prms, 4 * sizeof(Word16));
         prms += 4;
         D_ACELP_decode_4t(ind, 20, code);
      }
      else if(mode <= MODE_12k)
      {
         memcpy(ind, prms, 4 * sizeof(Word16));
         prms += 4;
         D_ACELP_decode_4t(ind, 36, code);
      }
      else if(mode <= MODE_14k)
      {
         memcpy(ind, prms, 4 * sizeof(Word16));
         prms += 4;
         D_ACELP_decode_4t(ind, 44, code);
      }
      else if(mode <= MODE_16k)
      {
         memcpy(ind, prms, 4 * sizeof(Word16));
         prms += 4;
         D_ACELP_decode_4t(ind, 52, code);
      }
      else if(mode <= MODE_18k)
      {
         memcpy(ind, prms, 8 * sizeof(Word16));
         prms += 8;
         D_ACELP_decode_4t(ind, 64, code);
      }
      else if(mode <= MODE_20k)
      {
         memcpy(ind, prms, 8 * sizeof(Word16));
         prms += 8;
         D_ACELP_decode_4t(ind, 72, code);
      }
      else
      {
         memcpy(ind, prms, 8 * sizeof(Word16));
         prms += 8;
         D_ACELP_decode_4t(ind, 88, code);
      }

      tmp = 0;
      D_UTIL_preemph(code, st->mem_tilt_code, L_SUBFR, &tmp);

      L_tmp = T0;

      if(T0_frac > 2)
      {
         L_tmp = L_tmp + 1;
      }

      D_GAIN_pitch_sharpening(code, L_tmp, PIT_SHARP);

      /*
       * Decode codebooks gains.
       */
      index = *prms++;   /* codebook gain index */

      if(mode <= MODE_9k)
      {
         D_GAIN_decode(index, 6, code, &gain_pit, &L_gain_code, bfi,
            st->mem_bfi, st->mem_state, unusable_frame, st->mem_vad_hist,
            st->mem_gain);
      }
      else
      {
         D_GAIN_decode(index, 7, code, &gain_pit, &L_gain_code, bfi,
            st->mem_bfi, st->mem_state, unusable_frame, st->mem_vad_hist,
            st->mem_gain);
      }

      /* find best scaling to perform on excitation (Q_new) */
      tmp = st->mem_subfr_q[0];

      for(i = 1; i < 4; i++)
      {
         if(st->mem_subfr_q[i] < tmp)
         {
            tmp = st->mem_subfr_q[i];
         }
      }

      /* limit scaling (Q_new) to Q_MAX */
      if(tmp > Q_MAX)
      {
         tmp = Q_MAX;
      }

      Q_new = 0;
      L_tmp = L_gain_code;   /* L_gain_code in Q16 */

      while((L_tmp < 0x08000000L) && (Q_new < tmp))
      {
         L_tmp = (L_tmp << 1);
         Q_new = (Word16)((Q_new + 1));
      }

      if(L_tmp < 0x7FFF7FFF)
      {
         gain_code = (Word16)((L_tmp + 0x8000) >> 16);
         /* scaled gain_code with Qnew */
      }
      else
      {
         gain_code = 32767;
      }

      if(Q_new > st->mem_q)
      {
         D_UTIL_signal_up_scale(exc + i_subfr - (PIT_MAX + L_INTERPOL),
            PIT_MAX + L_INTERPOL + L_SUBFR, (Word16)(Q_new - st->mem_q));
      }
      else
      {
         D_UTIL_signal_down_scale(exc + i_subfr - (PIT_MAX + L_INTERPOL),
            PIT_MAX + L_INTERPOL + L_SUBFR, (Word16)(st->mem_q - Q_new));
      }

      st->mem_q = Q_new;

      /*
       * Update parameters for the next subframe.
       * - tilt of code: 0.0 (unvoiced) to 0.5 (voiced)
       */
      if(bfi == 0)
      {
         /* LTP-Lag history update */
         for(i = 4; i > 0; i--)
         {
            st->mem_lag[i] = st->mem_lag[i - 1];
         }
         st->mem_lag[0] = (Word16)T0;
         st->mem_T0 = (Word16)T0;
         st->mem_T0_frac = 0;   /* Remove fraction in case of BFI */
      }

      /* find voice factor in Q15 (1=voiced, -1=unvoiced) */
      memcpy(exc2, &exc[i_subfr], L_SUBFR * sizeof(Word16));
      D_UTIL_signal_down_scale(exc2, L_SUBFR, 3);

      /* post processing of excitation elements */
      if(mode <= MODE_9k)
      {
         pit_sharp = (Word16)(gain_pit << 1);

         if(pit_sharp > 16384)
         {
            if(pit_sharp > 32767)
            {
               pit_sharp = 32767;
            }

            for(i = 0; i < L_SUBFR; i++)
            {
               L_tmp = (exc2[i] * pit_sharp) >> 15;
               L_tmp = L_tmp * gain_pit;
               excp[i] = (Word16)((L_tmp + 0x8000) >> 16);
            }
         }
      }

      voice_fac = D_GAIN_find_voice_factor(exc2, -3, gain_pit, code, gain_code,
         L_SUBFR);

      /* tilt of code for next subframe: 0.5=voiced, 0=unvoiced */
      st->mem_tilt_code = (Word16)((voice_fac >> 2) + 8192);

      /*
       * Find the total excitation.
       * Find synthesis speech corresponding to exc[].
       * Find maximum value of excitation for next scaling
       */
      memcpy(exc2, &exc[i_subfr], L_SUBFR * sizeof(Word16));
      max = 1;

      for(i = 0; i < L_SUBFR; i++)
      {
         L_tmp = (code[i] * gain_code) << 5;
         L_tmp = L_tmp + (exc[i + i_subfr] * gain_pit);
         L_tmp = (L_tmp + 0x2000) >> 14;

         if((L_tmp > MIN_16) & (L_tmp < 32768))
         {
            exc[i + i_subfr] = (Word16)L_tmp;
            tmp = (Word16)(abs(L_tmp));

            if(tmp > max)
            {
               max = tmp;
            }
         }
         else if(L_tmp > MAX_16)
         {
            exc[i + i_subfr] = MAX_16;
            max = MAX_16;
         }
         else
         {
            exc[i + i_subfr] = MIN_16;
            max = MAX_16;
         }
      }

      /* tmp = scaling possible according to max value of excitation */
      tmp = (Word16)((D_UTIL_norm_s(max) + Q_new) - 1);
      st->mem_subfr_q[3] = st->mem_subfr_q[2];
      st->mem_subfr_q[2] = st->mem_subfr_q[1];
      st->mem_subfr_q[1] = st->mem_subfr_q[0];
      st->mem_subfr_q[0] = tmp;

      /*
       * phase dispersion to enhance noise in low bit rate
       */

      /* L_gain_code in Q16 */
      D_UTIL_l_extract(L_gain_code, &gain_code, &gain_code_lo);

      if(mode <= MODE_7k)
      {
         j = 0;   /* high dispersion for rate <= 7.5 kbit/s */
      }
      else if(mode <= MODE_9k)
      {
         j = 1;   /* low dispersion for rate <= 9.6 kbit/s */
      }
      else
      {
         j = 2;   /* no dispersion for rate > 9.6 kbit/s */
      }

      D_ACELP_phase_dispersion(gain_code, gain_pit, code, (Word16)j,
         st->mem_ph_disp);

      /*
       * noise enhancer
       * - Enhance excitation on noise. (modify gain of code)
       *   If signal is noisy and LPC filter is stable, move gain
       *   of code 1.5 dB toward gain of code threshold.
       *   This decrease by 3 dB noise energy variation.
       */
      L_tmp = 16384 - (voice_fac >> 1);   /* 1=unvoiced, 0=voiced */
      fac = (Word16)((L_stab_fac * L_tmp) >> 15);
      L_tmp = L_gain_code;

      if(L_tmp < st->mem_gc_thres)
      {
         L_tmp = (L_tmp + D_UTIL_mpy_32_16(gain_code, gain_code_lo, 6226));

         if(L_tmp > st->mem_gc_thres)
         {
            L_tmp = st->mem_gc_thres;
         }
      }
      else
      {
         L_tmp = D_UTIL_mpy_32_16(gain_code, gain_code_lo, 27536);

         if(L_tmp < st->mem_gc_thres)
         {
            L_tmp = st->mem_gc_thres;
         }
      }
      st->mem_gc_thres = L_tmp;
      L_gain_code =
         D_UTIL_mpy_32_16(gain_code, gain_code_lo, (Word16)(32767 - fac));
      D_UTIL_l_extract(L_tmp, &gain_code, &gain_code_lo);
      L_gain_code =
         L_gain_code + D_UTIL_mpy_32_16(gain_code, gain_code_lo, fac);

      /*
       * pitch enhancer
       * - Enhance excitation on voice. (HP filtering of code)
       *   On voiced signal, filtering of code by a smooth fir HP
       *   filter to decrease energy of code in low frequency.
       */

      L_tmp2 = (voice_fac >> 3) + 4096;   /* 0.25=voiced, 0=unvoiced */
      L_tmp = (code[0] << 15) - (code[1] * L_tmp2);
      code2[0] = (L_tmp + 0x4000) >> 15;

      for(i = 1; i < L_SUBFR - 1; i++)
      {
         L_tmp = code[i] << 15;
         L_tmp = L_tmp - (code[i + 1] * L_tmp2);
         L_tmp = L_tmp - (code[i - 1] * L_tmp2);
         code2[i] = (L_tmp + 0x4000) >> 15;
      }

      L_tmp = code[L_SUBFR - 1] << 15;
      L_tmp = L_tmp - (code[L_SUBFR - 2] * L_tmp2);
      code2[L_SUBFR - 1] = (L_tmp + 0x4000) >> 15;

      /* build excitation */
      gain_code = (Word16)(((L_gain_code << Q_new) + 0x8000) >> 16);

      for(i = 0; i < L_SUBFR; i++)
      {
         L_tmp = (code2[i] * gain_code) << 5;
         L_tmp = L_tmp + (exc2[i] * gain_pit);
         L_tmp = (L_tmp + 0x2000) >> 14;

         exc2[i] = D_UTIL_saturate(L_tmp);
      }

      if(mode <= MODE_9k)
      {
         if(pit_sharp > 16384)
         {
            for(i = 0; i < L_SUBFR; i++)
            {
               L_tmp = (excp[i] + exc2[i]);
               excp[i] = D_UTIL_saturate(L_tmp);
            }

            D_GAIN_adaptive_control(exc2, excp, L_SUBFR);
            memcpy(exc2, excp, L_SUBFR * sizeof(Word16));
         }
      }

      if(mode <= MODE_7k)
      {
         j = (i_subfr >> 6);

         for(i = 0; i < M; i++)
         {
            L_tmp = isf_tmp[i] * (32767 - D_ROM_interpol_frac[j]);
            L_tmp = L_tmp + (isf[i] * D_ROM_interpol_frac[j]);
            HfIsf[i] = (Word16)((L_tmp + 0x4000) >> 15);
         }
      }
      else
      {
         memset(st->mem_syn_hf, 0, (M16k - M) * sizeof(Word16));
      }

      if(mode >= MODE_24k)
      {
         corr_gain = *prms++;
         D_UTIL_dec_synthesis(p_Aq, exc2, Q_new, &synth16k[i_subfr * 5 / 4],
            corr_gain, HfIsf, mode, newDTXState, bfi, st);
      }
      else
      {
         D_UTIL_dec_synthesis(p_Aq, exc2, Q_new, &synth16k[i_subfr * 5 / 4], 0,
            HfIsf, mode, newDTXState, bfi, st);
      }

      p_Aq += (M + 1);   /* interpolated LPC parameters for next subframe */
   }

   /*
    * Update signal for next frame
    * -> save past of exc[]
    * -> save pitch parameters.
    */

   memmove(st->mem_exc, &st->mem_exc[L_FRAME], (PIT_MAX + L_INTERPOL) * sizeof(Word16));
   D_UTIL_signal_down_scale(exc, L_FRAME, Q_new);
   D_DTX_activity_update(st->dtx_decSt, isf, exc);
   st->dtx_decSt->mem_dtx_global_state = (UWord8)newDTXState;
   st->mem_bfi = bfi;

   return(0);
}
