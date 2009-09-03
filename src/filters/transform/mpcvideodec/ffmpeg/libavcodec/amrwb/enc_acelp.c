/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#include <math.h>
#include <memory.h>
#include <float.h>

#include "typedef.h"
#include "enc_util.h"

#define L_SUBFR         64
#define NB_PULSE_MAX    24
#define NPMAXPT ((NB_PULSE_MAX + 4 - 1) / 4)
#define NB_QUA_GAIN7B   128   /* Number of quantization level */
#define MODE_23k        7

extern const UWord8 E_ROM_tipos[];
extern const Float32 E_ROM_qua_gain6b[];
extern const Float32 E_ROM_qua_gain7b[];

/*
 * E_ACELP_Gain2_Q_init
 *
 * Parameters:
 *    mem           O: static memory
 *
 * Function:
 *    Initializes state memory
 *
 * Returns:
 *    void
 */
void E_ACELP_Gain2_Q_init(Word16 *mem)
{
   Word32 i;

   /* 2nd order quantizer energy predictor */
   for (i = 0; i < 4; i++)
   {
      mem[i] = -14336;   /* past_qua_en[i] */
   }

   return;
}

/*
 * E_ACELP_xy1_corr
 *
 * Parameters:
 *    xn          I: target signal
 *    y1          I: filtered adaptive codebook excitation
 *    g_coeff     O: correlations <y1,y1>  and -2<xn,y1>
 *
 * Function:
 *    Find the correlations between the target xn[] and the filtered adaptive
 *    codebook excitation y1[]. ( <y1,y1>  and -2<xn,y1> )
 *    Subframe size = L_SUBFR
 *
 * Returns:
 *    pitch gain  (0 ... 1.2F) (Q14)
 */
Float32 E_ACELP_xy1_corr(Float32 xn[], Float32 y1[], Float32 g_corr[])
{
   Float32 gain;
   Float32 t0, t1;
   Word32 i;

   t0 = xn[0] * y1[0];
   t1 = y1[0] * y1[0];

   for (i = 1; i < L_SUBFR; i += 7)
   {
      t0 += xn[i] * y1[i];
      t1 += y1[i] * y1[i];
      t0 += xn[i + 1] * y1[i + 1];
      t1 += y1[i + 1] * y1[i + 1];
      t0 += xn[i + 2] * y1[i + 2];
      t1 += y1[i + 2] * y1[i + 2];
      t0 += xn[i + 3] * y1[i + 3];
      t1 += y1[i + 3] * y1[i + 3];
      t0 += xn[i + 4] * y1[i + 4];
      t1 += y1[i + 4] * y1[i + 4];
      t0 += xn[i + 5] * y1[i + 5];
      t1 += y1[i + 5] * y1[i + 5];
      t0 += xn[i + 6] * y1[i + 6];
      t1 += y1[i + 6] * y1[i + 6];
   }

   g_corr[0] = t1;
   g_corr[1] = -2.0F * t0 + 0.01F;

   /* find pitch gain and bound it by [0,1.2F] */

   if (t1)
   {
      gain = t0 / t1;
   }
   else
   {
      gain = 1.0F;
   }

   if (gain < 0.0)
   {
      gain = 0.0;
   }
   else if (gain > 1.2F)
   {
      gain = 1.2F;
   }

   return gain;
}

/*
 * E_ACELP_xy2_corr
 *
 * Parameters:
 *    xn          I: target signal
 *    y1          I: filtered adaptive codebook excitation
 *    y2          I: filtered fixed codebook excitation
 *    g_corr      O: correlations <y2,y2>, -2<xn,y2>, 2<y1,y2>
 *    L_subfr     I: subframe size
 *
 * Function:
 *    Find the correlations between the target xn[], the filtered adaptive
 *    codebook exc. y1[], and the filtered fixed codebook innovation y2[].
 *    ( <y2,y2> , -2<xn,y2> and 2<y1,y2> )
 *    Subrame size = L_SUBFR
 *
 * Returns:
 *    pitch gain  (0 ... 1.2F)
 */
void E_ACELP_xy2_corr(Float32 xn[], Float32 y1[], Float32 y2[],
                      Float32 g_corr[])
{
   Float32 temp1, temp2, temp3;
   Word32 i;

   temp1 = 0.01F + y2[0] * y2[0];
   temp2 = 0.01F + xn[0] * y2[0];
   temp3 = 0.01F + y1[0] * y2[0];
   temp1 += y2[1] * y2[1];
   temp2 += xn[1] * y2[1];
   temp3 += y1[1] * y2[1];
   temp1 += y2[2] * y2[2];
   temp2 += xn[2] * y2[2];
   temp3 += y1[2] * y2[2];
   temp1 += y2[3] * y2[3];
   temp2 += xn[3] * y2[3];
   temp3 += y1[3] * y2[3];

   for (i = 4; i < L_SUBFR; i += 6)
   {
      temp1 += y2[i] * y2[i];
      temp2 += xn[i] * y2[i];
      temp3 += y1[i] * y2[i];
      temp1 += y2[i + 1] * y2[i + 1];
      temp2 += xn[i + 1] * y2[i + 1];
      temp3 += y1[i + 1] * y2[i + 1];
      temp1 += y2[i + 2] * y2[i + 2];
      temp2 += xn[i + 2] * y2[i + 2];
      temp3 += y1[i + 2] * y2[i + 2];
      temp1 += y2[i + 3] * y2[i + 3];
      temp2 += xn[i + 3] * y2[i + 3];
      temp3 += y1[i + 3] * y2[i + 3];
      temp1 += y2[i + 4] * y2[i + 4];
      temp2 += xn[i + 4] * y2[i + 4];
      temp3 += y1[i + 4] * y2[i + 4];
      temp1 += y2[i + 5] * y2[i + 5];
      temp2 += xn[i + 5] * y2[i + 5];
      temp3 += y1[i + 5] * y2[i + 5];
   }

   g_corr[2] = temp1;
   g_corr[3] = -2.0F * temp2;
   g_corr[4] = 2.0F * temp3;

   return;
}

/*
 * E_ACELP_xh_corr
 *
 * Parameters:
 *    h           I: impulse response (of weighted synthesis filter) (Q12)
 *    x           I: target signal (Q0)
 *    y           O: correlation between x[] and h[]
 *
 * Function:
 *    Compute the correlation between the target signal and the impulse
 *    response of the weighted synthesis filter.
 *
 *           y[i]=sum(j=i,l-1) x[j]*h[j-i], i=0,l-1
 *
 *    Vector size is L_SUBFR
 *
 * Returns:
 *    void
 */
void E_ACELP_xh_corr(Float32 *x, Float32 *y, Float32 *h)
{
   Word32 i, j;
   Float32 s;

   for (i = 0; i < L_SUBFR; i++)
   {
      s = 0.0F;

      for (j = i; j < L_SUBFR ; j++)
      {
         s += x[j] * h[j - i];
      }

      y[i] = s;
   }

   return;
}

/*
 * E_ACELP_codebook_target_update
 *
 * Parameters:
 *    x           I: old target (for pitch search) (Q0)
 *    x2          O: new target (for codebook search) (Q0)
 *    y           I: filtered adaptive codebook vector (Q0)
 *    gain        I: adaptive codebook gain (Q14)
 *
 * Function:
 *    Update the target vector for codebook search.
 *    Subframe size = L_SUBFR
 * Returns:
 *    void
 */
void E_ACELP_codebook_target_update(Float32 *x, Float32 *x2, Float32 *y,
                                    Float32 gain)
{
   Word32 i;

   for (i = 0; i < L_SUBFR; i ++)
   {
      x2[i] = x[i] - gain * y[i];
   }

}

/*
 * E_ACELP_h_vec_corr?
 *
 * Parameters:
 *    h              I: scaled impulse response
 *    vec            I: vector to correlate with h[]
 *    track          I: track to use
 *    sign           I: sign vector
 *    rrixix         I: correlation of h[x] with h[x]
 *    cor            O: result of correlation (16 elements)
 *
 * Function:
 *    Calculate the correlations of h[] with vec[] for the specified track
 *
 * Returns:
 *    void
 */
