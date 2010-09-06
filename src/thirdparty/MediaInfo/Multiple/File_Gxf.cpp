// File_Gxf - Info for GXF (SMPTE 360M) files
// Copyright (C) 2010-2010 MediaArea.net SARL, Info@MediaArea.net
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

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_GXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Gxf.h"
#include "MediaInfo/Multiple/File_Gxf_TimeCode.h"
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
#if defined(MEDIAINFO_GXF_YES)
    #include "MediaInfo/Multiple/File_Umf.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* Gxf_Tag_Name(int8u Tag)
{
    switch (Tag)
    {
        case 0x40 : return "Media file name of material";
        case 0x41 : return "First field of material in stream";
        case 0x42 : return "Last field of material in stream";
        case 0x43 : return "Mark in for the stream";
        case 0x44 : return "Mark out for the stream";
        case 0x45 : return "Estimated size of stream in 1024 byte units";
        case 0x46 :
        case 0x47 :
        case 0x48 :
        case 0x49 :
        case 0x4A :
        case 0x4B : return "Reserved";
        case 0x4C : return "Media file name";
        case 0x4D : return "Auxiliary Information";
        case 0x4E : return "Media file system version";
        case 0x4F : return "MPEG auxiliary information";
        case 0x50 : return "Frame rate";
        case 0x51 : return "Lines per frame";
        case 0x52 : return "Fields per frame";
        default   : return "Unknown";
    }
}

//---------------------------------------------------------------------------
const char* Gxf_MediaTypes(int8u Type)
{
    switch (Type)
    {
        case  3 : return "M-JPEG"; //525 lines
        case  4 : return "M-JPEG"; //625 lines
        case  7 : return "SMPTE 12M"; //525 lines
        case  8 : return "SMPTE 12M"; //625 lines
        case  9 : return "PCM"; //24-bit
        case 10 : return "PCM"; //16-bit
        case 11 : return "MPEG-2 Video"; //525 lines
        case 12 : return "MPEG-2 Video"; //625 lines
        case 13 : return "DV"; //25 Mbps, 525 lines
        case 14 : return "DV"; //25 Mbps, 625 lines
        case 15 : return "DV"; //50 Mbps, 525 lines
        case 16 : return "DV"; //50 Mbps, 625 lines
        case 17 : return "AC-3"; //16-bit
        case 18 : return "AES"; //non-PCM
        case 19 : return "Reserved";
        case 20 : return "MPEG-2 Video"; //HD, Main Profile at High Level
        case 21 : return "Ancillary data"; //SMPTE 291M 10-bit type 2 component ancillary data
        case 22 : return "MPEG-1 Video"; //525 lines
        case 23 : return "MPEG-1 Video"; //625 lines
        case 24 : return "SMPTE 12M"; //HD
        default : return "Unknown";
    }
}

//---------------------------------------------------------------------------
stream_t Gxf_MediaTypes_StreamKind(int8u Type)
{
    switch (Type)
    {
        case  3 : return Stream_Video;
        case  4 : return Stream_Video;
        case  7 : return Stream_Max;
        case  8 : return Stream_Max;
        case  9 : return Stream_Audio;
        case 10 : return Stream_Audio;
        case 11 : return Stream_Video;
        case 12 : return Stream_Video;
        case 13 : return Stream_Video;
        case 14 : return Stream_Video;
        case 15 : return Stream_Video;
        case 16 : return Stream_Video;
        case 17 : return Stream_Audio;
        case 18 : return Stream_Audio;
        case 19 : return Stream_Max;
        case 20 : return Stream_Video;
        case 21 : return Stream_Max;
        case 22 : return Stream_Video;
        case 23 : return Stream_Video;
        case 24 : return Stream_Max;
        default : return Stream_Max;
    }
}

