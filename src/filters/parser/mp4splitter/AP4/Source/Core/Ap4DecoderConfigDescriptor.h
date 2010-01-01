/*****************************************************************
|
|    AP4 - DecoderConfig Descriptor 
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

#ifndef _AP4_DECODER_CONFIG_DESCRIPTOR_H_
#define _AP4_DECODER_CONFIG_DESCRIPTOR_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4List.h"
#include "Ap4Descriptor.h"
#include "Ap4DecoderSpecificInfoDescriptor.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_DESCRIPTOR_TAG_DECODER_CONFIG = 0x04;

/*----------------------------------------------------------------------
|   AP4_DecoderConfigDescriptor
+---------------------------------------------------------------------*/
class AP4_DecoderConfigDescriptor : public AP4_Descriptor
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_DecoderConfigDescriptor, AP4_Descriptor)

    // methods
    AP4_DecoderConfigDescriptor(AP4_UI08 stream_type,
                                AP4_UI08 oti,
                                AP4_UI32 buffer_size,
                                AP4_UI32 max_bitrate,
                                AP4_UI32 avg_bitrate,
                                AP4_DecoderSpecificInfoDescriptor* dsi);
    AP4_DecoderConfigDescriptor(AP4_ByteStream& stream, 
                                AP4_Size        header_size,
                                AP4_Size        payload_size);
    virtual ~AP4_DecoderConfigDescriptor();
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);
 
    // accessors
    virtual const AP4_DecoderSpecificInfoDescriptor*
        GetDecoderSpecificInfoDescriptor() const;
    virtual AP4_UI08 GetObjectTypeIndication() const { 
        return m_ObjectTypeIndication; 
    }
    virtual AP4_UI08 GetStreamType() const { return m_StreamType; } 
    virtual AP4_UI32 GetBufferSize() const { return m_BufferSize; }
    virtual AP4_UI32 GetMaxBitrate() const { return m_MaxBitrate; }
    virtual AP4_UI32 GetAvgBitrate() const { return m_AverageBitrate; }
    
 private:
    // members
    AP4_UI08                         m_StreamType;
    AP4_UI08                         m_ObjectTypeIndication;
    bool                             m_UpStream;
    AP4_UI32                         m_BufferSize;
    AP4_UI32                         m_MaxBitrate;
    AP4_UI32                         m_AverageBitrate;
    mutable AP4_List<AP4_Descriptor> m_SubDescriptors;
};

#endif // _AP4_DECODER_CONFIG_DESCRIPTOR_H_
