// File_Aac - Info for AAC files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPEG4_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Mpeg4_AudioSpecificConfig.h"
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const int32u MP4_SamplingRate[]=
{96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
 16000, 12000, 11025,  8000,  7350,     0,     0,     0,};

//---------------------------------------------------------------------------
const char* MP4_ID[]=
{
    "MPEG-4",
    "MPEG-2",
};

//---------------------------------------------------------------------------
const char* MP4_Format(int8u ID)
{
    switch (ID)
    {
        case    1 :
        case    2 :
        case    3 :
        case    4 : return "AAC";
        case    5 : return "SBR";
        case    6 : return "AAC Scalable";
        case    7 : return "TwinVQ";
        case    8 : return "CELP";
        case    9 : return "HVXC";
        case   12 : return "TTSI";
        case   13 : return "Main synthetic";
        case   14 : return "Wavetable synthesis";
        case   15 : return "General MIDI";
        case   16 : return "Algorithmic Synthesis and Audio FX";
        case   17 :
        case   19 :
        case   20 : return "ER AAC";
        case   21 : return "ER TwinVQ";
        case   22 : return "ER BSAC";
        case   23 : return "ER AAC LD";
        case   24 : return "ER CELP";
        case   25 : return "ER HVXC";
        case   26 : return "ER HILN";
        case   27 : return "ER Parametric";
        case   28 : return "SSC";
        case   29 : return "ParametricStereo";
        case   32 : return "Layer-1";
        case   33 : return "Layer-2";
        case   34 : return "Layer-3";
        case   35 : return "DST";
        case   36 : return "ALS";
        case   37 : return "SLS";
        case   38 : return "SLS non-core";
        case   39 : return "ER AAC ELD";
        case   40 : return "SMR Simple";
        case   41 : return "SMR Main";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* MP4_Format_Profile(int8u ID)
{
    switch (ID)
    {
        case    1 : return "Main";
        case    2 : return "LC";
        case    3 : return "SSR";
        case    4 : return "LTP";
        case   17 : return "LC";
        case   19 : return "LTP";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* MP4_Profile(int8u ID)
{
    switch (ID)
    {
        case    1 : return "A_AAC/MPEG4/MAIN";
        case    2 : return "A_AAC/MPEG4/LC";
        case    3 : return "A_AAC/MPEG4/SSR";
        case    4 : return "A_AAC/MPEG4/LTP";
        case    5 : return "SBR";
        case    6 : return "AAC Scalable";
        case    7 : return "TwinVQ";
        case    8 : return "CELP";
        case    9 : return "HVXC";
        case   12 : return "TTSI";
        case   13 : return "Main synthetic";
        case   14 : return "Wavetable synthesis";
        case   15 : return "General MIDI";
        case   16 : return "Algorithmic Synthesis and Audio FX";
        case   17 : return "ER AAC LC";
        case   19 : return "ER AAC LTP";
        case   20 : return "ER AAC Scalable";
        case   21 : return "ER TwinVQ";
        case   22 : return "ER BSAC";
        case   23 : return "ER AAC LD";
        case   24 : return "ER CELP";
        case   25 : return "ER HVXC";
        case   26 : return "ER HILN";
        case   27 : return "ER Parametric";
        case   28 : return "SSC";
        case   29 : return "PS";
        case   31 : return "(escape)";
        case   32 : return "Layer-1";
        case   33 : return "Layer-2";
        case   34 : return "Layer-3";
        case   35 : return "DST";
        case   36 : return "ALS";
        case   37 : return "SLS";
        case   38 : return "SLS non-core";
        case   39 : return "ER AAC ELD";
        case   40 : return "SMR Simple";
        case   41 : return "SMR Main";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const int8u MP4_Channels[]=
{
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    8,
};

//---------------------------------------------------------------------------
const char* MP4_ChannelConfiguration[]=
{
    "",
    "Front: C",
    "Front: L R",
    "Front: L C R",
    "Front: L C R, Side: C",
    "Front: L C R, Side: L R",
    "Front: L C R, Side: L R, LFE",
    "Front: L C R, Side: L R, Back: L R, LFE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* MP4_ChannelConfiguration2[]=
{
    "",
    "1/0/0",
    "2/0/0",
    "3/0/0",
    "3/1/0",
    "3/2/0",
    "3/2/0.1",
    "3/2/2.1",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpeg4_AudioSpecificConfig::File_Mpeg4_AudioSpecificConfig()
:File__Analyze()
{
    //In
    Channels_AreTrustable=false;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
// AAC in ES, 2+ bytes
void File_Mpeg4_AudioSpecificConfig::Read_Buffer_Continue()
{
    //Parsing
    Element_Offset=0;
    Element_Size=Buffer_Size;
    int8u samplingFrequencyIndex;
    BS_Begin();
    Get_S1 (5, audioObjectType,                                 "audioObjectType"); Param_Info(MP4_Profile(audioObjectType));
    if (audioObjectType==31)
    {
        int8u audioObjectTypeExt;
        Get_S1 (6, audioObjectTypeExt,                          "audioObjectTypeExt");
        audioObjectType=32+audioObjectTypeExt; Param_Info(audioObjectType); Param_Info(MP4_Profile(audioObjectType));
    }
    if (audioObjectType==36)
    {
        ALS();
        return;
    }

    Get_S1 (4, samplingFrequencyIndex,                          "samplingFrequencyIndex"); Param_Info(MP4_SamplingRate[samplingFrequencyIndex]);
    if (samplingFrequencyIndex>=0xF)
    {
        Get_S3 (24, samplingFrequency,                          "samplingFrequency");
    }
    else
        samplingFrequency=MP4_SamplingRate[samplingFrequencyIndex];
    Get_S1 (4, channelConfiguration,                            "channelConfiguration"); Param_Info(MP4_ChannelConfiguration[channelConfiguration]);

    sbrPresentFlag=false;
    psPresentFlag=false;
    if (audioObjectType==0x05 || audioObjectType==0x29)
    {
        extensionAudioObjectType=audioObjectType;
        sbrPresentFlag=true;
        if (audioObjectType==0x29)
            psPresentFlag=false;
        Get_S1 (4, samplingFrequencyIndex,                      "extensionSamplingFrequencyIndex"); Param_Info(MP4_SamplingRate[samplingFrequencyIndex]);
        if (samplingFrequencyIndex==0xF)
        {
            Get_S3 (24, samplingFrequency,                      "extensionSamplingFrequency");
        }
        else
            samplingFrequency=MP4_SamplingRate[samplingFrequencyIndex];
        Get_S1 (5, audioObjectType,                             "audioObjectType"); Param_Info(MP4_Profile(audioObjectType));
        if (audioObjectType==31)
        {
            int8u audioObjectTypeExt;
            Get_S1 (6, audioObjectTypeExt,                      "audioObjectTypeExt");
            audioObjectType=32+audioObjectTypeExt; Param_Info(MP4_Profile(audioObjectType));
        }
        if (audioObjectType==22) //BSAC
            Skip_S1(4,                                          "extensionChannelConfiguration");
    }
    else
        extensionAudioObjectType=0x00;

    switch(audioObjectType)
    {
        case  1:
        case  2:
        case  3:
        case  4:
        case  6:
        case  7:
        case 17:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23: GASpecificConfig();  break;
        case 8:
        //CelpSpecificConfig();
        break;
        case 9:
        //HvxcSpecificConfig();
        break;
        case 12:
        //TTSSpecificConfig();
        break;
        case 13:
        case 14:
        case 15:
        case 16:
        //StructuredAudioSpecificConfig();
        break;
        case 24:
        //ErrorResilientCelpSpecificConfig();
        break;
        case 25:
        //ErrorResilientHvxcSpecificConfig();
        break;
        case 26:
        case 27:
        //ParametricSpecificConfig();
        break;
        case 28:
        //SSCSpecificConfig();
        break;
        case 32:
        case 33:
        case 34:
        //MPEG_1_2_SpecificConfig();
        break;
        case 35:
        //DSTSpecificConfig();
        break;
        default: ;
    }

    switch (audioObjectType)
    {
        case 17:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
            {
                int8u epConfig;
                Get_S1 (2, epConfig,                            "epConfig");
                if (epConfig==2 || epConfig==3)
                {
                    //ErrorProtectionSpecificConfig();
                }
                if (epConfig==3)
                {
                    bool directMapping;
                    Get_SB (directMapping,                      "directMapping");
                    if (directMapping)
                    {
                        //tbd
                    }
                }
            }
        default : ;
    }

    bool sbrData=false;
    if (extensionAudioObjectType!=0x05 && Data_BS_Remain()>=16)
    {
        sbrData=true;
        SBR();
    }

    BS_End();

    //Handling implicit SBR and PS
    if (!Channels_AreTrustable) //if not channel map in the container
    {
        bool Is3GP=false;
        for (size_t Pos=0; Pos<ftyps.size(); Pos++)
            if ((ftyps[Pos]&0xFFFFFF00)==0x33677000)
                Is3GP=true;
        if (!Is3GP) //If this is not a 3GP file
        {
            if (!sbrPresentFlag && samplingFrequency<=24000)
            {
                samplingFrequency*=2;
                sbrPresentFlag=true;
            }
            if ((!sbrData || sbrPresentFlag) && !psPresentFlag && channelConfiguration<=1) //1 channel
                psPresentFlag=true;
        }
    }

    FILLING_BEGIN()
        Accept("AudioSpecificConfig");

        if (Count_Get(Stream_Audio)==0) //May be done elsewhere
            Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format, MP4_Format(audioObjectType));
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Version, "Version 4");
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, MP4_Format_Profile(audioObjectType));
        if (audioObjectType==2) //LC
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_SBR, "No");
            if (Channels_AreTrustable && !psPresentFlag && channelConfiguration<=1)
                Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_PS, "No");
        }
        if (!sbrPresentFlag && !psPresentFlag)
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, MP4_Profile(audioObjectType));
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, samplingFrequency);
        if (channelConfiguration && channelConfiguration<8)
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, MP4_Channels[channelConfiguration]);
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, MP4_ChannelConfiguration[channelConfiguration]);
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions_String2, MP4_ChannelConfiguration2[channelConfiguration]);
        }

        if (sbrPresentFlag)
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings, "SBR");
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_SBR, "Yes", Unlimited, true, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_PS, "No");
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Ztring().From_Local(MP4_Profile(audioObjectType))+_T("/SBR"), true);
            Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, samplingFrequency, 10, true);
        }
        if (psPresentFlag)
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 2, 10, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings, "PS");
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_PS, "Yes", Unlimited, true, true);
            Ztring Codec=Retrieve(Stream_Audio, StreamPos_Last, Audio_Codec);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Ztring().From_Local(MP4_Profile(audioObjectType))+(sbrPresentFlag?_T("/SBR"):_T(""))+_T("/PS"), true);
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, "Front: L R", Unlimited, true, true);
        }
    }

    Finish("AudioSpecificConfig");
}

