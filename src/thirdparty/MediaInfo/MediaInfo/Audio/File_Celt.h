/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about CELT files
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_CeltH
#define MediaInfo_File_CeltH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Celt
//***************************************************************************

class File_Celt : public File__Analyze
{
public :
    File_Celt();

private :
    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Identification();
    void Comment();

    //Temp
    bool Identification_Done;
};

} //NameSpace

#endif
