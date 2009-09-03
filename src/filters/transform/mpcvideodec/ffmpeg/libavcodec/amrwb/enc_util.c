/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <math.h>
#include <memory.h>
#include "typedef.h"
#include "enc_main.h"
#include "enc_lpc.h"

#ifdef WIN32
#pragma warning( disable : 4310)
#endif

#define MAX_16 (Word16)0x7FFF
#define MIN_16 (Word16)0x8000
#define MAX_31 (Word32)0x3FFFFFFF
#define MIN_31 (Word32)0xC0000000
#define L_FRAME16k   320     /* Frame size at 16kHz         */
#define L_SUBFR16k   80      /* Subframe size at 16kHz      */
#define L_SUBFR      64      /* Subframe size               */
#define M16k         20      /* Order of LP filter          */
#define L_WINDOW     384     /* window size in LP analysis  */
#define PREEMPH_FAC  0.68F   /* preemphasis factor          */

extern const Word16 E_ROM_pow2[];
extern const Word16 E_ROM_log2[];
extern const Word16 E_ROM_isqrt[];
extern const Float32 E_ROM_fir_6k_7k[];
extern const Float32 E_ROM_hp_gain[];
extern const Float32 E_ROM_fir_ipol[];
extern const Float32 E_ROM_hamming_cos[];

/*
 * E_UTIL_random
 *
 * Parameters:
 *    seed        I/O: seed for random number
 *
 * Function:
 *    Signed 16 bits random generator.
 *
 * Returns:
 *    random number
 */
Word16 E_UTIL_random(Word16 *seed)
{
  /*static Word16 seed = 21845;*/

  *seed = (Word16) (*seed * 31821L + 13849L);

  return(*seed);
}

/*
 * E_UTIL_saturate
 *
 * Parameters:
 *    inp        I: 32-bit number
 *
 * Function:
 *    Saturation to 16-bit number
 *
 * Returns:
 *    16-bit number
 */
Word16 E_UTIL_saturate(Word32 inp)
{
   Word16 out;

   if ((inp < MAX_16) & (inp > MIN_16))
   {
      out = (Word16)inp;
   }
   else
   {
      if (inp > 0)
      {
         out = MAX_16;
      }
      else
      {
         out = MIN_16;
      }
   }

   return(out);
}

/*
 * E_UTIL_saturate_31
 *
 * Parameters:
 *    inp        I: 32-bit number
 *
 * Function:
 *    Saturation to 31-bit number
 *
 * Returns:
 *    31(32)-bit number
 */
Word32 E_UTIL_saturate_31(Word32 inp)
{
   Word32 out;

   if ((inp < MAX_31) & (inp > MIN_31))
   {
      out = inp;
   }
   else
   {
      if (inp > 0)
      {
         out = MAX_31;
      }
      else
      {
         out = MIN_31;
      }
   }

   return(out);
}

/*
 * E_UTIL_norm_s
 *
 * Parameters:
 *    L_var1      I: 32 bit Word32 signed integer (Word32) whose value
 *                   falls in the range 0xffff 8000 <= var1 <= 0x0000 7fff.
 *
 * Function:
 *    Produces the number of left shift needed to normalize the 16 bit
 *    variable var1 for positive values on the interval with minimum
 *    of 16384 and maximum of 32767, and for negative values on
 *    the interval with minimum of -32768 and maximum of -16384.
 *
 * Returns:
 *    16 bit Word16 signed integer (Word16) whose value falls in the range
 *    0x0000 0000 <= var_out <= 0x0000 000f.
 */
Word16 E_UTIL_norm_s (Word16 var1)
{
   Word16 var_out;

   if (var1 == 0)
   {
      var_out = 0;
   }
   else
   {
      if (var1 == -1)
      {
         var_out = 15;
      }
      else
      {
         if (var1 < 0)
         {
            var1 = (Word16)~var1;
         }
         for (var_out = 0; var1 < 0x4000; var_out++)
         {
            var1 <<= 1;
         }
      }
   }

   return (var_out);
}

/*
 * E_UTIL_norm_l
 *
 * Parameters:
 *    L_var1      I: 32 bit Word32 signed integer (Word32) whose value
 *                   falls in the range 0x8000 0000 <= var1 <= 0x7fff ffff.
 *
 * Function:
 *    Produces the number of left shifts needed to normalize the 32 bit
 *    variable L_var1 for positive values on the interval with minimum of
 *    1073741824 and maximum of 2147483647, and for negative values on
 *    the interval with minimum of -2147483648 and maximum of -1073741824;
 *    in order to normalize the result, the following operation must be done:
 *    norm_L_var1 = L_shl(L_var1,norm_l(L_var1)).
 *
 * Returns:
 *    16 bit Word16 signed integer (Word16) whose value falls in the range
 *    0x0000 0000 <= var_out <= 0x0000 001f.
 */
