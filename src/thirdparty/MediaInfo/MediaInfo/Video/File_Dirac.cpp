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
#if defined(MEDIAINFO_DIRAC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Dirac.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char* Dirac_base_video_format(intu base_video_format)
{
    switch (base_video_format)
    {
        case   1 : return "QSIF525";
        case   2 : return "QCIF";
        case   3 : return "SIF525";
        case   4 : return "CIF";
        case   5 : return "4SIF525";
        case   6 : return "4CIF";
        case   7 : return "480i60";
        case   8 : return "576i50";
        case   9 : return "720p50";
        case  10 : return "720p60";
        case  11 : return "1080i60";
        case  12 : return "1080i50";
        case  13 : return "1080p60";
        case  14 : return "1080p60";
        case  15 : return "2K-24";
        case  16 : return "4K-24";
        case  17 : return "4K-60";
        case  18 : return "4K-50";
        case  19 : return "8K-60";
        case  20 : return "8K-50";
        default  : return "";
    }
}

//---------------------------------------------------------------------------
float32 Dirac_frame_rate(int32u frame_rate_index)
{
    switch (frame_rate_index)
    {
        case  0 : return (float32)0; //Reserved
        case  1 : return (float32)24000/(float32)1001;
        case  2 : return (float32)24;
        case  3 : return (float32)25;
        case  4 : return (float32)30000/(float32)1001;
        case  5 : return (float32)30;
        case  6 : return (float32)50;
        case  7 : return (float32)60000/(float32)1001;
        case  8 : return (float32)60;
        case  9 : return (float32)15000/(float32)1001;
        case 10 : return (float32)12.5;
        default : return (float32)0; //Unknown
    }
}

//---------------------------------------------------------------------------
float32 Dirac_pixel_aspect_ratio(int32u pixel_aspect_ratio_index)
{
    switch (pixel_aspect_ratio_index)
    {
        case  0 : return (float32)0; //Reserved
        case  1 : return (float32)1; //Reserved
        case  2 : return (float32)10/(float32)11;
        case  3 : return (float32)12/(float32)11;
        case  4 : return (float32)40/(float32)33;
        case  5 : return (float32)16/(float32)11;
        case  6 : return (float32) 4/(float32) 3;
        default : return (float32)0; //Unknown
    }
}

