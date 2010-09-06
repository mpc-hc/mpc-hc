// File_Ac3 - Info for AC3 files
// Copyright (C) 2004-2010 MediaArea.net SARL, Info@MediaArea.net
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
extern const char*  AC3_Mode[]=
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
extern const char*  AC3_Surround[]=
{
    "",
    "(No surround)",
    "(Surround)",
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
    "Front: L R",
    "Front: C",
    "Front: L R",
    "Front: L C R",
    "Front: L R,   Side: C",
    "Front: L C R, Side: C",
    "Front: L R,   Side: L R",
    "Front: L C R, Side: L R",
};

//---------------------------------------------------------------------------
const char*  AC3_ChannelPositions2[]=
{
    "2/0/0",
    "1/0/0",
    "2/0/0",
    "3/0/0",
    "2/1/0",
    "3/1/0",
    "2/2/0",
    "3/2/0",
};

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
    Text+=_T('/')+Ztring::ToZtring(Surround);
    Text+=_T('/')+Ztring::ToZtring(Rear);
    Text+=_T('.')+Ztring::ToZtring(LFE);
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
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;
    PTS_DTS_Needed=true;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:2;
    MustParse_dac3=false;
    MustParse_dec3=false;
    CalculateDelay=false;

    //Temp
    HD_Count=0;
    chanmap=0;
    frmsiz=0;
    fscod=0;
    fscod2=0;
    frmsizecod=0;
    bsid=0;
    bsmod=0;
    acmod=0;
    dsurmod=0;
    numblks=0;
    lfeon=false;
    dxc3_Parsed=false;
    HD_MajorSync_Parsed=false;
    HD_AlreadyCounted=false;
    Core_IsPresent=false;
    dynrnge_Exists=false;
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
            Fill(Stream_General, 0, General_Format_Profile, "TrueHD");
            Fill(Stream_General, 0, General_Format_Profile, "Core");
            Fill(Stream_Audio, 0, Audio_Format_Profile, "TrueHD");
            Fill(Stream_Audio, 0, Audio_Format_Profile, "Core");
            Fill(Stream_Audio, 0, Audio_Codec, "TrueHD");
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
            Fill(Stream_Audio, 0, Audio_BitRate, "Variable");
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_HD_SamplingRate(HD_SamplingRate1));
            Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_TrueHD_Channels(HD_Channels2));
            Fill(Stream_Audio, 0, Audio_ChannelPositions, AC3_TrueHD_Channels_Positions(HD_Channels2));
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, AC3_TrueHD_Channels_Positions2(HD_Channels2));
            if (!IsSub)
                Fill(Stream_Audio, 0, Audio_MuxingMode, "After core data");
        }

        if (HD_StreamType==0xBB) //TrueHD
        {
            Fill(Stream_General, 0, General_Format, "MLP");
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_HD_SamplingRate(HD_SamplingRate2));
            if (HD_SamplingRate1!=HD_SamplingRate2)
                Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_HD_SamplingRate(HD_SamplingRate2));
            Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_MLP_Channels[HD_Channels1]);
            if (HD_Channels1!=HD_Channels2)
                Fill(Stream_Audio, 0, Audio_Channel_s_, AC3_MLP_Channels[HD_Channels1]);
            Fill(Stream_Audio, 0, Audio_Resolution, AC3_MLP_Resolution[HD_Resolution2]);
            if (HD_Resolution1!=HD_Resolution2)
                Fill(Stream_Audio, 0, Audio_Resolution, AC3_MLP_Resolution[HD_Resolution1]);
        }
    }

    //MLP
    if (!Core_IsPresent)
    {
        Fill(Stream_Audio, 0, Audio_Format, "MLP");
        Fill(Stream_Audio, 0, Audio_Codec,  "MLP");
    }

    //AC-3
    else if (bsid<=0x08)
    {
        if (Count_Get(Stream_Audio)==0)
        {
            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Codec, "AC3");
        }
        Fill(Stream_Audio, 0, Audio_Format, "AC-3");
        Fill(Stream_Audio, 0, Audio_Resolution, 16);

        if (Ztring::ToZtring(AC3_SamplingRate[fscod])!=Retrieve(Stream_Audio, 0, Audio_SamplingRate))
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_SamplingRate[fscod]);
        if (frmsizecod/2<19)
        {
            int32u BitRate=AC3_BitRate[frmsizecod/2]*1000;
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
            if (CalculateDelay && Buffer_TotalBytes_FirstSynched>100 && BitRate>0)
            {
                Fill(Stream_Audio, 0, Audio_Delay, (float)Buffer_TotalBytes_FirstSynched*8*1000/BitRate, 0);
                Fill(Stream_Audio, 0, Audio_Delay_Source, "Stream");
            }
        }

        if (acmod==0)
            Fill(Stream_Audio, 0, Audio_Format_Settings_Mode, "Dual Mono");
        Fill(Stream_Audio, 0, Audio_Format_Settings_ModeExtension, AC3_Mode[bsmod]);
        int8u Channels=AC3_Channels[acmod];
        Ztring ChannelPositions; ChannelPositions.From_Local(AC3_ChannelPositions[acmod]);
        Ztring ChannelPositions2; ChannelPositions2.From_Local(AC3_ChannelPositions2[acmod]);
        if (lfeon)
        {
            Channels+=1;
            ChannelPositions+=_T(", LFE");
            ChannelPositions2+=_T(".1");
        }
        if (Ztring::ToZtring(Channels)!=Retrieve(Stream_Audio, 0, Audio_Channel_s_))
            Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        if (ChannelPositions!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions))
            Fill(Stream_Audio, 0, Audio_ChannelPositions, ChannelPositions);
        if (ChannelPositions2!=Retrieve(Stream_Audio, 0, Audio_ChannelPositions_String2))
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, ChannelPositions2);
        if (dsurmod==2)
        {
            Fill(Stream_Audio, 0, Audio_Format_Profile, "Dolby Digital");
            Fill(Stream_Audio, 0, Audio_Codec_Profile, "Dolby Digital");
        }
        if (_T("CBR")!=Retrieve(Stream_Audio, 0, Audio_BitRate_Mode))
            Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
    }

    //E-AC-3
    else if (bsid==0x10)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "E-AC-3");
        Fill(Stream_Audio, 0, Audio_Codec, "AC3+");

        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        if (numblks>0)
            Fill(Stream_Audio, 0, Audio_BitRate, ((frmsiz*2+2)*8*(750/numblks))/4);

        if (fscod!=2)
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_SamplingRate[fscod]);
        else
            Fill(Stream_Audio, 0, Audio_SamplingRate, AC3_SamplingRate2[fscod2]);

        if (chanmap==0)
        {
            if (acmod==0)
            {
                Fill(Stream_Audio, 0, Audio_Format_Profile, "Dual Mono");
                Fill(Stream_Audio, 0, Audio_Codec_Profile, "Dual Mono");
            }
            int8u Channels=AC3_Channels[acmod];
            Ztring ChannelPositions; ChannelPositions.From_Local(AC3_ChannelPositions[acmod]);
            if (lfeon)
            {
                Channels+=1;
                ChannelPositions+=_T(", LFE");
            }
            Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
            Fill(Stream_Audio, 0, Audio_ChannelPositions, ChannelPositions);
        }
    }

    if (HD_MajorSync_Parsed)
    {
        //Filling Maximum bitrate with the constant core bitrate for better coherancy
        ZtringList List;
        List.Separator_Set(0, _T(" / "));
        List.Write(Retrieve(Stream_Audio, 0, Audio_BitRate));
        if (List.size()>=2)
            Fill(Stream_Audio, 0, Audio_BitRate_Maximum, List[1]);
    }

    //Dolby Metadata
    Fill(Stream_Audio, 0, "dialnorm", FirstFrame_Dolby.dialnorm==0?-31:-FirstFrame_Dolby.dialnorm);
    (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm"), Info_Options)=_T("N NT");
    Fill(Stream_Audio, 0, "dialnorm/String", Ztring::ToZtring(FirstFrame_Dolby.dialnorm==0?-31:-FirstFrame_Dolby.dialnorm)+_T(" dB"));
    (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm/String"), Info_Options)=_T("N NT");
    if (FirstFrame_Dolby.compre)
    {
        float64 Value=AC3_compr[FirstFrame_Dolby.compr>>4]+20*std::log10(((float)(0x10+(FirstFrame_Dolby.compr&0x0F)))/32);
        Fill(Stream_Audio, 0, "compr", Value, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr/String", Ztring::ToZtring(Value, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr/String"), Info_Options)=_T("N NT");
    }
    if (FirstFrame_Dolby.dynrnge)
    {
        float64 Value;
        if (FirstFrame_Dolby.dynrng==0)
            Value=0; //Special case in the formula
        else
            Value=AC3_dynrng[FirstFrame_Dolby.dynrng>>5]+20*std::log10(((float)(0x20+(FirstFrame_Dolby.dynrng&0x1F)))/64);
        Fill(Stream_Audio, 0, "dynrng", Value, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng/String", Ztring::ToZtring(Value, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng/String"), Info_Options)=_T("N NT");
    }
    Fill(Stream_Audio, 0, "bsid", bsid);
    (*Stream_More)[Stream_Audio][0](Ztring().From_Local("bsid"), Info_Options)=_T("N NT");
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
        float64 Average_dB=log10(Sum_Intensity/Count)*10;
        Fill(Stream_Audio, 0, "dialnorm_Average", Average_dB, 0);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Average"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dialnorm_Average/String", Ztring::ToZtring(Average_dB, 0)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Average/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dialnorm_Minimum", -Minimum_Raw);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Minimum"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dialnorm_Minimum/String", Ztring::ToZtring(-Minimum_Raw)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Minimum/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dialnorm_Maximum", -Maximum_Raw);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Maximum"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dialnorm_Maximum/String", Ztring::ToZtring(-Maximum_Raw)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Maximum/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dialnorm_Count", Count);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dialnorm_Count"), Info_Options)=_T("N NT");
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
        float64 Average_dB=log10(Sum_Intensity/Count)*10;
        Fill(Stream_Audio, 0, "compr_Average", Average_dB, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Average"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr_Average/String", Ztring::ToZtring(Average_dB, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Average/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr_Minimum", Minimum_dB, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Minimum"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr_Minimum/String", Ztring::ToZtring(Minimum_dB, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Minimum/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr_Maximum", Maximum_dB, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Maximum"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr_Maximum/String", Ztring::ToZtring(Maximum_dB, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Maximum/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "compr_Count", Count);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("compr_Count"), Info_Options)=_T("N NT");
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
        float64 Average_dB=log10(Sum_Intensity/Count)*10;
        Fill(Stream_Audio, 0, "dynrng_Average", Average_dB, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Average"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng_Average/String", Ztring::ToZtring(Average_dB, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Average/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng_Minimum", Minimum_dB, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Minimum"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng_Minimum/String", Ztring::ToZtring(Minimum_dB, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Minimum/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng_Maximum", Maximum_dB, 2);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Maximum"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng_Maximum/String", Ztring::ToZtring(Maximum_dB, 2)+_T(" dB"));
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Maximum/String"), Info_Options)=_T("N NT");
        Fill(Stream_Audio, 0, "dynrng_Count", Count);
        (*Stream_More)[Stream_Audio][0](Ztring().From_Local("dynrng_Count"), Info_Options)=_T("N NT");
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
        else if (bsid<=8 && frmsizecods.size()==1 && fscods.size()==1)
        {
            int16u Size=AC3_FrameSize_Get(frmsizecods.begin()->first, fscods.begin()->first);
            Frame_Count_ForDuration=(File_Size-File_Offset_FirstSynched)/Size; //Only complete frames
            Fill(Stream_Audio, 0, Audio_StreamSize, Frame_Count_ForDuration*Size);
        }
        if (Frame_Count_ForDuration)
        {
            Fill(Stream_Audio, 0, Audio_Duration, Frame_Count_ForDuration*32);
            Fill(Stream_Audio, 0, Audio_FrameCount, Frame_Count_ForDuration);
        }
    }
    else if (PTS!=(int64u)-1)
    {
        Fill(Stream_Audio, 0, Audio_Duration, float64_int64s(((float64)PTS_End-PTS_Begin)/1000000));
        Fill(Stream_Audio, 0, Audio_FrameCount, float64_int64s(((float64)PTS_End-PTS_Begin)/1000000/32));
    }
}

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
    if (CC4(Buffer)==0x1A45DFA3) //EBML
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
        while (Buffer_Offset+8<=Buffer_Size)
        {
            if (CC2(Buffer+Buffer_Offset)==0x0B77) //AC-3
                break; //while()
            if (CC4(Buffer+Buffer_Offset+4)==0xF8726FBB) //MLP
                break; //while()
            Buffer_Offset++;
        }

        if (Buffer_Offset+8<=Buffer_Size && CC2(Buffer+Buffer_Offset)==0x0B77) //Testing if CRC is coherant
        {
            int8u bsid =CC1(Buffer+Buffer_Offset+5)>>3;
            int16u Size=0;
            if (bsid<=0x08)
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

                //Testing
                int16u CRC_16=0x0000;
                const int8u* CRC_16_Buffer=Buffer+Buffer_Offset+2; //After syncword
                while(CRC_16_Buffer<Buffer+Buffer_Offset+Size)
                {
                    CRC_16=(CRC_16<<8) ^ CRC_16_Table[(CRC_16>>8)^(*CRC_16_Buffer)];
                    CRC_16_Buffer++;
                }
                if (CRC_16!=0x0000)
                    Buffer_Offset++;
                else
                    break;
            }
            else
                Buffer_Offset++;
        }

        if (Buffer_Offset+8<=Buffer_Size && CC4(Buffer+Buffer_Offset+4)==0xF8726FBB) //MLP
        {
            break;
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+8>Buffer_Size)
    {
        if (Buffer_Offset+7==Buffer_Size && CC3(Buffer+Buffer_Offset+4)!=0xF8726F && CC2(Buffer+Buffer_Offset)!=0x0B77)
            Buffer_Offset++;
        if (Buffer_Offset+6==Buffer_Size && CC2(Buffer+Buffer_Offset+4)!=0xF872   && CC2(Buffer+Buffer_Offset)!=0x0B77)
            Buffer_Offset++;
        if (Buffer_Offset+5==Buffer_Size && CC1(Buffer+Buffer_Offset+4)!=0xF8     && CC2(Buffer+Buffer_Offset)!=0x0B77)
            Buffer_Offset++;
        if (Buffer_Offset+4==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0B77)
            Buffer_Offset++;
        if (Buffer_Offset+3==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0B77)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x0B77)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x0B)
            Buffer_Offset++;
        return false;
    }

    //Synched
    Data_Accept("AC-3");
    return true;
}

