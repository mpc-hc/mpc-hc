/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef WB_E_IF_H
#define WB_E_IF_H

#include "typedef.h"

#define L_FRAME16k   320   /* Frame size at 16kHz  */
#define NB_SERIAL_MAX 61   /* max serial size      */

int E_IF_encode(void *st, Word16 mode, Word16 *speech,
                UWord8 *serial, Word16 dtx);
void *E_IF_init(void);
void E_IF_exit(void *state);

#endif