static void E_ACELP_h_vec_corr1(Float32 h[], Float32 vec[], UWord8 track,
                                Float32 sign[], Float32 (*rrixix)[16],
                                Float32 cor[], Word32 dn2_pos[],
                                Word32 nb_pulse)
{
   Word32 i, j, dn;
   Word32 *dn2;
   Float32 *p0;
   Float32 s;

   dn2 = &dn2_pos[track * 8];
   p0 = rrixix[track];

   for (i = 0; i < nb_pulse; i++)
   {
      dn = dn2[i];
      s = 0.0F;
      for (j = 0; j < (L_SUBFR - dn); j++)
      {
         s += h[j] * vec[dn + j];
      }

      cor[dn >> 2] = sign[dn] * s + p0[dn >> 2];
   }

   return;
}

static void E_ACELP_h_vec_corr2(Float32 h[], Float32 vec[], UWord8 track,
                                Float32 sign[], Float32 (*rrixix)[16],
                                Float32 cor[])
{
   Word32 i, j;
   Float32 *p0;
   Float32 s;

   p0 = rrixix[track];
   for (i = 0; i < 16; i++)
   {
      s = 0.0F;
      for (j = 0; j < L_SUBFR - track; j++)
      {
         s += h[j] * vec[track + j];
      }

      cor[i] = s * sign[track] + p0[i];
      track += 4;
   }

   return;
}

/*
 * E_ACELP_2pulse_search
 *
 * Parameters:
 *    nb_pos_ix      I: nb of pos for pulse 1 (1..8)
 *    track_x        I: track of pulse 1
 *    track_y        I: track of pulse 2
 *    ps           I/O: correlation of all fixed pulses
 *    alp          I/O: energy of all fixed pulses
 *    ix             O: position of pulse 1
 *    iy             O: position of pulse 2
 *    dn             I: corr. between target and h[]
 *    dn2            I: vector of selected positions
 *    cor_x          I: corr. of pulse 1 with fixed pulses
 *    cor_y          I: corr. of pulse 2 with fixed pulses
 *    rrixiy         I: corr. of pulse 1 with pulse 2
 *
 * Function:
 *    Find the best positions of 2 pulses in a subframe
 *
 * Returns:
 *    void
 */
static void E_ACELP_2pulse_search(Word32 nb_pos_ix, UWord8 track_x,
                                  UWord8 track_y, Float32 *ps, Float32 *alp,
                                  Word32 *ix, Word32 *iy, Float32 dn[],
                                  Word32 *dn2, Float32 cor_x[],
                                  Float32 cor_y[], Float32 (*rrixiy)[256])
{
   Word32 x, x2, y, x_save = 0, y_save = 0, i, *pos_x;
   Float32 ps0, alp0;
   Float32 ps1, ps2, sq, sqk;
   Float32 alp1, alp2, alpk;
   Float32 *p1, *p2;
   Float32 s;

   /* eight dn2 max positions per track */
   pos_x = &dn2[track_x << 3];
   /* save these to limit memory searches */
   ps0 = *ps;
   alp0 = *alp;

   sqk = -1.0F;
   alpk = 1.0F;

   /* loop track 1 */
   for (i = 0; i < nb_pos_ix; i++)
   {
      x = pos_x[i];
      x2 = x >> 2;
      /* dn[x] has only nb_pos_ix positions saved */
      ps1 = ps0 + dn[x];
      alp1 = alp0 + cor_x[x2];
      p1 = cor_y;
      p2 = &rrixiy[track_x][x2 << 4];

      for (y = track_y; y < L_SUBFR; y += 4)
      {
         ps2 = ps1 + dn[y];
         alp2 = alp1 + (*p1++) + (*p2++);

         sq = ps2 * ps2;

         s = (alpk * sq) - (sqk * alp2);

         if (s > 0.0F)
         {
            sqk = sq;
            alpk = alp2;
            y_save = y;
            x_save = x;
         }
      }
   }

   *ps = ps0 + dn[x_save] + dn[y_save];
   *alp = alpk;
   *ix = x_save;
   *iy = y_save;

   return;
}

/*
 * E_ACELP_quant_1p_N1
 *
 * Parameters:
 *    pos      I: position of the pulse
 *    N        I: number of bits for position
 *
 * Function:
 *    Quantization of 1 pulse with N+1 bits
 *
 * Returns:
 *    return N+1 bits
 */
static Word32 E_ACELP_quant_1p_N1(Word32 pos, Word32 N)
{
   Word32 mask;
   Word32 index;

   mask = ((1<<N)-1);
   /*
    * Quantization of 1 pulse with N+1 bits:
    */
   index = (pos & mask);
   if ((pos & 16) != 0)
   {
      index += 1 << N;
   }

   return(index);
}

/*
 * E_ACELP_quant_2p_2N1
 *
 * Parameters:
 *    pos1     I: position of the pulse 1
 *    pos2     I: position of the pulse 2
 *    N        I: number of bits for position
 *
 * Function:
 *    Quantization of 2 pulses with 2*N+1 bits
 *
 * Returns:
 *    (2*N)+1 bits
 */
static Word32 E_ACELP_quant_2p_2N1(Word32 pos1, Word32 pos2, Word32 N)
{
   Word32 mask;
   Word32 index;

   mask = ((1 << N) - 1);
   /*
    * Quantization of 2 pulses with 2*N+1 bits:
    */
   if (((pos2 ^ pos1) & 16) == 0)
   {
      /* sign of 1st pulse == sign of 2th pulse */
      if ((pos1 - pos2) <= 0)
      {
         index = ((pos1 & mask) << N) + (pos2 & mask);
      }
      else
      {
         index = ((pos2 & mask) << N) + (pos1 & mask);
      }
      if ((pos1 & 16) != 0)
      {
         index += 1 << (2 * N);
      }
   }
   else
   {
      /* sign of 1st pulse != sign of 2th pulse */
      if (((pos1 & mask) - (pos2 & mask)) <= 0)
      {
         index = ((pos2 & mask) << N) + (pos1 & mask);
         if ((pos2 & 16) != 0)
         {
            index += 1 << (2 * N);
         }
      }
      else
      {
         index = ((pos1 & mask) << N) + (pos2 & mask);
         if ((pos1 & 16) != 0)
         {
            index += 1 << (2 * N);
         }
      }
   }

   return(index);
}

/*
 * E_ACELP_quant_3p_3N1
 *
 * Parameters:
 *    pos1     I: position of the pulse 1
 *    pos2     I: position of the pulse 2
 *    pos3     I: position of the pulse 3
 *    N        I: number of bits for position
 *
 * Function:
 *    Quantization of 3 pulses with 3*N+1 bits
 *
 * Returns:
 *    (3*N)+1 bits
 */
static Word32 E_ACELP_quant_3p_3N1(Word32 pos1, Word32 pos2, Word32 pos3,
                                   Word32 N)
{
   Word32 nb_pos;
   Word32 index;

   nb_pos = (1 << (N - 1));
   /*
    * Quantization of 3 pulses with 3*N+1 bits:
    */
   if (((pos1 ^ pos2) & nb_pos) == 0)
   {
      index = E_ACELP_quant_2p_2N1(pos1, pos2, (N - 1));
      index += (pos1 & nb_pos) << N;
      index += E_ACELP_quant_1p_N1(pos3, N) << (2 * N);
   }
   else if (((pos1 ^ pos3) & nb_pos) == 0)
   {
      index = E_ACELP_quant_2p_2N1(pos1, pos3, (N - 1));
      index += (pos1 & nb_pos) << N;
      index += E_ACELP_quant_1p_N1(pos2, N) << (2 * N);
   }
   else
   {
      index = E_ACELP_quant_2p_2N1(pos2, pos3, (N - 1));
      index += (pos2 & nb_pos) << N;
      index += E_ACELP_quant_1p_N1(pos1, N) << (2 * N);
   }

   return(index);
}

