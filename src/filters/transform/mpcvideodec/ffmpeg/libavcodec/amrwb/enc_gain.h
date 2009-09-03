/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_GAIN_H
#define ENC_GAIN_H

#include "typedef.h"

void E_GAIN_clip_init(Float32 mem[]);
Word32 E_GAIN_clip_test(Float32 mem[]);
void E_GAIN_clip_isf_test(Float32 isf[], Float32 mem[]);
void E_GAIN_clip_pit_test(Float32 gain_pit, Float32 mem[]);
void E_GAIN_lp_decim2(Float32 x[], Word32 l, Float32 *mem);
Word32 E_GAIN_olag_median(Word32 prev_ol_lag, Word32 old_ol_lag[5]);
Word32 E_GAIN_open_loop_search(Float32 *wsp, Word32 L_min, Word32 L_max,
                               Word32 nFrame, Word32 L_0, Float32 *gain,
                               Float32 *mem, Float32 hp_old_wsp[],
                               UWord8 weight_flg);
Word32 E_GAIN_closed_loop_search(Float32 exc[], Float32 xn[], Float32 h[],
                                 Word32 t0_min, Word32 t0_max,
                                 Word32 *pit_frac, Word32 i_subfr,
                                 Word32 t0_fr2, Word32 t0_fr1);
void E_GAIN_adaptive_codebook_excitation(Word16 exc[], Word16 T0, Word32 frac,
                                         Word16 L_subfr);
void E_GAIN_pitch_sharpening(Word16 *x, Word16 pit_lag);
void E_GAIN_f_pitch_sharpening(Float32 *x, Word32 pit_lag);
Word32 E_GAIN_voice_factor(Word16 exc[], Word16 Q_exc, Word16 gain_pit,
                           Word16 code[], Word16 gain_code);

#endif

