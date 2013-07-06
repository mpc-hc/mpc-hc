/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Dolby E files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DolbyEH
#define MediaInfo_File_DolbyEH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_DolbyE
//***************************************************************************

class File_DolbyE : public File__Analyze
{
public :
    //In
    int64u GuardBand_Before;
    int64u GuardBand_After;

    //Constructor/Destructor
    File_DolbyE();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Block();

    //Helpers
    bool Descramble_16bit();
    bool Descramble_20bit();
    bool Descramble_24bit();

    //Temp
    int64u  SMPTE_time_code_StartTimecode;
    int8u   ProgramConfiguration;
    int8u   FrameRate;
    int8u   BitDepth;
    bool    ScrambledBitStream;
    int8u*  Descrambled_Buffer; //Used in case of scrambled bitstream
    int64u  GuardBand_Before_Initial;
    int64u  GuardBand_After_Initial;
};

} //NameSpace

#endif
