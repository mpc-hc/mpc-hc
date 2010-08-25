// File_Mpegv - Info for MPEG Video files
// Copyright (C) 2004-2010 MediaArea.net SARL, Info@MediaArea.net
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

//***************************************************************************
// Infos (Global)
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPEGV_YES) || defined(MEDIAINFO_MPEGTS_YES) || defined(MEDIAINFO_MPEGPS_YES) || defined(MEDIAINFO_MXF_YES)
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

#include "ZenLib/Conf.h"
using namespace ZenLib;

//---------------------------------------------------------------------------
extern const float32 Mpegv_frame_rate[]=
{
    (float32) 0,
    (float32)23.976,
    (float32)24,
    (float32)25,
    (float32)29.97,
    (float32)30,
    (float32)50,
    (float32)59.94,
    (float32)60,
    (float32) 0,
    (float32) 0,
    (float32) 0,
    (float32) 0,
    (float32) 0,
    (float32) 0,
    (float32) 0,
};

//---------------------------------------------------------------------------
const char* Mpegv_Colorimetry_format[]=
{
    "",
    "4:2:0",
    "4:2:2",
    "4:4:4",
};

//---------------------------------------------------------------------------
const char* Mpegv_profile_and_level_indication (int8u profile_and_level_indication)
{
    switch (profile_and_level_indication)
    {
        case 0x82 : return "4:2:2@High";
        case 0x85 : return "4:2:2@Main";
        case 0x8A : return "Multi-view@High";
        case 0x8B : return "Multi-view@High-1440";
        case 0x8D : return "Multi-view@Main";
        case 0x8E : return "Multi-view@Low";
        default : return "";
    }
};

//---------------------------------------------------------------------------
const char* Mpegv_profile_and_level_indication_profile[]=
{
    "0",
    "High",
    "Spatial Scalable",
    "SNR Scalable",
    "Main",
    "Simple",
    "6",
    "7",
}; //4:2:2 Profile?

//---------------------------------------------------------------------------
const char* Mpegv_profile_and_level_indication_level[]=
{
    "0",
    "1",
    "2",
    "3",
    "High",
    "4",
    "High 1440",
    "5",
    "Main",
    "6",
    "Low",
    "7",
    "8",
    "9",
    "10",
    "11",
};

} //NameSpace

//---------------------------------------------------------------------------
#endif //...
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPEGV_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Mpegv.h"
#include "ZenLib/BitStream.h"
#include "ZenLib/Utils.h"
#if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #include "MediaInfo/Text/File_DtvccTransport.h"
#endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
#if defined(MEDIAINFO_SCTE20_YES)
    #include "MediaInfo/Text/File_Scte20.h"
#endif //defined(MEDIAINFO_SCTE20_YES)
#if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
    #include "MediaInfo/Text/File_Cdp.h"
    #include <cstring>
#endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
#if defined(MEDIAINFO_AFDBARDATA_YES)
    #include "MediaInfo/Video/File_AfdBarData.h"
    #include <cstring>
#endif //defined(MEDIAINFO_AFDBARDATA_YES)
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
using namespace ZenLib;

#undef FILLING_BEGIN
#define FILLING_BEGIN() \
    while (Element_Offset<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset]==0x00) \
        Element_Offset++; \
    if (Element_Offset!=Element_Size) \
        Trusted_IsNot("Size error"); \
    else if (Element_IsOK()) \
    { \

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const float32 Mpegv_aspect_ratio1[]=
{
    (float32)0,
    (float32)1,
    (float32)0.6735,
    (float32)0.7031, //16/9 PAL
    (float32)0.7615,
    (float32)0.8055,
    (float32)0.8437, //16/9 NTSC
    (float32)0.8935,
    (float32)0.9375, //4/3 PAL
    (float32)0.9815,
    (float32)1.0255,
    (float32)1.0695,
    (float32)1.1250, //4/3 NTSC
    (float32)1.1575,
    (float32)1.2015,
    (float32)0,
};

//---------------------------------------------------------------------------
const float32 Mpegv_aspect_ratio2[]=
{
    (float32)0,
    (float32)1,
    (float32)4/(float32)3,
    (float32)16/(float32)9,
    (float32)2.21,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
};

//---------------------------------------------------------------------------
const char* Mpegv_video_format[]=
{
    "Component",
    "PAL",
    "NTSC",
    "SECAM",
    "MAC",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Mpegv_picture_structure[]=
{
    "",
    "T", //Top Field
    "B", //Bottom Field
    "F", //Frame
};

const char* Mpegv_picture_coding_type[]=
{
    "",
    "I",
    "P",
    "B",
    "D",
    "",
    "",
    "",
};

const char* Mpegv_extension_start_code_identifier[]=
{
    "",
    "Sequence",
    "Sequence Display",
    "Quant Matrix",
    "Copyright",
    "Sequence Scalable",
    "",
    "Picture Display",
    "Picture Coding",
    "Picture Spatial Scalable",
    "Picture Temporal Scalable",
    "Camera Parameters",
    "ITU-T",
    "",
    "",
    "",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpegv::File_Mpegv()
:File__Analyze()
{
    //Configuration
    ParserName=_T("MPEG Video");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Mpegv;
        StreamIDs_Width[0]=16;
    #endif //MEDIAINFO_EVENTS
    Trusted_Multiplier=2;
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;

    //In
    MPEG_Version=1;
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?40:2;
    FrameIsAlwaysComplete=false;
    TimeCodeIsNotTrustable=false;
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
        Cdp_Data=NULL;
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)
        AfdBarData_Data=NULL;
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)

    //temporal_reference
    TemporalReference_Offset=0;
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        GA94_03_Parser=NULL;
        GA94_03_TemporalReference_Offset=0;
        GA94_03_IsPresent=false;
        CC___Parser=NULL;
        CC___IsPresent=false;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #if defined(MEDIAINFO_SCTE20_YES)
        Scte_Parser=NULL;
        Scte_TemporalReference_Offset=0;
        Scte_IsPresent=false;
    #endif //defined(MEDIAINFO_SCTE20_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        DTG1_Parser=NULL;
        GA94_06_Parser=NULL;
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
        Cdp_Parser=NULL;
        Cdp_IsPresent=false;
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)
        AfdBarData_Parser=NULL;
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)

    //Temp
    SizeToAnalyse_Begin=1*1024*1024;
    SizeToAnalyse_End=1*1024*1024;
    Searching_TimeStamp_Start_DoneOneTime=false;
    sequence_header_IsParsed=false;
    Parsing_End_ForDTS=false;
}

