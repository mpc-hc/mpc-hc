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
#if defined(MEDIAINFO_MPEGTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_MpegTs.h"
#include "MediaInfo/Multiple/File_MpegPs.h"
#include "MediaInfo/Multiple/File_Mpeg_Descriptors.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "ZenLib/File.h"
#include <memory>
#include <algorithm>
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
#if MEDIAINFO_IBI
    #if MEDIAINFO_SEEK
        #include "MediaInfo/Multiple/File_Ibi.h"
    #endif //MEDIAINFO_SEEK
#endif //MEDIAINFO_IBI
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

namespace Elements
{
    const int32u HDMV=0x48444D56; //BluRay
}


#if !MEDIAINFO_ADVANCED
    const float64   Config_VbrDetection_Delta=0;
    const int64u    Config_VbrDetection_Occurences=4;
#endif // MEDIAINFO_ADVANCED

//***************************************************************************
// Depends of configuration
//***************************************************************************

#if !defined(MEDIAINFO_BDAV_YES)
    const size_t BDAV_Size=0;
#endif
#if !defined(MEDIAINFO_TSP_YES)
    const size_t TSP_Size=0;
#endif
#if !defined(MEDIAINFO_BDAV_YES) && !defined(MEDIAINFO_TSP_YES)
    const size_t TS_Size=188;
#endif

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
//From Mpeg_Psi
extern const char* Mpeg_Descriptors_registration_format_identifier_Format(int32u format_identifier);
extern stream_t    Mpeg_Descriptors_registration_format_identifier_StreamKind(int32u format_identifier);

extern const char* Mpeg_Psi_stream_type_Format(int8u stream_type, int32u format_identifier);
extern const char* Mpeg_Psi_stream_type_Codec(int8u stream_type, int32u format_identifier);
extern stream_t    Mpeg_Psi_stream_type_StreamKind(int32u stream_type, int32u format_identifier);
extern const char* Mpeg_Psi_stream_type_Info(int8u stream_type, int32u format_identifier);

extern const char* Mpeg_Psi_table_id(int8u table_id);
extern const char* Mpeg_Descriptors_stream_Format(int8u descriptor_tag, int32u format_identifier);
extern const char* Mpeg_Descriptors_stream_Codec(int8u descriptor_tag, int32u format_identifier);
extern stream_t    Mpeg_Descriptors_stream_Kind(int8u descriptor_tag, int32u format_identifier);

extern const char* Mpeg_Descriptors_CA_system_ID(int16u CA_system_ID);

