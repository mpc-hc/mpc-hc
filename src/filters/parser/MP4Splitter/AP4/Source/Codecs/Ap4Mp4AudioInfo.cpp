/*****************************************************************
|
|    AP4 - AAC Info
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
#include "Ap4BitStream.h"
#include "Ap4Mp4AudioInfo.h"
#include "Ap4DataBuffer.h"
#include "Ap4SampleDescription.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const unsigned int AP4_AAC_MAX_SAMPLING_FREQUENCY_INDEX = 12;
static const unsigned int AP4_AacSamplingFreqTable[13] =
{
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350
};

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDsiParser
+---------------------------------------------------------------------*/
class AP4_Mp4AudioDsiParser
{
public:
    AP4_Mp4AudioDsiParser(const AP4_UI08* data, AP4_Size data_size) :
        m_Data(data, data_size),
        m_Position(0) {}

    AP4_Size BitsLeft()
    {
        return 8 * m_Data.GetDataSize() - m_Position;
    }
    AP4_UI32 ReadBits(unsigned int n)
    {
        AP4_UI32 result = 0;
        const AP4_UI08* data = m_Data.GetData();
        while(n)
        {
            unsigned int bits_avail = 8 - (m_Position % 8);
            unsigned int chunk_size = bits_avail >= n ? n : bits_avail;
            unsigned int chunk_bits = (((unsigned int)(data[m_Position/8])) >> (bits_avail - chunk_size)) & ((1 << chunk_size) - 1);
            result = (result << chunk_size) | chunk_bits;
            n -= chunk_size;
            m_Position += chunk_size;
        }

        return result;
    }

private:
    AP4_DataBuffer m_Data;
    unsigned int   m_Position;
};

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDecoderConfig::AP4_Mp4AudioDecoderConfig
+---------------------------------------------------------------------*/
AP4_Mp4AudioDecoderConfig::AP4_Mp4AudioDecoderConfig()
{
    Reset();
}

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDecoderConfig::Reset
+---------------------------------------------------------------------*/
void
AP4_Mp4AudioDecoderConfig::Reset()
{
    m_ObjectType             = 0;
    m_SamplingFrequencyIndex = 0;
    m_SamplingFrequency      = 0;
    m_ChannelCount           = 0;
    m_ChannelConfiguration   = CHANNEL_CONFIG_NONE;
    m_FrameLengthFlag        = false;
    m_DependsOnCoreCoder     = false;
    m_CoreCoderDelay         = 0;
    m_Extension.m_SbrPresent = false;
    m_Extension.m_PsPresent  = false;
    m_Extension.m_ObjectType = 0;
    m_Extension.m_SamplingFrequencyIndex = 0;
    m_Extension.m_SamplingFrequency      = 0;
}

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDecoderConfig::ParseAudioObjectType
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mp4AudioDecoderConfig::ParseAudioObjectType(AP4_Mp4AudioDsiParser& parser, AP4_UI08& object_type)
{
    if(parser.BitsLeft() < 5) return AP4_ERROR_INVALID_FORMAT;
    object_type = (AP4_UI08)parser.ReadBits(5);
    if((int)object_type == 31)
    {
        if(parser.BitsLeft() < 6) return AP4_ERROR_INVALID_FORMAT;
        object_type = (AP4_UI08)(32 + parser.ReadBits(6));
    }
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDecoderConfig::ParseGASpecificInfo
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mp4AudioDecoderConfig::ParseGASpecificInfo(AP4_Mp4AudioDsiParser& parser)
{
    if(parser.BitsLeft() < 2) return AP4_ERROR_INVALID_FORMAT;
    m_FrameLengthFlag = (parser.ReadBits(1) == 1);
    m_DependsOnCoreCoder = (parser.ReadBits(1) == 1);
    if(m_DependsOnCoreCoder)
    {
        if(parser.BitsLeft() < 14) return AP4_ERROR_INVALID_FORMAT;
        m_CoreCoderDelay = parser.ReadBits(14);
    }
    else
    {
        m_CoreCoderDelay = 0;
    }
    if(parser.BitsLeft() < 1) return AP4_ERROR_INVALID_FORMAT;
    parser.ReadBits(1); /* extensionFlag */
    if(m_ChannelConfiguration == CHANNEL_CONFIG_NONE)
    {
        /*program_config_element (); */
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDecoderConfig::ParseSamplingFrequency
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mp4AudioDecoderConfig::ParseSamplingFrequency(AP4_Mp4AudioDsiParser& parser,
        unsigned int&          sampling_frequency_index,
        unsigned int&          sampling_frequency)
{
    if(parser.BitsLeft() < 4)
    {
        return AP4_ERROR_INVALID_FORMAT;
    }

    sampling_frequency_index = parser.ReadBits(4);
    if(sampling_frequency_index == 0xF)
    {
        if(parser.BitsLeft() < 24)
        {
            return AP4_ERROR_INVALID_FORMAT;
        }
        sampling_frequency = parser.ReadBits(24);
    }
    else if(sampling_frequency_index <= AP4_AAC_MAX_SAMPLING_FREQUENCY_INDEX)
    {
        sampling_frequency = AP4_AacSamplingFreqTable[sampling_frequency_index];
    }
    else
    {
        sampling_frequency = 0;
        return AP4_ERROR_INVALID_FORMAT;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Mp4AudioDecoderConfig::Parse
+---------------------------------------------------------------------*/
AP4_Result
AP4_Mp4AudioDecoderConfig::Parse(const unsigned char* data,
                                 AP4_Size             data_size)
{
    AP4_Result            result;
    AP4_Mp4AudioDsiParser bits(data, data_size);

    // default config
    Reset();

    // parse the audio object type
    result = ParseAudioObjectType(bits, m_ObjectType);
    if(AP4_FAILED(result)) return result;

    // parse the sampling frequency
    result = ParseSamplingFrequency(bits,
                                    m_SamplingFrequencyIndex,
                                    m_SamplingFrequency);
    if(AP4_FAILED(result)) return result;

    if(bits.BitsLeft() < 4)
    {
        return AP4_ERROR_INVALID_FORMAT;
    }
    m_ChannelConfiguration = (ChannelConfiguration)bits.ReadBits(4);
    m_ChannelCount = (unsigned int)m_ChannelConfiguration;
    if(m_ChannelCount == 7)
    {
        m_ChannelCount = 8;
    }
    else if(m_ChannelCount > 7)
    {
        m_ChannelCount = 0;
    }

    if(m_ObjectType == AP4_MPEG4_AUDIO_OBJECT_TYPE_SBR ||
       m_ObjectType == AP4_MPEG4_AUDIO_OBJECT_TYPE_PS)
    {
        m_Extension.m_ObjectType = AP4_MPEG4_AUDIO_OBJECT_TYPE_SBR;
        m_Extension.m_SbrPresent = true;
        m_Extension.m_PsPresent  = m_ObjectType == AP4_MPEG4_AUDIO_OBJECT_TYPE_PS;
        result = ParseSamplingFrequency(bits,
                                        m_Extension.m_SamplingFrequencyIndex,
                                        m_Extension.m_SamplingFrequency);
        if(AP4_FAILED(result)) return result;
        result = ParseAudioObjectType(bits, m_ObjectType);
        if(AP4_FAILED(result)) return result;
    }
    else
    {
        m_Extension.m_ObjectType             = 0;
        m_Extension.m_SamplingFrequency      = 0;
        m_Extension.m_SamplingFrequencyIndex = 0;
        m_Extension.m_SbrPresent             = false;
        m_Extension.m_PsPresent              = false;
    }

    switch(m_ObjectType)
    {
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_MAIN:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LC:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_SSR:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LTP:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_SCALABLE:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_TWINVQ:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LC:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LTP:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LD:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_TWINVQ:
    case AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC:
        result = ParseGASpecificInfo(bits);
        break;

    default:
        return AP4_ERROR_NOT_SUPPORTED;
    }

    return AP4_SUCCESS;
}




