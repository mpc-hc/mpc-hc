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
#include "MediaInfo/Multiple/File_Mpeg_Descriptors.h"
#ifdef MEDIAINFO_MPEG4_YES
    #include "MediaInfo/Multiple/File_Mpeg4_Descriptors.h"
#endif
#include <cmath>
using namespace std;
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

namespace Elements
{
    const int32u AC_3=0x41432D33; //Exactly AC-3
    const int32u BSSD=0x42535344; //PCM
    const int32u CUEI=0x43554549; //SCTE
    const int32u DTS1=0x44545331; //DTS
    const int32u DTS2=0x44545332; //DTS
    const int32u DTS3=0x44545333; //DTS
    const int32u GA94=0x47413934; //ATSC - Terrestrial
    const int32u HDMV=0x48444D56; //BluRay
    const int32u HEVC=0x48455643; //HEVC
    const int32u KLVA=0x4B4C5641; //KLV Packets
    const int32u S14A=0x53313441; //ATSC - Satellite
    const int32u SCTE=0x53435445; //SCTE
    const int32u TSHV=0x54534856; //TSHV
    const int32u VC_1=0x56432D31; //Exactly VC-1
    const int32u drac=0x64726163; //Dirac

    const int32u MANZ=0x4D414E5A; //Manzanita Systems

    const int32u DVB =0x00000001; //Forced value, does not exist is stream
}

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
//Extern
extern const char* Avc_profile_idc(int8u profile_idc);

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_audio_type(int8u ID)
{
    switch (ID)
    {
        case 0x00 : return "Undefined";
        case 0x01 : return "Clean effects";
        case 0x02 : return "Hearing impaired";
        case 0x03 : return "Visual impaired commentary";
        default   : return "Reserved";
    }
}

const char* Mpeg_Descriptors_alignment_type(int8u alignment_type)
{
    switch (alignment_type)
    {
        case 0x01 : return "Slice or video access unit (Video), or sync word (Audio)";
        case 0x02 : return "Video access unit";
        case 0x03 : return "GOP, or SEQ";
        case 0x04 : return "SEQ";
        default   : return "Reserved";
    }
}

