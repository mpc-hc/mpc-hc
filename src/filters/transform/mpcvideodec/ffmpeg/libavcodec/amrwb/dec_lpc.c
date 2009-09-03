/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <math.h>
#include "typedef.h"
#include "dec_util.h"

#define M            16             /* Order of LP filter                  */
#define MP1          (M + 1)
#define M16k         20
#define NC16k        (M16k / 2)
#define MU           10923          /* Prediction factor (1.0/3.0) in Q15  */
#define L_MEANBUF    3
#define ALPHA        29491          /* 0. 9 in Q15                         */
#define ONE_ALPHA    (32768-ALPHA)  /* (1.0 - ALPHA) in Q15                */
#define ORDER        16             /* order of linear prediction filter   */
#define ISF_GAP      128            /* 50 Hz */
#define INV_LENGTH   2731           /* 1/12                                */

extern const Word16 D_ROM_dico1_isf[];
extern const Word16 D_ROM_dico2_isf[];
extern const Word16 D_ROM_dico21_isf_36b[];
extern const Word16 D_ROM_dico22_isf_36b[];
extern const Word16 D_ROM_dico23_isf_36b[];
extern const Word16 D_ROM_dico21_isf[];
extern const Word16 D_ROM_dico22_isf[];
extern const Word16 D_ROM_dico23_isf[];
extern const Word16 D_ROM_dico24_isf[];
extern const Word16 D_ROM_dico25_isf[];
extern const Word16 D_ROM_dico1_isf_noise[];
extern const Word16 D_ROM_dico2_isf_noise[];
extern const Word16 D_ROM_dico3_isf_noise[];
extern const Word16 D_ROM_dico4_isf_noise[];
extern const Word16 D_ROM_dico5_isf_noise[];
extern const Word16 D_ROM_mean_isf[];
extern const Word16 D_ROM_mean_isf_noise[];
extern const Word16 D_ROM_cos[];


/*
 * D_LPC_isf_reorder
 *
 * Parameters:
 *    isf          I/O: vector of isfs
 *    min_dist       I: quantized ISFs (in frequency domain)
 *    n              I: LPC order
 *
 * Function:
 *    To make sure that the  isfs are properly order and to keep a certain
 *    minimum distance between consecutive isfs.
 *
 * Returns:
 *    void
 */
static void D_LPC_isf_reorder(Word16 *isf, Word16 min_dist, Word16 n)
{
   Word32 i, isf_min;

   isf_min = min_dist;

   for(i = 0; i < n - 1; i++)
   {
      if(isf[i] < isf_min)
      {
         isf[i] = (Word16)isf_min;
      }
      isf_min = isf[i] + min_dist;
   }

   return;
}


/*
 * D_LPC_isf_noise_d
 *
 * Parameters:
 *    indice         I: indices of the selected codebook entries
 *    isf_q          O: quantized ISFs (in frequency domain)
 *
 * Function:
 *    Decoding of ISF parameters
 *
 * Returns:
 *    void
 */
void D_LPC_isf_noise_d(Word16 *indice, Word16 *isf_q)
{
   Word32 i;

   for(i = 0; i < 2; i++)
   {
      isf_q[i] = D_ROM_dico1_isf_noise[indice[0] * 2 + i];
   }

   for(i = 0; i < 3; i++)
   {
      isf_q[i + 2] = D_ROM_dico2_isf_noise[indice[1] * 3 + i];
   }

   for(i = 0; i < 3; i++)
   {
      isf_q[i + 5] = D_ROM_dico3_isf_noise[indice[2] * 3 + i];
   }

   for(i = 0; i < 4; i++)
   {
      isf_q[i + 8] = D_ROM_dico4_isf_noise[indice[3] * 4 + i];
   }

   for(i = 0; i < 4; i++)
   {
      isf_q[i + 12] = D_ROM_dico5_isf_noise[indice[4] * 4 + i];
   }

   for(i = 0; i < ORDER; i++)
   {
      isf_q[i] = (Word16)(isf_q[i]+ D_ROM_mean_isf_noise[i]);
   }

   D_LPC_isf_reorder(isf_q, ISF_GAP, ORDER);

   return;
}


