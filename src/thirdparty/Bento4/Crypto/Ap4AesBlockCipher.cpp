/*
* AES Block cipher
* (c) 2005 Gilles Boccon-Gibod
* Portions (c) 2001, Dr Brian Gladman (see below)
*/

/*
-------------------------------------------------------------------------
Copyright (c) 2001, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
All rights reserved.

LICENSE TERMS

The free distribution and use of this software in both source and binary 
form is allowed (with or without changes) provided that:

1. distributions of this source code include the above copyright 
notice, this list of conditions and the following disclaimer;

2. distributions in binary form include the above copyright
notice, this list of conditions and the following disclaimer
in the documentation and/or other associated materials;

3. the copyright holder's name is not used to endorse products 
built using this software without specific written permission. 

DISCLAIMER

This software is provided 'as is' with no explicit or implied warranties
in respect of its properties, including, but not limited to, correctness 
and fitness for purpose.
-------------------------------------------------------------------------
Issue Date: 29/07/2002
*/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4AesBlockCipher.h"
#include "Ap4Results.h"

/*----------------------------------------------------------------------
|       build options
+---------------------------------------------------------------------*/
#define ENCRYPTION_KEY_SCHEDULE
#define ENCRYPTION
#define BLOCK_SIZE AP4_AES_BLOCK_SIZE

/*----------------------------------------------------------------------
|       options
+---------------------------------------------------------------------*/
/*  START OF CONFIGURATION OPTIONS

    USE OF DEFINES
  
    Later in this section there are a number of defines that control the 
    operation of the code.  In each section, the purpose of each define is 
    explained so that the relevant form can be included or excluded by 
    setting either 1's or 0's respectively on the branches of the related 
    #if clauses.
*/

/*  1. BYTE ORDER IN 32-BIT WORDS

    To obtain the highest speed on processors with 32-bit words, this code 
    needs to determine the order in which bytes are packed into such words.
    The following block of code is an attempt to capture the most obvious 
    ways in which various environemnts define byte order. It may well fail, 
    in which case the definitions will need to be set by editing at the 
    points marked **** EDIT HERE IF NECESSARY **** below.
*/
#define AES_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define AES_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */

#if !defined(AP4_PLATFORM_BYTE_ORDER)
#  error AP4_PLATFORM_BYTE_ORDER is not set
#endif

#if 0
#if AP4_PLATFORM_BYTE_ORDER == AP4_PLATFORM_BIG_ENDIAN
#define PLATFORM_BYTE_ORDER AES_BIG_ENDIAN
#elif AP4_PLATFORM_BYTE_ORDER == AP4_PLATFORM_LITTLE_ENDIAN
#define PLATFORM_BYTE_ORDER AES_LITTLE_ENDIAN
#else
#error unsupported value for AP4_PLATFORM_BYTE_ORDER
#endif
#endif

#define PLATFORM_BYTE_ORDER AES_LITTLE_ENDIAN


/*  2. BYTE ORDER WITHIN 32 BIT WORDS

    The fundamental data processing units in Rijndael are 8-bit bytes. The 
    input, output and key input are all enumerated arrays of bytes in which 
    bytes are numbered starting at zero and increasing to one less than the 
    number of bytes in the array in question. This enumeration is only used 
    for naming bytes and does not imply any adjacency or order relationship 
    from one byte to another. When these inputs and outputs are considered 
    as bit sequences, bits 8*n to 8*n+7 of the bit sequence are mapped to 
    byte[n] with bit 8n+i in the sequence mapped to bit 7-i within the byte. 
    In this implementation bits are numbered from 0 to 7 starting at the 
    numerically least significant end of each byte (bit n represents 2^n).

    However, Rijndael can be implemented more efficiently using 32-bit 
    words by packing bytes into words so that bytes 4*n to 4*n+3 are placed
    into word[n]. While in principle these bytes can be assembled into words 
    in any positions, this implementation only supports the two formats in 
    which bytes in adjacent positions within words also have adjacent byte
    numbers. This order is called big-endian if the lowest numbered bytes 
    in words have the highest numeric significance and little-endian if the 
    opposite applies. 
    
    This code can work in either order irrespective of the order used by the 
    machine on which it runs. Normally the internal byte order will be set
    to the order of the processor on which the code is to be run but this
    define can be used to reverse this in special situations
*/
#if 1
#define INTERNAL_BYTE_ORDER PLATFORM_BYTE_ORDER
#elif defined(AES_LITTLE_ENDIAN)
#define INTERNAL_BYTE_ORDER AES_LITTLE_ENDIAN
#elif defined(AES_BIG_ENDIAN)
#define INTERNAL_BYTE_ORDER AES_BIG_ENDIAN
#endif

/*  3. FAST INPUT/OUTPUT OPERATIONS.  

    On some machines it is possible to improve speed by transferring the 
    bytes in the input and output arrays to and from the internal 32-bit 
    variables by addressing these arrays as if they are arrays of 32-bit 
    words.  On some machines this will always be possible but there may 
    be a large performance penalty if the byte arrays are not aligned on 
    the normal word boundaries. On other machines this technique will 
    lead to memory access errors when such 32-bit word accesses are not
    properly aligned. The option SAFE_IO avoids such problems but will 
    often be slower on those machines that support misaligned access 
    (especially so if care is taken to align the input  and output byte 
    arrays on 32-bit word boundaries). If SAFE_IO is not defined it is 
    assumed that access to byte arrays as if they are arrays of 32-bit 
    words will not cause problems when such accesses are misaligned.
*/
#if 1
#define SAFE_IO
#endif

/*  4. LOOP UNROLLING

    The code for encryption and decrytpion cycles through a number of rounds
    that can be implemented either in a loop or by expanding the code into a 
    long sequence of instructions, the latter producing a larger program but
    one that will often be much faster. The latter is called loop unrolling.
    There are also potential speed advantages in expanding two iterations in
    a loop with half the number of iterations, which is called partial loop
    unrolling.  The following options allow partial or full loop unrolling 
    to be set independently for encryption and decryption
*/
#if 1
#define ENC_UNROLL  FULL
#elif 0
#define ENC_UNROLL  PARTIAL
#else
#define ENC_UNROLL  NONE
#endif

#if 1
#define DEC_UNROLL  FULL
#elif 0
#define DEC_UNROLL  PARTIAL
#else
#define DEC_UNROLL  NONE
#endif

/*  5. FIXED OR DYNAMIC TABLES

    When this section is included the tables used by the code are comipled 
    statically into the binary file.  Otherwise they are computed once when 
    the code is first used.
*/
#if 1
#define FIXED_TABLES
#endif

/*  6. FAST FINITE FIELD OPERATIONS

    If this section is included, tables are used to provide faster finite 
    field arithmetic (this has no effect if FIXED_TABLES is defined).
*/
#if 1
#define FF_TABLES
#endif

/*  7. INTERNAL STATE VARIABLE FORMAT

    The internal state of Rijndael is stored in a number of local 32-bit 
    word varaibles which can be defined either as an array or as individual 
    names variables. Include this section if you want to store these local
    variables in arrays. Otherwise individual local variables will be used.
*/
#if 1
#define ARRAYS
#endif

/* In this implementation the columns of the state array are each held in
   32-bit words. The state array can be held in various ways: in an array
   of words, in a number of individual word variables or in a number of 
   processor registers. The following define maps a variable name x and
   a column number c to the way the state array variable is to be held.
   The first define below maps the state into an array x[c] whereas the 
   second form maps the state into a number of individual variables x0,
   x1, etc.  Another form could map individual state colums to machine
   register names.
*/

#if defined(ARRAYS)
#define s(x,c) x[c]
#else
#define s(x,c) x##c
#endif

/*  8. VARIABLE BLOCK SIZE SPEED

    This section is only relevant if you wish to use the variable block
    length feature of the code.  Include this section if you place more
    emphasis on speed rather than code size.
*/
#if 1
#define FAST_VARIABLE
#endif

/*  9. INTERNAL TABLE CONFIGURATION

    This cipher proceeds by repeating in a number of cycles known as 'rounds'
    which are implemented by a round function which can optionally be speeded
    up using tables.  The basic tables are each 256 32-bit words, with either 
    one or four tables being required for each round function depending on
    how much speed is required. The encryption and decryption round functions
    are different and the last encryption and decrytpion round functions are
    different again making four different round functions in all.

    This means that:
      1. Normal encryption and decryption rounds can each use either 0, 1 
         or 4 tables and table spaces of 0, 1024 or 4096 bytes each.
      2. The last encryption and decryption rounds can also use either 0, 1 
         or 4 tables and table spaces of 0, 1024 or 4096 bytes each.

    Include or exclude the appropriate definitions below to set the number
    of tables used by this implementation.
*/

#if 1   /* set tables for the normal encryption round */
#define ENC_ROUND   FOUR_TABLES
#elif 0
#define ENC_ROUND   ONE_TABLE
#else
#define ENC_ROUND   NO_TABLES
#endif

#if 1   /* set tables for the last encryption round */
#define LAST_ENC_ROUND  FOUR_TABLES
#elif 0
#define LAST_ENC_ROUND  ONE_TABLE
#else
#define LAST_ENC_ROUND  NO_TABLES
#endif

#if 1   /* set tables for the normal decryption round */
#define DEC_ROUND   FOUR_TABLES
#elif 0
#define DEC_ROUND   ONE_TABLE
#else
#define DEC_ROUND   NO_TABLES
#endif

#if 1   /* set tables for the last decryption round */
#define LAST_DEC_ROUND  FOUR_TABLES
#elif 0
#define LAST_DEC_ROUND  ONE_TABLE
#else
#define LAST_DEC_ROUND  NO_TABLES
#endif

/*  The decryption key schedule can be speeded up with tables in the same
    way that the round functions can.  Include or exclude the following 
    defines to set this requirement.
*/
#if 1
#define KEY_SCHED   FOUR_TABLES
#elif 0
#define KEY_SCHED   ONE_TABLE
#else
#define KEY_SCHED   NO_TABLES
#endif

/* END OF CONFIGURATION OPTIONS */

