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
#if defined(MEDIAINFO_DTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Dts.h"
#include "ZenLib/Utils.h"
#include "ZenLib/BitStream.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include <algorithm>
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char*  DTS_FrameType[]=
{
    "Termination",
    "Normal",
};

//---------------------------------------------------------------------------
const int32u DTS_SamplingRate[]=
{
        0,  8000,  16000,  32000,      0,      0,  11025,  22050,
    44100,     0,      0,  12000,  24000,  48000,  96000, 192000,
};

//---------------------------------------------------------------------------
const int32u DTS_BitRate[]=
{
      32000,   56000,   64000,   96000,  112000,  128000,  192000,  224000,
     256000,  320000,  384000,  448000,  512000,  576000,  640000,  754500,
     960000, 1024000, 1152000, 1280000, 1344000, 1408000, 1411200, 1472000,
    1509750, 1920000, 2048000, 3072000, 3840000,       0,       0,       0,
};

//---------------------------------------------------------------------------
const int8u DTS_Channels[]=
{
    1, 2, 2, 2, 2, 3, 3, 4,
    4, 5, 6, 6, 6, 7, 8, 8,
};

//---------------------------------------------------------------------------
const int8u DTS_Resolution[]=
{16, 20, 24, 24};

//---------------------------------------------------------------------------
const char*  DTS_ChannelPositions[16]=
{
    "Front: C",
    "Front: C C",
    "Front: L R",
    "Front: L R",
    "Front: L R",
    "Front: L C R",
    "Front: L R, Back: C",
    "Front: L C R, Back: C",
    "Front: L R, Side: L R",
    "Front: L C R, Side: L R",
    "Front: L C C R, Side: L R",
    "Front: L C R, Side: L R",
    "Front: L R, Side: L R, Back: L R",
    "Front: L C R, Side: L R, Back: L R",
    "Front: L R, Side: L R, Back: L C C R",
    "Front: L C R, Side: L R, Back: L C R",
};

//---------------------------------------------------------------------------
const char*  DTS_ChannelPositions2[16]=
{
    "1/0/0",
    "2/0/0",
    "2/0/0",
    "2/0/0",
    "2/0/0",
    "3/0/0",
    "2/1/0",
    "3/1/0",
    "2/2/0",
    "3/2/0",
    "4/2/0",
    "3/2/0",
    "2/2/2",
    "3/2/2",
    "2/2/4",
    "3/2/3",
};

//---------------------------------------------------------------------------
const char*  DTS_ChannelLayout[16]=
{
    "C",
    "1+1",
    "L R",
    "L R",
    "Lt Rt",
    "C L R",
    "L R Cs",
    "C L R Cs",
    "L R Ls Rs",
    "C L R Ls Rs",
    "Cl Cr L R Ls Rs",
    "C L R Ls Rs",
    "C L R Ls Rs Rls Rrs",
    "C L R Ls Rs Rls Rrs",
    "L R Ls Rs Rls Cs Cs Rrs",
    "C L R Ls Rs Rls Cs Rrs",
};

