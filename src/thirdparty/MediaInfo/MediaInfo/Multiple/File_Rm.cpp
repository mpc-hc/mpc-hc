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
#if defined(MEDIAINFO_RM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <ZenLib/ZtringListList.h>
#include <ZenLib/Utils.h>
#include "MediaInfo/Multiple/File_Rm.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

// https://common.helixcommunity.org/2003/HCS_SDK_r5/htmfiles/rmff.htm
// http://wiki.multimedia.cx/index.php?title=RealMedia

//***************************************************************************
// Const
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int32u  RMF=0x2E524D46;
    const int32u CONT=0x434F4E54;
    const int32u DATA=0x44415441;
    const int32u INDX=0x494E4458;
    const int32u MDPR=0x4D445052;
    const int32u PROP=0x50524F50;
    const int32u RJMD=0x524A4D44;
    const int32u RMJE=0x524D4A45;
    const int32u RMMD=0x524D4D44;
    const int32u TAG =0x54414700;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Rm::File_Rm()
:File__Analyze()
{
    //In
    FromMKV_StreamType=Stream_Max;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Rm::FileHeader_Begin()
{
    if (IsSub)
        return true;

    if (4>Buffer_Size)
        return false;
    if (Buffer[0]!=0x2E //".RMF"
     || Buffer[1]!=0x52
     || Buffer[2]!=0x4D
     || Buffer[3]!=0x46)
    {
        Reject();
        return false;
    }

    return true;
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Rm::Header_Parse()
{
    //Specific case
    if (FromMKV_StreamType!=Stream_Max)
    {
        //Filling
        Header_Fill_Code(0, __T("Real Media Header"));
        Header_Fill_Size(Element_Size);
        return;
    }

    //Parsing
    int32u Name, Size;
    Get_C4 (Name,                                               "Name");

    if (Name==Elements::RMMD)
    {
        Size=8; //Name + Size of the Metadata section (after Version) in bytes.
    }
    else if (Name==Elements::RJMD)
    {
        Skip_B4(                                                "Version");
        Get_B4 (Size,                                           "Size");
        Size+=8; //Name + Version + Size of the section (after Version) in bytes.
        if (Element_Size>=12)
            Element_Offset-=8; //Is valid, so we must keep Version and size in the stream
    }
    else if (Name==Elements::RMJE)
    {
        Size=12;
    }
    else if ((Name&0xFFFFFF00)==Elements::TAG)
    {
        Name=Elements::TAG;
        Element_Offset-=4;
        Size=0;
    }
    else
    {
        Get_B4 (Size,                                           "Size");
    }

    //Filling
    Header_Fill_Code(Name, Ztring().From_CC4(Name));
    Header_Fill_Size(Size);
}

//---------------------------------------------------------------------------
void File_Rm::Data_Parse()
{
    //Specific case
    if (FromMKV_StreamType!=Stream_Max)
    {
        switch (FromMKV_StreamType)
        {
            case Stream_Video : MDPR_realvideo(); break;
            case Stream_Audio : MDPR_realaudio(); break;
            default           : ;
        }

        Finish("RealMedia");
        return;
    }

    //Parsing
    DATA_BEGIN
    ATOM( RMF)
    ATOM(CONT)
    LIST_SKIP(DATA)
    ATOM(INDX)
    ATOM(MDPR)
    ATOM(PROP)
    ATOM(RJMD)
    ATOM(RMJE)
    ATOM(RMMD)
    ATOM(TAG)
    DATA_END
}

//***************************************************************************
// Elements
//***************************************************************************

#define NAME_VERSION(ELEMENT_NAME) \
    Element_Name(ELEMENT_NAME); \
    int16u Version; \
    { \
        Get_B2(Version,                                         "ObjectVersion"); \
    } \

#define INTEGRITY_VERSION(_VERSION) \
    if (Version>_VERSION) \
    { \
        Skip_XX(Element_Size-Element_Offset,                    "Data"); \
        return; \
    } \

//---------------------------------------------------------------------------
void File_Rm::RMF()
{
    NAME_VERSION("Real Media Format");
    INTEGRITY_VERSION(1);

    //Parsing
    if (Element_Size==4)
        Skip_B2(                                                "file_version"); //The version of the RealMedia file.
    else
        Skip_B4(                                                "file_version"); //The version of the RealMedia file.
    Skip_B4(                                                    "num_headers"); //The number of headers in the header section that follow the RealMedia File Header.

    //Filling
    Accept("RealMedia");
    Fill(Stream_General, 0, General_Format, "RealMedia");
}

//---------------------------------------------------------------------------
void File_Rm::CONT()
{
    NAME_VERSION("Content");
    INTEGRITY_VERSION(0);

    //Parsing
    Ztring title, author, copyright, comment;
    int16u title_len, author_len, copyright_len, comment_len;
    Get_B2 (title_len,                                          "title_len"); //The length of the title data in bytes.
    Get_Local(title_len, title,                                 "title"); //An array of ASCII characters that represents the title information for the RealMedia file.
    Get_B2 (author_len,                                         "author_len"); //The length of the author data in bytes.
    Get_Local(author_len, author,                               "author"); //An array of ASCII characters that represents the author information for the RealMedia file.
    Get_B2 (copyright_len,                                      "copyright_len"); //The length of the copyright data in bytes.
    Get_Local(copyright_len, copyright,                         "copyright"); //An array of ASCII characters that represents the copyright information for the RealMedia file.
    Get_B2 (comment_len,                                        "comment_len"); //The length of the comment data in bytes.
    Get_Local(comment_len, comment,                             "comment"); //An array of ASCII characters that represents the comment information for the RealMedia file.

    //Filling
    Fill(Stream_General, 0, General_Title, title);
    Fill(Stream_General, 0, General_Performer, author);
    Fill(Stream_General, 0, General_Copyright, copyright);
    Fill(Stream_General, 0, General_Comment, comment);
}

//---------------------------------------------------------------------------
void File_Rm::DATA()
{
    NAME_VERSION("Data");

    //Currently, we stop here, enough info
    Finish("RealMedia");
    return;

    /*
    //Parsing
    int32u num_packets;
    int16u length;
    int8u  flags;
    Get_B4 (num_packets,                                        "num_packets"); //Number of packets in the data chunk.
    Skip_B4(                                                    "next_data_header"); //Offset from start of file to the next data chunk. A non-zero value refers to the file offset of the next data chunk. A value of zero means there are no more data chunks in this file. This field is not typically used.
    for (int32u Pos=0; Pos<num_packets; Pos++)
    {
        Element_Begin1("packet");
        Get_B2 (Version,                                        "object_version");
        INTEGRITY_VERSION(1);
        Get_B2 (length,                                         "length"); //The length of the packet in bytes.
        if (Version==0)
            Element_Info1("Media_Packet_Header");
        else
            Element_Info1("Media_Packet_Header");
        Skip_B2(                                                "stream_number"); //The 16-bit alias used to associate data packets with their associated Media Properties Header.
        Skip_B4(                                                "timestamp"); //The time stamp of the packet in milliseconds.
        if (Version==0)
        {
            Skip_B1(                                            "packet_group"); //The packet group to which the packet belongs. If packet grouping is not used, set this field to 0 (zero).
            Get_B1 (flags,                                      "flags"); //Flags describing the properties of the packet.
                Skip_Flags(flags, 0,                            "reliable"); //If this flag is set, the packet is delivered reliably.
                Skip_Flags(flags, 1,                            "keyframe"); //If this flag is set, the packet is part of a key frame or in some way marks a boundary in your data stream.
        }
        if (Version==1)
        {
            Skip_B2(                                            "asm_rule"); //The ASM rule assigned to this packet.
            Skip_B1(                                            "asm_Flags"); //Contains HX_  flags that dictate stream switching points.
        }
        if (Version==0)
            Skip_XX(length-12,                                  "data");
        else
            Skip_XX(length-13,                                  "data");

        //Stopping if too far
        if (Pos>10)
        {
            Pos=num_packets;
            Element_Info1("(...)");
        }

        Element_End0();
    }
    */
}

//---------------------------------------------------------------------------
void File_Rm::INDX()
{
    NAME_VERSION("INDeX");

    //Parsing
    int32u num_indices;
    Get_B4 (num_indices,                                        "num_indices"); //Number of index records in the index chunk.
    Skip_B2(                                                    "stream_number"); //The stream number for which the index records in this index chunk are associated.
    Skip_B4(                                                    "next_index_header"); //Offset from start of file to the next index chunk. This member enables RealMedia file format readers to find all the index chunks quickly. A value of zero for this member indicates there are no more index headers in this file.
    for (int32u Pos=0; Pos<num_indices; Pos++)
    {
        Element_Begin1("index");
        Get_B2 (Version,                                        "object_version");
        INTEGRITY_VERSION(0);
        Element_Info1("Media_Packet_Header");
        Skip_B4(                                                "timestamp"); //The time stamp (in milliseconds) associated with this record.
        Skip_B4(                                                "offset"); //The offset from the start of the file at which this packet can be found.
        Skip_B4(                                                "packet_count_for_this_packet"); //The packet number of the packet for this record. This is the same number of packets that would have been seen had the file been played from the beginning to this point.
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Rm::MDPR()
{
    NAME_VERSION("MeDia PRoperties");
    INTEGRITY_VERSION(0);

    //Parsing
    Ztring stream_name;
    std::string mime_type;
    int32u avg_bit_rate, start_time, duration, type_specific_len;
    int16u stream_number;
    int8u stream_name_size, mime_type_size;
    Get_B2 (stream_number,                                      "stream_number"); //Unique value that identifies a physical stream
    Skip_B4(                                                    "max_bit_rate"); //The maximum bit rate required to deliver this stream over a network.
    Get_B4 (avg_bit_rate,                                       "avg_bit_rate"); //The average bit rate required to deliver this stream over a network.
    Skip_B4(                                                    "max_packet_size"); //The largest packet size (in bytes) in the stream of media data.
    Skip_B4(                                                    "avg_packet_size"); //The average packet size (in bytes) in the stream of media data.
    Get_B4 (start_time,                                         "start_time"); //The time offset in milliseconds to add to the time stamp of each packet in a physical stream.
    Skip_B4(                                                    "preroll"); //The time offset in milliseconds to subtract from the time stamp of each packet in a physical stream.
    Get_B4 (duration,                                           "duration"); //The duration of the stream in milliseconds.
    Get_B1 (stream_name_size,                                   "stream_name_size"); //The length of the following stream_name  member in bytes.
    Get_Local(stream_name_size, stream_name,                    "stream_name"); //A nonunique alias or name for the stream.
    Get_B1 (mime_type_size,                                     "mime_type_size"); //The length of the following mime_type  field in bytes.
    Get_String(mime_type_size, mime_type,                       "mime_type"); //A nonunique MIME style type/subtype string for data associated with the stream.
    Get_B4 (type_specific_len,                                  "type_specific_len"); //The length of the following type_specific_data in bytes

    //Parsing TypeSpecific
    Element_Info1(mime_type.c_str());
    MDPR_IsStream=true;
         if (mime_type=="audio/x-pn-multirate-realaudio")
        MDPR_IsStream=false; //What do we with this?
    else if (mime_type=="audio/X-MP3-draft-00")
    {
        Stream_Prepare(Stream_Audio);
        CodecID_Fill(Ztring(mime_type.c_str()), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Real);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "MPEG1AL3");
    }
    else if (mime_type=="audio/x-pn-realaudio")
        MDPR_realaudio();
    else if (mime_type=="audio/x-pn-realaudio-encrypted")
    {
        MDPR_realaudio();
        Fill(Stream_Audio, StreamPos_Last, Audio_Encryption, "Y");
    }
    else if (mime_type=="audio/x-ralf-mpeg4")
    {
        Stream_Prepare(Stream_Audio);
        CodecID_Fill(Ztring(mime_type.c_str()), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Real);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "ralf");
    }
    else if (mime_type=="audio/x-ralf-mpeg4-generic")
    {
        Stream_Prepare(Stream_Audio);
        CodecID_Fill(Ztring(mime_type.c_str()), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Real);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "ralf");
    }
    else if (mime_type.find("audio/")==0)
        Stream_Prepare(Stream_Audio);
    else if (mime_type=="video/text")
        Stream_Prepare(Stream_Text);
    else if (mime_type=="video/x-pn-multirate-realvideo")
        MDPR_IsStream=false; //What do we with this?
    else if (mime_type=="video/x-pn-realvideo")
        MDPR_realvideo();
    else if (mime_type=="video/x-pn-realvideo-encrypted")
    {
        MDPR_realvideo();
        Fill(Stream_Video, StreamPos_Last, Video_Encryption, "Y");
    }
    else if (mime_type.find("video/")==0)
        Stream_Prepare(Stream_Video);
    else if (mime_type=="logical-audio/x-pn-multirate-realaudio")
        MDPR_IsStream=false; //What do we with this?
    else if (mime_type.find("logical-audio/")==0)
        MDPR_IsStream=false; //What do we with this?
    else if (mime_type=="logical-fileinfo")
        MDPR_fileinfo();
    //else if (mime_type=="logical-video/x-pn-multirate-realvideo")
    //    MDPR_IsStream=false; //What do we with this?
    //else if (mime_type.find("logical-video/")==0)
    //    MDPR_IsStream=false; //What do we with this?
    else
        MDPR_IsStream=false;

    //Filling
    FILLING_BEGIN();
        if (MDPR_IsStream)
        {
            Fill(StreamKind_Last, StreamPos_Last, General_ID, stream_number);
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate), avg_bit_rate, 10, true);
            //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), start_time);
            //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), duration);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Rm::MDPR_realvideo()
{
    //Parsing
    int32u Codec;
    int16u Width, Height, FrameRate;
    Skip_B4(                                                    "Size");
    Skip_C4(                                                    "FCC");
    Get_C4 (Codec,                                              "Compression");
    Get_B2 (Width,                                              "Width");
    Get_B2 (Height,                                             "Height");
    Skip_B2(                                                    "bpp"); //Do not use it
    Skip_B4(                                                    "Unknown");
    Get_B2 (FrameRate,                                          "fps");
    Skip_B2(                                                    "Unknown");
    Skip_C4(                                                    "Type1");
    Skip_C4(                                                    "Type2");

    //Filling
    if (!Status[IsAccepted])
        Accept("RealMedia"); //Is subs

    Stream_Prepare(Stream_Video);
    if (FromMKV_StreamType==Stream_Max) //Using the one from the container
        CodecID_Fill(Ztring().From_CC4(Codec), Stream_Video, StreamPos_Last, InfoCodecID_Format_Real);
    Fill(Stream_Video, StreamPos_Last, Video_Codec, Ztring().From_CC4(Codec));
    Fill(Stream_Video, StreamPos_Last, Video_Width, Width); //Width
    Fill(Stream_Video, StreamPos_Last, Video_Height, Height); //Height
    switch (FrameRate)
    {
        case 23 : Fill(Stream_Video, StreamPos_Last, Video_FrameRate, ((float)24)*1000/1001); break; //Hard-coded frame rate?
        case 29 : Fill(Stream_Video, StreamPos_Last, Video_FrameRate, ((float)30)*1000/1001); break; //Hard-coded frame rate?
        default : Fill(Stream_Video, StreamPos_Last, Video_FrameRate, (float)FrameRate);
    }
}

//---------------------------------------------------------------------------
void File_Rm::MDPR_realaudio()
{
    //Parsing
    Ztring FourCC3="lpcJ"; //description of this codec : http://focus.ti.com/lit/an/spra136/spra136.pdf http://en.wikipedia.org/wiki/VSELP
    Ztring FourCC4;
    int32u FourCC5=0, BytesPerMinute=0;
    int16u Version, Samplerate=8000, Samplesize=16, Channels=0;
    Skip_C4(                                                    "Header signature");
    Get_B2 (Version,                                            "Version");
    INTEGRITY_VERSION(5);
    if (Version==3)
    {
        Ztring title, author, copyright, comment;
        int32u length;
        int8u title_len, author_len, copyright_len, comment_len;
        Skip_B2(                                                "Header size"); //Header size, after this tag.
        Get_B2 (Channels,                                       "Channels");
        Skip_B4(                                                "Uknown");
        Skip_B4(                                                "Uknown");
        Skip_B4(                                                "Data size");
        Get_B1 (title_len,                                      "title_len"); //The length of the title data in bytes.
        Get_Local(title_len, title,                             "title"); //An array of ASCII characters that represents the title information for the RealMedia file.
        Get_B1 (author_len,                                     "author_len"); //The length of the author data in bytes.
        Get_Local(author_len, author,                           "author"); //An array of ASCII characters that represents the author information for the RealMedia file.
        Get_B1 (copyright_len,                                  "copyright_len"); //The length of the copyright data in bytes.
        Get_Local(copyright_len, copyright,                     "copyright"); //An array of ASCII characters that represents the copyright information for the RealMedia file.
        Get_B1 (comment_len,                                    "comment_len"); //The length of the comment data in bytes.
        Get_Local(comment_len, comment,                         "comment"); //An array of ASCII characters that represents the comment information for the RealMedia file.
        if (Element_Offset<Element_Size) //Optional
        {
            Skip_B1(                                            "Uknown");
            Get_B4 (length,                                     "Fourcc string length");
            Get_Local(length, FourCC3,                          "Fourcc string");
        }

        //Filling
        Fill(Stream_General, 0, General_Duration, title);
        Fill(Stream_General, 0, General_Performer, author);
        Fill(Stream_General, 0, General_Copyright, copyright);
        Fill(Stream_General, 0, General_Comment, comment);
    }
    if (Version==4 || Version==5)
    {
        Skip_B2(                                                "Unused");
        Skip_C4(                                                "ra signature");
        Skip_B4(                                                "AudioFileSize");
        Skip_B2(                                                "Version2");
        Skip_B4(                                                "Header size");
        Skip_B2(                                                "Codec flavor");
        Skip_B4(                                                "Coded frame size");
        Skip_B4(                                                "AudioBytes");
        Get_B4 (BytesPerMinute,                                 "BytesPerMinute");
        Skip_B4(                                                "Unknown");
        Skip_B2(                                                "Sub packet h");
        Skip_B2(                                                "Frame size");
        Skip_B2(                                                "Subpacket size");
        Skip_B2(                                                "Unknown");
    }
    if (Version==5)
    {
        Skip_B2(                                                "Unknown");
        Skip_B2(                                                "Unknown");
        Skip_B2(                                                "Unknown");
    }
    if (Version==4 || Version==5)
    {
        Get_B2 (Samplerate,                                     "Samplerate");
        Skip_B2(                                                "Unknown");
        Get_B2 (Samplesize,                                     "Samplesize");
        Get_B2 (Channels,                                       "Channels");
    }
    if (Version==4)
    {
        int8u length;
        Get_B1 (length,                                         "Interleaver ID string lengt");
        Skip_Local(length,                                      "Interleaver ID string");
        Get_B1 (length,                                         "FourCC string lengt");
        Get_Local(length, FourCC4,                              "FourCC string");
    }
    if (Version==5)
    {
        Skip_C4(                                                "Interleaver ID");
        Get_C4 (FourCC5,                                        "FourCC");
    }
    if (Version==4 || Version==5)
    {
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
        Skip_B1(                                                "Unknown");
    }
    if (Version==5)
    {
        Skip_B1(                                                "Unknown");
    }
    if (Version==4 || Version==5)
    {
        int32u length;
        Get_B4 (length,                                         "Codec extradata length");
        Skip_XX(length,                                         "Codec extradata");
    }

    //Filling
    if (!Status[IsAccepted])
        Accept("RealMedia"); //Is subs

    Stream_Prepare(Stream_Audio);
    if (Version==3)
    {
        if (FromMKV_StreamType==Stream_Max) //Using the one from the container
            CodecID_Fill(FourCC3, Stream_Audio, StreamPos_Last, InfoCodecID_Format_Real);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, FourCC3);
    }
    if (Version==4)
    {
        if (FromMKV_StreamType==Stream_Max) //Using the one from the container
            CodecID_Fill(FourCC4, Stream_Audio, StreamPos_Last, InfoCodecID_Format_Real);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, FourCC4);
    }
    if (Version==5)
    {
        if (FromMKV_StreamType==Stream_Max) //Using the one from the container
            CodecID_Fill(Ztring().From_CC4(FourCC5), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Real);
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Ztring().From_CC4(FourCC5));
    }
    Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, Samplerate);
    Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, Samplesize);
    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels);
    if (BytesPerMinute)
        Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, BytesPerMinute*8/60, 10, true);
}

