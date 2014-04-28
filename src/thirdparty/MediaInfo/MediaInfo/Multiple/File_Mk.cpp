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
#if defined(MEDIAINFO_MK_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mk.h"
#if defined(MEDIAINFO_OGG_YES)
    #include "MediaInfo/Multiple/File_Ogg.h"
#endif
#if defined(MEDIAINFO_RM_YES)
    #include "MediaInfo/Multiple/File_Rm.h"
#endif
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_HEVC_YES)
    #include "MediaInfo/Video/File_Hevc.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif
#if defined(MEDIAINFO_DIRAC_YES)
    #include "MediaInfo/Video/File_Dirac.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_VP8_YES)
    #include "MediaInfo/Video/File_Vp8.h"
#endif
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_DTS_YES)
    #include "MediaInfo/Audio/File_Dts.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_FLAC_YES)
    #include "MediaInfo/Audio/File_Flac.h"
#endif
#if defined(MEDIAINFO_OPUS_YES)
    #include "MediaInfo/Audio/File_Opus.h"
#endif
#if defined(MEDIAINFO_WVPK_YES)
    #include "MediaInfo/Audio/File_Wvpk.h"
#endif
#if defined(MEDIAINFO_TTA_YES)
    #include "MediaInfo/Audio/File_Tta.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include <cstring>
#include <algorithm>
#if MEDIAINFO_DEMUX
    #include "base64.h"
#endif //MEDIAINFO_DEMUX
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mk_ContentCompAlgo(int64u Algo)
{
    switch (Algo)
    {
        case 0x00 : return "zlib";
        case 0x01 : return "bzlib";
        case 0x02 : return "lzo1x";
        case 0x03 : return "Header stripping";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mk_StereoMode(int64u StereoMode)
{
    switch (StereoMode)
    {
        case 0x00 : return ""; //Mono (default)
        case 0x01 : return "Side by Side (left eye first)";
        case 0x02 : return "Top-Bottom (right eye first)";
        case 0x03 : return "Top-Bottom (left eye first)";
        case 0x04 : return "Checkboard (right eye first)";
        case 0x05 : return "Checkboard (left eye first)";
        case 0x06 : return "Row Interleaved (right eye first)";
        case 0x07 : return "Row Interleaved (left eye first)";
        case 0x08 : return "Column Interleaved (right eye first)";
        case 0x09 : return "Column Interleaved (left eye first)";
        case 0x0A : return "Anaglyph (cyan/red)";
        case 0x0B : return "Side by Side (right eye first)";
        case 0x0C : return "Anaglyph (green/magenta)";
        case 0x0D : return "Both Eyes laced in one block (left eye first)";
        case 0x0E : return "Both Eyes laced in one block (right eye first)";
        default   : return "Unknown";
    }
}

//---------------------------------------------------------------------------
const char* Mk_StereoMode_v2(int64u StereoMode)
{
    switch (StereoMode)
    {
        case 0x00 : return ""; //Mono (default)
        case 0x01 : return "Right Eye";
        case 0x02 : return "Left Eye";
        case 0x03 : return "Both Eye";
        default   : return "Unknown";
    }
}

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
extern std::string ExtensibleWave_ChannelMask (int32u ChannelMask);

//---------------------------------------------------------------------------
extern std::string ExtensibleWave_ChannelMask2 (int32u ChannelMask);

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mk::File_Mk()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Matroska;
        StreamIDs_Width[0]=16;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    DataMustAlwaysBeComplete=false;

    //Temp
    Format_Version=0;
    TimecodeScale=1000000; //Default value
    Duration=0;
    Cluster_AlreadyParsed=false;

    //Helpers
    CodecPrivate=NULL;
}

//---------------------------------------------------------------------------
File_Mk::~File_Mk()
{
    delete[] CodecPrivate; //CodecPrivate=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mk::Streams_Finish()
{
    if (Duration!=0 && TimecodeScale!=0)
        Fill(Stream_General, 0, General_Duration, Duration*int64u_float64(TimecodeScale)/1000000.0, 0);
    for (std::map<int64u, stream>::iterator Temp=Stream.begin(); Temp!=Stream.end(); ++Temp)
    {
        StreamKind_Last=Temp->second.StreamKind;
        StreamPos_Last=Temp->second.StreamPos;

        if (Temp->second.DisplayAspectRatio!=0)
        {
            //Corrections
            if (Temp->second.DisplayAspectRatio>=1.777 && Temp->second.DisplayAspectRatio<=1.778)
                Temp->second.DisplayAspectRatio=((float32)16)/9;
            if (Temp->second.DisplayAspectRatio>=1.333 && Temp->second.DisplayAspectRatio<=1.334)
                Temp->second.DisplayAspectRatio=((float32)4)/3;
            Fill(Stream_Video, Temp->second.StreamPos, Video_DisplayAspectRatio, Temp->second.DisplayAspectRatio, 3, true);
            int64u Width=Retrieve(Stream_Video, Temp->second.StreamPos, Video_Width).To_int64u();
            int64u Height=Retrieve(Stream_Video, Temp->second.StreamPos, Video_Height).To_int64u();
            if (Width)
                Fill(Stream_Video, Temp->second.StreamPos, Video_PixelAspectRatio, Temp->second.DisplayAspectRatio*Height/Width, 3, true);
        }

        if (Temp->second.Parser)
        {
            Fill(Temp->second.Parser);
            if (Config->ParseSpeed<=1.0)
                Temp->second.Parser->Open_Buffer_Unsynch();
        }

        //Video specific
        if (StreamKind_Last==Stream_Video)
        {
            //FrameRate
            bool IsVfr=false;
            if (Temp->second.Segment_Cluster_BlockGroup_BlockDuration_Counts.size()>2)
                IsVfr=true;
            else if (Temp->second.TimeCodes.size()>1)
            {
                //Trying to detect VFR
                std::vector<int64s> FrameRate_Between;
                std::sort(Temp->second.TimeCodes.begin(), Temp->second.TimeCodes.end()); //This is PTS, no DTS --> Some frames are out of order
                size_t FramesToAdd=0;
                for (size_t Pos=1; Pos<Temp->second.TimeCodes.size(); Pos++)
                {
                    int64u Duration=Temp->second.TimeCodes[Pos]-Temp->second.TimeCodes[Pos-1];
                    if (Duration)
                        FrameRate_Between.push_back(Duration);
                    else
                        FramesToAdd++;
                }
                if (FrameRate_Between.size()>1)
                {
                    int64s FrameRate_Between_Last=FrameRate_Between[FrameRate_Between.size()-1];
                    size_t Pos=FrameRate_Between.size()-2;
                    while (Pos)
                    {
                        if (!(FrameRate_Between[Pos]*0.9<FrameRate_Between_Last
                           && FrameRate_Between[Pos]*1.1>FrameRate_Between_Last))
                            break;
                        Pos--;
                    }
                    if (Pos)
                        FrameRate_Between.resize(Pos+1); //We peek 40 frames, and remove the last ones, because this is PTS, no DTS --> Some frames are out of order
                }
                if (FrameRate_Between.size()>=30+1+FramesToAdd)
                    FrameRate_Between.resize((FrameRate_Between.size()-4>30+1+FramesToAdd)?(FrameRate_Between.size()-4):(30+1+FramesToAdd)); //We peek 40 frames, and remove the last ones, because this is PTS, no DTS --> Some frames are out of order
                std::sort(FrameRate_Between.begin(), FrameRate_Between.end());
                if (!FrameRate_Between.empty()
                 && FrameRate_Between[0]*0.9<FrameRate_Between[FrameRate_Between.size()-1]
                 && FrameRate_Between[0]*1.1>FrameRate_Between[FrameRate_Between.size()-1])
                {
                    float Time=0;
                    if (Temp->second.TimeCodes.size()>30+FramesToAdd)
                        Time=(float)(Temp->second.TimeCodes[30+FramesToAdd]-Temp->second.TimeCodes[0])/30; //30 frames for handling 30 fps rounding problems
                    else if (Temp->second.TrackDefaultDuration)
                        Time=(float)Temp->second.TrackDefaultDuration/TimecodeScale; //TrackDefaultDuration is maybe more precise than the time code
                    else
                        Time=(float)(Temp->second.TimeCodes[Temp->second.TimeCodes.size()-1]-Temp->second.TimeCodes[0])/(Temp->second.TimeCodes.size()-1);
                    if (Time)
                    {
                        float32 FrameRate_FromCluster=1000000000/Time/TimecodeScale;
                        if (Temp->second.Parser)
                        {
                            float32 FrameRate_FromParser=Temp->second.Parser->Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate).To_float32();
                            if (FrameRate_FromParser
                             && FrameRate_FromParser*2>FrameRate_FromCluster*0.9
                             && FrameRate_FromParser*2<FrameRate_FromCluster*1.1) //TODO: awfull method to detect interlaced content with one field per block
                                FrameRate_FromCluster/=2;
                        }
                        Fill(Stream_Video, StreamPos_Last, Video_FrameRate, FrameRate_FromCluster);
                    }
                }
                else if (!FrameRate_Between.empty())
                    IsVfr=true;
            }
            else if (Temp->second.TrackDefaultDuration)
            {
                float32 FrameRate_FromCluster=1000000000/(float32)Temp->second.TrackDefaultDuration;
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate, FrameRate_FromCluster);
            }

            Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode, IsVfr?"VFR":"CFR");
        }

        if (Temp->second.Parser)
        {
            //Delay
            if (Temp->second.TimeCode_Start!=(int64u)-1 && TimecodeScale)
            {
                //From TimeCode
                float64 Delay=Temp->second.TimeCode_Start*int64u_float64(TimecodeScale)/1000000.0;

                //From stream format
                if (StreamKind_Last==Stream_Audio && Count_Get(Stream_Video)==1 && Temp->second.Parser->Count_Get(Stream_General)>0)
                {
                         if (Temp->second.Parser->Buffer_TotalBytes_FirstSynched==0)
                        ;
                    else if (Temp->second.AvgBytesPerSec!=0)
                        Delay+=((float64)Temp->second.Parser->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.AvgBytesPerSec;
                    else if (Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u()!=0)
                        Delay+=((float64)Temp->second.Parser->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int64u();
                    else if (Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate_Nominal).To_int64u()!=0)
                        Delay+=((float64)Temp->second.Parser->Buffer_TotalBytes_FirstSynched)*1000/Temp->second.Parser->Retrieve(Stream_Audio, 0, Audio_BitRate_Nominal).To_int64u();
                }

                //Filling
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), Delay, 0, true);
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
            }

            Ztring Duration_Temp, Codec_Temp;
            Duration_Temp=Retrieve(StreamKind_Last, Temp->second.StreamPos, Fill_Parameter(StreamKind_Last, Generic_Duration)); //Duration from stream is sometimes false
            Codec_Temp=Retrieve(StreamKind_Last, Temp->second.StreamPos, Fill_Parameter(StreamKind_Last, Generic_Codec)); //We want to keep the 4CC

            Finish(Temp->second.Parser);
            Merge(*Temp->second.Parser, Temp->second.StreamKind, 0, Temp->second.StreamPos);
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), Duration_Temp, true);
            if (Temp->second.StreamKind==Stream_Video && !Codec_Temp.empty())
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), Codec_Temp, true);

            //Special case: AAC
            if (StreamKind_Last==Stream_Audio
             && (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("AAC")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("MPEG Audio")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("Vorbis")))
                Clear(Stream_Audio, StreamPos_Last, Audio_BitDepth); //Resolution is not valid for AAC / MPEG Audio / Vorbis

            //Special case: 5.1
            if (StreamKind_Last==Stream_Audio
             && (Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("AC-3")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("E-AC-3")
              || Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("DTS"))
             && Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s__Original)==__T("6")
             && Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s_)==__T("5"))
            {
                Clear(Stream_Audio, StreamPos_Last, Audio_Channel_s__Original);
                Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, 6, 10, true); //Some muxers do not count LFE in the channel count, let's say it is normal
            }

            //VFR
            if (Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate_Mode)==__T("VFR") && Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate_Original).empty())
            {
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Original, Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate));
                Clear(Stream_Video, StreamPos_Last, Video_FrameRate);
            }
        }

        if (Temp->second.FrameRate!=0 && Retrieve(Stream_Video, Temp->second.StreamPos, Video_FrameRate).empty())
            Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate, Temp->second.FrameRate, 3);

        //Flags
        Fill(Temp->second.StreamKind, Temp->second.StreamPos, "Default", Temp->second.Default?"Yes":"No");
        Fill(Temp->second.StreamKind, Temp->second.StreamPos, "Forced", Temp->second.Forced?"Yes":"No");
    }

    //Chapters
    if (TimecodeScale)
    {
        for (EditionEntries_Pos=0; EditionEntries_Pos<EditionEntries.size(); EditionEntries_Pos++)
        {
            Stream_Prepare(Stream_Menu);
            Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
            for (ChapterAtoms_Pos=0; ChapterAtoms_Pos<EditionEntries[EditionEntries_Pos].ChapterAtoms.size(); ChapterAtoms_Pos++)
            {
                if (EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterTimeStart!=(int64u)-1)
                {
                    Ztring Text;
                    for (ChapterDisplays_Pos=0; ChapterDisplays_Pos<EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays.size(); ChapterDisplays_Pos++)
                    {
                        Ztring PerLanguage;
                        if (!EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays[ChapterDisplays_Pos].ChapLanguage.empty())
                            PerLanguage=MediaInfoLib::Config.Iso639_1_Get(EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays[ChapterDisplays_Pos].ChapLanguage)+__T(':');
                        PerLanguage+=EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays[ChapterDisplays_Pos].ChapString;
                        Text+=PerLanguage+__T(" - ");
                    }
                    if (Text.size())
                        Text.resize(Text.size()-3);
                    Fill(Stream_Menu, StreamPos_Last, Ztring().Duration_From_Milliseconds(EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterTimeStart/1000000).To_UTF8().c_str(), Text);
                }
            }
            Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
        }
    }

    //Attachments
    for (size_t Pos=0; Pos<AttachedFiles.size(); Pos++)
    {
        if (Ztring(AttachedFiles[Pos]).MakeLowerCase().find(__T("cover"))==string::npos)
            Fill(Stream_General, 0, "Attachment", __T("Yes"));
        else
            Fill(Stream_General, 0, "Cover", __T("Yes"));
    }

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Stream.clear();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mk::Header_Parse()
{
    //Test of zero padding
    int8u Null;
    Peek_B1(Null);
    if (Null==0x00)
    {
        if (Buffer_Offset_Temp==0)
            Buffer_Offset_Temp=Buffer_Offset+1;

        while (Buffer_Offset_Temp<Buffer_Size)
        {
            if (Buffer[Buffer_Offset_Temp])
                break;
            Buffer_Offset_Temp++;
        }
        if (Buffer_Offset_Temp>=Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }

        Header_Fill_Code((int32u)-1); //Should be (int64u)-1 but Borland C++ does not like this
        Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
        Buffer_Offset_Temp=0;

        return;
    }

    //Parsing
    int64u Name, Size;
    Get_EB (Name,                                               "Name");
    Get_EB (Size,                                               "Size");

    //Filling
    Header_Fill_Code(Name, Ztring().From_Number(Name, 16));
    Header_Fill_Size(Element_Offset+Size);
}