//---------------------------------------------------------------------------
Ztring Decimal_Hexa(int64u Number)
{
    Ztring Temp;
    Temp.From_Number(Number);
    Temp+=__T(" (0x");
    Temp+=Ztring::ToZtring(Number, 16);
    Temp+=__T(")");
    return Temp;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_MpegTs::File_MpegTs()
#if MEDIAINFO_DUPLICATE
:File__Duplicate()
#endif //MEDIAINFO_DUPLICATE
{
    //Configuration
    ParserName=__T("MpegTs");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_MpegTs;
        StreamIDs_Width[0]=4;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=4; //Intermediate
    #endif //MEDIAINFO_DEMUX
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    Buffer_TotalBytes_Fill_Max=(int64u)-1; //Disabling this feature for this format, this is done in the parser
    Trusted_Multiplier=2;
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Internal config
    #if defined(MEDIAINFO_BDAV_YES)
        BDAV_Size=0; //No BDAV header
    #endif
    #if defined(MEDIAINFO_TSP_YES)
        TSP_Size=0; //No TSP footer
    #endif

    //Data
    MpegTs_JumpTo_Begin=MediaInfoLib::Config.MpegTs_MaximumOffset_Get();
    MpegTs_JumpTo_End=MediaInfoLib::Config.MpegTs_MaximumOffset_Get()/4;
    Searching_TimeStamp_Start=true;
    Complete_Stream=NULL;
    Begin_MaxDuration=MediaInfoLib::Config.ParseSpeed_Get()>=0.8?(int64u)-1:MediaInfoLib::Config.MpegTs_MaximumScanDuration_Get()*27/1000;
    ForceStreamDisplay=MediaInfoLib::Config.MpegTs_ForceStreamDisplay_Get();

    #if MEDIAINFO_SEEK
        Seek_Value=(int64u)-1;
        Seek_ID=(int64u)-1;
        InfiniteLoop_Detect=0;
        Duration_Detected=false;
    #endif //MEDIAINFO_SEEK
}

File_MpegTs::~File_MpegTs ()
{
    delete Complete_Stream; Complete_Stream=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, BDAV_Size?"BDAV":(TSP_Size?"MPEG-TS 188+16":"MPEG-TS"), Unlimited, true, true);

    #if MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
        if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            Config->Demux_EventWasSent=true;
    #endif //MEDIAINFO_DEMUX && MEDIAINFO_NEXTPACKET
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Fill ()
{
    Status[User_20]=true;
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Update()
{
    if (Status[User_19])
        Streams_Update_Programs();

    if (Status[User_18])
        Streams_Update_EPG();

    #ifdef MEDIAINFO_MPEGTS_PCR_YES
        if (Status[User_16])
            Streams_Update_Duration_Update();
    #endif //MEDIAINFO_MPEGTS_PCR_YES

    if (Status[User_17])
        Streams_Update_Duration_End();

    if (File_Name.empty() && Config->ParseSpeed>=1.0)
        Fill(Stream_General, 0, General_FileSize, (File_Offset+Buffer_Offset!=File_Size)?Buffer_TotalBytes:File_Size, 10, true);
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Update_Programs()
{
    //Per stream
    bool PerStream_AlwaysParse=ForceStreamDisplay;
    if (!PerStream_AlwaysParse)
    {
        size_t Programs_Size=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.size();
        PerStream_AlwaysParse=true;
        if (Programs_Size<=2)
        {
            //Testing if it is a Blu-ray
            for (complete_stream::transport_stream::programs::iterator Program=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.begin(); Program!=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.end(); ++Program)
                if (Program->first!=0x0000 && Program->second.registration_format_identifier!=Elements::HDMV)
                    {
                        PerStream_AlwaysParse=false;
                        break;
                    }
        }
    }
    for (std::set<int16u>::iterator StreamID=Complete_Stream->PES_PIDs.begin(); StreamID!=Complete_Stream->PES_PIDs.end(); ++StreamID)
        if (PerStream_AlwaysParse || Complete_Stream->Streams[*StreamID]->IsUpdated_IsRegistered || Complete_Stream->Streams[*StreamID]->IsUpdated_Info)
        {
            Streams_Update_Programs_PerStream(*StreamID);
            Complete_Stream->Streams[*StreamID]->IsUpdated_IsRegistered=false;
            Complete_Stream->Streams[*StreamID]->IsUpdated_Info=false;
        }

    //Fill General
    if (Complete_Stream->transport_stream_id_IsValid)
    {
        Fill(Stream_General, 0, General_ID, Complete_Stream->transport_stream_id, 10, true);
        Fill(Stream_General, 0, General_ID_String, Decimal_Hexa(Complete_Stream->transport_stream_id), true);
    }
    if (!Complete_Stream->network_name.empty())
    {
        Fill(Stream_General, 0, General_NetworkName, Complete_Stream->network_name, true);
        Complete_Stream->network_name.clear();
    }
    if (!Complete_Stream->original_network_name.empty())
    {
        Fill(Stream_General, 0, General_OriginalNetworkName, Complete_Stream->original_network_name, true);
        Complete_Stream->original_network_name.clear();
    }
    Ztring Countries;
    Ztring TimeZones;
    for (std::map<Ztring, Ztring>::iterator TimeZone=Complete_Stream->TimeZones.begin(); TimeZone!=Complete_Stream->TimeZones.end(); ++TimeZone)
    {
        Countries+=TimeZone->first+__T(" / ");
        TimeZones+=TimeZone->second+__T(" / ");
    }
    if (!Countries.empty())
    {
        Countries.resize(Countries.size()-3);
        Fill(Stream_General, 0, General_Country, Countries, true);
        Complete_Stream->TimeZones.clear();
    }
    if (!TimeZones.empty())
    {
        TimeZones.resize(TimeZones.size()-3);
        Fill(Stream_General, 0, General_TimeZone, TimeZones, true);
        Complete_Stream->TimeZones.clear();
    }
    if (!Complete_Stream->Duration_Start.empty())
    {
        Fill(Stream_General, 0, General_Duration_Start, Complete_Stream->Duration_Start, true);
        Complete_Stream->Duration_Start.clear();
    }
    complete_stream::transport_streams::iterator Transport_Stream=Complete_Stream->transport_stream_id_IsValid?Complete_Stream->Transport_Streams.find(Complete_Stream->transport_stream_id):Complete_Stream->Transport_Streams.end();
    if (Transport_Stream!=Complete_Stream->Transport_Streams.end())
    {
        //TS info
        for (std::map<std::string, ZenLib::Ztring>::iterator Info=Transport_Stream->second.Infos.begin(); Info!=Transport_Stream->second.Infos.end(); ++Info)
            Fill(Stream_General, 0, Info->first.c_str(), Info->second, true);
        Transport_Stream->second.Infos.clear();

        //Per source (ATSC)
        if (Transport_Stream->second.source_id_IsValid)
        {
            complete_stream::sources::iterator Source=Complete_Stream->Sources.find(Transport_Stream->second.source_id);
            if (Source!=Complete_Stream->Sources.end())
            {
                if (!Source->second.texts.empty())
                {
                    Ztring Texts;
                    for (std::map<int16u, Ztring>::iterator text=Source->second.texts.begin(); text!=Source->second.texts.end(); ++text)
                        Texts+=text->second+__T(" - ");
                    if (!Texts.empty())
                        Texts.resize(Texts.size()-3);
                    Fill(Stream_General, 0, General_ServiceProvider, Texts);
                }
            }
        }

        //Per program
        for (complete_stream::transport_stream::programs::iterator Program=Transport_Stream->second.Programs.begin(); Program!=Transport_Stream->second.Programs.end(); ++Program)
        {
            if (Program->second.IsParsed)
            {
                //Per pid
                Ztring Languages, Codecs, Formats, StreamKinds, StreamPoss, elementary_PIDs, elementary_PIDs_String, Delay;
                for (size_t Pos=0; Pos<Program->second.elementary_PIDs.size(); Pos++)
                {
                    int16u elementary_PID=Program->second.elementary_PIDs[Pos];
                    if (PerStream_AlwaysParse || Complete_Stream->Streams[elementary_PID]->IsRegistered)
                    {
                        Ztring Format=Retrieve(Complete_Stream->Streams[elementary_PID]->StreamKind, Complete_Stream->Streams[elementary_PID]->StreamPos, Fill_Parameter(Complete_Stream->Streams[elementary_PID]->StreamKind, Generic_Format));
                        if (Format.empty())
                            Format=Mpeg_Psi_stream_type_Format(Complete_Stream->Streams[elementary_PID]->stream_type, Program->second.registration_format_identifier);
                        if (Format.empty())
                        {
                            std::map<std::string, Ztring>::iterator Format_FromInfo=Complete_Stream->Streams[elementary_PID]->Infos.find("Format");
                            if (Format_FromInfo!=Complete_Stream->Streams[elementary_PID]->Infos.end())
                                Format=Format_FromInfo->second;
                        }
                        if (Format.empty())
                            Program->second.HasNotDisplayableStreams=true;
                        Formats+=Format+__T(" / ");
                        Codecs+=Retrieve(Complete_Stream->Streams[elementary_PID]->StreamKind, Complete_Stream->Streams[elementary_PID]->StreamPos, Fill_Parameter(Complete_Stream->Streams[elementary_PID]->StreamKind, Generic_Codec))+__T(" / ");
                        if (Complete_Stream->Streams[elementary_PID]->StreamKind!=Stream_Max)
                        {
                            StreamKinds+=Ztring::ToZtring(Complete_Stream->Streams[elementary_PID]->StreamKind);
                            StreamPoss+=Ztring::ToZtring(Complete_Stream->Streams[elementary_PID]->StreamPos);
                        }
                        StreamKinds+=__T(" / ");
                        StreamPoss+=__T(" / ");
                        elementary_PIDs+=Ztring::ToZtring(elementary_PID)+__T(" / ");
                        Ztring Language=Retrieve(Complete_Stream->Streams[elementary_PID]->StreamKind, Complete_Stream->Streams[elementary_PID]->StreamPos, "Language/String");
                        Languages+=Language+__T(" / ");
                        Ztring List_String=Decimal_Hexa(elementary_PID);
                        List_String+=__T(" (");
                        List_String+=Format;
                        if (!Language.empty())
                        {
                            List_String+=__T(", ");
                            List_String+=Language;
                        }
                        List_String+=__T(")");
                        elementary_PIDs_String+=List_String+__T(" / ");

                        if (Complete_Stream->Streams[elementary_PID]->IsPCR)
                        {
                            Delay=Ztring::ToZtring(((float64)Complete_Stream->Streams[elementary_PID]->TimeStamp_Start)/27000, 6);
                        }
                    }
                }

                if (Program->second.Update_Needed_Info || Program->second.Update_Needed_IsRegistered || Program->second.Update_Needed_StreamCount || Program->second.Update_Needed_StreamPos)
                {
                    if (!Transport_Stream->second.Programs.empty()
                     && (Transport_Stream->second.Programs.size()>1
                      || Transport_Stream->second.Programs.begin()->second.HasNotDisplayableStreams
                      || !Transport_Stream->second.Programs.begin()->second.Infos.empty()
                      || !Transport_Stream->second.Programs.begin()->second.DVB_EPG_Blocks.empty()
                      || (Transport_Stream->second.Programs.begin()->second.source_id_IsValid && Complete_Stream->Sources.find(Transport_Stream->second.Programs.begin()->second.source_id)!=Complete_Stream->Sources.end())
                      || Config->File_MpegTs_ForceMenu_Get()))
                    {
                        if (Program->second.StreamPos==(size_t)-1)
                        {
                            size_t StreamPos=(size_t)-1;
                            for (size_t program_number=0; program_number<Complete_Stream->program_number_Order.size(); program_number++)
                                if (Program->first<Complete_Stream->program_number_Order[program_number])
                                {
                                    StreamPos=program_number;
                                    for (size_t program_number2=program_number; program_number2<Complete_Stream->program_number_Order.size(); program_number2++)
                                        Transport_Stream->second.Programs[Complete_Stream->program_number_Order[program_number2]].StreamPos++;
                                    Complete_Stream->program_number_Order.insert(Complete_Stream->program_number_Order.begin()+program_number, Program->first);
                                    break;
                                }
                            if (StreamPos==(size_t)-1)
                            {
                                Complete_Stream->program_number_Order.push_back(Program->first);
                            }

                            Stream_Prepare(Stream_Menu, StreamPos);
                            Program->second.StreamPos=StreamPos_Last;
                        }
                        else
                            StreamPos_Last=Program->second.StreamPos;
                        Fill(Stream_Menu, StreamPos_Last, Menu_ID, Program->second.pid, 10, true);
                        Fill(Stream_Menu, StreamPos_Last, Menu_ID_String, Decimal_Hexa(Program->second.pid), true);
                        Fill(Stream_Menu, StreamPos_Last, Menu_MenuID, Program->first, 10, true);
                        Fill(Stream_Menu, StreamPos_Last, Menu_MenuID_String, Decimal_Hexa(Program->first), true);
                        Clear(Stream_Menu, StreamPos_Last, General_StreamOrder);
                        for (size_t programs_List_Pos=0; programs_List_Pos<Transport_Stream->second.programs_List.size(); ++programs_List_Pos)
                            if (Transport_Stream->second.programs_List[programs_List_Pos]==Program->first)
                                Fill(Stream_Menu, StreamPos_Last, General_StreamOrder, programs_List_Pos);
                        for (std::map<std::string, ZenLib::Ztring>::iterator Info=Program->second.Infos.begin(); Info!=Program->second.Infos.end(); ++Info)
                            Fill(Stream_Menu, StreamPos_Last, Info->first.c_str(), Info->second, true);
                        Program->second.Infos.clear();
                        for (std::map<std::string, ZenLib::Ztring>::iterator Info=Program->second.ExtraInfos_Content.begin(); Info!=Program->second.ExtraInfos_Content.end(); ++Info)
                            Fill(Stream_Menu, StreamPos_Last, Info->first.c_str(), Info->second, true);
                        Program->second.ExtraInfos_Content.clear();
                        for (std::map<std::string, ZenLib::Ztring>::iterator Info=Program->second.ExtraInfos_Option.begin(); Info!=Program->second.ExtraInfos_Option.end(); ++Info)
                            (*Stream_More)[Stream_Menu][StreamPos_Last](Ztring().From_Local(Info->first.c_str()), Info_Options)=Info->second;
                        Program->second.ExtraInfos_Option.clear();

                        if (!Formats.empty())
                            Formats.resize(Formats.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_Format, Formats, true);
                        if (!Codecs.empty())
                            Codecs.resize(Codecs.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_Codec, Codecs, true);
                        if (!StreamKinds.empty())
                            StreamKinds.resize(StreamKinds.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_List_StreamKind, StreamKinds, true);
                        if (!elementary_PIDs_String.empty())
                            elementary_PIDs_String.resize(elementary_PIDs_String.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_List_String, elementary_PIDs_String, true);
                        if (!elementary_PIDs.empty())
                            elementary_PIDs.resize(elementary_PIDs.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_List, elementary_PIDs, true);
                        if (!StreamPoss.empty())
                            StreamPoss.resize(StreamPoss.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_List_StreamPos, StreamPoss, true);
                        if (!Languages.empty())
                            Languages.resize(Languages.size()-3);
                        Fill(Stream_Menu, StreamPos_Last, Menu_Language, Languages, true);
                    }
                }

                //Delay
                if (Program->second.Update_Needed_IsRegistered)
                {
                    switch (Count_Get(Stream_Menu))
                    {
                        case 0 : Fill(Stream_General, 0, General_Delay, Delay, true); break;
                        default: Fill(Stream_Menu, StreamPos_Last, Menu_Delay, Delay, true); break;
                    }
                    Program->second.Update_Needed_IsRegistered=false;
                }
                if (Count_Get(Stream_Menu)==2)
                    Clear(Stream_General, 0, General_Delay); //Not valid, multiple menus
            }
        }
    }

    //Commercial name
    if (Count_Get(Stream_Video)==1
     && Count_Get(Stream_Audio)==1
     && Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video")
     && Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny).find(__T("HDV"))==0
     && Retrieve(Stream_Audio, 0, Audio_Format)==__T("MPEG Audio")
     && Retrieve(Stream_Audio, 0, Audio_Format_Version)==__T("Version 1")
     && Retrieve(Stream_Audio, 0, Audio_Format_Profile)==__T("Layer 2")
     && Retrieve(Stream_Audio, 0, Audio_BitRate)==__T("384000"))
        Fill(Stream_General, 0, General_Format_Commercial_IfAny, Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Update_Programs_PerStream(size_t StreamID)
{
    complete_stream::stream* Temp=Complete_Stream->Streams[StreamID];

    //No direct handling of Sub streams;
    if (Temp->stream_type==0x20 && Temp->SubStream_pid) //Stereoscopic is not alone
        return;

    //Merging from a previous merge
    size_t Count;
    if (Temp->StreamKind!=Stream_Max)
    {
        Count=1; //TODO: more than 1
        Merge(*Temp->Parser, Temp->StreamKind, 0, Temp->StreamPos);
        StreamKind_Last=Temp->StreamKind;
        StreamPos_Last=Temp->StreamPos;
    }
    else
    {
        //By the parser
        StreamKind_Last=Stream_Max;
        Count=0;
        if (Temp->Parser && Temp->Parser->Status[IsAccepted])
        {
            if (Temp->SubStream_pid!=0x0000) //With a substream
                Fill(Complete_Stream->Streams[Temp->SubStream_pid]->Parser);
            if (Temp->Parser->Count_Get(Stream_Video) && Temp->Parser->Count_Get(Stream_Text))
            {
                //Special case: Video and Text are together
                Stream_Prepare(Stream_Video);
                Count=Merge(*Temp->Parser, Stream_Video, 0, StreamPos_Last);
            }
            else
                Count=Merge(*Temp->Parser);

            //More from the FMC parser
            if (Temp->FMC_ES_ID_IsValid)
            {
                complete_stream::transport_stream::iod_ess::iterator IOD_ES=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.find(Temp->FMC_ES_ID);
                if (IOD_ES!=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.end() && IOD_ES->second.Parser)
                {
                    Finish(IOD_ES->second.Parser);
                    Count=Merge(*IOD_ES->second.Parser, StreamKind_Last, StreamPos_Last, 0);
                }
            }

            //LATM
            complete_stream::transport_stream::iod_ess::iterator IOD_ES=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.find(Complete_Stream->Streams[StreamID]->FMC_ES_ID);
            #ifdef MEDIAINFO_MPEG4_YES
                if (IOD_ES!=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.end() && IOD_ES->second.SLConfig && Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode).empty())
                    Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, "SL");
            #endif
            if (Complete_Stream->Streams[StreamID]->stream_type==0x11 && Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode).empty())
                Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, "LATM");
        }

        //By the descriptors
        if (StreamKind_Last==Stream_Max && Complete_Stream->transport_stream_id_IsValid && !Temp->program_numbers.empty() && !Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.empty())
        {
            int32u format_identifier=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Temp->program_numbers[0]].registration_format_identifier;
            if (Temp->IsRegistered
             && Mpeg_Descriptors_registration_format_identifier_StreamKind(format_identifier)!=Stream_Max)
            {
                StreamKind_Last=Mpeg_Descriptors_registration_format_identifier_StreamKind(format_identifier);
                Stream_Prepare(StreamKind_Last);
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), Mpeg_Descriptors_registration_format_identifier_Format(format_identifier));
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), Mpeg_Descriptors_registration_format_identifier_Format(format_identifier));
                Count=1;
            }
        }

        //By registration_format_identifier
        if (StreamKind_Last==Stream_Max && Temp->registration_format_identifier && Temp->IsRegistered && Mpeg_Descriptors_registration_format_identifier_StreamKind(Temp->registration_format_identifier)!=Stream_Max)
        {
            StreamKind_Last=Mpeg_Descriptors_registration_format_identifier_StreamKind(Temp->registration_format_identifier);
            Stream_Prepare(StreamKind_Last);
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), Mpeg_Descriptors_registration_format_identifier_Format(Temp->registration_format_identifier));
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), Mpeg_Descriptors_registration_format_identifier_Format(Temp->registration_format_identifier));
            Count=1;
        }

        //By the stream_type
        if (StreamKind_Last==Stream_Max && Complete_Stream->transport_stream_id_IsValid && !Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.empty())
        {
            int32u format_identifier=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Temp->program_numbers[0]].registration_format_identifier;
            if (Mpeg_Psi_stream_type_StreamKind(Temp->stream_type, format_identifier)!=Stream_Max && (Temp->IsRegistered || ForceStreamDisplay || format_identifier==Elements::HDMV))
            {
                StreamKind_Last=Mpeg_Psi_stream_type_StreamKind(Temp->stream_type, format_identifier);
                if (StreamKind_Last==Stream_General && Temp->Parser) //Only information, no streams
                {
                    Merge (*Temp->Parser, Stream_General, 0, 0);
                    StreamKind_Last=Stream_Max;
                }
                Stream_Prepare(StreamKind_Last);
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), Mpeg_Psi_stream_type_Format(Temp->stream_type, format_identifier));
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), Mpeg_Psi_stream_type_Codec(Temp->stream_type, format_identifier));
                Count=1;
            }
        }

        //By the StreamKind
        if (StreamKind_Last==Stream_Max && Temp->StreamKind_FromDescriptor!=Stream_Max && (Temp->IsRegistered || ForceStreamDisplay || Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Temp->program_numbers[0]].registration_format_identifier==Elements::HDMV))
        {
            Stream_Prepare(Temp->StreamKind_FromDescriptor);
            Count=1;
        }
    }

    //More info
    if (StreamKind_Last!=Stream_Max)
    {
        for (size_t StreamPos=StreamPos_Last+1-Count; StreamPos<=StreamPos_Last; StreamPos++)
        {
            Temp->StreamKind=StreamKind_Last;
            Temp->StreamPos=StreamPos;

            //Encryption
            if (Temp->CA_system_ID)
                Fill(StreamKind_Last, StreamPos, "Encryption", Mpeg_Descriptors_CA_system_ID(Temp->CA_system_ID));
            else if (Temp->IsScrambled>16)
                Fill(StreamKind_Last, StreamPos, "Encryption", "Encrypted");

            //TS info
            for (std::map<std::string, ZenLib::Ztring>::iterator Info=Temp->Infos.begin(); Info!=Temp->Infos.end(); ++Info)
            {
                if (Retrieve(StreamKind_Last, StreamPos, Info->first.c_str()).empty())
                    Fill(StreamKind_Last, StreamPos, Info->first.c_str(), Info->second, true);
            }
            Temp->Infos.clear();
            for (std::map<std::string, ZenLib::Ztring>::iterator Info=Temp->Infos_Option.begin(); Info!=Temp->Infos_Option.end(); ++Info)
                (*Stream_More)[StreamKind_Last][StreamPos](Ztring().From_Local(Info->first.c_str()), Info_Options)=Info->second;
            Temp->Infos_Option.clear();

            //Common
            if (Temp->SubStream_pid!=0x0000) //Wit a substream
            {
                Ztring Format_Profile=Retrieve(Stream_Video, StreamPos, Video_Format_Profile);
                Fill(Stream_Video, StreamPos, Video_ID, Ztring::ToZtring(Temp->SubStream_pid)+__T(" / ")+Ztring::ToZtring(StreamID), true);
                Fill(Stream_Video, StreamPos, Video_ID_String, Decimal_Hexa(Temp->SubStream_pid)+__T(" / ")+Decimal_Hexa(StreamID), true);
                if (!Format_Profile.empty() && Complete_Stream->Streams[Temp->SubStream_pid] && Complete_Stream->Streams[Temp->SubStream_pid]->Parser)
                    Fill(Stream_Video, StreamPos, Video_Format_Profile, Complete_Stream->Streams[Temp->SubStream_pid]->Parser->Retrieve(Stream_Video, 0, Video_Format_Profile)+__T(" / ")+Format_Profile, true);
            }
            else if (Count>1 || (StreamKind_Last==Stream_Text && Retrieve(StreamKind_Last, StreamPos, General_ID).find(__T('-'))!=string::npos))
            {
                Ztring ID=Retrieve(StreamKind_Last, StreamPos, General_ID);
                size_t ID_Pos=ID.find(__T('-'));
                if (ID_Pos!=string::npos)
                    ID.erase(ID.begin(), ID.begin()+ID_Pos+1); //Removing the PS part
                Ztring ID_String=Retrieve(StreamKind_Last, StreamPos, General_ID_String);
                size_t ID_String_Pos=ID_String.find(__T('-'));
                if (ID_String_Pos!=string::npos)
                    ID_String.erase(ID_String.begin(), ID_String.begin()+ID_String_Pos+1); //Removing the PS part
                Fill(StreamKind_Last, StreamPos, General_ID, Ztring::ToZtring(StreamID)+__T('-')+ID, true);
                Fill(StreamKind_Last, StreamPos, General_ID_String, Decimal_Hexa(StreamID)+__T('-')+ID_String, true);
            }
            else
            {
                Fill(StreamKind_Last, StreamPos, General_ID, StreamID, 10, true);
                Fill(StreamKind_Last, StreamPos, General_ID_String, Decimal_Hexa(StreamID), true);
            }
            for (size_t Pos=0; Pos<Temp->program_numbers.size(); Pos++)
            {
                Fill(StreamKind_Last, StreamPos, General_MenuID, Temp->program_numbers[Pos], 10, Pos==0);
                Fill(StreamKind_Last, StreamPos, General_MenuID_String, Decimal_Hexa(Temp->program_numbers[Pos]), Pos==0);
            }

            //StreamOrder
            Clear(StreamKind_Last, StreamPos, General_StreamOrder);
            for (size_t program_FromStream=0; program_FromStream<Temp->program_numbers.size(); ++program_FromStream)
            {
                int16u program_number=Temp->program_numbers[program_FromStream];
                std::vector<int16u> &programs_List=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].programs_List;
                size_t programs_List_Pos=0;
                for (; programs_List_Pos<programs_List.size(); ++programs_List_Pos)
                    if (programs_List[programs_List_Pos]==program_number)
                        break;
                if (programs_List_Pos<programs_List.size())
                {
                    complete_stream::transport_stream::program &Program=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number];
                    for (size_t elementary_PID_Pos=0; elementary_PID_Pos<Program.elementary_PIDs.size(); ++elementary_PID_Pos)
                        if (Program.elementary_PIDs[elementary_PID_Pos]==StreamID)
                            Fill(StreamKind_Last, StreamPos, General_StreamOrder,  Ztring::ToZtring(programs_List_Pos)+__T('-')+Ztring::ToZtring(elementary_PID_Pos));
                }
            }

            //Special cases
            if (StreamKind_Last==Stream_Video && Temp->Parser && Temp->Parser->Count_Get(Stream_Text))
            {
            }
        }

        //Special cases
        if (Temp->Parser && Temp->Parser->Count_Get(Stream_Video))
        {
            //Video and Text may be together

            //Retrieving IDs
            int16u ID_Video=Retrieve(Stream_Video, StreamPos_Last, Text_ID).To_int16u();

            //Language
            std::map<int8u, string>  Eia708_Languages; //Key is caption_service_number
            Eia708_Languages.insert(Temp->Eia708_Languages.begin(), Temp->Eia708_Languages.end()); //708 from video descriptor
            bool Eia608_FromDescriptor=Temp->Eia608_IsPresent;
            for (size_t ProgramPos=0; ProgramPos<Complete_Stream->Streams[ID_Video]->program_numbers.size(); ProgramPos++)
            {
                //608 from program descriptor
                if (!Eia608_FromDescriptor)
                    Eia608_FromDescriptor=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[ID_Video]->program_numbers[ProgramPos]].Eia608_IsPresent;

                //708 from program descriptor
                Eia708_Languages.insert(Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[ID_Video]->program_numbers[ProgramPos]].Eia708_Languages.begin(),
                                        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[ID_Video]->program_numbers[ProgramPos]].Eia708_Languages.end());

                //608/708 from ATSC EIT
                if (Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[ID_Video]->program_numbers[ProgramPos]].source_id_IsValid)
                {
                    int16u source_id=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[ID_Video]->program_numbers[ProgramPos]].source_id;
                    complete_stream::sources::iterator Source=Complete_Stream->Sources.find(source_id);
                    if (Source!=Complete_Stream->Sources.end())
                        for (complete_stream::source::atsc_epg_blocks::iterator ATSC_EPG_Block=Source->second.ATSC_EPG_Blocks.begin(); ATSC_EPG_Block!=Source->second.ATSC_EPG_Blocks.end(); ++ATSC_EPG_Block)
                            for (complete_stream::source::atsc_epg_block::events::iterator Event=ATSC_EPG_Block->second.Events.begin(); Event!=ATSC_EPG_Block->second.Events.end(); ++Event)
                            {
                                Eia708_Languages.insert(Event->second.Eia708_Languages.begin(), Event->second.Eia708_Languages.end());
                                if (!Eia608_FromDescriptor)
                                    Eia608_FromDescriptor=Event->second.Eia608_IsPresent;
                            }
                }
            }

            size_t Text_Count=Temp->Parser->Count_Get(Stream_Text);
            bool Eia608_IsPresent=false;
            for (size_t Text_Pos=0; Text_Pos<Text_Count; Text_Pos++)
            {
                Ztring Parser_ID=Temp->Parser->Retrieve(Stream_Text, Text_Pos, Text_ID);
                if (Parser_ID.find(__T('-'))!=string::npos)
                    Parser_ID.erase(Parser_ID.begin(), Parser_ID.begin()+Parser_ID.find(__T('-'))+1);
                Ztring ID=Retrieve(Stream_Video, Temp->StreamPos, Video_ID)+__T('-')+Parser_ID;
                Ztring ID_String=Retrieve(Stream_Video, Temp->StreamPos, Video_ID_String)+__T('-')+Parser_ID;
                StreamPos_Last=(size_t)-1;
                for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
                    if (Retrieve(Stream_Text, Pos, Text_ID)==ID && Retrieve(Stream_Text, Pos, "MuxingMode")==Temp->Parser->Retrieve(Stream_Text, Text_Pos, "MuxingMode"))
                    {
                        StreamPos_Last=Pos;
                        break;
                    }
                if (StreamPos_Last==(size_t)-1)
                    Stream_Prepare(Stream_Text, StreamPos_Last);
                if (!IsSub)
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode_MoreInfo", __T("Muxed in Video #")+Ztring().From_Number(Temp->StreamPos+1), true);
                Merge(*Temp->Parser, Stream_Text, Text_Pos, StreamPos_Last);

                Fill(Stream_Text, StreamPos_Last, Text_ID, ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, ID_String, true);
                Fill(Stream_Text, StreamPos_Last, General_StreamOrder, Retrieve(Stream_Video, Temp->StreamPos, General_StreamOrder), true);
                Fill(Stream_Text, StreamPos_Last, Text_MenuID, Retrieve(Stream_Video, Temp->StreamPos, Video_MenuID), true);
                Fill(Stream_Text, StreamPos_Last, Text_MenuID_String, Retrieve(Stream_Video, Temp->StreamPos, Video_MenuID_String), true);
                Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, Temp->StreamPos, Video_Duration), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Temp->StreamPos, Video_Delay), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay_Source, Retrieve(Stream_Video, Temp->StreamPos, Video_Delay_Source), true);

                if (Retrieve(Stream_Text, StreamPos_Last, Text_Format).find(__T("EIA-708"))!=string::npos && Retrieve(Stream_Text, StreamPos_Last, Text_Language).empty())
                {
                    Ztring ID_Complete=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                    int8u  ID_Text=Ztring(ID_Complete.substr(ID_Complete.rfind(__T('-'))+1, string::npos)).To_int8u();
                    std::map<int8u, string>::iterator Language=Eia708_Languages.find(ID_Text);
                    if (Language!=Eia708_Languages.end())
                    {
                        Fill(Stream_Text, StreamPos_Last, Text_Language, Language->second);
                        Eia708_Languages.erase(Language);
                    }
                }
                if (Retrieve(Stream_Text, StreamPos_Last, Text_Format).find(__T("EIA-608"))!=string::npos)
                    Eia608_IsPresent=true;
            }

            //Undetected text streams but present is service_descriptor - 608
            if (Eia608_FromDescriptor && !Eia608_IsPresent)
            {
                Ztring ID=Retrieve(Stream_Video, Temp->StreamPos, Video_ID)+__T("-608");
                Ztring ID_String=Retrieve(Stream_Video, Temp->StreamPos, Video_ID_String)+__T("-608");
                StreamPos_Last=(size_t)-1;
                for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
                    if (Retrieve(Stream_Text, Pos, Text_ID)==ID && Retrieve(Stream_Text, Pos, "MuxingMode")==__T("A/53 / DTVCC Transport"))
                    {
                        StreamPos_Last=Pos;
                        break;
                    }

                if (StreamPos_Last==(size_t)-1)
                {
                    Stream_Prepare(Stream_Text, StreamPos_Last);
                    Fill(Stream_Text, StreamPos_Last, Text_Format, "EIA-608");
                    Fill(Stream_Text, StreamPos_Last, Text_StreamSize, 0);
                    Fill(Stream_Text, StreamPos_Last, Text_BitRate_Mode, "CBR");
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode", __T("A/53 / DTVCC Transport"));
                }

                if (!IsSub)
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode_MoreInfo", __T("Muxed in Video #")+Ztring().From_Number(Temp->StreamPos+1), true);
                Fill(Stream_Text, StreamPos_Last, Text_ID, ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, ID_String, true);
                Fill(Stream_Text, StreamPos_Last, General_StreamOrder, Retrieve(Stream_Video, Temp->StreamPos, General_StreamOrder), true);
                Fill(Stream_Text, StreamPos_Last, Text_MenuID, Retrieve(Stream_Video, Temp->StreamPos, Video_MenuID), true);
                Fill(Stream_Text, StreamPos_Last, Text_MenuID_String, Retrieve(Stream_Video, Temp->StreamPos, Video_MenuID_String), true);
                Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, Temp->StreamPos, Video_Duration), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Temp->StreamPos, Video_Delay), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay_Source, Retrieve(Stream_Video, Temp->StreamPos, Video_Delay_Source), true);
            }

            //Undetected text streams but present is service_descriptor - 708
            for (std::map<int8u, string>::iterator Language=Eia708_Languages.begin(); Language!=Eia708_Languages.end(); ++Language)
            {
                //TODO: merge both methods (see above)
                Ztring Parser_ID=Ztring::ToZtring(Language->first);
                Ztring ID=Retrieve(Stream_Video, Temp->StreamPos, Video_ID)+__T('-')+Parser_ID;
                Ztring ID_String=Retrieve(Stream_Video, Temp->StreamPos, Video_ID_String)+__T('-')+Parser_ID;
                StreamPos_Last=(size_t)-1;
                for (size_t Pos=0; Pos<Count_Get(Stream_Text); Pos++)
                    if (Retrieve(Stream_Text, Pos, Text_ID)==ID && Retrieve(Stream_Text, Pos, "MuxingMode")==__T("A/53 / DTVCC Transport"))
                    {
                        StreamPos_Last=Pos;
                        break;
                    }
                if (StreamPos_Last==(size_t)-1)
                {
                    Stream_Prepare(Stream_Text, StreamPos_Last);
                    Fill(Stream_Text, StreamPos_Last, Text_ID, Language->first); //TODO: put this in File_Eia708
                    Fill(Stream_Text, StreamPos_Last, Text_Format, "EIA-708");
                    Fill(Stream_Text, StreamPos_Last, Text_StreamSize, 0);
                    Fill(Stream_Text, StreamPos_Last, Text_BitRate_Mode, "CBR");
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode", __T("A/53 / DTVCC Transport"));
                }

                if (!IsSub)
                    Fill(Stream_Text, StreamPos_Last, "MuxingMode_MoreInfo", __T("Muxed in Video #")+Ztring().From_Number(Temp->StreamPos+1), true);
                Fill(Stream_Text, StreamPos_Last, Text_ID, ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, ID_String, true);
                Fill(Stream_Text, StreamPos_Last, General_StreamOrder, Retrieve(Stream_Video, Temp->StreamPos, General_StreamOrder), true);
                Fill(Stream_Text, StreamPos_Last, Text_MenuID, Retrieve(Stream_Video, Temp->StreamPos, Video_MenuID), true);
                Fill(Stream_Text, StreamPos_Last, Text_MenuID_String, Retrieve(Stream_Video, Temp->StreamPos, Video_MenuID_String), true);
                Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, Temp->StreamPos, Video_Duration), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Temp->StreamPos, Video_Delay), true);
                Fill(Stream_Text, StreamPos_Last, Text_Delay_Source, Retrieve(Stream_Video, Temp->StreamPos, Video_Delay_Source), true);

                //Language
                Fill(Stream_Text, StreamPos_Last, Text_Language, Language->second);
            }

            StreamKind_Last=Temp->StreamKind;
            StreamPos_Last=Temp->StreamPos;
        }
    }

    //Teletext
    if (StreamKind_Last==Stream_Max)
    {
        for (std::map<int16u, complete_stream::stream::teletext>::iterator Teletext=Temp->Teletexts.begin(); Teletext!=Temp->Teletexts.end(); ++Teletext)
        {
            Stream_Prepare(Stream_Text);
            Fill(StreamKind_Last, StreamPos_Last, General_ID, Ztring::ToZtring(StreamID)+__T('-')+Ztring::ToZtring(Teletext->first), true);
            Fill(StreamKind_Last, StreamPos_Last, General_ID_String, Decimal_Hexa(StreamID)+__T('-')+Ztring::ToZtring(Teletext->first), true);

            for (size_t Pos=0; Pos<Temp->program_numbers.size(); Pos++)
            {
                Fill(StreamKind_Last, StreamPos_Last, General_MenuID, Temp->program_numbers[Pos], 10, Pos==0);
                Fill(StreamKind_Last, StreamPos_Last, General_MenuID_String, Decimal_Hexa(Temp->program_numbers[Pos]), Pos==0);
            }

            //TS info
            for (std::map<std::string, ZenLib::Ztring>::iterator Info=Teletext->second.Infos.begin(); Info!=Teletext->second.Infos.end(); ++Info)
            {
                if (Retrieve(StreamKind_Last, StreamPos_Last, Info->first.c_str()).empty())
                    Fill(StreamKind_Last, StreamPos_Last, Info->first.c_str(), Info->second);
            }
            Teletext->second.Infos.clear();
        }
    }
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Update_EPG()
{
    //EPG
    complete_stream::transport_streams::iterator Transport_Stream=Complete_Stream->transport_stream_id_IsValid?Complete_Stream->Transport_Streams.find(Complete_Stream->transport_stream_id):Complete_Stream->Transport_Streams.end();
    if (Transport_Stream==Complete_Stream->Transport_Streams.end())
        return;

    //Per source (ATSC)
    if (Transport_Stream->second.source_id_IsValid)
    {
        complete_stream::sources::iterator Source=Complete_Stream->Sources.find(Transport_Stream->second.source_id);
        if (Source!=Complete_Stream->Sources.end())
        {
            //EPG
            std::map<Ztring, Ztring> EPGs;
            for (complete_stream::source::atsc_epg_blocks::iterator ATSC_EPG_Block=Source->second.ATSC_EPG_Blocks.begin(); ATSC_EPG_Block!=Source->second.ATSC_EPG_Blocks.end(); ++ATSC_EPG_Block)
                for (complete_stream::source::atsc_epg_block::events::iterator Event=ATSC_EPG_Block->second.Events.begin(); Event!=ATSC_EPG_Block->second.Events.end(); ++Event)
                {
                    Ztring Texts;
                    for (std::map<int16u, Ztring>::iterator text=Event->second.texts.begin(); text!=Event->second.texts.end(); ++text)
                        Texts+=text->second+__T(" - ");
                    if (!Texts.empty())
                        Texts.resize(Texts.size()-3);
                    EPGs[Ztring().Date_From_Seconds_1970(Event->second.start_time+315964800-Complete_Stream->GPS_UTC_offset)]=Event->second.title+__T(" / ")+Texts+__T(" /  /  / ")+Event->second.duration+__T(" / ");
                }
            if (!EPGs.empty())
            {
                //Trashing old EPG
                size_t Begin=Retrieve(Stream_General, 0, General_EPG_Positions_Begin).To_int32u();
                size_t End=Retrieve(Stream_General, 0, General_EPG_Positions_End).To_int32u();
                if (Begin && End && Begin<End)
                    for (size_t Pos=End-1; Pos>=Begin; Pos--)
                        Clear(Stream_General, 0, Pos);

                //Filling
                Fill(Stream_General, 0, General_EPG_Positions_Begin, Count_Get(Stream_General, 0), 10, true);
                for (std::map<Ztring, Ztring>::iterator EPG=EPGs.begin(); EPG!=EPGs.end(); ++EPG)
                    Fill(Stream_General, 0, EPG->first.To_Local().c_str(), EPG->second, true);
                Fill(Stream_General, 0, General_EPG_Positions_End, Count_Get(Stream_General, 0), 10, true);
            }
        }
    }

    //Per program
    if (!Transport_Stream->second.Programs.empty()
     && (Transport_Stream->second.Programs.size()>1
      || !Transport_Stream->second.Programs.begin()->second.Infos.empty()
      || !Transport_Stream->second.Programs.begin()->second.DVB_EPG_Blocks.empty()
      || Complete_Stream->Sources.find(Transport_Stream->second.Programs.begin()->second.source_id)!=Complete_Stream->Sources.end()
      || Config->File_MpegTs_ForceMenu_Get()))
        for (complete_stream::transport_stream::programs::iterator Program=Transport_Stream->second.Programs.begin(); Program!=Transport_Stream->second.Programs.end(); ++Program)
        {
            if (Program->second.IsParsed)
            {
                bool EPGs_IsUpdated=false;
                std::map<Ztring, Ztring> EPGs;

                //EPG - DVB
                if (Program->second.DVB_EPG_Blocks_IsUpdated)
                {
                    for (complete_stream::transport_stream::program::dvb_epg_blocks::iterator DVB_EPG_Block=Program->second.DVB_EPG_Blocks.begin(); DVB_EPG_Block!=Program->second.DVB_EPG_Blocks.end(); ++DVB_EPG_Block)
                        for (complete_stream::transport_stream::program::dvb_epg_block::events::iterator Event=DVB_EPG_Block->second.Events.begin(); Event!=DVB_EPG_Block->second.Events.end(); ++Event)
                            if (EPGs.find(Event->second.start_time)==EPGs.end() || DVB_EPG_Block->first==0x4E) //Does not exist or "DVB - event_information_section - actual_transport_stream : return present/following"
                                EPGs[Event->second.start_time]=Event->second.short_event.event_name+__T(" / ")+Event->second.short_event.text+__T(" / ")+Event->second.content+__T(" /  / ")+Event->second.duration+__T(" / ")+Event->second.running_status;
                    Program->second.DVB_EPG_Blocks_IsUpdated=false;
                    EPGs_IsUpdated=true;
                }

                //EPG - ATSC
                if (Program->second.source_id_IsValid)
                {
                    complete_stream::sources::iterator Source=Complete_Stream->Sources.find(Program->second.source_id);
                    if (Source!=Complete_Stream->Sources.end())
                    {
                        if (!Source->second.texts.empty())
                        {
                            Ztring Texts;
                            for (std::map<int16u, Ztring>::iterator text=Source->second.texts.begin(); text!=Source->second.texts.end(); ++text)
                                Texts+=text->second+__T(" - ");
                            if (!Texts.empty())
                                Texts.resize(Texts.size()-3);
                            if (Program->second.StreamPos==(size_t)-1)
                            {
                                Complete_Stream->program_number_Order.push_back(Program->first);
                                Stream_Prepare(Stream_Menu);
                                Program->second.StreamPos=StreamPos_Last;
                            }
                            Fill(Stream_Menu, Program->second.StreamPos, Menu_ServiceProvider, Texts, true);
                        }
                        if (Source->second.ATSC_EPG_Blocks_IsUpdated)
                        {
                            for (complete_stream::source::atsc_epg_blocks::iterator ATSC_EPG_Block=Source->second.ATSC_EPG_Blocks.begin(); ATSC_EPG_Block!=Source->second.ATSC_EPG_Blocks.end(); ++ATSC_EPG_Block)
                                for (complete_stream::source::atsc_epg_block::events::iterator Event=ATSC_EPG_Block->second.Events.begin(); Event!=ATSC_EPG_Block->second.Events.end(); ++Event)
                                    if (Event->second.start_time!=(int32u)-1) //TODO: find the reason when start_time is not set
                                    {
                                        Ztring Texts;
                                        for (std::map<int16u, Ztring>::iterator text=Event->second.texts.begin(); text!=Event->second.texts.end(); ++text)
                                            Texts+=text->second+__T(" - ");
                                        if (!Texts.empty())
                                            Texts.resize(Texts.size()-3);
                                        EPGs[Ztring().Date_From_Seconds_1970(Event->second.start_time+315964800-Complete_Stream->GPS_UTC_offset)]=Event->second.title+__T(" / ")+Texts+__T(" /  /  / ")+Event->second.duration+__T(" / ");
                                    }
                            Source->second.ATSC_EPG_Blocks_IsUpdated=false;
                            EPGs_IsUpdated=true;
                        }
                    }
                }

                //EPG - Filling
                if (EPGs_IsUpdated)
                {
                    if (Program->second.StreamPos==(size_t)-1)
                    {
                        Complete_Stream->program_number_Order.push_back(Program->first);
                        Stream_Prepare(Stream_Menu);
                        Program->second.StreamPos=StreamPos_Last;
                    }

                    Program->second.EPGs=EPGs;
                    Streams_Update_EPG_PerProgram(Program);
                }
            }
        }

    Complete_Stream->Sources_IsUpdated=false;
    Complete_Stream->Programs_IsUpdated=false;
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Update_EPG_PerProgram(complete_stream::transport_stream::programs::iterator Program)
{
    size_t Chapters_Pos_Begin=Retrieve(Stream_Menu, Program->second.StreamPos, Menu_Chapters_Pos_Begin).To_int32u();
    size_t Chapters_Pos_End=Retrieve(Stream_Menu, Program->second.StreamPos, Menu_Chapters_Pos_End).To_int32u();
    if (Chapters_Pos_Begin && Chapters_Pos_End)
    {
        for (size_t Pos=Chapters_Pos_End-1; Pos>=Chapters_Pos_Begin; Pos--)
            Clear(Stream_Menu, Program->second.StreamPos, Pos);
        Clear(Stream_Menu, Program->second.StreamPos, Menu_Chapters_Pos_Begin);
        Clear(Stream_Menu, Program->second.StreamPos, Menu_Chapters_Pos_End);
    }
    if (!Program->second.EPGs.empty())
    {
        Fill(Stream_Menu, Program->second.StreamPos, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, Program->second.StreamPos), 10, true);
        for (std::map<Ztring, Ztring>::iterator EPG=Program->second.EPGs.begin(); EPG!=Program->second.EPGs.end(); ++EPG)
            Fill(Stream_Menu, Program->second.StreamPos, EPG->first.To_UTF8().c_str(), EPG->second, true);
        Fill(Stream_Menu, Program->second.StreamPos, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, Program->second.StreamPos), 10, true);
    }
}

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_MPEGTS_PCR_YES
void File_MpegTs::Streams_Update_Duration_Update()
{
    bool IsVbr=false;
    bool IsCbr=false;
    #if MEDIAINFO_ADVANCED
        float64 TimeStamp_InstantaneousBitRate_Min_Raw=DBL_MAX;
        float64 TimeStamp_InstantaneousBitRate_Max_Raw=0;
        int64u  TimeStamp_Distance_Min=(int64u)-1;
        int64u  TimeStamp_Distance_Max=0;
        int64u  TimeStamp_Distance_Total=0;
        int64u  TimeStamp_Distance_Count=0;
        int64u  TimeStamp_HasProblems=0;
    #endif // MEDIAINFO_ADVANCED

    for (std::map<int16u, int16u>::iterator PCR_PID=Complete_Stream->PCR_PIDs.begin(); PCR_PID!=Complete_Stream->PCR_PIDs.end(); ++PCR_PID)
    {
        complete_stream::streams::iterator Stream=Complete_Stream->Streams.begin()+PCR_PID->first;
        if (*Stream && (*Stream)->TimeStamp_End_IsUpdated)
        {
            if ((*Stream)->TimeStamp_End<0x100000000LL*300 && (*Stream)->TimeStamp_Start>0x100000000LL*300)
                (*Stream)->TimeStamp_End+=0x200000000LL*300; //33 bits, cyclic
            if ((*Stream)->TimeStamp_Start<(*Stream)->TimeStamp_End)
            {
                int64u  Duration=0;
                int64u  Bytes=0;
                #if MEDIAINFO_ADVANCED
                    if (Config->ParseSpeed>=1 && !(*Stream)->TimeStamp_Intermediate.empty())
                    {
                        Duration=(*Stream)->TimeStamp_Intermediate[0]-(*Stream)->TimeStamp_Start;
                        size_t Last=(*Stream)->TimeStamp_Intermediate.size()-1;
                        for (size_t Pos=1; Pos+1<Last; Pos+=2)
                            Duration+=(*Stream)->TimeStamp_Intermediate[Pos+1]-(*Stream)->TimeStamp_Intermediate[Pos];
                        Duration+=(*Stream)->TimeStamp_End-(*Stream)->TimeStamp_Intermediate[Last];
                    }
                    else
                #endif // MEDIAINFO_ADVANCED
                {
                    Duration=(*Stream)->TimeStamp_End-(*Stream)->TimeStamp_Start;
                }
                Bytes=(*Stream)->TimeStamp_End_Offset-(*Stream)->TimeStamp_Start_Offset;

                if (Duration && Bytes)
                {
                    Fill(Stream_General, 0, General_Duration, ((float64)Duration)/27000, 6, true);
                    Fill(Stream_General, 0, General_OverallBitRate, Bytes*8/(((float64)Duration)/27000000), 0, true);
                }

                (*Stream)->TimeStamp_End_IsUpdated=false;
                (*Stream)->IsPCR_Duration=(float64)Duration;

                //Filling menu duration
                if (Count_Get(Stream_Menu))
                {
                    complete_stream::transport_streams::iterator Transport_Stream=Complete_Stream->transport_stream_id_IsValid?Complete_Stream->Transport_Streams.find(Complete_Stream->transport_stream_id):Complete_Stream->Transport_Streams.end();
                    if (Transport_Stream!=Complete_Stream->Transport_Streams.end())
                    {
                        //Per program
                        for (size_t Pos=0; Pos<(*Stream)->program_numbers.size(); Pos++)
                        {
                            int16u program_number=(*Stream)->program_numbers[Pos];
                            if (Transport_Stream->second.Programs[program_number].IsRegistered) //Only if the menu is already displayed
                                Fill(Stream_Menu, Transport_Stream->second.Programs[program_number].StreamPos, Menu_Duration, ((float64)Duration)/27000, 6, true);
                        }
                    }
                }
            }

            if ((*Stream)->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr>=Config_VbrDetection_Occurences)
                IsVbr=true;
            if ((*Stream)->TimeStamp_InstantaneousBitRate_BitRateMode_IsCbr)
                IsCbr=true;
            #if MEDIAINFO_ADVANCED
                if (Config->ParseSpeed>=1)
                {
                    if (TimeStamp_InstantaneousBitRate_Min_Raw>(*Stream)->TimeStamp_InstantaneousBitRate_Min_Raw)
                        TimeStamp_InstantaneousBitRate_Min_Raw=(*Stream)->TimeStamp_InstantaneousBitRate_Min_Raw;
                    if (TimeStamp_InstantaneousBitRate_Max_Raw<(*Stream)->TimeStamp_InstantaneousBitRate_Max_Raw)
                        TimeStamp_InstantaneousBitRate_Max_Raw=(*Stream)->TimeStamp_InstantaneousBitRate_Max_Raw;
                    TimeStamp_Distance_Total+=(*Stream)->TimeStamp_Distance_Total;
                    TimeStamp_Distance_Count+=(*Stream)->TimeStamp_Distance_Count;
                    if (TimeStamp_Distance_Min>(*Stream)->TimeStamp_Distance_Min)
                        TimeStamp_Distance_Min=(*Stream)->TimeStamp_Distance_Min;
                    if (TimeStamp_Distance_Max<(*Stream)->TimeStamp_Distance_Max)
                        TimeStamp_Distance_Max=(*Stream)->TimeStamp_Distance_Max;
                    TimeStamp_HasProblems+=(*Stream)->TimeStamp_HasProblems;
                }
            #endif // MEDIAINFO_ADVANCED
        }
    }

    if (IsVbr)
        Fill(Stream_General, 0, General_OverallBitRate_Mode, "VBR", Unlimited, true, true);
    else if (IsCbr)
        Fill(Stream_General, 0, General_OverallBitRate_Mode, "CBR", Unlimited, true, true);
    else
        Clear(Stream_General, 0, General_OverallBitRate_Mode);
    #if MEDIAINFO_ADVANCED
        if (Config->ParseSpeed>=1)
        {
            if ((IsVbr || !IsCbr) && TimeStamp_InstantaneousBitRate_Min_Raw<DBL_MAX)
                Fill(Stream_General, 0, General_OverallBitRate_Minimum, TimeStamp_InstantaneousBitRate_Min_Raw, 0, true);
            else
                Clear(Stream_General, 0, General_OverallBitRate_Minimum);
            if ((IsVbr || !IsCbr) && TimeStamp_InstantaneousBitRate_Max_Raw)
                Fill(Stream_General, 0, General_OverallBitRate_Maximum, TimeStamp_InstantaneousBitRate_Max_Raw, 0, true);
            else
                Clear(Stream_General, 0, General_OverallBitRate_Maximum);
            if (TimeStamp_Distance_Count)
            {
                Fill(Stream_General, 0, "PCR_Distance_Average", ((float64)TimeStamp_Distance_Total)/27000000/TimeStamp_Distance_Count, 9, true);
                (*Stream_More)[Stream_General][0](Ztring().From_Local("PCR_Distance_Average"), Info_Options)=__T("N NT");
            }
            if (TimeStamp_Distance_Min!=(int64u)-1)
            {
                Fill(Stream_General, 0, "PCR_Distance_Min", ((float64)TimeStamp_Distance_Min)/27000000, 9, true);
                (*Stream_More)[Stream_General][0](Ztring().From_Local("PCR_Distance_Min"), Info_Options)=__T("N NT");
            }
            if (TimeStamp_Distance_Max)
            {
                Fill(Stream_General, 0, "PCR_Distance_Max", ((float64)TimeStamp_Distance_Max)/27000000, 9, true);
                (*Stream_More)[Stream_General][0](Ztring().From_Local("PCR_Distance_Max"), Info_Options)=__T("N NT");
            }
            {
                Fill(Stream_General, 0, "PCR_Invalid_Count", TimeStamp_HasProblems, 10, true);
                (*Stream_More)[Stream_General][0](Ztring().From_Local("PCR_Invalid_Count"), Info_Options)=__T("N NT");
            }

        }
    #endif // MEDIAINFO_ADVANCED
}
#endif //MEDIAINFO_MPEGTS_PCR_YES

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Update_Duration_End()
{
    //General
    Fill(Stream_General, 0, General_Duration_End, Complete_Stream->Duration_End, true);

    Complete_Stream->Duration_End_IsUpdated=false;
}

