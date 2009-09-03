/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "typedef.h"
#include "enc_lpc.h"
#include "enc_util.h"


#define DTX_HIST_SIZE_MIN_ONE       7
#define DTX_HANG_CONST              7     /* yields eight frames of SP HANGOVER  */
#define DTX_ELAPSED_FRAMES_THRESH   (24 + 7 -1)
#define MED_THRESH                  2.25
#define GAIN_THR                    1.406
#define ORDER                       16    /* order of linear prediction filter   */
#define RANDOM_INITSEED             21845 /* own random init value               */
#define MRDTX                       9

#define SIZE_BK_NOISE1  64
#define SIZE_BK_NOISE2  64
#define SIZE_BK_NOISE3  64
#define SIZE_BK_NOISE4  32
#define SIZE_BK_NOISE5  32

#define FRAME_LEN 256   /* Length (samples) of the input frame */
#define SCALE     128   /* (UNITY * UNITY) / 512               */
#define TONE_THR 0.65f  /* Threshold for tone detection        */

/* constants for speech level estimation */
#define SP_EST_COUNT       80
#define SP_ACTIVITY_COUNT  25
#define ALPHA_SP_UP     (1.0f - 0.85f)
#define ALPHA_SP_DOWN   (1.0f - 0.85f)

#define NOM_LEVEL          2050.0F              /* about -26 dBov                */
#define SPEECH_LEVEL_INIT  NOM_LEVEL
#define MIN_SPEECH_LEVEL1  (NOM_LEVEL * 0.063F) /* NOM_LEVEL -24 dB              */
#define MIN_SPEECH_LEVEL2  (NOM_LEVEL * 0.2F)   /* NOM_LEVEL -14 dB              */
#define MIN_SPEECH_SNR     0.125F               /* 0 dB, lowest SNR estimation   */

/* Constants for background spectrum update */
#define ALPHA_UP1   (1.0f - 0.95f)  /* Normal update, upwards:   */
#define ALPHA_DOWN1 (1.0f - 0.936f) /* Normal update, downwards  */
#define ALPHA_UP2   (1.0f - 0.985f) /* Forced update, upwards    */
#define ALPHA_DOWN2 (1.0f - 0.943f) /* Forced update, downwards  */
#define ALPHA3      (1.0f - 0.95f)  /* Update downwards          */
#define ALPHA4      (1.0f - 0.9f)   /* For stationary estimation */
#define ALPHA5      (1.0f - 0.5f)   /* For stationary estimation */

/* Constants for VAD threshold */
#define THR_MIN   (1.6F * SCALE) /* Minimum threshold                            */
#define THR_HIGH  (6.0F * SCALE) /* Highest threshold                            */
#define THR_LOW   (1.7F * SCALE) /* Lowest threshold                             */
#define NO_P1     31744.0F       /* ilog2(1), Noise level for highest threshold  */
#define NO_P2     19786.0F       /* ilog2(0.1, Noise level for lowest threshold  */
#define NO_SLOPE  ((Float32)(THR_LOW - THR_HIGH) / (Float32)(NO_P2 - NO_P1))
#define SP_CH_MIN (-0.75F * SCALE)
#define SP_CH_MAX (0.75F * SCALE)
#define SP_P1     22527.0F       /* ilog2(NOM_LEVEL / 4)                         */
#define SP_P2     17832.0F       /* ilog2(NOM_LEVEL * 4)                         */
#define SP_SLOPE  ((Float32)(SP_CH_MAX - SP_CH_MIN) / (Float32)(SP_P2 - SP_P1))

/* Constants for hangover length */
#define HANG_HIGH 12          /* longest hangover                 */
#define HANG_LOW  2           /* shortest hangover                */
#define HANG_P1   THR_LOW     /* threshold for longest hangover   */
#define HANG_P2   (4 * SCALE) /* threshold for Word16est hangover */
#define HANG_SLOPE ((Float32)(HANG_LOW - HANG_HIGH) / (Float32)(HANG_P2 - HANG_P1))

/* Constants for burst length */
#define BURST_HIGH   8        /* longest burst length          */
#define BURST_LOW    3        /* shortest burst length         */
#define BURST_P1     THR_HIGH /* threshold for Word32est burst */
#define BURST_P2     THR_LOW  /* threshold for Word16est burst */
#define BURST_SLOPE  ((Float32)(BURST_LOW - BURST_HIGH) / (Float32)(BURST_P2 - BURST_P1))

/* Parameters for background spectrum recovery function */
#define STAT_COUNT      20    /* threshold of stationary detection counter         */
#define STAT_THR_LEVEL  184   /* Threshold level for stationarity detection        */
#define STAT_THR        1000  /* Threshold for stationarity detection              */

/* Limits for background noise estimate */
#define NOISE_MIN    40    /* minimum */
#define NOISE_MAX    20000 /* maximum */
#define NOISE_INIT   150   /* initial */

/* Thresholds for signal power (now calculated on 2 frames) */
#define VAD_POW_LOW        30000.0f   /* If input power is lower than this, VAD is set to 0  */
#define POW_PITCH_TONE_THR 686080.0f  /* If input power is lower, pitch detection is ignored */

/* Constants for the filter bank */
#define COEFF3   0.407806f /* coefficient for the 3rd order filter     */
#define COEFF5_1 0.670013f /* 1st coefficient the for 5th order filter */
#define COEFF5_2 0.195007f /* 2nd coefficient the for 5th order filter */

extern const Float32 E_ROM_en_adjust[];
extern const Float32 E_ROM_mean_isf_noise[];
extern const Float32 E_ROM_dico1_isf_noise[];
extern const Float32 E_ROM_dico2_isf_noise[];
extern const Float32 E_ROM_dico3_isf_noise[];
extern const Float32 E_ROM_dico4_isf_noise[];
extern const Float32 E_ROM_dico5_isf_noise[];
extern const Float32 E_ROM_isf[];


