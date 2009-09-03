/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef DEC_ACELP_H
#define DEC_ACELP_H

#include "typedef.h"

void D_ACELP_decode_2t(Word16 index, Word16 code[]);
void D_ACELP_decode_4t(Word16 index[], Word16 nbbits, Word16 code[]);
void D_ACELP_phase_dispersion(Word16 gain_code, Word16 gain_pit, Word16 code[],
                              Word16 mode, Word16 disp_mem[]);

#endif

