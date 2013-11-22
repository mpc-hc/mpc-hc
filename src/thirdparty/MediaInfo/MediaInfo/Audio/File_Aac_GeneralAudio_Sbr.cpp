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
#include "MediaInfo/Audio/File_Aac_GeneralAudio_Sbr.h"
#include <cmath>
#include <algorithm>
#include <cstring>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern const char* Aac_audioObjectType(int8u audioObjectType);

//---------------------------------------------------------------------------
int8u Aac_AudioSpecificConfig_sampling_frequency_index(const int32u sampling_frequency);

//---------------------------------------------------------------------------
// Master frequency band table
// k0 = lower frequency boundary
int8u Aac_k0_Compute(int8u bs_start_freq, int8u extension_sampling_frequency_index)
{
    return Aac_k0_startMin[extension_sampling_frequency_index]+Aac_k0_offset[extension_sampling_frequency_index][bs_start_freq];
}

//---------------------------------------------------------------------------
// Master frequency band table
// k2 = upper frequency boundary
int8u Aac_k2_Compute(int8u bs_stop_freq, int8u extension_sampling_frequency_index, int8u  k0)
{
    switch (bs_stop_freq)
    {
        case 14 : return (int8u)min(64, 2*k0);
        case 15 : return (int8u)min(64, 3*k0);
        default : ;
    }

    return (int8u)min(64, Aac_k2_stopMin[extension_sampling_frequency_index]+Aac_k2_offset[extension_sampling_frequency_index][bs_stop_freq]);
}

//---------------------------------------------------------------------------
//Helper
int8u Aac_bands_Compute(bool warp, int8u bands, int8u a0, int8u a1)
{
    float div=(float)log(2.0);
    if (warp)
        div*=(float)1.3;

    return (int8u)(bands*log((float)a1/(float)a0)/div+0.5);
}

//---------------------------------------------------------------------------
// Master frequency band table
// Computing for bs_freq_scale = 0
bool Aac_f_master_Compute_0(int8u &num_env_bands_Master, int8u* f_Master, sbr_handler *sbr, int8u  k0, int8u  k2)
{
    int8u dk, numBands;
    if (sbr->bs_alter_scale)
    {
        dk=1;
        numBands=(((k2-k0+2)>>2)<<1);
    }
    else
    {
        dk=2;
        numBands=(((k2-k0)>>1)<<1);
    }

    int8u k2Achieved=k0+numBands*dk;
    int8s k2Diff=k2-k2Achieved;
    int8s vDk[64];
    for (int8u k=0; k<numBands; k++)
        vDk[k]=dk;

    if (k2Diff)
    {
        int8s  incr;
        int8u  k;
        if (k2Diff>0)
        {
            incr=-1;
            k=numBands-1;
        }
        else
        {
            incr=1;
            k=0;
        }

        while (k2Diff)
        {
            vDk[k]-=incr;
            k+=incr;
            k2Diff+=incr;
        }
    }

    f_Master[0]=k0;
    for (int8u k=1; k<=numBands; k++)
        f_Master[k]=f_Master[k-1]+vDk[k-1];

    num_env_bands_Master=numBands;

    return true;
}