/*
 * E_ACELP_quant_4p_4N1
 *
 * Parameters:
 *    pos1     I: position of the pulse 1
 *    pos2     I: position of the pulse 2
 *    pos3     I: position of the pulse 3
 *    pos4     I: position of the pulse 4
 *    N        I: number of bits for position
 *
 * Function:
 *    Quantization of 4 pulses with 4*N+1 bits
 *
 * Returns:
 *    (4*N)+1 bits
 */
static Word32 E_ACELP_quant_4p_4N1(Word32 pos1, Word32 pos2, Word32 pos3,
                                   Word32 pos4, Word32 N)
{
   Word32 nb_pos;
   Word32 index;

   nb_pos = (1 << (N - 1));
   /*
    * Quantization of 4 pulses with 4*N+1 bits:
    */
   if (((pos1 ^ pos2) & nb_pos) == 0)
   {
      index = E_ACELP_quant_2p_2N1(pos1, pos2, (N - 1));
      index += (pos1 & nb_pos) << N;
      index += E_ACELP_quant_2p_2N1(pos3, pos4, N) << (2 * N);
   }
   else if (((pos1 ^ pos3) & nb_pos) == 0)
   {
      index = E_ACELP_quant_2p_2N1(pos1, pos3, (N - 1));
      index += (pos1 & nb_pos) << N;
      index += E_ACELP_quant_2p_2N1(pos2, pos4, N) << (2 * N);
   }
   else
   {
      index = E_ACELP_quant_2p_2N1(pos2, pos3, (N - 1));
      index += (pos2 & nb_pos) << N;
      index += E_ACELP_quant_2p_2N1(pos1, pos4, N) << (2 * N);
   }

   return(index);
}

/*
 * E_ACELP_quant_4p_4N
 *
 * Parameters:
 *    pos      I: position of the pulse 1..4
 *    N        I: number of bits for position
 *
 * Function:
 *    Quantization of 4 pulses with 4*N bits
 *
 * Returns:
 *    4*N bits
 */
static Word32 E_ACELP_quant_4p_4N(Word32 pos[], Word32 N)
{
   Word32 i, j, k, nb_pos, n_1;
   Word32 posA[4], posB[4];
   Word32 index=0;

   n_1 = N - 1;
   nb_pos = (1 << n_1);

   i = 0;
   j = 0;
   for (k = 0; k < 4; k++)
   {
      if ((pos[k] & nb_pos) == 0)
      {
         posA[i++] = pos[k];
      }
      else
      {
         posB[j++] = pos[k];
      }
   }

   switch (i)
   {
   case 0:
      index = 1 << ((4 * N) - 3);
      index += E_ACELP_quant_4p_4N1(posB[0], posB[1], posB[2], posB[3], n_1);
      break;
   case 1:
      index = E_ACELP_quant_1p_N1(posA[0], n_1) << (( 3 * n_1) + 1);
      index += E_ACELP_quant_3p_3N1(posB[0], posB[1], posB[2], n_1);
      break;
   case 2:
      index = E_ACELP_quant_2p_2N1(posA[0], posA[1], n_1) << (( 2 * n_1) + 1);
      index += E_ACELP_quant_2p_2N1(posB[0], posB[1], n_1);
      break;
   case 3:
      index = E_ACELP_quant_3p_3N1(posA[0], posA[1], posA[2], n_1) << N;
      index += E_ACELP_quant_1p_N1(posB[0], n_1);
      break;
   case 4:
      index = E_ACELP_quant_4p_4N1(posA[0], posA[1], posA[2], posA[3], n_1);
      break;
   }
   index += (i & 3) << ((4 * N) - 2);

   return(index);
}

/*
 * E_ACELP_quant_5p_5N
 *
 * Parameters:
 *    pos      I: position of the pulse 1..5
 *    N        I: number of bits for position
 *
 * Function:
 *    Quantization of 5 pulses with 5*N bits
 *
 * Returns:
 *    5*N bits
 */
static Word32 E_ACELP_quant_5p_5N(Word32 pos[], Word32 N)
{
   Word32 i,j,k,nb_pos,n_1;
   Word32 posA[5], posB[5];
   Word32 index=0;

   n_1 = N-1;
   nb_pos = (1 << n_1);

   i = 0;
   j = 0;
   for (k = 0; k < 5; k++)
   {
      if ((pos[k] & nb_pos) == 0)
      {
         posA[i++] = pos[k];
      }
      else
      {
         posB[j++] = pos[k];
      }
   }

   switch (i)
   {
   case 0:
      index = 1 << ((5 * N) - 1);
      index +=
         E_ACELP_quant_3p_3N1(posB[0], posB[1], posB[2], n_1) << ((2 * N) + 1);
      index += E_ACELP_quant_2p_2N1(posB[3], posB[4], N);
      break;
   case 1:
      index = 1 << ((5 * N) - 1);
      index +=
         E_ACELP_quant_3p_3N1(posB[0], posB[1], posB[2], n_1) << ((2 * N) + 1);
      index += E_ACELP_quant_2p_2N1(posB[3], posA[0], N);
      break;
   case 2:
      index = 1 << ((5 * N) - 1);
      index +=
         E_ACELP_quant_3p_3N1(posB[0], posB[1], posB[2], n_1) << ((2 * N) + 1);
      index += E_ACELP_quant_2p_2N1(posA[0], posA[1], N);
      break;
   case 3:
      index =
         E_ACELP_quant_3p_3N1(posA[0], posA[1], posA[2], n_1) << ((2 * N) + 1);
      index += E_ACELP_quant_2p_2N1(posB[0], posB[1], N);
      break;
   case 4:
      index =
         E_ACELP_quant_3p_3N1(posA[0], posA[1], posA[2], n_1) << ((2 * N) + 1);
      index += E_ACELP_quant_2p_2N1(posA[3], posB[0], N);
      break;
   case 5:
      index =
         E_ACELP_quant_3p_3N1(posA[0], posA[1], posA[2], n_1) << ((2 * N) + 1);
      index += E_ACELP_quant_2p_2N1(posA[3], posA[4], N);
      break;
   }

   return(index);
}

/*
* E_ACELP_quant_6p_6N_2
*
* Parameters:
*    pos      I: position of the pulse 1..6
*    N        I: number of bits for position
*
* Function:
*    Quantization of 6 pulses with 6*N-2 bits
*
* Returns:
*    (6*N)-2 bits
*/
static Word32 E_ACELP_quant_6p_6N_2(Word32 pos[], Word32 N)
{
   Word32 i, j, k, nb_pos, n_1;
   Word32 posA[6], posB[6];
   Word32 index=0;

   n_1 = N - 1;
   nb_pos = 1 << n_1;

   i = 0;
   j = 0;
   for (k = 0; k < 6; k++)
   {
      if ((pos[k] & nb_pos) == 0)
      {
         posA[i++] = pos[k];
      }
      else
      {
         posB[j++] = pos[k];
      }
   }

   switch (i)
   {
   case 0:
      index = 1 << ((6 * N) - 5);
      index += E_ACELP_quant_5p_5N(posB, n_1) << N;
      index += E_ACELP_quant_1p_N1(posB[5], n_1);
      break;
   case 1:
      index = 1 << ((6 * N) - 5);
      index += E_ACELP_quant_5p_5N(posB, n_1) << N;
      index += E_ACELP_quant_1p_N1(posA[0], n_1);
      break;
   case 2:
      index = 1 << ((6 * N) - 5);
      index += E_ACELP_quant_4p_4N(posB, n_1) << ((2 * n_1) + 1);
      index += E_ACELP_quant_2p_2N1(posA[0], posA[1], n_1);
      break;
   case 3:
      index = E_ACELP_quant_3p_3N1(posA[0], posA[1], posA[2], n_1)
         << ((3 * n_1) + 1);
      index += E_ACELP_quant_3p_3N1(posB[0], posB[1], posB[2], n_1);
      break;
   case 4:
      i = 2;
      index = E_ACELP_quant_4p_4N(posA, n_1) << ((2 * n_1) + 1);
      index += E_ACELP_quant_2p_2N1(posB[0], posB[1], n_1);
      break;
   case 5:
      i = 1;
      index = E_ACELP_quant_5p_5N(posA, n_1) << N;
      index += E_ACELP_quant_1p_N1(posB[0], n_1);
      break;
   case 6:
      i = 0;
      index = E_ACELP_quant_5p_5N(posA, n_1) << N;
      index += E_ACELP_quant_1p_N1(posA[5], n_1);
      break;
   }

   index += (i & 3) << ((6 * N) - 4);

   return(index);
}

