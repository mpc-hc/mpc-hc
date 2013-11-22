/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
// For developer: you can disable or enable traces
//#define MEDIAINFO_DEBUG_CONFIG
//#define MEDIAINFO_DEBUG_BUFFER
//#define MEDIAINFO_DEBUG_OUTPUT
//#define MEDIAINFO_DEBUG_WARNING_GET
// For developer: customization of traces
#ifdef MEDIAINFO_DEBUG_BUFFER
    const size_t MEDIAINFO_DEBUG_BUFFER_SAVE_FileSize=128*1024*1024;
#endif //MEDIAINFO_DEBUG_BUFFER
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Config.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/Thread.h"
#if defined (MEDIAINFO_DEBUG_CONFIG) || defined (MEDIAINFO_DEBUG_BUFFER) || defined (MEDIAINFO_DEBUG_OUTPUT)
    #include <ZenLib/File.h>
    #include <map>
#endif //MEDIAINFO_DEBUG
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
    #if MEDIAINFO_SEEK
    size_t Open_Buffer_Seek        (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    void    Open_Buffer_Unsynch     ();
    size_t Open_Buffer_Finalize ();
    std::bitset<32> Open_NextPacket ();
    void Close ();

    //General information
    Ztring  Inform ();
    Ztring  Inform (stream_t StreamKind, size_t StreamNumber, bool IsDirect); //All about only a specific stream

    //Get
    Ztring Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind=Info_Text);
    Ztring Get (stream_t StreamKind, size_t StreamNumber, const String &Parameter, info_t InfoKind=Info_Text, info_t SearchKind=Info_Name);

    //Set
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue=__T(""));
    size_t Set (const String &ToSet, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue=__T(""));

    //Output_Buffered
    size_t Output_Buffer_Get (const String &Value);
    size_t Output_Buffer_Get (size_t Pos);

    //Info
    String Option (const String &Option, const String &Value=__T(""));
    size_t State_Get ();
    size_t Count_Get (stream_t StreamKind, size_t StreamNumber=(size_t)-1);

    //Position in a MediaInfoList class
    bool    IsFirst;
    bool    IsLast;

    //Internal
    static bool LibraryIsModified(); //Is the library has been modified? (#defines...)

private :
    friend class File_Bdmv;  //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Cdxa;  //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Lxf;   //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Mpeg4; //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_MpegTs;//Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_MpegPs;//Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_Mxf;   //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_DcpAm; //Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_DcpCpl;//Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File_DcpPkl;//Theses classes need access to internal structure for optimization. There is recursivity with theses formats
    friend class File__ReferenceFilesHelper; //Theses classes need access to internal structure for optimization. There is recursivity with theses formats

    //Parsing handles
    File__Analyze*  Info;
    Internet__Base* Internet;
    #if !defined(MEDIAINFO_READER_NO)
        Reader__Base*   Reader;
    #endif //defined(MEDIAINFO_READER_NO)

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
    void TestContinuousFileNames();
    #if MEDIAINFO_EVENTS
        void Event_Prepare (struct MediaInfo_Event_Generic* Event);
    #endif // MEDIAINFO_EVENTS
    #if !defined(MEDIAINFO_READER_NO)
        int  ListFormats(const String &File_Name=String());
    #else //!defined(MEDIAINFO_READER_NO)
        int  ListFormats(const String &File_Name=String()) {return 0;}
    #endif //!defined(MEDIAINFO_READER_NO)
    MediaInfo_Config_MediaInfo Config;

    Ztring Xml_Name_Escape(const Ztring &Name);
    Ztring Xml_Content_Escape(const Ztring &Content, size_t &Modified);
    Ztring &Xml_Content_Escape_Modifying(Ztring &Content, size_t &Modified);

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
