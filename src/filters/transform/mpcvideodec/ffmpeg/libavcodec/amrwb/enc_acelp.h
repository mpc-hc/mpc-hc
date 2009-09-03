/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_E_ACELP_H
#define ENC_E_ACELP_H

#include "typedef.h"

void E_ACELP_Gain2_Q_init(Word16 *mem);
void E_ACELP_xy2_corr(Float32 xn[], Float32 y1[], Float32 y2[],
                      Float32 g_corr[]);
Float32 E_ACELP_xy1_corr(Float32 xn[], Float32 y1[], Float32 g_corr[]);
void E_ACELP_xh_corr(Float32 *x, Float32 *y, Float32 *h);
void E_ACELP_codebook_target_update(Float32 *x, Float32 *x2, Float32 *y,
                                    Float32 gain);
void E_ACELP_2t(Float32 dn[], Float32 cn[], Float32 H[], Word16 code[],
                Float32 y[], Word32 *index);
void E_ACELP_4t(Float32 dn[], Float32 cn[], Float32 H[], Word16 code[],
                Float32 y[], Word32 nbbits, Word16 mode, Word32 _index[]);
Word32 E_ACELP_gains_quantise(Word16 code[], Word32 nbits, Float32 f_gain_pit,
                              Word16 *gain_pit, Word32 *gain_code,
                              Float32 *coeff, Word32 gp_clip,
                              Word16 *past_qua_en);

#endif
