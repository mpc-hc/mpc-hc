/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Read_Buffer_Unsynched();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    #endif //MEDIAINFO_DEMUX

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

    //Buffer
    bool FrameSynchPoint_Test();
    const int8u* Save_Buffer;
    size_t Save_Buffer_Offset;
    size_t Save_Buffer_Size;

    //Temp
    std::vector<ZenLib::int32u> Asset_Sizes;
    Ztring Profile;
    int32u Original_Size;
    int32u HD_size;
    int16u Primary_Frame_Byte_Size;
    int16u Number_Of_PCM_Sample_Blocks;
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
