/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DASH (.mpd) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DashMpdH
#define MediaInfo_File_DashMpdH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__ReferenceFilesHelper;

//***************************************************************************
// Class File_DashMpd
//***************************************************************************

class File_DashMpd : public File__Analyze
{
public :
    //Constructor/Destructor
    File_DashMpd();
    ~File_DashMpd();

private :
    //Streams management
    void Streams_Finish ();

    //Buffer - Global
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer - File header
    bool FileHeader_Begin();

    //Helpers
    void Representation();

    //Temp
    File__ReferenceFilesHelper*     ReferenceFiles;
};

} //NameSpace

#endif