Word16 E_UTIL_norm_l (Word32 L_var1)
{
   Word16 var_out;

   if (L_var1 == 0)
   {
      var_out = 0;
   }
   else
   {
      if (L_var1 == (Word32) 0xffffffffL)
      {
         var_out = 31;
      }
      else
      {
         if (L_var1 < 0)
         {
            L_var1 = ~L_var1;
         }
         for (var_out = 0; L_var1 < (Word32) 0x40000000L; var_out++)
         {
            L_var1 <<= 1;
         }
      }
   }

   return (var_out);
}

/*
 * E_UTIL_l_extract
 *
 * Parameters:
 *    L_32        I: 32 bit integer.
 *    hi          O: b16 to b31 of L_32
 *    lo          O: (L_32 - hi<<16)>>1
 *
 * Function:
 *    Extract from a 32 bit integer two 16 bit DPF.
 *
 * Returns:
 *    void
 */
void E_UTIL_l_extract(Word32 L_32, Word16 *hi, Word16 *lo)
{
   *hi = (Word16)(L_32 >> 16);
   *lo = (Word16)((L_32 >> 1) - ((*hi * 16384) << 1));
   return;
}

/*
 * E_UTIL_mpy_32_16
 *
 * Parameters:
 *    hi          I: hi part of 32 bit number
 *    lo          I: lo part of 32 bit number
 *    n           I: 16 bit number
 *
 * Function:
 *    Multiply a 16 bit integer by a 32 bit (DPF). The result is divided
 *    by 2^15.
 *
 *    L_32 = (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1
 *
 * Returns:
 *    32 bit result
 */
Word32 E_UTIL_mpy_32_16 (Word16 hi, Word16 lo, Word16 n)
{
   Word32 L_32;

   L_32 = (hi * n) << 1;
   L_32 = L_32 + (((lo * n) >> 15) << 1);

   return (L_32);
}

/*
 * E_UTIL_pow2
 *
 * Parameters:
 *    exponant    I: (Q0) Integer part.      (range: 0 <= val <= 30)
 *    fraction    I: (Q15) Fractionnal part.  (range: 0.0 <= val < 1.0)
 *
 * Function:
 *    L_x = pow(2.0, exponant.fraction)         (exponant = interger part)
 *        = pow(2.0, 0.fraction) << exponant
 *
 *    Algorithm:
 *
 *    The function Pow2(L_x) is approximated by a table and linear
 *    interpolation.
 *
 *    1 - i = bit10 - b15 of fraction,   0 <= i <= 31
 *    2 - a = bit0 - b9   of fraction
 *    3 - L_x = table[i] << 16 - (table[i] - table[i + 1]) * a * 2
 *    4 - L_x = L_x >> (30-exponant)     (with rounding)
 *
 * Returns:
 *    range 0 <= val <= 0x7fffffff
 */
Word32 E_UTIL_pow2(Word16 exponant, Word16 fraction)
{
   Word32 L_x, tmp, i, exp;
   Word16 a;

   L_x = fraction * 32;          /* L_x = fraction<<6             */
   i = L_x >> 15;                /* Extract b10-b16 of fraction   */
   a = (Word16)(L_x);            /* Extract b0-b9   of fraction   */
   a = (Word16)(a & (Word16)0x7fff);
   L_x = E_ROM_pow2[i] << 16;    /* table[i] << 16                */
   tmp = E_ROM_pow2[i] - E_ROM_pow2[i + 1];  /* table[i] - table[i+1] */
   L_x = L_x - ((tmp * a) << 1); /* L_x -= tmp*a*2                */
   exp = 30 - exponant;
   L_x = (L_x + (1 << (exp - 1))) >> exp;

   return(L_x);
}

/*
 * E_UTIL_normalised_log2
 *
 * Parameters:
 *    L_x      I: input value (normalized)
 *    exp      I: norm_l (L_x)
 *    exponent O: Integer part of Log2.   (range: 0<=val<=30)
 *    fraction O: Fractional part of Log2. (range: 0<=val<1)
 *
 * Function:
 *    Computes log2(L_x, exp),  where   L_x is positive and
 *    normalized, and exp is the normalisation exponent
 *    If L_x is negative or zero, the result is 0.
 *
 *    The function Log2(L_x) is approximated by a table and linear
 *    interpolation. The following steps are used to compute Log2(L_x)
 *
 *    1. exponent = 30 - norm_exponent
 *    2. i = bit25 - b31 of L_x;  32 <= i <= 63  (because of normalization).
 *    3. a = bit10 - b24
 *    4. i -= 32
 *    5. fraction = table[i] << 16 - (table[i] - table[i + 1]) * a * 2
 *
 *
 * Returns:
 *    void
 */
