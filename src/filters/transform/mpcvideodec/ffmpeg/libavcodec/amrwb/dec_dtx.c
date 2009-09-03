/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "typedef.h"
#include "dec_dtx.h"
#include "dec_lpc.h"
#include "dec_util.h"

#define MAX_31                      (Word32)0x3FFFFFFF
#define L_FRAME                     256   /* Frame size                          */
#define RX_SPEECH_LOST              2
#define RX_SPEECH_BAD               3
#define RX_SID_FIRST                4
#define RX_SID_UPDATE               5
#define RX_SID_BAD                  6
#define RX_NO_DATA                  7
#define ISF_GAP                     128   /* 50                                  */
#define D_DTX_MAX_EMPTY_THRESH      50
#define GAIN_FACTOR                 75
#define ISF_FACTOR_LOW              256
#define ISF_FACTOR_STEP             2
#define ISF_DITH_GAP                448
#define D_DTX_HANG_CONST            7     /* yields eight frames of SP HANGOVER  */
#define D_DTX_ELAPSED_FRAMES_THRESH (24 + 7 - 1)
#define RANDOM_INITSEED             21845 /* own random init value               */


/*
 * D_DTX_reset
 *
 * Parameters:
 *    st             O: state struct
 *
 * Function:
 *    Initializes state memory
 *
 * Returns:
 *    non-zero with error, zero for ok
 */
int D_DTX_reset(D_DTX_State *st, const Word16 *isf_init)
{
   Word32 i;

   if(st == (D_DTX_State*)NULL)
   {
      return(-1);
   }
   st->mem_since_last_sid = 0;
   st->mem_true_sid_period_inv = (1 << 13);   /* 0.25 in Q15 */
   st->mem_log_en = 3500;
   st->mem_log_en_prev = 3500;

   /* low level noise for better performance in  DTX handover cases */
   st->mem_cng_seed = RANDOM_INITSEED;
   st->mem_hist_ptr = 0;

   /* Init isf_hist[] and decoder log frame energy */
   memcpy(st->mem_isf, isf_init, M * sizeof(Word16));
   memcpy(st->mem_isf_prev, isf_init, M * sizeof(Word16));

   for(i = 0; i < D_DTX_HIST_SIZE; i++)
   {
      memcpy(&st->mem_isf_buf[i * M], isf_init, M * sizeof(Word16));
      st->mem_log_en_buf[i] = 3500;
   }
   st->mem_dtx_hangover_count = D_DTX_HANG_CONST;
   st->mem_dec_ana_elapsed_count = 127;
   st->mem_sid_frame = 0;
   st->mem_valid_data = 0;
   st->mem_dtx_hangover_added = 0;
   st->mem_dtx_global_state = SPEECH;
   st->mem_data_updated = 0;
   st->mem_dither_seed = RANDOM_INITSEED;
   st->mem_cn_dith = 0;

   return(0);
}


/*
 * D_DTX_init
 *
 * Parameters:
 *    st           I/O: state struct
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    non-zero with error, zero for ok
 */
int D_DTX_init(D_DTX_State **st, const Word16 *isf_init)
{
   D_DTX_State *s;

   if(st == (D_DTX_State**)NULL)
   {
      return(-1);
   }

   *st = NULL;

   /* allocate memory */
   if((s = (D_DTX_State*)malloc(sizeof(D_DTX_State))) == NULL)
   {
      return(-1);
   }

   D_DTX_reset(s, isf_init);
   *st = s;

   return(0);
}


/*
 * D_DTX_exit
 *
 * Parameters:
 *    state        I/0: State struct
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    void
 */
void D_DTX_exit(D_DTX_State **st)
{
   if(st == NULL || *st == NULL)
   {
      return;
   }

   /* deallocate memory */
   free(*st);
   *st = NULL;

   return;
}


