/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Important note about Dolby E parser:
// Open-Source version of MediaInfo has basic support of Dolby E,
// created by reverse engineering by an anonymous third-party,
// with the limitions due to reverse engineering: there is a risk
// of wrong interpretation of the reverse engineeried bitstream.
// If you want a safer Dolby E support, based on Dolby E specifications,
// contact http://www.dolby.com/about/contact_us/contactus.aspx?goto=28
// for a license before asking it in a specific (and not Open-Source) version
// of MediaInfo.
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
#if defined(MEDIAINFO_DOLBYE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_DolbyE.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const int8u DolbyE_Programs[64]=
{2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 8, 1, 2, 3, 3, 4, 5, 6, 1, 2, 3, 4, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//---------------------------------------------------------------------------
const int8u DolbyE_Channels[64]=
{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 6, 6, 4, 4, 4, 4, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//---------------------------------------------------------------------------
const int8u DolbyE_Channels_PerProgram(int8u program_config, int8u program)
{
    switch (program_config)
    {
        case  0 :   switch (program)
                    {
                        case  0 :   return 6;
                        default :   return 2;
                    }
        case  1 :   switch (program)
                    {
                        case  0 :   return 6;
                        default :   return 1;
                    }
        case  2 :
        case 18 :   return 4;
        case  3 :
        case 12 :   switch (program)
                    {
                        case  0 :   return 4;
                        default :   return 2;
                    }
        case  4 :   switch (program)
                    {
                        case  0 :   return 4;
                        case  1 :   return 2;
                        default :   return 1;
                    }
        case  5 :
        case 13 :   switch (program)
                    {
                        case  0 :   return 4;
                        default :   return 1;
                    }
        case  6 :
        case 14 :
        case 19 :   return 2;
        case  7 :   switch (program)
                    {
                        case  0 :
                        case  1 :
                        case  2 :   return 2;
                        default :   return 1;
                    }
        case  8 :
        case 15 :   switch (program)
                    {
                        case  0 :
                        case  1 :   return 2;
                        default :   return 1;
                    }
        case  9 :
        case 16 :
        case 20 :   switch (program)
                    {
                        case  0 :   return 2;
                        default :   return 1;
                    }
        case 10 :
        case 17 :
        case 21 :   return 1;
        case 11 :   return 6;
        case 22 :   return 8;
        case 23 :   return 8;
        default :   return 0;
    }
};

//---------------------------------------------------------------------------
const char*  DolbyE_ChannelPositions[64]=
{
    "Front: L C R, Side: L R, LFE / Front: L R",
    "Front: L C R, Side: L R, LFE / Front: C / Front: C",
    "Front: L C R, LFE / Front: L C R, LFE",
    "Front: L C R, LFE / Front: L R / Front: L R",
    "Front: L C R, LFE / Front: L R / Front: C / Front: C",
    "Front: L C R, LFE / Front: C / Front: C / Front: C / Front: C",
    "Front: L R / Front: L R / Front: L R / Front: L R",
    "Front: L R / Front: L R / Front: L R / Front: C / Front: C",
    "Front: L R / Front: L R / Front: C / Front: C / Front: C / Front: C",
    "Front: L R / Front: C / Front: C / Front: C / Front: C / Front: C / Front: C",
    "Front: C / Front: C / Front: C / Front: C / Front: C / Front: C / Front: C / Front: C",
    "Front: L C R, Side: L R, LFE",
    "Front: L C R, LFE / Front: L R",
    "Front: L C R, LFE / Front: C / Front: C",
    "Front: L R / Front: L R / Front: L R",
    "Front: L R / Front: L R / Front: C / Front: C",
    "Front: L R / Front: C / Front: C / Front: C / Front: C",
    "Front: C / Front: C / Front: C / Front: C / Front: C / Front: C",
    "Front: L C R, LFE",
    "Front: L R / Front: L R",
    "Front: L R / Front: C / Front: C",
    "Front: C / Front: C / Front: C / Front: C",
    "Front: L C R, Side: L R, Rear: L R, LFE",
    "Front: L C C C R, Side: L R, LFE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char*  DolbyE_ChannelPositions_PerProgram(int8u program_config, int8u program)
{
    switch (program_config)
    {
        case  0 :   switch (program)
                    {
                        case  0 :   return "Front: L C R, Side: L R, LFE";
                        default :   return "Front: L R";
                    }
        case  1 :   switch (program)
                    {
                        case  0 :   return "Front: L C R, Side: L R, LFE";
                        default :   return "Front: C";
                    }
        case  2 :
        case 18 :   return "Front: L C R, LFE";
        case  3 :
        case 12 :   switch (program)
                    {
                        case  0 :   return "Front: L C R, LFE";
                        default :   return "Front: L R";
                    }
        case  4 :   switch (program)
                    {
                        case  0 :   return "Front: L C R, LFE";
                        case  1 :   return "Front: L R";
                        default :   return "Front: C";
                    }
        case  5 :
        case 13 :   switch (program)
                    {
                        case  0 :   return "Front: L C R, LFE";
                        default :   return "Front: C";
                    }
        case  6 :
        case 14 :
        case 19 :   return "Front: L R";
        case  7 :   switch (program)
                    {
                        case  0 :
                        case  1 :
                        case  2 :   return "Front: L R";
                        default :   return "Front: C";
                    }
        case  8 :
        case 15 :   switch (program)
                    {
                        case  0 :
                        case  1 :   return "Front: L R";
                        default :   return "Front: C";
                    }
        case  9 :
        case 16 :
        case 20 :   switch (program)
                    {
                        case  0 :   return "Front: L R";
                        default :   return "Front: C";
                    }
        case 10 :
        case 17 :
        case 21 :   return "Front: C";
        case 11 :   return "Front: L C R, Side: L R, LFE";
        case 22 :   return "Front: L C R, Side: L R, Rear: L R, LFE";
        case 23 :   return "Front: L C C C R, Side: L R, LFE";
        default :   return "";
    }
};

//---------------------------------------------------------------------------
const char*  DolbyE_ChannelPositions2[64]=
{
    "3/2/0.1 / 2/0/0",
    "3/2/0.1 / 1/0/0 / 1/0/0",
    "3/0/0.1 / 3/0/0.1",
    "3/0/0.1 / 2/0/0 / 2/0/0",
    "3/0/0.1 / 2/0/0 / 1/0/0 / 1/0/0",
    "3/0/0.1 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "2/0/0 / 2/0/0 / 2/0/0 / 2/0/0",
    "2/0/0 / 2/0/0 / 2/0/0 / 1/0/0 / 1/0/0",
    "2/0/0 / 2/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "2/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "3/2/0.1",
    "3/0/0.1 / 2/0/0",
    "3/0/0.1 / 1/0/0 / 1/0/0",
    "2/0/0 / 2/0/0 / 2/0/0",
    "2/0/0 / 2/0/0 / 1/0/0 / 1/0/0",
    "2/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "3/0/0.1",
    "2/0/0 / 2/0/0",
    "2/0/0 / 1/0/0 / 1/0/0",
    "1/0/0 / 1/0/0 / 1/0/0 / 1/0/0",
    "3/2/2.1",
    "5/2/0.1",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char*  DolbyE_ChannelPositions2_PerProgram(int8u program_config, int8u program)
{
    switch (program_config)
    {
        case  0 :   switch (program)
                    {
                        case  0 :   return "3/2/0.1";
                        default :   return "2/0/0";
                    }
        case  1 :   switch (program)
                    {
                        case  0 :   return "3/2/0.1";
                        default :   return "1/0/0";
                    }
        case  2 :
        case 18 :   return "3/0/0.1";
        case  3 :
        case 12 :   switch (program)
                    {
                        case  0 :   return "3/0/0.1";
                        default :   return "2/0/0";
                    }
        case  4 :   switch (program)
                    {
                        case  0 :   return "3/0/0.1";
                        case  1 :   return "2/0/0";
                        default :   return "1/0/0";
                    }
        case  5 :
        case 13 :   switch (program)
                    {
                        case  0 :   return "3/0/0.1";
                        default :   return "1/0/0";
                    }
        case  6 :
        case 14 :
        case 19 :   return "Front: L R";
        case  7 :   switch (program)
                    {
                        case  0 :
                        case  1 :
                        case  2 :   return "2/0/0";
                        default :   return "1/0/0";
                    }
        case  8 :
        case 15 :   switch (program)
                    {
                        case  0 :
                        case  1 :   return "2/0/0";
                        default :   return "1/0/0";
                    }
        case  9 :
        case 16 :
        case 20 :   switch (program)
                    {
                        case  0 :   return "2/0/0";
                        default :   return "1/0/0";
                    }
        case 10 :
        case 17 :
        case 21 :   return "1/0/0";
        case 11 :   return "3/2/0.1";
        case 22 :   return "3/2/2.1";
        case 23 :   return "5/2/0.1";
        default :   return "";
    }
};

extern const char*  AC3_Surround[];

//---------------------------------------------------------------------------
const char*  DolbyE_ChannelLayout_PerProgram(int8u ProgramConfiguration, int8u ProgramNumber)
{
    switch (ProgramConfiguration)
    {
        case  0 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C Ls X R LFE Rs X";
                        default :   return "X X X L X X X R";
                    }
        case  1 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C Ls X R LFE Rs X";
                        case  1 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case  2 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C X X R S X X";
                        default :   return "X X L C X X R S";
                    }
        case  3 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C X X R S X X";
                        case  1 :   return "X X L X X X R X";
                        default :   return "X X X L X X X R";
                    }
        case  4 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C X X R S X X";
                        case  1 :   return "X X L X X X R X";
                        case  2 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case  5 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C X X R S X X";
                        case  1 :   return "X X C X X X X X";
                        case  2 :   return "X X X X X X C X";
                        case  3 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case  6 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X X R X X X";
                        case  1 :   return "X L X X X R X X";
                        case  2 :   return "X X L X X X R X";
                        default :   return "X X X L X X X R";
                    }
        case  7 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X X R X X X";
                        case  1 :   return "X L X X X R X X";
                        case  2 :   return "X X L X X X R X";
                        case  3 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case  8 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X X R X X X";
                        case  1 :   return "X L X X X R X X";
                        case  2 :   return "X X C X X X X X";
                        case  3 :   return "X X X X X X C X";
                        case  4 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case  9 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X X R X X X";
                        case  1 :   return "X C X X X X X X";
                        case  2 :   return "X X X X X C X X";
                        case  3 :   return "X X C X X X X X";
                        case  4 :   return "X X X X X X C X";
                        case  5 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case 10 :   switch (ProgramNumber)
                    {
                        case  0 :   return "C X X X X X X X";
                        case  1 :   return "X X X X C X X X";
                        case  2 :   return "X C X X X X X X";
                        case  3 :   return "X X X X X C X X";
                        case  4 :   return "X X C X X X X X";
                        case  5 :   return "X X X X X X C X";
                        case  6 :   return "X X X C X X X X";
                        default :   return "X X X X X X X C";
                    }
        case 11 :   return "L C Ls R LFE Rs";
        case 12 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C X R S X";
                        default :   return "X X L X X R";
                    }
        case 13 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L C X R S X";
                        case  1 :   return "X X C X X X";
                        default :   return "X X X X X C";
                    }
        case 14 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X R X X";
                        case  1 :   return "X L X X R X";
                        default :   return "X X L X X R";
                    }
        case 15 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X R X X";
                        case  1 :   return "X L X R X";
                        case  2 :   return "X X C X X X";
                        default :   return "X X X X X C";
                    }
        case 16 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X X R X X";
                        case  1 :   return "X C X X X X";
                        case  2 :   return "X X X X C X";
                        case  3 :   return "X X C X X X";
                        default :   return "X X X X X C";
                    }
        case 17 :   switch (ProgramNumber)
                    {
                        case  0 :   return "C X X X X X";
                        case  1 :   return "X X X C X X";
                        case  2 :   return "X C X X X X";
                        case  3 :   return "X X X X C X";
                        case  4 :   return "X X C X X X";
                        default :   return "X X X X X C";
                    }
        case 18 :   return "L C R S";
        case 19 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X R X";
                        default :   return "X L X R";
                    }
        case 20 :   switch (ProgramNumber)
                    {
                        case  0 :   return "L X R X";
                        case  1 :   return "X C X X";
                        default :   return "X X X C";
                    }
        case 21 :   switch (ProgramNumber)
                    {
                        case  0 :   return "C X X X";
                        case  1 :   return "X X C X";
                        case  2 :   return "X C X X";
                        default :   return "X X X C";
                    }
        case 22 :   return "L C Ls Lrs R LFE Rs Rrs";
        case 23 :   return "L C Ls Lc R LFE Rs Rc";
        default :   return "";
    }
};

