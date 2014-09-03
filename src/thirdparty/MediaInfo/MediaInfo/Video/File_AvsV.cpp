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
#if defined(MEDIAINFO_AVSV_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_AvsV.h"
#undef FILLING_BEGIN
#define FILLING_BEGIN() \
    while (Element_Offset<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset]==0x00) \
        Element_Offset++; \
    if (Element_Offset!=Element_Size) \
        Trusted_IsNot("Size error"); \
    else if (Element_IsOK()) \
    {
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
Ztring AvsV_profile(int8u profile_id)
{
    switch (profile_id)
    {
        case 0x20 : return "Base";
        default :
            return Ztring::ToZtring(profile_id);
    }
}

//---------------------------------------------------------------------------
Ztring AvsV_level(int8u level_id)
{
    switch (level_id)
    {
        case 0x00 : return Ztring();
        case 0x10 : return "@2.0";
        case 0x20 : return "@4.0";
        case 0x22 : return "@4.2";
        case 0x40 : return "@6.0";
        case 0x42 : return "@6.2";
        default :
            return __T('@')+Ztring::ToZtring(level_id);
    }
}

//---------------------------------------------------------------------------
const float32 AvsV_frame_rate[]=
{
    (float32)0,
    ((float32)24000)/1001,
    (float32)24,
    (float32)25,
    ((float32)30000)/1001,
    (float32)30,
    (float32)50,
    ((float32)60000)/1001,
    (float32)60,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
    (float32)0,
};

//---------------------------------------------------------------------------
const char* AvsV_chroma_format[]=
{
    "",
    "4:2:0",
    "4:2:2",
    "",
};

//---------------------------------------------------------------------------
const char* AvsV_extension_start_code_identifier[]=
{
    "",
    "",
    "sequence_display",
    "",
    "copyright",
    "",
    "",
    "picture_display",
    "",
    "",
    "",
    "camera_parameters",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* AvsV_video_format[]=
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
const float32 AvsV_aspect_ratio[]=
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
const char* AvsV_picture_coding_type[]=
{
    "",
    "P",
    "B",
    "",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_AvsV::File_AvsV()
:File__Analyze()
{
    //Config
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=30;
    FrameIsAlwaysComplete=false;

    //Temp
    video_sequence_start_IsParsed=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_AvsV::Streams_Fill()
{
    //Filling
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "AVS Video");
    Fill(Stream_Video, 0, Video_Codec, "AVS Video");

    //From sequence header
    Fill(Stream_Video, 0, Video_Format_Profile, AvsV_profile(profile_id)+AvsV_level(level_id));
    Fill(Stream_Video, 0, Video_Codec_Profile, AvsV_profile(profile_id)+AvsV_level(level_id));
    Fill(Stream_Video, StreamPos_Last, Video_Width, horizontal_size);
    Fill(Stream_Video, StreamPos_Last, Video_Height, vertical_size);
    Fill(Stream_Video, 0, Video_FrameRate, AvsV_frame_rate[frame_rate_code]/(progressive_sequence?1:2));
    if (aspect_ratio==0)
        ;//Forbidden
    else if (aspect_ratio==1)
            Fill(Stream_Video, 0, Video_PixelAspectRatio, 1.000, 3, true);
    else if (display_horizontal_size && display_vertical_size)
    {
        if (vertical_size && AvsV_aspect_ratio[aspect_ratio])
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, (float)horizontal_size/vertical_size
                                                                         *AvsV_aspect_ratio[aspect_ratio]/((float)display_horizontal_size/display_vertical_size), 3, true);
    }
    else if (AvsV_aspect_ratio[aspect_ratio])
        Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, AvsV_aspect_ratio[aspect_ratio], 3, true);
    Fill(Stream_Video, 0, Video_Colorimetry, AvsV_chroma_format[chroma_format]);
    if (progressive_frame_Count && progressive_frame_Count!=Frame_Count)
    {
        //This is mixed
    }
    else if (Frame_Count>0) //Only if we have at least one progressive_frame definition
    {
        if (progressive_sequence || progressive_frame_Count==Frame_Count)
        {
            Fill(Stream_Video, 0, Video_ScanType, "Progressive");
            Fill(Stream_Video, 0, Video_Interlacement, "PPF");
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
    }
    Fill(Stream_Video, 0, Video_BitRate_Nominal, bit_rate*8);

    //From extensions
    Fill(Stream_Video, 0, Video_Standard, AvsV_video_format[video_format]);

    //Library name
    if (!Library.empty())
    {
        Fill(Stream_Video, 0, Video_Encoded_Library, Library);
        Fill(Stream_Video, 0, Video_Encoded_Library_Name, Library_Name);
        Fill(Stream_Video, 0, Video_Encoded_Library_Version, Library_Version);
        Fill(Stream_Video, 0, Video_Encoded_Library_Date, Library_Date);
    }
}

//---------------------------------------------------------------------------
void File_AvsV::Streams_Finish()
{
    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Streams.clear();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_AvsV::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC3(Buffer+Buffer_Offset)!=0x000001)
        Synched=false;

    //Quick search
    if (Synched && !Header_Parser_QuickSearch())
        return false;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_AvsV::Synched_Init()
{
    //Count of a Packets
    progressive_frame_Count=0;
    Interlaced_Top=0;
    Interlaced_Bottom=0;

    //Temp
    bit_rate=0;
    horizontal_size=0;
    vertical_size=0;
    display_horizontal_size=0;
    display_vertical_size=0;
    profile_id=0;
    level_id=0;
    chroma_format=0;
    aspect_ratio=0;
    frame_rate_code=0;
    video_format=5; //Unspecified video format
    progressive_sequence=false;
    low_delay=false;

    //Default stream values
    Streams.resize(0x100);
    Streams[0xB0].Searching_Payload=true; //video_sequence_start
    for (int8u Pos=0xFF; Pos>=0xB9; Pos--)
        Streams[Pos].Searching_Payload=true; //Testing MPEG-PS
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_AvsV::Header_Parse()
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
bool File_AvsV::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+4<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x00
      &&   Buffer[Buffer_Offset+1]==0x00
      &&   Buffer[Buffer_Offset+2]==0x01)
    {
        //Getting start_code
        int8u start_code=Buffer[Buffer_Offset+3];

        //Searching start or timestamp
        if (Streams[start_code].Searching_Payload)
            return true;

        //Synchronizing
        Buffer_Offset+=4;
        Synched=false;
        if (!Synchronize_0x000001())
        {
            UnSynched_IsNotJunk=true;
            return false;
        }
    }

    if (Buffer_Offset+3==Buffer_Size)
        return false; //Sync is OK, but start_code is not available
    Trusted_IsNot("AVS Video, Synchronisation lost");
    return Synchronize();
}

//---------------------------------------------------------------------------
bool File_AvsV::Header_Parser_Fill_Size()
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
        if (Buffer_Offset_Temp>=Buffer_Size || Buffer[Buffer_Offset_Temp-1]==0x00)
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
void File_AvsV::Data_Parse()
{
    //Parsing
    switch (Element_Code)
    {
        case 0xB0: video_sequence_start(); break;
        case 0xB1: video_sequence_end(); break;
        case 0xB2: user_data_start(); break;
        case 0xB5: extension_start(); break;
        case 0xB3:
        case 0xB6: picture_start(); break;
        case 0xB7: video_edit(); break;
        case 0xB4:
        case 0xB8: reserved(); break;
        default:
            if (Element_Code<=0xAF)
                slice();
            else
            {
                if (Frame_Count==0 && Buffer_TotalBytes>Buffer_TotalBytes_FirstSynched_Max)
                    Trusted=0;
                Trusted_IsNot("Unattended element");

            }
    }

    if (File_Offset+Buffer_Offset+Element_Size==File_Size && Frame_Count>0 && Count_Get(Stream_Video)==0) //Finalize frames in case of there are less than Frame_Count_Valid frames
    {
        //No need of more
        Accept("AVS Video");
        Finish("AVS Video");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
// Packet "00" to "AF"
void File_AvsV::slice()
{
    Element_Name("Slice");

    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");

    FILLING_BEGIN();
        //NextCode
        NextCode_Test();
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B0"
void File_AvsV::video_sequence_start()
{
    Element_Name("video_sequence_start");

    //Parsing
    int32u bit_rate_upper, bit_rate_lower;
    Get_B1 (    profile_id,                                     "profile_id");
    Get_B1 (    level_id,                                       "level_id");
    BS_Begin();
    Get_SB (    progressive_sequence,                           "progressive_sequence");
    Get_S2 (14, horizontal_size,                                "horizontal_size");
    Get_S2 (14, vertical_size,                                  "vertical_size");
    Get_S1 ( 2, chroma_format,                                  "chroma_format");
    Skip_S1( 3,                                                 "sample_precision");
    Get_S1 ( 4, aspect_ratio,                                   "aspect_ratio"); Param_Info1(AvsV_aspect_ratio[aspect_ratio]);
    Get_S1 ( 4, frame_rate_code,                                "frame_rate_code"); Param_Info1(AvsV_frame_rate[frame_rate_code]);
    Get_S3 (18, bit_rate_lower,                                 "bit_rate_lower");
    Mark_1 ();
    Get_S3 (12, bit_rate_upper,                                 "bit_rate_upper");
    bit_rate=(bit_rate_upper<<18)+bit_rate_lower; Param_Info2(bit_rate*8, " bps");
    Get_SB (    low_delay,                                      "low_delay");
    Mark_1 ();
    Skip_S3(18,                                                 "bbv_buffer_size");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    BS_End();

    //Not sure, but the 3 first official files have this
    if (Element_Size-Element_Offset)
    {
        BS_Begin();
        Mark_1();
        BS_End();
    }

    FILLING_BEGIN();
        //NextCode
        NextCode_Clear();
        NextCode_Add(0xB2); //user_data_start
        NextCode_Add(0xB3); //picture_start (I)
        NextCode_Add(0xB5); //extension_start

        //Autorisation of other streams
        Streams[0xB1].Searching_Payload=true, //video_sequence_end
        Streams[0xB2].Searching_Payload=true; //user_data_start
        Streams[0xB3].Searching_Payload=true, //picture_start (I)
        Streams[0xB4].Searching_Payload=true, //reserved
        Streams[0xB5].Searching_Payload=true; //extension_start
        Streams[0xB6].Searching_Payload=true, //picture_start (P or B)
        Streams[0xB7].Searching_Payload=true; //video_edit
        Streams[0xB8].Searching_Payload=true, //reserved

        video_sequence_start_IsParsed=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B1"
void File_AvsV::video_sequence_end()
{
    Element_Name("video_sequence_start");

    FILLING_BEGIN();
        //NextCode
        NextCode_Clear();
        NextCode_Add(0xB0); //SeqenceHeader
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B2", User defined size, this is often used of library name
void File_AvsV::user_data_start()
{
    Element_Name("user_data_start");

    //Rejecting junk from the end
    size_t Library_End_Offset=(size_t)Element_Size;
    while (Library_End_Offset>0
        && (Buffer[Buffer_Offset+Library_End_Offset-1]<0x20
         || Buffer[Buffer_Offset+Library_End_Offset-1]>0x7D
         || (Buffer[Buffer_Offset+Library_End_Offset-1]>=0x3A
          && Buffer[Buffer_Offset+Library_End_Offset-1]<=0x40)))
        Library_End_Offset--;
    if (Library_End_Offset==0)
        return; //No good info

    //Accepting good data after junk
    size_t Library_Start_Offset=Library_End_Offset-1;
    while (Library_Start_Offset>0 && (Buffer[Buffer_Offset+Library_Start_Offset-1]>=0x20 && Buffer[Buffer_Offset+Library_Start_Offset-1]<=0x7D))
        Library_Start_Offset--;

    //But don't accept non-alpha caracters at the beginning (except for "3ivx")
    if (Library_End_Offset-Library_Start_Offset!=4 || CC4(Buffer+Buffer_Offset+Library_Start_Offset)!=0x33697678) //3ivx
        while (Library_Start_Offset<Element_Size && Buffer[Buffer_Offset+Library_Start_Offset]<=0x40)
            Library_Start_Offset++;

    //Parsing
    Ztring Temp;
    if (Library_Start_Offset>0)
        Skip_XX(Library_Start_Offset,                           "junk");
    if (Library_End_Offset-Library_Start_Offset)
        Get_Local(Library_End_Offset-Library_Start_Offset, Temp,"data");
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "junk");

    FILLING_BEGIN();
        //NextCode
        NextCode_Test();

        if (Temp.size()>=4)
            Library=Temp;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B5"
void File_AvsV::extension_start()
{
    Element_Name("Extension");

    //Parsing
    int8u extension_start_code_identifier;
    BS_Begin();
    Get_S1 ( 4, extension_start_code_identifier,                "extension_start_code_identifier"); Param_Info1(AvsV_extension_start_code_identifier[extension_start_code_identifier]);
    Element_Info1(AvsV_extension_start_code_identifier[extension_start_code_identifier]);

    switch (extension_start_code_identifier)
    {
        case 2  : //sequence_display
                {
                    //Parsing
                    Get_S1 ( 3, video_format,                   "video_format"); Param_Info1(AvsV_video_format[video_format]);
                    Skip_SB(                                    "sample_range");
                    TEST_SB_SKIP(                               "colour_description");
                        Skip_S1( 8,                             "colour_primaries");
                        Skip_S1( 8,                             "transfer_characteristics");
                        Skip_S1( 8,                             "matrix_coefficients");
                    TEST_SB_END();
                    Get_S2 (14, display_horizontal_size,        "display_horizontal_size");
                    Mark_1 ();
                    Get_S2 (14, display_vertical_size,          "display_vertical_size");
                    Skip_SB(                                    "reserved");
                    Skip_SB(                                    "reserved");
                    BS_End();
                }
                break;
        case 4  : //copyright
                {
                    //Parsing
                    Skip_SB(                                    "copyright_flag");
                    Skip_S1( 8,                                 "copyright_id");
                    Skip_SB(                                    "original_or_copy");
                    Skip_S1( 7,                                 "reserved");
                    Mark_1 ();
                    Info_S3(20, copyright_number_1,             "copyright_number_1");
                    Mark_1 ();
                    Info_S3(22, copyright_number_2,             "copyright_number_2");
                    Mark_1 ();
                    Info_S3(22, copyright_number_3,             "copyright_number_3"); Param_Info1(Ztring::ToZtring(((int64u)copyright_number_1<<44)+((int64u)copyright_number_2<<22)+(int64u)copyright_number_3, 16));
                    BS_End();
                }
                break;
        case 11 : //camera_parameters
                {
                    //Parsing
                    Skip_SB(                                    "reserved");
                    Skip_S1( 7,                                 "camera_id");
                    Mark_1 ();
                    Skip_S3(22,                                 "height_of_image_device");
                    Mark_1 ();
                    Skip_S3(22,                                 "focal_length");
                    Mark_1 ();
                    Skip_S3(22,                                 "f_number");
                    Mark_1 ();
                    Skip_S3(22,                                 "vertical_angle_of_view");
                    Mark_1 ();
                    Skip_S3(16,                                 "camera_position_x_upper");
                    Mark_1 ();
                    Skip_S3(16,                                 "camera_position_x_lower");
                    Mark_1 ();
                    Skip_S3(16,                                 "camera_position_y_upper");
                    Mark_1 ();
                    Skip_S3(16,                                 "camera_position_y_lower");
                    Mark_1 ();
                    Skip_S3(16,                                 "camera_position_z_upper");
                    Mark_1 ();
                    Skip_S3(16,                                 "camera_position_z_lower");
                    Mark_1 ();
                    Skip_S3(22,                                 "camera_direction_x");
                    Mark_1 ();
                    Skip_S3(22,                                 "camera_direction_y");
                    Mark_1 ();
                    Skip_S3(22,                                 "camera_direction_z");
                    Mark_1 ();
                    Skip_S3(22,                                 "camera_plane_vertical_x");
                    Mark_1 ();
                    Skip_S3(22,                                 "camera_plane_vertical_y");
                    Mark_1 ();
                    Skip_S3(22,                                 "camera_plane_vertical_z");
                    Mark_1 ();
                    Skip_S4(32,                                 "reserved");
                    BS_End();
                }
                break;
        default :
                {
                    //Parsing
                    Skip_S1(4,                                  "data");
                    BS_End();
                    Skip_XX(Element_Size-Element_Offset,        "data");
                }
    }

    //Not sure, but the 3 first official files have this
    if (Element_Size-Element_Offset)
    {
        BS_Begin();
        Mark_1();
        BS_End();
    }

    FILLING_BEGIN();
        //NextCode
        NextCode_Test();
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B3" or "B6"
void File_AvsV::picture_start()
{
    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finalize frames in case of there are less than Frame_Count_Valid frames
    Frame_Count++;

    //Name
    Element_Name("picture_start");
    Element_Info1(Ztring::ToZtring(Frame_Count));
    Element_Info1C((Element_Code==0xB3), __T("I"));

    //Parsing
    int8u picture_coding_type=(int8u)-1;
    bool time_code_flag, progressive_frame, picture_structure=true, top_field_first, repeat_first_field, skip_mode_flag=false, loop_filter_disable;
    Skip_B2(                                                    "bbv_delay");
    BS_Begin();
    if (Element_Code==0xB3) //Only I
    {
        Get_SB (    time_code_flag,                             "time_code_flag");
        if (time_code_flag)
        {
            Skip_SB(                                            "time_code_dropframe");
            Skip_S1(5,                                          "time_code_hours");
            Skip_S1(6,                                          "time_code_minutes");
            Skip_S1(6,                                          "time_code_seconds");
            Skip_S1(6,                                          "time_code_pictures");
        }
    }
    if (Element_Code==0xB6) //Only P or B
    {
        Get_S1 ( 2, picture_coding_type,                        "picture_coding_type"); Element_Info1(AvsV_picture_coding_type[picture_coding_type]);
    }
    Skip_S1( 8,                                                 "picture_distance");
    if (low_delay)
        Skip_UE(                                                "bbv_check_times");
    Get_SB (    progressive_frame,                              "progressive_frame");
    if (!progressive_frame)
    {
        Get_SB(    picture_structure,                           "picture_structure");
        if (Element_Code==0xB6) //Only P or B
        {
            if (picture_structure)
                Skip_SB(                                        "advanced_pred_mode_disable");
        }
    }
    Get_SB (    top_field_first,                                "top_field_first");
    Get_SB (    repeat_first_field,                             "repeat_first_field");
    Skip_SB(                                                    "fixed_picture_qp");
    Skip_S1( 6,                                                 "picture_qp");
    if (Element_Code==0xB3) //Only I
    {
        if (!progressive_frame && !picture_structure)
            Get_SB(    skip_mode_flag,                          "skip_mode_flag");
    }
    if (Element_Code==0xB6) //Only P or B
    {
        if (picture_coding_type!=2 || !picture_structure)
            Skip_SB(                                            "picture_reference_flag");
    }
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    if (Element_Code==0xB6) //Only P or B
    {
        Get_SB(    skip_mode_flag,                              "skip_mode_flag");
    }
    Get_SB (    loop_filter_disable,                            "loop_filter_disable");
    if (!loop_filter_disable)
    {
        bool loop_filter_parameter_flag;
        Get_SB (    loop_filter_parameter_flag,                 "loop_filter_parameter_flag");
        if (loop_filter_parameter_flag)
        {
            Skip_SE(                                            "alpha_c_offset");
            Skip_SE(                                            "beta_offset");
        }
    }
    BS_End();

    if (Element_Size-Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");

    FILLING_BEGIN();
        if (progressive_frame==false)
        {
            if (picture_structure==true)           //Frame
            {
                if (top_field_first)
                    Interlaced_Top++;
                else
                    Interlaced_Bottom++;
            }
        }
        else
            progressive_frame_Count++;

        //NextCode
        NextCode_Test();
        NextCode_Clear();
        for (int8u Pos=0x00; Pos<=0xAF; Pos++)
            NextCode_Add(Pos); //slice
        NextCode_Add(0xB0); //video_sequence_start
        NextCode_Add(0xB3); //picture_start
        NextCode_Add(0xB6); //picture_start

        //Autorisation of other streams
        for (int8u Pos=0x00; Pos<=0xAF; Pos++)
            Streams[Pos].Searching_Payload=true; //slice

        //Filling only if not already done
        if (Frame_Count>=Frame_Count_Valid && Count_Get(Stream_Video)==0)
        {
            //No need of more
            Accept("AVS Video");
            Finish("AVS Video");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "B7"
void File_AvsV::video_edit()
{
    Element_Name("video_edit");
}

//---------------------------------------------------------------------------
// Packet "B4" and "B8"
void File_AvsV::reserved()
{
    Element_Name("reserved");

    //Parsing
    if (Element_Size)
        Skip_XX(Element_Size,                                   "reserved");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AVSV_*
