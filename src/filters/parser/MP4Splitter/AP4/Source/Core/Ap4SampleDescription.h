/*****************************************************************
|
|    AP4 - Sample Descriptions
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

#ifndef _AP4_SAMPLE_DESCRIPTION_H_
#define _AP4_SAMPLE_DESCRIPTION_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4EsDescriptor.h"
#include "Ap4EsdsAtom.h"
#include "Ap4Array.h"
#include "Ap4AvccAtom.h"
#include "Ap4DynamicCast.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_SampleEntry;
class AP4_DataBuffer;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
#define AP4_SAMPLE_FORMAT_MP4A AP4_ATOM_TYPE('m','p','4','a')
#define AP4_SAMPLE_FORMAT_MP4V AP4_ATOM_TYPE('m','p','4','v')
#define AP4_SAMPLE_FORMAT_MP4S AP4_ATOM_TYPE('m','p','4','s')
#define AP4_SAMPLE_FORMAT_AVC1 AP4_ATOM_TYPE('a','v','c','1')
#define AP4_SAMPLE_FORMAT_ALAC AP4_ATOM_TYPE('a','l','a','c')
#define AP4_SAMPLE_FORMAT_OWMA AP4_ATOM_TYPE('o','w','m','a')
#define AP4_SAMPLE_FORMAT_OVC1 AP4_ATOM_TYPE('o','v','c','1')
#define AP4_SAMPLE_FORMAT_AVCP AP4_ATOM_TYPE('a','v','c','p')
#define AP4_SAMPLE_FORMAT_DRAC AP4_ATOM_TYPE('d','r','a','c')
#define AP4_SAMPLE_FORMAT_DRA1 AP4_ATOM_TYPE('d','r','a','1')
#define AP4_SAMPLE_FORMAT_AC_3 AP4_ATOM_TYPE('a','c','-','3')
#define AP4_SAMPLE_FORMAT_EC_3 AP4_ATOM_TYPE('e','c','-','3')
#define AP4_SAMPLE_FORMAT_G726 AP4_ATOM_TYPE('g','7','2','6')
#define AP4_SAMPLE_FORMAT_MJP2 AP4_ATOM_TYPE('m','j','p','2')
#define AP4_SAMPLE_FORMAT_OKSD AP4_ATOM_TYPE('o','k','s','d')
#define AP4_SAMPLE_FORMAT_RAW_ AP4_ATOM_TYPE('r','a','w',' ')
#define AP4_SAMPLE_FORMAT_RTP_ AP4_ATOM_TYPE('r','t','p',' ')
#define AP4_SAMPLE_FORMAT_S263 AP4_ATOM_TYPE('s','2','6','3')
#define AP4_SAMPLE_FORMAT_SAMR AP4_ATOM_TYPE('s','a','m','r')
#define AP4_SAMPLE_FORMAT_SAWB AP4_ATOM_TYPE('s','a','w','b')
#define AP4_SAMPLE_FORMAT_SAWP AP4_ATOM_TYPE('s','a','w','p')
#define AP4_SAMPLE_FORMAT_SEVC AP4_ATOM_TYPE('s','e','v','c')
#define AP4_SAMPLE_FORMAT_SQCP AP4_ATOM_TYPE('s','q','c','p')
#define AP4_SAMPLE_FORMAT_SRTP AP4_ATOM_TYPE('s','r','t','p')
#define AP4_SAMPLE_FORMAT_SSMV AP4_ATOM_TYPE('s','s','m','v')
#define AP4_SAMPLE_FORMAT_TEXT AP4_ATOM_TYPE('t','e','t','x')
#define AP4_SAMPLE_FORMAT_TWOS AP4_ATOM_TYPE('t','w','o','s')
#define AP4_SAMPLE_FORMAT_TX3G AP4_ATOM_TYPE('t','x','3','g')
#define AP4_SAMPLE_FORMAT_VC_1 AP4_ATOM_TYPE('v','c','-','1')
#define AP4_SAMPLE_FORMAT_XML_ AP4_ATOM_TYPE('x','m','l',' ')

const char*
AP4_GetFormatName(AP4_UI32 format);

/*----------------------------------------------------------------------
|   AP4_SampleDescription
+---------------------------------------------------------------------*/
class AP4_SampleDescription
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_SampleDescription)

    // type constants of the sample description
    enum Type {
        TYPE_UNKNOWN   = 0x00,
        TYPE_MPEG      = 0x01,
        TYPE_PROTECTED = 0x02,
        TYPE_AVC       = 0x03
    };

    // constructors & destructor
    AP4_SampleDescription(Type            type, 
                          AP4_UI32        format, 
                          AP4_AtomParent* details);
    virtual ~AP4_SampleDescription() {}
    virtual AP4_SampleDescription* Clone(AP4_Result* result = NULL);
    
    // accessors
    Type            GetType()   const { return m_Type;    }
    AP4_UI32        GetFormat() const { return m_Format;  }
    AP4_AtomParent& GetDetails()      { return m_Details; }
    
    // factories
    virtual AP4_Atom* ToAtom() const;

 protected:
    Type           m_Type;
    AP4_UI32       m_Format;
    AP4_AtomParent m_Details;
};