/*
 * E_ACELP_2t
 *
 * Parameters:
 *    dn          I: corr. between target and h[].
 *    cn          I: residual after Word32 term prediction
 *    H           I: impulse response of weighted synthesis filter (Q12)
 *    code        O: algebraic (fixed) codebook excitation (Q9)
 *    y           O: filtered fixed codebook excitation (Q9)
 *    index       O: index (12): 5 + 1 + 5 + 1 = 11 bits.
 *
 * Function:
 *    12 bits algebraic codebook.
 *    2 tracks x 32 positions per track = 64 samples.
 *
 *    12 bits --> 2 pulses in a frame of 64 samples.
 *
 *    All pulses can have two (2) possible amplitudes: +1 or -1.
 *    Each pulse can have 32 possible positions.
 *
 * Returns:
 *    void
 */
void E_ACELP_2t(Float32 dn[], Float32 cn[], Float32 H[],
                Word16 code[], Float32 y[], Word32 *index)
{
   Word32 i, j, k, i0, i1, ix, iy, pos = 0, pos2;

   Float32 ps, psk, ps1, ps2, alpk, alp1, alp2, sq;
   Float32 s, cor, alp, val;
   Float32 *p0, *p1, *p2, *psign;
   Float32 *h, *h_inv, *ptr_h1, *ptr_h2, *ptr_hf;
   static Float32 sign[L_SUBFR], vec[L_SUBFR], dn2[L_SUBFR];
   static Float32 h_buf[4 * L_SUBFR];
   static Float32 rrixix[2][32];
   static Float32 rrixiy[1024];

   /*
    * Find sign for each pulse position.
    */
   alp = 2.0;

   /* calculate energy for normalization of cn[] and dn[] */
   val = 1.0;
   cor = 1.0;

   for(i = 0; i < L_SUBFR; i++)
   {
      val += cn[i] * cn[i];
   }

   for(i = 0; i < L_SUBFR; i++)
   {
      cor += dn[i] * dn[i];
   }

   s = (Float32)sqrt(cor / val);

   for(i = 0; i < 2; i++)
   {
      for(j = i; j < L_SUBFR; j += 2)
      {
         val = dn[j];
         cor = (s * cn[j]) + (alp * val);

         if(cor >= 0.0)
         {
            sign[j] = 1.0;
            vec[j] = -1.0;
         }
         else
         {
            sign[j] = -1.0;
            vec[j] = 1.0;
            val = -val;
            cor = -cor;
         }

         dn[j] = val;   /* modify dn[] according to the fixed sign */
         dn2[j] = cor;
      }
   }

   /*
    * Select 16 position per track according to dn2[].
    */
   for(i = 0; i < 2; i++)
   {
      for(k = 0; k < 16; k++)
      {
         ps = -1;

         for(j = i; j < L_SUBFR; j += 2)
         {
            if(dn2[j] > ps)
            {
               ps = dn2[j];
               pos = j;
            }
         }
         dn2[pos] = (Float32)k - 16;   /* dn2 < 0 when position is selected */
      }
   }

   /*
    * Compute h_inv[i].
    */
   h = h_buf;
   h_inv = h_buf + (2 * L_SUBFR);

   for(i = 0; i < L_SUBFR; i++)
   {
      *h++ = 0.0F;
      *h_inv++ = 0.0F;
   }

   for(i = 0; i < L_SUBFR; i++)
   {
      h[i] = H[i];
      h_inv[i] = -h[i];
   }

   /*
    * Compute rrixix[][] needed for the codebook search.
    */
   /* Init pointers to last position of rrixix[] */
   p0 = &rrixix[0][32 - 1];
   p1 = &rrixix[1][32 - 1];
   ptr_h1 = h;
   cor = 0.0F;

   for(i = 0; i < 32; i++)
   {
      cor += (*ptr_h1) * (*ptr_h1);
      ptr_h1++;
      *p1-- = cor * 0.5F;
      cor += (*ptr_h1) * (*ptr_h1);
      ptr_h1++;
      *p0-- = cor * 0.5F;
   }

   /*
    * Compute rrixiy[][] needed for the codebook search.
    */
   pos = 1024 - 1;
   pos2 = 1024 - 2;
   ptr_hf = h + 1;

   for(k = 0; k < 32; k++)
   {
      p1 = &rrixiy[pos];
      p0 = &rrixiy[pos2];
      cor = 0.0;
      ptr_h1 = h;
      ptr_h2 = ptr_hf;

      for(i = k + 1; i < 32; i++)
      {
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p1 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p0 = cor;
         p1 -= (32 + 1);
         p0 -= (32 + 1);
      }

      cor += (*ptr_h1) * (*ptr_h2);
      ptr_h1++;
      ptr_h2++;
      *p1 = cor;
      pos -= 32;
      pos2--;
      ptr_hf += 2;
   }

   /*
    * Modification of rrixiy[][] to take signs into account.
    */
   p0 = rrixiy;

   for(i = 0; i < L_SUBFR; i += 2)
   {
      psign = sign;

      if(psign[i] < 0.0)
      {
         psign = vec;
      }

      for(j = 1; j < L_SUBFR; j += 2)
      {
         *p0 = *p0 * psign[j];
         p0++;
      }
   }

   /*
    * search 2 pulses:
    * ---------------
    * 32 pos x 32 pos = 1024 tests (all combinations are tested)
    */
   p0 = rrixix[0];
   p1 = rrixix[1];
   p2 = rrixiy;
   psk = -1.0;
   alpk = 1.0;
   ix = 0;
   iy = 1;

   for(i0 = 0; i0 < L_SUBFR; i0 += 2)
   {
      ps1 = dn[i0];
      alp1 = (*p0++);
      pos = -1;

      for(i1 = 1; i1 < L_SUBFR; i1 += 2)
      {
         ps2 = ps1 + dn[i1];
         alp2 = alp1 + (*p1++) + (*p2++);
         sq = ps2 * ps2;
         s = (alpk * sq) - (psk * alp2);

         if(s > 0.0)
         {
            psk = sq;
            alpk = alp2;
            pos = i1;
         }
      }
      p1 -= 32;

      if(pos >= 0)
      {
         ix = i0;
         iy = pos;
      }
   }

   /*
    * Build the codeword, the filtered codeword and index of codevector.
    */
   memset(code, 0, L_SUBFR * sizeof(Word16));

   i0 = ix / 2;   /* pos of pulse 1 (0..31) */
   i1 = iy / 2;   /* pos of pulse 2 (0..31) */

   if(sign[ix] > 0.0)
   {
      code[ix] = 512;
      p0 = h - ix;
   }
   else
   {
      code[ix] = -512;
      i0 += 32;
      p0 = h_inv - ix;
   }

   if(sign[iy] > 0.0)
   {
      code[iy] = 512;
      p1 = h - iy;
   }
   else
   {
      code[iy] = -512;
      i1 += 32;
      p1 = h_inv - iy;
   }

   *index = (i0 << 6) + i1;

   for(i = 0; i < L_SUBFR; i++)
   {
      y[i] = (*p0++) + (*p1++);
   }

   return;
}

