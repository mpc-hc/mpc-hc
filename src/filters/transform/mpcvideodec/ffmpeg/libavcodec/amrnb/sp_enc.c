/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

/*
 * sp_enc.c
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    This module contains all the functions needed encoding 160
 *    16-bit speech samples to AMR encoder parameters.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include "sp_enc.h"
#include "rom_enc.h"

/*
 * Definition of structures used in encoding process
 */
typedef struct
{
   Float32 y2;
   Float32 y1;
   Float32 x0;
   Float32 x1;

}Pre_ProcessState;

#ifdef VAD2

/* Defines for VAD2 */
#define	FRM_LEN1		80
#define	DELAY0			24
#define	FFT_LEN1		128

#define	UPDATE_CNT_THLD1	50

#define	INIT_FRAMES		4

#define	CNE_SM_FAC1		0.1
#define	CEE_SM_FAC1		0.55

#define	HYSTER_CNT_THLD1	6	/* forced update constants... */
#define	HIGH_ALPHA1		0.9
#define	LOW_ALPHA1		0.7
#define	ALPHA_RANGE1		(HIGH_ALPHA1-LOW_ALPHA1)

#define NORM_ENRG		(4.0)	/* account for div by 2 by the HPF */
#define	MIN_CHAN_ENRG		(0.0625 / NORM_ENRG)
#define	INE			(16.0 / NORM_ENRG)
#define	NOISE_FLOOR		(1.0 / NORM_ENRG)

#define	PRE_EMP_FAC1		(-0.8)

#define	NUM_CHAN		16
#define	LO_CHAN			0
#define	HI_CHAN			15
#define	UPDATE_THLD		35

#define	SINE_START_CHAN		2
#define	P2A_THRESH		10.0
#define	DEV_THLD1		28.0

/* Defines for the FFT function */
#define	SIZE			128
#define	SIZE_BY_TWO		64
#define	NUM_STAGE		6

#define	PI			3.141592653589793

#define	TRUE			1
#define	FALSE			0

/* Macros */
#define	min(a,b)		((a)<(b)?(a):(b))
#define	max(a,b)		((a)>(b)?(a):(b))
#define	square(a)		((a)*(a))

/* structures */
typedef struct
{
  Float32 pre_emp_mem;
  Word16  update_cnt;
  Word16  hyster_cnt;
  Word16  last_update_cnt;
  Float32 ch_enrg_long_db[NUM_CHAN];
  Word32  Lframe_cnt;
  Float32 ch_enrg[NUM_CHAN];
  Float32 ch_noise[NUM_CHAN];
  Float32 tsnr;
  Word16  hangover;
  Word16  burstcount;
  Word16  fupdate_flag;
  Float32 negSNRvar;
  Float32 negSNRbias;
  Float32 R0;
  Float32 Rmax;
  Word16  LTP_flag;
}vadState;
#else
typedef struct
{
   Float32 bckr_est[COMPLEN];   /* background noise estimate */
   Float32 ave_level[COMPLEN];


   /* averaged input components for stationary estimation */
   Float32 old_level[COMPLEN];   /* input levels of the previous frame */
   Float32 sub_level[COMPLEN];


   /* input levels calculated at the end of a frame (lookahead) */
   Float32 a_data5[3][2];   /* memory for the filter bank */
   Float32 a_data3[5];   /* memory for the filter bank */
   Float32 best_corr_hp;   /* FIP filtered value */


   /* counts length of a speech burst incl HO addition */
   Float32 corr_hp_fast;   /* filtered value */
   Word32 vadreg;   /* flags for intermediate VAD decisions */
   Word32 pitch;   /* flags for pitch detection */
   Word32 oldlag_count, oldlag;   /* variables for pitch detection */
   Word32 complex_high;   /* flags for complex detection */
   Word32 complex_low;   /* flags for complex detection */
   Word32 complex_warning;   /* complex background warning */
   Word32 tone;   /* flags for tone detection */
   Word16 burst_count;   /* counts length of a speech burst */
   Word16 hang_count;   /* hangover counter */
   Word16 stat_count;   /* stationary counter */
   Word16 complex_hang_count;   /* complex hangover counter, used by VAD */
   Word16 complex_hang_timer;   /* hangover initiator, used by CAD */
   Word16 speech_vad_decision;   /* final decision */
   Word16 sp_burst_count;


}vadState;
#endif
#define DTX_HIST_SIZE 8
#define DTX_ELAPSED_FRAMES_THRESH (24 + 7 -1)
#define DTX_HANG_CONST 7   /* yields eight frames of SP HANGOVER */
typedef struct
{
   Float32 lsp_hist[M * DTX_HIST_SIZE];
   Float32 log_en_hist[DTX_HIST_SIZE];
   Word32 init_lsf_vq_index;
   Word16 hist_ptr;
   Word16 log_en_index;
   Word16 lsp_index[3];


   /* DTX handler stuff */
   Word16 dtxHangoverCount;
   Word16 decAnaElapsedCount;


}dtx_encState;
typedef struct
{
   /* gain history */
   Float32 gp[N_FRAME];


   /* counters */
   Word16 count;


}tonStabState;
typedef struct
{
   Word32 past_qua_en[4];


   /* normal MA predictor memory, (contains 20*log10(qua_err)) */
}gc_predState;

typedef struct
{
   Float32 prev_alpha;   /* previous adaptor output, */
   Float32 prev_gc;   /* previous code gain, */
   Float32 ltpg_mem[LTPG_MEM_SIZE];   /* LTP coding gain history, */
   Word16 onset;   /* onset state, */


   /* (ltpg_mem[0] not used for history) */
}gain_adaptState;
typedef struct
{

   Float32 sf0_target_en;
   Float32 sf0_coeff[5];
   Word32 sf0_gcode0_exp;
   Word32 sf0_gcode0_fra;
   Word16 *gain_idx_ptr;


   gc_predState * gc_predSt;
   gc_predState * gc_predUncSt;
   gain_adaptState * adaptSt;
}gainQuantState;
typedef struct
{
   Word32 T0_prev_subframe;   /* integer pitch lag of previous sub-frame */


}Pitch_frState;
typedef struct
{
   Pitch_frState * pitchSt;
}clLtpState;
typedef struct
{
   Float32 ada_w;
   Word32 old_T0_med;
   Word16 wght_flg;


}pitchOLWghtState;
typedef struct
{
   Float32 past_rq[M];   /* Past quantized prediction error */


}Q_plsfState;
typedef struct
{
   /* Past LSPs */
   Float32 lsp_old[M];
   Float32 lsp_old_q[M];


   /* Quantization state */
   Q_plsfState * qSt;
}lspState;
typedef struct
{
   Float32 old_A[M + 1];   /* Last A(z) for case of unstable filter */


}LevinsonState;
typedef struct
{
   LevinsonState * LevinsonSt;
}lpcState;
typedef struct
{
   /* Speech vector */
   Float32 old_speech[L_TOTAL];
   Float32 *speech, *p_window, *p_window_12k2;
   Float32 *new_speech;   /* Global variable */


   /* Weight speech vector */
   Float32 old_wsp[L_FRAME + PIT_MAX];
   Float32 *wsp;


   /* OL LTP states */
   Word32 old_lags[5];
   Float32 ol_gain_flg[2];


   /* Excitation vector */
   Float32 old_exc[L_FRAME + PIT_MAX + L_INTERPOL];
   Float32 *exc;


   /* Zero vector */
   Float32 ai_zero[L_SUBFR + MP1];
   Float32 *zero;


   /* Impulse response vector */
   Float32 *h1;
   Float32 hvec[L_SUBFR * 2];


   /* Substates */
   lpcState * lpcSt;
   lspState * lspSt;
   clLtpState * clLtpSt;
   gainQuantState * gainQuantSt;
   pitchOLWghtState * pitchOLWghtSt;
   tonStabState * tonStabSt;
   vadState * vadSt;

   Word32 dtx;


   dtx_encState * dtxEncSt;

   /* Filter's memory */
   Float32 mem_syn[M], mem_w0[M], mem_w[M];
   Float32 mem_err[M + L_SUBFR], *error;
   Float32 sharp;


}cod_amrState;
typedef struct
{
   cod_amrState * cod_amr_state;
   Pre_ProcessState * pre_state;

   Word32 dtx;


}Speech_Encode_FrameState;


/*
 * Dotproduct40
 *
 *
 * Parameters:
 *    x                 I: First input
 *    y                 I: Second input
 * Function:
 *    Computes dot product size 40
 *
 * Returns:
 *    acc                dot product
 */
static Float64 Dotproduct40( Float32 *x, Float32 *y )
{
   Float64 acc;


   acc = x[0] * y[0] + x[1] * y[1] + x[2] * y[2] + x[3] * y[3];
   acc += x[4] * y[4] + x[5] * y[5] + x[6] * y[6] + x[7] * y[7];
   acc += x[8] * y[8] + x[9] * y[9] + x[10] * y[10] + x[11] * y[11];
   acc += x[12] * y[12] + x[13] * y[13] + x[14] * y[14] + x[15] * y[15];
   acc += x[16] * y[16] + x[17] * y[17] + x[18] * y[18] + x[19] * y[19];
   acc += x[20] * y[20] + x[21] * y[21] + x[22] * y[22] + x[23] * y[23];
   acc += x[24] * y[24] + x[25] * y[25] + x[26] * y[26] + x[27] * y[27];
   acc += x[28] * y[28] + x[29] * y[29] + x[30] * y[30] + x[31] * y[31];
   acc += x[32] * y[32] + x[33] * y[33] + x[34] * y[34] + x[35] * y[35];
   acc += x[36] * y[36] + x[37] * y[37] + x[38] * y[38] + x[39] * y[39];
   return( acc );
}


/*
 * Autocorr
 *
 *
 * Parameters:
 *    x                 I: Input signal
 *    r                 O: Autocorrelations
 *    wind              I: Window for LPC analysis
 * Function:
 *    Calculate autocorrelation with window, LPC order = M
 *
 * Returns:
 *    void
 */
static void Autocorr( Float32 x[], Float32 r[], const Float32 wind[] )
{
   Word32 i, j;   /* Counters */
   Float32 y[L_WINDOW + M + 1];   /* Windowed signal */
   Float64 sum;   /* temp */


   /*
    * Windowing of signal
    */
   for ( i = 0; i < L_WINDOW; i++ ) {
      y[i] = x[i] * wind[i];
   }

   /*
    * Zero remaining memory
    */
   memset( &y[L_WINDOW], 0, 44 );

   /*
    * Autocorrelation
    */
   for ( i = 0; i <= M; i++ ) {
      sum = 0;

      for ( j = 0; j < L_WINDOW; j += 40 ) {
         sum += Dotproduct40( &y[j], &y[j + i] );
      }
      r[i] = (Float32)sum;
   }
}


/*
 * Levinson
 *
 *
 * Parameters:
 *    old_A             I: Vector of old LP coefficients [M+1]
 *    r                 I: Vector of autocorrelations    [M+1]
 *    a                 O: LP coefficients               [M+1]
 *    rc                O: Reflection coefficients       [4]
 * Function:
 *    Levinson-Durbin algorithm
 *
 * Returns:
 *    void
 *
 */
static void Levinson( Float32 *old_A, Float32 *r, Float32 *A, Float32 *rc )
{
   Float32 sum, at, err;
   Word32 l, j, i;
   Float32 rct[M];   /* temporary reflection coefficients  0,...,m-1 */


   rct[0] = ( -r[1] ) / r[0];
   A[0] = 1.0F;
   A[1] = rct[0];
   err = r[0] + r[1] * rct[0];

   if ( err <= 0.0 )
      err = 0.01F;

   for ( i = 2; i <= M; i++ ) {
      sum = 0.0F;

      for ( j = 0; j < i; j++ )
         sum += r[i - j] * A[j];
      rct[i - 1] = ( -sum ) / ( err );

      for ( j = 1; j <= ( i / 2 ); j++ ) {
         l = i - j;
         at = A[j] + rct[i - 1] *A[l];
         A[l] += rct[i - 1] *A[j];
         A[j] = at;
      }
      A[i] = rct[i - 1];
      err += rct[i - 1] *sum;

      if ( err <= 0.0 )
         err = 0.01F;
   }
   memcpy( rc, rct, 4 * sizeof( Float32 ) );
   memcpy( old_A, A, MP1 * sizeof( Float32 ) );
}


/*
 * lpc
 *
 *
 * Parameters:
 *    old_A             O: Vector of old LP coefficients [M+1]
 *    x                 I: Input signal
 *    x_12k2            I: Input signal 12.2k
 *    a                 O: predictor coefficients
 *    mode              I: AMR mode
 * Function:
 *    LP analysis
 *
 *    In 12.2 kbit/s mode linear prediction (LP) analysis is performed
 *    twice per speech frame using the auto-correlation approach with
 *    30 ms asymmetric windows. No lookahead is used in
 *    the auto-correlation computation.
 *
 *    In other modes analysis is performed once per speech frame
 *    using the auto-correlation approach with 30 ms asymmetric windows.
 *    A lookahead of 40 samples (5 ms) is used in the auto-correlation computation.
 *
 *    The auto-correlations of windowed speech are converted to the LP
 *    coefficients using the Levinson-Durbin algorithm.
 *    Then the LP coefficients are transformed to the Line Spectral Pair
 *    (LSP) domain  for quantization and interpolation purposes.
 *    The interpolated quantified and unquantized filter coefficients
 *    are converted back to the LP filter coefficients
 *    (to construct the synthesis and weighting filters at each subframe).
 *
 * Returns:
 *    void
 *
 */
static void lpc( Float32 *old_A, Float32 x[], Float32 x_12k2[], Float32 a[], enum Mode
      mode )
{
   Word32 i;
   Float32 r[MP1];
   Float32 rc[4];


   if ( mode == MR122 ) {
      Autocorr( x_12k2, r, window_160_80 );

      /*
       * Lag windowing
       */
      for ( i = 1; i <= M; i++ ) {
         r[i] = r[i] * lag_wind[i - 1];
      }
      r[0] *= 1.0001F;

      if ( r[0] < 1.0F )
         r[0] = 1.0F;

      /*
       * Levinson Durbin
       */
      Levinson( old_A, r, &a[MP1], rc );

      /*
       * Autocorrelations
       */
      Autocorr( x_12k2, r, window_232_8 );

      /*
       * Lag windowing
       */
      for ( i = 1; i <= M; i++ ) {
         r[i] = r[i] * lag_wind[i - 1];
      }
      r[0] *= 1.0001F;

      if ( r[0] < 1.0F )
         r[0] = 1.0F;

      /*
       * Levinson Durbin
       */
      Levinson( old_A, r, &a[MP1 * 3], rc );
   }
   else {
      /*
       * Autocorrelations
       */
      Autocorr( x, r, window_200_40 );

      /*
       * a 60 Hz bandwidth expansion is used by lag windowing
       * the auto-correlations. Further, auto-correlation[0] is
       * multiplied by the white noise correction factor 1.0001
       * which is equivalent to adding a noise floor at -40 dB.
       */
      for ( i = 1; i <= M; i++ ) {
         r[i] = r[i] * lag_wind[i - 1];
      }
      r[0] *= 1.0001F;

      if ( r[0] < 1.0F )
         r[0] = 1.0F;

      /*
       * Levinson Durbin
       */
      Levinson( old_A, r, &a[MP1 * 3], rc );
   }
}


/*
 * Chebps
 *
 *
 * Parameters:
 *    x                 I: Cosine value for the freqency given
 *    f                 I: angular freqency
 * Function:
 *    Evaluates the Chebyshev polynomial series
 *
 * Returns:
 *    result of polynomial evaluation
 */
static Float32 Chebps( Float32 x, Float32 f[] )
{
   Float32 b0, b1, b2, x2;
   Word32 i;


   x2 = 2.0F * x;
   b2 = 1.0F;
   b1 = x2 + f[1];

   for ( i = 2; i < 5; i++ ) {
      b0 = x2 * b1 - b2 + f[i];
      b2 = b1;
      b1 = b0;
   }
   return( x * b1 - b2 + f[i] );
}


/*
 * Az_lsp
 *
 *
 * Parameters:
 *    a                 I: Predictor coefficients              [MP1]
 *    lsp               O: Line spectral pairs                 [M]
 *    old_lsp           I: Old lsp, in case not found 10 roots [M]
 *
 * Function:
 *    LP to LSP conversion
 *
 *    The LP filter coefficients A[], are converted to the line spectral pair
 *    (LSP) representation for quantization and interpolation purposes.
 *
 * Returns:
 *    void
 */
static void Az_lsp( Float32 a[], Float32 lsp[], Float32 old_lsp[] )
{
   Word32 i, j, nf, ip;
   Float32 xlow, ylow, xhigh, yhigh, xmid, ymid, xint;
   Float32 y;
   Float32 *coef;
   Float32 f1[6], f2[6];


   /*
    *  find the sum and diff. pol. F1(z) and F2(z)
    */
   f1[0] = 1.0F;
   f2[0] = 1.0F;

   for ( i = 0; i < ( NC ); i++ ) {
      f1[i + 1] = a[i + 1] +a[M - i] - f1[i];
      f2[i + 1] = a[i + 1] -a[M - i] + f2[i];
   }
   f1[NC] *= 0.5F;
   f2[NC] *= 0.5F;

   /*
    * find the LSPs using the Chebychev pol. evaluation
    */
   nf = 0;   /* number of found frequencies */
   ip = 0;   /* indicator for f1 or f2 */
   coef = f1;
   xlow = grid[0];
   ylow = Chebps( xlow, coef );
   j = 0;

   while ( ( nf < M ) && ( j < 60 ) ) {
      j++;
      xhigh = xlow;
      yhigh = ylow;
      xlow = grid[j];
      ylow = Chebps( xlow, coef );

      if ( ylow * yhigh <= 0 ) {
         /* divide 4 times the interval */
         for ( i = 0; i < 4; i++ ) {
            xmid = ( xlow + xhigh ) * 0.5F;
            ymid = Chebps( xmid, coef );

            if ( ylow * ymid <= 0.0F ) {
               yhigh = ymid;
               xhigh = xmid;
            }
            else {
               ylow = ymid;
               xlow = xmid;
            }
         }

         /*
          * Linear interpolation
          * xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow)
          */
         y = yhigh - ylow;

         if ( y == 0 ) {
            xint = xlow;
         }
         else {
            y = ( xhigh - xlow ) / ( yhigh - ylow );
            xint = xlow - ylow * y;
         }
         lsp[nf] = xint;
         xlow = xint;
         nf++;

         if ( ip == 0 ) {
            ip = 1;
            coef = f2;
         }
         else {
            ip = 0;
            coef = f1;
         }
         ylow = Chebps( xlow, coef );
      }
   }

   /* Check if M roots found */
   if ( nf < M ) {
      memcpy( lsp, old_lsp, M <<2 );
   }
   return;
}


/*
 * Get_lsp_pol
 *
 *
 * Parameters:
 *    lsp                 I: line spectral frequencies
 *    f                   O: polynomial F1(z) or F2(z)
 *
 * Function:
 *    Find the polynomial F1(z) or F2(z) from the LSPs.
 *
 *    F1(z) = product ( 1 - 2 lsp[i] z^-1 + z^-2 )
 *             i=0,2,4,6,8
 *    F2(z) = product   ( 1 - 2 lsp[i] z^-1 + z^-2 )
 *             i=1,3,5,7,9
 *
 *    where lsp[] is the LSP vector in the cosine domain.
 *
 *    The expansion is performed using the following recursion:
 *
 *    f[0] = 1
 *    b = -2.0 * lsp[0]
 *    f[1] = b
 *    for i=2 to 5 do
 *       b = -2.0 * lsp[2*i-2];
 *       f[i] = b*f[i-1] + 2.0*f[i-2];
 *       for j=i-1 down to 2 do
 *          f[j] = f[j] + b*f[j-1] + f[j-2];
 *       f[1] = f[1] + b;
 *
 * Returns:
 *    void
 */
static void Get_lsp_pol( Float32 *lsp, Float32 *f )
{
   Word32 i, j;
   Float32 T0;


   f[0] = 1.0F;
   f[1] = -2.0F * lsp[0];

   for ( i = 2; i <= 5; i++ ) {
      T0 = -2.0F * lsp[2 * i - 2];
      f[i] = ( Float32 )( T0 * f[i - 1] +2.0F * f[i - 2] );

      for ( j = i - 1; j >= 2; j-- ) {
         f[j] = f[j] + T0 * f[j - 1] +f[j - 2];
      }
      f[1] = f[1] + T0;
   }
   return;
}


/*
 * Lsp_Az
 *
 *
 * Parameters:
 *    lsp                 I: Line spectral frequencies
 *    a                   O: Predictor coefficients
 *
 * Function:
 *    Converts from the line spectral pairs (LSP) to LP coefficients,
 *    for a 10th order filter.
 *
 * Returns:
 *    void
 */
static void Lsp_Az( Float32 lsp[], Float32 a[] )
{
   Float32 f1[6], f2[6];
   Word32 i, j;


   Get_lsp_pol( &lsp[0], f1 );
   Get_lsp_pol( &lsp[1], f2 );

   for ( i = 5; i > 0; i-- ) {
      f1[i] += f1[i - 1];
      f2[i] -= f2[i - 1];
   }
   a[0] = 1;

   for ( i = 1, j = 10; i <= 5; i++, j-- ) {
      a[i] = ( Float32 )( ( f1[i] + f2[i] ) * 0.5F );
      a[j] = ( Float32 )( ( f1[i] - f2[i] ) * 0.5F );
   }
   return;
}


/*
 * Int_lpc_1and3_2
 *
 *
 * Parameters:
 *    lsp_old        I: LSP vector at the 4th subfr. of past frame      [M]
 *    lsp_mid        I: LSP vector at the 2nd subframe of present frame [M]
 *    lsp_new        I: LSP vector at the 4th subframe of present frame [M]
 *    az             O: interpolated LP parameters in subframes 1 and 3
 *                                                                [AZ_SIZE]
 *
 * Function:
 *    Interpolation of the LPC parameters. Same as the Int_lpc
 *    function but we do not recompute Az() for subframe 2 and
 *	   4 because it is already available.
 *
 * Returns:
 *    void
 */
static void Int_lpc_1and3_2( Float32 lsp_old[], Float32 lsp_mid[], Float32
      lsp_new[], Float32 az[] )
{
   Float32 lsp[M];
   Word32 i;


   for ( i = 0; i < M; i += 2 ) {
      lsp[i] = ( lsp_mid[i] + lsp_old[i] ) * 0.5F;
      lsp[i + 1] = ( lsp_mid[i + 1] +lsp_old[i+1] )*0.5F;
   }

   /* Subframe 1 */
   Lsp_Az( lsp, az );
   az += MP1 * 2;

   for ( i = 0; i < M; i += 2 ) {
      lsp[i] = ( lsp_mid[i] + lsp_new[i] ) * 0.5F;
      lsp[i + 1] = ( lsp_mid[i + 1] +lsp_new[i+1] )*0.5F;
   }

   /* Subframe 3 */
   Lsp_Az( lsp, az );
   return;
}


/*
 * Lsp_lsf
 *
 *
 * Parameters:
 *    lsp               I: LSP vector
 *    lsf               O: LSF vector
 *
 * Function:
 *    Transformation lsp to lsf, LPC order M
 *
 * Returns:
 *    void
 */
static void Lsp_lsf( Float32 lsp[], Float32 lsf[] )
{
   Word32 i;


   for ( i = 0; i < M; i++ ) {
      lsf[i] = ( Float32 )( acos( lsp[i] )*SCALE_LSP_FREQ );
   }
   return;
}


/*
 * Lsf_wt
 *
 *
 * Parameters:
 *    lsf               I: LSF vector
 *    wf                O: square of weighting factors
 *
 * Function:
 *    Compute LSF weighting factors
 *
 * Returns:
 *    void
 */
static void Lsf_wt( Float32 *lsf, Float32 *wf )
{
   Float32 temp;
   Word32 i;


   wf[0] = lsf[1];

   for ( i = 1; i < 9; i++ ) {
      wf[i] = lsf[i + 1] -lsf[i - 1];
   }
   wf[9] = 4000.0F - lsf[8];

   for ( i = 0; i < 10; i++ ) {
      if ( wf[i] < 450.0F ) {
         temp = 3.347F - SLOPE1_WGHT_LSF * wf[i];
      }
      else {
         temp = 1.8F - SLOPE2_WGHT_LSF * ( wf[i] - 450.0F );
      }
      wf[i] = temp * temp;
   }
   return;
}


/*
 * Vq_subvec
 *
 *
 * Parameters:
 *    lsf_r1            I: 1st LSF residual vector
 *    lsf_r2            I: 2nd LSF residual vector
 *    dico              I: quantization codebook
 *    wf1               I: 1st LSF weighting factors
 *    wf2               I: 2nd LSF weighting factors
 *    dico_size         I: size of quantization codebook
 * Function:
 *    Quantization of a 4 dimensional subvector
 *
 * Returns:
 *    index             quantization index
 */
static Word16 Vq_subvec( Float32 *lsf_r1, Float32 *lsf_r2, const Float32 *dico,
      Float32 *wf1, Float32 *wf2, Word16 dico_size )
{
   Float64 temp, dist, dist_min;
   const Float32 *p_dico;
   Word32 i, index = 0;


   dist_min = DBL_MAX;
   p_dico = dico;

   for ( i = 0; i < dico_size; i++ ) {
      temp = lsf_r1[0] - *p_dico++;
      dist = temp * temp * wf1[0];
      temp = lsf_r1[1] - *p_dico++;
      dist += temp * temp * wf1[1];
      temp = lsf_r2[0] - *p_dico++;
      dist += temp * temp * wf2[0];
      temp = lsf_r2[1] - *p_dico++;
      dist += temp * temp * wf2[1];

      if ( dist < dist_min ) {
         dist_min = dist;
         index = i;
      }
   }

   /* Reading the selected vector */
   p_dico = &dico[index << 2];
   lsf_r1[0] = *p_dico++;
   lsf_r1[1] = *p_dico++;
   lsf_r2[0] = *p_dico++;
   lsf_r2[1] = *p_dico++;
   return( Word16 )index;
}


/*
 * Vq_subvec_s
 *
 *
 * Parameters:
 *    lsf_r1            I: 1st LSF residual vector
 *    lsf_r2            I: 2nd LSF residual vector
 *    dico              I: quantization codebook
 *    wf1               I: 1st LSF weighting factors
 *    wf2               I: 2nd LSF weighting factors
 *    dico_size         I: size of quantization codebook
 * Function:
 *    Quantization of a 4 dimensional subvector with a signed codebook
 *
 * Returns:
 *    index             quantization index
 */
static Word16 Vq_subvec_s( Float32 *lsf_r1, Float32 *lsf_r2, const Float32 *dico
      , Float32 *wf1, Float32 *wf2, Word16 dico_size )
{
   Float64 dist_min, dist1, dist2, temp1, temp2;
   const Float32 *p_dico;
   Word32 i, index = 0;
   Word16 sign = 0;


   dist_min = DBL_MAX;
   p_dico = dico;

   for ( i = 0; i < dico_size; i++ ) {
      temp1 = lsf_r1[0] - *p_dico;
      temp2 = lsf_r1[0] + *p_dico++;
      dist1 = temp1 * temp1 * wf1[0];
      dist2 = temp2 * temp2 * wf1[0];
      temp1 = lsf_r1[1] - *p_dico;
      temp2 = lsf_r1[1] + *p_dico++;
      dist1 += temp1 * temp1 * wf1[1];
      dist2 += temp2 * temp2 * wf1[1];
      temp1 = lsf_r2[0] - *p_dico;
      temp2 = lsf_r2[0] + *p_dico++;
      dist1 += temp1 * temp1 * wf2[0];
      dist2 += temp2 * temp2 * wf2[0];
      temp1 = lsf_r2[1] - *p_dico;
      temp2 = lsf_r2[1] + *p_dico++;
      dist1 += temp1 * temp1 * wf2[1];
      dist2 += temp2 * temp2 * wf2[1];

      if ( dist1 < dist_min ) {
         dist_min = dist1;
         index = i;
         sign = 0;
      }

      if ( dist2 < dist_min ) {
         dist_min = dist2;
         index = i;
         sign = 1;
      }
   }

   /* Reading the selected vector */
   p_dico = &dico[index << 2];

   if ( sign == 0 ) {
      lsf_r1[0] = *p_dico++;
      lsf_r1[1] = *p_dico++;
      lsf_r2[0] = *p_dico++;
      lsf_r2[1] = *p_dico++;
   }
   else {
      lsf_r1[0] = -( *p_dico++ );
      lsf_r1[1] = -( *p_dico++ );
      lsf_r2[0] = -( *p_dico++ );
      lsf_r2[1] = -( *p_dico++ );
   }
   index = index << 1;
   index = index + sign;
   return( Word16 )index;
}


/*
 * Reorder_lsf
 *
 *
 * Parameters:
 *    lsf               B: vector of LSFs
 *    min_dist          I: minimum required distance
 *
 * Function:
 *    Make sure that the LSFs are properly ordered and to keep a certain minimum
 *    distance between adjacent LSFs. LPC order = M.
 *
 * Returns:
 *    void
 */
static void Reorder_lsf( Float32 *lsf, Float32 min_dist )
{
   Float32 lsf_min;
   Word32 i;


   lsf_min = min_dist;

   for ( i = 0; i < M; i++ ) {
      if ( lsf[i] < lsf_min ) {
         lsf[i] = lsf_min;
      }
      lsf_min = lsf[i] + min_dist;
   }
}


/*
 * Lsf_lsp
 *
 *
 * Parameters:
 *    lsf               I: vector of LSFs
 *    lsp	            O: vector of LSPs
 *
 * Function:
 *    Transformation lsf to lsp, order M
 *
 * Returns:
 *    void
 */
static void Lsf_lsp( Float32 lsf[], Float32 lsp[] )
{
   Word32 i;


   for ( i = 0; i < M; i++ ) {
      lsp[i] = ( Float32 )cos( SCALE_FREQ_LSP * lsf[i] );
   }
   return;
}


/*
 * Vq_subvec3
 *
 *
 * Parameters:
 *    lsf_r1            I: 1st LSF residual vector
 *    dico              I: quantization codebook
 *    wf1               I: 1st LSF weighting factors
 *    dico_size         I: size of quantization codebook
 *    use_half          I: use every second entry in codebook
 *
 * Function:
 *    Quantization of a 3 dimensional subvector
 *
 * Returns:
 *    index             quantization index
 */
static Word16 Vq_subvec3( Float32 *lsf_r1, const Float32 *dico, Float32 *wf1,
      Word16 dico_size, Word32 use_half )
{
   Float64 dist, dist_min;
   Float32 temp;
   const Float32 *p_dico;
   Word32 i, index = 0;


   dist_min = FLT_MAX;
   p_dico = dico;

   if ( use_half == 0 ) {
      for ( i = 0; i < dico_size; i++ ) {
         temp = lsf_r1[0] - *p_dico++;
         temp *= wf1[0];
         dist = temp * temp;
         temp = lsf_r1[1] - *p_dico++;
         temp *= wf1[1];
         dist += temp * temp;
         temp = lsf_r1[2] - *p_dico++;
         temp *= wf1[2];
         dist += temp * temp;

         if ( dist < dist_min ) {
            dist_min = dist;
            index = i;
         }
      }
      p_dico = &dico[( 3 * index )];
   }
   else {
      for ( i = 0; i < dico_size; i++ ) {
         temp = lsf_r1[0] - *p_dico++;
         temp *= wf1[0];
         dist = temp * temp;
         temp = lsf_r1[1] - *p_dico++;
         temp *= wf1[1];
         dist += temp * temp;
         temp = lsf_r1[2] - *p_dico++;
         temp *= wf1[2];
         dist += temp * temp;

         if ( dist < dist_min ) {
            dist_min = dist;
            index = i;
         }
         p_dico = p_dico + 3;
      }
      p_dico = &dico[6 * index];
   }

   /* Reading the selected vector */
   lsf_r1[0] = *p_dico++;
   lsf_r1[1] = *p_dico++;
   lsf_r1[2] = *p_dico++;
   return( Word16 )index;
}


/*
 * Vq_subvec4
 *
 *
 * Parameters:
 *    lsf_r1            I: 1st LSF residual vector
 *    dico              I: quantization codebook
 *    wf1               I: 1st LSF weighting factors
 *    dico_size         I: size of quantization codebook
 *
 * Function:
 *    Quantization of a 4 dimensional subvector
 *
 * Returns:
 *    index             quantization index
 */
static Word16 Vq_subvec4( Float32 *lsf_r1, const Float32 *dico, Float32 *wf1,
      Word16 dico_size )
{
   Float64 dist, dist_min;
   Float32 temp;
   const Float32 *p_dico;
   Word32 i, index = 0;


   dist_min = FLT_MAX;
   p_dico = dico;

   for ( i = 0; i < dico_size; i++ ) {
      temp = lsf_r1[0] - *p_dico++;
      temp *= wf1[0];
      dist = temp * temp;
      temp = lsf_r1[1] - *p_dico++;
      temp *= wf1[1];
      dist += temp * temp;
      temp = lsf_r1[2] - *p_dico++;
      temp *= wf1[2];
      dist += temp * temp;
      temp = lsf_r1[3] - *p_dico++;
      temp *= wf1[3];
      dist += temp * temp;

      if ( dist < dist_min ) {
         dist_min = dist;
         index = i;
      }
   }

   /* Reading the selected vector */
   p_dico = &dico[index << 2];
   lsf_r1[0] = *p_dico++;
   lsf_r1[1] = *p_dico++;
   lsf_r1[2] = *p_dico++;
   lsf_r1[3] = *p_dico++;
   return( Word16 )index;
}


/*
 * Q_plsf_3
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    past_rq           B: past quantized residual
 *    lsp1              I: 1st LSP vector
 *    lsp1_q            O: quantized 1st LSP vector
 *    indice            I: quantization indices of 5 matrices and
 *                         one sign for 3rd
 *    pred_init_i       O: init index for MA prediction in DTX mode
 *
 * Function:
 *    Quantization of LSF parameters with 1st order MA prediction and
 *    split by 3 vector quantization (split-VQ)
 *
 * Returns:
 *    void
 */
static void Q_plsf_3( enum Mode mode, Float32 *past_rq, Float32 *lsp1, Float32 *
      lsp1_q, Word16 *indice, Word32 *pred_init_i )
{
   Float32 lsf1[M], wf1[M], lsf_p[M], lsf_r1[M];
   Float32 lsf1_q[M];
   Float32 pred_init_err;
   Float32 min_pred_init_err;
   Float32 temp_r1[M];
   Float32 temp_p[M];
   Word32 j, i;


   /* convert LSFs to normalize frequency domain */
   Lsp_lsf( lsp1, lsf1 );

   /* compute LSF weighting factors */
   Lsf_wt( lsf1, wf1 );

   /* Compute predicted LSF and prediction error */
   if ( mode != MRDTX ) {
      for ( i = 0; i < M; i++ ) {
         lsf_p[i] = mean_lsf_3[i] + past_rq[i] * pred_fac[i];
         lsf_r1[i] = lsf1[i] - lsf_p[i];
      }
   }
   else {
      /*
       * DTX mode, search the init vector that yields
       * lowest prediction resuidual energy
       */
      *pred_init_i = 0;
      min_pred_init_err = FLT_MAX;

      for ( j = 0; j < PAST_RQ_INIT_SIZE; j++ ) {
         pred_init_err = 0;

         for ( i = 0; i < M; i++ ) {
            temp_p[i] = mean_lsf_3[i] + past_rq_init[j * M + i];
            temp_r1[i] = lsf1[i] - temp_p[i];
            pred_init_err += temp_r1[i] * temp_r1[i];
         }   /* next i */

         if ( pred_init_err < min_pred_init_err ) {
            min_pred_init_err = pred_init_err;
            memcpy( lsf_r1, temp_r1, M <<2 );
            memcpy( lsf_p, temp_p, M <<2 );
            memcpy( past_rq, &past_rq_init[j * M], M <<2 );
            *pred_init_i = j;
         }
      }
   }

   /* Split-VQ of prediction error */
   /* MR475, MR515 */
   if ( ( mode == MR475 ) || ( mode == MR515 ) ) {
      indice[0] = Vq_subvec3( &lsf_r1[0], dico1_lsf_3, &wf1[0], DICO1_SIZE_3, 0
            );
      indice[1] = Vq_subvec3( &lsf_r1[3], dico2_lsf_3, &wf1[3], DICO2_SIZE_3 /2,
            1 );
      indice[2] = Vq_subvec4( &lsf_r1[6], mr515_3_lsf, &wf1[6], MR515_3_SIZE );
   }

   /* MR795 */
   else if ( mode == MR795 ) {
      indice[0] = Vq_subvec3( &lsf_r1[0], mr795_1_lsf, &wf1[0], MR795_1_SIZE, 0
            );
      indice[1] = Vq_subvec3( &lsf_r1[3], dico2_lsf_3, &wf1[3], DICO2_SIZE_3, 0
            );
      indice[2] = Vq_subvec4( &lsf_r1[6], dico3_lsf_3, &wf1[6], DICO3_SIZE_3 );
   }

   /* MR59, MR67, MR74, MR102 , MRDTX */
   else {
      indice[0] = Vq_subvec3( &lsf_r1[0], dico1_lsf_3, &wf1[0], DICO1_SIZE_3, 0
            );
      indice[1] = Vq_subvec3( &lsf_r1[3], dico2_lsf_3, &wf1[3], DICO2_SIZE_3, 0
            );
      indice[2] = Vq_subvec4( &lsf_r1[6], dico3_lsf_3, &wf1[6], DICO3_SIZE_3 );
   }

   /* Compute quantized LSFs and update the past quantized residual */
   for ( i = 0; i < M; i++ ) {
      lsf1_q[i] = lsf_r1[i] + lsf_p[i];
      past_rq[i] = lsf_r1[i];
   }

   /* verification that LSFs has mimimum distance of LSF_GAP 50 Hz */
   Reorder_lsf( lsf1_q, 50.0F );

   /*  convert LSFs to the cosine domain */
   Lsf_lsp( lsf1_q, lsp1_q );
}


/*
 * Q_plsf_5
 *
 *
 * Parameters:
 *    past_rq           B: past quantized residual
 *    lsp1              I: 1st LSP vector
 *    lsp2              I: 2nd LSP vector
 *    lsp1_q            O: quantized 1st LSP vector
 *    lsp2_q            O: quantized 2nd LSP vector
 *    indice	         I: quantization indices of 5 matrices and
 *                         one sign for 3rd
 *
 * Function:
 *    Quantization of 2 sets of LSF parameters using 1st order MA
 *    prediction and split by 5 matrix quantization (split-MQ).
 *
 * Returns:
 *    void
 */
static void Q_plsf_5( Float32 *past_rq, Float32 *lsp1, Float32 *lsp2, Float32 *
      lsp1_q, Float32 *lsp2_q, Word16 *indice )
{
   Float32 lsf1[M], lsf2[M], wf1[M], wf2[M], lsf_p[M], lsf_r1[M], lsf_r2[M];
   Float32 lsf1_q[M], lsf2_q[M];
   Word32 i;


   /* convert LSFs to normalize frequency domain */
   Lsp_lsf( lsp1, lsf1 );
   Lsp_lsf( lsp2, lsf2 );

   /* Compute LSF weighting factors */
   Lsf_wt( lsf1, wf1 );
   Lsf_wt( lsf2, wf2 );

   /* Compute predicted LSF and prediction error */
   for ( i = 0; i < M; i++ ) {
      /* MR122 LSP prediction factor = 0.65 */
      lsf_p[i] = mean_lsf_5[i] + past_rq[i] * 0.65F;
      lsf_r1[i] = lsf1[i] - lsf_p[i];
      lsf_r2[i] = lsf2[i] - lsf_p[i];
   }

   /* Split-MQ of prediction error */
   indice[0] = Vq_subvec( &lsf_r1[0], &lsf_r2[0], dico1_lsf_5, &wf1[0], &wf2[0],
         DICO1_SIZE_5 );
   indice[1] = Vq_subvec( &lsf_r1[2], &lsf_r2[2], dico2_lsf_5, &wf1[2], &wf2[2],
         DICO2_SIZE_5 );
   indice[2] = Vq_subvec_s( &lsf_r1[4], &lsf_r2[4], dico3_lsf_5, &wf1[4], &wf2[4
         ], DICO3_SIZE_5 );
   indice[3] = Vq_subvec( &lsf_r1[6], &lsf_r2[6], dico4_lsf_5, &wf1[6], &wf2[6],
         DICO4_SIZE_5 );
   indice[4] = Vq_subvec( &lsf_r1[8], &lsf_r2[8], dico5_lsf_5, &wf1[8], &wf2[8],
         DICO5_SIZE_5 );

   /* Compute quantized LSFs and update the past quantized residual */
   for ( i = 0; i < M; i++ ) {
      lsf1_q[i] = lsf_r1[i] + lsf_p[i];
      lsf2_q[i] = lsf_r2[i] + lsf_p[i];
      past_rq[i] = lsf_r2[i];
   }

   /* verification that LSFs has minimum distance of LSF_GAP 50hz */
   Reorder_lsf( lsf1_q, 50.0F );
   Reorder_lsf( lsf2_q, 50.0F );

   /*  convert LSFs to the cosine domain */
   Lsf_lsp( lsf1_q, lsp1_q );
   Lsf_lsp( lsf2_q, lsp2_q );
}


/*
 * Int_lpc_1and3
 *
 *
 * Parameters:
 *    lsp_old        I: LSP vector at the 4th subfr. of past frame      [M]
 *    lsp_mid        I: LSP vector at the 2nd subframe of present frame [M]
 *    lsp_new        I: LSP vector at the 4th subframe of present frame [M]
 *    az             O: interpolated LP parameters in subframes 1 and 3
 *                                                                [AZ_SIZE]
 *
 * Function:
 *    Interpolates the LSPs and converts to LPC parameters
 *    to get a different LP filter in each subframe.
 *
 *    The 20 ms speech frame is divided into 4 subframes.
 *    The LSPs are quantized and transmitted at the 2nd and
 *    4th subframes (twice per frame) and interpolated at the
 *    1st and 3rd subframe.
 *
 * Returns:
 *    void
 */
static void Int_lpc_1and3( Float32 lsp_old[], Float32 lsp_mid[], Float32 lsp_new
      [], Float32 az[] )
{
   Word32 i;
   Float32 lsp[M];


   for ( i = 0; i < M; i++ ) {
      lsp[i] = ( lsp_mid[i] + lsp_old[i] ) * 0.5F;
   }

   /* Subframe 1 */
   Lsp_Az( lsp, az );
   az += MP1;

   /* Subframe 2 */
   Lsp_Az( lsp_mid, az );
   az += MP1;

   for ( i = 0; i < M; i++ ) {
      lsp[i] = ( lsp_mid[i] + lsp_new[i] ) * 0.5F;
   }

   /* Subframe 3 */
   Lsp_Az( lsp, az );
   az += MP1;

   /* Subframe 4 */
   Lsp_Az( lsp_new, az );
   return;
}


/*
 * Int_lpc_1to3_2
 *
 *
 * Parameters:
 *    lsp_old           I: LSP vector at the 4th subfr. of past frame      [M]
 *    lsp_new           I: LSP vector at the 4th subframe of present frame [M]
 *    az                O: interpolated LP parameters in subframes 1, 2 and 3
 *                                                                   [AZ_SIZE]
 *
 * Function:
 *    Interpolation of the LPC parameters.
 *
 * Returns:
 *    void
 */
static void Int_lpc_1to3_2( Float32 lsp_old[], Float32 lsp_new[], Float32 az[] )
{
   Float32 lsp[M];
   Word32 i;


   for ( i = 0; i < M; i += 2 ) {
      lsp[i] = lsp_new[i] * 0.25F + lsp_old[i] * 0.75F;
      lsp[i + 1] = lsp_new[i + 1] *0.25F + lsp_old[i + 1] *0.75F;
   }

   /* Subframe 1 */
   Lsp_Az( lsp, az );
   az += MP1;

   for ( i = 0; i < M; i += 2 ) {
      lsp[i] = ( lsp_old[i] + lsp_new[i] ) * 0.5F;
      lsp[i + 1] = ( lsp_old[i + 1] +lsp_new[i+1] )*0.5F;
   }

   /* Subframe 2 */
   Lsp_Az( lsp, az );
   az += MP1;

   for ( i = 0; i < M; i += 2 ) {
      lsp[i] = lsp_old[i] * 0.25F + lsp_new[i] * 0.75F;
      lsp[i + 1] = lsp_old[i + 1] *0.25F + lsp_new[i + 1] *0.75F;
   }

   /* Subframe 3 */
   Lsp_Az( lsp, az );
   return;
}


/*
 * Int_lpc_1to3
 *
 *
 * Parameters:
 *    lsp_old           I: LSP vector at the 4th subfr. of past frame      [M]
 *    lsp_new           I: LSP vector at the 4th subframe of present frame [M]
 *    az                O: interpolated LP parameters in all subframes
 *                                                                   [AZ_SIZE]
 *
 * Function:
 *    Interpolates the LSPs and converts to LPC parameters to get a different
 *    LP filter in each subframe.
 *
 *    The 20 ms speech frame is divided into 4 subframes.
 *    The LSPs are quantized and transmitted at the 4th
 *    subframes (once per frame) and interpolated at the
 *    1st, 2nd and 3rd subframe.
 *
 * Returns:
 *    void
 */
static void Int_lpc_1to3( Float32 lsp_old[], Float32 lsp_new[], Float32 az[] )
{
   Float32 lsp[M];
   Word32 i;


   for ( i = 0; i < M; i++ ) {
      lsp[i] = lsp_new[i] * 0.25F + lsp_old[i] * 0.75F;
   }

   /* Subframe 1 */
   Lsp_Az( lsp, az );
   az += MP1;

   for ( i = 0; i < M; i++ ) {
      lsp[i] = ( lsp_old[i] + lsp_new[i] ) * 0.5F;
   }

   /* Subframe 2 */
   Lsp_Az( lsp, az );
   az += MP1;

   for ( i = 0; i < M; i++ ) {
      lsp[i] = lsp_old[i] * 0.25F + lsp_new[i] * 0.75F;
   }

   /* Subframe 3 */
   Lsp_Az( lsp, az );
   az += MP1;

   /* Subframe 4 */
   Lsp_Az( lsp_new, az );
   return;
}


/*
 * lsp
 *
 *
 * Parameters:
 *    req_mode          I: requested mode
 *    used_mode         I: used mode
 *    lsp_old           B: old LSP vector
 *    lsp_old_q         B: old quantized LSP vector
 *    past_rq           B: past quantized residual
 *    az                B: interpolated LP parameters
 *    azQ               O: quantization interpol. LP parameters
 *    lsp_new           O: new lsp vector
 *    anap              O: analysis parameters
 *
 * Function:
 *    From A(z) to lsp. LSP quantization and interpolation
 *
 * Returns:
 *    void
 */
static void lsp( enum Mode req_mode, enum Mode used_mode, Float32 *lsp_old,
      Float32 *lsp_old_q, Float32 *past_rq, Float32 az[], Float32 azQ[], Float32
      lsp_new[], Word16 **anap )
{
   Float32 lsp_new_q[M];   /* LSPs at 4th subframe */
   Float32 lsp_mid[M], lsp_mid_q[M];   /* LSPs at 2nd subframe */
   Word32 pred_init_i;   /* init index for MA prediction in DTX mode */


   if ( req_mode == MR122 ) {
      Az_lsp( &az[MP1], lsp_mid, lsp_old );
      Az_lsp( &az[MP1 * 3], lsp_new, lsp_mid );

      /*
       * Find interpolated LPC parameters in all subframes
       * (both quantized and unquantized).
       * The interpolated parameters are in array A_t[] of size (M+1)*4
       * and the quantized interpolated parameters are in array Aq_t[]
       */
      Int_lpc_1and3_2( lsp_old, lsp_mid, lsp_new, az );

      if ( used_mode != MRDTX ) {
         /* LSP quantization (lsp_mid[] and lsp_new[] jointly quantized) */
         Q_plsf_5( past_rq, lsp_mid, lsp_new, lsp_mid_q, lsp_new_q, *anap );
         Int_lpc_1and3( lsp_old_q, lsp_mid_q, lsp_new_q, azQ );

         /* Advance analysis parameters pointer */
         ( *anap ) += 5;
      }
   }
   else {
      /* From A(z) to lsp */
      Az_lsp( &az[MP1 * 3], lsp_new, lsp_old );

      /*
       * Find interpolated LPC parameters in all subframes
       * (both quantized and unquantized).
       * The interpolated parameters are in array A_t[] of size (M+1)*4
       * and the quantized interpolated parameters are in array Aq_t[]
       */
      Int_lpc_1to3_2( lsp_old, lsp_new, az );

      /* LSP quantization */
      if ( used_mode != MRDTX ) {
         Q_plsf_3( req_mode, past_rq, lsp_new, lsp_new_q, *anap, &pred_init_i );
         Int_lpc_1to3( lsp_old_q, lsp_new_q, azQ );

         /* Advance analysis parameters pointer */
         ( *anap ) += 3;
      }
   }

   /* update the LSPs for the next frame */
   memcpy( lsp_old, lsp_new, M <<2 );
   memcpy( lsp_old_q, lsp_new_q, M <<2 );
}


/*
 * check_lsp
 *
 *
 * Parameters:
 *    count          B: counter for resonance
 *    lsp            B: LSP vector
 *
 * Function:
 *    Check the LSP's to detect resonances
 *
 *    Resonances in the LPC filter are monitored to detect possible problem
 *    areas where divergence between the adaptive codebook memories in
 *    the encoder and the decoder could cause unstable filters in areas
 *    with highly correlated continuos signals. Typically, this divergence
 *    is due to channel errors.
 *    The monitoring of resonance signals is performed using unquantized LSPs
 *    q(i), i = 1,...,10. The algorithm utilises the fact that LSPs are
 *    closely located at a peak in the spectrum. First, two distances,
 *    dist 1 and dist 2 ,are calculated in two different regions,
 *    defined as
 *
 *    dist1 = min[q(i) - q(i + 1)],  i = 4,...,8
 *    dist2 = min[q(i) - q(i + 1)],  i = 2,3
 *
 *    Either of these two minimum distance conditions must be fulfilled
 *    to classify the frame as a resonance frame and increase the resonance
 *    counter.
 *
 *    if(dist1 < TH1) || if (dist2 < TH2)
 *       counter++
 *    else
 *       counter = 0
 *
 *    TH1 = 0.046
 *    TH2 = 0.018, q(2) > 0.98
 *    TH2 = 0.024, 0.93 < q(2) <= 0.98
 *    TH2 = 0.018, otherwise
 *
 *    12 consecutive resonance frames are needed to indicate possible
 *    problem conditions, otherwise the LSP_flag is cleared.
 *
 * Returns:
 *    resonance flag
 */
static Word16 check_lsp( Word16 *count, Float32 *lsp )
{
   Float32 dist, dist_min1, dist_min2, dist_th;
   Word32 i;


   /*
    * Check for a resonance:
    * Find minimum distance between lsp[i] and lsp[i+1]
    */
   dist_min1 = FLT_MAX;

   for ( i = 3; i < 8; i++ ) {
      dist = lsp[i] - lsp[i + 1];

      if ( dist < dist_min1 ) {
         dist_min1 = dist;
      }
   }
   dist_min2 = FLT_MAX;

   for ( i = 1; i < 3; i++ ) {
      dist = lsp[i] - lsp[i + 1];

      if ( dist < dist_min2 ) {
         dist_min2 = dist;
      }
   }

   if ( lsp[1] > 0.98F ) {
      dist_th = 0.018F;
   }
   else if ( lsp[1] > 0.93F ) {
      dist_th = 0.024F;
   }
   else {
      dist_th = 0.034F;
   }

   if ( ( dist_min1 < 0.046F ) || ( dist_min2 < dist_th ) ) {
      *count += 1;
   }
   else {
      *count = 0;
   }

   /* Need 12 consecutive frames to set the flag */
   if ( *count >= 12 ) {
      *count = 12;
      return 1;
   }
   else {
      return 0;
   }
}


/*
 * Weight_Ai
 *
 *
 * Parameters:
 *    a                 I: LPC coefficients                    [M+1]
 *    fac               I: Spectral expansion factors.         [M+1]
 *    a_exp             O: Spectral expanded LPC coefficients  [M+1]
 *
 * Function:
 *    Spectral expansion of LP coefficients
 *
 * Returns:
 *    void
 */
static void Weight_Ai( Float32 a[], const Float32 fac[], Float32 a_exp[] )
{
   Word32 i;


   a_exp[0] = a[0];

   for ( i = 1; i <= M; i++ ) {
      a_exp[i] = a[i] * fac[i - 1];
   }
   return;
}


/*
 * Residu
 *
 *
 * Parameters:
 *    a                 I: prediction coefficients
 *    x                 I: speech signal
 *    y                 O: residual signal
 *
 * Function:
 *    Computes the LTP residual signal.
 *
 * Returns:
 *    void
 */
static void Residu( Float32 a[], Float32 x[], Float32 y[] )
{
   Float32 s;
   Word32 i;


   for ( i = 0; i < L_SUBFR; i += 4 ) {
      s = x[i] * a[0];
      s += x[i - 1] *a[1];
      s += x[i - 2] * a[2];
      s += x[i - 3] * a[3];
      s += x[i - 4] * a[4];
      s += x[i - 5] * a[5];
      s += x[i - 6] * a[6];
      s += x[i - 7] * a[7];
      s += x[i - 8] * a[8];
      s += x[i - 9] * a[9];
      s += x[i - 10] * a[10];
      y[i] = s;
      s = x[i + 1] *a[0];
      s += x[i] * a[1];
      s += x[i - 1] *a[2];
      s += x[i - 2] * a[3];
      s += x[i - 3] * a[4];
      s += x[i - 4] * a[5];
      s += x[i - 5] * a[6];
      s += x[i - 6] * a[7];
      s += x[i - 7] * a[8];
      s += x[i - 8] * a[9];
      s += x[i - 9] * a[10];
      y[i + 1] = s;
      s = x[i + 2] * a[0];
      s += x[i + 1] *a[1];
      s += x[i] * a[2];
      s += x[i - 1] *a[3];
      s += x[i - 2] * a[4];
      s += x[i - 3] * a[5];
      s += x[i - 4] * a[6];
      s += x[i - 5] * a[7];
      s += x[i - 6] * a[8];
      s += x[i - 7] * a[9];
      s += x[i - 8] * a[10];
      y[i + 2] = s;
      s = x[i + 3] * a[0];
      s += x[i + 2] * a[1];
      s += x[i + 1] *a[2];
      s += x[i] * a[3];
      s += x[i - 1] *a[4];
      s += x[i - 2] * a[5];
      s += x[i - 3] * a[6];
      s += x[i - 4] * a[7];
      s += x[i - 5] * a[8];
      s += x[i - 6] * a[9];
      s += x[i - 7] * a[10];
      y[i + 3] = s;
   }
   return;
}


/*
 * Syn_filt
 *
 *
 * Parameters:
 *    a                 I: prediction coefficients [M+1]
 *    x                 I: input signal
 *    y                 O: output signal
 *    mem               B: memory associated with this filtering
 *    update            I: 0=no update, 1=update of memory.
 *
 * Function:
 *    Perform synthesis filtering through 1/A(z).
 *
 * Returns:
 *    void
 */
static void Syn_filt( Float32 a[], Float32 x[], Float32 y[], Float32 mem[],
      Word16 update )
{
   Float64 tmp[50];
   Float64 sum;
   Float64 *yy;
   Word32 i;


   /* Copy mem[] to yy[] */
   yy = tmp;

   for ( i = 0; i < M; i++ ) {
      *yy++ = mem[i];
   }

   /* Do the filtering. */
   for ( i = 0; i < L_SUBFR; i = i + 4 ) {
      sum = x[i] * a[0];
      sum -= a[1] * yy[ - 1];
      sum -= a[2] * yy[ - 2];
      sum -= a[3] * yy[ - 3];
      sum -= a[4] * yy[ - 4];
      sum -= a[5] * yy[ - 5];
      sum -= a[6] * yy[ - 6];
      sum -= a[7] * yy[ - 7];
      sum -= a[8] * yy[ - 8];
      sum -= a[9] * yy[ - 9];
      sum -= a[10] * yy[ - 10];
      *yy++ = sum;
      y[i] = ( Float32 )yy[ - 1];
      sum = x[i + 1] *a[0];
      sum -= a[1] * yy[ - 1];
      sum -= a[2] * yy[ - 2];
      sum -= a[3] * yy[ - 3];
      sum -= a[4] * yy[ - 4];
      sum -= a[5] * yy[ - 5];
      sum -= a[6] * yy[ - 6];
      sum -= a[7] * yy[ - 7];
      sum -= a[8] * yy[ - 8];
      sum -= a[9] * yy[ - 9];
      sum -= a[10] * yy[ - 10];
      *yy++ = sum;
      y[i + 1] = ( Float32 )yy[ - 1];
      sum = x[i + 2] * a[0];
      sum -= a[1] * yy[ - 1];
      sum -= a[2] * yy[ - 2];
      sum -= a[3] * yy[ - 3];
      sum -= a[4] * yy[ - 4];
      sum -= a[5] * yy[ - 5];
      sum -= a[6] * yy[ - 6];
      sum -= a[7] * yy[ - 7];
      sum -= a[8] * yy[ - 8];
      sum -= a[9] * yy[ - 9];
      sum -= a[10] * yy[ - 10];
      *yy++ = sum;
      y[i + 2] = ( Float32 )yy[ - 1];
      sum = x[i + 3] * a[0];
      sum -= a[1] * yy[ - 1];
      sum -= a[2] * yy[ - 2];
      sum -= a[3] * yy[ - 3];
      sum -= a[4] * yy[ - 4];
      sum -= a[5] * yy[ - 5];
      sum -= a[6] * yy[ - 6];
      sum -= a[7] * yy[ - 7];
      sum -= a[8] * yy[ - 8];
      sum -= a[9] * yy[ - 9];
      sum -= a[10] * yy[ - 10];
      *yy++ = sum;
      y[i + 3] = ( Float32 )yy[ - 1];
   }

   /* Update of memory if update==1 */
   if ( update != 0 ) {
      for ( i = 0; i < M; i++ ) {
         mem[i] = y[30 + i];
      }
   }
   return;
}


/*
 * pre_big
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    gamma1            I: spectral exp. factor 1
 *    gamma1_12k2       I: spectral exp. factor 1 for modes above MR795
 *    gamma2            I: spectral exp. factor 2
 *    A_t               I: A(z) unquantized, for 4 subframes
 *    frame_offset      I: frameoffset, 1st or second big_sbf
 *    speech            I: speech
 *    mem_w             B: synthesis filter memory state
 *    wsp               O: weighted speech
 *
 * Function:
 *    Big subframe (2 subframes) preprocessing
 *
 *    Open-loop pitch analysis is performed in order to simplify the pitch
 *    analysis and confine the closed-loop pitch search to a small number of
 *    lags around the open-loop estimated lags.
 *    Open-loop pitch estimation is based on the weighted speech signal Sw(n)
 *    which is obtained by filtering the input speech signal through
 *    the weighting filter
 *
 *    W(z) = A(z/g1) / A(z/g2)
 *
 *    That is, in a subframe of size L, the weighted speech is given by:
 *
 *                    10                           10
 *    Sw(n) = S(n) + SUM[a(i) * g1(i) * S(n-i)] - SUM[a(i) * g2(i) * Sw(n-i)],
 *                   i=1                          i=1
 *    n = 0, ..., L-1
 *
 * Returns:
 *    void
 */
static Word32 pre_big( enum Mode mode, const Float32 gamma1[], const Float32
      gamma1_12k2[], const Float32 gamma2[], Float32 A_t[], Word16 frame_offset,
      Float32 speech[], Float32 mem_w[], Float32 wsp[] )
{
   Float32 Ap1[MP1], Ap2[MP1];
   Word32 offset, i;


   /* A(z) with spectral expansion */
   const Float32 *g1;


   g1 = gamma1_12k2;

   if ( mode <= MR795 ) {
      g1 = gamma1;
   }
   offset = 0;

   if ( frame_offset > 0 ) {
      offset = MP1 << 1;
   }

   /* process two subframes (which form the "big" subframe) */
   for ( i = 0; i < 2; i++ ) {
      /* a(i) * g1(i) */
      Weight_Ai( &A_t[offset], g1, Ap1 );

      /* a(i) * g2(i) */
      Weight_Ai( &A_t[offset], gamma2, Ap2 );

      /*
       *       10
       *  S(n) + SUM[a(i) * g1(i) * S(n-i)]
       *       i=1
       */
      Residu( Ap1, &speech[frame_offset], &wsp[frame_offset] );

      /*
       *          10                            10
       *  S(n) + SUM[a(i) * g1(i) * S(n-i)]    SUM[a(i) * g2(i) * Sn(n-i)]
       *         i=1                           i=1
       */
      Syn_filt( Ap2, &wsp[frame_offset], &wsp[frame_offset], mem_w, 1 );
      offset += MP1;
      frame_offset += L_SUBFR;
   }
   return 0;
}


/*
 * comp_corr
 *
 *
 * Parameters:
 *    sig               I: signal
 *    L_frame           I: length of frame to compute pitch
 *    lag_max           I: maximum lag
 *    lag_min           I: minimum lag
 *    corr              O: correlation of selected lag
 *
 * Function:
 *    Calculate all correlations in a given delay range.
 *
 * Returns:
 *    void
 */
static void comp_corr( Float32 sig[], Word32 L_frame, Word32 lag_max, Word32
      lag_min, Float32 corr[] )
{
   Word32 i, j;
   Float32 *p, *p1;
   Float32 T0;


   for ( i = lag_max; i >= lag_min; i-- ) {
      p = sig;
      p1 = &sig[ - i];
      T0 = 0.0F;

      for ( j = 0; j < L_frame; j = j + 40, p += 40, p1 += 40 ) {
         T0 += p[0] * p1[0] + p[1] * p1[1] + p[2] * p1[2] + p[3] * p1[3];
         T0 += p[4] * p1[4] + p[5] * p1[5] + p[6] * p1[6] + p[7] * p1[7];
         T0 += p[8] * p1[8] + p[9] * p1[9] + p[10] * p1[10] + p[11] * p1[11];
         T0 += p[12] * p1[12] + p[13] * p1[13] + p[14] * p1[14] + p[15] * p1[15]
         ;
         T0 += p[16] * p1[16] + p[17] * p1[17] + p[18] * p1[18] + p[19] * p1[19]
         ;
         T0 += p[20] * p1[20] + p[21] * p1[21] + p[22] * p1[22] + p[23] * p1[23]
         ;
         T0 += p[24] * p1[24] + p[25] * p1[25] + p[26] * p1[26] + p[27] * p1[27]
         ;
         T0 += p[28] * p1[28] + p[29] * p1[29] + p[30] * p1[30] + p[31] * p1[31]
         ;
         T0 += p[32] * p1[32] + p[33] * p1[33] + p[34] * p1[34] + p[35] * p1[35]
         ;
         T0 += p[36] * p1[36] + p[37] * p1[37] + p[38] * p1[38] + p[39] * p1[39]
         ;
      }
      corr[ - i] = T0;
   }
   return;
}


/*
 * vad_tone_detection
 *
 *
 * Parameters:
 *    st->tone          B: flags indicating presence of a tone
 *    T0                I: autocorrelation maxima
 *    t1                I: energy
 *
 * Function:
 *    Set tone flag if pitch gain is high.
 *    This is used to detect signaling tones and other signals
 *    with high pitch gain.
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void vad_tone_detection( vadState *st, Float32 T0, Float32 t1 )
{
   if ( ( t1 > 0 ) && ( T0 > t1 * TONE_THR ) ) {
      st->tone = st->tone | 0x00004000;
   }
}
#endif

/*
 * Lag_max
 *
 *
 * Parameters:
 *    vadSt          B: vad structure
 *    corr           I: correlation vector
 *    sig            I: signal
 *    L_frame        I: length of frame to compute pitch
 *    lag_max        I: maximum lag
 *    lag_min        I: minimum lag
 *    cor_max        O: maximum correlation
 *    dtx            I: dtx on/off
 *
 * Function:
 *    Compute the open loop pitch lag.
 *
 * Returns:
 *    p_max             lag found
 */
#ifdef VAD2
static Word16 Lag_max( Float32 corr[], Float32 sig[], Word16 L_frame,
		       Word32 lag_max, Word32 lag_min, Float32 *cor_max,
		       Word32 dtx, Float32 *rmax, Float32 *r0 )
#else
static Word16 Lag_max( vadState *vadSt, Float32 corr[], Float32 sig[], Word16
      L_frame, Word32 lag_max, Word32 lag_min, Float32 *cor_max, Word32 dtx )
#endif
{
   Float32 max, T0;
   Float32 *p;
   Word32 i, j, p_max;


   max = -FLT_MAX;
   p_max = lag_max;

   for ( i = lag_max, j = ( PIT_MAX - lag_max - 1 ); i >= lag_min; i--, j-- ) {
      if ( corr[ - i] >= max ) {
         max = corr[ - i];
         p_max = i;
      }
   }

   /* compute energy for normalization */
   T0 = 0.0F;
   p = &sig[ - p_max];

   for ( i = 0; i < L_frame; i++, p++ ) {
      T0 += *p * *p;
   }

   if ( dtx ) {
#ifdef VAD2
     *rmax = max;
     *r0 = T0;
#else
     /* check tone */
     vad_tone_detection( vadSt, max, T0 );
#endif
   }

   if ( T0 > 0.0F )
      T0 = 1.0F / ( Float32 )sqrt( T0 );
   else
      T0 = 0.0F;

   /* max = max/sqrt(energy) */
   max *= T0;
   *cor_max = max;
   return( ( Word16 )p_max );
}


/*
 * hp_max
 *
 *
 * Parameters:
 *    corr           I: correlation vector
 *    sig            I: signal
 *    L_frame        I: length of frame to compute pitch
 *    lag_max        I: maximum lag
 *    lag_min        I: minimum lag
 *    cor_hp_max     O: max high-pass filtered correlation
 *
 * Function:
 *    Find the maximum correlation of scal_sig[] in a given delay range.
 *
 *    The correlation is given by
 *       cor[t] = <scal_sig[n],scal_sig[n-t]>,  t=lag_min,...,lag_max
 *    The functions outputs the maximum correlation after normalization
 *    and the corresponding lag.
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void hp_max( Float32 corr[], Float32 sig[], Word32 L_frame, Word32
      lag_max, Word32 lag_min, Float32 *cor_hp_max )
{
   Float32 T0, t1, max;
   Float32 *p, *p1;
   Word32 i;


   max = -FLT_MAX;
   T0 = 0;

   for ( i = lag_max - 1; i > lag_min; i-- ) {
      /* high-pass filtering */
      T0 = ( ( corr[ - i] * 2 ) - corr[ - i-1] )-corr[ - i + 1];
      T0 = ( Float32 )fabs( T0 );

      if ( T0 >= max ) {
         max = T0;
      }
   }

   /* compute energy */
   p = sig;
   p1 = &sig[0];
   T0 = 0;

   for ( i = 0; i < L_frame; i++, p++, p1++ ) {
      T0 += *p * *p1;
   }
   p = sig;
   p1 = &sig[ - 1];
   t1 = 0;

   for ( i = 0; i < L_frame; i++, p++, p1++ ) {
      t1 += *p * *p1;
   }

   /* high-pass filtering */
   T0 = T0 - t1;
   T0 = ( Float32 )fabs( T0 );

   /* max/T0 */
   if ( T0 != 0 ) {
      *cor_hp_max = max / T0;
   }
   else {
      *cor_hp_max = 0;
   }
}
#endif

/*
 * vad_tone_detection_update
 *
 *
 * Parameters:
 *    st->tone          B: flags indicating presence of a tone
 *    one_lag_per_frame I: 1 open-loop lag is calculated per each frame
 *
 * Function:
 *    Update the tone flag register.
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void vad_tone_detection_update( vadState *st, Word16 one_lag_per_frame )
{
   /* Shift tone flags right by one bit */
   st->tone = st->tone >> 1;

   /*
    * If open-loop lag is calculated only once in each frame,
    * do extra update and assume that the other tone flag
    * of the frame is one.
    */
   if ( one_lag_per_frame != 0 ) {
      st->tone = st->tone >> 1;
      st->tone = st->tone | 0x00002000;
   }
}
#endif

/*
 * Pitch_ol
 *
 *
 * Parameters:
 *    mode           I: AMR mode
 *    vadSt          B: VAD state struct
 *    signal         I: signal used to compute the open loop pitch
 *                                                 [[-pit_max]:[-1]]
 *    pit_min        I: minimum pitch lag
 *    pit_max        I: maximum pitch lag
 *    L_frame        I: length of frame to compute pitch
 *    dtx            I: DTX flag
 *    idx            I: frame index
 *
 * Function:
 *    Compute the open loop pitch lag.
 *
 *    Open-loop pitch analysis is performed twice per frame (each 10 ms)
 *    to find two estimates of the pitch lag in each frame.
 *    Open-loop pitch analysis is performed as follows.
 *    In the first step, 3 maxima of the correlation:
 *
 *          79
 *    O(k) = SUM Sw(n)*Sw(n-k)
 *          n=0
 *
 *    are found in the three ranges:
 *       pit_min     ...      2*pit_min-1
 *       2*pit_min   ...      4*pit_min-1
 *       4*pit_min   ...      pit_max
 *
 *    The retained maxima O(t(i)), i = 1, 2, 3, are normalized by dividing by
 *
 *    SQRT[SUM[POW(Sw(n-t(i)), 2]], i = 1, 2, 3,
 *         n
 *
 *    respectively.
 *    The normalized maxima and corresponding delays are denoted by
 *    (M(i), t(i)), i = 1, 2, 3. The winner, Top, among the three normalized
 *    correlations is selected by favouring the delays with the values
 *    in the lower range. This is performed by weighting the normalized
 *    correlations corresponding to the longer delays. The best
 *    open-loop delay Top is determined as follows:
 *
 *    Top = t(1)
 *    M(Top) = M(1)
 *    if M(2) > 0.85 * M(Top)
 *       M(Top) = M(2)
 *       Top = t(2)
 *    end
 *    if M(3) > 0.85 * M(Top)
 *       M(Top) = M(3)
 *       Top = t(3)
 *    end
 *
 * Returns:
 *    void
 */
static Word32 Pitch_ol( enum Mode mode, vadState *vadSt, Float32 signal[],
      Word32 pit_min, Word32 pit_max, Word16 L_frame, Word32 dtx, Word16 idx )
{
   Float32 corr[PIT_MAX + 1];
   Float32 max1, max2, max3, p_max1, p_max2, p_max3;
   Float32 *corr_ptr;
   Word32 i, j;
#ifdef VAD2
   Float32 r01, r02, r03;
   Float32 rmax1, rmax2, rmax3;
#else
   Float32 corr_hp_max;
#endif


#ifndef VAD2
   if ( dtx ) {
      /* update tone detection */
      if ( ( mode == MR475 ) || ( mode == MR515 ) ) {
         vad_tone_detection_update( vadSt, 1 );
      }
      else {
         vad_tone_detection_update( vadSt, 0 );
      }
   }
#endif

   corr_ptr = &corr[pit_max];

   /*        79             */
   /* O(k) = SUM Sw(n)*Sw(n-k)   */
   /*        n=0               */
   comp_corr( signal, L_frame, pit_max, pit_min, corr_ptr );

#ifdef VAD2
   /* Find a maximum for each section.	*/
   /* Maxima 1	*/
   j = pit_min << 2;
   p_max1 =
     Lag_max( corr_ptr, signal, L_frame, pit_max, j, &max1, dtx, &rmax1, &r01 );

   /* Maxima 2	*/
   i = j - 1;
   j = pit_min << 1;
   p_max2 = Lag_max( corr_ptr, signal, L_frame, i, j, &max2, dtx, &rmax2, &r02 );

   /* Maxima 3	*/
   i = j - 1;
   p_max3 =
     Lag_max( corr_ptr, signal, L_frame, i, pit_min, &max3, dtx, &rmax3, &r03 );
#else
   /* Find a maximum for each section.	*/
   /* Maxima 1	*/
   j = pit_min << 2;
   p_max1 = Lag_max( vadSt, corr_ptr, signal, L_frame, pit_max, j, &max1, dtx );

   /* Maxima 2 */
   i = j - 1;
   j = pit_min << 1;
   p_max2 = Lag_max( vadSt, corr_ptr, signal, L_frame, i, j, &max2, dtx );

   /* Maxima 3 */
   i = j - 1;
   p_max3 = Lag_max( vadSt, corr_ptr, signal, L_frame, i, pit_min, &max3, dtx );

   if ( dtx ) {
      if ( idx == 1 ) {
         /* calculate max high-passed filtered correlation of all lags */
         hp_max( corr_ptr, signal, L_frame, pit_max, pit_min, &corr_hp_max );

         /* update complex background detector */
         vadSt->best_corr_hp = corr_hp_max * 0.5F;
      }
   }
#endif

   /* The best open-loop delay */
   if ( ( max1 * 0.85F ) < max2 ) {
      max1 = max2;
      p_max1 = p_max2;
#ifdef VAD2
      if (dtx) {
	rmax1 = rmax2;
	r01 = r02;
      }
#endif
   }

   if ( ( max1 * 0.85F ) < max3 ) {
      p_max1 = p_max3;
#ifdef VAD2
      if (dtx) {
	rmax1 = rmax3;
	r01 = r03;
      }
#endif
   }
#ifdef VAD2
   if (dtx) {
     vadSt->Rmax += rmax1;   /* Save max correlation */
     vadSt->R0   += r01;     /* Save max energy */
   }
#endif
   return( Word32 )p_max1;
}


/*
 * Lag_max_wght
 *
 *
 * Parameters:
 *    vadSt          B: vad structure
 *    corr           I: correlation vector
 *    signal         I: signal
 *    L_frame        I: length of frame to compute pitch
 *    old_lag        I: old open-loop lag
 *    cor_max        O: maximum correlation
 *    wght_flg       I: weighting function flag
 *    gain_flg       O: open-loop flag
 *    dtx            I: dtx on/off
 *
 * Function:
 *    Find the lag that has maximum correlation of signal in a given delay range.
 *    maximum lag = 143
 *    minimum lag = 20
 *
 * Returns:
 *    p_max             lag found
 */
static Word32 Lag_max_wght( vadState *vadSt, Float32 corr[], Float32 signal[],
      Word32 old_lag, Word32 *cor_max, Word32 wght_flg, Float32 *gain_flg,
      Word32 dtx )
{
   Float32 t0, t1, max;
   Float32 *psignal, *p1signal;
   const Float32 *ww, *we;
   Word32 i, j, p_max;


   ww = &corrweight[250];
   we = &corrweight[266 - old_lag];
   max = -FLT_MAX;
   p_max = PIT_MAX;

   /* see if the neigbouring emphasis is used */
   if ( wght_flg > 0 ) {
      /* find maximum correlation with weighting */
      for ( i = PIT_MAX; i >= PIT_MIN; i-- ) {
         /* Weighting of the correlation function. */
         t0 = corr[ - i] * *ww--;
          /* Weight the neighbourhood of the old lag. */
         t0 *= *we--;

         if ( t0 >= max ) {
            max = t0;
            p_max = i;
         }
      }

   }
   else {
      /* find maximum correlation with weighting */
      for ( i = PIT_MAX; i >= PIT_MIN; i-- ) {
         /* Weighting of the correlation function. */
         t0 = corr[ - i] * *ww--;

         if ( t0 >= max ) {
            max = t0;
            p_max = i;
         }
      }

   }
   psignal = &signal[0];
   p1signal = &signal[ - p_max];
   t0 = 0;
   t1 = 0;

   /* Compute energy */
   for ( j = 0; j < L_FRAME_BY2; j++, psignal++, p1signal++ ) {
      t0 += *psignal * *p1signal;
      t1 += *p1signal * *p1signal;
   }

   if ( dtx ) {
#ifdef VAD2
       vadSt->Rmax += t0;   /* Save max correlation */
       vadSt->R0   += t1;   /* Save max energy */
#else
      /* update and detect tone */
      vad_tone_detection_update( vadSt, 0 );
      vad_tone_detection( vadSt, t0, t1 );
#endif
   }

   /*
    * gain flag is set according to the open_loop gain
    * is t2/t1 > 0.4 ?
    */
   *gain_flg = t0 - ( t1 * 0.4F );
   *cor_max = 0;
   return( p_max );
}


/*
 * gmed_n
 *
 *
 * Parameters:
 *    ind               I: values
 *    n                 I: The number of gains
 *
 * Function:
 *    Calculates N-point median.
 *
 * Returns:
 *    index of the median value
 */
static Word32 gmed_n( Word32 ind[], Word32 n )
{
   Word32 i, j, ix = 0;
   Word32 max;
   Word32 medianIndex;
   Word32 tmp[9];
   Word32 tmp2[9];


   for ( i = 0; i < n; i++ ) {
      tmp2[i] = ind[i];
   }

   for ( i = 0; i < n; i++ ) {
      max = -32767;

      for ( j = 0; j < n; j++ ) {
         if ( tmp2[j] >= max ) {
            max = tmp2[j];
            ix = j;
         }
      }
      tmp2[ix] = -32768;
      tmp[i] = ix;
   }
   medianIndex = tmp[( n >>1 )];
   return( ind[medianIndex] );
}


/*
 * Pitch_ol_wgh
 *
 *
 * Parameters:
 *    old_T0_med     O: old Cl lags median
 *    wght_flg       I: weighting function flag
 *    ada_w          B:
 *    vadSt          B: VAD state struct
 *    signal         I: signal used to compute the open loop pitch
 *                                                  [[-pit_max]:[-1]]
 *    old_lags       I: history with old stored Cl lags
 *    ol_gain_flg    I: OL gain flag
 *    idx            I: frame index
 *    dtx            I: DTX flag
 *
 * Function:
 *    Open-loop pitch search with weight
 *
 *    Open-loop pitch analysis is performed twice per frame (every 10 ms)
 *    for the 10.2 kbit/s mode to find two estimates of the pitch lag
 *    in each frame. The open-loop pitch analysis is done in order to simplify
 *    the pitch analysis and confine the closed loop pitch search to
 *    a small number of lags around the open-loop estimated lags.
 *    Open-loop pitch estimation is based on the weighted speech signal
 *    which is obtained by filtering the input speech signal through
 *    the weighting filter.
 *    The correlation of weighted speech is determined.
 *    The estimated pitch-lag is the delay that maximises
 *    the weighted autocorrelation function. To enhance  pitch-lag analysis
 *    the autocorrelation function estimate is modified by a weighting window.
 *    The weighting emphasises relevant pitch-lags, thus increasing
 *    the likelihood of selecting the correct delay.
 *    minimum pitch lag = 20
 *    maximum pitch lag = 143
 *
 * Returns:
 *    p_max1            open loop pitch lag
 */
static Word32 Pitch_ol_wgh( Word32 *old_T0_med, Word16 *wght_flg, Float32 *ada_w,
      vadState *vadSt, Float32 signal[], Word32 old_lags[], Float32 ol_gain_flg[],
      Word16 idx, Word32 dtx )
{
   Float32 corr[PIT_MAX + 1];
#ifndef VAD2
   Float32 corr_hp_max;
#endif
   Float32 *corrPtr;
   Word32 i, max1, p_max1;


   /* calculate all coreelations of signal, from pit_min to pit_max */
   corrPtr = &corr[PIT_MAX];
   comp_corr( signal, L_FRAME_BY2, PIT_MAX, PIT_MIN, corrPtr );
   p_max1 = Lag_max_wght( vadSt, corrPtr, signal, *old_T0_med,
         &max1, *wght_flg, &ol_gain_flg[idx], dtx );

   if ( ol_gain_flg[idx] > 0 ) {
      /* Calculate 5-point median of previous lags */
      /* Shift buffer */
      for ( i = 4; i > 0; i-- ) {
         old_lags[i] = old_lags[i - 1];
      }
      old_lags[0] = p_max1;
      *old_T0_med = gmed_n( old_lags, 5 );
      *ada_w = 1;
   }
   else {
      *old_T0_med = p_max1;
      *ada_w = *ada_w * 0.9F;
   }

   if ( *ada_w < 0.3 ) {
      *wght_flg = 0;
   }
   else {
      *wght_flg = 1;
   }

#ifndef VAD2
   if ( dtx ) {
      if ( idx == 1 ) {
         /* calculate max high-passed filtered correlation of all lags */
         hp_max( corrPtr, signal, L_FRAME_BY2, PIT_MAX, PIT_MIN, &corr_hp_max );

         /* update complex background detector */
         vadSt->best_corr_hp = corr_hp_max * 0.5F;
      }
   }
#endif
   return( p_max1 );
}


/*
 * ol_ltp
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    vadSt             B: VAD state struct
 *    wsp               I: signal used to compute the OL pitch
 *    T_op              O: open loop pitch lag
 *    ol_gain_flg       I: OL gain flag
 *    old_T0_med        O: old Cl lags median
 *    wght_flg          I: weighting function flag
 *    ada_w             B:
 *    old_lags          I: history with old stored Cl lags
 *    ol_gain_flg       I: OL gain flag
 *    dtx               I: DTX flag
 *    idx               I: frame index
 *
 * Function:
 *    Compute the open loop pitch lag.
 *
 *    Open-loop pitch analysis is performed in order to simplify
 *    the pitch analysis and confine the closed-loop pitch search to
 *    a small number of lags around the open-loop estimated lags.
 *    Open-loop pitch estimation is based on the weighted speech signal Sw(n)
 *    which is obtained by filtering the input speech signal through
 *    the weighting filter W(z) = A(z/g1) / A(z/g2). That is,
 *    in a subframe of size L, the weighted speech is given by:
 *
 *                10
 *    Sw(n) = S(n) + SUM[ a(i) * g1(i) * S(n-i) ]
 *                i=1
 *                   10
 *                - SUM[ a(i) * g2(i) * Sw(n-i) ], n = 0, ..., L-1
 *                  i=1
 *
 * Returns:
 *    void
 */
static void ol_ltp( enum Mode mode, vadState *vadSt, Float32 wsp[], Word32 *T_op
      , Float32 ol_gain_flg[], Word32 *old_T0_med, Word16 *wght_flg, Float32 *ada_w
      , Word32 *old_lags, Word32 dtx, Word16 idx )
{
   if ( mode != MR102 ) {
      ol_gain_flg[0] = 0;
      ol_gain_flg[1] = 0;
   }

   if ( ( mode == MR475 ) || ( mode == MR515 ) ) {
      *T_op = Pitch_ol( mode, vadSt, wsp, PIT_MIN, PIT_MAX, L_FRAME, dtx, idx );
   }
   else {
      if ( mode <= MR795 ) {
         *T_op = Pitch_ol( mode, vadSt, wsp, PIT_MIN, PIT_MAX, L_FRAME_BY2, dtx,
               idx );
      }
      else if ( mode == MR102 ) {
         *T_op = Pitch_ol_wgh( old_T0_med, wght_flg, ada_w, vadSt, wsp, old_lags,
            ol_gain_flg, idx, dtx );
      }
      else {
         *T_op = Pitch_ol( mode, vadSt, wsp, PIT_MIN_MR122, PIT_MAX, L_FRAME_BY2
               , dtx, idx );
      }
   }
}


/*
 * subframePreProc
 *
 *
 * Parameters:
 *    mode           I: AMR mode
 *    gamma1         I: spectral exp. factor 1
 *    gamma1_12k2    I: spectral exp. factor 1 for EFR
 *    gamma2         I: spectral exp. factor 2
 *    A              I: A(z) unquantized for the 4 subframes
 *    Aq             I: A(z)   quantized for the 4 subframes
 *    speech         I: speech segment
 *    mem_err        I: pointer to error signal
 *    mem_w0         I: memory of weighting filter
 *    zero           I: pointer to zero vector
 *    ai_zero        O: history of weighted synth. filter
 *    exc            O: long term prediction residual
 *    h1             O: impulse response
 *    xn             O: target vector for pitch search
 *    res2           O: long term prediction residual
 *    error          O: error of LPC synthesis filter
 *
 * Function:
 *    Subframe preprocessing
 *
 *    Impulse response computation:
 *       The impulse response, h(n), of the weighted synthesis filter
 *
 *       H(z) * W(z) = A(z/g1) / ( A'(z) * A(z/g2) )
 *
 *       is computed each subframe. This impulse response is needed for
 *       the search of adaptive and fixed codebooks. The impulse response h(n)
 *       is computed by filtering the vector of coefficients of
 *       the filter A(z/g1) extended by zeros through the two filters
 *       1/A'(z) and 1/A(z/g2).
 *
 *    Target signal computation:
 *       The target signal for adaptive codebook search is usually computed
 *       by subtracting the zero input response of
 *       the weighted synthesis filter H(z) * W(z) from the weighted
 *       speech signal Sw(n). This is performed on a subframe basis.
 *       An equivalent procedure for computing the target signal is
 *       the filtering of the LP residual signal res(n) through
 *       the combination of synthesis filter 1/A'(z) and
 *       the weighting filter A(z/g1)/A(z/g2). After determining
 *       the excitation for the subframe, the initial states of
 *       these filters are updated by filtering the difference between
 *       the LP residual and excitation.
 *
 *       The residual signal res(n) which is needed for finding
 *       the target vector is also used in the adaptive codebook search
 *       to extend the past excitation buffer. This simplifies
 *       the adaptive codebook search procedure for delays less than
 *       the subframe size of 40. The LP residual is given by:
 *
 *                        10
 *       res(n) = S(n) + SUM[A'(i)* S(n-i)
 *                       i=1
 *
 * Returns:
 *    void
 */
static void subframePreProc( enum Mode mode, const Float32 gamma1[], const
      Float32 gamma1_12k2[], const Float32 gamma2[], Float32 *A, Float32 *Aq,
      Float32 *speech, Float32 *mem_err, Float32 *mem_w0, Float32 *zero, Float32
      ai_zero[], Float32 *exc, Float32 h1[], Float32 xn[], Float32 res2[],
      Float32 error[] )
{
   Float32 Ap1[MP1];   /* weighted LPC coefficients */
   Float32 Ap2[MP1];   /* weighted LPC coefficients */
   const Float32 *g1;


   /* mode specific pointer to gamma1 values */
   g1 = gamma1;

   if ( ( mode == MR122 ) || ( mode == MR102 ) ) {
      g1 = gamma1_12k2;
   }

   /* Find the weighted LPC coefficients for the weighting filter. */
   Weight_Ai( A, g1, Ap1 );
   Weight_Ai( A, gamma2, Ap2 );

   /*
    * Compute impulse response, h1[],
    * of weighted synthesis filter A(z/g1)/A(z/g2)
    */
   memcpy( ai_zero, Ap1, MP1 <<2 );
   Syn_filt( Aq, ai_zero, h1, zero, 0 );
   Syn_filt( Ap2, h1, h1, zero, 0 );

   /*
    * Find the target vector for pitch search:
    */
   /* LP residual */
   Residu( Aq, speech, res2 );
   memcpy( exc, res2, L_SUBFR <<2 );

   /* Synthesis filter */
   Syn_filt( Aq, exc, error, mem_err, 0 );
   Residu( Ap1, error, xn );

   /* target signal xn[] */
   Syn_filt( Ap2, xn, xn, mem_w0, 0 );
}


/*
 * getRange
 *
 *
 * Parameters:
 *    T0                I: integer pitch
 *    delta_low         I: search start offset
 *    delta_range       I: search range
 *    pitmin            I: minimum pitch
 *    pitmax            I: maximum pitch
 *    T0_min            I: search range minimum
 *    T0_max            I: search range maximum
 *
 * Function:
 *    Sets range around open-loop pitch or integer pitch of last subframe
 *
 *    Takes integer pitch T0 and calculates a range around it with
 *    T0_min = T0-delta_low and T0_max = (T0-delta_low) + delta_range
 *    T0_min and T0_max are bounded by pitmin and pitmax
 *
 * Returns:
 *    void
 */
static void getRange( Word32 T0, Word16 delta_low, Word16 delta_range,
      Word16 pitmin, Word16 pitmax, Word32 *T0_min, Word32 *T0_max )
{
   *T0_min = T0 - delta_low;

   if ( *T0_min < pitmin ) {
      *T0_min = pitmin;
   }
   *T0_max = *T0_min + delta_range;

   if ( *T0_max > pitmax ) {
      *T0_max = pitmax;
      *T0_min = *T0_max - delta_range;
   }
}


/*
 * Norm_Corr
 *
 *
 * Parameters:
 *    exc         I: excitation buffer                      [L_SUBFR]
 *    xn          I: target vector                          [L_SUBFR]
 *    h           I: impulse response of synthesis and weighting filters
 *                                                          [L_SUBFR]
 *    t_min       I: interval to compute normalized correlation
 *    t_max       I: interval to compute normalized correlation
 *    corr_norm   O: Normalized correlation                 [wT_min-wT_max]
 *
 * Function:
 *    Normalized correlation
 *
 *    The closed-loop pitch search is performed by minimizing
 *    the mean-square weighted error between the original and
 *    synthesized speech. This is achieved by maximizing the term:
 *
 *            39                           39
 *    R(k) = SUM[ X(n) * Yk(n)) ] / SQRT[ SUM[ Yk(n) * Yk(n)] ]
 *           n=0                          n=0
 *
 *    where X(n) is the target signal and Yk(n) is the past filtered
 *    excitation at delay k (past excitation convolved with h(n) ).
 *    The search range is limited around the open-loop pitch.
 *
 *    The convolution Yk(n) is computed for the first delay t_min in
 *    the searched range, and for the other delays in the search range
 *    k = t_min + 1, ..., t_max, it is updated using the recursive relation:
 *
 *    Yk(n) = Yk-1(n-1) + u(-k) * h(n),
 *
 *    where u(n), n = -( 143 + 11 ), ..., 39, is the excitation buffer.
 *    Note that in search stage, the samples u(n), n = 0, ..., 39,
 *    are not known, and they are needed for pitch delays less than 40.
 *    To simplify the search, the LP residual is copied to u(n) in order
 *    to make the relation in above equation valid for all delays.
 *
 * Returns:
 *    void
 */
static void Norm_Corr( Float32 exc[], Float32 xn[], Float32 h[], Word32 t_min,
      Word32 t_max, Float32 corr_norm[] )
{
   Float32 exc_temp[L_SUBFR];
   Float32 *p_exc;
   Float32 corr, norm;
   Float32 sum;
   Word32 i, j, k;


   k = -t_min;
   p_exc = &exc[ - t_min];

   /* compute the filtered excitation for the first delay t_min */
   /* convolution Yk(n) */
   for ( j = 0; j < L_SUBFR; j++ ) {
      sum = 0;

      for ( i = 0; i <= j; i++ ) {
         sum += p_exc[i] * h[j - i];
      }
      exc_temp[j] = sum;
   }

   /* loop for every possible period */
   for ( i = t_min; i <= t_max; i++ ) {
      /*        39                     */
      /* SQRT[ SUM[ Yk(n) * Yk(n)] ]   */
      /*       n=0                     */
      norm = (Float32)Dotproduct40( exc_temp, exc_temp );

      if ( norm == 0 )
         norm = 1.0;
      else
         norm = ( Float32 )( 1.0 / ( sqrt( norm ) ) );

      /*        39                  */
      /* SQRT[ SUM[ X(n) * Yk(n)] ] */
      /*       n=0                  */
      corr = (Float32)Dotproduct40( xn, exc_temp );

      /* R(k) */
      corr_norm[i] = corr * norm;

      /* modify the filtered excitation exc_tmp[] for the next iteration */
      if ( i != t_max ) {
         k--;

         for ( j = L_SUBFR - 1; j > 0; j-- ) {
            /* Yk(n) = Yk-1(n-1) + u(-k) * h(n) */
            exc_temp[j] = exc_temp[j - 1] + exc[k] * h[j];
         }
         exc_temp[0] = exc[k];
      }
   }
}


/*
 * Interpol_3or6
 *
 *
 * Parameters:
 *    x                 I: input vector
 *    frac              I: fraction  (-2..2 for 3*, -3..3 for 6*)
 *    flag3             I: if set, upsampling rate = 3 (6 otherwise)
 *
 * Function:
 *    Interpolating the normalized correlation with 1/3 or 1/6 resolution.
 *
 *    The interpolation is performed using an FIR filter b24
 *    based on a Hamming windowed sin(x)/x function truncated at ±23
 *    and padded with zeros at ±24 (b24(24) = 0). The filter has its
 *    cut-off frequency (-3 dB) at 3 600 Hz in the over-sampled domain.
 *    The interpolated values of R(k) for the fractions -3/6 to 3/6
 *    are obtained using the interpolation formula:
 *
 *              3                            3
 *    R(k)t = SUM[ R(k-i) * b24(t+i*6) ] + SUM [ R(k+1+i) * b24(6-t+i*6) ],
 *            i=0                          i=0
 *    t = 0, ..., 5,
 *
 *    where t = 0, ..., 5, corresponds to the fractions
 *    0, 1/6, 2/6, 3/6, -2/6, and -1/6, respectively. Note that it is
 *    necessary to compute the correlation terms using a range t_min - 4,
 *    t_max + 4, to allow for the proper interpolation.
 *
 * Returns:
 *    s                 interpolated value
 */
static Float32 Interpol_3or6( Float32 *x, Word32 frac, Word16 flag3 )
{
   Float32 s;
   Float32 *x1, *x2;
   const Float32 *c1, *c2;
   Word32 i, k;


   if ( flag3 != 0 ) {
      /* inter_3[k] = b60[2*k] -> k' = 2*k */
      frac <<= 1;
   }

   if ( frac < 0 ) {
      frac += UP_SAMP_MAX;
      x--;
   }
   x1 = &x[0];
   x2 = &x[1];
   c1 = &b24[frac];
   c2 = &b24[UP_SAMP_MAX - frac];
   s = 0;

   for ( i = 0, k = 0; i < L_INTER_SRCH; i++, k += UP_SAMP_MAX ) {
      /* R(k-i) * b24(t+i*6) */
      s += x1[ - i] * c1[k];

      /* R(k+1+i) * b24(6-t+i*6) */
      s += x2[i] * c2[k];
   }
   return s;
}


/*
 * searchFrac
 *
 *
 * Parameters:
 *    lag               B: integer pitch
 *    frac              B: start point of search - fractional pitch
 *    last_frac         I: endpoint of search
 *    corr              I: normalized correlation
 *    flag3             I: if set, upsampling rate = 3 (6 otherwise)
 *
 * Function:
 *    Find fractional pitch
 *
 *    The function interpolates the normalized correlation at the
 *    fractional positions around lag T0. The position at which the
 *    interpolation function reaches its maximum is the fractional pitch.
 *    Starting point of the search is frac, end point is last_frac.
 *    frac is overwritten with the fractional pitch.
 *
 * Returns:
 *    void
 */
static void searchFrac( Word32 *lag, Word32 *frac, Word16 last_frac, Float32
      corr[], Word16 flag3 )
{
   Float32 max, corr_int;
   Word32 i;


   /*
    * Test the fractions around T0 and choose the one which maximizes
    * the interpolated normalized correlation.
    */
   max = Interpol_3or6( &corr[ * lag], *frac, flag3 );

   for ( i = *frac + 1; i <= last_frac; i++ ) {
      corr_int = Interpol_3or6( &corr[ * lag], i, flag3 );

      if ( corr_int > max ) {
         max = corr_int;
         *frac = i;
      }
   }

   if ( flag3 == 0 ) {
      /* Limit the fraction value in the interval [-2,-1,0,1,2,3] */
      if ( *frac == -3 ) {
         *frac = 3;
         *lag -= 1;
      }
   }
   else {
      /* limit the fraction value between -1 and 1 */
      if ( *frac == -2 ) {
         *frac = 1;
         *lag -= 1;
      }

      if ( *frac == 2 ) {
         *frac = -1;
         *lag += 1;
      }
   }
}


/*
 * Enc_lag3
 *
 *
 * Parameters:
 *    T0             I: Pitch delay
 *    T0_frac        I: Fractional pitch delay
 *    T0_prev        I: Integer pitch delay of last subframe
 *    T0_min         I: minimum of search range
 *    T0_max         I: maximum of search range
 *    delta_flag     I: Flag for 1st (or 3rd) subframe
 *    flag4          I: Flag for encoding with 4 bits
 *
 * Function:
 *    Encoding of fractional pitch lag with 1/3 resolution.
 *
 * Returns:
 *    index             index of encoding
 */
static Word32 Enc_lag3( Word32 T0, Word32 T0_frac, Word32 T0_prev, Word32 T0_min
      , Word32 T0_max, Word16 delta_flag, Word16 flag4 )
{
   Word32 index, i, tmp_ind, uplag, tmp_lag;


   /* if 1st or 3rd subframe */
   if ( delta_flag == 0 ) {
      /* encode pitch delay (with fraction) */
      if ( T0 <= 85 ) {
         index = T0 * 3 - 58 + T0_frac;
      }
      else {
         index = T0 + 112;
      }
   }

   /* if second or fourth subframe */
   else {
      if ( flag4 == 0 ) {
         /* 'normal' encoding: either with 5 or 6 bit resolution */
         index = 3 * ( T0 - T0_min ) + 2 + T0_frac;
      }
      else {
         /* encoding with 4 bit resolution */
         tmp_lag = T0_prev;

         if ( ( tmp_lag - T0_min ) > 5 )
            tmp_lag = T0_min + 5;

         if ( ( T0_max - tmp_lag ) > 4 )
            tmp_lag = T0_max - 4;
         uplag = T0 + T0 + T0 + T0_frac;
         i = tmp_lag - 2;
         tmp_ind = i + i + i;

         if ( tmp_ind >= uplag ) {
            index = ( T0 - tmp_lag ) + 5;
         }
         else {
            i = tmp_lag + 1;
            i = i + i + i;

            if ( i > uplag ) {
               index = ( uplag - tmp_ind ) + 3;
            }
            else {
               index = ( T0 - tmp_lag ) + 11;
            }
         }
      }   /* end if (encoding with 4 bit resolution) */
   }   /* end if (second of fourth subframe) */
   return index;
}


/*
 * Enc_lag6
 *
 *
 * Parameters:
 *    T0             I: Pitch delay
 *    T0_frac        I: Fractional pitch delay
 *    T0_min         I: minimum of search range
 *    delta_flag     I: Flag for 1st (or 3rd) subframe
 *
 * Function:
 *    Encoding of fractional pitch lag with 1/6 resolution.
 *
 *    First and third subframes:
 *       The pitch range is divided as follows:
 *          17 3/6  to   94 3/6   resolution 1/6
 *          95      to   143      resolution 1
 *       The period is encoded with 9 bits.
 *       For the range with fractions:
 *          index = (T-17)*6 + frac - 3;
 *          where T=[17..94] and frac=[-2,-1,0,1,2,3]
 *       and for the integer only range
 *          index = (T - 95) + 463;
 *          where T=[95..143]
 *    Second and fourth subframes:
 *       For the 2nd and 4th subframes a resolution of 1/6 is always used,
 *       and the search range is relative to the lag in previous subframe.
 *       If T0 is the lag in the previous subframe then
 *       t_min=T0-5   and  t_max=T0+4   and  the range is given by
 *       (t_min-1) 3/6   to  (t_max) 3/6
 *       The period in the 2nd (and 4th) subframe is encoded with 6 bits:
 *          index = (T-(t_min-1))*6 + frac - 3;
 *          where T=[t_min-1..t_max] and frac=[-2,-1,0,1,2,3]
 *    Note that only 61 values are used. If the decoder receives 61, 62,
 *    or 63 as the relative pitch index, it means that a transmission
 *    error occurred and the pitch from previous subframe should be used.
 *
 * Returns:
 *    index             index of encoding
 */
static Word32 Enc_lag6( Word32 T0, Word32 T0_frac, Word32 T0_min, Word16
      delta_flag )
{
   Word32 index;


   /* if 1st or 3rd subframe */
   if ( delta_flag == 0 ) {
      /* encode pitch delay (with fraction) */
      if ( T0 <= 94 ) {
         /* index = (t-17)*6 + frac - 3 */
         index = T0 * 6 - 105 + T0_frac;
      }
      else {
         /* index = (t - 95) + 463 */
         index = T0 + 368;
      }
   }

   /* if second or fourth subframe */
   else {
      /* index = (t-(t_min-1))*6 + frac - 3 */
      index = 6 * ( T0 - T0_min ) + 3 + T0_frac;
   }
   return index;
}


/*
 * Pitch_fr
 *
 *
 * Parameters:
 *    T0_prev_subframe  B: integer pitch lag of previous sub-frame
 *    mode              I: codec mode
 *    T_op              I: open-loop pitch estimations for
 *                         the 2 big subframes [2]
 *    exc               I: excitation buffer
 *    xn                I: target vector
 *    h                 I: impulse response of synthesis
 *                         and weighting filters
 *    i_subfr           I: subframe number
 *    pit_frac          O: pitch period (fractional)
 *    resu3             O: subsample resolution 1/3 (=1) or 1/6 (=0)
 *    ana_index         O: index of encoding
 *
 * Function:
 *    Closed-loop pitch search
 *
 *    In the first and third subframes, a fractional pitch delay is used
 *    with resolutions: 1/6 in the range [17 3/6, 94 3/6] and integers only
 *    in the range [95, 143]. For the second and fourth subframes,
 *    a pitch resolution of 1/6 is always used in
 *    the range [T1 - 5 3/6, T1 + 4 /3/6], where T1 is nearest integer to
 *    the fractional pitch lag of the previous (1st or 3rd) subframe,
 *    bounded by 18...143.
 *
 *    Closed-loop pitch analysis is performed around
 *    the open-loop pitch estimates on a subframe basis.
 *    In the first (and third) subframe the range Top±3,
 *    bounded by 18...143, is searched. For the other subframes,
 *    closed-loop pitch analysis is performed around the integer pitch
 *    selected in the previous subframe, as described above.
 *    The pitch delay is encoded with 9 bits in the first and
 *    third subframes and the relative delay of the other subframes
 *    is encoded with 6 bits.
 *
 *    The closed-loop pitch search is performed by minimizing
 *    the mean-square weighted error between the original and
 *    synthesized speech. This is achieved by maximizing the term:
 *
 *            39                           39
 *    R(k) = SUM[ X(n) * Yk(n)) ] / SQRT[ SUM[ Yk(n) * Yk(n)] ]
 *           n=0                          n=0
 *
 *    where X(n) is the target signal and Yk(n) is the past filtered
 *    excitation at delay k (past excitation convolved with h(n) ).
 *
 *    Once the optimum integer pitch delay is determined, the fractions
 *    from -3/6 to 3/6 with a step of 1/6 around that integer are tested.
 *    The fractional pitch search is performed by interpolating
 *    the normalized correlation R(k) and searching for its maximum.
 *    The interpolation is performed using an FIR filter b24
 *    based on a Hamming windowed sin(x)/x function truncated at ±23
 *    and padded with zeros at ±24 (b24(24) = 0). The filter has its
 *    cut-off frequency (-3 dB) at 3 600 Hz in the over-sampled domain.
 *    The interpolated values of R(k) for the fractions -3/6 to 3/6
 *    are obtained using the interpolation formula:
 *
 *              3                            3
 *    R(k)t = SUM[ R(k-i) * b24(t+i*6) ] + SUM [ R(k+1+i) * b24(6-t+i*6) ],
 *            i=0                          i=0
 *    t = 0, ..., 5,
 *
 *    where t = 0, ..., 5, corresponds to the fractions
 *    0, 1/6, 2/6, 3/6, -2/6, and -1/6, respectively. Note that it is
 *    necessary to compute the correlation terms using a range t_min -4,
 *    t_max + 4, to allow for the proper interpolation.
 *
 * Returns:
 *    lag             closed-loop pitch lag
 */
static Word32 Pitch_fr( Word32 *T0_prev_subframe, enum Mode mode, Word32 T_op[],
      Float32 exc[], Float32 xn[], Float32 h[], Word16 i_subfr, Word32 *pit_frac
      , Word16 *resu3, Word32 *ana_index )
{
   Float32 corr_v[40];
   Float32 max;
   Float32 *corr;
   Word32 i, t_min, t_max, T0_min, T0_max;
   Word32 lag, frac, tmp_lag;
   Word16 max_frac_lag, flag3, flag4, last_frac;
   Word16 delta_int_low, delta_int_range, delta_frc_low, delta_frc_range;
   Word16 pit_min;
   Word16 frame_offset;
   Word16 delta_search;


   /* set mode specific variables */
   max_frac_lag = mode_dep_parm[mode].max_frac_lag;
   flag3 = mode_dep_parm[mode].flag3;
   frac = mode_dep_parm[mode].first_frac;
   last_frac = mode_dep_parm[mode].last_frac;
   delta_int_low = mode_dep_parm[mode].delta_int_low;
   delta_int_range = mode_dep_parm[mode].delta_int_range;
   delta_frc_low = mode_dep_parm[mode].delta_frc_low;
   delta_frc_range = mode_dep_parm[mode].delta_frc_range;
   pit_min = mode_dep_parm[mode].pit_min;

   /* decide upon full or differential search */
   delta_search = 1;

   if ( ( i_subfr == 0 ) || ( i_subfr == L_FRAME_BY2 ) ) {
      /* Subframe 1 and 3 */
      if ( ( ( mode != MR475 ) && ( mode != MR515 ) ) || ( i_subfr !=
            L_FRAME_BY2 ) ) {
         /*
          * set T0_min, T0_max for full search
          * this is *not* done for mode MR475, MR515 in subframe 3
          */
         delta_search = 0;   /* no differential search */

         /*
          * calculate index into T_op which contains the open-loop
          * pitch estimations for the 2 big subframes
          */
         frame_offset = 1;

         if ( i_subfr == 0 )
            frame_offset = 0;

         /*
          * get T_op from the corresponding half frame and
          * set T0_min, T0_max
          */
         getRange( T_op[frame_offset], delta_int_low, delta_int_range, pit_min,
               PIT_MAX, &T0_min, &T0_max );
      }
      else {
         /* mode MR475, MR515 and 3. Subframe: delta search as well */
         getRange( *T0_prev_subframe, delta_frc_low, delta_frc_range, pit_min,
               PIT_MAX, &T0_min, &T0_max );
      }
   }
   else {
      /*
       * for Subframe 2 and 4
       * get range around T0 of previous subframe for delta search
       */
      getRange( *T0_prev_subframe, delta_frc_low, delta_frc_range, pit_min,
            PIT_MAX, &T0_min, &T0_max );
   }

   /* Find interval to compute normalized correlation */
   t_min = T0_min - L_INTER_SRCH;
   t_max = T0_max + L_INTER_SRCH;
   corr = &corr_v[ - t_min];

   /* Compute normalized correlation between target and filtered excitation */
   Norm_Corr( exc, xn, h, t_min, t_max, corr );

   /* Find integer pitch */
   max = corr[T0_min];
   lag = T0_min;

   for ( i = T0_min + 1; i <= T0_max; i++ ) {
      if ( corr[i] >= max ) {
         max = corr[i];
         lag = i;
      }
   }

   /* Find fractional pitch   */
   if ( ( delta_search == 0 ) && ( lag > max_frac_lag ) ) {
      /*
       * full search and integer pitch greater than max_frac_lag
       * fractional search is not needed, set fractional to zero
       */
      frac = 0;
   }
   else {
      /*
       * if differential search AND mode MR475 OR MR515 OR MR59 OR MR67
       * then search fractional with 4 bits resolution
       */
      if ( ( delta_search != 0 ) && ( ( mode == MR475 ) || ( mode == MR515 ) ||
            ( mode == MR59 ) || ( mode == MR67 ) ) ) {
         /*
          * modify frac or last_frac according to position of last
          * integer pitch: either search around integer pitch,
          * or only on left or right side
          */
         tmp_lag = *T0_prev_subframe;

         if ( ( tmp_lag - T0_min ) > 5 )
            tmp_lag = T0_min + 5;

         if ( ( T0_max - tmp_lag ) > 4 )
            tmp_lag = T0_max - 4;

         if ( ( lag == tmp_lag ) || ( lag == ( tmp_lag - 1 ) ) ) {
            /* normal search in fractions around T0 */
            searchFrac( &lag, &frac, last_frac, corr, flag3 );
         }
         else if ( lag == ( tmp_lag - 2 ) ) {
            /* limit search around T0 to the right side */
            frac = 0;
            searchFrac( &lag, &frac, last_frac, corr, flag3 );
         }
         else if ( lag == ( tmp_lag + 1 ) ) {
            /* limit search around T0 to the left side */
            last_frac = 0;
            searchFrac( &lag, &frac, last_frac, corr, flag3 );
         }
         else {
            /* no fractional search */
            frac = 0;
         }
      }
      else

         /* test the fractions around T0 */
         searchFrac( &lag, &frac, last_frac, corr, flag3 );
   }

   /*
    *  encode pitch
    */
   if ( flag3 != 0 ) {
      /*
       * flag4 indicates encoding with 4 bit resolution;
       * this is needed for mode MR475, MR515 and MR59
       */
      flag4 = 0;

      if ( ( mode == MR475 ) || ( mode == MR515 ) || ( mode == MR59 ) || ( mode
            == MR67 ) ) {
         flag4 = 1;
      }

      /* encode with 1/3 subsample resolution */
      *ana_index = Enc_lag3( lag, frac, *T0_prev_subframe, T0_min, T0_max,
            delta_search, flag4 );
   }
   else {
      /* encode with 1/6 subsample resolution */
      *ana_index = Enc_lag6( lag, frac, T0_min, delta_search );
   }

   /*
    *  update state variables
    */
   *T0_prev_subframe = lag;

   /*
    * update output variables
    */
   *resu3 = flag3;
   *pit_frac = frac;
   return( lag );
}


/*
 * Pred_lt_3or6
 *
 *
 * Parameters:
 *    exc      B: excitation buffer
 *    T0       I: integer pitch lag
 *    frac     I: fraction of lag
 *    flag3    I: if set, upsampling rate = 3 (6 otherwise)
 *
 * Function:
 *    Compute the result of long term prediction with fractional
 *    interpolation of resolution 1/3 or 1/6. (Interpolated past excitation).
 *
 *    Once the fractional pitch lag is determined,
 *    the adaptive codebook vector v(n) is computed by interpolating
 *    the past excitation signal u(n) at the given integer delay k
 *    and phase (fraction)  :
 *
 *            9                              9
 *    v(n) = SUM[ u(n-k-i) * b60(t+i*6) ] + SUM[ u(n-k+1+i) * b60(6-t+i*6) ],
 *           i=0                            i=0
 *    n = 0, ...,39, t = 0, ...,5.
 *
 *    The interpolation filter b60 is based on a Hamming windowed sin(x)/x
 *    function truncated at ± 59 and padded with zeros at ± 60 (b60(60)=0)).
 *    The filter has a cut-off frequency (-3 dB) at 3 600 Hz in
 *    the over-sampled domain.
 *
 * Returns:
 *    void
 */
static void Pred_lt_3or6( Float32 exc[], Word32 T0, Word32 frac, Word16 flag3 )
{
   Float32 s;
   Float32 *x0, *x1, *x2;
   const Float32 *c1, *c2;
   Word32 j;


   x0 = &exc[ - T0];
   frac = -frac;

   if ( flag3 != 0 ) {
      /* inter_3l[k] = b60[2*k] -> k' = 2*k */
      frac <<= 1;
   }

   if ( frac < 0 ) {
      frac += UP_SAMP_MAX;
      x0--;
   }
   c1 = &b60[frac];
   c2 = &b60[UP_SAMP_MAX - frac];

   for ( j = 0; j < L_SUBFR; j += 4 ) {
      x1 = x0++;
      x2 = x0;
      s = x1[0] * c1[0] + x2[0] * c2[0];
      s += x1[ - 1] *c1[6] + x2[1] * c2[6];
      s += x1[ - 2] * c1[12] + x2[2] * c2[12];
      s += x1[ - 3] * c1[18] + x2[3] * c2[18];
      s += x1[ - 4] * c1[24] + x2[4] * c2[24];
      s += x1[ - 5] * c1[30] + x2[5] * c2[30];
      s += x1[ - 6] * c1[36] + x2[6] * c2[36];
      s += x1[ - 7] * c1[42] + x2[7] * c2[42];
      s += x1[ - 8] * c1[48] + x2[8] * c2[48];
      s += x1[ - 9] * c1[54] + x2[9] * c2[54];
      exc[j] = ( Float32 )floor(s + 0.5F);
      x1 = x0++;
      x2 = x0;
      s = x1[0] * c1[0] + x2[0] * c2[0];
      s += x1[ - 1] *c1[6] + x2[1] * c2[6];
      s += x1[ - 2] * c1[12] + x2[2] * c2[12];
      s += x1[ - 3] * c1[18] + x2[3] * c2[18];
      s += x1[ - 4] * c1[24] + x2[4] * c2[24];
      s += x1[ - 5] * c1[30] + x2[5] * c2[30];
      s += x1[ - 6] * c1[36] + x2[6] * c2[36];
      s += x1[ - 7] * c1[42] + x2[7] * c2[42];
      s += x1[ - 8] * c1[48] + x2[8] * c2[48];
      s += x1[ - 9] * c1[54] + x2[9] * c2[54];
      exc[j + 1] = ( Float32 )floor(s + 0.5F);
      x1 = x0++;
      x2 = x0;
      s = x1[0] * c1[0] + x2[0] * c2[0];
      s += x1[ - 1] *c1[6] + x2[1] * c2[6];
      s += x1[ - 2] * c1[12] + x2[2] * c2[12];
      s += x1[ - 3] * c1[18] + x2[3] * c2[18];
      s += x1[ - 4] * c1[24] + x2[4] * c2[24];
      s += x1[ - 5] * c1[30] + x2[5] * c2[30];
      s += x1[ - 6] * c1[36] + x2[6] * c2[36];
      s += x1[ - 7] * c1[42] + x2[7] * c2[42];
      s += x1[ - 8] * c1[48] + x2[8] * c2[48];
      s += x1[ - 9] * c1[54] + x2[9] * c2[54];
      exc[j + 2] = ( Float32 )floor(s + 0.5F);
      x1 = x0++;
      x2 = x0;
      s = x1[0] * c1[0] + x2[0] * c2[0];
      s += x1[ - 1] *c1[6] + x2[1] * c2[6];
      s += x1[ - 2] * c1[12] + x2[2] * c2[12];
      s += x1[ - 3] * c1[18] + x2[3] * c2[18];
      s += x1[ - 4] * c1[24] + x2[4] * c2[24];
      s += x1[ - 5] * c1[30] + x2[5] * c2[30];
      s += x1[ - 6] * c1[36] + x2[6] * c2[36];
      s += x1[ - 7] * c1[42] + x2[7] * c2[42];
      s += x1[ - 8] * c1[48] + x2[8] * c2[48];
      s += x1[ - 9] * c1[54] + x2[9] * c2[54];
      exc[j + 3] = ( Float32 )floor(s + 0.5F);
   }
   return;
}

static void Pred_lt_3or6_fixed( Word32 exc[], Word32 T0, Word32 frac, Word32 flag3 )
{
   Word32 s, i;
   Word32 *x0, *x1, *x2;
   const Word32 *c1, *c2;

   x0 = &exc[ - T0];
   frac = -frac;

   if ( flag3 != 0 ) {
      frac <<= 1;   /* inter_3l[k] = inter6[2*k] -> k' = 2*k */
   }

   if ( frac < 0 ) {
      frac += 6;
      x0--;
   }
   c1 = &inter6[frac];
   c2 = &inter6[6 - frac];

   for ( i = 0; i < 40; i++ ) {
      x1 = x0++;
      x2 = x0;
      s = x1[0] * c1[0];
      s += x1[ - 1] * c1[6];
      s += x1[ - 2] * c1[12];
      s += x1[ - 3] * c1[18];
      s += x1[ - 4] * c1[24];
      s += x1[ - 5] * c1[30];
      s += x1[ - 6] * c1[36];
      s += x1[ - 7] * c1[42];
      s += x1[ - 8] * c1[48];
      s += x1[ - 9] * c1[54];
      s += x2[0] * c2[0];
      s += x2[1] * c2[6];
      s += x2[2] * c2[12];
      s += x2[3] * c2[18];
      s += x2[4] * c2[24];
      s += x2[5] * c2[30];
      s += x2[6] * c2[36];
      s += x2[7] * c2[42];
      s += x2[8] * c2[48];
      s += x2[9] * c2[54];
      exc[i] = ( s + 0x4000 ) >> 15;

   }
}


/*
 * G_pitch
 *
 *
 * Parameters:
 *    xn       I: Pitch target
 *    y1       I: Filtered adaptive codebook
 *    gCoeff   O: Correlations need for gain quantization
 *
 * Function:
 *    Compute the pitch (adaptive codebook) gain.
 *
 *    The adaptive codebook gain is given by
 *
 *       g = <x[], y[]> / <y[], y[]>
 *
 *    where x[] is the target vector, y[] is the filtered adaptive
 *    codevector, and <> denotes dot product.
 *
 * Returns:
 *    gain              gain saturated to 1.2
 */
static Float32 G_pitch( Float32 xn[], Float32 y1[], Float32 gCoeff[] )
{
   Float32 gain, sum;


   /* Compute scalar product <y1[],y1[]> */
   sum = (Float32)Dotproduct40( y1, y1 );

   /* Avoid case of all zeros */
   sum += 0.01F;
   gCoeff[0] = sum;

   /* Compute scalar product <xn[],y1[]> */
   sum = (Float32)Dotproduct40( xn, y1 );
   gCoeff[1] = sum;

   /* compute gain = xy/yy */
   gain = ( Float32 )( gCoeff[1] / gCoeff[0] );

   /* if(gain >1.2) gain = 1.2 */
   if ( gain < 0.0 )
      gain = 0.0F;

   if ( gain > 1.2 )
      gain = 1.2F;
   return( gain );
}


/*
 * check_gp_clipping
 *
 *
 * Parameters:
 *    gp          I: old pitch gains
 *    g_pitch     I: pitch gain
 *
 * Function:
 *    Verify that the sum of the last (N_FRAME+1) pitch gains is under
 *    a certain threshold.
 *
 * Returns:
 *    True or false
 */
static Word16 check_gp_clipping( Float32 *gp, Float32 g_pitch )
{
   Float32 sum;
   Word32 i;


   sum = g_pitch;

   for ( i = 0; i < N_FRAME; i++ ) {
      sum += gp[i];
   }

   if ( sum > 7.6F /*FGP_CLIP*/ ) {
      return 1;
   }
   else {
      return 0;
   }
}


/*
 * q_gain_pitch
 *
 *
 * Parameters:
 *    mode              I: codec mode
 *    gp_limit          I: pitch gain limit
 *    gain              B: Pitch gain (unquant/quant)
 *    gain_cand         O: pitch gain candidates (3),   MR795 only
 *    gain_cind         O: pitch gain cand. indices (3),MR795 only
 *
 * Function:
 *    Closed-loop pitch search
 *
 * Returns:
 *    index             index of quantization
 */
static Word16 q_gain_pitch( enum Mode mode, Float32 gp_limit, Float32 *gain,
      Float32 gain_cand[], Word32 gain_cind[] )
{
   Float32 err_min, err;
   Word32 i, index;


   err_min = ( Float32 )fabs( *gain - qua_gain_pitch[0] );
   index = 0;

   for ( i = 1; i < NB_QUA_PITCH; i++ ) {
      if ( qua_gain_pitch[i] <= gp_limit ) {
         err = ( Float32 )fabs( *gain - qua_gain_pitch[i] );

         if ( err < err_min ) {
            err_min = err;
            index = i;
         }
      }
   }

   if ( mode == MR795 ) {
   /*
    * in MR795 mode, compute three gain_pit candidates around the index
    * found in the quantization loop: the index found and the two direct
    * neighbours, except for the extreme cases (i=0 or i=NB_QUA_PITCH-1),
    * where the direct neighbour and the neighbour to that is used.
    */
      Word32 ii;


      if ( index == 0 ) {
         ii = index;
      }
      else {
         ii = index - 1;

         if ( index == ( NB_QUA_PITCH - 1 ) || ( qua_gain_pitch[index + 1] >
               gp_limit ) ) {
            ii = index - 2;
         }
      }

      /* store candidate indices and values */
      for ( i = 0; i < 3; i++ ) {
         gain_cind[i] = ii;
         gain_cand[i] = qua_gain_pitch[ii];
         ii++;
      }
      *gain = qua_gain_pitch[index];
   }
   else {
      /* return the index and gain pitch found */
      *gain = qua_gain_pitch_MR122[index];
   }
   return( Word16 )index;
}


/*
 * cl_ltp
 *
 *
 * Parameters:
 *    T0_prev_subframe  B: Integer pitch lag of previous sub-frame
 *    gp                I: Gain history
 *    mode              I: Coder mode
 *    frame_offset      I: Offset to subframe
 *    T_op              I: Open loop pitch lags
 *    h1                I: Impulse response vector
 *    exc               B: Excitation vector
 *    res2              B: Long term prediction residual
 *    xn                I: Target vector for pitch search
 *    lsp_flag          I: LSP resonance flag
 *    xn2               O: Target vector for codebook search
 *    y1                O: Filtered adaptive excitation
 *    T0                O: Pitch delay (integer part)
 *    T0_frac           O: Pitch delay (fractional part)
 *    gain_pit          O: Pitch gain
 *    gCoeff[]          O: Correlations between xn, y1, & y2
 *    anap              O: Analysis parameters
 *    gp_limit          O: pitch gain limit
 *
 * Function:
 *    Closed-loop ltp search
 *
 *    Adaptive codebook search is performed on a subframe basis.
 *    It consists of performing closed-loop pitch search, and then computing
 *    the adaptive codevector by interpolating the past excitation at
 *    the selected fractional pitch lag.
 *    The adaptive codebook parameters (or pitch parameters) are
 *    the delay and gain of the pitch filter. In the adaptive codebook approach
 *    for implementing the pitch filter, the excitation is repeated for delays
 *    less than the subframe length. In the search stage, the excitation is
 *    extended by the LP residual to simplify the closed-loop search.
 *
 * Returns:
 *    void
 */
static void cl_ltp( Word32 *T0_prev_subframe, Float32 *gp, enum Mode mode,
      Word16 frame_offset, Word32 T_op[], Float32 *h1, Float32 *exc, Float32
      res2[], Float32 xn[], Word16 lsp_flag, Float32 xn2[], Float32 y1[], Word32
      *T0, Word32 *T0_frac, Float32 *gain_pit, Float32 gCoeff[], Word16 **anap,
      Float32 *gp_limit )
{
   Float32 s;
   Word32 i, n;
   Word16 gpc_flag, resu3;   /* flag for upsample resolution */

   Word32 exc_tmp[314];
   Word32 *exc_tmp_p;

   exc_tmp_p = exc_tmp + PIT_MAX + L_INTERPOL;


   /* Closed-loop fractional pitch search */
   *T0 = Pitch_fr( T0_prev_subframe, mode, T_op, exc, xn, h1, frame_offset,
         T0_frac, &resu3, &i );
   *( *anap )++ = ( Word16 )i;

   /*
    * Compute the adaptive codebook vector
    * using fixed point. This is required
    * to maintain encoder/decoder excitation
    * syncronisation
    */
   for (i = -(PIT_MAX + L_INTERPOL); i < 40; i++)
      exc_tmp_p[i] = (Word32)exc[i];

   Pred_lt_3or6_fixed( exc_tmp_p, *T0, *T0_frac, resu3 );

   for (i = -(PIT_MAX + L_INTERPOL); i < 40; i++)
      exc[i] = (Float32)exc_tmp_p[i];

   /*
    *   Convolve to get filtered adaptive codebook vector
    *  y[n] = sum_{i=0}^{n} x[i] h[n-i], n=0,...,L-1
    */
   for ( n = 0; n < L_SUBFR; n++ ) {
      s = 0;

      for ( i = 0; i <= n; i++ ) {
         s += exc[i] * h1[n - i];
      }
      y1[n] = s;
   }

   /* The adaptive codebook gain */
   *gain_pit = G_pitch( xn, y1, gCoeff );

   /* check if the pitch gain should be limit due to resonance in LPC filter */
   gpc_flag = 0;
   *gp_limit = 2.0F;

   if ( ( lsp_flag != 0 ) && ( *gain_pit > 0.95F ) ) {
      gpc_flag = check_gp_clipping( gp, *gain_pit );
   }

   /*
    * special for the MR475, MR515 mode; limit the gain to 0.85 to
    * cope with bit errors in the decoder in a better way.
    */
   if ( ( mode == MR475 ) || ( mode == MR515 ) ) {
      if ( *gain_pit > 0.85 ) {
         *gain_pit = 0.85F;
      }

      if ( gpc_flag != 0 )
         *gp_limit = GP_CLIP;
   }
   else {
      if ( gpc_flag != 0 ) {
         *gp_limit = GP_CLIP;
         *gain_pit = GP_CLIP;
      }

      /*
       * 12k2 gain_pit is quantized here and not in gainQuant.
       */
      if ( mode == MR122 ) {

         *( *anap )++ = q_gain_pitch( MR122, *gp_limit, gain_pit, NULL, NULL );
      }
   }

   /*
    * Update target vector for codebook search
    * Find LTP residual
    */
   for ( i = 0; i < L_SUBFR; i++ ) {
      xn2[i] = xn[i] - y1[i] * *gain_pit;
      res2[i] = res2[i] - exc[i] * *gain_pit;
   }
}


/*
 * DotProduct
 *
 *
 * Parameters:
 *    x                 I: first input
 *    y                 I: second input
 *    len               I: length of product
 *
 * Function:
 *    Computes dot product
 *
 * Returns:
 *    acc               dot product
 */
static Float32 DotProduct( Float32 *x, Float32 *y, Word32 len )
{
   Word32 i;
   Float32 acc;


   acc = 0.0F;

   for ( i = 0; i < len; i++ )
      acc += x[i] * y[i];
   return( acc );
}


/*
 * cor_h_x
 *
 *
 * Parameters:
 *    h                 I: impulse response of weighted synthesis filter
 *    x                 I: target
 *    dn                O: correlation between target and impulse response
 *
 * Function:
 *    Computes correlation between target signal and impulse response.
 *
 * Returns:
 *    void
 */
static void cor_h_x( Float32 h[], Float32 x[], Float32 dn[] )
{
   Word32 i;


   dn[0] = (Float32)Dotproduct40( h, x );

   for ( i = 1; i < L_CODE; i++ )
      dn[i] = (Float32)DotProduct( h, &x[i], L_CODE - i );
}


/*
 * set_sign
 *
 *
 * Parameters:
 *    dn                B: correlation between target and h[]
 *    sign              O: sign of dn[]
 *    dn2               O: maximum of correlation in each track
 *    n                 I: # of maximum correlations in dn2[]
 *
 * Function:
 *    Builds sign[] vector.
 *
 * Returns:
 *    void
 */
static void set_sign( Float32 dn[], Float32 sign[], Float32 dn2[], Word16 n )
{
   Float32 val, min;
   Word32 i, j, k, pos = 0;


   /* set sign according to dn[] */
   for ( i = 0; i < L_CODE; i++ ) {
      val = dn[i];

      if ( val >= 0 ) {
         sign[i] = 1.0F;
      }
      else {
         sign[i] = -1.0F;
         val = -val;
      }

      /* modify dn[] according to the fixed sign */
      dn[i] = val;
      dn2[i] = val;
   }

   /* keep 8-n maximum positions/8 of each track and store it in dn2[] */
   for ( i = 0; i < NB_TRACK; i++ ) {
      for ( k = 0; k < ( 8 - n ); k++ ) {
         min = FLT_MAX;

         for ( j = i; j < L_CODE; j += STEP ) {
            if ( dn2[j] >= 0 ) {
               val = dn2[j] - min;

               if ( val < 0 ) {
                  min = dn2[j];
                  pos = j;
               }
            }
         }
         dn2[pos] = -1.0F;
      }
   }
   return;
}


/*
 * cor_h
 *
 *
 * Parameters:
 *    h                I: h[]
 *    sign             I: sign information
 *    rr               O: correlations
 *
 * Function:
 *    Computes correlations of h[] needed for the codebook search,
 *    and includes the sign information into the correlations.
 *
 * Returns:
 *    void
 */
static void cor_h( Float32 h[], Float32 sign[], Float32 rr[][L_CODE] )
{
   Float32 sum;
   Float32 *prr, *ph, *ph_max;
   Float32 *rrj, *rri, *signi, *signj;
   Word32 ii, total_loops, four_loops;


   sum = 0.0F;

   /* Compute diagonal matrix of autocorrelation of h */
   rr[0][0] = (Float32)Dotproduct40( h, h );
   prr = &rr[39][39];
   ph = &h[0];
   ph_max = ph + 39;

   /*
    * speed optimization of code:
    * for (k=0; k<m; k++)
    * {
    * sum += h[k]*h[k];
    * rr[i][i] = sum;
    * i--;
    * }
    */
   do {
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
      sum += *ph * *ph;
      ph++;
      *prr = sum;
      prr -= 41;
   } while ( ph < ph_max );

   /*
    * Compute upper & bottom symmetric matrix of autocorrelation of h.
    */
   /* speed optimization of code:
    * for (ii=1; ii<L_CODE; ii++)
    * {
    * j = m;
    * i = j - ii;
    * sum = (Float32)0.0;
    * for ( k = 0; k < (L_CODE-ii); k++ )
    * {
    * sum += h[k]*h[k+ii];
    * rr[i][j] = rr[j][i] = (sum*sign[i]*sign[j]);
    * i--; j--;
    * }
    * }
    */
   ii = 1;

   for ( total_loops = 9; total_loops >= 0; total_loops-- ) {
      rrj = rri = &rr[39][39];
      rrj -= ii;
      rri = ( rri - 40 * ii );
      signi = signj = &sign[39];
      signi -= ii;
      sum = 0.0F;
      ph = &h[0];

      for ( four_loops = 0; four_loops < total_loops; four_loops++ ) {
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
      }
      sum += *ph * *( ph + ii );
      ph++;
      *rri = *rrj = sum * *signi * *signj;
      rri -= 41;
      rrj -= 41;
      signi--;
      signj--;
      sum += *ph * *( ph + ii );
      ph++;
      *rri = *rrj = sum * *signi * *signj;
      rri -= 41;
      rrj -= 41;
      signi--;
      signj--;
      sum += *ph * *( ph + ii );
      *rri = *rrj = sum * *signi * *signj;
      ii++;
      rrj = rri = &rr[39][39];
      rrj -= ii;
      rri = ( rri - 40 * ii );
      signi = signj = &sign[39];
      signi -= ii;
      sum = 0.0F;
      ph = &h[0];

      for ( four_loops = 0; four_loops < total_loops; four_loops++ ) {
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
      }
      sum += *ph * *( ph + ii );
      ph++;
      *rri = *rrj = sum * *signi * *signj;
      rri -= 41;
      rrj -= 41;
      signi--;
      signj--;
      sum += *ph * *( ph + ii );
      *rri = *rrj = sum * *signi * *signj;
      ii++;
      rrj = rri = &rr[39][39];
      rrj -= ii;
      rri = ( rri - 40 * ii );
      signi = signj = &sign[39];
      signi -= ii;
      sum = 0.0F;
      ph = &h[0];

      for ( four_loops = 0; four_loops < total_loops; four_loops++ ) {
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
      }
      sum += *ph * *( ph + ii );
      *rri = *rrj = sum * *signi * *signj;
      ii++;
      rrj = rri = &rr[39][39];
      rrj -= ii;
      rri = ( rri - 40 * ii );
      signi = signj = &sign[39];
      signi -= ii;
      sum = 0.0F;
      ph = &h[0];

      for ( four_loops = 0; four_loops < total_loops; four_loops++ ) {
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * ( *signi ) * ( *signj );
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
         sum += *ph * *( ph + ii );
         ph++;
         *rri = *rrj = sum * *signi * *signj;
         rri -= 41;
         rrj -= 41;
         signi--;
         signj--;
      }
      ii++;
   }
   return;
}


/*
 * search_2i40_9bits
 *
 *
 * Parameters:
 *    subNr             I: subframe number
 *    dn                I: correlation between target and h[]
 *    rr                I: matrix of autocorrelation
 *    codvec            O: algebraic codebook vector
 *
 * Function:
 *    Search the best codevector; determine positions of the 2 pulses
 *    in the 40-sample frame.
 *
 *    First subframe:
 *    first    i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  2, 7, 12, 17, 22, 27, 32, 37.
 *    second   i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             i1 :  3, 8, 13, 18, 23, 28, 33, 38.
 *
 *    Second subframe:
 *    first    i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  3, 8, 13, 18, 23, 28, 33, 38.
 *    second   i0 :  2, 7, 12, 17, 22, 27, 32, 37.
 *             i1 :  4, 9, 14, 19, 24, 29, 34, 39.
 *
 *    Third subframe:
 *    first    i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  2, 7, 12, 17, 22, 27, 32, 37.
 *    second   i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             i1 :  4, 9, 14, 19, 24, 29, 34, 39.
 *
 *    Fourth subframe:
 *    first    i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  3, 8, 13, 18, 23, 28, 33, 38.
 *    second   i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             i1 :  4, 9, 14, 19, 24, 29, 34, 39.
 *
 * Returns:
 *    void
 */
static void search_2i40_9bits( Word16 subNr, Float32 dn[], Float32 rr[][L_CODE],
      Word32 codvec[] )
{
   Float32 ps0, ps1, psk, alp, alp0, alp1, alpk, sq, sq1;
   Word32 i0, i1, ix, i;
   Word16 ipos[2];
   Word16 track1;


   psk = -1;
   alpk = 1;

   for ( i = 0; i < 2; i++ ) {
      codvec[i] = i;
   }

   /* main loop: try 2x4  tracks	*/
   for ( track1 = 0; track1 < 2; track1++ ) {
      ipos[0] = startPos[( subNr << 1 ) + ( track1 << 3 )];
      ipos[1] = startPos[( subNr << 1 ) + 1 + ( track1 << 3 )];

      /* i0 loop: try 8 positions	*/
      for ( i0 = ipos[0]; i0 < L_CODE; i0 += STEP ) {
         ps0 = dn[i0];
         alp0 = rr[i0][i0];

         /* i1 loop: 8 positions */
         sq = -1;
         alp = 1;
         ix = ipos[1];

         for ( i1 = ipos[1]; i1 < L_CODE; i1 += STEP ) {
            ps1 = ps0 + dn[i1];
            alp1 = alp0 + rr[i1][i1] + 2.0F * rr[i0][i1];
            sq1 = ps1 * ps1;

            if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
               sq = sq1;
               alp = alp1;
               ix = i1;
            }
         }

         /* memorise codevector if this one is better than the last one	*/
         if ( ( alpk * sq ) > ( psk * alp ) ) {
            psk = sq;
            alpk = alp;
            codvec[0] = i0;
            codvec[1] = ix;
         }
      }
   }
   return;
}


/*
 * build_code_2i40_9bits
 *
 *
 * Parameters:
 *    subNr             I: subframe number
 *    codvec            I: position of pulses
 *    dn_sign           I: sign of pulses
 *    cod               O: algebraic codebook vector
 *    h                 I: impulse response of weighted synthesis filter
 *    y                 I: filtered innovative code
 *    anap              O: analysis parameters
 *
 * Function:
 *    Builds the codeword, the filtered codeword and index of the
 *    codevector, based on the signs and positions of 2 pulses.
 *
 * Returns:
 *    void
 */
static void build_code_2i40_9bits( Word16 subNr, Word32 codvec[], Float32
      dn_sign[], Float32 cod[], Float32 h[], Float32 y[], Word16 *anap )
{
   Float32 s;
   Float32 *p0, *p1;
   Word32 _sign[2];
   Word32 i, j, k, track, index, indx = 0, rsign = 0;
   Word8 first, *pt;


   pt = &trackTable[subNr + ( subNr << 2 )];
   memset( cod, 0, 160 );

   for ( k = 0; k < 2; k++ ) {
      /* read pulse position */
      i = codvec[k];

      /* read sign */
      j = ( Word32 )dn_sign[i];

      /* index = pos/5 */
      index = i / 5;

      /* track = pos%5 */
      track = i % 5;
      first = pt[track];

      if ( first == 0 ) {
         if ( k == 0 ) {
            /*	position of 1st pulse	*/
            track = 0;
         }
         else {
            track = 1;

            /*	position of 2nd pulse	*/
            index <<= 3;
         }
      }
      else {
         if ( k == 0 ) {
            track = 0;

            /*	position of 1st pulse, subset 2	*/
            index += 64;
         }
         else {
            track = 1;
            index <<= 3;
         }
      }

      if ( j > 0 ) {
         cod[i] = 0.9998779296875F;
         _sign[k] = 1;

         /*	sign information */
         rsign = rsign + ( 1 << track );
      }
      else {
         cod[i] = -1;
         _sign[k] = -1;
      }
      indx = indx + index;
   }
   p0 = h - codvec[0];
   p1 = h - codvec[1];

   for ( i = 0; i < L_CODE; i++ ) {
      s = *p0++ * _sign[0];
      s += *p1++ * _sign[1];
      y[i] = s;
   }
   anap[0] = ( Word16 )indx;
   anap[1] = ( Word16 )rsign;
}


/*
 * code_2i40_9bits
 *
 *
 * Parameters:
 *    subNr             I: subframe number
 *    x                 I: target vector
 *    h                 I: impulse response of weighted synthesis filter
 *    T0                I: Pitch lag
 *    pitch_sharp       I: Last quantized pitch gain
 *    code              O: innovative codebook
 *    y                 O: filtered fixed codebook excitation
 *    anap              O: analysis parameters
 *
 * Function:
 *    Searches a 9 bit algebraic codebook containing 2 pulses
 *    in a frame of 40 samples.
 *
 *    The code length is 40, containing 2 nonzero pulses: i0...i1.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 can have 8 possible positions, pulse i1 can have
 *    8 positions. Also coded is which track pair should be used,
 *    i.e. first or second pair. Where each pair contains 2 tracks.
 *
 * Returns:
 *    void
 */
static void code_2i40_9bits( Word16 subNr, Float32 x[], Float32 h[], Word32 T0,
      Float32 pitch_sharp, Float32 code[], Float32 y[], Word16 *anap )
{
   Float32 rr[L_CODE][L_CODE];
   Float32 dn[L_CODE], dn_sign[L_CODE], dn2[L_CODE];
   Word32 codvec[2];
   Word32 i;


   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0.0F ) )
      for ( i = T0; i < L_CODE; i++ ) {
         h[i] += h[i - T0] * pitch_sharp;
      }
   cor_h_x( h, x, dn );
   set_sign( dn, dn_sign, dn2, 8 );
   cor_h( h, dn_sign, rr );
   search_2i40_9bits( subNr, dn, rr, codvec );
   build_code_2i40_9bits( subNr, codvec, dn_sign, code, h, y, anap );

      /*
       * Compute innovation vector gain.
       * Include fixed-gain pitch contribution into code[].
       */
   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0.0F ) )
      for ( i = T0; i < L_CODE; i++ ) {
         code[i] += code[i - T0] * pitch_sharp;
      }
}


/*
 * search_2i40_11bits
 *
 *
 * Parameters:
 *    dn                I: correlation between target and h[]
 *    rr                I: matrix of autocorrelation
 *    codvec            O: algebraic codebook vector
 *
 * Function:
 *    Search the best codevector; determine positions of the 2 pulses
 *    in the 40-sample frame.
 *
 * Returns:
 *    void
 */
static void search_2i40_11bits( Float32 dn[], Float32 rr[][L_CODE], Word32
      codvec[] )
{
   Float64 alpk, alp, alp0, alp1;
   Float32 psk, ps0, ps1, sq, sq1;
   Word32 i, i0, i1, ix = 0;
   Word16 ipos[2];
   Word16 track1, track2;


   psk = -1;
   alpk = 1;

   for ( i = 0; i < 2; i++ ) {
      codvec[i] = i;
   }

   /*
    * main loop: try 2x4  tracks.
    */
   for ( track1 = 0; track1 < 2; track1++ ) {
      for ( track2 = 0; track2 < 4; track2++ ) {
         /* fix starting position */
         ipos[0] = startPos1[track1];
         ipos[1] = startPos2[track2];

         /*
          * i0 loop: try 8 positions.
          */
         for ( i0 = ipos[0]; i0 < L_CODE; i0 += STEP ) {
            ps0 = dn[i0];
            alp0 = rr[i0][i0] * 0.25F;

            /*
             * i1 loop: 8 positions.
             */
            sq = -1;
            alp = 1;
            ix = ipos[1];

            for ( i1 = ipos[1]; i1 < L_CODE; i1 += STEP ) {
               ps1 = ps0 + dn[i1];

               /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */
               alp1 = alp0 + rr[i1][i1] * 0.25F;
               alp1 += rr[i0][i1] * 0.5F;
               sq1 = ps1 * ps1;

               if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
                  sq = sq1;
                  alp = alp1;
                  ix = i1;
               }
            }

            /*
             * memorise codevector if this one is better than the last one.
             */
            if ( ( alpk * sq ) > ( psk * alp ) ) {
               psk = sq;
               alpk = alp;
               codvec[0] = i0;
               codvec[1] = ix;
            }
         }
      }
   }
   return;
}


/*
 * build_code_2i40_11bits
 *
 *
 * Parameters:
 *    codvec            I: position of pulses
 *    dn_sign           I: sign of pulses
 *    cod               O: algebraic codebook vector
 *    h                 I: impulse response of weighted synthesis filter
 *    y                 O: filtered innovative code
 *    anap              O: analysis parameters
 *
 * Function:
 *    Builds the codeword, the filtered codeword and index of the
 *    codevector, based on the signs and positions of 2 pulses.
 *
 * Returns:
 *    void
 */
static void build_code_2i40_11bits( Word32 codvec[], Float32 dn_sign[], Float32
      cod[], Float32 h[], Float32 y[], Word16 *anap )
{
   Float64 s;
   Float32 *p0, *p1;
   Word32 _sign[2];
   Word32 i, j, k, track, index, indx = 0, rsign = 0;


   memset( cod, 0, 160 );

   for ( k = 0; k < 2; k++ ) {
      i = codvec[k];   /* read pulse position */
      j = ( Word16 )dn_sign[i];   /* read sign */
      index = i / 5;   /* index = pos/5 */

      /* track = pos%5 */
      track = i % 5;

      if ( track == 0 ) {
         track = 1;
         index = index << 6;
      }
      else if ( track == 1 ) {
         if ( k == 0 ) {
            track = 0;
            index = index << 1;
         }
         else {
            track = 1;
            index = ( index << 6 ) + 16;
         }
      }
      else if ( track == 2 ) {
         track = 1;
         index = ( index << 6 ) + 32;
      }
      else if ( track == 3 ) {
         track = 0;
         index = ( index << 1 ) + 1;
      }
      else if ( track == 4 ) {
         track = 1;
         index = ( index << 6 ) + 48;
      }

      if ( j > 0 ) {
         cod[i] = 0.9998779296875F;
         _sign[k] = 1;
         rsign = rsign + ( 1 << track );
      }
      else {
         cod[i] = -1;
         _sign[k] = -1;
      }
      indx = indx + index;
   }
   p0 = h - codvec[0];
   p1 = h - codvec[1];

   for ( i = 0; i < L_CODE; i++ ) {
      s = *p0++ * _sign[0];
      s += *p1++ * _sign[1];
      y[i] = ( Float32 )s;
   }
   anap[0] = ( Word16 )indx;
   anap[1] = ( Word16 )rsign;
}


/*
 * code_2i40_11bits
 *
 *
 * Parameters:
 *    x                 I: target vector
 *    h                 I: impulse response of weighted synthesis filter
 *    T0                I: Pitch lag
 *    pitch_sharp       I: Last quantized pitch gain
 *    code              O: innovative codebook
 *    y                 O: filtered fixed codebook excitation
 *    anap              O: analysis parameters
 *
 * Function:
 *    Searches a 11 bit algebraic codebook containing 2 pulses
 *    in a frame of 40 samples.
 *
 *    The code length is 40, containing 2 nonzero pulses: i0...i1.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 can have 2x8=16 possible positions, pulse i1 can have
 *    4x8=32 positions.
 *
 *    i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *          3, 8, 13, 18, 23, 28, 33, 38.
 *    i1 :  0, 5, 10, 15, 20, 25, 30, 35.
 *          1, 6, 11, 16, 21, 26, 31, 36.
 *          2, 7, 12, 17, 22, 27, 32, 37.
 *          4, 9, 14, 19, 24, 29, 34, 39.
 *
 * Returns:
 *    void
 */
static void code_2i40_11bits( Float32 x[], Float32 h[], Word32 T0, Float32
      pitch_sharp, Float32 code[], Float32 y[], Word16 *anap )
{
   Float32 rr[L_CODE][L_CODE];
   Float32 dn[L_CODE], dn2[L_CODE], dn_sign[L_CODE];
   Word32 codvec[2];
   Word32 i;


   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0.0F ) ) {
      for ( i = T0; i < L_CODE; i++ ) {
         h[i] = h[i] + ( h[i - T0] * pitch_sharp );
      }
   }
   cor_h_x( h, x, dn );
   set_sign( dn, dn_sign, dn2, 8 );
   cor_h( h, dn_sign, rr );
   search_2i40_11bits( dn, rr, codvec );
   build_code_2i40_11bits( codvec, dn_sign, code, h, y, anap );

   /*
    * Compute innovation vector gain.
    * Include fixed-gain pitch contribution into code[].
    */
   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0.0F ) ) {
      for ( i = T0; i < L_CODE; i++ ) {
         code[i] = code[i] + code[i - T0] * pitch_sharp;
      }
   }
}


/*
 * search_3i40
 *
 *
 * Parameters:
 *    dn                I: correlation between target and h[]
 *    dn2               I: maximum of corr. in each track
 *    rr                I: matrix of autocorrelation
 *    codvec            O: algebraic codebook vector
 *
 * Function:
 *    Searches a 14 bit algebraic codebook containing 3 pulses in
 *    a frame of 40 samples.
 *
 * Returns:
 *    void
 */
static void search_3i40( Float32 dn[], Float32 dn2[], Float32 rr[][L_CODE],
      Word32 codvec[] )
{
   Float32 psk, ps0, ps1, sq, sq1, alpk, alp, alp0, alp1, ps = 0.0F;
   Float32 *rr2, *rr1, *rr0, *pdn, *pdn_max;
   Word32 ipos[3];
   Word32 i0, i1, i2, ix, i, pos, track1, track2;


   psk = -1.0F;
   alpk = 1.0F;

   for ( track1 = 1; track1 < 4; track1 += 2 ) {
      for ( track2 = 2; track2 < 5; track2 += 2 ) {
         /* fix starting position */
         ipos[0] = 0;
         ipos[1] = track1;
         ipos[2] = track2;

         /* main loop: try 3 tracks */
         for ( i = 0; i < 3; i++ ) {
            /* i0 loop: try 8 positions */
            for ( i0 = ipos[0]; i0 < L_CODE; i0 += STEP ) {
               if ( dn2[i0] >= 0 ) {
                  ps0 = dn[i0];
                  alp0 = rr[i0][i0];

                  /* i1 loop: 8 positions */
                  sq = -1.0F;
                  alp = 1.0F;
                  ps = 0.0F;
                  ix = ipos[1];
                  i1 = ipos[1];
                  rr1 = &rr[i1][i1];
                  rr0 = &rr[i0][i1];
                  pdn = &dn[i1];
                  pdn_max = &dn[L_CODE];

                  do {
                     ps1 = ps0 + *pdn;
                     alp1 = alp0 + *rr1 + 2.0F * *rr0;
                     sq1 = ps1 * ps1;

                     if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
                        sq = sq1;
                        ps = ps1;
                        alp = alp1;
                        ix = ( Word16 )( pdn - dn );
                     }
                     pdn += STEP;
                     rr1 += ( 40 * STEP + STEP );
                     rr0 += STEP;
                  } while ( pdn < pdn_max );
                  i1 = ix;

                  /* i2 loop: 8 positions */
                  ps0 = ps;
                  alp0 = alp;
                  sq = -1.0F;
                  alp = 1.0F;
                  ps = 0.0F;
                  ix = ipos[2];
                  i2 = ipos[2];
                  rr2 = &rr[i2][i2];
                  rr1 = &rr[i1][i2];
                  rr0 = &rr[i0][i2];
                  pdn = &dn[i2];

                  do {
                     ps1 = ps0 + *pdn;
                     alp1 = alp0 + *rr2 + 2.0F * ( *rr1 + *rr0 );
                     sq1 = ps1 * ps1;

                     if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
                        sq = sq1;
                        ps = ps1;
                        alp = alp1;
                        ix = ( Word16 )( pdn - dn );
                     }
                     pdn += STEP;
                     rr2 += ( 40 * STEP + STEP );
                     rr1 += STEP;
                     rr0 += STEP;
                  } while ( pdn < pdn_max );
                  i2 = ix;

                  /* memorise codevector if this one is better than the last one */
                  if ( ( alpk * sq ) > ( psk * alp ) ) {
                     psk = sq;
                     alpk = alp;
                     codvec[0] = i0;
                     codvec[1] = i1;
                     codvec[2] = i2;
                  }
               }
            }

            /* Cyclic permutation of i0, i1 and i2 */
            pos = ipos[2];
            ipos[2] = ipos[1];
            ipos[1] = ipos[0];
            ipos[0] = pos;
         }
      }
   }
   return;
}


/*
 * build_code_3i40_14bits
 *
 *
 * Parameters:
 *    codvec            I: position of pulses
 *    dn_sign           I: sign of pulses
 *    cod               O: algebraic codebook vector
 *    h                 I: impulse response of weighted synthesis filter
 *    y                 I: filtered innovative code
 *    anap              O: analysis parameters
 *
 * Function:
 *    Builds the codeword, the filtered codeword and index of the
 *    codevector, based on the signs and positions of 3 pulses.
 *
 * Returns:
 *    void
 */
static void build_code_3i40_14bits( Word32 codvec[], Float32 dn_sign[], Float32
      cod[], Float32 h[], Float32 y[], Word16 *anap )
{
   Float64 s;
   Float32 _sign[3];
   Float32 *p0, *p1, *p2;
   Word32 i, j, k, track, index, indx, rsign;


   memset( cod, 0, 160 );
   indx = 0;
   rsign = 0;

   for ( k = 0; k < 3; k++ ) {
      /* read pulse position */
      i = codvec[k];

      /* read sign */
      j = ( Word16 )dn_sign[i];

      /* index = pos/5 */
      index = i / 5;

      /* track = pos%5 */
      track = i % 5;

      if ( track == 1 )
         index = index << 4;
      else if ( track == 2 ) {
         track = 2;
         index = index << 8;
      }
      else if ( track == 3 ) {
         track = 1;
         index = ( index << 4 ) + 8;
      }
      else if ( track == 4 ) {
         track = 2;
         index = ( index << 8 ) + 128;
      }

      if ( j > 0 ) {
         cod[i] = 0.9998779296875F;
         _sign[k] = 1.0F;
         rsign = rsign + ( 1 << track );
      }
      else {
         cod[i] = -1.0F;
         _sign[k] = -1.0F;
      }
      indx = indx + index;
   }
   p0 = h - codvec[0];
   p1 = h - codvec[1];
   p2 = h - codvec[2];

   for ( i = 0; i < L_CODE; i++ ) {
      s = *p0++ * _sign[0];
      s += *p1++ * _sign[1];
      s += *p2++ * _sign[2];
      y[i] = ( Float32 )s;
   }
   anap[0] = ( Word16 )indx;
   anap[1] = ( Word16 )rsign;
}


/*
 * code_3i40_14bits
 *
 *
 * Parameters:
 *    x                 I: target vector
 *    h                 I: impulse response of weighted synthesis filter
 *    T0                I: Pitch lag
 *    pitch_sharp       I: Last quantized pitch gain
 *    code              O: innovative codebook
 *    y                 O: filtered fixed codebook excitation
 *    anap              O: analysis parameters
 *
 * Function:
 *    Searches a 14 bit algebraic codebook containing 3 pulses
 *    in a frame of 40 samples.
 *
 *    The code length is 40, containing 3 nonzero pulses: i0...i2.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 can have 8 possible positions, pulses i1 and i2 can have
 *    2x8=16 positions.
 *
 *       i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *       i1 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             3, 8, 13, 18, 23, 28, 33, 38.
 *       i2 :  2, 7, 12, 17, 22, 27, 32, 37.
 *             4, 9, 14, 19, 24, 29, 34, 39.
 *
 * Returns:
 *    void
 */
static void code_3i40_14bits( Float32 x[], Float32 h[], Word32 T0, Float32
      pitch_sharp, Float32 code[], Float32 y[], Word16 *anap )
{
   Float32 rr[L_CODE][L_CODE];
   Float32 dn[L_CODE], dn2[L_CODE], dn_sign[L_CODE];
   Word32 codvec[3];
   Word32 i;


   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0 ) ) {
      for ( i = T0; i < L_CODE; i++ ) {
         h[i] = h[i] + ( h[i - T0] * pitch_sharp );
      }
   }
   cor_h_x( h, x, dn );
   set_sign( dn, dn_sign, dn2, 6 );
   cor_h( h, dn_sign, rr );
   search_3i40( dn, dn2, rr, codvec );

   /* function result */
   build_code_3i40_14bits( codvec, dn_sign, code, h, y, anap );

   /*
    * Compute innovation vector gain.
    * Include fixed-gain pitch contribution into code[].
    */
   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0 ) ) {
      for ( i = T0; i < L_CODE; i++ ) {
         code[i] = code[i] + ( code[i - T0] * pitch_sharp );
      }
   }
}


/*
 * search_4i40
 *
 *
 * Parameters:
 *    dn                I: correlation between target and h[]
 *    dn2               I: maximum of corr. in each track.
 *    rr                I: matrix of autocorrelation
 *    codvec            O: algebraic codebook vector
 *
 * Function:
 *    Search the best codevector; determine positions of the 4 pulses
 *    in the 40-sample frame.
 *
 * Returns:
 *    void
 */
static void search_4i40( Float32 dn[], Float32 dn2[], Float32 rr[][L_CODE],
      Word32 codvec[] )
{
   Float64 alpk, alp, alp0, alp1;
   Float32 ps, psk, ps0, ps1, sq, sq1;
   Word32 ipos[4];
   Word32 i0, i1, i2, i3, ix, i, pos, track;


   /* Default value */
   psk = -1;
   alpk = 1;

   for ( i = 0; i < 4; i++ ) {
      codvec[i] = i;
   }

   for ( track = 3; track < 5; track++ ) {
      /* fix starting position */
      ipos[0] = 0;
      ipos[1] = 1;
      ipos[2] = 2;
      ipos[3] = track;

      /*
       * main loop: try 4 tracks.
       */
      for ( i = 0; i < 4; i++ ) {
      /*
       * i0 loop: try 4 positions (use position with max of corr.).
       */
         for ( i0 = ipos[0]; i0 < L_CODE; i0 += STEP ) {
            if ( dn2[i0] >= 0 ) {
               ps0 = dn[i0];
               alp0 = rr[i0][i0] * 0.25F;

               /*
                * i1 loop: 8 positions.
                */
               sq = -1;
               alp = 1;
               ps = 0;
               ix = ipos[1];

               for ( i1 = ipos[1]; i1 < L_CODE; i1 += STEP ) {
                  ps1 = ps0 + dn[i1];

                  /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */
                  alp1 = alp0 + rr[i1][i1] * 0.25F;
                  alp1 = alp1 + rr[i0][i1] * 0.5F;
                  sq1 = ps1 * ps1;

                  if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
                     sq = sq1;
                     ps = ps1;
                     alp = alp1;
                     ix = i1;
                  }
               }
               i1 = ix;

               /*
                * i2 loop: 8 positions.
                */
               ps0 = ps;
               alp0 = alp * 0.25F;
               sq = -1;
               alp = 1;
               ps = 0;
               ix = ipos[2];

               for ( i2 = ipos[2]; i2 < L_CODE; i2 += STEP ) {
                  ps1 = ps0 + dn[i2];

                  /* alp1 = alp0 + rr[i0][i2] + rr[i1][i2] + 1/2*rr[i2][i2]; */
                  alp1 = alp0 + rr[i2][i2] * 0.0625F;
                  alp1 += rr[i1][i2] * 0.125F;
                  alp1 += rr[i0][i2] * 0.125F;
                  sq1 = ps1 * ps1;

                  if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
                     sq = sq1;
                     ps = ps1;
                     alp = alp1;
                     ix = i2;
                  }
               }
               i2 = ix;

               /*
                * i3 loop: 8 positions
                */
               ps0 = ps;
               alp0 = alp;
               sq = -1;
               alp = 1;
               ps = 0;
               ix = ipos[3];

               for ( i3 = ipos[3]; i3 < L_CODE; i3 += STEP ) {
                  ps1 = ps0 + dn[i3];

                  /* alp1 = alp0 + rr[i0][i3] + rr[i1][i3] + rr[i2][i3] + 1/2*rr[i3][i3]; */
                  alp1 = alp0 + rr[i3][i3] * 0.0625F;
                  alp1 += rr[i2][i3] * 0.125F;
                  alp1 += rr[i1][i3] * 0.125F;
                  alp1 += rr[i0][i3] * 0.125F;
                  sq1 = ps1 * ps1;

                  if ( ( alp * sq1 ) > ( sq * alp1 ) ) {
                     sq = sq1;
                     ps = ps1;
                     alp = alp1;
                     ix = i3;
                  }
               }

               /*
                * memorise codevector if this one is better than the last one.
                */
               if ( ( alpk * sq ) > ( psk * alp ) ) {
                  psk = sq;
                  alpk = alp;
                  codvec[0] = i0;
                  codvec[1] = i1;
                  codvec[2] = i2;
                  codvec[3] = ix;
               }
            }
         }

          /*
           * Cyclic permutation of i0,i1,i2 and i3.
           */
         pos = ipos[3];
         ipos[3] = ipos[2];
         ipos[2] = ipos[1];
         ipos[1] = ipos[0];
         ipos[0] = pos;
      }
   }
   return;
}


/*
 * build_code_4i40
 *
 *
 * Parameters:
 *    codvec            I: position of pulses
 *    dn_sign           I: sign of pulses
 *    cod               O: algebraic codebook vector
 *    h                 I: impulse response of weighted synthesis filter
 *    y                 O: filtered innovative code
 *    anap              O: analysis parameters
 *
 * Function:
 *    Builds the codeword, the filtered codeword and index of the
 *    codevector, based on the signs and positions of 4 pulses.
 *
 * Returns:
 *    void
 */
static void build_code_4i40( Word32 codvec[], Float32 dn_sign[], Float32 cod[],
      Float32 h[], Float32 y[], Word16 *anap )
{
   Float64 s;
   Float32 *p0, *p1, *p2, *p3;
   Word32 _sign[4];
   Word32 i, j, k, track, index, indx = 0, rsign = 0;


   memset( cod, 0, 160 );

   for ( k = 0; k < 4; k++ ) {
      /* read pulse position */
      i = codvec[k];

      /* read sign */
      j = ( Word16 )dn_sign[i];
      index = i / 5;
      track = i % 5;
      index = gray[index];

      if ( track == 1 )
         index = index << 3;
      else if ( track == 2 ) {
         index = index << 6;
      }
      else if ( track == 3 ) {
         index = index << 10;
      }
      else if ( track == 4 ) {
         track = 3;
         index = ( index << 10 ) + 512;
      }

      if ( j > 0 ) {
         cod[i] = 0.9998779296875F;
         _sign[k] = 1;
         rsign = rsign + ( 1 << track );
      }
      else {
         cod[i] = -1;
         _sign[k] = -1;
      }
      indx = indx + index;
   }
   p0 = h - codvec[0];
   p1 = h - codvec[1];
   p2 = h - codvec[2];
   p3 = h - codvec[3];

   for ( i = 0; i < L_CODE; i++ ) {
      s = *p0++ * _sign[0];
      s += *p1++ * _sign[1];
      s += *p2++ * _sign[2];
      s += *p3++ * _sign[3];
      y[i] = ( Float32 )( s );
   }
   anap[0] = ( Word16 )indx;
   anap[1] = ( Word16 )rsign;
}


/*
 * code_4i40_17bits
 *
 *
 * Parameters:
 *    x                 I: target vector
 *    h                 I: impulse response of weighted synthesis filter
 *    T0                I: Pitch lag
 *    pitch_sharp       I: Last quantized pitch gain
 *    code              O: innovative codebook
 *    y                 O: filtered fixed codebook excitation
 *    anap              O: analysis parameters
 *
 * Function:
 *    Searches a 17 bit algebraic codebook containing 4 pulses
 *    in a frame of 40 samples.
 *
 *    The code length is 40, containing 4 nonzero pulses: i0...i3.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 to i2 can have 8 possible positions, pulse i3 can have
 *    2x8=16 positions.
 *
 *       i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *       i1 :  1, 6, 11, 16, 21, 26, 31, 36.
 *       i2 :  2, 7, 12, 17, 22, 27, 32, 37.
 *       i3 :  3, 8, 13, 18, 23, 28, 33, 38.
 *             4, 9, 14, 19, 24, 29, 34, 39.
 *
 * Returns:
 *    void
 */
static void code_4i40_17bits( Float32 x[], Float32 h[], Word32 T0, Float32
      pitch_sharp, Float32 code[], Float32 y[], Word16 *anap )
{
   Float32 rr[L_CODE][L_CODE];
   Float32 dn[L_CODE], dn2[L_CODE], dn_sign[L_CODE];
   Word32 codvec[4];
   Word32 i;


   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0 ) ) {
      for ( i = T0; i < L_CODE; i++ ) {
         h[i] = h[i] + h[i - T0] * pitch_sharp;
      }
   }
   cor_h_x( h, x, dn );
   set_sign( dn, dn_sign, dn2, 4 );
   cor_h( h, dn_sign, rr );
   search_4i40( dn, dn2, rr, codvec );
   build_code_4i40( codvec, dn_sign, code, h, y, anap );

   /*
    * Compute innovation vector gain.
    * Include fixed-gain pitch contribution into code[].
    */
   if ( ( T0 < L_CODE ) && ( pitch_sharp != 0 ) ) {
      for ( i = T0; i < L_CODE; i++ ) {
         code[i] = code[i] + ( code[i - T0] * pitch_sharp );
      }
   }
}


/*
 * set_sign12k2
 *
 *
 * Parameters:
 *    dn                B: correlation between target and h[]
 *    cn                I: residual after long term prediction
 *    sign              O: sign of dn[]
 *    pos_max           O: position of maximum correlation
 *    nb_track          I: number of tracks
 *    ipos              O: starting position for each pulse
 *    step              I: the step size in the tracks
 *
 * Function:
 *    Builds sign[] vector according to "dn[]" and "cn[]", and modifies
 *    dn[] to include the sign information (dn[i]=sign[i]*dn[i]).
 *    Also finds the position of maximum of correlation in each track
 *    and the starting position for each pulse.
 *
 *
 *                  cn[n]                     dn[n]
 *    b(n) = ----------------------- + -----------------------  ;n = 0,..,39
 *                   39                        39
 *            sqrt( SUM cn[i]*cn[i] )   sqrt( SUM dn[i]*dn[i] )
 *                  i=0                       i=0
 *
 *    sign[n] = sign(b[n])
 *
 *    d'[n] = sign[n] * d[n]
 *
 * Returns:
 *    void
 */
static void set_sign12k2( Float32 dn[], Float32 cn[], Float32 sign[], Word32
      pos_max[], Word16 nb_track, Word32 ipos[], Word16 step )
{
   Float32 b[L_CODE];
   Float32 val, cor, k_cn, k_dn, max, max_of_all, sum;
   Word32 i, j, pos = 0;


   /* calculate energy for normalization of cn[] and dn[] */
   sum = 0.01F;
   sum += (Float32)Dotproduct40( cn, cn );
   k_cn = ( Float32 )( 1 / sqrt( sum ) );
   sum = 0.01F;
   sum += (Float32)Dotproduct40( dn, dn );
   k_dn = ( Float32 )( 1 / sqrt( sum ) );

   for ( i = 0; i < L_CODE; i++ ) {
      val = dn[i];
      cor = ( k_cn * cn[i] ) + ( k_dn * val );

      /* sign = +1 */
      sign[i] = 1;

      if ( cor < 0 ) {
         /* sign = -1 */
         sign[i] = -1;
         cor = -cor;
         val = -val;
      }

      /* modify dn[] according to the fixed sign */
      dn[i] = val;
      b[i] = cor;
   }
   max_of_all = -1;

   for ( i = 0; i < nb_track; i++ ) {
      max = -1;

      for ( j = i; j < L_CODE; j += step ) {
         cor = b[j];
         val = cor - max;

         if ( val > 0 ) {
            max = cor;
            pos = j;
         }
      }

      /* store maximum correlation position in track */
      pos_max[i] = pos;
      val = max - max_of_all;

      if ( val > 0 ) {
         /* store maximum correlation of all tracks */
         max_of_all = max;

         /* starting position for i0 */
         ipos[0] = i;
      }
   }

   /* Set starting position of each pulse	*/
   pos = ipos[0];
   ipos[nb_track] = pos;

   for ( i = 1; i < nb_track; i++ ) {
      pos++;

      if ( pos >= nb_track ) {
         pos = 0;
      }
      ipos[i] = pos;
      ipos[i + nb_track] = pos;
   }
}


/*
 * search_4i40
 *
 *
 * Parameters:
 *    dn                I: correlation between target and h[]
 *    rr                I: matrix of autocorrelation
 *    ipos              I: starting position for each pulse
 *    pos_max           I: maximum of correlation position
 *    codvec            O: algebraic codebook vector
 *
 * Function:
 *    Search the best codevector; determine positions of the 8 pulses
 *    in the 40-sample frame.
 *
 * Returns:
 *    void
 */
static void search_8i40( Float32 dn[], Float32 rr[][L_CODE], Word32 ipos[],
      Word32 pos_max[], Word32 codvec[] )
{
   Float32 rrv[L_CODE];
   Float32 psk, ps, ps0, ps1, ps2, sq, sq2, alpk, alp, alp0, alp1, alp2;
   Float32 *p_r, *p_r0, *p_r1, *p_r2, *p_r3, *p_r4, *p_r5, *p_r6, *p_r7, *p_r8;
   Float32 *p_rrv, *p_rrv0, *p_dn, *p_dn0, *p_dn1, *p_dn_max;
   Word32 i0, i1, i2, i3, i4, i5, i6, i7, j, k, ia, ib, i, pos;

   p_dn_max = &dn[39];

   /* fix i0 on maximum of correlation position */
   i0 = pos_max[ipos[0]];
   ia = ib = 0;
   ps = 0;

   /* i1 loop */
   /* Default value */
   psk = -1;
   alpk = 1;

   for ( i = 0; i < 8; i++ ) {
      codvec[i] = i;
   }
   p_r = &rr[i0][i0];

   for ( i = 1; i < 5; i++ ) {
      i1 = pos_max[ipos[1]];
      i2 = ipos[2];
      i3 = ipos[3];
      i4 = ipos[4];
      i5 = ipos[5];
      i6 = ipos[6];
      i7 = ipos[7];
      ps0 = dn[i0] + dn[i1];
      alp0 = *p_r + rr[i1][i1] + 2.0F * rr[i0][i1];

      /* i2 and i3 loop	*/
      p_rrv = &rrv[i3];
      p_r0 = &rr[i0][i3];
      p_r1 = &rr[i1][i3];
      p_r3 = &rr[i3][i3];
      *p_rrv = *p_r3 + 2.0F * ( *p_r0 + *p_r1 );
      *( p_rrv + 4 ) = *( p_r3 + 164 ) + 2.0F * ( *( p_r0 + 4 ) + *( p_r1 + 4 )
            );
      *( p_rrv + 8 ) = *( p_r3 + 328 ) + 2.0F * ( *( p_r0 + 8 ) + *( p_r1 + 8 )
            );
      *( p_rrv + 12 ) = *( p_r3 + 492 ) + 2.0F * ( *( p_r0 + 12 ) + *( p_r1 + 12
            ) );
      *( p_rrv + 16 ) = *( p_r3 + 656 ) + 2.0F * ( *( p_r0 + 16 ) + *( p_r1 + 16
            ) );
      *( p_rrv + 20 ) = *( p_r3 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) );
      *( p_rrv + 24 ) = *( p_r3 + 984 ) + 2.0F * ( *( p_r0 + 24 ) + *( p_r1 + 24
            ) );
      *( p_rrv + 28 ) = *( p_r3 + 1148 ) + 2.0F * ( *( p_r0 + 28 ) + *( p_r1 +
            28 ) );
      *( p_rrv + 32 ) = *( p_r3 + 1312 ) + 2.0F * ( *( p_r0 + 32 ) + *( p_r1 +
            32 ) );
      *( p_rrv + 36 ) = *( p_r3 + 1476 ) + 2.0F * ( *( p_r0 + 36 ) + *( p_r1 +
            36 ) );
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i2;
      ib = i3;
      p_rrv = rrv + i3;
      p_r0 = &rr[i0][i2];
      p_r1 = &rr[i1][i2];
      p_r2 = &rr[i2][i2];
      p_r3 = &rr[i2][i3];
      p_dn0 = dn + i2;
      p_dn1 = dn + i3;
      p_rrv0 = rrv + i3;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r2 + 2.0F * ( *p_r0 + *p_r1 );
         p_rrv = p_rrv0;
         p_dn = p_dn1;
         p_r4 = p_r3;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r4;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = p_dn0 - dn;
               ib = p_dn - dn;
            }
            p_rrv += 4;
            p_dn += 4;
            p_r4 += 4;
         } while ( p_dn < p_dn_max );
         p_dn0 += 4;
         p_r0 += 4;
         p_r1 += 4;
         p_r2 += 164;
         p_r3 += 160;
      } while ( p_dn0 <= p_dn_max );
      i2 = ia;
      i3 = ib;

      /* i4 and i5 loop	*/
      p_rrv = rrv + i5;
      p_r0 = &rr[i0][i5];
      p_r1 = &rr[i1][i5];
      p_r2 = &rr[i2][i5];
      p_r3 = &rr[i3][i5];
      p_r5 = &rr[i5][i5];
      *p_rrv = *p_r5 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 );
      *( p_rrv + 4 ) = *( p_r5 + 164 ) + 2.0F * ( *( p_r0 + 4 ) + *( p_r1 + 4 )
            + *( p_r2 + 4 ) + *( p_r3 + 4 ) );
      *( p_rrv + 8 ) = *( p_r5 + 328 ) + 2.0F * ( *( p_r0 + 8 ) + *( p_r1 + 8 )
            + *( p_r2 + 8 ) + *( p_r3 + 8 ) );
      *( p_rrv + 12 ) = *( p_r5 + 492 ) + 2.0F * ( *( p_r0 + 12 ) + *( p_r1 + 12
            ) + *( p_r2 + 12 ) + *( p_r3 + 12 ) );
      *( p_rrv + 16 ) = *( p_r5 + 656 ) + 2.0F * ( *( p_r0 + 16 ) + *( p_r1 + 16
            ) + *( p_r2 + 16 ) + *( p_r3 + 16 ) );
      *( p_rrv + 20 ) = *( p_r5 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) + *( p_r2 + 20 ) + *( p_r3 + 20 ) );
      *( p_rrv + 24 ) = *( p_r5 + 984 ) + 2.0F * ( *( p_r0 + 24 ) + *( p_r1 + 24
            ) + *( p_r2 + 24 ) + *( p_r3 + 24 ) );
      *( p_rrv + 28 ) = *( p_r5 + 1148 ) + 2.0F * ( *( p_r0 + 28 ) + *( p_r1 +
            28 ) + *( p_r2 + 28 ) + *( p_r3 + 28 ) );
      *( p_rrv + 32 ) = *( p_r5 + 1312 ) + 2.0F * ( *( p_r0 + 32 ) + *( p_r1 +
            32 ) + *( p_r2 + 32 ) + *( p_r3 + 32 ) );
      *( p_rrv + 36 ) = *( p_r5 + 1476 ) + 2.0F * ( *( p_r0 + 36 ) + *( p_r1 +
            36 ) + *( p_r2 + 36 ) + *( p_r3 + 36 ) );

      /* Default value */
      ps0 = ps;
      alp0 = alp;
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i4;
      ib = i5;
      p_dn0 = dn + i4;
      p_dn1 = dn + i5;
      p_r0 = &rr[i0][i4];
      p_r1 = &rr[i1][i4];
      p_r2 = &rr[i2][i4];
      p_r3 = &rr[i3][i4];
      p_r4 = &rr[i4][i4];
      p_r5 = &rr[i4][i5];
      p_rrv0 = rrv + i5;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r4 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 );
         p_dn = p_dn1;
         p_r6 = p_r5;
         p_rrv = p_rrv0;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r6;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = p_dn0 - dn;
               ib = p_dn - dn;
            }
            p_dn += 4;
            p_rrv += 4;
            p_r6 += 4;
         } while ( p_dn <= p_dn_max );
         p_r0 += 4;
         p_r1 += 4;
         p_r2 += 4;
         p_r3 += 4;
         p_r4 += 164;
         p_r5 += 160;
         p_dn0 += 4;
      } while ( p_dn0 < p_dn_max );
      i4 = ia;
      i5 = ib;

      /* i6 and i7 loop	*/
      p_rrv = rrv + i7;
      p_r0 = &rr[i0][i7];
      p_r1 = &rr[i1][i7];
      p_r2 = &rr[i2][i7];
      p_r3 = &rr[i3][i7];
      p_r4 = &rr[i4][i7];
      p_r5 = &rr[i5][i7];
      p_r7 = &rr[i7][i7];
      *p_rrv = *p_r7 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 + *p_r4 + *p_r5 );
      *( p_rrv + 4 ) = *( p_r7 + 164 ) + 2.0F * ( *( p_r0 + 4 ) + *( p_r1 + 4 )
            + *( p_r2 + 4 ) + *( p_r3 + 4 ) + *( p_r4 + 4 ) + *( p_r5 + 4 ) );
      *( p_rrv + 8 ) = *( p_r7 + 328 ) + 2.0F * ( *( p_r0 + 8 ) + *( p_r1 + 8 )
            + *( p_r2 + 8 ) + *( p_r3 + 8 ) + *( p_r4 + 8 ) + *( p_r5 + 8 ) );
      *( p_rrv + 12 ) = *( p_r7 + 492 ) + 2.0F * ( *( p_r0 + 12 ) + *( p_r1 + 12
            ) + *( p_r2 + 12 ) + *( p_r3 + 12 ) + *( p_r4 + 12 ) + *( p_r5 + 12
            ) );
      *( p_rrv + 16 ) = *( p_r7 + 656 ) + 2.0F * ( *( p_r0 + 16 ) + *( p_r1 + 16
            ) + *( p_r2 + 16 ) + *( p_r3 + 16 ) + *( p_r4 + 16 ) + *( p_r5 + 16
            ) );
      *( p_rrv + 20 ) = *( p_r7 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) + *( p_r2 + 20 ) + *( p_r3 + 20 ) + *( p_r4 + 20 ) + *( p_r5 + 20
            ) );
      *( p_rrv + 24 ) = *( p_r7 + 984 ) + 2.0F * ( *( p_r0 + 24 ) + *( p_r1 + 24
            ) + *( p_r2 + 24 ) + *( p_r3 + 24 ) + *( p_r4 + 24 ) + *( p_r5 + 24
            ) );
      *( p_rrv + 28 ) = *( p_r7 + 1148 ) + 2.0F * ( *( p_r0 + 28 ) + *( p_r1 +
            28 ) + *( p_r2 + 28 ) + *( p_r3 + 28 ) + *( p_r4 + 28 ) + *( p_r5 +
            28 ) );
      *( p_rrv + 32 ) = *( p_r7 + 1312 ) + 2.0F * ( *( p_r0 + 32 ) + *( p_r1 +
            32 ) + *( p_r2 + 32 ) + *( p_r3 + 32 ) + *( p_r4 + 32 ) + *( p_r5 +
            32 ) );
      *( p_rrv + 36 ) = *( p_r7 + 1476 ) + 2.0F * ( *( p_r0 + 36 ) + *( p_r1 +
            36 ) + *( p_r2 + 36 ) + *( p_r3 + 36 ) + *( p_r4 + 36 ) + *( p_r5 +
            36 ) );

      /* Default value */
      ps0 = ps;
      alp0 = alp;
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i6;
      ib = i7;
      p_dn0 = dn + i6;
      p_dn1 = dn + i7;
      p_r0 = &rr[i0][i6];
      p_r1 = &rr[i1][i6];
      p_r2 = &rr[i2][i6];
      p_r3 = &rr[i3][i6];
      p_r4 = &rr[i4][i6];
      p_r5 = &rr[i5][i6];
      p_r6 = &rr[i6][i6];
      p_r7 = &rr[i6][i7];
      p_rrv0 = rrv + i7;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r6 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 + *p_r4 +
               *p_r5 );
         p_dn = p_dn1;
         p_r8 = p_r7;
         p_rrv = p_rrv0;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r8;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = p_dn0 - dn;
               ib = p_dn - dn;
            }
            p_dn += 4;
            p_rrv += 4;
            p_r8 += 4;
         } while ( p_dn <= p_dn_max );
         p_r0 += 4;
         p_r1 += 4;
         p_r2 += 4;
         p_r3 += 4;
         p_r4 += 4;
         p_r5 += 4;
         p_r6 += 164;
         p_r7 += 160;
         p_dn0 += 4;
      } while ( p_dn0 < p_dn_max );

      /*
       * now finished searching a set of 8 pulses
       * test and memorise if this combination is better than the last one.
       */
      if ( ( alpk * sq ) > ( psk * alp ) ) {
         psk = sq;
         alpk = alp;
         codvec[0] = ( Word16 )i0;
         codvec[1] = ( Word16 )i1;
         codvec[2] = ( Word16 )i2;
         codvec[3] = ( Word16 )i3;
         codvec[4] = ( Word16 )i4;
         codvec[5] = ( Word16 )i5;
         codvec[6] = ( Word16 )ia;
         codvec[7] = ( Word16 )ib;
      }

      /*
       * Cyclic permutation of i1,i2,i3,i4,i5,i6,i7,(i8 and i9).
       */
      pos = ipos[1];

      for ( j = 1, k = 2; k < 8; j++, k++ ) {
         ipos[j] = ipos[k];
      }
      ipos[7] = pos;
   }   /* end 1..nb_tracks  loop*/
}


/*
 * build_code_8i40_31bits
 *
 *
 * Parameters:
 *    codvec            I: position of pulses
 *    dn_sign           I: sign of pulses
 *    cod               O: algebraic codebook vector
 *    h                 I: impulse response of weighted synthesis filter
 *    y                 O: filtered innovative code
 *    sign_indx         O: signs of 4  pulses (signs only)
 *    pos_indx          O: position index of 8 pulses (position only)
 *
 * Function:
 *    Builds the codeword, the filtered codeword and a
 *    linear uncombined version of  the index of the
 *    codevector, based on the signs and positions of 8  pulses.
 *
 * Returns:
 *    void
 */
static void build_code_8i40_31bits( Word32 codvec[], Float32 dn_sign[], Float32
      cod[], Float32 h[], Float32 y[], Word32 sign_indx[], Word32 pos_indx[] )
{
   Float64 s;
   Float32 *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;
   Word32 sign[8];
   Word32 i, j, k, track, sign_index, pos_index;


   memset( cod, 0, L_CODE <<2 );

   for ( i = 0; i < NB_TRACK_MR102; i++ ) {
      pos_indx[i] = -1;
      sign_indx[i] = -1;
   }

   for ( k = 0; k < 8; k++ ) {
      /* read pulse position */
      i = codvec[k];

      /* read sign */
      j = ( Word32 )dn_sign[i];

      /* index = pos/4 */
      pos_index = i >> 2;

      /* track = pos%4 */
      track = i & 3;

      if ( j > 0 ) {
         cod[i] = cod[i] + 0.99987792968750F;
         sign[k] = 1;

         /* bit=0 -> positive pulse */
         sign_index = 0;
      }
      else {
         cod[i] = cod[i] - 0.99987792968750F;
         sign[k] = -1;

         /* bit=1 => negative pulse */
         sign_index = 1;
      }

      /* first set first NB_TRACK pulses */
      if ( pos_indx[track] < 0 ) {
         pos_indx[track] = pos_index;
         sign_indx[track] = sign_index;
      }

      /* 2nd row of pulses , test if positions needs to be switched */
      else {
         if ( ( ( sign_index ^ sign_indx[track] ) & 1 ) == 0 ) {
            /* sign of 1st pulse == sign of 2nd pulse */
            if ( pos_indx[track] <= pos_index ) {
               /* no swap */
               pos_indx[track + NB_TRACK_MR102] = pos_index;
            }
            else {
               /* swap*/
               pos_indx[track + NB_TRACK_MR102] = pos_indx[track];
               pos_indx[track] = pos_index;
               sign_indx[track] = sign_index;
            }
         }
         else {
            /* sign of 1st pulse != sign of 2nd pulse */
            if ( pos_indx[track] <= pos_index ) {   /*swap*/
               pos_indx[track + NB_TRACK_MR102] = pos_indx[track];
               pos_indx[track] = pos_index;
               sign_indx[track] = sign_index;
            }
            else {
               /*no swap */
               pos_indx[track + NB_TRACK_MR102] = pos_index;
            }
         }
      }
   }
   p0 = h - codvec[0];
   p1 = h - codvec[1];
   p2 = h - codvec[2];
   p3 = h - codvec[3];
   p4 = h - codvec[4];
   p5 = h - codvec[5];
   p6 = h - codvec[6];
   p7 = h - codvec[7];

   for ( i = 0; i < L_CODE; i++ ) {
      s = *p0++ * sign[0];
      s += *p1++ * sign[1];
      s += *p2++ * sign[2];
      s += *p3++ * sign[3];
      s += *p4++ * sign[4];
      s += *p5++ * sign[5];
      s += *p6++ * sign[6];
      s += *p7++ * sign[7];
      y[i] = ( Float32 )( s );
   }
}


/*
 * compress10
 *
 *
 * Parameters:
 *    pos_indxA         I: signs of 4 pulses (signs only)
 *    pos_indxB         I: position index of 8 pulses (pos only)
 *    pos_indxC         I: position and sign of 8 pulses (compressed)
 * Function:
 *    Compression of three indeces [0..9] to one 10 bit index
 *    minimizing the phase shift of a bit error.
 *
 * Returns:
 *    indx
 */
static Word16 compress10( Word32 pos_indxA, Word32 pos_indxB, Word32
      pos_indxC )
{
   Word32 indx, ia, ib, ic;


   ia = pos_indxA >> 1;
   ib = ( ( pos_indxB >> 1 ) * 5 );
   ic = ( ( pos_indxC >> 1 ) * 25 );
   indx = ( ia + ( ib + ic ) ) << 3;
   ia = pos_indxA & 1;
   ib = ( pos_indxB & 1 ) << 1;
   ic = ( pos_indxC & 1 ) << 2;
   indx = indx + ( ia + ( ib + ic ) );
   return( Word16 )indx;
}


/*
 * compress_code
 *
 *
 * Parameters:
 *    sign_indx         I: signs of 4 pulses (signs only)
 *    pos_indx          I: position index of 8 pulses (pos only)
 *    indx              O: position and sign of 8 pulses (compressed)
 * Function:
 *    Compression of the linear codewords to 4+three indeces one bit from each
 *    pulse is made robust to errors by minimizing the phase shift of a bit error.
 *       4 signs (one for each track)
 *       i0,i4,i1 => one index (7+3) bits, 3   LSBs more robust
 *       i2,i6,i5 => one index (7+3) bits, 3   LSBs more robust
 *       i3,i7    => one index (5+2) bits, 2-3 LSBs more robust
 *
 * Returns:
 *    void
 */
static void compress_code( Word32 sign_indx[], Word32 pos_indx[], Word16 indx[]
      )
{
   Word32 i, ia, ib, ic;


   for ( i = 0; i < NB_TRACK_MR102; i++ ) {
      indx[i] = ( Word16 )sign_indx[i];
   }

   /*
    * First index
    * indx[NB_TRACK] = (ia/2+(ib/2)*5 +(ic/2)*25)*8 + ia%2 + (ib%2)*2 + (ic%2)*4;
    */
   indx[NB_TRACK_MR102] = compress10( pos_indx[0], pos_indx[4], pos_indx[1] );

   /*
    * Second index
    * indx[NB_TRACK+1] = (ia/2+(ib/2)*5 +(ic/2)*25)*8 + ia%2 + (ib%2)*2 + (ic%2)*4;
    */
   indx[NB_TRACK_MR102 + 1] = compress10( pos_indx[2], pos_indx[6], pos_indx[5]
         );

   /*
    * Third index
    * if ((ib/2)%2 == 1)
    * indx[NB_TRACK+2] = ((((4-ia/2) + (ib/2)*5)*32+12)/25)*4 + ia%2 + (ib%2)*2;
    * else
    * indx[NB_TRACK+2] = ((((ia/2) +   (ib/2)*5)*32+12)/25)*4 + ia%2 + (ib%2)*2;
    */
   ib = ( pos_indx[7] >> 1 ) & 1;

   if ( ib == 1 )
      ia = 4 - ( pos_indx[3] >> 1 );
   else
      ia = pos_indx[3] >> 1;
   ib = ( ( pos_indx[7] >> 1 ) * 5 );
   ib = ( ( ia + ib ) << 5 ) + 12;
   ic = ( ( ib * 1311 ) >> 15 ) << 2;
   ia = pos_indx[3] & 1;
   ib = ( pos_indx[7] & 1 ) << 1;
   indx[NB_TRACK_MR102 + 2] = ( Word16 )( ia + ( ib + ic ) );
}


/*
 * code_8i40_31bits
 *
 *
 * Parameters:
 *    x                 I: target vector
 *    cn                I: residual after long term prediction
 *    h                 I: impulse response of weighted synthesis filter
 *    T0                I: Pitch lag
 *    pitch_sharp       I: Last quantized pitch gain
 *    code              O: algebraic (fixed) codebook excitation
 *    y                 O: filtered fixed codebook excitation
 *    anap              O: 7 Word16, index of 8 pulses (signs+positions)
 *
 * Function:
 *    Searches a 31 bit algebraic codebook containing 8 pulses
 *    in a frame of 40 samples.
 *
 *    The code contains 8 nonzero pulses: i0...i7.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    The 40 positions in a subframe are divided into 4 tracks of
 *    interleaved positions. Each track contains two pulses.
 *    The pulses can have the following possible positions:
 *
 *       i0, i4 :  0, 4, 8,  12, 16, 20, 24, 28, 32, 36
 *       i1, i5 :  1, 5, 9,  13, 17, 21, 25, 29, 33, 37
 *       i2, i6 :  2, 6, 10, 14, 18, 22, 26, 30, 34, 38
 *       i3, i7 :  3, 7, 11, 15, 19, 23, 27, 31, 35, 39
 *
 *    Each pair of pulses require 1 bit for their signs. The positions
 *    are encoded together 3,3 and 2 resulting in
 *    (7+3) + (7+3) + (5+2) bits for their
 *    positions. This results in a 31 (4 sign and 27 pos) bit codebook.
 *    The function determines the optimal pulse signs and positions, builds
 *    the codevector, and computes the filtered codevector.
 *
 * Returns:
 *    void
 */
static void code_8i40_31bits( Float32 x[], Float32 cn[], Float32 h[],
                             Word32 T0, Float32 pitch_sharp, Float32 code[],
                             Float32 y[], Word16 anap[] )
{
   Float32 rr[L_CODE][L_CODE];
   Float32 dn[L_CODE], sign[L_CODE];
   Word32 ipos[8], pos_max[NB_TRACK_MR102], codvec[8], linear_signs[
      NB_TRACK_MR102], linear_codewords[8];
   Word32 i;

   if ( pitch_sharp > 1.0F )
      pitch_sharp = 1.0F;

   /* include pitch contribution into impulse resp. */
   if ( pitch_sharp != 0 ) {
      for ( i = T0; i < L_SUBFR; i++ ) {
         h[i] += h[i - T0] * pitch_sharp;
      }
   }

   cor_h_x( h, x, dn );
   set_sign12k2( dn, cn, sign, pos_max, NB_TRACK_MR102, ipos, STEP_MR102 );
   cor_h( h, sign, rr );
   search_8i40( dn, rr, ipos, pos_max, codvec );
   build_code_8i40_31bits( codvec, sign, code, h, y, linear_signs,
      linear_codewords );
   compress_code( linear_signs, linear_codewords, anap );

   /* Add the pitch contribution to code[]. */
   if ( pitch_sharp != 0 ) {
      for ( i = T0; i < L_SUBFR; i++ ) {
         code[i] += code[i - T0] * pitch_sharp;
      }
   }
   return;
}


/*
 * search_10i40
 *
 *
 * Parameters:
 *    dn                I: correlation between target and h[]
 *    rr                I: matrix of autocorrelation
 *    ipos              I: starting position for each pulse
 *    pos_max           I: maximum of correlation position
 *    codvec            O: algebraic codebook vector
 *
 * Function:
 *    Search the best codevector; determine positions of the 10
 *    pulses in the 40-sample frame.
 *
 *    First, for each of the five tracks the pulse positions with maximum
 *    absolute values of b(n) are searched. From these the global maximum value
 *    for all the pulse positions is selected. The first pulse i0 is always set
 *    into the position corresponding to the global maximum value.
 *    Next, four iterations are carried out. During each iteration the position
 *    of pulse i1 is set to the local maximum of one track.
 *    The rest of the pulses are searched in pairs by sequentially searching
 *    each of the pulse pairs {i2,i3}, {i4,i5}, {i6,i7} and {i8,i9}
 *    in nested loops. Every pulse has 8 possible positions, i.e., there are
 *    four 8x8-loops, resulting in 256 different combinations of
 *    pulse positions for each iteration. In each iteration all the 9 pulse
 *    starting positions are cyclically shifted, so that the pulse pairs are
 *    changed and the pulse i1 is placed in a local maximum of
 *    a different track. The rest of the pulses are searched also for
 *    the other positions in the tracks. At least one pulse is located in
 *    a position corresponding to the global maximum and one pulse is
 *    located in a position corresponding to one of the 4 local maxima.
 *
 * Returns:
 *    void
 */
static void search_10i40( Float32 dn[], Float32 rr[][L_CODE], Word32 ipos[],
      Word32 pos_max[], Word32 codvec[] )
{
   Float32 rrv[L_CODE];
   Float32 psk, ps, ps0, ps1, ps2, sq, sq2, alpk, alp, alp0, alp1, alp2;
   Float32 *p_r, *p_r0, *p_r1, *p_r2, *p_r3, *p_r4, *p_r5, *p_r6, *p_r7, *p_r8,
         *p_r9, *p_r10;
   Float32 *p_rrv, *p_rrv0, *p_dn, *p_dn0, *p_dn1, *p_dn_max;
   Word32 i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, j, k, ia, ib, i, pos;

   p_dn_max = &dn[39];

   /* fix i0 on maximum of correlation position */
   i0 = pos_max[ipos[0]];
   ia = ib = 0;
   ps = 0;

   /* i1 loop */
   psk = -1;
   alpk = 1;

   for ( i = 0; i < 10; i++ ) {
      codvec[i] = i;
   }
   p_r = &rr[i0][i0];

   for ( i = 1; i < 5; i++ ) {
      i1 = pos_max[ipos[1]];
      i2 = ipos[2];
      i3 = ipos[3];
      i4 = ipos[4];
      i5 = ipos[5];
      i6 = ipos[6];
      i7 = ipos[7];
      i8 = ipos[8];
      i9 = ipos[9];
      ps0 = dn[i0] + dn[i1];
      alp0 = *p_r + rr[i1][i1] + 2.0F * rr[i0][i1];

      /* i2 and i3 loop	*/
      p_rrv = &rrv[i3];
      p_r0 = &rr[i0][i3];
      p_r1 = &rr[i1][i3];
      p_r3 = &rr[i3][i3];
      *p_rrv = *p_r3 + 2.0F * ( *p_r0 + *p_r1 );
      *( p_rrv + 5 ) = *( p_r3 + 205 ) + 2.0F * ( *( p_r0 + 5 ) + *( p_r1 + 5 )
            );
      *( p_rrv + 10 ) = *( p_r3 + 410 ) + 2.0F * ( *( p_r0 + 10 ) + *( p_r1 + 10
            ) );
      *( p_rrv + 15 ) = *( p_r3 + 615 ) + 2.0F * ( *( p_r0 + 15 ) + *( p_r1 + 15
            ) );
      *( p_rrv + 20 ) = *( p_r3 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) );
      *( p_rrv + 25 ) = *( p_r3 + 1025 ) + 2.0F * ( *( p_r0 + 25 ) + *( p_r1 +
            25 ) );
      *( p_rrv + 30 ) = *( p_r3 + 1230 ) + 2.0F * ( *( p_r0 + 30 ) + *( p_r1 +
            30 ) );
      *( p_rrv + 35 ) = *( p_r3 + 1435 ) + 2.0F * ( *( p_r0 + 35 ) + *( p_r1 +
            35 ) );
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i2;
      ib = i3;
      p_rrv = rrv + i3;
      p_r0 = &rr[i0][i2];
      p_r1 = &rr[i1][i2];
      p_r2 = &rr[i2][i2];
      p_r3 = &rr[i2][i3];
      p_dn0 = dn + i2;
      p_dn1 = dn + i3;
      p_rrv0 = rrv + i3;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r2 + 2.0F * ( *p_r0 + *p_r1 );
         p_rrv = p_rrv0;
         p_dn = p_dn1;
         p_r4 = p_r3;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r4;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = ( Word16 )( p_dn0 - dn );
               ib = ( Word16 )( p_dn - dn );
            }
            p_rrv += 5;
            p_dn += 5;
            p_r4 += 5;
         } while ( p_dn < p_dn_max );
         p_dn0 += 5;
         p_r0 += 5;
         p_r1 += 5;
         p_r2 += 205;
         p_r3 += 200;
      } while ( p_dn0 <= p_dn_max );
      i2 = ia;
      i3 = ib;

      /* i4 and i5 loop	*/
      p_rrv = rrv + i5;
      p_r0 = &rr[i0][i5];
      p_r1 = &rr[i1][i5];
      p_r2 = &rr[i2][i5];
      p_r3 = &rr[i3][i5];
      p_r5 = &rr[i5][i5];
      *p_rrv = *p_r5 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 );
      *( p_rrv + 5 ) = *( p_r5 + 205 ) + 2.0F * ( *( p_r0 + 5 ) + *( p_r1 + 5 )
            + *( p_r2 + 5 ) + *( p_r3 + 5 ) );
      *( p_rrv + 10 ) = *( p_r5 + 410 ) + 2.0F * ( *( p_r0 + 10 ) + *( p_r1 + 10
            ) + *( p_r2 + 10 ) + *( p_r3 + 10 ) );
      *( p_rrv + 15 ) = *( p_r5 + 615 ) + 2.0F * ( *( p_r0 + 15 ) + *( p_r1 + 15
            ) + *( p_r2 + 15 ) + *( p_r3 + 15 ) );
      *( p_rrv + 20 ) = *( p_r5 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) + *( p_r2 + 20 ) + *( p_r3 + 20 ) );
      *( p_rrv + 25 ) = *( p_r5 + 1025 ) + 2.0F * ( *( p_r0 + 25 ) + *( p_r1 +
            25 ) + *( p_r2 + 25 ) + *( p_r3 + 25 ) );
      *( p_rrv + 30 ) = *( p_r5 + 1230 ) + 2.0F * ( *( p_r0 + 30 ) + *( p_r1 +
            30 ) + *( p_r2 + 30 ) + *( p_r3 + 30 ) );
      *( p_rrv + 35 ) = *( p_r5 + 1435 ) + 2.0F * ( *( p_r0 + 35 ) + *( p_r1 +
            35 ) + *( p_r2 + 35 ) + *( p_r3 + 35 ) );

      /* Default value */
      ps0 = ps;
      alp0 = alp;
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i4;
      ib = i5;
      p_dn0 = dn + i4;
      p_dn1 = dn + i5;
      p_r0 = &rr[i0][i4];
      p_r1 = &rr[i1][i4];
      p_r2 = &rr[i2][i4];
      p_r3 = &rr[i3][i4];
      p_r4 = &rr[i4][i4];
      p_r5 = &rr[i4][i5];
      p_rrv0 = rrv + i5;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r4 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 );
         p_dn = p_dn1;
         p_r6 = p_r5;
         p_rrv = p_rrv0;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r6;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = ( Word16 )( p_dn0 - dn );
               ib = ( Word16 )( p_dn - dn );
            }
            p_dn += 5;
            p_rrv += 5;
            p_r6 += 5;
         } while ( p_dn <= p_dn_max );
         p_r0 += 5;
         p_r1 += 5;
         p_r2 += 5;
         p_r3 += 5;
         p_r4 += 205;
         p_r5 += 200;
         p_dn0 += 5;
      } while ( p_dn0 < p_dn_max );
      i4 = ia;
      i5 = ib;

      /* i6 and i7 loop	*/
      p_rrv = rrv + i7;
      p_r0 = &rr[i0][i7];
      p_r1 = &rr[i1][i7];
      p_r2 = &rr[i2][i7];
      p_r3 = &rr[i3][i7];
      p_r4 = &rr[i4][i7];
      p_r5 = &rr[i5][i7];
      p_r7 = &rr[i7][i7];
      *p_rrv = *p_r7 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 + *p_r4 + *p_r5 );
      *( p_rrv + 5 ) = *( p_r7 + 205 ) + 2.0F * ( *( p_r0 + 5 ) + *( p_r1 + 5 )
            + *( p_r2 + 5 ) + *( p_r3 + 5 ) + *( p_r4 + 5 ) + *( p_r5 + 5 ) );
      *( p_rrv + 10 ) = *( p_r7 + 410 ) + 2.0F * ( *( p_r0 + 10 ) + *( p_r1 + 10
            ) + *( p_r2 + 10 ) + *( p_r3 + 10 ) + *( p_r4 + 10 ) + *( p_r5 + 10
            ) );
      *( p_rrv + 15 ) = *( p_r7 + 615 ) + 2.0F * ( *( p_r0 + 15 ) + *( p_r1 + 15
            ) + *( p_r2 + 15 ) + *( p_r3 + 15 ) + *( p_r4 + 15 ) + *( p_r5 + 15
            ) );
      *( p_rrv + 20 ) = *( p_r7 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) + *( p_r2 + 20 ) + *( p_r3 + 20 ) + *( p_r4 + 20 ) + *( p_r5 + 20
            ) );
      *( p_rrv + 25 ) = *( p_r7 + 1025 ) + 2.0F * ( *( p_r0 + 25 ) + *( p_r1 +
            25 ) + *( p_r2 + 25 ) + *( p_r3 + 25 ) + *( p_r4 + 25 ) + *( p_r5 +
            25 ) );
      *( p_rrv + 30 ) = *( p_r7 + 1230 ) + 2.0F * ( *( p_r0 + 30 ) + *( p_r1 +
            30 ) + *( p_r2 + 30 ) + *( p_r3 + 30 ) + *( p_r4 + 30 ) + *( p_r5 +
            30 ) );
      *( p_rrv + 35 ) = *( p_r7 + 1435 ) + 2.0F * ( *( p_r0 + 35 ) + *( p_r1 +
            35 ) + *( p_r2 + 35 ) + *( p_r3 + 35 ) + *( p_r4 + 35 ) + *( p_r5 +
            35 ) );

      /* Default value */
      ps0 = ps;
      alp0 = alp;
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i6;
      ib = i7;
      p_dn0 = dn + i6;
      p_dn1 = dn + i7;
      p_r0 = &rr[i0][i6];
      p_r1 = &rr[i1][i6];
      p_r2 = &rr[i2][i6];
      p_r3 = &rr[i3][i6];
      p_r4 = &rr[i4][i6];
      p_r5 = &rr[i5][i6];
      p_r6 = &rr[i6][i6];
      p_r7 = &rr[i6][i7];
      p_rrv0 = rrv + i7;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r6 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 + *p_r4 +
               *p_r5 );
         p_dn = p_dn1;
         p_r8 = p_r7;
         p_rrv = p_rrv0;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r8;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = ( Word16 )( p_dn0 - dn );
               ib = ( Word16 )( p_dn - dn );
            }
            p_dn += 5;
            p_rrv += 5;
            p_r8 += 5;
         } while ( p_dn <= p_dn_max );
         p_r0 += 5;
         p_r1 += 5;
         p_r2 += 5;
         p_r3 += 5;
         p_r4 += 5;
         p_r5 += 5;
         p_r6 += 205;
         p_r7 += 200;
         p_dn0 += 5;
      } while ( p_dn0 < p_dn_max );
      i6 = ia;
      i7 = ib;

      /* i8 and i9 loop	*/
      p_rrv = rrv + i9;
      p_r0 = &rr[i0][i9];
      p_r1 = &rr[i1][i9];
      p_r2 = &rr[i2][i9];
      p_r3 = &rr[i3][i9];
      p_r4 = &rr[i4][i9];
      p_r5 = &rr[i5][i9];
      p_r6 = &rr[i6][i9];
      p_r7 = &rr[i7][i9];
      p_r9 = &rr[i9][i9];
      *p_rrv = *p_r9 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 + *p_r4 + *p_r5 +
            *p_r6 + *p_r7 );
      *( p_rrv + 5 ) = *( p_r9 + 205 ) + 2.0F * ( *( p_r0 + 5 ) + *( p_r1 + 5 )
            + *( p_r2 + 5 ) + *( p_r3 + 5 ) + *( p_r4 + 5 ) + *( p_r5 + 5 ) + *(
            p_r6 + 5 ) + *( p_r7 + 5 ) );
      *( p_rrv + 10 ) = *( p_r9 + 410 ) + 2.0F * ( *( p_r0 + 10 ) + *( p_r1 + 10
            ) + *( p_r2 + 10 ) + *( p_r3 + 10 ) + *( p_r4 + 10 ) + *( p_r5 + 10
            ) + *( p_r6 + 10 ) + *( p_r7 + 10 ) );
      *( p_rrv + 15 ) = *( p_r9 + 615 ) + 2.0F * ( *( p_r0 + 15 ) + *( p_r1 + 15
            ) + *( p_r2 + 15 ) + *( p_r3 + 15 ) + *( p_r4 + 15 ) + *( p_r5 + 15
            ) + *( p_r6 + 15 ) + *( p_r7 + 15 ) );
      *( p_rrv + 20 ) = *( p_r9 + 820 ) + 2.0F * ( *( p_r0 + 20 ) + *( p_r1 + 20
            ) + *( p_r2 + 20 ) + *( p_r3 + 20 ) + *( p_r4 + 20 ) + *( p_r5 + 20
            ) + *( p_r6 + 20 ) + *( p_r7 + 20 ) );
      *( p_rrv + 25 ) = *( p_r9 + 1025 ) + 2.0F * ( *( p_r0 + 25 ) + *( p_r1 +
            25 ) + *( p_r2 + 25 ) + *( p_r3 + 25 ) + *( p_r4 + 25 ) + *( p_r5 +
            25 ) + *( p_r6 + 25 ) + *( p_r7 + 25 ) );
      *( p_rrv + 30 ) = *( p_r9 + 1230 ) + 2.0F * ( *( p_r0 + 30 ) + *( p_r1 +
            30 ) + *( p_r2 + 30 ) + *( p_r3 + 30 ) + *( p_r4 + 30 ) + *( p_r5 +
            30 ) + *( p_r6 + 30 ) + *( p_r7 + 30 ) );
      *( p_rrv + 35 ) = *( p_r9 + 1435 ) + 2.0F * ( *( p_r0 + 35 ) + *( p_r1 +
            35 ) + *( p_r2 + 35 ) + *( p_r3 + 35 ) + *( p_r4 + 35 ) + *( p_r5 +
            35 ) + *( p_r6 + 35 ) + *( p_r7 + 35 ) );

      /* Default value */
      ps0 = ps;
      alp0 = alp;
      sq = -1;
      alp = 1;
      ps = 0;
      ia = i8;
      ib = i9;
      p_dn0 = dn + i8;
      p_dn1 = dn + i9;
      p_r0 = &rr[i0][i8];
      p_r1 = &rr[i1][i8];
      p_r2 = &rr[i2][i8];
      p_r3 = &rr[i3][i8];
      p_r4 = &rr[i4][i8];
      p_r5 = &rr[i5][i8];
      p_r6 = &rr[i6][i8];
      p_r7 = &rr[i7][i8];
      p_r8 = &rr[i8][i8];
      p_r9 = &rr[i8][i9];
      p_rrv0 = rrv + i9;

      do {
         ps1 = ps0 + *p_dn0;
         alp1 = alp0 + *p_r8 + 2.0F * ( *p_r0 + *p_r1 + *p_r2 + *p_r3 + *p_r4 +
               *p_r5 + *p_r6 + *p_r7 );
         p_dn = p_dn1;
         p_r10 = p_r9;
         p_rrv = p_rrv0;

         do {
            ps2 = ps1 + *p_dn;
            sq2 = ps2 * ps2;
            alp2 = alp1 + *p_rrv + 2.0F * *p_r10;

            if ( ( alp * sq2 ) > ( sq * alp2 ) ) {
               sq = sq2;
               ps = ps2;
               alp = alp2;
               ia = ( Word16 )( p_dn0 - dn );
               ib = ( Word16 )( p_dn - dn );
            }
            p_dn += 5;
            p_rrv += 5;
            p_r10 += 5;
         } while ( p_dn <= p_dn_max );
         p_r0 += 5;
         p_r1 += 5;
         p_r2 += 5;
         p_r3 += 5;
         p_r4 += 5;
         p_r5 += 5;
         p_r6 += 5;
         p_r7 += 5;
         p_r8 += 205;
         p_r9 += 200;
         p_dn0 += 5;
      } while ( p_dn0 < p_dn_max );

      /*
       * test and memorise if this combination is better than the last one.
       */
      if ( ( alpk * sq ) > ( psk * alp ) ) {
         psk = sq;
         alpk = alp;
         codvec[0] = ( Word16 )i0;
         codvec[1] = ( Word16 )i1;
         codvec[2] = ( Word16 )i2;
         codvec[3] = ( Word16 )i3;
         codvec[4] = ( Word16 )i4;
         codvec[5] = ( Word16 )i5;
         codvec[6] = ( Word16 )i6;
         codvec[7] = ( Word16 )i7;
         codvec[8] = ( Word16 )ia;
         codvec[9] = ( Word16 )ib;
      }

      /*
       * Cyclic permutation of i1,i2,i3,i4,i5,i6,i7,(i8 and i9).
       */
      pos = ipos[1];

      for ( j = 1, k = 2; k < 10; j++, k++ ) {
         ipos[j] = ipos[k];
      }
      ipos[9] = pos;
   }   /* end 1..nb_tracks  loop*/
}


/*
 * build_code_10i40_35bits
 *
 *
 * Parameters:
 *    codvec            I: position of pulses
 *    dn_sign           I: sign of pulses
 *    cod               O: algebraic codebook vector
 *    h                 I: impulse response of weighted synthesis filter
 *    y                 O: filtered innovative code
 *    indx              O: index of 10 pulses (sign+position)
 *
 * Function:
 *    Builds the codeword, the filtered codeword and index of the
 *    codevector, based on the signs and positions of 10 pulses.
 *
 * Returns:
 *    void
 */
static void build_code_10i40_35bits( Word32 codvec[], Float32 dn_sign[], Float32
      cod[], Float32 h[], Float32 y[], Word16 indx[] )
{
   Word32 i, j, k, track, index, sign[10];
   Float32 *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9;
   Float64 s;


   memset( cod, 0, 160 );
   memset( y, 0, 160 );

   for ( i = 0; i < NB_TRACK; i++ ) {
      indx[i] = -1;
   }

   for ( k = 0; k < 10; k++ ) {
      /* read pulse position */
      i = codvec[k];

      /* read sign */
      j = ( Word16 )dn_sign[i];

      /* index = pos/5 */
      index = ( Word16 )( i / 5 );

      /* track = pos%5 */
      track = ( Word16 )( i % 5 );

      if ( j > 0 ) {
         cod[i] = cod[i] + 1;
         sign[k] = 1;
      }
      else {
         cod[i] = cod[i] - 1;
         sign[k] = -1;
         index = index + 8;
      }

      if ( indx[track] < 0 ) {
         indx[track] = ( Word16 )index;
      }
      else {
         if ( ( ( index ^ indx[track] ) & 8 ) == 0 ) {
            /* sign of 1st pulse == sign of 2nd pulse */
            if ( indx[track] <= index ) {
               indx[track + 5] = ( Word16 )index;
            }
            else {
               indx[track + 5] = ( Word16 )indx[track];
               indx[track] = ( Word16 )index;
            }
         }
         else {
            /* sign of 1st pulse != sign of 2nd pulse */
            if ( ( indx[track] & 7 ) <= ( index & 7 ) ) {
               indx[track + 5] = ( Word16 )indx[track];
               indx[track] = ( Word16 )index;
            }
            else {
               indx[track + 5] = ( Word16 )index;
            }
         }
      }
   }
   p0 = h - codvec[0];
   p1 = h - codvec[1];
   p2 = h - codvec[2];
   p3 = h - codvec[3];
   p4 = h - codvec[4];
   p5 = h - codvec[5];
   p6 = h - codvec[6];
   p7 = h - codvec[7];
   p8 = h - codvec[8];
   p9 = h - codvec[9];

   for ( i = 0; i < L_CODE; i++ ) {
      s = *p0++ * sign[0];
      s += *p1++ * sign[1];
      s += *p2++ * sign[2];
      s += *p3++ * sign[3];
      s += *p4++ * sign[4];
      s += *p5++ * sign[5];
      s += *p6++ * sign[6];
      s += *p7++ * sign[7];
      s += *p8++ * sign[8];
      s += *p9++ * sign[9];
      y[i] = ( Float32 )( s );
   }
}


/*
 * q_p
 *
 *
 * Parameters:
 *    ind               B: Pulse position
 *    n                 I: Pulse number
 *
 * Function:
 *    Gray coding
 *
 * Returns:
 *    void
 */
static void q_p( Word16 *ind, Word32 n )
{
   Word16 tmp;


   tmp = *ind;

   if ( n < 5 ) {
      *ind = ( Word16 )( ( tmp & 0x8 ) | gray[tmp & 0x7] );
   }
   else {
      *ind = gray[tmp & 0x7];
   }
}


/*
 * code_10i40_35bits
 *
 *
 * Parameters:
 *    x                 I: target vector
 *    cn                I: residual after long term prediction
 *    h                 I: impulse response of weighted synthesis filter
 *    gain_pit          I: quantified adaptive codebook gain
 *    code              O: algebraic (fixed) codebook excitation
 *    y                 O: filtered fixed codebook excitation
 *    anap              O: 7 Word16, index of 8 pulses (signs+positions)
 *
 * Function:
 *    Searches a 35 bit algebraic codebook containing 10 pulses
 *    in a frame of 40 samples.
 *
 *    The code contains 10 nonzero pulses: i0...i9.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    The 40 positions in a subframe are divided into 5 tracks of
 *    interleaved positions. Each track contains two pulses.
 *    The pulses can have the following possible positions:
 *       Track    Pulse       Positions
 *          1     i0, i5      0, 5, 10, 15, 20, 25, 30, 35.
 *          2     i1, i6      1, 6, 11, 16, 21, 26, 31, 36.
 *          3     i2, i       2, 7, 12, 17, 22, 27, 32, 37.
 *          4     i3, i8      3, 8, 13, 18, 23, 28, 33, 38.
 *          5     i4, i9      4, 9, 14, 19, 24, 29, 34, 39.
 *
 *    Each pair of pulses require 1 bit for their signs and 6 bits for their
 *    positions (3 bits + 3 bits). This results in a 35 bit codebook.
 *    The function determines the optimal pulse signs and positions, builds
 *    the codevector, and computes the filtered codevector.
 *
 *    The algebraic codebook is searched by minimizing the mean square error
 *    between the weighted input speech and the weighted synthesized speech.
 *    The target signal used in the closed-loop pitch search is updated by
 *    subtracting the adaptive codebook contribution. That is:
 *
 *    x2(n) = x(n) - Gp' * y(n), n = 0, ..., 39
 *
 *    where y(n) = v(n) * h(n) is the filtered adaptive codebook vector
 *    and Gp' is the quantified adaptive codebook gain. This is done
 *    already in function cl_ltp.
 *
 *    If c(k) is the algebraic codevector at index k, then
 *    the algebraic codebook is searched by maximizing the term:
 *
 *    A(k) = (C(k) * C(k)) / Ed(k) =
 *
 *                     39
 *                   [SUM sb(i)*d(i)]^2
 *                    i=0
 *                  ---------------------
 *                 transpose(sb) * PI * sb
 *
 *    where d(n) is the correlation between the target signal x2(n)
 *    and the impulse response h(n), H is a the lower triangular Toepliz
 *    convolution matrix with diagonal h(0) and lower diagonals
 *    h(1), ..., h(39), and PI = H_transpose * H is the matrix of
 *    correlations of h(n).
 *
 *    The pulse amplitudes are preset by the mere quantization of an signal
 *    b(n) used for presetting the amplitudes.
 *
 *                           39
 *    b(n) = res(n) / SQRT[ SUM[ res(i) * res(i) ] ]
 *                          i=0
 *                          39
 *          + d(n) / SQRT[ SUM [ d(i) * d(i) ] ], n = 0, ..., 39,
 *                         i=0
 *
 *    where res(n) is normalized long-term prediction residual and
 *    d(n) is normalized vector.
 *
 *            39
 *    d(n) = SUM[ x2(i) * h(i-n) ], n = 0, ..., 39,
 *           i=n
 *
 *    This is simply done by setting the amplitude of a pulse at
 *    a certain position equal to the sign of b(n) at that position.
 *    The simplification proceeds as follows (prior to the codebook search).
 *    First, the sign signal sb(n) = SIGN[ b(n) ] and
 *    the signal d'(n) = sb(n) * d(n) are computed.
 *
 * Returns:
 *    void
 */
static void code_10i40_35bits( Float32 x[], Float32 cn[], Float32 h[],
    Word32 T0, Float32 gain_pit, Float32 code[],
    Float32 y[], Word16 anap[] )
 {
    Float32 rr[L_CODE][L_CODE];
    Float32 dn[L_CODE], sign[L_CODE];
    Word32 ipos[10], pos_max[NB_TRACK], codvec[10];
    Word32 i;

    /* include pitch contribution into impulse resp. */
    if ( gain_pit > 1.0F )
       gain_pit = 1.0F;

    if ( gain_pit != 0 ) {
       for ( i = T0; i < L_SUBFR; i++ ) {
          h[i] += h[i - T0] * gain_pit;
       }
    }
    /*
    *            39
    *    d(n) = SUM[ x2(i) * h(i-n) ], n = 0, ..., 39
    *           i=n
    */
    cor_h_x( h, x, dn );

    /* sb(n) and d'(n) */
    set_sign12k2( dn, cn, sign, pos_max, NB_TRACK, ipos, STEP );

    /* Matrix of correlations */
    cor_h( h, sign, rr );
    search_10i40( dn, rr, ipos, pos_max, codvec );
    build_code_10i40_35bits( codvec, sign, code, h, y, anap );

    for ( i = 0; i < 10; i++ ) {
       q_p( &anap[i], i );
    }

    /*  Add the pitch contribution to code[]. */
    if ( gain_pit != 0 ) {
       for ( i = T0; i < L_SUBFR; i++ ) {
          code[i] += code[i - T0] * gain_pit;
       }
    }
    return;
 }


/*
 * cbsearch
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    subnr             I: Subframe
 *    x                 I: Target vector
 *    h                 B: Impulse response of weighted synthesis filter
 *    T0                I: Pitch lag
 *    pitch_sharp       I: Last quantized pitch gain
 *    gain_pit          I: Algebraic codebook gain
 *    code              O: Innovative codebook
 *    y                 O: Filtered fixed codebook excitation
 *    res2              I: residual after long term prediction
 *    anap              O: Signs and positions of the pulses
 *
 * Function:
 *    Innovative codebook search (find index and gain)
 *
 * Returns:
 *    void
 */
static void cbsearch( enum Mode mode, Word16 subnr, Float32 x[],
                     Float32 h[], Word32 T0, Float32 pitch_sharp,
                     Float32 gain_pit, Float32 code[], Float32 y[],
                     Float32 *res2, Word16 **anap )
{
   switch (mode){
   case MR475:
   case MR515:
      code_2i40_9bits( subnr, x, h, T0, pitch_sharp, code, y, *anap );
      ( *anap ) += 2;
      break;
   case MR59:
      code_2i40_11bits( x, h, T0, pitch_sharp, code, y, *anap );
      ( *anap ) += 2;
      break;
   case MR67:
      code_3i40_14bits( x, h, T0, pitch_sharp, code, y, *anap );
      ( *anap ) += 2;
      break;
   case MR74:
   case MR795:
      code_4i40_17bits( x, h, T0, pitch_sharp, code, y, *anap );
      ( *anap ) += 2;
      break;
   case MR102:
      code_8i40_31bits( x, res2, h, T0, pitch_sharp, code, y, *anap );
      *anap += 7;
      break;
   default:
      code_10i40_35bits( x, res2, h, T0, gain_pit, code, y, *anap );
      *anap += 10;
   }
}

/*
 * Log2_norm
 *
 *
 * Parameters:
 *    x                 I: input value
 *    exp               I: exponent
 *    exponent          O: Integer part of Log2. (range: 0<=val<=30)
 *    fraction          O: Fractional part of Log2. (range: 0<=val<1)
 *
 * Function:
 *    Computes log2
 *
 *    Computes log2(L_x, exp),  where   L_x is positive and
 *    normalized, and exp is the normalisation exponent
 *    If L_x is negative or zero, the result is 0.
 *
 *    The function Log2(L_x) is approximated by a table and linear
 *    interpolation. The following steps are used to compute Log2(L_x)
 *
 *    exponent = 30-normExponent
 *    i = bit25-b31 of L_x;  32<=i<=63  (because of normalization).
 *    a = bit10-b24
 *    i -=32
 *    fraction = table[i]<<16 - (table[i] - table[i+1]) * a * 2
 *
 * Returns:
 *    void
 */
static void Log2_norm( Word32 x, Word32 exp, Word32 *exponent, Word32 *
      fraction )
{
   Word32 y, i, a;

   if ( x <= 0 ) {
      *exponent = 0;
      *fraction = 0;
      return;
   }

   /* Extract b25-b31 */
   i = x >> 25;
   i = i - 32;

   /* Extract b10-b24 of fraction */
   a = x >> 9;
   a = a & 0xFFFE;   /* 2a */

   /* fraction */
   y = ( log2_table[i] << 16 ) - a * ( log2_table[i] - log2_table[i + 1] );
   *fraction = y >> 16;
   *exponent = 30 - exp;
   return;
}

/*
 * Log2
 *
 *
 * Parameters:
 *    x                 I: input value
 *    exponent          O: Integer part of Log2. (range: 0<=val<=30)
 *    fraction          O: Fractional part of Log2. (range: 0<=val<1)
 *
 * Function:
 *    Computes log2(L_x)
 *    If x is negative or zero, the result is 0.
 *
 * Returns:
 *    void
 */
static void Log2( Word32 x, Word32 *exponent, Word32 *fraction )
{
   int exp;


   frexp( ( Float64 )x, &exp );
   exp = 31 - exp;
   Log2_norm( x <<exp, exp, exponent, fraction );
}

/*
 * Pow2
 *
 *
 * Parameters:
 *    exponent          I: Integer part. (range: 0<=val<=30)
 *    fraction          O: Fractional part. (range: 0.0<=val<1.0)
 *
 * Function:
 *    pow(2.0, exponent.fraction)
 *
 *    The function Pow2(L_x) is approximated by a table and linear interpolation.
 *
 *    i = bit10-b15 of fraction, 0 <= i <= 31
 *    a = biT0-b9   of fraction
 *    x = table[i]<<16 - (table[i] - table[i+1]) * a * 2
 *    x = L_x >> (30-exponent) (with rounding)
 *
 * Returns:
 *    result (range: 0<=val<=0x7fffffff)
 */
static Word32 Pow2( Word32 exponent, Word32 fraction )
{
   Word32 i, a, tmp, x, exp;

   /* Extract b10-b16 of fraction */
   i = fraction >> 10;

   /* Extract b0-b9 of fraction */
   a = ( fraction << 5 ) & 0x7fff;

   /* table[i] << 16 */
   x = pow2_table[i] << 16;

   /* table[i] - table[i+1] */
   tmp = pow2_table[i] - pow2_table[i + 1];

   /* L_x -= tmp*a*2 */
   x -= ( tmp * a ) << 1;

   if ( exponent >= -1 ) {
      exp = ( 30 - exponent );

      /* Rounding */
      if ( ( x & ( ( Word32 )1 << ( exp - 1 ) ) ) != 0 ) {
         x = ( x >> exp ) + 1;
      }
      else
         x = x >> exp;
   }
   else
      x = 0;
   return( x );
}

/*
 * gc_pred
 *
 *
 * Parameters:
 *    past_qua_en       I: MA predictor
 *    mode              I: AMR mode
 *    code              I: innovative codebook vector
 *    gcode0            O: predicted gain factor
 *    en                I: innovation energy (only calculated for MR795)
 *
 * Function:
 *    MA prediction of the innovation energy
 *
 *    Mean removed innovation energy (dB) in subframe n
 *                          N-1
 *    E(n) = 10*log(gc*gc * SUM[(code(i) * code(i)]/N) - E_mean
 *                          i=0
 *    N=40
 *
 *    Mean innovation energy (dB)
 *                   N-1
 *    Ei(n) = 10*log(SUM[(code(i) * code(i)]/N)
 *                   i=0
 *
 *    Predicted energy
 *             4
 *    Ep(n) = SUM[b(i) * R(n-i)]
 *            i=1
 *    b = [0.68 0.58 0.34 0.19]
 *    R(k) is quantified prediction error at subframe k
 *
 *    E_Mean = 36 dB (MR122)
 *
 *    Predicted gain gc is found by
 *
 *    gc = POW[10, 0.05 * (Ep(n) + E_mean - Ei)]
 *
 * Returns:
 *    void
 */
static void gc_pred( Word32 *past_qua_en, enum Mode mode, Float32 *code,
      Word32 *gcode0_exp, Word32 *gcode0_fra, Float32 *en )
{
   Float64 ener_code;
   Word32 exp, frac, ener, ener_tmp, tmp;
   int exp_code;


   /* energy of code */
   ener_code = Dotproduct40( code, code );

   if ( mode == MR122 ) {

      ener = (Word32)(ener_code * 33554432);
      /* ener_code = ener_code / lcode; lcode = 40; 1/40 = 26214 Q20       */
      ener = ( ( ener + 0x00008000L ) >> 16 ) * 52428;

      Log2( ener, &exp, &frac );
      ener = ( ( exp - 30 ) << 16 ) + ( frac << 1 );

      ener_tmp = 44 * qua_gain_code_MR122[past_qua_en[0]];
      ener_tmp += 37 * qua_gain_code_MR122[past_qua_en[1]];
      ener_tmp += 22 * qua_gain_code_MR122[past_qua_en[2]];
      ener_tmp += 12 * qua_gain_code_MR122[past_qua_en[3]];

      ener_tmp = ener_tmp << 1;
      ener_tmp += 783741L;

      /*
       * predicted codebook gain
       * gc0 = Pow10( (ener*constant - ener_code*constant) / 20 )
       *     = Pow2(ener-ener_code)
       *     = Pow2(int(d)+frac(d))
       */
      ener = ( ener_tmp - ener ) >> 1;   /* Q16 */
      *gcode0_exp = ener >> 16;
      *gcode0_fra = ( ener >> 1 ) - ( *gcode0_exp << 15 );
   }
   else {
      ener = (Word32)(ener_code * 134217728);
      if (ener < 0)
         ener = 0x7fffffff;

      frexp( ( Float64 )ener, &exp_code );
      exp_code = 31 - exp_code;
      ener <<= exp_code;

      Log2_norm( ener, exp_code, &exp, &frac );

      tmp = ( exp * ( -49320 ) ) + ( ( ( frac * ( -24660 ) ) >> 15 ) << 1 );

      if ( mode == MR102 ) {
         /* mean = 33 dB */
         tmp += 2134784;   /* Q14 */
      }
      else if ( mode == MR795 ) {
         /* mean = 36 dB */
         tmp += 2183936;   /* Q14 */

         *en = (Float32)ener_code;
      }
      else if ( mode == MR74 ) {
         /* mean = 30 dB */
         tmp += 2085632;   /* Q14 */
      }
      else if ( mode == MR67 ) {
         /* mean = 28.75 dB */
         tmp += 2065152;   /* Q14 */
      }
      else /* MR59, MR515, MR475 */ {
         /* mean = 33 dB */
         tmp += 2134784;   /* Q14 */
      }

      tmp = tmp << 9;

      tmp += 5571 * qua_gain_code[past_qua_en[0]];
      tmp += 4751 * qua_gain_code[past_qua_en[1]];
      tmp += 2785 * qua_gain_code[past_qua_en[2]];
      tmp += 1556 * qua_gain_code[past_qua_en[3]];

      tmp = tmp >> 15;   /* Q8  */

      /*
       * gcode0 = pow(10.0, gcode0/20)
       *        = pow(2, 3.3219*gcode0/20)
       *        = pow(2, 0.166*gcode0)
       */
      /* 5439 Q15 = 0.165985                                        */
      /* (correct: 1/(20*log10(2)) 0.166096 = 5443 Q15)             */
      /* For IS641 bitexactness */
      if ( mode == MR74 ) {
         /* Q8 * Q15 -> Q24 */
         tmp = tmp * 10878;
      }
      else {
         /* Q8 * Q15 -> Q24 */
         tmp = tmp * 10886;
      }
      tmp = tmp >> 9;   /* -> Q15 */

      *gcode0_exp = tmp >> 15;
      *gcode0_fra = tmp - ( *gcode0_exp * 32768 );
   }
}

/*
 * calc_filt_energies
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    xn                I: LTP target vector
 *    xn2               I: CB target vector
 *    y1                I: Adaptive codebook
 *    y2                I: Filtered innovative vector
 *    gCoeff            I: Correlations <xn y1> <y1 y1>
 *    coeff             O: energy coefficients
 *    cod_gain          O: optimum codebook gain
 *
 * Function:
 *    Calculation of several energy coefficients for filtered excitation signals
 *
 *    Compute coefficients need for the quantization and the optimum
 *    codebook gain gcu (for MR475 only).
 *
 *       coeff[0] =    y1 y1
 *       coeff[1] = -2 xn y1
 *       coeff[2] =    y2 y2
 *       coeff[3] = -2 xn y2
 *       coeff[4] =  2 y1 y2
 *
 *
 *       gcu = <xn2, y2> / <y2, y2> (0 if <xn2, y2> <= 0)
 *
 *    Product <y1 y1> and <xn y1> have been computed in G_pitch() and
 *    are in vector gCoeff[].
 *
 * Returns:
 *    void
 */
static void calc_filt_energies( enum Mode mode, Float32 xn[], Float32 xn2[],
      Float32 y1[], Float32 y2[], Float32 gCoeff[], Float32 coeff[], Float32 *
      cod_gain )
{
   Float32 sum, ener_init = 0.01F;


   if ( ( mode == MR795 ) || ( mode == MR475 ) )
      ener_init = 0;
   coeff[0] = gCoeff[0];
   coeff[1] = -2.0F * gCoeff[1];

   /* Compute scalar product <y2[],y2[]> */
   sum = (Float32)Dotproduct40( y2, y2 );
   sum += ener_init;
   coeff[2] = sum;

   /* Compute scalar product -2*<xn[],y2[]> */
   sum = (Float32)Dotproduct40( xn, y2 );
   sum += ener_init;
   coeff[3] = -2.0F * sum;

   /* Compute scalar product 2*<y1[],y2[]> */
   sum = (Float32)Dotproduct40( y1, y2 );
   sum += ener_init;
   coeff[4] = 2.0F * sum;

   if ( ( mode == MR475 ) || ( mode == MR795 ) ) {
      /* Compute scalar product <xn2[],y2[]> */
      sum = (Float32)Dotproduct40( xn2, y2 );

      if ( sum <= 0 ) {
         *cod_gain = 0;
      }
      else {
      /*
       * gcu = <xn2, y2> / <y2, y2>
       */
         *cod_gain = sum / coeff[2];
      }
   }
}


/*
 * MR475_update_unq_pred
 *
 *
 * Parameters:
 *    past_qua_en       I: MA predictor memory, 20*log10(qua_err)
 *    gcode0            I: predicted CB gain
 *    cod_gain          I: optimum codebook gain
 *
 * Function:
 *    Use optimum codebook gain and update "unquantized"
 *    gain predictor with the (bounded) prediction error
 *
 *    Prediction error is given by:
 *
 *       R(n) = E(n) - E_pred(n) = 20 * log(cf),
 *
 *    where correction factor cf between the gain Gc and
 *    the estimated pne Gc' is given by:
 *
 *       cf = Gc/Gc'.
 *
 * Returns:
 *    void
 */
static void MR475_update_unq_pred( Word32 *past_qua_en, Float32 gcode0, Float32
      cod_gain )
{
   Float32 qua_ener, pred_err_fact;
   Word32 i, index, energy, max, s;


   if ( cod_gain <= 0 ) {
      /*MIN_QUA_ENER*/
      qua_ener = -32.0F;
   }
   else {
      if (gcode0 != 0) {
         pred_err_fact = cod_gain / gcode0;
      }
      else {
         pred_err_fact = 10.0F;
      }

      if ( pred_err_fact < 0.0251189F ) {
         /*MIN_QUA_ENER*/
         qua_ener = -32.0F;
      }
      else if ( pred_err_fact > 7.8125F ) {
         /*MAX_QUA_ENER*/
         qua_ener = 17.8558F;
      }
      else {
         qua_ener = ( Float32 )( 20.0F*log10( pred_err_fact ) );
      }
   }
   energy = (Word32)(qua_ener * 1024 + 0.5F);
   max = abs(energy - qua_gain_code[0]);
   index = 0;
   /* find match from table */
   for ( i = 1; i < NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+MR475_VQ_SIZE*2+3; i++ )
   {
      s = abs(energy - qua_gain_code[i]);
      if (s < max){
         max = s;
         index = i;
         if (s == 0) {
            break;
         }
      }
   }
   /* update MA predictor memory */
   for ( i = 3; i > 0; i-- ) {
      past_qua_en[i] = past_qua_en[i - 1];
   }
   past_qua_en[0] = index;
}


/*
 * MR475_gain_quant
 *
 *
 * Parameters:
 *    past_qua_en          B: MA predictor memory, 20*log10(qua_err)
 *    sf0_gcode0_exp(fra)  I: predicted CB gain from subframe 0 (or 2)
 *    sf0_coeff            I: energy coeff. from subframe 0 (or 2)
 *    sf0_target_en        I: target energy from subframe 0 (or 2)
 *    sf1_code_nosharp     I: innovative codebook vector (L_SUBFR)
 *                            (without pitch sharpening)
 *                            from subframe 1 (or 3)
 *    sf1_gcode0_exp(fra)  I: predicted CB gain from subframe 1 (or 3)
 *    sf1_coeff            I: energy coeff. subframe 1 (or 3)
 *    sf1_target_en        I: target energy from subframe 1 (or 3)
 *    gp_limit             I: pitch gain limit
 *    sf0_gain_pit         O: Pitch gain subframe 0 (or 2)
 *    sf0_gain_cod         O: Code gain subframe 0 (or 2)
 *    sf1_gain_pit         O: Pitch gain subframe 1 (or 3)
 *    sf1_gain_cod         O: Code gain subframe 1 (or 3)
 *
 * Function:
 *    Quantization of pitch and codebook gains for two subframes
 *    (using predicted codebook gain)
 *
 * Returns:
 *    index             index of quantization
 */
static Word16 MR475_gain_quant( Word32 *past_qua_en, Word32 sf0_gcode0_exp, Word32
                               sf0_gcode0_fra, Float32 sf0_coeff[], Float32 sf0_target_en,
                               Float32 sf1_code_nosharp[], Word32 sf1_gcode0_exp, Word32
                               sf1_gcode0_fra, Float32 sf1_coeff[], Float32 sf1_target_en,
                               Float32 gp_limit, Float32 *sf0_gain_pit, Float32
                               *sf0_gain_cod, Float32 *sf1_gain_pit, Float32 *sf1_gain_cod )
{
   Float32 temp, temp2, g_pitch, g2_pitch, g_code, g2_code, g_pit_cod, dist_min, sf0_gcode0, sf1_gcode0;
   const Float32 *p;
   Word32 i, tmp, g_code_tmp, gcode0, index = 0;

   sf0_gcode0 = (Float32)Pow2(sf0_gcode0_exp, sf0_gcode0_fra);
   sf1_gcode0 = (Float32)Pow2(sf1_gcode0_exp, sf1_gcode0_fra);

   if ( ( sf0_target_en * 2.0F ) < sf1_target_en ) {
      sf0_coeff[0] *= 2.0F;
      sf0_coeff[1] *= 2.0F;
      sf0_coeff[2] *= 2.0F;
      sf0_coeff[3] *= 2.0F;
      sf0_coeff[4] *= 2.0F;
   }
   else if ( sf0_target_en > ( sf1_target_en * 4.0F ) ) {
      sf1_coeff[0] *= 2.0F;
      sf1_coeff[1] *= 2.0F;
      sf1_coeff[2] *= 2.0F;
      sf1_coeff[3] *= 2.0F;
      sf1_coeff[4] *= 2.0F;
   }

   /*
    * Codebook search:
    * For each pair (g_pitch, g_fac) in the table calculate the
    * terms t[0..4] and sum them up; the result is the mean squared
    * error for the quantized gains from the table. The index for the
    * minimum MSE is stored and finally used to retrieve the quantized
    * gains
    */
   dist_min = FLT_MAX;
   p = &table_gain_MR475[0];

   for ( i = 0; i < MR475_VQ_SIZE; i++ ) {
      /* subframe 0 (and 2) calculations */
      g_pitch = *p++;
      g_code = *p++;
      g_code *= sf0_gcode0;
      g2_pitch = g_pitch * g_pitch;
      g2_code = g_code * g_code;
      g_pit_cod = g_code * g_pitch;
      temp = sf0_coeff[0] * g2_pitch;
      temp += sf0_coeff[1] * g_pitch;
      temp += sf0_coeff[2] * g2_code;
      temp += sf0_coeff[3] * g_code;
      temp += sf0_coeff[4] * g_pit_cod;
      temp2 = g_pitch - gp_limit;

      /* subframe 1 (and 3) calculations */
      g_pitch = *p++;
      g_code = *p++;

      if ( temp2 <= 0 && ( g_pitch <= gp_limit ) ) {
         g_code *= sf1_gcode0;
         g2_pitch = g_pitch * g_pitch;
         g2_code = g_code * g_code;
         g_pit_cod = g_code * g_pitch;
         temp += sf1_coeff[0] * g2_pitch;
         temp += sf1_coeff[1] * g_pitch;
         temp += sf1_coeff[2] * g2_code;
         temp += sf1_coeff[3] * g_code;
         temp += sf1_coeff[4] * g_pit_cod;

         /*
          * store table index if MSE for this index is lower
          * than the minimum MSE seen so far
          */
         if ( temp < dist_min ) {
            dist_min = temp;
            index = i;
         }
      }
   }

   /*
    *  read quantized gains and update MA predictor memories
    *
    * for subframe 0, the pre-calculated gcode0 is the same
    * as one calculated from the "real" predictor using quantized gains
    */
   tmp = index << 2;
   p = &table_gain_MR475[tmp];
   *sf0_gain_pit = *p++;
   g_code_tmp = (Word32)(*p++ * 4096 + 0.5F);

   gcode0 = Pow2( 14, sf0_gcode0_fra );
   if ( sf0_gcode0_exp < 11 ) {
      *sf0_gain_cod = (Float32)(( g_code_tmp * gcode0 ) >> ( 25 - sf0_gcode0_exp ));
   }
   else {
      i = ( ( g_code_tmp * gcode0 ) << ( sf0_gcode0_exp - 9 ) );

      if ( ( i >> ( sf0_gcode0_exp - 9 ) ) != ( g_code_tmp * gcode0 ) ) {
         *sf0_gain_cod = (Float32)0x7FFF;
      }
      else {
         *sf0_gain_cod = (Float32)(i >> 16);
      }
   }

   *sf0_gain_cod *= 0.5F;

   for ( i = 3; i > 0; i-- ) {
      past_qua_en[i] = past_qua_en[i - 1];
   }
   past_qua_en[0] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES + (index << 1);


   /*
    * calculate new predicted gain for subframe 1 (this time using
    * the real, quantized gains)
    */
   gc_pred( past_qua_en, MR475, sf1_code_nosharp, &sf1_gcode0_exp, &sf1_gcode0_fra, &sf0_gcode0 );

   tmp += 2;
   p = &table_gain_MR475[tmp];
   *sf1_gain_pit = *p++;
   g_code_tmp = (Word32)(*p++ * 4096 + 0.5F);

   gcode0 = Pow2( 14, sf1_gcode0_fra );
   if ( sf1_gcode0_exp < 11 ) {
      *sf1_gain_cod = (Float32)(( g_code_tmp * gcode0 ) >> ( 25 - sf1_gcode0_exp ));
   }
   else {
      i = ( ( g_code_tmp * gcode0 ) << ( sf1_gcode0_exp - 9 ) );

      if ( ( i >> ( sf1_gcode0_exp - 9 ) ) != ( g_code_tmp * gcode0 ) ) {
         *sf1_gain_cod = (Float32)0x7FFF;
      }
      else {
         *sf1_gain_cod = (Float32)(i >> 16);
      }
   }

   *sf1_gain_cod *= 0.5F;

   for ( i = 3; i > 0; i-- ) {
      past_qua_en[i] = past_qua_en[i - 1];
   }
   past_qua_en[0] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES + (index << 1) + 1;

   return( Word16 )index;
}


/*
 * q_gain_code
 *
 *
 * Parameters:
 *    gcode0            I: predicted CB gain
 *    gain              B: quantized fixed codebook gain
 *    qua_ener_index    O: quantized energy error index
 *
 * Function:
 *    Scalar quantization of the innovative codebook gain.
 *
 *    A correction factor between the gain gc and the estimated one gc'
 *    is given by:
 *
 *    cf = gc/gc'
 *
 *    The quantization table search is performed by minimizing the error:
 *
 *    Eq = POW[gc - cf' * gc', 2]
 *
 *    Once the optimum value cf' is chosen,
 *    the quantified fixed codebook gain is given by
 *
 *    gc_q = cf' * gc'
 *
 * Returns:
 *    index             quantization index
 */
static Word16 q_gain_code( Float32 gcode0, Float32 *gain, Word32 *qua_ener_index)
{
   Float64 err_min, err;
   const Float32 *p;
   Word32 i, index;


   p = &gain_factor[0];

   /* using abs instead pow */
   err_min = fabs( *gain - ( gcode0 * *p++ ) );
   index = 0;

   for ( i = 1; i < NB_QUA_CODE; i++ ) {
      err = fabs( *gain - ( gcode0 * *p++ ) );

      if ( err < err_min ) {
         err_min = err;
         index = i;
      }
   }
   p = &gain_factor[index];
   *gain = (Float32)floor(gcode0 * *p);
   *qua_ener_index = index;

   return( Word16 )index;
}


/*
 * MR795_gain_code_quant3
 *
 *
 * Parameters:
 *    gcode0            I: Predicted CB gain
 *    g_pitch_cand      I: Pitch gain candidates (3)
 *    g_pitch_cind      I: Pitch gain cand. indices (3)
 *    coeff             I: Energy coefficients
 *    gain_pit          O: Pitch gain
 *    gain_pit_ind      O: Pitch gain index
 *    gain_cod          O: Code gain
 *    gain_cod_ind      O: Code gain index
 *    qua_ener_index    O: quantized energy error index
 *
 * Function:
 *    Pre-quantization of codebook gains, given three possible
 *    LTP gains (using predicted codebook gain)
 *
 * Returns:
 *    void
 */
static void MR795_gain_code_quant3( Word32 gcode0_exp, Word32 gcode0_fra, Float32 g_pitch_cand[],
      Word32 g_pitch_cind[], Float32 coeff[], Float32 *gain_pit, Word32 *
      gain_pit_ind, Float32 *gain_cod, Word32 *gain_cod_ind, Word32 *qua_ener_index )
{
   Float32 gcode0, dist_min, g_pitch, g2_pitch, g_code, g2_code, g_pit_cod, tmp0, tmp;
   const Float32 *p;
   Word32 i, j, cod_ind, pit_ind, g_code0, g_code_tmp;

   gcode0 = (Float32)Pow2( gcode0_exp, gcode0_fra);
   /*
    * The error energy (sum) to be minimized consists of five terms, t[0..4].
    *
    * t[0] =    gp^2  * <y1 y1>
    * t[1] = -2*gp    * <xn y1>
    * t[2] =    gc^2  * <y2 y2>
    * t[3] = -2*gc    * <xn y2>
    * t[4] =  2*gp*gc * <y1 y2>
    */
   /*
    * Codebook search:
    * For each of the candiates LTP gains in g_pitch_cand[], the terms
    * t[0..4] are calculated from the values in the table (and the
    * pitch gain candidate) and summed up; the result is the mean
    * squared error for the LPT/CB gain pair. The index for the mini-
    * mum MSE is stored and finally used to retrieve the quantized CB
    * gain
    */
   dist_min = FLT_MAX;
   cod_ind = 0;
   pit_ind = 0;

   /* loop through LTP gain candidates */
   for ( j = 0; j < 3; j++ ) {
      /* pre-calculate terms only dependent on pitch gain */
      g_pitch = g_pitch_cand[j];
      g2_pitch = g_pitch * g_pitch;
      tmp0 = coeff[0] * g2_pitch;
      tmp0 += coeff[1] * g_pitch;
      p = &gain_factor[0];

      for ( i = 0; i < NB_QUA_CODE; i++ ) {
         /* this is g_fac */
         g_code = *p++;
         g_code = g_code * gcode0;
         g2_code = g_code * g_code;
         g_pit_cod = g_code * g_pitch;
         tmp = tmp0 + coeff[2] * g2_code;
         tmp += coeff[3] * g_code;
         tmp += coeff[4] * g_pit_cod;

         /*
          * store table index if MSE for this index is lower
          * than the minimum MSE seen so far; also store the
          * pitch gain for this (so far) lowest MSE
          */
         if ( tmp < dist_min ) {
            dist_min = tmp;
            cod_ind = i;
            pit_ind = j;
         }
      }
   }

   /*
    *  read quantized gains and new values for MA predictor memories
    */
   p = &gain_factor[cod_ind];
   g_code_tmp = (Word32)(2048 * *p);
   *qua_ener_index = cod_ind;

   /*
    *  calculate final fixed codebook gain:
    *   gc = gc0 * g
    */
   g_code0 = Pow2( 14, gcode0_fra);
   i = ( g_code_tmp * g_code0 ) << 1;
      gcode0_exp = 9 - gcode0_exp;

      if ( gcode0_exp > 0 )
         i = i >> gcode0_exp;
      else
         i = i << ( -gcode0_exp );
      *gain_cod = (Float32)(i >> 16);
     if (*gain_cod > 32767)
        *gain_cod = 32767;

     *gain_cod *= 0.5F;

   *gain_cod_ind = ( Word16 )cod_ind;
   *gain_pit = g_pitch_cand[pit_ind];
   *gain_pit_ind = g_pitch_cind[pit_ind];
}


/*
 * calc_unfilt_energies
 *
 *
 * Parameters:
 *    res               I: LP residual
 *    exc               I: LTP excitation (unfiltered)
 *    code              I: CB innovation (unfiltered)
 *    gain_pit          I: pitch gain
 *    en                O: energy coefficients [4]
 *    ltpg              O: LTP coding gain (log2())
 *
 * Function:
 *    Calculation of several energy coefficients for unfiltered
 *    excitation signals and the LTP coding gain
 *
 *    en[0] = <res res>    LP residual energy
 *    en[1] = <exc exc>    LTP residual energy
 *    en[2] = <exc code>   LTP/CB innovation dot product
 *    en[3] = <lres lres>  LTP residual energy
 *                         (lres = res - gain_pit*exc)
 *    ltpg = log2(LP_res_en / LTP_res_en)
 *
 * Returns:
 *    void
 */
static void calc_unfilt_energies( Float32 res[], Float32 exc[], Float32 code[],
      Float32 gain_pit, Float32 en[], Float32 *ltpg )
{
   Float32 sum, pred_gain;
   Word32 i;


   /* Compute residual energy */
   en[0] = (Float32)Dotproduct40( res, res );

   /* ResEn := 0 if ResEn < 200.0 */
   if ( en[0] < 200 ) {
      en[0] = 0;
   }

   /* Compute ltp excitation energy */
   en[1] = (Float32)Dotproduct40( exc, exc );

   /* Compute scalar product <exc[],code[]> */
   en[2] = (Float32)Dotproduct40( exc, code );

   /* Compute energy of LTP residual */
   en[3] = 0;

   for ( i = 0; i < L_SUBFR; i++ ) {
      /* LTP residual */
      sum = res[i] - ( exc[i] * gain_pit );
      en[3] += sum * sum;
   }

   /* calculate LTP coding gain, i.e. energy reduction LP res -> LTP res */
   if ( en[3] > 0 && en[0] != 0 ) {
      /* gain = ResEn / LTPResEn */
      pred_gain = en[0] / en[3];
      *ltpg = ( Float32 )( log10( pred_gain ) / log10( 2 ) );
   }
   else {
      *ltpg = 0;
   }
}


/*
 * gmed_n_f
 *
 *
 * Parameters:
 *    ind               I: values
 *    n                 I: The number of gains
 *
 * Function:
 *    Calculates N-point median (float).
 *
 * Returns:
 *    index of the median value
 */
static Float32 gmed_n_f( Float32 ind[], Word16 n )
{
   Word32 medianIndex;
   Word32 i, j, ix = 0;
   Word32 tmp[9];
   Float32 tmp2[9];
   Float32 max;


   for ( i = 0; i < n; i++ ) {
      tmp2[i] = ind[i];
   }

   for ( i = 0; i < n; i++ ) {
      max = -FLT_MAX;

      for ( j = 0; j < n; j++ ) {
         if ( tmp2[j] >= max ) {
            max = tmp2[j];
            ix = j;
         }
      }
      tmp2[ix] = -FLT_MAX;
      tmp[i] = ix;
   }
   medianIndex = tmp[n >> 1];
   return( ind[medianIndex] );
}


/*
 * gain_adapt
 *
 *
 * Parameters:
 *    prev_gc           B: previous code gain
 *    onset             B: onset indicator
 *    ltpg_mem          B: stored past LTP coding gains
 *    prev_alpha        B: revious gain adaptation factor
 *    ltpg              I: ltp coding gain (log2())
 *    gain_cod          I: code gain
 *    alpha             O: gain adaptation factor
 *
 * Function:
 *    Calculate pitch/codebook gain adaptation factor alpha
 *    (and update the adaptor state)
 *
 *    If the coding gain ag is less than 1 dB, the modified criterion is
 *    employed, except when an onset is detected. An onset is said to be
 *    detected if the fixed codebook gain in the current subframe is more
 *    than twice the value of the fixed codebook gain in the previous
 *    subframe. A hangover of 8 subframes is used in the onset detection
 *    so that the modified criterion is not used for the next 7 subframes
 *    either if an onset is detected. The balance factor a is computed from
 *    the median filtered adaptive coding gain. The current and
 *    the ag-values for the previous 4 subframes are median filtered to get
 *    agm. The  a-factor is computed by:
 *
 *          /  0                       ,  2 < agm
 *       a =   0.5 * (1 - 0.5 * agm)   ,  0 < agm < 2
 *          \  0.5                     ,      agm < 0
 *
 * Returns:
 *    void
 */
static void gain_adapt( Float32 *prev_gc, Word16 *onset, Float32 *ltpg_mem,
      Float32 *prev_alpha, Float32 ltpg, Float32 gain_cod, Float32 *alpha )
{
   Float32 result, filt;   /* alpha factor, median-filtered LTP coding gain */
   Word32 i;
   Word16 adapt;   /* adaptdation status; 0, 1, or 2 */


   /* basic adaptation */
   if ( ltpg <= 0.3321928F /*LTP_GAIN_THR1*/ ) {
      adapt = 0;
   }
   else {
      if ( ltpg <= 0.6643856 /*LTP_GAIN_THR2*/ ) {
         adapt = 1;
      }
      else {
         adapt = 2;
      }
   }

   /*
    * onset indicator:
    */
   if ( ( gain_cod > 2.0F * *prev_gc ) && ( gain_cod > 100 ) ) {
      *onset = 8;
   }
   else {
      if ( *onset != 0 ) {
         (*onset)--;
      }
   }

   /*
    * if onset, increase adaptor state
    */
   if ( ( *onset != 0 ) && ( adapt < 2 ) ) {
      adapt++;
   }
   ltpg_mem[0] = ltpg;
   filt = gmed_n_f( ltpg_mem, 5 );

   if ( adapt == 0 ) {
      if ( filt > 0.66443 ) {
         result = 0;
      }
      else {
         if ( filt < 0 ) {
            result = 0.5;
         }
         else {
            result = ( Float32 )( 0.5-0.75257499*filt );
         }
      }
   }
   else {
      result = 0;
   }

   /*
    * if (prev_alpha == 0.0) result = 0.5 * (result + prev_alpha);
    */
   if ( *prev_alpha == 0 ) {
      result = 0.5F * result;
   }

   /* store the result */
   *alpha = result;

   /* update adapter state memory */
   *prev_alpha = result;
   *prev_gc = gain_cod;

   for ( i = LTPG_MEM_SIZE - 1; i > 0; i-- ) {
      ltpg_mem[i] = ltpg_mem[i - 1];
   }

   /* mem[0] is just present for convenience in calling the gmed_n[5]
    * function above. The memory depth is really LTPG_MEM_SIZE-1.
    */
}


/*
 * MR795_gain_code_quant_mod
 *
 *
 * Parameters:
 *    gain_pit          I: Pitch gain
 *    gcode0            I: Predicted CB gain
 *    en                I: energy coefficients
 *    alpha             I: gain adaptation factor
 *    gain_cod_unq      I: Code gain (unquantized)
 *    gain_cod          I: Code gain
 *    qua_ener_index    O: quantized energy error index
 *
 * Function:
 *   Modified quantization of the MR795 codebook gain
 *
 *   Uses pre-computed energy coefficients in frac_en[]/exp_en[]
 *
 *   frac_en[0]*2^exp_en[0] = <res res>     LP residual energy
 *   frac_en[1]*2^exp_en[1] = <exc exc>     LTP residual energy
 *   frac_en[2]*2^exp_en[2] = <exc code>    LTP/CB innovation dot product
 *   frac_en[3]*2^exp_en[3] = <code code>   CB innovation energy
 *
 * Returns:
 *    index             codebook index
 */
static Word16 MR795_gain_code_quant_mod( Float32 gain_pit, Word32 gcode0_exp, Word32 gcode0_fra,
      Float32 en[], Float32 alpha, Float32 gain_cod_unq, Float32 *gain_cod,
      Word32 *qua_ener_index )
{
   Float32 coeff[5];
   Float32 gcode0, g2_pitch, g_code, g2_code, d2_code, dist_min, gain_code, tmp;
   const Float32 *p;
   Word32 i, index, g_code_tmp, g_code0;

   gcode0 = (Float32)Pow2(gcode0_exp, gcode0_fra);

   gain_code = *gain_cod;
   g2_pitch = gain_pit * gain_pit;
   coeff[0] = ( Float32 )( sqrt( alpha * en[0] ) );
   coeff[1] = alpha * en[1] * g2_pitch;
   coeff[2] = 2.0F * alpha * en[2] * gain_pit;
   coeff[3] = alpha * en[3];
   coeff[4] = ( 1.0F - alpha ) * en[3];

   /* search the quantizer table for the lowest value of the search criterion */
   dist_min = FLT_MAX;
   index = 0;
   p = &gain_factor[0];

   for ( i = 0; i < NB_QUA_CODE; i++ ) {
      /* this is g_fac */
      g_code = *p++;
      g_code = g_code * gcode0;

      /*
       * only continue if gc[i] < 2.0*gc
       */
      if ( g_code >= ( 2.0F * gain_code ) )
         break;
      g2_code = g_code * g_code;
      d2_code = g_code - gain_cod_unq;
      d2_code = d2_code * d2_code;
      tmp = coeff[1] + coeff[2] * g_code;
      tmp += coeff[3] * g2_code;
      tmp = ( Float32 )sqrt( tmp );
      tmp = tmp - coeff[0];
      tmp = tmp * tmp;
      tmp += coeff[4] * d2_code;

      /*
       * store table index if distance measure for this
       * index is lower than the minimum seen so far
       */
      if ( tmp < dist_min ) {
         dist_min = tmp;
         index = i;
      }
   }

   /*
    *  read quantized gains and new values for MA predictor memories
    */
   p = &gain_factor[index];
   g_code_tmp = (Word32)(2048 * *p);
   *qua_ener_index = index;

   /* calculate final fixed codebook gain:
    * gc = gc0 * g
    */
   g_code0 = Pow2( 14, gcode0_fra);
   i = ( g_code_tmp * g_code0 ) << 1;
      gcode0_exp = 9 - gcode0_exp;

      if ( gcode0_exp > 0 )
         i = i >> gcode0_exp;
      else
         i = i << ( -gcode0_exp );
      *gain_cod = (Float32)(i >> 16);
      if (*gain_cod > 32767)
        *gain_cod = 32767;

     *gain_cod *= 0.5F;
   return( Word16 )index;
}


/*
 * MR795_gain_quant
 *
 *
 * Parameters:
 *    prev_gc           B: previous code gain
 *    onset             B: onset indicator
 *    ltpg_mem          B: stored past LTP coding gains
 *    prev_alpha        B: previous gain adaptation factor
 *    res               I: LP residual
 *    exc               I: LTP excitation (unfiltered)
 *    code              I: CB innovation (unfiltered)
 *    coeff             I: energy coefficients (5)
 *    code_en           I: innovation energy
 *    gcode0            I: predicted CB gain
 *    cod_gain          I: codebook gain
 *    gp_limit          I: pitch gain limit
 *    gain_pit          B: Pitch gain
 *    gain_cod          O: Code gain
 *    qua_ener          O: quantized energy error
 *    anap              O: Index of quantization
 *    gain_pit          I: Pitch gain
 *    gcode0            I: Predicted CB gain
 *    en                I: energy coefficients
 *    alpha             I: gain adaptation factor
 *    gain_cod_unq      I: Code gain (unquantized)
 *    gain_cod          I: Code gain
 *    qua_ener          O: quantized energy error_index
 *
 * Function:
 *    Pitch and codebook quantization for MR795
 *
 * Returns:
 *    void
 */
static void MR795_gain_quant( Float32 *prev_gc, Word16 *onset, Float32 *ltpg_mem
      , Float32 *prev_alpha, Float32 res[], Float32 exc[], Float32 code[],
      Float32 coeff[], Float32 code_en, Word32 gcode0_exp, Word32 gcode0_fra, Float32 cod_gain,
      Float32 gp_limit, Float32 *gain_pit, Float32 *gain_cod, Word32 *qua_ener_index,
      Word16 **anap )
{
   Float32 en[4], g_pitch_cand[3];
   Float32 ltpg, alpha, gain_cod_unq;   /* code gain (unq.) */
   Word32 g_pitch_cind[3];   /* pitch gain indices */
   Word32 gain_pit_index, gain_cod_index;


   /*
    * get list of candidate quantized pitch gain values
    * and corresponding quantization indices
    */
   gain_pit_index = q_gain_pitch( MR795, gp_limit, gain_pit, g_pitch_cand,
         g_pitch_cind );

   /*
    * pre-quantization of codebook gain
    * (using three pitch gain candidates);
    * result: best guess of pitch gain and code gain
    */
   MR795_gain_code_quant3( gcode0_exp, gcode0_fra, g_pitch_cand, g_pitch_cind, coeff, gain_pit,
         &gain_pit_index, gain_cod, &gain_cod_index, qua_ener_index );

   /* calculation of energy coefficients and LTP coding gain */
   calc_unfilt_energies( res, exc, code, *gain_pit, en, &ltpg );

   /*
    * run gain adaptor, calculate alpha factor to balance LTP/CB gain
    * (this includes the gain adaptor update)
    */
   gain_adapt( prev_gc, onset, ltpg_mem, prev_alpha, ltpg, *gain_cod, &alpha );

   /*
    * if this is a very low energy signal (threshold: see
    * calc_unfilt_energies) or alpha <= 0 then don't run the modified quantizer
    */
   if ( ( en[0] != 0 ) && ( alpha > 0 ) ) {
      /*
       * innovation energy <cod cod> was already computed in gc_pred()
       * (this overwrites the LtpResEn which is no longer needed)
       */
      en[3] = code_en;

      /*
       * store optimum codebook gain
       */
      gain_cod_unq = cod_gain;

      /* run quantization with modified criterion */
      gain_cod_index = MR795_gain_code_quant_mod( *gain_pit, gcode0_exp, gcode0_fra, en, alpha,
            gain_cod_unq, gain_cod, qua_ener_index );
   }
   *( *anap )++ = ( Word16 )gain_pit_index;
   *( *anap )++ = ( Word16 )gain_cod_index;
}


/*
 * Qua_gain
 *
 *
 * Parameters:
 *    mode              I: AMR mode
 *    gcode0            I: predicted CB gain
 *    coeff             I: energy coefficients (5)
 *    gp_limit          I: pitch gain limit
 *    gain_pit          O: Pitch gain
 *    gain_cod          O: Code gain
 *    qua_ener_index    O: quantized energy error index
 *
 * Function:
 *    Quantization of pitch and codebook gains (using predicted codebook gain)
 *
 * Returns:
 *    index             index of quantization
 */
static Word16 Qua_gain( enum Mode mode, Word32 gcode0_exp, Word32 gcode0_fra, Float32 coeff[], Float32
      gp_limit, Float32 *gain_pit, Float32 *gain_cod, Word32 *qua_ener_index)
{
   Float32 g_pitch, g2_pitch, g_code, g2_code, g_pit_cod, tmp, dist_min, gcode0;
   const Float32 *table_gain, *p;
   Word32 i, index = 0, gcode_0, g_code_tmp;
   Word16 table_len;

   gcode0 = (Float32)Pow2( gcode0_exp, gcode0_fra );


   if ( ( mode == MR102 ) || ( mode == MR74 ) || ( mode == MR67 ) ) {
      table_len = VQ_SIZE_HIGHRATES;
      table_gain = table_highrates;
      *qua_ener_index = NB_QUA_CODE;
   }
   else {
      table_len = VQ_SIZE_LOWRATES;
      table_gain = table_lowrates;
      *qua_ener_index = NB_QUA_CODE + VQ_SIZE_HIGHRATES;
   }

   /*
    * Codebook search:
    * For each pair (g_pitch, g_fac) in the table calculate the
    * terms t[0..4] and sum them up; the result is the mean squared
    * error for the quantized gains from the table. The index for the
    * minimum MSE is stored and finally used to retrieve the quantized
    * gains
    */
   dist_min = FLT_MAX;
   p = &table_gain[0];

   for ( i = 0; i < table_len; i++ ) {
      g_pitch = *p++;

      /* this is g_fac */
      g_code = *p++;

      if ( g_pitch <= gp_limit ) {
         g_code *= gcode0;
         g2_pitch = g_pitch * g_pitch;
         g2_code = g_code * g_code;
         g_pit_cod = g_code * g_pitch;
         tmp = coeff[0] * g2_pitch;
         tmp += coeff[1] * g_pitch;
         tmp += coeff[2] * g2_code;
         tmp += coeff[3] * g_code;
         tmp += coeff[4] * g_pit_cod;

         /*
          * store table index if MSE for this index is lower
          * than the minimum MSE seen so far
          */
         if ( tmp < dist_min ) {
            dist_min = tmp;
            index = i;
         }
      }
   }

   /*
    * read quantized gains and new values for MA predictor memories
    */
   p = &table_gain[index << 1];
   *gain_pit = *p++;
   g_code_tmp = (Word32)(4096 * *p);

   /*
    * calculate final fixed codebook gain:
    * gc = gc0 * g
    */

   gcode_0 = Pow2( 14, gcode0_fra );
   if ( gcode0_exp < 11 ) {
      *gain_cod = (Float32)((g_code_tmp * gcode_0) >> ( 25 - gcode0_exp ));
   }
   else {
      i = ( ( g_code_tmp * gcode_0) << ( gcode0_exp - 9 ) );

      if ( ( i >> ( gcode0_exp - 9 ) ) != ( g_code_tmp * gcode_0) ) {
         *gain_cod = 0x7FFF;
      }
      else {
         *gain_cod = (Float32)(i >> 16);
      }
   }
   *gain_cod = *gain_cod * 0.5F;
   *qua_ener_index += index;

   return( Word16 )index;
}


/*
 * gainQuant
 *
 *
 * Parameters:
 *    gcode0            I: predicted CB gain
 *    coeff             I: energy coefficients (5)
 *    gp_limit          I: pitch gain limit
 *    gain_pit          O: Pitch gain
 *    gain_cod          O: Code gain
 *    qua_ener          O: quantized energy error,
 *    mode              I: AMR mode
 *    even_subframe     I: even subframe indicator flag
 *    past_qua_en       B: past quantized energies [4]
 *    past_qua_en_unq   B: past energies [4]
 *    sf0_coeff         B: energy coefficients subframe 0 (or 2)
 *    sf0_target_en     B: target energy from subframe 0 (or 2)
 *    sf0_gcode0        B: predicted gain factor subframe 0 (or 2)
 *    gain_idx_ptr      B: gain index pointer
 *    sf0_gain_pit      B: Pitch gain subframe 0 (or 2)
 *    sf0_gain_cod      B: Code gain subframe 0 (or 2)
 *    res               I: LP residual
 *    exc               I: LTP excitation (unfiltered)
 *    code              I: innovative codebook vector
 *    xn                I: Target vector
 *    xn2               I: CB target vector
 *    y1                I: Adaptive codebook
 *    y2                I: Filtered innovative vector
 *    gCoeff            I: Correlations <xn y1> <y1 y1>
 *    gp_limit          I: pitch gain limit
 *    gain_pit          O: Pitch gain
 *    gain_cod          O: Code gain
 *    prev_gc           B: B: previous code gain
 *    onset             B: onset indicator
 *    ltpg_mem          B: stored past LTP coding gains
 *    prev_alpha        B: previous gain adaptation factor
 *    anap              B: Index of quantization
 *
 * Function:
 *    Quantization of gains
 *
 * Returns:
 *    index             index of quantization
 */
static void gainQuant( enum Mode mode, Word32 even_subframe, Word32 *
      past_qua_en, Word32 *past_qua_en_unq, Float32 *sf0_coeff, Float32 *
      sf0_target_en, Word32 *sf0_gcode0_exp, Word32 *sf0_gcode0_fra,Word16 **gain_idx_ptr, Float32 *
      sf0_gain_pit, Float32 *sf0_gain_cod, Float32 *res, Float32 *exc, Float32
      code[], Float32 xn[], Float32 xn2[], Float32 y1[], Float32 y2[], Float32
      gCoeff[], Float32 gp_limit, Float32 *gain_pit, Float32 *gain_cod, Float32
      *prev_gc, Word16 *onset, Float32 *ltpg_mem, Float32 *prev_alpha, Word16 **
      anap )
{
   Float32 coeff[5];
   Float32 gcode0, cod_gain, en = 0;
   Word32 i, exp, frac, qua_ener_index;


   if ( mode == MR475 ) {
      if ( even_subframe != 0 ) {
      /*
       * save position in output parameter stream and current
       * state of codebook gain predictor
       */
         *gain_idx_ptr = ( *anap )++;
         past_qua_en_unq[0] = past_qua_en[0];
         past_qua_en_unq[1] = past_qua_en[1];
         past_qua_en_unq[2] = past_qua_en[2];
         past_qua_en_unq[3] = past_qua_en[3];

         /*
          * predict codebook gain (using "unquantized" predictor)
          * (note that code[] is unsharpened in MR475)
          */
         gc_pred( past_qua_en, mode, code, sf0_gcode0_exp, sf0_gcode0_fra, &en );
         gcode0 = (Float32)Pow2(*sf0_gcode0_exp, *sf0_gcode0_fra);

         /*
          * calculate energy coefficients for quantization
          * and store them in state structure (will be used
          * in next subframe when real quantizer is run)
          */
         calc_filt_energies( mode, xn, xn2, y1, y2, gCoeff, sf0_coeff, &cod_gain
               );

         /* store optimum codebook gain */
         *gain_cod = cod_gain;
         *sf0_target_en = (Float32)Dotproduct40( xn, xn );

         /*
          * calculate optimum codebook gain and update
          * "unquantized" predictor
          */
         MR475_update_unq_pred( past_qua_en_unq, gcode0, cod_gain );

         /* the real quantizer is not run here... */
      }
      else {
         /*
          * predict codebook gain (using "unquantized" predictor)
          * (note that code[] is unsharpened in MR475)
          */
         gc_pred( past_qua_en_unq, mode, code, &exp, &frac, &en );

         /* calculate energy coefficients for quantization */
         calc_filt_energies( mode, xn, xn2, y1, y2, gCoeff, coeff, &cod_gain );
         en = (Float32)Dotproduct40( xn, xn );

         /* run real (4-dim) quantizer and update real gain predictor */
         **gain_idx_ptr = MR475_gain_quant( past_qua_en, *sf0_gcode0_exp, *sf0_gcode0_fra, sf0_coeff,
               *sf0_target_en, code, exp, frac, coeff, en, gp_limit, sf0_gain_pit,
               sf0_gain_cod, gain_pit, gain_cod );
      }
   }
   else {
      /*
       * predict codebook gain and quantize
       *  (also compute normalized CB innovation energy for MR795)
       */
      gc_pred( past_qua_en, mode, code, &exp, &frac, &en );

      if ( mode == MR122 ) {
         /*
          * Compute the innovative codebook gain.
          * The innovative codebook gain is given by
          * g = <xn2[], y2[]> / <y2[], y2[]>
          * where xn2[] is the target vector,
          * y2[] is the filtered innovative
          * codevector
          */

         gcode0 = (Float32)Pow2( exp, frac );
         /* saturation at decoder */
         if (gcode0 > 2047.9375F) gcode0 = 2047.9375F;

         *gain_cod = (Float32)(Dotproduct40( xn2, y2 ) / ( Dotproduct40( y2, y2 )+ 0.01F ));

         if ( *gain_cod < 0 )
            *gain_cod = 0.0F;
         *( *anap )++ = q_gain_code( gcode0, gain_cod,&qua_ener_index);
      }
      else {
         /* calculate energy coefficients for quantization */
         calc_filt_energies( mode, xn, xn2, y1, y2, gCoeff, coeff, &cod_gain );

         if ( mode == MR795 ) {
            MR795_gain_quant( prev_gc, onset, ltpg_mem, prev_alpha, res, exc,
                  code, coeff, en, exp, frac , cod_gain, gp_limit, gain_pit,
                  gain_cod, &qua_ener_index, anap );
         }
         else {

            *( *anap )++ = Qua_gain( mode, exp, frac, coeff, gp_limit, gain_pit,
                  gain_cod, &qua_ener_index);
         }
      }

      /*
       * update table of past quantized energies
       */
      for ( i = 3; i > 0; i-- ) {
         past_qua_en[i] = past_qua_en[i - 1];
      }
      past_qua_en[0] = qua_ener_index;

   }
}


/*
 * subframePostProc
 *
 *
 * Parameters:
 *    speech            I: Speech segment
 *    i_subfr           I: Subframe number
 *    gain_pit          I: Pitch gain
 *    gain_code         I: Decoded innovation gain
 *    a_q               I: A(z) quantized for the 4 subframes
 *    synth             I: Local synthesis
 *    xn                I: Target vector for pitch search
 *    code              I: Fixed codebook exitation
 *    y1                I: Filtered adaptive exitation
 *    y2                I: Filtered fixed codebook excitation
 *    mem_syn           B: memory of synthesis filter
 *    mem_err           O: pointer to error signal
 *    mem_w0            O: memory of weighting filter
 *    exc               O: long term prediction residual
 *    sharp             O: pitch sharpening value
 *
 * Function:
 *    Subframe post processing
 *
 *    Memory update (all modes)
 *    An update of the states of the synthesis and weighting filters is needed
 *   in order to compute the target signal in the next subframe.
 *   After the two gains are quantified, the excitation signal, u(n),
 *   in the present subframe is found by:
 *
 *   u(n) = Gp_q * v(n) + Gc_q * c(n), n = 0, ..., 39,
 *
 *   where Gp_q and Gc_q are the quantified adaptive and fixed codebook gains,
 *   respectively, v(n) the adaptive codebook vector
 *   (interpolated past excitation), and c(n) is the fixed codebook vector
 *   (algebraic code including pitch sharpening). The states of the filters
 *   can be updated by filtering the signal res_lp(n) - u(n)
 *   (difference between residual and excitation) through the filters
 *   1 / A_q(z) and A(z/g1) / A(z/g2) for the 40-sample subframe and saving
 *   the states of the filters. This would require 3 filterings.
 *   A simpler approach which requires only one filtering is as follows.
 *   The local synthesized speech, S_s(n), is computed by filtering
 *   the excitation signal through 1 / A_q(z). The output of the filter
 *   due to the input res_lp(n) - u(n) is equivalent to e(n) = S(n) - S_s(n).
 *   So the states of the synthesis filter 1 / A_q(z) are given by
 *   e(n), n = 30, ..., 39. Updating the states of the filter A(z/g1) / A(z/g2)
 *   can be done by filtering the error signal e(n) through this filter
 *   to find the perceptually weighted error ew(n). However, the signal ew(n)
 *   can be equivalently found by:
 *
 *   ew(n) = x(n) - Gp_q * y(n) - Gc_q(n) * z(n)
 *
 *   Since the signals x(n), y(n) and z(n) are available, the states of
 *   the weighting filter are updated by computing ew(n) for n = 30, ..., 39.
 *   This saves two filterings.
 *
 * Returns:
 *    void
 */
static void subframePostProc( Float32 *speech, Word16 i_subfr, Float32 gain_pit,
      Float32 gain_code, Float32 *a_q, Float32 synth[], Float32 xn[], Float32
      code[], Float32 y1[], Float32 y2[], Float32 *mem_syn, Float32 *mem_err,
      Float32 *mem_w0, Float32 *exc, Float32 *sharp )
{
   Word32 i, j;


   /*
    * Update pitch sharpening "sharp" with quantized gain_pit
    */
   *sharp = gain_pit;
   if ( *sharp > 0.794556F ) {
      *sharp = 0.794556F;
   }

   /* Find the total excitation */
   for ( i = 0; i < L_SUBFR; i += 4 ) {
      exc[i + i_subfr] = (Float32)floor((gain_pit * exc[i + i_subfr] + gain_code * code[i]) + 0.5F);
      exc[i + i_subfr + 1] = (Float32)floor((gain_pit * exc[i + i_subfr + 1] + gain_code * code[i
            + 1]) + 0.5F);
      exc[i + i_subfr + 2] = (Float32)floor((gain_pit * exc[i + i_subfr + 2] + gain_code * code[
            i + 2]) + 0.5F);
      exc[i + i_subfr + 3] = (Float32)floor((gain_pit * exc[i + i_subfr + 3] + gain_code * code[
            i + 3]) + 0.5F);
   }


   /* The local synthesis speech */
   Syn_filt( a_q, &exc[i_subfr], &synth[i_subfr], mem_syn, 1 );

   for ( i = L_SUBFR - M, j = 0; i < L_SUBFR; i++, j++ ) {
      /* e(n) = S(n) - S_s(n) */
      mem_err[j] = speech[i_subfr + i] - synth[i_subfr + i];

      /* ew(n) = x(n) - Gp_q * y(n) - Gc_q(n) * z(n) */
      mem_w0[j] = xn[i] - y1[i] * gain_pit - y2[i] * gain_code;
   }
}


/*
 * Convolve
 *
 *
 * Parameters:
 *    x                 I: First input
 *    h                 I: second input
 *    y                 O: output
 *
 * Function:
 *    Convolution
 *
 * Returns:
 *    void
 */
static void Convolve( Float32 x[], Float32 h[], Float32 y[] )
{
   Word32 i, n;
   Float32 s;


   for ( n = 0; n < L_SUBFR; n++ ) {
      s = 0.0F;

      for ( i = 0; i <= n; i++ ) {
         s += x[i] * h[n - i];
      }
      y[n] = s;
   }
   return;
}


/*
 * tx_dtx_handler
 *
 *
 * Parameters:
 *    vad_flag             I:vad decision
 *    decAnaElapsedCount   B: state machine that synch with the GSMEFR txDtx machine
 *    dtxHangoverCount     B: dtx hangover counter
 *    used_mode            O: used mode
 *
 * Function:
 *    Adds extra speech hangover to analyze speech on the decoding side
 *
 * Returns:
 *    compute_new_sid_possible
 */
static Word16 tx_dtx_handler( Word16 vad_flag, Word16 *decAnaElapsedCount,
      Word16 *dtxHangoverCount, enum Mode *used_mode )
{
   Word16 compute_new_sid_possible;


   /* this state machine is in synch with the GSMEFR txDtx machine */
   *decAnaElapsedCount += 1;
   compute_new_sid_possible = 0;

   if ( vad_flag != 0 ) {
      *dtxHangoverCount = DTX_HANG_CONST;
   }

   /* non-speech */
   else {
      /* out of decoder analysis hangover */
      if ( *dtxHangoverCount == 0 ) {
         *decAnaElapsedCount = 0;
         *used_mode = MRDTX;
         compute_new_sid_possible = 1;
      }

      /* in possible analysis hangover */
      else {
         *dtxHangoverCount -= 1;

         /* decAnaElapsedCount + dtxHangoverCount < DTX_ELAPSED_FRAMES_THRESH */
         if ( ( *decAnaElapsedCount + *dtxHangoverCount ) <
               DTX_ELAPSED_FRAMES_THRESH ) {
            *used_mode = MRDTX;

            /* if short time since decoder update, do not add extra HO */
         }

         /*
          * else
          * override VAD and stay in
          * speech mode *used_mode
          * and add extra hangover
          */
      }
   }
   return compute_new_sid_possible;
}


/*
 * dtx_buffer
 *
 *
 * Parameters:
 *    hist_ptr       B: Circular buffer pointer
 *    lsp_hist       O: LSP history
 *    lsp_new        I: LSP vector
 *    speech         I: input speech
 *    log_en_hist    O: frame energy
 *
 * Function:
 *    Handles the DTX buffer
 *
 *    The frame energy is computed for each frame marked with VAD=0
 *    according to the equation:
 *
 *                                       N-1
 *    en_log(i) = 0.5 * log2 ( (1 / N) * SUM[ s(n) * s(n) ] )
 *                                       N=0
 *
 *    where s(n) is the HP-filtered input speech signal of
 *    the current frame i.
 *
 * Returns:
 *    void
 */
static void dtx_buffer( Word16 *hist_ptr, Float32 *lsp_hist, Float32 lsp_new[],
      Float32 speech[], Float32 *log_en_hist )
{
   Float64 frame_en;


   /* update pointer to circular buffer */
   *hist_ptr += 1;

   if ( *hist_ptr == DTX_HIST_SIZE ) {
      *hist_ptr = 0;
   }

   /* copy lsp vector into buffer */
   memcpy( &lsp_hist[ * hist_ptr * M], lsp_new, sizeof( Float32 )*M );

   /* compute log energy based on frame energy */
   frame_en = Dotproduct40( speech, speech );
   frame_en += Dotproduct40( &speech[40], &speech[40] );
   frame_en += Dotproduct40( &speech[80], &speech[80] );
   frame_en += Dotproduct40( &speech[120], &speech[120] );

   if ( frame_en > 1 ) {
      log_en_hist[ * hist_ptr] = ( Float32 )( log10( frame_en * 0.00625F )*
            1.660964F );
   }
   else {
      log_en_hist[ * hist_ptr] = -3.660965F;
   }
}


/*
 * dtx_enc
 *
 *
 * Parameters:
 *    log_en_index      O: logarithmic energy quantized
 *    log_en_hist       I: history of logarithmic energy
 *    lsp_hist          I: history of LSP
 *    lsp_index         O: quantization indices of 3 LSP vectors
 *    init_lsf_vq_index O: init index for MA prediction
 *    compute_sid_flag  I: SID flag
 *    past_rq           I: past quantized residual
 *    past_qua_en       O: gain predictor memory
 *    anap              O: analysis parameters
 *
 * Function:
 *    DTX encoder
 *
 *    The averaged logarithmic energy is computed by:
 *
 *                          7
 *    en_log_mean(i) = (1 / 8) * SUM[ en_log(i - n) ]
 *                         n=0
 *
 *    The averaged logarithmic energy is quantized means of a 6 bit
 *    algorithmic quantizer. The 6 bits for the energy index are
 *    transmitted in the SID frame.
 *
 * Returns:
 *    void
 */
static Word32 dtx_enc( Word16 *log_en_index, Float32 log_en_hist[], Float32
      lsp_hist[], Word16 *lsp_index, Word32 *init_lsf_vq_index, Word16
      compute_sid_flag, Float32 past_rq[], Word32 *past_qua_en, Word16 **anap )
{
   Float32 log_en, lsf[M], lsp[M], lsp_q[M];
   Word32 i, j;


   /* VOX mode computation of SID parameters */
   if ( ( compute_sid_flag != 0 ) ) {
   /*
    * compute new SID frame if safe i.e don't
    * compute immediately after a talk spurt
    */
      log_en = 0;
      memset( lsp, 0, sizeof( Float32 )*M );

      /* average energy and lsp */
      for ( i = 0; i < DTX_HIST_SIZE; i++ ) {
         log_en += log_en_hist[i];

         for ( j = 0; j < M; j++ ) {
            lsp[j] += lsp_hist[i * M + j];
         }
      }
      log_en = log_en * 0.125F;

      for ( j = 0; j < M; j++ ) {
         /* divide by 8 */
         lsp[j] = lsp[j] * 0.125F;
      }

      /*  quantize logarithmic energy to 6 bits */
      log_en = log_en + 2.5F;
      *log_en_index = ( Word16 )( ( log_en * 4 ) + 0.5F );   /* 6 bits */

      /* *log_en_index = ( Word16 )( ( log_en + 2.5F + 0.125F ) * 4.0F ); */
      if ( *log_en_index > 63 ) {
         *log_en_index = 63;
      }

      if ( *log_en_index < 0 ) {
         *log_en_index = 0;
      }

      if (*log_en_index > 46){
         past_qua_en[0] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + 46;
         past_qua_en[1] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + 46;
         past_qua_en[2] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + 46;
         past_qua_en[3] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + 46;
      }
      else {
         past_qua_en[0] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + *log_en_index;
         past_qua_en[1] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + *log_en_index;
         past_qua_en[2] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + *log_en_index;
         past_qua_en[3] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+(MR475_VQ_SIZE*2) + *log_en_index;
      }

      /* make sure that LSP's are ordered */
      Lsp_lsf( lsp, lsf );
      Reorder_lsf( lsf, 0.00625F );
      Lsf_lsp( lsf, lsp );

      /* Quantize lsp and put on parameter list */
      Q_plsf_3( MRDTX, past_rq, lsp, lsp_q, lsp_index, init_lsf_vq_index );
   }

   /* 3 bits */
   *( *anap )++ = ( Word16 )*init_lsf_vq_index;

   /* 8 bits */
   *( *anap )++ = lsp_index[0];

   /* 9 bits */
   *( *anap )++ = lsp_index[1];

   /* 9 bits */
   *( *anap )++ = lsp_index[2];

   /* 6 bits */
   *( *anap )++ = *log_en_index;

   /* = 35 bits */
   return 0;
}


/*
 * complex_estimate_adapt
 *
 *
 * Parameters:
 *    st->best_corr_hp  I: complex background detector
 *    st->corr_hp_fast  B: long term complex signal estimate
 *    low_power         I: very low level flag of the input frame
 *
 * Function:
 *    Update/adapt of complex signal estimate
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void complex_estimate_adapt( vadState *st, Word16 low_power )
{
   Float32 alpha;


   /* adapt speed on own state */
   /* decrease */
   if ( st->best_corr_hp < st->corr_hp_fast ) {
      /* low state */
      if ( st->corr_hp_fast < CVAD_THRESH_ADAPT_HIGH ) {
         alpha = CVAD_ADAPT_FAST;
      }

      /* high state */
      else {
         alpha = CVAD_ADAPT_REALLY_FAST;
      }
   }

   /* increase */
   else {
      if ( st->corr_hp_fast < CVAD_THRESH_ADAPT_HIGH ) {
         alpha = CVAD_ADAPT_FAST;
      }
      else {
         alpha = CVAD_ADAPT_SLOW;
      }
   }
   st->corr_hp_fast = st->corr_hp_fast - alpha * st->corr_hp_fast + alpha * st->
         best_corr_hp;

   if ( st->corr_hp_fast < CVAD_MIN_CORR ) {
      st->corr_hp_fast = CVAD_MIN_CORR;
   }

   if ( low_power != 0 ) {
      st->corr_hp_fast = CVAD_MIN_CORR;
   }
}
#endif

/*
 * complex_vad
 *
 *
 * Parameters:
 *    st->complex_high        B: 1 if (corr_hp_fast > CVAD_THRESH_ADAPT_HIGH)
 *    st->complex_low         B: 1 if (corr_hp_fast > CVAD_THRESH_ADAPT_LOW)
 *    low_power               I: flag power of the input frame
 *    st->best_corr_hp        I: complex background detector
 *    st->corr_hp_fast        B: long term complex signal estimate
 *    st->complex_hang_timer  B: complex hang timer
 *
 *
 * Function:
 *    Complex background decision
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static Word32 complex_vad( vadState *st, Word16 low_power )
{
   st->complex_high = st->complex_high >> 1;
   st->complex_low = st->complex_low >> 1;

   if ( low_power == 0 ) {
      if ( st->corr_hp_fast > CVAD_THRESH_ADAPT_HIGH ) {
         st->complex_high = st->complex_high | 0x00004000;
      }

      if ( st->corr_hp_fast > CVAD_THRESH_ADAPT_LOW ) {
         st->complex_low = st->complex_low | 0x00004000;
      }
   }

   if ( st->corr_hp_fast > CVAD_THRESH_HANG ) {
      st->complex_hang_timer += 1;
   }
   else {
      st->complex_hang_timer = 0;
   }
   return( Word16 )( ( ( st->complex_high & 0x00007f80 ) == 0x00007f80 ) || ( (
         st->complex_low & 0x00007fff ) == 0x00007fff ) );
}
#endif

/*
 * complex_vad
 *
 *
 * Parameters:
 *    st->complex_warning  I: flags for complex detection
 *    st->stat_count       B: stationary counter
 *    st->pitch            I: flags for pitch detection
 *    st->tone             I: flags indicating presence of a tone
 *    st->vadreg           I: intermediate VAD flags
 *    level                I: sub-band levels of the input frame
 *    st->ave_level        B: Average amplitude estimate
 *
 * Function:
 *    Control update of the background noise estimate
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void update_cntrl( vadState *st, Float32 level[] )
{
   Float32 stat_rat, num, denom;
   Float32 alpha;
   Word32 i;


   /*
    * handle highband complex signal input  separately
    * if ther has been highband correlation for some time
    * make sure that the VAD update speed is low for a while
    */
   if ( st->complex_warning != 0 ) {
      if ( st->stat_count < CAD_MIN_STAT_COUNT ) {
         st->stat_count = CAD_MIN_STAT_COUNT;
      }
   }

   /*
    * NB stat_count is allowed to be decreased by one below again
    * deadlock in speech is not possible unless the signal is very
    * complex and need a high rate
    * if fullband pitch or tone have been detected for a while, initialize stat_count
    */
   if ( ( ( st->pitch & 0x6000 ) == 0x6000 ) || ( ( st->tone & 0x00007c00 ) ==
         0x7c00 ) ) {
      st->stat_count = STAT_COUNT;
   }
   else {
      /* if 8 last vad-decisions have been "0", reinitialize stat_count */
      if ( ( st->vadreg & 0x7f80 ) == 0 ) {
         st->stat_count = STAT_COUNT;
      }
      else {
         stat_rat = 0;

         for ( i = 0; i < COMPLEN; i++ ) {
            if ( level[i] > st->ave_level[i] ) {
               num = level[i];
               denom = st->ave_level[i];
            }
            else {
               num = st->ave_level[i];
               denom = level[i];
            }

            /* Limit nimimum value of num and denom to STAT_THR_LEVEL */
            if ( num < STAT_THR_LEVEL ) {
               num = STAT_THR_LEVEL;
            }

            if ( denom < STAT_THR_LEVEL ) {
               denom = STAT_THR_LEVEL;
            }
            stat_rat += num / denom * 64;
         }

         /* compare stat_rat with a threshold and update stat_count */
         if ( stat_rat > STAT_THR ) {
            st->stat_count = STAT_COUNT;
         }
         else {
            if ( ( st->vadreg & 0x4000 ) != 0 ) {
               if ( st->stat_count != 0 ) {
                  st->stat_count -= 1;
               }
            }
         }
      }
   }

   /* Update average amplitude estimate for stationarity estimation */
   alpha = ALPHA4;

   if ( st->stat_count == STAT_COUNT ) {
      alpha = 1.0F;
   }
   else if ( ( st->vadreg & 0x4000 ) == 0 ) {
      alpha = ALPHA5;
   }

   for ( i = 0; i < COMPLEN; i++ ) {
      st->ave_level[i] += alpha * ( level[i] - st->ave_level[i] );
   }
}
#endif

/*
 * noise_estimate_update
 *
 *
 * Parameters:
 *    st                      B: State struct
 *    level                   I: sub-band levels of the input frame
 *    st->vadreg              I: intermediate VAD flags
 *    st->pitch               I: flags for pitch detection
 *    st->complex_hang_count  I: signal is too complex for VAD
 *    st->stat_count          B: stationary counter
 *    st->old_level           B: signal levels of the previous frame
 *    st->bckr_est            B: noise estimate
 *
 * Function:
 *    Update of background noise estimate
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void noise_estimate_update( vadState *st, Float32 level[] )
{
   Float32 alpha_up, alpha_down, bckr_add;
   Word32 i;


   /* Control update of bckr_est[] */
   update_cntrl( st, level );

   /* Choose update speed */
   bckr_add = 2;

   if ( ( ( 0x7800 & st->vadreg ) == 0 ) && ( ( st->pitch & 0x7800 ) == 0 ) && (
         st->complex_hang_count == 0 ) ) {
      alpha_up = ALPHA_UP1;
      alpha_down = ALPHA_DOWN1;
   }
   else {
      if ( ( st->stat_count == 0 ) && ( st->complex_hang_count == 0 ) ) {
         alpha_up = ALPHA_UP2;
         alpha_down = ALPHA_DOWN2;
      }
      else {
         alpha_up = 0;
         alpha_down = ALPHA3;
         bckr_add = 0;
      }
   }

   /* Update noise estimate (bckr_est) */
   for ( i = 0; i < COMPLEN; i++ ) {
      Float32 temp;


      temp = st->old_level[i] - st->bckr_est[i];

      /* update downwards*/
      if ( temp < 0 ) {
         st->bckr_est[i] = ( -2 + ( st->bckr_est[i] + ( alpha_down * temp ) ) );

         /* limit minimum value of the noise estimate to NOISE_MIN */
         if ( st->bckr_est[i] < NOISE_MIN ) {
            st->bckr_est[i] = NOISE_MIN;
         }
      }

      /* update upwards */
      else {
         st->bckr_est[i] = ( bckr_add + ( st->bckr_est[i] + ( alpha_up * temp )
               ) );

         /* limit maximum value of the noise estimate to NOISE_MAX */
         if ( st->bckr_est[i] > NOISE_MAX ) {
            st->bckr_est[i] = NOISE_MAX;
         }
      }
   }

   /* Update signal levels of the previous frame (old_level) */
   for ( i = 0; i < COMPLEN; i++ ) {
      st->old_level[i] = level[i];
   }
}
#endif

/*
 * hangover_addition
 *
 *
 * Parameters:
 *    noise_level             I: average level of the noise estimates
 *    low_power               I: flag power of the input frame
 *    st->burst_count         O: counter for the length of speech bursts
 *    st->hang_count          O: hangover counter
 *    st->complex_hang_count  B: signal is too complex for VAD
 *    st->complex_hang_timer  B: complex hang timer
 *    st->vadreg              I: intermediate VAD flags
 *    st->corr_hp_fast        I: long term complex signal estimate
 *
 * Function:
 *    Add hangover for complex signal or after speech bursts
 *
 * Returns:
 *    VAD_flag indicating final VAD decision
 */
#ifndef VAD2
static Word16 hangover_addition( vadState *st, Float32 noise_level, Word16
      low_power )
{
   Word16 hang_len, burst_len;


   /*
    * Calculate burst_len and hang_len
    * burst_len: number of consecutive intermediate vad flags with "1"-decision
    * required for hangover addition
    * hang_len:  length of the hangover
    */
   if ( noise_level > HANG_NOISE_THR ) {
      burst_len = BURST_LEN_HIGH_NOISE;
      hang_len = HANG_LEN_HIGH_NOISE;
   }
   else {
      burst_len = BURST_LEN_LOW_NOISE;
      hang_len = HANG_LEN_LOW_NOISE;
   }

   /*
    * if the input power (pow_sum) is lower than a threshold, clear
    * counters and set VAD_flag to "0"  "fast exit"
    */
   if ( low_power != 0 ) {
      st->burst_count = 0;
      st->hang_count = 0;
      st->complex_hang_count = 0;
      st->complex_hang_timer = 0;
      return 0;
   }

   if ( st->complex_hang_timer > CVAD_HANG_LIMIT ) {
      if ( st->complex_hang_count < CVAD_HANG_LENGTH ) {
         st->complex_hang_count = CVAD_HANG_LENGTH;
      }
   }

   /* long time very complex signal override VAD output function */
   if ( st->complex_hang_count != 0 ) {
      st->burst_count = BURST_LEN_HIGH_NOISE;
      st->complex_hang_count -= 1;
      return 1;
   }
   else {
      /* let hp_corr work in from a noise_period indicated by the VAD */
      if ( ( ( st->vadreg & 0x3ff0 ) == 0 ) && ( st->corr_hp_fast >
            CVAD_THRESH_IN_NOISE ) ) {
         return 1;
      }
   }

   /* update the counters (hang_count, burst_count) */
   if ( ( st->vadreg & 0x4000 ) != 0 ) {
      st->burst_count += 1;

      if ( st->burst_count >= burst_len ) {
         st->hang_count = hang_len;
      }
      return 1;
   }
   else {
      st->burst_count = 0;

      if ( st->hang_count > 0 ) {
         st->hang_count -= 1;
         return 1;
      }
   }
   return 0;
}
#endif

/*
 * vad_decision
 *
 *
 * Parameters:
 *    st                      B: State struct
 *    level                   I: sub-band levels of the input frame
 *    pow_sum                 I: power of the input frame
 *    st->bckr_est            I: background noise components
 *    st->vadreg              I: intermediate VAD flags
 *    st->complex_warning     O: flags for complex detection
 *    st->speech_vad_decision O: speech VAD flag
 *
 * Function:
 *    Calculates VAD_flag
 *
 * Returns:
 *    VAD_flag indicating final VAD decision
 */
#ifndef VAD2
static Word16 vad_decision( vadState *st, Float32 level[COMPLEN], Float32
      pow_sum )
{
   Float32 snr_sum, temp, vad_thr, noise_level;
   Word32 i;
   Word16 low_power_flag;


   /*
    * Calculate squared sum of the input levels (level)
    * divided by the background noise components (bckr_est).
    */
   snr_sum = 0;

   for ( i = 0; i < COMPLEN; i++ ) {
      temp = level[i] / st->bckr_est[i];
      snr_sum += temp * temp;
   }
   snr_sum = snr_sum * 56.8889F;

   /* Calculate average level of estimated background noise */
   noise_level = st->bckr_est[0] + st->bckr_est[1] + st->bckr_est[2] + st->
         bckr_est[3] + st->bckr_est[4] + st->bckr_est[5] + st->bckr_est[6] + st
         ->bckr_est[7] + st->bckr_est[8];
   noise_level = noise_level * 0.111111F;

   /* Calculate VAD threshold */
   vad_thr = VAD_SLOPE * ( noise_level - VAD_P1 ) + VAD_THR_HIGH;

   if ( vad_thr < VAD_THR_LOW ) {
      vad_thr = VAD_THR_LOW;
   }

   /* Shift VAD decision register */
   st->vadreg >>= 1;

   /* Make intermediate VAD decision */
   if ( snr_sum > vad_thr ) {
      st->vadreg = st->vadreg | 0x4000;
   }

   /*
    * primary vad decision made
    * check if the input power (pow_sum) is lower than a threshold"
    */
   if ( pow_sum < VAD_POW_LOW ) {
      low_power_flag = 1;
   }
   else {
      low_power_flag = 0;
   }

   /*
    * update complex signal estimate st->corr_hp_fast and hangover reset timer using
    * low_power_flag and corr_hp_fast and various adaptation speeds
    */
   complex_estimate_adapt( st, low_power_flag );

   /* check multiple thresholds of the st->corr_hp_fast value */
   st->complex_warning = complex_vad( st, low_power_flag );

   /* Update speech subband vad background noise estimates */
   noise_estimate_update( st, level );

   /*
    *  Add speech and complex hangover and return speech VAD_flag
    *  long term complex hangover may be added
    */
   st->speech_vad_decision = hangover_addition( st, noise_level, low_power_flag
         );
   return( st->speech_vad_decision );
}
#endif

/*
 * level_calculation
 *
 *
 * Parameters:
 *    data              I: signal buffer
 *    sub_level         B: level calculate at the end of the previous frame/
 *                         level of signal calculated from the last
 *                         (count2 - count1) samples
 *    count1            I: number of samples to be counted
 *    count2            I: number of samples to be counted
 *    ind_m             I: step size for the index of the data buffer
 *    ind_a             I: starting index of the data buffer
 *    scale             I: scaling for the level calculation
 *
 * Function:
 *    Calculate signal level in a sub-band.
 *
 *    Level is calculated by summing absolute values of the input data.
 *
 * Returns:
 *    signal level
 */
#ifndef VAD2
static Float32 level_calculation( Float32 data[], Float32 *sub_level, Word16
      count1, Word16 count2, Word16 ind_m, Word16 ind_a, Word16 scale )
{
   Float32 level, temp1;
   Word32 i;


   temp1 = 0;

   for ( i = count1; i < count2; i++ ) {
      temp1 += ( Float32 )fabs( data[ind_m * i + ind_a] );
   }
   level = temp1 + *sub_level;
   *sub_level = temp1;

   for ( i = 0; i < count1; i++ ) {
      level += ( Float32 )fabs( data[ind_m * i + ind_a] );
   }
   return( scale * level );
}
#endif

/*
 * filter3
 *
 *
 * Parameters:
 *    in0               B: input values; output low-pass part
 *    in1               B: input values; output high-pass part
 *    data              B: updated filter memory
 *
 * Function:
 *    Third-order half-band lowpass/highpass filter pair.
 *
 * Returns:
 *   void
 */
#ifndef VAD2
static void filter3( Float32 *in0, Float32 *in1, Float32 *data )
{
   Float32 temp1, temp2;


   temp1 = *in1 - ( COEFF3 * *data );
   temp2 = *data + ( COEFF3 * temp1 );
   *data = temp1;
   *in1 = ( *in0 - temp2 ) * 0.5F;
   *in0 = ( *in0 + temp2 ) * 0.5F;
}
#endif

/*
 * filter5
 *
 *
 * Parameters:
 *    in0               B: input values; output low-pass part
 *    in1               B: input values; output high-pass part
 *    data              B: updated filter memory
 *
 * Function:
 *    Fifth-order half-band lowpass/highpass filter pair.
 *
 * Returns:
 *   void
 */
#ifndef VAD2
static void filter5( Float32 *in0, Float32 *in1, Float32 data[] )
{
   Float32 temp0, temp1, temp2;


   temp0 = *in0 - ( COEFF5_1 * data[0] );
   temp1 = data[0] + ( COEFF5_1 * temp0 );
   data[0] = temp0;
   temp0 = *in1 - ( COEFF5_2 * data[1] );
   temp2 = data[1] + ( COEFF5_2 * temp0 );
   data[1] = temp0;
   *in0 = ( temp1 + temp2 ) * 0.5F;
   *in1 = ( temp1 - temp2 ) * 0.5F;
}
#endif

/*
 * first_filter_stage
 *
 *
 * Parameters:
 *    in                I: input signal
 *    out               O: output values,
 *                         every other output is low-pass part and
 *                         every other output is high-pass part
 *    data              B: updated filter memory
 *
 * Function:
 *    Calculate 5th order half-band lowpass/highpass filter pair
 *
 * Returns:
 *   void
 */
#ifndef VAD2
static void first_filter_stage( Float32 in[], Float32 out[], Float32 data[] )
{
   Float32 temp0, temp1, temp2, temp3;
   Float32 data0, data1;
   Word32 i;


   data0 = data[0];
   data1 = data[1];

   for ( i = 0; i < L_SUBFR; i++ ) {
      temp0 = ( in[4 * i + 0] * 0.25F ) - ( COEFF5_1 * data0 );
      temp1 = data0 + ( COEFF5_1 * temp0 );
      temp3 = ( in[4*i+1]*0.25F )-( COEFF5_2 * data1 );
      temp2 = data1 + ( COEFF5_2 * temp3 );
      out[4 * i + 0] = temp1 + temp2;
      out[4 * i + 1] = temp1 - temp2;
      data0 = ( in[4 * i + 2] * 0.25F ) - ( COEFF5_1 * temp0 );
      temp1 = temp0 + ( COEFF5_1 * data0 );
      data1 = ( in[4 * i + 3] * 0.25F ) - ( COEFF5_2 * temp3 );
      temp2 = temp3 + ( COEFF5_2 * data1 );
      out[4 * i + 2] = temp1 + temp2;
      out[4 * i + 3] = temp1 - temp2;
   }
   data[0] = data0;
   data[1] = data1;
}
#endif

/*
 * filter_bank
 *
 *
 * Parameters:
 *    in                I: input frame
 *    st->a_data5       B: filter memory
 *    st->a_data3       B: filter memory
 *    st->sub_level     B: level memory
 *    level             O: signal levels at each band
 *
 * Function:
 *    Divides input signal into 9-bands and calcultes level of the signal in each band
 *
 * Returns:
 *    void
 */
#ifndef VAD2
static void filter_bank( vadState *st, Float32 in[], Float32 level[] )
{
   Word32 i;
   Float32 tmp_buf[FRAME_LEN];


   /* calculate the filter bank */
   first_filter_stage( in, tmp_buf, st->a_data5[0] );

   for ( i = 0; i < FRAME_LEN / 4; i++ ) {
      filter5( &tmp_buf[4 * i], &tmp_buf[4 * i + 2], st->a_data5[1] );
      filter5( &tmp_buf[4 * i +1], &tmp_buf[4 * i + 3], st->a_data5[2] );
   }

   for ( i = 0; i < FRAME_LEN / 8; i++ ) {
      filter3( &tmp_buf[8 * i + 0], &tmp_buf[8 * i + 4], &st->a_data3[0] );
      filter3( &tmp_buf[8 * i + 2], &tmp_buf[8 * i + 6], &st->a_data3[1] );
      filter3( &tmp_buf[8 * i + 3], &tmp_buf[8 * i + 7], &st->a_data3[4] );
   }

   for ( i = 0; i < FRAME_LEN / 16; i++ ) {
      filter3( &tmp_buf[16 * i + 0], &tmp_buf[16 * i + 8], &st->a_data3[2] );
      filter3( &tmp_buf[16 * i + 4], &tmp_buf[16 * i + 12], &st->a_data3[3] );
   }

   /* calculate levels in each frequency band */
   /* 3000 - 4000 Hz*/
   level[8] = level_calculation( tmp_buf, &st->sub_level[8], FRAME_LEN /4 - 8,
         FRAME_LEN /4, 4, 1, 1 );

   /* 2500 - 3000 Hz*/
   level[7] = level_calculation( tmp_buf, &st->sub_level[7], FRAME_LEN /8 - 4,
         FRAME_LEN /8, 8, 7, 2 );

   /* 2000 - 2500 Hz*/
   level[6] = level_calculation( tmp_buf, &st->sub_level[6], FRAME_LEN /8 - 4,
         FRAME_LEN /8, 8, 3, 2 );

   /* 1500 - 2000 Hz*/
   level[5] = level_calculation( tmp_buf, &st->sub_level[5], FRAME_LEN /8 - 4,
         FRAME_LEN /8, 8, 2, 2 );

   /* 1000 - 1500 Hz*/
   level[4] = level_calculation( tmp_buf, &st->sub_level[4], FRAME_LEN /8 - 4,
         FRAME_LEN /8, 8, 6, 2 );

   /* 750 - 1000 Hz*/
   level[3] = level_calculation( tmp_buf, &st->sub_level[3], FRAME_LEN /16 - 2,
         FRAME_LEN /16, 16, 4, 2 );

   /* 500 - 750 Hz*/
   level[2] = level_calculation( tmp_buf, &st->sub_level[2], FRAME_LEN /16 - 2,
         FRAME_LEN /16, 16, 12, 2 );

   /* 250 - 500 Hz*/
   level[1] = level_calculation( tmp_buf, &st->sub_level[1], FRAME_LEN /16 - 2,
         FRAME_LEN /16, 16, 8, 2 );

   /* 0 - 250 Hz*/
   level[0] = level_calculation( tmp_buf, &st->sub_level[0], FRAME_LEN /16 - 2,
         FRAME_LEN /16, 16, 0, 2 );
}
#endif

/*
 * vad
 *
 *
 * Parameters:
 *    in_buf            I: samples of the input frame
 *    st                B: State struct
 *    st->pitch         B: flags for pitch detection
 *    st->complex_low   B: complex flag
 *
 * Function:
 *    Voice Activity Detection (VAD)
 *
 * Returns:
 *    VAD Decision, 1 = speech, 0 = noise
 */
#ifndef VAD2
static Word16 vad( vadState *st, Float32 in_buf[] )
{
   Float32 level[COMPLEN];
   Float32 pow_sum;
   Word32 i;


   /* Calculate power of the input frame. */
   pow_sum = 0L;

   for ( i = -40; i < 120; i += 8 ) {
      pow_sum += in_buf[i] * in_buf[i];
      pow_sum += in_buf[i + 1] *in_buf[i + 1];
      pow_sum += in_buf[i + 2] * in_buf[i + 2];
      pow_sum += in_buf[i + 3] * in_buf[i + 3];
      pow_sum += in_buf[i + 4] * in_buf[i + 4];
      pow_sum += in_buf[i + 5] * in_buf[i + 5];
      pow_sum += in_buf[i + 6] * in_buf[i + 6];
      pow_sum += in_buf[i + 7] * in_buf[i + 7];
   }

   /*
    * If input power is very low, clear pitch flag of the current frame
    */
   if ( pow_sum < POW_PITCH_THR ) {
      st->pitch = ( Word16 )( st->pitch & 0x3fff );
   }

   /*
    * If input power is very low, clear complex flag of the "current" frame
    */
   if ( pow_sum < POW_COMPLEX_THR ) {
      st->complex_low = ( Word16 )( st->complex_low & 0x3fff );
   }

   /*
    * Run the filter bank which calculates signal levels at each band
    */
   filter_bank( st, in_buf, level );
   return( vad_decision( st, level, pow_sum ) );
}
#endif

/*
 * vad_pitch_detection
 *
 *
 * Parameters:
 *    st->oldlag        B: old LTP lag
 *    T_op              I: speech encoder open loop lags
 *    st->pitch         B: flags for pitch detection
 *    st                B: State struct
 *    st->pitch         B: flags for pitch detection
 *    st->oldlag_count  B: lag count
 *
 * Function:
 *    Test if signal contains pitch or other periodic component.
 *
 * Returns:
 *    Boolean voiced / unvoiced decision in state variable
 */
#ifndef VAD2
static void vad_pitch_detection( vadState *st, Word32 T_op[] )
{
   Word32 lagcount, i;


   lagcount = 0;

   for ( i = 0; i < 2; i++ ) {
      if ( abs( st->oldlag - T_op[i] ) < LTHRESH ) {
         lagcount += 1;
      }

      /* Save the current LTP lag */
      st->oldlag = T_op[i];
   }

   /*
    * Make pitch decision.
    * Save flag of the pitch detection to the variable pitch.
    */
   st->pitch = st->pitch >> 1;

   if ( ( st->oldlag_count + lagcount ) >= NTHRESH ) {
      st->pitch = st->pitch | 0x4000;
   }

   /* Update oldlagcount */
   st->oldlag_count = lagcount;
}
#endif


#ifdef VAD2

/***************************************************************************
 *
 *   FUNCTION NAME: vad2()
 *
 *   PURPOSE:
 *     This function provides the Voice Activity Detection function option 2
 *     for the Adaptive Multi-rate (AMR) codec.
 *
 *   INPUTS:
 *
 *     vadState
 *                     pointer to vadState state structure
 *     farray_ptr
 *                     pointer to Float32[80] input array
 *
 *   OUTPUTS:
 *
 *     state variables in vadState are updated
 *
 *   RETURN VALUE:
 *
 *     Word16
 *                     VAD(m) - two successive calls to vad2() yield
 *                     the VAD decision for the 20 ms frame:
 *                     VAD_flag = VAD(m-1) || VAD(m)
 *
 *
 *************************************************************************/

int		vad2 (vadState *st, Float32 *farray_ptr)
{

  /* Static variables */

  /* The channel table is defined below.  In this table, the
     lower and higher frequency coefficients for each of the 16
     channels are specified.  The table excludes the coefficients
     with numbers 0 (DC), 1, and 64 (Foldover frequency).  For
     these coefficients, the gain is always set at 1.0 (0 dB). */

  static int	ch_tbl [NUM_CHAN][2] = {

    { 2,  3},
    { 4,  5},
    { 6,  7},
    { 8,  9},
    {10, 11},
    {12, 13},
    {14, 16},
    {17, 19},
    {20, 22},
    {23, 26},
    {27, 30},
    {31, 35},
    {36, 41},
    {42, 48},
    {49, 55},
    {56, 63}

  };

  /* The voice metric table is defined below.  It is a non-
     linear table with a deadband near zero.  It maps the SNR
     index (quantized SNR value) to a number that is a measure
     of voice quality. */

  static int	vm_tbl [90] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7,
    8, 8, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15,
    15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 23, 24,
    24, 25, 26, 27, 28, 28, 29, 30, 31, 32, 33, 34,
    35, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45,
    46, 47, 48, 49, 50, 50, 50, 50, 50, 50, 50, 50,
    50, 50
  };

  /* hangover as a function of peak SNR (3 dB steps) */
  static Word16 hangover_table[20] =
  {
    30, 30, 30, 30, 30, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 8, 8, 8
  };

  /* burst sensitivity as a function of peak SNR (3 dB steps) */
  static Word16 burstcount_table[20] =
  {
    8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4
  };

  /* voice metric sensitivity as a function of peak SNR (3 dB steps) */
  static Word16 vm_threshold_table[20] =
  {
    34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 40, 51, 71, 100, 139, 191, 257, 337, 432
  };


  /* Automatic variables */

  float		data_buffer [FFT_LEN1], enrg, snr;
  float		tne, tce, ftmp;
  int		ch_snr [NUM_CHAN];
  int		i, j, j1, j2;
  int		vm_sum;
  int		update_flag;

  float		ch_enrg_dev;		/* for forced update... */
  float		ch_enrg_db [NUM_CHAN];
  float		alpha;


  /* For detecting sine waves */
  float		peak, avg, peak2avg;
  int		sine_wave_flag;

  /* For computing frame SNR and long-term SNR */
  float		tce_db, tne_db;
  float		xt;

  /* More VAD stuff */
  int	tsnrq;
  int	ivad;


  /* Functions */

  void real_fft (float *, int);


  /****** Executable code starts here ******/

  /* Increment frame counter */
  st->Lframe_cnt++;

  /* Preemphasize the input data and store in the data buffer with
     appropriate delay */

  for (i = 0; i < DELAY0; i++)
    data_buffer [i] = 0.0;

  data_buffer [DELAY0] = *farray_ptr + PRE_EMP_FAC1 * st->pre_emp_mem;

  for (i = DELAY0+1, j = 1; i < DELAY0+FRM_LEN1; i++, j++)
    data_buffer [i] = *(farray_ptr + j) + PRE_EMP_FAC1 *
      *(farray_ptr + j - 1);

  st->pre_emp_mem = *(farray_ptr + FRM_LEN1 - 1);

  for (i = DELAY0+FRM_LEN1; i < FFT_LEN1; i++)
    data_buffer [i] = 0.0;

  /* Perform FFT on the data buffer */
  real_fft (data_buffer, +1);

  /* Estimate the energy in each channel */
  alpha = (st->Lframe_cnt == 1) ? 1.0 : CEE_SM_FAC1;
  for (i = LO_CHAN; i <= HI_CHAN; i++)
    {
      enrg = 0.0;
      j1 = ch_tbl [i][0], j2 = ch_tbl [i][1];
      for (j = j1; j <= j2; j++)
	enrg += square(data_buffer [2*j]) + square(data_buffer [2*j+1]);
      enrg /= (float) (j2 - j1 + 1);
      st->ch_enrg [i] = (1 - alpha) * st->ch_enrg [i] + alpha * enrg;
      if (st->ch_enrg [i] < MIN_CHAN_ENRG) st->ch_enrg [i] = MIN_CHAN_ENRG;
    }

  /* Compute the total channel energy estimate (tce) */
  tce = 0.0;
  for (i = LO_CHAN; i <= HI_CHAN; i++)
    tce += st->ch_enrg [i];

  /* Calculate spectral peak-to-average ratio */
  peak = avg = 0.;
  for (i = LO_CHAN; i <= HI_CHAN; i++) {
    /* Sine waves not valid for low frequencies: */
    if (i >= SINE_START_CHAN && st->ch_enrg [i] > peak)
      peak = st->ch_enrg [i];
    avg += st->ch_enrg [i];
  }
  avg /= HI_CHAN - LO_CHAN + 1;
  peak2avg = (avg < 1./NORM_ENRG) ? 0. : 10.*log10 (peak/avg);

  /* Detect sine waves */
  if (peak2avg > 10.)
    sine_wave_flag = TRUE;
  else
    sine_wave_flag = FALSE;

  /* Initialize channel noise estimate to channel energy of first few frames
     (if sufficiently low Peak-to-Average ratio) */
  if (st->Lframe_cnt <= INIT_FRAMES) {
    if (sine_wave_flag == TRUE) {
      for (i = LO_CHAN; i <= HI_CHAN; i++)
        st->ch_noise [i] = INE;
    }
    else {
      for (i = LO_CHAN; i <= HI_CHAN; i++)
        st->ch_noise [i] = max(st->ch_enrg [i], INE);
    }
  }

  /* Compute the channel SNR indices */
  for (i = LO_CHAN; i <= HI_CHAN; i++) {
    snr = 10.0 * log10 ((double)st->ch_enrg [i] / st->ch_noise [i]);
    if (snr < 0.0) snr = 0.0;
    ch_snr [i] = (snr + 0.1875) / 0.375;
  }

  /* Compute the sum of voice metrics */
  vm_sum = 0;
  for (i = LO_CHAN; i <= HI_CHAN; i++) {
    j = min(ch_snr[i],89);
    vm_sum += vm_tbl [j];
  }

  /* Initialize voice energy to nominal value */
  if (st->Lframe_cnt <= INIT_FRAMES  || st->fupdate_flag == TRUE ) {
#if NORM_ENERG==4
    tce_db = 49.918;
#elif NORM_ENERG==1
    tce_db = 55.938;
#else
    tce_db = (96. - 22. - 10*log10 (FFT_LEN1/2) - 10.*log10 (NORM_ENRG));
#endif

    st->negSNRvar = 0.0;
    st->negSNRbias = 0.0;

    /* Compute the total noise energy estimate (tne) */
    tne = 0.0;
    for (i = LO_CHAN; i <= HI_CHAN; i++)
      tne += st->ch_noise [i];

    /* Get total noise in dB */
    tne_db = 10 * log10 (tne);

    /* Initialise instantaneous and long-term peak signal-to-noise ratios */
    xt = tce_db - tne_db;
    st->tsnr = xt;

  }
  else {

    /* Calculate instantaneous signal-to-noise ratio */
    xt = 0;
    for (i=LO_CHAN; i<=HI_CHAN; i++)
      xt += st->ch_enrg[i]/st->ch_noise[i];
    xt = 10*log10(xt/NUM_CHAN);

    /* Estimate long-term "peak" SNR */
    if (xt > st->tsnr)
      st->tsnr = 0.9*st->tsnr + 0.1*xt;
    else if (xt > 0.625*st->tsnr)
      st->tsnr = 0.998*st->tsnr + 0.002*xt;
  }

  /* Quantize the long-term SNR in 3 dB steps */
  tsnrq = (int)(st->tsnr/3.);
  tsnrq = min(19, max(0, tsnrq));

  /* Calculate the negative SNR sensitivity bias */
  if (xt < 0) {
    st->negSNRvar = min (0.99*st->negSNRvar + 0.01*xt*xt, 4.0);
    st->negSNRbias = max (12.0*(st->negSNRvar - 0.65), 0.0);
  }

  /* Determine VAD as a function of the voice metric sum and quantized SNR */
  if (vm_sum > vm_threshold_table[tsnrq] + st->negSNRbias) {
    ivad = 1;
    if (++st->burstcount > burstcount_table[tsnrq]) {
      st->hangover = hangover_table[tsnrq];
    }
  } else {
    st->burstcount = 0;
    if (--st->hangover <= 0) {
      ivad = 0;
      st->hangover = 0;
    } else {
      ivad = 1;
    }
  }

  /* Calculate log spectral deviation */
  for (i = LO_CHAN; i <= HI_CHAN; i++)
    ch_enrg_db [i] = 10.*log10( st->ch_enrg [i] );

  ch_enrg_dev = 0.;
  if (st->Lframe_cnt == 1)
    for (i = LO_CHAN; i <= HI_CHAN; i++)
      st->ch_enrg_long_db [i] = ch_enrg_db [i];
  else
    for (i = LO_CHAN; i <= HI_CHAN; i++)
      ch_enrg_dev += fabs( st->ch_enrg_long_db [i] - ch_enrg_db [i] );


  /*
   * Calculate long term integration constant as a function of instantaneous SNR
   * (i.e., high SNR (tsnr dB) -> slower integration (alpha = HIGH_ALPHA),
   *         low SNR (0 dB) -> faster integration (alpha = LOW_ALPHA)
   */

  /* alpha = HIGH_ALPHA - ALPHA_RANGE * (tsnr - xt) / tsnr, low <= alpha <= high */
  ftmp = st->tsnr - xt;
  if (ftmp <= 0.0 || st->tsnr <= 0.0)
    alpha = HIGH_ALPHA1;
  else if (ftmp > st->tsnr)
    alpha = LOW_ALPHA1;
  else
    alpha = HIGH_ALPHA1 - (ALPHA_RANGE1 * ftmp / st->tsnr);

  /* Calc long term log spectral energy */
  for (i = LO_CHAN; i <= HI_CHAN; i++) {
    st->ch_enrg_long_db[i] = alpha*st->ch_enrg_long_db[i] + (1.-alpha)*ch_enrg_db[i];
  }

  /* Set or reset the update flag */
  update_flag = FALSE;
  st->fupdate_flag = FALSE;
  if ((vm_sum <= UPDATE_THLD) ||
      (st->Lframe_cnt <= INIT_FRAMES && sine_wave_flag == FALSE)) {
    update_flag = TRUE;
    st->update_cnt = 0;
  }
  else if (tce > NOISE_FLOOR && ch_enrg_dev < DEV_THLD1 &&
           sine_wave_flag == FALSE && st->LTP_flag == FALSE) {
    st->update_cnt++;
    if (st->update_cnt >= UPDATE_CNT_THLD1) {
      update_flag = TRUE;
      st->fupdate_flag = TRUE;
    }
  }

  if ( st->update_cnt == st->last_update_cnt )
    st->hyster_cnt++;
  else
    st->hyster_cnt = 0;
  st->last_update_cnt = st->update_cnt;

  if ( st->hyster_cnt > HYSTER_CNT_THLD1 )
    st->update_cnt = 0;

  /* Update the channel noise estimates */
  if (update_flag == TRUE) {
    for (i = LO_CHAN; i <= HI_CHAN; i++) {
      st->ch_noise [i] = (1.0 - CNE_SM_FAC1) * st->ch_noise [i] +
	CNE_SM_FAC1 * st->ch_enrg [i];
      if (st->ch_noise [i] < MIN_CHAN_ENRG) st->ch_noise [i] = MIN_CHAN_ENRG;
    }
  }

  return (ivad);

}		/* end vad2 () */


/**************************************************************************
 *
 *   FUNCTION NAME: real_fft()
 *
 *   PURPOSE: FFT/IFFT function for real sequences
 *
 **************************************************************************
 *
 * This is an implementation of decimation-in-time FFT algorithm for
 * real sequences.  The techniques used here can be found in several
 * books, e.g., i) Proakis and Manolakis, "Digital Signal Processing",
 * 2nd Edition, Chapter 9, and ii) W.H. Press et. al., "Numerical
 * Recipes in C", 2nd Ediiton, Chapter 12.
 *
 * Input -  There is two inputs to this function:
 *
 *	1) A float pointer to the input data array,
 *	2) A control parameter (isign) specifying forward (+1) or
 *         inverse (-1) FFT.
 *
 * Output - There is no return value.
 *	The input data are replaced with transformed data.  If the
 *	input is a real time domain sequence, it is replaced with
 *	the complex FFT for positive frequencies.  The FFT value
 *	for DC and the foldover frequency are combined to form the
 *	first complex number in the array.  The remaining complex
 *	numbers correspond to increasing frequencies.  If the input
 *	is a complex frequency domain sequence arranged	as above,
 *	it is replaced with the corresponding time domain sequence.
 *
 * Notes:
 *
 *	1) This function is designed to be a part of a VAD
 *	   algorithm that requires 128-point FFT of real
 *	   sequences.  This is achieved here through a 64-point
 *	   complex FFT.  Consequently, the FFT size information is
 *	   not transmitted explicitly.  However, some flexibility
 *	   is provided in the function to change the size of the
 *	   FFT by specifying the size information through "define"
 *	   statements.
 *
 *	2) The values of the complex sinusoids used in the FFT
 *	   algorithm are stored in a ROM table.
 *
 *	3) In the c_fft function, the FFT values are divided by
 *	   2 after each stage of computation thus dividing the
 *	   final FFT values by 64.  This is somewhat different
 *         from the usual definition of FFT where the factor 1/N,
 *         i.e., 1/64, used for the IFFT and not the FFT.  No factor
 *         is used in the r_fft function.
 *
 *************************************************************************/

static double	phs_tbl [SIZE];		/* holds the complex sinusoids */

void		real_fft (float *farray_ptr, int isign)
{

  float		ftmp1_real, ftmp1_imag, ftmp2_real, ftmp2_imag;
  int		i, j;
  static int	first = TRUE;

  void		cmplx_fft (float *, int);
  void		fill_tbl ();

  /* If this is the first call to the function, fill up the
     phase table  */
  if (first == TRUE) {
    fill_tbl ();
    first = FALSE;
  }

  /* The FFT part */
  if (isign == 1) {

    /* Perform the complex FFT */
    cmplx_fft (farray_ptr, isign);

    /* First, handle the DC and foldover frequencies */
    ftmp1_real = *farray_ptr;
    ftmp2_real = *(farray_ptr + 1);
    *farray_ptr = ftmp1_real + ftmp2_real;
    *(farray_ptr + 1) = ftmp1_real - ftmp2_real;

    /* Now, handle the remaining positive frequencies */
    for (i = 2, j = SIZE - i; i <= SIZE_BY_TWO; i = i + 2, j = SIZE - i) {

      ftmp1_real = *(farray_ptr + i) + *(farray_ptr + j);
      ftmp1_imag = *(farray_ptr + i + 1) - *(farray_ptr + j + 1);
      ftmp2_real = *(farray_ptr + i + 1) + *(farray_ptr + j + 1);
      ftmp2_imag = *(farray_ptr + j) - *(farray_ptr + i);

      *(farray_ptr + i) = (ftmp1_real + phs_tbl [i] * ftmp2_real -
			   phs_tbl [i + 1] * ftmp2_imag) / 2.0;
      *(farray_ptr + i + 1) = (ftmp1_imag + phs_tbl [i] * ftmp2_imag +
			       phs_tbl [i + 1] * ftmp2_real) / 2.0;
      *(farray_ptr + j) = (ftmp1_real + phs_tbl [j] * ftmp2_real +
			   phs_tbl [j + 1] * ftmp2_imag) / 2.0;
      *(farray_ptr + j + 1) = (-ftmp1_imag - phs_tbl [j] * ftmp2_imag +
			       phs_tbl [j + 1] * ftmp2_real) / 2.0;
    }
  }

  /* The IFFT part */
  else {

    /* First, handle the DC and foldover frequencies */

    ftmp1_real = *farray_ptr;
    ftmp2_real = *(farray_ptr + 1);
    *farray_ptr = (ftmp1_real + ftmp2_real) / 2.0;
    *(farray_ptr + 1) = (ftmp1_real - ftmp2_real) / 2.0;

    /* Now, handle the remaining positive frequencies */

    for (i = 2, j = SIZE - i; i <= SIZE_BY_TWO; i = i + 2, j = SIZE - i) {

      ftmp1_real = *(farray_ptr + i) + *(farray_ptr + j);
      ftmp1_imag = *(farray_ptr + i + 1) - *(farray_ptr + j + 1);
      ftmp2_real = -(*(farray_ptr + i + 1) + *(farray_ptr + j + 1));
      ftmp2_imag = -(*(farray_ptr + j) - *(farray_ptr + i));

      *(farray_ptr + i) = (ftmp1_real + phs_tbl [i] * ftmp2_real +
			   phs_tbl [i + 1] * ftmp2_imag) / 2.0;
      *(farray_ptr + i + 1) = (ftmp1_imag + phs_tbl [i] * ftmp2_imag -
			       phs_tbl [i + 1] * ftmp2_real) / 2.0;
      *(farray_ptr + j) = (ftmp1_real + phs_tbl [j] * ftmp2_real -
			   phs_tbl [j + 1] * ftmp2_imag) / 2.0;
      *(farray_ptr + j + 1) = (-ftmp1_imag - phs_tbl [j] * ftmp2_imag -
			       phs_tbl [j + 1] * ftmp2_real) / 2.0;
    }

    /* Perform the complex IFFT */
    cmplx_fft (farray_ptr, isign);
  }

  return;
}		/* end real_fft () */



/*
 * FFT/IFFT function for complex sequences
 *
 * The decimation-in-time complex FFT/IFFT is implemented below.
 * The input complex numbers are presented as real part followed by
 * imaginary part for each sample.  The counters are therefore
 * incremented by two to access the complex valued samples.
 */
void		cmplx_fft (float *farray_ptr, int isign)
{
  int		i, j, k, ii, jj, kk, ji, kj;
  float		ftmp, ftmp_real, ftmp_imag;

  /* Rearrange the input array in bit reversed order */
  for (i = 0, j = 0; i < SIZE-2; i = i + 2) {
    if (j > i) {
      ftmp = *(farray_ptr+i);
      *(farray_ptr+i) = *(farray_ptr+j);
      *(farray_ptr+j) = ftmp;

      ftmp = *(farray_ptr+i+1);
      *(farray_ptr+i+1) = *(farray_ptr+j+1);
      *(farray_ptr+j+1) = ftmp;
    }
    k = SIZE_BY_TWO;
    while (j >= k) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

  /* The FFT part */
  if (isign == 1) {
    for (i = 0; i < NUM_STAGE; i++) {		/* i is stage counter */
      jj = (2 << i);				/* FFT size */
      kk = (jj << 1);				/* 2 * FFT size */
      ii = SIZE / jj;				/* 2 * number of FFT's */
      for (j = 0; j < jj; j = j + 2) {		/* j is sample counter */
        ji = j * ii;				/* ji is phase table index */
        for (k = j; k < SIZE; k = k + kk) {	/* k is butterfly top */
          kj = k + jj;				/* kj is butterfly bottom */

	  /* Butterfly computations */
          ftmp_real = *(farray_ptr + kj) * phs_tbl [ji] -
	    *(farray_ptr + kj + 1) * phs_tbl [ji + 1];

          ftmp_imag = *(farray_ptr + kj + 1) * phs_tbl [ji] +
	    *(farray_ptr + kj) * phs_tbl [ji + 1];

          *(farray_ptr + kj) = (*(farray_ptr + k) - ftmp_real) / 2.0;
          *(farray_ptr + kj + 1) = (*(farray_ptr + k + 1) - ftmp_imag) / 2.0;

          *(farray_ptr + k) = (*(farray_ptr + k) + ftmp_real) / 2.0;
          *(farray_ptr + k + 1) = (*(farray_ptr + k + 1) + ftmp_imag) / 2.0;
        }
      }
    }
  }

  /* The IFFT part */
  else {
    for (i = 0; i < NUM_STAGE; i++) {		/* i is stage counter */
      jj = (2 << i);				/* FFT size */
      kk = (jj << 1);				/* 2 * FFT size */
      ii = SIZE / jj;				/* 2 * number of FFT's */
      for (j = 0; j < jj; j = j + 2) {		/* j is sample counter */
        ji = j * ii;				/* ji is phase table index */
        for (k = j; k < SIZE; k = k + kk) {	/* k is butterfly top */
          kj = k + jj;				/* kj is butterfly bottom */

	  /* Butterfly computations */
          ftmp_real = *(farray_ptr + kj) * phs_tbl [ji] +
	    *(farray_ptr + kj + 1) * phs_tbl [ji + 1];

          ftmp_imag = *(farray_ptr + kj + 1) * phs_tbl [ji] -
	    *(farray_ptr + kj) * phs_tbl [ji + 1];

          *(farray_ptr + kj) = *(farray_ptr + k) - ftmp_real;
          *(farray_ptr + kj + 1) = *(farray_ptr + k + 1) - ftmp_imag;

          *(farray_ptr + k) = *(farray_ptr + k) + ftmp_real;
          *(farray_ptr + k + 1) = *(farray_ptr + k + 1) + ftmp_imag;
        }
      }
    }
  }
  return;
}		/* end of cmplx_fft () */


/* Function to fill the phase table values
 */

void		fill_tbl ()
{
  int		i;
  double	delta_f, theta;

  delta_f = - PI / (double) SIZE_BY_TWO;
  for (i = 0; i < SIZE_BY_TWO; i++) {
    theta = delta_f * (double) i;
    phs_tbl[2*i] = cos(theta);
    phs_tbl[2*i+1] = sin(theta);
  }
  return;
}		/* end fill_tbl () */


/***************************************************************************
 *
 *   FUNCTION NAME: LTP_flag_update
 *
 *   PURPOSE:
 *     Set LTP_flag if the LTP gain > LTP_THRESHOLD, where the value of
 *     LTP_THRESHOLD depends on the LTP analysis window length.
 *
 *   INPUTS:
 *
 *     mode
 *                     AMR mode
 *     vadState->R0
 *                     LTP energy
 *     vadState->Rmax
 *                     LTP maximum autocorrelation
 *   OUTPUTS:
 *
 *     vadState->LTP_flag
 *                     Set if LTP gain > LTP_THRESHOLD
 *
 *   RETURN VALUE:
 *
 *     none
 *
 *************************************************************************/

void LTP_flag_update (vadState * st, Word16 mode)
{
  Float32 thresh;

  if ((mode == MR475) || (mode == MR515))
    thresh = 0.55;
  else if (mode == MR102)
    thresh = 0.60;
  else
    thresh = 0.65;

  if (st->Rmax  > thresh*st->R0)
    st->LTP_flag = TRUE;
  else
    st->LTP_flag = FALSE;

  return;
}

/***************************************************************************/
#endif

/*
 * cod_amr
 *
 *
 * Parameters:
 *    st          B: state structure
 *    mode        I: encoder mode
 *    new_speech  I: input speech frame, size L_FRAME
 *    st          B: State struct
 *    ana         O: Analysis parameters
 *    used_mode   B: In: -1 forces VAD on, Out:used encoder mode
 *    synth       O: local synthesis, size L_FRAME
 *
 * Function:
 *    GSM adaptive multi rate speech encoder
 *
 * Returns:
 *    void
 */
static void cod_amr( cod_amrState *st, enum Mode mode, Float32 new_speech[],
      Word16 ana[], enum Mode *used_mode, Float32 synth[] )
{
   /* LPC coefficients */
   Float32 A_t[( MP1 ) * 4];   /* A(z) unquantized for the 4 subframes */
   Float32 Aq_t[( MP1 ) * 4];   /* A(z)   quantized for the 4 subframes */
   Float32 *A, *Aq;   /* Pointer on Aq_t */
   Float32 lsp_new[M];


   /* Other vectors */
   Float32 xn[L_SUBFR];   /* Target vector for pitch search */
   Float32 xn2[L_SUBFR];   /* Target vector for codebook search */
   Float32 code[L_SUBFR];   /* Fixed codebook excitation */
   Float32 y1[L_SUBFR];   /* Filtered adaptive excitation */
   Float32 y2[L_SUBFR];   /* Filtered fixed codebook excitation */
   Float32 gCoeff[3];   /* Correlations between xn, y1, & y2: */
   Float32 res[L_SUBFR];   /* Short term (LPC) prediction residual */
   Float32 res2[L_SUBFR];   /* Long term (LTP) prediction residual */


   /* Vector and scalars needed for the MR475 */
   Float32 xn_sf0[L_SUBFR];   /* Target vector for pitch search */
   Float32 y2_sf0[L_SUBFR];   /* Filtered codebook innovation */
   Float32 code_sf0[L_SUBFR];   /* Fixed codebook excitation */
   Float32 h1_sf0[L_SUBFR];   /* The impulse response of sf0 */
   Float32 mem_syn_save[M];   /* Filter memory */
   Float32 mem_w0_save[M];   /* Filter memory */
   Float32 mem_err_save[M];   /* Filter memory */
   Float32 sharp_save = 0;   /* Sharpening */
   Float32 gain_pit_sf0;   /* Quantized pitch gain for sf0 */
   Float32 gain_code_sf0;   /* Quantized codebook gain for sf0 */
   Word16 i_subfr_sf0 = 0;   /* Position in exc[] for sf0 */


   /* Scalars & Flags */
   Float32 gain_pit, gain_code;
   Float32 gp_limit;   /* pitch gain limit value */
   Word32 T0_sf0 = 0;   /* Integer pitch lag of sf0 */
   Word32 T0_frac_sf0 = 0;   /* Fractional pitch lag of sf0 */
   Word32 T0, T0_frac;
   Word32 T_op[2];
   Word32 evenSubfr;
   Word32 i;
   Word16 i_subfr, subfrNr;
   Word16 lsp_flag = 0;   /* indicates resonance in LPC filter */
   Word16 compute_sid_flag;
   Word16 vad_flag;


   memcpy( st->new_speech, new_speech, L_FRAME <<2 );

   if ( st->dtx ) {
#ifdef VAD2
     /* Find VAD decision (option 2) */
     vad_flag = vad2 (st->vadSt, st->new_speech);
     vad_flag = vad2 (st->vadSt, st->new_speech+80) || vad_flag;
#else
      /* Find VAD decision (option 1) */
      vad_flag = vad( st->vadSt, st->new_speech );
#endif
      /* force VAD on   */
      if ( *used_mode < 0 )
         vad_flag = 1;
      *used_mode = mode;

      /* NB! used_mode may change here */
      compute_sid_flag = tx_dtx_handler( vad_flag, &st->dtxEncSt->
            decAnaElapsedCount, &st->dtxEncSt->dtxHangoverCount, used_mode );
   }
   else {
      compute_sid_flag = 0;
      *used_mode = mode;
   }

   /*
    * Perform LPC analysis:
    * Autocorrelation + Lag windowing.
    * Levinson-durbin algorithm to find a[].
    * Convert a[] to lsp[].
    * Quantize and code the LSPs.
    * find the interpolated LSPs and convert to a[] for all
    * subframes (both quantized and unquantized).
    */
   /* LP analysis */
   lpc( st->lpcSt->LevinsonSt->old_A, st->p_window, st->p_window_12k2, A_t, mode
         );

   /*
    * The LP filter coefficients, are converted to
    * the line spectral pair (LSP) representation for
    * quantization and interpolation purposes.
    */
   lsp( mode, *used_mode, st->lspSt->lsp_old, st->lspSt->lsp_old_q, st->lspSt->
         qSt->past_rq, A_t, Aq_t, lsp_new, &ana );

   /* Buffer lsp's and energy */
   dtx_buffer( &st->dtxEncSt->hist_ptr, st->dtxEncSt->lsp_hist, lsp_new, st->
         new_speech, st->dtxEncSt->log_en_hist );

   if ( *used_mode == MRDTX ) {
      dtx_enc( &st->dtxEncSt->log_en_index, st->dtxEncSt->log_en_hist, st->
            dtxEncSt->lsp_hist, st->dtxEncSt->lsp_index, &st->dtxEncSt->
            init_lsf_vq_index, compute_sid_flag, &st->lspSt->qSt->past_rq[0], st
            ->gainQuantSt->gc_predSt->past_qua_en, &ana );
      memset( st->old_exc, 0, ( PIT_MAX + L_INTERPOL )<<2 );
      memset( st->mem_w0, 0, M <<2 );
      memset( st->mem_err, 0, M <<2 );
      memset( st->zero, 0, L_SUBFR <<2 );
      memset( st->hvec, 0, L_SUBFR <<2 );
      memset( st->lspSt->qSt->past_rq, 0, M <<2 );
      memcpy( st->lspSt->lsp_old, lsp_new, M <<2 );
      memcpy( st->lspSt->lsp_old_q, lsp_new, M <<2 );

      /* Reset clLtp states */
      st->clLtpSt->pitchSt->T0_prev_subframe = 0;
      st->sharp = 0;
   }
   else {
      /* check resonance in the filter */
      lsp_flag = check_lsp( &st->tonStabSt->count, st->lspSt->lsp_old );
   }

#ifdef VAD2
   if (st->dtx) {
      st->vadSt->Rmax = 0.0;
      st->vadSt->R0 = 0.0;
   }
#endif

   for ( subfrNr = 0, i_subfr = 0; subfrNr < 2; subfrNr++, i_subfr +=
         L_FRAME_BY2 ) {
      /*
       * Pre-processing on 80 samples
       * Find the weighted input speech for the whole speech frame
       */
      pre_big( mode, gamma1, gamma1_12k2, gamma2, A_t, i_subfr, st->speech, st->
            mem_w, st->wsp );

      /* Find open loop pitch lag for two subframes */
      if ( ( mode != MR475 ) && ( mode != MR515 ) ) {
         ol_ltp( mode, st->vadSt, &st->wsp[i_subfr], &T_op[subfrNr], st->
               ol_gain_flg, &st->pitchOLWghtSt->old_T0_med, &st->pitchOLWghtSt->
               wght_flg, &st->pitchOLWghtSt->ada_w, st->old_lags, st->dtx,
               subfrNr );
      }
   }

   if ( ( mode == MR475 ) || ( mode == MR515 ) ) {
      /*
       * Find open loop pitch lag for ONE FRAME ONLY
       * search on 160 samples
       */
      ol_ltp( mode, st->vadSt, &st->wsp[0], &T_op[0], st->ol_gain_flg, &st->
            pitchOLWghtSt->old_T0_med, &st->pitchOLWghtSt->wght_flg, &st->
            pitchOLWghtSt->ada_w, st->old_lags, st->dtx, 1 );
      T_op[1] = T_op[0];
   }

#ifdef VAD2
   if (st->dtx) {
      LTP_flag_update(st->vadSt, mode);
   }
#endif

#ifndef VAD2
   if ( st->dtx ) {
      vad_pitch_detection( st->vadSt, T_op );
   }
#endif

   if ( *used_mode == MRDTX ) {
      goto the_end;
   }

   /*
    * Loop for every subframe in the analysis frame
    *
    * To find the pitch and innovation parameters. The subframe size is
    * L_SUBFR and the loop is repeated L_FRAME/L_SUBFR times.
    *     - find the weighted LPC coefficients
    *     - find the LPC residual signal res[]
    *     - compute the target signal for pitch search
    *     - compute impulse response of weighted synthesis filter (h1[])
    *     - find the closed-loop pitch parameters
    *     - encode the pitch dealy
    *     - update the impulse response h1[] by including fixed-gain pitch
    *     - find target vector for codebook search
    *     - codebook search
    *     - encode codebook address
    *     - VQ of pitch and codebook gains
    *     - find synthesis speech
    *     - update states of weighting filter
    */
   /* pointer to interpolated LPC parameters */
   A = A_t;

   /* pointer to interpolated quantized LPC parameters */
   Aq = Aq_t;
   evenSubfr = 0;
   subfrNr = -1;

   for ( i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR ) {
      subfrNr += 1;
      evenSubfr = 1 - evenSubfr;

      if ( ( evenSubfr != 0 ) && ( *used_mode == MR475 ) ) {
         memcpy( mem_syn_save, st->mem_syn, M <<2 );
         memcpy( mem_w0_save, st->mem_w0, M <<2 );
         memcpy( mem_err_save, st->mem_err, M <<2 );
         sharp_save = st->sharp;
      }

      /* Preprocessing of subframe */
      if ( *used_mode != MR475 ) {
         subframePreProc( *used_mode, gamma1, gamma1_12k2, gamma2, A, Aq, &st->
               speech[i_subfr], st->mem_err, st->mem_w0, st->zero, st->ai_zero,
               &st->exc[i_subfr], st->h1, xn, res, st->error );
      }

      /* MR475 */
      else {
         subframePreProc( *used_mode, gamma1, gamma1_12k2, gamma2, A, Aq, &st->
               speech[i_subfr], st->mem_err, mem_w0_save, st->zero, st->ai_zero,
               &st->exc[i_subfr], st->h1, xn, res, st->error );

         if ( evenSubfr != 0 ) {
            memcpy( h1_sf0, st->h1, L_SUBFR <<2 );
         }
      }

      /* copy the LP residual (res2 is modified in the CL LTP search) */
      memcpy( res2, res, L_SUBFR <<2 );

      /* Closed-loop LTP search */
      cl_ltp( &st->clLtpSt->pitchSt->T0_prev_subframe, st->tonStabSt->gp, *
            used_mode, i_subfr, T_op, st->h1, &st->exc[i_subfr], res2, xn,
            lsp_flag, xn2, y1, &T0, &T0_frac, &gain_pit, gCoeff, &ana, &gp_limit
            );

      /* update LTP lag history */
      if ( ( subfrNr == 0 ) && ( st->ol_gain_flg[0] > 0 ) ) {
         st->old_lags[1] = T0;
      }

      if ( ( subfrNr == 3 ) && ( st->ol_gain_flg[1] > 0 ) ) {
         st->old_lags[0] = T0;
      }

      /* Innovative codebook search (find index and gain) */
      cbsearch( *used_mode, subfrNr, xn2, st->h1, T0, st->sharp, gain_pit, code,
            y2, res2, &ana );

      /* Quantization of gains. */
      gainQuant( *used_mode, evenSubfr, st->gainQuantSt->gc_predSt->past_qua_en,
            st->gainQuantSt->gc_predUncSt->past_qua_en, st->gainQuantSt->
            sf0_coeff, &st->gainQuantSt->sf0_target_en, &st->gainQuantSt->
            sf0_gcode0_exp, &st->gainQuantSt->
            sf0_gcode0_fra, &st->gainQuantSt->gain_idx_ptr, &gain_pit_sf0, &
            gain_code_sf0, res, &st->exc[i_subfr], code, xn, xn2, y1, y2, gCoeff
            , gp_limit, &gain_pit, &gain_code, &st->gainQuantSt->adaptSt->
            prev_gc, &st->gainQuantSt->adaptSt->onset, st->gainQuantSt->adaptSt
            ->ltpg_mem, &st->gainQuantSt->adaptSt->prev_alpha, &ana );

      /* update gain history */
      for ( i = 0; i < N_FRAME - 1; i++ ) {
         st->tonStabSt->gp[i] = st->tonStabSt->gp[i + 1];
      }
      st->tonStabSt->gp[N_FRAME - 1] = gain_pit;

      /* Subframe Post Processing */
      if ( *used_mode != MR475 ) {
         subframePostProc( st->speech, i_subfr, gain_pit, gain_code, Aq, synth,
               xn, code, y1, y2, st->mem_syn, st->mem_err, st->mem_w0, st->exc,
               &st->sharp );
      }
      else {
         if ( evenSubfr != 0 ) {
            i_subfr_sf0 = i_subfr;
            memcpy( xn_sf0, xn, L_SUBFR <<2 );
            memcpy( y2_sf0, y2, L_SUBFR <<2 );
            memcpy( code_sf0, code, L_SUBFR <<2 );
            T0_sf0 = T0;
            T0_frac_sf0 = T0_frac;

            /* Subframe Post Porcessing */
            subframePostProc( st->speech, i_subfr, gain_pit, gain_code, Aq,
                  synth, xn, code, y1, y2, mem_syn_save, st->mem_err,
                  mem_w0_save, st->exc, &st->sharp );
            st->sharp = sharp_save;
         }
         else {
            /*
             * update both subframes for the MR475
             * Restore states for the MR475 mode
             */
            memcpy( st->mem_err, mem_err_save, M <<2 );

            /* re-build excitation for sf 0 */
            Pred_lt_3or6( &st->exc[i_subfr_sf0], T0_sf0, T0_frac_sf0, 1 );
            Convolve( &st->exc[i_subfr_sf0], h1_sf0, y1 );
            Aq -= MP1;
            subframePostProc( st->speech, i_subfr_sf0, gain_pit_sf0,
                  gain_code_sf0, Aq, synth, xn_sf0, code_sf0, y1, y2_sf0, st->
                  mem_syn, st->mem_err, st->mem_w0, st->exc, &sharp_save );

            /* overwrites sharp_save */
            Aq += MP1;

            /*
             * re-run pre-processing to get xn right (needed by postproc)
             * (this also reconstructs the unsharpened h1 for sf 1)
             */
            subframePreProc( *used_mode, gamma1, gamma1_12k2, gamma2, A, Aq, &st
                  ->speech[i_subfr], st->mem_err, st->mem_w0, st->zero, st->
                  ai_zero, &st->exc[i_subfr], st->h1, xn, res, st->error );

            /* re-build excitation sf 1 (changed if lag < L_SUBFR) */
            Pred_lt_3or6( &st->exc[i_subfr], T0, T0_frac, 1 );
            Convolve( &st->exc[i_subfr], st->h1, y1 );
            subframePostProc( st->speech, i_subfr, gain_pit, gain_code, Aq,
                  synth, xn, code, y1, y2, st->mem_syn, st->mem_err, st->mem_w0,
                  st->exc, &st->sharp );
         }
      }

      /* interpolated LPC parameters for next subframe */
      A += MP1;
      Aq += MP1;
   }
the_end:

   /* Update signal for next frame. */
   for ( i = 0; i < PIT_MAX; i++ ) {
      st->old_wsp[i] = st->old_wsp[L_FRAME + i];
   }

   for ( i = 0; i < PIT_MAX + L_INTERPOL; i++ ) {
      st->old_exc[i] = st->old_exc[L_FRAME + i];
   }

   for ( i = 0; i < L_TOTAL - L_FRAME; i++ ) {
      st->old_speech[i] = st->old_speech[L_FRAME + i];
   }
}


/*
 * Pre_Process_reset
 *
 *
 * Parameters:
 *    state                O: state structure
 *
 * Function:
 *    Initializes state memory to zero
 *
 * Returns:
 *
 */
static Word32 Pre_Process_reset( Pre_ProcessState *state )
{
   if ( state == ( Pre_ProcessState * )NULL ) {
      fprintf( stderr, "Pre_Process_reset: invalid parameter\n" );
      return-1;
   }
   state->y2 = 0;
   state->y1 = 0;
   state->x0 = 0;
   state->x1 = 0;
   return 0;
}


/*
 * Pre_Process_exit
 *
 *
 * Parameters:
 *    state             I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
static void Pre_Process_exit( Pre_ProcessState **state )
{
   if ( state == NULL || *state == NULL )
      return;

   /* deallocate memory */
   free( *state );
   *state = NULL;
   return;
}


/*
 * Pre_Process_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    succeed = 0
 */
static Word32 Pre_Process_init( Pre_ProcessState **state )
{
   Pre_ProcessState * s;

   if ( state == ( Pre_ProcessState * * )NULL ) {
      fprintf( stderr, "Pre_Process_init: invalid parameter\n" );
      return-1;
   }
   *state = NULL;

   /* allocate memory */
   if ( ( s = ( Pre_ProcessState * ) malloc( sizeof( Pre_ProcessState ) ) ) ==
         NULL ) {
      fprintf( stderr, "Pre_Process_init: can not malloc state structure\n" );
      return-1;
   }
   Pre_Process_reset( s );
   *state = s;
   return 0;
}


/*
 * Pre_Process
 *
 *
 * Parameters:
 *    y2, y1, x0, x1    B: filter memory
 *    speech            I: speech vector to be processed
 *    fspeech           O: processed vector
 *    size              I: size of the vector
 *
 * Function:
 *    Pre-processing
 *
 *    Two pre-processing functions are applied prior to
 *    the encoding process: high-pass filtering and signal down-scaling.
 *    Down-scaling consists of dividing the input by a factor of 2
 *    to reduce the possibility of overflows in the fixed-point
 *    implementation. The high-pass filter serves as a precaution
 *    against undesired low frequency components. A filter with
 *    a cut off frequency of 80 Hz is used, and it is given by:
 *
 *            0.927246093 - 1.8544941z^-1 + 0.927246903z^-2
 *    H(z) = -----------------------------------------------
 *                1 - 1.906005859z^-1 + 0.911376953z^-2
 *
 *    Down-scaling and high-pass filtering are combined by dividing
 *    the coefficients at the numerator by 2.
 *
 * Returns:
 *    void
 */
static void Pre_Process( Float32 *y2, Float32 *y1, Float32 *x0, Float32
      *x1, Word16 *speech, Float32 *f_speech )
{
   Word32 i;
   Float32 x2;
   Float32 tmp;


   for ( i = 0; i < 160; i++ ) {
      x2 = *x1;
      *x1 = *x0;
      *x0 = speech[i];
      tmp = ( Float32 )( 0.4636230465* *x0 - 0.92724705 * *x1 + 0.4636234515 *
            x2 + 1.906005859 * *y1 - 0.911376953 * *y2 );
      f_speech[i] = tmp;
      *y2 = *y1;
      *y1 = tmp;
   }

   if ( ( fabs( *y1 )+fabs( *y2 ) ) < 0.0000000001 )
      *y2 = *y1 = 0;
}


/*
 * cod_amr_reset
 *
 *
 * Parameters:
 *    s                 B: state structure
 *    dtx               I: dtx on/off
 *
 * Function:
 *    Resets state memory
 *
 * Returns:
 *    void
 */
static void cod_amr_reset( cod_amrState *s, Word32 dtx )
{
   Word32 i;


   /* reset DTX */
   s->dtx = dtx;

   /* reset Pitch_frState */
   s->clLtpSt->pitchSt->T0_prev_subframe = 0;

   /* reset Q_plsfState */
   memset( s->lspSt->qSt->past_rq, 0, sizeof( Float32 )*M );
   memcpy( s->lspSt->lsp_old, lsp_init_data, sizeof( lsp_init_data ) );
   memcpy( s->lspSt->lsp_old_q, lsp_init_data, sizeof( lsp_init_data ) );

   /* reset gc_predState */
   for ( i = 0; i < NPRED; i++ ) {
      s->gainQuantSt->gc_predSt->past_qua_en[i] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+MR475_VQ_SIZE*2+DTX_VQ_SIZE;
      s->gainQuantSt->gc_predUncSt->past_qua_en[i] = NB_QUA_CODE+VQ_SIZE_HIGHRATES+VQ_SIZE_LOWRATES+MR475_VQ_SIZE*2+DTX_VQ_SIZE;
   }

   /* reset gain_adaptState */
   s->gainQuantSt->adaptSt->onset = 0;
   s->gainQuantSt->adaptSt->prev_alpha = 0.0F;
   s->gainQuantSt->adaptSt->prev_gc = 0.0F;
   memset( s->gainQuantSt->adaptSt->ltpg_mem, 0, sizeof( Float32 )*LTPG_MEM_SIZE
         );
   s->gainQuantSt->sf0_gcode0_exp = 0;
   s->gainQuantSt->sf0_gcode0_fra = 0;
   s->gainQuantSt->sf0_target_en = 0.0F;
   memset( s->gainQuantSt->sf0_coeff, 0, sizeof( Float32 )*5 );
   s->gainQuantSt->gain_idx_ptr = NULL;

   /* reset pitchOLWghtState */
   s->pitchOLWghtSt->old_T0_med = 40;
   s->pitchOLWghtSt->ada_w = 0.0F;
   s->pitchOLWghtSt->wght_flg = 0;

   /* reset tonStabState */
   s->tonStabSt->count = 0;
   memset( s->tonStabSt->gp, 0, sizeof( Float32 )*N_FRAME );

   /* reset LevinsonState */
   s->lpcSt->LevinsonSt->old_A[0] = 1.0F;
   memset( &s->lpcSt->LevinsonSt->old_A[1], 0, sizeof( Float32 )*M );

#ifdef VAD2
   /* reset vadState */
   s->vadSt->pre_emp_mem = 0.0;
   s->vadSt->update_cnt = 0;
   s->vadSt->hyster_cnt = 0;
   s->vadSt->last_update_cnt = 0;
   for ( i = 0; i < NUM_CHAN; i++ ) {
     s->vadSt->ch_enrg_long_db[i] = 0.0;
     s->vadSt->ch_enrg[i] = 0.0;
     s->vadSt->ch_noise[i] = 0.0;
   }
   s->vadSt->Lframe_cnt = 0L;
   s->vadSt->tsnr = 0.0;
   s->vadSt->hangover = 0;
   s->vadSt->burstcount = 0;
   s->vadSt->fupdate_flag = 0;
   s->vadSt->negSNRvar = 0.0;
   s->vadSt->negSNRbias = 0.0;
   s->vadSt->R0 = 0.0;
   s->vadSt->Rmax = 0.0;
   s->vadSt->LTP_flag = 0;
#else
   /* reset vadState */
   s->vadSt->oldlag_count = 0;
   s->vadSt->oldlag = 0;
   s->vadSt->pitch = 0;
   s->vadSt->tone = 0;
   s->vadSt->complex_high = 0;
   s->vadSt->complex_low = 0;
   s->vadSt->complex_hang_timer = 0;
   s->vadSt->vadreg = 0;
   s->vadSt->burst_count = 0;
   s->vadSt->hang_count = 0;
   s->vadSt->complex_hang_count = 0;

   /* initialize memory used by the filter bank */
   for ( i = 0; i < 3; i++ ) {
      s->vadSt->a_data5[i][0] = 0;
      s->vadSt->a_data5[i][1] = 0;
   }

   for ( i = 0; i < 5; i++ ) {
      s->vadSt->a_data3[i] = 0;
   }

   /* reset dtx_encState */
   /* initialize the rest of the memory */
   for ( i = 0; i < COMPLEN; i++ ) {
      s->vadSt->bckr_est[i] = NOISE_INIT;
      s->vadSt->old_level[i] = NOISE_INIT;
      s->vadSt->ave_level[i] = NOISE_INIT;
      s->vadSt->sub_level[i] = 0;
   }
   s->vadSt->best_corr_hp = CVAD_LOWPOW_RESET;
   s->vadSt->speech_vad_decision = 0;
   s->vadSt->complex_warning = 0;
   s->vadSt->sp_burst_count = 0;
   s->vadSt->corr_hp_fast = CVAD_LOWPOW_RESET;
#endif

   s->dtxEncSt->hist_ptr = 0;
   s->dtxEncSt->log_en_index = 0;
   s->dtxEncSt->init_lsf_vq_index = 0;
   s->dtxEncSt->lsp_index[0] = 0;
   s->dtxEncSt->lsp_index[1] = 0;
   s->dtxEncSt->lsp_index[2] = 0;

   for ( i = 0; i < DTX_HIST_SIZE; i++ ) {
      memcpy( &s->dtxEncSt->lsp_hist[i * M], lsp_init_data, sizeof( Float32 )*M
            );
   }
   memset( s->dtxEncSt->log_en_hist, 0, M * sizeof( Float32 ) );
   s->dtxEncSt->dtxHangoverCount = DTX_HANG_CONST;
   s->dtxEncSt->decAnaElapsedCount = DTX_ELAPSED_FRAMES_THRESH;

   /* init speech pointers */
   /* New speech */
   s->new_speech = s->old_speech + L_TOTAL - L_FRAME;

   /* Present frame */
   s->speech = s->new_speech - L_NEXT;
   s->p_window = s->old_speech + L_TOTAL - L_WINDOW;

   /* For LPC window				*/
   s->p_window_12k2 = s->p_window - L_NEXT;

   /* Initialize static pointers */
   s->wsp = s->old_wsp + PIT_MAX;
   s->exc = s->old_exc + PIT_MAX + L_INTERPOL;
   s->zero = s->ai_zero + MP1;
   s->error = s->mem_err + M;
   s->h1 = &s->hvec[L_SUBFR];

   /* Static vectors to zero */
   memset( s->old_speech, 0, sizeof( Float32 )*L_TOTAL );
   memset( s->old_exc, 0, sizeof( Float32 )*( PIT_MAX + L_INTERPOL ) );
   memset( s->old_wsp, 0, sizeof( Float32 )*PIT_MAX );
   memset( s->mem_syn, 0, sizeof( Float32 )*M );
   memset( s->mem_w, 0, sizeof( Float32 )*M );
   memset( s->mem_w0, 0, sizeof( Float32 )*M );
   memset( s->mem_err, 0, sizeof( Float32 )*M );
   memset( s->ai_zero, 0, sizeof( Float32 )*L_SUBFR );
   memset( s->hvec, 0, sizeof( Float32 )*L_SUBFR );

   for ( i = 0; i < 5; i++ ) {
      s->old_lags[i] = 40;
   }
   s->sharp = 0.0F;
}


/*
 * cod_amr_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *    dtx               I: dtx mode used
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    succeed = 0
 */
static Word32 cod_amr_init( cod_amrState **state, Word32 dtx )
{
   cod_amrState * s;

   if ( ( s = ( cod_amrState * ) malloc( sizeof( cod_amrState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init clLtpState */
   if ( ( s->clLtpSt = ( clLtpState * ) malloc( sizeof( clLtpState ) ) ) == NULL
         ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init Pitch_frState */
   if ( ( s->clLtpSt->pitchSt = ( Pitch_frState * ) malloc( sizeof(
         Pitch_frState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init lspState */
   if ( ( s->lspSt = ( lspState * ) malloc( sizeof( lspState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init Q_plsfState */
   if ( ( s->lspSt->qSt = ( Q_plsfState * ) malloc( sizeof( Q_plsfState ) ) ) ==
         NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init gainQuantState */
   if ( ( s->gainQuantSt = ( gainQuantState * ) malloc( sizeof( gainQuantState )
         ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init gc_predState x2 */
   if ( ( s->gainQuantSt->gc_predSt = ( gc_predState * ) malloc( sizeof(
         gc_predState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   if ( ( s->gainQuantSt->gc_predUncSt = ( gc_predState * ) malloc( sizeof(
         gc_predState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init gain_adaptState */
   if ( ( s->gainQuantSt->adaptSt = ( gain_adaptState * ) malloc( sizeof(
         gain_adaptState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init pitchOLWghtState */
   if ( ( s->pitchOLWghtSt = ( pitchOLWghtState * ) malloc( sizeof(
         pitchOLWghtState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init tonStabState */
   if ( ( s->tonStabSt = ( tonStabState * ) malloc( sizeof( tonStabState ) ) )
         == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init lpcState */
   if ( ( s->lpcSt = ( lpcState * ) malloc( sizeof( lpcState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* init LevinsonState */
   if ( ( s->lpcSt->LevinsonSt = ( LevinsonState * ) malloc( sizeof(
         LevinsonState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   if ( ( s->vadSt = ( vadState * ) malloc( sizeof( vadState ) ) ) == NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }

   /* Init dtx_encState */
   if ( ( s->dtxEncSt = ( dtx_encState * ) malloc( sizeof( dtx_encState ) ) ) ==
         NULL ) {
      fprintf( stderr, "can not malloc state structure\n" );
      return-1;
   }
   cod_amr_reset( s, dtx );
   *state = s;
   return 0;
}


/*
 * cod_amr_exit
 *
 *
 * Parameters:
 *    state             I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
static void cod_amr_exit( cod_amrState **state )
{
   if ( state == NULL || *state == NULL )
      return;

   /* deallocate memory */
   free( ( *state )->vadSt );
   free( ( *state )->gainQuantSt->gc_predSt );
   free( ( *state )->gainQuantSt->gc_predUncSt );
   free( ( *state )->gainQuantSt->adaptSt );
   free( ( *state )->clLtpSt->pitchSt );
   free( ( *state )->lspSt->qSt );
   free( ( *state )->lpcSt->LevinsonSt );
   free( ( *state )->lpcSt );
   free( ( *state )->lspSt );
   free( ( *state )->clLtpSt );
   free( ( *state )->gainQuantSt );
   free( ( *state )->pitchOLWghtSt );
   free( ( *state )->tonStabSt );
   free( ( *state )->dtxEncSt );
   free( *state );
   *state = NULL;
   return;
}


/*
 * Speech_Encode_Frame_init
 *
 *
 * Parameters:
 *    state             O: state structure
 *    dtx               I: dtx mode used
 *
 * Function:
 *    Allocates state memory and initializes state memory
 *
 * Returns:
 *    succeed = 0
 */
void * Speech_Encode_Frame_init( int dtx )
{
   Speech_Encode_FrameState * s;

   /* allocate memory */
   if ( ( s = ( Speech_Encode_FrameState * ) malloc( sizeof(
         Speech_Encode_FrameState ) ) ) == NULL ) {
      fprintf( stderr, "Speech_Encode_Frame_init: can not malloc state "
            "structure\n" );
      return NULL;
   }
   s->pre_state = NULL;
   s->cod_amr_state = NULL;
   s->dtx = dtx;

   if ( Pre_Process_init( &s->pre_state ) || cod_amr_init( &s->cod_amr_state,
         dtx ) ) {
      Speech_Encode_Frame_exit( ( void ** )( &s ) );
      return NULL;
   }
   return s;
}


/*
 * Speech_Encode_Frame_reset
 *
 *
 * Parameters:
 *    state          O: state structure
 *
 * Function:
 *    Resets state memory
 *
 * Returns:
 *
 */
int Speech_Encode_Frame_reset( void *st, int dtx )
{
   Speech_Encode_FrameState * state;
   state = ( Speech_Encode_FrameState * )st;

   if ( ( Speech_Encode_FrameState * )state == NULL ) {
      fprintf( stderr, "Speech_Encode_Frame_reset: invalid parameter\n" );
      return-1;
   }
   Pre_Process_reset( state->pre_state );
   cod_amr_reset( state->cod_amr_state, dtx );
   return 0;
}


/*
 * Speech_Encode_Frame_exit
 *
 *
 * Parameters:
 *    state            I: state structure
 *
 * Function:
 *    The memory used for state memory is freed
 *
 * Returns:
 *    Void
 */
void Speech_Encode_Frame_exit( void **st )
{
   if ( ( Speech_Encode_FrameState * )( *st ) == NULL )
      return;
   Pre_Process_exit( &( ( ( Speech_Encode_FrameState * )( *st ) )->pre_state ) )
   ;
   cod_amr_exit( &( ( ( Speech_Encode_FrameState * )( *st ) )->cod_amr_state ) )
   ;

   /* deallocate memory */
   free( *st );
   *st = NULL;
   return;
}


/*
 * Speech_Encode_Frame
 *
 *
 * Parameters:
 *    st                B: state structure
 *    mode              I: speech coder mode
 *    new_speech        I: speech input, size L_FRAME
 *    prm               O: Analysis parameters
 *    used_mode         B: force VAD/used_mode
 * Function:
 *    Encode one frame
 *
 * Returns:
 *    Void
 */
void Speech_Encode_Frame( void *st, enum Mode mode, Word16 *new_speech, Word16 *
      prm, enum Mode *used_mode )
{
   Float32 syn[L_FRAME];   /* Buffer for synthesis speech */
   Float32 speech[160];
   Word32 i;


   Speech_Encode_FrameState * state;
   state = ( Speech_Encode_FrameState * )st;

   for ( i = 0; i < 160; i++ ) {
      new_speech[i] = ( Word16 )( new_speech[i] & 0xfff8 );
   }

   /* filter + downscaling */
   Pre_Process( &state->pre_state->y2, &state->pre_state->y1, &state->pre_state
         ->x0, &state->pre_state->x1, new_speech, speech );

   /* Call the speech encoder */
   cod_amr( state->cod_amr_state, mode, speech, prm, used_mode, syn );

}