/*----------------------------------------------------------------------
|   AP4_UnknownSampleDescription
+---------------------------------------------------------------------*/
class AP4_UnknownSampleDescription : public AP4_SampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_UnknownSampleDescription, AP4_SampleDescription)

    // this constructor takes makes a copy of the atom passed as an argument
    AP4_UnknownSampleDescription(AP4_Atom* atom);
    ~AP4_UnknownSampleDescription();

    virtual AP4_SampleDescription* Clone(AP4_Result* result);
    virtual AP4_Atom* ToAtom() const;    
    
    // accessor
    const AP4_Atom* GetAtom() { return m_Atom; }
    
private:
    AP4_Atom* m_Atom;
};

/*----------------------------------------------------------------------
|   AP4_AudioSampleDescription  // MIXIN class
+---------------------------------------------------------------------*/
class AP4_AudioSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_AudioSampleDescription)

    // constructor
    AP4_AudioSampleDescription(unsigned int sample_rate,
                               unsigned int sample_size,
                               unsigned int channel_count) :
    m_SampleRate(sample_rate),
    m_SampleSize(sample_size),
    m_ChannelCount(channel_count) {}

    // accessors
    AP4_UI32 GetSampleRate()   { return m_SampleRate;   }
    AP4_UI16 GetSampleSize()   { return m_SampleSize;   }
    AP4_UI16 GetChannelCount() { return m_ChannelCount; }

protected:
    // members
    AP4_UI32 m_SampleRate;
    AP4_UI16 m_SampleSize;
    AP4_UI16 m_ChannelCount;
};

/*----------------------------------------------------------------------
|   AP4_VideoSampleDescription  // MIXIN class
+---------------------------------------------------------------------*/
class AP4_VideoSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_VideoSampleDescription)

    // constructor
    AP4_VideoSampleDescription(AP4_UI16    width,
                               AP4_UI16    height,
                               AP4_UI16    depth,
                               const char* compressor_name) :
    m_Width(width),
    m_Height(height),
    m_Depth(depth),
    m_CompressorName(compressor_name) {}

    // accessors
    AP4_UI16    GetWidth()          { return m_Width;  }
    AP4_UI16    GetHeight()         { return m_Height; }
    AP4_UI16    GetDepth()          { return m_Depth;  }
    const char* GetCompressorName() { return m_CompressorName.GetChars(); }

protected:
    // members
    AP4_UI16   m_Width;
    AP4_UI16   m_Height;
    AP4_UI16   m_Depth;
    AP4_String m_CompressorName;
};

/*----------------------------------------------------------------------
|   AP4_GenericAudioSampleDescription
+---------------------------------------------------------------------*/
class AP4_GenericAudioSampleDescription : public AP4_SampleDescription,
                                          public AP4_AudioSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D2(AP4_GenericAudioSampleDescription, AP4_SampleDescription, AP4_AudioSampleDescription)

    // constructors
    AP4_GenericAudioSampleDescription(AP4_UI32        format,
                                      unsigned int    sample_rate,
                                      unsigned int    sample_size,
                                      unsigned int    channel_count,
                                      AP4_AtomParent* details) :
        AP4_SampleDescription(TYPE_UNKNOWN, format, details),
        AP4_AudioSampleDescription(sample_rate, sample_size, channel_count) {}
};

