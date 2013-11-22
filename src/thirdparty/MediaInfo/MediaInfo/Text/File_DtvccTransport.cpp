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
#if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_DtvccTransport.h"
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
const char* DtvccTransport_cc_type (int8u cc_type)
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

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_DtvccTransport::File_DtvccTransport()
:File__Analyze()
{
    //Configuration
    ParserName=__T("DTVCC Transport");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_DtvccTransport;
        StreamIDs_Width[0]=1;
    #endif //MEDIAINFO_EVENTS
    PTS_DTS_Needed=true;

    //In
    Format=Format_Unknown;
    AspectRatio=0;

    //Temp
    Streams.resize(3); //CEA-608 Field 1, CEA-608 Field 2, CEA-708 Channel
}

//---------------------------------------------------------------------------
File_DtvccTransport::~File_DtvccTransport()
{
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        delete Streams[Pos]; //Streams[Pos]=NULL
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DtvccTransport::Streams_Update()
{
    Clear(Stream_Text);

    //Per stream
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser && Streams[Pos]->Parser->Status[IsFilled] /*&& Streams[Pos]->Parser->Status[IsUpdated]*/ && Streams[Pos]->Parser->Count_Get(Stream_Text))
            Streams_Update_PerStream(Pos);
}

//---------------------------------------------------------------------------
void File_DtvccTransport::Streams_Update_PerStream(size_t Pos)
{
    if (Streams[Pos]==NULL)
        return;

    Update(Streams[Pos]->Parser);

    if (Streams[Pos]->Parser)
    {
        for (size_t Pos2=0; Pos2<Streams[Pos]->Parser->Count_Get(Stream_Text); Pos2++)
        {
            Stream_Prepare(Stream_Text);
            Merge(*Streams[Pos]->Parser, Stream_Text, Pos2, StreamPos_Last);
            Fill(Stream_Text, StreamPos_Last, "MuxingMode", Format==Format_DVD?__T("DVD-Video"):__T("DTVCC Transport"));
            Fill(Stream_Text, StreamPos_Last, Text_ID, Streams[Pos]->Parser->Retrieve(Stream_Text, Pos2, Text_ID), true);
        }

        Ztring LawRating=Streams[Pos]->Parser->Retrieve(Stream_General, 0, General_LawRating);
        if (!LawRating.empty())
            Fill(Stream_General, 0, General_LawRating, LawRating, true);
    }
}