/*
 * E_ACELP_4t
 *
 * Parameters:
 *    dn          I: corr. between target and h[].
 *    cn          I: residual after Word32 term prediction
 *    H           I: impulse response of weighted synthesis filter (Q12)
 *    code        O: algebraic (fixed) codebook excitation (Q9)
 *    y           O: filtered fixed codebook excitation (Q9)
 *    nbbits      I: 20, 36, 44, 52, 64, 72 or 88 bits
 *    mode        I: speech mode
 *    _index      O: index
 *
 * Function:
 *    20, 36, 44, 52, 64, 72, 88 bits algebraic codebook.
 *    4 tracks x 16 positions per track = 64 samples.
 *
 *    20 bits 5 + 5 + 5 + 5 --> 4 pulses in a frame of 64 samples.
 *    36 bits 9 + 9 + 9 + 9 --> 8 pulses in a frame of 64 samples.
 *    44 bits 13 + 9 + 13 + 9 --> 10 pulses in a frame of 64 samples.
 *    52 bits 13 + 13 + 13 + 13 --> 12 pulses in a frame of 64 samples.
 *    64 bits 2 + 2 + 2 + 2 + 14 + 14 + 14 + 14 -->
 *                                  16 pulses in a frame of 64 samples.
 *    72 bits 10 + 2 + 10 + 2 + 10 + 14 + 10 + 14 -->
 *                                  18 pulses in a frame of 64 samples.
 *    88 bits 11 + 11 + 11 + 11 + 11 + 11 + 11 + 11 -->
 *                                  24 pulses in a frame of 64 samples.
 *
 *    All pulses can have two (2) possible amplitudes: +1 or -1.
 *    Each pulse can sixteen (16) possible positions.
 *
 * Returns:
 *    void
 */
