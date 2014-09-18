/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG files, Descriptors
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Mpeg_DescriptorsH
#define MediaInfo_Mpeg_DescriptorsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4_Descriptors.h"
#include "MediaInfo/Duplicate/File__Duplicate_MpegTs.h"
#if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
    #include "MediaInfo/File__Analyze.h"
#endif
#include <cfloat>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Global object
//***************************************************************************

struct complete_stream
{
    //Global
    int16u transport_stream_id; //The processed transport_stream_id
    bool   transport_stream_id_IsValid; //The processed transport_stream_id
    Ztring original_network_name;
    Ztring network_name;
    Ztring Duration_Start;
    Ztring Duration_End;
    bool   Duration_End_IsUpdated;
    std::map<Ztring, Ztring> TimeZones; //Key is country code

    //Per transport_stream
    struct transport_stream
    {
        bool HasChanged;
        std::map<std::string, Ztring> Infos;
        struct program
        {
            #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                File__Analyze::servicedescriptors* ServiceDescriptors;
            #endif
            bool HasChanged;
            std::map<std::string, Ztring> Infos;
            std::map<std::string, Ztring> ExtraInfos_Content;
            std::map<std::string, Ztring> ExtraInfos_Option;
            std::map<Ztring, Ztring> EPGs;
            std::vector<int16u> elementary_PIDs;
            size_t StreamPos; //Stream_Menu
            int32u registration_format_identifier;
            int16u pid;
            int16u PCR_PID;
            int16u source_id; //ATSC
            bool   source_id_IsValid;
            bool   IsParsed;
            bool   IsRegistered;
            bool   HasNotDisplayableStreams; //e.g. unknown stream, KLV, SCTE 35
            bool   Update_Needed_IsRegistered;
            bool   Update_Needed_StreamCount;
            bool   Update_Needed_StreamPos;
            bool   Update_Needed_Info;

            //DVB
            struct dvb_epg_block
            {
                struct event
                {
                    Ztring  start_time;
                    Ztring  duration;
                    struct short_event_
                    {
                        Ztring  event_name;
                        Ztring  text;
                    };
                    short_event_ short_event;
                    Ztring  content;
                    Ztring  running_status;
                };

                typedef std::map<int16u, event> events; //Key is event_id
                events Events; //Key is event_id
            };
            typedef std::map<int8u, dvb_epg_block> dvb_epg_blocks; //Key is table_id
            dvb_epg_blocks DVB_EPG_Blocks; //Key is table_id
            bool DVB_EPG_Blocks_IsUpdated;

            //SCTE 35
            struct scte35
            {
                struct segmentation
                {
                    struct segment
                    {
                        int8u Status; //If it is currently in the program: 0=Running, 1=Ended, 2=Early termination

                        segment()
                        {
                            Status=(int8u)-1;
                        }
                    };

                    typedef std::map<int8u, segment> segments; //Key is segmentation_type_id
                    segments Segments;
                    ;
                };

                typedef std::map<int32u, segmentation> segmentations; //Key is segmentation_event_id
                segmentations Segmentations;
                int16u  pid;

                scte35()
                {
                    pid=(int16u)-1;
                }
            };
            scte35* Scte35;

            //Constructor/Destructor
            program()
            :
                #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                    ServiceDescriptors(NULL),
                #endif
                HasChanged(false),
                StreamPos((size_t)-1),
                registration_format_identifier(0x00000000),
                pid(0x00000),
                PCR_PID(0x0000),
                source_id((int16u)-1),
                source_id_IsValid(false),
                IsParsed(false),
                IsRegistered(false),
                HasNotDisplayableStreams(false),
                Update_Needed_IsRegistered(false),
                Update_Needed_StreamCount(false),
                Update_Needed_StreamPos(false),
                Update_Needed_Info(false),
                DVB_EPG_Blocks_IsUpdated(false),
                Scte35(NULL)
            {}