static void E_UTIL_normalised_log2(Word32 L_x, Word16 exp, Word16 *exponent,
                          Word16 *fraction)
{
   Word32 i, a, tmp;
   Word32 L_y;

   if (L_x <= 0)
   {
      *exponent = 0;
      *fraction = 0;
      return;
   }

   *exponent = (Word16)(30 - exp);

   L_x = L_x >> 10;
   i = L_x >> 15;         /* Extract b25-b31               */
   a = L_x;               /* Extract b10-b24 of fraction   */
   a = a & 0x00007fff;
   i = i - 32;
   L_y = E_ROM_log2[i] << 16;               /* table[i] << 16        */
   tmp = E_ROM_log2[i] - E_ROM_log2[i + 1]; /* table[i] - table[i+1] */
   L_y = L_y - ((tmp * a) << 1);            /* L_y -= tmp*a*2        */
   *fraction = (Word16)(L_y >> 16);

   return;
}


/*
 * E_UTIL_log2
 *
 * Parameters:
 *    L_x      I: input value
 *    exponent O: Integer part of Log2.   (range: 0<=val<=30)
 *    fraction O: Fractional part of Log2. (range: 0<=val<1)
 *
 * Function:
 *    Computes log2(L_x),  where   L_x is positive.
 *    If L_x is negative or zero, the result is 0.
 *
 * Returns:
 *    void
 */
void E_UTIL_log2_32 (Word32 L_x, Word16 *exponent, Word16 *fraction)
{
   Word16 exp;

   exp = E_UTIL_norm_l(L_x);
   E_UTIL_normalised_log2((L_x << exp), exp, exponent, fraction);
}

/*
 * E_UTIL_interpol
 *
 * Parameters:
 *    x           I: input vector
 *    fir         I: filter coefficient
 *    frac        I: fraction (0..resol)
 *    resol       I: resolution
 *    nb_coef     I: number of coefficients
 *
 * Function:
 *    Fractional interpolation of signal at position (frac/up_samp)
 *
 * Returns:
 *    result of interpolation
 */
static Float32 E_UTIL_interpol(Float32 *x, Word32 frac, Word32 up_samp,
                               Word32 nb_coef)
{
   Word32 i;
   Float32 s;
   Float32 *x1, *x2;
   const Float32 *c1, *c2;

   x1 = &x[0];
   x2 = &x[1];
   c1 = &E_ROM_fir_ipol[frac];
   c2 = &E_ROM_fir_ipol[up_samp - frac];

   s = 0.0;

   for(i = 0; i < nb_coef; i++)
   {
      s += x1[-i] * c1[up_samp * i] + x2[i] * c2[up_samp * i];
   }

   return s;
}

/*
 * E_UTIL_hp50_12k8
 *
 * Parameters:
 *    signal       I/O: signal
 *    lg             I: lenght of signal
 *    mem          I/O: filter memory [6]
 *
 * Function:
 *    2nd order high pass filter with cut off frequency at 50 Hz.
 *
 *    Algorithm:
 *
 *    y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2]
 *                     + a[1]*y[i-1] + a[2]*y[i-2];
 *
 *    b[3] = {0.989501953f, -1.979003906f, 0.989501953f};
 *    a[3] = {1.000000000F,  1.978881836f,-0.966308594f};
 *
 *
 * Returns:
 *    void
 */
void E_UTIL_hp50_12k8(Float32 signal[], Word32 lg, Float32 mem[])
{
   Word32 i;
   Float32 x0, x1, x2, y0, y1, y2;

   y1 = mem[0];
   y2 = mem[1];
   x0 = mem[2];
   x1 = mem[3];

   for(i = 0; i < lg; i++)
   {
      x2 = x1;
      x1 = x0;
      x0 = signal[i];

      y0 = y1 * 1.978881836F + y2 * -0.979125977F + x0 * 0.989501953F +
         x1 * -1.979003906F + x2 * 0.989501953F;

      signal[i] = y0;
      y2 = y1;
      y1 = y0;
   }

   mem[0] = ((y1 > 1e-10) | (y1 < -1e-10)) ? y1 : 0;
   mem[1] = ((y2 > 1e-10) | (y2 < -1e-10)) ? y2 : 0;
   mem[2] = ((x0 > 1e-10) | (x0 < -1e-10)) ? x0 : 0;
   mem[3] = ((x1 > 1e-10) | (x1 < -1e-10)) ? x1 : 0;

   return;
}

/*
 * E_UTIL_hp400_12k8
 *
 * Parameters:
 *    signal       I/O: signal
 *    lg             I: lenght of signal
 *    mem          I/O: filter memory [4]
 *
 * Function:
 *    2nd order high pass filter with cut off frequency at 400 Hz.
 *
 *    Algorithm:
 *
 *    y[i] = b[0]*x[i] + b[1]*x[i-1] + b[2]*x[i-2]
 *                     + a[1]*y[i-1] + a[2]*y[i-2];
 *
 *    b[3] = {0.893554687, -1.787109375,  0.893554687};
 *    a[3] = {1.000000000,  1.787109375, -0.864257812};
 *
 *
 * Returns:
 *    void
 */
