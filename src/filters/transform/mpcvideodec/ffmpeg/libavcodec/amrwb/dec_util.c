/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <math.h>
#include <memory.h>
#include "typedef.h"
#include "dec_main.h"
#include "dec_lpc.h"

#define MAX_16       (Word16)0x7FFF
#define MIN_16       (Word16)0x8000
#define L_SUBFR      64       /* Subframe size                    */
#define L_SUBFR16k   80       /* Subframe size at 16kHz           */
#define M16k         20       /* Order of LP filter               */
#define PREEMPH_FAC  22282    /* preemphasis factor (0.68 in Q15) */
#define FAC4         4
#define FAC5         5
#define UP_FAC       20480    /* 5/4 in Q14                       */
#define INV_FAC5     6554     /* 1/5 in Q15                       */
#define NB_COEF_UP   12
#define L_FIR        31
#define MODE_7k      0
#define MODE_24k     8


extern const Word16 D_ROM_pow2[];
extern const Word16 D_ROM_isqrt[];
extern const Word16 D_ROM_log2[];
extern const Word16 D_ROM_fir_up[];
extern const Word16 D_ROM_fir_6k_7k[];
extern const Word16 D_ROM_fir_7k[];
extern const Word16 D_ROM_hp_gain[];

#ifdef WIN32
#pragma warning( disable : 4310)
#endif
/*
 * D_UTIL_random
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
Word16 D_UTIL_random(Word16 *seed)
{
   /*static Word16 seed = 21845;*/
   *seed = (Word16)(*seed * 31821L + 13849L);
   return(*seed);
}


/*
 * D_UTIL_pow2
 *
 * Parameters:
 *    exponant    I: (Q0) Integer part.      (range: 0 <= val <= 30)
 *    fraction    I: (Q15) Fractionnal part. (range: 0.0 <= val < 1.0)
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
Word32 D_UTIL_pow2(Word16 exponant, Word16 fraction)
{
	Word32 L_x, tmp, i, exp;
	Word16 a;

	L_x = fraction * 32;          /* L_x = fraction<<6             */
	i = L_x >> 15;                /* Extract b10-b16 of fraction   */
	a = (Word16)(L_x);            /* Extract b0-b9   of fraction   */
	a = (Word16)(a & (Word16)0x7fff);
	L_x = D_ROM_pow2[i] << 16;    /* table[i] << 16                */
	tmp = D_ROM_pow2[i] - D_ROM_pow2[i + 1];  /* table[i] - table[i+1] */
	tmp = L_x - ((tmp * a) << 1); /* L_x -= tmp*a*2                */
	exp = 30 - exponant;
	if (exp <= 31)
	{
		L_x = tmp >> exp;

		if ((1 << (exp - 1)) & tmp)
		{
			L_x++;
		}
	}
	else
	{
		L_x = 0;
	}

	return(L_x);
}


/*
 * D_UTIL_norm_l
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
 *    in order to normalize the result, the following operation must be done :
 *    norm_L_var1 = L_shl(L_var1,norm_l(L_var1)).
 *
 * Returns:
 *    16 bit Word16 signed integer (Word16) whose value falls in the range
 *    0x0000 0000 <= var_out <= 0x0000 001f.
 */
Word16 D_UTIL_norm_l(Word32 L_var1)
{
   Word16 var_out;

   if(L_var1 == 0)
   {
      var_out = 0;
   }
   else
   {
      if(L_var1 == (Word32)0xffffffffL)
      {
         var_out = 31;
      }
      else
      {
         if(L_var1 < 0)
         {
            L_var1 = ~L_var1;
         }

         for(var_out = 0; L_var1 < (Word32)0x40000000L; var_out++)
         {
            L_var1 <<= 1;
         }
      }
   }

   return(var_out);
}


