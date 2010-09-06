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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/File__MultipleParsing.h"
#include "ZenLib/Dir.h"
#include "ZenLib/File.h"
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
        const Char* MediaInfo_Debug_Name=_T("MediaInfo_Debug");
    #else
        const Char* MediaInfo_Debug_Name=_T("/tmp/MediaInfo_Debug");
    #endif
#endif

#ifdef MEDIAINFO_DEBUG_CONFIG
    #define MEDIAINFO_DEBUG_CONFIG_TEXT(_TOAPPEND) \
        { \
            Ztring Debug; \
            _TOAPPEND; \
            Debug+=_T("\r\n"); \
            if (!Debug_Config.Opened_Get()) \
                Debug_Config.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Config.txt")); \
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
                Debug_Buffer_Stream.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Buffer.Stream.0000000000000000")); \
                Debug_Buffer_Stream_Order=0; \
                Debug_Buffer_Sizes.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Buffer.Sizes.0000000000000000")); \
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
                    Before.insert(0, 1, _T('0')); \
                Ztring Next=Ztring::ToZtring(Debug_Buffer_Stream_Order+1); \
                while (Next.size()<16) \
                    Next.insert(0, 1, _T('0')); \
                Debug_Buffer_Stream.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Buffer.Stream.")+Next); \
                Debug_Buffer_Sizes.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Buffer.Sizes.")+Next); \
                File::Delete(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Buffer.Stream.")+Before); \
                File::Delete(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Buffer.Sizes.")+Before); \
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
            if (OptionLower==_T("file_duplicate")) \
            { \
                size_t Pos=(size_t)ToReturn2.To_int64u(); \
                if (Pos>=Debug_Output_Pos_Stream.size()) \
                { \
                    Debug_Output_Pos_Stream.resize(Pos+1); \
                    Debug_Output_Pos_Stream[Pos]=new File(); \
                    Debug_Output_Pos_Sizes.resize(Pos+1); \
                    Debug_Output_Pos_Sizes[Pos]=new File(); \
                    Debug_Output_Pos_Pointer.resize(Pos+1); \
                    Debug_Output_Pos_Pointer[Pos]=(void*)Ztring(Value).SubString(_T("memory:/""/"), _T(":")).To_int64u(); \
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
            void* ValueH=(void*)Ztring(Value).SubString(_T("memory:/""/"), _T(":")).To_int64u(); \
            map<void*, File>::iterator F_Stream=Debug_Output_Value_Stream.find(ValueH); \
            if (F_Stream!=Debug_Output_Value_Stream.end()) \
            { \
                map<void*, File>::iterator F_Sizes=Debug_Output_Value_Stream.find(ValueH); \
                if (!F_Stream->second.Opened_Get()) \
                { \
                    F_Stream->second.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Output.")+Ztring::ToZtring((size_t)ValueH, 16)+_T(".Stream")); \
                    F_Sizes->second.Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Output.")+Ztring::ToZtring((size_t)ValueH, 16)+_T(".Sizes")); \
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
                    Debug_Output_Pos_Stream[_POS]->Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Output.")+Ztring::ToZtring(Pos, 16)+_T(".Stream")); \
                    Debug_Output_Pos_Sizes[_POS]->Create(Ztring(MediaInfo_Debug_Name)+_T(".")+Ztring::ToZtring((size_t)this, 16)+_T(".Output.")+Ztring::ToZtring(Pos, 16)+_T(".Sizes")); \
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

    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Construction");)

    MediaInfoLib::Config.Init(); //Initialize Configuration

    BlockMethod=BlockMethod_Local;
    Info=NULL;
    #if !defined(MEDIAINFO_READER_NO)
        Reader=NULL;
    #endif //!defined(MEDIAINFO_READER_NO)
    Info_IsMultipleParsing=false;

    Stream.resize(Stream_Max);
    Stream_More.resize(Stream_Max);
    
    //Threading
    BlockMethod=0;
    IsInThread=false;
}

//---------------------------------------------------------------------------
MediaInfo_Internal::~MediaInfo_Internal()
{
    Close();

    CriticalSectionLocker CSL(CS);

    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Destruction");)

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
    
    CS.Enter();
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Open, File=");Debug+=Ztring(File_Name_).c_str();)
    File_Name=File_Name_;
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
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Entry");)
    Config.State_Set(0);
    CS.Leave();

        if (0);
    #if defined(MEDIAINFO_LIBCURL_YES)
        else if ((File_Name.size()>=7
          && File_Name[0]==_T('h')
          && File_Name[1]==_T('t')
          && File_Name[2]==_T('t')
          && File_Name[3]==_T('p')
          && File_Name[4]==_T(':')
          && File_Name[5]==_T('/')
          && File_Name[6]==_T('/'))
         || (File_Name.size()>=6
          && File_Name[0]==_T('f')
          && File_Name[1]==_T('t')
          && File_Name[2]==_T('p')
          && File_Name[3]==_T(':')
          && File_Name[4]==_T('/')
          && File_Name[5]==_T('/')))
            Reader_libcurl().Format_Test(this, File_Name);
    #endif //MEDIAINFO_LIBCURL_YES

    #if defined(MEDIAINFO_LIBMMS_YES)
        else if ((File_Name.size()>=6
          && File_Name[0]==_T('m')
          && File_Name[1]==_T('m')
          && File_Name[2]==_T('s')
          && File_Name[3]==_T(':')
          && File_Name[4]==_T('/')
          && File_Name[5]==_T('/'))
         || (File_Name.size()>=7
          && File_Name[0]==_T('m')
          && File_Name[1]==_T('m')
          && File_Name[2]==_T('s')
          && File_Name[3]==_T('h')
          && File_Name[4]==_T(':')
          && File_Name[5]==_T('/')
          && File_Name[6]==_T('/')))
            Reader_libmms().Format_Test(this, File_Name);
    #endif //MEDIAINFO_LIBMMS_YES

    #if defined(MEDIAINFO_DIRECTORY_YES)
        else if (Dir::Exists(File_Name))
            Reader_Directory().Format_Test(this, File_Name);
    #endif //MEDIAINFO_DIRECTORY_YES

    #if defined(MEDIAINFO_FILE_YES)
        else if (File::Exists(File_Name))
        {
            CS.Enter();
            if (Reader)
            {
                CS.Leave();
                return; //There is a problem
            }
            Reader=new Reader_File();
            CS.Leave();

            Reader->Format_Test(this, File_Name);

            if (Config.NextPacket_Get())
                return;
        }
    #endif //MEDIAINFO_FILE_YES

    CS.Enter();
    Config.State_Set(1);
    CS.Leave();
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open (const int8u* Begin, size_t Begin_Size, const int8u*, size_t, int64u File_Size)
{
    Open_Buffer_Init(File_Size);
    Open_Buffer_Continue(Begin, Begin_Size);
    Open_Buffer_Finalize();

    return 1;
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open_Buffer_Init (int64u File_Size_, const String &File_Name)
{
    CriticalSectionLocker CSL(CS);
    if (Info==NULL)
    {
        if (!Config.File_ForceParser_Get().empty())
        {
            CS.Leave();
            SelectFromExtension(Config.File_ForceParser_Get());
            CS.Enter();
        }
        else
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

    return 1;
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open_Buffer_Init (int64u File_Size_, int64u File_Offset_)
{
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Open_Buffer_Init, File_Size=");Debug+=Ztring::ToZtring(File_Size_);Debug+=_T(", File_Offset=");Debug+=Ztring::ToZtring(File_Offset_);)

    if (Info==NULL)
        Open_Buffer_Init(File_Size_);

    if (File_Offset_!=(int64u)-1 && Info)
    {
        CriticalSectionLocker CSL(CS);
        Info->Open_Buffer_Position_Set(File_Offset_);
        //Info->Open_Buffer_Unsynch();
    }

    #if MEDIAINFO_EVENTS
        if (Info->Status[File__Analyze::IsAccepted])
        {
            struct MediaInfo_Event_General_Move_Done_0 Event;
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Move_Done, 0);
            Event.Stream_Offset=File_Offset_;
            Config.Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_Move_Done_0));
        }
        else
        {
            struct MediaInfo_Event_General_Start_0 Event;
            Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_General_Start, 0);
            Event.Stream_Size=File_Size_;
            Config.Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_General_Start_0));
        }
    #endif //MEDIAINFO_EVENTS

    EXECUTE_SIZE_T(1, Debug+=_T("Open_Buffer_Init, will return 1");)
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
        MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Open_Buffer_Continue, will return 0");)
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

    if (!Info->Status[File__Analyze::IsFilled] && Info->Status[File__Analyze::IsUpdated])
        Info->Status[File__Analyze::IsUpdated]=false; //No updated info until IsFilled is set

    return Info->Status;
    #endif
}

