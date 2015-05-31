/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
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

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_HEVC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Hevc.h"
#include <cmath>
#include <algorithm>
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
const int8u Hevc_SubWidthC[]=
{
    1,
    2,
    2,
    1,
};

//---------------------------------------------------------------------------
const int8u Hevc_SubHeightC[]=
{
    1,
    2,
    1,
    1,
};

//---------------------------------------------------------------------------
const char* Hevc_tier_flag(bool tier_flag)
{
    switch (tier_flag)
    {
        case   true  : return "High";
        default      : return "Main";
    }
}

//---------------------------------------------------------------------------
const char* Hevc_profile_idc(int32u profile_idc)
{
    switch (profile_idc)
    {
        case   0 : return "No profile";
        case   1 : return "Main";
        case   2 : return "Main 10";
        case   3 : return "Main Still";
        default  : return "";
    }
}

//---------------------------------------------------------------------------
const char* Hevc_chroma_format_idc(int8u chroma_format_idc)
{
    switch (chroma_format_idc)
    {
        case   0 : return "monochrome";
        case   1 : return "4:2:0";
        case   2 : return "4:2:2";
        case   3 : return "4:4:4";
        default  : return "Unknown";
    }
}

//---------------------------------------------------------------------------
const char* Hevc_pic_type[]=
{
    "I",
    "I, P",
    "I, P, B",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Hevc_slice_type(int32u slice_type)
{
    switch (slice_type)
    {
        case 0 : return "P";
        case 1 : return "B";
        case 2 : return "I";
        default: return "";
    }
};

//---------------------------------------------------------------------------
extern const char* Mpegv_colour_primaries(int8u colour_primaries);
extern const char* Mpegv_transfer_characteristics(int8u transfer_characteristics);
extern const char* Mpegv_matrix_coefficients(int8u matrix_coefficients);

//---------------------------------------------------------------------------
extern const int8u Avc_PixelAspectRatio_Size;
extern const float32 Avc_PixelAspectRatio[];
extern const char* Avc_video_format[];
extern const char* Avc_video_full_range[];

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Hevc::File_Hevc()
{
    //Config
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Hevc;
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
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.3?16:16; //Note: should be replaced by "512:2" when I-frame/GOP detection is OK
    FrameIsAlwaysComplete=false;
    MustParse_VPS_SPS_PPS=false;
    MustParse_VPS_SPS_PPS_FromMatroska=false;
    MustParse_VPS_SPS_PPS_FromFlv=false;
    SizedBlocks=false;

    //File specific
    lengthSizeMinusOne=(int8u)-1;
}

//---------------------------------------------------------------------------
File_Hevc::~File_Hevc()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Hevc::Streams_Fill()
{
    if (MustParse_VPS_SPS_PPS_FromFlv)
        return;

    if (Count_Get(Stream_Video)==0)
        Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "HEVC");
    Fill(Stream_Video, 0, Video_Codec, "HEVC");

    for (std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item=seq_parameter_sets.begin(); seq_parameter_set_Item!=seq_parameter_sets.end(); ++seq_parameter_set_Item)
        if ((*seq_parameter_set_Item))
            Streams_Fill(seq_parameter_set_Item);

    //Library name
    Fill(Stream_General, 0, General_Encoded_Library, Encoded_Library);
    Fill(Stream_General, 0, General_Encoded_Library_Name, Encoded_Library_Name);
    Fill(Stream_General, 0, General_Encoded_Library_Version, Encoded_Library_Version);
    Fill(Stream_General, 0, General_Encoded_Library_Settings, Encoded_Library_Settings);
    Fill(Stream_Video, 0, Video_Encoded_Library, Encoded_Library);
    Fill(Stream_Video, 0, Video_Encoded_Library_Name, Encoded_Library_Name);
    Fill(Stream_Video, 0, Video_Encoded_Library_Version, Encoded_Library_Version);
    Fill(Stream_Video, 0, Video_Encoded_Library_Settings, Encoded_Library_Settings);
}

//---------------------------------------------------------------------------
void File_Hevc::Streams_Fill(std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item)
{
    int32u Width = (*seq_parameter_set_Item)->pic_width_in_luma_samples;
    int32u Height= (*seq_parameter_set_Item)->pic_height_in_luma_samples;
    int32u CropUnitX=Hevc_SubWidthC [(*seq_parameter_set_Item)->ChromaArrayType()];
    int32u CropUnitY=Hevc_SubHeightC[(*seq_parameter_set_Item)->ChromaArrayType()];
    Width -=((*seq_parameter_set_Item)->conf_win_left_offset+(*seq_parameter_set_Item)->conf_win_right_offset)*CropUnitX;
    Height-=((*seq_parameter_set_Item)->conf_win_top_offset +(*seq_parameter_set_Item)->conf_win_bottom_offset)*CropUnitY;

    Ztring Profile;
    if ((*seq_parameter_set_Item)->profile_space==0)
    {
        if ((*seq_parameter_set_Item)->profile_idc)
            Profile=Ztring().From_Local(Hevc_profile_idc((*seq_parameter_set_Item)->profile_idc));
        if ((*seq_parameter_set_Item)->level_idc)
        {
            if ((*seq_parameter_set_Item)->profile_idc)
                Profile+=__T('@');
            Profile+=__T('L')+Ztring().From_Number(((float)(*seq_parameter_set_Item)->level_idc)/30, ((*seq_parameter_set_Item)->level_idc%10)?1:0);
            Profile+=__T('@');
            Profile+=Ztring().From_Local(Hevc_tier_flag((*seq_parameter_set_Item)->tier_flag));
        }
    }
    Fill(Stream_Video, 0, Video_Format_Profile, Profile);
    Fill(Stream_Video, 0, Video_Codec_Profile, Profile);
    Fill(Stream_Video, StreamPos_Last, Video_Width, Width);
    Fill(Stream_Video, StreamPos_Last, Video_Height, Height);

    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
    Fill(Stream_Video, 0, Video_Colorimetry, Hevc_chroma_format_idc((*seq_parameter_set_Item)->chroma_format_idc));
    if ((*seq_parameter_set_Item)->bit_depth_luma_minus8==(*seq_parameter_set_Item)->bit_depth_chroma_minus8)
        Fill(Stream_Video, 0, Video_BitDepth, (*seq_parameter_set_Item)->bit_depth_luma_minus8+8);

    if ((*seq_parameter_set_Item)->vui_parameters)
    {
        if ((*seq_parameter_set_Item)->vui_parameters->timing_info_present_flag)
        {
                if ((*seq_parameter_set_Item)->vui_parameters->time_scale && (*seq_parameter_set_Item)->vui_parameters->num_units_in_tick)
                        Fill(Stream_Video, StreamPos_Last, Video_FrameRate, (float64)(*seq_parameter_set_Item)->vui_parameters->time_scale / (*seq_parameter_set_Item)->vui_parameters->num_units_in_tick);
        }

        if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_info_present_flag)
        {
                float64 PixelAspectRatio = 1;
                if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc<Avc_PixelAspectRatio_Size)
                        PixelAspectRatio = Avc_PixelAspectRatio[(*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc];
                else if ((*seq_parameter_set_Item)->vui_parameters->aspect_ratio_idc == 0xFF && (*seq_parameter_set_Item)->vui_parameters->sar_height)
                        PixelAspectRatio = ((float64) (*seq_parameter_set_Item)->vui_parameters->sar_width) / (*seq_parameter_set_Item)->vui_parameters->sar_height;

                Fill(Stream_Video, 0, Video_PixelAspectRatio, PixelAspectRatio, 3, true);
                Fill(Stream_Video, 0, Video_DisplayAspectRatio, Width*PixelAspectRatio/Height, 3, true); //More precise
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
    }
}

//---------------------------------------------------------------------------
void File_Hevc::Streams_Finish()
{
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Hevc::FileHeader_Begin()
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
bool File_Hevc::Synchronize()
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

    if (File_Offset==0 && Buffer_Offset==0 && (Buffer[3]==0xE0 || Buffer[3]==0xFE))
    {
        //It is from MPEG-PS
        Reject();
        return false;
    }

    //Synched is OK
    Synched=true;
    return true;
}

