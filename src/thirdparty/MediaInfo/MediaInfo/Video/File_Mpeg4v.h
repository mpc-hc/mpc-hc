/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG-4 Visual files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Mpeg4vH
#define MediaInfo_Mpeg4vH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg4v
//***************************************************************************

class File_Mpeg4v : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;
    void   OnlyVOP(); //Data has only VOPs in it (configuration is elsewhere)

    //Constructor/Destructor
    File_Mpeg4v();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin() {return FileHeader_Begin_0x000001();}

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //Buffer - Synchro
    bool Synchronize() {return Synchronize_0x000001();}
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    #endif //MEDIAINFO_DEMUX

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_QuickSearch();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Elements
    void video_object_start();
    void video_object_layer_start();
    void fgs_bp_start();
    void visual_object_sequence_start();
    void visual_object_sequence_end();
    void user_data_start();
    void user_data_start_SNC();
    void group_of_vop_start();
    void video_session_error();
    void visual_object_start();
    void vop_start();
    void slice_start();
    void extension_start();
    void fgs_vop_start();
    void fba_object_start();
    void fba_object_plane_start();
    void mesh_object_start();
    void mesh_object_plane_start();
    void still_texture_object_start();
    void texture_spatial_layer_start();
    void texture_snr_layer_start();
    void texture_tile_start();
    void texture_shape_layer_start();
    void stuffing_start();
    void reserved();

    //Streams
    struct stream
    {
        bool   Searching_Payload;

        stream()
        {
            Searching_Payload=false;
        }
    };
    std::vector<stream> Streams;

    //Count of a Packets
    size_t IVOP_Count;
    size_t PVOP_Count;
    size_t BVOP_Count;
    size_t BVOP_Count_Max;
    size_t SVOP_Count;
    size_t NVOP_Count;
    size_t Interlaced_Top;
    size_t Interlaced_Bottom;
    int64u Frame_Count_InThisBlock_Max;

    //From video_object_layer
    int32u fixed_vop_time_increment;
    int32u Time_Begin_Seconds;
    int32u Time_End_Seconds;
    int16u Time_Begin_MilliSeconds;
    int16u Time_End_MilliSeconds;
    int16u object_layer_width;
    int16u object_layer_height;
    int16u vop_time_increment_resolution;
    int8u  time_size;
    int8u  visual_object_verid;
    int8u  profile_and_level_indication;
    int8u  no_of_sprite_warping_points;
    int8u  aspect_ratio_info;
    int8u  par_width;
    int8u  par_height;
    int8u  bits_per_pixel;
    int8u  shape;
    int8u  sprite_enable;
    int8u  estimation_method;
    int8u  chroma_format;
    int8u  colour_primaries;
    int8u  transfer_characteristics;
    int8u  matrix_coefficients;
    bool   quarter_sample;
    bool   low_delay;
    bool   load_intra_quant_mat;
    bool   load_nonintra_quant_mat;
    bool   load_intra_quant_mat_grayscale;
    bool   load_nonintra_quant_mat_grayscale;
    bool   interlaced;
    bool   newpred_enable;
    bool   reduced_resolution_vop_enable;
    bool   scalability;
    bool   enhancement_type;
    bool   complexity_estimation_disable;
    bool   opaque;
    bool   transparent;
    bool   intra_cae;
    bool   inter_cae;
    bool   no_update;
    bool   upsampling;
    bool   intra_blocks;
    bool   inter_blocks;
    bool   inter4v_blocks;
    bool   not_coded_blocks;
    bool   dct_coefs;
    bool   dct_lines;
    bool   vlc_symbols;
    bool   vlc_bits;
    bool   apm;
    bool   npm;
    bool   interpolate_mc_q;
    bool   forw_back_mc_q;
    bool   halfpel2;
    bool   halfpel4;
    bool   sadct;
    bool   quarterpel;
    bool   video_object_layer_start_IsParsed;
    bool   quant_type;
    bool   data_partitioned;
    bool   reversible_vlc;
    bool   colour_description;

    //From user_data
    Ztring Library;
    Ztring Library_Name;
    Ztring Library_Version;
    Ztring Library_Date;
    Ztring Matrix_intra;
    Ztring Matrix_nonintra;
    ZtringListList user_data_start_SNC_Data;
};

} //NameSpace

#endif