            program(const program& p)
            :
                HasChanged(p.HasChanged),
                Infos(p.Infos),
                ExtraInfos_Content(p.ExtraInfos_Content),
                ExtraInfos_Option(p.ExtraInfos_Option),
                EPGs(p.EPGs),
                elementary_PIDs(p.elementary_PIDs),
                StreamPos(p.StreamPos),
                registration_format_identifier(p.registration_format_identifier),
                pid(p.pid),
                PCR_PID(p.PCR_PID),
                source_id(p.source_id),
                source_id_IsValid(p.source_id_IsValid),
                IsParsed(p.IsParsed),
                IsRegistered(p.IsRegistered),
                HasNotDisplayableStreams(p.HasNotDisplayableStreams),
                Update_Needed_IsRegistered(p.Update_Needed_IsRegistered),
                Update_Needed_StreamCount(p.Update_Needed_StreamCount),
                Update_Needed_StreamPos(p.Update_Needed_StreamPos),
                Update_Needed_Info(p.Update_Needed_Info),
                DVB_EPG_Blocks_IsUpdated(p.DVB_EPG_Blocks_IsUpdated),
                Scte35(p.Scte35)
            {
                #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                    if (p.ServiceDescriptors)
                    {
                        ServiceDescriptors=new File__Analyze::servicedescriptors;
                        *ServiceDescriptors=*p.ServiceDescriptors;
                    }
                    else
                        ServiceDescriptors=NULL;
                #endif
            }

            program& operator=(const program& p)
            {
                #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                    if (p.ServiceDescriptors)
                    {
                        ServiceDescriptors=new File__Analyze::servicedescriptors;
                        *ServiceDescriptors=*p.ServiceDescriptors;
                    }
                    else
                        ServiceDescriptors=NULL;
                #endif
                HasChanged=p.HasChanged;
                Infos=p.Infos;
                ExtraInfos_Content=p.ExtraInfos_Content;
                ExtraInfos_Option=p.ExtraInfos_Option;
                EPGs=p.EPGs;
                elementary_PIDs=p.elementary_PIDs;
                StreamPos=p.StreamPos;
                registration_format_identifier=p.registration_format_identifier;
                pid=p.pid;
                PCR_PID=p.PCR_PID;
                source_id=p.source_id;
                source_id_IsValid=p.source_id_IsValid;
                IsParsed=p.IsParsed;
                IsRegistered=p.IsRegistered;
                HasNotDisplayableStreams=p.HasNotDisplayableStreams;
                Update_Needed_IsRegistered=p.Update_Needed_IsRegistered;
                Update_Needed_StreamCount=p.Update_Needed_StreamCount;
                Update_Needed_StreamPos=p.Update_Needed_StreamPos;
                Update_Needed_Info=p.Update_Needed_Info;
                DVB_EPG_Blocks_IsUpdated=p.DVB_EPG_Blocks_IsUpdated;
                Scte35=p.Scte35;

                return *this;
            }

            ~program()
            {
                #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                    delete ServiceDescriptors;
                #endif
            }
        };
        typedef std::map<int16u, program> programs; //Key is program_number
        programs Programs; //Key is program_number
        std::vector<int16u> programs_List;
        size_t   Programs_NotParsedCount;

        //Per IOD
        struct iod_es
        {
            File__Analyze*                                  Parser;
            #ifdef MEDIAINFO_MPEG4_YES
                File_Mpeg4_Descriptors::slconfig*           SLConfig;
            #endif

            //Constructor/Destructor
            iod_es()
            {
                Parser=NULL;
                #ifdef MEDIAINFO_MPEG4_YES
                    SLConfig=NULL;
                #endif
            }

            ~iod_es()
            {
                delete Parser; //Parser=NULL;
                #ifdef MEDIAINFO_MPEG4_YES
                    delete SLConfig; //SLConfig=NULL;
                #endif
            }
        };
        typedef std::map<int16u, iod_es> iod_ess; //Key is ES_ID
        std::map<int16u, iod_es> IOD_ESs; //Key is ES_ID

