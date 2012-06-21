// File__Tags - Info for all kind of framed tags tagged files
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about all kind of framed tags tagged files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File__TagsH
#define MediaInfo_File__TagsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__Tags_Helper
//***************************************************************************

class File__Tags_Helper
{
public :
    //In
    File__Analyze* Base;

    //Out
    int64u TagsSize;
    int64u File_BeginTagSize;
    int64u File_EndTagSize;

    //Constructor/Destructor
    File__Tags_Helper();
    ~File__Tags_Helper();

    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin() {return Synched_Test();}

    //Buffer - Synchro
    bool Synchronize(bool &Tag_Found, size_t Synchro_Offset=0);
    bool Synched_Test();

    //Buffer - Global
    bool Read_Buffer_Continue ();

    //Per element
    bool Header_Begin() {return Synched_Test();}

    //Streams
    size_t Stream_Prepare(stream_t StreamKind);

    //End
    void GoTo           (int64u GoTo, const char* ParserName=NULL);
    void GoToFromEnd    (int64u GoToFromEnd=0, const char* ParserName=NULL);
    void Accept         (const char* ParserName=NULL);
    void Reject         (const char* ParserName=NULL);
    void Finish         (const char* ParserName=NULL);

private :
    //Temp
    File__Analyze* Parser;
    File__Analyze* Parser_Streams_Fill; //Parser to merge when filling
    size_t         Parser_Buffer_Size;
    int64u Id3v1_Offset;
    int64u Lyrics3_Offset;
    int64u Lyrics3v2_Offset;
    int64u ApeTag_Offset;
    int64u JumpTo_WantedByParser;
    int64u Id3v1_Size;
    int64u Lyrics3_Size;
    int64u Lyrics3v2_Size;
    int64u ApeTag_Size;
    bool TagSizeIsFinal;
    bool SearchingForEndTags;

    //Helpers
    bool DetectBeginOfEndTags();        //return true if we can continue, false if want return
    bool DetectBeginOfEndTags_Test();

};} //NameSpace

#endif
