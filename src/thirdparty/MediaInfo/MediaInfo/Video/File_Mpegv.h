/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG Video files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_MpegvH
#define MediaInfo_MpegvH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include <MediaInfo/Multiple/File_Ancillary.h>
#endif //defined(MEDIAINFO_ANCILLARY_YES)
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpegv
//***************************************************************************

class File_Mpegv : public File__Analyze
{
public :
    //In
    int8u  MPEG_Version;
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;
    bool   TimeCodeIsNotTrustable;
    #if defined(MEDIAINFO_ANCILLARY_YES)
        File_Ancillary** Ancillary;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)
    #if MEDIAINFO_ADVANCED
        bool    InitDataNotRepeated_Optional;
    #endif // MEDIAINFO_ADVANCED

    //Constructor/Destructor
    File_Mpegv();
    ~File_Mpegv();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Update();
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin() {return FileHeader_Begin_0x000001();}

    //Buffer - Synchro
    bool Synchronize() {return Synchronize_0x000001();}
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

    //EOF
    void Detect_EOF();

    //Elements
    void picture_start();
    void slice_start();
    #if MEDIAINFO_MACROBLOCKS
    void slice_start_macroblock();
    void slice_start_macroblock_motion_vectors(bool s);
    void slice_start_macroblock_motion_vectors_motion_vector(bool r, bool s);
    void slice_start_macroblock_coded_block_pattern();
    void slice_start_macroblock_block(int8u i);
    #endif //MEDIAINFO_MACROBLOCKS
    void user_data_start();
    void user_data_start_3();
    void user_data_start_CC();
    void user_data_start_DTG1();
    void user_data_start_GA94();
    void user_data_start_GA94_03();
    void user_data_start_GA94_06();
    void sequence_header();
    void sequence_error();
    void extension_start();
    void sequence_end();
    void group_start();


    //Helpers
    void temporal_reference_Adapt();

    //Streams
    struct stream
    {
        bool   Searching_Payload;
        bool   Searching_TimeStamp_Start;
        bool   Searching_TimeStamp_End;

        stream()
        {
            Searching_Payload=false;
            Searching_TimeStamp_Start=false;
            Searching_TimeStamp_End=false;
        }
    };
    std::vector<stream> Streams;

    //Temporal reference
    struct temporalreference
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
        #if defined(MEDIAINFO_SCTE20_YES)
            std::vector<buffer_data*> Scte;
            std::vector<bool> Scte_Parsed;
        #endif //MEDIAINFO_SCTE20_YES

        int8u  picture_coding_type;
        int8u  picture_structure;

        bool   IsValid;
        bool   HasPictureCoding;

        bool   progressive_frame;
        bool   top_field_first;
        bool   repeat_first_field;

        temporalreference()
        {
            #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
                GA94_03=NULL;
            #endif //MEDIAINFO_DTVCCTRANSPORT_YES
            picture_coding_type=(int8u)-1;
            picture_structure=(int8u)-1;
            IsValid=false;
            HasPictureCoding=false;
        }

        ~temporalreference()
        {
            #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
                delete GA94_03; //GA94_03=NULL;
            #endif //MEDIAINFO_DTVCCTRANSPORT_YES
            #if defined(MEDIAINFO_SCTE20_YES)
                for (size_t Pos=0; Pos<Scte.size(); Pos++)
                    delete Scte[Pos]; //Scte[Pos]=NULL;
            #endif //MEDIAINFO_SCTE20_YES
        }
    };
    std::vector<temporalreference*> TemporalReference; //per temporal_reference
    size_t                          TemporalReference_Offset;
    struct text_position
    {
        File__Analyze**  Parser;
        size_t          StreamPos;

        text_position()
        {
            Parser=NULL;
            StreamPos=(size_t)-1;
        }

        text_position(File__Analyze* &Parser_)
        {
            Parser=&Parser_;
            StreamPos=0;
        }
    };
    std::vector<text_position> Text_Positions;
    #if defined(MEDIAINFO_DTVCCTRANSPORT_YES)
        File__Analyze*              GA94_03_Parser;
        size_t                      GA94_03_TemporalReference_Offset;
        bool                        GA94_03_IsPresent;
        File__Analyze*              CC___Parser;
        bool                        CC___IsPresent;
    #endif //defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #if defined(MEDIAINFO_SCTE20_YES)
        File__Analyze*              Scte_Parser;
        size_t                      Scte_TemporalReference_Offset;
        bool                        Scte_IsPresent;
    #endif //defined(MEDIAINFO_SCTE20_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        File__Analyze*              DTG1_Parser;
        File__Analyze*              GA94_06_Parser;
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_CDP_YES)
        File__Analyze*              Cdp_Parser;
        bool                        Cdp_IsPresent;
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        File__Analyze*              AfdBarData_Parser;
    #endif //defined(MEDIAINFO_GXF_YES) && defined(MEDIAINFO_AFDBARDATA_YES)

    //Temp
    Ztring Library;
    Ztring Library_Name;
    Ztring Library_Version;
    Ztring Matrix_intra;
    Ztring Matrix_nonintra;
    string TimeCode_FirstFrame;
    size_t BVOP_Count;
    size_t progressive_frame_Count;
    size_t Interlaced_Top;
    size_t Interlaced_Bottom;
    size_t PictureStructure_Field;
    size_t PictureStructure_Frame;
    size_t Time_Current_Seconds;
    size_t Time_Begin_Seconds;
    size_t Time_End_Seconds;
    int64u SizeToAnalyse_Begin; //Total size of a chunk to analyse, it may be changed by the parser
    int64u SizeToAnalyse_End; //Total size of a chunk to analyse, it may be changed by the parser
    int64u Frame_Count_LastIFrame;
    int32u bit_rate_value;
    float32 FrameRate;
    int16u horizontal_size_value;
    int16u vertical_size_value;
    int16u bit_rate_extension;
    int16u temporal_reference;
    int16u temporal_reference_Old;
    int16u temporal_reference_Max;
    int16u display_horizontal_size;
    int16u display_vertical_size;
    int16u vbv_delay;
    int16u vbv_buffer_size_value;
    int8u  Time_Current_Frames;
    int8u  Time_Begin_Frames;
    int8u  Time_End_Frames;
    int8u  picture_coding_type;
    int8u  aspect_ratio_information;
    int8u  frame_rate_code;
    int8u  profile_and_level_indication;
    int8u  profile_and_level_indication_profile;
    int8u  profile_and_level_indication_level;
    int8u  chroma_format;
    int8u  horizontal_size_extension;
    int8u  vertical_size_extension;
    int8u  frame_rate_extension_n;
    int8u  frame_rate_extension_d;
    int8u  video_format;
    int8u  colour_primaries;
    int8u  transfer_characteristics;
    int8u  matrix_coefficients;
    int8u  picture_structure;
    int8u  vbv_buffer_size_extension;
    int8u  intra_dc_precision;
    bool   load_intra_quantiser_matrix;
    bool   load_non_intra_quantiser_matrix;
    bool   progressive_sequence;
    bool   progressive_frame;
    bool   top_field_first;
    bool   repeat_first_field;
    bool   FirstFieldFound;
    bool   sequence_header_IsParsed;
    bool   group_start_IsParsed;
    bool   group_start_FirstPass;
    bool   group_start_drop_frame_flag;
    bool   group_start_closed_gop;
    size_t group_start_closed_gop_Closed;
    size_t group_start_closed_gop_Open;
    bool   group_start_broken_link;
    bool   Searching_TimeStamp_Start_DoneOneTime;
    bool   Parsing_End_ForDTS;
    bool   bit_rate_value_IsValid;
    bool   profile_and_level_indication_escape;
    bool   colour_description;
    bool   low_delay;
    int8u  RefFramesCount;
    int8u  BVOPsSinceLastRefFrames;
    int16u temporal_reference_LastIFrame;
    int64u PTS_LastIFrame;
    int16u PTS_End_temporal_reference;
    int64u tc;
    bool    IFrame_IsParsed;
    size_t  IFrame_Count;
    std::map<std::string, int64u>   picture_coding_types; //per picture_coding_type value ("IPBB..."), updated at each I-frame
    std::string                     picture_coding_types_Current; //Current picture_coding_type value ("IPBB..."), updated at each frame

    #if MEDIAINFO_MACROBLOCKS
        int64u  macroblock_x;
        int64u  macroblock_x_PerFrame;
        int64u  macroblock_y_PerFrame;
        int16u  cbp;
        int8u   frame_motion_type;
        int8u   field_motion_type;
        int8u   spatial_temporal_weight_code;
        int8u   block_count; //Computed from chroma_format
        int8u   macroblock_type; //Temp
        int8u   spatial_temporal_weight_code_table_index;
        int8u   f_code[2][2];
        bool    Macroblocks_Parse;
        bool    sequence_scalable_extension_Present;
        bool    frame_pred_frame_dct;
        bool    concealment_motion_vectors;
        bool    intra_vlc_format;
        vlc_fast macroblock_address_increment_Vlc;
        vlc_fast dct_dc_size_luminance;
        vlc_fast dct_dc_size_chrominance;
        vlc_fast dct_coefficients_0;
        vlc_fast dct_coefficients_1;
        vlc_fast macroblock_type_I;
        vlc_fast macroblock_type_P;
        vlc_fast macroblock_type_B;
        vlc_fast motion_code;
        vlc_fast dmvector;
        vlc_fast coded_block_pattern;
    #endif //MEDIAINFO_MACROBLOCKS

    #if MEDIAINFO_IBI
        bool    Ibi_SliceParsed;
    #endif //MEDIAINFO_IBI

    #if MEDIAINFO_ADVANCED
        int64u  InitDataNotRepeated;
        int64u  Config_InitDataNotRepeated_Occurences;
        bool    Config_InitDataNotRepeated_GiveUp;
        int64u  Config_VariableGopDetection_Occurences;
        bool    Config_VariableGopDetection_GiveUp;
    #endif // MEDIAINFO_ADVANCED

    #if MEDIAINFO_ADVANCED || MEDIAINFO_EVENTS
        size_t  Slices_Count;
        bool    Has_sequence_header;
        bool    Has_sequence_extension;
    #endif // MEDIAINFO_ADVANCED || MEDIAINFO_EVENTS
};

} //NameSpace

#endif