        //ATSC
        int16u source_id; //Global
        int16u source_id_IsValid;

        transport_stream()
        {
            HasChanged=false;
            source_id=(int16u)-1;
            source_id_IsValid=false;
            Programs_NotParsedCount=(size_t)-1;
        }
    };
    typedef std::map<int16u, transport_stream> transport_streams; //Key is transport_stream_id
    transport_streams Transport_Streams; //Key is transport_stream_id

    //Per pid
    struct stream
    {
        File__Analyze*                              Parser;

        enum ts_kind
        {
            //MPEG
            unknown,
            pes,
            psi,
            ts_kind_Max,
        };
        std::vector<int16u>                         program_numbers;
        struct table_id
        {
            struct table_id_extension
            {
                typedef std::vector<bool>           section_numbers; //Key is section_number
                section_numbers                     Section_Numbers; //Key is section_number
                int8u version_number;
            };
            typedef std::map<int16u, table_id_extension> table_id_extensions; //Key is table_id_extensions
            table_id_extensions                     Table_ID_Extensions; //Key is table_id_extensions
            bool                                    Table_ID_Extensions_CanAdd;

            table_id()
            {
                Table_ID_Extensions_CanAdd=true;
            }
        };
        typedef std::vector<table_id*>              table_ids;
        table_ids                                   Table_IDs; //Key is table_id
        std::map<std::string, Ztring>               Infos;
        std::map<std::string, Ztring>               Infos_Option;
        struct teletext
        {
            std::map<std::string, Ztring>           Infos;
        };
        std::map<int16u, teletext>                  Teletexts; //Key is teletext_magazine_number
        #if MEDIAINFO_TRACE
            Ztring Element_Info1;
        #endif //MEDIAINFO_TRACE
        stream_t                                    StreamKind;
        stream_t                                    StreamKind_FromDescriptor;
        size_t                                      StreamPos;
        ts_kind                                     Kind;
        bool                                        IsParsed;
        bool                                        IsPCR;
        float64                                     IsPCR_Duration;
        #ifdef MEDIAINFO_MPEGTS_PCR_YES
            int64u                                  TimeStamp_Start;
            int64u                                  TimeStamp_Start_Offset;
            int64u                                  TimeStamp_End;
            int64u                                  TimeStamp_End_Offset;
            int16u                                  PCR_PID; //If this pid has no PCR, decide which PCR should be used
            bool                                    TimeStamp_End_IsUpdated;
            float64                                 TimeStamp_InstantaneousBitRate_Current_Min;
            float64                                 TimeStamp_InstantaneousBitRate_Current_Raw;
            float64                                 TimeStamp_InstantaneousBitRate_Current_Max;
            int64u                                  TimeStamp_InstantaneousBitRate_BitRateMode_IsCbr;
            int64u                                  TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr;
            #if MEDIAINFO_ADVANCED
                float64                             TimeStamp_InstantaneousBitRate_Min_Raw;
                float64                             TimeStamp_InstantaneousBitRate_Max_Raw;
                int64u                              TimeStamp_Distance_Min;
                int64u                              TimeStamp_Distance_Max;
                int64u                              TimeStamp_Distance_Total;
                int64u                              TimeStamp_Distance_Count;
                int64u                              TimeStamp_HasProblems;
                std::vector<int64u>                 TimeStamp_Intermediate;
            #endif // MEDIAINFO_ADVANCED
        #endif //MEDIAINFO_MPEGTS_PCR_YES
        int32u                                      registration_format_identifier;
        int16u                                      FMC_ES_ID;
        int16u                                      table_type; //ATSC
        int8u                                       stream_type;
        int8u                                       descriptor_tag;
        int8u                                       DtsNeural_config_id;
        bool                                        FMC_ES_ID_IsValid;
        bool                                        Searching;
        bool                                        Searching_Payload_Start;
        bool                                        Searching_Payload_Continue;
        #ifdef MEDIAINFO_MPEGTS_PCR_YES
            bool                                    Searching_TimeStamp_Start;
            bool                                    Searching_TimeStamp_End;
        #endif //MEDIAINFO_MPEGTS_PCR_YES
        #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            bool                                    Searching_ParserTimeStamp_Start;
            bool                                    Searching_ParserTimeStamp_End;
        #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
        bool                                        EndTimeStampMoreThanxSeconds;
        bool                                        ShouldDuplicate;
        bool                                        IsRegistered;
        bool                                        IsUpdated_IsRegistered;
        bool                                        IsUpdated_Info;
        bool                                        CA_system_ID_MustSkipSlices;
        bool                                        EBP_IsPresent;
        size_t                                      IsScrambled;
        int16u                                      CA_system_ID;
        int16u                                      SubStream_pid;
        #if MEDIAINFO_IBI
            int64u                                  Ibi_SynchronizationOffset_BeginOfFrame;
        #endif //MEDIAINFO_IBI
        #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
            File__Analyze::servicedescriptors ServiceDescriptors;
            bool                              ServiceDescriptors_IsPresent;
        #endif

