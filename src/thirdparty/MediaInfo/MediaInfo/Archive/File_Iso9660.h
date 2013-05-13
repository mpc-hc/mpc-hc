/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Iso9660 files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Iso9660H
#define MediaInfo_File_Iso9660H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Iso9660
//***************************************************************************

class File_Iso9660 : public File__Analyze
{
protected :
    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Elements
    void Primary_Volume_Descriptor();
};

} //NameSpace

#endif
