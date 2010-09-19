// File_Avc - Info for AVC Video files
// Copyright (C) 2006-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AVC_YES) || defined(MEDIAINFO_MPEGPS_YES) || defined(MEDIAINFO_MPEGTS_YES)
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

#include "ZenLib/Conf.h"
using namespace ZenLib;

//---------------------------------------------------------------------------
const char* Avc_profile_idc(int8u profile_idc)
{
    switch (profile_idc)
    {
        case  44 : return "CAVLC 4:4:4 Intra";
        case  66 : return "Baseline";
        case  77 : return "Main";
        case  83 : return "Scalable Baseline";
        case  86 : return "Scalable High";
        case  88 : return "Extended";
        case 100 : return "High";
        case 110 : return "High 10";
        case 118 : return "Multiview High";
        case 122 : return "High 4:2:2";
        case 128 : return "Stereo High";
        case 144 : return "High 4:4:4";
        case 244 : return "High 4:4:4 Predictive";
        default  : return "Unknown";
    }
}

//---------------------------------------------------------------------------
} //NameSpace

//---------------------------------------------------------------------------
#endif //...
//---------------------------------------------------------------------------

//***************************************************************************
//
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AVC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Avc.h"
#include <cstring>
#include <cmath>
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if defined(MEDIAINFO_EIA708_YES)
    #include "MediaInfo/Text/File_Eia708.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const size_t Avc_Errors_MaxCount=32;

//---------------------------------------------------------------------------
const int8u Avc_PixelAspectRatio_Size=17;
const float32 Avc_PixelAspectRatio[]=
{
    (float32)1, //Reserved
    (float32)1,
    (float32)12/(float32)11,
    (float32)10/(float32)11,
    (float32)16/(float32)11,
    (float32)40/(float32)33,
    (float32)24/(float32)11,
    (float32)20/(float32)11,
    (float32)32/(float32)11,
    (float32)80/(float32)33,
    (float32)18/(float32)11,
    (float32)15/(float32)11,
    (float32)64/(float32)33,
    (float32)160/(float32)99,
    (float32)4/(float32)3,
    (float32)3/(float32)2,
    (float32)2,
};

