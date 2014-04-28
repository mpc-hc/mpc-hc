/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about AAC files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_AacH
#define MediaInfo_File_AacH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_MPEG4_YES
    #include "MediaInfo/Multiple/File_Mpeg4_Descriptors.h"
#endif
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Aac
//***************************************************************************

struct sbr_handler
{
    //sbr_header
    int8u  bs_amp_res[2];
    int8u  bs_amp_res_FromHeader;
    int8u  bs_start_freq;
    int8u  bs_stop_freq;
    int8u  bs_xover_band;
    int8u  bs_freq_scale;
    int8u  bs_alter_scale;
    int8u  bs_noise_bands;

    //sbr_grid
    int8u   bs_num_env[2];
    bool    bs_freq_res[2][8];
    int8u   bs_num_noise[2];

    //sbr_dtdf
    int8u   bs_df_env[2][4];
    int8u   bs_df_noise[2][2];

    //Computed values
    int8u  num_noise_bands;
    int8u  num_env_bands[2];
};

struct ps_handler
{
    bool   enable_iid;
    bool   enable_icc;
    bool   enable_ext;
    int8u  iid_mode;
    int8u  icc_mode;
};

typedef const int8s (*sbr_huffman)[2];

class File_Aac : public File__Analyze, public File__Tags_Helper
{
public :
    //In
    int64u  Frame_Count_Valid;
    bool    FrameIsAlwaysComplete;
    enum mode
    {
        Mode_Unknown,
        Mode_AudioSpecificConfig,
        Mode_raw_data_block,
        Mode_ADIF,
        Mode_ADTS,
        Mode_LATM,
    };
    mode   Mode;
    void   AudioSpecificConfig_OutOfBand(int32u sampling_frequency, int8u audioObjectType=(int8u)-1, bool sbrData=false, bool psData=false, bool sbrPresentFlag=false, bool psPresentFlag=false);

    //Constructor/Destructor
    File_Aac();
    ~File_Aac();

protected :
    //Streams management
    void Streams_Accept();
    void Streams_Fill();
    void Streams_Update();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();
    void FileHeader_Parse_ADIF();

    //Buffer - Global
    void Read_Buffer_Continue ();
    void Read_Buffer_Continue_AudioSpecificConfig();
    void Read_Buffer_Continue_raw_data_block();

    //Buffer - Synchro
    bool Synchronize();
    bool Synchronize_ADTS();
    bool Synchronize_LATM();
    bool Synched_Test();
    bool Synched_Test_ADTS();
    bool Synched_Test_LATM();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    bool Demux_UnpacketizeContainer_Test_ADTS();
    bool Demux_UnpacketizeContainer_Test_LATM();
    #endif //MEDIAINFO_DEMUX

    //Buffer - Per element
    bool Header_Begin();
    bool Header_Begin_ADTS();
    bool Header_Begin_LATM();
    void Header_Parse();
    void Header_Parse_ADTS();
    void Header_Parse_LATM();
    void Data_Parse();
    void Data_Parse_ADTS();
    void Data_Parse_LATM();

    //***********************************************************************
    // Elements - Main
    //***********************************************************************

    //Elements - Interface to MPEG-4 container
    void AudioSpecificConfig                (size_t End=(size_t)-1);
    void GetAudioObjectType                 (int8u &ObjectType, const char* Name);

    //Elements - Multiplex layer
    void EPMuxElement                       ();
    void AudioMuxElement                    ();
    void StreamMuxConfig                    ();
    int32u LatmGetValue                     ();
    void PayloadLengthInfo                  ();
    void PayloadMux                         ();
    bool muxConfigPresent;

    //Elements - Error protection
    void ErrorProtectionSpecificConfig      ();

    //Elements - MPEG-2 AAC Audio_Data_Interchange_Format, ADIF
    void adif_header                        ();

    //Elements - Audio_Data_Transport_Stream frame, ADTS
    void adts_frame                         ();
    void adts_fixed_header                  ();
    void adts_variable_header               ();

    //Temp
    int8u   numSubFrames;
    int8u   numProgram;
    int8u   numLayer;
    int8u   numChunk;
    bool    audioMuxVersionA;
    int8u   streamID[16][8];
    int8u   progSIndx[128];
    int8u   laySIndx[128];
    int8u   progCIndx[128];
    int8u   layCIndx[128];
    int8u   frameLengthType[128];
    int16u  frameLength[128];
    int32u  MuxSlotLengthBytes[128];
    int32u  otherDataLenBits;
    bool    otherDataPresent;
    bool    allStreamsSameTimeFraming;
    int8u   audioObjectType;
    int8u   extensionAudioObjectType;
    int8u   channelConfiguration;
    int16u  frame_length;
    int8u   sampling_frequency_index;
    int32u  sampling_frequency;
    int8u   extension_sampling_frequency_index;
    int32u  extension_sampling_frequency;
    bool    aacScalefactorDataResilienceFlag;
    bool    aacSectionDataResilienceFlag;
    bool    aacSpectralDataResilienceFlag;
    std::vector<int16u> aac_frame_lengths;
    int8u   num_raw_data_blocks;
    bool    protection_absent;
    int64u  FrameSize_Min;
    int64u  FrameSize_Max;

