/*****************************************************************
|
|    AP4 - AVC Parser
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

#ifndef _AP4_AVC_PARSER_H_
#define _AP4_AVC_PARSER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Results.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   AP4_AvcParser
+---------------------------------------------------------------------*/
class AP4_AvcParser {
public:
    static const char* NaluTypeName(unsigned int nalu_type);
    static const char* PrimaryPicTypeName(unsigned int primary_pic_type);
    static const char* SliceTypeName(unsigned int slice_type);
    
    AP4_AvcParser();
    
    /**
     * Feed some data to the parser and look for the next NAL Unit.
     *
     * @param data: Pointer to the memory buffer with the data to feed.
     * @param data_size: Size in bytes of the buffer pointed to by the
     * data pointer.
     * @param bytes_consumed: Number of bytes from the data buffer that were
     * consumed and stored by the parser.
     * @param nalu: Reference to a pointer to a buffer object that contains
     * a NAL unit found in the previously fed data, or a NULL pointer if no 
     * NAL unit can be found so far.
     * @param eos: Boolean flag that indicates if this buffer is the last
     * buffer in the stream/file (End Of Stream).
     *
     * @result: AP4_SUCCESS is the call succeeds, or an error code if it
     * fails.
     * 
     * The caller must not feed the same data twice. When this method
     * returns, the caller should inspect the value of bytes_consumed and
     * advance the input stream source accordingly, such that the next
     * buffer passed to this method will be exactly bytes_consumed bytes
     * after what was passed in this call.
     */
    AP4_Result Feed(const void*            data, 
                    AP4_Size               data_size, 
                    AP4_Size&              bytes_consumed,
                    const AP4_DataBuffer*& nalu,
                    bool                   eos=false);
    
    /**
     * Reset the state of the parser (for example, to parse a new stream).
     */
    AP4_Result Reset();
    
private:
    enum {
        STATE_RESET,
        STATE_START_CODE_1,
        STATE_START_CODE_2,
        STATE_START_NALU,
        STATE_IN_NALU
    }              m_State;
    AP4_Cardinal   m_ZeroTrail;
    AP4_DataBuffer m_Buffer;
};

#endif // _AP4_AVC_PARSER_H_
