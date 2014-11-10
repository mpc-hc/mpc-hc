/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Matroska files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_MatroskaH
#define MediaInfo_File_MatroskaH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Classe File_Matroska
//***************************************************************************

class File_Mk : public File__Analyze
{
protected :
    //Streams management
    void Streams_Finish();

public :
    File_Mk();
    ~File_Mk();
    void Segment_Attachements_AttachedFile_FileName();

private :
    //Buffer
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Zero();
    void CRC32();
    void Void();
    void Ebml();
    void Ebml_Version();
    void Ebml_ReadVersion();
    void Ebml_MaxIDLength();
    void Ebml_MaxSizeLength();
    void Ebml_DocType();
    void Ebml_DocTypeVersion();
    void Ebml_DocTypeReadVersion();
    void Segment();
    void Segment_Attachements();
    void Segment_Attachements_AttachedFile();
    void Segment_Attachements_AttachedFile_FileData();
    void Segment_Attachements_AttachedFile_FileDescription();
    void Segment_Attachements_AttachedFile_FileMimeType();
    void Segment_Attachements_AttachedFile_FileReferral();
    void Segment_Attachements_AttachedFile_FileUID();
    void Segment_Chapters();
    void Segment_Chapters_EditionEntry();
    void Segment_Chapters_EditionEntry_ChapterAtom();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCodecID();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessData();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessTime();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessPrivate();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapCountry();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapLanguage();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapString();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagHidden();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagEnabled();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterPhysicalEquiv();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentEditionUID();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentUID();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeEnd();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeStart();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack_ChapterTrackNumber();
    void Segment_Chapters_EditionEntry_ChapterAtom_ChapterUID();
    void Segment_Chapters_EditionEntry_EditionFlagDefault();
    void Segment_Chapters_EditionEntry_EditionFlagHidden();
    void Segment_Chapters_EditionEntry_EditionFlagOrdered();
    void Segment_Chapters_EditionEntry_EditionUID();
    void Segment_Cluster();
    void Segment_Cluster_BlockGroup();
    void Segment_Cluster_BlockGroup_Block();
    void Segment_Cluster_BlockGroup_BlockAdditions();
    void Segment_Cluster_BlockGroup_BlockAdditions_BlockMore();
    void Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAddID();
    void Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAdditional();
    void Segment_Cluster_BlockGroup_BlockDuration();
    void Segment_Cluster_BlockGroup_ReferenceBlock();
    void Segment_Cluster_BlockGroup_ReferencePriority();
    void Segment_Cluster_BlockGroup_Slices();
    void Segment_Cluster_BlockGroup_Slices_TimeSlice();
    void Segment_Cluster_BlockGroup_Slices_TimeSlice_Duration();
    void Segment_Cluster_BlockGroup_Slices_TimeSlice_LaceNumber();
    void Segment_Cluster_Position();
    void Segment_Cluster_PrevSize();
    void Segment_Cluster_SilentTracks();
    void Segment_Cluster_SilentTracks_SilentTrackNumber();
    void Segment_Cluster_SimpleBlock();
    void Segment_Cluster_Timecode();
    void Segment_Cues();
    void Segment_Cues_CuePoint();
    void Segment_Cues_CuePoint_CueTime();
    void Segment_Cues_CuePoint_CueTrackPositions();
    void Segment_Cues_CuePoint_CueTrackPositions_CueTrack();
    void Segment_Cues_CuePoint_CueTrackPositions_CueClusterPosition();
    void Segment_Cues_CuePoint_CueTrackPositions_CueBlockNumber();
    void Segment_Info();
    void Segment_Info_ChapterTranslate();
    void Segment_Info_ChapterTranslate_ChapterTranslateCodec();
    void Segment_Info_ChapterTranslate_ChapterTranslateEditionUID();
    void Segment_Info_ChapterTranslate_ChapterTranslateID();
    void Segment_Info_DateUTC();
    void Segment_Info_Duration();
    void Segment_Info_MuxingApp();
    void Segment_Info_NextFilename();
    void Segment_Info_NextUID();
    void Segment_Info_PrevFilename();
    void Segment_Info_PrevUID();
    void Segment_Info_SegmentFamily();
    void Segment_Info_SegmentFilename();
    void Segment_Info_SegmentUID();
    void Segment_Info_TimecodeScale();
    void Segment_Info_Title();
    void Segment_Info_WritingApp();
    void Segment_SeekHead();
    void Segment_SeekHead_Seek();
    void Segment_SeekHead_Seek_SeekID();
    void Segment_SeekHead_Seek_SeekPosition();
    void Segment_Tags();
    void Segment_Tags_Tag();
    void Segment_Tags_Tag_SimpleTag();
    void Segment_Tags_Tag_SimpleTag_TagBinary();
    void Segment_Tags_Tag_SimpleTag_TagDefault();
    void Segment_Tags_Tag_SimpleTag_TagLanguage();
    void Segment_Tags_Tag_SimpleTag_TagName();
    void Segment_Tags_Tag_SimpleTag_TagString();
    void Segment_Tags_Tag_Targets();
    void Segment_Tags_Tag_Targets_AttachmentUID();
    void Segment_Tags_Tag_Targets_ChapterUID();
    void Segment_Tags_Tag_Targets_EditionUID();
    void Segment_Tags_Tag_Targets_TargetType();
    void Segment_Tags_Tag_Targets_TargetTypeValue();
    void Segment_Tags_Tag_Targets_TrackUID();
    void Segment_Tracks();
    void Segment_Tracks_TrackEntry();
    void Segment_Tracks_TrackEntry_AttachmentLink();
    void Segment_Tracks_TrackEntry_Audio();
    void Segment_Tracks_TrackEntry_Audio_BitDepth();
    void Segment_Tracks_TrackEntry_Audio_Channels();
    void Segment_Tracks_TrackEntry_Audio_OutputSamplingFrequency();
    void Segment_Tracks_TrackEntry_Audio_SamplingFrequency();
    void Segment_Tracks_TrackEntry_CodecDecodeAll();
    void Segment_Tracks_TrackEntry_CodecID();
    void Segment_Tracks_TrackEntry_ContentEncodings() {};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding() {};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Order() {UInteger_Info();};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Scope() {UInteger_Info();};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Type() {UInteger_Info();};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression();
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompAlgo();
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompSettings();
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption() {};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentEncAlgo() {UInteger_Info();};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentEncKeyID() {Skip_XX(Element_Size, "Data");};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSignature() {Skip_XX(Element_Size, "Data");};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigKeyID() {Skip_XX(Element_Size, "Data");};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigAlgo() {UInteger_Info();};
    void Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigHashAlgo() {UInteger_Info();};
    void Segment_Tracks_TrackEntry_CodecName();
    void Segment_Tracks_TrackEntry_CodecPrivate();
    void Segment_Tracks_TrackEntry_CodecPrivate_auds();
    void Segment_Tracks_TrackEntry_CodecPrivate_auds_ExtensibleWave();
    void Segment_Tracks_TrackEntry_CodecPrivate_vids();
    void Segment_Tracks_TrackEntry_DefaultDuration();
    void Segment_Tracks_TrackEntry_FlagDefault();
    void Segment_Tracks_TrackEntry_FlagEnabled();
    void Segment_Tracks_TrackEntry_FlagForced();
    void Segment_Tracks_TrackEntry_FlagLacing();
    void Segment_Tracks_TrackEntry_Language();
    void Segment_Tracks_TrackEntry_MaxBlockAdditionID();
    void Segment_Tracks_TrackEntry_MaxCache();
    void Segment_Tracks_TrackEntry_MinCache();
    void Segment_Tracks_TrackEntry_Name();
    void Segment_Tracks_TrackEntry_TrackNumber();
    void Segment_Tracks_TrackEntry_TrackTimecodeScale();
    void Segment_Tracks_TrackEntry_TrackType();
    void Segment_Tracks_TrackEntry_TrackUID();
    void Segment_Tracks_TrackEntry_Video();
    void Segment_Tracks_TrackEntry_Video_AspectRatioType();
    void Segment_Tracks_TrackEntry_Video_ColourSpace();
    void Segment_Tracks_TrackEntry_Video_DisplayHeight();
    void Segment_Tracks_TrackEntry_Video_DisplayUnit();
    void Segment_Tracks_TrackEntry_Video_DisplayWidth();
    void Segment_Tracks_TrackEntry_Video_FlagInterlaced();
    void Segment_Tracks_TrackEntry_Video_FrameRate();
    void Segment_Tracks_TrackEntry_Video_PixelCropBottom();
    void Segment_Tracks_TrackEntry_Video_PixelCropLeft();
    void Segment_Tracks_TrackEntry_Video_PixelCropRight();
    void Segment_Tracks_TrackEntry_Video_PixelCropTop();
    void Segment_Tracks_TrackEntry_Video_PixelHeight();
    void Segment_Tracks_TrackEntry_Video_PixelWidth();
    void Segment_Tracks_TrackEntry_Video_StereoMode();
    void Segment_Tracks_TrackEntry_Video_StereoModeBuggy() {Segment_Tracks_TrackEntry_Video_StereoMode();}
    void Segment_Tracks_TrackEntry_TrackOverlay();
    void Segment_Tracks_TrackEntry_TrackTranslate();
    void Segment_Tracks_TrackEntry_TrackTranslate_Codec();
    void Segment_Tracks_TrackEntry_TrackTranslate_EditionUID();
    void Segment_Tracks_TrackEntry_TrackTranslate_TrackID();

