/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_UTIL_H
#define ENC_UTIL_H

#include "typedef.h"
#include "enc_main.h"

Word16 E_UTIL_saturate(Word32 inp);
Word32 E_UTIL_saturate_31(Word32 inp);
Word16 E_UTIL_random(Word16 *seed);
Word32 E_UTIL_mpy_32_16 (Word16 hi, Word16 lo, Word16 n);
Word32 E_UTIL_pow2(Word16 exponant, Word16 fraction);
void E_UTIL_log2_32 (Word32 L_x, Word16 *exponent, Word16 *fraction);
void E_UTIL_normalised_inverse_sqrt(Word32 *frac, Word16 *exp);
Word32 E_UTIL_dot_product12(Word16 x[], Word16 y[], Word32 lg, Word32 *exp);
Word16 E_UTIL_norm_s (Word16 var1);
Word16 E_UTIL_norm_l (Word32 L_var1);
void E_UTIL_l_extract(Word32 L_32, Word16 *hi, Word16 *lo);
void E_UTIL_hp50_12k8(Float32 signal[], Word32 lg, Float32 mem[]);
void E_UTIL_preemph(Word16 x[], Word16 mu, Word32 lg, Word16 *mem);
void E_UTIL_f_preemph(Float32 *signal, Float32 mu, Word32 L, Float32 *mem);
void E_UTIL_deemph(Float32 *signal, Float32 mu, Word32 L, Float32 *mem);
void E_UTIL_deemph_32(Word16 x_hi[], Word16 x_lo[], Word16 y[],
                      Word16 mu, Word16 L, Word16 *mem);
void E_UTIL_decim_12k8(Float32 sig16k[], Word32 lg, Float32 sig12k8[],
                       Float32 mem[]);
void E_UTIL_residu(Float32 *a, Float32 *x, Float32 *y, Word32 l);
void E_UTIL_convolve(Word16 x[], Word16 q, Float32 h[], Float32 y[]);
void E_UTIL_f_convolve(Float32 x[], Float32 h[], Float32 y[]);
void E_UTIL_autocorr(Float32 *x, Float32 *r);
void E_UTIL_signal_up_scale(Word16 x[], Word16 exp);
void E_UTIL_signal_down_scale(Word16 x[], Word32 lg, Word16 exp);
void E_UTIL_synthesis(Float32 a[], Float32 x[], Float32 y[],
                      Word32 l, Float32 mem[], Word32 update_m);
Word32 E_UTIL_enc_synthesis(Float32 Aq[], Float32 exc[], Float32 synth16k[],
                            Coder_State *st);


#endif
