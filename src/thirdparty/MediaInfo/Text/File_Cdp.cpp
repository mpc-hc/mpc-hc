// File_Cdp - Info for EIA-608 files
// Copyright (C) 2009-2010 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_CDP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Cdp.h"
#if defined(MEDIAINFO_EIA708_YES)
    #include "MediaInfo/Text/File_Eia708.h"
#endif
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
//---------------------------------------------------------------------------


//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
const char* Cdp_cc_type(int8u cc_type)
{
    switch (cc_type)
    {
        case  0 : return "CEA-608 line 21 field 1 closed captions"; //closed caption 3 if this is second field
        case  1 : return "CEA-608 line 21 field 2 closed captions"; //closed caption 4 if this is second field
        case  2 : return "DTVCC Channel Packet Data";
        case  3 : return "DTVCC Channel Packet Start";
        default : return "";
    }
}

//---------------------------------------------------------------------------
float32 Cdp_cdp_frame_rate(int8u cdp_frame_rate)
{
    switch (cdp_frame_rate)
    {
        case  1 : return (float32)23.976;
        case  2 : return (float32)24.000;
        case  3 : return (float32)25.000;
        case  4 : return (float32)29.970;
        case  5 : return (float32)30.000;
        case  6 : return (float32)50.000;
        case  7 : return (float32)59.940;
        case  8 : return (float32)60.000;
        default : return (float32) 0.000;
    }
}
    
//***************************************************************************
//
//***************************************************************************

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Cdp::File_Cdp()
:File__Analyze()
{
    //Temp
    ParserName=_T("CDP");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Cdp;
        StreamIDs_Width[0]=1;
    #endif //MEDIAINFO_EVENTS
    Streams.resize(3); //CEA-608 Field 1, CEA-608 Field 2, CEA-708 Channel
    Streams_Count=0;

    //In
    AspectRatio=0;
}

//---------------------------------------------------------------------------
File_Cdp::~File_Cdp()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        delete Streams[Pos]; //Streams[Pos]=NULL
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cdp::Streams_Fill()
{
    //Filling
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser && Streams[Pos]->Parser->Status[IsFilled])
        {
            Merge(*Streams[Pos]->Parser);
            if (Pos<2)
                Fill(Stream_Text, StreamPos_Last, Text_ID, _T("608-")+Ztring::ToZtring(Pos+1));
            Fill(Stream_Text, StreamPos_Last, "MuxingMode", _T("CDP"), Unlimited);
        }
}

//---------------------------------------------------------------------------
void File_Cdp::Streams_Finish()
{
    //Filling
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser && Streams[Pos]->Parser->Status[IsFilled])
            Finish(Streams[Pos]->Parser);
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cdp::Read_Buffer_Unsynched()
{
    //Parsing
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser)
            Streams[Pos]->Parser->Open_Buffer_Unsynch();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cdp::Read_Buffer_Continue()
{
    if (!Status[IsAccepted])
        Accept("CDP");

    cdp_header();
    while(Element_Offset<Element_Size)
    {
        int8u section_id;
        Peek_L1(section_id);
        switch (section_id)
        {
            case 0x71 : time_code_section(); break;
            case 0x72 : ccdata_section(); break;
            case 0x73 : ccsvcinfo_section(); break;
            case 0x74 : cdp_footer(); break;
            case 0xFF : Skip_B1("Padding?"); break;
            default   : if (section_id>=0x75 && section_id<=0xEF)
                            future_section();
                        else
                            Skip_XX(Element_Size-Element_Offset, "Unknown");
        }
    }
}

