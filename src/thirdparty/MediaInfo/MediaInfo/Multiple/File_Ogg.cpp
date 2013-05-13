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
#if defined(MEDIAINFO_OGG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ogg.h"
#include "MediaInfo/Multiple/File_Ogg_SubElement.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ogg::File_Ogg()
:File__Analyze()
{
    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    SizedBlocks=false;
    XiphLacing=false;

    //Temp - Global
    StreamsToDo=0;
    Parsing_End=false;

    //Temp - Stream
    Chunk_Sizes_Finished=true;
    packet_type=0;
    continued=false;
    eos=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ogg::Streams_Fill()
{
    std::map<int64u, stream>::iterator Stream_Temp=Stream.begin();
    while (Stream_Temp!=Stream.end())
    {
        //Filling
        if (Stream_Temp->second.Parser)
        {
            Stream_Temp->second.Parser->Fill();
            Merge(*Stream_Temp->second.Parser);
            Merge(*Stream_Temp->second.Parser, Stream_General, 0, 0);
            Stream_Temp->second.StreamKind=((File_Ogg_SubElement*)Stream_Temp->second.Parser)->StreamKind;
            Stream_Temp->second.StreamPos=Count_Get(Stream_Temp->second.StreamKind)-1;
            if (!SizedBlocks && !XiphLacing)
                Stream_Temp->second.absolute_granule_position_Resolution=((File_Ogg_SubElement*)Stream_Temp->second.Parser)->absolute_granule_position_Resolution;
            if (Stream_Temp->second.StreamKind==Stream_Audio && Stream_Temp->second.absolute_granule_position_Resolution==0)
                Stream_Temp->second.absolute_granule_position_Resolution=Retrieve(Stream_Audio, Stream_Temp->second.StreamPos, Audio_SamplingRate).To_int64u();
            if (!IsSub && Stream_Temp->second.absolute_granule_position && Stream_Temp->second.absolute_granule_position_Resolution)
            {
                if (Stream_Temp->second.StreamKind==Stream_Audio)
                    Fill(Stream_Temp->second.StreamKind, Stream_Temp->second.StreamPos, Fill_Parameter(Stream_Temp->second.StreamKind, Generic_Duration), float64_int64s(((float64)(Stream_Temp->second.absolute_granule_position))*1000/Stream_Temp->second.absolute_granule_position_Resolution), 10, true);
            }
            if (!IsSub)
            {
                if (Stream_Temp->second.StreamKind==Stream_Max)
                {
                    Stream_Temp->second.StreamKind=Stream_General;
                    Stream_Temp->second.StreamPos=0;
                }
                Fill(Stream_Temp->second.StreamKind, Stream_Temp->second.StreamPos, General_ID, Stream_Temp->first);
                Fill(Stream_Temp->second.StreamKind, Stream_Temp->second.StreamPos, General_ID_String, Ztring::ToZtring(Stream_Temp->first)+__T(" (0x")+Ztring::ToZtring(Stream_Temp->first, 16)+__T(')'), true);
            }
        }
        ++Stream_Temp;
    }

    Fill(Stream_General, 0, General_Format, "OGG", Unlimited, true, true);
    if (Count_Get(Stream_Video)==0 && Count_Get(Stream_Image)==0)
        Fill(Stream_General, 0, General_InternetMediaType, "audio/ogg", Unlimited, true, true);
}

//---------------------------------------------------------------------------
void File_Ogg::Streams_Finish()
{
    std::map<int64u, stream>::iterator Stream_Temp=Stream.begin();
    while (Stream_Temp!=Stream.end())
    {
        //Filling
        if (Stream_Temp->second.Parser)
        {
            Finish(Stream_Temp->second.Parser);
            Merge(*Stream_Temp->second.Parser, Stream_Temp->second.StreamKind, 0, Stream_Temp->second.StreamPos);
            Merge(*Stream_Temp->second.Parser, Stream_General, 0, 0);
        }
        ++Stream_Temp;
    }

    //No more need
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Stream.clear();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ogg::FileHeader_Begin()
{
    //Must have enough buffer for having header
    if (Buffer_Size<4)
        return false; //Must wait for more data

    //False positives detection: Detect AVI files, or the parser can synchronize with OggS stream in a AVI chunk
    if (CC4(Buffer)==0x52494646) //"RIFF"
    {
        Finish("OGG");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ogg::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+4<=Buffer_Size)
    {
        while(Buffer_Offset+4<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x4F
                                            || Buffer[Buffer_Offset+1]!=0x67
                                            || Buffer[Buffer_Offset+2]!=0x67
                                            || Buffer[Buffer_Offset+3]!=0x53)) //"OggS"
        {
            Buffer_Offset+=1+2;
            while(Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x67)
                Buffer_Offset+=2;
            if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x67)
                Buffer_Offset--;
            Buffer_Offset--;
        }

        if (Buffer_Offset+4<=Buffer_Size) //Testing if size is coherant
        {
            //Retrieving some info
            if (Buffer_Offset+27>Buffer_Size)
                return false; //Need more data
            int8u page_segments=CC1(Buffer+Buffer_Offset+26);
            if (Buffer_Offset+27+page_segments>Buffer_Size)
                return false; //Need more data
            size_t Size=0;
            for (int8u Pos=0; Pos<page_segments; Pos++)
                Size+=CC1(Buffer+Buffer_Offset+27+Pos);

            //Testing
            if (Buffer_Offset+27+page_segments+Size+4>Buffer_Size)
                return false; //Need more data
            if (CC4(Buffer+Buffer_Offset+27+page_segments+Size)!=0x4F676753) //"OggS"
                Buffer_Offset++;
            else
                break;
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+4>Buffer_Size)
    {
        if (Buffer_Offset+3==Buffer_Size && CC3(Buffer+Buffer_Offset)!=0x4F6767) //"Ogg"
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x4F67)   //"Og"
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x4F)     //"O"
            Buffer_Offset++;
        return false;
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Ogg::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+4>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC4(Buffer+Buffer_Offset)!=0x4F676753) //"OggS"
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ogg::Header_Parse()
{
    //Specific case
    if (SizedBlocks)
    {
        int16u Size;
        Get_B2 (Size,                                           "Size");

        Chunk_Sizes.clear();
        Chunk_Sizes.push_back(Size);
        Header_Fill_Size(2+Size);
        Header_Fill_Code(0, Ztring::ToZtring(0, 16));
        return;
    }
    if (XiphLacing)
    {
        if (Chunk_Sizes.empty())
        {
            int8u CountMinus1;
            Get_B1 (CountMinus1,                                    "Number of frames minus one");

            int64u UsedSize=0;
            for (size_t Pos=0; Pos<CountMinus1; Pos++)
            {
                int32u Size=0;
                int8u Size8;
                do
                {
                    Get_B1 (Size8,              "Size");
                    Size+=Size8;
                }
                while (Size8==0xFF);
                Param_Info1(Size);
                Chunk_Sizes.push_back(Size);
                UsedSize+=Size;
            }
            Chunk_Sizes.push_back((size_t)(Element_Size-UsedSize-1));
        }

        Header_Fill_Size(Element_Size);
        Header_Fill_Code(0, Ztring::ToZtring(0, 16));
        return;
    }

    //Parsing
    int64u absolute_granule_position;
    int32u stream_serial_number, page_sequence_no;
    int16u total_page_size;
    int8u  stream_structure_version, flags, page_segments, packet_lacing_value;
    Skip_C4(                                                    "capture_pattern");
    Get_L1 (stream_structure_version,                           "stream_structure_version");
    Get_L1 (flags,                                              "header_type_flag");
        Get_Flags (flags, 0, continued,                         "continued packet");
        Skip_Flags(flags, 1,                                    "first page of logical bitstream (bos)");
        Get_Flags (flags, 2, eos,                               "last page of logical bitstream (eos)");
    Get_L8 (absolute_granule_position,                          "absolute granule position");
    Get_L4 (stream_serial_number,                               "stream serial number");
    Get_L4 (page_sequence_no,                                   "page sequence no");
    Skip_L4(                                                    "page checksum");
    Get_L1 (page_segments,                                      "page_segments");
    total_page_size=0;
    Chunk_Sizes.clear();
    Chunk_Sizes.push_back(0);
    for (int8u Pos=0; Pos<page_segments; Pos++)
    {
        Get_L1 (packet_lacing_value,                            "packet lacing value");
        total_page_size+=packet_lacing_value;
        Chunk_Sizes[Chunk_Sizes.size()-1]+=packet_lacing_value;
        if (packet_lacing_value!=0xFF)
        {
            Chunk_Sizes.push_back(0);
            Chunk_Sizes_Finished=true;
        }
        else
            Chunk_Sizes_Finished=false;
    }
    if (Chunk_Sizes_Finished)
        Chunk_Sizes.resize(Chunk_Sizes.size()-1); //Keep out the last value

    //Filling
    Header_Fill_Size(27+page_segments+total_page_size);
    Header_Fill_Code(stream_serial_number, Ztring::ToZtring(stream_serial_number, 16));
    Stream[stream_serial_number].absolute_granule_position=absolute_granule_position;
}

