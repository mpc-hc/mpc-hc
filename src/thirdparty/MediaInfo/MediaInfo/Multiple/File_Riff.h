/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about RIFF files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_RiffH
#define MediaInfo_File_RiffH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include <MediaInfo/Multiple/File_Ancillary.h>
#endif //defined(MEDIAINFO_ANCILLARY_YES)
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Riff
//***************************************************************************

class File_Riff : public File__Analyze
{
public :
    //In/Out
    #if defined(MEDIAINFO_ANCILLARY_YES)
        File_Ancillary** Ancillary;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)

protected :
    //Streams management
    void Streams_Finish();

public :
    File_Riff();
    ~File_Riff();

private :
    //Buffer - Global
    void Read_Buffer_Init();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    #if MEDIAINFO_DEMUX
    void Read_Buffer_Continue ();
    #endif //MEDIAINFO_DEMUX
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    bool BookMark_Needed();

    //Data
    struct stream
    {
        std::vector<File__Analyze*> Parsers;
        int32u                  fccType;
        int32u                  fccHandler;
        int32u                  Scale;
        int32u                  Rate;
        int32u                  Start;
        int32u                  Length;
        int32u                  Compression;
        stream_t                StreamKind;
        size_t                  StreamPos;
        int32u                  AvgBytesPerSec;
        size_t                  PacketPos;
        size_t                  PacketCount;
        int64u                  StreamSize;
        int64u                  indx_Duration;
        bool                    SearchingPayload;
        bool                    Specific_IsMpeg4v;
        bool                    ChunksAreComplete;
        bool                    IsPcm;

        stream()
        {
            fccType=0x00000000;
            fccHandler=0x00000000;
            Scale=0;
            Rate=0;
            Start=0;
            Length=0;
            Compression=0x00000000;
            StreamKind=Stream_Max;
            StreamPos=0;
            AvgBytesPerSec=0;
            PacketPos=0;
            PacketCount=0;
            StreamSize=0;
            indx_Duration=0;
            SearchingPayload=true;
            Specific_IsMpeg4v=false;
            ChunksAreComplete=true;
            IsPcm=false;
        }

        ~stream()
        {
            for (size_t Pos=0; Pos<Parsers.size(); Pos++)
                delete Parsers[Pos]; //Parser=NULL;
        }
    };
    std::map<int32u, stream> Stream;
    int32u                   Stream_ID;

    struct stream_structure
    {
        int64u                  Name;
        int64u                  Size;
    };
    std::map<int64u, stream_structure> Stream_Structure;
    std::map<int64u, stream_structure>::iterator Stream_Structure_Temp;
    std::map<int64u, int64u> Index_Pos;
    int64u Interleaved0_1;
    int64u Interleaved0_10;
    int64u Interleaved1_1;
    int64u Interleaved1_10;

    //Temp
    Ztring INFO_ISMP;
    Ztring Tdat_tc_A;
    Ztring Tdat_tc_O;
    ZtringList MD5s;
    int64u WAVE_data_Size;  //RF64 WAVE_data real chunk size
    int64u WAVE_fact_samplesCount;  //RF64 WAVE_fact real samplesCount
    int64u Alignement_ExtraByte; //Padding from the container
    int64u Buffer_DataToParse_Begin;
    int64u Buffer_DataToParse_End;
    int32u AvgBytesPerSec;
    int16u BlockAlign;
    float64 PAR;
    float64 Demux_Rate;
    float64 avih_FrameRate; //FrameRate of the first video stream in one MOVI chunk
    int32u avih_TotalFrame; //Count of frames in one MOVI chunk
    int32u dmlh_TotalFrame; //Count of frames in the whole AVI file (with odml too)
    int64u Idx1_Offset;     //Pos of the data part (AVI) for Idx1 chunk
    int64u movi_Size;       //Size of the data part (AVI and AVIX)
    int64u TimeReference;   //Only used by Brodcast extension
    int32u SMV_BlockSize;   //Size of a SMV block, 0 if not SMV
    int32u SMV_FrameCount;  //Frame count of a SMV block, 0 if not SMV
    int32u SamplesPerSec;   //For bext
    int16u BitsPerSample;   //For PCM only
    int8u  stream_Count;    //How many stream we have to parse
    bool   rec__Present;    //True if synchro element is present
    bool   NeedOldIndex;
    bool   IsBigEndian;
    bool   IsWave64;
    bool   IsRIFF64;
    bool   IsWaveBroken;
    bool   IsNotWordAligned;
    bool   IsNotWordAligned_Tested;
    bool   SecondPass;      //Second pass for streams
    File__Analyze*  DV_FromHeader;
    enum kind
    {
        Kind_None,
        Kind_Avi,
        Kind_Wave,
        Kind_Aiff,
        Kind_Rmp3,
    };
    kind Kind;
    #if defined(MEDIAINFO_GXF_YES)
        int32u rcrd_fld__anc__pos__LineNumber;
    #endif //defined(MEDIAINFO_GXF_YES)

    void TimeCode_Fill(const Ztring &Name, const Ztring &Value);

    //Chunks
    void AIFC ();
    void AIFC_COMM ();
    void AIFC_COMT ();
    void AIFC_FVER ();
    void AIFC_SSND ();
    void AIFF_SSND_Continue();
    void AIFC_xxxx ();
    void AIFF ();
    void AIFF_COMM ();
    void AIFF_COMT ();
    void AIFF_ID3_ () {WAVE_ID3_();}
    void AIFF_SSND ();
    void AIFF_xxxx ();
    void AVI_ ();
    void AVI__cset ();
    void AVI__Cr8r ();
    void AVI__exif ();
    void AVI__exif_xxxx ();
    void AVI__goog ();
    void AVI__goog_GDAT ();
    void AVI__hdlr ();
    void AVI__hdlr_avih ();
    void AVI__hdlr_JUNK ();
    void AVI__hdlr_strl ();
    void AVI__hdlr_strl_indx ();
    void AVI__hdlr_strl_indx_StandardIndex (int32u Entry_Count, int32u ChunkId);
    void AVI__hdlr_strl_indx_FieldIndex (int32u Entry_Count, int32u ChunkId);
    void AVI__hdlr_strl_indx_SuperIndex (int32u Entry_Count, int32u ChunkId);
    void AVI__hdlr_strl_JUNK ();
    void AVI__hdlr_strl_strd ();
    void AVI__hdlr_strl_strf ();
    void AVI__hdlr_strl_strf_auds ();
    void AVI__hdlr_strl_strf_auds_Mpega();
    void AVI__hdlr_strl_strf_auds_Aac();
    void AVI__hdlr_strl_strf_auds_Vorbis();
    void AVI__hdlr_strl_strf_auds_Vorbis2();
    void AVI__hdlr_strl_strf_auds_ExtensibleWave();
    void AVI__hdlr_strl_strf_iavs ();
    void AVI__hdlr_strl_strf_mids ();
    void AVI__hdlr_strl_strf_txts ();
    void AVI__hdlr_strl_strf_vids ();
    void AVI__hdlr_strl_strf_vids_Avc ();
    void AVI__hdlr_strl_strf_vids_Ffv1();
    void AVI__hdlr_strl_strf_vids_HuffYUV(int16u BitCount, int32u Height);
    void AVI__hdlr_strl_strh ();
    void AVI__hdlr_strl_strn ();
    void AVI__hdlr_strl_vprp ();
    void AVI__hdlr_odml ();
    void AVI__hdlr_odml_dmlh ();
    void AVI__hdlr_ON2h ();
    void AVI__hdlr_xxxx ();
    void AVI__idx1 ();
    void AVI__INFO ();
    void AVI__INFO_IID3 ();
    void AVI__INFO_ILYC ();
    void AVI__INFO_IMP3 ();
    void AVI__INFO_JUNK ();
    void AVI__INFO_xxxx ();
    void AVI__JUNK ();
    void AVI__MD5_ ();
    void AVI__movi ();
    void AVI__movi_xxxx ();
    void AVI__movi_xxxx___dc ();
    void AVI__movi_xxxx___tx ();
    void AVI__movi_xxxx___wb ();
    void AVI__movi_rec_ ();
    void AVI__movi_rec__xxxx ();
    void AVI__movi_StreamJump ();
    void AVI__PrmA ();
    void AVI__Tdat ();
    void AVI__Tdat_rn_A ();
    void AVI__Tdat_rn_O ();
    void AVI__Tdat_tc_A ();
    void AVI__Tdat_tc_O ();
    void AVI__GMET ();
    void AVI__xxxx ();
    void AVIX ();
    void AVIX_idx1 ();
    void AVIX_movi ();
    void AVIX_movi_xxxx ();
    void AVIX_movi_rec_ ();
    void AVIX_movi_rec__xxxx ();
    void CADP ();
    void CDDA ();
    void CDDA_fmt_ ();
    void CMJP ();
    void CMP4 ();
    void IDVX ();
    void INDX ();
    void INDX_xxxx ();
    void JUNK ();
    void menu ();
    void MThd ();
    void MTrk ();
    void PAL_ ();
    void QLCM ();
    void QLCM_fmt_ ();
    #if defined(MEDIAINFO_GXF_YES)
    void rcrd ();
    void rcrd_desc ();
    void rcrd_fld_ ();
    void rcrd_fld__anc_ ();
    void rcrd_fld__anc__pos_ ();
    void rcrd_fld__anc__pyld ();
    void rcrd_fld__finf ();
    #endif //defined(MEDIAINFO_GXF_YES)
    void RDIB ();
    void RMID ();
    void RMMP ();
    void RMP3 ();
    void RMP3_data ();
    void RMP3_data_Continue();
    void RMP3_INFO() {AVI__INFO();}
    void RMP3_INFO_IID3() {AVI__INFO_IID3();}
    void RMP3_INFO_ILYC() {AVI__INFO_ILYC();}
    void RMP3_INFO_IMP3() {AVI__INFO_IMP3();}
    void RMP3_INFO_JUNK() {AVI__INFO_JUNK ();}
    void RMP3_INFO_xxxx() {AVI__INFO_xxxx ();}
    void SMV0 ();
    void SMV0_xxxx ();
    void WAVE ();
    void WAVE__pmx ();
    void WAVE_aXML ();
    void WAVE_bext ();
    void WAVE_cue_ ();
    void WAVE_data ();
    void WAVE_data_Continue ();
    void WAVE_ds64 ();
    void WAVE_fact ();
    void WAVE_fmt_ ();
    void WAVE_ID3_ ();
    void WAVE_id3_ () {WAVE_ID3_();}
    void WAVE_INFO() {AVI__INFO();}
    void WAVE_INFO_xxxx() {AVI__INFO_xxxx ();}
    void WAVE_iXML ();
    void wave ();
    void wave_data () {WAVE_data();}
    void wave_fmt_ () {WAVE_fmt_();}
    void W3DI();

    //Temp
    #if MEDIAINFO_DEMUX
    File__Analyze*  Demux_Parser;
    #endif //MEDIAINFO_DEMUX
};

} //NameSpace

#endif
