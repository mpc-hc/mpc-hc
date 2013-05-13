/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AAC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aac.h"
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
#include <cmath>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Speech coding (HVXC)
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::HvxcSpecificConfig()
{
    Element_Begin1("HvxcSpecificConfig");
    bool isBaseLayer;
    Get_SB(isBaseLayer,                                         "isBaseLayer");
    if (isBaseLayer)
        HVXCconfig();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::HVXCconfig()
{
    Element_Begin1("HVXCconfig");
    Skip_SB(                                                    "HVXCvarMode");
    Skip_S1(2,                                                  "HVXCrateMode");
    Skip_SB(                                                    "extensionFlag");
    //~ if (extensionFlag) {
        /*< to be defined in MPEG-4 Version 2 >*/
    //~ }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::ErrorResilientHvxcSpecificConfig() {
    Element_Begin1("ErrorResilientHvxcSpecificConfig");
    bool isBaseLayer;
    Get_SB(isBaseLayer,"isBaseLayer");
    if (isBaseLayer) {
        ErHVXCconfig();
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::ErHVXCconfig()
{
    Element_Begin1("ErHVXCconfig");
    bool extensionFlag;
    Skip_SB(                                                    "HVXCvarMode");
    Skip_S1(2,                                                  "HVXCrateMode");
    Get_SB (extensionFlag,                                      "extensionFlag");

    if (extensionFlag) {
        Skip_SB(                                                "var_ScalableFlag");
    }
    Element_End0();
}

//***************************************************************************
// Speech Coding (CELP)
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::CelpSpecificConfig ()
{
    Element_Begin1("CelpSpecificConfig");
    bool isBaseLayer;
    Get_SB(isBaseLayer,                                         "isBaseLayer");
    if (isBaseLayer)
    {
        CelpHeader ();
    }
    else
    {
        bool isBWSLayer;
        Get_SB(isBWSLayer,                                      "isBWSLayer");
        if (isBWSLayer)
        {
            //~ CelpBWSenhHeader ()
            //~ {
            Skip_S1(2,                                          "BWS_configuration");
            //~ }

        }
        else
        {
            Skip_S1(2,                                          "CELP-BRS-id");
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::CelpHeader ()
{
    Element_Begin1("CelpHeader");
    bool ExcitationMode;
    Get_SB(ExcitationMode,                                      "ExcitationMode");
    Skip_SB(                                                    "SampleRateMode");
    Skip_SB(                                                    "FineRateControl");
    if (ExcitationMode == 1/*RPE*/)
    {
        Skip_S1(3,                                              "RPE_Configuration");
    }
    if (ExcitationMode == 0/*MPE*/)
    {
        Skip_S1(5,                                              "MPE_Configuration");
        Skip_S1(2,                                              "NumEnhLayers");
        Skip_SB(                                                "BandwidthScalabilityMode");
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::ErrorResilientCelpSpecificConfig ()
{
    Element_Begin1("ErrorResilientCelpSpecificConfig");
    bool isBaseLayer;
    Get_SB(isBaseLayer,                                         "isBaseLayer");
    if (isBaseLayer)
    {
        ER_SC_CelpHeader ();
    }
    else
    {
        bool isBWSLayer;
        Get_SB(isBWSLayer,                                      "isBWSLayer");
        if (isBWSLayer)
        {
            //~ CelpBWSenhHeader ()
            //~ {
            Skip_S1(2,                                          "BWS_configuration");
            //~ }

        }
        else
        {
            Skip_S1(2,                                          "CELP-BRS-id");
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::ER_SC_CelpHeader ()
{
    Element_Begin1("ER_SC_CelpHeader");
    bool ExcitationMode;
    Get_SB(ExcitationMode,                                      "ExcitationMode");
    Skip_SB(                                                    "SampleRateMode");
    Skip_SB(                                                    "FineRateControl");
    Skip_SB(                                                    "SilenceCompression");

    if (ExcitationMode == 1/*RPE*/) {
        Skip_S1(3,                                              "RPE_Configuration");
    }
    if (ExcitationMode == 0/*MPE*/) {
        Skip_S1(5,                                              "MPE_Configuration");
        Skip_S1(2,                                              "NumEnhLayers");
        Skip_SB(                                                "BandwidthScalabilityMode");
    }
    Element_End0();
}

//***************************************************************************
// Structured Audio (SA)
//***************************************************************************

//***************************************************************************
// Text to Speech Interface (TTSI)
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::TTSSpecificConfig()
{
    Element_Begin1("TTSSpecificConfig");
    //~ TTS_Sequence()
    //~ {
    Skip_S1(5,                                                  "TTS_Sequence_ID");
    Skip_BS(18,                                                 "Language_Code");
    Skip_SB(                                                    "Gender_Enable");
    Skip_SB(                                                    "Age_Enable");
    Skip_SB(                                                    "Speech_Rate_Enable");
    Skip_SB(                                                    "Prosody_Enable");
    Skip_SB(                                                    "Video_Enable");
    Skip_SB(                                                    "Lip_Shape_Enable");
    Skip_SB(                                                    "Trick_Mode_Enable");
    //~ }
    Element_End0();
}

//***************************************************************************
// Parametric Audio (HILN)
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::HILNconfig()
{
    Element_Begin1("HILNconfig");
    Skip_SB(                                                    "HILNquantMode");
    Skip_S1(8,                                                  "HILNmaxNumLine");
    Skip_S1(4,                                                  "HILNsampleRateCode");
    Skip_S2(12,                                                 "HILNframeLength");
    Skip_S1(2,                                                  "HILNcontMode");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::HILNenexConfig()
{
    Element_Begin1("HILNenexConfig");
    bool HILNenhaLayer;
    Get_SB(HILNenhaLayer,                                       "HILNenhaLayer");
    if (HILNenhaLayer)
        Skip_S1(2,                                              "HILNenhaQuantMode");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::ParametricSpecificConfig()
{
    Element_Begin1("ParametricSpecificConfig");
    bool isBaseLayer;
    Get_SB(isBaseLayer,                                         "isBaseLayer");
    if (isBaseLayer)
        PARAconfig();
    else
        HILNenexConfig();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::PARAconfig()
{
    Element_Begin1("PARAconfig");
    int8u PARAmode;
    Get_S1(2,PARAmode,                                          "PARAmode");
    if (PARAmode != 1)
        ErHVXCconfig();
    if (PARAmode != 0)
        HILNconfig();
    bool PARAextensionFlag;
    Get_SB(PARAextensionFlag,                                   "PARAextensionFlag");
    if (PARAextensionFlag) {
        /* to be defined in MPEG-4 Phase 3 */
    }
    Element_End0();
}

//***************************************************************************
// Technical description of parametric coding for high quality audio
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::SSCSpecificConfig ()
{
    Element_Begin1("SSCSpecificConfig");
    Skip_S1(2,"decoder_level");
    Skip_S1(4,"update_rate");
    Skip_S1(2,"synthesis_method");
    if (channelConfiguration != 1)
    {
        int8u mode_ext;
        Get_S1(2,mode_ext,"mode_ext");
        if ((channelConfiguration == 2) && (mode_ext == 1))
        {
            /*reserved*/
        }
    }
    Element_End0();
}

//***************************************************************************
// MPEG-1/2 Audio
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::MPEG_1_2_SpecificConfig()
{
    Element_Begin1("MPEG_1_2_SpecificConfig");
    Skip_SB(                                                    "extension");
    Element_End0();
}

//***************************************************************************
// Technical description of lossless coding of oversampled audio
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::DSTSpecificConfig()
{
    Element_Begin1("DSTSpecificConfig");
    Skip_SB("DSDDST_Coded");
    Skip_S2(14,"N_Channels");
    Skip_SB("reserved");
    Element_End0();
}

//***************************************************************************
// Audio Lossless
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::ALSSpecificConfig()
{
    //Not in spec, but something weird in the example I have
    int32u Junk;
    while (Data_BS_Remain())
    {
        Peek_S4(32, Junk);
        if (Junk!=0x414C5300)
        {
            Skip_SB(                                            "Unknown");
        }
        else
            break;
    }
    if (Data_BS_Remain()==0)
        return; //There is a problem

    Element_Begin1("ALSSpecificConfig");
    bool chan_config,chan_sort,crc_enabled,aux_data_enabled;
    int32u samp_freq, samples;
    int16u channels,frame_length;
    int8u ra_flag,random_access, file_type;
    Skip_BS(32,"als_id");
    Get_BS (32, samp_freq,                                      "samp_freq");
    Get_BS (32, samples,                                        "samples");
    Get_S2 (16, channels,                                       "channels"); Param_Info2(channels+1, " channel(s)");
    Get_S1 (3, file_type,                                       "file_type");
    Skip_S1(3,"resolution");
    Skip_SB("floating");
    Skip_SB("msb_first");
    Get_S2 (16,frame_length,"frame_length");
    Get_S1 (8,random_access,"random_access");
    Get_S1 (2,ra_flag,"ra_flag");
    Skip_SB("adapt_order");
    Skip_S1(2,"coef_table");
    Skip_SB("long_term_prediction");
    Skip_S2(10,"max_order");
    Skip_S1(2,"block_switching");
    Skip_SB("bgmc_mode");
    Skip_SB("sb_part");
    Skip_SB("joint_stereo");
    Skip_SB("mc_coding");
    Get_SB (chan_config,"chan_config");
    Get_SB (chan_sort,"chan_sort");
    Get_SB (crc_enabled,"crc_enabled");
    Skip_SB("RLSLMS");
    Skip_BS(5,"(reserved)");
    Get_SB (aux_data_enabled,"aux_data_enabled");
    if (chan_config)
        Skip_S2(16,"chan_config_info");
    if (chan_sort)
    {
        int16u ChBits=(int16u)ceil(log((double)(channels+1))/log((double)2));
        for (int8u c=0; c<=channels; c++)
            Skip_BS(ChBits,                                     "chan_pos[c]");
    }
    if(Data_BS_Remain()%8)
        Skip_S1(Data_BS_Remain()%8,                             "byte_align");
    BS_End();
    int32u header_size,trailer_size;
    Get_B4(header_size,                                         "header_size");
    Get_B4(trailer_size,                                        "trailer_size");
    #ifdef MEDIAINFO_RIFF_YES
    if (file_type==1) //WAVE file
    {
        Element_Begin1("orig_header");
        File_Riff MI;
        Open_Buffer_Init(&MI);
        Open_Buffer_Continue(&MI, Buffer+Buffer_Offset+(size_t)Element_Offset, header_size);
        Element_Offset+=header_size;
        File__Analyze::Finish(&MI); //No merge of data, only for trace information, because this is the data about the decoded stream, not the encoded stream
        Element_End0();
    }
    else
    #endif //MEDIAINFO_RIFF_YES
        Skip_XX(header_size,                                    "orig_header[]");

    Skip_XX(trailer_size,                                       "orig_trailer[]");
    if (crc_enabled)
        Skip_B4(                                                "crc");
    if ((ra_flag == 2) && (random_access > 0))
        for (int32u f=0; f<((samples-1)/(frame_length+1))+1; f++)
            Skip_B4(                                            "ra_unit_size[f]");
    if (aux_data_enabled)
    {
        int32u aux_size;
        Get_B4(aux_size,                                        "aux_size");
        Skip_XX(aux_size,                                       "aux_data[]");
    }
    Element_End0();
    BS_Begin(); //To be in sync with other objectTypes

    FILLING_BEGIN();
        //Filling
        File__Analyze::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, channels+1);

        //Forcing default confignuration (something weird in the example I have)
        channelConfiguration=0;
        sampling_frequency_index=(int8u)-1;
        sampling_frequency=samp_freq;
    FILLING_END();
}

//***************************************************************************
// Scalable lossless
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::SLSSpecificConfig()
{
    Element_Begin1("SLSSpecificConfig");
    Skip_S1(3,"pcmWordLength");
    Skip_SB("aac_core_present");
    Skip_SB("lle_main_stream");
    Skip_SB("reserved_bit");
    Skip_S1(3,"frameLength");
    if (!channelConfiguration)
        program_config_element();
    Element_End0();
}

} //NameSpace

#endif //MEDIAINFO_AAC_YES

