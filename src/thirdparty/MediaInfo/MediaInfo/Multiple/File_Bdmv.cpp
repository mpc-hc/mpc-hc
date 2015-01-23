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
#if defined(MEDIAINFO_BDMV_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Bdmv.h"
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "ZenLib/Dir.h"
#include "ZenLib/FileName.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//     index        (INDX) with title
// --> MovieObject  (MOBJ) with mobj
// --> PlayList     (MPLS)
// --> PlayItem     (MPLS) with Mark
// --> ClipInfo     (CLPI)
// --> Clip (?)     (CLPI)
// --> Stream       (M2TS)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int32u CLPI=0x48444D56; //HDMV
    const int32u INDX=0x494E4458;
    const int32u MOBJ=0x4D4F424A;
    const int32u MPLS=0x4D504C53;
}

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Clpi_Offsets[]=
{
    "ClipInfo",
    "SequenceInfo",
    "ProgramInfo",
    "CPI",
    "ClipMark",
    "ExtensionData",
    "Reserved",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
const char* Indx_Offsets[]=
{
    "AppInfoBDMV",
    "Indexes",
    "ExtensionData",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
const char* Mobj_Offsets[]=
{
    "MovieObjects",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
const char* Mpls_Offsets[]=
{
    "AppInfoPlayList",
    "PlayList",
    "PlayListMarks",
    "ExtensionData",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

//---------------------------------------------------------------------------
const char* Bdmv_Type(int32u Type_Indicator, size_t Start_Adress_Pos)
{
    switch (Type_Indicator)
    {
        case Elements::CLPI : return Clpi_Offsets[Start_Adress_Pos];
        case Elements::INDX : return Indx_Offsets[Start_Adress_Pos];
        case Elements::MOBJ : return Mobj_Offsets[Start_Adress_Pos];
        case Elements::MPLS : return Mpls_Offsets[Start_Adress_Pos];
        default             : return "";
    }
}

//---------------------------------------------------------------------------
const char* Clpi_Format(int8u StreamType)
{
    switch (StreamType)
    {
        case 0x01 : return "MPEG-1 Video";
        case 0x02 : return "MPEG-2 Video";
        case 0x03 : return "MPEG-1 Audio";
        case 0x04 : return "MPEG-2 Audio";
        case 0x1B : return "AVC";
        case 0x20 : return "AVC";
        case 0x80 : return "PCM";
        case 0x81 : return "AC-3";
        case 0x82 : return "DTS";
        case 0x83 : return "TrueHD";
        case 0x84 : return "E-AC-3";
        case 0x85 : return "DTS";
        case 0x86 : return "DTS";
        case 0x90 : return "PGS";
        case 0x91 : return "Interactive";
        case 0x92 : return "Subtitle";
        case 0xA1 : return "E-AC-3"; //Secondary
        case 0xA2 : return "DTS"; //Secondary
        case 0xEA : return "VC-1";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Clpi_Format_Profile(int8u StreamType)
{
    switch (StreamType)
    {
        case 0x85 : return "HD";
        case 0x86 : return "MA";
        case 0xA2 : return "HD"; //Secondary
        default   : return "";
    }
}

//---------------------------------------------------------------------------
stream_t Clpi_Type(int8u StreamType)
{
    switch (StreamType)
    {
        case 0x01 : return Stream_Video;
        case 0x02 : return Stream_Video;
        case 0x03 : return Stream_Audio;
        case 0x04 : return Stream_Audio;
        case 0x1B : return Stream_Video;
        case 0x20 : return Stream_Video;
        case 0x80 : return Stream_Audio;
        case 0x81 : return Stream_Audio;
        case 0x82 : return Stream_Audio;
        case 0x83 : return Stream_Audio;
        case 0x84 : return Stream_Audio;
        case 0x85 : return Stream_Audio;
        case 0x86 : return Stream_Audio;
        case 0x90 : return Stream_Text;
        case 0x91 : return Stream_Max;
        case 0x92 : return Stream_Text;
        case 0xA1 : return Stream_Audio;
        case 0xA2 : return Stream_Audio;
        case 0xEA : return Stream_Video;
        default   : return Stream_Max;
    }
}

//---------------------------------------------------------------------------
const char* Clpi_Video_Format[]=
{
    "",
    "480i",
    "576i",
    "480p",
    "1080i",
    "720p",
    "1080p",
    "576p",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Clpi_Video_Interlacement[]=
{
    "",
    "Interlaced",
    "Interlaced",
    "PPF",
    "Interlaced",
    "PPF",
    "PPF",
    "PPF",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
const char* Clpi_Video_Standard[]=
{
    "",
    "NTSC",
    "PAL",
    "NTSC",
    "",
    "",
    "",
    "PAL",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

//---------------------------------------------------------------------------
int16u Clpi_Video_Width[]=
{
    0,
    720,
    720,
    720,
    1920,
    1280,
    1920,
    720,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

//---------------------------------------------------------------------------
int16u Clpi_Video_Height[]=
{
    0,
    480,
    576,
    480,
    1080,
    720,
    1080,
    576,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

//---------------------------------------------------------------------------
float32 Clpi_Video_FrameRate[]=
{
    (float32) 0.000,
    (float32)23.976,
    (float32)24.000,
    (float32)25.000,
    (float32)29.970,
    (float32) 0.000,
    (float32)50.000,
    (float32)59.940,
    (float32) 0.000,
    (float32) 0.000,
    (float32) 0.000,
    (float32) 0.000,
    (float32) 0.000,
    (float32) 0.000,
    (float32) 0.000,
    (float32) 0.000,
};

//---------------------------------------------------------------------------
float32 Clpi_Video_AspectRatio[]=
{
    (float32)0.000,
    (float32)0.000,
    (float32)1.333,
    (float32)1.778,
    (float32)2.210,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
    (float32)0.000,
};

//---------------------------------------------------------------------------
int8u Clpi_Audio_Channels[]=
{
    0,
    1,
    0,
    2,
    0,
    0,
    0, //Multi 6-8
    0,
    0,
    0,
    0,
    0,
    0, //Combo
    0,
    0,
    0,
};

//---------------------------------------------------------------------------
int32u Clpi_Audio_SamplingRate[]=
{
        0,
    48000,
        0,
        0,
    96000,
   192000,
        0,
        0,
        0,
        0,
        0,
        0,
    48000, //192000?
    48000, // 96000?
        0,
        0,
};

//---------------------------------------------------------------------------
const char* Indx_object_type[]=
{
    "",
    "HDMV",
    "BD-J",
    "",
};

//---------------------------------------------------------------------------
const char* Indx_playback_type[4][4]=
{
    {"",            "",             "",             "",             },
    {"Movie",       "Interactive",  "",             "",             },
    {"",            "",             "Movie",        "Interactive",  },
    {"",            "",             "",             "",             },
};

//---------------------------------------------------------------------------
const char* Indx_title_search[]=
{
    "Permitted",
    "Prohibited1",
    "Prohibited2",
    "",
};

//---------------------------------------------------------------------------
const char* Mpls_playback_type[]=
{
    "Sequential",
    "Random",
    "Shuffle",
    "",
};

//---------------------------------------------------------------------------
const char* Mpls_PlayListMarks_Mark_type(int8u type)
{
    switch (type)
    {
        case 1 : return "entry-mark";
        case 2 : return "link point";
        default: return "";
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
Ztring Bdmv_Decimal_Hexa(int64u Number)
{
    Ztring Temp;
    Temp.From_Number(Number);
    Temp+=__T(" (0x");
    Temp+=Ztring::ToZtring(Number, 16);
    Temp+=__T(")");
    return Temp;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Bdmv::FileHeader_Begin()
{
    size_t BDMV_Pos=File_Name.find(Ztring(1, PathSeparator)+__T("BDMV"));
    if (BDMV_Pos!=string::npos && BDMV_Pos+5==File_Name.size()) //Blu-ray directory
        return true;

    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    switch (CC4(Buffer))
    {
        case Elements::CLPI :
        case Elements::INDX :
        case Elements::MOBJ :
        case Elements::MPLS :
                              break;
        default             : Reject("Blu-ray");
                              return false;
    }

    //Init
    Mpls_PlayList_IsParsed=false;

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Bdmv::Read_Buffer_Continue()
{
    size_t BDMV_Pos=File_Name.find(Ztring(1, PathSeparator)+__T("BDMV"));
    if (BDMV_Pos!=string::npos && BDMV_Pos+5==File_Name.size()) //Blu-ray directory
    {
        BDMV();
        return;
    }

    if (Buffer_Size<File_Size)
    {
        Element_WaitForMoreData();
        return; //Wait for more data
    }

    //Parsing
    int32u type_indicator;
    int16u version_numberH;
    Element_Begin1("Header");
    Get_C4 (type_indicator,                                     "type_indicator");
    Data_Accept("Blu-ray");
    Get_C2 (version_numberH,                                    "version_number (High)");
    Skip_C2(                                                    "version_number (Low)");
    Element_End0();

    FILLING_BEGIN();
        Accept("BDMV");
        switch (type_indicator)
        {
            case Elements::CLPI : Fill(Stream_General, 0, General_Format, "Blu-ray Clip info"); break;
            case Elements::INDX : Fill(Stream_General, 0, General_Format, "Blu-ray Index"); break;
            case Elements::MOBJ : Fill(Stream_General, 0, General_Format, "Blu-ray Movie object"); break;
            case Elements::MPLS : Fill(Stream_General, 0, General_Format, "Blu-ray Playlist"); break;
            default             : ;
        }
    FILLING_END();

    if (version_numberH==0x3031 || version_numberH==0x3032) //Version 1 or 2
    {
        Element_Begin1("Offsets");
        Types[0x28]=0; //First object
        for (size_t Start_Adress_Pos=1; Start_Adress_Pos<9; Start_Adress_Pos++)
        {
            int32u Start_Adress;
            Get_B4 (Start_Adress,                               Bdmv_Type(type_indicator, Start_Adress_Pos));
            Types[Start_Adress]=Start_Adress_Pos;
        }
        Element_End0();

        for (std::map<int32u, size_t>::iterator Type=Types.begin(); Type!=Types.end(); ++Type)
        {
            if (Type->first>=Element_Offset) //If valid
            {
                if (Type->first>Element_Offset)
                    Skip_XX(Type->first-Element_Offset,         "unknown");

                Element_Begin1(Bdmv_Type(type_indicator, Type->second));
                int32u length;
                Get_B4 (length,                                 "length");
                int64u End=Element_Offset+length;
                switch (type_indicator)
                {
                    case Elements::CLPI :
                                            switch(Type->second)
                                            {
                                                case 2 : Clpi_ProgramInfo(); break;
                                                case 5 : Clpi_ExtensionData(); break;
                                                default: ;
                                            }
                                            break;
                    case Elements::INDX :
                                            switch(Type->second)
                                            {
                                                case 0 : Indx_AppInfoBDMV(); break;
                                                case 1 : Indx_Indexes(); break;
                                                case 2 : Indx_ExtensionData(); break;
                                                default: ;
                                            }
                                            break;
                    case Elements::MOBJ :
                                            switch(Type->second)
                                            {
                                                case 0 : Mobj_MovieObjects(); break;
                                                case 1 : Mobj_ExtensionData(); break;
                                                default: ;
                                            }
                                            break;
                    case Elements::MPLS :
                                            switch(Type->second)
                                            {
                                                case 0 : Mpls_AppInfoPlayList(); break;
                                                case 1 : Mpls_PlayList(); break;
                                                case 2 : Mpls_PlayListMarks (); break;
                                                case 3 : Mpls_ExtensionData (); break;
                                                default: ;
                                            }
                                            break;
                    default             :   ;
                }
                if (End>Element_Offset)
                    Skip_XX(End-Element_Offset,                 "Unknown");
                Element_End0();
            }
        }

        if (Element_Size>Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "Unknown");
    }
    else
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Bdmv::BDMV()
{
    Accept("BDMV");

    //Searching the longest playlist
    ZtringList List=Dir::GetAllFileNames(File_Name+PathSeparator+__T("PLAYLIST")+PathSeparator+__T("*.mpls"), Dir::Include_Files);
    std::vector<MediaInfo_Internal*> MIs;
    MIs.resize(List.size());
    size_t MaxDuration_Pos=(size_t)-1;
    int64u MaxDuration=0;
    if (Config->File_Bdmv_ParseTargetedFile_Get())
    {
        for (size_t Pos=0; Pos<MIs.size(); Pos++)
        {
            MIs[Pos]=new MediaInfo_Internal();
            MIs[Pos]->Option(__T("File_Bdmv_ParseTargetedFile"), __T("0"));
            MIs[Pos]->Option(__T("File_IsReferenced"), __T("1"));
            MIs[Pos]->Open(List[Pos]);
            int64u Duration=Ztring(MIs[Pos]->Get(Stream_General, 0, General_Duration)).To_int64u();
            if (Duration>MaxDuration)
            {
                MaxDuration=Duration;
                MaxDuration_Pos=Pos;
            }
        }
    }

    if (MaxDuration_Pos!=(size_t)-1)
    {
        //Merging
        MediaInfo_Internal MI;
        MI.Option(__T("File_IsReferenced"), __T("1"));
        MI.Open(List[MaxDuration_Pos]); //Open it again for having the M2TS part
        Merge(MI);

        Clear(Stream_General, 0, General_Format);
        Clear(Stream_General, 0, General_Format_String);
        Clear(Stream_General, 0, General_Format_Extensions);
        Clear(Stream_General, 0, General_Format_Info);
        Clear(Stream_General, 0, General_Codec);
        Clear(Stream_General, 0, General_Codec_String);
        Clear(Stream_General, 0, General_Codec_Extensions);
        Clear(Stream_General, 0, General_FileSize);
        Clear(Stream_Video,   0, Video_ScanType_String);
        Clear(Stream_Video,   0, Video_Bits__Pixel_Frame_);
    }

    for (size_t Pos=0; Pos<MIs.size(); Pos++)
        delete MIs[Pos]; //MIs[Pos]=NULL;
    MIs.clear();

    //Detecting some directories
    if (Dir::Exists(File_Name+PathSeparator+__T("BDSVM"))
     || Dir::Exists(File_Name+PathSeparator+__T("SLYVM"))
     || Dir::Exists(File_Name+PathSeparator+__T("ANYVM")))
        Fill(Stream_General, 0, General_Format_Profile, "BD+");
    if (Dir::Exists(File_Name+PathSeparator+__T("BDJO")) && !Dir::GetAllFileNames(File_Name+PathSeparator+__T("BDJO")).empty())
        Fill(Stream_General, 0, General_Format_Profile, "BD-Java");

    //Filling
    File_Name.resize(File_Name.size()-5); //Removing "/BDMV"
    Fill(Stream_General, 0, General_Format, "Blu-ray movie", Unlimited, true, true);
    Fill(Stream_General, 0, General_CompleteName, File_Name, true);
    Fill(Stream_General, 0, General_FolderName, FileName::Path_Get(File_Name), true);
    if (FileName::Extension_Get(File_Name).empty())
        Fill(Stream_General, 0, General_FileName, FileName::Name_Get(File_Name), true);
    else
        Fill(Stream_General, 0, General_FileName, FileName::Name_Get(File_Name)+__T('.')+FileName::Extension_Get(File_Name), true);
    File_Name.clear();

    Finish("BDMV");
}

//---------------------------------------------------------------------------
void File_Bdmv::Clpi_ProgramInfo()
{
    //Retrieving data from the M2TS file
    std::map<int16u, stream_t> PIDs_StreamKind;
    std::map<int16u, size_t> PIDs_StreamPos;
    if (Config->File_Bdmv_ParseTargetedFile_Get() && File_Name.size()>10+1+7)
    {
        Ztring file=File_Name.substr(File_Name.size()-10, 5);
        Ztring M2TS_File=File_Name;
        M2TS_File.resize(M2TS_File.size()-(10+1+7));
        M2TS_File+=__T("STREAM");
        M2TS_File+=PathSeparator;
        M2TS_File+=file;
        M2TS_File+=__T(".m2ts");

        MediaInfo_Internal MI;
        MI.Option(__T("File_Bdmv_ParseTargetedFile"), __T("0"));
        MI.Option(__T("File_IsReferenced"), __T("1"));
        if (MI.Open(M2TS_File))
        {
            Merge(MI);
            for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
                for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
                    Fill((stream_t)StreamKind, StreamPos, "Source", file+__T(".m2ts"));
        }

        //Retrieving PID mapping
        for (size_t StreamKind=(size_t)Stream_General+1; StreamKind<(size_t)Stream_Max; StreamKind++)
            for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
            {
                int16u PID=Retrieve((stream_t)StreamKind, StreamPos, General_ID).To_int16u();
                PIDs_StreamKind[PID]=(stream_t)StreamKind;
                PIDs_StreamPos[PID]=StreamPos;
            }
    }

    //Parsing
    int8u number_of_program_sequences;
    Skip_B1(                                                    "Unknown");
    Get_B1 (number_of_program_sequences,                        "number_of_program_sequences");
    for (int8u program_sequence=0; program_sequence<number_of_program_sequences; program_sequence++)
    {
        int8u number_of_streams_in_ps;
        Skip_B4(                                                "Unknown");
        Skip_B2(                                                "program_map_PID");
        Get_B1 (number_of_streams_in_ps,                        "number_of_streams_in_ps");
        Skip_B1(                                                "Unknown");
        for (int16u Pos=0; Pos<number_of_streams_in_ps; Pos++)
        {
            Element_Begin1("Stream");
            int16u stream_PID;
            int8u  Stream_Length;
            Get_B2 (stream_PID,                                 "stream_PID");
            Get_B1 (Stream_Length,                              "Length");
            int64u Stream_End=Element_Offset+Stream_Length;
            StreamKind_Last=Stream_Max;
            std::map<int16u, stream_t>::iterator PID_StreamKind=PIDs_StreamKind.find(stream_PID);
            if (PID_StreamKind!=PIDs_StreamKind.end())
            {
                StreamKind_Last=PID_StreamKind->second;
                StreamPos_Last=PIDs_StreamPos.find(stream_PID)->second;
            }
            Get_B1 (stream_type,                                "Stream type"); Param_Info1(Clpi_Format(stream_type)); Element_Info1(Clpi_Format(stream_type));
            switch (Clpi_Type(stream_type))
            {
                case Stream_Video : StreamCodingInfo_Video(); break;
                case Stream_Audio : StreamCodingInfo_Audio(); break;
                case Stream_Text  : StreamCodingInfo_Text() ; break;
                default           : ;
            }

            if (Stream_End-Element_Offset)
                Skip_XX(Stream_End-Element_Offset,              "Unknown");
            Element_End0();

            FILLING_BEGIN();
                if (StreamKind_Last!=Stream_Max)
                {
                    Fill(StreamKind_Last, StreamPos_Last, General_ID, stream_PID, 10, true);
                    Fill(StreamKind_Last, StreamPos_Last, General_ID_String, Bdmv_Decimal_Hexa(stream_PID), true);
                }
            FILLING_END();
        }
    }
}

struct entry
{
    int16u ID1;
    int16u ID2;
    int32u Length;
};
typedef std::map<int32u, entry> entries; //Key is the start address

//---------------------------------------------------------------------------
void File_Bdmv::Clpi_ExtensionData()
{
    entries Entries; //Key is the start address

    int32u Base_Pos=(int32u)Element_Offset-4;

    int8u number_of_ext_data_entries;
    Skip_B4(                                                    "Unknown");
    Skip_B3(                                                    "Unknown");
    Element_Begin1("Offsets");
    Get_B1 (number_of_ext_data_entries,                         "number_of_ext_data_entries");
    for (size_t Start_Adress_Pos=0; Start_Adress_Pos<number_of_ext_data_entries; Start_Adress_Pos++)
    {
        int32u Start_Adress, Length;
        int16u ID1, ID2;
        Get_B2 (ID1,                                            "ID1");
        Get_B2 (ID2,                                            "ID2");
        Get_B4 (Start_Adress,                                   "Start_Adress");
        Get_B4 (Length,                                         "Length");
        Entries[Base_Pos+Start_Adress].ID1=ID1;
        Entries[Base_Pos+Start_Adress].ID2=ID2;
        Entries[Base_Pos+Start_Adress].Length=Length;
    }
    Element_End0();

    for (entries::iterator Entry=Entries.begin(); Entry!=Entries.end(); ++Entry)
    {
        if (Entry->first>=Element_Offset) //If valid
        {
            if (Entry->first>Element_Offset)
                Skip_XX(Entry->first-Element_Offset, "unknown");

            Element_Begin1("Entry");
            int32u length;
            Get_B4 (length,                                 "length");
            int64u End=Element_Offset+length;
            switch (Entry->second.ID1)
            {
                case 0x0002 :
                                switch(Entry->second.ID2)
                                {
                                    case 0x0005 : Clpi_ProgramInfo(); break;
                                    default: ;
                                }
                                break;
                default             :   ;
            }
            if (End>Element_Offset)
                Skip_XX(End-Element_Offset,                 "Unknown");
            Element_End0();
        }
    }

    if (Element_Size>Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                "Unknown");
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_AppInfoBDMV()
{
    //Parsing
    Skip_B2(                                                    "reserved");
    Skip_Local(32,                                              "user_data");
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_Indexes()
{
    //Parsing
    int16u number_of_Titles;
    Element_Begin1("FirstPlayback");
        BS_Begin();
        int8u FirstPlayback_object_type;
        Get_S1 ( 2, FirstPlayback_object_type,                  "object_type"); Param_Info1(Indx_object_type[FirstPlayback_object_type]);
        Skip_S4(30,                                             "reserved");
        BS_End();
        Indx_Indexes_Index(FirstPlayback_object_type);
    Element_End0();
    Element_Begin1("TopMenu");
        BS_Begin();
        int8u TopMenu_object_type;
        Get_S1 ( 2, TopMenu_object_type,                        "object_type"); Param_Info1(Indx_object_type[TopMenu_object_type]);
        Skip_S4(30,                                             "reserved");
        BS_End();
        Indx_Indexes_Index(TopMenu_object_type);
    Element_End0();
    Get_B2 (number_of_Titles,                                   "number_of_Titles");
    for (int16u Pos=0; Pos<number_of_Titles; Pos++)
    {
        Element_Begin1("Title");
        BS_Begin();
        int8u Title_object_type;
        Get_S1 ( 2, Title_object_type,                          "object_type"); Param_Info1(Indx_object_type[Title_object_type]);
        Info_S1( 2, Title_title_search,                         "title_search"); Param_Info1(Indx_title_search[Title_title_search]);
        Skip_S4(28,                                             "reserved");
        BS_End();
        Indx_Indexes_Index(Title_object_type);
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_Indexes_Index(int8u object_type)
{
    BS_Begin();
    Info_S1( 2, playback_type,                                  "playback_type"); Param_Info1(Indx_playback_type[object_type][playback_type]);
    Skip_S2(14,                                                 "reserved");
    BS_End();
    switch (object_type)
    {
        case 1 : //HDMV
                {
                Info_B2(id_ref,                                 "id_ref"); Element_Info1(id_ref);
                Skip_B4(                                        "reserved");
                }
                break;
        case 2 : //BD-J
                {
                Info_Local(5, id_ref,                           "id_ref"); Element_Info1(id_ref);
                Skip_B1(                                        "reserved");
                }
                break;
        default:
                Skip_XX(6,                                      "unknown");
    }
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_ExtensionData()
{
    //Parsing
    std::map<int32u, int32u> exts; //Key is the start address, value is length
    int64u Base_Offset=Element_Offset-4; //Size is included
    int8u number_of_ext_data_entries;
    Skip_B4(                                                    "data_block_start_adress");
    Skip_B3(                                                    "reserved");
    Get_B1 (number_of_ext_data_entries,                         "number_of_ext_data_entries");
    for (int16u Pos=0; Pos<number_of_ext_data_entries; Pos++)
    {
        Element_Begin1("ext_data_entry");
        int32u ext_data_start_adress, ext_data_length;
        Skip_B2(                                                "ID1 (AVCHD)");
        Skip_B2(                                                "ID2 (Version)");
        Get_B4 (ext_data_start_adress,                          "ext_data_start_adress");
        Get_B4 (ext_data_length,                                "ext_data_length");
        Element_End0();
        exts[ext_data_start_adress]=ext_data_length;
    }

    for (std::map<int32u, int32u>::iterator ext=exts.begin(); ext!=exts.end(); ++ext)
    {
        if (Base_Offset+ext->first>=Element_Offset)
        {
            if (Base_Offset+ext->first>Element_Offset)
                Skip_XX(ext->first-Element_Offset,              "Unknown");

            Element_Begin0();
            int64u End=Element_Offset+ext->second;

            int32u type_indicator;
            Get_C4(type_indicator,                              "type_indicator"); Element_Info1(Ztring().From_CC4(type_indicator));
            switch (type_indicator)
            {
                case 0x49444558 : Indx_ExtensionData_IDEX(); break;
                default         : Element_Name("Unknown");
                                  Skip_XX(ext->second-4,        "Unknown");
            }
            if (End>Element_Offset)
                Skip_XX(End-Element_Offset,                     "Unknown");
            Element_End0();
        }
    }
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_ExtensionData_IDEX()
{
    Element_Name("IndexExtension");

    //Parsing
    int64u Base_Offset=Element_Offset-4; //Size is included
    int32u TableOfPlayLists_start_adress, MakersPrivateData_start_adress;
    Skip_B4(                                                    "reserved");
    Get_B4 (TableOfPlayLists_start_adress,                      "TableOfPlayLists_start_adress");
    Get_B4 (MakersPrivateData_start_adress,                     "MakersPrivateData_start_adress");
    Skip_XX(24,                                                 "reserved");

    Indx_ExtensionData_IDEX_UIAppInfoAVCHD();
    if (TableOfPlayLists_start_adress)
    {
        if (Base_Offset+TableOfPlayLists_start_adress>Element_Offset)
            Skip_XX(Base_Offset+TableOfPlayLists_start_adress-Element_Offset, "Unknown");
        Indx_ExtensionData_IDEX_TableOfPlayLists();
    }
    if (MakersPrivateData_start_adress)
    {
        if (Base_Offset+MakersPrivateData_start_adress>Element_Offset)
            Skip_XX(Base_Offset+MakersPrivateData_start_adress-Element_Offset, "Unknown");
        Indx_ExtensionData_IDEX_MakersPrivateData();
    }
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_ExtensionData_IDEX_UIAppInfoAVCHD()
{
    Element_Begin1("UIAppInfoAVCHD");

    //Parsing
    int32u length, length2;
    int8u AVCHD_name_length;
    Get_B4 (length,                                             "length");
    Skip_B2(                                                    "maker_ID");
    Skip_B2(                                                    "maker_model_code");
    Skip_XX(32,                                                 "maker_private_area");
    BS_Begin();
    Skip_BS(15,                                                 "reserved");
    Skip_SB(                                                    "AVCHD_write_protect_flag");
    BS_End();
    Skip_B2(                                                    "ref_to_menu_thumbail_index");
    Skip_B1(                                                    "time_zone");
    Skip_XX(7,                                                  "record_time_and_date");
    Skip_B1(                                                    "reserved");
    Skip_B1(                                                    "AVCHD_character_set");
    Get_B1 (AVCHD_name_length,                                  "AVCHD_name_length");
    Skip_Local(AVCHD_name_length,                               "AVCHD_name");
    Skip_XX(255-AVCHD_name_length,                              "AVCHD_name (junk)");
    Element_Begin1("additional data");
    Get_B4 (length2,                                            "length2");
    Skip_XX(length2,                                            "reserved");
    Element_End0();

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_ExtensionData_IDEX_TableOfPlayLists()
{
    Element_Begin1("TableOfPlayLists");

    //Parsing
    int32u length;
    Get_B4 (length,                                             "length");
    Skip_XX(length,                                             "unknown");

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Bdmv::Indx_ExtensionData_IDEX_MakersPrivateData()
{
    Element_Begin1("MakersPrivateData");

    //Parsing
    int64u Base_Offset=Element_Offset-4; //Size is included
    int32u length, datablock_start_adress;
    int8u number_of_maker_entries;
    Get_B4 (length,                                             "length");
    Get_B4 (datablock_start_adress,                             "datablock_start_adress");
    Skip_XX(24,                                                 "reserved");
    Get_B1 (number_of_maker_entries,                            "number_of_maker_entries");
    for (int8u Pos=0; Pos<number_of_maker_entries; Pos++)
    {
        Element_Begin1("maker_entry");
        Skip_B2(                                                "maker_ID");
        Skip_B2(                                                "maker_model_code");
        Skip_B4(                                                "mpd_start_adress");
        Skip_B4(                                                "mpd_length");
        Element_End0();
    }

    if (datablock_start_adress)
    {
        if (Base_Offset+datablock_start_adress>Element_Offset)
            Skip_XX(Base_Offset+datablock_start_adress-Element_Offset, "Unknown");
        Skip_XX(length-datablock_start_adress,                  "Unknown");
    }

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mobj_MovieObjects()
{
    //Parsing
    int16u number_of_mobjs;
    Skip_B4(                                                    "reserved");
    Get_B2 (number_of_mobjs,                                    "number_of_mobj");
    for (int16u mobjs_Pos=0; mobjs_Pos<number_of_mobjs; mobjs_Pos++)
    {
        Element_Begin1("mobj");
        int16u number_of_navigation_commands;
        BS_Begin();
        Info_SB(resume,                                         "resume"); Param_Info1(resume?"suspend":"discard");
        Info_SB(menu_call,                                      "menu_call"); Param_Info1(menu_call?"enable":"disable");
        Info_SB(title_search,                                   "title_search"); Param_Info1(title_search?"enable":"disable");
        Skip_BS(13,                                             "reserved");
        BS_End();
        Get_B2 (number_of_navigation_commands,                  "number_of_navigation_commands");
        for (int16u navigation_command_Pos=0; navigation_command_Pos<number_of_navigation_commands; navigation_command_Pos++)
        {
            Element_Begin1("navigation_command");
            Skip_B4(                                            "opcode");
            Skip_B4(                                            "destination");
            Skip_B4(                                            "source");
            Element_End0();
        }
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Bdmv::Mobj_ExtensionData()
{
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_AppInfoPlayList()
{
    //Parsing
    Skip_B1(                                                    "unknown");
    BS_Begin();
    Skip_S1(6,                                                  "unknown");
    Info_S2(2, playback_type,                                   "playback_type"); Param_Info1(Mpls_playback_type[playback_type]);
    BS_End();
    Skip_B2(                                                    "playback_count");
    Skip_B4(                                                    "user_operation_mask_code 1");
    Skip_B4(                                                    "user_operation_mask_code 2");
    BS_Begin();
    Skip_SB(                                                    "random access");
    Skip_SB(                                                    "audio mix");
    Skip_SB(                                                    "bypass mixer");
    Skip_S2(13,                                                 "reserved");
    BS_End();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList()
{
    //Parsing
    Mpls_PlayList_Duration=0;
    int16u number_of_PlayItems, number_of_SubPaths;
    Skip_B2(                                                    "reserved");
    Get_B2 (number_of_PlayItems,                                "number_of_PlayItems");
    Get_B2 (number_of_SubPaths,                                 "number_of_SubPaths");
    for (int16u Pos=0; Pos<number_of_PlayItems; Pos++)
        Mpls_PlayList_PlayItem();

    if (Mpls_PlayList_Duration)
        Fill(Stream_General, 0, General_Duration, Mpls_PlayList_Duration/45);

    for (int16u SubPath_Pos=0; SubPath_Pos<number_of_SubPaths; SubPath_Pos++)
    {
        Element_Begin1("SubPath");
        int32u SubPath_length;
        int16u number_of_SubPlayItems;
        int8u  SubPath_type;
        Get_B4 (SubPath_length,                                 "length");
        int64u SubPath_End=Element_Offset+SubPath_length;
        Skip_B1(                                                "Unknown");
        Get_B1 (SubPath_type,                                   "SubPath_type");
        Skip_B2(                                                "repeat");
        Get_B2 (number_of_SubPlayItems,                         "number_of_SubPlayItems");
        for (int16u Pos=0; Pos<number_of_SubPlayItems; Pos++)
            Mpls_PlayList_SubPlayItem(SubPath_type, Pos);

        if (SubPath_End>Element_Offset)
            Skip_XX(SubPath_End-Element_Offset,                 "unknown");
        Element_End0();
    }

    FILLING_BEGIN();
        if (!Mpls_PlayList_IsParsed)
        {
            Mpls_PlayList_number_of_SubPaths=number_of_SubPaths;
            Mpls_PlayList_IsParsed=true;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList_PlayItem()
{
    Element_Begin1("PlayItem");
    Ztring Clip_Information_file_name;
    int32u Time_In, Time_Out;
    int16u length;
    Get_B2 (length,                                         "length");
    int64u End=Element_Offset+length;
    Get_Local (5, Clip_Information_file_name,               "Clip_Information_file_name"); Element_Info1(Clip_Information_file_name);
    Skip_Local(4,                                           "Clip_codec_identifier");
    Skip_B2(                                                "unknown");
    Skip_B1(                                                "Unknown");
    Get_B4 (Time_In,                                        "Time (In)"); Param_Info1((float32)Time_In/45000);
    Get_B4 (Time_Out,                                       "Time (Out)"); Param_Info1((float32)Time_Out/45000);
    Skip_B4(                                                "UO1");
    Skip_B4(                                                "UO2");
    Skip_B4(                                                "An?");

    Mpls_PlayList_PlayItem_Duration=Time_Out-Time_In;
    if (Time_Out>Time_In)
        Mpls_PlayList_Duration+=Mpls_PlayList_PlayItem_Duration;

    std::vector<size_t> StreamCount_Before;
    for (size_t StreamKind=Stream_General; StreamKind<Stream_Max; StreamKind++)
        StreamCount_Before.push_back(Count_Get((stream_t)StreamKind));

    Mpls_PlayList_PlayItem_STN_table();

    if (Clip_Information_file_names.find(Clip_Information_file_name)==Clip_Information_file_names.end() && File_Name.size()>10+1+8)
    {
        Ztring CLPI_File=File_Name;
        CLPI_File.resize(CLPI_File.size()-(10+1+8));
        CLPI_File+=__T("CLIPINF");
        CLPI_File+=PathSeparator;
        CLPI_File+=Clip_Information_file_name;
        CLPI_File+=__T(".clpi");

        MediaInfo_Internal MI;
        MI.Option(__T("File_Bdmv_ParseTargetedFile"), Config->File_Bdmv_ParseTargetedFile_Get()?__T("1"):__T("0"));
        MI.Option(__T("File_IsReferenced"), __T("1"));
        if (MI.Open(CLPI_File))
        {
            for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
                for (size_t StreamPos=0; StreamPos<MI.Count_Get((stream_t)StreamKind); StreamPos++)
                {
                    while (StreamCount_Before[StreamKind]+StreamPos>=Count_Get((stream_t)StreamKind))
                        Stream_Prepare((stream_t)StreamKind);
                    Merge(MI, (stream_t)StreamKind, StreamPos, StreamCount_Before[StreamKind]+StreamPos);
                }
        }

        Clip_Information_file_names.insert(Clip_Information_file_name);
    }

    if (End>Element_Offset)
        Skip_XX(End-Element_Offset,                             "unknown");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList_PlayItem_STN_table()
{
    Element_Begin1("STN");

    int16u length;
    Get_B2 (length,                                         "length");
    int64u End=Element_Offset+length;
    if (End>Element_Size)
    {
        Skip_XX(Element_Size-Element_Offset,                "Problem");
        return;
    }
    Skip_B2(                                                "unknown");
    Skip_B1(                                                "Vi");
    Skip_B1(                                                "Au");
    Skip_B1(                                                "PG");
    Skip_B1(                                                "IG");
    Skip_B1(                                                "sV");
    Skip_B1(                                                "sA");
    Skip_B1(                                                "PIP");
    Skip_B1(                                                "unknown");
    Skip_B1(                                                "unknown");
    Skip_B1(                                                "unknown");
    Skip_B1(                                                "unknown");
    Skip_B1(                                                "unknown");

    while (Element_Offset+16<=End)
    {
        Element_Begin0();
        Ztring language;
        int16u mPID;
        int8u IDs_length;
        Skip_B1(                                            "type");
        Skip_B1(                                            "unknown");
        Get_B2 (mPID,                                       "mPID"); Element_Name(Ztring::ToZtring(mPID, 16));
        Skip_B2(                                            "SPid");
        Skip_B2(                                            "sCid");
        Skip_B2(                                            "sPID");
        Get_B1 (IDs_length,                                 "length");
        int64u IDs_End=Element_Offset+IDs_length;
        Get_B1 (stream_type,                                "stream_type"); Param_Info1(Clpi_Format(stream_type)); Element_Info1(Clpi_Format(stream_type));
        switch (Clpi_Type(stream_type))
        {
            case Stream_Video : Mpls_PlayList_PlayItem_STN_table_Video(); break;
            case Stream_Audio : Mpls_PlayList_PlayItem_STN_table_Audio(); break;
            case Stream_Text  : Mpls_PlayList_PlayItem_STN_table_Text() ; break;
            default           : StreamKind_Last=Stream_Max;
        }
        Get_Local(3, language,                              "language"); Element_Info1(language);

        if (IDs_End-Element_Offset)
            Skip_XX(IDs_End-Element_Offset,                 "unknown");
        Element_End0();

        FILLING_BEGIN();
            if (StreamKind_Last!=Stream_Max)
            {
                if (mPID)
                {
                    Fill(StreamKind_Last, StreamPos_Last, General_ID, mPID, 10, true);
                    Fill(StreamKind_Last, StreamPos_Last, General_ID_String, Bdmv_Decimal_Hexa(mPID), true);
                }
                Fill(StreamKind_Last, StreamPos_Last, "Language", language);
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), Mpls_PlayList_PlayItem_Duration/45);
            }
        FILLING_END();
    }

    if (End>Element_Offset)
        Skip_XX(End-Element_Offset,                             "unknown");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList_PlayItem_STN_table_Video()
{
    //Parsing
    int8u Format, FrameRate;
    BS_Begin();
    Get_S1 (4, Format,                                          "format"); Param_Info1(Clpi_Video_Format[Format]);
    Get_S1 (4, FrameRate,                                       "frame_rate"); Param_Info1(Clpi_Video_FrameRate[FrameRate]);
    BS_End();

    FILLING_BEGIN();
        Stream_Prepare(Stream_Video);
        Fill(Stream_Video, StreamPos_Last, Video_Format, Clpi_Format(stream_type));
        if (Clpi_Video_Width[Format])
            Fill(Stream_Video, StreamPos_Last, Video_Width, Clpi_Video_Width[Format]);
        if (Clpi_Video_Height[Format])
            Fill(Stream_Video, StreamPos_Last, Video_Height, Clpi_Video_Height[Format]);
        Fill(Stream_Video, StreamPos_Last, Video_Interlacement, Clpi_Video_Interlacement[Format]);
        Fill(Stream_Video, StreamPos_Last, Video_Standard, Clpi_Video_Standard[Format]);
        if (Clpi_Video_FrameRate[FrameRate])
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, Clpi_Video_FrameRate[FrameRate]);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList_PlayItem_STN_table_Audio()
{
    //Parsing
    int8u Channels, SamplingRate;
    BS_Begin();
    Get_S1 (4, Channels,                                        "channel_layout"); Param_Info1(Clpi_Audio_Channels[Channels]);
    Get_S1 (4, SamplingRate,                                    "sampling_rate"); Param_Info1(Clpi_Audio_SamplingRate[SamplingRate]);
    BS_End();

    FILLING_BEGIN();
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format, Clpi_Format(stream_type));
        Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, Clpi_Format_Profile(stream_type));
        if (Clpi_Audio_Channels[Channels])
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Clpi_Audio_Channels[Channels]);
        if (Clpi_Audio_SamplingRate[SamplingRate])
            Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, Clpi_Audio_SamplingRate[SamplingRate]);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList_PlayItem_STN_table_Text()
{
    //Parsing
    if (stream_type==0x92) //Subtitle
        Skip_B1(                                                "Unknown");

    FILLING_BEGIN();
        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, StreamPos_Last, Text_Format, Clpi_Format(stream_type));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayList_SubPlayItem(int8u SubPath_type, int16u Pos)
{
    Element_Begin1("SubPlayItem");
    Ztring Clip_Information_file_name;
    int16u length;
    Get_B2 (length,                                             "length");
    int64u End=Element_Offset+length;
    Get_Local (5, Clip_Information_file_name,                   "Clip_Information_file_name"); Element_Info1(Clip_Information_file_name);
    Skip_Local(4,                                               "Clip_codec_identifier");
    Skip_B4(                                                    "unknown");
    Skip_B1(                                                    "unknown");
    Info_B4(Time_In,                                            "time (in)"); Param_Info1((float32)Time_In/45000);
    Info_B4(Time_Out,                                           "time (out)"); Param_Info1((float32)Time_Out/45000);
    Skip_B2(                                                    "sync PI");
    Skip_B4(                                                    "sync PTS");

    if (End>Element_Offset)
        Skip_XX(End-Element_Offset,                             "unknown");
    Element_End0();

    FILLING_BEGIN();
        if (SubPath_type==8 && Pos!=(int16u)-1) //MVC
        {
            if (File_Name.size()>=10+1+8)
            {
                Ztring CLPI_File=File_Name;
                CLPI_File.resize(CLPI_File.size()-(10+1+8));
                CLPI_File+=__T("CLIPINF");
                CLPI_File+=PathSeparator;
                CLPI_File+=Clip_Information_file_name;
                CLPI_File+=__T(".clpi");

                MediaInfo_Internal MI;
                MI.Option(__T("File_Bdmv_ParseTargetedFile"), Config->File_Bdmv_ParseTargetedFile_Get()?__T("1"):__T("0"));
                MI.Option(__T("File_IsReferenced"), __T("1"));
                if (MI.Open(CLPI_File))
                {
                    if (MI.Count_Get(Stream_Video))
                    {
                        Ztring ID=Retrieve(Stream_Video, Pos, Video_ID);
                        Ztring ID_String=Retrieve(Stream_Video, Pos, Video_ID_String);
                        Ztring Format_Profile=Retrieve(Stream_Video, Pos, Video_Format_Profile);
                        Ztring BitRate=Retrieve(Stream_Video, Pos, Video_BitRate);
                        Ztring Source=Retrieve(Stream_Video, Pos, "Source");
                        Fill(Stream_Video, Pos, Video_ID, MI.Get(Stream_Video, 0, Video_ID)+__T(" / ")+ID, true);
                        Fill(Stream_Video, Pos, Video_ID_String, MI.Get(Stream_Video, 0, Video_ID_String)+__T(" / ")+ID_String, true);
                        if (!Format_Profile.empty())
                            Fill(Stream_Video, Pos, Video_Format_Profile, MI.Get(Stream_Video, 0, Video_Format_Profile)+__T(" / ")+Format_Profile, true);
                        if (!BitRate.empty())
                            Fill(Stream_Video, Pos, Video_BitRate, Ztring::ToZtring(BitRate.To_int32u()+MI.Get(Stream_Video, 0, Video_BitRate).To_int32u())+__T(" / ")+BitRate, true);
                        if (!Source.empty())
                            Fill(Stream_Video, Pos, "Source", Clip_Information_file_name +__T(".m2ts / ")+Source, true);
                    }
                }
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_PlayListMarks()
{
    Stream_Prepare(Stream_Menu);
    Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, StreamPos_Last), 10, true);

    //Parsing
    int32u time_Pos0=0, time_Pos=1;
    int16u count;
    Get_B2 (count,                                              "count");
    for (int16u Pos=0; Pos<count; Pos++)
    {
        Element_Begin1("Mark");
        int8u type;
        Skip_B1(                                                "unknown");
        Get_B1 (type,                                           "type"); Param_Info1(Mpls_PlayListMarks_Mark_type(type));
        switch (type)
        {
            case 1 : //entry-mark
            case 2 : //link point
                    {
                    int32u time;
                    int16u stream_file_index;
                    Get_B2 (stream_file_index,                  "stream_file_index");
                    Get_B4 (time,                               "time"); Param_Info2(time/45, " milliseconds");
                    Skip_B2(                                    "unknown");
                    Skip_B4(                                    "unknown");

                    FILLING_BEGIN();
                        if (Pos==0)
                            time_Pos0=time;
                        if (stream_file_index==0 && type==1) //We currently handle only the first file
                        {
                            Fill(Stream_Menu, 0, Ztring().Duration_From_Milliseconds((int64u)((time-time_Pos0)/45)).To_UTF8().c_str(), __T("Chapter ")+Ztring::ToZtring(time_Pos));
                            time_Pos++;
                        }
                    FILLING_END();
                    }
                    break;
            default:
                    Skip_XX(12,                                 "unknwon");
        }
        Element_End0();
    }

    Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_ExtensionData()
{
    entries Entries; //Key is the start address

    int32u Base_Pos=(int32u)Element_Offset-4;

    int8u number_of_ext_data_entries;
    Skip_B4(                                                    "Unknown");
    Skip_B3(                                                    "Unknown");
    Element_Begin1("Offsets");
    Get_B1 (number_of_ext_data_entries,                         "number_of_ext_data_entries");
    for (size_t Start_Adress_Pos=0; Start_Adress_Pos<number_of_ext_data_entries; Start_Adress_Pos++)
    {
        int32u Start_Adress, Length;
        int16u ID1, ID2;
        Get_B2 (ID1,                                            "ID1");
        Get_B2 (ID2,                                            "ID2");
        Get_B4 (Start_Adress,                                   "Start_Adress");
        Get_B4 (Length,                                         "Length");
        Entries[Base_Pos+Start_Adress].ID1=ID1;
        Entries[Base_Pos+Start_Adress].ID2=ID2;
        Entries[Base_Pos+Start_Adress].Length=Length;
    }
    Element_End0();

    for (entries::iterator Entry=Entries.begin(); Entry!=Entries.end(); ++Entry)
    {
        if (Entry->first>=Element_Offset) //If valid
        {
            if (Entry->first>Element_Offset)
                Skip_XX(Entry->first-Element_Offset,            "unknown");

            Element_Begin1("Entry");
            int64u End=Element_Offset+Entry->second.Length;
            switch (Entry->second.ID1)
            {
                case 0x0001 :
                                switch(Entry->second.ID2)
                                {
                                    case 0x0001 : break; //Mpls_ExtensionData_pip_metadata(); break;
                                    default: ;
                                }
                                break;
                case 0x0002 :
                                switch(Entry->second.ID2)
                                {
                                    case 0x0001 : break; //Mpls_ExtensionData_STN_table(); break;
                                    case 0x0002 : Mpls_ExtensionData_SubPath_entries(); break;
                                    case 0x0003 : break; //Mpls_ExtensionData_active_video_window(); break;
                                    default: ;
                                }
                                break;
                default             :   ;
            }
            if (End>Element_Offset)
                Skip_XX(End-Element_Offset,                     "Unknown");
            Element_End0();
        }
    }

    if (Element_Size>Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Bdmv::Mpls_ExtensionData_SubPath_entries()
{
    Element_Begin1("SubPath_entries");
    int32u length;
    int16u number_of_SubPath_extensions;
    int8u SubPath_type;
    Get_B4 (length,                                             "length");
    int64u End=Element_Offset+length;
    Get_B2 (number_of_SubPath_extensions,                       "number_of_SubPath_extensions");
    for (int8u SubPath_extension=0; SubPath_extension<number_of_SubPath_extensions; SubPath_extension++)
    {
        Element_Begin1("SubPath_extension");
        int32u SubPath_extension_length;
        Get_B4 (SubPath_extension_length,                       "length");
        int64u SubPath_extension_End=Element_Offset+SubPath_extension_length;
        Skip_B1(                                                "Unknown");
        Get_B1 (SubPath_type,                                   "SubPath_type");
        switch(SubPath_type)
        {
            case 0x08 :
                        {
                        int8u number_of_SubPlayItems;
                        Skip_B3(                                "Unknown");
                        Get_B1 (number_of_SubPlayItems,         "number_of_SubPlayItems");
                        for (int8u Pos=0; Pos<number_of_SubPlayItems; Pos++)
                            Mpls_PlayList_SubPlayItem(SubPath_type, Pos);
                        }
            default   : ;
        }
        if (SubPath_extension_End-Element_Offset)
            Skip_XX(SubPath_extension_End-Element_Offset,       "Padding");
        Element_End0();
    }
    if (End-Element_Offset)
        Skip_XX(End-Element_Offset,                             "Padding");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Bdmv::StreamCodingInfo_Video()
{
    //Parsing
    int8u Format, FrameRate, AspectRatio;
    BS_Begin();
    Get_S1 (4, Format,                                          "Format"); Param_Info1(Clpi_Video_Format[Format]);
    Get_S1 (4, FrameRate,                                       "Frame rate"); Param_Info1(Clpi_Video_FrameRate[FrameRate]);
    Get_S1 (4, AspectRatio,                                     "Aspect ratio"); Param_Info1(Clpi_Video_AspectRatio[AspectRatio]);
    Skip_BS(4,                                                  "Reserved");
    BS_End();

    FILLING_BEGIN();
        if (StreamKind_Last==Stream_Max)
        {
            Stream_Prepare(Stream_Video);
            Fill(Stream_Video, StreamPos_Last, Video_Format, Clpi_Format(stream_type));
            if (Clpi_Video_Width[Format])
                Fill(Stream_Video, StreamPos_Last, Video_Width, Clpi_Video_Width[Format]);
            if (Clpi_Video_Height[Format])
                Fill(Stream_Video, StreamPos_Last, Video_Height, Clpi_Video_Height[Format]);
            Fill(Stream_Video, StreamPos_Last, Video_Interlacement, Clpi_Video_Interlacement[Format]);
            Fill(Stream_Video, StreamPos_Last, Video_Standard, Clpi_Video_Standard[Format]);
            if (Clpi_Video_FrameRate[FrameRate])
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate, Clpi_Video_FrameRate[FrameRate]);
            if (Clpi_Video_Height[AspectRatio])
                Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, Clpi_Video_AspectRatio[AspectRatio], 3, true);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::StreamCodingInfo_Audio()
{
    //Parsing
    Ztring Language;
    int8u Channels, SamplingRate;
    BS_Begin();
    Get_S1 (4, Channels,                                        "Channel layout"); Param_Info1(Clpi_Audio_Channels[Channels]);
    Get_S1 (4, SamplingRate,                                    "Sampling Rate"); Param_Info1(Clpi_Audio_SamplingRate[SamplingRate]);
    BS_End();
    Get_Local(3, Language,                                      "Language"); Element_Info1(Language);

    FILLING_BEGIN();
        if (StreamKind_Last==Stream_Max)
        {
            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, StreamPos_Last, Audio_Format, Clpi_Format(stream_type));
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, Clpi_Format_Profile(stream_type));
            if (Clpi_Audio_Channels[Channels])
                Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Clpi_Audio_Channels[Channels]);
            if (Clpi_Audio_SamplingRate[SamplingRate])
                Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, Clpi_Audio_SamplingRate[SamplingRate]);
        }
        Fill(Stream_Audio, StreamPos_Last, Audio_Language, Language);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Bdmv::StreamCodingInfo_Text()
{
    //Parsing
    Ztring Language;
    if (stream_type==0x92) //Subtitle
        Skip_B1(                                                "Unknown");
    Get_Local(3, Language,                                      "Language"); Element_Info1(Language);

    FILLING_BEGIN();
        if (StreamKind_Last==Stream_Max)
        {
            Stream_Prepare(Stream_Text);
            Fill(Stream_Text, StreamPos_Last, Text_Format, Clpi_Format(stream_type));
        }
        Fill(Stream_Text, StreamPos_Last, Text_Language, Language);
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_BDMV_YES
