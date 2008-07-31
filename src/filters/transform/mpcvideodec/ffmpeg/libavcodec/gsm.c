
/*
   Written by Mark Podlipec <podlipec@ici.net>.

   Most of this code comes from a GSM 06.10 library by
   Jutta Degener and Carsten Bormann, available via
   <http://www.pobox.com/~jutta/toast.html>.

   That library is distributed with the following copyright:

    Copyright 1992 by Jutta Degener and Carsten Bormann,
    Technische Universitaet Berlin

Any use of this software is permitted provided that this notice is not
removed and that neither the authors nor the Technische Universitaet Berlin
are deemed to have made any representations as to the suitability of this
software for any purpose nor are held responsible for any defects of
this software.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.

As a matter of courtesy, the authors request to be informed about uses
this software has found, about bugs in this software, and about any
improvements that may be of general interest.

    Berlin, 15.09.1992
    Jutta Degener
    Carsten Bormann
*/

#include "avcodec.h"

typedef struct XAGSMstate {

	int16_t		dp0[ 280 ];

	int16_t		z1;		/* preprocessing.c, Offset_com. */
	int32_t	L_z2;		/*                  Offset_com. */
	int		mp;		/*                  Preemphasis	*/

	int16_t		u[8];		/* short_term_aly_filter.c	*/
	int16_t		LARpp[2][8]; 	/*                              */
	int16_t		j;		/*                              */

	int16_t            ltp_cut;        /* long_term.c, LTP crosscorr.  */
	int16_t		nrp; /* 40 */	/* long_term.c, synthesis	*/
	int16_t		v[9];		/* short_term.c, synthesis	*/
	int16_t		msr;		/* decoder.c,	Postprocessing	*/

	char		verbose;	/* only used if !NDEBUG		*/
	char		fast;		/* only used if FAST		*/

	char		wav_fmt;	/* only used if WAV49 defined	*/
	unsigned char	frame_index;	/*            odd/even chaining	*/
	unsigned char	frame_chain;	/*   half-byte to carry forward	*/
} XAGSMstate;

static int gsm_decode_init(AVCodecContext * avctx)
{
    XAGSMstate *s = avctx->priv_data;

    memset(s,0,sizeof(XAGSMstate));
    s->nrp = 40;
    
    avctx->sample_fmt = SAMPLE_FMT_S16;

    return 0;
}

/*   Table 4.3b   Quantization levels of the LTP gain quantizer
 */
/* bc                 0          1        2          3                  */
static const int16_t gsm_QLB[4] = {  3277,    11469,    21299,     32767        };

/*   Table 4.6   Normalized direct mantissa used to compute xM/xmax
 */
/* i                  0      1       2      3      4      5      6      7   */
static const int16_t gsm_FAC[8] = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };

#define	MIN_WORD	(-32767 - 1)
#define	MAX_WORD	  32767

#ifdef	SASR		/* flag: >> is a signed arithmetic shift right */
#undef	SASR
#define	SASR(x, by)	((x) >> (by))
#else
#define	SASR(x, by)	((x) >= 0 ? (x) >> (by) : (~(-((x) + 1) >> (by))))
#endif	/* SASR */

/*
 * #define GSM_MULT_R(a, b) (* word a, word b, !(a == b == MIN_WORD) *)	\
 *	(0x0FFFF & SASR(((int32_t)(a) * (int32_t)(b) + 16384), 15))
 */
#define GSM_MULT_R(a, b) /* word a, word b, !(a == b == MIN_WORD) */	\
	(SASR( ((int32_t)(a) * (int32_t)(b) + 16384), 15 ))

# define GSM_MULT(a,b)	 /* word a, word b, !(a == b == MIN_WORD) */	\
	(SASR( ((int32_t)(a) * (int32_t)(b)), 15 ))

# define GSM_L_MULT(a, b) /* word a, word b */	\
	(((int32_t)(a) * (int32_t)(b)) << 1)

