/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef DEC_IF_H
#define DEC_IF_H

#include "typedef.h"

#define NB_SERIAL_MAX   61    /* max serial size      */
#define L_FRAME16k      320   /* Frame size at 16kHz  */

#define _good_frame  0
#define _bad_frame   1
#define _lost_frame  2
#define _no_frame    3

void D_IF_decode(void *st, UWord8 *bits, Word16 *synth, Word32 bfi);
void * D_IF_init(void);
void D_IF_exit(void *state);

#endif
