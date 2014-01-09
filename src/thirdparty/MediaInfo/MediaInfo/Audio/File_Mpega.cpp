/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// A good start : http://www.codeproject.com/audio/MPEGAudioInfo.asp
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

//***************************************************************************
// Constants (Common)
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MPEGA_YES) || defined(MEDIAINFO_MPEGTS_YES) || defined(MEDIAINFO_MPEGPS_YES)
//---------------------------------------------------------------------------

#include "ZenLib/Conf.h"
using namespace ZenLib;

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char* Mpega_Version[4]=
{
    "MPA2.5",
    "",
    "MPA2",
    "MPA1"
};

//---------------------------------------------------------------------------
const char* Mpega_Layer[4]=
{
    "",
    "L3",
    "L2",
    "L1",
};

//---------------------------------------------------------------------------
const char* Mpega_Format_Profile_Version[4]=
{
    "Version 2.5",
    "",
    "Version 2",
    "Version 1"
};

//---------------------------------------------------------------------------
const char* Mpega_Format_Profile_Layer[4]=
{
    "",
    "Layer 3",
    "Layer 2",
    "Layer 1",
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
#if defined(MEDIAINFO_MPEGA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Mpega.h"
#include "ZenLib/BitStream.h"
#include "ZenLib/Utils.h"
#if MEDIAINFO_ADVANCED
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#endif //MEDIAINFO_ADVANCED
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mpega_Version_String[4]=
{
    "MPEG-2.5 Audio",
    "",
    "MPEG-2 Audio",
    "MPEG-1 Audio",
};

//---------------------------------------------------------------------------
const char* Mpega_Layer_String[4]=
{
    "",
    " layer 3",
    " layer 2",
    " layer 1",
};

//---------------------------------------------------------------------------
const int16u Mpega_BitRate[4][4][16]=
{
    {{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //MPEG Audio 2.5 layer X
     {0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160,   0},  //MPEG Audio 2.5 layer 3
     {0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160,   0},  //MPEG Audio 2.5 layer 2
     {0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256,   0}}, //MPEG Audio 2.5 layer 1
    {{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //MPEG Audio X layer X
     {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //MPEG Audio X layer 3
     {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //MPEG Audio X layer 2
     {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}}, //MPEG Audio X layer 1
    {{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //MPEG Audio 2 layer X
     {0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160,   0},  //MPEG Audio 2 layer 3
     {0,   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160,   0},  //MPEG Audio 2 layer 2
     {0,  32,  48,  56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256,   0}}, //MPEG Audio 2 layer 1
    {{0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  //MPEG Audio 1 layer X
     {0,  32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320,   0},  //MPEG Audio 1 layer 3
     {0,  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384,   0},  //MPEG Audio 1 layer 2
     {0,  32,  64,  96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,   0}}, //MPEG Audio 1 layer 1
};

//---------------------------------------------------------------------------
const int16u Mpega_SamplingRate[4][4]=
{
    {11025, 12000,  8000, 0}, //MPEG Audio 2.5
    {    0,     0,     0, 0}, //MPEG Audio X
    {22050, 24000, 16000, 0}, //MPEG Audio 2
    {44100, 48000, 32000, 0}, //MPEG Audio 1
};

//---------------------------------------------------------------------------
const int16u Mpega_Channels[4]=
{
    2,
    2,
    2,
    1,
};

//---------------------------------------------------------------------------
const char* Mpega_Codec_Profile[4]=
{
    "",
    "Joint stereo",
    "Dual mono",
    "",
};

//---------------------------------------------------------------------------
const char* Mpega_Codec_Profile_Extension[]=
{
    "",
    "Intensity Stereo",
    "MS Stereo",
    "Intensity Stereo + MS Stereo",
};

//---------------------------------------------------------------------------
const char* Mpega_Emphasis[]=
{
    "",
    "50/15ms",
    "Reserved",
    "CCITT",
};

//---------------------------------------------------------------------------
const char* Lame_BitRate_Mode[]=
{
    "",
    "CBR",
    "VBR",
    "VBR",
    "VBR",
    "VBR",
    "VBR",
    "",
    "CBR",
    "VBR",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Lame_Method[]=
{
    "",
    "CBR",
    "ABR",
    "VBR (rh)",
    "VBR (mtrh)",
    "VBR (rh)",
    "VBR",
    "",
    "CBR (2-pass)",
    "ABR (2-pass)",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const int8u Mpega_Coefficient[4][4] = //Samples per Frame / 8
{
    {  0,  72, 144,  12}, //MPEG Audio 2.5
    {  0,   0,   0,   0}, //MPEG Audio X
    {  0,  72, 144,  12}, //MPEG Audio 2
    {  0, 144, 144,  12}, //MPEG Audio 1
};

//---------------------------------------------------------------------------
const int8u Mpega_SlotSize[4]= //A frame is coposed of slots
{
    0, // Layer X
    1, // Layer3
    1, // Layer2
    4, // Layer1
};

//---------------------------------------------------------------------------
const int16u Mpega_CRC12_Table[]=
{
  0x000, 0x80f, 0x811, 0x01e, 0x82d, 0x022, 0x03c, 0x833,
  0x855, 0x05a, 0x044, 0x84b, 0x078, 0x877, 0x869, 0x066,
  0x8a5, 0x0aa, 0x0b4, 0x8bb, 0x088, 0x887, 0x899, 0x096,
  0x0f0, 0x8ff, 0x8e1, 0x0ee, 0x8dd, 0x0d2, 0x0cc, 0x8c3,
  0x945, 0x14a, 0x154, 0x95b, 0x168, 0x967, 0x979, 0x176,
  0x110, 0x91f, 0x901, 0x10e, 0x93d, 0x132, 0x12c, 0x923,
  0x1e0, 0x9ef, 0x9f1, 0x1fe, 0x9cd, 0x1c2, 0x1dc, 0x9d3,
  0x9b5, 0x1ba, 0x1a4, 0x9ab, 0x198, 0x997, 0x989, 0x186,
  0xa85, 0x28a, 0x294, 0xa9b, 0x2a8, 0xaa7, 0xab9, 0x2b6,
  0x2d0, 0xadf, 0xac1, 0x2ce, 0xafd, 0x2f2, 0x2ec, 0xae3,
  0x220, 0xa2f, 0xa31, 0x23e, 0xa0d, 0x202, 0x21c, 0xa13,
  0xa75, 0x27a, 0x264, 0xa6b, 0x258, 0xa57, 0xa49, 0x246,
  0x3c0, 0xbcf, 0xbd1, 0x3de, 0xbed, 0x3e2, 0x3fc, 0xbf3,
  0xb95, 0x39a, 0x384, 0xb8b, 0x3b8, 0xbb7, 0xba9, 0x3a6,
  0xb65, 0x36a, 0x374, 0xb7b, 0x348, 0xb47, 0xb59, 0x356,
  0x330, 0xb3f, 0xb21, 0x32e, 0xb1d, 0x312, 0x30c, 0xb03,
  0xd05, 0x50a, 0x514, 0xd1b, 0x528, 0xd27, 0xd39, 0x536,
  0x550, 0xd5f, 0xd41, 0x54e, 0xd7d, 0x572, 0x56c, 0xd63,
  0x5a0, 0xdaf, 0xdb1, 0x5be, 0xd8d, 0x582, 0x59c, 0xd93,
  0xdf5, 0x5fa, 0x5e4, 0xdeb, 0x5d8, 0xdd7, 0xdc9, 0x5c6,
  0x440, 0xc4f, 0xc51, 0x45e, 0xc6d, 0x462, 0x47c, 0xc73,
  0xc15, 0x41a, 0x404, 0xc0b, 0x438, 0xc37, 0xc29, 0x426,
  0xce5, 0x4ea, 0x4f4, 0xcfb, 0x4c8, 0xcc7, 0xcd9, 0x4d6,
  0x4b0, 0xcbf, 0xca1, 0x4ae, 0xc9d, 0x492, 0x48c, 0xc83,
  0x780, 0xf8f, 0xf91, 0x79e, 0xfad, 0x7a2, 0x7bc, 0xfb3,
  0xfd5, 0x7da, 0x7c4, 0xfcb, 0x7f8, 0xff7, 0xfe9, 0x7e6,
  0xf25, 0x72a, 0x734, 0xf3b, 0x708, 0xf07, 0xf19, 0x716,
  0x770, 0xf7f, 0xf61, 0x76e, 0xf5d, 0x752, 0x74c, 0xf43,
  0xec5, 0x6ca, 0x6d4, 0xedb, 0x6e8, 0xee7, 0xef9, 0x6f6,
  0x690, 0xe9f, 0xe81, 0x68e, 0xebd, 0x6b2, 0x6ac, 0xea3,
  0x660, 0xe6f, 0xe71, 0x67e, 0xe4d, 0x642, 0x65c, 0xe53,
  0xe35, 0x63a, 0x624, 0xe2b, 0x618, 0xe17, 0xe09, 0x606
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpega::File_Mpega()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;

    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;
    Frame_Count_NotParsedIncluded=0;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.5?128:(MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:4);
    FrameIsAlwaysComplete=false;
    CalculateDelay=false;

    //Temp - BitStream info
    Surround_Frames=0;
    Block_Count[0]=0;
    Block_Count[1]=0;
    Block_Count[2]=0;
    Channels_Count[0]=0;
    Channels_Count[1]=0;
    Channels_Count[2]=0;
    Channels_Count[3]=0;
    Extension_Count[0]=0;
    Extension_Count[1]=0;
    Extension_Count[2]=0;
    Extension_Count[3]=0;
    Emphasis_Count[0]=0;
    Emphasis_Count[1]=0;
    Emphasis_Count[2]=0;
    Emphasis_Count[3]=0;
    Scfsi=0;
    Scalefac=0;
    Reservoir=0;
    LastSync_Offset=(int64u)-1;
    VBR_FileSize=0;
    VBR_Frames=0;
    Reservoir_Max=0;
    Xing_Scale=0;
    BitRate=0;
    MpegPsPattern_Count=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpega::Streams_Fill()
{
    //VBR detection without header
    if (VBR_Frames==0)
    {
        //How much kinds of bitrates?
        if (BitRate_Count.size()>1)
            BitRate_Mode=__T("VBR");
    }

    File__Tags_Helper::Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "MPEG Audio");
    Fill(Stream_Audio, 0, Audio_Format_Version, Mpega_Format_Profile_Version[ID]);
    Fill(Stream_Audio, 0, Audio_Format_Profile, Mpega_Format_Profile_Layer[layer]);
    if (mode && mode<4)
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Mpega_Codec_Profile[mode]);
        Fill(Stream_Audio, 0, Audio_Format_Settings_Mode, Mpega_Codec_Profile[mode]);
    }
    if (mode_extension && mode_extension<4)
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Mpega_Codec_Profile_Extension[mode_extension]);
        Fill(Stream_Audio, 0, Audio_Format_Settings_ModeExtension, Mpega_Codec_Profile_Extension[mode_extension]);
    }
    if (emphasis && emphasis<4)
    {
        Fill(Stream_Audio, 0, Audio_Format_Settings, Mpega_Emphasis[emphasis]);
        Fill(Stream_Audio, 0, Audio_Format_Settings_Emphasis, Mpega_Emphasis[emphasis]);
    }
    Fill(Stream_Audio, 0, Audio_Codec, Ztring(Mpega_Version[ID])+Ztring(Mpega_Layer[layer]));
    Fill(Stream_Audio, 0, Audio_Codec_String, Ztring(Mpega_Version_String[ID])+Ztring(Mpega_Layer_String[layer]), true);
    Fill(Stream_Audio, 0, Audio_SamplingRate, Mpega_SamplingRate[ID][sampling_frequency]);
    if (mode<4)
    {
        Fill(Stream_Audio, 0, Audio_Channel_s_, Mpega_Channels[mode]);
        Fill(Stream_Audio, 0, Audio_Codec_Profile, Mpega_Codec_Profile[mode]);
    }

    //Bitrate, if CBR
    if (VBR_Frames==0 && BitRate_Mode!=__T("VBR"))
    {
        BitRate_Mode=__T("CBR");
        BitRate=Mpega_BitRate[ID][layer][bitrate_index]*1000;
        Fill(Stream_General, 0, General_OverallBitRate, BitRate);
        Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
        if (CalculateDelay && Buffer_TotalBytes_FirstSynched>10 && BitRate>0)
        {
            Fill(Stream_Audio, 0, Audio_Delay, Buffer_TotalBytes_FirstSynched*8*1000/BitRate, 0);
            Fill(Stream_Audio, 0, Audio_Delay_Source, "Stream");
        }
    }

    //Bitrate mode
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, BitRate_Mode);
    Fill(Stream_Audio, 0, Audio_BitRate_Minimum, BitRate_Minimum);
    Fill(Stream_Audio, 0, Audio_BitRate_Nominal, BitRate_Nominal);

    //Tags
    File__Tags_Helper::Streams_Fill();
}

//---------------------------------------------------------------------------
void File_Mpega::Streams_Finish()
{
    //Reservoir
    //Fill("Reservoir_Avg", Reservoir/Frame_Count);
    //Fill("Reservoir_Max", Reservoir_Max);
    //size_t Granules=(Mpeg==3?2:1);
    //size_t Ch=Mpega_Channels[Channels];
    //Fill("Scalefactors", Ztring::ToZtring(Scalefac*100/(Granules*Ch*Frame_Count))+__T('%'));

    //VBR_FileSize calculation
    if (!IsSub && (File_Size!=(int64u)-1 || LastSync_Offset!=(int64u)-1) && VBR_FileSize==0)
    {
        //We calculate VBR_FileSize from the last synch or File_Size
        if (LastSync_Offset!=(int64u)-1)
        {
            VBR_FileSize=LastSync_Offset;
            VBR_FileSize-=File_BeginTagSize;
        }
        else
        {
            VBR_FileSize=File_Size;
            VBR_FileSize-=File_BeginTagSize;
            VBR_FileSize-=File_EndTagSize;
        }
    }

    //Bitrate calculation if VBR
    int64u FrameCount=0;
    if (VBR_Frames>0)
    {
        FrameCount=VBR_Frames;
        float32 FrameLength=((float32)(VBR_FileSize?VBR_FileSize:File_Size-File_EndTagSize-File_BeginTagSize))/VBR_Frames;
        size_t Divider;
        if (ID==3 && layer==3) //MPEG 1 layer 1
             Divider=384/8;
        else if ((ID==2 || ID==0) && layer==3) ///MPEG 2 or 2.5 layer 1
             Divider=192/8;
        else if ((ID==2 || ID==0) && layer==1) //MPEG 2 or 2.5 layer 3
            Divider=576/8;
        else
            Divider=1152/8;
        if (ID<4 && sampling_frequency<4)
            BitRate=(int32u)(FrameLength*Mpega_SamplingRate[ID][sampling_frequency]/Divider);
        BitRate_Mode=__T("VBR");
    }
    //if (BitRate_Count.size()>1)
    //{
    //    Ztring BitRate_VBR;
    //    if (!BitRate_VBR.empty())
    //        BitRate_VBR+=__T(' ');
    //    BitRate_VBR+=Ztring::ToZtring(8);
    //    BitRate_VBR+=__T(':');
    //    BitRate_VBR+=Ztring::ToZtring(BitRate_Count[8]);
    //    Fill("BitRate_VBR", Ztring::ToZtring(BitRate_Count[8]));
    //}
    if (VBR_FileSize)
    {
        if (BitRate)
        {
            Fill(Stream_General, 0, General_Duration, VBR_FileSize*8*1000/BitRate, 10, true);
            Fill(Stream_General, 0, General_OverallBitRate, BitRate, 10, true);
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate, 10, true);
            if (CalculateDelay && Buffer_TotalBytes_FirstSynched>10 && BitRate>0)
            {
                Fill(Stream_Audio, 0, Audio_Delay, Buffer_TotalBytes_FirstSynched*8*1000/BitRate, 0, true);
                Fill(Stream_Audio, 0, Audio_Delay_Source, "Stream", Unlimited, true, true);
            }
        }
        Fill(Stream_Audio, 0, Audio_StreamSize, VBR_FileSize);
    }
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, BitRate_Mode, true);

    //Encoding library
    if (!Encoded_Library.empty())
        Fill(Stream_General, 0, General_Encoded_Library, Encoded_Library, true);
    if (Encoded_Library.empty())
        Encoded_Library_Guess();
    Fill(Stream_Audio, 0, Audio_Encoded_Library, Encoded_Library, true);
    Fill(Stream_Audio, 0, Audio_Encoded_Library_Settings, Encoded_Library_Settings, true);

    //Surround
    if (Surround_Frames>=Frame_Count*0.9)
    {
        //Fill(Stream_Audio, 0, Audio_Channel_s_, 6);
    }

    if (FrameInfo.PTS!=(int64u)-1 && FrameInfo.PTS>PTS_Begin)
    {
        Fill(Stream_Audio, 0, Audio_Duration, float64_int64s(((float64)(FrameInfo.PTS-PTS_Begin))/1000000));
        if (Retrieve(Stream_Audio, 0, Audio_BitRate_Mode)==__T("CBR"))
        {
            int16u Samples;
            if (ID==3 && layer==3) //MPEG 1 layer 1
                 Samples=384;
            else if ((ID==2 || ID==0) && layer==1) //MPEG 2 or 2.5 layer 3
                Samples=576;
            else
                Samples=1152;

            float64 Frame_Duration=((float64)1)/Mpega_SamplingRate[ID][sampling_frequency]*Samples;
            FrameCount=float64_int64s(((float64)(FrameInfo.PTS-PTS_Begin))/1000000000/Frame_Duration);
        }
    }

    if (FrameCount==0 && VBR_FileSize && Retrieve(Stream_Audio, 0, Audio_BitRate_Mode)==__T("CBR") && Mpega_SamplingRate[ID][sampling_frequency])
    {
        size_t Size=(Mpega_Coefficient[ID][layer]*Mpega_BitRate[ID][layer][bitrate_index]*1000/Mpega_SamplingRate[ID][sampling_frequency])*Mpega_SlotSize[layer];
        if (Size)
            FrameCount=float64_int64s(((float64)VBR_FileSize)/Size);
    }

    if (FrameCount)
    {
        int16u Samples;
        if (ID==3 && layer==3) //MPEG 1 layer 1
             Samples=384;
        else if ((ID==2 || ID==0) && layer==1) //MPEG 2 or 2.5 layer 3
            Samples=576;
        else
            Samples=1152;
        Fill(Stream_Audio, 0, Audio_FrameCount, FrameCount, 10, true);
        Fill(Stream_Audio, 0, Audio_SamplingCount, FrameCount*Samples, 10, true);
    }

    File__Tags_Helper::Streams_Finish();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mpega::FileHeader_Begin()
{
    //Buffer size
    if (Buffer_Size<8)
        return File_Size<8; //Must wait for more data

    //Detecting WAV/SWF/FLV/ELF/DPG/WM/MZ/DLG files
    int32u Magic4=CC4(Buffer);
    int32u Magic3=Magic4>>8;
    int16u Magic2=Magic4>>16;
    if (Magic4==0x52494646 || Magic3==0x465753 || Magic3==0x464C56 || Magic4==0x7F454C46 || Magic4==0x44504730 || Magic4==0x3026B275 || Magic2==0x4D5A || Magic4==0x000001BA || Magic4==0x000001B3 || Magic4==0x00000100 || CC8(Buffer+Buffer_Offset)==0x444C472056312E30LL)
    {
        File__Tags_Helper::Reject("MPEG Audio");
        return false;
    }

    //Seems OK
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mpega::Synchronize()
{
    //Tags
    bool Tag_Found_Begin;
    if (!File__Tags_Helper::Synchronize(Tag_Found_Begin))
        return false;
    if (Tag_Found_Begin)
        return true;

    //Synchronizing
    while (Buffer_Offset+4<=Buffer_Size)
    {
        while (Buffer_Offset+4<=Buffer_Size)
        {
            if (Buffer[Buffer_Offset  ]==0xFF
             && (Buffer[Buffer_Offset+1]&0xE0)==0xE0
             && (Buffer[Buffer_Offset+2]&0xF0)!=0xF0
             && (Buffer[Buffer_Offset+2]&0x0C)!=0x0C)
                break; //while()

            //Tags
            bool Tag_Found_Synchro;
            if (!File__Tags_Helper::Synchronize(Tag_Found_Synchro))
                return false;
            if (Tag_Found_Synchro)
                return true;

            //Better detect MPEG-PS
            if (Frame_Count==0
             && Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x00
             && Buffer[Buffer_Offset+2]==0x01
             && Buffer[Buffer_Offset+3]==0xBA)
            {
                MpegPsPattern_Count++;
                if (MpegPsPattern_Count>=2)
                {
                    File__Tags_Helper::Reject("MPEG Audio");
                    return false;
                }
            }

            Buffer_Offset++;
        }

        if (Buffer_Offset+4<=Buffer_Size)//Testing if size is coherant
        {
            //Retrieving some info
            int8u ID0                =(CC1(Buffer+Buffer_Offset+1)>>3)&0x03;
            int8u layer0             =(CC1(Buffer+Buffer_Offset+1)>>1)&0x03;
            int8u bitrate_index0     =(CC1(Buffer+Buffer_Offset+2)>>4)&0x0F;
            int8u sampling_frequency0=(CC1(Buffer+Buffer_Offset+2)>>2)&0x03;
            int8u padding_bit0       =(CC1(Buffer+Buffer_Offset+2)>>1)&0x01;
            //Coherancy
            if (Mpega_SamplingRate[ID0][sampling_frequency0]==0 || Mpega_Coefficient[ID0][layer0]==0 || Mpega_BitRate[ID0][layer0][bitrate_index0]==0 || Mpega_SlotSize[layer0]==0)
                Buffer_Offset++; //False start
            else
            {
                //Testing next start, to be sure
                size_t Size0=(Mpega_Coefficient[ID0][layer0]*Mpega_BitRate[ID0][layer0][bitrate_index0]*1000/Mpega_SamplingRate[ID0][sampling_frequency0]+(padding_bit0?1:0))*Mpega_SlotSize[layer0];
                if (IsSub && Buffer_Offset+Size0==Buffer_Size)
                    break;
                if (File_Offset+Buffer_Offset+Size0!=File_Size-File_EndTagSize)
                {
                    //Padding
                    while (Buffer_Offset+Size0+4<=Buffer_Size && Buffer[Buffer_Offset+Size0]==0x00)
                        Size0++;

                    if (Buffer_Offset+Size0+4>Buffer_Size)
                        return false; //Need more data

                    //Tags
                    bool Tag_Found0;
                    if (!File__Tags_Helper::Synchronize(Tag_Found0, Size0))
                        return false;
                    if (Tag_Found0)
                        return true;
                    if (File_Offset+Buffer_Offset+Size0==File_Size-File_EndTagSize)
                        break;

                    //Testing
                    if ((CC2(Buffer+Buffer_Offset+Size0)&0xFFE0)!=0xFFE0 || (CC1(Buffer+Buffer_Offset+Size0+2)&0xF0)==0xF0 || (CC1(Buffer+Buffer_Offset+Size0+2)&0x0C)==0x0C)
                    {
                        //Testing VBRI in a malformed frame
                        bool VbriFound=false;
                        for (size_t Pos=Buffer_Offset+3; Pos+4<Buffer_Offset+Size0; Pos++)
                        {
                            if (Buffer[Pos  ]==0x56
                             && Buffer[Pos+1]==0x42
                             && Buffer[Pos+2]==0x52
                             && Buffer[Pos+3]==0x49)
                            {
                                VbriFound=true;
                                break;
                            }
                            if (Buffer[Pos])
                                break; //Only NULL bytes are authorized before VBRI header
                        }
                        if (VbriFound)
                            break;
                        Buffer_Offset++;
                    }
                    else
                    {
                        //Retrieving some info
                        int8u ID1                =(CC1(Buffer+Buffer_Offset+Size0+1)>>3)&0x03;
                        int8u layer1             =(CC1(Buffer+Buffer_Offset+Size0+1)>>1)&0x03;
                        int8u bitrate_index1     =(CC1(Buffer+Buffer_Offset+Size0+2)>>4)&0x0F;
                        int8u sampling_frequency1=(CC1(Buffer+Buffer_Offset+Size0+2)>>2)&0x03;
                        int8u padding_bit1       =(CC1(Buffer+Buffer_Offset+Size0+2)>>1)&0x01;
                        //Coherancy
                        if (Mpega_SamplingRate[ID1][sampling_frequency1]==0 || Mpega_Coefficient[ID1][layer1]==0 || Mpega_BitRate[ID1][layer1][bitrate_index1]==0 || Mpega_SlotSize[layer1]==0)
                            Buffer_Offset++; //False start
                        else
                        {
                            //Testing next start, to be sure
                            size_t Size1=(Mpega_Coefficient[ID1][layer1]*Mpega_BitRate[ID1][layer1][bitrate_index1]*1000/Mpega_SamplingRate[ID1][sampling_frequency1]+(padding_bit1?1:0))*Mpega_SlotSize[layer1];
                            if (IsSub && Buffer_Offset+Size0+Size1==Buffer_Size)
                                break;
                            if (File_Offset+Buffer_Offset+Size0+Size1!=File_Size-File_EndTagSize)
                            {
                                //Padding
                                while (Buffer_Offset+Size0+Size1+4<=Buffer_Size && Buffer[Buffer_Offset+Size0+Size1]==0x00)
                                    Size0++;

                                if (Buffer_Offset+Size0+Size1+4>Buffer_Size)
                                    return false; //Need more data

                                //Tags
                                bool Tag_Found1;
                                if (!File__Tags_Helper::Synchronize(Tag_Found1, Size0+Size1))
                                    return false;
                                if (Tag_Found1)
                                    return true;
                                if (File_Offset+Buffer_Offset+Size0+Size1==File_Size-File_EndTagSize)
                                    break;

                                //Testing
                                if ((CC2(Buffer+Buffer_Offset+Size0+Size1)&0xFFE0)!=0xFFE0 || (CC1(Buffer+Buffer_Offset+Size0+Size1+2)&0xF0)==0xF0 || (CC1(Buffer+Buffer_Offset+Size0+Size1+2)&0x0C)==0x0C)
                                    Buffer_Offset++;
                                else
                                {
                                    //Retrieving some info
                                    int8u ID2                =(CC1(Buffer+Buffer_Offset+Size0+Size1+1)>>3)&0x03;
                                    int8u layer2             =(CC1(Buffer+Buffer_Offset+Size0+Size1+1)>>1)&0x03;
                                    int8u bitrate_index2     =(CC1(Buffer+Buffer_Offset+Size0+Size1+2)>>4)&0x0F;
                                    int8u sampling_frequency2=(CC1(Buffer+Buffer_Offset+Size0+Size1+2)>>2)&0x03;
                                    int8u padding_bit2       =(CC1(Buffer+Buffer_Offset+Size0+Size1+2)>>1)&0x01;
                                    //Coherancy
                                    if (Mpega_SamplingRate[ID2][sampling_frequency2]==0 || Mpega_Coefficient[ID2][layer2]==0 || Mpega_BitRate[ID2][layer2][bitrate_index2]==0 || Mpega_SlotSize[layer2]==0)
                                        Buffer_Offset++; //False start
                                    else
                                    {
                                        //Testing next start, to be sure
                                        size_t Size2=(Mpega_Coefficient[ID2][layer2]*Mpega_BitRate[ID2][layer2][bitrate_index2]*1000/Mpega_SamplingRate[ID2][sampling_frequency2]+(padding_bit2?1:0))*Mpega_SlotSize[layer2];
                                        if (IsSub && Buffer_Offset+Size0+Size1+Size2==Buffer_Size)
                                            break;
                                        if (File_Offset+Buffer_Offset+Size0+Size1+Size2!=File_Size-File_EndTagSize)
                                        {
                                            //Padding
                                            while (Buffer_Offset+Size0+Size1+Size2+4<=Buffer_Size && Buffer[Buffer_Offset+Size0+Size1+Size2]==0x00)
                                                Size0++;

                                            if (Buffer_Offset+Size0+Size1+Size2+4>Buffer_Size)
                                            {
                                                if (IsSub || File_Offset+Buffer_Offset+Size0+Size1+Size2<File_Size)
                                                    break;
                                                return false; //Need more data
                                            }

                                            //Tags
                                            bool Tag_Found2;
                                            if (!File__Tags_Helper::Synchronize(Tag_Found2, Size0+Size1+Size2))
                                                return false;
                                            if (Tag_Found2)
                                                return true;
                                            if (File_Offset+Buffer_Offset+Size0+Size1+Size2==File_Size-File_EndTagSize)
                                                break;

                                            //Testing
                                            if ((CC2(Buffer+Buffer_Offset+Size0+Size1+Size2)&0xFFE0)!=0xFFE0 || (CC1(Buffer+Buffer_Offset+Size0+Size1+Size2+2)&0xF0)==0xF0 || (CC1(Buffer+Buffer_Offset+Size0+Size1+Size2+2)&0x0C)==0x0C)
                                                Buffer_Offset++;
                                            else
                                                break; //while()
                                        }
                                        else
                                            break; //while()
                                    }
                                }
                            }
                            else
                                break; //while()
                        }
                    }
                }
                else
                    break; //while()
            }
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+4>Buffer_Size)
    {
        if (Buffer_Offset+3==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFE0)!=0xFFE0)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFE0)!=0xFFE0)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x00)
            Buffer_Offset++;
        return false;
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Mpega::Synched_Test()
{
    //Tags
    if (!File__Tags_Helper::Synched_Test())
        return false;

    //Padding
    while (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]==0x00)
        Buffer_Offset++;

    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0xFF
     || (Buffer[Buffer_Offset+1]&0xE0)!=0xE0
     || (Buffer[Buffer_Offset+2]&0xF0)==0xF0
     || (Buffer[Buffer_Offset+2]&0x0C)==0x0C)
    {
        Synched=false;
        return true;
    }

    //Retrieving some info
    int8u ID0                =(CC1(Buffer+Buffer_Offset+1)>>3)&0x03;
    int8u layer0             =(CC1(Buffer+Buffer_Offset+1)>>1)&0x03;
    int8u bitrate_index0     =(CC1(Buffer+Buffer_Offset+2)>>4)&0x0F;
    int8u sampling_frequency0=(CC1(Buffer+Buffer_Offset+2)>>2)&0x03;
    if (Mpega_SamplingRate[ID0][sampling_frequency0]==0 || Mpega_Coefficient[ID0][layer0]==0 || Mpega_BitRate[ID0][layer0][bitrate_index0]==0 || Mpega_SlotSize[layer0]==0)
    {
        Synched=false;
        return true;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Mpega::Demux_UnpacketizeContainer_Test()
{
    //Retrieving some info
    int8u ID0                =(CC1(Buffer+Buffer_Offset+1)>>3)&0x03;
    int8u layer0             =(CC1(Buffer+Buffer_Offset+1)>>1)&0x03;
    int8u bitrate_index0     =(CC1(Buffer+Buffer_Offset+2)>>4)&0x0F;
    int8u sampling_frequency0=(CC1(Buffer+Buffer_Offset+2)>>2)&0x03;
    int8u padding_bit0       =(CC1(Buffer+Buffer_Offset+2)>>1)&0x01;

    if (Mpega_SamplingRate[ID][sampling_frequency]==0 || Mpega_Coefficient[ID][layer]==0 || Mpega_BitRate[ID][layer][bitrate_index]==0 || Mpega_SlotSize[layer]==0)
        return true; //Synhro issue

    #if MEDIAINFO_ADVANCED
        if (Frame_Count && File_Demux_Unpacketize_StreamLayoutChange_Skip)
        {
            int8u mode0              =CC1(Buffer+Buffer_Offset+3)>>6;
            if (sampling_frequency0!=sampling_frequency_Frame0 || mode0!=mode_Frame0)
            {
                return true;
            }
        }
    #endif //MEDIAINFO_ADVANCED

    Demux_Offset=Buffer_Offset+(Mpega_Coefficient[ID0][layer0]*Mpega_BitRate[ID0][layer0][bitrate_index0]*1000/Mpega_SamplingRate[ID0][sampling_frequency0]+(padding_bit0?1:0))*Mpega_SlotSize[layer0];

    if (Demux_Offset>Buffer_Size)
        return false;

    Demux_UnpacketizeContainer_Demux();

    return true;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpega::Header_Parse()
{
    //Parsing
    BS_Begin();
    Skip_S2(11,                                                 "syncword");
    Get_S1 (2, ID,                                              "ID"); Param_Info1(Mpega_Version[ID]);
    Get_S1 (2, layer,                                           "layer"); Param_Info1(Mpega_Layer[layer]);
    Get_SB (   protection_bit,                                  "protection_bit");
    Get_S1 (4, bitrate_index,                                   "bitrate_index"); Param_Info2(Mpega_BitRate[ID][layer][bitrate_index], " Kbps");
    Get_S1 (2, sampling_frequency,                              "sampling_frequency"); Param_Info2(Mpega_SamplingRate[ID][sampling_frequency], " Hz");
    Get_SB (   padding_bit,                                     "padding_bit");
    Skip_SB(                                                    "private_bit");
    Get_S1 (2, mode,                                            "mode"); Param_Info2(Mpega_Channels[mode], " channels"); Param_Info1(Mpega_Codec_Profile[mode]);
    Get_S1 (2, mode_extension,                                  "mode_extension"); Param_Info1(Mpega_Codec_Profile_Extension[mode_extension]);
    Get_SB (   copyright,                                       "copyright");
    Get_SB (   original_home,                                   "original_home");
    Get_S1 (2, emphasis,                                        "emphasis"); Param_Info1(Mpega_Emphasis[emphasis]);
    BS_End();

    //Coherancy
    if (Mpega_SamplingRate[ID][sampling_frequency]==0 || Mpega_Coefficient[ID][layer]==0 || Mpega_BitRate[ID][layer][bitrate_index]==0 || Mpega_SlotSize[layer]==0)
    {
        Element_Offset=1;
        Header_Fill_Size(1);
        Header_Fill_Code(0, "False start");
        Synched=false;
        return;
    }

    //Filling
    int64u Size=(Mpega_Coefficient[ID][layer]*Mpega_BitRate[ID][layer][bitrate_index]*1000/Mpega_SamplingRate[ID][sampling_frequency]+(padding_bit?1:0))*Mpega_SlotSize[layer];

    //Special case: tags is inside the last frame
    if (File_Offset+Buffer_Offset+Size>=File_Size-File_EndTagSize)
        Size=File_Size-File_EndTagSize-(File_Offset+Buffer_Offset);

    Header_Fill_Size(Size);
    Header_Fill_Code(0, "frame");

    //Filling error detection
    sampling_frequency_Count[sampling_frequency]++;
    mode_Count[mode]++;

    FILLING_BEGIN();
        #if MEDIAINFO_DEMUX
            #if MEDIAINFO_ADVANCED
                if (!Frame_Count)
                {
                    File_Demux_Unpacketize_StreamLayoutChange_Skip=Config->File_Demux_Unpacketize_StreamLayoutChange_Skip_Get();
                    if (File_Demux_Unpacketize_StreamLayoutChange_Skip)
                    {
                        sampling_frequency_Frame0=sampling_frequency;
                        mode_Frame0=mode;
                    }
                }
            #endif //MEDIAINFO_ADVANCED
        #endif //MEDIAINFO_DEMUX
    FILLING_END();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpega::Data_Parse()
{
    //If false start
    if (Element_Size==0)
    {
        Element_DoNotShow();
        return;
    }

    //Partial frame
    if (Header_Size+Element_Size<(int64u)((Mpega_Coefficient[ID][layer]*Mpega_BitRate[ID][layer][bitrate_index]*1000/Mpega_SamplingRate[ID][sampling_frequency]+(padding_bit?1:0))*Mpega_SlotSize[layer]))
    {
        Element_Name("Partial frame");
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    //PTS
    Element_Info1C((FrameInfo.PTS!=(int64u)-1), __T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000)));

    //Name
    Element_Info1(__T("Frame ")+Ztring::ToZtring(Frame_Count));

    //VBR and library headers
    if (Frame_Count<3) //No need to do it too much
    {
        if (!Header_Xing())
            Header_VBRI();
    }

    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size-File_EndTagSize)
        Frame_Count_Valid=Frame_Count; //Finish MPEG Audio frames in case of there are less than Frame_Count_Valid frames
    if (Frame_Count==0 && Frame_Count_NotParsedIncluded==0)
        PTS_Begin=FrameInfo.PTS;
    Frame_Count++;
    Frame_Count_InThisBlock++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
    LastSync_Offset=File_Offset+Buffer_Offset+Element_Size;
    {
        int16u Samples;
        if (ID==3 && layer==3) //MPEG 1 layer 1
             Samples=384;
        else if ((ID==2 || ID==0) && layer==1) //MPEG 2 or 2.5 layer 3
            Samples=576;
        else
            Samples=1152;
        FrameInfo.DUR=float64_int64s(((float64)1)/Mpega_SamplingRate[ID][sampling_frequency]*Samples*1000000000);
        if (FrameInfo.DTS!=(int64u)-1)
            FrameInfo.DTS+=FrameInfo.DUR;
        if (FrameInfo.PTS!=(int64u)-1)
            FrameInfo.PTS=FrameInfo.DTS;
    }

    //LAME
    if (Encoded_Library.empty() && (Frame_Count<Frame_Count_Valid || File_Offset+Buffer_Offset+Element_Size==File_Size-File_EndTagSize)) //Can be elsewhere... At the start, or end frame
        Header_Encoders();

    //Filling
    BitRate_Count[Mpega_BitRate[ID][layer][bitrate_index]]++;
    Channels_Count[mode]++;
    Extension_Count[mode_extension]++;
    Emphasis_Count[emphasis]++;

    if (Status[IsFilled])
    {
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    //error_check
    if (protection_bit)
    {
        Element_Begin1("error_check");
        Skip_B2(                                                "crc_check");
        Element_End0();
    }

    //audio_data
    Element_Begin1("audio_data");
    switch (layer)
    {
        case 1 : //Layer 3
                audio_data_Layer3();
                break;
        default: Skip_XX(Element_Size-Element_Offset,           "(data)");
    }
    Element_End0();

    //MP3 Surround detection
    for (int64u Element_Offset_S=Element_Offset; Element_Offset_S+4<Element_Size; Element_Offset_S++)
    {
        if ( Buffer[(size_t)(Buffer_Offset+Element_Offset_S  )]      ==0xCF
         && (Buffer[(size_t)(Buffer_Offset+Element_Offset_S+1)]&0xF0)==0x30) //12 bits, 0xCF3x
        {
            int8u Surround_Size=((Buffer[(size_t)(Buffer_Offset+Element_Offset_S+1)]&0x0F)<<4)
                              | ((Buffer[(size_t)(Buffer_Offset+Element_Offset_S+2)]&0xF0)>>4);
            int16u CRC12       =((Buffer[(size_t)(Buffer_Offset+Element_Offset_S+2)]&0x0F)<<8)
                              |   Buffer[(size_t)(Buffer_Offset+Element_Offset_S+3)];
            if (Element_Offset_S+Surround_Size-4>Element_Size)
                break;

            //CRC
            int16u CRC12_Calculated=0x0FFF;
            int8u* Data=(int8u*)Buffer+(size_t)(Buffer_Offset+Element_Offset_S+4);
            if (Element_Offset_S+Surround_Size+4>=Element_Size)
                break;
            for (int8u Surround_Pos=0; Surround_Pos<Surround_Size-4; Surround_Pos++)
                CRC12_Calculated=0x0FFF & (((CRC12_Calculated<<8)&0xff00)^Mpega_CRC12_Table[((CRC12_Calculated>>4) ^ *Data++) & 0xff]);
            if (CRC12_Calculated!=CRC12)
                break;

            //Parsing
            Skip_XX(Element_Offset_S-Element_Offset,            "data");
            BS_Begin();
            Element_Begin1("Surround");
            Skip_S2(12,                                         "Sync");
            Skip_S1( 8,                                         "Size");
            Skip_S2(12,                                         "CRC12");
            BS_End();
            Skip_XX(Surround_Size-4,                            "data");
            Element_End0();

            //Filling
            Surround_Frames++;
            break;
        }
    }

    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "next data");

    FILLING_BEGIN();
        //Filling
        if (IsSub && BitRate_Count.size()>1 && !Encoded_Library.empty())
            Frame_Count_Valid=Frame_Count;
        if (!Status[IsAccepted])
            File__Analyze::Accept("MPEG Audio");
        if (!Status[IsFilled] && Frame_Count>=Frame_Count_Valid)
        {
            Fill("MPEG Audio");

            //Jumping
            if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1.0 && File_Offset+Buffer_Offset<File_Size/2)
            {
                File__Tags_Helper::GoToFromEnd(16*1024, "MPEG-A");
                LastSync_Offset=(int64u)-1;
                if (File_GoTo!=(int64u)-1)
                    Open_Buffer_Unsynch();
            }
        }

        //Detect Id3v1 tags inside a frame
        if (!IsSub && File_Offset+Buffer_Offset+(size_t)Element_Size>File_Size-File_EndTagSize)
        {
            Open_Buffer_Unsynch();
            File__Analyze::Data_GoTo(File_Size-File_EndTagSize, "Tags inside a frame, parsing the tags");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpega::audio_data_Layer3()
{
    int16u main_data_end;
    BS_Begin();
    if (ID==3) //MPEG-1
        Get_S2 (9, main_data_end,                               "main_data_end");
    else
        Get_S2 (8, main_data_end,                               "main_data_end");
    if ((int32u)main_data_end>Reservoir_Max)
        Reservoir_Max=main_data_end;
    Reservoir+=main_data_end;
    if (ID==3) //MPEG-1
    {
        if (mode==3) //Mono
            Skip_S1(5,                                          "private_bits");
        else
            Skip_S1(3,                                          "private_bits");
    }
    else
    {
        if (mode==3) //Mono
            Skip_S1(1,                                          "private_bits");
        else
            Skip_S1(2,                                          "private_bits");
    }
    if (ID==3) //MPEG-1
    {
    Element_Begin1("scfsi");
        for(int8u ch=0; ch<Mpega_Channels[mode]; ch++)
            for(int8u scfsi_band=0; scfsi_band<4; scfsi_band++)
            {
                bool scfsi;
                Get_SB (   scfsi,                               "scfsi");
                if (scfsi)
                    Scfsi++;
            }
        Element_End0();
    }
    for(int8u gr=0; gr<(ID==3?2:1); gr++)
    {
        Element_Begin1("granule");
        if (mode>=4)
            return;
        for(int8u ch=0; ch<Mpega_Channels[mode]; ch++)
        {
            Element_Begin1("channel");
            Skip_S2(12,                                         "part2_3_length");
            Skip_S2(9,                                          "big_values");
            Skip_S1(8,                                          "global_gain");
            if (ID==3) //MPEG-1
                Skip_S1(4,                                      "scalefac_compress");
            else
                Skip_S2(9,                                      "scalefac_compress");
            bool blocksplit_flag;
            Get_SB (   blocksplit_flag,                         "blocksplit_flag");
            if (blocksplit_flag==1)
            {
                int8u block_type;
                bool  mixed_block_flag;
                Get_S1 (2, block_type,                          "block_type");
                Get_SB (   mixed_block_flag,                    "mixed_block_flag");
                for (int8u region=0; region<2; region++)
                    Skip_S1(5,                                  "table_select");
                for (int8u window=0; window<3; window++)
                    Skip_S1(3,                                  "subblock_gain");
                if (block_type == 2)
                {
                    if (mixed_block_flag==1)
                    {
                        Param_Info1("Mixed");
                        Block_Count[2]++; //Mixed
                    }
                    else
                    {
                        Param_Info1("Short");
                        Block_Count[1]++; //Short
                    }
                }
                else
                {
                    Param_Info1("Long");
                    Block_Count[0]++; //Long
                }
            }
            else
            {
                for (int8u region=0; region<3; region++)
                    Skip_S1(5,                                  "table_select");
                Skip_S1(4,                                      "region0_count");
                Skip_S1(3,                                      "region1_count");
                Param_Info1("Long");
                Block_Count[0]++; //Long
            }
            if (ID==3) //MPEG-1
                Skip_SB(                                        "preflag");
            bool scalefac;
            Get_SB (   scalefac,                                "scalefac_scale");
            if (scalefac)
                Scalefac++;
            Skip_SB(                                            "count1table_select");
            Element_End0();
        } //channels
        Element_End0();
    } //granules
    BS_End();
    //Skip_XX(Element_Size-main_data_end-Element_Offset,          "main_data");
}

//---------------------------------------------------------------------------
void File_Mpega::Data_Parse_Fill()
{
}

//---------------------------------------------------------------------------
bool File_Mpega::Header_Xing()
{
    int32u Xing_Header_Offset;
    if (ID==3) //MPEG-1
        if (mode==3) //Mono
            Xing_Header_Offset=21-4;
        else
            Xing_Header_Offset=36-4;
    else //MPEG-2 or 2.5
        if (mode==3) //Mono
            Xing_Header_Offset=13-4;
        else
            Xing_Header_Offset=21-4;
    if (Buffer_Offset+Xing_Header_Offset+128<Buffer_Size)
    {
        const int8u* Xing_Header=Buffer+Buffer_Offset+Xing_Header_Offset;
        if (CC4(Xing_Header)==CC4("Xing") || CC4(Xing_Header)==CC4("Info"))
        {
            //This is a "tag"
            Element_Info1("Tag (Xing)");

            //Parsing
            Element_Begin1("Xing");
            Element_Begin1("Xing header");
            Skip_XX(Xing_Header_Offset,                         "Junk");
            int32u Flags;
            bool FrameCount, FileSize, TOC, Scale, Lame;
            Skip_C4(                                            "Xing");
            Get_B4 (Flags,                                      "Flags");
                Get_Flags(Flags, 0, FrameCount,                 "FrameCount");
                Get_Flags(Flags, 1, FileSize,                   "FileSize");
                Get_Flags(Flags, 2, TOC,                        "TOC");
                Get_Flags(Flags, 3, Scale,                      "Scale");
                Get_Flags(Flags, 4, Lame,                       "Lame");
            int32u Xing_Header_Size=8
                                   +(FrameCount?  4:0)    //FrameCount
                                   +(FileSize?    4:0)    //FileSize
                                   +(TOC?       100:0)    //TOC
                                   +(Scale?       4:0)    //Scale
                                   +(Lame?      348:0);   //Lame
            Element_End0();
            //Element size
            if (Xing_Header_Size>Element_Size-Xing_Header_Offset)
                return false; //Error tag size

            //Parsing
            if (FrameCount)
                Get_B4 (VBR_Frames,                             "FrameCount"); //FrameCount exclude this frame
            if (FileSize)
            {
                int32u VBR_FileSize_Temp;
                Get_B4 (VBR_FileSize_Temp,                      "FileSize");
                if (VBR_FileSize_Temp>4+Element_Size)
                   VBR_FileSize=VBR_FileSize_Temp-4-Element_Size; //FileSize include the Xing element
            }
            if (TOC)
                Skip_XX(100,                                    "TOC");
            if (Scale)
                Get_B4 (Xing_Scale,                             "Scale");
            Ztring Lib;
            Element_End0();
            Peek_Local(4, Lib);
            if (Lame || Lib==__T("LAME") || Lib==__T("GOGO") || Lib==__T("L3.9"))
                Header_Encoders_Lame();

            if (CC4(Xing_Header)==CC4("Info"))
                VBR_Frames=0; //This is not a VBR file

            //Clearing Error detection
            sampling_frequency_Count.clear();
            mode_Count.clear();

            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
bool File_Mpega::Header_VBRI()
{
    const size_t Fraunhofer_Header_Offset=36-4;
    if (Buffer_Offset+Fraunhofer_Header_Offset+32<Buffer_Size)
    {
        const int8u* Fraunhofer_Header=Buffer+Buffer_Offset+Fraunhofer_Header_Offset;
        if (CC4(Fraunhofer_Header)==CC4("VBRI") && CC2(Fraunhofer_Header+4)==0x0001) //VBRI v1 only
        {
            //This is a "tag"

            Element_Info1("Tag (VBRI)");

            //Parsing
            int32u VBR_FileSize_Temp;
            int16u TableSize, TableScale, EntryBytes;
            Skip_XX(Fraunhofer_Header_Offset,                   "Junk");
            Element_Begin1("VBRI");
            Skip_C4(                                            "Sync");
            Skip_B2(                                            "Version");
            Skip_B2(                                            "Delay");
            Skip_B2(                                            "Quality");
            Get_B4 (VBR_FileSize_Temp,                          "StreamBytes");
            Get_B4 (VBR_Frames,                                 "StreamFrames"); //Multiplied by SamplesPerFrame (1152 for >=32KHz, else 576) --> Duration in samples
            Get_B2 (TableSize,                                  "TableSize");
            Get_B2 (TableScale,                                 "TableScale");
            Get_B2 (EntryBytes,                                 "EntryBytes");
            Skip_B2(                                            "EntryFrames"); //Count of frames per entry
            Element_Begin1("Table");
                for (int16u Pos=0; Pos<TableSize; Pos++)
                {
                    switch (EntryBytes)
                    {
                        case 1 : {Info_B1(Entry,                "Entry"); Param_Info2 (Entry*TableScale, " bytes");} break;
                        case 2 : {Info_B2(Entry,                "Entry"); Param_Info2 (Entry*TableScale, " bytes");} break;
                        case 4 : {Info_B4(Entry,                "Entry"); Param_Info2 (Entry*TableScale, " bytes");} break;
                        default: Skip_XX(EntryBytes,            "Entry");
                    }
                }
            Element_End0();
            Element_End0();
            VBR_FileSize=VBR_FileSize_Temp;

            //Clearing Error detection
            sampling_frequency_Count.clear();
            mode_Count.clear();

            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
bool File_Mpega::Header_Encoders()
{
    std::string BufferS((const char*)(Buffer+Buffer_Offset), (size_t)Element_Size);
    size_t Buffer_Pos;

    //Lame
    Buffer_Pos=BufferS.find("LAME");
    if (Buffer_Pos!=std::string::npos && Buffer_Pos<=Element_Size-8)
    {
        Element_Info1("With tag (Lame)");
        Element_Offset=Buffer_Pos;
        if (Element_Offset+20<=Element_Size)
            Get_Local(20, Encoded_Library,                      "Encoded_Library");
        else
            Get_Local( 8, Encoded_Library,                      "Encoded_Library");
        Encoded_Library.Trim(__T('A'));
        Encoded_Library.Trim(__T('U'));
        Encoded_Library.Trim(__T('\xAA'));
        Element_Offset=0; //Reseting it
        return true;
    }

    //RCA
    Buffer_Pos=BufferS.find("RCA mp3PRO Encoder");
    if (Buffer_Pos!=std::string::npos && Buffer_Pos<Element_Size-23)
    {
        Element_Info1("With tag (RCA)");
        Encoded_Library="RCA ";
        Encoded_Library+=Ztring((const char*)(Buffer+Buffer_Offset+18), 5);
        return true;
    }

    //Thomson
    Buffer_Pos=BufferS.find("THOMSON mp3PRO Encoder");
    if (Buffer_Pos!=std::string::npos && Buffer_Pos<Element_Size-29)
    {
        Element_Info1("With tag (Thomson)");
        Encoded_Library="Thomson ";
        Encoded_Library+=Ztring((const char*)(Buffer+Buffer_Offset+22), 6);
        return true;
    }

    //Gogo (old)
    Buffer_Pos=BufferS.find("MPGE");
    if (Buffer_Pos!=std::string::npos)
    {
        Element_Info1("With tag (Gogo)");
        Encoded_Library="Gogo <3.0";
        return true;
    }

    //Gogo (new)
    Buffer_Pos=BufferS.find("GOGO");
    if (Buffer_Pos!=std::string::npos)
    {
        Element_Info1("With tag (Gogo)");
        Encoded_Library="Gogo >=3.0";
        return true;
    }

    return false;
}

void File_Mpega::Header_Encoders_Lame()
{
    Peek_Local(8, Encoded_Library);
    if (Encoded_Library.find(__T("L3.99"))==0)
        Encoded_Library.insert(1, __T("AME")); //Ugly version string in Lame 3.99.1 "L3.99r1\0"
    if ((Encoded_Library>=__T("LAME3.90")) && Element_IsNotFinished())
    {
        int8u Flags, lowpass, EncodingFlags, BitRate, StereoMode;
        Param_Info1(Ztring(__T("V "))+Ztring::ToZtring((100-Xing_Scale)/10));
        Param_Info1(Ztring(__T("q "))+Ztring::ToZtring((100-Xing_Scale)%10));
        Get_Local(9, Encoded_Library,                           "Encoded_Library");
        Get_B1 (Flags,                                          "Flags");
        if ((Flags&0xF0)<=0x20) //Rev. 0 or 1, http://gabriel.mp3-tech.org/mp3infotag.html and Rev. 2 was seen.
        {
            Param_Info1(Lame_Method[Flags&0x0F]);
            BitRate_Mode=Lame_BitRate_Mode[Flags&0x0F];
            if ((Flags&0x0F)==1 || (Flags&0x0F)==8) //2 possible values for CBR
                VBR_Frames=0;
        }
        Get_B1 (lowpass,                                        "Lowpass filter value"); Param_Info2(lowpass*100, " Hz");
        Skip_B4(                                                "Peak signal amplitude");
        Skip_B2(                                                "Radio Replay Gain");
        Skip_B2(                                                "Audiophile Replay Gain");
        Get_B1 (EncodingFlags,                                  "Encoding Flags"); Param_Info1(Ztring(__T("ATH Type="))+Ztring::ToZtring(Flags&0x0F));
            Skip_Flags(EncodingFlags, 4,                        "nspsytune");
            Skip_Flags(EncodingFlags, 5,                        "nssafejoint");
            Skip_Flags(EncodingFlags, 6,                        "nogap (after)");
            Skip_Flags(EncodingFlags, 7,                        "nogap (before)");
        Get_B1 (BitRate,                                        "BitRate");
        Skip_B3(                                                "Encoder delays");
        BS_Begin();
        Skip_S1(2,                                              "Source sample frequency");
        Skip_SB(                                                "unwise settings used");
        Get_S1 (3, StereoMode,                                  "Stereo mode");
        Skip_S1(2,                                              "noise shapings");
        BS_End();
        Skip_B1(                                                "MP3 Gain");
        Skip_B2(                                                "Preset and surround info");
        Skip_B4(                                                "MusicLength");
        Skip_B2(                                                "MusicCRC");
        Skip_B2(                                                "CRC-16 of Info Tag");

        FILLING_BEGIN();
            Encoded_Library_Settings+=__T("-m ");
            switch(StereoMode)
            {
                case 0 : Encoded_Library_Settings+=__T("m"); break;
                case 1 : Encoded_Library_Settings+=__T("s"); break;
                case 2 : Encoded_Library_Settings+=__T("d"); break;
                case 3 : Encoded_Library_Settings+=__T("j"); break;
                case 4 : Encoded_Library_Settings+=__T("f"); break;
                case 5 : Encoded_Library_Settings+=__T("a"); break;
                case 6 : Encoded_Library_Settings+=__T("i"); break;
                default: ;
            }
            if (Xing_Scale<=100) //Xing_Scale is used for LAME quality
            {
                Encoded_Library_Settings+=__T( " -V ")+Ztring::ToZtring((100-Xing_Scale)/10);
                Encoded_Library_Settings+=__T( " -q ")+Ztring::ToZtring((100-Xing_Scale)%10);
            }
            if (lowpass)
                Encoded_Library_Settings+=(Encoded_Library_Settings.empty()?__T("-lowpass "):__T(" -lowpass "))+((lowpass%10)?Ztring::ToZtring(((float)lowpass)/10, 1):Ztring::ToZtring(lowpass/10));
            switch (Flags&0x0F)
            {
                case  2 :
                case  9 : //ABR
                            Encoded_Library_Settings+=__T(" --abr"); break;
                case  3 : //VBR (old/rh)
                            Encoded_Library_Settings+=__T(" --vbr-old"); break;
                case  4 : //VBR (new/mtrh)
                            Encoded_Library_Settings+=__T(" --vbr-new"); break;
                case  5 : //VBR (?/mt)
                            Encoded_Library_Settings+=__T(" --vbr-mt"); break;
                default : ;
            }
            if (BitRate!=0x00 && BitRate!=0xFF)
            {
                switch (Flags&0x0F)
                {
                    case  1 :
                    case  8 : //CBR
                        Encoded_Library_Settings+=__T(" -b ")+Ztring::ToZtring(BitRate);
                        break;
                    case  2 :
                    case  9 : //ABR
                        BitRate_Nominal.From_Number(BitRate*1000);
                        Encoded_Library_Settings+=__T(" ")+Ztring::ToZtring(BitRate);
                        break;
                    case  3 : //VBR (old/rh)
                    case  4 : //VBR (new/mtrh)
                    case  5 : //VBR (?/mt)
                        BitRate_Minimum.From_Number(BitRate*1000);
                        Encoded_Library_Settings+=__T(" -b ")+Ztring::ToZtring(BitRate);
                        break;
                    default : ;
                }
            }
        FILLING_END();
    }
    else
        Get_Local(20, Encoded_Library,                          "Encoded_Library");
}

void File_Mpega::Encoded_Library_Guess()
{
    //TODO: Not yet enough precise

    /*
    if (Block_Count[1]==0) //No short blocks
    {
        if (mode==2) //Dual Mono
        {
            if (Scfsi>0) //scfsi used
                {}
            else //no scfsi
            {
                if (Scalefac>0) //scalefacors used
                    {}
                else //scalefacors not used
                    Encoded_Library="Shine";
            }
        }
        else //Other than dual mono
        {
            if (Extension_Count[1]>0 || Extension_Count[3]>0) //Intensity Stereo
                Encoded_Library="Xing (very old)";
            else //No Intensity Stereo
            {
                if (Scfsi>0) //Scfsi used
                    Encoded_Library="Xing (new)";
                else //Scsfi not used
                {
                    if (Channels_Count[2]>0) //Joint Stereo
                    {
                        if (Channels_Count[0]>0) //also includes no Joint Stereo frames
                        {
                            if (padding_bit) //Padding
                            {
                                if (original_home)
                                    Encoded_Library="FhG (l3enc)";
                                else
                                    Encoded_Library="FhG (fastenc or mp3enc)";
                            }
                            else //No padding
                                Encoded_Library="FhG (ACM or producer pro)";
                        }
                        else //No stereo frames: joint stereo was forced
                        {
                            if (padding_bit && !original_home && !copyright)
                                Encoded_Library="QDesign (fast mode)";
                        }
                    }
                    else
                    {
                        if (Channels_Count[0]>0 && Scalefac==0 && !original_home) //Stereo
                            Encoded_Library="Plugger";
                        else
                            Encoded_Library="Xing (old)";
                    }
                }
            }
        }
    }
    else //Short blocks
    {
        if (Scfsi)  //scfsi used
        {
            if (Scalefac>0) //Scalefactor used
                Encoded_Library="Gogo (after 3.0)"; //Could be lame, but with a label, detected elsewhere before
            else
                Encoded_Library="Lame (old) or m3e";
        }
        else //Scfsi not used
        {
            if (Scalefac>0) //Scalefactor used
            {
                if (padding_bit)
                {
                    if (original_home)
                    {
                        //10 last bytes
                        //int sum = get_final_sum(data);
                        //if (sum==0)
                        //    return guess = __T("FhG (fastenc, low quality mode)");
                        //else if (sum==10 * 0xFF)
                        //    return guess = __T("FhG (l3enc)");
                        //else if (sum==5 * 0x20)
                        //    return guess = __T("FhG (fastenc, medium or high quality mode)");
                        //else
                        //    return guess = __T("FhG (l3enc or fastenc)");
                    }
                    else
                    {
                        if (Channels_Count[1]>0 && Extension_Count[1]>0)        //Joint Stereo and some Intensity Stereo
                            Encoded_Library="Thomson mp3PRO Encoder";
                        else
                            Encoded_Library="FhG (fastenc or mp3enc)";
                    }
                }
                else //No padding
                {
                    if (BitRate_Mode.find(__T("VBR"))==0) //VBR
                        Encoded_Library="FhG (fastenc)";
                    else
                        Encoded_Library="FhG (ACM or producer pro)";
                }
            }
            else //scalefactors not used
            {
                if (Channels_Count[1]>0) //Joint Stereo
                {
                    if (padding_bit && !original_home && !copyright)
                        Encoded_Library="QDesign";
                }
                else //Joint Stereo not used
                {
                    if (BitRate_Mode.find(__T("VBR"))==0) //VBR
                        Encoded_Library="Lame (old)";
                    else //CBR
                    {

                        if (mode==2) //Dual Mono
                        {
                            if (padding_bit)
                                Encoded_Library="Blade";
                            else
                                Encoded_Library="dist10 encoder or other encoder";
                        }
                        else //Stereo or Mono
                        {
                            //if (data.av_reservoir < 40 && !data.vbr) //ISO based encoders are unable to properly use bit reservoir... average reservoir usage is about 10
                            //{
                            //    if (data.padding)
                            //        return guess = __T("Blade");
                            //    else
                            //        return guess = __T("dist10 encoder or other encoder");
                            //}
                            //else
                            //    return guess = __T("Gogo (before 3.0)");
                        }
                    }
                }
            }
        }
    }
    */
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_MPEGA_YES