//---------------------------------------------------------------------------
File_Mpegv::~File_Mpegv()
{
    for (size_t Pos=0; Pos<TemporalReference.size(); Pos++)
        delete TemporalReference[Pos]; //TemporalReference[Pos]=NULL;
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        delete GA94_03_Parser; //GA94_03_Parser=NULL;
        delete CC___Parser; //CC___Parser=NULL;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #if defined(MEDIAINFO_SCTE20_YES)
        delete Scte_Parser; //Scte_Parser=NULL;
    #endif //defined(MEDIAINFO_SCTE20_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        delete DTG1_Parser; //DTG1_Parser=NULL;
        delete GA94_06_Parser; //GA94_06_Parser=NULL;
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpegv::Streams_Fill()
{
    //Filling
    Stream_Prepare(Stream_Video);

    //Version
    if (MPEG_Version==2)
    {
        Fill(Stream_General, 0, General_Format_Version, "Version 2");
        Fill(Stream_Video, 0, Video_Format, "MPEG Video");
        Fill(Stream_Video, 0, Video_Format_Version, "Version 2");
        Fill(Stream_Video, 0, Video_Format_Commercial, "MPEG-2 Video");
        Fill(Stream_Video, 0, Video_Codec, "MPEG-2V");
        Fill(Stream_Video, 0, Video_Codec_String, "MPEG-2 Video", Unlimited, true, true);
    }
    else
    {
        Fill(Stream_General, 0, General_Format_Version, "Version 1");
        Fill(Stream_Video, 0, Video_Format, "MPEG Video");
        Fill(Stream_Video, 0, Video_Format_Version, "Version 1");
        Fill(Stream_Video, 0, Video_Format_Commercial, "MPEG-1 Video");
        Fill(Stream_Video, 0, Video_Codec, "MPEG-1V");
        Fill(Stream_Video, 0, Video_Codec_String, "MPEG-1 Video", Unlimited, true, true);
    }

    Fill(Stream_Video, 0, Video_Width, 0x1000*horizontal_size_extension+horizontal_size_value);
    Fill(Stream_Video, 0, Video_Height, 0x1000*vertical_size_extension+vertical_size_value);
    Fill(Stream_Video, 0, Video_Colorimetry, Mpegv_Colorimetry_format[chroma_format]);
    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
    Fill(Stream_Video, 0, Video_Resolution, 8);

    //AspectRatio
    if (MPEG_Version==2)
    {
        if (aspect_ratio_information==0)
            ;//Forbidden
        else if (aspect_ratio_information==1)
            Fill(Stream_Video, 0, Video_PixelAspectRatio, 1.000, 3, true);
        else if (display_horizontal_size && display_vertical_size)
        {
            if (vertical_size_value && Mpegv_aspect_ratio2[aspect_ratio_information])
                Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, (float)(0x1000*horizontal_size_extension+horizontal_size_value)/(0x1000*vertical_size_extension+vertical_size_value)
                                                                             *Mpegv_aspect_ratio2[aspect_ratio_information]/((float)display_horizontal_size/display_vertical_size), 3, true);
        }
        else if (Mpegv_aspect_ratio2[aspect_ratio_information])
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, Mpegv_aspect_ratio2[aspect_ratio_information], 3, true);
    }
    else //Version 1
    {
        if (vertical_size_value && Mpegv_aspect_ratio1[aspect_ratio_information])
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, (float)(0x1000*horizontal_size_extension+horizontal_size_value)/(0x1000*vertical_size_extension+vertical_size_value)/Mpegv_aspect_ratio1[aspect_ratio_information], 3, true);
    }

    //FrameRate
    Fill(Stream_Video, StreamPos_Last, Video_FrameRate, (float)(Mpegv_frame_rate[frame_rate_code] * (frame_rate_extension_n + 1)) / (float)(frame_rate_extension_d + 1));

    //BitRate
    if (vbv_delay==0xFFFF || (MPEG_Version==1 && bit_rate_value==0x3FFFF))
        Fill(Stream_Video, 0, Video_BitRate_Mode, "VBR");
    else if ((MPEG_Version==1 && bit_rate_value!=0x3FFFF) || MPEG_Version==2)
        Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");
    if (bit_rate_value_IsValid && (bit_rate_extension>0 || bit_rate_value!=0x3FFFF))
        Fill(Stream_Video, 0, Video_BitRate_Nominal, ((((int32u)bit_rate_extension<<12))+bit_rate_value)*400);

    //Interlacement
    if (MPEG_Version==1)
    {
        Fill(Stream_Video, 0, Video_ScanType, "Progressive");
        Fill(Stream_Video, 0, Video_Interlacement, "PPF");
    }
    else if (progressive_frame_Count && progressive_frame_Count!=Frame_Count)
    {
        //This is mixed
    }
    else if (Frame_Count>0) //Only if we have at least one progressive_frame definition
    {
        if (progressive_sequence || progressive_frame_Count==Frame_Count)
        {
            Fill(Stream_Video, 0, Video_ScanType, "Progressive");
            Fill(Stream_Video, 0, Video_Interlacement, "PPF");
            if (!progressive_sequence && !(Interlaced_Top && Interlaced_Bottom) && !(!Interlaced_Top && !Interlaced_Bottom))
                Fill(Stream_Video, 0, Video_ScanOrder, Interlaced_Top?"TFF":"BFF");
        }
        else
        {
            Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
            if ((Interlaced_Top && Interlaced_Bottom) || (!Interlaced_Top && !Interlaced_Bottom))
                Fill(Stream_Video, 0, Video_Interlacement, "Interlaced");
            else
            {
                Fill(Stream_Video, 0, Video_ScanOrder, Interlaced_Top?"TFF":"BFF");
                Fill(Stream_Video, 0, Video_Interlacement, Interlaced_Top?"TFF":"BFF");
            }
        }
        std::string TempRef, CodingType;
        for (size_t Pos=0; Pos<TemporalReference.size(); Pos++)
            if (TemporalReference[Pos] && TemporalReference[Pos]->HasPictureCoding)
            {
                TempRef+=TemporalReference[Pos]->top_field_first?"T":"B";
                TempRef+=TemporalReference[Pos]->repeat_first_field?"3":"2";
                CodingType+=Mpegv_picture_coding_type[TemporalReference[Pos]->picture_coding_type];
            }
        if (TempRef.find('3')!=std::string::npos)
        {
            if (TempRef.find("T2T3B2B3T2T3B2B3")!=std::string::npos
             || TempRef.find("B2B3T2T3B2B3T2T3")!=std::string::npos)
            {
                Fill(Stream_Video, 0, Video_ScanOrder, "2:3 Pulldown", Unlimited, true, true);
                Fill(Stream_Video, 0, Video_FrameRate, FrameRate*24/30, 3, true); //Real framerate
                Fill(Stream_Video, 0, Video_ScanType, "Progressive", Unlimited, true, true);
                Fill(Stream_Video, 0, Video_Interlacement, "PPF", Unlimited, true, true);
            }
            if (TempRef.find("T2T2T2T2T2T2T2T2T2T2T2T3B2B2B2B2B2B2B2B2B2B2B2B3")!=std::string::npos
             || TempRef.find("B2B2B2B2B2B2B2B2B2B2B2B3T2T2T2T2T2T2T2T2T2T2T2T3")!=std::string::npos)
            {
                Fill(Stream_Video, 0, Video_ScanOrder, "2:2:2:2:2:2:2:2:2:2:2:3 Pulldown", Unlimited, true, true);
                Fill(Stream_Video, 0, Video_FrameRate, FrameRate*24/25, 3, true); //Real framerate
                Fill(Stream_Video, 0, Video_ScanType, "Progressive", Unlimited, true, true);
                Fill(Stream_Video, 0, Video_Interlacement, "PPF", Unlimited, true, true);
            }
        }

        //GOP
        std::vector<Ztring> GOPs;
        size_t GOP_Frame_Count=0;
        size_t GOP_BFrames_Max=0;
        size_t I_Pos1=CodingType.find(_T('I'));
        while (I_Pos1!=std::string::npos)
        {
            size_t I_Pos2=CodingType.find(_T('I'), I_Pos1+1);
            if (I_Pos2!=std::string::npos)
            {
                std::vector<size_t> P_Positions;
                size_t P_Position=I_Pos1;
                do
                {
                    P_Position=CodingType.find(_T('P'), P_Position+1);
                    if (P_Position<I_Pos2)
                        P_Positions.push_back(P_Position);
                }
                while (P_Position<I_Pos2);
                Ztring GOP;
                if (!P_Positions.empty())
                {
                    GOP+=_T("M=")+Ztring::ToZtring(P_Positions[0]-I_Pos1)+_T(", ");
                    if (P_Positions[0]-I_Pos1>GOP_BFrames_Max)
                        GOP_BFrames_Max=P_Positions[0]-I_Pos1;
                }
                GOP+=_T("N=")+Ztring::ToZtring(I_Pos2-I_Pos1);
                GOPs.push_back(GOP);
                GOP_Frame_Count+=I_Pos2-I_Pos1;
            }
            I_Pos1=I_Pos2;
        }

        if (GOP_Frame_Count+GOP_BFrames_Max>Frame_Count && !GOPs.empty())
            GOPs.resize(GOPs.size()-1); //Removing the last one, there may have uncomplete B-frame filling

        if (!GOPs.empty())
        {
            size_t Unique=0;
            for (size_t Pos=1; Pos<GOPs.size(); Pos++)
                if (GOPs[Pos]!=GOPs[0])
                    Unique++;
            if ((Frame_Count<Frame_Count_Valid*10 && Unique) || Unique>2) //In order to accept some unsynch //TODO: change the method, synching with next I-Frame
                GOPs.clear(); //Not a fixed GOP
        }
        if (!GOPs.empty())
            Fill(Stream_Video, 0, Video_Format_Settings_GOP, GOPs[0]);
    }

    //Profile
    if (!profile_and_level_indication_escape && profile_and_level_indication_profile!=(int8u)-1 && profile_and_level_indication_level!=(int8u)-1)
    {
        Fill(Stream_Video, 0, Video_Format_Profile, Ztring().From_Local(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile])+_T("@")+Ztring().From_Local(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]));
        Fill(Stream_Video, 0, Video_Codec_Profile, Ztring().From_Local(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile])+_T("@")+Ztring().From_Local(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]));
    }
    else if (profile_and_level_indication_escape)
    {
        Fill(Stream_Video, 0, Video_Format_Profile, Ztring().From_Local(Mpegv_profile_and_level_indication(profile_and_level_indication)));
        Fill(Stream_Video, 0, Video_Codec_Profile, Ztring().From_Local(Mpegv_profile_and_level_indication(profile_and_level_indication)));
    }

    //Standard
    Fill(Stream_Video, 0, Video_Standard, Mpegv_video_format[video_format]);

    //Matrix
    if (load_intra_quantiser_matrix || load_non_intra_quantiser_matrix)
    {
        Fill(Stream_Video, 0, Video_Format_Settings, "CustomMatrix");
        Fill(Stream_Video, 0, Video_Format_Settings_Matrix, "Custom");
        Fill(Stream_Video, 0, Video_Format_Settings_Matrix_Data, Matrix_intra);
        Fill(Stream_Video, 0, Video_Format_Settings_Matrix_Data, Matrix_nonintra);
        Fill(Stream_Video, 0, Video_Codec_Settings, "CustomMatrix");
        Fill(Stream_Video, 0, Video_Codec_Settings_Matrix, "Custom");
    }
    else
    {
        Fill(Stream_Video, 0, Video_Format_Settings_Matrix, "Default");
        Fill(Stream_Video, 0, Video_Codec_Settings_Matrix, "Default");
    }

    //library
    if (Library.size()>=8)
    {
        Fill(Stream_Video, 0, Video_Encoded_Library, Library);
        Fill(Stream_Video, 0, Video_Encoded_Library_Name, Library_Name);
        Fill(Stream_Video, 0, Video_Encoded_Library_Version, Library_Version);
        Fill(Stream_Video, 0, General_Encoded_Library, Library);
        Fill(Stream_Video, 0, General_Encoded_Library_Name, Library_Name);
        Fill(Stream_Video, 0, General_Encoded_Library_Version, Library_Version);
    }

    //Delay
    if (group_start_IsParsed)
    {
        size_t Time_Begin=Time_Begin_Seconds*1000;
        if (FrameRate)
            Time_Begin+=(size_t)(Time_Begin_Frames*1000/FrameRate);
        Fill(Stream_Video, 0, Video_Delay, Time_Begin);
        Fill(Stream_Video, 0, Video_Delay_Settings, Ztring(_T("drop_frame_flag="))+(group_start_drop_frame_flag?_T("1"):_T("0")));
        Fill(Stream_Video, 0, Video_Delay_Settings, Ztring(_T("closed_gop="))+(group_start_closed_gop?_T("1"):_T("0")));
        Fill(Stream_Video, 0, Video_Delay_Settings, Ztring(_T("broken_link="))+(group_start_broken_link?_T("1"):_T("0")));
    }

    //BVOP
    if (BVOP_Count>0)
    {
        Fill(Stream_Video, 0, Video_Format_Settings, "BVOP");
        Fill(Stream_Video, 0, Video_Format_Settings_BVOP, "Yes");
    }
    else
        Fill(Stream_Video, 0, Video_Format_Settings_BVOP, "No");

    //Buffer
    Fill(Stream_Video, 0, Video_BufferSize, 2*1024*((((int32u)vbv_buffer_size_extension)<<10)+vbv_buffer_size_value));

    //Autorisation of other streams
    NextCode_Clear();
    NextCode_Add(0x00);
    NextCode_Add(0xB8);
    for (int8u Pos=0x00; Pos<=0xB9; Pos++)
        Streams[Pos].Searching_Payload=false;
    Streams[0xB8].Searching_TimeStamp_End=true;
    if (IsSub)
        Streams[0x00].Searching_TimeStamp_End=true;

    //Caption may be in user_data, must be activated if full parsing is requested
    if (MediaInfoLib::Config.ParseSpeed_Get()>=1)
    {
        Streams[0x00].Searching_Payload=true;
        Streams[0xB2].Searching_Payload=true;
        Streams[0xB3].Searching_Payload=true;
        Streams[0xB5].Searching_Payload=true;
    }
}

