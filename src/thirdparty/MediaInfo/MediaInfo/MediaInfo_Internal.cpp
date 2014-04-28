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
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/File__MultipleParsing.h"
#include "ZenLib/Dir.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#if defined(MEDIAINFO_DIRECTORY_YES)
    #include "MediaInfo/Reader/Reader_Directory.h"
#endif
#if defined(MEDIAINFO_FILE_YES)
    #include "MediaInfo/Reader/Reader_File.h"
#endif
#if defined(MEDIAINFO_LIBCURL_YES)
    #include "MediaInfo/Reader/Reader_libcurl.h"
#endif
#if defined(MEDIAINFO_LIBMMS_YES)
    #include "MediaInfo/Reader/Reader_libmms.h"
#endif
#if defined(MEDIAINFO_IBI_YES)
    #include "MediaInfo/Multiple/File_Ibi.h"
#endif
#include "MediaInfo/Multiple/File_Dxw.h"
#include <cmath>
#ifdef MEDIAINFO_DEBUG_WARNING_GET
    #include <iostream>
#endif //MEDIAINFO_DEBUG_WARNING_GET
#ifdef MEDIAINFO_DEBUG_BUFFER
    #include "ZenLib/FileName.h"
    #include <cstring>
#endif //MEDIAINFO_DEBUG_BUFFER
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
//To clarify the code
namespace MediaInfo_Debug_MediaInfo_Internal
{

#if defined (MEDIAINFO_DEBUG_CONFIG) || defined (MEDIAINFO_DEBUG_BUFFER) || defined (MEDIAINFO_DEBUG_OUTPUT)
    #ifdef WINDOWS
        const Char* MediaInfo_Debug_Name=__T("MediaInfo_Debug");
    #else
        const Char* MediaInfo_Debug_Name=__T("/tmp/MediaInfo_Debug");
    #endif
#endif

#ifdef MEDIAINFO_DEBUG_CONFIG
    #define MEDIAINFO_DEBUG_CONFIG_TEXT(_TOAPPEND) \
        { \
            Ztring Debug; \
            _TOAPPEND; \
            Debug+=__T("\r\n"); \
            if (!Debug_Config.Opened_Get()) \
            { \
                if (Config.File_Names.empty()) \
                    Debug_Config.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Config.txt")); \
                else \
                { \
                    Ztring File_Temp; \
                    if (Config.File_Names[0].rfind(__T('\\'))!=string::npos) \
                        File_Temp=Config.File_Names[0].substr(Config.File_Names[0].rfind(__T('\\'))+1, string::npos); \
                    else if (Config.File_Names[0].rfind(__T('/'))!=string::npos) \
                        File_Temp=Config.File_Names[0].substr(Config.File_Names[0].rfind(__T('/'))+1, string::npos); \
                    else \
                        File_Temp=Config.File_Names[0]; \
                    Debug_Config.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".")+File_Temp+__T(".Config.txt")); \
                } \
            } \
            Debug_Config.Write(Debug); \
        }
#else // MEDIAINFO_DEBUG_CONFIG
    #define MEDIAINFO_DEBUG_CONFIG_TEXT(_TOAPPEND)
#endif // MEDIAINFO_DEBUG_CONFIG

#ifdef MEDIAINFO_DEBUG_CONFIG
    #define EXECUTE_SIZE_T(_METHOD,_DEBUGB) \
        { \
            size_t ToReturn=_METHOD; \
            MEDIAINFO_DEBUG_CONFIG_TEXT(_DEBUGB) \
            return ToReturn; \
        }
#else //MEDIAINFO_DEBUG_CONFIG
    #define EXECUTE_SIZE_T(_METHOD, _DEBUGB) \
        return _METHOD;
#endif //MEDIAINFO_DEBUG_CONFIG

#ifdef MEDIAINFO_DEBUG_CONFIG
    #define EXECUTE_INT64U(_METHOD,_DEBUGB) \
        { \
            int64u ToReturn=_METHOD; \
            MEDIAINFO_DEBUG_CONFIG_TEXT(_DEBUGB) \
            return ToReturn; \
        }
#else //MEDIAINFO_DEBUG_CONFIG
    #define EXECUTE_INT64U(_METHOD, _DEBUGB) \
        return _METHOD;
#endif //MEDIAINFO_DEBUG_CONFIG

#ifdef MEDIAINFO_DEBUG_CONFIG
    #define EXECUTE_STRING(_METHOD,_DEBUGB) \
        { \
            Ztring ToReturn=_METHOD; \
            MEDIAINFO_DEBUG_CONFIG_TEXT(_DEBUGB) \
            return ToReturn; \
        }
#else //MEDIAINFO_DEBUG_CONFIG
    #define EXECUTE_STRING(_METHOD,_DEBUGB) \
        return _METHOD;
#endif //MEDIAINFO_DEBUG_CONFIG

#ifdef MEDIAINFO_DEBUG_BUFFER
    #define MEDIAINFO_DEBUG_BUFFER_SAVE(_BUFFER, _SIZE) \
        { \
            if (!Debug_Buffer_Stream.Opened_Get()) \
            { \
                if (Config.File_Names.empty()) \
                    Debug_Buffer_Stream.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Buffer.Stream.0000000000000000")); \
                else \
                { \
                    Ztring File_Temp; \
                    if (Config.File_Names[0].rfind(__T('\\'))!=string::npos) \
                        File_Temp=Config.File_Names[0].substr(Config.File_Names[0].rfind(__T('\\'))+1, string::npos); \
                    else if (Config.File_Names[0].rfind(__T('/'))!=string::npos) \
                        File_Temp=Config.File_Names[0].substr(Config.File_Names[0].rfind(__T('/'))+1, string::npos); \
                    else \
                        File_Temp=Config.File_Names[0]; \
                    Debug_Buffer_Stream.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".")+File_Temp+__T(".Buffer.Stream.0000000000000000")); \
                } \
                Debug_Buffer_Stream_Order=0; \
                if (Config.File_Names.empty()) \
                    Debug_Buffer_Sizes.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Buffer.Sizes.0000000000000000")); \
                else \
                { \
                    Ztring File_Temp; \
                    if (Config.File_Names[0].rfind(__T('\\'))!=string::npos) \
                        File_Temp=Config.File_Names[0].substr(Config.File_Names[0].rfind(__T('\\'))+1, string::npos); \
                    else if (Config.File_Names[0].rfind(__T('/'))!=string::npos) \
                        File_Temp=Config.File_Names[0].substr(Config.File_Names[0].rfind(__T('/'))+1, string::npos); \
                    else \
                        File_Temp=Config.File_Names[0]; \
                    Debug_Buffer_Sizes.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".")+File_Temp+__T(".Buffer.Sizes.0000000000000000")); \
                } \
                Debug_Buffer_Sizes_Count=0; \
            } \
            Debug_Buffer_Stream.Write(_BUFFER, _SIZE); \
            Debug_Buffer_Sizes.Write((int8u*)&_SIZE, sizeof(size_t)); \
            Debug_Buffer_Sizes_Count+=_SIZE; \
            if (Debug_Buffer_Sizes_Count>=MEDIAINFO_DEBUG_BUFFER_SAVE_FileSize) \
            { \
                Debug_Buffer_Stream.Close(); \
                Debug_Buffer_Sizes.Close(); \
                Ztring Before=Ztring::ToZtring(Debug_Buffer_Stream_Order-1); \
                while (Before.size()<16) \
                    Before.insert(0, 1, __T('0')); \
                Ztring Next=Ztring::ToZtring(Debug_Buffer_Stream_Order+1); \
                while (Next.size()<16) \
                    Next.insert(0, 1, __T('0')); \
                Debug_Buffer_Stream.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Buffer.Stream.")+Next); \
                Debug_Buffer_Sizes.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Buffer.Sizes.")+Next); \
                File::Delete(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Buffer.Stream.")+Before); \
                File::Delete(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Buffer.Sizes.")+Before); \
                Debug_Buffer_Stream_Order++; \
                Debug_Buffer_Sizes_Count=0; \
            } \
        }
