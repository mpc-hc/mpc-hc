/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Lyrics3 tagged files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Lyrics3H
#define MediaInfo_File_Lyrics3H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Lyrics3
//***************************************************************************

class File_Lyrics3 : public File__Analyze
{
public :
    //In
    int64u TotalSize;

    //Constructor/Destructor
    File_Lyrics3();

private :
    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