//---------------------------------------------------------------------------
void File_Mpegv::Streams_Finish()
{
    //Duration
    if (Time_End_NeedComplete && MediaInfoLib::Config.ParseSpeed_Get()!=1)
        Time_End_Seconds=Error;
    if (Time_End_Seconds!=Error)
    {
        size_t Time_Begin=Time_Begin_Seconds*1000;
        size_t Time_End =Time_End_Seconds*1000;
        if (FrameRate)
        {
            Time_Begin+=(size_t)(Time_Begin_Frames*1000/FrameRate);
            Time_End  +=(size_t)(Time_End_Frames  *1000/FrameRate);
        }
        if (Time_End>Time_Begin)
            Fill(Stream_Video, 0, Video_Duration, Time_End-Time_Begin);
    }

    //Other parsers
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        if (GA94_03_Parser && !GA94_03_Parser->Status[IsFinished] && GA94_03_Parser->Status[IsAccepted])
        {
            Finish(GA94_03_Parser);
            Merge(*GA94_03_Parser);
        }
        if (CC___Parser && !CC___Parser->Status[IsFinished] && CC___Parser->Status[IsAccepted])
        {
            Finish(CC___Parser);
            Merge(*CC___Parser);
        }
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #if defined(MEDIAINFO_SCTE20_YES)
        if (Scte_Parser && !Scte_Parser->Status[IsFinished] && Scte_Parser->Status[IsAccepted])
        {
            Finish(Scte_Parser);
            Merge(*Scte_Parser);
        }
    #endif //defined(MEDIAINFO_SCTE20_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        if (DTG1_Parser && !DTG1_Parser->Status[IsFinished] && DTG1_Parser->Status[IsAccepted])
        {
            Finish(DTG1_Parser);
            Merge(*DTG1_Parser);
        }
        if (GA94_06_Parser && !GA94_06_Parser->Status[IsFinished] && GA94_06_Parser->Status[IsAccepted])
        {
            Finish(GA94_06_Parser);
            Merge(*GA94_06_Parser);
        }
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
        if (Cdp_Parser && !Cdp_Parser->Status[IsFinished] && Cdp_Parser->Status[IsAccepted])
        {
            Finish(Cdp_Parser);
            Merge(*Cdp_Parser);
        }
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)
        if (AfdBarData_Parser && !AfdBarData_Parser->Status[IsFinished] && AfdBarData_Parser->Status[IsAccepted])
        {
            Finish(AfdBarData_Parser);
            Merge(*AfdBarData_Parser);
        }
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
    {
        Streams.clear();
        for (size_t Pos=0; Pos<TemporalReference.size(); Pos++)
            delete TemporalReference[Pos]; //TemporalReference[Pos]=NULL;
        TemporalReference.clear();
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mpegv::Synched_Test()
{
    //Trailing 0xFF
    while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]==0xFF)
        Buffer_Offset++;

    //Trailing 0x00
    while(Buffer_Offset+3<=Buffer_Size && Buffer[Buffer_Offset+2]==0x00
                                       && Buffer[Buffer_Offset+1]==0x00
                                       && Buffer[Buffer_Offset  ]==0x00)
        Buffer_Offset++;

    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x00
     || Buffer[Buffer_Offset+1]!=0x00
     || Buffer[Buffer_Offset+2]!=0x01)
        Synched=false;

    //Quick search
    if (Synched && !Header_Parser_QuickSearch())
        return false;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Mpegv::Synched_Init()
{
    //Temp
    Frame_Count=0;
    BVOP_Count=0;
    progressive_frame_Count=0;
    Interlaced_Top=0;
    Interlaced_Bottom=0;
    display_horizontal_size=0;
    display_vertical_size=0;
    vbv_delay=0;
    vbv_buffer_size_value=0;
    Time_Begin_Seconds=Error;
    Time_Begin_Frames=(int8u)-1;
    Time_End_Seconds=Error;
    Time_End_Frames=(int8u)-1;
    picture_coding_type=(int8u)-1;
    bit_rate_value=0;
    FrameRate=0;
    horizontal_size_value=0;
    vertical_size_value=0;
    bit_rate_extension=0;
    temporal_reference_Old=(int16u)-1;
    aspect_ratio_information=0;
    frame_rate_code=0;
    profile_and_level_indication_profile=(int8u)-1;
    profile_and_level_indication_level=(int8u)-1;
    chroma_format=0;
    horizontal_size_extension=0;
    vertical_size_extension=0;
    frame_rate_extension_n=0;
    frame_rate_extension_d=0;
    video_format=5; //Unspecified video format
    vbv_buffer_size_extension=0;
    Time_End_NeedComplete=false;
    load_intra_quantiser_matrix=false;
    load_non_intra_quantiser_matrix=false;
    progressive_sequence=true; //progressive by default
    top_field_first=false;
    repeat_first_field=false;
    FirstFieldFound=false;
    group_start_IsParsed=false;
    bit_rate_value_IsValid=false;
    profile_and_level_indication_escape=false;

    //Default stream values
    Streams.resize(0x100);
    Streams[0xB3].Searching_Payload=true;
    for (int8u Pos=0xFF; Pos>=0xB9; Pos--)
        Streams[Pos].Searching_Payload=true; //Testing MPEG-PS
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpegv::Read_Buffer_Unsynched()
{
    Time_End_Seconds=Error;
    Time_End_Frames=(int8u)-1;

    temporal_reference_Old=(int16u)-1;
    for (size_t Pos=0; Pos<TemporalReference.size(); Pos++)
        delete TemporalReference[Pos]; //TemporalReference[Pos]=NULL;
    TemporalReference.clear();
    TemporalReference_Offset=0;
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        GA94_03_TemporalReference_Offset=0;
        if (GA94_03_Parser)
            GA94_03_Parser->Open_Buffer_Unsynch();
        if (CC___Parser)
            CC___Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #if defined(MEDIAINFO_SCTE20_YES)
        Scte_TemporalReference_Offset=0;
        if (Scte_Parser)
            Scte_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_SCTE20_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        if (DTG1_Parser)
            DTG1_Parser->Open_Buffer_Unsynch();
        if (GA94_06_Parser)
            GA94_06_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
        if (Cdp_Parser)
            Cdp_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)
        if (AfdBarData_Parser)
            AfdBarData_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)

    //NextCode
    NextCode_Clear();
    NextCode_Add(0x00);
    NextCode_Add(0xB8);
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpegv::Header_Parse()
{
    //Parsing
    int8u start_code;
    Skip_B3(                                                    "synchro");
    Get_B1 (start_code,                                         "start_code");
    if (!Header_Parser_Fill_Size())
    {
        Element_WaitForMoreData();
        return;
    }

    //Filling
    Header_Fill_Code(start_code, Ztring().From_CC1(start_code));
}

