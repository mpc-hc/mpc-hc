// File_Mpeg4 - Info for MPEG-4 files
// Copyright (C) 2004-2010 MediaArea.net SARL, Info@MediaArea.net
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
#ifndef MediaInfo_File_Mpeg4H
#define MediaInfo_File_Mpeg4H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <map>
class File_MpegPs;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg4
//***************************************************************************

class File_Mpeg4 : public File__Analyze
{
protected :
    //Streams management
    void Streams_Finish();

public :
    File_Mpeg4();

private :
    //Buffer
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();
    bool BookMark_Needed();

    //Elements
    void free();
    void ftyp();
    void idat();
    void idsc();
    void jp2c();
    void jp2h();
    void jp2h_ihdr();
    void jp2h_colr();
    void mdat();
    void mdat_xxxx();
    void mdat_StreamClear();
    void mdat_StreamJump();
    void mfra();
    void mfra_mfro();
    void mfra_tfra();
    void moof();
    void moof_mfhd();
    void moof_traf();
    void moof_traf_sdtp();
    void moof_traf_tfhd();
    void moof_traf_trun();
    void moov();
    void moov_cmov();
    void moov_cmov_cmvd();
    void moov_cmov_cmvd_zlib();
    void moov_cmov_dcom();
    void moov_ctab();
    void moov_iods();
    void moov_meta();
    void moov_meta_hdlr();
    void moov_meta_bxml();
    void moov_meta_keys();
    void moov_meta_keys_mdta();
    void moov_meta_ilst();
    void moov_meta_ilst_xxxx();
    void moov_meta_ilst_xxxx_data();
    void moov_meta_ilst_xxxx_mean();
    void moov_meta_ilst_xxxx_name();
    void moov_meta_xml();
    void moov_mvex();
    void moov_mvex_mehd();
    void moov_mvex_trex();
    void moov_mvhd();
    void moov_trak();
    void moov_trak_edts();
    void moov_trak_edts_elst();
    void moov_trak_load();
    void moov_trak_mdia();
    void moov_trak_mdia_hdlr();
    void moov_trak_mdia_imap();
    void moov_trak_mdia_imap_sean();
    void moov_trak_mdia_imap_sean___in();
    void moov_trak_mdia_imap_sean___in___ty();
    void moov_trak_mdia_imap_sean___in_dtst();
    void moov_trak_mdia_imap_sean___in_obid();
    void moov_trak_mdia_mdhd();
    void moov_trak_mdia_minf();
    void moov_trak_mdia_minf_code();
    void moov_trak_mdia_minf_code_sean();
    void moov_trak_mdia_minf_code_sean_RU_A();
    void moov_trak_mdia_minf_dinf();
    void moov_trak_mdia_minf_dinf_url_();
    void moov_trak_mdia_minf_dinf_urn_();
    void moov_trak_mdia_minf_dinf_dref();
    void moov_trak_mdia_minf_dinf_dref_alis();
    void moov_trak_mdia_minf_dinf_dref_rsrc();
    void moov_trak_mdia_minf_gmhd();
    void moov_trak_mdia_minf_gmhd_gmin();
    void moov_trak_mdia_minf_gmhd_tmcd();
    void moov_trak_mdia_minf_gmhd_tmcd_tcmi();
    void moov_trak_mdia_minf_hint();
    void moov_trak_mdia_minf_hdlr();
    void moov_trak_mdia_minf_hmhd();
    void moov_trak_mdia_minf_nmhd();
    void moov_trak_mdia_minf_smhd();
    void moov_trak_mdia_minf_vmhd();
    void moov_trak_mdia_minf_stbl();
    void moov_trak_mdia_minf_stbl_cslg();
    void moov_trak_mdia_minf_stbl_co64();
    void moov_trak_mdia_minf_stbl_ctts();
    void moov_trak_mdia_minf_stbl_sdtp();
    void moov_trak_mdia_minf_stbl_stco();
    void moov_trak_mdia_minf_stbl_stdp();
    void moov_trak_mdia_minf_stbl_stps();
    void moov_trak_mdia_minf_stbl_stsc();
    void moov_trak_mdia_minf_stbl_stsd();
    void moov_trak_mdia_minf_stbl_stsd_text();
    void moov_trak_mdia_minf_stbl_stsd_tmcd();
    void moov_trak_mdia_minf_stbl_stsd_tmcd_name();
    void moov_trak_mdia_minf_stbl_stsd_tx3g();
    void moov_trak_mdia_minf_stbl_stsd_tx3g_ftab();
    void moov_trak_mdia_minf_stbl_stsd_xxxx();
    void moov_trak_mdia_minf_stbl_stsd_xxxxSound();
    void moov_trak_mdia_minf_stbl_stsd_xxxxVideo();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_avcC();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_alac();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_bitr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_btrt();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_chan();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_clap();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_colr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_d263();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_dac3();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_dec3();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_damr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_esds();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_idfm();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_pasp();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave_acbf();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave_enda();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave_frma();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave_samr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave_srcq();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_wave_xxxx();
    void moov_trak_mdia_minf_stbl_stsh();
    void moov_trak_mdia_minf_stbl_stss();
    void moov_trak_mdia_minf_stbl_stsz();
    void moov_trak_mdia_minf_stbl_stts();
    void moov_trak_tapt();
    void moov_trak_tapt_clef();
    void moov_trak_tapt_prof();
    void moov_trak_tapt_enof();
    void moov_trak_tkhd();
    void moov_trak_tref();
    void moov_trak_tref_dpnd();
    void moov_trak_tref_ipir();
    void moov_trak_tref_hint();
    void moov_trak_tref_mpod();
    void moov_trak_tref_ssrc();
    void moov_trak_tref_sync();
    void moov_trak_tref_tmcd();
    void moov_udta();
    void moov_udta_AllF();
    void moov_udta_chpl();
    void moov_udta_clsf();
    void moov_udta_cprt();
    void moov_udta_DcMD();
    void moov_udta_DcMD_Cmbo();
    void moov_udta_DcMD_DcME();
    void moov_udta_DcMD_DcME_Keyw();
    void moov_udta_DcMD_DcME_Mtmd();
    void moov_udta_DcMD_DcME_Rate();
    void moov_udta_FIEL();
    void moov_udta_FXTC();
    void moov_udta_hinf();
    void moov_udta_hinv();
    void moov_udta_hnti();
    void moov_udta_hnti_rtp ();
    void moov_udta_ID32();
    void moov_udta_kywd();
    void moov_udta_loci();
    void moov_udta_LOOP();
    void moov_udta_MCPS();
    void moov_udta_meta();
    void moov_udta_meta_hdlr();
    void moov_udta_meta_ilst();
    void moov_udta_meta_ilst_xxxx();
    void moov_udta_meta_ilst_xxxx_data();
    void moov_udta_meta_ilst_xxxx_mean();
    void moov_udta_meta_ilst_xxxx_name();
    void moov_udta_ndrm();
    void moov_udta_nsav();
    void moov_udta_rtng();
    void moov_udta_ptv ();
    void moov_udta_Sel0();
    void moov_udta_tags();
    void moov_udta_tags_meta();
    void moov_udta_tags_tseg();
    void moov_udta_tags_tseg_tshd();
    void moov_udta_WLOC();
    void moov_udta_XMP_();
    void moov_udta_yrrc();
    void moov_udta_xxxx();
    void PICT();
    void pckg();
    void pnot();
    void skip();
    void wide();

