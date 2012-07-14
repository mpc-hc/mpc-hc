/*****************************************************************
|
|    AP4 - RTP Hint Objects
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4RtpHint.h"
#include "Ap4ByteStream.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|       AP4_RtpSampleData::~AP4_RtpSampleData
+---------------------------------------------------------------------*/
AP4_RtpSampleData::~AP4_RtpSampleData()
{
    AP4_List<AP4_RtpPacket>::Item* it = m_Packets.FirstItem();
    while (it != NULL) {
        it->GetData()->Release();
        it = it->GetNext();
    }
}

/*----------------------------------------------------------------------
|       AP4_RtpSampleData::AP4_RtpSampleData
+---------------------------------------------------------------------*/
AP4_RtpSampleData::AP4_RtpSampleData(AP4_ByteStream& stream, AP4_Size size)
{
    // save the start position
    AP4_Offset start, extra_data_start;
    stream.Tell(start);

    AP4_UI16 packet_count;
    stream.ReadUI16(packet_count);
    
    AP4_UI16 reserved;
    stream.ReadUI16(reserved); // later, check that reserved is 0

    // packets
    for (AP4_UI16 i=0; i<packet_count; i++) {
        AP4_RtpPacket* packet = new AP4_RtpPacket(stream);
        m_Packets.Add(packet);
    }

    // extra data
    stream.Tell(extra_data_start);
    AP4_Size extra_data_size = size - (extra_data_start-start);
    if (extra_data_size != 0) {
        m_ExtraData.SetDataSize(extra_data_size);
        stream.Read(m_ExtraData.UseData(), extra_data_size);
    }
}

/*----------------------------------------------------------------------
|       AP4_RtpSampleData::GetSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_RtpSampleData::GetSize()
{
    // packet count and reserved
    AP4_Size result = 4;

    // packets
    AP4_List<AP4_RtpPacket>::Item* it = m_Packets.FirstItem();
    while (it != NULL) {
        result = it->GetData()->GetSize();
        it = it->GetNext();
    }

    // extra data
    result += m_ExtraData.GetDataSize();

    return result;
}

/*----------------------------------------------------------------------
|       AP4_RtpSampleData::ToByteStream
+---------------------------------------------------------------------*/
AP4_ByteStream*
AP4_RtpSampleData::ToByteStream()
{
    // refresh the size
    AP4_Size size = GetSize();

    // create a memory stream
    AP4_MemoryByteStream* stream = new AP4_MemoryByteStream(size);

    // write in it
    AP4_Result result = stream->WriteUI16(static_cast<AP4_UI16>(m_Packets.ItemCount()));
    if (AP4_FAILED(result)) goto bail;

    result = stream->WriteUI16(0); // reserved
    if (AP4_FAILED(result)) goto bail;

    {
		AP4_List<AP4_RtpPacket>::Item* it = m_Packets.FirstItem();
		while (it != NULL) {
			result = it->GetData()->Write(*stream);
			if (AP4_FAILED(result)) goto bail;
			it = it->GetNext();
		}
	}

    result = stream->Write(m_ExtraData.GetData(), m_ExtraData.GetDataSize());
    if (AP4_FAILED(result)) goto bail;

    // return
    return stream;

bail:
    stream->Release();
    return NULL;
}