//---------------------------------------------------------------------------
void File_MpegTs::Streams_Finish()
{
    //Per stream
    for (size_t StreamID=0; StreamID<0x2000; StreamID++)
        if (Complete_Stream->Streams[StreamID]->Parser)
        {
            if (!Complete_Stream->Streams[StreamID]->Parser->Status[IsFinished])
            {
                int64u File_Size_Temp=File_Size;
                File_Size=File_Offset+Buffer_Offset+Element_Offset;
                Open_Buffer_Continue(Complete_Stream->Streams[StreamID]->Parser, Buffer, 0, false);
                File_Size=File_Size_Temp;
                Finish(Complete_Stream->Streams[StreamID]->Parser);
                #if MEDIAINFO_DEMUX
                    if (Config->Demux_EventWasSent)
                        return;
                #endif //MEDIAINFO_DEMUX
            }
        }

    #if MEDIAINFO_DUPLICATE
        File__Duplicate_Streams_Finish();
    #endif //MEDIAINFO_DUPLICATE

    #if MEDIAINFO_IBI
        if (!IsSub && Config_Ibi_Create)
        {
            for (ibi::streams::iterator IbiStream_Temp=Ibi.Streams.begin(); IbiStream_Temp!=Ibi.Streams.end(); ++IbiStream_Temp)
            {
                if (IbiStream_Temp->second && IbiStream_Temp->second->DtsFrequencyNumerator==1000000000 && IbiStream_Temp->second->DtsFrequencyDenominator==1)
                {
                    bool IsOk=true;
                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                        if (!IbiStream_Temp->second->Infos[Pos].IsContinuous && Pos+1!=IbiStream_Temp->second->Infos.size())
                            IsOk=false;
                    if (IsOk) //Only is all items are continuous (partial IBI not yet supported)
                    {
                        IbiStream_Temp->second->DtsFrequencyNumerator=90000;
                        for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                        {
                            int64u Temp=IbiStream_Temp->second->Infos[Pos].Dts*90/1000000;
                            IbiStream_Temp->second->Infos[Pos].Dts=Temp;
                        }
                    }
                }
            }
        }
    #endif //MEDIAINFO_IBI
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_MpegTs::Synchronize()
{
    //Synchronizing
    while (       Buffer_Offset+188*16+BDAV_Size*16+TSP_Size*16<=Buffer_Size
      && !(Buffer[Buffer_Offset+188* 0+BDAV_Size* 1+TSP_Size* 0]==0x47
        && Buffer[Buffer_Offset+188* 1+BDAV_Size* 2+TSP_Size* 1]==0x47
        && Buffer[Buffer_Offset+188* 2+BDAV_Size* 3+TSP_Size* 2]==0x47
        && Buffer[Buffer_Offset+188* 3+BDAV_Size* 4+TSP_Size* 3]==0x47
        && Buffer[Buffer_Offset+188* 4+BDAV_Size* 5+TSP_Size* 4]==0x47
        && Buffer[Buffer_Offset+188* 5+BDAV_Size* 6+TSP_Size* 5]==0x47
        && Buffer[Buffer_Offset+188* 6+BDAV_Size* 7+TSP_Size* 6]==0x47
        && Buffer[Buffer_Offset+188* 7+BDAV_Size* 8+TSP_Size* 7]==0x47
        && Buffer[Buffer_Offset+188* 8+BDAV_Size* 9+TSP_Size* 8]==0x47
        && Buffer[Buffer_Offset+188* 9+BDAV_Size*10+TSP_Size* 9]==0x47
        && Buffer[Buffer_Offset+188*10+BDAV_Size*11+TSP_Size*10]==0x47
        && Buffer[Buffer_Offset+188*11+BDAV_Size*12+TSP_Size*11]==0x47
        && Buffer[Buffer_Offset+188*12+BDAV_Size*13+TSP_Size*12]==0x47
        && Buffer[Buffer_Offset+188*13+BDAV_Size*14+TSP_Size*13]==0x47
        && Buffer[Buffer_Offset+188*14+BDAV_Size*15+TSP_Size*14]==0x47
        && Buffer[Buffer_Offset+188*15+BDAV_Size*16+TSP_Size*15]==0x47))
    {
        Buffer_Offset++;
        while (       Buffer_Offset+BDAV_Size+1<=Buffer_Size
            && Buffer[Buffer_Offset+BDAV_Size]!=0x47)
            Buffer_Offset++;
    }

    if (Buffer_Offset+188*16+BDAV_Size*16+TSP_Size*16>=Buffer_Size)
        return false;

    //Synched is OK
    if (!Status[IsAccepted])
    {
        Accept();
    }
    return true;
}

//---------------------------------------------------------------------------
bool File_MpegTs::Synched_Test()
{
    while (Buffer_Offset+TS_Size<=Buffer_Size)
    {
        //Synchro testing
        if (Buffer[Buffer_Offset+BDAV_Size]!=0x47)
        {
            Synched=false;
            #if MEDIAINFO_DUPLICATE
                if (File__Duplicate_Get())
                    Trusted++; //We don't want to stop parsing if duplication is requested, TS is not a lot stable, normal...
            #endif //MEDIAINFO_DUPLICATE
            return true;
        }

        //Getting pid
        pid=(Buffer[Buffer_Offset+BDAV_Size+1]&0x1F)<<8
          |  Buffer[Buffer_Offset+BDAV_Size+2];

        complete_stream::stream* Stream=Complete_Stream->Streams[pid];
        if (Stream->Searching)
        {
            //Trace config
            #if MEDIAINFO_TRACE
                if (Config_Trace_Level)
                {
                    if (Stream->Kind==complete_stream::stream::pes)
                    {
                        if (!Trace_Layers[8])
                            Trace_Layers_Update(8); //Stream
                    }
                    else
                        Trace_Layers_Update(IsSub?1:0);
                }
            #endif //MEDIAINFO_TRACE

            payload_unit_start_indicator=(Buffer[Buffer_Offset+BDAV_Size+1]&0x40)!=0;
            if (payload_unit_start_indicator)
            {
                //Searching start
                if (Stream->Searching_Payload_Start) //payload_unit_start_indicator
                {
                    if (Stream->Kind==complete_stream::stream::psi)
                    {
                        //Searching table_id
                        size_t Version_Pos=BDAV_Size
                                          +4 //standart header
                                          +((Buffer[Buffer_Offset+BDAV_Size+3]&0x20)?(1+Buffer[Buffer_Offset+BDAV_Size+4]):0); //adaptation_field_control (adaptation) --> adaptation_field_length
                        if (Version_Pos>=BDAV_Size+188)
                            return true; //There is a problem with this block, accelerated parsing disabled
                        Version_Pos+=1+Buffer[Buffer_Offset+Version_Pos]; //pointer_field
                        if (Version_Pos>=BDAV_Size+188)
                            return true; //There is a problem with this block, accelerated parsing disabled
                        int8u table_id=Buffer[Buffer_Offset+Version_Pos]; //table_id
                        #if MEDIAINFO_TRACE
                            if (Trace_Activated)
                                Stream->Element_Info1=Mpeg_Psi_table_id(table_id);
                        #endif //MEDIAINFO_TRACE
                        if (table_id==0xCD) //specifc case for ATSC STT
                        {
                            if (Config_Trace_TimeSection_OnlyFirstOccurrence)
                            {
                                if (!TimeSection_FirstOccurrenceParsed)
                                    TimeSection_FirstOccurrenceParsed=true;
                                #if MEDIAINFO_TRACE
                                else
                                {
                                    Trace_Layers.reset(); //We do not want to display data about other occurences
                                    Trace_Layers_Update();
                                }
                                #endif //MEDIAINFO_TRACE
                            }
                            return true; //Version has no meaning
                        }
                        #if MEDIAINFO_IBI
                            if (table_id==0x00)
                                Complete_Stream->Streams[pid]->Ibi_SynchronizationOffset_BeginOfFrame=File_Offset+Buffer_Offset;
                            if (table_id==0x02)
                                Complete_Stream->Streams[pid]->Ibi_SynchronizationOffset_BeginOfFrame=Complete_Stream->Streams[0x0000]->Ibi_SynchronizationOffset_BeginOfFrame;
                        #endif //MEDIAINFO_IBI
                        complete_stream::stream::table_ids::iterator Table_ID=Stream->Table_IDs.begin()+table_id;
                        if (*Table_ID)
                        {
                            //Searching table_id_extension, version_number, section_number
                            if (!(Buffer[Buffer_Offset+Version_Pos+1]&0x80)) //section_syntax_indicator
                            {
                                if (table_id==0x70 && Config_Trace_TimeSection_OnlyFirstOccurrence)
                                {
                                    if (!TimeSection_FirstOccurrenceParsed)
                                        TimeSection_FirstOccurrenceParsed=true;
                                    #if MEDIAINFO_TRACE
                                    else
                                    {
                                        Trace_Layers.reset(); //We do not want to display data about other occurences
                                        Trace_Layers_Update();
                                    }
                                    #endif //MEDIAINFO_TRACE
                                }
                                return true; //No version
                            }
                            Version_Pos+=3; //Header size
                            if (Version_Pos+5>=BDAV_Size+188)
                                return true; //Not able to detect version (too far)
                            int16u table_id_extension=(Buffer[Buffer_Offset+Version_Pos]<<8)|Buffer[Buffer_Offset+Version_Pos+1];
                            int8u  version_number=(Buffer[Buffer_Offset+Version_Pos+2]&0x3F)>>1;
                            int8u  section_number=Buffer[Buffer_Offset+Version_Pos+3];
                            complete_stream::stream::table_id::table_id_extensions::iterator Table_ID_Extension=(*Table_ID)->Table_ID_Extensions.find(table_id_extension);
                            if (Table_ID_Extension==(*Table_ID)->Table_ID_Extensions.end())
                            {
                                if ((*Table_ID)->Table_ID_Extensions_CanAdd)
                                {
                                    (*Table_ID)->Table_ID_Extensions[table_id_extension].version_number=version_number;
                                    (*Table_ID)->Table_ID_Extensions[table_id_extension].Section_Numbers.resize(0x100);
                                    (*Table_ID)->Table_ID_Extensions[table_id_extension].Section_Numbers[section_number]=true;
                                    return true; //table_id_extension is not yet parsed
                                }
                            }
                            else if (Table_ID_Extension->second.version_number!=version_number)
                            {
                                if (Table_ID_Extension->second.version_number!=(int8u)-1 && Config_Trace_TimeSection_OnlyFirstOccurrence)
                                    break;
                                Table_ID_Extension->second.version_number=version_number;
                                Table_ID_Extension->second.Section_Numbers.clear();
                                Table_ID_Extension->second.Section_Numbers.resize(0x100);
                                Table_ID_Extension->second.Section_Numbers[section_number]=true;
                                return true; //version is different
                            }
                            else if (!Table_ID_Extension->second.Section_Numbers[section_number])
                            {
                                Table_ID_Extension->second.Section_Numbers[section_number]=true;
                                return true; //section is not yet parsed
                            }
                            else if (table_id==0x02 && Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs_NotParsedCount)
                            {
                                //PMT is looping, so this is nearly sure that all PMT available in the stream are assigned
                                #ifdef MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                                    for (size_t pid=0x10; pid<0x1FFF; pid++) //Wanting 0x10-->0x2F (DVB), 0x1ABC (cea_osd), 0x1FF7-->0x1FFF (ATSC)
                                        for (size_t Table_ID=0x00; Table_ID<0xFF; Table_ID++)
                                        {
                                            Complete_Stream->Streams[pid]->Searching_Payload_Start_Set(true);
                                            Complete_Stream->Streams[pid]->Kind=complete_stream::stream::psi;
                                            Complete_Stream->Streams[pid]->Table_IDs.resize(0x100);
                                            Complete_Stream->Streams[pid]->Table_IDs[Table_ID]=new complete_stream::stream::table_id; //event_information_section - actual_transport_stream, schedule

                                            if (Pos==0x001F)
                                                Pos=0x1ABB; //Skipping normal data
                                            if (Pos==0x01ABC)
                                                Pos=0x1FF6; //Skipping normal data
                                        }
                                #else //MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                                    if (Complete_Stream->Streams[0x0010]->Kind==complete_stream::stream::unknown)
                                    {
                                        Complete_Stream->Streams[0x0010]->Searching_Payload_Start_Set(true);
                                        Complete_Stream->Streams[0x0010]->Kind=complete_stream::stream::psi;
                                        Complete_Stream->Streams[0x0010]->Table_IDs.resize(0x100);
                                        Complete_Stream->Streams[0x0010]->Table_IDs[0x40]=new complete_stream::stream::table_id; //network_information_section - actual_network
                                    }
                                    if (Complete_Stream->Streams[0x0011]->Kind==complete_stream::stream::unknown)
                                    {
                                        Complete_Stream->Streams[0x0011]->Searching_Payload_Start_Set(true);
                                        Complete_Stream->Streams[0x0011]->Kind=complete_stream::stream::psi;
                                        Complete_Stream->Streams[0x0011]->Table_IDs.resize(0x100);
                                        Complete_Stream->Streams[0x0011]->Table_IDs[0x42]=new complete_stream::stream::table_id; //service_description_section - actual_transport_stream
                                    }
                                    if (Complete_Stream->Streams[0x0012]->Kind==complete_stream::stream::unknown)
                                    {
                                        Complete_Stream->Streams[0x0012]->Searching_Payload_Start_Set(true);
                                        Complete_Stream->Streams[0x0012]->Kind=complete_stream::stream::psi;
                                        Complete_Stream->Streams[0x0012]->Table_IDs.resize(0x100);
                                        Complete_Stream->Streams[0x0012]->Table_IDs[0x4E]=new complete_stream::stream::table_id; //event_information_section - actual_transport_stream, present/following
                                        for (size_t Table_ID=0x50; Table_ID<0x60; Table_ID++)
                                            Complete_Stream->Streams[0x0012]->Table_IDs[Table_ID]=new complete_stream::stream::table_id; //event_information_section - actual_transport_stream, schedule
                                    }
                                    if (Complete_Stream->Streams[0x0014]->Kind==complete_stream::stream::unknown)
                                    {
                                        Complete_Stream->Streams[0x0014]->Searching_Payload_Start_Set(true);
                                        Complete_Stream->Streams[0x0014]->Kind=complete_stream::stream::psi;
                                        Complete_Stream->Streams[0x0014]->Table_IDs.resize(0x100);
                                        Complete_Stream->Streams[0x0014]->Table_IDs[0x70]=new complete_stream::stream::table_id; //time_date_section
                                        Complete_Stream->Streams[0x0014]->Table_IDs[0x73]=new complete_stream::stream::table_id; //time_offset_section
                                    }
                                    if (Complete_Stream->Streams[0x1FFB]->Kind==complete_stream::stream::unknown)
                                    {
                                        Complete_Stream->Streams[0x1FFB]->Searching_Payload_Start_Set(true);
                                        Complete_Stream->Streams[0x1FFB]->Kind=complete_stream::stream::psi;
                                        Complete_Stream->Streams[0x1FFB]->Table_IDs.resize(0x100);
                                        Complete_Stream->Streams[0x1FFB]->Table_IDs[0xC7]=new complete_stream::stream::table_id; //Master Guide Table
                                        Complete_Stream->Streams[0x1FFB]->Table_IDs[0xCD]=new complete_stream::stream::table_id; //System Time Table
                                    }
                                #endif //MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                            }
                        }
                    }
                    else
                        return true; //No version in this pid
                }
            }

            //Searching continue and parser timestamp
            if (Stream->Searching_Payload_Continue
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                 || Stream->Searching_ParserTimeStamp_Start
                 || Stream->Searching_ParserTimeStamp_End
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
             )
                return true;

            //Adaptation layer
            #ifdef MEDIAINFO_MPEGTS_PCR_YES
                if (( Stream->Searching_TimeStamp_Start
                  ||  Stream->Searching_TimeStamp_End)
                #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                 &&  !Stream->Searching_ParserTimeStamp_End
                #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                  )
                {
                    if ((Buffer[Buffer_Offset+BDAV_Size+3]&0x20)==0x20) //adaptation_field_control (adaptation)
                    {
                        int8u adaptation_field_length=Buffer[Buffer_Offset+BDAV_Size+4];
                        if (adaptation_field_length>=5) //adaptation_field_length
                        {
                            bool discontinuity_indicator=(Buffer[Buffer_Offset+BDAV_Size+5]&0x80)!=0;
                            bool PCR_flag=(Buffer[Buffer_Offset+BDAV_Size+5]&0x10)!=0;
                            if (PCR_flag)
                            {
                                int64u program_clock_reference=(  (((int64u)Buffer[Buffer_Offset+BDAV_Size+6])<<25)
                                                                | (((int64u)Buffer[Buffer_Offset+BDAV_Size+7])<<17)
                                                                | (((int64u)Buffer[Buffer_Offset+BDAV_Size+8])<< 9)
                                                                | (((int64u)Buffer[Buffer_Offset+BDAV_Size+9])<< 1)
                                                                | (((int64u)Buffer[Buffer_Offset+BDAV_Size+10])>>7));
                                program_clock_reference*=300;
                                program_clock_reference+=(  (((int64u)Buffer[Buffer_Offset+BDAV_Size+10]&0x01)<<8)
                                                          | (((int64u)Buffer[Buffer_Offset+BDAV_Size+11])   ));
                                if (Complete_Stream->Streams[pid]->Searching_TimeStamp_End
                                #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                                 && (!Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End
                                  || Complete_Stream->Streams[pid]->IsPCR) //If PCR, we always want it.
                                #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                                )
                                {
                                    Header_Parse_Events_Duration(program_clock_reference);
                                    if (program_clock_reference!=Complete_Stream->Streams[pid]->TimeStamp_End) //Some PCRs are buggy (low precision), using the first stream offset in the case of duplicate PCR value
                                    {
                                        if (Complete_Stream->Streams[pid]->TimeStamp_End_Offset!=(int64u)-1)
                                        {
                                            if (program_clock_reference+0x12c00000000LL<Complete_Stream->Streams[pid]->TimeStamp_End)
                                                program_clock_reference+=0x25800000000LL; //33 bits and *300
                                            if (!discontinuity_indicator && program_clock_reference>Complete_Stream->Streams[pid]->TimeStamp_End && program_clock_reference<Complete_Stream->Streams[pid]->TimeStamp_End+10*27000000) //Not before, not after 10 seconds, else there is a problem
                                            {
                                                float64 Duration_InstantaneousBitRate_Min=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End-(Config_VbrDetection_Delta?0:810));
                                                float64 Duration_InstantaneousBitRate_Max=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End+(Config_VbrDetection_Delta?0:810));
                                                float64 Bytes_InstantaneousBitRate=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);
                                                float64 TimeStamp_InstantaneousBitRate_Current_Min=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Max*27000000*(1-Config_VbrDetection_Delta);
                                                float64 TimeStamp_InstantaneousBitRate_Current_Raw=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Min*27000000;
                                                float64 TimeStamp_InstantaneousBitRate_Current_Max=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Min*27000000*(1+Config_VbrDetection_Delta);
                                                if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min)
                                                {
                                                    if (TimeStamp_InstantaneousBitRate_Current_Max<Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min || TimeStamp_InstantaneousBitRate_Current_Min>Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Max)
                                                    {
                                                        Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr++;
                                                        #if MEDIAINFO_ADVANCED
                                                            if (Config_VbrDetection_GiveUp && Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr>=Config_VbrDetection_Occurences)
                                                                Config->ParseSpeed=0;
                                                        #endif // MEDIAINFO_ADVANCED
                                                    }
                                                    else
                                                        Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsCbr++;
                                                }
                                                float64 Duration_Min=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End-1);
                                                float64 Duration_Raw=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End);
                                                float64 Duration_Max=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End+1);
                                                float64 Bytes=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);
                                                float64 TimeStamp_Min=Bytes*8/Duration_Max*27000000*(1-Config_VbrDetection_Delta);
                                                float64 TimeStamp_Raw=Bytes*8/Duration_Raw*27000000;
                                                float64 TimeStamp_Max=Bytes*8/Duration_Min*27000000*(1+Config_VbrDetection_Delta);
                                                Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min=TimeStamp_Min;
                                                Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw=TimeStamp_Raw;
                                                Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Max=TimeStamp_Max;
                                                #if MEDIAINFO_ADVANCED
                                                    if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Min_Raw>TimeStamp_InstantaneousBitRate_Current_Raw)
                                                        Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Min_Raw=TimeStamp_InstantaneousBitRate_Current_Raw;
                                                    if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Max_Raw<TimeStamp_InstantaneousBitRate_Current_Raw)
                                                        Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Max_Raw=TimeStamp_InstantaneousBitRate_Current_Raw;
                                                    int64u Distance=program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End;
                                                    if (Complete_Stream->Streams[pid]->TimeStamp_Distance_Min>Distance)
                                                        Complete_Stream->Streams[pid]->TimeStamp_Distance_Min=Distance;
                                                    if (Complete_Stream->Streams[pid]->TimeStamp_Distance_Max<Distance)
                                                        Complete_Stream->Streams[pid]->TimeStamp_Distance_Max=Distance;
                                                    Complete_Stream->Streams[pid]->TimeStamp_Distance_Total+=Distance;
                                                    Complete_Stream->Streams[pid]->TimeStamp_Distance_Count++;
                                                #endif // MEDIAINFO_ADVANCED
                                            }
                                            #if MEDIAINFO_ADVANCED
                                                else
                                                {
                                                   if (!discontinuity_indicator)
                                                        Complete_Stream->Streams[pid]->TimeStamp_HasProblems++;
                                                   float64 Bytes=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);;
                                                   int64u TimeToAdd;
                                                   if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw)
                                                       TimeToAdd=float64_int64s(Bytes*8/Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw*27000000);
                                                   else
                                                       TimeToAdd=0;
                                                   Complete_Stream->Streams[pid]->TimeStamp_Intermediate.push_back(Complete_Stream->Streams[pid]->TimeStamp_End+TimeToAdd);
                                                   Complete_Stream->Streams[pid]->TimeStamp_Intermediate.push_back(program_clock_reference);
                                                }
                                            #endif // MEDIAINFO_ADVANCED
                                        }
                                        Complete_Stream->Streams[pid]->TimeStamp_End=program_clock_reference;
                                        Complete_Stream->Streams[pid]->TimeStamp_End_IsUpdated=true;
                                        Complete_Stream->Streams[pid]->TimeStamp_End_Offset=File_Offset+Buffer_Offset;
                                        {
                                            Status[IsUpdated]=true;
                                            Status[User_16]=true;
                                        }
                                    }
                                }
                                if (Complete_Stream->Streams[pid]->Searching_TimeStamp_Start)
                                {
                                    //This is the first PCR
                                    Complete_Stream->Streams[pid]->TimeStamp_Start=program_clock_reference;
                                    Complete_Stream->Streams[pid]->TimeStamp_Start_Offset=File_Offset+Buffer_Offset;
                                    Complete_Stream->Streams[pid]->TimeStamp_End=program_clock_reference;
                                    Complete_Stream->Streams[pid]->TimeStamp_End_IsUpdated=true;
                                    Complete_Stream->Streams[pid]->TimeStamp_End_Offset=File_Offset+Buffer_Offset;
                                    Complete_Stream->Streams[pid]->Searching_TimeStamp_Start_Set(false);
                                    Complete_Stream->Streams[pid]->Searching_TimeStamp_End_Set(true);
                                    Complete_Stream->Streams_With_StartTimeStampCount++;
                                    {
                                        Status[IsUpdated]=true;
                                        Status[User_16]=true;
                                    }
                                }

                                //Test if we can find the TS bitrate
                                if (!Complete_Stream->Streams[pid]->EndTimeStampMoreThanxSeconds && Complete_Stream->Streams[pid]->TimeStamp_Start!=(int64u)-1
                                && (File_Offset+Buffer_Offset-Buffer_TotalBytes_FirstSynched)*2<File_Size)
                                {
                                    if (program_clock_reference<Complete_Stream->Streams[pid]->TimeStamp_Start)
                                        program_clock_reference+=0x200000000LL*300; //33 bits, cyclic
                                    if ((program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_Start)>Begin_MaxDuration)
                                    {
                                        Complete_Stream->Streams[pid]->EndTimeStampMoreThanxSeconds=true;
                                        Complete_Stream->Streams_With_EndTimeStampMoreThanxSecondsCount++;
                                        if (Complete_Stream->Streams_NotParsedCount
                                         && Complete_Stream->Streams_With_StartTimeStampCount>0
                                         && Complete_Stream->Streams_With_StartTimeStampCount==Complete_Stream->Streams_With_EndTimeStampMoreThanxSecondsCount)
                                        {
                                            //We are already parsing 16 seconds (for all PCRs), we don't hope to have more info
                                            MpegTs_JumpTo_Begin=File_Offset+Buffer_Offset-Buffer_TotalBytes_FirstSynched;
                                            if (MpegTs_JumpTo_End>MpegTs_JumpTo_Begin)
                                                MpegTs_JumpTo_End=MpegTs_JumpTo_Begin;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            #endif //MEDIAINFO_MPEGTS_PCR_YES
        }

        #if MEDIAINFO_DUPLICATE
            if (Stream->ShouldDuplicate)
            {
                Element_Size=TS_Size;
                File__Duplicate_Write();
            }
        #endif //MEDIAINFO_DUPLICATE

        Header_Parse_Events();

        Buffer_Offset+=TS_Size;
    }

    if (File_Offset+Buffer_Size>=File_Size)
        Detect_EOF(); //for TRP files

    return false; //Not enough data
}

