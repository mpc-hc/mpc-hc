// File__Base - Base for other files
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
// Give common methods for all file types
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo__BaseH
#define MediaInfo__BaseH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/ZtringListList.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class MediaInfo_Config_MediaInfo;

//***************************************************************************
// Class File__Base
//***************************************************************************

class File__Base
{
public :
    //Constructor/Destructor
    File__Base();
    virtual ~File__Base();
    #if MEDIAINFO_TRACE
        void Init(MediaInfo_Config_MediaInfo * Config, Ztring* Details, std::vector<std::vector<ZtringList> > * Stream_=NULL, std::vector<std::vector<ZtringListList> > * Stream_More=NULL);
    #else //MEDIAINFO_TRACE
        void Init(MediaInfo_Config_MediaInfo * Config, std::vector<std::vector<ZtringList> > * Stream_=NULL, std::vector<std::vector<ZtringListList> > * Stream_More=NULL);
    #endif //MEDIAINFO_TRACE

    //Save
    int     Save ();

    //Get information
    const Ztring &Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t KindOfInfo=Info_Text);
    const Ztring &Get (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, info_t KindOfInfo=Info_Text, info_t KindOfSearch=Info_Name);

    //Set information
    int           Set (stream_t StreamKind, size_t StreamNumber, size_t Parameter, const Ztring &ToSet, const Ztring &OldValue=_T(""));
    int           Set (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, const Ztring &ToSet, const Ztring &OldValue=_T(""));

    //Output buffer
    virtual size_t Output_Buffer_Get (const String &) {return 0;};
    virtual size_t Output_Buffer_Get (size_t) {return 0;};

    //Options
    size_t Count_Get (stream_t StreamKind, size_t Pos=Error) const;
    void   Language_Set (); //Update language for an existing File__Base

protected :
    //Read
    virtual void Read_Buffer_Init     () {} //To overload
    virtual void Read_Buffer_Unsynched() {} //To overload
    virtual void Read_Buffer_Continue () {} //To overload
    virtual void Read_Buffer_Finalize () {} //To overload

    //Write
    virtual int Write       (stream_t /*StreamKind*/, size_t /*StreamNumber*/, const Ztring &/*Parameter*/, const Ztring &/*ToSet*/, const Ztring &/*OldValue*/) {return -1;} //Write the value in memory
    virtual int WriteToDisk () {return -1;} //Write modified tags on disk

    //Arrays
//private : //TODO:put it as private (DvDif)
    std::vector<std::vector<ZtringList> > * Stream; //pointer to others listed streams
    std::vector<std::vector<ZtringListList> > * Stream_More; //pointer to others listed streams
    bool Stream_MustBeDeleted;
protected :
    stream_t StreamKind_Last;
    size_t   StreamPos_Last;

    //Config
    MediaInfo_Config_MediaInfo* Config;

    //Details
    #if MEDIAINFO_TRACE
        Ztring* Details;
    #endif //MEDIAINFO_TRACE

public :
    #if MEDIAINFO_TRACE
        void   Details_Add(const char* Parameter);
    #endif //MEDIAINFO_TRACE
    virtual void Option_Manage() {};

    //File
    Ztring File_Name;
    int64u File_Size;
    int64u File_Offset;
    int64u File_Offset_FirstSynched;
    int64u File_GoTo; //How many byte to skip?

    //Divers
    void Clear();

public :  //A virer
    friend class File__Analyze;
    friend class File__MultipleParsing;
};

} //NameSpace

#endif
