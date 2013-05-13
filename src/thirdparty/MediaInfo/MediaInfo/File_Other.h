/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Use magic number to detect only the format
// Theses formats are not planned to be expanded, this is only to detect
// well all files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_OtherH
#define MediaInfo_File_OtherH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Other
//***************************************************************************

class File_Other : public File__Analyze
{
protected :
    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