#define NO_TABLES   0   /* DO NOT CHANGE */
#define ONE_TABLE   1   /* DO NOT CHANGE */
#define FOUR_TABLES 4   /* DO NOT CHANGE */
#define NONE        0   /* DO NOT CHANGE */
#define PARTIAL     1   /* DO NOT CHANGE */
#define FULL        2   /* DO NOT CHANGE */

#if defined(BLOCK_SIZE) && ((BLOCK_SIZE & 3) || BLOCK_SIZE < 16 || BLOCK_SIZE > 32)
#error An illegal block size has been specified.
#endif  

#if !defined(BLOCK_SIZE)
#define RC_LENGTH    29
#else
#define RC_LENGTH   5 * BLOCK_SIZE / 4 - (BLOCK_SIZE == 16 ? 10 : 11)
#endif

/* Disable at least some poor combinations of options */

#if ENC_ROUND == NO_TABLES && LAST_ENC_ROUND != NO_TABLES
#undef  LAST_ENC_ROUND
#define LAST_ENC_ROUND  NO_TABLES
#elif ENC_ROUND == ONE_TABLE && LAST_ENC_ROUND == FOUR_TABLES
#undef  LAST_ENC_ROUND
#define LAST_ENC_ROUND  ONE_TABLE 
#endif

#if ENC_ROUND == NO_TABLES && ENC_UNROLL != NONE
#undef  ENC_UNROLL
#define ENC_UNROLL  NONE
#endif

#if DEC_ROUND == NO_TABLES && LAST_DEC_ROUND != NO_TABLES
#undef  LAST_DEC_ROUND
#define LAST_DEC_ROUND  NO_TABLES
#elif DEC_ROUND == ONE_TABLE && LAST_DEC_ROUND == FOUR_TABLES
#undef  LAST_DEC_ROUND
#define LAST_DEC_ROUND  ONE_TABLE 
#endif

#if DEC_ROUND == NO_TABLES && DEC_UNROLL != NONE
#undef  DEC_UNROLL
#define DEC_UNROLL  NONE
#endif

/*  upr(x,n):  rotates bytes within words by n positions, moving bytes to
               higher index positions with wrap around into low positions
    ups(x,n):  moves bytes by n positions to higher index positions in 
               words but without wrap around
    bval(x,n): extracts a byte from a word

    NOTE:      The definitions given here are intended only for use with 
               unsigned variables and with shift counts that are compile
               time constants
*/

#if (INTERNAL_BYTE_ORDER == AES_LITTLE_ENDIAN)
#if defined(_MSC_VER)
#define upr(x,n)        _lrotl((aes_32t)(x), 8 * (n))
#else
#define upr(x,n)        ((aes_32t)(x) << 8 * (n) | (aes_32t)(x) >> (32 - 8 * (n)))
#endif
#define ups(x,n)        ((aes_32t)(x) << 8 * (n))
#define bval(x,n)       ((aes_08t)((x) >> 8 * (n)))
#define bytes2word(b0, b1, b2, b3)  \
        (((aes_32t)(b3) << 24) | ((aes_32t)(b2) << 16) | ((aes_32t)(b1) << 8) | (b0))
#endif

#if (INTERNAL_BYTE_ORDER == AES_BIG_ENDIAN)
#define upr(x,n)        ((aes_32t)(x) >> 8 * (n) | (aes_32t)(x) << 32 - 8 * (n))
#define ups(x,n)        ((aes_32t)(x) >> 8 * (n)))
#define bval(x,n)       ((aes_08t)((x) >> (24 - 8 * (n))))
#define bytes2word(b0, b1, b2, b3)  \
        (((aes_32t)(b0) << 24) | ((aes_32t)(b1) << 16) | ((aes_32t)(b2) << 8) | (b3))
#endif

#if defined(SAFE_IO)

#define word_in(x)      bytes2word((x)[0], (x)[1], (x)[2], (x)[3])
#define word_out(x,v)   { (x)[0] = bval(v,0); (x)[1] = bval(v,1);   \
                          (x)[2] = bval(v,2); (x)[3] = bval(v,3);   }

#elif (INTERNAL_BYTE_ORDER == PLATFORM_BYTE_ORDER)

#define word_in(x)      *(aes_32t*)(x)
#define word_out(x,v)   *(aes_32t*)(x) = (v)

#else

#if !defined(bswap_32)
#if !defined(_MSC_VER)
#define _lrotl(x,n)     ((((aes_32t)(x)) <<  n) | (((aes_32t)(x)) >> (32 - n)))
#endif
#define bswap_32(x)     ((_lrotl((x),8) & 0x00ff00ff) | (_lrotl((x),24) & 0xff00ff00)) 
#endif

#define word_in(x)      bswap_32(*(aes_32t*)(x))
#define word_out(x,v)   *(aes_32t*)(x) = bswap_32(v)

#endif

/* the finite field modular polynomial and elements */

#define WPOLY   0x011b
#define BPOLY     0x1b

/* multiply four bytes in GF(2^8) by 'x' {02} in parallel */

#define m1  0x80808080
#define m2  0x7f7f7f7f
#define FFmulX(x)  ((((x) & m2) << 1) ^ ((((x) & m1) >> 7) * BPOLY))

/* The following defines provide alternative definitions of FFmulX that might
   give improved performance if a fast 32-bit multiply is not available. Note
   that a temporary variable u needs to be defined where FFmulX is used.

#define FFmulX(x) (u = (x) & m1, u |= (u >> 1), ((x) & m2) << 1) ^ ((u >> 3) | (u >> 6)) 
#define m4  (0x01010101 * BPOLY)
#define FFmulX(x) (u = (x) & m1, ((x) & m2) << 1) ^ ((u - (u >> 7)) & m4) 
*/

/* Work out which tables are needed for the different options   */

#ifdef  AES_ASM
#ifdef  ENC_ROUND
#undef  ENC_ROUND
#endif
#define ENC_ROUND   FOUR_TABLES
#ifdef  LAST_ENC_ROUND
#undef  LAST_ENC_ROUND
#endif
#define LAST_ENC_ROUND  FOUR_TABLES
#ifdef  DEC_ROUND
#undef  DEC_ROUND
#endif
#define DEC_ROUND   FOUR_TABLES
#ifdef  LAST_DEC_ROUND
#undef  LAST_DEC_ROUND
#endif
#define LAST_DEC_ROUND  FOUR_TABLES
#ifdef  KEY_SCHED
#undef  KEY_SCHED
#define KEY_SCHED   FOUR_TABLES
#endif
#endif

#if defined(ENCRYPTION) || defined(AES_ASM)
#if ENC_ROUND == ONE_TABLE
#define FT1_SET
#elif ENC_ROUND == FOUR_TABLES
#define FT4_SET
#else
#define SBX_SET
#endif
#if LAST_ENC_ROUND == ONE_TABLE
#define FL1_SET
#elif LAST_ENC_ROUND == FOUR_TABLES
#define FL4_SET
#elif !defined(SBX_SET)
#define SBX_SET
#endif
#endif

#if defined(DECRYPTION) || defined(AES_ASM)
#if DEC_ROUND == ONE_TABLE
#define IT1_SET
#elif DEC_ROUND == FOUR_TABLES
#define IT4_SET
#else
#define ISB_SET
#endif
#if LAST_DEC_ROUND == ONE_TABLE
#define IL1_SET
#elif LAST_DEC_ROUND == FOUR_TABLES
#define IL4_SET
#elif !defined(ISB_SET)
#define ISB_SET
#endif
#endif

#if defined(ENCRYPTION_KEY_SCHEDULE) || defined(DECRYPTION_KEY_SCHEDULE)
#if KEY_SCHED == ONE_TABLE
#define LS1_SET
#define IM1_SET
#elif KEY_SCHED == FOUR_TABLES
#define LS4_SET
#define IM4_SET
#elif !defined(SBX_SET)
#define SBX_SET
#endif
#endif

#ifdef  FIXED_TABLES
#define prefx   static const
#else
#define prefx   extern
extern aes_08t  tab_init;
void gen_tabs(void);
#endif

//prefx aes_32t  rcon_tab[29];
//
//#ifdef  SBX_SET
//prefx aes_08t s_box[256];
//#endif
//
//#ifdef  ISB_SET
//prefx aes_08t inv_s_box[256];
//#endif
//
//#ifdef  FT1_SET
//prefx aes_32t ft_tab[256];
//#endif
//
//#ifdef  FT4_SET
//prefx aes_32t ft_tab[4][256];
//#endif
//
//#ifdef  FL1_SET
//prefx aes_32t fl_tab[256];
//#endif
//
//#ifdef  FL4_SET
//prefx aes_32t fl_tab[4][256];
//#endif
//
//#ifdef  IT1_SET
//prefx aes_32t it_tab[256];
//#endif
//
//#ifdef  IT4_SET
//prefx aes_32t it_tab[4][256];
//#endif
//
//#ifdef  IL1_SET
//prefx aes_32t il_tab[256];
//#endif
//
//#ifdef  IL4_SET
//prefx aes_32t il_tab[4][256];
//#endif
//
//#ifdef  LS1_SET
//#ifdef  FL1_SET
//#undef  LS1_SET
//#else
//prefx aes_32t ls_tab[256];
//#endif
//#endif
//
//#ifdef  LS4_SET
//#ifdef  FL4_SET
//#undef  LS4_SET
//#else
//prefx aes_32t ls_tab[4][256];
//#endif
//#endif
//
//#ifdef  IM1_SET
//prefx aes_32t im_tab[256];
//#endif
//
//#ifdef  IM4_SET
//prefx aes_32t im_tab[4][256];
//#endif

/* Set the number of columns in nc.  Note that it is important
   that nc is a constant which is known at compile time if the
   highest speed version of the code is needed.
*/

#if defined(BLOCK_SIZE)
#define nc  (BLOCK_SIZE >> 2)
#else
#define nc  (cx->n_blk >> 2)
#endif

/* generic definitions of Rijndael macros that use tables    */