/*----------------------------------------------------------------------
|       AP4_RtpSampleData::AddPacket
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpSampleData::AddPacket(AP4_RtpPacket* packet)
{
    packet->AddReference();
    return m_Packets.Add(packet);
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::AP4_RtpPacket
+---------------------------------------------------------------------*/
AP4_RtpPacket::AP4_RtpPacket(AP4_Integer relative_time, 
                             bool p_bit, 
                             bool x_bit, 
                             bool m_bit, 
                             AP4_UI08 payload_type, 
                             AP4_UI16 sequence_seed, 
                             AP4_Integer time_stamp_offset /* = 0 */,
                             bool bframe_flag /* = false */, 
                             bool repeat_flag /* = false */) :
    m_ReferenceCount(1),
    m_RelativeTime(relative_time),
    m_PBit(p_bit),
    m_XBit(x_bit),
    m_MBit(m_bit),
    m_PayloadType(payload_type),
    m_SequenceSeed(sequence_seed),
    m_TimeStampOffset(time_stamp_offset),
    m_BFrameFlag(bframe_flag),
    m_RepeatFlag(repeat_flag)
{}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::AP4_RtpPacket
+---------------------------------------------------------------------*/
AP4_RtpPacket::AP4_RtpPacket(AP4_ByteStream& stream) :
    m_ReferenceCount(1),
    m_TimeStampOffset(0)
{
    AP4_UI08 octet;

    // relative time
    AP4_UI32 relative_time;
    stream.ReadUI32(relative_time);
    m_RelativeTime = relative_time;

    // pbit and xbit
    stream.ReadUI08(octet);
    m_PBit = (octet & 0x20) != 0;
    m_XBit = (octet & 0x10) != 0;

    // mbit and payload type
    stream.ReadUI08(octet);
    m_MBit = (octet & 0x80) != 0;
    m_PayloadType = octet & 0x7F;

    // sequence seed
    stream.ReadUI16(m_SequenceSeed);

    // extra, bframe and repeat flags
    stream.ReadUI08(octet);
    stream.ReadUI08(octet); // repeat on purpose
    bool extra_flag = (octet & 0x04) != 0;

    // bframe and repeat flags
    m_BFrameFlag = (octet & 0x02) != 0;
    m_RepeatFlag = (octet & 0x01) != 0;

    // constructor count
    AP4_UI16 constructor_count;
    stream.ReadUI16(constructor_count);

    // parse the packet extra data
    if (extra_flag) {
        // read the length
        AP4_UI32 extra_length;
        stream.ReadUI32(extra_length);

        // check it 
        if (extra_length < 4) 
            throw AP4_Exception(AP4_ERROR_INVALID_RTP_PACKET_EXTRA_DATA);

        // now read the entries
        extra_length -= 4;
        while (extra_length > 0) {
            AP4_UI32 entry_length;
            AP4_UI32 entry_tag;
            stream.ReadUI32(entry_length);
            stream.ReadUI32(entry_tag);

            // check the entry
            if (entry_length < 8) {
                throw AP4_Exception(AP4_ERROR_INVALID_RTP_PACKET_EXTRA_DATA);
            }

            // parse the single entry that's currently defined in the spec
            if (entry_tag == AP4_ATOM_TYPE('r','t','p','o') && entry_length == 12) {
                AP4_UI32 time_stamp_offset;
                stream.ReadUI32(time_stamp_offset);
                m_TimeStampOffset = time_stamp_offset;
            } else {
                // ignore it
                AP4_Offset cur_pos;
                stream.Tell(cur_pos);
                stream.Seek(cur_pos + entry_length - 8); // 8 = length + tag
            }

            extra_length -= entry_length;
        }
    }

    // constructors
    for (AP4_UI16 i=0; i<constructor_count; i++) {
        AP4_RtpConstructor* constructor = NULL;
        AP4_RtpConstructorFactory::CreateConstructorFromStream(stream, constructor);
        m_Constructors.Add(constructor);
    }
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::AP4_RtpPacket
+---------------------------------------------------------------------*/
AP4_RtpPacket::~AP4_RtpPacket()
{
    AP4_List<AP4_RtpConstructor>::Item* it = m_Constructors.FirstItem();
    while (it != NULL) {
        it->GetData()->Release();
        it = it->GetNext();
    }
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::AddReference
+---------------------------------------------------------------------*/
void
AP4_RtpPacket::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::Release
+---------------------------------------------------------------------*/
void
AP4_RtpPacket::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::GetSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_RtpPacket::GetSize()
{
    AP4_Size result = 12 + (m_TimeStampOffset != 0)?16:0; 
    result += m_Constructors.ItemCount() * AP4_RTP_CONSTRUCTOR_SIZE;
    return result;
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::Write
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpPacket::Write(AP4_ByteStream& stream) 
{
    // check the payload type
    if (m_PayloadType > 128) return AP4_FAILURE;

    // now write
    AP4_Result result = stream.WriteUI32(m_RelativeTime);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI08(0x80 | m_PBit << 5 | m_XBit << 4);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI08(m_MBit << 7 | m_PayloadType);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI16(m_SequenceSeed);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI08(0);
    if (AP4_FAILED(result)) return result;

    // deal with extra flag
    bool extra_flag = m_TimeStampOffset != 0;
    result = stream.WriteUI08(0x00 | extra_flag << 2 
                                   | m_BFrameFlag << 1 
                                   | m_RepeatFlag << 0);
    if (AP4_FAILED(result)) return result;


    // constructor count
    result = stream.WriteUI16(static_cast<AP4_UI16>(m_Constructors.ItemCount()));

    // write extra data
    if (extra_flag) {
        // extra_length
        result = stream.WriteUI32(16); // 4 (extra_length) + 12 (rtpo atom)
        if (AP4_FAILED(result)) return result;

        // rtpo atom
        result = stream.WriteUI32(12); // size
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(AP4_ATOM_TYPE('r','t','p','o'));
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TimeStampOffset);
        if (AP4_FAILED(result)) return result;
    }

    // constructors
    AP4_List<AP4_RtpConstructor>::Item* it = m_Constructors.FirstItem();
    while (it != NULL) {
        result = it->GetData()->Write(stream);
        if (AP4_FAILED(result)) return result;
        it = it->GetNext();
    }
    return result;
}

/*----------------------------------------------------------------------
|       AP4_RtpPacket::AddConstructor
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpPacket::AddConstructor(AP4_RtpConstructor* constructor) 
{
    constructor->AddReference();
    return m_Constructors.Add(constructor);
}

/*----------------------------------------------------------------------
|       AP4_RtpConstructor::GetConstructedDataSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_RtpPacket::GetConstructedDataSize()
{
    // header + ssrc
    AP4_Size size = 12;

    // constructed data from constructors
    AP4_List<AP4_RtpConstructor>::Item* constructors_it 
        = m_Constructors.FirstItem();
    while (constructors_it != NULL) {
        size += constructors_it->GetData()->GetConstructedDataSize();
        constructors_it = constructors_it->GetNext();
    }

    return size;
}


/*----------------------------------------------------------------------
|       AP4_RtpConstructor::AddReference
+---------------------------------------------------------------------*/
void
AP4_RtpConstructor::AddReference()
{
    m_ReferenceCount++;
}

/*----------------------------------------------------------------------
|       AP4_RtpConstructor::Release
+---------------------------------------------------------------------*/
void
AP4_RtpConstructor::Release()
{
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}
/*----------------------------------------------------------------------
|       AP4_RtpConstructor::Write
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpConstructor::Write(AP4_ByteStream& stream)
{
    AP4_Result result = stream.WriteUI08(m_Type);
    if (AP4_FAILED(result)) return result;

    return DoWrite(stream);
}

/*----------------------------------------------------------------------
|       AP4_NoopRtpConstructor::AP4_NoopRtpConstructor
+---------------------------------------------------------------------*/
AP4_NoopRtpConstructor::AP4_NoopRtpConstructor(AP4_ByteStream& stream) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_NOOP)
{
    AP4_Offset cur_offset;
    stream.Tell(cur_offset);
    stream.Seek(cur_offset+15);
}

/*----------------------------------------------------------------------
|       AP4_NoopRtpConstructor::DoWrite
+---------------------------------------------------------------------*/
AP4_Result
AP4_NoopRtpConstructor::DoWrite(AP4_ByteStream& stream)
{
    AP4_UI08 pad[15];

    return stream.Write(pad, sizeof(pad));
}

/*----------------------------------------------------------------------
|       AP4_ImmediateRtpConstructor::AP4_ImmediateRtpConstructor
+---------------------------------------------------------------------*/
AP4_ImmediateRtpConstructor::AP4_ImmediateRtpConstructor(const AP4_DataBuffer& data) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_IMMEDIATE),
    m_Data(data)
{}

