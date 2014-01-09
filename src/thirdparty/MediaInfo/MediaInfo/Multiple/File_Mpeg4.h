/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Mpeg4H
#define MediaInfo_File_Mpeg4H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/MediaInfo_Internal.h"
class File_MpegPs;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class File__ReferenceFilesHelper;

//***************************************************************************
// Class File_Mpeg4
//***************************************************************************

class File_Mpeg4 : public File__Analyze
{
protected :
    //Streams management
    void Streams_Accept();
    void Streams_Finish();
    void Streams_Finish_CommercialNames ();

public :
    File_Mpeg4();
    ~File_Mpeg4();

private :
    //Buffer - Global
    void Read_Buffer_Init();
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();
    bool BookMark_Needed();

    //Elements
    void bloc();
    void cdat();
    void cdt2() {cdat();}
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
    void moov_ainf();
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
    void moov_trak_mdia_minf_gmhd_tcmi();
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
    void moov_trak_mdia_minf_stbl_stsd_xxxxStream();
    void moov_trak_mdia_minf_stbl_stsd_xxxxText();
    void moov_trak_mdia_minf_stbl_stsd_xxxxVideo();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_alac();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_ACLR();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_APRG();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_ARES();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_avcC();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_bitr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_btrt();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_chan();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_clap();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_colr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_d263();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_dac3();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_damr();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_dec3();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_ddts();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_dvc1();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_esds();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_fiel();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_glbl();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_hvcC();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_idfm();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h() {jp2h();}
    void moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h_colr() {jp2h_colr();}
    void moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h_ihdr() {jp2h_ihdr();}
    void moov_trak_mdia_minf_stbl_stsd_xxxx_pasp();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_sinf();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_frma();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_imif();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schm();
    void moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schi();
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
    void moov_trak_mdia_minf_stbl_stts_Common(int32u SampleCount, int32u SampleDuration, int32u Pos=0, int32u NumberOfEntries=1);
    void moov_trak_mdia_minf_stbl_stz2() {moov_trak_mdia_minf_stbl_stsz();}
    void moov_trak_meta() {moov_meta();}
    void moov_trak_meta_hdlr() {moov_meta_hdlr();}
    void moov_trak_meta_bxml() {moov_meta_bxml();}
    void moov_trak_meta_keys() {moov_meta_keys();}
    void moov_trak_meta_keys_mdta() {moov_meta_keys_mdta();}
    void moov_trak_meta_ilst() {moov_meta_ilst();}
    void moov_trak_meta_ilst_xxxx() {moov_meta_ilst_xxxx();}
    void moov_trak_meta_ilst_xxxx_data() {moov_meta_ilst_xxxx_data();}
    void moov_trak_meta_ilst_xxxx_mean() {moov_meta_ilst_xxxx_mean();}
    void moov_trak_meta_ilst_xxxx_name() {moov_meta_ilst_xxxx_name();}
    void moov_trak_meta_xml() {moov_meta_xml();}
    void moov_trak_tapt();
    void moov_trak_tapt_clef();
    void moov_trak_tapt_prof();
    void moov_trak_tapt_enof();
    void moov_trak_tkhd();
    void moov_trak_tref();
    void moov_trak_tref_chap();
    void moov_trak_tref_clcp();
    void moov_trak_tref_dpnd();
    void moov_trak_tref_fall();
    void moov_trak_tref_folw();
    void moov_trak_tref_forc();
    void moov_trak_tref_ipir();
    void moov_trak_tref_hint();
    void moov_trak_tref_mpod();
    void moov_trak_tref_scpt();
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
    void pdin();
    void PICT();
    void pckg();
    void pnot();
    void RDAO();
    void RDAS();
    void RDVO();
    void RDVS();
    void RED1();
    void REDA();
    void REDV();
    void REOB();
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
    void TimeCode_Associate(int32u TrackID);

    //Temp
    bool List;
    bool                                    mdat_MustParse;
    int32u                                  moov_cmov_dcom_Compressor;
    int32u                                  moov_meta_hdlr_Type;
    std::string                             moov_meta_ilst_xxxx_name_Name;
    size_t                                  moov_trak_mdia_minf_stbl_stsd_Pos;
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
    int64u                                  FirstMdatPos;
    int64u                                  LastMdatPos; //This is the position of the byte after the last byte of mdat
    int64u                                  FirstMoovPos;
    int64u                                  moof_base_data_offset;
    int64u                                  FrameCount_MaxPerStream;
    bool                                    data_offset_present;
    int64u                                  moof_traf_base_data_offset;
    int32u                                  moof_traf_default_sample_duration;
    int32u                                  moof_traf_default_sample_size;
    int32u                                  MajorBrand;
    bool                                    IsSecondPass;
    bool                                    IsParsing_mdat;
    bool                                    IsFragmented;