static void E_UTIL_hp400_12k8(Float32 signal[], Word32 lg, Float32 mem[])
{
   Word32 i;

   Float32 x0, x1, x2;
   Float32 y0, y1, y2;

   y1 = mem[0];
   y2 = mem[1];
   x0 = mem[2];
   x1 = mem[3];

   for(i = 0; i < lg; i++)
   {
      x2 = x1;
      x1 = x0;
      x0 = signal[i];
      y0 = y1 * 1.787109375F + y2 * -0.864257812F + x0 * 0.893554687F + x1 * -
         1.787109375F + x2 * 0.893554687F;
      signal[i] = y0;
      y2 = y1;
      y1 = y0;
   }

   mem[0] = y1;
   mem[1] = y2;
   mem[2] = x0;
   mem[3] = x1;

   return;
}

/*
 * E_UTIL_bp_6k_7k
 *
 * Parameters:
 *    signal       I/O: signal
 *    lg             I: lenght of signal
 *    mem          I/O: filter memory [4]
 *
 * Function:
 *    15th order band pass 6kHz to 7kHz FIR filter.
 *
 * Returns:
 *    void
 */
static void E_UTIL_bp_6k_7k(Float32 signal[], Word32 lg, Float32 mem[])
{
   Float32 x[L_SUBFR16k + 30];
   Float32 s0, s1, s2, s3;
   Float32 *px;
   Word32 i, j;

   memcpy(x, mem, 30 * sizeof(Float32));
   memcpy(x + 30, signal, lg * sizeof(Float32));

   px = x;

   for(i = 0; i < lg; i++)
   {
      s0 = 0;
      s1 = px[0] * E_ROM_fir_6k_7k[0];
      s2 = px[1] * E_ROM_fir_6k_7k[1];
      s3 = px[2] * E_ROM_fir_6k_7k[2];

      for(j = 3; j < 31; j += 4)
      {
         s0 += px[j] * E_ROM_fir_6k_7k[j];
         s1 += px[j + 1] * E_ROM_fir_6k_7k[j + 1];
         s2 += px[j + 2] * E_ROM_fir_6k_7k[j + 2];
         s3 += px[j + 3] * E_ROM_fir_6k_7k[j + 3];
      }

      px++;

      signal[i] = (Float32)((s0 + s1 + s2 + s3) * 0.25F);   /* gain of coef = 4.0 */
   }

   memcpy(mem, x + lg, 30 * sizeof(Float32));

   return;
}

/*
 * E_UTIL_preemph
 *
 * Parameters:
 *    x            I/O: signal
 *    mu             I: preemphasis factor
 *    lg             I: vector size
 *    mem          I/O: memory (x[-1])
 *
 * Function:
 *    Filtering through 1 - mu z^-1
 *
 *
 * Returns:
 *    void
 */
void E_UTIL_preemph(Word16 x[], Word16 mu, Word32 lg, Word16 *mem)
{
   Word32 i, L_tmp;
   Word16 temp;

   temp = x[lg - 1];

   for (i = lg - 1; i > 0; i--)
   {
      L_tmp = x[i] << 15;
      L_tmp -= x[i - 1] * mu;
      x[i] = (Word16)((L_tmp + 0x4000) >> 15);
   }

   L_tmp = (x[0] << 15);
   L_tmp -= *mem * mu;
   x[0] = (Word16)((L_tmp + 0x4000) >> 15);

   *mem = temp;

   return;
}

void E_UTIL_f_preemph(Float32 *signal, Float32 mu, Word32 L, Float32 *mem)
{
   Word32 i;
   Float32 temp;

   temp = signal[L - 1];

   for (i = L - 1; i > 0; i--)
   {
      signal[i] = signal[i] - mu * signal[i - 1];
   }

   signal[0] -= mu * (*mem);
   *mem = temp;

   return;
}

/*
 * E_UTIL_deemph
 *
 * Parameters:
 *    signal       I/O: signal
 *    mu             I: deemphasis factor
 *    L              I: vector size
 *    mem          I/O: memory (signal[-1])
 *
 * Function:
 *    Filtering through 1/(1-mu z^-1)
 *    Signal is divided by 2.
 *
 * Returns:
 *    void
 */
void E_UTIL_deemph(Float32 *signal, Float32 mu, Word32 L, Float32 *mem)
{
   Word32 i;

   signal[0] = signal[0] + mu * (*mem);

   for (i = 1; i < L; i++)
   {
      signal[i] = signal[i] + mu * signal[i - 1];
   }

   *mem = signal[L - 1];

   if ((*mem < 1e-10) & (*mem > -1e-10))
   {
      *mem = 0;
   }

   return;
}

