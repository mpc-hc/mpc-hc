// MediaInfoList_Internal - A list of MediaInfo
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
// MediaInfoList_Internal
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Give information about a lot of media files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfoList_InternalH
#define MediaInfoList_InternalH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/Thread.h"
#include "ZenLib/CriticalSection.h"
#include <vector>
#include <queue>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class MediaInfoList_Internal : public ZenLib::Thread
{
public :
    //Class
    MediaInfoList_Internal (size_t Count_Init=64);
    virtual ~MediaInfoList_Internal ();

    //Files
    size_t Open (const String &File, const fileoptions_t Options=FileOption_Nothing);
    size_t Open_Buffer_Init (ZenLib::int64u File_Size=(ZenLib::int64u)-1, ZenLib::int64u File_Offset=0);
    size_t Open_Buffer_Continue (size_t FilePos, const ZenLib::int8u* Buffer, size_t Buffer_Size);
    ZenLib::int64u Open_Buffer_Continue_GoTo_Get (size_t FilePos);
    size_t Open_Buffer_Finalize (size_t FilePos);
    size_t Save (size_t FilePos);
    void Close (size_t FilePos=(size_t)-1);
    String Inform (size_t FilePos=(size_t)-1, size_t Reserved=0);

    //Get
    String Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t KindOfInfo=Info_Text); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name, text, measure, options, name (language), measure (language), info, how to
    String Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber, const String &Parameter, info_t KindOfInfo=Info_Text, info_t KindOfSearch=Info_Name); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name text measure options name(language) measure(language) information how to, KindOfSearch=which Kind Of information Parameter must be searched?

    //Set
    size_t Set (const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, size_t Parameter, const String &OldValue=_T("")); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name, text, measure, options name(language) measure(language) information how to
    size_t Set (const String &ToSet, size_t FilePos, stream_t StreamKind, size_t StreamNumber, const String &Parameter, const String &OldValue=_T("")); //Get info, FilePos=File position, StreamKind=General video audio text chapter, StreamNumber=stream number, PosInStream=parameter you want, KindOfInfo=name text measure options name (language) measure (language) information how to, KindOfSearch=which Kind Of information Parameter must be searched?

    //Output_Buffered
    char* Output_Buffer_Get (size_t File_Pos, size_t &Output_Buffer_Size);

    //Info
    String        Option (const String &Option, const String &Value=String(_T("")));
    static String Option_Static (const String &Option, const String &Value=String(_T("")));
    size_t                  State_Get ();
    size_t                  Count_Get (size_t FilePos, stream_t StreamKind, size_t StreamNumber=(size_t)-1);
    size_t                  Count_Get ();

private :
    std::vector<MediaInfo*> Info;
    std::queue<String> ToParse;
    std::map<String, String> Config_MediaInfo_Items; //Config per file
    size_t  ToParse_AlreadyDone;
    size_t  ToParse_Total;
    size_t  CountValid;
    MediaInfo_Config_MediaInfo Config;

    //Threading
    size_t  BlockMethod; //Open() return: 0=immedialtly, 1=after local info, 2=when user interaction is needed
    size_t  State;
    bool    IsInThread;
    void    Entry();
    ZenLib::CriticalSection CS;
};

} //NameSpace
#endif
