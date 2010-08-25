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
#ifndef MediaInfo_AvcH
#define MediaInfo_AvcH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/File__Duplicate.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Avc
//***************************************************************************

class File_Avc : public File__Duplicate
{
public :
    //In
    size_t Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;
    bool   MustParse_SPS_PPS;
    bool   MustParse_SPS_PPS_Only;
    bool   MustParse_SPS_PPS_Done;
    bool   SizedBlocks;

    //Constructor/Destructor
    File_Avc();
    ~File_Avc();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize() {return Synchronize_0x000001();}
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_QuickSearch();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Output buffer
    size_t Output_Buffer_Get (const String &Value);
    size_t Output_Buffer_Get (size_t Pos);

    //Options
    void Option_Manage ();

    //Elements
    void slice_layer_without_partitioning_IDR();
    void slice_layer_without_partitioning_non_IDR();
    void slice_header();
    void seq_parameter_set();
    void pic_parameter_set();
    void sei();
    void sei_message();
    void sei_message_buffering_period(int32u payloadSize);
    void sei_message_pic_timing(int32u payloadSize);
    void sei_message_user_data_registered_itu_t_t35(int32u payloadSize);
    void sei_message_user_data_registered_itu_t_t35_DTG1();
    void sei_message_user_data_registered_itu_t_t35_GA94();
    void sei_message_user_data_registered_itu_t_t35_GA94_03();
    void sei_message_user_data_registered_itu_t_t35_GA94_06();
    void sei_message_user_data_unregistered(int32u payloadSize);
    void sei_message_user_data_unregistered_x264(int32u payloadSize);
    void sei_message_recovery_point(int32u payloadSize);
    void sei_message_mainconcept(int32u payloadSize);
    void access_unit_delimiter();
    void filler_data();
    void prefix_nal_unit();
    void subset_seq_parameter_set();
    void slice_layer_extension();

    //Packets - SubElements
    void seq_parameter_set_data();
    void scaling_list(int32u ScalingList_Size);
    void vui_parameters();
    void hrd_parameters(bool vcl);
    void nal_unit_header_svc_extension();
    void nal_unit_header_mvc_extension();
    void seq_parameter_set_svc_extension();
    void svc_vui_parameters_extension();
    void seq_parameter_set_mvc_extension();
    void mvc_vui_parameters_extension();

    //Packets - Specific
    void SPS_PPS();

    //Streams
    struct stream
    {
        bool   Searching_Payload;
        bool   ShouldDuplicate;

        stream()
        {
            Searching_Payload=false;
            ShouldDuplicate=false;
        }
    };
    std::vector<stream> Streams;

    //Temporal reference
    struct temporalreference
    {
        struct cc_data_
        {
            int8u cc_type;
            int8u cc_data[2];
            bool  cc_valid;

            cc_data_()
            {
                cc_valid=false;
            }
        };
        std::vector<cc_data_> GA94_03_CC; //Per cc offset

        bool   IsValid;

        int32u frame_num;
        int8u  slice_type;
        bool   IsTop;
        bool   IsField;

        temporalreference()
        {
            slice_type=(int8u)-1;
            IsValid=false;
        }
    };
    std::vector<temporalreference> TemporalReference; //per pic_order_cnt_lsb
    temporalreference              TemporalReference_Temp;
    size_t                         TemporalReference_Offset;
    bool                           TemporalReference_Offset_Moved;
    size_t                         TemporalReference_GA94_03_CC_Offset;
    size_t                         TemporalReference_Offset_pic_order_cnt_lsb_Last;

    //seq_parameter_set
    struct seq_parameter_set_
    {
        int8u profile_idc;
        int8u level_idc;

        seq_parameter_set_()
        {
            profile_idc=0;
            level_idc=0;
        }
    };
    std::map<int32u, seq_parameter_set_> seq_parameter_set_ids;
    std::map<int32u, seq_parameter_set_> subset_seq_parameter_set_ids;

