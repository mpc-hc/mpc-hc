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

#if MEDIAINFO_READTHREAD
void Reader_File_Thread::Entry()
{
    ReadSize_Max=Base->Buffer_Max>>3;

    for (;;)
    {
        Base->CS.Enter();
        if (Base->Buffer_Begin==Base->Buffer_Max)
        {
            Base->IsLooping=false;
            Base->Buffer_End=Base->Buffer_End2;
            Base->Buffer_End2=0;
            Base->Buffer_Begin=0;
        }

        size_t ToRead;
        size_t Buffer_ToReadOffset;
        if (Base->IsLooping)
        {
            ToRead=Base->Buffer_Begin-Base->Buffer_End2;
            Buffer_ToReadOffset=Base->Buffer_End2;
        }
        else
        {
            ToRead=Base->Buffer_Max-Base->Buffer_End;
            Buffer_ToReadOffset=Base->Buffer_End;
        }
        Base->CS.Leave();

        if (ToRead)
        {
            if (ToRead>ReadSize_Max)
                ToRead=ReadSize_Max;
            size_t BytesRead=Base->F.Read(Base->Buffer+Buffer_ToReadOffset, ToRead);
            if (!BytesRead)
                break;

            Base->CS.Enter();
            if (Base->IsLooping)
            {
                Base->Buffer_End2+=BytesRead;
            }
            else
            {
                Base->Buffer_End+=BytesRead;
                if (Base->Buffer_End==Base->Buffer_Max)
                {
                    Base->IsLooping=true;
                }
            }
            Base->CS.Leave();

            #ifdef WINDOWS
                SetEvent(Base->Condition_WaitingForMoreData);
            #endif //WINDOWS
        }
        #ifdef WINDOWS
            else
                WaitForSingleObject(Base->Condition_WaitingForMorePlace, INFINITE);
        #endif //WINDOWS

        if (IsTerminating())
            break;
        Yield();
    }

    #ifdef WINDOWS
        SetEvent(Base->Condition_WaitingForMoreData); //Sending the last event in case the main threading is waiting for more data
    #endif //WINDOWS
}
#endif //MEDIAINFO_READTHREAD

const size_t Buffer_NoJump=128*1024;

//---------------------------------------------------------------------------
Reader_File::~Reader_File()
{
    #if MEDIAINFO_READTHREAD
        if (ThreadInstance)
        {
            ThreadInstance->RequestTerminate();
            SetEvent(Condition_WaitingForMorePlace);
            while (!ThreadInstance->IsExited())
                Sleep(0);
            #ifdef WINDOWS
                CloseHandle(Condition_WaitingForMorePlace);
                CloseHandle(Condition_WaitingForMoreData);
            #endif //WINDOWS
            delete ThreadInstance;

            MI_Internal->Config.File_Buffer=NULL;
            MI_Internal->Config.File_Buffer_Size=0;
            MI_Internal->Config.File_Buffer_Size_Max=0;
            delete[] Buffer;
        }
    #endif //MEDIAINFO_READTHREAD
}