/*
 * E_DTX_isf_history_aver
 *
 * Parameters:
 *    isf_old         I/O: ISF vectors
 *    indices           I: ISF indices
 *    isf_aver          O: averaged ISFs
 *
 * Function:
 *    Perform the ISF averaging
 *
 * Returns:
 *    void
 */
static void E_DTX_isf_history_aver(Float32 isf_old[], Word16 indices[],
                                   Float32 isf_aver[])
{
   Float32 isf_tmp[2 * M];
   Float32 tmp;
   Word32 i, j, k;

   /*
    * Memorize in isf_tmp[][] the ISF vectors to be replaced by
    * the median ISF vector prior to the averaging
    */
   for (k = 0; k < 2; k++)
   {
      if (indices[k] != -1)
      {
         for (i = 0; i < M; i++)
         {
            isf_tmp[k * M + i] = isf_old[indices[k] * M + i];
            isf_old[indices[k] * M + i] = isf_old[indices[2] * M + i];
         }
      }
   }

   /* Perform the ISF averaging */
   for (j = 0; j < M; j++)
   {
      tmp = 0;

      for (i = 0; i < DTX_HIST_SIZE; i++)
      {
         tmp += isf_old[i * M + j];
      }

      isf_aver[j] = tmp;
   }

   /* Retrieve from isf_tmp[][] the ISF vectors saved prior to averaging */
   for (k = 0; k < 2; k++)
   {
      if (indices[k] != -1)
      {
         for (i = 0; i < M; i++)
         {
            isf_old[indices[k] * M + i] = isf_tmp[k * M + i];
         }
      }
   }

   return;
}

/*
 * E_DTX_dithering_control
 *
 * Parameters:
 *    st                I: state struct
 *
 * Function:
 *    Analysis of the variation and stationarity
 *    of the background noise.
 *
 * Returns:
 *    Dithering decision
 */
static Word16 E_DTX_dithering_control(E_DTX_State * st)
{
   Float32 ISF_diff, gain_diff, mean, tmp;
   Word32 i;
   Word16 CN_dith;

   /* determine how stationary the spectrum of background noise is */
   ISF_diff = 0.0F;

   for (i = 0; i < 8; i++)
   {
      ISF_diff += st->mem_distance_sum[i];
   }

   if (ISF_diff > 5147609.0f)
   {
      CN_dith = 1;
   }
   else
   {
      CN_dith = 0;
   }

   /* determine how stationary the energy of background noise is */
   mean = 0.0f;

   for (i = 0; i < DTX_HIST_SIZE; i++)
   {
      mean += st->mem_log_en[i] / (Float32)DTX_HIST_SIZE;
   }

   gain_diff = 0.0f;

   for (i = 0; i < DTX_HIST_SIZE; i++)
   {
      tmp = (Float32)fabs(st->mem_log_en[i] - mean);
      gain_diff += tmp;
   }

   if (gain_diff > GAIN_THR)
   {
      CN_dith = 1;
   }

   return CN_dith;
}

/*
 * E_DTX_buffer
 *
 * Parameters:
 *    st           I/O: state struct
 *    isf_new        I: isf vector
 *    enr            I: residual energy (for L_FRAME)
 *    codec_mode     I: speech coder mode
 *
 * Function:
 *    Handles the DTX buffer
 *
 * Returns:
 *    void
 */
void E_DTX_buffer(E_DTX_State *st, Float32 isf_new[], Float32 enr,
                  Word16 codec_mode)
{
   Float32 log_en;

   /* update pointer to circular buffer */
   st->mem_hist_ptr++;

   if (st->mem_hist_ptr == DTX_HIST_SIZE)
   {
      st->mem_hist_ptr = 0;
   }

   /* copy isf vector into buffer */
   memcpy(&st->mem_isf[st->mem_hist_ptr * M], isf_new, M * sizeof(Float32));

   enr += 1e-10F;

   log_en = (Float32)(log10(enr / ((Float64)L_FRAME)) / log10(2.0F));

   /* Subtract ~ 3 dB */
   st->mem_log_en[st->mem_hist_ptr] = log_en + E_ROM_en_adjust[codec_mode];

   return;
}

/*
 * E_DTX_frame_indices_find
 *
 * Parameters:
 *    st           I/O: state struct
 *    isf_old_tx     I: isf vector
 *    indices        I: distance indices
 *
 * Function:
 *    Find indices for min/max distances
 *
 * Returns:
 *    void
 */