//---------------------------------------------------------------------------
namespace Elements
{
    //Common
    const int64u Zero=(int32u)-1; //Should be (int64u)-1 but Borland C++ does not like this
    const int64u CRC32=0x3F;
    const int64u Void=0x6C;

    //EBML
    const int64u Ebml=0xA45DFA3;
    const int64u Ebml_Version=0x286;
    const int64u Ebml_ReadVersion=0x2F7;
    const int64u Ebml_MaxIDLength=0x2F2;
    const int64u Ebml_MaxSizeLength=0x2F3;
    const int64u Ebml_DocType=0x282;
    const int64u Ebml_DocTypeVersion=0x287;
    const int64u Ebml_DocTypeReadVersion=0x285;

    //Segment
    const int64u Segment=0x8538067;
    const int64u Segment_Attachements=0x0941A469;
    const int64u Segment_Attachements_AttachedFile=0x21A7;
    const int64u Segment_Attachements_AttachedFile_FileData=0x065C;
    const int64u Segment_Attachements_AttachedFile_FileDescription=0x067E;
    const int64u Segment_Attachements_AttachedFile_FileName=0x066E;
    const int64u Segment_Attachements_AttachedFile_FileMimeType=0x0660;
    const int64u Segment_Attachements_AttachedFile_FileReferral=0x0675;
    const int64u Segment_Attachements_AttachedFile_FileUID=0x06AE;
    const int64u Segment_Chapters=0x43A770;
    const int64u Segment_Chapters_EditionEntry=0x05B9;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom=0x36;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess=0x2944;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCodecID=0x2955;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand=0x2911;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessData=0x2933;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessTime=0x2922;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessPrivate=0x050D;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay=0x00;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapCountry=0x037E;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapLanguage=0x037C;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapString=0x05;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagEnabled=0x0598;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagHidden=0x18;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterPhysicalEquiv=0x23C3;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentEditionUID=0x2EBC;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentUID=0x2E67;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeStart=0x11;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeEnd=0x12;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack=0x0F;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack_ChapterTrackNumber=0x09;
    const int64u Segment_Chapters_EditionEntry_ChapterAtom_ChapterUID=0x33C4;
    const int64u Segment_Chapters_EditionEntry_EditionFlagDefault=0x05DB;
    const int64u Segment_Chapters_EditionEntry_EditionFlagHidden=0x05BD;
    const int64u Segment_Chapters_EditionEntry_EditionFlagOrdered=0x05DD;
    const int64u Segment_Chapters_EditionEntry_EditionUID=0x05BC;
    const int64u Segment_Cluster=0xF43B675;
    const int64u Segment_Cluster_BlockGroup=0x20;
    const int64u Segment_Cluster_BlockGroup_Block=0x21;
    const int64u Segment_Cluster_BlockGroup_BlockAdditions=0x35A1;
    const int64u Segment_Cluster_BlockGroup_BlockAdditions_BlockMore=0x26;
    const int64u Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAddID=0x6E;
    const int64u Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAdditional=0x25;
    const int64u Segment_Cluster_BlockGroup_BlockDuration=0x1B;
    const int64u Segment_Cluster_BlockGroup_ReferenceBlock=0x7B;
    const int64u Segment_Cluster_BlockGroup_ReferencePriority=0x7A;
    const int64u Segment_Cluster_BlockGroup_Slices=0xE;
    const int64u Segment_Cluster_BlockGroup_Slices_TimeSlice=0x68;
    const int64u Segment_Cluster_BlockGroup_Slices_TimeSlice_Duration=0x4F;
    const int64u Segment_Cluster_BlockGroup_Slices_TimeSlice_LaceNumber=0x4C;
    const int64u Segment_Cluster_Position=0x27;
    const int64u Segment_Cluster_PrevSize=0x2B;
    const int64u Segment_Cluster_SilentTracks=0x1854;
    const int64u Segment_Cluster_SilentTracks_SilentTrackNumber=0x18D7;
    const int64u Segment_Cluster_SimpleBlock=0x23;
    const int64u Segment_Cluster_Timecode=0x67;
    const int64u Segment_Cues=0xC53BB6B;
    const int64u Segment_Cues_CuePoint=0x3B;
    const int64u Segment_Cues_CuePoint_CueTime=0x33;
    const int64u Segment_Cues_CuePoint_CueTrackPositions=0x37;
    const int64u Segment_Cues_CuePoint_CueTrackPositions_CueTrack=0x77;
    const int64u Segment_Cues_CuePoint_CueTrackPositions_CueClusterPosition=0x71;
    const int64u Segment_Cues_CuePoint_CueTrackPositions_CueBlockNumber=0x1378;
    const int64u Segment_Info=0x549A966;
    const int64u Segment_Info_ChapterTranslate=0x2924;
    const int64u Segment_Info_ChapterTranslate_ChapterTranslateCodec=0x29BF;
    const int64u Segment_Info_ChapterTranslate_ChapterTranslateEditionUID=0x29FC;
    const int64u Segment_Info_ChapterTranslate_ChapterTranslateID=0x29A5;
    const int64u Segment_Info_DateUTC=0x461;
    const int64u Segment_Info_Duration=0x489;
    const int64u Segment_Info_MuxingApp=0xD80;
    const int64u Segment_Info_NextFilename=0x1E83BB;
    const int64u Segment_Info_NextUID=0x1EB923;
    const int64u Segment_Info_PrevFilename=0x1C83AB;
    const int64u Segment_Info_PrevUID=0x1CB923;
    const int64u Segment_Info_SegmentFamily=0x444;
    const int64u Segment_Info_SegmentFilename=0x3384;
    const int64u Segment_Info_SegmentUID=0x33A4;
    const int64u Segment_Info_TimecodeScale=0xAD7B1;
    const int64u Segment_Info_Title=0x3BA9;
    const int64u Segment_Info_WritingApp=0x1741;
    const int64u Segment_SeekHead=0x14D9B74;
    const int64u Segment_SeekHead_Seek=0xDBB;
    const int64u Segment_SeekHead_Seek_SeekID=0x13AB;
    const int64u Segment_SeekHead_Seek_SeekPosition=0x13AC;
    const int64u Segment_Tags=0x0254C367;
    const int64u Segment_Tags_Tag=0x3373;
    const int64u Segment_Tags_Tag_SimpleTag=0x27C8;
    const int64u Segment_Tags_Tag_SimpleTag_TagBinary=0x485;
    const int64u Segment_Tags_Tag_SimpleTag_TagDefault=0x484;
    const int64u Segment_Tags_Tag_SimpleTag_TagLanguage=0x47A;
    const int64u Segment_Tags_Tag_SimpleTag_TagName=0x5A3;
    const int64u Segment_Tags_Tag_SimpleTag_TagString=0x487;
    const int64u Segment_Tags_Tag_Targets=0x23C0;
    const int64u Segment_Tags_Tag_Targets_AttachmentUID=0x23C6;
    const int64u Segment_Tags_Tag_Targets_ChapterUID=0x23C4;
    const int64u Segment_Tags_Tag_Targets_EditionUID=0x23C9;
    const int64u Segment_Tags_Tag_Targets_TargetType=0x23CA;
    const int64u Segment_Tags_Tag_Targets_TargetTypeValue=0x28CA;
    const int64u Segment_Tags_Tag_Targets_TrackUID=0x23C5;
    const int64u Segment_Tracks=0x654AE6B;
    const int64u Segment_Tracks_TrackEntry=0x2E;
    const int64u Segment_Tracks_TrackEntry_AttachmentLink=0x3446;
    const int64u Segment_Tracks_TrackEntry_Audio=0x61;
    const int64u Segment_Tracks_TrackEntry_Audio_BitDepth=0x2264;
    const int64u Segment_Tracks_TrackEntry_Audio_Channels=0x1F;
    const int64u Segment_Tracks_TrackEntry_Audio_OutputSamplingFrequency=0x38B5;
    const int64u Segment_Tracks_TrackEntry_Audio_SamplingFrequency=0x35;
    const int64u Segment_Tracks_TrackEntry_CodecDecodeAll=0x2A;
    const int64u Segment_Tracks_TrackEntry_CodecID=0x6;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings=0x2D80;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding=0x2240;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Order=0x1031;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Scope=0x1032;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Type=0x1033;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression=0x1034;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompAlgo=0x0254;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompSettings=0x0255;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption=0x1035;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentEncAlgo=0x07E1;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentEncKeyID=0x07E2;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSignature=0x07E3;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigKeyID=0x07E4;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigAlgo=0x07E5;
    const int64u Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigHashAlgo=0x07E6;
    const int64u Segment_Tracks_TrackEntry_CodecName=0x58688;
    const int64u Segment_Tracks_TrackEntry_CodecPrivate=0x23A2;
    const int64u Segment_Tracks_TrackEntry_DefaultDuration=0x3E383;
    const int64u Segment_Tracks_TrackEntry_FlagDefault=0x8;
    const int64u Segment_Tracks_TrackEntry_FlagEnabled=0x39;
    const int64u Segment_Tracks_TrackEntry_FlagForced=0x15AA;
    const int64u Segment_Tracks_TrackEntry_FlagLacing=0x1C;
    const int64u Segment_Tracks_TrackEntry_Language=0x2B59C;
    const int64u Segment_Tracks_TrackEntry_MaxBlockAdditionID=0x15EE;
    const int64u Segment_Tracks_TrackEntry_MaxCache=0x2DF8;
    const int64u Segment_Tracks_TrackEntry_MinCache=0x2DE7;
    const int64u Segment_Tracks_TrackEntry_Name=0x136E;
    const int64u Segment_Tracks_TrackEntry_TrackNumber=0x57;
    const int64u Segment_Tracks_TrackEntry_TrackTimecodeScale=0x3314F;
    const int64u Segment_Tracks_TrackEntry_TrackType=0x3;
    const int64u Segment_Tracks_TrackEntry_TrackUID=0x33C5;
    const int64u Segment_Tracks_TrackEntry_Video=0x60;
    const int64u Segment_Tracks_TrackEntry_Video_AspectRatioType=0x14B3;
    const int64u Segment_Tracks_TrackEntry_Video_ColourSpace=0xEB524;
    const int64u Segment_Tracks_TrackEntry_Video_DisplayHeight=0x14BA;
    const int64u Segment_Tracks_TrackEntry_Video_DisplayUnit=0x14B2;
    const int64u Segment_Tracks_TrackEntry_Video_DisplayWidth=0x14B0;
    const int64u Segment_Tracks_TrackEntry_Video_FlagInterlaced=0x1A;
    const int64u Segment_Tracks_TrackEntry_Video_FrameRate=0x383E3;
    const int64u Segment_Tracks_TrackEntry_Video_PixelCropBottom=0x14AA;
    const int64u Segment_Tracks_TrackEntry_Video_PixelCropLeft=0x14CC;
    const int64u Segment_Tracks_TrackEntry_Video_PixelCropRight=0x14DD;
    const int64u Segment_Tracks_TrackEntry_Video_PixelCropTop=0x14BB;
    const int64u Segment_Tracks_TrackEntry_Video_PixelHeight=0x3A;
    const int64u Segment_Tracks_TrackEntry_Video_PixelWidth=0x30;
    const int64u Segment_Tracks_TrackEntry_Video_StereoMode=0x13B8;
    const int64u Segment_Tracks_TrackEntry_Video_StereoModeBuggy=0x13B9;
    const int64u Segment_Tracks_TrackEntry_TrackOverlay=0x2FAB;
    const int64u Segment_Tracks_TrackEntry_TrackTranslate=0x2624;
    const int64u Segment_Tracks_TrackEntry_TrackTranslate_Codec=0x26BF;
    const int64u Segment_Tracks_TrackEntry_TrackTranslate_EditionUID=0x26FC;
    const int64u Segment_Tracks_TrackEntry_TrackTranslate_TrackID=0x26A5;
}