    struct stream
    {
        std::vector<int64u>     TimeCodes;
        int64u                  TimeCode_Start;
        File__Analyze*          Parser;
        stream_t                StreamKind;
        size_t                  StreamPos;
        size_t                  PacketCount;
        int32u                  AvgBytesPerSec; //Only used by x_MS/* codecIDs
        float32                 DisplayAspectRatio;
        float64                 FrameRate;
        bool                    Searching_Payload;
        bool                    Searching_TimeStamps;
        bool                    Searching_TimeStamp_Start;
        bool                    Default;
        bool                    Forced;
        int64u                  ContentCompAlgo;
        size_t                  ContentCompSettings_Buffer_Size;
        int8u*                  ContentCompSettings_Buffer;
        std::map<std::string, Ztring> Infos;
        int64u                  TrackDefaultDuration;
        std::map<int64u, int64u> Segment_Cluster_BlockGroup_BlockDuration_Counts;

        stream()
        {
            TimeCode_Start=(int64u)-1;
            Parser=NULL;
            StreamKind=Stream_Max;
            StreamPos=0;
            PacketCount=0;
            AvgBytesPerSec=0;
            DisplayAspectRatio=0;
            FrameRate=0;
            Searching_Payload=false;
            Searching_TimeStamps=false;
            Searching_TimeStamp_Start=false;
            Default=true;
            Forced=false;
            ContentCompAlgo=(int32u)-1;
            ContentCompSettings_Buffer_Size=0;
            ContentCompSettings_Buffer=NULL;
            TrackDefaultDuration=0;
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
            delete[] ContentCompSettings_Buffer; //ContentCompSettings_Buffer=NULL;
        }
    };
    std::map<int64u, stream> Stream;
    size_t                   Stream_Count;