/*
 * D_UTIL_norm_s
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
Word16 D_UTIL_norm_s(Word16 var1)
{
   Word16 var_out;

   if(var1 == 0)
   {
      var_out = 0;
   }
   else
   {
      if(var1 == -1)
      {
         var_out = 15;
      }
      else
      {
         if(var1 < 0)
         {
            var1 = (Word16)~var1;
         }

         for(var_out = 0; var1 < 0x4000; var_out++)
         {
            var1 <<= 1;
         }
      }
   }
   return(var_out);
}


/*
 * D_UTIL_dot_product12
 *
 * Parameters:
 *    x        I: 12bit x vector
 *    y        I: 12bit y vector
 *    lg       I: vector length
 *    exp      O: exponent of result (0..+30)
 *
 * Function:
 *    Compute scalar product of <x[],y[]> using accumulator.
 *    The result is normalized (in Q31) with exponent (0..30).
 *
 * Returns:
 *    Q31 normalised result (1 < val <= -1)
 */
Word32 D_UTIL_dot_product12(Word16 x[], Word16 y[], Word16 lg, Word16 *exp)
{
   Word32 sum, i, sft;

   sum = 0L;

   for(i = 0; i < lg; i++)
   {
      sum += x[i] * y[i];
   }
   sum = (sum << 1) + 1;

   /* Normalize acc in Q31 */
   sft = D_UTIL_norm_l(sum);
   sum = sum << sft;
   *exp = (Word16)(30 - sft);   /* exponent = 0..30 */

   return(sum);
}


/*
 * D_UTIL_normalised_inverse_sqrt
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
void D_UTIL_normalised_inverse_sqrt(Word32 *frac, Word16 *exp)
{
   Word32 i, tmp;
   Word16 a;

   if(*frac <= (Word32)0)
   {
      *exp = 0;
      *frac = 0x7fffffffL;
      return;
   }

   if((*exp & 0x1) == 1)   /* If exponant odd -> shift right */
   {
      *frac = *frac >> 1;
   }
   *exp = (Word16)(-((*exp - 1) >> 1));
   *frac = *frac >> 9;
   i = *frac >>16;      /* Extract b25-b31   */
   *frac = *frac >> 1;
   a = (Word16)(*frac); /* Extract b10-b24   */
   a = (Word16)(a & (Word16)0x7fff);
   i = i - 16;
   *frac = D_ROM_isqrt[i] << 16; /* table[i] << 16    */
   tmp = D_ROM_isqrt[i] - D_ROM_isqrt[i + 1];   /* table[i] - table[i+1]) */
   *frac = *frac - ((tmp * a) << 1);   /* frac -=  tmp*a*2  */

   return;
}


/*
 * D_UTIL_inverse_sqrt
 *
 * Parameters:
 *    L_x     I/O: (Q0) input value (range: 0<=val<=7fffffff)
 *
 * Function:
 *    Compute 1/sqrt(L_x).
 *    If value is negative or zero, result is 1 (7fffffff).
 *
 *    The function 1/sqrt(value) is approximated by a table and linear
 *    interpolation.
 *    1. Normalization of L_x
 *    2. call Normalised_Inverse_sqrt(L_x, exponant)
 *    3. L_y = L_x << exponant
 *
 * Returns:
 *    (Q31) output value (range: 0 <= val < 1)
 */
Word32 D_UTIL_inverse_sqrt(Word32 L_x)
{
   Word32 L_y;
   Word16 exp;

   exp = D_UTIL_norm_l(L_x);
   L_x = (L_x << exp);   /* L_x is normalized */
   exp = (Word16)(31 - exp);
   D_UTIL_normalised_inverse_sqrt(&L_x, &exp);

   if(exp < 0)
   {
      L_y = (L_x >> -exp);   /* denormalization   */
   }
   else
   {
      L_y = (L_x << exp);   /* denormalization   */
   }

   return(L_y);
}


/*
 * D_UTIL_normalised_log2
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
static void D_UTIL_normalised_log2(Word32 L_x, Word16 exp, Word16 *exponent,
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
   L_y = D_ROM_log2[i] << 16;               /* table[i] << 16        */
   tmp = D_ROM_log2[i] - D_ROM_log2[i + 1]; /* table[i] - table[i+1] */
   L_y = L_y - ((tmp * a) << 1);            /* L_y -= tmp*a*2        */
   *fraction = (Word16)(L_y >> 16);

   return;
}


