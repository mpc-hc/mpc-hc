/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Descriptors part
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#ifdef MEDIAINFO_MPEG4_YES
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4_Descriptors.h"
#include <cstring>
#if defined(MEDIAINFO_OGG_YES)
    #include "MediaInfo/Multiple/File_Ogg.h"
#endif
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif
#if defined(MEDIAINFO_DIRAC_YES)
    #include "MediaInfo/Video/File_Dirac.h"
#endif
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#if defined(MEDIAINFO_PNG_YES)
    #include "MediaInfo/Image/File_Png.h"
#endif
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_DTS_YES)
    #include "MediaInfo/Audio/File_Dts.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if MEDIAINFO_DEMUX
    #include "base64.h"
#endif //MEDIAINFO_DEMUX
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_MPEG4V_YES
    const char* Mpeg4v_Profile_Level(int32u Profile_Level);
#endif //MEDIAINFO_MPEG4V_YES
//---------------------------------------------------------------------------

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_Predefined(int8u ID)
{
    switch (ID)
    {
        case 0x00 : return "Custom";
        case 0x01 : return "null SL packet header";
        case 0x02 : return "Reserved for use in MP4 files";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_ObjectTypeIndication(int8u ID)
{
    switch (ID)
    {
        case 0x01 : return "Systems ISO/IEC 14496-1";
        case 0x02 : return "Systems ISO/IEC 14496-1 (v2)";
        case 0x03 : return "Interaction Stream";
        case 0x05 : return "AFX Stream";
        case 0x06 : return "Font Data Stream";
        case 0x07 : return "Synthesized Texture Stream";
        case 0x08 : return "Streaming Text Stream";
        case 0x20 : return "Visual ISO/IEC 14496-2 (MPEG-4 Visual)";
        case 0x21 : return "Visual ISO/IEC 14496-10 (AVC)";
        case 0x22 : return "Parameter Sets for Visual ISO/IEC 14496-10 (AVC)";
        case 0x24 : return "ALS"; //Not sure
        case 0x2B : return "SAOC"; //Not sure
        case 0x40 : return "Audio ISO/IEC 14496-3 (AAC)";
        case 0x60 : return "Visual ISO/IEC 13818-2 Simple Profile (MPEG Video)";
        case 0x61 : return "Visual ISO/IEC 13818-2 Main Profile (MPEG Video)";
        case 0x62 : return "Visual ISO/IEC 13818-2 SNR Profile (MPEG Video)";
        case 0x63 : return "Visual ISO/IEC 13818-2 Spatial Profile (MPEG Video)";
        case 0x64 : return "Visual ISO/IEC 13818-2 High Profile (MPEG Video)";
        case 0x65 : return "Visual ISO/IEC 13818-2 422 Profile (MPEG Video)";
        case 0x66 : return "Audio ISO/IEC 13818-7 Main Profile (AAC)";
        case 0x67 : return "Audio ISO/IEC 13818-7 Low Complexity Profile (AAC)";
        case 0x68 : return "Audio ISO/IEC 13818-7 Scaleable Sampling Rate Profile (AAC)";
        case 0x69 : return "Audio ISO/IEC 13818-3 (MPEG Audio)";
        case 0x6A : return "Visual ISO/IEC 11172-2 (MPEG Video)";
        case 0x6B : return "Audio ISO/IEC 11172-3 (MPEG Audio)";
        case 0x6C : return "Visual ISO/IEC 10918-1 (JPEG)";
        case 0x6D : return "PNG";
        case 0xA0 : return "EVRC";
        case 0xA1 : return "SMV";
        case 0xA2 : return "3GPP2 Compact Multimedia Format (CMF)";
        case 0xA3 : return "VC-1";
        case 0xA4 : return "Dirac";
        case 0xA5 : return "AC-3";
        case 0xA6 : return "E-AC-3";
        case 0xA9 : return "DTS";
        case 0xAA : return "DTS-HD High Resolution";
        case 0xAB : return "DTS-HD Master Audio";
        case 0xAC : return "DTS-HD Express";
        case 0xD1 : return "Private - EVRC";
        case 0xD3 : return "Private - AC-3";
        case 0xD4 : return "Private - DTS";
        case 0xDD : return "Private - Ogg";
        case 0xDE : return "Private - Ogg";
        case 0xE1 : return "Private - QCELP";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_StreamType(int8u ID)
{
    switch (ID)
    {
        case 0x01 : return "ObjectDescriptorStream";
        case 0x02 : return "ClockReferenceStream";
        case 0x03 : return "SceneDescriptionStream";
        case 0x04 : return "VisualStream";
        case 0x05 : return "AudioStream";
        case 0x06 : return "MPEG7Stream";
        case 0x07 : return "IPMPStream";
        case 0x08 : return "ObjectContentInfoStream";
        case 0x09 : return "MPEGJStream";
        case 0x0A : return "Interaction Stream";
        case 0x0B : return "IPMPToolStream";
        case 0x0C : return "FontDataStream";
        case 0x0D : return "StreamingText";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_ODProfileLevelIndication(int8u /*ID*/)
{
    return "";
}

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_SceneProfileLevelIndication(int8u ID)
{
    switch (ID)
    {
        case    1 : return "Simple2D@L1";
        case    2 : return "Simple2D@L2";
        case   11 : return "Basic2D@L1";
        case   12 : return "Core2D@L1";
        case   13 : return "Core2D@L2";
        case   14 : return "Advanced2D@L1";
        case   15 : return "Advanced2D@L2";
        case   16 : return "Advanced2D@L3";
        case   17 : return "Main2D@L1";
        case   18 : return "Main2D@L2";
        case   19 : return "Main2D@L3";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_AudioProfileLevelIndication(int8u ID)
{
    switch (ID)
    {
        case    1 : return "Main@L1";
        case    2 : return "Main@L2";
        case    3 : return "Main@L3";
        case    4 : return "Main@L4";
        case    5 : return "Scalable@L1";
        case    6 : return "Scalable@L2";
        case    7 : return "Scalable@L3";
        case    8 : return "Scalable@L4";
        case    9 : return "Speech@L1";
        case   10 : return "Speech@L2";
        case   11 : return "Synthesis@L1";
        case   12 : return "Synthesis@L2";
        case   13 : return "Synthesis@L3";
        case   14 : return "HighQualityAudio@L1";
        case   15 : return "HighQualityAudio@L2";
        case   16 : return "HighQualityAudio@L3";
        case   17 : return "HighQualityAudio@L4";
        case   18 : return "HighQualityAudio@L5";
        case   19 : return "HighQualityAudio@L6";
        case   20 : return "HighQualityAudio@L7";
        case   21 : return "HighQualityAudio@L8";
        case   22 : return "LowDelayAudio@L1";
        case   23 : return "LowDelayAudio@L2";
        case   24 : return "LowDelayAudio@L3";
        case   25 : return "LowDelayAudio@L4";
        case   26 : return "LowDelayAudio@L5";
        case   27 : return "LowDelayAudio@L6";
        case   28 : return "LowDelayAudio@L7";
        case   29 : return "LowDelayAudio@L8";
        case   30 : return "NaturalAudio@L1";
        case   31 : return "NaturalAudio@L2";
        case   32 : return "NaturalAudio@L3";
        case   33 : return "NaturalAudio@L4";
        case   34 : return "MobileAudioInternetworking@L1";
        case   35 : return "MobileAudioInternetworking@L2";
        case   36 : return "MobileAudioInternetworking@L3";
        case   37 : return "MobileAudioInternetworking@L4";
        case   38 : return "MobileAudioInternetworking@L5";
        case   39 : return "MobileAudioInternetworking@L6";
        case   40 : return "AAC@L1";
        case   41 : return "AAC@L2";
        case   42 : return "AAC@L4";
        case   43 : return "AAC@L5";
        case   44 : return "HighEfficiencyAAC@L2";
        case   45 : return "HighEfficiencyAAC@L3";
        case   46 : return "HighEfficiencyAAC@L4";
        case   47 : return "HighEfficiencyAAC@L5";
        case   59 : return "HighDefinitionAAC@L1";
        case   60 : return "ALSSimple@L1";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
extern const char* Mpeg4v_Profile_Level(int32u Profile_Level);

//---------------------------------------------------------------------------
const char* Mpeg4_Descriptors_GraphicsProfileLevelIndication(int8u ID)
{
    switch (ID)
    {
        case    1 : return "Simple2D@L1";
        case    2 : return "Simple2D+Text@L1";
        case    3 : return "Simple2D+Text@L2";
        case    4 : return "Core2D@L1";
        case    5 : return "Core2D@L2";
        case    6 : return "Advanced2D@L1";
        case    7 : return "Advanced2D@L2";
        default   : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpeg4_Descriptors::File_Mpeg4_Descriptors()
:File__Analyze()
{
    //Configuration
    ParserName=__T("MPEG-4 Descriptor");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Mpeg4_Desc;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    IsRawStream=true;

    //In
    KindOfStream=Stream_Max;
    PosOfStream=(size_t)-1;
    Parser_DoNotFreeIt=false;
    SLConfig_DoNotFreeIt=false;

    //Out
    Parser=NULL;
    ES_ID=0x0000;
    SLConfig=NULL;

    //Temp
    ObjectTypeId=0x00;
}

//---------------------------------------------------------------------------
File_Mpeg4_Descriptors::~File_Mpeg4_Descriptors()
{
    if (!Parser_DoNotFreeIt)
        delete Parser;// Parser=NULL;
    if (!SLConfig_DoNotFreeIt)
        delete SLConfig;// SLConfig=NULL;
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Header_Parse()
{
    //Parsing
    size_t Size=0;
    int8u type, Size_ToAdd;
    Get_B1(type,                                            "type");
    if (type==0)
    {
        Header_Fill_Code(0x00, "Padding");
        Header_Fill_Size(1);
        return;
    }
    do
    {
        Get_B1(Size_ToAdd,                                  "size");
        Size=(Size<<7) | (Size_ToAdd&0x7F);
    }
    while (Size_ToAdd&0x80);

    //Filling
    Header_Fill_Code(type, Ztring().From_CC1(type));
    if (Element_Offset+Size>=Element_Size)
        Size=(size_t)(Element_Size-Element_Offset); //Found one file with too big size but content is OK, cutting the block
    Header_Fill_Size(Element_Offset+Size);
}


//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Data_Parse()
{
    //Preparing
    Status[IsAccepted]=true;

    #define ELEMENT_CASE(_NAME, _DETAIL) \
        case 0x##_NAME : Element_Name(_DETAIL); Descriptor_##_NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        ELEMENT_CASE(00, "Forbidden");
        ELEMENT_CASE(01, "ObjectDescrTag");
        ELEMENT_CASE(02, "InitialObjectDescrTag");
        ELEMENT_CASE(03, "ES_DescrTag");
        ELEMENT_CASE(04, "DecoderConfigDescrTag");
        ELEMENT_CASE(05, "DecSpecificInfoTag");
        ELEMENT_CASE(06, "SLConfigDescrTag");
        ELEMENT_CASE(07, "ContentIdentDescrTag");
        ELEMENT_CASE(08, "SupplContentIdentDescrTag");
        ELEMENT_CASE(09, "IPI_DescrPointerTag");
        ELEMENT_CASE(0A, "IPMP_DescrPointerTag");
        ELEMENT_CASE(0B, "IPMP_DescrTag");
        ELEMENT_CASE(0C, "QoS_DescrTag");
        ELEMENT_CASE(0D, "RegistrationDescrTag");
        ELEMENT_CASE(0E, "ES_ID_IncTag");
        ELEMENT_CASE(0F, "ES_ID_RefTag");
        ELEMENT_CASE(10, "MP4_IOD_Tag");
        ELEMENT_CASE(11, "MP4_OD_Tag");
        ELEMENT_CASE(12, "IPL_DescrPointerRefTag");
        ELEMENT_CASE(13, "ExtendedProfileLevelDescrTag");
        ELEMENT_CASE(14, "profileLevelIndicationIndexDescrTag");
        ELEMENT_CASE(40, "ContentClassificationDescrTag");
        ELEMENT_CASE(41, "KeyWordDescrTag");
        ELEMENT_CASE(42, "RatingDescrTag");
        ELEMENT_CASE(43, "LanguageDescrTag");
        ELEMENT_CASE(44, "ShortTextualDescrTag");
        ELEMENT_CASE(45, "ExpandedTextualDescrTag");
        ELEMENT_CASE(46, "ContentCreatorNameDescrTag");
        ELEMENT_CASE(47, "ContentCreationDateDescrTag");
        ELEMENT_CASE(48, "OCICreatorNameDescrTag");
        ELEMENT_CASE(49, "OCICreationDateDescrTag");
        ELEMENT_CASE(4A, "SmpteCameraPositionDescrTag");
        ELEMENT_CASE(4B, "SegmentDescrTag");
        ELEMENT_CASE(4C, "MediaTimeDescrTag");
        ELEMENT_CASE(60, "IPMP_ToolsListDescrTag");
        ELEMENT_CASE(61, "IPMP_ToolTag");
        ELEMENT_CASE(62, "FLEXmuxTimingDescrTag");
        ELEMENT_CASE(63, "FLEXmuxCodeTableDescrTag");
        ELEMENT_CASE(64, "ExtSLConfigDescrTag");
        ELEMENT_CASE(65, "FLEXmuxBufferSizeDescrTag");
        ELEMENT_CASE(66, "FLEXmuxIdentDescrTag");
        ELEMENT_CASE(67, "DependencyPointerTag");
        ELEMENT_CASE(68, "DependencyMarkerTag");
        ELEMENT_CASE(69, "FLEXmuxChannelDescrTag");
        default: if (Element_Code>=0xC0)
                    Element_Name("user private");
                 else
                    Element_Name("unknown");
                 Skip_XX(Element_Size,                          "Data");
                 break;
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_01()
{
    //Parsing
    bool URL_Flag;
    BS_Begin();
    Skip_S2(10,                                                 "ObjectDescriptorID");
    Get_SB (    URL_Flag,                                       "URL_Flag");
    Skip_SB(                                                    "includeInlineProfileLevelFlag");
    Skip_S1( 4,                                                 "reserved");
    BS_End();
    if (URL_Flag)
    {
        int8u URLlength;
        Get_B1 (URLlength,                                      "URLlength");
        Skip_UTF8(URLlength,                                    "URLstring");
    }
    if (Element_Code==0x02 || Element_Code==0x10)
    {
        Info_B1(ODProfileLevel,                                 "ODProfileLevelIndication"); Param_Info1(Mpeg4_Descriptors_ODProfileLevelIndication(ODProfileLevel));
        Info_B1(SceneProfileLevel,                              "sceneProfileLevelIndication"); Param_Info1(Mpeg4_Descriptors_SceneProfileLevelIndication(SceneProfileLevel));
        Info_B1(AudioProfileLevel,                              "audioProfileLevelIndication"); Param_Info1(Mpeg4_Descriptors_AudioProfileLevelIndication(AudioProfileLevel));
        Info_B1(VisualProfileLevel,                             "visualProfileLevelIndication"); Param_Info1(Mpeg4v_Profile_Level(VisualProfileLevel));
        Info_B1(GraphicsProfileLevel,                           "graphicsProfileLevelIndication"); Param_Info1(Mpeg4_Descriptors_GraphicsProfileLevelIndication(GraphicsProfileLevel));
    }

    FILLING_BEGIN();
        Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_03()
{
    //Parsing
    bool streamDependenceFlag, URL_Flag, OCRstreamFlag;
    Get_B2 (ES_ID,                                              "ES_ID");
    BS_Begin();
    Get_SB (   streamDependenceFlag,                            "streamDependenceFlag");
    Get_SB (   URL_Flag,                                        "URL_Flag");
    Get_SB (   OCRstreamFlag,                                   "OCRstreamFlag");
    Skip_S1(5,                                                  "streamPriority");
    BS_End();
    if (streamDependenceFlag)
        Skip_B2(                                                "dependsOn_ES_ID");
    if (URL_Flag)
    {
        int8u URLlength;
        Get_B1 (URLlength,                                      "URLlength");
        Skip_UTF8(URLlength,                                    "URLstring");
    }
    if (OCRstreamFlag)
        Skip_B2(                                                "OCR_ES_Id");

    FILLING_BEGIN();
        Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_04()
{
    //Parsing
    int32u bufferSizeDB, MaxBitrate, AvgBitrate;
    int8u  streamType;
    Get_B1 (ObjectTypeId,                                       "objectTypeIndication"); Param_Info1(Mpeg4_Descriptors_ObjectTypeIndication(ObjectTypeId));
    BS_Begin();
    Get_S1 (6, streamType,                                      "streamType"); Param_Info1(Mpeg4_Descriptors_StreamType(streamType));
    Skip_SB(                                                    "upStream");
    Skip_SB(                                                    "reserved");
    BS_End();
    Get_B3 (bufferSizeDB,                                       "bufferSizeDB");
    Get_B4 (MaxBitrate,                                         "maxBitrate");
    Get_B4 (AvgBitrate,                                         "avgBitrate");

    FILLING_BEGIN();
        if (KindOfStream==Stream_Max)
            switch (ObjectTypeId)
            {
                case 0x20 :
                case 0x21 :
                case 0x60 :
                case 0x61 :
                case 0x62 :
                case 0x63 :
                case 0x64 :
                case 0x65 :
                case 0x6A :
                case 0x6C :
                case 0x6D :
                case 0x6E :
                case 0xA3 :
                case 0xA4 :
                            KindOfStream=Stream_Video; break;
                case 0x40 :
                case 0x66 :
                case 0x67 :
                case 0x68 :
                case 0x69 :
                case 0x6B :
                case 0xA0 :
                case 0xA1 :
                case 0xA5 :
                case 0xA6 :
                case 0xA9 :
                case 0xAA :
                case 0xAB :
                case 0xAC :
                case 0xD1 :
                case 0xD3 :
                case 0xD4 :
                case 0xE1 :
                            KindOfStream=Stream_Audio; break;
                case 0x08 :
                            KindOfStream=Stream_Text; break;
                default: ;
            }
        if (Count_Get(KindOfStream)==0)
            Stream_Prepare(KindOfStream);
        switch (ObjectTypeId)
        {
            case 0x01 : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "System", Error, false, true); break;
            case 0x02 : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "System Core", Error, false, true); break;
            //case 0x03 Interaction Stream
            //case 0x05 AFX
            //case 0x06 Font Data
            //case 0x07 Synthesized Texture Stream
            case 0x08 : Fill(Stream_Text    , StreamPos_Last, Text_Format, "Streaming Text", Error, false, true); break;
            case 0x20 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG-4 Visual", Error, false, true); break;
            case 0x21 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "AVC", Error, false, true); break;
            //case 0x22 Parameter Sets for AVC
            case 0x40 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "AAC", Error, false, true); break; //MPEG-4 AAC
            case 0x60 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, "Simple" , Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 2", Error, false, true); break; //MPEG-2V Simple
            case 0x61 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, "Main"   , Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 2", Error, false, true); break; //MPEG-2V Main
            case 0x62 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, "SNR"    , Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 2", Error, false, true); break; //MPEG-2V SNR
            case 0x63 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, "Spatial", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 2", Error, false, true); break; //MPEG-2V Spatial
            case 0x64 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, "High"   , Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 2", Error, false, true); break; //MPEG-2V High
            case 0x65 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, "4:2:2"  , Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 2", Error, false, true); break; //MPEG-2V 4:2:2
            case 0x66 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "AAC", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "Main", Error, false, true); break; //MPEG-2 AAC Main
            case 0x67 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "AAC", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "LC", Error, false, true); break; //MPEG-2 AAC LC
            case 0x68 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "AAC", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "SSR", Error, false, true); break; //MPEG-2 AAC SSR
            case 0x69 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "MPEG Audio", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Version, "Version 2", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "Layer 3", Error, false, true); break;
            case 0x6A : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); Fill(Stream_Video, StreamPos_Last, Video_Format_Version, "Version 1", Error, false, true); break;
            case 0x6B : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "MPEG Audio", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Version, "Version 1", Error, false, true); break;
            case 0x6C : Fill(Stream_Video   , StreamPos_Last, Video_Format, "JPEG", Error, false, true); break;
            case 0x6D : Fill(Stream_Video   , StreamPos_Last, Video_Format, "PNG", Error, false, true); break;
            case 0x6E : Fill(Stream_Video   , StreamPos_Last, Video_Format, "MPEG Video", Error, false, true); break;
            case 0xA0 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "EVRC", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 8000, 10, true); Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 1, 10, true); break;
            case 0xA1 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "SMV", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 8000, 10, true); Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 1, 10, true);  break;
            case 0xA2 : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "3GPP2", Error, false, true); break;
            case 0xA3 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "VC-1", Error, false, true); break;
            case 0xA4 : Fill(Stream_Video   , StreamPos_Last, Video_Format, "Dirac", Error, false, true); break;
            case 0xA5 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "AC-3", Error, false, true); break;
            case 0xA6 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "E-AC-3", Error, false, true); break;
            case 0xA9 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "DTS", Error, false, true); break;
            case 0xAA : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "DTS", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "HRA", Error, false, true); break; // DTS-HD High Resolution
            case 0xAB : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "DTS", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "MA", Error, false, true); break;  // DTS-HD Master Audio
            case 0xAC : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "DTS", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, "Express", Error, false, true); break;  // DTS Express a.k.a. LBR
            case 0xD1 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "EVRC", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 8000, 10, true); Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 1, 10, true);  break;
            case 0xD3 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "AC-3", Error, false, true); break;
            case 0xD4 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "DTS", Error, false, true); break;
            case 0xDD : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "Ogg", Error, false, true); break;
            case 0xDE : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "Ogg", Error, false, true); break;
            case 0xE1 : Fill(Stream_Audio   , StreamPos_Last, Audio_Format, "QCELP", Error, false, true); Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 8000, 10, true); Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 1, 10, true);  break;
            default: ;
        }
        switch (ObjectTypeId)
        {
            case 0x01 : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), "System", Error, false, true); break;
            case 0x02 : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), "System Core", Error, false, true); break;
            case 0x20 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-4V", Error, false, true); break;
            case 0x21 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "H264", Error, false, true); break;
            case 0x40 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AAC", Error, false, true); break; //MPEG-4 AAC
            case 0x60 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-2V", Error, false, true); break; //MPEG-2V Simple
            case 0x61 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-2V", Error, false, true); break; //MPEG-2V Main
            case 0x62 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-2V", Error, false, true); break; //MPEG-2V SNR
            case 0x63 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-2V", Error, false, true); break; //MPEG-2V Spatial
            case 0x64 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-2V", Error, false, true); break; //MPEG-2V High
            case 0x65 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-2V", Error, false, true); break; //MPEG-2V 4:2:2
            case 0x66 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AAC", Error, false, true); break; //MPEG-2 AAC Main
            case 0x67 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AAC", Error, false, true); break; //MPEG-2 AAC LC
            case 0x68 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AAC", Error, false, true); break; //MPEG-2 AAC SSR
            case 0x69 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "MPEG-2A L3", Error, false, true); break;
            case 0x6A : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-1V", Error, false, true); break;
            case 0x6B : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "MPEG-1A", Error, false, true); break;
            case 0x6C : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "JPEG", Error, false, true); break;
            case 0x6D : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "PNG", Error, false, true); break;
            case 0x6E : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-4V", Error, false, true); break;
            case 0xA0 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "EVRC", Error, false, true); break;
            case 0xA1 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "SMV", Error, false, true); break;
            case 0xA2 : Fill(Stream_Video   , StreamPos_Last, Video_Codec, "MPEG-4V", Error, false, true); break;
            case 0xA3 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "VC-1", Error, false, true); break;
            case 0xA4 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "Dirac", Error, false, true); break;
            case 0xA5 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AC3", Error, false, true); break;
            case 0xA6 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AC3+", Error, false, true); break;
            case 0xA9 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "DTS", Error, false, true); break;
            case 0xAA :
            case 0xAB : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "DTS-HD", Error, false, true); break;
            case 0xAC : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "DTS Express", Error, false, true); break;
            case 0xD1 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "EVRC", Error, false, true); break;
            case 0xD3 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "AC3", Error, false, true); break;
            case 0xD4 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "DTS", Error, false, true); break;
            case 0xDD : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), "Ogg", Error, false, true); break;
            case 0xDE : Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), "Ogg", Error, false, true); break;
            case 0xE1 : Fill(Stream_Audio   , StreamPos_Last, Audio_Codec, "QCELP", Error, false, true); break;
            default: ;
        }
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_CodecID), ObjectTypeId, 16, true);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec_CC), ObjectTypeId, 16, true);

        //Bitrate mode
        if (AvgBitrate>0
         && !(bufferSizeDB==AvgBitrate && bufferSizeDB==MaxBitrate && bufferSizeDB==0x1000)) //Some buggy data were found
        {
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate_Nominal), AvgBitrate);
            if (MaxBitrate<=AvgBitrate*1.005)
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate_Mode), "CBR");
            else
            {
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate_Mode), "VBR");
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate_Maximum), MaxBitrate);
            }
        }

        //Creating parser
        delete Parser; Parser=NULL;
        switch (ObjectTypeId)
        {
            case 0x01 : switch (streamType)
                        {
                            case 0x01 : Parser=new File_Mpeg4_Descriptors; break;
                            default   : ;
                        }
                        break;
            case 0x20 : //MPEG-4 Visual
                        #if defined(MEDIAINFO_MPEG4V_YES)
                            Parser=new File_Mpeg4v;
                            ((File_Mpeg4v*)Parser)->Frame_Count_Valid=1;
                            ((File_Mpeg4v*)Parser)->FrameIsAlwaysComplete=true;
                        #endif
                        break;
            case 0x21 : //AVC
                        #if defined(MEDIAINFO_AVC_YES)
                            Parser=new File_Avc;
                            ((File_Avc*)Parser)->MustParse_SPS_PPS=true;
                            ((File_Avc*)Parser)->MustSynchronize=false;
                            ((File_Avc*)Parser)->SizedBlocks=true;
                        #endif
                        break;
            case 0x40 : //MPEG-4 AAC
            case 0x66 :
            case 0x67 :
            case 0x68 : //MPEG-2 AAC
                        #if defined(MEDIAINFO_AAC_YES)
                            Parser=new File_Aac;
                            ((File_Aac*)Parser)->Mode=File_Aac::Mode_AudioSpecificConfig;
                            ((File_Aac*)Parser)->FrameIsAlwaysComplete=true;
                        #endif
                        break;
            case 0x60 :
            case 0x61 :
            case 0x62 :
            case 0x63 :
            case 0x64 :
            case 0x65 :
            case 0x6A : //MPEG Video
                        #if defined(MEDIAINFO_MPEGV_YES)
                            Parser=new File_Mpegv;
                            ((File_Mpegv*)Parser)->FrameIsAlwaysComplete=true;
                        #endif
                        break;
            case 0x69 :
            case 0x6B : //MPEG Audio
                        #if defined(MEDIAINFO_MPEGA_YES)
                            Parser=new File_Mpega;
                        #endif
                        break;
            case 0x6C : //JPEG
                        #if defined(MEDIAINFO_JPEG_YES)
                            Parser=new File_Jpeg;
                            ((File_Jpeg*)Parser)->StreamKind=Stream_Video;
                        #endif
                        break;
            case 0x6D : //PNG
                        #if defined(MEDIAINFO_PNG_YES)
                            Parser=new File_Png;
                        #endif
                        break;
            case 0xA3 : //VC-1
                        #if defined(MEDIAINFO_VC1_YES)
                            Parser=new File_Vc1;
                        #endif
                        break;
            case 0xA4 : //Dirac
                        #if defined(MEDIAINFO_DIRAC_YES)
                            Parser=new File_Dirac;
                        #endif
                        break;
            case 0xA5 : //AC-3
            case 0xA6 : //E-AC-3
            case 0xD3 : //AC-3
                        #if defined(MEDIAINFO_AC3_YES)
                            Parser=new File_Ac3;
                        #endif
                        break;
            case 0xA9 : //DTS
            case 0xAA : //DTS HRA
            case 0xAB : //DTS MA
            case 0xAC : //DTS Express
            case 0xD4 : //DTS
                        #if defined(MEDIAINFO_DTS_YES)
                            Parser=new File_Dts;
                        #endif
                        break;
            case 0xDD :
            case 0xDE : //OGG
                        #if defined(MEDIAINFO_OGG_YES)
                            Parser=new File_Ogg;
                            Parser->MustSynchronize=false;
                            ((File_Ogg*)Parser)->SizedBlocks=true;
                        #endif
                        break;
            default: ;
        }

        Element_Code=(int64u)-1;
        Open_Buffer_Init(Parser);

        Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_05()
{
    if (ObjectTypeId==0x00 && Parser==NULL) //If no ObjectTypeId detected
    {
        switch (KindOfStream)
        {
            case Stream_Video :
                                #if defined(MEDIAINFO_MPEG4V_YES)
                                    delete Parser; Parser=new File_Mpeg4v;
                                    ((File_Mpeg4v*)Parser)->FrameIsAlwaysComplete=true;
                                #endif
                                break;
            case Stream_Audio :
                                #if defined(MEDIAINFO_AAC_YES)
                                    delete Parser; Parser=new File_Aac;
                                    ((File_Aac*)Parser)->Mode=File_Aac::Mode_AudioSpecificConfig;
                                #endif
                                break;
            default: ;
        }

        Element_Code=(int64u)-1;
        Open_Buffer_Init(Parser);
    }

    if (Parser==NULL)
    {
        Skip_XX(Element_Size,                                   "Unknown");
        return;
    }

    //Parser configuration before the parsing
    switch (ObjectTypeId)
    {
            case 0x60 :
            case 0x61 :
            case 0x62 :
            case 0x63 :
            case 0x64 :
            case 0x65 :
            case 0x6A : //MPEG Video
                    #if defined(MEDIAINFO_MPEGV_YES)
                        ((File_Mpegv*)Parser)->TimeCodeIsNotTrustable=true;
                    #endif
                    break;
        default: ;
    }

    //Parsing
    Open_Buffer_Continue(Parser);

    //Demux
    #if MEDIAINFO_DEMUX
        if (ObjectTypeId!=0x21 || !Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get()) //0x21 is AVC
            switch (Config->Demux_InitData_Get())
            {
                case 0 :    //In demux event
                            Demux_Level=2; //Container
                            Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_Header);
                            break;
                case 1 :    //In field
                            {
                            std::string Data_Raw((const char*)(Buffer+Buffer_Offset), (size_t)Element_Size);
                            std::string Data_Base64(Base64::encode(Data_Raw));
                            Parser->Fill(KindOfStream, PosOfStream, "Demux_InitBytes", Data_Base64);
                            if (PosOfStream<(*Parser->Stream_More)[KindOfStream].size())
                                (*Parser->Stream_More)[KindOfStream][PosOfStream](Ztring().From_Local("Demux_InitBytes"), Info_Options)=__T("N NT");
                            }
                            break;
                default :   ;
            }
    #endif //MEDIAINFO_DEMUX

    //Parser configuration after the parsing
    switch (ObjectTypeId)
    {
            case 0x60 :
            case 0x61 :
            case 0x62 :
            case 0x63 :
            case 0x64 :
            case 0x65 :
            case 0x6A : //MPEG Video
                    #if defined(MEDIAINFO_MPEGV_YES)
                        ((File_Mpegv*)Parser)->TimeCodeIsNotTrustable=false;
                    #endif
                    break;
        default: ;
    }

    //Positionning
    Element_Offset=Element_Size;
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_06()
{
    delete SLConfig; SLConfig=new slconfig;

    //Parsing
    int8u predefined;
    Get_B1 (predefined,                                         "predefined"); Param_Info1(Mpeg4_Descriptors_Predefined(predefined));
    switch (predefined)
    {
        case 0x00 :
            {
                    BS_Begin();
                    Get_SB (SLConfig->useAccessUnitStartFlag,   "useAccessUnitStartFlag");
                    Get_SB (SLConfig->useAccessUnitEndFlag,     "useAccessUnitEndFlag");
                    Get_SB (SLConfig->useRandomAccessPointFlag, "useRandomAccessPointFlag");
                    Get_SB (SLConfig->hasRandomAccessUnitsOnlyFlag, "hasRandomAccessUnitsOnlyFlag");
                    Get_SB (SLConfig->usePaddingFlag,           "usePaddingFlag");
                    Get_SB (SLConfig->useTimeStampsFlag,        "useTimeStampsFlag");
                    Get_SB (SLConfig->useIdleFlag,              "useIdleFlag");
                    Get_SB (SLConfig->durationFlag,             "durationFlag");
                    BS_End();
                    Get_B4 (SLConfig->timeStampResolution,      "timeStampResolution");
                    Get_B4( SLConfig->OCRResolution,            "OCRResolution");
                    Get_B1 (SLConfig->timeStampLength,          "timeStampLength");
                    Get_B1 (SLConfig->OCRLength,                "OCRLength");
                    Get_B1 (SLConfig->AU_Length,                "AU_Length");
                    Get_B1 (SLConfig->instantBitrateLength,     "instantBitrateLength");
                    BS_Begin();
                    Get_S1 (4, SLConfig->degradationPriorityLength, "degradationPriorityLength");
                    Get_S1 (5, SLConfig->AU_seqNumLength,       "AU_seqNumLength");
                    Get_S1 (5, SLConfig->packetSeqNumLength,    "packetSeqNumLength");
                    Skip_S1(2,                                  "reserved");
                    BS_End();
            }
            break;
        case 0x01 :
                    SLConfig->useAccessUnitStartFlag            =false;
                    SLConfig->useAccessUnitEndFlag              =false;
                    SLConfig->useRandomAccessPointFlag          =false;
                    SLConfig->hasRandomAccessUnitsOnlyFlag      =false;
                    SLConfig->usePaddingFlag                    =false;
                    SLConfig->useTimeStampsFlag                 =false;
                    SLConfig->useIdleFlag                       =false;
                    SLConfig->durationFlag                      =false; //-
                    SLConfig->timeStampResolution               =1000;
                    SLConfig->OCRResolution                     =0; //-
                    SLConfig->timeStampLength                   =32;
                    SLConfig->OCRLength                         =0; //-
                    SLConfig->AU_Length                         =0;
                    SLConfig->instantBitrateLength              =0; //-
                    SLConfig->degradationPriorityLength         =0;
                    SLConfig->AU_seqNumLength                   =0;
                    SLConfig->packetSeqNumLength                =0;
                    break;
        case 0x02 :
                    SLConfig->useAccessUnitStartFlag            =false;
                    SLConfig->useAccessUnitEndFlag              =false;
                    SLConfig->useRandomAccessPointFlag          =false;
                    SLConfig->hasRandomAccessUnitsOnlyFlag      =false;
                    SLConfig->usePaddingFlag                    =false;
                    SLConfig->useTimeStampsFlag                 =true;
                    SLConfig->useIdleFlag                       =false;
                    SLConfig->durationFlag                      =false;
                    SLConfig->timeStampResolution               =0; //-
                    SLConfig->OCRResolution                     =0; //-
                    SLConfig->timeStampLength                   =0;
                    SLConfig->OCRLength                         =0;
                    SLConfig->AU_Length                         =0;
                    SLConfig->instantBitrateLength              =0;
                    SLConfig->degradationPriorityLength         =0;
                    SLConfig->AU_seqNumLength                   =0;
                    SLConfig->packetSeqNumLength                =0;
                    break;
        default   :
                    SLConfig->useAccessUnitStartFlag            =false;
                    SLConfig->useAccessUnitEndFlag              =false;
                    SLConfig->useRandomAccessPointFlag          =false;
                    SLConfig->hasRandomAccessUnitsOnlyFlag      =false;
                    SLConfig->usePaddingFlag                    =false;
                    SLConfig->useTimeStampsFlag                 =false;
                    SLConfig->useIdleFlag                       =false;
                    SLConfig->durationFlag                      =false;
                    SLConfig->timeStampResolution               =0;
                    SLConfig->OCRResolution                     =0;
                    SLConfig->timeStampLength                   =0;
                    SLConfig->AU_Length                         =0;
                    SLConfig->instantBitrateLength              =0;
                    SLConfig->degradationPriorityLength         =0;
                    SLConfig->AU_seqNumLength                   =0;
                    SLConfig->packetSeqNumLength                =0;
    }
    if (SLConfig->durationFlag)
    {
        Get_B4 (SLConfig->timeScale,                            "timeScale");
        Get_B2 (SLConfig->accessUnitDuration,                   "accessUnitDuration");
        Get_B2 (SLConfig->compositionUnitDuration,              "compositionUnitDuration");
    }
    else
    {
                SLConfig->timeScale                             =0; //-
                SLConfig->accessUnitDuration                    =0; //-
                SLConfig->compositionUnitDuration               =0; //-
    }
    if (!SLConfig->useTimeStampsFlag)
    {
        BS_Begin();
        Get_S8 (SLConfig->timeStampLength, SLConfig->startDecodingTimeStamp, "startDecodingTimeStamp");
        Get_S8 (SLConfig->timeStampLength, SLConfig->startCompositionTimeStamp, "startCompositionTimeStamp");
        BS_End();
    }
    else
    {
                SLConfig->startDecodingTimeStamp                =0; //-
                SLConfig->startCompositionTimeStamp             =0; //-
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_09()
{
    //Parsing
    Skip_B2(                                                    "IPI_ES_Id");
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_0E()
{
    //Parsing
    Skip_B4(                                                    "Track_ID"); //ID of the track to use
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_0F()
{
    //Parsing
    Skip_B2(                                                    "ref_index");  //track ref. index of the track to use
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_10()
{
    Descriptor_02();
}

//---------------------------------------------------------------------------
void File_Mpeg4_Descriptors::Descriptor_11()
{
    Descriptor_01();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_MPEG4_YES