extern const float32 Mpegv_frame_rate[16];

const bool Mpegv_frame_rate_type[16]=
{false, false, false, false, false, false, true, true, true, false, false, false, false, false, false, false};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_DolbyE::File_DolbyE()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_DolbyE;
    #endif //MEDIAINFO_EVENTS

    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;

    //In
    GuardBand_Before=0;
    GuardBand_After=0;

    //Temp
    SMPTE_time_code_StartTimecode=(int64u)-1;
    FrameInfo.DTS=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_DolbyE::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "Dolby E");
    for (int8u program=0; program<DolbyE_Programs[ProgramConfiguration]; program++)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format, "Dolby E");
        if (DolbyE_Programs[ProgramConfiguration]>1)
            Fill(Stream_Audio, StreamPos_Last, Audio_ID, Count_Get(Stream_Audio));
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, DolbyE_Channels_PerProgram(ProgramConfiguration, program));
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, DolbyE_ChannelPositions_PerProgram(ProgramConfiguration, program));
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions_String2, DolbyE_ChannelPositions2_PerProgram(ProgramConfiguration, program));
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelLayout, DolbyE_ChannelLayout_PerProgram(ProgramConfiguration, program));
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, 48000);
        Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, BitDepth);
        if (SMPTE_time_code_StartTimecode!=(int64u)-1)
        {
            Fill(StreamKind_Last, StreamPos_Last, Audio_Delay, SMPTE_time_code_StartTimecode);
            Fill(StreamKind_Last, StreamPos_Last, Audio_Delay_Source, "Stream");
        }

        Fill(Stream_Audio, StreamPos_Last, Audio_FrameRate, Mpegv_frame_rate[FrameRate]);
        if (FrameInfo.PTS!=(int64u)-1 && BitDepth)
        {
            float BitRate=(float)(96000*BitDepth);

            if (GuardBand_Before_Initial)
            {
                float GuardBand_Before_Initial_Duration=GuardBand_Before_Initial*8/BitRate;
                Fill(Stream_Audio, StreamPos_Last, "GuardBand_Before", GuardBand_Before_Initial_Duration, 9);
                Fill(Stream_Audio, StreamPos_Last, "GuardBand_Before/String", Ztring::ToZtring(GuardBand_Before_Initial_Duration*1000000, 0)+Ztring().From_UTF8(" \xC2xB5s")); //0xC2 0xB5 = micro sign
                (*Stream_More)[Stream_Audio][StreamPos_Last](Ztring().From_Local("GuardBand_Before"), Info_Options)=__T("N NT");
                (*Stream_More)[Stream_Audio][StreamPos_Last](Ztring().From_Local("GuardBand_Before/String"), Info_Options)=__T("N NT");
            }
            if (GuardBand_Before_Initial)
            {
                float GuardBand_After_Initial_Duration=GuardBand_After_Initial*8/BitRate;
                Fill(Stream_Audio, StreamPos_Last, "GuardBand_After", GuardBand_After_Initial_Duration, 9);
                Fill(Stream_Audio, StreamPos_Last, "GuardBand_After/String", Ztring::ToZtring(GuardBand_After_Initial_Duration*1000000, 0)+Ztring().From_UTF8(" \xC2xB5s")); //0xC2 0xB5 = micro sign
                (*Stream_More)[Stream_Audio][StreamPos_Last](Ztring().From_Local("GuardBand_After"), Info_Options)=__T("N NT");
                (*Stream_More)[Stream_Audio][StreamPos_Last](Ztring().From_Local("GuardBand_After/String"), Info_Options)=__T("N NT");
            }
        }
    }
    Fill(Stream_General, 0, General_OverallBitRate, Element_Size*8*Mpegv_frame_rate[FrameRate], 0);
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_DolbyE::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+32<=Buffer_Size)
    {
        if ((CC2(Buffer+Buffer_Offset_Temp)&0xFFFE)==0x078E) //16-bit
        {
            BitDepth=16;
            ScrambledBitStream=(CC2(Buffer+Buffer_Offset)&0x0001)?true:false;
            break; //while()
        }
        if ((CC3(Buffer+Buffer_Offset)&0xFFFFE0)==0x0788E0) //20-bit
        {
            BitDepth=20;
            ScrambledBitStream=(CC3(Buffer+Buffer_Offset)&0x000010)?true:false;
            break; //while()
        }
        if ((CC3(Buffer+Buffer_Offset)&0xFFFFFE)==0x07888E) //24-bit
        {
            BitDepth=24;
            ScrambledBitStream=(CC3(Buffer+Buffer_Offset)&0x000001)?true:false;
            break; //while()
        }
        Buffer_Offset++;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+8>Buffer_Size)
        return false;

    //Synched
    return true;
}