void E_ACELP_4t(Float32 dn[], Float32 cn[], Float32 H[], Word16 code[],
                Float32 y[], Word32 nbbits, Word16 mode, Word32 _index[])
{
   Float32 sign[L_SUBFR], vec[L_SUBFR];
   Float32 cor_x[16], cor_y[16], h_buf[4 * L_SUBFR];
   Float32 rrixix[4][16];
   Float32 rrixiy[4][256];
   Float32 dn2[L_SUBFR];
   Word32 ind[NPMAXPT*4];
   Word32 codvec[NB_PULSE_MAX];
   Word32 nbpos[10];
   Word32 pos_max[4];
   Word32 dn2_pos[8 * 4];
   UWord8 ipos[NB_PULSE_MAX];
   Word32 i, j, k, st, pos = 0, index, track, nb_pulse = 0, nbiter = 4;
   Word32 L_index;
   Float32 psk, ps, alpk, alp = 0.0F;
   Float32 val;
   Float32 s, cor;
   Float32 *p0, *p1, *p2, *p3, *psign;
   Float32 *h, *h_inv, *ptr_h1, *ptr_h2, *ptr_hf;

   switch (nbbits)
   {
   case 20:          /* 20 bits, 4 pulses, 4 tracks   */
      nbiter = 4;    /* 4x16x16=1024 loop             */
      alp = 2.0;
      nb_pulse = 4;
      nbpos[0] = 4;
      nbpos[1] = 8;
      break;
   case 36:          /* 36 bits, 8 pulses, 4 tracks   */
      nbiter = 4;    /* 4x20x16=1280 loop             */
      alp = 1.0;     /* coeff for sign setting        */
      nb_pulse = 8;
      nbpos[0] = 4;
      nbpos[1] = 8;
      nbpos[2] = 8;
      break;
   case 44:          /* 44 bits, 10 pulses, 4 tracks  */
      nbiter = 4;    /* 4x26x16=1664 loop             */
      alp = 1.0;
      nb_pulse = 10;
      nbpos[0] = 4;
      nbpos[1] = 6;
      nbpos[2] = 8;
      nbpos[3] = 8;
      break;
   case 52:          /* 52 bits, 12 pulses, 4 tracks  */
      nbiter = 4;    /* 4x26x16=1664 loop             */
      alp = 1.0;
      nb_pulse = 12;
      nbpos[0] = 4;
      nbpos[1] = 6;
      nbpos[2] = 8;
      nbpos[3] = 8;
      break;
   case 64:          /* 64 bits, 16 pulses, 4 tracks  */
      nbiter = 3;    /* 3x36x16=1728 loop             */
      alp = 0.8F;
      nb_pulse = 16;
      nbpos[0] = 4;
      nbpos[1] = 4;
      nbpos[2] = 6;
      nbpos[3] = 6;
      nbpos[4] = 8;
      nbpos[5] = 8;
      break;
   case 72:          /* 72 bits, 18 pulses, 4 tracks  */
      nbiter = 3;    /* 3x35x16=1680 loop             */
      alp = 0.75F;
      nb_pulse = 18;
      nbpos[0] = 2;
      nbpos[1] = 3;
      nbpos[2] = 4;
      nbpos[3] = 5;
      nbpos[4] = 6;
      nbpos[5] = 7;
      nbpos[6] = 8;
      break;
   case 88:          /* 88 bits, 24 pulses, 4 tracks  */
      if (mode > MODE_23k)
      {
         nbiter = 1;
      }
      else
      {
         nbiter = 2; /* 2x53x16=1696 loop             */
      }

      alp = 0.5;
      nb_pulse = 24;
      nbpos[0] = 2;
      nbpos[1] = 2;
      nbpos[2] = 3;
      nbpos[3] = 4;
      nbpos[4] = 5;
      nbpos[5] = 6;
      nbpos[6] = 7;
      nbpos[7] = 8;
      nbpos[8] = 8;
      nbpos[9] = 8;
      break;
   }

   /*
    * Find sign for each pulse position.
    */

   /* calculate energy for normalization of cn[] and dn[] */

   val = (cn[0] * cn[0]) + 1.0F;
   cor = (dn[0] * dn[0]) + 1.0F;

   for (i = 1; i < L_SUBFR; i += 7)
   {
      val += (cn[i] * cn[i]);
      cor += (dn[i] * dn[i]);
      val += (cn[i + 1] * cn[i + 1]);
      cor += (dn[i + 1] * dn[i + 1]);
      val += (cn[i + 2] * cn[i + 2]);
      cor += (dn[i + 2] * dn[i + 2]);
      val += (cn[i + 3] * cn[i + 3]);
      cor += (dn[i + 3] * dn[i + 3]);
      val += (cn[i + 4] * cn[i + 4]);
      cor += (dn[i + 4] * dn[i + 4]);
      val += (cn[i + 5] * cn[i + 5]);
      cor += (dn[i + 5] * dn[i + 5]);
      val += (cn[i + 6] * cn[i + 6]);
      cor += (dn[i + 6] * dn[i + 6]);
   }

   s = (Float32)sqrt(cor / val);

   for (j = 0; j < L_SUBFR; j++)
   {
      cor = (s * cn[j]) + (alp * dn[j]);

      if (cor >= 0.0F)
      {
         sign[j] = 1.0F;
         vec[j] = -1.0F;
         dn2[j] = cor;     /* dn2[] = mix of dn[] and cn[]   */
      }
      else
      {
         sign[j] = -1.0F;
         vec[j] = 1.0F;
         dn[j] = -dn[j];   /* modify dn[] according to the fixed sign */
         dn2[j] = -cor;    /* dn2[] = mix of dn[] and cn[]            */
      }
   }

   /*
    * Select 8 position per track according to dn2[].
    */
   for (i = 0; i < 4; i++)
   {
      for (k = 0; k < 8; k++)
      {
         ps = -1;

         for (j = i; j < L_SUBFR; j += 4)
         {
            if (dn2[j] > ps)
            {
               ps = dn2[j];
               pos = j;
            }
         }

         dn2[pos] = (Float32)k - 8;    /* dn2 < 0 when position is selected */
         dn2_pos[i * 8 + k] = pos;
      }

      pos_max[i] = dn2_pos[i * 8];
   }

   /*
    * Compute h_inv[i].
    */
   memset(h_buf, 0, L_SUBFR * sizeof(Float32));
   memset(h_buf + (2 * L_SUBFR), 0, L_SUBFR * sizeof(Float32));

   h = h_buf + L_SUBFR;
   h_inv = h_buf + (3 * L_SUBFR);

   memcpy(h, H, L_SUBFR * sizeof(Float32));

   h_inv[0] = -h[0];
   h_inv[1] = -h[1];
   h_inv[2] = -h[2];
   h_inv[3] = -h[3];

   for(i = 4; i < L_SUBFR; i += 6)
   {
      h_inv[i] = -h[i];
      h_inv[i + 1] = -h[i + 1];
      h_inv[i + 2] = -h[i + 2];
      h_inv[i + 3] = -h[i + 3];
      h_inv[i + 4] = -h[i + 4];
      h_inv[i + 5] = -h[i + 5];
   }

   /*
    * Compute rrixix[][] needed for the codebook search.
    */

   /* storage order --> i3i3, i2i2, i1i1, i0i0 */

   /* Init pointers to last position of rrixix[] */
   p0 = &rrixix[0][16 - 1];
   p1 = &rrixix[1][16 - 1];
   p2 = &rrixix[2][16 - 1];
   p3 = &rrixix[3][16 - 1];

   ptr_h1 = h;
   cor    = 0.0F;

   for(i = 0; i < 16; i++)
   {
      cor += (*ptr_h1) * (*ptr_h1);
      ptr_h1++;
      *p3-- = cor * 0.5F;
      cor += (*ptr_h1) * (*ptr_h1);
      ptr_h1++;
      *p2-- = cor * 0.5F;
      cor += (*ptr_h1) * (*ptr_h1);
      ptr_h1++;
      *p1-- = cor * 0.5F;
      cor += (*ptr_h1) * (*ptr_h1);
      ptr_h1++;
      *p0-- = cor * 0.5F;
   }


   /*
    * Compute rrixiy[][] needed for the codebook search.
    */

   /* storage order --> i2i3, i1i2, i0i1, i3i0 */

   pos = 256 - 1;
   ptr_hf = h + 1;

   for(k = 0; k < 16; k++)
   {

      p3 = &rrixiy[2][pos];
      p2 = &rrixiy[1][pos];
      p1 = &rrixiy[0][pos];
      p0 = &rrixiy[3][pos - 16];

      cor = 0.0F;
      ptr_h1 = h;
      ptr_h2 = ptr_hf;

      for(i = k + 1; i < 16; i++)
      {
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p3 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p2 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p1 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p0 = cor;

         p3 -= (16 + 1);
         p2 -= (16 + 1);
         p1 -= (16 + 1);
         p0 -= (16 + 1);
      }

      cor += (*ptr_h1) * (*ptr_h2);
      ptr_h1++;
      ptr_h2++;
      *p3 = cor;
      cor += (*ptr_h1) * (*ptr_h2);
      ptr_h1++;
      ptr_h2++;
      *p2 = cor;
      cor += (*ptr_h1) * (*ptr_h2);
      ptr_h1++;
      ptr_h2++;
      *p1 = cor;

      pos -= 16;
      ptr_hf += 4;
   }

   /* storage order --> i3i0, i2i3, i1i2, i0i1 */

   pos = 256 - 1;
   ptr_hf = h + 3;

   for(k = 0; k < 16; k++)
   {

      p3 = &rrixiy[3][pos];
      p2 = &rrixiy[2][pos - 1];
      p1 = &rrixiy[1][pos - 1];
      p0 = &rrixiy[0][pos - 1];

      cor = 0.0F;
      ptr_h1 = h;
      ptr_h2 = ptr_hf;

      for(i= k + 1; i < 16; i++ )
      {
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p3 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p2 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p1 = cor;
         cor += (*ptr_h1) * (*ptr_h2);
         ptr_h1++;
         ptr_h2++;
         *p0 = cor;

         p3 -= (16 + 1);
         p2 -= (16 + 1);
         p1 -= (16 + 1);
         p0 -= (16 + 1);
      }

      cor += (*ptr_h1) * (*ptr_h2);
      ptr_h1++;
      ptr_h2++;
      *p3 = cor;

      pos--;
      ptr_hf += 4;
   }

   /*
    * Modification of rrixiy[][] to take signs into account.
    */

   p0 = &rrixiy[0][0];

   for (k = 0; k < 4; k++)
   {
      for(i = k; i < L_SUBFR; i += 4)
      {
         psign = sign;

         if (psign[i] < 0.0F)
         {
            psign = vec;
         }

         j = (k + 1) % 4;

         p0[0] = p0[0] * psign[j];
         p0[1] = p0[1] * psign[j + 4];
         p0[2] = p0[2] * psign[j + 8];
         p0[3] = p0[3] * psign[j + 12];
         p0[4] = p0[4] * psign[j + 16];
         p0[5] = p0[5] * psign[j + 20];
         p0[6] = p0[6] * psign[j + 24];
         p0[7] = p0[7] * psign[j + 28];
         p0[8] = p0[8] * psign[j + 32];
         p0[9] = p0[9] * psign[j + 36];
         p0[10] = p0[10] * psign[j + 40];
         p0[11] = p0[11] * psign[j + 44];
         p0[12] = p0[12] * psign[j + 48];
         p0[13] = p0[13] * psign[j + 52];
         p0[14] = p0[14] * psign[j + 56];
         p0[15] = p0[15] * psign[j + 60];
         p0 += 16;
      }
   }

   /*
    * Deep first search:
    * ------------------
    * 20 bits (4p):  4 iter x ((4x16)+(8x16))              = 768 tests
    * 36 bits (8p):  4 iter x ((1x1)+(4x16)+(8x16)+(8x16)) = 1280 tests
    * 52 bits (12p): 3 iter x ((1x1)+(1x1)+(4x16)+(6x16)
    *                                      +(8x16)+(8x16)) = 1248 tests
    * 64 bits (16p): 2 iter x ((1x1)+(1x1)+(4x16)+(6x16)
    *                        +(6x16)+(8x16)+(8x16)+(8x16)) = 1280 tests
    */

   psk = -1.0;
   alpk = 1.0;

   for (k = 0; k < nbiter; k++)
   {
      for (i = 0; i < nb_pulse - (nb_pulse % 3); i += 3)
      {
         ipos[i] = E_ROM_tipos[(k * 4) + i];
         ipos[i + 1] = E_ROM_tipos[(k * 4) + i + 1];
         ipos[i + 2] = E_ROM_tipos[(k * 4) + i + 2];
      }

      for (; i < nb_pulse; i ++)
      {
         ipos[i] = E_ROM_tipos[(k * 4) + i];
      }

      if (nbbits == 20)
      {
         pos = 0;
         ps = 0.0F;
         alp = 0.0F;
         memset(vec, 0, L_SUBFR * sizeof(Float32));
      }
      else if ((nbbits == 36) | (nbbits == 44))
      {
         /* first stage: fix 2 pulses */
         pos = 2;

         ind[0] = pos_max[ipos[0]];
         ind[1] = pos_max[ipos[1]];
         ps = dn[ind[0]] + dn[ind[1]];
         alp = rrixix[ipos[0]][ind[0] >> 2] + rrixix[ipos[1]][ind[1] >> 2] +
            rrixiy[ipos[0]][((ind[0] >> 2) << 4) + (ind[1] >> 2)];

         if (sign[ind[0]] < 0.0)
         {
            p0 = h_inv - ind[0];
         }
         else
         {
            p0 = h - ind[0];
         }

         if (sign[ind[1]] < 0.0)
         {
            p1 = h_inv - ind[1];
         }
         else
         {
            p1 = h - ind[1];
         }

         vec[0] = p0[0] + p1[0];
         vec[1] = p0[1] + p1[1];
         vec[2] = p0[2] + p1[2];
         vec[3] = p0[3] + p1[3];

         for (i = 4; i < L_SUBFR; i += 6)
         {
            vec[i] = p0[i] + p1[i];
            vec[i + 1] = p0[i + 1] + p1[i + 1];
            vec[i + 2] = p0[i + 2] + p1[i + 2];
            vec[i + 3] = p0[i + 3] + p1[i + 3];
            vec[i + 4] = p0[i + 4] + p1[i + 4];
            vec[i + 5] = p0[i + 5] + p1[i + 5];
         }

         if (nbbits == 44)
         {
            ipos[8] = 0;
            ipos[9] = 1;
         }
      }
      else
      {
         /* first stage: fix 4 pulses */
         pos = 4;

         ind[0] = pos_max[ipos[0]];
         ind[1] = pos_max[ipos[1]];
         ind[2] = pos_max[ipos[2]];
         ind[3] = pos_max[ipos[3]];
         ps = dn[ind[0]] + dn[ind[1]] + dn[ind[2]] + dn[ind[3]];

         p0 = h - ind[0];
         if (sign[ind[0]] < 0.0)
         {
            p0 = h_inv - ind[0];
         }

         p1 = h - ind[1];
         if (sign[ind[1]] < 0.0)
         {
            p1 = h_inv - ind[1];
         }

         p2 = h - ind[2];
         if (sign[ind[2]] < 0.0)
         {
            p2 = h_inv - ind[2];
         }

         p3 = h - ind[3];
         if (sign[ind[3]] < 0.0)
         {
            p3 = h_inv - ind[3];
         }

         vec[0] = p0[0] + p1[0] + p2[0] + p3[0];

         for (i = 1; i < L_SUBFR; i += 3)
         {
            vec[i] = p0[i] + p1[i] + p2[i] + p3[i];
            vec[i + 1] = p0[i + 1] + p1[i + 1] + p2[i + 1] + p3[i + 1];
            vec[i + 2] = p0[i + 2] + p1[i + 2] + p2[i + 2] + p3[i + 2];
         }

         alp = 0.0F;
         alp += vec[0] * vec[0] + vec[1] * vec[1];
         alp += vec[2] * vec[2] + vec[3] * vec[3];

         for (i = 4; i < L_SUBFR; i += 6)
         {
            alp += vec[i] * vec[i];
            alp += vec[i + 1] * vec[i + 1];
            alp += vec[i + 2] * vec[i + 2];
            alp += vec[i + 3] * vec[i + 3];
            alp += vec[i + 4] * vec[i + 4];
            alp += vec[i + 5] * vec[i + 5];
         }

         alp *= 0.5F;

         if (nbbits == 72)
         {
            ipos[16] = 0;
            ipos[17] = 1;
         }
      }

      /* other stages of 2 pulses */

      for (j = pos, st = 0; j < nb_pulse; j += 2, st++)
      {
         /*
          * Calculate correlation of all possible positions
          * of the next 2 pulses with previous fixed pulses.
          * Each pulse can have 16 possible positions.
          */
         E_ACELP_h_vec_corr1(h, vec, ipos[j], sign, rrixix, cor_x, dn2_pos,
            nbpos[st]);

         E_ACELP_h_vec_corr2(h, vec, ipos[j + 1], sign, rrixix, cor_y);

         /*
          * Find best positions of 2 pulses.
          */
         E_ACELP_2pulse_search(nbpos[st], ipos[j], ipos[j + 1], &ps, &alp,
            &ind[j], &ind[j+1], dn, dn2_pos, cor_x, cor_y, rrixiy);

         if (j < (nb_pulse - 2))
         {
            p0 = h - ind[j];
            if (sign[ind[j]] < 0.0)
            {
               p0 = h_inv - ind[j];
            }

            p1 = h - ind[j + 1];
            if (sign[ind[j + 1]] < 0.0)
            {
               p1 = h_inv - ind[j + 1];
            }

            vec[0] += p0[0] + p1[0];
            vec[1] += p0[1] + p1[1];
            vec[2] += p0[2] + p1[2];
            vec[3] += p0[3] + p1[3];

            for (i = 4; i < L_SUBFR; i += 6)
            {
               vec[i] += p0[i] + p1[i];
               vec[i + 1] += p0[i + 1] + p1[i + 1];
               vec[i + 2] += p0[i + 2] + p1[i + 2];
               vec[i + 3] += p0[i + 3] + p1[i + 3];
               vec[i + 4] += p0[i + 4] + p1[i + 4];
               vec[i + 5] += p0[i + 5] + p1[i + 5];

            }
         }
      }

      /* memorise the best codevector */

      ps = ps * ps;
      s = (alpk * ps) - (psk * alp);

      if (s > 0.0F)
      {
         psk = ps;
         alpk = alp;
         memcpy(codvec, ind, nb_pulse * sizeof(Word32));
      }
  }

  /*
   * Build the codeword, the filtered codeword and index of codevector.
   */

  memset(code, 0,  L_SUBFR * sizeof(Word16));
  memset(y, 0,  L_SUBFR * sizeof(Float32));
  memset(ind, 0xffffffff, NPMAXPT * 4 * sizeof(Word32));

  for (k = 0; k < nb_pulse; k++)
  {
     i = codvec[k];  /* read pulse position  */
     val = sign[i];  /* read sign            */

     index = i / 4;  /* pos of pulse (0..15) */
     track = i % 4;

     if (val > 0)
     {
        code[i] += 512;
        codvec[k] += (2 * L_SUBFR);
     }
     else
     {
        code[i] -= 512;
        index += 16;
     }

     i = track * NPMAXPT;

     while (ind[i] >= 0)
     {
        i++;
     }

     ind[i] = index;

     p0 = h_inv - codvec[k];
     y[0] += p0[0];

     for(i = 1; i < L_SUBFR; i += 3)
     {
        y[i] += p0[i];
        y[i + 1] += p0[i + 1];
        y[i + 2] += p0[i + 2];
     }
  }

  if (nbbits == 20)
  {
     for (track = 0; track < 4; track++)
     {
        k = track * NPMAXPT;
        _index[track] = E_ACELP_quant_1p_N1(ind[k], 4);
     }
  }
  else if (nbbits == 36)
  {
     for (track = 0; track < 4; track++)
     {
        k = track * NPMAXPT;
        _index[track] = E_ACELP_quant_2p_2N1(ind[k], ind[k + 1], 4);
     }
  }
  else if (nbbits == 44)
  {
     for (track = 0; track < (4 - 2); track++)
     {
        k = track * NPMAXPT;
        _index[track] =
           E_ACELP_quant_3p_3N1(ind[k], ind[k + 1], ind[k + 2], 4);
     }
     for (track = 2; track < 4; track++)
     {
        k = track * NPMAXPT;
        _index[track] = E_ACELP_quant_2p_2N1(ind[k], ind[k + 1], 4);
     }
  }
  else if (nbbits == 52)
  {
     for (track = 0; track < 4; track++)
     {
        k = track*NPMAXPT;
        _index[track] =
           E_ACELP_quant_3p_3N1(ind[k], ind[k + 1], ind[k + 2], 4);
     }
  }
  else if (nbbits == 64)
  {
     for (track = 0; track < 4; track++)
     {
        k = track * NPMAXPT;
        L_index = E_ACELP_quant_4p_4N(&ind[k], 4);
        _index[track] = ((L_index >> 14) & 3);
        _index[track + 4] = (L_index & 0x3FFF);
     }
  }
  else if (nbbits == 72)
  {
     for (track=0; track< (4 - 2); track++)
     {
        k = track * NPMAXPT;
        L_index = E_ACELP_quant_5p_5N(&ind[k], 4);
        _index[track] = ((L_index >> 10) & 0x03FF);
        _index[track + 4] = (L_index & 0x03FF);
     }
     for (track = 2; track < 4; track++)
     {
        k = track * NPMAXPT;
        L_index = E_ACELP_quant_4p_4N(&ind[k], 4);
        _index[track] = ((L_index >> 14) & 3);
        _index[track + 4] = (L_index & 0x3FFF);
     }
  }
  else if (nbbits == 88)
  {
     for (track = 0; track < 4; track++)
     {
        k = track * NPMAXPT;
        L_index = E_ACELP_quant_6p_6N_2(&ind[k], 4);
        _index[track] = ((L_index >> 11) & 0x07FF);
        _index[track + 4] = (L_index & 0x07FF);
     }
  }

  return;
}