static void E_DTX_frame_indices_find(E_DTX_State * st, Word16 indices[])
{
   Float32 L_tmp, tmp, summin, summax, summax2nd;
   Word32 i, j, k;
   Word16 ptr;

   /*
    * Remove the effect of the oldest frame from the column
    * sum sumD[0..E_DTX_HIST_SIZE-1]. sumD[E_DTX_HIST_SIZE] is
    * not updated since it will be removed later.
    */

   k = DTX_HIST_SIZE_MIN_ONE;
   j = -1;

   for (i = 0; i < DTX_HIST_SIZE_MIN_ONE; i++)
   {
      j = j + k;
      st->mem_distance_sum[i] = st->mem_distance_sum[i] - st->mem_distance[j];
      k--;
   }

   /*
    * Shift the column sum sumD. The element sumD[E_DTX_HIST_SIZE-1]
    * corresponding to the oldest frame is removed. The sum of
    * the distances between the latest isf and other isfs,
    * i.e. the element sumD[0], will be computed during this call.
    * Hence this element is initialized to zero.
    */

   for (i = DTX_HIST_SIZE_MIN_ONE; i > 0; i--)
   {
      st->mem_distance_sum[i] = st->mem_distance_sum[i - 1];
   }
   st->mem_distance_sum[0] = 0.0F;

   /*
    * Remove the oldest frame from the distance matrix.
    * Note that the distance matrix is replaced by a one-
    * dimensional array to save static memory.
    */

   k = 0;

   for (i = 27; i >= 12; i = i - k)
   {
      k++;
      for (j = k; j > 0; j--)
      {
         st->mem_distance[i - j + 1] = st->mem_distance[i - j - k];
      }
   }

   /*
    * Compute the first column of the distance matrix D
    * (squared Euclidean distances from isf1[] to isf_old_tx[][]).
    */

   ptr = st->mem_hist_ptr;

   for (i = 1; i < DTX_HIST_SIZE; i++)
   {
      /* Compute the distance between the latest isf and the other isfs. */
      ptr--;

      if (ptr < 0)
      {
         ptr = DTX_HIST_SIZE_MIN_ONE;
      }
      L_tmp = 0;

      for (j = 0; j < M; j++)
      {
         tmp = st->mem_isf[st->mem_hist_ptr * M + j] - st->mem_isf[ptr * M + j];
         L_tmp += tmp * tmp;
      }

      st->mem_distance[i - 1] = L_tmp;

      /* Update also the column sums. */
      st->mem_distance_sum[0] += st->mem_distance[i - 1];
      st->mem_distance_sum[i] += st->mem_distance[i - 1];
   }

   /* Find the minimum and maximum distances */
   summax = st->mem_distance_sum[0];
   summin = st->mem_distance_sum[0];
   indices[0] = 0;
   indices[2] = 0;

   for (i = 1; i < DTX_HIST_SIZE; i++)
   {
      if (st->mem_distance_sum[i] > summax)
      {
         indices[0] = (Word16)i;
         summax = st->mem_distance_sum[i];
      }

      if (st->mem_distance_sum[i] < summin)
      {
         indices[2] = (Word16)i;
         summin = st->mem_distance_sum[i];
      }
   }

   /* Find the second largest distance */
   summax2nd = -100000000.0;
   indices[1] = -1;
   for (i = 0; i < DTX_HIST_SIZE; i++)
   {
      if ((st->mem_distance_sum[i] > summax2nd) && (i != indices[0]))
      {
         indices[1] = (Word16)i;
         summax2nd = st->mem_distance_sum[i];
      }
   }

   for (i = 0; i < 3; i++)
   {
      indices[i] = (Word16)(st->mem_hist_ptr - indices[i]);
      if (indices[i] < 0)
      {
         indices[i] += DTX_HIST_SIZE;
      }
   }

   /*
    * If maximum distance / MED_THRESH is smaller than minimum distance
    * then the median ISF vector replacement is not performed
    */
   L_tmp = (Float32)(summax / MED_THRESH);

   if (L_tmp <= summin)
   {
      indices[0] = -1;
   }

   /*
    * If second largest distance/MED_THRESH is smaller than
    * minimum distance then the median ISF vector replacement is
    * not performed
    */
   L_tmp = (Float32)(summax2nd / MED_THRESH);

   if (L_tmp <= summin)
   {
      indices[1] = -1;
   }

   return;
}

/*
 * E_DTX_isf_q
 *
 * Parameters:
 *    isf            I: ISF in the frequency domain (0..6400)
 *    isf_q          O: quantised ISF
 *    indice         O: quantisation indices
 *
 * Function:
 *    The ISF vector is quantized using VQ with split-by-5
 *
 * Returns:
 *    void
 */
static void E_DTX_isf_q(Float32 *isf, Word16 **indice)
{
   Word32 i;
   Float32 tmp;

   for (i = 0; i < ORDER; i++)
   {
      isf[i] = isf[i] - E_ROM_mean_isf_noise[i];
   }

   (*indice)[0] = E_LPC_isf_sub_vq(&isf[0], E_ROM_dico1_isf_noise, 2,
      SIZE_BK_NOISE1, &tmp);
   (*indice)[1] = E_LPC_isf_sub_vq(&isf[2], E_ROM_dico2_isf_noise, 3,
      SIZE_BK_NOISE2, &tmp);
   (*indice)[2] = E_LPC_isf_sub_vq(&isf[5], E_ROM_dico3_isf_noise, 3,
      SIZE_BK_NOISE3, &tmp);
   (*indice)[3] = E_LPC_isf_sub_vq(&isf[8], E_ROM_dico4_isf_noise, 4,
      SIZE_BK_NOISE4, &tmp);
   (*indice)[4] = E_LPC_isf_sub_vq(&isf[12], E_ROM_dico5_isf_noise, 4,
      SIZE_BK_NOISE5, &tmp);

   return;
}

/*
 * E_DTX_exe
 *
 * Parameters:
 *    st           I/O: state struct
 *    exc2           O: CN excitation
 *    pt_prms        O: analysis parameters
 *
 * Function:
 *    Confort noise parameters are encoded for the SID frame
 *
 * Returns:
 *    void
 */
void E_DTX_exe(E_DTX_State *st, Float32 *exc2, Word16 **pt_prms)
{
   Float32 isf[M];
   Float32 log_en, level, gain, ener;
   Word32 i,j;
   Word16 isf_order[3];
   Word16 CN_dith;

   /* VOX mode computation of SID parameters */

   log_en = 0.0F;
   memset(isf, 0, M * sizeof(Float32));

   /* average energy and isf */
   for (i = 0; i < DTX_HIST_SIZE; i++)
   {
      log_en += st->mem_log_en[i] / (Float32)DTX_HIST_SIZE;
   }

   E_DTX_frame_indices_find(st, isf_order);
   E_DTX_isf_history_aver(st->mem_isf, isf_order, isf);

   for (j = 0; j < M; j++)
   {
      isf[j] = isf[j] / (Float32)DTX_HIST_SIZE;   /* divide by 8 */
   }

   /*  quantize logarithmic energy to 6 bits (-6 : 66 dB) */

   st->mem_log_en_index = (Word16)((log_en + 2.0F) * 2.625F);

   if(st->mem_log_en_index > 63)
   {
      st->mem_log_en_index = 63;
   }

   if(st->mem_log_en_index < 0)
   {
      st->mem_log_en_index = 0;
   }

   E_DTX_isf_q(isf, pt_prms);
   (*pt_prms) += 5;

   **pt_prms = st->mem_log_en_index;
   (*pt_prms) += 1;

   CN_dith = E_DTX_dithering_control(st);

   **pt_prms = CN_dith;
   (*pt_prms) += 1;

   /* adjust level to speech coder mode */

   log_en = (Float32)((Float32)st->mem_log_en_index / 2.625 - 2.0);
   level = (Float32)(pow( 2.0, log_en ));

   /* generate white noise vector */

   for (i = 0; i < L_FRAME; i++)
   {
      exc2[i] = (Float32)E_UTIL_random(&(st->mem_cng_seed));
   }

   ener = 0.01F;

   for (i = 0; i < L_FRAME; i++)
   {
      ener += exc2[i] * exc2[i];
   }

   gain = (Float32)sqrt(level * L_FRAME / ener);

   for (i = 0; i < L_FRAME; i++)
   {
      exc2[i] *= gain;
   }

   return;
}