#define no_table(x,box,vf,rf,c) bytes2word( \
    box[bval(vf(x,0,c),rf(0,c))], \
    box[bval(vf(x,1,c),rf(1,c))], \
    box[bval(vf(x,2,c),rf(2,c))], \
    box[bval(vf(x,3,c),rf(3,c))])

#define one_table(x,op,tab,vf,rf,c) \
 (     tab[bval(vf(x,0,c),rf(0,c))] \
  ^ op(tab[bval(vf(x,1,c),rf(1,c))],1) \
  ^ op(tab[bval(vf(x,2,c),rf(2,c))],2) \
  ^ op(tab[bval(vf(x,3,c),rf(3,c))],3))

#define four_tables(x,tab,vf,rf,c) \
 (  tab[0][bval(vf(x,0,c),rf(0,c))] \
  ^ tab[1][bval(vf(x,1,c),rf(1,c))] \
  ^ tab[2][bval(vf(x,2,c),rf(2,c))] \
  ^ tab[3][bval(vf(x,3,c),rf(3,c))])

#define vf1(x,r,c)  (x)
#define rf1(r,c)    (r)
#define rf2(r,c)    ((r-c)&3)

/* perform forward and inverse column mix operation on four bytes in long word x in */
/* parallel. NOTE: x must be a simple variable, NOT an expression in these macros.  */

#define dec_fmvars
#if defined(FM4_SET)    /* not currently used */
#define fwd_mcol(x)     four_tables(x,fm_tab,vf1,rf1,0)
#elif defined(FM1_SET)  /* not currently used */
#define fwd_mcol(x)     one_table(x,upr,fm_tab,vf1,rf1,0)
#else
#undef  dec_fmvars
#define dec_fmvars      aes_32t f1, f2;
#define fwd_mcol(x)     (f1 = (x), f2 = FFmulX(f1), f2 ^ upr(f1 ^ f2, 3) ^ upr(f1, 2) ^ upr(f1, 1))
#endif

#define dec_imvars
#if defined(IM4_SET)
#define inv_mcol(x)     four_tables(x,im_tab,vf1,rf1,0)
#elif defined(IM1_SET)
#define inv_mcol(x)     one_table(x,upr,im_tab,vf1,rf1,0)
#else
#undef  dec_imvars
#define dec_imvars      aes_32t    f2, f4, f8, f9;
#define inv_mcol(x) \
    (f9 = (x), f2 = FFmulX(f9), f4 = FFmulX(f2), f8 = FFmulX(f4), f9 ^= f8, \
    f2 ^= f4 ^ f8 ^ upr(f2 ^ f9,3) ^ upr(f4 ^ f9,2) ^ upr(f9,1))
#endif

#if defined(FL4_SET)
#define ls_box(x,c)     four_tables(x,fl_tab,vf1,rf2,c)
#elif   defined(LS4_SET)
#define ls_box(x,c)     four_tables(x,ls_tab,vf1,rf2,c)
#elif defined(FL1_SET)
#define ls_box(x,c)     one_table(x,upr,fl_tab,vf1,rf2,c)
#elif defined(LS1_SET)
#define ls_box(x,c)     one_table(x,upr,ls_tab,vf1,rf2,c)
#else
#define ls_box(x,c)     no_table(x,s_box,vf1,rf2,c)
#endif

/*----------------------------------------------------------------------
|       tables
+---------------------------------------------------------------------*/
#if defined(FIXED_TABLES) || !defined(FF_TABLES) 

/*  finite field arithmetic operations */

#define f2(x)   ((x<<1) ^ (((x>>7) & 1) * WPOLY))
#define f4(x)   ((x<<2) ^ (((x>>6) & 1) * WPOLY) ^ (((x>>6) & 2) * WPOLY))
#define f8(x)   ((x<<3) ^ (((x>>5) & 1) * WPOLY) ^ (((x>>5) & 2) * WPOLY) \
                        ^ (((x>>5) & 4) * WPOLY))
#define f3(x)   (f2(x) ^ x)
#define f9(x)   (f8(x) ^ x)
#define fb(x)   (f8(x) ^ f2(x) ^ x)
#define fd(x)   (f8(x) ^ f4(x) ^ x)
#define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

#endif

#if defined(FIXED_TABLES)

#define sb_data(w) \
    w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
    w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
    w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
    w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
    w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
    w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
    w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
    w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
    w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
    w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
    w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
    w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
    w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
    w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
    w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
    w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
    w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
    w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
    w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
    w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
    w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
    w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
    w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
    w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
    w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
    w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
    w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
    w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
    w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
    w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
    w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
    w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16)

#define isb_data(w) \
    w(0x52), w(0x09), w(0x6a), w(0xd5), w(0x30), w(0x36), w(0xa5), w(0x38),\
    w(0xbf), w(0x40), w(0xa3), w(0x9e), w(0x81), w(0xf3), w(0xd7), w(0xfb),\
    w(0x7c), w(0xe3), w(0x39), w(0x82), w(0x9b), w(0x2f), w(0xff), w(0x87),\
    w(0x34), w(0x8e), w(0x43), w(0x44), w(0xc4), w(0xde), w(0xe9), w(0xcb),\
    w(0x54), w(0x7b), w(0x94), w(0x32), w(0xa6), w(0xc2), w(0x23), w(0x3d),\
    w(0xee), w(0x4c), w(0x95), w(0x0b), w(0x42), w(0xfa), w(0xc3), w(0x4e),\
    w(0x08), w(0x2e), w(0xa1), w(0x66), w(0x28), w(0xd9), w(0x24), w(0xb2),\
    w(0x76), w(0x5b), w(0xa2), w(0x49), w(0x6d), w(0x8b), w(0xd1), w(0x25),\
    w(0x72), w(0xf8), w(0xf6), w(0x64), w(0x86), w(0x68), w(0x98), w(0x16),\
    w(0xd4), w(0xa4), w(0x5c), w(0xcc), w(0x5d), w(0x65), w(0xb6), w(0x92),\
    w(0x6c), w(0x70), w(0x48), w(0x50), w(0xfd), w(0xed), w(0xb9), w(0xda),\
    w(0x5e), w(0x15), w(0x46), w(0x57), w(0xa7), w(0x8d), w(0x9d), w(0x84),\
    w(0x90), w(0xd8), w(0xab), w(0x00), w(0x8c), w(0xbc), w(0xd3), w(0x0a),\
    w(0xf7), w(0xe4), w(0x58), w(0x05), w(0xb8), w(0xb3), w(0x45), w(0x06),\
    w(0xd0), w(0x2c), w(0x1e), w(0x8f), w(0xca), w(0x3f), w(0x0f), w(0x02),\
    w(0xc1), w(0xaf), w(0xbd), w(0x03), w(0x01), w(0x13), w(0x8a), w(0x6b),\
    w(0x3a), w(0x91), w(0x11), w(0x41), w(0x4f), w(0x67), w(0xdc), w(0xea),\
    w(0x97), w(0xf2), w(0xcf), w(0xce), w(0xf0), w(0xb4), w(0xe6), w(0x73),\
    w(0x96), w(0xac), w(0x74), w(0x22), w(0xe7), w(0xad), w(0x35), w(0x85),\
    w(0xe2), w(0xf9), w(0x37), w(0xe8), w(0x1c), w(0x75), w(0xdf), w(0x6e),\
    w(0x47), w(0xf1), w(0x1a), w(0x71), w(0x1d), w(0x29), w(0xc5), w(0x89),\
    w(0x6f), w(0xb7), w(0x62), w(0x0e), w(0xaa), w(0x18), w(0xbe), w(0x1b),\
    w(0xfc), w(0x56), w(0x3e), w(0x4b), w(0xc6), w(0xd2), w(0x79), w(0x20),\
    w(0x9a), w(0xdb), w(0xc0), w(0xfe), w(0x78), w(0xcd), w(0x5a), w(0xf4),\
    w(0x1f), w(0xdd), w(0xa8), w(0x33), w(0x88), w(0x07), w(0xc7), w(0x31),\
    w(0xb1), w(0x12), w(0x10), w(0x59), w(0x27), w(0x80), w(0xec), w(0x5f),\
    w(0x60), w(0x51), w(0x7f), w(0xa9), w(0x19), w(0xb5), w(0x4a), w(0x0d),\
    w(0x2d), w(0xe5), w(0x7a), w(0x9f), w(0x93), w(0xc9), w(0x9c), w(0xef),\
    w(0xa0), w(0xe0), w(0x3b), w(0x4d), w(0xae), w(0x2a), w(0xf5), w(0xb0),\
    w(0xc8), w(0xeb), w(0xbb), w(0x3c), w(0x83), w(0x53), w(0x99), w(0x61),\
    w(0x17), w(0x2b), w(0x04), w(0x7e), w(0xba), w(0x77), w(0xd6), w(0x26),\
    w(0xe1), w(0x69), w(0x14), w(0x63), w(0x55), w(0x21), w(0x0c), w(0x7d),

