/*****************************************************************
|
|    AP4 - DecoderConfig Descriptors
|
|    Copyright 2002 Gilles Boccon-Gibod
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
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4DecoderConfigDescriptor.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_DecoderConfigDescriptor::AP4_DecoderConfigDescriptor
+---------------------------------------------------------------------*/
AP4_DecoderConfigDescriptor::AP4_DecoderConfigDescriptor(
    AP4_UI08 stream_type,
    AP4_UI08 oti,
    AP4_UI32 buffer_size,
    AP4_UI32 max_bitrate,
    AP4_UI32 avg_bitrate,
    AP4_DecoderSpecificInfoDescriptor* dsi) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_DECODER_CONFIG, 2, 13),
    m_StreamType(stream_type),
    m_ObjectTypeIndication(oti),
    m_UpStream(false),
    m_BufferSize(buffer_size),
    m_MaxBitrate(max_bitrate),
    m_AverageBitrate(avg_bitrate)
{
    if (dsi) {
        m_SubDescriptors.Add(dsi);
        m_PayloadSize += dsi->GetSize();
        m_HeaderSize = MinHeaderSize(m_PayloadSize);
    }
}

/*----------------------------------------------------------------------
|       AP4_DecoderConfigDescriptor::AP4_DecoderConfigDescriptor
+---------------------------------------------------------------------*/
AP4_DecoderConfigDescriptor::AP4_DecoderConfigDescriptor(
    AP4_ByteStream& stream, AP4_Size header_size, AP4_Size payload_size) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_DECODER_CONFIG, 
                   header_size, 
                   payload_size)
{
    // record the start position
    AP4_Offset start;
    stream.Tell(start);

    // read descriptor fields
    stream.ReadUI08(m_ObjectTypeIndication);
    unsigned char bits;
    stream.ReadUI08(bits);
    m_StreamType = (bits>>2)&0x3F;
    m_UpStream   = bits&2 ? true:false; 
    stream.ReadUI24(m_BufferSize);
    stream.ReadUI32(m_MaxBitrate);
    stream.ReadUI32(m_AverageBitrate);

    // read other descriptors
    AP4_SubStream* substream = new AP4_SubStream(stream, start+13, payload_size-13);
    AP4_Descriptor* descriptor = NULL;
    while (AP4_DescriptorFactory::CreateDescriptorFromStream(*substream, 
                                                             descriptor) 
           == AP4_SUCCESS) {
        m_SubDescriptors.Add(descriptor);
    }
    substream->Release();
}

/*----------------------------------------------------------------------
|       AP4_DecoderConfigDescriptor::~AP4_DecoderConfigDescriptor
+---------------------------------------------------------------------*/
AP4_DecoderConfigDescriptor::~AP4_DecoderConfigDescriptor()
{
    m_SubDescriptors.DeleteReferences();
}

/*----------------------------------------------------------------------
|       AP4_DecoderConfigDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DecoderConfigDescriptor::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI08(m_ObjectTypeIndication);
    AP4_UI08 bits = (m_StreamType<<2) | (m_UpStream? 2 : 0) | 1;
    stream.WriteUI08(bits);
    stream.WriteUI24(m_BufferSize);
    stream.WriteUI32(m_MaxBitrate);
    stream.WriteUI32(m_AverageBitrate);

    m_SubDescriptors.Apply(AP4_DescriptorListWriter(stream));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_DecoderConfigDescriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_DecoderConfigDescriptor::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_StringFormat(info, sizeof(info), "size=%ld+%ld", 
        GetHeaderSize(),
        m_PayloadSize);
    inspector.StartElement("#[DecoderConfig]", info);
    inspector.AddField("stream_type", m_StreamType);
    inspector.AddField("object_type", m_ObjectTypeIndication);
    inspector.AddField("up_stream", m_UpStream);
    inspector.AddField("buffer_size", m_BufferSize);
    inspector.AddField("max_bitrate", m_MaxBitrate);
    inspector.AddField("avg_bitrate", m_AverageBitrate);

    // inspect children
    m_SubDescriptors.Apply(AP4_DescriptorListInspector(inspector));

    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_DecoderConfigDescriptor::GetDecoderSpecificInfoDescriptor
+---------------------------------------------------------------------*/
const AP4_DecoderSpecificInfoDescriptor*
AP4_DecoderConfigDescriptor::GetDecoderSpecificInfoDescriptor() const
{
    // find the decoder specific info
    AP4_Descriptor* descriptor = NULL;
    AP4_Result result = 
        m_SubDescriptors.Find(AP4_DescriptorFinder(AP4_DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO),
                              descriptor);
    
    // return it
    if (AP4_SUCCEEDED(result)) {
        return dynamic_cast<AP4_DecoderSpecificInfoDescriptor*>(descriptor);
    } else {
        return NULL;
    }
}