/*
 * E_DTX_reset
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
Word32 E_DTX_reset(E_DTX_State *st)
{
   Word32 i;

   if (st == (E_DTX_State *) NULL)
   {
      return -1;
   }

   st->mem_hist_ptr = 0;
   st->mem_log_en_index = 0;

   /* Init isf_hist[] */
   for(i = 0; i < DTX_HIST_SIZE; i++)
   {
      memcpy(&st->mem_isf[i * M], E_ROM_isf, M * sizeof(Float32));
   }

   st->mem_cng_seed = RANDOM_INITSEED;

   /* Reset energy history */
   memset(st->mem_log_en, 0, DTX_HIST_SIZE * sizeof(Float32));

   st->mem_dtx_hangover_count = DTX_HANG_CONST;
   st->mem_dec_ana_elapsed_count = DTX_ELAPSED_FRAMES_THRESH;

   memset(st->mem_distance, 0, 28 * sizeof(Float32));
   memset(st->mem_distance_sum, 0, (DTX_HIST_SIZE - 1) * sizeof(Float32));

   return 0;
}

/*
 * E_DTX_init
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
Word32 E_DTX_init (E_DTX_State **st)
{
   E_DTX_State* s;

   if (st == (E_DTX_State **) NULL)
   {
      return -1;
   }

   *st = NULL;

   /* allocate memory */
   if ((s= (E_DTX_State *) malloc(sizeof(E_DTX_State))) == NULL)
   {
      return -1;
   }

   E_DTX_reset(s);
   *st = s;

   return 0;
}

/*
 * E_DTX_exit
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
void E_DTX_exit (E_DTX_State **st)
{
   if (st == NULL || *st == NULL)
   {
      return;
   }

   /* deallocate memory */
   free(*st);
   *st = NULL;

   return;
}


/*
 * E_DTX_tx_handler
 *
 * Parameters:
 *    st           I/O: State struct
 *    vad_flag       I: vad decision
 *    usedMode     I/O: mode changed or not
 *
 * Function:
 *    Adds extra speech hangover to analyze speech on the decoding side.
 *
 * Returns:
 *    void
 */
void E_DTX_tx_handler(E_DTX_State *st, Word32 vad_flag, Word16 *usedMode)
{

   /* this state machine is in synch with the GSMEFR txDtx machine */
   st->mem_dec_ana_elapsed_count++;

   if (vad_flag != 0)
   {
      st->mem_dtx_hangover_count = DTX_HANG_CONST;
   }
   else
   {  /* non-speech */
      if (st->mem_dtx_hangover_count == 0)
      {  /* out of decoder analysis hangover  */
         st->mem_dec_ana_elapsed_count = 0;
         *usedMode = MRDTX;
      }
      else
      { /* in possible analysis hangover */
         st->mem_dtx_hangover_count--;

         /* decAnaElapsedCount + dtxHangoverCount < E_DTX_ELAPSED_FRAMES_THRESH */
         if ((st->mem_dec_ana_elapsed_count + st->mem_dtx_hangover_count)
            < DTX_ELAPSED_FRAMES_THRESH)
         {
            *usedMode = MRDTX;
            /* if Word16 time since decoder update, do not add extra HO */
         }
         /*
         else
         override VAD and stay in
         speech mode *usedMode
         and add extra hangover
         */
      }
   }

   return;
}

/*
 * E_DTX_filter5
 *
 * Parameters:
 *    in0         I/O: input values / output low-pass part
 *    in1         I/O: input values / output high-pass part
 *    data        I/O: updated filter memory
 *
 * Function:
 *    Fifth-order half-band lowpass/highpass filter pair with decimation.
 *
 * Returns:
 *    void
 */
static void E_DTX_filter5(Float32 *in0, Float32 *in1,  Float32 data[])
{
   Float32 temp0, temp1, temp2;

   temp0 = *in0 - COEFF5_1 * data[0];
   temp1 = data[0] + COEFF5_1 * temp0;
   data[0] = ((temp0 > 1e-10) | (temp0 < -1e-10)) ? temp0 : 0;

   temp0 = *in1 - COEFF5_2 * data[1];
   temp2 = data[1] + COEFF5_2 * temp0;
   data[1] = ((temp0 > 1e-10) | (temp0 < -1e-10)) ? temp0 : 0;

   *in0 = (temp1 + temp2) * 0.5F;
   *in1 = (temp1 - temp2) * 0.5F;
}

/*
 * E_DTX_filter3
 *
 * Parameters:
 *    in0         I/O: input values / output low-pass part
 *    in1         I/O: input values / output high-pass part
 *    data        I/O: updated filter memory
 *
 * Function:
 *    Third-order half-band lowpass/highpass filter pair with decimation.
 *
 * Returns:
 *    void
 */