//---------------------------------------------------------------------------
int64u MediaInfo_Internal::Open_Buffer_Continue_GoTo_Get ()
{
    CriticalSectionLocker CSL(CS);
    if (Info==NULL)
        return 0;

    return Info->File_GoTo;
}

bool MediaInfo_Internal::Open_Buffer_Position_Set(int64u File_Offset)
{
    CriticalSectionLocker CSL(CS);
    if (Info==NULL)
        return false;

    Info->Open_Buffer_Position_Set(File_Offset);

    return true;
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Open_Buffer_Finalize ()
{
    CriticalSectionLocker CSL(CS);
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Open_Buffer_Finalize");)
    if (Info==NULL)
        return 0;

    Info->Open_Buffer_Finalize();

    //Cleanup
    if (!Config.File_IsSub_Get() && !Config.File_KeepInfo_Get()) //We need info for the calling parser
    {
        delete Info; Info=NULL;
    }

    EXECUTE_SIZE_T(1, Debug+=_T("Open_Buffer_Finalize, will return 1"))
}

//---------------------------------------------------------------------------
std::bitset<32> MediaInfo_Internal::Open_NextPacket ()
{
    CriticalSectionLocker CSL(CS);

    bool Demux_EventWasSent=false;
    if (Info==NULL || !Info->Status[File__Analyze::IsFinished])
    {
        #if !defined(MEDIAINFO_READER_NO)
            Demux_EventWasSent=(((Reader_File*)Reader)->Format_Test_PerParser_Continue(this)==2);
        #endif //defined(MEDIAINFO_READER_NO)
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
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Close");)
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
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Get, StreamKind=");Debug+=Ztring::ToZtring((size_t)StreamKind);Debug+=_T(", StreamPos=");Debug+=Ztring::ToZtring(StreamPos);Debug+=_T(", Parameter=");Debug+=Ztring::ToZtring(Parameter);)

    if (Info)
    {
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
            EXECUTE_STRING(MediaInfoLib::Config.Info_Get(StreamKind, Parameter, KindOfInfo), Debug+=_T("Get, will return ");Debug+=ToReturn;) //look for static information only
        else if (Parameter<Stream[StreamKind][StreamPos].size())
            EXECUTE_STRING(Stream[StreamKind][StreamPos][Parameter], Debug+=_T("Get, will return ");Debug+=ToReturn;)
        else
            EXECUTE_STRING(MediaInfoLib::Config.EmptyString_Get(), Debug+=_T("Get, will return ");Debug+=ToReturn;) //This parameter is known, but not filled
    }
    else
        EXECUTE_STRING(Stream_More[StreamKind][StreamPos][Parameter-MediaInfoLib::Config.Info_Get(StreamKind).size()](KindOfInfo), Debug+=_T("Get, will return ");Debug+=ToReturn;)
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Internal::Get(stream_t StreamKind, size_t StreamPos, const String &Parameter, info_t KindOfInfo, info_t KindOfSearch)
{
    //Legacy
    if (Parameter.find(_T("_String"))!=Error)
    {
        Ztring S1=Parameter;
        S1.FindAndReplace(_T("_String"), _T("/String"));
        return Get(StreamKind, StreamPos, S1, KindOfInfo, KindOfSearch);
    }
    if (Parameter==_T("Channels"))
        return Get(StreamKind, StreamPos, _T("Channel(s)"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("Artist"))
        return Get(StreamKind, StreamPos, _T("Performer"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("AspectRatio"))
        return Get(StreamKind, StreamPos, _T("DisplayAspectRatio"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("AspectRatio/String"))
        return Get(StreamKind, StreamPos, _T("DisplayAspectRatio/String"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("Chroma"))
        return Get(StreamKind, StreamPos, _T("Colorimetry"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("PlayTime"))
        return Get(StreamKind, StreamPos, _T("Duration"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("PlayTime/String"))
        return Get(StreamKind, StreamPos, _T("Duration/String"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("PlayTime/String1"))
        return Get(StreamKind, StreamPos, _T("Duration/String1"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("PlayTime/String2"))
        return Get(StreamKind, StreamPos, _T("Duration/String2"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("PlayTime/String3"))
        return Get(StreamKind, StreamPos, _T("Duration/String3"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate/String"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate/String"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate_Minimum"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate_Minimum"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate_Minimum/String"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate_Minimum/String"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate_Nominal"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate_Nominal"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate_Nominal/String"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate_Nominal/String"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate_Maximum"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate_Maximum"), KindOfInfo, KindOfSearch);
    if (StreamKind==Stream_General && Parameter==_T("BitRate_Maximum/String"))
        return Get(Stream_General, StreamPos, _T("OverallBitRate_Maximum/String"), KindOfInfo, KindOfSearch);
    if (Parameter==_T("AFD"))
        return Get(StreamKind, StreamPos, _T("ActiveFormatDescription"), KindOfInfo, KindOfSearch);

    CS.Enter();
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Get, StreamKind=");Debug+=Ztring::ToZtring((size_t)StreamKind);Debug+=_T(", StreamKind=");Debug+=Ztring::ToZtring(StreamPos);Debug+=_T(", Parameter=");Debug+=Ztring(Parameter);)

    if (Info)
    {
        Info->Status[File__Analyze::IsUpdated]=false;
        for (size_t Pos=File__Analyze::User_16; Pos<File__Analyze::User_16+16; Pos++)
            Info->Status[Pos]=false;
    }

    //Check integrity
    if (StreamKind>=Stream_Max || StreamPos>=Stream[StreamKind].size() || KindOfInfo>=Info_Max)
    {
        CS.Leave();
        EXECUTE_STRING(MediaInfoLib::Config.EmptyString_Get(), Debug+=_T("Get, will return empty string");) //Parameter is unknown
    }

    //Special cases
    //-Inform for a stream
    if (Parameter==_T("Inform"))
    {
        CS.Leave();
        Ztring InformZtring=Inform(StreamKind, StreamPos);
        CS.Enter();
        size_t Pos=MediaInfoLib::Config.Info_Get(StreamKind).Find(_T("Inform"));
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
            CS.Leave();
            EXECUTE_STRING(MediaInfoLib::Config.EmptyString_Get(), Debug+=_T("Get, will return empty string");) //Parameter is unknown
        }
        CS.Leave();
        CriticalSectionLocker CSL(CS);
        return Stream_More[StreamKind][StreamPos][ParameterI](KindOfInfo);
    }

    CS.Leave();

    EXECUTE_STRING(Get(StreamKind, StreamPos, ParameterI, KindOfInfo), Debug+=_T("Get, will return ");Debug+=ToReturn;)
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
    MEDIAINFO_DEBUG_CONFIG_TEXT(Debug+=_T("Option, Option=");Debug+=Ztring(Option);Debug+=_T(", Value=");Debug+=Ztring(Value);)
    Ztring OptionLower=Option; OptionLower.MakeLowerCase();
         if (Option.empty())
        return _T("");
    else if (OptionLower==_T("language_update"))
    {
        if (!Info || Info->Get(Stream_General, 0, _T("CompleteName"))==_T(""))
            return _T("");

        ZtringListList Language=Value.c_str();
        MediaInfoLib::Config.Language_Set(Language);

        return _T("");
    }
    else if (OptionLower==_T("create_dummy"))
    {
        CreateDummy (Value);
        delete Info; Info=NULL;
        return _T("");
    }
    else if (OptionLower==_T("thread"))
    {
        BlockMethod=1;
        return _T("");
    }
    else if (Option==_T("info_capacities"))
    {
        return _T("Option removed");
    }
    #if MEDIAINFO_TRACE
    else if (OptionLower.find(_T("file_details_clear"))==0)
    {
        if (Info)
            Info->Details_Clear();

        return _T("");
    }
    #endif //MEDIAINFO_TRACE
    else if (OptionLower.find(_T("file_"))==0)
    {
        Ztring ToReturn2=Config.Option(Option, Value);
        if (Info)
            Info->Option_Manage();

        MEDIAINFO_DEBUG_OUTPUT_INIT(ToReturn2, Debug+=_T("Option, will return ");Debug+=ToReturn;)
    }
    else
        EXECUTE_STRING(MediaInfoLib::Config.Option(Option, Value), Debug+=_T("Option, will return ");Debug+=ToReturn;)
}

//---------------------------------------------------------------------------
size_t MediaInfo_Internal::Count_Get (stream_t StreamKind, size_t StreamPos)
{
    CriticalSectionLocker CSL(CS);
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

} //NameSpace


