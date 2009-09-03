/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <memory.h>
#include "typedef.h"
#include "dec_util.h"

#define L_SUBFR         64       /* Subframe size */
#define L_LTPHIST       5
#define ONE_PER_3       10923
#define ONE_PER_LTPHIST 6554
#define UP_SAMP         4
#define L_INTERPOL2     16

extern const Word16 D_ROM_inter4_2[];
extern const Word16 D_ROM_pdown_unusable[];
extern const Word16 D_ROM_pdown_usable[];
extern const Word16 D_ROM_cdown_unusable[];
extern const Word16 D_ROM_cdown_usable[];
extern const Word16 D_ROM_qua_gain6b[];
extern const Word16 D_ROM_qua_gain7b[];

/*
 * D_GAIN_init
 *
 * Parameters:
 *    mem         O: static memory
 *
 * Function:
 *    Initialisation of 2nd order quantiser energy predictor.
 *
 * Returns:
 *    void
 */
void D_GAIN_init(Word16 *mem)
{

   /* 4nd order quantizer energy predictor (init to -14.0 in Q10) */
   mem[0] = -14336;   /* past_qua_en[0] */
   mem[1] = -14336;   /* past_qua_en[1] */
   mem[2] = -14336;   /* past_qua_en[2] */
   mem[3] = -14336;   /* past_qua_en[3] */
   /*
    * mem[4] = 0;       past_gain_pit
    * mem[5] = 0;       past_gain_code
    * mem[6] = 0;       prev_gc
    * mem[7 - 11] = 0;  pbuf[i]
    * mem[12 - 16] = 0; gbuf[i]
    * mem[17 - 21] = 0; pbuf2[i]
    */
   memset(&mem[4], 0, 18 * sizeof(Word16));

   mem[22] = 21845;   /* seed */
   return;
}


/*
 * D_GAIN_median
 *
 * Parameters:
 *    buf            I: previous gains
 *
 * Function:
 *    Median of gains
 *
 * Returns:
 *    median of 5 previous gains
 */
static Word16 D_GAIN_median(Word16 x[])
{
   Word16 x1, x2, x3, x4, x5;
   Word16 tmp;
   x1 = x[ - 2];
   x2 = x[ - 1];
   x3 = x[0];
   x4 = x[1];
   x5 = x[2];

   if(x2 < x1)
   {
      tmp = x1;
      x1 = x2;
      x2 = tmp;
   }

   if(x3 < x1)
   {
      tmp = x1;
      x1 = x3;
      x3 = tmp;
   }

   if(x4 < x1)
   {
      tmp = x1;
      x1 = x4;
      x4 = tmp;
   }

   if(x5 < x1)
   {
      x5 = x1;
   }

   if(x3 < x2)
   {
      tmp = x2;
      x2 = x3;
      x3 = tmp;
   }

   if(x4 < x2)
   {
      tmp = x2;
      x2 = x4;
      x4 = tmp;
   }

   if(x5 < x2)
   {
      x5 = x2;
   }

   if(x4 < x3)
   {
      x3 = x4;
   }

   if(x5 < x3)
   {
      x3 = x5;
   }

   return(x3);
}


/*
 * D_GAIN_decode
 *
 * Parameters:
 *    index             I: Quantization index
 *    nbits             I: number of bits (6 or 7)
 *    code              I: Innovative code vector
 *    L_subfr           I: Subframe size
 *    gain_pit          O: (Q14) Quantized pitch gain
 *    gain_code         O: (Q16) Quantized codebook gain
 *    bfi               I: Bad frame indicator
 *    prev_bfi          I: Previous BF indicator
 *    state             I: State of BFH
 *    unusable_frame    I: UF indicator
 *    vad_hist          I: number of non-speech frames
 *    mem             I/O: static memory (4 words)
 *
 *
 * Function:
 *    Decoding of pitch and codebook gains
 *
 * Returns:
 *    void
 */