//---------------------------------------------------------------------------
// GASpecificConfig
void File_Mpeg4_AudioSpecificConfig::GASpecificConfig ()
{
    Element_Begin("GASpecificConfig");
    bool dependsOnCoreCoder, extensionFlag;
    Info_SB(   frameLengthFlag,                                 "frameLengthFlag"); Param_Info(frameLengthFlag==0?1024:960, " bytes");
    Get_SB (   dependsOnCoreCoder,                              "dependsOnCoreCoder");
    if (dependsOnCoreCoder)
        Skip_S2(14,                                             "coreCoderDelay");
    Get_SB (   extensionFlag,                                   "extensionFlag");
    if (channelConfiguration==0)
    {
        Element_Begin("Extension");
        int8u Channels=0, Channels_Front=0, Channels_Side=0, Channels_Back=0, Channels_LFE=0;
        int8u num_front_channel_elements, num_side_channel_elements, num_back_channel_elements, num_lfe_channel_elements, num_assoc_data_elements, num_valid_cc_elements, comment_field_bytes;
        Skip_S1(4,                                              "element_instance_tag");
        Skip_S1(2,                                              "object_type");
        Skip_S1(4,                                              "sampling_frequency_index"); //Not used, is often 0
        Get_S1 (4, num_front_channel_elements,                  "num_front_channel_elements");
        Get_S1 (4, num_side_channel_elements,                   "num_side_channel_elements");
        Get_S1 (4, num_back_channel_elements,                   "num_back_channel_elements");
        Get_S1 (2, num_lfe_channel_elements,                    "num_lfe_channel_elements");
        Get_S1 (3, num_assoc_data_elements,                     "num_assoc_data_elements");
        Get_S1 (4, num_valid_cc_elements,                       "num_valid_cc_elements");
        TEST_SB_SKIP(                                           "mono_mixdown_present");
            Skip_S1(4,                                          "mono_mixdown_element_number");
        TEST_SB_END();
        TEST_SB_SKIP(                                           "stereo_mixdown_present");
            Skip_S1(4,                                          "stereo_mixdown_element_number");
        TEST_SB_END();
        TEST_SB_SKIP(                                           "matrix_mixdown_idx_present");
            Skip_S1(2,                                          "matrix_mixdown_idx");
            Skip_SB(                                            "pseudo_surround_enable");
        TEST_SB_END();
        for (int8u Pos=0; Pos<num_front_channel_elements; Pos++)
        {
            Element_Begin("Front channel");
            bool front_element_is_cpe;
            Get_SB (   front_element_is_cpe,                    "front_element_is_cpe");
            Skip_S1(4,                                          "front_element_tag_select");
            if (front_element_is_cpe)
            {
                Channels_Front+=2;
                Channels+=2;
            }
            else
            {
                Channels_Front++;
                Channels++;
            }
            Element_End();
        }
        for (int8u Pos=0; Pos<num_side_channel_elements; Pos++)
        {
            Element_Begin("Side channel");
            bool side_element_is_cpe;
            Get_SB (   side_element_is_cpe,                     "side_element_is_cpe");
            Skip_S1(4,                                          "side_element_tag_select");
            if (side_element_is_cpe)
            {
                Channels_Side+=2;
                Channels+=2;
            }
            else
            {
                Channels_Side++;
                Channels++;
            }
            Element_End();
        }
        for (int8u Pos=0; Pos<num_back_channel_elements; Pos++)
        {
            Element_Begin("Back channel");
            bool back_element_is_cpe;
            Get_SB (   back_element_is_cpe,                     "back_element_is_cpe");
            Skip_S1(4,                                          "back_element_tag_select");
            if (back_element_is_cpe)
            {
                Channels_Back+=2;
                Channels+=2;
            }
            else
            {
                Channels_Back++;
                Channels++;
            }
            Element_End();
        }
        for (int8u Pos=0; Pos<num_lfe_channel_elements; Pos++)
        {
            Element_Begin("LFE");
            Skip_S1(4,                                          "lfe_element_tag_select");
            Channels_LFE++;
            Channels++;
            Element_End();
        }
        for (int8u Pos=0; Pos<num_assoc_data_elements; Pos++)
        {
            Element_Begin("assoc_data_element");
            Skip_S1(4,                                          "assoc_data_element_tag_select");
            Element_End();
        }
        for (int8u Pos=0; Pos<num_valid_cc_elements; Pos++)
        {
            Element_Begin("valid_cc_element");
            Skip_SB(                                            "cc_element_is_ind_sw");
            Skip_S1(4,                                          "valid_cc_element_tag_select");
            Element_End();
        }
        BS_End(); //Byte align
        Get_B1 (comment_field_bytes,                            "comment_field_bytes");
        if (comment_field_bytes)
            Skip_XX(comment_field_bytes,                        "comment_field_data");
        BS_Begin(); //The stream needs continuity in the bitstream
        Element_End();

        //Filling
        Ztring Channels_Positions, Channels_Positions2;
        switch (Channels_Front)
        {
            case  0 : break;
            case  1 : Channels_Positions+=_T("Front: C"); break;
            case  2 : Channels_Positions+=_T("Front: L R"); break;
            case  3 : Channels_Positions+=_T("Front: L C R"); break;
            default : Channels_Positions+=_T("Front: "); Channels_Positions+=Ztring::ToZtring(Channels_Front); //Which config?
        }
        switch (Channels_Side)
        {
            case  0 : break;
            case  1 : Channels_Positions+=_T(", Side: C"); break;
            case  2 : Channels_Positions+=_T(", Side: L R"); break;
            case  3 : Channels_Positions+=_T(", Side: L C R"); break;
            default : Channels_Positions+=_T(", Side: "); Channels_Positions+=Ztring::ToZtring(Channels_Side); //Which config?
        }
        switch (Channels_Back)
        {
            case  0 : break;
            case  1 : Channels_Positions+=_T(", Back: C"); break;
            case  2 : Channels_Positions+=_T(", Back: L R"); break;
            case  3 : Channels_Positions+=_T(", Back: L C R"); break;
            default : Channels_Positions+=_T(", Back: "); Channels_Positions+=Ztring::ToZtring(Channels_Back); //Which config?
        }
        switch (Channels_LFE)
        {
            case  0 : break;
            case  1 : Channels_Positions+=_T(", LFE"); break;
            default : Channels_Positions+=_T(", LFE= "); Channels_Positions+=Ztring::ToZtring(Channels_LFE); //Which config?
        }
        Channels_Positions2=Ztring::ToZtring(Channels_Front)+_T('/')
                           +Ztring::ToZtring(Channels_Side)+_T('/')
                           +Ztring::ToZtring(Channels_Back)
                           +(Channels_LFE?_T(".1"):_T(""));

        //Filling
        Accept("AudioSpecificConfig");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels_Front+Channels_Side+Channels_Back+Channels_LFE);
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, Channels_Positions);
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions_String2, Channels_Positions2);
    }
    if (audioObjectType==06 || audioObjectType==20)
        Skip_S1(3,                                              "layerNr");
    if (extensionFlag)
    {
        bool extensionFlag3;
        if (audioObjectType==22)
        {
            Skip_S1( 5,                                         "numOfSubFrame");
            Skip_S2(11,                                         "layer_length");
        }
        if (audioObjectType==17
         || audioObjectType==19
         || audioObjectType==20
         || audioObjectType==23)
        {
            Skip_SB(                                            "aacSectionDataResilienceFlag");
            Skip_SB(                                            "aacScalefactorDataResilienceFlag");
            Skip_SB(                                            "aacSpectralDataResilienceFlag");
        }
        Get_SB (   extensionFlag3,                              "extensionFlag3");
        if (extensionFlag3)
        {
            //TODO...
        }
    }
    Element_End();
}

