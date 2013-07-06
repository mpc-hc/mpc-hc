/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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

class File_Avc :
#if MEDIAINFO_DUPLICATE
    public File__Duplicate
#else //MEDIAINFO_DUPLICATE
    public File__Analyze
#endif //MEDIAINFO_DUPLICATE
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;
    bool   MustParse_SPS_PPS;
    bool   SizedBlocks;

    //Constructor/Destructor
    File_Avc();
    ~File_Avc();

private :
    File_Avc(const File_Avc &File_Avc); //No copy

    //Structures - seq_parameter_set
    struct seq_parameter_set_struct
    {
        struct vui_parameters_struct
        {
            struct xxl
            {
                struct xxl_data
                {
                    //HRD configuration
                    int32u bit_rate_value;
                    int32u cpb_size_value;
                    bool   cbr_flag;

                    //sei_message_buffering_period
                    int32u initial_cpb_removal_delay;
                    int32u initial_cpb_removal_delay_offset;

                    xxl_data()
                    {
                        //HRD configuration
                        bit_rate_value=(int32u)-1;
                        cpb_size_value=(int32u)-1;
                        cbr_flag=true;

                        //sei_message_buffering_period
                        initial_cpb_removal_delay=(int32u)-1;
                        initial_cpb_removal_delay_offset=(int32u)-1;
                    }
                };
                vector<xxl_data> SchedSel;
                int8u   initial_cpb_removal_delay_length_minus1;
                int8u   cpb_removal_delay_length_minus1;
                int8u   dpb_output_delay_length_minus1;
                int8u   time_offset_length;
            };
            struct bitstream_restriction_struct
            {
                int8u   num_reorder_frames;
            };
            xxl*    NAL;
            xxl*    VCL;
            bitstream_restriction_struct* bitstream_restriction;
            int32u  num_units_in_tick;
            int32u  time_scale;
            int16u  sar_width;
            int16u  sar_height;
            int8u   aspect_ratio_idc;
            int8u   video_format;
            int8u   colour_primaries;
            int8u   transfer_characteristics;
            int8u   matrix_coefficients;
            bool    aspect_ratio_info_present_flag;
            bool    video_signal_type_present_flag;
            bool    colour_description_present_flag;
            bool    timing_info_present_flag;
            bool    fixed_frame_rate_flag;
            bool    pic_struct_present_flag;

            vui_parameters_struct()
            {
                NAL=NULL;
                VCL=NULL;
                bitstream_restriction=NULL;
                aspect_ratio_info_present_flag=false;
                video_signal_type_present_flag=false;
                colour_description_present_flag=false;
                timing_info_present_flag=false;
                fixed_frame_rate_flag=false;
                pic_struct_present_flag=false;
            }

            ~vui_parameters_struct()
            {
                delete NAL; //NAL=NULL;
                delete VCL; //VCL=NULL;
                delete bitstream_restriction; //bitstream_restriction=NULL;
            }
        };
        vui_parameters_struct* vui_parameters;
        int32u  pic_width_in_mbs_minus1;
        int32u  pic_height_in_map_units_minus1;
        int32u  frame_crop_left_offset;
        int32u  frame_crop_right_offset;
        int32u  frame_crop_top_offset;
        int32u  frame_crop_bottom_offset;
        int32u  MaxPicOrderCntLsb; //Computed value (for speed)
        int32u  MaxFrameNum; //Computed value (for speed)
        int16u  num_views_minus1; //MultiView specific field
        int8u   chroma_format_idc;
        int8u   profile_idc;
        int8u   level_idc;
        int8u   bit_depth_luma_minus8;
        int8u   bit_depth_chroma_minus8;
        int8u   log2_max_frame_num_minus4;
        int8u   pic_order_cnt_type;
        int8u   log2_max_pic_order_cnt_lsb_minus4;
        int8u   max_num_ref_frames;
        int8u   pic_struct_FirstDetected; //For stats only
        int8u   log2_max_slice_group_change_cycle_minus4;
        bool    constraint_set3_flag;
        bool    separate_colour_plane_flag;
        bool    delta_pic_order_always_zero_flag;
        bool    frame_mbs_only_flag;
        bool    mb_adaptive_frame_field_flag;
        bool    IsSynched; //Computed value

        //Computed values
        bool    NalHrdBpPresentFlag() {return vui_parameters && vui_parameters->NAL;}
        bool    VclHrdBpPresentFlag() {return vui_parameters && vui_parameters->VCL;}
        bool    CpbDpbDelaysPresentFlag() {return vui_parameters && (vui_parameters->NAL || vui_parameters->VCL);}
        int8u   ChromaArrayType() {return separate_colour_plane_flag?0:chroma_format_idc;}

        #if MEDIAINFO_DEMUX
        int8u*  Iso14496_10_Buffer;
        size_t  Iso14496_10_Buffer_Size;
        #endif //MEDIAINFO_DEMUX

        //Constructor/Destructor
        #if MEDIAINFO_DEMUX
        seq_parameter_set_struct()
        {
            Iso14496_10_Buffer=NULL;
            Iso14496_10_Buffer_Size=0;
        }
        #endif //MEDIAINFO_DEMUX
        ~seq_parameter_set_struct()
        {
            delete vui_parameters; //vui_parameters=NULL;
            #if MEDIAINFO_DEMUX
                delete[] Iso14496_10_Buffer;
            #endif //MEDIAINFO_DEMUX
        }
    };
    typedef vector<seq_parameter_set_struct*> seq_parameter_set_structs;

    //Structures - pic_parameter_set
    struct pic_parameter_set_struct
    {
        int8u   seq_parameter_set_id;
        int8u   num_ref_idx_l0_default_active_minus1;
        int8u   num_ref_idx_l1_default_active_minus1;
        int8u   weighted_bipred_idc;
        int32u  num_slice_groups_minus1;
        int32u  slice_group_map_type;
        bool    entropy_coding_mode_flag;
        bool    bottom_field_pic_order_in_frame_present_flag;
        bool    weighted_pred_flag;
        bool    redundant_pic_cnt_present_flag;
        bool    IsSynched; //Computed value
        bool    deblocking_filter_control_present_flag;

        #if MEDIAINFO_DEMUX
        int8u*  Iso14496_10_Buffer;
        size_t  Iso14496_10_Buffer_Size;
        #endif //MEDIAINFO_DEMUX

        //Constructor/Destructor
        #if MEDIAINFO_DEMUX
        pic_parameter_set_struct()
        {
            Iso14496_10_Buffer=NULL;
            Iso14496_10_Buffer_Size=0;
        }
        #endif //MEDIAINFO_DEMUX
        ~pic_parameter_set_struct()
        {
            #if MEDIAINFO_DEMUX
                delete[] Iso14496_10_Buffer;
            #endif //MEDIAINFO_DEMUX
        }
    };
    typedef vector<pic_parameter_set_struct*> pic_parameter_set_structs;

    //Streams management
    void Streams_Fill();
    void Streams_Fill(vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item);
    void Streams_Fill_subset(vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item);
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    bool Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10;
    #endif //MEDIAINFO_DEMUX

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_QuickSearch();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    #if MEDIAINFO_DUPLICATE
        //Output buffer
        size_t Output_Buffer_Get (const String &Value);
        size_t Output_Buffer_Get (size_t Pos);
    #endif //MEDIAINFO_DUPLICATE

    //Options
    void Option_Manage ();

    //Elements
    void slice_layer_without_partitioning_IDR();
    void slice_layer_without_partitioning_non_IDR();
    void slice_header();
    void slice_data (bool AllCategories);
    void seq_parameter_set();
    void pic_parameter_set();
    void sei();
    void sei_message(int32u &seq_parameter_set_id);
    void sei_message_buffering_period(int32u &seq_parameter_set_id);
    void sei_message_buffering_period_xxl(void* xxl);
    void sei_message_pic_timing(int32u payloadSize, int32u seq_parameter_set_id);
    void sei_message_user_data_registered_itu_t_t35();
    void sei_message_user_data_registered_itu_t_t35_DTG1();
    void sei_message_user_data_registered_itu_t_t35_GA94();
    void sei_message_user_data_registered_itu_t_t35_GA94_03();
    void sei_message_user_data_registered_itu_t_t35_GA94_03_Delayed(int32u seq_parameter_set_id);
    void sei_message_user_data_registered_itu_t_t35_GA94_06();
    void sei_message_user_data_unregistered(int32u payloadSize);
    void sei_message_user_data_unregistered_x264(int32u payloadSize);
    void sei_message_recovery_point();
    void sei_message_mainconcept(int32u payloadSize);
    void access_unit_delimiter();
    void filler_data();
    void prefix_nal_unit(bool svc_extension_flag);
    void subset_seq_parameter_set();
    void slice_layer_extension(bool svc_extension_flag);

    //Packets - SubElements
    bool seq_parameter_set_data(vector<seq_parameter_set_struct*> &Data, int32u &Data_id);
    void seq_parameter_set_svc_extension();
    void seq_parameter_set_mvc_extension(int32u subset_seq_parameter_sets_id);
    void scaling_list(int32u ScalingList_Size);
    void vui_parameters(void* &vui_parameters_Item);
    void svc_vui_parameters_extension();
    void mvc_vui_parameters_extension();
    void hrd_parameters(void* &hrd_parameters_Item);
    void nal_unit_header_svc_extension();
    void nal_unit_header_mvc_extension();
    void ref_pic_list_modification(int32u slice_type, bool mvc);
    void pred_weight_table(int32u num_ref_idx_l0_active_minus1, int32u num_ref_idx_l1_active_minus1, int8u ChromaArrayType);
    void dec_ref_pic_marking(vector<int8u> &memory_management_control_operations);

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
    vector<stream> Streams;

    //Temporal references
    struct temporal_reference
    {
        struct buffer_data
        {
            size_t Size;
            int8u* Data;

            buffer_data()
            {
                Size=0;
                Data=NULL;
            }

            ~buffer_data()
            {
                delete[] Data; //Data=NULL;
            }
        };
        #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
            buffer_data* GA94_03;
        #endif //MEDIAINFO_DTVCCTRANSPORT_YES

        int32u frame_num;
        int8u  slice_type;
        bool   IsTop;
        bool   IsField;

        temporal_reference()
        {
            #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
                GA94_03=NULL;
            #endif //MEDIAINFO_DTVCCTRANSPORT_YES
            slice_type=(int8u)-1;
        }

        ~temporal_reference()
        {
            #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
                delete GA94_03; //GA94_03=NULL;
            #endif //MEDIAINFO_DTVCCTRANSPORT_YES
        }
    };
    typedef vector<temporal_reference*> temporal_references;
    temporal_references                 TemporalReferences; //per pic_order_cnt_lsb
    temporal_reference*                 TemporalReferences_DelayedElement;
    size_t                              TemporalReferences_Min;
    size_t                              TemporalReferences_Max;
    size_t                              TemporalReferences_Reserved;
    size_t                              TemporalReferences_Offset;
    size_t                              TemporalReferences_Offset_pic_order_cnt_lsb_Last;
    int64s                              TemporalReferences_pic_order_cnt_Min;

    //Text
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        File__Analyze*                  GA94_03_Parser;
        bool                            GA94_03_IsPresent;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)

    //Replacement of File__Analyze buffer
    const int8u*                        Buffer_ToSave;
    size_t                              Buffer_Size_ToSave;

    //parameter_sets
    seq_parameter_set_structs           seq_parameter_sets;
    seq_parameter_set_structs           subset_seq_parameter_sets;
    pic_parameter_set_structs           pic_parameter_sets;

    //File specific
    int8u                               SizeOfNALU_Minus1;

    //Status
    size_t                              IFrame_Count;
    int32s                              prevPicOrderCntMsb;
    int32u                              prevPicOrderCntLsb;
    int32u                              prevTopFieldOrderCnt;
    int32u                              prevFrameNum;
    int32u                              prevFrameNumOffset;
    vector<int8u>                       prevMemoryManagementControlOperations;

    //Count of a Packets
    size_t                              Block_Count;
    size_t                              Interlaced_Top;
    size_t                              Interlaced_Bottom;
    size_t                              Structure_Field;
    size_t                              Structure_Frame;

    //Temp
    Ztring                              Encoded_Library;
    Ztring                              Encoded_Library_Name;
    Ztring                              Encoded_Library_Version;
    Ztring                              Encoded_Library_Date;
    Ztring                              Encoded_Library_Settings;
    Ztring                              BitRate_Nominal;
    Ztring                              MuxingMode;
    string                              PictureTypes_PreviousFrames;
    int64u                              tc;
    int32u                              Firstpic_order_cnt_lsbInBlock;
    int8u                               nal_ref_idc;
    int8u                               FrameRate_Divider;
    bool                                FirstPFrameInGop_IsParsed;

    //Helpers
    string                              GOP_Detect                              (string PictureTypes);
    string                              ScanOrder_Detect                        (string ScanOrders);

    #if MEDIAINFO_DUPLICATE
        bool   File__Duplicate_Set  (const Ztring &Value); //Fill a new File__Duplicate value
        void   File__Duplicate_Write (int64u Element_Code, int32u frame_num=(int32u)-1);
        File__Duplicate__Writer Writer;
        int8u  Duplicate_Buffer[1024*1024];
        size_t Duplicate_Buffer_Size;
        size_t frame_num_Old;
        bool   SPS_PPS_AlreadyDone;
        bool   FLV;
    #endif //MEDIAINFO_DUPLICATE
};

} //NameSpace

#endif
