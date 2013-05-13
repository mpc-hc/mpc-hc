/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_PCMM2TS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Pcm_M2ts.h"
#if MEDIAINFO_DEMUX
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#endif //MEDIAINFO_DEMUX
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
int8u Pcm_M2TS_channel_assignment[16]=
{
        0,
        1,
        0,
        2,
        3,
        3,
        4,
        4,
        5,
        6,
        7,
        8,
        0,
        0,
        0,
        0,
};

//---------------------------------------------------------------------------
int32u Pcm_M2TS_sampling_frequency[16]=
{
        0,
    48000,
        0,
        0,
    96000,
   192000,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
};

//---------------------------------------------------------------------------
int8u Pcm_M2TS_bits_per_sample[4]=
{
        0,
       16,
       20,
       24,
};

//---------------------------------------------------------------------------
extern const char* Pcm_VOB_ChannelsPositions(int8u channel_assignment);
extern const char* Pcm_VOB_ChannelsPositions2(int8u channel_assignment);

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Pcm_M2ts::File_Pcm_M2ts()
{
    //Configuration
    ParserName=__T("PCM M2TS");
    IsRawStream=true;
    PTS_DTS_Needed=true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm_M2ts::Streams_Fill()
{
    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "PCM");
    Fill(Stream_Audio, 0, Audio_Codec, "PCM");
    Fill(Stream_Audio, 0, Audio_Codec_Family, "PCM");
    Fill(Stream_Audio, 0, Audio_MuxingMode, "Blu-ray");
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");

    int8u Channels=Pcm_M2TS_channel_assignment[channel_assignment];
    if (Channels)
    {
        if (Pcm_M2TS_sampling_frequency[sampling_frequency])
            Fill(Stream_Audio, 0, Audio_SamplingRate, Pcm_M2TS_sampling_frequency[sampling_frequency]);
        if (Pcm_M2TS_bits_per_sample[bits_per_sample])
            Fill(Stream_Audio, 0, Audio_BitDepth, Pcm_M2TS_bits_per_sample[bits_per_sample]);
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, 0, Audio_ChannelPositions, Pcm_VOB_ChannelsPositions(channel_assignment));
        Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Pcm_VOB_ChannelsPositions2(channel_assignment));
        if (Pcm_M2TS_sampling_frequency[sampling_frequency] && Pcm_M2TS_bits_per_sample[bits_per_sample])
        {
            if (Channels%2)
                Fill(Stream_Audio, 0, Audio_BitRate_Encoded, Pcm_M2TS_sampling_frequency[sampling_frequency]*(Channels+1)*Pcm_M2TS_bits_per_sample[bits_per_sample]); //Always by pair
            Fill(Stream_Audio, 0, Audio_BitRate, Pcm_M2TS_sampling_frequency[sampling_frequency]*Channels*Pcm_M2TS_bits_per_sample[bits_per_sample]);
        }
    }

    Fill(Stream_Audio, 0, Audio_Format_Settings, "Big");
    Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, "Big");
    Fill(Stream_Audio, 0, Audio_Codec_Settings, "Big");
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Endianness, "Big");
    Fill(Stream_Audio, 0, Audio_Format_Settings, "Signed");
    Fill(Stream_Audio, 0, Audio_Format_Settings_Sign, "Signed");
    Fill(Stream_Audio, 0, Audio_Codec_Settings, "Signed");
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Sign, "Signed");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm_M2ts::Read_Buffer_Continue()
{
    if (Buffer_Size==0)
        return;

    //Parsing
    int16u  audio_data_payload_size;
    Get_B2 (   audio_data_payload_size,                         "audio_data_payload_size");
    BS_Begin();
    Get_S1 (4, channel_assignment,                              "channel_assignment"); Param_Info2(Pcm_M2TS_channel_assignment[channel_assignment], " channel(s)");
    Get_S1 (4, sampling_frequency,                              "sampling_frequency"); Param_Info2(Pcm_M2TS_sampling_frequency[sampling_frequency], " Hz");
    Get_S1 (2, bits_per_sample,                                 "bits_per_sample"); Param_Info2(Pcm_M2TS_bits_per_sample[bits_per_sample], " bits");
    Skip_SB(                                                    "start_flag");
    Skip_S1(5,                                                  "reserved");
    BS_End();
    Skip_XX(audio_data_payload_size,                            "audio_data_payload");

    FILLING_BEGIN_PRECISE();
        if (!Status[IsAccepted])
        {
            Accept();
            Finish();
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_PCMM2TS_YES