//---------------------------------------------------------------------------
void File_Mk::Data_Parse()
{
    #define LIS2(_ATOM, _NAME) \
        case Elements::_ATOM : \
                if (Level==Element_Level) \
                { \
                    Element_Name(_NAME); \
                    _ATOM(); \
                    Element_ThisIsAList(); \
                } \

    #define ATO2(_ATOM, _NAME) \
                case Elements::_ATOM : \
                        if (Level==Element_Level) \
                        { \
                            if (Element_IsComplete_Get()) \
                            { \
                                Element_Name(_NAME); \
                                _ATOM(); \
                            } \
                            else \
                            { \
                                Element_WaitForMoreData(); \
                                return; \
                            } \
                        } \
                        break; \

    #define ATOM_END_MK \
        ATOM(Zero) \
        ATOM(CRC32) \
        ATOM(Void) \
        ATOM_END

    //Parsing
    DATA_BEGIN
    LIST(Ebml)
        ATOM_BEGIN
        ATOM(Ebml_Version)
        ATOM(Ebml_ReadVersion)
        ATOM(Ebml_MaxIDLength)
        ATOM(Ebml_MaxSizeLength)
        ATOM(Ebml_DocType)
        ATOM(Ebml_DocTypeVersion)
        ATOM(Ebml_DocTypeReadVersion)
        ATOM_END_MK
    LIST(Segment)
        ATOM_BEGIN
        LIST(Segment_Attachements)
            ATOM_BEGIN
            LIST(Segment_Attachements_AttachedFile)
                ATOM_BEGIN
                LIST_SKIP(Segment_Attachements_AttachedFile_FileData) //This is ATOM, but some ATOMs are too big
                ATOM(Segment_Attachements_AttachedFile_FileDescription)
                ATOM(Segment_Attachements_AttachedFile_FileName)
                ATOM(Segment_Attachements_AttachedFile_FileMimeType)
                ATOM(Segment_Attachements_AttachedFile_FileReferral)
                ATOM(Segment_Attachements_AttachedFile_FileUID)
                ATOM_END_MK
            ATOM_END_MK
        LIST(Segment_Chapters)
            ATOM_BEGIN
            LIST(Segment_Chapters_EditionEntry)
                ATOM_BEGIN
                LIST(Segment_Chapters_EditionEntry_ChapterAtom)
                    ATOM_BEGIN
                    LIST(Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess)
                        ATOM_BEGIN
                        ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCodecID)
                        LIST(Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand)
                            ATOM_BEGIN
                            ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessData)
                            ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessTime)
                            ATOM_END_MK
                        ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessPrivate)
                        ATOM_END_MK
                    LIST(Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay)
                        ATOM_BEGIN
                        ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapCountry)
                        ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapLanguage)
                        ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapString)
                        ATOM_END_MK
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagEnabled)
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagHidden)
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterPhysicalEquiv)
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentEditionUID)
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentUID)
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeEnd)
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeStart)
                    LIST(Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack)
                        ATOM_BEGIN
                        ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack_ChapterTrackNumber)
                        ATOM_END_MK
                    ATOM(Segment_Chapters_EditionEntry_ChapterAtom_ChapterUID)
                    ATOM_END_MK
                ATOM(Segment_Chapters_EditionEntry_EditionFlagDefault)
                ATOM(Segment_Chapters_EditionEntry_EditionFlagHidden)
                ATOM(Segment_Chapters_EditionEntry_EditionFlagOrdered)
                ATOM(Segment_Chapters_EditionEntry_EditionUID)
                ATOM_END_MK
            ATOM_END_MK
        LIST(Segment_Cluster)
            ATOM_BEGIN
            LIST(Segment_Cluster_BlockGroup)
                ATOM_BEGIN
                ATOM(Segment_Cluster_BlockGroup_Block)
                LIST(Segment_Cluster_BlockGroup_BlockAdditions)
                    ATOM_BEGIN
                    LIST(Segment_Cluster_BlockGroup_BlockAdditions_BlockMore)
                        ATOM_BEGIN
                        ATOM(Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAddID)
                        ATOM(Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAdditional)
                        ATOM_END_MK
                    ATOM_END_MK
                ATOM(Segment_Cluster_BlockGroup_BlockDuration)
                ATOM(Segment_Cluster_BlockGroup_ReferenceBlock)
                ATOM(Segment_Cluster_BlockGroup_ReferencePriority)
                LIST(Segment_Cluster_BlockGroup_Slices)
                    ATOM_BEGIN
                    LIST(Segment_Cluster_BlockGroup_Slices_TimeSlice)
                        ATOM_BEGIN
                        ATOM(Segment_Cluster_BlockGroup_Slices_TimeSlice_Duration)
                        ATOM(Segment_Cluster_BlockGroup_Slices_TimeSlice_LaceNumber)
                        ATOM_END_MK
                    ATOM_END_MK
                ATOM_END_MK
            ATOM(Segment_Cluster_Position)
            ATOM(Segment_Cluster_PrevSize)
            LIST(Segment_Cluster_SilentTracks)
                ATOM_BEGIN
                ATOM(Segment_Cluster_SilentTracks_SilentTrackNumber)
                ATOM_END_MK
            ATOM(Segment_Cluster_SimpleBlock)
            ATOM(Segment_Cluster_Timecode)
            ATOM_END_MK
        LIST(Segment_Cues)
            ATOM_BEGIN
            LIST(Segment_Cues_CuePoint)
                ATOM_BEGIN
                ATOM(Segment_Cues_CuePoint_CueTime)
                LIST(Segment_Cues_CuePoint_CueTrackPositions)
                    ATOM_BEGIN
                    ATOM(Segment_Cues_CuePoint_CueTrackPositions_CueTrack)
                    ATOM(Segment_Cues_CuePoint_CueTrackPositions_CueClusterPosition)
                    ATOM(Segment_Cues_CuePoint_CueTrackPositions_CueBlockNumber)
                    ATOM_END_MK
                ATOM_END_MK
            ATOM_END_MK
        LIST(Segment_Info)
            ATOM_BEGIN
            LIST(Segment_Info_ChapterTranslate)
                ATOM_BEGIN
                ATOM(Segment_Info_ChapterTranslate_ChapterTranslateCodec)
                ATOM(Segment_Info_ChapterTranslate_ChapterTranslateEditionUID)
                ATOM(Segment_Info_ChapterTranslate_ChapterTranslateID)
                ATOM_END_MK
            ATOM(Segment_Info_DateUTC)
            ATOM(Segment_Info_Duration)
            ATOM(Segment_Info_MuxingApp)
            ATOM(Segment_Info_NextFilename)
            ATOM(Segment_Info_NextUID)
            ATOM(Segment_Info_PrevFilename)
            ATOM(Segment_Info_PrevUID)
            ATOM(Segment_Info_SegmentFamily)
            ATOM(Segment_Info_SegmentFilename)
            ATOM(Segment_Info_SegmentUID)
            ATOM(Segment_Info_TimecodeScale)
            ATOM(Segment_Info_Title)
            ATOM(Segment_Info_WritingApp)
            ATOM_END_MK
        LIST(Segment_SeekHead)
            ATOM_BEGIN
            LIST(Segment_SeekHead_Seek)
                ATOM_BEGIN
                ATOM(Segment_SeekHead_Seek_SeekID)
                ATOM(Segment_SeekHead_Seek_SeekPosition)
                ATOM_END_MK
            ATOM_END_MK
        LIST(Segment_Tags)
            ATOM_BEGIN
            LIST(Segment_Tags_Tag)
                ATOM_BEGIN
                LIST(Segment_Tags_Tag_SimpleTag)
                    ATOM_BEGIN
                    LIST(Segment_Tags_Tag_SimpleTag)
                        ATOM_BEGIN
                        LIST(Segment_Tags_Tag_SimpleTag)
                            ATOM_BEGIN
                            ATOM(Segment_Tags_Tag_SimpleTag_TagBinary)
                            ATOM(Segment_Tags_Tag_SimpleTag_TagDefault)
                            ATOM(Segment_Tags_Tag_SimpleTag_TagLanguage)
                            ATOM(Segment_Tags_Tag_SimpleTag_TagName)
                            ATOM(Segment_Tags_Tag_SimpleTag_TagString)
                            ATOM_END_MK
                        ATOM(Segment_Tags_Tag_SimpleTag_TagBinary)
                        ATOM(Segment_Tags_Tag_SimpleTag_TagDefault)
                        ATOM(Segment_Tags_Tag_SimpleTag_TagLanguage)
                        ATOM(Segment_Tags_Tag_SimpleTag_TagName)
                        ATOM(Segment_Tags_Tag_SimpleTag_TagString)
                        ATOM_END_MK
                    ATOM(Segment_Tags_Tag_SimpleTag_TagBinary)
                    ATOM(Segment_Tags_Tag_SimpleTag_TagDefault)
                    ATOM(Segment_Tags_Tag_SimpleTag_TagLanguage)
                    ATOM(Segment_Tags_Tag_SimpleTag_TagName)
                    ATOM(Segment_Tags_Tag_SimpleTag_TagString)
                    ATOM_END_MK
                LIST(Segment_Tags_Tag_Targets)
                    ATOM_BEGIN
                    ATOM(Segment_Tags_Tag_Targets_AttachmentUID)
                    ATOM(Segment_Tags_Tag_Targets_ChapterUID)
                    ATOM(Segment_Tags_Tag_Targets_EditionUID)
                    ATOM(Segment_Tags_Tag_Targets_TargetType)
                    ATOM(Segment_Tags_Tag_Targets_TargetTypeValue)
                    ATOM(Segment_Tags_Tag_Targets_TrackUID)
                    ATOM_END_MK
                ATOM_END_MK
            ATOM_END_MK
        LIST(Segment_Tracks)
            ATOM_BEGIN
            LIST(Segment_Tracks_TrackEntry)
                ATOM_BEGIN
                ATOM(Segment_Tracks_TrackEntry_AttachmentLink)
                LIST(Segment_Tracks_TrackEntry_Audio)
                    ATOM_BEGIN
                    ATOM(Segment_Tracks_TrackEntry_Audio_BitDepth)
                    ATOM(Segment_Tracks_TrackEntry_Audio_Channels)
                    ATOM(Segment_Tracks_TrackEntry_Audio_OutputSamplingFrequency)
                    ATOM(Segment_Tracks_TrackEntry_Audio_SamplingFrequency)
                    ATOM_END_MK
                ATOM(Segment_Tracks_TrackEntry_CodecDecodeAll)
                ATOM(Segment_Tracks_TrackEntry_CodecID)
                LIS2(Segment_Tracks_TrackEntry_ContentEncodings, "ContentEncodings")
                    ATOM_BEGIN
                    LIS2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding, "ContentEncoding")
                    ATOM_BEGIN
                        ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Order, "Order")
                        ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Scope, "Scope")
                        ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Type, "Type")
                        LIS2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression, "Compression")
                            ATOM_BEGIN
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompAlgo, "Algo")
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompSettings, "Settings")
                            ATOM_END_MK
                        LIS2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption, "Encryption")
                            ATOM_BEGIN
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentEncAlgo, "Algo")
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentEncKeyID, "KeyID")
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSignature, "Signature")
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigKeyID, "SigKeyID")
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigAlgo, "SigAlgo")
                            ATO2(Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Encryption_ContentSigHashAlgo, "SigHashAlgo")
                            ATOM_END_MK
                        ATOM_END_MK
                    ATOM_END_MK
                ATOM(Segment_Tracks_TrackEntry_CodecName)
                ATOM(Segment_Tracks_TrackEntry_CodecPrivate)
                ATOM(Segment_Tracks_TrackEntry_DefaultDuration)
                ATOM(Segment_Tracks_TrackEntry_FlagDefault)
                ATOM(Segment_Tracks_TrackEntry_FlagEnabled)
                ATOM(Segment_Tracks_TrackEntry_FlagForced)
                ATOM(Segment_Tracks_TrackEntry_FlagLacing)
                ATOM(Segment_Tracks_TrackEntry_Language)
                ATOM(Segment_Tracks_TrackEntry_MaxBlockAdditionID)
                ATOM(Segment_Tracks_TrackEntry_MaxCache)
                ATOM(Segment_Tracks_TrackEntry_MinCache)
                ATOM(Segment_Tracks_TrackEntry_Name)
                ATOM(Segment_Tracks_TrackEntry_TrackNumber)
                ATOM(Segment_Tracks_TrackEntry_TrackTimecodeScale)
                ATOM(Segment_Tracks_TrackEntry_TrackType)
                ATOM(Segment_Tracks_TrackEntry_TrackUID)
                LIST(Segment_Tracks_TrackEntry_Video)
                    ATOM_BEGIN
                    ATOM(Segment_Tracks_TrackEntry_Video_AspectRatioType)
                    ATOM(Segment_Tracks_TrackEntry_Video_ColourSpace)
                    ATOM(Segment_Tracks_TrackEntry_Video_DisplayHeight)
                    ATOM(Segment_Tracks_TrackEntry_Video_DisplayUnit)
                    ATOM(Segment_Tracks_TrackEntry_Video_DisplayWidth)
                    ATOM(Segment_Tracks_TrackEntry_Video_FlagInterlaced)
                    ATOM(Segment_Tracks_TrackEntry_Video_FrameRate)
                    ATOM(Segment_Tracks_TrackEntry_Video_PixelCropBottom)
                    ATOM(Segment_Tracks_TrackEntry_Video_PixelCropLeft)
                    ATOM(Segment_Tracks_TrackEntry_Video_PixelCropRight)
                    ATOM(Segment_Tracks_TrackEntry_Video_PixelCropTop)
                    ATOM(Segment_Tracks_TrackEntry_Video_PixelHeight)
                    ATOM(Segment_Tracks_TrackEntry_Video_PixelWidth)
                    ATOM(Segment_Tracks_TrackEntry_Video_StereoMode)
                    ATOM_END_MK
                ATOM(Segment_Tracks_TrackEntry_TrackOverlay)
                LIST(Segment_Tracks_TrackEntry_TrackTranslate)
                    ATOM_BEGIN
                    ATOM(Segment_Tracks_TrackEntry_TrackTranslate_Codec)
                    ATOM(Segment_Tracks_TrackEntry_TrackTranslate_EditionUID)
                    ATOM(Segment_Tracks_TrackEntry_TrackTranslate_TrackID)
                    ATOM_END_MK
                ATOM_END_MK
            ATOM_END_MK
        ATOM_END_MK
    DATA_END
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mk::Zero()
{
    Element_Name("ZeroPadding");

    Skip_XX(Element_Size,                                       "Padding");
}

