/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// For user: you can disable or enable it
//#define MEDIAINFO_DEBUG
//---------------------------------------------------------------------------

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
#include "MediaInfo/Reader/Reader_File.h"
#include "MediaInfo/File__Analyze.h"
#include "ZenLib/FileName.h"
#ifdef WINDOWS
    #undef __TEXT
    #include "Windows.h"
#endif //WINDOWS
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

const size_t Buffer_NoJump=128*1024;

//---------------------------------------------------------------------------
size_t Reader_File::Format_Test(MediaInfo_Internal* MI, String File_Name)
{
    //std::cout<<Ztring(File_Name).To_Local().c_str()<<std::endl;
    #if MEDIAINFO_EVENTS
        {
            struct MediaInfo_Event_General_Start_0 Event;
            memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Start, 0);
            Event.EventSize=sizeof(struct MediaInfo_Event_General_Start_0);
            Event.StreamIDs_Size=0;
            Event.Stream_Size=File::Size_Get(File_Name);
            Event.FileName=NULL;
            Event.FileName_Unicode=NULL;
            MI->Config.Event_Send(NULL, (const int8u*)&Event, sizeof(MediaInfo_Event_General_Start_0));
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
    InfoMap::iterator Format=FormatList.end();
    if (!MI->Config.File_ForceParser_Get().empty())
        Format=FormatList.find(MI->Config.File_ForceParser_Get());
    if (Format==FormatList.end())
    {
        Format=FormatList.begin();
        while (Format!=FormatList.end())
        {
            const Ztring &Extensions=FormatList.Get(Format->first, InfoFormat_Extensions);
            if (Extensions.find(Extension)!=Error)
            {
                if(Extension.size()==Extensions.size())
                    break; //Only one extenion in the list
                if(Extensions.find(Extension+__T(" "))!=Error
                || Extensions.find(__T(" ")+Extension)!=Error)
                    break;
            }
            ++Format;
        }
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
    F.Open(File_Name);
    if (!F.Opened_Get())
        return 0;

    //Info
    Status=0;
    MI->Config.File_Size=F.Size_Get();
    MI->Config.File_Current_Offset=0;
    MI->Config.File_Current_Size=MI->Config.File_Size;
    MI->Config.File_Sizes.clear();
    MI->Config.File_Sizes.push_back(MI->Config.File_Size);
    if (MI->Config.File_Names.size()>1
        #if MEDIAINFO_ADVANCED
            && !MI->Config.File_IgnoreSequenceFileSize_Get()
        #endif //MEDIAINFO_ADVANCED
            )
    {
        for (size_t Pos=1; Pos<MI->Config.File_Names.size(); Pos++)
        {
            int64u Size=File::Size_Get(MI->Config.File_Names[Pos]);
            MI->Config.File_Sizes.push_back(Size);
            MI->Config.File_Size+=Size;
        }
    }

    //Partial file handling
    Ztring Config_Partial_Begin=MI->Config.File_Partial_Begin_Get();
    if (!Config_Partial_Begin.empty() && Config_Partial_Begin[0]>=__T('0') && Config_Partial_Begin[0]<=__T('9'))
    {
        if (Config_Partial_Begin.find(__T('%'))==Config_Partial_Begin.size()-1)
            Partial_Begin=float64_int64s(MI->Config.File_Size*Config_Partial_Begin.To_float64()/100);
        else
            Partial_Begin=Config_Partial_Begin.To_int64u();
        if (Partial_Begin)
            F.GoTo(Partial_Begin);
    }
    else
        Partial_Begin=0;
    Ztring Config_Partial_End=MI->Config.File_Partial_End_Get();
    if (!Config_Partial_End.empty() && Config_Partial_End[0]>=__T('0') && Config_Partial_End[0]<=__T('9'))
    {
        if (Config_Partial_End.find(__T('%'))==Config_Partial_End.size()-1)
            Partial_End=float64_int64s(MI->Config.File_Size*Config_Partial_End.To_float64()/100);
        else
            Partial_End=Config_Partial_End.To_int64u();
    }
    else
        Partial_End=(int64u)-1;
    if (Partial_Begin>MI->Config.File_Size)
        Partial_Begin=0; //Wrong value
    if (Partial_Begin>Partial_End)
        Partial_Begin=0; //Wrong value

    //Parser
    MI->Open_Buffer_Init((Partial_End<=MI->Config.File_Size?Partial_End:MI->Config.File_Size)-Partial_Begin, File_Name);

    //Buffer
    MI->Option(__T("File_Buffer_Size_Hint_Pointer"), Ztring::ToZtring((size_t)(&MI->Config.File_Buffer_Size_ToRead)));
    MI->Config.File_Buffer_Repeat_IsSupported=true;

    //Test the format with buffer
    return Format_Test_PerParser_Continue(MI);
}

//---------------------------------------------------------------------------
size_t Reader_File::Format_Test_PerParser_Continue (MediaInfo_Internal* MI)
{
    bool StopAfterFilled=MI->Config.File_StopAfterFilled_Get();
    bool ShouldContinue=true;

    //Previous data
    if (MI->Config.File_Buffer_Repeat)
    {
        MI->Config.File_Buffer_Repeat=false;
        #if MEDIAINFO_DEMUX
            MI->Config.Demux_EventWasSent=false;
        #endif //MEDIAINFO_DEMUX

        Status=MI->Open_Buffer_Continue(MI->Config.File_Buffer, MI->Config.File_Buffer_Size);

        #if MEDIAINFO_DEMUX
            //Demux
            if (MI->Config.Demux_EventWasSent)
                return 2; //Must return immediately
        #endif //MEDIAINFO_DEMUX

        //Threading
        if (MI->IsTerminating())
            return 1; //Termination is requested

        if (Status[File__Analyze::IsFinished] || (StopAfterFilled && Status[File__Analyze::IsFilled]))
            ShouldContinue=false;
    }

    #if MEDIAINFO_DEMUX
    //PerPacket
    if (ShouldContinue && MI->Config.Demux_EventWasSent)
    {
        MI->Config.Demux_EventWasSent=false;

        Status=MI->Open_Buffer_Continue(NULL, 0);

        //Demux
        if (MI->Config.Demux_EventWasSent)
            return 2; //Must return immediately

        //Threading
        if (MI->IsTerminating())
            return 1; //Termination is requested

        if (Status[File__Analyze::IsFinished] || (StopAfterFilled && Status[File__Analyze::IsFilled]))
            ShouldContinue=false;
    }
    #endif //MEDIAINFO_DEMUX

    if (ShouldContinue)
    {
        //Test the format with buffer
        while (!(Status[File__Analyze::IsFinished] || (StopAfterFilled && Status[File__Analyze::IsFilled])))
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

                int64u GoTo=Partial_Begin+MI->Open_Buffer_Continue_GoTo_Get();
                MI->Config.File_Current_Offset=0;
                int64u Buffer_NoJump_Temp=Buffer_NoJump;
                if (MI->Config.File_Names.size()>1)
                {
                    size_t Pos;
                    for (Pos=0; Pos<MI->Config.File_Sizes.size(); Pos++)
                    {
                        if (GoTo>=MI->Config.File_Sizes[Pos])
                        {
                            GoTo-=MI->Config.File_Sizes[Pos];
                            MI->Config.File_Current_Offset+=MI->Config.File_Sizes[Pos];
                        }
                        else
                            break;
                    }
                    if (Pos>=MI->Config.File_Sizes.size())
                        break;
                    if (Pos!=MI->Config.File_Names_Pos-1)
                    {
                        F.Close();
                        F.Open(MI->Config.File_Names[Pos]);
                        MI->Config.File_Names_Pos=Pos+1;
                        MI->Config.File_Current_Size=MI->Config.File_Current_Offset+F.Size_Get();
                        Buffer_NoJump_Temp=0;
                    }
                }

                if (GoTo>=F.Size_Get())
                    break; //Seek requested, but on a file bigger in theory than what is in the real file, we can't do this
                if (!(GoTo>F.Position_Get() && GoTo<F.Position_Get()+Buffer_NoJump_Temp)) //No smal jumps
                {
                     if (!F.GoTo(GoTo))
                        break; //File is not seekable

                    MI->Open_Buffer_Init((int64u)-1, MI->Config.File_Current_Offset+F.Position_Get()-Partial_Begin);
                }
            }

            //Handling of hints
            if (MI->Config.File_Buffer_Size_ToRead==0)
                break; //Problem while config
            if (MI->Config.File_Buffer_Size_ToRead>MI->Config.File_Buffer_Size_Max)
            {
                delete[] MI->Config.File_Buffer;
                if (MI->Config.File_Buffer_Size_Max==0)
                    MI->Config.File_Buffer_Size_Max=1;
                while (MI->Config.File_Buffer_Size_ToRead>MI->Config.File_Buffer_Size_Max)
                    MI->Config.File_Buffer_Size_Max*=2;
                MI->Config.File_Buffer=new int8u[MI->Config.File_Buffer_Size_Max];
            }

            //Testing multiple file per stream
            if (F.Position_Get()>=F.Size_Get())
            {
                if (MI->Config.File_Names_Pos<MI->Config.File_Names.size())
                {
                    MI->Config.File_Current_Offset+=F.Size_Get();
                    F.Close();
                    #if MEDIAINFO_EVENTS
                        MI->Config.Event_SubFile_Start(MI->Config.File_Names[MI->Config.File_Names_Pos]);
                    #endif //MEDIAINFO_EVENTS
                    F.Open(MI->Config.File_Names[MI->Config.File_Names_Pos]);
                    MI->Config.File_Names_Pos++;
                    MI->Config.File_Current_Size+=F.Size_Get();
                }
            }

            MI->Config.File_Buffer_Size=F.Read(MI->Config.File_Buffer, (F.Position_Get()+MI->Config.File_Buffer_Size_ToRead<(Partial_End<=MI->Config.File_Size?Partial_End:MI->Config.File_Size))?MI->Config.File_Buffer_Size_ToRead:((size_t)((Partial_End<=MI->Config.File_Size?Partial_End:MI->Config.File_Size)-F.Position_Get())));

            //Testing growing files
            int64u Growing_Temp=(int64u)-1;
            if (MI->Config.ParseSpeed>=1.0 && !MI->Config.File_IsGrowing && MI->Config.File_Current_Offset+F.Position_Get()>=MI->Config.File_Size)
            {
                if (MI->Config.File_Names.size()==1)
                {
                    Growing_Temp=F.Size_Get();
                    if (MI->Config.File_Size!=Growing_Temp)
                        MI->Config.File_IsGrowing=true;
                }
                else if (MI->Config.File_TestContinuousFileNames_Get())
                {
                    Growing_Temp=MI->Config.File_Names.size();
                    MI->TestContinuousFileNames();
                    if (MI->Config.File_Names.size()!=Growing_Temp)
                        MI->Config.File_IsGrowing=true;
                }
            }
            if (MI->Config.File_IsNotGrowingAnymore)
            {
                MI->Config.File_Current_Size=MI->Config.File_Size=F.Size_Get();
                MI->Open_Buffer_Init(MI->Config.File_Size, F.Position_Get()-MI->Config.File_Buffer_Size);
                MI->Config.File_IsGrowing=false;
                MI->Config.File_IsNotGrowingAnymore=false;
            }
            if (MI->Config.File_IsGrowing && (Growing_Temp!=(int64u)-1 || MI->Config.File_Current_Offset+F.Position_Get()>=MI->Config.File_Size))
            {
                for (size_t CountOfSeconds=0; CountOfSeconds<(size_t)MI->Config.File_GrowingFile_Delay_Get(); CountOfSeconds++)
                {
                    if (MI->Config.File_Names.size()==1)
                    {
                        Growing_Temp=F.Size_Get();
                        if (MI->Config.File_Size!=Growing_Temp)
                        {
                            MI->Config.File_Current_Size=MI->Config.File_Size=Growing_Temp;
                            MI->Open_Buffer_Init(MI->Config.File_Size, MI->Config.File_Current_Offset+F.Position_Get()-MI->Config.File_Buffer_Size);
                            break;
                        }
                    }
                    else
                    {
                        Growing_Temp=MI->Config.File_Names.size();
                        MI->TestContinuousFileNames();
                        if (MI->Config.File_Names.size()!=Growing_Temp)
                        {
                            MI->Open_Buffer_Init(MI->Config.File_Size, MI->Config.File_Current_Offset+F.Position_Get()-MI->Config.File_Buffer_Size);
                            break;
                        }
                    }
                    #ifdef WINDOWS
                        Sleep(1000);
                    #endif //WINDOWS
                }
            }

            #ifdef MEDIAINFO_DEBUG
                Reader_File_BytesRead_Total+=MI->Config.File_Buffer_Size;
                Reader_File_BytesRead+=MI->Config.File_Buffer_Size;
            #endif //MEDIAINFO_DEBUG

            //Parser
            Status=MI->Open_Buffer_Continue(MI->Config.File_Buffer, MI->Config.File_Buffer_Size);

            if (MI->Config.File_Buffer_Size==0)
            {
                #if MEDIAINFO_EVENTS
                    MediaInfoLib::Config.Log_Send(0xC0, 0xFF, 0xF0F00101, "File read error");
                #endif //MEDIAINFO_EVENTS
                break;
            }

            #if MEDIAINFO_DEMUX
                if (MI->Config.Demux_EventWasSent)
                    return 2; //Must return immediately
            #endif //MEDIAINFO_DEMUX

            //Threading
            if (MI->IsTerminating())
                break; //Termination is requested
        }
    }

    //Deleting buffer
    delete[] MI->Config.File_Buffer; MI->Config.File_Buffer=NULL;
    MI->Config.File_Buffer_Size_Max=0;

    #ifdef MEDIAINFO_DEBUG
        std::cout<<std::hex<<Reader_File_Offset<<" - "<<Reader_File_Offset+Reader_File_BytesRead<<" : "<<std::dec<<Reader_File_BytesRead<<" bytes"<<std::endl;
        std::cout<<"Total: "<<std::dec<<Reader_File_BytesRead_Total<<" bytes in "<<Reader_File_Count<<" blocks"<<std::endl;
    #endif //MEDIAINFO_DEBUG

    if (MI==NULL || !MI->Config.File_KeepInfo_Get())
    {
        //File
        F.Close();
    }

    //Is this file detected?
    if (!Status[File__Analyze::IsAccepted])
        return 0;

    MI->Open_Buffer_Finalize();

    #if MEDIAINFO_DEMUX
        if (MI->Config.Demux_EventWasSent)
            return 2; //Must return immediately
    #endif //MEDIAINFO_DEMUX

    return 1;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t Reader_File::Format_Test_PerParser_Seek (MediaInfo_Internal* MI, size_t Method, int64u Value, int64u ID)
{
    size_t ToReturn=MI->Open_Buffer_Seek(Method, Value, ID);

    if (ToReturn==0 || ToReturn==1)
    {
        //Reset
        Status=0;
    }

    return ToReturn;
}
#endif //MEDIAINFO_SEEK

} //NameSpace