void D_GAIN_decode(Word16 index, Word16 nbits, Word16 code[], Word16 *gain_pit,
                   Word32 *gain_cod, Word16 bfi, Word16 prev_bfi,
                   Word16 state, Word16 unusable_frame, Word16 vad_hist,
                   Word16 *mem)
{

   Word32 gcode0, qua_ener, L_tmp;
   const Word16 * p;
   Word16 *past_gain_pit, *past_gain_code, *past_qua_en, *prev_gc;
   Word16 *gbuf, *pbuf, *pbuf2;
   Word16 i, tmp, exp, frac, exp_gcode0, gcode_inov;
   Word16 g_code;

   past_qua_en = mem;
   past_gain_pit = mem + 4;
   past_gain_code = mem + 5;
   prev_gc = mem + 6;
   pbuf = mem + 7;
   gbuf = mem + 12;
   pbuf2 = mem + 17;

   /*
    * Find energy of code and compute:
    *
    *    L_tmp = 1.0 / sqrt(energy of code/ L_subfr)
    */
   L_tmp = D_UTIL_dot_product12(code, code, L_SUBFR, &exp);
   exp = (Word16)(exp - (18 + 6));   /* exp: -18 (code in Q9), -6 (/L_subfr) */
   D_UTIL_normalised_inverse_sqrt(&L_tmp, &exp);

   if(exp > 3)
   {
      L_tmp <<= (exp - 3);
   }
   else
   {
      L_tmp >>= (3 - exp);
   }

   gcode_inov = (Word16)(L_tmp >>16);   /* g_code_inov in Q12 */

   /*
    * Case of erasure.
    */
   if(bfi != 0)
   {
      tmp = D_GAIN_median(&pbuf[2]);
      *past_gain_pit = tmp;

      if(*past_gain_pit > 15565)
      {
         *past_gain_pit = 15565;   /* 0.95 in Q14 */
      }

      if(unusable_frame != 0)
      {
         *gain_pit =
            (Word16)((D_ROM_pdown_unusable[state] * *past_gain_pit) >> 15);
      }
      else
      {
         *gain_pit =
            (Word16)((D_ROM_pdown_usable[state] * *past_gain_pit) >> 15);
      }

      tmp = D_GAIN_median(&gbuf[2]);

      if(vad_hist > 2)
      {
         *past_gain_code = tmp;
      }
      else
      {
         if(unusable_frame != 0)
         {
            *past_gain_code =
               (Word16)((D_ROM_cdown_unusable[state] * tmp) >> 15);
         }
         else
         {
            *past_gain_code =
               (Word16)((D_ROM_cdown_usable[state] * tmp) >> 15);
         }
      }

      /* update table of past quantized energies */
      L_tmp = past_qua_en[0] + past_qua_en[1]+ past_qua_en[2] + past_qua_en[3];
      qua_ener = L_tmp >> 2;
      qua_ener = qua_ener - 3072;   /* -3 in Q10 */

      if(qua_ener < - 14336)
      {
         qua_ener = -14336;   /* -14 in Q10 */
      }

      past_qua_en[3] = past_qua_en[2];
      past_qua_en[2] = past_qua_en[1];
      past_qua_en[1] = past_qua_en[0];
      past_qua_en[0] = (Word16)qua_ener;

      for(i = 1; i < 5; i++)
      {
         gbuf[i - 1] = gbuf[i];
      }
      gbuf[4] = *past_gain_code;

      for(i = 1; i < 5; i++)
      {
         pbuf[i - 1] = pbuf[i];
      }
      pbuf[4] = *past_gain_pit;

      /* adjust gain according to energy of code */
      /* past_gain_code(Q3) * gcode_inov(Q12) => Q16 */
      *gain_cod = (*past_gain_code * gcode_inov) << 1;

      return;
   }

   /*
    * Compute gcode0.
    *  = Sum(i=0,1) pred[i]*past_qua_en[i] + mean_ener - ener_code
    */

   /* MEAN_ENER in Q24 = 0x1e000000 */
   /* MA prediction coeff = {0.5, 0.4, 0.3, 0.2} in Q13 */
   L_tmp = 0xF000000 + (4096 * past_qua_en[0]); /* Q13*Q10 -> Q24 */
   L_tmp = L_tmp + (3277 * past_qua_en[1]);     /* Q13*Q10 -> Q24 */
   L_tmp = L_tmp + (2458 * past_qua_en[2]);     /* Q13*Q10 -> Q24 */
   L_tmp = L_tmp + (1638 * past_qua_en[3]);     /* Q13*Q10 -> Q24 */
   gcode0 = L_tmp >> 15;               /* From Q24 to Q8 */

   /*
    * gcode0 = pow(10.0, gcode0/20)
    *        = pow(2, 3.321928*gcode0/20)
    *        = pow(2, 0.166096*gcode0)
    */
   L_tmp = (gcode0 * 5443) >> 7;
   /* *0.166096 in Q15 -> Q24, From Q24 to Q16 */
   D_UTIL_l_extract(L_tmp, &exp_gcode0, &frac);
   /* Extract exponant of gcode0  */
   gcode0 = D_UTIL_pow2(14, frac); /* Put 14 as exponant so that */

   /*
    * output of Pow2() will be:
    * 16384 < Pow2() <= 32767
    */
   exp_gcode0 = (Word16)(exp_gcode0 - 14);

   /* Read the quantized gains */
   if(nbits == 6)
   {
      p = &D_ROM_qua_gain6b[(index << 1)];
   }
   else
   {
      p = &D_ROM_qua_gain7b[(index << 1)];
   }

   *gain_pit = *p++; /* selected pitch gain in Q14 */
   g_code = *p++;    /* selected code gain in Q11  */
   L_tmp = g_code * gcode0;
   exp_gcode0 += 5;

   if(exp_gcode0 >= 0)
   {
      *gain_cod = L_tmp << exp_gcode0;    /* gain of code in Q16 */
   }
   else
   {
      *gain_cod = L_tmp >> -exp_gcode0;   /* gain of code in Q16 */
   }

   if(prev_bfi == 1)
   {
      L_tmp = (*prev_gc * 5120) << 1;  /* prev_gc(Q3) * 1.25(Q12) = Q16 */

      /* if((*gain_cod > ((*prev_gc) * 1.25)) && (*gain_cod > 100.0)) */
      if((*gain_cod > L_tmp) & (*gain_cod > 6553600))
      {
         *gain_cod = L_tmp;
      }
   }

   /* keep past gain code in Q3 for frame erasure (can saturate) */
   L_tmp = (*gain_cod + 0x1000) >> 13;

   if(L_tmp < 32768)
   {
      *past_gain_code = (Word16)L_tmp;
   }
   else
   {
      *past_gain_code = 32767;
   }

   *past_gain_pit = *gain_pit;
   *prev_gc = *past_gain_code;

   for(i = 1; i < 5; i++)
   {
      gbuf[i - 1] = gbuf[i];
   }
   gbuf[4] = *past_gain_code;

   for(i = 1; i < 5; i++)
   {
      pbuf[i - 1] = pbuf[i];
   }
   pbuf[4] = *past_gain_pit;

   for(i = 1; i < 5; i++)
   {
      pbuf2[i - 1] = pbuf2[i];
   }
   pbuf2[4] = *past_gain_pit;

   /* adjust gain according to energy of code */
   D_UTIL_l_extract(*gain_cod, &exp, &frac);
   L_tmp = D_UTIL_mpy_32_16(exp, frac, gcode_inov);

   if(L_tmp < 0xFFFFFFF)
   {
      *gain_cod = (L_tmp << 3);   /* gcode_inov in Q12 */
   }
   else
   {
      *gain_cod = 0x7FFFFFFF;
   }

   /*
    * qua_ener = 20*log10(g_code)
    *          = 6.0206*log2(g_code)
    *          = 6.0206*(log2(g_codeQ11) - 11)
    */
   L_tmp = (Word32)(g_code);
   D_UTIL_log2(L_tmp, &exp, &frac);
   exp = (Word16)(exp - 11);
   L_tmp = D_UTIL_mpy_32_16(exp, frac, 24660);   /* x 6.0206 in Q12 */
   qua_ener = L_tmp >>3;   /* result in Q10 */

   /* update table of past quantized energies */
   past_qua_en[3] = past_qua_en[2];
   past_qua_en[2] = past_qua_en[1];
   past_qua_en[1] = past_qua_en[0];
   past_qua_en[0] = (Word16)qua_ener;

   return;
}