/*
 * D_DTX_rx_handler
 *
 * Parameters:
 *    st              I/O: State struct
 *    frame_type        I: Frame type
 *
 * Function:
 *    Analyze received frame
 *
 *    Table of new SPD synthesis states
 *
 *                          |       previous SPD_synthesis_state
 *    Incoming              |
 *    frame_type            | SPEECH       | DTX           | D_DTX_MUTE
 *    ---------------------------------------------------------------
 *    RX_SPEECH_GOOD ,      |              |               |
 *    RX_SPEECH_PR_DEGRADED | SPEECH       | SPEECH        | SPEECH
 *    ----------------------------------------------------------------
 *    RX_SPEECH_BAD,        | SPEECH       | DTX           | D_DTX_MUTE
 *    ----------------------------------------------------------------
 *    RX_SID_FIRST,         | DTX          | DTX/(D_DTX_MUTE)| D_DTX_MUTE
 *    ----------------------------------------------------------------
 *    RX_SID_UPDATE,        | DTX          | DTX           | DTX
 *    ----------------------------------------------------------------
 *    RX_SID_BAD,           | DTX          | DTX/(D_DTX_MUTE)| D_DTX_MUTE
 *    ----------------------------------------------------------------
 *    RX_NO_DATA,           | SPEECH       | DTX/(D_DTX_MUTE)| D_DTX_MUTE
 *    RX_SPARE              |(class2 garb.)|               |
 *    ----------------------------------------------------------------
 *
 * Returns:
 *    new state
 */
UWord8 D_DTX_rx_handler(D_DTX_State *st, UWord8 frame_type)
{
   UWord8 newState;
   UWord8 encState;

   /* DTX if SID frame or previously in DTX{_MUTE}
    * and (NO_RX OR BAD_SPEECH)
    */
   if((frame_type == RX_SID_FIRST) | (frame_type == RX_SID_UPDATE) |
      (frame_type == RX_SID_BAD) | (((st->mem_dtx_global_state == DTX) |
      (st->mem_dtx_global_state == D_DTX_MUTE)) & ((frame_type == RX_NO_DATA) |
      (frame_type == RX_SPEECH_BAD) | (frame_type == RX_SPEECH_LOST))))
   {
      newState = DTX;

      /* stay in mute for these input types */
      if((st->mem_dtx_global_state == D_DTX_MUTE) &
         ((frame_type == RX_SID_BAD) | (frame_type == RX_SID_FIRST) |
         (frame_type == RX_SPEECH_LOST) | (frame_type == RX_NO_DATA)))
      {
         newState = D_DTX_MUTE;
      }

      /* evaluate if noise parameters are too old                     */
      /* since_last_sid is reset when CN parameters have been updated */
      st->mem_since_last_sid = D_UTIL_saturate(st->mem_since_last_sid + 1);

      /* no update of sid parameters in DTX for a Word32 while */
      if(st->mem_since_last_sid > D_DTX_MAX_EMPTY_THRESH)
      {
         newState = D_DTX_MUTE;
      }
   }
   else
   {
      newState = SPEECH;
      st->mem_since_last_sid = 0;
   }

   /*
    * reset the decAnaElapsed Counter when receiving CNI data the first
    * time, to robustify counter missmatch after handover
    * this might delay the bwd CNI analysis in the new decoder slightly.
    */
   if((st->mem_data_updated == 0) & (frame_type == RX_SID_UPDATE))
   {
      st->mem_dec_ana_elapsed_count = 0;
   }

   /*
    * update the SPE-SPD DTX hangover synchronization
    * to know when SPE has added dtx hangover
    */
   st->mem_dec_ana_elapsed_count++;

   /* saturate */
   if(st->mem_dec_ana_elapsed_count > 127)
   {
      st->mem_dec_ana_elapsed_count = 127;
   }

   st->mem_dtx_hangover_added = 0;

   if((frame_type == RX_SID_FIRST) | (frame_type == RX_SID_UPDATE) |
      (frame_type == RX_SID_BAD) | (frame_type == RX_NO_DATA))
   {
      encState = DTX;
   }
   else
   {
      encState = SPEECH;
   }

   if(encState == SPEECH)
   {
      st->mem_dtx_hangover_count = D_DTX_HANG_CONST;
   }
   else
   {
      if(st->mem_dec_ana_elapsed_count > D_DTX_ELAPSED_FRAMES_THRESH)
      {
         st->mem_dtx_hangover_added = 1;
         st->mem_dec_ana_elapsed_count = 0;
         st->mem_dtx_hangover_count = 0;
      }
      else if(st->mem_dtx_hangover_count == 0)
      {
         st->mem_dec_ana_elapsed_count = 0;
      }
      else
      {
         st->mem_dtx_hangover_count--;
      }
   }

   if(newState != SPEECH)
   {
      /*
       * DTX or D_DTX_MUTE
       * CN data is not in a first SID, first SIDs are marked as SID_BAD
       *  but will do backwards analysis if a hangover period has been added
       *  according to the state machine above
       */
      st->mem_sid_frame = 0;
      st->mem_valid_data = 0;

      if(frame_type == RX_SID_FIRST)
      {
         st->mem_sid_frame = 1;
      }
      else if(frame_type == RX_SID_UPDATE)
      {
         st->mem_sid_frame = 1;
         st->mem_valid_data = 1;
      }
      else if(frame_type == RX_SID_BAD)
      {
         st->mem_sid_frame = 1;
         st->mem_dtx_hangover_added = 0;   /* use old data */
      }
   }

   return newState;

   /* newState is used by both SPEECH AND DTX synthesis routines */
}


