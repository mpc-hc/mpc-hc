/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_MpegPsH
#define MediaInfo_MpegPsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/Multiple/File_Mpeg4_Descriptors.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg
//***************************************************************************

class File_MpegPs : public File__Analyze
{
public :
    //In
    bool   FromTS;                      //Indicate if stream comes from TS
    int8u  FromTS_stream_type;          //ID from TS
    int32u FromTS_program_format_identifier; //Registration from TS
    int32u FromTS_format_identifier;    //Registration from TS
    int8u  FromTS_descriptor_tag;       //Descriptor from TS
    int8u  MPEG_Version;                //MPEG Version (or automaticly detected)
    bool   Searching_TimeStamp_Start;
    #ifdef MEDIAINFO_MPEG4_YES
        File__Analyze* ParserFromTs;
        File_Mpeg4_Descriptors::slconfig* SLConfig;
    #endif
    #if MEDIAINFO_DEMUX
        struct demux
        {
            struct buffer
            {
                int64u  DTS;
                size_t  Buffer_Size;
                size_t  Buffer_Size_Max;
                int8u*  Buffer;

                buffer()
                {
                    DTS=(int64u)-1;
                    Buffer_Size=0;
                    Buffer_Size_Max=0;
                    Buffer=NULL;
                }

                ~buffer()
                {
                    delete[] Buffer;
                }
            };
            std::vector<buffer*> Buffers;

            demux()
            {
            }

            ~demux()
            {
                for (size_t Pos=0; Pos<Buffers.size(); Pos++)
                    delete Buffers[Pos]; //Buffers[Pos]=NULL;
            }
        };
        demux* SubStream_Demux;
        int8u Demux_StreamIsBeingParsed_type;
        int8u Demux_StreamIsBeingParsed_stream_id;
    #endif //MEDIAINFO_DEMUX
    #if MEDIAINFO_SEEK
        int64u Unsynch_Frame_Count_Temp;
    #endif //MEDIAINFO_SEEK
    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        bool FromAribStdB24B37;
    #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)

    //Out
    bool   HasTimeStamps;

    //Constructor/Destructor
    File_MpegPs();
    ~File_MpegPs();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Update();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin() {return FileHeader_Begin_0x000001();}

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Global
    void Read_Buffer_Init ();
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif //MEDIAINFO_SEEK
    void Read_Buffer_Continue ();
    void Read_Buffer_AfterParsing();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parse_Fill_Size();
    bool Header_Parse_PES_packet(int8u stream_id);
    void Header_Parse_PES_packet_MPEG1(int8u stream_id);
    void Header_Parse_PES_packet_MPEG2(int8u stream_id);
    void Data_Parse();
    bool BookMark_Needed();

    //Packet
    void MPEG_program_end();    //0xB9
    void pack_start();          //0xBA
    void system_header_start(); //0xBB
    void program_stream_map();  //0xBC
    void private_stream_1();    //0xBD
    void padding_stream();      //0xBE
    void private_stream_2();    //0xBF
    void audio_stream();        //0xC0 --> 0xDF
    void video_stream();        //0xE0 --> 0xEF
    void SL_packetized_stream();//0xFA
    void extension_stream();    //0xFD

    //private_stream_1 specific
    bool           private_stream_1_Choose_DVD_ID();
    File__Analyze* private_stream_1_ChooseParser();
    const ZenLib::Char* private_stream_1_ChooseExtension();
    #if MEDIAINFO_TRACE
    void           private_stream_1_Element_Info1();
    #endif //MEDIAINFO_TRACE
    int8u          private_stream_1_ID;
    size_t         private_stream_1_Offset;
    bool           private_stream_1_IsDvdVideo;

    //private_stream_2 specific
    void           private_stream_2_TSHV_A0();
    void           private_stream_2_TSHV_A1();

    //extension_stream specific
    const ZenLib::Char*  extension_stream_ChooseExtension();

    //Count
    int8u video_stream_Count;
    int8u audio_stream_Count;
    int8u private_stream_1_Count;
    int8u private_stream_2_Count;
    int8u extension_stream_Count;
    int8u SL_packetized_stream_Count;

    //From packets
    int32u program_mux_rate;

    //PS
    struct ps_stream
    {
        struct Mpeg_TimeStamp
        {
            struct Mpeg_TimeStamp_TS
            {
                int64u File_Pos;
                int64u TimeStamp;

                Mpeg_TimeStamp_TS()
                {
                    File_Pos=(int64u)-1;
                    TimeStamp=(int64u)-1;
                }
            };