    //Helpers
    bool Element_Level_Get();
    bool Element_Name_Get();
    bool Element_Size_Get();
    Ztring Language_Get(int16u Language);
    enum method
    {
        Method_None,
        Method_String,
        Method_String2,
        Method_String3,
        Method_Integer,
        Method_Binary
    };
    method Metadata_Get(std::string &Parameter, int64u Meta);
    method Metadata_Get(std::string &Parameter, const std::string &Meta);
    void Descriptors();

    //Temp
    bool List;
    bool                                    mdat_MustParse;
    bool                                    moov_Done;
    int32u                                  moov_cmov_dcom_Compressor;
    int32u                                  moov_meta_hdlr_Type;
    std::string                             moov_meta_ilst_xxxx_name_Name;
    int32u                                  moov_trak_mdia_mdhd_Duration;
    int32u                                  moov_trak_mdia_mdhd_TimeScale;
    int32u                                  moov_trak_tkhd_TrackID;
    float32                                 moov_trak_tkhd_Width;
    float32                                 moov_trak_tkhd_Height;
    float32                                 moov_trak_tkhd_DisplayAspectRatio;
    float32                                 moov_trak_tkhd_Rotation;
    std::vector<std::string>                moov_udta_meta_keys_List;
    size_t                                  moov_udta_meta_keys_ilst_Pos;
    int32u                                  TimeScale;
    int32u                                  Vendor;
    Ztring                                  Vendor_Version;

    //Data
    struct stream
    {
        Ztring                  File_Name;
        File__Analyze*          Parser;
        stream_t                StreamKind;
        size_t                  StreamPos;
        std::vector<int64u>     stco;
        struct stsc_struct
        {
            int32u FirstChunk;
            int32u SamplesPerChunk;
        };
        std::vector<stsc_struct> stsc;
        std::vector<int64u>     stsz;
        int64u                  stsz_Sample_Size;
        int64u                  stsz_Sample_Count;
        int32u                  TimeCode_TrackID;
        bool                    TimeCode_IsVisual;
        float32                 CleanAperture_Width;
        float32                 CleanAperture_Height;
        float32                 CleanAperture_PixelAspectRatio;

        stream()
        {
            Parser=NULL;
            StreamKind=Stream_Max;
            StreamPos=0;
            stsz_Sample_Size=0;
            stsz_Sample_Count=0;
            TimeCode_TrackID=(int32u)-1;
            TimeCode_IsVisual=false;
            CleanAperture_Width=0;
            CleanAperture_Height=0;
            CleanAperture_PixelAspectRatio=0;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
        }
    };
    std::map<int32u, stream> Stream;

    //Positions
    struct mdat_Pos_Type
    {
        int32u StreamID;
        int64u Size;
    };
    std::map<int64u, mdat_Pos_Type> mdat_Pos;
    bool IsParsing_mdat;
};

} //NameSpace

#endif