#else // MEDIAINFO_DEBUG_BUFFER
    #define MEDIAINFO_DEBUG_BUFFER_SAVE(_BUFFER, _SIZE)
#endif // MEDIAINFO_DEBUG_BUFFER

#ifdef MEDIAINFO_DEBUG_OUTPUT
    #define MEDIAINFO_DEBUG_OUTPUT_INIT(_VALUE, _DEBUGB) \
        { \
            if (OptionLower==__T("file_duplicate")) \
            { \
                size_t Pos=(size_t)ToReturn2.To_int64u(); \
                if (Pos>=Debug_Output_Pos_Stream.size()) \
                { \
                    Debug_Output_Pos_Stream.resize(Pos+1); \
                    Debug_Output_Pos_Stream[Pos]=new File(); \
                    Debug_Output_Pos_Sizes.resize(Pos+1); \
                    Debug_Output_Pos_Sizes[Pos]=new File(); \
                    Debug_Output_Pos_Pointer.resize(Pos+1); \
                    Debug_Output_Pos_Pointer[Pos]=(void*)Ztring(Value).SubString(__T("memory://"), __T(":")).To_int64u(); \
                } \
            } \
            EXECUTE_STRING(_VALUE, _DEBUGB) \
        }
#else // MEDIAINFO_DEBUG_OUTPUT
    #define MEDIAINFO_DEBUG_OUTPUT_INIT(_VALUE, _DEBUGB) \
        EXECUTE_STRING(_VALUE, _DEBUGB)
#endif // MEDIAINFO_DEBUG_OUTPUT

#ifdef MEDIAINFO_DEBUG_OUTPUT
    #define MEDIAINFO_DEBUG_OUTPUT_VALUE(_VALUE, _METHOD) \
        { \
            size_t ByteCount=Info->Output_Buffer_Get(Value); \
            void* ValueH=(void*)Ztring(Value).SubString(__T("memory://"), __T(":")).To_int64u(); \
            map<void*, File>::iterator F_Stream=Debug_Output_Value_Stream.find(ValueH); \
            if (F_Stream!=Debug_Output_Value_Stream.end()) \
            { \
                map<void*, File>::iterator F_Sizes=Debug_Output_Value_Stream.find(ValueH); \
                if (!F_Stream->second.Opened_Get()) \
                { \
                    F_Stream->second.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Output.")+Ztring::ToZtring((size_t)ValueH, 16)+__T(".Stream")); \
                    F_Sizes->second.Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Output.")+Ztring::ToZtring((size_t)ValueH, 16)+__T(".Sizes")); \
                } \
                F_Stream->second.Write((int8u*)ValueH, ByteCount); \
                F_Sizes->second.Write((int8u*)&ByteCount, sizeof(ByteCount)); \
            } \
            return ByteCount; \
        }
#else // MEDIAINFO_DEBUG_OUTPUT
    #define MEDIAINFO_DEBUG_OUTPUT_VALUE(_VALUE, _METHOD) \
        return _METHOD
#endif // MEDIAINFO_DEBUG_OUTPUT

