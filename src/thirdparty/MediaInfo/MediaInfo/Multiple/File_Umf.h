/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about UMF files
// Unified Material Format
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_UmfH
#define MediaInfo_File_UmfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Umf
//***************************************************************************

class File_Umf : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Umf();

    //Out
    #if MEDIAINFO_SEEK || MEDIAINFO_DEMUX
        int64u GopSize;
    #endif //MEDIAINFO_SEEK || MEDIAINFO_DEMUX

protected :
    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