    //Temp
    std::vector<File__Analyze*> GA94_03_CC_Parsers;

    //Replacement of File__Analyze
    const int8u* Buffer_ToSave;
    size_t Buffer_Size_ToSave;

    //Count of a Packets
    size_t Frame_Count;
    size_t Block_Count;
    size_t Interlaced_Top;
    size_t Interlaced_Bottom;
    size_t Structure_Field;
    size_t Structure_Frame;
    int8u  FrameRate_Divider;

    //From seq_parameter_set
    struct xxl
    {
        int32u bit_rate_value;
        int32u cpb_size_value;
        bool   cbr_flag;
    };
    std::vector<xxl> NAL;
    std::vector<xxl> VCL;
    Ztring Encoded_Library;
    Ztring Encoded_Library_Name;
    Ztring Encoded_Library_Version;
    Ztring Encoded_Library_Date;
    Ztring Encoded_Library_Settings;
    Ztring BitRate_Nominal;
    Ztring MuxingMode;
    int32u pic_width_in_mbs_minus1;
    int32u pic_height_in_map_units_minus1;
    int32u log2_max_frame_num_minus4;
    int32u log2_max_pic_order_cnt_lsb_minus4;
    int32u num_units_in_tick;
    int32u time_scale;
    int32u chroma_format_idc;
    int32u frame_crop_left_offset;
    int32u frame_crop_right_offset;
    int32u frame_crop_top_offset;
    int32u frame_crop_bottom_offset;
    int32u max_num_ref_frames;
    int32u pic_order_cnt_type;
    int32u bit_depth_luma_minus8;
    int32u bit_depth_chroma_minus8;
    int32u pic_order_cnt_lsb;
    int32u pic_order_cnt_lsb_Last;
    int32u seq_parameter_set_id;
    int32u num_views_minus1;
    int32u cpb_cnt_minus1;
    int32u initial_cpb_removal_delay;
    int32u initial_cpb_removal_delay_offset;
    int32u cpb_removal_delay;
    int32u pic_order_cnt_lsb_Old;
    int16u sar_width;
    int16u sar_height;
    int8u  profile_idc;
    int8u  level_idc;
    int8u  aspect_ratio_idc;
    int8u  video_format;
    int8u  initial_cpb_removal_delay_length_minus1;
    int8u  cpb_removal_delay_length_minus1;
    int8u  dpb_output_delay_length_minus1;
    int8u  time_offset_length;
    int8u  pic_struct;
    int8u  pic_struct_FirstDetected;
    int8u  SizeOfNALU_Minus1;
    int8u  colour_primaries;
    int8u  transfer_characteristics;
    int8u  matrix_coefficients;
    int8u  nal_unit_type;
    bool   GA94_03_CC_IsPresent;
    bool   frame_mbs_only_flag;
    bool   timing_info_present_flag;
    bool   fixed_frame_rate_flag;
    bool   pic_struct_present_flag;
    bool   field_pic_flag;
    bool   entropy_coding_mode_flag;
    bool   NalHrdBpPresentFlag;
    bool   VclHrdBpPresentFlag;
    bool   CpbDpbDelaysPresentFlag;
    bool   mb_adaptive_frame_field_flag;
    bool   pic_order_present_flag;
    bool   svc_extension_flag;
    bool   field_pic_flag_AlreadyDetected;

    //Temp
    bool SPS_IsParsed;
    bool PPS_IsParsed;

    //File__Duplicate
    bool   File__Duplicate_Set  (const Ztring &Value); //Fill a new File__Duplicate value
    void   File__Duplicate_Write (int64u Element_Code, int32u frame_num=(int32u)-1);
    File__Duplicate__Writer Writer;
    int8u  Duplicate_Buffer[1024*1024];
    size_t Duplicate_Buffer_Size;
    size_t frame_num_Old;
    bool   SPS_PPS_AlreadyDone;
    bool   FLV;
};

} //NameSpace

#endif
