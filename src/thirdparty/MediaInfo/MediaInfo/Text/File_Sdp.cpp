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
#if defined(MEDIAINFO_SDP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Sdp.h"
#include "MediaInfo/Text/File_Teletext.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Sdp::File_Sdp()
:File__Analyze()
{
    //Configuration
    ParserName=__T("SDP");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Sdp;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    PTS_DTS_Needed=true;
    MustSynchronize=true;
}

//---------------------------------------------------------------------------
File_Sdp::~File_Sdp()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Sdp::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "SDP");
}

//---------------------------------------------------------------------------
void File_Sdp::Streams_Finish()
{
    for (streams::iterator Stream=Streams.begin(); Stream!=Streams.end(); ++Stream)
    {
        if (Stream->second.Parser && Stream->first<0x80) //For the moment, we filter and use only field 1)
        {
            Finish(Stream->second.Parser);
            Merge(*Stream->second.Parser);
            //Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring((Stream->first&0x80)?2:1)+__T('-')+Ztring::ToZtring(Stream->first&0x1F)+__T("-")+Stream->second.Parser->Get(Stream_Text, 0, Text_ID), true);
            Fill(Stream_Text, StreamPos_Last, Text_ID, Stream->second.Parser->Get(Stream_Text, 0, Text_ID), true);
        }
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Sdp::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+2<Buffer_Size)
    {
        while (Buffer_Offset+2<Buffer_Size)
        {
            if (Buffer[Buffer_Offset  ]==0x51
             && Buffer[Buffer_Offset+1]==0x15)
                break; //while()

            Buffer_Offset++;
        }

        if (IsSub)
            break; // Found one file with unknown bytes at the end of the stream, so removing this integrity test for the moment

        if (Buffer_Offset+2<Buffer_Size) //Testing if size is coherant
        {
            if (Buffer_Offset+Buffer[Buffer_Offset+2]==Buffer_Size)
                break;

            if (Buffer_Offset+Buffer[Buffer_Offset+2]+3>Buffer_Size)
                return false; //Wait for more data

            if (Buffer[Buffer_Offset+Buffer[Buffer_Offset+2]  ]==0x51
             && Buffer[Buffer_Offset+Buffer[Buffer_Offset+2]+1]==0x15)
                break; //while()

            Buffer_Offset++;
        }
    }

    //Must have enough buffer for having header
    if (Buffer_Offset+2>=Buffer_Size)
        return false;

    //Synched is OK
    if (!Status[IsAccepted])
    {
        //For the moment, we accept only if the file is in sync, the test is not strict enough
        if (Buffer_Offset)
        {
            Reject();
            return false;
        }

        Accept();
    }
    return true;
}

//---------------------------------------------------------------------------
bool File_Sdp::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x51
     || Buffer[Buffer_Offset+1]!=0x15)
    {
        Synched=false;
        return true;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Sdp::Read_Buffer_Unsynched()
{
    for (streams::iterator Stream=Streams.begin(); Stream!=Streams.end(); ++Stream)
    {
        if (Stream->second.Parser)
        {
            Stream->second.Parser->Open_Buffer_Unsynch();
        }
    }
}

//***************************************************************************
// Buffer - Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Sdp::Header_Parse()
{
    //Parsing
    int8u Length, FormatCode;
    Skip_B2(                                                    "Identifier");
    Get_B1 (Length,                                             "Length");
    Get_B1 (FormatCode,                                         "Format Code");
    for (int8u Pos=0; Pos<5; Pos++)
    {
        FieldLines[Pos]=0;
        #if MEDIAINFO_TRACE
            Element_Begin1("Field/Line");
            BS_Begin();
            Info_SB(   Field,                                   "Field Number");
            Info_S1(2, Reserved,                                "Reserved");
            Info_S1(5, Line,                                    "Line Number");
            BS_End();
            FieldLines[Pos]=((Field?1:0)<<7) |(Reserved<<5) | Line; //Removing field information ((Field?1:0)<<7) |
            if (FieldLines[Pos])
            {
                Element_Info1(Field?2:1);
                Element_Info1(Line);
            }
            else
                Element_Info1("None");
            Element_End0();
        #else //MEDIAINFO_TRACE
            Get_B1(FieldLines[Pos],                             "Field/Line");
            FieldLines[Pos]&=0x7F; //Removing field information
        #endif //MEDIAINFO_TRACE
    }

    if (IsSub)
        Header_Fill_Size(Buffer_Size);
    else
        Header_Fill_Size(Length);
}

//---------------------------------------------------------------------------
void File_Sdp::Data_Parse()
{
    Element_Name("Packet");

    for (int8u Pos=0; Pos<5; Pos++)
    {
        if (FieldLines[Pos])
        {
            Element_Code=FieldLines[Pos];
            stream &Stream=Streams[FieldLines[Pos]];
            if (Stream.Parser==NULL)
            {
                Stream.Parser=new File_Teletext();
                Stream.Parser->IsSubtitle=true;
                Open_Buffer_Init(Stream.Parser);
            }
            if (Stream.Parser->PTS_DTS_Needed)
                Stream.Parser->FrameInfo=FrameInfo;
            Demux(Buffer+Buffer_Offset+Element_Offset, 45, ContentType_MainStream);
            Open_Buffer_Continue(Stream.Parser, Buffer+Buffer_Offset+Element_Offset, 45);
            Element_Offset+=45;
        }
    }

    Element_Begin1("SDP Footer");
    Skip_B1(                                                    "Footer ID");
    Skip_B2(                                                    "Footer Sequence number");
    Skip_B1(                                                    "SDP Cheksum");
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Unknown, out of specs");
    Element_End0();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SDP_YES