//---------------------------------------------------------------------------
void File_Ogg::Data_Parse()
{
    //Counting
    Frame_Count++;

    //If first chunk of a stream
    stream& Stream_Item=Stream[Element_Code]; //[+] FlylinkDC++ Team
    if (Stream_Item.Parser==NULL)
    {
        if (Parsing_End)
            return; //Maybe multitracks concatained, not supported
        Stream_Item.Parser=new File_Ogg_SubElement;
        Open_Buffer_Init(Stream_Item.Parser);
        ((File_Ogg_SubElement*)Stream_Item.Parser)->InAnotherContainer=IsSub;
        StreamsToDo++;
    }
    ((File_Ogg_SubElement*)Stream_Item.Parser)->MultipleStreams=Stream.size()>1; //has no sens for the first init, must check allways

    //Parsing
    File_Ogg_SubElement* Parser=(File_Ogg_SubElement*)Stream_Item.Parser;
    if (Stream_Item.SearchingPayload)
        //For each chunk
        for (size_t Chunk_Sizes_Pos=0; Chunk_Sizes_Pos<Chunk_Sizes.size(); Chunk_Sizes_Pos++)
        {
            //Info
            if (!continued)
                Peek_L1(packet_type); //Only for information
            Element_Info1(Ztring::ToZtring(packet_type, 16));
            Element_Info1C((continued), "Continue");

            //Parsing
            if (continued || Parser->File_Offset!=Parser->File_Size)
                Open_Buffer_Continue(Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, Chunk_Sizes[Chunk_Sizes_Pos]);
            if (Chunk_Sizes_Pos<Chunk_Sizes.size()-1
             || (Chunk_Sizes_Pos==Chunk_Sizes.size()-1 && Chunk_Sizes_Finished))
            {
                Open_Buffer_Continue(Parser, Buffer+Buffer_Offset, 0); //Purge old datas
            }

            Element_Offset+=Chunk_Sizes[Chunk_Sizes_Pos];
            continued=false; //If there is another chunk, this can not be a continued chunk
            if (Parser->File_GoTo!=(int64u)-1)
                Chunk_Sizes_Pos=Chunk_Sizes.size();

            if (!Status[IsAccepted] && Parser->Status[IsAccepted])
                Accept("OGG");
            if (Parser->Status[IsFinished] || (Element_Offset==Element_Size && eos))
            {
                StreamsToDo--;
                Stream_Item.SearchingPayload=false;
                break;
            }
        }
    else
        Skip_XX(Element_Size,                                   "Data");

    //End of stream
    if (!Parsing_End &&
        (StreamsToDo==0 || File_Offset+Buffer_Offset+Element_Offset>256*1024))
    {
        if (IsSub)
            Finish("OGG");
        else
            GoToFromEnd(256*1024, "OGG");
        std::map<int64u, stream>::iterator Stream_Temp=Stream.begin();
        if (File_GoTo!=(int64u)-1)
            while (Stream_Temp!=Stream.end())
            {
                Stream_Temp->second.absolute_granule_position=0;
                ++Stream_Temp;
            }
        Parsing_End=true;
    }

    Element_Show();
}

} //NameSpace

#endif //MEDIAINFO_OGG_YES