/*
 * D_LPC_isf_isp_conversion
 *
 * Parameters:
 *    isp            O: (Q15) isp[m] (range: -1<=val<1)
 *    isf            I: (Q15) isf[m] normalized (range: 0.0 <= val <= 0.5)
 *    m              I: LPC order
 *
 * Function:
 *    Transformation isf to isp
 *
 *    ISP are immitance spectral pair in cosine domain (-1 to 1).
 *    ISF are immitance spectral pair in frequency domain (0 to 6400).
 * Returns:
 *    void
 */
void D_LPC_isf_isp_conversion(Word16 isf[], Word16 isp[], Word16 m)
{
   Word32 i, ind, offset, tmp;

   for(i = 0; i < m - 1; i++)
   {
      isp[i] = isf[i];
   }
   isp[m - 1] = (Word16)(isf[m - 1] << 1);

   for(i = 0; i < m; i++)
   {
      ind = isp[i] >> 7;         /* ind = b7-b15 of isf[i]     */
      offset = isp[i] & 0x007f;  /* offset = b0-b6 of isf[i]   */

      /* isp[i] = table[ind]+ ((table[ind+1]-table[ind])*offset) / 128 */
      tmp = (D_ROM_cos[ind + 1] - D_ROM_cos[ind]) * offset;
      isp[i] = (Word16)(D_ROM_cos[ind] + (tmp >> 7));
   }
   return;
}


/*
 * D_LPC_isp_pol_get
 *
 * Parameters:
 *    isp            I: Immitance spectral pairs (cosine domaine)
 *    f              O: the coefficients of F1 or F2
 *    n              I: no of coefficients (m/2)
 *    k16            I: 16k flag
 *
 * Function:
 *    Find the polynomial F1(z) or F2(z) from the ISPs.
 *    This is performed by expanding the product polynomials:
 *
 *    F1(z) =   product   ( 1 - 2 isp_i z^-1 + z^-2 )
 *            i=0,2,4,6,8
 *    F2(z) =   product   ( 1 - 2 isp_i z^-1 + z^-2 )
 *             i=1,3,5,7
 *
 *    where isp_i are the ISPs in the cosine domain.
 *
 * Returns:
 *    void
 */
static void D_LPC_isp_pol_get(Word16 *isp, Word32 *f, Word32 n, Word16 k16)
{
   Word32 i, j, t0, s1, s2;
   Word16 hi, lo;

   s1 = 8388608;
   s2 = 512;

   if(k16)
   {
      s1 >>= 2;
      s2 >>= 2;
   }

   /* All computation in Q23 */
   f[0] = s1;              /* f[0] = 1.0; in Q23         */
   f[1] = isp[0] * (-s2);  /* f[1] = -2.0*isp[0] in Q23  */
   f += 2;                 /* Advance f pointer          */
   isp += 2;               /* Advance isp pointer        */

   for(i = 2; i <= n; i++)
   {
      *f = f[ - 2];

      for(j = 1; j < i; j++, f--)
      {
         D_UTIL_l_extract(f[- 1], &hi, &lo);
         t0 = D_UTIL_mpy_32_16(hi, lo, *isp);   /* t0 = f[-1] * isp */
         t0 = (t0 << 1);
         *f = (*f - t0);         /* *f -= t0    */
         *f = (*f + f[ - 2]);    /* *f += f[-2] */
      }

      *f = *f - (*isp * s2);     /* *f -= isp << 8 */
      f += i;     /* Advance f pointer   */
      isp += 2;   /* Advance isp pointer */
   }

   return;
}


/*
 * D_LPC_isp_a_conversion
 *
 * Parameters:
 *    isp            I: (Q15) Immittance spectral pairs
 *    a              O: (Q12) Predictor coefficients (order = M)
 *    m              I: order of LP filter
 *
 * Function:
 *    Convert ISPs to predictor coefficients a[]
 *
 * Returns:
 *    void
 */