const char* Mpeg_Descriptors_teletext_type(int8u teletext_type)
{
    switch (teletext_type)
    {
        case 0x01 : return "Teletext";
        case 0x02 : return "Teletext Subtitle";
        case 0x03 : return "Teletext"; //additional information page
        case 0x04 : return "Teletext"; //programme schedule page
        case 0x05 : return "Teletext Subtitle"; //for hearing impaired people
        default   : return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_1(int8u content_nibble_level_1)
{
    switch (content_nibble_level_1)
    {
        case 0x00 : return "undefined";
        case 0x01 : return "movie/drama";
        case 0x02 : return "news/current affairs";
        case 0x03 : return "show/game show";
        case 0x04 : return "sports";
        case 0x05 : return "children's/youth programmes";
        case 0x06 : return "music/ballet/dance";
        case 0x07 : return "arts/culture (without music)";
        case 0x08 : return "social/political issues/economics";
        case 0x09 : return "education/science/factual topics";
        case 0x0A : return "leisure hobbies";
        case 0x0B : return "Special characteristics:";
        default   :
            if (content_nibble_level_1==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_01(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "movie/drama";
        case 0x01 : return "detective/thriller";
        case 0x02 : return "adventure/western/war";
        case 0x03 : return "science fiction/fantasy/horror";
        case 0x04 : return "comedy";
        case 0x05 : return "soap/melodrama/folkloric";
        case 0x06 : return "romance";
        case 0x07 : return "serious/classical/religious/historical movie/drama";
        case 0x08 : return "adult movie/drama";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_02(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "news/current affairs";
        case 0x01 : return "news/weather report";
        case 0x02 : return "news magazine";
        case 0x03 : return "documentary";
        case 0x04 : return "discussion/interview/debate";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_03(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "show/game show";
        case 0x01 : return "game show/quiz/contest";
        case 0x02 : return "variety show";
        case 0x03 : return "talk show";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_04(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "sports";
        case 0x01 : return "special events";
        case 0x02 : return "sports magazines";
        case 0x03 : return "football/soccer";
        case 0x04 : return "tennis/squash";
        case 0x05 : return "team sports (excluding football)";
        case 0x06 : return "athletics";
        case 0x07 : return "motor sport";
        case 0x08 : return "water sport";
        case 0x09 : return "winter sports";
        case 0x0A : return "equestrian";
        case 0x0B : return "martial sports";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_05(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "children's/youth programmes";
        case 0x01 : return "pre-school children's programmes";
        case 0x02 : return "entertainment programmes for 6 to 14";
        case 0x03 : return "entertainment programmes for 10 to 16";
        case 0x04 : return "informational/educational/school programmes";
        case 0x05 : return "cartoons/puppets";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_06(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "music/ballet/dance";
        case 0x01 : return "rock/pop";
        case 0x02 : return "serious music/classical music";
        case 0x03 : return "folk/traditional music";
        case 0x04 : return "jazz";
        case 0x05 : return "musical/opera";
        case 0x06 : return "ballet";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_07(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "arts/culture (without music)";
        case 0x01 : return "performing arts";
        case 0x02 : return "fine arts";
        case 0x03 : return "religion";
        case 0x04 : return "popular culture/traditional arts";
        case 0x05 : return "literature";
        case 0x06 : return "film/cinema";
        case 0x07 : return "experimental film/video";
        case 0x08 : return "broadcasting/press";
        case 0x09 : return "new media";
        case 0x0A : return "arts/culture magazines";
        case 0x0B : return "fashion";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_08(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "social/political issues/economics";
        case 0x01 : return "magazines/reports/documentary";
        case 0x02 : return "economics/social advisory";
        case 0x03 : return "remarkable people";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_09(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "education/science/factual topics";
        case 0x01 : return "nature/animals/environment";
        case 0x02 : return "technology/natural sciences";
        case 0x03 : return "medicine/physiology/psychology";
        case 0x04 : return "foreign countries/expeditions";
        case 0x05 : return "social/spiritual sciences";
        case 0x06 : return "further education";
        case 0x07 : return "languages";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_0A(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "leisure hobbies";
        case 0x01 : return "tourism/travel";
        case 0x02 : return "handicraft";
        case 0x03 : return "motoring";
        case 0x04 : return "fitness and health";
        case 0x05 : return "cooking";
        case 0x06 : return "advertisement/shopping";
        case 0x07 : return "gardening";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2_0B(int8u content_nibble_level_2)
{
    switch (content_nibble_level_2)
    {
        case 0x00 : return "original language";
        case 0x01 : return "black and white";
        case 0x02 : return "unpublished";
        case 0x03 : return "live broadcast";
        default   :
            if (content_nibble_level_2==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_content_nibble_level_2(int8u content_nibble_level_1, int8u content_nibble_level_2)
{
    switch (content_nibble_level_1)
    {
        case 0x00 : return "undefined";
        case 0x01 : return Mpeg_Descriptors_content_nibble_level_2_01(content_nibble_level_2);
        case 0x02 : return Mpeg_Descriptors_content_nibble_level_2_02(content_nibble_level_2);
        case 0x03 : return Mpeg_Descriptors_content_nibble_level_2_03(content_nibble_level_2);
        case 0x04 : return Mpeg_Descriptors_content_nibble_level_2_04(content_nibble_level_2);
        case 0x05 : return Mpeg_Descriptors_content_nibble_level_2_05(content_nibble_level_2);
        case 0x06 : return Mpeg_Descriptors_content_nibble_level_2_06(content_nibble_level_2);
        case 0x07 : return Mpeg_Descriptors_content_nibble_level_2_07(content_nibble_level_2);
        case 0x08 : return Mpeg_Descriptors_content_nibble_level_2_08(content_nibble_level_2);
        case 0x09 : return Mpeg_Descriptors_content_nibble_level_2_09(content_nibble_level_2);
        case 0x0A : return Mpeg_Descriptors_content_nibble_level_2_0A(content_nibble_level_2);
        case 0x0B : return Mpeg_Descriptors_content_nibble_level_2_0B(content_nibble_level_2);
        default   :
            if (content_nibble_level_1==0x0F)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_linkage_type(int8u linkage_type)
{
    switch (linkage_type)
    {
        case 0x00 : return "reserved for future use";
        case 0x01 : return "information service";
        case 0x02 : return "Electronic Programme Guide (EPG) service";
        case 0x03 : return "CA replacement service";
        case 0x04 : return "transport stream containing complete Network/Bouquet SI";
        case 0x05 : return "service replacement service";
        case 0x06 : return "data broadcast service";
        case 0xFF : return "reserved for future use";
        default   :
            if (linkage_type>=0x80)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_dvb_service_type(int8u service_type)
{
    switch (service_type)
    {
        case 0x01 : return "digital television";
        case 0x02 : return "digital radio";
        case 0x03 : return "teletext";
        case 0x04 : return "NVOD reference";
        case 0x05 : return "NVOD time-shifted";
        case 0x06 : return "Mosaic";
        case 0x0A : return "advanced codec digital radio sound";
        case 0x0B : return "advanced codec mosaic service";
        case 0x0C : return "data broadcast";
        case 0x0D : return "reserved for Common Interface Usage";
        case 0x0E : return "RCS Map";
        case 0x0F : return "RCS FLS";
        case 0x10 : return "DVB MHP";
        case 0x11 : return "MPEG-2 HD digital television";
        case 0x16 : return "advanced codec SD digital television";
        case 0x17 : return "advanced codec SD NVOD time-shifted";
        case 0x18 : return "advanced codec SD NVOD reference";
        case 0x19 : return "advanced codec HD digital television";
        case 0x1A : return "advanced codec HD NVOD time-shifted";
        case 0x1B : return "advanced codec HD NVOD reference";
        case 0xFF : return "reserved for future use";
        default   :
            if (service_type>=0x80)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_stream_content(int8u stream_content)
{
    switch (stream_content)
    {
        case 0x01 : return "MPEG-2 Video";
        case 0x02 : return "MPEG-1 Audio L2";
        case 0x03 : return "Subtitle";
        case 0x04 : return "AC3";
        case 0x05 : return "AVC";
        case 0x06 : return "HE-AAC";
        case 0x07 : return "DTS";
        default   :
            if (stream_content>=0x0C)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type_O1(int8u component_type)
{
    switch (component_type)
    {
        case 0x01 : return "4:3 aspect ratio, 25 Hz";
        case 0x02 : return "16:9 aspect ratio with pan vectors, 25 Hz";
        case 0x03 : return "16:9 aspect ratio without pan vectors, 25 Hz";
        case 0x04 : return ">16:9 aspect ratio, 25 Hz";
        case 0x05 : return "4:3 aspect ratio, 30 Hz";
        case 0x06 : return "16:9 aspect ratio with pan vectors, 30 Hz";
        case 0x07 : return "16:9 aspect ratio without pan vectors, 30 Hz";
        case 0x08 : return ">16:9 aspect ratio, 30 Hz";
        case 0x09 : return "4:3 aspect ratio, 25 Hz (high definition)";
        case 0x0A : return "16:9 aspect ratio with pan vectors, 25 Hz (high definition)";
        case 0x0B : return "16:9 aspect ratio without pan vectors, 25 Hz (high definition)";
        case 0x0C : return ">16:9 aspect ratio, 25 Hz (high definition)";
        case 0x0D : return "4:3 aspect ratio, 30 Hz (high definition)";
        case 0x0E : return "16:9 aspect ratio with pan vectors, 30 Hz (high definition)";
        case 0x0F : return "16:9 aspect ratio without pan vectors, 30 Hz (high definition)";
        case 0x10 : return ">16:9 aspect ratio, 30 Hz (high definition)";
        default   :
            if (component_type>=0xB0 && component_type<=0xFE)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type_O2(int8u component_type)
{
    switch (component_type)
    {
        case 0x01 : return "single mono channel";
        case 0x02 : return "dual mono channel";
        case 0x03 : return "stereo (2 channel)";
        case 0x04 : return "multi-lingual, multi-channel";
        case 0x05 : return "surround sound";
        case 0x40 : return "description for the visually impaired";
        case 0x41 : return "for the hard of hearing";
        case 0x42 : return "receiver-mixed supplementary audio";
        default   :
            if (component_type>=0xB0 && component_type<=0xFE)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type_O3(int8u component_type)
{
    switch (component_type)
    {
        case 0x01 : return "EBU Teletext subtitles";
        case 0x02 : return "associated EBU Teletext";
        case 0x03 : return "VBI data";
        case 0x10 : return "DVB subtitle (normal) with no monitor aspect ratio criticality";
        case 0x11 : return "DVB subtitle (normal) for display on 4:3 aspect ratio monitor";
        case 0x12 : return "DVB subtitle (normal) for display on 16:9 aspect ratio monitor";
        case 0x13 : return "DVB subtitle (normal) for display on 2.21:1 aspect ratio monitor";
        case 0x20 : return "DVB subtitle (for the hard of hearing) with no monitor aspect ratio criticality";
        case 0x21 : return "DVB subtitle (for the hard of hearing) for display on 4:3 aspect ratio monitor";
        case 0x22 : return "DVB subtitle (for the hard of hearing) for display on 16:9 aspect ratio monitor";
        case 0x23 : return "DVB subtitle (for the hard of hearing) for display on 2.21:1 aspect ratio monitor";
        default   :
            if (component_type>=0xB0 && component_type<=0xFE)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type_O4(int8u)
{
    return "Defined by AC3";
}

const char* Mpeg_Descriptors_component_type_O5(int8u component_type)
{
    switch (component_type)
    {
        case 0x01 : return "4:3 aspect ratio, 25 Hz";
        case 0x03 : return "16:9 aspect ratio, 25 Hz";
        case 0x04 : return ">16:9 aspect ratio, 25 Hz";
        case 0x05 : return "4:3 aspect ratio, 30 Hz";
        case 0x07 : return "16:9 aspect ratio, 30 Hz";
        case 0x08 : return ">16:9 aspect ratio, 30 Hz";
        case 0x0B : return "16:9 aspect ratio, 25 Hz (high definition)";
        case 0x0C : return ">16:9 aspect ratio, 25 Hz (high definition)";
        case 0x0F : return "16:9 aspect ratio, 30 Hz (high definition)";
        case 0x10 : return ">16:9 aspect ratio, 30 Hz (high definition)";
        default   :
            if (component_type>=0xB0 && component_type<=0xFE)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type_O6(int8u component_type)
{
    switch (component_type)
    {
        case 0x01 : return "single mono channel";
        case 0x03 : return "stereo";
        case 0x05 : return "surround sound";
        case 0x40 : return "description for the visually impaired";
        case 0x41 : return "for the hard of hearing";
        case 0x42 : return "receiver-mixed supplementary audio";
        case 0x43 : return "astereo (v2)";
        case 0x44 : return "description for the visually impaired (v2)";
        case 0x45 : return "for the hard of hearing (v2)";
        case 0x46 : return "receiver-mixed supplementary audio (v2)";
        default   :
            if (component_type>=0xB0 && component_type<=0xFE)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type_O7(int8u)
{
    return "Defined by DTS";
}

const char* Mpeg_Descriptors_codepage_1(int8u codepage)
{
    switch (codepage)
    {
        case 0x01 : return "ISO/IEC 8859-5 (Cyrillic)";
        case 0x02 : return "ISO/IEC 8859-6 (Arabic)";
        case 0x03 : return "ISO/IEC 8859-7 (Greek)";
        case 0x04 : return "ISO/IEC 8859-8 (Hebrew)";
        case 0x05 : return "ISO/IEC 8859-9 (Latin)";
        case 0x06 : return "ISO/IEC 8859-10 (Latin)";
        case 0x07 : return "ISO/IEC 8859-11 (Thai)";
        case 0x08 : return "ISO/IEC 8859-12 (Indian)";
        case 0x09 : return "ISO/IEC 8859-13 (Latin)";
        case 0x0A : return "ISO/IEC 8859-14 (Celtic)";
        case 0x0B : return "ISO/IEC 8859-15 (Latin)";
        case 0x11 : return "ISO/IEC 10646-1 (Basic Multilingual Plane)";
        case 0x12 : return "KSC5601-1987 (Korean)";
        case 0x13 : return "GB-2312-1980 (Simplified Chinese)";
        case 0x14 : return "Big5 (Traditional Chinese)";
        case 0x15 : return "UTF-8 (Basic Multilingual Plane)";
        default   : return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_component_type(int8u stream_content, int8u component_type)
{
    switch (stream_content)
    {
        case 0x01 : return Mpeg_Descriptors_component_type_O1(component_type);
        case 0x02 : return Mpeg_Descriptors_component_type_O2(component_type);
        case 0x03 : return Mpeg_Descriptors_component_type_O3(component_type);
        case 0x04 : return Mpeg_Descriptors_component_type_O4(component_type);
        case 0x05 : return Mpeg_Descriptors_component_type_O5(component_type);
        case 0x06 : return Mpeg_Descriptors_component_type_O6(component_type);
        case 0x07 : return Mpeg_Descriptors_component_type_O7(component_type);
        default   :
            if (component_type>=0xB0 && component_type<=0xFE)
                    return "user defined";
            else
                    return "reserved for future use";
    }
}

const char* Mpeg_Descriptors_registration_format_identifier_Format(int32u format_identifier)
{
    switch (format_identifier)
    {
        case Elements::AC_3 : return "AC-3";
        case Elements::BSSD : return "PCM"; //AES3
        case Elements::CUEI : return "SCTE 35 2003 - Digital Program Insertion Cueing Message for Cable";
        case Elements::DTS1 : return "DTS"; //512
        case Elements::DTS2 : return "DTS"; //1024
        case Elements::DTS3 : return "DTS"; //2048
        case Elements::GA94 : return "ATSC - Terrestrial";
        case Elements::HDMV : return "Blu-ray";
        case Elements::HEVC : return "HEVC";
        case Elements::KLVA : return "KLV";
        case Elements::S14A : return "ATSC - Satellite";
        case Elements::SCTE : return "SCTE 54 2003 - DV Service Multiplex and Transport System for Cable Television";
        case Elements::TSHV : return "DV";
        case Elements::VC_1 : return "VC-1";
        case Elements::MANZ : return "Manzanita Systems"; //Manzanita Systems
        default :                     return "";
    }
}

stream_t Mpeg_Descriptors_registration_format_identifier_StreamKind(int32u format_identifier)
{
    switch (format_identifier)
    {
        case Elements::AC_3 : return Stream_Audio;
        case Elements::BSSD : return Stream_Audio;
        case Elements::DTS1 : return Stream_Audio;
        case Elements::DTS2 : return Stream_Audio;
        case Elements::DTS3 : return Stream_Audio;
        case Elements::HEVC : return Stream_Video;
        case Elements::VC_1 : return Stream_Video;
        default :             return Stream_Max;
    }
}

const char* Mpeg_Descriptors_stream_Format(int8u descriptor_tag, int32u format_identifier)
{
    switch (descriptor_tag)
    {
        case 0x02 : return "MPEG Video";
        case 0x03 : return "MPEG Audio";
        case 0x1B : return "MPEG-4 Visual";
        case 0x1C : return "AAC";
        case 0x28 : return "AVC";
        case 0x2B : return "AAC";
        case 0x2D : return "Text";
        default :
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::SCTE : //SCTE
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                        switch (descriptor_tag)
                        {
                            case 0x81 : return "AC-3";
                            default   : return "";
                        }
                case Elements::AC_3 : return "AC-3";
                case Elements::DTS1 : return "DTS";
                case Elements::DTS2 : return "DTS";
                case Elements::DTS3 : return "DTS";
                case Elements::KLVA : return "KLV";
                case Elements::HEVC : return "HEVC";
                case Elements::VC_1 : return "VC-1";
                case Elements::drac : return "Dirac";
                default                     :
                        switch (descriptor_tag)
                        {
                            case 0x56 : return "Teletext";
                            case 0x59 : return "DVB Subtitle";
                            case 0x6A : return "AC-3";
                            case 0x7A : return "E-AC-3";
                            case 0x7B : return "DTS";
                            case 0x7C : return "AAC";
                            case 0x81 : return "AC-3";
                            default   : return "";
                        }
            }
    }
}

const char* Mpeg_Descriptors_stream_Codec(int8u descriptor_tag, int32u format_identifier)
{
    switch (descriptor_tag)
    {
        case 0x02 : return "MPEG-V";
        case 0x03 : return "MPEG-A";
        case 0x1B : return "MPEG-4V";
        case 0x1C : return "AAC";
        case 0x28 : return "AVC";
        case 0x2B : return "AAC";
        case 0x2D : return "Text";
        default :
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::SCTE : //SCTE
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                        switch (descriptor_tag)
                        {
                            case 0x81 : return "AC3";
                            default   : return "";
                        }
                case Elements::AC_3 : return "AC3";
                case Elements::DTS1 : return "DTS";
                case Elements::DTS2 : return "DTS";
                case Elements::DTS3 : return "DTS";
                case Elements::KLVA : return "KLV";
                case Elements::HEVC : return "HEVC";
                case Elements::VC_1 : return "VC-1";
                case Elements::drac : return "Dirac";
                default                     :
                        switch (descriptor_tag)
                        {
                            case 0x56 : return "Teletext";
                            case 0x59 : return "DVB Subtitle";
                            case 0x6A : return "AC3";
                            case 0x7A : return "AC3+";
                            case 0x7B : return "DTS";
                            case 0x7C : return "AAC";
                            case 0x81 : return "AC3";
                            default   : return "";
                        }
            }
    }
}

stream_t Mpeg_Descriptors_stream_Kind(int8u descriptor_tag, int32u format_identifier)
{
    switch (descriptor_tag)
    {
        case 0x02 : return Stream_Video;
        case 0x03 : return Stream_Audio;
        case 0x1B : return Stream_Video;
        case 0x1C : return Stream_Audio;
        case 0x28 : return Stream_Video;
        case 0x2B : return Stream_Audio;
        case 0x2D : return Stream_Text;
        default :
            switch (format_identifier)
            {
                case Elements::CUEI :
                case Elements::SCTE : //SCTE
                case Elements::GA94 :
                case Elements::S14A : //ATSC
                        switch (descriptor_tag)
                        {
                            case 0x81 : return Stream_Audio;
                            default   : return Stream_Max;
                        }
                case Elements::AC_3 : return Stream_Audio;
                case Elements::DTS1 : return Stream_Audio;
                case Elements::DTS2 : return Stream_Audio;
                case Elements::DTS3 : return Stream_Audio;
                case Elements::HEVC : return Stream_Video;
                case Elements::VC_1 : return Stream_Video;
                case Elements::drac : return Stream_Video;
                default                     :
                        switch (descriptor_tag)
                        {
                            case 0x56 : return Stream_Text;
                            case 0x59 : return Stream_Text;
                            case 0x6A : return Stream_Audio;
                            case 0x7A : return Stream_Audio;
                            case 0x7B : return Stream_Audio;
                            case 0x7C : return Stream_Audio;
                            case 0x81 : return Stream_Audio;
                            default   : return Stream_Max;
                        }
            }
    }
}

const char* Mpeg_Descriptors_MPEG_4_audio_profile_and_level(int8u MPEG_4_audio_profile_and_level)
{
    switch (MPEG_4_audio_profile_and_level)
    {
        case 0x10 : return "Main@L1";
        case 0x11 : return "Main@L2";
        case 0x12 : return "Main@L3";
        case 0x13 : return "Main@L4";
        case 0x18 : return "Scalable@L1";
        case 0x19 : return "Scalable@L2";
        case 0x1A : return "Scalable@L3";
        case 0x1B : return "Scalable@L4";
        case 0x20 : return "Speech@L1";
        case 0x21 : return "Speech@L2";
        case 0x28 : return "Synthesis@L1";
        case 0x29 : return "Synthesis@L2";
        case 0x2A : return "Synthesis@L3";
        case 0x30 : return "High quality audio@L1";
        case 0x31 : return "High quality audio@L2";
        case 0x32 : return "High quality audio@L3";
        case 0x33 : return "High quality audio@L4";
        case 0x34 : return "High quality audio@L5";
        case 0x35 : return "High quality audio@L6";
        case 0x36 : return "High quality audio@L7";
        case 0x37 : return "High quality audio@L8";
        case 0x38 : return "Low delay audio@L1";
        case 0x39 : return "Low delay audio@L2";
        case 0x3A : return "Low delay audio@L3";
        case 0x3B : return "Low delay audio@L4";
        case 0x3C : return "Low delay audio@L5";
        case 0x3D : return "Low delay audio@L6";
        case 0x3E : return "Low delay audio@L7";
        case 0x3F : return "Low delay audio@L8";
        case 0x40 : return "Natural audio@L1";
        case 0x41 : return "Natural audio@L2";
        case 0x42 : return "Natural audio@L3";
        case 0x43 : return "Natural audio@L4";
        case 0x48 : return "Mobile audio internetworking@L1";
        case 0x49 : return "Mobile audio internetworking@L2";
        case 0x4A : return "Mobile audio internetworking@L3";
        case 0x4B : return "Mobile audio internetworking@L4";
        case 0x4C : return "Mobile audio internetworking@L5";
        case 0x4D : return "Mobile audio internetworking@L6";
        case 0x50 : return "LC@L1";
        case 0x51 : return "LC@L2";
        case 0x52 : return "LC@L3";
        case 0x53 : return "LC@L4";
        case 0x58 : return "HE-AAC@L2 / LC@L2";
        case 0x59 : return "HE-AAC@L3 / LC@L3";
        case 0x5A : return "HE-AAC@L4 / LC@L4";
        case 0x5B : return "HE-AAC@L5 / LC@L5";
        case 0x60 : return "HE-AACv2@L2 / HE-AAC@L2 / LC@L2";
        case 0x61 : return "HE-AACv2@L3 / HE-AAC@L3 / LC@L3";
        case 0x62 : return "HE-AACv2@L4 / HE-AAC@L4 / LC@L4";
        case 0x63 : return "HE-AACv2@L5 / HE-AAC@L5 / LC@L5";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
extern const float32 Mpegv_frame_rate[]; //In Video/File_Mpegv.cpp
extern const char*  Mpegv_Colorimetry_format[]; //In Video/File_Mpegv.cpp
extern const char*  Mpegv_profile_and_level_indication_profile[]; //In Video/File_Mpegv.cpp
extern const char*  Mpegv_profile_and_level_indication_level[]; //In Video/File_Mpegv.cpp

//---------------------------------------------------------------------------
extern const char*  Mpega_Version[]; //In Audio/File_Mpega.cpp
extern const char*  Mpega_Layer[]; //In Audio/File_Mpega.cpp
extern const char*  Mpega_Format_Profile_Version[]; //In Audio/File_Mpega.cpp
extern const char*  Mpega_Format_Profile_Layer[]; //In Audio/File_Mpega.cpp

//---------------------------------------------------------------------------
extern const int32u AC3_SamplingRate[]; //In Audio/File_Ac3.cpp
extern const int16u AC3_BitRate[]; //In Audio/File_Ac3.cpp
extern const char*  AC3_ChannelPositions[]; //In Audio/File_Ac3.cpp
extern const int8u  AC3_Channels[]; //In Audio/File_Ac3.cpp
extern const char*  AC3_Mode[]; //In Audio/File_Ac3.cpp
extern const char*  AC3_Surround[]; //In Audio/File_Ac3.cpp

const char* Mpeg_Descriptors_AC3_Channels[]=
{
    "1",
    "2",
    "2",
    "2",
    "3+",
    "6+",
    "",
    "",
};

const char* Mpeg_Descriptors_AC3_Priority[]=
{
    "",
    "Primary Audio",
    "Other Audio",
    "",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_bandwidth[]=
{
    "8 MHz",
    "7 MHz",
    "6 MHz",
    "5 MHz",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_constellation[]=
{
    "QPSK",
    "16-QAM",
    "64-QAM",
    "",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_hierarchy_information[]=
{
    "non-hierarchical, native interleaver",
    "1, native interleaver",
    "2, native interleaver",
    "4, native interleaver",
    "non-hierarchical, in-depth interleaver",
    "1, in-depth interleaver",
    "2, in-depth interleaver",
    "4, in-depth interleaver",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_code_rate[]=
{
    "1/2",
    "2/3",
    "3/4",
    "5/6",
    "7/8",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_guard_interval[]=
{
    "1/32",
    "1/16",
    "1/8",
    "1/4",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_transmission_mode[]=
{
    "2k mode",
    "8k mode",
    "4k mode",
    "",
};

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_original_network_id(int16u original_network_id)
{
    switch (original_network_id)
    {
        case 0x0001 : return "Astra Satellite Network 19,2'E";
        case 0x0002 : return "Astra Satellite Network 28,2'E";
        case 0x0003 : return "Astra 1";
        case 0x0004 : return "Astra 2";
        case 0x0005 : return "Astra 3";
        case 0x0006 : return "Astra 4";
        case 0x0007 : return "Astra 5";
        case 0x0008 : return "Astra 6";
        case 0x0009 : return "Astra 7";
        case 0x000A : return "Astra 8";
        case 0x000B : return "Astra 9";
        case 0x000C : return "Astra 10";
        case 0x000D : return "Astra 11";
        case 0x000E : return "Astra 12";
        case 0x000F : return "Astra 13";
        case 0x0010 : return "Astra 14";
        case 0x0011 : return "Astra 15";
        case 0x0012 : return "Astra 16";
        case 0x0013 : return "Astra 17";
        case 0x0014 : return "Astra 18";
        case 0x0015 : return "Astra 19";
        case 0x0016 : return "Astra 20";
        case 0x0017 : return "Astra 21";
        case 0x0018 : return "Astra 22";
        case 0x0019 : return "Astra 23";
        case 0x0020 : return "ASTRA";
        case 0x0021 : return "Hispasat Network 1";
        case 0x0022 : return "Hispasat Network 2";
        case 0x0023 : return "Hispasat Network 3";
        case 0x0024 : return "Hispasat Network 4";
        case 0x0025 : return "Hispasat Network 5";
        case 0x0026 : return "Hispasat Network 6";
        case 0x0027 : return "Hispasat 30'W (FSS)";
        case 0x0028 : return "Hispasat 30'W (DBS)";
        case 0x0029 : return "Hispasat 30'W (America)";
        case 0x0030 : return "Canal+ Satellite Network";
        case 0x0031 : return "Hispasat – VIA DIGITAL";
        case 0x0032 : return "Hispasat Network 7";
        case 0x0033 : return "Hispasat Network 8";
        case 0x0034 : return "Hispasat Network 9";
        case 0x0035 : return "Nethold Main Mux System";
        case 0x0037 : return "STENTOR";
        case 0x0040 : return "HPT – Croatian Post and Telecommunications";
        case 0x0041 : return "Mindport";
        case 0x0046 : return "1 degree W (Telenor)";
        case 0x0047 : return "1 degree W (Telenor)";
        case 0x0050 : return "HRT – Croatian Radio and Television";
        case 0x0051 : return "Havas";
        case 0x0052 : return "Osaka Yusen Satellite";
        case 0x0055 : return "Sirius Satellite System";
        case 0x0058 : return "Thiacom 1 & 2 co-located 78.5'E (UBC Thailand)";
        case 0x005E : return "Sirius Satellite System (Nordic Coverage)";
        case 0x005F : return "Sirius Satellite System (FSS)";
        case 0x0060 : return "Deutsche Telekom";
        case 0x0069 : return "Optus B3 156'E";
        case 0x0070 : return "BONUM1 36 Degrees East (NTV+)";
        case 0x007E : return "Eutelsat Satellite System at 7'E";
        case 0x0073 : return "PanAmSat 4 68.5'E";
        case 0x0085 : return "BetaTechnik";
        case 0x0090 : return "TDF";
        case 0x00A0 : return "News Datacom";
        case 0x00A1 : return "News Datacom";
        case 0x00A2 : return "News Datacom";
        case 0x00A3 : return "News Datacom";
        case 0x00A4 : return "News Datacom";
        case 0x00A5 : return "News Datacom";
        case 0x00A6 : return "ART";
        case 0x00A7 : return "Globecast";
        case 0x00A8 : return "Foxtel";
        case 0x00A9 : return "Sky New Zealand";
        case 0x00B0 : return "TPS";
        case 0x00B1 : return "TPS";
        case 0x00B2 : return "TPS";
        case 0x00B3 : return "TPS";
        case 0x00B4 : return "Telesat 107.3'W";
        case 0x00B5 : return "Telesat 111.1'W";
        case 0x00BA : return "Satellite Express – 6 (80'E)";
        case 0x00C0 : return "Canal+";
        case 0x00C1 : return "Canal+";
        case 0x00C2 : return "Canal+";
        case 0x00C3 : return "Canal+";
        case 0x00C4 : return "Canal+";
        case 0x00C5 : return "Canal+";
        case 0x00C6 : return "Canal+";
        case 0x00C7 : return "Canal+";
        case 0x00C8 : return "Canal+";
        case 0x00C9 : return "Canal+";
        case 0x00CA : return "Canal+";
        case 0x00CB : return "Canal+";
        case 0x00CC : return "Canal+";
        case 0x00CD : return "Canal+";
        case 0x0100 : return "ExpressVu Express";
        case 0x010E : return "Eutelsat Satellite System at 10'E";
        case 0x0110 : return "Mediaset";
        case 0x013E : return "Eutelsat Satellite System at 13'E";
        case 0x016E : return "Eutelsat Satellite System at 16'E";
        case 0x029E : return "Eutelsat Satellite System at 29'E";
        case 0x02BE : return "Arabsat Arabsat (Scientific Atlanta, Eutelsat)";
        case 0x036E : return "Eutelsat Satellite System at 36'E";
        case 0x03E8 : return "Telia";
        case 0x048E : return "Eutelsat Satellite System at 48'E";
        case 0x0800 : return "Nilesat 101";
        case 0x0801 : return "Nilesat 101";
        case 0x0880 : return "MEASAT 1, 91.5'E";
        case 0x0882 : return "MEASAT 2, 91.5'E";
        case 0x0883 : return "MEASAT 2, 148.0'E";
        case 0x088F : return "MEASAT 3";
        case 0x1000 : return "Optus B3 156'E Optus Communications";
        case 0x1001 : return "DISH Network Echostar Communications";
        case 0x1002 : return "Dish Network 61.5 W Echostar Communications";
        case 0x1003 : return "Dish Network 83 W Echostar Communications";
        case 0x1004 : return "Dish Network 119 W Echostar Communications";
        case 0x1005 : return "Dish Network 121 W Echostar Communications";
        case 0x1006 : return "Dish Network 148 W Echostar Communications";
        case 0x1007 : return "Dish Network 175 W Echostar Communications";
        case 0x1008 : return "Dish Network W Echostar Communications";
        case 0x1009 : return "Dish Network X Echostar Communications";
        case 0x100A : return "Dish Network Y Echostar Communications";
        case 0x100B : return "Dish Network Z Echostar Communications";
        case 0x2000 : return "Thiacom 1 & 2 co-located 78.5'E";
        case 0x22D4 : return "Spanish Digital Terrestrial Television";
        case 0x22F1 : return "Swedish Digital Terrestrial Television";
        case 0x233A : return "UK Digital Terrestrial Television";
        case 0x2024 : return "Australian Digital Terrestrial Television";
        case 0x2114 : return "German Digital Terrestrial Television";
        case 0x3000 : return "PanAmSat 4 68.5'E";
        case 0x5000 : return "Irdeto Mux System";
        case 0xF000 : return "Small Cable networks";
        case 0xF001 : return "Deutsche Telekom";
        case 0xF010 : return "Telefonica Cable";
        case 0xF020 : return "Cable and Wireless Communication";
        case 0xFBFC : return "MATAV";
        case 0xFBFD : return "Telia Kabel-TV";
        case 0xFBFE : return "TPS";
        case 0xFBFF : return "Stream";
        case 0xFC00 : return "France Telecom Cable";
        case 0xFC10 : return "Rhone Vision Cable";
        case 0xFD00 : return "Lyonnaise Communications";
        case 0xFE00 : return "TeleDenmark Cable TV";
        default     : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg_Descriptors_CA_system_ID(int16u CA_system_ID)
{
    switch (CA_system_ID)
    {
        case 0x0100 : return "Seca Mediaguard 1/2";
        case 0x0101 : return "RusCrypto";
        case 0x0464 : return "EuroDec";
        case 0x0500 : return "TPS-Crypt  or Viaccess";
        case 0x0602 :
        case 0x0604 :
        case 0x0606 :
        case 0x0608 :
        case 0x0622 :
        case 0x0626 : return "Irdeto";
        case 0x0700 : return "DigiCipher 2";
        case 0x0911 :
        case 0x0919 :
        case 0x0960 :
        case 0x0961 : return "NDS Videoguard 1/2";
        case 0x0B00 : return "Conax CAS 5 /7";
        case 0x0D00 :
        case 0x0D02 :
        case 0x0D03 :
        case 0x0D05 :
        case 0x0D07 :
        case 0x0D20 : return "Cryptoworks";
        case 0x0E00 : return "PowerVu";
        case 0x1000 : return "RAS (Remote Authorisation System)";
        case 0x1702 :
        case 0x1722 :
        case 0x1762 : return "BetaCrypt 1 or Nagravision";
        case 0x1710 : return "BetaCrypt 2";
        case 0x1800 :
        case 0x1801 :
        case 0x1810 :
        case 0x1830 : return "Nagravision";
        case 0x22F0 : return "Codicrypt";
        case 0x2600 : return "BISS";
        case 0x4800 : return "Accessgate";
        case 0x4900 : return "China Crypt";
        case 0x4A10 : return "EasyCas";
        case 0x4A20 : return "AlphaCrypt";
        case 0x4A60 :
        case 0x4A61 :
        case 0x4A63 : return "SkyCrypt or Neotioncrypt or Neotion SHL";
        case 0x4A70 : return "DreamCrypt";
        case 0x4A80 : return "ThalesCrypt";
        case 0x4AA1 : return "KeyFly";
        case 0x4ABF : return "DG-Crypt";
        case 0x4AD0 :
        case 0x4AD1 : return "X-Crypt";
        case 0x4AD4 : return "OmniCrypt";
        case 0x4AE0 : return "RossCrypt";
        case 0x4B13 : return "PlayReady";
        case 0x5500 : return "Z-Crypt or DRE-Crypt";
        case 0x5501 : return "Griffin";
        default     : return "Encrypted";
    }
}

//---------------------------------------------------------------------------
bool Mpeg_Descriptors_CA_system_ID_MustSkipSlices(int16u CA_system_ID)
{
    switch (CA_system_ID)
    {
        case 0x4B13 : // PlayReady
                      return true;
        default     : return false; //We try, it is not sure
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mpeg_Descriptors::File_Mpeg_Descriptors()
{
    //In
    Complete_Stream=NULL;
    transport_stream_id=0x0000;
    pid=0x0000;
    table_id=0x00;
    table_id_extension=0x0000;
    elementary_PID=0x0000;
    program_number=0x0000;
    stream_type=0x00;
    event_id=0x0000;
    elementary_PID_IsValid=false;
    program_number_IsValid=false;
    stream_type_IsValid=false;
    event_id_IsValid=false;

    //Out
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::FileHeader_Parse()
{
    Accept();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Header_Parse()
{
    int8u descriptor_tag=0, descriptor_length=0;
    Get_B1 (descriptor_tag,                                     "descriptor_tag");
    Get_B1 (descriptor_length,                                  "descriptor_length");

    //Size
    if (Element_Size)
        Header_Fill_Size(Element_Size);
    if (Element_Offset)
        Header_Fill_Size(Element_Offset);
    if (descriptor_length)
        Header_Fill_Size(descriptor_length);

    if (Element_Size<Element_Offset+descriptor_length)
    {
        Element_WaitForMoreData();
        return;
    }

    //Filling
    Header_Fill_Code(descriptor_tag, Ztring().From_Number(descriptor_tag, 16));
    Header_Fill_Size(2+descriptor_length);
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Data_Parse()
{
    #define ELEMENT_CASE(_NAME, _DETAIL) \
        case 0x##_NAME : Element_Name(_DETAIL); Descriptor_##_NAME(); break;

    //Parsing
         if (table_id> 0x00 && table_id<0x40)
    {
        switch (Element_Code)
        {
            ELEMENT_CASE(00, "Reserved");
            ELEMENT_CASE(01, "Reserved");
            ELEMENT_CASE(02, "video_stream");
            ELEMENT_CASE(03, "audio_stream");
            ELEMENT_CASE(04, "hierarchy");
            ELEMENT_CASE(05, "registration");
            ELEMENT_CASE(06, "data_stream_alignment");
            ELEMENT_CASE(07, "target_background_grid");
            ELEMENT_CASE(08, "Video_window");
            ELEMENT_CASE(09, "CA");
            ELEMENT_CASE(0A, "ISO_639_language");
            ELEMENT_CASE(0B, "System_clock");
            ELEMENT_CASE(0C, "Multiplex_buffer_utilization");
            ELEMENT_CASE(0D, "Copyright");
            ELEMENT_CASE(0E, "Maximum_bitrate");
            ELEMENT_CASE(0F, "Private_data_indicator");
            ELEMENT_CASE(10, "Smoothing_buffer");
            ELEMENT_CASE(11, "STD");
            ELEMENT_CASE(12, "IBP");
            ELEMENT_CASE(13, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(14, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(15, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(16, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(17, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(18, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(19, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(1A, "Defined in ISO/IEC 13818-6");
            ELEMENT_CASE(1B, "MPEG-4_video");
            ELEMENT_CASE(1C, "MPEG-4_audio");
            ELEMENT_CASE(1D, "IOD");
            ELEMENT_CASE(1E, "SL");
            ELEMENT_CASE(1F, "FMC");
            ELEMENT_CASE(20, "External_ES_ID");
            ELEMENT_CASE(21, "MuxCode");
            ELEMENT_CASE(22, "FmxBufferSize");
            ELEMENT_CASE(23, "multiplexbuffer");
            ELEMENT_CASE(24, "content_labeling");
            ELEMENT_CASE(25, "metadata_pointer");
            ELEMENT_CASE(26, "metadata");
            ELEMENT_CASE(27, "metadata_STD");
            ELEMENT_CASE(28, "AVC video");
            ELEMENT_CASE(29, "IPMP"); //ISO-IEC 13818-11
            ELEMENT_CASE(2A, "AVC timing and HRD");
            ELEMENT_CASE(2B, "MPEG-2 AAC audio");
            ELEMENT_CASE(2C, "FlexMux_Timing");
            ELEMENT_CASE(2D, "MPEG-4_text");
            ELEMENT_CASE(2E, "MPEG-4_audio_extension");
            ELEMENT_CASE(2F, "Auxiliary_video_data");
            ELEMENT_CASE(30, "SVC extension");
            ELEMENT_CASE(31, "MVC extension");
            ELEMENT_CASE(32, "J2K video");
            ELEMENT_CASE(33, "MVC operation point");
            ELEMENT_CASE(34, "MPEG2_stereoscopic_video_format");
            ELEMENT_CASE(35, "Stereoscopic_program_info");
            ELEMENT_CASE(36, "Stereoscopic_video_info");
            ELEMENT_CASE(37, "ODUpdate");
            ELEMENT_CASE(38, "Transport_profile");
            ELEMENT_CASE(39, "HEVC video");
            ELEMENT_CASE(3A, "HEVC timing and HRD");
            ELEMENT_CASE(3F, "Extension");

            //Following is in private sections, in case there is not network type detected
            ELEMENT_CASE(40, "DVB - network_name_descriptor");
            ELEMENT_CASE(41, "DVB - service_list_descriptor");
            ELEMENT_CASE(42, "DVB - stuffing_descriptor");
            ELEMENT_CASE(43, "DVB - satellite_delivery_system_descriptor");
            ELEMENT_CASE(44, "DVB - cable_delivery_system_descriptor");
            ELEMENT_CASE(45, "DVB - VBI_data_descriptor");
            ELEMENT_CASE(46, "DVB - VBI_teletext_descriptor");
            ELEMENT_CASE(47, "DVB - bouquet_name_descriptor");
            ELEMENT_CASE(48, "DVB - service_descriptor");
            ELEMENT_CASE(49, "DVB - country_availability_descriptor");
            ELEMENT_CASE(4A, "DVB - linkage_descriptor");
            ELEMENT_CASE(4B, "DVB - NVOD_reference_descriptor");
            ELEMENT_CASE(4C, "DVB - time_shifted_service_descriptor");
            ELEMENT_CASE(4D, "DVB - short_event_descriptor");
            ELEMENT_CASE(4E, "DVB - extended_event_descriptor");
            ELEMENT_CASE(4F, "DVB - time_shifted_event_descriptor");
            ELEMENT_CASE(50, "DVB - component_descriptor");
            ELEMENT_CASE(51, "DVB - mosaic_descriptor");
            ELEMENT_CASE(52, "DVB - stream_identifier_descriptor");
            ELEMENT_CASE(53, "DVB - CA_identifier_descriptor");
            ELEMENT_CASE(54, "DVB - content_descriptor");
            ELEMENT_CASE(55, "DVB - parental_rating_descriptor");
            ELEMENT_CASE(56, "DVB - teletext_descriptor");
            ELEMENT_CASE(57, "DVB - telephone_descriptor");
            ELEMENT_CASE(58, "DVB - local_time_offset_descriptor");
            ELEMENT_CASE(59, "DVB - subtitling_descriptor");
            ELEMENT_CASE(5A, "DVB - terrestrial_delivery_system_descriptor");
            ELEMENT_CASE(5B, "DVB - multilingual_network_name_descriptor");
            ELEMENT_CASE(5C, "DVB - multilingual_bouquet_name_descriptor");
            ELEMENT_CASE(5D, "DVB - multilingual_service_name_descriptor");
            ELEMENT_CASE(5E, "DVB - multilingual_component_descriptor");
            ELEMENT_CASE(5F, "DVB - private_data_specifier_descriptor");
            ELEMENT_CASE(60, "DVB - service_move_descriptor");
            ELEMENT_CASE(61, "DVB - short_smoothing_buffer_descriptor");
            ELEMENT_CASE(62, "DVB - frequency_list_descriptor");
            ELEMENT_CASE(63, "DVB - partial_transport_stream_descriptor");
            ELEMENT_CASE(64, "DVB - data_broadcast_descriptor");
            ELEMENT_CASE(65, "DVB - scrambling_descriptor");
            ELEMENT_CASE(66, "DVB - data_broadcast_id_descriptor");
            ELEMENT_CASE(67, "DVB - transport_stream_descriptor");
            ELEMENT_CASE(68, "DVB - DSNG_descriptor");
            ELEMENT_CASE(69, "DVB - PDC_descriptor");
            ELEMENT_CASE(6A, "DVB - AC-3_descriptor");
            ELEMENT_CASE(6B, "DVB - ancillary_data_descriptor");
            ELEMENT_CASE(6C, "DVB - cell_list_descriptor");
            ELEMENT_CASE(6D, "DVB - cell_frequency_link_descriptor");
            ELEMENT_CASE(6E, "DVB - announcement_support_descriptor");
            ELEMENT_CASE(6F, "DVB - application_signalling_descriptor");
            ELEMENT_CASE(70, "DVB - adaptation_field_data_descriptor");
            ELEMENT_CASE(71, "DVB - service_identifier_descriptor");
            ELEMENT_CASE(72, "DVB - service_availability_descriptor");
            ELEMENT_CASE(73, "DVB - default_authority_descriptor");
            ELEMENT_CASE(74, "DVB - related_content_descriptor");
            ELEMENT_CASE(75, "DVB - TVA_id_descriptor");
            ELEMENT_CASE(76, "DVB - content_identifier_descriptor");
            ELEMENT_CASE(77, "DVB - time_slice_fec_identifier_descriptor");
            ELEMENT_CASE(78, "DVB - ECM_repetition_rate_descriptor");
            ELEMENT_CASE(79, "DVB - S2_satellite_delivery_system_descriptor");
            ELEMENT_CASE(7A, "DVB - enhanced_AC-3_descriptor");
            ELEMENT_CASE(7B, "DVB - DTS descriptor");
            ELEMENT_CASE(7C, "DVB - AAC descriptor");
            ELEMENT_CASE(7D, "DVB - reserved for future use");
            ELEMENT_CASE(7E, "DVB - reserved for future use");
            ELEMENT_CASE(7F, "DVB - extension descriptor");
            ELEMENT_CASE(80, "ATSC - stuffing");
            ELEMENT_CASE(81, "ATSC - AC-3 audio");
            ELEMENT_CASE(86, "ATSC - caption service");
            ELEMENT_CASE(87, "ATSC - content advisory");
            ELEMENT_CASE(A0, "ATSC - extended channel name");
            ELEMENT_CASE(A1, "ATSC - service location");
            ELEMENT_CASE(A2, "ATSC - time-shifted service");
            ELEMENT_CASE(A3, "ATSC - component name");
            ELEMENT_CASE(A8, "ATSC - DCC Departing Request");
            ELEMENT_CASE(A9, "ATSC - DCC Arriving Request");
            ELEMENT_CASE(AA, "ATSC - Redistribution Control");
            ELEMENT_CASE(AB, "ATSC - DCC Location Code");
            ELEMENT_CASE(C1, "ARIB - Digital Copy Control");
            ELEMENT_CASE(C4, "SMPTE - ANC"); //SMPTE ST 2038
            ELEMENT_CASE(C8, "ARIB - Video Decode Control");
            ELEMENT_CASE(DE, "ARIB - Content Availability");
            ELEMENT_CASE(E9, "CableLabs - Encoder Boundary Point");
            ELEMENT_CASE(FC, "ARIB - Emergency Information");
            ELEMENT_CASE(FD, "ARIB - Data Component");

            default: if (Element_Code>=0x40)
                        Element_Info1("user private");
                     else
                        Element_Info1("unknown");
                     Skip_XX(Element_Size,                          "Data");
                     break;
        }
    }
    else if (table_id>=0x40 && table_id<0x80)
    {
        switch (Element_Code)
        {
            ELEMENT_CASE(40, "DVB - network_name_descriptor");
            ELEMENT_CASE(41, "DVB - service_list_descriptor");
            ELEMENT_CASE(42, "DVB - stuffing_descriptor");
            ELEMENT_CASE(43, "DVB - satellite_delivery_system_descriptor");
            ELEMENT_CASE(44, "DVB - cable_delivery_system_descriptor");
            ELEMENT_CASE(45, "DVB - VBI_data_descriptor");
            ELEMENT_CASE(46, "DVB - VBI_teletext_descriptor");
            ELEMENT_CASE(47, "DVB - bouquet_name_descriptor");
            ELEMENT_CASE(48, "DVB - service_descriptor");
            ELEMENT_CASE(49, "DVB - country_availability_descriptor");
            ELEMENT_CASE(4A, "DVB - linkage_descriptor");
            ELEMENT_CASE(4B, "DVB - NVOD_reference_descriptor");
            ELEMENT_CASE(4C, "DVB - time_shifted_service_descriptor");
            ELEMENT_CASE(4D, "DVB - short_event_descriptor");
            ELEMENT_CASE(4E, "DVB - extended_event_descriptor");
            ELEMENT_CASE(4F, "DVB - time_shifted_event_descriptor");
            ELEMENT_CASE(50, "DVB - component_descriptor");
            ELEMENT_CASE(51, "DVB - mosaic_descriptor");
            ELEMENT_CASE(52, "DVB - stream_identifier_descriptor");
            ELEMENT_CASE(53, "DVB - CA_identifier_descriptor");
            ELEMENT_CASE(54, "DVB - content_descriptor");
            ELEMENT_CASE(55, "DVB - parental_rating_descriptor");
            ELEMENT_CASE(56, "DVB - teletext_descriptor");
            ELEMENT_CASE(57, "DVB - telephone_descriptor");
            ELEMENT_CASE(58, "DVB - local_time_offset_descriptor");
            ELEMENT_CASE(59, "DVB - subtitling_descriptor");
            ELEMENT_CASE(5A, "DVB - terrestrial_delivery_system_descriptor");
            ELEMENT_CASE(5B, "DVB - multilingual_network_name_descriptor");
            ELEMENT_CASE(5C, "DVB - multilingual_bouquet_name_descriptor");
            ELEMENT_CASE(5D, "DVB - multilingual_service_name_descriptor");
            ELEMENT_CASE(5E, "DVB - multilingual_component_descriptor");
            ELEMENT_CASE(5F, "DVB - private_data_specifier_descriptor");
            ELEMENT_CASE(60, "DVB - service_move_descriptor");
            ELEMENT_CASE(61, "DVB - short_smoothing_buffer_descriptor");
            ELEMENT_CASE(62, "DVB - frequency_list_descriptor");
            ELEMENT_CASE(63, "DVB - partial_transport_stream_descriptor");
            ELEMENT_CASE(64, "DVB - data_broadcast_descriptor");
            ELEMENT_CASE(65, "DVB - scrambling_descriptor");
            ELEMENT_CASE(66, "DVB - data_broadcast_id_descriptor");
            ELEMENT_CASE(67, "DVB - transport_stream_descriptor");
            ELEMENT_CASE(68, "DVB - DSNG_descriptor");
            ELEMENT_CASE(69, "DVB - PDC_descriptor");
            ELEMENT_CASE(6A, "DVB - AC-3_descriptor");
            ELEMENT_CASE(6B, "DVB - ancillary_data_descriptor");
            ELEMENT_CASE(6C, "DVB - cell_list_descriptor");
            ELEMENT_CASE(6D, "DVB - cell_frequency_link_descriptor");
            ELEMENT_CASE(6E, "DVB - announcement_support_descriptor");
            ELEMENT_CASE(6F, "DVB - application_signalling_descriptor");
            ELEMENT_CASE(70, "DVB - adaptation_field_data_descriptor");
            ELEMENT_CASE(71, "DVB - service_identifier_descriptor");
            ELEMENT_CASE(72, "DVB - service_availability_descriptor");
            ELEMENT_CASE(73, "DVB - default_authority_descriptor");
            ELEMENT_CASE(74, "DVB - related_content_descriptor");
            ELEMENT_CASE(75, "DVB - TVA_id_descriptor");
            ELEMENT_CASE(76, "DVB - content_identifier_descriptor");
            ELEMENT_CASE(77, "DVB - time_slice_fec_identifier_descriptor");
            ELEMENT_CASE(78, "DVB - ECM_repetition_rate_descriptor");
            ELEMENT_CASE(79, "DVB - S2_satellite_delivery_system_descriptor");
            ELEMENT_CASE(7A, "DVB - enhanced_AC-3_descriptor");
            ELEMENT_CASE(7B, "DVB - DTS descriptor");
            ELEMENT_CASE(7C, "DVB - AAC descriptor");
            ELEMENT_CASE(7D, "DVB - reserved for future use");
            ELEMENT_CASE(7E, "DVB - reserved for future use");
            ELEMENT_CASE(7F, "DVB - extension descriptor");
            default: if (Element_Code>=0x40)
                        Element_Info1("user private");
                     else
                        Element_Info1("unknown");
                     Skip_XX(Element_Size,                          "Data");
                     break;
        }
    }
    else if ((table_id>=0xC0 && table_id<0xE0))
    {
        switch (Element_Code)
        {
            ELEMENT_CASE(80, "ATSC - stuffing");
            ELEMENT_CASE(81, "ATSC - AC-3 audio");
            ELEMENT_CASE(86, "ATSC - caption service");
            ELEMENT_CASE(87, "ATSC - content advisory");
            ELEMENT_CASE(A0, "ATSC - extended channel name");
            ELEMENT_CASE(A1, "ATSC - service location");
            ELEMENT_CASE(A2, "ATSC - time-shifted service");
            ELEMENT_CASE(A3, "ATSC - component name");
            ELEMENT_CASE(A8, "ATSC - DCC Departing Request");
            ELEMENT_CASE(A9, "ATSC - DCC Arriving Request");
            ELEMENT_CASE(AA, "ATSC - Redistribution Control");
            ELEMENT_CASE(AB, "ATSC - DCC Location Code");
            default: if (Element_Code>=0x40)
                        Element_Info1("user private");
                     else
                        Element_Info1("unknown");
                     Skip_XX(Element_Size,                          "Data");
                     break;
        }
    }
    else if (table_id==0xFC)
    {
        //SCTE 35
        #undef ELEMENT_CASE
        #define ELEMENT_CASE(_NAME, _DETAIL) \
            case 0x##_NAME : Element_Name(_DETAIL); CUEI_##_NAME(); break;
        switch (Element_Code)
        {
            ELEMENT_CASE(00, "SCTE35 - avail_descriptor");
            ELEMENT_CASE(01, "SCTE35 - DTMF_descriptor");
            ELEMENT_CASE(02, "SCTE35 - segmentation_descriptor");
            default: Element_Info1("SCTE35 - Reserved");
                     Skip_XX(Element_Size,                      "Data");
                     break;
        }
    }
    else
    {
        if (Element_Code>=0x40)
        Element_Info1("user private");
        else
        Element_Info1("unknown");
        Skip_XX(Element_Size,                                   "Data");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_02()
{
    //Parsing
    int8u frame_rate_code;
    bool multiple_frame_rate_flag, MPEG_1_only_flag;
    int8u profile_and_level_indication_profile=4, profile_and_level_indication_level=10, chroma_format=1;
    bool frame_rate_extension_flag=false;
    BS_Begin();
    Get_SB (   multiple_frame_rate_flag,                        "multiple_frame_rate_flag");
    Get_S1 (4, frame_rate_code,                                 "frame_rate_code"); Param_Info1(Mpegv_frame_rate[frame_rate_code]);
    Get_SB (   MPEG_1_only_flag,                                "MPEG_1_only_flag");
    Skip_SB(                                                    "constrained_parameter_flag");
    Skip_SB(                                                    "still_picture_flag");
    if (MPEG_1_only_flag==0)
    {
        Skip_SB(                                                "profile_and_level_indication_escape");
        Get_S1 (3, profile_and_level_indication_profile,        "profile_and_level_indication_profile"); Param_Info1(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile]);
        Get_S1 (4, profile_and_level_indication_level,          "profile_and_level_indication_level"); Param_Info1(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
        Get_S1 (2, chroma_format,                               "chroma_format"); Param_Info1(Mpegv_Colorimetry_format[chroma_format]);
        Get_SB (   frame_rate_extension_flag,                   "frame_rate_extension_flag");
        Skip_S1(5,                                              "reserved");
    }
    BS_End();

    //Filling
    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            if (!multiple_frame_rate_flag && !frame_rate_extension_flag && frame_rate_code)
                                Complete_Stream->Streams[elementary_PID]->Infos["FrameRate"]=Ztring::ToZtring(Mpegv_frame_rate[frame_rate_code]);
                            Complete_Stream->Streams[elementary_PID]->Infos["Format_Version"]=MPEG_1_only_flag?__T("Version 1"):__T("Version 2");
                            Complete_Stream->Streams[elementary_PID]->Infos["Colorimetry"]=Mpegv_Colorimetry_format[chroma_format];
                            if (profile_and_level_indication_profile)
                            {
                                Complete_Stream->Streams[elementary_PID]->Infos["Format_Profile"]=Ztring().From_Local(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile])+__T("@")+Ztring().From_Local(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
                                Complete_Stream->Streams[elementary_PID]->Infos["Codec_Profile"]=Ztring().From_Local(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile])+__T("@")+Ztring().From_Local(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
                            }
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_03()
{
    //Parsing
    int8u ID, layer;
    bool variable_rate_audio_indicator;
    BS_Begin();
    Skip_SB(                                                    "free_format_flag");
    Get_S1 (1, ID,                                              "ID"); Param_Info1(Mpega_Version[2+ID]); //Mpega_Version is with MPEG2.5 hack
    Get_S1 (2, layer,                                           "layer");  Param_Info1(Mpega_Layer[layer]);
    Get_SB (   variable_rate_audio_indicator,                   "variable_rate_audio_indicator");
    Skip_S1(3,                                                  "reserved");
    BS_End();

    FILLING_BEGIN();
        if (elementary_PID_IsValid)
        {
            Complete_Stream->Streams[elementary_PID]->Infos["BitRate_Mode"]=variable_rate_audio_indicator?__T("VBR"):__T("CBR");
            Complete_Stream->Streams[elementary_PID]->Infos["Codec"]=Ztring(Mpega_Version[ID])+Ztring(Mpega_Layer[layer]);
            Complete_Stream->Streams[elementary_PID]->Infos["Format"]=__T("MPEG Audio");
            Complete_Stream->Streams[elementary_PID]->Infos["Format_Version"]=Mpega_Format_Profile_Version[ID];
            Complete_Stream->Streams[elementary_PID]->Infos["Format_Profile"]=Mpega_Format_Profile_Layer[layer];
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_05()
{
    //Parsing
    int32u format_identifier;
    Get_B4 (format_identifier,                                  "format_identifier"); Element_Info1(Mpeg_Descriptors_registration_format_identifier_Format(format_identifier)); Param_Info1(Mpeg_Descriptors_registration_format_identifier_Format(format_identifier));
    if (Element_Size-Element_Offset>0)
        Skip_XX(Element_Size-Element_Offset,                    "additional_identification_info");

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        switch (elementary_PID_IsValid)
                        {
                            case false : //Per program
                                        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].registration_format_identifier=format_identifier;
                                        break;
                            case true : //Per PES
                                        Complete_Stream->Streams[elementary_PID]->registration_format_identifier=format_identifier;
                                        Complete_Stream->Streams[elementary_PID]->Infos["format_identifier"]=Ztring().From_CC4(format_identifier);
                                        if (Complete_Stream->Streams[elementary_PID]->Infos["format_identifier"].size()!=4)
                                        {
                                            Ztring Temp; Temp.From_Number(format_identifier, 16);
                                            if (Temp.size()<8)
                                                Temp.insert(0, 8-Temp.size(), __T('0'));
                                            Complete_Stream->Streams[elementary_PID]->Infos["format_identifier"]=__T("0x")+Temp;
                                        }
                                        Complete_Stream->Streams[elementary_PID]->Infos_Option["format_identifier"]=__T("N NT");
                                        if (format_identifier==Elements::KLVA)
                                        {
                                            Complete_Stream->Streams[elementary_PID]->Infos["Format"]=__T("KLV");
                                            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].HasNotDisplayableStreams=true;
                                        }
                                        //Coherency
                                        if (stream_type==0x81 && Complete_Stream->Streams[elementary_PID]->registration_format_identifier==Elements::BSSD)
                                            Complete_Stream->Streams[elementary_PID]->registration_format_identifier=0x00000000; //Reseting it, this combinaision is not possible but a stream has it
                                        break;
                        }
                        break;
            default    : ;
        }
    FILLING_ELSE()
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        switch (elementary_PID_IsValid)
                        {
                            case false : //Per program
                                        break;
                            case true : //Per PES
                                        Complete_Stream->Streams[elementary_PID]->Infos["format_identifier"]=__T("(INVALID)");
                                        Complete_Stream->Streams[elementary_PID]->Infos_Option["format_identifier"]=__T("N NT");
                                        break;
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_06()
{
    //Parsing
    Info_B1(alignment_type,                                     "alignment_type"); Param_Info1(Mpeg_Descriptors_alignment_type(alignment_type));
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_07()
{
    //Parsing
    BS_Begin();
    Skip_S1(14,                                                 "horizontal_size");
    Skip_S1(14,                                                 "vertical_size");
    Skip_S1( 4,                                                 "aspect_ratio_information"); //Same as ISO/IEC 13818-2
    BS_End();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_08()
{
    //Parsing
    BS_Begin();
    Skip_S1(14,                                                 "horizontal_offset");
    Skip_S1(14,                                                 "vertical_offset");
    Skip_S1( 4,                                                 "window_priority");
    BS_End();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_09()
{
    //Parsing
    int16u CA_system_ID, CA_PID;
    Get_B2 (CA_system_ID,                                       "CA_system_ID"); Param_Info1(Mpeg_Descriptors_CA_system_ID(CA_system_ID));
    BS_Begin();
    Skip_S1( 3,                                                 "reserved");
    Get_S2 (13, CA_PID,                                         "CA_PID");
    BS_End();
    if (Element_Size-Element_Offset>0)
        Skip_XX(Element_Size-Element_Offset,                    "private_data_byte");

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x01 : //conditional_access_section
                    if (Complete_Stream->Streams[CA_PID]->Kind==complete_stream::stream::unknown) //Priority to PES, if this is a PES, we skip the CA
                        {
                            Complete_Stream->Streams[CA_PID]->Kind=complete_stream::stream::psi;
                            Complete_Stream->Streams[CA_PID]->Table_IDs.resize(0x100);
                            #ifdef MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                                Complete_Stream->Streams[CA_PID]->Searching_Payload_Start_Set(true);
                            #endif
                        }
                        break;
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->CA_system_ID=CA_system_ID;
                            Complete_Stream->Streams[elementary_PID]->CA_system_ID_MustSkipSlices=Mpeg_Descriptors_CA_system_ID_MustSkipSlices(CA_system_ID);
                            if (CA_PID<Complete_Stream->Streams.size() && Complete_Stream->Streams[CA_PID]->Kind==complete_stream::stream::unknown) //Priority to PES, if this is a PES, we skip the CA
                            {
                                Complete_Stream->Streams[CA_PID]->Kind=complete_stream::stream::psi;
                                Complete_Stream->Streams[CA_PID]->Table_IDs.resize(0x100);
                                #ifdef MEDIAINFO_MPEGTS_ALLSTREAMS_YES
                                    Complete_Stream->Streams[CA_PID]->Searching_Payload_Start_Set(true);
                                #endif
                            }
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_0A()
{
    //Parsing
    int32u ISO_639_language_code;
    int8u audio_type;
    Get_C3 (ISO_639_language_code,                              "ISO_639_language_code");
    Get_B1 (audio_type,                                         "audio_type"); Param_Info1(Mpeg_Descriptors_audio_type(audio_type));

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Ztring ISO_639_2;
                            if (ISO_639_language_code)
                                ISO_639_2.From_CC3(ISO_639_language_code);
                            const Ztring& ISO_639_1=MediaInfoLib::Config.Iso639_1_Get(ISO_639_2);
                            Complete_Stream->Streams[elementary_PID]->Infos["Language"]=ISO_639_1.empty()?ISO_639_2:ISO_639_1;
                            if (audio_type)
                                Complete_Stream->Streams[elementary_PID]->Infos["Language_More"]=Mpeg_Descriptors_audio_type(audio_type);
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_0B()
{
    //Parsing
    int8u clock_accuracy_integer, clock_accuracy_exponent;
    BS_Begin();
    Skip_SB(                                                    "external_clock_reference_indicator");
    Skip_SB(                                                    "reserved");
    Get_S1 (6, clock_accuracy_integer,                          "clock_accuracy_integer");
    Get_S1 (3, clock_accuracy_exponent,                         "clock_accuracy_exponent"); Param_Info1(Ztring::ToZtring(clock_accuracy_integer*(int64u)pow(10.0, clock_accuracy_exponent)));
    Skip_S1(5,                                                  "reserved");
    BS_End();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_0D()
{
    //Parsing
    int32u copyright_identifier;
    Get_B4 (copyright_identifier,                             "copyright_identifier");
    if ((copyright_identifier&0xFF000000)>=0x61000000 && (copyright_identifier&0xFF000000)<=0x7A000000
     && (copyright_identifier&0x00FF0000)>=0x00610000 && (copyright_identifier&0x00FF0000)<=0x007A0000
     && (copyright_identifier&0x0000FF00)>=0x00006100 && (copyright_identifier&0x0000FF00)<=0x00007A00
     && (copyright_identifier&0x000000FF)>=0x00000061 && (copyright_identifier&0x000000FF)<=0x0000007A)
    {
        Param_Info1(Ztring().From_CC4(copyright_identifier));
        Element_Info1(Ztring().From_CC4(copyright_identifier));
    }
    if (copyright_identifier==Elements::MANZ)
    {
        if (Element_Offset<Element_Size)
            Skip_Local(Element_Size-Element_Offset,             "Info");
        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Infos["Encoded_Library"]=__T("Manzanita Systems");
    }

    if (Element_Offset<Element_Size)
        Skip_Local(Element_Size-Element_Offset,                 "Info");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_0E()
{
    //Parsing
    int32u maximum_bitrate;
    BS_Begin();
    Skip_S1( 2,                                                 "reserved");
    Get_S3 (22, maximum_bitrate,                                "maximum_bitrate"); Param_Info2(maximum_bitrate*400, " bps");
    BS_End();

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                            Complete_Stream->Streams[elementary_PID]->Infos["BitRate_Maximum"]=Ztring::ToZtring(maximum_bitrate*400);
                        else
                            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Programs[table_id_extension].Infos["BitRate_Maximum"]=Ztring::ToZtring(maximum_bitrate*400);
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_0F()
{
    //Parsing
    int32u private_data_indicator;
    Get_B4 (private_data_indicator,                             "private_data_indicator");
    if ((private_data_indicator&0xFF000000)>=0x41000000 && (private_data_indicator&0xFF000000)<=0x7A000000
     && (private_data_indicator&0x00FF0000)>=0x00410000 && (private_data_indicator&0x00FF0000)<=0x007A0000
     && (private_data_indicator&0x0000FF00)>=0x00004100 && (private_data_indicator&0x0000FF00)<=0x00007A00
     && (private_data_indicator&0x000000FF)>=0x00000041 && (private_data_indicator&0x000000FF)<=0x0000007A)
    {
        Param_Info1(Ztring().From_CC4(private_data_indicator));
        Element_Info1(Ztring().From_CC4(private_data_indicator));
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_10()
{
    //Parsing
    BS_Begin();
    Skip_S1( 2,                                                 "reserved");
    Info_S3(22, sb_leak_rate,                                   "sb_leak_rate"); Param_Info2(sb_leak_rate*400, " bps");
    Skip_S1( 2,                                                 "reserved");
    Info_S3(22, sb_size,                                        "sb_size"); Param_Info2(sb_size, " bytes");
    BS_End();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_11()
{
    //Parsing
    BS_Begin();
    Skip_S1( 7,                                                 "reserved");
    Skip_SB(                                                    "leak_valid_flag");
    BS_End();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_1C()
{
    //Parsing
    int8u Profile_and_level;
    Get_B1 (   Profile_and_level,                               "Profile_and_level"); Param_Info1(Mpeg_Descriptors_MPEG_4_audio_profile_and_level(Profile_and_level));

    FILLING_BEGIN();
        Complete_Stream->Streams[elementary_PID]->Infos["Format_Profile"]=Mpeg_Descriptors_MPEG_4_audio_profile_and_level(Profile_and_level);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_1D()
{
    //Parsing
    int8u IOD_label;
    Skip_B1(                                                    "Scope_of_IOD_label");
    Get_B1 (IOD_label,                                          "IOD_label");

    #ifdef MEDIAINFO_MPEG4_YES
        if (Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.find(IOD_label)==Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs.end())
        {
            File_Mpeg4_Descriptors MI;
            MI.Parser_DoNotFreeIt=true;
            MI.SLConfig_DoNotFreeIt=true;
            Open_Buffer_Init(&MI);
            Open_Buffer_Continue(&MI);
            Finish(&MI);
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs[MI.ES_ID].Parser=MI.Parser;
            Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].IOD_ESs[MI.ES_ID].SLConfig=MI.SLConfig;
        }
    #else
        Skip_XX(Element_Size-Element_Offset,                    "MPEG-4 Descriptor");
    #endif
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_1F()
{
    //Parsing
    int16u ES_ID;
    while (Element_Offset<Element_Size)
    {
        Element_Begin1("FlexMux");
        Get_B2 (ES_ID,                                          "ES_ID");
        if (Element_Offset!=Element_Size)
            Skip_B1(                                            "FlexMuxChannel");
        Element_End0();

        FILLING_BEGIN();
            switch (table_id)
            {
                case 0x02 : //program_map_section
                            if (elementary_PID_IsValid)
                            {
                                Complete_Stream->Streams[elementary_PID]->FMC_ES_ID=ES_ID;
                                Complete_Stream->Streams[elementary_PID]->FMC_ES_ID_IsValid=true;
                            }
                            break;
                default    : ;
            }
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_28()
{
    //Parsing
    int8u profile_idc, level_idc;
    Get_B1 (profile_idc,                                        "profile_idc"); Param_Info1(Avc_profile_idc(profile_idc));
    BS_Begin();
    Element_Begin1("constraints");
        Skip_SB(                                                "constraint_set0_flag");
        Skip_SB(                                                "constraint_set1_flag");
        Skip_SB(                                                "constraint_set2_flag");
        Skip_SB(                                                "constraint_set3_flag");
        Skip_SB(                                                "reserved_zero_4bits");
        Skip_SB(                                                "reserved_zero_4bits");
        Skip_SB(                                                "reserved_zero_4bits");
        Skip_SB(                                                "reserved_zero_4bits");
    Element_End0();
    BS_End();
    Get_B1 (level_idc,                                          "level_idc");
    BS_Begin();
    Skip_SB(                                                    "AVC_still_present");
    Skip_SB(                                                    "AVC_24_hour_picture_flag");
    Skip_S1(6,                                                  "reserved");
    BS_End();

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->Infos["Format"]=__T("AVC");
                            Complete_Stream->Streams[elementary_PID]->Infos["Format_Profile"]=Ztring().From_Local(Avc_profile_idc(profile_idc))+__T("@L")+Ztring().From_Number(((float)level_idc)/10, 1);
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_2A()
{
    //Parsing
    BS_Begin();
    Skip_SB(                                                "hrd_management_valid_flag");
    Skip_S1(6,                                              "reserved");
    TEST_SB_SKIP(                                           "picture_and_timing_info_present");
        bool x90kHz_flag;
        Get_SB (x90kHz_flag,                                "90kHz_flag");
        Skip_S1(7,                                          "reserved");
        BS_End();
        if (x90kHz_flag)
        {
            Skip_B4(                                        "N");
            Skip_B4(                                        "K");
        }
        Skip_B4(                                            "num_units_in_tick");
        BS_Begin();
    TEST_SB_END();
    Skip_SB(                                                "fixed_frame_rate_flag");
    Skip_SB(                                                "temporal_poc_flag");
    Skip_SB(                                                "picture_to_display_conversion_flag");
    Skip_S1(5,                                              "reserved");
    BS_End();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_2F()
{
    //Parsing
    int8u aux_video_params_length;
    Skip_B1(                                                    "aux_video_type"); //ISO/IEC 23002-3
    Get_B1 (aux_video_params_length,                            "aux_video_params_length");
    Skip_XX(aux_video_params_length,                            "aux_video_params");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_40()
{
    //Parsing
    Ztring network_name;
    Get_DVB_Text(Element_Size, network_name,                    "network_name");

    FILLING_BEGIN();
        Complete_Stream->network_name=network_name;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_41()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        Element_Begin1("service");
        int16u service_id;
        int8u service_type;
        Get_B2 (service_id,                                     "service_id"); Element_Info1(Ztring::ToZtring(service_id, 16));
        Get_B1 (service_type,                                   "service_type"); Param_Info1(Mpeg_Descriptors_dvb_service_type(service_type));
        Element_End1(Ztring::ToZtring(service_id));

        FILLING_BEGIN();
            Complete_Stream->Transport_Streams[table_id_extension].Programs[service_id].Infos["ServiceType"]=Mpeg_Descriptors_dvb_service_type(service_type);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_43()
{
    //Parsing
    int32u frequency, symbol_rate;
    int16u orbital_position;
    int8u polarization, roll_off, modulation_type, FEC_inner;
    bool west_east_flag, modulation_system;
    Get_B4 (frequency,                                          "frequency"); Param_Info1(Frequency_DVB__BCD(frequency));
    Get_B2 (orbital_position,                                   "orbital_position"); Param_Info1(OrbitalPosition_DVB__BCD(orbital_position));
    BS_Begin();
    Get_SB (    west_east_flag,                                 "west_east_flag"); Param_Info1(west_east_flag?"E":"W");
    Get_S1 ( 2, polarization,                                   "polarization");
    Get_S1 ( 2, roll_off,                                       "roll_off");
    Get_SB (    modulation_system,                              "modulation_system");
    Get_S1 ( 2, modulation_type,                                "modulation_type");
    Get_S4 (28, symbol_rate,                                    "symbol_rate");
    Get_S1 ( 4, FEC_inner,                                      "FEC_inner");
    BS_End();

    FILLING_BEGIN();
        Complete_Stream->Transport_Streams[transport_stream_id].Infos["Frequency"]=Frequency_DVB__BCD(frequency);
        Complete_Stream->Transport_Streams[transport_stream_id].Infos["OrbitalPosition"]=OrbitalPosition_DVB__BCD(orbital_position)+(west_east_flag?__T('E'):__T('W'));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_48()
{
    //Parsing
    Ztring service_provider_name, service_name;
    int8u service_type, service_provider_name_length, service_name_length;
    Get_B1 (service_type,                                       "service_type"); Param_Info1(Mpeg_Descriptors_dvb_service_type(service_type));
    Get_B1 (service_provider_name_length,                       "service_provider_name_length");
    Get_DVB_Text(service_provider_name_length, service_provider_name, "service_provider_name");
    Get_B1 (service_name_length,                                "service_name_length");
    Get_DVB_Text(service_name_length, service_name,             "service_name");

    //Filling
    FILLING_BEGIN();
        if (program_number_IsValid)
        {
            Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceName"]=service_name;
            Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceProvider"]=service_provider_name;
            Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceType"]=Mpeg_Descriptors_dvb_service_type(service_type);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_4A()
{
    //Parsing
    int8u linkage_type;
    Skip_B2(                                                    "transport_stream_id");
    Info_B2(original_network_id,                                "original_network_id"); Param_Info1(Mpeg_Descriptors_original_network_id(original_network_id));
    Skip_B2(                                                    "service_id");
    Get_B1 (linkage_type,                                       "linkage_type"); Param_Info1(Mpeg_Descriptors_linkage_type(linkage_type));
    if (Element_Size>7)
        Skip_XX(Element_Size-7,                                 "private_data");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_4D()
{
    //Parsing
    Ztring event_name, text;
    int32u ISO_639_language_code;
    int8u event_name_length, text_length;
    Get_C3 (ISO_639_language_code,                              "ISO_639_language_code");
    Get_B1 (event_name_length,                                  "event_name_length");
    Get_DVB_Text(event_name_length, event_name,                 "event_name"); Element_Info1(event_name);
    Get_B1 (text_length,                                        "text_length");
    Get_DVB_Text(text_length, text,                             "text");

    FILLING_BEGIN();
        if (table_id>=0x4E && table_id<=0x6F) //event_information_section
        {
            if (event_id_IsValid)
            {
                Ztring ISO_639_2; ISO_639_2.From_CC3(ISO_639_language_code);
                const Ztring& ISO_639_1=MediaInfoLib::Config.Iso639_1_Get(ISO_639_2);
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].short_event.event_name=(ISO_639_1.empty()?ISO_639_2:ISO_639_1)+__T(':')+event_name;
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].short_event.text=(ISO_639_1.empty()?ISO_639_2:ISO_639_1)+__T(':')+text;
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks_IsUpdated=true;
                Complete_Stream->Programs_IsUpdated=true;
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_50()
{
    //Parsing
    int32u ISO_639_language_code;
    int8u stream_content;
    BS_Begin();
    Skip_S1(4,                                                  "reserved_future_use");
    Get_S1 (4, stream_content,                                  "stream_content"); Param_Info1(Mpeg_Descriptors_stream_content(stream_content)); Element_Info1(Mpeg_Descriptors_stream_content(stream_content));
    BS_End();
    Info_B1(component_type,                                     "component_type"); Param_Info1(Mpeg_Descriptors_component_type(stream_content, component_type)); Element_Info1(Mpeg_Descriptors_component_type(stream_content, component_type));
    Info_B1(component_tag,                                      "component_tag");
    Get_C3 (ISO_639_language_code,                              "ISO_639_language_code");
    Skip_DVB_Text(Element_Size-Element_Offset,                  "text");

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Ztring ISO_639_2; ISO_639_2.From_CC3(ISO_639_language_code);
                            const Ztring& ISO_639_1=MediaInfoLib::Config.Iso639_1_Get(ISO_639_2);
                            Complete_Stream->Streams[elementary_PID]->Infos["Language"]=ISO_639_1.empty()?ISO_639_2:ISO_639_1;
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_52()
{
    //Parsing
    Skip_B1(                                                    "component_tag");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_54()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        BS_Begin();
        int8u content_nibble_level_1, content_nibble_level_2;
        Get_S1 (4, content_nibble_level_1,                      "content_nibble_level_1"); Param_Info1(Mpeg_Descriptors_content_nibble_level_1(content_nibble_level_1)); Element_Info1(Mpeg_Descriptors_content_nibble_level_1(content_nibble_level_1));
        Get_S1 (4, content_nibble_level_2,                      "content_nibble_level_2"); Param_Info1(Mpeg_Descriptors_content_nibble_level_2(content_nibble_level_1, content_nibble_level_2)); Element_Info1C((content_nibble_level_1==0xB || content_nibble_level_2!=0), Mpeg_Descriptors_content_nibble_level_2(content_nibble_level_1, content_nibble_level_2));
        Skip_S1(4,                                              "user_nibble");
        Skip_S1(4,                                              "user_nibble");
        BS_End();

        FILLING_BEGIN();
            if (event_id_IsValid)
            {
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].content=Ztring().From_UTF8(Mpeg_Descriptors_content_nibble_level_2(content_nibble_level_1, content_nibble_level_2))+__T(", ");
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks_IsUpdated=true;
                Complete_Stream->Programs_IsUpdated=true;
            }
        FILLING_END();
    }

    FILLING_BEGIN();
        if (event_id_IsValid)
        {
            if (!Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].content.empty())
            {
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].content.resize(Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks[table_id].Events[event_id].content.size()-2);
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[table_id_extension].DVB_EPG_Blocks_IsUpdated=true;
                Complete_Stream->Programs_IsUpdated=true;
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_55()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        Skip_Local(3,                                           "country_code");
        Info_B1(rating,                                         "rating"); Param_Info2(rating+3, " years old"); Element_Info2(rating+3, " years old");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_56()
{
    //Parsing
    Ztring Languages;
    while (Element_Offset<Element_Size)
    {
        Element_Begin1("teletext");
        Ztring ISO_639_language_code;
        int8u teletext_type;
        int8u teletext_magazine_number;
        int8u teletext_page_number_1;
        int8u teletext_page_number_2;
        Get_Local(3, ISO_639_language_code,                     "ISO_639_language_code");
        BS_Begin();
        Get_S1 (5, teletext_type,                               "teletext_type"); Param_Info1(Mpeg_Descriptors_teletext_type(teletext_type));
        Get_S1 (3, teletext_magazine_number,                    "teletext_magazine_number");
        Get_S1 (4, teletext_page_number_1,                      "teletext_page_number_1");
        Get_S1 (4, teletext_page_number_2,                      "teletext_page_number_2");
        BS_End();

        FILLING_BEGIN();
            switch (table_id)
            {
                case 0x02 : //program_map_section
                            if (elementary_PID_IsValid /*&& (teletext_type==2 || teletext_type==5)*/) //Subtitles are the only supported format
                            {
                                int16u ID=(teletext_magazine_number==0?8:teletext_magazine_number)*100+teletext_page_number_1*10+teletext_page_number_2;
                                Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x56;
                                Complete_Stream->Streams[elementary_PID]->Teletexts[ID].Infos["Language"]=MediaInfoLib::Config.Iso639_1_Get(ISO_639_language_code);
                                Complete_Stream->Streams[elementary_PID]->Teletexts[ID].Infos["Format"]=Mpeg_Descriptors_teletext_type(teletext_type);
                                Complete_Stream->Streams[elementary_PID]->Teletexts[ID].Infos["Codec"]=Mpeg_Descriptors_teletext_type(teletext_type);
                            }
                            break;
                default    : ;
            }
        FILLING_END();

        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_58()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        int32u country_code;
        int16u local_time_offset;
        int8u country_region_id;
        bool local_time_offset_polarity;
        Get_C3 (country_code,                                       "country_code");
        BS_Begin();
        Get_S1 (6, country_region_id,                               "country_region_id");
        Skip_SB(                                                    "reserved");
        Get_SB (local_time_offset_polarity,                         "local_time_offset_polarity"); Param_Info1(local_time_offset_polarity?"-":"+");
        BS_End();
        Get_B2 (local_time_offset,                                  "local_time_offset"); Param_Info1(TimeHHMM_BCD(local_time_offset));
        Info_B2(date,                                               "time_of_change (date)"); Param_Info1(Date_MJD(date));
        Info_B3(time,                                               "time_of_change (time)"); Param_Info1(Time_BCD(time));
        Info_B2(next_time_offset,                                   "next_time_offset"); Param_Info1(TimeHHMM_BCD(next_time_offset));

        FILLING_BEGIN();
            Ztring Country; Country.From_CC3(country_code);
            if (country_region_id)
                Country+=__T(" (")+Ztring::ToZtring(country_region_id)+__T(")");
            Complete_Stream->TimeZones[Country]=(local_time_offset_polarity?__T('-'):__T('+'))+TimeHHMM_BCD(local_time_offset);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_59()
{
    //Parsing
    Ztring Languages;
    while (Element_Offset<Element_Size)
    {
        Element_Begin1("subtitle");
        int32u ISO_639_language_code;
        Get_C3 (ISO_639_language_code,                              "ISO_639_language_code");
        Info_B1(subtitling_type,                                    "subtitling_type"); Param_Info1(Mpeg_Descriptors_component_type_O3(subtitling_type));
        Skip_B2(                                                    "composition_page_id");
        Skip_B2(                                                    "ancillary_page_id");

        FILLING_BEGIN();
            switch (table_id)
            {
                case 0x02 : //program_map_section
                            if (elementary_PID_IsValid)
                            {
                                Ztring ISO_639_2; ISO_639_2.From_CC3(ISO_639_language_code);
                                const Ztring& ISO_639_1=MediaInfoLib::Config.Iso639_1_Get(ISO_639_2);
                                Languages+=(ISO_639_1.empty()?ISO_639_2:ISO_639_1)+__T(" / ");
                                //TODO: this stream is teletext. Be careful, multiple stream in a pid
                            }
                            break;
                default    : ;
            }
        FILLING_END();

        Element_End0();
    }

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->StreamKind_FromDescriptor=Stream_Text;
                            Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x59;
                            if (!Languages.empty())
                                Languages.resize(Languages.size()-3);
                            Complete_Stream->Streams[elementary_PID]->Infos["Language"]=Languages;
                            Complete_Stream->Streams[elementary_PID]->Infos["Format"]=__T("DVB Subtitle");
                            Complete_Stream->Streams[elementary_PID]->Infos["Codec"]=__T("DVB Subtitle");
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_5A()
{
    //Parsing
    Info_B4(centre_frequency,                                   "centre_frequency"); Param_Info2(((int64u)centre_frequency)*10, " Hz");
    BS_Begin();
    Info_S1(3, bandwidth,                                       "bandwidth"); Param_Info1(Mpeg_Descriptors_bandwidth[bandwidth]);
    Info_SB(   priority,                                        "priority"); Param_Info1(priority?"HP":"LP");
    Skip_SB(                                                    "Time_Slicing_indicator");
    Skip_SB(                                                    "MPE-FEC_indicator");
    Skip_S1(2,                                                  "reserved");
    Info_S1(2, constellation,                                   "constellation"); Param_Info1(Mpeg_Descriptors_constellation[constellation]);
    Info_S1(3, hierarchy_information,                           "hierarchy_information"); Param_Info1(Mpeg_Descriptors_hierarchy_information[hierarchy_information]);
    Info_S1(3, code_rate_HP,                                    "code_rate-HP_stream"); Param_Info1(Mpeg_Descriptors_code_rate[code_rate_HP]);
    Info_S1(3, code_rate_LP,                                    "code_rate-LP_stream"); Param_Info1(Mpeg_Descriptors_code_rate[code_rate_LP]);
    Info_S1(2, guard_interval,                                  "guard_interval"); Param_Info1(Mpeg_Descriptors_guard_interval[guard_interval]);
    Info_S1(2, transmission_mode,                               "transmission_mode"); Param_Info1(Mpeg_Descriptors_transmission_mode[transmission_mode]);
    Skip_SB(                                                    "other_frequency_flag");
    BS_End();
    Skip_B4(                                                    "reserved");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_5D()
{
    //Parsing
    Ztring ServiceProvider, ServiceName;
    while (Element_Offset<Element_Size)
    {
        Ztring service_provider_name, service_name;
        int32u ISO_639_language_code;
        int8u  service_provider_name_length, service_name_length;
        Get_C3 (ISO_639_language_code,                          "ISO_639_language_code");
        Get_B1 (service_provider_name_length,                   "service_provider_name_length");
        Get_DVB_Text(service_provider_name_length, service_provider_name, "service_provider_name");
        Get_B1 (service_name_length,                            "service_name_length");
        Get_DVB_Text(service_name_length, service_name,         "service_name");

        //Filling
        FILLING_BEGIN();
            Ztring ISO_639_2=Ztring().From_CC3(ISO_639_language_code);
            const Ztring& ISO_639_1=MediaInfoLib::Config.Iso639_1_Get(ISO_639_2);
            ServiceProvider+=(ISO_639_1.empty()?ISO_639_2:ISO_639_1)+__T(':')+service_provider_name+__T( " - ");
            ServiceName+=(ISO_639_1.empty()?ISO_639_2:ISO_639_1)+__T(':')+service_name+__T( " - ");
        FILLING_END();
    }

    if (!ServiceProvider.empty())
    {
        ServiceProvider.resize(ServiceProvider.size()-3);
        if (program_number_IsValid)
            Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceProvider"]=ServiceProvider;
    }
    if (!ServiceName.empty())
    {
        ServiceName.resize(ServiceName.size()-3);
        if (program_number_IsValid)
            Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceName"]=ServiceName;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_5F()
{
    //Parsing
    Info_B4(private_data_specifier,                             "private_data_specifier"); Param_Info1(Ztring().From_CC4(private_data_specifier));
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_63()
{
    //Parsing
    int32u peak_rate;
    BS_Begin();
    Skip_S1( 2,                                                 "DVB_reserved_future_use");
    Get_S3 (22, peak_rate,                                      "peak_rate");
    Skip_S1( 2,                                                 "DVB_reserved_future_use");
    Skip_S3(22,                                                 "minimum_overall_smoothing_rate");
    Skip_S1( 2,                                                 "DVB_reserved_future_use");
    Skip_S2(14,                                                 "maximum_overall_smoothing_buffer");
    BS_End();

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                            Complete_Stream->Streams[elementary_PID]->Infos["OverallBitRate_Maximum"]=Ztring::ToZtring(peak_rate*400);
                        break;
            case 0x7F : //selection_information_section
                        Complete_Stream->Transport_Streams[Complete_Stream->transport_stream_id].Infos["OverallBitRate_Maximum"]=Ztring::ToZtring(peak_rate*400);
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_66()
{
    //Parsing
    Ztring ISO_639_language_code;
    int8u selector_length, text_length;
    Skip_B2(                                                    "data_broadcast_id");
    Skip_B1(                                                    "component_tag");
    Get_B1 (selector_length,                                    "selector_length");
    Skip_XX(selector_length,                                    "selector_bytes");
    Get_Local(3, ISO_639_language_code,                         "ISO_639_language_code");
    Get_B1 (text_length,                                        "text_length");
    Skip_Local(text_length,                                     "text_chars");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_6A()
{
    //Parsing
    BS_Begin();
    bool component_type_flag, bsid_flag, mainid_flag, asvc_flag, enhanced_ac3=false;
    Get_SB (   component_type_flag,                             "component_type_flag");
    Get_SB (   bsid_flag,                                       "bsid_flag");
    Get_SB (   mainid_flag,                                     "mainid_flag");
    Get_SB (   asvc_flag,                                       "asvc_flag");
    Skip_SB(                                                    "reserved_flag");
    Skip_SB(                                                    "reserved_flag");
    Skip_SB(                                                    "reserved_flag");
    Skip_SB(                                                    "reserved_flag");
    BS_End();
    if (component_type_flag)
    {
        int8u service_type, number_of_channels;
        BS_Begin();
        Get_SB (   enhanced_ac3,                                "enhanced AC-3");
        Skip_SB(                                                "full_service");
        Get_S1 (3, service_type,                                "service_type"); Param_Info1(AC3_Mode[service_type]);
        Get_S1 (3, number_of_channels,                          "number_of_channels"); Param_Info2(Mpeg_Descriptors_AC3_Channels[number_of_channels], " channels");
        BS_End();

        FILLING_BEGIN();
            switch (table_id)
            {
                case 0x02 : //program_map_section
                            if (elementary_PID_IsValid)
                            {
                                Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x6A;
                                Complete_Stream->Streams[elementary_PID]->Infos["Channel(s)"]=Ztring().From_Local(Mpeg_Descriptors_AC3_Channels[number_of_channels]);
                            }
                            break;
                default    : ;
            }
        FILLING_END();
    }
    if (bsid_flag)
    {
        BS_Begin();
        Skip_S1(3,                                              "zero");
        Skip_S1(5,                                              "bsid");
        BS_End();
    }
    if (mainid_flag)
    {
        Skip_B1(                                                "mainid");
    }
    if (asvc_flag)
    {
        Skip_B1(                                                "asvc");
    }

    FILLING_BEGIN(); //Can be more
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->StreamKind_FromDescriptor=Stream_Audio;
                            Complete_Stream->Streams[elementary_PID]->Infos["Format"]=enhanced_ac3?__T("E-AC-3"):__T("AC-3");
                            Complete_Stream->Streams[elementary_PID]->Infos["Codec"]=__T("AC3+");
                            if (Complete_Stream->Streams[elementary_PID]->registration_format_identifier==Elements::BSSD)
                                Complete_Stream->Streams[elementary_PID]->registration_format_identifier=0x00000000; //Reseting it, this combinaision is not possible but an stream has it
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_7A()
{
    //Parsing
    bool component_type_flag, bsid_flag, mainid_flag, asvc_flag, mixinfoexists, substream1_flag, substream2_flag, substream3_flag, enhanced_ac3=0;
    BS_Begin();
    Get_SB (  component_type_flag,                             "component_type_flag");
    Get_SB (  bsid_flag,                                       "bsid_flag");
    Get_SB (  mainid_flag,                                     "mainid_flag");
    Get_SB (  asvc_flag,                                       "asvc_flag");
    Get_SB (  mixinfoexists,                                   "mixinfoexists");
    Get_SB (  substream1_flag,                                 "substream1_flag");
    Get_SB (  substream2_flag,                                 "substream2_flag");
    Get_SB (  substream3_flag,                                 "substream3_flag");
    BS_End();
    if (component_type_flag)
    {
        int8u service_type, number_of_channels;
        BS_Begin();
        Get_SB (  enhanced_ac3,                                "enhanced AC-3");
        Skip_SB(                                               "full_service");
        Get_S1 (3, service_type,                               "service_type"); Param_Info1(AC3_Mode[service_type]);
        Get_S1 (3, number_of_channels,                         "number_of_channels"); Param_Info2(Mpeg_Descriptors_AC3_Channels[number_of_channels], " channels");
        FILLING_BEGIN();
            switch (table_id)
            {
                case 0x02 : //program_map_section
                            if (elementary_PID_IsValid)
                            {
                                Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x7A;
                                Complete_Stream->Streams[elementary_PID]->Infos["Channel(s)"]=Ztring().From_Local(Mpeg_Descriptors_AC3_Channels[number_of_channels]);
                            }
                            break;
                default    : ;
            }
        FILLING_END();
        BS_End();
    }
    if (bsid_flag)
    {
        BS_Begin();
        Skip_S1(3,                                              "zero");
        Skip_S1(5,                                              "bsid");
        BS_End();
    }
    if (mainid_flag)
    {
        Skip_B1(                                                "mainid");
    }
    if (asvc_flag)
    {
        Skip_B1(                                                "asvc");
    }
    if (substream1_flag)
    {
        Skip_B1(                                                "substream1");
    }
    if (substream2_flag)
    {
        Skip_B1(                                                "substream2");
    }
    if (substream3_flag)
    {
        Skip_B1(                                                "substream3");
    }

    FILLING_BEGIN(); //Can be more
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->StreamKind_FromDescriptor=Stream_Audio;
                            Complete_Stream->Streams[elementary_PID]->Infos["Format"]=enhanced_ac3?__T("E-AC-3"):__T("AC-3");
                            Complete_Stream->Streams[elementary_PID]->Infos["Codec"]=__T("AC3+");
                        }
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_7B()
{
    //Parsing
    BS_Begin();
    Skip_S1(6,                                                  "bit_rate_code");
    Skip_S2(7,                                                  "nblks");
    Skip_S2(14,                                                 "fsize");
    Skip_S1(6,                                                  "surround_mode");
    Skip_SB(                                                    "lfe_flag");
    Skip_S1(2,                                                  "extended_surround_flag");
    BS_End();
    //BS_End_CANBEMORE();

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x7B;
                        }
            default   : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_7C()
{
    //Parsing
    int8u Profile_and_level;
    bool AAC_type_flag;
    Get_B1 (   Profile_and_level,                               "Profile_and_level"); Param_Info1(Mpeg_Descriptors_MPEG_4_audio_profile_and_level(Profile_and_level));
    BS_Begin();
    Get_SB (   AAC_type_flag,                                   "AAC_type_flag");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    Skip_SB(                                                    "reserved");
    BS_End();
    if (AAC_type_flag)
    {
        Skip_B1(                                                "AAC_type");
    }
    if (Element_Size-Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x7C;
                            Complete_Stream->Streams[elementary_PID]->Infos["Format_Profile"]=Mpeg_Descriptors_MPEG_4_audio_profile_and_level(Profile_and_level);
                        }
            default   : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_81()
{
    //Parsing
    Ztring Text, Language1, Language2;
    int8u sample_rate_code, bit_rate_code, surround_mode, bsmod, num_channels, langcod, textlen, text_code;
    bool language_flag, language_flag_2;
    BS_Begin();
    Get_S1 (3, sample_rate_code,                                "sample_rate_code"); if (sample_rate_code<4) {Param_Info2(AC3_SamplingRate[sample_rate_code], " Hz");}
    Skip_S1(5,                                                  "bsid");
    Get_S1 (6, bit_rate_code,                                   "bit_rate_code"); Param_Info2(AC3_BitRate[bit_rate_code]*1000, " Kbps");
    Get_S1 (2, surround_mode,                                   "surround_mode"); Param_Info1(AC3_Surround[surround_mode]);
    Get_S1 (3, bsmod,                                           "bsmod");
    Get_S1 (4, num_channels,                                    "num_channels"); if (num_channels<8) {Param_Info2(AC3_Channels[num_channels], " channels");}
    Skip_SB(                                                    "full_svc");
    BS_End();

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                        {
                            Complete_Stream->Streams[elementary_PID]->descriptor_tag=0x81;
                            if (sample_rate_code<4)
                                Complete_Stream->Streams[elementary_PID]->Infos["SamplingRate"]=Ztring::ToZtring(AC3_SamplingRate[sample_rate_code]);
                            Complete_Stream->Streams[elementary_PID]->Infos["BitRate"]=Ztring::ToZtring(AC3_BitRate[bit_rate_code]*1000);
                            if (num_channels<8)
                                Complete_Stream->Streams[elementary_PID]->Infos["Channel(s)"]=Ztring::ToZtring(AC3_Channels[num_channels]);
                        }
        }
    FILLING_END();

    //Parsing
    if (Element_Offset==Element_Size) return;
    Get_B1 (langcod,                                            "langcod");

    //Parsing
    if (Element_Offset==Element_Size) return;
    if (num_channels==0) //1+1 mode
        Skip_B1(                                                "langcod2");

    //Parsing
    if (Element_Offset==Element_Size) return;
    if (bsmod<2)
    {
        BS_Begin();
        Skip_S1(3,                                              "mainid");
        Info_BS(2, priority,                                    "priority"); Param_Info1(Mpeg_Descriptors_AC3_Priority[priority]);
        Skip_S1(3,                                              "reserved");
        BS_End();
    }
    else
        Skip_B1(                                                "asvcflags");

    //Parsing
    if (Element_Offset==Element_Size) return;
    BS_Begin();
    Get_S1 (7, textlen,                                         "textlen");
    Get_S1 (1, text_code,                                       "text_code"); Param_Info1C((text_code), "Unicode");
    BS_End();
    if (textlen)
        Get_Local(textlen, Text,                                "text");

    //Parsing
    if (Element_Offset==Element_Size) return;
    BS_Begin();
    Get_SB (   language_flag,                                   "language_flag");
    Get_SB (   language_flag_2,                                 "language_flag_2");
    Skip_S1(6,                                                  "reserved");
    BS_End();

    //Parsing
    if (Element_Offset==Element_Size) return;
    if (language_flag)
        Get_Local(3, Language1,                                 "language1");

    //Parsing
    if (Element_Offset==Element_Size) return;
    if (language_flag_2)
        Get_Local(3, Language2,                                 "language2");

    //Parsing
    if (Element_Offset==Element_Size) return;
    Skip_XX(Element_Size-Element_Offset,                        "additional_info");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_86()
{
    if (event_id_IsValid)
    {
        Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].Eia708_Languages.clear();
        Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].Eia608_IsPresent=false;
    }
    else if (elementary_PID_IsValid)
    {
        Complete_Stream->Streams[elementary_PID]->Eia708_Languages.clear();
        Complete_Stream->Streams[elementary_PID]->Eia608_IsPresent=false;
    }
    else if (program_number_IsValid)
    {
        Complete_Stream->Transport_Streams[transport_stream_id].Programs[program_number].Eia708_Languages.clear();
        Complete_Stream->Transport_Streams[transport_stream_id].Programs[program_number].Eia608_IsPresent=false;
    }

    //Parsing
    Ztring Text, Language1, Language2;
    int8u number_of_services;
    BS_Begin();
    Skip_S1(3,                                                  "reserved");
    Get_S1 (5, number_of_services,                              "number_of_services");
    BS_End();

    for (int8u Pos=0; Pos<number_of_services; Pos++)
    {
        Element_Begin1("service");
        string language;
        int8u caption_service_number;
        bool digital_cc;
        Get_String(3, language,                                 "language");
        BS_Begin();
        Get_SB (digital_cc,                                     "digital_cc");
        Skip_SB(                                                "reserved");
        if (digital_cc) //line21
            Get_S1 (6, caption_service_number,                  "caption_service_number");
        else
        {
            bool line21_field;
            Skip_S1(5,                                          "reserved");
            Get_SB (   line21_field,                            "line21_field");
        }
        Skip_SB(                                                "easy_reader");
        Skip_SB(                                                "wide_aspect_ratio");
        Skip_S2(14,                                             "reserved");
        BS_End();
        Element_End0();

        if (event_id_IsValid)
        {
            if (digital_cc)
            {
                string &Value=Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].Eia708_Languages[caption_service_number];
                if (!Value.empty())
                    Value+=" / ";
                Value+=language;
            }
            else
                Complete_Stream->Sources[table_id_extension].ATSC_EPG_Blocks[Complete_Stream->Streams[pid]->table_type].Events[event_id].Eia608_IsPresent=true;
        }
        else if (elementary_PID_IsValid)
        {
            if (digital_cc)
            {
                string &Value=Complete_Stream->Streams[elementary_PID]->Eia708_Languages[caption_service_number];
                if (!Value.empty())
                    Value+=" / ";
                Value+=language;
            }
            else
                Complete_Stream->Streams[elementary_PID]->Eia608_IsPresent=true;
        }
        else if (program_number_IsValid)
        {
            if (digital_cc)
            {
                string &Value=Complete_Stream->Transport_Streams[transport_stream_id].Programs[program_number].Eia708_Languages[caption_service_number];
                if (!Value.empty())
                    Value+=" / ";
                Value+=language;
            }
            else
                Complete_Stream->Transport_Streams[transport_stream_id].Programs[program_number].Eia608_IsPresent=true;
        }
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_87()
{
    //Parsing
    int8u rating_region_count;
    BS_Begin();
    Skip_S1(2,                                                  "reserved");
    Get_S1 (6, rating_region_count,                             "rating_region_count");
    BS_End();
    for (int8u rating_region_Pos=0; rating_region_Pos<rating_region_count; rating_region_Pos++)
    {
        Element_Begin1("rating_region");
        int8u rated_dimensions;
        Skip_B1(                                                "rating_region");
        Get_B1 (rated_dimensions,                               "rated_dimensions");
        for (int8u rated_dimension_Pos=0; rated_dimension_Pos<rated_dimensions; rated_dimension_Pos++)
        {
            Element_Begin1("rated_dimension");
            Skip_B1(                                            "rating_dimension_j");
            BS_Begin();
            Skip_S1(4,                                          "reserved");
            Skip_S1(4,                                          "rating_value");
            BS_End();
            Element_End0();
        }
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_A0()
{
    //Parsing
    Ztring title;
    ATSC_multiple_string_structure(title,                       "title");

    FILLING_BEGIN(); //Can be more
        switch (table_id)
        {
            case 0xC8 : //TVCT
            case 0xC9 : //CVCT
            case 0xDA : //SVCT
                        if (program_number_IsValid)
                            if (!title.empty())
                                Complete_Stream->Transport_Streams[table_id_extension].Programs[program_number].Infos["ServiceName"]=title;
                        break;
            default    : ;
        }
    FILLING_END();
    FILLING_BEGIN();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_A1()
{
    //Parsing
    int8u number_elements;
    BS_Begin();
    Skip_S1( 3,                                                 "reserved");
    Skip_S2(13,                                                 "PCR_PID");
    BS_End();
    Get_B1 (    number_elements,                                "number_elements");
    for (int8u Pos=0; Pos<number_elements; Pos++)
    {
        Element_Begin0();
        Ztring Language;
        int16u elementary_PID;
        Skip_B1(                                                "stream_type");
        BS_Begin();
        Skip_S1( 3,                                             "reserved");
        Get_S2 (13, elementary_PID,                             "elementary_PID");
        BS_End();
        Get_Local(3, Language,                                  "ISO_639_language_code");
        Element_End1(Ztring().From_CC2(elementary_PID));

        //Filling
        if (elementary_PID<Complete_Stream->Streams.size() && Complete_Stream->Streams[elementary_PID]->Infos["Language"].empty()) //We use only the first detected value
            Complete_Stream->Streams[elementary_PID]->Infos["Language"]=Language;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_A3()
{
    //Parsing
    Ztring Value;
    ATSC_multiple_string_structure(Value,                       "name");

    FILLING_BEGIN();
        switch (table_id)
        {
            case 0x02 : //program_map_section
                        if (elementary_PID_IsValid)
                            if (!Value.empty())
                                Complete_Stream->Streams[elementary_PID]->Infos["Name"]=Value;
                        break;
            default    : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_AA()
{
    //Parsing
    Skip_XX(Element_Size,                                       "rc_information");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_C1()
{
    // ARIB B15

    //Parsing
    int8u copy_control_type;
    bool  maximum_bit_rate_flag, component_control_flag;
    BS_Begin();
    Skip_S1(2,                                                  "digital_recording_control_data");
    Get_SB (   maximum_bit_rate_flag,                           "maximum_bit_rate_flag ");
    Get_SB (   component_control_flag,                          "component_control_flag ");
    Get_S1 (2, copy_control_type,                               "copy_control_type");
    //Skip_S1(2,                                                  (copy_control_type&0x1)?"copy_control_type":"reserved_future_use");
    BS_End();

    while (Element_Offset<Element_Size)
        Skip_B1(                                                "(ToDo)");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_C8()
{
    //Parsing
    Skip_XX(Element_Size,                                       "?");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_DE()
{
    // ARIB B15

    //Parsing
    BS_Begin();
    Skip_SB(                                                    "reserved_future_use");
    Skip_SB(                                                    "copy_restriction_mode");
    Skip_SB(                                                    "image_constraint_token");
    Skip_SB(                                                    "retention_mode");
    Skip_S1(3,                                                  "retention_state");
    Skip_SB(                                                    "encryption_mode");
    BS_End();

    while (Element_Offset<Element_Size)
        Skip_B1(                                                "reserved_future_use");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_E9()
{
    //Parsing
    int64u EBP_distance=(int64u)-1;
    int32u ticks_per_second=1;
    int8u num_partitions, EBP_distance_width_minus_1=0;
    bool timescale_flag;
    if (Element_Size==0)
        return; // It is authorized
    BS_Begin();
    Get_S1 (5, num_partitions,                                  "num_partitions");
    Get_SB (   timescale_flag,                                  "timescale_flag");
    Skip_S1(2,                                                  "reserved");
    if (timescale_flag)
    {
        Get_S3 (21, ticks_per_second,                           "ticks_per_second");
        Get_S1 ( 3, EBP_distance_width_minus_1,                 "EBP_distance_width_minus_1");
    }
    for (int8u i=0; i<num_partitions; ++i)
    {
        Element_Begin1("partition");
        bool EBP_data_explicit_flag, representation_id_flag;
        Get_SB (   EBP_data_explicit_flag,                      "EBP_data_explicit_flag");
        Get_SB (   representation_id_flag,                      "representation_id_flag");
        Skip_S1(5,                                              "partition_id");
        if (EBP_data_explicit_flag)
        {
            bool boundary_flag;
            Get_SB (boundary_flag,                              "boundary_flag");
            if (EBP_distance_width_minus_1<8)
                Get_S8 (8*(EBP_distance_width_minus_1+1), EBP_distance, "EBP_distance");
            else
            {
                //Not supported
                Skip_S1(8,                                      "EBP_distance");
                Skip_S8(64,                                     "EBP_distance");
            }
            if (boundary_flag)
            {
                Skip_S1(3,                                      "SAP_type_max");
                Skip_S1(4,                                      "reserved");
            }
            else
            {
                Skip_S1(7,                                      "reserved");
            }
            Skip_SB(                                            "acquisition_time_flag");
        }
        else
        {
            Skip_SB(                                            "reserved");
            Skip_S2(13,                                         "EBP_PID");
            Skip_S1( 3,                                         "reserved");
        }
        if (representation_id_flag)
        {
            Skip_S8(64,                                         "representation_id");
        }
        Element_End0();

        FILLING_BEGIN();
            Complete_Stream->Streams[elementary_PID]->Infos["EBP_Mode"]=EBP_data_explicit_flag?__T("Explicit"):__T("Implicit");
            if (EBP_distance!=(int64u)-1)
                Complete_Stream->Streams[elementary_PID]->Infos["EBP_Distance"]=ticks_per_second==1?Ztring::ToZtring(EBP_distance):Ztring::ToZtring(((float64)EBP_distance)/ticks_per_second, 3);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_FC()
{
    //Parsing
    Skip_XX(Element_Size,                                       "?");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::Descriptor_FD()
{
    //Parsing
    int16u data_component_id;
    Get_B2 (data_component_id,                                  "data_component_id");

    while (Element_Offset<Element_Size)
        Skip_B1(                                                "?");

    if (data_component_id==0x0008)
    {
        //Is maybe ARIB caption

        FILLING_BEGIN();
            switch (table_id)
            {
                case 0x02 : //program_map_section
                            if (elementary_PID_IsValid)
                            {
                                Complete_Stream->Streams[elementary_PID]->Infos["Format"]=__T("ARIB STD B24/B37");
                                //Complete_Stream->Streams[elementary_PID]->StreamKind=Stream_Text;
                            }
                            break;
                default    : ;
            }
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::CUEI_00()
{
    Skip_C4(                                                    "identifier (\"CUEI\")"); //CUEI
    Skip_B4(                                                    "provider_avail_id");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::CUEI_01()
{
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::CUEI_02()
{
    //Parsing
    int32u segmentation_event_id;
    bool segmentation_event_cancel_indicator;
    Skip_C4(                                                    "identifier (\"CUEI\")"); //CUEI
    Get_B4 (segmentation_event_id,                              "segmentation_event_id");
    BS_Begin();
    Get_SB (    segmentation_event_cancel_indicator,            "segmentation_event_cancel_indicator");
    Skip_S1( 7,                                                 "reserved");
    BS_End();
    if (!segmentation_event_cancel_indicator)
    {
        int8u segmentation_upid_length, segmentation_type_id;
        bool program_segmentation_flag, segmentation_duration_flag;
        BS_Begin();
        Get_SB (    program_segmentation_flag,                  "program_segmentation_flag");
        Get_SB (    segmentation_duration_flag,                 "segmentation_duration_flag");
        Skip_S1( 6,                                             "reserved");
        BS_End();
        if (!program_segmentation_flag)
        {
            int8u component_count;
            Get_B1 (component_count,                            "component_count");
            for (int8u Pos=0; Pos<component_count; Pos++)
            {
                Skip_B1(                                        "component_tag");
                BS_Begin();
                Skip_S1( 7,                                     "reserved");
                Skip_S5(33,                                     "pts_offset");
                BS_End();
            }
        }
        if (segmentation_duration_flag)
        {
            Skip_B5(                                            "segmentation_duration");
        }
        Skip_B1(                                                "segmentation_upid_type");
        Get_B1 (segmentation_upid_length,                       "segmentation_upid_length");
        Skip_XX(segmentation_upid_length,                       "segmentation_upid"); //TODO
        Get_B1 (segmentation_type_id,                           "segmentation_type_id");
        Skip_B1(                                                "segment_num");
        Skip_B1(                                                "segments_expected");

        FILLING_BEGIN();
            for (size_t Program_Pos=0; Program_Pos<Complete_Stream->Streams[pid]->program_numbers.size(); Program_Pos++)
            {
                complete_stream::transport_stream::program::scte35* Scte35=Complete_Stream->Transport_Streams[transport_stream_id].Programs[Complete_Stream->Streams[pid]->program_numbers[Program_Pos]].Scte35;
                if (Scte35)
                {
                    int8u Status=0; //Running
                    switch (segmentation_type_id)
                    {
                        case 0x11 : segmentation_type_id=0x10; Status=1; break; //Program Start/End
                        case 0x12 : segmentation_type_id=0x10; Status=2; break; //Program Start/Early Termination
                        case 0x14 : segmentation_type_id=0x13; Status=1; break; //Program Breakaway/Resumption
                        case 0x21 : segmentation_type_id=0x20; Status=1; break; //Chapter Start/End
                        case 0x31 : segmentation_type_id=0x30; Status=1; break; //Provider Advertisement Start/End
                        case 0x33 : segmentation_type_id=0x32; Status=1; break; //Distributor Advertisement Start/End
                        case 0x41 : segmentation_type_id=0x40; Status=1; break; //Unscheduled Event Start/End
                        default   : ;
                    }

                    Scte35->Segmentations[segmentation_event_id].Segments[segmentation_type_id].Status=Status;
                }
            }
        FILLING_END();
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg_Descriptors::ATSC_multiple_string_structure(Ztring &Value, const char* Name)
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
void File_Mpeg_Descriptors::Get_DVB_Text(int64u Size, Ztring &Value, const char* Info)
{
    if (Size<1)
    {
        Get_Local(Size, Value,                                  Info);
        return;
    }

    //Testing if there is a codepage
    int8u CodePage1;
    Peek_B1(CodePage1);
    if (CodePage1<0x20)
    {
        Skip_B1(                                                "CodePage"); Param_Info1(Mpeg_Descriptors_codepage_1(CodePage1));
        if (CodePage1!=0x10)
        {
            Get_Local(Size-1, Value,                            Info);
        }
        else
        {
            if (Size<3)
            {
                Value.clear();
                return;
            }
            int16u CodePage2;
            Get_B2 (CodePage2,                                  "CodePage2");
            if (CodePage2==0x02)
            {
                Get_ISO_8859_2(Size-3, Value,                   Info);
            }
            else //Unknown
                Get_Local(Size-3, Value,                        Info);
        }

    }
    else
        Get_Local(Size, Value,                                  Info);
}

//---------------------------------------------------------------------------
//Modified Julian Date
Ztring File_Mpeg_Descriptors::Date_MJD(int16u Date_)
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
Ztring File_Mpeg_Descriptors::Time_BCD(int32u Time)
{
    return (((Time>>16)&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time>>16)&0xFF, 16)+__T(":") //BCD
         + (((Time>> 8)&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time>> 8)&0xFF, 16)+__T(":") //BCD
         + (((Time    )&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time    )&0xFF, 16);        //BCD
}

//---------------------------------------------------------------------------
//Form: HHMM, BCD
Ztring File_Mpeg_Descriptors::TimeHHMM_BCD(int16u Time)
{
    return (((Time>> 8)&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time>> 8)&0xFF, 16)+__T(":") //BCD
         + (((Time    )&0xFF)<10?__T("0"):__T("")) + Ztring::ToZtring((Time    )&0xFF, 16)+__T(":00"); //BCD
}

//---------------------------------------------------------------------------
//Form: Frequency in 10 KHz
Ztring File_Mpeg_Descriptors::Frequency_DVB__BCD(int32u Frequency)
{
    int64u ToReturn=((((int64u)Frequency)>>28)&0xF)*10000000
                  + ((((int64u)Frequency)>>24)&0xF)* 1000000
                  + ((((int64u)Frequency)>>20)&0xF)*  100000
                  + ((((int64u)Frequency)>>16)&0xF)*   10000
                  + ((((int64u)Frequency)>>12)&0xF)*    1000
                  + ((((int64u)Frequency)>> 8)&0xF)*     100
                  + ((((int64u)Frequency)>> 4)&0xF)*      10
                  + ((((int64u)Frequency)    )&0xF)*       1;
    return Ztring::ToZtring(ToReturn*10000);
}

//---------------------------------------------------------------------------
//Form: Orbital Position
Ztring File_Mpeg_Descriptors::OrbitalPosition_DVB__BCD(int32u OrbitalPosition)
{
    int64u ToReturn=((OrbitalPosition>>12)&0xF)*    1000
                  + ((OrbitalPosition>> 8)&0xF)*     100
                  + ((OrbitalPosition>> 4)&0xF)*      10
                  + ((OrbitalPosition    )&0xF)         ;
    return Ztring::ToZtring(((float)ToReturn)/10, 1);
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_MPEGTS_YES
