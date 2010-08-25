// File_Mpc - Info for Musepack files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Source: http://trac.musepack.net/trac/wiki/SV7Specification
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//***************************************************************************
// Infos (Common)
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPC_YES) || defined(MEDIAINFO_MPCSV8_YES)
//---------------------------------------------------------------------------

#include "ZenLib/Conf.h"
using namespace ZenLib;

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern const int16u Mpc_SampleFreq[]=
{
    44100, //CD
    48000, //DAT, DVC, ADR
    37800, //CD-ROM-XA
    32000, //DSR, DAT-LP, DVC-LP
};

//---------------------------------------------------------------------------
} //NameSpace

//---------------------------------------------------------------------------
#endif //...
//---------------------------------------------------------------------------

//***************************************************************************
//
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Mpc.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mpc_Profile[]=
{
    "no profile",
    "Unstable/Experimental",
    "",
    "",
    "",
    "Below Telephone (q=0)",
    "Below Telephone (q=1)",
    "Telephone (q=2)",
    "Thumb (q=3)",
    "Radio (q=4)",
    "Standard (q=5)",
    "Xtreme (q=6)",
    "Insane (q=7)",
    "BrainDead (q=8)",
    "Above BrainDead (q=9)",
    "Above BrainDead (q=10)",
};

//---------------------------------------------------------------------------
const char* Mpc_Link[]=
{
    "Starts or ends with a very low level",
    "Ends loudly",
    "Starts loudly",
    "Starts loudly and ends loudly",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpc::File_Mpc()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;
}


//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mpc::FileHeader_Begin()
{
    //Tags
    if (!File__Tags_Helper::FileHeader_Begin())
        return false;

    if (Buffer_Offset+4>Buffer_Size)
        return false;

    //Test
    if (CC3(Buffer)!=0x4D502B || (CC1(Buffer+3)&0x0F)!=7) //"MP+" version 7
    {
        File__Tags_Helper::Reject("Musepack SV7");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Mpc::FileHeader_Parse()
{
    //Parsing
    Element_Begin("SV7 header", 21);
    Ztring Encoder;
    int32u FrameCount;
    int16u TitleGain, AlbumGain;
    int8u  Profile, Link, SampleFreq, EncoderVersion;

    Skip_C3(                                                    "Signature");
    BS_Begin();
    Skip_S1(4,                                                  "PNS");
    Skip_S1(4,                                                  "Version");
    BS_End();

    Get_L4 (FrameCount,                                         "FrameCount");

    Skip_L2(                                                    "MaxLevel");
    BS_Begin();
    Get_S1 (4, Profile,                                         "Profile"); Param_Info(Mpc_Profile[Profile]);
    Get_S1 (2, Link,                                            "Link"); Param_Info(Mpc_Link[Link]);
    Get_S1 (2, SampleFreq,                                      "SampleFreq"); Param_Info(Mpc_SampleFreq[SampleFreq]);
    Skip_SB(                                                    "IntensityStereo");
    Skip_SB(                                                    "MidSideStereo");
    Skip_S1(6,                                                  "MaxBand");
    BS_End();

    Skip_L2(                                                    "TitlePeak");
    Get_L2 (TitleGain,                                          "TitleGain"); Param_Info(((float32)((int16s)TitleGain))/1000, 2, " dB");

    Skip_L2(                                                    "AlbumPeak");
    Get_L2 (AlbumGain,                                          "AlbumGain"); Param_Info(((float32)((int16s)TitleGain))/1000, 2, " dB");

    BS_Begin();
    Skip_S2(16,                                                 "unused");
    Skip_S1( 4,                                                 "LastFrameLength (part 1)");
    Skip_SB(                                                    "FastSeekingSafe");
    Skip_S1( 3,                                                 "unused");
    Skip_SB(                                                    "TrueGapless");
    Skip_S1( 7,                                                 "LastFrameLength (part 2)");
    BS_End();

    Get_L1 (EncoderVersion,                                     "EncoderVersion");
    Encoder.From_Number(((float)EncoderVersion)/100, 2); if (EncoderVersion%10==0); else if (EncoderVersion%2==0) Encoder+=_T(" Beta"); else if (EncoderVersion%2==1) Encoder+=_T(" Alpha"); Param_Info(Encoder);

    Element_End();

    FILLING_BEGIN();
        File__Tags_Helper::Accept("Musepack SV7");

        File__Tags_Helper::Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_SamplingRate, Mpc_SampleFreq[SampleFreq]);
        Fill(Stream_Audio, 0, Audio_SamplingCount, FrameCount*1152);
        Fill(Stream_Audio, 0, Audio_Format, "Musepack SV7");
        Fill(Stream_Audio, 0, Audio_Codec, "SV7");
        Fill(Stream_Audio, 0, Audio_Codec_Settings, Mpc_Profile[Profile]);
        Fill(Stream_Audio, 0, Audio_Encoded_Library, Encoder);
        Fill(Stream_Audio, 0, Audio_Resolution, 16); //MPC support only 16 bits
        Fill(Stream_Audio, 0, Audio_Duration, ((int64u)FrameCount)*1152*1000/Mpc_SampleFreq[SampleFreq]);
        if (FrameCount)
            Fill(Stream_Audio, 0, Audio_BitRate, (File_Size-25)*8*Mpc_SampleFreq[SampleFreq]/FrameCount/1152);

        //No more need data
        File__Tags_Helper::Finish("Musepack SV7");
    FILLING_END();
}

} //Namespace

#endif //MEDIAINFO_MPC_YES