//---------------------------------------------------------------------------
const char* Avc_video_format[]=
{
    "Component",
    "PAL",
    "NTSC",
    "SECAM",
    "MAC",
    "",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
const char* Avc_primary_pic_type[]=
{
    "I",
    "I, P",
    "I, P, B",
    "SI",
    "SI, SP",
    "I, SI",
    "I, SI, P, SP",
    "I, SI, P, SP, B",
};

//---------------------------------------------------------------------------
const char* Avc_slice_type[]=
{
    "P",
    "B",
    "I",
    "SP",
    "SI",
    "P",
    "B",
    "I",
    "SP",
    "SI",
};

//---------------------------------------------------------------------------
const int8u Avc_pic_struct_Size=9;
const char* Avc_pic_struct[]=
{
    "frame",
    "top field",
    "bottom field",
    "top field, bottom field",
    "bottom field, top field",                                                     
    "top field, bottom field, top field repeated",
    "bottom field, top field, bottom field repeated",
    "frame doubling",
    "frame tripling",
};

//---------------------------------------------------------------------------
const int8u Avc_NumClockTS[]=
{
    1,
    1,
    1,
    2,
    2,
    3,
    3,
    2,
    3,
};

//---------------------------------------------------------------------------
const char* Avc_ct_type[]=
{
    "Progressive",
    "Interlaced",
    "Unknown",
    "Reserved",
};

//---------------------------------------------------------------------------
const char* Avc_Colorimetry_format_idc[]=
{
    "monochrome",
    "4:2:0",
    "4:2:2",
    "4:4:4",
};

//---------------------------------------------------------------------------
const int8u Avc_SubWidthC[]=
{
    1,
    2,
    2,
    1,
};

//---------------------------------------------------------------------------
const int8u Avc_SubHeightC[]=
{
    1,
    2,
    1,
    1,
};

//---------------------------------------------------------------------------
const char* Avc_user_data_DTG1_active_format[]=
{
    //1st value is for 4:3, 2nd is for 16:9
    "", //Undefined
    "Reserved",
    "Not recommended",
    "Not recommended",
    "Aspect ratio greater than 16:9", //Use GA94
    "Reserved",
    "Reserved",
    "Reserved",
    "4:3 full frame image / 16:9 full frame image",
    "4:3 full frame image / 4:3 pillarbox image",
    "16:9 letterbox image / 16:9 full frame image",
    "14:9 letterbox image / 14:9 pillarbox image",
    "Reserved",
    "4:3 full frame image, alternative 14:9 center / 4:3 pillarbox image, alternative 14:9 center",
    "16:9 letterbox image, alternative 14:9 center / 16:9 full frame image, alternative 14:9 center",
    "16:9 letterbox image, alternative 4:3 center / 16:9 full frame image, alternative 4:3 center",
};

//---------------------------------------------------------------------------
const char* Avc_colour_primaries(int8u colour_primaries)
{
    switch (colour_primaries)
    {
        case  1 : return "BT.709-5, BT.1361, IEC 61966-2-4, SMPTE RP177";
        case  4 : return "BT.470-6 system M, NTSC, FTC 73.682";
        case  5 : return "BT.470-6 System B, BT.470-6 System G, BT.601-6 625, BT.1358 625, BT.1700 625 PAL, BT.1700 625 SECAM";
        case  6 : return "BT.601-6 525, BT.1358 525, BT.1700 NTSC, SMPTE 170M";
        case  7 : return "SMPTE 240M";
        case  8 : return "Generic film";
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Avc_transfer_characteristics(int8u transfer_characteristics)
{
    switch (transfer_characteristics)
    {
        case  1 : return "BT.709-5, BT.1361";
        case  4 : return "BT.470-6 System M, NTSC, FTC 73.682, BT.1700 625 PAL, BT.1700 625 SECAM";
        case  5 : return "BT.470-6 System B, BT.470-6 System G";
        case  6 : return "BT.601-6 525, BT.601-6 625, BT.1358 525, BT.1358 625, BT.1700 NTSC, SMPTE 170M";
        case  7 : return "SMPTE 240M";
        case  8 : return "Linear";
        case  9 : return "Logarithmic (100:1)";
        case 10 : return "Logarithmic (316.22777:1)";
        case 11 : return "IEC 61966-2-4";
        case 12 : return "BT.1361 extended colour gamut system";
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Avc_matrix_coefficients(int8u matrix_coefficients)
{
    switch (matrix_coefficients)
    {
        case  0 : return "RGB";
        case  1 : return "BT.709-5, BT.1361, IEC 61966-2-4 709, SMPTE RP177";
        case  4 : return "FTC 73.682";
        case  5 : return "BT.470-6 System B, BT.470-6 System G, BT.601-6 625, BT.1358 625, BT.1700 625 PAL, BT.1700 625 SECAM, IEC 61966-2-4 601";
        case  6 : return "BT.601-6 525, BT.1358 525, BT.1700 NTSC, SMPTE 170M";
        case  7 : return "SMPTE 240M";
        case  8 : return "YCgCo";
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Avc_user_data_GA94_cc_type(int8u cc_type)
{
    switch (cc_type)
    {
        case  0 : return "CEA-608 line 21 field 1 closed captions"; //closed caption 3 if this is second field
        case  1 : return "CEA-608 line 21 field 2 closed captions"; //closed caption 4 if this is second field
        case  2 : return "DTVCC Channel Packet Data";
        case  3 : return "DTVCC Channel Packet Start";
        default : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Avc::File_Avc()
:File__Duplicate()
{
    //Config
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Avc;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?64:2; //Currently no 3:2 pulldown detection
    FrameIsAlwaysComplete=false;
    MustParse_SPS_PPS=false;
    MustParse_SPS_PPS_Only=false;
    MustParse_SPS_PPS_Done=false;
    SizedBlocks=false;

    //Temp
    SizeOfNALU_Minus1=(int8u)-1;
    SPS_IsParsed=false;
    PPS_IsParsed=false;
}

//---------------------------------------------------------------------------
File_Avc::~File_Avc()
{
    for (size_t Pos=0; Pos<GA94_03_CC_Parsers.size(); Pos++)
        delete GA94_03_CC_Parsers[Pos]; //GA94_03_CC_Parsers[Pos]=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Avc::Streams_Fill()
{
    //Calculating - Pixels
    int32u Width =(pic_width_in_mbs_minus1       +1)*16;
    int32u Height=(pic_height_in_map_units_minus1+1)*16*(2-frame_mbs_only_flag);
    int32u CropUnitX=Avc_SubWidthC [chroma_format_idc];
    int32u CropUnitY=Avc_SubHeightC[chroma_format_idc]*(2-frame_mbs_only_flag);
    Width -=(frame_crop_left_offset+frame_crop_right_offset )*CropUnitX;
    Height-=(frame_crop_top_offset +frame_crop_bottom_offset)*CropUnitY;

    //Calculating - PixelAspectRatio
    float32 PixelAspectRatio;
    if (aspect_ratio_idc<Avc_PixelAspectRatio_Size)
        PixelAspectRatio=Avc_PixelAspectRatio[aspect_ratio_idc];
    else if (sar_height)
        PixelAspectRatio=((float)sar_width)/sar_height;
    else
        PixelAspectRatio=1; //Unknown

    if (Count_Get(Stream_Video)==0)
        Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "AVC");
    Fill(Stream_Video, 0, Video_Codec, "AVC");

    if (!subset_seq_parameter_set_ids.empty())
    {
        std::map<int32u, seq_parameter_set_>::iterator subset_seq_parameter_set_id=subset_seq_parameter_set_ids.begin(); //Currently, only 1 SPS is supported
        Ztring Profile=Ztring().From_Local(Avc_profile_idc(subset_seq_parameter_set_id->second.profile_idc))+_T("@L")+Ztring().From_Number(((float)subset_seq_parameter_set_id->second.level_idc)/10, 1);
        Fill(Stream_Video, 0, Video_Format_Profile, Profile);
        Fill(Stream_Video, 0, Video_MultiView_Count, num_views_minus1+1);
    }
    if (!seq_parameter_set_ids.empty())
    {
        std::map<int32u, seq_parameter_set_>::iterator seq_parameter_set_id=seq_parameter_set_ids.begin(); //Currently, only 1 SPS is supported
        Ztring Profile=Ztring().From_Local(Avc_profile_idc(seq_parameter_set_id->second.profile_idc))+_T("@L")+Ztring().From_Number(((float)seq_parameter_set_id->second.level_idc)/10, 1);
        Fill(Stream_Video, 0, Video_Format_Profile, Profile);
        Fill(Stream_Video, 0, Video_Codec_Profile, Profile);
    }
    Fill(Stream_Video, StreamPos_Last, Video_Width, Width);
    Fill(Stream_Video, StreamPos_Last, Video_Height, Height);
    Fill(Stream_Video, 0, Video_Standard, Avc_video_format[video_format]);
    Fill(Stream_Video, 0, Video_PixelAspectRatio, PixelAspectRatio, 3, true);
    Fill(Stream_Video, 0, Video_DisplayAspectRatio, Width*PixelAspectRatio/Height, 3, true); //More precise
    if (timing_info_present_flag)
    {
        if (!fixed_frame_rate_flag)
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode, "VFR");
        else if (time_scale && num_units_in_tick)
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, (float)time_scale/num_units_in_tick/(frame_mbs_only_flag?2:(pic_order_cnt_type==2?1:2))/FrameRate_Divider);
    }
    if (FrameRate_Divider==2)
    {
        Fill(Stream_Video, StreamPos_Last, Video_Format_Settings_FrameMode, "Frame doubling");
        Fill(Stream_Video, StreamPos_Last, Video_Format_Settings, "Frame doubling");
    }
    if (FrameRate_Divider==3)
    {
        Fill(Stream_Video, StreamPos_Last, Video_Format_Settings_FrameMode, "Frame tripling");
        Fill(Stream_Video, StreamPos_Last, Video_Format_Settings, "Frame tripling");
    }
    Fill(Stream_Video, 0, Video_Colorimetry, Avc_Colorimetry_format_idc[chroma_format_idc]);

    //Interlacement
    if (mb_adaptive_frame_field_flag && Structure_Frame>0) //Interlaced macro-block
    {
        Fill(Stream_Video, 0, Video_ScanType, "MBAFF");
        Fill(Stream_Video, 0, Video_Interlacement, "MBAFF");
    }
    else if (frame_mbs_only_flag || Structure_Field==0) //No interlaced frame
    {
        Fill(Stream_Video, 0, Video_ScanType, "Progressive");
        Fill(Stream_Video, 0, Video_Interlacement, "PPF");
    }
    else
    {
        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
        Fill(Stream_Video, 0, Video_Interlacement, "Interlaced");
    }
    std::string TempRef, CodingType;
    for (size_t Pos=0; Pos<TemporalReference.size(); Pos++)
        if (TemporalReference[Pos].IsValid)
        {
            TempRef+=TemporalReference[Pos].IsTop?"T":"B";
            CodingType+=Avc_slice_type[TemporalReference[Pos].slice_type];
        }
    if (TempRef.find("TBTBTBTB")==0)
    {
        Fill(Stream_Video, 0, Video_ScanOrder, "TFF");
        Fill(Stream_Video, 0, Video_Interlacement, "TFF", Unlimited, true, true);
    }
    if (TempRef.find("BTBTBTBT")==0)
    {
        Fill(Stream_Video, 0, Video_ScanOrder, "BFF");
        Fill(Stream_Video, 0, Video_Interlacement, "BFF", Unlimited, true, true);
    }

    //GOP
    std::vector<Ztring> GOPs;
    size_t GOP_Frame_Count=0;
    size_t GOP_BFrames_Max=0;
    size_t I_Pos1=CodingType.find(_T('I'));
    while (I_Pos1!=std::string::npos)
    {
        size_t I_Pos2=CodingType.find(_T('I'), I_Pos1+1);
        if (I_Pos2!=std::string::npos)
        {
            std::vector<size_t> P_Positions;
            size_t P_Position=I_Pos1;
            do
            {
                P_Position=CodingType.find(_T('P'), P_Position+1);
                if (P_Position<I_Pos2)
                    P_Positions.push_back(P_Position);
            }
            while (P_Position<I_Pos2);
            Ztring GOP;
            if (!P_Positions.empty())
            {
                GOP+=_T("M=")+Ztring::ToZtring(P_Positions[0]-I_Pos1)+_T(", ");
                if (P_Positions[0]-I_Pos1>GOP_BFrames_Max)
                    GOP_BFrames_Max=P_Positions[0]-I_Pos1;
            }
            GOP+=_T("N=")+Ztring::ToZtring(I_Pos2-I_Pos1);
            GOPs.push_back(GOP);
            GOP_Frame_Count+=I_Pos2-I_Pos1;
        }
        I_Pos1=I_Pos2;
    }

    if (GOP_Frame_Count+GOP_BFrames_Max>Frame_Count && !GOPs.empty())
        GOPs.resize(GOPs.size()-1); //Removing the last one, there may have uncomplete B-frame filling

    if (!GOPs.empty())
    {
        size_t Unique=0;
        for (size_t Pos=1; Pos<GOPs.size(); Pos++)
            if (GOPs[Pos]!=GOPs[0])
                Unique++;
        if ((Frame_Count<Frame_Count_Valid*10 && Unique) || Unique>2) //In order to accept some unsynch //TODO: change the method, synching with next I-Frame
            GOPs.clear(); //Not a fixed GOP
    }
    if (!GOPs.empty())
        Fill(Stream_Video, 0, Video_Format_Settings_GOP, GOPs[0]);
    else if (CodingType.size()==1 && Frame_Count>1)
        Fill(Stream_Video, 0, Video_Format_Settings_GOP, "N=1"); //Only I-Frames, pic_order_cnt_lsb is always 0

    /*
    if (frame_mbs_only_flag)
    {
        Fill(Stream_Video, 0, Video_ScanType, "Progressive");
        Fill(Stream_Video, 0, Video_Interlacement, "PPF");
    }
    if (pic_struct_FirstDetected==1)
    {
        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
        Fill(Stream_Video, 0, Video_ScanOrder, "TFF");
        Fill(Stream_Video, 0, Video_Interlacement, "TFF");
    }
    if (pic_struct_FirstDetected==2)
    {
        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
        Fill(Stream_Video, 0, Video_ScanOrder, "BFF");
        Fill(Stream_Video, 0, Video_Interlacement, "BFF");
    }
    */
    Fill(Stream_Video, 0, Video_Encoded_Library, Encoded_Library);
    Fill(Stream_Video, 0, Video_Encoded_Library_Name, Encoded_Library_Name);
    Fill(Stream_Video, 0, Video_Encoded_Library_Version, Encoded_Library_Version);
    Fill(Stream_Video, 0, Video_Encoded_Library_Settings, Encoded_Library_Settings);
    Fill(Stream_Video, 0, Video_BitRate_Nominal, BitRate_Nominal);
    Fill(Stream_Video, 0, Video_MuxingMode, MuxingMode);
    if (entropy_coding_mode_flag)
    {
        Fill(Stream_Video, 0, Video_Format_Settings, "CABAC");
        Fill(Stream_Video, 0, Video_Format_Settings_CABAC, "Yes");
        Fill(Stream_Video, 0, Video_Codec_Settings, "CABAC");
        Fill(Stream_Video, 0, Video_Codec_Settings_CABAC, "Yes");
    }
    else
    {
        Fill(Stream_Video, 0, Video_Format_Settings_CABAC, "No");
        Fill(Stream_Video, 0, Video_Codec_Settings_CABAC, "No");
    }
    if (max_num_ref_frames>0)
    {
        Fill(Stream_Video, 0, Video_Format_Settings, Ztring::ToZtring(max_num_ref_frames)+_T(" Ref Frames"));
        Fill(Stream_Video, 0, Video_Codec_Settings, Ztring::ToZtring(max_num_ref_frames)+_T(" Ref Frames"));
    }
    if (max_num_ref_frames)
    {
        Fill(Stream_Video, 0, Video_Format_Settings_RefFrames, max_num_ref_frames);
        Fill(Stream_Video, 0, Video_Codec_Settings_RefFrames, max_num_ref_frames);
    }
    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
    if (bit_depth_luma_minus8==bit_depth_chroma_minus8)
        Fill(Stream_Video, 0, Video_Resolution, bit_depth_luma_minus8+8);

    //Colour description
    Fill(Stream_Video, 0, "colour_primaries", Avc_colour_primaries(colour_primaries));
    Fill(Stream_Video, 0, "transfer_characteristics", Avc_transfer_characteristics(transfer_characteristics));
    Fill(Stream_Video, 0, "matrix_coefficients", Avc_matrix_coefficients(matrix_coefficients));

    //Buffer
    int32u bit_rate_value=(int32u)-1;
    bool   bit_rate_value_IsValid=true;
    bool   cbr_flag=false;
    bool   cbr_flag_IsSet=false;
    bool   cbr_flag_IsValid=true;
    for (size_t Pos=0; Pos<NAL.size(); Pos++)
    {
        if (NAL[Pos].cpb_size_value!=(int32u)-1)
            Fill(Stream_Video, 0, Video_BufferSize, NAL[Pos].cpb_size_value);
        if (bit_rate_value!=(int32u)-1 && bit_rate_value!=NAL[Pos].bit_rate_value)
            bit_rate_value_IsValid=false;
        if (bit_rate_value==(int32u)-1)
            bit_rate_value=NAL[Pos].bit_rate_value;
        if (cbr_flag_IsSet==true && cbr_flag!=NAL[Pos].cbr_flag)
            cbr_flag_IsValid=false;
        if (cbr_flag_IsSet==0)
        {
            cbr_flag=NAL[Pos].cbr_flag;
            cbr_flag_IsSet=true;
        }
    }
    for (size_t Pos=0; Pos<VCL.size(); Pos++)
    {
        Fill(Stream_Video, 0, Video_BufferSize, VCL[Pos].cpb_size_value);
        if (bit_rate_value!=(int32u)-1 && bit_rate_value!=VCL[Pos].bit_rate_value)
            bit_rate_value_IsValid=false;
        if (bit_rate_value==(int32u)-1)
            bit_rate_value=VCL[Pos].bit_rate_value;
        if (cbr_flag_IsSet==true && cbr_flag!=VCL[Pos].cbr_flag)
            cbr_flag_IsValid=false;
        if (cbr_flag_IsSet==0)
        {
            cbr_flag=VCL[Pos].cbr_flag;
            cbr_flag_IsSet=true;
        }
    }
    if (cbr_flag_IsSet && cbr_flag_IsValid)
    {
        Fill(Stream_Video, 0, Video_BitRate_Mode, cbr_flag?"CBR":"VBR");
        if (bit_rate_value!=(int32u)-1 && bit_rate_value_IsValid)
            Fill(Stream_Video, 0, cbr_flag?Video_BitRate_Nominal:Video_BitRate_Maximum, bit_rate_value);
    }
}

//---------------------------------------------------------------------------
void File_Avc::Streams_Finish()
{
    if (PTS_End!=(int64u)-1)
        Fill(Stream_Video, 0, Video_Duration, float64_int64s(((float64)(PTS_End-PTS_Begin))/1000000));

    //GA94 captions
    for (size_t Pos=0; Pos<GA94_03_CC_Parsers.size(); Pos++)
        if (GA94_03_CC_Parsers[Pos] && GA94_03_CC_Parsers[Pos]->Status[IsAccepted])
        {
            Finish(GA94_03_CC_Parsers[Pos]);
            Merge(*GA94_03_CC_Parsers[Pos]);
            if (Pos<2)
                Fill(Stream_Text, StreamPos_Last, Text_ID, _T("608-")+Ztring::ToZtring(Pos));
            Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, _T("SCTE 128 / DTVCC Transport"));
        }

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Streams.clear();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Avc::FileHeader_Begin()
{
    if (!File__Analyze::FileHeader_Begin_0x000001())
        return false;

    if (!MustSynchronize)
    {
        Synched_Init();
        Buffer_TotalBytes_FirstSynched+=0;
        File_Offset_FirstSynched=File_Offset;
    }

    //All should be OK
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Avc::Synched_Test()
{
    //Trailing 0x00
    while(Buffer_Offset+3<=Buffer_Size && Buffer[Buffer_Offset]==0x00 && CC3(Buffer+Buffer_Offset)!=0x000001)
        Buffer_Offset++;

    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC3(Buffer+Buffer_Offset)!=0x000001)
        Synched=false;

    //Quick search
    if (Synched && !Header_Parser_QuickSearch())
        return false;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Avc::Synched_Init()
{
    //Count of a Packets
    Block_Count=0;
    pic_order_cnt_lsb_Old=(int32u)-1;
    Interlaced_Top=0;
    Interlaced_Bottom=0;
    Structure_Field=0;
    Structure_Frame=0;
    FrameRate_Divider=1;
    TemporalReference_Offset=0;
    TemporalReference_Offset_Moved=false;
    TemporalReference_GA94_03_CC_Offset=0;
    TemporalReference_Offset_pic_order_cnt_lsb_Last=(size_t)-1;

    //From seq_parameter_set
    pic_width_in_mbs_minus1=0;
    pic_height_in_map_units_minus1=0;
    log2_max_frame_num_minus4=0;
    log2_max_pic_order_cnt_lsb_minus4=0;
    num_units_in_tick=0;
    time_scale=0;
    chroma_format_idc=1;
    frame_crop_left_offset=0;
    frame_crop_right_offset=0;
    frame_crop_top_offset=0;
    frame_crop_bottom_offset=0;
    max_num_ref_frames=0;
    pic_order_cnt_type=0;
    pic_order_cnt_lsb_Last=(int32u)-1;
    bit_depth_luma_minus8=0;
    bit_depth_chroma_minus8=0;
    pic_order_cnt_lsb=(int32u)-1;
    cpb_cnt_minus1=0;
    initial_cpb_removal_delay=0;
    initial_cpb_removal_delay_offset=0;
    cpb_removal_delay=0;
    sar_width=0;
    sar_height=0;
    profile_idc=0;
    level_idc=0;
    aspect_ratio_idc=0xFF;
    video_format=5;
    initial_cpb_removal_delay_length_minus1=0;
    cpb_removal_delay_length_minus1=0;
    dpb_output_delay_length_minus1=0;
    time_offset_length=0;
    pic_struct=0;
    pic_struct_FirstDetected=(int8u)-1;
    colour_primaries=2;
    transfer_characteristics=2;
    matrix_coefficients=2;
    GA94_03_CC_IsPresent=false;
    frame_mbs_only_flag=false;
    timing_info_present_flag=false;
    pic_struct_present_flag=false;
    field_pic_flag=false;
    entropy_coding_mode_flag=false;
    NalHrdBpPresentFlag=false;
    VclHrdBpPresentFlag=false;
    CpbDpbDelaysPresentFlag=false;
    mb_adaptive_frame_field_flag=false;
    pic_order_present_flag=false;
    field_pic_flag_AlreadyDetected=false;
    Field_Count_AfterLastCompleFrame=false;
    RefFramesCount=0;

    //Default values
    Streams.resize(0x100);
    Streams[0x06].Searching_Payload=true; //sei
    Streams[0x07].Searching_Payload=true; //seq_parameter_set
    Streams[0x09].Searching_Payload=true; //access_unit_delimiter
    Streams[0x0C].Searching_Payload=true; //filler_data
    Streams[0x0F].Searching_Payload=true; //subset_seq_parameter_set
    for (int8u Pos=0xFF; Pos>=0xB9; Pos--)
        Streams[Pos].Searching_Payload=true; //Testing MPEG-PS

    //Options
    Option_Manage();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Avc::Read_Buffer_Unsynched()
{
    TemporalReference.clear();
    TemporalReference_Offset=0;
    TemporalReference_Offset_Moved=false;
    TemporalReference_GA94_03_CC_Offset=0;
    TemporalReference_Offset_pic_order_cnt_lsb_Last=(size_t)-1;
    RefFramesCount=0;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Avc::Header_Parse()
{
    //Specific case
    if (MustParse_SPS_PPS)
    {
        Header_Fill_Size(Element_Size);
        Header_Fill_Code((int64u)-1, "Specific");
        return;
    }

    //Parsing
    if (!SizedBlocks)
    {
        Skip_B3(                                                "sync");
        BS_Begin();
        Mark_0 ();
        Skip_S1( 2,                                             "nal_ref_idc");
        Get_S1 ( 5, nal_unit_type,                              "nal_unit_type");
        BS_End();
        if (!Header_Parser_Fill_Size())
        {
            Element_WaitForMoreData();
            return;
        }
    }
    else
    {
        int32u Size;
        switch (SizeOfNALU_Minus1)
        {
            case 0: {
                        int8u Size_;
                        Get_B1 (Size_,                          "size");
                        Size=Size_;
                    }
                    break;
            case 1: {
                        int16u Size_;
                        Get_B2 (Size_,                          "size");
                        Size=Size_;
                    }
                    break;
            case 2: {
                        int32u Size_;
                        Get_B3 (Size_,                          "size");
                        Size=Size_;
                    }
                    break;
            case 3:     Get_B4 (Size,                           "size");
                    break;
            default:    Trusted_IsNot("No size of NALU defined");
                        Size=(int32u)(Buffer_Size-Buffer_Offset);
        }
        BS_Begin();
        Mark_0 ();
        Skip_S1( 2,                                             "nal_ref_idc");
        Get_S1 ( 5, nal_unit_type,                              "nal_unit_type");
        BS_End();

        //Filling
        Header_Fill_Size(Element_Offset+Size-1);
    }

    //Filling
    Header_Fill_Code(nal_unit_type, Ztring().From_CC1(nal_unit_type));
}

//---------------------------------------------------------------------------
bool File_Avc::Header_Parser_Fill_Size()
{
    //Look for next Sync word
    if (Buffer_Offset_Temp==0) //Buffer_Offset_Temp is not 0 if Header_Parse_Fill_Size() has already parsed first frames
        Buffer_Offset_Temp=Buffer_Offset+4;
    while (Buffer_Offset_Temp+4<=Buffer_Size
        && CC3(Buffer+Buffer_Offset_Temp)!=0x000001)
    {
        Buffer_Offset_Temp+=2;
        while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]!=0x00)
            Buffer_Offset_Temp+=2;
        if (Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp-1]==0x00 || Buffer_Offset_Temp>=Buffer_Size)
            Buffer_Offset_Temp--;

        if (nal_unit_type==0x01 || nal_unit_type==0x05) //slice, we need only few bytes
        {
            if (Buffer_Offset_Temp-Buffer_Offset>20)
            {
                //OK, we continue, we have enough for a slice
                Header_Fill_Size(16);
                Buffer_Offset_Temp=0;
                return true;
            }
        }
    }

    //Must wait more data?
    if (Buffer_Offset_Temp+4>Buffer_Size)
    {
        if (FrameIsAlwaysComplete || File_Offset+Buffer_Size==File_Size)
            Buffer_Offset_Temp=Buffer_Size; //We are sure that the next bytes are a start
        else
            return false;
    }

    //Keeping out trailing zeroes
    if (Buffer_Offset_Temp+3<=Buffer_Size)
        while (CC3(Buffer+Buffer_Offset_Temp-1)==0x000000)
            Buffer_Offset_Temp--;

    //OK, we continue
    Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
    Buffer_Offset_Temp=0;
    return true;
}

//---------------------------------------------------------------------------
bool File_Avc::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+4<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x00
      &&   Buffer[Buffer_Offset+1]==0x00
      &&   Buffer[Buffer_Offset+2]==0x01)
    {
        //Getting start_code
        int8u start_code=CC1(Buffer+Buffer_Offset+3)&0x1F;

        //Searching start33
        if (Streams[start_code].Searching_Payload
         || Streams[start_code].ShouldDuplicate)
            return true;

        //Synchronizing
        Buffer_Offset+=4;
        Synched=false;
        if (!Synchronize_0x000001())
            return false;
    }

    if (Buffer_Offset+3==Buffer_Size)
        return false; //Sync is OK, but start_code is not available
    Trusted_IsNot("AVC, Synchronisation lost");
    return Synchronize();
}

//---------------------------------------------------------------------------
void File_Avc::Data_Parse()
{
    //Specific case
    if (Element_Code==(int64u)-1)
    {
        SPS_PPS();
        return;
    }

    //svc_extension
    if (Element_Code==0x0E || Element_Code==0x14)
    {
        BS_Begin();
        Get_SB (svc_extension_flag,                             "svc_extension_flag");
        if (svc_extension_flag)
            nal_unit_header_svc_extension();
        else
            nal_unit_header_mvc_extension();
        BS_End();
    }

    //Searching emulation_prevention_three_byte
    int8u* Buffer_3Bytes=NULL;
    const int8u* Save_Buffer=Buffer;
    int64u Save_File_Offset=File_Offset;
    size_t Save_Buffer_Offset=Buffer_Offset;
    int64u Save_Element_Size=Element_Size;
    size_t Element_Offset_3Bytes=(size_t)Element_Offset;
    std::vector<size_t> ThreeByte_List;
    while (Element_Offset_3Bytes+3<=Element_Size)
    {
        if (CC3(Buffer+Buffer_Offset+(size_t)Element_Offset_3Bytes)==0x000003)
            ThreeByte_List.push_back(Element_Offset_3Bytes+2);
        Element_Offset_3Bytes+=2;
        while(Element_Offset_3Bytes<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset_3Bytes]!=0x00)
            Element_Offset_3Bytes+=2;
        if (Element_Offset_3Bytes<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset_3Bytes-1]==0x00 || Element_Offset_3Bytes>=Element_Size)
            Element_Offset_3Bytes--;
    }

    if (!ThreeByte_List.empty())
    {
        //We must change the buffer for keeping out
        Element_Size=Save_Element_Size-ThreeByte_List.size();
        File_Offset+=Buffer_Offset;
        Buffer_Offset=0;
        Buffer_3Bytes=new int8u[(size_t)Element_Size];
        for (size_t Pos=0; Pos<=ThreeByte_List.size(); Pos++)
        {
            size_t Pos0=(Pos==ThreeByte_List.size())?(size_t)Save_Element_Size:(ThreeByte_List[Pos]);
            size_t Pos1=(Pos==0)?0:(ThreeByte_List[Pos-1]+1);
            size_t Buffer_3bytes_Begin=Pos1-Pos;
            size_t Save_Buffer_Begin  =Pos1;
            size_t Size=               Pos0-Pos1;
            std::memcpy(Buffer_3Bytes+Buffer_3bytes_Begin, Save_Buffer+Save_Buffer_Offset+Save_Buffer_Begin, Size);
        }
        Buffer=Buffer_3Bytes;
    }

    //Parsing
    switch (Element_Code)
    {
        case 0x00 : Element_Name("unspecified"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x01 : slice_layer_without_partitioning_non_IDR(); break;
        case 0x02 : Element_Name("slice_data_partition_a_layer"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x03 : Element_Name("slice_data_partition_b_layer"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x04 : Element_Name("slice_data_partition_c_layer"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x05 : slice_layer_without_partitioning_IDR(); break;
        case 0x06 : sei(); break;
        case 0x07 : seq_parameter_set(); break;
        case 0x08 : pic_parameter_set(); break;
        case 0x09 : access_unit_delimiter(); break;
        case 0x0A : Element_Name("end_of_seq"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x0B : Element_Name("end_of_stream"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x0C : filler_data(); break;
        case 0x0D : Element_Name("seq_parameter_set_extension"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x0E : prefix_nal_unit(); break;
        case 0x0F : subset_seq_parameter_set(); break;
        case 0x13 : Element_Name("slice_layer_without_partitioning"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x14 : slice_layer_extension(); break;
        default :
            if (Element_Code<0x18)
                Element_Name("reserved");
            else
                Element_Name("unspecified");
            Skip_XX(Element_Size-Element_Offset, "Data");
    }

    if (!ThreeByte_List.empty())
    {
        //We must change the buffer for keeping out
        Element_Size=Save_Element_Size;
        File_Offset=Save_File_Offset;
        Buffer_Offset=Save_Buffer_Offset;
        delete[] Buffer; Buffer=Save_Buffer;
        Buffer_3Bytes=NULL; //Same as Buffer...
        Element_Offset+=ThreeByte_List.size();
    }

    //Duplicate
    if (!Streams.empty() && Streams[(size_t)Element_Code].ShouldDuplicate)
        File__Duplicate_Write(Element_Code);
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
// Packet "01"
void File_Avc::slice_layer_without_partitioning_non_IDR()
{
    Element_Name("slice_layer_without_partitioning (non-IDR)");

    //Parsing
    slice_header();
}

//---------------------------------------------------------------------------
// Packet "05"
void File_Avc::slice_layer_without_partitioning_IDR()
{
    Element_Name("slice_layer_without_partitioning (IDR)");

    //Parsing
    slice_header();

    FILLING_BEGIN_PRECISE();
        //NextCode
        for (int8u Pos=0x01; Pos<=0x05; Pos++)
            NextCode_Add(Pos);
    FILLING_END();
}

//---------------------------------------------------------------------------
//
void File_Avc::slice_header()
{
    //Parsing
    pic_order_cnt_lsb=(int32u)-1;
    int32u first_mb_in_slice, slice_type, frame_num;
    bool   bottom_field_flag=0;
    BS_Begin();
    Get_UE (first_mb_in_slice,                                  "first_mb_in_slice");
    Get_UE (slice_type,                                         "slice_type"); if (slice_type<9) {Param_Info(Avc_slice_type[slice_type]);Element_Info(Avc_slice_type[slice_type]);}
    Skip_UE(                                                    "pic_parameter_set_id");
    Get_BS (log2_max_frame_num_minus4+4, frame_num,             "frame_num");
    if (!frame_mbs_only_flag)
    {
        TEST_SB_GET(field_pic_flag,                             "field_pic_flag");
            Get_SB (bottom_field_flag,                          "bottom_field_flag");
        TEST_SB_END();
    }
    if (Element_Code==5)
        Skip_UE(                                                "idr_pic_id");
    if (pic_order_cnt_type==0)
    {
        Get_BS (log2_max_pic_order_cnt_lsb_minus4+4, pic_order_cnt_lsb, "pic_order_cnt_lsb"); Element_Info(pic_order_cnt_lsb);
        if (pic_order_present_flag && !field_pic_flag)
            Skip_SE(                                            "delta_pic_order_cnt_bottom");
    }
    //TODO...
    BS_End();
    Skip_XX(Element_Size-Element_Offset,                        "ToDo...");
    if (!field_pic_flag)
        Element_Info("Frame");
    else
        Element_Info(bottom_field_flag?"Bottom":"Top");

    FILLING_BEGIN_PRECISE();
        //pic_struct
        if (field_pic_flag && pic_struct_FirstDetected==(int8u)-1)
            pic_struct_FirstDetected=bottom_field_flag?2:1; //2=BFF, 1=TFF

        //Saving some info
        if (pic_order_cnt_type==0)
        {
            if (field_pic_flag)
            {
                Structure_Field++;
                if (bottom_field_flag)
                    Interlaced_Bottom++;
                else
                    Interlaced_Top++;
            }
            else
                Structure_Frame++;

            // trying to know the order
            // first  1/4: no change
            // second 1/4: all is after 0
            // third  1/4: no change
            // fourth 1/4: all is before max_frame_num
            size_t max_frame_num=1<<(log2_max_frame_num_minus4+4)<<1;
            if (TemporalReference_Offset==0 || (!TemporalReference_Offset_Moved && pic_order_cnt_lsb==max_frame_num*3/4))
            {
                TemporalReference_Offset+=max_frame_num;
                TemporalReference_Offset_Moved=true;

                //Purging the start
                if (TemporalReference_Offset==2*max_frame_num)
                {
                    size_t Pos=0;
                    for(; Pos<TemporalReference.size(); Pos++)
                        if (TemporalReference[Pos].IsValid)
                            break;
                    if (Pos && Pos<TemporalReference.size())
                    {
                        TemporalReference.erase(TemporalReference.begin(), TemporalReference.begin()+Pos);
                        if (Pos<TemporalReference_Offset)
                            TemporalReference_Offset-=Pos;
                        else
                            TemporalReference_Offset=0;
                        if (Pos<TemporalReference_GA94_03_CC_Offset)
                            TemporalReference_GA94_03_CC_Offset-=Pos;
                        else
                            TemporalReference_GA94_03_CC_Offset=0;
                    }
                }

                //Purging too big array
                if (TemporalReference.size()>=max_frame_num*4)
                {
                    TemporalReference.erase(TemporalReference.begin(), TemporalReference.begin()+max_frame_num*2);
                    if (max_frame_num*2<TemporalReference_Offset)
                        TemporalReference_Offset-=max_frame_num*2;
                    if (max_frame_num*2<TemporalReference_GA94_03_CC_Offset)
                        TemporalReference_GA94_03_CC_Offset-=max_frame_num*2;
                    else
                        TemporalReference_GA94_03_CC_Offset=0;
                }
            }
            if ( TemporalReference_Offset_Moved && pic_order_cnt_lsb>=max_frame_num/4 && pic_order_cnt_lsb<=max_frame_num/2)
            {
                TemporalReference_Offset_Moved=false;
            }

            TemporalReference_Offset_pic_order_cnt_lsb_Last=TemporalReference_Offset-(TemporalReference_Offset_Moved && pic_order_cnt_lsb>=max_frame_num/2?max_frame_num:0)+pic_order_cnt_lsb;

            if (TemporalReference_Offset_pic_order_cnt_lsb_Last>=TemporalReference.size())
                TemporalReference.resize(TemporalReference_Offset_pic_order_cnt_lsb_Last+1);
            TemporalReference[TemporalReference_Offset_pic_order_cnt_lsb_Last].frame_num=frame_num;
            TemporalReference[TemporalReference_Offset_pic_order_cnt_lsb_Last].slice_type=(int8u)slice_type;
            TemporalReference[TemporalReference_Offset_pic_order_cnt_lsb_Last].IsTop=!bottom_field_flag;
            TemporalReference[TemporalReference_Offset_pic_order_cnt_lsb_Last].IsField=field_pic_flag;
            TemporalReference[TemporalReference_Offset_pic_order_cnt_lsb_Last].IsValid=true;
        }

        //Name
        if (Frame_Count && ((!frame_mbs_only_flag && Interlaced_Top==Interlaced_Bottom && field_pic_flag) || first_mb_in_slice!=0 || (Element_Code==0x14 && !seq_parameter_set_ids.empty())))
        {
            Frame_Count--;
            Frame_Count_InThisBlock--;
            if (slice_type==0 || slice_type==2 || slice_type==5 || slice_type==7) //IFrame or PFrame
                Field_Count_AfterLastCompleFrame=true;
        }
        else if (slice_type==0 || slice_type==2 || slice_type==5 || slice_type==7) //IFrame or PFrame
            Field_Count_AfterLastCompleFrame=false;
        Element_Info(Ztring::ToZtring(Frame_Count));

        //Counting
        if (File_Offset+Buffer_Offset+Element_Size==File_Size)
            Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
        Frame_Count++;
        Frame_Count_InThisBlock++;
        if (RefFramesCount<2 && (slice_type==0 || slice_type==2 || slice_type==5 || slice_type==7))
            RefFramesCount++;
        if (PTS!=(int64u)-1)
        {
            if (PTS_Begin==(int64u)-1 && (slice_type==2 || slice_type==7)) //IFrame
                PTS_Begin=PTS;
            if ((slice_type==0 || slice_type==2 || slice_type==5 || slice_type==7) && Frame_Count_InThisBlock<=1 && !Field_Count_AfterLastCompleFrame) //IFrame or PFrame
                PTS_End=PTS;
            if ((slice_type==0 || slice_type==2 || slice_type==5 || slice_type==7) || (Frame_Count_InThisBlock>=2 && RefFramesCount>=2)) //IFrame or PFrame or more than 2 RefFrame for BFrames
            {
                if (timing_info_present_flag && first_mb_in_slice==0)
                    PTS_End+=float64_int64s(((float64)1000000000)/((float)time_scale/num_units_in_tick/(pic_order_cnt_type==2?1:2)/FrameRate_Divider)/((!frame_mbs_only_flag && field_pic_flag)?2:1));
            }
        }

        //Duplicate
        if (Streams[(size_t)Element_Code].ShouldDuplicate)
            File__Duplicate_Write(Element_Code, pic_order_cnt_type==0?pic_order_cnt_lsb:frame_num);

        //Filling only if not already done
        if (Frame_Count==2 && !Status[IsAccepted])
            Accept("AVC");
        if (!Status[IsFilled] && ((!GA94_03_CC_IsPresent && Frame_Count>=Frame_Count_Valid) || Frame_Count>=Frame_Count_Valid*10)) //10 times the normal test
        {
            Fill("AVC");
            if (!Streams[(size_t)Element_Code].ShouldDuplicate && MediaInfoLib::Config.ParseSpeed_Get()<1.0)
                Finish("AVC");
        }
    FILLING_END();

    Synched=false; //We do not have the complete slice
}

//---------------------------------------------------------------------------
// Packet "06"
void File_Avc::sei()
{
    Element_Name("sei");

    //Parsing
    while(Element_Offset+1<Element_Size)
    {
        Element_Begin("sei message");
            sei_message();
        Element_End();
    }
    BS_Begin();
    Mark_1(                                                     );
    BS_End();
}

//---------------------------------------------------------------------------
void File_Avc::sei_message()
{
    //Parsing
    int32u payloadType=0, payloadSize=0;
    int8u payload_type_byte, payload_size_byte;
    Element_Begin("sei message header");
        do
        {
            Get_B1 (payload_type_byte,                          "payload_type_byte");
            payloadType+=payload_type_byte;
        }
        while(payload_type_byte==0xFF);
        do
        {
            Get_B1 (payload_size_byte,                          "payload_size_byte");
            payloadSize+=payload_size_byte;
        }
        while(payload_size_byte==0xFF);
    Element_End();

    int64u Element_Offset_Save=Element_Offset+payloadSize;
    int64u Element_Size_Save=Element_Size;
    Element_Size=Element_Offset_Save;
    switch (payloadType)
    {
        case  0 :   sei_message_buffering_period(); break;
        case  1 :   sei_message_pic_timing(payloadSize); break;
        case  4 :   sei_message_user_data_registered_itu_t_t35(); break;
        case  5 :   sei_message_user_data_unregistered(payloadSize); break;
        case  6 :   sei_message_recovery_point(); break;
        case 32 :   sei_message_mainconcept(payloadSize); break;
        default :
                    Element_Info("unknown");
                    Skip_XX(payloadSize,                        "data");
    }
    Element_Offset=Element_Offset_Save; //Positionning in the right place.
    Element_Size=Element_Size_Save; //Positionning in the right place.
}

//---------------------------------------------------------------------------
// SEI - 0
void File_Avc::sei_message_buffering_period()
{
    Element_Info("buffering_period");

    //Parsing
    BS_Begin();
    Skip_UE(                                                    "seq_parameter_set_id");
    if (NalHrdBpPresentFlag)
        for (int32u SchedSelIdx=0; SchedSelIdx<=cpb_cnt_minus1; SchedSelIdx++)
        {
            Get_S4 (initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay, "initial_cpb_removal_delay"); Param_Info(initial_cpb_removal_delay/90, " ms");
            Get_S4 (initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay_offset, "initial_cpb_removal_delay_offset"); Param_Info(initial_cpb_removal_delay_offset/90, " ms");
        }
    if (VclHrdBpPresentFlag)
        for (int32u SchedSelIdx=0; SchedSelIdx<=cpb_cnt_minus1; SchedSelIdx++)
        {
            Get_S4 (initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay, "initial_cpb_removal_delay"); Param_Info(initial_cpb_removal_delay/90, " ms");
            Get_S4 (initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay_offset, "initial_cpb_removal_delay_offset"); Param_Info(initial_cpb_removal_delay_offset/90, " ms");
        }
}

//---------------------------------------------------------------------------
// SEI - 1
void File_Avc::sei_message_pic_timing(int32u payloadSize)
{
    Element_Info("pic_timing");

    //Testing if we can parsing it now
    if (!SPS_IsParsed) //There is sometimes one problem in one message, should not untrusting all
    {
        Skip_XX(payloadSize,                                    "Data");
        return;
    }

    //Parsing
    BS_Begin();
    if (CpbDpbDelaysPresentFlag)
    {
        Get_S4 (cpb_removal_delay_length_minus1+1, cpb_removal_delay, "cpb_removal_delay");
        Skip_S4(dpb_output_delay_length_minus1+1,               "dpb_output_delay");
    }
    if (pic_struct_present_flag)
    {
        Get_S1 (4, pic_struct,                                  "pic_struct");
        switch (pic_struct)
        {
            case  0 :
            case  1 :
            case  2 :
            case  3 :
            case  4 :
            case  5 :
            case  6 : break;
            case  7 : FrameRate_Divider=2; break;
            case  8 : FrameRate_Divider=3; break;
            default : Param_Info("Reserved"); return; //NumClockTS is unknown
        }
        Param_Info(Avc_pic_struct[pic_struct]);
        int8u NumClockTS=Avc_NumClockTS[pic_struct];
        int8u seconds_value=0, minutes_value=0, hours_value=0; //Here because theses values can be reused in later ClockTSs.
        for (int8u i=0; i<NumClockTS; i++)
        {
            Element_Begin("ClockTS");
            TEST_SB_SKIP(                                       "clock_timestamp_flag");
                Ztring TimeStamp;
                int32u time_offset=0;
                int8u n_frames;
                bool full_timestamp_flag, nuit_field_based_flag;
                Info_S1(2, ct_type,                             "ct_type"); Param_Info(Avc_ct_type[ct_type]);
                Get_SB (   nuit_field_based_flag,               "nuit_field_based_flag");
                Skip_S1(5,                                      "counting_type");
                Get_SB (   full_timestamp_flag,                 "full_timestamp_flag");
                Skip_SB(                                        "discontinuity_flag");
                Skip_SB(                                        "cnt_dropped_flag");
                Get_S1 (8, n_frames,                            "n_frames");
                if (full_timestamp_flag)
                {
                    Get_S1 (6, seconds_value,                    "seconds_value");
                    Get_S1 (6, minutes_value,                    "minutes_value");
                    Get_S1 (5, hours_value,                      "hours_value");
                }
                else
                {
                    TEST_SB_SKIP(                               "seconds_flag");
                        Get_S1 (6, seconds_value,               "seconds_value");
                        TEST_SB_SKIP(                           "minutes_flag");
                            Get_S1 (6, minutes_value,           "minutes_value");
                            TEST_SB_SKIP(                       "hours_flag");
                                Get_S1 (5, hours_value,         "hours_value");
                            TEST_SB_END();
                        TEST_SB_END();
                    TEST_SB_END();
                }
                TimeStamp=Ztring::ToZtring(hours_value)+_T(':')+Ztring::ToZtring(minutes_value)+_T(':')+Ztring::ToZtring(seconds_value);
                if (time_offset_length>0)
                    Get_S4 (time_offset_length, time_offset,    "time_offset");
                if (time_scale)
                {
                    float32 Milliseconds=((float32)(n_frames*(num_units_in_tick*(1+(nuit_field_based_flag?1:0)))+time_offset))/time_scale;
                    TimeStamp+=_T('.');
                    TimeStamp+=Ztring::ToZtring(Milliseconds);
                }
                Param_Info(TimeStamp);
            TEST_SB_END();
            Element_End();
        }
    }
    BS_End();

    FILLING_BEGIN_PRECISE();
        if (pic_struct_FirstDetected==(int8u)-1)
            pic_struct_FirstDetected=pic_struct;
    FILLING_END();
}

//---------------------------------------------------------------------------
// SEI - 5
void File_Avc::sei_message_user_data_registered_itu_t_t35()
{
    Element_Info("user_data_registered_itu_t_t35");

    //Parsing
    int8u itu_t_t35_country_code;
    Get_B1 (itu_t_t35_country_code,                             "itu_t_t35_country_code");
    if (itu_t_t35_country_code==0xFF)
        Skip_B1(                                                "itu_t_t35_country_code_extension_byte");
    if (itu_t_t35_country_code!=0xB5 || Element_Offset+2>=Element_Size)
    {
        if (Element_Size-Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "Unknown");
        return;
    }

    //United-States
    int16u id;
    Get_B2 (id,                                                 "id?");
    if (id!=0x0031 || Element_Offset+4>=Element_Size)
    {
        if (Element_Size-Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "Unknown");
        return;
    }

    int32u Identifier;
    Peek_B4(Identifier);
    switch (Identifier)
    {
        case 0x44544731 :   sei_message_user_data_registered_itu_t_t35_DTG1(); return;
        case 0x47413934 :   sei_message_user_data_registered_itu_t_t35_GA94(); return;
        default         :   if (Element_Size-Element_Offset)
                                Skip_XX(Element_Size-Element_Offset, "Unknown");
    }
}

//---------------------------------------------------------------------------
// SEI - 5 - DTG1
void File_Avc::sei_message_user_data_registered_itu_t_t35_DTG1()
{
    Element_Info("Active Format Description");

    //Parsing
    bool active_format_flag;
    Skip_C4(                                                    "afd_identifier");
    BS_Begin();
    Mark_0();
    Get_SB (active_format_flag,                                 "active_format_flag");
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_0_NoTrustError();
    Mark_1_NoTrustError();
    if (active_format_flag)
    {
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Info_S1(4, active_format,                               "active_format"); Param_Info(Avc_user_data_DTG1_active_format[active_format]);
    }
    BS_End();
}

//---------------------------------------------------------------------------
// SEI - 5 - GA94
void File_Avc::sei_message_user_data_registered_itu_t_t35_GA94()
{
    //Parsing
    int8u user_data_type_code;
    Skip_B4(                                                    "GA94_identifier");
    Get_B1 (user_data_type_code,                                "user_data_type_code");
    switch (user_data_type_code)
    {
        case 0x03 : sei_message_user_data_registered_itu_t_t35_GA94_03(); break;
        case 0x06 : sei_message_user_data_registered_itu_t_t35_GA94_06(); break;
        default   : Skip_XX(Element_Size-Element_Offset,        "GA94_reserved_user_data");
    }
}

//---------------------------------------------------------------------------
// SEI - 5 - GA94 - 0x03
void File_Avc::sei_message_user_data_registered_itu_t_t35_GA94_03()
{
    //Saving date in the right pic_order_cnt_lsb
    if (pic_order_cnt_lsb!=(int32u)-1)
    {
        if (TemporalReference_Offset_pic_order_cnt_lsb_Last<=TemporalReference.size())
            TemporalReference[TemporalReference_Offset_pic_order_cnt_lsb_Last].GA94_03_CC=TemporalReference_Temp.GA94_03_CC;
    }

    GA94_03_CC_IsPresent=true;

    Element_Info("Styled captioning");

    //Handling missing frames
    size_t max_frame_num=1<<(log2_max_frame_num_minus4+4)<<1;
    if (TemporalReference_GA94_03_CC_Offset+max_frame_num/2<TemporalReference_Offset-(TemporalReference_Offset_Moved && pic_order_cnt_lsb>=max_frame_num/2?max_frame_num:0)+pic_order_cnt_lsb)
    {
        size_t Pos=TemporalReference_Offset+pic_order_cnt_lsb;
        for(; Pos<TemporalReference.size(); Pos++)
            if (!TemporalReference[Pos].IsValid)
                break;
        TemporalReference_GA94_03_CC_Offset=Pos+1;
    }

    //Parsing
    int8u  cc_count;
    bool   process_em_data_flag, process_cc_data_flag, additional_data_flag;
    BS_Begin();
    Get_SB (process_em_data_flag,                               "process_em_data_flag");
    Get_SB (process_cc_data_flag,                               "process_cc_data_flag");
    Get_SB (additional_data_flag,                               "additional_data_flag");
    Get_S1 (5, cc_count,                                        "cc_count");
    BS_End();
    Skip_B1(                                                    process_em_data_flag?"em_data":"junk"); //Emergency message
    if (TemporalReference_Temp.GA94_03_CC.size()<cc_count)
        TemporalReference_Temp.GA94_03_CC.resize(cc_count);
    if (process_cc_data_flag)
    {
        for (int8u Pos=0; Pos<cc_count; Pos++)
        {
            Element_Begin("cc");
            int8u cc_type, cc_data_1, cc_data_2;
            bool   cc_valid;
            BS_Begin();
            Mark_1();
            Mark_1();
            Mark_1();
            Mark_1();
            Mark_1();
            Get_SB (   cc_valid,                                    "cc_valid");
            Get_S1 (2, cc_type,                                     "cc_type");
            BS_End();
            Get_B1 (cc_data_1,                                      "cc_data_1");
            Get_B1 (cc_data_2,                                      "cc_data_2");
            TemporalReference_Temp.GA94_03_CC[Pos].cc_valid=cc_valid;
            TemporalReference_Temp.GA94_03_CC[Pos].cc_type=cc_type;
            TemporalReference_Temp.GA94_03_CC[Pos].cc_data[0]=cc_data_1;
            TemporalReference_Temp.GA94_03_CC[Pos].cc_data[1]=cc_data_2;
            Element_End();
        }
    }
    else
        Skip_XX(cc_count*2,                                         "Junk");

    //Parsing Captions after reordering
    bool CanBeParsed=true;
    for (size_t GA94_03_CC_Pos=TemporalReference_GA94_03_CC_Offset; GA94_03_CC_Pos<TemporalReference.size(); GA94_03_CC_Pos+=2)
        if (!TemporalReference[GA94_03_CC_Pos].IsValid)
            CanBeParsed=false; //There is a missing field/frame
    if (CanBeParsed)
    {
       for (size_t GA94_03_CC_Pos=TemporalReference_GA94_03_CC_Offset; GA94_03_CC_Pos<TemporalReference.size(); GA94_03_CC_Pos+=2)
            for (int8u Pos=0; Pos<TemporalReference[GA94_03_CC_Pos].GA94_03_CC.size(); Pos++)
            {
                if (TemporalReference[GA94_03_CC_Pos].GA94_03_CC[Pos].cc_valid)
                {
                    int8u cc_type=TemporalReference[GA94_03_CC_Pos].GA94_03_CC[Pos].cc_type;
                    size_t Parser_Pos=cc_type;
                    if (Parser_Pos==3)
                        Parser_Pos=2; //cc_type 2 and 3 are for the same text

                    while (Parser_Pos>=GA94_03_CC_Parsers.size())
                        GA94_03_CC_Parsers.push_back(NULL);
                    if (GA94_03_CC_Parsers[Parser_Pos]==NULL)
                    {
                        if (cc_type<2)
                        {
                            GA94_03_CC_Parsers[Parser_Pos]=new File_Eia608();
                        }
                        else
                            GA94_03_CC_Parsers[Parser_Pos]=new File_Eia708();
                    }
                    if (!GA94_03_CC_Parsers[Parser_Pos]->Status[IsFinished])
                    {
                        if (cc_type>=2)
                            ((File_Eia708*)GA94_03_CC_Parsers[2])->cc_type=cc_type;
                        Element_Begin(Ztring(_T("ReorderedCaptions,"))+Ztring().From_Local(Avc_user_data_GA94_cc_type(cc_type)));
                        Open_Buffer_Init(GA94_03_CC_Parsers[Parser_Pos]);
                        if (cc_type==1)
                        Open_Buffer_Continue(GA94_03_CC_Parsers[Parser_Pos], TemporalReference[GA94_03_CC_Pos].GA94_03_CC[Pos].cc_data, 2);
                        Element_End();
                    }

                    //Demux
                    if (cc_type<2)
                        Demux(TemporalReference[GA94_03_CC_Pos].GA94_03_CC[Pos].cc_data, 2, ContentType_MainStream);
                    else
                        Demux(TemporalReference[GA94_03_CC_Pos].GA94_03_CC[Pos].cc_data, 2, ContentType_MainStream);
                }
            }

        TemporalReference_GA94_03_CC_Offset=TemporalReference.size()+1;
    }

    BS_Begin();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    BS_End();

    if (additional_data_flag)
        Skip_XX(Element_Size-Element_Offset,                    "additional_user_data");
}

//---------------------------------------------------------------------------
// SEI - 5 - GA94 - 0x03
void File_Avc::sei_message_user_data_registered_itu_t_t35_GA94_06()
{
    Element_Info("Bar data");

    //Parsing
    bool   top_bar_flag, bottom_bar_flag, left_bar_flag, right_bar_flag;
    BS_Begin();
    Get_SB (top_bar_flag,                                       "top_bar_flag");
    Get_SB (bottom_bar_flag,                                    "bottom_bar_flag");
    Get_SB (left_bar_flag,                                      "left_bar_flag");
    Get_SB (right_bar_flag,                                     "right_bar_flag");
    Mark_1_NoTrustError();
    Mark_1_NoTrustError();
    Mark_1_NoTrustError();
    Mark_1_NoTrustError();
    BS_End();
    if (top_bar_flag)
    {
        Mark_1();
        Mark_1();
        Skip_S2(14,                                             "line_number_end_of_top_bar");
    }
    if (bottom_bar_flag)
    {
        Mark_1();
        Mark_1();
        Skip_S2(14,                                             "line_number_start_of_bottom_bar");
    }
    if (left_bar_flag)
    {
        Mark_1();
        Mark_1();
        Skip_S2(14,                                             "pixel_number_end_of_left_bar");
    }
    if (right_bar_flag)
    {
        Mark_1();
        Mark_1();
        Skip_S2(14,                                             "pixel_number_start_of_right_bar");
    }
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    Mark_1();
    BS_End();

    if (Element_Size-Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                    "additional_bar_data");
}

//---------------------------------------------------------------------------
// SEI - 5
void File_Avc::sei_message_user_data_unregistered(int32u payloadSize)
{
    Element_Info("user_data_unregistered");

    //Parsing
    int128u uuid_iso_iec_11578;
    Get_GUID(uuid_iso_iec_11578,                               "uuid_iso_iec_11578");

    switch (uuid_iso_iec_11578.hi)
    {
        case  0xB748D9E6BDE945DCLL : Element_Info("x264");
                                     sei_message_user_data_unregistered_x264(payloadSize-16); break;
        case  0x684E92AC604A57FBLL : Element_Info("eavc");
                                     sei_message_user_data_unregistered_x264(payloadSize-16); break;
        default :
                    Element_Info("unknown");
                    Skip_XX(payloadSize-8,                      "data");
    }
}

//---------------------------------------------------------------------------
// SEI - 5 - x264
void File_Avc::sei_message_user_data_unregistered_x264(int32u payloadSize)
{
    //Parsing
    Ztring Data;
    Peek_Local(payloadSize, Data);
    if (Data.size()!=payloadSize && Data.size()+1!=payloadSize)
    {
        Skip_XX(payloadSize,                                    "Unknown");
        return; //This is not a text string
    }
    size_t Data_Pos;
    size_t Data_Pos_Before=0;
    size_t Loop=0;
    do
    {
        Data_Pos=Data.find(_T(" - "), Data_Pos_Before);
        if (Data_Pos==std::string::npos)
            Data_Pos=Data.size();
        if (Data.find(_T("options: "), Data_Pos_Before)==Data_Pos_Before)
        {
            Element_Begin("options");
            size_t Options_Pos;
            size_t Options_Pos_Before=Data_Pos_Before;
            Encoded_Library_Settings.clear();
            do
            {
                Options_Pos=Data.find(_T(" "), Options_Pos_Before);
                if (Options_Pos==std::string::npos)
                    Options_Pos=Data.size();
                Ztring option;
                Get_Local (Options_Pos-Options_Pos_Before, option, "option");
                Options_Pos_Before=Options_Pos;
                do
                {
                    Ztring Separator;
                    Peek_Local(1, Separator);
                    if (Separator==_T(" "))
                    {
                        Skip_Local(1,                               "separator");
                        Options_Pos_Before+=1;
                    }
                    else
                        break;
                }
                while (Options_Pos_Before!=Data.size());

                //Filling
                if (option!=_T("options:"))
                {
                    if (!Encoded_Library_Settings.empty())
                        Encoded_Library_Settings+=_T(" / ");
                    Encoded_Library_Settings+=option;
                    if (option.find(_T("bitrate="))==0)
                        BitRate_Nominal=option.substr(8)+_T("000"); //After "bitrate="
                }
            }
            while (Options_Pos_Before!=Data.size());
            Element_End();
        }
        else
        {
            Ztring Value;
            Get_Local(Data_Pos-Data_Pos_Before, Value,          "data");

            //Saving
            if (Loop==0)
            {
                //Cleaning a little the value
                while (!Value.empty() && Value[0]<0x30)
                    Value.erase(Value.begin());
                while (!Value.empty() && Value[Value.size()-1]<0x30)
                    Value.erase(Value.end()-1);
                Encoded_Library=Value;
            }
            if (Loop==1 && Encoded_Library.find(_T("x264"))==0)
            {
                Encoded_Library+=_T(" - ");
                Encoded_Library+=Value;
            }
        }
        Data_Pos_Before=Data_Pos;
        if (Data_Pos_Before+3<=Data.size())
        {
            Skip_Local(3,                                       "separator");
            Data_Pos_Before+=3;
        }

        Loop++;
    }
    while (Data_Pos_Before!=Data.size());

    //Encoded_Library
    if (Encoded_Library.find(_T("eavc "))==0)
    {
        Encoded_Library_Name=_T("eavc");
        Encoded_Library_Version=Encoded_Library.SubString(_T("eavc "), _T(""));
    }
    else if (Encoded_Library.find(_T("x264 - "))==0)
    {
        Encoded_Library_Name=_T("x264");
        Encoded_Library_Version=Encoded_Library.SubString(_T("x264 - "), _T(""));
    }
    else if (Encoded_Library.find(_T("SUPER(C) by eRightSoft "))==0)
    {
        Encoded_Library_Name=_T("SUPER(C) by eRightSoft");
        Encoded_Library_Date=Ztring(_T("UTC "))+Encoded_Library.SubString(_T("2000-"), _T(" "));
    }
    else
        Encoded_Library_Name=Encoded_Library;
}

//---------------------------------------------------------------------------
// SEI - 6
void File_Avc::sei_message_recovery_point()
{
    Element_Info("recovery_point");

    //Parsing
    BS_Begin();
    Skip_UE(                                                    "recovery_frame_cnt");
    Skip_SB(                                                    "exact_match_flag");
    Skip_SB(                                                    "broken_link_flag");
    Skip_S1(2,                                                  "changing_slice_group_idc");
    BS_End();
}

//---------------------------------------------------------------------------
// SEI - 32
void File_Avc::sei_message_mainconcept(int32u payloadSize)
{
    Element_Info("MainConcept text");

    //Parsing
    Ztring Text;
    Get_Local(payloadSize, Text,                                "text");

    if (Text.find(_T("produced by MainConcept H.264/AVC Codec v"))!=std::string::npos)
    {
        Encoded_Library=Text.SubString(_T("produced by "), _T(" MainConcept AG"));
        Encoded_Library_Name=_T("MainConcept H.264/AVC Codec");
        Encoded_Library_Version=Text.SubString(_T("produced by MainConcept H.264/AVC Codec v"), _T(" (c) "));
        Encoded_Library_Date=MediaInfoLib::Config.Library_Get(InfoLibrary_Format_MainConcept_Avc, Encoded_Library_Version, InfoLibrary_Date);
    }
}

//---------------------------------------------------------------------------
// Packet "07"
void File_Avc::seq_parameter_set()
{
    Element_Name("seq_parameter_set");

    //parsing
    seq_parameter_set_data();
    Mark_1(                                                     );
    size_t BS_bits=Data_BS_Remain()%8;
    while (BS_bits)
    {
        Mark_0(                                                 );
        BS_bits--;
    }
    BS_End();

    //Hack for 00003.m2ts: There is a trailing 0x89, why?
    if (Element_Offset+1==Element_Size)
    {
        int8u ToTest;
        Peek_B1(ToTest);
        if (ToTest==0x98)
            Skip_B1(                                            "Unknown");

    }

    //Hack for : There is a trailing data, why?
    if (Element_Offset+4==Element_Size)
    {
        int32u ToTest;
        Peek_B4(ToTest);
        if (ToTest==0xE30633C0)
            Skip_B4(                                            "Unknown");
    }

    //NULL bytes
    while (Element_Offset<Element_Size)
    {
        int8u Null;
        Get_B1 (Null,                                           "NULL byte");
        if (Null)
            Trusted_IsNot("Should be NULL byte");
    }

    FILLING_BEGIN_PRECISE();
        //Filling
        seq_parameter_set_ids[seq_parameter_set_id].profile_idc=profile_idc;
        seq_parameter_set_ids[seq_parameter_set_id].level_idc=level_idc;

        //NextCode
        NextCode_Clear();
        NextCode_Add(0x08);

        //Autorisation of other streams
        Streams[0x08].Searching_Payload=true; //pic_parameter_set
        if (Streams[0x07].ShouldDuplicate)
            Streams[0x08].ShouldDuplicate=true; //pic_parameter_set
        Streams[0x0A].Searching_Payload=true; //end_of_seq
        if (Streams[0x07].ShouldDuplicate)
            Streams[0x0A].ShouldDuplicate=true; //end_of_seq
        Streams[0x0B].Searching_Payload=true; //end_of_stream
        if (Streams[0x07].ShouldDuplicate)
            Streams[0x0B].ShouldDuplicate=true; //end_of_stream

        //Setting as OK
        SPS_IsParsed=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "08"
void File_Avc::pic_parameter_set()
{
    Element_Name("pic_parameter_set");

    //Parsing
    int32u num_slice_groups_minus1;
    BS_Begin();
    Skip_UE(                                                    "pic_parameter_set_id");
    Skip_UE(                                                    "seq_parameter_set_id");
    Get_SB (entropy_coding_mode_flag,                           "entropy_coding_mode_flag");
    Get_SB (pic_order_present_flag,                             "pic_order_present_flag");
    Get_UE (num_slice_groups_minus1,                            "num_slice_groups_minus1");
    if (num_slice_groups_minus1>7)
    {
        Trusted_IsNot("num_slice_groups_minus1 too high");
        num_slice_groups_minus1=0;
    }
    if (num_slice_groups_minus1>0)
    {
        int32u slice_group_map_type;
        Get_UE (slice_group_map_type,                           "slice_group_map_type");
        if (slice_group_map_type==0)
        {
            for (int32u Pos=0; Pos<=num_slice_groups_minus1; Pos++)
                Skip_UE(                                        "run_length_minus1");
        }
        else if (slice_group_map_type==2)
        {
            for (int32u Pos=0; Pos<num_slice_groups_minus1; Pos++)
            {
                Skip_UE(                                        "top_left");
                Skip_UE(                                        "bottom_right");
            }
        }
        else if (slice_group_map_type==3
              || slice_group_map_type==4
              || slice_group_map_type==5)
        {
            Skip_SB(                                            "slice_group_change_direction_flag");
            Skip_UE(                                            "slice_group_change_rate_minus1");
        }
        else if (slice_group_map_type==6)
        {
            int32u pic_size_in_map_units_minus1;
            Get_UE (pic_size_in_map_units_minus1,               "pic_size_in_map_units_minus1");
            if(pic_size_in_map_units_minus1>(pic_width_in_mbs_minus1+1)*(pic_height_in_map_units_minus1+1))
            {
                Trusted_IsNot("pic_size_in_map_units_minus1 too high");
                pic_size_in_map_units_minus1=0;
            }
            #if defined (__mips__)       || defined (__mipsel__)
                int32u slice_group_id_Size=(int32u)(std::ceil(std::log((double)(num_slice_groups_minus1+1))/std::log((double)10))); //std::log is natural logarithm
            #else
                int32u slice_group_id_Size=(int32u)(std::ceil(std::log((float32)(num_slice_groups_minus1+1))/std::log((float32)10))); //std::log is natural logarithm
            #endif
            for (int32u Pos=0; Pos<=pic_size_in_map_units_minus1; Pos++)
                Skip_S4(slice_group_id_Size,                    "slice_group_id");
        }
    }
    Skip_UE(                                                    "num_ref_idx_l0_active_minus1");
    Skip_UE(                                                    "num_ref_idx_l1_active_minus1");
    Skip_SB(                                                    "weighted_pred_flag");
    Skip_S1(2,                                                  "weighted_bipred_idc");
    Skip_SE(                                                    "pic_init_qp_minus26");
    Skip_SE(                                                    "pic_init_qs_minus26");
    Skip_SE(                                                    "chroma_qp_index_offset");
    Skip_SB(                                                    "deblocking_filter_control_present_flag");
    Skip_SB(                                                    "constrained_intra_pred_flag");
    Skip_SB(                                                    "redundant_pic_cnt_present_flag");
    bool more_rbsp_data=false;
    if (Element_Size)
    {
        int64u Offset=Element_Size-1;
        while (Offset && Buffer[Buffer_Offset+(size_t)Offset]==0x00) //Searching if there are NULL bytes at the end of the data
            Offset--;
        size_t Bit_Pos=7;
        while (!(Buffer[Buffer_Offset+(size_t)Offset]&(1<<(7-Bit_Pos))))
            Bit_Pos--;
        if (Data_BS_Remain()>1+(7-Bit_Pos)+(Element_Size-Offset-1)*8)
            more_rbsp_data=true;
    }
    if (more_rbsp_data)
    {
        bool transform_8x8_mode_flag;
        Get_SB (transform_8x8_mode_flag,                        "transform_8x8_mode_flag");
        TEST_SB_SKIP(                                           "pic_scaling_matrix_present_flag");
        for (int8u Pos=0; Pos<6+(transform_8x8_mode_flag?2:0); Pos++ )
            {
                TEST_SB_SKIP(                                   "pic_scaling_list_present_flag");
                    scaling_list(Pos<6?16:64);
                TEST_SB_END();
            }
        TEST_SB_END();
        Skip_SE(                                                "second_chroma_qp_index_offset");
    }
    Mark_1(                                                     );
    BS_End();

    while (Element_Offset<Element_Size) //Not always removed from the stream, ie in MPEG-4
    {
        int8u Padding;
        Peek_B1(Padding);
        if (!Padding)
            Skip_B1(                                            "Padding");
        else
            break;
    }

    FILLING_BEGIN_PRECISE();
        //NextCode
        NextCode_Clear();
        NextCode_Add(0x05);
        NextCode_Add(0x06);
        if (!subset_seq_parameter_set_ids.empty())
            NextCode_Add(0x14); //slice_layer_extension

        //Autorisation of other streams
        if (!seq_parameter_set_ids.empty())
        {
            for (int8u Pos=0x01; Pos<=0x06; Pos++)
            {
                Streams[Pos].Searching_Payload=true; //Coded slice...
                if (Streams[0x08].ShouldDuplicate)
                    Streams[Pos].ShouldDuplicate=true;
            }
        }
        if (!subset_seq_parameter_set_ids.empty())
        {
            Streams[0x14].Searching_Payload=true; //slice_layer_extension
            if (Streams[0x08].ShouldDuplicate)
                Streams[0x14].ShouldDuplicate=true; //slice_layer_extension
        }

        //Setting as OK
        PPS_IsParsed=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "09"
void File_Avc::access_unit_delimiter()
{
    Element_Name("access_unit_delimiter");

    int8u primary_pic_type;
    BS_Begin();
    Get_S1 ( 3, primary_pic_type,                               "primary_pic_type"); Param_Info(Avc_primary_pic_type[primary_pic_type]);
    Mark_1(                                                     );
    BS_End();
}

//---------------------------------------------------------------------------
// Packet "09"
void File_Avc::filler_data()
{
    Element_Name("filler_data");

    while (Element_Offset<Element_Size)
    {
        int8u FF;
        Peek_B1(FF);
        if (FF!=0xFF)
            break;
        Element_Offset++;
    }
    BS_Begin();
    Mark_1(                                                     );
    BS_End();
}

//---------------------------------------------------------------------------
// Packet "0E"
void File_Avc::prefix_nal_unit()
{
    Element_Name("prefix_nal_unit");

    //Parsing
    if (svc_extension_flag)
    {
        Skip_XX(Element_Size-Element_Offset,                    "prefix_nal_unit_svc");
    }
}

//---------------------------------------------------------------------------
// Packet "0F"
void File_Avc::subset_seq_parameter_set()
{
    Element_Name("subset_seq_parameter_set");

    //Parsing
    seq_parameter_set_data();
    if (profile_idc==83 || profile_idc==86)
    {
        //bool svc_vui_parameters_present_flag;
        seq_parameter_set_svc_extension();
        /* The rest is not yet implemented
        Get_SB (svc_vui_parameters_present_flag,                "svc_vui_parameters_present_flag");
        if (svc_vui_parameters_present_flag)
            svc_vui_parameters_extension();
        */
    }
    else if (profile_idc==118 || profile_idc==128)
    {
        //bool mvc_vui_parameters_present_flag, additional_extension2_flag;
        Mark_1();
        seq_parameter_set_mvc_extension();
        /* The rest is not yet implemented
        Get_SB (mvc_vui_parameters_present_flag,                "mvc_vui_parameters_present_flag");
        if (mvc_vui_parameters_present_flag)
            mvc_vui_parameters_extension();
        Get_SB (additional_extension2_flag,                     "additional_extension2_flag");
        if (additional_extension2_flag)
        {
            //Not handled, should skip all bits except 1
            BS_End();
            return;
        }
        */
    }
    /* The rest is not yet implemented
    Mark_1(                                                     );
    */
    BS_End();

    FILLING_BEGIN();
        //Filling
        subset_seq_parameter_set_ids[seq_parameter_set_id].profile_idc=profile_idc;
        subset_seq_parameter_set_ids[seq_parameter_set_id].level_idc=level_idc;

        //NextCode
        NextCode_Clear();
        NextCode_Add(0x08);

        //Autorisation of other streams
        Streams[0x08].Searching_Payload=true; //pic_parameter_set
        if (Streams[0x0F].ShouldDuplicate)
            Streams[0x08].ShouldDuplicate=true; //pic_parameter_set
        Streams[0x0A].Searching_Payload=true; //end_of_seq
        if (Streams[0x0F].ShouldDuplicate)
            Streams[0x0A].ShouldDuplicate=true; //end_of_seq
        Streams[0x0B].Searching_Payload=true; //end_of_stream
        if (Streams[0x0F].ShouldDuplicate)
            Streams[0x0B].ShouldDuplicate=true; //end_of_stream
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "14"
void File_Avc::slice_layer_extension()
{
    Element_Name("slice_layer_extension");

    //Parsing
    if (svc_extension_flag)
    {
        Skip_XX(Element_Size-Element_Offset,                    "slice_header_in_scalable_extension + slice_data_in_scalable_extension");
    }
    else
    {
        slice_header();
        //slice_data();
    }
}

//***************************************************************************
// SubElements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Avc::seq_parameter_set_data()
{
    Get_B1 (profile_idc,                                        "profile_idc");
    BS_Begin();
    Element_Begin("constraints");
        Skip_SB(                                                "constraint_set0_flag");
        Skip_SB(                                                "constraint_set1_flag");
        Skip_SB(                                                "constraint_set2_flag");
        Skip_SB(                                                "constraint_set3_flag");
        Skip_SB(                                                "reserved_zero_4bits");
        Skip_SB(                                                "reserved_zero_4bits");
        Skip_SB(                                                "reserved_zero_4bits");
        Skip_SB(                                                "reserved_zero_4bits");
    Element_End();
    Get_S1 ( 8, level_idc,                                      "level_idc");
    Get_UE (    seq_parameter_set_id,                           "seq_parameter_set_id");
    if (profile_idc==100
     || profile_idc==110
     || profile_idc==122
     || profile_idc==244
     || profile_idc== 44
     || profile_idc== 83
     || profile_idc== 86
     || profile_idc==118
     || profile_idc==128) //High profiles
    {
        Element_Begin("high profile specific");
        Get_UE (chroma_format_idc,                              "chroma_format_idc");
        if (chroma_format_idc>3)
        {
            Trusted_IsNot("chroma_format_idc is too high");
            chroma_format_idc=1;
        }
        Param_Info(Avc_Colorimetry_format_idc[chroma_format_idc]);
        if (chroma_format_idc==3)
            Skip_SB(                                            "residual_colour_transform_flag");
        Get_UE (bit_depth_luma_minus8,                          "bit_depth_luma_minus8");
        Get_UE (bit_depth_chroma_minus8,                        "bit_depth_chroma_minus8");
        Skip_SB(                                                "qpprime_y_zero_transform_bypass_flag");
        TEST_SB_SKIP(                                           "seq_scaling_matrix_present_flag");
            for (int32u Pos=0; Pos<8; Pos++)
            {
                TEST_SB_SKIP(                                   "seq_scaling_list_present_flag");
                    scaling_list(Pos<6?16:64);
                TEST_SB_END();
            }
        TEST_SB_END();
        Element_End();
    }
    Get_UE (log2_max_frame_num_minus4,                          "log2_max_frame_num_minus4");
    Get_UE (pic_order_cnt_type,                                 "pic_order_cnt_type");
    if (pic_order_cnt_type==0)
        Get_UE (log2_max_pic_order_cnt_lsb_minus4,              "log2_max_pic_order_cnt_lsb_minus4");
    else if (pic_order_cnt_type==1)
    {
        int32u num_ref_frames_in_pic_order_cnt_cycle;
        Skip_SB(                                                "delta_pic_order_always_zero_flag");
        Skip_SE(                                                "offset_for_non_ref_pic");
        Skip_SE(                                                "offset_for_top_to_bottom_field");
        Get_UE (num_ref_frames_in_pic_order_cnt_cycle,          "num_ref_frames_in_pic_order_cnt_cycle");
        if (num_ref_frames_in_pic_order_cnt_cycle>=256)
        {
            Trusted_IsNot("num_ref_frames_in_pic_order_cnt_cycle too high");
            num_ref_frames_in_pic_order_cnt_cycle=0;
        }
        for(int32u Pos=0; Pos<num_ref_frames_in_pic_order_cnt_cycle; Pos++)
            Skip_SE(                                            "offset_for_ref_frame");
    }
    Get_UE (max_num_ref_frames,                                 "max_num_ref_frames");
    Skip_SB(                                                    "gaps_in_frame_num_value_allowed_flag");
    Get_UE (pic_width_in_mbs_minus1,                            "pic_width_in_mbs_minus1");
    Get_UE (pic_height_in_map_units_minus1,                     "pic_height_in_map_units_minus1");
    Get_SB (frame_mbs_only_flag,                                "frame_mbs_only_flag");
    if (!frame_mbs_only_flag)
        Get_SB (mb_adaptive_frame_field_flag,                   "mb_adaptive_frame_field_flag");
    Skip_SB(                                                    "direct_8x8_inference_flag");
    TEST_SB_SKIP(                                               "frame_cropping_flag");
        Get_UE (frame_crop_left_offset,                         "frame_crop_left_offset");
        Get_UE (frame_crop_right_offset,                        "frame_crop_right_offset");
        Get_UE (frame_crop_top_offset,                          "frame_crop_top_offset");
        Get_UE (frame_crop_bottom_offset,                       "frame_crop_bottom_offset");
    TEST_SB_END();
    TEST_SB_SKIP(                                               "vui_parameters_present_flag");
        vui_parameters();
    TEST_SB_END();
}

//---------------------------------------------------------------------------
void File_Avc::scaling_list(int32u ScalingList_Size)
{
    //From http://mpeg4ip.cvs.sourceforge.net/mpeg4ip/mpeg4ip/util/h264/main.cpp?revision=1.17&view=markup
    int32u lastScale=8, nextScale=8;
    for (int32u Pos=0; Pos<ScalingList_Size; Pos++)
    {
        if (nextScale!=0)
        {
            int32s delta_scale;
            Get_SE (delta_scale,                                "scale_delta");
            nextScale=(lastScale+delta_scale+256)%256;
        }
        if (nextScale)
            lastScale=nextScale;
    }
}

//---------------------------------------------------------------------------
void File_Avc::vui_parameters()
{
    bool nal_hrd_parameters_present_flag, vcl_hrd_parameters_present_flag;
    TEST_SB_SKIP(                                               "aspect_ratio_info_present_flag");
        Get_S1 (8, aspect_ratio_idc,                            "aspect_ratio_idc"); if (aspect_ratio_idc<Avc_PixelAspectRatio_Size) Param_Info(Avc_PixelAspectRatio[aspect_ratio_idc]);
        if (aspect_ratio_idc==0xFF)
        {
            Get_S2 (16, sar_width,                              "sar_width");
            Get_S2 (16, sar_height,                             "sar_height");
        }
    TEST_SB_END();
    TEST_SB_SKIP(                                               "overscan_info_present_flag");
        Skip_SB(                                                "overscan_appropriate_flag");
    TEST_SB_END();
    TEST_SB_SKIP(                                               "video_signal_type_present_flag");
        Get_S1 (3, video_format,                                "video_format"); Param_Info(Avc_video_format[video_format]);
        Skip_SB(                                                "video_full_range_flag");
        TEST_SB_SKIP(                                           "colour_description_present_flag");
            Get_S1 (8, colour_primaries,                        "colour_primaries"); Param_Info(Avc_colour_primaries(colour_primaries));
            Get_S1 (8, transfer_characteristics,                "transfer_characteristics"); Param_Info(Avc_transfer_characteristics(transfer_characteristics));
            Get_S1 (8, matrix_coefficients,                     "matrix_coefficients"); Param_Info(Avc_matrix_coefficients(matrix_coefficients));
        TEST_SB_END();
    TEST_SB_END();
    TEST_SB_SKIP(                                               "chroma_loc_info_present_flag");
        Skip_UE(                                                "chroma_sample_loc_type_top_field");
        Skip_UE(                                                "chroma_sample_loc_type_bottom_field");
    TEST_SB_END();
    TEST_SB_GET (timing_info_present_flag,                      "timing_info_present_flag");
        Get_S4 (32, num_units_in_tick,                          "num_units_in_tick");
        Get_S4 (32, time_scale,                                 "time_scale");
        Get_SB (fixed_frame_rate_flag,                          "fixed_frame_rate_flag");
    TEST_SB_END();
    TEST_SB_GET (nal_hrd_parameters_present_flag,               "nal_hrd_parameters_present_flag");
        hrd_parameters(false);
        NalHrdBpPresentFlag=true;
    TEST_SB_END();
    TEST_SB_GET (vcl_hrd_parameters_present_flag,               "vcl_hrd_parameters_present_flag");
        hrd_parameters(true);
        VclHrdBpPresentFlag=true;
    TEST_SB_END();
    if(nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
    {
        CpbDpbDelaysPresentFlag=true;
        Skip_SB(                                                "low_delay_hrd_flag");
    }
    Get_SB (pic_struct_present_flag,                            "pic_struct_present_flag");
    TEST_SB_SKIP(                                               "bitstream_restriction_flag");
        Skip_SB(                                                "motion_vectors_over_pic_boundaries_flag");
        Skip_UE(                                                "max_bytes_per_pic_denom");
        Skip_UE(                                                "max_bits_per_mb_denom");
        Skip_UE(                                                "log2_max_mv_length_horizontal");
        Skip_UE(                                                "log2_max_mv_length_vertical");
        Skip_UE(                                                "num_reorder_frames");
        Skip_UE(                                                "max_dec_frame_buffering");
    TEST_SB_END();
}

//---------------------------------------------------------------------------
void File_Avc::hrd_parameters(bool vcl)
{
    //Filling
    if (vcl)
        VCL.clear();
    else
        NAL.clear();

    //Parsing
    int8u  bit_rate_scale, cpb_size_scale;
    Get_UE (   cpb_cnt_minus1,                                  "cpb_cnt_minus1");
    Get_S1 (4, bit_rate_scale,                                  "bit_rate_scale");
    Get_S1 (4, cpb_size_scale,                                  "cpb_size_scale");
    if (cpb_cnt_minus1>31)
    {
        Trusted_IsNot("cpb_cnt_minus1 too high");
        cpb_cnt_minus1=0;
    }
    for (int32u SchedSelIdx=0; SchedSelIdx<=cpb_cnt_minus1; SchedSelIdx++)
    {
        Element_Begin("ShedSel");
        xxl Item;
        int32u bit_rate_value_minus1, cpb_size_value_minus1;
        Get_UE (bit_rate_value_minus1,                          "bit_rate_value_minus1");
        if (bit_rate_value_minus1)
            Item.bit_rate_value=(int32u)((bit_rate_value_minus1+1)*pow(2.0, 6+bit_rate_scale)); Param_Info(Item.bit_rate_value, " bps");
        Get_UE (cpb_size_value_minus1,                          "cpb_size_value_minus1");
        if (cpb_size_value_minus1)
            Item.cpb_size_value=(int32u)((cpb_size_value_minus1+1)*pow(2.0, cpb_size_scale)); Param_Info(Item.cpb_size_value, " bytes");
        Get_SB (Item.cbr_flag,                                  "cbr_flag");
        Element_End();

        //Filling
        if (vcl)
            VCL.push_back(Item);
        else
            NAL.push_back(Item);
    }
    Get_S1 (5, initial_cpb_removal_delay_length_minus1,         "initial_cpb_removal_delay_length_minus1");
    Get_S1 (5, cpb_removal_delay_length_minus1,                 "cpb_removal_delay_length_minus1");
    Get_S1 (5, dpb_output_delay_length_minus1,                  "dpb_output_delay_length_minus1");
    Get_S1 (5, time_offset_length,                              "time_offset_length");    
}

//---------------------------------------------------------------------------
void File_Avc::nal_unit_header_svc_extension()
{
    //Parsing
    Element_Begin("nal_unit_header_svc_extension");
    Skip_SB(                                                    "idr_flag");
    Skip_S1( 6,                                                 "priority_id");
    Skip_SB(                                                    "no_inter_layer_pred_flag");
    Skip_S1( 3,                                                 "dependency_id");
    Skip_S1( 4,                                                 "quality_id");
    Skip_S1( 3,                                                 "temporal_id");
    Skip_SB(                                                    "use_ref_base_pic_flag");
    Skip_SB(                                                    "discardable_flag");
    Skip_SB(                                                    "output_flag");
    Skip_S1( 2,                                                 "reserved_three_2bits");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Avc::nal_unit_header_mvc_extension()
{
    //Parsing
    Element_Begin("nal_unit_header_mvc_extension");
    Skip_SB(                                                    "non_idr_flag");
    Skip_S1( 6,                                                 "priority_id");
    Skip_S1(10,                                                 "view_id");
    Skip_S1( 3,                                                 "temporal_id");
    Skip_SB(                                                    "anchor_pic_flag");
    Skip_SB(                                                    "inter_view_flag");
    Skip_SB(                                                    "reserved_one_bit");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Avc::seq_parameter_set_svc_extension()
{
    //Parsing
    Element_Begin("seq_parameter_set_svc_extension");
    //Skip_SB(                                                    "");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Avc::svc_vui_parameters_extension()
{
    //Parsing
    Element_Begin("svc_vui_parameters_extension");
    //Skip_SB(                                                    "");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Avc::seq_parameter_set_mvc_extension()
{
    //Parsing
    Element_Begin("seq_parameter_set_mvc_extension");
    Get_UE (num_views_minus1,                                   "num_views_minus1");
    //Skip_SB(                                                    "");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Avc::mvc_vui_parameters_extension()
{
    //Parsing
    Element_Begin("mvc_vui_parameters_extension");
    Skip_SB(                                                    "");
    Element_End();
}

//***************************************************************************
// Specific
//***************************************************************************

//---------------------------------------------------------------------------
void File_Avc::SPS_PPS()
{
    //Parsing
    int8u Profile, Level, seq_parameter_set_count, pic_parameter_set_count;
    if (SizedBlocks)
        Skip_B1(                                                "Version");
    Get_B1 (Profile,                                            "Profile");
    Skip_B1(                                                    "Compatible profile");
    Get_B1 (Level,                                              "Level");
    BS_Begin();
    Skip_S1(6,                                                  "Reserved");
    Get_S1 (2, SizeOfNALU_Minus1,                               "Size of NALU length minus 1");
    Skip_S1(3,                                                  "Reserved");
    Get_S1 (5, seq_parameter_set_count,                         "seq_parameter_set count");
    BS_End();
    for (int8u Pos=0; Pos<seq_parameter_set_count; Pos++)
    {
        Element_Begin("seq_parameter_set");
        int16u Size;
        Get_B2 (Size,                                           "Size");
        BS_Begin();
        Mark_0 ();
        Skip_S1( 2,                                             "nal_ref_idc");
        Skip_S1( 5,                                             "nal_unit_type");
        BS_End();
        if (Element_Offset+Size-1>Element_Size)
        {
            Trusted_IsNot("Size is wrong");
            break; //There is an error
        }
        int64u Element_Offset_Save=Element_Offset;
        int64u Element_Size_Save=Element_Size;
        Buffer_Offset+=(size_t)Element_Offset_Save;
        Element_Offset=0;
        Element_Size=Size-1;
        Element_Code=0x07; //seq_parameter_set
        Data_Parse();
        Buffer_Offset-=(size_t)Element_Offset_Save;
        Element_Offset=Element_Offset_Save+Size-1;
        Element_Size=Element_Size_Save;
        Element_End();
    }
    Get_B1 (pic_parameter_set_count,                            "pic_parameter_set count");
    for (int8u Pos=0; Pos<pic_parameter_set_count; Pos++)
    {
        Element_Begin("pic_parameter_set");
        int16u Size;
        Get_B2 (Size,                                           "Size");
        BS_Begin();
        Mark_0 ();
        Skip_S1( 2,                                             "nal_ref_idc");
        Skip_S1( 5,                                             "nal_unit_type");
        BS_End();
        int64u Element_Offset_Save=Element_Offset;
        int64u Element_Size_Save=Element_Size;
        Buffer_Offset+=(size_t)Element_Offset_Save;
        Element_Offset=0;
        Element_Size=Size-1;
        if (Element_Size>Element_Size_Save-Element_Offset_Save)
            break; //There is an error
        Element_Code=0x08; //pic_parameter_set
        Data_Parse();
        Buffer_Offset-=(size_t)Element_Offset_Save;
        Element_Offset=Element_Offset_Save+Size-1;
        Element_Size=Element_Size_Save;
        Element_End();
    }
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding?");

    //Filling
    FILLING_BEGIN_PRECISE();
        //Detection of some bugs in the file
        if (Profile!=profile_idc || Level!=level_idc)
            MuxingMode=Ztring("Container profile=")+Ztring().From_Local(Avc_profile_idc(Profile))+_T("@")+Ztring().From_Number(((float)Level)/10, 1);

        MustParse_SPS_PPS=false;
        MustParse_SPS_PPS_Done=true;
        if (!Status[IsAccepted])
            Accept("AVC");
        if (MustParse_SPS_PPS_Only)
            Finish("AVC");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_AVC_YES

