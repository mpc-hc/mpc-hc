/*****************************************************************
|
|      File: Ap4BitStream.c
|
|      AP4 - Bit Streams
|
|      (c) 2004 Gilles Boccon-Gibod
|      Author: Gilles Boccon-Gibod (bok@bok.net)
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       For efficiency reasons, this bitstream library only handles
|       data buffers that are a power of 2 in size
+---------------------------------------------------------------------*/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4BitStream.h"

/*----------------------------------------------------------------------
|       AP4_BitStream::AP4_BitStream
+---------------------------------------------------------------------*/
AP4_BitStream::AP4_BitStream()
{
    m_Buffer = new AP4_UI08[AP4_BITSTREAM_BUFFER_SIZE];
    Reset();
}

/*----------------------------------------------------------------------
|       AP4_BitStream::~AP4_BitStream
+---------------------------------------------------------------------*/
AP4_BitStream::~AP4_BitStream()
{
    delete[] m_Buffer;
}

/*----------------------------------------------------------------------
|       AP4_BitStream::Reset
+---------------------------------------------------------------------*/
AP4_Result
AP4_BitStream::Reset()
{
    m_In         = 0;
    m_Out        = 0;
    m_BitsCached = 0;
    m_Cache      = 0;
    m_Flags      = 0;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_BitStream::ByteAlign
+---------------------------------------------------------------------*/
AP4_Result   
AP4_BitStream::ByteAlign()
{
    unsigned int to_flush = m_BitsCached & 7;
    if (to_flush > 0) SkipBits(to_flush);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_BitStream::GetContiguousBytesFree
+---------------------------------------------------------------------*/
AP4_Size
AP4_BitStream::GetContiguousBytesFree()
{
    return 
        (m_In < m_Out) ?
        (m_Out - m_In - 1) :
        (m_Out == 0 ? (AP4_BITSTREAM_BUFFER_SIZE - m_In - 1) :
                      (AP4_BITSTREAM_BUFFER_SIZE - m_In));
}

/*----------------------------------------------------------------------
|       AP4_BitStream_GetBytesFree
+---------------------------------------------------------------------*/
AP4_Size
AP4_BitStream::GetBytesFree()
{
    return  
        (m_In < m_Out) ? 
        (m_Out - m_In - 1) : 
        (AP4_BITSTREAM_BUFFER_SIZE  + (m_Out - m_In) - 1);
}

/*----------------------------------------------------------------------+
|        AP4_BitStream::WriteBytes
+----------------------------------------------------------------------*/
AP4_Result
AP4_BitStream::WriteBytes(const AP4_UI08* bytes, 
                          AP4_Size        byte_count)
{
    /* check parameters */
    if (byte_count == 0) return AP4_SUCCESS;
    if (bytes == NULL) return AP4_ERROR_INVALID_PARAMETERS;

    /* check that we have enough space */
    if (GetBytesFree() < byte_count) {
        return AP4_FAILURE;
    }

    /* write the bytes */
    if (m_In < m_Out) {
        memcpy(m_Buffer+m_In, bytes, byte_count);
        AP4_BITSTREAM_POINTER_ADD(m_In, byte_count);
    } else {
        unsigned int chunk = AP4_BITSTREAM_BUFFER_SIZE - m_In;
        if (chunk > byte_count) chunk = byte_count;

        memcpy(m_Buffer+m_In, bytes, chunk);
        AP4_BITSTREAM_POINTER_ADD(m_In, chunk);

        if (chunk != byte_count) {
            memcpy(m_Buffer+m_In, 
                   bytes+chunk, byte_count-chunk);
            AP4_BITSTREAM_POINTER_ADD(m_In, byte_count-chunk);
        }
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_BitStream_GetContiguousBytesAvailable
+---------------------------------------------------------------------*/
AP4_Size
AP4_BitStream::GetContiguousBytesAvailable()
{
    return 
        (m_Out <= m_In) ? 
        (m_In - m_Out) :
        (AP4_BITSTREAM_BUFFER_SIZE - m_Out);
}

/*----------------------------------------------------------------------
|       AP4_BitStream::GetBytesAvailable
+---------------------------------------------------------------------*/
AP4_Size
AP4_BitStream::GetBytesAvailable()
{
    return 
        (m_Out <= m_In) ? 
        (m_In - m_Out) :
        (m_In + (AP4_BITSTREAM_BUFFER_SIZE - m_Out));
}

/*----------------------------------------------------------------------+
|        AP4_BitStream::ReadBytes
+----------------------------------------------------------------------*/
AP4_Result
AP4_BitStream::ReadBytes(AP4_UI08* bytes, 
                         AP4_Size  byte_count)
{
   if (byte_count == 0 || bytes == NULL) {
      return AP4_ERROR_INVALID_PARAMETERS;
   }

   /* Gets bytes from the cache */
   ByteAlign();
   while (m_BitsCached > 0 && byte_count > 0) {
      *bytes = ReadBits(8);
      ++ bytes;
      -- byte_count;
   }

   /* Get other bytes */
   if (byte_count > 0) {
      if (m_Out < m_In) {
         memcpy(bytes, m_Buffer + m_Out, byte_count);
         AP4_BITSTREAM_POINTER_ADD(m_Out, byte_count);
      } else {
         unsigned int chunk = AP4_BITSTREAM_BUFFER_SIZE - m_Out;
         if (chunk >= byte_count) chunk = byte_count;

         memcpy(bytes, m_Buffer+m_Out, chunk);
         AP4_BITSTREAM_POINTER_ADD(m_Out, chunk);

         if (chunk != byte_count) {
            memcpy(bytes+chunk, 
                   m_Buffer+m_Out, 
                   byte_count-chunk);
            AP4_BITSTREAM_POINTER_ADD(m_Out, byte_count-chunk);
         }
      }
   }

   return AP4_SUCCESS;
}

/*----------------------------------------------------------------------+
|        AP4_BitStream::PeekBytes
+----------------------------------------------------------------------*/
AP4_Result
AP4_BitStream::PeekBytes(AP4_UI08* bytes, 
                         AP4_Size  byte_count)
{
   int bits_cached_byte;

   if (byte_count == 0 || bytes == NULL) {
      return AP4_ERROR_INVALID_PARAMETERS;
   }

   /* Gets bytes from the cache */
   bits_cached_byte = m_BitsCached & ~7;
   while (bits_cached_byte > 0 && byte_count > 0) {
      *bytes = (m_Cache >> bits_cached_byte) & 0xFF;
      ++ bytes;
      -- byte_count;
      bits_cached_byte -= 8;
   }

   /* Get other bytes */
   if (byte_count > 0) {
      if (m_In > m_Out) {
         memcpy(bytes, m_Buffer + m_Out, byte_count);
      } else {
         unsigned int out = m_Out;
         unsigned int chunk = AP4_BITSTREAM_BUFFER_SIZE - out;
         if (chunk >= byte_count) {
            chunk = byte_count;
         }

         memcpy(bytes, m_Buffer+out, chunk);
         AP4_BITSTREAM_POINTER_ADD(out, chunk);

         if (chunk != byte_count) {
            memcpy(bytes+chunk, 
                   m_Buffer+out, 
                   byte_count-chunk);
         }
      }
   }

   return AP4_SUCCESS;
}

/*----------------------------------------------------------------------+
|        AP4_BitStream::SkipBytes
+----------------------------------------------------------------------*/
AP4_Result
AP4_BitStream::SkipBytes(AP4_Size byte_count)
{
    AP4_BITSTREAM_POINTER_ADD(m_Out, byte_count);
    return AP4_SUCCESS;
}