//---------------------------------------------------------------------------
const char* DTS_ExtensionAudioDescriptor[]=
{
    "Channel Extension",
    "Reserved",
    "Frequency Extension",
    "Channel Extension + Frequency Extension",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* DTS_HD_RefClockCode[]=
{
    "1/32000",
    "1/44100",
    "1/48000",
    "",
};

//---------------------------------------------------------------------------
std::string DTS_HD_SpeakerActivityMask (int16u SpeakerActivityMask)
{
    std::string Text;
    if ((SpeakerActivityMask&0x0003)==0x0003)
        Text+="Front: L C R";
    else
    {
        if (SpeakerActivityMask&0x0001)
            Text+="Front: C";
        if (SpeakerActivityMask&0x0002)
            Text+="Front: L R";
    }

    if (SpeakerActivityMask&0x0004)
        Text+=", Side: L R";

    if (SpeakerActivityMask&0x0010)
        Text+=", Back: C";

    if ((SpeakerActivityMask&0x00A0)==0x00A0)
        Text+=", High: L C R";
    else
    {
        if (SpeakerActivityMask&0x0020)
            Text+=", High: L R";
        if (SpeakerActivityMask&0x0080)
            Text+=", High: C";
    }

    if (SpeakerActivityMask&0x0800)
        Text+=", Side: L R";

    if (SpeakerActivityMask&0x0040)
        Text+=", Back: L R";

    if (SpeakerActivityMask&0x0100)
        Text+=", TopCtrSrrd";
    if (SpeakerActivityMask&0x0200)
        Text+=", Ctr: L R";
    if (SpeakerActivityMask&0x0400)
        Text+=", Wide: L R";
    if (SpeakerActivityMask&0x2000)
        Text+=", HiSide: L R";

    if ((SpeakerActivityMask&0xC000)==0x0C000)
        Text+=", HiRear: L C R";
    else
    {
        if (SpeakerActivityMask&0x4000)
            Text+=", HiRear: C";
        if (SpeakerActivityMask&0x8000)
            Text+=", HiRear: L R";
    }

    if (SpeakerActivityMask&0x0008)
        Text+=", LFE";
    if (SpeakerActivityMask&0x1000)
        Text+=", LFE2";

    return Text;
}

//---------------------------------------------------------------------------
std::string DTS_HD_SpeakerActivityMask2 (int16u SpeakerActivityMask)
{
    std::string Text;
    if ((SpeakerActivityMask&0x0003)==0x0003)
        Text+="3";
    else
    {
        if (SpeakerActivityMask&0x0001)
            Text+="1";
        else if (SpeakerActivityMask&0x0002)
            Text+="2";
        else
            Text+="0";
    }

    if (SpeakerActivityMask&0x0004)
        Text+="/2";
    else if ((SpeakerActivityMask&0x0840)==0x0000)
        Text+="/0";

    if (SpeakerActivityMask&0x0010)
        Text+="/1";
    else if ((SpeakerActivityMask&0x0840)==0x0000)
        Text+="/0";

    if ((SpeakerActivityMask&0x00A0)==0x00A0)
        Text+=".3";
    else
    {
        if (SpeakerActivityMask&0x0020)
            Text+=".2";
        if (SpeakerActivityMask&0x0080)
            Text+=".2";
    }

    if (SpeakerActivityMask&0x0800)
        Text+="/2";
    if (SpeakerActivityMask&0x0040)
        Text+="/2";

    if (SpeakerActivityMask&0x0100)
        Text+=".1";
    if (SpeakerActivityMask&0x0200)
        Text+=".2";
    if (SpeakerActivityMask&0x0400)
        Text+=".2";
    if (SpeakerActivityMask&0x2000)
        Text+=".2";

    if ((SpeakerActivityMask&0xC000)==0x0C000)
        Text+=".3";
    else
    {
        if (SpeakerActivityMask&0x4000)
            Text+=".1";
        if (SpeakerActivityMask&0x8000)
            Text+=".2";
    }

    if (SpeakerActivityMask&0x0008)
        Text+=".1";
    if (SpeakerActivityMask&0x1000)
        Text+=".1";

    return Text;
}

//---------------------------------------------------------------------------
const char* DTS_HD_TypeDescriptor[]=
{
    "Music",
    "Effects",
    "Dialog",
    "Commentary",
    "Visually Impaired",
    "Hearing  Impaired",
    "Isolated Music Object",
    "Music and Effects",
    "Dialog and Commentary",
    "Effects and Commentary",
    "Isolated Music Object and Commentary",
    "Isolated Music Object and Effects",
    "Karaoke",
    "Music, Effects, Dialog",
    "Complete Audio Presentation",
    "",
};

//---------------------------------------------------------------------------
int32u DTS_HD_MaximumSampleRate[]=
{
      8000,
     16000,
     32000,
     64000,
    128000,
     22050,
     44100,
     88200,
    176400,
    352800,
     12000,
     24000,
     48000,
     96000,
    192000,
    384000,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dts::File_Dts()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Dts");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Dts;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:2;

    //Buffer
    Save_Buffer=NULL;

    //Temp
    HD_size=0;
    Primary_Frame_Byte_Size=0;
    HD_SpeakerActivityMask=(int16u)-1;
    channel_arrangement=(int8u)-1;
    channel_arrangement_XCh=(int8u)-1;
    sample_frequency=(int8u)-1;
    sample_frequency_X96k=(int8u)-1;
    bit_rate=(int8u)-1;
    lfe_effects=(int8u)-1;
    bits_per_sample=(int8u)-1;
    ExtensionAudioDescriptor=(int8u)-1;
    HD_BitResolution=(int8u)-1;
    HD_MaximumSampleRate=(int8u)-1;
    HD_TotalNumberChannels=(int8u)-1;
    HD_ExSSFrameDurationCode=(int8u)-1;
    ExtendedCoding=false;
    ES=false;
    Core_Exists=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dts::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "DTS");

    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "DTS");

    if (!Profile.empty())
        Fill(Stream_Audio, 0, Audio_Format_Profile, Profile+(Core_Exists?__T(" / Core"):__T("")));
    else if (ES)
        Fill(Stream_Audio, 0, Audio_Format_Profile, "ES");
    Fill(Stream_General, 0, General_Format_Profile, Retrieve(Stream_Audio, 0, Audio_Format_Profile));
    Fill(Stream_Audio, 0, Audio_Codec, (Profile.find(__T("MA"))==0 || Profile.find(__T("HRA"))==0)?"DTS-HD":"DTS");

    //HD data
    if ((ExtendedCoding && (ExtensionAudioDescriptor==0 || ExtensionAudioDescriptor==3 || ExtensionAudioDescriptor==6)))
    {
        switch(channel_arrangement_XCh)
        {
            case 1 :
                    Fill(Stream_Audio, 0, Audio_Channel_s_, 7);
                    Fill(Stream_Audio, 0, Audio_ChannelPositions, Ztring("Front: L C R, Side: L R, Back: C")+(lfe_effects?__T(", LFE"):__T("")));
                    Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Ztring("3/2/1")+(lfe_effects?__T(".1"):__T(".0")));
                    break;
            case 2 :
                    Fill(Stream_Audio, 0, Audio_Channel_s_, 8);
                    Fill(Stream_Audio, 0, Audio_ChannelPositions, Ztring("Front: L C R, Side: L R, Back: L R")+(lfe_effects?__T(", LFE"):__T("")));
                    Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Ztring("3/2/2")+(lfe_effects?__T(".1"):__T(".0")));
                    break;
            default:;
        }
    }
    else
    {
        if (HD_SpeakerActivityMask!=(int16u)-1)
        {
            Fill(Stream_Audio, 0, Audio_ChannelPositions, DTS_HD_SpeakerActivityMask(HD_SpeakerActivityMask).c_str());
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, DTS_HD_SpeakerActivityMask2(HD_SpeakerActivityMask).c_str());
        }
        if(HD_BitResolution!=(int8u)-1)
            Fill(Stream_Audio, 0, Audio_BitDepth, HD_BitResolution);
        if(HD_MaximumSampleRate!=(int8u)-1)
            Fill(Stream_Audio, 0, Audio_SamplingRate, DTS_HD_MaximumSampleRate[HD_MaximumSampleRate]);
        else if (sample_frequency_X96k!=(int8u)-1)
            Fill(Stream_Audio, 0, Audio_SamplingRate, DTS_SamplingRate[sample_frequency_X96k]);
        if(HD_TotalNumberChannels!=(int8u)-1)
            Fill(Stream_Audio, 0, Audio_Channel_s_, HD_TotalNumberChannels);
        if (Profile==__T("Express"))
        {
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate_Get(true), 0);
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        }
    }

    //Core data
    if (Core_Exists)
    {
        if ((Profile.find(__T("MA"))==0?__T("VBR"):__T("CBR"))!=Retrieve(Stream_Audio, 0, Audio_BitRate_Mode))
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, Profile.find(__T("MA"))==0?"VBR":"CBR");
        if (DTS_SamplingRate[sample_frequency]!=Retrieve(Stream_Audio, 0, Audio_SamplingRate).To_int32u())
            Fill(Stream_Audio, 0, Audio_SamplingRate, DTS_SamplingRate[sample_frequency]);
        Ztring BitRate;
        if (bit_rate<29 || Profile==__T("Express"))
            BitRate=Ztring::ToZtring(BitRate_Get(), 0);
        else if (bit_rate==29)
            BitRate=__T("Open");
        else if (bit_rate==30)
            BitRate=__T("Variable");
        else if (bit_rate==31)
            BitRate=__T("LossLess");
        if (Profile==__T("HRA"))
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate_Get(true), 0);
        if ( Profile==__T("MA"))
            Fill(Stream_Audio, 0, Audio_BitRate, "Unknown");
        if (BitRate!=Retrieve(Stream_Audio, 0, Audio_BitRate))
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
        if (Profile.find(__T("MA"))==0 || bit_rate==31)
            Fill(Stream_Audio, 0, Audio_Compression_Mode, "Lossless / Lossy", Unlimited, true, true);

        int8u Channels;
        Ztring ChannelPositions, ChannelPositions2, ChannelLayout;
        if (channel_arrangement<16)
        {
            Channels=DTS_Channels[channel_arrangement]+(lfe_effects?1:0);
            ChannelPositions.From_Local(DTS_ChannelPositions[channel_arrangement]);
            ChannelPositions2.From_Local(DTS_ChannelPositions2[channel_arrangement]);
            ChannelLayout.From_Local(DTS_ChannelLayout[channel_arrangement]);
        }
        else
        {
            Channels=8;
            ChannelPositions.From_Local("User defined");
        }
        if (lfe_effects)
        {
            ChannelPositions+=__T(", LFE");
            ChannelPositions2+=__T(".1");
            ChannelLayout+=__T(" LFE");
        }
        if (Channels!=Retrieve(Stream_Audio, 0, Audio_Channel_s_).To_int8u())
            Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        if (ChannelPositions!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions))
            Fill(Stream_Audio, 0, Audio_ChannelPositions, ChannelPositions);
        if (ChannelPositions2!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions_String2))
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, ChannelPositions2);
        if (ChannelLayout!=Retrieve(Stream_Audio, 0, Audio_ChannelLayout))
            Fill(Stream_Audio, 0, Audio_ChannelLayout, ChannelLayout);
        if (DTS_Resolution[bits_per_sample]!=Retrieve(Stream_Audio, 0, Audio_BitDepth).To_int8u())
            Fill(Stream_Audio, 0, Audio_BitDepth, DTS_Resolution[bits_per_sample]);
    }
}

