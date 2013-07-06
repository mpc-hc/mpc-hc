/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_HevcH
#define MediaInfo_HevcH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Hevc
//***************************************************************************

class File_Hevc : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;
    bool   MustParse_VPS_SPS_PPS;
    bool   MustParse_VPS_SPS_PPS_FromMatroska;
    bool   SizedBlocks;

    //Config
    int8u                               lengthSizeMinusOne;

    //Constructor/Destructor
    File_Hevc();
    ~File_Hevc();

private :
    File_Hevc(const File_Hevc &File_Hevc); //No copy

    //Structures - video_parameter_set
    struct video_parameter_set_struct
    {
        bool    IsSynched; //Computed value
    };
    typedef vector<video_parameter_set_struct*> video_parameter_set_structs;

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
            xxl*    NAL;
            xxl*    VCL;
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
            }
        };
        vui_parameters_struct* vui_parameters;
        int32u  profile_space;
        int32u  profile_idc;
        int32u  level_idc;
        int32u  pic_width_in_luma_samples;
        int32u  pic_height_in_luma_samples;
        int8u   video_parameter_set_id;
        int8u   chroma_format_idc;
        int8u   log2_max_pic_order_cnt_lsb_minus4;
        int8u   bit_depth_luma_minus8;
        int8u   bit_depth_chroma_minus8;
        bool    general_progressive_source_flag;
        bool    general_interlaced_source_flag;
        bool    general_frame_only_constraint_flag;
        bool    IsSynched; //Computed value
    };
    typedef vector<seq_parameter_set_struct*> seq_parameter_set_structs;

    //Structures - pic_parameter_set
    struct pic_parameter_set_struct
    {
        int8u   seq_parameter_set_id;
        int8u   num_ref_idx_l0_default_active_minus1;
        int8u   num_ref_idx_l1_default_active_minus1;
        int8u   num_extra_slice_header_bits;
        bool    dependent_slice_segments_enabled_flag;
        bool    IsSynched; //Computed value
    };
    typedef vector<pic_parameter_set_struct*> pic_parameter_set_structs;

    //Streams management
    void Streams_Fill();
    void Streams_Fill(vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item);
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
    #endif //MEDIAINFO_DEMUX

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_QuickSearch();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Elements
    void slice_segment_layer();
    void slice_layer();
    void video_parameter_set();
    void seq_parameter_set();
    void pic_parameter_set();
    void access_unit_delimiter();
    void end_of_seq();
    void end_of_bitstream();
    void filler_data();
    void sei();
    void sei_message();
    void sei_message_decoded_picture_hash(int32u payloadSize);

    //Packets - SubElements
    void slice_segment_header();
    void profile_tier_level(int8u maxNumSubLayersMinus1);
    void short_term_ref_pic_sets(int8u num_short_term_ref_pic_sets);
    void vui_parameters(void* &vui_parameters_Item);
    void hrd_parameters(void* &hrd_parameters_Item);

    //Packets - Specific
    void VPS_SPS_PPS();
    void VPS_SPS_PPS_FromMatroska();

    //Streams
    struct stream
    {
        bool   Searching_Payload;

        stream()
        {
            Searching_Payload=false;
        }
    };
    vector<stream> Streams;

    //Replacement of File__Analyze buffer
    const int8u*                        Buffer_ToSave;
    size_t                              Buffer_Size_ToSave;

    //parameter_sets
    video_parameter_set_structs         video_parameter_sets;
    seq_parameter_set_structs           seq_parameter_sets;
    pic_parameter_set_structs           pic_parameter_sets;

    //File specific
    size_t                              IFrame_Count;

    //Temp
    int32u  chroma_format_idc;
    int32u  slice_pic_parameter_set_id;
    int32u  slice_type;
    int8u   nuh_layer_id;
    int8u   profile_space;
    int8u   profile_idc;
    int8u   level_idc;
    bool    general_progressive_source_flag;
    bool    general_interlaced_source_flag;
    bool    general_frame_only_constraint_flag;
    bool    RapPicFlag;
    bool    first_slice_segment_in_pic_flag;
};

} //NameSpace

#endif