/*
 * D_UTIL_log2
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
void D_UTIL_log2(Word32 L_x, Word16 *exponent, Word16 *fraction)
{
   Word16 exp;

   exp = D_UTIL_norm_l(L_x);
   D_UTIL_normalised_log2((L_x <<exp), exp, exponent, fraction);
}


/*
 * D_UTIL_l_extract
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
void D_UTIL_l_extract(Word32 L_32, Word16 *hi, Word16 *lo)
{
   *hi = (Word16)(L_32 >> 16);
   *lo = (Word16)((L_32 >> 1) - (*hi * 32768));

   return;
}


/*
 * D_UTIL_mpy_32_16
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
Word32 D_UTIL_mpy_32_16(Word16 hi, Word16 lo, Word16 n)
{
   Word32 L_32;

   L_32 = hi * n;
   L_32 += (lo * n) >> 15;

   return(L_32 << 1);
}


/*
 * D_UTIL_mpy_32
 *
 * Parameters:
 *    hi1         I: hi part of first number
 *    lo1         I: lo part of first number
 *    hi2         I: hi part of second number
 *    lo2         I: lo part of second number
 *
 * Function:
 *    Multiply two 32 bit integers (DPF). The result is divided by 2^31
 *
 *    L_32 = (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1
 *
 * Returns:
 *    32 bit result
 */
Word32 D_UTIL_mpy_32(Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2)
{
   Word32 L_32;

   L_32 = hi1 * hi2;
   L_32 += (hi1 * lo2) >> 15;
   L_32 += (lo1 * hi2) >> 15;

   return(L_32 << 1);
}

/*
 * D_UTIL_saturate
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
Word16 D_UTIL_saturate(Word32 inp)
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
 * D_UTIL_signal_up_scale
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
void D_UTIL_signal_up_scale(Word16 x[], Word16 lg, Word16 exp)
{
    Word32 i, tmp;

    for (i = 0; i < lg; i++)
    {
       tmp = x[i] << exp;
       x[i] = D_UTIL_saturate(tmp);
    }

    return;
}


/*
 * D_UTIL_signal_down_scale
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
void D_UTIL_signal_down_scale(Word16 x[], Word16 lg, Word16 exp)
{
   Word32 i, tmp;

   for(i = 0; i < lg; i++)
   {
      tmp = x[i] << 16;
      tmp = tmp >> exp;
      x[i] = (Word16)((tmp + 0x8000) >> 16);
   }

   return;
}


/*
 * D_UTIL_deemph_32
 *
 * Parameters:
 *    x_hi           I: input signal (bit31..16)
 *    x_lo           I: input signal (bit15..4)
 *    y              O: output signal (x16)
 *    mu             I: (Q15) deemphasis factor
 *    L              I: vector size
 *    mem          I/O: memory (y[-1])
 *
 * Function:
 *    Filtering through 1/(1-mu z^-1)
 *
 * Returns:
 *    void
 */
static void D_UTIL_deemph_32(Word16 x_hi[], Word16 x_lo[], Word16 y[],
                             Word16 mu, Word16 L, Word16 *mem)
{
   Word32 i, fac;
   Word32 tmp;

   fac = mu >> 1;   /* Q15 --> Q14 */

   /* L_tmp = hi<<16 + lo<<4 */
   tmp = (x_hi[0] << 12) + x_lo[0];
   tmp = (tmp << 6) + (*mem * fac);
   tmp = (tmp + 0x2000) >> 14;
   y[0] = D_UTIL_saturate(tmp);

   for(i = 1; i < L; i++)
   {
      tmp = (x_hi[i] << 12) + x_lo[i];
      tmp = (tmp << 6) + (y[i - 1] * fac);
      tmp = (tmp + 0x2000) >> 14;
      y[i] = D_UTIL_saturate(tmp);
   }

   *mem = y[L - 1];

   return;
}


/*
 * D_UTIL_synthesis_32
 *
 * Parameters:
 *    a              I: LP filter coefficients
 *    m              I: order of LP filter
 *    exc            I: excitation
 *    Qnew           I: exc scaling = 0(min) to 8(max)
 *    sig_hi         O: synthesis high
 *    sig_lo         O: synthesis low
 *    lg             I: size of filtering
 *
 * Function:
 *    Perform the synthesis filtering 1/A(z).
 *
 * Returns:
 *    void
 */
