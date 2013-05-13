/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SMPTE ST 331 streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SmpteSt0331H
#define MediaInfo_File_SmpteSt0331H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_SmpteSt0331
//***************************************************************************

class File_SmpteSt0331 : public File__Analyze
{
public :
    //In
    int32u  QuantizationBits;

    //Constructor/Destructor
    File_SmpteSt0331();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Temp
    int8u   Channels_valid;
};

} //NameSpace

#endif

