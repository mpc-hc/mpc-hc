// File_Scte20 - Info for SCTE 20 streams
// Copyright (C) 2010-2010 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_SCTE20_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Scte20.h"
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if defined(MEDIAINFO_EIA708_YES)
    #include "MediaInfo/Text/File_Eia708.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Scte20_field_number (int8u field_number)
{
    switch (field_number)
    {
        case  0 : return "Forbidden";
        case  1 : return "1st display field";
        case  2 : return "2nd display field";
        case  3 : return "3rd display field";
        default : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Scte20::File_Scte20()
:File__Analyze()
{
    //Configuration
    ParserName=_T("SCTE 20");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Scte20;
        StreamIDs_Width[0]=1;
    #endif //MEDIAINFO_EVENTS
    PTS_DTS_Needed=true;

    //In
    picture_structure=(int8u)-1;
    progressive_sequence=false;
    progressive_frame=false;
    top_field_first=false;
    repeat_first_field=false;

    //Temp
    Streams.resize(2); //CEA-608 Field 1, CEA-608 Field 2
    Streams_Count=0;
}

//---------------------------------------------------------------------------
File_Scte20::~File_Scte20()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        delete Streams[Pos]; //Streams[Pos]=NULL
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Scte20::Streams_Fill()
{
    //Filling
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser && Streams[Pos]->Parser->Status[IsFilled])
        {
            Merge(*Streams[Pos]->Parser);
            Streams[Pos]->StreamPos=StreamPos_Last;
            if (Pos<3)
                Fill(Stream_Text, StreamPos_Last, Text_ID, _T("608-")+Ztring::ToZtring(Pos+1));
            Fill(Stream_Text, StreamPos_Last, "MuxingMode", _T("SCTE 20"));
        }
}

//---------------------------------------------------------------------------
void File_Scte20::Streams_Finish()
{
    //Filling
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser && Streams[Pos]->Parser->Status[IsFilled])
        {
            Finish(Streams[Pos]->Parser);
            if (Streams[Pos]->StreamPos!=(size_t)-1)
                Merge(*Streams[Pos]->Parser, Stream_Text, 0, Streams[Pos]->StreamPos);
        }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
void File_Scte20::Read_Buffer_Unsynched()
{
    //Parsing
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser)
            Streams[Pos]->Parser->Open_Buffer_Unsynch();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

static inline int8u ReverseBits(int8u c)
{
    // Input: bit order is 76543210
    //Output: bit order is 01234567
    c = (c & 0x0F) << 4 | (c & 0xF0) >> 4;
    c = (c & 0x33) << 2 | (c & 0xCC) >> 2;
    c = (c & 0x55) << 1 | (c & 0xAA) >> 1;
    return c;
}

//---------------------------------------------------------------------------
void File_Scte20::Read_Buffer_Continue()
{
    //Parsing
    Element_Begin("SCTE 20");
    int8u  cc_count;
    bool vbi_data_flag;
    BS_Begin();
    Mark_1_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Get_SB (vbi_data_flag,                                      "vbi_data_flag");
    if (vbi_data_flag)
    {
        Get_S1 (5, cc_count,                                    "cc_count");
        for (int8u Pos=0; Pos<cc_count; Pos++)
        {
            Element_Begin("cc");
            int8u cc_data[2];
            int8u field_number, cc_data_1, cc_data_2;
            Skip_S1(2,                                          "cc_priority");
            Get_S1 (2, field_number,                            "field_number"); Param_Info(Scte20_field_number(field_number));
            Skip_S1(5,                                          "line_offset");
            Get_S1 (8, cc_data_1,                               "cc_data_1");
            cc_data[0]=ReverseBits(cc_data_1);
            Param_Info(Ztring::ToZtring(cc_data[0], 16));
            Get_S1 (8, cc_data_2,                               "cc_data_2");
            cc_data[1]=ReverseBits(cc_data_2);
            Param_Info(Ztring::ToZtring(cc_data[1], 16));
            Mark_1_NoTrustError();
            if (field_number!=0 && picture_structure!=(int8u)-1 && picture_structure!=0)
            {
                Element_Begin("cc_data");

                //Finding the corresponding cc_type (CEA-608 1st field or 2nd field)
                int8u cc_type;
                if (progressive_sequence)
                    cc_type=0;
                else if (picture_structure!=3)
                    cc_type=picture_structure-1;
                else if (field_number==2)
                    cc_type=top_field_first?1:0;
                else //if (field_number==1 || field_number==3)
                    cc_type=top_field_first?0:1;

                //Parsing
                #if MEDIAINFO_DEMUX
                    Element_Code=cc_type;
                #endif //MEDIAINFO_DEMUX
                if (Streams[cc_type]==NULL)
                    Streams[cc_type]=new stream;
                if (Streams[cc_type]->Parser==NULL)
                {
                    Streams[cc_type]->Parser=new File_Eia608();
                    Open_Buffer_Init(Streams[cc_type]->Parser);
                }
                Demux(cc_data, 2, ContentType_MainStream);
                if (!Streams[cc_type]->Parser->Status[IsFinished])
                {
                    //Parsing
                    if (Streams[cc_type]->Parser->PTS_DTS_Needed)
                    {
                        Streams[cc_type]->Parser->PCR=PCR;
                        Streams[cc_type]->Parser->PTS=PTS;
                        Streams[cc_type]->Parser->DTS=DTS;
                    }
                    Open_Buffer_Continue(Streams[cc_type]->Parser, cc_data, 2);
                    Element_Show();

                    //Filled
                    if (!Streams[cc_type]->IsFilled && Streams[cc_type]->Parser->Status[IsFilled])
                    {
                        if (Count_Get(Stream_General)==0)
                            Accept("SCTE 20");
                        Streams_Count++;
                        if (Streams_Count==3)
                            Fill("SCTE 20");
                        Streams[cc_type]->IsFilled=true;
                    }
                }
                else
                    Skip_XX(2,                                  "Data");
                Element_End();
            }
            Element_End();
        }
    }
    Skip_S1(4,                                                  "non_real_time_video_count");
    BS_End();

    if (Element_Size-Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                    "non_real_time_video + reserved");
    Element_End();
    Element_Show();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SCTE20_YES

