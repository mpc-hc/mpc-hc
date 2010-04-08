/*****************************************************************
|
|    AP4 - Bitstream Utility
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

#ifndef _AP4_BIT_STREAM_H_
#define _AP4_BIT_STREAM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Results.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const int AP4_ERROR_BASE_BITSTREAM   = -10000;

// the max frame size we can handle
const unsigned int AP4_BITSTREAM_BUFFER_SIZE  = 8192;

// flags
#define AP4_BITSTREAM_FLAG_EOS 0x01

// error codes
const int AP4_ERROR_CORRUPTED_BITSTREAM    = (AP4_ERROR_BASE_BITSTREAM - 0);
const int AP4_ERROR_NOT_ENOUGH_FREE_BUFFER = (AP4_ERROR_BASE_BITSTREAM - 1);

/*----------------------------------------------------------------------
|   types helpers
+---------------------------------------------------------------------*/
/* use long by default */
typedef unsigned int AP4_BitsWord;
#define AP4_WORD_BITS  32
#define AP4_WORD_BYTES 4

/*----------------------------------------------------------------------
|   types
+---------------------------------------------------------------------*/
class AP4_BitStream
{
public:
    // constructor and destructor
    AP4_BitStream();
    ~AP4_BitStream();

    // methods
    AP4_Result   Reset();
    AP4_Size     GetContiguousBytesFree();
    AP4_Size     GetBytesFree();
    AP4_Result   WriteBytes(const AP4_UI08* bytes, AP4_Size byte_count);
    AP4_Size     GetContiguousBytesAvailable();
    AP4_Size     GetBytesAvailable();
    AP4_UI08     ReadByte();
    AP4_Result   ReadBytes(AP4_UI08* bytes, AP4_Size byte_count);
    AP4_UI08     PeekByte();
    AP4_Result   PeekBytes(AP4_UI08* bytes, AP4_Size byte_count);
    int          ReadBit();
    AP4_UI32     ReadBits(unsigned int bit_count);
    int          PeekBit();
    AP4_UI32     PeekBits(unsigned int bit_count);
    AP4_Result   SkipBytes(AP4_Size byte_count);
    void         SkipBit();
    void         SkipBits(unsigned int bit_count);
    AP4_Result   ByteAlign();

    // members
    AP4_UI08*    m_Buffer;
    unsigned int m_In;
    unsigned int m_Out;
    AP4_BitsWord m_Cache;
    unsigned int m_BitsCached;
    unsigned int m_Flags;

private:
    // methods
    AP4_BitsWord ReadCache() const;
};

/*----------------------------------------------------------------------
|   macros
+---------------------------------------------------------------------*/
#define AP4_BIT_MASK(_n) ((1<<(_n))-1)

#define AP4_BITSTREAM_POINTER_VAL(offset) \
    ((offset)&(AP4_BITSTREAM_BUFFER_SIZE-1))

#define AP4_BITSTREAM_POINTER_OFFSET(pointer, offset) \
    (AP4_BITSTREAM_POINTER_VAL((pointer)+(offset)))

#define AP4_BITSTREAM_POINTER_ADD(pointer, offset) \
    ((pointer) = AP4_BITSTREAM_POINTER_OFFSET(pointer, offset))

/*----------------------------------------------------------------------
|   AP4_BitStream::ReadCache
+---------------------------------------------------------------------*/
inline AP4_BitsWord
AP4_BitStream::ReadCache() const
{
    unsigned int pos = m_Out;
    AP4_BitsWord cache;

#if AP4_WORD_BITS != 32
#error unsupported word size /* 64 and other word size not yet implemented */
#endif

    if(pos <= AP4_BITSTREAM_BUFFER_SIZE - AP4_WORD_BYTES)
    {
        unsigned char* out_ptr = &m_Buffer[pos];
        cache = (((AP4_BitsWord) out_ptr[0]) << 24)
                | (((AP4_BitsWord) out_ptr[1]) << 16)
                | (((AP4_BitsWord) out_ptr[2]) <<  8)
                | (((AP4_BitsWord) out_ptr[3]));
    }
    else
    {
        unsigned char* buf_ptr = m_Buffer;
        cache = (((AP4_BitsWord) buf_ptr[                              pos    ]) << 24)
                | (((AP4_BitsWord) buf_ptr[AP4_BITSTREAM_POINTER_OFFSET(pos, 1)]) << 16)
                | (((AP4_BitsWord) buf_ptr[AP4_BITSTREAM_POINTER_OFFSET(pos, 2)]) <<  8)
                | (((AP4_BitsWord) buf_ptr[AP4_BITSTREAM_POINTER_OFFSET(pos, 3)]));
    }

    return cache;
}

