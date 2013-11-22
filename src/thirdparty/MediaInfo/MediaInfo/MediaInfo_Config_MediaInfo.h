/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Configuration of MediaInfo (per MediaInfo block)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Config_MediaInfoH
#define MediaInfo_Config_MediaInfoH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config.h"
    #include "MediaInfo/MediaInfo_Events.h"
    #include "ZenLib/File.h"
#endif //MEDIAINFO_EVENTS
#include "ZenLib/CriticalSection.h"
#include "ZenLib/Translation.h"
#include "ZenLib/InfoMap.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

#if MEDIAINFO_EVENTS
    class File__Analyze;
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// Class MediaInfo_Config_MediaInfo
//***************************************************************************

class MediaInfo_Config_MediaInfo
{
public :
    //Constructor/Destructor
    MediaInfo_Config_MediaInfo();
    ~MediaInfo_Config_MediaInfo();

    //General
    Ztring Option (const String &Option, const String &Value=Ztring());

    void          File_IsSeekable_Set (bool NewValue);
    bool          File_IsSeekable_Get ();

    void          File_IsSub_Set (bool NewValue);
    bool          File_IsSub_Get ();

    void          File_IsDetectingDuration_Set (bool NewValue);
    bool          File_IsDetectingDuration_Get ();

    void          File_IsReferenced_Set (bool NewValue);
    bool          File_IsReferenced_Get ();

    void          File_TestContinuousFileNames_Set (bool NewValue);
    bool          File_TestContinuousFileNames_Get ();

    void          File_KeepInfo_Set (bool NewValue);
    bool          File_KeepInfo_Get ();

    void          File_StopAfterFilled_Set (bool NewValue);
    bool          File_StopAfterFilled_Get ();

    void          File_StopSubStreamAfterFilled_Set (bool NewValue);
    bool          File_StopSubStreamAfterFilled_Get ();

    void          File_Audio_MergeMonoStreams_Set (bool NewValue);
    bool          File_Audio_MergeMonoStreams_Get ();

    void          File_Demux_Interleave_Set (bool NewValue);
    bool          File_Demux_Interleave_Get ();

    void          File_ID_OnlyRoot_Set (bool NewValue);
    bool          File_ID_OnlyRoot_Get ();

    #if MEDIAINFO_ADVANCED
        void          File_IgnoreSequenceFileSize_Set (bool NewValue);
        bool          File_IgnoreSequenceFileSize_Get ();
    #endif //MEDIAINFO_ADVANCED

    #if MEDIAINFO_ADVANCED
        void          File_DefaultFrameRate_Set (float64 NewValue);
        float64       File_DefaultFrameRate_Get ();
    #endif //MEDIAINFO_ADVANCED

    #if MEDIAINFO_ADVANCED
        void          File_Source_List_Set (bool NewValue);
        bool          File_Source_List_Get ();
    #endif //MEDIAINFO_ADVANCED

    #if MEDIAINFO_ADVANCED
        void          File_RiskyBitRateEstimation_Set (bool NewValue);
        bool          File_RiskyBitRateEstimation_Get ();
    #endif //MEDIAINFO_ADVANCED

    #if MEDIAINFO_DEMUX
        #if MEDIAINFO_ADVANCED
            void          File_Demux_Unpacketize_StreamLayoutChange_Skip_Set (bool NewValue);
            bool          File_Demux_Unpacketize_StreamLayoutChange_Skip_Get ();
        #endif //MEDIAINFO_ADVANCED
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_MD5
        void          File_Md5_Set (bool NewValue);
        bool          File_Md5_Get ();
    #endif //MEDIAINFO_MD5

    #if defined(MEDIAINFO_REFERENCES_YES)
        void          File_CheckSideCarFiles_Set (bool NewValue);
        bool          File_CheckSideCarFiles_Get ();
    #endif //defined(MEDIAINFO_REFERENCES_YES)

    void          File_FileName_Set (const Ztring &NewValue);
    Ztring        File_FileName_Get ();

    void          File_FileNameFormat_Set (const Ztring &NewValue);
    Ztring        File_FileNameFormat_Get ();

    void          File_TimeToLive_Set (float64 NewValue);
    float64       File_TimeToLive_Get ();

    void          File_Partial_Begin_Set (const Ztring &NewValue);
    Ztring        File_Partial_Begin_Get ();
    void          File_Partial_End_Set (const Ztring &NewValue);
    Ztring        File_Partial_End_Get ();

    void          File_ForceParser_Set (const Ztring &NewValue);
    Ztring        File_ForceParser_Get ();