/*
 * D_GAIN_adaptive_control
 *
 * Parameters:
 *    sig_in            I: postfilter input signal
 *    sig_out         I/O: postfilter output signal
 *    l_trm             I: subframe size
 *
 * Function:
 *    Adaptive gain control is used to compensate for
 *    the gain difference between the non-emphasized excitation and
 *    emphasized excitation.
 *
 * Returns:
 *    void
 */
void D_GAIN_adaptive_control(Word16 *sig_in, Word16 *sig_out, Word16 l_trm)
{
   Word32 s, temp, i, exp;
   Word32 gain_in, gain_out, g0;

   /* calculate gain_out with exponent */
   temp = sig_out[0] >> 2;
   s = temp * temp;

   for(i = 1; i < l_trm; i++)
   {
      temp = sig_out[i] >> 2;
      s += temp * temp;
   }

   s <<= 1;

   if(s == 0)
   {
      return;
   }
   exp = (D_UTIL_norm_l(s) - 1);

   if(exp >= 0)
   {
      gain_out = ((s << exp) + 0x8000) >> 16;
   }
   else
   {
      gain_out = ((s >> -exp) + 0x8000) >> 16;
   }

   /* calculate gain_in with exponent */
   temp = sig_in[0] >> 2;
   s = temp * temp;

   for(i = 1; i < l_trm; i++)
   {
      temp = sig_in[i] >> 2;
      s += temp * temp;
   }

   s <<= 1;

   if(s == 0)
   {
      g0 = 0;
   }
   else
   {
      i = D_UTIL_norm_l(s);
      s = ((s << i) + 0x8000) >> 16;

      if((s < 32768) & (s > 0))
      {
         gain_in = s;
      }
      else
      {
         gain_in = 32767;
      }
      exp = exp - i;

      /*
       * g0 = sqrt(gain_in/gain_out)
       */
      s = (gain_out << 15) / gain_in;
      s = s << (7 - exp);   /* s = gain_out / gain_in */
      s = D_UTIL_inverse_sqrt(s);
      g0 = ((s << 9) + 0x8000) >> 16;
   }

   /* sig_out(n) = gain(n) sig_out(n) */
   for(i = 0; i < l_trm; i++)
   {
      s = (sig_out[i] * g0) >> 13;
      sig_out[i] = D_UTIL_saturate(s);
   }

   return;
}


