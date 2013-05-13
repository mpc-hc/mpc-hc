/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Empty container
// This is only to have a void parser
// It fill basic fields (filename...) only
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_UnknownH
#define MediaInfo_File_UnknownH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Unknown
//***************************************************************************

class File_Unknown : public File__Analyze
{
protected :
    //Buffer - Global
    void Read_Buffer_Init ();
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
