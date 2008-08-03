/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

/*
 * interf_enc.h
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    Defines interface to AMR encoder
 *
 */

#ifndef _interf_enc_h_
#define _interf_enc_h_

/*
 * include files
 */
#include"sp_enc.h"

/*
 * Function prototypes
 */
/*
 * Encodes one frame of speech
 * Returns packed octets
 */
int Encoder_Interface_Encode( void *st, enum Mode mode, short *speech,

#ifndef ETSI
      unsigned char *serial,  /* max size 31 bytes */

#else
      short *serial, /* size 500 bytes */
#endif

      int forceSpeech );   /* use speech mode */

/*
 * Reserve and init. memory
 */
void *Encoder_Interface_init( int dtx );

/*
 * Exit and free memory
 */
void Encoder_Interface_exit( void *state );
#endif
