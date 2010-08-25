// File_Ape - Info for Ape files
// Copyright (C) 2003-2009 Jasper van de Gronde, th.v.d.gronde@hccnet.nl
// Copyright (C) 2003-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_APE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Ape.h"
using namespace std;
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
int32u Ape_SamplesPerFrame(int16u Version, int16u CompressionLevel)
{
    if (Version>=3950)
        return 73728*4;
    else if (Version>=3900)
        return 73728;
    else if (Version>=3800 && CompressionLevel==4000)
        return 73728;
    else
        return 9216;
}

//---------------------------------------------------------------------------
const char* Ape_Codec_Settings(int16u Setting)
{
    switch (Setting)
    {
        case 1000 : return "Fast";
        case 2000 : return "Normal";
        case 3000 : return "High";
        case 4000 : return "Extra-high";
        case 5000 : return "Insane";
        default   : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ape::File_Ape()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ape::Streams_Finish()
{
    //Filling
    int64u CompressedSize=File_Size-TagsSize;
    float32 CompressionRatio=((float32)UncompressedSize)/CompressedSize;
    int64u BitRate=Duration?(CompressedSize*8*1000/Duration):0;

    Fill(Stream_Audio, 0, Audio_CompressionRatio, CompressionRatio);
    Fill(Stream_Audio, 0, Audio_BitRate, BitRate);

    File__Tags_Helper::Streams_Finish();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ape::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Testing
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)!=0x4D414320) //"MAC "
    {
        File__Tags_Helper::Reject("APE");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Ape::FileHeader_Parse()
{
    //Parsing
    int32u SampleRate=0, TotalFrames=0, FinalFrameSamples=0, SamplesPerFrame=0, SeekElements;
    int16u Version, CompressionLevel=0, Flags=0, Channels=0, Resolution=0;
    bool Resolution8=false, Resolution24=false, no_wav_header;
    Skip_C4(                                                    "Identifier");
    Get_L2 (Version,                                            "Version");
    if (Version<3980) //<3.98
    {
        Get_L2 (CompressionLevel,                               "CompressionLevel"); Param_Info(Ape_Codec_Settings(CompressionLevel));
        Get_L2 (Flags,                                          "FormatFlags");
            Get_Flags (Flags, 0, Resolution8,                   "8-bit");
            Skip_Flags(Flags, 1,                                "crc-32");
            Skip_Flags(Flags, 2,                                "peak_level");
            Get_Flags (Flags, 3, Resolution24,                  "24-bit");
            Skip_Flags(Flags, 4,                                "seek_elements");
            Get_Flags (Flags, 5, no_wav_header,                 "no_wav_header");
        if (Resolution8)
            Resolution=8;
        else if (Resolution24)
            Resolution=24;
        else
            Resolution=16;
        Get_L2 (Channels,                                       "Channels");
        Get_L4 (SampleRate,                                     "SampleRate");
        Skip_L4(                                                "WavHeaderDataBytes");
        Skip_L4(                                                "WavTerminatingBytes");
        Get_L4 (TotalFrames,                                    "TotalFrames");
        Get_L4 (FinalFrameSamples,                              "FinalFrameSamples");
        SamplesPerFrame=Ape_SamplesPerFrame(Version, CompressionLevel);
        Skip_L4(                                                "PeakLevel");
        Get_L4 (SeekElements,                                   "SeekElements");
        if (!no_wav_header)
            Skip_XX(44,                                         "RIFF header");
        Skip_XX(SeekElements*4,                                 "Seek table");
    }
    else
    {
        Skip_L2(                                                "Version_High");
        Skip_L4(                                                "DescriptorBytes");
        Skip_L4(                                                "HeaderBytes");
        Skip_L4(                                                "SeekTableBytes");
        Skip_L4(                                                "WavHeaderDataBytes");
        Skip_L4(                                                "APEFrameDataBytes");
        Skip_L4(                                                "APEFrameDataBytesHigh");
        Skip_L4(                                                "WavTerminatingDataBytes");
        Skip_L16(                                               "FileMD5");
        Get_L2 (CompressionLevel,                               "CompressionLevel"); Param_Info(Ape_Codec_Settings(CompressionLevel));
        Get_L2 (Flags,                                          "FormatFlags");
        Get_L4 (SamplesPerFrame,                                "BlocksPerFrame");
        Get_L4 (FinalFrameSamples,                              "FinalFrameBlocks");
        Get_L4 (TotalFrames,                                    "TotalFrames");
        Get_L2 (Resolution,                                     "BitsPerSample");
        Get_L2 (Channels,                                       "Channels");
        Get_L4 (SampleRate,                                     "SampleRate");
    }

    FILLING_BEGIN();
        //Coherancy
        int32u Samples=(TotalFrames-1)*SamplesPerFrame+FinalFrameSamples;
        if (Samples==0 || SampleRate==0 || Channels==0 || Resolution==0)
        {
            File__Tags_Helper::Reject("APE");
            return;
        }

        //Filling
        File__Tags_Helper::Accept("APE");
        File__Tags_Helper::Streams_Fill();

        Duration=((int64u)Samples)*1000/SampleRate;
        UncompressedSize=Samples*Channels*(Resolution/8);

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "Monkey's Audio");
        Fill(Stream_Audio, 0, Audio_Encoded_Library_Settings, Ape_Codec_Settings(CompressionLevel));
        Fill(Stream_Audio, 0, Audio_Codec, "APE");
        Fill(Stream_Audio, 0, Audio_Resolution, Resolution);
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, 0, Audio_SamplingRate, SampleRate);
        Fill(Stream_Audio, 0, Audio_Duration, Duration);

        //No more need data
        File__Tags_Helper::Finish("APE");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_APE_YES

