// File_Wvpk - Info for WavePack files
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#include "MediaInfo/File__Analyze.h"
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