# define GSM_L_ADD(a, b)	\
	( (a) <  0 ? ( (b) >= 0 ? (a) + (b)	\
		 : (utmp = (uint32_t)-((a) + 1) + (uint32_t)-((b) + 1)) \
		   >= MAX_LONGWORD ? MIN_LONGWORD : -(int32_t)utmp-2 )   \
	: ((b) <= 0 ? (a) + (b)   \
	          : (utmp = (uint32_t)(a) + (uint32_t)(b)) >= MAX_LONGWORD \
		    ? MAX_LONGWORD : utmp))

/*
 * # define GSM_ADD(a, b)	\
 * 	((ltmp = (int32_t)(a) + (int32_t)(b)) >= MAX_WORD \
 * 	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)
 */
/* Nonportable, but faster: */

#define	GSM_ADD(a, b)	\
	((uint32_t)((ltmp = (int32_t)(a) + (int32_t)(b)) - MIN_WORD) > \
		MAX_WORD - MIN_WORD ? (ltmp > 0 ? MAX_WORD : MIN_WORD) : ltmp)

# define GSM_SUB(a, b)	\
	((ltmp = (int32_t)(a) - (int32_t)(b)) >= MAX_WORD \
	? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)

# define GSM_ABS(a)	((a) < 0 ? ((a) == MIN_WORD ? MAX_WORD : -(a)) : (a))

/****************/
#define saturate(x)     \
        ((x) < MIN_WORD ? MIN_WORD : (x) > MAX_WORD ? MAX_WORD: (x))

/****************/
static int16_t gsm_sub (
int16_t a,
int16_t b)
{
        int32_t diff = (int32_t)a - (int32_t)b;
        return saturate(diff);
}

/****************/
static int16_t gsm_asr (
int16_t a,
int n)
{
        if (n >= 16) return -(a < 0);
        if (n <= -16) return 0;
        if (n < 0) return a << -n;

#       ifdef   SASR
                return a >> n;
#       else
                if (a >= 0) return a >> n;
                else return -(int16_t)( -(uint16_t)a >> n );
#       endif
}

/****************/
static int16_t gsm_asl (
int16_t a,
int n)
{
        if (n >= 16) return 0;
        if (n <= -16) return -(a < 0);
        if (n < 0) return gsm_asr(a, -n);
        return a << n;
}


/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/**** 4.2.17 */
static void RPE_grid_positioning(
int16_t            Mc,             /* grid position        IN      */
register int16_t   * xMp,          /* [0..12]              IN      */
register int16_t   * ep)           /* [0..39]              OUT     */
/*
 *  This procedure computes the reconstructed long term residual signal
 *  ep[0..39] for the LTP analysis filter.  The inputs are the Mc
 *  which is the grid position selection and the xMp[0..12] decoded
 *  RPE samples which are upsampled by a factor of 3 by inserting zero
 *  values.
 */
{
        int     i = 13;

        assert(0 <= Mc && Mc <= 3);

        switch (Mc) {
                case 3: *ep++ = 0;
                case 2:  do {
                                *ep++ = 0;
                case 1:         *ep++ = 0;
                case 0:         *ep++ = *xMp++;
                         } while (--i);
        }
        while (++Mc < 4) *ep++ = 0;

        /*

        int i, k;
        for (k = 0; k <= 39; k++) ep[k] = 0;
        for (i = 0; i <= 12; i++) {
                ep[ Mc + (3*i) ] = xMp[i];
        }
        */
}


/**** 4.2.16 */
static void APCM_inverse_quantization (
register int16_t   * xMc,  /* [0..12]                      IN      */
int16_t            mant,
int16_t            exp,
register int16_t   * xMp)  /* [0..12]                      OUT     */
/*
 *  This part is for decoding the RPE sequence of coded xMc[0..12]
 *  samples to obtain the xMp[0..12] array.  Table 4.6 is used to get
 *  the mantissa of xmaxc (FAC[0..7]).
 */
{
        int     i;
        int16_t    temp, temp1, temp2, temp3;
        int32_t        ltmp;

        assert( mant >= 0 && mant <= 7 );

        temp1 = gsm_FAC[ mant ];        /* see 4.2-15 for mant */
        temp2 = gsm_sub( 6, exp );      /* see 4.2-15 for exp  */
        temp3 = gsm_asl( 1, gsm_sub( temp2, 1 ));

        for (i = 13; i--;) {

                assert( *xMc <= 7 && *xMc >= 0 );       /* 3 bit unsigned */

                /* temp = gsm_sub( *xMc++ << 1, 7 ); */
                temp = (*xMc++ << 1) - 7;               /* restore sign   */
                assert( temp <= 7 && temp >= -7 );      /* 4 bit signed   */

                temp <<= 12;                            /* 16 bit signed  */
                temp = GSM_MULT_R( temp1, temp );
                temp = GSM_ADD( temp, temp3 );
                *xMp++ = gsm_asr( temp, temp2 );
        }
}


