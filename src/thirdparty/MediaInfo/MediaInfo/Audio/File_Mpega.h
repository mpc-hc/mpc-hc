/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_MpegaH
#define MediaInfo_File_MpegaH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File__Tags.h"
#include <map>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpega
//***************************************************************************

class File_Mpega : public File__Analyze, public File__Tags_Helper
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;
    bool   CalculateDelay;

    //Constructor/Destructor
    File_Mpega();

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
    bool Demux_UnpacketizeContainer_Test();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();
    void Data_Parse_Fill();
    void audio_data_Layer3();

    //Element
    bool Header_Xing();
    bool Header_VBRI();
    bool Header_Encoders();
    void Header_Encoders_Lame();
    void Encoded_Library_Guess();

    //Temp
    Ztring BitRate_Mode;
    Ztring BitRate_Nominal;
    Ztring BitRate_Minimum;
    Ztring Encoded_Library;
    Ztring Encoded_Library_Settings;
    std::map<int16u, size_t> BitRate_Count;
    std::map<int8u, size_t> sampling_frequency_Count;
    std::map<int8u, size_t> mode_Count;
    size_t Surround_Frames;
    size_t Block_Count[3]; //long, short, mixed
    size_t Channels_Count[4]; //Stereo, Join Stereo, Dual mono, mono
    size_t Extension_Count[4]; //No, IS, MS, IS+MS
    size_t Emphasis_Count[4]; //No, 50/15ms, Reserved, CCITT
    size_t Scfsi; //Total
    size_t Scalefac; //Total
    size_t Reservoir; //Total
    int64u LastSync_Offset;
    int64u VBR_FileSize;
    int32u VBR_Frames;
    int32u Reservoir_Max;
    int32u Xing_Scale;
    int32u BitRate; //Average
    int8u  ID;
    int8u  layer;
    int8u  bitrate_index;
    int8u  sampling_frequency;
    int8u  mode;
    int8u  mode_extension;
    int8u  emphasis;
    bool   protection_bit;
    bool   padding_bit;
    bool   copyright;
    bool   original_home;
    size_t MpegPsPattern_Count;

    //Helpers
    bool Element_Name_IsOK();

    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_ADVANCED
            int8u sampling_frequency_Frame0;
            int8u mode_Frame0;
            bool  File_Demux_Unpacketize_StreamLayoutChange_Skip;
        #endif //MEDIAINFO_ADVANCED
    #endif //MEDIAINFO_DEMUX
};

} //NameSpace

#endif