//---------------------------------------------------------------------------
bool File_Mpegv::Header_Parser_Fill_Size()
{
    //Look for next Sync word
    if (Buffer_Offset_Temp==0) //Buffer_Offset_Temp is not 0 if Header_Parse_Fill_Size() has already parsed first frames
        Buffer_Offset_Temp=Buffer_Offset+4;
    while (Buffer_Offset_Temp+4<=Buffer_Size
        && CC3(Buffer+Buffer_Offset_Temp)!=0x000001)
    {
        Buffer_Offset_Temp+=2;
        while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]!=0x00)
            Buffer_Offset_Temp+=2;
        if (Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp-1]==0x00 || Buffer_Offset_Temp>=Buffer_Size)
            Buffer_Offset_Temp--;
    }

    //Must wait more data?
    if (Buffer_Offset_Temp+4>Buffer_Size)
    {
        if (FrameIsAlwaysComplete || File_Offset+Buffer_Size==File_Size)
            Buffer_Offset_Temp=Buffer_Size; //We are sure that the next bytes are a start
        else
            return false;
    }

    //OK, we continue
    Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
    Buffer_Offset_Temp=0;
    return true;
}

//---------------------------------------------------------------------------
bool File_Mpegv::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+4<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x00
      &&   Buffer[Buffer_Offset+1]==0x00
      &&   Buffer[Buffer_Offset+2]==0x01)
    {
        //Getting start_code
        int8u start_code=Buffer[Buffer_Offset+3];

        //Searching start or timestamp
        if (Streams[start_code].Searching_Payload
         || Streams[start_code].Searching_TimeStamp_Start
         || Streams[start_code].Searching_TimeStamp_End)
            return true;

        //Synchronizing
        Buffer_Offset+=4;
        Synched=false;
        if (!Synchronize_0x000001())
        {
            if (!IsSub && File_Offset+Buffer_Size==File_Size && !Status[IsFilled] && Frame_Count>=1)
            {
                //End of file, and we have some frames
                Accept("MPEG Video");
                Fill("MPEG Video");
                Detect_EOF();
                return false;
            }
        }
    }

    if (Buffer_Offset+3==Buffer_Size)
        return false; //Sync is OK, but start_code is not available
    if (!Synched)
        return false;
    Trusted_IsNot("MPEG Video, Synchronisation lost");
    return Synchronize();
}

//---------------------------------------------------------------------------
void File_Mpegv::Data_Parse()
{
    //Parsing
    switch (Element_Code)
    {
        case 0x00: picture_start(); break;
        case 0xB0: Skip_XX(Element_Size,                        "Unknown"); break;
        case 0xB1: Skip_XX(Element_Size,                        "Unknown"); break;
        case 0xB2: user_data_start(); break;
        case 0xB3: sequence_header(); break;
        case 0xB4: sequence_error(); break;
        case 0xB5: extension_start(); break;
        case 0xB6: Skip_XX(Element_Size,                        "Unknown"); break;
        case 0xB7: sequence_end(); break;
        case 0xB8: group_start(); break;
        default:
            if (Element_Code>=0x01
             && Element_Code<=0xAF) slice_start();
            else
                Trusted_IsNot("Unattended element");
    }
}