static void D_UTIL_synthesis_32(Word16 a[], Word16 m, Word16 exc[],
                                Word16 Qnew, Word16 sig_hi[], Word16 sig_lo[],
                                Word16 lg)
{
   Word32 i, j, a0, s;
   Word32 tmp, tmp2;

   /* See if a[0] is scaled */
   s = D_UTIL_norm_s((Word16)a[0]) - 2;

   a0 = a[0] >> (4 + Qnew);   /* input / 16 and >>Qnew */

   /* Do the filtering. */
   for(i = 0; i < lg; i++)
   {
      tmp = 0;

      for(j = 1; j <= m; j++)
      {
         tmp -= sig_lo[i - j] * a[j];
      }

      tmp = tmp >> (15 - 4);   /* -4 : sig_lo[i] << 4 */

      tmp2 = exc[i] * a0;

      for(j = 1; j <= m; j++)
      {
         tmp2 -= sig_hi[i - j] * a[j];
      }

      tmp += tmp2 << 1;
      tmp <<= s;

      /* sig_hi = bit16 to bit31 of synthesis */
      sig_hi[i] = (Word16)(tmp >> 13);

      /* sig_lo = bit4 to bit15 of synthesis */
      sig_lo[i] = (Word16)((tmp  >> 1) - (sig_hi[i] * 4096));
   }

   return;
}


/*
 * D_UTIL_hp50_12k8
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
static void D_UTIL_hp50_12k8(Word16 signal[], Word16 lg, Word16 mem[])
{
   Word32 i, L_tmp;
   Word16 y2_hi, y2_lo, y1_hi, y1_lo, x0, x1, x2;

   y2_hi = mem[0];
   y2_lo = mem[1];
   y1_hi = mem[2];
   y1_lo = mem[3];
   x0 = mem[4];
   x1 = mem[5];

   for(i = 0; i < lg; i++)
   {
      x2 = x1;
      x1 = x0;
      x0 = signal[i];

      /* y[i] = b[0]*x[i] + b[1]*x[i-1] + b140[2]*x[i-2]  */
      /* + a[1]*y[i-1] + a[2] * y[i-2];  */
      L_tmp = 8192L;   /* rounding to maximise precision */
      L_tmp = L_tmp + (y1_lo * 16211);
      L_tmp = L_tmp + (y2_lo * (-8021));
      L_tmp = L_tmp >> 14;
      L_tmp = L_tmp + (y1_hi * 32422);
      L_tmp = L_tmp + (y2_hi * (-16042));
      L_tmp = L_tmp + (x0 * 8106);
      L_tmp = L_tmp + (x1 * (-16212));
      L_tmp = L_tmp + (x2 * 8106);
      L_tmp = L_tmp << 2;  /* coeff Q11 --> Q14 */
      y2_hi = y1_hi;
      y2_lo = y1_lo;
      D_UTIL_l_extract(L_tmp, &y1_hi, &y1_lo);
      L_tmp = (L_tmp + 0x4000) >> 15;   /* coeff Q14 --> Q15 with saturation */
      signal[i] = D_UTIL_saturate(L_tmp);

   }
   mem[0] = y2_hi;
   mem[1] = y2_lo;
   mem[2] = y1_hi;
   mem[3] = y1_lo;
   mem[4] = x0;
   mem[5] = x1;

   return;
}


/*
 * D_UTIL_interpol
 *
 * Parameters:
 *    x           I: input vector
 *    fir         I: filter coefficient
 *    frac        I: fraction (0..resol)
 *    up_samp     I: resolution
 *    nb_coef     I: number of coefficients
 *
 * Function:
 *    Fractional interpolation of signal at position (frac/up_samp)
 *
 * Returns:
 *    result of interpolation
 */
Word16 D_UTIL_interpol(Word16 *x, Word16 const *fir, Word16 frac,
                       Word16 resol, Word16 nb_coef)
{
   Word32 i, k;
   Word32 sum;

   x = x - nb_coef + 1;
   sum = 0L;

   for(i = 0, k = ((resol - 1) - frac); i < 2 * nb_coef; i++,
      k = (Word16)(k + resol))
   {
      sum = sum + (x[i] * fir[k]);
   }

   if((sum < 536846336) & (sum > -536879104))
   {
      sum = (sum + 0x2000) >> 14;
   }
   else if(sum > 536846336)
   {
      sum = 32767;
   }
   else
   {
      sum = -32768;
   }

   return((Word16)sum);   /* saturation can occur here */
}


