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

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AAC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aac.h"
#include "MediaInfo/Audio/File_Aac_GeneralAudio.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
extern const int32u Aac_sampling_frequency[];
extern const char* Aac_audioObjectType(int8u audioObjectType);
extern const char* Aac_Format_Profile(int8u ID);

//---------------------------------------------------------------------------
const char* Aac_id_syn_ele[8]=
{
    "SCE - single_channel_element",
    "CPE - channel_pair_element",
    "CCE - coupling_channel_element",
    "LFE - lfe_channel_element",
    "DSE - data_stream_element",
    "PCE - program_config_element",
    "FIL - fill_element",
    "END - End"
};

//---------------------------------------------------------------------------
const char* Aac_window_sequence[4]=
{
    "ONLY_LONG_SEQUENCE",
    "LONG_START_SEQUENCE",
    "EIGHT_SHORT_SEQUENCE",
    "LONG_STOP_SEQUENCE"
};

//***************************************************************************
// Elements - Decoder configuration
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::GASpecificConfig ()
{
    //Parsing
    Element_Begin1("GASpecificConfig");
    bool frameLengthFlag, dependsOnCoreCoder, extensionFlag;
    Get_SB (   frameLengthFlag,                                 "frameLengthFlag");
    frame_length=frameLengthFlag==0?1024:960; Param_Info2(frame_length, " bytes");
    Get_SB (   dependsOnCoreCoder,                              "dependsOnCoreCoder");
    if (dependsOnCoreCoder)
        Skip_S2(14,                                             "coreCoderDelay");
    Get_SB (   extensionFlag,                                   "extensionFlag");
    if (channelConfiguration==0)
        program_config_element();
    if (audioObjectType==06 || audioObjectType==20)
        Skip_S1(3,                                              "layerNr");
    if (extensionFlag)
    {
        bool extensionFlag3;
        if (audioObjectType==22)
        {
            Skip_S1( 5,                                         "numOfSubFrame");
            Skip_S2(11,                                         "layer_length");
        }
        if (audioObjectType==17
         || audioObjectType==19
         || audioObjectType==20
         || audioObjectType==23)
        {
            Skip_SB(                                            "aacSectionDataResilienceFlag");
            Skip_SB(                                            "aacScalefactorDataResilienceFlag");
            Skip_SB(                                            "aacSpectralDataResilienceFlag");
        }
        Get_SB (   extensionFlag3,                              "extensionFlag3");
        if (extensionFlag3)
        {
            Skip_BS(Data_BS_Remain(),                           "Not implemented");
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::program_config_element()
{
    Element_Begin1("program_config_element");
    Ztring comment_field_data;
    int8u Channels=0, Channels_Front=0, Channels_Side=0, Channels_Back=0, Channels_LFE=0;
    int8u num_front_channel_elements, num_side_channel_elements, num_back_channel_elements, num_lfe_channel_elements, num_assoc_data_elements, num_valid_cc_elements, comment_field_bytes;
    int8u audioObjectType_Temp, sampling_frequency_index_Temp;
    Skip_S1(4,                                                  "element_instance_tag");
    Get_S1 (2, audioObjectType_Temp,                            "object_type"); audioObjectType_Temp++; Param_Info1(Aac_audioObjectType(audioObjectType_Temp));
    Get_S1 (4, sampling_frequency_index_Temp,                   "sampling_frequency_index"); Param_Info1(Aac_sampling_frequency[sampling_frequency_index_Temp]);
    Get_S1 (4, num_front_channel_elements,                      "num_front_channel_elements");
    Get_S1 (4, num_side_channel_elements,                       "num_side_channel_elements");
    Get_S1 (4, num_back_channel_elements,                       "num_back_channel_elements");
    Get_S1 (2, num_lfe_channel_elements,                        "num_lfe_channel_elements");
    Get_S1 (3, num_assoc_data_elements,                         "num_assoc_data_elements");
    Get_S1 (4, num_valid_cc_elements,                           "num_valid_cc_elements");
    TEST_SB_SKIP(                                               "mono_mixdown_present");
        Skip_S1(4,                                              "mono_mixdown_element_number");
    TEST_SB_END();
    TEST_SB_SKIP(                                               "stereo_mixdown_present");
        Skip_S1(4,                                              "stereo_mixdown_element_number");
    TEST_SB_END();
    TEST_SB_SKIP(                                               "matrix_mixdown_idx_present");
        Skip_S1(2,                                              "matrix_mixdown_idx");
        Skip_SB(                                                "pseudo_surround_enable");
    TEST_SB_END();
    bool front1_element_is_cpe=false;
    if (!num_side_channel_elements && num_back_channel_elements && num_back_channel_elements<3) // Hack: e.g. in case of 5.1,
    {
        num_side_channel_elements=num_back_channel_elements;
        num_back_channel_elements=0;
    }
    for (int8u Pos=0; Pos<num_front_channel_elements; Pos++)
    {
        Element_Begin1("front_element");
        bool front_element_is_cpe;
        Get_SB (   front_element_is_cpe,                        "front_element_is_cpe");
        Skip_S1(4,                                              "front_element_tag_select");
        if (front_element_is_cpe)
        {
            Channels_Front+=2;
            Channels+=2;
            if (Pos==0)
                front1_element_is_cpe=true;
        }
        else
        {
            Channels_Front++;
            Channels++;
        }
        Element_End0();
    }
    for (int8u Pos=0; Pos<num_side_channel_elements; Pos++)
    {
        Element_Begin1("side_element");
        bool side_element_is_cpe;
        Get_SB (   side_element_is_cpe,                         "side_element_is_cpe");
        Skip_S1(4,                                              "side_element_tag_select");
        if (side_element_is_cpe)
        {
            Channels_Side+=2;
            Channels+=2;
        }
        else
        {
            Channels_Side++;
            Channels++;
        }
        Element_End0();
    }
    for (int8u Pos=0; Pos<num_back_channel_elements; Pos++)
    {
        Element_Begin1("back_element");
        bool back_element_is_cpe;
        Get_SB (   back_element_is_cpe,                         "back_element_is_cpe");
        Skip_S1(4,                                              "back_element_tag_select");
        if (back_element_is_cpe)
        {
            Channels_Back+=2;
            Channels+=2;
        }
        else
        {
            Channels_Back++;
            Channels++;
        }
        Element_End0();
    }
    for (int8u Pos=0; Pos<num_lfe_channel_elements; Pos++)
    {
        Element_Begin1("lfe_element");
        Skip_S1(4,                                              "lfe_element_tag_select");
        Channels_LFE++;
        Channels++;
        Element_End0();
    }
    for (int8u Pos=0; Pos<num_assoc_data_elements; Pos++)
    {
        Element_Begin1("assoc_data_element");
        Skip_S1(4,                                              "assoc_data_element_tag_select");
        Element_End0();
    }
    for (int8u Pos=0; Pos<num_valid_cc_elements; Pos++)
    {
        Element_Begin1("valid_cc_element");
        Skip_SB(                                                "cc_element_is_ind_sw");
        Skip_S1(4,                                              "valid_cc_element_tag_select");
        Element_End0();
    }
    BS_End(); //Byte align
    Get_B1 (comment_field_bytes,                                "comment_field_bytes");
    if (comment_field_bytes)
        Get_Local(comment_field_bytes, comment_field_data,      "comment_field_data");
    BS_Begin(); //The stream needs continuity in the bitstream
    Element_End0();

    //Filling
    Ztring Channels_Positions, Channels_Positions2, ChannelLayout;
    switch (Channels_Front)
    {
        case  0 : break;
        case  1 : Channels_Positions+=__T("Front: C"); ChannelLayout+=__T("C "); break;
        case  2 : Channels_Positions+=__T("Front: L R"); ChannelLayout+=__T("L R "); break;
        case  3 : Channels_Positions+=__T("Front: L C R"); ChannelLayout+=num_front_channel_elements==2?(front1_element_is_cpe?__T("L R C "):__T("C L R ")):__T("? ? ? "); break;
        default : Channels_Positions+=__T("Front: "); Channels_Positions+=Ztring::ToZtring(Channels_Front); ChannelLayout+=__T("? "); //Which config?
    }
    switch (Channels_Side)
    {
        case  0 : break;
        case  1 : Channels_Positions+=__T(", Side: C"); ChannelLayout+=__T("Cs "); break;
        case  2 : Channels_Positions+=__T(", Side: L R"); ChannelLayout+=__T("Ls Rs "); break;
        case  3 : Channels_Positions+=__T(", Side: L C R"); ChannelLayout+=__T("? ? ? "); break;
        default : Channels_Positions+=__T(", Side: "); Channels_Positions+=Ztring::ToZtring(Channels_Side); ChannelLayout+=__T("? "); //Which config?
    }
    switch (Channels_Back)
    {
        case  0 : break;
        case  1 : Channels_Positions+=__T(", Back: C"); ChannelLayout+=__T("Cs "); break;
        case  2 : Channels_Positions+=__T(", Back: L R"); ChannelLayout+=__T("Rls Rrs "); break;
        case  3 : Channels_Positions+=__T(", Back: L C R"); ChannelLayout+=__T("Rls Cs Rrs "); break;
        default : Channels_Positions+=__T(", Back: "); Channels_Positions+=Ztring::ToZtring(Channels_Back); ChannelLayout+=__T("? "); //Which config?
    }
    switch (Channels_LFE)
    {
        case  0 : break;
        case  1 : Channels_Positions+=__T(", LFE"); ChannelLayout+=__T("LFE "); break;
        default : Channels_Positions+=__T(", LFE= "); Channels_Positions+=Ztring::ToZtring(Channels_LFE); ChannelLayout+=__T("? "); //Which config?
    }
    Channels_Positions2=Ztring::ToZtring(Channels_Front)+__T('/')
                       +Ztring::ToZtring(Channels_Side)+__T('/')
                       +Ztring::ToZtring(Channels_Back)
                       +(Channels_LFE?__T(".1"):__T(""));
    if (!ChannelLayout.empty())
        ChannelLayout.resize(ChannelLayout.size()-1);

    FILLING_BEGIN();
        //Integrity test
        if (Aac_sampling_frequency[sampling_frequency_index_Temp]==0 || Channels>24) // TODO: full_2023548870.mp4 is buggy
        {
            Trusted_IsNot("sampling frequency / channels");
            Skip_BS(Data_BS_Remain(),                               "(Unknown frequency)");
            return;
        }

        if (audioObjectType==(int8u)-1)
            audioObjectType=audioObjectType_Temp;
        if (sampling_frequency_index==(int8u)-1)
            sampling_frequency_index=sampling_frequency_index_Temp;

        Infos_General["Comment"]=comment_field_data;

        Infos["Format"].From_Local("AAC");
        Infos["Format_Profile"].From_Local(Aac_Format_Profile(audioObjectType));
        Infos["Codec"].From_Local(Aac_audioObjectType(audioObjectType));
        Infos["SamplingRate"].From_Number(Aac_sampling_frequency[sampling_frequency_index]);
        Infos["Channel(s)"].From_Number(Channels);
        Infos["ChannelPositions"]=Channels_Positions;
        Infos["ChannelPositions/String2"]=Channels_Positions2;
        Infos["ChannelLayout"]=ChannelLayout;

        if (!Infos["Format_Settings_SBR"].empty())
        {
            Infos["Format_Profile"]=__T("HE-AAC");
            Ztring SamplingRate=Infos["SamplingRate"];
            Infos["SamplingRate"].From_Number((extension_sampling_frequency_index==(int8u)-1)?(sampling_frequency*2):extension_sampling_frequency, 10);
            if (MediaInfoLib::Config.LegacyStreamDisplay_Get())
            {
                Infos["Format_Profile"]+=__T(" / LC");
                Infos["SamplingRate"]+=__T(" / ")+SamplingRate;
            }
            Infos["Format_Settings_SBR"]=__T("Yes (Implicit)");
            Infos["Codec"]=Ztring().From_Local(Aac_audioObjectType(audioObjectType))+__T("-SBR");
        }

        if (!Infos["Format_Settings_PS"].empty())
        {
            Infos["Format_Profile"]=__T("HE-AACv2");
            Ztring Channels=Infos["Channel(s)"];
            Ztring ChannelPositions=Infos["ChannelPositions"];
            Ztring SamplingRate=Infos["SamplingRate"];
            Infos["Channel(s)"]=__T("2");
            Infos["ChannelPositions"]=__T("Front: L R");
            if (MediaInfoLib::Config.LegacyStreamDisplay_Get())
            {
                Infos["Format_Profile"]+=__T(" / HE-AAC / LC");
                Infos["Channel(s)"]+=__T(" / ")+Channels+__T(" / ")+Channels;
                Infos["ChannelPositions"]+=__T(" / ")+ChannelPositions+__T(" / ")+ChannelPositions;
                Infos["SamplingRate"]=Ztring().From_Number((extension_sampling_frequency_index==(int8u)-1)?(sampling_frequency*2):extension_sampling_frequency, 10)+__T(" / ")+SamplingRate;
            }
            Infos["Format_Settings_PS"]=__T("Yes (Implicit)");
            Ztring Codec=Retrieve(Stream_Audio, StreamPos_Last, Audio_Codec);
            Infos["Codec"]=Ztring().From_Local(Aac_audioObjectType(audioObjectType))+__T("-SBR-PS");
        }
    FILLING_END();
}

//***************************************************************************
// Elements - GA bitstream
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::raw_data_block()
{
    if (sampling_frequency_index>=13)
    {
        Trusted_IsNot("(Problem)");
        Skip_BS(Data_BS_Remain(),                               "(Problem)");
        return;
    }

    if (audioObjectType!=2)
    {
        Skip_BS(Data_BS_Remain(),                               "Data");
        return; //We test only AAC LC
    }

    //Parsing
    Element_Begin1("raw_data_block");
    int8u id_syn_ele=0, id_syn_ele_Previous;
    do
    {
        Element_Begin0();
        id_syn_ele_Previous=id_syn_ele;
        Get_S1 (3, id_syn_ele,                                  "id_syn_ele"); Param_Info1(Aac_id_syn_ele[id_syn_ele]); Element_Name(Aac_id_syn_ele[id_syn_ele]);
        switch (id_syn_ele)
        {
            case 0x00 : single_channel_element();           break; //ID_SCE
            case 0x01 : channel_pair_element();             break; //ID_CPE
            case 0x02 : coupling_channel_element();         break; //ID_CCE
            case 0x03 : lfe_channel_element();              break; //ID_LFE
            case 0x04 : data_stream_element();              break; //ID_DSE
            case 0x05 : program_config_element();           break; //ID_PCE
            case 0x06 : fill_element(id_syn_ele_Previous);  break; //ID_FIL
            case 0x07 :                                     break; //ID_END
            default   :                                          ; //Can not happen
        }
        Element_End0();
    }
    while(Element_IsOK() && Data_BS_Remain() && id_syn_ele!=0x07); //ID_END
    if (Element_IsOK() && Data_BS_Remain()%8)
        Skip_S1(Data_BS_Remain()%8,                             "byte_alignment");
    Element_End0();

    if (sampling_frequency)
    {
        #if MEDIAINFO_TRACE
            if (FrameInfo.PTS!=(int64u)-1)
                Element_Info1(__T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000)));
            if (FrameInfo.DTS!=(int64u)-1)
                Element_Info1(__T("DTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.DTS)/1000000)));
        #endif //MEDIAINFO_TRACE
        FrameInfo.DUR=float64_int64s(((float64)frame_length)*1000000000/sampling_frequency);
        FrameInfo.DTS+=FrameInfo.DUR;
        FrameInfo.PTS=FrameInfo.DTS;
    }
}

//---------------------------------------------------------------------------
void File_Aac::single_channel_element()
{
    //Parsing
    Skip_S1 (4,                                                 "element_instance_tag");
    individual_channel_stream(false, false);
}

//---------------------------------------------------------------------------
void File_Aac::channel_pair_element()
{
    //Parsing
    Skip_S1(4,                                                  "element_instance_tag");
    Get_SB (common_window,                                      "common_window");
    if (common_window)
    {
        int8u ms_mask_present;
        ics_info();
        Get_S1(2, ms_mask_present,                              "ms_mask_present");
        if (ms_mask_present==1)
        {
            Element_Begin1("ms_mask");
            for (int8u g=0; g<num_window_groups; g++)
            {
                Element_Begin1("window");
                for (int8u sfb=0; sfb<max_sfb; sfb++)
                    Skip_SB(                                    "ms_used[g][sfb]");
                Element_End0();
            }
            Element_End0();
        }
    }
    individual_channel_stream(common_window, false);
    if (!Element_IsOK())
        return;
    individual_channel_stream(common_window, false);
}

//---------------------------------------------------------------------------
void File_Aac::ics_info()
{
    //Parsing
    Element_Begin1("ics_info");
    Skip_SB(                                                    "ics_reserved_bit");
    Get_S1 (2, window_sequence,                                 "window_sequence"); Param_Info1(Aac_window_sequence[window_sequence]);
    Skip_SB(                                                    "window_shape");
    if (window_sequence==2) //EIGHT_SHORT_SEQUENCE
    {
        Get_S1 (4, max_sfb,                                     "max_sfb");
        Get_S1 (7, scale_factor_grouping,                       "scale_factor_grouping");
    }
    else
    {
        bool predictor_data_present;
        Get_S1 (6, max_sfb,                                     "max_sfb");
        Get_SB (   predictor_data_present,                      "predictor_data_present");
        if (predictor_data_present)
        {
            if (audioObjectType==1) //AAC Main
            {
                bool predictor_reset;
                Get_SB (predictor_reset,                        "predictor_reset");
                if (predictor_reset)
                    Skip_S1(5,                                  "predictor_reset_group_number");
                int8u PRED_SFB_MAX=max_sfb;
                if (PRED_SFB_MAX>Aac_PRED_SFB_MAX[sampling_frequency_index])
                    PRED_SFB_MAX=Aac_PRED_SFB_MAX[sampling_frequency_index];
                for (int8u sfb=0; sfb<PRED_SFB_MAX; sfb++)
                    Skip_SB(                                    "prediction_used[sfb]");
            }
            else
            {
                bool ltp_data_present;
                Get_SB (ltp_data_present,                       "ltp_data_present");
                if (ltp_data_present)
                    ltp_data();
                if (common_window)
                {
                    Get_SB (ltp_data_present,                   "ltp_data_present");
                    if (ltp_data_present)
                        ltp_data();
                }
            }
        }
    }
    Element_End0();

    //Calculation of windows
    switch (window_sequence)
    {
        case 0 :    //ONLY_LONG_SEQUENCE
        case 1 :    //LONG_START_SEQUENCE
        case 3 :    //LONG_STOP_SEQUENCE
                    num_windows=1;
                    num_window_groups=1;
                    window_group_length[0]=1;
                    num_swb=Aac_swb_offset_long_window[sampling_frequency_index]->num_swb;
                    for (int8u i=0; i<num_swb+1; i++)
                    {
                        if (Aac_swb_offset_long_window[sampling_frequency_index]->swb_offset[i]<frame_length)
                            swb_offset[i]=Aac_swb_offset_long_window[sampling_frequency_index]->swb_offset[i];
                        else
                            swb_offset[i]=frame_length;
                        sect_sfb_offset[0][i]=swb_offset[i];
                    }
                    break;
        case 2 :    //EIGHT_SHORT_SEQUENCE
                    num_windows=8;
                    num_window_groups=1;
                    window_group_length[0]=1;
                    num_swb=Aac_swb_offset_short_window[sampling_frequency_index]->num_swb;
                    for (int8u i=0; i<num_swb + 1; i++)
                        swb_offset[i] = Aac_swb_offset_short_window[sampling_frequency_index]->swb_offset[i];
                    swb_offset[num_swb] = frame_length/8;
                    for (int8u i=0; i<num_windows-1; i++)
                    {
                        if (!(scale_factor_grouping&(1<<(6-i))))
                        {
                            num_window_groups++;
                            window_group_length[num_window_groups-1]=1;
                        }
                        else
                            window_group_length[num_window_groups-1]++;
                    }
                    for (int g = 0; g < num_window_groups; g++)
                    {
                        int8u sect_sfb = 0;
                        int16u offset = 0;
                        for (int8u i=0; i<num_swb; i++)
                        {
                            int16u width = Aac_swb_offset_short_window[sampling_frequency_index]->swb_offset[i+1] - Aac_swb_offset_short_window[sampling_frequency_index]->swb_offset[i];
                            width *= window_group_length[g];
                            sect_sfb_offset[g][sect_sfb++] = offset;
                            offset += width;
                        }
                        sect_sfb_offset[g][sect_sfb] = offset;
                    }
                    break;
        default:    ;
    }
}

//---------------------------------------------------------------------------
void File_Aac::pulse_data()
{
    //Parsing
    int8u number_pulse;
    Get_S1(2,number_pulse,                                      "number_pulse");
    Skip_S1(6,                                                  "pulse_start_sfb");
    for (int i = 0; i < number_pulse+1; i++)
    {
        Skip_S1(5,                                              "pulse_offset[i]");
        Skip_S1(4,                                              "pulse_amp[i]");
    }
}

//---------------------------------------------------------------------------
void File_Aac::coupling_channel_element()
{
    //Parsing
    int8u num_coupled_elements;
    bool ind_sw_cce_flag;
    Skip_S1(4,                                                  "element_instance_tag");
    Get_SB (   ind_sw_cce_flag,                                 "ind_sw_cce_flag");
    Get_S1 (3, num_coupled_elements,                            "num_coupled_elements");
    size_t num_gain_element_lists=0;
    for (int8u c=0; c<num_coupled_elements+1; c++)
    {
        num_gain_element_lists++;
        bool cc_target_is_cpe;
        Get_SB (   cc_target_is_cpe,                            "cc_target_is_cpe[c]");
        Skip_S1(4,                                              "cc_target_tag_select[c]");
        if (cc_target_is_cpe)
        {
            bool cc_l, cc_r;
            Get_SB (cc_l,                                       "cc_l[c]");
            Get_SB (cc_r,                                       "cc_r[c]");
            if (cc_l && cc_r)
                num_gain_element_lists++;
        }
    }
    Skip_SB(                                                    "cc_domain");
    Skip_SB(                                                    "gain_element_sign");
    Skip_S1(2,                                                  "gain_element_scale");
    individual_channel_stream(false, false);
    if (!Element_IsOK())
        return;
    bool cge;
    for (size_t c=1; c<num_gain_element_lists; c++)
    {
        if (ind_sw_cce_flag)
            cge = true;
        else
            Get_SB (cge,                                        "common_gain_element_present[c]");
        if (cge)
            hcod_sf(                                            "hcod_sf[common_gain_element[c]]");
        else
        {
            for (int g = 0; g < num_window_groups; g++)
            {
                for (int sfb=0; sfb<max_sfb; sfb++)
                {
                    if (sfb_cb[g][sfb]) //Not ZERO_HCB
                        hcod_sf(                                "hcod_sf[dpcm_gain_element[c][g][sfb]]");
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Aac::lfe_channel_element()
{
    Skip_S1(4,                                                  "element_instance_tag");
    individual_channel_stream(false, false);
}

//---------------------------------------------------------------------------
void File_Aac::data_stream_element()
{
    bool data_byte_align_flag;
    int16u cnt;
    int8u  count;
    Skip_S1(4,                                                  "element_instance_tag");
    Get_SB (   data_byte_align_flag,                            "data_byte_align_flag");
    Get_S1 (8, count,                                           "count");
    cnt=count;
    if (cnt==255)
    {
        Get_S1(8, count,                                        "esc_count");
        cnt+=count;
    }
    if (data_byte_align_flag)
    {
        if (Data_BS_Remain()%8)
            Skip_S1(Data_BS_Remain()%8,                         "byte_alignment");
    }
    Element_Begin1("data_stream_byte[element_instance_tag]");
    for (int16u i=0; i<cnt; i++)
        Skip_S1(8,                                                "[i]");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::fill_element(int8u id_syn_ele)
{
    //Parsing
    int8u  count;
    Get_S1 (4, count,                                           "count");
    size_t cnt=count;
    if (count==15)
    {
        int8u esc_count;
        Get_S1 (8, esc_count,                                   "esc_count");
        cnt+=esc_count-1;
    }
    if (cnt)
    {
        if (Data_BS_Remain()>=8*cnt)
        {
            size_t End=Data_BS_Remain()-8*cnt;
            extension_payload(End, id_syn_ele);
        }
        else
            Skip_BS(Data_BS_Remain(),                           "(Error)");
    }
}

//---------------------------------------------------------------------------
void File_Aac::gain_control_data()
{
    int8u max_band, adjust_num, aloc_bits, aloc_bits0;
    int8u wd_max=0;
    switch(window_sequence)
    {
        case 0 :    //ONLY_LONG_SEQUENCE
                    wd_max = 1;
                    aloc_bits0 = 5;
                    aloc_bits = 5;
                    break;
        case 1 :    //LONG_START_SEQUENCE
                    wd_max = 2;
                    aloc_bits0 = 4;
                    aloc_bits = 2;
                    break;
        case 2 :    //EIGHT_SHORT_SEQUENCE
                    wd_max = 8;
                    aloc_bits0 = 2;
                    aloc_bits = 2;
                    break;
        case 3 :    //LONG_STOP_SEQUENCE
                    wd_max = 2;
                    aloc_bits0 = 4;
                    aloc_bits = 5;
                    break;
        default:    return; //Never happens but makes compiler happy
    }
    Get_S1 (2, max_band,                                        "max_band");
    for (int8u bd=1; bd<=max_band; bd++)
    {
        for (int8u wd=0; wd<wd_max; wd++)
        {
            Get_S1(3, adjust_num,                               "adjust_num[bd][wd]");
            for (int8u ad=0; ad<adjust_num; ad++)
            {
                Skip_S1(4,                                      "alevcode[bd][wd][ad]");
                Skip_S1(wd==0?aloc_bits0:aloc_bits,             "aloccode[bd][wd][ad]");
            }
        }
    }
}

//***************************************************************************
// Elements - Subsidiary
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::individual_channel_stream (bool common_window, bool scale_flag)
{
    Element_Begin1("individual_channel_stream");
    Skip_S1(8,                                                  "global_gain");
    if (!common_window && !scale_flag)
        ics_info();
    if (!Element_IsOK())
    {
        Element_End0();
        return;
    }
   section_data();
    if (!Element_IsOK())
    {
        Element_End0();
        return;
    }
    scale_factor_data();
    if (!Element_IsOK())
    {
        Element_End0();
        return;
    }
    if (!scale_flag)
    {
        bool pulse_data_present;
        Get_SB (pulse_data_present,                             "pulse_data_present");
        if (pulse_data_present)
            pulse_data ();
        bool tns_data_present;
        Get_SB(tns_data_present,                                "tns_data_present");
        if (tns_data_present)
            tns_data ();
        bool gain_control_data_present;
        Get_SB(gain_control_data_present,                       "gain_control_data_present");
        if (gain_control_data_present)
            gain_control_data ();
    }
    if (!aacSpectralDataResilienceFlag)
        spectral_data ();
    else
    {
        Skip_BS(Data_BS_Remain(),                               "Not implemented");
        //~ length_of_reordered_spectral_data;
        //~ length_of_longest_codeword;
        //~ reordered_spectral_data ();
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::section_data()
{
    Element_Begin1("section_data");
    int8u sect_esc_val;
    if (window_sequence==2) //EIGHT_SHORT_SEQUENCE
        sect_esc_val=(1<<3)-1;
    else
        sect_esc_val=(1<<5)-1;
    for (int8u g=0; g<num_window_groups; g++)
    {
        if (num_window_groups>1)
            Element_Begin1("windows");
        int8u k=0;
        int8u i=0;
        while (k<max_sfb)
        {
            if (aacSectionDataResilienceFlag)
                Get_S1(5, sect_cb[g][i],                        "sect_cb[g][i]");
            else
                Get_S1(4, sect_cb[g][i],                        "sect_cb[g][i]");
            int8u sect_len=0;
            int8u sect_len_incr;
            if (!aacSectionDataResilienceFlag || sect_cb[g][i]<11 || (sect_cb[g][i]>11 && sect_cb[g][i]<16))
            {
                for (;;)
                {
                    if (Data_BS_Remain()==0)
                    {
                        Trusted_IsNot("Size is wrong");
                        if (num_window_groups>1)
                            Element_End0();
                        Element_End0();
                        return; //Error
                    }
                    Get_S1 ((window_sequence==2?3:5), sect_len_incr, "sect_len_incr"); // (window_sequence == EIGHT_SHORT_SEQUENCE) => 3
                    if (sect_len_incr!=sect_esc_val)
                        break;
                    sect_len+=sect_esc_val;
                }
            }
            else
                sect_len_incr=1;
            sect_len+=sect_len_incr;
            sect_start[g][i]=k;
            sect_end[g][i]=k+sect_len;
            for (int16u sfb=k; sfb<k+sect_len; sfb++)
                sfb_cb[g][sfb]=sect_cb[g][i];
            k+= sect_len;
            i++;
            if (i>64)
            {
                Trusted_IsNot("Increment is wrong");
                if (num_window_groups>1)
                    Element_End0();
                Element_End0();
                return; //Error
            }
        }
        num_sec[g]=i;
        if (num_window_groups>1)
            Element_End0();
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::scale_factor_data()
{
    Element_Begin1("scale_factor_data");
    if (!aacScalefactorDataResilienceFlag)
    {
        bool noise_pcm_flag=true;
        for (int g=0; g<num_window_groups; g++)
        {
            for (int8u sfb=0; sfb<max_sfb; sfb++)
            {
                if (sfb_cb[g][sfb]) //Not ZERO_HCB
                {
                    if (is_intensity( g, sfb))
                        hcod_sf(                                "hcod_sf[dpcm_is_position[g][sfb]]");
                    else
                    {
                        if (is_noise(g, sfb))
                        {
                            if (noise_pcm_flag)
                            {
                                noise_pcm_flag = 0;
                                Skip_S2(9,                      "dpcm_noise_nrg[g][sfb]");
                            }
                            else
                                hcod_sf(                        "hcod_sf[dpcm_noise_nrg[g][sfb]]");
                        }
                        else
                            hcod_sf(                            "hcod_sf[dpcm_sf[g][sfb]]");
                    }
                }
            }
        }
    }
    else
    {
        //scale_factor_data - part not implemented
        Skip_BS(Data_BS_Remain(),                               "Not implemented");
        //~ intensity_used = 0;
        //~ noise_used = 0;
        //~ sf_concealment;
        //~ rev_global_gain;
        //~ length_of_rvlc_sf;
        //~ for ( g = 0; g < num_window_groups; g++ ) {
            //~ for ( sfb=0; sfb < max_sfb; sfb++ ) {
                //~ if ( sfb_cb[g][sfb] ) { //Not ZERO_HCB
                    //~ if ( is_intensity ( g, sfb) ) {
                        //~ intensity_used = 1;
                        //~ rvlc_cod_sf[dpcm_is_position[g][sfb]];
                    //~ } else {
                        //~ if ( is_noise(g,sfb) ) {
                            //~ if ( ! noise_used ) {
                                //~ noise_used = 1;
                                //~ dpcm_noise_nrg[g][sfb];
                            //~ } else {
                                //~ rvlc_cod_sf[dpcm_noise_nrg[g][sfb]];
                            //~ }
                        //~ } else {
                            //~ rvlc_cod_sf[dpcm_sf[g][sfb]];
                        //~ }
                    //~ }
                //~ }
            //~ }
        //~ }
        //~ if ( intensity_used ) {
            //~ rvlc_cod_sf[dpcm_is_last_position];
        //~ }
        //~ noise_used = 0;
        //~ sf_escapes_present;
        //~ if ( sf_escapes_present ) {
            //~ length_of_rvlc_escapes;
            //~ for ( g = 0; g < num_window_groups; g++ ) {
                //~ for ( sfb = 0; sfb < max_sfb; sfb++ ) {
                    //~ if ( sfb_cb[g][sfb]) {  //Not ZERO_HCB
                        //~ if ( is_intensity ( g, sfb ) &&    dpcm_is_position[g][sfb] == ESC_FLAG ) {
                            //~ rvlc_esc_sf[dpcm_is_position[g][sfb]];
                        //~ } else {
                            //~ if ( is_noise ( g, sfb ) {
                                //~ if ( ! noise_used ) {
                                    //~ noise_used = 1;
                                //~ } else {
                                    //~ if (dpcm_noise_nrg[g][sfb] == ESC_FLAG ) {
                                        //~ rvlc_esc_sf[dpcm_noise_nrg[g][sfb]];
                                    //~ }
                                //~ }
                            //~ } else {
                                //~ if (dpcm_sf[g][sfb] == ESC_FLAG ) {
                                    //~ rvlc_esc_sf[dpcm_sf[g][sfb]];
                                //~ }
                            //~ }
                        //~ }
                    //~ }
                //~ }
            //~ }
            //~ if ( intensity_used && dpcm_is_last_position == ESC_FLAG ) {
                //~ rvlc_esc_sf[dpcm_is_last_position];
            //~ }
        //~ }
        //~ if ( noise_used ) {
            //~ dpcm_noise_last_position;
        //~ }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::tns_data()
{
    int8u n_filt_bits=2;
    int8u length_bits=6;
    int8u order_bits=5;

    if (window_sequence==2) //EIGHT_SHORT_SEQUENCE
    {
        n_filt_bits=1;
        length_bits=4;
        order_bits=3;
    }

    for (int8u w=0; w<num_windows; w++)
    {
        int8u start_coef_bits, n_filt;
        Get_S1(n_filt_bits, n_filt,                             "n_filt[w]");
        if (n_filt)
        {
            bool coef_res;
            Get_SB (coef_res,                                   "coef_res[w]");
            start_coef_bits=coef_res?4:3;

            for (int8u filt=0; filt<n_filt; filt++)
            {
                int8u order;
                Skip_S1(length_bits,                            "length[w][filt]");
                Get_S1 (order_bits, order,                      "order[w][filt]");
                if (order)
                {
                    bool coef_compress;
                    Skip_SB(                                    "direction[w][filt]");
                    Get_SB (coef_compress,                      "coef_compress[w][filt]");
                    int8u coef_bits=start_coef_bits-(coef_compress?1:0);
                    for (int8u i=0; i<order; i++)
                        Skip_S1(coef_bits,                      "coef[w][filt][i]");
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Aac::ltp_data()
{
    Element_Begin1("ltp_data");
    //int sfb;
    //bool ltp_lag_update;
    //if (AudioObjectType == ER_AAC_LD ) {
        //~ Get_SB(ltp_lag_update,"ltp_lag_update");
        //~ if ( ltp_lag_update ) {
            //~ Get_S2(10,ltp_lag,"ltp_lag");
        //~ } else {
            //~ //ltp_lag = ltp_prev_lag;
        //~ }
        //~ Skip_S1(3,"ltp_coef");
        //~ for (sfb = 0; sfb <(max_sfb<MAX_LTP_LONG_SFB?max_sfb:MAX_LTP_LONG_SFB);& sfb++ ) {
            //~ Skip_SB("ltp_long_used[sfb]");
        //~ }
    //} else {
        Get_S2(11,ltp_lag,                                        "ltp_lag");
        Skip_S1(3,                                                "ltp_coef");
        if(window_sequence!=2) //EIGHT_SHORT_SEQUENCE
        {
            for (int8u sfb=0; sfb<(max_sfb<40?max_sfb:40); sfb++ ) //MAX_LTP_LONG_SFB=40
                Skip_SB("ltp_long_used[sfb]");
        }
    //}
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::spectral_data()
{
    Element_Begin1("spectral_data");
    for (int g = 0; g < num_window_groups; g++)
    {
        if (num_window_groups>1)
            Element_Begin1("windows");
        for (int8u i=0; i<num_sec[g]; i++)
        {
            switch (sect_cb[g][i])
            {
                case  0 :   //ZERO_HCB
                case 13 :   //NOISE_HCB
                case 14 :   //INTENSITY_HCB2
                case 15 :   //INTENSITY_HCB
                            break;
                default :
                            if (sect_end[g][i]>=num_swb+1)
                            {
                                Trusted_IsNot("(Problem)");
                                Skip_BS(Data_BS_Remain(),       "(Problem)");
                                if (num_window_groups>1)
                                    Element_End0();
                                Element_End0();
                                return;
                            }
                            for (int16u k=sect_sfb_offset[g][sect_start[g][i]]; k<sect_sfb_offset[g][sect_end[g][i]]; k+=(sect_cb[g][i]<5?4:2))
                            {
                                hcod(sect_cb[g][i],             "sect_cb");
                                if (!Element_IsOK())
                                {
                                    Skip_BS(Data_BS_Remain(),   "(Problem)");
                                    if (num_window_groups>1)
                                        Element_End0();
                                    Element_End0();
                                    return;
                                }
                            }
            }
        }
        if (num_window_groups>1)
            Element_End0();
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::extension_payload(size_t End, int8u id_aac)
{
    Element_Begin1("extension_payload");
    int8u extension_type;
    Get_S1 (4, extension_type,                                  "extension_type");
    switch(extension_type)
    {
        case 11 :   dynamic_range_info(); break;                //EXT_DYNAMIC_RANGE
        case 12 :   sac_extension_data(End); break;             //EXT_SAC_DATA
        case 13 :   sbr_extension_data(End, id_aac, 0); break;  //EXT_SBR_DATA
        case 14 :   sbr_extension_data(End, id_aac, 1); break;  //EXT_SBR_DATA_CRC
        case  1 :   //EXT_FILL_DATA
                    Skip_S1(4,                                  "fill_nibble"); Param_Info1("must be 0000");
                    if (Data_BS_Remain()>End)
                    {
                        Element_Begin1("fill_byte");
                        while (Data_BS_Remain()>End)
                            Skip_S1(8,                          "fill_byte[i]"); Param_Info1("must be 10100101");
                        Element_End0();
                    }
                    break;
        case  2 :   //EXT_DATA_ELEMENT
                    int8u data_element_version;
                    Get_S1 (4,data_element_version,             "data_element_version");
                    switch(data_element_version)
                    {
                        case 0 :    //ANC_DATA
                                    {
                                    int16u dataElementLength=0;
                                    int8u dataElementLengthPart;
                                    do
                                    {
                                        Get_S1 (8, dataElementLengthPart, "dataElementLengthPart");
                                        dataElementLength+=dataElementLengthPart;
                                    }
                                    while (dataElementLengthPart==255);
                                    Skip_BS(8*dataElementLength, "data_element_byte[i]");
                                    }
                                    break;
                        default:    ;
                    }
                    break;
        case 0 : //EXT_FILL
        default:
            Skip_BS(Data_BS_Remain()-End,                       "other_bits");
    }

    Element_End0();
    if (End<Data_BS_Remain())
        Skip_BS(Data_BS_Remain()-End,                           "padding");
    if (Data_BS_Remain()!=End)
    {
        Skip_BS(Data_BS_Remain(),                               "Wrong size");
        Trusted_IsNot("Wrong size");
    }
}

//---------------------------------------------------------------------------
void File_Aac::dynamic_range_info()
{
    Element_Begin1("dynamic_range_info");
    int8u  drc_num_bands=1;
    bool   present;
    Get_SB (present,                                            "pce_tag_present");
    if (present)
    {
        Skip_S1(4,                                              "pce_ instance_tag");
        Skip_S1(4,                                              "drc_tag_reserved_bits");
    }
    Skip_SB(                                                    "excluded_chns_present");
    Get_SB (present,                                            "drc_bands_present");
    if (present)
    {
        int8u drc_band_incr;
        Get_S1 (4, drc_band_incr,                               "drc_band_incr");
        Skip_S1(4,                                              "drc_interpolation_scheme");
        drc_num_bands+=drc_band_incr;
        for (int8u i=0; i<drc_num_bands; i++)
        {
            Skip_S1(8,                                          "drc_band_top[i]");
        }
    }
    Get_SB (present,                                            "prog_ref_level_present");
    if (present)
    {
        Skip_S1(7,                                              "prog_ref_level");
        Skip_S1(1,                                              "prog_ref_level_reserved_bits");
    }
    for (int8u i=0; i<drc_num_bands; i++)
    {
        Skip_S1(1,                                              "dyn_rng_sgn[i]");
        Skip_S1(7,                                              "dyn_rng_ctl[i]");
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sac_extension_data(size_t End)
{
    Element_Begin1("sac_extension_data");
    Skip_S1(2,                                                  "ancType");
    Skip_SB(                                                    "ancStart");
    Skip_SB(                                                    "ancStop");
    Element_Begin1("ancDataSegmentByte");
    while (Data_BS_Remain()>End)
        Skip_S1(8,                                              "ancDataSegmentByte[i]");
    Element_End0();
    Element_End0();
}

//***************************************************************************
// Elements - Perceptual noise substitution (PNS)
//***************************************************************************

//---------------------------------------------------------------------------
int File_Aac::is_intensity(size_t group, size_t sfb)
{
    switch (sfb_cb[group][sfb])
    {
        case 14 :   return 1;
        case 15 :   return -1;
        default :   return 0;
    }
}

//---------------------------------------------------------------------------
bool File_Aac::is_noise(size_t group, size_t sfb)
{
    return (sfb_cb[group][sfb]==13);
}
//---------------------------------------------------------------------------
void File_Aac::hcod_sf(const char* Name)
{
    Element_Begin1(Name);
    int16u Pos=0;

    while (huffman_sf[Pos][1])
    {
        bool h;
        Get_SB (h,                                              "huffman");
        Pos+=huffman_sf[Pos][h];

        if (Pos>240)
        {
            Skip_BS(Data_BS_Remain(),                           "Error");
            Element_End0();
            return;
        }
    }
    Element_Info1(huffman_sf[Pos][0]-60);
    Element_End0();

    return;
}

//---------------------------------------------------------------------------
void File_Aac::hcod_2step(int8u CodeBook, int8s* Values, int8u Values_Count)
{
    int8u CodeWord;
    int8u ToRead=hcb_2step_Bytes[CodeBook];
    if ((size_t)ToRead>Data_BS_Remain())
        ToRead=(int8u)Data_BS_Remain(); //Read a maximum of remaining bytes
    Peek_S1(ToRead, CodeWord);
    int16u Offset=hcb_2step[CodeBook][CodeWord].Offset;
    int8u  Extra=hcb_2step[CodeBook][CodeWord].Extra;

    if (Extra)
    {
        Skip_BS(hcb_2step_Bytes[CodeBook],                            "extra");
        int8u Offset_inc;
        Peek_S1(Extra, Offset_inc);
        Offset+=Offset_inc;
        if(hcb_table[CodeBook][Offset][0]-hcb_2step_Bytes[CodeBook])
            Skip_BS(hcb_table[CodeBook][Offset][0]-hcb_2step_Bytes[CodeBook],"extra");
    }
    else
    {
        Skip_BS(hcb_table[CodeBook][Offset][0],                 "bits");
    }

    if (Offset>=hcb_table_size[CodeBook])
    {
        Skip_BS(Data_BS_Remain(),                               "Error");
        return;
    }

    for (int8u Pos=0; Pos<Values_Count; Pos++)
        Values[Pos]=hcb_table[CodeBook][Offset][Pos+1];
}

//---------------------------------------------------------------------------
void File_Aac::hcod_binary(int8u CodeBook, int8s* Values, int8u Values_Count)
{
    int16u Offset=0;
    while (!hcb_table[CodeBook][Offset][0])
    {
        bool b;
        Get_SB(b,                                               "huffman binary");
        Offset+=hcb_table[CodeBook][Offset][1+(b?1:0)];
    }

    if (Offset>=hcb_table_size[CodeBook])
    {
        Skip_BS(Data_BS_Remain(),                               "Error");
        return;
    }

    for (int8u Pos=0; Pos<Values_Count; Pos++)
        Values[Pos]=hcb_table[CodeBook][Offset][Pos+1];
}



//***************************************************************************
// Elements - Enhanced Low Delay Codec
//***************************************************************************


//---------------------------------------------------------------------------
void File_Aac::ELDSpecificConfig ()
{
    Element_Begin1("ELDSpecificConfig");
    Skip_SB("frameLengthFlag");
    Skip_SB("aacSectionDataResilienceFlag");
    Skip_SB("aacScalefactorDataResilienceFlag");
    Skip_SB("aacSpectralDataResilienceFlag");

    bool ldSbrPresentFlag;
    Get_SB(ldSbrPresentFlag,"ldSbrPresentFlag");
    if (ldSbrPresentFlag)
    {
        Skip_SB("ldSbrSamplingRate");
        Skip_SB("ldSbrCrcFlag");
        ld_sbr_header();
    }

    int8u eldExtType;
    for (;;)
    {
        Get_S1(4,eldExtType,"eldExtType");
        if (eldExtType == 0/*ELDEXT_TERM*/)
            break;

        int8u eldExtLen,eldExtLenAdd=0;
        int16u eldExtLenAddAdd;
        Get_S1(4,eldExtLen,"eldExtLen");
        int32u len = eldExtLen;
        if (eldExtLen == 15)
        {
            Get_S1(8,eldExtLenAdd,"eldExtLenAdd");
            len += eldExtLenAdd;
        }
        if (eldExtLenAdd==255)
        {
            Get_S2(16,eldExtLenAddAdd,"eldExtLenAddAdd");
            len += eldExtLenAdd;
        }
        //~ switch (eldExtType) {
            /* add future eld extension configs here */
                //~ default:
                for(int32u cnt=0; cnt<len; cnt++)
                    Skip_S1(8,"other_byte");
                //~ break;
        //~ }

    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::ld_sbr_header()
{
    int8u numSbrHeader;
    switch (channelConfiguration)
    {
        case 1:
        case 2:
            numSbrHeader = 1;
            break;
        case 3:
            numSbrHeader = 2;
            break;
        case 4:
        case 5:
        case 6:
            numSbrHeader = 3;
            break;
        case 7:
            numSbrHeader = 4;
            break;
        default:
            numSbrHeader = 0;
            break;
    }

    for (int el=0; el<numSbrHeader; el++) {
        //~ sbr_header();
        Element_Begin1("not implemented");
        Element_End0();
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aac::hcod(int8u sect_cb, const char* Name)
{
    int8s Values[4];

    Element_Begin1(Name);
    switch (sect_cb)
    {
        case  1 :
        case  2 :
        case  4 :   //4-values, 2-step method
                    hcod_2step(sect_cb, Values, 4);
                    break;
        case  3 :   //4-values, binary search method
                    hcod_binary(sect_cb, Values, 4);
                    break;
        case  5 :
        case  7 :
        case  9 :   //2-values, binary search method
                    hcod_binary(sect_cb, Values, 2);
                    break;
        case  6 :
        case  8 :
        case 10 :
        case 11 :   //2-values, 2-step method
                    hcod_2step(sect_cb, Values, 2);
                    break;
        default:    Trusted_IsNot("(Problem)");
                    Element_End0();
                    return;
    }

    switch (sect_cb)
    {
        case  1 :
        case  2 :
        case  5 :
        case  6 :
                    break;
        default :   //With sign
                    for(int i=0; i<((sect_cb<5)?4:2); i++)
                        if(Values[i])
                            Skip_SB(                            "sign");
    }

    switch (sect_cb)
    {
        case 11 : //With hcod_esc
            for (int i=0; i<2; i++)
                if (Values[i]==16 || Values[i] == -16)
                {
                    Element_Begin1("hcod_esc");
                    bool Escape;
                    int BitCount=3;
                    do
                    {
                        BitCount++;
                        Get_SB(Escape,                          "bit count");
                    }
                    while (Escape);


                    Skip_BS(BitCount,                           "value");
                    Element_End0();

                }
            break;
        default: ;
    }

    Element_End0();
}



//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AAC_YES