//---------------------------------------------------------------------------
void File_Rm::MDPR_fileinfo()
{
    MDPR_IsStream=false;

    //Parsing
    int16u Version, num_physical_streams, num_rules, num_properties;
    Skip_B4(                                                    "size");
    Get_B2 (Version,                                            "object_version");
    INTEGRITY_VERSION(0);
    Get_B2 (num_physical_streams,                               "num_physical_streams"); //The number of physical streams that make up this logical stream.
    for (int16u Pos=0; Pos<num_physical_streams; Pos++)
    {
        Skip_B2(                                                "physical_stream_numbers"); //The list of physical stream numbers that comprise this logical stream.
        Skip_B4(                                                "data_offsets"); //The list of data offsets indicating the start of the data section for each physical stream.
    }
    Get_B2 (num_rules,                                          "num_rules"); //The number of ASM rules for the logical stream. Each physical stream in the logical stream has at least one ASM rule associated with it or it will never get played. The mapping of ASM rule numbers to physical stream numbers is stored in a list immediately following this member. These physical stream numbers refer to the stream_number  field found in the Media Properties Object for each physical stream belonging to this logical stream.
    for (int16u Pos=0; Pos<num_physical_streams; Pos++)
        Skip_B2(                                                "rule_to_physical_stream_number_map"); //The list of physical stream numbers that map to each rule. Each entry in the map corresponds to a 0-based rule number. The value in each entry is set to the physical stream number for the rule.
    Get_B2 (num_properties,                                     "num_properties"); //The number of NameValueProperty structures contained in this structure. These name/value structures can be used to identify properties of this logical stream (for example, language).

    //Parsing
    for (int16u Pos=0; Pos<num_properties; Pos++)
    {
        Element_Begin1("property");
        std::string name;
        int32u size, type;
        int16u value_length;
        int8u name_length;
        Peek_B4(size);
        Skip_B4(                                                "size");
        Skip_B2(                                                "object_version");
        Get_B1 (name_length,                                    "name_length"); //The length of the name data.
        Get_String(name_length, name,                           "name"); //The name string data.
        Get_B4 (type,                                           "type"); //The type of the value data.
        Get_B2 (value_length,                                   "value_length"); //value_length
        switch (type)
        {
            case 0 : //Unsigned integer
                Skip_B4(                                        "value_data"); break; //unsigned integer
            case 2 : //String
                Skip_Local(value_length,                        "value_data"); break; //string
            default : Skip_XX(value_length,                     "unknown");
        }
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Rm::PROP()
{
    NAME_VERSION("PROPerties");
    INTEGRITY_VERSION(0);

    //Parsing
    int32u avg_bit_rate, duration;
    int16u flags;
    Skip_B4(                                                    "max_bit_rate"); //The maximum bit rate required to deliver this file over a network.
    Get_B4 (avg_bit_rate,                                       "avg_bit_rate"); //The average bit rate required to deliver this file over a network.
    Skip_B4(                                                    "max_packet_size"); //The largest packet size (in bytes) in the media data.
    Skip_B4(                                                    "avg_packet_size"); //The average packet size (in bytes) in the media data.
    Skip_B4(                                                    "num_packets"); //The number of packets in the media data.
    Get_B4 (duration,                                           "duration"); //The duration of the file in milliseconds.
    Skip_B4(                                                    "preroll"); //The number of milliseconds to prebuffer before starting playback.
    Skip_B4(                                                    "index_offset"); //The offset in bytes from the start of the file to the start of the index header object.
    Skip_B4(                                                    "data_offset"); //The offset in bytes from the start of the file to the start of the Data Section. \n Note: There can be a number of Data_Chunk_Headers in a RealMedia file. The data_offset  value specifies the offset in bytes to the first Data_Chunk_Header. The offsets to the other Data_Chunk_Headers can be derived from the next_data_header field in a Data_Chunk_Header.
    Skip_B2(                                                    "num_streams"); //The total number of media properties headers in the main headers section.
    Get_B2 (flags,                                              "flags"); //Bit mask containing information about this file.
        Skip_Flags(flags, 0,                                    "Save_Enabled"); //If 1, clients are allowed to save this file to disk.
        Skip_Flags(flags, 1,                                    "Perfect_Play"); //If 1, clients are instructed to use extra buffering.
        Skip_Flags(flags, 2,                                    "Live_Broadcast"); //If 1, these streams are from a live broadcast.
        Skip_Flags(flags, 3,                                    "Allow_Download");

    //Filling
    Fill(Stream_General, 0, General_OverallBitRate, avg_bit_rate);
    Fill(Stream_General, 0, General_Duration, duration);
}

//---------------------------------------------------------------------------
void File_Rm::RJMD()
{
    Element_Name("Metadata Tag");

    //Parsing
    Skip_B4(                                                    "object_version");

    //Parsing
    RJMD_property(std::string());
}

//---------------------------------------------------------------------------
void File_Rm::RJMD_property(std::string Name)
{
    //Element_Name("Property");

    //Parsing
    Ztring value;
    std::string name;
    int32u type, flags, num_subproperties, name_length, value_length;
    Element_Begin1("MetadataProperty");
    Skip_B4(                                                    "size");
    Get_B4 (type,                                               "type");
    Get_B4 (flags,                                              "flags");
        Skip_Flags(flags, 0,                                    "readonly"); //Read only, cannot be modified.
        Skip_Flags(flags, 1,                                    "private"); //Private, do not expose to users.
        Skip_Flags(flags, 2,                                    "type_dexcriptor"); //Type descriptor used to further define type of value.
    Skip_B4(                                                    "value_offset"); //The offset to the value_length , relative to the beginning of the MetadataProperty  structure.
    Skip_B4(                                                    "subproperties_offset"); //The offset to the subproperties_list , relative to the beginning of the MetadataProperty  structure.
    Get_B4 (num_subproperties,                                  "num_subproperties"); //The number of subproperties for this MetadataProperty  structure.
    Get_B4 (name_length,                                        "name_length"); //The length of the name data, including the null-terminator.
    Get_String(name_length, name,                               "name"); //The name of the property (string data).
    Get_B4 (value_length,                                       "value_length"); //The length of the value data.
    switch(type)
    {
        case 0x00 : //Nothing
                    Skip_XX(value_length,                       "Junk");
                    break;
        case 0x01 : //String (text).
                    Get_Local(value_length, value,              "value"); //String.
                    break;
        case 0x02 : //Separated list of strings, separator specified as sub-property/type descriptor.
                    Get_Local(value_length, value,              "value"); //String.
                    break;
        case 0x03 : //Boolean flag—either 1 byte or 4 bytes, check size value.
                    switch(value_length)
                    {
                    case 1 : {
                                 int8u valueI;
                                 Get_L1(valueI,                 "value"); //1-byte boolean.
                                 value.From_Number(valueI);
                             }
                             break;
                    case 4 : {
                                 int32u valueI;
                                 Get_L4(valueI,                 "value"); //4-byte boolean.
                                 value.From_Number(valueI);
                             }
                             break;
                        default: Skip_XX(value_length,          "Unknown");
                    }
                    break;
        case 0x04 : //Four-byte integer.
                    {
                    int32u valueI;
                    Get_L4(valueI,                              "value");
                    value.From_Number(valueI);
                    }
                    break;
        case 0x05 : //Byte stream.
                    Skip_XX(value_length,                       "Byte stream");
                    break;
        case 0x06 : //String (URL).
                    Get_Local(value_length, value,              "value"); //String.
                    break;
        case 0x07 : //String representation of the date in the form: YYYYmmDDHHMMSS (m = month, M = minutes).
                    Get_Local(value_length, value,              "value"); //String.
                    break;
        case 0x08 : //String (file name)
                    Get_Local(value_length, value,              "value"); //String.
                    break;
        case 0x09 : //This property has subproperties, but its own value is empty.
                    Skip_XX(value_length,                       "junk");
                    break;
        case 0x0A : //Large buffer of data, use sub-properties/type descriptors to identify mime-type.
                    Skip_XX(value_length,                       "data");
                    break;
        default   : //Unknown
                    Skip_XX(value_length,                       "unknown");
                    break;
    }

    //Filling
    if (!Name.empty())
        Name+='/';
    Name+=name;
    if (Name!="Track/Comments/DataSize"
     && Name!="Track/Comments/MimeType"
       )
    Fill(Stream_General, 0, Name.c_str(), value);

    //Parsing
    for (int32u Pos=0; Pos<num_subproperties; Pos++)
    {
        Element_Begin1("PropListEntry");
        Skip_B4(                                                "offset"); //The offset for this indexed sub-property, relative to the beginning of the containing MetadataProperty.
        Skip_B4(                                                "num_props_for_name"); //The number of sub-properties that share the same name. For example, a lyrics property could have multiple versions as differentiated by the language sub-property type descriptor.
        Element_End0();
    }
    for (int32u Pos=0; Pos<num_subproperties; Pos++)
    {
        RJMD_property(Name);
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Rm::RMJE()
{
    Element_Name("Metadata Section Footer");

    //Parsing
    Skip_B4(                                                    "object_version");
    Skip_B4(                                                    "size"); //The size of the preceding metadata tag.
}

//---------------------------------------------------------------------------
void File_Rm::RMMD()
{
    Element_Name("Metadata Section Header");

    //Parsing
    Skip_B4(                                                    "size"); //The size of the full metadata section in bytes.
}

//---------------------------------------------------------------------------
void File_Rm::TAG()
{
    Element_Name("Id3v1 Tag");
}

} //NameSpace

#endif //MEDIAINFO_RM_YES