void D_LPC_isp_a_conversion(Word16 isp[], Word16 a[], Word32 adaptive_scaling, 
                            Word16 m)
{
   Word32 j, i, nc, tmax, q, q_sug, r;
   Word32 f1[NC16k + 1], f2[NC16k];
   Word32 t0;
   Word16 hi, lo;

   nc = m >> 1;

   if(nc > 8)
   {
      D_LPC_isp_pol_get(&isp[0], f1, nc, 1);

      for(i = 0; i <= nc; i++)
      {
         f1[i] = (f1[i] << 2);
      }
   }
   else
   {
      D_LPC_isp_pol_get(&isp[0], f1, nc, 0);
   }

   if(nc > 8)
   {
      D_LPC_isp_pol_get(&isp[1], f2, nc - 1, 1);

      for(i = 0; i <= nc - 1; i++)
      {
         f2[i] = (f2[i] << 2);
      }
   }
   else
   {
      D_LPC_isp_pol_get(&isp[1], f2, nc - 1, 0);
   }

   /*
    *  Multiply F2(z) by (1 - z^-2)
    */
   for(i = nc - 1; i > 1; i--)
   {
      f2[i] = f2[i] - f2[i - 2];   /* f2[i] -= f2[i-2]; */
   }

   /*
    *  Scale F1(z) by (1+isp[m-1]) and F2(z) by (1-isp[m-1])
    */
   for(i = 0; i < nc; i++)
   {
      /* f1[i] *= (1.0 + isp[M-1]); */
      D_UTIL_l_extract(f1[i], &hi, &lo);
      t0 = D_UTIL_mpy_32_16(hi, lo, isp[m - 1]);
      f1[i] = f1[i] + t0;

      /* f2[i] *= (1.0 - isp[M-1]); */
      D_UTIL_l_extract(f2[i], &hi, &lo);
      t0 = D_UTIL_mpy_32_16(hi, lo, isp[m - 1]);
      f2[i] = f2[i] - t0;
   }

   /*
    *  A(z) = (F1(z)+F2(z))/2
    *  F1(z) is symmetric and F2(z) is antisymmetric
    */

   /* a[0] = 1.0; */
   a[0] = 4096;
   tmax = 1;

   for(i = 1, j = m - 1; i < nc; i++, j--)
   {
      /* a[i] = 0.5*(f1[i] + f2[i]); */
      t0 = f1[i] + f2[i];   /* f1[i] + f2[i] */
      tmax |= labs(t0);
      a[i] = (Word16)((t0 + 0x800) >> 12);   /* from Q23 to Q12 and * 0.5 */
      
      /* a[j] = 0.5*(f1[i] - f2[i]); */
      t0 = (f1[i] - f2[i]);   /* f1[i] - f2[i] */
      tmax |= labs(t0);
      a[j] = (Word16)((t0 + 0x800) >> 12);   /* from Q23 to Q12 and * 0.5 */
   }

   /* rescale data if overflow has occured and reprocess the loop */

    if (adaptive_scaling)
    {
       q = 4 - D_UTIL_norm_l(tmax);        /* adaptive scaling enabled */
    }
    else
    {
       q = 0;                           /* adaptive scaling disabled */
    }

    if (q > 0) 
    {
      q_sug = 12 + q;
      r = 1 << (q_sug - 1);

      for (i = 1, j = m - 1; i < nc; i++, j--)
        {
          /* a[i] = 0.5*(f1[i] + f2[i]); */
          t0 = f1[i] + f2[i];          /* f1[i] + f2[i]             */
          a[i] = (Word16)((t0 + r) >> q_sug); /* from Q23 to Q12 and * 0.5 */
          
          /* a[j] = 0.5*(f1[i] - f2[i]); */
          t0 = f1[i] - f2[i];          /* f1[i] - f2[i]             */
          a[j] = (Word16)((t0 + r) >> q_sug); /* from Q23 to Q12 and * 0.5 */
        }
      a[0] = (Word16)(a[0] >> q);             
    } 
    else 
    {
      q_sug = 12;
      r = 1 << (q_sug - 1);
      q     = 0;                          
    }
   
   /* a[NC] = 0.5*f1[NC]*(1.0 + isp[M-1]); */
   D_UTIL_l_extract(f1[nc], &hi, &lo);
   t0 = D_UTIL_mpy_32_16(hi, lo, isp[m - 1]);
   t0 = f1[nc] + t0;
   a[nc] = (Word16)((t0 + r) >> q_sug);  /* from Q23 to Q12 and * 0.5 */

   /* a[m] = isp[m-1]; */
   a[m] = (Word16)((isp[m - 1] >> (2 + q)) + 1); /* from Q15 to Q12 */
   a[m] = (Word16)(a[m] >> 1);

   return;
}