#define mm_data(w) \
    w(0x00), w(0x01), w(0x02), w(0x03), w(0x04), w(0x05), w(0x06), w(0x07),\
    w(0x08), w(0x09), w(0x0a), w(0x0b), w(0x0c), w(0x0d), w(0x0e), w(0x0f),\
    w(0x10), w(0x11), w(0x12), w(0x13), w(0x14), w(0x15), w(0x16), w(0x17),\
    w(0x18), w(0x19), w(0x1a), w(0x1b), w(0x1c), w(0x1d), w(0x1e), w(0x1f),\
    w(0x20), w(0x21), w(0x22), w(0x23), w(0x24), w(0x25), w(0x26), w(0x27),\
    w(0x28), w(0x29), w(0x2a), w(0x2b), w(0x2c), w(0x2d), w(0x2e), w(0x2f),\
    w(0x30), w(0x31), w(0x32), w(0x33), w(0x34), w(0x35), w(0x36), w(0x37),\
    w(0x38), w(0x39), w(0x3a), w(0x3b), w(0x3c), w(0x3d), w(0x3e), w(0x3f),\
    w(0x40), w(0x41), w(0x42), w(0x43), w(0x44), w(0x45), w(0x46), w(0x47),\
    w(0x48), w(0x49), w(0x4a), w(0x4b), w(0x4c), w(0x4d), w(0x4e), w(0x4f),\
    w(0x50), w(0x51), w(0x52), w(0x53), w(0x54), w(0x55), w(0x56), w(0x57),\
    w(0x58), w(0x59), w(0x5a), w(0x5b), w(0x5c), w(0x5d), w(0x5e), w(0x5f),\
    w(0x60), w(0x61), w(0x62), w(0x63), w(0x64), w(0x65), w(0x66), w(0x67),\
    w(0x68), w(0x69), w(0x6a), w(0x6b), w(0x6c), w(0x6d), w(0x6e), w(0x6f),\
    w(0x70), w(0x71), w(0x72), w(0x73), w(0x74), w(0x75), w(0x76), w(0x77),\
    w(0x78), w(0x79), w(0x7a), w(0x7b), w(0x7c), w(0x7d), w(0x7e), w(0x7f),\
    w(0x80), w(0x81), w(0x82), w(0x83), w(0x84), w(0x85), w(0x86), w(0x87),\
    w(0x88), w(0x89), w(0x8a), w(0x8b), w(0x8c), w(0x8d), w(0x8e), w(0x8f),\
    w(0x90), w(0x91), w(0x92), w(0x93), w(0x94), w(0x95), w(0x96), w(0x97),\
    w(0x98), w(0x99), w(0x9a), w(0x9b), w(0x9c), w(0x9d), w(0x9e), w(0x9f),\
    w(0xa0), w(0xa1), w(0xa2), w(0xa3), w(0xa4), w(0xa5), w(0xa6), w(0xa7),\
    w(0xa8), w(0xa9), w(0xaa), w(0xab), w(0xac), w(0xad), w(0xae), w(0xaf),\
    w(0xb0), w(0xb1), w(0xb2), w(0xb3), w(0xb4), w(0xb5), w(0xb6), w(0xb7),\
    w(0xb8), w(0xb9), w(0xba), w(0xbb), w(0xbc), w(0xbd), w(0xbe), w(0xbf),\
    w(0xc0), w(0xc1), w(0xc2), w(0xc3), w(0xc4), w(0xc5), w(0xc6), w(0xc7),\
    w(0xc8), w(0xc9), w(0xca), w(0xcb), w(0xcc), w(0xcd), w(0xce), w(0xcf),\
    w(0xd0), w(0xd1), w(0xd2), w(0xd3), w(0xd4), w(0xd5), w(0xd6), w(0xd7),\
    w(0xd8), w(0xd9), w(0xda), w(0xdb), w(0xdc), w(0xdd), w(0xde), w(0xdf),\
    w(0xe0), w(0xe1), w(0xe2), w(0xe3), w(0xe4), w(0xe5), w(0xe6), w(0xe7),\
    w(0xe8), w(0xe9), w(0xea), w(0xeb), w(0xec), w(0xed), w(0xee), w(0xef),\
    w(0xf0), w(0xf1), w(0xf2), w(0xf3), w(0xf4), w(0xf5), w(0xf6), w(0xf7),\
    w(0xf8), w(0xf9), w(0xfa), w(0xfb), w(0xfc), w(0xfd), w(0xfe), w(0xff)

#define h0(x)   (x)

/*  These defines are used to ensure tables are generated in the 
    right format depending on the internal byte order required
*/

#define w0(p)   bytes2word(p, 0, 0, 0)
#define w1(p)   bytes2word(0, p, 0, 0)
#define w2(p)   bytes2word(0, 0, p, 0)
#define w3(p)   bytes2word(0, 0, 0, p)

/*  Number of elements required in this table for different
    block and key lengths is:

    Rcon Table      key length (bytes)
    Length          16  20  24  28  32
                ---------------------
    block     16 |  10   9   8   7   7
    length    20 |  14  11  10   9   9
    (bytes)   24 |  19  15  12  11  11
              28 |  24  19  16  13  13
              32 |  29  23  19  17  14

    this table can be a table of bytes if the key schedule
    code is adjusted accordingly
*/

#define u0(p)   bytes2word(f2(p), p, p, f3(p))
#define u1(p)   bytes2word(f3(p), f2(p), p, p)
#define u2(p)   bytes2word(p, f3(p), f2(p), p)
#define u3(p)   bytes2word(p, p, f3(p), f2(p))

#define v0(p)   bytes2word(fe(p), f9(p), fd(p), fb(p))
#define v1(p)   bytes2word(fb(p), fe(p), f9(p), fd(p))
#define v2(p)   bytes2word(fd(p), fb(p), fe(p), f9(p))
#define v3(p)   bytes2word(f9(p), fd(p), fb(p), fe(p))

static const aes_32t rcon_tab[29] =
{
    w0(0x01), w0(0x02), w0(0x04), w0(0x08),
    w0(0x10), w0(0x20), w0(0x40), w0(0x80),
    w0(0x1b), w0(0x36), w0(0x6c), w0(0xd8),
    w0(0xab), w0(0x4d), w0(0x9a), w0(0x2f),
    w0(0x5e), w0(0xbc), w0(0x63), w0(0xc6),
    w0(0x97), w0(0x35), w0(0x6a), w0(0xd4),
    w0(0xb3), w0(0x7d), w0(0xfa), w0(0xef),
    w0(0xc5)
};

#ifdef  SBX_SET
static const aes_08t s_box[256] = { sb_data(h0) };
#endif
#ifdef  ISB_SET
static const aes_08t inv_s_box[256] = { isb_data(h0) };
#endif

#ifdef  FT1_SET
static const aes_32t ft_tab[256] = { sb_data(u0) };
#endif
#ifdef  FT4_SET
static const aes_32t ft_tab[4][256] = 
    { {  sb_data(u0) }, {  sb_data(u1) }, {  sb_data(u2) }, {  sb_data(u3) } };
#endif

#ifdef  FL1_SET
static const aes_32t fl_tab[256] = { sb_data(w0) };
#endif
#ifdef  FL4_SET
static const aes_32t fl_tab[4][256] = 
    { {  sb_data(w0) }, {  sb_data(w1) }, {  sb_data(w2) }, {  sb_data(w3) } };
#endif

#ifdef  IT1_SET
static const aes_32t it_tab[256] = { isb_data(v0) };
#endif
#ifdef  IT4_SET
static const aes_32t it_tab[4][256] =
    { { isb_data(v0) }, { isb_data(v1) }, { isb_data(v2) }, { isb_data(v3) } };
#endif

#ifdef  IL1_SET
static const aes_32t il_tab[256] = { isb_data(w0) };
#endif
#ifdef  IL4_SET
static const aes_32t il_tab[4][256] = 
    { { isb_data(w0) }, { isb_data(w1) }, { isb_data(w2) }, { isb_data(w3) } };
#endif

#ifdef  LS1_SET
static const aes_32t ls_tab[256] = { sb_data(w0) };
#endif
#ifdef  LS4_SET
static const aes_32t ls_tab[4][256] =
    { {  sb_data(w0) }, {  sb_data(w1) }, {  sb_data(w2) }, {  sb_data(w3) } };
#endif

#ifdef  IM1_SET
static const aes_32t im_tab[256] = { mm_data(v0) };
#endif
#ifdef  IM4_SET
static const aes_32t im_tab[4][256] = 
    { {  mm_data(v0) }, {  mm_data(v1) }, {  mm_data(v2) }, {  mm_data(v3) } };
#endif

#else   /* dynamic table generation */

aes_08t tab_init = 0;

#define const

static aes_32t  rcon_tab[RC_LENGTH];

#ifdef  SBX_SET
aes_08t s_box[256];
#endif
#ifdef  ISB_SET
aes_08t inv_s_box[256];
#endif

#ifdef  FT1_SET
aes_32t ft_tab[256];
#endif
#ifdef  FT4_SET
aes_32t ft_tab[4][256];
#endif

#ifdef  FL1_SET
aes_32t fl_tab[256];
#endif
#ifdef  FL4_SET
aes_32t fl_tab[4][256];
#endif

#ifdef  IT1_SET
aes_32t it_tab[256];
#endif
#ifdef  IT4_SET
aes_32t it_tab[4][256];
#endif

#ifdef  IL1_SET
aes_32t il_tab[256];
#endif
#ifdef  IL4_SET
aes_32t il_tab[4][256];
#endif

#ifdef  LS1_SET
aes_32t ls_tab[256];
#endif
#ifdef  LS4_SET
aes_32t ls_tab[4][256];
#endif

#ifdef  IM1_SET
aes_32t im_tab[256];
#endif
#ifdef  IM4_SET
aes_32t im_tab[4][256];
#endif

#if !defined(FF_TABLES)

/*  Generate the tables for the dynamic table option

    It will generally be sensible to use tables to compute finite 
    field multiplies and inverses but where memory is scarse this 
    code might sometimes be better. But it only has effect during
    initialisation so its pretty unimportant in overall terms.
*/

/*  return 2 ^ (n - 1) where n is the bit number of the highest bit
    set in x with x in the range 1 < x < 0x00000200.   This form is
    used so that locals within fi can be bytes rather than words
*/

static aes_08t hibit(const aes_32t x)
{   aes_08t r = (aes_08t)((x >> 1) | (x >> 2));
    
    r |= (r >> 2);
    r |= (r >> 4);
    return (r + 1) >> 1;
}

/* return the inverse of the finite field element x */

static aes_08t fi(const aes_08t x)
{   aes_08t p1 = x, p2 = BPOLY, n1 = hibit(x), n2 = 0x80, v1 = 1, v2 = 0;

    if(x < 2) return x;

    for(;;)
    {
        if(!n1) return v1;

        while(n2 >= n1)
        {   
            n2 /= n1; p2 ^= p1 * n2; v2 ^= v1 * n2; n2 = hibit(p2);
        }
        
        if(!n2) return v2;

        while(n1 >= n2)
        {   
            n1 /= n2; p1 ^= p2 * n1; v1 ^= v2 * n1; n1 = hibit(p1);
        }
    }
}

#else

/* define the finite field multiplies required for Rijndael */

