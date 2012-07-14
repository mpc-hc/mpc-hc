/*****************************************************************
|
|    AP4 - Sample Description Objects
|
|    Copyright 2002 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4SampleDescription.h"
#include "Ap4EsDescriptor.h"
#include "Ap4SampleEntry.h"

/*----------------------------------------------------------------------
|       AP4_UnknownSampleDescription::AP4_UnknownSampleDescription
+---------------------------------------------------------------------*/
AP4_UnknownSampleDescription::AP4_UnknownSampleDescription(AP4_SampleEntry* entry) :
    AP4_SampleDescription(AP4_SampleDescription::TYPE_UNKNOWN),
    m_SampleEntry(entry)
{
}

/*----------------------------------------------------------------------
|       AP4_UnknownSampleDescription::~AP4_UnknownSampleDescription
+---------------------------------------------------------------------*/
AP4_UnknownSampleDescription::~AP4_UnknownSampleDescription()
{
}

/*----------------------------------------------------------------------
|       AP4_UnknownSampleDescription::ToAtom
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_UnknownSampleDescription::ToAtom() const
{
    return new AP4_SampleEntry(m_SampleEntry->GetType(),
                               m_SampleEntry->GetDataReferenceIndex());
}

/*----------------------------------------------------------------------
|       AP4_MpegSampleDescription::AP4_MpegSampleDescription
+---------------------------------------------------------------------*/
AP4_MpegSampleDescription::AP4_MpegSampleDescription(
    StreamType            stream_type,
    OTI                   oti,
    const AP4_DataBuffer* decoder_info,
    AP4_UI32              buffer_size,
    AP4_UI32              max_bitrate,
    AP4_UI32              avg_bitrate) :
    AP4_SampleDescription(TYPE_MPEG),
    m_StreamType(stream_type),
    m_ObjectTypeId(oti),
    m_DecoderInfo(NULL),
    m_BufferSize(buffer_size),
    m_MaxBitrate(max_bitrate),
    m_AvgBitrate(avg_bitrate)
{
    if (decoder_info != NULL) {
        m_DecoderInfo = new AP4_DataBuffer(*decoder_info);
    }
}

/*----------------------------------------------------------------------
|       AP4_MpegSampleDescription::~AP4_MpegSampleDescription
+---------------------------------------------------------------------*/
AP4_MpegSampleDescription::~AP4_MpegSampleDescription()
{
    delete m_DecoderInfo;
}

/*----------------------------------------------------------------------
|       AP4_MpegSampleDescription::CreateEsDescriptor
+---------------------------------------------------------------------*/
AP4_EsDescriptor* 
AP4_MpegSampleDescription::CreateEsDescriptor() const
{
    AP4_EsDescriptor* desc = new AP4_EsDescriptor(0);
    AP4_DecoderSpecificInfoDescriptor* dsi_desc;
    if (m_DecoderInfo) {
        dsi_desc = new AP4_DecoderSpecificInfoDescriptor(*m_DecoderInfo);
    } else {
        dsi_desc = NULL;
    }
    AP4_DecoderConfigDescriptor* decoder_config = 
        new AP4_DecoderConfigDescriptor(m_StreamType,
        m_ObjectTypeId,
        m_BufferSize,
        m_MaxBitrate,
        m_AvgBitrate,
        dsi_desc);
    desc->AddSubDescriptor(decoder_config);
    return desc;
}

/*----------------------------------------------------------------------
|       AP4_MpegSystemSampleDescription::AP4_MpegSystemSampleDescription
+---------------------------------------------------------------------*/
AP4_MpegSystemSampleDescription::AP4_MpegSystemSampleDescription(
    StreamType            stream_type,
    OTI                   oti,
    const AP4_DataBuffer* decoder_info,
    AP4_UI32              buffer_size,
    AP4_UI32              max_bitrate,
    AP4_UI32              avg_bitrate) :
    AP4_MpegSampleDescription(stream_type,
                              oti,
                              decoder_info,
                              buffer_size,
                              max_bitrate,
                              avg_bitrate)
{
}

/*----------------------------------------------------------------------
|       AP4_MpegSystemSampleDescription::ToAtom
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_MpegSystemSampleDescription::ToAtom() const
{
    return new AP4_Mp4sSampleEntry(CreateEsDescriptor());
}

/*----------------------------------------------------------------------
|       AP4_MpegAudioSampleDescription::AP4_MpegAudioSampleDescription
+---------------------------------------------------------------------*/
AP4_MpegAudioSampleDescription::AP4_MpegAudioSampleDescription(
    OTI                   oti,
    unsigned int          sample_rate,
    unsigned int          sample_size,
    unsigned int          channel_count,
    const AP4_DataBuffer* decoder_info,
    AP4_UI32              buffer_size,
    AP4_UI32              max_bitrate,
    AP4_UI32              avg_bitrate) :
    AP4_MpegSampleDescription(AP4_AUDIO_STREAM_TYPE,
                              oti, 
                              decoder_info, buffer_size, 
                              max_bitrate, avg_bitrate),
    m_SampleRate(sample_rate),
    m_SampleSize(sample_size),
    m_ChannelCount(channel_count)
{
}