/*
 * D_GAIN_insert_lag
 *
 * Parameters:
 *    array        I/O: pitch lag history
 *    n              I: history size
 *    x              I: lag value
 *
 * Function:
 *    Insert lag into correct location
 *
 * Returns:
 *    void
 */
static void D_GAIN_insert_lag(Word16 array[], Word32 n, Word16 x)
{
   Word32 i;

   for(i = n - 1; i >= 0; i--)
   {
      if(x < array[i])
      {
         array[i + 1] = array[i];
      }
      else
      {
         break;
      }
   }

   array[i + 1] = x;
}


/*
 * D_GAIN_sort_lag
 *
 * Parameters:
 *    array        I/O: pitch lag history
 *    n              I: history size
 *
 * Function:
 *    Sorting of the lag history
 *
 * Returns:
 *    void
 */
static void D_GAIN_sort_lag(Word16 array[], Word16 n)
{
   Word32 i;

   for(i = 0; i < n; i++)
   {
      D_GAIN_insert_lag(array, i, array[i]);
   }
}


/*
 * D_GAIN_lag_concealment_init
 *
 * Parameters:
 *    lag_hist       O: pitch lag history
 *
 * Function:
 *    Initialise lag history to 64
 *
 * Returns:
 *    void
 */
void D_GAIN_lag_concealment_init(Word16 lag_hist[])
{
   Word32 i;

   for(i = 0; i < L_LTPHIST; i++)
   {
      lag_hist[i] = 64;
   }
}


/*
 * D_GAIN_lag_concealment
 *
 * Parameters:
 *    gain_hist         I: gain history
 *    lag_hist          I: pitch lag history
 *    T0                O: current lag
 *    old_T0            I: previous lag
 *    seed            I/O: seed for random
 *    unusable_frame    I: lost frame
 *
 * Function:
 *    Concealment of LTP lags during bad frames
 *
 * Returns:
 *    void
 */
