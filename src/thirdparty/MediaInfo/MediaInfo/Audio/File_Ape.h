/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Ape files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_ApeH
#define MediaInfo_File_ApeH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ape
//***************************************************************************

class File_Ape : public File__Analyze, public File__Tags_Helper
{
public :
    //Constructor/Destructor
    File_Ape();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Global
    void Read_Buffer_Continue()                                                 {File__Tags_Helper::Read_Buffer_Continue();}

    //Temp
    int64u Duration;
    int64u UncompressedSize;
};

} //NameSpace

#endif

