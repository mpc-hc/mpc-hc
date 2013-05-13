/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Musepack files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_AmrH
#define MediaInfo_File_AmrH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <map>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Amr
//***************************************************************************

class File_Amr : public File__Analyze
{
public :
    //In
    ZenLib::Ztring Codec;

public :
    File_Amr();

protected :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse ();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    size_t Frame_Number;
    std::map<int8u, size_t> FrameTypes;
    int64u Header_Size;
    int8u  FrameType;
    int8u  Channels;
    bool   IsWB;
};

} //NameSpace

#endif