//---------------------------------------------------------------------------
// AAC in ES, SBR part, 3+ bytes
void File_Mpeg4_AudioSpecificConfig::SBR ()
{
    //Parsing
    int16u syncExtensionType;
    int8u  samplingFrequencyIndex;
    Element_Begin("SBR");
    Get_S2 (11, syncExtensionType,                             "syncExtensionType");
    if (syncExtensionType!=0x2B7)
    {
        Element_End();
        return;
    }
    Get_S1 (5, extensionAudioObjectType,                        "extensionAudioObjectType");
    if (extensionAudioObjectType==0x05)
    {
        Get_SB (   sbrPresentFlag,                              "sbrPresentFlag");
        if (sbrPresentFlag)
        {
            Get_S1 (4, samplingFrequencyIndex,                  "extensionSamplingFrequencyIndex"); Param_Info(MP4_SamplingRate[samplingFrequencyIndex]);
            if (samplingFrequencyIndex>=0xF)
            {
                Get_S3 (24, samplingFrequency,                  "extensionSamplingFrequency");
            }
            else
                samplingFrequency=MP4_SamplingRate[samplingFrequencyIndex];
        }
    }
    Element_End();

    //PS stuff
    if (Data_Remain())
        PS();
}

//---------------------------------------------------------------------------
// AAC in ES, PS part, 2 bytes
// Format is unknown
void File_Mpeg4_AudioSpecificConfig::PS ()
{
    //Parsing
    Element_Begin("PS");
    int16u syncExtensionType;
    Get_S2 (11, syncExtensionType,                             "syncExtensionType");
    if (syncExtensionType!=0x548)
    {
        Element_End();
        return;
    }
    Get_SB (psPresentFlag,                                     "psPresentFlag");
    Element_End();

    //BSAC stuff
    //if (extensionAudioObjectType==22)
    //    BSAC();
    //if ( extensionAudioObjectType == 22 ) {
    //    sbrPresentFlag;
    //    if (sbrPresentFlag == 1) {
    //        extensionSamplingFrequencyIndex;
    //        if ( extensionSamplingFrequencyIndex == 0xf )
    //            extensionSamplingFrequency;
    //    }
    //    extensionChannelConfiguration
    //}
}