/*----------------------------------------------------------------------
|   AP4_GenericVideoSampleDescription
+---------------------------------------------------------------------*/
class AP4_GenericVideoSampleDescription : public AP4_SampleDescription,
                                          public AP4_VideoSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D2(AP4_GenericVideoSampleDescription, AP4_SampleDescription, AP4_VideoSampleDescription)

    // constructor
    AP4_GenericVideoSampleDescription(AP4_UI32        format,
                                      AP4_UI16        width,
                                      AP4_UI16        height,
                                      AP4_UI16        depth,
                                      const char*     compressor_name,
                                      AP4_AtomParent* details) :
    AP4_SampleDescription(TYPE_UNKNOWN, format, details),
    AP4_VideoSampleDescription(width, height, depth, compressor_name) {}
};

/*----------------------------------------------------------------------
|   AP4_AvcSampleDescription
+---------------------------------------------------------------------*/
class AP4_AvcSampleDescription : public AP4_SampleDescription,
                                 public AP4_VideoSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D2(AP4_AvcSampleDescription, AP4_SampleDescription, AP4_VideoSampleDescription)

    // constructors
    AP4_AvcSampleDescription(AP4_UI16            width,
                             AP4_UI16            height,
                             AP4_UI16            depth,
                             const char*         compressor_name,
                             const AP4_AvccAtom* avcc);
    
    AP4_AvcSampleDescription(AP4_UI16        width,
                             AP4_UI16        height,
                             AP4_UI16        depth,
                             const char*     compressor_name,
                             AP4_AtomParent* details);

    AP4_AvcSampleDescription(AP4_UI16                         width,
                             AP4_UI16                         height,
                             AP4_UI16                         depth,
                             const char*                      compressor_name,
                             AP4_UI08                         profile,
                             AP4_UI08                         level,
                             AP4_UI08                         profile_compatibility,
                             AP4_UI08                         nalu_length_size,
                             const AP4_Array<AP4_DataBuffer>& sequence_parameters,
                             const AP4_Array<AP4_DataBuffer>& picture_parameters);
    
    // accessors
    AP4_UI08 GetConfigurationVersion() const { return m_AvccAtom->GetConfigurationVersion(); }
    AP4_UI08 GetProfile() const { return m_AvccAtom->GetProfile(); }
    AP4_UI08 GetLevel() const { return m_AvccAtom->GetLevel(); }
    AP4_UI08 GetProfileCompatibility() const { return m_AvccAtom->GetProfileCompatibility(); }
    AP4_UI08 GetNaluLengthSize() const { return m_AvccAtom->GetNaluLengthSize(); }
    AP4_Array<AP4_DataBuffer>& GetSequenceParameters() {return m_AvccAtom->GetSequenceParameters(); }
    AP4_Array<AP4_DataBuffer>& GetPictureParameters() { return m_AvccAtom->GetPictureParameters(); }
    const AP4_DataBuffer& GetRawBytes() const { return m_AvccAtom->GetRawBytes(); }
    
    // inherited from AP4_SampleDescription
    virtual AP4_Atom* ToAtom() const;
    
    // static methods
    static const char* GetProfileName(AP4_UI08 profile) {
        return AP4_AvccAtom::GetProfileName(profile);
    }

private:
    AP4_AvccAtom* m_AvccAtom;
};

/*----------------------------------------------------------------------
|   AP4_MpegSampleDescription
+---------------------------------------------------------------------*/
class AP4_MpegSampleDescription : public AP4_SampleDescription
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_MpegSampleDescription, AP4_SampleDescription)

    // types
    typedef AP4_UI08 StreamType;
    typedef AP4_UI08 OTI;
    
    // class methods
    static const char* GetStreamTypeString(StreamType type);
    static const char* GetObjectTypeString(OTI oti);

    // constructors & destructor
    AP4_MpegSampleDescription(AP4_UI32      format,
                              AP4_EsdsAtom* esds);
    AP4_MpegSampleDescription(AP4_UI32              format,
                              StreamType            stream_type,
                              OTI                   oti,
                              const AP4_DataBuffer* decoder_info,
                              AP4_UI32              buffer_size,
                              AP4_UI32              max_bitrate,
                              AP4_UI32              avg_bitrate);
    
    // accessors
    AP4_Byte GetStreamType()   const { return m_StreamType; }
    AP4_Byte GetObjectTypeId() const { return m_ObjectTypeId; }
    AP4_UI32 GetBufferSize()   const { return m_BufferSize; }
    AP4_UI32 GetMaxBitrate()   const { return m_MaxBitrate; }
    AP4_UI32 GetAvgBitrate()   const { return m_AvgBitrate; }
    const AP4_DataBuffer& GetDecoderInfo() const { return m_DecoderInfo; }

    // methods
    AP4_EsDescriptor* CreateEsDescriptor() const;

 protected:
    // members
    AP4_UI32       m_Format;
    StreamType     m_StreamType;
    OTI            m_ObjectTypeId;
    AP4_UI32       m_BufferSize;
    AP4_UI32       m_MaxBitrate;
    AP4_UI32       m_AvgBitrate;
    AP4_DataBuffer m_DecoderInfo;
};

