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
#include "Ap4Hmac.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
#define AP4_SHA256_BLOCK_SIZE 64

static const AP4_UI32 AP4_Sha256_K[64] =
{
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
    0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
    0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
    0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
    0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
    0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
    0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
    0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
    0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
    0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

/*----------------------------------------------------------------------
|   AP4_DigestSha256
+---------------------------------------------------------------------*/
class AP4_DigestSha256
{
public:
    AP4_DigestSha256();
    virtual ~AP4_DigestSha256() {}

    // AP4_Hmac methods
    virtual AP4_Result Update(const AP4_UI08* data, AP4_Size data_size);
    virtual AP4_Result Final(AP4_DataBuffer& digest);

private:
    // methods
    void CompressBlock(const AP4_UI08* block);

    // members
    AP4_UI64 m_Length;
    AP4_UI32 m_Pending;
    AP4_UI32 m_State[8];
    AP4_UI08 m_Buffer[64];
};

/*----------------------------------------------------------------------
|   AP4_HmacSha256
|
|	compute SHA256(key XOR opad, SHA256(key XOR ipad, data))
|   key is the MAC key
|   ipad is the byte 0x36 repeated 64 times
|   opad is the byte 0x5c repeated 64 times
|   and data is the data to authenticate
|
+---------------------------------------------------------------------*/
class AP4_HmacSha256 : public AP4_Hmac
{
public:
    AP4_HmacSha256(const AP4_UI08* key, AP4_Size key_size);

    // AP4_Hmac methods
    virtual AP4_Result Update(const AP4_UI08* data, AP4_Size data_size)
    {
        return m_InnerDigest.Update(data, data_size);
    }
    virtual AP4_Result Final(AP4_DataBuffer& buffer);

private:
    AP4_DigestSha256 m_InnerDigest;
    AP4_DigestSha256 m_OuterDigest;
};

/*----------------------------------------------------------------------
|   AP4_DigestSha256::AP4_DigestSha256
+---------------------------------------------------------------------*/
AP4_DigestSha256::AP4_DigestSha256() :
    m_Length(0),
    m_Pending(0)
{
    m_State[0] = 0x6A09E667UL;
    m_State[1] = 0xBB67AE85UL;
    m_State[2] = 0x3C6EF372UL;
    m_State[3] = 0xA54FF53AUL;
    m_State[4] = 0x510E527FUL;
    m_State[5] = 0x9B05688CUL;
    m_State[6] = 0x1F83D9ABUL;
    m_State[7] = 0x5BE0CD19UL;
}

/*----------------------------------------------------------------------
|   local macros
+---------------------------------------------------------------------*/
#define AP4_Sha256_RORc(x, y) \
( ((((unsigned long) (x) & 0xFFFFFFFFUL) >> (unsigned long) ((y) & 31)) | \
   ((unsigned long) (x) << (unsigned long) (32 - ((y) & 31)))) & 0xFFFFFFFFUL)
#define AP4_Sha256_Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define AP4_Sha256_Maj(x,y,z)      (((x | y) & z) | (x & y))
#define AP4_Sha256_S(x, n)         AP4_Sha256_RORc((x), (n))
#define AP4_Sha256_R(x, n)         (((x)&0xFFFFFFFFUL)>>(n))
#define AP4_Sha256_Sigma0(x)       (AP4_Sha256_S(x,  2) ^ AP4_Sha256_S(x, 13) ^ AP4_Sha256_S(x, 22))
#define AP4_Sha256_Sigma1(x)       (AP4_Sha256_S(x,  6) ^ AP4_Sha256_S(x, 11) ^ AP4_Sha256_S(x, 25))
#define AP4_Sha256_Gamma0(x)       (AP4_Sha256_S(x,  7) ^ AP4_Sha256_S(x, 18) ^ AP4_Sha256_R(x,  3))
#define AP4_Sha256_Gamma1(x)       (AP4_Sha256_S(x, 17) ^ AP4_Sha256_S(x, 19) ^ AP4_Sha256_R(x, 10))

/*----------------------------------------------------------------------
|   AP4_DigestSha256::CompressBlock
+---------------------------------------------------------------------*/
void
AP4_DigestSha256::CompressBlock(const AP4_UI08* block)
{
    AP4_UI32 S[8], W[64];

    /* copy the state into S */
    for(unsigned int i = 0; i < 8; i++)
    {
        S[i] = m_State[i];
    }

    /* copy the 512-bit block into W[0..15] */
    for(unsigned int i = 0; i < 16; i++)
    {
        W[i] = AP4_BytesToUInt32BE(&block[4*i]);
    }

    /* fill W[16..63] */
    for(unsigned int i = 16; i < AP4_SHA256_BLOCK_SIZE; i++)
    {
        W[i] = AP4_Sha256_Gamma1(W[i-2]) + W[i-7] + AP4_Sha256_Gamma0(W[i-15]) + W[i-16];
    }

    /* compress */
    AP4_UI32 t, t0, t1;
    for(unsigned int i = 0; i < AP4_SHA256_BLOCK_SIZE; ++i)
    {
        t0 = S[7] + AP4_Sha256_Sigma1(S[4]) + AP4_Sha256_Ch(S[4], S[5], S[6]) + AP4_Sha256_K[i] + W[i];
        t1 = AP4_Sha256_Sigma0(S[0]) + AP4_Sha256_Maj(S[0], S[1], S[2]);
        S[3] += t0;
        S[7]  = t0 + t1;
        t = S[7];
        S[7] = S[6];
        S[6] = S[5];
        S[5] = S[4];
        S[4] = S[3];
        S[3] = S[2];
        S[2] = S[1];
        S[1] = S[0];
        S[0] = t;
    }

    /* feedback */
    for(unsigned int i = 0; i < 8; i++)
    {
        m_State[i] = m_State[i] + S[i];
    }
}


/*----------------------------------------------------------------------
|   AP4_DigestSha256::Update
+---------------------------------------------------------------------*/
AP4_Result
AP4_DigestSha256::Update(const AP4_UI08* data, AP4_Size data_size)
{
    while(data_size > 0)
    {
        if(m_Pending == 0 && data_size >= AP4_SHA256_BLOCK_SIZE)
        {
            CompressBlock(data);
            m_Length  += AP4_SHA256_BLOCK_SIZE * 8;
            data      += AP4_SHA256_BLOCK_SIZE;
            data_size -= AP4_SHA256_BLOCK_SIZE;
        }
        else
        {
            unsigned int chunk = data_size;
            if(chunk > (AP4_SHA256_BLOCK_SIZE - m_Pending))
            {
                chunk = AP4_SHA256_BLOCK_SIZE - m_Pending;
            }
            AP4_CopyMemory(&m_Buffer[m_Pending], data, chunk);
            m_Pending += chunk;
            data      += chunk;
            data_size -= chunk;
            if(m_Pending == AP4_SHA256_BLOCK_SIZE)
            {
                CompressBlock(m_Buffer);
                m_Length += 8 * AP4_SHA256_BLOCK_SIZE;
                m_Pending = 0;
            }
        }
    }

    return AP4_SUCCESS;
}


/*----------------------------------------------------------------------
|   AP4_DigestSha256::Final
+---------------------------------------------------------------------*/
AP4_Result
AP4_DigestSha256::Final(AP4_DataBuffer& digest)
{
    /* increase the length of the message */
    m_Length += m_Pending * 8;

    /* append the '1' bit */
    m_Buffer[m_Pending++] = 0x80;

    /* if the length is currently above 56 bytes we append zeros
     * then compress.  Then we can fall back to padding zeros and length
     * encoding like normal.
     */
    if(m_Pending > 56)
    {
        while(m_Pending < 64)
        {
            m_Buffer[m_Pending++] = 0;
        }
        CompressBlock(m_Buffer);
        m_Pending = 0;
    }

    /* pad upto 56 bytes of zeroes */
    while(m_Pending < 56)
    {
        m_Buffer[m_Pending++] = 0;
    }

    /* store length */
    AP4_BytesFromUInt64BE(&m_Buffer[56], m_Length);
    CompressBlock(m_Buffer);

    /* copy output */
    digest.SetDataSize(32);
    AP4_UI08* out = digest.UseData();
    for(unsigned int i = 0; i < 8; i++)
    {
        AP4_BytesFromUInt32BE(out, m_State[i]);
        out += 4;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_HmacSha256::AP4_HmacSha256
+---------------------------------------------------------------------*/
AP4_HmacSha256::AP4_HmacSha256(const AP4_UI08* key, AP4_Size key_size)
{
    AP4_UI08 workspace[AP4_SHA256_BLOCK_SIZE];

    /* if the key is larger than the block size, use a digest of the key */
    AP4_DataBuffer hk;
    if(key_size > AP4_SHA256_BLOCK_SIZE)
    {
        AP4_DigestSha256 kdigest;
        kdigest.Update(key, key_size);
        kdigest.Final(hk);
        key = hk.GetData();
        key_size = hk.GetDataSize();
    }

    /* compute key XOR ipad */
    for(unsigned int i = 0; i < key_size; i++)
    {
        workspace[i] = key[i] ^ 0x36;
    }
    for(unsigned int i = key_size; i < AP4_SHA256_BLOCK_SIZE; i++)
    {
        workspace[i] = 0x36;
    }

    /* start the inner digest with (key XOR ipad) */
    m_InnerDigest.Update(workspace, AP4_SHA256_BLOCK_SIZE);

    /* compute key XOR opad */
    for(unsigned int i = 0; i < key_size; i++)
    {
        workspace[i] = key[i] ^ 0x5c;
    }
    for(unsigned int i = key_size; i < AP4_SHA256_BLOCK_SIZE; i++)
    {
        workspace[i] = 0x5c;
    }

    /* start the outer digest with (key XOR opad) */
    m_OuterDigest.Update(workspace, AP4_SHA256_BLOCK_SIZE);
}

/*----------------------------------------------------------------------
|   AP4_HmacSha256::Final
+---------------------------------------------------------------------*/
AP4_Result
AP4_HmacSha256::Final(AP4_DataBuffer& mac)
{
    /* finish the outer digest with the value of the inner digest */
    AP4_DataBuffer inner;
    m_InnerDigest.Final(inner);
    m_OuterDigest.Update(inner.GetData(), inner.GetDataSize());

    /* return the value of the outer digest */
    return m_OuterDigest.Final(mac);
}

/*----------------------------------------------------------------------
|   AP4_Hmac::Create
+---------------------------------------------------------------------*/
AP4_Result
AP4_Hmac::Create(Algorithm       algorithm,
                 const AP4_UI08* key,
                 AP4_Size        key_size,
                 AP4_Hmac*&      hmac)
{
    switch(algorithm)
    {
    case SHA256:
        hmac = new AP4_HmacSha256(key, key_size);
        return AP4_SUCCESS;
    default:
        hmac = NULL;
        return AP4_ERROR_NOT_SUPPORTED;
    }
}