static void E_DTX_filter3(Float32 *in0, Float32 *in1, Float32 *data)
{
   Float32 temp1, temp2;

   temp1 = *in1 - COEFF3 * *data;
   temp2 = *data + COEFF3 * temp1;
   *data = ((temp1 > 1e-10) | (temp1 < -1e-10)) ? temp1 : 0;

   *in1 = (*in0 - temp2) * 0.5F;
   *in0 = (*in0 + temp2) * 0.5F;
}

/*
 * E_DTX_level_calculation
 *
 * Parameters:
 *    data          I: signal buffer
 *    sub_level   I/0: level calculated at the end of the previous frame /
 *                     level of signal calculated from the last
 *                     (count2 - count1) samples
 *    count1        I: number of samples to be counted
 *    count2        I: number of samples to be counted
 *    ind_m         I: step size for the index of the data buffer
 *    ind_a         I: starting index of the data buffer
 *    scale         I: scaling for the level calculation
 *
 * Function:
 *    Calculate signal level in a sub-band. Level is calculated
 *    by summing absolute values of the input data.
 *
 *    Because speech coder has a lookahead, signal level calculated
 *    over the lookahead (data[count1 - count2]) is stored (*sub_level)
 *    and added to the level of the next frame. Additionally, group
 *    delay and decimation of the filter bank is taken into the count
 *    for the values of the counters (count1, count2).
 *
 * Returns:
 *    signal level
 */
static Float32 E_DTX_level_calculation(Float32 data[], Float32 *sub_level,
                                       Word16 count1, Word16 count2,
                                       Word16 ind_m, Word16 ind_a,
                                       Float32 scale)
{
  Float64 l_temp1, l_temp2;
  Float32 level;
  Word32 i;

  l_temp1 = 0.0;

  for (i = count1; i < count2; i++)
  {
     l_temp1 += fabs(data[ind_m * i + ind_a]);
  }

  l_temp1 *= 2.0;
  l_temp2 = l_temp1 + *sub_level / scale;
  *sub_level = (Float32)(l_temp1 * scale);

  for (i = 0; i < count1; i++)
  {
     l_temp2 += 2.0f * fabs(data[ind_m * i + ind_a]);
  }

  level = (Float32)(l_temp2 * scale);

  return level;
}

/*
 * E_DTX_filter_bank
 *
 * Parameters:
 *    st          I/0: State struct
 *    in            I: input frame
 *    level         I: signal levels at each band
 *
 * Function:
 *    Divide input signal into bands and calculate level of
 *    the signal in each band
 *
 * Returns:
 *    void
 */
static void E_DTX_filter_bank(E_DTX_Vad_State *st, Float32 in[],
                              Float32 level[])
{
   Float32 tmp_buf[FRAME_LEN];
   Word32 i, j;

   /* shift input 1 bit down for safe scaling */
   for (i = 0; i < FRAME_LEN; i++)
   {
      tmp_buf[i] = in[i] * 0.5F;
   }

   /* run the filter bank */
   for (i = 0; i < (FRAME_LEN >> 1); i++)
   {
      j = i << 1;
      E_DTX_filter5(&tmp_buf[j], &tmp_buf[j + 1], st->mem_a_data5[0]);
   }
   for (i = 0; i < (FRAME_LEN >> 2); i++)
   {
      j = i << 2;
      E_DTX_filter5(&tmp_buf[j], &tmp_buf[j + 2], st->mem_a_data5[1]);
      E_DTX_filter5(&tmp_buf[j + 1], &tmp_buf[j + 3], st->mem_a_data5[2]);
   }
   for (i = 0; i < (FRAME_LEN >> 3); i++)
   {
      j = i << 3;
      E_DTX_filter5(&tmp_buf[j], &tmp_buf[j + 4], st->mem_a_data5[3]);
      E_DTX_filter5(&tmp_buf[j + 2], &tmp_buf[j + 6], st->mem_a_data5[4]);
      E_DTX_filter3(&tmp_buf[j + 3], &tmp_buf[j + 7], &st->mem_a_data3[0]);
   }
   for (i = 0; i < (FRAME_LEN >> 4); i++)
   {
      j = i << 4;
      E_DTX_filter3(&tmp_buf[j], &tmp_buf[j + 8], &st->mem_a_data3[1]);
      E_DTX_filter3(&tmp_buf[j + 4], &tmp_buf[j + 12], &st->mem_a_data3[2]);
      E_DTX_filter3(&tmp_buf[j + 6], &tmp_buf[j + 14], &st->mem_a_data3[3]);
   }

   for (i = 0; i < (FRAME_LEN >> 5); i++)
   {
      j = i << 5;
      E_DTX_filter3(&tmp_buf[j + 0], &tmp_buf[j + 16], &st->mem_a_data3[4]);
      E_DTX_filter3(&tmp_buf[j + 8], &tmp_buf[j + 24], &st->mem_a_data3[5]);
   }

   /* calculate levels in each frequency band */

   /* 4800 - 6400 Hz*/
   level[11] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[11],
      (FRAME_LEN >> 2) - 48, FRAME_LEN >> 2, 4, 1, 0.25F);
   /* 4000 - 4800 Hz*/
   level[10] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[10],
      (FRAME_LEN >> 3) - 24, FRAME_LEN >> 3, 8, 7, 0.5F);
   /* 3200 - 4000 Hz*/
   level[9] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[9],
      (FRAME_LEN >> 3) - 24, FRAME_LEN >> 3, 8, 3, 0.5F);
   /* 2400 - 3200 Hz*/
   level[8] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[8],
      (FRAME_LEN >> 3) - 24, FRAME_LEN >> 3, 8, 2, 0.5F);
   /* 2000 - 2400 Hz*/
   level[7] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[7],
      (FRAME_LEN >> 4) - 12, FRAME_LEN >> 4, 16, 14, 1.0F);
   /* 1600 - 2000 Hz*/
   level[6] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[6],
      (FRAME_LEN >> 4) - 12, FRAME_LEN >> 4, 16, 6, 1.0F);
   /* 1200 - 1600 Hz*/
   level[5] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[5],
      (FRAME_LEN >> 4) - 12, FRAME_LEN >> 4, 16, 4, 1.0F);
   /* 800 - 1200 Hz*/
   level[4] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[4],
      (FRAME_LEN >> 4) - 12, FRAME_LEN >> 4, 16, 12, 1.0F);
   /* 600 - 800 Hz*/
   level[3] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[3],
      (FRAME_LEN >> 5) - 6, FRAME_LEN >> 5, 32, 8, 2.0F);
   /* 400 - 600 Hz*/
   level[2] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[2],
      (FRAME_LEN >> 5) - 6, FRAME_LEN >> 5, 32, 24, 2.0F);
   /* 200 - 400 Hz*/
   level[1] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[1],
      (FRAME_LEN >> 5) - 6, FRAME_LEN >> 5, 32, 16, 2.0F);
   /* 0 - 200 Hz*/
   level[0] = E_DTX_level_calculation(tmp_buf, &st->mem_sub_level[0],
      (FRAME_LEN >> 5) - 6, FRAME_LEN >> 5, 32, 0, 2.0F);
}

