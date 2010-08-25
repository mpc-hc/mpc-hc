// MediaInfo_Internal - All info about media files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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
// For user: you can disable or enable it
//#define MEDIAINFO_DEBUG
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Reader/Reader_File.h"
#include "MediaInfo/File__Analyze.h"
#include "ZenLib/FileName.h"
#include "ZenLib/File.h"
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------
// Debug stuff
#ifdef MEDIAINFO_DEBUG
    int64u Reader_File_Offset=0;
    int64u Reader_File_BytesRead_Total=0;
    int64u Reader_File_BytesRead=0;
    int64u Reader_File_Count=1;
    #include <iostream>
#endif // MEDIAINFO_DEBUG
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

const size_t Buffer_NormalSize=/*188*7;//*/64*1024;
const size_t Buffer_NoJump=128*1024;

//---------------------------------------------------------------------------
size_t Reader_File::Format_Test(MediaInfo_Internal* MI, const String &File_Name)
{
    #if MEDIAINFO_EVENTS
        {
            struct MediaInfo_Event_General_Start_0 Event;
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Start, 0);
            Event.Stream_Size=File::Size_Get(File_Name);
            MI->Config.Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_Start_0));
        }
    #endif //MEDIAINFO_EVENTS

    //With Parser MultipleParsing
    /*
    MI->Open_Buffer_Init((int64u)-1, File_Name);
    if (Format_Test_PerParser(MI, File_Name))
         return 1;
    return 0; //There is a problem
    */

    //Get the Extension
    Ztring Extension=FileName::Extension_Get(File_Name);
    Extension.MakeLowerCase();

    //Search the theorical format from extension
    InfoMap &FormatList=MediaInfoLib::Config.Format_Get();
    InfoMap::iterator Format=FormatList.begin();
    while (Format!=FormatList.end())
    {
        const Ztring &Extensions=FormatList.Get(Format->first, InfoFormat_Extensions);
        if (Extensions.find(Extension)!=Error)
        {
            if(Extension.size()==Extensions.size())
                break; //Only one extenion in the list
            if(Extensions.find(Extension+_T(" "))!=Error
            || Extensions.find(_T(" ")+Extension)!=Error)
                break;
        }
        Format++;
    }
    if (Format!=FormatList.end())
    {
        const Ztring &Parser=Format->second(InfoFormat_Parser);
        if (MI->SelectFromExtension(Parser))
        {
            //Test the theorical format
            if (Format_Test_PerParser(MI, File_Name)>0)
                 return 1;
        }
    }

    size_t ToReturn=MI->ListFormats(File_Name);
    return ToReturn;
}

//---------------------------------------------------------------------------
size_t Reader_File::Format_Test_PerParser(MediaInfo_Internal* MI, const String &File_Name)
{
    //Opening the file
    File F;
    F.Open(File_Name);
    if (!F.Opened_Get())
        return 0;

    //Buffer
    size_t Buffer_Size_Max=Buffer_NormalSize;
    int8u* Buffer=new int8u[Buffer_Size_Max];

    //Parser
    MI->Open_Buffer_Init(F.Size_Get(), File_Name);

    //Test the format with buffer
    bool StopAfterFilled=MI->Config.File_StopAfterFilled_Get();
    std::bitset<32> Status;
    do
    {
        //Seek (if needed)
        if (MI->Open_Buffer_Continue_GoTo_Get()!=(int64u)-1)
        {
            #ifdef MEDIAINFO_DEBUG
                std::cout<<std::hex<<Reader_File_Offset<<" - "<<Reader_File_Offset+Reader_File_BytesRead<<" : "<<std::dec<<Reader_File_BytesRead<<" bytes"<<std::endl;
                Reader_File_Offset=MI->Open_Buffer_Continue_GoTo_Get();
                Reader_File_BytesRead=0;
                Reader_File_Count++;
            #endif //MEDIAINFO_DEBUG

            if (MI->Open_Buffer_Continue_GoTo_Get()>=F.Size_Get())
                break; //Seek requested, but on a file bigger in theory than what is in the real file, we can't do this
            if (!(MI->Open_Buffer_Continue_GoTo_Get()>F.Position_Get() && MI->Open_Buffer_Continue_GoTo_Get()<F.Position_Get()+Buffer_NoJump)) //No smal jumps
            {
                 if (!F.GoTo(MI->Open_Buffer_Continue_GoTo_Get()))
                    break; //File is not seekable

                MI->Open_Buffer_Init((int64u)-1, F.Position_Get());
            }
        }

        //Buffering
        size_t Buffer_Size=F.Read(Buffer, Buffer_Size_Max);
        if (Buffer_Size==0)
            break; //Problem while reading

        #ifdef MEDIAINFO_DEBUG
            Reader_File_BytesRead_Total+=Buffer_Size;
            Reader_File_BytesRead+=Buffer_Size;
        #endif //MEDIAINFO_DEBUG

        //Parser
        Status=MI->Open_Buffer_Continue(Buffer, Buffer_Size);

        //Threading
        if (MI->IsTerminating())
            break; //Termination is requested
    }
    while (!(Status[File__Analyze::IsFinished] || (StopAfterFilled && Status[File__Analyze::IsFilled])));
    if (F.Size_Get()==0) //If Size==0, Status is never updated
        Status=MI->Open_Buffer_Continue(NULL, 0);

    #ifdef MEDIAINFO_DEBUG
        std::cout<<std::hex<<Reader_File_Offset<<" - "<<Reader_File_Offset+Reader_File_BytesRead<<" : "<<std::dec<<Reader_File_BytesRead<<" bytes"<<std::endl;
        std::cout<<"Total: "<<std::dec<<Reader_File_BytesRead_Total<<" bytes in "<<Reader_File_Count<<" blocks"<<std::endl;
    #endif //MEDIAINFO_DEBUG

    //File
    F.Close();

    //Buffer
    delete[] Buffer; //Buffer=NULL;

    //Is this file detected?
    if (!Status[File__Analyze::IsAccepted])
        return 0;

    MI->Open_Buffer_Finalize();
    return 1;
}

} //NameSpace


