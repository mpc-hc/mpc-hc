/*****************************************************************
|
|    AP4 - MP4 Audio Info
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

#ifndef _AP4_MP4_AUDIO_INFO_H_
#define _AP4_MP4_AUDIO_INFO_H_

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_Mp4AudioDsiParser;

/*----------------------------------------------------------------------
|   AP4_Mp4DecoderConfig
+---------------------------------------------------------------------*/
/**
 * Detailed AAC decoder configuration information.
 * This information is necessary in order to create a decoder object.
 * It is normally obtained from the DecoderSpecificInfo field of the
 * DecoderConfigDescriptor descriptor carried in the sample description
 * for the audio samples. See 14496-1, subpart 2, p 2.6.6 for details.
 */
class AP4_Mp4AudioDecoderConfig {
public:
    /**
     * Channel configuration for multichannel audio buffers.
     */
    typedef enum {
        CHANNEL_CONFIG_NONE   = 0,             /**< No channel (not used)       */
        CHANNEL_CONFIG_MONO   = 1,             /**< Mono (single audio channel) */
        CHANNEL_CONFIG_STEREO = 2,             /**< Stereo (Two audio channels) */
        CHANNEL_CONFIG_STEREO_PLUS_CENTER = 3, /**< Stereo plus one center channel */
        CHANNEL_CONFIG_STEREO_PLUS_CENTER_PLUS_REAR_MONO = 4, /**< Stereo plus one center and one read channel */
        CHANNEL_CONFIG_FIVE = 5,               /**< Five channels */
        CHANNEL_CONFIG_FIVE_PLUS_ONE = 6,      /**< Five channels plus one low frequency channel */
        CHANNEL_CONFIG_SEVEN_PLUS_ONE = 7,     /**< Seven channels plus one low frequency channel */
        CHANNEL_CONFIG_UNSUPPORTED
    } ChannelConfiguration;

    // constructor
    AP4_Mp4AudioDecoderConfig();
    
    /**
     * Parser a DecoderSpecificInfo buffer
     */
    AP4_Result Parse(const AP4_UI08* data, AP4_Size data_size);
    
    /**
     * Reset all members to default values (0)
     */
    void Reset();
    
    // members
    AP4_UI08             m_ObjectType;             /**< Type identifier for the audio data */
    unsigned int         m_SamplingFrequencyIndex; /**< Index of the sampling frequency in the sampling frequency table */
    unsigned int         m_SamplingFrequency;      /**< Sampling frequency */
    unsigned int         m_ChannelCount;           /**< Number of audio channels */
    ChannelConfiguration m_ChannelConfiguration;   /**< Channel configuration */
    bool                 m_FrameLengthFlag;        /**< Frame Length Flag     */
    bool                 m_DependsOnCoreCoder;     /**< Depends on Core Coder */
    unsigned int         m_CoreCoderDelay;         /**< Core Code delay       */
    /** Extension details */
    struct {
        bool         m_SbrPresent;             /**< SBR is present        */
        bool         m_PsPresent;              /**< PS is present         */
        AP4_UI08     m_ObjectType;             /**< Extension object type */
        unsigned int m_SamplingFrequencyIndex; /**< Sampling frequency index of the extension */
        unsigned int m_SamplingFrequency;      /**< Sampling frequency of the extension */
    } m_Extension;
    
private:
    AP4_Result ParseAudioObjectType(AP4_Mp4AudioDsiParser& parser, AP4_UI08& object_type);
    AP4_Result ParseGASpecificInfo(AP4_Mp4AudioDsiParser& parser);
    AP4_Result ParseSamplingFrequency(AP4_Mp4AudioDsiParser& parser, 
                                      unsigned int&          sampling_frequency_index,
                                      unsigned int&          sampling_frequency);
    AP4_Result ParseExtension(AP4_Mp4AudioDsiParser& parser);
};


#endif // _AP4_MP4_AUDIO_INFO_H_
