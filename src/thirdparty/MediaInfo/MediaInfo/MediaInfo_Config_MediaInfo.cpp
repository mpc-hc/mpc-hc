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
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/ZtringListListF.h"
#if MEDIAINFO_EVENTS
    #include "ZenLib/FileName.h"
#endif //MEDIAINFO_EVENTS
#if MEDIAINFO_IBI
    #include "base64.h"
#endif //MEDIAINFO_IBI
#include <algorithm>
#if MEDIAINFO_DEMUX
    #include <cmath>
#endif //MEDIAINFO_DEMUX
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

const size_t Buffer_NormalSize=/*188*7;//*/64*1024;

//***************************************************************************
// Info
//***************************************************************************

MediaInfo_Config_MediaInfo::MediaInfo_Config_MediaInfo()
{
    FileIsSeekable=true;
    FileIsSub=false;
    FileIsDetectingDuration=false;
    FileIsReferenced=false;
    FileTestContinuousFileNames=true;
    FileKeepInfo=false;
    FileStopAfterFilled=false;
    FileStopSubStreamAfterFilled=false;
    Audio_MergeMonoStreams=false;
    File_Demux_Interleave=false;
    File_ID_OnlyRoot=false;
    #if MEDIAINFO_ADVANCED
        File_IgnoreSequenceFileSize=false;
        File_IgnoreSequenceFilesCount=false;
        File_DefaultFrameRate=0;
        File_Source_List=false;
        File_RiskyBitRateEstimation=false;
        #if MEDIAINFO_DEMUX
            File_Demux_Unpacketize_StreamLayoutChange_Skip=false;
        #endif //MEDIAINFO_DEMUX
    #endif //MEDIAINFO_ADVANCED
    #if MEDIAINFO_MD5
        File_Md5=false;
    #endif //MEDIAINFO_MD5
    #if defined(MEDIAINFO_REFERENCES_YES)
        File_CheckSideCarFiles=false;
    #endif //defined(MEDIAINFO_REFERENCES_YES)
    File_TimeToLive=0;
    File_Buffer_Size_Hint_Pointer=NULL;
    #if MEDIAINFO_NEXTPACKET
        NextPacket=false;
    #endif //MEDIAINFO_NEXTPACKET
    #if MEDIAINFO_FILTER
        File_Filter_Audio=false;
        File_Filter_HasChanged_=false;
    #endif //MEDIAINFO_FILTER
    #if MEDIAINFO_EVENTS
        Event_CallBackFunction=NULL;
        Event_UserHandler=NULL;
        SubFile_StreamID=(int64u)-1;
        ParseUndecodableFrames=false;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_ForceIds=false;
        Demux_PCM_20bitTo16bit=false;
        Demux_PCM_20bitTo24bit=false;
        Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10=false;
        Demux_Unpacketize=false;
        Demux_Rate=0;
        Demux_FirstDts=(int64u)-1;
        Demux_FirstFrameNumber=(int64u)-1;
        Demux_InitData=0; //In Demux event
    #endif //MEDIAINFO_DEMUX
    #if MEDIAINFO_IBI
        Ibi_Create=false;
        Ibi_UseIbiInfoIfAvailable=false;
    #endif //MEDIAINFO_IBI

    //Specific
    File_MpegTs_ForceMenu=false;
    File_MpegTs_stream_type_Trust=true;
    File_MpegTs_Atsc_transport_stream_id_Trust=true;
    File_MpegTs_RealTime=false;
    File_Bdmv_ParseTargetedFile=true;
    #if defined(MEDIAINFO_DVDIF_YES)
    File_DvDif_DisableAudioIfIsInContainer=false;
    File_DvDif_IgnoreTransmittingFlags=false;
    #endif //defined(MEDIAINFO_DVDIF_YES)
    #if defined(MEDIAINFO_DVDIF_ANALYZE_YES)
        File_DvDif_Analysis=false;
    #endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    #if MEDIAINFO_MACROBLOCKS
        File_Macroblocks_Parse=false;
    #endif //MEDIAINFO_MACROBLOCKS
    File_GrowingFile_Delay=10;
    #if defined(MEDIAINFO_LIBMMS_YES)
        File_Mmsh_Describe_Only=false;
    #endif //defined(MEDIAINFO_LIBMMS_YES)
    File_Eia608_DisplayEmptyStream=false;
    File_Eia708_DisplayEmptyStream=false;
    State=0;
    #if defined(MEDIAINFO_AC3_YES)
    File_Ac3_IgnoreCrc=false;
    #endif //defined(MEDIAINFO_AC3_YES)

    //Internal to MediaInfo, not thread safe
    File_Names_Pos=0;
    File_Buffer=NULL;
    File_Buffer_Size_Max=0;
    File_Buffer_Size_ToRead=Buffer_NormalSize;
    File_Buffer_Size=0;
    File_Buffer_Repeat=false;
    File_Buffer_Repeat_IsSupported=false;
    File_IsGrowing=false;
    File_IsNotGrowingAnymore=false;
    File_Current_Offset=0;
    File_Current_Size=(int64u)-1;
    File_IgnoreFramesBefore=0;
    File_IgnoreFramesAfter=(int64u)-1;
    File_IgnoreFramesRate=0;
    File_Size=(int64u)-1;
    ParseSpeed=MediaInfoLib::Config.ParseSpeed_Get();
    #if MEDIAINFO_EVENTS
        Config_PerPackage=NULL;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent=false;
        Demux_Offset_Frame=(int64u)-1;
        Demux_Offset_DTS=(int64u)-1;
        Demux_Offset_DTS_FromStream=(int64u)-1;
        Events_Delayed_CurrentSource=NULL;
        #if MEDIAINFO_SEEK
           Demux_IsSeeking=false;
        #endif //MEDIAINFO_SEEK
    #endif //MEDIAINFO_DEMUX
}

MediaInfo_Config_MediaInfo::~MediaInfo_Config_MediaInfo()
{
    delete[] File_Buffer; //File_Buffer=NULL;

    #if MEDIAINFO_EVENTS
        for (events_delayed::iterator Event=Events_Delayed.begin(); Event!=Events_Delayed.end(); ++Event)
            for (size_t Pos=0; Pos<Event->second.size(); Pos++)
                delete Event->second[Pos]; //Event->second[Pos]=NULL;
    #endif //MEDIAINFO_EVENTS
}

//***************************************************************************
// Info
//***************************************************************************

