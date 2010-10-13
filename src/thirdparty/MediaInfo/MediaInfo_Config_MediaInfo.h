// MediaInfo_Config_MediaInfo - Configuration class
// Copyright (C) 2005-2010 MediaArea.net SARL, Info@MediaArea.net
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
//
// Configuration of MediaInfo (per MediaInfo block)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Config_MediaInfoH
#define MediaInfo_Config_MediaInfoH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config.h"
    #include "MediaInfo/MediaInfo_Events.h"
    #include "ZenLib/File.h"
#endif //MEDIAINFO_EVENTS
#include "ZenLib/CriticalSection.h"
#include "ZenLib/ZtringListList.h"
#include "ZenLib/Translation.h"
#include "ZenLib/InfoMap.h"
#include <map>
#include <vector>
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class MediaInfo_Config_MediaInfo
//***************************************************************************

class MediaInfo_Config_MediaInfo
{
public :
    //Constructor/Destructor
    MediaInfo_Config_MediaInfo();

    //General
    Ztring Option (const String &Option, const String &Value=Ztring());

    void          File_IsSeekable_Set (bool NewValue);
    bool          File_IsSeekable_Get ();

    void          File_IsSub_Set (bool NewValue);
    bool          File_IsSub_Get ();

    void          File_KeepInfo_Set (bool NewValue);
    bool          File_KeepInfo_Get ();

    void          File_StopAfterFilled_Set (bool NewValue);
    bool          File_StopAfterFilled_Get ();

    void          File_Audio_MergeMonoStreams_Set (bool NewValue);
    bool          File_Audio_MergeMonoStreams_Get ();

    void          File_FileName_Set (const Ztring &NewValue);
    Ztring        File_FileName_Get ();

    void          File_Partial_Begin_Set (const Ztring &NewValue);
    Ztring        File_Partial_Begin_Get ();
    void          File_Partial_End_Set (const Ztring &NewValue);
    Ztring        File_Partial_End_Get ();

    void          File_ForceParser_Set (const Ztring &NewValue);
    Ztring        File_ForceParser_Get ();

    void          File_Filter_Set     (int64u NewValue);
    bool          File_Filter_Get     (const int16u  Value);
    bool          File_Filter_Get     ();
    bool          File_Filter_HasChanged();

    Ztring        File_Duplicate_Set  (const Ztring &Value);
    Ztring        File_Duplicate_Get  (size_t AlreadyRead_Pos); //Requester must say how many Get() it already read
    bool          File_Duplicate_Get_AlwaysNeeded (size_t AlreadyRead_Pos); //Requester must say how many Get() it already read

    #if MEDIAINFO_DEMUX
    void          Demux_ForceIds_Set (bool NewValue);
    bool          Demux_ForceIds_Get ();
    void          Demux_PCM_20bitTo16bit_Set (bool NewValue);
    bool          Demux_PCM_20bitTo16bit_Get ();
    void          Demux_Unpacketize_Set (bool NewValue);
    bool          Demux_Unpacketize_Get ();
    #endif //MEDIAINFO_DEMUX

    void          NextPacket_Set (bool NewValue);
    bool          NextPacket_Get ();

    size_t        File__Duplicate_Memory_Indexes_Get (const Ztring &ToFind);
    void          File__Duplicate_Memory_Indexes_Erase (const Ztring &ToFind);

    #if MEDIAINFO_EVENTS
    bool          Event_CallBackFunction_IsSet ();
    Ztring        Event_CallBackFunction_Set (const Ztring &Value);
    void          Event_Send(const int8u* Data_Content, size_t Data_Size);
    void          Event_Send(const int8u* Data_Content, size_t Data_Size, const Ztring &File_Name);
    #else //MEDIAINFO_EVENTS
    bool          Event_CallBackFunction_IsSet () {return false;}
    void          Event_Send(const int8u*, size_t) {};
    void          Event_Send(const int8u*, size_t, const Ztring &) {};
    #endif //MEDIAINFO_EVENTS

    //Specific
    void          File_MpegTs_ForceMenu_Set (bool NewValue);
    bool          File_MpegTs_ForceMenu_Get ();
    void          File_MpegTs_stream_type_Trust_Set (bool NewValue);
    bool          File_MpegTs_stream_type_Trust_Get ();
    void          File_Bdmv_ParseTargetedFile_Set (bool NewValue);
    bool          File_Bdmv_ParseTargetedFile_Get ();
    void          File_DvDif_Analysis_Set (bool NewValue);
    bool          File_DvDif_Analysis_Get ();
    void          File_Curl_Set (const Ztring &NewValue);
    Ztring        File_Curl_Get (const Ztring &Field);

    //Analysis internal
    void          State_Set (float State);
    float         State_Get ();

    //Internal to MediaInfo, not thread safe
    #if MEDIAINFO_DEMUX
    bool          Demux_EventWasSent;
    #endif //MEDIAINFO_DEMUX

private :
    bool                    FileIsSeekable;
    bool                    FileIsSub;
    bool                    FileKeepInfo;
    bool                    FileStopAfterFilled;
    bool                    Audio_MergeMonoStreams;
    Ztring                  File_FileName;
    Ztring                  File_Partial_Begin;
    Ztring                  File_Partial_End;
    Ztring                  File_ForceParser;

    //Extra
    std::map<int16u, bool>  File_Filter_16;
    bool                    File_Filter_HasChanged_;

    std::vector<Ztring>     File__Duplicate_List;
    ZtringList              File__Duplicate_Memory_Indexes;

    bool                    Demux_ForceIds;
    bool                    Demux_PCM_20bitTo16bit;
    bool                    Demux_Unpacketize;
    bool                    NextPacket;

    //Event
    #if MEDIAINFO_EVENTS
    MediaInfo_Event_CallBackFunction* Event_CallBackFunction; //void Event_Handler(unsigned char* Data_Content, size_t Data_Size, void* UserHandler)
    void*                   Event_UserHandler;
    #endif //MEDIAINFO_EVENTS

    //Specific
    bool                    File_MpegTs_ForceMenu;
    bool                    File_MpegTs_stream_type_Trust;
    bool                    File_Bdmv_ParseTargetedFile;
    bool                    File_DvDif_Analysis;

    //Analysis internal
    float                   State;

    //Generic
    std::map<Ztring, Ztring> Curl;

    ZenLib::CriticalSection CS;
};

} //NameSpace

#endif

