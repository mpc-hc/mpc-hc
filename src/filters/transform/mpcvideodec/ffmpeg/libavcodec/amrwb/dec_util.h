/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef DEC_UTIL_H
#define DEC_UTIL_H

#include "typedef.h"
#include "dec_main.h"

Word16 D_UTIL_random(Word16 *seed);

Word32 D_UTIL_pow2(Word16 exponant, Word16 fraction);
Word16 D_UTIL_norm_l (Word32 L_var1);
Word16 D_UTIL_norm_s (Word16 var1);
Word32 D_UTIL_dot_product12(Word16 x[], Word16 y[], Word16 lg, Word16 *exp);
void D_UTIL_normalised_inverse_sqrt(Word32 *frac, Word16 *exp);
Word32 D_UTIL_inverse_sqrt(Word32 L_x);
void D_UTIL_log2(Word32 L_x, Word16 *exponent, Word16 *fraction);
void D_UTIL_l_extract(Word32 L_32, Word16 *hi, Word16 *lo);
Word32 D_UTIL_mpy_32_16 (Word16 hi, Word16 lo, Word16 n);
Word32 D_UTIL_mpy_32 (Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2);
Word16 D_UTIL_saturate(Word32 inp);
void D_UTIL_signal_up_scale(Word16 x[], Word16 lg, Word16 exp);
void D_UTIL_signal_down_scale(Word16 x[], Word16 lg, Word16 exp);
void D_UTIL_dec_synthesis(Word16 Aq[], Word16 exc[], Word16 Q_new,
                          Word16 synth16k[], Word16 prms, Word16 HfIsf[],
                          Word16 mode, Word16 newDTXState, Word16 bfi,
                          Decoder_State *st);
void D_UTIL_preemph(Word16 x[], Word16 mu, Word16 lg, Word16 *mem);

#endif