//---------------------------------------------------------------------------
const char* Gxf_MediaTypes_Format(int8u Type)
{
    switch (Type)
    {
        case  3 : return "JPEG"; //525 lines
        case  4 : return "JPEG"; //625 lines
        case  9 : return "PCM"; //24-bit
        case 10 : return "PCM"; //16-bit
        case 11 : return "MPEG Video"; //525 lines
        case 12 : return "MPEG Video"; //625 lines
        case 13 : return "DV"; //25 Mbps, 525 lines
        case 14 : return "DV"; //25 Mbps, 625 lines
        case 15 : return "DV"; //50 Mbps, 525 lines
        case 16 : return "DV"; //50 Mbps, 625 lines
        case 17 : return "AC-3"; //16-bit
        case 18 : return "SMPTE 338M, table 1, data type 28"; //SMPTE 338M, table 1, data type 28
        case 20 : return "MPEG Video"; //HD, Main Profile at High Level
        case 22 : return "MPEG Video"; //525 lines
        case 23 : return "MPEG Video"; //625 lines
        default : return "";
    }
}

//---------------------------------------------------------------------------
double Gxf_FrameRate(int32u Content)
{
    switch (Content)
    {
        case 1 : return 60.000;
        case 2 : return 59.940;
        case 3 : return 50.000;
        case 4 : return 30.000;
        case 5 : return 29.970;
        case 6 : return 25.000;
        case 7 : return 24.000;
        case 8 : return 23.976;
        default: return  0.000;
    }
}

//---------------------------------------------------------------------------
int32u Gxf_LinesPerFrame_Height(int32u Content)
{
    switch (Content)
    {
        case 1 : return  480;
        case 2 : return  576;
        case 4 : return 1080;
        case 6 : return  720;
        default: return    0;
    }
}

//---------------------------------------------------------------------------
int32u Gxf_LinesPerFrame_Width(int32u Content)
{
    switch (Content)
    {
        case 1 : return  720;
        case 2 : return  720;
        case 4 : return 1920;
        case 6 : return 1080;
        default: return    0;
    }
}

