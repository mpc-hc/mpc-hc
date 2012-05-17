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

#ifndef _AP4_AES_BLOCK_CIPHER_H_
#define _AP4_AES_BLOCK_CIPHER_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Config.h"

/*----------------------------------------------------------------------
|       AES constants
+---------------------------------------------------------------------*/
#define AP4_AES_BLOCK_SIZE  16
#define AP4_AES_KEY_LENGTH  16

/*----------------------------------------------------------------------
|       AES types
+---------------------------------------------------------------------*/
typedef AP4_UI32     aes_32t;
typedef AP4_UI08     aes_08t;
typedef unsigned int aes_rval;
typedef struct                     // the AES context for encryption
{   aes_32t    k_sch[4*AP4_AES_BLOCK_SIZE];   // the encryption key schedule
    aes_32t    n_rnd;              // the number of cipher rounds
    aes_32t    n_blk;              // the number of bytes in the state
} aes_ctx;
#define aes_bad      0             // bad function return value
#define aes_good     1             // good function return value

/*----------------------------------------------------------------------
|       AP4_AesBlockCipher class
+---------------------------------------------------------------------*/
class AP4_AesBlockCipher
{
 public:
    // constructor and destructor
    AP4_AesBlockCipher(const AP4_UI08* key);
   ~AP4_AesBlockCipher();
    
    // methods
    AP4_Result EncryptBlock(const AP4_UI08* block_in, AP4_UI08* block_out);

 private:
    aes_ctx m_Context;
};

#endif // _AP4_AES_BLOCK_CIPHER_H_ 
