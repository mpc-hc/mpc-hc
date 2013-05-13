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
#ifndef MediaInfo_File_MpcH
#define MediaInfo_File_MpcH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpc
//***************************************************************************

class File_Mpc : public File__Analyze, public File__Tags_Helper
{
public :
    //Constructor/Destructor
    File_Mpc();

private :
    //Streams management
    void Streams_Finish()                                                       {File__Tags_Helper::Streams_Finish();}

    //Buffer - Global
    void Read_Buffer_Continue()                                                 {File__Tags_Helper::Read_Buffer_Continue();}

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();
};

} //NameSpace

#endif

