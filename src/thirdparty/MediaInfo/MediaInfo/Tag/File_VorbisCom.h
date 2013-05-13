/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Vorbis comments
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_VorbisComH
#define MediaInfo_File_VorbisComH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "ZenLib/ZtringList.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_VorbisCom
//***************************************************************************

class File_VorbisCom : public File__Analyze
{
public :
    //In
    stream_t StreamKind_Specific; //Always in this stream kind whatever is the configuration
    stream_t StreamKind_Multiple; //Specific stream kind depend if there is multiple streams or not
    stream_t StreamKind_Common;   //Stream kind for common values

    //Constructor/Destructor
    File_VorbisCom();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    int32u user_comment_list_length;
    Ztring Chapter_Pos;
    Ztring Chapter_Time;
    ZtringList Performers;
    ZtringList Artists;
    ZtringList Accompaniments;
    ZtringList AlbumArtists;
};

} //NameSpace

#endif