        //Constructor/Destructor
        stream()
        {
            Parser=NULL;
            StreamKind=Stream_Max;
            StreamKind_FromDescriptor=Stream_Max;
            StreamPos=(size_t)-1;
            Kind=unknown;
            IsParsed=false;
            IsPCR=false;
            IsPCR_Duration=0;
            #ifdef MEDIAINFO_MPEGTS_PCR_YES
                TimeStamp_Start=(int64u)-1;
                TimeStamp_Start_Offset=(int64u)-1;
                TimeStamp_End=(int64u)-1;
                TimeStamp_End_Offset=(int64u)-1;
                PCR_PID=0x0000;
                TimeStamp_End_IsUpdated=false;
                TimeStamp_InstantaneousBitRate_Current_Min=0;
                TimeStamp_InstantaneousBitRate_Current_Raw=0;
                TimeStamp_InstantaneousBitRate_Current_Max=0;
                TimeStamp_InstantaneousBitRate_BitRateMode_IsCbr=0;
                TimeStamp_InstantaneousBitRate_BitRateMode_IsVbr=0;
                #if MEDIAINFO_ADVANCED
                    TimeStamp_InstantaneousBitRate_Min_Raw=DBL_MAX;
                    TimeStamp_InstantaneousBitRate_Max_Raw=0;
                    TimeStamp_Distance_Min=(int64u)-1;
                    TimeStamp_Distance_Max=0;
                    TimeStamp_Distance_Total=0;
                    TimeStamp_Distance_Count=0;
                    TimeStamp_HasProblems=0;
                #endif // MEDIAINFO_ADVANCED
            #endif //MEDIAINFO_MPEGTS_PCR_YES
            registration_format_identifier=0x00000000;
            FMC_ES_ID=0x0000;
            table_type=0x0000;
            stream_type=(int8u)-1;
            descriptor_tag=(int8u)-1;
            DtsNeural_config_id=(int8u)-1;
            FMC_ES_ID_IsValid=false;
            Searching=false;
            Searching_Payload_Start=false;
            Searching_Payload_Continue=false;
            #ifdef MEDIAINFO_MPEGTS_PCR_YES
                Searching_TimeStamp_Start=false;
                Searching_TimeStamp_End=false;
            #endif //MEDIAINFO_MPEGTS_PCR_YES
            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                Searching_ParserTimeStamp_Start=false;
                Searching_ParserTimeStamp_End=false;
            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            EndTimeStampMoreThanxSeconds=false;
            ShouldDuplicate=false;
            IsRegistered=false;
            IsUpdated_IsRegistered=false;
            IsUpdated_Info=false;
            IsScrambled=false;
            CA_system_ID_MustSkipSlices=false;
            CA_system_ID=0x0000;
            EBP_IsPresent=false;
            SubStream_pid=0x0000;
            #if MEDIAINFO_IBI
                Ibi_SynchronizationOffset_BeginOfFrame=(int64u)-1;
            #endif //MEDIAINFO_IBI
            #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                ServiceDescriptors_IsPresent=false;
            #endif
        }

