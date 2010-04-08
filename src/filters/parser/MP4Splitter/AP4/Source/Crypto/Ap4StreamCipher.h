/*****************************************************************
|
|    AP4 - Stream Cipher
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
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

#ifndef _AP4_STREAM_CIPHER_H_
#define _AP4_STREAM_CIPHER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Protection.h"
#include "Ap4Results.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
// we only support this for now
const unsigned int AP4_CIPHER_BLOCK_SIZE = 16;

/*----------------------------------------------------------------------
|   AP4_StreamCipher interface
+---------------------------------------------------------------------*/
class AP4_StreamCipher
{
public:
    // types
    typedef enum
    {
        ENCRYPT,
        DECRYPT
    } CipherDirection;

    // methods
    virtual            ~AP4_StreamCipher() {}

    virtual AP4_UI64    GetStreamOffset() = 0;

    virtual AP4_Result  ProcessBuffer(const AP4_UI08* in,
                                      AP4_Size        in_size,
                                      AP4_UI08*       out,
                                      AP4_Size*       out_size,
                                      bool            is_last_buffer = false) = 0;

    // preroll gives the number of bytes you have to preroll your input and feed
    // it through ProcessBuffer (in one shot) in order to be able to spit out
    // the output at the given offset
    virtual AP4_Result  SetStreamOffset(AP4_UI64      offset,
                                        AP4_Cardinal* preroll) = 0;

    virtual AP4_Result SetIV(const AP4_UI08* iv) = 0;
    virtual const AP4_UI08* GetIV() = 0;
};


/*----------------------------------------------------------------------
|   AP4_CtrStreamCipher
+---------------------------------------------------------------------*/
class AP4_CtrStreamCipher : public AP4_StreamCipher
{
public:
    // methods

    /**
     * The block cipher is passed with transfer of ownership (it will
     * be destroyed when this object is destroyed).
     */
    AP4_CtrStreamCipher(AP4_BlockCipher* block_cipher,
                        const AP4_UI08*  salt,
                        AP4_Size         counter_size);
    ~AP4_CtrStreamCipher();

    // AP4_StreamCipher implementation
    virtual AP4_Result      SetStreamOffset(AP4_UI64      offset,
                                            AP4_Cardinal* preroll = NULL);
    virtual AP4_UI64        GetStreamOffset()
    {
        return m_StreamOffset;
    }
    virtual AP4_Result      ProcessBuffer(const AP4_UI08* in,
                                          AP4_Size        in_size,
                                          AP4_UI08*       out,
                                          AP4_Size*       out_size       = NULL,
                                          bool            is_last_buffer = false);

    virtual AP4_Result      SetIV(const AP4_UI08* iv);
    virtual const AP4_UI08* GetIV()
    {
        return m_BaseCounter;
    }

private:
    // methods
    void SetCounterOffset(AP4_UI32 offset);
    void UpdateKeyStream();

    // members
    AP4_UI64         m_StreamOffset;
    AP4_Size         m_CounterSize;
    AP4_UI08         m_BaseCounter[AP4_CIPHER_BLOCK_SIZE];
    AP4_UI08         m_XBlock[AP4_CIPHER_BLOCK_SIZE];
    AP4_BlockCipher* m_BlockCipher;
};

/*----------------------------------------------------------------------
|   AP4_CbcStreamCipher
+---------------------------------------------------------------------*/
class AP4_CbcStreamCipher : public AP4_StreamCipher
{
public:
    // methods

    /**
     * The block cipher is passed with transfer of ownership (it will
     * be destroyed when this object is destroyed).
     */
    AP4_CbcStreamCipher(AP4_BlockCipher* block_cipher, CipherDirection direction);
    ~AP4_CbcStreamCipher();

    // AP4_StreamCipher implementation
    virtual AP4_Result      SetStreamOffset(AP4_UI64      offset,
                                            AP4_Cardinal* preroll);
    virtual AP4_UI64        GetStreamOffset()
    {
        return m_StreamOffset;
    }
    virtual AP4_Result      ProcessBuffer(const AP4_UI08* in,
                                          AP4_Size        in_size,
                                          AP4_UI08*       out,
                                          AP4_Size*       out_size,
                                          bool            is_last_buffer = false);
    virtual AP4_Result      SetIV(const AP4_UI08* iv);
    virtual const AP4_UI08* GetIV()
    {
        return m_Iv;
    };

private:
    // members
    CipherDirection  m_Direction;
    AP4_UI64         m_StreamOffset;
    AP4_Size         m_OutputSkip;
    AP4_UI08         m_InBlockCache[AP4_CIPHER_BLOCK_SIZE];
    AP4_UI08         m_OutBlockCache[AP4_CIPHER_BLOCK_SIZE];
    AP4_UI08         m_Iv[AP4_CIPHER_BLOCK_SIZE];
    AP4_BlockCipher* m_BlockCipher;
    bool             m_Eos;
    AP4_Cardinal     m_PrerollByteCount;
};

#endif // _AP4_STREAM_CIPHER_H_