/**** 4.12.15 */
static void APCM_quantization_xmaxc_to_exp_mant (
int16_t            xmaxc,          /* IN   */
int16_t            * exp_out,      /* OUT  */
int16_t            * mant_out)    /* OUT  */
{
  int16_t    exp, mant;

  /* Compute exponent and mantissa of the decoded version of xmaxc
   */

        exp = 0;
        if (xmaxc > 15) exp = SASR(xmaxc, 3) - 1;
        mant = xmaxc - (exp << 3);

        if (mant == 0) {
                exp  = -4;
                mant = 7;
        }
        else {
                while (mant <= 7) {
                        mant = mant << 1 | 1;
                        exp--;
                }
                mant -= 8;
        }

        assert( exp  >= -4 && exp <= 6 );
        assert( mant >= 0 && mant <= 7 );

        *exp_out  = exp;
        *mant_out = mant;
}

static void Gsm_RPE_Decoding (
XAGSMstate        * S,
int16_t            xmaxcr,
int16_t            Mcr,
int16_t            * xMcr,  /* [0..12], 3 bits             IN      */
int16_t            * erp)   /* [0..39]                     OUT     */

{
        int16_t    exp, mant;
        int16_t    xMp[ 13 ];

        APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &exp, &mant );
        APCM_inverse_quantization( xMcr, mant, exp, xMp );
        RPE_grid_positioning( Mcr, xMp, erp );

}


/*
 *  4.3 FIXED POINT IMPLEMENTATION OF THE RPE-LTP DECODER
 */

static void Postprocessing(
XAGSMstate	* S,
register int16_t 	* s)
{
  register int		k;
  register int16_t		msr = S->msr;
  register int32_t	ltmp;	/* for GSM_ADD */
  register int16_t		tmp;

  for (k = 160; k--; s++)
  {
    tmp = GSM_MULT_R( msr, 28180 );
    msr = GSM_ADD(*s, tmp);  	   /* Deemphasis 	     */
    *s  = GSM_ADD(msr, msr) & 0xFFF8;  /* Truncation & Upscaling */
  }
  S->msr = msr;
}

/**** 4.3.2 */
static void Gsm_Long_Term_Synthesis_Filtering (
XAGSMstate        * S,
int16_t                    Ncr,
int16_t                    bcr,
register int16_t           * erp,     /* [0..39]                    IN */
register int16_t           * drp)     /* [-120..-1] IN, [-120..40] OUT */

/*
 *  This procedure uses the bcr and Ncr parameter to realize the
 *  long term synthesis filtering.  The decoding of bcr needs
 *  table 4.3b.
 */
{
        register int32_t       ltmp;   /* for ADD */
        register int            k;
        int16_t                    brp, drpp, Nr;

        /*  Check the limits of Nr.
         */
        Nr = Ncr < 40 || Ncr > 120 ? S->nrp : Ncr;
        S->nrp = Nr;
        assert(Nr >= 40 && Nr <= 120);

        /*  Decoding of the LTP gain bcr
         */
        brp = gsm_QLB[ bcr ];

        /*  Computation of the reconstructed short term residual
         *  signal drp[0..39]
         */
        assert(brp != MIN_WORD);

        for (k = 0; k <= 39; k++) {
                drpp   = GSM_MULT_R( brp, drp[ k - Nr ] );
                drp[k] = GSM_ADD( erp[k], drpp );
        }

        /*
         *  Update of the reconstructed short term residual signal
         *  drp[ -1..-120 ]
         */

        for (k = 0; k <= 119; k++) drp[ -120 + k ] = drp[ -80 + k ];
}

