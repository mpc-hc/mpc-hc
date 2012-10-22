// File_Opus - Info for Opus files
// Copyright (C) 2012-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
//
// Note : the buffer must be given in ONE call
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
#if defined(MEDIAINFO_OPUS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Opus.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char*  Opus_ChannelPositions[]=
{
    "",
    "Front: C",
    "Front: L R",
    "Front: L C R",
    "Front: L R,   Rear: L R",
    "Front: L C R, Rear: L R",
    "Front: L C R, Rear: L R, LFE",
    "Front: L C R, Side: L R, Rear: C, LFE",
    "Front: L C R, Side: L R, Rear: L R, LFE",
};

//---------------------------------------------------------------------------
const char*  Opus_ChannelPositions2[]=
{
    "",
    "1/0/0",
    "2/0/0",
    "2/1/0",
    "2/2/0",
    "3/2/0",
    "3/2/1",
    "3/2/2",
    "3/2/3",
};


//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Opus::File_Opus()
:File__Analyze()
{
    //Internal
    Identification_Done=false;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Opus::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Opus");
    Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
void File_Opus::Data_Parse()
{
    //Parsing
    if (Identification_Done)
        Stream();
    else
        Identification();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Opus::Identification()
{
    Element_Name("Identification");

    //Parsing
    Ztring opus_version, Channels_Positions;
    int8u Opus_version_id, ch_count, ch_map;
    int16u preskip;
    int32u sample_rate;
    Get_Local(8,opus_version,                                   "opus_codec_id");
    Get_L1 (Opus_version_id,                                    "opus_version_id");
    Get_L1 (ch_count,                                           "channel_count");
    Get_L2 (preskip,                                            "preskip");
    Get_L4 (sample_rate,                                        "rate");
    Skip_L2(                                                    "ouput_gain");
    Get_L1 (ch_map,                                             "channel_map");
    if (ch_map)
    {
        Skip_L1(                                                "Stream count (N)");
        Skip_L1(                                                "Two-channel stream count (M)");
        for (int8u Pos=0; Pos<ch_count; Pos++)
            Skip_L1(                                            "Channel mapping");
    }
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "unknown");

    //Filling
    FILLING_BEGIN()
        Accept("Opus");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "Opus");
        Fill(Stream_Audio, 0, Audio_Codec, "Opus");

        if (!opus_version.empty())
        {
            Fill(Stream_Audio, 0, Audio_SamplingRate, sample_rate?sample_rate:48000);
            Fill(Stream_Audio, 0, Audio_Channel_s_, ch_count);
        }

        switch (ch_map)
        {
            case 0 : // Mono/Stereo
                    if (ch_count>2)
                        break; // Not in spec
                    // else it is as Vorbis specs, no break
            case 1 : // Vorbis order
                    {
                    Ztring ChannelPositions; ChannelPositions.From_Local(Opus_ChannelPositions[ch_count]);
                    Ztring ChannelPositions2; ChannelPositions2.From_Local(Opus_ChannelPositions2[ch_count]);
                    if (ChannelPositions!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions))
                        Fill(Stream_Audio, 0, Audio_ChannelPositions, ChannelPositions);
                    if (ChannelPositions2!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions_String2))
                        Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, ChannelPositions2);
                    }
            default: ; //Unknown
        }

    FILLING_END();

    //Filling
    Identification_Done=true;
}

//---------------------------------------------------------------------------
void File_Opus::Stream()
{
    Element_Name("Stream");

    Skip_XX(Element_Size,                                       "Data");

    Finish("Opus");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_OPUS_YES