/*----------------------------------------------------------------------
|   AP4_BitStream::ReadBits
+---------------------------------------------------------------------*/
inline AP4_UI32
AP4_BitStream::ReadBits(unsigned int n)
{
    AP4_BitsWord   result;
    if(m_BitsCached >= n)
    {
        /* we have enough bits in the cache to satisfy the request */
        m_BitsCached -= n;
        result = (m_Cache >> m_BitsCached) & AP4_BIT_MASK(n);
    }
    else
    {
        /* not enough bits in the cache */
        AP4_BitsWord word;

        /* read the next word */
        {
            word = ReadCache();
            m_Out = AP4_BITSTREAM_POINTER_OFFSET(m_Out, AP4_WORD_BYTES);
        }

        /* combine the new word and the cache, and update the state */
        {
            AP4_BitsWord cache = m_Cache & AP4_BIT_MASK(m_BitsCached);
            n -= m_BitsCached;
            m_BitsCached = AP4_WORD_BITS - n;
            result = (word >> m_BitsCached) | (cache << n);
            m_Cache = word;
        }
    }

    return result;
}

/*----------------------------------------------------------------------
|   AP4_BitStream::ReadBit
+---------------------------------------------------------------------*/
inline int
AP4_BitStream::ReadBit()
{
    AP4_BitsWord result;
    if(m_BitsCached == 0)
    {
        /* the cache is empty */

        /* read the next word into the cache */
        m_Cache = ReadCache();
        m_Out = AP4_BITSTREAM_POINTER_OFFSET(m_Out, AP4_WORD_BYTES);
        m_BitsCached = AP4_WORD_BITS - 1;

        /* return the first bit */
        result = m_Cache >> (AP4_WORD_BITS - 1);
    }
    else
    {
        /* get the bit from the cache */
        result = (m_Cache >> (--m_BitsCached)) & 1;
    }
    return result;
}

/*----------------------------------------------------------------------
|   AP4_BitStream::PeekBits
+---------------------------------------------------------------------*/
inline AP4_UI32
AP4_BitStream::PeekBits(unsigned int n)
{
    /* we have enough bits in the cache to satisfy the request */
    if(m_BitsCached >= n)
    {
        return (m_Cache >> (m_BitsCached - n)) & AP4_BIT_MASK(n);
    }
    else
    {
        /* not enough bits in the cache, read the next word */
        AP4_BitsWord word = ReadCache();

        /* combine the new word and the cache, and update the state */
        AP4_BitsWord   cache = m_Cache & AP4_BIT_MASK(m_BitsCached);
        n -= m_BitsCached;
        return (word >> (AP4_WORD_BITS - n)) | (cache << n);
    }
}

/*----------------------------------------------------------------------
|   AP4_BitStream::PeekBit
+---------------------------------------------------------------------*/
inline int
AP4_BitStream::PeekBit()
{
    /* the cache is empty */
    if(m_BitsCached == 0)
    {
        /* read the next word into the cache */
        AP4_BitsWord cache = ReadCache();

        /* return the first bit */
        return cache >> (AP4_WORD_BITS - 1);
    }
    else
    {
        /* get the bit from the cache */
        return (m_Cache >> (m_BitsCached - 1)) & 1;
    }
}

/*----------------------------------------------------------------------
|   AP4_BitStream::SkipBits
+---------------------------------------------------------------------*/
inline void
AP4_BitStream::SkipBits(unsigned int n)
{
    if(n <= m_BitsCached)
    {
        m_BitsCached -= n;
    }
    else
    {
        n -= m_BitsCached;
        while(n >= AP4_WORD_BITS)
        {
            m_Out = AP4_BITSTREAM_POINTER_OFFSET(m_Out, AP4_WORD_BYTES);
            n -= AP4_WORD_BITS;
        }
        if(n)
        {
            m_Cache = ReadCache();
            m_BitsCached = AP4_WORD_BITS - n;
            m_Out = AP4_BITSTREAM_POINTER_OFFSET(m_Out, AP4_WORD_BYTES);
        }
        else
        {
            m_BitsCached = 0;
            m_Cache = 0;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_BitStream::SkipBit
+---------------------------------------------------------------------*/
inline void
AP4_BitStream::SkipBit()
{
    if(m_BitsCached == 0)
    {
        m_Cache = ReadCache();
        m_Out = AP4_BITSTREAM_POINTER_OFFSET(m_Out, AP4_WORD_BYTES);
        m_BitsCached = AP4_WORD_BITS - 1;
    }
    else
    {
        --m_BitsCached;
    }
}

/*----------------------------------------------------------------------
|   AP4_BitStream::ReadByte
+---------------------------------------------------------------------*/
inline AP4_UI08
AP4_BitStream::ReadByte()
{
    SkipBits(m_BitsCached & 7);
    return ReadBits(8);
}

/*----------------------------------------------------------------------
|   AP4_BitStream::PeekByte
+---------------------------------------------------------------------*/
inline AP4_UI08
AP4_BitStream::PeekByte()
{
    int extra_bits = m_BitsCached & 7;
    int data = PeekBits(extra_bits + 8);
    int byte = data & 0xFF;

    return byte;
}

#endif // _AP4_BIT_STREAM_H_