/*
 * D_LPC_a_weight
 *
 * Parameters:
 *    a              I: LP filter coefficients
 *    ap             O: weighted LP filter coefficients
 *    gamma          I: weighting factor
 *    m              I: order of LP filter
 *
 * Function:
 *    Weighting of LP filter coefficients, ap[i] = a[i] * (gamma^i).
 *
 * Returns:
 *    void
 */
void D_LPC_a_weight(Word16 a[], Word16 ap[], Word16 gamma, Word16 m)
{
   Word32 i, fac;

   ap[0] = a[0];
   fac = gamma;

   for(i = 1; i < m; i++)
   {
      ap[i] = (Word16)(((a[i] * fac) + 0x4000) >> 15);
      fac = ((fac * gamma)  + 0x4000) >> 15;
   }

   ap[m] = (Word16)(((a[m] * fac) + 0x4000) >> 15);

   return;
}


/*
 * D_LPC_isf_2s3s_decode
 *
 * Parameters:
 *    indice            I: quantisation indices
 *    isf_q             O: quantised ISFs in the cosine domain
 *    past_isfq       I/O: past ISF quantizer
 *    isfold            I: past quantised ISF
 *    isf_buf           O: isf buffer
 *    bfi               I: Bad frame indicator
 *
 * Function:
 *    Decoding of ISF parameters.
 *
 * Returns:
 *    void
 */
void D_LPC_isf_2s3s_decode(Word16 *indice, Word16 *isf_q, Word16 *past_isfq,
                           Word16 *isfold, Word16 *isf_buf, Word16 bfi)
{

   Word32 ref_isf[M];
   Word32 L_tmp, i, j;
   Word16 tmp;

   if(bfi == 0) /* Good frame */
   {
      for(i = 0; i < 9; i++)
      {
         isf_q[i] = D_ROM_dico1_isf[indice[0] * 9 + i];
      }

      for(i = 0; i < 7; i++)
      {
         isf_q[i + 9] = D_ROM_dico2_isf[indice[1] * 7 + i];
      }

      for(i = 0; i < 5; i++)
      {
         isf_q[i] =
            (Word16)(isf_q[i] + D_ROM_dico21_isf_36b[indice[2] * 5 + i]);
      }

      for(i = 0; i < 4; i++)
      {
         isf_q[i + 5] =
            (Word16)(isf_q[i + 5] + D_ROM_dico22_isf_36b[indice[3] * 4 + i]);
      }

      for(i = 0; i < 7; i++)
      {
         isf_q[i + 9] =
            (Word16)(isf_q[i + 9] + D_ROM_dico23_isf_36b[indice[4] * 7 + i]);
      }

      for(i = 0; i < ORDER; i++)
      {
         tmp = isf_q[i];
         isf_q[i] =
            (Word16)((tmp + D_ROM_mean_isf[i]) + ((MU * past_isfq[i]) >> 15));
         past_isfq[i] = tmp;
      }

      for(i = 0; i < M; i++)
      {
         for(j = (L_MEANBUF - 1); j > 0; j--)
         {
            isf_buf[j * M + i] = isf_buf[(j - 1) * M + i];
         }
         isf_buf[i] = isf_q[i];
      }

   }
   else
   {   /* bad frame */

      for(i = 0; i < M; i++)
      {
         L_tmp = D_ROM_mean_isf[i];

         for(j = 0; j < L_MEANBUF; j++)
         {
            L_tmp = L_tmp + isf_buf[j * M + i];
         }
         ref_isf[i] = (L_tmp + 0x1) >> 2;
      }

      /* use the past ISFs slightly shifted towards their mean */
      for(i = 0; i < ORDER; i++)
      {
         isf_q[i] = (Word16)((((ALPHA * isfold[i]) >> 15) +
            ((ONE_ALPHA * ref_isf[i]) >> 15)));
      }

      /* estimate past quantized residual to be used in next frame */
      for(i = 0; i < ORDER; i++)
      {
         /* predicted ISF */
         L_tmp = ref_isf[i] + ((past_isfq[i] * MU) >> 15);
         /* past_isfq[i] *= 0.5 */
         past_isfq[i] = (Word16)((isf_q[i] - L_tmp) >> 1);
      }
   }

   D_LPC_isf_reorder(isf_q, ISF_GAP, ORDER);

   return;
}