//---------------------------------------------------------------------------
void File_Dts::Streams_Finish()
{
    Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, BigEndian?"Big":"Little", Unlimited, true, true);
    Fill(Stream_Audio, 0, Audio_Format_Settings_Mode, Word?"16":"14", Unlimited, true, true);

    if (FrameInfo.PTS!=(int64u)-1 && FrameInfo.PTS>PTS_Begin)
    {
        Fill(Stream_Audio, 0, Audio_Duration, ((float64)(FrameInfo.PTS-PTS_Begin))/1000000, 0, true);
        Fill(Stream_Audio, 0, Audio_FrameCount, ((float64)(FrameInfo.PTS-PTS_Begin))/1000000/32, 0, true);
    }
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dts::FileHeader_Begin()
{
    //Must have enough buffer for having header
    if (Buffer_Size<4)
        return false; //Must wait for more data

    //False positives detection: Detect WAV files, the parser can't detect it easily, there is only 70 bytes of beginning for saying WAV
    switch (CC4(Buffer))
    {
        case 0x52494646 : //"RIFF"
        case 0x000001FD : //MPEG-PS private
                            Finish("DTS");
                            return false;
        default         :   ;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dts::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+6<=Buffer_Size)
    {
        if (!FrameSynchPoint_Test())
            return false; //Need more data
        if (Synched)
            break;
        Buffer_Offset++;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+6>Buffer_Size)
    {
        if (Buffer_Offset+5==Buffer_Size)
        {
            int64u Value=CC5(Buffer+Buffer_Offset);
            if ((Value&0xFFFFFFFFFCLL)!=0x7FFE8001FCLL  //16 bits and big    endian Core
             && (Value&0xFFFFFFFF00LL)!=0xFE7F018000LL  //16 bits and little endian Core
             && (Value&0xFFFFFFFFF7LL)!=0x1FFFE80007LL  //14 bits and big    endian Core
             && (Value&0xFFFFFFFFF0LL)!=0xFF1F00E8F0LL  //14 bits and little endian Core
             && (Value&0xFFFFFFFF00LL)!=0x6458202500LL) //16 bits and big    endian HD
                Buffer_Offset++;
        }
        if (Buffer_Offset+4==Buffer_Size)
        {
            int32u Value=CC4(Buffer+Buffer_Offset);
            if (Value!=0x7FFE8001  //16 bits and big    endian Core
             && Value!=0xFE7F0180  //16 bits and little endian Core
             && Value!=0x1FFFE800  //14 bits and big    endian Core
             && Value!=0xFF1F00E8  //14 bits and little endian Core
             && Value!=0x64582025) //16 bits and big    endian HD
                Buffer_Offset++;
        }
        if (Buffer_Offset+3==Buffer_Size)
        {
            int32u Value=CC3(Buffer+Buffer_Offset);
            if (Value!=0x7FFE80  //16 bits and big    endian Core
             && Value!=0xFE7F01  //16 bits and little endian Core
             && Value!=0x1FFFE8  //14 bits and big    endian Core
             && Value!=0xFF1F00  //14 bits and little endian Core
             && Value!=0x645820) //16 bits and big    endian HD
                Buffer_Offset++;
        }
        if (Buffer_Offset+2==Buffer_Size)
        {
            int16u Value=CC2(Buffer+Buffer_Offset);
            if (Value!=0x7FFE  //16 bits and big    endian Core
             && Value!=0xFE7F  //16 bits and little endian Core
             && Value!=0x1FFF  //14 bits and big    endian Core
             && Value!=0xFF1F  //14 bits and little endian Core
             && Value!=0x6458) //16 bits and big    endian HD
                Buffer_Offset++;
        }
        if (Buffer_Offset+1==Buffer_Size)
        {
            int8u Value=CC1(Buffer+Buffer_Offset);
            if (Value!=0x7F  //16 bits and big    endian Core
             && Value!=0xFE  //16 bits and little endian Core
             && Value!=0x1F  //14 bits and big    endian Core
             && Value!=0xFF  //14 bits and little endian Core
             && Value!=0x64) //16 bits and big    endian HD
                Buffer_Offset++;
        }
        return false;
    }

    //Synched
    return true;
}