//---------------------------------------------------------------------------
void File_MpegTs::Synched_Init()
{
    //Config->File_Filter_Set(462);
    //Default values
    Complete_Stream=new complete_stream;
    Complete_Stream->Streams.resize(0x2000);
    for (size_t StreamID=0; StreamID<0x2000; StreamID++)
        Complete_Stream->Streams[StreamID]=new complete_stream::stream;
    Complete_Stream->Streams[0x0000]->Searching_Payload_Start_Set(true);
    Complete_Stream->Streams[0x0000]->Kind=complete_stream::stream::psi;                        // Program Association Table
    Complete_Stream->Streams[0x0000]->Table_IDs.resize(0x100);
    Complete_Stream->Streams[0x0000]->Table_IDs[0x00]=new complete_stream::stream::table_id;    // program_association_section
    Complete_Stream->Streams[0x0001]->Searching_Payload_Start_Set(true);
    Complete_Stream->Streams[0x0001]->Kind=complete_stream::stream::psi;                        // Conditional Access Table
    Complete_Stream->Streams[0x0001]->Table_IDs.resize(0x100);
    Complete_Stream->Streams[0x0001]->Table_IDs[0x01]=new complete_stream::stream::table_id;    // CA_section
    Complete_Stream->Streams[0x0002]->Searching_Payload_Start_Set(true);
    Complete_Stream->Streams[0x0002]->Kind=complete_stream::stream::psi;                        // Transport Stream Description Table
    Complete_Stream->Streams[0x0002]->Table_IDs.resize(0x100);
    Complete_Stream->Streams[0x0003]->Searching_Payload_Start_Set(true);
    Complete_Stream->Streams[0x0003]->Kind=complete_stream::stream::psi;                        // IPMP Control Information Table
    Complete_Stream->Streams[0x0003]->Table_IDs.resize(0x100);

    //Temp
    MpegTs_JumpTo_Begin=(File_Offset_FirstSynched==(int64u)-1?0:Buffer_TotalBytes_LastSynched)+MediaInfoLib::Config.MpegTs_MaximumOffset_Get();
    MpegTs_JumpTo_End=MediaInfoLib::Config.MpegTs_MaximumOffset_Get()/4;
    Buffer_TotalBytes_LastSynched=Buffer_TotalBytes_FirstSynched;
    if (MpegTs_JumpTo_Begin==(int64u)-1 || MpegTs_JumpTo_Begin+MpegTs_JumpTo_End>=File_Size)
    {
        if (MpegTs_JumpTo_Begin+MpegTs_JumpTo_End>File_Size)
        {
            MpegTs_JumpTo_Begin=File_Size;
            MpegTs_JumpTo_End=0;
        }
        else
            MpegTs_JumpTo_Begin=File_Size-MpegTs_JumpTo_End;
    }

    //Config
    Config_Trace_TimeSection_OnlyFirstOccurrence=MediaInfoLib::Config.Trace_TimeSection_OnlyFirstOccurrence_Get();
    TimeSection_FirstOccurrenceParsed=false;
    #if MEDIAINFO_ADVANCED
        Config_VbrDetection_Delta=MediaInfoLib::Config.MpegTs_VbrDetection_Delta_Get();
        Config_VbrDetection_Occurences=MediaInfoLib::Config.MpegTs_VbrDetection_Occurences_Get();
        Config_VbrDetection_GiveUp=MediaInfoLib::Config.MpegTs_VbrDetection_GiveUp_Get();
    #endif // MEDIAINFO_ADVANCED

    //Continue, again, for Duplicate and Filter
    Option_Manage();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegTs::Read_Buffer_Unsynched()
{
    if (Complete_Stream==NULL || Complete_Stream->Streams.empty())
        return;

    for (size_t StreamID=0; StreamID<0x2000; StreamID++)//std::map<int64u, stream>::iterator Stream=Streams.begin(); Stream!=Streams.end(); Stream++)
    {
        //End timestamp is out of date
        #if defined(MEDIAINFO_MPEGTS_PCR_YES) || defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
        Complete_Stream->Streams[StreamID]->Searching_TimeStamp_Start_Set(false); //No more searching start
        Complete_Stream->Streams[StreamID]->TimeStamp_End=(int64u)-1;
        Complete_Stream->Streams[StreamID]->TimeStamp_End_IsUpdated=false;
        Complete_Stream->Streams[StreamID]->TimeStamp_End_Offset=(int64u)-1;
        if (Complete_Stream->Streams[StreamID]->TimeStamp_Start!=(int64u)-1)
            Complete_Stream->Streams[StreamID]->Searching_TimeStamp_End_Set(true); //Searching only for a start found
        #endif //defined(MEDIAINFO_MPEGTS_PCR_YES) ||  defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
        if (Complete_Stream->Streams[StreamID]->Parser)
        {
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                Complete_Stream->Streams[StreamID]->Searching_ParserTimeStamp_Start_Set(false); //No more searching start
                if (((File_MpegPs*)Complete_Stream->Streams[StreamID]->Parser)->HasTimeStamps)
                    Complete_Stream->Streams[StreamID]->Searching_ParserTimeStamp_End_Set(true); //Searching only for a start found
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            if (File_GoTo==0)
                Complete_Stream->Streams[StreamID]->Parser->Unsynch_Frame_Count=0;
            Complete_Stream->Streams[StreamID]->Parser->Open_Buffer_Unsynch();
        }
        #if MEDIAINFO_IBI
            Complete_Stream->Streams[StreamID]->Ibi_SynchronizationOffset_BeginOfFrame=(int64u)-1;
            for (complete_stream::stream::table_ids::iterator TableID=Complete_Stream->Streams[StreamID]->Table_IDs.begin(); TableID!=Complete_Stream->Streams[StreamID]->Table_IDs.end(); ++TableID)
                if (*TableID)
                    for (complete_stream::stream::table_id::table_id_extensions::iterator TableIdExtension=(*TableID)->Table_ID_Extensions.begin(); TableIdExtension!=(*TableID)->Table_ID_Extensions.end(); ++TableIdExtension)
                        TableIdExtension->second.version_number=(int8u)-1;
        #endif //MEDIAINFO_IBI
    }
    Complete_Stream->Duration_End.clear();

    //Clearing durations
    Clear(Stream_General, 0, General_Duration);
    Clear(Stream_General, 0, General_Duration_End);
    for (size_t StreamPos=0; StreamPos<Count_Get(Stream_Menu); StreamPos++)
        Clear(Stream_Menu, StreamPos, Menu_Duration);
}

//---------------------------------------------------------------------------
void File_MpegTs::Read_Buffer_Continue()
{
    if (!IsSub)
    {
        if (Config->ParseSpeed>=1.0)
            Config->State_Set(((float)Buffer_TotalBytes)/File_Size);
        else if (Buffer_TotalBytes>MpegTs_JumpTo_Begin+MpegTs_JumpTo_End)
            Config->State_Set((float)0.99); //Nearly the end
        else
            Config->State_Set(((float)Buffer_TotalBytes)/(MpegTs_JumpTo_Begin+MpegTs_JumpTo_End));
    }

    #if MEDIAINFO_DEMUX
        if (Complete_Stream && pid<0x2000 && Complete_Stream->Streams[pid]->Kind==complete_stream::stream::pes && Complete_Stream->Streams[pid]->Parser && ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->Demux_StreamIsBeingParsed_type!=(int8u)-1)
        {
            Open_Buffer_Continue(Complete_Stream->Streams[pid]->Parser, Buffer, 0, false);
            PES_Parse_Finish();
        }
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_MpegTs::Read_Buffer_AfterParsing()
{
    if (Complete_Stream==NULL)
        return; //No synchronization yet

    if (!Status[IsFilled])
    {
        //Test if parsing of headers is OK
        if ((Complete_Stream->Streams_NotParsedCount==0 && (Complete_Stream->NoPatPmt || (Complete_Stream->transport_stream_id_IsValid && Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs_NotParsedCount==0)))
         || (Buffer_TotalBytes-Buffer_TotalBytes_FirstSynched>=MpegTs_JumpTo_Begin && Config->ParseSpeed<0.8)
         || File_Offset+Buffer_Size==File_Size)
        {
            //Test if PAT/PMT are missing (ofen in .trp files)
            if (!Complete_Stream->transport_stream_id_IsValid
             && !Complete_Stream->NoPatPmt)
            {
                //Activating all streams as PES
                Complete_Stream->Streams_NotParsedCount=(size_t)-1;
                for (size_t StreamID=0; StreamID<0x2000; StreamID++)
                {
                    delete Complete_Stream->Streams[StreamID]; Complete_Stream->Streams[StreamID]=new complete_stream::stream;
                }
                for (size_t StreamID=0x20; StreamID<0x1FFF; StreamID++)
                {
                    Complete_Stream->Streams[StreamID]->Kind=complete_stream::stream::pes;
                    Complete_Stream->Streams[StreamID]->Searching_Payload_Start_Set(true);
                    Complete_Stream->Streams[StreamID]->Searching_Payload_Continue_Set(false);
                    #if MEDIAINFO_TRACE
                        if (Trace_Activated)
                            Complete_Stream->Streams[StreamID]->Element_Info1="PES";
                    #endif //MEDIAINFO_TRACE
                    #ifdef MEDIAINFO_MPEGTS_PCR_YES
                        Complete_Stream->Streams[StreamID]->Searching_TimeStamp_Start_Set(true);
                        Complete_Stream->Streams[StreamID]->Searching_TimeStamp_End_Set(false);
                    #endif //MEDIAINFO_MPEGTS_PCR_YES
                    #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                        Complete_Stream->Streams[StreamID]->Searching_ParserTimeStamp_Start_Set(true);
                        Complete_Stream->Streams[StreamID]->Searching_ParserTimeStamp_End_Set(false);
                    #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                }
                Complete_Stream->NoPatPmt=true;
                Fill(Stream_General, 0, General_Format_Profile, "No PAT/PMT");
                Buffer_TotalBytes=0;
                Buffer_TotalBytes_LastSynched=(int64u)-1;
                Open_Buffer_Unsynch();
                GoTo(0);
                return;
            }

            //Filling
            for (std::set<int16u>::iterator StreamID=Complete_Stream->PES_PIDs.begin(); StreamID!=Complete_Stream->PES_PIDs.end(); ++StreamID)
            {
                if (Complete_Stream->Streams[*StreamID]->Parser)
                {
                    Fill(Complete_Stream->Streams[*StreamID]->Parser);

                    Complete_Stream->Streams[*StreamID]->Parser->Status[IsUpdated]=false;
                    Complete_Stream->Streams[*StreamID]->IsUpdated_Info=true;
                }

                for (size_t Pos=0; Pos<Complete_Stream->Streams[*StreamID]->program_numbers.size(); Pos++)
                    Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[*StreamID]->program_numbers[Pos]].Update_Needed_IsRegistered=true;
            }
            Complete_Stream->Streams_NotParsedCount=0;
            Fill();

            //Deactivating
            if (Config->File_StopSubStreamAfterFilled_Get())
                for (std::set<int16u>::iterator StreamID=Complete_Stream->PES_PIDs.begin(); StreamID!=Complete_Stream->PES_PIDs.end(); ++StreamID)
                {
                    Complete_Stream->Streams[*StreamID]->Searching_Payload_Start_Set(false);
                    Complete_Stream->Streams[*StreamID]->Searching_Payload_Continue_Set(false);
                }

            //Status
            Status[IsUpdated]=true;
            Status[User_19]=true;

            //Jumping
            if (Config->ParseSpeed<1.0 && Config->File_IsSeekable_Get() && File_Offset+Buffer_Size<File_Size-MpegTs_JumpTo_End && MpegTs_JumpTo_End)
            {
                #if !defined(MEDIAINFO_MPEGTS_PCR_YES) && !defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
                    GoToFromEnd(47); //TODO: Should be changed later (when Finalize stuff will be split)
                #else //!defined(MEDIAINFO_MPEGTS_PCR_YES) && !defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
                    GoToFromEnd(MpegTs_JumpTo_End);
                    Searching_TimeStamp_Start=false;
                #endif //!defined(MEDIAINFO_MPEGTS_PCR_YES) && !defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
                Open_Buffer_Unsynch();
            }
        }
    }
}

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_MpegTs::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    //Reset
    Seek_Value=(int64u)-1;
    Seek_ID=(int64u)-1;
    InfiniteLoop_Detect=0;
    #if MEDIAINFO_DEMUX
        Config->Demux_IsSeeking=false;
    #endif //MEDIAINFO_DEMUX

    //Init
    if (!Duration_Detected)
    {
        //External IBI
        #if MEDIAINFO_IBI
            std::string IbiFile=Config->Ibi_Get();
            if (!IbiFile.empty())
            {
                Ibi.Streams.clear(); //TODO: support IBI data from different inputs

                File_Ibi MI;
                Open_Buffer_Init(&MI, IbiFile.size());
                MI.Ibi=&Ibi;
                MI.Open_Buffer_Continue((const int8u*)IbiFile.c_str(), IbiFile.size());
            }
            //Creating base IBI from a quick analysis of the file
            else
            {
                MediaInfo_Internal MI;
                MI.Option(__T("File_KeepInfo"), __T("1"));
                Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
                Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
                MI.Option(__T("ParseSpeed"), __T("0"));
                MI.Option(__T("Demux"), Ztring());
                Ztring File_Names=Config->File_Names.Read();
                MI.Option(__T("File_FileNameFormat"), __T("CSV"));
                size_t MiOpenResult=MI.Open(File_Names);
                MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
                MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
                if (!MiOpenResult)
                    return 0;
                for (ibi::streams::iterator IbiStream_Temp=((File_MpegTs*)MI.Info)->Ibi.Streams.begin(); IbiStream_Temp!=((File_MpegTs*)MI.Info)->Ibi.Streams.end(); ++IbiStream_Temp)
                {
                    if (Ibi.Streams[IbiStream_Temp->first]==NULL)
                        Ibi.Streams[IbiStream_Temp->first]=new ibi::stream(*IbiStream_Temp->second);
                    Ibi.Streams[IbiStream_Temp->first]->Unsynch();
                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                    {
                        Ibi.Streams[IbiStream_Temp->first]->Add(IbiStream_Temp->second->Infos[Pos]);
                        if (!IbiStream_Temp->second->Infos[Pos].IsContinuous)
                            Ibi.Streams[IbiStream_Temp->first]->Unsynch();
                    }
                    Ibi.Streams[IbiStream_Temp->first]->Unsynch();
                }
                if (Ibi.Streams.empty())
                    return 4; //Problem during IBI file parsing
            }
        #endif //#if MEDIAINFO_IBI

        Duration_Detected=true;
    }

    //Parsing
    switch (Method)
    {
        case 0  :
                    GoTo(Value);
                    Open_Buffer_Unsynch();
                    return 1;
        case 1  :
                    GoTo(File_Size*Value/10000);
                    Open_Buffer_Unsynch();
                    return 1;
        case 2  :   //Timestamp
                    #if MEDIAINFO_IBI
                    {
                    ibi::streams::iterator IbiStream_Temp;
                    if (ID==(int64u)-1)
                        IbiStream_Temp=Ibi.Streams.begin();
                    else
                        IbiStream_Temp=Ibi.Streams.find(ID);
                    if (IbiStream_Temp==Ibi.Streams.end() || IbiStream_Temp->second->Infos.empty())
                    {
                        for (IbiStream_Temp=Ibi.Streams.begin(); IbiStream_Temp!=Ibi.Streams.end(); ++IbiStream_Temp)
                            if (!IbiStream_Temp->second->Infos.empty())
                                break;

                        if (IbiStream_Temp==Ibi.Streams.end())
                            return 5; //Invalid ID
                    }

                    if (!(IbiStream_Temp->second->DtsFrequencyNumerator==1000000000 && IbiStream_Temp->second->DtsFrequencyDenominator==1))
                    {
                        float64 ValueF=(float64)Value;
                        ValueF/=1000000000; //Value is in ns
                        ValueF/=IbiStream_Temp->second->DtsFrequencyDenominator;
                        ValueF*=IbiStream_Temp->second->DtsFrequencyNumerator;
                        Value=float64_int64s(ValueF);
                    }

                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream_Temp->second->Infos[Pos].Dts || Pos+1==IbiStream_Temp->second->Infos.size())
                        {
                            if (Value<IbiStream_Temp->second->Infos[Pos].Dts && Pos)
                                Pos--;

                            //Checking continuity of Ibi
                            if (!IbiStream_Temp->second->Infos[Pos].IsContinuous && Pos+1<IbiStream_Temp->second->Infos.size() && InfiniteLoop_Detect<8) //With infinite loop detect
                            {
                                InfiniteLoop_Detect++;
                                Config->Demux_IsSeeking=true;
                                Seek_Value=Value;
                                Seek_Value_Maximal=IbiStream_Temp->second->Infos[Pos+1].StreamOffset;
                                Seek_ID=IbiStream_Temp->first;
                                GoTo((IbiStream_Temp->second->Infos[Pos].StreamOffset+IbiStream_Temp->second->Infos[Pos+1].StreamOffset)/2);
                                Open_Buffer_Unsynch();

                                return 1;
                            }

                            InfiniteLoop_Detect=0;
                            Config->Demux_IsSeeking=false;
                            if (Complete_Stream && Complete_Stream->Streams[(size_t)IbiStream_Temp->first] && Complete_Stream->Streams[(size_t)IbiStream_Temp->first]->Parser)
                                Complete_Stream->Streams[(size_t)IbiStream_Temp->first]->Parser->Unsynch_Frame_Count=IbiStream_Temp->second->Infos[Pos].FrameNumber;
                            else
                                Unsynch_Frame_Counts[(int16u)IbiStream_Temp->first]=IbiStream_Temp->second->Infos[Pos].FrameNumber;

                            GoTo(IbiStream_Temp->second->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    return 2; //Invalid value
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        case 3  :   //FrameNumber
                    #if MEDIAINFO_IBI
                    {
                    ibi::streams::iterator IbiStream_Temp;
                    if (ID==(int64u)-1)
                        IbiStream_Temp=Ibi.Streams.begin();
                    else
                        IbiStream_Temp=Ibi.Streams.find(ID);
                    if (IbiStream_Temp==Ibi.Streams.end() || IbiStream_Temp->second->Infos.empty())
                    {
                        for (IbiStream_Temp=Ibi.Streams.begin(); IbiStream_Temp!=Ibi.Streams.end(); ++IbiStream_Temp)
                            if (!IbiStream_Temp->second->Infos.empty())
                                break;

                        if (IbiStream_Temp==Ibi.Streams.end())
                            return 5; //Invalid ID
                    }

                    for (size_t Pos=0; Pos<IbiStream_Temp->second->Infos.size(); Pos++)
                    {
                        if (Value<=IbiStream_Temp->second->Infos[Pos].FrameNumber || Pos+1==IbiStream_Temp->second->Infos.size())
                        {
                            if (Value<IbiStream_Temp->second->Infos[Pos].FrameNumber && Pos)
                                Pos--;

                            Config->Demux_IsSeeking=false;
                            if (Complete_Stream && Complete_Stream->Streams[(size_t)IbiStream_Temp->first] && Complete_Stream->Streams[(size_t)IbiStream_Temp->first]->Parser)
                                Complete_Stream->Streams[(size_t)IbiStream_Temp->first]->Parser->Unsynch_Frame_Count=IbiStream_Temp->second->Infos[Pos].FrameNumber;
                            else
                                Unsynch_Frame_Counts[(int16u)IbiStream_Temp->first]=IbiStream_Temp->second->Infos[Pos].FrameNumber;

                            GoTo(IbiStream_Temp->second->Infos[Pos].StreamOffset);
                            Open_Buffer_Unsynch();

                            return 1;
                        }
                    }

                    return 2; //Invalid value
                    }
                    #else //MEDIAINFO_IBI
                    return (size_t)-2; //Not supported / IBI disabled
                    #endif //MEDIAINFO_IBI
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_MpegTs::FileHeader_Begin()
{
    if (Buffer_Size<8)
        return false; //Wait for more data

    //False positives detection: detect some headers from other files, DV parser is not smart enough
    if (CC8(Buffer+Buffer_Offset)==0x444C472056312E30LL //DLG
     || CC4(Buffer)==0x52494646  //RIFF
     || CC4(Buffer+4)==0x66747970  //ftyp
     || CC4(Buffer+4)==0x66726565  //free
     || CC4(Buffer+4)==0x6D646174  //mdat
     || CC4(Buffer+4)==0x6D6F6F76  //moov
     || CC4(Buffer+4)==0x736B6970  //skip
     || CC4(Buffer+4)==0x77696465  //wide
     || CC4(Buffer)==0x060E2B34) //MXF begin
    {
        Reject("MPEG-TS");
        return true;
    }

    //Configuring
    #if defined(MEDIAINFO_BDAV_YES) || defined(MEDIAINFO_TSP_YES)
        TS_Size=188
        #if defined(MEDIAINFO_BDAV_YES)
            +BDAV_Size
        #endif
        #if defined(MEDIAINFO_TSP_YES)
            +TSP_Size
        #endif
        ;
    #endif

    //Configuration
    Option_Manage();

    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegTs::Header_Parse()
{
    #if MEDIAINFO_TRACE
    if (Trace_Activated)
    {
        //Parsing
        bool  adaptation, payload;
        if (BDAV_Size)
            Skip_B4(                                                "BDAV"); //BDAV supplement
        Skip_B1(                                                    "sync_byte");
        BS_Begin();
        Skip_SB(                                                    "transport_error_indicator");
        Get_SB (    payload_unit_start_indicator,                   "payload_unit_start_indicator");
        Skip_SB(                                                    "transport_priority");
        Get_S2 (13, pid,                                            "pid");
        Get_S1 ( 2, transport_scrambling_control,                   "transport_scrambling_control");
        Get_SB (    adaptation,                                     "adaptation_field_control (adaptation)");
        Get_SB (    payload,                                        "adaptation_field_control (payload)");
        Skip_S1( 4,                                                 "continuity_counter");
        BS_End();

        //Info
        /* Trace: used to display program number(s)
        if (!Complete_Stream->Streams[pid]->program_numbers.empty())
        {
            Ztring Program_Numbers;
            for (size_t Pos=0; Pos<Complete_Stream->Streams[pid]->program_numbers.size(); Pos++)
                Program_Numbers+=Ztring::ToZtring_From_CC2(Complete_Stream->Streams[pid]->program_numbers[Pos])+__T('/');
            if (!Program_Numbers.empty())
                Program_Numbers.resize(Program_Numbers.size()-1);
            Data_Info(Program_Numbers);
        }
        else
            Data_Info("    ");
        */
        Data_Info(Complete_Stream->Streams[pid]->Element_Info1);

        //Adaptation
        if (adaptation)
            Header_Parse_AdaptationField();
        else
        {
        }

        //Data
        if (payload)
        {
            //Encryption
            if (transport_scrambling_control>0)
                Complete_Stream->Streams[pid]->IsScrambled++;
        }
        else if (Element_Offset<TS_Size)
            Skip_XX(TS_Size-Element_Offset,                         "Junk");

        //Filling
        Header_Fill_Code(pid, __T("0x")+Ztring().From_CC2(pid));
        Header_Fill_Size(TS_Size);

        Header_Parse_Events();
    }
    else
    {
    #endif //MEDIAINFO_TRACE
        //Parsing
               payload_unit_start_indicator=(Buffer[Buffer_Offset+BDAV_Size+1]&0x40)!=0;
               transport_scrambling_control= Buffer[Buffer_Offset+BDAV_Size+3]&0xC0;
        bool   adaptation=                  (Buffer[Buffer_Offset+BDAV_Size+3]&0x20)!=0;
        bool   payload=                     (Buffer[Buffer_Offset+BDAV_Size+3]&0x10)!=0;
        Element_Offset+=BDAV_Size+4;

        //Adaptation
        if (adaptation)
            Header_Parse_AdaptationField();
        else
        {
        }

        //Data
        if (payload)
        {
            //Encryption
            if (transport_scrambling_control>0)
                Complete_Stream->Streams[pid]->IsScrambled++;
        }

        //Filling
        /*Element[1].Next=File_Offset+Buffer_Offset+TS_Size;  //*/Header_Fill_Size(TS_Size);
        //Element[1].IsComplete=true;                         //Header_Fill_Size(TS_Size);

        Header_Parse_Events();
    #if MEDIAINFO_TRACE
    }
    #endif //MEDIAINFO_TRACE
}

//---------------------------------------------------------------------------
void File_MpegTs::Header_Parse_AdaptationField()
{
    #if MEDIAINFO_TRACE
    if (Trace_Activated)
    {
        int64u Element_Pos_Save=Element_Offset;
        Element_Begin1("adaptation_field");
        int8u adaptation_field_length;
        Get_B1 (adaptation_field_length,                            "adaptation_field_length");
        if (adaptation_field_length>188-4-1) //TS size - header - adaptation_field_length
        {
            adaptation_field_length=188-4-1;
            Skip_XX(188-4-1,                                        "stuffing_bytes");
        }
        else if (adaptation_field_length>0)
        {
            bool discontinuity_indicator, PCR_flag, OPCR_flag, splicing_point_flag, transport_private_data_flag, adaptation_field_extension_flag;
            BS_Begin();
            Get_SB (    discontinuity_indicator,                    "discontinuity_indicator");
            Skip_SB(                                                "random_access_indicator");
            Skip_SB(                                                "elementary_stream_priority_indicator");
            Get_SB (    PCR_flag,                                   "PCR_flag");
            Get_SB (    OPCR_flag,                                  "OPCR_flag");
            Get_SB (    splicing_point_flag,                        "splicing_point_flag");
            Get_SB (    transport_private_data_flag,                "transport_private_data_flag");
            Get_SB (    adaptation_field_extension_flag,            "adaptation_field_extension_flag");
            BS_End();
            if (PCR_flag)
            {
                #ifdef MEDIAINFO_MPEGTS_PCR_YES
                    BS_Begin();
                    int64u program_clock_reference_base;
                    int16u program_clock_reference_extension;
                    Get_S8 (33, program_clock_reference_base,           "program_clock_reference_base"); Param_Info_From_Milliseconds(program_clock_reference_base/90);
                    Data_Info_From_Milliseconds(program_clock_reference_base/90);
                    Skip_S1( 6,                                         "reserved");
                    Get_S2 ( 9, program_clock_reference_extension,      "program_clock_reference_extension");
                    int64u program_clock_reference=program_clock_reference_base*300+program_clock_reference_extension;
                    Param_Info1(program_clock_reference);
                    BS_End();
                    if (Complete_Stream->Streams[pid]->Searching_TimeStamp_End
                    #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                     && (!Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End
                      || Complete_Stream->Streams[pid]->IsPCR) //If PCR, we always want it.
                    #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                     )
                    {
                        Header_Parse_Events_Duration(program_clock_reference);
                        if (program_clock_reference!=Complete_Stream->Streams[pid]->TimeStamp_End) //Some PCRs are buggy (low precision), using the first stream offset in the case of duplicate PCR value
                        {
                            if (Complete_Stream->Streams[pid]->TimeStamp_End_Offset!=(int64u)-1)
                            {
                                if (program_clock_reference+0x12c00000000LL<Complete_Stream->Streams[pid]->TimeStamp_End)
                                    program_clock_reference+=0x25800000000LL; //33 bits and *300
                                if (!discontinuity_indicator && program_clock_reference>Complete_Stream->Streams[pid]->TimeStamp_End && program_clock_reference<Complete_Stream->Streams[pid]->TimeStamp_End+10*27000000) //Not before, not after 10 seconds, else there is a problem
                                {
                                    float64 Duration_InstantaneousBitRate_Min=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End-(Config_VbrDetection_Delta?0:810));
                                    float64 Duration_InstantaneousBitRate_Max=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End+(Config_VbrDetection_Delta?0:810));
                                    float64 Bytes_InstantaneousBitRate=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);
                                    float64 TimeStamp_InstantaneousBitRate_Current_Min=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Max*27000000*(1-Config_VbrDetection_Delta);
                                    float64 TimeStamp_InstantaneousBitRate_Current_Raw=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Min*27000000;
                                    float64 TimeStamp_InstantaneousBitRate_Current_Max=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Min*27000000*(1+Config_VbrDetection_Delta);
                                    if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min)
                                    {
                                        if (TimeStamp_InstantaneousBitRate_Current_Max<Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min || TimeStamp_InstantaneousBitRate_Current_Min>Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Max)
                                        {
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr++;
                                            #if MEDIAINFO_ADVANCED
                                                if (Config_VbrDetection_GiveUp && Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr>=Config_VbrDetection_Occurences)
                                                    Config->ParseSpeed=0;
                                            #endif // MEDIAINFO_ADVANCED
                                        }
                                        else
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsCbr++;
                                    }
                                    float64 Duration_Min=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End-1);
                                    float64 Duration_Raw=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End);
                                    float64 Duration_Max=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End+1);
                                    float64 Bytes=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);
                                    float64 TimeStamp_Min=Bytes*8/Duration_Max*27000000*(1-Config_VbrDetection_Delta);
                                    float64 TimeStamp_Raw=Bytes*8/Duration_Raw*27000000;
                                    float64 TimeStamp_Max=Bytes*8/Duration_Min*27000000*(1+Config_VbrDetection_Delta);
                                    Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min=TimeStamp_Min;
                                    Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw=TimeStamp_Raw;
                                    Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Max=TimeStamp_Max;
                                    #if MEDIAINFO_ADVANCED
                                        if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Min_Raw>TimeStamp_InstantaneousBitRate_Current_Raw)
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Min_Raw=TimeStamp_InstantaneousBitRate_Current_Raw;
                                        if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Max_Raw<TimeStamp_InstantaneousBitRate_Current_Raw)
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Max_Raw=TimeStamp_InstantaneousBitRate_Current_Raw;
                                        int64u Distance=program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End;
                                        if (Complete_Stream->Streams[pid]->TimeStamp_Distance_Min>Distance)
                                            Complete_Stream->Streams[pid]->TimeStamp_Distance_Min=Distance;
                                        if (Complete_Stream->Streams[pid]->TimeStamp_Distance_Max<Distance)
                                            Complete_Stream->Streams[pid]->TimeStamp_Distance_Max=Distance;
                                        Complete_Stream->Streams[pid]->TimeStamp_Distance_Total+=Distance;
                                        Complete_Stream->Streams[pid]->TimeStamp_Distance_Count++;
                                    #endif // MEDIAINFO_ADVANCED
                                }
                                #if MEDIAINFO_ADVANCED
                                    else
                                    {
                                       if (!discontinuity_indicator)
                                            Complete_Stream->Streams[pid]->TimeStamp_HasProblems++;
                                       float64 Bytes=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);;
                                       int64u TimeToAdd;
                                       if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw)
                                           TimeToAdd=float64_int64s(Bytes*8/Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw*27000000);
                                       else
                                           TimeToAdd=0;
                                       Complete_Stream->Streams[pid]->TimeStamp_Intermediate.push_back(Complete_Stream->Streams[pid]->TimeStamp_End+TimeToAdd);
                                       Complete_Stream->Streams[pid]->TimeStamp_Intermediate.push_back(program_clock_reference);
                                    }
                                #endif // MEDIAINFO_ADVANCED
                            }
                            Complete_Stream->Streams[pid]->TimeStamp_End=program_clock_reference;
                            Complete_Stream->Streams[pid]->TimeStamp_End_IsUpdated=true;
                            Complete_Stream->Streams[pid]->TimeStamp_End_Offset=File_Offset+Buffer_Offset;
                            {
                                Status[IsUpdated]=true;
                                Status[User_16]=true;
                            }
                        }
                    }
                    if (Complete_Stream->Streams[pid]->Searching_TimeStamp_Start)
                    {
                        //This is the first PCR
                        Complete_Stream->Streams[pid]->TimeStamp_Start=program_clock_reference;
                        Complete_Stream->Streams[pid]->TimeStamp_Start_Offset=File_Offset+Buffer_Offset;
                        Complete_Stream->Streams[pid]->TimeStamp_End=program_clock_reference;
                        Complete_Stream->Streams[pid]->TimeStamp_End_IsUpdated=true;
                        Complete_Stream->Streams[pid]->TimeStamp_End_Offset=File_Offset+Buffer_Offset;
                        Complete_Stream->Streams[pid]->Searching_TimeStamp_Start_Set(false);
                        Complete_Stream->Streams[pid]->Searching_TimeStamp_End_Set(true);
                        Complete_Stream->Streams_With_StartTimeStampCount++;
                        {
                            Status[IsUpdated]=true;
                            Status[User_16]=true;
                        }
                    }

                    //Test if we can find the TS bitrate
                    if (!Complete_Stream->Streams[pid]->EndTimeStampMoreThanxSeconds && Complete_Stream->Streams[pid]->TimeStamp_Start!=(int64u)-1
                    && (File_Offset+Buffer_Offset-Buffer_TotalBytes_FirstSynched)*2<File_Size)
                    {
                        if (program_clock_reference<Complete_Stream->Streams[pid]->TimeStamp_Start)
                            program_clock_reference+=0x200000000LL*300; //33 bits, cyclic
                        if ((program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_Start)>Begin_MaxDuration)
                        {
                            Complete_Stream->Streams[pid]->EndTimeStampMoreThanxSeconds=true;
                            Complete_Stream->Streams_With_EndTimeStampMoreThanxSecondsCount++;
                            if (Complete_Stream->Streams_NotParsedCount
                             && Complete_Stream->Streams_With_StartTimeStampCount>0
                             && Complete_Stream->Streams_With_StartTimeStampCount==Complete_Stream->Streams_With_EndTimeStampMoreThanxSecondsCount)
                            {
                                //We are already parsing 16 seconds (for all PCRs), we don't hope to have more info
                                MpegTs_JumpTo_Begin=File_Offset+Buffer_Offset-Buffer_TotalBytes_FirstSynched;
                                if (MpegTs_JumpTo_End>MpegTs_JumpTo_Begin)
                                    MpegTs_JumpTo_End=MpegTs_JumpTo_Begin;
                            }
                        }
                    }
                #else //MEDIAINFO_MPEGTS_PCR_YES
                    Skip_B6(                                        "program_clock_reference");
                #endif //MEDIAINFO_MPEGTS_PCR_YES
            }
            if (OPCR_flag)
            {
                BS_Begin();
                Skip_S8(33,                                         "original_program_clock_reference_base");
                Skip_S1( 6,                                         "reserved");
                Skip_S2( 9,                                         "original_program_clock_reference_extension");
                BS_End();
            }
            if (splicing_point_flag)
            {
                Skip_B1(                                            "splice_countdown");
            }
            if (transport_private_data_flag)
            {
                int8u transport_private_data_length;
                Get_B1 (transport_private_data_length,              "transport_private_data_length");
                if (Element_Offset+transport_private_data_length<=Element_Pos_Save+1+adaptation_field_length)
                    Skip_XX(transport_private_data_length,          "transport_private_data");
                else
                    Skip_XX(Element_Pos_Save+1+adaptation_field_length-Element_Offset, "problem");
            }
            if (adaptation_field_extension_flag)
            {
                int8u adaptation_field_extension_length;
                Get_B1 (adaptation_field_extension_length,          "adaptation_field_extension_length");
                if (Element_Offset+adaptation_field_extension_length<=Element_Pos_Save+1+adaptation_field_length)
                {
                    Element_Begin1("adaptation_field_extension");
                    int64u End=Element_Offset+adaptation_field_extension_length;
                    bool ltw_flag, piecewise_rate_flag, seamless_splice_flag;
                    BS_Begin();
                    Get_SB (    ltw_flag,                                   "ltw_flag");
                    Get_SB (    piecewise_rate_flag,                        "piecewise_rate_flag");
                    Get_SB (    seamless_splice_flag,                       "seamless_splice_flag");
                    Skip_S1( 5,                                             "reserved");
                    if (ltw_flag)
                    {
                        Skip_SB(                                            "ltw_valid_flag");
                        Skip_S2(15,                                         "ltw_offset");
                    }
                    if (piecewise_rate_flag)
                    {
                        Skip_S1( 2,                                         "reserved");
                        Skip_S3(22,                                         "piecewise_rate");
                    }
                    if (seamless_splice_flag)
                    {
                        Skip_S1( 4,                                         "splice_type");
                        int16u DTS_29, DTS_14;
                        int8u  DTS_32;
                        Element_Begin1("DTS");
                        Get_S1 ( 3, DTS_32,                                 "DTS_32");
                        Mark_1();
                        Get_S2 (15, DTS_29,                                 "DTS_29");
                        Mark_1();
                        Get_S2 (15, DTS_14,                                 "DTS_14");
                        Mark_1();

                        //Filling
                        int64u DTS;
                        DTS=(((int64u)DTS_32)<<30)
                          | (((int64u)DTS_29)<<15)
                          | (((int64u)DTS_14));
                        Element_Info_From_Milliseconds(DTS/90);
                        Element_End0();
                    }
                    BS_End();
                    if (Element_Offset<End)
                        Skip_XX(End-Element_Offset,                         "reserved");
                    Element_End0();
                }
                else
                    Skip_XX(Element_Pos_Save+1+adaptation_field_length-Element_Offset, "problem");
            }
        }

        if (Element_Offset<Element_Pos_Save+1+adaptation_field_length)
            Skip_XX(Element_Pos_Save+1+adaptation_field_length-Element_Offset, "stuffing_bytes");
        Element_End0();
    }
    else
    {
    #endif //MEDIAINFO_TRACE
        int8u adaptation_field_length=Buffer[Buffer_Offset+BDAV_Size+4];
        #ifdef MEDIAINFO_MPEGTS_PCR_YES
            if (adaptation_field_length>188-4-1) //TS size - header - adaptation_field_length
                adaptation_field_length=188-4-1;
            else if (adaptation_field_length)
            {
                bool discontinuity_indicator=(Buffer[Buffer_Offset+BDAV_Size+5]&0x80)!=0;
                bool PCR_flag=(Buffer[Buffer_Offset+BDAV_Size+5]&0x10)!=0;
                if (PCR_flag)
                {
                    int64u program_clock_reference=(  (((int64u)Buffer[Buffer_Offset+BDAV_Size+6])<<25)
                                                    | (((int64u)Buffer[Buffer_Offset+BDAV_Size+7])<<17)
                                                    | (((int64u)Buffer[Buffer_Offset+BDAV_Size+8])<< 9)
                                                    | (((int64u)Buffer[Buffer_Offset+BDAV_Size+9])<< 1)
                                                    | (((int64u)Buffer[Buffer_Offset+BDAV_Size+10])>>7));
                    program_clock_reference*=300;
                    program_clock_reference+=(  (((int64u)Buffer[Buffer_Offset+BDAV_Size+10]&0x01)<<8)
                                              | (((int64u)Buffer[Buffer_Offset+BDAV_Size+11])   ));
                    if (Complete_Stream->Streams[pid]->Searching_TimeStamp_End
                    #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                     && (!Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End
                      || Complete_Stream->Streams[pid]->IsPCR) //If PCR, we always want it.
                    #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                     )
                    {
                        Header_Parse_Events_Duration(program_clock_reference);
                        if (program_clock_reference!=Complete_Stream->Streams[pid]->TimeStamp_End) //Some PCRs are buggy (low precision), using the first stream offset in the case of duplicate PCR value
                        {
                            if (Complete_Stream->Streams[pid]->TimeStamp_End_Offset!=(int64u)-1)
                            {
                                if (program_clock_reference+0x12c00000000LL<Complete_Stream->Streams[pid]->TimeStamp_End)
                                    program_clock_reference+=0x25800000000LL; //33 bits and *300
                                if (!discontinuity_indicator && program_clock_reference>Complete_Stream->Streams[pid]->TimeStamp_End && program_clock_reference<Complete_Stream->Streams[pid]->TimeStamp_End+10*27000000) //Not before, not after 10 seconds, else there is a problem
                                {
                                    float64 Duration_InstantaneousBitRate_Min=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End-(Config_VbrDetection_Delta?0:810));
                                    float64 Duration_InstantaneousBitRate_Max=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End+(Config_VbrDetection_Delta?0:810));
                                    float64 Bytes_InstantaneousBitRate=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);
                                    float64 TimeStamp_InstantaneousBitRate_Current_Min=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Max*27000000*(1-Config_VbrDetection_Delta);
                                    float64 TimeStamp_InstantaneousBitRate_Current_Max=Bytes_InstantaneousBitRate*8/Duration_InstantaneousBitRate_Min*27000000*(1+Config_VbrDetection_Delta);
                                    if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min)
                                    {
                                        if (TimeStamp_InstantaneousBitRate_Current_Max<Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min || TimeStamp_InstantaneousBitRate_Current_Min>Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Max)
                                        {
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr++;
                                            #if MEDIAINFO_ADVANCED
                                                if (Config_VbrDetection_GiveUp && Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr>=Config_VbrDetection_Occurences)
                                                    Config->ParseSpeed=0;
                                            #endif // MEDIAINFO_ADVANCED
                                        }
                                        else
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_BitRateMode_IsCbr++;
                                    }
                                    float64 Duration_Min=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End-1);
                                    float64 Duration_Raw=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End);
                                    float64 Duration_Max=(float64)(program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End+1);
                                    float64 Bytes=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);
                                    float64 InstantaneousBitRate_Min=Bytes*8/Duration_Max*27000000*(1-Config_VbrDetection_Delta);
                                    float64 InstantaneousBitRate_Raw=Bytes*8/Duration_Raw*27000000;
                                    float64 InstantaneousBitRate_Max=Bytes*8/Duration_Min*27000000*(1+Config_VbrDetection_Delta);
                                    Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Min=InstantaneousBitRate_Min;
                                    Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw=InstantaneousBitRate_Raw;
                                    Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Max=InstantaneousBitRate_Max;
                                    #if MEDIAINFO_ADVANCED
                                        if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Min_Raw>InstantaneousBitRate_Raw)
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Min_Raw=InstantaneousBitRate_Raw;
                                        if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Max_Raw<InstantaneousBitRate_Raw)
                                            Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Max_Raw=InstantaneousBitRate_Raw;
                                        int64u Distance=program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_End;
                                        if (Complete_Stream->Streams[pid]->TimeStamp_Distance_Min>Distance)
                                            Complete_Stream->Streams[pid]->TimeStamp_Distance_Min=Distance;
                                        if (Complete_Stream->Streams[pid]->TimeStamp_Distance_Max<Distance)
                                            Complete_Stream->Streams[pid]->TimeStamp_Distance_Max=Distance;
                                        Complete_Stream->Streams[pid]->TimeStamp_Distance_Total+=Distance;
                                        Complete_Stream->Streams[pid]->TimeStamp_Distance_Count++;
                                    #endif // MEDIAINFO_ADVANCED
                                }
                                #if MEDIAINFO_ADVANCED
                                    else
                                    {
                                       if (!discontinuity_indicator)
                                            Complete_Stream->Streams[pid]->TimeStamp_HasProblems++;
                                       float64 Bytes=(float64)(File_Offset+Buffer_Offset-Complete_Stream->Streams[pid]->TimeStamp_End_Offset);;
                                       int64u TimeToAdd;
                                       if (Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw)
                                           TimeToAdd=float64_int64s(Bytes*8/Complete_Stream->Streams[pid]->TimeStamp_InstantaneousBitRate_Current_Raw*27000000);
                                       else
                                           TimeToAdd=0;
                                       Complete_Stream->Streams[pid]->TimeStamp_Intermediate.push_back(Complete_Stream->Streams[pid]->TimeStamp_End+TimeToAdd);
                                       Complete_Stream->Streams[pid]->TimeStamp_Intermediate.push_back(program_clock_reference);
                                    }
                                #endif // MEDIAINFO_ADVANCED
                            }
                            Complete_Stream->Streams[pid]->TimeStamp_End=program_clock_reference;
                            Complete_Stream->Streams[pid]->TimeStamp_End_IsUpdated=true;
                            Complete_Stream->Streams[pid]->TimeStamp_End_Offset=File_Offset+Buffer_Offset;
                            {
                                Status[IsUpdated]=true;
                                Status[User_16]=true;
                            }
                        }
                    }
                    if (Complete_Stream->Streams[pid]->Searching_TimeStamp_Start)
                    {
                        //This is the first PCR
                        Complete_Stream->Streams[pid]->TimeStamp_Start=program_clock_reference;
                        Complete_Stream->Streams[pid]->TimeStamp_Start_Offset=File_Offset+Buffer_Offset;
                        Complete_Stream->Streams[pid]->TimeStamp_End=program_clock_reference;
                        Complete_Stream->Streams[pid]->TimeStamp_End_IsUpdated=true;
                        Complete_Stream->Streams[pid]->TimeStamp_End_Offset=File_Offset+Buffer_Offset;
                        Complete_Stream->Streams[pid]->Searching_TimeStamp_Start_Set(false);
                        Complete_Stream->Streams[pid]->Searching_TimeStamp_End_Set(true);
                        Complete_Stream->Streams_With_StartTimeStampCount++;
                        {
                            Status[IsUpdated]=true;
                            Status[User_16]=true;
                        }
                    }

                    //Test if we can find the TS bitrate
                    if (!Complete_Stream->Streams[pid]->EndTimeStampMoreThanxSeconds && Complete_Stream->Streams[pid]->TimeStamp_Start!=(int64u)-1
                    && (File_Offset+Buffer_Offset-Buffer_TotalBytes_FirstSynched)*2<File_Size)
                    {
                        if (program_clock_reference<Complete_Stream->Streams[pid]->TimeStamp_Start)
                            program_clock_reference+=0x200000000LL*300; //33 bits, cyclic
                        if ((program_clock_reference-Complete_Stream->Streams[pid]->TimeStamp_Start)>Begin_MaxDuration)
                        {
                            Complete_Stream->Streams[pid]->EndTimeStampMoreThanxSeconds=true;
                            Complete_Stream->Streams_With_EndTimeStampMoreThanxSecondsCount++;
                            if (Complete_Stream->Streams_NotParsedCount
                             && Complete_Stream->Streams_With_StartTimeStampCount>0
                             && Complete_Stream->Streams_With_StartTimeStampCount==Complete_Stream->Streams_With_EndTimeStampMoreThanxSecondsCount)
                            {
                                //We are already parsing 16 seconds (for all PCRs), we don't hope to have more info
                                MpegTs_JumpTo_Begin=File_Offset+Buffer_Offset-Buffer_TotalBytes_FirstSynched;
                                if (MpegTs_JumpTo_End>MpegTs_JumpTo_Begin)
                                    MpegTs_JumpTo_End=MpegTs_JumpTo_Begin;
                            }
                        }
                    }
                }
            }
        #endif //MEDIAINFO_MPEGTS_PCR_YES
        Element_Offset+=1+adaptation_field_length;
    #if MEDIAINFO_TRACE
    }
    #endif //MEDIAINFO_TRACE
}

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void File_MpegTs::Header_Parse_Events()
{
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void File_MpegTs::Header_Parse_Events_Duration(int64u program_clock_reference)
{
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
void File_MpegTs::Data_Parse()
{
    //Counting
    Frame_Count++;

    //TSP specific
    if (TSP_Size && Element_Size>TSP_Size)
        Element_Size-=TSP_Size;

    #if MEDIAINFO_DUPLICATE
        if (Complete_Stream->Streams[pid]->ShouldDuplicate)
            File__Duplicate_Write();
    #endif //MEDIAINFO_DUPLICATE

    //Parsing
    if (!Complete_Stream->Streams[pid]->Searching_Payload_Start
     && !Complete_Stream->Streams[pid]->Searching_Payload_Continue
     #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
         && !Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start
         && !Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End
     #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
     )
        Skip_XX(Element_Size,                                   "data");
    else
        switch (Complete_Stream->Streams[pid]->Kind)
        {
            case complete_stream::stream::pes : PES(); break;
            case complete_stream::stream::psi : PSI(); break;
            default: ;
        }

    //TSP specific
    if (TSP_Size && Element_Size>TSP_Size)
    {
        Element_Size+=TSP_Size;
        Skip_B4(                                                "TSP"); //TSP supplement
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_MpegTs::PES()
{
    //Info
    DETAILS_INFO(if (Complete_Stream->transport_stream_id_IsValid) Element_Info1(Mpeg_Psi_stream_type_Info(Complete_Stream->Streams[pid]->stream_type, Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[0]].registration_format_identifier));)

    //Demux
    #if MEDIAINFO_DEMUX
        Element_Code=pid;
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Exists
    if (!Complete_Stream->Streams[pid]->IsRegistered)
    {
        Complete_Stream->Streams[pid]->IsRegistered=true;
        for (size_t Pos=0; Pos<Complete_Stream->Streams[pid]->program_numbers.size(); Pos++)
            if (!Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[Pos]].IsRegistered)
            {
                Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[Pos]].Update_Needed_IsRegistered=true;

                Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[Pos]].IsRegistered=true;
            }

        Complete_Stream->Streams[pid]->IsUpdated_IsRegistered=true;
        Complete_Stream->PES_PIDs.insert(pid);

        Status[IsUpdated]=true;
        Status[User_19]=true;
    }

    //Case of encrypted streams
    if (transport_scrambling_control)
    {
        if (!Complete_Stream->Streams[pid]->Searching_Payload_Continue)
            Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(true); //In order to count the packets

        if (Complete_Stream->Streams[pid]->IsScrambled>16)
        {
            //Don't need anymore
            Complete_Stream->Streams[pid]->Searching_Payload_Start_Set(false);
            Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(false);
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start_Set(false);
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            if (!Complete_Stream->Streams[pid]->IsParsed && Complete_Stream->Streams_NotParsedCount)
            {
                Complete_Stream->Streams[pid]->IsParsed=true;
                Complete_Stream->Streams_NotParsedCount--;
            }
        }
        Skip_XX(Element_Size-Element_Offset,                    "Scrambled data");

        return;
    }
    else if (Complete_Stream->Streams[pid]->IsScrambled)
        Complete_Stream->Streams[pid]->IsScrambled--;

    //Parser creation
    if (Complete_Stream->Streams[pid]->Parser==NULL)
    {
        //Waiting for first payload_unit_start_indicator
        if (!payload_unit_start_indicator)
        {
            Element_DoNotShow(); //We don't want to show this item because there is no interessant info
            return; //This is not the start of the PES
        }

        //If unknown stream_type
        if (Complete_Stream->transport_stream_id_IsValid
         && Mpeg_Psi_stream_type_StreamKind(Complete_Stream->Streams[pid]->stream_type, Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[0]].registration_format_identifier)==Stream_Max
         && Complete_Stream->Streams[pid]->stream_type!=0x06 //Exception for private data
         && Complete_Stream->Streams[pid]->stream_type<=0x7F //Exception for private data
         && Mpeg_Descriptors_registration_format_identifier_StreamKind(Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[0]].registration_format_identifier)==Stream_Max //From Descriptor
         && Config->File_MpegTs_stream_type_Trust_Get())
        {
            Complete_Stream->Streams[pid]->Searching_Payload_Start_Set(false);
            Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(false);
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start_Set(false);
                Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End_Set(false);
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            if (!Complete_Stream->Streams[pid]->IsParsed && Complete_Stream->Streams_NotParsedCount)
            {
                Complete_Stream->Streams[pid]->IsParsed=true;
                Complete_Stream->Streams_NotParsedCount--;
            }
            return;
        }

        //Allocating an handle if needed
        #if defined(MEDIAINFO_MPEGPS_YES)
            Complete_Stream->Streams[pid]->Parser=new File_MpegPs;
            #if MEDIAINFO_SEEK
                if (Unsynch_Frame_Counts.find(pid)!=Unsynch_Frame_Counts.end())
                {
                    ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->Unsynch_Frame_Count_Temp=Unsynch_Frame_Counts[pid];
                    Unsynch_Frame_Counts.erase(pid);
                }
            #endif //MEDIAINFO_SEEK
            #if MEDIAINFO_DEMUX
                if (Config_Demux)
                {
                    if (Complete_Stream->Streams[pid]->stream_type==0x20 && Complete_Stream->Streams[pid]->SubStream_pid)
                    {
                        //Creating the demux buffer
                        ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->SubStream_Demux=new File_MpegPs::demux;
                        //If main parser is already created, associating the new demux buffer
                        if (Complete_Stream->Streams[Complete_Stream->Streams[pid]->SubStream_pid]->Parser)
                            ((File_MpegPs*)Complete_Stream->Streams[Complete_Stream->Streams[pid]->SubStream_pid]->Parser)->SubStream_Demux=((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->SubStream_Demux;
                    }
                    if (Complete_Stream->Streams[pid]->stream_type!=0x20 && Complete_Stream->Streams[pid]->SubStream_pid && (File_MpegPs*)Complete_Stream->Streams[Complete_Stream->Streams[pid]->SubStream_pid]->Parser)
                    {
                        //If SubStream parser is already created, associating the SubStream demux buffer
                        ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->SubStream_Demux=((File_MpegPs*)Complete_Stream->Streams[Complete_Stream->Streams[pid]->SubStream_pid]->Parser)->SubStream_Demux;
                    }
                }
            #endif //MEDIAINFO_DEMUX
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                if (Searching_TimeStamp_Start)
                    Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start_Set(true);
                ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->Searching_TimeStamp_Start=Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start;
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->FromTS=true;
            if (Config->File_MpegTs_stream_type_Trust_Get())
                ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->FromTS_stream_type=Complete_Stream->Streams[pid]->stream_type;
            ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->FromTS_descriptor_tag=Complete_Stream->Streams[pid]->descriptor_tag;
            if (!Complete_Stream->Streams[pid]->program_numbers.empty())
                ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->FromTS_program_format_identifier=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[0]].registration_format_identifier;
            ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->FromTS_format_identifier=Complete_Stream->Streams[pid]->registration_format_identifier;
            ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->MPEG_Version=2;
            complete_stream::transport_stream::iod_ess::iterator IOD_ES=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.find(Complete_Stream->Streams[pid]->FMC_ES_ID);
            if (IOD_ES!=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.end())
            {
                #ifdef MEDIAINFO_MPEG4_YES
                    ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->ParserFromTs=IOD_ES->second.Parser; IOD_ES->second.Parser=NULL;
                    ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->SLConfig=IOD_ES->second.SLConfig; IOD_ES->second.SLConfig=NULL;
                #endif
            }
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                Complete_Stream->Streams[pid]->Parser->ShouldContinueParsing=true;
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(true);
        #else
            //Filling
            Streams[pid]->Parser=new File_Unknown();
        #endif
        Complete_Stream->Streams[pid]->Parser->CA_system_ID_MustSkipSlices=Complete_Stream->Streams[pid]->CA_system_ID_MustSkipSlices;
        #if MEDIAINFO_IBI
            if (Ibi.Streams[pid]==NULL)
                Ibi.Streams[pid]=new ibi::stream;
            Complete_Stream->Streams[pid]->Parser->IbiStream=Ibi.Streams[pid];
        #endif //MEDIAINFO_IBI
        Open_Buffer_Init(Complete_Stream->Streams[pid]->Parser);
    }

    //If unsynched, waiting for first payload_unit_start_indicator
    if (!Complete_Stream->Streams[pid]->Parser->Synched && !payload_unit_start_indicator)
    {
        Element_DoNotShow(); //We don't want to show this item because there is no interessant info
        return; //This is not the start of the PES
    }

    //Parsing
    if (Complete_Stream->Streams[pid]->IsPCR)
        ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->FrameInfo.PCR=Complete_Stream->Streams[pid]->TimeStamp_End==(int64u)-1?(int64u)-1:Complete_Stream->Streams[pid]->TimeStamp_End*1000/27; //27 MHz
    #if MEDIAINFO_IBI
        if (Complete_Stream->transport_stream_id!=(int16u)-1 && !Complete_Stream->Streams[pid]->program_numbers.empty())
        {
            int16u Program_PID=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[0]].pid;
            Complete_Stream->Streams[pid]->Parser->Ibi_SynchronizationOffset_Current=Complete_Stream->Streams[Program_PID]->Ibi_SynchronizationOffset_BeginOfFrame;
            if (Complete_Stream->Streams[pid]->Parser->Ibi_SynchronizationOffset_Current==(int64u)-1)
                return; //Not yet synchronized
        }
        else
            Complete_Stream->Streams[pid]->Parser->Ibi_SynchronizationOffset_Current=File_Offset+Buffer_Offset-Header_Size;
    #endif //MEDIAINFO_IBI

    Open_Buffer_Continue(Complete_Stream->Streams[pid]->Parser);
    PES_Parse_Finish();
}

