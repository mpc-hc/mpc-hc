/*
 * Copyright (c) 2005 Gilles Boccon-Gibod 
 */

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4StreamCipher.h"

/*----------------------------------------------------------------------
|       AP4_StreamCipher::AP4_StreamCipher
+---------------------------------------------------------------------*/
AP4_StreamCipher::AP4_StreamCipher(const AP4_UI08* key,
                                   const AP4_UI08* salt,
                                   AP4_Size        iv_size) :
    m_StreamOffset(0),
    m_IvSize(iv_size),
    m_BlockCipher(NULL)
{
    // clamp the IV size to the max supported size
    if (iv_size > 4) {
        m_IvSize = 4;
    }

    // set the initial state
    Reset(key, salt);
}

/*----------------------------------------------------------------------
|       AP4_StreamCipher::~AP4_StreamCipher
+---------------------------------------------------------------------*/
AP4_StreamCipher::~AP4_StreamCipher()
{
    // delete the block cipher
    if (m_BlockCipher) {
        delete m_BlockCipher;
    }
}

/*----------------------------------------------------------------------
|       AP4_StreamCipher::Reset
+---------------------------------------------------------------------*/
AP4_Result
AP4_StreamCipher::Reset(const AP4_UI08* key, const AP4_UI08* salt) 
{
    if (salt) {
        // initialize the counter with a salting key
        for (AP4_UI32 i=0; i<AP4_AES_BLOCK_SIZE; i++) {
            m_CBlock[i] = salt[i];
        }
    } else {
        // initialize the counter with no salting key
        for (AP4_UI32 i = 0; i < AP4_AES_BLOCK_SIZE; i++) {
            m_CBlock[i] = 0;
        }
    }

    // (re)create the block cipher
    if (key != NULL) {
        // delete the block cipher if needed
        if (m_BlockCipher) {
            delete m_BlockCipher;
        }
        
        // (re)create one
        m_BlockCipher = new AP4_AesBlockCipher(key);
    }

    // reset the stream offset
    SetStreamOffset(0);
  
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_StreamCipher::SetCounter
+---------------------------------------------------------------------*/
void
AP4_StreamCipher::SetCounter(AP4_Offset block_offset)
{
    // set the counter bytes
    for (AP4_UI32 i = 0; i < m_IvSize; i++) {
        m_CBlock[AP4_AES_BLOCK_SIZE-1-i] = 
			(AP4_UI08)((block_offset>>(8*i)) & 0xFF);
    }
}

/*----------------------------------------------------------------------
|       AP4_StreamCipher::SetStreamOffset
+---------------------------------------------------------------------*/
AP4_Result
AP4_StreamCipher::SetStreamOffset(AP4_Offset offset)
{
    // do nothing if we're already at that offset
    if (offset == m_StreamOffset) return AP4_SUCCESS;

    // update the offset
    m_StreamOffset = offset;

    // update the key stream if necessary
    if (m_StreamOffset & 0xF) {
        return UpdateKeyStream(m_StreamOffset/AP4_AES_BLOCK_SIZE);
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_StreamCipher::UpdateKeyStream
+---------------------------------------------------------------------*/
AP4_Result
AP4_StreamCipher::UpdateKeyStream(AP4_Offset block_offset)
{
    // compute the new counter
    SetCounter(block_offset);

    // compute the key block (x) from the counter block (c)
    return m_BlockCipher->EncryptBlock(m_CBlock, m_XBlock);
}

/*----------------------------------------------------------------------
|       AP4_StreamCipher::ProcessBuffer
+---------------------------------------------------------------------*/
AP4_Result
AP4_StreamCipher::ProcessBuffer(const AP4_UI08* in, 
                                AP4_UI08*       out, 
                                AP4_Size        size)
{
    if (m_BlockCipher == NULL) return AP4_ERROR_INVALID_STATE;

    while (size) {
        // compute the number of bytes available in this chunk
        AP4_UI32 index = m_StreamOffset & (AP4_AES_BLOCK_SIZE-1);
        AP4_UI32 chunk;

        // update the key stream if we are on a boundary
        if (index == 0) {
            UpdateKeyStream(m_StreamOffset/AP4_AES_BLOCK_SIZE);
            chunk = AP4_AES_BLOCK_SIZE;
        }

        // compute the number of bytes remaining in the chunk
        chunk = AP4_AES_BLOCK_SIZE - index;
        if (chunk > size) chunk = size;

        // encrypt/decrypt the chunk
        AP4_UI08* x = &m_XBlock[index];
        for (AP4_UI32 i = 0; i < chunk; i++) {
            *out++ = *in++ ^ *x++;
        }

        // update offset and size
        m_StreamOffset += chunk;
        size -= chunk;
    }

    return AP4_SUCCESS;
}
