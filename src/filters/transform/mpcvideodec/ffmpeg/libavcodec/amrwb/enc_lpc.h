/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_LPC_H
#define ENC_LPC_H

#include "typedef.h"

void E_LPC_int_isp_find(Word16 isp_old[], Word16 isp_new[],
                        const Word16 frac[], Word16 Az[]);
void E_LPC_f_int_isp_find(Float32 isp_old[], Float32 isp_new[], Float32 a[],
                          Word32 nb_subfr, Word32 m);
void E_LPC_a_weight(Float32 *a, Float32 *ap, Float32 gamma, Word32 m);
void E_LPC_isf_isp_conversion(Word16 isf[], Word16 isp[], Word16 m);
Word16 E_LPC_isf_sub_vq(Float32 *x, const Float32 *E_ROM_dico, Word32 dim,
                        Word32 E_ROM_dico_size, Float32 *distance);
void E_LPC_isp_a_conversion(Word16 isp[], Word16 a[], Word16 m);
void E_LPC_isp_isf_conversion(Float32 isp[], Float32 isf[], Word32 m);
void E_LPC_lag_wind(Float32 r[], Word32 m);
void E_LPC_lev_dur(Float32 *a, Float32 *r, Word32 m);
void E_LPC_a_isp_conversion(Float32 *a, Float32 *isp, Float32 *old_isp,
                            Word32 m);
void E_LPC_isf_2s3s_quantise(Float32 *isf1, Word16 *isf_q, Word16 *past_isfq,
                             Word32 *indice, Word32 nb_surv);
void E_LPC_isf_2s5s_quantise(Float32 *isf1, Word16 *isf_q, Word16 *past_isfq,
                             Word32 *indice, Word32 nb_surv);


#endif