/*
 * D_DTX_cn_dithering
 *
 * Parameters:
 *    isf             I/O: CN ISF vector
 *    L_log_en_int    I/O: energy parameter
 *    dither_seed     I/O: random seed
 *
 * Function:
 *    Confort noise dithering
 *
 * Returns:
 *    void
 */
static void D_DTX_cn_dithering(Word16 isf[M], Word32 *L_log_en_int,
                               Word16 *dither_seed)
{
   Word32 temp, temp1, i, dither_fac, rand_dith,rand_dith2;

   /* Insert comfort noise dithering for energy parameter */
   rand_dith = D_UTIL_random(dither_seed) >> 1;
   rand_dith2 = D_UTIL_random(dither_seed) >>1;
   rand_dith = rand_dith + rand_dith2;
   *L_log_en_int = *L_log_en_int + ((rand_dith * GAIN_FACTOR) << 1);

   if(*L_log_en_int < 0)
   {
      *L_log_en_int = 0;
   }

   /* Insert comfort noise dithering for spectral parameters (ISF-vector) */
   dither_fac = ISF_FACTOR_LOW;
   rand_dith = D_UTIL_random(dither_seed) >> 1;
   rand_dith2 = D_UTIL_random(dither_seed) >> 1;
   rand_dith = rand_dith + rand_dith2;
   temp = isf[0] + (((rand_dith * dither_fac) + 0x4000) >> 15);

   /* Make sure that isf[0] will not get negative values */
   if(temp < ISF_GAP)
   {
      isf[0] = ISF_GAP;
   }
   else
   {
      isf[0] = (Word16)temp;
   }

   for(i = 1; i < M - 1; i++)
   {
      dither_fac = dither_fac + ISF_FACTOR_STEP;
      rand_dith = D_UTIL_random(dither_seed) >> 1;
      rand_dith2 = D_UTIL_random(dither_seed) >> 1;
      rand_dith = rand_dith + rand_dith2;
      temp = isf[i] + (((rand_dith * dither_fac) + 0x4000) >> 15);
      temp1 = temp - isf[i - 1];

      /* Make sure that isf spacing remains at least ISF_DITH_GAP Hz */
      if(temp1 < ISF_DITH_GAP)
      {
         isf[i] = (Word16)(isf[i - 1] + ISF_DITH_GAP);
      }
      else
      {
         isf[i] = (Word16)temp;
      }
   }

   /* Make sure that isf[M-2] will not get values above 16384 */
   if(isf[M - 2] > 16384)
   {
      isf[M - 2] = 16384;
   }

   return;
}