/*
 * E_UTIL_synthesis
 *
 * Parameters:
 *    a              I: LP filter coefficients
 *    m              I: order of LP filter
 *    x              I: input signal
 *    y              O: output signal
 *    lg             I: size of filtering
 *    mem          I/O: initial filter states
 *    update_m       I: update memory flag
 *
 * Function:
 *    Perform the synthesis filtering 1/A(z).
 *    Memory size is always M.
 *
 * Returns:
 *    void
 */
void E_UTIL_synthesis(Float32 a[], Float32 x[], Float32 y[], Word32 l,
                      Float32 mem[], Word32 update_m)
{

   Float32 buf[L_FRAME16k + M16k];     /* temporary synthesis buffer */
   Float32 s;
   Float32 *yy;
   Word32 i, j;

   /* copy initial filter states into synthesis buffer */
   memcpy(buf, mem, M * sizeof(Float32));
   yy = &buf[M];

   for (i = 0; i < l; i++)
   {
      s = x[i];

      for (j = 1; j <= M; j += 4)
      {
         s -= a[j] * yy[i - j];
         s -= a[j + 1] * yy[i - (j + 1)];
         s -= a[j + 2] * yy[i - (j + 2)];
         s -= a[j + 3] * yy[i - (j + 3)];
      }

      yy[i] = s;
      y[i] = s;
   }

   /* Update memory if required */
   if (update_m)
   {
      memcpy(mem, &yy[l - M], M * sizeof(Float32));
   }

   return;
}

/*
 * E_UTIL_down_samp
 *
 * Parameters:
 *    res            I: signal to down sample
 *    res_d          O: down sampled signal
 *    L_frame_d      I: length of output
 *
 * Function:
 *    Down sample to 4/5
 *
 * Returns:
 *    void
 */
static void E_UTIL_down_samp(Float32 *res, Float32 *res_d, Word32 L_frame_d)
{
   Word32 i, j, frac;
   Float32 pos, fac;

   fac = 0.8F;
   pos = 0;

   for(i = 0; i < L_frame_d; i++)
   {
      j = (Word32)pos;    /* j = (Word32)( (Float32)i * inc); */
      frac = (Word32)(((pos - (Float32)j) * 4) + 0.5);
      res_d[i] = fac * E_UTIL_interpol(&res[j], frac, 4, 15);
      pos += 1.25F;
   }

   return;
}

/*
 * E_UTIL_decim_12k8
 *
 * Parameters:
 *    sig16k         I: signal to decimate
 *    lg             I: length of input
 *    sig12k8        O: decimated signal
 *    mem          I/O: memory (2*15)
 *
 * Function:
 *    Decimation of 16kHz signal to 12.8kHz.
 *
 * Returns:
 *    void
 */
void E_UTIL_decim_12k8(Float32 sig16k[], Word32 lg, Float32 sig12k8[],
                       Float32 mem[])
{
   Float32 signal[(2 * 15) + L_FRAME16k];

   memcpy(signal, mem, 2 * 15 * sizeof(Float32));
   memcpy(&signal[2 * 15], sig16k, lg * sizeof(Float32));

   E_UTIL_down_samp(signal + 15, sig12k8, lg * 4 / 5);

   memcpy(mem, &signal[lg], 2 * 15 * sizeof(Float32));

   return;
}

/*
 * E_UTIL_residu
 *
 * Parameters:
 *    a           I: LP filter coefficients (Q12)
 *    x           I: input signal (usually speech)
 *    y           O: output signal (usually residual)
 *    l           I: size of filtering
 *
 * Function:
 *    Compute the LP residual by filtering the input speech through A(z).
 *    Order of LP filter = M.
 *
 * Returns:
 *    void
 */
void E_UTIL_residu(Float32 *a, Float32 *x, Float32 *y, Word32 l)
{
   Float32 s;
   Word32 i;

   for (i = 0; i < l; i++)
   {
      s = x[i];
      s += a[1] * x[i - 1];
      s += a[2] * x[i - 2];
      s += a[3] * x[i - 3];
      s += a[4] * x[i - 4];
      s += a[5] * x[i - 5];
      s += a[6] * x[i - 6];
      s += a[7] * x[i - 7];
      s += a[8] * x[i - 8];
      s += a[9] * x[i - 9];
      s += a[10] * x[i - 10];
      s += a[11] * x[i - 11];
      s += a[12] * x[i - 12];
      s += a[13] * x[i - 13];
      s += a[14] * x[i - 14];
      s += a[15] * x[i - 15];
      s += a[16] * x[i - 16];
      y[i] = s;
   }

   return;
}

/*
 * E_UTIL_convolve
 *
 * Parameters:
 *    x           I: input vector
 *    h           I: impulse response (or second input vector) (Q15)
 *    y           O: output vetor (result of convolution)
 *
 * Function:
 *    Perform the convolution between two vectors x[] and h[] and
 *    write the result in the vector y[]. All vectors are of length L.
 *    Only the first L samples of the convolution are considered.
 *    Vector size = L_SUBFR
 *
 * Returns:
 *    void
 */