/*
 * E_DTX_update_cntrl
 *
 * Parameters:
 *    st          I/0: State struct
 *    level         I: sub-band levels of the input frame
 *
 * Function:
 *    Control update of the background noise estimate.
 *
 * Returns:
 *    void
 */
static void E_DTX_update_cntrl(E_DTX_Vad_State *st, Float32 level[])
{

   Float32 stat_rat;
   Float32 num, denom;
   Float32 alpha;
   Word32 i;

   /* if fullband pitch or tone have been detected for a while, initialize stat_count */

   if ((st->mem_pitch_tone & 0x7c00) == 0x7c00)
   {
      st->mem_stat_count = STAT_COUNT;

   }
   else
   {
      /* if 8 last vad-decisions have been "0", reinitialize stat_count */

      if ((st->mem_vadreg & 0x7f80) == 0)
      {
         st->mem_stat_count = STAT_COUNT;
      }
      else
      {
         stat_rat = 0;
         for (i = 0; i < COMPLEN; i++)
         {

            if (level[i] > st->mem_ave_level[i])
            {
               num = level[i];
               denom = st->mem_ave_level[i];
            }
            else
            {
               num = st->mem_ave_level[i];
               denom = level[i];
            }
            /* Limit nimimum value of num and denom to STAT_THR_LEVEL */

            if (num  < STAT_THR_LEVEL)
            {
               num = STAT_THR_LEVEL;
            }

            if (denom < STAT_THR_LEVEL)
            {
               denom = STAT_THR_LEVEL;
            }

            stat_rat += num/denom * 64;

         }

         /* compare stat_rat with a threshold and update stat_count */

         if (stat_rat  > STAT_THR)
         {
            st->mem_stat_count = STAT_COUNT;
         }
         else
         {

            if ((st->mem_vadreg & 0x4000) != 0)
            {

               if (st->mem_stat_count != 0)
               {
                  st->mem_stat_count--;
               }
            }
         }
      }
   }

   /* Update average amplitude estimate for stationarity estimation */
   alpha = ALPHA4;

   if (st->mem_stat_count == STAT_COUNT)
   {
      alpha = 1.0;
   }
   else if ((st->mem_vadreg & 0x4000) == 0)
   {

      alpha = ALPHA5;
   }

   for (i = 0; i < COMPLEN; i++)
   {
      st->mem_ave_level[i] += alpha * (level[i] - st->mem_ave_level[i]);
   }

}

/*
 * E_DTX_hangover_addition
 *
 * Parameters:
 *    st              I/0: State struct
 *    low_power         I: flag power of the input frame
 *    hang_len          I: hangover length
 *    burst_len         I: minimum burst length for hangover addition
 *
 * Function:
 *    Add hangover after speech bursts.
 *
 * Returns:
 *    VAD_flag indicating final VAD decision
 */
static Word16 E_DTX_hangover_addition(E_DTX_Vad_State *st, Word16 low_power,
                                      Word16 hang_len, Word16 burst_len)
{
   /*
    * if the input power (pow_sum) is lower than a threshold, clear
    * counters and set VAD_flag to "0"  "fast exit"
    */

   if (low_power != 0)
   {
      st->mem_burst_count = 0;
      st->mem_hang_count = 0;
      return 0;
   }

   /* update the counters (hang_count, burst_count) */

   if ((st->mem_vadreg & 0x4000) != 0)
   {
      st->mem_burst_count++;

      if (st->mem_burst_count >= burst_len)
      {
         st->mem_hang_count = hang_len;
      }
      return 1;
   }
   else
   {
      st->mem_burst_count = 0;

      if (st->mem_hang_count > 0)
      {
         st->mem_hang_count--;
         return 1;
      }
   }
   return 0;
}

/*
 * E_DTX_noise_estimate_update
 *
 * Parameters:
 *    st           I/0: State struct
 *    level          I: sub-band levels of the input frame
 *
 * Function:
 *    Update of background noise estimate
 *
 * Returns:
 *    void
 */
