/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Lxf files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_LxfH
#define MediaInfo_File_LxfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include <MediaInfo/Multiple/File_Ancillary.h>
#endif //defined(MEDIAINFO_ANCILLARY_YES)
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Lxf
//***************************************************************************

class File_Lxf : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Lxf();
    ~File_Lxf();

protected :
    //Streams management
    void Streams_Fill ();
    void Streams_Fill_PerStream (File__Analyze* Parser, stream_t Container_StreamKind, size_t Parser_Pos, int8u Format=(int8u)-1);
    void Streams_Finish ();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Global
    void Read_Buffer_Continue();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    bool Header_Begin ();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Header();
    void Header_Info();
    void Header_Meta();
    void Audio();
    void Audio_Stream(size_t Pos);
    void Video();
    void Video_Stream(size_t Pos);
    void Video_Stream_1();
    void Video_Stream_2();

    //Streams
    struct stream
    {
        std::vector<File__Analyze*> Parsers;
        int64u                      BytesPerFrame;
        int8u                       Format;
        bool                        IsFilled;

        stream()
        {
            BytesPerFrame=(int64u)-1;
            Format=(int8u)-1;
            IsFilled=false;
        }
    };
    typedef std::vector<stream> streams;
    streams Videos;
    streams Audios;
    struct stream_header
    {
        int64u          TimeStamp_Begin;
        int64u          TimeStamp_End;
        int64u          Duration;
        int64u          Duration_First;
        int8u           PictureType;

        stream_header()
        {
            TimeStamp_Begin=(int64u)-1;
            TimeStamp_End=(int64u)-1;
            Duration=(int64u)-1;
            Duration_First=(int64u)-1;
            PictureType=(int8u)-1;
        }
        stream_header(int64u TimeStamp_Begin_, int64u TimeStamp_End_, int64u Duration_, int8u PictureType_)
        {
            TimeStamp_Begin=TimeStamp_Begin_;
            TimeStamp_End=TimeStamp_End_;
            Duration=Duration_;
            Duration_First = (int64u)-1;
            PictureType = PictureType_;
        }
    };
    stream_header Videos_Header;
    stream_header Audios_Header;
    #if defined(MEDIAINFO_ANCILLARY_YES)
        File_Ancillary* Ancillary;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)

    //Temp
    bool                    LookingForLastFrame;
    int64u                  Stream_Count;
    int64u                  Info_General_StreamSize;
    std::vector<int64u>     Header_Sizes;
    std::vector<int64u>     Audio_Sizes;
    size_t                  Audio_Sizes_Pos;
    std::vector<int64u>     Video_Sizes;
    size_t                  Video_Sizes_Pos;
    int8u                   SampleSize;
    int32u                  Version;

    //Hints
    size_t*                 File_Buffer_Size_Hint_Pointer;

    //Demux
    #if MEDIAINFO_DEMUX
        File__Analyze* DemuxParser;
    #endif //MEDIAINFO_DEMUX

    //Seek
    typedef std::map<int64u, stream_header> time_offsets;
    time_offsets            TimeOffsets;
    #if MEDIAINFO_SEEK
        int64u              SeekRequest_Divider;
        int64u              SeekRequest;
    #endif //MEDIAINFO_SEEK
    float64                 FrameRate;
    float64                 TimeStamp_Rate;
    bool                    Duration_Detected;
    int64u                  LastAudio_BufferOffset;
    stream_header           LastAudio_TimeOffset;
};

} //NameSpace

#endif


