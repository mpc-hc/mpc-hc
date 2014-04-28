/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about OpenMG (OMA) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_OpenMGH
#define MediaInfo_File_OpenMGH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_OpenMG
//***************************************************************************

class File_OpenMG : public File__Analyze, public File__Tags_Helper
{
public :
    //Constructor/Destructor
    File_OpenMG();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish()                                                       {File__Tags_Helper::Streams_Finish();}

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif
