/*****************************************************************
|
|    AP4 - MPEG2 Transport Streams
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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
#include "Ap4Mpeg2Ts.h"
#include "Ap4ByteStream.h"
#include "Ap4Sample.h"
#include "Ap4SampleDescription.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const unsigned int AP4_MPEG2TS_PACKET_SIZE         = 188;
const unsigned int AP4_MPEG2TS_PACKET_PAYLOAD_SIZE = 184;
const unsigned int AP4_MPEG2TS_SYNC_BYTE           = 0x47;
const unsigned int AP4_MPEG2TS_PCR_ADAPTATION_SIZE = 6;

static unsigned char const StuffingBytes[AP4_MPEG2TS_PACKET_SIZE] = 
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

/*----------------------------------------------------------------------
|   GetSamplingFrequencyIndex
+---------------------------------------------------------------------*/
static unsigned int
GetSamplingFrequencyIndex(unsigned int sampling_frequency)
{
    switch (sampling_frequency) {
        case 96000: return 0;
        case 88200: return 1;
        case 64000: return 2;
        case 48000: return 3;
        case 44100: return 4;
        case 32000: return 5;
        case 24000: return 6;
        case 22050: return 7;
        case 16000: return 8;
        case 12000: return 9;
        case 11025: return 10;
        case 8000:  return 11;
        case 7350:  return 12;
        default:    return 0;
    }
}

/*----------------------------------------------------------------------
|   MakeAdtsHeader
+---------------------------------------------------------------------*/
static void
MakeAdtsHeader(unsigned char bits[7], 
               unsigned int  frame_size,
               unsigned int  sampling_frequency_index,
               unsigned int  channel_configuration)
{
	bits[0] = 0xFF;
	bits[1] = 0xF1; // 0xF9 (MPEG2)
	bits[2] = 0x40 | (sampling_frequency_index << 2) | (channel_configuration >> 2);
	bits[3] = ((channel_configuration&0x3)<<6) | ((frame_size+7) >> 11);
    bits[4] = ((frame_size+7) >> 3)&0xFF;
	bits[5] = (((frame_size+7) << 5)&0xFF) | 0x1F;
	bits[6] = 0xFC;
    
	/*
     0:  syncword 12 always: '111111111111' 
     12: ID 1 0: MPEG-4, 1: MPEG-2 
     13: layer 2 always: '00' 
     15: protection_absent 1  
     16: profile 2  
     18: sampling_frequency_index 4  
     22: private_bit 1  
     23: channel_configuration 3  
     26: original/copy 1  
     27: home 1  
     28: emphasis 2 only if ID == 0 
     
     ADTS Variable header: these can change from frame to frame 
     28: copyright_identification_bit 1  
     29: copyright_identification_start 1  
     30: aac_frame_length 13 length of the frame including header (in bytes) 
     43: adts_buffer_fullness 11 0x7FF indicates VBR 
     54: no_raw_data_blocks_in_frame 2  
     ADTS Error check 
     crc_check 16 only if protection_absent == 0 
     */
}

