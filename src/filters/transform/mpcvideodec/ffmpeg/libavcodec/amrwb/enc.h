/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef ENC_H
#define ENC_H

#include "typedef.h"

Word16 E_MAIN_init(void **spe_state);
void E_MAIN_reset(void *st, Word16 reset_all);
Word16 E_MAIN_encode(Word16 * mode, Word16 input_sp[], Word16 prms[],
                     void *spe_state, Word16 allow_dtx);
void E_MAIN_close(void **spe_state);

#endif