/*----------------------------------------------------------------------
|       AP4_ImmediateRtpConstructor::AP4_ImmediateRtpConstructor
+---------------------------------------------------------------------*/
AP4_ImmediateRtpConstructor::AP4_ImmediateRtpConstructor(AP4_ByteStream& stream) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_IMMEDIATE)
{
    AP4_Offset cur_offset;
    stream.Tell(cur_offset);

    // data
    AP4_UI08 data_size;
    stream.ReadUI08(data_size);
    m_Data.SetDataSize(data_size);
    stream.Read(m_Data.UseData(), data_size);

    // reposition the stream
    stream.Seek(cur_offset+15);
}


/*----------------------------------------------------------------------
|       AP4_ImmediateRtpConstructor::DoWrite
+---------------------------------------------------------------------*/
AP4_Result
AP4_ImmediateRtpConstructor::DoWrite(AP4_ByteStream& stream)
{
    // first check that the data is not too large
    if (m_Data.GetDataSize() > 14) return AP4_FAILURE;

    // now write
    AP4_Result result = stream.WriteUI08(static_cast<AP4_UI08>(m_Data.GetDataSize()));
    if (AP4_FAILED(result)) return result;

    result = stream.Write(m_Data.GetData(), m_Data.GetDataSize());
    if (AP4_FAILED(result)) return result;

    // pad
    AP4_Byte pad[14];
    return stream.Write(pad, sizeof(pad)-m_Data.GetDataSize());
}
/*----------------------------------------------------------------------
|       AP4_SampleRtpConstructor::AP4_SampleRtpConstructor
+---------------------------------------------------------------------*/
AP4_SampleRtpConstructor::AP4_SampleRtpConstructor(AP4_UI08 track_ref_index, 
                                                   AP4_UI16 length, 
                                                   AP4_UI32 sample_num, 
                                                   AP4_UI32 sample_offset) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE),
    m_TrackRefIndex(track_ref_index),
    m_Length(length),
    m_SampleNum(sample_num),
    m_SampleOffset(sample_offset)
{}

