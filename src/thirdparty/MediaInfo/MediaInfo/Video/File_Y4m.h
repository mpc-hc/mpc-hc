/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about YUV4MPEG2 files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Y4mH
#define MediaInfo_Y4mH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Y4m
//***************************************************************************

class File_Y4m : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Y4m();
    ~File_Y4m();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Fill();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Temp
    size_t HeaderEnd;
};

} //NameSpace

#endif