//---------------------------------------------------------------------------
// ALS is detected
// Format is unknown
void File_Mpeg4_AudioSpecificConfig::ALS ()
{
    //Parsing
    BS_End();

    FILLING_BEGIN();
        //Filling
        Accept("AudioSpecificConfig");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format, "ALS");
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "ALS");
    FILLING_END();

    std::string Riff;
    Riff.append((const char*)Buffer+Buffer_Offset, (size_t)(Element_Size-Element_Offset));
    if (Riff.find("RIFF") && Riff.find("WAVEfmt"))
    {
        #if defined(MEDIAINFO_RIFF_YES)
            //Creating the parser
            File_Riff MI;
            Open_Buffer_Init(&MI);

            //Parsing
            size_t Riff_Pos=Riff.find("RIFF");
            Skip_XX(Riff_Pos,                                   "Unknown");
            Open_Buffer_Continue(&MI);

            //Filling
            Finish(&MI);
            Merge(MI, StreamKind_Last, 0, StreamPos_Last);

            //The RIFF header is for PCM
            Clear(Stream_Audio, StreamPos_Last, Audio_ID);
            Clear(Stream_Audio, StreamPos_Last, Audio_Codec_String);
            Clear(Stream_Audio, StreamPos_Last, Audio_Codec_Family);
            Fill(Stream_Audio, StreamPos_Last, Audio_Format, "ALS", Unlimited, true, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "ALS", Unlimited, true, true);
            Clear(Stream_Audio, StreamPos_Last, Audio_CodecID);
            Clear(Stream_Audio, StreamPos_Last, Audio_CodecID_Hint);
            Clear(Stream_Audio, StreamPos_Last, Audio_CodecID_Url);
            Clear(Stream_Audio, StreamPos_Last, Audio_BitRate);
            Clear(Stream_Audio, StreamPos_Last, Audio_BitRate_Mode);
            Clear(Stream_Audio, StreamPos_Last, Audio_Codec_CC);

        #else
            Skip_XX(Element_Size-Element_Offset,                "(RIFF chunck)");
        #endif

    }
    else
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");

    //No need more
    Finish("AudioSpecificConfig");
}

} //NameSpace

#endif //MEDIAINFO_MPEG4_YES

