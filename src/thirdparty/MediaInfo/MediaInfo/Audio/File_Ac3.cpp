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

//***************************************************************************
// Infos (Common)
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AC3_YES) || defined(MEDIAINFO_DVDV_YES) || defined(MEDIAINFO_MPEGPS_YES) || defined(MEDIAINFO_MPEGTS_YES)
//---------------------------------------------------------------------------

#include "ZenLib/Conf.h"
using namespace ZenLib;

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern const int32u AC3_SamplingRate[]=
{ 48000,  44100,  32000,      0,};

//---------------------------------------------------------------------------
const char*  AC3_Mode[]=
{
    "CM (complete main)",
    "ME (music and effects)",
    "VI (visually impaired)",
    "HI (hearing impaired)",
    "D (dialogue)",
    "C (commentary)",
    "E (emergency)",
    "VO (voice over)",
};

//---------------------------------------------------------------------------
const char*  AC3_Surround[]=
{
    "",
    "Not Dolby Surround encoded",
    "Dolby Surround encoded",
    "",
};

//---------------------------------------------------------------------------
extern const int16u AC3_BitRate[]=
{

     32,
     40,
     48,
     56,
     64,
     80,
     96,
    112,
    128,
    160,
    192,
    224,
    256,
    320,
    384,
    448,
    512,
    576,
    640,
};

//---------------------------------------------------------------------------
extern const int8u AC3_Channels[]=
{2, 1, 2, 3, 3, 4, 4, 5};

//---------------------------------------------------------------------------
} //NameSpace

//---------------------------------------------------------------------------
#endif //...
//---------------------------------------------------------------------------

//***************************************************************************
//
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AC3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Ac3.h"
#include <vector>
#include <cmath>
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
#include "MediaInfo/MediaInfo_Internal.h"
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const int32u AC3_SamplingRate2[]=
{ 24000,  22050,  16000,      0,};

//---------------------------------------------------------------------------
const char*  AC3_ChannelPositions[]=
{
    "Dual mono",
    "Front: C",
    "Front: L R",
    "Front: L C R",
    "Front: L R,   Back: C",
    "Front: L C R, Back: C",
    "Front: L R,   Side: L R",
    "Front: L C R, Side: L R",
};

//---------------------------------------------------------------------------
const char*  AC3_ChannelPositions2[]=
{
    "1+1",
    "1/0/0",
    "2/0/0",
    "3/0/0",
    "2/1/0",
    "3/1/0",
    "2/2/0",
    "3/2/0",
};

//---------------------------------------------------------------------------
const char*  AC3_ChannelLayout_lfeoff[]=
{
    "1+1",
    "C",
    "L R",
    "L C R",
    "L R S",
    "L C R Cs",
    "L R Ls Rs",
    "L C R Ls Rs",
};

//---------------------------------------------------------------------------
const char*  AC3_ChannelLayout_lfeon[]=
{
    "1+1 LFE",
    "C LFE",
    "L R LFE",
    "L C R LFE",
    "L R S LFE",
    "L C R LFE Cs",
    "L R LFE Ls Rs",
    "L C R LFE Ls Rs",
};

//---------------------------------------------------------------------------
int16u AC3_acmod2chanmap[]=
{
    0xA000,
    0x4000,
    0xA000,
    0xE000,
    0xA100,
    0xE100,
    0xB900,
    0xF800,
};

//---------------------------------------------------------------------------
Ztring AC3_chanmap_ChannelPositions (int16u chanmap)
{
    Ztring Front;
    Ztring Side;
    Ztring Back;
    Ztring More;

    for (int8u Pos=0; Pos<16; Pos++)
    {
        if (chanmap&(1<<(15-Pos)))
        {
            switch (Pos)
            {
                case  0 :   Front+=__T(" L"); break;
                case  1 :   Front+=__T(" C"); break;
                case  2 :   Front+=__T(" R"); break;
                case  3 :   Side+=__T(" L"); break;
                case  4 :   Side+=__T(" R"); break;
                case  5 :   {
                            bool HasR=false;
                            if (Front.find(__T(" R"))!=string::npos)
                            {
                                Front.resize(Front.size()-2);
                                HasR=true;
                            }
                            Front+=__T(" C C");
                            if (HasR)
                                Front+=__T(" R");
                            }
                            break;
                case  6 :   Back+=__T(" L R"); break;
                case  7 :   if (Back.empty())
                                Back=__T(" C");
                            else
                                Back=__T(" L C R");
                            break;
                case 15 :   More+=__T(", LFE");
                            break;
                default: ;
            }
        }
    }

    Ztring ToReturn;
    if (!Front.empty())
    {
        ToReturn+=__T("Front:")+Front;
    }
    if (!Side.empty())
    {
        if (!ToReturn.empty())
            ToReturn+=__T(", ");
        ToReturn+=__T("Side:")+Side;
    }
    if (!Back.empty())
    {
        if (!ToReturn.empty())
            ToReturn+=__T(", ");
        ToReturn+=__T("Back:")+Back;
    }
    ToReturn+=More;

    return ToReturn;
}

//---------------------------------------------------------------------------
int8u AC3_chanmap_Channels (int16u chanmap)
{
    int8u Channels=0;

    for (int8u Pos=0; Pos<16; Pos++)
    {
        if (chanmap&(1<<(15-Pos)))
        {
            switch (Pos)
            {
                case  5 :
                case  6 :
                case  9 :
                case 10 :
                case 11 :
                            Channels+=2; break;
                default:
                            Channels++; break;
            }
        }
    }

    return Channels;
}

//---------------------------------------------------------------------------
Ztring AC3_chanmap_ChannelLayout (int16u chanmap, const Ztring &ChannelLayout0)
{
    Ztring ToReturn=ChannelLayout0;

    for (int8u Pos=0; Pos<16; Pos++)
    {
        if (chanmap&(1<<(15-Pos)))
        {
            switch (Pos)
            {
                case  5 :   ToReturn+=__T(" Lc Rc"); break;
                case  6 :   ToReturn+=__T(" Lrs Rrs"); break;
                case  7 :   ToReturn+=__T(" Cs");
                default: ;
            }
        }
    }

    return ToReturn;
}

//---------------------------------------------------------------------------
const int16u AC3_FrameSize[27][4]=
{
    { 128,  138,  192,    0},
    { 160,  174,  240,    0},
    { 192,  208,  288,    0},
    { 224,  242,  336,    0},
    { 256,  278,  384,    0},
    { 320,  348,  480,    0},
    { 384,  416,  576,    0},
    { 448,  486,  672,    0},
    { 512,  556,  768,    0},
    { 640,  696,  960,    0},
    { 768,  834, 1152,    0},
    { 896,  974, 1344,    0},
    {1024, 1114, 1536,    0},
    {1280, 1392, 1920,    0},
    {1536, 1670, 2304,    0},
    {1792, 1950, 2688,    0},
    {2048, 2228, 3072,    0},
    {2304, 2506, 3456,    0},
    {2560, 2786, 3840,    0},
    {   0,    0,    0,    0},
    {   0,    0,    0,    0},
    {   0,    0,    0,    0},
    {   0,    0,    0,    0},
    {   0,    0,    0,    0},
    {   0,    0,    0,    0},
    {   0,    0,    0,    0},
    { 768,    0,    0,    0},
};

//---------------------------------------------------------------------------
int16u AC3_FrameSize_Get(int8u frmsizecod, int8u fscod)
{
    bool Padding=(frmsizecod%2)?true:false;
    int16u frame_size_id=frmsizecod/2;

    if (frame_size_id>26 || fscod>3)
        return 0;

    int16u FrameSize=AC3_FrameSize[frame_size_id][fscod];
    if (fscod==1 && Padding)
        FrameSize+=2; // frame lengths are padded by 1 word (16 bits) at 44100 Hz
    return FrameSize;
}

//---------------------------------------------------------------------------
// CRC_16_Table
// A CRC is computed like this:
// Init: int32u CRC_16 = 0x0000;
// for each data byte do
//     CRC_16=(CRC_16<<8) ^ CRC_16_Table[(CRC_16>>8)^(data_byte)];
int16u CRC_16_Table[256] =
{
    0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
    0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
    0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
    0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
    0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
    0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
    0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
    0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
    0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
    0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
    0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
    0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
    0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
    0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
    0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
    0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
    0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
};

int CRC16_Init(int16u *Table, int16u Polynomial)
{
    for (size_t Pos=0; Pos<256; Pos++)
    {
        Table[Pos]=(int16u)Pos<<8;

        for(int8u bit=0; bit<8; bit++)
        {
            if (Table[Pos]&0x8000)
                Table[Pos]=(Table[Pos]<<1)^Polynomial;
            else
                Table[Pos]=Table[Pos]<<1;
        }
    }
    return 0;
}

//---------------------------------------------------------------------------
const float64 AC3_dynrng[]=
{
      6.02,
     12.04,
     18.06,
     24.08,
    -18.06,
    -12.04,
    - 6.02,
      0.00,
};

//---------------------------------------------------------------------------
const float64 AC3_compr[]=
{
      6.02,
     12.04,
     18.06,
     24.08,
     30.10,
     36.12,
     42.14,
     48.16,
    -42.14,
    -36.12,
    -30.10,
    -24.08,
    -18.06,
    -12.04,
    - 6.02,
      0.00,
};

