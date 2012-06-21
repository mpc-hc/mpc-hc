// File_Dts - Info for DTS files
// Copyright (C) 2004-2011 MediaArea.net SARL, Info@MediaArea.net
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

//---------------------------------------------------------------------------
#ifndef MediaInfo_DtsH
#define MediaInfo_DtsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#ifdef ES
   #undef ES //Solaris defines this somewhere
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dts
//***************************************************************************

class File_Dts : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;

    //Constructor/Destructor
    File_Dts();
    ~File_Dts();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    #endif //MEDIAINFO_DEMUX

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Core();
    void Core_XCh(int64u Size);
    void Core_XXCh(int64u Size);
    void Core_X96k(int64u Size);
    void HD();
    void HD_XCh(int64u Size);
    void HD_XXCh(int64u Size);
    void HD_X96k(int64u Size);
    void HD_XLL(int64u Size);
    void HD_XBR(int64u Size);
    void HD_XSA(int64u Size);

    //Temp
    std::map<int32u, int64u> HD_Sizes;
    std::vector<ZenLib::int32u> Asset_Sizes;
    Ztring Profile;
    File__Analyze* Parser; //14 bits or Little Endian
    int32u HD_size;
    int16u Primary_Frame_Byte_Size_minus_1;
    int16u HD_SpeakerActivityMask;
    int8u  channel_arrangement;
    int8u  channel_arrangement_XCh;
    int8u  sample_frequency;
    int8u  sample_frequency_X96k;
    int8u  bit_rate;
    int8u  lfe_effects;
    int8u  bits_per_sample;
    int8u  ExtensionAudioDescriptor;
    int8u  HD_BitResolution;
    int8u  HD_MaximumSampleRate;
    int8u  HD_TotalNumberChannels;
    int8u  HD_ExSSFrameDurationCode;
    bool   ExtendedCoding;
    bool   Word;
    bool   BigEndian;
    bool   ES;
    bool   Core_Exists;

    //Helpers
    float64 BitRate_Get(bool WithHD=false);
};

} //NameSpace

#endif