//---------------------------------------------------------------------------
const char* Dirac_picture_coding_mode(int32u picture_coding_mode)
{
    switch (picture_coding_mode)
    {
        case 0 : return "PPF";
        case 1 : return "Interlaced";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* Dirac_source_sampling(int32u source_sampling)
{
    switch (source_sampling)
    {
        case 0 : return "Progressive";
        case 1 : return "Interlaced";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* Dirac_source_sampling_Codec(int32u source_sampling)
{
    switch (source_sampling)
    {
        case 0 : return "PPF";
        case 1 : return "Interlaced";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* Dirac_chroma_format(int32u chroma_format)
{
    switch (chroma_format)
    {
        case 0 : return "4:4:4";
        case 1 : return "4:2:2";
        case 2 : return "4:2:0";
        default: return "";
    }
}

//---------------------------------------------------------------------------
void Dirac_base_video_format(int32u   base_video_format,
                             int32u  &frame_width,
                             int32u  &frame_height,
                             int32u  &chroma_format,
                             int32u  &source_sampling,
                             int32u  &clean_width,
                             int32u  &clean_height,
                             int32u  &clean_left_offset,
                             int32u  &clean_top_offset,
                             float32 &frame_rate,
                             float32 &pixel_aspect_ratio)
{
    switch (base_video_format)
    {
        case   0 :  frame_width=640;
                    frame_height=480;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=640;
                    clean_height=480;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(1);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case   1 :  frame_width=176;
                    frame_height=120;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=176;
                    clean_height=144;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(9);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(2);
                    return;
        case   2 :  frame_width=176;
                    frame_height=144;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=176;
                    clean_height=144;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(10);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(3);
                    return;
        case   3 :  frame_width=352;
                    frame_height=240;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=352;
                    clean_height=240;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(9);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(2);
                    return;
        case   4 :  frame_width=352;
                    frame_height=288;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=352;
                    clean_height=288;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(10);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(3);
                    return;
        case   5 :  frame_width=704;
                    frame_height=480;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=704;
                    clean_height=480;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(9);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(2);
                    return;
        case   6 :  frame_width=704;
                    frame_height=576;
                    chroma_format=2;
                    source_sampling=0;
                    clean_width=704;
                    clean_height=576;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(10);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(3);
                    return;
        case   7 :  frame_width=720;
                    frame_height=480;
                    chroma_format=1;
                    source_sampling=1;
                    clean_width=704;
                    clean_height=480;
                    clean_left_offset=8;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(4);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(2);
                    return;
        case   8 :  frame_width=720;
                    frame_height=576;
                    chroma_format=1;
                    source_sampling=1;
                    clean_width=704;
                    clean_height=576;
                    clean_left_offset=8;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(3);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(3);
                    return;
        case   9 :  frame_width=1280;
                    frame_height=720;
                    chroma_format=1;
                    source_sampling=0;
                    clean_width=1280;
                    clean_height=720;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(7);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  10 :  frame_width=1280;
                    frame_height=720;
                    chroma_format=1;
                    source_sampling=0;
                    clean_width=1280;
                    clean_height=720;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(6);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  11 :  frame_width=1920;
                    frame_height=1080;
                    chroma_format=1;
                    source_sampling=1;
                    clean_width=1920;
                    clean_height=1080;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(4);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  12 :  frame_width=1920;
                    frame_height=1080;
                    chroma_format=1;
                    source_sampling=1;
                    clean_width=1920;
                    clean_height=1080;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(3);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  13 :  frame_width=1920;
                    frame_height=1080;
                    chroma_format=1;
                    source_sampling=0;
                    clean_width=1920;
                    clean_height=1080;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(7);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  14 :  frame_width=1920;
                    frame_height=1080;
                    chroma_format=1;
                    source_sampling=0;
                    clean_width=1920;
                    clean_height=1080;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(6);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  15 :  frame_width=2048;
                    frame_height=1080;
                    chroma_format=0;
                    source_sampling=0;
                    clean_width=2048;
                    clean_height=1080;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(2);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  16 :  frame_width=4096;
                    frame_height=2160;
                    chroma_format=0;
                    source_sampling=0;
                    clean_width=4096;
                    clean_height=2160;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(2);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  17 :  frame_width=3840;
                    frame_height=2160;
                    chroma_format=0;
                    source_sampling=0;
                    clean_width=3840;
                    clean_height=2160;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(7);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  18 :  frame_width=3840;
                    frame_height=2160;
                    chroma_format=0;
                    source_sampling=0;
                    clean_width=3840;
                    clean_height=2160;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(6);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  19 :  frame_width=7680;
                    frame_height=4320;
                    chroma_format=0;
                    source_sampling=0;
                    clean_width=7680;
                    clean_height=4320;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(7);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        case  20 :  frame_width=7680;
                    frame_height=4320;
                    chroma_format=0;
                    source_sampling=0;
                    clean_width=7680;
                    clean_height=4320;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate(6);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio(1);
                    return;
        default  :  frame_width=0;
                    frame_height=0;
                    chroma_format=(int32u)-1;
                    source_sampling=(int32u)-1;
                    clean_width=0;
                    clean_height=0;
                    clean_left_offset=0;
                    clean_top_offset=0;
                    frame_rate=Dirac_frame_rate((int32u)-1);
                    pixel_aspect_ratio=Dirac_pixel_aspect_ratio((int32u)-1);
                    return;
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dirac::File_Dirac()
:File__Analyze()
{
    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=1;
    Ignore_End_of_Sequence=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dirac::Streams_Fill()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "Dirac");
    Fill(Stream_Video, 0, Video_Codec, "Dirac");

    if (clean_width)
        Fill(Stream_Video, StreamPos_Last, Video_Width, clean_width);
    if (clean_height)
        Fill(Stream_Video, StreamPos_Last, Video_Height, clean_height);
    if (pixel_aspect_ratio)
    {
        Fill(Stream_Video, 0, Video_PixelAspectRatio, pixel_aspect_ratio, 3, true);
        if (clean_height!=0)
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, ((float)clean_width)/clean_height*pixel_aspect_ratio, 3, true);
    }
    if (frame_rate)
        Fill(Stream_Video, StreamPos_Last, Video_FrameRate, frame_rate);
    Fill(Stream_Video, 0, Video_Colorimetry, Dirac_chroma_format(chroma_format));
    Fill(Stream_Video, 0, Video_ScanType, Dirac_source_sampling(source_sampling));
    Fill(Stream_Video, 0, Video_Interlacement, Dirac_source_sampling_Codec(source_sampling));
}

//---------------------------------------------------------------------------
void File_Dirac::Streams_Finish()
{
    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Streams.clear();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dirac::Synchronize()
{
    //Synchronizing
    while(Buffer_Offset+4<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x42
                                        || Buffer[Buffer_Offset+1]!=0x42
                                        || Buffer[Buffer_Offset+2]!=0x43
                                        || Buffer[Buffer_Offset+3]!=0x44)) //"BBCD"
    {
        Buffer_Offset+=2;
        while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x42)
            Buffer_Offset+=2;
        if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x42)
            Buffer_Offset--;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+4>Buffer_Size)
    {
        if (Buffer_Offset+3==Buffer_Size && CC3(Buffer+Buffer_Offset)!=0x424243)    //"BBC"
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x4242)      //"BB"
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x42)        //"B"
            Buffer_Offset++;
        return false;
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Dirac::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+4>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC4(Buffer+Buffer_Offset)!=0x42424344) //"BBCD"
        Synched=false;

    //Quick search
    if (Synched && !Header_Parser_QuickSearch())
        return false;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Dirac::Synched_Init()
{
    //Temp
    Dirac_base_video_format((int32u)-1, frame_width, frame_height, chroma_format, source_sampling,
                            clean_width, clean_height, clean_left_offset, clean_top_offset,
                            frame_rate, pixel_aspect_ratio);

    //Default stream values
    Streams.resize(0x100);
    Streams[0x00].Searching_Payload=true; //Sequence header
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dirac::Header_Parse()
{
    //Parsing
    int32u Next_Parse_Offset, Previous_Parse_Offset;
    int8u  Parse_Code;
    Skip_C4(                                                    "Parse Info Prefix");
    Get_B1 (Parse_Code,                                         "Parse Code");
    Get_B4 (Next_Parse_Offset,                                  "Next Parse Offset");
    Get_B4 (Previous_Parse_Offset,                              "Previous Parse Offset");

    //Filling
    Header_Fill_Code(Parse_Code, Ztring().From_CC1(Parse_Code));
    Header_Fill_Size((Parse_Code==0x10 && Next_Parse_Offset==0)?13:Next_Parse_Offset); //Speacial case if this is the End Of Sequence
}

//---------------------------------------------------------------------------
bool File_Dirac::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+5<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x42
      &&   Buffer[Buffer_Offset+1]==0x42
      &&   Buffer[Buffer_Offset+2]==0x43
      &&   Buffer[Buffer_Offset+3]==0x44) //"BBCD"
    {
        //Getting start_code
        int8u start_code=CC1(Buffer+Buffer_Offset+4);

        //Searching start
        if (Streams[start_code].Searching_Payload)
            return true;

        //Getting size
        Buffer_Offset+=BigEndian2int32u(Buffer+Buffer_Offset+5);
    }

    if (Buffer_Offset+4==Buffer_Size)
        return false; //Sync is OK, but start_code is not available
    if (Buffer_Offset+5<=Buffer_Size)
        Trusted_IsNot("Dirac, Synchronisation lost");
    Synched=false;
    return Synchronize();
}

//---------------------------------------------------------------------------
void File_Dirac::Data_Parse()
{
    //Parsing
    switch (Element_Code)
    {
        case 0x00 : Sequence_header(); break;
        case 0x10 : End_of_Sequence(); break;
        case 0x20 : Auxiliary_data(); break;
        case 0x30 : Padding_data(); break;
        case 0x0C : Intra_Reference_Picture(); break;
        case 0x08 : Intra_Non_Reference_Picture(); break;
        case 0x4C : Intra_Reference_Picture_No(); break;
        case 0x48 : Intra_Non_Reference_Picture_No(); break;
        case 0x0D : Inter_Reference_Picture_1(); break;
        case 0x0E : Inter_Reference_Picture_2(); break;
        case 0x09 : Inter_Non_Reference_Picture_1(); break;
        case 0x0A : Inter_Non_Reference_Picture_2(); break;
        case 0xCC : Reference_Picture_Low(); break;
        case 0xC8 : Intra_Non_Reference_Picture_Low(); break;
        default   : Reserved();
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
// Packet "00"
void File_Dirac::Sequence_header()
{
    Element_Name("Sequence header");

    //Parsing
    int32u version_major, version_minor, profile, level, base_video_format;
    BS_Begin();
    Get_UI(version_major,                                       "version major");
    Get_UI(version_minor,                                       "version minor");
    Get_UI(profile,                                             "profile");
    Get_UI(level,                                               "level");

    if (version_major<=2)
    {
        Get_UI(base_video_format,                               "base video format"); //Param_Info1(Dirac_base_video_format(base_video_format));
        Dirac_base_video_format(base_video_format, frame_width, frame_height, chroma_format, source_sampling,
                                clean_width, clean_height, clean_left_offset, clean_top_offset,
                                frame_rate, pixel_aspect_ratio);
        TEST_SB_SKIP(                                           "custom dimensions flag");
            Get_UI (frame_width,                                "frame width");
            Get_UI (frame_height,                               "frame height");
        TEST_SB_END();
        TEST_SB_SKIP(                                           "custom chroma format flag");
            Get_UI (chroma_format,                              "chroma format"); Param_Info1(Dirac_chroma_format(chroma_format));
        TEST_SB_END();
        TEST_SB_SKIP(                                           "custom scan format flag");
            Get_UI (source_sampling,                            "source sampling"); Param_Info1(Dirac_source_sampling(source_sampling));
        TEST_SB_END();
        TEST_SB_SKIP(                                           "frame rate flag");
            int32u frame_rate_index;
            Get_UI (frame_rate_index,                           "index"); Param_Info1(Dirac_frame_rate(frame_rate_index));
            if (frame_rate_index==0)
            {
                int32u frame_rate_numer, frame_rate_denom;
                Get_UI (frame_rate_numer,                       "frame rate numer");
                Get_UI (frame_rate_denom,                       "frame rate denom");
                frame_rate=((float32)frame_rate_numer)/((float32)frame_rate_denom);
            }
            else
                frame_rate=Dirac_frame_rate(frame_rate_index);
        TEST_SB_END();
        TEST_SB_SKIP(                                           "pixel aspect ratio flag");
            int32u pixel_aspect_ratio_index;
            Get_UI (pixel_aspect_ratio_index,                   "index"); Param_Info1(Dirac_pixel_aspect_ratio(pixel_aspect_ratio_index));
            if (pixel_aspect_ratio_index==0)
            {
                int32u pixel_aspect_ratio_numer, pixel_aspect_ratio_denom;
                Get_UI (pixel_aspect_ratio_numer,               "pixel aspect ratio numer");
                Get_UI (pixel_aspect_ratio_denom,               "pixel aspect ratio denom");
                pixel_aspect_ratio=((float32)pixel_aspect_ratio_numer)/((float32)pixel_aspect_ratio_denom);
            }
            else
                pixel_aspect_ratio=Dirac_pixel_aspect_ratio(pixel_aspect_ratio_index);
        TEST_SB_END();
        TESTELSE_SB_SKIP(                                       "custom clean area flag");
            Get_UI (clean_width,                                "clean width");
            Get_UI (clean_height,                               "clean height");
            Get_UI (clean_left_offset,                          "clean left offset");
            Get_UI (clean_top_offset,                           "clean top offset");
        TESTELSE_SB_ELSE(                                       "custom clean area flag");
            clean_width=frame_width;
            clean_height=frame_height;
        TESTELSE_SB_END();
        TEST_SB_SKIP(                                           "custom signal range flag");
            int32u custom_signal_range_index;
            Get_UI(custom_signal_range_index,                   "index");
            if (custom_signal_range_index==0)
            {
                Skip_UI(                                        "luma offset");
                Skip_UI(                                        "luma excursion");
                Skip_UI(                                        "chroma offset");
                Skip_UI(                                        "chroma excursion");
            }
        TEST_SB_END();
        TEST_SB_SKIP(                                           "custom colour spec flag");
            int32u custom_colour_spec_index;
            Get_UI(custom_colour_spec_index,                    "index");
            if (custom_colour_spec_index==0)
            {
                TEST_SB_SKIP(                                   "custom colour primaries flag");
                    Skip_UI(                                    "custom colour primaries index");
                TEST_SB_END();
                TEST_SB_SKIP(                                   "colour matrix flag");
                    Skip_UI(                                    "colour matrix index");
                TEST_SB_END();
                TEST_SB_SKIP(                                   "custom transfer function flag");
                    Skip_UI(                                    "custom transfer function index");
                TEST_SB_END();
            }
        TEST_SB_END();
        Info_UI(picture_coding_mode,                            "picture coding mode"); Param_Info1(Dirac_picture_coding_mode(picture_coding_mode));
    }
    else
    {
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");
    }

    FILLING_BEGIN();
        //Autorisation of other streams
        Streams[0x10].Searching_Payload=true; //End_of_Sequence
        Streams[0x20].Searching_Payload=true; //Auxiliary_data
        Streams[0x30].Searching_Payload=true; //Padding_data
        Streams[0x0C].Searching_Payload=true; //Intra_Reference_Picture
        Streams[0x08].Searching_Payload=true; //Intra_Non_Reference_Picture
        Streams[0x4C].Searching_Payload=true; //Intra_Reference_Picture_No
        Streams[0x48].Searching_Payload=true; //Intra_Non_Reference_Picture_No
        Streams[0x0D].Searching_Payload=true; //Inter_Reference_Picture_1
        Streams[0x0E].Searching_Payload=true; //Inter_Reference_Picture_2
        Streams[0x09].Searching_Payload=true; //Inter_Non_Reference_Picture_1
        Streams[0x0A].Searching_Payload=true; //Inter_Non_Reference_Picture_2
        Streams[0xCC].Searching_Payload=true; //Reference_Picture_Low
        Streams[0xC8].Searching_Payload=true; //Intra_Non_Reference_Picture_Low
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "10"
void File_Dirac::End_of_Sequence()
{
    Element_Name("End of Sequence");

    //Parsing
    if (!Ignore_End_of_Sequence)
    {
        NextCode_Clear();
        Accept("Dirac");
        Finish("Dirac");
    }
}

//---------------------------------------------------------------------------
// Packet "20"
void File_Dirac::Auxiliary_data()
{
    Element_Name("Auxiliary data");

    //Parsing
    Skip_XX(Element_Size,                                       "Auxiliary data");
}

//---------------------------------------------------------------------------
// Packet "30"
void File_Dirac::Padding_data()
{
    Element_Name("Padding data");

    //Parsing
    Skip_XX(Element_Size,                                       "Padding data");
}

//---------------------------------------------------------------------------
// Packet "0C"
void File_Dirac::Intra_Reference_Picture()
{
    Element_Name("Intra Reference Picture");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "08"
void File_Dirac::Intra_Non_Reference_Picture()
{
    Element_Name("Intra Non Reference Picture");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "4C"
void File_Dirac::Intra_Reference_Picture_No()
{
    Element_Name("Intra Reference Picture (no arithmetic coding)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "48"
void File_Dirac::Intra_Non_Reference_Picture_No()
{
    Element_Name("Intra Non Reference Picture (no arithmetic coding)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "0D"
void File_Dirac::Inter_Reference_Picture_1()
{
    Element_Name("Inter Reference Picture (1 picture)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "0E"
void File_Dirac::Inter_Reference_Picture_2()
{
    Element_Name("Inter Reference Picture (2 pictures)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "09"
void File_Dirac::Inter_Non_Reference_Picture_1()
{
    Element_Name("Inter Non Reference Picture (1 picture)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "0A"
void File_Dirac::Inter_Non_Reference_Picture_2()
{
    Element_Name("Inter Non Reference Picture (2 pictures)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "CC"
void File_Dirac::Reference_Picture_Low()
{
    Element_Name("Reference Picture (low-delay)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
// Packet "C8"
void File_Dirac::Intra_Non_Reference_Picture_Low()
{
    Element_Name("Intra Non Reference Picture (low-delay)");

    //Parsing
    picture();
}

//---------------------------------------------------------------------------
void File_Dirac::Reserved()
{
    Element_Name("Reserved");

    Skip_XX(Element_Size,                                       "Unknown");
}

//---------------------------------------------------------------------------
void File_Dirac::picture()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        //Counting
        if (File_Offset+Buffer_Offset+Element_Size==File_Size)
            Frame_Count_Valid=Frame_Count; //Finalize frames in case of there are less than Frame_Count_Valid frames

        //Name
        Element_Info1(Ztring::ToZtring(Frame_Count));

        //Filling only if not already done
        Frame_Count++;
        Frame_Count_InThisBlock++;
        if (Frame_Count>=Frame_Count_Valid && Count_Get(Stream_Video)==0)
        {
            NextCode_Clear();
            Accept("Dirac");
            Finish("Dirac");
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_DIRAC_YES