//---------------------------------------------------------------------------
// Master frequency band table
// Computing for bs_freq_scale != 0
int int8u_cmp(const void *a, const void *b)
{
    return ((int8u)(*(int8u*)a - *(int8u*)b));
}
bool Aac_f_master_Compute(int8u &num_env_bands_Master, int8u* f_Master, sbr_handler *sbr, int8u  k0, int8u  k2)
{
    int8u temp1[]={6, 5, 4 };
    int8u bands=temp1[sbr->bs_freq_scale-1];

    int8u twoRegions, k1;
    if ((float)k2/(float)k0>2.2449)
    {
        twoRegions=1;
        k1=2*k0;
    }
    else
    {
        twoRegions=0;
        k1=k2;
    }

    int8u numBands0=2*Aac_bands_Compute(false, bands, k0, k1);
    if (numBands0>=64)
        return false;

    int8u vDk0[64];
    int8u vk0[64];
    float Power=pow((float)k1/(float)k0, (float)1/(float)numBands0);
    float Power2=(float)k0;
    int8s Temp1=(int8s)(Power2+0.5);
    for (int8u k=0; k<=numBands0-1; k++)
    {
        int8s Temp0=Temp1;
        Power2*=Power;
        Temp1=(int8s)(Power2+0.5);
        vDk0[k]=Temp1-Temp0;
    }
    qsort(vDk0, numBands0, sizeof(int8u), int8u_cmp);
    vk0[0]=k0;
    for (int8u k=1; k<=numBands0; k++)
    {
        if (vDk0[k-1]==0)
            return false;
        vk0[k]=vk0[k-1]+vDk0[k-1];
    }

    if (!twoRegions)
    {
        for (int8u k=0; k<=numBands0; k++)
            f_Master[k]=vk0[k];
        num_env_bands_Master=numBands0;
        return true;
    }

    //With twoRegions
    int8u numBands1;
    int8u vDk1[64];
    int8u vk1[64];
    numBands1=2*Aac_bands_Compute(true, bands, k1, k2);
    if (numBands0+numBands1>=64)
        return false;

    Power=(float)pow((float)k2/(float)k1, (float)1/(float)numBands1);
    Power2=(float)k1;
    Temp1=(int8s)(Power2+0.5);
    for (int8u k=0; k<=numBands1-1; k++)
    {
        int8s Temp0=Temp1;
        Power2*=Power;
        Temp1=(int8s)(Power2+0.5);
        vDk1[k]=Temp1-Temp0;
    }

    if (vDk1[0]<vDk0[numBands0-1])
    {
        qsort(vDk1, numBands1+1, sizeof(int8u), int8u_cmp);
        int8u change=vDk0[numBands0-1]-vDk1[0];
        vDk1[0]=vDk0[numBands0-1];
        vDk1[numBands1-1]=vDk1[numBands1-1]-change;
    }

    qsort(vDk1, numBands1, sizeof(int8u), int8u_cmp);
    vk1[0]=k1;
    for (int8u k=1; k<=numBands1; k++)
    {
        if (vDk1[k-1]==0)
            return false;
        vk1[k]=vk1[k-1]+vDk1[k-1];
    }

    num_env_bands_Master=numBands0+numBands1;
    for (int8u k=0; k<=numBands0; k++)
        f_Master[k]=vk0[k];
    for (int8u k=numBands0+1; k <=num_env_bands_Master; k++)
        f_Master[k]=vk1[k-numBands0];

    return true;
}