#define f2(x) ((x) ? pow[log[x] + 0x19] : 0)
#define f3(x) ((x) ? pow[log[x] + 0x01] : 0)
#define f9(x) ((x) ? pow[log[x] + 0xc7] : 0)
#define fb(x) ((x) ? pow[log[x] + 0x68] : 0)
#define fd(x) ((x) ? pow[log[x] + 0xee] : 0)
#define fe(x) ((x) ? pow[log[x] + 0xdf] : 0)
#define fi(x) ((x) ?   pow[255 - log[x]]: 0)

#endif

/* The forward and inverse affine transformations used in the S-box */

#define fwd_affine(x) \
    (w = (aes_32t)x, w ^= (w<<1)^(w<<2)^(w<<3)^(w<<4), 0x63^(aes_08t)(w^(w>>8)))

#define inv_affine(x) \
    (w = (aes_32t)x, w = (w<<1)^(w<<3)^(w<<6), 0x05^(aes_08t)(w^(w>>8)))

void gen_tabs(void)
{   aes_32t  i, w;

#if defined(FF_TABLES)

    aes_08t  pow[512], log[256];

    /*  log and power tables for GF(2^8) finite field with
        WPOLY as modular polynomial - the simplest primitive
        root is 0x03, used here to generate the tables
    */

    i = 0; w = 1; 
    do
    {   
        pow[i] = (aes_08t)w;
        pow[i + 255] = (aes_08t)w;
        log[w] = (aes_08t)i++;
        w ^=  (w << 1) ^ (w & 0x80 ? WPOLY : 0);
    }
    while (w != 1);

#endif

    for(i = 0, w = 1; i < RC_LENGTH; ++i)
    {
        rcon_tab[i] = bytes2word(w, 0, 0, 0);
        w = f2(w);
    }

    for(i = 0; i < 256; ++i)
    {   aes_08t    b;

        b = fwd_affine(fi((aes_08t)i));
        w = bytes2word(f2(b), b, b, f3(b));

#ifdef  SBX_SET
        s_box[i] = b;
#endif

#ifdef  FT1_SET                 /* tables for a normal encryption round */
        ft_tab[i] = w;
#endif
#ifdef  FT4_SET
        ft_tab[0][i] = w;
        ft_tab[1][i] = upr(w,1);
        ft_tab[2][i] = upr(w,2);
        ft_tab[3][i] = upr(w,3);
#endif
        w = bytes2word(b, 0, 0, 0);

#ifdef  FL1_SET                 /* tables for last encryption round (may also   */
        fl_tab[i] = w;          /* be used in the key schedule)                 */
#endif
#ifdef  FL4_SET
        fl_tab[0][i] = w;
        fl_tab[1][i] = upr(w,1);
        fl_tab[2][i] = upr(w,2);
        fl_tab[3][i] = upr(w,3);
#endif

#ifdef  LS1_SET                 /* table for key schedule if fl_tab above is    */
        ls_tab[i] = w;          /* not of the required form                     */
#endif
#ifdef  LS4_SET
        ls_tab[0][i] = w;
        ls_tab[1][i] = upr(w,1);
        ls_tab[2][i] = upr(w,2);
        ls_tab[3][i] = upr(w,3);
#endif

        b = fi(inv_affine((aes_08t)i));
        w = bytes2word(fe(b), f9(b), fd(b), fb(b));

#ifdef  IM1_SET                 /* tables for the inverse mix column operation  */
        im_tab[b] = w;
#endif
#ifdef  IM4_SET
        im_tab[0][b] = w;
        im_tab[1][b] = upr(w,1);
        im_tab[2][b] = upr(w,2);
        im_tab[3][b] = upr(w,3);
#endif

#ifdef  ISB_SET
        inv_s_box[i] = b;
#endif
#ifdef  IT1_SET                 /* tables for a normal decryption round */
        it_tab[i] = w;
#endif
#ifdef  IT4_SET
        it_tab[0][i] = w;
        it_tab[1][i] = upr(w,1);
        it_tab[2][i] = upr(w,2);
        it_tab[3][i] = upr(w,3);
#endif
        w = bytes2word(b, 0, 0, 0);
#ifdef  IL1_SET                 /* tables for last decryption round */
        il_tab[i] = w;
#endif
#ifdef  IL4_SET
        il_tab[0][i] = w;
        il_tab[1][i] = upr(w,1);
        il_tab[2][i] = upr(w,2);
        il_tab[3][i] = upr(w,3);
#endif
    }

    tab_init = 1;
}

#endif

/*----------------------------------------------------------------------
|       key schedule
+---------------------------------------------------------------------*/
#if !defined(BLOCK_SIZE)

static aes_rval aes_blk_len(unsigned int blen, aes_ctx cx[1])
{
#if !defined(FIXED_TABLES)
    if(!tab_init) gen_tabs();
#endif

    if((blen & 7) || blen < 16 || blen > 32) 
    {     
        cx->n_blk = 0; return aes_bad;
    }

    cx->n_blk = blen;
    return aes_good;
}

#endif

/* Initialise the key schedule from the user supplied key. The key
   length is now specified in bytes - 16, 24 or 32 as appropriate.
   This corresponds to bit lengths of 128, 192 and 256 bits, and
   to Nk values of 4, 6 and 8 respectively.

   The following macros implement a single cycle in the key 
   schedule generation process. The number of cycles needed 
   for each cx->n_col and nk value is:
 
    nk =             4  5  6  7  8
    ------------------------------
    cx->n_col = 4   10  9  8  7  7
    cx->n_col = 5   14 11 10  9  9
    cx->n_col = 6   19 15 12 11 11
    cx->n_col = 7   21 19 16 13 14
    cx->n_col = 8   29 23 19 17 14
*/

#define ke4(k,i) \
{   k[4*(i)+4] = ss[0] ^= ls_box(ss[3],3) ^ rcon_tab[i]; k[4*(i)+5] = ss[1] ^= ss[0]; \
    k[4*(i)+6] = ss[2] ^= ss[1]; k[4*(i)+7] = ss[3] ^= ss[2]; \
}
#define kel4(k,i) \
{   k[4*(i)+4] = ss[0] ^= ls_box(ss[3],3) ^ rcon_tab[i]; k[4*(i)+5] = ss[1] ^= ss[0]; \
    k[4*(i)+6] = ss[2] ^= ss[1]; k[4*(i)+7] = ss[3] ^= ss[2]; \
}

#define ke6(k,i) \
{   k[6*(i)+ 6] = ss[0] ^= ls_box(ss[5],3) ^ rcon_tab[i]; k[6*(i)+ 7] = ss[1] ^= ss[0]; \
    k[6*(i)+ 8] = ss[2] ^= ss[1]; k[6*(i)+ 9] = ss[3] ^= ss[2]; \
    k[6*(i)+10] = ss[4] ^= ss[3]; k[6*(i)+11] = ss[5] ^= ss[4]; \
}
#define kel6(k,i) \
{   k[6*(i)+ 6] = ss[0] ^= ls_box(ss[5],3) ^ rcon_tab[i]; k[6*(i)+ 7] = ss[1] ^= ss[0]; \
    k[6*(i)+ 8] = ss[2] ^= ss[1]; k[6*(i)+ 9] = ss[3] ^= ss[2]; \
}

#define ke8(k,i) \
{   k[8*(i)+ 8] = ss[0] ^= ls_box(ss[7],3) ^ rcon_tab[i]; k[8*(i)+ 9] = ss[1] ^= ss[0]; \
    k[8*(i)+10] = ss[2] ^= ss[1]; k[8*(i)+11] = ss[3] ^= ss[2]; \
    k[8*(i)+12] = ss[4] ^= ls_box(ss[3],0); k[8*(i)+13] = ss[5] ^= ss[4]; \
    k[8*(i)+14] = ss[6] ^= ss[5]; k[8*(i)+15] = ss[7] ^= ss[6]; \
}
#define kel8(k,i) \
{   k[8*(i)+ 8] = ss[0] ^= ls_box(ss[7],3) ^ rcon_tab[i]; k[8*(i)+ 9] = ss[1] ^= ss[0]; \
    k[8*(i)+10] = ss[2] ^= ss[1]; k[8*(i)+11] = ss[3] ^= ss[2]; \
}

#if defined(ENCRYPTION_KEY_SCHEDULE)