/*----------------------------------------------------------------------
|       AP4_SampleRtpConstructor::AP4_SampleRtpConstructor
+---------------------------------------------------------------------*/
AP4_SampleRtpConstructor::AP4_SampleRtpConstructor(AP4_ByteStream& stream) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE)
{
    // offset
    AP4_Offset cur_offset;
    stream.Tell(cur_offset);

    // data
    stream.ReadUI08(m_TrackRefIndex);
    stream.ReadUI16(m_Length);
    stream.ReadUI32(m_SampleNum);
    stream.ReadUI32(m_SampleOffset);

    // reposition the stream
    stream.Seek(cur_offset+15);
}
/*----------------------------------------------------------------------
|       AP4_SampleRtpConstructor::DoWrite
+---------------------------------------------------------------------*/
AP4_Result
AP4_SampleRtpConstructor::DoWrite(AP4_ByteStream& stream)
{
    AP4_Result result = stream.WriteUI08(m_TrackRefIndex);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI16(m_Length);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI32(m_SampleNum);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI32(m_SampleOffset);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI16(1); // bytes per block
    if (AP4_FAILED(result)) return result;

    return stream.WriteUI16(1); // samples per block
}

/*----------------------------------------------------------------------
|       AP4_SampleDescRtpConstructor::AP4_SampleDescRtpConstructor
+---------------------------------------------------------------------*/
AP4_SampleDescRtpConstructor::AP4_SampleDescRtpConstructor(AP4_UI08 track_ref_index, 
                                                           AP4_UI16 length, 
                                                           AP4_UI32 sample_desc_index, 
                                                           AP4_UI32 sample_desc_offset) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE_DESC),
    m_TrackRefIndex(track_ref_index),
    m_Length(length),
    m_SampleDescIndex(sample_desc_index),
    m_SampleDescOffset(sample_desc_offset)
{}

/*----------------------------------------------------------------------
|       AP4_SampleDescRtpConstructor::AP4_SampleDescRtpConstructor
+---------------------------------------------------------------------*/
AP4_SampleDescRtpConstructor::AP4_SampleDescRtpConstructor(AP4_ByteStream& stream) :
    AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE_DESC)
{
    // offset
    AP4_Offset cur_offset;
    stream.Tell(cur_offset);

    // data
    stream.ReadUI08(m_TrackRefIndex);
    stream.ReadUI16(m_Length);
    stream.ReadUI32(m_SampleDescIndex);
    stream.ReadUI32(m_SampleDescOffset);

    // reposition the stream
    stream.Seek(cur_offset+15);
}

/*----------------------------------------------------------------------
|       AP4_SampleDescRtpConstructor::DoWrite
+---------------------------------------------------------------------*/
AP4_Result
AP4_SampleDescRtpConstructor::DoWrite(AP4_ByteStream& stream)
{
    AP4_Result result = stream.WriteUI08(m_TrackRefIndex);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI16(m_Length);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI32(m_SampleDescIndex);
    if (AP4_FAILED(result)) return result;

    result = stream.WriteUI32(m_SampleDescOffset);
    if (AP4_FAILED(result)) return result;

    return stream.WriteUI32(0); // reserved
}

/*----------------------------------------------------------------------
|       AP4_RtpConstructorFactory::CreateConstructorFromStream
+---------------------------------------------------------------------*/
AP4_Result
AP4_RtpConstructorFactory::CreateConstructorFromStream(AP4_ByteStream& stream, 
                                                       AP4_RtpConstructor*& constructor)
{
    // read the first byte (type)
    AP4_RtpConstructor::Type type;
    AP4_Result result = stream.ReadUI08(type);
    if (AP4_FAILED(result)) return result;

    switch(type) {
        case AP4_RTP_CONSTRUCTOR_TYPE_NOOP:
            constructor = new AP4_NoopRtpConstructor(stream);
            break;
        case AP4_RTP_CONSTRUCTOR_TYPE_IMMEDIATE:
            constructor = new AP4_ImmediateRtpConstructor(stream);
            break;
        case AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE:
            constructor = new AP4_SampleRtpConstructor(stream);
            break;
        case AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE_DESC:
            constructor = new AP4_SampleDescRtpConstructor(stream);
            break;
        default:
            return AP4_ERROR_INVALID_RTP_CONSTRUCTOR_TYPE;
    }

    return AP4_SUCCESS;
}