Ztring MediaInfo_Config_MediaInfo::Option (const String &Option, const String &Value)
{
    #if MEDIAINFO_EVENTS
        SubFile_Config(Option)=Value;
    #endif //MEDIAINFO_EVENTS

    String Option_Lower(Option);
    size_t Egal_Pos=Option_Lower.find(__T('='));
    if (Egal_Pos==string::npos)
        Egal_Pos=Option_Lower.size();
    transform(Option_Lower.begin(), Option_Lower.begin()+Egal_Pos, Option_Lower.begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix

    if (Option_Lower==__T("file_isseekable"))
    {
        File_IsSeekable_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_isseekable_get"))
    {
        return File_IsSeekable_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_issub"))
    {
        File_IsSub_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_issub_get"))
    {
        return File_IsSub_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_isdetectingduration"))
    {
        File_IsDetectingDuration_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_isdetectingduration_get"))
    {
        return File_IsDetectingDuration_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_isreferenced"))
    {
        File_IsReferenced_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_isreferenced_get"))
    {
        return File_IsReferenced_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_testcontinuousfilenames"))
    {
        File_TestContinuousFileNames_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_testcontinuousfilenames_get"))
    {
        return File_TestContinuousFileNames_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_keepinfo"))
    {
        File_KeepInfo_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_keepinfo_get"))
    {
        return File_KeepInfo_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_stopafterfilled"))
    {
        File_StopAfterFilled_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_stopafterfilled_get"))
    {
        return File_StopAfterFilled_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_stopsubstreamafterfilled"))
    {
        File_StopSubStreamAfterFilled_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_stopsubstreamafterfilled_get"))
    {
        return File_StopSubStreamAfterFilled_Get()?"1":"0";
    }
    if (Option_Lower==__T("file_audio_mergemonostreams"))
    {
        File_Audio_MergeMonoStreams_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_audio_mergemonostreams_get"))
    {
        return File_Audio_MergeMonoStreams_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_demux_interleave"))
    {
        File_Demux_Interleave_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_demux_interleave_get"))
    {
        return File_Demux_Interleave_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_id_onlyroot"))
    {
        File_ID_OnlyRoot_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_id_onlyroot_get"))
    {
        return File_ID_OnlyRoot_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_ignoresequencefilescount"))
    {
        #if MEDIAINFO_MD5
            File_IgnoreSequenceFilesCount_Set(!(Value==__T("0") || Value.empty()));
            return Ztring();
        #else //MEDIAINFO_MD5
            return __T("Disabled due to compilation options");
        #endif //MEDIAINFO_MD5
    }
    else if (Option_Lower==__T("file_defaultframerate"))
    {
        #if MEDIAINFO_MD5
            File_DefaultFrameRate_Set(Ztring(Value).To_float64());
            return Ztring();
        #else //MEDIAINFO_MD5
            return __T("File_DefaultFrameRate is disabled due to compilation options");
        #endif //MEDIAINFO_MD5
    }
    else if (Option_Lower==__T("file_source_list"))
    {
        #if MEDIAINFO_MD5
            File_Source_List_Set(!(Value==__T("0") || Value.empty()));
            return Ztring();
        #else //MEDIAINFO_MD5
            return __T("MD5 is disabled due to compilation options");
        #endif //MEDIAINFO_MD5
    }
    else if (Option_Lower==__T("file_riskybitrateestimation"))
    {
        #if MEDIAINFO_ADVANCED
            File_RiskyBitRateEstimation_Set(!(Value==__T("0") || Value.empty()));
            return Ztring();
        #else //MEDIAINFO_ADVANCED
            return __T("Advanced features are disabled due to compilation options");
        #endif //MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("file_demux_unpacketize_streamlayoutchange_skip"))
    {
        #if MEDIAINFO_DEMUX
            #if MEDIAINFO_ADVANCED
                File_Demux_Unpacketize_StreamLayoutChange_Skip_Set(!(Value==__T("0") || Value.empty()));
                return Ztring();
            #else //MEDIAINFO_ADVANCED
                return __T("Advanced features disabled due to compilation options");
            #endif //MEDIAINFO_ADVANCED
        #else //MEDIAINFO_ADVANCED
            return __T("Advanced features disabled due to compilation options");
        #endif //MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("file_md5"))
    {
        #if MEDIAINFO_MD5
            File_Md5_Set(!(Value==__T("0") || Value.empty()));
            return Ztring();
        #else //MEDIAINFO_MD5
            return __T("MD5 is disabled due to compilation options");
        #endif //MEDIAINFO_MD5
    }
    else if (Option_Lower==__T("file_checksidecarfiles"))
    {
        #if defined(MEDIAINFO_REFERENCES_YES)
            File_CheckSideCarFiles_Set(!(Value==__T("0") || Value.empty()));
            return Ztring();
        #else //defined(MEDIAINFO_REFERENCES_YES)
            return __T("Disabled due to compilation options");
        #endif //defined(MEDIAINFO_REFERENCES_YES)
    }
    else if (Option_Lower==__T("file_filename"))
    {
        File_FileName_Set(Value);
        return __T("");
    }
    else if (Option_Lower==__T("file_filename_get"))
    {
        return File_FileName_Get();
    }
    else if (Option_Lower==__T("file_filenameformat"))
    {
        File_FileNameFormat_Set(Value);
        return __T("");
    }
    else if (Option_Lower==__T("file_filenameformat_get"))
    {
        return File_FileNameFormat_Get();
    }
    else if (Option_Lower==__T("file_timetolive"))
    {
        File_TimeToLive_Set(Ztring(Value).To_float64());
        return __T("");
    }
    else if (Option_Lower==__T("file_timetolive_get"))
    {
        return Ztring::ToZtring(File_TimeToLive_Get(), 9);
    }
    else if (Option_Lower==__T("file_partial_begin"))
    {
        File_Partial_Begin_Set(Value);
        return __T("");
    }
    else if (Option_Lower==__T("file_partial_begin_get"))
    {
        return File_Partial_Begin_Get();
    }
    else if (Option_Lower==__T("file_partial_end"))
    {
        File_Partial_End_Set(Value);
        return __T("");
    }
    else if (Option_Lower==__T("file_partial_end_get"))
    {
        return File_Partial_End_Get();
    }
    else if (Option_Lower==__T("file_forceparser"))
    {
        File_ForceParser_Set(Value);
        return __T("");
    }
    else if (Option_Lower==__T("file_forceparser_get"))
    {
        return File_ForceParser_Get();
    }
    else if (Option_Lower==__T("file_buffer_size_hint_pointer"))
    {
        File_Buffer_Size_Hint_Pointer_Set((size_t*)Ztring(Value).To_int64u());
        return __T("");
    }
    else if (Option_Lower==__T("file_buffer_size_hint_pointer_get"))
    {
        return Ztring::ToZtring((size_t)File_Buffer_Size_Hint_Pointer_Get());
    }
    else if (Option_Lower==__T("file_filter"))
    {
        #if MEDIAINFO_FILTER
            Ztring ValueLowerCase=Ztring(Value).MakeLowerCase();
            if (ValueLowerCase==__T("audio"))
                File_Filter_Audio_Set(true);
            else
                File_Filter_Set(ValueLowerCase.To_int64u());
            return Ztring();
        #else //MEDIAINFO_FILTER
            return __T("Filter manager is disabled due to compilation options");
        #endif //MEDIAINFO_FILTER
    }
    else if (Option_Lower==__T("file_filter_get"))
    {
        #if MEDIAINFO_FILTER
            return Ztring();//.From_Number(File_Filter_Get());
        #else //MEDIAINFO_FILTER
            return __T("Filter manager is disabled due to compilation options");
        #endif //MEDIAINFO_FILTER
    }
    else if (Option_Lower==__T("file_duplicate"))
    {
        #if MEDIAINFO_DUPLICATE
            return File_Duplicate_Set(Value);
        #else //MEDIAINFO_DUPLICATE
            return __T("Duplicate manager is disabled due to compilation options");
        #endif //MEDIAINFO_DUPLICATE
    }
    else if (Option_Lower==__T("file_duplicate_get"))
    {
        #if MEDIAINFO_DUPLICATE
            //if (File_Duplicate_Get())
                return __T("1");
            //else
            //    return __T("");
        #else //MEDIAINFO_DUPLICATE
            return __T("Duplicate manager is disabled due to compilation options");
        #endif //MEDIAINFO_DUPLICATE
    }
    else if (Option_Lower==__T("file_demux_forceids"))
    {
        #if MEDIAINFO_DEMUX
            if (Value.empty())
                Demux_ForceIds_Set(false);
            else
                Demux_ForceIds_Set(true);
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_pcm_20bitto16bit"))
    {
        #if MEDIAINFO_DEMUX
            if (Value.empty())
                Demux_PCM_20bitTo16bit_Set(false);
            else
                Demux_PCM_20bitTo16bit_Set(true);
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_pcm_20bitto24bit"))
    {
        #if MEDIAINFO_DEMUX
            if (Value.empty())
                Demux_PCM_20bitTo24bit_Set(false);
            else
                Demux_PCM_20bitTo24bit_Set(true);
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_avc_transcode_iso14496_15_to_iso14496_10"))
    {
        #if MEDIAINFO_DEMUX
            if (Value.empty())
                Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Set(false);
            else
                Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Set(true);
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_unpacketize"))
    {
        #if MEDIAINFO_DEMUX
            if (Value.empty())
                Demux_Unpacketize_Set(false);
            else
                Demux_Unpacketize_Set(true);
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_rate"))
    {
        #if MEDIAINFO_DEMUX
            Demux_Rate_Set(Ztring(Value).To_float64());
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_firstdts"))
    {
        #if MEDIAINFO_DEMUX
            int64u ValueInt64u;
            if (Value.find(__T(":"))!=string::npos)
            {
                Ztring ValueZ=Value;
                ValueInt64u=0;
                size_t Value_Pos=ValueZ.find(__T(":"));
                if (Value_Pos==string::npos)
                    Value_Pos=ValueZ.size();
                ValueInt64u+=Ztring(ValueZ.substr(0, Value_Pos)).To_int64u()*60*60*1000*1000*1000;
                ValueZ.erase(0, Value_Pos+1);
                Value_Pos=ValueZ.find(__T(":"));
                if (Value_Pos==string::npos)
                    Value_Pos=ValueZ.size();
                ValueInt64u+=Ztring(ValueZ.substr(0, Value_Pos)).To_int64u()*60*1000*1000*1000;
                ValueZ.erase(0, Value_Pos+1);
                Value_Pos=ValueZ.find(__T("."));
                if (Value_Pos==string::npos)
                    Value_Pos=ValueZ.size();
                ValueInt64u+=Ztring(ValueZ.substr(0, Value_Pos)).To_int64u()*1000*1000*1000;
                ValueZ.erase(0, Value_Pos+1);
                if (!ValueZ.empty())
                    ValueInt64u+=Ztring(ValueZ).To_int64u()*1000*1000*1000/(int64u)pow(10.0, (int)ValueZ.size());
            }
            else
                ValueInt64u=Ztring(Value).To_int64u();
            Demux_FirstDts_Set(ValueInt64u);
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_firstframenumber"))
    {
        #if MEDIAINFO_DEMUX
            Demux_FirstFrameNumber_Set(Ztring(Value).To_int64u());
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_demux_initdata"))
    {
        #if MEDIAINFO_DEMUX
            Ztring Value_Lower(Value); Value_Lower.MakeLowerCase();
                 if (Value_Lower==__T("event")) Demux_InitData_Set(0);
            else if (Value_Lower==__T("field")) Demux_InitData_Set(1);
            else return __T("Invalid value");
            return Ztring();
        #else //MEDIAINFO_DEMUX
            return __T("Demux manager is disabled due to compilation options");
        #endif //MEDIAINFO_DEMUX
    }
    else if (Option_Lower==__T("file_ibi"))
    {
        #if MEDIAINFO_IBI
            Ibi_Set(Value);
            return Ztring();
        #else //MEDIAINFO_IBI
            return __T("IBI support is disabled due to compilation options");
        #endif //MEDIAINFO_IBI
    }
    else if (Option_Lower==__T("file_ibi_create"))
    {
        #if MEDIAINFO_IBI
            if (Value.empty())
                Ibi_Create_Set(false);
            else
                Ibi_Create_Set(true);
            return Ztring();
        #else //MEDIAINFO_IBI
            return __T("IBI support is disabled due to compilation options");
        #endif //MEDIAINFO_IBI
    }
    else if (Option_Lower==__T("file_ibi_useibiinfoifavailable"))
    {
        #if MEDIAINFO_IBI
            if (Value.empty())
                Ibi_UseIbiInfoIfAvailable_Set(false);
            else
                Ibi_UseIbiInfoIfAvailable_Set(true);
            return Ztring();
        #else //MEDIAINFO_IBI
            return __T("IBI support is disabled due to compilation options");
        #endif //MEDIAINFO_IBI
    }
    else if (Option_Lower==__T("file_nextpacket"))
    {
        #if MEDIAINFO_NEXTPACKET
            if (Value.empty())
                NextPacket_Set(false);
            else
                NextPacket_Set(true);
            return Ztring();
        #else //MEDIAINFO_NEXTPACKET
            return __T("NextPacket manager is disabled due to compilation options");
        #endif //MEDIAINFO_NEXTPACKET
    }
    else if (Option_Lower==__T("file_subfile_streamid_set"))
    {
        #if MEDIAINFO_EVENTS
            SubFile_StreamID_Set(Value.empty()?(int64u)-1:Ztring(Value).To_int64u());
            return Ztring();
        #else //MEDIAINFO_EVENTS
            return __T("Event manager is disabled due to compilation options");
        #endif //MEDIAINFO_EVENTS
    }
    else if (Option_Lower==__T("file_subfile_ids_set"))
    {
        #if MEDIAINFO_EVENTS
            SubFile_IDs_Set(Value);
            return Ztring();
        #else //MEDIAINFO_EVENTS
            return __T("Event manager is disabled due to compilation options");
        #endif //MEDIAINFO_EVENTS
    }
    else if (Option_Lower==__T("file_parseundecodableframes"))
    {
        #if MEDIAINFO_EVENTS
            ParseUndecodableFrames_Set(!(Value==__T("0") || Value.empty()));
            return Ztring();
        #else //MEDIAINFO_EVENTS
            return __T("Event manager is disabled due to compilation options");
        #endif //MEDIAINFO_EVENTS
    }
    else if (Option_Lower==__T("file_mpegts_forcemenu"))
    {
        File_MpegTs_ForceMenu_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_mpegts_forcemenu_get"))
    {
        return File_MpegTs_ForceMenu_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_mpegts_stream_type_trust"))
    {
        File_MpegTs_stream_type_Trust_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_mpegts_stream_type_trust_get"))
    {
        return File_MpegTs_stream_type_Trust_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_mpegts_atsc_transport_stream_id_trust"))
    {
        File_MpegTs_Atsc_transport_stream_id_Trust_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_mpegts_atsc_transport_stream_id_trust_get"))
    {
        return File_MpegTs_Atsc_transport_stream_id_Trust_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_mpegts_realtime"))
    {
        File_MpegTs_RealTime_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_mpegts_realtime_get"))
    {
        return File_MpegTs_RealTime_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_bdmv_parsetargetedfile"))
    {
        File_Bdmv_ParseTargetedFile_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_bdmv_parsetargetedfile_get"))
    {
        return File_Bdmv_ParseTargetedFile_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_dvdif_disableaudioifisincontainer"))
    {
        #if defined(MEDIAINFO_DVDIF_YES)
            File_DvDif_DisableAudioIfIsInContainer_Set(!(Value==__T("0") || Value.empty()));
            return __T("");
        #else //defined(MEDIAINFO_DVDIF_YES)
            return __T("DVDIF is disabled due to compilation options");
        #endif //defined(MEDIAINFO_DVDIF_YES)
    }
    else if (Option_Lower==__T("file_dvdif_disableaudioifisincontainer_get"))
    {
        #if defined(MEDIAINFO_DVDIF_YES)
            return File_DvDif_DisableAudioIfIsInContainer_Get()?"1":"0";
        #else //defined(MEDIAINFO_DVDIF_YES)
            return __T("DVDIF is disabled due to compilation options");
        #endif //defined(MEDIAINFO_DVDIF_YES)
    }
    else if (Option_Lower==__T("file_dvdif_ignoretransmittingflags"))
    {
        #if defined(MEDIAINFO_DVDIF_YES)
            File_DvDif_IgnoreTransmittingFlags_Set(!(Value==__T("0") || Value.empty()));
            return __T("");
        #else //defined(MEDIAINFO_DVDIF_YES)
            return __T("DVDIF is disabled due to compilation options");
        #endif //defined(MEDIAINFO_DVDIF_YES)
    }
    else if (Option_Lower==__T("file_dvdif_ignoretransmittingflags_get"))
    {
        #if defined(MEDIAINFO_DVDIF_YES)
            return File_DvDif_IgnoreTransmittingFlags_Get()?"1":"0";
        #else //defined(MEDIAINFO_DVDIF_YES)
            return __T("DVDIF is disabled due to compilation options");
        #endif //defined(MEDIAINFO_DVDIF_YES)
    }
    else if (Option_Lower==__T("file_dvdif_analysis"))
    {
        #if defined(MEDIAINFO_DVDIF_ANALYZE_YES)
            File_DvDif_Analysis_Set(!(Value==__T("0") || Value.empty()));
            return __T("");
        #else //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
            return __T("DVDIF Analysis is disabled due to compilation options");
        #endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    }
    else if (Option_Lower==__T("file_dvdif_analysis_get"))
    {
        #if defined(MEDIAINFO_DVDIF_ANALYZE_YES)
            return File_DvDif_Analysis_Get()?"1":"0";
        #else //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
            return __T("DVDIF Analysis is disabled due to compilation options");
        #endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    }
    else if (Option_Lower==__T("file_macroblocks_parse"))
    {
        #if MEDIAINFO_MACROBLOCKS
            File_Macroblocks_Parse_Set(!(Value==__T("0") || Value.empty()));
            return __T("");
        #else //MEDIAINFO_MACROBLOCKS
            return __T("Macroblock parsing is disabled due to compilation options");
        #endif //MEDIAINFO_MACROBLOCKS
    }
    else if (Option_Lower==__T("file_macroblocks_parse_get"))
    {
        #if MEDIAINFO_MACROBLOCKS
            return File_Macroblocks_Parse_Get()?"1":"0";
        #else //MEDIAINFO_MACROBLOCKS
            return __T("Macroblock parsing is disabled due to compilation options");
        #endif //MEDIAINFO_MACROBLOCKS
    }
    else if (Option_Lower==__T("file_growingfile_delay"))
    {
        File_GrowingFile_Delay_Set(Ztring(Value).To_float64());
        return Ztring();
    }
    else if (Option_Lower==__T("file_growingfile_delay_get"))
    {
        return Ztring::ToZtring(File_GrowingFile_Delay_Get());
    }
    else if (Option_Lower==__T("file_curl"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            File_Curl_Set(Value);
            return __T("");
        #else //defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower.find(__T("file_curl,"))==0 || Option_Lower.find(__T("file_curl;"))==0)
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            File_Curl_Set(Option.substr(10), Value);
            return __T("");
        #else //defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("file_curl_get"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            return File_Curl_Get(Value);
        #else //defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("file_mmsh_describe_only"))
    {
        #if defined(MEDIAINFO_LIBMMS_YES)
            File_Mmsh_Describe_Only_Set(!(Value==__T("0") || Value.empty()));
            return __T("");
        #else //defined(MEDIAINFO_LIBMMS_YES)
            return __T("Libmms support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_LIBMMS_YES)
    }
    else if (Option_Lower==__T("file_mmsh_describe_only_get"))
    {
        #if defined(MEDIAINFO_LIBMMS_YES)
            return File_Mmsh_Describe_Only_Get()?"1":"0";
        #else //defined(MEDIAINFO_LIBMMS_YES)
            return __T("Libmms support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_LIBMMS_YES)
    }
    else if (Option_Lower==__T("file_eia708_displayemptystream"))
    {
        File_Eia708_DisplayEmptyStream_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_eia708_displayemptystream_get"))
    {
        return File_Eia708_DisplayEmptyStream_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_eia608_displayemptystream"))
    {
        File_Eia608_DisplayEmptyStream_Set(!(Value==__T("0") || Value.empty()));
        return __T("");
    }
    else if (Option_Lower==__T("file_eia608_displayemptystream_get"))
    {
        return File_Eia608_DisplayEmptyStream_Get()?"1":"0";
    }
    else if (Option_Lower==__T("file_ac3_ignorecrc"))
    {
        #if defined(MEDIAINFO_AC3_YES)
            File_Ac3_IgnoreCrc_Set(!(Value==__T("0") || Value.empty()));
            return __T("");
        #else //defined(MEDIAINFO_AC3_YES)
            return __T("AC-3 support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_AC3_YES)
    }
    else if (Option_Lower==__T("file_ac3_ignorecrc_get"))
    {
        #if defined(MEDIAINFO_AC3_YES)
            return File_Ac3_IgnoreCrc_Get()?"1":"0";
        #else //defined(MEDIAINFO_AC3_YES)
            return __T("AC-3 support is disabled due to compilation options");
        #endif //defined(MEDIAINFO_AC3_YES)
    }
    else if (Option_Lower==__T("file_event_callbackfunction"))
    {
        #if MEDIAINFO_EVENTS
            return Event_CallBackFunction_Set(Value);
        #else //MEDIAINFO_EVENTS
            return __T("Event manager is disabled due to compilation options");
        #endif //MEDIAINFO_EVENTS
    }
    else
        return __T("Option not known");
}

//***************************************************************************
// File Is Seekable
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_IsSeekable_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileIsSeekable=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_IsSeekable_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileIsSeekable;
}

//***************************************************************************
// File Is Sub
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_IsSub_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileIsSub=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_IsSub_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileIsSub;
}

//***************************************************************************
// File Is Detecting Duration
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_IsDetectingDuration_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileIsDetectingDuration=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_IsDetectingDuration_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileIsDetectingDuration;
}

//***************************************************************************
// File Is Referenced
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_IsReferenced_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileIsReferenced=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_IsReferenced_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileIsReferenced;
}

//***************************************************************************
// File Keep Info
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_KeepInfo_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileKeepInfo=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_KeepInfo_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileKeepInfo;
}

//***************************************************************************
// File test continuous file names
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_TestContinuousFileNames_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileTestContinuousFileNames=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_TestContinuousFileNames_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileTestContinuousFileNames;
}

//***************************************************************************
// Stop after filled
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_StopAfterFilled_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileStopAfterFilled=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_StopAfterFilled_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileStopAfterFilled;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_StopSubStreamAfterFilled_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    FileStopSubStreamAfterFilled=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_StopSubStreamAfterFilled_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FileStopSubStreamAfterFilled;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Audio_MergeMonoStreams_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Audio_MergeMonoStreams=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Audio_MergeMonoStreams_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Audio_MergeMonoStreams;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Demux_Interleave_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Demux_Interleave=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Demux_Interleave_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Demux_Interleave;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_ID_OnlyRoot_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_ID_OnlyRoot=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_ID_OnlyRoot_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_ID_OnlyRoot;
}

//---------------------------------------------------------------------------
#if MEDIAINFO_MD5
void MediaInfo_Config_MediaInfo::File_Md5_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Md5=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Md5_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Md5;
}
#endif //MEDIAINFO_MD5

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_REFERENCES_YES)
void MediaInfo_Config_MediaInfo::File_CheckSideCarFiles_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_CheckSideCarFiles=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_CheckSideCarFiles_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_CheckSideCarFiles;
}
#endif //defined(MEDIAINFO_REFERENCES_YES)

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED
void MediaInfo_Config_MediaInfo::File_IgnoreSequenceFileSize_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_IgnoreSequenceFileSize=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_IgnoreSequenceFileSize_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_IgnoreSequenceFileSize;
}
#endif //MEDIAINFO_ADVANCED

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED
void MediaInfo_Config_MediaInfo::File_IgnoreSequenceFilesCount_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_IgnoreSequenceFilesCount=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_IgnoreSequenceFilesCount_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_IgnoreSequenceFilesCount;
}
#endif //MEDIAINFO_ADVANCED

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED
void MediaInfo_Config_MediaInfo::File_DefaultFrameRate_Set (float64 NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_DefaultFrameRate=NewValue;
    #if MEDIAINFO_DEMUX
        Demux_Rate=File_DefaultFrameRate;
    #endif //MEDIAINFO_DEMUX
}

float64 MediaInfo_Config_MediaInfo::File_DefaultFrameRate_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_DefaultFrameRate;
}
#endif //MEDIAINFO_ADVANCED

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED
void MediaInfo_Config_MediaInfo::File_Source_List_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Source_List=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Source_List_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Source_List;
}
#endif //MEDIAINFO_ADVANCED

//---------------------------------------------------------------------------
#if MEDIAINFO_ADVANCED
void MediaInfo_Config_MediaInfo::File_RiskyBitRateEstimation_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_RiskyBitRateEstimation=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_RiskyBitRateEstimation_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_RiskyBitRateEstimation;
}
#endif //MEDIAINFO_ADVANCED

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
#if MEDIAINFO_ADVANCED
void MediaInfo_Config_MediaInfo::File_Demux_Unpacketize_StreamLayoutChange_Skip_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Demux_Unpacketize_StreamLayoutChange_Skip=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Demux_Unpacketize_StreamLayoutChange_Skip_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Demux_Unpacketize_StreamLayoutChange_Skip;
}
#endif //MEDIAINFO_ADVANCED
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// File name from somewhere else
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_FileName_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_FileName=NewValue;
}

Ztring MediaInfo_Config_MediaInfo::File_FileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_FileName;
}

//***************************************************************************
// File name format
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_FileNameFormat_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_FileNameFormat=NewValue;
}

Ztring MediaInfo_Config_MediaInfo::File_FileNameFormat_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_FileNameFormat;
}

//***************************************************************************
// Time to live
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_TimeToLive_Set (float64 NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_TimeToLive=NewValue;
}

float64 MediaInfo_Config_MediaInfo::File_TimeToLive_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_TimeToLive;
}

//***************************************************************************
// Partial file (begin and end are cut)
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Partial_Begin_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Partial_Begin=NewValue;
}

Ztring MediaInfo_Config_MediaInfo::File_Partial_Begin_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Partial_Begin;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Partial_End_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Partial_End=NewValue;
}

Ztring MediaInfo_Config_MediaInfo::File_Partial_End_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Partial_End;
}

//***************************************************************************
// Force Parser
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_ForceParser_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_ForceParser=NewValue;
}

Ztring MediaInfo_Config_MediaInfo::File_ForceParser_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_ForceParser;
}

//***************************************************************************
/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Buffer_Size_Hint_Pointer_Set (size_t* NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Buffer_Size_Hint_Pointer=NewValue;
}

size_t*  MediaInfo_Config_MediaInfo::File_Buffer_Size_Hint_Pointer_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Buffer_Size_Hint_Pointer;
}

//***************************************************************************
// Filter
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_FILTER
void MediaInfo_Config_MediaInfo::File_Filter_Set (int64u NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Filter_16[(int16u)NewValue]=true;
    File_Filter_HasChanged_=true;
}

bool MediaInfo_Config_MediaInfo::File_Filter_Get (const int16u Value)
{
    CriticalSectionLocker CSL(CS);
    //Test
    bool Exists;
    if (File_Filter_16.empty())
        Exists=true;
    else
        Exists=(File_Filter_16.find(Value)!=File_Filter_16.end());
    return Exists;
}

bool MediaInfo_Config_MediaInfo::File_Filter_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Exist=!File_Filter_16.empty();
    return Exist;
}

void MediaInfo_Config_MediaInfo::File_Filter_Audio_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Filter_Audio=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Filter_Audio_Get ()
{
    CriticalSectionLocker CSL(CS);
    return File_Filter_Audio;
}

bool MediaInfo_Config_MediaInfo::File_Filter_HasChanged ()
{
    CriticalSectionLocker CSL(CS);
    bool File_Filter_HasChanged_Temp=File_Filter_HasChanged_;
    File_Filter_HasChanged_=false;
    return File_Filter_HasChanged_Temp;
}
#endif //MEDIAINFO_FILTER

//***************************************************************************
// Duplicate
//***************************************************************************

#if MEDIAINFO_DUPLICATE
Ztring MediaInfo_Config_MediaInfo::File_Duplicate_Set (const Ztring &Value_In)
{
    //Preparing for File__Duplicate...
    CS.Enter();
    File__Duplicate_List.push_back(Value_In);

    //Handling Memory index
    Ztring ToReturn;
    ZtringList List=Value_In;
    for (size_t Pos=0; Pos<List.size(); Pos++)
    {
        //Form= "(-)Data", if "-" the value will be removed
        Ztring &Value=List[Pos];
        bool ToRemove=false;
        if (Value.find(__T('-'))==0)
        {
            Value.erase(Value.begin());
            ToRemove=true;
        }

        //Testing if this is information about a target
        if (List[Pos].find(__T("memory:"))==0 || List[Pos].find(__T("file:"))==0)
        {
            //Searching if already exist
            size_t Memory_Pos=File__Duplicate_Memory_Indexes.Find(List[Pos]);
            if (!ToRemove && Memory_Pos==Error)
            {
                //Does not exist yet (and adding is wanted)
                Memory_Pos=File__Duplicate_Memory_Indexes.Find(__T(""));
                if (Memory_Pos!=Error)
                    File__Duplicate_Memory_Indexes[Memory_Pos]=List[Pos]; //A free place is found
                else
                {
                    //Adding the place at the end
                    Memory_Pos=File__Duplicate_Memory_Indexes.size();
                    File__Duplicate_Memory_Indexes.push_back(List[Pos]);
                }
            }
            else if (ToRemove)
            {
                //Exists yet but Removal is wanted
                File__Duplicate_Memory_Indexes[Memory_Pos].clear();
                Memory_Pos=(size_t)-1;
            }

            ToReturn+=__T(";")+Ztring().From_Number(Memory_Pos);
        }
    }
    if (!ToReturn.empty())
        ToReturn.erase(ToReturn.begin()); //Remove first ";"

    CS.Leave();
    File_IsSeekable_Set(false); //If duplication, we can not seek anymore

    return ToReturn;
}

Ztring MediaInfo_Config_MediaInfo::File_Duplicate_Get (size_t AlreadyRead_Pos)
{
    CriticalSectionLocker CSL(CS);
    if (AlreadyRead_Pos>=File__Duplicate_List.size())
        return Ztring(); //Nothing or not more than the last time
    Ztring Temp=File__Duplicate_List[AlreadyRead_Pos];
    return Temp;
}

bool MediaInfo_Config_MediaInfo::File_Duplicate_Get_AlwaysNeeded (size_t AlreadyRead_Pos)
{
    CriticalSectionLocker CSL(CS);
    bool Temp=AlreadyRead_Pos>=File__Duplicate_List.size();
    return !Temp; //True if there is something to read
}

size_t MediaInfo_Config_MediaInfo::File__Duplicate_Memory_Indexes_Get (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    return File__Duplicate_Memory_Indexes.Find(Value);
}

void MediaInfo_Config_MediaInfo::File__Duplicate_Memory_Indexes_Erase (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    size_t Pos=File__Duplicate_Memory_Indexes.Find(Value);
    if (Pos!=Error)
        File__Duplicate_Memory_Indexes[Pos].clear();
}
#endif //MEDIAINFO_DUPLICATE

//***************************************************************************
// Demux
//***************************************************************************

#if MEDIAINFO_DEMUX
//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Demux_ForceIds_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_ForceIds=NewValue;
}

bool MediaInfo_Config_MediaInfo::Demux_ForceIds_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_ForceIds;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Demux_PCM_20bitTo16bit_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_PCM_20bitTo16bit=NewValue;
}

bool MediaInfo_Config_MediaInfo::Demux_PCM_20bitTo16bit_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_PCM_20bitTo16bit;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Demux_PCM_20bitTo24bit_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_PCM_20bitTo24bit=NewValue;
}

bool MediaInfo_Config_MediaInfo::Demux_PCM_20bitTo24bit_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_PCM_20bitTo24bit;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10=NewValue;
}

bool MediaInfo_Config_MediaInfo::Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Demux_Unpacketize_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_Unpacketize=NewValue;
}

bool MediaInfo_Config_MediaInfo::Demux_Unpacketize_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_Unpacketize;
}
#endif //MEDIAINFO_DEMUX

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
void MediaInfo_Config_MediaInfo::Demux_Rate_Set (float64 NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_Rate=NewValue;
}

float64 MediaInfo_Config_MediaInfo::Demux_Rate_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_Rate;
}
#endif //MEDIAINFO_DEMUX

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
void MediaInfo_Config_MediaInfo::Demux_FirstDts_Set (int64u NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_FirstDts=NewValue;
}

int64u MediaInfo_Config_MediaInfo::Demux_FirstDts_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_FirstDts;
}
#endif //MEDIAINFO_DEMUX

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
void MediaInfo_Config_MediaInfo::Demux_FirstFrameNumber_Set (int64u NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_FirstFrameNumber=NewValue;
}

int64u MediaInfo_Config_MediaInfo::Demux_FirstFrameNumber_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_FirstFrameNumber;
}
#endif //MEDIAINFO_DEMUX

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
void MediaInfo_Config_MediaInfo::Demux_InitData_Set (int8u NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux_InitData=NewValue;
}

int8u MediaInfo_Config_MediaInfo::Demux_InitData_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux_InitData;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// IBI support
//***************************************************************************

#if MEDIAINFO_IBI
//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Ibi_Set (const Ztring &Value)
{
    string Data_Base64=Value.To_UTF8();

    CriticalSectionLocker CSL(CS);
    Ibi=Base64::decode(Data_Base64);
}

string MediaInfo_Config_MediaInfo::Ibi_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ibi;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Ibi_Create_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Ibi_Create=NewValue;
}

bool MediaInfo_Config_MediaInfo::Ibi_Create_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ibi_Create;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Ibi_UseIbiInfoIfAvailable_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    Ibi_UseIbiInfoIfAvailable=NewValue;
}

bool MediaInfo_Config_MediaInfo::Ibi_UseIbiInfoIfAvailable_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ibi_UseIbiInfoIfAvailable;
}
#endif //MEDIAINFO_IBI

//***************************************************************************
// NextPacket
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_NEXTPACKET
void MediaInfo_Config_MediaInfo::NextPacket_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    NextPacket=NewValue;
}

bool MediaInfo_Config_MediaInfo::NextPacket_Get ()
{
    CriticalSectionLocker CSL(CS);
    return NextPacket;
}
#endif //MEDIAINFO_NEXTPACKET

//***************************************************************************
// SubFile
//***************************************************************************

#if MEDIAINFO_EVENTS
//---------------------------------------------------------------------------
ZtringListList MediaInfo_Config_MediaInfo::SubFile_Config_Get ()
{
    CriticalSectionLocker CSL(CS);

    return SubFile_Config;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::SubFile_StreamID_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);

    SubFile_StreamID=Value;
}

//---------------------------------------------------------------------------
int64u MediaInfo_Config_MediaInfo::SubFile_StreamID_Get ()
{
    CriticalSectionLocker CSL(CS);

    return SubFile_StreamID;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::SubFile_IDs_Set (Ztring Value)
{
    CriticalSectionLocker CSL(CS);

    SubFile_IDs=Value;
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Config_MediaInfo::SubFile_IDs_Get ()
{
    CriticalSectionLocker CSL(CS);

    return SubFile_IDs;
}
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// SubFile
//***************************************************************************

#if MEDIAINFO_EVENTS
//---------------------------------------------------------------------------
bool MediaInfo_Config_MediaInfo::ParseUndecodableFrames_Get ()
{
    CriticalSectionLocker CSL(CS);

    return ParseUndecodableFrames;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::ParseUndecodableFrames_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);

    ParseUndecodableFrames=Value;
}
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// Event
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
bool MediaInfo_Config_MediaInfo::Event_CallBackFunction_IsSet ()
{
    CriticalSectionLocker CSL(CS);

    return Event_CallBackFunction?true:false;
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
Ztring MediaInfo_Config_MediaInfo::Event_CallBackFunction_Set (const Ztring &Value)
{
    ZtringList List=Value;

    CriticalSectionLocker CSL(CS);

    if (List.empty())
    {
        Event_CallBackFunction=(MediaInfo_Event_CallBackFunction*)NULL;
        Event_UserHandler=NULL;
    }
    else
        for (size_t Pos=0; Pos<List.size(); Pos++)
        {
            if (List[Pos].find(__T("CallBack=memory://"))==0)
                Event_CallBackFunction=(MediaInfo_Event_CallBackFunction*)Ztring(List[Pos].substr(18, std::string::npos)).To_int64u();
            else if (List[Pos].find(__T("UserHandle=memory://"))==0)
                Event_UserHandler=(void*)Ztring(List[Pos].substr(20, std::string::npos)).To_int64u();
            else if (List[Pos].find(__T("UserHandler=memory://"))==0)
                Event_UserHandler=(void*)Ztring(List[Pos].substr(21, std::string::npos)).To_int64u();
            else
                return("Problem during Event_CallBackFunction value parsing");
        }

    return Ztring();
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
Ztring MediaInfo_Config_MediaInfo::Event_CallBackFunction_Get ()
{
    CriticalSectionLocker CSL(CS);

    return __T("CallBack=memory://")+Ztring::ToZtring((size_t)Event_CallBackFunction)+__T(";UserHandler=memory://")+Ztring::ToZtring((size_t)Event_UserHandler);
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void MediaInfo_Config_MediaInfo::Event_Send (File__Analyze* Source, const int8u* Data_Content, size_t Data_Size, const Ztring &File_Name)
{
    CriticalSectionLocker CSL(CS);

    if (Source==NULL)
    {
        MediaInfo_Event_Generic* Temp=(MediaInfo_Event_Generic*)Data_Content;

        if (Demux_Offset_Frame!=(int64u)-1)
        {
            if (Temp->FrameNumber!=(int64u)-1)
                Temp->FrameNumber+=Demux_Offset_Frame;
            if (Temp->FrameNumber_PresentationOrder!=(int64u)-1)
                Temp->FrameNumber_PresentationOrder+=Demux_Offset_Frame;
        }
        if (Demux_Offset_DTS!=(int64u)-1)
        {
            if (Temp->DTS!=(int64u)-1)
                Temp->DTS+=Demux_Offset_DTS;
            if (Temp->PTS!=(int64u)-1)
                Temp->PTS+=Demux_Offset_DTS;
            if (Demux_Offset_DTS_FromStream!=(int64u)-1)
            {
                if (Temp->DTS!=(int64u)-1)
                    Temp->DTS-=Demux_Offset_DTS_FromStream;
                if (Temp->PTS!=(int64u)-1)
                    Temp->PTS-=Demux_Offset_DTS_FromStream;
            }
        }
        if (File_IgnoreFramesBefore)
        {
            if (Temp->FrameNumber!=(int64u)-1)
            {
                if (Temp->FrameNumber>File_IgnoreFramesBefore)
                    Temp->FrameNumber-=File_IgnoreFramesBefore;
                else
                    Temp->FrameNumber=0;
            }
            if (Temp->DTS!=(int64u)-1)
            {
                if (File_IgnoreFramesBefore && File_IgnoreFramesRate)
                {
                    int64u TimeOffset=float64_int64s(((float64)File_IgnoreFramesBefore)/File_IgnoreFramesRate*1000000000);
                    if (Temp->DTS>TimeOffset)
                        Temp->DTS-=TimeOffset;
                    else
                        Temp->DTS=0;
                }
            }
            if (Temp->PTS!=(int64u)-1)
            {
                if (File_IgnoreFramesBefore && File_IgnoreFramesRate)
                {
                    int64u TimeOffset=float64_int64s(((float64)File_IgnoreFramesBefore)/File_IgnoreFramesRate*1000000000);
                    if (Temp->PTS>TimeOffset)
                        Temp->PTS-=TimeOffset;
                    else
                        Temp->PTS=0;
                }
            }
        }
    }

    if (Source)
    {
        event_delayed* Event=new event_delayed(Data_Content, Data_Size, File_Name);
        Events_Delayed[Source].push_back(Event);

        // Copying buffers
        int32u* EventCode=(int32u*)Data_Content;
        if (((*EventCode)&0x00FFFFFF)==((MediaInfo_Event_Global_Demux<<8)|4) && Data_Size==sizeof(MediaInfo_Event_Global_Demux_4))
        {
            MediaInfo_Event_Global_Demux_4* Old=(MediaInfo_Event_Global_Demux_4*)Data_Content;
            MediaInfo_Event_Global_Demux_4* New=(MediaInfo_Event_Global_Demux_4*)Event->Data_Content;
            if (New->Content_Size)
            {
                int8u* Content=new int8u[New->Content_Size];
                std::memcpy(Content, Old->Content, New->Content_Size*sizeof(int8u));
                New->Content=Content;
            }
            if (New->Offsets_Size)
            {
                int64u* Offsets_Stream=new int64u[New->Offsets_Size];
                std::memcpy(Offsets_Stream, Old->Offsets_Stream, New->Offsets_Size*sizeof(int64u));
                New->Offsets_Stream=Offsets_Stream;
                int64u* Offsets_Content=new int64u[New->Offsets_Size];
                std::memcpy(Offsets_Content, Old->Offsets_Content, New->Offsets_Size*sizeof(int64u));
                New->Offsets_Content=Offsets_Content;
            }
            if (New->OriginalContent_Size)
            {
                int8u* OriginalContent=new int8u[New->OriginalContent_Size];
                std::memcpy(OriginalContent, Old->OriginalContent, New->OriginalContent_Size*sizeof(int8u));
                New->OriginalContent=OriginalContent;
            }
        }
    }
    else if (Event_CallBackFunction)
        Event_CallBackFunction ((unsigned char*)Data_Content, Data_Size, Event_UserHandler);
    else if (!File_Name.empty())
    {
        MediaInfo_Event_Generic* Event_Generic=(MediaInfo_Event_Generic*)Data_Content;
        if ((Event_Generic->EventCode&0x00FFFFFF)==((MediaInfo_Event_Global_Demux<<8)|0x04)) //Demux version 4
        {
            if (!MediaInfoLib::Config.Demux_Get())
                return;

            MediaInfo_Event_Global_Demux_4* Event=(MediaInfo_Event_Global_Demux_4*)Data_Content;

            Ztring File_Name_Final(File_Name);
            if (Event->StreamIDs_Size==0)
                File_Name_Final+=__T(".demux");
            else for (size_t Pos=0; Pos<Event->StreamIDs_Size; Pos++)
            {
                if (Event->StreamIDs_Width[Pos]==17)
                {
                    Ztring ID;
                    ID.From_CC4((int32u)Event->StreamIDs[Pos]);
                    File_Name_Final+=__T('.')+ID;
                }
                else if (Event->StreamIDs_Width[Pos])
                {
                    Ztring ID;
                    ID.From_Number(Event->StreamIDs[Pos], 16);
                    while (ID.size()<Event->StreamIDs_Width[Pos])
                        ID.insert(0,  1, __T('0'));
                    if (ID.size()>Event->StreamIDs_Width[Pos])
                        ID.erase(0, ID.size()-Event->StreamIDs_Width[Pos]);
                    File_Name_Final+=__T('.')+ID;
                }
                else
                    File_Name_Final+=__T(".raw");
            }

            File F;
            F.Open(File_Name_Final, File::Access_Write_Append);
            F.Write(Event->Content, Event->Content_Size);
        }
    }
}

void MediaInfo_Config_MediaInfo::Event_Accepted (File__Analyze* Source)
{
    #if MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
        if (Demux_EventWasSent && NextPacket_Get())
        {
            Events_Delayed_CurrentSource=Source;
            return;
        }
    #endif //MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET

    for (events_delayed::iterator Event=Events_Delayed.begin(); Event!=Events_Delayed.end(); ++Event)
        if (Event->first==Source)
        {
            for (size_t Pos=0; Pos<Event->second.size(); Pos++)
                if (Event->second[Pos])
                {
                    Event_Send(NULL, Event->second[Pos]->Data_Content, Event->second[Pos]->Data_Size, Event->second[Pos]->File_Name);

                    int32u EventCode=*((int32u*)Event->second[Pos]->Data_Content);
                    bool IsDemux=(EventCode&0x00FFFF00)==(MediaInfo_Event_Global_Demux<<8);

                    if (IsDemux)
                    {
                        MediaInfo_Event_Global_Demux_4* Old=(MediaInfo_Event_Global_Demux_4*)Event->second[Pos]->Data_Content;
                        delete[] Old->Content; Old->Content=NULL;
                        if (Old->Offsets_Size)
                        {
                            delete[] Old->Offsets_Content; Old->Offsets_Content=NULL;
                        }
                        if (Old->Offsets_Size)
                        {
                            delete[] Old->OriginalContent; Old->OriginalContent=NULL;
                        }
                    }

                    delete Event->second[Pos]; Event->second[Pos]=NULL;

                    #if MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
                        if (IsDemux && NextPacket_Get())
                        {
                            Demux_EventWasSent=true;
                            Event->second.erase(Event->second.begin(), Event->second.begin()+Pos);
                            Events_Delayed_CurrentSource=Source;
                            return;
                        }
                    #endif //MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
                }

            Events_Delayed.erase(Event->first);
            return;
        }
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::Event_SubFile_Start(const Ztring &FileName_Absolute)
{
    Ztring FileName_Relative;
    if (File_Names_RootDirectory.empty())
    {
        FileName FN(FileName_Absolute);
        FileName_Relative=FN.Name_Get();
        if (!FN.Extension_Get().empty())
        {
            FileName_Relative+=__T('.');
            FileName_Relative+=FN.Extension_Get();
        }
    }
    else
    {
        Ztring Root=File_Names_RootDirectory+PathSeparator;
        FileName_Relative=FileName_Absolute;
        if (FileName_Relative.find(Root)==0)
            FileName_Relative.erase(0, Root.size());
    }

    struct MediaInfo_Event_General_SubFile_Start_0 Event;
    memset(&Event, 0xFF, sizeof(struct MediaInfo_Event_Generic));
    Event.EventCode=MediaInfo_EventCode_Create(0, MediaInfo_Event_General_SubFile_Start, 0);
    Event.EventSize=sizeof(struct MediaInfo_Event_General_SubFile_Start_0);
    Event.StreamIDs_Size=0;

    std::string FileName_Relative_Ansi=FileName_Relative.To_UTF8();
    std::wstring FileName_Relative_Unicode=FileName_Relative.To_Unicode();
    std::string FileName_Absolute_Ansi=FileName_Absolute.To_UTF8();
    std::wstring FileName_Absolute_Unicode=FileName_Absolute.To_Unicode();
    Event.FileName_Relative=FileName_Relative_Ansi.c_str();
    Event.FileName_Relative_Unicode=FileName_Relative_Unicode.c_str();
    Event.FileName_Absolute=FileName_Absolute_Ansi.c_str();
    Event.FileName_Absolute_Unicode=FileName_Absolute_Unicode.c_str();

    Event_Send(NULL, (const int8u*)&Event, Event.EventSize);
}
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// Force Parser
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_MpegTs_ForceMenu_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_MpegTs_ForceMenu=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_MpegTs_ForceMenu_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_MpegTs_ForceMenu;
    return Temp;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_MpegTs_stream_type_Trust_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_MpegTs_stream_type_Trust=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_MpegTs_stream_type_Trust_Get ()
{
    CS.Enter();
    bool Temp=File_MpegTs_stream_type_Trust;
    CS.Leave();
    return Temp;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_MpegTs_Atsc_transport_stream_id_Trust_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_MpegTs_Atsc_transport_stream_id_Trust=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_MpegTs_Atsc_transport_stream_id_Trust_Get ()
{
    CS.Enter();
    bool Temp=File_MpegTs_Atsc_transport_stream_id_Trust;
    CS.Leave();
    return Temp;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_MpegTs_RealTime_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_MpegTs_RealTime=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_MpegTs_RealTime_Get ()
{
    CS.Enter();
    bool Temp=File_MpegTs_RealTime;
    CS.Leave();
    return Temp;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Bdmv_ParseTargetedFile_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Bdmv_ParseTargetedFile=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Bdmv_ParseTargetedFile_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_Bdmv_ParseTargetedFile;
    return Temp;
}

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_DVDIF_YES)
void MediaInfo_Config_MediaInfo::File_DvDif_DisableAudioIfIsInContainer_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_DvDif_DisableAudioIfIsInContainer=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_DvDif_DisableAudioIfIsInContainer_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_DvDif_DisableAudioIfIsInContainer;
    return Temp;
}
#endif //defined(MEDIAINFO_DVDIF_YES)

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_DVDIF_YES)
void MediaInfo_Config_MediaInfo::File_DvDif_IgnoreTransmittingFlags_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_DvDif_IgnoreTransmittingFlags=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_DvDif_IgnoreTransmittingFlags_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_DvDif_IgnoreTransmittingFlags;
    return Temp;
}
#endif //defined(MEDIAINFO_DVDIF_YES)

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_DVDIF_ANALYZE_YES)
void MediaInfo_Config_MediaInfo::File_DvDif_Analysis_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_DvDif_Analysis=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_DvDif_Analysis_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_DvDif_Analysis;
    return Temp;
}
#endif //defined(MEDIAINFO_DVDIF_ANALYZE_YES)

//---------------------------------------------------------------------------
#if MEDIAINFO_MACROBLOCKS
void MediaInfo_Config_MediaInfo::File_Macroblocks_Parse_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Macroblocks_Parse=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Macroblocks_Parse_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_Macroblocks_Parse;
    return Temp;
}
#endif //MEDIAINFO_MACROBLOCKS

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_GrowingFile_Delay_Set (float64 NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_GrowingFile_Delay=NewValue;
}

float64 MediaInfo_Config_MediaInfo::File_GrowingFile_Delay_Get ()
{
    CriticalSectionLocker CSL(CS);
    float64 Temp=File_GrowingFile_Delay;
    return Temp;
}

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_LIBCURL_YES)
void MediaInfo_Config_MediaInfo::File_Curl_Set (const Ztring &NewValue)
{
    size_t Pos=NewValue.find(__T(','));
    if (Pos==string::npos)
        Pos=NewValue.find(__T(';'));
    if (Pos!=string::npos)
    {
        Ztring Field=NewValue.substr(0, Pos); Field.MakeLowerCase();
        Ztring Value=NewValue.substr(Pos+1, string::npos);
        CriticalSectionLocker CSL(CS);
        Curl[Field]=Value;
    }
}

void MediaInfo_Config_MediaInfo::File_Curl_Set (const Ztring &Field_, const Ztring &NewValue)
{
    Ztring Field=Field_; Field.MakeLowerCase();
    CriticalSectionLocker CSL(CS);
    Curl[Field]=NewValue;
}

Ztring MediaInfo_Config_MediaInfo::File_Curl_Get (const Ztring &Field_)
{
    Ztring Field=Field_; Field.MakeLowerCase();
    CriticalSectionLocker CSL(CS);
    std::map<Ztring, Ztring>::iterator Value=Curl.find(Field);
    if (Value==Curl.end())
        return Ztring();
    else
        return Curl[Field];
}
#endif //defined(MEDIAINFO_LIBCURL_YES)

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_LIBMMS_YES)
void MediaInfo_Config_MediaInfo::File_Mmsh_Describe_Only_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Mmsh_Describe_Only=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Mmsh_Describe_Only_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_Mmsh_Describe_Only;
    return Temp;
}
#endif //defined(MEDIAINFO_LIBMMS_YES)

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Eia608_DisplayEmptyStream_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Eia608_DisplayEmptyStream=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Eia608_DisplayEmptyStream_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_Eia608_DisplayEmptyStream;
    return Temp;
}

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::File_Eia708_DisplayEmptyStream_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Eia708_DisplayEmptyStream=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Eia708_DisplayEmptyStream_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_Eia708_DisplayEmptyStream;
    return Temp;
}

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AC3_YES)
void MediaInfo_Config_MediaInfo::File_Ac3_IgnoreCrc_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    File_Ac3_IgnoreCrc=NewValue;
}

bool MediaInfo_Config_MediaInfo::File_Ac3_IgnoreCrc_Get ()
{
    CriticalSectionLocker CSL(CS);
    bool Temp=File_Ac3_IgnoreCrc;
    return Temp;
}
#endif //defined(MEDIAINFO_AC3_YES)

//***************************************************************************
// Analysis internal
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config_MediaInfo::State_Set (float NewValue)
{
    CriticalSectionLocker CSL(CS);
    State=NewValue;
}

float MediaInfo_Config_MediaInfo::State_Get ()
{
    CriticalSectionLocker CSL(CS);
    float Temp=State;
    return Temp;
}

} //NameSpace