static aes_rval aes_enc_key(const unsigned char in_key[], unsigned int klen, aes_ctx cx[1])
{   aes_32t    ss[8]; 

#if !defined(FIXED_TABLES)
    if(!tab_init) gen_tabs();
#endif

#if !defined(BLOCK_SIZE)
    if(!cx->n_blk) cx->n_blk = 16;
#else
    cx->n_blk = BLOCK_SIZE;
#endif
    
    cx->n_blk = (cx->n_blk & ~3) | 1;

    cx->k_sch[0] = ss[0] = word_in(in_key     );
    cx->k_sch[1] = ss[1] = word_in(in_key +  4);
    cx->k_sch[2] = ss[2] = word_in(in_key +  8);
    cx->k_sch[3] = ss[3] = word_in(in_key + 12);

#if (BLOCK_SIZE == 16) && (ENC_UNROLL != NONE)

    switch(klen)
    {
    case 16:    ke4(cx->k_sch, 0); ke4(cx->k_sch, 1); 
                ke4(cx->k_sch, 2); ke4(cx->k_sch, 3);
                ke4(cx->k_sch, 4); ke4(cx->k_sch, 5); 
                ke4(cx->k_sch, 6); ke4(cx->k_sch, 7);
                ke4(cx->k_sch, 8); kel4(cx->k_sch, 9); 
                cx->n_rnd = 10; break;
    case 24:    cx->k_sch[4] = ss[4] = word_in(in_key + 16);
                cx->k_sch[5] = ss[5] = word_in(in_key + 20);
                ke6(cx->k_sch, 0); ke6(cx->k_sch, 1); 
                ke6(cx->k_sch, 2); ke6(cx->k_sch, 3);
                ke6(cx->k_sch, 4); ke6(cx->k_sch, 5); 
                ke6(cx->k_sch, 6); kel6(cx->k_sch, 7); 
                cx->n_rnd = 12; break;
    case 32:    cx->k_sch[4] = ss[4] = word_in(in_key + 16);
                cx->k_sch[5] = ss[5] = word_in(in_key + 20);
                cx->k_sch[6] = ss[6] = word_in(in_key + 24);
                cx->k_sch[7] = ss[7] = word_in(in_key + 28);
                ke8(cx->k_sch, 0); ke8(cx->k_sch, 1); 
                ke8(cx->k_sch, 2); ke8(cx->k_sch, 3);
                ke8(cx->k_sch, 4); ke8(cx->k_sch, 5); 
                kel8(cx->k_sch, 6); 
                cx->n_rnd = 14; break;
    default:    cx->n_rnd = 0; return aes_bad; 
    }
#else
    {   aes_32t i, l;
        cx->n_rnd = ((klen >> 2) > nc ? (klen >> 2) : nc) + 6;
        l = (nc * cx->n_rnd + nc - 1) / (klen >> 2);

        switch(klen)
        {
        case 16:    for(i = 0; i < l; ++i)
                        ke4(cx->k_sch, i);
                    break;
        case 24:    cx->k_sch[4] = ss[4] = word_in(in_key + 16);
                    cx->k_sch[5] = ss[5] = word_in(in_key + 20);
                    for(i = 0; i < l; ++i)
                        ke6(cx->k_sch, i);
                    break;
        case 32:    cx->k_sch[4] = ss[4] = word_in(in_key + 16);
                    cx->k_sch[5] = ss[5] = word_in(in_key + 20);
                    cx->k_sch[6] = ss[6] = word_in(in_key + 24);
                    cx->k_sch[7] = ss[7] = word_in(in_key + 28);
                    for(i = 0; i < l; ++i)
                        ke8(cx->k_sch,  i);
                    break;
        default:    cx->n_rnd = 0; return aes_bad; 
        }
    }
#endif

    return aes_good;
}

#endif

#if defined(DECRYPTION_KEY_SCHEDULE)

#if (DEC_ROUND != NO_TABLES)
#define d_vars  dec_imvars
#define ff(x)   inv_mcol(x)
#else
#define ff(x)   (x)
#define d_vars
#endif

#if 1
#define kdf4(k,i) \
{   ss[0] = ss[0] ^ ss[2] ^ ss[1] ^ ss[3]; ss[1] = ss[1] ^ ss[3]; ss[2] = ss[2] ^ ss[3]; ss[3] = ss[3]; \
    ss[4] = ls_box(ss[(i+3) % 4], 3) ^ rcon_tab[i]; ss[i % 4] ^= ss[4]; \
    ss[4] ^= k[4*(i)];   k[4*(i)+4] = ff(ss[4]); ss[4] ^= k[4*(i)+1]; k[4*(i)+5] = ff(ss[4]); \
    ss[4] ^= k[4*(i)+2]; k[4*(i)+6] = ff(ss[4]); ss[4] ^= k[4*(i)+3]; k[4*(i)+7] = ff(ss[4]); \
}
#define kd4(k,i) \
{   ss[4] = ls_box(ss[(i+3) % 4], 3) ^ rcon_tab[i]; ss[i % 4] ^= ss[4]; ss[4] = ff(ss[4]); \
    k[4*(i)+4] = ss[4] ^= k[4*(i)]; k[4*(i)+5] = ss[4] ^= k[4*(i)+1]; \
    k[4*(i)+6] = ss[4] ^= k[4*(i)+2]; k[4*(i)+7] = ss[4] ^= k[4*(i)+3]; \
}
#define kdl4(k,i) \
{   ss[4] = ls_box(ss[(i+3) % 4], 3) ^ rcon_tab[i]; ss[i % 4] ^= ss[4]; \
    k[4*(i)+4] = (ss[0] ^= ss[1]) ^ ss[2] ^ ss[3]; k[4*(i)+5] = ss[1] ^ ss[3]; \
    k[4*(i)+6] = ss[0]; k[4*(i)+7] = ss[1]; \
}
#else
#define kdf4(k,i) \
{   ss[0] ^= ls_box(ss[3],3) ^ rcon_tab[i]; k[4*(i)+ 4] = ff(ss[0]); ss[1] ^= ss[0]; k[4*(i)+ 5] = ff(ss[1]); \
    ss[2] ^= ss[1]; k[4*(i)+ 6] = ff(ss[2]); ss[3] ^= ss[2]; k[4*(i)+ 7] = ff(ss[3]); \
}
#define kd4(k,i) \
{   ss[4] = ls_box(ss[3],3) ^ rcon_tab[i]; \
    ss[0] ^= ss[4]; ss[4] = ff(ss[4]); k[4*(i)+ 4] = ss[4] ^= k[4*(i)]; \
    ss[1] ^= ss[0]; k[4*(i)+ 5] = ss[4] ^= k[4*(i)+ 1]; \
    ss[2] ^= ss[1]; k[4*(i)+ 6] = ss[4] ^= k[4*(i)+ 2]; \
    ss[3] ^= ss[2]; k[4*(i)+ 7] = ss[4] ^= k[4*(i)+ 3]; \
}
#define kdl4(k,i) \
{   ss[0] ^= ls_box(ss[3],3) ^ rcon_tab[i]; k[4*(i)+ 4] = ss[0]; ss[1] ^= ss[0]; k[4*(i)+ 5] = ss[1]; \
    ss[2] ^= ss[1]; k[4*(i)+ 6] = ss[2]; ss[3] ^= ss[2]; k[4*(i)+ 7] = ss[3]; \
}
#endif

#define kdf6(k,i) \
{   ss[0] ^= ls_box(ss[5],3) ^ rcon_tab[i]; k[6*(i)+ 6] = ff(ss[0]); ss[1] ^= ss[0]; k[6*(i)+ 7] = ff(ss[1]); \
    ss[2] ^= ss[1]; k[6*(i)+ 8] = ff(ss[2]); ss[3] ^= ss[2]; k[6*(i)+ 9] = ff(ss[3]); \
    ss[4] ^= ss[3]; k[6*(i)+10] = ff(ss[4]); ss[5] ^= ss[4]; k[6*(i)+11] = ff(ss[5]); \
}
#define kd6(k,i) \
{   ss[6] = ls_box(ss[5],3) ^ rcon_tab[i]; \
    ss[0] ^= ss[6]; ss[6] = ff(ss[6]); k[6*(i)+ 6] = ss[6] ^= k[6*(i)]; \
    ss[1] ^= ss[0]; k[6*(i)+ 7] = ss[6] ^= k[6*(i)+ 1]; \
    ss[2] ^= ss[1]; k[6*(i)+ 8] = ss[6] ^= k[6*(i)+ 2]; \
    ss[3] ^= ss[2]; k[6*(i)+ 9] = ss[6] ^= k[6*(i)+ 3]; \
    ss[4] ^= ss[3]; k[6*(i)+10] = ss[6] ^= k[6*(i)+ 4]; \
    ss[5] ^= ss[4]; k[6*(i)+11] = ss[6] ^= k[6*(i)+ 5]; \
}
#define kdl6(k,i) \
{   ss[0] ^= ls_box(ss[5],3) ^ rcon_tab[i]; k[6*(i)+ 6] = ss[0]; ss[1] ^= ss[0]; k[6*(i)+ 7] = ss[1]; \
    ss[2] ^= ss[1]; k[6*(i)+ 8] = ss[2]; ss[3] ^= ss[2]; k[6*(i)+ 9] = ss[3]; \
}

#define kdf8(k,i) \
{   ss[0] ^= ls_box(ss[7],3) ^ rcon_tab[i]; k[8*(i)+ 8] = ff(ss[0]); ss[1] ^= ss[0]; k[8*(i)+ 9] = ff(ss[1]); \
    ss[2] ^= ss[1]; k[8*(i)+10] = ff(ss[2]); ss[3] ^= ss[2]; k[8*(i)+11] = ff(ss[3]); \
    ss[4] ^= ls_box(ss[3],0); k[8*(i)+12] = ff(ss[4]); ss[5] ^= ss[4]; k[8*(i)+13] = ff(ss[5]); \
    ss[6] ^= ss[5]; k[8*(i)+14] = ff(ss[6]); ss[7] ^= ss[6]; k[8*(i)+15] = ff(ss[7]); \
}
#define kd8(k,i) \
{   aes_32t g = ls_box(ss[7],3) ^ rcon_tab[i]; \
    ss[0] ^= g; g = ff(g); k[8*(i)+ 8] = g ^= k[8*(i)]; \
    ss[1] ^= ss[0]; k[8*(i)+ 9] = g ^= k[8*(i)+ 1]; \
    ss[2] ^= ss[1]; k[8*(i)+10] = g ^= k[8*(i)+ 2]; \
    ss[3] ^= ss[2]; k[8*(i)+11] = g ^= k[8*(i)+ 3]; \
    g = ls_box(ss[3],0); \
    ss[4] ^= g; g = ff(g); k[8*(i)+12] = g ^= k[8*(i)+ 4]; \
    ss[5] ^= ss[4]; k[8*(i)+13] = g ^= k[8*(i)+ 5]; \
    ss[6] ^= ss[5]; k[8*(i)+14] = g ^= k[8*(i)+ 6]; \
    ss[7] ^= ss[6]; k[8*(i)+15] = g ^= k[8*(i)+ 7]; \
}
#define kdl8(k,i) \
{   ss[0] ^= ls_box(ss[7],3) ^ rcon_tab[i]; k[8*(i)+ 8] = ss[0]; ss[1] ^= ss[0]; k[8*(i)+ 9] = ss[1]; \
    ss[2] ^= ss[1]; k[8*(i)+10] = ss[2]; ss[3] ^= ss[2]; k[8*(i)+11] = ss[3]; \
}

