/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PCM (from Blu-ray) streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Pcm_M2tsH
#define MediaInfo_File_Pcm_M2tsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Pcm_M2ts
//***************************************************************************

class File_Pcm_M2ts : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Pcm_M2ts();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Temp
    int8u   channel_assignment;
    int8u   sampling_frequency;
    int8u   bits_per_sample;
};

} //NameSpace

#endif
