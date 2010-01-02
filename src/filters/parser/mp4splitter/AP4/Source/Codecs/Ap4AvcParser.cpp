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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4AvcParser.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   AP4_AvcParser::NaluTypeName
+---------------------------------------------------------------------*/
const char*
AP4_AvcParser::NaluTypeName(unsigned int nalu_type)
{
    switch (nalu_type) {
        case  0: return "Unspecified";
        case  1: return "Coded slice of a non-IDR picture";
        case  2: return "Coded slice data partition A"; 
        case  3: return "Coded slice data partition B";
        case  4: return "Coded slice data partition C";
        case  5: return "Coded slice of an IDR picture";
        case  6: return "Supplemental enhancement information (SEI)";
        case  7: return "Sequence parameter set";
        case  8: return "Picture parameter set";
        case  9: return "Access unit delimiter";
        case 10: return "End of sequence";
        case 11: return "End of stream";
        case 12: return "Filler data";
        case 13: return "Sequence parameter set extension";
        case 14: return "Prefix NAL unit in scalable extension";
        case 15: return "Subset sequence parameter set";
        case 19: return "Coded slice of an auxiliary coded picture without partitioning";
        case 20: return "Coded slice in scalable extension";
        default: return NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_AvcParser::PrimaryPicTypeName
+---------------------------------------------------------------------*/
const char*
AP4_AvcParser::PrimaryPicTypeName(unsigned int primary_pic_type)
{
	switch (primary_pic_type) {
        case 0: return "I";
        case 1: return "I, P";
        case 2: return "I, P, B";
        case 3: return "SI";
        case 4: return "SI, SP";
        case 5: return "I, SI";
        case 6: return "I, SI, P, SP";
        case 7: return "I, SI, P, SP, B";
        default: return NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_AvcParser::SliceTypeName
+---------------------------------------------------------------------*/
const char*
AP4_AvcParser::SliceTypeName(unsigned int slice_type)
{
	switch (slice_type) {
        case 0: return "P";
        case 1: return "B";
        case 2: return "I";
        case 3:	return "SP";
        case 4: return "SI";
        case 5: return "P";
        case 6: return "B";
        case 7: return "I";
        case 8:	return "SP";
        case 9: return "SI";
        default: return NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_AvcParser::AP4_AvcParser
+---------------------------------------------------------------------*/
AP4_AvcParser::AP4_AvcParser() :
    m_State(STATE_RESET),
    m_ZeroTrail(0)
{
}

/*----------------------------------------------------------------------
|   AP4_AvcParser::Feed
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AvcParser::Feed(const void*            data, 
                    AP4_Size               data_size, 
                    AP4_Size&              bytes_consumed,
                    const AP4_DataBuffer*& nalu,
                    bool                   is_eos)
{
    // default return values
    nalu = NULL;
    bytes_consumed = 0;
        
    // iterate the state machine
    unsigned int data_offset;
    unsigned int payload_start = 0;
    unsigned int payload_end  = 0;
    bool         found_nalu = false;
    for (data_offset=0; data_offset<data_size && !found_nalu; data_offset++) {
        unsigned char byte = ((const unsigned char*)data)[data_offset];
        switch (m_State) {
            case STATE_RESET:
                if (byte == 0) {
                    m_State = STATE_START_CODE_1;
                }
                break;
                
            case STATE_START_CODE_1:
                if (byte == 0) {
                    m_State = STATE_START_CODE_2;
                } else {
                    m_State = STATE_RESET;
                }
                break;

            case STATE_START_CODE_2:
                if (byte == 0) break;
                if (byte == 1) {
                    m_State = STATE_START_NALU;
                } else {
                    m_State = STATE_RESET;
                }
                break;

            case STATE_START_NALU:
                m_Buffer.SetDataSize(0);
                m_ZeroTrail = 0;
                payload_start = payload_end = data_offset;
                m_State = STATE_IN_NALU;
                // FALLTHROUGH
                
            case STATE_IN_NALU:
                if (byte == 0) {
                    m_ZeroTrail++;
                    break;
                } 
                if (m_ZeroTrail >= 2) {
                    if (byte == 1) {
                        found_nalu = true;
                        m_State = STATE_START_NALU;
                    } else {
                        payload_end += m_ZeroTrail+1;
                    }
                } else {
                    payload_end += m_ZeroTrail+1;
                }
                m_ZeroTrail = 0; 
                break;
        }
    }
    if (is_eos && m_State == STATE_IN_NALU && data_offset == data_size) {
        found_nalu = true;
        m_ZeroTrail = 0;
        m_State = STATE_RESET;
    }
    if (payload_end > payload_start) {
        AP4_Size current_payload_size = m_Buffer.GetDataSize();
        m_Buffer.SetDataSize(m_Buffer.GetDataSize()+(payload_end-payload_start));
        AP4_CopyMemory(((unsigned char *)m_Buffer.UseData())+current_payload_size, 
                       ((const unsigned char*)data)+payload_start, 
                       payload_end-payload_start);
    }
    
    // compute how many bytes we have consumed
    bytes_consumed = data_offset;
    
    // return the NALU if we found one
    if (found_nalu) nalu = &m_Buffer;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AvcParser::Reset
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AvcParser::Reset()
{
    m_State     = STATE_RESET;
    m_ZeroTrail = 0;
    m_Buffer.SetDataSize(0);
    
    return AP4_SUCCESS;
}