static void E_DTX_noise_estimate_update(E_DTX_Vad_State *st, Float32 level[])
{

   Float32 alpha_up, alpha_down, bckr_add, temp;
   Word32 i;

   /* Control update of bckr_est[] */
   E_DTX_update_cntrl(st, level);

   /* Choose update speed */
   bckr_add = 2.0;


   if ((0x7800 & st->mem_vadreg) == 0)
   {
      alpha_up = ALPHA_UP1;
      alpha_down = ALPHA_DOWN1;
   }
   else
   {

      if (st->mem_stat_count == 0)
      {
         alpha_up = ALPHA_UP2;
         alpha_down = ALPHA_DOWN2;
      }
      else
      {
         alpha_up = 0.0;
         alpha_down = ALPHA3;
         bckr_add = 0.0;
      }
   }

   /* Update noise estimate (bckr_est) */
   for (i = 0; i < COMPLEN; i++)
   {
      temp = st->mem_level[i] - st->mem_bckr_est[i];

      if (temp < 0.0)
      { /* update downwards*/
         st->mem_bckr_est[i] += -2 + (alpha_down * temp);

         /* limit minimum value of the noise estimate to NOISE_MIN */

         if (st->mem_bckr_est[i] < NOISE_MIN)
         {
            st->mem_bckr_est[i] = NOISE_MIN;
         }
      }
      else
      { /* update upwards */
         st->mem_bckr_est[i] += bckr_add + (alpha_up * temp);

         /* limit maximum value of the noise estimate to NOISE_MAX */

         if (st->mem_bckr_est[i] > NOISE_MAX)
         {
            st->mem_bckr_est[i] = NOISE_MAX;
         }
      }
   }

   /* Update signal levels of the previous frame (old_level) */
   memcpy(st->mem_level, level, COMPLEN * sizeof(Float32));
}

/*
 * E_DTX_decision
 *
 * Parameters:
 *    st           I/0: State struct
 *    level          I: sub-band levels of the input frame
 *    pow_sum        I: power of the input frame
 *
 * Function:
 *    Calculates VAD_flag
 *
 * Returns:
 *    VAD_flag
 */
static Word16 E_DTX_decision(E_DTX_Vad_State *st, Float32 level[COMPLEN], Float64 pow_sum)
{
   Float64 snr_sum;
   Float32 vad_thr, temp, noise_level;
   Float32 ilog2_speech_level, ilog2_noise_level;
   Float32 temp2;
   Word32 i;
   Word16 low_power_flag;
   Word16 hang_len,burst_len;

   /*
    * Calculate squared sum of the input levels (level)
    * divided by the background noise components (bckr_est).
    */
   snr_sum = 0.0;

   for (i = 0; i < COMPLEN; i++)
   {
      temp = level[i] / st->mem_bckr_est[i];
      snr_sum += temp * temp;
   }

   /* Calculate average level of estimated background noise */
   temp = 0.0;

   for (i = 1; i < COMPLEN; i++) /* ignore lowest band */
   {
      temp += st->mem_bckr_est[i];
   }

   noise_level = (Float32)(temp * 0.0625);
   /*
    * if SNR is lower than a threshold (MIN_SPEECH_SNR),
    * and increase speech_level
    */
   temp = noise_level * MIN_SPEECH_SNR * 8;

   if (st->mem_speech_level <= temp)
   {
      st->mem_speech_level = temp;

      /* avoid log10 error */
      temp -= 1E-8F;
   }
   
   ilog2_noise_level = (Float32)(-1024.0F * log10(noise_level / 2147483648.0F) / log10(2.0F));
  
   /*
    * If SNR is very poor, speech_level is probably corrupted by noise level. This
    * is correctred by subtracting -MIN_SPEECH_SNR*noise_level from speech level
    */
   ilog2_speech_level = (Float32)(-1024.0F * log10((st->mem_speech_level - temp) / 2147483648.0F) / log10(2.0F));

   temp = NO_SLOPE * (ilog2_noise_level- NO_P1) + THR_HIGH;

   temp2 = SP_CH_MIN + SP_SLOPE * (ilog2_speech_level - SP_P1);

   if (temp2 < SP_CH_MIN)
   {
      temp2 = SP_CH_MIN;
   }

   if (temp2 > SP_CH_MAX)
   {
      temp2 = SP_CH_MAX;
   }

   vad_thr = temp + temp2;

   if (vad_thr < THR_MIN)
   {
      vad_thr = THR_MIN;
   }

   /* Shift VAD decision register */
   st->mem_vadreg = (Word16)(st->mem_vadreg >> 1);

   /* Make intermediate VAD decision */

   if (snr_sum > (vad_thr * (Float32)COMPLEN / 128.0F))
   {
      st->mem_vadreg = (Word16)(st->mem_vadreg | 0x4000);
   }
   /* primary vad decision made */

   /* check if the input power (pow_sum) is lower than a threshold" */

   if (pow_sum < VAD_POW_LOW)
   {
      low_power_flag = 1;
   }
   else
   {
      low_power_flag = 0;
   }

   /* Update speech subband background noise estimates */
   E_DTX_noise_estimate_update(st, level);

   hang_len = (Word16)((HANG_SLOPE * (vad_thr - HANG_P1) - 0.5) + HANG_HIGH);

   if (hang_len < HANG_LOW)
   {
      hang_len = HANG_LOW;
   }

   burst_len = (Word16)((BURST_SLOPE * (vad_thr - BURST_P1) - 0.5) + BURST_HIGH);

   return(E_DTX_hangover_addition(st, low_power_flag, hang_len,burst_len));
}

/*
 * E_DTX_dpeech_estimate
 *
 * Parameters:
 *    st           I/0: State struct
 *    in_level       I: level of the input frame
 *
 * Function:
 *    Estimate speech level
 *
 *    Maximum signal level is searched and stored to the variable sp_max.
 *    The speech frames must locate within SP_EST_COUNT number of frames to be counted.
 *    Thus, noisy frames having occasional VAD = "1" decisions will not
 *    affect to the estimated speech_level.
 *
 * Returns:
 *    void
 */