//---------------------------------------------------------------------------
const char* AC3_HD_StreamType(int8u StreamType)
{
    switch (StreamType)
    {
        case 0xBA : return "TrueHD";
        case 0xBB : return "MLP";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
int32u AC3_HD_SamplingRate(int8u SamplingRate)
{
    if (SamplingRate==0xF)
        return 0;

    return ((SamplingRate&8)?44100:48000)<<(SamplingRate&7) ;
}

//---------------------------------------------------------------------------
static const int8u AC3_TrueHD_ChannelCountPerBit[13]=
{
    2, //LR
    1, //C
    1, //LFE
    2, //LRs
    2, //LRvh
    2, //LRc
    2, //LRrs
    1, //Cs
    1, //Ts
    2, //LRsd
    2, //LRw
    1, //Cvh
    1, //LFE2
};

//---------------------------------------------------------------------------
int8u AC3_TrueHD_Channels(int16u ChannelsMap)
{
    int8u Channels=0;

    for (int8u Pos=0; Pos<13; Pos++)
        Channels+=AC3_TrueHD_ChannelCountPerBit[Pos]*((ChannelsMap>>Pos)&0x1);

    return Channels;
}

//---------------------------------------------------------------------------
std::string AC3_TrueHD_Channels_Positions(int16u ChannelsMap)
{
    std::string Text;
    if ((ChannelsMap&0x0003)==0x0003)
        Text+="Front: L C R";
    else
    {
        if (ChannelsMap&0x0001)
            Text+="Front: C";
        if (ChannelsMap&0x0002)
            Text+="Front: L, R";
    }

    if (ChannelsMap&0x08)
        Text+=", Side: L R";

    if (ChannelsMap&0x80)
        Text+=", Back: C";

    if ((ChannelsMap&0x0810)==0x0810)
        Text+=", vh: L C R";
    else
    {
        if (ChannelsMap&0x0010)
            Text+=", vh: L R";
        if (ChannelsMap&0x0800)
            Text+=", vh: C";
    }

    if (ChannelsMap&0x0020)
        Text+=", c: L R";
    if (ChannelsMap&0x0040)
        Text+=", Back: L R";
    if (ChannelsMap&0x0100)
        Text+=", s: T";
    if (ChannelsMap&0x0200)
        Text+=", sd: L R";
    if (ChannelsMap&0x0400)
        Text+=", w: L R";

    if (ChannelsMap&0x0004)
        Text+=", LFE";
    if (ChannelsMap&0x1000)
        Text+=", LFE2";

    return Text;
}

//---------------------------------------------------------------------------
Ztring AC3_TrueHD_Channels_Positions2(int16u ChannelsMap)
{
    int8u Front=0, Surround=0, Rear=0, LFE=0;

    if (ChannelsMap&0x0001)
        Front++;
    if (ChannelsMap&0x0002)
        Front+=2;

    if (ChannelsMap&0x08)
        Surround+=2;
    if (ChannelsMap&0x80)
        Surround++;

    if (ChannelsMap&0x0010)
        Rear+=2; //vh
    if (ChannelsMap&0x0800)
        Rear++;  //vh


    if (ChannelsMap&0x0020)
        Rear+=2; //c
    if (ChannelsMap&0x0040)
        Rear+=2; //rs
    if (ChannelsMap&0x0100)
        Rear+=2; //s
    if (ChannelsMap&0x0200)
        Rear+=2; //sd
    if (ChannelsMap&0x0400)
        Rear+=2; //w

    if (ChannelsMap&0x0004)
        LFE++;
    if (ChannelsMap&0x1000)
        LFE++;

    Ztring Text;
    Text+=Ztring::ToZtring(Front);
    Text+=__T('/')+Ztring::ToZtring(Surround);
    Text+=__T('/')+Ztring::ToZtring(Rear);
    Text+=__T('.')+Ztring::ToZtring(LFE);
    return Text;
}

//---------------------------------------------------------------------------
static const int32u AC3_MLP_Channels[32]=
{
    1,
    2,
    3,
    4,
    3,
    4,
    5,
    3,
    4,
    5,
    4,
    5,
    6,
    4,
    5,
    4,
    5,
    6,
    5,
    5,
    6,
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
    0,
};

//---------------------------------------------------------------------------
static const int32u AC3_MLP_Resolution[16]=
{
    16,
    20,
    24,
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
    0,
    0,
    0,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ac3::File_Ac3()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Ac3;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;
    Buffer_TotalBytes_Fill_Max=1024*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;
    Frame_Count_NotParsedIncluded=0;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:2;
    MustParse_dac3=false;
    MustParse_dec3=false;
    CalculateDelay=false;

    //Buffer
    Save_Buffer=NULL;

    //Temp
    Frame_Count_HD=0;
    fscod=0;
    fscod2=0;
    frmsizecod=0;
    bsid_Max=(int8u)-1;
    for (int8u Pos=0; Pos<8; Pos++)
        for (int8u Pos2=0; Pos2<9; Pos2++)
        {
            frmsizplus1_Max[Pos][Pos2]=0;
            acmod_Max[Pos][Pos2]=(int8u)-1;
            lfeon_Max[Pos][Pos2]=false;
            bsmod_Max[Pos][Pos2]=0;
            dsurmod_Max[Pos][Pos2]=0;
            chanmape_Max[Pos][Pos2]=false;
            chanmap_Max[Pos][Pos2]=0;
        }
    numblkscod=0;
    substreamid_Independant_Current=0;
    substreams_Count=0;
    dxc3_Parsed=false;
    HD_MajorSync_Parsed=false;
    Core_IsPresent=false;
    HD_IsPresent=false;
    dynrnge_Exists=false;
    TimeStamp_IsPresent=false;
    TimeStamp_IsParsing=false;
    TimeStamp_Parsed=false;
    TimeStamp_DropFrame_IsValid=false;
    BigEndian=true;
    IgnoreCrc_Done=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ac3::Streams_Fill()
{
    if (HD_MajorSync_Parsed)
    {
        Stream_Prepare(Stream_Audio);
        if (HD_BitRate_Max)
            Fill(Stream_Audio, 0, Audio_BitRate_Maximum, (HD_BitRate_Max*AC3_HD_SamplingRate(HD_SamplingRate2)+8)>>4);

        if (HD_StreamType==0xBA) //TrueHD
        {
            Fill(Stream_General, 0, General_Format, "TrueHD");
            Fill(Stream_Audio, 0, Audio_Format, "TrueHD");

            Fill(Stream_Audio, 0, Audio_Codec, "TrueHD");
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_HD_SamplingRate(HD_SamplingRate1));
            Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_TrueHD_Channels(HD_Channels2));
            Fill(Stream_Audio, 0, Audio_ChannelPositions, AC3_TrueHD_Channels_Positions(HD_Channels2));
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, AC3_TrueHD_Channels_Positions2(HD_Channels2));
            if (Core_IsPresent && !IsSub)
                Fill(Stream_Audio, 0, Audio_MuxingMode, "After core data");
        }

        if (HD_StreamType==0xBB) //TrueHD
        {
            Fill(Stream_General, 0, General_Format, "MLP");
            if (!Core_IsPresent)
            {
                Fill(Stream_Audio, 0, Audio_Format, "MLP");
                Fill(Stream_Audio, 0, Audio_Codec,  "MLP");
            }
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_HD_SamplingRate(HD_SamplingRate2));
            if (HD_SamplingRate1!=HD_SamplingRate2)
                Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_HD_SamplingRate(HD_SamplingRate2));
            Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_MLP_Channels[HD_Channels1]);
            if (HD_Channels1!=HD_Channels2)
                Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_MLP_Channels[HD_Channels1]);
            Fill(Stream_Audio, 0, Audio_BitDepth, AC3_MLP_Resolution[HD_Resolution2]);
            if (HD_Resolution1!=HD_Resolution2)
                Fill(Stream_Audio, 0, Audio_BitDepth, AC3_MLP_Resolution[HD_Resolution1]);
        }
    }

    //AC-3
    if (bsid_Max<=0x09)
    {
        if (Count_Get(Stream_Audio)==0)
            Stream_Prepare(Stream_Audio);
        Fill(Stream_General, 0, General_Format, "AC-3");
        Fill(Stream_Audio, 0, Audio_Format, "AC-3");
        Fill(Stream_Audio, 0, Audio_Codec, "AC3");
        Fill(Stream_Audio, 0, Audio_BitDepth, 16);

        int32u Divider=bsid_Max==9?2:1; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
        if (Ztring::ToZtring(AC3_SamplingRate[fscod]/Divider)!=Retrieve(Stream_Audio, 0, Audio_SamplingRate))
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_SamplingRate[fscod]/Divider);
        if (frmsizecod/2<19)
        {
            if (Frame_Count_HD)
                Fill(Stream_Audio, 0, Audio_BitRate, "Unknown");
            int32u BitRate=AC3_BitRate[frmsizecod/2]*1000;
            int32u Divider=bsid_Max==9?2:1; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate/Divider);
            if (CalculateDelay && Buffer_TotalBytes_FirstSynched>100 && BitRate>0)
            {
                Fill(Stream_Audio, 0, Audio_Delay, (float)Buffer_TotalBytes_FirstSynched*8*1000/BitRate, 0);
                Fill(Stream_Audio, 0, Audio_Delay_Source, "Stream");
            }
        }

        Fill(Stream_Audio, 0, Audio_Format_Settings_ModeExtension, AC3_Mode[bsmod_Max[0][0]]);
        if (acmod_Max[0][0]==0)
            Fill(Stream_Audio, 0, Audio_Format_Settings_Mode, "Dual Mono");
        if (acmod_Max[0][0]!=(int8u)-1)
        {
            int8u Channels=AC3_Channels[acmod_Max[0][0]];
            Ztring ChannelPositions; ChannelPositions.From_Local(AC3_ChannelPositions[acmod_Max[0][0]]);
            Ztring ChannelPositions2; ChannelPositions2.From_Local(AC3_ChannelPositions2[acmod_Max[0][0]]);
            Ztring ChannelLayout; ChannelLayout.From_Local(lfeon_Max[0][0]?AC3_ChannelLayout_lfeon[acmod_Max[0][0]]:AC3_ChannelLayout_lfeoff[acmod_Max[0][0]]);
            if (lfeon_Max[0][0])
            {
                Channels+=1;
                ChannelPositions+=__T(", LFE");
                ChannelPositions2+=__T(".1");
            }
            if (Ztring::ToZtring(Channels)!=Retrieve(Stream_Audio, 0, Audio_Channel_s_))
                Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
            if (ChannelPositions!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions))
                Fill(Stream_Audio, 0, Audio_ChannelPositions, ChannelPositions);
            if (ChannelPositions2!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions_String2))
                Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, ChannelPositions2);
            if (ChannelLayout!=Retrieve(Stream_Audio, 0, Audio_ChannelLayout))
                Fill(Stream_Audio, 0, Audio_ChannelLayout, ChannelLayout);
        }
        if (dsurmod_Max[0][0]==2)
        {
            Fill(Stream_Audio, 0, Audio_Format_Profile, "Dolby Digital");
            Fill(Stream_Audio, 0, Audio_Codec_Profile, "Dolby Digital");
        }
        if (__T("CBR")!=Retrieve(Stream_Audio, 0, Audio_BitRate_Mode))
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
    }

    //E-AC-3
    else if (bsid_Max<=0x10)
    {
        for (size_t Pos=0; Pos<8; Pos++)
            if (acmod_Max[Pos][0]!=(int8u)-1)
            {
                Stream_Prepare(Stream_Audio);
                Fill(Stream_Audio, 0, Audio_Format, "E-AC-3");
                Fill(Stream_Audio, 0, Audio_Codec, "AC3+");

                if (acmod_Max[1][0]!=(int8u)-1)
                    Fill(Stream_Audio, 0, Audio_ID, 1+Pos);

                Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
                int8u numblks=numblkscod==3?6:numblkscod+1;
                int32u frmsiz_Total=0;
                for (size_t Pos2=0; Pos2<8; Pos2++)
                    frmsiz_Total+=frmsizplus1_Max[Pos][Pos2];
                if (numblks)
                {
                    Fill(Stream_Audio, 0, Audio_BitRate, ((frmsiz_Total*2)*8*(750/((int32u)numblks)))/4);
                    if (frmsizplus1_Max[Pos][1]) //If dependand substreams
                        Fill(Stream_Audio, 0, Audio_BitRate, ((frmsizplus1_Max[Pos][0]*2)*8*(750/((int16u)numblks)))/4);

                }

                if (fscod!=2)
                    Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_SamplingRate[fscod]);
                else
                    Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_SamplingRate2[fscod2]);

                if (acmod_Max[Pos][1]!=(int8u)-1)
                {
                    int16u chanmap_Final=0;
                    for (int8u Pos2=0; Pos2<9; Pos2++)
                        if (acmod_Max[Pos][Pos2]!=(int8u)-1)
                        {
                            if (chanmape_Max[Pos][Pos2])
                                chanmap_Final|=chanmap_Max[Pos][Pos2];
                            else
                            {
                                chanmap_Final|=AC3_acmod2chanmap[acmod_Max[Pos][Pos2]];
                                if (lfeon_Max[Pos][Pos2])
                                    chanmap_Final|=1; // LFE position in chanmap is bit 0
                            }
                        }

                        Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_chanmap_Channels(chanmap_Final));
                        Fill(Stream_Audio, 0, Audio_ChannelPositions, AC3_chanmap_ChannelPositions(chanmap_Final));
                        Ztring ChannelLayout; ChannelLayout.From_Local(lfeon_Max[0][0]?AC3_ChannelLayout_lfeon[acmod_Max[0][0]]:AC3_ChannelLayout_lfeoff[acmod_Max[0][0]]);
                        Fill(Stream_Audio, 0, Audio_ChannelLayout, AC3_chanmap_ChannelLayout(chanmap_Final, ChannelLayout));
                }
                if (acmod_Max[Pos][0]==0)
                {
                    Fill(Stream_Audio, 0, Audio_Format_Profile, "Dual Mono");
                    Fill(Stream_Audio, 0, Audio_Codec_Profile, "Dual Mono");
                }
                else if (acmod_Max[Pos][0]!=(int8u)-1)
                {
                    int8u Channels=AC3_Channels[acmod_Max[Pos][0]];
                    Ztring ChannelPositions; ChannelPositions.From_Local(AC3_ChannelPositions[acmod_Max[Pos][0]]);
                    if (lfeon_Max[Pos][0])
                    {
                        Channels+=1;
                        ChannelPositions+=__T(", LFE");
                    }
                    Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
                    Fill(Stream_Audio, 0, Audio_ChannelPositions, ChannelPositions);
                    Fill(Stream_Audio, 0, Audio_ChannelLayout, lfeon_Max[0][0]?AC3_ChannelLayout_lfeon[acmod_Max[0][0]]:AC3_ChannelLayout_lfeoff[acmod_Max[0][0]]);
                }
            }
    }

    if (HD_MajorSync_Parsed)
    {
        //Filling Maximum bitrate with the constant core bitrate for better coherancy
        ZtringList List;
        List.Separator_Set(0, __T(" / "));
        List.Write(Retrieve(Stream_Audio, 0, Audio_BitRate));
        if (List.size()>=2)
            Fill(Stream_Audio, 0, Audio_BitRate_Maximum, List[1]);
    }

    //Dolby Metadata
    if (Core_IsPresent)
    {
        //Endianess
        Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, BigEndian?"Big":"Little");
        Fill(Stream_Audio, 0, "bsid", bsid_Max);

        Fill(Stream_Audio, 0, "dialnorm", FirstFrame_Dolby.dialnorm==0?-31:-FirstFrame_Dolby.dialnorm);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm"), Info_Options)=__T("N NT");
        Fill(Stream_Audio, 0, "dialnorm/String", Ztring::ToZtring(FirstFrame_Dolby.dialnorm==0?-31:-FirstFrame_Dolby.dialnorm)+__T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm/String"), Info_Options)=__T("N NT");
        if (FirstFrame_Dolby.compre)
        {
            float64 Value=AC3_compr[FirstFrame_Dolby.compr>>4]+20*std::log10(((float)(0x10+(FirstFrame_Dolby.compr&0x0F)))/32);
            Fill(Stream_Audio, 0, "compr", Value, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr"), Info_Options)=__T("N NT");
            Fill(Stream_Audio, 0, "compr/String", Ztring::ToZtring(Value, 2)+__T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr/String"), Info_Options)=__T("N NT");
        }
        if (FirstFrame_Dolby.dynrnge)
        {
            float64 Value;
            if (FirstFrame_Dolby.dynrng==0)
                Value=0; //Special case in the formula
            else
                Value=AC3_dynrng[FirstFrame_Dolby.dynrng>>5]+20*std::log10(((float)(0x20+(FirstFrame_Dolby.dynrng&0x1F)))/64);
            Fill(Stream_Audio, 0, "dynrng", Value, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng"), Info_Options)=__T("N NT");
            Fill(Stream_Audio, 0, "dynrng/String", Ztring::ToZtring(Value, 2)+__T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng/String"), Info_Options)=__T("N NT");
        }

        for (int8u Pos=0; Pos<8; Pos++)
            for (int8u Pos2=0; Pos2<9; Pos2++)
            {
                if (acmod_Max[Pos][Pos2]==(int8u)-1)
                    break;
                if (acmod_Max[Pos][Pos2]!=(int8u)-1)
                {
                    if (acmod_Max[Pos][Pos2]==2)
                    {
                        Fill(Stream_Audio, 0, "dsurmod", dsurmod_Max[Pos][Pos2]);
                        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dsurmod"), Info_Options)=__T("N NT");
                        Fill(Stream_Audio, 0, "dsurmod/String", AC3_Surround[dsurmod_Max[Pos][Pos2]]);
                        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dsurmod/String"), Info_Options)=__T("N NT");
                    }
                    (*Stream_More)[Stream_Audio][0](Ztring().From_Local("bsid"), Info_Options)=__T("N NT");
                    Fill(Stream_Audio, 0, "acmod", acmod_Max[Pos][Pos2]);
                    (*Stream_More)[Stream_Audio][0](Ztring().From_Local("acmod"), Info_Options)=__T("N NT");
                    Fill(Stream_Audio, 0, "lfeon", (lfeon_Max[Pos][Pos2])?1:0);
                    (*Stream_More)[Stream_Audio][0](Ztring().From_Local("lfeon"), Info_Options)=__T("N NT");
                }
            }
    }

    //TimeStamp
    if (TimeStamp_IsPresent)
    {
        Fill(Stream_Audio, 0, Audio_Delay, TimeStamp_Content*1000, 0);
        Fill(Stream_Audio, 0, Audio_Delay_Source, "Stream");
        if (TimeStamp_DropFrame_IsValid)
            Fill(Stream_Audio, 0, Audio_Delay_Settings, TimeStamp_DropFrame_Content?"drop_frame_flag=1":"drop_frame_flag=0");
    }
}

//---------------------------------------------------------------------------
void File_Ac3::Streams_Finish()
{
    //Stats
    if (!dialnorms.empty())
    {
        int8u Minimum_Raw=1;
        int8u Maximum_Raw=31;
        float64 Sum_Intensity=0;
        int64u Count=0;
        for (int8u Pos=0; (size_t)Pos<dialnorms.size(); Pos++)
            if (dialnorms[Pos])
            {
                if (Minimum_Raw<(Pos==0?31:Pos))
                    Minimum_Raw=(Pos==0?31:Pos);
                if (Maximum_Raw>(Pos==0?31:Pos))
                    Maximum_Raw=(Pos==0?31:Pos);
                Sum_Intensity+=dialnorms[Pos]*pow(10, -((float64)Pos)/10);
                Count+=dialnorms[Pos];
            }
        if (Count)
        {
            float64 Average_dB = log10(Sum_Intensity / Count) * 10;
            Fill(Stream_Audio, 0, "dialnorm_Average", Average_dB, 0);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Average"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dialnorm_Average/String", Ztring::ToZtring(Average_dB, 0) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Average/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dialnorm_Minimum", -Minimum_Raw);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Minimum"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dialnorm_Minimum/String", Ztring::ToZtring(-Minimum_Raw) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Minimum/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dialnorm_Maximum", -Maximum_Raw);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Maximum"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dialnorm_Maximum/String", Ztring::ToZtring(-Maximum_Raw) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Maximum/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dialnorm_Count", Count);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Count"), Info_Options) = __T("N NT");
        }
    }
    if (!comprs.empty())
    {
        float64 Minimum_dB=47.89;
        float64 Maximum_dB=-48.16;
        float64 Sum_Intensity=0;
        int64u Count=0;
        for (size_t Pos=0; Pos<comprs.size(); Pos++)
            if (comprs[Pos])
            {
                float64 Value=AC3_compr[Pos>>4]+20*std::log10(((float)(0x10+(Pos&0x0F)))/32);
                if (Minimum_dB>Value)
                    Minimum_dB=Value;
                if (Maximum_dB<Value)
                    Maximum_dB=Value;
                Sum_Intensity+=comprs[Pos]*pow(10, Value/10);
                Count+=comprs[Pos];
            }
        if (Count)
        {
            float64 Average_dB = log10(Sum_Intensity / Count) * 10;
            Fill(Stream_Audio, 0, "compr_Average", Average_dB, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Average"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "compr_Average/String", Ztring::ToZtring(Average_dB, 2) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Average/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "compr_Minimum", Minimum_dB, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Minimum"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "compr_Minimum/String", Ztring::ToZtring(Minimum_dB, 2) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Minimum/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "compr_Maximum", Maximum_dB, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Maximum"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "compr_Maximum/String", Ztring::ToZtring(Maximum_dB, 2) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Maximum/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "compr_Count", Count);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Count"), Info_Options) = __T("N NT");
        }
    }
    if (dynrnge_Exists && !dynrngs.empty())
    {
        float64 Minimum_dB=23.95;
        float64 Maximum_dB=-24.08;
        float64 Sum_Intensity=0;
        int64u Count=0;
        for (size_t Pos=0; Pos<dynrngs.size(); Pos++)
            if (dynrngs[Pos])
            {
                float64 Value;
                if (Pos==0)
                    Value=0; //Special case in the formula
                else
                    Value=AC3_dynrng[Pos>>5]+20*std::log10(((float)(0x20+(Pos&0x1F)))/64);
                if (Minimum_dB>Value)
                    Minimum_dB=Value;
                if (Maximum_dB<Value)
                    Maximum_dB=Value;
                Sum_Intensity+=dynrngs[Pos]*pow(10, Value/10);
                Count+=dynrngs[Pos];
            }
        if (Count)
        {
            float64 Average_dB = log10(Sum_Intensity / Count) * 10;
            Fill(Stream_Audio, 0, "dynrng_Average", Average_dB, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Average"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dynrng_Average/String", Ztring::ToZtring(Average_dB, 2) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Average/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dynrng_Minimum", Minimum_dB, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Minimum"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dynrng_Minimum/String", Ztring::ToZtring(Minimum_dB, 2) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Minimum/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dynrng_Maximum", Maximum_dB, 2);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Maximum"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dynrng_Maximum/String", Ztring::ToZtring(Maximum_dB, 2) + __T(" dB"));
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Maximum/String"), Info_Options) = __T("N NT");
            Fill(Stream_Audio, 0, "dynrng_Count", Count);
            (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Count"), Info_Options) = __T("N NT");
        }
    }

    //Duration
    if (!IsSub)
    {
        int64u Frame_Count_ForDuration=0;
        if (MediaInfoLib::Config.ParseSpeed_Get()==1)
        {
            Frame_Count_ForDuration=Frame_Count; //We have the exact count of frames
            Fill(Stream_Audio, 0, Audio_StreamSize, File_Offset+Buffer_Offset+Element_Size-File_Offset_FirstSynched);
        }
        else if (bsid_Max<=9 && frmsizecods.size()==1 && fscods.size()==1 && Frame_Count_HD==0)
        {
            int16u Size=AC3_FrameSize_Get(frmsizecods.begin()->first, fscods.begin()->first);
            if (Size)
            {
                if (TimeStamp_IsPresent)
                    Size+=16;
                Frame_Count_ForDuration=(File_Size-File_Offset_FirstSynched)/Size; //Only complete frames
                Fill(Stream_Audio, 0, Audio_StreamSize, Frame_Count_ForDuration*Size);
            }
        }
        if (Frame_Count_ForDuration)
        {
            Clear(Stream_Audio, 0, Audio_BitRate);

            //HD part
            if (Frame_Count_HD)
            {
                int32u HD_SamplingRate=AC3_HD_SamplingRate(HD_SamplingRate1);
                if (HD_SamplingRate)
                {
                    int8u FrameDuration; //In samples
                    if (HD_SamplingRate<44100)
                        FrameDuration=0; //Unknown
                    else if (HD_SamplingRate<=48000)
                        FrameDuration=40;
                    else if (HD_SamplingRate<=96000)
                        FrameDuration=80;
                    else if (HD_SamplingRate<=192000)
                        FrameDuration=160;
                    else
                        FrameDuration=0; //Unknown
                    if (FrameDuration)
                    {
                        int64u SamplingCount=Frame_Count_HD*FrameDuration;
                        Fill(Stream_Audio, 0, Audio_Duration, SamplingCount/(((float64)HD_SamplingRate)/1000), 0);
                        Fill(Stream_Audio, 0, Audio_SamplingCount, SamplingCount);
                        Fill(Stream_Audio, 0, Audio_BitRate, (File_Size-File_Offset_FirstSynched)/(SamplingCount/(((float64)HD_SamplingRate)/1000))*8, 0);
                    }
                    Fill(Stream_Audio, 0, Audio_FrameCount, Frame_Count_HD);
                }
            }
            if (Core_IsPresent)
            {
                Fill(Stream_Audio, 0, Audio_FrameCount, Frame_Count_ForDuration);
                if (AC3_SamplingRate[fscod])
                {
                    float64 FrameDuration;
                    if (bsid_Max<=0x08)
                        FrameDuration=32;
                    else if (bsid_Max<=0x09)
                        FrameDuration=16; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
                    else
                        FrameDuration=0;
                    if (FrameDuration)
                    {
                        FrameDuration*=((float64)48000)/AC3_SamplingRate[fscod]; //32 ms for 48 KHz, else proportional (34.83 for 44.1 KHz, 48 ms for 32 KHz)
                        Fill(Stream_Audio, 0, Audio_SamplingCount, Frame_Count_ForDuration*1536);
                        Fill(Stream_Audio, 0, Audio_Duration, Frame_Count_ForDuration*FrameDuration, 0);
                        int32u BitRate=AC3_BitRate[frmsizecod/2]*1000;
                        int32u Divider=bsid_Max==9?2:1; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
                        Fill(Stream_Audio, 0, Audio_BitRate, BitRate/Divider);
                    }
                }
            }
        }
    }
    else if (FrameInfo.PTS!=(int64u)-1 && FrameInfo.PTS>PTS_Begin)
    {
        Fill(Stream_Audio, 0, Audio_Duration, float64_int64s(((float64)(FrameInfo.PTS-PTS_Begin))/1000000));
        float64 FrameDuration;
        if (bsid_Max<=0x08)
            FrameDuration=32;
        else if (bsid_Max<=0x09)
            FrameDuration=16; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
        else if (bsid_Max>0x0A && bsid_Max<=0x10)
            FrameDuration=((float64)32)/6;
        else
            FrameDuration=0;
        if (FrameDuration)
            Fill(Stream_Audio, 0, Audio_FrameCount, float64_int64s(((float64)(FrameInfo.PTS-PTS_Begin))/1000000/FrameDuration));
    }
}

