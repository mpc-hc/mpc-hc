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
#if defined(MEDIAINFO_ARIBSTDB24B37_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_AribStdB24B37.h"
#if defined(MEDIAINFO_MPEGTS_YES)
    #include "MediaInfo/Multiple/File_MpegTs.h"
#endif
#include <vector>
#ifdef __WINDOWS__
    #undef __TEXT
    #include "windows.h"
#endif // __WINDOWS__
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
const char* AribStdB24B37_Caption_conversion_type(int8u Caption_conversion_type)
{
    switch (Caption_conversion_type)
    {
        case 0 : return "Analog";
        case 1 : return "HD side panel";
        case 2 : return "SD (4:3)";
        case 3 : return "SD wide side panel";
        case 4 : return "Mobile closed caption";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_Caption_DataIdentifier(int8u DataIdentifier)
{
    switch (DataIdentifier)
    {
        case 0 : return "Exchange format data (closed caption data label)";
        case 1 : return "Exchange format data (program management information)";
        case 2 : return "Exchange format data (page information 1)";
        case 3 : return "Exchange format data (page information 2)";
        case 4 : return "Short form data (closed caption management data)";
        case 5 : return "Short form data (closed caption text data)";
        case 6 : return "Undefined";
        case 7 : return "Dummy data";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_DRCS_conversion_type(int8u DRCS_conversion_type)
{
    switch (DRCS_conversion_type)
    {
        case 0 : return "DRCS conversion mode A";
        case 1 : return "DRCS conversion mode B";
        case 2 : return "Mobile DRCS";
        case 3 : return "DRCS conversion not possible";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_data_group_id(int8u data_group_id)
{
    switch (data_group_id)
    {
        case 0 : return "Caption management";
        case 1 : return "Caption statement (1st)";
        case 2 : return "Caption statement (2nd)";
        case 3 : return "Caption statement (3rd)";
        case 4 : return "Caption statement (4th)";
        case 5 : return "Caption statement (5th)";
        case 6 : return "Caption statement (6th)";
        case 7 : return "Caption statement (7th)";
        case 8 : return "Caption statement (8th)";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_TMD(int8u TMD)
{
    switch (TMD)
    {
        case 0 : return "Free";
        case 1 : return "Real time";
        case 2 : return "Offset time";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_DMF_reception(int8u DMF_reception)
{
    switch (DMF_reception)
    {
        case 0 : return "Automatic display when received";
        case 1 : return "Non-displayed automatically when received";
        case 2 : return "Selectable display when received";
        case 3 : return "Automatic display/non-display under specific condition when received";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_DMF_recording(int8u DMF_recording)
{
    switch (DMF_recording)
    {
        case 0 : return "Automatic display when recording and playback";
        case 1 : return "Non- displayed automatically when recording and playback";
        case 2 : return "Selectable display when recording and playback";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_format(int8u format)
{
    switch (format)
    {
        case  0 : return "Horizontal writing in standard density";
        case  1 : return "Vertical writing in standard density";
        case  2 : return "Horizontal writing in high density";
        case  3 : return "Vertical writing in high density";
        case  4 : return "Horizontal writing of Western language";
        case  5 : return "Horizontal writing in 1920 x 1080";
        case  6 : return "Vertical writing in 1920 x 1080";
        case  7 : return "Horizontal writing in 960 x 540";
        case  8 : return "Vertical writing in 960 x 540";
        case  9 : return "Horizontal writing in 1280 x 720";
        case 10 : return "Vertical writing in 1280 x 720";
        case 11 : return "Horizontal writing in 720 x 480";
        case 12 : return "Vertical writing in 720 x 480";
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_TCS(int8u TCS)
{
    switch (TCS)
    {
        case  0 : return "8-bit character codes";
        case  1 : return "UCS";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_rollup_mode(int8u rollup_mode)
{
    switch (rollup_mode)
    {
        case 0 : return "Non-roll up";
        case 1 : return "Roll up";
        default: return "";
    }
}

//---------------------------------------------------------------------------
const char* AribStdB24B37_data_unit_parameter(int8u data_unit_parameter)
{
    switch (data_unit_parameter)
    {
        case 0x20 : return "Texts";
        case 0x28 : return "Geometric graphics";
        case 0x2C : return "Synthesized sound";
        case 0x30 : return "1 byte DRCS";
        case 0x31 : return "2 byte DRCS";
        case 0x34 : return "color map";
        case 0x35 : return "Bit map";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
// Table 7-18 Default macro code strings
static const int8u AribStdB24B37_DefaultMacros[][19] =
{
    { 0x1B, 0x24, 0x42, 0x1B, 0x29, 0x4A, 0x1B, 0x2A, 0x30, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x24, 0x42, 0x1B, 0x29, 0x31, 0x1B, 0x2A, 0x30, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x24, 0x42, 0x1B, 0x29, 0x20, 0x41, 0x1B, 0x2A, 0x30, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x32, 0x1B, 0x29, 0x34, 0x1B, 0x2A, 0x35, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x32, 0x1B, 0x29, 0x33, 0x1B, 0x2A, 0x35, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x32, 0x1B, 0x29, 0x20, 0x41, 0x1B, 0x2A, 0x35, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x20, 0x41, 0x1B, 0x29, 0x20, 0x42, 0x1B, 0x2A, 0x20, 0x43, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x20, 0x44, 0x1B, 0x29, 0x20, 0x45, 0x1B, 0x2A, 0x20, 0x46, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x20, 0x47, 0x1B, 0x29, 0x20, 0x48, 0x1B, 0x2A, 0x20, 0x49, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x20, 0x4A, 0x1B, 0x29, 0x20, 0x4B, 0x1B, 0x2A, 0x20, 0x4C, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x20, 0x4D, 0x1B, 0x29, 0x20, 0x4E, 0x1B, 0x2A, 0x20, 0x4F, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x24, 0x42, 0x1B, 0x29, 0x20, 0x42, 0x1B, 0x2A, 0x30, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x24, 0x42, 0x1B, 0x29, 0x20, 0x43, 0x1B, 0x2A, 0x30, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x24, 0x42, 0x1B, 0x29, 0x20, 0x44, 0x1B, 0x2A, 0x30, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x31, 0x1B, 0x29, 0x30, 0x1B, 0x2A, 0x4A, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
    { 0x1B, 0x28, 0x4A, 0x1B, 0x29, 0x32, 0x1B, 0x2A, 0x20, 0x41, 0x1B, 0x2B, 0x20, 0x70, 0x0F, 0x1B, 0x7D },
};

//---------------------------------------------------------------------------
static const int8u AribStdB24B37_DefaultMacros_size[] =
{
    16,
    16,
    17,
    16,
    16,
    17,
    19,
    19,
    19,
    19,
    19,
    17,
    17,
    17,
    16,
    17,
};

//---------------------------------------------------------------------------
// CRC_CCIT_Xmodem_Table
// A CRC is computed like this:
// Init: int16u CRC_CCIT_Xmodem = 0x0000;
// for each data byte do
//     CRC_CCIT_Xmodem=(CRC_CCIT_Xmodem<<8) ^ CRC_CCIT_Xmodem_Table[(CRC_CCIT_Xmodem>>8)^(data_byte)];
// Array built with the help of http://www.sanity-free.com/133/crc_16_ccitt_in_csharp.html
int16u AribStdB24B37_CRC_CCIT_Xmodem_Table[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063,
    0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B,
    0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252,
    0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A,
    0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401,
    0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509,
    0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630,
    0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738,
    0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7,
    0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF,
    0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96,
    0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E,
    0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5,
    0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
    0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4,
    0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC,
    0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB,
    0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3,
    0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA,
    0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2,
    0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589,
    0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481,
    0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8,
    0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0,
    0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F,
    0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827,
    0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E,
    0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16,
    0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D,
    0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45,
    0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C,
    0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74,
    0x2E93, 0x3EB2, 0x0ED1, 0x1EF0,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_AribStdB24B37::File_AribStdB24B37()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_AribStdB24B37;
        StreamIDs_Width[0]=1;
    #endif //MEDIAINFO_EVENTS
    PTS_DTS_Needed=true;

    //In
    HasCcis=false;
    ParseCcis=false;
    IsAncillaryData=false;

    //Config
    Caption_conversion_type=(int8u)-1;

    //Ancillary
    #if defined(MEDIAINFO_MPEGTS_YES)
        Parser=NULL;
    #endif
}

//---------------------------------------------------------------------------
File_AribStdB24B37::~File_AribStdB24B37()
{
    #if defined(MEDIAINFO_MPEGTS_YES)
        delete Parser;
    #endif
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_AribStdB24B37::Streams_Fill()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
    {
        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, StreamPos_Last, Text_ID, Pos+1);
        Fill(Stream_Text, StreamPos_Last, Text_Format, "ARIB STD B24/B37");
        if (HasCcis)
        {
            Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, "CCIS");
            Fill(Stream_Text, StreamPos_Last, Text_Format_Profile, AribStdB24B37_Caption_conversion_type(Caption_conversion_type));
        }
        Fill(Stream_Text, StreamPos_Last, Text_StreamSize, 0);
        Fill(Stream_Text, StreamPos_Last, Text_BitRate_Mode, "CBR");
        Fill(Stream_Text, StreamPos_Last, Text_Language, Streams[Pos].ISO_639_language_code);
    }
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::Streams_Finish()
{
    #if defined(MEDIAINFO_MPEGTS_YES)
        if (Parser)
        {
            Finish(Parser);
            Merge(*Parser);
        }
    #endif
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_AribStdB24B37::Read_Buffer_Continue()
{
    if (Buffer_Size==0)
        return;

    if (IsAncillaryData)
    {
        if (!Status[IsAccepted])
            Accept();
        int8u DataIdentifier;
        BS_Begin();
        Skip_SB(                                                "Error correction");
        Skip_SB(                                                "Undefined");
        Skip_SB(                                                "Undefined");
        Skip_SB(                                                "Undefined");
        Skip_S1(4,                                              "Continuity Index");
        Skip_S1(8,                                              "Undefined");
        Skip_SB(                                                "Undefined");
        Skip_SB(                                                "Start packet flag");
        Skip_SB(                                                "End packet flag");
        Skip_SB(                                                "Send mode");
        Info_S1(4, Caption_conversion_type,                     "Format identifier"); Param_Info1(AribStdB24B37_Caption_conversion_type(Caption_conversion_type));
        Skip_S1(2,                                              "Undefined");
        Get_S1 (3, DataIdentifier,                              "Closed caption data identifier"); Param_Info1(AribStdB24B37_Caption_DataIdentifier(DataIdentifier));
        Info_S1(3, data_group_id,                               "Language identifier"); if (DataIdentifier) {Param_Info1(AribStdB24B37_data_group_id(data_group_id));}
        BS_End();

        if (DataIdentifier>6)
        {
            Skip_XX(245,                                        "Dummy");
        }
        else if (DataIdentifier<4)
        {
            Skip_XX(245,                                        "Exchange format data, not supported");
        }
        else
        {
            Element_Begin1("Short form data");
            int8u LEN, Label_01, Label_3A, Data_Length;
            Get_B1(LEN,                                         "LEN");
            Element_Begin1("display timing");
                Get_B1(Label_01,                                "Label (01)");
                BS_Begin();
                Skip_S1(6,                                      "Undefined");
                Skip_S1(2,                                      "Data-type identifier");
                Skip_S1(6,                                      "Undefined");
                Skip_S1(2,                                      "Timing-type identifier");
                Skip_S1(6,                                      "Undefined");
                Skip_S1(2,                                      "Timing-direction identifier");
                Skip_B5(                                        "Display timing value");
                BS_End();
            Element_End0();
            Element_Begin1("closed caption data");
                Get_B1(Label_3A,                                "Label (3A)");
                Get_B1(Data_Length,                             "Data Length");
                #if defined(MEDIAINFO_MPEGTS_YES)
                    if (Parser==NULL)
                    {
                        Parser=new File_MpegTs;
                        ((File_MpegTs*)Parser)->FromAribStdB24B37=true;
                        Open_Buffer_Init(Parser);
                    }
                    if (FrameInfo.PTS==(int64u)-1)
                        FrameInfo.PTS=FrameInfo.DTS;
                    Parser->FrameInfo=FrameInfo;
                    Open_Buffer_Continue(Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, 188);
                    Element_Offset+=188;
                #else
                    Skip_XX(188,                                "TS data");
                #endif
                if (Data_Length==192)
                {
                    Skip_B2(                                    "Group-A CRC");
                    Skip_B2(                                    "Group-B CRC");
                }
                else if (Data_Length>188)
                    Skip_XX(Data_Length-188,                    "Unknown");
            Element_End0();
            if (LEN>203)
                Skip_XX(LEN-204,                                "User Data");
            if (LEN<244)
                Skip_XX(244-LEN,                                "Unused");
            Skip_XX(Element_Size-Element_Offset-6,              "Format data");
            Element_End0();
        }

        Skip_B6(                                                "ECC");
        return;
    }


    if (ParseCcis)
    {
        int32u CCIS_code;
        Get_C4 (   CCIS_code,                                   "CCIS_code");
        if (CCIS_code==0xFFFFFFFF)
        {
            Skip_XX(Element_Size,                               "?");
            return;
        }
        Get_B1 (   Caption_conversion_type,                     "Caption_conversion_type"); Param_Info1(AribStdB24B37_Caption_conversion_type(Caption_conversion_type));
        BS_Begin();
        Info_S1(2, DRCS_conversion_type,                        "DRCS_conversion_type"); Param_Info1(AribStdB24B37_DRCS_conversion_type(DRCS_conversion_type));
        Skip_S1(6,                                              "reserved");
        BS_End();
        Skip_B2(                                                "reserved");
        Skip_B8(                                                "reserved");
        ParseCcis=false;
        return;
    }

    Skip_B1(                                                    "Data_identifier");
    Skip_B1(                                                    "Private_stream_id");
    BS_Begin();
    Skip_S1(4,                                                  "reserved");
    Skip_S1(4,                                                  "PES_data_packet_header_length");
    BS_End();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::Read_Buffer_Unsynched()
{
    #if defined(MEDIAINFO_MPEGTS_YES)
        if (Parser)
            Parser->Open_Buffer_Unsynch();
    #endif
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_AribStdB24B37::Header_Parse()
{
    //Parsing
    int16u data_group_size;
    int8u  data_group_id;
    BS_Begin();
    Skip_SB(                                                    "data_group_id (update part)");
    Get_S1 (5, data_group_id,                                   "data_group_id"); Param_Info1(AribStdB24B37_data_group_id(data_group_id));
    Skip_S1(2,                                                  "data_group_version");
    BS_End();
    Skip_B1(                                                    "data_group_link_number");
    Skip_B1(                                                    "last_data_group_link_number");
    Get_B2 (data_group_size,                                    "data_group_size");

    //Filling
    Header_Fill_Code(data_group_id, AribStdB24B37_data_group_id(data_group_id));
    Header_Fill_Size(Element_Offset+data_group_size+2);
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::Data_Parse()
{
    //CRC
    int16u CRC_CCIT_Xmodem=0x0000;
    const int8u* CRC_CCIT_Xmodem_Buffer=Buffer+Buffer_Offset-Header_Size; //data_group_id position
    while(CRC_CCIT_Xmodem_Buffer<Buffer+Buffer_Offset+(size_t)Element_Size) //from data_group_id to the end, CRC included
    {
        CRC_CCIT_Xmodem=(CRC_CCIT_Xmodem<<8) ^ AribStdB24B37_CRC_CCIT_Xmodem_Table[(CRC_CCIT_Xmodem>>8)^(*CRC_CCIT_Xmodem_Buffer)];
        CRC_CCIT_Xmodem_Buffer++;
    }
    if (CRC_CCIT_Xmodem)
    {
        Skip_XX(Element_Size,                                   "Data");
        Trusted_IsNot("CRC error");
        return;
    }

    Element_Size-=2;

    switch (Element_Code)
    {
        case 0 :
                caption_management(); break;
        case 1 :
        case 2 :
        case 3 :
        case 4 :
        case 5 :
        case 6 :
        case 7 :
        case 8 :
                if (Streams.empty())
                {
                    Skip_XX(Element_Size,                       "Waiting for caption_management");
                    break;
                }
                if (Element_Code>Streams.size())
                {
                    Skip_XX(Element_Size,                       "Unknown service");
                    Trusted_IsNot("Invalid service number");
                    break;
                }

                Streams[(size_t)(Element_Code-1)].Line.clear();
                caption_statement();
                Streams[(size_t)(Element_Code-1)].Line.clear();
                break;
        default: Skip_XX(Element_Size,                          "Unknown");
    }

    Element_Size+=2;
    Skip_B2(                                                    "CRC_16");
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_AribStdB24B37::caption_management() //caption_management_data()
{
    //Parsing
    int32u data_unit_loop_length;
    int8u  TMD, num_languages;
    BS_Begin();
    Get_S1 (2, TMD,                                             "TMD"); Param_Info1(AribStdB24B37_TMD(TMD));
    Skip_S1(6,                                                  "Reserved");
    if (TMD==2)
    {
        Skip_S5(36,                                             "OTM"); //Note: time offset (HHMMSSmmm in BCD format) is currently not supported
        Skip_S5( 4,                                             "Reserved");
    }
    BS_End();
    Get_B1 (num_languages,                                      "num_languages");

    Streams.clear();
    Streams.resize(num_languages);
    for (int8u pos_languages=0; pos_languages<num_languages; pos_languages++)
    {
        int8u DMF_reception, Format;
        string ISO_639_language_code;
        Element_Begin1("language");
        BS_Begin();
        Skip_S1(3,                                              "language_tag");
        Skip_SB(                                                "Reserved");
        Get_S1 (2, DMF_reception,                               "DMF (reception)"); Param_Info1(AribStdB24B37_DMF_reception(DMF_reception)); //Note: Display Method is currently not supported (all is displayed immediatly)
        Info_S1(2, DMF_recording,                               "DMF (recording)"); Param_Info1(AribStdB24B37_DMF_recording(DMF_recording));
        BS_End();
        if (DMF_reception==3)
            Skip_B1(                                            "DC");
        Get_String(3, ISO_639_language_code,                    "ISO_639_language_code");
        BS_Begin();
        Get_S1 (4, Format,                                      "Format"); Param_Info1(AribStdB24B37_format(Format));
        Info_S1(2, TCS,                                         "TCS"); Param_Info1(AribStdB24B37_TCS(TCS));
        Info_S1(2, rollup_mode,                                 "rollup_mode"); Param_Info1(AribStdB24B37_rollup_mode(rollup_mode));
        BS_End();
        Element_End0();

        FILLING_BEGIN();
            Streams[pos_languages].ISO_639_language_code=ISO_639_language_code;
            Streams[pos_languages].DMF_reception=DMF_reception;
            Streams[pos_languages].Format=Format;

            // Special case
            if (ISO_639_language_code=="por")
            {
                Streams[pos_languages].G[0]=GS_Alphanumeric; //GS_Kanji;
                Streams[pos_languages].G[1]=GS_Alphanumeric;
                Streams[pos_languages].G[2]=GS_Alphanumeric; //GS_Hiragana;
                Streams[pos_languages].G[3]=GS_Alphanumeric; //GS_DRCS|GS_Macro;
                Streams[pos_languages].G_Width[0]=2;
                Streams[pos_languages].G_Width[1]=1;
                Streams[pos_languages].G_Width[2]=1;
                Streams[pos_languages].G_Width[3]=1;
            }
        FILLING_END();
    }
    Get_B3 (data_unit_loop_length,                              "data_unit_loop_length");
    if (data_unit_loop_length)
        Skip_XX(data_unit_loop_length,                          "data_unit");

    FILLING_BEGIN();
        if (!Status[IsAccepted])
            Accept();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::caption_statement() //caption_data()
{
    if (Streams[(size_t)(Element_Code-1)].ISO_639_language_code!="jpn")
    {
        Skip_XX(Element_Size-Element_Offset,                    "Data");
        return; //Not supported (e.g. Portuguese from Brazil, ISDB-Tb)
    }

    //Parsing
    int32u data_unit_loop_length;
    int8u  TMD;
    BS_Begin();
    Get_S1 (2, TMD,                                             "TMD"); Param_Info1(AribStdB24B37_TMD(TMD));
    Skip_S1(6,                                                  "Reserved");
    if (TMD==2)
    {
        Skip_S5(36,                                             "STM"); //Note: start time (HHMMSSmmm in BCD format) is currently not supported (and is applicable only if out of band PTS is not avaialble)
        Skip_S5( 4,                                             "Reserved");
    }
    BS_End();
    Get_B3 (data_unit_loop_length,                              "data_unit_loop_length");

    if (Element_Offset+data_unit_loop_length!=Element_Size)
    {
        Skip_XX(Element_Size-Element_Offset,                    "Problem");
        return;
    }

    while (Element_Offset<Element_Size)
    {
        Element_Begin1("data_unit");
        int8u unit_separator;
        Get_B1 (unit_separator,                                 "unit_separator"); // Should always be 0x1F?
        if (unit_separator==0x1F)
        {
            int32u data_unit_size;
            int8u  data_unit_parameter;
            Get_B1 (data_unit_parameter,                        "data_unit_parameter"); Param_Info1(AribStdB24B37_data_unit_parameter(data_unit_parameter));
            Get_B3 (data_unit_size,                             "data_unit_size");
            switch (data_unit_parameter)
            {
                case 0x20 : data_unit_data(Element_Offset+data_unit_size); break;
                default   : Skip_XX(data_unit_size,             "(Not implemented)");
            }
        }
        Element_End0();
    }

    Frame_Count++;
    Frame_Count_NotParsedIncluded++;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::data_unit_data(int64u End)
{
    Element_Begin1("data_unit_data");

    //data_unit_data_byte
    while (Element_Offset<End)
    {
        int8u header;
        Peek_B1(header);
        if (header&0x60) // Not C0 or C1
        {
            if ((header&0x7F)==0x20
             || (header&0x7F)==0x7F)
            {
                Skip_C1 (                                       "Character");
                Add((Char)header);
            }
            else if (header&0x80) // GR //TODO: buffer check //Note about Caption_conversion_type == Mobile closed caption: see B37 Figure 3-4 Code Ranges for Mobile Closed Caption
                Character(Caption_conversion_type==4?GS_Kanji:Streams[(size_t)(Element_Code-1)].G[Streams[(size_t)(Element_Code-1)].GR],
                          Streams[(size_t)(Element_Code-1)].GR,
                          Buffer[Buffer_Offset+Element_Offset]&0x7F,
                          Buffer[Buffer_Offset+Element_Offset+1]&0x7F);
            else // GL
            {
                Character(Caption_conversion_type==4?GS_DRCS:Streams[(size_t)(Element_Code-1)].G[Streams[(size_t)(Element_Code-1)].GL_SS?Streams[(size_t)(Element_Code-1)].GL_SS:Streams[(size_t)(Element_Code-1)].GL],
                          Streams[(size_t)(Element_Code-1)].GL_SS?Streams[(size_t)(Element_Code-1)].GL_SS:Streams[(size_t)(Element_Code-1)].GL,
                          Buffer[Buffer_Offset+Element_Offset],
                          Buffer[Buffer_Offset+Element_Offset+1]);
                Streams[(size_t)(Element_Code-1)].GL_SS=0;
            }
        }
        else
            control_code(); // C0 or C1
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::JIS (int8u Row, int8u Column)
{
    // Encoding is JIS, but we have only Shift-JIS to Unicode algo, we transform JIS to Shift-JIS (Windows-31J)
    // See http://en.wikipedia.org/wiki/Shift_JIS for the formula

    if (Column<32)
        return; // Problem

    #ifdef __WINDOWS__
        char ShiftJIS[2];
        ShiftJIS[0]=((Row+1)>>1) + (Row<=94?112:176);
        if (Row%2)
            ShiftJIS[1]=Column+31+(Column>=96);
        else
            ShiftJIS[1]=Column+126;

        wchar_t Temp[2];
        int CharSize=MultiByteToWideChar(932, 0, ShiftJIS, 2, Temp, 2); //932 = Shift-JIS (Windows-31J)
        if (CharSize>0)
        {
            Temp[CharSize]=__T('\0');
            Param_Info1(Ztring().From_Unicode(Temp));
            Add (Ztring().From_Unicode(Temp));
        }
    #else // __WINDOWS__
        //TODO
    #endif // __WINDOWS__
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::Add (Char Character)
{
    Streams[(size_t)(Element_Code-1)].Line+=Character;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::Add (Ztring Character)
{
    Streams[(size_t)(Element_Code-1)].Line+=Character;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::DefaultMacro()
{
    Element_Begin1("Default Macro");
    int8u control_code;
    Get_B1 (control_code,                                       "control_code");

    if ((control_code&0xF0)==0x60) //Known macros
    {
        //Buffer
        const int8u* Save_Buffer=Buffer;
        size_t Save_Buffer_Offset=Buffer_Offset;
        size_t Save_Buffer_Size=Buffer_Size;
        int64u Save_Element_Offset=Element_Offset;
        int64u Save_Element_Size=Element_Size;

        Buffer=AribStdB24B37_DefaultMacros[control_code&0x0F];
        Buffer_Offset=0;
        Buffer_Size=AribStdB24B37_DefaultMacros_size[control_code&0x0F];
        Element_Offset=0;
        Element_Size=Buffer_Size;

        data_unit_data(Element_Size);

        Buffer=Save_Buffer; Save_Buffer=NULL;
        Buffer_Offset=Save_Buffer_Offset;
        Buffer_Size=Save_Buffer_Size;
        Element_Offset=Save_Element_Offset;
        Element_Size=Save_Element_Size;
    }
    else
    {
        Element_Info1("Unknown");
        Param_Info1("Unknown");
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::Character (int16u CharacterSet, int8u G_Value, int8u FirstByte, int8u SecondByte)
{
    int16u Value=(FirstByte<<8) | SecondByte;

    //ARIB STD B24/B37 provide numbers with 2 number (Row and Colupmn) a shift of 32 (b00100000)
    #define Compute(f,s) (((f+32)<<8) | (s+32))

    switch (CharacterSet)
    {
        case GS_Kanji:
                        Skip_B2(                                "Character");
                        if (Value<=Compute(84, 6))
                            JIS(FirstByte, SecondByte);
                        else
                            switch (Value)
                            {
                                case Compute(92,  1): JIS( 35,  42); break;
                                case Compute(92,  2): JIS( 35,  43); break;
                                case Compute(92,  3): JIS( 35,  44); break;
                                case Compute(92,  4): JIS( 35,  45); break;
                                case Compute(93, 79): JIS( 40, 110); break;
                                case Compute(93, 88): Param_Info1(Ztring().From_UTF8("\xE2\x99\xAB")+__T(" (not exact)")); Add (Ztring().From_UTF8("\xE2\x99\xAB")); break; //Ending music note?
                                case Compute(93, 89): Param_Info1(Ztring().From_UTF8("\xE2\x99\xAB")+__T(" (not exact)")); Add (Ztring().From_UTF8("\xE2\x99\xAB")); break; //Opening music note?
                                case Compute(93, 90): Param_Info1(Ztring().From_UTF8("\xE2\x99\xAB")); Add (Ztring().From_UTF8("\xE2\x99\xAB")); break; //Music note
                                default: Param_Info1("(Unsupported)"); //empty in spec or not yet mapped
                            }
                        break;
        case GS_Hiragana:
        case GS_PropHiragana:
                        Skip_C1(                                "Character");
                        switch (FirstByte)
                        {
                            case 0x74:
                            case 0x75:
                            case 0x76:
                                       Param_Info1("(Unsupported)");
                                       break; //empty in spec
                            case 0x77: JIS( 33,  53); break;
                            case 0x78: JIS( 33,  54); break;
                            case 0x79: JIS( 33,  60); break;
                            case 0x7A: JIS( 33,  35); break;
                            case 0x7B: JIS( 33,  86); break;
                            case 0x7C: JIS( 33,  87); break;
                            case 0x7D: JIS( 33,  34); break;
                            case 0x7E: JIS( 33,  38); break;
                            default  : JIS( 36, FirstByte);
                        }
                        break;
        case GS_Katakana:
        case GS_PropKatakana:
                        Skip_C1(                                "Character");
                        switch (FirstByte)
                        {
                            case 0x77: JIS( 33,  41); break;
                            case 0x78: JIS( 33,  42); break;
                            case 0x79: JIS( 33,  51); break;
                            case 0x7A: JIS( 33,  33); break;
                            case 0x7B: JIS( 33,  86); break;
                            case 0x7C: JIS( 33,  87); break;
                            case 0x7D: JIS( 33,  34); break;
                            case 0x7E: JIS( 33,  38); break;
                            default  : JIS( 37, FirstByte); break;
                        }
                        break;
        case GS_Alphanumeric:
        case GS_PropAscii:
                        Skip_C1(                                "Character");
                        Add(FirstByte);
                        break;
        case GS_DRCS|GS_Macro :
                        DefaultMacro();
                        break;
        default : ;
                        switch (Streams[(size_t)(Element_Code-1)].G_Width[G_Value])
                        {
                            case 1 : Skip_C1("Character (unsupported)"); break;
                            case 2 : Skip_C2("Character (unsupported)"); break;
                            default: Skip_XX(Streams[(size_t)(Element_Code-1)].G_Width[G_Value], "Character (unsupported)");
                        }
    }
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::control_code()
{
    int8u control_code;
    Peek_B1(control_code);

    switch (control_code)
    {
        // Table 7-15 C0 control set
        case 0x00 : NUL(); break;
        case 0x07 : BEL(); break;
        case 0x08 : APB(); break;
        case 0x09 : APF(); break;
        case 0x0A : APD(); break;
        case 0x0B : APU(); break;
        case 0x0C : CS(); break;
        case 0x0D : APR(); break;
        case 0x0E : LS1(); break;
        case 0x0F : LS0(); break;
        case 0x16 : PAPF(); break;
        case 0x18 : CAN(); break;
        case 0x19 : SS2(); break;
        case 0x1B : ESC(); break;
        case 0x1C : APS(); break;
        case 0x1D : SS3(); break;
        case 0x1E : RS(); break;
        case 0x1F : US(); break;

        // Table 7-16 C1 control set
        case 0x80: //BKF
        case 0x81: //RDF
        case 0x82: //GRF
        case 0x83: //YLF
        case 0x84: //BLF
        case 0x85: //MGF
        case 0x86: //CNF
        case 0x87: //WHF
                    xxF(); break;
        case 0x88: //SSZ
        case 0x89: //MSZ
        case 0x8A: //NSZ
                    xxZ(); break;
        case 0x8B : SZX(); break;
        case 0x90 : COL(); break;
        case 0x91 : FLC(); break;
        case 0x92 : CDC(); break;
        case 0x93 : POL(); break;
        case 0x94 : WMM(); break;
        case 0x95 : MACRO(); break;
        case 0x97 : HLC(); break;
        case 0x98 : RPC(); break;
        case 0x99 : SPL(); break;
        case 0x9A : STL(); break;
        case 0x9B : CSI(); break;
        case 0x9D : TIME(); break;

        // Unknown
        default   : Skip_XX(Element_Size-Element_Offset,        "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::NUL()
{
    Element_Begin1("NUL - Empty");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::BEL()
{
    Element_Begin1("BEL - Bell");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::APB()
{
    Element_Begin1("APB - Active position backward");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::APF()
{
    Element_Begin1("APF - Active position forwards");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::APD()
{
    Element_Begin1("APD - Active position down");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::APU()
{
    Element_Begin1("APU - Active position up");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::CS()
{
    Element_Begin1("CS - Clear Screen");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::APR()
{
    Element_Begin1("APR - Line return at operation position");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::LS1()
{
    Element_Begin1("LS1 - Locking shift 1");
    Skip_B1(                                                    "control_code");
    Element_End0();

    Streams[(size_t)(Element_Code-1)].GL=1;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::LS0()
{
    Element_Begin1("LS0 - Locking shift 0");
    Skip_B1(                                                    "control_code");
    Element_End0();

    Streams[(size_t)(Element_Code-1)].GL=0;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::PAPF()
{
    Element_Begin1("PAPF - Move forwards at specified operation position");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::CAN()
{
    Element_Begin1("CAN - Cancel");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::SS2()
{
    Element_Begin1("SS2 - Single shift 2");
    Skip_B1(                                                    "control_code");
    Element_End0();

    Streams[(size_t)(Element_Code-1)].GL_SS=3;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::ESC()
{
    Element_Begin1("ESC - Escape");
    int8u P1;
    Skip_B1(                                                    "control_code");
    Get_B1 (P1,                                                 "P1");

    switch (P1)
    {
        // Table 7-2 Designation of graphic sets
        case 0x24 :
                    {
                    int8u P2;
                    Get_B1 (P2,                                 "P2");
                    switch (P2)
                    {
                        case 0x28 :
                                    {
                                    int8u P3;
                                    Get_B1 (P3,                 "P3");
                                    switch (P3)
                                    {
                                        case 0x20 :
                                                    {
                                                    int8u P4;
                                                    Get_B1 (P4, "P4");
                                                    Streams[(size_t)(Element_Code-1)].G[0]=GS_DRCS|P4;
                                                    Streams[(size_t)(Element_Code-1)].G_Width[0]=2;
                                                    }
                                                    break;
                                        case 0x29 :
                                        case 0x2A :
                                        case 0x2B :
                                        default   :
                                                    Streams[(size_t)(Element_Code-1)].G[0]=P2;
                                                    Streams[(size_t)(Element_Code-1)].G_Width[0]=2;
                                    }
                                    }
                                    break;
                        case 0x29 :
                        case 0x2A :
                        case 0x2B :
                                    {
                                    int8u P3;
                                    Get_B1 (P3,                 "P3");
                                    if (P3==0x20)
                                    {
                                        int8u P4;
                                        Get_B1 (P4,             "P4");
                                        Streams[(size_t)(Element_Code-1)].G[P2-0x28]=GS_DRCS|P4;
                                    }
                                    else
                                        Streams[(size_t)(Element_Code-1)].G[P2 - 0x28]=P3;
                                    Streams[(size_t)(Element_Code-1)].G_Width[P2-0x28]=2;
                                    }
                                    break;
                        default :
                                    Streams[(size_t)(Element_Code-1)].G[0]=P2;
                                    Streams[(size_t)(Element_Code-1)].G_Width[0]=2;
                    }
                    }
                    break;
        case 0x28 :
        case 0x29 :
        case 0x2A :
        case 0x2B :
                    {
                    int8u P2;
                    Get_B1 (P2,                                 "P2");
                    if (P2==0x20)
                    {
                        int8u P3;
                        Get_B1 (P3,                             "P3");
                        Streams[(size_t)(Element_Code-1)].G[P1-0x28]=GS_DRCS|P3;
                    }
                    else
                        Streams[(size_t)(Element_Code-1)].G[P1 - 0x28]=P2;
                    Streams[(size_t)(Element_Code-1)].G_Width[P1-0x28]=1;
                    }
                    break;

        // Table 7-1 Invocation of code elements (locking shift)
        case 0x6E : Streams[(size_t)(Element_Code-1)].GL=2; break;
        case 0x6F : Streams[(size_t)(Element_Code-1)].GL=3; break;
        case 0x7C : Streams[(size_t)(Element_Code-1)].GR=3; break;
        case 0x7D : Streams[(size_t)(Element_Code-1)].GR=2; break;
        case 0x7E : Streams[(size_t)(Element_Code-1)].GR=1; break;

        default   : ; //Problem?
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::APS()
{
    Element_Begin1("APS - Specify operation position");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Skip_B1(                                                    "P2");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::SS3()
{
    Element_Begin1("SS3 - Single shift 3");
    Skip_B1(                                                    "control_code");
    Element_End0();

    Streams[(size_t)(Element_Code-1)].GL_SS=3;
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::RS()
{
    Element_Begin1("RS - Record separator");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::US()
{
    Element_Begin1("US - Unit separator");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::xxF()
{
    Element_Begin1("xxF - foreground");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::xxZ()
{
    Element_Begin1("xxZ - size");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::SZX()
{
    Element_Begin1("SZX - Specified size");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::COL()
{
    Element_Begin1("COL - Color specification");
    int8u P1;
    Skip_B1(                                                    "control_code");
    Get_B1 (P1,                                                 "P1");
    if (P1==0x20)
        Skip_B1(                                                "P2");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::FLC()
{
    Element_Begin1("FLC - Flashing control");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::CDC()
{
    Element_Begin1("CDC - Conceal Display Controls");
    int8u P1;
    Skip_B1(                                                    "control_code");
    Get_B1 (P1,                                                 "P1");
    if (P1==0x20)
        Skip_B1(                                                "P2");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::POL()
{
    Element_Begin1("POL - Pattern polarity");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::WMM()
{
    Element_Begin1("WMM - Modification of write mode");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::MACRO()
{
    Element_Begin1("MACRO - Macro specification");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();

    //TODO: save macros
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::HLC()
{
    Element_Begin1("HLC - Enclosure control");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::RPC()
{
    Element_Begin1("RPC - Character repeat");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::SPL()
{
    Element_Begin1("SPL - End of underline and mosaic separation");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::STL()
{
    Element_Begin1("STL - Start of underline and mosaic separation");
    Skip_B1(                                                    "control_code");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::CSI()
{
    Element_Begin1("CSI - Extended Control Codes");
    Skip_B1(                                                    "control_code");

    vector<int64u> Params;
    Params.push_back(0);
    size_t Pos=0;

    while (Element_Offset+Pos<=Element_Size)
    {
        int8u Value=Buffer[Buffer_Offset+Element_Offset+Pos];
        Pos++;
        if (Value==0x3B)
            Params.push_back(0);
        else if (Value>=0x30 && Value<0x3A)
        {
            Params[Params.size()-1]*=10;
            Params[Params.size()-1]+=Value&0x0F;
        }
        else if (Value>=0x40 && Value<0x80)
        {
            Pos--;
            Skip_Local(Pos,                                     "Values");
            Get_B1(Value,                                       "Delimiter");
            switch (Value)
            {
                case 0x3B:
                            Element_Info1("SRC - Raster Colour Designation");
                            break;
                case 0x42:
                            Element_Info1("GSM - Character deformation");
                            break;
                case 0x53:
                            Element_Info1("SWF - Set Writing Format");
                            if (!Params.empty() && Params[0]<0x100)
                                Streams[(size_t)(Element_Code-1)].Format=(int8u)Params[0];
                            break;
                case 0x54:
                            Element_Info1("CCC - Composite Character Composition");
                            break;
                case 0x56:
                            Element_Info1("SDF - Set Display Format");
                            break;
                case 0x57:
                            Element_Info1("SSM - Character composition dot designation");
                            break;
                case 0x58:
                            Element_Info1("SHS - Set Horizontal Spacing");
                            break;
                case 0x59:
                            Element_Info1("SVS - Set Vertical Spacing");
                            break;
                case 0x5B:
                            Element_Info1("PLD - Partially Line Down");
                            break;
                case 0x5C:
                            Element_Info1("PLU - Partialyl Line Up");
                            break;
                case 0x5D:
                            Element_Info1("GAA - Colouring block");
                            break;
                case 0x5F:
                            Element_Info1("SDF - Set Display Position");
                            break;
                case 0x61:
                            Element_Info1("ACPS - Active Coordinate Position Set");
                            //TODO: positioning
                            break;
                case 0x62:
                            Element_Info1("TCC - Switching control"); // Also Table 5-3
                            break;
                case 0x63:
                            Element_Info1("ORN - Ornament Control");
                            break;
                case 0x64:
                            Element_Info1("MDF - Font");
                            break;
                case 0x65:
                            Element_Info1("CFS - Character Font Set");
                            break;
                case 0x66:
                            Element_Info1("XCS - External Character Set");
                            break;
                case 0x67:
                            Element_Info1("SCR - Scroll designation"); // Also Table 5-4
                            break;
                case 0x68:
                            Element_Info1("PRA - Built-in sound replay");
                            break;
                case 0x69:
                            Element_Info1("ACS - Alternative Character Set");
                            break;
                case 0x6E:
                            Element_Info1("RCS - Raster Colour command");
                            break;
                case 0x6F:
                            Element_Info1("SCS - Skip Character Set");
                            break;
                default:    ; //Unknown
            }
            break;
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_AribStdB24B37::TIME()
{
    Element_Begin1("TIME - Time");
    Skip_B1(                                                    "control_code");
    Skip_B1(                                                    "P1");
    Skip_B1(                                                    "P2");
    Element_End0();

    // if P1 is 0x20, there should be a delay
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_ARIBSTDB24B37_YES