//***************************************************************************
// EOF
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpegv::Detect_EOF()
{
    if (IsSub && Status[IsFilled]
     || (!IsSub && File_Size>SizeToAnalyse_Begin+SizeToAnalyse_End && File_Offset+Buffer_Offset+Element_Offset>SizeToAnalyse_Begin && File_Offset+Buffer_Offset+Element_Offset<File_Size-SizeToAnalyse_End && MediaInfoLib::Config.ParseSpeed_Get()<=0.01))
    {
        if ((GA94_03_IsPresent || CC___IsPresent || Scte_IsPresent || Cdp_IsPresent) && Frame_Count<Frame_Count_Valid*10 //10 times the normal test
         && !(!IsSub && File_Size>SizeToAnalyse_Begin*10+SizeToAnalyse_End*10 && File_Offset+Buffer_Offset+Element_Offset>SizeToAnalyse_Begin*10 && File_Offset+Buffer_Offset+Element_Offset<File_Size-SizeToAnalyse_End*10))
        {
            Streams[0x00].Searching_Payload=GA94_03_IsPresent || Cdp_IsPresent;
            Streams[0xB2].Searching_Payload=GA94_03_IsPresent || CC___IsPresent || Scte_IsPresent;
            Streams[0xB3].Searching_Payload=GA94_03_IsPresent || Cdp_IsPresent;
            return;
        }

        //
        Time_End_Seconds=Error;
        Time_End_Frames=(int8u)-1;

        //Autorisation of other streams
        if (!IsSub)
            Streams[0x00].Searching_TimeStamp_End=false;

        //Jumping
        if (!Status[IsFilled])
            Fill("MPEG Video");

        GoToFromEnd(SizeToAnalyse_End*2, "MPEG Video");
        EOF_AlreadyDetected=true; //Sometimes called from Filling
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
// Packet "00"
void File_Mpegv::picture_start()
{
    Element_Name("picture_start");

    //Coherency
    if (!NextCode_Test())
        return;

    //Parsing
    BS_Begin();
    Get_S2 (10, temporal_reference,                             "temporal_reference");
    Get_S1 ( 3, picture_coding_type,                            "picture_coding_type"); Param_Info(Mpegv_picture_coding_type[picture_coding_type]);
    Get_S2 (16, vbv_delay,                                      "vbv_delay");
    if (picture_coding_type==2 || picture_coding_type==3) //P or B
    {
        Skip_S1(1,                                              "full_pel_forward_vector");
        Skip_S1(3,                                              "forward_f_code");
    }
    if (picture_coding_type==3) //B
    {
        Skip_S1(1,                                              "full_pel_backward_vector");
        Skip_S1(3,                                              "backward_f_code");
    }
    bool extra_bit_picture;
    do
    {
        Peek_SB(extra_bit_picture);
        if (extra_bit_picture)
        {
            Skip_S1(1,                                          "extra_bit_picture");
            Skip_S1(8,                                          "extra_information_picture");
        }
    }
    while (extra_bit_picture);
    BS_End();

    FILLING_BEGIN();
        if (temporal_reference==temporal_reference_Old)
        {
            Frame_Count--;
            Frame_Count_InThisBlock--;
        }
        else
            temporal_reference_Old=temporal_reference;

        //Temporal reference
        if (TemporalReference_Offset+temporal_reference>=TemporalReference.size())
            TemporalReference.resize(TemporalReference_Offset+temporal_reference+1);
        if (TemporalReference[TemporalReference_Offset+temporal_reference]==NULL)
            TemporalReference[TemporalReference_Offset+temporal_reference]=new temporalreference;
        TemporalReference[TemporalReference_Offset+temporal_reference]->IsValid=true;

        //Info
        #if MEDIAINFO_TRACE
            Element_Info(_T("Frame ")+Ztring::ToZtring(Frame_Count));
            Element_Info(_T("picture_coding_type ")+Ztring().From_Local(Mpegv_picture_coding_type[picture_coding_type]));
            Element_Info(_T("temporal_reference ")+Ztring::ToZtring(temporal_reference));
            if (PTS!=(int64u)-1)
                Element_Info(_T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)PTS)/1000000)));
            if (DTS!=(int64u)-1)
                Element_Info(_T("DTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)DTS)/1000000)));
            if (Time_End_Seconds!=Error)
            {
                int32u Time_End  =Time_End_Seconds  *1000;
                if (FrameRate)
                    Time_End  +=(int32u)float32_int32s(Time_End_Frames  *1000/FrameRate);
                size_t Hours  = Time_End/60/60/1000;
                size_t Minutes=(Time_End-(Hours*60*60*1000))/60/1000;
                size_t Seconds=(Time_End-(Hours*60*60*1000)-(Minutes*60*1000))/1000;
                size_t Milli  =(Time_End-(Hours*60*60*1000)-(Minutes*60*1000)-(Seconds*1000));

                Ztring Time;
                Time+=Ztring::ToZtring(Hours);
                Time+=_T(':');
                Time+=Ztring::ToZtring(Minutes);
                Time+=_T(':');
                Time+=Ztring::ToZtring(Seconds);
                if (FrameRate!=0)
                {
                    Time+=_T('.');
                    Time+=Ztring::ToZtring(Milli);
                }
                Element_Info(_T("time_code ")+Time);
            }
        #endif //MEDIAINFO_TRACE

        //Time
        if (Time_End_Seconds!=Error)
        {
            Time_End_Frames++; //One frame
            if (progressive_sequence && repeat_first_field)
            {
                Time_End_Frames++; //Frame repeated a second time
                if (top_field_first)
                    Time_End_Frames++; //Frame repeated a third time
            }
        }

        //Counting
        if (File_Offset+Buffer_Offset+Element_Size==File_Size)
            Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
        Frame_Count++;
        Frame_Count_InThisBlock++;
        if (picture_coding_type==3)
            BVOP_Count++;

        //Need to parse?
        if (!Streams[0x00].Searching_Payload)
            return;

        //NextCode
        NextCode_Clear();
        for (int64u Element_Name_Next=0x01; Element_Name_Next<=0x1F; Element_Name_Next++)
            NextCode_Add(Element_Name_Next);
        NextCode_Add(0xB2);
        NextCode_Add(0xB5);
        NextCode_Add(0xB8);

        //Autorisation of other streams
        for (int8u Pos=0x01; Pos<=0x1F; Pos++)
            Streams[Pos].Searching_Payload=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "01" --> "AF"
void File_Mpegv::slice_start()
{
    if (!NextCode_Test())
        return;
    Element_Name("slice_start");

    //Parsing
    Skip_XX(Element_Size,                                       "data");

    FILLING_BEGIN();
        //NextCode
        NextCode_Clear();
        NextCode_Add(0x00);
        NextCode_Add(0xB3);
        NextCode_Add(0xB8);

        //Autorisation of other streams
        for (int8u Pos=0x01; Pos<=0x1F; Pos++)
            Streams[Pos].Searching_Payload=false;

        //Filling only if not already done
        if (Frame_Count==2 && !Status[IsAccepted])
            Accept("MPEG Video");
        if (!Status[IsFilled] && (!CC___IsPresent && !Scte_IsPresent && !GA94_03_IsPresent && !Cdp_IsPresent && Frame_Count>=Frame_Count_Valid || Frame_Count>=Frame_Count_Valid*10))
            Fill("MPEG Video");
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B2"
void File_Mpegv::user_data_start()
{
    Element_Name("user_data_start");

    //GA94 stuff
    if (Element_Size>=4)
    {
        int32u GA94_Identifier;
        Peek_B4(GA94_Identifier);
        switch (GA94_Identifier)
        {
            case 0x434301F8 :   user_data_start_CC(); return;
            case 0x44544731 :   user_data_start_DTG1(); return;
            case 0x47413934 :   user_data_start_GA94(); return;
            default         :   {
                                int8u SCTE20_Identifier;
                                Peek_B1(SCTE20_Identifier);
                                if (SCTE20_Identifier==0x03)
                                {
                                    user_data_start_3();
                                    return;
                                }
                                }
        }
    }

    //Rejecting junk at the begin
    size_t Library_Start_Offset=0;
    while (Library_Start_Offset+4<=Element_Size)
    {
        bool OK=true;
        for (size_t Pos=0; Pos<4; Pos++)
        {
            if (!((Buffer[Buffer_Offset+Library_Start_Offset+Pos]==0x20 && Pos)
               ||  Buffer[Buffer_Offset+Library_Start_Offset+Pos]==0x22
               ||  Buffer[Buffer_Offset+Library_Start_Offset+Pos]==0x27
               ||  Buffer[Buffer_Offset+Library_Start_Offset+Pos]==0x28
               || (Buffer[Buffer_Offset+Library_Start_Offset+Pos]==0x29 && Pos)
               || (Buffer[Buffer_Offset+Library_Start_Offset+Pos]>=0x30
               &&  Buffer[Buffer_Offset+Library_Start_Offset+Pos]<=0x3F)
               || (Buffer[Buffer_Offset+Library_Start_Offset+Pos]>=0x41
               && Buffer[Buffer_Offset+Library_Start_Offset+Pos]<=0x7D)))
            {
                OK=false;
                break;
            }
        }
        if (OK)
            break;
        Library_Start_Offset++;
    }
    if (Library_Start_Offset+4>Element_Size)
    {
        Skip_XX(Element_Size,                                   "junk");
        return; //No good info
    }

    //Accepting good data after junk
    size_t Library_End_Offset=Library_Start_Offset+4;
    while (Library_End_Offset<Element_Size
        && (Buffer[Buffer_Offset+Library_End_Offset]==0x0D
         || Buffer[Buffer_Offset+Library_End_Offset]==0x0A
         || Buffer[Buffer_Offset+Library_End_Offset]>=0x20
         && Buffer[Buffer_Offset+Library_End_Offset]<=0x3F
         || Buffer[Buffer_Offset+Library_End_Offset]>=0x41
         && Buffer[Buffer_Offset+Library_End_Offset]<=0x7D))
        Library_End_Offset++;

    //Parsing
    Ztring Temp;
    if (Library_Start_Offset>0)
        Skip_XX(Library_Start_Offset,                           "junk");
    if (Library_End_Offset-Library_Start_Offset)
        Get_Local(Library_End_Offset-Library_Start_Offset, Temp,"data");
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "junk");

    //Cleanup
    while(Temp.size()>3 && Temp[1]==_T('e') && Temp[2]==_T('n') && Temp[3]==_T('c'))
        Temp.erase(0, 1);
    while(Temp.size()>5 && Temp[3]==_T('M') && Temp[4]==_T('P') && Temp[5]==_T('E'))
        Temp.erase(0, 1);

    //Cleanup
    while(!Temp.empty() && Temp[0]==_T('0'))
        Temp.erase(0, 1);

    FILLING_BEGIN();
        if (!Temp.empty())
        {
            if (Temp.find(_T("build"))==0)
                Library+=Ztring(_T(" "))+Temp;
            else
                Library=Temp;

            //Library
            if (Temp.find(_T("Created with Nero"))==0)
            {
                Library_Name=_T("Ahead Nero");
            }
            else if (Library.find(_T("encoded by avi2mpg1 ver "))==0)
            {
                Library_Name=_T("avi2mpg1");
                Library_Version=Library.SubString(_T("encoded by avi2mpg1 ver "), _T(""));
            }
            else if (Library.find(_T("encoded by TMPGEnc (ver. "))==0)
            {
                Library_Name=_T("TMPGEnc");
                Library_Version=Library.SubString(_T("encoded by TMPGEnc (ver. "), _T(")"));
            }
            else if (Library.find(_T("encoded by TMPGEnc 4.0 XPress Version. "))==0)
            {
                Library_Name=_T("TMPGEnc XPress");
                Library_Version=Library.SubString(_T("encoded by TMPGEnc 4.0 XPress Version. "), _T(""));
            }
            else if (Library.find(_T("encoded by TMPGEnc MPEG Editor "))==0)
            {
                Library_Name=_T("TMPGEnc MPEG Editor");
                Library_Version=Library.SubString(_T("Version. "), _T(""));
            }
            else if (Library.find(_T("encoded by TMPGEnc "))==0)
            {
                Library_Name=_T("TMPGEnc");
                Library_Version=Library.SubString(_T("encoded by TMPGEnc "), _T(""));
            }
            else if (Library.find(_T("MPEG Encoder v"))==0)
            {
                Library_Name=_T("MPEG Encoder by Tristan Savatier");
                Library_Version=Library.SubString(_T("MPEG Encoder v"), _T(" by"));
            }
            else
                Library_Name=Library;

        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B2", CC (From DVD)
void File_Mpegv::user_data_start_CC()
{
    Skip_B4(                                                    "identifier");

    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        Element_Info("DVD Captions");

        //Parsing
        #if MEDIAINFO_DEMUX
            Element_Code=0x434301F800000000LL;
        #endif //MEDIAINFO_DEMUX
        if (CC___Parser==NULL)
        {
            CC___IsPresent=true;
            CC___Parser=new File_DtvccTransport;
            Open_Buffer_Init(CC___Parser);
            ((File_DtvccTransport*)CC___Parser)->Format=File_DtvccTransport::Format_DVD;
        }
        if (CC___Parser->PTS_DTS_Needed)
        {
            CC___Parser->PCR=PCR;
            CC___Parser->PTS=PTS;
            CC___Parser->DTS=DTS;
        }
        Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset), ContentType_MainStream);
        Open_Buffer_Continue(CC___Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        Element_Offset=Element_Size;
    #else //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        Skip_XX(Element_Size-Element_Offset,                    "DVD Captions");
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
}

//---------------------------------------------------------------------------
// Packet "B2", 0x03 (SCTE20)
void File_Mpegv::user_data_start_3()
{
    Skip_B1(                                                    "identifier");

    #if defined(MEDIAINFO_SCTE20_YES)
        Scte_IsPresent=true;

        Element_Info("SCTE 20");

        //Coherency
        if (TemporalReference_Offset+temporal_reference>=TemporalReference.size())
            return;

        //Purging too old orphelins
        if (Scte_TemporalReference_Offset+8<TemporalReference_Offset+temporal_reference)
        {
            size_t Pos=TemporalReference_Offset+temporal_reference;
            do
            {
                if (TemporalReference[Pos]==NULL || !TemporalReference[Pos]->IsValid)
                    break;
                Pos--;
            }
            while (Pos>0);
            Scte_TemporalReference_Offset=Pos+1;
        }

        if (TemporalReference[TemporalReference_Offset+temporal_reference]->Scte==NULL)
            TemporalReference[TemporalReference_Offset+temporal_reference]->Scte=new temporalreference::buffer_data;
        TemporalReference[TemporalReference_Offset+temporal_reference]->Scte->Size=(size_t)(Element_Size-Element_Offset);
        delete[] TemporalReference[TemporalReference_Offset+temporal_reference]->Scte->Data;
        TemporalReference[TemporalReference_Offset+temporal_reference]->Scte->Data=new int8u[(size_t)(Element_Size-Element_Offset)];
        std::memcpy(TemporalReference[TemporalReference_Offset+temporal_reference]->Scte->Data, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));

        //Parsing
        Skip_XX(Element_Size-Element_Offset,                    "CC data");

        //Parsing Captions after reordering
        bool CanBeParsed=true;
        for (size_t Scte_Pos=Scte_TemporalReference_Offset; Scte_Pos<TemporalReference.size(); Scte_Pos++)
            if (TemporalReference[Scte_Pos]==NULL || !TemporalReference[Scte_Pos]->IsValid || TemporalReference[Scte_Pos]->Scte==NULL)
                CanBeParsed=false; //There is a missing field/frame
        if (CanBeParsed)
        {
            for (size_t Scte_Pos=Scte_TemporalReference_Offset; Scte_Pos<TemporalReference.size(); Scte_Pos++)
            {
                Element_Begin("SCTE 20 data");

                //Parsing
                #if MEDIAINFO_DEMUX
                    Element_Code=0x0000000300000000LL;
                #endif //MEDIAINFO_DEMUX
                if (Scte_Parser==NULL)
                {
                    Scte_Parser=new File_Scte20;
                    Open_Buffer_Init(Scte_Parser);
                }
                if (Scte_Parser->PTS_DTS_Needed)
                {
                    Scte_Parser->PCR=PCR;
                    Scte_Parser->PTS=PTS;
                    Scte_Parser->DTS=DTS;
                }
                ((File_Scte20*)Scte_Parser)->picture_structure=TemporalReference[Scte_Pos]->picture_structure;
                ((File_Scte20*)Scte_Parser)->progressive_sequence=progressive_sequence;
                ((File_Scte20*)Scte_Parser)->progressive_frame=TemporalReference[Scte_Pos]->progressive_frame;
                ((File_Scte20*)Scte_Parser)->top_field_first=TemporalReference[Scte_Pos]->top_field_first;
                ((File_Scte20*)Scte_Parser)->repeat_first_field=TemporalReference[Scte_Pos]->repeat_first_field;
                Demux(TemporalReference[Scte_Pos]->Scte->Data, TemporalReference[Scte_Pos]->Scte->Size, ContentType_MainStream);
                Open_Buffer_Continue(Scte_Parser, TemporalReference[Scte_Pos]->Scte->Data, TemporalReference[Scte_Pos]->Scte->Size);

                Element_End();
            }
            Scte_TemporalReference_Offset=TemporalReference.size();
        }
    #else //defined(MEDIAINFO_SCTE20_YES)
        Skip_XX(Element_Size-Element_Offset,                    "SCTE 20 data");
    #endif //defined(MEDIAINFO_SCTE20_YES)
}

//---------------------------------------------------------------------------
// Packet "B2", DTG1
void File_Mpegv::user_data_start_DTG1()
{
    Skip_B4(                                                    "identifier");

    #if defined(MEDIAINFO_AFDBARDATA_YES)
        Element_Info("Active Format Description");

        //Parsing
        if (DTG1_Parser==NULL)
        {
            DTG1_Parser=new File_AfdBarData;
            Open_Buffer_Init(DTG1_Parser);
            ((File_AfdBarData*)DTG1_Parser)->Format=File_AfdBarData::Format_A53_4_DTG1;
        }
        if (DTG1_Parser->PTS_DTS_Needed)
        {
            DTG1_Parser->PCR=PCR;
            DTG1_Parser->PTS=PTS;
            DTG1_Parser->DTS=DTS;
        }
        Open_Buffer_Continue(DTG1_Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        Element_Offset=Element_Size;
    #else //defined(MEDIAINFO_AFDBARDATA_YES)
        Skip_XX(Element_Size-Element_Offset,                    "Active Format Description");
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
}

//---------------------------------------------------------------------------
// Packet "B2", GA94
void File_Mpegv::user_data_start_GA94()
{
    //Parsing
    int8u user_data_type_code;
    Skip_B4(                                                    "GA94_identifier");
    Get_B1 (user_data_type_code,                                "user_data_type_code");
    switch (user_data_type_code)
    {
        case 0x03 : user_data_start_GA94_03(); break;
        case 0x06 : user_data_start_GA94_06(); break;
        default   : Skip_XX(Element_Size-Element_Offset,        "GA94_reserved_user_data");
    }
}

//---------------------------------------------------------------------------
// Packet "B2", GA94 0x03 (styled captioning)
void File_Mpegv::user_data_start_GA94_03()
{
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        GA94_03_IsPresent=true;

        Element_Info("DTVCC Transport");

        //Coherency
        if (TemporalReference_Offset+temporal_reference>=TemporalReference.size())
            return;

        //Purging too old orphelins
        if (GA94_03_TemporalReference_Offset+8<TemporalReference_Offset+temporal_reference)
        {
            size_t Pos=TemporalReference_Offset+temporal_reference;
            do
            {
                if (TemporalReference[Pos]==NULL || !TemporalReference[Pos]->IsValid)
                    break;
                Pos--;
            }
            while (Pos>0);
            GA94_03_TemporalReference_Offset=Pos+1;
        }

        if (TemporalReference[TemporalReference_Offset+temporal_reference]->GA94_03==NULL)
            TemporalReference[TemporalReference_Offset+temporal_reference]->GA94_03=new temporalreference::buffer_data;
        TemporalReference[TemporalReference_Offset+temporal_reference]->GA94_03->Size=(size_t)(Element_Size-Element_Offset);
        delete[] TemporalReference[TemporalReference_Offset+temporal_reference]->GA94_03->Data;
        TemporalReference[TemporalReference_Offset+temporal_reference]->GA94_03->Data=new int8u[(size_t)(Element_Size-Element_Offset)];
        std::memcpy(TemporalReference[TemporalReference_Offset+temporal_reference]->GA94_03->Data, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));

        //Parsing
        Skip_XX(Element_Size-Element_Offset,                    "CC data");

        //Parsing Captions after reordering
        bool CanBeParsed=true;
        for (size_t GA94_03_Pos=GA94_03_TemporalReference_Offset; GA94_03_Pos<TemporalReference.size(); GA94_03_Pos++)
            if (TemporalReference[GA94_03_Pos]==NULL || !TemporalReference[GA94_03_Pos]->IsValid || TemporalReference[GA94_03_Pos]->GA94_03==NULL)
                CanBeParsed=false; //There is a missing field/frame
        if (CanBeParsed)
        {
            for (size_t GA94_03_Pos=GA94_03_TemporalReference_Offset; GA94_03_Pos<TemporalReference.size(); GA94_03_Pos++)
            {
                Element_Begin("Reordered DTVCC Transport");

                //Parsing
                #if MEDIAINFO_DEMUX
                    Element_Code=0x4741393400000003LL;
                #endif //MEDIAINFO_DEMUX
                if (GA94_03_Parser==NULL)
                {
                    GA94_03_Parser=new File_DtvccTransport;
                    Open_Buffer_Init(GA94_03_Parser);
                    ((File_DtvccTransport*)GA94_03_Parser)->Format=File_DtvccTransport::Format_A53_4_GA94_03;
                }
                if (GA94_03_Parser->PTS_DTS_Needed)
                {
                    GA94_03_Parser->PCR=PCR;
                    GA94_03_Parser->PTS=PTS;
                    GA94_03_Parser->DTS=DTS;
                }
                Demux(TemporalReference[GA94_03_Pos]->GA94_03->Data, TemporalReference[GA94_03_Pos]->GA94_03->Size, ContentType_MainStream);
                Open_Buffer_Continue(GA94_03_Parser, TemporalReference[GA94_03_Pos]->GA94_03->Data, TemporalReference[GA94_03_Pos]->GA94_03->Size);

                Element_End();
            }
            GA94_03_TemporalReference_Offset=TemporalReference.size();
        }
    #else //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        Skip_XX(Element_Size-Element_Offset,                    "DTVCC Transport data");
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
}

//---------------------------------------------------------------------------
// Packet "B2", GA94 0x06 (bar data)
void File_Mpegv::user_data_start_GA94_06()
{
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        Element_Info("Bar Data");

        //Parsing
        if (GA94_06_Parser==NULL)
        {
            GA94_06_Parser=new File_AfdBarData;
            Open_Buffer_Init(GA94_06_Parser);
            ((File_AfdBarData*)GA94_06_Parser)->Format=File_AfdBarData::Format_A53_4_GA94_06;
        }
        if (GA94_06_Parser->PTS_DTS_Needed)
        {
            GA94_06_Parser->PCR=PCR;
            GA94_06_Parser->PTS=PTS;
            GA94_06_Parser->DTS=DTS;
        }
        Open_Buffer_Init(GA94_06_Parser);
        Open_Buffer_Continue(GA94_06_Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        Element_Offset=Element_Size;
    #else //defined(MEDIAINFO_AFDBARDATA_YES)
        Skip_XX(Element_Size-Element_Offset,                    "Bar Data");
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
}

//---------------------------------------------------------------------------
// Packet "B3"
void File_Mpegv::sequence_header()
{
    Element_Name("sequence_header");

    //Reading
    int32u bit_rate_value_temp;
    bool  load_intra_quantiser_matrix, load_non_intra_quantiser_matrix;
    BS_Begin();
    Get_S2 (12, horizontal_size_value,                          "horizontal_size_value");
    Get_S2 (12, vertical_size_value,                            "vertical_size_value");
    Get_S1 ( 4, aspect_ratio_information,                       "aspect_ratio_information"); if (vertical_size_value && Mpegv_aspect_ratio1[aspect_ratio_information]) Param_Info((float)horizontal_size_value/vertical_size_value/Mpegv_aspect_ratio1[aspect_ratio_information]); Param_Info(Mpegv_aspect_ratio2[aspect_ratio_information]);
    Get_S1 ( 4, frame_rate_code,                                "frame_rate_code"); Param_Info(Mpegv_frame_rate[frame_rate_code]);
    Get_S3 (18, bit_rate_value_temp,                            "bit_rate_value"); Param_Info(bit_rate_value_temp*400);
    Mark_1 ();
    Get_S2 (10, vbv_buffer_size_value,                          "vbv_buffer_size_value"); Param_Info(2*1024*((int32u)vbv_buffer_size_value), " bytes");
    Skip_SB(                                                    "constrained_parameters_flag");
    TEST_SB_GET(load_intra_quantiser_matrix,                    "load_intra_quantiser_matrix");
        for (size_t Pos=0; Pos<64; Pos++)
        {
            int8u intra_quantiser;
            Get_S1 (8, intra_quantiser,                         "intra_quantiser");
            Ztring Value=Ztring::ToZtring(intra_quantiser, 16);
            if (Value.size()==1)
                Value.insert(0, _T("0"));
            Matrix_intra+=Value;
        }
    TEST_SB_END();
    TEST_SB_GET(load_non_intra_quantiser_matrix,                "load_non_intra_quantiser_matrix");
        for (size_t Pos=0; Pos<64; Pos++)
        {
            int8u non_intra_quantiser;
            Get_S1 (8, non_intra_quantiser,                     "non_intra_quantiser");
            Ztring Value=Ztring::ToZtring(non_intra_quantiser, 16);
            if (Value.size()==1)
                Value.insert(0, _T("0"));
            Matrix_nonintra+=Value;
        }
    TEST_SB_END();
    BS_End();

    //0x00 at the end
    if (Element_Offset<Element_Size)
    {
        int64u NullBytes_Begin=Element_Size-1;
        while (NullBytes_Begin>Element_Offset && Buffer[Buffer_Offset+(size_t)NullBytes_Begin]==0x00)
            NullBytes_Begin--;

        if (NullBytes_Begin==Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "Padding");
    }

    FILLING_BEGIN_PRECISE();
        //Temporal reference
        temporal_reference_Old=(int16u)-1;
        TemporalReference_Offset=TemporalReference.size();
        if (TemporalReference_Offset>=0x800)
        {
            TemporalReference.erase(TemporalReference.begin(), TemporalReference.begin()+0x400);
            if (0x400<TemporalReference_Offset)
                TemporalReference_Offset-=0x400;
            else
                TemporalReference_Offset=0;
            #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
                if (0x400<GA94_03_TemporalReference_Offset)
                    GA94_03_TemporalReference_Offset-=0x400;
                else
                    GA94_03_TemporalReference_Offset=0;
            #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
            #if defined(MEDIAINFO_SCTE20_YES)
                if (0x400<Scte_TemporalReference_Offset)
                    Scte_TemporalReference_Offset-=0x400;
                else
                    Scte_TemporalReference_Offset=0;
            #endif //defined(MEDIAINFO_SCTE20_YES)
        }

        //Bit_rate
        if (bit_rate_value_IsValid && bit_rate_value_temp!=bit_rate_value)
            bit_rate_value_IsValid=false; //two bit_rate_values, not handled.
        else if (bit_rate_value==0)
        {
            bit_rate_value=bit_rate_value_temp;
            bit_rate_value_IsValid=true;
        }

        if (sequence_header_IsParsed)
            return;

        //NextCode
        NextCode_Clear();
        NextCode_Add(0x00);
        NextCode_Add(0xB2);
        NextCode_Add(0xB5);
        NextCode_Add(0xB8);

        //Autorisation of other streams
        Streams[0x00].Searching_Payload=true;
        Streams[0xB2].Searching_Payload=true;
        Streams[0xB5].Searching_Payload=true;
        Streams[0xB8].Searching_TimeStamp_Start=true;
        Streams[0xB8].Searching_TimeStamp_End=true;

        //Temp
        FrameRate=Mpegv_frame_rate[frame_rate_code];
        SizeToAnalyse_Begin=bit_rate_value*50*2; //standard delay between TimeStamps is 0.7s, we try 2s to be sure to have at least 2 timestamps (for integrity checking)
        SizeToAnalyse_End=bit_rate_value*50*2; //standard delay between TimeStamps is 0.7s, we try 2s to be sure

        //Setting as OK
        sequence_header_IsParsed=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B4"
void File_Mpegv::sequence_error()
{
    Element_Name("sequence_error");
}

//---------------------------------------------------------------------------
// Packet "B5"
void File_Mpegv::extension_start()
{
    Element_Name("extension_start");
    MPEG_Version=2; //extension_start only exists in MPEG-2 specs

    //Parsing
    int8u extension_start_code_identifier;
    BS_Begin();
    Get_S1 ( 4, extension_start_code_identifier,                "extension_start_code_identifier"); Param_Info(Mpegv_extension_start_code_identifier[extension_start_code_identifier]);
    Element_Info(Mpegv_extension_start_code_identifier[extension_start_code_identifier]);

         switch (extension_start_code_identifier)
    {
        case 1 :{ //Sequence
                    //Parsing
                    Peek_SB(profile_and_level_indication_escape);
                    if (profile_and_level_indication_escape)
                    {
                        Get_S1 ( 8, profile_and_level_indication, "profile_and_level_indication"); Param_Info(Mpegv_profile_and_level_indication(profile_and_level_indication));
                    }
                    else
                    {
                        Skip_SB(                               "profile_and_level_indication_escape");
                        Get_S1 ( 3, profile_and_level_indication_profile, "profile_and_level_indication_profile"); Param_Info(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile]);
                        Get_S1 ( 4, profile_and_level_indication_level, "profile_and_level_indication_level"); Param_Info(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
                    }
                    Get_SB (    progressive_sequence,           "progressive_sequence");
                    Get_S1 ( 2, chroma_format,                  "chroma_format"); Param_Info(Mpegv_Colorimetry_format[chroma_format]);
                    Get_S1 ( 2, horizontal_size_extension,      "horizontal_size_extension");
                    Get_S1 ( 2, vertical_size_extension,        "vertical_size_extension");
                    Get_S2 (12, bit_rate_extension,             "bit_rate_extension");
                    Mark_1 ();
                    Get_S1 ( 8, vbv_buffer_size_extension,      "vbv_buffer_size_extension"); Param_Info(2*1024*((((int32u)vbv_buffer_size_extension)<<10)+vbv_buffer_size_value), " bytes");
                    Skip_SB(                                    "low_delay");
                    Get_S1 ( 2, frame_rate_extension_n,         "frame_rate_extension_n");
                    Get_S1 ( 5, frame_rate_extension_d,         "frame_rate_extension_d");
                    BS_End();

                    FILLING_BEGIN();
                        if (frame_rate_extension_d)
                            FrameRate=(float)frame_rate_extension_n/frame_rate_extension_d;
                    FILLING_END();
                }
                break;
        case 2 :{ //Sequence Display
                    //Parsing
                    Get_S1 ( 3, video_format,                   "video_format"); Param_Info(Mpegv_video_format[video_format]);
                    TEST_SB_SKIP(                               "load_intra_quantiser_matrix");
                        Skip_S1( 8,                             "colour_primaries");
                        Skip_S1( 8,                             "transfer_characteristics");
                        Skip_S1( 8,                             "matrix_coefficients");
                    TEST_SB_END();
                    Get_S2 (14, display_horizontal_size,        "display_horizontal_size");
                    Mark_1 ();
                    Get_S2 (14, display_vertical_size,          "display_vertical_size");
                    BS_End();
                }
                break;
        case 8 :{ //Picture Coding
                    //Parsing
                    bool progressive_frame;
                    Skip_S1( 4,                                 "f_code_forward_horizontal");
                    Skip_S1( 4,                                 "f_code_forward_vertical");
                    Skip_S1( 4,                                 "f_code_backward_horizontal");
                    Skip_S1( 4,                                 "f_code_backward_vertical");
                    Skip_S1( 2,                                 "intra_dc_precision");
                    Get_S1 ( 2, picture_structure,              "picture_structure"); Param_Info(Mpegv_picture_structure[picture_structure]);
                    Get_SB (    top_field_first,                "top_field_first");
                    Skip_SB(                                    "frame_pred_frame_dct");
                    Skip_SB(                                    "concealment_motion_vectors");
                    Skip_SB(                                    "q_scale_type");
                    Skip_SB(                                    "intra_vlc_format");
                    Skip_SB(                                    "alternate_scan");
                    Get_SB (    repeat_first_field,             "repeat_first_field");
                    Skip_SB(                                    "chroma_420_type");
                    Get_SB (    progressive_frame,              "progressive_frame");
                    TEST_SB_SKIP(                               "composite_display_flag");
                        Skip_SB(                                "v_axis");
                        Skip_S1( 3,                             "field_sequence");
                        Skip_SB(                                "sub_carrier");
                        Skip_S1( 7,                             "burst_amplitude");
                        Skip_S1( 8,                             "sub_carrier_phase");
                    TEST_SB_END();
                    BS_End();

                    FILLING_BEGIN();
                        if (progressive_frame==false)
                        {
                            if (picture_structure==3)           //Frame
                            {
                                if (top_field_first)
                                    Interlaced_Top++;
                                else
                                    Interlaced_Bottom++;
                                FirstFieldFound=false;
                                if (TemporalReference_Offset+temporal_reference>=TemporalReference.size())
                                    TemporalReference.resize(TemporalReference_Offset+temporal_reference+1);
                                TemporalReference[TemporalReference_Offset+temporal_reference]->picture_coding_type=picture_coding_type;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->progressive_frame=progressive_frame;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->picture_structure=picture_structure;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->top_field_first=top_field_first;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->repeat_first_field=repeat_first_field;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->HasPictureCoding=true;
                            }
                            else                                //Field
                            {
                                if (!FirstFieldFound)
                                {
                                    if (picture_structure==1)   //-Top
                                        Interlaced_Top++;
                                    else                        //-Bottom
                                        Interlaced_Bottom++;
                                }
                                FirstFieldFound=!FirstFieldFound;
                            }
                        }
                        else
                        {
                            progressive_frame_Count++;
                            if (top_field_first)
                                Interlaced_Top++;
                            else
                                Interlaced_Bottom++;
                            if (picture_structure==3)           //Frame
                            {
                                if (TemporalReference_Offset+temporal_reference>=TemporalReference.size())
                                    TemporalReference.resize(TemporalReference_Offset+temporal_reference+1);
                                TemporalReference[TemporalReference_Offset+temporal_reference]->picture_coding_type=picture_coding_type;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->progressive_frame=progressive_frame;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->picture_structure=picture_structure;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->top_field_first=top_field_first;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->repeat_first_field=repeat_first_field;
                                TemporalReference[TemporalReference_Offset+temporal_reference]->HasPictureCoding=true;
                            }
                        }

                        if (picture_structure==2) //Bottom, and we want to add a frame only one time if 2 fields
                            Time_End_Frames--; //One frame

                        //CDP
                        #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
                            if (Cdp_Data && !Cdp_Data->empty())
                            {
                                Cdp_IsPresent=true;

                                Element_Begin("CDP");

                                //Parsing
                                #if MEDIAINFO_DEMUX
                                    Element_Code=0x000000B500000000LL;
                                #endif //MEDIAINFO_DEMUX
                                if (Cdp_Parser==NULL)
                                {
                                    Cdp_Parser=new File_Cdp;
                                    Open_Buffer_Init(Cdp_Parser);
                                }
                                Demux((*Cdp_Data)[0]->Data, (*Cdp_Data)[0]->Size, ContentType_MainStream);
                                if (!Cdp_Parser->Status[IsFinished])
                                {
                                    if (Cdp_Parser->PTS_DTS_Needed)
                                        Cdp_Parser->DTS=DTS;
                                    ((File_Cdp*)Cdp_Parser)->AspectRatio=MPEG_Version==1?Mpegv_aspect_ratio1[aspect_ratio_information]:Mpegv_aspect_ratio2[aspect_ratio_information];
                                    Open_Buffer_Continue(Cdp_Parser, (*Cdp_Data)[0]->Data, (*Cdp_Data)[0]->Size);
                                }

                                //Removing data from stack
                                delete (*Cdp_Data)[0]; //Cdp_Data[0]=NULL;
                                Cdp_Data->erase(Cdp_Data->begin());

                                Element_End();
                            }
                        #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)

                        //Active Format Description & Bar Data
                        #if defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)
                            if (AfdBarData_Data && !AfdBarData_Data->empty())
                            {
                                Element_Begin("Active Format Description & Bar Data");

                                //Parsing
                                if (AfdBarData_Parser==NULL)
                                {
                                    AfdBarData_Parser=new File_AfdBarData;
                                    Open_Buffer_Init(AfdBarData_Parser);
                                    ((File_AfdBarData*)AfdBarData_Parser)->Format=File_AfdBarData::Format_S2016_3;
                                }
                                if (AfdBarData_Parser->PTS_DTS_Needed)
                                    AfdBarData_Parser->DTS=DTS;
                                if (!AfdBarData_Parser->Status[IsFinished])
                                    Open_Buffer_Continue(AfdBarData_Parser, (*AfdBarData_Data)[0]->Data, (*AfdBarData_Data)[0]->Size);

                                //Removing data from stack
                                delete (*AfdBarData_Data)[0]; //AfdBarData_Data[0]=NULL;
                                AfdBarData_Data->erase(AfdBarData_Data->begin());

                                Element_End();
                            }
                        #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)
                    FILLING_END();
                }
                break;
        default:{
                    //Parsing
                    Skip_S1(4,                                  "data");
                    BS_End();
                    Skip_XX(Element_Size-Element_Offset,        "data");
                }
    }
}

//---------------------------------------------------------------------------
// Packet "B7"
void File_Mpegv::sequence_end()
{
    Element_Name("sequence_end");

    if (!Status[IsFilled] && sequence_header_IsParsed)
    {
        //End of file, and we have some frames
        Accept("MPEG Video");
        Finish("MPEG Video");
    }
}

//---------------------------------------------------------------------------
// Packet "B8"
void File_Mpegv::group_start()
{
    if (!NextCode_Test())
        return;
    Element_Name("group_start");

    //Reading
    int8u Hours, Minutes, Seconds, Frames;
    bool drop_frame_flag, closed_gop, broken_link;
    BS_Begin();
    Get_SB (    drop_frame_flag,                                "time_code_drop_frame_flag");
    Get_S1 ( 5, Hours,                                          "time_code_time_code_hours");
    Get_S1 ( 6, Minutes,                                        "time_code_time_code_minutes");
    Mark_1();
    Get_S1 ( 6, Seconds,                                        "time_code_time_code_seconds");
    Get_S1 ( 6, Frames,                                         "time_code_time_code_pictures");
    Get_SB (    closed_gop,                                     "closed_gop");
    Get_SB (    broken_link,                                    "broken_link");
    BS_End();
    Ztring Time;
    Time+=Ztring::ToZtring(Hours);
    Time+=_T(':');
    Time+=Ztring::ToZtring(Minutes);
    Time+=_T(':');
    Time+=Ztring::ToZtring(Seconds);
    if (FrameRate!=0)
    {
        Time+=_T('.');
        Time+=Ztring::ToZtring(Frames*1000/FrameRate, 0);
    }
    Element_Info(Time);

    FILLING_BEGIN();
        //NextCode
        NextCode_Clear();
        NextCode_Add(0x00);
        NextCode_Add(0xB2);
        NextCode_Add(0xB5);
        NextCode_Add(0xB8);

        if (TimeCodeIsNotTrustable)
            return;

        //Calculating
        if (Time_Begin_Seconds==Error)
        {
            Time_Begin_Seconds=60*60*Hours+60*Minutes+Seconds;
            Time_Begin_Frames =Frames;
        }
        if (Time_Begin_Seconds==Error)
        {
            //Verifying if time_code is trustable
            if ((size_t)60*60*Hours+60*Minutes+Seconds==Time_Begin_Seconds && Frames==Time_Begin_Frames)
                Time_End_NeedComplete=true; //we can't trust time_code
        }
        if (!Time_End_NeedComplete)
        {
            Time_End_Seconds=60*60*Hours+60*Minutes+Seconds;
            Time_End_Frames =Frames;
        }
        if (!group_start_IsParsed)
        {
            group_start_IsParsed=true;
            group_start_drop_frame_flag=drop_frame_flag;
            group_start_closed_gop=closed_gop;
            group_start_broken_link=broken_link;
        }

        //Autorisation of other streams
        if (Searching_TimeStamp_Start_DoneOneTime)
            Streams[0xB8].Searching_TimeStamp_Start=false; //group_start
        else
            Searching_TimeStamp_Start_DoneOneTime=true;
        Streams[0x00].Searching_TimeStamp_End=true; //picture_start
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_MPEGV_YES