static void Short_term_synthesis_filtering (
XAGSMstate *S,
register int16_t   *rrp,  /* [0..7]       IN      */
register int    k,      /* k_end - k_start      */
register int16_t   *wt,   /* [0..k-1]     IN      */
register int16_t   *sr)   /* [0..k-1]     OUT     */
{
        register int16_t           * v = S->v;
        register int            i;
        register int16_t           sri, tmp1, tmp2;
        register int32_t       ltmp;   /* for GSM_ADD  & GSM_SUB */

        while (k--) {
                sri = *wt++;
                for (i = 8; i--;) {

                        /* sri = GSM_SUB( sri, gsm_mult_r( rrp[i], v[i] ) );
                         */
                        tmp1 = rrp[i];
                        tmp2 = v[i];
                        tmp2 =  ( tmp1 == MIN_WORD && tmp2 == MIN_WORD
                                ? MAX_WORD
                                : 0x0FFFF & (( (int32_t)tmp1 * (int32_t)tmp2
                                             + 16384) >> 15)) ;

                        sri  = GSM_SUB( sri, tmp2 );

                        /* v[i+1] = GSM_ADD( v[i], gsm_mult_r( rrp[i], sri ) );
                         */
                        tmp1  = ( tmp1 == MIN_WORD && sri == MIN_WORD
                                ? MAX_WORD
                                : 0x0FFFF & (( (int32_t)tmp1 * (int32_t)sri
                                             + 16384) >> 15)) ;

                        v[i+1] = GSM_ADD( v[i], tmp1);
                }
                *sr++ = v[0] = sri;
        }
}

/* 4.2.8 */

static void Decoding_of_the_coded_Log_Area_Ratios (
int16_t    * LARc,         /* coded log area ratio [0..7]  IN      */
int16_t    * LARpp)        /* out: decoded ..                      */
{
        register int16_t   temp1 /* , temp2 */;
        register long   ltmp;   /* for GSM_ADD */

        /*  This procedure requires for efficient implementation
         *  two tables.
         *
         *  INVA[1..8] = integer( (32768 * 8) / real_A[1..8])
         *  MIC[1..8]  = minimum value of the LARc[1..8]
         */

        /*  Compute the LARpp[1..8]
         */

        /*      for (i = 1; i <= 8; i++, B++, MIC++, INVA++, LARc++, LARpp++) {
         *
         *              temp1  = GSM_ADD( *LARc, *MIC ) << 10;
         *              temp2  = *B << 1;
         *              temp1  = GSM_SUB( temp1, temp2 );
         *
         *              assert(*INVA != MIN_WORD);
         *
         *              temp1  = GSM_MULT_R( *INVA, temp1 );
         *              *LARpp = GSM_ADD( temp1, temp1 );
         *      }
         */

#undef  STEP
#define STEP( B, MIC, INVA )    \
                temp1    = GSM_ADD( *LARc++, MIC ) << 10;       \
                temp1    = GSM_SUB( temp1, B << 1 );            \
                temp1    = GSM_MULT_R( INVA, temp1 );           \
                *LARpp++ = GSM_ADD( temp1, temp1 );

        STEP(      0,  -32,  13107 );
        STEP(      0,  -32,  13107 );
        STEP(   2048,  -16,  13107 );
        STEP(  -2560,  -16,  13107 );

        STEP(     94,   -8,  19223 );
        STEP(  -1792,   -8,  17476 );
        STEP(   -341,   -4,  31454 );
        STEP(  -1144,   -4,  29708 );

        /* NOTE: the addition of *MIC is used to restore
         *       the sign of *LARc.
         */
}

/* 4.2.9 */
/* Computation of the quantized reflection coefficients
 */

/* 4.2.9.1  Interpolation of the LARpp[1..8] to get the LARp[1..8]
 */