void D_GAIN_lag_concealment(Word16 gain_hist[], Word16 lag_hist[],
                            Word32 *T0, Word16 *old_T0, Word16 *seed,
                            Word16 unusable_frame)
{
   Word32 i, lagDif, tmp, tmp2, D2, meanLag = 0;
   Word16 lag_hist2[L_LTPHIST] = {0};
   Word16 maxLag, minLag, lastLag;
   Word16 minGain, lastGain, secLastGain;
   Word16 D;

   /*
    * Is lag index such that it can be aplied directly
    * or does it has to be subtituted
    */
   lastGain = gain_hist[4];
   secLastGain = gain_hist[3];
   lastLag = lag_hist[0];

   /* SMALLEST history lag */
   minLag = lag_hist[0];

   for(i = 1; i < L_LTPHIST; i++)
   {
      if(lag_hist[i] < minLag)
      {
         minLag = lag_hist[i];
      }
   }

   /* BIGGEST history lag */
   maxLag = lag_hist[0];

   for(i = 1; i < L_LTPHIST; i++)
   {
      if(lag_hist[i] > maxLag)
      {
         maxLag = lag_hist[i];
      }
   }

   /* SMALLEST history gain */
   minGain = gain_hist[0];

   for(i = 1; i < L_LTPHIST; i++)
   {
      if(gain_hist[i] < minGain)
      {
         minGain = gain_hist[i];
      }
   }

   /* Difference between MAX and MIN lag */
   lagDif = maxLag - minLag;

   if(unusable_frame != 0)
   {
      /*
       * LTP-lag for RX_SPEECH_LOST
       * Recognition of the LTP-history
       */
      if((minGain > 8192) & (lagDif < 10))
      {
         *T0 = *old_T0;
      }
      else if((lastGain > 8192) && (secLastGain > 8192))
      {
         *T0 = lag_hist[0];
      }
      else
      {
         /*
          * SORT
          * The sorting of the lag history
          */
         for(i = 0; i < L_LTPHIST; i++)
         {
            lag_hist2[i] = lag_hist[i];
         }
         D_GAIN_sort_lag(lag_hist2, 5);

         /*
          * Lag is weighted towards bigger lags
          * and random variation is added
          */
         lagDif = (lag_hist2[4] - lag_hist2[2]);

         if(lagDif > 40)
         {
            lagDif = 40;
         }

         D = D_UTIL_random(seed);   /* D={-1, ...,1} */

         /* D2={-lagDif/2..lagDif/2} */
         tmp = lagDif >> 1;
         D2 = (tmp * D) >> 15;
         tmp = (lag_hist2[2] + lag_hist2[3]) + lag_hist2[4];
         *T0 = ((tmp * ONE_PER_3) >> 15) + D2;
      }

      /* New lag is not allowed to be bigger or smaller than last lag values */
      if(*T0 > maxLag)
      {
         *T0 = maxLag;
      }

      if(*T0 < minLag)
      {
         *T0 = minLag;
      }
   }
   else
   {
      /*
       * LTP-lag for RX_BAD_FRAME
       * MEAN lag
       */
      meanLag = 0;

      for(i = 0; i < L_LTPHIST; i++)
      {
         meanLag = meanLag + lag_hist[i];
      }

      meanLag = (meanLag * ONE_PER_LTPHIST) >> 15;
      tmp = *T0 - maxLag;
      tmp2 = *T0 - lastLag;

      if((lagDif < 10) & (*T0 > (minLag - 5)) & (tmp < 5))
      {
         *T0 = *T0;
      }
      else if((lastGain > 8192) & (secLastGain > 8192) & ((tmp2 > - 10)
         & (tmp2 < 10)))
      {
         *T0 = *T0;
      }
      else if((minGain < 6554) & (lastGain == minGain) & ((*T0 > minLag)
         & (*T0 < maxLag)))
      {
         *T0 = *T0;
      }
      else if((lagDif < 70) & (*T0 > minLag) & (*T0 < maxLag))
      {
         *T0 = *T0;
      }
      else if((*T0 > meanLag) & (*T0 < maxLag))
      {
         *T0 = *T0;
      }
      else
      {
         if((minGain > 8192) & (lagDif < 10))
         {
            *T0 = lag_hist[0];
         }
         else if((lastGain > 8192) & (secLastGain > 8192))
         {
            *T0 = lag_hist[0];
         }
         else
         {
            /*
             * SORT
             * The sorting of the lag history
             */
            for(i = 0; i < L_LTPHIST; i++)
            {
               lag_hist2[i] = lag_hist[i];
            }

            D_GAIN_sort_lag(lag_hist2, 5);

            /*
             * Lag is weighted towards bigger lags
             * and random variation is added
             */
            lagDif = lag_hist2[4] - lag_hist2[2];

            if(lagDif > 40)
            {
               lagDif = 40;
            }

            D = D_UTIL_random(seed);   /* D={-1,.., 1} */

            /* D2={-lagDif/2..lagDif/2} */
            tmp = lagDif >> 1;
            D2 = (tmp * D) >> 15;
            tmp = (lag_hist2[2] + lag_hist2[3]) + lag_hist2[4];
            *T0 = ((tmp * ONE_PER_3) >> 15) + D2;
         }

         /*
          * New lag is not allowed to be bigger or
          * smaller than last lag values
          */
         if(*T0 > maxLag)
         {
            *T0 = maxLag;
         }

         if(*T0 < minLag)
         {
            *T0 = minLag;
         }
      }
   }
}


