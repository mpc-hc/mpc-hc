/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PSD files
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_PsdH
#define MediaInfo_File_PsdH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Psd
//***************************************************************************

class File_Psd : public File__Analyze
{
protected :
    //Buffer - File header
    bool FileHeader_Begin();

    //Elements
    void Read_Buffer_Continue();
};

} //NameSpace

#endif