static aes_rval aes_dec_key(const unsigned char in_key[], unsigned int klen, aes_ctx cx[1])
{   aes_32t    ss[8]; 
    d_vars

#if !defined(FIXED_TABLES)
    if(!tab_init) gen_tabs();
#endif

#if !defined(BLOCK_SIZE)
    if(!cx->n_blk) cx->n_blk = 16;
#else
    cx->n_blk = BLOCK_SIZE;
#endif

    cx->n_blk = (cx->n_blk & ~3) | 2;

    cx->k_sch[0] = ss[0] = word_in(in_key     );
    cx->k_sch[1] = ss[1] = word_in(in_key +  4);
    cx->k_sch[2] = ss[2] = word_in(in_key +  8);
    cx->k_sch[3] = ss[3] = word_in(in_key + 12);

#if (BLOCK_SIZE == 16) && (DEC_UNROLL != NONE)

    switch(klen)
    {
    case 16:    kdf4(cx->k_sch, 0); kd4(cx->k_sch, 1); 
                kd4(cx->k_sch, 2); kd4(cx->k_sch, 3);
                kd4(cx->k_sch, 4); kd4(cx->k_sch, 5); 
                kd4(cx->k_sch, 6); kd4(cx->k_sch, 7);
                kd4(cx->k_sch, 8); kdl4(cx->k_sch, 9); 
                cx->n_rnd = 10; break;
    case 24:    cx->k_sch[4] = ff(ss[4] = word_in(in_key + 16));
                cx->k_sch[5] = ff(ss[5] = word_in(in_key + 20));
                kdf6(cx->k_sch, 0); kd6(cx->k_sch, 1); 
                kd6(cx->k_sch, 2); kd6(cx->k_sch, 3);
                kd6(cx->k_sch, 4); kd6(cx->k_sch, 5); 
                kd6(cx->k_sch, 6); kdl6(cx->k_sch, 7); 
                cx->n_rnd = 12; break;
    case 32:    cx->k_sch[4] = ff(ss[4] = word_in(in_key + 16));
                cx->k_sch[5] = ff(ss[5] = word_in(in_key + 20));
                cx->k_sch[6] = ff(ss[6] = word_in(in_key + 24));
                cx->k_sch[7] = ff(ss[7] = word_in(in_key + 28));
                kdf8(cx->k_sch, 0); kd8(cx->k_sch, 1); 
                kd8(cx->k_sch, 2); kd8(cx->k_sch, 3);
                kd8(cx->k_sch, 4); kd8(cx->k_sch, 5); 
                kdl8(cx->k_sch, 6); 
                cx->n_rnd = 14; break;
    default:    cx->n_rnd = 0; return aes_bad; 
    }
#else
    {   aes_32t i, l;
        cx->n_rnd = ((klen >> 2) > nc ? (klen >> 2) : nc) + 6;
        l = (nc * cx->n_rnd + nc - 1) / (klen >> 2);

        switch(klen)
        {
        case 16: 
                    for(i = 0; i < l; ++i)
                        ke4(cx->k_sch, i);
                    break;
        case 24:    cx->k_sch[4] = ss[4] = word_in(in_key + 16);
                    cx->k_sch[5] = ss[5] = word_in(in_key + 20);
                    for(i = 0; i < l; ++i)
                        ke6(cx->k_sch, i);
                    break;
        case 32:    cx->k_sch[4] = ss[4] = word_in(in_key + 16);
                    cx->k_sch[5] = ss[5] = word_in(in_key + 20);
                    cx->k_sch[6] = ss[6] = word_in(in_key + 24);
                    cx->k_sch[7] = ss[7] = word_in(in_key + 28);
                    for(i = 0; i < l; ++i)
                        ke8(cx->k_sch,  i);
                    break;
        default:    cx->n_rnd = 0; return aes_bad; 
        }
#if (DEC_ROUND != NO_TABLES)
        for(i = nc; i < nc * cx->n_rnd; ++i)
            cx->k_sch[i] = inv_mcol(cx->k_sch[i]);
#endif
    }
#endif

    return aes_good;
}

#endif

/*----------------------------------------------------------------------
|       cipher
+---------------------------------------------------------------------*/
#define unused  77  /* Sunset Strip */

#define si(y,x,k,c) s(y,c) = word_in(x + 4 * c) ^ k[c]
#define so(y,x,c)   word_out(y + 4 * c, s(x,c))

#if BLOCK_SIZE == 16

#if defined(ARRAYS)
#define locals(y,x)     x[4],y[4]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,y##0,y##1,y##2,y##3
 /* 
   the following defines prevent the compiler requiring the declaration
   of generated but unused variables in the fwd_var and inv_var macros
 */
#define b04 unused
#define b05 unused
#define b06 unused
#define b07 unused
#define b14 unused
#define b15 unused
#define b16 unused
#define b17 unused
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)

#elif BLOCK_SIZE == 24

#if defined(ARRAYS)
#define locals(y,x)     x[6],y[6]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,x##4,x##5, \
                        y##0,y##1,y##2,y##3,y##4,y##5
#define b06 unused
#define b07 unused
#define b16 unused
#define b17 unused
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3); \
                        s(y,4) = s(x,4); s(y,5) = s(x,5);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); \
                        si(y,x,k,3); si(y,x,k,4); si(y,x,k,5)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); \
                        so(y,x,3); so(y,x,4); so(y,x,5)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); \
                        rm(y,x,k,3); rm(y,x,k,4); rm(y,x,k,5)
#else

#if defined(ARRAYS)
#define locals(y,x)     x[8],y[8]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,x##4,x##5,x##6,x##7, \
                        y##0,y##1,y##2,y##3,y##4,y##5,y##6,y##7
#endif
#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3); \
                        s(y,4) = s(x,4); s(y,5) = s(x,5); \
                        s(y,6) = s(x,6); s(y,7) = s(x,7);

#if BLOCK_SIZE == 32

#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3); \
                        si(y,x,k,4); si(y,x,k,5); si(y,x,k,6); si(y,x,k,7)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3); \
                        so(y,x,4); so(y,x,5); so(y,x,6); so(y,x,7)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3); \
                        rm(y,x,k,4); rm(y,x,k,5); rm(y,x,k,6); rm(y,x,k,7)
#else

#define state_in(y,x,k) \
switch(nc) \
{   case 8: si(y,x,k,7); si(y,x,k,6); \
    case 6: si(y,x,k,5); si(y,x,k,4); \
    case 4: si(y,x,k,3); si(y,x,k,2); \
            si(y,x,k,1); si(y,x,k,0); \
}

#define state_out(y,x) \
switch(nc) \
{   case 8: so(y,x,7); so(y,x,6); \
    case 6: so(y,x,5); so(y,x,4); \
    case 4: so(y,x,3); so(y,x,2); \
            so(y,x,1); so(y,x,0); \
}

#if defined(FAST_VARIABLE)

#define round(rm,y,x,k) \
switch(nc) \
{   case 8: rm(y,x,k,7); rm(y,x,k,6); \
            rm(y,x,k,5); rm(y,x,k,4); \
            rm(y,x,k,3); rm(y,x,k,2); \
            rm(y,x,k,1); rm(y,x,k,0); \
            break; \
    case 6: rm(y,x,k,5); rm(y,x,k,4); \
            rm(y,x,k,3); rm(y,x,k,2); \
            rm(y,x,k,1); rm(y,x,k,0); \
            break; \
    case 4: rm(y,x,k,3); rm(y,x,k,2); \
            rm(y,x,k,1); rm(y,x,k,0); \
            break; \
}
#else

#define round(rm,y,x,k) \
switch(nc) \
{   case 8: rm(y,x,k,7); rm(y,x,k,6); \
    case 6: rm(y,x,k,5); rm(y,x,k,4); \
    case 4: rm(y,x,k,3); rm(y,x,k,2); \
            rm(y,x,k,1); rm(y,x,k,0); \
}

#endif

#endif
#endif

#if defined(ENCRYPTION)

/* I am grateful to Frank Yellin for the following construction
   (and that for decryption) which, given the column (c) of the 
   output state variable, gives the input state variables which 
   are needed in its computation for each row (r) of the state.

   For the fixed block size options, compilers should be able to 
   reduce this complex expression (and the equivalent one for 
   decryption) to a static variable reference at compile time. 
   But for variable block size code, there will be some limbs on
   which conditional clauses will be returned.
*/

/* y = output word, x = input word, r = row, c = column for r = 0, 
   1, 2 and 3 = column accessed for row r.
*/

#define fwd_var(x,r,c)\
 ( r == 0 ?           \
    ( c == 0 ? s(x,0) \
    : c == 1 ? s(x,1) \
    : c == 2 ? s(x,2) \
    : c == 3 ? s(x,3) \
    : c == 4 ? s(x,4) \
    : c == 5 ? s(x,5) \
    : c == 6 ? s(x,6) \
    :          s(x,7))\
 : r == 1 ?           \
    ( c == 0 ? s(x,1) \
    : c == 1 ? s(x,2) \
    : c == 2 ? s(x,3) \
    : c == 3 ? nc == 4 ? s(x,0) : s(x,4) \
    : c == 4 ? s(x,5) \
    : c == 5 ? nc == 8 ? s(x,6) : s(x,0) \
    : c == 6 ? s(x,7) \
    :          s(x,0))\
 : r == 2 ?           \
    ( c == 0 ? nc == 8 ? s(x,3) : s(x,2) \
    : c == 1 ? nc == 8 ? s(x,4) : s(x,3) \
    : c == 2 ? nc == 4 ? s(x,0) : nc == 8 ? s(x,5) : s(x,4) \
    : c == 3 ? nc == 4 ? s(x,1) : nc == 8 ? s(x,6) : s(x,5) \
    : c == 4 ? nc == 8 ? s(x,7) : s(x,0) \
    : c == 5 ? nc == 8 ? s(x,0) : s(x,1) \
    : c == 6 ? s(x,1) \
    :          s(x,2))\
 :                    \
    ( c == 0 ? nc == 8 ? s(x,4) : s(x,3) \
    : c == 1 ? nc == 4 ? s(x,0) : nc == 8 ? s(x,5) : s(x,4) \
    : c == 2 ? nc == 4 ? s(x,1) : nc == 8 ? s(x,6) : s(x,5) \
    : c == 3 ? nc == 4 ? s(x,2) : nc == 8 ? s(x,7) : s(x,0) \
    : c == 4 ? nc == 8 ? s(x,0) : s(x,1) \
    : c == 5 ? nc == 8 ? s(x,1) : s(x,2) \
    : c == 6 ? s(x,2) \
    :          s(x,3)))