    void          File_Buffer_Size_Hint_Pointer_Set (size_t* NewValue);
    size_t*       File_Buffer_Size_Hint_Pointer_Get ();

    #if MEDIAINFO_NEXTPACKET
    void          NextPacket_Set (bool NewValue);
    bool          NextPacket_Get ();
    #endif //MEDIAINFO_NEXTPACKET

    #if MEDIAINFO_FILTER
    void          File_Filter_Set     (int64u NewValue);
    bool          File_Filter_Get     (const int16u  Value);
    bool          File_Filter_Get     ();
    void          File_Filter_Audio_Set (bool NewValue);
    bool          File_Filter_Audio_Get ();
    bool          File_Filter_HasChanged();
    #endif //MEDIAINFO_FILTER

    #if MEDIAINFO_DUPLICATE
    Ztring        File_Duplicate_Set  (const Ztring &Value);
    Ztring        File_Duplicate_Get  (size_t AlreadyRead_Pos); //Requester must say how many Get() it already read
    bool          File_Duplicate_Get_AlwaysNeeded (size_t AlreadyRead_Pos); //Requester must say how many Get() it already read
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_DUPLICATE
    size_t        File__Duplicate_Memory_Indexes_Get (const Ztring &ToFind);
    void          File__Duplicate_Memory_Indexes_Erase (const Ztring &ToFind);
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_EVENTS
    ZtringListList SubFile_Config_Get ();
    void          SubFile_StreamID_Set(int64u Value);
    int64u        SubFile_StreamID_Get();
    void          SubFile_IDs_Set(Ztring Value);
    Ztring        SubFile_IDs_Get();
    #endif //MEDIAINFO_EVENTS

    #if MEDIAINFO_EVENTS
    bool          ParseUndecodableFrames_Get ();
    void          ParseUndecodableFrames_Set (bool Value);
    #endif //MEDIAINFO_EVENTS

    #if MEDIAINFO_EVENTS
    bool          Event_CallBackFunction_IsSet ();
    Ztring        Event_CallBackFunction_Set (const Ztring &Value);
    Ztring        Event_CallBackFunction_Get ();
    void          Event_Send(File__Analyze* Source, const int8u* Data_Content, size_t Data_Size, const Ztring &File_Name=Ztring());
    void          Event_Accepted(File__Analyze* Source);
    void          Event_SubFile_Start(const Ztring &FileName_Absolute);
    #endif //MEDIAINFO_EVENTS

    #if MEDIAINFO_DEMUX
    void          Demux_ForceIds_Set (bool NewValue);
    bool          Demux_ForceIds_Get ();
    void          Demux_PCM_20bitTo16bit_Set (bool NewValue);
    bool          Demux_PCM_20bitTo16bit_Get ();
    void          Demux_PCM_20bitTo24bit_Set (bool NewValue);
    bool          Demux_PCM_20bitTo24bit_Get ();
    void          Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Set (bool NewValue);
    bool          Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get ();
    void          Demux_Unpacketize_Set (bool NewValue);
    bool          Demux_Unpacketize_Get ();
    void          Demux_Rate_Set (float64 NewValue);
    float64       Demux_Rate_Get ();
    void          Demux_FirstDts_Set (int64u NewValue);
    int64u        Demux_FirstDts_Get ();
    void          Demux_FirstFrameNumber_Set (int64u NewValue);
    int64u        Demux_FirstFrameNumber_Get ();
    void          Demux_InitData_Set (int8u NewValue);
    int8u         Demux_InitData_Get ();
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_IBI
    void          Ibi_Set (const Ztring &NewValue);
    std::string   Ibi_Get ();
    void          Ibi_Create_Set (bool NewValue);
    bool          Ibi_Create_Get ();
    void          Ibi_UseIbiInfoIfAvailable_Set (bool NewValue);
    bool          Ibi_UseIbiInfoIfAvailable_Get ();
    #endif //MEDIAINFO_IBI

