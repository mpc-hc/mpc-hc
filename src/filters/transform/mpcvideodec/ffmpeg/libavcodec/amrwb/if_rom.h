/*
 *===================================================================
 *  3GPP AMR Wideband Floating-point Speech Codec
 *===================================================================
 */
#ifndef IF_ROM_H
#define IF_ROM_H

#include "typedef.h"

/*
 * definition of constants
 */
#define NUM_OF_SPMODES 9

/* number of parameters */
#define PRMNO_7k 18
#define PRMNO_9k 32
#define PRMNO_12k 36
#define PRMNO_14k 36
#define PRMNO_16k 36
#define PRMNO_18k 52
#define PRMNO_20k 52
#define PRMNO_23k 52
#define PRMNO_24k 56
#define PRMNO_SID 7
#define NB_PARM_MAX PRMNO_24k

/* number of bits */
#ifndef NBBITS_7k
#define NBBITS_7k 132
#define NBBITS_9k 177
#define NBBITS_12k 253
#define NBBITS_14k 285
#define NBBITS_16k 317
#define NBBITS_18k 365
#define NBBITS_20k 397
#define NBBITS_23k 461
#define NBBITS_24k 477
#define NBBITS_SID 35
#endif

/* number of total bits */
#define HEADER_SIZE 6   /* real size + 1 */
#define T_NBBITS_7k (NBBITS_7k + HEADER_SIZE)
#define T_NBBITS_9k (NBBITS_9k + HEADER_SIZE)
#define T_NBBITS_12k (NBBITS_12k + HEADER_SIZE)
#define T_NBBITS_14k (NBBITS_14k + HEADER_SIZE)
#define T_NBBITS_16k (NBBITS_16k + HEADER_SIZE)
#define T_NBBITS_18k (NBBITS_18k + HEADER_SIZE)
#define T_NBBITS_20k (NBBITS_20k + HEADER_SIZE)
#define T_NBBITS_23k (NBBITS_23k + HEADER_SIZE)
#define T_NBBITS_24k (NBBITS_24k + HEADER_SIZE)
#define T_NBBITS_SID (NBBITS_SID + HEADER_SIZE)

#define TX_SPEECH 0
#define TX_SID_FIRST 1
#define TX_SID_UPDATE 2
#define TX_NO_DATA 3

#define RX_SPEECH_GOOD              0
#define RX_SPEECH_PROBABLY_DEGRADED 1
#define RX_SPEECH_LOST              2
#define RX_SPEECH_BAD               3
#define RX_SID_FIRST                4
#define RX_SID_UPDATE               5
#define RX_SID_BAD                  6
#define RX_NO_DATA                  7

#endif