//---------------------------------------------------------------------------
bool File_Hevc::Synched_Test()
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

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Hevc::Demux_UnpacketizeContainer_Test()
{
    const int8u*    Buffer_Temp=NULL;
    size_t          Buffer_Temp_Size=0;
    bool            RandomAccess=true; //Default, in case of problem

    if ((MustParse_VPS_SPS_PPS || SizedBlocks) && Demux_Transcode_Iso14496_15_to_AnnexB)
    {
        if (MustParse_VPS_SPS_PPS)
            return true; //Wait for SPS and PPS

        //Random access check
        RandomAccess=false;

        //Computing final size
        size_t TranscodedBuffer_Size=0;
        while (Buffer_Offset+lengthSizeMinusOne+1+1<=Buffer_Size)
        {
            size_t Size;
            switch (lengthSizeMinusOne)
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
            Size+=lengthSizeMinusOne+1;

            //Coherency checking
            if (Size==0 || Buffer_Offset+Size>Buffer_Size || (Buffer_Offset+Size!=Buffer_Size && Buffer_Offset+Size+lengthSizeMinusOne+1>Buffer_Size))
                Size=Buffer_Size-Buffer_Offset;

            //Random access check
            if (!RandomAccess && Buffer_Offset+lengthSizeMinusOne+1<Buffer_Size && (Buffer[Buffer_Offset+lengthSizeMinusOne+1]&0x40)==0) //Is a slice
            {
                Element_Offset=lengthSizeMinusOne+1+2;
                Element_Size=Size;

                BS_Begin();
                Get_SB (   first_slice_segment_in_pic_flag,                 "first_slice_segment_in_pic_flag");
                if (first_slice_segment_in_pic_flag)
                {
                    Element_Code=(Buffer[Buffer_Offset+lengthSizeMinusOne+1]&0x3E)>>1; //nal_unit_type
                    RapPicFlag=Element_Code>=16 && Element_Code<=23;
                    if (RapPicFlag)
                        Skip_SB(                                                "no_output_of_prior_pics_flag");
                    Get_UE (   slice_pic_parameter_set_id,                      "slice_pic_parameter_set_id");
                    int8u num_extra_slice_header_bits=(int8u)-1;
                    std::vector<pic_parameter_set_struct*>::iterator pic_parameter_set_Item;
                    if (MustParse_VPS_SPS_PPS_FromFlv)
                        num_extra_slice_header_bits=0; // We bet it is old, so without num_extra_slice_header_bits
                    else if (!(slice_pic_parameter_set_id>=pic_parameter_sets.size() || (*(pic_parameter_set_Item=pic_parameter_sets.begin()+slice_pic_parameter_set_id))==NULL))
                    {
                        num_extra_slice_header_bits=(*pic_parameter_set_Item)->num_extra_slice_header_bits;
                    }
                    if (num_extra_slice_header_bits!=(int8u)-1)
                    {
                        int32u slice_type;
                        Skip_S1(num_extra_slice_header_bits,                    "slice_reserved_flags");
                        Get_UE (slice_type,                                     "slice_type");

                        switch (slice_type)
                        {
                            case 2 :
                            case 7 :
                                        RandomAccess=true;
                        }
                    }
                }
                BS_End();
            }

            TranscodedBuffer_Size+=Size;
            Buffer_Offset+=Size;
        }
        Buffer_Offset=0;

        //Adding VPS/SPS/PPS sizes
        if (RandomAccess)
        {
            for (video_parameter_set_structs::iterator Data_Item=video_parameter_sets.begin(); Data_Item!=video_parameter_sets.end(); ++Data_Item)
                TranscodedBuffer_Size+=(*Data_Item)->AnnexB_Buffer_Size;
            for (seq_parameter_set_structs::iterator Data_Item=seq_parameter_sets.begin(); Data_Item!=seq_parameter_sets.end(); ++Data_Item)
                TranscodedBuffer_Size+=(*Data_Item)->AnnexB_Buffer_Size;
            for (pic_parameter_set_structs::iterator Data_Item=pic_parameter_sets.begin(); Data_Item!=pic_parameter_sets.end(); ++Data_Item)
                TranscodedBuffer_Size+=(*Data_Item)->AnnexB_Buffer_Size;
        }

        //Copying
        int8u* TranscodedBuffer=new int8u[TranscodedBuffer_Size+100];
        size_t TranscodedBuffer_Pos=0;
        if (RandomAccess)
        {
            for (video_parameter_set_structs::iterator Data_Item=video_parameter_sets.begin(); Data_Item!=video_parameter_sets.end(); ++Data_Item)
            {
                std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, (*Data_Item)->AnnexB_Buffer, (*Data_Item)->AnnexB_Buffer_Size);
                TranscodedBuffer_Pos+=(*Data_Item)->AnnexB_Buffer_Size;
            }
            for (seq_parameter_set_structs::iterator Data_Item=seq_parameter_sets.begin(); Data_Item!=seq_parameter_sets.end(); ++Data_Item)
            {
                std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, (*Data_Item)->AnnexB_Buffer, (*Data_Item)->AnnexB_Buffer_Size);
                TranscodedBuffer_Pos+=(*Data_Item)->AnnexB_Buffer_Size;
            }
            for (pic_parameter_set_structs::iterator Data_Item=pic_parameter_sets.begin(); Data_Item!=pic_parameter_sets.end(); ++Data_Item)
            {
                std::memcpy(TranscodedBuffer+TranscodedBuffer_Pos, (*Data_Item)->AnnexB_Buffer, (*Data_Item)->AnnexB_Buffer_Size);
                TranscodedBuffer_Pos+=(*Data_Item)->AnnexB_Buffer_Size;
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
            switch (lengthSizeMinusOne)
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
            if (Size==0 || Buffer_Offset+Size>Buffer_Size || (Buffer_Offset+Size!=Buffer_Size && Buffer_Offset+Size+lengthSizeMinusOne+1>Buffer_Size))
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
        if (!(((Buffer[Buffer_Offset+(zero_byte?4:3)]&0x40)==0 && (Buffer[Buffer_Offset+(zero_byte?6:5)]&0x80)!=0x80)
           || (Buffer[Buffer_Offset+(zero_byte?4:3)]&0x7E)==(38<<1)))
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
                    if (!(((Buffer[Demux_Offset+(zero_byte?4:3)]&0x40)==0 && (Buffer[Demux_Offset+(zero_byte?6:5)]&0x80)!=0x80)
                        || (Buffer[Demux_Offset+(zero_byte?4:3)]&0x7E)==(38<<1)))
                        break;
                }
                else
                {
                    if ((Buffer[Demux_Offset+(zero_byte?4:3)]&0x40)==0 && (Buffer[Demux_Offset+(zero_byte?6:5)]&0x80)==0x80)
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
            if ((Buffer[Buffer_Offset_Random+(zero_byte?4:3)]&0x7E)==(35<<1))
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
            RandomAccess=Buffer_Offset_Random+6<=Buffer_Size && (Buffer[Buffer_Offset_Random+(zero_byte?4:3)]&0x7E)==(32<<1); //video_parameter_set
        }
    }

    if (!Status[IsAccepted])
    {
        if (Config->Demux_EventWasSent)
            return false;
        File_Hevc* MI=new File_Hevc;
        Open_Buffer_Init(MI);
        Open_Buffer_Continue(MI, Buffer, Buffer_Size);
        bool IsOk=MI->Status[IsAccepted];
        delete MI;
        if (!IsOk)
            return false;
    }

    if (IFrame_Count || RandomAccess)
    {
        int64u PTS_Temp=FrameInfo.PTS;
        if (!IsSub)
            FrameInfo.PTS=(int64u)-1;
        Demux_UnpacketizeContainer_Demux(RandomAccess);
        if (!IsSub)
            FrameInfo.PTS=PTS_Temp;
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
void File_Hevc::Synched_Init()
{
    //FrameInfo
    PTS_End=0;
    if (FrameInfo.DTS==(int64u)-1)
        FrameInfo.DTS=0; //No DTS in container
    DTS_Begin=FrameInfo.DTS;
    DTS_End=FrameInfo.DTS;

    //Status
    IFrame_Count=0;

    //Temp

    //Default values
    Streams.resize(0x100);
    Streams[32].Searching_Payload=true; //video_parameter_set
    Streams[35].Searching_Payload=true; //access_unit_delimiter
    Streams[39].Searching_Payload=true; //sei
    for (int8u Pos=0xFF; Pos>=48; Pos--)
        Streams[Pos].Searching_Payload=true; //unspecified

    #if MEDIAINFO_DEMUX
        Demux_Transcode_Iso14496_15_to_AnnexB=Config->Demux_Hevc_Transcode_Iso14496_15_to_AnnexB_Get();
    #endif //MEDIAINFO_DEMUX
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Hevc::Read_Buffer_Unsynched()
{
    //Impossible to know TimeStamps now
    PTS_End=0;
    DTS_End=0;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Hevc::Header_Parse()
{
    //Specific case
    if (MustParse_VPS_SPS_PPS)
    {
        Header_Fill_Size(Element_Size);
        Header_Fill_Code((int64u)-1, "Specific");
        return;
    }

    //Parsing
    int8u nal_unit_type, nuh_temporal_id_plus1;
    if (!SizedBlocks)
    {
        if (Buffer[Buffer_Offset+2]==0x00)
            Skip_B1(                                            "zero_byte");
        Skip_B3(                                                "start_code_prefix_one_3bytes");
        BS_Begin();
        Mark_0 ();
        Get_S1 (6, nal_unit_type,                               "nal_unit_type");
        Get_S1 (6, nuh_layer_id,                                "nuh_layer_id");
        Get_S1 (3, nuh_temporal_id_plus1,                       "nuh_temporal_id_plus1");
        BS_End();

        if (!Header_Parser_Fill_Size())
        {
            Element_WaitForMoreData();
            return;
        }

        if (nuh_temporal_id_plus1==0)
            Trusted_IsNot("nuh_temporal_id_plus1");
    }
    else
    {
        int32u Size;
        switch (lengthSizeMinusOne)
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
        Get_S1 (6, nal_unit_type,                               "nal_unit_type");
        Get_S1 (6, nuh_layer_id,                                "nuh_layer_id");
        Get_S1 (3, nuh_temporal_id_plus1,                       "nuh_temporal_id_plus1");
        BS_End();

        if (nuh_temporal_id_plus1==0)
            Trusted_IsNot("nuh_temporal_id_plus1");

        FILLING_BEGIN();
            Header_Fill_Size(Size?(Element_Offset-2+Size):(Buffer_Size-Buffer_Offset)); //If Size is 0, it is not normal, we skip the complete frame
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
bool File_Hevc::Header_Parser_Fill_Size()
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
bool File_Hevc::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+6<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x00
      &&   Buffer[Buffer_Offset+1]==0x00
      &&  (Buffer[Buffer_Offset+2]==0x01
        || (Buffer[Buffer_Offset+2]==0x00 && Buffer[Buffer_Offset+3]==0x01)))
    {
        //Getting start_code
        int8u nal_unit_type;
        if (Buffer[Buffer_Offset+2]==0x00)
            nal_unit_type=(CC1(Buffer+Buffer_Offset+4)&0x7E)>>1;
        else
            nal_unit_type=(CC1(Buffer+Buffer_Offset+3)&0x7E)>>1;

        //Searching start
        if (Streams[nal_unit_type].Searching_Payload)
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

    Trusted_IsNot("HEVC, Synchronisation lost");
    return Synchronize();
}

//---------------------------------------------------------------------------
void File_Hevc::Data_Parse()
{
    //Specific case
    if (Element_Code==(int64u)-1)
    {
        VPS_SPS_PPS();
        return;
    }

    if (nuh_layer_id)
    {
        //Skip_XX(Element_Size,                                   "(Extension)");
        //return;
    }

    //Trailing zeroes
    int64u Element_Size_SaveBeforeZeroes=Element_Size;
    if (Element_Size)
    {
        while (Element_Size && Buffer[Buffer_Offset+(size_t)Element_Size-1]==0)
            Element_Size--;
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
        case  0 :
        case  1 :
        case  2 :
        case  3 :
        case 16 :
        case 17 :
        case 18 :
        case 19 :
        case 20 :
        case 21 :
                  slice_segment_layer(); break;
        case  4 :
        case  5 :
        case  6 :
        case  7 :
        case  8 :
        case  9 :
                  slice_layer(); break;
        case 32 : video_parameter_set(); break;
        case 33 : seq_parameter_set(); break;
        case 34 : pic_parameter_set(); break;
        case 35 : access_unit_delimiter(); break;
        case 36 : end_of_seq(); break;
        case 37 : end_of_bitstream(); break;
        case 38 : filler_data(); break;
        case 39 :
        case 40 :
                  sei(); break;
        default :
                  Skip_XX(Element_Size-Element_Offset, "Data");
                  if (Element_Code>=48)
                      Trusted_IsNot("Unspecified");
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

    #if MEDIAINFO_DEMUX
        if (Demux_Transcode_Iso14496_15_to_AnnexB)
        {
            if (Element_Code==32)
            {
                std::vector<video_parameter_set_struct*>::iterator Data_Item=video_parameter_sets.begin();
                if (Data_Item!=video_parameter_sets.end() && (*Data_Item))
                {
                    delete[] (*Data_Item)->AnnexB_Buffer;
                    (*Data_Item)->AnnexB_Buffer_Size=(size_t)(Element_Size+5);
                    (*Data_Item)->AnnexB_Buffer=new int8u[(*Data_Item)->AnnexB_Buffer_Size];
                    (*Data_Item)->AnnexB_Buffer[0]=0x00;
                    (*Data_Item)->AnnexB_Buffer[1]=0x00;
                    (*Data_Item)->AnnexB_Buffer[2]=0x01;
                    (*Data_Item)->AnnexB_Buffer[3]=Buffer[Buffer_Offset-2];
                    (*Data_Item)->AnnexB_Buffer[4]=Buffer[Buffer_Offset-1];
                    std::memcpy((*Data_Item)->AnnexB_Buffer+5, Buffer+Buffer_Offset, (size_t)Element_Size);
                }
            }
            if (Element_Code==33)
            {
                std::vector<seq_parameter_set_struct*>::iterator Data_Item=seq_parameter_sets.begin();
                if (Data_Item!=seq_parameter_sets.end() && (*Data_Item))
                {
                    delete[] (*Data_Item)->AnnexB_Buffer;
                    (*Data_Item)->AnnexB_Buffer_Size=(size_t)(Element_Size+5);
                    (*Data_Item)->AnnexB_Buffer=new int8u[(*Data_Item)->AnnexB_Buffer_Size];
                    (*Data_Item)->AnnexB_Buffer[0]=0x00;
                    (*Data_Item)->AnnexB_Buffer[1]=0x00;
                    (*Data_Item)->AnnexB_Buffer[2]=0x01;
                    (*Data_Item)->AnnexB_Buffer[3]=Buffer[Buffer_Offset-2];
                    (*Data_Item)->AnnexB_Buffer[4]=Buffer[Buffer_Offset-1];
                    std::memcpy((*Data_Item)->AnnexB_Buffer+5, Buffer+Buffer_Offset, (size_t)Element_Size);
                }
            }
            if (Element_Code==34)
            {
                std::vector<pic_parameter_set_struct*>::iterator Data_Item=pic_parameter_sets.begin();
                if (Data_Item!=pic_parameter_sets.end() && (*Data_Item))
                {
                    delete[] (*Data_Item)->AnnexB_Buffer;
                    (*Data_Item)->AnnexB_Buffer_Size=(size_t)(Element_Size+5);
                    (*Data_Item)->AnnexB_Buffer=new int8u[(*Data_Item)->AnnexB_Buffer_Size];
                    (*Data_Item)->AnnexB_Buffer[0]=0x00;
                    (*Data_Item)->AnnexB_Buffer[1]=0x00;
                    (*Data_Item)->AnnexB_Buffer[2]=0x01;
                    (*Data_Item)->AnnexB_Buffer[3]=Buffer[Buffer_Offset-2];
                    (*Data_Item)->AnnexB_Buffer[4]=Buffer[Buffer_Offset-1];
                    std::memcpy((*Data_Item)->AnnexB_Buffer+5, Buffer+Buffer_Offset, (size_t)Element_Size);
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
// Packets
void File_Hevc::slice_segment_layer()
{
    #if MEDIAINFO_TRACE
        Element_Name("slice_segment_layer");
        switch (Element_Code)
        {
            case 0 :
            case 1 : break;
            case 2 :
            case 3 : Element_Info("TSA"); break;
            case 16 :
            case 17 :
            case 18 : Element_Info("BLA"); break;
            case 19 :
            case 20 : Element_Info("IDR"); break;
            case 21 : Element_Info("CRA"); break;
            default: ;
        }
    #endif //MEDIAINFO_TRACE

    //Parsing
    RapPicFlag=Element_Code>=16 && Element_Code<=23;
    BS_Begin();
    slice_segment_header();
    BS_End();
    Skip_XX(Element_Size-Element_Offset,                        "(ToDo)");

    FILLING_BEGIN();
        if (slice_pic_parameter_set_id==(int32u)-1)
            return;

        //Count of I-Frames
        if (first_slice_segment_in_pic_flag && (Element_Code==19 || Element_Code==20))
            IFrame_Count++;

        if (first_slice_segment_in_pic_flag)
        {
            //Frame_Count
            Frame_Count++;
            if (IFrame_Count && Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded++;
            Frame_Count_InThisBlock++;

            //Filling only if not already done
            if (Frame_Count==1 && !Status[IsAccepted])
                Accept("HEVC");
            if (!Status[IsFilled])
            {
                if (IFrame_Count>=8)
                    Frame_Count_Valid=Frame_Count; //We have enough frames
                if (Frame_Count>=Frame_Count_Valid)
                {
                    Fill("HEVC");
                    if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1.0)
                        Finish("HEVC");
                }
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packets
void File_Hevc::slice_layer()
{
    #if MEDIAINFO_TRACE
        Element_Name("slice_layer");
        switch (Element_Code)
        {
            case 4 :
            case 5 : Element_Info("STSA"); break;
            case 6 :
            case 7 : Element_Info("RADL"); break;
            case 8 :
            case 9 : Element_Info("RASL"); break;
            default: ;
        }
    #endif //MEDIAINFO_TRACE

    //Parsing
    Skip_XX(Element_Size-Element_Offset,                        "(ToDo)");
}

//---------------------------------------------------------------------------
// Packet "32"
void File_Hevc::video_parameter_set()
{
    Element_Name("video_parameter_set");

    //Parsing
    int32u  vps_num_layer_sets_minus1;
    int8u   vps_video_parameter_set_id, vps_max_sub_layers_minus1, vps_max_layer_id;
    bool    vps_temporal_id_nesting_flag, vps_sub_layer_ordering_info_present_flag;
    BS_Begin();
    Get_S1 (4,  vps_video_parameter_set_id,                     "vps_video_parameter_set_id");
    if (MustParse_VPS_SPS_PPS_FromFlv)
    {
        BS_End();
        Skip_XX(Element_Size-Element_Offset,                     "Data");

        //Creating Data
        if (vps_video_parameter_set_id>=video_parameter_sets.size())
            video_parameter_sets.resize(vps_video_parameter_set_id+1);
        std::vector<video_parameter_set_struct*>::iterator Data_Item=video_parameter_sets.begin()+vps_video_parameter_set_id;
        delete *Data_Item; *Data_Item=new video_parameter_set_struct(
                                                                        0 //TODO: check which code is intended here
                                                                    );

        //NextCode
        NextCode_Clear();
        NextCode_Add(33);

        //Autorisation of other streams
        Streams[33].Searching_Payload=true; //seq_parameter_set
        Streams[36].Searching_Payload=true; //end_of_seq
        Streams[37].Searching_Payload=true; //end_of_bitstream
        Streams[38].Searching_Payload=true; //filler_data

        return;
    }
    Skip_S1(2,                                                  "vps_reserved_three_2bits");
    Skip_S1(6,                                                  "vps_reserved_zero_6bits");
    Get_S1 (3,  vps_max_sub_layers_minus1,                      "vps_max_sub_layers_minus1");
    if (vps_max_sub_layers_minus1>6)
    {
        Trusted_IsNot("vps_max_sub_layers_minus1 not valid");
        BS_End();
        return; //Problem, not valid
    }
    Get_SB (    vps_temporal_id_nesting_flag,                   "vps_temporal_id_nesting_flag");
    //if (vps_max_sub_layers_minus1==0 && !vps_temporal_id_nesting_flag)
    //{
    //    Trusted_IsNot("vps_temporal_id_nesting_flag not valid");
    //    BS_End();
    //    return; //Problem, not valid
    //}
    Skip_S2(16,                                                 "vps_reserved_0xffff_16bits");
    profile_tier_level(vps_max_sub_layers_minus1);
    Get_SB (   vps_sub_layer_ordering_info_present_flag,        "vps_sub_layer_ordering_info_present_flag");
    for (int32u SubLayerPos=(vps_sub_layer_ordering_info_present_flag?0:vps_max_sub_layers_minus1); SubLayerPos<=vps_max_sub_layers_minus1; SubLayerPos++)
    {
        Element_Begin1("SubLayer");
        Skip_UE(                                                "vps_max_dec_pic_buffering_minus1");
        Skip_UE(                                                "vps_max_num_reorder_pics");
        Skip_UE(                                                "vps_max_latency_increase_plus1");
        Element_End0();
    }
    Get_S1 ( 6, vps_max_layer_id,                               "vps_max_layer_id");
    Get_UE (    vps_num_layer_sets_minus1,                      "vps_num_layer_sets_minus1");
    if (vps_num_layer_sets_minus1>=1024)
    {
        Trusted_IsNot("vps_num_layer_sets_minus1 not valid");
        BS_End();
        return; //Problem, not valid
    }
    for (int32u LayerSetPos=1; LayerSetPos<=vps_num_layer_sets_minus1; LayerSetPos++)
        for (int8u LayerId=0; LayerId<=vps_max_layer_id; LayerId++)
            Skip_SB(                                            "layer_id_included_flag");
    TEST_SB_SKIP(                                               "vps_timing_info_present_flag");
        int32u vps_time_scale, vps_num_hrd_parameters;
        Skip_S4(32,                                             "vps_num_units_in_tick");
        Get_S4 (32, vps_time_scale,                             "vps_time_scale");
        if (vps_time_scale==0)
        {
            Trusted_IsNot("vps_time_scale not valid");
            Element_End0();
            BS_End();
            return; //Problem, not valid
        }
        TEST_SB_SKIP(                                           "vps_poc_proportional_to_timing_flag");
            Skip_UE(                                            "vps_num_ticks_poc_diff_one_minus1");
        TEST_SB_END();
        Get_UE (    vps_num_hrd_parameters,                     "vps_num_hrd_parameters");
        if (vps_num_hrd_parameters>1024)
        {
            Trusted_IsNot("vps_num_hrd_parameters not valid");
            vps_num_hrd_parameters=0;
        }
        for (int32u HrdPos=0; HrdPos<vps_num_hrd_parameters; HrdPos++)
        {
            int32u hrd_layer_set_idx;
            Get_UE (   hrd_layer_set_idx,                       "hrd_layer_set_idx");
            if (hrd_layer_set_idx>=1024)
                Trusted_IsNot("hrd_layer_set_idx not valid");
            if (HrdPos)
                Skip_SB(                                        "cprms_present_flag");
            Trusted_IsNot("hrd_parameters not supported");
            //hrd_parameters
        }
    TEST_SB_END();
    TESTELSE_SB_SKIP(                                           "vps_extension_flag");
        Skip_BS(Data_BS_Remain(),                               "vps_extension_data");
        Trusted_IsNot("(Not supported)"); // Should be skipped, but the detection is too sensible in that case, do not remove it until the detection is adapted
    TESTELSE_SB_ELSE(                                           "vps_extension_flag");
        Mark_1();
        while (Data_BS_Remain())
            Mark_0();
    TESTELSE_SB_END();
    BS_End();

    FILLING_BEGIN_PRECISE();
        //Creating Data
        if (vps_video_parameter_set_id>=video_parameter_sets.size())
            video_parameter_sets.resize(vps_video_parameter_set_id+1);
        std::vector<video_parameter_set_struct*>::iterator Data_Item=video_parameter_sets.begin()+vps_video_parameter_set_id;
        delete *Data_Item; *Data_Item=new video_parameter_set_struct(
                                                                        vps_max_sub_layers_minus1
                                                                    );

        //NextCode
        NextCode_Clear();
        NextCode_Add(33);

        //Autorisation of other streams
        Streams[33].Searching_Payload=true; //seq_parameter_set
        Streams[36].Searching_Payload=true; //end_of_seq
        Streams[37].Searching_Payload=true; //end_of_bitstream
        Streams[38].Searching_Payload=true; //filler_data
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "33"
void File_Hevc::seq_parameter_set()
{
    Element_Name("seq_parameter_set");

    //Parsing
    seq_parameter_set_struct::vui_parameters_struct* vui_parameters_Item=NULL;
    int32u  sps_seq_parameter_set_id, chroma_format_idc, pic_width_in_luma_samples, pic_height_in_luma_samples, bit_depth_luma_minus8, bit_depth_chroma_minus8, log2_max_pic_order_cnt_lsb_minus4, num_short_term_ref_pic_sets;
    int32u  conf_win_left_offset=0, conf_win_right_offset=0, conf_win_top_offset=0, conf_win_bottom_offset=0;
    int8u   video_parameter_set_id, max_sub_layers_minus1;
    bool    separate_colour_plane_flag=false, sps_sub_layer_ordering_info_present_flag;
    BS_Begin();
    Get_S1 (4, video_parameter_set_id,                          "sps_video_parameter_set_id");
    std::vector<video_parameter_set_struct*>::iterator video_parameter_set_Item;
    if (video_parameter_set_id >= video_parameter_sets.size() || (*(video_parameter_set_Item = video_parameter_sets.begin() + video_parameter_set_id)) == NULL)
    {
        //Not yet present
        Skip_BS(Data_BS_Remain(),                               "Data (video_parameter_set is missing)");
        BS_End();
        return;
    }
    Get_S1 (3, max_sub_layers_minus1,                           "sps_max_sub_layers_minus1");
    Skip_SB(                                                    "sps_temporal_id_nesting_flag");
    profile_tier_level(max_sub_layers_minus1);
    Get_UE (   sps_seq_parameter_set_id,                        "sps_seq_parameter_set_id");
    if (MustParse_VPS_SPS_PPS_FromFlv)
    {
        BS_End();
        Skip_XX(Element_Size-Element_Offset,                     "Data");

        //Creating Data
        if (sps_seq_parameter_set_id>=seq_parameter_sets.size())
            seq_parameter_sets.resize(sps_seq_parameter_set_id+1);
        std::vector<seq_parameter_set_struct*>::iterator Data_Item=seq_parameter_sets.begin()+sps_seq_parameter_set_id;
        delete *Data_Item; *Data_Item=new seq_parameter_set_struct(
                                                                    NULL, //TODO: check which code is intended here
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
                                                                    0,
                                                                    0,
                                                                    0,
                                                                    false,
                                                                    false,
                                                                    false
                                                                    );

        //NextCode
        NextCode_Clear();
        NextCode_Add(34);

        //Autorisation of other streams
        Streams[34].Searching_Payload=true; //pic_parameter_set

        return;
    }
    Get_UE (   chroma_format_idc,                               "chroma_format_idc"); Param_Info1(Hevc_chroma_format_idc((int8u)chroma_format_idc));
    if (chroma_format_idc>=4)
    {
        Trusted_IsNot("chroma_format_idc not valid");
        BS_End();
        return; //Problem, not valid
    }
    if (chroma_format_idc==3)
        Get_SB (separate_colour_plane_flag,                     "separate_colour_plane_flag");
    Get_UE (    pic_width_in_luma_samples,                      "pic_width_in_luma_samples");
    Get_UE (    pic_height_in_luma_samples,                     "pic_height_in_luma_samples");
    TEST_SB_SKIP(                                               "conformance_window_flag");
        Get_UE (conf_win_left_offset,                           "conf_win_left_offset");
        Get_UE (conf_win_right_offset,                          "conf_win_right_offset");
        Get_UE (conf_win_top_offset,                            "conf_win_top_offset");
        Get_UE (conf_win_bottom_offset,                         "conf_win_bottom_offset");
    TEST_SB_END();
    Get_UE (   bit_depth_luma_minus8,                           "bit_depth_luma_minus8");
    if (bit_depth_luma_minus8>6)
    {
        Trusted_IsNot("bit_depth_luma_minus8 not valid");
        return; //Problem, not valid
    }
    Get_UE (   bit_depth_chroma_minus8,                         "bit_depth_chroma_minus8");
    if (bit_depth_chroma_minus8>6)
    {
        Trusted_IsNot("bit_depth_chroma_minus8 not valid");
        return; //Problem, not valid
    }
    Get_UE (   log2_max_pic_order_cnt_lsb_minus4,               "log2_max_pic_order_cnt_lsb_minus4");
    if (log2_max_pic_order_cnt_lsb_minus4>12)
    {
        Trusted_IsNot("log2_max_pic_order_cnt_lsb_minus4 not valid");
        return; //Problem, not valid
    }
    Get_SB (   sps_sub_layer_ordering_info_present_flag,        "sps_sub_layer_ordering_info_present_flag");
    for (int32u SubLayerPos = (sps_sub_layer_ordering_info_present_flag ? 0 : max_sub_layers_minus1); SubLayerPos <= max_sub_layers_minus1; SubLayerPos++)
    {
        Element_Begin1("SubLayer");
        Skip_UE(                                                "sps_max_dec_pic_buffering_minus1");
        Skip_UE(                                                "sps_max_num_reorder_pics");
        Skip_UE(                                                "sps_max_latency_increase_plus1");
        Element_End0();
    }
    Skip_UE(                                                    "log2_min_luma_coding_block_size_minus3");
    Skip_UE(                                                    "log2_diff_max_min_luma_coding_block_size");
    Skip_UE(                                                    "log2_min_transform_block_size_minus2");
    Skip_UE(                                                    "log2_diff_max_min_transform_block_size");
    Skip_UE(                                                    "max_transform_hierarchy_depth_inter");
    Skip_UE(                                                    "max_transform_hierarchy_depth_intra");
    TEST_SB_SKIP(                                               "scaling_list_enabled_flag");
        TEST_SB_SKIP(                                           "sps_scaling_list_data_present_flag");
            scaling_list_data();
        TEST_SB_END();
    TEST_SB_END();
    Skip_SB(                                                    "amp_enabled_flag");
    Skip_SB(                                                    "sample_adaptive_offset_enabled_flag");
    TEST_SB_SKIP(                                               "pcm_enabled_flag");
        Element_Begin1("pcm");
        Skip_S1(4,                                              "pcm_sample_bit_depth_luma_minus1");
        Skip_S1(4,                                              "pcm_sample_bit_depth_chroma_minus1");
        Skip_UE(                                                "log2_min_pcm_luma_coding_block_size_minus3");
        Skip_UE(                                                "log2_diff_max_min_pcm_luma_coding_block_size");
        Skip_SB(                                                "pcm_loop_filter_disabled_flag");
        Element_End0();
    TEST_SB_END();
    Get_UE (   num_short_term_ref_pic_sets,                     "num_short_term_ref_pic_sets");
    if (num_short_term_ref_pic_sets>64)
    {
        BS_End();
        Trusted_IsNot("num_short_term_ref_pic_sets not valid");
        return; //Problem, not valid
    }
    short_term_ref_pic_sets((int8u)num_short_term_ref_pic_sets);
    TEST_SB_SKIP(                                               "long_term_ref_pics_present_flag");
        Element_Begin1("long_term_ref_pics");
        int32u num_long_term_ref_pics_sps;
        Get_UE (num_long_term_ref_pics_sps,                     "num_long_term_ref_pics_sps");
        for (int32u long_term_ref_pics_sps_pos=0; long_term_ref_pics_sps_pos<num_long_term_ref_pics_sps; long_term_ref_pics_sps_pos++)
        {
            Skip_BS(log2_max_pic_order_cnt_lsb_minus4+4,        "lt_ref_pic_poc_lsb_sps");
            Skip_SB(                                            "used_by_curr_pic_lt_sps_flag");
        }
        Element_End0();
    TEST_SB_END();
    Skip_SB(                                                    "sps_temporal_mvp_enabled_flag");
    Skip_SB(                                                    "strong_intra_smoothing_enabled_flag");
    TEST_SB_SKIP(                                               "vui_parameters_present_flag");
        vui_parameters(video_parameter_set_Item, vui_parameters_Item);
    TEST_SB_END();
    TESTELSE_SB_SKIP(                                           "sps_extension_flag");
        Skip_BS(Data_BS_Remain(),                               "sps_extension_data");
        Trusted_IsNot("(Not supported)"); // Should be skipped, but the detection is too sensible in that case, do not remove it until the detection is adapted
    TESTELSE_SB_ELSE(                                           "sps_extension_flag");
        Mark_1();
        while (Data_BS_Remain())
            Mark_0();
    TESTELSE_SB_END();
    BS_End();

    FILLING_BEGIN_PRECISE();
        //Creating Data
        if (sps_seq_parameter_set_id>=seq_parameter_sets.size())
            seq_parameter_sets.resize(sps_seq_parameter_set_id+1);
        std::vector<seq_parameter_set_struct*>::iterator Data_Item=seq_parameter_sets.begin()+sps_seq_parameter_set_id;
        delete *Data_Item; *Data_Item=new seq_parameter_set_struct(
                                                                    vui_parameters_Item,
                                                                    profile_space,
                                                                    tier_flag,
                                                                    profile_idc,
                                                                    level_idc,
                                                                    pic_width_in_luma_samples,
                                                                    pic_height_in_luma_samples,
                                                                    conf_win_left_offset,
                                                                    conf_win_right_offset,
                                                                    conf_win_top_offset,
                                                                    conf_win_bottom_offset,
                                                                    (int8u)video_parameter_set_id,
                                                                    (int8u)chroma_format_idc,
                                                                    separate_colour_plane_flag,
                                                                    (int8u)log2_max_pic_order_cnt_lsb_minus4,
                                                                    (int8u)bit_depth_luma_minus8,
                                                                    (int8u)bit_depth_chroma_minus8,
                                                                    general_progressive_source_flag,
                                                                    general_interlaced_source_flag,
                                                                    general_frame_only_constraint_flag
                                                                  );

        //NextCode
        NextCode_Clear();
        NextCode_Add(34);

        //Autorisation of other streams
        Streams[34].Searching_Payload=true; //pic_parameter_set
    FILLING_ELSE();
        delete vui_parameters_Item; //vui_parameters_Item=NULL;
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "34"
void File_Hevc::pic_parameter_set()
{
    Element_Name("pic_parameter_set");

    //Parsing
    int32u  pps_pic_parameter_set_id, pps_seq_parameter_set_id, num_ref_idx_l0_default_active_minus1, num_ref_idx_l1_default_active_minus1;
    int8u   num_extra_slice_header_bits;
    bool    tiles_enabled_flag, dependent_slice_segments_enabled_flag;
    BS_Begin();
    Get_UE (    pps_pic_parameter_set_id,                       "pps_pic_parameter_set_id");
    if (pps_pic_parameter_set_id>=64)
    {
        Trusted_IsNot("pic_parameter_set_id not valid");
        BS_End();
        return; //Problem, not valid
    }
    Get_UE (    pps_seq_parameter_set_id,                       "pps_seq_parameter_set_id");
    if (pps_seq_parameter_set_id>=16)
    {
        Trusted_IsNot("seq_parameter_set_id not valid");
        return; //Problem, not valid
    }
    //std::vector<seq_parameter_set_struct*>::iterator seq_parameter_set_Item;
    if (pps_seq_parameter_set_id>=seq_parameter_sets.size() || (*(seq_parameter_sets.begin()+pps_seq_parameter_set_id))==NULL) //(seq_parameter_set_Item=seq_parameter_sets.begin()+pps_seq_parameter_set_id) not usd for the moment
    {
        //Not yet present
        Skip_BS(Data_BS_Remain(),                               "Data (seq_parameter_set is missing)");
        BS_End();
        return;
    }
    if (MustParse_VPS_SPS_PPS_FromFlv)
    {
        BS_End();
        Skip_XX(Element_Size-Element_Offset,                     "Data");

        //Filling
        if (pps_pic_parameter_set_id>=pic_parameter_sets.size())
            pic_parameter_sets.resize(pps_pic_parameter_set_id+1);
        std::vector<pic_parameter_set_struct*>::iterator pic_parameter_sets_Item=pic_parameter_sets.begin()+pps_pic_parameter_set_id;
        delete *pic_parameter_sets_Item; *pic_parameter_sets_Item=new pic_parameter_set_struct(
                                                                                                    0,
                                                                                                    0,
                                                                                                    0,
                                                                                                    0,
                                                                                                    false
                                                                                              );

        //NextCode
        NextCode_Clear();

        //Autorisation of other streams
        Streams[ 0].Searching_Payload=true; //slice_segment_layer
        Streams[ 1].Searching_Payload=true; //slice_segment_layer
        Streams[ 2].Searching_Payload=true; //slice_segment_layer
        Streams[ 3].Searching_Payload=true; //slice_segment_layer
        Streams[ 4].Searching_Payload=true; //slice_layer
        Streams[ 5].Searching_Payload=true; //slice_layer
        Streams[ 6].Searching_Payload=true; //slice_layer
        Streams[ 7].Searching_Payload=true; //slice_layer
        Streams[ 8].Searching_Payload=true; //slice_layer
        Streams[ 9].Searching_Payload=true; //slice_layer
        Streams[16].Searching_Payload=true; //slice_segment_layer
        Streams[17].Searching_Payload=true; //slice_segment_layer
        Streams[18].Searching_Payload=true; //slice_segment_layer
        Streams[19].Searching_Payload=true; //slice_segment_layer
        Streams[20].Searching_Payload=true; //slice_segment_layer
        Streams[21].Searching_Payload=true; //slice_segment_layer

        return;
    }
    Get_SB (    dependent_slice_segments_enabled_flag,          "dependent_slice_segments_enabled_flag");
    Skip_SB(                                                    "output_flag_present_flag");
    Get_S1 (3,  num_extra_slice_header_bits,                    "num_extra_slice_header_bits");
    Skip_SB(                                                    "sign_data_hiding_flag");
    Skip_SB(                                                    "cabac_init_present_flag");
    Get_UE (    num_ref_idx_l0_default_active_minus1,           "num_ref_idx_l0_default_active_minus1");
    Get_UE (    num_ref_idx_l1_default_active_minus1,           "num_ref_idx_l1_default_active_minus1");
    Skip_SE(                                                    "init_qp_minus26");
    Skip_SB(                                                    "constrained_intra_pred_flag");
    Skip_SB(                                                    "transform_skip_enabled_flag");
    TEST_SB_SKIP(                                               "cu_qp_delta_enabled_flag");
        Skip_UE(                                                "diff_cu_qp_delta_depth");
    TEST_SB_END();
    Skip_SE(                                                    "pps_cb_qp_offset");
    Skip_SE(                                                    "pps_cr_qp_offset");
    Skip_SB(                                                    "pps_slice_chroma_qp_offsets_present_flag");
    Skip_SB(                                                    "weighted_pred_flag");
    Skip_SB(                                                    "weighted_bipred_flag");
    Skip_SB(                                                    "transquant_bypass_enable_flag");
    Get_SB (    tiles_enabled_flag,                             "tiles_enabled_flag");
    Skip_SB(                                                    "entropy_coding_sync_enabled_flag");
    if (tiles_enabled_flag)
    {
        Element_Begin1("tiles");
        int32u  num_tile_columns_minus1, num_tile_rows_minus1;
        bool    uniform_spacing_flag;
        Get_UE (    num_tile_columns_minus1,                    "num_tile_columns_minus1");
        Get_UE (    num_tile_rows_minus1,                       "num_tile_rows_minus1");
        Get_SB (    uniform_spacing_flag,                       "uniform_spacing_flag");
        if (!uniform_spacing_flag)
        {
            for (int32u tile_pos=0; tile_pos<num_tile_columns_minus1; tile_pos++)
               Skip_UE(                                         "column_width_minus1");
            for (int32u tile_pos=0; tile_pos<num_tile_rows_minus1; tile_pos++)
               Skip_UE(                                         "row_height_minus1");
        }
        Skip_SB(                                                "loop_filter_across_tiles_enabled_flag");
        Element_End0();
    }
    Skip_SB(                                                    "pps_loop_filter_across_slices_enabled_flag");
    TEST_SB_SKIP(                                               "deblocking_filter_control_present_flag");
        bool pps_disable_deblocking_filter_flag;
        Skip_SB(                                                "deblocking_filter_override_enabled_flag");
        Get_SB (    pps_disable_deblocking_filter_flag,         "pps_disable_deblocking_filter_flag");
        if (!pps_disable_deblocking_filter_flag)
        {
           Skip_SE(                                             "pps_beta_offset_div2");
           Skip_SE(                                             "pps_tc_offset_div2");
        }
    TEST_SB_END();
    TEST_SB_SKIP(                                               "pps_scaling_list_data_present_flag ");
        scaling_list_data();
    TEST_SB_END();
    Skip_SB(                                                    "lists_modification_present_flag");
    Skip_UE(                                                    "log2_parallel_merge_level_minus2");
    Skip_SB(                                                    "slice_segment_header_extension_present_flag");
    TESTELSE_SB_SKIP(                                           "pps_extension_flag");
        Skip_BS(Data_BS_Remain(),                               "pps_extension_data");
        Trusted_IsNot("(Not supported)"); // Should be skipped, but the detection is too sensible in that case, do not remove it until the detection is adapted
    TESTELSE_SB_ELSE(                                           "pps_extension_flag");
        Mark_1();
        while (Data_BS_Remain())
            Mark_0();
    TESTELSE_SB_END();
    BS_End();

    FILLING_BEGIN_PRECISE();
        //NextCode
        //NextCode_Clear();
        //NextCode_Add(0x05);
        //NextCode_Add(0x06);

        //Filling
        if (pps_pic_parameter_set_id>=pic_parameter_sets.size())
            pic_parameter_sets.resize(pps_pic_parameter_set_id+1);
        std::vector<pic_parameter_set_struct*>::iterator pic_parameter_sets_Item=pic_parameter_sets.begin()+pps_pic_parameter_set_id;
        delete *pic_parameter_sets_Item; *pic_parameter_sets_Item = new pic_parameter_set_struct(
                                                                                                    (int8u)pps_seq_parameter_set_id,
                                                                                                    (int8u)num_ref_idx_l0_default_active_minus1,
                                                                                                    (int8u)num_ref_idx_l1_default_active_minus1,
                                                                                                    num_extra_slice_header_bits,
                                                                                                    dependent_slice_segments_enabled_flag
                                                                                                );
        //Autorisation of other streams
        //if (!seq_parameter_sets.empty())
        //{
        //    for (int8u Pos=0x01; Pos<=0x06; Pos++)
        //    {
        //        Streams[Pos].Searching_Payload=true; //Coded slice...
        //        if (Streams[0x08].ShouldDuplicate)
        //            Streams[Pos].ShouldDuplicate=true;
        //    }
        //}

        //NextCode
        NextCode_Clear();

        //Autorisation of other streams
        Streams[ 0].Searching_Payload=true; //slice_segment_layer
        Streams[ 1].Searching_Payload=true; //slice_segment_layer
        Streams[ 2].Searching_Payload=true; //slice_segment_layer
        Streams[ 3].Searching_Payload=true; //slice_segment_layer
        Streams[ 4].Searching_Payload=true; //slice_layer
        Streams[ 5].Searching_Payload=true; //slice_layer
        Streams[ 6].Searching_Payload=true; //slice_layer
        Streams[ 7].Searching_Payload=true; //slice_layer
        Streams[ 8].Searching_Payload=true; //slice_layer
        Streams[ 9].Searching_Payload=true; //slice_layer
        Streams[16].Searching_Payload=true; //slice_segment_layer
        Streams[17].Searching_Payload=true; //slice_segment_layer
        Streams[18].Searching_Payload=true; //slice_segment_layer
        Streams[19].Searching_Payload=true; //slice_segment_layer
        Streams[20].Searching_Payload=true; //slice_segment_layer
        Streams[21].Searching_Payload=true; //slice_segment_layer
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "35"
void File_Hevc::access_unit_delimiter()
{
    Element_Name("access_unit_delimiter");

    //Parsing
    BS_Begin();
    Info_S1( 3, pic_type,                                   "pic_type"); Param_Info1(Hevc_pic_type[pic_type]);
    BS_End();
}

//---------------------------------------------------------------------------
// Packet "36"
void File_Hevc::end_of_seq()
{
    Element_Name("end_of_seq");
}

//---------------------------------------------------------------------------
// Packet "37"
void File_Hevc::end_of_bitstream()
{
    Element_Name("end_of_bitstream");
}

//---------------------------------------------------------------------------
// Packet "38"
void File_Hevc::filler_data()
{
    Element_Name("filler_data");

    //Parsing
    Skip_XX(Element_Size,                                       "ff_bytes");
}

//---------------------------------------------------------------------------
// Packet "39" or "40
void File_Hevc::sei()
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
void File_Hevc::sei_message(int32u &seq_parameter_set_id)
{
    //Parsing
    int32u  payloadType=0, payloadSize=0;
    int8u   payload_type_byte, payload_size_byte;
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
        case   0 :   sei_message_buffering_period(seq_parameter_set_id, payloadSize); break;
        case   1 :   sei_message_pic_timing(seq_parameter_set_id, payloadSize); break;
        //case   4 :   sei_message_user_data_registered_itu_t_t35(); break;
        case   5 :   sei_message_user_data_unregistered(payloadSize); break;
        //case   6 :   sei_message_recovery_point(); break;
        //case  32 :   sei_message_mainconcept(payloadSize); break;
        case 129 :   sei_message_active_parameter_sets(); break;
        case 132 :   sei_message_decoded_picture_hash(payloadSize); break;
        default :
                    Element_Info1("unknown");
                    Skip_XX(payloadSize,                        "data");
    }
    Element_Offset=Element_Offset_Save; //Positionning in the right place.
    Element_Size=Element_Size_Save; //Positionning in the right place.
}

//---------------------------------------------------------------------------
// SEI - 0
void File_Hevc::sei_message_buffering_period(int32u &seq_parameter_set_id, int32u payloadSize)
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
    bool sub_pic_hrd_params_present_flag=false; //Default
    bool irap_cpb_params_present_flag=((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->xxL_Common)?(*seq_parameter_set_Item)->vui_parameters->xxL_Common->sub_pic_hrd_params_present_flag:false;
    if (!sub_pic_hrd_params_present_flag)
        Get_SB (irap_cpb_params_present_flag,                   "irap_cpb_params_present_flag");
    int8u au_cpb_removal_delay_length_minus1=((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->xxL_Common)?(*seq_parameter_set_Item)->vui_parameters->xxL_Common->au_cpb_removal_delay_length_minus1:23;
    int8u dpb_output_delay_length_minus1=((*seq_parameter_set_Item)->vui_parameters && (*seq_parameter_set_Item)->vui_parameters->xxL_Common)?(*seq_parameter_set_Item)->vui_parameters->xxL_Common->dpb_output_delay_length_minus1:23;
    if (irap_cpb_params_present_flag)
    {
         Skip_S4(au_cpb_removal_delay_length_minus1+1,          "cpb_delay_offset");
         Skip_S4(dpb_output_delay_length_minus1+1,              "dpb_delay_offset");
    }
    Skip_SB(                                                    "concatenation_flag");
    Skip_S4(au_cpb_removal_delay_length_minus1+1,               "au_cpb_removal_delay_delta_minus1");
    if ((*seq_parameter_set_Item)->NalHrdBpPresentFlag())
        sei_message_buffering_period_xxl((*seq_parameter_set_Item)->vui_parameters?(*seq_parameter_set_Item)->vui_parameters->xxL_Common:NULL, irap_cpb_params_present_flag, (*seq_parameter_set_Item)->vui_parameters->NAL);
    if ((*seq_parameter_set_Item)->VclHrdBpPresentFlag())
        sei_message_buffering_period_xxl((*seq_parameter_set_Item)->vui_parameters?(*seq_parameter_set_Item)->vui_parameters->xxL_Common:NULL, irap_cpb_params_present_flag, (*seq_parameter_set_Item)->vui_parameters->VCL);
    BS_End();
}

void File_Hevc::sei_message_buffering_period_xxl(seq_parameter_set_struct::vui_parameters_struct::xxl_common* xxL_Common, bool irap_cpb_params_present_flag, seq_parameter_set_struct::vui_parameters_struct::xxl* xxl)
{
    if (xxL_Common==NULL || xxl==NULL)
    {
        //Problem?
        Skip_BS(Data_BS_Remain(),                               "Problem?");
        return;
    }
    for (int32u SchedSelIdx=0; SchedSelIdx<xxl->SchedSel.size(); SchedSelIdx++)
    {
        //Get_S4 (xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay_length_minus1+1, xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay, "initial_cpb_removal_delay"); Param_Info2(xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay/90, " ms");
        //Get_S4 (xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay_length_minus1+1, xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay_offset, "initial_cpb_removal_delay_offset"); Param_Info2(xxl->SchedSel[SchedSelIdx].initial_cpb_removal_delay_offset/90, " ms");
        Info_S4 (xxL_Common->initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay, "initial_cpb_removal_delay"); Param_Info2(initial_cpb_removal_delay/90, " ms");
        Info_S4 (xxL_Common->initial_cpb_removal_delay_length_minus1+1, initial_cpb_removal_delay_offset, "initial_cpb_removal_delay_offset"); Param_Info2(initial_cpb_removal_delay_offset/90, " ms");
        if (xxL_Common->sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
        {
            Info_S4 (xxL_Common->initial_cpb_removal_delay_length_minus1+1, initial_alt_cpb_removal_delay, "initial_alt_cpb_removal_delay"); Param_Info2(initial_alt_cpb_removal_delay/90, " ms");
            Info_S4 (xxL_Common->initial_cpb_removal_delay_length_minus1+1, initial_alt_cpb_removal_delay_offset, "initial_alt_cpb_removal_delay_offset"); Param_Info2(initial_alt_cpb_removal_delay_offset/90, " ms");
        }
    }
}

//---------------------------------------------------------------------------
// SEI - 1
void File_Hevc::sei_message_pic_timing(int32u &seq_parameter_set_id, int32u payloadSize)
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
    BS_Begin();
    if ((*seq_parameter_set_Item)->vui_parameters?(*seq_parameter_set_Item)->vui_parameters->frame_field_info_present_flag:((*seq_parameter_set_Item)->general_progressive_source_flag && (*seq_parameter_set_Item)->general_interlaced_source_flag))
    {
        Skip_S1(4,                                              "pic_struct");
        Skip_S1(2,                                              "source_scan_type");
        Skip_SB(                                                "duplicate_flag");
    }
    if ((*seq_parameter_set_Item)->CpbDpbDelaysPresentFlag())
    {
        int8u au_cpb_removal_delay_length_minus1=(*seq_parameter_set_Item)->vui_parameters->xxL_Common->au_cpb_removal_delay_length_minus1;
        int8u dpb_output_delay_length_minus1=(*seq_parameter_set_Item)->vui_parameters->xxL_Common->dpb_output_delay_length_minus1;
        bool  sub_pic_hrd_params_present_flag=(*seq_parameter_set_Item)->vui_parameters->xxL_Common->sub_pic_hrd_params_present_flag;
        Skip_S4(au_cpb_removal_delay_length_minus1+1,           "au_cpb_removal_delay_minus1");
        Skip_S4(dpb_output_delay_length_minus1+1,               "pic_dpb_output_delay");
        if (sub_pic_hrd_params_present_flag)
        {
            int8u dpb_output_delay_du_length_minus1=(*seq_parameter_set_Item)->vui_parameters->xxL_Common->dpb_output_delay_du_length_minus1;
            Skip_S4(dpb_output_delay_du_length_minus1+1,        "pic_dpb_output_du_delay");
        }
    }
    BS_End();
}

//---------------------------------------------------------------------------
// SEI - 5
void File_Hevc::sei_message_user_data_unregistered(int32u payloadSize)
{
    Element_Info1("user_data_unregistered");

    //Parsing
    int128u uuid_iso_iec_11578;
    Get_GUID(uuid_iso_iec_11578,                                "uuid_iso_iec_11578");

    switch (uuid_iso_iec_11578.hi)
    {
        case 0x214892b89bCC7f42LL : Element_Info1("Ateme");
                                     sei_message_user_data_unregistered_Ateme(payloadSize-16); break;
        case 0xDB4717b509DEA22CLL : Element_Info1("x265");
                                     sei_message_user_data_unregistered_x265(payloadSize-16); break;
        default :
                    Element_Info1("unknown");
                    Skip_XX(payloadSize-16,                     "data");
    }
}

//---------------------------------------------------------------------------
// SEI - 5 - Ateme
void File_Hevc::sei_message_user_data_unregistered_Ateme(int32u payloadSize)
{
    //Parsing
    Get_Local(payloadSize, Encoded_Library,                     "Library name");

    //Encoded_Library
    if (Encoded_Library.find(__T("ATEME "))==0)
    {
        size_t Pos=Encoded_Library.find_first_of(__T("0123456789"));
        if (Pos && Encoded_Library[Pos-1]==__T(' '))
        {
            Encoded_Library_Name=Encoded_Library.substr(0, Pos-1);
            Encoded_Library_Version=Encoded_Library.substr(Pos);
        }
    }
}

//---------------------------------------------------------------------------
// SEI - 5 - x265
void File_Hevc::sei_message_user_data_unregistered_x265(int32u payloadSize)
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
            while (Options_Pos_Before!=Data.size())
            {
                size_t Options_Pos=Data.find(__T(" "), Options_Pos_Before);
                if (Options_Pos==std::string::npos)
                    Options_Pos=Data.size();
                Ztring option;
                Get_Local (Options_Pos-Options_Pos_Before, option, "option");
                Options_Pos_Before=Options_Pos;
                while (Options_Pos_Before!=Data.size())
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

                //Filling
                if (option!=__T("options:") && !(!option.empty() && option[0]>=__T('0') && option[0]<=__T('9')) && option.find(__T("fps="))!=0 && option.find(__T("bitdepth="))!=0) //Ignoring redundant information e.g. width, height, frame rate, bit depth
                {
                    if (!Encoded_Library_Settings.empty())
                        Encoded_Library_Settings+=__T(" / ");
                    Encoded_Library_Settings+=option;
                }
            }
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
                size_t Value_Pos=Value.find(__T(" "));
                if (Value_Pos!=string::npos)
                    Value.resize(Value_Pos);
                Encoded_Library=Value;
            }
            if (Loop==1 && Encoded_Library.find(__T("x265"))==0)
            {
                size_t Value_Pos=Value.find(__T(" 8bpp"));
                if (Value_Pos!=string::npos)
                    Value.resize(Value_Pos);

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
    if (Encoded_Library.find(__T("x265 - "))==0)
    {
        Encoded_Library_Name=__T("x265");
        Encoded_Library_Version=Encoded_Library.SubString(__T("x265 - "), Ztring());
    }
    else
        Encoded_Library_Name=Encoded_Library;
}

//---------------------------------------------------------------------------
void File_Hevc::sei_message_active_parameter_sets()
{
    Element_Info1("active_parameter_sets");

    //Parsing
    int32u num_sps_ids_minus1;
    BS_Begin();
    Skip_S1(4,                                                  "active_video_parameter_set_id");
    Skip_SB(                                                    "self_contained_cvs_flag");
    Skip_SB(                                                    "no_parameter_set_update_flag");
    Get_UE (   num_sps_ids_minus1,                              "num_sps_ids_minus1");
    for (int32u i=0; i<=num_sps_ids_minus1; ++i)
    {
        Skip_UE(                                                "active_seq_parameter_set_id");
    }
    BS_End();
}

//---------------------------------------------------------------------------
void File_Hevc::sei_message_decoded_picture_hash(int32u payloadSize)
{
    Element_Info1("decoded_picture_hash");

    //Parsing
    int8u hash_type;
    Get_B1 (hash_type,                                          "hash_type");
    for (int8u cIdx=0; cIdx<(chroma_format_idc?3:1); cIdx++)
        switch (hash_type)
        {
            case 0 :    // md5
                        Skip_XX(16,                             "md5");
                        break;
            case 1 :    // crc
                        Skip_XX( 2,                             "crc");
                        break;
            case 2 :    // checksum
                        Skip_XX( 4,                             "checksum");
                        break;
            default :   //
                        Skip_XX((Element_Size-1)/(chroma_format_idc?1:3), "unknown");
                        break;
        }
}

//***************************************************************************
// Sub-elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Hevc::slice_segment_header()
{
    Element_Begin1("slice_segment_header");

    //Parsing
    bool    dependent_slice_segment_flag=false;
    Get_SB (   first_slice_segment_in_pic_flag,                 "first_slice_segment_in_pic_flag");
    if (RapPicFlag)
        Skip_SB(                                                "no_output_of_prior_pics_flag");
    Get_UE (   slice_pic_parameter_set_id,                      "slice_pic_parameter_set_id");
    std::vector<pic_parameter_set_struct*>::iterator pic_parameter_set_Item;
    if (slice_pic_parameter_set_id>=pic_parameter_sets.size() || (*(pic_parameter_set_Item=pic_parameter_sets.begin()+slice_pic_parameter_set_id))==NULL)
    {
        //Not yet present
        Skip_BS(Data_BS_Remain(),                               "Data (pic_parameter_set is missing)");
        Element_End0();
        slice_pic_parameter_set_id=(int32u)-1;
        slice_type=(int32u)-1;
        return;
    }
    if (!first_slice_segment_in_pic_flag)
    {
        if (!MustParse_VPS_SPS_PPS_FromFlv && (*pic_parameter_set_Item)->dependent_slice_segments_enabled_flag)
            Get_SB (dependent_slice_segment_flag,               "dependent_slice_segment_flag");
        //Skip_BS(Ceil( Log2( PicSizeInCtbsY ) ),               "slice_segment_address");
        Skip_BS(Data_BS_Remain(),                               "(ToDo)");
        Element_End0();
        slice_type=(int32u)-1;
        return;
    }
    if (!dependent_slice_segment_flag)
    {
        if (!MustParse_VPS_SPS_PPS_FromFlv)
            Skip_S1((*pic_parameter_set_Item)->num_extra_slice_header_bits, "slice_reserved_flags");
        Get_UE (slice_type,                                     "slice_type"); Param_Info1(Hevc_slice_type(slice_type));
    }
    //TODO...
    Skip_BS(Data_BS_Remain(),                                   "(ToDo)");

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Hevc::profile_tier_level(int8u maxNumSubLayersMinus1)
{
    Element_Begin1("profile_tier_level");

    //Parsing
    std::vector<bool>sub_layer_profile_present_flags, sub_layer_level_present_flags;
    Get_S1 (2,  profile_space,                                  "general_profile_space");
    Get_SB (    tier_flag,                                      "general_tier_flag");
    Get_S1 (5,  profile_idc,                                    "general_profile_idc");
    Element_Begin1("general_profile_compatibility_flags");
        for (int8u profile_pos=0; profile_pos<32; profile_pos++)
            if (profile_pos==profile_idc)
            {
                bool general_profile_compatibility_flag;
                Get_SB (    general_profile_compatibility_flag, "general_profile_compatibility_flag");
                //if (!general_profile_compatibility_flag && !profile_space)  //found some files without this flag, ignoring the test for the moment (not really important)
                //    Trusted_IsNot("general_profile_compatibility_flag not valid");
            }
            else
                Skip_SB(                                        "general_profile_compatibility_flag");
    Element_End0();
    Get_SB (    general_progressive_source_flag,                "general_progressive_source_flag");
    Get_SB (    general_interlaced_source_flag,                 "general_interlaced_source_flag");
    Skip_SB(                                                    "general_non_packed_constraint_flag");
    Get_SB (    general_frame_only_constraint_flag,             "general_frame_only_constraint_flag");
    Skip_S8(44,                                                 "general_reserved_zero_44bits");
    Get_S1 (8,  level_idc,                                      "general_level_idc");
    for (int32u SubLayerPos=0; SubLayerPos<maxNumSubLayersMinus1; SubLayerPos++)
    {
        Element_Begin1("SubLayer");
        bool sub_layer_profile_present_flag, sub_layer_level_present_flag;
        Get_SB (   sub_layer_profile_present_flag,              "sub_layer_profile_present_flag");
        Get_SB (   sub_layer_level_present_flag,                "sub_layer_level_present_flag");
        sub_layer_profile_present_flags.push_back(sub_layer_profile_present_flag);
        sub_layer_level_present_flags.push_back(sub_layer_level_present_flag);
        Element_End0();
    }
    if (maxNumSubLayersMinus1)
        for(int32u SubLayerPos=maxNumSubLayersMinus1; SubLayerPos<8; SubLayerPos++)
            Skip_S1(2,                                          "reserved_zero_2bits");
    for (int32u SubLayerPos=0; SubLayerPos<maxNumSubLayersMinus1; SubLayerPos++)
    {
        Element_Begin1("SubLayer");
        if (sub_layer_profile_present_flags[SubLayerPos])
        {
            Skip_S1(2,                                          "sub_layer_profile_space");
            Skip_SB(                                            "sub_layer_tier_flag");
            Skip_S1(5,                                          "sub_layer_profile_idc");
            Skip_S4(32,                                         "sub_layer_profile_compatibility_flags");
            Skip_SB(                                            "sub_layer_progressive_source_flag");
            Skip_SB(                                            "sub_layer_interlaced_source_flag");
            Skip_SB(                                            "sub_layer_non_packed_constraint_flag");
            Skip_SB(                                            "sub_layer_frame_only_constraint_flag");
            Skip_S8(44,                                         "sub_layer_reserved_zero_44bits");
        }
        if (sub_layer_level_present_flags[SubLayerPos])
        {
            Skip_S1(8,                                          "sub_layer_level_idc");
        }
        Element_End0();
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Hevc::short_term_ref_pic_sets(int8u num_short_term_ref_pic_sets)
{
    Element_Begin1("short_term_ref_pic_sets");
    int32u num_pics=0;

    for (int32u stRpsIdx=0; stRpsIdx<num_short_term_ref_pic_sets; stRpsIdx++)
    {
        Element_Begin1("short_term_ref_pic_set");
        bool inter_ref_pic_set_prediction_flag=false;
        if (stRpsIdx)
            Get_SB (inter_ref_pic_set_prediction_flag,              "inter_ref_pic_set_prediction_flag");
        if (inter_ref_pic_set_prediction_flag)
        {
            int32u  delta_idx_minus1=0, abs_delta_rps_minus1;
            bool    delta_rps_sign;
            if (stRpsIdx==num_short_term_ref_pic_sets)
                Get_UE (delta_idx_minus1,                           "delta_idx_minus1");
            if (delta_idx_minus1+1>stRpsIdx)
            {
                Skip_BS(Data_BS_Remain(),                           "(Problem)");
                Element_End0();
                Element_End0();
                return;
            }
            Get_SB (   delta_rps_sign,                              "delta_rps_sign");
            Get_UE (   abs_delta_rps_minus1,                        "abs_delta_rps_minus1");
            int32u num_pics_new=0;
            for(int32u pic_pos=0 ; pic_pos<=num_pics; pic_pos++)
            {
                TESTELSE_SB_SKIP(                                   "used_by_curr_pic_flag");
                    num_pics_new++;
                TESTELSE_SB_ELSE(                                   "used_by_curr_pic_flag");
                    bool use_delta_flag;
                    Get_SB (use_delta_flag,                         "use_delta_flag");
                    if (use_delta_flag)
                        num_pics_new++;
                TESTELSE_SB_END();
            }
            num_pics=num_pics_new;
        }
        else
        {
            int32u num_negative_pics, num_positive_pics;
            Get_UE (num_negative_pics,                              "num_negative_pics");
            Get_UE (num_positive_pics,                              "num_positive_pics");
            num_pics=num_negative_pics+num_positive_pics;
            for (int32u i=0; i<num_negative_pics; i++)
            {
                Skip_UE(                                            "delta_poc_s0_minus1");
                Skip_SB(                                            "used_by_curr_pic_s0_flag");
            }
            for (int32u i=0; i<num_positive_pics; i++)
            {
                Skip_UE(                                            "delta_poc_s1_minus1");
                Skip_SB(                                            "used_by_curr_pic_s1_flag");
            }
        }
        Element_End0();
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Hevc::vui_parameters(std::vector<video_parameter_set_struct*>::iterator video_parameter_set_Item, seq_parameter_set_struct::vui_parameters_struct* &vui_parameters_Item_)
{
    //Parsing
    seq_parameter_set_struct::vui_parameters_struct::xxl_common *xxL_Common=NULL;
    seq_parameter_set_struct::vui_parameters_struct::xxl *NAL = NULL, *VCL = NULL;
    int32u  num_units_in_tick = (int32u)-1, time_scale = (int32u)-1;
    int16u  sar_width=(int16u)-1, sar_height=(int16u)-1;
    int8u   aspect_ratio_idc=0, video_format=5, video_full_range_flag=0, colour_primaries=2, transfer_characteristics=2, matrix_coefficients=2;
    bool    aspect_ratio_info_present_flag, video_signal_type_present_flag, frame_field_info_present_flag, colour_description_present_flag=false, timing_info_present_flag;
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
    Skip_SB(                                                    "neutral_chroma_indication_flag");
    Skip_SB(                                                    "field_seq_flag");
    Get_SB (   frame_field_info_present_flag,                   "frame_field_info_present_flag");
    TEST_SB_SKIP(                                               "default_display_window_flag ");
        Skip_UE(                                                "def_disp_win_left_offset");
        Skip_UE(                                                "def_disp_win_right_offset");
        Skip_UE(                                                "def_disp_win_top_offset");
        Skip_UE(                                                "def_disp_win_bottom_offset");
    TEST_SB_END();
    TEST_SB_GET (timing_info_present_flag,                      "timing_info_present_flag");
        Get_S4 (32, num_units_in_tick,                          "num_units_in_tick");
        Get_S4 (32, time_scale,                                 "time_scale");
        TEST_SB_SKIP(                                           "vui_poc_proportional_to_timing_flag");
            Skip_UE(                                            "vui_num_ticks_poc_diff_one_minus1");
        TEST_SB_END();
        TEST_SB_SKIP(                                           "hrd_parameters_present_flag");
            hrd_parameters(true, (*video_parameter_set_Item)->vps_max_sub_layers_minus1, xxL_Common, NAL, VCL);
        TEST_SB_END();
    TEST_SB_END();
    TEST_SB_SKIP(                                               "bitstream_restriction_flag");
        Skip_SB(                                                "tiles_fixed_structure_flag");
        Skip_SB(                                                "motion_vectors_over_pic_boundaries_flag");
        Skip_SB(                                                "restricted_ref_pic_lists_flag");
        Skip_UE(                                                "min_spatial_segmentation_idc");
        Skip_UE(                                                "max_bytes_per_pic_denom");
        Skip_UE(                                                "max_bits_per_min_cu_denom");
        Skip_UE(                                                "log2_max_mv_length_horizontal");
        Skip_UE(                                                "log2_max_mv_length_vertical");
    TEST_SB_END();

    FILLING_BEGIN();
        vui_parameters_Item_ = new seq_parameter_set_struct::vui_parameters_struct(
                                                                                    NAL,
                                                                                    VCL,
                                                                                    xxL_Common,
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
                                                                                    frame_field_info_present_flag,
                                                                                    colour_description_present_flag,
                                                                                    timing_info_present_flag
                                                                                  );
    FILLING_ELSE();
    delete xxL_Common;
    delete NAL;
    delete VCL;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Hevc::hrd_parameters(bool commonInfPresentFlag, int8u maxNumSubLayersMinus1, seq_parameter_set_struct::vui_parameters_struct::xxl_common* &xxL_Common, seq_parameter_set_struct::vui_parameters_struct::xxl* &NAL, seq_parameter_set_struct::vui_parameters_struct::xxl* &VCL)
{
    //Parsing
    int8u bit_rate_scale=0, cpb_size_scale=0, du_cpb_removal_delay_increment_length_minus1=0, dpb_output_delay_du_length_minus1=0, initial_cpb_removal_delay_length_minus1=0, au_cpb_removal_delay_length_minus1=0, dpb_output_delay_length_minus1=0;
    bool nal_hrd_parameters_present_flag=false, vcl_hrd_parameters_present_flag=false, sub_pic_hrd_params_present_flag=false;
    if (commonInfPresentFlag)
    {
        Get_SB (nal_hrd_parameters_present_flag,                "nal_hrd_parameters_present_flag");
        Get_SB (vcl_hrd_parameters_present_flag,                "vcl_hrd_parameters_present_flag");
        if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
        {
            TEST_SB_GET (sub_pic_hrd_params_present_flag,       "sub_pic_hrd_params_present_flag");
                Skip_S1(8,                                      "tick_divisor_minus2");
                Get_S1 (5, du_cpb_removal_delay_increment_length_minus1,  "du_cpb_removal_delay_increment_length_minus1");
                Skip_SB(                                        "sub_pic_cpb_params_in_pic_timing_sei_flag");
                Get_S1 (5, dpb_output_delay_du_length_minus1,   "dpb_output_delay_du_length_minus1");
            TEST_SB_END();
            Get_S1 (4, bit_rate_scale,                          "bit_rate_scale");
            Get_S1 (4, cpb_size_scale,                          "cpb_size_scale");
            if (sub_pic_hrd_params_present_flag)
                Skip_S1(4,                                      "cpb_size_du_scale");
            Get_S1 (5, initial_cpb_removal_delay_length_minus1, "initial_cpb_removal_delay_length_minus1");
            Get_S1 (5, au_cpb_removal_delay_length_minus1,      "au_cpb_removal_delay_length_minus1");
            Get_S1 (5, dpb_output_delay_length_minus1,          "dpb_output_delay_length_minus1");
        }
    }

    for (int8u NumSubLayer=0; NumSubLayer<=maxNumSubLayersMinus1; NumSubLayer++)
    {
        int32u cpb_cnt_minus1=0;
        bool fixed_pic_rate_general_flag, fixed_pic_rate_within_cvs_flag=true, low_delay_hrd_flag=false;
        Get_SB (fixed_pic_rate_general_flag,                   "fixed_pic_rate_general_flag");
        if (!fixed_pic_rate_general_flag)
            Get_SB (fixed_pic_rate_within_cvs_flag,            "fixed_pic_rate_within_cvs_flag");
        if (fixed_pic_rate_within_cvs_flag)
            Skip_UE(                                           "elemental_duration_in_tc_minus1");
        else
            Get_SB (low_delay_hrd_flag,                        "low_delay_hrd_flag");
        if (!low_delay_hrd_flag)
        {
            Get_UE (cpb_cnt_minus1,                            "cpb_cnt_minus1");
            if (cpb_cnt_minus1>31)
            {
                Trusted_IsNot("cpb_cnt_minus1 too high");
                cpb_cnt_minus1=0;
                return;
            }
        }
        if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
            xxL_Common=new seq_parameter_set_struct::vui_parameters_struct::xxl_common(
                                                                                        sub_pic_hrd_params_present_flag,
                                                                                        du_cpb_removal_delay_increment_length_minus1,
                                                                                        dpb_output_delay_du_length_minus1,
                                                                                        initial_cpb_removal_delay_length_minus1,
                                                                                        au_cpb_removal_delay_length_minus1,
                                                                                        dpb_output_delay_length_minus1
                                                                                      );
        if (nal_hrd_parameters_present_flag)
            sub_layer_hrd_parameters(xxL_Common, bit_rate_scale, cpb_size_scale, cpb_cnt_minus1, NAL); //TODO: save HRD per NumSubLayer
        if (vcl_hrd_parameters_present_flag)
            sub_layer_hrd_parameters(xxL_Common, bit_rate_scale, cpb_size_scale, cpb_cnt_minus1, VCL); //TODO: save HRD per NumSubLayer
    }
}

//---------------------------------------------------------------------------
void File_Hevc::sub_layer_hrd_parameters(seq_parameter_set_struct::vui_parameters_struct::xxl_common* xxL_Common, int8u bit_rate_scale, int8u cpb_size_scale, int32u cpb_cnt_minus1, seq_parameter_set_struct::vui_parameters_struct::xxl* &hrd_parameters_Item_)
{
    //Parsing
    vector<seq_parameter_set_struct::vui_parameters_struct::xxl::xxl_data>  SchedSel;
    SchedSel.reserve(cpb_cnt_minus1 + 1);
    for (int8u SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; ++SchedSelIdx)
    {
        Element_Begin1("ShedSel");
        int64u bit_rate_value, cpb_size_value;
        int32u bit_rate_value_minus1, cpb_size_value_minus1;
        bool cbr_flag;
        Get_UE (bit_rate_value_minus1,                          "bit_rate_value_minus1");
        bit_rate_value = (int64u)((bit_rate_value_minus1 + 1)*pow(2.0, 6 + bit_rate_scale)); Param_Info2(bit_rate_value, " bps");
        Get_UE (cpb_size_value_minus1,                          "cpb_size_value_minus1");
        cpb_size_value = (int64u)((cpb_size_value_minus1 + 1)*pow(2.0, 4 + cpb_size_scale)); Param_Info2(cpb_size_value, " bits");
        if (xxL_Common->sub_pic_hrd_params_present_flag)
        {
            Skip_UE(                                            "cpb_size_du_value_minus1");
            Skip_UE(                                            "bit_rate_du_value_minus1");
        }
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

    //Validity test
    if (!Element_IsOK() || (SchedSel.size() == 1 && SchedSel[0].bit_rate_value == 64))
    {
        return; //We do not trust this kind of data
    }

    //Filling
    hrd_parameters_Item_=new seq_parameter_set_struct::vui_parameters_struct::xxl(
                                                                                    SchedSel
                                                                                 );
}

//---------------------------------------------------------------------------
void File_Hevc::scaling_list_data()
{
    for(int8u sizeId=0; sizeId<4; sizeId++)
        for (int8u matrixId = 0; matrixId<((sizeId == 3) ? 2 : 6); matrixId++)
        {
            bool scaling_list_pred_mode_flag;
            Get_SB (scaling_list_pred_mode_flag,                "scaling_list_pred_mode_flag");
            if(!scaling_list_pred_mode_flag)
                Skip_UE(                                        "scaling_list_pred_matrix_id_delta");
            else
            {
                //nextCoef = 8
                size_t coefNum=std::min(64, (1<<(4+(sizeId<<1))));
                if( sizeId > 1 )
                {
                    Skip_SE(                                    "scaling_list_dc_coef_minus8"); //[ sizeId ? 2 ][ matrixId ] se(v)
                    //nextCoef = scaling_list_dc_coef_minus8[ sizeId ? 2 ][ matrixId ] + 8
                }
                for(size_t i=0; i<coefNum; i++)
                {
                    Skip_SE(                                    "scaling_list_delta_coef");
                    //nextCoef = ( nextCoef + scaling_list_delta_coef + 256 ) % 256
                    //ScalingList[ sizeId ][ matrixId ][ i ] = nextCoef
                }
            }
        }
}

//***************************************************************************
// Specific
//***************************************************************************

//---------------------------------------------------------------------------
void File_Hevc::VPS_SPS_PPS()
{
    if (MustParse_VPS_SPS_PPS_FromMatroska || MustParse_VPS_SPS_PPS_FromFlv)
    {
        if (Element_Size>=5
         && Buffer[Buffer_Offset  ]==0x01
         && Buffer[Buffer_Offset+1]==0x00
         && Buffer[Buffer_Offset+2]==0x00
         && Buffer[Buffer_Offset+3]==0x00
         && Buffer[Buffer_Offset+4]==0xFF) //Trying to detect old proposal of the form of Matroska implementation
            return VPS_SPS_PPS_FromMatroska();

        MustParse_VPS_SPS_PPS_FromMatroska=false;
        MustParse_VPS_SPS_PPS_FromFlv=false;
    }

    //Parsing
    int64u general_constraint_indicator_flags;
    int32u general_profile_compatibility_flags;
    int8u  configurationVersion;
    int8u  chromaFormat, bitDepthLumaMinus8, bitDepthChromaMinus8;
    int8u  general_profile_space, general_profile_idc, general_level_idc;
    int8u  numOfArrays, constantFrameRate, numTemporalLayers;
    bool   general_tier_flag, temporalIdNested;
    Get_B1 (configurationVersion,                               "configurationVersion");
    BS_Begin();
        Get_S1 (2, general_profile_space,                       "general_profile_space");
        Get_SB (   general_tier_flag,                           "general_tier_flag");
        Get_S1 (5, general_profile_idc,                         "general_profile_idc");
    BS_End();
    Get_B4 (general_profile_compatibility_flags,                "general_profile_compatibility_flags");
    Get_B6 (general_constraint_indicator_flags,                 "general_constraint_indicator_flags");
    Get_B1 (general_level_idc,                                  "general_level_idc");
    BS_Begin();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Skip_S2(12,                                             "min_spatial_segmentation_idc");
    BS_End();
    BS_Begin();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Skip_S1(2,                                              "parallelismType");
    BS_End();
    BS_Begin();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Get_S1 (2, chromaFormat,                                "chromaFormat");
    BS_End();
    BS_Begin();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Get_S1 (3, bitDepthLumaMinus8,                          "bitDepthLumaMinus8");
    BS_End();
    BS_Begin();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Mark_1_NoTrustError();
        Get_S1 (3, bitDepthChromaMinus8,                        "bitDepthChromaMinus8");
    BS_End();
    Skip_B2(                                                    "avgFrameRate");
    BS_Begin();
        Get_S1 (2, constantFrameRate,                           "constantFrameRate");
        Get_S1 (3, numTemporalLayers,                           "numTemporalLayers");
        Get_SB (   temporalIdNested,                            "temporalIdNested");
        Get_S1 (2, lengthSizeMinusOne,                          "lengthSizeMinusOne");
    BS_End();
    Get_B1 (numOfArrays,                                        "numOfArrays");
    for (size_t ArrayPos=0; ArrayPos<numOfArrays; ArrayPos++)
    {
        Element_Begin1("Array");
        int16u numNalus;
        int8u NAL_unit_type;
        BS_Begin();
            Skip_SB(                                            "array_completeness");
            Mark_0_NoTrustError();
            Get_S1 (6, NAL_unit_type,                           "NAL_unit_type");
        BS_End();
        Get_B2 (numNalus,                                       "numNalus");
        for (size_t NaluPos=0; NaluPos<numNalus; NaluPos++)
        {
            Element_Begin1("nalUnit");
            int16u nalUnitLength;
            Get_B2 (nalUnitLength,                              "nalUnitLength");
            if (nalUnitLength<2 || Element_Offset+nalUnitLength>Element_Size)
            {
                Trusted_IsNot("Size is wrong");
                break; //There is an error
            }

            //Header
            int8u nal_unit_type, nuh_temporal_id_plus1;
            BS_Begin();
            Mark_0 ();
            Get_S1 (6, nal_unit_type,                           "nal_unit_type");
            Get_S1 (6, nuh_layer_id,                            "nuh_layer_id");
            Get_S1 (3, nuh_temporal_id_plus1,                   "nuh_temporal_id_plus1");
            if (nuh_temporal_id_plus1==0)
                Trusted_IsNot("nuh_temporal_id_plus1 is invalid");
            BS_End();

            //Data
            int64u Element_Offset_Save=Element_Offset;
            int64u Element_Size_Save=Element_Size;
            Buffer_Offset+=(size_t)Element_Offset_Save;
            Element_Offset=0;
            Element_Size=nalUnitLength-2;
            Element_Code=nal_unit_type;
            Data_Parse();
            Buffer_Offset-=(size_t)Element_Offset_Save;
            Element_Offset=Element_Offset_Save+nalUnitLength-2;
            Element_Size=Element_Size_Save;

            Element_End0();
        }
        Element_End0();
    }

    MustParse_VPS_SPS_PPS=false;
    FILLING_BEGIN_PRECISE();
        Accept("HEVC");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Hevc::VPS_SPS_PPS_FromMatroska()
{
    //Parsing
    int8u Profile, Level, vid_parameter_set_count, seq_parameter_set_count, pic_parameter_set_count;
    if (SizedBlocks)
        Skip_B1(                                                "Version");
    Get_B1 (Profile,                                            "Profile");
    Skip_B1(                                                    "Compatible profile");
    Get_B1 (Level,                                              "Level");
    BS_Begin();
    Skip_S1(6,                                                  "Reserved");
    Get_S1 (2, lengthSizeMinusOne,                              "Size of NALU length minus 1");
    Skip_S1(3,                                                  "Reserved");
    Get_S1 (5, vid_parameter_set_count,                         MustParse_VPS_SPS_PPS_FromFlv?"vid_parameter_set+seq_parameter_set count":"vid_parameter_set count");
    BS_End();
    for (int8u Pos=0; Pos<vid_parameter_set_count; Pos++)
    {
        Element_Begin1("nalUnit");
        int16u nalUnitLength;
        Get_B2 (nalUnitLength,                              "nalUnitLength");
        if (nalUnitLength<2 || Element_Offset+nalUnitLength>Element_Size)
        {
            Trusted_IsNot("Size is wrong");
            break; //There is an error
        }

        //Header
        int8u nal_unit_type, nuh_temporal_id_plus1;
        BS_Begin();
        Mark_0 ();
        Get_S1 (6, nal_unit_type,                           "nal_unit_type");
        Get_S1 (6, nuh_layer_id,                            "nuh_layer_id");
        Get_S1 (3, nuh_temporal_id_plus1,                   "nuh_temporal_id_plus1");
        if (nuh_temporal_id_plus1==0)
            Trusted_IsNot("nuh_temporal_id_plus1 is invalid");
        BS_End();

        //Data
        int64u Element_Offset_Save=Element_Offset;
        int64u Element_Size_Save=Element_Size;
        Buffer_Offset+=(size_t)Element_Offset_Save;
        Element_Offset=0;
        Element_Size=nalUnitLength-2;
        Element_Code=nal_unit_type;
        Data_Parse();
        Buffer_Offset-=(size_t)Element_Offset_Save;
        Element_Offset=Element_Offset_Save+nalUnitLength-2;
        Element_Size=Element_Size_Save;

        Element_End0();
    }
    if (MustParse_VPS_SPS_PPS_FromFlv)
        seq_parameter_set_count=0;
    else
    {
        BS_Begin();
        Skip_S1(3,                                              "Reserved");
        Get_S1 (5, seq_parameter_set_count,                     "seq_parameter_set count");
        BS_End();
    }
    for (int8u Pos=0; Pos<seq_parameter_set_count; Pos++)
    {
        Element_Begin1("nalUnit");
        int16u nalUnitLength;
        Get_B2 (nalUnitLength,                              "nalUnitLength");
        if (nalUnitLength<2 || Element_Offset+nalUnitLength>Element_Size)
        {
            Trusted_IsNot("Size is wrong");
            break; //There is an error
        }

        //Header
        int8u nal_unit_type, nuh_temporal_id_plus1;
        BS_Begin();
        Mark_0 ();
        Get_S1 (6, nal_unit_type,                           "nal_unit_type");
        Get_S1 (6, nuh_layer_id,                            "nuh_layer_id");
        Get_S1 (3, nuh_temporal_id_plus1,                   "nuh_temporal_id_plus1");
        if (nuh_temporal_id_plus1==0)
            Trusted_IsNot("nuh_temporal_id_plus1 is invalid");
        BS_End();

        //Data
        int64u Element_Offset_Save=Element_Offset;
        int64u Element_Size_Save=Element_Size;
        Buffer_Offset+=(size_t)Element_Offset_Save;
        Element_Offset=0;
        Element_Size=nalUnitLength-2;
        Element_Code=nal_unit_type;
        Data_Parse();
        Buffer_Offset-=(size_t)Element_Offset_Save;
        Element_Offset=Element_Offset_Save+nalUnitLength-2;
        Element_Size=Element_Size_Save;

        Element_End0();
    }
    Get_B1 (pic_parameter_set_count,                            "pic_parameter_set count");
    for (int8u Pos=0; Pos<pic_parameter_set_count; Pos++)
    {
        Element_Begin1("nalUnit");
        int16u nalUnitLength;
        Get_B2 (nalUnitLength,                              "nalUnitLength");
        if (nalUnitLength<2 || Element_Offset+nalUnitLength>Element_Size)
        {
            Trusted_IsNot("Size is wrong");
            break; //There is an error
        }

        //Header
        int8u nal_unit_type, nuh_temporal_id_plus1;
        BS_Begin();
        Mark_0 ();
        Get_S1 (6, nal_unit_type,                           "nal_unit_type");
        Get_S1 (6, nuh_layer_id,                            "nuh_layer_id");
        Get_S1 (3, nuh_temporal_id_plus1,                   "nuh_temporal_id_plus1");
        if (nuh_temporal_id_plus1==0)
            Trusted_IsNot("nuh_temporal_id_plus1 is invalid");
        BS_End();

        //Data
        int64u Element_Offset_Save=Element_Offset;
        int64u Element_Size_Save=Element_Size;
        Buffer_Offset+=(size_t)Element_Offset_Save;
        Element_Offset=0;
        Element_Size=nalUnitLength-2;
        Element_Code=nal_unit_type;
        Data_Parse();
        Buffer_Offset-=(size_t)Element_Offset_Save;
        Element_Offset=Element_Offset_Save+nalUnitLength-2;
        Element_Size=Element_Size_Save;

        Element_End0();
    }
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding?");

    //Filling
    MustParse_VPS_SPS_PPS=false;
    FILLING_BEGIN_PRECISE();
        Accept("HEVC");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_HEVC_YES