//---------------------------------------------------------------------------
void File_DtvccTransport::Streams_Finish()
{
    Clear(Stream_Text);

    //Per stream
    for (size_t Pos=0; Pos<Streams.size(); Pos++)
        if (Streams[Pos] && Streams[Pos]->Parser && Streams[Pos]->Parser->Status[IsAccepted] /*&& Streams[Pos]->Parser->Status[IsUpdated]*/)
        {
            Finish(Streams[Pos]->Parser);
            Streams_Update_PerStream(Pos);
        }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
void File_DtvccTransport::Read_Buffer_Unsynched()
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
void File_DtvccTransport::Read_Buffer_Continue()
{
    //Parsing
    Element_Begin1(Format==Format_DVD?"DVD Captions":"DTVCC Transport");
    int8u  cc_count;
    bool   process_cc_data_flag, additional_data_flag;
    BS_Begin();
    if (Format==Format_DVD)
    {
        //Modified DTVCC Transport from DVD
        Skip_SB(                                                "field 1 then field 2");
        Get_S1 (7, cc_count,                                    "cc_count");
        process_cc_data_flag=true;
        additional_data_flag=false;
    }
    else
    {
        //Normal DTVCC Transport
        bool process_em_data_flag;
        Get_SB (process_em_data_flag,                           "process_em_data_flag");
        Get_SB (process_cc_data_flag,                           "process_cc_data_flag");
        Get_SB (additional_data_flag,                           "additional_data_flag");
        Get_S1 (5, cc_count,                                    "cc_count");
        Skip_S1(8,                                              process_em_data_flag?"em_data":"reserved"); //Emergency message
    }
    BS_End();
    if (process_cc_data_flag)
    {
        for (int8u Pos=0; Pos<cc_count; Pos++)
        {
            Element_Begin1("cc");
            int8u cc_type;
            bool  cc_valid;
            BS_Begin();
            Mark_1();
            Mark_1_NoTrustError();
            Mark_1_NoTrustError();
            Mark_1_NoTrustError();
            Mark_1_NoTrustError();
            if (Format==Format_DVD)
            {
                //Modified DTVCC Transport from DVD
                Mark_1();
                Mark_1();
                Get_S1 (1, cc_type,                             "cc_type"); Param_Info1(DtvccTransport_cc_type(cc_type));
                cc_valid=true;
            }
            else
            {
                //Normal DTVCC Transport
                Get_SB (   cc_valid,                            "cc_valid");
                Get_S1 (2, cc_type,                             "cc_type"); Param_Info1(DtvccTransport_cc_type(cc_type));
            }
            BS_End();
            if (cc_valid)
            {
                Element_Begin1("cc_data");
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
                        #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                            if (cc_type<2)
                            {
                                #if defined(MEDIAINFO_EIA608_YES)
                                    Streams[Parser_Pos]->Parser=new File_Eia608();
                                    ((File_Eia608*)Streams[Parser_Pos]->Parser)->cc_type=cc_type;
                                #else //defined(MEDIAINFO_EIA608_YES)
                                    Streams[Parser_Pos]->Parser=new File__Analyze();
                                #endif //defined(MEDIAINFO_EIA608_YES)
                            }
                            else
                            {
                                #if defined(MEDIAINFO_EIA708_YES)
                                    Streams[Parser_Pos]->Parser=new File_Eia708();
                                #else //defined(MEDIAINFO_EIA708_YES)
                                    Streams[Parser_Pos]->Parser=new File__Analyze();
                                #endif //defined(MEDIAINFO_EIA708_YES)
                            }
                        #else //defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                            Streams[Parser_Pos]->Parser=new File__Analyze();
                        #endif //defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                        Open_Buffer_Init(Streams[Parser_Pos]->Parser);
                    }
                    Demux(Buffer+(size_t)(Buffer_Offset+Element_Offset), 2, ContentType_MainStream);
                    if (!Streams[Parser_Pos]->Parser->Status[IsFinished])
                    {
                        //Parsing
                        if (Streams[Parser_Pos]->Parser->PTS_DTS_Needed)
                        {
                            Streams[Parser_Pos]->Parser->FrameInfo.PCR=FrameInfo.PCR;
                            Streams[Parser_Pos]->Parser->FrameInfo.PTS=FrameInfo.PTS;
                            Streams[Parser_Pos]->Parser->FrameInfo.DTS=FrameInfo.DTS;
                        }
                        if (Parser_Pos==2)
                        {
                            #if defined(MEDIAINFO_EIA708_YES)
                                ((File_Eia708*)Streams[2]->Parser)->cc_type=cc_type;
                                if (AspectRatio)
                                    ((File_Eia708*)Streams[2]->Parser)->AspectRatio=AspectRatio;
                            #endif
                            if (cc_type==3)
                            {
                                ((File_Eia708*)Streams[2]->Parser)->cc_type=4; //Magic value saying that the buffer must be kept (this is only a point of synchro from the undelying layer)
                                Open_Buffer_Continue(Streams[Parser_Pos]->Parser, Buffer+(size_t)(Buffer_Offset+Element_Offset), 0);
                                ((File_Eia708*)Streams[2]->Parser)->cc_type=3;
                            }
                        }
                        else
                        {
                        }
                        Open_Buffer_Continue(Streams[Parser_Pos]->Parser, Buffer+(size_t)(Buffer_Offset+Element_Offset), 2);
                        Element_Show();
                        Element_Offset+=2;

                        //Filled
                        if (!Status[IsAccepted])
                            Accept("DTVCC Transport");
                    }
                    else
                        Skip_XX(2,                                  "Data");
                Element_End0();
            }
            else
                Skip_XX(2,                                          "Junk");
            Element_End0();
        }
    }
    else
        Skip_XX(cc_count*2,                                         "Junk");

    if (Format==Format_A53_4_GA94_03)
    {
        //Normal DTVCC Transport
        BS_Begin();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        BS_End();

        if (additional_data_flag)
        {
            Skip_XX(Element_Size-Element_Offset,                "additional_user_data");
        }

        while (Element_Offset<Element_Size)
        {
            Skip_B1(                                                "Zero"); //TODO: test Zero
        }
    }

    Element_End0();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_DTVCCTRANSPORT_YES