void E_UTIL_convolve(Word16 x[], Word16 q, Float32 h[], Float32 y[])
{
   Float32 fx[L_SUBFR];
   Float32 temp, scale;
   Word32 i, n;

   scale = (Float32)pow(2, -q);

   for (i = 0; i < L_SUBFR; i++)
   {
      fx[i] = (Float32)(scale * x[i]);
   }

   for (n = 0; n < L_SUBFR; n += 2)
   {
      temp = 0.0;
      for (i = 0; i <= n; i++)
      {
         temp += (Float32)(fx[i] * h[n - i]);
      }
      y[n] = temp;

      temp = 0.0;
      for (i = 0; i <= (n + 1); i += 2)
      {
         temp += (Float32)(fx[i] * h[(n + 1) - i]);
         temp += (Float32)(fx[i + 1] * h[n - i]);
      }
      y[n + 1] = temp;

   }

   return;
}

void E_UTIL_f_convolve(Float32 x[], Float32 h[], Float32 y[])
{
   Float32 temp;
   Word32 i, n;

   for (n = 0; n < L_SUBFR; n += 2)
   {
      temp = 0.0;

      for (i = 0; i <= n; i++)
      {
         temp += x[i] * h[n - i];
      }

      y[n] = temp;

      temp = 0.0;

      for (i = 0; i <= (n + 1); i += 2)
      {
         temp += x[i] * h[(n + 1) - i];
         temp += x[i + 1] * h[n - i];
      }

      y[n + 1] = temp;
   }

   return;
}

/*
 * E_UTIL_signal_up_scale
 *
 * Parameters:
 *    x         I/O: signal to scale
 *    exp         I: exponent: x = round(x << exp)
 *
 * Function:
 *    Scale signal up to get maximum of dynamic.
 *
 * Returns:
 *    void
 */
void E_UTIL_signal_up_scale(Word16 x[], Word16 exp)
{
   Word32 i;
   Word32  tmp;

   for (i = 0; i < (PIT_MAX + L_INTERPOL + L_SUBFR); i++)
   {
      tmp = x[i] << exp;
      x[i] = E_UTIL_saturate(tmp);
   }

   return;
}

/*
 * E_UTIL_signal_down_scale
 *
 * Parameters:
 *    x         I/O: signal to scale
 *    lg          I: size of x[]
 *    exp         I: exponent: x = round(x << exp)
 *
 * Function:
 *    Scale signal up to get maximum of dynamic.
 *
 * Returns:
 *    32 bit result
 */
void E_UTIL_signal_down_scale(Word16 x[], Word32 lg, Word16 exp)
{
   Word32 i, tmp;

   for (i = 0; i < lg; i++)
   {
      tmp = x[i] << 16;
      tmp = tmp >> exp;
      x[i] = (Word16)((tmp + 0x8000) >> 16);
   }

   return;
}


/*
 * E_UTIL_dot_product12
 *
 * Parameters:
 *    x        I: 12bit x vector
 *    y        I: 12bit y vector
 *    lg       I: vector length (x*4)
 *    exp      O: exponent of result (0..+30)
 *
 * Function:
 *    Compute scalar product of <x[],y[]> using accumulator.
 *    The result is normalized (in Q31) with exponent (0..30).
 *
 * Returns:
 *    Q31 normalised result (1 < val <= -1)
 */
Word32 E_UTIL_dot_product12(Word16 x[], Word16 y[], Word32 lg, Word32 *exp)
{
   Word32 i, sft, L_sum, L_sum1, L_sum2, L_sum3, L_sum4;

   L_sum1 = 0L;
   L_sum2 = 0L;
   L_sum3 = 0L;
   L_sum4 = 0L;

   for (i = 0; i < lg; i += 4)
   {
      L_sum1 += x[i] * y[i];
      L_sum2 += x[i + 1] * y[i + 1];
      L_sum3 += x[i + 2] * y[i + 2];
      L_sum4 += x[i + 3] * y[i + 3];
   }

   L_sum1 = E_UTIL_saturate_31(L_sum1);
   L_sum2 = E_UTIL_saturate_31(L_sum2);
   L_sum3 = E_UTIL_saturate_31(L_sum3);
   L_sum4 = E_UTIL_saturate_31(L_sum4);
   L_sum1 += L_sum3;
   L_sum2 += L_sum4;
   L_sum1 = E_UTIL_saturate_31(L_sum1);
   L_sum2 = E_UTIL_saturate_31(L_sum2);
   L_sum = L_sum1 + L_sum2;
   L_sum = (E_UTIL_saturate_31(L_sum) << 1) + 1;

   /* Normalize acc in Q31 */

   sft = E_UTIL_norm_l(L_sum);
   L_sum = (L_sum << sft);

   *exp = (30 - sft);  /* exponent = 0..30 */

   return (L_sum);
}