    //Data
    struct stream
    {
        Ztring                  File_Name;
        std::vector<File__Analyze*> Parsers;
        MediaInfo_Internal*     MI;
        struct timecode
        {
            int32u TimeScale;
            int32u FrameDuration;
            bool   DropFrame;
            bool   H24;
            bool   NegativeTimes;
        };
        timecode* TimeCode;
        stream_t                StreamKind;
        size_t                  StreamPos;
        struct edts_struct
        {
            int32u  Duration;
            int32u  Delay;
            int32u  Rate;
        };
        std::vector<edts_struct> edts;
        std::vector<int64u>     stco;
        struct stsc_struct
        {
            int32u FirstChunk;
            int32u SamplesPerChunk;
        };
        std::vector<stsc_struct> stsc;
        std::vector<int64u>     stsz;
        std::vector<int64u>     stsz_Total; //TODO: merge with stsz
        int64u                  stsz_StreamSize; //TODO: merge with stsz
        std::vector<int64u>     stss; //Sync Sample, base=0
        struct stts_struct
        {
            int32u SampleCount;
            int32u SampleDuration;
        };
        std::vector<stts_struct> stts;
        int64u                  stsz_Sample_Size;
        int64u                  stsz_Sample_Multiplier;
        int64u                  stsz_Sample_Count;
        int64u                  tkhd_Duration;
        int32u                  mdhd_TimeScale;
        int64u                  mdhd_Duration;
        int32u                  stts_Min;
        int32u                  stts_Max;
        int64u                  stts_FrameCount;
        int64u                  stts_Duration;
        int64u                  stts_Duration_FirstFrame;
        int64u                  stts_Duration_LastFrame;
        int64u                  stts_SampleDuration;
        int32u                  mvex_trex_default_sample_duration;
        int32u                  mvex_trex_default_sample_size;
        int32u                  TimeCode_TrackID;
        bool                    TimeCode_IsVisual;
        bool                    IsPcm;
        bool                    IsPcmMono;
        bool                    IsPriorityStream;
        bool                    IsFilled;
        bool                    IsChapter;
        bool                    ScanOrder_StoredDisplayedInverted;
        float32                 CleanAperture_Width;
        float32                 CleanAperture_Height;
        float32                 CleanAperture_PixelAspectRatio;
        #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
            int8u               Demux_Level;
            int64u              Demux_Offset;

            struct stts_duration
            {
                int64u Pos_Begin;
                int64u Pos_End;
                int64u DTS_Begin;
                int64u DTS_End;
                int32u SampleDuration;
            };
            typedef std::vector<stts_duration> stts_durations;
            stts_durations  stts_Durations;
            size_t          stts_Durations_Pos;
            int64u          stts_FramePos;
        #endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK
        #if MEDIAINFO_DEMUX
            bool            PtsDtsAreSame;
            bool            Demux_EventWasSent;
        #endif //MEDIAINFO_DEMUX

        stream()
        {
            MI=NULL;
            TimeCode=NULL;
            StreamKind=Stream_Max;
            StreamPos=0;
            stsz_StreamSize=0;
            stsz_Sample_Size=0;
            stsz_Sample_Multiplier=1;
            stsz_Sample_Count=0;
            tkhd_Duration=0;
            mdhd_TimeScale=0;
            mdhd_Duration=0;
            stts_Min=(int32u)-1;
            stts_Max=0;
            stts_FrameCount=0;
            stts_Duration=0;
            stts_Duration_FirstFrame=0;
            stts_Duration_LastFrame=0;
            mvex_trex_default_sample_duration=0;
            mvex_trex_default_sample_size=0;
            TimeCode_TrackID=(int32u)-1;
            TimeCode_IsVisual=false;
            IsPcm=false;
            IsPcmMono=false;
            IsPriorityStream=false;
            IsFilled=false;
            IsChapter=false;
            ScanOrder_StoredDisplayedInverted=false;
            CleanAperture_Width=0;
            CleanAperture_Height=0;
            CleanAperture_PixelAspectRatio=0;
            #if MEDIAINFO_DEMUX
                Demux_Level=2; //Container
                Demux_Offset=0;
                stts_Durations_Pos=0;
                stts_FramePos=0;
            #endif //MEDIAINFO_DEMUX
            #if MEDIAINFO_DEMUX
                PtsDtsAreSame=false;
                Demux_EventWasSent=false;
            #endif //MEDIAINFO_DEMUX
        }

        ~stream()
        {
            for (size_t Pos=0; Pos<Parsers.size(); Pos++)
                delete Parsers[Pos];
            delete MI; //MI=NULL;
            delete TimeCode; //TimeCode=NULL;
        }
    };
    typedef std::map<int32u, stream> streams;
    streams             Streams;
    streams::iterator   Stream;
    #if defined(MEDIAINFO_REFERENCES_YES)
        File__ReferenceFilesHelper* ReferenceFiles;
    #endif //defined(MEDIAINFO_REFERENCES_YES)
    #if MEDIAINFO_NEXTPACKET
        bool                    ReferenceFiles_IsParsing;
    #endif //MEDIAINFO_NEXTPACKET

    //Positions
    struct mdat_Pos_Type
    {
        int32u StreamID;
        int64u Size;
    };
    typedef std::map<int64u, mdat_Pos_Type> mdat_pos;
    mdat_pos mdat_Pos;
    mdat_pos::iterator mdat_Pos_Temp;
    std::vector<int32u> mdat_Pos_ToParseInPriority_StreamIDs;
    bool                mdat_Pos_NormalParsing;

    #if MEDIAINFO_DEMUX
        int64u          TimeCode_FrameOffset;
        int64u          TimeCode_DtsOffset;
        std::map<int64u, int64u> StreamOffset_Jump; //Key is the current position, value is the jump position
    #endif //MEDIAINFO_DEMUX
};

} //NameSpace

#endif