/*
 * D_LPC_isf_2s5s_decode
 *
 * Parameters:
 *    indice            I: quantization indices
 *    isf_q             O: quantized ISFs in the cosine domain
 *    past_isfq       I/O: past ISF quantizer
 *    isfold            I: past quantized ISF
 *    isf_buf           O: isf buffer
 *    bfi               I: Bad frame indicator
 *
 * Function:
 *    Decoding of ISF parameters.
 *
 * Returns:
 *    void
 */
void D_LPC_isf_2s5s_decode(Word16 *indice, Word16 *isf_q, Word16 *past_isfq,
                           Word16 *isfold, Word16 *isf_buf, Word16 bfi)
{
   Word32 ref_isf[M];
   Word32 i, j, L_tmp;
   Word16 tmp;

   if(bfi == 0) /* Good frame */
   {
      for(i = 0; i < 9; i++)
      {
         isf_q[i] = D_ROM_dico1_isf[indice[0] * 9 + i];
      }

      for(i = 0; i < 7; i++)
      {
         isf_q[i + 9] = D_ROM_dico2_isf[indice[1] * 7 + i];
      }

      for(i = 0; i < 3; i++)
      {
         isf_q[i] = (Word16)(isf_q[i] + D_ROM_dico21_isf[indice[2] * 3 + i]);
      }

      for(i = 0; i < 3; i++)
      {
         isf_q[i + 3] =
            (Word16)(isf_q[i + 3] + D_ROM_dico22_isf[indice[3] * 3 + i]);
      }

      for(i = 0; i < 3; i++)
      {
         isf_q[i + 6] =
            (Word16)(isf_q[i + 6] + D_ROM_dico23_isf[indice[4] * 3 + i]);
      }

      for(i = 0; i < 3; i++)
      {
         isf_q[i + 9] =
            (Word16)(isf_q[i + 9] + D_ROM_dico24_isf[indice[5] * 3 + i]);
      }

      for(i = 0; i < 4; i++)
      {
         isf_q[i + 12] =
            (Word16)(isf_q[i + 12] + D_ROM_dico25_isf[indice[6] * 4 + i]);
      }

      for(i = 0; i < ORDER; i++)
      {
         tmp = isf_q[i];
         isf_q[i] =
            (Word16)((tmp + D_ROM_mean_isf[i]) + ((MU * past_isfq[i]) >> 15));
         past_isfq[i] = tmp;
      }


      for(i = 0; i < M; i++)
      {
         for(j = (L_MEANBUF - 1); j > 0; j--)
         {
            isf_buf[j * M + i] = isf_buf[(j - 1) * M + i];
         }
         isf_buf[i] = isf_q[i];
      }

   }
   else
   {   /* bad frame */

      for(i = 0; i < M; i++)
      {
         L_tmp = D_ROM_mean_isf[i];

         for(j = 0; j < L_MEANBUF; j++)
         {
            L_tmp = L_tmp + isf_buf[j * M + i];
         }

         ref_isf[i] = (L_tmp + 0x1) >> 2;
      }

      /* use the past ISFs slightly shifted towards their mean */
      for(i = 0; i < ORDER; i++)
      {
         isf_q[i] = (Word16)(((ALPHA * isfold[i]) >> 15) +
            ((ONE_ALPHA * ref_isf[i]) >> 15));
      }

      /* estimate past quantized residual to be used in next frame */
      for(i = 0; i < ORDER; i++)
      {
         /* predicted ISF */
         L_tmp = ref_isf[i] + ((past_isfq[i] * MU) >> 15);
         /* past_isfq[i] *= 0.5 */
         past_isfq[i] = (Word16)((isf_q[i] - L_tmp) >> 1);
      }
   }

   D_LPC_isf_reorder(isf_q, ISF_GAP, ORDER);

   return;
}