    //Specific
    void          File_MpegTs_ForceMenu_Set (bool NewValue);
    bool          File_MpegTs_ForceMenu_Get ();
    void          File_MpegTs_stream_type_Trust_Set (bool NewValue);
    bool          File_MpegTs_stream_type_Trust_Get ();
    void          File_MpegTs_Atsc_transport_stream_id_Trust_Set (bool NewValue);
    bool          File_MpegTs_Atsc_transport_stream_id_Trust_Get ();
    void          File_MpegTs_RealTime_Set (bool NewValue);
    bool          File_MpegTs_RealTime_Get ();
    void          File_Bdmv_ParseTargetedFile_Set (bool NewValue);
    bool          File_Bdmv_ParseTargetedFile_Get ();
    #if defined(MEDIAINFO_DVDIF_YES)
    void          File_DvDif_DisableAudioIfIsInContainer_Set (bool NewValue);
    bool          File_DvDif_DisableAudioIfIsInContainer_Get ();
    void          File_DvDif_IgnoreTransmittingFlags_Set (bool NewValue);
    bool          File_DvDif_IgnoreTransmittingFlags_Get ();
    #endif //defined(MEDIAINFO_DVDIF_YES)
    #if defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    void          File_DvDif_Analysis_Set (bool NewValue);
    bool          File_DvDif_Analysis_Get ();
    #endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    #if MEDIAINFO_MACROBLOCKS
    void          File_Macroblocks_Parse_Set (bool NewValue);
    bool          File_Macroblocks_Parse_Get ();
    #endif //MEDIAINFO_MACROBLOCKS
    void          File_GrowingFile_Delay_Set(float64 Value);
    float64       File_GrowingFile_Delay_Get();
    #if defined(MEDIAINFO_LIBCURL_YES)
    void          File_Curl_Set (const Ztring &NewValue);
    void          File_Curl_Set (const Ztring &Field, const Ztring &NewValue);
    Ztring        File_Curl_Get (const Ztring &Field);
    #endif //defined(MEDIAINFO_LIBCURL_YES)
    #if defined(MEDIAINFO_LIBMMS_YES)
    void          File_Mmsh_Describe_Only_Set (bool NewValue);
    bool          File_Mmsh_Describe_Only_Get ();
    #endif //defined(MEDIAINFO_LIBMMS_YES)
    void          File_Eia608_DisplayEmptyStream_Set (bool NewValue);
    bool          File_Eia608_DisplayEmptyStream_Get ();
    void          File_Eia708_DisplayEmptyStream_Set (bool NewValue);
    bool          File_Eia708_DisplayEmptyStream_Get ();
    #if defined(MEDIAINFO_AC3_YES)
    void          File_Ac3_IgnoreCrc_Set (bool NewValue);
    bool          File_Ac3_IgnoreCrc_Get ();
    #endif //defined(MEDIAINFO_AC3_YES)

    //Analysis internal
    void          State_Set (float State);
    float         State_Get ();

    //Internal to MediaInfo, not thread safe
    ZtringList    File_Names;
    std::vector<int64u> File_Sizes;
    size_t        File_Names_Pos;
    size_t        File_Buffer_Size_Max;
    size_t        File_Buffer_Size_ToRead;
    size_t        File_Buffer_Size;
    int8u*        File_Buffer;
    bool          File_Buffer_Repeat;
    bool          File_Buffer_Repeat_IsSupported;
    bool          File_IsGrowing;
    bool          File_IsNotGrowingAnymore;
    int64u        File_Current_Offset;
    int64u        File_Current_Size;
    int64u        File_Size;
    float32       ParseSpeed;
    #if MEDIAINFO_EVENTS
    Ztring        File_Names_RootDirectory;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
    bool          Demux_EventWasSent;
    int64u          Demux_Offset_Frame;
    int64u          Demux_Offset_DTS;
    int64u          Demux_Offset_DTS_FromStream;
    File__Analyze*  Events_Delayed_CurrentSource;
        #if MEDIAINFO_SEEK
        bool      Demux_IsSeeking;
        #endif //MEDIAINFO_SEEK
    #endif //MEDIAINFO_DEMUX

private :
    bool                    FileIsSeekable;
    bool                    FileIsSub;
    bool                    FileIsDetectingDuration;
    bool                    FileIsReferenced;
    bool                    FileTestContinuousFileNames;
    bool                    FileKeepInfo;
    bool                    FileStopAfterFilled;
    bool                    FileStopSubStreamAfterFilled;
    bool                    Audio_MergeMonoStreams;
    bool                    File_Demux_Interleave;
    bool                    File_ID_OnlyRoot;
    #if MEDIAINFO_ADVANCED
        bool                File_IgnoreSequenceFileSize;
        float64             File_DefaultFrameRate;
        bool                File_Source_List;
        bool                File_RiskyBitRateEstimation;
        #if MEDIAINFO_DEMUX
            bool                File_Demux_Unpacketize_StreamLayoutChange_Skip;
        #endif //MEDIAINFO_DEMUX
    #endif //MEDIAINFO_ADVANCED
    #if MEDIAINFO_MD5
        bool                File_Md5;
    #endif //MEDIAINFO_MD5
    #if defined(MEDIAINFO_REFERENCES_YES)
        bool                File_CheckSideCarFiles;
    #endif //defined(MEDIAINFO_REFERENCES_YES)
    Ztring                  File_FileName;
    Ztring                  File_FileNameFormat;
    float64                 File_TimeToLive;
    Ztring                  File_Partial_Begin;
    Ztring                  File_Partial_End;
    Ztring                  File_ForceParser;
    size_t*                 File_Buffer_Size_Hint_Pointer;