/*----------------------------------------------------------------------
|       AP4_MpegAudioSampleDescription::ToAtom
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_MpegAudioSampleDescription::ToAtom() const
{
    return new AP4_Mp4aSampleEntry(m_SampleRate<<16,
                                   m_SampleSize,
                                   m_ChannelCount,
                                   CreateEsDescriptor());
}

/*----------------------------------------------------------------------
|       AP4_MpegVideoSampleDescription::AP4_MpegVideoSampleDescription
+---------------------------------------------------------------------*/
AP4_MpegVideoSampleDescription::AP4_MpegVideoSampleDescription(
    OTI                   oti,
    AP4_UI16              width,
    AP4_UI16              height,
    AP4_UI16              depth,
    const char*           compressor_name,
    const AP4_DataBuffer* decoder_info,
    AP4_UI32              buffer_size,
    AP4_UI32              max_bitrate,
    AP4_UI32              avg_bitrate) :
    AP4_MpegSampleDescription(AP4_VISUAL_STREAM_TYPE,
                              oti,
                              decoder_info,
                              buffer_size,
                              max_bitrate,
                              avg_bitrate),
    m_Width(width),
    m_Height(height),
    m_Depth(depth),
    m_CompressorName(compressor_name)
{
}

/*----------------------------------------------------------------------
|       AP4_MpegVideoSampleDescription::ToAtom
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_MpegVideoSampleDescription::ToAtom() const
{
    return new AP4_Mp4vSampleEntry(m_Width,
                                   m_Height,
                                   m_Depth,
                                   m_CompressorName.c_str(),
                                   CreateEsDescriptor());
}

/*----------------------------------------------------------------------
|       AP4_MpegSampleDescription::GetStreamTypeString
+---------------------------------------------------------------------*/
const char* 
AP4_MpegSampleDescription::GetStreamTypeString(StreamType type)
{
    switch (type) {
        case AP4_FORBIDDEN_STREAM_TYPE: return "INVALID"; 
        case AP4_OD_STREAM_TYPE:        return "Object Descriptor";
        case AP4_CR_STREAM_TYPE:        return "CR";	
        case AP4_BIFS_STREAM_TYPE:      return "BIFS";
        case AP4_VISUAL_STREAM_TYPE:    return "Visual";
        case AP4_AUDIO_STREAM_TYPE:     return "Audio";
        case AP4_MPEG7_STREAM_TYPE:     return "MPEG-7";
        case AP4_IPMP_STREAM_TYPE:      return "IPMP";
        case AP4_OCI_STREAM_TYPE:       return "OCI";
        case AP4_MPEGJ_STREAM_TYPE:     return "MPEG-J";
        default:                        return "UNKNOWN";
    }
}

/*----------------------------------------------------------------------
|       AP4_MpegSampleDescription::GetObjectTypeString
+---------------------------------------------------------------------*/
const char* 
AP4_MpegSampleDescription::GetObjectTypeString(OTI oti)
{
    switch (oti) {
        case AP4_MPEG4_SYSTEM_OTI:         return "MPEG-4 System";
        case AP4_MPEG4_SYSTEM_COR_OTI:     return "MPEG-4 System COR";
        case AP4_MPEG4_VISUAL_OTI:         return "MPEG-4 Video";
        case AP4_MPEG4_AUDIO_OTI:          return "MPEG-4 Audio";
        case AP4_MPEG2_VISUAL_SIMPLE_OTI:  return "MPEG-2 Video Simple Profile";
        case AP4_MPEG2_VISUAL_MAIN_OTI:    return "MPEG-2 Video Main Profile";
        case AP4_MPEG2_VISUAL_SNR_OTI:     return "MPEG-2 Video SNR";
        case AP4_MPEG2_VISUAL_SPATIAL_OTI: return "MPEG-2 Video Spatial";
        case AP4_MPEG2_VISUAL_HIGH_OTI:    return "MPEG-2 Video High";
        case AP4_MPEG2_VISUAL_422_OTI:     return "MPEG-2 Video 4:2:2";
        case AP4_MPEG2_AAC_AUDIO_MAIN_OTI: return "MPEG-2 Audio AAC Main Profile";
        case AP4_MPEG2_AAC_AUDIO_LC_OTI:   return "MPEG-2 Audio AAC Low Complexity";
        case AP4_MPEG2_AAC_AUDIO_SSRP_OTI: return "MPEG-2 Audio AAC SSRP";
        case AP4_MPEG2_PART3_AUDIO_OTI:    return "MPEG-2 Audio Part-3";
        case AP4_MPEG1_VISUAL_OTI:         return "MPEG-1 Video";
        case AP4_MPEG1_AUDIO_OTI:          return "MPEG-1 Audio";
        case AP4_JPEG_OTI:                 return "JPEG";
        case AP4_DTSC_AUDIO_OTI:           return "DTS audio";
        case AP4_DTSH_AUDIO_OTI:           return "DTS-HD High Resolution Audio";
        case AP4_DTSL_AUDIO_OTI:           return "DTS-HD Master Audio";
        case AP4_NERO_VOBSUB:              return "VobSub Subtitle";
        default:                           return "UNKNOWN";
    }
}