/*
 *  Within each frame of 160 analyzed speech samples the short term
 *  analysis and synthesis filters operate with four different sets of
 *  coefficients, derived from the previous set of decoded LARs(LARpp(j-1))
 *  and the actual set of decoded LARs (LARpp(j))
 *
 * (Initial value: LARpp(j-1)[1..8] = 0.)
 */

static void Coefficients_0_12 (
register int16_t * LARpp_j_1,
register int16_t * LARpp_j,
register int16_t * LARp)
{
        register int    i;
        register int32_t ltmp;

        for (i = 1; i <= 8; i++, LARp++, LARpp_j_1++, LARpp_j++) {
                *LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
                *LARp = GSM_ADD( *LARp,  SASR( *LARpp_j_1, 1));
        }
}

static void Coefficients_13_26 (
register int16_t * LARpp_j_1,
register int16_t * LARpp_j,
register int16_t * LARp)
{
        register int i;
        register int32_t ltmp;
        for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
                *LARp = GSM_ADD( SASR( *LARpp_j_1, 1), SASR( *LARpp_j, 1 ));
        }
}

static void Coefficients_27_39 (
register int16_t * LARpp_j_1,
register int16_t * LARpp_j,
register int16_t * LARp)
{
        register int i;
        register int32_t ltmp;

        for (i = 1; i <= 8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
                *LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
                *LARp = GSM_ADD( *LARp, SASR( *LARpp_j, 1 ));
        }
}


static void Coefficients_40_159 (
register int16_t * LARpp_j,
register int16_t * LARp)
{
        register int i;

        for (i = 1; i <= 8; i++, LARp++, LARpp_j++)
                *LARp = *LARpp_j;
}
/* 4.2.9.2 */

static void LARp_to_rp (
register int16_t * LARp)  /* [0..7] IN/OUT  */
/*
 *  The input of this procedure is the interpolated LARp[0..7] array.
 *  The reflection coefficients, rp[i], are used in the analysis
 *  filter and in the synthesis filter.
 */
{
        register int            i;
        register int16_t           temp;
        register int32_t       ltmp;

        for (i = 1; i <= 8; i++, LARp++) {

                /* temp = GSM_ABS( *LARp );
                 *
                 * if (temp < 11059) temp <<= 1;
                 * else if (temp < 20070) temp += 11059;
                 * else temp = GSM_ADD( temp >> 2, 26112 );
                 *
                 * *LARp = *LARp < 0 ? -temp : temp;
                 */

                if (*LARp < 0) {
                        temp = *LARp == MIN_WORD ? MAX_WORD : -(*LARp);
                        *LARp = - ((temp < 11059) ? temp << 1
                                : ((temp < 20070) ? temp + 11059
                                :  GSM_ADD( temp >> 2, 26112 )));
                } else {
                        temp  = *LARp;
                        *LARp =    (temp < 11059) ? temp << 1
                                : ((temp < 20070) ? temp + 11059
                                :  GSM_ADD( temp >> 2, 26112 ));
                }
        }
}





/**** */
static void Gsm_Short_Term_Synthesis_Filter (
XAGSMstate * S,
int16_t    * LARcr,        /* received log area ratios [0..7] IN  */
int16_t    * wt,           /* received d [0..159]             IN  */
int16_t    * s)            /* signal   s [0..159]            OUT  */
{
        int16_t            * LARpp_j       = S->LARpp[ S->j     ];
        int16_t            * LARpp_j_1     = S->LARpp[ S->j ^=1 ];

        int16_t            LARp[8];

#undef  FILTER
#if     defined(FAST) && defined(USE_FLOAT_MUL)

#       define  FILTER  (* (S->fast                     \
                           ? Fast_Short_term_synthesis_filtering        \
                           : Short_term_synthesis_filtering     ))