//---------------------------------------------------------------------------
void File_Ac3::Read_Buffer_Unsynched()
{
    delete[] Save_Buffer; Save_Buffer=NULL;

    if (File_GoTo==0)
        Synched_Init();
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_Ac3::Read_Buffer_Seek (size_t Method, int64u Value, int64u /*ID*/)
{
    GoTo(0);
    Open_Buffer_Unsynch();
    return 1;
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ac3::FileHeader_Begin()
{
    //Specific cases
    if (MustParse_dac3 || MustParse_dec3)
        return true;

    //Must have enough buffer for having header
    if (Buffer_Size<4)
        return false; //Must wait for more data

    //False positives detection: detect Matroska files, AC-3 parser is not smart enough
    if (!FileHeader_Begin_0x000001())
    {
        Finish("AC-3");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ac3::Synchronize()
{
    //Specific cases
    if (MustParse_dac3 || MustParse_dec3)
        return true;

    //Synchronizing
    while (Buffer_Offset+8<=Buffer_Size)
    {
        if (!FrameSynchPoint_Test())
            return false; //Need more data
        if (Synched)
            break;
        Buffer_Offset++;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+8>Buffer_Size)
    {
        //We must keep more bytes in order to detect TimeStamp
        if (Frame_Count==0)
        {
            if (Buffer_Offset>=16)
                Buffer_Offset-=16;
            else
                Buffer_Offset=0;
            return false;
        }

        if (Buffer_Offset+7==Buffer_Size && CC3(Buffer+Buffer_Offset+4)!=0xF8726F && CC2(Buffer+Buffer_Offset)!=0x0B77 && CC2(Buffer+Buffer_Offset)!=0x770B)
            Buffer_Offset++;
        if (Buffer_Offset+6==Buffer_Size && CC2(Buffer+Buffer_Offset+4)!=0xF872   && CC2(Buffer+Buffer_Offset)!=0x0B77 && CC2(Buffer+Buffer_Offset)!=0x770B)
            Buffer_Offset++;
        if (Buffer_Offset+5==Buffer_Size && CC1(Buffer+Buffer_Offset+4)!=0xF8     && CC2(Buffer+Buffer_Offset)!=0x0B77 && CC2(Buffer+Buffer_Offset)!=0x770B)
            Buffer_Offset++;
        if (Buffer_Offset+4==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0B77 && CC2(Buffer+Buffer_Offset)!=0x770B)
            Buffer_Offset++;
        if (Buffer_Offset+3==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0B77 && CC2(Buffer+Buffer_Offset)!=0x770B)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0B77 && CC2(Buffer+Buffer_Offset)!=0x770B)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x0B && CC1(Buffer+Buffer_Offset)!=0x77)
            Buffer_Offset++;
        return false;
    }

    //Testing if we have TimeStamp
    if (Buffer_Offset>=16)
    {
        if ( Buffer[Buffer_Offset-0x10+0x00]==0x01      //Magic value? Always 0x01
         &&  Buffer[Buffer_Offset-0x10+0x01]==0x10      //Size? Always 0x10
         &&  Buffer[Buffer_Offset-0x10+0x02]==0x00      //First  byte of HH? Always 0x00
         && (Buffer[Buffer_Offset-0x10+0x03]>>4 )<0x6   //Second byte of HH? First  4 bits must be <0x6
         && (Buffer[Buffer_Offset-0x10+0x03]&0xF)<0xA   //Second byte of HH? Second 4 bits must be <0xA
         &&  Buffer[Buffer_Offset-0x10+0x04]==0x00      //First  byte of MM? Always 0x00
         && (Buffer[Buffer_Offset-0x10+0x05]>>4 )<0x6   //Second byte of MM? First  4 bits must be <0x6
         && (Buffer[Buffer_Offset-0x10+0x05]&0xF)<0xA   //Second byte of MM? Second 4 bits must be <0xA
         &&  Buffer[Buffer_Offset-0x10+0x06]==0x00      //First  byte of SS? Always 0x00
         && (Buffer[Buffer_Offset-0x10+0x07]>>4 )<0x6   //Second byte of SS? First  4 bits must be <0x6
         && (Buffer[Buffer_Offset-0x10+0x07]&0xF)<0xA   //Second byte of SS? Second 4 bits must be <0xA
         &&  Buffer[Buffer_Offset-0x10+0x08]==0x00      //First  byte of FF? Always 0x00
         && (Buffer[Buffer_Offset-0x10+0x09]>>4 )<0x4   //Second byte of FF? First  4 bits must be <0x4
         && (Buffer[Buffer_Offset-0x10+0x09]&0xF)<0xA   //Second byte of FF? Second 4 bits must be <0xA
         && !(Buffer[Buffer_Offset-0x10+0x00]==0x00     //We want at least a byte not zero, in order to differentiate TimeStamp from padding
           && Buffer[Buffer_Offset-0x10+0x01]==0x00
           && Buffer[Buffer_Offset-0x10+0x0C]==0x00
           && Buffer[Buffer_Offset-0x10+0x0D]==0x00
           && Buffer[Buffer_Offset-0x10+0x0E]==0x00
           && Buffer[Buffer_Offset-0x10+0x0F]==0x00))
        {
            TimeStamp_IsPresent=true;
            Buffer_Offset-=16;

            if (Frame_Count_Valid<10000)
                Frame_Count_Valid=10000; //Setting it to 10000 in order to be able to have the drop frame for extreme stream (60 fps...)
        }
    }

    //Synched
    return true;
}

//---------------------------------------------------------------------------
void File_Ac3::Synched_Init()
{
    //FrameInfo
    PTS_End=0;
    if (FrameInfo.DTS==(int64u)-1)
        FrameInfo.DTS=0; //No DTS in container
    if (FrameInfo.PTS==(int64u)-1)
        FrameInfo.PTS=0; //No PTS in container
    DTS_Begin=FrameInfo.DTS;
    DTS_End=FrameInfo.DTS;
    if (Frame_Count_NotParsedIncluded==(int64u)-1)
        Frame_Count_NotParsedIncluded=0; //No Frame_Count_NotParsedIncluded in the container
}

//---------------------------------------------------------------------------
bool File_Ac3::Synched_Test()
{
    //Specific cases
    if (MustParse_dac3 || MustParse_dec3)
        return true;

    //Must have enough buffer for having header
    if (Buffer_Offset+(TimeStamp_IsPresent?16:0)+6>Buffer_Size)
        return false;

    //TimeStamp
    if (TimeStamp_IsPresent && !TimeStamp_Parsed)
    {
        if (!( Buffer[Buffer_Offset+0x00]==0x01         //Magic value? Always 0x01
           &&  Buffer[Buffer_Offset+0x01]==0x10         //Size? Always 0x10
           &&  Buffer[Buffer_Offset+0x02]==0x00         //First  byte of HH? Always 0x00
           && (Buffer[Buffer_Offset+0x03]>>4 )<0x6      //Second byte of HH? First  4 bits must be <0x6
           && (Buffer[Buffer_Offset+0x03]&0xF)<0xA      //Second byte of HH? Second 4 bits must be <0xA
           &&  Buffer[Buffer_Offset+0x04]==0x00         //First  byte of MM? Always 0x00
           && (Buffer[Buffer_Offset+0x05]>>4 )<0x6      //Second byte of MM? First  4 bits must be <0x6
           && (Buffer[Buffer_Offset+0x05]&0xF)<0xA      //Second byte of MM? Second 4 bits must be <0xA
           &&  Buffer[Buffer_Offset+0x06]==0x00         //First  byte of SS? Always 0x00
           && (Buffer[Buffer_Offset+0x07]>>4 )<0x6      //Second byte of SS? First  4 bits must be <0x6
           && (Buffer[Buffer_Offset+0x07]&0xF)<0xA      //Second byte of SS? Second 4 bits must be <0xA
           &&  Buffer[Buffer_Offset+0x08]==0x00         //First  byte of FF? Always 0x00
           && (Buffer[Buffer_Offset+0x09]>>4 )<0x4      //Second byte of FF? First  4 bits must be <0x4
           && (Buffer[Buffer_Offset+0x09]&0xF)<0xA))    //Second byte of FF? Second 4 bits must be <0xA
            TimeStamp_IsPresent=false;
    }
    if (TimeStamp_IsPresent && !TimeStamp_Parsed)
        Buffer_Offset+=16;

    //Quick test of synchro
    if (!FrameSynchPoint_Test())
        return false; //Need more data
    if (!Synched)
        return true;

    //TimeStamp
    if (TimeStamp_IsPresent && !TimeStamp_Parsed)
    {
        Buffer_Offset-=16;
        if (Synched)
        {
            TimeStamp_IsParsing=true;
            TimeStamp_Parsed=false;
        }
        else
        {
            TimeStamp_IsParsing=false;
            TimeStamp_Parsed=false;
        }
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Ac3::Demux_UnpacketizeContainer_Test()
{
    if (TimeStamp_IsPresent)
        Buffer_Offset+=16;

    if (!HD_IsPresent && Frame_Count==0 && Save_Buffer==NULL)
    {
        //Searching HD part
        size_t Buffer_Offset_Save=Buffer_Offset;
        Buffer_Offset++;
        Synched=false;
        while (Buffer_Offset+8<=Buffer_Size)
        {
            if (!FrameSynchPoint_Test())
            {
                Buffer_Offset=Buffer_Offset_Save;
                return false; //Need more data
            }
            if (Synched)
                break;
            Buffer_Offset++;
        }
        Buffer_Offset=Buffer_Offset_Save;
        if (!Synched)
        {
            Synched=true;
            if (TimeStamp_IsPresent)
                Buffer_Offset-=16;
            return false; //Need more data
        }
    }

    if (Save_Buffer)
    {
        Demux_TotalBytes-=Buffer_Offset;
        Demux_Offset-=Buffer_Offset;
        File_Offset+=Buffer_Offset;
        swap(Buffer, Save_Buffer);
        swap(Buffer_Offset, Save_Buffer_Offset);
        swap(Buffer_Size, Save_Buffer_Size);
    }

    if (Buffer[Buffer_Offset]==0x0B && Buffer[Buffer_Offset+1]==0x77)
    {
        int8u bsid=Buffer[Buffer_Offset+5]>>3;
        if (bsid<=0x08)
            FrameInfo.DUR=32000000;
        else if (bsid<=0x09)
            FrameInfo.DUR=16000000; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
        else if (bsid>0x0A && bsid<=0x10)
        {
            numblkscod=(Buffer[Buffer_Offset+4]>>4)&0x3;
            int64u numblks=numblkscod==3?6:numblkscod+1;
            FrameInfo.DUR=32000000*numblks/6;
        }

        Demux_Offset=Buffer_Offset+Core_Size_Get();

        //Core part
        if (HD_IsPresent)
        {
            if (TimeStamp_IsPresent)
                Buffer_Offset-=16;

            if (Save_Buffer)
            {
                swap(Buffer, Save_Buffer);
                swap(Buffer_Offset, Save_Buffer_Offset);
                swap(Buffer_Size, Save_Buffer_Size);
                Demux_TotalBytes+=Buffer_Offset;
                Demux_Offset+=Buffer_Offset;
                File_Offset-=Buffer_Offset;
            }

            return true; //No AC-3 demux
        }
    }
    else
    {
        Demux_Offset=Buffer_Offset+HD_Size_Get();
    }

    if (Demux_Offset>Buffer_Size && File_Offset+Buffer_Size!=File_Size)
    {
        if (TimeStamp_IsPresent)
            Buffer_Offset-=16;

        if (Save_Buffer)
        {
            swap(Buffer, Save_Buffer);
            swap(Buffer_Offset, Save_Buffer_Offset);
            swap(Buffer_Size, Save_Buffer_Size);
            Demux_TotalBytes+=Buffer_Offset;
            Demux_Offset+=Buffer_Offset;
            File_Offset-=Buffer_Offset;
        }

        return false; //No complete frame
    }

    Demux_UnpacketizeContainer_Demux();

    if (Save_Buffer)
    {
        swap(Buffer, Save_Buffer);
        swap(Buffer_Offset, Save_Buffer_Offset);
        swap(Buffer_Size, Save_Buffer_Size);
        Demux_TotalBytes+=Buffer_Offset;
        Demux_Offset+=Buffer_Offset;
        File_Offset-=Buffer_Offset;
    }

    if (TimeStamp_IsPresent)
        Buffer_Offset-=16;

    return true;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ac3::Read_Buffer_Continue()
{
    if (MustParse_dac3)
    {
        dac3();
        return;
    }
    if (MustParse_dec3)
    {
        dec3();
        return;
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ac3::Header_Parse()
{
    //TimeStamp
    if (TimeStamp_IsParsing)
    {
        Header_Fill_Size(16);
        Header_Fill_Code(2, "TimeStamp");
        return;
    }
    else
        TimeStamp_Parsed=false; //Currently, only one kind of intermediate element is detected (no TimeStamp and HD part together), and we don't know the precise specification of MLP nor TimeStamp, so we consider next eleemnt is TimeStamp

    if (Save_Buffer)
    {
        File_Offset+=Buffer_Offset;
        swap(Buffer, Save_Buffer);
        swap(Buffer_Offset, Save_Buffer_Offset);
        swap(Buffer_Size, Save_Buffer_Size);
    }

    //Filling
    if ((Buffer[Buffer_Offset]==0x0B && Buffer[Buffer_Offset+1]==0x77)
     || (Buffer[Buffer_Offset]==0x77 && Buffer[Buffer_Offset+1]==0x0B))
    {
        Header_Fill_Size(Core_Size_Get());
        Header_Fill_Code(0, "syncframe");

        //Little Endian management
        if (Save_Buffer)
        {
            swap(Buffer, Save_Buffer);
            swap(Buffer_Offset, Save_Buffer_Offset);
            swap(Buffer_Size, Save_Buffer_Size);
            File_Offset-=Buffer_Offset;
        }

        return;
    }

    //MLP or TrueHD specific
    int16u Size;
    BS_Begin();
    Skip_S1( 4,                                                 "CRC?");
    Get_S2 (12, Size,                                           "Size");
    BS_End();
    Skip_B2(                                                    "Timestamp?");

    //Little Endian management
    if (Save_Buffer)
    {
        swap(Buffer, Save_Buffer);
        swap(Buffer_Offset, Save_Buffer_Offset);
        swap(Buffer_Size, Save_Buffer_Size);
    }

    //Filling
    if (Size<2)
    {
        Synched=false;
        Size=2;
    }

    Size*=2;
    Header_Fill_Size(Size);
    Header_Fill_Code(1, "HD");
}

//---------------------------------------------------------------------------
void File_Ac3::Data_Parse()
{
    if (Save_Buffer)
    {
        File_Offset+=Buffer_Offset;
        swap(Buffer, Save_Buffer);
        swap(Buffer_Offset, Save_Buffer_Offset);
        swap(Buffer_Size, Save_Buffer_Size);
    }

    //Parsing
    switch (Element_Code)
    {
        case 0 :
                    Element_Info1C((FrameInfo.PTS!=(int64u)-1), __T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000)));
                    Element_Info1(Frame_Count);
                    Core();
                    break;
        case 1 :
                    Element_Info1C((FrameInfo.PTS!=(int64u)-1), __T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000)));
                    Element_Info1(Frame_Count);
                    HD();
                    break;
        case 2 : TimeStamp();   break;
        default: ;
    }

    //Little Endian management
    if (Save_Buffer)
    {
        delete[] Buffer;
        Buffer=Save_Buffer; Save_Buffer=NULL;
        Buffer_Offset=Save_Buffer_Offset;
        Buffer_Size=Save_Buffer_Size;
        File_Offset-=Buffer_Offset;
    }
}

//---------------------------------------------------------------------------
void File_Ac3::Core()
{
    while (Element_Offset<Element_Size)
    {
        if (substreams_Count)
        {
            Element_Name("Block");
            Element_Begin1("syncframe");
        }
        Core_Frame();
        if (substreams_Count)
            Element_End0();
    }

    if (acmod_Max[0][0]==(int8u)-1)
        return; //Waiting for the first sync frame

    FILLING_BEGIN();
        //Counting
        if (Frame_Count==0)
        {
            Core_IsPresent=true;
            PTS_Begin=FrameInfo.PTS;
        }
        Frame_Count++;
        Frame_Count_InThisBlock++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (bsid<=0x08)
            FrameInfo.DUR=32000000;
        else if (bsid<=0x09)
            FrameInfo.DUR=16000000; // Unofficial hack for low sample rate (e.g. 22.05 kHz)
        else if (bsid>0x0A && bsid<=0x10)
        {
            int64u numblks = numblkscod == 3 ? 6 : numblkscod + 1;
            FrameInfo.DUR=32000000*numblks/6;
        }
        if (fscod && AC3_SamplingRate[fscod])
        {
            FrameInfo.DUR*=48000;
            FrameInfo.DUR/=AC3_SamplingRate[fscod];
        }
        if (fscod==3 && AC3_SamplingRate2[fscod2])
        {
            FrameInfo.DUR*=48000;
            FrameInfo.DUR/=AC3_SamplingRate2[fscod2];
        }
        if (FrameInfo.DTS!=(int64u)-1)
            FrameInfo.DTS+=FrameInfo.DUR;
        if (FrameInfo.PTS!=(int64u)-1)
            FrameInfo.PTS=FrameInfo.DTS;

        if (File_Offset+Buffer_Offset+Element_Size==File_Size)
            Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames

        //Filling
        if (!Status[IsAccepted])
            Accept("AC-3");
        if (!Status[IsFilled] && Frame_Count>=Frame_Count_Valid)
        {
            Fill("AC-3");

            //No more need data
            if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1)
                Finish("AC-3");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Ac3::Core_Frame()
{
    //Parsing
    int16u frmsiz=0, chanmap=0;
    int8u  dialnorm=(int8u)-1, dialnorm2=(int8u)-1, compr=(int8u)-1, compr2=(int8u)-1, dynrng=(int8u)-1, dynrng2=(int8u)-1;
    int8u  strmtyp=0, substreamid=0, acmod=0, bsmod=0, dsurmod=0;
    bool   compre=false, compr2e=false, dynrnge=false, dynrng2e=false;
    bool   lfeon=false, chanmape=false;
    if (bsid<=0x09)
    {
        Element_Begin1("synchinfo");
            Skip_B2(                                                "syncword");
            Skip_B2(                                                "crc1");
            BS_Begin();
            Get_S1 (2, fscod,                                       "fscod - Sample Rate Code"); Param_Info2(AC3_SamplingRate[fscod], " Hz");
            Get_S1 (6, frmsizecod,                                  "frmsizecod - Frame Size Code"); if (frmsizecod/2<19) {Param_Info2(AC3_BitRate[frmsizecod/2]*1000, " bps");}
        Element_End0();
        Element_Begin1("bsi");
            Get_S1 (5, bsid,                                        "bsid - Bit Stream Identification");
            Get_S1 (3, bsmod,                                       "bsmod - Bit Stream Mode"); Param_Info1(AC3_Mode[bsmod]);
            Get_S1 (3, acmod,                                       "acmod - Audio Coding Mode"); Param_Info1(AC3_ChannelPositions[acmod]);
            if ((acmod&1) && acmod!=1) //central present
                Skip_S1(2,                                          "cmixlev - Center Mix Level");
            if (acmod&4) //back present
                Skip_S1(2,                                          "surmixlev - Surround Mix Level");
            if (acmod==2)
                Get_S1 (2, dsurmod,                                 "dsurmod - Dolby Surround Mode"); Param_Info1(AC3_Surround[dsurmod]);
            Get_SB (   lfeon,                                       "lfeon - Low Frequency Effects");
            Get_S1 (5, dialnorm,                                    "dialnorm - Dialogue Normalization");
            Get_SB (   compre,                                      "compre - Compression Gain Word Exists");
            if (compre)
                Get_S1 (8, compr,                                   "compr - Compression Gain Word");
            TEST_SB_SKIP(                                           "langcode - Language Code Exists");
                Skip_S1(8,                                          "langcod - Language Code");
            TEST_SB_END();
            TEST_SB_SKIP(                                           "audprodie - Audio Production Information Exists");
                Skip_S1(8,                                          "mixlevel - Mixing Level");
                Skip_S1(2,                                          "roomtyp - Room Type");
            TEST_SB_END();
            if (acmod==0) //1+1 mode
            {
                Get_S1 (5, dialnorm2,                               "dialnorm2 - Dialogue Normalization");
                Get_SB (   compr2e,                                 "compr2e - Compression Gain Word Exists");
                if (compr2e)
                    Get_S1 (8, compr2,                              "compr2 - Compression Gain Word");
                TEST_SB_SKIP(                                       "langcod2e - Language Code Exists");
                    Skip_S1(8,                                      "langcod2 - Language Code");
                TEST_SB_END();
                TEST_SB_SKIP(                                       "audprodi2e - Audio Production Information Exists");
                    Skip_S1(8,                                      "mixlevel2 - Mixing Level");
                    Skip_S1(2,                                      "roomtyp2 - Room Type");
                TEST_SB_END();
            }
            Skip_SB(                                                "copyrightb - Copyright Bit");
            Skip_SB(                                                "origbs - Original Bit Stream");
            TEST_SB_SKIP(                                           "timecod1e");
                Skip_S1(14,                                         "timecod1"); //Note: if timecod is used, change the bitstream parsing for bsid==0x06
            TEST_SB_END();
            TEST_SB_SKIP(                                           "timecod2e");
                Skip_S1(14,                                         "timecod2"); //Note: if timecod is used, change the bitstream parsing for bsid==0x06
            TEST_SB_END();
            TEST_SB_SKIP(                                           "addbsie");
                int8u addbsil;
                Get_S1 (6, addbsil,                                 "addbsil");
                for (int8u Pos=0; Pos<=addbsil; Pos++) //addbsil+1 bytes
                    Skip_S1(8,                                      "addbsi");
            TEST_SB_END();
        Element_End0();
        Element_Begin1("audblk");
            for (int8u Pos=0; Pos<AC3_Channels[acmod]; Pos++)
                Skip_SB(                                            "blksw - Block Switch Flag");
            for (int8u Pos=0; Pos<AC3_Channels[acmod]; Pos++)
                Skip_SB(                                            "dithflag - Dither Flag");
            Get_SB (   dynrnge,                                     "dynrnge - Dynamic Range Gain Word Exists");
            if (dynrnge)
                Get_S1 (8, dynrng,                                  "dynrng - Dynamic Range Gain Word");
            if (acmod==0) //1+1 mode
            {
                Get_SB (   dynrng2e,                                "dynrng2e - Dynamic Range Gain Word Exists");
                if (dynrng2e)
                    Get_S1 (8, dynrng2,                             "dynrng2 - Dynamic Range Gain Word");
            }
            BS_End();
        Element_End0();
        Skip_XX(Element_Size-Element_Offset,                        "audblk(continue)+5*audblk+auxdata+errorcheck");
    }
    else if (bsid>0x0A && bsid<=0x10)
    {
        Element_Begin1("synchinfo");
            Skip_B2(                                               "syncword");
        Element_End0();
        Element_Begin1("bsi");
            BS_Begin();
            size_t Bits_Begin=Data_BS_Remain();
            Get_S1 ( 2, strmtyp,                                    "strmtyp");
            Get_S1 ( 3, substreamid,                                "substreamid");
            Get_S2 (11, frmsiz,                                     "frmsiz");
            Get_S1 ( 2, fscod,                                      "fscod"); Param_Info2(AC3_SamplingRate[fscod], " Hz");
            if (fscod==3)
            {
                Get_S1 ( 2, fscod2,                                 "fscod2"); Param_Info2(AC3_SamplingRate2[fscod2], " Hz");
                numblkscod=3;
            }
            else
                Get_S1 ( 2, numblkscod,                             "numblkscod");
            Get_S1 (3, acmod,                                       "acmod - Audio Coding Mode"); Param_Info1(AC3_ChannelPositions[acmod]);
            Get_SB (   lfeon,                                       "lfeon - Low Frequency Effects");
            Get_S1 (5, bsid,                                        "bsid - Bit Stream Identification");
            Get_S1 (5, dialnorm,                                    "dialnorm");
            TEST_SB_GET(compre,                                     "compre");
                Get_S1 (8, compr,                                   "compr");
            TEST_SB_END();
            if (acmod==0) //1+1 mode
            {
                Get_S1 (5, dialnorm2,                              "dialnorm2");
                TEST_SB_GET(compr2e,                                "compr2e");
                    Get_S1 (8, compr2,                              "compr2");
                TEST_SB_END();
            }
            if (strmtyp==1) //dependent stream
            {
                TEST_SB_GET (chanmape,                              "chanmape");
                    Get_S2(16, chanmap,                             "chanmap"); Param_Info1(AC3_chanmap_ChannelPositions(chanmap));
                TEST_SB_END();
            }
        Element_End0();
        if (Data_BS_Remain()<17)
        {
            BS_End();
            Trusted_IsNot("Not enough data");
        }
        else
        {
            Element_Begin1("errorcheck");
            size_t BitsUpToEndOfFrame=(frmsiz*2)*8-(Bits_Begin-Data_BS_Remain());
            Skip_BS(BitsUpToEndOfFrame-17,                          "bsi(continue)+audfrm+x*audblk+auxdata+errorcheck");
            Skip_SB(                                                "encinfo");
            BS_End();
            Skip_B2(                                                "crc2");
            Element_End0();
        }
    }

    FILLING_BEGIN();
        if (bsid>0x10)
            return; //Not supported

        //Information
        if (strmtyp>1)
            strmtyp=0; //TODO: check a file with strmtyp==2
        if (strmtyp==0)
            substreamid_Independant_Current=substreamid;
        if (bsid_Max==(int8u)-1 || bsid>bsid_Max)
            bsid_Max=bsid;

        //Specific to first frame
        if (Frame_Count==0)
        {
            frmsizplus1_Max[substreamid_Independant_Current][strmtyp+substreamid]=frmsiz+1;
            acmod_Max[substreamid_Independant_Current][strmtyp+substreamid]=acmod;
            lfeon_Max[substreamid_Independant_Current][strmtyp+substreamid]=lfeon;
            bsmod_Max[substreamid_Independant_Current][strmtyp+substreamid]=bsmod;
            dsurmod_Max[substreamid_Independant_Current][strmtyp+substreamid]=dsurmod;
            chanmape_Max[substreamid_Independant_Current][strmtyp+substreamid]=chanmape;
            chanmap_Max[substreamid_Independant_Current][strmtyp+substreamid]=chanmap;

            FirstFrame_Dolby.dialnorm=dialnorm;
            if (compre)
                FirstFrame_Dolby.compr=compr;
            if (dynrnge)
                FirstFrame_Dolby.dynrng=dynrng;
            FirstFrame_Dolby.compre=compre;
            FirstFrame_Dolby.dynrnge=dynrnge;
            if (acmod==0) //1+1 mode
            {
                FirstFrame_Dolby2.dialnorm=dialnorm2;
                if (compr2e)
                    FirstFrame_Dolby2.compr=compr2;
                if (dynrng2e)
                    FirstFrame_Dolby2.dynrng=dynrng2;
                FirstFrame_Dolby2.compre=compr2e;
                FirstFrame_Dolby2.dynrnge=dynrng2e;
            }
        }

        //Stats
        if (dialnorms.empty())
            dialnorms.resize(32);
        dialnorms[dialnorm]++;
        if (compre)
        {
            if (comprs.empty())
                comprs.resize(256);
            comprs[compr]++;
        }
        if (dynrnge)
        {
            //Saving new value
            dynrnge_Exists=true;
            dynrng_Old=dynrng;
        }
        if (!dynrnge)
            dynrng=0;
        if (dynrngs.empty())
            dynrngs.resize(256);
        dynrngs[dynrng]++;
        if (acmod==0) //1+1 mode
        {
            if (dialnorm2s.empty())
                dialnorm2s.resize(32);
            dialnorm2s[dialnorm2]++;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Ac3::HD()
{
    //Parsing
    int32u Synch;
    Peek_B3(Synch);
    if (Synch==0xF8726F)
    {
        if (Buffer_Offset+28>Buffer_Size)
        {
            Trusted_IsNot("Not enough data");
            return; //Need more data
        }

        //Testing
        /* Not working
        int16u CRC_16_Table_HD[256];
        CRC16_Init(CRC_16_Table_HD, 0x002D);

        int16u CRC_16=0x0000;
        const int8u* CRC_16_Buffer=Buffer+Buffer_Offset;
        while(CRC_16_Buffer<Buffer+Buffer_Offset+24)
        {
            CRC_16=(CRC_16<<8) ^ CRC_16_Table_HD[(CRC_16>>8)^(*CRC_16_Buffer)];
            CRC_16_Buffer++;
        }
        CRC_16^=LittleEndian2int16u(Buffer+Buffer_Offset+24);
        */

        Element_Begin1("MajorSync");
        Skip_B3(                                                "Synch");
        Get_B1 (HD_StreamType,                                  "Stream type"); Param_Info1(AC3_HD_StreamType(HD_StreamType));

        if (HD_StreamType==0xBA)
        {
            BS_Begin();
            Get_S1 ( 4, HD_SamplingRate1,                       "Sampling rate"); Param_Info2(AC3_HD_SamplingRate(HD_SamplingRate1), " Hz");
            Skip_S1( 8,                                         "Unknown");
            Get_S1 ( 5, HD_Channels1,                           "Channels (1st substream)"); Param_Info1(AC3_TrueHD_Channels(HD_Channels1)); Param_Info1(Ztring().From_Local(AC3_TrueHD_Channels_Positions(HD_Channels1)));
            Skip_S1( 2,                                         "Unknown");
            Get_S2 (13, HD_Channels2,                           "Channels (2nd substream)"); Param_Info1(AC3_TrueHD_Channels(HD_Channels2)); Param_Info1(Ztring().From_Local(AC3_TrueHD_Channels_Positions(HD_Channels2)));
            BS_End();
            HD_Resolution2=HD_Resolution1=24; //Not sure
            HD_SamplingRate2=HD_SamplingRate1;
        }
        else if (HD_StreamType==0xBB)
        {
            BS_Begin();
            Get_S1 ( 4, HD_Resolution1,                         "Resolution1"); Param_Info2(AC3_MLP_Resolution[HD_Resolution1], " bits");
            Get_S1 ( 4, HD_Resolution2,                         "Resolution2"); Param_Info2(AC3_MLP_Resolution[HD_Resolution2], " bits");
            Get_S1 ( 4, HD_SamplingRate1,                       "Sampling rate"); Param_Info2(AC3_HD_SamplingRate(HD_SamplingRate1), " Hz");
            Get_S1 ( 4, HD_SamplingRate2,                       "Sampling rate"); Param_Info2(AC3_HD_SamplingRate(HD_SamplingRate2), " Hz");
            Skip_S1(11,                                         "Unknown");
            Get_S1 ( 5, HD_Channels1,                           "Channels"); Param_Info1(AC3_MLP_Channels[HD_Channels1]);
            BS_End();
            HD_Channels2=HD_Channels1;
        }
        else
        {
            Skip_XX(Element_Size-Element_Offset,                "Data");
            return;
        }

        Skip_B6(                                                "Unknown");
        BS_Begin();
        Get_SB (    HD_IsVBR,                                   "Is VBR");
        Get_S2 (15, HD_BitRate_Max,                             "Maximum bitrate"); Param_Info2((HD_BitRate_Max*(AC3_HD_SamplingRate(HD_SamplingRate2)?AC3_HD_SamplingRate(HD_SamplingRate2):AC3_HD_SamplingRate(HD_SamplingRate1))+8)>>4, " bps");
        Get_S1 ( 4, HD_SubStreams_Count,                        "SubStreams_Count");
        Skip_S1( 4,                                             "Unknown");
        BS_End();
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Element_End0();

        FILLING_BEGIN();
            HD_MajorSync_Parsed=true;

            if (HD_SubStreams_Count==1 && HD_StreamType==0xBB) //MLP with only 1 stream
            {
                HD_Resolution2=HD_Resolution1;
                HD_SamplingRate2=HD_SamplingRate1;
            }
        FILLING_END();
    }

    /*
    if (HD_MajorSync_Parsed)
    {
        Element_Begin1("Sizes");
        std::vector<int16u> Sizes;
        for (int8u Pos=0; Pos<HD_SubStreams_Count; Pos++)
        {
            Element_Begin1("Size");
            int16u Size;
            bool HD_Unknown1_Present, HD_NoRestart, HD_ExtraParity;
            BS_Begin();
            Get_SB (    HD_Unknown1_Present,                    "Unknown present"); //Only TrueHD
            Get_SB (    HD_NoRestart,                           "No restart"); //Not present if MajorSync, present if no MajorSync
            Get_SB (    HD_ExtraParity,                         "Extra parity information");
            Skip_SB(                                            "Unknown");
            Get_S2 (12, Size,                                   "Size");
            BS_End();
            if (HD_Unknown1_Present)
                Skip_B2(                                        "Unknown");
            Sizes.push_back(Size);
            Element_End0();
        }
        Element_End0();

        int64u Element_Offset_Begin=Element_Offset;
        for (int8u Pos=0; Pos<HD_SubStreams_Count; Pos++)
        {
            Element_Begin1("Block");
            bool DecodingParameterBlockPresent;
            BS_Begin();
            Get_SB (DecodingParameterBlockPresent,              "Decoding parameter block is present");
            if (DecodingParameterBlockPresent)
            {
                TEST_SB_SKIP(                                   "Restart header");
                    int16u SyncWord;
                    int8u max_matrix_channel;
                    Get_S2(13, SyncWord,                        "SyncWord");
                    if (SyncWord==0x18F5)
                    {
                        Skip_SB(                                "noise_type"); //Only for TrueHD
                        Skip_S2(16,                             "Output timestamp");
                        Skip_S1( 4,                             "min_channel");
                        Skip_S1( 4,                             "max_channel");
                        Get_S1 ( 4, max_matrix_channel,         "max_matrix_channel");
                        Skip_S1( 4,                             "noise_shift");
                        Skip_S3(23,                             "noisegen_seed");
                        Skip_S3(19,                             "unknown");
                        Skip_SB(                                "data_check_present");
                        Skip_S1( 8,                             "lossless_check");
                        Skip_S2(16,                             "unknown");
                        for (int8u matrix_channel=0; matrix_channel<max_matrix_channel; matrix_channel++)
                            Skip_S1(6,                          "ch_assign");
                        Skip_S1( 8,                             "checksum");
                    }
                TEST_SB_END();
            }
            BS_End();
            Skip_XX(Element_Offset_Begin+Sizes[Pos]*2-Element_Offset, "Data");
            Element_End0();
        }
    }
    else
        Skip_XX(Element_Size-Element_Offset,                    "Waiting for MajorSync...");
    */
        Skip_XX(Element_Size-Element_Offset,                    "(Data)");

    FILLING_BEGIN_PRECISE();
        if (Frame_Count==0)
            PTS_Begin=FrameInfo.PTS;
        Frame_Count++;
        Frame_Count_InThisBlock++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        FrameInfo.DUR=833333;
        int64u HD_SamplingRate=AC3_HD_SamplingRate(HD_SamplingRate1);
        if (HD_SamplingRate && HD_SamplingRate!=48000)
        {
            FrameInfo.DUR*=48000;
            FrameInfo.DUR/=HD_SamplingRate;
        }
        if (FrameInfo.DTS!=(int64u)-1)
            FrameInfo.DTS+=FrameInfo.DUR;
        if (FrameInfo.PTS!=(int64u)-1)
            FrameInfo.PTS=FrameInfo.DTS;

        //Filling
        if (!Status[IsAccepted])
        {
            Accept("AC-3");
            if (Frame_Count_Valid<10000)
                Frame_Count_Valid*=32;
        }
        if (!Status[IsFilled] && !Core_IsPresent && Frame_Count>=Frame_Count_Valid)
        {
            Fill("AC-3");

            //No more need data
            if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1)
                Finish("AC-3");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Ac3::TimeStamp()
{
    //Parsing
    int8u H1, H2, M1, M2, S1, S2, F1, F2;
    Skip_B1(                                                    "Magic value");
    Skip_B1(                                                    "Size?");
    BS_Begin();
    Skip_S1(8,                                                  "H");
    Get_S1 (4, H1,                                              "H");
    Get_S1( 4, H2,                                              "H");
    Skip_S1(8,                                                  "M");
    Get_S1 (4, M1,                                              "M");
    Get_S1( 4, M2,                                              "M");
    Skip_S1(8,                                                  "S");
    Get_S1 (4, S1,                                              "S");
    Get_S1( 4, S2,                                              "S");
    Skip_S1(8,                                                  "F");
    Get_S1 (4, F1,                                              "F");
    Get_S1( 4, F2,                                              "F");
    BS_End();
    Skip_B2(                                                    "Unknown");
    Skip_B2(                                                    "Unknown");
    Skip_B2(                                                    "Unknown (fixed)");

    FILLING_BEGIN();
        float64 Temp=H1*10*60*60
                   + H2   *60*60
                   + M1   *10*60
                   + M2      *60
                   + S1      *10
                   + S2
                   + (F1*10+F2)/29.97; //No idea about where is the frame rate
        #ifdef MEDIAINFO_TRACE
            Element_Info1(Ztring::ToZtring(H1)+Ztring::ToZtring(H2)+__T(':')
                       + Ztring::ToZtring(M1)+Ztring::ToZtring(M2)+__T(':')
                       + Ztring::ToZtring(S1)+Ztring::ToZtring(S2)+__T(':')
                       + Ztring::ToZtring(F1)+Ztring::ToZtring(F2));
        #endif //MEDIAINFO_TRACE
        if (Frame_Count==0)
            TimeStamp_Content=Temp;
        TimeStamp_IsParsing=false;
        TimeStamp_Parsed=true;

        if (!TimeStamp_DropFrame_IsValid && M2 && !S2 && !S1 && !F1)
        {
            //If drop frame configuration, frames 0 and 1 are dropped for every minutes except 00 10 20 30 40 50
            switch (F2)
            {
                case 0 :
                case 1 :
                            TimeStamp_DropFrame_IsValid=true;
                            TimeStamp_DropFrame_Content=false;
                            break;
                case 2 :
                            if (Frame_Count>=2)
                            {
                                TimeStamp_DropFrame_IsValid=true;
                                TimeStamp_DropFrame_Content=true;
                            }
                default:    ;
            }

            if (TimeStamp_DropFrame_IsValid)
                Frame_Count_Valid=32; //Reset
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Ac3::dac3()
{
    BS_Begin();
    Get_S1 (2, fscod,                                           "fscod");
    Get_S1 (5, bsid,                                            "bsid");
    Get_S1 (3, bsmod_Max[0][0],                                 "bsmod");
    Get_S1 (3, acmod_Max[0][0],                                 "acmod");
    Get_SB (   lfeon_Max[0][0],                                 "lfeon");
    Get_S1 (5, frmsizecod,                                      "bit_rate_code"); frmsizecod*=2;
    Skip_S1(5,                                                  "reserved");
    BS_End();

    MustParse_dac3=false;
    dxc3_Parsed=true;
}

//---------------------------------------------------------------------------
void File_Ac3::dec3()
{
    //Parsing
    BS_Begin();
    int8u num_ind_sub;
    Skip_S2(13,                                                 "data_rate");
    Get_S1 ( 3, num_ind_sub,                                    "num_ind_sub");
    for (int8u Pos=0; Pos<=num_ind_sub; Pos++)
    {
        Element_Begin1("independent substream");
        int8u num_dep_sub;
        Get_S1 (2, fscod,                                       "fscod");
        Get_S1 (5, bsid,                                        "bsid");
        Get_S1 (3, bsmod_Max[Pos][0],                           "bsmod");
        Get_S1 (3, acmod_Max[Pos][0],                           "acmod");
        Get_SB (   lfeon_Max[Pos][0],                           "lfeon");
        Skip_S1(3,                                              "reserved");
        Get_S1 (4, num_dep_sub,                                 "num_dep_sub");
        if (num_dep_sub>0)
            Skip_S2(9,                                          "chan_loc");
        else
            Skip_SB(                                            "reserved");
        Element_End0();
    }
    BS_End();

    MustParse_dec3=false;
    dxc3_Parsed=true;
}

//---------------------------------------------------------------------------
bool File_Ac3::FrameSynchPoint_Test()
{
    if (Save_Buffer)
        return true; //Test already made by Synchronize()

    if (Buffer[Buffer_Offset  ]==0x0B
     && Buffer[Buffer_Offset+1]==0x77) //AC-3
    {
        bsid=CC1(Buffer+Buffer_Offset+5)>>3;
        int16u  Size=0;
        if (bsid<=0x09)
        {
            int8u fscod     =(CC1(Buffer+Buffer_Offset+4)>>6)&0x03;
            int8u frmsizecod=(CC1(Buffer+Buffer_Offset+4)   )&0x3F;
            Size=AC3_FrameSize_Get(frmsizecod, fscod);
        }
        else if (bsid>0x0A && bsid<=0x10)
        {
            int16u frmsiz=CC2(Buffer+Buffer_Offset+2)&0x07FF;
            Size=2+frmsiz*2;
        }
        if (Size>=6)
        {
            if (Buffer_Offset+Size>Buffer_Size)
                return false; //Need more data
            if (CRC_Compute(Size))
            {
                Synched=true;
                return true;
            }
        }
    }

    if (Buffer[Buffer_Offset+0]==0x77
     && Buffer[Buffer_Offset+1]==0x0B) //AC-3 LE
    {
        bsid=CC1(Buffer+Buffer_Offset+4)>>3;
        int16u  Size=0;
        if (bsid<=0x09)
        {
            int8u fscod     =(CC1(Buffer+Buffer_Offset+5)>>6)&0x03;
            int8u frmsizecod=(CC1(Buffer+Buffer_Offset+5)   )&0x3F;
            Size=AC3_FrameSize_Get(frmsizecod, fscod);
        }
        else if (bsid>0x0A && bsid<=0x10)
        {
            int16u frmsiz=LittleEndian2int16u(Buffer+Buffer_Offset+2)&0x07FF;
            Size=2+frmsiz*2;
        }
        if (Size>=6)
        {
            if (Buffer_Offset+Size>Buffer_Size)
                return false; //Need more data

            Save_Buffer=Buffer;
            Save_Buffer_Offset=Buffer_Offset;
            Save_Buffer_Size=Buffer_Size;

            //Exception handling
            try
            {
                int8u* Buffer_Little=new int8u[Size];
                for (size_t Pos=0; Pos+1<Size; Pos+=2)
                {
                    Buffer_Little[Pos+1]=Save_Buffer[Buffer_Offset+Pos  ];
                    Buffer_Little[Pos  ]=Save_Buffer[Buffer_Offset+Pos+1];
                }
                Buffer=Buffer_Little;
                Buffer_Offset=0;
                Buffer_Size=Size;

                Synched=CRC_Compute(Size);

                if (Synched)
                {
                    BigEndian=false;

                    swap(Buffer, Save_Buffer);
                    swap(Buffer_Offset, Save_Buffer_Offset);
                    swap(Buffer_Size, Save_Buffer_Size);

                    return true;
                }

                delete[] Buffer_Little;
            }
            catch(...)
            {
            }
            Buffer=Save_Buffer; Save_Buffer=NULL;
            Buffer_Offset=Save_Buffer_Offset;
            Buffer_Size=Save_Buffer_Size;
        }
    }

    if (HD_MajorSync_Parsed
     || (Buffer[Buffer_Offset+4]==0xF8
      && Buffer[Buffer_Offset+5]==0x72
      && Buffer[Buffer_Offset+6]==0x6F
      && (Buffer[Buffer_Offset+7]&0xFE)==0xBA)) //TrueHD or MLP
    {
        HD_IsPresent=true;
        Synched=true;
        return true;
    }

    Synched=false;
    return true;
}

//---------------------------------------------------------------------------
bool File_Ac3::CRC_Compute(size_t Size)
{
    //Config
    if (!IgnoreCrc_Done)
    {
        IgnoreCrc=Config->File_Ac3_IgnoreCrc_Get();
        IgnoreCrc_Done=true;
    }
    if (IgnoreCrc && !Status[IsAccepted]) //Else there are some wrong synchronizations
    {
        MediaInfo_Internal MI;
        Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
        Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
        MI.Option(__T("ParseSpeed"), __T("0"));
        MI.Option(__T("Demux"), Ztring());
        size_t MiOpenResult=MI.Open(File_Name);
        MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
        MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
        if (MiOpenResult)
        {
            Ztring Format=MI.Get(Stream_General, 0, General_Format);
            if (Format!=__T("AC-3") && Format!=__T("E-AC-3"))
                IgnoreCrc=false;
        }
        else
            IgnoreCrc=false; // Problem
    }
    if (IgnoreCrc)
        return true;

    int16u CRC_16=0x0000;
    const int8u* CRC_16_Buffer=Buffer+Buffer_Offset+2; //After syncword
    const int8u* CRC_16_Buffer_5_8=Buffer+Buffer_Offset+(((Size>>2)+(Size>>4))<<1); //Magic formula to meet 5/8 frame size from Dolby
    const int8u* CRC_16_Buffer_EndMinus3=Buffer+Buffer_Offset+Size-3; //End of frame minus 3
    const int8u* CRC_16_Buffer_End=Buffer+Buffer_Offset+Size; //End of frame
    while(CRC_16_Buffer<CRC_16_Buffer_End)
    {
        CRC_16=(CRC_16<<8) ^ CRC_16_Table[(CRC_16>>8)^(*CRC_16_Buffer)];

        //CRC bytes inversion
        if (CRC_16_Buffer==CRC_16_Buffer_EndMinus3 && bsid<=0x09 && ((*CRC_16_Buffer)&0x01)) //CRC inversion bit
        {
            CRC_16_Buffer++;
            CRC_16=(CRC_16<<8) ^ CRC_16_Table[(CRC_16>>8)^((int8u)(~(*CRC_16_Buffer)))];
            CRC_16_Buffer++;
            CRC_16=(CRC_16<<8) ^ CRC_16_Table[(CRC_16>>8)^((int8u)(~(*CRC_16_Buffer)))];
        }

        CRC_16_Buffer++;

        //5/8 intermediate test
        if (CRC_16_Buffer==CRC_16_Buffer_5_8 && bsid<=0x09 && CRC_16!=0x0000)
            break;
    }

    return (CRC_16==0x0000);
}

//---------------------------------------------------------------------------
size_t File_Ac3::Core_Size_Get()
{
    int16u Size=1;
    bsid=(Buffer[(size_t)(Buffer_Offset+5)]&0xF8)>>3;
    if (bsid<=0x09)
    {
        fscod     =(Buffer[(size_t)(Buffer_Offset+4)]&0xC0)>>6;
        frmsizecod= Buffer[(size_t)(Buffer_Offset+4)]&0x3F;

        //Filling
        fscods[fscod]++;
        frmsizecods[frmsizecod]++;
        Size=AC3_FrameSize_Get(frmsizecod, fscod);
    }
    else if (bsid>0x0A && bsid<=0x10)
    {
        int16u frmsiz    =((int16u)(Buffer[(size_t)(Buffer_Offset+2)]&0x07)<<8)
                        | (         Buffer[(size_t)(Buffer_Offset+3)]         );

        //Filling
        Size=2+frmsiz*2;

        substreams_Count=0;
        int8u substreams_Count_Independant=0;
        int8u substreams_Count_Dependant=0;
        for (;;)
        {
            if (Buffer_Offset+Size+6>Buffer_Size)
            {
                if (!IsSub && !Save_Buffer)
                    Element_WaitForMoreData();
                break;
            }

            int8u bsid=Buffer[Buffer_Offset+Size+5]>>3;
            if (bsid<=0x09 || bsid>0x10)
                break; //Not E-AC-3

            int8u substreamid=(Buffer[Buffer_Offset+Size+2]>>3)&0x07;
            if (substreamid!=substreams_Count_Independant)
                break; //Problem
            if (substreamid!=substreams_Count_Dependant)
                break; //Problem

            frmsiz    =((int16u)(Buffer[(size_t)(Buffer_Offset+Size+2)]&0x07)<<8)
                     | (         Buffer[(size_t)(Buffer_Offset+Size+3)]         );

            //Filling
            Size+=2+frmsiz*2;

            int8u strmtyp = Buffer[Buffer_Offset + Size + 2] >> 6;
            if (strmtyp == 0)
                substreams_Count_Independant++;
            else
                substreams_Count_Dependant++;
            substreams_Count++;
        }
    }

    return Size;
}

//---------------------------------------------------------------------------
size_t File_Ac3::HD_Size_Get()
{
    size_t Size=BigEndian2int16u(Buffer+Buffer_Offset)&0x0FFF;
    Size*=2;

    return Size;
}

} //NameSpace

#endif //MEDIAINFO_AC3_YES