#ifdef MEDIAINFO_DEBUG_OUTPUT
    #define MEDIAINFO_DEBUG_OUTPUT_POS(_POS, _METHOD) \
        { \
            size_t ByteCount=Info->Output_Buffer_Get(_POS); \
            if (_POS<Debug_Output_Pos_Stream.size()) \
            { \
                if (!Debug_Output_Pos_Stream[_POS]->Opened_Get()) \
                { \
                    Debug_Output_Pos_Stream[_POS]->Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Output.")+Ztring::ToZtring(Pos, 16)+__T(".Stream")); \
                    Debug_Output_Pos_Sizes[_POS]->Create(Ztring(MediaInfo_Debug_Name)+__T(".")+Ztring::ToZtring((size_t)this, 16)+__T(".Output.")+Ztring::ToZtring(Pos, 16)+__T(".Sizes")); \
                } \
                Debug_Output_Pos_Stream[_POS]->Write((int8u*)Debug_Output_Pos_Pointer[_POS], ByteCount); \
                Debug_Output_Pos_Sizes[_POS]->Write((int8u*)&ByteCount, sizeof(ByteCount)); \
            } \
            return ByteCount; \
        }
#else // MEDIAINFO_DEBUG_OUTPUT
    #define MEDIAINFO_DEBUG_OUTPUT_POS(_VALUE, _METHOD) \
        return _METHOD
#endif // MEDIAINFO_DEBUG_OUTPUT

}
using namespace MediaInfo_Debug_MediaInfo_Internal;

//***************************************************************************
// Constructor/destructor
//***************************************************************************

//---------------------------------------------------------------------------
MediaInfo_Internal::MediaInfo_Internal()
: Thread()
{
    CriticalSectionLocker CSL(CS);

    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Construction");)

    MediaInfoLib::Config.Init(); //Initialize Configuration

    BlockMethod=BlockMethod_Local;
    Info=NULL;
    #if !defined(MEDIAINFO_READER_NO)
        Reader=NULL;
    #endif //!defined(MEDIAINFO_READER_NO)
    Info_IsMultipleParsing=false;

    Stream.resize(Stream_Max);
    Stream_More.resize(Stream_Max);

    //Position in a MediaInfoList class
    IsFirst=true;
    IsLast=true;

    //Threading
    BlockMethod=0;
    IsInThread=false;
}

//---------------------------------------------------------------------------
MediaInfo_Internal::~MediaInfo_Internal()
{
    Close();

    CriticalSectionLocker CSL(CS);

    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Destruction");)

    delete Info; //Info=NULL;
    #if !defined(MEDIAINFO_READER_NO)
        delete Reader; //Reader=NULL;
    #endif //!defined(MEDIAINFO_READER_NO)
    #ifdef MEDIAINFO_DEBUG_OUTPUT
        for (size_t Pos=0; Pos<Debug_Output_Pos_Stream.size(); Pos++)
        {
            delete Debug_Output_Pos_Stream[Pos]; //Debug_Output_Pos_Stream[Pos]=NULL;
            delete Debug_Output_Pos_Sizes[Pos]; //Debug_Output_Pos_Sizes[Pos]=NULL;
        }
    #endif //MEDIAINFO_DEBUG_OUTPUT
}

//***************************************************************************
// Files
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open(const String &File_Name_)
{
    Close();

    //External IBI
    #if defined(MEDIAINFO_IBI_YES)
        if (Config.Ibi_UseIbiInfoIfAvailable_Get())
        {
            std::string IbiFile=Config.Ibi_Get();
            if (!IbiFile.empty())
            {
                Info=new File_Ibi();
                Open_Buffer_Init(IbiFile.size(), File_Name_);
                Open_Buffer_Continue((const int8u*)IbiFile.c_str(), IbiFile.size());
                Open_Buffer_Finalize();

                if (!Get(Stream_General, 0, __T("Format")).empty() && Get(Stream_General, 0, __T("Format"))!=__T("Ibi"))
                    return 1;

                //Nothing interesting
                Close();
            }
        }
    #endif //MEDIAINFO_IBI_YES

    CS.Enter();
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Open, File=");Debug+=Ztring(File_Name_).c_str();)
    Config.File_Names.clear();
    if (Config.File_FileNameFormat_Get()==__T("CSV"))
    {
        Config.File_Names.Separator_Set(0, __T(","));
        Config.File_Names.Write(File_Name_);
    }
    else if (!File_Name_.empty())
        Config.File_Names.push_back(File_Name_);
    if (Config.File_Names.empty())
    {
        CS.Leave();
        return 0;
    }
    Config.File_Names_Pos=1;
    CS.Leave();

    //Parsing
    if (BlockMethod==1)
    {
        if (!IsInThread) //If already created, the routine will read the new files
        {
            Run();
            IsInThread=true;
        }
        return 0;
    }
    else
    {
        Entry(); //Normal parsing
        return Count_Get(Stream_General);
    }
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::Entry()
{
    CS.Enter();
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Entry");)
    Config.State_Set(0);
    CS.Leave();

    if ((Config.File_Names[0].size()>=6
        && Config.File_Names[0][0]==__T('m')
        && Config.File_Names[0][1]==__T('m')
        && Config.File_Names[0][2]==__T('s')
        && Config.File_Names[0][3]==__T(':')
        && Config.File_Names[0][4]==__T('/')
        && Config.File_Names[0][5]==__T('/'))
        || (Config.File_Names[0].size()>=7
        && Config.File_Names[0][0]==__T('m')
        && Config.File_Names[0][1]==__T('m')
        && Config.File_Names[0][2]==__T('s')
        && Config.File_Names[0][3]==__T('h')
        && Config.File_Names[0][4]==__T(':')
        && Config.File_Names[0][5]==__T('/')
        && Config.File_Names[0][6]==__T('/')))
        #if defined(MEDIAINFO_LIBMMS_YES)
            Reader_libmms().Format_Test(this, Config.File_Names[0]);
        #else //MEDIAINFO_LIBMMS_YES
            {
            #if MEDIAINFO_EVENTS
                struct MediaInfo_Event_Log_0 Event;
                Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_Log, 0);
                Event.Type=0xC0;
                Event.Severity=0xFF;
                Event.MessageCode=0;
                Event.MessageStringU=L"Libmms cupport is disabled due to compilation options";
                Event.MessageStringA="Libmms cupport is disabled due to compilation options";
                MediaInfoLib::Config.Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_Log_0));
            #endif //MEDIAINFO_EVENTS
            }
        #endif //MEDIAINFO_LIBMMS_YES

    else if (Config.File_Names[0].find(__T("://"))!=string::npos)
        #if defined(MEDIAINFO_LIBCURL_YES)
        {
            CS.Enter();
            if (Reader)
            {
                CS.Leave();
                return; //There is a problem
            }
            Reader=new Reader_libcurl();
            CS.Leave();

            Reader->Format_Test(this, Config.File_Names[0]);

            #if MEDIAINFO_NEXTPACKET
                if (Config.NextPacket_Get())
                    return;
            #endif //MEDIAINFO_NEXTPACKET
        }
        #else //MEDIAINFO_LIBCURL_YES
            {
            #if MEDIAINFO_EVENTS
                struct MediaInfo_Event_Log_0 Event;
                Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_Log, 0);
                Event.Type=0xC0;
                Event.Severity=0xFF;
                Event.MessageCode=0;
                Event.MessageStringU=L"Libcurl support is disabled due to compilation options";
                Event.MessageStringA="Libcurl support is disabled due to compilation options";
                MediaInfoLib::Config.Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_Log_0));
            #endif //MEDIAINFO_EVENTS
            }
        #endif //MEDIAINFO_LIBCURL_YES

    #if defined(MEDIAINFO_DIRECTORY_YES)
        else if (Dir::Exists(Config.File_Names[0]))
            Reader_Directory().Format_Test(this, Config.File_Names[0]);
    #endif //MEDIAINFO_DIRECTORY_YES

    #if defined(MEDIAINFO_FILE_YES)
        else if (File::Exists(Config.File_Names[0]))
        {
            #if defined(MEDIAINFO_REFERENCES_YES)
                string Dxw;
                if (Config.File_CheckSideCarFiles_Get() && !Config.File_IsReferenced_Get())
                {
                    FileName Test(Config.File_Names[0]);
                    Ztring FileExtension=Test.Extension_Get();
                    FileExtension.MakeLowerCase();

                    if (FileExtension!=__T("dfxp"))
                    {
                        Test.Extension_Set(__T("dfxp"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".dfxp\" />\r\n";
                    }
                    if (FileExtension!=__T("sami"))
                    {
                        Test.Extension_Set(__T("sami"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".sami\" />\r\n";
                    }
                    if (FileExtension!=__T("sc2"))
                    {
                        Test.Extension_Set(__T("sc2"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".sc2\" />\r\n";
                    }
                    if (FileExtension!=__T("scc"))
                    {
                        Test.Extension_Set(__T("scc"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".scc\" />\r\n";
                    }
                    if (FileExtension!=__T("smi"))
                    {
                        Test.Extension_Set(__T("smi"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".smi\" />\r\n";
                    }
                    if (FileExtension!=__T("srt"))
                    {
                        Test.Extension_Set(__T("srt"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".srt\" />\r\n";
                    }
                    if (FileExtension!=__T("stl"))
                    {
                        Test.Extension_Set(__T("stl"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".stl\" />\r\n";
                    }
                    if (FileExtension!=__T("ttml"))
                    {
                        Test.Extension_Set(__T("ttml"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".ttml\" />\r\n";
                    }
                    if (FileExtension!=__T("ssa"))
                    {
                        Test.Extension_Set(__T("ssa"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".ssa\" />\r\n";
                    }
                    if (FileExtension!=__T("ass"))
                    {
                        Test.Extension_Set(__T("ass"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".ass\" />\r\n";
                    }
                    if (FileExtension!=__T("vtt"))
                    {
                        Test.Extension_Set(__T("vtt"));
                        if (File::Exists(Test))
                            Dxw+=" <clip file=\""+Test.Name_Get().To_UTF8()+".vtt\" />\r\n";
                    }

                    Ztring Name=Test.Name_Get();
                    Ztring BaseName=Name.SubString(Ztring(), __T("_"));
                    if (!BaseName.empty())
                    {
                        ZtringList List;
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_audio.mp4"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.dfxp"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.sami"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.sc2"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.scc"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.smi"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.srt"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.stl"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_sub.vtt"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.dfxp"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.sami"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.sc2"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.scc"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.smi"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.srt"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.stl"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_forcesub.vtt"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.dfxp"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.sami"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.sc2"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.scc"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.smi"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.srt"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.stl"), Dir::Include_Files);
                        List+=Dir::GetAllFileNames(Test.Path_Get()+PathSeparator+BaseName+__T("_*_cc.vtt"), Dir::Include_Files);
                        for (size_t Pos=0; Pos<List.size(); Pos++)
                            Dxw+=" <clip file=\""+List[Pos].To_UTF8()+"\" />\r\n";
                    }

                    if (!Dxw.empty())
                    {
                        Dxw.insert(0, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n"
                                "<indexFile xmlns=\"urn:digimetrics-xml-wrapper\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:digimetrics-xml-wrapper DMSCLIP.XSD\">\r\n"
                                " <clip source=\"main\" file=\""+FileName(Config.File_Names[0]).Name_Get().To_UTF8()+"."+FileName(Config.File_Names[0]).Extension_Get().To_UTF8()+"\" />\r\n");
                        Dxw.append("</indexFile>\r\n");
                        Config.File_FileNameFormat_Set(__T("Dxw"));
                    }
                }

                if (Dxw.empty())
                {
                    CS.Enter();
                    if (Reader)
                    {
                        CS.Leave();
                        return; //There is a problem
                    }
                    Reader=new Reader_File();
                    CS.Leave();

                    Reader->Format_Test(this, Config.File_Names[0]);
                }
                else
                {
                    Open_Buffer_Init(Dxw.size(), FileName(Config.File_Names[0]).Path_Get()+PathSeparator+FileName(Config.File_Names[0]).Name_Get());
                    Open_Buffer_Continue((const int8u*)Dxw.c_str(), Dxw.size());
                    #if MEDIAINFO_NEXTPACKET
                        if (Config.NextPacket_Get())
                            return;
                   #endif //MEDIAINFO_NEXTPACKET
                    Open_Buffer_Finalize();
                }
            #else //defined(MEDIAINFO_REFERENCES_YES)
                CS.Enter();
                if (Reader)
                {
                    CS.Leave();
                    return; //There is a problem
                }
                Reader=new Reader_File();
                CS.Leave();

                Reader->Format_Test(this, Config.File_Names[0]);
            #endif //defined(MEDIAINFO_REFERENCES_YES)

            #if MEDIAINFO_NEXTPACKET
                if (Config.NextPacket_Get())
                    return;
            #endif //MEDIAINFO_NEXTPACKET
        }
    #endif //MEDIAINFO_FILE_YES

    CS.Enter();
    Config.State_Set(1);
    CS.Leave();
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open (const int8u* Begin, size_t Begin_Size, const int8u* End, size_t End_Size, int64u File_Size)
{
    Open_Buffer_Init(File_Size);
    Open_Buffer_Continue(Begin, Begin_Size);
    if (End && Begin_Size+End_Size<=File_Size)
    {
        Open_Buffer_Init(File_Size, File_Size-End_Size);
        Open_Buffer_Continue(End, End_Size);
    }
    Open_Buffer_Finalize();

    return 1;
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open_Buffer_Init (int64u File_Size_, const String &File_Name)
{
    CriticalSectionLocker CSL(CS);

    if (Config.File_Names.size()<=1) //If analyzing multiple files, theses members are adapted in File_Reader.cpp
    {
        if (File_Size_!=(int64u)-1)
            Config.File_Size=Config.File_Current_Size=File_Size_;
    }

    if (Info==NULL)
    {
        if (!Config.File_ForceParser_Get().empty())
        {
            CS.Leave();
            SelectFromExtension(Config.File_ForceParser_Get());
            CS.Enter();
        }
        if (Info==NULL)
        {
            Info=new File__MultipleParsing;
            Info_IsMultipleParsing=true;
        }
    }
    #if MEDIAINFO_TRACE
        Info->Init(&Config, &Details, &Stream, &Stream_More);
    #else //MEDIAINFO_TRACE
        Info->Init(&Config, &Stream, &Stream_More);
    #endif //MEDIAINFO_TRACE
    if (!File_Name.empty())
        Info->File_Name=File_Name;
    Info->Open_Buffer_Init(File_Size_);

    #if MEDIAINFO_EVENTS
        {
            struct MediaInfo_Event_General_Start_0 Event;
            memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Start, 0);
            Event.EventSize=sizeof(struct MediaInfo_Event_General_Start_0);
            Event.StreamIDs_Size=0;
            Event.Stream_Size=File_Size_;
            Event.FileName=NULL;
            Event.FileName_Unicode=File_Name.c_str();
            Config.Event_Send(NULL, (const int8u*)&Event, sizeof(MediaInfo_Event_General_Start_0));
        }
    #endif //MEDIAINFO_EVENTS

    return 1;
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open_Buffer_Init (int64u File_Size_, int64u File_Offset_)
{
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Open_Buffer_Init, File_Size=");Debug+=Ztring::ToZtring(File_Size_);Debug+=__T(", File_Offset=");Debug+=Ztring::ToZtring(File_Offset_);)
    #ifdef MEDIAINFO_DEBUG_BUFFER
        if (Info && File_Offset_>Info->File_Offset)
        {
            size_t Temp_Size=(size_t)(File_Offset_-Info->File_Offset);
            int8u* Temp=new int8u[Temp_Size];
            std::memset(Temp, 0xCC, Temp_Size);
            MEDIAINFO_DEBUG_BUFFER_SAVE(Temp, Temp_Size);
            delete[] Temp;
        }
    #endif //MEDIAINFO_DEBUG_BUFFER

    if (Config.File_Names.size()<=1) //If analyzing multiple files, theses members are adapted in File_Reader.cpp
    {
        if (File_Size_!=(int64u)-1)
            Config.File_Size=Config.File_Current_Size=File_Size_;
    }

    if (Info==NULL || File_Size_!=(int64u)-1)
        Open_Buffer_Init(File_Size_);

    if (File_Offset_!=(int64u)-1 && Info)
    {
        CriticalSectionLocker CSL(CS);
        Info->Open_Buffer_Position_Set(File_Offset_);
    }

    #if MEDIAINFO_EVENTS
        if (Info && Info->Status[File__Analyze::IsAccepted])
        {
            struct MediaInfo_Event_General_Move_Done_0 Event;
            memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Move_Done, 0);
            Event.EventSize=sizeof(struct MediaInfo_Event_General_Move_Done_0);
            Event.StreamIDs_Size=0;
            Event.StreamOffset=File_Offset_;
            Config.Event_Send(NULL, (const int8u*)&Event, sizeof(MediaInfo_Event_General_Move_Done_0));
        }
        else
        {
            struct MediaInfo_Event_General_Start_0 Event;
            memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Start, 0);
            Event.EventSize=sizeof(struct MediaInfo_Event_General_Start_0);
            Event.StreamIDs_Size=0;
            Event.Stream_Size=File_Size_;
            Event.FileName=NULL;
            Event.FileName_Unicode=NULL;
            Config.Event_Send(NULL, (const int8u*)&Event, sizeof(MediaInfo_Event_General_Start_0));
        }
    #endif //MEDIAINFO_EVENTS

    EXECUTE_SIZE_T(1, Debug+=__T("Open_Buffer_Init, will return 1");)
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::Open_Buffer_Unsynch ()
{
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Open_Buffer_Unsynch");)

    if (Info==NULL)
        return;

    CriticalSectionLocker CSL(CS);
    Info->Open_Buffer_Unsynch();
}

//---------------------------------------------------------------------------
std::bitset<32> MediaInfo_Internal::Open_Buffer_Continue (const int8u* ToAdd, size_t ToAdd_Size)
{
    CriticalSectionLocker CSL(CS);
    MEDIAINFO_DEBUG_BUFFER_SAVE(ToAdd, ToAdd_Size);
    if (Info==NULL)
        return 0;

    Info->Open_Buffer_Continue(ToAdd, ToAdd_Size);

    if (Info_IsMultipleParsing && Info->Status[File__Analyze::IsAccepted])
    {
        //Found
        File__Analyze* Info_ToDelete=Info;
        Info=((File__MultipleParsing*)Info)->Parser_Get();
        delete Info_ToDelete; //Info_ToDelete=NULL;
        Info_IsMultipleParsing=false;
    }

    #if 0 //temp, for old users
    //The parser wanted seek but the buffer is not seekable
    if (Info->File_GoTo!=(int64u)-1 && Config.File_IsSeekable_Get()==0)
    {
        Info->Open_Buffer_Finalize(true);
        Info->File_GoTo=(int64u)-1;
        MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Open_Buffer_Continue, will return 0");)
        return 0;
    }

    return 1;
    #else
    //The parser wanted seek but the buffer is not seekable
    if (Info->File_GoTo!=(int64u)-1 && Config.File_IsSeekable_Get()==0)
    {
        Info->Fill();
        Info->File_GoTo=(int64u)-1;
    }

    return Info->Status;
    #endif
}

//---------------------------------------------------------------------------
int64u MediaInfo_Internal::Open_Buffer_Continue_GoTo_Get ()
{
    CriticalSectionLocker CSL(CS);
    if (Info==NULL)
        return (int64u)-1;

    if (Info->File_GoTo==(int64u)-1
     || (Info->File_GoTo>=Info->File_Offset && Info->File_GoTo<Info->File_Offset+0x10000)) //If jump is tiny, this is not worth the performance cost due to seek
        return (int64u)-1;
    else
        return Info->File_GoTo;
}

//---------------------------------------------------------------------------
bool MediaInfo_Internal::Open_Buffer_Position_Set(int64u File_Offset)
{
    CriticalSectionLocker CSL(CS);
    if (Info==NULL)
        return false;

    Info->Open_Buffer_Position_Set(File_Offset);

    return true;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t MediaInfo_Internal::Open_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    CriticalSectionLocker CSL(CS);
    if (Info==NULL)
        return false;

    return Info->Open_Buffer_Seek(Method, Value, ID);
}
#endif //MEDIAINFO_SEEK

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open_Buffer_Finalize ()
{
    CS.Enter();
    if (Info && Info->Status[File__Analyze::IsUpdated])
    {
        Info->Open_Buffer_Update();
        Info->Status[File__Analyze::IsUpdated]=false;
        for (size_t Pos=File__Analyze::User_16; Pos<File__Analyze::User_16+16; Pos++)
            Info->Status[Pos]=false;
    }
    CS.Leave();

    CriticalSectionLocker CSL(CS);
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Open_Buffer_Finalize");)
    if (Info==NULL)
        return 0;

    Info->Open_Buffer_Finalize();
    #if MEDIAINFO_DEMUX
        if (Config.Demux_EventWasSent)
            return 0;
    #endif //MEDIAINFO_DEMUX

    //Cleanup
    if (!Config.File_IsSub_Get() && !Config.File_KeepInfo_Get()) //We need info for the calling parser
    {
        delete Info; Info=NULL;
    }
    if (Config.File_Names_Pos>=Config.File_Names.size())
    {
        delete[] Config.File_Buffer; Config.File_Buffer=NULL; Config.File_Buffer_Size=0; Config.File_Buffer_Size_Max=0;
    }

    EXECUTE_SIZE_T(1, Debug+=__T("Open_Buffer_Finalize, will return 1"))
}

//---------------------------------------------------------------------------
std::bitset<32> MediaInfo_Internal::Open_NextPacket ()
{
    CriticalSectionLocker CSL(CS);

    bool Demux_EventWasSent=false;
    if (Info==NULL || !Info->Status[File__Analyze::IsFinished])
    {
        #if !defined(MEDIAINFO_READER_NO)
            if (Reader)
            {
                CS.Leave();
                Demux_EventWasSent=(Reader->Format_Test_PerParser_Continue(this)==2);
                CS.Enter();
            }
            else
        #endif //defined(MEDIAINFO_READER_NO)
            {
                #if MEDIAINFO_NEXTPACKET
                    Config.Demux_EventWasSent=false;
                #endif //MEDIAINFO_NEXTPACKET
                Open_Buffer_Continue(NULL, 0);
                #if MEDIAINFO_NEXTPACKET
                    Demux_EventWasSent=Config.Demux_EventWasSent;
                    if (!Demux_EventWasSent)
                #endif //MEDIAINFO_NEXTPACKET
                        Open_Buffer_Finalize();
            }
    }

    std::bitset<32> ToReturn=Info==NULL?std::bitset<32>(0x0F):Info->Status;
    if (Demux_EventWasSent)
        ToReturn[8]=true; //bit 8 is for the reception of a frame

    return ToReturn;
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::Close()
{
    if (IsRunning())
    {
        RequestTerminate();
        while(IsExited())
            Yield();
    }

    CriticalSectionLocker CSL(CS);
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Close");)
    Stream.clear();
    Stream.resize(Stream_Max);
    Stream_More.clear();
    Stream_More.resize(Stream_Max);
    delete Info; Info=NULL;
    #if !defined(MEDIAINFO_READER_NO)
        delete Reader; Reader=NULL;
    #endif //defined(MEDIAINFO_READER_NO)
}

//***************************************************************************
// Get File info
//***************************************************************************

/*//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Inform(size_t)
{
    //Info case
    if (Info)
        return Info->Inform();

    if (!Info)
        return MediaInfoLib::Config.EmptyString_Get();

    return Info->Inform();
} */

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Get(stream_t StreamKind, size_t StreamPos, size_t Parameter, info_t KindOfInfo)
{
    CriticalSectionLocker CSL(CS);
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Get, StreamKind=");Debug+=Ztring::ToZtring((size_t)StreamKind);Debug+=__T(", StreamPos=");Debug+=Ztring::ToZtring(StreamPos);Debug+=__T(", Parameter=");Debug+=Ztring::ToZtring(Parameter);)

    if (Info && Info->Status[File__Analyze::IsUpdated])
    {
        Info->Open_Buffer_Update();
        Info->Status[File__Analyze::IsUpdated]=false;
        for (size_t Pos=File__Analyze::User_16; Pos<File__Analyze::User_16+16; Pos++)
            Info->Status[Pos]=false;
    }

    //Check integrity
    if (StreamKind>=Stream_Max || StreamPos>=Stream[StreamKind].size() || Parameter>=MediaInfoLib::Config.Info_Get(StreamKind).size()+Stream_More[StreamKind][StreamPos].size() || KindOfInfo>=Info_Max)
        return MediaInfoLib::Config.EmptyString_Get(); //Parameter is unknown

    else if (Parameter<MediaInfoLib::Config.Info_Get(StreamKind).size())
    {
        //Optimization : KindOfInfo>Info_Text is in static lists
        if (KindOfInfo!=Info_Text)
            EXECUTE_STRING(MediaInfoLib::Config.Info_Get(StreamKind, Parameter, KindOfInfo), Debug+=__T("Get, will return ");Debug+=ToReturn;) //look for static information only
        else if (Parameter<Stream[StreamKind][StreamPos].size())
            EXECUTE_STRING(Stream[StreamKind][StreamPos][Parameter], Debug+=__T("Get, will return ");Debug+=ToReturn;)
        else
            EXECUTE_STRING(MediaInfoLib::Config.EmptyString_Get(), Debug+=__T("Get, will return ");Debug+=ToReturn;) //This parameter is known, but not filled
    }
    else
        EXECUTE_STRING(Stream_More[StreamKind][StreamPos][Parameter-MediaInfoLib::Config.Info_Get(StreamKind).size()](KindOfInfo), Debug+=__T("Get, will return ");Debug+=ToReturn;)
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Get(stream_t StreamKind, size_t StreamPos, const String &Parameter, info_t KindOfInfo, info_t KindOfSearch)
{
    //Legacy
    if (Parameter.find(__T("_String"))!=Error)
    {
        Ztring S1=Parameter;
        S1.FindAndReplace(__T("_String"), __T("/String"));
        return Get(StreamKind, StreamPos, S1, KindOfInfo, KindOfSearch);
    }
    if (Parameter==__T("Channels"))
        return Get(StreamKind, StreamPos, __T("Channel(s)"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("Artist"))
        return Get(StreamKind, StreamPos, __T("Performer"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("AspectRatio"))
        return Get(StreamKind, StreamPos, __T("DisplayAspectRatio"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("AspectRatio/String"))
        return Get(StreamKind, StreamPos, __T("DisplayAspectRatio/String"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("Chroma"))
        return Get(StreamKind, StreamPos, __T("Colorimetry"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("PlayTime"))
        return Get(StreamKind, StreamPos, __T("Duration"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("PlayTime/String"))
        return Get(StreamKind, StreamPos, __T("Duration/String"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("PlayTime/String1"))
        return Get(StreamKind, StreamPos, __T("Duration/String1"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("PlayTime/String2"))
        return Get(StreamKind, StreamPos, __T("Duration/String2"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("PlayTime/String3"))
        return Get(StreamKind, StreamPos, __T("Duration/String3"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate/String"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate/String"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate_Minimum"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate_Minimum"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate_Minimum/String"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate_Minimum/String"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate_Nominal"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate_Nominal"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate_Nominal/String"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate_Nominal/String"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate_Maximum"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate_Maximum"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==__T("BitRate_Maximum/String"))
        return Get(Stream_General, StreamPos, __T("OverallBitRate_Maximum/String"), KindOfInfo, KindOfSearch);
    if (Parameter==__T("AFD"))
        return Get(StreamKind, StreamPos, __T("ActiveFormatDescription"), KindOfInfo, KindOfSearch);

    CS.Enter();
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Get, StreamKind=");Debug+=Ztring::ToZtring((size_t)StreamKind);Debug+=__T(", StreamKind=");Debug+=Ztring::ToZtring(StreamPos);Debug+=__T(", Parameter=");Debug+=Ztring(Parameter);)

    if (Info && Info->Status[File__Analyze::IsUpdated])
    {
        Info->Open_Buffer_Update();
        Info->Status[File__Analyze::IsUpdated]=false;
        for (size_t Pos=File__Analyze::User_16; Pos<File__Analyze::User_16+16; Pos++)
            Info->Status[Pos]=false;
    }

    //Check integrity
    if (StreamKind>=Stream_Max || StreamPos>=Stream[StreamKind].size() || KindOfInfo>=Info_Max)
    {
        CS.Leave();
        EXECUTE_STRING(MediaInfoLib::Config.EmptyString_Get(), Debug+=__T("Get, stream and/or pos is invalid will return empty string");) //Parameter is unknown
    }

    //Special cases
    //-Inform for a stream
    if (Parameter==__T("Inform"))
    {
        CS.Leave();
        Ztring InformZtring=Inform(StreamKind, StreamPos, true);
        CS.Enter();
        size_t Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(__T("Inform"));
        if (Pos!=Error)
            Stream[StreamKind][StreamPos](Pos)=InformZtring;
    }

    //Case of specific info
    size_t ParameterI=MediaInfoLib::Config.Info_Get(StreamKind).Find(Parameter, KindOfSearch);
    if (ParameterI==Error)
    {
        ParameterI=Stream_More[StreamKind][StreamPos].Find(Parameter, KindOfSearch);
        if (ParameterI==Error)
        {
            #ifdef MEDIAINFO_DEBUG_WARNING_GET
                if (Ztring(Parameter)!=__T("SCTE35_PID")) //TODO: define a special interface for parser-specific parameters
                    std::cerr<<"MediaInfo: Warning, Get(), parameter \""<<Ztring(Parameter).To_Local()<<"\""<<std::endl;
            #endif //MEDIAINFO_DEBUG_WARNING_GET

            CS.Leave();
            EXECUTE_STRING(MediaInfoLib::Config.EmptyString_Get(), Debug+=__T("Get, parameter is unknown, will return empty string");) //Parameter is unknown
        }
        CS.Leave();
        CriticalSectionLocker CSL(CS);
        return Stream_More[StreamKind][StreamPos][ParameterI](KindOfInfo);
    }

    CS.Leave();

    EXECUTE_STRING(Get(StreamKind, StreamPos, ParameterI, KindOfInfo), Debug+=__T("Get, will return ");Debug+=ToReturn;)
}

//***************************************************************************
// Set File info
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Set(const String &ToSet, stream_t StreamKind, size_t StreamPos, size_t Parameter, const String &OldValue)
{
    CriticalSectionLocker CSL(CS);
    if (!Info)
        return 0;

    return Info->Set(StreamKind, StreamPos, Parameter, ToSet, OldValue);
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Set(const String &ToSet, stream_t StreamKind, size_t StreamPos, const String &Parameter, const String &OldValue)
{
    CriticalSectionLocker CSL(CS);
    if (!Info)
        return 0;

    return Info->Set(StreamKind, StreamPos, Parameter, ToSet, OldValue);
}

//***************************************************************************
// Output buffer
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Output_Buffer_Get (const String &Value)
{
    CriticalSectionLocker CSL(CS);
    if (!Info)
        return 0;

    MEDIAINFO_DEBUG_OUTPUT_VALUE(Value, Info->Output_Buffer_Get(Value));
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Output_Buffer_Get (size_t Pos)
{
    CriticalSectionLocker CSL(CS);
    if (!Info)
        return 0;

    MEDIAINFO_DEBUG_OUTPUT_POS(Pos, Info->Output_Buffer_Get(Pos));
}

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfo_Internal::Option (const String &Option, const String &Value)
{
    CriticalSectionLocker CSL(CS);
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=__T("Option, Option=");Debug+=Ztring(Option);Debug+=__T(", Value=");Debug+=Ztring(Value);)
    Ztring OptionLower=Option; OptionLower.MakeLowerCase();
         if (Option.empty())
        return __T("");
    else if (OptionLower==__T("language_update"))
    {
        if (!Info || Info->Get(Stream_General, 0, __T("CompleteName"))==__T(""))
            return __T("");

        ZtringListList Language=Value.c_str();
        MediaInfoLib::Config.Language_Set(Language);

        return __T("");
    }
    else if (OptionLower==__T("create_dummy"))
    {
        CreateDummy (Value);
        delete Info; Info=NULL;
        return __T("");
    }
    else if (OptionLower==__T("thread"))
    {
        BlockMethod=1;
        return __T("");
    }
    else if (Option==__T("info_capacities"))
    {
        return __T("Option removed");
    }
    #if MEDIAINFO_TRACE
    else if (OptionLower.find(__T("file_details_clear"))==0)
    {
        if (Info)
            Info->Details_Clear();

        return __T("");
    }
    #endif //MEDIAINFO_TRACE
    else if (OptionLower.find(__T("file_seek"))==0)
    {
        #if MEDIAINFO_SEEK
            if (Reader==NULL && Info==NULL)
                return __T("Error: Reader pointer is empty");

            size_t Method=(size_t)-1;
            int64u SeekValue=(int64u)-1;
            int64u ID=(int64u)-1;

            ZtringList List; List.Separator_Set(0, __T(","));
            List.Write(Value);
            for (size_t Pos=0; Pos<List.size(); Pos++)
            {
                if (!List[Pos].empty() && List[Pos].find(__T('%'))==List[Pos].size()-1)
                {
                    Method=1;
                    SeekValue=(int64u)(Ztring(List[Pos]).To_float32()*100);
                }
                else if (!List[Pos].empty() && List[Pos].find_first_not_of(__T("0123456789"))==string::npos)
                {
                    Method=0;
                    SeekValue=Ztring(List[Pos]).To_int64u();
                }
                else if (!List[Pos].empty() && List[Pos].find(__T("Frame="))!=string::npos)
                {
                    Method=3;
                    Ztring FrameNumberZ=List[Pos].substr(List[Pos].find(__T("Frame="))+6, string::npos);
                    SeekValue=FrameNumberZ.To_int64u();
                }
                else if (!List[Pos].empty() && List[Pos].find(__T(":"))!=string::npos)
                {
                    Method=2;
                    Ztring ValueZ=List[Pos];
                    SeekValue=0;
                    size_t Value_Pos=ValueZ.find(__T(":"));
                    if (Value_Pos==string::npos)
                        Value_Pos=ValueZ.size();
                    SeekValue+=Ztring(ValueZ.substr(0, Value_Pos)).To_int64u()*60*60*1000*1000*1000;
                    ValueZ.erase(0, Value_Pos+1);
                    Value_Pos=ValueZ.find(__T(":"));
                    if (Value_Pos==string::npos)
                        Value_Pos=ValueZ.size();
                    SeekValue+=Ztring(ValueZ.substr(0, Value_Pos)).To_int64u()*60*1000*1000*1000;
                    ValueZ.erase(0, Value_Pos+1);
                    Value_Pos=ValueZ.find(__T("."));
                    if (Value_Pos==string::npos)
                        Value_Pos=ValueZ.size();
                    SeekValue+=Ztring(ValueZ.substr(0, Value_Pos)).To_int64u()*1000*1000*1000;
                    ValueZ.erase(0, Value_Pos+1);
                    if (!ValueZ.empty())
                        SeekValue+=Ztring(ValueZ).To_int64u()*1000*1000*1000/(int64u)pow(10.0, (int)ValueZ.size());
                }
                else if (!List[Pos].empty() && List[Pos].find(__T("ID="))!=string::npos)
                {
                    Ztring IDZ=List[Pos].substr(List[Pos].find(__T("ID="))+3, string::npos);
                    ID=IDZ.To_int64u();
                }
            }

            CS.Leave();
            size_t Result;
            if (Reader)
                Result=Reader->Format_Test_PerParser_Seek(this, Method, SeekValue, ID);
            else
                Result=Open_Buffer_Seek(Method, SeekValue, ID);
            CS.Enter();
            switch (Result)
            {
                case 1  : return __T("");
                case 2  : return __T("Invalid value");
                #if MEDIAINFO_IBI
                case 3  : return __T("Feature not supported / IBI file not provided");
                case 4  : return __T("Problem during IBI file parsing");
                #endif //MEDIAINFO_IBI
                case 5  : return __T("Invalid ID");
                case 6  : return __T("Internal error");
                #if !MEDIAINFO_IBI
                case (size_t)-2 : return __T("Feature not supported / IBI support disabled due to compilation options");
                #endif //MEDIAINFO_IBI
                case (size_t)-1 : return __T("Feature not supported");
                default : return __T("Unknown error");
            }
        #else //MEDIAINFO_SEEK
            return __T("Seek manager is disabled due to compilation options");
        #endif //MEDIAINFO_SEEK
    }
    else if (OptionLower.find(__T("file_"))==0)
    {
        Ztring ToReturn2=Config.Option(Option, Value);
        if (Info)
            Info->Option_Manage();

        MEDIAINFO_DEBUG_OUTPUT_INIT(ToReturn2, Debug+=__T("Option, will return ");Debug+=ToReturn;)
    }
    else
        EXECUTE_STRING(MediaInfoLib::Config.Option(Option, Value), Debug+=__T("Option, will return ");Debug+=ToReturn;)
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Count_Get (stream_t StreamKind, size_t StreamPos)
{
    CriticalSectionLocker CSL(CS);

    if (Info && Info->Status[File__Analyze::IsUpdated])
    {
        Info->Open_Buffer_Update();
        Info->Status[File__Analyze::IsUpdated]=false;
        for (size_t Pos=File__Analyze::User_16; Pos<File__Analyze::User_16+16; Pos++)
            Info->Status[Pos]=false;
    }

    //Integrity
    if (StreamKind>=Stream_Max)
        return 0;

    //Count of streams
    if (StreamPos==Error)
        return Stream[StreamKind].size();

    //Integrity
    if (StreamPos>=Stream[StreamKind].size())
        return 0;

    //Count of piece of information in a stream
    return MediaInfoLib::Config.Info_Get(StreamKind).size()+Stream_More[StreamKind][StreamPos].size();
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::State_Get ()
{
    CriticalSectionLocker CSL(CS);
    return (size_t)(Config.State_Get()*10000);
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::TestContinuousFileNames ()
{
    CriticalSectionLocker CSL(CS);
    if (Info)
        Info->TestContinuousFileNames();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void MediaInfo_Internal::Event_Prepare (struct MediaInfo_Event_Generic* Event)
{
    CriticalSectionLocker CSL(CS);
    if (Info)
        Info->Event_Prepare(Event);
}
#endif // MEDIAINFO_EVENTS

} //NameSpace