/*
 * D_UTIL_up_samp
 *
 * Parameters:
 *    res_d          I: signal to upsampling
 *    res_u          O: upsampled output
 *    L_frame        I: length of output
 *
 * Function:
 *    Upsampling
 *
 * Returns:
 *    void
 */
static void D_UTIL_up_samp(Word16 *sig_d, Word16 *sig_u, Word16 L_frame)
{
   Word32 pos, i, j;
   Word16 frac;

   pos = 0;   /* position with 1/5 resolution */

   for(j = 0; j < L_frame; j++)
   {
      i = (pos * INV_FAC5) >> 15;   /* integer part = pos * 1/5 */
      frac = (Word16)(pos - ((i << 2) + i));   /* frac = pos - (pos/5)*5   */
      sig_u[j] = D_UTIL_interpol(&sig_d[i], D_ROM_fir_up, frac, FAC5, NB_COEF_UP);
      pos = pos + FAC4;   /* position + 4/5 */
   }

   return;
}


/*
 * D_UTIL_oversamp_16k
 *
 * Parameters:
 *    sig12k8        I: signal to oversampling
 *    lg             I: length of input
 *    sig16k         O: oversampled signal
 *    mem          I/O: memory (2*12)
 *
 * Function:
 *    Oversampling from 12.8kHz to 16kHz
 *
 * Returns:
 *    void
 */
static void D_UTIL_oversamp_16k(Word16 sig12k8[], Word16 lg, Word16 sig16k[],
                                Word16 mem[])
{
   Word16 lg_up;
   Word16 signal[L_SUBFR + (2 * NB_COEF_UP)];

   memcpy(signal, mem, (2 * NB_COEF_UP) * sizeof(Word16));
   memcpy(signal + (2 * NB_COEF_UP), sig12k8, lg * sizeof(Word16));
   lg_up = (Word16)(((lg * UP_FAC) >> 15) << 1);
   D_UTIL_up_samp(signal + NB_COEF_UP, sig16k, lg_up);
   memcpy(mem, signal + lg, (2 * NB_COEF_UP) * sizeof(Word16));

   return;
}


/*
 * D_UTIL_hp400_12k8
 *
 * Parameters:
 *    signal       I/O: signal
 *    lg             I: lenght of signal
 *    mem          I/O: filter memory [6]
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
void D_UTIL_hp400_12k8(Word16 signal[], Word16 lg, Word16 mem[])
{

   Word32 i, L_tmp;
   Word16 y2_hi, y2_lo, y1_hi, y1_lo, x0, x1, x2;

   y2_hi = mem[0];
   y2_lo = mem[1];
   y1_hi = mem[2];
   y1_lo = mem[3];
   x0 = mem[4];
   x1 = mem[5];

   for(i = 0; i < lg; i++)
   {
      x2 = x1;
      x1 = x0;
      x0 = signal[i];

      /* y[i] = b[0]*x[i] + b[1]*x[i-1] + b140[2]*x[i-2]  */
      /* + a[1]*y[i-1] + a[2] * y[i-2];  */
      L_tmp = 8192L + (y1_lo * 29280);
      L_tmp = L_tmp + (y2_lo * (-14160));
      L_tmp = (L_tmp >> 14);
      L_tmp = L_tmp + (y1_hi * 58560);
      L_tmp = L_tmp + (y2_hi * (-28320));
      L_tmp = L_tmp + (x0 * 1830);
      L_tmp = L_tmp + (x1 * (-3660));
      L_tmp = L_tmp + (x2 * 1830);
      L_tmp = (L_tmp << 1);   /* coeff Q12 --> Q13 */
      y2_hi = y1_hi;
      y2_lo = y1_lo;
      D_UTIL_l_extract(L_tmp, &y1_hi, &y1_lo);

      /* signal is divided by 16 to avoid overflow in energy computation */
      signal[i] = (Word16)((L_tmp + 0x8000) >> 16);
   }
   mem[0] = y2_hi;
   mem[1] = y2_lo;
   mem[2] = y1_hi;
   mem[3] = y1_lo;
   mem[4] = x0;
   mem[5] = x1;

   return;
}


