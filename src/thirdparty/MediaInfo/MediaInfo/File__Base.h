/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
    virtual const Ztring &Get (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, info_t KindOfInfo=Info_Text, info_t KindOfSearch=Info_Name);

    //Set information
    int           Set (stream_t StreamKind, size_t StreamNumber, size_t Parameter, const Ztring &ToSet, const Ztring &OldValue=__T(""));
    int           Set (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, const Ztring &ToSet, const Ztring &OldValue=__T(""));

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
public : //TODO:put it as private
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

private :
    //Constructor
    File__Base (const File__Base&);                         // Prevent copy-construction
    File__Base& operator=(const File__Base&);               // Prevent assignment

public :  //A virer
    friend class File__Analyze;
    friend class File__MultipleParsing;
    friend class File__ReferenceFilesHelper;
};

} //NameSpace

#endif