/*
 * E_ACELP_gains_quantise
 *
 * Parameters:
 *    code        I: Innovative code vector
 *    nbits       I: number of bits (6 or 7)
 *    gain_pit  I/O: Pitch gain / Quantized pitch gain
 *    gain_code   O: Quantized codebook gain
 *    coeff       O: correlations
 *                   <y1,y1>, -2<xn,y1>, <y2,y2>, -2<xn,y2>, 2<y1,y2>
 *    gp_clip     I: gain pitch clipping flag (1 = clipping)
 *    mem       I/O: static memory
 *
 * Function:
 *    Quantization of pitch and codebook gains.
 *    MA prediction is performed on the innovation energy
 *    (in dB with mean removed).
 *    An initial predicted gain, g_0, is first determined and the correction
 *    factor alpha = gain / g_0 is quantized.
 *    The pitch gain and the correction factor are vector quantized and the
 *    mean-squared weighted error criterion is used in the quantizer search.
 *    Subrame size is L_SUBFR
 *
 * Returns:
 *    index of quantizer
 */
Word32 E_ACELP_gains_quantise(Word16 code[], Word32 nbits, Float32 f_gain_pit,
                              Word16 *gain_pit, Word32 *gain_code,
                              Float32 *coeff, Word32 gp_clip,
                              Word16 *past_qua_en)
{

   Word32 i, j, indice = 0, min_ind, size, L_tmp, gcode_inov, L_gcode0;
   Word32 exp;
   Float32 gcode0;
   Float32 dist, dist_min, g_pitch, g_code, ener_code, pred_code;
   Float32 coef0, coef1, coef2, coef3, coef4;
   const Float32 *t_qua_gain, *p;
   Word16 s_exp, s_gcode0, exp_gcode0, frac;

   /*
    * Find the initial quantization pitch index
    * Set gains search range
    */
   if (nbits == 6)
   {
      t_qua_gain = E_ROM_qua_gain6b;
      min_ind = 0;
      size = 64;
      if (gp_clip == 1)
      {
         size -= 16; /* limit gain pitch to 1.0 */
      }
   }
   else
   {
      t_qua_gain = E_ROM_qua_gain7b;

      p = E_ROM_qua_gain7b + 64; /* pt at 1/4th of table */

      j = NB_QUA_GAIN7B - 64;
      if (gp_clip == 1)
      {
         j -= 27; /* limit gain pitch to 1.0 */
      }

      min_ind = 0;
      g_pitch = f_gain_pit;

      for (i = 0; i < j; i++, p += 2)
      {
         if (g_pitch > *p)
         {
            min_ind++;
         }
      }

      size = 64;
   }

   /* innovation energy */
   L_tmp = E_UTIL_dot_product12(code, code, L_SUBFR, &exp);

   ener_code = (Float32)(L_tmp * pow(2, (exp - 31) - 18));
   ener_code = (Float32)(10.0F * log10(ener_code * 0.015625F));
   /* exp: -18 (code in Q9), -6 (/L_subfr) */
   s_exp = (Word16)(exp - (18 + 6));

   E_UTIL_normalised_inverse_sqrt(&L_tmp, &s_exp);

   if (s_exp > 3)
   {
      L_tmp <<= (s_exp - 3);
   }
   else
   {
      L_tmp >>= (3 - s_exp);
   }

   gcode_inov = (Word16)(L_tmp >> 16);  /* g_code_inov in Q12 */

   /*
    * Compute gcode0.
    *  = Sum(i=0,1) pred[i] * past_qua_en[i] + mean_ener - ener_code
    */

   /* MEAN_ENER in Q24 = 0x1e000000 */
   /* MA prediction coeff = {0.5, 0.4, 0.3, 0.2} in Q13 */
   L_tmp = 0xF000000 + 4096 * past_qua_en[0]; /* Q13 * Q10 -> Q24 */
   L_tmp = L_tmp + 3277 * past_qua_en[1];     /* Q13 * Q10 -> Q24 */
   L_tmp = L_tmp + 2458 * past_qua_en[2];     /* Q13 * Q10 -> Q24 */
   L_tmp = L_tmp + 1638 * past_qua_en[3];     /* Q13 * Q10 -> Q24 */

   L_gcode0 = L_tmp >> 15;             /* From Q24 to Q8  */
   pred_code = (Float32)(L_gcode0 * pow(2, -8));

   /*
    * gcode0 = pow(10.0, gcode0/20)
    *        = pow(2, 3.321928*gcode0/20)
    *        = pow(2, 0.166096*gcode0)
    */

   L_tmp = (L_gcode0 * 5443) >> 7; /* *0.166096 in Q15 -> Q24, From Q24 to Q16 */
   E_UTIL_l_extract(L_tmp, &exp_gcode0, &frac); /* Extract exponant of gcode0  */

   s_gcode0 = (Word16)(E_UTIL_pow2(14, frac));  /* Put 14 as exponant so that  */
   /*
    * output of Pow2() will be:
    * 16384 < Pow2() <= 32767
    */
   exp_gcode0 = (Word16)(exp_gcode0 - 14);
   /* Search for best quantizer */
   gcode0 = pred_code - ener_code;
   gcode0 = (Float32)pow(10.0, gcode0 * 0.05F);   /* predicted gain */

   dist_min = FLT_MAX;
   p = t_qua_gain + min_ind * 2;

   coef0 = coeff[0];
   coef1 = coeff[1];
   coef2 = coeff[2];
   coef3 = coeff[3];
   coef4 = coeff[4];

   for (i = 0; i < size; i++)
   {
      g_pitch = *p++;                  /* pitch gain */

      g_code = gcode0 * *p++;         /* codebook gain */

      dist = g_pitch * g_pitch * coef0
         + g_pitch * coef1
         + g_code * g_code * coef2
         + g_code * coef3
         + g_pitch * g_code * coef4;

      if (dist < dist_min)
      {
         dist_min = dist;
         indice = i;
      }
   }

   indice += min_ind;
   *gain_pit  = (Word16)floor(t_qua_gain[indice * 2] * (1 << 14) + 0.5F);
   L_tmp = (Word32)floor(t_qua_gain[indice * 2 + 1] * (1 << 11) + 0.5F);
   L_tmp = E_UTIL_saturate(L_tmp);
   L_tmp *= s_gcode0;
   exp_gcode0 += 5;

   if (exp_gcode0 >= 0)
   {
      *gain_code = L_tmp << exp_gcode0; /* gain of code in Q16 */
   }
   else
   {
      *gain_code = L_tmp >> -exp_gcode0; /* gain of code in Q16 */
   }

   /* adjust gain according to energy of code */
   E_UTIL_l_extract((Word32)*gain_code, &s_exp, &frac);
   L_tmp = E_UTIL_mpy_32_16(s_exp, frac, (Word16)gcode_inov);

   if (L_tmp < 0xFFFFFFF)
   {
      *gain_code = L_tmp << 3;             /* gcode_inov in Q12 */
   }
   else
   {
      *gain_code = 0x7FFFFFFF;
   }

   /*
    * qua_ener = 20*log10(g_code)
    *          = 6.0206*log2(g_code)
    *          = 6.0206*(log2(g_codeQ11) - 11)
    */

   L_tmp = (Word32)floor(t_qua_gain[indice * 2 + 1] * (1 << 11) + 0.5F);
   L_tmp = E_UTIL_saturate(L_tmp);
   E_UTIL_log2_32(L_tmp, &s_exp, &frac);
   s_exp = (Word16)(s_exp - 11);
   L_tmp = E_UTIL_mpy_32_16(s_exp, frac, 24660);   /* x 6.0206 in Q12 */

   /* update table of past quantized energies */

   past_qua_en[3] = past_qua_en[2];
   past_qua_en[2] = past_qua_en[1];
   past_qua_en[1] = past_qua_en[0];
   past_qua_en[0] = (Word16)(L_tmp >> 3); /* result in Q10 */

   return indice;
}