/*----------------------------------------------------------------------
|   AP4_MpegSystemSampleDescription
+---------------------------------------------------------------------*/
class AP4_MpegSystemSampleDescription : public AP4_MpegSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_MpegSystemSampleDescription, AP4_MpegSampleDescription)

    // constructor
    AP4_MpegSystemSampleDescription(AP4_EsdsAtom* esds);
    AP4_MpegSystemSampleDescription(StreamType            stream_type,
                                    OTI                   oti,
                                    const AP4_DataBuffer* decoder_info,
                                    AP4_UI32              buffer_size,
                                    AP4_UI32              max_bitrate,
                                    AP4_UI32              avg_bitrate);

    // methods
    AP4_Atom* ToAtom() const;
};

/*----------------------------------------------------------------------
|   AP4_MpegAudioSampleDescription
+---------------------------------------------------------------------*/
class AP4_MpegAudioSampleDescription : public AP4_MpegSampleDescription,
                                       public AP4_AudioSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D2(AP4_MpegAudioSampleDescription, AP4_MpegSampleDescription, AP4_AudioSampleDescription)

    // types
    typedef AP4_UI08 Mpeg4AudioObjectType;
    
    // class methods
    static const char* GetMpeg4AudioObjectTypeString(Mpeg4AudioObjectType type);
    
    // constructor
    AP4_MpegAudioSampleDescription(unsigned int  sample_rate,
                                   unsigned int  sample_size,
                                   unsigned int  channel_count,
                                   AP4_EsdsAtom* esds);
                                   
    AP4_MpegAudioSampleDescription(OTI                   oti,
                                   unsigned int          sample_rate,
                                   unsigned int          sample_size,
                                   unsigned int          channel_count,
                                   const AP4_DataBuffer* decoder_info,
                                   AP4_UI32              buffer_size,
                                   AP4_UI32              max_bitrate,
                                   AP4_UI32              avg_bitrate);

    // methods
    AP4_Atom* ToAtom() const;

    /**
     * For sample descriptions of MPEG-4 audio tracks (i.e GetObjectTypeId() 
     * returns AP4_OTI_MPEG4_AUDIO), this method returns the MPEG4 Audio Object 
     * Type. For other sample descriptions, this method returns 0.
     */
    Mpeg4AudioObjectType GetMpeg4AudioObjectType() const;
};

/*----------------------------------------------------------------------
|   AP4_MpegVideoSampleDescription
+---------------------------------------------------------------------*/
class AP4_MpegVideoSampleDescription : public AP4_MpegSampleDescription,
                                       public AP4_VideoSampleDescription
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D2(AP4_MpegVideoSampleDescription, AP4_MpegSampleDescription, AP4_VideoSampleDescription)

    // constructor
    AP4_MpegVideoSampleDescription(AP4_UI16      width,
                                   AP4_UI16      height,
                                   AP4_UI16      depth,
                                   const char*   compressor_name,
                                   AP4_EsdsAtom* esds);
                                   
    AP4_MpegVideoSampleDescription(OTI                   oti,
                                   AP4_UI16              width,
                                   AP4_UI16              height,
                                   AP4_UI16              depth,
                                   const char*           compressor_name,
                                   const AP4_DataBuffer* decoder_info,
                                   AP4_UI32              buffer_size,
                                   AP4_UI32              max_bitrate,
                                   AP4_UI32              avg_bitrate);

    // methods
    AP4_Atom* ToAtom() const;
};

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_FORBIDDEN = 0x00;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_OD        = 0x01;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_CR        = 0x02;	
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_BIFS      = 0x03;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_VISUAL    = 0x04;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_AUDIO     = 0x05;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_MPEG7     = 0x06;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_IPMP      = 0x07;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_OCI       = 0x08;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_MPEGJ     = 0x09;
const AP4_MpegSampleDescription::StreamType AP4_STREAM_TYPE_TEXT      = 0x0D;