        ~stream()
        {
            delete Parser; //Parser=NULL;
            for (size_t Pos=0; Pos<Table_IDs.size(); Pos++)
                delete Table_IDs[Pos]; //Table_IDs[Pos]=NULL;
        }

        //Helpers
        void Searching_Payload_Start_Set(bool ToSet)
        {
            Searching_Payload_Start=ToSet;
            Searching_Test();
        }
        void Searching_Payload_Continue_Set(bool ToSet)
        {
            Searching_Payload_Continue=ToSet;
            Searching_Test();
        }
        #ifdef MEDIAINFO_MPEGTS_PCR_YES
            void Searching_TimeStamp_Start_Set(bool ToSet)
            {
                Searching_TimeStamp_Start=ToSet;
                Searching_Test();
            }
            void Searching_TimeStamp_End_Set(bool ToSet)
            {
                Searching_TimeStamp_End=ToSet;
                Searching_Test();
            }
        #endif //MEDIAINFO_MPEGTS_PCR_YES
        #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            void Searching_ParserTimeStamp_Start_Set(bool ToSet)
            {
                Searching_ParserTimeStamp_Start=ToSet;
                Searching_Test();
            }
            void Searching_ParserTimeStamp_End_Set(bool ToSet)
            {
                Searching_ParserTimeStamp_End=ToSet;
                Searching_Test();
            }
        #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
        void Searching_Test()
        {
            Searching=Searching_Payload_Start
                    | Searching_Payload_Continue
                    #ifdef MEDIAINFO_MPEGTS_PCR_YES
                    | Searching_TimeStamp_Start
                    | Searching_TimeStamp_End
                    #endif //MEDIAINFO_MPEGTS_PCR_YES
                    #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                        | Searching_ParserTimeStamp_Start
                        | Searching_ParserTimeStamp_End
                    #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                    ;
        }
    };
    typedef std::vector<stream*> streams;
    streams Streams; //Key is pid
    size_t Streams_NotParsedCount;
    size_t Streams_With_StartTimeStampCount;
    size_t Streams_With_EndTimeStampMoreThanxSecondsCount;

    //ATSC
    int8u GPS_UTC_offset;
    struct source
    {
        std::map<int16u, Ztring> texts;
        struct atsc_epg_block
        {
            struct event
            {
                #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                    File__Analyze::servicedescriptors* ServiceDescriptors;
                #endif
                int32u  start_time;
                Ztring  duration;
                Ztring  title;
                std::map<int16u, Ztring> texts;

                event()
                :
                    #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                        ServiceDescriptors(NULL),
                    #endif
                    start_time((int32u)-1)
                {}

                event(const event& e)
                :
                    start_time(e.start_time)
                {
                    #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                        if (e.ServiceDescriptors)
                        {
                            ServiceDescriptors=new File__Analyze::servicedescriptors;
                            *ServiceDescriptors=*e.ServiceDescriptors;
                        }
                        else
                            ServiceDescriptors=NULL;
                    #endif
                }

                event& operator=(const event& e)
                {
                    #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                        if (e.ServiceDescriptors)
                        {
                            ServiceDescriptors=new File__Analyze::servicedescriptors;
                            *ServiceDescriptors=*e.ServiceDescriptors;
                        }
                        else
                            ServiceDescriptors=NULL;
                    #endif
                    start_time=e.start_time;

                    return *this;
                }

                ~event()
                {
                    #if defined(MEDIAINFO_EIA608_YES) || defined(MEDIAINFO_EIA708_YES)
                        delete ServiceDescriptors;
                    #endif
                }
            };

            typedef std::map<int16u, event> events; //Key is event_id
            events Events; //Key is event_id
        };
        typedef std::map<int16u, atsc_epg_block> atsc_epg_blocks; //Key is table_id
        atsc_epg_blocks ATSC_EPG_Blocks; //Key is table_id
        bool ATSC_EPG_Blocks_IsUpdated;