//---------------------------------------------------------------------------
void File_MpegTs::PES_Parse_Finish()
{
    if (Complete_Stream->Streams[pid]->Parser->Status[IsUpdated])
    {
        Complete_Stream->Streams[pid]->Parser->Status[IsUpdated]=false;
        Complete_Stream->Streams[pid]->IsUpdated_Info=true;
        for (size_t Pos=0; Pos<Complete_Stream->Streams[pid]->program_numbers.size(); Pos++)
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[Pos]].Update_Needed_IsRegistered=true;

        Status[IsUpdated]=true;
        Status[User_19]=true;
    }

    #if defined(MEDIAINFO_MPEGPS_YES) && defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
        if (MpegTs_JumpTo_Begin+MpegTs_JumpTo_End>File_Size
         && !Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End
         && ((File_MpegPs*)Complete_Stream->Streams[pid]->Parser)->HasTimeStamps)
        {
            Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start_Set(false);
            Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_End_Set(true);
        }
    #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES

    //Need anymore?
    if (Complete_Stream->Streams[pid]->Parser->Status[IsFilled]
     || Complete_Stream->Streams[pid]->Parser->Status[IsFinished])
    {
        if ((Complete_Stream->Streams[pid]->Searching_Payload_Start || Complete_Stream->Streams[pid]->Searching_Payload_Continue) && Config->ParseSpeed<1 && MpegTs_JumpTo_End)
        {
            Complete_Stream->Streams[pid]->Searching_Payload_Start_Set(false);
            Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(false);
            if (!Complete_Stream->Streams[pid]->IsParsed && Complete_Stream->Streams_NotParsedCount)
            {
                Complete_Stream->Streams[pid]->IsParsed=true;
                Complete_Stream->Streams_NotParsedCount--;
            }
        }
        #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            if (Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start)
                Complete_Stream->Streams[pid]->Searching_ParserTimeStamp_Start_Set(false);
        #else //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            if (Config->ParseSpeed<1.0)
                Finish(Complete_Stream->Streams[pid]->Parser);
        #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
    }

    #if MEDIAINFO_SEEK && MEDIAINFO_IBI
        if (Seek_ID!=(int64u)-1)
        {
            if (Ibi.Streams[Seek_ID]->IsModified)
            {
                Read_Buffer_Seek(2, Seek_Value, Seek_ID);
            }
            else if (Ibi.Streams[Seek_ID]->Infos_Pos>=2 && Ibi.Streams[Seek_ID]->IsSynchronized && Ibi.Streams[Seek_ID]->Infos[Ibi.Streams[Seek_ID]->Infos_Pos-1].StreamOffset>=Seek_Value_Maximal)
            {
                InfiniteLoop_Detect++;
                if (InfiniteLoop_Detect>16)
                {
                    //Infinite loop
                    Seek_ID=(int64u)-1;
                    Seek_Value=(int64u)-1;
                    InfiniteLoop_Detect=0;
                    Config->Demux_IsSeeking=false;
                }
                else
                {
                    //No intermediate seek point found, going to previous seek point
                    GoTo(Ibi.Streams[Seek_ID]->Infos[Ibi.Streams[Seek_ID]->Infos_Pos-2].StreamOffset);
                    Open_Buffer_Unsynch();
                }
            }
        }
    #endif //MEDIAINFO_SEEK && MEDIAINFO_IBI
}

