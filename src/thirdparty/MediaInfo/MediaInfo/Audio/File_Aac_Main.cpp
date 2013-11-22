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
#if MEDIAINFO_ADVANCED
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#endif //MEDIAINFO_ADVANCED
#include <cmath>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
extern const int32u Aac_sampling_frequency[]=
{96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
 16000, 12000, 11025,  8000,  7350,     0,     0,     0,};

//---------------------------------------------------------------------------
const char* Aac_Adts_ID[]=
{
    "MPEG-4",
    "MPEG-2",
};

//---------------------------------------------------------------------------
const char* Aac_Format(int8u ID)
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
        case   37 :
        case   38 : return "SLS";
        case   39 : return "ER AAC ELD";
        case   40 : return "SMR Simple";
        case   41 : return "SMR Main";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Aac_Format_Profile(int8u ID)
{
    switch (ID)
    {
        case    1 : return "Main";
        case    2 : return "LC";
        case    3 : return "SSR";
        case    4 : return "LTP";
        case   17 : return "LC";
        case   19 : return "LTP";
        case   37 : return "non-core";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Aac_audioObjectType(int8u audioObjectType)
{
    switch (audioObjectType)
    {
        case    1 : return "AAC Main";
        case    2 : return "AAC LC";
        case    3 : return "AAC SSR";
        case    4 : return "AAC LTP";
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
const int8u Aac_Channels[]=
{
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    8,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

//---------------------------------------------------------------------------
const char* Aac_ChannelConfiguration[]=
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
const char* Aac_ChannelConfiguration2[]=
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

int8u Aac_AudioSpecificConfig_sampling_frequency_index(const int32u sampling_frequency)
{
    if (sampling_frequency>=92017) return 0;
    if (sampling_frequency>=75132) return 1;
    if (sampling_frequency>=55426) return 2;
    if (sampling_frequency>=46009) return 3;
    if (sampling_frequency>=37566) return 4;
    if (sampling_frequency>=27713) return 5;
    if (sampling_frequency>=23004) return 6;
    if (sampling_frequency>=18783) return 7;
    if (sampling_frequency>=13856) return 8;
    if (sampling_frequency>=11502) return 9;
    if (sampling_frequency>=9391) return 10;
    return 11;
}

//---------------------------------------------------------------------------
void File_Aac::AudioSpecificConfig (size_t End)
{
    //Parsing
    bool    sbrData=false, sbrPresentFlag=false, psData=false, psPresentFlag=false;
    Element_Begin1("AudioSpecificConfig");
    GetAudioObjectType(audioObjectType,                         "audioObjectType");
    Get_S1 (4, sampling_frequency_index,                        "samplingFrequencyIndex"); Param_Info1(Aac_sampling_frequency[sampling_frequency_index]);
    if (sampling_frequency_index==0xF)
    {
        Get_S3 (24, sampling_frequency,                         "samplingFrequency");
        sampling_frequency_index=Aac_AudioSpecificConfig_sampling_frequency_index(sampling_frequency);
    }
    else
        sampling_frequency=Aac_sampling_frequency[sampling_frequency_index];
    Get_S1 (4, channelConfiguration,                            "channelConfiguration"); Param_Info1(Aac_ChannelConfiguration[channelConfiguration]);
    if (audioObjectType==0x05 || audioObjectType==0x29)
    {
        extensionAudioObjectType=5;
        sbrPresentFlag=true;
        if (audioObjectType==29)
            psPresentFlag=true;
        Get_S1 (4, extension_sampling_frequency_index,          "extensionSamplingFrequencyIndex"); Param_Info1(Aac_sampling_frequency[extension_sampling_frequency_index]);
        if (extension_sampling_frequency_index==0xF)
        {
            Get_S3 (24, extension_sampling_frequency,           "extensionSamplingFrequency");
            sampling_frequency_index=Aac_AudioSpecificConfig_sampling_frequency_index(extension_sampling_frequency);
        }
        else
            extension_sampling_frequency=Aac_sampling_frequency[extension_sampling_frequency_index];
        GetAudioObjectType(audioObjectType,                     "audioObjectType");
        if (audioObjectType==22) //BSAC
            Skip_S1(4,                                          "extensionChannelConfiguration");
    }
    else
        extensionAudioObjectType=0x00;

    switch (audioObjectType)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 17:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            GASpecificConfig();
            break;
        case 8:
            CelpSpecificConfig();
            break;
        case 9:
            HvxcSpecificConfig();
            break;
        case 12:
            TTSSpecificConfig();
            break;
        //~ case 13:
        //~ case 14:
        //~ case 15:
        //~ case 16:
            //~ StructuredAudioSpecificConfig();
            //~ break;
        case 24:
            ErrorResilientCelpSpecificConfig();
            break;
        case 25:
            ErrorResilientHvxcSpecificConfig();
            break;
        case 26:
        case 27:
            ParametricSpecificConfig();
            break;
        case 28:
            SSCSpecificConfig();
            break;
        //~ case 30:
            //~ Skip_S1(1,                                          "sacPayloadEmbedding");
            //~ SpatialSpecificConfig(); //ISO/IEC 23003-1
            //~ break;
        case 32:
        case 33:
        case 34:
            MPEG_1_2_SpecificConfig();
            break;
        case 35:
            DSTSpecificConfig();
            break;
        case 36:
            Skip_S1(5,                                          "fillBits");
            ALSSpecificConfig();
            break;
        case 37:
        case 38:
            SLSSpecificConfig();
            break;
        case 39:
            ELDSpecificConfig();
            break;
        //~ case 40:
        //~ case 41:
            //~ SymbolicMusicSpecificConfig(); //ISO/IEC 14496-23
            //~ break;
        default:
            Element_Begin1("not implemented part");
            Skip_BS(Data_BS_Remain()-End,                       "(Not implemented)");
            Element_End0();
            if (Mode==File_Aac::Mode_ADIF || Mode==File_Aac::Mode_ADTS)
                File__Tags_Helper::Finish();
            else
                File__Analyze::Finish();
            Frame_Count=(size_t)-1; //Forcing not to parse following data anymore (if ParseSpeed==1)
            return;
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
        case 39:
            int8u epConfig;
            Get_S1(2,epConfig,                                  "epConfig");
            if ( epConfig == 2 || epConfig == 3 )
                ErrorProtectionSpecificConfig();
            if ( epConfig == 3 )
            {
                bool directMapping;
                Get_SB(directMapping,                           "directMapping");
                if ( ! directMapping )
                {
                    Element_Begin1("not implemented part");
                    Skip_BS(Data_BS_Remain()-End,               "(Not implemented)");
                    Element_End0();
                    if (Mode==File_Aac::Mode_ADIF || Mode==File_Aac::Mode_ADTS)
                        File__Tags_Helper::Finish();
                    else
                        File__Analyze::Finish();
                    Frame_Count=(size_t)-1; //Forcing not to parse following data anymore (if ParseSpeed==1)
                    return;
                }
            }
        default : ;
    }

    if (extensionAudioObjectType!=5 && End!=(size_t)-1 && Data_BS_Remain()>=End+16)
    {
        int16u syncExtensionType;
        Get_S2(11,syncExtensionType,                            "syncExtensionType");
        if (syncExtensionType == 0x2b7)
        {
            sbrData=true;
            GetAudioObjectType(extensionAudioObjectType,        "extensionAudioObjectType");
            if (extensionAudioObjectType==5 )
            {
                Get_SB(sbrPresentFlag,                          "sbrPresentFlag");
                if (sbrPresentFlag)
                {
                    Get_S1 (4, extension_sampling_frequency_index, "extensionSamplingFrequencyIndex"); Param_Info1(Aac_sampling_frequency[extension_sampling_frequency_index]);
                    if (extension_sampling_frequency_index==0xF)
                    {
                        Get_S3 (24, extension_sampling_frequency, "extensionSamplingFrequency");
                        extension_sampling_frequency_index=Aac_AudioSpecificConfig_sampling_frequency_index(extension_sampling_frequency);
                    }
                    else
                        extension_sampling_frequency=Aac_sampling_frequency[extension_sampling_frequency_index];
                    if (End!=(size_t)-1 && Data_BS_Remain()>=End+12)
                    {
                        int16u syncExtensionType;
                        Get_S2(11,syncExtensionType,            "syncExtensionType");
                        if (syncExtensionType == 0x548)
                        {
                            psData=true;
                            Get_SB (psPresentFlag,              "psPresentFlag");
                        }
                    }
                }
            }
            if ( extensionAudioObjectType == 22 )
            {
                Get_SB(sbrPresentFlag,                          "sbrPresentFlag");
                if (sbrPresentFlag)
                {
                    Get_S1 (4, extension_sampling_frequency_index, "extensionSamplingFrequencyIndex"); Param_Info1(Aac_sampling_frequency[extension_sampling_frequency_index]);
                    if (extension_sampling_frequency_index==0xF)
                    {
                        Get_S3 (24, extension_sampling_frequency, "extensionSamplingFrequency");
                        extension_sampling_frequency_index=Aac_AudioSpecificConfig_sampling_frequency_index(extension_sampling_frequency);
                    }
                    else
                        extension_sampling_frequency=Aac_sampling_frequency[extension_sampling_frequency_index];
                }
                Skip_S1(4,                                      "extensionChannelConfiguration");
            }
        }
    }
    Element_End0();
    if (Data_BS_Remain()>End)
    {
        int8u LastByte=0xFF;
        if (Data_BS_Remain()-End<8)
            Peek_S1((int8u)(Data_BS_Remain()-End), LastByte);
        Skip_BS(Data_BS_Remain()-End,                           LastByte?"Unknown":"Padding");
    }

    FILLING_BEGIN();
        AudioSpecificConfig_OutOfBand (sampling_frequency, audioObjectType, sbrData, psData, sbrPresentFlag, psPresentFlag);

        //Parsing the rest
        if (audioObjectType!=2) //We continue only if AAC LC (in order to detect SBR and PS)
        {
            File__Analyze::Finish();
            Frame_Count=(size_t)-1; //Forcing not to parse following data anymore (if ParseSpeed==1)
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Aac::AudioSpecificConfig_OutOfBand (int32u sampling_frequency_, int8u audioObjectType_, bool sbrData, bool psData, bool sbrPresentFlag, bool psPresentFlag)
{
    if (sampling_frequency==(int32u)-1 && sampling_frequency_) //Only if not yet set
    {
        sampling_frequency=sampling_frequency_;
        sampling_frequency_index=Aac_AudioSpecificConfig_sampling_frequency_index(sampling_frequency);
    }

    if (audioObjectType_==(int8u)-1)
    {
        if (audioObjectType==(int8u)-1)
            return; //All data is not yet available
        if (Infos.find("Format_Settings_SBR")!=Infos.end())
        {
            sbrData=true;
            sbrPresentFlag=Infos["Format_Settings_SBR"].find(__T("Yes"))!=string::npos;
        }
        else
        {
            sbrData=false;
            sbrPresentFlag=false;
        }
        if (Infos.find("Format_Settings_PS")!=Infos.end())
        {
            psData=true;
            psPresentFlag=Infos["Format_Settings_PS"].find(__T("Yes"))!=string::npos;
        }
        else
        {
            psData=false;
            psPresentFlag=false;
        }
    }
    else
        audioObjectType=audioObjectType_;

    Infos["SamplingRate"].From_Number(sampling_frequency);
    Infos["Format"].From_Local(Aac_Format(audioObjectType));
    Infos["Format_Profile"].From_Local(Aac_Format_Profile(audioObjectType));
    Infos["Codec"].From_Local(Aac_audioObjectType(audioObjectType));
    if (channelConfiguration && channelConfiguration<8)
    {
        Infos["Channel(s)"].From_Number(Aac_Channels[channelConfiguration]);
        Infos["ChannelPositions"].From_Local(Aac_ChannelConfiguration[channelConfiguration]);
        Infos["ChannelPositions/String2"].From_Local(Aac_ChannelConfiguration2[channelConfiguration]);
    }

    if (sbrPresentFlag || !Infos["Format_Settings_SBR"].empty())
    {
        Infos["Format_Profile"]=__T("HE-AAC");
        Ztring SamplingRate=Infos["SamplingRate"];
        Infos["SamplingRate"].From_Number((extension_sampling_frequency_index==(int8u)-1)?(sampling_frequency*2):extension_sampling_frequency, 10);
        if (MediaInfoLib::Config.LegacyStreamDisplay_Get())
        {
            Infos["Format_Profile"]+=__T(" / LC");
            Infos["SamplingRate"]+=__T(" / ")+SamplingRate;
        }
        Infos["Format_Settings_SBR"]=__T("Yes (Implicit)");
        Infos["Codec"]=Ztring().From_Local(Aac_audioObjectType(audioObjectType))+__T("-SBR");
    }
    else if (sbrData)
        Infos["Format_Settings_SBR"]=__T("No (Explicit)");

    if (psPresentFlag || !Infos["Format_Settings_PS"].empty())
    {
        Infos["Format_Profile"]=__T("HE-AACv2");
        Ztring Channels=Infos["Channel(s)"];
        Ztring ChannelPositions=Infos["ChannelPositions"];
        Ztring SamplingRate=Infos["SamplingRate"];
        Infos["Channel(s)"]=__T("2");
        Infos["ChannelPositions"]=__T("Front: L R");
        if (MediaInfoLib::Config.LegacyStreamDisplay_Get())
        {
            Infos["Format_Profile"]+=__T(" / HE-AAC / LC");
            Infos["Channel(s)"]+=__T(" / ")+Channels+__T(" / ")+Channels;
            Infos["ChannelPositions"]+=__T(" / ")+ChannelPositions+__T(" / ")+ChannelPositions;
            Infos["SamplingRate"]=Ztring().From_Number((extension_sampling_frequency_index==(int8u)-1)?(sampling_frequency*2):extension_sampling_frequency, 10)+__T(" / ")+SamplingRate;
        }
        Infos["Format_Settings_PS"]=__T("Yes (Implicit)");
        if (StreamPos_Last!=(size_t)-1)
        {
            Ztring Codec=Retrieve(Stream_Audio, StreamPos_Last, Audio_Codec);
            Infos["Codec"]=Ztring().From_Local(Aac_audioObjectType(audioObjectType))+__T("-SBR-PS");
        }
    }
    else if (psData)
        Infos["Format_Settings_PS"]=__T("No (Explicit)");
}

//---------------------------------------------------------------------------
void File_Aac::GetAudioObjectType(int8u &ObjectType, const char* Name)
{
    Element_Begin1(Name);
    Get_S1(5, ObjectType,                                       "audioObjectType");
    if (ObjectType==31)
    {
        Get_S1(6, ObjectType,                                   "audioObjectTypeExt");
        ObjectType+=32;
    }
    Element_Info1(ObjectType); Element_Info1(Aac_Format_Profile(ObjectType));
    Element_End0();
}

//***************************************************************************
// Elements - Multiplex layer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::AudioMuxElement()
{
    Element_Begin1("AudioMuxElement");
    if (muxConfigPresent)
    {
        bool useSameStreamMux;
        Get_SB (useSameStreamMux,                               "useSameStreamMux");
        if (!useSameStreamMux)
            StreamMuxConfig();
    }
    if (sampling_frequency_index==(int8u)-1) //No previous config
    {
        CanFill=false;
        Skip_BS(Data_BS_Remain(),                               "(Waiting for configuration)");
        return;
    }
    if (audioMuxVersionA==0)
    {
        for (int8u i=0; i<=numSubFrames; i++)
        {
            PayloadLengthInfo();
            PayloadMux();
        }
        if (otherDataPresent)
        {
            for(int32u i=0; i<otherDataLenBits; i++)
                Skip_SB(                                        "otherDataBit");
        }
    }
    else
    {
        Element_Begin1("(not implemented)");
        Skip_BS(Data_BS_Remain(),                               "(not implemented)");
        Element_End0();
    }
    if (Data_BS_Remain()%8)
        Skip_S1(Data_BS_Remain()%8,                             "byte_alignment");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::StreamMuxConfig()
{
    Element_Begin1("StreamMuxConfig");

    bool audioMuxVersion;
    Get_SB (audioMuxVersion,                                    "audioMuxVersion");
    if (audioMuxVersion)
        Get_SB (audioMuxVersionA,                               "audioMuxVersionA");
    else
        audioMuxVersionA=false;

    if (!audioMuxVersionA)
    {
        if (audioMuxVersion==1)
        {
            Element_Begin1("(not implemented)");
            Skip_BS(Data_BS_Remain(),                               "(not implemented)");
            Element_End0();
            //taraBufferFullness=LatmGetValue();
        }

        int8u streamCnt = 0;
        bool useSameConfig;

        Get_SB (allStreamsSameTimeFraming,                      "allStreamsSameTimeFraming");
        Get_S1 (6, numSubFrames,                                "numSubFrames");
        Get_S1 (4, numProgram,                                  "numProgram");

        for (int8u prog=0; prog<=numProgram; prog++)
        {
            Get_S1(3,numLayer,                                  "numLayer");
            for (int8u lay=0; lay<=numLayer; lay++)
            {
                progSIndx[streamCnt]=prog;
                laySIndx[streamCnt]=lay;
                streamID[prog][lay]=streamCnt++;
                if (prog==0 && lay==0)
                    useSameConfig=0;
                else
                    Get_SB(useSameConfig,                       "useSameConfig");

                if (!useSameConfig)
                {
                    if (audioMuxVersion==0)
                        AudioSpecificConfig();
                    else
                    {
                        int ascLen=LatmGetValue();
                        AudioSpecificConfig(Data_BS_Remain()-ascLen);
                    }
                }

                Get_S1(3,frameLengthType[streamID[prog][lay]],  "frameLengthType[streamID[prog][lay]]");
                switch(frameLengthType[streamID[prog][lay]])
                {
                    case 0 :
                            Skip_S1(8,                          "latmBufferFullness[streamID[prog][lay]]");
                            if(!allStreamsSameTimeFraming)
                            {
                                Element_Begin1("(not implemented)");
                                Skip_BS(Data_BS_Remain(),       "(not implemented)");
                                Element_End0();
                            }
                            //~ if ((!allStreamsSameTimeFraming) &&
                            //~ (AudioObjectType[lay] == 6 || AudioObjectType[lay] == 20) &&
                            //~ (AudioObjectType[lay-1] == 8 || AudioObjectType[lay-1] == 24))
                            //~ {
                                //~ Skip_S1(6,                          "coreFrameOffset");
                            //~ }
                            break;
                    case 1 :
                            Get_S2(9,frameLength[streamID[prog][lay]],"frameLength[streamID[prog][lay]]");
                            break;
                    case 3 :
                    case 4 :
                    case 5 :
                            Skip_S1(6,                          "CELPframeLengthTableIndex[streamID[prog][lay]]");
                            break;
                    case 6 :
                    case 7 :
                            Skip_S1(1,                          "HVXCframeLengthTableIndex[streamID[prog][lay]]");
                            break;
                    default :
                            Element_Begin1("(not implemented)");
                            Skip_BS(Data_BS_Remain(),       "(not implemented)");
                            Element_End0();
                }
            }
        }
        bool crcCheckPresent;
        Get_SB (otherDataPresent,                               "otherDataPresent");
        if (otherDataPresent)
        {
            if (audioMuxVersion==1)
                otherDataLenBits=LatmGetValue();
            else
            {
                otherDataLenBits=0;
                bool otherDataLenEsc;
                do
                {
                    int8u otherDataLenTmp;
                    otherDataLenBits<<=8;
                    Get_SB(   otherDataLenEsc,                   "otherDataLenEsc");
                    Get_S1(8, otherDataLenTmp,                   "otherDataLenTmp");
                    otherDataLenBits+=otherDataLenTmp;
                }
                while (otherDataLenEsc);
            }
        }
        Get_SB(crcCheckPresent,                                 "crcCheckPresent");
        if(crcCheckPresent)
            Skip_S1(8,                                          "crcCheckSum");
    }
    else
    {
        Element_Begin1("(not implemented)");
        Skip_BS(Data_BS_Remain(),                               "(not implemented)");
        Element_End0();
    }
    Element_End0();

    FILLING_BEGIN();
        CanFill=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
int32u File_Aac::LatmGetValue()
{
    Element_Begin1("LatmGetValue");
    int8u valueTmp, bytesForValue;
    Get_S1(2, bytesForValue,                                    "bytesForValue");
    int32u value=0;
    for (int8u i=0; i<=bytesForValue; i++)
    {
        value<<=8;
        Get_S1(8, valueTmp,                                     "valueTmp");
        value+=valueTmp;
    }
    Element_End0();
    return value;
}

//---------------------------------------------------------------------------
void File_Aac::PayloadLengthInfo()
{
    Element_Begin1("PayloadLengthInfo");
    int8u tmp;
    if (allStreamsSameTimeFraming)
    {
        for (int8u prog=0; prog<=numProgram; prog++)
        {
            for (int8u lay=0; lay<=numLayer; lay++)
            {
                if (frameLengthType[streamID[prog][lay]]==0)
                {
                    MuxSlotLengthBytes[streamID[prog][lay]]=0;
                    do
                    {
                        Get_S1(8, tmp,                          "tmp");
                        MuxSlotLengthBytes[streamID[prog][lay]]+=tmp;
                    }
                    while(tmp==255);
                }
                else if (frameLengthType[streamID[prog][lay]]==3
                      || frameLengthType[streamID[prog][lay]]==5
                      || frameLengthType[streamID[prog][lay]]==7)
                {
                    Skip_S1(2,                                  "MuxSlotLengthCoded[streamID[prog][lay]]");
                }
            }
        }
    }
    else
    {
        int8u streamIndx;
        int8u prog, lay;
        Get_S1(4, numChunk,                                     "numChunk");
        for (int chunkCnt=0; chunkCnt<=numChunk; chunkCnt++)
        {
            Get_S1(4,streamIndx,                                "streamIndx");
            prog=progCIndx[chunkCnt]=progSIndx[streamIndx];
            lay=layCIndx[chunkCnt]=laySIndx[streamIndx];
            if (frameLengthType[streamID[prog][lay]]==0)
            {
                MuxSlotLengthBytes[streamID[prog][lay]]=0;
                do
                {
                    Get_S1(8, tmp,                              "tmp");
                    MuxSlotLengthBytes[streamID[prog][lay]]+=tmp;
                }
                while(tmp==255);
                Skip_SB(                                        "AuEndFlag[streamID[prog][lay]]");
            }
            else if (frameLengthType[streamID[prog][lay]]==3
                  || frameLengthType[streamID[prog][lay]]==5
                  || frameLengthType[streamID[prog][lay]]==7)
            {
                Skip_S1(2,                                      "MuxSlotLengthCoded[streamID[prog][lay]]");
            }
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::PayloadMux()
{
    Element_Begin1("PayloadMux");
    if (allStreamsSameTimeFraming)
    {
        for (int8u prog=0; prog<=numProgram; prog++)
            for (int8u lay=0; lay<=numLayer; lay++)
            {
                switch(frameLengthType[streamID[prog][lay]])
                {
                    case 0 :
                            if (CA_system_ID_MustSkipSlices)
                            {
                                //Encryption management
                                Skip_BS(8*MuxSlotLengthBytes[streamID[prog][lay]], "Encrypted payload[streamID[prog][lay]]");
                                Frame_Count_Valid=0;
                            }
                            else
                                raw_data_block();
                            break;
                    case 1 :
                            Skip_BS(8 * (frameLength[streamID[prog][lay]] + 20),"payload[streamID[prog][lay]]");
                            break;
                    default:
                            Element_Begin1("(not implemented)");
                            Skip_BS(Data_BS_Remain(),           "(not implemented)");
                            Element_End0();
                }
            }
    }
    else
    {
        for (int8u chunkCnt=0; chunkCnt<=numChunk; chunkCnt++)
        {
            int8u prog=progCIndx[chunkCnt];
            int8u lay=layCIndx[chunkCnt];
            switch(frameLengthType[streamID[prog][lay]])
            {
                case 0 :
                        raw_data_block(); //Skip_BS(MuxSlotLengthBytes[streamID[prog][lay]], "payload[streamID[prog][lay]]");
                        break;
                case 1 :
                        Skip_BS(8*(frameLength[streamID[prog][lay]]+20), "payload[streamID[prog][lay]]");
                        break;
                default:
                        Element_Begin1("not implemented");
                        Element_End0();
            }
        }
    }
    Element_End0();
}

//***************************************************************************
// Elements - Error protection
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::ErrorProtectionSpecificConfig()
{
    Element_Begin1("ErrorProtectionSpecificConfig");
    int8u number_of_predefined_set,number_of_concatenated_frame,interleave_type;
    Get_S1(8,number_of_predefined_set,                          "number_of_predefined_set");
    Get_S1(2,interleave_type,                                   "interleave_type");
    Skip_S1(3,                                                  "bit_stuffing");
    Get_S1(3,number_of_concatenated_frame,                      "number_of_concatenated_frame");
    for (int8u i = 0; i < number_of_predefined_set; i++ )
    {
        int8u number_of_class;
        Get_S1(6,number_of_class,                               "number_of_class[i]");
        for (int8u j = 0; j < number_of_class; j++)
        {
            bool length_escape,fec_type,rate_escape,crclen_escape;
            Get_SB(length_escape,                               "length_escape[i][j]");
            Get_SB(rate_escape,                                 "rate_escape[i][j]");
            Get_SB(crclen_escape,                               "crclen_escape[i][j]");
            if (number_of_concatenated_frame != 1)
                Skip_SB(                                        "concatenate_flag[i][j]");
            Get_SB(fec_type,                                    "fec_type[i][j]");
            if(!fec_type)
                Skip_SB(                                        "termination_switch[i][j]");
            if (interleave_type == 2)
                Skip_S1(2,                                      "interleave_switch[i][j]");
            Skip_SB(                                            "class_optional");
            if (length_escape)
            {
                /* ESC */
                Skip_S1(4,                                      "number_of_bits_for_length[i][j]");
            }
            else
            {
                Skip_S2(16,                                     "class_length[i][j]");
            }
            if (!rate_escape)
            { /* not ESC */
                if(fec_type)
                {
                    Skip_S1(7,                                  "class_rate[i][j]");
                }
                else
                {
                    Skip_S1(5,                                  "class_rate[i][j]");
                }
            }
            if (!crclen_escape)
            {
            /* not ESC */
            Skip_S1(5,                                          "class_crclen[i][j]");
            }
        }
        bool class_reordered_output;
        Get_SB(class_reordered_output,                          "class_reordered_output");
        if ( class_reordered_output )
        {
            for (int j = 0; j < number_of_class; j++ )
            {
                Skip_S1(6,                                      "class_output_order[i][j]");
            }
        }
    }
    bool header_protection;
    Get_SB(header_protection,                                   "header_protection");
    if (header_protection)
    {
        Skip_S1(5,                                              "header_rate");
        Skip_S1(5,                                              "header_crclen");
    }
    Element_End0();
}

//***************************************************************************
// Elements - MPEG-2 AAC Audio_Data_Interchange_Format, ADIF
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::adif_header()
{
    //Parsing
    int32u bitrate;
    int8u  num_program_config_elements;
    bool   bitstream_type;
    Skip_C4(                                                    "adif_id");
    BS_Begin();
    TEST_SB_SKIP(                                               "copyright_id_present");
        Skip_S4(32,                                             "copyright_id");
        Skip_S4(32,                                             "copyright_id");
        Skip_S4( 8,                                             "copyright_id");
    TEST_SB_END();
    Skip_SB(                                                    "original_copy");
    Skip_SB(                                                    "home");
    Get_SB (    bitstream_type,                                 "bitstream_type"); Param_Info1(bitstream_type?"VBR":"CBR");
    Get_S3 (23, bitrate,                                        "bitrate");
    Get_S1 ( 4, num_program_config_elements,                    "num_program_config_elements");
    if (!bitstream_type)
        Skip_S3(20,                                             "adif_buffer_fullness");
    for (int8u Pos=0; Pos<num_program_config_elements+1; Pos++)
        program_config_element();
    BS_End();

    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Format, "ADIF", Unlimited, true, true);
        Fill(Stream_General, 0, General_HeaderSize, Element_Size);
        Fill(Stream_General, 0, General_OverallBitRate_Mode, bitstream_type?"VBR":"CBR");

        for (size_t StreamPos=0; StreamPos<Count_Get(Stream_Audio); StreamPos++)
            Fill(Stream_Audio, StreamPos, Audio_MuxingMode, "ADIF");
        if (num_program_config_elements==0) //Easy to fill only if 1 audio stream
        {
            Infos["BitRate_Mode"].From_Local(bitstream_type?"VBR":"CBR");
            if (bitrate>0)
                Infos[bitstream_type?"BitRate_Maximum":"BitRate"].From_Number(bitrate);
        }

        //No more need data
        File__Tags_Helper::Finish("ADIF");
    FILLING_END();
}

//***************************************************************************
// Elements - Audio_Data_Transport_Stream frame, ADTS
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::adts_frame()
{
    //Parsing
    adts_fixed_header();
    adts_variable_header();

    //Encryption management
    if (CA_system_ID_MustSkipSlices)
    {
        //Is not decodable
        BS_End();
        Skip_XX(Element_Size-Element_Offset,                    "Encrypted data");
        Frame_Count_Valid=0;
        return;
    }

    if (num_raw_data_blocks==0)
    {
        if (!protection_absent)
        {
            Element_Begin1("adts_error_check");
                Skip_S2(16,                                     "crc_check");
            Element_End0();
        }
        raw_data_block();
    }
    else
    {
        Element_Begin1("adts_header_error_check");
            if (!protection_absent)
                for (int i=1; i<=num_raw_data_blocks; i++)
                    Skip_S2(16,                                 "raw_data_block_position(i)");
            Skip_S2(16,                                         "crc_check");
        Element_End0();

        for(int i=0; i<=num_raw_data_blocks; i++)
        {
            raw_data_block();
            if (!protection_absent)
            {
                Element_Begin1("adts_raw_data_block_error_check");
                    Skip_BS(16,                                 "crc_check");
                Element_End0();
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Aac::adts_fixed_header()
{
    //Parsing
    bool    id;
    Element_Begin1("adts_fixed_header");
    Skip_BS(12,                                                 "syncword");
    Get_SB (    id,                                             "id"); Param_Info1(Aac_Adts_ID[id]);
    Skip_BS( 2,                                                 "layer");
    Get_SB (    protection_absent,                              "protection_absent");
    Get_S1 ( 2, audioObjectType,                                "profile_ObjectType"); audioObjectType++; Param_Info1(Aac_audioObjectType(audioObjectType));
    Get_S1 ( 4, sampling_frequency_index,                       "sampling_frequency_index"); Param_Info2(Aac_sampling_frequency[sampling_frequency_index], " Hz");
    sampling_frequency=Aac_sampling_frequency[sampling_frequency_index];
    Skip_SB(                                                    "private");
    Get_S1 ( 3, channelConfiguration,                           "channel_configuration");
    Skip_SB(                                                    "original");
    Skip_SB(                                                    "home");
    Element_End0();

    FILLING_BEGIN();
        if (Infos["Format"].empty())
        {
            Infos_General["Format"].From_Local("ADTS");

            Infos["Format"].From_Local("AAC");
            Infos["Format_Version"].From_Local(id?"Version 2":"Version 4");
            Infos["Format_Profile"].From_Local(Aac_Format_Profile(audioObjectType));
            Infos["Codec"].From_Local(Aac_audioObjectType(audioObjectType));
            Infos["SamplingRate"].From_Number(Aac_sampling_frequency[sampling_frequency_index]);
            Infos["Channel(s)"].From_Number(channelConfiguration);
            Infos["ChannelPositions"].From_Local(Aac_ChannelConfiguration[channelConfiguration]);
            Infos["ChannelPositions/String2"].From_Local(Aac_ChannelConfiguration2[channelConfiguration]);
            if (IsSub)
                Infos["MuxingMode"].From_Local("ADTS");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Aac::adts_variable_header()
{
    //Parsing
    int16u  aac_frame_length, adts_buffer_fullness;
    Element_Begin1("adts_variable_header");
    Skip_SB(                                                    "copyright_id");
    Skip_SB(                                                    "copyright_id_start");
    Get_S2 (13, aac_frame_length,                               "aac_frame_length");
    Get_S2 (11, adts_buffer_fullness,                           "adts_buffer_fullness"); Param_Info1(adts_buffer_fullness==0x7FF?"VBR":"CBR");
    Get_S1 ( 2, num_raw_data_blocks,                            "num_raw_data_blocks");
    Element_End0();

    FILLING_BEGIN();
        aac_frame_lengths.push_back(aac_frame_length);

        Infos["BitRate_Mode"].From_Local(adts_buffer_fullness==0x7FF?"VBR":"CBR");
        if (adts_buffer_fullness!=0x7FF
        #if MEDIAINFO_ADVANCED
         || Config->File_RiskyBitRateEstimation_Get()
        #endif //MEDIAINFO_ADVANCED
        )
        {
            //Calculating
            int64u aac_frame_length_Total=0;
            for (size_t Pos=0; Pos<aac_frame_lengths.size(); Pos++)
                aac_frame_length_Total+=aac_frame_lengths[Pos];

            int64u BitRate=(Aac_sampling_frequency[sampling_frequency_index]/1024);
            BitRate*=aac_frame_length_Total*8;
            BitRate/=aac_frame_lengths.size();

            //Filling
            Infos["BitRate"].From_Number(BitRate);
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AAC_YES

