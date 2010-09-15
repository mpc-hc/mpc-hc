// MediaInfo_Internal - All information about media files
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
//
// Give information about a lot of media files
// Dispatch the file to be tested by all containers
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_InternalH
#define MediaInfo_InternalH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// For developper: you can disable or enable traces
//#define MEDIAINFO_DEBUG_CONFIG
//#define MEDIAINFO_DEBUG_BUFFER
//#define MEDIAINFO_DEBUG_OUTPUT
// For developper: customization of traces
#ifdef MEDIAINFO_DEBUG_BUFFER
    const size_t MEDIAINFO_DEBUG_BUFFER_SAVE_FileSize=128*1024*1024;
#endif //MEDIAINFO_DEBUG_BUFFER
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/Thread.h"
#include "ZenLib/CriticalSection.h"
#include <bitset>
#if defined (MEDIAINFO_DEBUG_CONFIG) || defined (MEDIAINFO_DEBUG_BUFFER) || defined (MEDIAINFO_DEBUG_OUTPUT)
    #include <ZenLib/File.h>
    #include <map>
#endif //MEDIAINFO_DEBUG
using namespace std;
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__Analyze;
class Internet__Base;
class Reader__Base;

//***************************************************************************
/// @brief MediaInfo_Internal
/// @version 0.7
//***************************************************************************

class MediaInfo_Internal : public ZenLib::Thread
{
public :
    //Constructor/Destructor
    MediaInfo_Internal ();
    ~MediaInfo_Internal ();

    //File
    size_t Open (const String &File_Name);
    size_t Open (const ZenLib::int8u* Begin, size_t Begin_Size, const ZenLib::int8u* End=NULL, size_t End_Size=0, ZenLib::int64u File_Size=0);
    size_t Open_Buffer_Init (ZenLib::int64u File_Size=(ZenLib::int64u)-1, const String &File_Name=String());
    size_t Open_Buffer_Init (ZenLib::int64u File_Size, ZenLib::int64u File_Offset);
    std::bitset<32> Open_Buffer_Continue (const ZenLib::int8u* Buffer, size_t Buffer_Size);
    ZenLib::int64u Open_Buffer_Continue_GoTo_Get ();
    bool   Open_Buffer_Position_Set(int64u File_Offset);
    size_t Open_Buffer_Finalize ();
    std::bitset<32> Open_NextPacket ();
    void Close ();

    //General information
    Ztring  Inform ();
    Ztring  Inform (stream_t StreamKind, size_t StreamNumber=0); //All about only a specific stream

    //Get
    Ztring Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind=Info_Text);
    Ztring Get (stream_t StreamKind, size_t StreamNumber, const String &Parameter, info_t InfoKind=Info_Text, info_t SearchKind=Info_Name);

    //Set
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue=_T(""));
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue=_T(""));

    //Output_Buffered
    size_t Output_Buffer_Get (const String &Value);
    size_t Output_Buffer_Get (size_t Pos);

    //Info
    String Option (const String &Option, const String &Value=_T(""));
    size_t State_Get ();
    size_t Count_Get (stream_t StreamKind, size_t StreamNumber=(size_t)-1);

    //Internal
    static bool LibraryIsModified(); //Is the library has been modified? (#defines...)

private :
    friend class File_Bdmv;  //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Cdxa;  //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Mpeg4; //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Mxf;   //Theses classes need access to internal structure for optimization. There is recursivity with theses formats

    //Parsing handles
    File__Analyze*  Info;
    Internet__Base* Internet;
    #if !defined(MEDIAINFO_READER_NO)
        Reader__Base*   Reader;
    #endif //defined(MEDIAINFO_READER_NO)
    Ztring          File_Name;

    //Helpers
    void CreateDummy (const String& Value); //Create dummy Information
    MediaInfo_Internal(const MediaInfo_Internal&); // Copy Constructor

    //Open Buffer
    bool Info_IsMultipleParsing;

    //Config
    std::vector<std::vector<ZtringList> > Stream;
    std::vector<std::vector<ZtringListList> > Stream_More;
    Ztring Details;
    void Traiter(Ztring &C); //enleve les $if...

public :
    bool SelectFromExtension (const String &Parser); //Select File_* from the parser name
    #if !defined(MEDIAINFO_READER_NO)
        int  ListFormats(const String &File_Name=String());
    #else //!defined(MEDIAINFO_READER_NO)
        int  ListFormats(const String &File_Name=String()) {return 0;}
    #endif //!defined(MEDIAINFO_READER_NO)
    MediaInfo_Config_MediaInfo Config;

private :
    //Threading
    size_t  BlockMethod; //Open() return: 0=immedialtly, 1=after local info, 2=when user interaction is needed
    bool    IsInThread;
    void    Entry();
    ZenLib::CriticalSection CS;

    #ifdef MEDIAINFO_DEBUG_CONFIG
        File Debug_Config;
    #endif //MEDIAINFO_DEBUG_CONFIG
    #ifdef MEDIAINFO_DEBUG_BUFFER
        File    Debug_Buffer_Stream;
        int64u  Debug_Buffer_Stream_Order;
        File    Debug_Buffer_Sizes;
        int64u  Debug_Buffer_Sizes_Count;
    #endif //MEDIAINFO_DEBUG_BUFFER
    #ifdef MEDIAINFO_DEBUG_OUTPUT
        map<void*, File> Debug_Output_Value_Stream; //Key is the memory address
        map<void*, File> Debug_Output_Value_Sizes; //Key is the memory address
        vector<File*> Debug_Output_Pos_Stream; //Key is the pos
        vector<File*> Debug_Output_Pos_Sizes; //Key is the pos
        vector<void*> Debug_Output_Pos_Pointer; //Key is the pos
    #endif //MEDIAINFO_DEBUG_OUTPUT
};

} //NameSpace
#endif