/*----------------------------------------------------------------------
|   CRC_Table
+---------------------------------------------------------------------*/
static AP4_UI32 
const CRC_Table[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

/*----------------------------------------------------------------------
|   ComputeCRC
+---------------------------------------------------------------------*/
static AP4_UI32
ComputeCRC(const unsigned char* data, unsigned int data_size)
{
    AP4_UI32 crc = 0xFFFFFFFF;
    
    for (unsigned int i=0; i<data_size; i++) {
        crc = (crc << 8) ^ CRC_Table[((crc >> 24) ^ *data++) & 0xFF];
    }
    
    return crc;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::Stream::WritePacketHeader
+---------------------------------------------------------------------*/
void
AP4_Mpeg2TsWriter::Stream::WritePacketHeader(bool            payload_start, 
                                             unsigned int&   payload_size,
                                             bool            with_pcr,
                                             AP4_UI64        pcr,
                                             AP4_ByteStream& output)
{
    unsigned char header[4];
    header[0] = AP4_MPEG2TS_SYNC_BYTE;
    header[1] = ((payload_start?1:0)<<6) | (m_PID >> 8);
    header[2] = m_PID & 0xFF;
    
    unsigned int adaptation_field_size = 0;
    if (with_pcr) adaptation_field_size += 2+AP4_MPEG2TS_PCR_ADAPTATION_SIZE;
    
    // clamp the payload size
    if (payload_size+adaptation_field_size > AP4_MPEG2TS_PACKET_PAYLOAD_SIZE) {
        payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-adaptation_field_size;
    }
    
    // adjust the adaptation field to include stuffing if necessary
    if (adaptation_field_size+payload_size < AP4_MPEG2TS_PACKET_PAYLOAD_SIZE) {
        adaptation_field_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-payload_size;
    }
    
    if (adaptation_field_size == 0) {
        // no adaptation field
        header[3] = (1<<4) | ((m_ContinuityCounter++)&0x0F);
        output.Write(header, 4);
    } else {
        // adaptation field present
        header[3] = (3<<4) | ((m_ContinuityCounter++)&0x0F);
        output.Write(header, 4);
        
        if (adaptation_field_size == 1) {
            // just one byte (stuffing)
            output.WriteUI08(0);
        } else {
            // two or more bytes (stuffing and/or PCR)
            output.WriteUI08(adaptation_field_size-1);
            output.WriteUI08(with_pcr?(1<<4):0);
            unsigned int pcr_size = 0;
            if (with_pcr) {
                pcr_size = AP4_MPEG2TS_PCR_ADAPTATION_SIZE;
                AP4_UI64 pcr_base = pcr/300;
                AP4_UI32 pcr_ext  = (AP4_UI32)(pcr%300);
                AP4_BitWriter writer(pcr_size);
                writer.Write((AP4_UI32)(pcr_base>>32), 1);
                writer.Write((AP4_UI32)pcr_base, 32);
                writer.Write(0x3F, 6);
                writer.Write(pcr_ext, 9);
                output.Write(writer.GetData(), pcr_size);
            } 
            if (adaptation_field_size > 2) {
                output.Write(StuffingBytes, adaptation_field_size-pcr_size-2);
            }
        }
    }
} 

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::SampleStream::WritePES
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mpeg2TsWriter::SampleStream::WritePES(const unsigned char* data, 
                                          unsigned int         data_size, 
                                          AP4_UI64             dts, 
                                          bool                 with_dts, 
                                          AP4_UI64             pts, 
                                          bool                 with_pcr, 
                                          AP4_ByteStream&      output)
{
    unsigned int pes_header_size = 14+(with_dts?5:0);
    AP4_BitWriter pes_header(pes_header_size);
    
    pes_header.Write(0x000001, 24);    // packet_start_code_prefix
    pes_header.Write(m_StreamId, 8);   // stream_id
    pes_header.Write(m_StreamId == AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO?0:(data_size+pes_header_size-6), 16); // PES_packet_length
    pes_header.Write(2, 2);            // '01'
    pes_header.Write(0, 2);            // PES_scrambling_control
    pes_header.Write(0, 1);            // PES_priority
    pes_header.Write(1, 1);            // data_alignment_indicator
    pes_header.Write(0, 1);            // copyright
    pes_header.Write(0, 1);            // original_or_copy
    pes_header.Write(with_dts?3:2, 2); // PTS_DTS_flags
    pes_header.Write(0, 1);            // ESCR_flag
    pes_header.Write(0, 1);            // ES_rate_flag
    pes_header.Write(0, 1);            // DSM_trick_mode_flag
    pes_header.Write(0, 1);            // additional_copy_info_flag
    pes_header.Write(0, 1);            // PES_CRC_flag
    pes_header.Write(0, 1);            // PES_extension_flag
    pes_header.Write(pes_header_size-9, 8);// PES_header_data_length
    
    pes_header.Write(with_dts?3:2, 4);         // '0010' or '0011'
    pes_header.Write((AP4_UI32)(pts>>30), 3);  // PTS[32..30]
    pes_header.Write(1, 1);                    // marker_bit
    pes_header.Write((AP4_UI32)(pts>>15), 15); // PTS[29..15]
    pes_header.Write(1, 1);                    // marker_bit
    pes_header.Write((AP4_UI32)pts, 15);       // PTS[14..0]
    pes_header.Write(1, 1);                    // market_bit
    
    if (with_dts) {
        pes_header.Write(1, 4);                    // '0001'
        pes_header.Write((AP4_UI32)(dts>>30), 3);  // DTS[32..30]
        pes_header.Write(1, 1);                    // marker_bit
        pes_header.Write((AP4_UI32)(dts>>15), 15); // DTS[29..15]
        pes_header.Write(1, 1);                    // marker_bit
        pes_header.Write((AP4_UI32)dts, 15);       // DTS[14..0]
        pes_header.Write(1, 1);                    // market_bit
    }
    
    bool first_packet = true;
    data_size += pes_header_size; // add size of PES header
    while (data_size) {
        unsigned int payload_size = data_size;
        if (payload_size > AP4_MPEG2TS_PACKET_PAYLOAD_SIZE) payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
        
        if (first_packet)  {
            WritePacketHeader(first_packet, payload_size, with_pcr, (with_dts?dts:pts)*300, output);
            first_packet = false;
            output.Write(pes_header.GetData(), pes_header_size);
            output.Write(data, payload_size-pes_header_size);
            data += payload_size-pes_header_size;
        } else {
            WritePacketHeader(first_packet, payload_size, false, 0, output);
            output.Write(data, payload_size);
            data += payload_size;
        }
        data_size -= payload_size;
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsAudioSampleStream
+---------------------------------------------------------------------*/
class AP4_Mpeg2TsAudioSampleStream : public AP4_Mpeg2TsWriter::SampleStream 
{
public:
    static AP4_Result Create(AP4_UI16                          pid, 
                             AP4_UI32                          timescale, 
                             AP4_Mpeg2TsWriter::SampleStream*& stream);
    AP4_Result WriteSample(AP4_Sample&            sample, 
                           AP4_SampleDescription* sample_description,
                           bool                   with_pcr, 
                           AP4_ByteStream&        output);
    
private:
    AP4_Mpeg2TsAudioSampleStream(AP4_UI16 pid, AP4_UI32 timescale) : 
    AP4_Mpeg2TsWriter::SampleStream(pid, 
                                    AP4_MPEG2_TS_DEFAULT_STREAM_ID_AUDIO, 
                                    AP4_MPEG2_STREAM_TYPE_ISO_IEC_13818_7,
                                    timescale) {}
};

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsAudioSampleStream::Create
+---------------------------------------------------------------------*/
AP4_Result 
AP4_Mpeg2TsAudioSampleStream::Create(AP4_UI16                          pid, 
                                     AP4_UI32                          timescale, 
                                     AP4_Mpeg2TsWriter::SampleStream*& stream)
{
    stream = new AP4_Mpeg2TsAudioSampleStream(pid, timescale);
    return AP4_SUCCESS;
}
                                                       
/*----------------------------------------------------------------------
|   AP4_Mpeg2TsAudioSampleStream::WriteSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mpeg2TsAudioSampleStream::WriteSample(AP4_Sample&            sample, 
                                          AP4_SampleDescription* sample_description,
                                          bool                   with_pcr, 
                                          AP4_ByteStream&        output)
{
    // check the sample description
    AP4_MpegAudioSampleDescription* audio_desc = AP4_DYNAMIC_CAST(AP4_MpegAudioSampleDescription, sample_description);
    if (audio_desc == NULL) return AP4_ERROR_NOT_SUPPORTED;
    if (audio_desc->GetMpeg4AudioObjectType() != AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LC) return AP4_ERROR_NOT_SUPPORTED;

    unsigned int sampling_frequency_index = GetSamplingFrequencyIndex(audio_desc->GetSampleRate());
    unsigned int channel_configuration = audio_desc->GetChannelCount();

    AP4_DataBuffer data;
    if (AP4_SUCCEEDED(sample.ReadData(data))) {
        unsigned char* buffer = new unsigned char[7+sample.GetSize()];
        MakeAdtsHeader(buffer, sample.GetSize(), sampling_frequency_index, channel_configuration);
        AP4_CopyMemory(buffer+7, data.GetData(), data.GetDataSize());
        AP4_UI64 ts = AP4_ConvertTime(sample.GetDts(), m_TimeScale, 90000);
        WritePES(buffer, 7+sample.GetSize(), ts, false, ts, with_pcr, output);
        delete[] buffer;
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsVideoSampleStream
+---------------------------------------------------------------------*/
class AP4_Mpeg2TsVideoSampleStream : public AP4_Mpeg2TsWriter::SampleStream 
{
public:
    static AP4_Result Create(AP4_UI16                          pid, 
                             AP4_UI32                          timescale, 
                             AP4_Mpeg2TsWriter::SampleStream*& stream);
    AP4_Result WriteSample(AP4_Sample&            sample, 
                           AP4_SampleDescription* sample_description,
                           bool                   with_pcr, 
                           AP4_ByteStream&        output);
    
private:
    AP4_Mpeg2TsVideoSampleStream(AP4_UI16 pid, AP4_UI32 timescale) :
        AP4_Mpeg2TsWriter::SampleStream(pid, 
                                        AP4_MPEG2_TS_DEFAULT_STREAM_ID_VIDEO, 
                                        AP4_MPEG2_STREAM_TYPE_AVC,
                                        timescale),
    m_SampleDescriptionIndex(-1),
    m_NaluLengthSize(0) {}
    
    int            m_SampleDescriptionIndex;
    AP4_DataBuffer m_Prefix;
    unsigned int   m_NaluLengthSize;
};

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsVideoSampleStream::AP4_Mpeg2TsVideoSampleStream
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mpeg2TsVideoSampleStream::Create(AP4_UI16                          pid, 
                                     AP4_UI32                          timescale, 
                                     AP4_Mpeg2TsWriter::SampleStream*& stream)
{
    // create the stream object
    stream = new AP4_Mpeg2TsVideoSampleStream(pid, timescale);        
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsVideoSampleStream::WriteSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mpeg2TsVideoSampleStream::WriteSample(AP4_Sample&            sample, 
                                          AP4_SampleDescription* sample_description,
                                          bool                   with_pcr, 
                                          AP4_ByteStream&        output)
{
    // check the sample description
    AP4_AvcSampleDescription* avc_desc = AP4_DYNAMIC_CAST(AP4_AvcSampleDescription, sample_description);
    if (avc_desc == NULL) return AP4_ERROR_NOT_SUPPORTED;
    
    if ((int)sample.GetDescriptionIndex() != m_SampleDescriptionIndex) {
        m_SampleDescriptionIndex = sample.GetDescriptionIndex();

        // make the SPS/PPS prefix
        m_NaluLengthSize = avc_desc->GetNaluLengthSize();
        m_Prefix.SetDataSize(0);
        for (unsigned int i=0; i<avc_desc->GetSequenceParameters().ItemCount(); i++) {
            AP4_DataBuffer& buffer = avc_desc->GetSequenceParameters()[i];
            unsigned int prefix_size = m_Prefix.GetDataSize();
            m_Prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
            unsigned char* p = m_Prefix.UseData()+prefix_size;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 1;
            AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
        }
        for (unsigned int i=0; i<avc_desc->GetPictureParameters().ItemCount(); i++) {
            AP4_DataBuffer& buffer = avc_desc->GetPictureParameters()[i];
            unsigned int prefix_size = m_Prefix.GetDataSize();
            m_Prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
            unsigned char* p = m_Prefix.UseData()+prefix_size;
            *p++ = 0;
            *p++ = 0;
            *p++ = 0;
            *p++ = 1;
            AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
        }
    }
    
    // read the sample data
    AP4_DataBuffer sample_data;
    AP4_CHECK(sample.ReadData(sample_data));
    
    // allocate a buffer for the PES packet
    AP4_DataBuffer pes_data;
    pes_data.SetDataSize(6+m_Prefix.GetDataSize());
    unsigned char* pes_buffer = pes_data.UseData();
    
    // start of access unit
    pes_buffer[0] = 0;
    pes_buffer[1] = 0;
    pes_buffer[2] = 0;
    pes_buffer[3] = 1;
    pes_buffer[4] = 9;    // NAL type = Access Unit Delimiter;
    pes_buffer[5] = 0xE0; // Slice types = ANY
    
    // copy the prefix
    AP4_CopyMemory(pes_buffer+6, m_Prefix.GetData(), m_Prefix.GetDataSize());
    
    // write the NAL units
    const unsigned char* data      = sample_data.GetData();
    unsigned int         data_size = sample_data.GetDataSize();
    
    while (data_size) {
        // sanity check
        if (data_size < m_NaluLengthSize) break;
        
        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (m_NaluLengthSize == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (m_NaluLengthSize == 2) {
            nalu_size = AP4_BytesToInt16BE(data);
            data      += 2;
            data_size -= 2;
        } else if (m_NaluLengthSize == 4) {
            nalu_size = AP4_BytesToInt32BE(data);
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;
        
        // add a start code before the NAL unit
        unsigned int offset = pes_data.GetDataSize(); 
        pes_data.SetDataSize(offset+3+nalu_size);
        pes_buffer = pes_data.UseData()+offset;
        pes_buffer[0] = 0;
        pes_buffer[1] = 0;
        pes_buffer[2] = 1;
        AP4_CopyMemory(pes_buffer+3, data, nalu_size);
        
        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    } 
    
    // compute the timestamp
    AP4_UI64 dts = AP4_ConvertTime(sample.GetDts(), m_TimeScale, 90000);
    AP4_UI64 pts = AP4_ConvertTime(sample.GetCts(), m_TimeScale, 90000);
    
    // write the packet
    return WritePES(pes_data.GetData(), pes_data.GetDataSize(), dts, true, pts, with_pcr, output);
}


/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::AP4_Mpeg2TsWriter
+---------------------------------------------------------------------*/
AP4_Mpeg2TsWriter::AP4_Mpeg2TsWriter() :
    m_Audio(NULL),
    m_Video(NULL)
{
    m_PAT = new Stream(0);
    m_PMT = new Stream(AP4_MPEG2_TS_DEFAULT_PID_PMT);
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::~AP4_Mpeg2TsWriter
+---------------------------------------------------------------------*/
AP4_Mpeg2TsWriter::~AP4_Mpeg2TsWriter()
{
    delete m_PAT;
    delete m_PMT;
    delete m_Audio;
    delete m_Video;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::WritePAT
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mpeg2TsWriter::WritePAT(AP4_ByteStream& output)
{
    unsigned int payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
    m_PAT->WritePacketHeader(true, payload_size, false, 0, output);
    
    AP4_BitWriter writer(1024);
    
    writer.Write(0, 8);  // pointer
    writer.Write(0, 8);  // table_id
    writer.Write(1, 1);  // section_syntax_indicator
    writer.Write(0, 1);  // '0'
    writer.Write(3, 2);  // reserved
    writer.Write(13, 12);// section_length
    writer.Write(1, 16); // transport_stream_id
    writer.Write(3, 2);  // reserved
    writer.Write(0, 5);  // version_number
    writer.Write(1, 1);  // current_next_indicator
    writer.Write(0, 8);  // section_number
    writer.Write(0, 8);  // last_section_number
    writer.Write(1, 16); // program number
    writer.Write(7, 3);  // reserved
    writer.Write(m_PMT->GetPID(), 13); // program_map_PID
    writer.Write(ComputeCRC(writer.GetData()+1, 17-1-4), 32);
    
    output.Write(writer.GetData(), 17);
    
    output.Write(StuffingBytes, AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-17);
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::WritePMT
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mpeg2TsWriter::WritePMT(AP4_ByteStream& output)
{
    // check that we have at least one media stream
    if (m_Audio == NULL && m_Video == NULL) {
        return AP4_ERROR_INVALID_STATE;
    }
    
    unsigned int payload_size = AP4_MPEG2TS_PACKET_PAYLOAD_SIZE;
    m_PMT->WritePacketHeader(true, payload_size, false, 0, output);
    
    AP4_BitWriter writer(1024);
    
    unsigned int section_length = 13;
    unsigned int pcr_pid = 0;
    if (m_Audio) {
        section_length += 5;
        pcr_pid = m_Audio->GetPID();
    } 
    if (m_Video) {
        section_length += 5;
        pcr_pid = m_Video->GetPID();
    }

    writer.Write(0, 8);        // pointer
    writer.Write(2, 8);        // table_id
    writer.Write(1, 1);        // section_syntax_indicator
    writer.Write(0, 1);        // '0'
    writer.Write(3, 2);        // reserved
    writer.Write(section_length, 12); // section_length
    writer.Write(1, 16);       // program_number
    writer.Write(3, 2);        // reserved
    writer.Write(0, 5);        // version_number
    writer.Write(1, 1);        // current_next_indicator
    writer.Write(0, 8);        // section_number
    writer.Write(0, 8);        // last_section_number
    writer.Write(7, 3);        // reserved
    writer.Write(pcr_pid, 13); // PCD_PID
    writer.Write(0xF, 4);      // reserved
    writer.Write(0, 12);       // program_info_length
    
    if (m_Audio) {
        writer.Write(m_Audio->m_StreamType, 8); // stream_type 
        writer.Write(0x7, 3);                   // reserved
        writer.Write(m_Audio->GetPID(), 13);    // elementary_PID
        writer.Write(0xF, 4);                   // reserved
        writer.Write(0, 12);                    // ES_info_length
    }
    
    if (m_Video) {
        writer.Write(m_Video->m_StreamType, 8); // stream_type
        writer.Write(0x7, 3);                   // reserved
        writer.Write(m_Video->GetPID(), 13);    // elementary_PID
        writer.Write(0xF, 4);                   // reserved
        writer.Write(0, 12);                    // ES_info_length
    }
    
    writer.Write(ComputeCRC(writer.GetData()+1, section_length-1), 32); // CRC
    
    output.Write(writer.GetData(), section_length+4);
    output.Write(StuffingBytes, AP4_MPEG2TS_PACKET_PAYLOAD_SIZE-(section_length+4));
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::SetAudioStream
+---------------------------------------------------------------------*/
AP4_Result 
AP4_Mpeg2TsWriter::SetAudioStream(AP4_UI32 timescale, SampleStream*& stream)
{
    // default
    stream = NULL;
    
    AP4_Result result = AP4_Mpeg2TsAudioSampleStream::Create(AP4_MPEG2_TS_DEFAULT_PID_AUDIO,
                                                             timescale,
                                                             m_Audio);
    if (AP4_FAILED(result)) return result;
    stream = m_Audio;
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mpeg2TsWriter::SetVideoStream
+---------------------------------------------------------------------*/
AP4_Result 
AP4_Mpeg2TsWriter::SetVideoStream(AP4_UI32 timescale, SampleStream*& stream)
{
    // default
    stream = NULL;
    
    AP4_Result result = AP4_Mpeg2TsVideoSampleStream::Create(AP4_MPEG2_TS_DEFAULT_PID_VIDEO, 
                                                  timescale,
                                                  m_Video);
    if (AP4_FAILED(result)) return result;
    stream = m_Video;
    return AP4_SUCCESS;
}

