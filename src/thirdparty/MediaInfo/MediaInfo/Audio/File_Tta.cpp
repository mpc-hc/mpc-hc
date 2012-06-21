// File_Tta - Info for TTA  files
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
// Source : http://tta.sourceforge.net/codec.format
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#if defined(MEDIAINFO_TTA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Tta.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Tta::File_Tta()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tta::Streams_Finish()
{
    //Filling
    int64u CompressedSize=File_Size-TagsSize;
    float32 CompressionRatio=((float32)UncompressedSize)/CompressedSize;

    Fill(Stream_Audio, 0, Audio_StreamSize, CompressedSize);
    Fill(Stream_Audio, 0, Audio_Compression_Ratio, CompressionRatio);
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");

    File__Tags_Helper::Streams_Finish();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Tta::FileHeader_Begin()
{
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    //Synchro
    if (Buffer_Offset+4>Buffer_Size)
        return false;
    if (CC4(Buffer+Buffer_Offset)!=0x54544131) //"TTA1"
    {
        File__Tags_Helper::Reject("TTA");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Tta::FileHeader_Parse()
{
    //Parsing
    int32u SampleRate, Samples, CRC32;
    int16u AudioFormat, Channels, BitsPerSample;
    Skip_C4(                                                    "Signature");
    Get_L2 (AudioFormat,                                        "AudioFormat");
    Get_L2 (Channels,                                           "NumChannels");
    Get_L2 (BitsPerSample,                                      "BitsPerSample");
    Get_L4 (SampleRate,                                         "SampleRate");
    Get_L4 (Samples,                                            "DataLength");
    Get_L4 (CRC32,                                              "CRC32");

    FILLING_BEGIN();
        if (SampleRate==0)
            return;
        Duration=((int64u)Samples)*1000/SampleRate;
        if (Duration==0)
            return;
        UncompressedSize=Samples*Channels*(BitsPerSample/8);
        if (UncompressedSize==0)
            return;

        //Filling data
        File__Tags_Helper::Accept("TTA");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "TTA");
        Fill(Stream_Audio, 0, Audio_Codec, "TTA ");
        Fill(Stream_Audio, 0, Audio_BitDepth, BitsPerSample);
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SampleRate);
        Fill(Stream_Audio, 0, Audio_Duration, Duration);
    FILLING_END();

    //No more need data
    File__Tags_Helper::Finish("TTA");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_TTA_YES