#if defined(FT4_SET)
#undef  dec_fmvars
#define dec_fmvars
#define fwd_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,ft_tab,fwd_var,rf1,c)
#elif defined(FT1_SET)
#undef  dec_fmvars
#define dec_fmvars
#define fwd_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ one_table(x,upr,ft_tab,fwd_var,rf1,c)
#else
#define fwd_rnd(y,x,k,c)    s(y,c) = fwd_mcol(no_table(x,s_box,fwd_var,rf1,c)) ^ (k)[c]
#endif

#if defined(FL4_SET)
#define fwd_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,fl_tab,fwd_var,rf1,c)
#elif defined(FL1_SET)
#define fwd_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ one_table(x,ups,fl_tab,fwd_var,rf1,c)
#else
#define fwd_lrnd(y,x,k,c)   s(y,c) = no_table(x,s_box,fwd_var,rf1,c) ^ (k)[c]
#endif

static aes_rval aes_enc_blk(const unsigned char in_blk[], unsigned char out_blk[], const aes_ctx cx[1])
{   aes_32t        locals(b0, b1);
    const aes_32t  *kp = cx->k_sch;
    dec_fmvars  /* declare variables for fwd_mcol() if needed */

    if(!(cx->n_blk & 1)) return aes_bad;

    state_in(b0, in_blk, kp); 

#if (ENC_UNROLL == FULL)

    kp += (cx->n_rnd - 9) * nc;

    switch(cx->n_rnd)
    {
    case 14:    round(fwd_rnd,  b1, b0, kp - 4 * nc); 
                round(fwd_rnd,  b0, b1, kp - 3 * nc);
    case 12:    round(fwd_rnd,  b1, b0, kp - 2 * nc); 
                round(fwd_rnd,  b0, b1, kp -     nc);
    case 10:    round(fwd_rnd,  b1, b0, kp         );             
                round(fwd_rnd,  b0, b1, kp +     nc);
                round(fwd_rnd,  b1, b0, kp + 2 * nc); 
                round(fwd_rnd,  b0, b1, kp + 3 * nc);
                round(fwd_rnd,  b1, b0, kp + 4 * nc); 
                round(fwd_rnd,  b0, b1, kp + 5 * nc);
                round(fwd_rnd,  b1, b0, kp + 6 * nc); 
                round(fwd_rnd,  b0, b1, kp + 7 * nc);
                round(fwd_rnd,  b1, b0, kp + 8 * nc);
                round(fwd_lrnd, b0, b1, kp + 9 * nc);
    }
#else
    
#if (ENC_UNROLL == PARTIAL)
    {   aes_32t    rnd;
        for(rnd = 0; rnd < (cx->n_rnd >> 1) - 1; ++rnd)
        {
            kp += nc;
            round(fwd_rnd, b1, b0, kp); 
            kp += nc;
            round(fwd_rnd, b0, b1, kp); 
        }
        kp += nc;
        round(fwd_rnd,  b1, b0, kp);
#else
    {   aes_32t    rnd, *p0 = b0, *p1 = b1, *pt;
        for(rnd = 0; rnd < cx->n_rnd - 1; ++rnd)
        {
            kp += nc;
            round(fwd_rnd, p1, p0, kp); 
            pt = p0, p0 = p1, p1 = pt;
        }
#endif
        kp += nc;
        round(fwd_lrnd, b0, b1, kp);
    }
#endif

    state_out(out_blk, b0);
    return aes_good;
}

#endif

#if defined(DECRYPTION)

#define inv_var(x,r,c) \
 ( r == 0 ?           \
    ( c == 0 ? s(x,0) \
    : c == 1 ? s(x,1) \
    : c == 2 ? s(x,2) \
    : c == 3 ? s(x,3) \
    : c == 4 ? s(x,4) \
    : c == 5 ? s(x,5) \
    : c == 6 ? s(x,6) \
    :          s(x,7))\
 : r == 1 ?           \
    ( c == 0 ? nc == 4 ? s(x,3) : nc == 8 ? s(x,7) : s(x,5) \
    : c == 1 ? s(x,0) \
    : c == 2 ? s(x,1) \
    : c == 3 ? s(x,2) \
    : c == 4 ? s(x,3) \
    : c == 5 ? s(x,4) \
    : c == 6 ? s(x,5) \
    :          s(x,6))\
 : r == 2 ?           \
    ( c == 0 ? nc == 4 ? s(x,2) : nc == 8 ? s(x,5) : s(x,4) \
    : c == 1 ? nc == 4 ? s(x,3) : nc == 8 ? s(x,6) : s(x,5) \
    : c == 2 ? nc == 8 ? s(x,7) : s(x,0) \
    : c == 3 ? nc == 8 ? s(x,0) : s(x,1) \
    : c == 4 ? nc == 8 ? s(x,1) : s(x,2) \
    : c == 5 ? nc == 8 ? s(x,2) : s(x,3) \
    : c == 6 ? s(x,3) \
    :          s(x,4))\
 :                    \
    ( c == 0 ? nc == 4 ? s(x,1) : nc == 8 ? s(x,4) : s(x,3) \
    : c == 1 ? nc == 4 ? s(x,2) : nc == 8 ? s(x,5) : s(x,4) \
    : c == 2 ? nc == 4 ? s(x,3) : nc == 8 ? s(x,6) : s(x,5) \
    : c == 3 ? nc == 8 ? s(x,7) : s(x,0) \
    : c == 4 ? nc == 8 ? s(x,0) : s(x,1) \
    : c == 5 ? nc == 8 ? s(x,1) : s(x,2) \
    : c == 6 ? s(x,2) \
    :          s(x,3)))

#if defined(IT4_SET)
#undef  dec_imvars
#define dec_imvars
#define inv_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,it_tab,inv_var,rf1,c)
#elif defined(IT1_SET)
#undef  dec_imvars
#define dec_imvars
#define inv_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ one_table(x,upr,it_tab,inv_var,rf1,c)
#else
#define inv_rnd(y,x,k,c)    s(y,c) = inv_mcol(no_table(x,inv_s_box,inv_var,rf1,c) ^ (k)[c])
#endif

#if defined(IL4_SET)
#define inv_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,il_tab,inv_var,rf1,c)
#elif defined(IL1_SET)
#define inv_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ one_table(x,ups,il_tab,inv_var,rf1,c)
#else
#define inv_lrnd(y,x,k,c)   s(y,c) = no_table(x,inv_s_box,inv_var,rf1,c) ^ (k)[c]
#endif

static aes_rval aes_dec_blk(const unsigned char in_blk[], unsigned char out_blk[], const aes_ctx cx[1])
{   aes_32t        locals(b0, b1);
    const aes_32t  *kp = cx->k_sch + nc * cx->n_rnd;
    dec_imvars  /* declare variables for inv_mcol() if needed */

    if(!(cx->n_blk & 2)) return aes_bad;

    state_in(b0, in_blk, kp);

#if (DEC_UNROLL == FULL)

    kp = cx->k_sch + 9 * nc;
    switch(cx->n_rnd)
    {
    case 14:    round(inv_rnd,  b1, b0, kp + 4 * nc);
                round(inv_rnd,  b0, b1, kp + 3 * nc);
    case 12:    round(inv_rnd,  b1, b0, kp + 2 * nc);
                round(inv_rnd,  b0, b1, kp + nc    );
    case 10:    round(inv_rnd,  b1, b0, kp         );             
                round(inv_rnd,  b0, b1, kp -     nc);
                round(inv_rnd,  b1, b0, kp - 2 * nc); 
                round(inv_rnd,  b0, b1, kp - 3 * nc);
                round(inv_rnd,  b1, b0, kp - 4 * nc); 
                round(inv_rnd,  b0, b1, kp - 5 * nc);
                round(inv_rnd,  b1, b0, kp - 6 * nc); 
                round(inv_rnd,  b0, b1, kp - 7 * nc);
                round(inv_rnd,  b1, b0, kp - 8 * nc);
                round(inv_lrnd, b0, b1, kp - 9 * nc);
    }
#else
    
#if (DEC_UNROLL == PARTIAL)
    {   aes_32t    rnd;
        for(rnd = 0; rnd < (cx->n_rnd >> 1) - 1; ++rnd)
        {
            kp -= nc; 
            round(inv_rnd, b1, b0, kp); 
            kp -= nc; 
            round(inv_rnd, b0, b1, kp); 
        }
        kp -= nc;
        round(inv_rnd, b1, b0, kp);
#else
    {   aes_32t    rnd, *p0 = b0, *p1 = b1, *pt;
        for(rnd = 0; rnd < cx->n_rnd - 1; ++rnd)
        {
            kp -= nc;
            round(inv_rnd, p1, p0, kp); 
            pt = p0, p0 = p1, p1 = pt;
        }
#endif
        kp -= nc;
        round(inv_lrnd, b0, b1, kp);
    }
#endif

    state_out(out_blk, b0);
    return aes_good;
}

#endif

/*----------------------------------------------------------------------
|       AP4_AesBlockCipher::AP4_AesBlockCipher
+---------------------------------------------------------------------*/
AP4_AesBlockCipher::AP4_AesBlockCipher(const AP4_UI08* key)
{
    aes_enc_key(key, AP4_AES_KEY_LENGTH, &m_Context);
}

/*----------------------------------------------------------------------
|       AP4_AesBlockCipher::~AP4_AesBlockCipher
+---------------------------------------------------------------------*/
AP4_AesBlockCipher::~AP4_AesBlockCipher()
{
}

/*----------------------------------------------------------------------
|       AP4_AesCipher::EncryptBlock
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AesBlockCipher::EncryptBlock(const AP4_UI08* block_in, AP4_UI08* block_out)
{
    aes_rval result;
    result = aes_enc_blk(block_in, block_out, &m_Context);
    return result == aes_good ? AP4_SUCCESS : AP4_FAILURE;
}