//---------------------------------------------------------------------------
size_t Reader_File::Format_Test(MediaInfo_Internal* MI, String File_Name)
{
    //std::cout<<Ztring(File_Name).To_Local().c_str()<<std::endl;
    #if MEDIAINFO_EVENTS
        {
            string File_Name_Local=Ztring(File_Name).To_Local();
            wstring File_Name_Unicode=Ztring(File_Name).To_Unicode();
            struct MediaInfo_Event_General_Start_0 Event;
            memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Start, 0);
            Event.EventSize=sizeof(struct MediaInfo_Event_General_Start_0);
            Event.StreamIDs_Size=0;
            Event.Stream_Size=File::Size_Get(File_Name);
            Event.FileName=File_Name_Local.c_str();
            Event.FileName_Unicode=File_Name_Unicode.c_str();
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
    //Init
    MI_Internal=MI;
    #if MEDIAINFO_READTHREAD
        ThreadInstance=NULL;
        Buffer_End2=0; //Is also used for counting bytes before activating the thread
    #endif //MEDIAINFO_READTHREAD

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
    if (MI->Config.File_Names.size()>1)
    {
        #if MEDIAINFO_ADVANCED
            if (MI->Config.File_IgnoreSequenceFileSize_Get())
            {
                MI->Config.File_Size=(int64u)-1;
            }
            else
        #endif //MEDIAINFO_ADVANCED
            {
                for (size_t Pos=1; Pos<MI->Config.File_Names.size(); Pos++)
                {
                    int64u Size=File::Size_Get(MI->Config.File_Names[Pos]);
                    MI->Config.File_Sizes.push_back(Size);
                    MI->Config.File_Size+=Size;
                }
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
    if (MI == NULL)
        return 0;

    bool StopAfterFilled=MI->Config.File_StopAfterFilled_Get();
    bool ShouldContinue=true;
    if (MI->Info)
        Status=MI->Info->Status;

    //Previous data
    if (MI->Config.File_Buffer_Repeat)
    {
        MI->Config.File_Buffer_Repeat=false;
        #if MEDIAINFO_DEMUX
            MI->Config.Demux_EventWasSent=false;
        #endif //MEDIAINFO_DEMUX

        Status=MI->Open_Buffer_Continue(MI->Config.File_Buffer, MI->Config.File_Buffer_Size);

        #if MEDIAINFO_READTHREAD
            if (ThreadInstance && !MI->Config.File_Buffer_Repeat)
            {
                CS.Enter();
                Buffer_Begin+=MI->Config.File_Buffer_Size;
                #ifdef WINDOWS
                    if (Buffer_Begin==Buffer_Max)
                    {
                        CS.Leave();
                        SetEvent(Condition_WaitingForMorePlace);
                    }
                    else
                #endif //WINDOWS
                        CS.Leave();
            }
        #endif //MEDIAINFO_READTHREAD

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

                #if MEDIAINFO_READTHREAD
                    if (ThreadInstance)
                    {
                        ThreadInstance->RequestTerminate();
                        SetEvent(Condition_WaitingForMorePlace);
                        while (!ThreadInstance->IsExited())
                            Sleep(0);
                        #ifdef WINDOWS
                            CloseHandle(Condition_WaitingForMorePlace);
                            CloseHandle(Condition_WaitingForMoreData);
                        #endif //WINDOWS
                        delete ThreadInstance; ThreadInstance=NULL;

                        MI->Config.File_Buffer=NULL;
                        MI->Config.File_Buffer_Size=0;
                        MI->Config.File_Buffer_Size_Max=0;
                        Buffer_Max=0;
                        delete[] Buffer; Buffer=NULL;
                        Buffer_Begin=0;
                        Buffer_End=0;
                        Buffer_End2=0;
                        IsLooping=false;
                    }
                    if (Buffer_End2!=(size_t)-1)
                        Buffer_End2=0;
                #endif //MEDIAINFO_READTHREAD

                int64u GoTo=Partial_Begin+MI->Open_Buffer_Continue_GoTo_Get();
                MI->Config.File_Current_Offset=0;
                int64u Buffer_NoJump_Temp=Buffer_NoJump;
                if (MI->Config.File_Names.size()>1)
                {
                    size_t Pos;
                    #if MEDIAINFO_SEEK
                        if (MI->Config.File_GoTo_IsFrameOffset)
                        {
                            Pos=(size_t)MI->Open_Buffer_Continue_GoTo_Get(); //File_GoTo is the frame offset in that case
                            MI->Info->File_GoTo=(int64u)-1;
                            MI->Config.File_GoTo_IsFrameOffset=false;
                            GoTo=0;
                        }
                        else
                    #endif //MEDIAINFO_SEEK
                    {
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
                    }
                    if (Pos!=MI->Config.File_Names_Pos-1)
                    {
                        F.Close();
                        F.Open(MI->Config.File_Names[Pos]);
                        if (Pos>=MI->Config.File_Sizes.size())
                        {
                            MI->Config.File_Sizes.resize(Pos, (int64u)-1);
                            MI->Config.File_Sizes.push_back(F.Size_Get());
                        }
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

            #if MEDIAINFO_READTHREAD
                if (ThreadInstance==NULL && Buffer_End2!=(size_t)-1 && Buffer_End2>=16*1024*1024)
                {
                    if (!MI->Config.File_IsGrowing && MI->Config.File_Names.size()==1)
                    {
                        delete[] MI->Config.File_Buffer; MI->Config.File_Buffer=NULL;
                        MI->Config.File_Buffer_Size_Max=0;
                        Buffer_Max=MI->Config.File_Buffer_Read_Size_Get();
                        Buffer=new int8u[Buffer_Max];
                        Buffer_Begin=0;
                        Buffer_End=0;
                        Buffer_End2=0;
                        IsLooping=false;
                        #ifdef WINDOWS
                            Condition_WaitingForMorePlace=CreateEvent(NULL, FALSE, FALSE, NULL);
                            Condition_WaitingForMoreData=CreateEvent(NULL, FALSE, FALSE, NULL);
                        #endif //WINDOWS
                        ThreadInstance=new Reader_File_Thread();
                        ThreadInstance->Base=this;
                        ThreadInstance->Run();
                    }
                    else
                        Buffer_End2=(size_t)-1;
                }
            #endif //MEDIAINFO_READTHREAD

            //Handling of hints
            if (MI->Config.File_Buffer_Size_ToRead==0)
                break; //Problem while config
            if (
                #if MEDIAINFO_READTHREAD
                    ThreadInstance==NULL &&
                #endif //MEDIAINFO_READTHREAD
                MI->Config.File_Buffer_Size_ToRead>MI->Config.File_Buffer_Size_Max)
            {
                delete[] MI->Config.File_Buffer;
                if (MI->Config.File_Buffer_Size_Max==0)
                    MI->Config.File_Buffer_Size_Max=1;
                while (MI->Config.File_Buffer_Size_ToRead>MI->Config.File_Buffer_Size_Max)
                    MI->Config.File_Buffer_Size_Max*=2;
                MI->Config.File_Buffer=new int8u[MI->Config.File_Buffer_Size_Max];
            }

            //Testing multiple file per stream
            if (
                #if MEDIAINFO_READTHREAD
                    ThreadInstance==NULL &&
                #endif //MEDIAINFO_READTHREAD
                F.Position_Get()>=F.Size_Get())
            {
                #if MEDIAINFO_ADVANCED2
                MI->Open_Buffer_SegmentChange();
                #endif //MEDIAINFO_ADVANCED2
                if (MI->Config.File_Names_Pos<MI->Config.File_Names.size())
                {
                    MI->Config.File_Current_Offset+=MI->Config.File_Names_Pos<=MI->Config.File_Sizes.size()?MI->Config.File_Sizes[MI->Config.File_Names_Pos-1]:F.Size_Get();
                    F.Close();
                    #if MEDIAINFO_EVENTS
                        MI->Config.Event_SubFile_Start(MI->Config.File_Names[MI->Config.File_Names_Pos]);
                    #endif //MEDIAINFO_EVENTS
                    F.Open(MI->Config.File_Names[MI->Config.File_Names_Pos]);
                    if (MI->Config.File_Names_Pos>=MI->Config.File_Sizes.size())
                    {
                        MI->Config.File_Sizes.resize(MI->Config.File_Names_Pos, (int64u)-1);
                        MI->Config.File_Sizes.push_back(F.Size_Get());
                    }
                    MI->Config.File_Names_Pos++;
                    MI->Config.File_Current_Size+=F.Size_Get();
                }
            }

            #if MEDIAINFO_READTHREAD
                if (ThreadInstance)
                {
                    CS.Enter();
                    #ifdef WINDOWS
                        if (Buffer_End2+Buffer_End-Buffer_Begin<Buffer_Max/8*7)
                        {
                            CS.Leave();
                            SetEvent(Condition_WaitingForMorePlace);
                            CS.Enter();
                        }
                    #endif //WINDOWS

                    for (;;)
                    {
                        MI->Config.File_Buffer_Size=Buffer_End-Buffer_Begin;

                        if (MI->Config.File_Buffer_Size)
                            break;

                        if (!ThreadInstance->IsExited())
                        {
                            CS.Leave();
                            #ifdef WINDOWS
                                WaitForSingleObject(Condition_WaitingForMoreData, INFINITE);
                            #else //WINDOWS
                                Sleep(0);
                            #endif //WINDOWS
                            CS.Enter();
                        }
                        else
                        {
                            if (IsLooping)
                            {
                                IsLooping=false;
                                Buffer_End=Buffer_End2;
                                Buffer_End2=0;
                                Buffer_Begin=0;
                            }

                            MI->Config.File_Buffer_Size=Buffer_End-Buffer_Begin;
                            break;
                        }
                    }
                    MI->Config.File_Buffer=Buffer+Buffer_Begin;
                    CS.Leave();
                    if (MI->Config.File_Buffer_Size>MI->Config.File_Buffer_Size_ToRead)
                        MI->Config.File_Buffer_Size=MI->Config.File_Buffer_Size_ToRead;
                }
                else
            #endif //MEDIAINFO_READTHREAD
            {
                MI->Config.File_Buffer_Size=F.Read(MI->Config.File_Buffer, (F.Position_Get()+MI->Config.File_Buffer_Size_ToRead<(Partial_End<=MI->Config.File_Size?Partial_End:MI->Config.File_Size))?MI->Config.File_Buffer_Size_ToRead:((size_t)((Partial_End<=MI->Config.File_Size?Partial_End:MI->Config.File_Size)-F.Position_Get())));
                #if MEDIAINFO_READTHREAD
                    if (ThreadInstance==NULL && Buffer_End2!=(size_t)-1)
                        Buffer_End2+=MI->Config.File_Buffer_Size;
                #endif //MEDIAINFO_READTHREAD
            }

            /* High CPU usage
            #if MEDIAINFO_EVENTS
                if (MI->Config.File_Buffer_Size)
                {
                    struct MediaInfo_Event_Global_BytesRead_0 Event;
                    memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
                    Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_Global_BytesRead, 0);
                    Event.EventSize=sizeof(struct MediaInfo_Event_Global_BytesRead_0);
                    Event.StreamIDs_Size=0;
                    Event.StreamOffset=F.Position_Get()-MI->Config.File_Buffer_Size;
                    Event.Content_Size=MI->Config.File_Buffer_Size;
                    Event.Content=MI->Config.File_Buffer;
                    MI->Config.Event_Send(NULL, (const int8u*)&Event, sizeof(MediaInfo_Event_Global_BytesRead_0));
                }
            #endif //MEDIAINFO_EVENTS
            */

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

            #if MEDIAINFO_READTHREAD
                if (ThreadInstance && !MI->Config.File_Buffer_Repeat)
                {
                    CS.Enter();
                    Buffer_Begin+=MI->Config.File_Buffer_Size;
                    #ifdef WINDOWS
                        if (Buffer_Begin==Buffer_Max)
                        {
                            CS.Leave();
                            SetEvent(Condition_WaitingForMorePlace);
                        }
                        else
                    #endif //WINDOWS
                           CS.Leave();
                }
            #endif //MEDIAINFO_READTHREAD

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
    #if MEDIAINFO_READTHREAD
        if (ThreadInstance)
        {
            ThreadInstance->RequestTerminate();
            SetEvent(Condition_WaitingForMorePlace);
            while (!ThreadInstance->IsExited())
                Sleep(0);
            #ifdef WINDOWS
                CloseHandle(Condition_WaitingForMorePlace);
                CloseHandle(Condition_WaitingForMoreData);
            #endif //WINDOWS
            delete ThreadInstance; ThreadInstance=NULL;

            MI->Config.File_Buffer=NULL;
            MI->Config.File_Buffer_Size=0;
            MI->Config.File_Buffer_Size_Max=0;
            Buffer_Max=0;
            delete[] Buffer; Buffer=NULL;
            Buffer_Begin=0;
            Buffer_End=0;
            Buffer_End2=0;
            IsLooping=false;
        }
        else
    #endif //MEDIAINFO_READTHREAD
    {
        delete[] MI->Config.File_Buffer; MI->Config.File_Buffer=NULL;
        MI->Config.File_Buffer_Size_Max=0;
    }

    #ifdef MEDIAINFO_DEBUG
        std::cout<<std::hex<<Reader_File_Offset<<" - "<<Reader_File_Offset+Reader_File_BytesRead<<" : "<<std::dec<<Reader_File_BytesRead<<" bytes"<<std::endl;
        std::cout<<"Total: "<<std::dec<<Reader_File_BytesRead_Total<<" bytes in "<<Reader_File_Count<<" blocks"<<std::endl;
    #endif //MEDIAINFO_DEBUG

    if (!MI->Config.File_KeepInfo_Get())
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