    //Extra
    #if MEDIAINFO_NEXTPACKET
    bool                    NextPacket;
    #endif //MEDIAINFO_NEXTPACKET

    #if MEDIAINFO_FILTER
    std::map<int16u, bool>  File_Filter_16;
    bool                    File_Filter_Audio;
    bool                    File_Filter_HasChanged_;
    #endif //MEDIAINFO_FILTER

    #if MEDIAINFO_DUPLICATE
    std::vector<Ztring>     File__Duplicate_List;
    ZtringList              File__Duplicate_Memory_Indexes;
    #endif //MEDIAINFO_DUPLICATE

    //Event
    #if MEDIAINFO_EVENTS
    MediaInfo_Event_CallBackFunction* Event_CallBackFunction; //void Event_Handler(unsigned char* Data_Content, size_t Data_Size, void* UserHandler)
    struct event_delayed
    {
        int8u* Data_Content;
        size_t Data_Size;
        Ztring File_Name;

        event_delayed (const int8u* Data_Content_, size_t Data_Size_, const Ztring &File_Name_)
        {
            File_Name=File_Name_;
            Data_Size=Data_Size_;
            Data_Content=new int8u[Data_Size];
            std::memcpy(Data_Content, Data_Content_, Data_Size);
        }

        ~event_delayed ()
        {
            delete[] Data_Content; //Data_Content=NULL;
        }
    };
    typedef std::map<File__Analyze*, std::vector<event_delayed*> > events_delayed;
    events_delayed Events_Delayed;
    void*                   Event_UserHandler;
    ZtringListList          SubFile_Config;
    int64u                  SubFile_StreamID;
    bool                    ParseUndecodableFrames;
    Ztring                  SubFile_IDs;
    #endif //MEDIAINFO_EVENTS

    #if MEDIAINFO_DEMUX
    bool                    Demux_ForceIds;
    bool                    Demux_PCM_20bitTo16bit;
    bool                    Demux_PCM_20bitTo24bit;
    bool                    Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10;
    bool                    Demux_Unpacketize;
    float64                 Demux_Rate;
    int64u                  Demux_FirstDts;
    int64u                  Demux_FirstFrameNumber;
    int8u                   Demux_InitData;
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_IBI
    std::string             Ibi;
    bool                    Ibi_Create;
    bool                    Ibi_UseIbiInfoIfAvailable;
    #endif //MEDIAINFO_IBI

    //Specific
    bool                    File_MpegTs_ForceMenu;
    bool                    File_MpegTs_stream_type_Trust;
    bool                    File_MpegTs_Atsc_transport_stream_id_Trust;
    bool                    File_MpegTs_RealTime;
    bool                    File_Bdmv_ParseTargetedFile;
    #if defined(MEDIAINFO_DVDIF_YES)
    bool                    File_DvDif_DisableAudioIfIsInContainer;
    bool                    File_DvDif_IgnoreTransmittingFlags;
    #endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    #if defined(MEDIAINFO_DVDIF_YES)
    bool                    File_DvDif_Analysis;
    #endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    #if MEDIAINFO_MACROBLOCKS
    bool                    File_Macroblocks_Parse;
    #endif //MEDIAINFO_MACROBLOCKS
    float64                 File_GrowingFile_Delay;
    #if defined(MEDIAINFO_LIBMMS_YES)
    bool                    File_Mmsh_Describe_Only;
    #endif //defined(MEDIAINFO_LIBMMS_YES)
    bool                    File_Eia608_DisplayEmptyStream;
    bool                    File_Eia708_DisplayEmptyStream;
    #if defined(MEDIAINFO_AC3_YES)
    bool                    File_Ac3_IgnoreCrc;
    #endif //defined(MEDIAINFO_AC3_YES)

    //Analysis internal
    float                   State;

    //Generic
    #if defined(MEDIAINFO_LIBCURL_YES)
    std::map<Ztring, Ztring> Curl;
    #endif //defined(MEDIAINFO_LIBCURL_YES)

    ZenLib::CriticalSection CS;

    //Constructor
    MediaInfo_Config_MediaInfo (const MediaInfo_Config_MediaInfo&);             // Prevent copy-construction
    MediaInfo_Config_MediaInfo& operator=(const MediaInfo_Config_MediaInfo&);   // Prevent assignment
};

} //NameSpace

#endif
