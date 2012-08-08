// File_Scc - Info for SCC streams
// Copyright (C) 2011-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#if defined(MEDIAINFO_SCC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_Scc.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "MediaInfo/Text/File_Eia608.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Scc::File_Scc()
:File__Analyze()
{
    //Configuration
    ParserName=__T("SCC");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Scc;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    PTS_DTS_Needed=true;

    //Temp
    Parser=NULL;
}

//---------------------------------------------------------------------------
File_Scc::~File_Scc()
{
    delete Parser; //Parser=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Scc::Streams_Finish()
{
    if (Parser && Parser->Status[IsAccepted])
    {
        Finish(Parser);
        for (size_t Pos2=0; Pos2<Parser->Count_Get(Stream_Text); Pos2++)
        {
            Stream_Prepare(Stream_Text);
            Merge(*Parser, Stream_Text, StreamPos_Last, Pos2);
            Fill(Stream_Text, StreamPos_Last, Text_ID, Parser->Retrieve(Stream_Text, Pos2, Text_ID), true);
        }
    }
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Scc::FileHeader_Begin()
{
    //Element_Size
    if (File_Size<22)
    {
        Reject("N19");
        return false;
    }
    if (Buffer_Size<22)
        return false; //Must wait for more data

    if (Buffer[ 0]!=0x53
     || Buffer[ 1]!=0x63
     || Buffer[ 2]!=0x65
     || Buffer[ 3]!=0x6E
     || Buffer[ 4]!=0x61
     || Buffer[ 5]!=0x72
     || Buffer[ 6]!=0x69
     || Buffer[ 7]!=0x73
     || Buffer[ 8]!=0x74
     || Buffer[ 9]!=0x5F
     || Buffer[10]!=0x53
     || Buffer[11]!=0x43
     || Buffer[12]!=0x43
     || Buffer[13]!=0x20
     || Buffer[14]!=0x56
     || Buffer[15]!=0x31
     || Buffer[16]!=0x2E
     || Buffer[17]!=0x30
    )
    {
        Reject("N19");
        return false;
    }

    //Element_Size
    if (Buffer_Size<File_Size)
        return false; //Must wait for more data

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Scc::FileHeader_Parse()
{
    Skip_String(18,                                             "Magic");
    while (Element_Offset<Buffer_Size)
    {
        if (Buffer[(size_t)Element_Offset]!=0x0D && Buffer[(size_t)Element_Offset]!=0x0A)
            break;
        Element_Offset++;
    }

    Accept();
    Fill(Stream_General, 0, General_Format, "SCC");

    //Init
    Parser=new File_Eia608();
    Open_Buffer_Init(Parser);
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Scc::Header_Parse()
{
    size_t End=Buffer_Offset;

    //Content
    while (End<Buffer_Size)
    {
        if (Buffer[End]==0x0D || Buffer[End]==0x0A)
            break;
        End++;
    }

    //EOL
    while (End<Buffer_Size)
    {
        if (Buffer[End]!=0x0D && Buffer[End]!=0x0A)
            break;
        End++;
    }

    //Filling
    Header_Fill_Size(End-Buffer_Offset);
    Header_Fill_Code(0, __T("Block"));
}

//---------------------------------------------------------------------------
void File_Scc::Data_Parse()
{
    while (Element_Offset<Element_Size && (Buffer[Buffer_Offset+(size_t)Element_Offset]==0x0D || Buffer[Buffer_Offset+(size_t)Element_Offset]==0x0A))
        Element_Offset++;
    if (Element_Offset==Element_Size)
        return;

    //Parsing
    Skip_String(11,                                             "TimeStamp");
    while (Element_Offset+5<=Element_Size)
    {
        int8u Buffer_Temp[2];
        Buffer_Temp[0]=(Buffer[Buffer_Offset+(size_t)Element_Offset+1]-(Buffer[Buffer_Offset+(size_t)Element_Offset+1]>='a'?('a'-10):'0'))<<4
                     | (Buffer[Buffer_Offset+(size_t)Element_Offset+2]-(Buffer[Buffer_Offset+(size_t)Element_Offset+2]>='a'?('a'-10):'0'));
        Buffer_Temp[1]=(Buffer[Buffer_Offset+(size_t)Element_Offset+3]-(Buffer[Buffer_Offset+(size_t)Element_Offset+3]>='a'?('a'-10):'0'))<<4
                     | (Buffer[Buffer_Offset+(size_t)Element_Offset+4]-(Buffer[Buffer_Offset+(size_t)Element_Offset+4]>='a'?('a'-10):'0'));
        Open_Buffer_Continue(Parser, Buffer_Temp, 2);
        Element_Offset+=5;
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SCC_YES