#else
#       define  FILTER  Short_term_synthesis_filtering
#endif

        Decoding_of_the_coded_Log_Area_Ratios( LARcr, LARpp_j );

        Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
        LARp_to_rp( LARp );
        FILTER( S, LARp, 13, wt, s );

        Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
        LARp_to_rp( LARp );
        FILTER( S, LARp, 14, wt + 13, s + 13 );

        Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
        LARp_to_rp( LARp );
        FILTER( S, LARp, 13, wt + 27, s + 27 );

        Coefficients_40_159( LARpp_j, LARp );
        LARp_to_rp( LARp );
        FILTER(S, LARp, 120, wt + 40, s + 40);
}

static void GSM_Decode(
XAGSMstate	*S,
int16_t		*LARcr,		/* [0..7]		IN	*/
int16_t		*Ncr,		/* [0..3] 		IN 	*/
int16_t		*bcr,		/* [0..3]		IN	*/
int16_t		*Mcr,		/* [0..3] 		IN 	*/
int16_t		*xmaxcr,	/* [0..3]		IN 	*/
int16_t		*xMcr,		/* [0..13*4]		IN	*/
int16_t		*s)		/* [0..159]		OUT 	*/
{
  int		j, k;
  int16_t		erp[40], wt[160];
  int16_t		*drp = S->dp0 + 120;

  for (j=0; j <= 3; j++, xmaxcr++, bcr++, Ncr++, Mcr++, xMcr += 13)
  {
    Gsm_RPE_Decoding( S, *xmaxcr, *Mcr, xMcr, erp );
    Gsm_Long_Term_Synthesis_Filtering( S, *Ncr, *bcr, erp, drp );
    for (k = 0; k <= 39; k++) wt[ j * 40 + k ] =  drp[ k ];
  }

  Gsm_Short_Term_Synthesis_Filter( S, LARcr, wt, s );
  Postprocessing(S, s);
}

/****-------------------------------------------------------------------****
 **** Podlipec:  For AVI/WAV files GSM 6.10 combines two 33 bytes frames
 **** into one 65 byte frame.
 ****-------------------------------------------------------------------****/
static void XA_MSGSM_Decoder(XAGSMstate *gsm_state,const unsigned char *ibuf,unsigned short *obuf)
{ int16_t sr;
  int16_t  LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];

  sr = *ibuf++;

  LARc[0] = sr & 0x3f;  sr >>= 6;
  sr |= (int16_t)*ibuf++ << 2;
  LARc[1] = sr & 0x3f;  sr >>= 6;
  sr |= (int16_t)*ibuf++ << 4;
  LARc[2] = sr & 0x1f;  sr >>= 5;
  LARc[3] = sr & 0x1f;  sr >>= 5;
  sr |= (int16_t)*ibuf++ << 2;
  LARc[4] = sr & 0xf;  sr >>= 4;
  LARc[5] = sr & 0xf;  sr >>= 4;
  sr |= (int16_t)*ibuf++ << 2;			/* 5 */
  LARc[6] = sr & 0x7;  sr >>= 3;
  LARc[7] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 4;
  Nc[0] = sr & 0x7f;  sr >>= 7;
  bc[0] = sr & 0x3;  sr >>= 2;
  Mc[0] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 1;
  xmaxc[0] = sr & 0x3f;  sr >>= 6;
  xmc[0] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[1] = sr & 0x7;  sr >>= 3;
  xmc[2] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[3] = sr & 0x7;  sr >>= 3;
  xmc[4] = sr & 0x7;  sr >>= 3;
  xmc[5] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;			/* 10 */
  xmc[6] = sr & 0x7;  sr >>= 3;
  xmc[7] = sr & 0x7;  sr >>= 3;
  xmc[8] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[9] = sr & 0x7;  sr >>= 3;
  xmc[10] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[11] = sr & 0x7;  sr >>= 3;
  xmc[12] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 4;
  Nc[1] = sr & 0x7f;  sr >>= 7;
  bc[1] = sr & 0x3;  sr >>= 2;
  Mc[1] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 1;
  xmaxc[1] = sr & 0x3f;  sr >>= 6;
  xmc[13] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;				/* 15 */
  xmc[14] = sr & 0x7;  sr >>= 3;
  xmc[15] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[16] = sr & 0x7;  sr >>= 3;
  xmc[17] = sr & 0x7;  sr >>= 3;
  xmc[18] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[19] = sr & 0x7;  sr >>= 3;
  xmc[20] = sr & 0x7;  sr >>= 3;
  xmc[21] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[22] = sr & 0x7;  sr >>= 3;
  xmc[23] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[24] = sr & 0x7;  sr >>= 3;
  xmc[25] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 4;			/* 20 */
  Nc[2] = sr & 0x7f;  sr >>= 7;
  bc[2] = sr & 0x3;  sr >>= 2;
  Mc[2] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 1;
  xmaxc[2] = sr & 0x3f;  sr >>= 6;
  xmc[26] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[27] = sr & 0x7;  sr >>= 3;
  xmc[28] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[29] = sr & 0x7;  sr >>= 3;
  xmc[30] = sr & 0x7;  sr >>= 3;
  xmc[31] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[32] = sr & 0x7;  sr >>= 3;
  xmc[33] = sr & 0x7;  sr >>= 3;
  xmc[34] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;				/* 25 */
  xmc[35] = sr & 0x7;  sr >>= 3;
  xmc[36] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[37] = sr & 0x7;  sr >>= 3;
  xmc[38] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 4;
  Nc[3] = sr & 0x7f;  sr >>= 7;
  bc[3] = sr & 0x3;  sr >>= 2;
  Mc[3] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 1;
  xmaxc[3] = sr & 0x3f;  sr >>= 6;
  xmc[39] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[40] = sr & 0x7;  sr >>= 3;
  xmc[41] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;			/* 30 */
  xmc[42] = sr & 0x7;  sr >>= 3;
  xmc[43] = sr & 0x7;  sr >>= 3;
  xmc[44] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[45] = sr & 0x7;  sr >>= 3;
  xmc[46] = sr & 0x7;  sr >>= 3;
  xmc[47] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[48] = sr & 0x7;  sr >>= 3;
  xmc[49] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[50] = sr & 0x7;  sr >>= 3;
  xmc[51] = sr & 0x7;  sr >>= 3;

  GSM_Decode(gsm_state, LARc, Nc, bc, Mc, xmaxc, xmc, obuf);