    //Data
    int64u   UInteger_Get();
    int128u  UInteger16_Get();
    void     UInteger_Info();

    float64  Float_Get();
    void     Float_Info();

    Ztring   UTF8_Get();
    void     UTF8_Info();

    Ztring   Local_Get();
    void     Local_Info();

    //Temp - TrackEntry
    int8u*   CodecPrivate;
    size_t   CodecPrivate_Size;
    void     CodecPrivate_Manage();
    Ztring   CodecID;
    infocodecid_format_t InfoCodecID_Format_Type;
    void     CodecID_Manage();
    int64u   TrackType;

    //Temp
    int64u  Format_Version;
    int64u  TimecodeScale;
    float64 Duration;
    int64u  TrackNumber;
    int64u  TrackVideoDisplayWidth;
    int64u  TrackVideoDisplayHeight;
    int32u  AvgBytesPerSec;
    int64u  Segment_Cluster_TimeCode_Value;
    bool    Cluster_AlreadyParsed;

    //Chapters
    struct chapterdisplay
    {
        Ztring ChapLanguage;
        Ztring ChapString;
    };
    struct chapteratom
    {
        int64u ChapterTimeStart;
        std::vector<chapterdisplay> ChapterDisplays;

        chapteratom()
        {
            ChapterTimeStart=(int64u)-1;
        }
    };
    struct editionentry
    {
        std::vector<chapteratom> ChapterAtoms;
    };
    std::vector<editionentry> EditionEntries;
    size_t EditionEntries_Pos;
    size_t ChapterAtoms_Pos;
    size_t ChapterDisplays_Pos;
    int64u              Segment_Offset_Begin;
    int64u              Segment_Offset_End;
    std::vector<int64u> Segment_Seeks;
    size_t              Segment_Seeks_Pos;
    int64u              Segment_Tag_TrackUID;
    std::vector<Ztring> Segment_Tag_SimpleTag_TagNames;
    int64u Segment_Cluster_BlockGroup_BlockDuration_Value;
    int64u Segment_Cluster_BlockGroup_BlockDuration_TrackNumber;
};

} //NameSpace

#endif