const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG4_SYSTEM         = 0x01;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG4_SYSTEM_COR     = 0x02;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG4_TEXT           = 0x08;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG4_VISUAL         = 0x20;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG4_AUDIO          = 0x40;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_VISUAL_SIMPLE  = 0x60;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_VISUAL_MAIN    = 0x61;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_VISUAL_SNR     = 0x62;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_VISUAL_SPATIAL = 0x63;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_VISUAL_HIGH    = 0x64;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_VISUAL_422     = 0x65;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_AAC_AUDIO_MAIN = 0x66;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_AAC_AUDIO_LC   = 0x67;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_AAC_AUDIO_SSRP = 0x68;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG2_PART3_AUDIO    = 0x69;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG1_VISUAL         = 0x6A;
const AP4_MpegSampleDescription::OTI AP4_OTI_MPEG1_AUDIO          = 0x6B;
const AP4_MpegSampleDescription::OTI AP4_OTI_JPEG                 = 0x6C;

const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_MAIN              = 1;  /**< AAC Main Profile                             */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LC                = 2;  /**< AAC Low Complexity                           */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_SSR               = 3;  /**< AAC Scalable Sample Rate                     */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_LTP               = 4;  /**< AAC Long Term Predictor                      */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_SBR                   = 5;  /**< Spectral Band Replication                    */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_AAC_SCALABLE          = 6;  /**< AAC Scalable                                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_TWINVQ                = 7;  /**< Twin VQ                                      */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_CELP                  = 8;  /**< CELP                                         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_HVXC                  = 9;  /**< HVXC                                         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_TTSI                  = 12; /**< TTSI                                         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_MAIN_SYNTHETIC        = 13; /**< Main Synthetic                               */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_WAVETABLE_SYNTHESIS   = 14; /**< WavetableSynthesis                           */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_GENERAL_MIDI          = 15; /**< General MIDI                                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ALGORITHMIC_SYNTHESIS = 16; /**< Algorithmic Synthesis                        */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LC             = 17; /**< Error Resilient AAC Low Complexity           */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LTP            = 19; /**< Error Resilient AAC Long Term Prediction     */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE       = 20; /**< Error Resilient AAC Scalable                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_TWINVQ             = 21; /**< Error Resilient Twin VQ                      */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC               = 22; /**< Error Resilient Bit Sliced Arithmetic Coding */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LD             = 23; /**< Error Resilient AAC Low Delay                */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_CELP               = 24; /**< Error Resilient CELP                         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_HVXC               = 25; /**< Error Resilient HVXC                         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_HILN               = 26; /**< Error Resilient HILN                         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_PARAMETRIC         = 27; /**< Error Resilient Parametric                   */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_SSC                   = 28; /**< SSC                                          */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_PS                    = 29; /**< Parametric Stereo                            */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_MPEG_SURROUND         = 30; /**< MPEG Surround                                */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_LAYER_1               = 32; /**< MPEG Layer 1                                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_LAYER_2               = 33; /**< MPEG Layer 2                                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_LAYER_3               = 34; /**< MPEG Layer 3                                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_DST                   = 35; /**< DST Direct Stream Transfer                   */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ALS                   = 36; /**< ALS Lossless Coding                          */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_SLS                   = 37; /**< SLS Scalable Lossless Coding                 */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_SLS_NON_CORE          = 38; /**< SLS Sclable Lossless Coding Non-Core         */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_ELD            = 39; /**< Error Resilient AAC ELD                      */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_SMR_SIMPLE            = 40; /**< SMR Simple                                   */
const AP4_UI08 AP4_MPEG4_AUDIO_OBJECT_TYPE_SMR_MAIN              = 41; /**< SMR Main                                     */

// ==> Start patch MPC
const AP4_MpegSampleDescription::OTI AP4_NERO_VOBSUB              = 0xE0;
// <== End patch MPC

#endif // _AP4_SAMPLE_DESCRIPTION_H_