        source()
        {
            ATSC_EPG_Blocks_IsUpdated=false;
        }
    };
    typedef std::map<int16u, source> sources; //Key is source_id
    sources Sources; //Key is source_id
    bool Sources_IsUpdated; //For EPG ATSC
    bool Programs_IsUpdated; //For EPG DVB
    bool NoPatPmt;

    //File__Duplicate
    bool                                                File__Duplicate_HasChanged_;
    size_t                                              Config_File_Duplicate_Get_AlwaysNeeded_Count;
    std::vector<File__Duplicate_MpegTs*>                Duplicates_Speed;
    std::vector<std::vector<File__Duplicate_MpegTs*> >  Duplicates_Speed_FromPID;
    std::map<const String, File__Duplicate_MpegTs*>     Duplicates;
    bool File__Duplicate_Get_From_PID (int16u pid)
    {
        if (Duplicates_Speed_FromPID.empty())
            return false;
        return !Duplicates_Speed_FromPID[pid].empty();
    }

    //SpeedUp information
    std::vector<std::vector<size_t> >   StreamPos_ToRemove;
    std::map<int16u, int16u>            PCR_PIDs; //Key is PCR_PID, value is count of programs using it
    std::set<int16u>                    PES_PIDs; //Key is pid
    std::vector<int16u>                 program_number_Order;

    //Constructor/Destructor
    complete_stream()
    {
        transport_stream_id=(int16u)-1;
        transport_stream_id_IsValid=false;
        Duration_End_IsUpdated=false;
        Streams_NotParsedCount=(size_t)-1;
        Streams_With_StartTimeStampCount=0;
        Streams_With_EndTimeStampMoreThanxSecondsCount=0;
        GPS_UTC_offset=0;
        Sources_IsUpdated=false;
        Programs_IsUpdated=false;
        NoPatPmt=false;
        StreamPos_ToRemove.resize(Stream_Max);
        File__Duplicate_HasChanged_ = false;
        Config_File_Duplicate_Get_AlwaysNeeded_Count = 0;
    }

    ~complete_stream()
    {
        for (size_t StreamID=0; StreamID<Streams.size(); StreamID++)
            delete Streams[StreamID]; //Streams[StreamID]=NULL;

        std::map<const String, File__Duplicate_MpegTs*>::iterator Duplicates_Temp=Duplicates.begin();
        while (Duplicates_Temp!=Duplicates.end())
        {
            delete Duplicates_Temp->second; //Duplicates_Temp->second=NULL
            ++Duplicates_Temp;
        }
    }
};

//***************************************************************************
// Class File_Mpeg_Descriptors
//***************************************************************************

class File_Mpeg_Descriptors : public File__Analyze
{
public :
    //In
    complete_stream* Complete_Stream;
    int16u transport_stream_id;
    int16u pid;
    int8u  table_id;
    int16u table_id_extension;
    int16u elementary_PID;
    int16u program_number;
    int32u registration_format_identifier;
    int8u  stream_type;
    int16u event_id;
    bool   elementary_PID_IsValid;
    bool   program_number_IsValid;
    bool   registration_format_identifier_IsValid;
    bool   stream_type_IsValid;
    bool   event_id_IsValid;

    //Out