/*
 * D_UTIL_synthesis
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
 *
 * Returns:
 *    void
 */
static void D_UTIL_synthesis(Word16 a[], Word16 m, Word16 x[], Word16 y[],
                             Word16 lg, Word16 mem[], Word16 update)
{
   Word32 i, j, tmp, s;
   Word16 y_buf[L_SUBFR16k + M16k], a0;
   Word16 *yy;

   yy = &y_buf[m];

   /* See if a[0] is scaled */
   s = D_UTIL_norm_s(a[0]) - 2;
   /* copy initial filter states into synthesis buffer */
   memcpy(y_buf, mem, m * sizeof(Word16));

   a0 = (Word16)(a[0] >> 1);   /* input / 2 */

   /* Do the filtering. */
   for(i = 0; i < lg; i++)
   {
      tmp = x[i] * a0;

      for(j = 1; j <= m; j++)
      {
         tmp -= a[j] * yy[i - j];
      }
      tmp <<= s;

      y[i] = yy[i] = (Word16)((tmp + 0x800) >> 12);
   }

   /* Update memory if required */
   if(update)
   {
      memcpy(mem, &yy[lg - m], m * sizeof(Word16));
   }

   return;
}


/*
 * D_UTIL_bp_6k_7k
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
void D_UTIL_bp_6k_7k(Word16 signal[], Word16 lg, Word16 mem[])
{
   Word32 x[L_SUBFR16k + (L_FIR - 1)];
   Word32 i, j, tmp;

   for(i = 0; i < (L_FIR - 1); i++)
   {
      x[i] = (Word16)mem[i];   /* gain of filter = 4 */
   }

   for(i = 0; i < lg; i++)
   {
      x[i + L_FIR - 1] = signal[i] >> 2;   /* gain of filter = 4 */
   }

   for(i = 0; i < lg; i++)
   {
      tmp = 0;

      for(j = 0; j < L_FIR; j++)
      {
         tmp += x[i + j] * D_ROM_fir_6k_7k[j];
      }

      signal[i] = (Word16)((tmp + 0x4000) >> 15);
   }

   for(i = 0; i < (L_FIR - 1); i++)
   {
      mem[i] = (Word16)x[lg + i];   /* gain of filter = 4 */
   }

   return;
}


/*
 * D_UTIL_hp_7k
 *
 * Parameters:
 *    signal          I/O: ISF vector
 *    lg                I: length of signal
 *    mem             I/O: memory (30)
 *
 * Function:
 *    15th order high pass 7kHz FIR filter
 *
 * Returns:
 *    void
 */
static void D_UTIL_hp_7k(Word16 signal[], Word16 lg, Word16 mem[])
{

   Word32 i, j, tmp;
   Word16 x[L_SUBFR16k + (L_FIR - 1)];

   memcpy(x, mem, (L_FIR - 1) * sizeof(Word16));
   memcpy(&x[L_FIR - 1], signal, lg * sizeof(Word16));

   for(i = 0; i < lg; i++)
   {
      tmp = 0;

      for(j = 0; j < L_FIR; j++)
      {
         tmp += x[i + j] * D_ROM_fir_7k[j];
      }

      signal[i] = (Word16)((tmp + 0x4000) >> 15);
   }

   memcpy(mem, x + lg, (L_FIR - 1) * sizeof(Word16));

   return;
}


/*
 * D_UTIL_Dec_synthesis
 *
 * Parameters:
 *    Aq             I: quantized Az
 *    exc            I: excitation at 12kHz
 *    Q_new          I: scaling performed on exc
 *    synth16k       O: 16kHz synthesis signal
 *    prms           I: parameters
 *    HfIsf        I/O: High frequency ISF:s
 *    mode           I: codec mode
 *    newDTXState    I: dtx state
 *    bfi            I: bad frame indicator
 *    st           I/O: State structure
 *
 * Function:
 *    Synthesis of signal at 16kHz with HF extension.
 *
 * Returns:
 *    void
 */