    //***********************************************************************
    // Elements - Speech coding (HVXC)
    //***********************************************************************

    void HvxcSpecificConfig                 ();
    void HVXCconfig                         ();
    void ErrorResilientHvxcSpecificConfig   ();
    void ErHVXCconfig                       ();

    //***********************************************************************
    // Elements - Speech Coding (CELP)
    //***********************************************************************

    void CelpSpecificConfig                 ();
    void CelpHeader                         ();
    void ErrorResilientCelpSpecificConfig   ();
    void ER_SC_CelpHeader                   ();

    //***********************************************************************
    // Elements - General Audio (GA)
    //***********************************************************************

    //Elements - Decoder configuration
    void GASpecificConfig                   ();
    void program_config_element             ();

    //Elements - GA bitstream
    void raw_data_block                     ();
    void single_channel_element             ();
    void channel_pair_element               ();
    void ics_info                           ();
    void pulse_data                         ();
    void coupling_channel_element           ();
    void lfe_channel_element                ();
    void data_stream_element                ();
    void fill_element                       (int8u old_id);
    void gain_control_data                  ();

    //Elements - Subsidiary
    void individual_channel_stream          (bool common_window, bool scale_flag);
    void section_data                       ();
    void scale_factor_data                  ();
    void tns_data                           ();
    void ltp_data                           ();
    void spectral_data                      ();
    void extension_payload                  (size_t End, int8u id_aac);
    void dynamic_range_info                 ();
    void sac_extension_data                 (size_t End);

    //Elements - SBR
    void sbr_extension_data                 (size_t End, int8u id_aac, bool crc_flag);
    void sbr_header                         ();
    void sbr_data                           (int8u id_aac);
    void sbr_single_channel_element         ();
    void sbr_channel_pair_element           ();
    void sbr_grid                           (bool ch);
    void sbr_dtdf                           (bool ch);
    void sbr_invf                           (bool ch);
    void sbr_envelope                       (bool ch, bool bs_coupling);
    void sbr_noise                          (bool ch, bool bs_coupling);
    void sbr_sinusoidal_coding              (bool ch);
    int16u sbr_huff_dec                     (sbr_huffman Table, const char* Name);

    //Elements - SBR - PS
    void ps_data                            (size_t End);

    //Elements - Perceptual noise substitution (PNS)
    bool is_noise                           (size_t group, size_t sfb);
    int  is_intensity                       (size_t group, size_t sfb);

    //Elements - Enhanced Low Delay Codec
    void ELDSpecificConfig                  ();
    void ld_sbr_header                      ();

    //Helpers
    void hcod                               (int8u sect_cb, const char* Name);
    void hcod_sf                            (const char* Name);
    void hcod_binary                        (int8u CodeBook, int8s* Values, int8u Values_Count);
    void hcod_2step                         (int8u CodeBook, int8s* Values, int8u Values_Count);

    //Temp - channel_pair_element
    bool    common_window;

    //Temp - ics_info
    int8u   window_sequence;
    int8u   max_sfb;
    int8u   scale_factor_grouping;
    int8u   num_windows;
    int8u   num_window_groups;
    int8u   window_group_length             [8];
    int16u  sect_sfb_offset                 [8][1024];
    int16u  swb_offset                      [64];
    int8u   sfb_cb                          [8][64];
    int8u   num_swb;

    //Temp - section_data
    int8u   num_sec                         [8];
    int8u   sect_cb                         [8][64];
    int16u  sect_start                      [8][64];
    int16u  sect_end                        [8][64];

    //Temp - ltp_data
    int16u  ltp_lag;

    //Temp - SBR
    sbr_handler* sbr;

    //Temp - PS
    ps_handler* ps;

    //***********************************************************************
    // Elements - Structured Audio (SA)
    //***********************************************************************

    void StructuredAudioSpecificConfig      ();

    //***********************************************************************
    // Elements - Text to Speech Interface (TTSI)
    //***********************************************************************

    void TTSSpecificConfig                  ();

    //***********************************************************************
    // Elements - Parametric Audio (HILN)
    //***********************************************************************

    void HILNconfig                         ();
    void HILNenexConfig                     ();
    void ParametricSpecificConfig           ();
    void PARAconfig                         ();

    //***********************************************************************
    // Elements - Technical description of parametric coding for high quality audio
    //***********************************************************************

    void SSCSpecificConfig                  ();

    //***********************************************************************
    // Elements - MPEG-1/2 Audio
    //***********************************************************************

    void MPEG_1_2_SpecificConfig            ();

    //***********************************************************************
    // Elements - Technical description of lossless coding of oversampled audio
    //***********************************************************************

    void DSTSpecificConfig                  ();
    //***********************************************************************
    // Elements - Audio Lossless
    //***********************************************************************

    void ALSSpecificConfig                  ();

    //***********************************************************************
    // Elements - Scalable lossless
    //***********************************************************************

    void SLSSpecificConfig                  ();

    //***********************************************************************
    // Temp
    //***********************************************************************

    std::map<std::string, Ztring>   Infos_General;
    std::map<std::string, Ztring>   Infos;
    bool                            CanFill;
};

} //NameSpace

#endif