    //Constructor/Destructor
    File_Mpeg_Descriptors();

private :
    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Descriptors();
    void Descriptor();
    void Descriptor_00() {Skip_XX(Element_Size, "Data");};
    void Descriptor_01() {Skip_XX(Element_Size, "Data");};
    void Descriptor_02();
    void Descriptor_03();
    void Descriptor_04() {Skip_XX(Element_Size, "Data");};
    void Descriptor_05();
    void Descriptor_06();
    void Descriptor_07();
    void Descriptor_08();
    void Descriptor_09();
    void Descriptor_0A();
    void Descriptor_0B();
    void Descriptor_0C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_0D();
    void Descriptor_0E();
    void Descriptor_0F();
    void Descriptor_10();
    void Descriptor_11();
    void Descriptor_12() {Skip_XX(Element_Size, "Data");};
    void Descriptor_13() {Skip_XX(Element_Size, "Data");};
    void Descriptor_14() {Skip_XX(Element_Size, "Data");};
    void Descriptor_15() {Skip_XX(Element_Size, "Data");};
    void Descriptor_16() {Skip_XX(Element_Size, "Data");};
    void Descriptor_17() {Skip_XX(Element_Size, "Data");};
    void Descriptor_18() {Skip_XX(Element_Size, "Data");};
    void Descriptor_19() {Skip_XX(Element_Size, "Data");};
    void Descriptor_1A() {Skip_XX(Element_Size, "Data");};
    void Descriptor_1B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_1C();
    void Descriptor_1D();
    void Descriptor_1E() {Skip_XX(Element_Size, "Data");};
    void Descriptor_1F();
    void Descriptor_20() {Skip_XX(Element_Size, "Data");};
    void Descriptor_21() {Skip_XX(Element_Size, "Data");};
    void Descriptor_22() {Skip_XX(Element_Size, "Data");};
    void Descriptor_23() {Skip_XX(Element_Size, "Data");};
    void Descriptor_24() {Skip_XX(Element_Size, "Data");};
    void Descriptor_25() {Skip_XX(Element_Size, "Data");};
    void Descriptor_26() {Skip_XX(Element_Size, "Data");};
    void Descriptor_27() {Skip_XX(Element_Size, "Data");};
    void Descriptor_28();
    void Descriptor_29() {Skip_XX(Element_Size, "Data");};
    void Descriptor_2A();
    void Descriptor_2B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_2C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_2D() {Skip_XX(Element_Size, "Data");};
    void Descriptor_2E() {Skip_XX(Element_Size, "Data");};
    void Descriptor_2F();
    void Descriptor_30() {Skip_XX(Element_Size, "Data");};
    void Descriptor_31() {Skip_XX(Element_Size, "Data");};
    void Descriptor_32() {Skip_XX(Element_Size, "Data");};
    void Descriptor_33() {Skip_XX(Element_Size, "Data");};
    void Descriptor_34() {Skip_XX(Element_Size, "Data");};
    void Descriptor_35() {Skip_XX(Element_Size, "Data");};
    void Descriptor_36() {Skip_XX(Element_Size, "Data");};
    void Descriptor_37() {Skip_XX(Element_Size, "Data");};
    void Descriptor_38() {Skip_XX(Element_Size, "Data");};
    void Descriptor_39() {Skip_XX(Element_Size, "Data");};
    void Descriptor_3A() {Skip_XX(Element_Size, "Data");};
    void Descriptor_3F() {Skip_XX(Element_Size, "Data");};
    void Descriptor_40();
    void Descriptor_41();
    void Descriptor_42() {Skip_XX(Element_Size, "Data");};
    void Descriptor_43();
    void Descriptor_44() {Skip_XX(Element_Size, "Data");};
    void Descriptor_45() {Skip_XX(Element_Size, "Data");};
    void Descriptor_46() {Skip_XX(Element_Size, "Data");};
    void Descriptor_47() {Skip_XX(Element_Size, "Data");};
    void Descriptor_48();
    void Descriptor_49() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4A();
    void Descriptor_4B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4D();
    void Descriptor_4E() {Skip_XX(Element_Size, "Data");};
    void Descriptor_4F() {Skip_XX(Element_Size, "Data");};
    void Descriptor_50();
    void Descriptor_51() {Skip_XX(Element_Size, "Data");};
    void Descriptor_52();
    void Descriptor_53() {Skip_XX(Element_Size, "Data");};
    void Descriptor_54();
    void Descriptor_55();
    void Descriptor_56();
    void Descriptor_57() {Skip_XX(Element_Size, "Data");};
    void Descriptor_58();
    void Descriptor_59();
    void Descriptor_5A();
    void Descriptor_5B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_5C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_5D();
    void Descriptor_5E() {Skip_XX(Element_Size, "Data");};
    void Descriptor_5F();
    void Descriptor_60() {Skip_XX(Element_Size, "Data");};
    void Descriptor_61() {Skip_XX(Element_Size, "Data");};
    void Descriptor_62() {Skip_XX(Element_Size, "Data");};
    void Descriptor_63();
    void Descriptor_64() {Skip_XX(Element_Size, "Data");};
    void Descriptor_65() {Skip_XX(Element_Size, "Data");};
    void Descriptor_66();
    void Descriptor_67() {Skip_XX(Element_Size, "Data");};
    void Descriptor_68() {Skip_XX(Element_Size, "Data");};
    void Descriptor_69() {Skip_XX(Element_Size, "Data");};
    void Descriptor_6A();
    void Descriptor_6B() {Skip_XX(Element_Size, "Data");};
    void Descriptor_6C() {Skip_XX(Element_Size, "Data");};
    void Descriptor_6D() {Skip_XX(Element_Size, "Data");};
    void Descriptor_6E() {Skip_XX(Element_Size, "Data");};
    void Descriptor_6F() {Skip_XX(Element_Size, "Data");};
    void Descriptor_70() {Skip_XX(Element_Size, "Data");};
    void Descriptor_71() {Skip_XX(Element_Size, "Data");};
    void Descriptor_72() {Skip_XX(Element_Size, "Data");};
    void Descriptor_73() {Skip_XX(Element_Size, "Data");};
    void Descriptor_74() {Skip_XX(Element_Size, "Data");};
    void Descriptor_75() {Skip_XX(Element_Size, "Data");};
    void Descriptor_76() {Skip_XX(Element_Size, "Data");};
    void Descriptor_77() {Skip_XX(Element_Size, "Data");};
    void Descriptor_78() {Skip_XX(Element_Size, "Data");};
    void Descriptor_79() {Skip_XX(Element_Size, "Data");};
    void Descriptor_7A();
    void Descriptor_7B();
    void Descriptor_7C();
    void Descriptor_7D() {Skip_XX(Element_Size, "Data");};
    void Descriptor_7E() {Skip_XX(Element_Size, "Data");};
    void Descriptor_7F();
    void Descriptor_7F_0F();
    void Descriptor_80() {Skip_XX(Element_Size, "Data");};
    void Descriptor_81();
    void Descriptor_86();
    void Descriptor_87();
    void Descriptor_A0();
    void Descriptor_A1();
    void Descriptor_A2() {Skip_XX(Element_Size, "Data");};
    void Descriptor_A3();
    void Descriptor_A8() {Skip_XX(Element_Size, "Data");};
    void Descriptor_A9() {Skip_XX(Element_Size, "Data");};
    void Descriptor_AA();
    void Descriptor_AB() {Skip_XX(Element_Size, "Data");};
    void Descriptor_C1();
    void Descriptor_C4() {Skip_XX(Element_Size, "Data");};
    void Descriptor_C8();
    void Descriptor_DE();
    void Descriptor_E9();
    void Descriptor_FC();
    void Descriptor_FD();

    //SCTE 35
    void CUEI_00();
    void CUEI_01();
    void CUEI_02();

    //Helpers
    void ATSC_multiple_string_structure(Ztring &Value, const char* Info);
    void Get_DVB_Text(int64u Size, Ztring &Value, const char* Info);
    void Skip_DVB_Text(int64u Size, const char* Info) {Ztring Temp; Get_DVB_Text(Size, Temp, Info);};
    Ztring Date_MJD(int16u Date);
    Ztring Time_BCD(int32u Time);
    Ztring TimeHHMM_BCD(int16u Time);
    Ztring Frequency_DVB__BCD(int32u Frequency);
    Ztring OrbitalPosition_DVB__BCD(int32u OrbitalPosition);
};

} //NameSpace

#endif
