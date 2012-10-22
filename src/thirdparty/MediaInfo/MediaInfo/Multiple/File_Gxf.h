// File_Gxf - Info for GXF (SMPTE 360M) files
// Copyright (C) 2010-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about GXF files
// SMPTE 360M - General Exchange Format
// SMPTE RDD 14-2007 - General Exchange Format-2
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_GxfH
#define MediaInfo_File_GxfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include <MediaInfo/Multiple/File_Ancillary.h>
#endif //defined(MEDIAINFO_ANCILLARY_YES)
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Gxf
//***************************************************************************

class File_Gxf : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Gxf();
    ~File_Gxf();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Packets
    void map();
    void media();
    void end_of_stream();
    void field_locator_table();
    void UMF_file();

    //Temp - Global
    #if defined(MEDIAINFO_ANCILLARY_YES)
        File_Ancillary* Ancillary;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)
    int32u Material_Fields_First;
    int32u Material_Fields_Last;
    int32u Material_File_Size;
    int32u Material_Fields_FieldsPerFrame;
    int8u  Parsers_Count;
    int8u  AncillaryData_StreamID;
    std::map<int8u, int64u> TimeCodes; //Key is StreamID
    bool   Material_Fields_First_IsValid;
    bool   Material_Fields_Last_IsValid;
    bool   Material_File_Size_IsValid;

    //Temp - Stream
    struct stream
    {
        File__Analyze* Parser;
        int64u FirstFrameDuration; //In case of audio, indicates the duration of the first frame
        stream_t StreamKind;
        size_t StreamPos;
        int32u TimeStamp_Start;
        int32u TimeStamp_End;
        int32u FrameRate_Code;
        int32u LinesPerFrame_Code;
        int32u FieldsPerFrame_Code;
        int8u  MediaType;
        int8u  TrackID;
        bool   Searching_Payload;
        bool   Searching_TimeStamp_Start;
        bool   Searching_TimeStamp_End;
        bool   IsChannelGrouping;
        bool   DisplayInfo; //In case of channel grouping, info is about the complete (2*half) stream, so second stream info must not be used
        Ztring MediaName;
        std::map<std::string, Ztring> Infos;

        stream()
        {
            Parser=NULL;
            FirstFrameDuration=0;
            StreamKind=Stream_Max;
            StreamPos=(size_t)-1;
            Searching_Payload=false;
            Searching_TimeStamp_Start=false;
            Searching_TimeStamp_End=false;
            FrameRate_Code=(int32u)-1;
            LinesPerFrame_Code=(int32u)-1;
            FieldsPerFrame_Code=(int32u)-1;
            MediaType=(int8u)-1;
            TrackID=(int8u)-1;
            IsChannelGrouping=false;
            DisplayInfo=true;
        }
        ~stream()
        {
            delete Parser; //Parser=NULL
        }
    };
    std::vector<stream> Streams;
    File__Analyze*      UMF_File;
    int64u              SizeToAnalyze; //Total size of a chunk to analyse, it may be changed by the parser
    int8u               Audio_Count;
    int8u               TrackNumber;

    //File__Analyze helpers
    void Streams_Finish_PerStream(size_t StreamID, stream &Temp);
    void Detect_EOF();
    File__Analyze* ChooseParser_ChannelGrouping(int8u TrackID);

    #if MEDIAINFO_DEMUX
        bool Demux_HeaderParsed;
    #endif //MEDIAINFO_DEMUX

    #if MEDIAINFO_SEEK
        int32u Flt_FieldPerEntry;
        std::vector<int32u> Flt_Offsets; //In 1024-byte
        struct seek
        {
            int64u FrameNumber;
            int32u StreamOffset; //In 1024-byte
        };
        std::vector<seek> Seeks;
        bool IFrame_IsParsed;
    #endif //MEDIAINFO_SEEK
};

} //NameSpace

#endif