/*
 * D_DTX_exe
 *
 * Parameters:
 *    st           I/O: state struct
 *    exc2           O: CN excitation
 *    new_state      I: New DTX state
 *    prms           I: Vector of synthesis parameters
 *    isf            O: CN ISF vector
 *
 * Function:
 *    Confort noise generation
 *
 * Returns:
 *    void
 */
void D_DTX_exe(D_DTX_State *st, Word16 *exc2, Word16 new_state, Word16 isf[],
               Word16 **prms)
{

   Word32 i, j, L_tmp, ptr;
   Word32 exp0, int_fac;
   Word32 gain;
   Word32 L_isf[M], L_log_en_int, level32, ener32;
   Word16 log_en_index;
   Word16 tmp_int_length;
   Word16 exp, log_en_int_e, log_en_int_m, level;


   /*
    * This function is called if synthesis state is not SPEECH.
    * The globally passed inputs to this function are
    *    st->sid_frame
    *    st->valid_data
    *    st->dtxHangoverAdded
    *    new_state (SPEECH, DTX, D_DTX_MUTE)
    */
   if((st->mem_dtx_hangover_added != 0) & (st->mem_sid_frame != 0))
   {
      /* sid_first after dtx hangover period
       * or sid_upd after dtxhangover
       * consider twice the last frame
       */
      ptr = st->mem_hist_ptr + 1;

      if(ptr == D_DTX_HIST_SIZE)
      {
         ptr = 0;
      }

      memcpy(&st->mem_isf_buf[ptr * M], &st->mem_isf_buf[st->mem_hist_ptr * M],
         M * sizeof(Word16));

      st->mem_log_en_buf[ptr] = st->mem_log_en_buf[st->mem_hist_ptr];

      /* compute mean log energy and isf from decoded signal (SID_FIRST) */
      st->mem_log_en = 0;
      memset(L_isf, 0, M * sizeof(Word32));

      /* average energy and isf */
      for(i = 0; i < D_DTX_HIST_SIZE; i++)
      {
         /*
          * Division by D_DTX_HIST_SIZE = 8 has been done in dtx_buffer log_en
          * is in Q10
          */
         st->mem_log_en = (Word16)(st->mem_log_en + st->mem_log_en_buf[i]);

         for(j = 0; j < M; j++)
         {
            L_isf[j] = L_isf[j] + st->mem_isf_buf[i * M + j];
         }
      }

      /* st->log_en in Q9 */
      st->mem_log_en = (Word16)(st->mem_log_en >> 1);

      /*
       * Add 2 in Q9, in order to have only positive values for Pow2
       * this value is subtracted back after Pow2 function
       */
      st->mem_log_en = (Word16)(st->mem_log_en + 1024);

      if(st->mem_log_en < 0)
      {
         st->mem_log_en = 0;
      }

      for(j = 0; j < M; j++)
      {
         st->mem_isf[j] = (Word16)(L_isf[j]>>3);   /* divide by 8 */
      }
   }

   if(st->mem_sid_frame != 0)
   {
      /*
       * Set old SID parameters, always shift
       * even if there is no new valid_data
       */
      memcpy(st->mem_isf_prev, st->mem_isf, M * sizeof(Word16));
      st->mem_log_en_prev = st->mem_log_en;

      if(st->mem_valid_data != 0) /* new data available (no CRC) */
      {
         /* st->true_sid_period_inv = 1.0f/st->since_last_sid; */

         /*
          * Compute interpolation factor, since the division only works
          * for values of since_last_sid < 32 we have to limit
          * the interpolation to 32 frames
          */
         tmp_int_length = st->mem_since_last_sid;

         if(tmp_int_length > 32)
         {
            tmp_int_length = 32;
         }

         if(tmp_int_length >= 2)
         {
            st->mem_true_sid_period_inv =
               (Word16)(0x2000000 / (tmp_int_length << 10));
         }
         else
         {
            st->mem_true_sid_period_inv = 1 << 14;   /* 0.5 it Q15 */
         }

         D_LPC_isf_noise_d(*prms, st->mem_isf);
         (*prms) += 5;
         log_en_index = *(*prms)++;

         /* read background noise stationarity information */
         st->mem_cn_dith = *(*prms)++;

         /*
          * st->log_en = (Float32)log_en_index / 2.625 - 2.0;
          * log2(E) in Q9 (log2(E) lies in between -2:22)
          */
         st->mem_log_en = (Word16)(log_en_index << (15 - 6));

         /* Divide by 2.625  */
         st->mem_log_en = (Word16)((st->mem_log_en * 12483) >> 15);

         /*
          * Subtract 2 in Q9 is done later, after Pow2 function
          * no interpolation at startup after coder reset
          * or when SID_UPD has been received right after SPEECH
          */
         if((st->mem_data_updated == 0) ||
            (st->mem_dtx_global_state == SPEECH))
         {
            memcpy(st->mem_isf_prev, st->mem_isf, M * sizeof(Word16));
            st->mem_log_en_prev = st->mem_log_en;
         }
      }   /* endif valid_data */
   }   /* endif sid_frame */

   if((st->mem_sid_frame != 0) && (st->mem_valid_data != 0))
   {
      st->mem_since_last_sid = 0;
   }

   /* Interpolate SID info */
   if(st->mem_since_last_sid < 32)
   {
      int_fac = st->mem_since_last_sid << 10;   /* Q10 */
   }
   else
   {
      int_fac = 32767;
   }
   /* Q10 * Q15 -> Q10 */
   int_fac = (int_fac * st->mem_true_sid_period_inv) >> 15;

   /* Maximize to 1.0 in Q10 */
   if(int_fac > 1024)
   {
      int_fac = 1024;
   }
   int_fac = int_fac << 4;   /* Q10 -> Q14 */
   L_log_en_int = (int_fac * st->mem_log_en) << 1;   /* Q14 * Q9 -> Q24 */

   for(i = 0; i < M; i++)
   {
      /* Q14 * Q15 -> Q14 */
      isf[i] = (Word16)((int_fac * st->mem_isf[i]) >> 15);
   }
   int_fac = 16384 - int_fac;   /* 1-k in Q14 */

   /* ( Q14 * Q9 -> Q24 ) + Q24 -> Q24 */
   L_log_en_int = L_log_en_int + ((int_fac * st->mem_log_en_prev) << 1);

   for(i = 0; i < M; i++)
   {
      /* Q14 + (Q14 * Q15 -> Q14) -> Q14 */
      L_tmp = isf[i] + ((int_fac * st->mem_isf_prev[i]) >> 15);
      isf[i] = (Word16)(L_tmp << 1);   /* Q14 -> Q15 */
   }

   /* If background noise is non-stationary, insert comfort noise dithering */
   if(st->mem_cn_dith != 0)
   {
      D_DTX_cn_dithering(isf, &L_log_en_int, &st->mem_dither_seed);
   }

   /* L_log_en_int corresponds to log2(E)+2 in Q24, i.e log2(gain)+1 in Q25 */
   L_log_en_int = (L_log_en_int >> 9); /* Q25 -> Q16 */

   /* Find integer part  */
   log_en_int_e = (Word16)((L_log_en_int)>>16);

   /* Find fractional part */
   log_en_int_m = (Word16)((L_log_en_int - (log_en_int_e << 16)) >> 1);

   /*
    * Subtract 2 from L_log_en_int in Q9,
    * i.e divide the gain by 2 (energy by 4)
    * Add 16 in order to have the result of pow2 in Q16
    */
   log_en_int_e = (Word16)(log_en_int_e + (16 - 1));

   /* level = (Float32)( pow( 2.0f, log_en ) );  */
   level32 = D_UTIL_pow2(log_en_int_e, log_en_int_m);   /* Q16 */
   exp0 = D_UTIL_norm_l(level32);
   level32 = (level32 << exp0);   /* level in Q31 */
   exp0 = (15 - exp0);
   level = (Word16)(level32 >> 16);   /* level in Q15 */

   /* generate white noise vector */
   for(i = 0; i < L_FRAME; i++)
   {
      exc2[i] = (Word16)((D_UTIL_random(&(st->mem_cng_seed)) >> 4));
   }

   /* gain = level / sqrt(ener) * sqrt(L_FRAME) */
   /* energy of generated excitation */
   ener32 = D_UTIL_dot_product12(exc2, exc2, L_FRAME, &exp);
   D_UTIL_normalised_inverse_sqrt(&ener32, &exp);
   gain = ener32 >>16;
   gain = (level * gain) >> 15;   /* gain in Q15 */

   /* Multiply by sqrt(L_FRAME)=16, i.e. shift left by 4 */
   exp = (Word16)(exp0 + exp  + 4);

   if(exp >= 0)
   {
      for(i = 0; i < L_FRAME; i++)
      {
         L_tmp = (exc2[i] * gain) >> 15;   /* Q0 * Q15 */
         exc2[i] = (Word16)(L_tmp << exp);
      }
   }
   else
   {
      exp = (Word16)-exp;

      for(i = 0; i < L_FRAME; i++)
      {
         L_tmp = (exc2[i] * gain) >> 15;   /* Q0 * Q15 */
         exc2[i] = (Word16)(L_tmp >> exp);
      }
   }

   if(new_state == D_DTX_MUTE)
   {
      /*
       * mute comfort noise as it has been quite a long time since
       * last SID update was performed
       */
      tmp_int_length = st->mem_since_last_sid;

      if(tmp_int_length > 32)
      {
         tmp_int_length = 32;
      }

      st->mem_true_sid_period_inv = D_UTIL_saturate((0x02000000 / (tmp_int_length << 10)));
      st->mem_since_last_sid = 0;
      st->mem_log_en_prev = st->mem_log_en;

      /* subtract 1/8 in Q9 (energy), i.e -3/8 dB */
      st->mem_log_en = D_UTIL_saturate(st->mem_log_en - 64);
   }

   /* reset interpolation length timer if data has been updated.        */
   if((st->mem_sid_frame != 0) && ((st->mem_valid_data != 0) ||
      ((st->mem_valid_data == 0) && (st->mem_dtx_hangover_added) != 0)))
   {
      st->mem_since_last_sid = 0;
      st->mem_data_updated = 1;
   }

   return;
}


