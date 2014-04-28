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
#if defined(MEDIAINFO_MPEGPS_YES) || defined(MEDIAINFO_MPEGTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg_Psi.h"
#include "MediaInfo/Multiple/File_Mpeg_Descriptors.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "ZenLib/Dir.h"
#include <memory>
#include <algorithm>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

namespace Elements
{
    const int32u CUEI=0x43554549; //SCTE
    const int32u GA94=0x47413934; //ATSC - Terrestrial
    const int32u HDMV=0x48444D56; //BluRay
    const int32u S14A=0x53313441; //ATSC - Satellite
    const int32u SCTE=0x53435445; //SCTE
    const int32u TSHV=0x54534856; //TSHV
}

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mpeg_Psi_ATSC_table_type(int16u ID)
{
    switch (ID)
    {
        case 0x0000 : return "Terrestrial VCT with current_next_indicator=1";
        case 0x0001 : return "Terrestrial VCT with current_next_indicator=0";
        case 0x0002 : return "Cable VCT with current_next_indicator=1";
        case 0x0003 : return "Cable VCT with current_next_indicator==0";
        case 0x0004 : return "Channel ETT";
        case 0x0005 : return "DCCSCT";
        case 0x0010 : return "Short-form Virtual Channel Table-VCM Subtyp";
        case 0x0011 : return "Short-form Virtual Channel Table-DCM Subtyp";
        case 0x0012 : return "Short-form Virtual Channel Table-ICM Subtyp";
        case 0x0020 : return "Network Information Table-CDS Table Subtype";
        case 0x0021 : return "Network Information Table-MMS Table Subtype";
        case 0x0030 : return "Network Text Tabl e-SNS Subtype";
        default :
            if (ID>=0x0100
             && ID<=0x017F) return "Event Information Table (EIT)";
            if (ID>=0x0200
             && ID<=0x027F) return "Event Extended Text Table (EETT)";
            if (ID>=0x0300
             && ID<=0x03FF) return "Rating Region Table (RRT)";
            if (ID>=0x0400
             && ID<=0x0FFF) return "User private";
            if (ID>=0x1000
             && ID<=0x10FF) return "Aggregate Event Information Table (AEIT)";
            if (ID>=0x1100
             && ID<=0x11FF) return "Aggregate Extended Text Table (AETT)";
            if (ID>=0x1400
             && ID<=0x14FF) return "DCCT";
            return "Reserved";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Psi_stream_type_Format(int8u stream_type, int32u format_identifier)
{
    switch (stream_type)
    {
        case 0x01 : return "MPEG Video"; //Version 1
        case 0x02 : return "MPEG Video"; //Version 2
        case 0x03 : return "MPEG Audio"; //Version 1
        case 0x04 : return "MPEG Audio"; //Version 2
        case 0x0F : return "AAC";
        case 0x10 : return "MPEG-4 Visual";
        case 0x11 : return "AAC";
        case 0x1B : return "AVC";
        case 0x1C : return "AAC";
        case 0x1D : return "Timed Text";
        case 0x1E : return "MPEG Video"; //ISO/IEC 23002-3
        case 0x1F : return "AVC";
        case 0x20 : return "AVC";
        case 0x24 :
        case 0x27 : return "HEVC";
        default :
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::SCTE : //SCTE
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                        switch (stream_type)
                        {
                            case 0x80 : return "MPEG Video";
                            case 0x81 : return "AC-3";
                            case 0x82 : return "Text";
                            case 0x86 : return "SCTE 35";
                            case 0x87 : return "E-AC-3";
                            default   : return "";
                        }
                case Elements::HDMV : //Bluray
                        switch (stream_type)
                        {
                            case 0x80 : return "PCM";
                            case 0x81 : return "AC-3";
                            case 0x82 : return "DTS";
                            case 0x83 : return "AC-3"; // (TrueHD)"
                            case 0x84 : return "E-AC-3";
                            case 0x85 : return "DTS"; //" (HD-HRA)"
                            case 0x86 : return "DTS"; //" (HD-MA)"
                            case 0x90 : return "PGS";
                            case 0xA1 : return "AC-3";
                            case 0xA2 : return "DTS";
                            case 0xEA : return "VC-1";
                            default   : return "";
                        }
                case 0xFFFFFFFF : //Unknown
                        return "";
                default                     :
                        switch (stream_type)
                        {
                            case 0x80 : return "MPEG Video";
                            case 0x81 : return "AC-3";
                            case 0x87 : return "E-AC-3";
                            case 0x88 : return "VC-1";
                            case 0xD1 : return "Dirac";
                            default   : return "";
                        }
            }
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Psi_stream_type_Codec(int8u stream_type, int32u format_identifier)
{
    switch (stream_type)
    {
        case 0x01 : return "MPEG-1V";
        case 0x02 : return "MPEG-2V";
        case 0x03 : return "MPEG-1A";
        case 0x04 : return "MPEG-2A";
        case 0x0F : return "AAC";
        case 0x10 : return "MPEG-4V";
        case 0x11 : return "AAC";
        case 0x1B : return "AVC";
        case 0x1C : return "AAC";
        case 0x1D : return "Text";
        case 0x1E : return "MPEG-2V";
        case 0x1F : return "AVC";
        case 0x20 : return "AVC";
        case 0x24 :
        case 0x27 : return "HEVC";
        default :
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::SCTE : //SCTE
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                        switch (stream_type)
                        {
                            case 0x80 : return "MPEG-2V";
                            case 0x81 : return "AC3";
                            case 0x82 : return "Text";
                            case 0x87 : return "AC3+";
                            default   : return "";
                        }
                case Elements::HDMV : //Bluray
                        switch (stream_type)
                        {
                            case 0x80 : return "PCM";
                            case 0x81 : return "AC3";
                            case 0x82 : return "DTS";
                            case 0x83 : return "AC3+";
                            case 0x86 : return "DTS";
                            case 0x90 : return "PGS";
                            case 0xEA : return "VC1";
                            default   : return "";
                        }
                case 0xFFFFFFFF : //Unknown
                        return "";
                default                     :
                        switch (stream_type)
                        {
                            case 0x80 : return "MPEG-2V";
                            case 0x81 : return "AC3";
                            case 0x87 : return "AC3+";
                            case 0x88 : return "VC-1";
                            case 0xD1 : return "Dirac";
                            default   : return "";
                        }
            }
    }
}

//---------------------------------------------------------------------------
stream_t Mpeg_Psi_stream_type_StreamKind(int32u stream_type, int32u format_identifier)
{
    switch (stream_type)
    {
        case 0x01 :
        case 0x02 :
        case 0x10 :
        case 0x1B :
        case 0x1E :
        case 0x1F :
        case 0x20 :
        case 0x24 :
        case 0x27 :
                    return Stream_Video;
        case 0x03 :
        case 0x04 :
        case 0x0F :
        case 0x11 :
        case 0x1C :
                    return Stream_Audio;
        case 0x1D :
                    return Stream_Text;
        default :
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::SCTE : //SCTE
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                        switch (stream_type)
                        {
                            case 0x80 : return Stream_Video;
                            case 0x81 : return Stream_Audio;
                            case 0x82 : return Stream_Text;
                            case 0x87 : return Stream_Audio;
                            default   : return Stream_Max;
                        }
                case Elements::HDMV : //Bluray
                        switch (stream_type)
                        {
                            case 0x80 : return Stream_Audio;
                            case 0x81 : return Stream_Audio;
                            case 0x82 : return Stream_Audio;
                            case 0x83 : return Stream_Audio;
                            case 0x84 : return Stream_Audio;
                            case 0x85 : return Stream_Audio;
                            case 0x86 : return Stream_Audio;
                            case 0x90 : return Stream_Text;
                            case 0xA1 : return Stream_Audio;
                            case 0xA2 : return Stream_Audio;
                            case 0xEA : return Stream_Video;
                            default   : return Stream_Max;
                        }
                case Elements::TSHV : //DV
                        switch (stream_type)
                        {
                            case 0xA0 : return Stream_General;
                            case 0xA1 : return Stream_General;
                            default   : return Stream_Max;
                        }
                case 0xFFFFFFFF : //Unknown
                        return Stream_Max;
                default                     :
                        switch (stream_type)
                        {
                            case 0x80 : return Stream_Video;
                            case 0x81 : return Stream_Audio;
                            case 0x87 : return Stream_Audio;
                            case 0x88 : return Stream_Video;
                            case 0xD1 : return Stream_Video;
                            default   : return Stream_Max;
                        }
            }
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Psi_stream_type_Info(int8u stream_type, int32u format_identifier)
{
    switch (stream_type)
    {
        case 0x00 : return "ITU-T | ISO/IEC Reserved";
        case 0x01 : return "ISO/IEC 11172 Video";
        case 0x02 : return "ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream";
        case 0x03 : return "ISO/IEC 11172 Audio";
        case 0x04 : return "ISO/IEC 13818-3 Audio";
        case 0x05 : return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 private_sections";
        case 0x06 : return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 PES packets containing private data";
        case 0x07 : return "ISO/IEC 13522 MHEG";
        case 0x08 : return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A DSM-CC";
        case 0x09 : return "ITU-T Rec. H.222.1";
        case 0x0A : return "ISO/IEC 13818-6 type A";
        case 0x0B : return "ISO/IEC 13818-6 type B";
        case 0x0C : return "ISO/IEC 13818-6 type C";
        case 0x0D : return "ISO/IEC 13818-6 type D";
        case 0x0E : return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 auxiliary";
        case 0x0F : return "ISO/IEC 13818-7 Audio with ADTS transport syntax";
        case 0x10 : return "ISO/IEC 14496-2 Visual";
        case 0x11 : return "ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3 / AMD 1";
        case 0x12 : return "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets";
        case 0x13 : return "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC14496_sections.";
        case 0x14 : return "ISO/IEC 13818-6 Synchronized Download Protocol";
        case 0x15 : return "Metadata carried in PES packets";
        case 0x16 : return "Metadata carried in metadata_sections";
        case 0x17 : return "Metadata carried in ISO/IEC 13818-6 Data Carousel";
        case 0x18 : return "Metadata carried in ISO/IEC 13818-6 Object Carousel";
        case 0x19 : return "Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol";
        case 0x1A : return "IPMP stream (defined in ISO/IEC 13818-11, MPEG-2 IPMP)";
        case 0x1B : return "AVC video stream as defined in ITU-T Rec. H.264 | ISO/IEC 14496-10 Video";
        case 0x1C : return "ISO/IEC 14496-3 Audio, without using any additional transport syntax";
        case 0x1D : return "ISO/IEC 14496-17 Text";
        case 0x1E : return "Auxiliary video data stream as defined in ISO/IEC 23002-3";
        case 0x1F : return "SVC video sub-bitstream of an AVC video stream conforming to one or more profiles defined in Annex G of ITU-T Rec. H.264 | ISO/IEC 14496-10";
        case 0x20 : return "MVC video sub-bitstream of an AVC video stream conforming to one or more profiles defined in Annex H of ITU-T Rec. H.264 | ISO/IEC 14496-10";
        case 0x24 :
        case 0x27 : return "ITU-T Rec. H.265 | ISO/IEC 23008-2 MPEG-H Part 2 / HEVC video stream";
        case 0x7F : return "IPMP stream";
        default :
            if (stream_type<=0x7F) return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved";
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                case Elements::SCTE : //SCTE
                        switch (stream_type)
                        {
                            case 0x80 : return "SCTE - MPEG Video";
                            case 0x81 : return "ATSC - AC-3";
                            case 0x82 : return "SCTE - Standard Subtitle";
                            case 0x83 : return "SCTE - Isochronous Data";
                            case 0x84 : return "ATSC - Reserved";
                            case 0x85 : return "ATSC - Program Identifier";
                            case 0x86 : return "SCTE - Splice";
                            case 0x87 : return "ATSC - E-AC-3";
                            case 0x90 : return "DVB  - stream_type value for Time Slicing / MPE-FEC";
                            case 0x95 : return "ATSC - Data Service Table, Network Resources Table";
                            default   : return "ATSC/SCTE - Unknown";
                        }
                case Elements::HDMV : //Bluray
                        switch (stream_type)
                        {
                            case 0x80 : return "BluRay - PCM";
                            case 0x81 : return "BluRay - AC-3";
                            case 0x82 : return "BluRay - DTS";
                            case 0x83 : return "BluRay - AC-3 (TrueHD)";
                            case 0x84 : return "BluRay - E-AC-3";
                            case 0x85 : return "BluRay - DTS (HD-HRA)";
                            case 0x86 : return "BluRay - DTS (HD-MA)";
                            case 0x90 : return "BluRay - PGS";
                            case 0xA1 : return "BluRay - AC-3";
                            case 0xA2 : return "BluRay - DTS";
                            case 0xEA : return "BluRay - VC-1";
                            default   : return "Bluray - Unknown";
                        }
                case Elements::TSHV : //DV
                        switch (stream_type)
                        {
                            case 0xA0 : return "DV - Data 0";
                            case 0xA1 : return "DV - Data 1";
                            default   : return "Bluray - Unknown";
                        }
                case 0xFFFFFFFF : //Unknown
                        return "";
                default                     :
                        switch (stream_type)
                        {
                            case 0x80 : return "DigiCipher II video";
                            case 0x81 : return "AC-3";
                            case 0x88 : return "VC-1";
                            case 0x87 : return "E-AC-3";
                            case 0xD1 : return "Dirac";
                            default   : return "User Private";
                        }
            }
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Psi_table_id(int8u table_id)
{
    switch (table_id)
    {
        case 0x00 : return "program_association_section";
        case 0x01 : return "conditional_access_section";
        case 0x02 : return "TS_program_map_section";
        case 0x03 : return "TS_description_section";
        case 0x04 : return "ISO_IEC_14496_scene_description_section";
        case 0x05 : return "ISO_IEC_14496_object_descriptor_section";
        case 0x06 : return "Metadata?";
        case 0x38 : return "ISO/IEC 13818-6 reserved";
        case 0x39 : return "DSM-CC addressable section";
        case 0x3A : return "DSM-CC : return MPE";
        case 0x3B : return "DSM-CC : return U-N messages : return except DDM";
        case 0x3C : return "DSM-CC : return DDM";
        case 0x3D : return "DSM-CC : return stream descriptors";
        case 0x3E : return "DSM-CC : return private data : return IP-Datagram";
        case 0x3F : return "DSM-CC addressable section";
        case 0x40 : return "DVB - network_information_section - actual_network";
        case 0x41 : return "DVB - network_information_section - other_network";
        case 0x42 : return "DVB - service_description_section - actual_transport_stream";
        case 0x46 : return "DVB - service_description_section - other_transport_stream";
        case 0x4A : return "DVB - bouquet_association_section";
        case 0x4E : return "DVB - event_information_section - actual_transport_stream : return present/following";
        case 0x4F : return "DVB - event_information_section - other_transport_stream : return present/following";
        case 0x50 :
        case 0x51 :
        case 0x52 :
        case 0x53 :
        case 0x54 :
        case 0x55 :
        case 0x56 :
        case 0x57 :
        case 0x58 :
        case 0x59 :
        case 0x5A :
        case 0x5B :
        case 0x5C :
        case 0x5E :
        case 0x5F : return "DVB - event_information_section - actual_transport_stream : return schedule";
        case 0x60 :
        case 0x61 :
        case 0x62 :
        case 0x63 :
        case 0x64 :
        case 0x65 :
        case 0x66 :
        case 0x67 :
        case 0x68 :
        case 0x69 :
        case 0x6A :
        case 0x6B :
        case 0x6C :
        case 0x6D :
        case 0x6E :
        case 0x6F : return "DVB - event_information_section - other_transport_stream : return schedule";
        case 0x70 : return "DVB - time_date_section";
        case 0x71 : return "DVB - running_status_section";
        case 0x72 : return "DVB - stuffing_section";
        case 0x73 : return "DVB - time_offset_section";
        case 0x74 : return "DVB - application information section";
        case 0x75 : return "DVB - container section";
        case 0x76 : return "DVB - related content section";
        case 0x77 : return "DVB - content identifier section";
        case 0x78 : return "DVB - MPE-FEC section";
        case 0x79 : return "DVB - resolution notification section";
        case 0x7E : return "DVB - discontinuity_information_section";
        case 0x7F : return "DVB - selection_information_section";
        case 0xC0 : return "ATSC - Program Information Message";
        case 0xC1 : return "ATSC - Program Name Message";
        case 0xC2 : return "ATSC/SCTE - Network Information Message";
        case 0xC3 : return "ATSC/SCTE - Network Text Table (NTT)";
        case 0xC4 : return "ATSC/SCTE - Short Form Virtual Channel Table (S-VCT)";
        case 0xC5 : return "ATSC/SCTE - System Time Table (STT)";
        case 0xC6 : return "ATSC/SCTE - Subtitle Message";
        case 0xC7 : return "ATSC - Master Guide Table (MGT)";
        case 0xC8 : return "ATSC - Terrestrial Virtual Channel Table (TVCT)";
        case 0xC9 : return "ATSC - Cable Virtual Channel Table (CVCT) / Long-form Virtual Channel Table (L-VCT)";
        case 0xCA : return "ATSC - Rating Region Table (RRT)";
        case 0xCB : return "ATSC - Event Information Table (EIT)";
        case 0xCC : return "ATSC - Extended Text Table (ETT)";
        case 0xCD : return "ATSC - System Time Table (STT)";
        case 0xCE : return "ATSC - Data Event Table (DET)";
        case 0xCF : return "ATSC - Data Service Table (DST)";
        case 0xD0 : return "ATSC - Program Identifier Table (PIT)";
        case 0xD1 : return "ATSC - Network Resource Table (NRT)";
        case 0xD2 : return "ATSC - Long-term Service Table (L-TST)";
        case 0xD3 : return "ATSC - Directed Channel Change Table (DCCT)";
        case 0xD4 : return "ATSC - DCC Selection Code Table (DCCSCT)";
        case 0xD5 : return "ATSC - Selection Information Table (SIT)";
        case 0xD6 : return "ATSC - Aggregate Event Information Table (AEIT)";
        case 0xD7 : return "ATSC - Aggregate Extended Text Table (AETT)";
        case 0xD8 : return "ATSC - Cable Emergency Alert";
        case 0xD9 : return "ATSC - Aggregate Data Event Table";
        case 0xDA : return "ATSC - Satellite VCT (SVCT)";
        case 0xFC : return "SCTE - Splice";
        default :
            if (table_id>=0x06
             && table_id<=0x37) return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved";
            if (table_id>=0x40
             && table_id<=0x7F) return "DVB - reserved";
            if (table_id>=0x80
             && table_id<=0x8F) return "CA message";
            if (table_id>=0xC0
             && table_id<=0xDF) return "ATSC/SCTE - reserved";
            if (table_id<=0xFE) return "User Private";
            return "unknown";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Psi_atsc_service_type(int8u service_type)
{
    switch (service_type)
    {
        case 0x00 : return "reserved";
        case 0x01 : return "Analog television";
        case 0x02 : return "Digital television";
        case 0x03 : return "Digital radio";
        case 0x04 : return "Data";
        default   : return "reserved for future use";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Psi_table_id_extension(int8u table_id)
{
    switch (table_id)
    {
        case 0x00 : return "transport_stream_id";
        case 0x01 : return "reserved";
        case 0x02 : return "program_number";
        case 0x03 : return "reserved";
        case 0x40 : return "network_id";
        case 0x42 :
        case 0x46 : return "transport_stream_id";
        case 0x4E :
        case 0x4F :
        case 0x50 :
        case 0x51 :
        case 0x52 :
        case 0x53 :
        case 0x54 :
        case 0x55 :
        case 0x56 :
        case 0x57 :
        case 0x58 :
        case 0x59 :
        case 0x5A :
        case 0x5B :
        case 0x5C :
        case 0x5E :
        case 0x5F :
        case 0x60 :
        case 0x61 :
        case 0x62 :
        case 0x63 :
        case 0x64 :
        case 0x65 :
        case 0x66 :
        case 0x67 :
        case 0x68 :
        case 0x69 :
        case 0x6A :
        case 0x6B :
        case 0x6C :
        case 0x6D :
        case 0x6E :
        case 0x6F : return "service_id";
        case 0x7F : return "DVB_reserved_for_future_use";
        case 0xC8 :
        case 0xC9 : return "transport_stream_id";
        case 0xCA : return "reserved + rating_region";
        case 0xCB : return "source_id";
        case 0xCC : return "ETT_table_id_extension";
        case 0xD9 : return "AEIT_subtype + MGT_tag";
        case 0xDA : return "SVCT_subtype + SVCT_id";
        default : return "table_id_extension";
    }
}

//---------------------------------------------------------------------------
// CRC_32_Table
// A CRC is computed like this:
// Init: int32u CRC_32 = 0xFFFFFFFF;
// for each data byte do
//     CRC_32=(CRC_32<<8) ^ CRC_32_Table[(CRC_32>>24)^(data_byte)];
int32u Psi_CRC_32_Table[256] =
{
  0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
  0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
  0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
  0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
  0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
  0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
  0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
  0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
  0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
  0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
  0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
  0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
  0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
  0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
  0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
  0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
  0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
  0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
  0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
  0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
  0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
  0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
  0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
  0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
  0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
  0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
  0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
  0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
  0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
  0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
  0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
  0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
  0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
  0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
  0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
  0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
  0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
  0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
  0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
  0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
  0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
  0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

//---------------------------------------------------------------------------
const char* Mpeg_Psi_running_status[]=
{
    "",
    "Not running",
    "Starts in a few seconds",
    "Pausing",
    "Running",
    "Reserved",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
extern const char* Mpeg_Descriptors_original_network_id(int16u original_network_id);

//---------------------------------------------------------------------------
const char* Mpeg_Psi_splice_command_type(int8u splice_command_type)
{
    switch (splice_command_type)
    {
        case 0x00 : return "splice_null";
        case 0x04 : return "splice_schedule";
        case 0x05 : return "splice_insert";
        case 0x06 : return "time_signal";
        case 0x07 : return "bandwidth_reservation";
        default   : return "Reserved";
    }
};



//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpeg_Psi::File_Mpeg_Psi()
:File__Analyze()
{
    //In
    From_TS=true; //Default is from TS
    Complete_Stream=NULL;
    pid=(int16u)-1;

    //Temp
    transport_stream_id=0x0000; //Impossible
    CRC_32=0;
    elementary_PID=0x0000;
    program_number=0x0000;
    stream_type=0x00;
    event_id=0x0000;
    elementary_PID_IsValid=false;
    program_number_IsValid=false;
    stream_type_IsValid=false;
    event_id_IsValid=false;
    current_next_indicator=false;
    IsATSC=false;
    ForceStreamDisplay=MediaInfoLib::Config.MpegTs_ForceStreamDisplay_Get();
}

//---------------------------------------------------------------------------
File_Mpeg_Psi::~File_Mpeg_Psi()
{
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Psi::FileHeader_Parse()
{
    //Parsing
    if (From_TS)
    {
        int8u pointer_field;
        Get_B1 (pointer_field,                                  "pointer_field");
        if (pointer_field)
            Skip_XX(pointer_field,                              "payload");
    }
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mpeg_Psi::Header_Begin()
{
    if (Buffer_Offset) //Not the first one
    {
        Peek_B1(table_id);
        if (table_id==0xFF)
        {
            Accept();
            Fill();
            Finish();
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Header_Parse()
{
    //From Program stream
    if (!From_TS)
    {
        //Filling
        table_id=0xFF; //Make it invalid
        section_syntax_indicator=false;
        Header_Fill_Code((int64u)-1, "program_stream_map"); //(int64u)-1 for precising "out of scope"
        Header_Fill_Size(Element_Size-4);
        return;
    }

    //Parsing
    int16u section_length;
    Get_B1 (table_id,                                           "table_id");
    BS_Begin();
    Get_SB (    section_syntax_indicator,                       "section_syntax_indicator");
    Skip_SB(                                                    "private_indicator");
    Skip_S1( 2,                                                 "reserved");
    Get_S2 (12, section_length,                                 "section_length");
    BS_End();

    //Size
    if ((size_t)section_length<Element_Offset+(section_syntax_indicator?4:0)) //We must have 4 more byte for CRC
    {
        Reject("PSI"); //Error, we exit
        return;
    }
    if (Element_Size<Element_Offset+section_length)
    {
        Element_WaitForMoreData();
        return;
    }
    //Element[Element_Level-1].IsComplete=true;

    //CRC32
    if (table_id<=0x06 && !section_syntax_indicator)
    {
        Trusted_IsNot("CRC error");
        CRC_32=0xffffffff;
        Reject();
        return;
    }
    if (section_syntax_indicator || table_id==0xC1)
    {
        CRC_32=0xffffffff;
        const int8u* CRC_32_Buffer=Buffer+Buffer_Offset+(size_t)Element_Offset-3; //table_id position

        while(CRC_32_Buffer<Buffer+Buffer_Offset+(size_t)Element_Offset+section_length) //from table_id to the end, CRC_32 included
        {
            CRC_32=(CRC_32<<8) ^ Psi_CRC_32_Table[(CRC_32>>24)^(*CRC_32_Buffer)];
            CRC_32_Buffer++;
        }

        if (CRC_32)
        {
            Trusted_IsNot("CRC error");
            Reject();
            return;
        }
    }

    //Filling
    Header_Fill_Code(table_id, Ztring().From_Number(table_id, 16));
    Header_Fill_Size(3+section_length);
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Data_Parse()
{
    //Check if OK
    if (table_id<=0x06 && !section_syntax_indicator)
    {
        Skip_XX(Element_Size,                                   "Data (section_syntax_indicator failed)");
        Finish("PSI");
        return;
    }

    if (section_syntax_indicator)
    {
        Element_Size-=4; //Reserving size of CRC32
        Get_B2(     table_id_extension,                         Mpeg_Psi_table_id_extension(table_id)); Element_Name(Ztring(Mpeg_Psi_table_id_extension(table_id))+__T("=")+Ztring::ToZtring_From_CC2(table_id_extension));
        BS_Begin();
        Skip_S1( 2,                                             "reserved");
        Get_S1 ( 5, version_number,                             "version_number"); Element_Info1(__T("Version=")+Ztring::ToZtring(version_number));
        Get_SB (    current_next_indicator,                     "current_next_indicator");
        BS_End();
        Info_B1(    section_number,                             "section_number"); Element_Info1(__T("Section=")+Ztring::ToZtring(section_number));
        Skip_B1(                                                "last_section_number");
    }
    else if (table_id==0xC1)
    {
        Element_Size-=4; //Reserving size of CRC32
    }

    #define ELEMENT_CASE(_NAME, _DETAIL) \
        case 0x##_NAME : Element_Name(_DETAIL); Table_##_NAME(); break;

    switch (table_id)
    {
        ELEMENT_CASE(00, "program_association_section");
        ELEMENT_CASE(01, "conditional_access_section");
        ELEMENT_CASE(02, "TS_program_map_section");
        ELEMENT_CASE(03, "TS_description_section");
        ELEMENT_CASE(04, "ISO_IEC_14496_scene_description_section");
        ELEMENT_CASE(05, "ISO_IEC_14496_object_descriptor_section");
        ELEMENT_CASE(06, "Metadata?");
        ELEMENT_CASE(38, "ISO/IEC 13818-6 reserved");
        ELEMENT_CASE(39, "DSM-CC addressable section");
        ELEMENT_CASE(3A, "DSM-CC, MPE");
        ELEMENT_CASE(3B, "DSM-CC, U-N messages, except DDM");
        ELEMENT_CASE(3C, "DSM-CC, DDM");
        ELEMENT_CASE(3D, "DSM-CC, stream descriptors");
        ELEMENT_CASE(3E, "DSM-CC, private data, IP-Datagram");
        ELEMENT_CASE(3F, "DSM-CC addressable section");
        ELEMENT_CASE(40, "DVB - network_information_section - actual_network");
        ELEMENT_CASE(41, "DVB - network_information_section - other_network");
        ELEMENT_CASE(42, "DVB - service_description_section - actual_transport_stream");
        ELEMENT_CASE(46, "DVB - service_description_section - other_transport_stream");
        ELEMENT_CASE(4A, "DVB - bouquet_association_section");
        ELEMENT_CASE(4E, "DVB - event_information_section - actual_transport_stream, present/following");
        ELEMENT_CASE(4F, "DVB - event_information_section - other_transport_stream, present/following");
        case 0x50 :
        case 0x51 :
        case 0x52 :
        case 0x53 :
        case 0x54 :
        case 0x55 :
        case 0x56 :
        case 0x57 :
        case 0x58 :
        case 0x59 :
        case 0x5A :
        case 0x5B :
        case 0x5C :
        case 0x5E :
        ELEMENT_CASE(5F, "DVB - event_information_section - actual_transport_stream, schedule");
        case 0x60 :
        case 0x61 :
        case 0x62 :
        case 0x63 :
        case 0x64 :
        case 0x65 :
        case 0x66 :
        case 0x67 :
        case 0x68 :
        case 0x69 :
        case 0x6A :
        case 0x6B :
        case 0x6C :
        case 0x6D :
        case 0x6E :
        ELEMENT_CASE(6F, "DVB - event_information_section - other_transport_stream, schedule");
        ELEMENT_CASE(70, "DVB - time_date_section");
        ELEMENT_CASE(71, "DVB - running_status_section");
        ELEMENT_CASE(72, "DVB - stuffing_section");
        ELEMENT_CASE(73, "DVB - time_offset_section");
        ELEMENT_CASE(74, "DVB - application information section");
        ELEMENT_CASE(75, "DVB - container section");
        ELEMENT_CASE(76, "DVB - related content section");
        ELEMENT_CASE(77, "DVB - content identifier section");
        ELEMENT_CASE(78, "DVB - MPE-FEC section");
        ELEMENT_CASE(79, "DVB - resolution notification section");
        ELEMENT_CASE(7E, "DVB - discontinuity_information_section");
        ELEMENT_CASE(7F, "DVB - selection_information_section");
        ELEMENT_CASE(C0, "ATSC - Program Information Message");
        ELEMENT_CASE(C1, "ATSC - Program Name Message");
        ELEMENT_CASE(C2, "ATSC/SCTE - Network Information Message");
        ELEMENT_CASE(C3, "ATSC/SCTE - Network Text Table (NTT)");
        ELEMENT_CASE(C4, "ATSC/SCTE - Short Form Virtual Channel Table (S-VCT)");
        ELEMENT_CASE(C5, "ATSC/SCTE - System Time Table (STT)");
        ELEMENT_CASE(C6, "ATSC/SCTE - Subtitle Message");
        ELEMENT_CASE(C7, "ATSC - Master Guide Table (MGT)");
        ELEMENT_CASE(C8, "ATSC - Terrestrial Virtual Channel Table (TVCT)");
        ELEMENT_CASE(C9, "ATSC - Cable Virtual Channel Table (CVCT) / Long-form Virtual Channel Table (L-VCT)");
        ELEMENT_CASE(CA, "ATSC - Rating Region Table (RRT)");
        ELEMENT_CASE(CB, "ATSC - Event Information Table (EIT)");
        ELEMENT_CASE(CC, "ATSC - Extended Text Table (ETT)");
        ELEMENT_CASE(CD, "ATSC - System Time Table (STT)");
        ELEMENT_CASE(CE, "ATSC - Data Event Table (DET)");
        ELEMENT_CASE(CF, "ATSC - Data Service Table (DST)");
        ELEMENT_CASE(D0, "ATSC - Program Identifier Table (PIT)");
        ELEMENT_CASE(D1, "ATSC - Network Resource Table (NRT)");
        ELEMENT_CASE(D2, "ATSC - Long-term Service Table (L-TST)");
        ELEMENT_CASE(D3, "ATSC - Directed Channel Change Table (DCCT)");
        ELEMENT_CASE(D4, "ATSC - DCC Selection Code Table (DCCSCT)");
        ELEMENT_CASE(D5, "ATSC - Selection Information Table (SIT)");
        ELEMENT_CASE(D6, "ATSC - Aggregate Event Information Table (AEIT)");
        ELEMENT_CASE(D7, "ATSC - Aggregate Extended Text Table (AETT)");
        ELEMENT_CASE(D8, "ATSC - Cable Emergency Alert");
        ELEMENT_CASE(D9, "ATSC - Aggregate Data Event Table");
        ELEMENT_CASE(DA, "ATSC - Satellite VCT");
        ELEMENT_CASE(FC, "SCTE - Splice");
        default :
            if (table_id>=0x06
             && table_id<=0x37) {Element_Name("ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved"); Skip_XX(Element_Size, "Unknown"); break;}
            if (table_id>=0x40
             && table_id<=0x7F) {Element_Name("DVB - reserved"); Skip_XX(Element_Size, "Unknown"); break;}
            if (table_id>=0x80
             && table_id<=0x8F) {Element_Name("CA message, EMM, ECM"); Skip_XX(Element_Size, "Unknown"); break;}
            if (table_id>=0xC0
             && table_id<=0xDF) {Element_Name("ATSC/SCTE - reserved");Skip_XX(Element_Size, "Unknown");  break;}
            if (table_id<=0xFE) {Element_Name("User Private"); Skip_XX(Element_Size, "Unknown"); break;}
            if (Element_Code==(int64u)-1) {program_stream_map(); break;} //Specific to MPEG-PS
                                {Element_Name("forbidden"); Skip_XX(Element_Size, "Unknown"); break;}
    }

    if (section_syntax_indicator || table_id==0xC1)
    {
        Element_Size+=4;
        Skip_B4(                                                "CRC32");
    }

    if (table_id>=0x40
     && Config->ParseSpeed>=0.5
     && Complete_Stream->Streams_NotParsedCount!=(size_t)-1 && Complete_Stream->Streams_NotParsedCount!=0)
        Complete_Stream->Streams_NotParsedCount=(size_t)-1; //Disabling speed up for detection in case of DVB/ATSC tables, we want all of them.

    if (Buffer_Offset+Element_Size==Buffer_Size)
    {
        Accept();
        Fill();
        Finish();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_reserved()
{

}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_iso13818_6()
{
    Element_Info1("Defined in ISO/IEC 13818-6");
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_user_private()
{
    Element_Info1("user_private");
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_forbidden()
{
    Element_Info1("forbidden");
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::program_stream_map()
{
    Element_Name("program_stream_map");
    table_id=0x02; // program_map_section

    //Parsing
    int16u elementary_stream_map_length;
    bool single_extension_stream_flag;
    BS_Begin();
    Skip_SB(                                                    "current_next_indicator");
    Get_SB (single_extension_stream_flag,                       "single_extension_stream_flag");
    Skip_SB(                                                    "reserved");
    Skip_S1( 5,                                                 "program_stream_map_version");
    Skip_S1( 7,                                                 "reserved");
    Mark_1 ();
    BS_End();
    Get_B2 (Descriptors_Size,                                   "program_stream_info_length");
    if (Descriptors_Size>0)
        Descriptors();

    Get_B2 (elementary_stream_map_length,                       "elementary_stream_map_length");
    int16u elementary_stream_map_Pos=0;
    while (Element_Offset<Element_Size && elementary_stream_map_Pos<elementary_stream_map_length)
    {
        Element_Begin0();
        int16u ES_info_length;
        int8u stream_type, elementary_stream_id;
        Get_B1 (stream_type,                                    "stream_type"); Param_Info1(Mpeg_Psi_stream_type_Info(stream_type, 0x00000000));
        Get_B1 (elementary_stream_id,                           "elementary_stream_id");
        Get_B2 (ES_info_length,                                 "ES_info_length");
        Descriptors_Size=ES_info_length;
        Element_Name(Ztring::ToZtring(elementary_stream_id, 16));
        if (elementary_stream_id==0xFD && !single_extension_stream_flag)
        {
            Skip_S1(8,                                          "pseudo_descriptor_tag");
            Skip_S1(8,                                          "pseudo_descriptor_length");
            Mark_1();
            Skip_S1(7,                                          "elementary_stream_id_extension");
            if (Descriptors_Size>=3)
                Descriptors_Size-=3;
        }
        if (Descriptors_Size>0)
        {
            elementary_PID=elementary_stream_id;
            elementary_PID_IsValid=true;
            Descriptors();
        }
        Element_End0();
        elementary_stream_map_Pos+=4+ES_info_length;

        FILLING_BEGIN();
            Complete_Stream->Streams[elementary_stream_id]->stream_type=stream_type;
            Complete_Stream->Streams[elementary_stream_id]->Infos["CodecID"].From_Number(stream_type);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_00()
{
    //transport_stream_id
    if (!Complete_Stream->transport_stream_id_IsValid || table_id_extension!=Complete_Stream->transport_stream_id)
    {
        if (Complete_Stream->Transport_Streams.find(Complete_Stream->transport_stream_id)!=Complete_Stream->Transport_Streams.end())
            while (!Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.empty())
            {
                program_number=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.begin()->first;
                program_number_Remove();
            }

        Complete_Stream->transport_stream_id=table_id_extension;
        Complete_Stream->transport_stream_id_IsValid=true;
    }

    if (Complete_Stream->Transport_Streams[table_id_extension].Programs_NotParsedCount==(size_t)-1)
        Complete_Stream->Transport_Streams[table_id_extension].Programs_NotParsedCount=0;
    Complete_Stream->Transport_Streams[table_id_extension].programs_List.clear();

    //Saving previous status
    std::map<int16u, complete_stream::transport_stream::program> program_numbers_Previous=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs;

    //Reseting
    std::vector<int16u> Table_ID_Extension_List;
    for (complete_stream::stream::table_id::table_id_extensions::iterator Table_ID_Extension=Complete_Stream->Streams[0x0000]->Table_IDs[0x00]->Table_ID_Extensions.begin(); Table_ID_Extension!=Complete_Stream->Streams[0x0000]->Table_IDs[0x00]->Table_ID_Extensions.end(); ++Table_ID_Extension)
        if (Table_ID_Extension->first!=table_id_extension)
            Table_ID_Extension_List.push_back(Table_ID_Extension->first);
    for (size_t Pos=0; Pos<Table_ID_Extension_List.size(); Pos++)
        Complete_Stream->Streams[0x0000]->Table_IDs[0x00]->Table_ID_Extensions.erase(Table_ID_Extension_List[Pos]);

    //Parsing
    while (Element_Offset<Element_Size)
    {
        Element_Begin1("program");
        Get_B2 (    program_number,                             "program_number");
        BS_Begin();
        Skip_S1( 3,                                             "reserved");
        Get_S2 ( 13, elementary_PID,                            program_number?"program_map_PID":"network_PID"); Element_Info1(Ztring::ToZtring_From_CC2(elementary_PID));
        BS_End();
        Element_End0();

        FILLING_BEGIN();
            if (elementary_PID
            #if MEDIAINFO_FILTER
                && Config->File_Filter_Get(program_number)
            #endif //MEDIAINFO_FILTER
                )
            {
                program_number_Update();

                std::map<int16u, complete_stream::transport_stream::program>::iterator program_number_Previous=program_numbers_Previous.find(program_number);
                if (program_number_Previous!=program_numbers_Previous.end())
                    program_numbers_Previous.erase(program_number_Previous);
            }
        FILLING_END();
    }
    BS_End();

    FILLING_BEGIN();
        //Removing previous elementary_PIDs no more used
        for (std::map<int16u, complete_stream::transport_stream::program>::iterator program_number_Previous=program_numbers_Previous.begin(); program_number_Previous!=program_numbers_Previous.end(); ++program_number_Previous)
        {
            program_number=program_number_Previous->first;
            program_number_Remove();
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_01()
{
    //Parsing
    if (Element_Offset<Element_Size)
    {
        Descriptors_Size=(int16u)(Element_Size-Element_Offset);
        if (Descriptors_Size>0)
            Descriptors();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_02()
{
    //Informing PSI is parsed
    if (!Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].IsParsed && Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs_NotParsedCount)
    {
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs_NotParsedCount--;
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].IsParsed=true;
    }

    //Saving previous status
    std::vector<int16u> elementary_PIDs_Previous=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].elementary_PIDs;

    //Parsing
    int16u PCR_PID;
    BS_Begin();
    Skip_S1( 3,                                                 "reserved");
    Get_S2 (13, PCR_PID,                                        "PCR_PID");
    Skip_S1( 4,                                                 "reserved");
    Get_S2 (12, Descriptors_Size,                               "program_info_length");
    BS_End();

    //Descriptors
    transport_stream_id=Complete_Stream->transport_stream_id;
    program_number=table_id_extension;
    program_number_IsValid=true;
    if (Descriptors_Size>0)
        Descriptors();

    //Parsing
    while (Element_Offset<Element_Size)
    {
        Element_Begin0();
        BS_Begin();
        Get_S1 ( 8, stream_type,                                "stream_type"); Element_Info1(Mpeg_Psi_stream_type_Info(stream_type, Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].registration_format_identifier)); Param_Info1(Mpeg_Psi_stream_type_Info(stream_type, Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].registration_format_identifier));
        Skip_S1( 3,                                             "reserved");
        Get_S2 (13, elementary_PID,                             "elementary_PID");
        Skip_S1( 4,                                             "reserved");
        Get_S2 (12, Descriptors_Size,                           "ES_info_length");
        BS_End();

        FILLING_BEGIN();
            //Searching for hidden Stereoscopic stream
            if (stream_type==0x20 && File_Name_WithoutDemux.size()>=4+1+6+1+4+1+10 && Config->File_Bdmv_ParseTargetedFile_Get())
            {
                //Searching the playlist with the pid
                Ztring Name=File_Name_WithoutDemux;
                Name.resize(Name.size()-(4+1+6+1+4+1+10)); //Removing BDMV/STREAM/SSIF/xxxxx.ssif
                ZtringList List=Dir::GetAllFileNames(Name+__T("BDMV")+PathSeparator+__T("PLAYLIST")+PathSeparator+__T("*.mpls"), Dir::Include_Files);
                std::vector<MediaInfo_Internal*> MIs;
                MIs.resize(List.size());
                size_t FileWithRightPID_Pos=(size_t)-1;
                for (size_t Pos=0; Pos<MIs.size(); Pos++)
                {
                    MIs[Pos]=new MediaInfo_Internal();
                    MIs[Pos]->Option(__T("File_Bdmv_ParseTargetedFile"), __T("0"));
                    MIs[Pos]->Open(List[Pos]);
                    if (MIs[Pos]->Count_Get(Stream_Video)==1)
                    {
                        int16u pid=Ztring(MIs[Pos]->Get(Stream_Video, 0, Video_ID)).To_int16u();
                        if (pid==elementary_PID)
                        {
                            FileWithRightPID_Pos=Pos;
                            break;
                        }
                    }
                }

                if (FileWithRightPID_Pos!=(size_t)-1)
                {
                    ZtringList ID_List;
                    ID_List.Separator_Set(0, __T(" / "));
                    ID_List.Write(MIs[FileWithRightPID_Pos]->Get(Stream_Video, 0, Video_ID));
                    if (ID_List.size()==2)
                    {
                        Complete_Stream->Streams[ID_List[1].To_int16u()]->SubStream_pid=elementary_PID;
                        Complete_Stream->Streams[elementary_PID]->SubStream_pid=ID_List[1].To_int16u();

                        elementary_PID=ID_List[1].To_int16u();
                        stream_type=0x1B;

                        bool IsAlreadyPresent=false;
                        for (size_t Pos=0; Pos<Complete_Stream->Streams[elementary_PID]->program_numbers.size(); Pos++)
                            if (Complete_Stream->Streams[elementary_PID]->program_numbers[Pos]==program_number)
                                IsAlreadyPresent=true;
                        if (!IsAlreadyPresent)
                        {
                            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs.push_back(elementary_PID);
                            Complete_Stream->Streams[elementary_PID]->program_numbers.push_back(program_number);
                            Complete_Stream->Streams[elementary_PID]->registration_format_identifier=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].registration_format_identifier;
                        }
                        if (Complete_Stream->Streams[elementary_PID]->Kind!=complete_stream::stream::pes)
                        {
                            if (Complete_Stream->Streams_NotParsedCount==(size_t)-1)
                                Complete_Stream->Streams_NotParsedCount=0;
                            Complete_Stream->Streams_NotParsedCount++;
                            Complete_Stream->Streams[elementary_PID]->Kind=complete_stream::stream::pes;
                            Complete_Stream->Streams[elementary_PID]->stream_type=stream_type;
                            Complete_Stream->Streams[elementary_PID]->Searching_Payload_Start_Set(true);
                            #ifdef MEDIAINFO_MPEGTS_PCR_YES
                                Complete_Stream->Streams[elementary_PID]->Searching_TimeStamp_Start_Set(true);
                                Complete_Stream->Streams[elementary_PID]->PCR_PID=PCR_PID;
                            #endif //MEDIAINFO_MPEGTS_PCR_YES
                            #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                                //Complete_Stream->Streams[elementary_PID]->Searching_ParserTimeStamp_Start_Set(true);
                            #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
                            #if MEDIAINFO_TRACE
                                Complete_Stream->Streams[elementary_PID]->Element_Info1="PES";
                            #endif //MEDIAINFO_TRACE
                            if (Complete_Stream->File__Duplicate_Get_From_PID(elementary_PID))
                                Complete_Stream->Streams[elementary_PID]->ShouldDuplicate=true;
                        }
                    }

                }

                for (size_t Pos=0; Pos<MIs.size(); Pos++)
                    delete MIs[Pos]; //MIs[Pos]=NULL;
            }

            if (elementary_PID)
            {
                elementary_PID_Update(PCR_PID);

                //Removing from the list of previous elementary_PIDs to remove
                for (size_t Pos=0; Pos<elementary_PIDs_Previous.size(); Pos++)
                    if (elementary_PIDs_Previous[Pos]==elementary_PID)
                        elementary_PIDs_Previous.erase(elementary_PIDs_Previous.begin()+Pos);
            }
        FILLING_END();

        //Descriptors
        elementary_PID_IsValid=true;
        if (Descriptors_Size>0)
            Descriptors();

        Element_End1(Ztring::ToZtring_From_CC2(elementary_PID));
    }

    FILLING_BEGIN();
        //Removing previous elementary_PIDs no more used
        if (Config->File_MpegTs_RealTime_Get())
        {
            for (size_t Pos=0; Pos<elementary_PIDs_Previous.size(); Pos++)
            {
                elementary_PID=elementary_PIDs_Previous[Pos];
                elementary_PID_Remove();

                Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].Update_Needed_StreamCount=true;
            }
        }

        #ifdef MEDIAINFO_MPEGTS_PCR_YES
            if (PCR_PID!=0x1FFF) //Not padding packet
            {
                Complete_Stream->Streams[PCR_PID]->IsPCR=true;
                Complete_Stream->PCR_PIDs[PCR_PID]++;
                Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].PCR_PID=PCR_PID;
                if (Complete_Stream->Streams[PCR_PID]->TimeStamp_Start==(int64u)-1)
                    Complete_Stream->Streams[PCR_PID]->Searching_TimeStamp_Start_Set(true);
                #if MEDIAINFO_TRACE
                    if (Complete_Stream->Streams[PCR_PID]->Kind==complete_stream::stream::unknown)
                        Complete_Stream->Streams[PCR_PID]->Element_Info1="PCR";
                #endif //MEDIAINFO_TRACE
            }
        #endif //MEDIAINFO_MPEGTS_PCR_YES

        //Handling ATSC/CEA/DVB
        if (!Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs_NotParsedCount)
        {
            //We know what is each pid, so we can try known values
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

        if (Buffer_Offset>=4)
        {
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].ExtraInfos_Content["pointer_field"].From_Number(Buffer_Offset-4);
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].ExtraInfos_Option["pointer_field"]=__T("N NT");
        }
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].ExtraInfos_Content["section_length"].From_Number(Element_Size+4);
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].ExtraInfos_Option["section_length"]=__T("N NT");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_03()
{
    //Parsing
    if (Element_Offset<Element_Size)
    {
        Descriptors_Size=(int16u)(Element_Size-Element_Offset);
        if (Descriptors_Size>0)
            Descriptors();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_40()
{
    if (IsATSC)
    {
        Skip_XX(Element_Size,                               "Unknown ATSC");
        return;
    }

    //Parsing
    BS_Begin();
    Skip_S1( 4,                                             "reserved");
    Get_S2 (12, Descriptors_Size,                           "network_descriptors_length");
    BS_End();

    //Descriptors
    if (Descriptors_Size>0)
        Descriptors();

    //Parsing
    int16u transport_stream_loop_length;
    BS_Begin();
    Skip_S1( 4,                                             "reserved");
    Get_S2 (12, transport_stream_loop_length,               "transport_stream_loop_length");
    BS_End();

    if (Element_Offset<Element_Size)
    {
        int16u original_network_id;
        Element_Begin0();
        Get_B2 (transport_stream_id,                        "transport_stream_id"); Element_Info1(transport_stream_id);
        Get_B2 (original_network_id,                        "original_network_id"); Param_Info1(Mpeg_Descriptors_original_network_id(original_network_id));
        BS_Begin();
        Skip_S1( 4,                                         "reserved");
        Get_S2 (12, Descriptors_Size,                       "transport_descriptors_length");
        BS_End();

        //Descriptors
        if (Descriptors_Size>0)
            Descriptors();

        Element_End0();

        FILLING_BEGIN();
            Complete_Stream->original_network_name=Mpeg_Descriptors_original_network_id(original_network_id);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_42()
{
    //Parsing
    Skip_B2(                                                    "original_network_id");
    Skip_B1(                                                    "reserved_future_use");
    while (Element_Offset<Element_Size)
    {
        Element_Begin0();
        int64u Test;
        Peek_B5(Test);
        if (Test!=0xFFFFFFFFFFULL)
        {
            Get_B2 (    program_number,                         "service_id");
            BS_Begin();
            Skip_S1( 6,                                         "reserved_future_use");
            Skip_SB(                                            "EIT_schedule_flag");
            Skip_SB(                                            "EIT_present_following_flag");
            Info_S1( 3, running_status,                         "running_status"); Param_Info1(Mpeg_Psi_running_status[running_status]);
            Skip_SB(                                            "free_CA_mode");
            Get_S2 (12, Descriptors_Size,                       "ES_info_length");
            BS_End();

            //Descriptors
            program_number_IsValid=true;
            if (Descriptors_Size>0)
                Descriptors();

            Element_End1(Ztring::ToZtring_From_CC2(program_number));
        }
        else
        {
            Skip_XX(Element_Size-Element_Offset,                    "Junk");
            Element_End1("Junk");
        }
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_46()
{
    Table_42();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_4E()
{
    //Clearing
    Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events.clear();
    Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks_IsUpdated=true;
    Complete_Stream->Programs_IsUpdated=true;

    //Parsing
    Get_B2 (transport_stream_id,                                "transport_stream_id");
    if (table_id==0x4E || (table_id&0xF0)==0x50) //current transport_stream_id
        transport_stream_id=Complete_Stream->transport_stream_id; //On the example I have, transport_stream_id is something else, what?
    Skip_B2(                                                    "original_network_id");
    Skip_B1(                                                    "segment_last_section_number");
    Skip_B1(                                                    "last_table_id");
    if (Element_Offset==Element_Size)
    {
        Element_DoNotShow(); //This is empty!
        return;
    }
    while (Element_Offset<Element_Size)
    {
        Element_Begin0();
        int32u time, duration;
        int16u date;
        int8u running_status;
        Get_B2 (event_id,                                       "event_id");
        Get_B2 (date,                                           "start_time (date)"); Param_Info1(Date_MJD(date));
        Get_B3 (time,                                           "start_time (time)"); Param_Info1(Time_BCD(time));
        Get_B3 (duration,                                       "duration"); Param_Info1(Time_BCD(duration));
        BS_Begin();
        Get_S1 ( 3, running_status,                             "running_status"); Param_Info1(Mpeg_Psi_running_status[running_status]);
        Skip_SB(                                                "free_CA_mode");
        Get_S2 (12, Descriptors_Size,                           "descriptors_loop_length");
        BS_End();

        //Descriptors
        event_id_IsValid=true;
        if (Descriptors_Size>0)
            Descriptors();

        Element_End1(Ztring::ToZtring_From_CC2(event_id));

        FILLING_BEGIN();
            Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].start_time=__T("UTC ")+Date_MJD(date)+__T(" ")+Time_BCD(time);
            Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].duration=Time_BCD(duration);
            if (running_status)
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].running_status=Mpeg_Psi_running_status[running_status];
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_4F()
{
    Table_4E();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_5F()
{
    Table_4E();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_6F()
{
    Table_4F();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_70()
{
    //Parsing
    int32u time;
    int16u date;
    Get_B2 (date,                                               "UTC_time (date)"); Param_Info1(Date_MJD(date));
    Get_B3 (time,                                               "UTC_time (time)"); Param_Info1(Time_BCD(time));

    FILLING_BEGIN();
        if (Complete_Stream->Duration_Start.empty())
            Complete_Stream->Duration_Start=__T("UTC ")+Date_MJD(date)+__T(" ")+Time_BCD(time);
        Complete_Stream->Duration_End=__T("UTC ")+Date_MJD(date)+__T(" ")+Time_BCD(time);
        Complete_Stream->Duration_End_IsUpdated=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_73()
{
    //Parsing
    int32u time;
    int16u date;
    Get_B2 (date,                                               "UTC_time (date)"); Param_Info1(Date_MJD(date));
    Get_B3 (time,                                               "UTC_time (time)"); Param_Info1(Time_BCD(time));
    BS_Begin();
    Skip_S1( 4,                                                 "DVB_reserved_for_future_use");
    Get_S2 (12, Descriptors_Size,                               "transmission_info_loop_length");
    BS_End();

    //Descriptors
    if (Descriptors_Size>0)
        Descriptors();

    Skip_B4(                                                    "CRC32");

    FILLING_BEGIN();
        if (Complete_Stream->Duration_Start.empty())
            Complete_Stream->Duration_Start=__T("UTC ")+Date_MJD(date)+__T(" ")+Time_BCD(time);
        Complete_Stream->Duration_End=__T("UTC ")+Date_MJD(date)+__T(" ")+Time_BCD(time);
        Complete_Stream->Duration_End_IsUpdated=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_7F()
{
    //Parsing
    BS_Begin();
    Skip_S1( 4,                                                 "DVB_reserved_for_future_use");
    Get_S2 (12, Descriptors_Size,                               "transmission_info_loop_length");
    BS_End();

    //Descriptors
    if (Descriptors_Size>0)
        Descriptors();

    while (Element_Offset<Element_Size)
    {
        Element_Begin0();
        Get_B2 (    program_number,                             "service_id");
        BS_Begin();
        Skip_SB(                                                "DVB_reserved_future_use");
        Info_S1( 3, running_status,                             "running_status"); Param_Info1(Mpeg_Psi_running_status[running_status]);
        Get_S2 (12, Descriptors_Size,                           "service_loop_length");
        BS_End();

        //Descriptors
        program_number_IsValid=true;
        if (Descriptors_Size>0)
            Descriptors();

        Element_End1(Ztring::ToZtring_From_CC2(program_number));
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_C0()
{
        //TODO
        Skip_XX(Element_Size-Element_Offset,                    "data");
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_C1()
{
    IsATSC=true;

    //Parsing
    Ztring program_name, alternate_program_name;
    int8u protocol_version, program_name_length, alternate_program_name_length, package_count;
    BS_Begin();
    Skip_S1(3,                                                  "reserved");
    Get_S1 (5, protocol_version,                                "protocol_version");
    BS_End();
    if (protocol_version!=0)
    {
        Skip_XX(Element_Size-Element_Offset,                    "data");
        return;
    }
    Skip_C3(                                                    "ISO_639_language_code");
    Skip_B2(                                                    "program_number");
    Skip_B1(                                                    "reserved");
    Skip_B1(                                                    "sequence");
    Skip_B1(                                                    "program_epoch_number");
    BS_Begin();
    Skip_SB(                                                    "display_name_when_not_auth");
    Skip_SB(                                                    "use_alt_name_in_purchase_history");
    Skip_SB(                                                    "use_alt_name_if_not_auth");
    Skip_SB(                                                    "display_ratings");
    Skip_S1(4,                                                  "reserved");
    BS_End();
    Get_B1 (program_name_length,                                "program_name_length");
    SCTE_multilingual_text_string(program_name_length, program_name, "program_name");
    Get_B1 (alternate_program_name_length,                      "alternate_program_name_length");
    SCTE_multilingual_text_string(alternate_program_name_length, alternate_program_name, "alternate_program_name");
    BS_Begin();
    Skip_S1(3,                                                  "reserved");
    Get_S1 (5, package_count,                                   "package_count");
    BS_End();
    for (int8u Pos=0; Pos<package_count; Pos++)
    {
        Ztring package_name;
        int8u package_name_length;
        Get_B1 (package_name_length,                            "package_name_length");
        SCTE_multilingual_text_string(package_name_length, package_name, "package_name");
    }

    if (Element_Offset<Element_Size)
    {
        BS_Begin();
        Skip_S1( 6,                                             "reserved");
        Get_S2 (10, Descriptors_Size,                           "descriptors_length");
        BS_End();

        //Descriptors
        if (Descriptors_Size>0)
            Descriptors();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_C7()
{
    //Parsing
    int16u tables_defined;
    int8u protocol_version;
    Get_B1 (protocol_version,                                   "protocol_version");
    if (protocol_version!=0)
    {
        Skip_XX(Element_Size-Element_Offset,                    "data");
        return;
    }
    Get_B2 (tables_defined,                                     "tables_defined");
    for (int16u Pos=0; Pos<tables_defined; Pos++)
    {
        int16u table_type, table_type_PID;
        Element_Begin0();
        Get_B2 (    table_type,                                 "table_type"); Param_Info1(Mpeg_Psi_ATSC_table_type(table_type));
        BS_Begin();
        Skip_S1( 3,                                             "reserved");
        Get_S2 (13, table_type_PID,                             "table_type_PID");
        Skip_S1( 3,                                             "reserved");
        Skip_S1( 5,                                             "table_type_version_number");
        BS_End();
        Skip_B4(                                                "number_bytes");
        BS_Begin();
        Skip_S1( 4,                                             "reserved");
        Get_S2 (12, Descriptors_Size,                           "table_type_descriptors_length");
        BS_End();

        //Descriptors
        if (Descriptors_Size>0)
            Descriptors();

        Element_Info1(Mpeg_Psi_ATSC_table_type(table_type));
        Element_Info1C((table_type>=0x0100), table_type%0x100);
        Element_End1(Ztring::ToZtring_From_CC2(table_type_PID));

        FILLING_BEGIN();
            if (Complete_Stream->Streams[table_type_PID]->Kind==complete_stream::stream::unknown && table_type!=0x0001 && table_type!=0x0003) //Not activing current_next_indicator='0'
            {
                Complete_Stream->Streams[table_type_PID]->Searching_Payload_Start_Set(true);
                Complete_Stream->Streams[table_type_PID]->Kind=complete_stream::stream::psi;
                Complete_Stream->Streams[table_type_PID]->Table_IDs.resize(0x100);
            }
            #ifdef MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                for (int8u table_id=0x00; table_id<0xFF; table_id++)
                    if (Complete_Stream->Streams[table_type_PID].Table_IDs[table_id]==NULL)
                        Complete_Stream->Streams[table_type_PID].Table_IDs[table_id]=new complete_stream::stream::table_id; //Master Guide Table
            #else //MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                int8u table_id;
                     if (table_type==0x0000) //Terrestrial VCT with current_next_indicator=1
                    table_id=0xC8;
                else if (table_type==0x0002) //Cable VCT with current_next_indicator=1
                    table_id=0xC9;
                else if (table_type==0x0004) //Channel ETT
                    table_id=0xCC;
                else if (table_type>=0x0100 && table_type<=0x017F) //EIT-0 to EIT-127
                    table_id=0xCB;
                else if (table_type>=0x0200 && table_type<=0x027F) //Event ETT-0 to event ETT-127
                    table_id=0xCC;
                else if (table_type>=0x0301 && table_type<=0x03FF) //RRT with rating_region 1-255
                    table_id=0xCA;
                else if (table_type>=0x1000 && table_type<0x10FF) //Aggregate Event Information Table
                    table_id=0xD6;
                else if (table_type>=0x1100 && table_type<0x11FF) //Aggregate Extended Text Table
                    table_id=0xD7;
                else if (table_type>=0x1600 && table_type<0x16FF) //Satellite Virtual Channel Table
                    table_id=0xDA;
                else
                    table_id=0xFF;
                if (table_id!=0xFF && Complete_Stream->Streams[table_type_PID]->Table_IDs[table_id]==NULL)
                    Complete_Stream->Streams[table_type_PID]->Table_IDs[table_id]=new complete_stream::stream::table_id; //Master Guide Table
            #endif //MEDIAINFO_MPEGTS_ALLSTREAMS_YES
            Complete_Stream->Streams[table_type_PID]->table_type=table_type-((table_type&0x200)?0x100:0); //For having the same table_type for both EIT and ETT
        FILLING_END();
    }
    BS_Begin();
    Skip_S1( 4,                                                 "reserved");
    Get_S2 (12, Descriptors_Size,                               "descriptors_length");
    BS_End();

    //Descriptors
    if (Descriptors_Size>0)
        Descriptors();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_C9()
{
    //Parsing
    Ztring short_name;
    int8u num_channels_in_section;
    Skip_B1(                                                    "protocol_version");
    Get_B1 (    num_channels_in_section,                        "num_channels_in_section");
    BS_End();
    for (int8u Pos=0; Pos<num_channels_in_section; Pos++)
    {
        int16u major_channel_number, minor_channel_number, source_id;
        int8u service_type;
        Element_Begin0();
        Get_UTF16B(table_id==0xDA?16:14, short_name,            "short_name"); //8 chars for satellite, else 7 chars
        BS_Begin();
        Skip_S1( 4,                                             "reserved");
        Get_S2 (10, major_channel_number,                       "major_channel_number");
        Get_S2 (10, minor_channel_number,                       "minor_channel_number");
        if (table_id==0xDA) //Satellite
        {
            Skip_S1( 6,                                         "modulation_mode");
            Skip_S4(32,                                         "carrier_frequency");
            Skip_S4(32,                                         "carrier_symbol_rate");
            Skip_S1( 2,                                         "polarization");
            BS_End();
            Skip_B1(                                            "FEC_Inner");
        }
        else //Terrestrial and Cable
        {
            BS_End();
            Skip_B1(                                            "modulation_mode");
            Skip_B4(                                            "carrier_frequency");
        }
        Skip_B2(                                                "channel_TSID");
        Get_B2 (    program_number,                             "program_number");
        BS_Begin();
        Skip_S1( 2,                                             "ETM_location");
        Skip_SB(                                                table_id==0xDA?"reserved":"access_controlled");
        Skip_SB(                                                "hidden");
        if (table_id==0xC8) //Terrestrial
        {
            Skip_SB(                                            "path_select");
            Skip_SB(                                            "out_of_band");
        }
        else //Cable and satellite
            Skip_S1( 2,                                         "reserved");
        Skip_SB(                                                "hide_guide");
        Skip_S1( 3,                                             "reserved");
        Get_S1 ( 6, service_type,                               "service_type");
        BS_End();
        Get_B2 (    source_id,                                  "source_id");
        if (table_id==0xDA) //Satellite
            Skip_B1(                                            "feed_id");
        BS_Begin();
        Skip_S1( 6,                                             "reserved");
        Get_S2 (10, Descriptors_Size,                           "descriptors_length");
        BS_End();

        FILLING_BEGIN();
            if (!Config->File_MpegTs_Atsc_transport_stream_id_Trust_Get())
                table_id_extension=Complete_Stream->transport_stream_id;
            Ztring Channel=Ztring::ToZtring(major_channel_number);
            if (minor_channel_number)
                Channel+=__T("-")+Ztring::ToZtring(minor_channel_number);
            if (minor_channel_number==0 || program_number==0xFFFF)
            {
                Complete_Stream->Transport_Streams[table_id_extension].Infos["ServiceName"]=short_name;
                Complete_Stream->Transport_Streams[table_id_extension].Infos["ServiceChannel"]=Channel;
                Complete_Stream->Transport_Streams[table_id_extension].Infos["ServiceType"]=Mpeg_Psi_atsc_service_type(service_type);
                Complete_Stream->Transport_Streams[table_id_extension].source_id=source_id;
                Complete_Stream->Transport_Streams[table_id_extension].source_id_IsValid=true;
            }
            else if (program_number<0x2000)
            {
                Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceName"]=short_name;
                Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceChannel"]=Channel;
                Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceType"]=Mpeg_Psi_atsc_service_type(service_type);
                Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].source_id=source_id;
                Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].source_id_IsValid=true;
            }
        FILLING_END();

        //Descriptors
        program_number_IsValid=true;
        if (Descriptors_Size>0)
            Descriptors();

        Element_End1(Ztring::ToZtring_From_CC2(program_number));
    }

    BS_Begin();
    Skip_S1( 6,                                             "reserved");
    Get_S2 (10, Descriptors_Size,                           "additional_descriptors_length");
    BS_End();

    //Descriptors
    if (Descriptors_Size>0)
        Descriptors();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_CA()
{
    //Parsing
    Ztring rating_region_name;
    int8u dimensions_defined;
    Skip_B1(                                                    "protocol_version");
    Skip_B1(                                                    "rating_region_name_length"); //Not used
    ATSC_multiple_string_structure(rating_region_name,          "rating_region_name");
    Get_B1 (    dimensions_defined,                             "dimensions_defined");
    BS_End();
    for (int8u dimension_Pos=0; dimension_Pos<dimensions_defined; dimension_Pos++)
    {
        Element_Begin1("dimension");
        Ztring dimension_name;
        int8u values_defined;
        Skip_B1(                                                "dimension_name_length"); //Not used
        ATSC_multiple_string_structure(dimension_name,          "dimension_name");  Element_Info1(dimension_name);
        BS_Begin();
        Skip_S1( 3,                                             "reserved");
        Skip_SB(                                                "graduated_scale");
        Get_S1 ( 4, values_defined,                             "values_defined");
        BS_End();
        for (int8u value_Pos=0; value_Pos<values_defined; value_Pos++)
        {
            Element_Begin1("value");
            Ztring abbrev_rating_value, rating_value;
            Skip_B1(                                            "abbrev_rating_value_length"); //Not used
            ATSC_multiple_string_structure(abbrev_rating_value, "abbrev_rating_value");  Element_Info1(abbrev_rating_value);
            Skip_B1(                                            "rating_value_length"); //Not used
            ATSC_multiple_string_structure(rating_value,        "rating_value"); Element_Info1(rating_value);
            Element_End0();
        }
            Element_End0();
    }

    BS_Begin();
    Skip_S1( 6,                                             "reserved");
    Get_S2 (10, Descriptors_Size,                           "descriptors_length");
    BS_End();

    //Descriptors
    if (Descriptors_Size>0)
        Descriptors();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_CB()
{
    //Clear
    Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[table_id].Events.clear();
    Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks_IsUpdated=true;
    Status[IsUpdated]=true;

    //Parsing
    int8u num_events_in_section;
    if (table_id==0xCB) //EIT (not A-EIT)
        Skip_B1(                                                "protocol_version");
    Get_B1 (    num_events_in_section,                          "num_events_in_section");
    BS_End();
    for (int8u Pos=0; Pos<num_events_in_section; Pos++)
    {
        Ztring title;
        int32u start_time, length_in_seconds;
        Element_Begin0();
        BS_Begin();
        Skip_SB(                                                table_id==0xD9?"off_air":"reserved");
        Skip_SB(                                                "reserved");
        Get_S2 (14, event_id,                                   "event_id");
        BS_End();
        Get_B4 (    start_time,                                 "start_time"); Param_Info1(Ztring().Date_From_Seconds_1970(start_time+315964800)); Element_Info1(Ztring().Date_From_Seconds_1970(start_time+315964800-Complete_Stream->GPS_UTC_offset)); //UTC 1980-01-06 00:00:00
        BS_Begin();
        Skip_S1( 2,                                             "reserved");
        Skip_S1( 2,                                             table_id==0xCB?"ETM_location":"reserved");
        Get_S3 (20, length_in_seconds,                          "length_in_seconds");
        BS_End();
        Skip_B1 (                                               "title_length"); //We don't use it for verification
        ATSC_multiple_string_structure(title,                   "title");
        BS_Begin();
        Skip_S1( 6,                                             "reserved");
        Get_S2 (10, Descriptors_Size,                           "descriptors_length");
        BS_End();

        //Descriptors
        event_id_IsValid=true;
        if (Descriptors_Size>0)
            Descriptors();

        Element_End1(Ztring::ToZtring_From_CC2(event_id));

        FILLING_BEGIN();
            Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].start_time=start_time;
            Ztring duration =(length_in_seconds<36000?__T("0"):__T(""))+Ztring::ToZtring(length_in_seconds/3600)+__T(":");
            length_in_seconds%=3600;
                   duration+=(length_in_seconds<  600?__T("0"):__T(""))+Ztring::ToZtring(length_in_seconds/  60)+__T(":");
            length_in_seconds%=60;
                   duration+=(length_in_seconds<   10?__T("0"):__T(""))+Ztring::ToZtring(length_in_seconds     );
            Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].duration=duration;
            Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].title=title;
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_CC()
{
    //Parsing
    Ztring extended_text_message;
    int16u source_id, event_id;
    Skip_B1(                                                    "protocol_version");
    Element_Begin1("ETM_id");
        Get_B2 (    source_id,                                  "source_id");
        BS_Begin();
        Get_S2 (14, event_id,                                   "event_id");
        Skip_S1( 2,                                             "lsb");
        BS_End();
    Element_End0();
    ATSC_multiple_string_structure(extended_text_message,       "extended_text_message");

    FILLING_BEGIN();
        if (Complete_Stream->Streams[pid]->table_type==4)
            Complete_Stream->Sources[source_id].texts[table_id_extension]=extended_text_message;
        else
        {
            Complete_Stream->Sources[source_id].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].texts[table_id_extension]=extended_text_message;
            Complete_Stream->Sources[source_id].ATSC_EPG_Blocks_IsUpdated=true;
            Complete_Stream->Sources_IsUpdated=true;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_CD()
{
    //Parsing
    int32u system_time;
    int8u  GPS_UTC_offset;
    Skip_B1(                                                    "protocol_version");
    Get_B4 (system_time,                                        "system_time"); Param_Info1(Ztring().Date_From_Seconds_1970(system_time+315964800)); //UTC 1980-01-06 00:00:00
    Get_B1 (GPS_UTC_offset,                                     "GPS_UTC_offset");
    Element_Begin1("daylight_savings");
        BS_Begin();
        Skip_SB(                                                "DS_status");
        Skip_SB(                                                "Reserved");
        Skip_SB(                                                "Reserved");
        Skip_S1(5,                                              "DS_day_of_month");
        BS_End();
        Skip_B1(                                                "DS_hour");
    Element_End0();

    //Descriptors
    Descriptors_Size=(int16u)(Element_Size-Element_Offset);
    if (Descriptors_Size>0)
        Descriptors();

    FILLING_BEGIN();
        if (Complete_Stream->Duration_Start.empty())
            Complete_Stream->Duration_Start=Ztring().Date_From_Seconds_1970(system_time+315964800-GPS_UTC_offset);
        Complete_Stream->Duration_End=Ztring().Date_From_Seconds_1970(system_time+315964800-GPS_UTC_offset);
        Complete_Stream->Duration_End_IsUpdated=true;
        Complete_Stream->GPS_UTC_offset=GPS_UTC_offset;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_D6()
{
    //Parsing
    if ((table_id_extension&0xFF00)==0x0000)
    {
        int8u num_sources_in_section;
        Get_B1 (    num_sources_in_section,                     "num_sources_in_section");
        for (int8u Pos=0; Pos<num_sources_in_section; Pos++)
        {
            Get_B2 (table_id_extension,                         "source_id");
            Table_CB();
        }
    }
    else
        Skip_XX(Element_Size,                                   "reserved");
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC()
{
    //Parsing
    int16u splice_command_length;
    int8u splice_command_type;
    bool encrypted_packet;
    Skip_B1(                                                    "protocol_version");
    BS_Begin();
    Get_SB (    encrypted_packet,                               "encrypted_packet");
    Skip_S1( 6,                                                 "encryption_algorithm");
    Skip_S5(33,                                                 "pts_adjustment");
    Skip_S1( 8,                                                 "cw_index");
    Skip_S2(12,                                                 "reserved");
    Get_S2 (12, splice_command_length,                          "splice_command_length");
    if (splice_command_length==0xFFF)
        splice_command_length=(int16u)(Element_Size-4-Element_Offset);
    Get_S1 ( 8, splice_command_type,                            "splice_command_type"); Param_Info1(Mpeg_Psi_splice_command_type(splice_command_type));
    BS_End();

    Element_Begin0();
    #undef ELEMENT_CASE
    #define ELEMENT_CASE(_NAME, _DETAIL) \
        case 0x##_NAME : Element_Name(_DETAIL); Table_FC_##_NAME(); break;
    switch (splice_command_type)
    {
        ELEMENT_CASE (00, "splice_null"); break;
        ELEMENT_CASE (04, "splice_schedule"); break;
        ELEMENT_CASE (05, "splice_insert"); break;
        ELEMENT_CASE (06, "time_signal"); break;
        ELEMENT_CASE (07, "bandwidth_reservation"); break;
        default   : Skip_XX(splice_command_length,              "Unknown");
    }
    Element_End0();

    if (Element_Offset+4<Element_Size)
    {
        Get_B2 (Descriptors_Size,                               "descriptor_loop_length");
        transport_stream_id=Complete_Stream->transport_stream_id; //SCTE 35 is automaticly linked to the current transport_stream_id
        if (Descriptors_Size>0)
            Descriptors();
    }

    if (Element_Offset+4<Element_Size)
        Skip_XX(Element_Size-(Element_Offset+4),                "alignment_stuffing");
    if (encrypted_packet)
        Skip_B4(                                                "E_CRC_32");
    Skip_B4(                                                    "CRC32");
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_00()
{
    //Null
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_04()
{
    //TODO
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_05()
{
    //Parsing
    bool splice_event_cancel_indicator;
    Skip_B4(                                                    "splice_event_id");
    BS_Begin();
    Get_SB (    splice_event_cancel_indicator,                  "splice_event_cancel_indicator");
    Skip_S1( 7,                                                 "reserved");
    BS_End();
    if (!splice_event_cancel_indicator)
    {
        bool program_splice_flag, duration_flag, splice_immediate_flag;
        BS_Begin();
        Skip_SB(                                                "out_of_network_indicator");
        Get_SB (    program_splice_flag,                        "program_splice_flag");
        Get_SB (    duration_flag,                              "duration_flag");
        Get_SB (    splice_immediate_flag,                      "splice_immediate_flag");
        Skip_S1( 4,                                             "reserved");
        BS_End();
        if(program_splice_flag && !splice_immediate_flag)
            Table_FC_05_splice_time();
        if (!program_splice_flag)
        {
            int8u component_count;
            Get_B1(component_count,                             "component_count");
            for (int8u component=0; component<component_count; component++)
            {
                Skip_B1(                                        "component_tag");
                Table_FC_05_splice_time();
            }
        }
        if(duration_flag)
            Table_FC_05_break_duration();
        Skip_B2(                                                "unique_program_id");
        Skip_B1(                                                "avail_num");
        Skip_B1(                                                "avails_expected");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_05_break_duration()
{
    Element_Begin1("break_duration");

    //Parsing
    BS_Begin();
    Skip_SB(                                                    "auto_return");
    Skip_S1( 6,                                                 "reserved");
    Skip_S5(33,                                                 "duration");
    BS_End();

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_05_splice_time()
{
    Element_Begin1("splice_time");

    //Parsing
    bool time_specified_flag;
    BS_Begin();
    Get_SB (    time_specified_flag,                            "time_specified_flag");
    if (time_specified_flag)
    {
        Skip_S1( 6,                                             "reserved");
        Skip_S5(33,                                             "pts_time");

    }
    else
        Skip_S5(7,                                              "reserved");
    BS_End();

   Element_End0();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_06()
{
    Table_FC_05_splice_time();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Table_FC_07()
{
    //TODO
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Psi::Descriptors()
{
    if (Element_Offset+Descriptors_Size>Element_Size)
    {
        Trusted_IsNot("Descriptor size too big");
        return;
    }

    //Configuring
    File_Mpeg_Descriptors Descriptors;
    Descriptors.Complete_Stream=Complete_Stream;
    Descriptors.transport_stream_id=transport_stream_id;
    Descriptors.pid=pid;
    Descriptors.table_id=table_id;
    Descriptors.table_id_extension=table_id_extension;
    Descriptors.elementary_PID=elementary_PID;
    Descriptors.program_number=program_number;
    Descriptors.stream_type=stream_type;
    Descriptors.event_id=event_id;
    Descriptors.elementary_PID_IsValid=elementary_PID_IsValid;
    Descriptors.program_number_IsValid=program_number_IsValid;
    Descriptors.stream_type_IsValid=stream_type_IsValid;
    Descriptors.event_id_IsValid=event_id_IsValid;

    //Parsing
    if (Descriptors_Size!=0)
    {
        Element_Begin1("Descriptors");
        Open_Buffer_Init(&Descriptors);
        Open_Buffer_Continue(&Descriptors, Descriptors_Size);
        Element_End0();
    }

    //Configuring
    elementary_PID_IsValid=false;
    program_number_IsValid=false;
    stream_type_IsValid=false;
    event_id_IsValid=false;
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::ATSC_multiple_string_structure(Ztring &Value, const char* Name)
{
    //Parsing
    Ztring string;
    int8u number_strings, number_segments;
    Element_Begin1(Name);
    Get_B1(number_strings,                                      "number_strings");
    for (int8u string_Pos=0; string_Pos<number_strings; string_Pos++)
    {
        Element_Begin1("String");
        int32u ISO_639_language_code;
        Get_C3(ISO_639_language_code,                           "ISO_639_language_code");
        Get_B1(number_segments,                                 "number_segments");
        for (int8u segment_Pos=0; segment_Pos<number_segments; segment_Pos++)
        {
            Element_Begin1("Segment");
            Ztring segment;
            int8u compression_type, mode, number_bytes;
            Get_B1 (compression_type,                           "compression_type");
            Get_B1 (mode,                                       "mode");
            Get_B1 (number_bytes,                               "number_bytes");
            switch (compression_type)
            {
                case 0x00 :
                            switch (mode)
                            {
                                case 0x00 : Get_Local(number_bytes, segment, "string"); break;
                                case 0x3F : Get_UTF16B(number_bytes, segment, "string"); break;
                                default   : Skip_XX(number_bytes, "Unknown");
                                            segment=__T("(Encoded with mode=0x")+Ztring::ToZtring(mode, 16)+__T(')');
                            }
                            break;
                default   : Skip_XX(number_bytes,               "(Compressed)");
                            segment=__T("(Compressed)");
            }
            Element_End0();

            FILLING_BEGIN();
                if (segment.find_first_not_of(__T("\t\n "))!=std::string::npos)
                    string+=segment+__T(" - ");
            FILLING_END();
        }

        FILLING_BEGIN();
            if (!string.empty())
                string.resize(string.size()-3);
            Ztring ISO_639_2=Ztring().From_CC3(ISO_639_language_code);
            const Ztring& ISO_639_1=MediaInfoLib::Config.Iso639_1_Get(ISO_639_2);
            Value+=(ISO_639_1.empty()?ISO_639_2:ISO_639_1)+__T(':')+string+__T(" - ");
        FILLING_END();

        Element_Info1(string);
        Element_End1("String");
    }

    if (!Value.empty())
        Value.resize(Value.size()-3);

    Element_Info1(Value);
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::SCTE_multilingual_text_string(int8u Size, Ztring &Value, const char* Name)
{
    //Parsing
    Element_Begin1(Name);
    int64u End=Element_Offset+Size;
    while (Element_Offset<End)
    {
        int8u mode;
        Get_B1 (mode,                                           "mode");
        if (mode<0x3F)
        {
            int8u eightbit_string_length;
            Get_B1 (eightbit_string_length,                     "eightbit_string_length");
            if (mode==0)
                Get_ISO_8859_1(eightbit_string_length, Value,   "eightbit_string");
            else
                Skip_XX(eightbit_string_length,                 "eightbit_string (unsupporeted)");
        }
        else if (mode==0x3F)
        {
            int8u sixteenbit_string_length;
            Get_B1 (sixteenbit_string_length,                   "sixteenbit_string_length");
            Get_UTF16B(sixteenbit_string_length, Value,         "sixteenbit_string");
        }
        else if (mode>=0xA0)
        {
            int8u format_effector_param_length;
            Get_B1 (format_effector_param_length,               "format_effector_param_length");
            Skip_XX(format_effector_param_length,               "format_effector_data");
        }
    }
    Element_End0();
}

//---------------------------------------------------------------------------
//Modified Julian Date
Ztring File_Mpeg_Psi::Date_MJD(int16u Date_)
{
    //Calculating
    float64 Date=Date_;
    int Y2=(int)((Date-15078.2)/365.25);
    int M2=(int)(((Date-14956.1) - ((int)(Y2*365.25))) /30.6001);
    int D =(int)(Date-14956 - ((int)(Y2*365.25)) - ((int)(M2*30.6001)));
    int K=0;
    if (M2==14 || M2==15)
        K=1;
    int Y =Y2+K;
    int M =M2-1-K*12;

    //Formating
    return                       Ztring::ToZtring(1900+Y)+__T("-")
         + (M<10?__T("0"):__T(""))+Ztring::ToZtring(     M)+__T("-")
         + (D<10?__T("0"):__T(""))+Ztring::ToZtring(     D);
}

//---------------------------------------------------------------------------
//Form: HHMMSS, BCD
Ztring File_Mpeg_Psi::Time_BCD(int32u Time)
{
    return (((Time>>16)&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time>>16)&0xFF, 16)+__T(":") //BCD
         + (((Time>> 8)&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time>> 8)&0xFF, 16)+__T(":") //BCD
         + (((Time    )&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time    )&0xFF, 16);        //BCD
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Psi::program_number_Update()
{
    //Setting the pid as program_map_section
    if (Complete_Stream->Streams[elementary_PID]->Kind!=complete_stream::stream::psi)
    {
        Complete_Stream->Streams[elementary_PID]->Searching_Payload_Start_Set(true);
        Complete_Stream->Streams[elementary_PID]->Kind=complete_stream::stream::psi;
        Complete_Stream->Streams[elementary_PID]->Table_IDs.resize(0x100);
        if (program_number)
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]=new complete_stream::stream::table_id; //program_map_section
    }
    if (Complete_Stream->File__Duplicate_Get_From_PID(elementary_PID))
        Complete_Stream->Streams[elementary_PID]->ShouldDuplicate=true;

    //Handling a program
    if (program_number)
    {
        Complete_Stream->Transport_Streams[table_id_extension].Programs_NotParsedCount++;
        Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].pid=elementary_PID;
        Complete_Stream->Transport_Streams[table_id_extension].programs_List.push_back(program_number);
        if (Complete_Stream->Streams.size()<0x2000)
            Complete_Stream->Streams.resize(0x2000); //TODO: find the reason this code is called
        Complete_Stream->Streams[elementary_PID]->program_numbers.push_back(program_number);
        if (Complete_Stream->Streams[elementary_PID]->Table_IDs.size()<0x100)
            Complete_Stream->Streams[elementary_PID]->Table_IDs.resize(0x100); //TODO: find the reason this code is called
        if (Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]==NULL)
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]=new complete_stream::stream::table_id; //TODO: find the reason this code is called
        if (Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]->Table_ID_Extensions.find(program_number)==Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]->Table_ID_Extensions.end())
        {
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]->Table_ID_Extensions_CanAdd=false;
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]->Table_ID_Extensions[program_number].version_number=0xFF;
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]->Table_ID_Extensions[program_number].Section_Numbers.clear();
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0x02]->Table_ID_Extensions[program_number].Section_Numbers.resize(0x100);
        }
    }

    //Handling a network except basic version
    else if (Complete_Stream->Streams[elementary_PID]->Table_IDs[0x00]==NULL)
    {
        for (size_t Table_ID=1; Table_ID<0x100; Table_ID++)
        {
            if (Complete_Stream->Streams[elementary_PID]->Table_IDs[Table_ID]==NULL)
                Complete_Stream->Streams[elementary_PID]->Table_IDs[Table_ID]=new complete_stream::stream::table_id; //all

            if (Table_ID==1)
                Table_ID++; //Skipping TableID 2
        }
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::program_number_Remove()
{
    //Removing this program_number from the list of program_numbers for each elementary_PID
    for (size_t Pos=0; Pos<Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs.size(); Pos++)
    {
        int16u elementary_PID_Temp=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs[Pos];

        //Removing this program_number from the list of program_numbers for this elementary_PID
        for (size_t Pos=0; Pos<Complete_Stream->Streams[elementary_PID_Temp]->program_numbers.size(); Pos++)
            if (Complete_Stream->Streams[elementary_PID_Temp]->program_numbers[Pos]==program_number)
                Complete_Stream->Streams[elementary_PID_Temp]->program_numbers.erase(Complete_Stream->Streams[elementary_PID_Temp]->program_numbers.begin()+Pos);

        //Removing parser if no more program_number
        if (Complete_Stream->Streams[elementary_PID_Temp]->program_numbers.empty())
        {
            stream_t StreamKind=Complete_Stream->Streams[elementary_PID_Temp]->StreamKind;
            size_t StreamPos=Complete_Stream->Streams[elementary_PID_Temp]->StreamPos;
            if (StreamKind!=Stream_Max && StreamPos!=(size_t)-1)
                Complete_Stream->StreamPos_ToRemove[StreamKind].push_back(StreamPos);

            if (Complete_Stream->Streams_NotParsedCount!=(size_t)-1 && Complete_Stream->Streams_NotParsedCount && !Complete_Stream->Streams[elementary_PID_Temp]->IsParsed)
                Complete_Stream->Streams_NotParsedCount--; //Not parsed, and no need to parse it now
            delete Complete_Stream->Streams[elementary_PID_Temp]; Complete_Stream->Streams[elementary_PID_Temp]=new complete_stream::stream;
        }
    }

    //Removing related PCR
    std::map<int16u, int16u>::iterator PCR_PID=Complete_Stream->PCR_PIDs.find(Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].PCR_PID);
    if (PCR_PID!=Complete_Stream->PCR_PIDs.end())
    {
        PCR_PID->second--;
        if (PCR_PID->second==0)
            Complete_Stream->PCR_PIDs.erase(PCR_PID);
    }

    //Removing program_number
    size_t StreamPos=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].StreamPos;
    if (StreamPos!=(size_t)-1)
    {
        Complete_Stream->StreamPos_ToRemove[Stream_Menu].push_back(StreamPos);
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].StreamPos=(size_t)-1;
    }
    int16u program_number_pid=Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].pid;
    if (program_number_pid)
    {
        for (size_t Pos=0; Pos<Complete_Stream->Streams[program_number_pid]->program_numbers.size(); Pos++)
            if (Complete_Stream->Streams[program_number_pid]->program_numbers[Pos]==program_number)
                Complete_Stream->Streams[program_number_pid]->program_numbers.erase(Complete_Stream->Streams[program_number_pid]->program_numbers.begin()+Pos);
        if (Complete_Stream->Streams[program_number_pid]->Table_IDs[0x02])
            Complete_Stream->Streams[program_number_pid]->Table_IDs[0x02]->Table_ID_Extensions.erase(program_number);
    }
    Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs.erase(program_number);
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::elementary_PID_Update(int16u PCR_PID)
{
    if (Complete_Stream->Streams[elementary_PID]->Kind==complete_stream::stream::psi)
    {
        //A PID can not be PSI and PES at the same time
        return;
    }

    //stream_type
    if (stream_type!=Complete_Stream->Streams[elementary_PID]->stream_type && Complete_Stream->Streams[elementary_PID]->stream_type!=(int8u)-1)
    {
        if (Complete_Stream->Streams_NotParsedCount!=(size_t)-1 && Complete_Stream->Streams_NotParsedCount && !Complete_Stream->Streams[elementary_PID]->IsParsed)
            Complete_Stream->Streams_NotParsedCount--; //Not parsed, and no need to parse it now
        delete Complete_Stream->Streams[elementary_PID]; Complete_Stream->Streams[elementary_PID]=new complete_stream::stream;
        Complete_Stream->Streams[elementary_PID]->Kind=complete_stream::stream::unknown;
    }
    if (Complete_Stream->Streams[elementary_PID]->Kind!=complete_stream::stream::pes)
    {
        delete Complete_Stream->Streams[elementary_PID]; Complete_Stream->Streams[elementary_PID]=new complete_stream::stream;

        if (Complete_Stream->Streams_NotParsedCount==(size_t)-1)
            Complete_Stream->Streams_NotParsedCount=0;
        Complete_Stream->Streams_NotParsedCount++;
        if (stream_type==0x86 && Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].registration_format_identifier==Elements::CUEI)
        {
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].HasNotDisplayableStreams=true;
            Complete_Stream->Streams[elementary_PID]->Kind=complete_stream::stream::psi;
            Complete_Stream->Streams[elementary_PID]->Table_IDs.resize(0x100);
            Complete_Stream->Streams[elementary_PID]->Table_IDs[0xFC]=new complete_stream::stream::table_id; //Splice
            if (Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].Scte35==NULL)
            {
                Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].Scte35=new complete_stream::transport_stream::program::scte35;
                Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].Scte35->pid=elementary_PID;
            }
            #if MEDIAINFO_TRACE
                Complete_Stream->Streams[elementary_PID]->Element_Info1="PSI";
            #endif //MEDIAINFO_TRACE
        }
        else
        {
            Complete_Stream->Streams[elementary_PID]->Kind=complete_stream::stream::pes;
            Complete_Stream->Streams[elementary_PID]->Infos["CodecID"].From_Number(stream_type);
            #if MEDIAINFO_TRACE
                Complete_Stream->Streams[elementary_PID]->Element_Info1="PES";
            #endif //MEDIAINFO_TRACE
        }
        Complete_Stream->Streams[elementary_PID]->stream_type=stream_type;
        Complete_Stream->Streams[elementary_PID]->Searching_Payload_Start_Set(true);
        #ifdef MEDIAINFO_MPEGTS_PCR_YES
            Complete_Stream->Streams[elementary_PID]->Searching_TimeStamp_Start_Set(true);
            Complete_Stream->Streams[elementary_PID]->PCR_PID=PCR_PID;
        #endif //MEDIAINFO_MPEGTS_PCR_YES
        #ifdef MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
            //Complete_Stream->Streams[elementary_PID]->Searching_ParserTimeStamp_Start_Set(true);
        #endif //MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
        if (Complete_Stream->File__Duplicate_Get_From_PID(elementary_PID))
            Complete_Stream->Streams[elementary_PID]->ShouldDuplicate=true;
    }

    //Program information
    bool IsAlreadyPresent=false;
    for (size_t Pos=0; Pos<Complete_Stream->Streams[elementary_PID]->program_numbers.size(); Pos++)
        if (Complete_Stream->Streams[elementary_PID]->program_numbers[Pos]==program_number)
            IsAlreadyPresent=true;
    if (!IsAlreadyPresent)
    {
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs.push_back(elementary_PID);
        Complete_Stream->Streams[elementary_PID]->program_numbers.push_back(program_number);
        if (ForceStreamDisplay || (Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].registration_format_identifier==Elements::HDMV && Complete_Stream->Streams[elementary_PID]->stream_type==0x90)) //Testing if forcing display of all streams or if it is a PGS from Blu-ray
            Complete_Stream->PES_PIDs.insert(elementary_PID); //Adding it for sure
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Psi::elementary_PID_Remove()
{
    //Removing this elementary_PID from the list of elementary_PIDs for this program_number
    for (size_t Pos=0; Pos<Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs.size(); Pos++)
        if (Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs[Pos]==elementary_PID)
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs.erase(Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[program_number].elementary_PIDs.begin()+Pos);

    //Removing this program_number from the list of program_numbers for this elementary_PID
    for (size_t Pos=0; Pos<Complete_Stream->Streams[elementary_PID]->program_numbers.size(); Pos++)
        if (Complete_Stream->Streams[elementary_PID]->program_numbers[Pos]==program_number)
            Complete_Stream->Streams[elementary_PID]->program_numbers.erase(Complete_Stream->Streams[elementary_PID]->program_numbers.begin()+Pos);

    //Removing parser if no more program_number
    if (Complete_Stream->Streams[elementary_PID]->program_numbers.empty())
    {
        stream_t StreamKind=Complete_Stream->Streams[elementary_PID]->StreamKind;
        size_t StreamPos=Complete_Stream->Streams[elementary_PID]->StreamPos;
        if (StreamKind!=Stream_Max && StreamPos!=(size_t)-1)
            Complete_Stream->StreamPos_ToRemove[StreamKind].push_back(StreamPos);

        if (Complete_Stream->Streams_NotParsedCount!=(size_t)-1 && Complete_Stream->Streams_NotParsedCount && !Complete_Stream->Streams[elementary_PID]->IsParsed)
            Complete_Stream->Streams_NotParsedCount--; //Not parsed, and no need to parse it now
        delete Complete_Stream->Streams[elementary_PID]; Complete_Stream->Streams[elementary_PID]=new complete_stream::stream;
        Complete_Stream->PES_PIDs.erase(elementary_PID);
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_MPEGTS_YES

