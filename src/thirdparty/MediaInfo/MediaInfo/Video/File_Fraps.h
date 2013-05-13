/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Fraps files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_FrapsH
#define MediaInfo_File_FrapsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Fraps
//***************************************************************************

class File_Fraps : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Fraps();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Elements
    void Version0();
    void Version1();
    void Version2();
};

} //NameSpace

#endif
