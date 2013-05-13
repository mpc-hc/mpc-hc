/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
// Information about IVF files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_IvfH
#define MediaInfo_IvfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ivf
//***************************************************************************

class File_Ivf : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
private :
    //Buffer
    bool FileHeader_Begin();
    void FileHeader_Parse();
};

} //NameSpace

#endif