//---------------------------------------------------------------------------
bool File_Dts::Synched_Test()
{
    //Padding
    while (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]==0x00)
        Buffer_Offset++;

    //Must have enough buffer for having header
    if (Buffer_Offset+6>Buffer_Size)
        return false;

    //Quick test of synchro
    if (!FrameSynchPoint_Test())
        return false; //Need more data
    if (!Synched)
        return true;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Dts::Read_Buffer_Unsynched()
{
    FrameInfo=frame_info();
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Dts::Demux_UnpacketizeContainer_Test()
{
    int32u Value=CC4(Buffer+Buffer_Offset);
    if (Value==0x7FFE8001)
    {
        int16u Size=((BigEndian2int24u(Buffer+Buffer_Offset+5)>>4)&0x3FFF)+1;
        Demux_Offset=Buffer_Offset+Size;

        if (Buffer_Offset+Size>Buffer_Size && File_Offset+Buffer_Size!=File_Size)
            return false; //No complete frame

        Demux_UnpacketizeContainer_Demux();
    }

    return true;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dts::Header_Parse()
{
    Header_Fill_Size(Original_Size);
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dts::Data_Parse()
{
    if (Save_Buffer)
    {
        File_Offset+=Buffer_Offset;
        swap(Buffer, Save_Buffer);
        swap(Buffer_Offset, Save_Buffer_Offset);
        swap(Buffer_Size, Save_Buffer_Size);
        if (!Word)
            Element_Size=Element_Size*14/16;
    }

    //Parsing
    int32u Sync;
    Get_B4 (Sync,                                               "Sync");
    if (Sync==0x64582025)
    {
        //HD
        Element_Name("HD");
        Element_Code=1;

        int16u header_size;
        int8u  SubStreamIndex, NumAssets, NumAudioPresent;
        bool isBlownUpHeader, StaticFieldsPresent;
        Skip_B1(                                                "Unknown");
        BS_Begin();
        Get_S1 (2, SubStreamIndex,                              "Substream index");
        Get_SB (isBlownUpHeader,                                "Is blown up header");
        if (isBlownUpHeader)
        {
            Get_S2 (12, header_size,                            "Header size"); header_size++; Param_Info2(header_size, " bytes");
            Get_S3 (20, HD_size,                                "HD block size"); HD_size++; Param_Info2(HD_size, " bytes");
        }
        else
        {
            Get_S2 ( 8, header_size,                            "Header size"); header_size++; Param_Info2(header_size, " bytes");
            Get_S3 (16, HD_size,                                "HD block size"); HD_size++; Param_Info2(HD_size, " bytes");
        }
        TESTELSE_SB_GET(StaticFieldsPresent,                    "Static fields present");
            std::vector<int32u> ActiveExSSMasks;
            Info_S1(2, RefClockCode,                            "Reference clock code"); Param_Info1(DTS_HD_RefClockCode[RefClockCode]);
            Get_S1 (3, HD_ExSSFrameDurationCode,                "ExSS frame duration code"); HD_ExSSFrameDurationCode++; Param_Info1(HD_ExSSFrameDurationCode);
            TEST_SB_SKIP(                                       "Timestamp flag");
                Skip_S5(36,                                     "Timestamp");
            TEST_SB_END();
            Get_S1 (3, NumAudioPresent,                         "Num audio present"); NumAudioPresent++; Param_Info2(NumAudioPresent, " channels");
            Get_S1 (3, NumAssets,                               "Num assets"); NumAssets++; Param_Info2(NumAssets, " assets");
            Element_Begin1("Active masks");
            for (int8u Pos=0; Pos<NumAudioPresent; Pos++)
            {
                int32u ActiveExSSMask;
                Get_S4 (SubStreamIndex+1, ActiveExSSMask,       "Active ExSS mask");
                ActiveExSSMasks.push_back(ActiveExSSMask);
            }
            Element_End0();
            Element_Begin1("Active masks 2");
            for (int8u Pos=0; Pos<NumAudioPresent; Pos++)
                for (int8u Pos2=0; Pos2<SubStreamIndex+1; Pos2+=2)
                    if (ActiveExSSMasks[Pos]%2)
                        Skip_S1(8,                              "Active ExSS Mask");
            Element_End0();
            TEST_SB_SKIP(                                       "Mix metadata Enabled");
                int8u Bits4MixOutMask, NumMixOutConfigs;
                Skip_S1(2,                                      "Mix metadata adjustment level");
                Get_S1 (2, Bits4MixOutMask,                     "Bits4Mix out mask"); Bits4MixOutMask=4+Bits4MixOutMask*4; Param_Info2(Bits4MixOutMask, " bits");
                Get_S1 (2, NumMixOutConfigs,                    "Number of mix out configs"); NumMixOutConfigs++; Param_Info2(NumMixOutConfigs, " configs");
                for (int8u Pos=0; Pos<NumMixOutConfigs; Pos++)
                    Skip_S1(Bits4MixOutMask,                    "MixOutChMask");
            TEST_SB_END();
        TESTELSE_SB_ELSE("Static fields present");
            NumAudioPresent=1;
            NumAssets=1;
        TESTELSE_SB_END();
        Asset_Sizes.clear();
        Element_Begin1("Sizes");
        for (int8u Pos=0; Pos<NumAssets; Pos++)
        {
            int32u Size;
            if (isBlownUpHeader)
            {
                Get_S3 (20, Size,                               "Size"); Size++; Param_Info2(Size, " bytes");
            }
            else
            {
                Get_S3 (16, Size,                               "Size"); Size++; Param_Info2(Size, " bytes");
            }
            Asset_Sizes.push_back(Size);
        }
        Element_End0();
        for (int8u Asset_Pos=0; Asset_Pos<NumAssets; Asset_Pos++)
        {
            Element_Begin1("Asset");
            int16u AssetSize;
            Get_S2 (9, AssetSize,                               "Asset size");
            AssetSize++;  Param_Info2(AssetSize, " bytes?");
            Skip_S1(3,                                          "Descriptor data for asset index");
            if (StaticFieldsPresent)
            {
                TEST_SB_SKIP(                                   "Asset type descriptor present");
                    Info_S1( 4, TypeDescriptor,                 "Asset type descriptor"); Param_Info1(DTS_HD_TypeDescriptor[TypeDescriptor]);
                TEST_SB_END();
                TEST_SB_SKIP(                                   "Language descriptor present");
                    Info_S3(24, LanguageDescriptor,             "Language descriptor"); Param_Info1(Ztring().From_CC3(LanguageDescriptor));
                TEST_SB_END();
                TEST_SB_SKIP(                                   "Info text present");
                    int16u InfoTextByteSize;
                    Get_S2(10, InfoTextByteSize,                "Info text size"); InfoTextByteSize++; Param_Info2(InfoTextByteSize, "bytes");
                    for (int16u Pos=0; Pos<InfoTextByteSize; Pos++)
                        Skip_S1(8,                              "Info text");
                TEST_SB_END();
                Get_S1 (5, HD_BitResolution,                    "Bit resolution"); HD_BitResolution++; Param_Info2(HD_BitResolution, " bits");
                Get_S1 (4, HD_MaximumSampleRate,                "Maximum sample rate"); Param_Info2(DTS_HD_MaximumSampleRate[HD_MaximumSampleRate], " Hz");
                Get_S1 (8, HD_TotalNumberChannels,              "Total number of channels"); HD_TotalNumberChannels++; Param_Info2(HD_TotalNumberChannels, " channels");
                TEST_SB_SKIP(                                   "1 to 1 map channels to speakers");
                    int8u SpeakerActivityMaskBits, SpeakerRemapSetsCount;
                    if (HD_TotalNumberChannels>2)
                        Skip_SB(                                "Embedded stereo flag"); //else is 0
                    if (HD_TotalNumberChannels>6)
                        Skip_SB(                                "Embedded 6 Channels flag"); //else is 0
                    TESTELSE_SB_SKIP(                           "Speaker mask enabled");
                        Get_S1 (2, SpeakerActivityMaskBits,     "Speaker activity mask bits"); Param_Info2(4+SpeakerActivityMaskBits*4, " bits");
                        SpeakerActivityMaskBits=4+SpeakerActivityMaskBits*4;
                        Get_S2 (SpeakerActivityMaskBits, HD_SpeakerActivityMask, "Speaker activity mask"); Param_Info1(DTS_HD_SpeakerActivityMask(HD_SpeakerActivityMask).c_str());
                    TESTELSE_SB_ELSE(                           "Speaker mask enabled");
                        SpeakerActivityMaskBits=0;
                    TESTELSE_SB_END();
                    Get_S1 (3, SpeakerRemapSetsCount,           "Speaker remap sets count");
                    for (int8u Pos=0; Pos<SpeakerRemapSetsCount; Pos++)
                        Skip_S2(SpeakerActivityMaskBits,        "Standard speaker layout mask");
                    for (int8u Pos=0; Pos<SpeakerRemapSetsCount; Pos++)
                    {
                        int8u NumDecCh4Remap;
                        Get_S1(5, NumDecCh4Remap,               "NumDecCh4Remap");
                        //Not finnished!
                    }
                TEST_SB_END();
                Element_End0();
            }
        }
        BS_End();
        Skip_XX(header_size-Element_Offset,                   "Unknown");
    }
    else
    {
        //Frame
        Element_Name("Frame");
        Element_Code=0;

        int8u  EncoderSoftwareRevision;
        bool   crc_present;
        BS_Begin();
        Info_SB(    FrameType,                                      "Frame Type"); Param_Info1(DTS_FrameType[FrameType]);
        Skip_S1( 5,                                                 "Deficit Sample Count");
        Get_SB (    crc_present,                                    "CRC Present");
        Get_S2 ( 7, Number_Of_PCM_Sample_Blocks,                    "Number of PCM Sample Blocks");
        Number_Of_PCM_Sample_Blocks++;
        Get_S2 (14, Primary_Frame_Byte_Size,                        "Primary Frame Byte Size minus 1");
        Primary_Frame_Byte_Size++;
        if (!Word)
            Primary_Frame_Byte_Size=Primary_Frame_Byte_Size*16/14; //Word is on 14 bits
        Param_Info2(Primary_Frame_Byte_Size, " bytes");
        Get_S1 ( 6, channel_arrangement,                            "Audio Channel Arrangement"); Param_Info2(DTS_Channels[channel_arrangement], " channels");
        Get_S1 ( 4, sample_frequency,                               "Core Audio Sampling Frequency"); Param_Info2(DTS_SamplingRate[sample_frequency], " Hz");
        Get_S1 ( 5, bit_rate,                                       "Transmission Bit Rate"); Param_Info2(DTS_BitRate[bit_rate], " bps");
        Skip_SB(                                                    "Embedded Down Mix Enabled");
        Skip_SB(                                                    "Embedded Dynamic Range");
        Skip_SB(                                                    "Embedded Time Stamp");
        Skip_SB(                                                    "Auxiliary Data");
        Skip_SB(                                                    "HDCD");
        Get_S1 ( 3, ExtensionAudioDescriptor,                       "Extension Audio Descriptor"); Param_Info1(DTS_ExtensionAudioDescriptor[ExtensionAudioDescriptor]);
        Get_SB (    ExtendedCoding,                                 "Extended Coding");
        Skip_SB(                                                    "Audio Sync Word Insertion");
        Get_S1 ( 2, lfe_effects,                                    "Low Frequency Effects");
        Skip_SB(                                                    "Predictor History");
        if (crc_present)
            Skip_S2(16,                                             "Header CRC Check");
        Skip_SB(                                                    "Multirate Interpolator");
        Get_S1 ( 4, EncoderSoftwareRevision,                        "Encoder Software Revision");
        Skip_S1( 2,                                                 "Copy History");
        Get_S1 ( 2, bits_per_sample,                                "Source PCM Resolution"); Param_Info1(Ztring::ToZtring(DTS_Resolution[bits_per_sample])+__T(" bits"));
        Get_SB (    ES,                                             "ES");
        Skip_SB(                                                    "Front Sum/Difference");
        Skip_SB(                                                    "Surrounds Sum/Difference");
        Skip_S1( 4,                                                 "Dialog Normalisation Parameter");
        switch (EncoderSoftwareRevision)
        {
            case 6 :
                    Skip_S1( 4,                                     "Dialog Normalisation Parameter");
                    break;
            case 7 :
                    Skip_S1( 4,                                     "Dialog Normalisation Parameter");
                    break;
            default :
                    Skip_S1( 4,                                     "Unspecified");
                    break;
        }
        BS_End();
    }

    //Name
    Element_Info1(Ztring::ToZtring(Frame_Count));

    //PTS
    Element_Info1C((FrameInfo.PTS!=(int64u)-1), __T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000)));

    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
    if (Element_Code==0 || !Core_Exists)
    {
        if (Frame_Count==0)
            PTS_Begin=FrameInfo.PTS;
        Frame_Count++;
        Frame_Count_InThisBlock++;
        float64 BitRate=BitRate_Get();
        if (BitRate)
        {
            FrameInfo.DUR=float64_int64s(((float64)(Element_Size+Header_Size))*8/BitRate*1000000000);
            if (FrameInfo.DTS!=(int64u)-1)
                FrameInfo.DTS+=FrameInfo.DUR;
            if (FrameInfo.PTS!=(int64u)-1)
                FrameInfo.PTS=FrameInfo.DTS;
        }
    }

    //If filled
    if (Status[IsFilled])
    {
        //Little Endian and 14-bit streams management
        if (Save_Buffer)
        {
            delete[] Buffer;
            Buffer=Save_Buffer; Save_Buffer=NULL;
            Buffer_Offset=Save_Buffer_Offset;
            Buffer_Size=Save_Buffer_Size;
            File_Offset-=Buffer_Offset;
            if (!Word)
                Element_Size=Element_Size*16/14;
        }

        Skip_XX(Element_Size-Element_Offset,                    "Data");
        return;
    }

    //Parsing
    switch(Element_Code)
    {
        case 0  : Core(); break;
        case 1  : HD(); break;
        default : Skip_XX(Element_Size,                         "Data");
    }

    //Little Endian and 14-bit streams management
    if (Save_Buffer)
    {
        delete[] Buffer;
        Buffer=Save_Buffer; Save_Buffer=NULL;
        Buffer_Offset=Save_Buffer_Offset;
        Buffer_Size=Save_Buffer_Size;
        File_Offset-=Buffer_Offset;
        if (!Word)
            Element_Size=Element_Size*16/14;
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dts::Core()
{
    Element_Name("Frame");

    //It exists (not in XSA streams)
    Core_Exists=true;

    //Looking for extensions
    int64u Core_Size=Element_Size, XCh_Sync=Element_Size, XXCh_Sync=Element_Size, X96k_Sync=Element_Size;
    if (ExtendedCoding)
    {
        //XCh
        if (ExtensionAudioDescriptor==0 || ExtensionAudioDescriptor==3)
            for (size_t Pos=0; Pos+4<=Element_Size; Pos++)
                if (CC4(Buffer+Buffer_Offset+Pos)==0x5A5A5A5A)
                    XCh_Sync=Pos;

        //XXCh
        if (ExtensionAudioDescriptor==6)
            for (size_t Pos=0; Pos+4<=Element_Size; Pos++)
                if (CC4(Buffer+Buffer_Offset+Pos)==0x47004A03)
                    XXCh_Sync=Pos;

        //X96k
        if (ExtensionAudioDescriptor==2 || ExtensionAudioDescriptor==3)
            for (size_t Pos=0; Pos+4<=Element_Size; Pos++)
                if (CC4(Buffer+Buffer_Offset+Pos)==0x1D95F262)
                    X96k_Sync=Pos;

        //Finding core size
        if (XCh_Sync<Core_Size)
            Core_Size=XCh_Sync;
        if (XXCh_Sync<Core_Size)
            Core_Size=XXCh_Sync;
        if (X96k_Sync<Core_Size)
            Core_Size=X96k_Sync;
    }

    //Parsing
    Skip_XX(Core_Size-Element_Offset,                           "Core data");
    if (ExtendedCoding && (ExtensionAudioDescriptor==2 || ExtensionAudioDescriptor==3))
    {
        Element_Begin0();
        Skip_B4(                                                "Magic");
        Core_X96k(XCh_Sync-Element_Offset);
        Element_End0();
    }
    if (ExtendedCoding && (ExtensionAudioDescriptor==0 || ExtensionAudioDescriptor==3))
    {
        Element_Begin0();
        Skip_B4(                                                "Magic");
        Core_XCh(Element_Size-Element_Offset);
        Element_End0();
    }
    if (ExtendedCoding && ExtensionAudioDescriptor==6)
    {
        Element_Begin0();
        Skip_B4(                                                "Magic");
        Core_XXCh(Element_Size-Element_Offset);
        Element_End0();
    }

    //Filling
    FILLING_BEGIN();
        if (Count_Get(Stream_Audio)==0 && Frame_Count>=Frame_Count_Valid)
        {
            Accept("DTS");
            Fill("DTS");

            //No more need data
            if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1)
                Finish("DTS");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::Core_XCh(int64u Size)
{
    //Parsing
    Element_Name("XCh (6.1 channels)");
    int16u XChFSIZE;
    int8u  AMODE;
    BS_Begin();
    Get_S2 (10, XChFSIZE,                                       "Primary Frame Byte Size");
    Get_S1 ( 4, AMODE,                                          "Extension Channel Arrangement");
    BS_End();
    if (XChFSIZE==Element_Size-(Element_Offset-6))
        XChFSIZE--; //Compatibility reason (from specs)
    if ((int64u)XChFSIZE+1-6<=Size-2)
        Skip_XX(XChFSIZE+1-6,                                   "XCh data");
    else
        Skip_XX(Size-2,                                         "XCh data (with problem)");

    FILLING_BEGIN();
        channel_arrangement_XCh=AMODE;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::Core_XXCh(int64u Size)
{
    Element_Name("XXCh (6.1 or 7.1 channels)");
    int8u ChannelsAdded;
    BS_Begin();
    Skip_S1 (8,                                                 "?");
    Get_S1  (2, ChannelsAdded,                                  "Channels added?");
    Skip_S1 (6,                                                 "?");
    BS_End();
    Skip_XX(Size-2,                                             "Data");

    FILLING_BEGIN();
        channel_arrangement_XCh=ChannelsAdded;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::Core_X96k(int64u Size)
{
    //Parsing
    Element_Name("X96k (96 KHz)");
    int16u FSIZE96;
    int8u  REVNO;
    BS_Begin();
    Get_S2 (12, FSIZE96,                                        "96 kHz Extension Frame Byte Data Size");
    Get_S1 ( 4, REVNO,                                          "Revision Number");
    BS_End();
    Skip_XX(Size-2,                                             "X96k data"); //FSIZE96 is until end, not X96k size

    FILLING_BEGIN();
        sample_frequency_X96k=14; //96KHz
        Profile="96/24";
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD()
{
    //Parsing
    while (Element_Offset<Element_Size)

    {
        //Looking for size
        int64u Next=Element_Offset+4;
        while (Next+4<=Element_Size)
        {
            int32u CC=CC4(Buffer+Buffer_Offset+(size_t)Next);
            if (CC==0x0A801921
             || CC==0x1D95F262
             || CC==0x41A29547
             || CC==0x47004A03
             || CC==0x5A5A5A5A
             || CC==0x655E315E)
                break;
            Next++;
        }
        if (Next+4>Element_Size)
            Next=Element_Size;

        Element_Begin0();
        int32u Magic;
        Get_B4 (Magic,                                          "Magic");
        switch (Magic)
        {
            case 0x0A801921 : HD_XSA(Next-Element_Offset);  break;
            case 0x1D95F262 : HD_X96k(Next-Element_Offset); break;
            case 0x41A29547 : HD_XLL(Next-Element_Offset);  break;
            case 0x47004A03 : HD_XXCh(Next-Element_Offset); break;
            case 0x5A5A5A5A : HD_XCh(Next-Element_Offset);  break;
            case 0x655E315E : HD_XBR(Next-Element_Offset);  break;
            default :
                        //Magic value is unknown
                        if (Profile.empty())
                            Profile="HD";
                        Skip_XX(Next-Element_Offset,            "Data");
        }
        Element_End0();
    }

    //Filling
    FILLING_BEGIN();
        if (Count_Get(Stream_Audio)==0 && Frame_Count>=Frame_Count_Valid)
        {
            Accept("DTS");
            Fill("DTS");

            //No more need data
            if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1)
                Finish("DTS");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD_XCh(int64u Size)
{
    //Parsing
    Element_Name("XCh (6.1 channels)");
    Skip_XX(Size,                                               "Data");

    FILLING_BEGIN();
        channel_arrangement_XCh=1;
        if (Profile.empty())
            Profile="HRA";
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD_XXCh(int64u Size)
{
    Element_Name("XXCh (6.1 or 7.1 channels)");
    int8u ChannelsAdded;
    BS_Begin();
    Skip_S1 (8,                                                 "?");
    Get_S1  (2, ChannelsAdded,                                  "Channels added?");
    Skip_S1 (6,                                                 "?");
    BS_End();
    Skip_XX(Size-2,                                             "Data");

    FILLING_BEGIN();
        channel_arrangement_XCh=ChannelsAdded;
        if (Profile.empty())
            Profile="HRA";
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD_X96k(int64u Size)
{
    //Parsing
    Element_Name("X96k (96 KHz)");
    Skip_XX(Size,                                               "Data");

    FILLING_BEGIN();
        sample_frequency=14; //96KHz
        Profile="HRA";
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD_XLL(int64u Size)
{
    Element_Name("XLL (LossLess)");
    Skip_XX(Size,                                               "Data");

    FILLING_BEGIN();
        Profile="MA";
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD_XBR(int64u Size)
{
    Element_Name("XBR (BitRate extension)");
    Skip_XX(Size,                                               "Data");

    FILLING_BEGIN();
        Profile="HRA";
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dts::HD_XSA(int64u Size)
{
    Element_Name("XSA (low bitrate)");
    Skip_XX(Size,                                               "Data");

    FILLING_BEGIN();
        Profile="Express";
    FILLING_END();
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
float64 File_Dts::BitRate_Get(bool WithHD)
{
    if (bit_rate<29 || Profile==__T("Express"))
    {
        float64 BitRate;
        if (Profile==__T("Express"))
            BitRate=0; //No core bitrate
        else if (DTS_SamplingRate[sample_frequency])
            BitRate=((float64)Primary_Frame_Byte_Size)*8/(Number_Of_PCM_Sample_Blocks*32)*DTS_SamplingRate[sample_frequency]; //(float64)DTS_BitRate[bit_rate];
        else
            BitRate=0; //Problem
        if (WithHD && HD_ExSSFrameDurationCode!=(int8u)-1)
        {
            int32u SamplePerFrames=HD_ExSSFrameDurationCode;
            switch (HD_MaximumSampleRate)
            {
                case  0 : //  8000
                case 10 : // 12000
                                SamplePerFrames*= 128; break;
                case  1 : // 16000
                case  5 : // 22050
                case 11 : // 24000
                                SamplePerFrames*= 256; break;
                case  2 : // 32000
                case  6 : // 44100
                case 12 : // 48000
                                SamplePerFrames*= 512; break;
                case  3 : // 64000
                case  7 : // 88200
                case 13 : // 96000
                                SamplePerFrames*=1024; break;
                case  4 : //128000
                case  8 : //176400
                case 14 : //192000
                                SamplePerFrames*=2048; break;
                case  9 : //352800
                case 15 : //384000
                                SamplePerFrames*=4096; break;
                default     :   SamplePerFrames=    0; break; //Can never happen (4 bits)
            }
            if (SamplePerFrames)
                BitRate+=((float64)HD_size)*8*DTS_HD_MaximumSampleRate[HD_MaximumSampleRate]/SamplePerFrames;
        }
        //if (Primary_Frame_Byte_Size && Profile==__T("HRA"))
        //    BitRate*=1+((float64)HD_size)/Primary_Frame_Byte_Size; //HD block are not in the nominal bitrate
        return BitRate;
    }
    else
        return 0;
}

//---------------------------------------------------------------------------
bool File_Dts::FrameSynchPoint_Test()
{
    if (Save_Buffer)
        return true; //Test already made by Synchronize()

    int64u Value=CC6(Buffer+Buffer_Offset);
    if ((Value&0xFFFFFFFFFC00LL)==0x7FFE8001FC00LL  //16 bits and big    endian Core
     || (Value&0xFFFFFFFF00FCLL)==0xFE7F018000FCLL  //16 bits and little endian Core
     || (Value&0xFFFFFFFFF7F0LL)==0x1FFFE80007F0LL  //14 bits and big    endian Core
     || (Value&0xFFFFFFFFF0F7LL)==0xFF1F00E8F007LL  //14 bits and little endian Core
     || (Value&0xFFFFFFFF0000LL)==0x645820250000LL) //16 bits and big    endian HD
    {
        //Configuration - 14 bits or Little Endian
        switch (Buffer[Buffer_Offset])
        {
            default   : Word=true;  BigEndian=true;  break; //16 bits and big    endian bitstream
            case 0xFE : Word=true;  BigEndian=false; break; //16 bits and little endian bitstream
            case 0x1F : Word=false; BigEndian=true;  break; //14 bits and big    endian bitstream
            case 0xFF : Word=false; BigEndian=false; break; //14 bits and little endian bitstream
        }

        int32u Size;
        if (Buffer[Buffer_Offset]==0x64) //HD
        {
            if (Buffer_Offset+10>Buffer_Size)
                return false; //Need more data

            bool isBlownUpHeader=(Buffer[Buffer_Offset+5]&0x20)?true:false;
            if (isBlownUpHeader)
                Size=((Buffer[Buffer_Offset+6]&0x01)<<19)
                   | ( Buffer[Buffer_Offset+7]      <<11)
                   | ( Buffer[Buffer_Offset+8]      << 3)
                   | ( Buffer[Buffer_Offset+9]      >> 5);
            else
                Size=((Buffer[Buffer_Offset+6]&0x1F)<<13)
                   | ( Buffer[Buffer_Offset+7]      << 3)
                   | ( Buffer[Buffer_Offset+8]      >> 5);
            Size++;
            Original_Size=Size;
        }
        else
        {
            if (Word)
            {
                if (BigEndian)
                    Size=((Buffer[Buffer_Offset+5]&0x03)<<12)
                       | ( Buffer[Buffer_Offset+6]      << 4)
                       | ( Buffer[Buffer_Offset+7]      >> 4);
                else
                    Size=((Buffer[Buffer_Offset+4]&0x03)<<12)
                       | ( Buffer[Buffer_Offset+7]      << 4)
                       | ( Buffer[Buffer_Offset+6]      >> 4);
                Size++;
                Original_Size=Size;
            }
            else
            {
                if (BigEndian)
                    Size=((Buffer[Buffer_Offset+6]&0x03)<<12)
                       | ( Buffer[Buffer_Offset+7]      << 4)
                       | ((Buffer[Buffer_Offset+8]&0x3C)>> 2);
                else
                    Size=((Buffer[Buffer_Offset+7]&0x03)<<12)
                       | ( Buffer[Buffer_Offset+6]      << 4)
                       | ((Buffer[Buffer_Offset+9]&0x3C)>> 2);
                Size++;
                Original_Size=Size*16/14;
            }
        }
        if (Buffer_Offset+Size>Buffer_Size)
            return false; //Need more data

        if (!Word || !BigEndian)
        {
            Save_Buffer=Buffer;
            Save_Buffer_Offset=Buffer_Offset;
            Save_Buffer_Size=Buffer_Size;

            //Preparing new buffer
            if (Word)
            {
                int8u* Buffer_Little=new int8u[Size];
                for (size_t Pos=0; Pos+1<Size; Pos+=2)
                {
                    Buffer_Little[Pos+1]=Save_Buffer[Buffer_Offset+Pos  ];
                    Buffer_Little[Pos  ]=Save_Buffer[Buffer_Offset+Pos+1];
                }
                Save_Buffer=Buffer_Little;
                Save_Buffer_Offset=0;
                Save_Buffer_Size=Size;
            }
            else
            {
                int8u* Dest=new int8u[Size];
                for (size_t Pos=0; Pos+8<=Original_Size; Pos+=8)
                {
                    int64u Value;
                    if (BigEndian)
                        Value =(((int64u)(   BigEndian2int16u(Buffer+Pos  )&0x3FFF))<<42)
                             | (((int64u)(   BigEndian2int16u(Buffer+Pos+2)&0x3FFF))<<28)
                             | (((int64u)(   BigEndian2int16u(Buffer+Pos+4)&0x3FFF))<<14)
                             | (((int64u)(   BigEndian2int16u(Buffer+Pos+6)&0x3FFF))    );
                    else
                        Value =(((int64u)(LittleEndian2int16u(Buffer+Pos  )&0x3FFF))<<42)
                             | (((int64u)(LittleEndian2int16u(Buffer+Pos+2)&0x3FFF))<<28)
                             | (((int64u)(LittleEndian2int16u(Buffer+Pos+4)&0x3FFF))<<14)
                             | (((int64u)(LittleEndian2int16u(Buffer+Pos+6)&0x3FFF))    );
                    int56u2BigEndian(Dest+Pos*14/16, Value);
                }
                Save_Buffer=Dest;
                Save_Buffer_Offset=0;
                Save_Buffer_Size=Size;
            }
        }

        Synched=true;
    }
    else
        Synched=false;

    return true;
}

//---------------------------------------------------------------------------
} //NameSpace

#endif //MEDIAINFO_DTS_YES

