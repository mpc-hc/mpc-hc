/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef DEC_LPC_H
#define DEC_LPC_H

#include "typedef.h"

void D_LPC_isf_noise_d(Word16 *indice, Word16 *isf_q);
void D_LPC_isf_isp_conversion(Word16 isf[], Word16 isp[], Word16 m);
void D_LPC_isp_a_conversion(Word16 isp[], Word16 a[], Word32 adaptive_scaling, 
                            Word16 m);
void D_LPC_a_weight(Word16 a[], Word16 ap[], Word16 gamma, Word16 m);
void D_LPC_isf_2s3s_decode(Word16 *indice, Word16 *isf_q, Word16* past_isfq,
                           Word16 *isfold, Word16 *isf_buf, Word16 bfi);
void D_LPC_isf_2s5s_decode(Word16 *indice, Word16 *isf_q, Word16 *past_isfq,
                           Word16 *isfold, Word16 *isf_buf, Word16 bfi);
void D_LPC_int_isp_find(Word16 isp_old[], Word16 isp_new[],
                        const Word16 frac[], Word16 Az[]);
void D_LPC_isf_extrapolation(Word16 HfIsf[]);

#endif