/*
 * D_LPC_int_isp_find
 *
 * Parameters:
 *    isp_old           I: isps from past frame
 *    isp_new           I: isps from present frame
 *    frac              I: (Q15) fraction for 3 first subfr
 *    Az                O: LP coefficients in 4 subframes
 *
 * Function:
 *    Find the interpolated ISP parameters for all subframes.
 *
 * Returns:
 *    void
 */
void D_LPC_int_isp_find(Word16 isp_old[], Word16 isp_new[],
                        const Word16 frac[], Word16 Az[])
{
   Word32 tmp, i, k, fac_old, fac_new;
   Word16 isp[M];

   for(k = 0; k < 3; k++)
   {
      fac_new = frac[k];
      fac_old = (32767 - fac_new) + 1;   /* 1.0 - fac_new */

      for(i = 0; i < M; i++)
      {
         tmp = isp_old[i] * fac_old;
         tmp += isp_new[i] * fac_new;
         isp[i] = (Word16)((tmp + 0x4000) >> 15);
      }

      D_LPC_isp_a_conversion(isp, Az, 0, M);
      Az += MP1;
   }

   /* 4th subframe: isp_new (frac=1.0) */
   D_LPC_isp_a_conversion(isp_new, Az, 0, M);

   return;
}


/*
 * D_LPC_isf_extrapolation
 *
 * Parameters:
 *    HfIsf          I/O: ISF vector
 *
 * Function:
 *    Conversion of 16th-order 12.8kHz ISF vector
 *    into 20th-order 16kHz ISF vector
 *
 * Returns:
 *    void
 */