/*
 * D_DTX_activity_update
 *
 * Parameters:
 *    st           I/O: state struct
 *    isf            O: ISF vector
 *    exc            O: excitation
 *
 * Function:
 *    Confort noise generation
 *
 * Returns:
 *    void
 */
void D_DTX_activity_update(D_DTX_State *st, Word16 isf[], Word16 exc[])
{

   Word32 L_frame_en, log_en;
   Word32 i;
   Word16 log_en_e, log_en_m;

   st->mem_hist_ptr = (Word16)(st->mem_hist_ptr + 1);

   if(st->mem_hist_ptr == D_DTX_HIST_SIZE)
   {
      st->mem_hist_ptr = 0;
   }

   memcpy(&st->mem_isf_buf[st->mem_hist_ptr * M], isf, M * sizeof(Word16));

   /* compute log energy based on excitation frame energy in Q0 */
   L_frame_en = 0;

   for(i = 0; i < L_FRAME; i++)
   {
      L_frame_en = L_frame_en + (exc[i] * exc[i]);
      if (L_frame_en > MAX_31)
      {
         L_frame_en = MAX_31;
         break;
      }
   }

   /*
    * log_en =
    * (Float32)log10(L_frame_en/(Float32)L_FRAME)/(Float32)log10(2.0f);
    */
   D_UTIL_log2(L_frame_en, &log_en_e, &log_en_m);

   /*
    * convert exponent and mantissa to Word16 Q7.
    * Q7 is used to simplify averaging in dtx_enc
    */
   log_en = log_en_e << 7;   /* Q7 */
   log_en = log_en + (log_en_m >> (15 - 7));

   /* Divide by L_FRAME = 256, i.e subtract 8 in Q7 = 1024 */
   log_en = log_en - 1024;

   /* insert into log energy buffer */
   st->mem_log_en_buf[st->mem_hist_ptr] = (Word16)log_en;

   return;
}
