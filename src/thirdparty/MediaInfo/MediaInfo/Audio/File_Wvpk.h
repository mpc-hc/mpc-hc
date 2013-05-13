/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about WavePack files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_WvpkH
#define MediaInfo_File_WvpkH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Wvpk
//***************************************************************************

class File_Wvpk : public File__Analyze, public File__Tags_Helper
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FromMKV;
    bool   FromMKV_CodecPrivateParsed;

    //Constructor - Destructor
    File_Wvpk();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();
    void Data_Parse_Fill();

    //Elements
    void id_07();
    void id_0D();
    void id_25();

    //Temp - Technical info
    int32u total_samples_FirstFrame;
    int32u block_index_FirstFrame;
    int32u block_index_LastFrame;
    int32u block_samples_LastFrame;
    bool   resolution0;
    bool   resolution1;
    bool   mono;
    bool   hybrid;
    bool   joint_stereo;
    bool   cross_channel_decorrelation;
    int8u  SamplingRate;
    int8u  num_channels;
    int32u channel_mask;
    int32u Size;
    int16u version;
    Ztring Encoded_Library_Settings;
};

} //NameSpace

#endif
