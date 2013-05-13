/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about VP8 files
// http://datatracker.ietf.org/doc/rfc6386/?include_text=1
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Vp8H
#define MediaInfo_Vp8H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Vp8
//***************************************************************************

class File_Vp8 : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;

    //Constructor/Destructor
    File_Vp8();
    ~File_Vp8();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Update();
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif
