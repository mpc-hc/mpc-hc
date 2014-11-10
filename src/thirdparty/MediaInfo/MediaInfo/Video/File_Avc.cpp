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
        case 138 : return "Multiview Depth High";
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
#if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #include "MediaInfo/Text/File_DtvccTransport.h"
#endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
#include <cstring>
#include <algorithm>
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
extern const int8u Avc_PixelAspectRatio_Size=17;
extern const float32 Avc_PixelAspectRatio[]=
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
extern const char* Avc_video_format[]=
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
extern const char* Avc_video_full_range[]=
{
    "Limited",
    "Full",
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
const char* Mpegv_colour_primaries(int8u colour_primaries);
const char* Mpegv_transfer_characteristics(int8u transfer_characteristics);
const char* Mpegv_matrix_coefficients(int8u matrix_coefficients);

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

//---------------------------------------------------------------------------
int32u Avc_MaxDpbMbs(int8u level)
{
    switch (level)
    {
        case  10 : return    1485;
        case  11 : return    3000;
        case  12 : return    6000;
        case  13 :
        case  20 : return   11880;
        case  21 : return   19800;
        case  22 : return   20250;
        case  30 : return   40500;
        case  31 : return  108000;
        case  32 : return  216000;
        case  40 :
        case  41 : return  245760;
        case  42 : return  522240;
        case  50 : return  589824;
        case  51 : return  983040;
        default  : return       0;
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Avc::File_Avc()
#if MEDIAINFO_DUPLICATE
:File__Duplicate()
#endif //MEDIAINFO_DUPLICATE
{
    //Config
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Avc;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;
    Frame_Count_NotParsedIncluded=0;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?512:2;
    FrameIsAlwaysComplete=false;
    MustParse_SPS_PPS=false;
    SizedBlocks=false;

    //Temporal references
    TemporalReferences_DelayedElement=NULL;

    //Text
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        GA94_03_Parser=NULL;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
}

//---------------------------------------------------------------------------
File_Avc::~File_Avc()
{
    for (size_t Pos=0; Pos<TemporalReferences.size(); Pos++)
        delete TemporalReferences[Pos]; //TemporalReferences[Pos]=NULL;
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        delete GA94_03_Parser; //GA94_03_Parser=NULL;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)

    for (size_t Pos=0; Pos<seq_parameter_sets.size(); Pos++)
        delete seq_parameter_sets[Pos]; //TemporalReferences[Pos]=NULL;

    for (size_t Pos=0; Pos<subset_seq_parameter_sets.size(); Pos++)
        delete subset_seq_parameter_sets[Pos]; //TemporalReferences[Pos]=NULL;

    for (size_t Pos=0; Pos<pic_parameter_sets.size(); Pos++)
        delete pic_parameter_sets[Pos]; //TemporalReferences[Pos]=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Avc::Streams_Fill()
{
    for (std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item=seq_parameter_sets.begin(); seq_parameter_set_Item!=seq_parameter_sets.end(); ++seq_parameter_set_Item)
        if ((*seq_parameter_set_Item))
            Streams_Fill(seq_parameter_set_Item);
    for (std::vector<seq_parameter_set_struct*>::iterator subset_seq_parameter_set_Item=subset_seq_parameter_sets.begin(); subset_seq_parameter_set_Item!=subset_seq_parameter_sets.end(); ++subset_seq_parameter_set_Item)
        if ((*subset_seq_parameter_set_Item))
        {
            if (seq_parameter_sets.empty())
                Streams_Fill(subset_seq_parameter_set_Item);
            else
                Streams_Fill_subset(subset_seq_parameter_set_Item);
            Fill(Stream_Video, 0, Video_MultiView_Count, (*subset_seq_parameter_set_Item)->num_views_minus1+1);
        }
}

//---------------------------------------------------------------------------
void File_Avc::Streams_Fill(std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item)
{
    //Calculating - Pixels
    int32u Width =((*seq_parameter_set_Item)->pic_width_in_mbs_minus1       +1)*16;
    int32u Height=((*seq_parameter_set_Item)->pic_height_in_map_units_minus1+1)*16*(2-(*seq_parameter_set_Item)->frame_mbs_only_flag);
    int32u CropUnitX=Avc_SubWidthC [(*seq_parameter_set_Item)->ChromaArrayType()];
    int32u CropUnitY=Avc_SubHeightC[(*seq_parameter_set_Item)->ChromaArrayType()]*(2-(*seq_parameter_set_Item)->frame_mbs_only_flag);
    Width -=((*seq_parameter_set_Item)->frame_crop_left_offset+(*seq_parameter_set_Item)->frame_crop_right_offset )*CropUnitX;
    Height-=((*seq_parameter_set_Item)->frame_crop_top_offset +(*seq_parameter_set_Item)->frame_crop_bottom_offset)*CropUnitY;

    //From vui_parameters
    float64 PixelAspectRatio=1;
    if ((*seq_parameter_set_Item)->vui_parameters)
    {
        if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_info_present_flag)
        {
            if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc<Avc_PixelAspectRatio_Size)
                PixelAspectRatio=Avc_PixelAspectRatio[(*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc];
            else if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc==0xFF && (*seq_parameter_set_Item)->vui_parameters->sar_height)
                PixelAspectRatio=((float64)(*seq_parameter_set_Item)->vui_parameters->sar_width)/(*seq_parameter_set_Item)->vui_parameters->sar_height;
        }
        if ((*seq_parameter_set_Item)->vui_parameters->timing_info_present_flag)
        {
            if (!(*seq_parameter_set_Item)->vui_parameters->fixed_frame_rate_flag)
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode, "VFR");
            else if ((*seq_parameter_set_Item)->vui_parameters->timing_info_present_flag && (*seq_parameter_set_Item)->vui_parameters->time_scale && (*seq_parameter_set_Item)->vui_parameters->num_units_in_tick)
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate, (float64)(*seq_parameter_set_Item)->vui_parameters->time_scale/(*seq_parameter_set_Item)->vui_parameters->num_units_in_tick/((*seq_parameter_set_Item)->frame_mbs_only_flag?2:((*seq_parameter_set_Item)->pic_order_cnt_type==2?1:2))/FrameRate_Divider);
        }

        //Colour description
        if ((*seq_parameter_set_Item)->vui_parameters->video_signal_type_present_flag)
        {
            Fill(Stream_Video, 0, Video_Standard, Avc_video_format[(*seq_parameter_set_Item)->vui_parameters->video_format]);
            Fill(Stream_Video, 0, Video_colour_range, Avc_video_full_range[(*seq_parameter_set_Item)->vui_parameters->video_full_range_flag]);
            if ((*seq_parameter_set_Item)->vui_parameters->colour_description_present_flag)
            {
                Fill(Stream_Video, 0, Video_colour_description_present, "Yes");
                Fill(Stream_Video, 0, Video_colour_primaries, Mpegv_colour_primaries((*seq_parameter_set_Item)->vui_parameters->colour_primaries));
                Fill(Stream_Video, 0, Video_transfer_characteristics, Mpegv_transfer_characteristics((*seq_parameter_set_Item)->vui_parameters->transfer_characteristics));
                Fill(Stream_Video, 0, Video_matrix_coefficients, Mpegv_matrix_coefficients((*seq_parameter_set_Item)->vui_parameters->matrix_coefficients));
            }
        }

        //hrd_parameter_sets
        int64u bit_rate_value=(int64u)-1;
        bool   bit_rate_value_IsValid=true;
        bool   cbr_flag=false;
        bool   cbr_flag_IsSet=false;
        bool   cbr_flag_IsValid=true;
        seq_parameter_set_struct::vui_parameters_struct::xxl* NAL=(*seq_parameter_set_Item)->vui_parameters->NAL;
        if (NAL)
            for (size_t Pos=0; Pos<NAL->SchedSel.size(); Pos++)
            {
                if (NAL->SchedSel[Pos].cpb_size_value!=(int32u)-1)
                    Fill(Stream_Video, 0, Video_BufferSize, NAL->SchedSel[Pos].cpb_size_value);
                if (bit_rate_value!=(int64u)-1 && bit_rate_value!=NAL->SchedSel[Pos].bit_rate_value)
                    bit_rate_value_IsValid=false;
                if (bit_rate_value==(int64u)-1)
                    bit_rate_value=NAL->SchedSel[Pos].bit_rate_value;
                if (cbr_flag_IsSet==true && cbr_flag!=NAL->SchedSel[Pos].cbr_flag)
                    cbr_flag_IsValid=false;
                if (cbr_flag_IsSet==0)
                {
                    cbr_flag=NAL->SchedSel[Pos].cbr_flag;
                    cbr_flag_IsSet=true;
                }
            }
        seq_parameter_set_struct::vui_parameters_struct::xxl* VCL=(*seq_parameter_set_Item)->vui_parameters->VCL;
        if (VCL)
            for (size_t Pos=0; Pos<VCL->SchedSel.size(); Pos++)
            {
                Fill(Stream_Video, 0, Video_BufferSize, VCL->SchedSel[Pos].cpb_size_value);
                if (bit_rate_value!=(int64u)-1 && bit_rate_value!=VCL->SchedSel[Pos].bit_rate_value)
                    bit_rate_value_IsValid=false;
                if (bit_rate_value==(int64u)-1)
                    bit_rate_value=VCL->SchedSel[Pos].bit_rate_value;
                if (cbr_flag_IsSet==true && cbr_flag!=VCL->SchedSel[Pos].cbr_flag)
                    cbr_flag_IsValid=false;
                if (cbr_flag_IsSet==0)
                {
                    cbr_flag=VCL->SchedSel[Pos].cbr_flag;
                    cbr_flag_IsSet=true;
                }
            }
        if (cbr_flag_IsSet && cbr_flag_IsValid)
        {
            Fill(Stream_Video, 0, Video_BitRate_Mode, cbr_flag?"CBR":"VBR");
            if (bit_rate_value!=(int64u)-1 && bit_rate_value_IsValid)
                Fill(Stream_Video, 0, cbr_flag?Video_BitRate_Nominal:Video_BitRate_Maximum, bit_rate_value);
        }
    }

    if (Count_Get(Stream_Video)==0)
        Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "AVC");
    Fill(Stream_Video, 0, Video_Codec, "AVC");

    Ztring Profile=Ztring().From_Local(Avc_profile_idc((*seq_parameter_set_Item)->profile_idc));
    switch ((*seq_parameter_set_Item)->profile_idc)
    {
        case  44 : // CAVLC 4:4:4 Intra
        case 100 : // High
        case 110 : // High 10
        case 122 : // High 4:2:2"
        case 244 : // High 4:4:4 Predictive
                    if ((*seq_parameter_set_Item)->constraint_set3_flag)
                        Profile+=__T(" Intra");
    }
    Profile+=__T("@L")+Ztring().From_Number(((float)(*seq_parameter_set_Item)->level_idc)/10, 1);
    Fill(Stream_Video, 0, Video_Format_Profile, Profile);
    Fill(Stream_Video, 0, Video_Codec_Profile, Profile);
    Fill(Stream_Video, StreamPos_Last, Video_Width, Width);
    Fill(Stream_Video, StreamPos_Last, Video_Height, Height);
    Fill(Stream_Video, 0, Video_PixelAspectRatio, PixelAspectRatio, 3, true);
    Fill(Stream_Video, 0, Video_DisplayAspectRatio, Width*PixelAspectRatio/Height, 3, true); //More precise
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

    //Interlacement
    if ((*seq_parameter_set_Item)->mb_adaptive_frame_field_flag && Structure_Frame>0) //Interlaced macro-block
    {
        Fill(Stream_Video, 0, Video_ScanType, "MBAFF");
        Fill(Stream_Video, 0, Video_Interlacement, "MBAFF");
    }
    else if ((*seq_parameter_set_Item)->frame_mbs_only_flag || (Structure_Frame>0 && Structure_Field==0)) //No interlaced frame
    {
        Fill(Stream_Video, 0, Video_ScanType, "Progressive");
        Fill(Stream_Video, 0, Video_Interlacement, "PPF");
    }
    else if (Structure_Field>0)
    {
        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
        Fill(Stream_Video, 0, Video_Interlacement, "Interlaced");
    }
    std::string ScanOrders, PictureTypes(PictureTypes_PreviousFrames);
    ScanOrders.reserve(TemporalReferences.size());
    for (size_t Pos=0; Pos<TemporalReferences.size(); Pos++)
        if (TemporalReferences[Pos])
        {
            ScanOrders+=TemporalReferences[Pos]->IsTop?'T':'B';
            if ((Pos%2)==0)
                PictureTypes+=Avc_slice_type[TemporalReferences[Pos]->slice_type];
        }
        else if (!PictureTypes.empty()) //Only if stream already started
        {
            ScanOrders+=' ';
            if ((Pos%2)==0)
                PictureTypes+=' ';
        }
    Fill(Stream_Video, 0, Video_ScanOrder, ScanOrder_Detect(ScanOrders));
    { //Legacy
        string Result=ScanOrder_Detect(ScanOrders);
        if (!Result.empty())
            Fill(Stream_Video, 0, Video_Interlacement, Result, true, true);
        else
        {
            switch ((*seq_parameter_set_Item)->pic_struct_FirstDetected)
            {
                case  1 :
                case  3 :
                            Fill(Stream_Video, 0, Video_ScanOrder, (string) "TFF");
                            break;
                case  2 :
                case  4 :
                            Fill(Stream_Video, 0, Video_ScanOrder, (string) "BFF");
                            break;
                default : ;
            }
        }
    }
    Fill(Stream_Video, 0, Video_Format_Settings_GOP, GOP_Detect(PictureTypes));

    Fill(Stream_General, 0, General_Encoded_Library, Encoded_Library);
    Fill(Stream_General, 0, General_Encoded_Library_Name, Encoded_Library_Name);
    Fill(Stream_General, 0, General_Encoded_Library_Version, Encoded_Library_Version);
    Fill(Stream_General, 0, General_Encoded_Library_Settings, Encoded_Library_Settings);
    Fill(Stream_Video, 0, Video_Encoded_Library, Encoded_Library);
    Fill(Stream_Video, 0, Video_Encoded_Library_Name, Encoded_Library_Name);
    Fill(Stream_Video, 0, Video_Encoded_Library_Version, Encoded_Library_Version);
    Fill(Stream_Video, 0, Video_Encoded_Library_Settings, Encoded_Library_Settings);
    Fill(Stream_Video, 0, Video_BitRate_Nominal, BitRate_Nominal);
    Fill(Stream_Video, 0, Video_MuxingMode, MuxingMode);
    for (std::vector<pic_parameter_set_struct*>::iterator pic_parameter_set_Item=pic_parameter_sets.begin(); pic_parameter_set_Item!=pic_parameter_sets.end(); ++pic_parameter_set_Item)
        if (*pic_parameter_set_Item && (*pic_parameter_set_Item)->seq_parameter_set_id==seq_parameter_set_Item-(seq_parameter_sets.empty()?subset_seq_parameter_sets.begin():seq_parameter_sets.begin()))
        {
            if ((*pic_parameter_set_Item)->entropy_coding_mode_flag)
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
            break; //TODO: currently, testing only the first pic_parameter_set
        }
    if ((*seq_parameter_set_Item)->max_num_ref_frames>0)
    {
        Fill(Stream_Video, 0, Video_Format_Settings, Ztring::ToZtring((*seq_parameter_set_Item)->max_num_ref_frames)+__T(" Ref Frames"));
        Fill(Stream_Video, 0, Video_Codec_Settings, Ztring::ToZtring((*seq_parameter_set_Item)->max_num_ref_frames)+__T(" Ref Frames"));
        Fill(Stream_Video, 0, Video_Format_Settings_RefFrames, (*seq_parameter_set_Item)->max_num_ref_frames);
        Fill(Stream_Video, 0, Video_Codec_Settings_RefFrames, (*seq_parameter_set_Item)->max_num_ref_frames);
    }
    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
    Fill(Stream_Video, 0, Video_Colorimetry, Avc_Colorimetry_format_idc[(*seq_parameter_set_Item)->chroma_format_idc]);
    if ((*seq_parameter_set_Item)->bit_depth_luma_minus8==(*seq_parameter_set_Item)->bit_depth_chroma_minus8)
        Fill(Stream_Video, 0, Video_BitDepth, (*seq_parameter_set_Item)->bit_depth_luma_minus8+8);
}

//---------------------------------------------------------------------------
void File_Avc::Streams_Fill_subset(const std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item)
{
    Ztring Profile=Ztring().From_Local(Avc_profile_idc((*seq_parameter_set_Item)->profile_idc))+__T("@L")+Ztring().From_Number(((float)(*seq_parameter_set_Item)->level_idc)/10, 1);
    Ztring Profile_Base=Retrieve(Stream_Video, 0, Video_Format_Profile);
    Fill(Stream_Video, 0, Video_Format_Profile, Profile, true);
    if (!Profile_Base.empty())
        Fill(Stream_Video, 0, Video_Format_Profile, Profile_Base);
}

//---------------------------------------------------------------------------
void File_Avc::Streams_Finish()
{
    if (PTS_End!=(int64u)-1 && (IsSub || File_Offset+Buffer_Offset+Element_Size==File_Size))
    {
        if (PTS_End>PTS_Begin)
            Fill(Stream_Video, 0, Video_Duration, float64_int64s(((float64)(PTS_End-PTS_Begin))/1000000));
    }

    //GA94 captions
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        if (GA94_03_Parser && GA94_03_Parser->Status[IsAccepted])
        {
            Finish(GA94_03_Parser);
            Merge(*GA94_03_Parser);

            Ztring LawRating=GA94_03_Parser->Retrieve(Stream_General, 0, General_LawRating);
            if (!LawRating.empty())
                Fill(Stream_General, 0, General_LawRating, LawRating, true);
            Ztring Title=GA94_03_Parser->Retrieve(Stream_General, 0, General_Title);
            if (!Title.empty() && Retrieve(Stream_General, 0, General_Title).empty())
                Fill(Stream_General, 0, General_Title, Title);

            for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
            {
                Ztring MuxingMode=Retrieve(Stream_Text, Pos, "MuxingMode");
                Fill(Stream_Text, Pos, "MuxingMode", __T("SCTE 128 / ")+MuxingMode, true);
            }
        }
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)

    #if MEDIAINFO_IBI
        if (seq_parameter_sets.size()==1 && (*seq_parameter_sets.begin())->vui_parameters && (*seq_parameter_sets.begin())->vui_parameters->timing_info_present_flag && (*seq_parameter_sets.begin())->vui_parameters->fixed_frame_rate_flag)
            Ibi_Stream_Finish((*seq_parameter_sets.begin())->vui_parameters->time_scale, (*seq_parameter_sets.begin())->vui_parameters->num_units_in_tick);
    #endif //MEDIAINFO_IBI
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
        Buffer_TotalBytes_FirstSynched=0;
        File_Offset_FirstSynched=File_Offset;
    }

    //All should be OK
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Avc::Synchronize()
{
    //Synchronizing
    size_t Buffer_Offset_Min=Buffer_Offset;
    while(Buffer_Offset+4<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                        || Buffer[Buffer_Offset+1]!=0x00
                                        || Buffer[Buffer_Offset+2]!=0x01))
    {
        Buffer_Offset+=2;
        while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
            Buffer_Offset+=2;
        if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x00)
            Buffer_Offset--;
    }
    if (Buffer_Offset>Buffer_Offset_Min && Buffer[Buffer_Offset-1]==0x00)
        Buffer_Offset--;

    //Parsing last bytes if needed
    if (Buffer_Offset+4==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x00
                                      || Buffer[Buffer_Offset+3]!=0x01))
        Buffer_Offset++;
    if (Buffer_Offset+3==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x01))
        Buffer_Offset++;
    if (Buffer_Offset+2==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00))
        Buffer_Offset++;
    if (Buffer_Offset+1==Buffer_Size &&  Buffer[Buffer_Offset  ]!=0x00)
        Buffer_Offset++;

    if (Buffer_Offset+4>Buffer_Size)
        return false;

    //Synched is OK
    Synched=true;
    return true;
}

//---------------------------------------------------------------------------
bool File_Avc::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+6>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x00
     || Buffer[Buffer_Offset+1]!=0x00
     || (Buffer[Buffer_Offset+2]!=0x01 && (Buffer[Buffer_Offset+2]!=0x00 || Buffer[Buffer_Offset+3]!=0x01)))
    {
        Synched=false;
        return true;
    }

    //Quick search
    if (!Header_Parser_QuickSearch())
        return false;

    #if MEDIAINFO_IBI
        bool zero_byte=Buffer[Buffer_Offset+2]==0x00;
        bool RandomAccess=(Buffer[Buffer_Offset+(zero_byte?4:3)]&0x1F)==0x07 || ((Buffer[Buffer_Offset+(zero_byte?4:3)]&0x1F)==0x09 && ((Buffer[Buffer_Offset+(zero_byte?5:4)]&0xE0)==0x00 || (Buffer[Buffer_Offset+(zero_byte?5:4)]&0xE0)==0xA0)); //seq_parameter_set or access_unit_delimiter with value=0 or 5 (3 bits)
        if (RandomAccess)
            Ibi_Add();
    #endif //MEDIAINFO_IBI

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Avc::Demux_UnpacketizeContainer_Test()
{
    const int8u*    Buffer_Temp=NULL;
    size_t          Buffer_Temp_Size=0;
    bool            RandomAccess=true; //Default, in case of problem

    if ((MustParse_SPS_PPS || SizedBlocks) && Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10)
    {
        if (MustParse_SPS_PPS)
            return true; //Wait for SPS and PPS

        //Random access check
        RandomAccess=false;

        //Computing final size
        size_t TranscodedBuffer_Size=0;
        while (Buffer_Offset+SizeOfNALU_Minus1+1+1<=Buffer_Size)
        {
            size_t Size;
            switch (SizeOfNALU_Minus1)
            {
                case 0: Size=Buffer[Buffer_Offset];
                        TranscodedBuffer_Size+=2;
                        break;
                case 1: Size=BigEndian2int16u(Buffer+Buffer_Offset);
                        TranscodedBuffer_Size++;
                        break;
                case 2: Size=BigEndian2int24u(Buffer+Buffer_Offset);
                        break;
                case 3: Size=BigEndian2int32u(Buffer+Buffer_Offset);
                        TranscodedBuffer_Size--;
                        break;
                default:    return true; //Problem
            }
            Size+=SizeOfNALU_Minus1+1;

            //Coherency checking
            if (Size==0 || Buffer_Offset+Size>Buffer_Size || (Buffer_Offset+Size!=Buffer_Size && Buffer_Offset+Size+SizeOfNALU_Minus1+1>Buffer_Size))
                Size=Buffer_Size-Buffer_Offset;

            //Random access check
            if (!RandomAccess && Buffer_Offset+SizeOfNALU_Minus1+1<Buffer_Size && (Buffer[Buffer_Offset+SizeOfNALU_Minus1+1]&0x1F) && (Buffer[Buffer_Offset+SizeOfNALU_Minus1+1]&0x1F)<=5) //Is a slice
            {
                int32u slice_type;
                Element_Offset=SizeOfNALU_Minus1+1+1;
                Element_Size=Size;
                BS_Begin();
                Skip_UE("first_mb_in_slice");
                Get_UE (slice_type, "slice_type");
                BS_End();
                Element_Offset=0;

                switch (slice_type)
                {
                    case 2 :
                    case 7 :
                                RandomAccess=true;
                }
            }

            TranscodedBuffer_Size+=Size;
            Buffer_Offset+=Size;
        }
        Buffer_Offset=0;

        //Adding SPS/PPS sizes
        if (RandomAccess)
        {
            for (seq_parameter_set_structs::iterator Data_Item=seq_parameter_sets.begin(); Data_Item!=seq_parameter_sets.end(); ++Data_Item)
                TranscodedBuffer_Size+=(*Data_Item)->Iso14496_10_Buffer_Size;
            for (seq_parameter_set_structs::iterator Data_Item=subset_seq_parameter_sets.begin(); Data_Item!=subset_seq_parameter_sets.end(); ++Data_Item)
                TranscodedBuffer_Size+=(*Data_Item)->Iso14496_10_Buffer_Size;
            for (pic_parameter_set_structs::iterator Data_Item=pic_parameter_sets.begin(); Data_Item!=pic_parameter_sets.end(); ++Data_Item)
                TranscodedBuffer_Size+=(*Data_Item)->Iso14496_10_Buffer_Size;
        }

        //Copying
        int8u* TranscodedBuffer=new int8u[TranscodedBuffer_Size+100];
        size_t TranscodedBuffer_Pos=0;
        if (RandomAccess)
        {
            for (seq_parameter_set_structs::iterator Data_Item=seq_parameter_sets.begin(); Data_Item!=seq_parameter_sets.end(); ++Data_Item)
            {
                std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, (*Data_Item)->Iso14496_10_Buffer, (*Data_Item)->Iso14496_10_Buffer_Size);
                TranscodedBuffer_Pos+=(*Data_Item)->Iso14496_10_Buffer_Size;
            }
            for (seq_parameter_set_structs::iterator Data_Item=subset_seq_parameter_sets.begin(); Data_Item!=subset_seq_parameter_sets.end(); ++Data_Item)
            {
                std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, (*Data_Item)->Iso14496_10_Buffer, (*Data_Item)->Iso14496_10_Buffer_Size);
                TranscodedBuffer_Pos+=(*Data_Item)->Iso14496_10_Buffer_Size;
            }
            for (pic_parameter_set_structs::iterator Data_Item=pic_parameter_sets.begin(); Data_Item!=pic_parameter_sets.end(); ++Data_Item)
            {
                std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, (*Data_Item)->Iso14496_10_Buffer, (*Data_Item)->Iso14496_10_Buffer_Size);
                TranscodedBuffer_Pos+=(*Data_Item)->Iso14496_10_Buffer_Size;
            }
        }
        while (Buffer_Offset<Buffer_Size)
        {
            //Sync layer
            TranscodedBuffer[TranscodedBuffer_Pos]=0x00;
            TranscodedBuffer_Pos++;
            TranscodedBuffer[TranscodedBuffer_Pos]=0x00;
            TranscodedBuffer_Pos++;
            TranscodedBuffer[TranscodedBuffer_Pos]=0x01;
            TranscodedBuffer_Pos++;

            //Block
            size_t Size;
            switch (SizeOfNALU_Minus1)
            {
                case 0: Size=Buffer[Buffer_Offset];
                        Buffer_Offset++;
                        break;
                case 1: Size=BigEndian2int16u(Buffer+Buffer_Offset);
                        Buffer_Offset+=2;
                        break;
                case 2: Size=BigEndian2int24u(Buffer+Buffer_Offset);
                        Buffer_Offset+=3;
                        break;
                case 3: Size=BigEndian2int32u(Buffer+Buffer_Offset);
                        Buffer_Offset+=4;
                        break;
                default: //Problem
                        delete [] TranscodedBuffer;
                        return false;
            }

            //Coherency checking
            if (Size==0 || Buffer_Offset+Size>Buffer_Size || (Buffer_Offset+Size!=Buffer_Size && Buffer_Offset+Size+SizeOfNALU_Minus1+1>Buffer_Size))
                Size=Buffer_Size-Buffer_Offset;

            std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, Buffer+Buffer_Offset, Size);
            TranscodedBuffer_Pos+=Size;
            Buffer_Offset+=Size;
        }
        Buffer_Offset=0;

        Buffer_Temp=Buffer;
        Buffer=TranscodedBuffer;
        Buffer_Temp_Size=Buffer_Size;
        Buffer_Size=TranscodedBuffer_Size;
        Demux_Offset=Buffer_Size;
    }
    else
    {
        bool zero_byte=Buffer[Buffer_Offset+2]==0x00;
        if (!(((Buffer[Buffer_Offset+(zero_byte?4:3)]&0x1B)==0x01 && (Buffer[Buffer_Offset+(zero_byte?5:4)]&0x80)!=0x80)
           || (Buffer[Buffer_Offset+(zero_byte?4:3)]&0x1F)==0x0C))
        {
            if (Demux_Offset==0)
            {
                Demux_Offset=Buffer_Offset;
                Demux_IntermediateItemFound=false;
            }
            while (Demux_Offset+6<=Buffer_Size)
            {
                //Synchronizing
                while(Demux_Offset+6<=Buffer_Size && (Buffer[Demux_Offset  ]!=0x00
                                                   || Buffer[Demux_Offset+1]!=0x00
                                                   || Buffer[Demux_Offset+2]!=0x01))
                {
                    Demux_Offset+=2;
                    while(Demux_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
                        Demux_Offset+=2;
                    if (Demux_Offset>=Buffer_Size || Buffer[Demux_Offset-1]==0x00)
                        Demux_Offset--;
                }

                if (Demux_Offset+6>Buffer_Size)
                {
                    if (File_Offset+Buffer_Size==File_Size)
                        Demux_Offset=Buffer_Size;
                    break;
                }

                zero_byte=Buffer[Demux_Offset+2]==0x00;
                if (Demux_IntermediateItemFound)
                {
                    if (!(((Buffer[Demux_Offset+(zero_byte?4:3)]&0x1B)==0x01 && (Buffer[Demux_Offset+(zero_byte?5:4)]&0x80)!=0x80)
                        || (Buffer[Demux_Offset+(zero_byte?4:3)]&0x1F)==0x0C))
                        break;
                }
                else
                {
                    if ((Buffer[Demux_Offset+(zero_byte?4:3)]&0x1B)==0x01 && (Buffer[Demux_Offset+(zero_byte?5:4)]&0x80)==0x80)
                        Demux_IntermediateItemFound=true;
                }

                Demux_Offset++;
            }

            if (Demux_Offset+6>Buffer_Size && !FrameIsAlwaysComplete && File_Offset+Buffer_Size<File_Size)
                return false; //No complete frame

            if (Demux_Offset && Buffer[Demux_Offset-1]==0x00)
                Demux_Offset--;

            zero_byte=Buffer[Buffer_Offset+2]==0x00;
            size_t Buffer_Offset_Random=Buffer_Offset;
            if ((Buffer[Buffer_Offset_Random+(zero_byte?4:3)]&0x1F)==0x09)
            {
                Buffer_Offset_Random++;
                if (zero_byte)
                    Buffer_Offset_Random++;
                while(Buffer_Offset_Random+6<=Buffer_Size && (Buffer[Buffer_Offset_Random  ]!=0x00
                                                           || Buffer[Buffer_Offset_Random+1]!=0x00
                                                           || Buffer[Buffer_Offset_Random+2]!=0x01))
                    Buffer_Offset_Random++;
                zero_byte=Buffer[Buffer_Offset_Random+2]==0x00;
            }
            RandomAccess=Buffer_Offset_Random+6<=Buffer_Size && (Buffer[Buffer_Offset_Random+(zero_byte?4:3)]&0x1F)==0x07; //seq_parameter_set
        }
    }

    if (!Status[IsAccepted])
    {
        if (Config->Demux_EventWasSent)
            return false;
        File_Avc* MI=new File_Avc;
        Open_Buffer_Init(MI);
        Open_Buffer_Continue(MI, Buffer, Buffer_Size);
        bool IsOk=MI->Status[IsAccepted];
        delete MI;
        if (!IsOk)
            return false;
    }

    if (IFrame_Count || RandomAccess)
    {
        bool Frame_Count_NotParsedIncluded_PlusOne=false;
        int64u PTS_Temp=FrameInfo.PTS;
        if (!IsSub)
            FrameInfo.PTS=(int64u)-1;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1 && Interlaced_Top!=Interlaced_Bottom)
        {
            Frame_Count_NotParsedIncluded--;
            Frame_Count_NotParsedIncluded_PlusOne=true;
        }
        Demux_UnpacketizeContainer_Demux(RandomAccess);
        if (!IsSub)
            FrameInfo.PTS=PTS_Temp;
        if (Frame_Count_NotParsedIncluded_PlusOne)
            Frame_Count_NotParsedIncluded++;
    }
    else
        Demux_UnpacketizeContainer_Demux_Clear();

    if (Buffer_Temp)
    {
        Demux_TotalBytes-=Buffer_Size;
        Demux_TotalBytes+=Buffer_Temp_Size;
        delete[] Buffer;
        Buffer=Buffer_Temp;
        Buffer_Size=Buffer_Temp_Size;
    }

    return true;
}
#endif //MEDIAINFO_DEMUX

//---------------------------------------------------------------------------
void File_Avc::Synched_Init()
{
    //FrameInfo
    PTS_End=0;
    if (FrameInfo.DTS==(int64u)-1)
        FrameInfo.DTS=0; //No DTS in container
    DTS_Begin=FrameInfo.DTS;
    DTS_End=FrameInfo.DTS;

    //Temporal references
    TemporalReferences_DelayedElement=NULL;
    TemporalReferences_Min=0;
    TemporalReferences_Max=0;
    TemporalReferences_Reserved=0;
    TemporalReferences_Offset=0;
    TemporalReferences_Offset_pic_order_cnt_lsb_Last=0;
    TemporalReferences_pic_order_cnt_Min=0;

    //Text
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        GA94_03_IsPresent=false;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)

    //File specific
    SizeOfNALU_Minus1=(int8u)-1;

    //Status
    IFrame_Count=0;
    prevPicOrderCntMsb=0;
    prevPicOrderCntLsb=(int32u)-1;
    prevTopFieldOrderCnt=(int32u)-1;
    prevFrameNum=(int32u)-1;
    prevFrameNumOffset=(int32u)-1;

    //Count of a Packets
    Block_Count=0;
    Interlaced_Top=0;
    Interlaced_Bottom=0;
    Structure_Field=0;
    Structure_Frame=0;

    //Temp
    FrameRate_Divider=1;
    FirstPFrameInGop_IsParsed=false;
    tc=0;

    //Default values
    Streams.resize(0x100);
    Streams[0x06].Searching_Payload=true; //sei
    Streams[0x07].Searching_Payload=true; //seq_parameter_set
    Streams[0x09].Searching_Payload=true; //access_unit_delimiter
    Streams[0x0F].Searching_Payload=true; //subset_seq_parameter_set
    for (int8u Pos=0xFF; Pos>=0xB9; Pos--)
        Streams[Pos].Searching_Payload=true; //Testing MPEG-PS

    //Options
    Option_Manage();

    //Specific cases
    #if MEDIAINFO_EVENTS
        if (Config->ParseUndecodableFrames_Get())
        {
            Accept(); //In some case, we must accept the stream very quickly and before the sequence header is detected
            Streams[0x01].Searching_Payload=true; //slice_header
            Streams[0x05].Searching_Payload=true; //slice_header
        }
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10=Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get();
    #endif //MEDIAINFO_DEMUX
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED2
void File_Avc::Read_Buffer_SegmentChange()
{
}
#endif //MEDIAINFO_ADVANCED2

//---------------------------------------------------------------------------
void File_Avc::Read_Buffer_Unsynched()
{
    //Temporal references
    for (size_t Pos=0; Pos<TemporalReferences.size(); Pos++)
        delete TemporalReferences[Pos]; //TemporalReferences[Pos]=NULL;
    TemporalReferences.clear();
    delete TemporalReferences_DelayedElement; TemporalReferences_DelayedElement=NULL;
    TemporalReferences_Min=0;
    TemporalReferences_Max=0;
    TemporalReferences_Reserved=0;
    TemporalReferences_Offset=0;
    TemporalReferences_Offset_pic_order_cnt_lsb_Last=0;
    TemporalReferences_pic_order_cnt_Min=0;

    //Text
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        if (GA94_03_Parser)
            GA94_03_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)

    //parameter_sets
    if (SizedBlocks) //If sized blocks, it is not a broadcasted stream so SPS/PPS are only in container header, we must not disable them.
    {
        //Rebuilding immediatly TemporalReferences
        for (std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item=seq_parameter_sets.begin(); seq_parameter_set_Item!=seq_parameter_sets.end(); ++seq_parameter_set_Item)
            if ((*seq_parameter_set_Item))
            {
                size_t MaxNumber;
                switch ((*seq_parameter_set_Item)->pic_order_cnt_type)
                {
                    case 0 : MaxNumber=(*seq_parameter_set_Item)->MaxPicOrderCntLsb; break;
                    case 2 : MaxNumber=(*seq_parameter_set_Item)->MaxFrameNum*2; break;
                    default: Trusted_IsNot("Not supported"); return;
                }

                TemporalReferences.resize(4*MaxNumber);
                TemporalReferences_Reserved=MaxNumber;
            }
    }
    else
    {
        seq_parameter_sets.clear();
        subset_seq_parameter_sets.clear();
        pic_parameter_sets.clear();
    }

    //Status
    Interlaced_Top=0;
    Interlaced_Bottom=0;
    prevPicOrderCntMsb=0;
    prevPicOrderCntLsb=(int32u)-1;
    prevTopFieldOrderCnt=(int32u)-1;
    prevFrameNum=(int32u)-1;
    prevFrameNumOffset=(int32u)-1;

    //Temp
    FrameRate_Divider=1;
    FirstPFrameInGop_IsParsed=false;
    tc=0;

    //Impossible to know TimeStamps now
    PTS_End=0;
    DTS_End=0;
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
    int8u nal_unit_type;
    if (!SizedBlocks)
    {
        if (Buffer[Buffer_Offset+2]==0x00)
            Skip_B1(                                            "zero_byte");
        Skip_B3(                                                "start_code_prefix_one_3bytes");
        BS_Begin();
        Mark_0 ();
        Get_S1 ( 2, nal_ref_idc,                                "nal_ref_idc");
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
        Get_S1 ( 2, nal_ref_idc,                                "nal_ref_idc");
        Get_S1 ( 5, nal_unit_type,                              "nal_unit_type");
        BS_End();

        FILLING_BEGIN();
            Header_Fill_Size(Size?(Element_Offset-1+Size):(Buffer_Size-Buffer_Offset)); //If Size is 0, it is not normal, we skip the complete frame
        FILLING_END();
    }

    //Filling
    #if MEDIAINFO_TRACE
        if (Trace_Activated)
            Header_Fill_Code(nal_unit_type, Ztring().From_CC1(nal_unit_type));
        else
    #endif //MEDIAINFO_TRACE
            Header_Fill_Code(nal_unit_type);
}

//---------------------------------------------------------------------------
bool File_Avc::Header_Parser_Fill_Size()
{
    //Look for next Sync word
    if (Buffer_Offset_Temp==0) //Buffer_Offset_Temp is not 0 if Header_Parse_Fill_Size() has already parsed first frames
        Buffer_Offset_Temp=Buffer_Offset+4;
    while (Buffer_Offset_Temp+5<=Buffer_Size
        && CC3(Buffer+Buffer_Offset_Temp)!=0x000001)
    {
        Buffer_Offset_Temp+=2;
        while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]!=0x00)
            Buffer_Offset_Temp+=2;
        if (Buffer_Offset_Temp>=Buffer_Size || Buffer[Buffer_Offset_Temp-1]==0x00)
            Buffer_Offset_Temp--;
    }

    //Must wait more data?
    if (Buffer_Offset_Temp+5>Buffer_Size)
    {
        if (FrameIsAlwaysComplete || File_Offset+Buffer_Size>=File_Size)
            Buffer_Offset_Temp=Buffer_Size; //We are sure that the next bytes are a start
        else
            return false;
    }

    if (Buffer[Buffer_Offset_Temp-1]==0x00)
        Buffer_Offset_Temp--;

    //OK, we continue
    Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
    Buffer_Offset_Temp=0;
    return true;
}

//---------------------------------------------------------------------------
bool File_Avc::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+6<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x00
      &&   Buffer[Buffer_Offset+1]==0x00
      &&  (Buffer[Buffer_Offset+2]==0x01
        || (Buffer[Buffer_Offset+2]==0x00 && Buffer[Buffer_Offset+3]==0x01)))
    {
        //Getting start_code
        int8u start_code;
        if (Buffer[Buffer_Offset+2]==0x00)
            start_code=CC1(Buffer+Buffer_Offset+4)&0x1F;
        else
            start_code=CC1(Buffer+Buffer_Offset+3)&0x1F;

        //Searching start
        if (Streams[start_code].Searching_Payload
         || Streams[start_code].ShouldDuplicate)
            return true;

        //Synchronizing
        Buffer_Offset+=4;
        Synched=false;
        if (!Synchronize())
        {
            UnSynched_IsNotJunk=true;
            return false;
        }

        if (Buffer_Offset+6>Buffer_Size)
        {
            UnSynched_IsNotJunk=true;
            return false;
        }
    }

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

    //Trailing zeroes
    int64u Element_Size_SaveBeforeZeroes=Element_Size;
    if (Element_Size)
    {
        while (Element_Size && Buffer[Buffer_Offset+(size_t)Element_Size-1]==0)
            Element_Size--;
    }

    //svc_extension
    bool svc_extension_flag=false;
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
        if (Element_Offset_3Bytes>=Element_Size || Buffer[Buffer_Offset+(size_t)Element_Offset_3Bytes-1]==0x00)
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
        case 0x0E : prefix_nal_unit(svc_extension_flag); break;
        case 0x0F : subset_seq_parameter_set(); break;
        case 0x13 : Element_Name("slice_layer_without_partitioning"); Skip_XX(Element_Size-Element_Offset, "Data"); break;
        case 0x14 : slice_layer_extension(svc_extension_flag); break;
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
    #if MEDIAINFO_DUPLICATE
        if (!Streams.empty() && Streams[(size_t)Element_Code].ShouldDuplicate)
            File__Duplicate_Write(Element_Code);
    #endif //MEDIAINFO_DUPLICATE

    #if MEDIAINFO_DEMUX
        if (Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10)
        {
            if (Element_Code==0x07)
            {
                std::vector<seq_parameter_set_struct*>::iterator Data_Item=seq_parameter_sets.begin();
                if (Data_Item!=seq_parameter_sets.end() && (*Data_Item))
                {
                    delete[] (*Data_Item)->Iso14496_10_Buffer;
                    (*Data_Item)->Iso14496_10_Buffer_Size=(size_t)(Element_Size+4);
                    (*Data_Item)->Iso14496_10_Buffer=new int8u[(*Data_Item)->Iso14496_10_Buffer_Size];
                    (*Data_Item)->Iso14496_10_Buffer[0]=0x00;
                    (*Data_Item)->Iso14496_10_Buffer[1]=0x00;
                    (*Data_Item)->Iso14496_10_Buffer[2]=0x01;
                    (*Data_Item)->Iso14496_10_Buffer[3]=0x67;
                    std::memcpy((*Data_Item)->Iso14496_10_Buffer+4, Buffer+Buffer_Offset, (size_t)Element_Size);
                }
            }
            if (Element_Code==0x08)
            {
                std::vector<pic_parameter_set_struct*>::iterator Data_Item=pic_parameter_sets.begin();
                if (Data_Item!=pic_parameter_sets.end() && (*Data_Item))
                {
                    delete[] (*Data_Item)->Iso14496_10_Buffer;
                    (*Data_Item)->Iso14496_10_Buffer_Size=(size_t)(Element_Size+4);
                    (*Data_Item)->Iso14496_10_Buffer=new int8u[(*Data_Item)->Iso14496_10_Buffer_Size];
                    (*Data_Item)->Iso14496_10_Buffer[0]=0x00;
                    (*Data_Item)->Iso14496_10_Buffer[1]=0x00;
                    (*Data_Item)->Iso14496_10_Buffer[2]=0x01;
                    (*Data_Item)->Iso14496_10_Buffer[3]=0x68;
                    std::memcpy((*Data_Item)->Iso14496_10_Buffer+4, Buffer+Buffer_Offset, (size_t)Element_Size);
                }
            }
            if (Element_Code==0x0F)
            {
                std::vector<seq_parameter_set_struct*>::iterator Data_Item=subset_seq_parameter_sets.begin();
                if (Data_Item!=subset_seq_parameter_sets.end() && (*Data_Item))
                {
                    SizeOfNALU_Minus1=0;
                    delete[] (*Data_Item)->Iso14496_10_Buffer;
                    (*Data_Item)->Iso14496_10_Buffer_Size=(size_t)(Element_Size+4);
                    (*Data_Item)->Iso14496_10_Buffer=new int8u[(*Data_Item)->Iso14496_10_Buffer_Size];
                    (*Data_Item)->Iso14496_10_Buffer[0]=0x00;
                    (*Data_Item)->Iso14496_10_Buffer[1]=0x00;
                    (*Data_Item)->Iso14496_10_Buffer[2]=0x01;
                    (*Data_Item)->Iso14496_10_Buffer[3]=0x6F;
                    std::memcpy((*Data_Item)->Iso14496_10_Buffer+4, Buffer+Buffer_Offset, (size_t)Element_Size);
                }
            }
        }
    #endif //MEDIAINFO_DEMUX

    //Trailing zeroes
    Element_Size=Element_Size_SaveBeforeZeroes;
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
    BS_Begin();
    slice_header();
    slice_data(true);
    BS_End();
}

//---------------------------------------------------------------------------
// Packet "05"
void File_Avc::slice_layer_without_partitioning_IDR()
{
    Element_Name("slice_layer_without_partitioning (IDR)");

    //Parsing
    BS_Begin();
    slice_header();
    slice_data(true);
    BS_End();

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
    //Encryption management
    if (CA_system_ID_MustSkipSlices)
    {
        //Is not decodable
        Skip_BS(Data_BS_Remain(),                               "Data");
        Finish("AVC");
        return;
    }

    Element_Begin1("slice_header");

    //Parsing
    int32u  slice_type, pic_order_cnt_lsb=(int32u)-1;
    int32u  first_mb_in_slice, pic_parameter_set_id, frame_num, num_ref_idx_l0_active_minus1, num_ref_idx_l1_active_minus1, disable_deblocking_filter_idc, num_slice_groups_minus1, slice_group_map_type;
    int32s  delta_pic_order_cnt_bottom=0;
    bool    field_pic_flag=false, bottom_field_flag=false;
    Get_UE (first_mb_in_slice,                                  "first_mb_in_slice");
    Get_UE (slice_type,                                         "slice_type"); Param_Info1C((slice_type<10), Avc_slice_type[slice_type]);
    #if MEDIAINFO_EVENTS
        {
            EVENT_BEGIN (Video, SliceInfo, 0)
                Event.FieldPosition=Field_Count;
                Event.SlicePosition=Element_IsOK()?first_mb_in_slice:(int64u)-1;
                switch (slice_type)
                {
                    case 0 :
                    case 3 :
                    case 5 :
                    case 8 :
                                Event.SliceType=1; break;
                    case 1 :
                    case 6 :
                                Event.SliceType=2; break;
                    case 2 :
                    case 4 :
                    case 7 :
                    case 9 :
                                Event.SliceType=0; break;
                    default:
                                Event.SliceType=(int8u)-1;
                }
                Event.Flags=0;
            EVENT_END   ()
        }
    #endif //MEDIAINFO_EVENTS
    if (slice_type>=10)
    {
        Skip_BS(Data_BS_Remain(),                               "Data");
        Element_End0();
        return;
    }
    Get_UE (pic_parameter_set_id,                               "pic_parameter_set_id");
    std::vector<pic_parameter_set_struct*>::iterator pic_parameter_set_Item;
    if (pic_parameter_set_id>=pic_parameter_sets.size() || (*(pic_parameter_set_Item=pic_parameter_sets.begin()+pic_parameter_set_id))==NULL)
    {
        //Not yet present
        Skip_BS(Data_BS_Remain(),                               "Data (pic_parameter_set is missing)");
        Element_End0();
        return;
    }
    std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item;
    if ((*pic_parameter_set_Item)->seq_parameter_set_id>=seq_parameter_sets.size() || (*(seq_parameter_set_Item=seq_parameter_sets.begin()+(*pic_parameter_set_Item)->seq_parameter_set_id))==NULL)
    {
        if ((*pic_parameter_set_Item)->seq_parameter_set_id>=subset_seq_parameter_sets.size() || (*(seq_parameter_set_Item=subset_seq_parameter_sets.begin()+(*pic_parameter_set_Item)->seq_parameter_set_id))==NULL)
        {
            //Not yet present
            Skip_BS(Data_BS_Remain(),                           "Data (seq_parameter_set is missing)");
            Element_End0();
            return;
        }
    }
    if ((*seq_parameter_set_Item)->separate_colour_plane_flag==1)
        Skip_S1(2,                                              "color_plane_id");
    num_ref_idx_l0_active_minus1=(*pic_parameter_set_Item)->num_ref_idx_l0_default_active_minus1; //Default
    num_ref_idx_l1_active_minus1=(*pic_parameter_set_Item)->num_ref_idx_l1_default_active_minus1; //Default
    Get_BS ((*seq_parameter_set_Item)->log2_max_frame_num_minus4+4, frame_num, "frame_num");
    if (!(*seq_parameter_set_Item)->frame_mbs_only_flag)
    {
        TEST_SB_GET(field_pic_flag,                             "field_pic_flag");
            Get_SB (bottom_field_flag,                          "bottom_field_flag");
        TEST_SB_END();
    }
    if (Element_Code==5) //IdrPicFlag
        Skip_UE(                                                "idr_pic_id");
    if ((*seq_parameter_set_Item)->pic_order_cnt_type==0)
    {
        Get_BS ((*seq_parameter_set_Item)->log2_max_pic_order_cnt_lsb_minus4+4, pic_order_cnt_lsb, "pic_order_cnt_lsb");
        if ((*pic_parameter_set_Item)->bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
            Get_SE (delta_pic_order_cnt_bottom,                 "delta_pic_order_cnt_bottom");
    }
    if ((*seq_parameter_set_Item)->pic_order_cnt_type==1 && !(*seq_parameter_set_Item)->delta_pic_order_always_zero_flag )
    {
        Skip_SE(                                                "delta_pic_order_cnt[0]");
        if((*pic_parameter_set_Item)->bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
            Skip_SE(                                            "delta_pic_order_cnt[1]");
    }
    if((*pic_parameter_set_Item)->redundant_pic_cnt_present_flag)
        Skip_UE(                                                "redundant_pic_cnt");
    if (slice_type==1 || slice_type==6) //B-Frame
        Skip_SB(                                                "direct_spatial_mv_pred_flag");
    switch (slice_type)
    {
        case 0 : //P-Frame
        case 1 : //B-Frame
        case 3 : //SP-Frame
        case 5 : //P-Frame
        case 6 : //B-Frame
        case 8 : //SP-Frame
                    TEST_SB_SKIP(                               "num_ref_idx_active_override_flag");
                        Get_UE (num_ref_idx_l0_active_minus1,   "num_ref_idx_l0_active_minus1");
                        switch (slice_type)
                        {
                            case 1 : //B-Frame
                            case 6 : //B-Frame
                                        Get_UE (num_ref_idx_l1_active_minus1, "num_ref_idx_l1_active_minus1");
                                        break;
                            default:    ;
                        }
                    TEST_SB_END();
                    break;
        default:    ;
    }
    ref_pic_list_modification(slice_type, Element_Code==20); //nal_unit_type==20 --> ref_pic_list_mvc_modification()
    if (((*pic_parameter_set_Item)->weighted_pred_flag && (slice_type==0 || slice_type==3 || slice_type==5 || slice_type==8))
     || ((*pic_parameter_set_Item)->weighted_bipred_idc==1 && (slice_type==1 || slice_type==6)))
        pred_weight_table(num_ref_idx_l0_active_minus1, num_ref_idx_l1_active_minus1, (*seq_parameter_set_Item)->ChromaArrayType());
    std::vector<int8u> memory_management_control_operations;
    if (nal_ref_idc)
        dec_ref_pic_marking(memory_management_control_operations);

    if ((*pic_parameter_set_Item)->entropy_coding_mode_flag &&
        (slice_type!=2 && slice_type!=7 && //I-Frames
         slice_type!=4 && slice_type!=9))  //SI-Frames
        Skip_UE(                                               "cabac_init_idc");
    Skip_SE(                                                   "slice_qp_delta");
    switch (slice_type)
    {
        case 3 : //SP-Frame
        case 4 : //SI-Frame
        case 8 : //SP-Frame
        case 9 : //SI-Frame
                switch (slice_type)
                {
                    case 3 : //SP-Frame
                    case 8 : //SP-Frame
                            Skip_SB(                           "sp_for_switch_flag");
                            break;
                    default:    ;
                }
                Skip_SE (                                      "slice_qs_delta");
                break;
        default:    ;
    }
    if ((*pic_parameter_set_Item)->deblocking_filter_control_present_flag)
    {
        Get_UE(disable_deblocking_filter_idc,                  "disable_deblocking_filter_idc");
        if (disable_deblocking_filter_idc!=1)
        {
            Skip_SE(                                           "slice_alpha_c0_offset_div2");
            Skip_SE(                                           "slice_beta_offset_div2");
        }
    }
    num_slice_groups_minus1=(*pic_parameter_set_Item)->num_slice_groups_minus1; //Default
    slice_group_map_type=(*pic_parameter_set_Item)->slice_group_map_type; //Default
    //if (num_slice_groups_minus1 > 0 && slice_group_map_type >=3 && slice_group_map_type <=5)
    //    Get_BS ((*seq_parameter_set_Item)->log2_max_slice_group_change_cycle_minus4+4, slice_group_change_cycle, "slice_group_change_cycle");

    Element_End0();

    FILLING_BEGIN();
        //Count of I-Frames
        if (first_mb_in_slice==0 && Element_Code!=20 && (slice_type==2 || slice_type==7)) //Not slice_layer_extension, I-Frame
            IFrame_Count++;

        //pic_struct
        if (field_pic_flag && (*seq_parameter_set_Item)->pic_struct_FirstDetected==(int8u)-1)
            (*seq_parameter_set_Item)->pic_struct_FirstDetected=bottom_field_flag?2:1; //2=BFF, 1=TFF

        //Saving some info
        int32s TemporalReferences_Offset_pic_order_cnt_lsb_Diff=0;
        if ((*seq_parameter_set_Item)->pic_order_cnt_type!=1 && first_mb_in_slice==0 && (Element_Code!=0x14 || seq_parameter_sets.empty())) //Not slice_layer_extension except if MVC only
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

            //Frame order detection
            int64s pic_order_cnt=0;
            switch ((*seq_parameter_set_Item)->pic_order_cnt_type)
            {
                case 0 :
                            {
                            if (Element_Code==5) //IDR
                            {
                                prevPicOrderCntMsb=0;
                                prevPicOrderCntLsb=0;
                                TemporalReferences_Offset=TemporalReferences_Max;
                                if (TemporalReferences_Offset%2)
                                    TemporalReferences_Offset++;
                                TemporalReferences_pic_order_cnt_Min=0;
                            }
                            else
                            {
                                bool Has5=false;
                                for (std::vector<int8u>::iterator Temp=memory_management_control_operations.begin(); Temp!=memory_management_control_operations.end(); ++Temp)
                                    if ((*Temp)==5)
                                    {
                                        Has5=true;
                                        break;
                                    }
                                if (Has5)
                                {
                                    prevPicOrderCntMsb=0;
                                    if (bottom_field_flag)
                                        prevPicOrderCntLsb=0;
                                    else
                                        prevPicOrderCntLsb=prevTopFieldOrderCnt;
                                }
                            }
                            int32s PicOrderCntMsb;
                            if (prevPicOrderCntLsb==(int32u)-1)
                            {
                                PicOrderCntMsb=0;
                                if ((int32u)(2*((*seq_parameter_set_Item)->max_num_ref_frames+3))<pic_order_cnt_lsb)
                                    TemporalReferences_Min=pic_order_cnt_lsb-2*((*seq_parameter_set_Item)->max_num_ref_frames+3);
                            }
                            else if (pic_order_cnt_lsb<prevPicOrderCntLsb && prevPicOrderCntLsb-pic_order_cnt_lsb>=(*seq_parameter_set_Item)->MaxPicOrderCntLsb/2)
                                PicOrderCntMsb=prevPicOrderCntMsb+(*seq_parameter_set_Item)->MaxPicOrderCntLsb;
                            else if (pic_order_cnt_lsb>prevPicOrderCntLsb && pic_order_cnt_lsb-prevPicOrderCntLsb>(*seq_parameter_set_Item)->MaxPicOrderCntLsb/2)
                                PicOrderCntMsb=prevPicOrderCntMsb-(*seq_parameter_set_Item)->MaxPicOrderCntLsb;
                            else
                                PicOrderCntMsb=prevPicOrderCntMsb;

                            int32s TopFieldOrderCnt=PicOrderCntMsb+pic_order_cnt_lsb;
                            int32s BottomFieldOrderCnt;
                            if (field_pic_flag)
                                BottomFieldOrderCnt=TopFieldOrderCnt+delta_pic_order_cnt_bottom;
                            else
                                BottomFieldOrderCnt=PicOrderCntMsb+pic_order_cnt_lsb;

                            prevPicOrderCntMsb=PicOrderCntMsb;
                            prevPicOrderCntLsb=pic_order_cnt_lsb;
                            prevTopFieldOrderCnt=TopFieldOrderCnt;

                            pic_order_cnt=bottom_field_flag?BottomFieldOrderCnt:TopFieldOrderCnt;
                            }
                            break;
                case 2 :
                            {
                            bool Has5=false;
                            for (std::vector<int8u>::iterator Temp=memory_management_control_operations.begin(); Temp!=memory_management_control_operations.end(); ++Temp)
                                if ((*Temp)==5)
                                {
                                    Has5=true;
                                    break;
                                }
                            if (Has5)
                                prevFrameNumOffset=0;
                            int32u FrameNumOffset;

                            if (Element_Code==5) //IdrPicFlag
                            {
                                TemporalReferences_Offset=TemporalReferences_Max;
                                if (TemporalReferences_Offset%2)
                                    TemporalReferences_Offset++;
                                FrameNumOffset=0;
                            }
                            else if (prevFrameNumOffset==(int32u)-1)
                                FrameNumOffset=0;
                            else if (prevFrameNum>frame_num)
                                FrameNumOffset=prevFrameNumOffset+(*seq_parameter_set_Item)->MaxFrameNum;
                            else
                                FrameNumOffset=prevFrameNumOffset;

                            int32u tempPicOrderCnt;
                            if (Element_Code==5) //IdrPicFlag
                                tempPicOrderCnt=0;
                            else if (nal_ref_idc)
                                tempPicOrderCnt=2*(FrameNumOffset+frame_num);
                            else
                                tempPicOrderCnt=2*(FrameNumOffset+frame_num)-1;

                            pic_order_cnt=tempPicOrderCnt;

                            prevFrameNum=frame_num;
                            prevFrameNumOffset=FrameNumOffset;

                            pic_order_cnt_lsb=frame_num;
                            }
                            break;
                default:    ;
            }

            if (pic_order_cnt<TemporalReferences_pic_order_cnt_Min)
            {
                if (pic_order_cnt<0)
                {
                    size_t Base=(size_t)(TemporalReferences_Offset+TemporalReferences_pic_order_cnt_Min);
                    size_t ToInsert=(size_t)(TemporalReferences_pic_order_cnt_Min-pic_order_cnt);
                    if (Base+ToInsert>=4*TemporalReferences_Reserved || Base>=4*TemporalReferences_Reserved || ToInsert+TemporalReferences_Max>=4*TemporalReferences_Reserved || TemporalReferences_Max>=4*TemporalReferences_Reserved || TemporalReferences_Max-Base>=4*TemporalReferences_Reserved)
                    {
                        Trusted_IsNot("Problem in temporal references");
                        return;
                    }
                    Element_Info1(__T("Offset of ")+Ztring::ToZtring(ToInsert));
                    TemporalReferences.insert(TemporalReferences.begin()+Base, ToInsert, NULL);
                    TemporalReferences_Offset+=ToInsert;
                    TemporalReferences_Max+=ToInsert;
                    TemporalReferences_pic_order_cnt_Min=pic_order_cnt;
                }
                else if (TemporalReferences_Min>(size_t)(TemporalReferences_Offset+pic_order_cnt))
                    TemporalReferences_Min=(size_t)(TemporalReferences_Offset+pic_order_cnt);
            }

            if (pic_order_cnt<0 && TemporalReferences_Offset<(size_t)(-pic_order_cnt)) //Found in playreadyEncryptedBlowUp.ts without encryption test
            {
                Trusted_IsNot("Problem in temporal references");
                return;
            }

            if ((size_t)(TemporalReferences_Offset+pic_order_cnt)>=3*TemporalReferences_Reserved)
            {
                size_t Offset=TemporalReferences_Max-TemporalReferences_Offset;
                if (Offset%2)
                    Offset++;
                if (Offset>=TemporalReferences_Reserved && pic_order_cnt>=(int64s)TemporalReferences_Reserved)
                {
                    TemporalReferences_Offset+=TemporalReferences_Reserved;
                    pic_order_cnt-=TemporalReferences_Reserved;
                    switch ((*seq_parameter_set_Item)->pic_order_cnt_type)
                    {
                        case 0 :
                                prevPicOrderCntMsb-=(int32u)TemporalReferences_Reserved;
                                break;
                        case 2 :
                                prevFrameNumOffset-=(int32u)TemporalReferences_Reserved/2;
                                break;
                        default:;
                    }
                }
                else if (Offset && Offset<=2) //Only I-frames
                    TemporalReferences_Offset+=2;
                while (TemporalReferences_Offset+pic_order_cnt>=3*TemporalReferences_Reserved)
                {
                    for (size_t Pos=0; Pos<TemporalReferences_Reserved; Pos++)
                    {
                        if (TemporalReferences[Pos])
                        {
                            if ((Pos%2)==0)
                                PictureTypes_PreviousFrames+=Avc_slice_type[TemporalReferences[Pos]->slice_type];
                        }
                        else if (!PictureTypes_PreviousFrames.empty()) //Only if stream already started
                        {
                            if ((Pos%2)==0)
                                PictureTypes_PreviousFrames+=' ';
                        }
                        delete TemporalReferences[Pos];
                    }
                    if (PictureTypes_PreviousFrames.size()>=8*TemporalReferences.size())
                        PictureTypes_PreviousFrames.erase(PictureTypes_PreviousFrames.begin(), PictureTypes_PreviousFrames.begin()+PictureTypes_PreviousFrames.size()-TemporalReferences.size());
                    TemporalReferences.erase(TemporalReferences.begin(), TemporalReferences.begin()+TemporalReferences_Reserved);
                    TemporalReferences.resize(4*TemporalReferences_Reserved);
                    if (TemporalReferences_Reserved<TemporalReferences_Offset)
                        TemporalReferences_Offset-=TemporalReferences_Reserved;
                    else
                        TemporalReferences_Offset=0;
                    if (TemporalReferences_Reserved<TemporalReferences_Min)
                        TemporalReferences_Min-=TemporalReferences_Reserved;
                    else
                        TemporalReferences_Min=0;
                    if (TemporalReferences_Reserved<TemporalReferences_Max)
                        TemporalReferences_Max-=TemporalReferences_Reserved;
                    else
                        TemporalReferences_Max=0;
                }
            }

            TemporalReferences_Offset_pic_order_cnt_lsb_Diff=(int32s)((int32s)(TemporalReferences_Offset+pic_order_cnt)-TemporalReferences_Offset_pic_order_cnt_lsb_Last);
            TemporalReferences_Offset_pic_order_cnt_lsb_Last=(size_t)(TemporalReferences_Offset+pic_order_cnt);
            if (TemporalReferences_Max<=TemporalReferences_Offset_pic_order_cnt_lsb_Last)
                TemporalReferences_Max=TemporalReferences_Offset_pic_order_cnt_lsb_Last+((*seq_parameter_set_Item)->frame_mbs_only_flag?2:1);
            if (TemporalReferences_Min>TemporalReferences_Offset_pic_order_cnt_lsb_Last)
                TemporalReferences_Min=TemporalReferences_Offset_pic_order_cnt_lsb_Last;
            if (TemporalReferences_DelayedElement)
            {
                delete TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]; TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]=TemporalReferences_DelayedElement;
            }
            if (TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]==NULL)
                TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]=new temporal_reference();
            TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]->frame_num=frame_num;
            TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]->slice_type=(int8u)slice_type;
            TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]->IsTop=!bottom_field_flag;
            TemporalReferences[TemporalReferences_Offset_pic_order_cnt_lsb_Last]->IsField=field_pic_flag;
            if (TemporalReferences_DelayedElement)
            {
                TemporalReferences_DelayedElement=NULL;
                sei_message_user_data_registered_itu_t_t35_GA94_03_Delayed((*pic_parameter_set_Item)->seq_parameter_set_id);
            }
        }

        if ((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->timing_info_present_flag && (*seq_parameter_set_Item)->vui_parameters->num_units_in_tick)
            tc=float64_int64s(((float64)1000000000)/((float64)(*seq_parameter_set_Item)->vui_parameters->time_scale/(*seq_parameter_set_Item)->vui_parameters->num_units_in_tick/((*seq_parameter_set_Item)->pic_order_cnt_type==2?1:2)/FrameRate_Divider)/((!(*seq_parameter_set_Item)->frame_mbs_only_flag && field_pic_flag)?2:1));
        if (first_mb_in_slice==0)
        {
            if (Frame_Count==0)
            {
                if (FrameInfo.PTS==(int64u)-1)
                    FrameInfo.PTS=FrameInfo.DTS+tc*(TemporalReferences_Offset_pic_order_cnt_lsb_Diff?2:1)*((!(*seq_parameter_set_Item)->frame_mbs_only_flag && field_pic_flag)?2:1); //No PTS in container
                PTS_Begin=FrameInfo.PTS;
            }
            #if MEDIAINFO_ADVANCED2
                if (PTS_Begin_Segment==(int64u)-1 && File_Offset>=Config->File_Current_Offset)
                {
                    PTS_Begin_Segment=FrameInfo.PTS;
                }
            #endif //MEDIAINFO_ADVANCED2
            if (slice_type==2 || slice_type==7) //IFrame
                FirstPFrameInGop_IsParsed=false;
        }
        else
        {
            if (FrameInfo.PTS!=(int64u)-1)
                FrameInfo.PTS-=tc;
            if (FrameInfo.DTS!=(int64u)-1)
                FrameInfo.DTS-=tc;
        }

        //Frame pos
        if (Frame_Count!=(int64u)-1 && Frame_Count && ((!(*seq_parameter_set_Item)->frame_mbs_only_flag && Interlaced_Top==Interlaced_Bottom && field_pic_flag) || first_mb_in_slice!=0 || (Element_Code==0x14 && !seq_parameter_sets.empty())))
        {
            Frame_Count--;
            if (IFrame_Count && Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded--;
            Frame_Count_InThisBlock--;
        }
        else if (first_mb_in_slice==0)
        {
            if ((*seq_parameter_set_Item)->pic_order_cnt_type!=1 && (Element_Code!=0x14 || seq_parameter_sets.empty())) //Not slice_layer_extension except if MVC only
            {
                if ((!IsSub || Frame_Count_InThisBlock) && TemporalReferences_Offset_pic_order_cnt_lsb_Diff && TemporalReferences_Offset_pic_order_cnt_lsb_Diff!=2)
                    FrameInfo.PTS+=(TemporalReferences_Offset_pic_order_cnt_lsb_Diff-(field_pic_flag?1:2))/((!(*seq_parameter_set_Item)->frame_mbs_only_flag && field_pic_flag)?1:2)*(int64s)tc;
            }

            if (!FirstPFrameInGop_IsParsed && (slice_type==0 || slice_type==5)) //P-Frame
            {
                FirstPFrameInGop_IsParsed=true;

                //Testing if we have enough to test GOP
                if (Frame_Count<=Frame_Count_Valid)
                {
                    std::string PictureTypes(PictureTypes_PreviousFrames);
                    PictureTypes.reserve(TemporalReferences.size());
                    for (size_t Pos=0; Pos<TemporalReferences.size(); Pos++)
                        if (TemporalReferences[Pos])
                        {
                            if ((Pos%2)==0)
                                PictureTypes+=Avc_slice_type[TemporalReferences[Pos]->slice_type];
                        }
                        else if (!PictureTypes.empty()) //Only if stream already started
                        {
                            if ((Pos%2)==0)
                                PictureTypes+=' ';
                        }
                    if (!GOP_Detect(PictureTypes).empty() && !GA94_03_IsPresent)
                        Frame_Count_Valid=Frame_Count; //We have enough frames
                }
            }
        }

        #if MEDIAINFO_TRACE
            if (Trace_Activated)
            {
                Element_Info1(TemporalReferences_Offset_pic_order_cnt_lsb_Last);
                Element_Info1((((*seq_parameter_set_Item)->frame_mbs_only_flag || !field_pic_flag)?__T("Frame "):(bottom_field_flag?__T("Field (Bottom) "):__T("Field (Top) ")))+Ztring::ToZtring(Frame_Count));
                if (slice_type<9)
                    Element_Info1(__T("slice_type ")+Ztring().From_Local(Avc_slice_type[slice_type]));
                Element_Info1(__T("frame_num ")+Ztring::ToZtring(frame_num));
                if ((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->fixed_frame_rate_flag)
                {
                    if (FrameInfo.PCR!=(int64u)-1)
                        Element_Info1(__T("PCR ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PCR)/1000000)));
                    if (FrameInfo.DTS!=(int64u)-1)
                        Element_Info1(__T("DTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.DTS)/1000000)));
                    if (FrameInfo.PTS!=(int64u)-1)
                        Element_Info1(__T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000)));
                }
                if ((*seq_parameter_set_Item)->pic_order_cnt_type==0)
                    Element_Info1(__T("pic_order_cnt_lsb ")+Ztring::ToZtring(pic_order_cnt_lsb));
                if (first_mb_in_slice)
                    Element_Info1(__T("first_mb_in_slice ")+Ztring::ToZtring(first_mb_in_slice));
            }
        #endif //MEDIAINFO_TRACE

        //Counting
        if (Frame_Count!=(int64u)-1)
        {
            if (File_Offset+Buffer_Offset+Element_Size==File_Size)
                Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
            Frame_Count++;
            if (IFrame_Count && Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded++;
            Frame_Count_InThisBlock++;
        }
        if ((*seq_parameter_set_Item)->pic_order_cnt_type==0 && field_pic_flag)
        {
            Field_Count++;
            Field_Count_InThisBlock++;
        }
        if (FrameInfo.PTS!=(int64u)-1)
            FrameInfo.PTS+=tc;
        if (FrameInfo.DTS!=(int64u)-1)
            FrameInfo.DTS+=tc;
        if (FrameInfo.PTS!=(int64u)-1 && (FrameInfo.PTS>PTS_End || (PTS_End>1000000000 && FrameInfo.PTS<=PTS_End-1000000000))) //More than current PTS_End or less than current PTS_End minus 1 second (there is a problem?)
            PTS_End=FrameInfo.PTS;

        #if MEDIAINFO_DUPLICATE
            if (Streams[(size_t)Element_Code].ShouldDuplicate)
                File__Duplicate_Write(Element_Code, (*seq_parameter_set_Item)->pic_order_cnt_type==0?pic_order_cnt_lsb:frame_num);
        #endif //MEDIAINFO_DUPLICATE

        //Filling only if not already done
        if (Frame_Count==1 && !Status[IsAccepted])
            Accept("AVC");
        if (!Status[IsFilled])
        {
            if (!GA94_03_IsPresent && IFrame_Count>=8)
                Frame_Count_Valid=Frame_Count; //We have enough frames
            if (Frame_Count>=Frame_Count_Valid)
            {
                Fill("AVC");
                if (!IsSub && !Streams[(size_t)Element_Code].ShouldDuplicate && MediaInfoLib::Config.ParseSpeed_Get()<1.0)
                    Finish("AVC");
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
//
void File_Avc::slice_data(bool AllCategories)
{
    Element_Begin1("slice_data");

    Skip_BS(Data_BS_Remain(),                                   "(ToDo)");

    Element_End0();
}

//---------------------------------------------------------------------------
//
void File_Avc::ref_pic_list_modification(int32u slice_type, bool mvc)
{
    if ((slice_type%5)!=2 && (slice_type%5)!=4)
    {
        TEST_SB_SKIP(                                           "ref_pic_list_modification_flag_l0");
            int32u modification_of_pic_nums_idc;
            do
            {
                Get_UE (modification_of_pic_nums_idc,           "modification_of_pic_nums_idc");
                if (modification_of_pic_nums_idc<2)
                    Skip_UE(                                    "abs_diff_pic_num_minus1");
                else if (modification_of_pic_nums_idc==2)
                    Skip_UE(                                    "long_term_pic_num");
                else if (mvc && (modification_of_pic_nums_idc==4 || modification_of_pic_nums_idc==5)) //ref_pic_list_mvc_modification only
                    Skip_UE(                                    "abs_diff_view_idx_minus1");
                else if (modification_of_pic_nums_idc!=3)
                {
                    Trusted_IsNot("ref_pic_list_modification_flag_l0");
                    Skip_BS(Data_BS_Remain(),                   "(Remaining bits)");
                }
            }
            while (modification_of_pic_nums_idc!=3 && Data_BS_Remain());
        TEST_SB_END();
    }
    if ((slice_type%5)==1)
    {
        TEST_SB_SKIP(                                           "ref_pic_list_modification_flag_l1");
            int32u modification_of_pic_nums_idc;
            do
            {
                Get_UE (modification_of_pic_nums_idc,           "modification_of_pic_nums_idc");
                if (modification_of_pic_nums_idc<2)
                    Skip_UE(                                    "abs_diff_pic_num_minus1");
                else if (modification_of_pic_nums_idc==2)
                    Skip_UE(                                    "long_term_pic_num");
                else if (mvc && (modification_of_pic_nums_idc==4 || modification_of_pic_nums_idc==5)) //ref_pic_list_mvc_modification only
                    Skip_UE(                                    "abs_diff_view_idx_minus1");
                else if (modification_of_pic_nums_idc!=3)
                {
                    Trusted_IsNot("ref_pic_list_modification_flag_l1");
                    Skip_BS(Data_BS_Remain(),                   "(Remaining bits)");
                }
            }
            while (modification_of_pic_nums_idc!=3 && Data_BS_Remain());
        TEST_SB_END();
    }
}

//---------------------------------------------------------------------------
//
void File_Avc::pred_weight_table(int32u num_ref_idx_l0_active_minus1, int32u num_ref_idx_l1_active_minus1, int8u ChromaArrayType)
{
    Skip_UE(                                                    "luma_log2_weight_denom");
    if (ChromaArrayType)
        Skip_UE(                                                "chroma_log2_weight_denom");
    for(int32u i=0; i<=num_ref_idx_l0_active_minus1; i++)
    {
        TEST_SB_SKIP(                                           "luma_weight_l0_flag");
            Skip_SE(                                            "luma_weight_l0");
            Skip_SE(                                            "luma_offset_l0");
        TEST_SB_END();
    }
    if (ChromaArrayType)
    {
        TEST_SB_SKIP(                                           "chroma_weight_l0_flag");
            Skip_SE(                                            "chroma_weight_l0");
            Skip_SE(                                            "chroma_offset_l0");
        TEST_SB_END();
    }
}

//---------------------------------------------------------------------------
//
void File_Avc::dec_ref_pic_marking(std::vector<int8u> &memory_management_control_operations)
{
    if (Element_Code==5) //IdrPicFlag
    {
        Skip_SB(                                                "no_output_of_prior_pics_flag");
        Skip_SB(                                                "long_term_reference_flag");
    }
    else
    {
        TEST_SB_SKIP(                                           "adaptive_ref_pic_marking_mode_flag");
            int32u memory_management_control_operation;
            do
            {
                Get_UE (memory_management_control_operation,    "memory_management_control_operation");
                switch (memory_management_control_operation)
                {
                    case 1 :
                                Skip_UE(                        "difference_of_pic_nums_minus1");
                                break;
                    case 2 :
                                Skip_UE(                        "long_term_pic_num");
                                break;
                    case 3 :
                                Skip_UE(                        "difference_of_pic_nums_minus1");
                                //break; 3 --> difference_of_pic_nums_minus1 then long_term_frame_idx
                    case 6 :
                                Skip_UE(                        "long_term_frame_idx");
                                break;
                    case 4 :
                                Skip_UE(                        "max_long_term_frame_idx_plus1");
                                break;
                }
                memory_management_control_operations.push_back((int8u)memory_management_control_operation);
            }
            while (Data_BS_Remain() && memory_management_control_operation);
        TEST_SB_END()
    }
}

//---------------------------------------------------------------------------
// Packet "06"
void File_Avc::sei()
{
    Element_Name("sei");

    //Parsing
    int32u seq_parameter_set_id=(int32u)-1;
    while(Element_Offset+1<Element_Size)
    {
        Element_Begin1("sei message");
            sei_message(seq_parameter_set_id);
        Element_End0();
    }
    BS_Begin();
    Mark_1(                                                     );
    BS_End();
}

//---------------------------------------------------------------------------
void File_Avc::sei_message(int32u &seq_parameter_set_id)
{
    //Parsing
    int32u payloadType=0, payloadSize=0;
    int8u payload_type_byte, payload_size_byte;
    Element_Begin1("sei message header");
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
    Element_End0();

    int64u Element_Offset_Save=Element_Offset+payloadSize;
    if (Element_Offset_Save>Element_Size)
    {
        Trusted_IsNot("Wrong size");
        Skip_XX(Element_Size-Element_Offset,                    "unknown");
        return;
    }
    int64u Element_Size_Save=Element_Size;
    Element_Size=Element_Offset_Save;
    switch (payloadType)
    {
        case  0 :   sei_message_buffering_period(seq_parameter_set_id); break;
        case  1 :   sei_message_pic_timing(payloadSize, seq_parameter_set_id); break;
        case  4 :   sei_message_user_data_registered_itu_t_t35(); break;
        case  5 :   sei_message_user_data_unregistered(payloadSize); break;
        case  6 :   sei_message_recovery_point(); break;
        case 32 :   sei_message_mainconcept(payloadSize); break;
        default :
                    Element_Info1("unknown");
                    Skip_XX(payloadSize,                        "data");
    }
    Element_Offset=Element_Offset_Save; //Positionning in the right place.
    Element_Size=Element_Size_Save; //Positionning in the right place.
}

//---------------------------------------------------------------------------
// SEI - 0
void File_Avc::sei_message_buffering_period(int32u &seq_parameter_set_id)
{
    Element_Info1("buffering_period");

    //Parsing
    if (Element_Offset==Element_Size)
        return; //Nothing to do
    BS_Begin();
    Get_UE (seq_parameter_set_id,                               "seq_parameter_set_id");
    std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item;
    if (seq_parameter_set_id>=seq_parameter_sets.size() || (*(seq_parameter_set_Item=seq_parameter_sets.begin()+seq_parameter_set_id))==NULL)
    {
        //Not yet present
        Skip_BS(Data_BS_Remain(),                               "Data (seq_parameter_set is missing)");
        BS_End();
        return;
    }
    if ((*seq_parameter_set_Item)->NalHrdBpPresentFlag())
        sei_message_buffering_period_xxl((*seq_parameter_set_Item)->vui_parameters->NAL);
    if ((*seq_parameter_set_Item)->VclHrdBpPresentFlag())
        sei_message_buffering_period_xxl((*seq_parameter_set_Item)->vui_parameters->VCL);
    BS_End();
}

void File_Avc::sei_message_buffering_period_xxl(seq_parameter_set_struct::vui_parameters_struct::xxl* xxl)
{
    if (xxl==NULL)
        return;
    for (int32u SchedSelIdx=0; SchedSelIdx<xxl->SchedSel.size(); SchedSelIdx++)
    {
        //Get_S4 (xxl->initial_cpb_removal_delay_length_minus1+1, xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay, "initial_cpb_removal_delay"); Param_Info2(xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay/90, " ms");
        //Get_S4 (xxl->initial_cpb_removal_delay_length_minus1+1, xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay_offset, "initial_cpb_removal_delay_offset"); Param_Info2(xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay_offset/90, " ms");
        Info_S4 (xxl->initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay, "initial_cpb_removal_delay"); Param_Info2(initial_cpb_removal_delay/90, " ms");
        Info_S4 (xxl->initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay_offset, "initial_cpb_removal_delay_offset"); Param_Info2(initial_cpb_removal_delay_offset/90, " ms");
    }
}

//---------------------------------------------------------------------------
// SEI - 1
void File_Avc::sei_message_pic_timing(int32u /*payloadSize*/, int32u seq_parameter_set_id)
{
    Element_Info1("pic_timing");

    //Testing if we can parsing it now. TODO: handle case seq_parameter_set_id is unknown (buffering of message, decoding in slice parsing)
    if (seq_parameter_set_id==(int32u)-1 && seq_parameter_sets.size()==1)
        seq_parameter_set_id=0;
    std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item;
    if (seq_parameter_set_id>=seq_parameter_sets.size() || (*(seq_parameter_set_Item=seq_parameter_sets.begin()+seq_parameter_set_id))==NULL)
    {
        //Not yet present
        Skip_BS(Data_BS_Remain(),                               "Data (seq_parameter_set is missing)");
        return;
    }

    //Parsing
    int8u   pic_struct=(int8u)-1;
    BS_Begin();
    if ((*seq_parameter_set_Item)->CpbDpbDelaysPresentFlag())
    {
        int8u cpb_removal_delay_length_minus1=(*seq_parameter_set_Item)->vui_parameters->NAL?(*seq_parameter_set_Item)->vui_parameters->NAL->cpb_removal_delay_length_minus1:(*seq_parameter_set_Item)->vui_parameters->VCL->cpb_removal_delay_length_minus1; //Spec is not precise, I am not sure
        int8u dpb_output_delay_length_minus1=(*seq_parameter_set_Item)->vui_parameters->NAL?(*seq_parameter_set_Item)->vui_parameters->NAL->dpb_output_delay_length_minus1:(*seq_parameter_set_Item)->vui_parameters->VCL->dpb_output_delay_length_minus1; //Spec is not precise, I am not sure
        Skip_S4(cpb_removal_delay_length_minus1+1,              "cpb_removal_delay");
        Skip_S4(dpb_output_delay_length_minus1+1,               "dpb_output_delay");
    }
    if ((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->pic_struct_present_flag)
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
            default : Param_Info1("Reserved"); return; //NumClockTS is unknown
        }
        Param_Info1(Avc_pic_struct[pic_struct]);
        int8u NumClockTS=Avc_NumClockTS[pic_struct];
        int8u seconds_value=0, minutes_value=0, hours_value=0; //Here because theses values can be reused in later ClockTSs.
        for (int8u i=0; i<NumClockTS; i++)
        {
            Element_Begin1("ClockTS");
            TEST_SB_SKIP(                                       "clock_timestamp_flag");
                Ztring TimeStamp;
                int32u time_offset=0;
                int8u n_frames;
                bool full_timestamp_flag, nuit_field_based_flag;
                Info_S1(2, ct_type,                             "ct_type"); Param_Info1(Avc_ct_type[ct_type]);
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
                TimeStamp=Ztring::ToZtring(hours_value)+__T(':')+Ztring::ToZtring(minutes_value)+__T(':')+Ztring::ToZtring(seconds_value);
                if ((*seq_parameter_set_Item)->CpbDpbDelaysPresentFlag())
                {
                    int8u time_offset_length=(*seq_parameter_set_Item)->vui_parameters->NAL?(*seq_parameter_set_Item)->vui_parameters->NAL->time_offset_length:(*seq_parameter_set_Item)->vui_parameters->VCL->time_offset_length; //Spec is not precise, I am not sure
                    if (time_offset_length)
                        Get_S4 (time_offset_length, time_offset,    "time_offset");
                }
                if ((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->timing_info_present_flag && (*seq_parameter_set_Item)->vui_parameters->time_scale)
                {
                    float32 Milliseconds=((float32)(n_frames*((*seq_parameter_set_Item)->vui_parameters->num_units_in_tick*(1+(nuit_field_based_flag?1:0)))+time_offset))/(*seq_parameter_set_Item)->vui_parameters->time_scale;
                    TimeStamp+=__T('.');
                    TimeStamp+=Ztring::ToZtring(Milliseconds);
                }
                Param_Info1(TimeStamp);
            TEST_SB_END();
            Element_End0();
        }
    }
    BS_End();

    FILLING_BEGIN();
        if ((*seq_parameter_set_Item)->pic_struct_FirstDetected==(int8u)-1 && (*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->pic_struct_present_flag)
            (*seq_parameter_set_Item)->pic_struct_FirstDetected=pic_struct;
    FILLING_END();
}

//---------------------------------------------------------------------------
// SEI - 5
void File_Avc::sei_message_user_data_registered_itu_t_t35()
{
    Element_Info1("user_data_registered_itu_t_t35");

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
    Element_Info1("Active Format Description");

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
        Info_S1(4, active_format,                               "active_format"); Param_Info1(Avc_user_data_DTG1_active_format[active_format]);
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
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        GA94_03_IsPresent=true;
        MustExtendParsingDuration=true;
        Buffer_TotalBytes_Fill_Max=(int64u)-1; //Disabling this feature for this format, this is done in the parser

        Element_Info1("DTVCC Transport");

        //Coherency
        delete TemporalReferences_DelayedElement; TemporalReferences_DelayedElement=new temporal_reference();

        TemporalReferences_DelayedElement->GA94_03=new temporal_reference::buffer_data;
        TemporalReferences_DelayedElement->GA94_03->Size=(size_t)(Element_Size-Element_Offset);
        delete[] TemporalReferences_DelayedElement->GA94_03->Data;
        TemporalReferences_DelayedElement->GA94_03->Data=new int8u[(size_t)(Element_Size-Element_Offset)];
        std::memcpy(TemporalReferences_DelayedElement->GA94_03->Data, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));

        //Parsing
        Skip_XX(Element_Size-Element_Offset,                    "CC data");
    #else //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        Skip_XX(Element_Size-Element_Offset,                    "DTVCC Transport data");
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
}

void File_Avc::sei_message_user_data_registered_itu_t_t35_GA94_03_Delayed(int32u seq_parameter_set_id)
{
    // Skipping missing frames
    if (TemporalReferences_Max-TemporalReferences_Min>(size_t)(4*(seq_parameter_sets[seq_parameter_set_id]->max_num_ref_frames+3))) // max_num_ref_frames ref frame maximum
    {
        TemporalReferences_Min=TemporalReferences_Max-4*(seq_parameter_sets[seq_parameter_set_id]->max_num_ref_frames+3);
        while (TemporalReferences[TemporalReferences_Min]==NULL)
            TemporalReferences_Min++;
    }

    // Parsing captions
    while (TemporalReferences[TemporalReferences_Min] && TemporalReferences_Min+2*seq_parameter_sets[seq_parameter_set_id]->max_num_ref_frames<TemporalReferences_Max)
    {
        #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
            Element_Begin1("Reordered DTVCC Transport");

            //Parsing
            #if MEDIAINFO_DEMUX
                int64u Element_Code_Old=Element_Code;
                Element_Code=0x4741393400000003LL;
            #endif //MEDIAINFO_DEMUX
            if (GA94_03_Parser==NULL)
            {
                GA94_03_Parser=new File_DtvccTransport;
                Open_Buffer_Init(GA94_03_Parser);
                ((File_DtvccTransport*)GA94_03_Parser)->Format=File_DtvccTransport::Format_A53_4_GA94_03;
            }
            if (((File_DtvccTransport*)GA94_03_Parser)->AspectRatio==0)
            {
                float64 PixelAspectRatio=1;
                std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item=seq_parameter_sets.begin();
                for (; seq_parameter_set_Item!=seq_parameter_sets.end(); ++seq_parameter_set_Item)
                    if ((*seq_parameter_set_Item))
                        break;
                if (seq_parameter_set_Item!=seq_parameter_sets.end())
                {
                    if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_info_present_flag)
                    {
                        if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc<Avc_PixelAspectRatio_Size)
                            PixelAspectRatio=Avc_PixelAspectRatio[(*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc];
                        else if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc==0xFF && (*seq_parameter_set_Item)->vui_parameters->sar_height)
                            PixelAspectRatio=((float64)(*seq_parameter_set_Item)->vui_parameters->sar_width)/(*seq_parameter_set_Item)->vui_parameters->sar_height;
                    }
                    int32u Width =((*seq_parameter_set_Item)->pic_width_in_mbs_minus1       +1)*16;
                    int32u Height=((*seq_parameter_set_Item)->pic_height_in_map_units_minus1+1)*16*(2-(*seq_parameter_set_Item)->frame_mbs_only_flag);
                    ((File_DtvccTransport*)GA94_03_Parser)->AspectRatio=Width*PixelAspectRatio/Height;
                }
            }
            if (GA94_03_Parser->PTS_DTS_Needed)
            {
                GA94_03_Parser->FrameInfo.PCR=FrameInfo.PCR;
                GA94_03_Parser->FrameInfo.PTS=FrameInfo.PTS;
                GA94_03_Parser->FrameInfo.DTS=FrameInfo.DTS;
            }
            #if MEDIAINFO_DEMUX
                if (TemporalReferences[TemporalReferences_Min]->GA94_03)
                {
                    int8u Demux_Level_Save=Demux_Level;
                    Demux_Level=8; //Ancillary
                    Demux(TemporalReferences[TemporalReferences_Min]->GA94_03->Data, TemporalReferences[TemporalReferences_Min]->GA94_03->Size, ContentType_MainStream);
                    Demux_Level=Demux_Level_Save;
                }
                Element_Code=Element_Code_Old;
            #endif //MEDIAINFO_DEMUX
            if (TemporalReferences[TemporalReferences_Min]->GA94_03)
            {
                #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                    GA94_03_Parser->ServiceDescriptors=ServiceDescriptors;
                #endif
                Open_Buffer_Continue(GA94_03_Parser, TemporalReferences[TemporalReferences_Min]->GA94_03->Data, TemporalReferences[TemporalReferences_Min]->GA94_03->Size);
            }

            Element_End0();
        #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)

        TemporalReferences_Min+=((seq_parameter_sets[seq_parameter_set_id]->frame_mbs_only_flag | !TemporalReferences[TemporalReferences_Min]->IsField)?2:1);
    }
}

//---------------------------------------------------------------------------
// SEI - 5 - GA94 - 0x03
void File_Avc::sei_message_user_data_registered_itu_t_t35_GA94_06()
{
    Element_Info1("Bar data");

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
    Element_Info1("user_data_unregistered");

    //Parsing
    int128u uuid_iso_iec_11578;
    Get_GUID(uuid_iso_iec_11578,                                "uuid_iso_iec_11578");

    switch (uuid_iso_iec_11578.hi)
    {
        case 0xB748D9E6BDE945DCLL : Element_Info1("x264");
                                     sei_message_user_data_unregistered_x264(payloadSize-16); break;
        case 0x684E92AC604A57FBLL : Element_Info1("eavc");
                                     sei_message_user_data_unregistered_x264(payloadSize-16); break;
        case 0xD9114Df8608CEE17LL : Element_Info1("Blu-ray");
                                    sei_message_user_data_unregistered_bluray(payloadSize-16); break;
        default :
                    Element_Info1("unknown");
                    Skip_XX(payloadSize-16,                     "data");
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
    size_t Data_Pos_Before=0;
    size_t Loop=0;
    do
    {
        size_t Data_Pos=Data.find(__T(" - "), Data_Pos_Before);
        if (Data_Pos==std::string::npos)
            Data_Pos=Data.size();
        if (Data.find(__T("options: "), Data_Pos_Before)==Data_Pos_Before)
        {
            Element_Begin1("options");
            size_t Options_Pos_Before=Data_Pos_Before;
            Encoded_Library_Settings.clear();
            do
            {
                size_t Options_Pos=Data.find(__T(" "), Options_Pos_Before);
                if (Options_Pos==std::string::npos)
                    Options_Pos=Data.size();
                Ztring option;
                Get_Local (Options_Pos-Options_Pos_Before, option, "option");
                Options_Pos_Before=Options_Pos;
                do
                {
                    Ztring Separator;
                    Peek_Local(1, Separator);
                    if (Separator==__T(" "))
                    {
                        Skip_Local(1,                               "separator");
                        Options_Pos_Before+=1;
                    }
                    else
                        break;
                }
                while (Options_Pos_Before!=Data.size());

                //Filling
                if (option!=__T("options:"))
                {
                    if (!Encoded_Library_Settings.empty())
                        Encoded_Library_Settings+=__T(" / ");
                    Encoded_Library_Settings+=option;
                    if (option.find(__T("bitrate="))==0)
                        BitRate_Nominal=option.substr(8)+__T("000"); //After "bitrate="
                }
            }
            while (Options_Pos_Before!=Data.size());
            Element_End0();
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
            if (Loop==1 && Encoded_Library.find(__T("x264"))==0)
            {
                Encoded_Library+=__T(" - ");
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
    if (Encoded_Library.find(__T("eavc "))==0)
    {
        Encoded_Library_Name=__T("eavc");
        Encoded_Library_Version=Encoded_Library.SubString(__T("eavc "), __T(""));
    }
    else if (Encoded_Library.find(__T("x264 - "))==0)
    {
        Encoded_Library_Name=__T("x264");
        Encoded_Library_Version=Encoded_Library.SubString(__T("x264 - "), __T(""));
    }
    else if (Encoded_Library.find(__T("SUPER(C) by eRightSoft "))==0)
    {
        Encoded_Library_Name=__T("SUPER(C) by eRightSoft");
        Encoded_Library_Date=Ztring(__T("UTC "))+Encoded_Library.SubString(__T("2000-"), __T(" "));
    }
    else
        Encoded_Library_Name=Encoded_Library;
}

//---------------------------------------------------------------------------
// SEI - 5 - x264
void File_Avc::sei_message_user_data_unregistered_bluray(int32u payloadSize)
{
    if (payloadSize<4)
    {
        Skip_XX(payloadSize,                                    "Unknown");
        return;
    }
    int32u Identifier;
    Get_B4 (Identifier,                                         "Identifier");
    switch (Identifier)
    {
        case 0x47413934 :   sei_message_user_data_registered_itu_t_t35_GA94_03(); return;
        default         :   Skip_XX(Element_Size-Element_Offset, "Unknown");
    }
}

//---------------------------------------------------------------------------
// SEI - 6
void File_Avc::sei_message_recovery_point()
{
    Element_Info1("recovery_point");

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
    Element_Info1("MainConcept text");

    //Parsing
    Ztring Text;
    Get_Local(payloadSize, Text,                                "text");

    if (Text.find(__T("produced by MainConcept H.264/AVC Codec v"))!=std::string::npos)
    {
        Encoded_Library=Text.SubString(__T("produced by "), __T(" MainConcept AG"));
        Encoded_Library_Name=__T("MainConcept H.264/AVC Codec");
        Encoded_Library_Version=Text.SubString(__T("produced by MainConcept H.264/AVC Codec v"), __T(" (c) "));
        Encoded_Library_Date=MediaInfoLib::Config.Library_Get(InfoLibrary_Format_MainConcept_Avc, Encoded_Library_Version, InfoLibrary_Date);
    }
}

//---------------------------------------------------------------------------
// Packet "07"
void File_Avc::seq_parameter_set()
{
    Element_Name("seq_parameter_set");

    //parsing
    int32u seq_parameter_set_id;
    if (!seq_parameter_set_data(seq_parameter_sets, seq_parameter_set_id))
        return;
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
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "08"
void File_Avc::pic_parameter_set()
{
    Element_Name("pic_parameter_set");

    //Parsing
    int32u  pic_parameter_set_id, seq_parameter_set_id, num_slice_groups_minus1, num_ref_idx_l0_default_active_minus1, num_ref_idx_l1_default_active_minus1, slice_group_map_type=0;
    int8u   weighted_bipred_idc=0;
    bool    entropy_coding_mode_flag,bottom_field_pic_order_in_frame_present_flag, redundant_pic_cnt_present_flag, weighted_pred_flag, deblocking_filter_control_present_flag;
    BS_Begin();
    Get_UE (pic_parameter_set_id,                               "pic_parameter_set_id");
    Get_UE (seq_parameter_set_id,                               "seq_parameter_set_id");
    std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item;
    if (seq_parameter_set_id>=seq_parameter_sets.size() || (*(seq_parameter_set_Item=seq_parameter_sets.begin()+seq_parameter_set_id))==NULL)
    {
        if (seq_parameter_set_id>=subset_seq_parameter_sets.size() || (*(seq_parameter_set_Item=subset_seq_parameter_sets.begin()+seq_parameter_set_id))==NULL)
        {
            //Not yet present
            Skip_BS(Data_BS_Remain(),                               "Data (seq_parameter_set is missing)");
            return;
        }
    }
    Get_SB (entropy_coding_mode_flag,                           "entropy_coding_mode_flag");
    Get_SB (bottom_field_pic_order_in_frame_present_flag,       "bottom_field_pic_order_in_frame_present_flag");
    Get_UE (num_slice_groups_minus1,                            "num_slice_groups_minus1");
    if (num_slice_groups_minus1>7)
    {
        Trusted_IsNot("num_slice_groups_minus1 too high");
        num_slice_groups_minus1=0;
    }
    if (num_slice_groups_minus1>0)
    {
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
            if(pic_size_in_map_units_minus1>((*seq_parameter_set_Item)->pic_width_in_mbs_minus1+1)*((*seq_parameter_set_Item)->pic_height_in_map_units_minus1+1))
            {
                Trusted_IsNot("pic_size_in_map_units_minus1 too high");
                return;
            }
            #if defined (__mips__)       || defined (__mipsel__)
                int32u slice_group_id_Size=(int32u)(std::ceil(std::log((double)(num_slice_groups_minus1+1))/std::log((double)10))); //std::log is natural logarithm
            #else
                int32u slice_group_id_Size=(int32u)(std::ceil(std::log((float32)(num_slice_groups_minus1+1))/std::log((float32)10))); //std::log is natural logarithm
            #endif
            for (int32u Pos=0; Pos<=pic_size_in_map_units_minus1; Pos++)
                Skip_BS(slice_group_id_Size,                    "slice_group_id");
        }
    }
    Get_UE (num_ref_idx_l0_default_active_minus1,               "num_ref_idx_l0_default_active_minus1");
    Get_UE (num_ref_idx_l1_default_active_minus1,               "num_ref_idx_l1_default_active_minus1");
    Get_SB (weighted_pred_flag,                                 "weighted_pred_flag");
    Get_S1 (2, weighted_bipred_idc,                             "weighted_bipred_idc");
    Skip_SE(                                                    "pic_init_qp_minus26");
    Skip_SE(                                                    "pic_init_qs_minus26");
    Skip_SE(                                                    "chroma_qp_index_offset");
    Get_SB (deblocking_filter_control_present_flag,             "deblocking_filter_control_present_flag");
    Skip_SB(                                                    "constrained_intra_pred_flag");
    Get_SB (redundant_pic_cnt_present_flag,                     "redundant_pic_cnt_present_flag");
    bool more_rbsp_data=false;
    if (Element_Size)
    {
        int64u Offset=Element_Size-1;
        while (Offset && Buffer[Buffer_Offset+(size_t)Offset]==0x00) //Searching if there are NULL bytes at the end of the data
            Offset--;
        size_t Bit_Pos=7;
        while (Bit_Pos && !(Buffer[Buffer_Offset+(size_t)Offset]&(1<<(7-Bit_Pos))))
            Bit_Pos--;
        if (Data_BS_Remain()>1+(7-Bit_Pos)+(Element_Size-Offset-1)*8)
            more_rbsp_data=true;
    }
    if (more_rbsp_data)
    {
        bool transform_8x8_mode_flag;
        Get_SB (transform_8x8_mode_flag,                        "transform_8x8_mode_flag");
        TEST_SB_SKIP(                                           "pic_scaling_matrix_present_flag");
        for (int8u Pos=0; Pos<6+(transform_8x8_mode_flag?((*seq_parameter_set_Item)->chroma_format_idc!=3?2:6):0); Pos++ )
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
        //Integrity
        if (pic_parameter_set_id>=256)
        {
            Trusted_IsNot("pic_parameter_set_id not valid");
            return; //Problem, not valid
        }
        if (seq_parameter_set_id>=32)
        {
            Trusted_IsNot("seq_parameter_set_id not valid");
            return; //Problem, not valid
        }

        //NextCode
        NextCode_Clear();
        NextCode_Add(0x05);
        NextCode_Add(0x06);
        if (!subset_seq_parameter_sets.empty())
            NextCode_Add(0x14); //slice_layer_extension

        //Filling
        if (pic_parameter_set_id>=pic_parameter_sets.size())
            pic_parameter_sets.resize(pic_parameter_set_id+1);
        std::vector<pic_parameter_set_struct*>::iterator pic_parameter_sets_Item=pic_parameter_sets.begin()+pic_parameter_set_id;
        delete *pic_parameter_sets_Item; *pic_parameter_sets_Item = new pic_parameter_set_struct(
                                                                                                    (int8u)seq_parameter_set_id,
                                                                                                    (int8u)num_ref_idx_l0_default_active_minus1,
                                                                                                    (int8u)num_ref_idx_l1_default_active_minus1,
                                                                                                    weighted_bipred_idc,
                                                                                                    num_slice_groups_minus1,
                                                                                                    slice_group_map_type,
                                                                                                    entropy_coding_mode_flag,
                                                                                                    bottom_field_pic_order_in_frame_present_flag,
                                                                                                    weighted_pred_flag,
                                                                                                    redundant_pic_cnt_present_flag,
                                                                                                    deblocking_filter_control_present_flag
                                                                                                );

        //Autorisation of other streams
        if (!seq_parameter_sets.empty())
        {
            for (int8u Pos=0x01; Pos<=0x06; Pos++)
            {
                Streams[Pos].Searching_Payload=true; //Coded slice...
                if (Streams[0x08].ShouldDuplicate)
                    Streams[Pos].ShouldDuplicate=true;
            }
        }
        if (!subset_seq_parameter_sets.empty())
        {
            Streams[0x14].Searching_Payload=true; //slice_layer_extension
            if (Streams[0x08].ShouldDuplicate)
                Streams[0x14].ShouldDuplicate=true; //slice_layer_extension
        }

        //Setting as OK
        if (!Status[IsAccepted])
            Accept("AVC");
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "09"
void File_Avc::access_unit_delimiter()
{
    Element_Name("access_unit_delimiter");

    int8u primary_pic_type;
    BS_Begin();
    Get_S1 ( 3, primary_pic_type,                               "primary_pic_type"); Param_Info1(Avc_primary_pic_type[primary_pic_type]);
    Mark_1_NoTrustError(                                        ); //Found 1 file without this bit
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
void File_Avc::prefix_nal_unit(bool svc_extension_flag)
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
    int32u subset_seq_parameter_set_id;
    if (!seq_parameter_set_data(subset_seq_parameter_sets, subset_seq_parameter_set_id))
        return;
    std::vector<seq_parameter_set_struct*>::iterator subset_seq_parameter_sets_Item=subset_seq_parameter_sets.begin()+subset_seq_parameter_set_id;
    if ((*subset_seq_parameter_sets_Item)->profile_idc==83 || (*subset_seq_parameter_sets_Item)->profile_idc==86)
    {
        //bool svc_vui_parameters_present_flag;
        seq_parameter_set_svc_extension();
        /* The rest is not yet implemented
        Get_SB (svc_vui_parameters_present_flag,                "svc_vui_parameters_present_flag");
        if (svc_vui_parameters_present_flag)
            svc_vui_parameters_extension();
        */
    }
    else if ((*subset_seq_parameter_sets_Item)->profile_idc==118 || (*subset_seq_parameter_sets_Item)->profile_idc==128)
    {
        //bool mvc_vui_parameters_present_flag, additional_extension2_flag;
        Mark_1();
        seq_parameter_set_mvc_extension(subset_seq_parameter_set_id);
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
void File_Avc::slice_layer_extension(bool svc_extension_flag)
{
    Element_Name("slice_layer_extension");

    //Parsing
    if (svc_extension_flag)
    {
        Skip_XX(Element_Size-Element_Offset,                    "slice_header_in_scalable_extension + slice_data_in_scalable_extension");
    }
    else
    {
        BS_Begin();
        slice_header();
        slice_data(true);
        BS_End();
    }
}

//***************************************************************************
// SubElements
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Avc::seq_parameter_set_data(std::vector<seq_parameter_set_struct*> &Data, int32u &Data_id)
{
    //Parsing
    seq_parameter_set_struct::vui_parameters_struct* vui_parameters_Item=NULL;
    int32u  chroma_format_idc=1, bit_depth_luma_minus8=0, bit_depth_chroma_minus8=0, log2_max_frame_num_minus4, pic_order_cnt_type, log2_max_pic_order_cnt_lsb_minus4=(int32u)-1, max_num_ref_frames, pic_width_in_mbs_minus1, pic_height_in_map_units_minus1, frame_crop_left_offset=0, frame_crop_right_offset=0, frame_crop_top_offset=0, frame_crop_bottom_offset=0;
    int8u   profile_idc, level_idc;
    bool    constraint_set3_flag, separate_colour_plane_flag=false, delta_pic_order_always_zero_flag=false, frame_mbs_only_flag, mb_adaptive_frame_field_flag=false;
    Get_B1 (profile_idc,                                        "profile_idc");
    BS_Begin();
    Element_Begin1("constraints");
        Skip_SB(                                                "constraint_set0_flag");
        Skip_SB(                                                "constraint_set1_flag");
        Skip_SB(                                                "constraint_set2_flag");
        Get_SB (constraint_set3_flag,                           "constraint_set3_flag");
        Skip_SB(                                                "constraint_set4_flag");
        Skip_SB(                                                "constraint_set5_flag");
        Skip_BS(2,                                              "reserved_zero_2bits");
    Element_End0();
    Get_S1 ( 8, level_idc,                                      "level_idc");
    Get_UE (    Data_id,                                        "seq_parameter_set_id");
    switch (profile_idc)
    {
        case 100 :
        case 110 :
        case 122 :
        case 244 :
        case  44 :
        case  83 :
        case  86 :
        case 118 :
        case 128 :  //High profiles
        case 138 :
                    Element_Begin1("high profile specific");
                    Get_UE (chroma_format_idc,                  "chroma_format_idc"); Param_Info1C((chroma_format_idc<3), Avc_Colorimetry_format_idc[chroma_format_idc]);
                    if (chroma_format_idc==3)
                        Get_SB (separate_colour_plane_flag,     "separate_colour_plane_flag");
                    Get_UE (bit_depth_luma_minus8,              "bit_depth_luma_minus8");
                    Get_UE (bit_depth_chroma_minus8,            "bit_depth_chroma_minus8");
                    Skip_SB(                                    "qpprime_y_zero_transform_bypass_flag");
                    TEST_SB_SKIP(                               "seq_scaling_matrix_present_flag");
                        for (int32u Pos=0; Pos<(int32u)((chroma_format_idc!=3) ? 8 : 12); Pos++)
                        {
                            TEST_SB_SKIP(                       "seq_scaling_list_present_flag");
                                scaling_list(Pos<6?16:64);
                            TEST_SB_END();
                        }
                    TEST_SB_END();
                    Element_End0();
                    break;
        default  :  ;
    }
    Get_UE (log2_max_frame_num_minus4,                          "log2_max_frame_num_minus4");
    Get_UE (pic_order_cnt_type,                                 "pic_order_cnt_type");
    if (pic_order_cnt_type==0)
        Get_UE (log2_max_pic_order_cnt_lsb_minus4,              "log2_max_pic_order_cnt_lsb_minus4");
    else if (pic_order_cnt_type==1)
    {
        int32u num_ref_frames_in_pic_order_cnt_cycle;
        Get_SB (delta_pic_order_always_zero_flag,               "delta_pic_order_always_zero_flag");
        Skip_SE(                                                "offset_for_non_ref_pic");
        Skip_SE(                                                "offset_for_top_to_bottom_field");
        Get_UE (num_ref_frames_in_pic_order_cnt_cycle,          "num_ref_frames_in_pic_order_cnt_cycle");
        if (num_ref_frames_in_pic_order_cnt_cycle>=256)
        {
            Trusted_IsNot("num_ref_frames_in_pic_order_cnt_cycle too high");
            return false;
        }
        for(int32u Pos=0; Pos<num_ref_frames_in_pic_order_cnt_cycle; Pos++)
            Skip_SE(                                            "offset_for_ref_frame");
    }
    else if (pic_order_cnt_type > 2)
    {
        Trusted_IsNot("pic_order_cnt_type not supported");
        return false;
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
        vui_parameters(vui_parameters_Item);
    TEST_SB_END();

    FILLING_BEGIN();
        //Integrity
        if (Data_id>=32)
        {
            Trusted_IsNot("seq_parameter_set_id not valid");
            delete (seq_parameter_set_struct::vui_parameters_struct*)vui_parameters_Item;
            return false; //Problem, not valid
        }
        if (pic_order_cnt_type==0 && log2_max_pic_order_cnt_lsb_minus4>12)
        {
            Trusted_IsNot("log2_max_pic_order_cnt_lsb_minus4 not valid");
            delete (seq_parameter_set_struct::vui_parameters_struct*)vui_parameters_Item;
            return false; //Problem, not valid
        }
        if (log2_max_frame_num_minus4>12)
        {
            Trusted_IsNot("log2_max_frame_num_minus4 not valid");
            delete (seq_parameter_set_struct::vui_parameters_struct*)vui_parameters_Item;
            return false; //Problem, not valid
        }

        //Creating Data
        if (Data_id>=Data.size())
            Data.resize(Data_id+1);
        std::vector<seq_parameter_set_struct*>::iterator Data_Item=Data.begin()+Data_id;
        delete *Data_Item; *Data_Item=new seq_parameter_set_struct(
                                                                    vui_parameters_Item,
                                                                    pic_width_in_mbs_minus1,
                                                                    pic_height_in_map_units_minus1,
                                                                    frame_crop_left_offset,
                                                                    frame_crop_right_offset,
                                                                    frame_crop_top_offset,
                                                                    frame_crop_bottom_offset,
                                                                    (int8u)chroma_format_idc,
                                                                    profile_idc,
                                                                    level_idc,
                                                                    (int8u)bit_depth_luma_minus8,
                                                                    (int8u)bit_depth_chroma_minus8,
                                                                    (int8u)log2_max_frame_num_minus4,
                                                                    (int8u)pic_order_cnt_type,
                                                                    (int8u)log2_max_pic_order_cnt_lsb_minus4,
                                                                    (int8u)max_num_ref_frames,
                                                                    constraint_set3_flag,
                                                                    separate_colour_plane_flag,
                                                                    delta_pic_order_always_zero_flag,
                                                                    frame_mbs_only_flag,
                                                                    mb_adaptive_frame_field_flag
                                                                  );

        //Computing values (for speed)
        size_t MaxNumber;
        switch (pic_order_cnt_type)
        {
            case 0 :
                        MaxNumber=(*Data_Item)->MaxPicOrderCntLsb;
                        break;
            case 1 :
            case 2 :
                        MaxNumber=(*Data_Item)->MaxFrameNum*2;
                        break;
            default:
                        MaxNumber = 0;
        }

        if (MaxNumber>TemporalReferences_Reserved)
        {
            TemporalReferences.resize(4*MaxNumber);
            TemporalReferences_Reserved=MaxNumber;
        }
    FILLING_ELSE();
        delete vui_parameters_Item; //vui_parameters_Item=NULL;
        return false;
    FILLING_END();
    return true;
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
void File_Avc::vui_parameters(seq_parameter_set_struct::vui_parameters_struct* &vui_parameters_Item_)
{
    //Parsing
    seq_parameter_set_struct::vui_parameters_struct::xxl *NAL=NULL, *VCL=NULL;
    seq_parameter_set_struct::vui_parameters_struct::bitstream_restriction_struct* bitstream_restriction=NULL;
    int32u  num_units_in_tick=(int32u)-1, time_scale=(int32u)-1;
    int16u  sar_width=(int16u)-1, sar_height=(int16u)-1;
    int8u   aspect_ratio_idc=0, video_format=5, video_full_range_flag = 0, colour_primaries=2, transfer_characteristics=2, matrix_coefficients=2;
    bool    aspect_ratio_info_present_flag, video_signal_type_present_flag, colour_description_present_flag=false, timing_info_present_flag, fixed_frame_rate_flag=false, nal_hrd_parameters_present_flag, vcl_hrd_parameters_present_flag, pic_struct_present_flag;
    TEST_SB_GET (aspect_ratio_info_present_flag,                "aspect_ratio_info_present_flag");
        Get_S1 (8, aspect_ratio_idc,                            "aspect_ratio_idc"); Param_Info1C((aspect_ratio_idc<Avc_PixelAspectRatio_Size), Avc_PixelAspectRatio[aspect_ratio_idc]);
        if (aspect_ratio_idc==0xFF)
        {
            Get_S2 (16, sar_width,                              "sar_width");
            Get_S2 (16, sar_height,                             "sar_height");
        }
    TEST_SB_END();
    TEST_SB_SKIP(                                               "overscan_info_present_flag");
        Skip_SB(                                                "overscan_appropriate_flag");
    TEST_SB_END();
    TEST_SB_GET (video_signal_type_present_flag,                "video_signal_type_present_flag");
        Get_S1 (3, video_format,                                "video_format"); Param_Info1(Avc_video_format[video_format]);
        Get_S1 (1, video_full_range_flag,                       "video_full_range_flag"); Param_Info1(Avc_video_full_range[video_full_range_flag]);
        TEST_SB_GET (colour_description_present_flag,           "colour_description_present_flag");
            Get_S1 (8, colour_primaries,                        "colour_primaries"); Param_Info1(Mpegv_colour_primaries(colour_primaries));
            Get_S1 (8, transfer_characteristics,                "transfer_characteristics"); Param_Info1(Mpegv_transfer_characteristics(transfer_characteristics));
            Get_S1 (8, matrix_coefficients,                     "matrix_coefficients"); Param_Info1(Mpegv_matrix_coefficients(matrix_coefficients));
        TEST_SB_END();
    TEST_SB_END();
    TEST_SB_SKIP(                                               "chroma_loc_info_present_flag");
        Skip_UE(                                                "chroma_sample_loc_type_top_field");
        Skip_UE(                                                "chroma_sample_loc_type_bottom_field");
    TEST_SB_END();
    TEST_SB_GET (timing_info_present_flag,                      "timing_info_present_flag");
        Get_S4 (32, num_units_in_tick,                          "num_units_in_tick");
        Get_S4 (32, time_scale,                                 "time_scale");
        Get_SB (    fixed_frame_rate_flag,                      "fixed_frame_rate_flag");
    TEST_SB_END();
    TEST_SB_GET (nal_hrd_parameters_present_flag,               "nal_hrd_parameters_present_flag");
        hrd_parameters(NAL);
    TEST_SB_END();
    TEST_SB_GET (vcl_hrd_parameters_present_flag,               "vcl_hrd_parameters_present_flag");
        hrd_parameters(VCL);
    TEST_SB_END();
    if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
        Skip_SB(                                                "low_delay_hrd_flag");
    Get_SB (   pic_struct_present_flag,                         "pic_struct_present_flag");
    TEST_SB_SKIP(                                               "bitstream_restriction_flag");
        int32u  max_num_reorder_frames;
        Skip_SB(                                                "motion_vectors_over_pic_boundaries_flag");
        Skip_UE(                                                "max_bytes_per_pic_denom");
        Skip_UE(                                                "max_bits_per_mb_denom");
        Skip_UE(                                                "log2_max_mv_length_horizontal");
        Skip_UE(                                                "log2_max_mv_length_vertical");
        Get_UE (max_num_reorder_frames,                         "max_num_reorder_frames");
        Skip_UE(                                                "max_dec_frame_buffering");
        if (max_num_reorder_frames<256)
            bitstream_restriction=new seq_parameter_set_struct::vui_parameters_struct::bitstream_restriction_struct(
                                                                                                                    (int8u)max_num_reorder_frames
                                                                                                                   );
    TEST_SB_END();

    FILLING_BEGIN();
        vui_parameters_Item_=new seq_parameter_set_struct::vui_parameters_struct(
                                                                                    NAL,
                                                                                    VCL,
                                                                                    bitstream_restriction,
                                                                                    num_units_in_tick,
                                                                                    time_scale,
                                                                                    sar_width,
                                                                                    sar_height,
                                                                                    aspect_ratio_idc,
                                                                                    video_format,
                                                                                    video_full_range_flag,
                                                                                    colour_primaries,
                                                                                    transfer_characteristics,
                                                                                    matrix_coefficients,
                                                                                    aspect_ratio_info_present_flag,
                                                                                    video_signal_type_present_flag,
                                                                                    colour_description_present_flag,
                                                                                    timing_info_present_flag,
                                                                                    fixed_frame_rate_flag,
                                                                                    pic_struct_present_flag
                                                                                );
    FILLING_ELSE();
        delete NAL;
        delete VCL;
        delete bitstream_restriction;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Avc::hrd_parameters(seq_parameter_set_struct::vui_parameters_struct::xxl* &hrd_parameters_Item_)
{
    //Parsing
    int32u cpb_cnt_minus1;
    int8u  bit_rate_scale, cpb_size_scale, initial_cpb_removal_delay_length_minus1, cpb_removal_delay_length_minus1, dpb_output_delay_length_minus1, time_offset_length;
    Get_UE (   cpb_cnt_minus1,                                  "cpb_cnt_minus1");
    Get_S1 (4, bit_rate_scale,                                  "bit_rate_scale");
    Get_S1 (4, cpb_size_scale,                                  "cpb_size_scale");
    if (cpb_cnt_minus1>31)
    {
        Trusted_IsNot("cpb_cnt_minus1 too high");
        cpb_cnt_minus1=0;
    }
    vector<seq_parameter_set_struct::vui_parameters_struct::xxl::xxl_data>  SchedSel;
    SchedSel.reserve(cpb_cnt_minus1+1);
    for (int8u SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; ++SchedSelIdx)
    {
        Element_Begin1("ShedSel");
        int64u bit_rate_value, cpb_size_value;
        int32u bit_rate_value_minus1, cpb_size_value_minus1;
        bool cbr_flag;
        Get_UE (bit_rate_value_minus1,                          "bit_rate_value_minus1");
        bit_rate_value=(int64u)((bit_rate_value_minus1+1)*pow(2.0, 6+bit_rate_scale)); Param_Info2(bit_rate_value, " bps");
        Get_UE (cpb_size_value_minus1,                          "cpb_size_value_minus1");
        cpb_size_value=(int64u)((cpb_size_value_minus1+1)*pow(2.0, 4+cpb_size_scale)); Param_Info2(cpb_size_value, " bits");
        Get_SB (cbr_flag,                                       "cbr_flag");
        Element_End0();

        FILLING_BEGIN();
            SchedSel.push_back(seq_parameter_set_struct::vui_parameters_struct::xxl::xxl_data(
                                                                                                bit_rate_value,
                                                                                                cpb_size_value,
                                                                                                cbr_flag
                                                                                             ));
        FILLING_END();
    }
    Get_S1 (5, initial_cpb_removal_delay_length_minus1,         "initial_cpb_removal_delay_length_minus1");
    Get_S1 (5, cpb_removal_delay_length_minus1,                 "cpb_removal_delay_length_minus1");
    Get_S1 (5, dpb_output_delay_length_minus1,                  "dpb_output_delay_length_minus1");
    Get_S1 (5, time_offset_length,                              "time_offset_length");

    //Validity test
    if (!Element_IsOK() || (SchedSel.size() == 1 && SchedSel[0].bit_rate_value == 64))
    {
        return; //We do not trust this kind of data
    }

    //Filling
    hrd_parameters_Item_=new seq_parameter_set_struct::vui_parameters_struct::xxl(
                                                                                    SchedSel,
                                                                                    initial_cpb_removal_delay_length_minus1,
                                                                                    cpb_removal_delay_length_minus1,
                                                                                    dpb_output_delay_length_minus1,
                                                                                    time_offset_length
                                                                                 );
}

//---------------------------------------------------------------------------
void File_Avc::nal_unit_header_svc_extension()
{
    //Parsing
    Element_Begin1("nal_unit_header_svc_extension");
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
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Avc::nal_unit_header_mvc_extension()
{
    //Parsing
    Element_Begin1("nal_unit_header_mvc_extension");
    Skip_SB(                                                    "non_idr_flag");
    Skip_S1( 6,                                                 "priority_id");
    Skip_S1(10,                                                 "view_id");
    Skip_S1( 3,                                                 "temporal_id");
    Skip_SB(                                                    "anchor_pic_flag");
    Skip_SB(                                                    "inter_view_flag");
    Skip_SB(                                                    "reserved_one_bit");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Avc::seq_parameter_set_svc_extension()
{
    //Parsing
    Element_Begin1("seq_parameter_set_svc_extension");
    //Skip_SB(                                                    "");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Avc::svc_vui_parameters_extension()
{
    //Parsing
    Element_Begin1("svc_vui_parameters_extension");
    //Skip_SB(                                                    "");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Avc::seq_parameter_set_mvc_extension(int32u subset_seq_parameter_sets_id)
{
    //Parsing
    Element_Begin1("seq_parameter_set_mvc_extension");
    int32u num_views_minus1;
    Get_UE (num_views_minus1,                                   "num_views_minus1");
    //(Not implemented)
    Element_End0();

    FILLING_BEGIN();
        subset_seq_parameter_sets[subset_seq_parameter_sets_id]->num_views_minus1 = (int16u)num_views_minus1;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Avc::mvc_vui_parameters_extension()
{
    //Parsing
    Element_Begin1("mvc_vui_parameters_extension");
    Skip_SB(                                                    "");
    Element_End0();
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
        Element_Begin1("seq_parameter_set");
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
        Element_Size=Size-(Size?1:0);
        Element_Code=0x07; //seq_parameter_set
        Data_Parse();
        Buffer_Offset-=(size_t)Element_Offset_Save;
        Element_Offset=Element_Offset_Save+Size-1;
        Element_Size=Element_Size_Save;
        Element_End0();
    }
    Get_B1 (pic_parameter_set_count,                            "pic_parameter_set count");
    for (int8u Pos=0; Pos<pic_parameter_set_count; Pos++)
    {
        Element_Begin1("pic_parameter_set");
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
        Element_End0();
    }
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding?");

    //Filling
    FILLING_BEGIN_PRECISE();
        //Detection of some bugs in the file
        if (!seq_parameter_sets.empty() && seq_parameter_sets[0] && (Profile!=seq_parameter_sets[0]->profile_idc || Level!=seq_parameter_sets[0]->level_idc))
            MuxingMode=Ztring("Container profile=")+Ztring().From_Local(Avc_profile_idc(Profile))+__T("@")+Ztring().From_Number(((float)Level)/10, 1);

        MustParse_SPS_PPS=false;
        if (!Status[IsAccepted])
            Accept("AVC");
    FILLING_END();
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
std::string File_Avc::GOP_Detect (std::string PictureTypes)
{
    //Finding a string without blanks
    size_t PictureTypes_Limit=PictureTypes.find(' ');
    if (PictureTypes_Limit!=string::npos)
    {
        if (PictureTypes_Limit>PictureTypes.size()/2)
            PictureTypes.resize(PictureTypes_Limit);
        else
        {
            //Trim
            size_t TrimPos;
            TrimPos=PictureTypes.find_first_not_of(' ');
            if (TrimPos!=string::npos)
                PictureTypes.erase(0, TrimPos);
            TrimPos=PictureTypes.find_last_not_of(' ');
            if (TrimPos!=string::npos)
                PictureTypes.erase(TrimPos+1);

            //Finding the longest string
            ZtringList List; List.Separator_Set(0, __T(" "));
            List.Write(Ztring().From_Local(PictureTypes));
            size_t MaxLength=0;
            size_t MaxLength_Pos=0;
            for (size_t Pos=0; Pos<List.size(); Pos++)
                if (List[Pos].size()>MaxLength)
                {
                    MaxLength=List[Pos].size();
                    MaxLength_Pos=Pos;
                }
            PictureTypes=List[MaxLength_Pos].To_Local();

        }
    }

    //Creating all GOP values
    std::vector<Ztring> GOPs;
    size_t GOP_Frame_Count=0;
    size_t GOP_BFrames_Max=0;
    size_t I_Pos1=PictureTypes.find('I');
    while (I_Pos1!=std::string::npos)
    {
        size_t I_Pos2=PictureTypes.find('I', I_Pos1+1);
        if (I_Pos2!=std::string::npos)
        {
            std::vector<size_t> P_Positions;
            size_t P_Position=I_Pos1;
            do
            {
                P_Position=PictureTypes.find('P', P_Position+1);
                if (P_Position<I_Pos2)
                    P_Positions.push_back(P_Position);
            }
            while (P_Position<I_Pos2);
            if (P_Positions.size()>1 && P_Positions[0]>I_Pos1+1 && P_Positions[P_Positions.size()-1]==I_Pos2-1)
                P_Positions.resize(P_Positions.size()-1); //Removing last P-Frame for next test, this is often a terminating P-Frame replacing a B-Frame
            Ztring GOP;
            bool IsOK=true;
            if (!P_Positions.empty())
            {
                size_t Delta=P_Positions[0]-I_Pos1;
                for (size_t Pos=1; Pos<P_Positions.size(); Pos++)
                    if (P_Positions[Pos]-P_Positions[Pos-1]!=Delta)
                    {
                        IsOK=false;
                        break;
                    }
                if (IsOK)
                {
                    GOP+=__T("M=")+Ztring::ToZtring(P_Positions[0]-I_Pos1)+__T(", ");
                    if (P_Positions[0]-I_Pos1>GOP_BFrames_Max)
                        GOP_BFrames_Max=P_Positions[0]-I_Pos1;
                }
            }
            if (IsOK)
            {
                GOP+=__T("N=")+Ztring::ToZtring(I_Pos2-I_Pos1);
                GOPs.push_back(GOP);
            }
            else
                GOPs.push_back(Ztring()); //There is a problem, blank
            GOP_Frame_Count+=I_Pos2-I_Pos1;
        }
        I_Pos1=I_Pos2;
    }

    //Some clean up
    if (GOP_Frame_Count+GOP_BFrames_Max>Frame_Count && !GOPs.empty())
        GOPs.resize(GOPs.size()-1); //Removing the last one, there may have uncomplete B-frame filling
    if (GOPs.size()>4)
        GOPs.erase(GOPs.begin()); //Removing the first one, it is sometime different and we have enough to deal with

    //Filling
    if (GOPs.size()>=4)
    {
        bool IsOK=true;
        for (size_t Pos=1; Pos<GOPs.size(); Pos++)
            if (GOPs[Pos]!=GOPs[0])
            {
                IsOK=false;
                break;
            }
        if (IsOK)
            return GOPs[0].To_Local();
    }

    return string();
}

//---------------------------------------------------------------------------
std::string File_Avc::ScanOrder_Detect (std::string ScanOrders)
{
    //Finding a string without blanks
    size_t ScanOrders_Limit=ScanOrders.find(' ');
    if (ScanOrders_Limit!=string::npos)
    {
        if (ScanOrders_Limit>ScanOrders.size()/2)
            ScanOrders.resize(ScanOrders_Limit);
        else
        {
            //Trim
            size_t TrimPos;
            TrimPos=ScanOrders.find_first_not_of(' ');
            if (TrimPos!=string::npos)
                ScanOrders.erase(0, TrimPos);
            TrimPos=ScanOrders.find_last_not_of(' ');
            if (TrimPos!=string::npos)
                ScanOrders.erase(TrimPos+1);

            //Finding the longest string
            ZtringList List; List.Separator_Set(0, __T(" "));
            List.Write(Ztring().From_Local(ScanOrders));
            size_t MaxLength=0;
            size_t MaxLength_Pos=0;
            for (size_t Pos=0; Pos<List.size(); Pos++)
                if (List[Pos].size()>MaxLength)
                {
                    MaxLength=List[Pos].size();
                    MaxLength_Pos=Pos;
                }
            ScanOrders=List[MaxLength_Pos].To_Local();

        }
    }

    //Filling
    if (ScanOrders.find("TBTBTBTB")==0)
        return ("TFF");
    if (ScanOrders.find("BTBTBTBT")==0)
        return ("BFF");
    return string();
}

} //NameSpace

#endif //MEDIAINFO_AVC_YES