            Mpeg_TimeStamp_TS PTS;
            Mpeg_TimeStamp_TS DTS;
        };

        stream_t       StreamKind;
        size_t         StreamPos;
        int8u          stream_type;
        int32u         program_format_identifier;
        int32u         format_identifier;
        int8u          descriptor_tag;
        int8u          DVD_Identifier;
        std::vector<File__Analyze*> Parsers; //Sometimes, we need to do parallel tests
        Mpeg_TimeStamp TimeStamp_Start;
        Mpeg_TimeStamp TimeStamp_End;
        size_t         StreamIsRegistred;
        size_t         StreamOrder;
        size_t         FirstPacketOrder;
        bool           Searching_Payload;
        bool           Searching_TimeStamp_Start;
        bool           Searching_TimeStamp_End;
        bool           IsFilled;

        ps_stream()
        {
            StreamKind=Stream_Max;
            StreamPos=0;
            stream_type=0;
            program_format_identifier=0x00000000; //No info
            format_identifier=0x00000000; //No info
            descriptor_tag=0x00; //No info
            DVD_Identifier=0;
            StreamIsRegistred=0;
            StreamOrder=(size_t)-1;
            FirstPacketOrder=(size_t)-1;
            Searching_Payload=false;
            Searching_TimeStamp_Start=false;
            Searching_TimeStamp_End=false;
            IsFilled=false;
        }

        ~ps_stream()
        {
            for (size_t Pos=0; Pos<Parsers.size(); Pos++)
                delete Parsers[Pos]; //Parsers[Pos]=NULL;
        }
    };
    std::vector<ps_stream> Streams;
    std::vector<ps_stream> Streams_Private1; //There can have multiple streams in one private stream
    std::vector<ps_stream> Streams_Extension; //There can have multiple streams in one private stream
    int8u stream_id;

    //Temp
    int64u SizeToAnalyze; //Total size of a chunk to analyse, it may be changed by the parser
    int8u  stream_id_extension;
    bool   video_stream_Unlimited;
    int16u Buffer_DataSizeToParse;
    std::vector<int64u> video_stream_PTS;
    size_t StreamOrder_CountOfPrivateStreams_Minus1;
    size_t StreamOrder_CountOfPrivateStreams_Temp;
    size_t FirstPacketOrder_Last;

    //Helpers
    bool Header_Parser_QuickSearch();

    //Parsers
    File__Analyze* ChooseParser_Mpegv();
    File__Analyze* ChooseParser_Mpeg4v();
    File__Analyze* ChooseParser_Avc();
    File__Analyze* ChooseParser_Hevc();
    File__Analyze* ChooseParser_VC1();
    File__Analyze* ChooseParser_Dirac();
    File__Analyze* ChooseParser_Mpega();
    File__Analyze* ChooseParser_Adts();
    File__Analyze* ChooseParser_Latm();
    File__Analyze* ChooseParser_AC3();
    File__Analyze* ChooseParser_DTS();
    File__Analyze* ChooseParser_SDDS();
    File__Analyze* ChooseParser_AAC();
    File__Analyze* ChooseParser_PCM();
    File__Analyze* ChooseParser_SmpteSt0302();
    File__Analyze* ChooseParser_RLE();
    File__Analyze* ChooseParser_AribStdB24B37(bool HasCcis=false);
    File__Analyze* ChooseParser_DvbSubtitle();
    File__Analyze* ChooseParser_PGS();
    File__Analyze* ChooseParser_Teletext();
    File__Analyze* ChooseParser_PS2();
    File__Analyze* ChooseParser_NULL();

    //File__Analyze helpers
    enum kindofstream
    {
        KindOfStream_Main,
        KindOfStream_Private,
        KindOfStream_Extension,
    };
    void Streams_Fill_PerStream(size_t StreamID, ps_stream &Temp, kindofstream KindOfStream);
    void Streams_Finish_PerStream(size_t StreamID, ps_stream &Temp, kindofstream KindOfStream);
    void xxx_stream_Parse(ps_stream &Temp, int8u &stream_Count);

    //Output buffer
    size_t Output_Buffer_Get (const String &Value);
    size_t Output_Buffer_Get (size_t Pos);

    #if MEDIAINFO_SEEK
        std::map<int16u, int64u>    Unsynch_Frame_Counts;
        int64u                      Seek_Value;
        int64u                      Seek_Value_Maximal;
        int64u                      Seek_ID;
        bool                        Duration_Detected;
    #endif //MEDIAINFO_SEEK
};

} //NameSpace

#endif