//---------------------------------------------------------------------------
void File_MpegTs::PSI()
{
//    if (pid!=0 && pid!=0x101/*0x2f0*/)
    {
  //      Element_DoNotShow();
    //    return;
    }

    //Initializing
    if (payload_unit_start_indicator)
    {
        delete ((File_Mpeg_Psi*)Complete_Stream->Streams[pid]->Parser); Complete_Stream->Streams[pid]->Parser=new File_Mpeg_Psi;
        Open_Buffer_Init(Complete_Stream->Streams[pid]->Parser);
        ((File_Mpeg_Psi*)Complete_Stream->Streams[pid]->Parser)->Complete_Stream=Complete_Stream;
        ((File_Mpeg_Psi*)Complete_Stream->Streams[pid]->Parser)->pid=pid;
    }
    else if (Complete_Stream->Streams[pid]->Parser==NULL)
    {
        Skip_XX(Element_Size,                                   "data");
        return; //This is not the start of the PSI
    }

    //Parsing
    #if MEDIAINFO_IBI
        Complete_Stream->Streams[pid]->Parser->Ibi_SynchronizationOffset_Current=File_Offset+Buffer_Offset-Header_Size;
    #endif //MEDIAINFO_IBI
    Open_Buffer_Continue(Complete_Stream->Streams[pid]->Parser);

    //Filling
    if (Complete_Stream->Streams[pid]->Parser->Status[IsFilled])
    {
        //Accept
        if (!Status[IsAccepted] && pid==0x0000 && Complete_Stream->Streams[pid]->Parser->Status[IsAccepted])
            Accept("MPEG-TS");

        //Disabling this pid
        delete Complete_Stream->Streams[pid]->Parser; Complete_Stream->Streams[pid]->Parser=NULL;
        Complete_Stream->Streams[pid]->Searching_Payload_Start_Set(true);
        Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(false);

        //EPG
        if (Complete_Stream->Sources_IsUpdated || Complete_Stream->Programs_IsUpdated)
        {
            Status[IsUpdated]=true;
            Status[User_18]=true;
        }

        //Duration
        if (Complete_Stream->Duration_End_IsUpdated)
        {
            Status[IsUpdated]=true;
            Status[User_17]=true;
        }

        //Program change
        if (pid==0x0000)
        {
            Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset-Header_Size;
            Status[IsFilled]=false;

            Status[IsUpdated]=true;
            Status[User_19]=true;
        }
        if (!Complete_Stream->Streams[pid]->Table_IDs.empty() && Complete_Stream->Streams[pid]->Table_IDs[0x02])
        {
            Buffer_TotalBytes_LastSynched=Buffer_TotalBytes+Buffer_Offset-Header_Size;
            Status[IsFilled]=false;

            //Status[IsUpdated]=true;
            //Status[User_19]=true;
        }

        //Item removal
        if (pid==0x0000 || (!Complete_Stream->Streams[pid]->Table_IDs.empty() && Complete_Stream->Streams[pid]->Table_IDs[0x02]))
        {
            for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
                if (!Complete_Stream->StreamPos_ToRemove[StreamKind].empty())
                {
                    sort(Complete_Stream->StreamPos_ToRemove[StreamKind].begin(), Complete_Stream->StreamPos_ToRemove[StreamKind].end());
                    size_t Pos=Complete_Stream->StreamPos_ToRemove[StreamKind].size();
                    do
                    {
                        Pos--;

                        //Erasing text substreams
                        Ztring ID_ToFind=Retrieve((stream_t)StreamKind, Complete_Stream->StreamPos_ToRemove[StreamKind][Pos], General_ID)+__T('-');
                        for (size_t TextPos=0; TextPos<Count_Get(Stream_Text); TextPos++)
                            if (Retrieve(Stream_Text, TextPos, General_ID).find(ID_ToFind)==0)
                                 Stream_Erase(Stream_Text, TextPos);

                        //Erasing the stream
                        Stream_Erase((stream_t)StreamKind, Complete_Stream->StreamPos_ToRemove[StreamKind][Pos]);

                        //Moving other StreamPos
                        for (size_t Pos2=Pos+1; Pos2<Complete_Stream->StreamPos_ToRemove[StreamKind].size(); Pos2++)
                            Complete_Stream->StreamPos_ToRemove[StreamKind][Pos]--;

                        //Informing that the menu must be recalculated - TODO: only the related programs
                        if (StreamKind!=Stream_Menu)
                        {
                            for (complete_stream::transport_stream::programs::iterator Program=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.begin(); Program!=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.end(); ++Program)
                                Program->second.Update_Needed_StreamPos=true;
                        }
                        else if (Complete_Stream->StreamPos_ToRemove[StreamKind][Pos]<Complete_Stream->program_number_Order.size())
                        {
                            Complete_Stream->program_number_Order.erase(Complete_Stream->program_number_Order.begin()+Complete_Stream->StreamPos_ToRemove[StreamKind][Pos]);
                        }
                    }
                    while (Pos);
                    Complete_Stream->StreamPos_ToRemove[StreamKind].clear();
                }

            Status[IsUpdated]=true;
            Status[User_19]=true;
        }
    }
    else
        //Waiting for more data
        Complete_Stream->Streams[pid]->Searching_Payload_Continue_Set(true);
}

} //NameSpace

#endif //MEDIAINFO_MPEGTS_YES
