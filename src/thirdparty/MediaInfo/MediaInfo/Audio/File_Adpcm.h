/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about ADPCM files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_AdpcmH
#define MediaInfo_File_AdpcmH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Adpcm
//***************************************************************************

class File_Adpcm : public File__Analyze
{
public :
    //In
    ZenLib::Ztring Codec;

protected :
    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