//---------------------------------------------------------------------------
bool File_Ac3::Synched_Test()
{
    //Specific cases
    if (MustParse_dac3 || MustParse_dec3)
        return true;

    //Must have enough buffer for having header
    if (Buffer_Offset+6>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC2(Buffer+Buffer_Offset)!=0x0B77)
    {
        //MLP or TrueHD CRC, not working
        /*
        int16u CRC_16_Table_HD[256];
        CRC16_Init(CRC_16_Table_HD, 0x002D);

        if (Buffer_Offset+28>Buffer_Size)
            return false; //Need more data

        //Testing
        int16u CRC_16=0x0000;
        const int8u* CRC_16_Buffer=Buffer+Buffer_Offset+4;
        while(CRC_16_Buffer<Buffer+Buffer_Offset+4+24)
        {
            CRC_16=(CRC_16<<8) ^ CRC_16_Table_HD[(CRC_16>>8)^(*CRC_16_Buffer)];
            CRC_16_Buffer++;
        }
        if (CRC_16!=0x0000)
            return false;
        */

        //TrueHD detection
        if ((Frame_Count>=1 && HD_Count+(HD_AlreadyCounted?0:1)==Frame_Count && bsid<=0x08) || !Core_IsPresent)
        {
            Synched=true;
            return true;
        }

        Synched=false;
        return true;
    }

    //AC-3 CRC
    bsid=(Buffer[(size_t)(Buffer_Offset+5)]&0xF8)>>3;
    int16u Size=0;
    if (bsid<=0x08)
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
    if (Size!=0)
    {
        if (Buffer_Offset+Size>Buffer_Size)
            return false; //Need more data

        //Testing
        int16u CRC_16=0x0000;
        const int8u* CRC_16_Buffer=Buffer+Buffer_Offset+2; //After syncword
        while(CRC_16_Buffer<Buffer+Buffer_Offset+Size)
        {
            CRC_16=(CRC_16<<8) ^ CRC_16_Table[(CRC_16>>8)^(*CRC_16_Buffer)];
            CRC_16_Buffer++;
        }
        if (CRC_16!=0x0000)
            Synched=false;
    }
    else
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ac3::Read_Buffer_Continue()
{
    if (MustParse_dac3)
        dac3();
    if (MustParse_dec3)
        dec3();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ac3::Header_Parse()
{
    //MLP or TrueHD specific
    if (CC2(Buffer+Buffer_Offset)!=0x0B77)
    {
        BS_Begin();
        Skip_S1( 4,                                             "Unknown");
        Get_S2 (12, Size,                                       "Size");
        BS_End();
        Skip_B2(                                                "Timestamp?");

        //Filling
        if (Size<2)
        {
            Synched=false;
            Size=2;
        }

        Size*=2;
        Header_Fill_Size(Size);
        Header_Fill_Code(1, "HD");
        return;
    }

    //Testing bsid before parsing
    bsid=(Buffer[(size_t)(Buffer_Offset+5)]&0xF8)>>3;
    if (bsid<=0x08)
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
        frmsiz    =((int16u)(Buffer[(size_t)(Buffer_Offset+2)]&0x07)<<8)
                 | (         Buffer[(size_t)(Buffer_Offset+3)]         );
        fscod     =         (Buffer[(size_t)(Buffer_Offset+4)]&0xC0)>>6;
        int8u numblkscod;
        if (fscod==0x03)
            numblkscod=0x03;
        else
            numblkscod=     (Buffer[(size_t)(Buffer_Offset+4)]&0x30)>>4;

        //Filling
        Size=2+frmsiz*2;
        numblks=numblkscod==3?6:numblkscod+1;
    }
    else
    {
        Reject("AC-3");
        return;
    }


    //Filling
    Header_Fill_Size(Size);
    Header_Fill_Code(0, "syncframe");
}

//---------------------------------------------------------------------------
void File_Ac3::Data_Parse()
{
    //Partial frame
    if (Header_Size+Element_Size<Size)
    {
        Element_Name("Partial frame");
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    //PTS
    if (PTS!=(int64u)-1)
        Element_Info(_T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)(Frame_Count_InThisBlock==0?PTS:PTS_End))/1000000)));

    if (Status[IsFilled])
    {
        if (Element_Code==0 || !Core_IsPresent)
        {
            Frame_Count++;
            Frame_Count_InThisBlock++;
            if (PTS!=(int64u)-1)
            {
                if (Frame_Count_InThisBlock<=1)
                    PTS_End=PTS;
                PTS_End+=32000000;
            }
        }

        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    //Parsing
    switch(Element_Code)
    {
        case 0 : Core(); break;
        case 1 : HD();   break;
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Ac3::Core()
{
    //Parsing
    if (bsid<=0x08)
    {
        int8u  dialnorm, dialnorm2=(int8u)-1, compr=(int8u)-1, compr2=(int8u)-1, dynrng=(int8u)-1, dynrng2=(int8u)-1;
        bool   compre, compr2e=false, dynrnge, dynrng2e=false;
        Element_Begin("synchinfo");
            Skip_B2(                                               "syncword");
            Skip_B2(                                                "crc1");
            BS_Begin();
            Get_S1 (2, fscod,                                       "fscod - Sample Rate Code"); Param_Info(AC3_SamplingRate[fscod], " Hz");
            Get_S1 (6, frmsizecod,                                  "frmsizecod - Frame Size Code"); if (frmsizecod/2<19) {Param_Info(AC3_BitRate[frmsizecod/2]*1000, " bps");}
        Element_End();
        Element_Begin("bsi");
            Get_S1 (5, bsid,                                        "bsid - Bit Stream Identification");
            Get_S1 (3, bsmod,                                       "bsmod - Bit Stream Mode"); Param_Info(AC3_Mode[bsmod]);
            Get_S1 (3, acmod,                                       "acmod - Audio Coding Mode"); Param_Info(AC3_ChannelPositions[acmod]);
            if ((acmod&1) && acmod!=1) //central present
                Skip_S1(2,                                          "cmixlev - Center Mix Level");
            if (acmod&4) //back present
                Skip_S1(2,                                          "surmixlev - Surround Mix Level");
            if (acmod==2)
                Get_S1 (2, dsurmod,                                 "dsurmod - Dolby Surround Mode"); Param_Info(AC3_Surround[dsurmod]);
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
                Skip_S1(14,                                         "timecod1");
            TEST_SB_END();
            TEST_SB_SKIP(                                           "timecod2e");
                Skip_S1(14,                                         "timecod2");
            TEST_SB_END();
            TEST_SB_SKIP(                                           "addbsie");
                int8u addbsil;
                Get_S1 (6, addbsil,                                 "addbsil");
                for (int8u Pos=0; Pos<=addbsil; Pos++) //addbsil+1 bytes
                    Skip_S1(8,                                      "addbsi");
            TEST_SB_END();
        Element_End();
        Element_Begin("audblk");
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
        Element_End();
        Skip_XX(Element_Size-Element_Offset,                        "audblk(continue)+5*audblk+auxdata+errorcheck");

        FILLING_BEGIN();
            //Specific to first frame
            if (Frame_Count==0)
            {
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
    else if (bsid>0x0A && bsid<=0x10)
    {
        Element_Begin("synchinfo");
            Skip_B2(                                               "syncword");
        Element_End();
        Element_Begin("bsi");
            int8u  strmtyp, numblkscod;
            BS_Begin();
            Get_S1 ( 2, strmtyp,                                    "strmtyp");
            Skip_S1( 3,                                             "substreamid");
            Get_S2 (11, frmsiz,                                     "frmsiz");
            Get_S1 ( 2, fscod,                                      "fscod");
            if (fscod==3)
            {
                Get_S1 ( 2, fscod2,                                 "fscod2");
                numblkscod=3;
            }
            else
                Get_S1 ( 2, numblkscod,                             "numblkscod");
            Get_S1 (3, acmod,                                       "acmod - Audio Coding Mode"); Param_Info(AC3_ChannelPositions[acmod]);
            Get_SB (   lfeon,                                       "lfeon - Low Frequency Effects");
            Get_S1 ( 5, bsid,                                       "bsid - Bit Stream Identification");
            TEST_SB_SKIP(                                           "compre");
                Skip_S1(8,                                          "compr");
            TEST_SB_END();
            if (acmod==0) //1+1 mode
            {
                Skip_SB(                                            "dialnorm2");
                TEST_SB_SKIP(                                       "compr2e");
                    Skip_S1(1,                                      "compr2");
                TEST_SB_END();
            }
            if (strmtyp==1) //dependent stream
            {
                TEST_SB_SKIP(                                       "chanmape");
                    Get_S2(16, chanmap,                             "chanmap");
                TEST_SB_END();
            }
            BS_End();
        Element_End();
        Skip_XX(Element_Size-Element_Offset,                        "bsi(continue)+audfrm+x*audblk+auxdata+errorcheck");
    }

    FILLING_BEGIN();
        //Name
        Element_Info(_T("Frame ")+Ztring::ToZtring(Frame_Count));

        //Counting
        if (!Core_IsPresent)
        {
            Frame_Count=0;
            Core_IsPresent=true;
            if (PTS_Begin==(int64u)-1)
                PTS_Begin=PTS;
        }
        if (File_Offset+Buffer_Offset+Element_Size==File_Size)
            Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
        Frame_Count++;
        Frame_Count_InThisBlock++;
        HD_AlreadyCounted=false;
        if (PTS!=(int64u)-1)
        {
            if (Frame_Count_InThisBlock<=1)
                PTS_End=PTS;
            PTS_End+=32000000;
        }

        //Filling
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

        Element_Begin("MajorSync", 28);
        Skip_B3(                                                "Synch");
        Get_B1 (HD_StreamType,                                  "Stream type"); Param_Info(AC3_HD_StreamType(HD_StreamType));

        if (HD_StreamType==0xBA)
        {
            BS_Begin();
            Get_S1 ( 4, HD_SamplingRate1,                       "Sampling rate"); Param_Info(AC3_HD_SamplingRate(HD_SamplingRate1), " Hz");
            Skip_S1( 8,                                         "Unknown");
            Get_S1 ( 5, HD_Channels1,                           "Channels (1st substream)"); Param_Info(AC3_TrueHD_Channels(HD_Channels1)); Param_Info(Ztring().From_Local(AC3_TrueHD_Channels_Positions(HD_Channels1)));
            Skip_S1( 2,                                         "Unknown");
            Get_S2 (13, HD_Channels2,                           "Channels (2nd substream)"); Param_Info(AC3_TrueHD_Channels(HD_Channels2)); Param_Info(Ztring().From_Local(AC3_TrueHD_Channels_Positions(HD_Channels2)));
            BS_End();
            HD_Resolution2=HD_Resolution1=24; //Not sure
            HD_SamplingRate2=HD_SamplingRate1;
        }
        else if (HD_StreamType==0xBB)
        {
            BS_Begin();
            Get_S1 ( 4, HD_Resolution1,                         "Resolution1"); Param_Info(AC3_MLP_Resolution[HD_Resolution1], " bits");
            Get_S1 ( 4, HD_Resolution2,                         "Resolution2"); Param_Info(AC3_MLP_Resolution[HD_Resolution2], " bits");
            Get_S1 ( 4, HD_SamplingRate1,                       "Sampling rate"); Param_Info(AC3_HD_SamplingRate(HD_SamplingRate1), " Hz");
            Get_S1 ( 4, HD_SamplingRate2,                       "Sampling rate"); Param_Info(AC3_HD_SamplingRate(HD_SamplingRate2), " Hz");
            Skip_S1(11,                                         "Unknown");
            Get_S1 ( 5, HD_Channels1,                           "Channels"); Param_Info(AC3_MLP_Channels[HD_Channels1]);
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
        Get_S2 (15, HD_BitRate_Max,                             "Maximum bitrate"); Param_Info((HD_BitRate_Max*(AC3_HD_SamplingRate(HD_SamplingRate2)?AC3_HD_SamplingRate(HD_SamplingRate2):AC3_HD_SamplingRate(HD_SamplingRate1))+8)>>4, " bps");
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
        Element_End();

        FILLING_BEGIN();
            HD_MajorSync_Parsed=true;

            if (HD_SubStreams_Count==1 && HD_StreamType==0xBB) //MLP with only 1 stream
            {
                HD_Resolution2=HD_Resolution1;
                HD_SamplingRate2=HD_SamplingRate1;
            }
        FILLING_END();
    }

    if (HD_MajorSync_Parsed)
    {
        Element_Begin("Sizes");
        std::vector<int16u> Sizes;
        for (int8u Pos=0; Pos<HD_SubStreams_Count; Pos++)
        {
            Element_Begin("Size");
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
            Element_End();
        }
        Element_End();

        int64u Element_Offset_Begin=Element_Offset;
        for (int8u Pos=0; Pos<HD_SubStreams_Count; Pos++)
        {
            Element_Begin("Block", Sizes[Pos]);
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
            Element_End();
        }
    }
    else
        Skip_XX(Element_Size-Element_Offset,                    "Waiting for MajorSync...");

    FILLING_BEGIN_PRECISE();
        if (!Core_IsPresent)
        {
            Frame_Count++;
            Frame_Count_InThisBlock++;
            if (PTS!=(int64u)-1)
            {
                if (PTS_Begin==(int64u)-1)
                    PTS_Begin=PTS;
                if (Frame_Count_InThisBlock<=1)
                    PTS_End=PTS;
                PTS_End+=32000000;
            }
        }

        if (!HD_AlreadyCounted)
        {
            HD_Count++;
            HD_AlreadyCounted=true;
        }

        //Filling
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
void File_Ac3::dac3()
{
    BS_Begin();
    Get_S1 (2, fscod,                                           "fscod");
    Get_S1 (5, bsid,                                            "bsid");
    Get_S1 (3, bsmod,                                           "bsmod");
    Get_S1 (3, acmod,                                           "acmod");
    Get_SB (   lfeon,                                           "lfeon");
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
    for (int8u Pos=0; Pos<num_ind_sub; Pos++)
    {
        Element_Begin("independent substream");
        int8u num_dep_sub;
        Get_S1 (2, fscod,                                       "fscod");
        Get_S1 (5, bsid,                                        "bsid");
        Get_S1 (3, bsmod,                                       "bsmod");
        Get_S1 (3, acmod,                                       "acmod");
        Get_SB (   lfeon,                                       "lfeon");
        Skip_S1(3,                                              "reserved");
        Get_S1 (4, num_dep_sub,                                 "num_dep_sub");
        if (num_dep_sub>0)
            Skip_S2(9,                                          "chan_loc");
        else
            Skip_SB(                                            "reserved");
        Element_End();
    }
    BS_End();

    MustParse_dec3=false;
    dxc3_Parsed=true;
}

} //NameSpace

#endif //MEDIAINFO_AC3_YES