/*
 * D_GAIN_adaptive_codebook_excitation
 *
 * Parameters:
 *    exc          I/O: excitation buffer
 *    T0             I: integer pitch lag
 *    frac           I: fraction of lag
 *
 * Function:
 *    Compute the result of Word32 term prediction with fractional
 *    interpolation of resolution 1/4.
 *
 * Returns:
 *    interpolated signal (adaptive codebook excitation)
 */
void D_GAIN_adaptive_codebook_excitation(Word16 exc[], Word32 T0, Word32 frac)
{
   Word32 i, j, k, sum;
   Word16 *x;

   x = &exc[ - T0];
   frac = -(frac);

   if(frac < 0)
   {
      frac = (frac + UP_SAMP);
      x--;
   }
   x = x - L_INTERPOL2 + 1;

   for(j = 0; j < L_SUBFR + 1; j++)
   {
      sum = 0L;

      for(i = 0, k = ((UP_SAMP - 1) - frac); i < 2 * L_INTERPOL2; i++,
         k += UP_SAMP)
      {
         sum += x[i] * D_ROM_inter4_2[k];
      }
      sum = (sum + 0x2000) >> 14;

      exc[j] = D_UTIL_saturate(sum);

      x++;
   }
   return;
}


/*
 * D_GAIN_pitch_sharpening
 *
 * Parameters:
 *    x            I/O: impulse response (or algebraic code)
 *    pit_lag        I: pitch lag
 *    sharp          I: (Q15) pitch sharpening factor
 *
 * Function:
 *    Performs Pitch sharpening routine for one subframe.
 *
 * Returns:
 *    void
 */
void D_GAIN_pitch_sharpening(Word16 *x, Word32 pit_lag, Word16 sharp)
{
   Word32 i;
   Word32 tmp;

   for(i = pit_lag; i < L_SUBFR; i++)
   {
      tmp = x[i] << 15;
      tmp += x[i - pit_lag] * sharp;
      x[i] = (Word16)((tmp + 0x4000) >> 15);
   }
   return;
}


/*
 * D_GAIN_find_voice_factor
 *
 * Parameters:
 *    exc            I: pitch excitation
 *    Q_exc          I: exc format
 *    gain_pit       I: (Q14) gain of pitch
 *    code           I: (Q9) fixed codebook excitation
 *    gain_code      I: (Q0) gain of code
 *    L_subfr        I: subframe length
 *
 * Function:
 *    Find the voicing factor.
 *
 * Returns:
 *    (Q15) 1=voice to -1=unvoiced
 */
Word16 D_GAIN_find_voice_factor(Word16 exc[], Word16 Q_exc,
                                Word16 gain_pit, Word16 code[],
                                Word16 gain_code, Word16 L_subfr)
{

   Word32 tmp, ener1, ener2, i;
   Word16 exp, exp1, exp2;

   ener1 = (D_UTIL_dot_product12(exc, exc, L_subfr, &exp1)) >> 16;
   exp1 = (Word16)(exp1 - (Q_exc + Q_exc));
   tmp = (gain_pit * gain_pit) << 1;
   exp = D_UTIL_norm_l(tmp);
   tmp = (tmp << exp) >> 16;
   ener1 = (ener1 * tmp) >> 15;
   exp1 = (Word16)((exp1 - exp) - 10);   /* 10 -> gain_pit Q14 to Q9 */
   ener2 = D_UTIL_dot_product12(code, code, L_subfr, &exp2) >> 16;
   exp = D_UTIL_norm_s(gain_code);
   tmp = gain_code << exp;
   tmp = (tmp * tmp) >> 15;
   ener2 = (ener2 * tmp) >> 15;
   exp2 = (Word16)(exp2 - (exp << 1));
   i = exp1 - exp2;

   if(i >= 0)
   {
      ener1 = ener1 >> 1;
      ener2 = ener2 >> (i + 1);
   }
   else if(i > (-16))
   {
      ener1 = ener1 >> (1 - i);
      ener2 = ener2 >> 1;
   }
   else
   {
      ener1 = 0;
      ener2 = ener2 >> 1;
   }

   tmp = ener1 - ener2;
   ener1 = (ener1 + ener2) + 1;
   tmp = (tmp << 15) / ener1;

   return((Word16)tmp);
}
