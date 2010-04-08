/*
 * AES Block cipher
 * (c) 2005-2008 Axiomatic Systems, LLC
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
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Config.h"
#include "Ap4Protection.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
struct aes_ctx;

/*----------------------------------------------------------------------
|   AES constants
+---------------------------------------------------------------------*/
#define AP4_AES_BLOCK_SIZE  16
#define AP4_AES_KEY_LENGTH  16

/*----------------------------------------------------------------------
|   AP4_AesBlockCipher class
+---------------------------------------------------------------------*/
class AP4_AesBlockCipher : public AP4_BlockCipher
{
public:
    // constructor and destructor
    AP4_AesBlockCipher(const AP4_UI08*                  key,
                       AP4_BlockCipher::CipherDirection direction);
    ~AP4_AesBlockCipher();

    // AP4_AesBlockCipher methods
    virtual AP4_Result ProcessBlock(const AP4_UI08* input,
                                    AP4_UI08*       output);

private:
    // members
    CipherDirection m_Direction;
    aes_ctx*        m_Context;
};

#endif // _AP4_AES_BLOCK_CIPHER_H_ 