//---------------------------------------------------------------------------
// Derived frequency border tables
bool Aac_bands_Compute(int8u &num_env_bands_Master, int8u* f_Master, sbr_handler *sbr, int8u  k2)
{
    sbr->num_env_bands[1]=num_env_bands_Master-sbr->bs_xover_band;
    sbr->num_env_bands[0]=(sbr->num_env_bands[1]>>1)+(sbr->num_env_bands[1]-((sbr->num_env_bands[1]>>1)<<1));

    if (f_Master[sbr->bs_xover_band]>32)
        return false;

    if (sbr->bs_noise_bands==0)
        sbr->num_noise_bands=1;
    else
    {
        sbr->num_noise_bands=Aac_bands_Compute(false, sbr->bs_noise_bands, f_Master[sbr->bs_xover_band], k2);
        if (sbr->num_noise_bands<1 || sbr->num_noise_bands>5)
            return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool Aac_Sbr_Compute(sbr_handler *sbr, int8u extension_sampling_frequency_index)
{
    if (extension_sampling_frequency_index>=9)
        return 0; //Not supported
    int8u k0=Aac_k0_Compute(sbr->bs_start_freq, extension_sampling_frequency_index);
    int8u k2=Aac_k2_Compute(sbr->bs_stop_freq, extension_sampling_frequency_index, k0);
    if (k2<=k0) return false;
    switch (extension_sampling_frequency_index)
    {
        case  0 :
        case  1 :
        case  2 :
        case  3 : if ((k2-k0)>32) return false; break;
        case  4 : if ((k2-k0)>35) return false; break;
        case  5 :
        case  6 :
        case  7 :
        case  8 : if ((k2-k0)>48) return false; break;
        default : ;
    }

    int8u  num_env_bands_Master;
    int8u  f_Master[64];
    if (sbr->bs_freq_scale==0)
    {
        if (!Aac_f_master_Compute_0(num_env_bands_Master, f_Master, sbr, k0, k2))
            return false;
    }
    else
    {
        if (!Aac_f_master_Compute(num_env_bands_Master, f_Master, sbr, k0, k2))
            return false;
    }
    if (num_env_bands_Master<=sbr->bs_xover_band)
        return false;
    if (!Aac_bands_Compute(num_env_bands_Master, f_Master, sbr, k2))
        return false;

    return true;
}

//---------------------------------------------------------------------------
int16u File_Aac::sbr_huff_dec(sbr_huffman Table, const char* Name)
{
    int8u bit;
    int16s index = 0;

    Element_Begin1(Name);
    while (index>=0)
    {
        Get_S1(1, bit,                                          "bit");
        index=Table[index][bit];
    }
    Element_End0();

    return index+64;
}

//---------------------------------------------------------------------------
void File_Aac::sbr_extension_data(size_t End, int8u id_aac, bool crc_flag)
{
    FILLING_BEGIN();
        if (Infos["Format_Settings_SBR"].empty())
        {
            Infos["Format_Profile"]=__T("HE-AAC");
            Ztring SamplingRate=Infos["SamplingRate"];
            if (SamplingRate.empty())
                SamplingRate.From_Number(sampling_frequency);
            Infos["SamplingRate"].From_Number((extension_sampling_frequency_index==(int8u)-1)?(sampling_frequency*2):extension_sampling_frequency, 10);
            if (MediaInfoLib::Config.LegacyStreamDisplay_Get())
            {
                Infos["Format_Profile"]+=__T(" / LC");
                Infos["SamplingRate"]+=__T(" / ")+SamplingRate;
            }
            Infos["Format_Settings_SBR"]=__T("Yes (Implicit)");
            Infos["Codec"]=Ztring().From_Local(Aac_audioObjectType(audioObjectType))+__T("-SBR");

            if (Frame_Count_Valid<32)
                Frame_Count_Valid=32; //We need to find the SBR header
        }
    FILLING_END();

    Element_Begin1("sbr_extension_data");
    bool bs_header_flag;
    if (crc_flag)
        Skip_S2(10,                                             "bs_sbr_crc_bits");
    //~ if (sbr_layer != SBR_STEREO_ENHANCE)
    //~ {
        Get_SB(bs_header_flag,                                  "bs_header_flag");
        if (bs_header_flag)
        {
            if (extension_sampling_frequency_index==(int8u)-1)
            {
                extension_sampling_frequency=sampling_frequency*2;
                extension_sampling_frequency_index=Aac_AudioSpecificConfig_sampling_frequency_index(extension_sampling_frequency);
            }

            delete sbr;
            sbr=new sbr_handler;

            sbr_header();

            if (!Aac_Sbr_Compute(sbr, extension_sampling_frequency_index))
            {
                delete sbr; sbr=NULL;
            }
        }
    //~ }

    //Parsing
    if (sbr) //only if a header is found
    {
        sbr->bs_amp_res[0]=sbr->bs_amp_res_FromHeader; //Set up with header data
        sbr->bs_amp_res[1]=sbr->bs_amp_res_FromHeader; //Set up with header data
        sbr_data(id_aac);

        FILLING_BEGIN();
            if (MediaInfoLib::Config.ParseSpeed_Get()<0.3)
            {
                Frame_Count_Valid=Frame_Count+1;
                #if MEDIAINFO_ADVANCED
                    if (aac_frame_lengths.size()<8)
                        Frame_Count_Valid+=8-aac_frame_lengths.size();
                #endif //MEDIAINFO_ADVANCED
            }
        FILLING_END();
    }
    if (Data_BS_Remain()>End)
        Skip_BS(Data_BS_Remain()-End,                           "bs_fill_bits");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_header()
{
    Element_Begin1("sbr_header");
    Get_S1 (1, sbr->bs_amp_res_FromHeader,                      "bs_amp_res");
    Get_S1 (4, sbr->bs_start_freq,                              "bs_start_freq");
    Get_S1 (4, sbr->bs_stop_freq,                               "bs_stop_freq");
    Get_S1 (3, sbr->bs_xover_band,                              "bs_xover_band");
    Skip_S1(2,                                                  "bs_reserved");
    bool bs_header_extra_1, bs_header_extra_2;
    Get_SB (bs_header_extra_1,                                  "bs_header_extra_1");
    Get_SB (bs_header_extra_2,                                  "bs_header_extra_2");
    if(bs_header_extra_1)
    {
        Get_S1 (2, sbr->bs_freq_scale,                          "bs_freq_scale");
        Get_S1 (1, sbr->bs_alter_scale,                         "bs_alter_scale");
        Get_S1 (2, sbr->bs_noise_bands,                         "bs_noise_bands");
    }
    else
    {
        sbr->bs_freq_scale=2;
        sbr->bs_alter_scale=1;
        sbr->bs_noise_bands=2;
    }
    if(bs_header_extra_2)
    {
        Skip_S1(2,                                              "bs_limiter_bands");
        Skip_S1(2,                                              "bs_limiter_gains");
        Skip_SB(                                                "bs_interpol_freq");
        Skip_SB(                                                "bs_smoothing_mode");
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_data(int8u id_aac)
{
    Element_Begin1("sbr_data");
    switch (id_aac)
    {
        case  0 : sbr_single_channel_element();     break; //ID_SCE
        case  1 : sbr_channel_pair_element();       break; //ID_CPE
        default : ;
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_single_channel_element()
{
    Element_Begin1("sbr_single_channel_element");
    bool bs_data_extra, bs_add_harmonic_flag, bs_extended_data;
    int8u bs_extension_size, bs_esc_count, bs_extension_id;
    Get_SB (bs_data_extra,                                      "bs_data_extra");
    if (bs_data_extra)
    {
        Skip_S1(4,                                              "bs_reserved");
    }
    sbr_grid(0);
    sbr_dtdf(0);
    sbr_invf(0);
    sbr_envelope(0, 0);
    sbr_noise(0, 0);
    Get_SB (bs_add_harmonic_flag,                               "bs_add_harmonic_flag[0]");
    if (bs_add_harmonic_flag)
        sbr_sinusoidal_coding(0);

    Get_SB (bs_extended_data,                                   "bs_extended_data[0]");
    if (bs_extended_data) {
        Get_S1 (4,bs_extension_size,                            "bs_extension_size");
        size_t cnt=bs_extension_size;
        if (cnt==15)
        {
            Get_S1 (8, bs_esc_count,                            "bs_esc_count");
            cnt+=bs_esc_count;
        }

        if (Data_BS_Remain()>=8*cnt)
        {
            size_t End=Data_BS_Remain()-8*cnt;
            while (Data_BS_Remain()>End+7)
            {
                Get_S1 (2,bs_extension_id,                      "bs_extension_id");
                switch (bs_extension_id)
                {
                    case 2 : ps_data(End); break; //EXTENSION_ID_PS
                    default: ;
                }
            }
            if (End<Data_BS_Remain())
                Skip_BS(Data_BS_Remain()-End,                   "bs_fill_bits");
        }
        else
            Skip_BS(Data_BS_Remain(),                           "(Error)");

    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_grid(bool ch)
{
    Element_Begin1("sbr_grid");
    int8u bs_frame_class, bs_num_rel_0, bs_num_rel_1, tmp;
    int ptr_bits;
    Get_S1(2, bs_frame_class,                                   "bs_frame_class");
    switch (bs_frame_class)
    {
        case 0 : //FIXFIX
            Get_S1 (2, tmp,                                     "tmp");
            sbr->bs_num_env[ch]=(int8u)pow(2.0, tmp);
            if (sbr->bs_num_env[ch]==1)
                sbr->bs_amp_res[ch]=0;
            Get_SB (   sbr->bs_freq_res[ch][0],                 "bs_freq_res[ch][0]");
            for (int8u env=1; env<sbr->bs_num_env[ch]; env++)
                sbr->bs_freq_res[ch][env]=sbr->bs_freq_res[ch][0];
            break;
        case 1 : //FIXVAR
            Skip_S1(2,                                          "bs_var_bord_1[ch]");
            Get_S1 (2, bs_num_rel_1,                            "bs_num_rel_1[ch]");
            sbr->bs_num_env[ch]=bs_num_rel_1+1;
            for (int8u rel=0; rel<sbr->bs_num_env[ch]-1; rel++) {
                Skip_S1(2,                                      "tmp");
            }
            ptr_bits=(int8u)ceil(log((double)sbr->bs_num_env[ch]+1)/log((double)2));
            Skip_BS(ptr_bits,                                   "bs_pointer[ch]");
            Element_Begin1("bs_freq_res[ch]");
            for (int8u env=0; env<sbr->bs_num_env[ch]; env++)
                Get_SB (sbr->bs_freq_res[ch][sbr->bs_num_env[ch]-1-env], "bs_freq_res[ch][bs_num_env[ch]-1-env]");
            Element_End0();
            break;
        case 2 : //VARFIX
            Skip_S1(2,                                          "bs_var_bord_0[ch]");
            Get_S1 (2,bs_num_rel_0,                             "bs_num_rel_0[ch]");
            sbr->bs_num_env[ch] = bs_num_rel_0 + 1;
            for (int8u rel=0; rel<sbr->bs_num_env[ch]-1; rel++)
                Skip_S1(2,                                      "tmp");
            ptr_bits=(int8u)ceil(log((double)sbr->bs_num_env[ch]+1)/log((double)2));
            Skip_BS(ptr_bits,                                   "bs_pointer[ch]");
            Element_Begin1("bs_freq_res[ch]");
            for (int8u env = 0; env < sbr->bs_num_env[ch]; env++)
                Get_SB (sbr->bs_freq_res[ch][env],              "bs_freq_res[ch][env]");
            Element_End0();
            break;
        case 3 : //VARVAR
            Skip_S1(2,                                          "bs_var_bord_0[ch]");
            Skip_S1(2,                                          "bs_var_bord_1[ch]");
            Get_S1 (2,bs_num_rel_0,                             "bs_num_rel_0[ch]");
            Get_S1 (2,bs_num_rel_1,                             "bs_num_rel_1[ch]");
            sbr->bs_num_env[ch] = bs_num_rel_0 + bs_num_rel_1 + 1;
            for (int8u rel=0; rel<bs_num_rel_0; rel++)
                Skip_S1(2,                                      "tmp");
            for (int8u rel=0; rel<bs_num_rel_1; rel++)
                Skip_S1(2,                                      "tmp");
            ptr_bits=(int8u)ceil(log((double)(sbr->bs_num_env[ch]+1))/log((double)2));
            Skip_BS(ptr_bits,                                   "bs_pointer[ch]");
            Element_Begin1("bs_freq_res[ch]");
            for (int8u env=0; env<sbr->bs_num_env[ch]; env++)
                Get_SB (sbr->bs_freq_res[ch][env],              "bs_freq_res[ch][env]");
            Element_End0();
            break;
    }
    if (sbr->bs_num_env[ch]>1)
        sbr->bs_num_noise[ch]=2;
    else
        sbr->bs_num_noise[ch]=1;
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_channel_pair_element()
{
    Element_Begin1("sbr_channel_pair_element");
    bool bs_data_extra,bs_coupling,bs_add_harmonic_flag,bs_extended_data;
    int8u bs_extension_size,bs_esc_count,bs_extension_id;
    Get_SB (bs_data_extra,                                      "bs_data_extra");
    if (bs_data_extra) {
        Skip_S1(4,                                              "bs_reserved");
        Skip_S1(4,                                              "bs_reserved");
    }

    Get_SB (bs_coupling,                                        "bs_coupling");
    sbr_grid(0);
    if (bs_coupling)
    {
        //Coupling
        sbr->bs_num_env    [1]=sbr->bs_num_env    [0];
        sbr->bs_num_noise  [1]=sbr->bs_num_noise  [0];
        for (int8u env=0; env<sbr->bs_num_env[0]; env++)
            sbr->bs_freq_res[1][env]=sbr->bs_freq_res[0][env];
    }
    else
        sbr_grid(1);
    sbr_dtdf(0);
    sbr_dtdf(1);
    sbr_invf(0);
    if (!bs_coupling)
        sbr_invf(1);
    sbr_envelope(0, bs_coupling);
    if (bs_coupling)
    {
        sbr_noise(0, bs_coupling);
        sbr_envelope(1, bs_coupling);
    }
    else
    {
        sbr_envelope(1, bs_coupling);
        sbr_noise(0, bs_coupling);
    }
    sbr_noise(1, bs_coupling);
    Get_SB (bs_add_harmonic_flag,                               "bs_add_harmonic_flag[0]");
    if (bs_add_harmonic_flag)
        sbr_sinusoidal_coding(0);
    Get_SB (bs_add_harmonic_flag,                               "bs_add_harmonic_flag[1]");
    if (bs_add_harmonic_flag)
        sbr_sinusoidal_coding(1);

    Get_SB (bs_extended_data,                                   "bs_extended_data");
    if (bs_extended_data) {
        Get_S1(4,bs_extension_size,                             "bs_extension_size");
        size_t cnt = bs_extension_size;
        if (cnt == 15)
        {
            Get_S1(8,bs_esc_count,                              "bs_esc_count");
            cnt += bs_esc_count;
        }
        if (Data_BS_Remain()>=8*cnt)
        {
            size_t End=Data_BS_Remain()-8*cnt;
            while (Data_BS_Remain()>End+7)
            {
                Get_S1 (2,bs_extension_id,                      "bs_extension_id");
                switch (bs_extension_id)
                {
                    case 2 : ps_data(End); break; //EXTENSION_ID_PS
                    default: ;
                }
            }
            if (End<Data_BS_Remain())
                Skip_BS(Data_BS_Remain()-End,                   "bs_fill_bits");
        }
        else
            Skip_BS(Data_BS_Remain(),                           "(Error)");

    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_dtdf(bool ch)
{
    Element_Begin1("sbr_dtdf");
    for (int env=0; env<sbr->bs_num_env[ch]; env++)
        Get_S1 (1, sbr->bs_df_env[ch][env],                     "bs_df_env[ch][env]");
    for (int noise=0; noise<sbr->bs_num_noise[ch]; noise++)
        Get_S1 (1, sbr->bs_df_noise[ch][noise],                 "bs_df_noise[ch][noise]");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_invf(bool)
{
    Element_Begin1("sbr_invf");
    for (int n = 0; n<sbr->num_noise_bands; n++ )
         Skip_S1(2,                                             "bs_invf_mode[ch][n]");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_envelope(bool ch, bool bs_coupling)
{
    sbr_huffman t_huff, f_huff;
    Element_Begin1("sbr_envelope");
    if (bs_coupling && ch)
    {
        if (sbr->bs_amp_res[ch])
        {
            t_huff = t_huffman_env_bal_3_0dB;
            f_huff = f_huffman_env_bal_3_0dB;
        }
        else
        {
            t_huff = t_huffman_env_bal_1_5dB;
            f_huff = f_huffman_env_bal_1_5dB;
        }
    }
    else
    {
        if (sbr->bs_amp_res[ch])
        {
            t_huff = t_huffman_env_3_0dB;
            f_huff = f_huffman_env_3_0dB;
        }
        else
        {
            t_huff = t_huffman_env_1_5dB;
            f_huff = f_huffman_env_1_5dB;
        }
    }

    for (int8u env=0; env<sbr->bs_num_env[ch]; env++)
    {
        if (sbr->bs_df_env[ch][env] == 0)
        {
            if (bs_coupling && ch)
                Skip_S1(sbr->bs_amp_res[ch]?5:6,                "bs_env_start_value_balance");
            else
                Skip_S1(sbr->bs_amp_res[ch]?6:7,                "bs_env_start_value_level");
            for (int8u band = 1; band < sbr->num_env_bands[sbr->bs_freq_res[ch][env]]; band++)
                sbr_huff_dec(f_huff,                            "bs_data_env[ch][env][band]");
        }
        else
        {
            for (int8u band = 0; band < sbr->num_env_bands[sbr->bs_freq_res[ch][env]]; band++)
                sbr_huff_dec(t_huff,                            "bs_data_env[ch][env][band]");
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_noise(bool ch, bool bs_coupling)
{
    sbr_huffman t_huff, f_huff;
    Element_Begin1("sbr_noise");
    if (bs_coupling && ch)
    {
        t_huff = t_huffman_noise_bal_3_0dB;
        f_huff = f_huffman_env_bal_3_0dB;
    }
    else
    {
        t_huff = t_huffman_noise_3_0dB;
        f_huff = f_huffman_env_3_0dB;
    }

    for (int noise=0; noise<sbr->bs_num_noise[ch]; noise++)
    {
        if (sbr->bs_df_noise[ch][noise] == 0)
        {
            Skip_S1(5,                                          (bs_coupling && ch)?"bs_noise_start_value_balance":"bs_noise_start_value_level");
            for (int8u band=1; band<sbr->num_noise_bands; band++)
                sbr_huff_dec(f_huff,                            "bs_data_noise[ch][noise][band]");
        }
        else
        {
            for (int8u band = 0; band < sbr->num_noise_bands; band++)
                sbr_huff_dec(t_huff,                            "bs_data_noise[ch][noise][band]");
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Aac::sbr_sinusoidal_coding(bool)
{
    Element_Begin1("sbr_sinusoidal_coding");
    for (int8u n=0; n<sbr->num_env_bands[1]; n++)
        Skip_SB(                                                "bs_add_harmonic[ch][n]");
    Element_End0();
}

} //NameSpace

#endif //MEDIAINFO_AAC_YES