//---------------------------------------------------------------------------
void File_Mk::CRC32()
{
    Element_Name("CRC32");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Void()
{
    Element_Name("Void");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml()
{
    Element_Name("Ebml");
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_Version()
{
    Element_Name("Version");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_ReadVersion()
{
    Element_Name("ReadVersion");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_MaxIDLength()
{
    Element_Name("MaxIDLength");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_MaxSizeLength()
{
    Element_Name("MaxSizeLength");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_DocType()
{
    Element_Name("DocType");

    //Parsing
    Ztring Data;
    Get_Local(Element_Size, Data,                               "Data"); Element_Info1(Data);

    //Filling
    FILLING_BEGIN();
        if (Data==__T("matroska"))
        {
            Accept("Matroska");
            Fill(Stream_General, 0, General_Format, "Matroska");
        }
        else if (Data==__T("webm"))
        {
            Accept("Matroska");
            Fill(Stream_General, 0, General_Format, "WebM");
        }
        else
        {
            Reject("Matroska");
            return;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_DocTypeVersion()
{
    Element_Name("DocTypeVersion");

    //Parsing
    Format_Version=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Format_Version, __T("Version ")+Ztring::ToZtring(Format_Version));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Ebml_DocTypeReadVersion()
{
    Element_Name("DocTypeReadVersion");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        if (UInteger!=Format_Version)
            Fill(Stream_General, 0, General_Format_Version, __T("Version ")+Ztring::ToZtring(UInteger)); //Adding compatible version for info about legacy decoders
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment()
{
    Element_Name("Segment");

    if (!Status[IsAccepted])
    {
        Accept("Matroska");
        Fill(Stream_General, 0, General_Format, "Matroska"); //Default is Matroska
    }

    Segment_Offset_Begin=File_Offset+Buffer_Offset;
    Segment_Offset_End=File_Offset+Buffer_Offset+Element_TotalSize_Get();
}

void File_Mk::Segment_Attachements()
{
    Element_Name("Attachements");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile()
{
    Element_Name("AttachedFile");

    AttachedFiles.resize(AttachedFiles.size()+1);
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile_FileData()
{
    Element_Name("FileData");

    //Parsing
    Skip_XX(Element_TotalSize_Get(),                            "Data");

    FILLING_BEGIN();
        if (AttachedFiles[AttachedFiles.size()-1].empty())
            AttachedFiles[AttachedFiles.size()-1]=__T("Yes");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile_FileDescription()
{
    Element_Name("FileDescription");

    //Parsing
    Local_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile_FileName()
{
    Element_Name("FileName");

    //Parsing
    Ztring Data=UTF8_Get();
    
    FILLING_BEGIN();
        if (!Data.empty())
            AttachedFiles[AttachedFiles.size()-1]=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile_FileMimeType()
{
    Element_Name("FileMimeType");

    //Parsing
    Local_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile_FileReferral()
{
    Element_Name("FileReferral");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Attachements_AttachedFile_FileUID()
{
    Element_Name("FileUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters()
{
    Element_Name("Chapters");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry()
{
    Element_Name("EditionEntry");

    //Filling
    EditionEntries_Pos=EditionEntries.size();
    EditionEntries.resize(EditionEntries_Pos+1);
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom()
{
    Element_Name("ChapterAtom");

    //Filling
    ChapterAtoms_Pos=EditionEntries[EditionEntries_Pos].ChapterAtoms.size();
    EditionEntries[EditionEntries_Pos].ChapterAtoms.resize(ChapterAtoms_Pos+1);
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess()
{
    Element_Name("ChapProcess");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCodecID()
{
    Element_Name("ChapProcessCodecID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand()
{
    Element_Name("ChapProcessCommand");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessData()
{
    Element_Name("ChapProcessData");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessCommand_ChapProcessTime()
{
    Element_Name("ChapProcessTime");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapProcess_ChapProcessPrivate()
{
    Element_Name("ChapProcessPrivate");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay()
{
    Element_Name("ChapterDisplay");

    //Filling
    ChapterDisplays_Pos=EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays.size();
    EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays.resize(ChapterDisplays_Pos+1);
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapCountry()
{
    Element_Name("ChapCountry");

    //Parsing
    Local_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapLanguage()
{
    Element_Name("ChapLanguage");

    //Parsing
    Ztring Data=Local_Get();

    FILLING_BEGIN();
        EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays[ChapterDisplays_Pos].ChapLanguage=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterDisplay_ChapString()
{
    Element_Name("ChapString");

    //Parsing
    Ztring Data=UTF8_Get();

    FILLING_BEGIN();
        EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterDisplays[ChapterDisplays_Pos].ChapString=Data;
        //if (TimecodeScale!=0 && ChapterTimeStart!=(int64u)-1)
        //    Fill(StreamKind_Last, StreamPos_Last, Ztring::ToZtring(Chapter_Pos).To_Local().c_str(), Ztring().Duration_From_Milliseconds(ChapterTimeStart/TimecodeScale)+__T(" - ")+ChapterString, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagHidden()
{
    Element_Name("ChapterFlagHidden");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterFlagEnabled()
{
    Element_Name("ChapterFlagEnabled");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterPhysicalEquiv()
{
    Element_Name("ChapterPhysicalEquiv");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentEditionUID()
{
    Element_Name("ChapterSegmentEditionUID");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterSegmentUID()
{
    Element_Name("ChapterSegmentUID");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeEnd()
{
    Element_Name("ChapterTimeEnd");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterTimeStart()
{
    Element_Name("ChapterTimeStart");

    //Parsing
    int64u Data=UInteger_Get();

    FILLING_BEGIN();
        EditionEntries[EditionEntries_Pos].ChapterAtoms[ChapterAtoms_Pos].ChapterTimeStart=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack()
{
    Element_Name("ChapterTrack");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterTrack_ChapterTrackNumber()
{
    Element_Name("ChapterTrackNumber");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_ChapterAtom_ChapterUID()
{
    Element_Name("ChapterUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_EditionFlagDefault()
{
    Element_Name("EditionFlagDefault");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_EditionFlagHidden()
{
    Element_Name("EditionFlagHidden");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_EditionFlagOrdered()
{
    Element_Name("EditionFlagOrdered");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Chapters_EditionEntry_EditionUID()
{
    Element_Name("EditionUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster()
{
    Element_Name("Cluster");

    //For each stream
    std::map<int64u, stream>::iterator Temp=Stream.begin();
    if (!Cluster_AlreadyParsed)
    {
        Stream_Count=0;
        while (Temp!=Stream.end())
        {
            if (Temp->second.Parser)
                Temp->second.Searching_Payload=true;
            if (Temp->second.StreamKind==Stream_Video || Temp->second.StreamKind==Stream_Audio)
                Temp->second.Searching_TimeStamp_Start=true;
            if (Temp->second.StreamKind==Stream_Video)
                Temp->second.Searching_TimeStamps=true;
            if (Temp->second.Searching_Payload
             || Temp->second.Searching_TimeStamp_Start
             || Temp->second.Searching_TimeStamps)
                Stream_Count++;

            //Specific cases
            if (Retrieve(Temp->second.StreamKind, Temp->second.StreamPos, Audio_CodecID).find(__T("A_AAC/"))==0)
                ((File_Aac*)Stream[Temp->first].Parser)->Mode=File_Aac::Mode_raw_data_block; //In case AudioSpecificConfig is not present

            ++Temp;
        }

        //We must parse moov?
        if (Stream_Count==0)
        {
            //Jumping
            std::sort(Segment_Seeks.begin(), Segment_Seeks.end());
            for (size_t Pos=0; Pos<Segment_Seeks.size(); Pos++)
                if (Segment_Seeks[Pos]>File_Offset+Buffer_Offset+Element_Size)
                {
                    GoTo(Segment_Seeks[Pos]);
                    break;
                }
            if (File_GoTo==(int64u)-1)
                GoTo(Segment_Offset_End);
            return;
        }
    }
    Cluster_AlreadyParsed=true;
    Segment_Cluster_TimeCode_Value=0; //Default
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup()
{
    Element_Name("BlockGroup");

    Segment_Cluster_BlockGroup_BlockDuration_Value=(int64u)-1;
    Segment_Cluster_BlockGroup_BlockDuration_TrackNumber=(int64u)-1;
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_Block()
{
    Element_Name("Block");

    //Parsing
    int64u TrackNumber;
    Get_EB (TrackNumber,                                        "TrackNumber");

    //Finished?
    Stream[TrackNumber].PacketCount++;
    if (Stream[TrackNumber].Searching_Payload || Stream[TrackNumber].Searching_TimeStamps || Stream[TrackNumber].Searching_TimeStamp_Start)
    {
        //Parsing
        int16u TimeCode;
        Get_B2 (TimeCode,                                       "TimeCode");

        FILLING_BEGIN();
            if (Stream[TrackNumber].Searching_TimeStamp_Start)
            {
                FILLING_BEGIN();
                    Stream[TrackNumber].TimeCode_Start=Segment_Cluster_TimeCode_Value+TimeCode;
                    Stream[TrackNumber].Searching_TimeStamp_Start=false;
                FILLING_END();
            }
            if (Stream[TrackNumber].Searching_TimeStamps)
            {
                Stream[TrackNumber].TimeCodes.push_back(Segment_Cluster_TimeCode_Value+TimeCode);
                if (Stream[TrackNumber].TimeCodes.size()>40)
                    Stream[TrackNumber].Searching_TimeStamps=false;
            }

            if (Segment_Cluster_BlockGroup_BlockDuration_Value!=(int64u)-1)
            {
                Stream[TrackNumber].Segment_Cluster_BlockGroup_BlockDuration_Counts[Segment_Cluster_BlockGroup_BlockDuration_Value]++;
                Segment_Cluster_BlockGroup_BlockDuration_Value=(int64u)-1;
            }
        FILLING_END();

        if (Stream[TrackNumber].Searching_Payload)
        {
            std::vector<int64u> Laces;
            int32u Lacing;
            Element_Begin1("Flags");
                BS_Begin();
                Skip_BS(1,                                      "KeyFrame");
                Skip_BS(3,                                      "Reserved");
                Skip_BS(1,                                      "Invisible");
                Get_BS (2, Lacing,                              "Lacing");
                Skip_BS(1,                                      "Discardable");
                BS_End();
            Element_End0();
            if (Lacing>0)
            {
                Element_Begin1("Lacing");
                    int8u FrameCountMinus1;
                    Get_B1(FrameCountMinus1,                    "Frame count minus 1");
                    switch (Lacing)
                    {
                        case 1 : //Xiph lacing
                                {
                                    int64u Element_Offset_Virtual=0;
                                    for (int8u Pos=0; Pos<FrameCountMinus1; Pos++)
                                    {
                                        int32u Size=0;
                                        int8u Size8;
                                        do
                                        {
                                            Get_B1 (Size8,      "Size");
                                            Size+=Size8;
                                        }
                                        while (Size8==0xFF);
                                        Param_Info1(Size);
                                        Element_Offset_Virtual+=Size;
                                        Laces.push_back(Size);
                                    }
                                    Laces.push_back(Element_Size-Element_Offset-Element_Offset_Virtual); //last lace
                                }
                                break;
                        case 2 : //Fixed-size lacing - No more data
                                {
                                    int64u Size=(Element_Size-Element_Offset)/(FrameCountMinus1+1);
                                    Laces.resize(FrameCountMinus1+1, Size);
                                }
                                break;
                        case 3 : //EBML lacing
                                {
                                    int64u Element_Offset_Virtual=0, Size;
                                    Get_EB (Size,                "Size");
                                    Laces.push_back(Size);
                                    Element_Offset_Virtual+=Size;
                                    for (int8u Pos=1; Pos<FrameCountMinus1; Pos++)
                                    {
                                        int64s Diff;
                                        Get_ES (Diff,           "Difference");
                                        Size+=Diff; Param_Info1(Size);
                                        Element_Offset_Virtual+=Size;
                                        Laces.push_back(Size);
                                    }
                                    Laces.push_back(Element_Size-Element_Offset-Element_Offset_Virtual); Param_Info1(Size); //last lace
                                }
                                break;
                        default : ; //Should never be here
                    }
                Element_End0();
            }
            else
                Laces.push_back(Element_Size-Element_Offset);

            FILLING_BEGIN();
                //Parsing
                for (size_t Pos=0; Pos<Laces.size(); Pos++)
                {
                    //Content compression
                    if (Stream[TrackNumber].ContentCompAlgo!=(int32u)-1 && Stream[TrackNumber].ContentCompAlgo!=3)
                        Stream[TrackNumber].Searching_Payload=false; //Unsupported

                    //Integrity test
                    if (Element_Offset+Laces[Pos]>Element_Size)
                        Stream[TrackNumber].Searching_Payload=false; //There is a problem

                    if (Stream[TrackNumber].Searching_Payload)
                    {
                        Element_Code=TrackNumber;

                        //Content compression
                        if (Stream[TrackNumber].ContentCompAlgo==3) //Header Stripping
                        {
                            Element_Offset-=(size_t)Stream[TrackNumber].ContentCompSettings_Buffer_Size; //This is an extra array, not in the stream
                            Open_Buffer_Continue(Stream[TrackNumber].Parser, Stream[TrackNumber].ContentCompSettings_Buffer, (size_t)Stream[TrackNumber].ContentCompSettings_Buffer_Size);
                            Element_Offset+=(size_t)Stream[TrackNumber].ContentCompSettings_Buffer_Size;
                            Demux(Stream[TrackNumber].ContentCompSettings_Buffer, (size_t)Stream[TrackNumber].ContentCompSettings_Buffer_Size, ContentType_MainStream);
                        }

                        //Parsing
                        #if MEDIAINFO_DEMUX
                            int8u Demux_Level_old=Demux_Level;
                            if (Stream[TrackNumber].Parser && Stream[TrackNumber].Parser->Demux_Level==2)
                                Demux_Level=4;
                            Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset), ContentType_MainStream);
                            Demux_Level=Demux_Level_old;
                        #endif //MEDIAINFO_DEMUX
                        Open_Buffer_Continue(Stream[TrackNumber].Parser, (size_t)Laces[Pos]);
                        if (Stream[TrackNumber].Parser->Status[IsFilled]
                         || Stream[TrackNumber].Parser->Status[IsFinished]
                         || Stream[TrackNumber].PacketCount>=300)
                            Stream[TrackNumber].Searching_Payload=false;
                    }
                    else
                        Skip_XX(Laces[Pos],                         "Data");
                }

                //Positionning
                Element_Offset=Element_Size;
            FILLING_END();
        }
        else
        {
            Skip_XX(Element_Size-Element_Offset,                    "Data");
        }

        if (!Stream[TrackNumber].Searching_Payload && !Stream[TrackNumber].Searching_TimeStamps && !Stream[TrackNumber].Searching_TimeStamp_Start)
            Stream_Count--;
    }
    else
    {
        Skip_XX(Element_Size-Element_Offset,                        "Data");
    }

    //Filling
    Frame_Count++;
    if (!Status[IsFilled] && ((Frame_Count>6 && (Stream_Count==0 ||Config->ParseSpeed==0.0)) || Frame_Count>512*Stream.size()))
    {
        Fill();
        if (MediaInfoLib::Config.ParseSpeed_Get()<1)
        {
            //Jumping
            std::sort(Segment_Seeks.begin(), Segment_Seeks.end());
            for (size_t Pos=0; Pos<Segment_Seeks.size(); Pos++)
                if (Segment_Seeks[Pos]>File_Offset+Buffer_Offset+Element_Size)
                {
                    GoTo(Segment_Seeks[Pos]);
                    break;
                }
            if (File_GoTo==(int64u)-1)
                GoTo(Segment_Offset_End);
        }
    }

    Element_Show(); //For debug
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_BlockAdditions()
{
    Element_Name("BlockAdditions");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_BlockAdditions_BlockMore()
{
    Element_Name("BlockMore");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAddID()
{
    Element_Name("BlockAddID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_BlockAdditions_BlockMore_BlockAdditional()
{
    Element_Name("BlockAdditional");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_BlockDuration()
{
    Element_Name("BlockDuration");

    //Parsing
    int64u Segment_Cluster_TimeCode_Value=UInteger_Get();

    FILLING_BEGIN();
        if (Segment_Cluster_BlockGroup_BlockDuration_TrackNumber!=(int64u)-1)
        {
            Stream[Segment_Cluster_BlockGroup_BlockDuration_TrackNumber].Segment_Cluster_BlockGroup_BlockDuration_Counts[Segment_Cluster_TimeCode_Value]++;
            Segment_Cluster_BlockGroup_BlockDuration_TrackNumber=(int64u)-1;
        }
        else
            Segment_Cluster_BlockGroup_BlockDuration_Value=Segment_Cluster_TimeCode_Value;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_ReferenceBlock()
{
    Element_Name("ReferenceBlock");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_ReferencePriority()
{
    Element_Name("ReferencePriority");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_Slices()
{
    Element_Name("Slices");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_Slices_TimeSlice()
{
    Element_Name("TimeSlice");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_Slices_TimeSlice_Duration()
{
    Element_Name("Duration");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_BlockGroup_Slices_TimeSlice_LaceNumber()
{
    Element_Name("LaceNumber");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_Position()
{
    Element_Name("Position");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_PrevSize()
{
    Element_Name("PrevSize");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_SilentTracks()
{
    Element_Name("SilentTracks");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_SilentTracks_SilentTrackNumber()
{
    Element_Name("SilentTrackNumber");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_SimpleBlock()
{
    Segment_Cluster_BlockGroup_BlockDuration_Value=(int64u)-1;
    Segment_Cluster_BlockGroup_BlockDuration_TrackNumber=(int64u)-1;

    Segment_Cluster_BlockGroup_Block();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cluster_Timecode()
{
    Element_Name("Timecode");

    //Parsing
    Segment_Cluster_TimeCode_Value=UInteger_Get();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues()
{
    Element_Name("Cues");

    //Skipping Cues, we don't need of them
    Skip_XX(Element_TotalSize_Get(),                            "Cues data, skipping");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues_CuePoint()
{
    Element_Name("CuePoint");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues_CuePoint_CueTime()
{
    Element_Name("CueTime");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues_CuePoint_CueTrackPositions()
{
    Element_Name("CueTrackPositions");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues_CuePoint_CueTrackPositions_CueTrack()
{
    Element_Name("CueTrack");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues_CuePoint_CueTrackPositions_CueClusterPosition()
{
    Element_Name("CueClusterPosition");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Cues_CuePoint_CueTrackPositions_CueBlockNumber()
{
    Element_Name("CueBlockNumber");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info()
{
    Element_Name("Info");

    if (Cluster_AlreadyParsed)
        Skip_XX(Element_TotalSize_Get(),                        "Alreadys parsed, skipping");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_ChapterTranslate()
{
    Element_Name("ChapterTranslate");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_ChapterTranslate_ChapterTranslateCodec()
{
    Element_Name("ChapterTranslateCodec");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_ChapterTranslate_ChapterTranslateEditionUID()
{
    Element_Name("ChapterTranslateEditionUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_ChapterTranslate_ChapterTranslateID()
{
    Element_Name("ChapterTranslateID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_DateUTC()
{
    Element_Name("DateUTC");

    //Parsing
    int64u Data;
    Get_B8(Data,                                                "Data"); Element_Info1(Data/1000000000+978307200); //From Beginning of the millenium, in nanoseconds

    FILLING_BEGIN();
        Fill(Stream_General, 0, "Encoded_Date", Ztring().Date_From_Seconds_1970((int32u)(Data/1000000000+978307200))); //978307200s between beginning of the millenium and 1970
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_Duration()
{
    Element_Name("Duration");

    //Parsing
    float64 Float=Float_Get();

    FILLING_BEGIN();
        Duration=Float;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_MuxingApp()
{
    Element_Name("MuxingApp");

    //Parsing
    Ztring Data=UTF8_Get();

    FILLING_BEGIN();
        Fill(Stream_General, 0, "Encoded_Library", Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_NextFilename()
{
    Element_Name("NextFilename");

    //Parsing
    UTF8_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_NextUID()
{
    Element_Name("NextUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_PrevFilename()
{
    Element_Name("PrevFilename");

    //Parsing
    UTF8_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_PrevUID()
{
    Element_Name("PrevUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_SegmentFamily()
{
    Element_Name("SegmentFamily");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_SegmentFilename()
{
    Element_Name("SegmentFilename");

    //Parsing
    UTF8_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_SegmentUID()
{
    Element_Name("SegmentUID");

    //Parsing
    int128u Data;
    Data=UInteger16_Get();

    FILLING_BEGIN();
        Fill(Stream_General, 0, General_UniqueID, Ztring().From_Local(Data.toString(10)));
        Fill(Stream_General, 0, General_UniqueID_String, Ztring().From_Local(Data.toString(10))+__T(" (0x")+Ztring().From_Local(Data.toString(16))+__T(')'));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_TimecodeScale()
{
    Element_Name("TimecodeScale");

    //Parsing
    int64u UInteger=UInteger_Get();

    FILLING_BEGIN();
        TimecodeScale=UInteger;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_Title()
{
    Element_Name("Title");

    //Parsing
    Ztring Data=UTF8_Get();

    FILLING_BEGIN();
        Fill(Stream_General, 0, "Title", Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Info_WritingApp()
{
    Element_Name("WritingApp");

    //Parsing
    Ztring Data=UTF8_Get();

    FILLING_BEGIN();
        Fill(Stream_General, 0, "Encoded_Application", Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_SeekHead()
{
    Element_Name("SeekHead");

    Segment_Seeks.clear();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_SeekHead_Seek()
{
    Element_Name("Seek");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_SeekHead_Seek_SeekID()
{
    Element_Name("SeekID");

    //Parsing
    int64u Data;
    Get_EB (Data,                                               "Data");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_SeekHead_Seek_SeekPosition()
{
    Element_Name("SeekPosition");

    //Parsing
    int64u Data=UInteger_Get();

    Segment_Seeks.push_back(Segment_Offset_Begin+Data);
    Element_Info1(Ztring::ToZtring(Segment_Offset_Begin+Data, 16));
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags()
{
    Element_Name("Tags");

    Segment_Tag_SimpleTag_TagNames.clear();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag()
{
    Element_Name("Tag");

    Segment_Tag_TrackUID=(int64u)-1;
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_SimpleTag()
{
    Element_Name("SimpleTag");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_SimpleTag_TagBinary()
{
    Element_Name("TagBinary");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_SimpleTag_TagDefault()
{
    Element_Name("TagDefault");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_SimpleTag_TagLanguage()
{
    Element_Name("TagLanguage");

    //Parsing
    Ztring Data;
    Get_Local(Element_Size, Data,                              "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        //Fill(StreamKind_Last, StreamPos_Last, "Language", Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_SimpleTag_TagName()
{
    Element_Name("TagName");

    //Parsing
    Ztring TagName=UTF8_Get();

    Segment_Tag_SimpleTag_TagNames.resize(Element_Level-5); //5 is the first level of a tag
    Segment_Tag_SimpleTag_TagNames.push_back(TagName);
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_SimpleTag_TagString()
{
    Element_Name("TagString");

    //Parsing
    Ztring TagString;
    TagString=UTF8_Get();

    if (Segment_Tag_SimpleTag_TagNames.empty())
        return;
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("BITSPS")) return; //Useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("BPS")) return; //Useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("COMPATIBLE_BRANDS")) return; //QuickTime techinical info, useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("CREATION_TIME")) {Segment_Tag_SimpleTag_TagNames[0]=__T("Encoded_Date"); TagString.insert(0, __T("UTC "));}
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("DATE_DIGITIZED")) {Segment_Tag_SimpleTag_TagNames[0]=__T("Mastered_Date"); TagString.insert(0, __T("UTC "));}
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("ENCODED_BY")) Segment_Tag_SimpleTag_TagNames[0]=__T("EncodedBy");
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("ENCODER")) Segment_Tag_SimpleTag_TagNames[0]=__T("Encoded_Library");
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("FPS")) return; //Useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("LANGUAGE")) Segment_Tag_SimpleTag_TagNames[0]=__T("Language");
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("MAJOR_BRAND")) return; //QuickTime techinical info, useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("MINOR_VERSION")) return; //QuickTime techinical info, useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("ORIGINAL_MEDIA_TYPE")) Segment_Tag_SimpleTag_TagNames[0]=__T("OriginalSourceForm");
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("STEREO_MODE")) return; //Useless
    if (Segment_Tag_SimpleTag_TagNames[0]==__T("TERMS_OF_USE")) Segment_Tag_SimpleTag_TagNames[0]=__T("TermsOfUse");
    for (size_t Pos=1; Pos<Segment_Tag_SimpleTag_TagNames.size(); Pos++)
    {
        if (Segment_Tag_SimpleTag_TagNames[Pos]==__T("BARCODE")) Segment_Tag_SimpleTag_TagNames[Pos]=__T("BarCode");
        if (Segment_Tag_SimpleTag_TagNames[Pos]==__T("COMMENT")) Segment_Tag_SimpleTag_TagNames[Pos]=__T("Comment");
        if (Segment_Tag_SimpleTag_TagNames[Pos]==__T("URL")) Segment_Tag_SimpleTag_TagNames[Pos]=__T("Url");
    }

    Ztring TagName;
    for (size_t Pos=0; Pos<Segment_Tag_SimpleTag_TagNames.size(); Pos++)
    {
        TagName+=Segment_Tag_SimpleTag_TagNames[Pos];
        if (Pos+1<Segment_Tag_SimpleTag_TagNames.size())
            TagName+=__T('/');
    }

    StreamKind_Last=Stream_General;
    StreamPos_Last=0;
    if (Segment_Tag_TrackUID!=(int64u)-1 && Segment_Tag_TrackUID!=0)//0: Specs say this is for all tracks, but I prefer to wait for a sample in order to see how it is used in the reality
    {
        Ztring ID=Ztring::ToZtring(Segment_Tag_TrackUID);
        for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
            for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
                if (Retrieve((stream_t)StreamKind, StreamPos, General_ID)==ID)
                {
                    StreamKind_Last=(stream_t)StreamKind;
                    StreamPos_Last=StreamPos;
                }
    }

    Fill(StreamKind_Last, StreamPos_Last, TagName.To_Local().c_str(), TagString, true);
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets()
{
    Element_Name("Targets");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets_AttachmentUID()
{
    Element_Name("AttachmentUID");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets_ChapterUID()
{
    Element_Name("ChapterUID");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets_EditionUID()
{
    Element_Name("EditionUID");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets_TargetType()
{
    Element_Name("TargetType");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets_TargetTypeValue()
{
    Element_Name("TargetTypeValue");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tags_Tag_Targets_TrackUID()
{
    Element_Name("TrackUID");

    //Parsing
    Segment_Tag_TrackUID=UInteger_Get();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks()
{
    Element_Name("Tracks");

    if (Cluster_AlreadyParsed)
        Skip_XX(Element_TotalSize_Get(),                        "Alreadys parsed, skipping");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry()
{
    Element_Name("TrackEntry");

    //Clearing
    CodecID.clear();
    InfoCodecID_Format_Type=InfoCodecID_Format_Matroska;
    TrackType=(int64u)-1;
    TrackNumber=(int64u)-1;
    TrackVideoDisplayWidth=0;
    TrackVideoDisplayHeight=0;
    AvgBytesPerSec=0;

    //Preparing
    Stream_Prepare(Stream_Max);

    //Default values
    Fill_Flush();
    Fill(StreamKind_Last, StreamPos_Last, "Language", "eng");
    Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, Stream.size());
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_AttachmentLink()
{
    Element_Name("AttachmentLink");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Audio()
{
    Element_Name("Audio");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Audio_BitDepth()
{
    Element_Name("BitDepth");

    //Parsing
    int64u UInteger=UInteger_Get();

    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, "BitDepth", UInteger, 10, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Audio_Channels()
{
    Element_Name("Channels");

    //Parsing
    int64u UInteger=UInteger_Get();

    FILLING_BEGIN();
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, UInteger, 10, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Audio_OutputSamplingFrequency()
{
    Element_Name("OutputSamplingFrequency");

    //Parsing
    float64 Float=Float_Get();

    FILLING_BEGIN();
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, Float, 0, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Audio_SamplingFrequency()
{
    Element_Name("SamplingFrequency");

    //Parsing
    float64 Float=Float_Get();

    FILLING_BEGIN();
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, Float, 0, true);
        if (Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID).find(__T("A_AAC/"))==0)
            ((File_Aac*)Stream[TrackNumber].Parser)->AudioSpecificConfig_OutOfBand((int32u)float64_int64s(Float));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecDecodeAll()
{
    Element_Name("CodecDecodeAll");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecID()
{
    Element_Name("CodecID");

    //Parsing
    Ztring Data;
    Get_Local(Element_Size, Data,                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        CodecID=Data;
        CodecID_Manage();
        CodecPrivate_Manage();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression()
{
    FILLING_BEGIN();
        Stream[TrackNumber].ContentCompAlgo=0; //0 is default
        Fill(StreamKind_Last, StreamPos_Last, "MuxingMode", Mk_ContentCompAlgo(0), Unlimited, true, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompAlgo()
{
    //Parsing
    int64u Algo=UInteger_Get(); Param_Info1(Mk_ContentCompAlgo(Algo));

    FILLING_BEGIN();
        Stream[TrackNumber].ContentCompAlgo=Algo;
        Fill(StreamKind_Last, StreamPos_Last, "MuxingMode", Mk_ContentCompAlgo(Algo), Unlimited, true, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_ContentEncodings_ContentEncoding_Compression_ContentCompSettings()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        Stream[TrackNumber].ContentCompSettings_Buffer=new int8u[(size_t)Element_Size];
        std::memcpy(Stream[TrackNumber].ContentCompSettings_Buffer, Buffer+Buffer_Offset, (size_t)Element_Size);
        Stream[TrackNumber].ContentCompSettings_Buffer_Size=(size_t)Element_Size;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecName()
{
    Element_Name("CodecName");

    //Parsing
    UTF8_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecPrivate()
{
    Element_Name("CodecPrivate");

    //Creating the parser
    if (Stream.find(TrackNumber)==Stream.end() || Stream[TrackNumber].Parser==NULL)
    {
        if (Stream.find(TrackNumber)==Stream.end() || Retrieve(Stream[TrackNumber].StreamKind, Stream[TrackNumber].StreamPos, "CodecID").empty())
        {
            //Codec not already known, saving CodecPrivate
            if (CodecPrivate)
                delete[] CodecPrivate; //CodecPrivate=NULL.
            CodecPrivate_Size=(size_t)Element_Size;
            CodecPrivate=new int8u[(size_t)Element_Size];
            std::memcpy(CodecPrivate, Buffer+Buffer_Offset, (size_t)Element_Size);
            return;
        }

        if (Stream[TrackNumber].StreamKind==Stream_Audio && Retrieve(Stream_Audio, Stream[TrackNumber].StreamPos, Audio_CodecID)==__T("A_MS/ACM"))
            Segment_Tracks_TrackEntry_CodecPrivate_auds();
        else if (Stream[TrackNumber].StreamKind==Stream_Video && Retrieve(Stream_Video, Stream[TrackNumber].StreamPos, Video_CodecID)==__T("V_MS/VFW/FOURCC"))
            Segment_Tracks_TrackEntry_CodecPrivate_vids();
        else if (Element_Size>0)
            Skip_XX(Element_Size,                 "Unknown");
        return;
    }

    #if MEDIAINFO_DEMUX
        switch (Config->Demux_InitData_Get())
        {
            case 0 :    //In demux event
                        {
                        Demux_Level=2; //Container
                        int64u Element_Code_Old=Element_Code;
                        Element_Code=TrackNumber;
                        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_Header);
                        Element_Code=Element_Code_Old;
                        }
                        break;
            case 1 :    //In field
                        {
                        std::string Data_Raw((const char*)(Buffer+Buffer_Offset), (size_t)Element_Size);
                        std::string Data_Base64(Base64::encode(Data_Raw));
                        Fill(StreamKind_Last, StreamPos_Last, "Demux_InitBytes", Data_Base64);
                        (*Stream_More)[StreamKind_Last][StreamPos_Last](Ztring().From_Local("Demux_InitBytes"), Info_Options)=__T("N NT");
                        }
                        break;
            default :   ;
        }
    #endif // MEDIAINFO_DEMUX

    //Parsing
    Open_Buffer_Continue(Stream[TrackNumber].Parser);

    //Filling
    if (Stream[TrackNumber].Parser->Status[IsFinished]) //Can be finnished here...
    {
        Stream[TrackNumber].Searching_Payload=false;
        Stream_Count--;
    }

    //In case of problem
    Element_Show();
}

//--------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecPrivate_auds()
{
    Element_Info1("Copy of auds");

    //Parsing
    int32u SamplesPerSec;
    int16u FormatTag, Channels, BitsPerSample;
    Get_L2 (FormatTag,                                          "FormatTag");
    Get_L2 (Channels,                                           "Channels");
    Get_L4 (SamplesPerSec,                                      "SamplesPerSec");
    Get_L4 (AvgBytesPerSec,                                     "AvgBytesPerSec");
    Skip_L2(                                                    "BlockAlign");
    Get_L2 (BitsPerSample,                                      "BitsPerSample");

    //Filling
    FILLING_BEGIN();
        InfoCodecID_Format_Type=InfoCodecID_Format_Riff;
        CodecID.From_Number(FormatTag, 16);
        CodecID_Fill(CodecID, Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, CodecID, true); //May be replaced by codec parser
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, CodecID);
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels!=5?Channels:6, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SamplesPerSec, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, AvgBytesPerSec*8, 10, true);
        if (BitsPerSample)
            Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, BitsPerSample);

        CodecID_Manage();
        if (TrackNumber!=(int64u)-1)
            Stream[TrackNumber].AvgBytesPerSec=AvgBytesPerSec;
    FILLING_END();

    //Options
    if (Element_Offset+2>Element_Size)
        return; //No options

    //Parsing
    int16u Option_Size;
    Get_L2 (Option_Size,                                        "cbSize");

    //Filling
    if (Option_Size>0)
    {
             if (FormatTag==0xFFFE) //Extensible Wave
            Segment_Tracks_TrackEntry_CodecPrivate_auds_ExtensibleWave();
        else Skip_XX(Option_Size,                               "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecPrivate_auds_ExtensibleWave()
{
    //Parsing
    int128u SubFormat;
    int32u ChannelMask;
    Skip_L2(                                                    "ValidBitsPerSample / SamplesPerBlock");
    Get_L4 (ChannelMask,                                        "ChannelMask");
    Get_GUID(SubFormat,                                         "SubFormat");

    FILLING_BEGIN();
        if ((SubFormat.hi&0xFFFFFFFFFFFF0000LL)==0x0010000000000000LL && SubFormat.lo==0x800000AA00389B71LL)
        {
            CodecID_Fill(Ztring().From_Number((int16u)SubFormat.hi, 16), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
            Fill(Stream_Audio, StreamPos_Last, Audio_CodecID, Ztring().From_GUID(SubFormat), true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, MediaInfoLib::Config.Codec_Get(Ztring().From_Number((int16u)SubFormat.hi, 16)), true);

            //Creating the parser
            #if defined(MEDIAINFO_PCM_YES)
            if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Riff, Ztring().From_Number((int16u)SubFormat.hi, 16))==__T("PCM"))
            {
                //Creating the parser
                File_Pcm MI;
                MI.Frame_Count_Valid=0;
                MI.Codec=Ztring().From_Number((int16u)SubFormat.hi, 16);

                //Parsing
                Open_Buffer_Init(&MI);
                Open_Buffer_Continue(&MI, 0);

                //Filling
                Finish(&MI);
                Merge(MI, StreamKind_Last, 0, StreamPos_Last);
            }
            #endif
        }
        else
        {
            CodecID_Fill(Ztring().From_GUID(SubFormat), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
        }
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, ExtensibleWave_ChannelMask(ChannelMask));
        Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions_String2, ExtensibleWave_ChannelMask2(ChannelMask));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_CodecPrivate_vids()
{
    Element_Info1("Copy of vids");

    //Parsing
    int32u Width, Height, Compression;
    int16u Resolution;
    Skip_L4(                                                    "Size");
    Get_L4 (Width,                                              "Width");
    Get_L4 (Height,                                             "Height");
    Skip_L2(                                                    "Planes");
    Get_L2 (Resolution,                                         "BitCount");
    Get_C4 (Compression,                                        "Compression");
    Skip_L4(                                                    "SizeImage");
    Skip_L4(                                                    "XPelsPerMeter");
    Skip_L4(                                                    "YPelsPerMeter");
    Skip_L4(                                                    "ClrUsed");
    Skip_L4(                                                    "ClrImportant");

    FILLING_BEGIN();
        Ztring Codec;
        if (((Compression&0x000000FF)>=0x00000020 && (Compression&0x000000FF)<=0x0000007E
          && (Compression&0x0000FF00)>=0x00002000 && (Compression&0x0000FF00)<=0x00007E00
          && (Compression&0x00FF0000)>=0x00200000 && (Compression&0x00FF0000)<=0x007E0000
          && (Compression&0xFF000000)>=0x20000000 && (Compression&0xFF000000)<=0x7E000000)
         ||   Compression==0x00000000
           ) //Sometimes this value is wrong, we have to test this
        {
            //Filling
            InfoCodecID_Format_Type=InfoCodecID_Format_Riff;
            CodecID.From_CC4(Compression);
            if (Compression==0x00000000)
            {
                Fill(Stream_Video, StreamPos_Last, Video_Format, "RGB", Unlimited, true, true);
                Fill(Stream_Video, StreamPos_Last, Video_Codec, "RGB", Unlimited, true, true); //Raw RGB, not handled by automatic codec mapping
            }
            else
            {
                CodecID_Fill(CodecID, Stream_Video, StreamPos_Last, InfoCodecID_Format_Riff);
                Fill(Stream_Video, StreamPos_Last, Video_Codec, CodecID, true); //FormatTag, may be replaced by codec parser
                Fill(Stream_Video, StreamPos_Last, Video_Codec_CC, CodecID); //FormatTag
            }
            Fill(Stream_Video, StreamPos_Last, Video_Width, Width, 10, true);
            Fill(Stream_Video, StreamPos_Last, Video_Height, Height, 10, true);
            if (Resolution==32 && Compression==0x74736363) //tscc
                Fill(StreamKind_Last, StreamPos_Last, "BitDepth", 8);
            else if (Compression==0x44495633) //DIV3
                Fill(StreamKind_Last, StreamPos_Last, "BitDepth", 8);
            else if (Compression==0x44585342) //DXSB
                Fill(StreamKind_Last, StreamPos_Last, "BitDepth", Resolution);
            else if (Resolution>16 && MediaInfoLib::Config.CodecID_Get(StreamKind_Last, InfoCodecID_Format_Riff, Ztring().From_CC4(Compression), InfoCodecID_ColorSpace).find(__T("RGBA"))!=std::string::npos) //RGB codecs
                Fill(StreamKind_Last, StreamPos_Last, "BitDepth", Resolution/4);
            else if (Compression==0x00000000 //RGB
                  || MediaInfoLib::Config.CodecID_Get(StreamKind_Last, InfoCodecID_Format_Riff, Ztring().From_CC4(Compression), InfoCodecID_ColorSpace).find(__T("RGB"))!=std::string::npos) //RGB codecs
            {
                if (Resolution==32)
                {
                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "RGBA", Unlimited, true, true);
                    Fill(StreamKind_Last, StreamPos_Last, "BitDepth", Resolution/4); //With Alpha
                }
                else
                    Fill(StreamKind_Last, StreamPos_Last, "BitDepth", Resolution<=16?8:(Resolution/3)); //indexed or normal
            }
            else if (Compression==0x56503632 //VP62
                  || MediaInfoLib::Config.CodecID_Get(StreamKind_Last, InfoCodecID_Format_Riff, Ztring().From_CC4(Compression), InfoCodecID_Format)==__T("H.263") //H.263
                  || MediaInfoLib::Config.CodecID_Get(StreamKind_Last, InfoCodecID_Format_Riff, Ztring().From_CC4(Compression), InfoCodecID_Format)==__T("VC-1")) //VC-1
                Fill(StreamKind_Last, StreamPos_Last, "BitDepth", Resolution/3);
        }

        //Creating the parser
        CodecID_Manage();

    FILLING_END();

    if (Data_Remain())
    {
        Element_Begin1("Private data");
        if (Stream[TrackNumber].Parser)
            Open_Buffer_Continue(Stream[TrackNumber].Parser);
        else
            Skip_XX(Data_Remain(),                                  "Unknown");
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_DefaultDuration()
{
    Element_Name("DefaultDuration");

    //Parsing
    int64u UInteger=UInteger_Get();

    FILLING_BEGIN();
        Stream[TrackNumber].TrackDefaultDuration=UInteger;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_FlagDefault()
{
    Element_Name("FlagDefault");

    //Parsing
    int64u UInteger=UInteger_Get();

    FILLING_BEGIN();
        Stream[TrackNumber].Default=UInteger?true:false;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_FlagEnabled()
{
    Element_Name("FlagEnabled");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_FlagForced()
{
    Element_Name("FlagForced");

    //Parsing
    int64u UInteger=UInteger_Get();

    FILLING_BEGIN();
        Stream[TrackNumber].Forced=UInteger?true:false;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_FlagLacing()
{
    Element_Name("FlagLacing");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Language()
{
    Element_Name("Language");

    //Parsing
    Ztring Data;
    Get_Local(Element_Size, Data,                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, "Language", Data, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_MaxBlockAdditionID()
{
    Element_Name("MaxBlockAdditionID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_MaxCache()
{
    Element_Name("MaxCache");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_MinCache()
{
    Element_Name("MinCache");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Name()
{
    Element_Name("Name");

    //Parsing
    Ztring Data;
    Get_UTF8(Element_Size, Data,                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, "Title", Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackNumber()
{
    Element_Name("TrackNumber");

    //Parsing
    TrackNumber=UInteger_Get();

    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, General_ID, TrackNumber);
        if (StreamKind_Last!=Stream_Max)
        {
            Stream[TrackNumber].StreamKind=StreamKind_Last;
            Stream[TrackNumber].StreamPos=StreamPos_Last;
        }
        if (TrackVideoDisplayWidth && TrackVideoDisplayHeight)
            Stream[TrackNumber].DisplayAspectRatio=((float)TrackVideoDisplayWidth)/(float)TrackVideoDisplayHeight;
        if (AvgBytesPerSec)
            Stream[TrackNumber].AvgBytesPerSec=AvgBytesPerSec;

        CodecID_Manage();
        CodecPrivate_Manage();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackTimecodeScale()
{
    Element_Name("TrackTimecodeScale");

    Float_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackType()
{
    Element_Name("TrackType");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        TrackType=UInteger;
        switch(UInteger)
        {
            case 0x01 :
                        Stream_Prepare(Stream_Video);
                        break;
            case 0x02 :
                        Stream_Prepare(Stream_Audio);
                        break;
            case 0x11 :
                        Stream_Prepare(Stream_Text );
                        break;
            default   : ;
        }

        if (TrackNumber!=(int64u)-1 && StreamKind_Last!=Stream_Max)
        {
            Stream[TrackNumber].StreamKind=StreamKind_Last;
            Stream[TrackNumber].StreamPos=StreamPos_Last;
        }

        CodecID_Manage();
        CodecPrivate_Manage();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackUID()
{
    Element_Name("TrackUID");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, General_UniqueID, UInteger);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video()
{
    Element_Name("Video");

    //Preparing
    TrackVideoDisplayWidth=0;
    TrackVideoDisplayHeight=0;
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_AspectRatioType()
{
    Element_Name("AspectRatioType");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_ColourSpace()
{
    Element_Name("ColourSpace");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_DisplayHeight()
{
    Element_Name("DisplayHeight");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        TrackVideoDisplayHeight=UInteger;
        if (TrackNumber!=(int64u)-1 && TrackVideoDisplayWidth && TrackVideoDisplayHeight)
            Stream[TrackNumber].DisplayAspectRatio=((float)TrackVideoDisplayWidth)/(float)TrackVideoDisplayHeight;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_DisplayUnit()
{
    Element_Name("DisplayUnit");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_DisplayWidth()
{
    Element_Name("DisplayWidth");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        TrackVideoDisplayWidth=UInteger;
        if (TrackNumber!=(int64u)-1 && TrackVideoDisplayWidth && TrackVideoDisplayHeight)
            Stream[TrackNumber].DisplayAspectRatio=((float)TrackVideoDisplayWidth)/(float)TrackVideoDisplayHeight;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_FlagInterlaced()
{
    Element_Name("FlagInterlaced");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_FrameRate()
{
    Element_Name("FrameRate");

    //Parsing
   float64 Value=Float_Get();

    //Filling
    FILLING_BEGIN();
        Stream[TrackNumber].FrameRate=Value;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_PixelCropBottom()
{
    Element_Name("PixelCropBottom");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_PixelCropLeft()
{
    Element_Name("PixelCropLeft");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_PixelCropRight()
{
    Element_Name("PixelCropRight");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_PixelCropTop()
{
    Element_Name("PixelCropTop");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_PixelHeight()
{
    Element_Name("PixelHeight");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        Fill(Stream_Video, StreamPos_Last, Video_Height, UInteger, 10, true);
        if (!TrackVideoDisplayHeight)
            TrackVideoDisplayHeight=UInteger; //Default value of DisplayHeight is PixelHeight
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_PixelWidth()
{
    Element_Name("PixelWidth");

    //Parsing
    int64u UInteger=UInteger_Get();

    //Filling
    FILLING_BEGIN();
        Fill(Stream_Video, StreamPos_Last, Video_Width, UInteger, 10, true);
        if (!TrackVideoDisplayWidth)
            TrackVideoDisplayWidth=UInteger; //Default value of DisplayWidth is PixelWidth
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_Video_StereoMode()
{
    Element_Name("StereoMode");

    //Parsing
    int64u UInteger=UInteger_Get(); Element_Info1(Format_Version==2?Mk_StereoMode_v2(UInteger):Mk_StereoMode(UInteger));

    //Filling
    FILLING_BEGIN();
        Fill(Stream_Video, StreamPos_Last, Video_MultiView_Count, 2); //Matroska seems to be limited to 2 views
        Fill(Stream_Video, StreamPos_Last, Video_MultiView_Layout, Format_Version==2?Mk_StereoMode_v2(UInteger):Mk_StereoMode(UInteger));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackOverlay()
{
    Element_Name("TrackOverlay");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackTranslate()
{
    Element_Name("TrackTranslate");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackTranslate_Codec()
{
    Element_Name("Codec");
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackTranslate_EditionUID()
{
    Element_Name("EditionUID");

    //Parsing
    UInteger_Info();
}

//---------------------------------------------------------------------------
void File_Mk::Segment_Tracks_TrackEntry_TrackTranslate_TrackID()
{
    Element_Name("TrackID");

    //Parsing
    UInteger_Info();
}

//***************************************************************************
// Data
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mk::UInteger_Info()
{
    switch (Element_Size)
    {
        case 1 :
                {
                    Info_B1(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 2 :
                {
                    Info_B2(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 3 :
                {
                    Info_B3(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 4 :
                {
                    Info_B4(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 5 :
                {
                    Info_B5(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 6 :
                {
                    Info_B6(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 7 :
                {
                    Info_B7(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 8 :
                {
                    Info_B8(Data,                               "Data"); Element_Info1(Data);
                    return;
                }
        case 16:
                {
                    Info_B16(Data,                              "Data"); Element_Info1(Data);
                    return;
                }
        default : Skip_XX(Element_Size,                         "Data");
    }
}

//---------------------------------------------------------------------------
int64u File_Mk::UInteger_Get()
{
    switch (Element_Size)
    {
        case 1 :
                {
                    int8u Data;
                    Get_B1 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 2 :
                {
                    int16u Data;
                    Get_B2 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 3 :
                {
                    int32u Data;
                    Get_B3 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 4 :
                {
                    int32u Data;
                    Get_B4 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 5 :
                {
                    int64u Data;
                    Get_B5 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 6 :
                {
                    int64u Data;
                    Get_B6 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 7 :
                {
                    int64u Data;
                    Get_B7 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 8 :
                {
                    int64u Data;
                    Get_B8 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        default :   Skip_XX(Element_Size,                       "Data");
                    return 0;
    }
}

//---------------------------------------------------------------------------
int128u File_Mk::UInteger16_Get()
{
    switch (Element_Size)
    {
        case 1 :
                {
                    int8u Data;
                    Get_B1 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 2 :
                {
                    int16u Data;
                    Get_B2 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 3 :
                {
                    int32u Data;
                    Get_B3 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 4 :
                {
                    int32u Data;
                    Get_B4 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 5 :
                {
                    int64u Data;
                    Get_B5 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 6 :
                {
                    int64u Data;
                    Get_B6 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 7 :
                {
                    int64u Data;
                    Get_B7 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 8 :
                {
                    int64u Data;
                    Get_B8 (Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 16:
                {
                    int128u Data;
                    Get_B16(Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        default :   Skip_XX(Element_Size,                       "Data");
                    return 0;
    }
}

//---------------------------------------------------------------------------
float64 File_Mk::Float_Get()
{
    switch (Element_Size)
    {
        case 4 :
                {
                    float32 Data;
                    Get_BF4(Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        case 8 :
                {
                    float64 Data;
                    Get_BF8(Data,                               "Data"); Element_Info1(Data);
                    return Data;
                }
        default :   Skip_XX(Element_Size,                       "Data");
                    return 0.0;
    }
}

//---------------------------------------------------------------------------
void File_Mk::Float_Info()
{
    switch (Element_Size)
    {
        case 4 :
                {
                    Info_BF4(Data,                              "Data"); Element_Info1(Data);
                    return;
                }
        case 8 :
                {
                    Info_BF8(Data,                              "Data"); Element_Info1(Data);
                    return;
                }
        default :   Skip_XX(Element_Size,                       "Data");
                    return;
    }
}

//---------------------------------------------------------------------------
Ztring File_Mk::UTF8_Get()
{
    Ztring Data;
    Get_UTF8(Element_Size, Data,                                "Data"); Element_Info1(Data);
    return Data;
}

//---------------------------------------------------------------------------
void File_Mk::UTF8_Info()
{
    Info_UTF8(Element_Size, Data,                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
Ztring File_Mk::Local_Get()
{
    Ztring Data;
    Get_Local(Element_Size, Data,                               "Data"); Element_Info1(Data);
    return Data;
}

//---------------------------------------------------------------------------
void File_Mk::Local_Info()
{
    Info_Local(Element_Size, Data,                              "Data"); Element_Info1(Data);
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mk::CodecID_Manage()
{
    if (TrackType==(int64u)-1 || TrackNumber==(int64u)-1 || CodecID.empty() || Stream[TrackNumber].Parser)
        return; //Not ready (or not needed)

    if (Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_CodecID)).empty())
    {
        CodecID_Fill(CodecID, StreamKind_Last, StreamPos_Last, InfoCodecID_Format_Matroska);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), CodecID);
    }

    //Creating the parser
    #if defined(MEDIAINFO_MPEG4V_YES) || defined(MEDIAINFO_AVC_YES) || defined(MEDIAINFO_HEVC_YES) || defined(MEDIAINFO_VC1_YES) || defined(MEDIAINFO_DIRAC_YES) || defined(MEDIAINFO_MPEGV_YES) || defined(MEDIAINFO_VP8_YES) || defined(MEDIAINFO_OGG_YES)
        const Ztring &Format=MediaInfoLib::Config.CodecID_Get(StreamKind_Last, InfoCodecID_Format_Type, CodecID, InfoCodecID_Format);
    #endif
        if (0);
    #if defined(MEDIAINFO_MPEG4V_YES)
    else if (Format==__T("MPEG-4 Visual"))
    {
        Stream[TrackNumber].Parser=new File_Mpeg4v;
        ((File_Mpeg4v*)Stream[TrackNumber].Parser)->FrameIsAlwaysComplete=true;
        ((File_Mpeg4v*)Stream[TrackNumber].Parser)->Frame_Count_Valid=1;
    }
    #endif
    #if defined(MEDIAINFO_AVC_YES)
    else if (Format==__T("AVC"))
    {
        Stream[TrackNumber].Parser=new File_Avc;
        ((File_Avc*)Stream[TrackNumber].Parser)->FrameIsAlwaysComplete=true;
        if (InfoCodecID_Format_Type==InfoCodecID_Format_Matroska)
        {
            ((File_Avc*)Stream[TrackNumber].Parser)->MustSynchronize=false;
            ((File_Avc*)Stream[TrackNumber].Parser)->MustParse_SPS_PPS=true;
            ((File_Avc*)Stream[TrackNumber].Parser)->SizedBlocks=true;
        }
    }
    #endif
    #if defined(MEDIAINFO_HEVC_YES)
    else if (Format==__T("HEVC"))
    {
        Stream[TrackNumber].Parser=new File_Hevc;
        ((File_Hevc*)Stream[TrackNumber].Parser)->FrameIsAlwaysComplete=true;
        if (InfoCodecID_Format_Type==InfoCodecID_Format_Matroska)
        {
            ((File_Hevc*)Stream[TrackNumber].Parser)->MustSynchronize=false;
            ((File_Hevc*)Stream[TrackNumber].Parser)->MustParse_VPS_SPS_PPS=true;
            ((File_Hevc*)Stream[TrackNumber].Parser)->MustParse_VPS_SPS_PPS_FromMatroska=true;
            ((File_Hevc*)Stream[TrackNumber].Parser)->SizedBlocks=true;
            #if MEDIAINFO_DEMUX
                if (Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get())
                {
                    Stream[TrackNumber].Parser->Demux_Level=2; //Container
                    Stream[TrackNumber].Parser->Demux_UnpacketizeContainer=true;
                }
            #endif //MEDIAINFO_DEMUX
        }
    }
    #endif
    #if defined(MEDIAINFO_VC1_YES)
    else if (Format==__T("VC-1"))
    {
        Stream[TrackNumber].Parser=new File_Vc1;
        ((File_Vc1*)Stream[TrackNumber].Parser)->FrameIsAlwaysComplete=true;
        ((File_Vc1*)Stream[TrackNumber].Parser)->Frame_Count_Valid=1;
    }
    #endif
    #if defined(MEDIAINFO_DIRAC_YES)
    else if (Format==__T("Dirac"))
    {
        Stream[TrackNumber].Parser=new File_Dirac;
        ((File_Dirac*)Stream[TrackNumber].Parser)->Frame_Count_Valid=1;
    }
    #endif
    #if defined(MEDIAINFO_MPEGV_YES)
    else if (Format==__T("MPEG Video"))
    {
        Stream[TrackNumber].Parser=new File_Mpegv;
        ((File_Mpegv*)Stream[TrackNumber].Parser)->FrameIsAlwaysComplete=true;
        ((File_Mpegv*)Stream[TrackNumber].Parser)->Frame_Count_Valid=1;
    }
    #endif
    #if defined(MEDIAINFO_VP8_YES)
    else if (Format==__T("VP8"))
    {
        Stream[TrackNumber].Parser=new File_Vp8;
    }
    #endif
    #if defined(MEDIAINFO_OGG_YES)
    else if (Format==__T("Theora")  || Format==__T("Vorbis"))
    {
        Stream[TrackNumber].Parser=new File_Ogg;
        Stream[TrackNumber].Parser->MustSynchronize=false;
        ((File_Ogg*)Stream[TrackNumber].Parser)->XiphLacing=true;
    }
    #endif
    #if defined(MEDIAINFO_RM_YES)
    else if (CodecID.find(__T("V_REAL/"))==0)
    {
        Stream[TrackNumber].Parser=new File_Rm;
        ((File_Rm*)Stream[TrackNumber].Parser)->FromMKV_StreamType=Stream_Video;
    }
    #endif
    #if defined(MEDIAINFO_AC3_YES)
    else if (Format==__T("AC-3") || Format==__T("E-AC-3") || Format==__T("TrueHD"))
    {
        Stream[TrackNumber].Parser=new File_Ac3;
        ((File_Ac3*)Stream[TrackNumber].Parser)->Frame_Count_Valid=2;
    }
    #endif
    #if defined(MEDIAINFO_DTS_YES)
    else if (Format==__T("DTS"))
    {
        Stream[TrackNumber].Parser=new File_Dts;
        ((File_Dts*)Stream[TrackNumber].Parser)->Frame_Count_Valid=2;
    }
    #endif
    #if defined(MEDIAINFO_AAC_YES)
    else if (CodecID==(__T("A_AAC")))
    {
        Stream[TrackNumber].Parser=new File_Aac;
        ((File_Aac*)Stream[TrackNumber].Parser)->Mode=File_Aac::Mode_AudioSpecificConfig;
    }
    #endif
    #if defined(MEDIAINFO_AAC_YES)
    else if (CodecID.find(__T("A_AAC/"))==0)
    {
        Ztring Profile;
        int8u audioObjectType=0, Version=0, SBR=2, PS=2;
             if (CodecID==__T("A_AAC/MPEG2/MAIN"))     {Version=2; Profile=__T("Main");                   audioObjectType=1;}
        else if (CodecID==__T("A_AAC/MPEG2/LC"))       {Version=2; Profile=__T("LC");                     audioObjectType=2; SBR=0;}
        else if (CodecID==__T("A_AAC/MPEG2/LC/SBR"))   {Version=2; Profile=__T("HE-AAC / LC");            audioObjectType=2; SBR=1;}
        else if (CodecID==__T("A_AAC/MPEG2/SSR"))      {Version=2; Profile=__T("SSR");                    audioObjectType=3;}
        else if (CodecID==__T("A_AAC/MPEG4/MAIN"))     {Version=4; Profile=__T("Main");                   audioObjectType=1;}
        else if (CodecID==__T("A_AAC/MPEG4/LC"))       {Version=4; Profile=__T("LC");                     audioObjectType=2; SBR=0;}
        else if (CodecID==__T("A_AAC/MPEG4/LC/SBR"))   {Version=4; Profile=__T("HE-AAC / LC");            audioObjectType=2; SBR=1; PS=0;}
        else if (CodecID==__T("A_AAC/MPEG4/LC/SBR/PS")){Version=4; Profile=__T("HE-AACv2 / HE-AAC / LC"); audioObjectType=2; SBR=1; PS=1;}
        else if (CodecID==__T("A_AAC/MPEG4/SSR"))      {Version=4; Profile=__T("SSR");                    audioObjectType=3;}
        else if (CodecID==__T("A_AAC/MPEG4/LTP"))      {Version=4; Profile=__T("LTP");                    audioObjectType=4;}
        else if (CodecID==__T("raac"))                 {           Profile=__T("LC");                     audioObjectType=2;}
        else if (CodecID==__T("racp"))                 {           Profile=__T("HE-AAC / LC");            audioObjectType=2; SBR=1; PS=0;}

        if (Version>0)
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Version, Version==2?"Version 2":"Version 4");
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, Profile);
        if (SBR!=2)
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_SBR, SBR?"Yes":"No");
        if (PS!=2)
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Settings_PS, PS?"Yes":"No");
        int32u sampling_frequency=Retrieve(Stream_Audio, StreamPos_Last, Audio_SamplingRate).To_int32u();

        Stream[TrackNumber].Parser=new File_Aac;
        ((File_Aac*)Stream[TrackNumber].Parser)->Mode=File_Aac::Mode_AudioSpecificConfig;
        ((File_Aac*)Stream[TrackNumber].Parser)->AudioSpecificConfig_OutOfBand(sampling_frequency, audioObjectType, SBR==1?true:false, PS==1?true:false, SBR==1?true:false, PS==1?true:false);
    }
    #endif
    #if defined(MEDIAINFO_AAC_YES)
    else if (Format==(__T("AAC")))
    {
        Stream[TrackNumber].Parser=new File_Aac;
        ((File_Aac*)Stream[TrackNumber].Parser)->Mode=File_Aac::Mode_ADTS;
    }
    #endif
    #if defined(MEDIAINFO_MPEGA_YES)
    else if (Format==__T("MPEG Audio"))
    {
        Stream[TrackNumber].Parser=new File_Mpega;
    }
    #endif
    #if defined(MEDIAINFO_FLAC_YES)
    else if (Format==__T("Flac"))
    {
        Stream[TrackNumber].Parser=new File_Flac;
    }
    #endif
    #if defined(MEDIAINFO_OPUS_YES)
    else if (CodecID.find(__T("A_OPUS"))==0) //http://wiki.xiph.org/MatroskaOpus
    {
        Stream[TrackNumber].Parser=new File_Opus;
    }
    #endif
    #if defined(MEDIAINFO_WVPK_YES)
    else if (Format==__T("WavPack"))
    {
        Stream[TrackNumber].Parser=new File_Wvpk;
        ((File_Wvpk*)Stream[TrackNumber].Parser)->FromMKV=true;
    }
    #endif
    #if defined(MEDIAINFO_TTA_YES)
    else if (Format==__T("TTA"))
    {
        //Stream[TrackNumber].Parser=new File_Tta; //Parser is not needed, because header is useless and dropped (the parser analyses only the header)
    }
    #endif
    #if defined(MEDIAINFO_PCM_YES)
    else if (Format==__T("PCM"))
    {
        Stream[TrackNumber].Parser=new File_Pcm;
        ((File_Pcm*)Stream[TrackNumber].Parser)->Codec=CodecID;
    }
    #endif
    #if defined(MEDIAINFO_RM_YES)
    else if (CodecID.find(__T("A_REAL/"))==0)
    {
        Stream[TrackNumber].Parser=new File_Rm;
        ((File_Rm*)Stream[TrackNumber].Parser)->FromMKV_StreamType=Stream_Audio;
    }
    #endif
    Element_Code=TrackNumber;
    Open_Buffer_Init(Stream[TrackNumber].Parser);

    CodecID.clear();
}

//---------------------------------------------------------------------------
void File_Mk::CodecPrivate_Manage()
{
    if (CodecPrivate==NULL || TrackNumber==(int64u)-1 || TrackType==(int64u)-1)
        return; //Not ready (or not needed)

    //Codec Private is already here, so we can parse it now
    const int8u* Buffer_Save=Buffer;
    size_t Buffer_Offset_Save=Buffer_Offset;
    size_t Buffer_Size_Save=Buffer_Size;
    int64u Element_Size_Save=Element_Size;
    Buffer=CodecPrivate;
    Buffer_Offset=0;
    Buffer_Size=CodecPrivate_Size;
    Element_Offset=0;
    Element_Size=Buffer_Size;
    Segment_Tracks_TrackEntry_CodecPrivate();
    Buffer=Buffer_Save;
    Buffer_Offset=Buffer_Offset_Save;
    Buffer_Size=Buffer_Size_Save;
    Element_Size=Element_Size_Save;
    Element_Offset=Element_Size_Save;
    delete[] CodecPrivate; CodecPrivate=NULL;
    CodecPrivate_Size=0;
    Element_Name("(Multiple info)");
}

} //NameSpace

#endif //MEDIAINFO_MK_YES