//***************************************************************************
// Functions
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cdp::cdp_header()
{
    Element_Begin("cdp_header");
    int8u cdp_frame_rate;
    Skip_B2(                                                    "cdp_identifier");
    Skip_B1(                                                    "cdp_length");
    BS_Begin();
    Get_S1 (4, cdp_frame_rate,                                  "cdp_frame_rate"); Param_Info(Ztring::ToZtring(Cdp_cdp_frame_rate(cdp_frame_rate))+_T(" fps"));
    Skip_S1(4,                                                  "Reserved");
    Skip_SB(                                                    "time_code_present");
    Skip_SB(                                                    "ccdata_present");
    Skip_SB(                                                    "svcinfo_present");
    Skip_SB(                                                    "svc_info_start");
    Skip_SB(                                                    "svc_info_change");
    Skip_SB(                                                    "svc_info_complete");
    Skip_SB(                                                    "caption_service_active");
    Skip_SB(                                                    "Reserved");
    BS_End();
    Skip_B2(                                                    "cdp_hdr_sequence_cntr");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Cdp::time_code_section()
{
    Element_Begin("time_code_section");
    Skip_B1(                                                    "time_code_section_id");
    BS_Begin();
    Mark_1();
    Mark_1();
    Skip_S1(2,                                                  "tc_10hrs");
    Skip_S1(4,                                                  "tc_1hrs");
    Mark_1();
    Skip_S1(3,                                                  "tc_10min");
    Skip_S1(4,                                                  "tc_1min");
    Skip_SB(                                                    "tc_field_flag");
    Skip_S1(3,                                                  "tc_10sec");
    Skip_S1(4,                                                  "tc_1sec");
    Skip_SB(                                                    "drop_frame_flag");
    Mark_0();
    Skip_S1(2,                                                  "tc_10fr");
    Skip_S1(4,                                                  "tc_1fr");
    BS_End();
    Element_End();
}

//---------------------------------------------------------------------------
void File_Cdp::ccdata_section()
{
    //Parsing
    int8u cc_count;
    Element_Begin("ccdata_section");
    Skip_B1(                                                    "ccdata_id");
    BS_Begin();
    Mark_1();
    Mark_1();
    Mark_1();
    Get_S1 (5, cc_count,                                        "cc_count");
    BS_End();
    for (int8u Pos=0; Pos<cc_count; Pos++)
    {
        Element_Begin("cc");
        int8u cc_type;
        bool  cc_valid;
        BS_Begin();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        Mark_1();
        Get_SB (   cc_valid,                                    "cc_valid");
        Get_S1 (2, cc_type,                                     "cc_type"); Param_Info(Cdp_cc_type(cc_type));
        BS_End();
        if (cc_valid)
        {
            Element_Begin("cc_data");
                //Calculating the parser position
                int8u Parser_Pos=cc_type==3?2:cc_type; //cc_type 2 and 3 are for the same text

                //Parsing
                #if MEDIAINFO_DEMUX
                    Element_Code=Parser_Pos;
                #endif //MEDIAINFO_DEMUX
                if (Streams[Parser_Pos]==NULL)
                    Streams[Parser_Pos]=new stream;
                if (Streams[Parser_Pos]->Parser==NULL)
                {
                    if (cc_type<2)
                    {
                        Streams[Parser_Pos]->Parser=new File_Eia608();
                    }
                    else
                    {
                        Streams[Parser_Pos]->Parser=new File_Eia708();
                    }
                    Open_Buffer_Init(Streams[Parser_Pos]->Parser);
                }
                Demux(Buffer+(size_t)(Buffer_Offset+Element_Offset), 2, ContentType_MainStream);
                if (!Streams[Parser_Pos]->Parser->Status[IsFinished])
                {
                    if (Streams[Parser_Pos]->Parser->PTS_DTS_Needed)
                    {
                        Streams[Parser_Pos]->Parser->PCR=PCR;
                        Streams[Parser_Pos]->Parser->PTS=PTS;
                        Streams[Parser_Pos]->Parser->DTS=DTS;
                    }
                    if (Parser_Pos==2)
                    {
                        ((File_Eia708*)Streams[2]->Parser)->cc_type=cc_type;
                        ((File_Eia708*)Streams[2]->Parser)->AspectRatio=AspectRatio;
                    }
                    else
                    {
                    }
                    Open_Buffer_Continue(Streams[Parser_Pos]->Parser, Buffer+(size_t)(Buffer_Offset+Element_Offset), 2);
                    Element_Offset+=2;

                    //Filled
                    if (!Streams[Parser_Pos]->IsFilled && Streams[Parser_Pos]->Parser->Status[IsFilled])
                    {
                        if (Count_Get(Stream_General)==0)
                            Accept("CDP");
                        Streams_Count++;
                        if (Streams_Count==3)
                            Fill("CDP");
                        Streams[Parser_Pos]->IsFilled=true;
                    }
                }
                else
                    Skip_XX(2,                                  "Data");
            Element_End();
        }
        else
            Skip_XX(2,                                          "Junk");
        Element_End();
    }
    Element_End();
}

//---------------------------------------------------------------------------
void File_Cdp::ccsvcinfo_section()
{
    //Parsing
    int8u svc_count;
    Element_Begin("ccsvcinfo_section");
    Skip_B1(                                                    "ccsvcinfo_id");
    BS_Begin();
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "svc_info_start");
    Skip_SB(                                                    "svc_info_change");
    Skip_SB(                                                    "svc_info_complete");
    Get_S1 (4, svc_count,                                       "svc_count");
    BS_End();
    for (int8u Pos=0; Pos<svc_count; Pos++)
    {
        Element_Begin("svc");
        bool  csn_size;
        BS_Begin();
        Skip_SB(                                                "reserved");
        Get_SB (   csn_size,                                    "csn_size");
        if (csn_size)
        {
            Skip_SB(                                            "reserved");
            Skip_S1(5,                                          "caption_service_number");
        }
        else
            Skip_S1(6,                                          "caption_service_number");
        BS_End();

        //svc_data_byte - caption_service_descriptor
        Element_Begin("service");
        Ztring language;
        bool digital_cc;
        Get_Local(3, language,                                  "language");
        BS_Begin();
        Get_SB (digital_cc,                                     "digital_cc");
        Skip_SB(                                                "reserved");
        if (digital_cc) //line21
        {
            Skip_S1(5,                                          "reserved");
            Skip_SB(                                            "line21_field");
        }
        else
            Skip_S1(6,                                          "caption_service_number");
        Skip_SB(                                                "easy_reader");
        Skip_SB(                                                "wide_aspect_ratio");
        Skip_S2(14,                                             "reserved");
        BS_End();
        Element_End();
        Element_End();
    }
    Element_End();
}

//---------------------------------------------------------------------------
void File_Cdp::cdp_footer()
{
    Element_Begin("cdp_footer");
    Skip_B1(                                                    "cdp_footer_id");
    Skip_B2(                                                    "cdp_ftr_sequence_cntr");
    Skip_B1(                                                    "packet_checksum");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Cdp::future_section()
{
    //Parsing
    int8u length;
    Element_Begin("future_section");
    Skip_B1(                                                    "future_section_id");
    Get_B1 (length,                                             "length");
    Skip_XX(length,                                             "Unknown");
    Element_End();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_CDP_YES
