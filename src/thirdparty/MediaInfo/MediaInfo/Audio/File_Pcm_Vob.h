/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PCM (from DVD) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Pcm_VobH
#define MediaInfo_File_Pcm_VobH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Pcm_Vob
//***************************************************************************

class File_Pcm_Vob : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Pcm_Vob();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Temp
    int8u   BitDepth;
    int8u   Frequency;
    int8u   NumberOfChannelsMinusOne;
};

} //NameSpace

#endif
