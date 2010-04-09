/*****************************************************************
|
|    AP4 - HMAC Algorithms
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

/*
 Portions of this code are based on the code of LibTomCrypt
 that was released into public domain by Tom St Denis. 
*/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4KeyWrap.h"
#include "Ap4AesBlockCipher.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/

/*----------------------------------------------------------------------
|   AP4_AesKeyWrap
+---------------------------------------------------------------------*/
AP4_Result
AP4_AesKeyWrap(const AP4_UI08* kek, 
               const AP4_UI08* cleartext_key, 
               AP4_Size        cleartext_key_size,
               AP4_DataBuffer& wrapped_key)
{
    // check parameters
    if (cleartext_key_size % 8) {
        // not a multiple of 64 bits
        return AP4_ERROR_INVALID_PARAMETERS;
    }
    
    // the output size is (n+1)*64 bits
    // where n is the number of 64-bit blocks
    // of the cleartext key
    unsigned int n = cleartext_key_size/8;
    wrapped_key.SetDataSize((n+1)*8);
    
    // Step 1. Initialize variables.
    // Set A = IV, an initial value (0xA6)
    // For i = 1 to n
    //     R[i] = P[i]
    AP4_UI08* a = (AP4_UI08*)wrapped_key.UseData();
    AP4_SetMemory(a, 0xA6, 8);
    AP4_UI08* r = a+8;
    AP4_CopyMemory(r, cleartext_key, cleartext_key_size);
     
    // Step 2. Calculate intermediate values.
    // For j = 0 to 5
    //     For i=1 to n
    //         B = AES(K, A | R[i])
    //         A = MSB(64, B) ^ t where t = (n*j)+i
    //         R[i] = LSB(64, B)    
    AP4_AesBlockCipher block_cipher(kek, AP4_BlockCipher::ENCRYPT);
	for (unsigned int j=0; j <= 5; j++) {
		r = a + 8;
		for (unsigned int i=1; i<=n; i++) {
            AP4_UI08 workspace[16];
            AP4_UI08 b[16];
            AP4_CopyMemory(workspace, a, 8);
            AP4_CopyMemory(&workspace[8], r, 8);
            block_cipher.ProcessBlock(workspace, b);
            AP4_CopyMemory(a, b, 8);
            a[7] ^= n*j+i;
            AP4_CopyMemory(r, &b[8], 8);
            r += 8;
		}
	}
    
    // Step 3. Output the results.
    // (Nothing to do here since we've worked in-place 
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AesKeyUnwrap
+---------------------------------------------------------------------*/
AP4_Result
AP4_AesKeyUnwrap(const AP4_UI08* kek,
                 const AP4_UI08* wrapped_key,
                 AP4_Size        wrapped_key_size,
                 AP4_DataBuffer& cleartext_key)
{
    // check parameters
    if ((wrapped_key_size % 8) || (wrapped_key_size < 24)) {
        // not a multiple of 64 bits or too small
        return AP4_ERROR_INVALID_PARAMETERS;
    }
    
    // setup the output buffer
    unsigned int n = (wrapped_key_size/8)-1;
    cleartext_key.SetDataSize(n*8);
    
    // Step 1. Initialize variables.
    // Set A = C[0]
    // For i = 1 to n
    //     R[i] = C[i]
    AP4_UI08 a[8]; 
    AP4_CopyMemory(a, wrapped_key, 8);
    AP4_UI08* r = (AP4_UI08*)cleartext_key.UseData();
    AP4_CopyMemory(r, wrapped_key+8, 8*n);
    
    // Step 2. Compute intermediate values.
    // For j = 5 to 0
    //   For i = n to 1
    //     B = AES-1(K, (A ^ t) | R[i]) where t = n*j+i
    //     A = MSB(64, B)
    //     R[i] = LSB(64, B)
    AP4_AesBlockCipher block_cipher(kek, AP4_BlockCipher::DECRYPT);
    for (int j=5; j>=0; j--) {
        r = (AP4_UI08*)cleartext_key.UseData()+(n-1)*8;
        for (int i=n; i>=1; i--) {
            AP4_UI08 workspace[16];
            AP4_UI08 b[16];
            AP4_CopyMemory(workspace, a, 8);
            workspace[7] ^= (n*j)+i;
            AP4_CopyMemory(&workspace[8], r, 8);
            block_cipher.ProcessBlock(workspace, b);
            AP4_CopyMemory(a, b, 8);
            AP4_CopyMemory(r, &b[8], 8);
            r -= 8;
        }
    }
    
    // Step 3. Output results.
    // If A is an appropriate initial value (see 2.2.3),
    // Then
    //   For i = 1 to n
    //     P[i] = R[i]
    // Else
    // Return an error
 	for (unsigned int i=0; i<8; i++) {
		if (a[i] != 0xA6) {
            cleartext_key.SetDataSize(0);
            return AP4_ERROR_INVALID_FORMAT;
        }
	}

    return AP4_SUCCESS;
}