/*
 * E_UTIL_normalised_inverse_sqrt
 *
 * Parameters:
 *    frac     I/O: (Q31) normalized value (1.0 < frac <= 0.5)
 *    exp      I/O: exponent (value = frac x 2^exponent)
 *
 * Function:
 *    Compute 1/sqrt(value).
 *    If value is negative or zero, result is 1 (frac=7fffffff, exp=0).
 *
 *    The function 1/sqrt(value) is approximated by a table and linear
 *    interpolation.
 *    1. If exponant is odd then shift fraction right once.
 *    2. exponant = -((exponant - 1) >> 1)
 *    3. i = bit25 - b30 of fraction, 16 <= i <= 63 ->because of normalization.
 *    4. a = bit10 - b24
 *    5. i -= 16
 *    6. fraction = table[i]<<16 - (table[i] - table[i+1]) * a * 2
 *
 * Returns:
 *    void
 */
void E_UTIL_normalised_inverse_sqrt(Word32 *frac, Word16 *exp)
{
   Word32 i, a, tmp;

   if (*frac <= (Word32) 0)
   {
      *exp = 0;
      *frac = 0x7fffffffL;
      return;
   }

   if ((Word16) (*exp & 1) == 1)  /* If exponant odd -> shift right */
   {
      *frac = (*frac >> 1);
   }

   *exp = (Word16)(-((*exp - 1) >> 1));

   *frac = (*frac >> 9);
   i = *frac >> 16;                    /* Extract b25-b31 */
   *frac = (*frac >> 1);
   a = (Word16)*frac;                  /* Extract b10-b24 */
   a = a & 0x00007fff;

   i = i - 16;

   *frac = E_ROM_isqrt[i] << 16;                /* table[i] << 16         */
   tmp = E_ROM_isqrt[i] - E_ROM_isqrt[i + 1];   /* table[i] - table[i+1]) */

   *frac = *frac - ((tmp * a) << 1);            /* frac -=  tmp*a*2       */

   return;
}

/*
 * E_UTIL_enc_synthesis
 *
 * Parameters:
 *    Aq             I: quantized Az
 *    exc            I: excitation at 12kHz
 *    synth16k       O: 16kHz synthesis signal
 *    st           I/O: State structure
 *
 * Function:
 *    Synthesis of signal at 16kHz with HF extension.
 *
 * Returns:
 *    The quantised gain index when using the highest mode, otherwise zero
 */