void D_UTIL_dec_synthesis(Word16 Aq[], Word16 exc[], Word16 Q_new,
                          Word16 synth16k[], Word16 prms, Word16 HfIsf[],
                          Word16 mode, Word16 newDTXState, Word16 bfi,
                          Decoder_State *st)
{
   Word32 tmp, i;
   Word16 exp;
   Word16 ener, exp_ener;
   Word32 fac;
   Word16 synth_hi[M + L_SUBFR], synth_lo[M + L_SUBFR];
   Word16 synth[L_SUBFR];
   Word16 HF[L_SUBFR16k];   /* High Frequency vector      */
   Word16 Ap[M16k + 1];
   Word16 HfA[M16k + 1];
   Word16 HF_corr_gain;
   Word16 HF_gain_ind;
   Word32 gain1, gain2;
   Word16 weight1, weight2;

   /*
    * Speech synthesis
    *
    * - Find synthesis speech corresponding to exc2[].
    * - Perform fixed deemphasis and hp 50hz filtering.
    * - Oversampling from 12.8kHz to 16kHz.
    */
   memcpy(synth_hi, st->mem_syn_hi, M * sizeof(Word16));
   memcpy(synth_lo, st->mem_syn_lo, M * sizeof(Word16));
   D_UTIL_synthesis_32(Aq, M, exc, Q_new, synth_hi + M, synth_lo + M, L_SUBFR);
   memcpy(st->mem_syn_hi, synth_hi + L_SUBFR, M * sizeof(Word16));
   memcpy(st->mem_syn_lo, synth_lo + L_SUBFR, M * sizeof(Word16));
   D_UTIL_deemph_32(synth_hi + M, synth_lo + M, synth, PREEMPH_FAC, L_SUBFR,
      &(st->mem_deemph));
   D_UTIL_hp50_12k8(synth, L_SUBFR, st->mem_sig_out);
   D_UTIL_oversamp_16k(synth, L_SUBFR, synth16k, st->mem_oversamp);

   /*
    * HF noise synthesis
    *
    * - Generate HF noise between 5.5 and 7.5 kHz.
    * - Set energy of noise according to synthesis tilt.
    *     tilt > 0.8 ==> - 14 dB (voiced)
    *     tilt   0.5 ==> - 6 dB  (voiced or noise)
    *     tilt < 0.0 ==>   0 dB  (noise)
    */

   /* generate white noise vector */
   for(i = 0; i < L_SUBFR16k; i++)
   {
      HF[i] = (Word16)(D_UTIL_random(&(st->mem_seed2)) >> 3);
   }

   /* energy of excitation */
   D_UTIL_signal_down_scale(exc, L_SUBFR, 3);
   Q_new = (Word16)(Q_new - 3);
   ener = (Word16)(D_UTIL_dot_product12(exc, exc, L_SUBFR, &exp_ener) >> 16);
   exp_ener = (Word16)(exp_ener - (Q_new << 1));

   /* set energy of white noise to energy of excitation */
   tmp = (Word16)(D_UTIL_dot_product12(HF, HF, L_SUBFR16k, &exp) >> 16);

   if(tmp > ener)
   {
      tmp = tmp >> 1;   /* Be sure tmp < ener */
      exp = (Word16)(exp + 1);
   }

   tmp = (tmp << 15) / ener;

   if(tmp > 32767)
   {
      tmp = 32767;
   }

   tmp = tmp << 16;   /* result is normalized */
   exp = (Word16)(exp - exp_ener);
   D_UTIL_normalised_inverse_sqrt(&tmp, &exp);

   /* L_tmp x 2, L_tmp in Q31 */
   /* tmp = 2 x sqrt(ener_exc/ener_hf) */
   if(exp >= 0)
   {
      tmp = tmp >> (15 - exp);
   }
   else
   {
      tmp = tmp >> (-exp);
      tmp = tmp >> 15;
   }

   /* saturation */
   if(tmp > 0x7FFF)
   {
      tmp = 0x7FFF;
   }

   for(i = 0; i < L_SUBFR16k; i++)
   {
      HF[i] = (Word16)((HF[i] * tmp) >> 15);
   }

   /* find tilt of synthesis speech (tilt: 1=voiced, -1=unvoiced) */
   D_UTIL_hp400_12k8(synth, L_SUBFR, st->mem_hp400);
   tmp = 0L;

   for(i = 0; i < L_SUBFR; i++)
   {
      tmp = tmp + (synth[i] * synth[i]);
   }

   tmp = (tmp << 1) + 1;
   exp = D_UTIL_norm_l(tmp);
   ener = (Word16)((tmp << exp) >> 16);   /* ener = r[0] */
   tmp = 0L;

   for(i = 1; i < L_SUBFR; i++)
   {
      tmp = tmp + (synth[i] * synth[i - 1]);
   }

   tmp = (tmp << 1) + 1;
   tmp = (tmp << exp) >> 16;   /* tmp = r[1] */

   if(tmp > 0)
   {
      fac = ((tmp << 15) / ener);

      if(fac > 32767)
      {
         fac = 32767;
      }
   }
   else
   {
      fac = 0;
   }

   /* modify energy of white noise according to synthesis tilt */
   gain1 = (32767 - fac);
   gain2 = ((32767 - fac) * 20480) >> 15;
   gain2 = (gain2 << 1);

   if(gain2 > 32767)
      gain2 = 32767;

   if(st->mem_vad_hist > 0)
   {
      weight1 = 0;
      weight2 = 32767;
   }
   else
   {
      weight1 = 32767;
      weight2 = 0;
   }

   tmp = (weight1 * gain1) >> 15;
   tmp = tmp + ((weight2 * gain2) >> 15);

   if(tmp != 0)
   {
      tmp = tmp + 1;
   }

   if(tmp < 3277)
   {
      tmp = 3277;   /* 0.1 in Q15 */
   }

   if((mode == MODE_24k) & (bfi == 0))
   {
      /* HF correction gain */
      HF_gain_ind = prms;
      HF_corr_gain = D_ROM_hp_gain[HF_gain_ind];

      /* HF gain */
      for(i = 0; i < L_SUBFR16k; i++)
      {
         HF[i] = (Word16)(((HF[i] * HF_corr_gain) >> 15) << 1);
      }
   }
   else
   {
      for(i = 0; i < L_SUBFR16k; i++)
      {
         HF[i] = (Word16)((HF[i] * tmp) >> 15);
      }
   }

   if((mode <= MODE_7k) & (newDTXState == SPEECH))
   {
      D_LPC_isf_extrapolation(HfIsf);
      D_LPC_isp_a_conversion(HfIsf, HfA, 0, M16k);
      D_LPC_a_weight(HfA, Ap, 29491, M16k);   /* fac=0.9 */
      D_UTIL_synthesis(Ap, M16k, HF, HF, L_SUBFR16k, st->mem_syn_hf, 1);
   }
   else
   {
      /* synthesis of noise: 4.8kHz..5.6kHz --> 6kHz..7kHz */
      D_LPC_a_weight(Aq, Ap, 19661, M);   /* fac=0.6 */
      D_UTIL_synthesis(Ap, M, HF, HF, L_SUBFR16k, st->mem_syn_hf + (M16k - M), 1);
   }

   /* noise High Pass filtering (1ms of delay) */
   D_UTIL_bp_6k_7k(HF, L_SUBFR16k, st->mem_hf);

   if(mode == MODE_24k)
   {
      /* Low Pass filtering (7 kHz) */
      D_UTIL_hp_7k(HF, L_SUBFR16k, st->mem_hf3);
   }

   /* add filtered HF noise to speech synthesis */
   for(i = 0; i < L_SUBFR16k; i++)
   {
      tmp = (synth16k[i] + HF[i]);
      synth16k[i] = D_UTIL_saturate(tmp);
   }

   return;
}


/*
 * D_UTIL_preemph
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
void D_UTIL_preemph(Word16 x[], Word16 mu, Word16 lg, Word16 *mem)
{
   Word32 i, L_tmp;
   Word16 temp;

   temp = x[lg - 1];

   for(i = lg - 1; i > 0; i--)
   {
      L_tmp = x[i] << 15;
      L_tmp = L_tmp - (x[i - 1] * mu);
      x[i] = (Word16)((L_tmp + 0x4000) >> 15);
   }

   L_tmp = x[0] << 15;
   L_tmp = L_tmp - (*mem * mu);
   x[0] = (Word16)((L_tmp + 0x4000) >> 15);
   *mem = temp;

   return;
}