/*
  carry = sr & 0xf;
  sr = carry;
*/
  /* 2nd frame */
  sr &= 0xf;
  sr |= (int16_t)*ibuf++ << 4;			/* 1 */
  LARc[0] = sr & 0x3f;  sr >>= 6;
  LARc[1] = sr & 0x3f;  sr >>= 6;
  sr = *ibuf++;
  LARc[2] = sr & 0x1f;  sr >>= 5;
  sr |= (int16_t)*ibuf++ << 3;
  LARc[3] = sr & 0x1f;  sr >>= 5;
  LARc[4] = sr & 0xf;  sr >>= 4;
  sr |= (int16_t)*ibuf++ << 2;
  LARc[5] = sr & 0xf;  sr >>= 4;
  LARc[6] = sr & 0x7;  sr >>= 3;
  LARc[7] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;				/* 5 */
  Nc[0] = sr & 0x7f;  sr >>= 7;
  sr |= (int16_t)*ibuf++ << 1;
  bc[0] = sr & 0x3;  sr >>= 2;
  Mc[0] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 5;
  xmaxc[0] = sr & 0x3f;  sr >>= 6;
  xmc[0] = sr & 0x7;  sr >>= 3;
  xmc[1] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[2] = sr & 0x7;  sr >>= 3;
  xmc[3] = sr & 0x7;  sr >>= 3;
  xmc[4] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[5] = sr & 0x7;  sr >>= 3;
  xmc[6] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;			/* 10 */
  xmc[7] = sr & 0x7;  sr >>= 3;
  xmc[8] = sr & 0x7;  sr >>= 3;
  xmc[9] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[10] = sr & 0x7;  sr >>= 3;
  xmc[11] = sr & 0x7;  sr >>= 3;
  xmc[12] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  Nc[1] = sr & 0x7f;  sr >>= 7;
  sr |= (int16_t)*ibuf++ << 1;
  bc[1] = sr & 0x3;  sr >>= 2;
  Mc[1] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 5;
  xmaxc[1] = sr & 0x3f;  sr >>= 6;
  xmc[13] = sr & 0x7;  sr >>= 3;
  xmc[14] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;			/* 15 */
  xmc[15] = sr & 0x7;  sr >>= 3;
  xmc[16] = sr & 0x7;  sr >>= 3;
  xmc[17] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[18] = sr & 0x7;  sr >>= 3;
  xmc[19] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[20] = sr & 0x7;  sr >>= 3;
  xmc[21] = sr & 0x7;  sr >>= 3;
  xmc[22] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[23] = sr & 0x7;  sr >>= 3;
  xmc[24] = sr & 0x7;  sr >>= 3;
  xmc[25] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  Nc[2] = sr & 0x7f;  sr >>= 7;
  sr |= (int16_t)*ibuf++ << 1;			/* 20 */
  bc[2] = sr & 0x3;  sr >>= 2;
  Mc[2] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 5;
  xmaxc[2] = sr & 0x3f;  sr >>= 6;
  xmc[26] = sr & 0x7;  sr >>= 3;
  xmc[27] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[28] = sr & 0x7;  sr >>= 3;
  xmc[29] = sr & 0x7;  sr >>= 3;
  xmc[30] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  xmc[31] = sr & 0x7;  sr >>= 3;
  xmc[32] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[33] = sr & 0x7;  sr >>= 3;
  xmc[34] = sr & 0x7;  sr >>= 3;
  xmc[35] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;			/* 25 */
  xmc[36] = sr & 0x7;  sr >>= 3;
  xmc[37] = sr & 0x7;  sr >>= 3;
  xmc[38] = sr & 0x7;  sr >>= 3;
  sr = *ibuf++;
  Nc[3] = sr & 0x7f;  sr >>= 7;
  sr |= (int16_t)*ibuf++ << 1;
  bc[3] = sr & 0x3;  sr >>= 2;
  Mc[3] = sr & 0x3;  sr >>= 2;
  sr |= (int16_t)*ibuf++ << 5;
  xmaxc[3] = sr & 0x3f;  sr >>= 6;
  xmc[39] = sr & 0x7;  sr >>= 3;
  xmc[40] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[41] = sr & 0x7;  sr >>= 3;
  xmc[42] = sr & 0x7;  sr >>= 3;
  xmc[43] = sr & 0x7;  sr >>= 3;
  sr = (int16_t)*ibuf++;				/* 30 */
  xmc[44] = sr & 0x7;  sr >>= 3;
  xmc[45] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 2;
  xmc[46] = sr & 0x7;  sr >>= 3;
  xmc[47] = sr & 0x7;  sr >>= 3;
  xmc[48] = sr & 0x7;  sr >>= 3;
  sr |= (int16_t)*ibuf++ << 1;
  xmc[49] = sr & 0x7;  sr >>= 3;
  xmc[50] = sr & 0x7;  sr >>= 3;
  xmc[51] = sr & 0x7;  sr >>= 3;

  GSM_Decode(gsm_state, LARc, Nc, bc, Mc, xmaxc, xmc, &obuf[160]);

  /* Return number of source bytes consumed and output samples produced */
//  *icnt = 65;
//  *ocnt = 320;
  return;
}

static int gsm_decode_frame(AVCodecContext *avctx,
			    void *data, int *data_size,
			    const uint8_t *buf, int buf_size)
{
    XAGSMstate *s = avctx->priv_data;
    XA_MSGSM_Decoder(s,buf,data);
    *data_size=320*2;
    return 65;
}

AVCodec msgsm_decoder = {
    "msgsm",
    CODEC_TYPE_AUDIO,
    CODEC_ID_GSM_MS,
    sizeof(XAGSMstate),
    gsm_decode_init,
    NULL,
    NULL,
    gsm_decode_frame,
    /*.capabilities = */0,
    /*.next = */NULL,
    /*.flush = */NULL,
    /*.supported_framerates = */NULL,
    /*.pix_fmts = */NULL,
    /*.long_name = */NULL_IF_CONFIG_SMALL("libgsm GSM Microsoft variant"),
};
