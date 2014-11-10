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
#if defined(MEDIAINFO_CDXA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Cdxa.h"
#include "ZenLib/Utils.h"
#include "MediaInfo/MediaInfo_Internal.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Format
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// CDXA = RIFF header + Raw sectors
// Riff header size = 44
// Raw sector size = 2352
//
// Raw sector :
// Sync             12 bytes (00 FF .. FF 00)
// Header           4 bytes
// SubHeader        8 bytes
// Datas            2324 bytes
// EDC (CRC)        4 bytes
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Cdxa::File_Cdxa()
:File__Analyze()
{
    //Configuration
    ParserName=__T("CDXA");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Cdxa;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    MustSynchronize=true;

    //Temp
    MI=NULL;
}

//---------------------------------------------------------------------------
File_Cdxa::~File_Cdxa()
{
    delete MI; //MI=NULL;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************
// RIFF Header, 44 bytes
// RIFF header                      4 bytes, Pos=0
// RIFF data size                   4 bytes, Pos=4
// Format (CDXA)                    4 bytes, Pos=8
// Format Header                    4 bytes, Pos=12
// Format Size                      4 bytes, Pos=16
// Format user_id                   2 bytes, Pos=20
// Format group_id                  2 bytes, Pos=22
// Format attributes                2 bytes, Pos=24
// Format xa_signature              2 bytes, Pos=26 ("XA")
// Format xa_track_number           4 bytes, Pos=28
// Format Reserved                  4 bytes, Pos=32
// Data Header                      4 bytes, Pos=36
// Data Size                        4 bytes, Pos=40
//
// Attributes (big endian):
// 15 Directory
// 14 CDDA
// 13 Interleaved
// 12 Mode2Form2 --> 2324 bytes/block
// 11 Mode2Form1 --> 2048 bytes/block
// 10 Exec_Other
// 09 Reserved
// 08 Read_Other
// 07 Reserved
// 06 Exec_Group
// 05 Reserved
// 04 Read_Group
// 03 Reserved
// 02 Exec_User
// 01 Reserved
// 00 Read_User

//---------------------------------------------------------------------------
bool File_Cdxa::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<0x28)
        return false; //Must wait for more data

    if (                CC4(Buffer+0x00)!=0x52494646  //"RIFF"
     || LittleEndian2int32u(Buffer+0x04)!=LittleEndian2int32u(Buffer+0x28)+0x24 //Sizes of chunks
     ||                 CC4(Buffer+0x08)!=0x43445841  //"CDXA"
     ||                 CC4(Buffer+0x0C)!=0x666D7420  //"fmt "
     || LittleEndian2int32u(Buffer+0x10)!=0x10
     ||                 CC2(Buffer+0x1A)!=0x5841      //"XA"
     ||                 CC4(Buffer+0x24)!=0x64617461) //"data"
    {
        Reject("CDXA");
        return false;
    }

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Cdxa::FileHeader_Parse()
{
    //Parsing
    Skip_C4(                                                    "RIFF header");
    Skip_L4(                                                    "RIFF data size");
    Skip_C4(                                                    "CDXA");
    Skip_C4(                                                    "fmt header");
    Skip_L4(                                                    "fmt size");
    Skip_L2(                                                    "user_id");
    Skip_L2(                                                    "group_id");
    Skip_L2(                                                    "attributes");
    Skip_C2(                                                    "xa_signature");
    Skip_L4(                                                    "xa_track_number");
    Skip_L4(                                                    "reserved");
    Skip_C4(                                                    "data header");
    Skip_L4(                                                    "data size");

    FILLING_BEGIN();
        Accept("CDXA");
        MI=new MediaInfo_Internal;
        MI->Option(__T("FormatDetection_MaximumOffset"), __T("1048576"));
        MI->Option(__T("File_IsReferenced"), __T("1"));
        //MI->Option(__T("File_IsSub"), __T("1"));
        MI->Open_Buffer_Init(File_Size, File_Offset+Buffer_Offset);
    FILLING_END();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Cdxa::Synchronize()
{
    //Synchronizing
    while (           Buffer_Offset+2352*3+12<=Buffer_Size
      && !(CC8(Buffer+Buffer_Offset+2352*0)==0x00FFFFFFFFFFFFFFLL && CC4(Buffer+Buffer_Offset+2352*0+8)==0xFFFFFF00
        && CC8(Buffer+Buffer_Offset+2352*1)==0x00FFFFFFFFFFFFFFLL && CC4(Buffer+Buffer_Offset+2352*1+8)==0xFFFFFF00
        && CC8(Buffer+Buffer_Offset+2352*2)==0x00FFFFFFFFFFFFFFLL && CC4(Buffer+Buffer_Offset+2352*2+8)==0xFFFFFF00
        && CC8(Buffer+Buffer_Offset+2352*3)==0x00FFFFFFFFFFFFFFLL && CC4(Buffer+Buffer_Offset+2352*3+8)==0xFFFFFF00))
        Buffer_Offset++;
    if (Buffer_Offset+2352*3+12>Buffer_Size)
        return false;

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Cdxa::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+12>Buffer_Size)
        return false;

    //Quick test of synchro
    if (!(CC8(Buffer+Buffer_Offset)==0x00FFFFFFFFFFFFFFLL && CC4(Buffer+Buffer_Offset+8)==0xFFFFFF00))
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cdxa::Streams_Finish ()
{
    if (!MI)
        return;

    //If nothing
    if (MI->Info==NULL || !MI->Info->Status[IsAccepted])
    {
        Fill(Stream_General, 0, General_Format, "CDXA");
    }
    else
    {

        //General
        MI->Info->Open_Buffer_Finalize();
        Merge(*(MI->Info));
        Merge(*(MI->Info), Stream_General, 0, 0);
        const Ztring &Format=Retrieve(Stream_General, 0, General_Format);
        Fill(Stream_General, 0, General_Format, (Ztring(__T("CDXA/"))+Format).c_str(), Unlimited, true);
        Clear(Stream_General, 0, General_Duration);
        Clear(Stream_Video, 0, Video_Duration);
    }

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
    {
        delete MI; MI=NULL;
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Cdxa::Header_Parse()
{
    //Parsing
    Skip_B4(                                                    "Sync1");
    Skip_B4(                                                    "Sync2");
    Skip_B4(                                                    "Sync3");
    Skip_B4(                                                    "Header");
    Skip_B8(                                                    "SubHeader");

    //Filling
    Header_Fill_Size(2352);
    Header_Fill_Code(0, "Chunk");
}

//---------------------------------------------------------------------------
void File_Cdxa::Data_Parse()
{
    if (MI==NULL)
    {
        //Where is the header? --> Problem
        Reject("CDXA");
        return;
    }

    //CRC or not?
    int64u CRC_Size=4;
    if (Element_Size!=2328)
        CRC_Size=0;

    //Parsing
    Skip_XX(Element_Size-CRC_Size,                              "Data");
    if (CRC_Size>0)
        Skip_B4(                                                "CRC");

    //Preparing to fill MediaInfo with a buffer
    MI->Open_Buffer_Position_Set(File_Offset+Buffer_Offset);

    //Sending the buffer to MediaInfo
    MI->Open_Buffer_Continue(Buffer+Buffer_Offset, (size_t)(Element_Size-CRC_Size));

    //Testing if filled
    if (MI->Info->Status[IsFilled]) {

    }

    //Testing if MediaInfo always need data
    File_GoTo=MI->Open_Buffer_Continue_GoTo_Get();
    if (File_GoTo==(int64u)-1 && MI->Info->Status[IsFilled] && File_Size!=(int64u)-1 && File_Offset+Buffer_Size<File_Size/2)
        GoToFromEnd(File_Offset+Buffer_Size);
    if (File_GoTo!=(int64u)-1)
        Info("CDXA, Jumping to end of file");

    //Details
    #if MEDIAINFO_TRACE
    if (Config_Trace_Level)
    {
        if (!MI->Inform().empty())
            Element_Show_Add(MI->Inform());
    }
    #endif //MEDIAINFO_TRACE

    //Demux
    Demux(Buffer+Buffer_Offset, (size_t)(Element_Size-CRC_Size), ContentType_MainStream);
}

} //NameSpace

#endif //MEDIAINFO_CDXA_YES