//---------------------------------------------------------------------------
bool File_DolbyE::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+32>Buffer_Size)
        return false;

    //Quick test of synchro
    switch (BitDepth)
    {
        case 16 : if ((CC2(Buffer+Buffer_Offset)&0xFFFE  )!=0x078E  ) {Synched=false; return true;} break;
        case 20 : if ((CC3(Buffer+Buffer_Offset)&0xFFFFE0)!=0x0788E0) {Synched=false; return true;} break;
        case 24 : if ((CC3(Buffer+Buffer_Offset)&0xFFFFFE)!=0x07888E) {Synched=false; return true;} break;
        default : ;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_DolbyE::Header_Parse()
{
    //Filling
    if (IsSub)
        Header_Fill_Size(Buffer_Size-Buffer_Offset);
    else
    {
        //Looking for synchro
        //Synchronizing
        Buffer_Offset_Temp=Buffer_Offset+3;
        if (BitDepth==16)
            while (Buffer_Offset_Temp+2<=Buffer_Size)
            {
                if ((CC2(Buffer+Buffer_Offset_Temp)&0xFFFE)==0x078E) //16-bit
                    break; //while()
                Buffer_Offset_Temp++;
            }
        if (BitDepth==20)
            while (Buffer_Offset_Temp+3<=Buffer_Size)
            {
                if ((CC3(Buffer+Buffer_Offset_Temp)&0xFFFFE0)==0x0788E0) //20-bit
                    break; //while()
                Buffer_Offset_Temp++;
            }
        if (BitDepth==24)
            while (Buffer_Offset_Temp+3<=Buffer_Size)
            {
                if ((CC3(Buffer+Buffer_Offset_Temp)&0xFFFFFE)==0x07888E) //24-bit
                    break; //while()
                Buffer_Offset_Temp++;
            }

        if (Buffer_Offset_Temp+(BitDepth>16?3:2)>Buffer_Size)
        {
            if (File_Offset+Buffer_Size==File_Size)
                Buffer_Offset_Temp=Buffer_Size;
            else
            {
                Element_WaitForMoreData();
                return;
            }
        }

        Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
    }
    Header_Fill_Code(0, "Frame");
}

//---------------------------------------------------------------------------
void File_DolbyE::Data_Parse()
{
    //In case of scrambling
    const int8u*    Save_Buffer=NULL;
    size_t          Save_Buffer_Offset=0;
    int64u          Save_File_Offset=0;
    if (ScrambledBitStream)
    {
        //We must change the buffer,
        Save_Buffer=Buffer;
        Save_Buffer_Offset=Buffer_Offset;
        Save_File_Offset=File_Offset;
        File_Offset+=Buffer_Offset;
        Buffer_Offset=0;
        Descrambled_Buffer=new int8u[(size_t)Element_Size];
        std::memcpy(Descrambled_Buffer, Save_Buffer+Save_Buffer_Offset, (size_t)Element_Size);
        Buffer=Descrambled_Buffer;
    }

    //Parsing
    BS_Begin();
    Block();
    BS_End();

    //In case of scrambling
    if (ScrambledBitStream)
    {
        delete[] Buffer; Buffer=Save_Buffer;
        Buffer_Offset=Save_Buffer_Offset;
        File_Offset=Save_File_Offset;
    }

    FILLING_BEGIN();
        if (!Status[IsAccepted])
        {
            Accept("Dolby E");

            //Guard band
            GuardBand_Before_Initial=GuardBand_Before;
            GuardBand_After_Initial=GuardBand_After;
        }
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (Mpegv_frame_rate[FrameRate])
            FrameInfo.DUR=float64_int64s(1000000000/Mpegv_frame_rate[FrameRate]);
        else
            FrameInfo.DUR=(int64u)-1;
        if (FrameInfo.DTS!=(int64u)-1)
            FrameInfo.PTS=FrameInfo.DTS+=FrameInfo.DUR;
        if (Frame_Count==1)
        {
            Finish("Dolby E");
        }
    FILLING_END();
    if (Frame_Count==0 && Buffer_TotalBytes>Buffer_TotalBytes_FirstSynched_Max)
        Reject("Dolby E");
}

//---------------------------------------------------------------------------
void File_DolbyE::Block()
{
    //Parsing
    Skip_S3(BitDepth,                                      "Synchro");
    if (ScrambledBitStream)
    {
        //We must change the buffer
        switch (BitDepth)
        {
            case 16 :
                        if (!Descramble_16bit())
                            return;
                        break;
            case 20 :
                        if (!Descramble_20bit())
                            return;
                        break;
            case 24 :
                        if (!Descramble_24bit())
                            return;
                        break;
            default :   ;
        }
    }
    Skip_S2(14,                                                 "Unknown");
    Get_S1 ( 6, ProgramConfiguration,                           "Program configuration"); Param_Info1(DolbyE_ChannelPositions[ProgramConfiguration]);
    Get_S1 ( 4, FrameRate,                                      "Frame rate 1"); Param_Info3(Mpegv_frame_rate[FrameRate], 3, " fps");
    Skip_S1( 4,                                                 "Frame rate 2? Always same as Frame rate 1");
    Skip_S2(16,                                                 "Frame number?");
    Element_Begin1("SMPTE time code?");
    int8u Frames_Units, Frames_Tens, Seconds_Units, Seconds_Tens, Minutes_Units, Minutes_Tens, Hours_Units, Hours_Tens;
    bool  DropFrame;

    Skip_S1(4,                                                  "BG8");
    Skip_S1(4,                                                  "BG7");

    Skip_SB(                                                    "BGF2 / Field Phase");
    Skip_SB(                                                    "BGF1");
    Get_S1 (2, Hours_Tens,                                      "Hours (Tens)");
    Get_S1 (4, Hours_Units,                                     "Hours (Units)");

    Skip_S1(4,                                                  "BG6");
    Skip_S1(4,                                                  "BG5");

    Skip_SB(                                                    "BGF0 / BGF2");
    Get_S1 (3, Minutes_Tens,                                    "Minutes (Tens)");
    Get_S1 (4, Minutes_Units,                                   "Minutes (Units)");

    Skip_S1(4,                                                  "BG4");
    Skip_S1(4,                                                  "BG3");

    Skip_SB(                                                    "FP - Field Phase / BGF0");
    Get_S1 (3, Seconds_Tens,                                    "Seconds (Tens)");
    Get_S1 (4, Seconds_Units,                                   "Seconds (Units)");

    Skip_S1(4,                                                  "BG2");
    Skip_S1(4,                                                  "BG1");

    Skip_SB(                                                    "CF - Color fame");
    Get_SB (   DropFrame,                                       "DP - Drop frame");
    Get_S1 (2, Frames_Tens,                                     "Frames (Tens)");
    Get_S1 (4, Frames_Units,                                    "Frames (Units)");

    Skip_BS(Data_BS_Remain(),                                   "Unknown");

    if (Hours_Tens<3)
    {
        int64u TimeCode=(int64u)(Hours_Tens     *10*60*60*1000
                               + Hours_Units       *60*60*1000
                               + Minutes_Tens      *10*60*1000
                               + Minutes_Units        *60*1000
                               + Seconds_Tens         *10*1000
                               + Seconds_Units           *1000
                               + (Mpegv_frame_rate[FrameRate]?float64_int32s((Frames_Tens*10+Frames_Units)*1000/Mpegv_frame_rate[FrameRate]):0));

        Element_Info1(Ztring().Duration_From_Milliseconds(TimeCode));

        //TimeCode
        if (SMPTE_time_code_StartTimecode==(int64u)-1)
            SMPTE_time_code_StartTimecode=TimeCode;
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
bool File_DolbyE::Descramble_16bit ()
{
    int16u ScrambleMask;
    Get_S2 (16, ScrambleMask, "Scramble mask");
    int16u Size=((BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Size-Data_BS_Remain()/8)^ScrambleMask)>>2)&0x3FF;

    if (Data_BS_Remain()<(size_t)((Size+1)*BitDepth)) //+1 for additional unknown word
        return false; //There is a problem

    int8u* Temp=Descrambled_Buffer+(size_t)Element_Size-Data_BS_Remain()/8;
    for (int16u Pos=0; Pos<Size; Pos++)
        int16u2BigEndian(Temp+Pos*2, BigEndian2int16u(Temp+Pos*2)^ScrambleMask);

    return true;
}

//---------------------------------------------------------------------------
bool File_DolbyE::Descramble_20bit ()
{
    int32u ScrambleMask;
    Get_S3 (20, ScrambleMask, "Scramble mask");
    int16u Size=((BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Size-Data_BS_Remain()/8)^(ScrambleMask>>4))>>2)&0x3FF;

    if (Data_BS_Remain()<(size_t)((Size+1)*BitDepth)) //+1 for additional unknown word
        return false; //There is a problem

    int8u* Temp=Descrambled_Buffer+(size_t)Element_Size-Data_BS_Remain()/8;
    int64u ScrambleMasks=(((int64u)ScrambleMask)<<20)|ScrambleMask;
    bool Half;
    if (Data_BS_Remain()%8)
    {
        Temp--;
        int24u2BigEndian(Temp, BigEndian2int24u(Temp)^(ScrambleMask));
        Half=true;
    }
    else
        Half=false;
    for (int16u Pos=0; Pos<Size-(Half?1:0); Pos+=2)
        int40u2BigEndian(Temp+(Half?3:0)+Pos*5/2, BigEndian2int40u(Temp+(Half?3:0)+Pos*5/2)^ScrambleMasks);
    if ((Size-((Size && Half)?1:0))%2==0)
        int24u2BigEndian(Temp+(Half?3:0)+(Size-((Size && Half)?1:0))*5/2, BigEndian2int24u(Temp+(Half?3:0)+(Size-((Size && Half)?1:0))*5/2)^(((int32u)ScrambleMasks)<<4));

    return true;
}

//---------------------------------------------------------------------------
bool File_DolbyE::Descramble_24bit ()
{
    int32u ScrambleMask;
    Get_S3 (24, ScrambleMask, "Scramble mask");
    int32u Size=((BigEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Size-Data_BS_Remain()/8)^ScrambleMask)>>2)&0x3FF;

    if (Data_BS_Remain()<(size_t)((Size+1)*BitDepth)) //+1 for additional unknown word
        return false; //There is a problem

    int8u* Temp=Descrambled_Buffer+(size_t)Element_Size-Data_BS_Remain()/8;
    for (int16u Pos=0; Pos<Size; Pos++)
        int24u2BigEndian(Temp+Pos*2, BigEndian2int24u(Temp+Pos*2)^ScrambleMask);

    return true;
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_DOLBYE_YES
