/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about TTA files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TtaH
#define MediaInfo_File_TtaH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Tta
//***************************************************************************

class File_Tta : public File__Analyze, public File__Tags_Helper
{
public :
    //Constructor/Destructor
    File_Tta();

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
