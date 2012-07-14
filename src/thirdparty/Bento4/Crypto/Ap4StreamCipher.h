/*
 * Copyright (c) 2005 Gilles Boccon-Gibod
 */


#ifndef _AP4_STREAM_CIPHER_H_
#define _AP4_STREAM_CIPHER_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4AesBlockCipher.h"
#include "Ap4Results.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|       AP4_StreamCipher class
+---------------------------------------------------------------------*/
class AP4_StreamCipher
{
 public:
    // methods
    AP4_StreamCipher(const AP4_UI08* key = NULL, 
                     const AP4_UI08* salt = NULL,
                     AP4_Size        iv_size = 4);
    ~AP4_StreamCipher();
    AP4_Result SetStreamOffset(AP4_Offset offset);
    AP4_Result Reset(const AP4_UI08* key, const AP4_UI08* salt);
    AP4_Result ProcessBuffer(const AP4_UI08* in, 
                             AP4_UI08*       out,
                             AP4_Size        size);
    AP4_Offset GeStreamOffset() { return m_StreamOffset; }

 private:
    // members
    AP4_Offset          m_StreamOffset;
    AP4_Size            m_IvSize;
    AP4_UI08            m_CBlock[AP4_AES_BLOCK_SIZE];
    AP4_UI08            m_XBlock[AP4_AES_BLOCK_SIZE];
    AP4_AesBlockCipher* m_BlockCipher;

    // methods
    void SetCounter(AP4_Offset block_offset);
    AP4_Result UpdateKeyStream(AP4_Offset block_offset);
};

#endif // _AP4_STREAM_CIPHER_H_