Word32 E_UTIL_enc_synthesis(Float32 Aq[], Float32 exc[], Float32 synth16k[],
                            Coder_State *st)
{
   Float32 synth[L_SUBFR];
   Float32 HF[L_SUBFR16k];   /* High Frequency vector      */
   Float32 Ap[M + 1];
   Float32 HF_SP[L_SUBFR16k];   /* High Frequency vector (from original signal) */
   Float32 HP_est_gain, HP_calc_gain, HP_corr_gain, fac, tmp, ener, dist_min;
   Float32 dist, gain2;
   Word32 i, hp_gain_ind = 0;

   /*
    * speech synthesis
    * ----------------
    * - Find synthesis speech corresponding to exc2[].
    * - Perform fixed deemphasis and hp 50hz filtering.
    * - Oversampling from 12.8kHz to 16kHz.
    */
   E_UTIL_synthesis(Aq, exc, synth, L_SUBFR, st->mem_syn2, 1);
   E_UTIL_deemph(synth, PREEMPH_FAC, L_SUBFR, &(st->mem_deemph));
   E_UTIL_hp50_12k8(synth, L_SUBFR, st->mem_sig_out);

   /* Original speech signal as reference for high band gain quantisation */
   memcpy(HF_SP, synth16k, L_SUBFR16k * sizeof(Float32));

   /*
    * HF noise synthesis
    * ------------------
    * - Generate HF noise between 6 and 7 kHz.
    * - Set energy of noise according to synthesis tilt.
    *     tilt > 0.8 ==> - 14 dB (voiced)
    *     tilt   0.5 ==> - 6 dB  (voiced or noise)
    *     tilt < 0.0 ==>   0 dB  (noise)
    */
   /* generate white noise vector */
   for(i = 0; i < L_SUBFR16k; i++)
   {
      HF[i] = (Float32)E_UTIL_random(&(st->mem_seed));
   }

   /* set energy of white noise to energy of excitation */
   ener = 0.01F;
   tmp = 0.01F;

   for(i = 0; i < L_SUBFR; i++)
   {
      ener += exc[i] * exc[i];
   }

   for(i = 0; i < L_SUBFR16k; i++)
   {
      tmp += HF[i] * HF[i];
   }

   tmp = (Float32)(sqrt(ener / tmp));

   for(i = 0; i < L_SUBFR16k; i++)
   {
      HF[i] *= tmp;
   }

   /* find tilt of synthesis speech (tilt: 1=voiced, -1=unvoiced) */
   E_UTIL_hp400_12k8(synth, L_SUBFR, st->mem_hp400);
   ener = 0.001f;
   tmp = 0.001f;

   for(i = 1; i < L_SUBFR; i++)
   {
      ener += synth[i] * synth[i];
      tmp += synth[i] * synth[i - 1];
   }

   fac = tmp / ener;

   /* modify energy of white noise according to synthesis tilt */
   HP_est_gain = 1.0F - fac;
   gain2 = (1.0F - fac) * 1.25F;

   if(st->mem_vad_hist)
   {
      HP_est_gain = gain2;
   }

   if(HP_est_gain < 0.1)
   {
      HP_est_gain = 0.1f;
   }

   if(HP_est_gain > 1.0)
   {
      HP_est_gain = 1.0f;
   }

   /* synthesis of noise: 4.8kHz..5.6kHz --> 6kHz..7kHz */
   E_LPC_a_weight(Aq, Ap, 0.6f, M);
   E_UTIL_synthesis(Ap, HF, HF, L_SUBFR16k, st->mem_syn_hf, 1);

   /* noise High Pass filtering (0.94ms of delay) */
   E_UTIL_bp_6k_7k(HF, L_SUBFR16k, st->mem_hf);

   /* noise High Pass filtering (0.94ms of delay) */
   E_UTIL_bp_6k_7k(HF_SP, L_SUBFR16k, st->mem_hf2);

   ener = 0.001F;
   tmp = 0.001F;

   for(i = 0; i < L_SUBFR16k; i++)
   {
      ener += HF_SP[i] * HF_SP[i];
      tmp += HF[i] * HF[i];
   }

   HP_calc_gain = (Float32)sqrt(ener /tmp);
   st->mem_gain_alpha *= st->dtx_encSt->mem_dtx_hangover_count / 7;

   if(st->dtx_encSt->mem_dtx_hangover_count > 6)
   {
      st->mem_gain_alpha = 1.0F;
   }

   HP_corr_gain = (HP_calc_gain * st->mem_gain_alpha) +
      ((1.0F - st->mem_gain_alpha) * HP_est_gain);

   /* Quantise the correction gain */
   dist_min = 100000.0F;

   for(i = 0; i < 16; i++)
   {
      dist = (HP_corr_gain - E_ROM_hp_gain[i]) * (HP_corr_gain - E_ROM_hp_gain[i]);

      if(dist_min > dist)
      {
         dist_min = dist;
         hp_gain_ind = i;
      }
   }

   HP_corr_gain = (Float32)E_ROM_hp_gain[hp_gain_ind];

   /* return the quantised gain index when using the highest mode, otherwise zero */
   return(hp_gain_ind);
}

/*
 * E_UTIL_autocorr
 *
 * Parameters:
 *    x           I: input signal
 *    r_h         O: autocorrelations
 *
 * Function:
 *    Compute the autocorrelations of windowed speech signal.
 *    order of LP filter is M. Window size is L_WINDOW.
 *    Analysis window is "window".
 *
 * Returns:
 *    void
 */
void E_UTIL_autocorr(Float32 *x, Float32 *r)
{
   Float32 t[L_WINDOW + M];
   Word32 i, j;

   for (i = 0; i < L_WINDOW; i += 4)
   {
      t[i] = x[i] * E_ROM_hamming_cos[i];
      t[i + 1] = x[i + 1] * E_ROM_hamming_cos[i + 1];
      t[i + 2] = x[i + 2] * E_ROM_hamming_cos[i + 2];
      t[i + 3] = x[i + 3] * E_ROM_hamming_cos[i + 3];
   }

   memset(&t[L_WINDOW], 0, M * sizeof(Float32));
   memset(r, 0, (M + 1) * sizeof(Float32));

   for (j = 0; j < L_WINDOW; j++)
   {
      r[0] += t[j] * t[j];
      r[1] += t[j] * t[j + 1];
      r[2] += t[j] * t[j + 2];
      r[3] += t[j] * t[j + 3];
      r[4] += t[j] * t[j + 4];
      r[5] += t[j] * t[j + 5];
      r[6] += t[j] * t[j + 6];
      r[7] += t[j] * t[j + 7];
      r[8] += t[j] * t[j + 8];
      r[9] += t[j] * t[j + 9];
      r[10] += t[j] * t[j + 10];
      r[11] += t[j] * t[j + 11];
      r[12] += t[j] * t[j + 12];
      r[13] += t[j] * t[j + 13];
      r[14] += t[j] * t[j + 14];
      r[15] += t[j] * t[j + 15];
      r[16] += t[j] * t[j + 16];
   }

   if (r[0] < 1.0F)
   {
      r[0] = 1.0F;
   }

   return;
}