//---------------------------------------------------------------------------
const char* Gxf_FieldsPerFrame(int32u Tag)
{
    switch (Tag)
    {
        case 1 : return "Progressive";
        case 2 : return "Interlaced";
        default: return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Gxf::File_Gxf()
:File__Analyze()
{
    //Configuration
    ParserName=_T("GXF");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Gxf;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //Temp
    Material_Fields_FieldsPerFrame=(int8u)-1;
    Parsers_Count=0;
    AncillaryData_StreamID=(int8u)-1;
    TimeCode_StreamID=(int8u)-1;
    Material_Fields_First_IsValid=false;
    Material_Fields_Last_IsValid=false;
    Material_File_Size_IsValid=false;
    UMF_File=NULL;
    #if defined(MEDIAINFO_ANCILLARY_YES)
        Ancillary=NULL;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)
    SizeToAnalyze=16*1024*1024;
    TimeCode_First=(int64u)-1;
}

//---------------------------------------------------------------------------
File_Gxf::~File_Gxf()
{
    //Temp
    delete Ancillary; //Ancillary=NULL;
    delete UMF_File; //UMF_File=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf::Streams_Finish()
{
    //TimeCode
    if (TimeCode_First==(int64u)-1 && TimeCode_StreamID!=(int8u)-1 && Streams[TimeCode_StreamID].Parser)
        TimeCode_First=Streams[TimeCode_StreamID].Parser->Retrieve(Stream_Video, 0, Video_Delay).To_int64u();

    //Merging audio if Title are same
    for (size_t StreamID=0; StreamID<Streams.size(); StreamID++)
    {
        if (Gxf_MediaTypes_StreamKind(Streams[StreamID].MediaType)==Stream_Video)
        {
            Ztring Title=Streams[StreamID].MediaName;
            size_t Title_Extension_Offset=Title.find(_T(".M0"));
            if (Title_Extension_Offset==std::string::npos || Title_Extension_Offset!=Title.size()-3)
                Title_Extension_Offset=Title.find(_T(".H0"));
            if (Title_Extension_Offset!=std::string::npos && Title_Extension_Offset==Title.size()-3)
            {
                Title.resize(Title.size()-3);
                Streams[StreamID].MediaName=Title;
            }
        }
        if (Gxf_MediaTypes_StreamKind(Streams[StreamID].MediaType)==Stream_Audio && Config->File_Audio_MergeMonoStreams_Get())
        {
            Ztring Title=Streams[StreamID].MediaName;
            size_t Title_Extension_Offset=Title.find(_T(".A0"));
            if (Title_Extension_Offset!=std::string::npos && Title_Extension_Offset==Title.size()-3)
            {
                Title.resize(Title.size()-3);
                for (size_t StreamID2=StreamID+1; StreamID2<Streams.size(); StreamID2++)
                {
                    if (Streams[StreamID2].MediaName==Title+_T(".A")+Ztring::ToZtring(StreamID2-StreamID))
                    {
                        Streams[StreamID].MediaName=Title;
                        if (Streams[StreamID].Parser && Streams[StreamID2].Parser)
                        {
                            int32u Channels=Streams[StreamID].Parser->Retrieve(Stream_Audio, 0, Audio_Channel_s_).To_int32u()+Streams[StreamID2].Parser->Retrieve(Stream_Audio, 0, Audio_Channel_s_).To_int32u();
                            Streams[StreamID].Parser->Fill(Stream_Audio, 0, Audio_Channel_s_, Channels, 10, true);
                            int32u BitRate=Streams[StreamID].Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int32u()+Streams[StreamID2].Parser->Retrieve(Stream_Audio, 0, Audio_BitRate).To_int32u();
                            Streams[StreamID].Parser->Fill(Stream_Audio, 0, Audio_BitRate, BitRate, 10, true);
                        }
                        Streams[StreamID2].MediaType=(int8u)-1;
                    }
                }
            }
        }
    }

    //For each Streams
    for (size_t StreamID=0; StreamID<Streams.size(); StreamID++)
        Streams_Finish_PerStream(StreamID, Streams[StreamID]);

    //Global
    if (Material_Fields_First_IsValid && Material_Fields_Last_IsValid && Material_Fields_FieldsPerFrame!=(int8u)-1 && Material_Fields_Last-Material_Fields_First)
    {
        Fill(Stream_Video, 0, Video_FrameCount, (Material_Fields_Last+1-Material_Fields_First)/(Material_Fields_FieldsPerFrame==2?2:1));

        //We trust more the MPEG Video bitrate thant the rest
        //TODO: Chech why there is incohenrency (mainly about Material File size info in the sample)
        if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video"))
            Fill(Stream_Video, 0, Video_BitRate, Retrieve(Stream_Video, 0, Video_BitRate_Nominal));
    }
    if (Material_File_Size_IsValid)
    {
        //Fill(Stream_General, 0, General_OverallBitRate, ((int64u)Material_File_Size)*1024*8/???);
    }
}

//---------------------------------------------------------------------------
void File_Gxf::Streams_Finish_PerStream(size_t StreamID, stream &Temp)
{
    if (Temp.MediaType==(int8u)-1)
        return;

    //By the parser
    if (Temp.Parser && Temp.Parser->Status[IsFilled])
    {
        StreamKind_Last=Stream_Max;
        StreamPos_Last=(size_t)-1;
        Finish(Temp.Parser);

        //Video
        if (Temp.Parser->Count_Get(Stream_Video) && StreamID!=TimeCode_StreamID)
        {
            Stream_Prepare(Stream_Video);

            if (TimeCode_First!=(int64u)-1)
            {
                Fill(Stream_Video, StreamPos_Last, Video_Delay, TimeCode_First, 0, true);
                Fill(Stream_Video, StreamPos_Last, Video_Delay_Source, "Container");
            }

            Merge(*Temp.Parser, Stream_Video, 0, StreamPos_Last);

            //Special cases
            if (Temp.Parser->Count_Get(Stream_Text))
            {
                //Video and Text are together
                size_t Parser_Text_Count=Temp.Parser->Count_Get(Stream_Text);
                for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
                {
                    Stream_Prepare(Stream_Text);
                    Merge(*Temp.Parser, Stream_Text, Parser_Text_Pos, StreamPos_Last);
                    Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                    Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring(AncillaryData_StreamID)+_T("-")+ID, true);
                    Fill(Stream_Text, StreamPos_Last, Text_ID_String, Ztring::ToZtring(AncillaryData_StreamID)+_T("-")+ID, true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Source), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay_Original, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Delay_Original_Source, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Delay_Original_Source), true);
                }

                StreamKind_Last=Stream_Video;
                StreamPos_Last=Count_Get(Stream_Video)-1;
            }
        }

        //Audio
        if (Temp.Parser->Count_Get(Stream_Audio) && StreamID!=TimeCode_StreamID)
        {
            Stream_Prepare(Stream_Audio);

            if (TimeCode_First!=(int64u)-1)
            {
                Fill(Stream_Audio, StreamPos_Last, Audio_Delay, TimeCode_First, 0, true);
                Fill(Stream_Audio, StreamPos_Last, Audio_Delay_Source, "Container");
            }

            Merge(*Temp.Parser, Stream_Audio, 0, StreamPos_Last);
        }

        //Metadata
        if (StreamKind_Last!=Stream_Max)
        {
            Fill(StreamKind_Last, StreamPos_Last, General_ID, StreamID, 10, true);
            Fill(StreamKind_Last, StreamPos_Last, "Title", Temp.MediaName);
        }
    }
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Gxf::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+16<=Buffer_Size)
    {
        while (Buffer_Offset+16<=Buffer_Size)
        {
            if (CC5(Buffer+Buffer_Offset   )==0x0000000001
             && CC2(Buffer+Buffer_Offset+14)==0xE1E2)
                break;
            Buffer_Offset++;
        }

        if (Buffer_Offset+16<=Buffer_Size) //Testing if size is coherant
        {
            //Retrieving some info
            int32u Size=CC4(Buffer+Buffer_Offset+6);

            //Testing
            if (Buffer_Offset+Size+16>Buffer_Size)
                return false; //Need more data
            if (CC5(Buffer+Buffer_Offset+Size   )!=0x0000000001
             || CC2(Buffer+Buffer_Offset+Size+14)!=0xE1E2)
                Buffer_Offset++;
            else
                break;
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+16>Buffer_Size)
    {
        return false;
    }
    
    if (!Status[IsAccepted])
    {
        Accept("GXF");
        Fill(Stream_General, 0, General_Format, "GXF");
        Streams.resize(0x40);
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Gxf::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC5(Buffer+Buffer_Offset   )!=0x0000000001
     || CC2(Buffer+Buffer_Offset+14)!=0xE1E2)
        Synched=false;

    //Test if the next synchro is available
    int32u PacketLength=BigEndian2int32u(Buffer+Buffer_Offset+6);
    if (File_Offset+Buffer_Offset+PacketLength+16<=File_Size)
    {
        if (Buffer_Offset+PacketLength+16>Buffer_Size)
            return false;
        if (CC5(Buffer+Buffer_Offset+PacketLength   )!=0x0000000001
         || CC2(Buffer+Buffer_Offset+PacketLength+14)!=0xE1E2)
            Synched=false;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf::Header_Parse()
{
    //Parsing
    int32u PacketLength;
    int8u  PacketType;
    Skip_B5(                                                    "Packet leader");
    Get_B1 (PacketType,                                         "Packet type");
    Get_B4 (PacketLength,                                       "Packet length");
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Packet trailer");

    //Filling
    Header_Fill_Size(PacketLength);
    Header_Fill_Code(PacketType);
}

//---------------------------------------------------------------------------
void File_Gxf::Data_Parse()
{
    //Counting
    Frame_Count++;

    switch (Element_Code)
    {
        case 0x00 : Finish("GXF"); break;
        case 0xBC : map(); break;
        case 0xBF : media(); break;
        case 0xFB : end_of_stream(); break;
        case 0xFC : field_locator_table(); break;
        case 0xFD : UMF_file(); break;
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Gxf::map()
{
    Element_Name("map");

    //Parsing
    int8u Version;
    Element_Begin("Preamble");
        BS_Begin();
        Mark_1();
        Mark_1();
        Mark_1();
        Get_S1(5, Version,                                      "Version");
        BS_End();
        Skip_B1(                                                "Reserved");
    Element_End();

    Element_Begin("Material Data");
        int16u SectionLength;
        Get_B2 (SectionLength,                                  "Section Length");
        if (Element_Offset+SectionLength>=Element_Size)
            SectionLength=(int16u)(Element_Size-Element_Offset);
        int64u Material_Data_End=Element_Offset+SectionLength;
        while (Element_Offset<Material_Data_End)
        {
            Element_Begin("Tag");
            int8u Tag, DataLength;
            Get_B1(Tag,                                         "Tag");
            Get_B1(DataLength,                                  "Data Length");
            Element_Name(Gxf_Tag_Name(Tag));
            switch (Tag)
            {
                case 0x40 : //Media file name of material
                            {
                            Ztring MediaFileName;
                            Get_Local(DataLength, MediaFileName, "Content");
                            Fill(Stream_General, 0, General_Title, MediaFileName, true);
                            }
                            break;
                case 0x41 : //First field of material in stream
                            if (DataLength==4)
                            {
                                Get_B4 (Material_Fields_First,  "Content");
                                Material_Fields_First_IsValid=true;
                            }
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x42 : //Last field of material in stream
                            if (DataLength==4)
                            {
                                Get_B4 (Material_Fields_Last,   "Content");
                                Material_Fields_Last_IsValid=true;
                            }
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x43 : //Mark in for the stream
                            if (DataLength==4)
                                Skip_B4(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x44 : //Mark out for the stream
                            if (DataLength==4)
                                Skip_B4(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x45 : //Estimated size of stream in 1024 byte units
                            if (DataLength==4)
                            {
                                Get_B4 (Material_File_Size  ,   "Content");
                                Material_File_Size_IsValid=true;
                            }
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x46 : //Reserved
                            if (DataLength==4)
                                Skip_B4(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x47 : //Reserved
                            if (DataLength==8)
                                Skip_B8(                        "Content");
                            else
                                Skip_XX(DataLength,             "Unknown");
                            break;
                case 0x48 : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                case 0x49 : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                case 0x4A : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                case 0x4B : //Reserved
                            Skip_String(DataLength,             "Content");
                            break;
                default   : Skip_XX(DataLength,                 "Unknown");
            }
            Element_End();
        }
    Element_End();

    Element_Begin("Track Description");
        Get_B2 (SectionLength,                                  "Section Length");
        if (Element_Offset+SectionLength>=Element_Size)
            SectionLength=(int16u)(Element_Size-Element_Offset);
        int64u Track_Data_End=Element_Offset+SectionLength;
        while (Element_Offset<Track_Data_End)
        {
            Element_Begin("Track");
            int16u TrackLength;
            int8u  MediaType, TrackID;
            Get_B1 (MediaType,                                  "Media type"); Param_Info(Gxf_MediaTypes(MediaType&0x7F));
            Get_B1 (TrackID,                                    "Track ID");
            Get_B2 (TrackLength,                                "Track Length");
            if (Element_Offset+TrackLength>=Track_Data_End)
                TrackLength=(int16u)(Track_Data_End-Element_Offset);
            int64u Track_End=Element_Offset+TrackLength;
            Element_Info(TrackID&0x3F);
            Element_Info(Gxf_MediaTypes(MediaType&0x7F));

            FILLING_BEGIN();
                MediaType&=0x7F; //Remove the last bit
                TrackID&=0x3F; //Remove the 2 last bits
                if (Streams[TrackID].Parser==NULL)
                {
                    Streams[TrackID].MediaType=MediaType;
                    Streams[TrackID].TrackID=TrackID;

                    //Parsers
                    #if MEDIAINFO_DEMUX
                        Element_Code=TrackID;
                    #endif //MEDIAINFO_DEMUX
                    switch (MediaType)
                    {
                        case  3 :
                        case  4 :  //M-JPEG
                                    Streams[TrackID].Parser=new File__Analyze; //Filling with following data
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Streams[TrackID].Parser->Accept();
                                    Streams[TrackID].Parser->Fill();
                                    Streams[TrackID].Parser->Stream_Prepare(Stream_Video);
                                    Streams[TrackID].Parser->Fill(Stream_Video, 0, Video_Format, "M-JPEG");
                                    break;
                        case  7 :
                        case  8 :
                        case 24 :  //TimeCode
                                    Streams[TrackID].Parser=new File_Gxf_TimeCode;
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Parsers_Count++;
                                    Streams[TrackID].Searching_Payload=true;
                                    TimeCode_StreamID=TrackID;
                                    break;
                        case  9 :
                        case 10 :
                        case 18 :  //PCM
                                    Streams[TrackID].Parser=new File__Analyze; //Filling with following data
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Streams[TrackID].Parser->Accept();
                                    Streams[TrackID].Parser->Fill();
                                    Streams[TrackID].Parser->Stream_Prepare(Stream_Audio);
                                    Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_Format, "PCM");
                                    break;
                        case 11 :
                        case 12 :
                        case 20 :
                        case 22 :
                        case 23 :   //MPEG Video
                                    Streams[TrackID].Parser=new File_Mpegv();
                                    ((File_Mpegv*)Streams[TrackID].Parser)->Ancillary=&Ancillary;
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Parsers_Count++;
                                    Streams[TrackID].Searching_Payload=true;
                                    break;
                        case 13 :
                        case 14 :
                        case 15 :
                        case 16 :   //DV
                                    Streams[TrackID].Parser=new File__Analyze; //Filling with following data
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Streams[TrackID].Parser->Accept();
                                    Streams[TrackID].Parser->Fill();
                                    Streams[TrackID].Parser->Stream_Prepare(Stream_Video);
                                    Streams[TrackID].Parser->Fill(Stream_Video, 0, Video_Format, "DV");
                                    break;
                        case 17 :   //AC-3
                                    Streams[TrackID].Parser=new File__Analyze; //Filling with following data
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Streams[TrackID].Parser->Accept();
                                    Streams[TrackID].Parser->Fill();
                                    Streams[TrackID].Parser->Stream_Prepare(Stream_Audio);
                                    Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_Format, "AC-3");
                                    break;
                        case 21 :   //Ancillary Metadata
                                    Streams[TrackID].Parser=new File_Riff();
                                    ((File_Riff*)Streams[TrackID].Parser)->Ancillary=&Ancillary;
                                    Open_Buffer_Init(Streams[TrackID].Parser);
                                    Parsers_Count++;
                                    Streams[TrackID].Searching_Payload=true;
                                    Ancillary=new File_Ancillary;
                                    Ancillary->WithTenBit=true;
                                    Ancillary->WithChecksum=true;
                                    Open_Buffer_Init(Ancillary);
                                    AncillaryData_StreamID=TrackID;
                                    if (SizeToAnalyze<8*16*1024*1024)
                                        SizeToAnalyze*=8; //10x more, to be sure to find captions
                                    break;
                        default :   ;
                    }

                    if (Gxf_MediaTypes_StreamKind(MediaType)==Stream_Audio)
                    {
                        //Resolution
                        switch (MediaType)
                        {
                            case  9 :
                            case 18 :   //24-bit
                                        Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_Resolution, 24);
                                        break;
                            case 10 :
                            case 17 :   //16-bit
                                        Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_Resolution, 16);
                                        break;
                            default : ;
                        }

                        //Channels
                        switch (MediaType)
                        {
                            case  9 :
                            case 10 :   //Mono
                                        Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_Channel_s_, 1);
                                        break;
                            default : ;
                        }

                        //Sampling rate
                        switch (MediaType)
                        {
                            case  9 :
                            case 10 :
                            case 17 :   //48000
                                        Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_SamplingRate, 48000);
                                        break;
                            default : ;
                        }

                        //Bit rate
                        switch (MediaType)
                        {
                            case  9 :
                            case 17 :   //Mono, 48 KHz, 24-bit (or padded up to 24-bit)
                                        Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_BitRate, 1*48000*24);
                                        break;
                            case 10 :   //Mono, 48 KHz, 16-bit
                                        Streams[TrackID].Parser->Fill(Stream_Audio, 0, Audio_BitRate, 1*48000*16);
                                        break;
                            default : ;
                        }
                    }
                }
            FILLING_END();

            int8u Hours=(int8u)-1, Minutes=(int8u)-1, Seconds=(int8u)-1, Frames=(int8u)-1;
            bool  Invalid, DropFrame=true;
            bool  TimeCode_Parsed=false;

            while (Element_Offset<Track_End)
            {
                Element_Begin("Tag");
                int8u Tag, DataLength;
                Get_B1(Tag,                                     "Tag");
                Get_B1(DataLength,                              "Data Length");
                Element_Name(Gxf_Tag_Name(Tag));
                switch (Tag)
                {
                    case 0x4C : //Media name
                                {
                                    Get_Local(DataLength, Streams[TrackID].MediaName, "Content");
                                }
                                break;
                    case 0x4D : //Auxiliary Information
                                if (DataLength==8)
                                {
                                    if (MediaType==21)
                                    {
                                        //Ancillary
                                        Skip_B1(                "Reserved");
                                        Skip_B1(                "Reserved");
                                        Skip_B1(                "Ancillary data presentation format");
                                        Skip_B1(                "Number of ancillary data fields per ancillary data media packet");
                                        Skip_B2(                "Byte size of each ancillary data field");
                                        Skip_B2(                "Byte size of the ancillary data media packet in 256 byte units");
                                    }
                                    if (MediaType==7 || MediaType==8 || MediaType==24)
                                    {
                                        //TimeCode
                                        Get_B1 (Frames,         "Frame");
                                        Get_B1 (Seconds,        "Second");
                                        Get_B1 (Minutes,        "Minute");
                                        BS_Begin();
                                        Get_SB (   Invalid,     "Invalid");
                                        Skip_SB(                "Color frame");
                                        Get_SB (   DropFrame,   "Drop frame");
                                        Get_S1 (5, Hours,       "Hour");
                                        BS_End();
                                        Skip_B1(                "User bits");
                                        Skip_B1(                "User bits");
                                        Skip_B1(                "User bits");
                                        Skip_B1(                "User bits");
                                        TimeCode_Parsed=true;
                                    }
                                    else
                                        Skip_B8(                "Content");
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x4E : //Media file system version
                                if (DataLength==4)
                                    Skip_B4(                    "Content");
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x4F : //MPEG auxiliary information
                                Skip_String(DataLength,         "Content");
                                break;
                    case 0x50 : //Frame rate
                                if (DataLength==4)
                                {
                                    Get_B4 (Streams[TrackID].FrameRate_Code, "Content"); Param_Info(Gxf_FrameRate(Streams[TrackID].FrameRate_Code)); Element_Info(Gxf_FrameRate(Streams[TrackID].FrameRate_Code));
                                    if (TrackID==TimeCode_StreamID)
                                        ((File_Gxf_TimeCode*)Streams[TrackID].Parser)->FrameRate_Code=Streams[0x00].FrameRate_Code;
                                   }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x51 : //Lines per frame
                                if (DataLength==4)
                                {
                                    Get_B4 (Streams[TrackID].LinesPerFrame_Code, "Content"); Param_Info(Gxf_LinesPerFrame_Height(Streams[TrackID].LinesPerFrame_Code)); Element_Info(Gxf_LinesPerFrame_Height(Streams[TrackID].LinesPerFrame_Code));
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    case 0x52 : //Fields per frame
                                if (DataLength==4)
                                {
                                    Get_B4 (Streams[TrackID].FieldsPerFrame_Code, "Content"); Param_Info(Gxf_FieldsPerFrame(Streams[TrackID].FieldsPerFrame_Code)); Element_Info(Gxf_FieldsPerFrame(Streams[TrackID].FieldsPerFrame_Code));
                                    if (Gxf_MediaTypes_StreamKind(MediaType)==Stream_Video)
                                        Material_Fields_FieldsPerFrame=Streams[TrackID].FieldsPerFrame_Code;
                                    if (TrackID==TimeCode_StreamID)
                                        ((File_Gxf_TimeCode*)Streams[TrackID].Parser)->FieldsPerFrame_Code=Streams[0x00].FieldsPerFrame_Code;
                                }
                                else
                                    Skip_XX(DataLength,         "Unknown");
                                break;
                    default   : Skip_XX(DataLength,             "Unknown");
                }
                Element_End();
            }
            Element_End();

            //Test on TimeCode
            if (TimeCode_Parsed)
            {
                if (!Invalid && TimeCode_First==(int64u)-1)
                {
                    float32 FrameRate=Gxf_FrameRate(Streams[TrackID].FrameRate_Code);
                    TimeCode_First=Hours  *60*60*1000
                                  +Minutes   *60*1000
                                  +Seconds      *1000
                                  +float32_int64s(Frames*1000/FrameRate);
                }
            }
        }
    Element_End();
    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding");
}

//---------------------------------------------------------------------------
void File_Gxf::media()
{
    Element_Name("media");

    //Parsing
    int8u TrackNumber;
    Element_Begin("Preamble");
        Skip_B1(                                                "Media type");
        Get_B1 (TrackNumber,                                    "Track number");
        Skip_B4(                                                "Media field number");
        Skip_B1(                                                "Field information");
        Skip_B1(                                                "Field information");
        Skip_B1(                                                "Field information");
        Skip_B1(                                                "Field information");
        Skip_B4(                                                "Time line field number");
        Skip_B1(                                                "Flags");
        Skip_B1(                                                "Reserved");
        TrackNumber&=0x3F;
    Element_End();
    Element_Info(TrackNumber);

    #if MEDIAINFO_DEMUX
        Element_Code=TrackNumber;
        Demux(Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset), ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Needed?
    if (!Streams[TrackNumber].Searching_Payload)
    {
        Skip_XX(Element_Size,                                   "data");
        Element_DoNotShow();
        return;
    }

    if (Streams[TrackNumber].Parser)
    {
        Open_Buffer_Continue(Streams[TrackNumber].Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        if (MediaInfoLib::Config.ParseSpeed_Get()<1 && Streams[TrackNumber].Parser->Status[IsFilled])
        {
            Streams[TrackNumber].Searching_Payload=false;

            if (Parsers_Count>0)
                Parsers_Count--;
            if (Parsers_Count==0)
            {
                Finish();
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Gxf::end_of_stream()
{
    Element_Name("end of stream");
}

//---------------------------------------------------------------------------
void File_Gxf::field_locator_table()
{
    Element_Name("field locator table");

    //Parsing
    int32u Entries;
    Skip_L4(                                                    "Number of fields per FLT entry");
    Get_L4 (Entries,                                            "Number of FLT entries");
    for (size_t Pos=0; Pos<Entries; Pos++)
    {
        Skip_L4(                                                "Offset to fields");
        if (Element_Offset==Element_Size)
            break;
    }
}

//---------------------------------------------------------------------------
void File_Gxf::UMF_file()
{
    Element_Name("UMF file");

    //Parsing
    int32u PayloadDataLength;
    Element_Begin("Preamble");
        Skip_B1(                                                "First/last packet flag");
        Get_B4 (PayloadDataLength,                              "Payload data length");
    Element_End();

    if (UMF_File==NULL)
    {
        UMF_File=new File_Umf();
        Open_Buffer_Init(UMF_File);
    }
    Open_Buffer_Continue(UMF_File, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf::Detect_EOF()
{
    if (File_Offset+Buffer_Size>=SizeToAnalyze)
    {
        Finish();
    }
}

} //NameSpace

#endif //MEDIAINFO_GXF_YES