static void E_DTX_speech_estimate(E_DTX_Vad_State *st, Float32 in_level)
{
   Float32 alpha, tmp;

   /* if the required activity count cannot be achieved, reset counters */
   if (SP_ACTIVITY_COUNT  > (SP_EST_COUNT - st->mem_sp_est_cnt + st->mem_sp_max_cnt))
   {
      st->mem_sp_est_cnt = 0;
      st->mem_sp_max = 0.0;
      st->mem_sp_max_cnt = 0;
   }

   st->mem_sp_est_cnt++;

   if (((st->mem_vadreg & 0x4000) || (in_level > st->mem_speech_level))
      && (in_level > MIN_SPEECH_LEVEL1))
   {
      if (in_level > st->mem_sp_max)
      {
         st->mem_sp_max = in_level;
      }

      st->mem_sp_max_cnt++;

      if (st->mem_sp_max_cnt >= SP_ACTIVITY_COUNT)
      {

         tmp = st->mem_sp_max / 2.0F; /* scale to get "average" speech level*/

         if (tmp > st->mem_speech_level)
         {
            alpha = ALPHA_SP_UP;
         }
         else
         {
            alpha = ALPHA_SP_DOWN;
         }

         if (tmp > MIN_SPEECH_LEVEL2)
         {
            st->mem_speech_level += alpha * (tmp - st->mem_speech_level);
         }

         st->mem_sp_max = 0.0;
         st->mem_sp_max_cnt = 0;
         st->mem_sp_est_cnt = 0;
      }
   }
}

/*
 * E_DTX_vad_reset
 *
 * Parameters:
 *    state        I/0: State struct
 *
 * Function:
 *    Initialises state memory
 *
 * Returns:
 *    non-zero with error, zero for ok
 */
Word32 E_DTX_vad_reset (E_DTX_Vad_State *state)
{
   Word32 i;

   if (state == (E_DTX_Vad_State *) NULL)
   {
      return -1;
   }

   /* Initialize pitch detection variables */
   state->mem_pitch_tone = 0;
   state->mem_vadreg = 0;
   state->mem_hang_count = 0;
   state->mem_burst_count = 0;
   state->mem_hang_count = 0;

   /* initialize memory used by the filter bank */
   memset(state->mem_a_data5, 0, F_5TH_CNT * 2 * sizeof(Float32));
   memset(state->mem_a_data3, 0, F_3TH_CNT * sizeof(Float32));

   /* initialize the rest of the memory */
   for (i = 0; i < COMPLEN; i++)
   {
      state->mem_bckr_est[i] = NOISE_INIT;
      state->mem_level[i] = NOISE_INIT;
      state->mem_ave_level[i] = NOISE_INIT;
      state->mem_sub_level[i] = 0;
   }

   state->mem_sp_est_cnt = 0;
   state->mem_sp_max = 0;
   state->mem_sp_max_cnt = 0;
   state->mem_speech_level = SPEECH_LEVEL_INIT;
   state->mem_pow_sum = 0;
   state->mem_stat_count = 0;

   return 0;
}

/*
 * E_DTX_vad_init
 *
 * Parameters:
 *    state        I/0: State struct
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    non-zero with error, zero for ok
 */
Word32 E_DTX_vad_init (E_DTX_Vad_State **state)
{
   E_DTX_Vad_State* s;

   if (state == (E_DTX_Vad_State **) NULL)
   {
      return -1;
   }
   *state = NULL;

   /* allocate memory */
   if ((s = (E_DTX_Vad_State *) malloc(sizeof(E_DTX_Vad_State))) == NULL)
   {
      return -1;
   }

   E_DTX_vad_reset(s);

   *state = s;

   return 0;
}

/*
 * E_DTX_vad_exit
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
void E_DTX_vad_exit (E_DTX_Vad_State **state)
{
   if (state == NULL || *state == NULL)
   {
      return;
   }

   /* deallocate memory */
   free(*state);
   *state = NULL;

   return;
}

/*
 * E_DTX_pitch_tone_detection
 *
 * Parameters:
 *    state        I/0: State struct
 *    p_gain         I: pitch gain
 *
 * Function:
 *    Set tone flag if pitch gain is high. This is used to detect
 *    signaling tones and other signals with high pitch gain.
 *
 * Returns:
 *    void
 */
void E_DTX_pitch_tone_detection (E_DTX_Vad_State *st, Float32 p_gain)
{
   /* update tone flag and pitch flag */
   st->mem_pitch_tone = (Word16)(st->mem_pitch_tone >> 1);

   /* if (pitch_gain > TONE_THR) set tone flag */
   if (p_gain > TONE_THR)
   {
      st->mem_pitch_tone = (Word16)(st->mem_pitch_tone | 0x4000);
   }
}

/*
 * E_DTX_vad
 *
 * Parameters:
 *    st           I/0: State struct
 *    in_buf         I: samples of the input frame
 *
 * Function:
 *    Main program for Voice Activity Detection (VAD)
 *
 * Returns:
 *    VAD Decision, 1 = speech, 0 = noise
 */
Word16 E_DTX_vad(E_DTX_Vad_State *st, Float32 in_buf[])
{
   Float64 L_temp, pow_sum;
   Float32 level[COMPLEN];
   Float32 temp;
   Word32 i;
   Word16 VAD_flag;

   /* Calculate power of the input frame. */
   L_temp = 0.0;

   for (i = 0; i < FRAME_LEN; i++)
   {
      L_temp += in_buf[i] * in_buf[i];
   }

   L_temp *= 2.0;

   /* pow_sum = power of current frame and previous frame */
   pow_sum = L_temp + st->mem_pow_sum;

   /* save power of current frame for next call */
   st->mem_pow_sum = L_temp;

   /* If input power is very low, clear tone flag */
   if (pow_sum < POW_PITCH_TONE_THR)
   {
      st->mem_pitch_tone = (Word16)(st->mem_pitch_tone & 0x1fff);
   }

   /*  Run the filter bank and calculate signal levels at each band */
   E_DTX_filter_bank(st, in_buf, level);

   /* compute VAD decision */
   VAD_flag = E_DTX_decision(st, level, pow_sum);

   /* Calculate input level */
   L_temp = 0.0;
   for (i = 1; i < COMPLEN; i++) /* ignore lowest band */
   {
      L_temp += level[i];
   }

   temp = (Float32)(L_temp / 16.0F);

   E_DTX_speech_estimate(st, temp); /* Estimate speech level */

   return(VAD_flag);
}