void D_LPC_isf_extrapolation(Word16 HfIsf[])
{
   Word32 IsfDiff[M - 2];
   Word32 IsfCorr[3];
   Word32 tmp, tmp2, tmp3, mean, i;
   Word32 MaxCorr, exp, exp2, coeff;
   Word16 hi, lo;

   HfIsf[M16k - 1] = HfIsf[M - 1];

   /* Difference vector */
   for(i = 1; i < M - 1; i++)
   {
      IsfDiff[i - 1] = HfIsf[i] - HfIsf[i - 1];
   }

   tmp = 0;

   /* Mean of difference vector */
   for(i = 3; i < (M - 1); i++)
   {
      tmp = tmp + (IsfDiff[i - 1] * INV_LENGTH);
   }

   mean = (tmp + 0x4000) >> 15;
   IsfCorr[0] = 0;
   IsfCorr[1] = 0;
   IsfCorr[2] = 0;
   tmp = 0;

   for(i = 0; i < M - 2; i++)
   {
      if(IsfDiff[i] > tmp)
      {
         tmp = IsfDiff[i];
      }
   }

   exp = D_UTIL_norm_s((Word16)tmp);

   for(i = 0; i < M - 2; i++)
   {
      IsfDiff[i] = IsfDiff[i] << exp;
   }

   mean = mean << exp;

   for(i = 7; i < M - 2; i++)
   {
      tmp2 = IsfDiff[i] - mean;
      tmp3 = IsfDiff[i - 2] - mean;
      tmp = (tmp2 * tmp3) << 1;
      D_UTIL_l_extract(tmp, &hi, &lo);
      tmp = D_UTIL_mpy_32(hi, lo, hi, lo);
      IsfCorr[0] = (IsfCorr[0] + tmp);
   }

   for(i = 7; i < M - 2; i++)
   {
      tmp2 = IsfDiff[i] - mean;
      tmp3 = IsfDiff[i - 3] - mean;
      tmp = (tmp2 * tmp3) << 1;
      D_UTIL_l_extract(tmp, &hi, &lo);
      tmp = D_UTIL_mpy_32(hi, lo, hi, lo);
      IsfCorr[1] = (IsfCorr[1] + tmp);
   }

   for(i = 7; i < M - 2; i++)
   {
      tmp2 = IsfDiff[i] - mean;
      tmp3 = IsfDiff[i - 4] - mean;
      tmp = (tmp2 * tmp3) << 1;
      D_UTIL_l_extract(tmp, &hi, &lo);
      tmp = D_UTIL_mpy_32(hi, lo, hi, lo);
      IsfCorr[2] = (IsfCorr[2] + tmp);
   }

   if(IsfCorr[0] > IsfCorr[1])
   {
      MaxCorr = 0;
   }
   else
   {
      MaxCorr = 1;
   }

   if(IsfCorr[2] > IsfCorr[MaxCorr])
   {
      MaxCorr = 2;
   }

   MaxCorr = MaxCorr + 1;   /* Maximum correlation of difference vector */

   for(i = M - 1; i < (M16k - 1); i++)
   {
      tmp = (HfIsf[i - 1 - MaxCorr] - HfIsf[i - 2 - MaxCorr]);
      HfIsf[i] = (Word16)(HfIsf[i - 1] + tmp);
   }

   /* tmp=7965+(HfIsf[2]-HfIsf[3]-HfIsf[4])/6; */
   tmp = HfIsf[4] + HfIsf[3];
   tmp = HfIsf[2] - tmp;
   tmp = (tmp * 5461) >> 15;
   tmp = tmp + 20390;

   if(tmp > 19456)
   {   /* Maximum value of ISF should be at most 7600 Hz */
      tmp = 19456;
   }

   tmp = tmp - HfIsf[M - 2];
   tmp2 = HfIsf[M16k - 2] - HfIsf[M - 2];
   exp2 = D_UTIL_norm_s((Word16)tmp2);
   exp = D_UTIL_norm_s((Word16)tmp);
   exp = exp - 1;
   tmp = tmp << exp;
   tmp2 = tmp2 << exp2;
   coeff = (tmp << 15) / tmp2;   /* Coefficient for stretching the ISF vector */
   exp = exp2 - exp;

   if(exp >= 0)
   {
      for(i = M - 1; i < M16k - 1; i++)
      {
         tmp = ((HfIsf[i] - HfIsf[i - 1]) * coeff) >> 15;
         IsfDiff[i - (M - 1)] = tmp << exp;
      }
   }
   else
   {
      exp = 15 - exp;

      for(i = M - 1; i < M16k - 1; i++)
      {
         IsfDiff[i - (M - 1)] = ((HfIsf[i] - HfIsf[i - 1]) * coeff) >> exp;
      }
   }

   for(i = M; i < (M16k - 1); i++)
   {
      /* The difference between ISF(n) and ISF(n-2) should be at least 500 Hz */
      tmp = ((IsfDiff[i - (M - 1)] + IsfDiff[i - M]) - 1280);

      if(tmp < 0)
      {
         if(IsfDiff[i - (M - 1)] > IsfDiff[i - M])
         {
            IsfDiff[i - M] = (1280 - IsfDiff[i - (M - 1)]);
         }
         else
         {
            IsfDiff[i - (M - 1)] = (1280 - IsfDiff[i - M]);
         }
      }
   }

   for(i = M - 1; i < M16k - 1; i++)
   {
      HfIsf[i] = (Word16)(HfIsf[i - 1] + IsfDiff[i - (M - 1)]);
   }

   for(i = 0; i < M16k - 1; i++)
   {
      HfIsf[i] = (Word16)((HfIsf[i] * 13107) >> 14);
      /* Scale the ISF vector correctly for 16000 kHz */
   }

   D_LPC_isf_isp_conversion(HfIsf, HfIsf, M16k);

   return;
}
