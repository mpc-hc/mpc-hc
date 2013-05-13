/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DVD objects
// (.ifo files on DVD-Video)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Mainly from http://dvd.sourceforge.net/dvdinfo/ifo.html
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#include <vector>
using namespace std;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_DVDV_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Dvdv.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char*  IFO_VTS_Category[]=
{
    "Normal",
    "Karaoke",
};

const char*  IFO_Format_V[]=
{
    "MPEG Video",
    "MPEG Video",
    "",
    "",
};

const char*  IFO_Format_Version_V[]=
{
    "Version 1",
    "Version 2",
    "",
    "",
};

const char*  IFO_CodecV[]=
{
    "MPEG-1V",
    "MPEG-2V",
    "",
    "",
};

const char*  IFO_Standard[]=
{
    "NTSC",
    "PAL",
    "",
    "",
};

float32  IFO_AspectRatio[]=
{
    (float32)1.333,
    (float32)0.000,
    (float32)0.000,
    (float32)1.778,
};

const char*  IFO_BitRate_Mode[]=
{
    "VBR",
    "CBR",
};

const size_t IFO_Width[]=
{720, 704, 352, 352,   0,   0,   0,   0};

const size_t IFO_Height[4][8]=
{{480, 480, 480, 240,   0,   0,   0,   0}, //NTSC
 {576, 576, 576, 288,   0,   0,   0,   0}, //PAL
 {  0,   0,   0,   0,   0,   0,   0,   0}, //Unknown
 {  0,   0,   0,   0,   0,   0,   0,   0}, //Unknown
 };

const float64 IFO_FrameRate[]=
{29.970, 25.000};

const char*  IFO_Format_A[]=
{
    "AC-3",
    "",
    "MPEG Audio",
    "MPEG Audio",
    "PCM",
    "",
    "DTS",
    "SDDS",
};

const char*  IFO_Format_Profile_A[]=
{
    "",
    "",
    "Version 1",
    "Version 2",
    "",
    "",
    "",
    "",
};

const char*  IFO_CodecA[]=
{
    "AC3",
    "",
    "MPEG-1A",
    "MPEG-2A",
    "LPCM (Big Endian)",
    "",
    "DTS",
    "SDDS",
};

const char*  IFO_ModeA[]=
{
    "",
    "Karaoke",
    "Surround",
    "",
};

const char*  IFO_ResolutionA[]=
{
    "16",
    "20",
    "24",
    "DRC",
};

const int16u IFO_SamplingRate[]=
{48000, 0, 0, 0, 0, 0, 0, 0};

const char*  IFO_Language_MoreA[]=
{
    "",
    "",
    "For visually impaired",
    "Director's comments",
    "Director's comments",
    "",
    "",
    "",
};

const char*  IFO_Format_T[]=
{
    "RLE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

const char*  IFO_Resolution_T[]=
{
    "2",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

const char*  IFO_CodecT[]=
{
    "2-bit RLE",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

const char*  IFO_Language_MoreT[]=
{
    "",
    "Normal",
    "Large",
    "Children",
    "",
    "",
    "Large",
    "Children",
    "",
    "Forced",
    "",
    "",
    "",
    "Director comments",
    "Director comments large",
    "Director comments children",
};

const size_t IFO_PlaybackTime_FrameRate[]=
{1, 25, 1, 30};

const char*  IFO_MenuType[]=
{
    "",
    "",
    "",
    "root",
    "sub-picture",
    "audio",
    "angle",
    "PTT (chapter)",
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
extern const char*  AC3_ChannelPositions[];
extern const char*  AC3_ChannelPositions2[];

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dvdv::File_Dvdv()
:File__Analyze()
{
    //Temp
    VTS_Attributes_AreHere=false;
    Program_Pos=0;
    Time_Pos=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dvdv::Streams_Finish()
{
    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Sectors.clear();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dvdv::FileHeader_Parse()
{
    //Parsing
    int64u Identifier;
    int32u Type;
    Get_C8 (Identifier,                                         "Identifier");
    Get_C4 (Type,                                               "Type");

    FILLING_BEGIN();
        //Identifier
        if (Identifier!=CC8("DVDVIDEO"))
        {
            Reject("DVD Video");
            return;
        }

        Accept("DVD Video");

        Fill(Stream_General, 0, General_Format, "DVD Video");

        //Versions
        switch (Type)
        {
            case Dvdv::VMG : VMG(); break;
            case Dvdv::VTS : VTS(); break;
            default        :
                        Reject("DVD Video");
                        return;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------


//***************************************************************************
// Elements
//***************************************************************************

#define SUBELEMENT(_ELEMENT) \
    { \
        Element_Begin1(#_ELEMENT); \
        _ELEMENT(); \
        Element_End0(); \
    } \

//---------------------------------------------------------------------------
void File_Dvdv::VMG()
{
    int32u Sector_Pointer_LastSector, Sector_Pointer_TT_SRPT, Sector_Pointer_VMGM_PGCI_UT, Sector_Pointer_VMG_PTL_MAIT, Sector_Pointer_VMG_VTS_ATRT, Sector_Pointer_VMG_TXTDT_MG, Sector_Pointer_VMGM_C_ADT, Sector_Pointer_VMGM_VOBU_ADMAP;
    int16u Version, Audio_Count, Text_Count;
    Element_Info1("DVD Video - VMG");
    Element_Begin1("Header");
        Info_B4(LastSector,                                     "Last sector of VMG set (last sector of BUP)"); Param_Info2((LastSector+1)*2048, " bytes");
        Skip_XX(12,                                             "Unknown");
        Get_B4 (Sector_Pointer_LastSector,                      "last sector of IFO");
        Get_B2 (Version,                                        "version number"); Param_Info1(Ztring::ToZtring((Version&0x00F0)>>4)+__T(".")+Ztring::ToZtring(Version&0x000F));
        Info_B4(Category,                                       "VMG category");
        Skip_B2(                                                "number of volumes");
        Skip_B2(                                                "volume number");
        Skip_B1(                                                "side ID");
        Skip_XX(19,                                             "Unknown");
        Skip_B2(                                                "number of title sets");
        Skip_Local(32,                                          "Provider ID");
        Skip_B8(                                                "VMG POS");
        Skip_XX(24,                                             "Unknown");
        Skip_B4(                                                "end byte address of VMGI_MAT");
        Skip_B4(                                                "start address of FP_PGC (First Play program chain)");
        Skip_XX(56,                                             "Unknown");
        Info_B4(Sector_Pointer_Menu,                            "start sector of Menu VOB");
        Get_B4 (Sector_Pointer_TT_SRPT,                         "sector pointer to TT_SRPT (table of titles)");
        Get_B4 (Sector_Pointer_VMGM_PGCI_UT,                    "sector pointer to VMGM_PGCI_UT (Menu Program Chain table)");
        Get_B4 (Sector_Pointer_VMG_PTL_MAIT,                    "sector pointer to VMG_PTL_MAIT (Parental Management masks)");
        Get_B4 (Sector_Pointer_VMG_VTS_ATRT,                    "sector pointer to VMG_VTS_ATRT (copies of VTS audio/sub-picture attributes)");
        Get_B4 (Sector_Pointer_VMG_TXTDT_MG,                    "sector pointer to VMG_TXTDT_MG (text data)");
        Get_B4 (Sector_Pointer_VMGM_C_ADT,                      "sector pointer to VMGM_C_ADT (menu cell address table)");
        Get_B4 (Sector_Pointer_VMGM_VOBU_ADMAP,                 "sector pointer to VMGM_VOBU_ADMAP (menu VOBU address map)");
        Skip_XX(32,                                             "Unknown");
    Element_End0();

    //-VTSM
    VTS_Attributes_AreHere=true;
    Element_Begin1("VMGM (VMG for Menu)");
        Element_Begin1("Video streams");
            Element_Info2(1, " streams");
            SUBELEMENT(Video)
        Element_End0();
        Element_Begin1("Audio streams");
            Get_B2 (Audio_Count,                                "number of audio streams in VMGM_VOBS");
            Element_Info2(Audio_Count, " streams");
            for (int16u Pos=0; Pos<8; Pos++)
            {
                if (Pos<Audio_Count)
                    SUBELEMENT(Audio)
                else
                    Skip_XX(8,                                  "Reserved for Audio");
            }
            Skip_XX(16,                                         "Unknown");
        Element_End0();
        Element_Begin1("Text streams");
            Get_B2 (Text_Count,                                 "number of subpicture streams in VMGM_VOBS");
            Element_Info2(Text_Count, " streams");
            for (int16u Pos=0; Pos<1; Pos++)
            {
                if (Pos<Text_Count)
                    SUBELEMENT(Text)
                else
                    Skip_XX(6,                                  "Reserved for Text");
            }
            Skip_XX(164,                                        "Unknown");
        Element_End0();
    Element_End0();
    Skip_XX(2048-Element_Offset,                                "Junk");

    //Filling
    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Format_Profile, "Menu");

        if (Version>0x001F)
            return;
        Sectors.resize(Sector_Pointer_LastSector+1);
        if (Sector_Pointer_TT_SRPT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_TT_SRPT]=Sector_TT_SRPT;
        if (Sector_Pointer_VMGM_PGCI_UT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VMGM_PGCI_UT]=Sector_VMGM_PGCI_UT;
        if (Sector_Pointer_VMG_PTL_MAIT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VMG_PTL_MAIT]=Sector_VMG_PTL_MAIT;
        if (Sector_Pointer_VMG_VTS_ATRT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VMG_VTS_ATRT]=Sector_VMG_VTS_ATRT;
        if (Sector_Pointer_VMG_TXTDT_MG<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VMG_TXTDT_MG]=Sector_VMG_TXTDT_MG;
        if (Sector_Pointer_VMGM_C_ADT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VMGM_C_ADT]=Sector_VMGM_C_ADT;
        if (Sector_Pointer_VMGM_VOBU_ADMAP<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VMGM_VOBU_ADMAP]=Sector_VMGM_VOBU_ADMAP;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dvdv::VTS()
{
    //Parsing
    int32u Sector_Pointer_LastSector, Sector_Pointer_VTS_PTT_SRPT, Sector_Pointer_VTS_PGCI, Sector_Pointer_VTSM_PGCI_UT, Sector_Pointer_VTS_TMAPTI, Sector_Pointer_VTSM_C_ADT, Sector_Pointer_VTSM_VOBU_ADMAP, Sector_Pointer_VTS_C_ADT, Sector_Pointer_VTS_VOBU_ADMAP;
    int16u Version, Audio_Count, Text_Count;
    Element_Info1("DVD Video - VTS (Video Title Set)");
    Element_Begin1("Header");
        Info_B4(LastSector,                                     "Last sector of Title set (last sector of BUP)"); Param_Info2((LastSector+1)*2048, " bytes");
        Skip_XX(12,                                             "Unknown");
        Get_B4 (Sector_Pointer_LastSector,                      "last sector of IFO");
        Get_B2 (Version,                                        "version number"); Param_Info1(Ztring::ToZtring((Version&0x00F0)>>4)+__T(".")+Ztring::ToZtring(Version&0x000F));
        Info_B4(Category,                                       "VTS category");
        #if MEDIAINFO_TRACE
            if (Category<2) Param_Info1(IFO_VTS_Category[Category]);
        #endif //MEDIAINFO_TRACE
        Skip_XX(90,                                             "Unknown");
        Skip_B4(                                                "end byte address of VTS_MAT");
        Skip_XX(60,                                             "Unknown");
        Info_B4(StartSector_Menu,                               "start sector of Menu VOB"); Param_Info2((StartSector_Menu+1)*2048, " bytes");
        Info_B4(StartSector_Title,                              "start sector of Title Vob"); Param_Info2((StartSector_Title+1)*2048, " bytes");
        Get_B4 (Sector_Pointer_VTS_PTT_SRPT,                    "sector pointer to VTS_PTT_SRPT (Table of Titles and Chapters)");
        Get_B4 (Sector_Pointer_VTS_PGCI,                        "sector pointer to VTS_PGCI (Title Program Chain table)");
        Get_B4 (Sector_Pointer_VTSM_PGCI_UT,                    "sector pointer to VTSM_PGCI_UT (Menu Program Chain table)");
        Get_B4 (Sector_Pointer_VTS_TMAPTI,                      "sector pointer to VTS_TMAPTI (Time map)");
        Get_B4 (Sector_Pointer_VTSM_C_ADT,                      "sector pointer to VTSM_C_ADT (Menu cell address table)");
        Get_B4 (Sector_Pointer_VTSM_VOBU_ADMAP,                 "sector pointer to VTSM_VOBU_ADMAP(menu VOBU address map)");
        Get_B4 (Sector_Pointer_VTS_C_ADT,                       "sector pointer to VTS_C_ADT (Title set cell address table)");
        Get_B4 (Sector_Pointer_VTS_VOBU_ADMAP,                  "sector pointer to VTS_VOBU_ADMAP (Title set VOBU address map)");
        Skip_XX(24,                                             "Unknown");
    Element_End0();

    //-VTSM
    Element_Begin1("VTSM (VTS for Menu, Vob 0)");
        Element_Begin1("Video streams");
            Element_Info2(1, " streams");
            SUBELEMENT(Video)
        Element_End0();
        Element_Begin1("Audio streams");
            Get_B2 (Audio_Count,                                "number of audio streams in VTSM_VOBS");
            Element_Info2(Audio_Count, " streams");
            for (int16u Pos=0; Pos<8; Pos++)
            {
                if (Pos<Audio_Count)
                    SUBELEMENT(Audio)
                else
                    Skip_XX(8,                                  "Reserved for Audio");
            }
            Skip_XX(16,                                         "Unknown");
        Element_End0();
        Element_Begin1("Text streams");
            Get_B2 (Text_Count,                                 "number of subpicture streams in VTSM_VOBS");
            Element_Info2(Text_Count, " streams");
            for (int16u Pos=0; Pos<1; Pos++)
            {
                if (Pos<Text_Count)
                    SUBELEMENT(Text)
                else
                    Skip_XX(6,                                  "Reserved for Text");
            }
            Skip_XX(164,                                        "Unknown");
        Element_End0();
    Element_End0();

    //-VTS
    VTS_Attributes_AreHere=true;
    Element_Begin1("VTS (VTS for movie, Vob 1-9)");
        Element_Begin1("Video streams");
            Element_Info2(1, " streams");
            SUBELEMENT(Video)
        Element_End0();
        Element_Begin1("Audio streams");
            Get_B2 (Audio_Count,                                "number of audio streams in VMGM_VOBS");
            Element_Info2(Audio_Count, " streams");
            for (int16u Pos=0; Pos<8; Pos++)
            {
                if (Pos<Audio_Count)
                    SUBELEMENT(Audio)
                else
                    Skip_XX(8,                                  "Reserved for Audio");
            }
            Skip_XX(16,                                         "Unknown");
        Element_End0();
        Element_Begin1("Text streams");
            Get_B2 (Text_Count,                                 "number of subpicture streams in VMGM_VOBS");
            Element_Info2(Text_Count, " streams");
            for (int16u Pos=0; Pos<32; Pos++)
            {
                if (Pos<Text_Count)
                    SUBELEMENT(Text)
                else
                    Skip_XX(6,                                  "Reserved for Text");
            }
            Skip_XX(2,                                          "Unknown");
        Element_End0();
        Element_Begin1("MultiChannel Info");
            Element_Info2(Audio_Count, " streams");
            for (int16u Pos=0; Pos<8; Pos++)
            {
                if (Pos<Audio_Count)
                    SUBELEMENT(MultiChannel)
                else
                    Skip_XX(24,                                 "Reserved for multichannel extension");
            }
        Element_End0();
    Element_End0();
    Skip_XX(2048-Element_Offset,                                "Junk");

    //Filling
    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Format_Profile, "Program");

        if (Version>0x001F)
            return;
        if (Sector_Pointer_LastSector==(int32u)-1 || Sector_Pointer_LastSector+1>File_Size/2048)
            Sector_Pointer_LastSector=(int32u)(File_Size/2048);
        Sectors.resize(Sector_Pointer_LastSector+1);
        if (Sector_Pointer_VTS_PTT_SRPT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTS_PTT_SRPT]=Sector_VTS_PTT_SRPT;
        if (Sector_Pointer_VTS_PGCI<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTS_PGCI]=Sector_VTS_PGCI;
        if (Sector_Pointer_VTSM_PGCI_UT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTSM_PGCI_UT]=Sector_VTSM_PGCI_UT;
        if (Sector_Pointer_VTS_TMAPTI<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTS_TMAPTI]=Sector_VTS_TMAPTI;
        if (Sector_Pointer_VTSM_C_ADT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTSM_C_ADT]=Sector_VTSM_C_ADT;
        if (Sector_Pointer_VTSM_VOBU_ADMAP<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTSM_VOBU_ADMAP]=Sector_VTSM_VOBU_ADMAP;
        if (Sector_Pointer_VTS_C_ADT<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTS_C_ADT]=Sector_VTS_C_ADT;
        if (Sector_Pointer_VTS_VOBU_ADMAP<=Sector_Pointer_LastSector)
            Sectors[Sector_Pointer_VTS_VOBU_ADMAP]=Sector_VTS_VOBU_ADMAP;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dvdv::Video()
{
    //Parsing
    int32u Codec, Standard, AspectRatio, Resolution, BitRate_Mode;
    BS_Begin();
    Get_BS (2, Codec,                                           "Coding mode"); Param_Info1(IFO_CodecV[Codec]);
    Get_BS (2, Standard,                                        "Standard"); Param_Info1(IFO_Standard[Standard]);
    Get_BS (2, AspectRatio,                                     "Aspect ratio"); Param_Info1(IFO_AspectRatio[AspectRatio]);
    Info_BS(1, Pan,                                             "Automatic Pan/Scan"); Param_Info1(Pan?"No":"Yes");
    Info_BS(1, Letter,                                          "Automatic Letterbox"); Param_Info1(Letter?"No":"Yes");
    Skip_BS(1,                                                  "CC for line 21 field 1 in GOP (NTSC only)");
    Skip_BS(1,                                                  "CC for line 21 field 2 in GOP (NTSC only)");
    Get_BS (3, Resolution,                                      "Resolution"); Param_Info1(Ztring::ToZtring(IFO_Width[Resolution])+__T("x")+Ztring::ToZtring(IFO_Height[Standard][Resolution]));
    Info_BS(1, Letterboxed,                                     "Letterboxed"); Param_Info1(Letter?"Yes":"No");
    Get_BS (1, BitRate_Mode,                                    "Bitrate mode"); Param_Info1(IFO_BitRate_Mode[BitRate_Mode]);
    Info_BS(1, Camera,                                          "Camera/Film"); Param_Info1(Letter?"Film":"Camera");
    BS_End();

    //Filling
    FILLING_BEGIN();
        if (VTS_Attributes_AreHere)
        {
            Stream_Prepare(Stream_Video);
            Fill(Stream_Video, StreamPos_Last, Video_Format, IFO_Format_V[Codec]);
            Fill(Stream_Video, StreamPos_Last, Video_Format_Version, IFO_Format_Version_V[Codec]);
            Fill(Stream_Video, StreamPos_Last, Video_Codec, IFO_CodecV[Codec]);
            Fill(Stream_Video, StreamPos_Last, Video_Width, IFO_Width[Resolution]);
            Fill(Stream_Video, StreamPos_Last, Video_Height, IFO_Height[Standard][Resolution]);
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, IFO_AspectRatio[AspectRatio], 3, true);
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, IFO_FrameRate[Standard]);
            Fill(Stream_Video, StreamPos_Last, Video_BitRate_Mode, IFO_BitRate_Mode[BitRate_Mode]);
            Fill(Stream_Video, StreamPos_Last, General_ID, __T("224"));
            Fill(Stream_Video, StreamPos_Last, General_ID_String, __T("224 (0xE0)"), Unlimited, true);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dvdv::Audio()
{
    //Parsing
    Ztring Language;
    int32u Codec, LanguageType, Mode, Resolution, SamplingRate, Channels;
    int8u Language_Extension, ChannelsK=(int8u)-1;
    BS_Begin();
    Get_BS (3, Codec,                                           "Coding mode"); Param_Info1(IFO_CodecA[Codec]);
    Info_BS(1, MultiChannel,                                    "Multichannel extension present"); Param_Info1(MultiChannel?"Yes":"No");
    Get_BS (2, LanguageType,                                    "Language type"); Param_Info1(LanguageType==1?"2CC":"Unknown");
    Get_BS (2, Mode,                                            "Application mode"); Param_Info1(IFO_ModeA[Mode]);
    Get_BS (2, Resolution,                                      "Resolution"); Param_Info1C((Codec==2 || Codec==3), IFO_ResolutionA[Resolution]); Param_Info1C((Codec==4), Mode?"DRC":"No DRC");
    Get_BS (2, SamplingRate,                                    "Sampling rate"); Param_Info1(Ztring::ToZtring(IFO_SamplingRate[SamplingRate]));
    Get_BS (4, Channels,                                        "Channels"); Param_Info2(Channels+1, " channels");
    BS_End();
    Get_Local(3, Language,                                      "Language code");
    if (!Language.empty() && Language[0]>=0x80)
        Language.clear(); //this is 0xFF...
    if (Language==__T("iw"))
        Language=__T("he"); //Hebrew patch, is "iw" in DVDs
    Get_B1 (Language_Extension,                                 "Language extension"); Param_Info1C((Language_Extension<8), IFO_Language_MoreA[Language_Extension]);
    Skip_B1(                                                    "Unknown");
    switch (Mode)
    {
        case 1 : //Karaoke
            {
            BS_Begin();
            Skip_BS(1,                                          "Zero");
            Get_S1 (3, ChannelsK,                               "Channels");
                #ifdef MEDIAINFO_AC3_YES
                    Param_Info1(AC3_ChannelPositions[ChannelsK]);
                #endif //MEDIAINFO_AC3_YES
            Skip_BS(2,                                          "Version");
            Info_BS(1, MC,                                      "MC intro present"); Param_Info1(MC?"Yes":"No");
            Info_BS(1, Duet,                                    "Duet"); Param_Info1(Duet?"Duet":"Solo");
            BS_End();
            }
            break;
        case 2 : //Surround
            {
            BS_Begin();
            Skip_BS(4,                                          "Reserved");
            Info_BS(1, DolbyDecode,                             "Suitable for Dolby surround decoding"); Param_Info1(DolbyDecode?"Yes":"No");
            Skip_BS(3,                                          "Reserved");
            BS_End();
            }
            break;
    default:
            {
            Skip_B1(                                            "Reserved");
            }
    }

    //Filling
    FILLING_BEGIN();
        if (VTS_Attributes_AreHere)
        {
            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, StreamPos_Last, Audio_Format, IFO_Format_A[Codec]);
            Fill(Stream_Audio, StreamPos_Last, Audio_Format_Profile, IFO_Format_Profile_A[Codec]);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, IFO_CodecA[Codec]);
            Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, IFO_SamplingRate[SamplingRate]);
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels+1);
            if (Codec==3)
                Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, IFO_ResolutionA[Resolution]);
            else if (Codec==4 && Mode)
                Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, "DRC");
            Fill(Stream_Audio, StreamPos_Last, Audio_Language, Language);
            if (Language_Extension<8)
                Fill(Stream_Audio, StreamPos_Last, Audio_Language_More, IFO_Language_MoreA[Language_Extension]);
            #ifdef MEDIAINFO_AC3_YES
                if (Codec==0 && ChannelsK!=(int8u)-1) //AC-3
                {
                    Fill(Stream_Audio, 0, Audio_ChannelPositions, AC3_ChannelPositions[ChannelsK]);
                    Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, AC3_ChannelPositions2[ChannelsK]);
                }
            #endif //MEDIAINFO_AC3_YES
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dvdv::Text()
{
    //Parsing
    Ztring Language;
    int32u Codec, LanguageType;
    int8u Language_Extension;
    BS_Begin();
    Get_BS (3, Codec,                                           "Coding mode"); Param_Info1(IFO_CodecT[Codec]);
    Skip_BS(3,                                                  "Reserved");
    Get_BS (2, LanguageType,                                    "Language type"); Param_Info1(LanguageType==1?"2CC":"Unknown");
    BS_End();
    Skip_B1(                                                    "Reserved");
    Get_Local(3, Language,                                      "Language code");
    if (!Language.empty() && Language[0]>=0x80)
        Language.clear(); //this is 0xFF...
    if (Language==__T("iw"))
        Language=__T("he"); //Hebrew patch, is "iw" in DVDs
    Get_B1 (Language_Extension,                                 "Language extension"); Param_Info1C((Language_Extension<16), IFO_Language_MoreT[Language_Extension]);

    //Filling
    FILLING_BEGIN();
        if (VTS_Attributes_AreHere)
        {
            Stream_Prepare(Stream_Text);
            Fill(Stream_Text, StreamPos_Last, Text_Format, IFO_Format_T[Codec]);
            Fill(Stream_Text, StreamPos_Last, Text_BitDepth, IFO_Resolution_T[Codec]);
            Fill(Stream_Text, StreamPos_Last, Text_Codec, IFO_CodecT[Codec]);
            Fill(Stream_Text, StreamPos_Last, Text_Language, Language);

            if (Language_Extension<16)
                 Fill(Stream_Text, StreamPos_Last, Text_Language_More, IFO_Language_MoreT[Language_Extension]);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dvdv::MultiChannel()
{
    //Parsing
    BS_Begin();
    Element_Begin1("ACH0");
        Skip_BS(7,                                              "Reserved");
        Skip_BS(1,                                              "ACH0 Guide Melody exists");
    Element_End0();
    Element_Begin1("ACH1");
        Skip_BS(7,                                              "Reserved");
        Skip_BS(1,                                              "ACH1 Guide Melody exists");
    Element_End0();
    Element_Begin1("ACH2");
        Skip_BS(4,                                              "Reserved");
        Skip_BS(1,                                              "ACH2 Guide Vocal 1 exists");
        Skip_BS(1,                                              "ACH2 Guide Vocal 2 exists");
        Skip_BS(1,                                              "ACH2 Guide Melody 1 exists");
        Skip_BS(1,                                              "ACH2 Guide Melody 2 exists");
    Element_End0();
    Element_Begin1("ACH3");
        Skip_BS(4,                                              "Reserved");
        Skip_BS(1,                                              "ACH3 Guide Vocal 1 exists");
        Skip_BS(1,                                              "ACH3 Guide Vocal 2 exists");
        Skip_BS(1,                                              "ACH3 Guide Melody A exists");
        Skip_BS(1,                                              "ACH3 Sound Effect A exists");
    Element_End0();
    Element_Begin1("ACH4");
        Skip_BS(4,                                              "Reserved");
        Skip_BS(1,                                              "ACH4 Guide Vocal 1 exists");
        Skip_BS(1,                                              "ACH4 Guide Vocal 2 exists");
        Skip_BS(1,                                              "ACH4 Guide Melody B exists");
        Skip_BS(1,                                              "ACH4 Sound Effect B exists");
    Element_End0();
    BS_End();
    Skip_XX(19,                                                 "Unknown");
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dvdv::Header_Parse()
{
    //Calculating
    size_t Sector_Pos=(size_t)((File_Offset+Buffer_Offset)/2048);
    size_t Sector_Count=1;
    while (Sector_Pos+Sector_Count<Sectors.size() && Sectors[Sector_Pos+Sector_Count]==Sector_Nothing)
        Sector_Count++;

    //Filling
    Header_Fill_Size(Sector_Count*2048);
}

//---------------------------------------------------------------------------
void File_Dvdv::Data_Parse()
{
    //Parsing
    size_t Sector_Pos=(size_t)((File_Offset+Buffer_Offset)/2048);
    if (Sector_Pos>=Sectors.size())
    {
        Accept("DVD Video");
        Finish("DVD Video");
        return;
    }

    //Parsing
    switch(Sectors[Sector_Pos])
    {
        case Sector_VTS_PTT_SRPT    : VTS_PTT_SRPT(); break;
        case Sector_VTS_PGCI        : VTS_PGCI(); break;
        case Sector_VTSM_PGCI_UT    : VTSM_PGCI_UT(); break;
        case Sector_VTS_TMAPTI      : VTS_TMAPTI(); break;
        case Sector_VTSM_C_ADT      : VTSM_C_ADT(); break;
        case Sector_VTSM_VOBU_ADMAP : VTSM_VOBU_ADMAP(); break;
        case Sector_VTS_C_ADT       : VTS_C_ADT(); break;
        case Sector_VTS_VOBU_ADMAP  : VTS_VOBU_ADMAP(); break;
        case Sector_TT_SRPT         : TT_SRPT(); break;
        case Sector_VMGM_PGCI_UT    : VMGM_PGCI_UT(); break;
        case Sector_VMG_PTL_MAIT    : VMG_PTL_MAIT(); break;
        case Sector_VMG_VTS_ATRT    : VMG_VTS_ATRT(); break;
        case Sector_VMG_TXTDT_MG    : VMG_TXTDT_MG(); break;
        case Sector_VMGM_C_ADT      : VMGM_C_ADT(); break;
        case Sector_VMGM_VOBU_ADMAP : VMGM_VOBU_ADMAP(); break;
        default                     : ;
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dvdv::VTS_PTT_SRPT ()
{
    Element_Name("Table of Titles and Chapters");

    //Parsing
    int32u Element_RealSize;
    Element_Begin1("Header");
        Skip_B2(                                                "Count of elements");
        Skip_B2(                                                "Unknown");
        Get_B4 (Element_RealSize,                               "End address");
        Element_RealSize++; //Last byte
    Element_End0();
    Element_Begin1("Extra data");
        int32u Offset;
        Get_B4 (Offset,                                         "Offset of first element");
        int64u Extra_Size=Offset-Element_Offset;
        if (Extra_Size>0)
            Skip_XX(Extra_Size,                                 "Extra data (Unknown)");
    Element_End0();

    //For each chapter
    while (Element_Offset<Element_RealSize)
    {
        //VTS_PTT
        int16u PGCN, PGN;
        Element_Begin0();
        Get_B2 (PGCN,                                           "Program Chain (PGCN)");
        Get_B2 (PGN,                                            "Program (PGN)");
        Element_Name("Chapter"); Element_Info1(Ztring::ToZtring(PGCN)); Element_Info1(Ztring::ToZtring(PGN));
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTS_PGCI ()
{
    Element_Name("Title Program Chain table");

    //Parsing
    int32u EndAddress;
    Element_Begin1("Header");
        int32u Offset;
        Skip_B2(                                                "Number of Program Chains");
        Skip_B2(                                                "Reserved");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
        Element_Begin1("PGC category");
            BS_Begin();
            Skip_BS(1,                                          "entry PGC");
            Skip_BS(7,                                          "title number");
            BS_End();
            Skip_B1(                                            "Unknown");
            Skip_B2(                                            "parental management mask");
        Element_End0();
        Get_B4 (Offset,                                         "offset to VTS_PGC - relative to VTS_PGCI");
        if (Offset-16>0)
            Skip_XX(Offset-16,                                  "Unknown");
    Element_End0();

    //For each Program
    //DETAILLEVEL_SET(1.0);
    while (Element_Offset<=EndAddress)
    {
        PGC(Offset, true);
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTSM_PGCI_UT ()
{
    Element_Name("Menu Program Chain table");

    //Parsing
    int16u LU_Count;
    Element_Begin1("Header");
        int32u EndAddress, Offset;
        int8u Flags;
        Get_B2 (LU_Count,                                       "Number of Language Units");
        Skip_B2(                                                "Reserved");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
        Skip_C3(                                                "Language");
        Get_B1 (Flags,                                          "Menu existence flags");
            Skip_Flags(Flags, 3,                                "PTT");
            Skip_Flags(Flags, 4,                                "angle");
            Skip_Flags(Flags, 5,                                "audio");
            Skip_Flags(Flags, 6,                                "sub-picture");
            Skip_Flags(Flags, 7,                                "root");
        Get_B4 (Offset,                                         "Offset to VTSM_LU relative to VTSM_PGCI_UT");
        if (Offset-16>0)
            Skip_XX(Offset-16,                                  "Unknown");
    Element_End0();

    for (int16u LU_Pos=0; LU_Pos<LU_Count; LU_Pos++)
    {
        Element_Begin1("Language Unit");
        int32u LU_Size;
        int16u PGC_Count;
        Element_Begin1("Header");
            Get_B2 (PGC_Count,                                  "Number of Program Chains");
            Skip_B2(                                            "Reserved");
            Get_B4 (LU_Size,                                    "end address (last byte of last PGC in this LU) relative to VTSM_LU");
            LU_Size++; //Last byte
            Element_Begin1("PGC category");
                int32u EntryPGC;
                BS_Begin();
                Get_BS (1, EntryPGC,                            "Entry PGC");
                Skip_BS(3,                                      "Unknown");
                if (EntryPGC)
                {
                    Info_BS(4, MenuType,                        "menu type"); Param_Info1(IFO_MenuType[MenuType]);
                }
                else
                {
                    Skip_BS(4,                                  "Reserved");
                }
                BS_End();
                Skip_B1(                                        "Unknown");
                Skip_B2(                                        "parental management mask");
            Element_End0();
            Get_B4 (Offset,                                     "offset to VTSM_PGC relative to VTSM_LU");
            if (Offset-16>0)
                Skip_XX(Offset-16,                              "Unknown");
        Element_End0();
        for (int16u PGC_Pos=0; PGC_Pos<PGC_Count; PGC_Pos++)
            PGC(Element_Offset);

        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTS_TMAPTI ()
{
    Element_Name("Time map");

    //Parsing
    Element_Begin1("Header");
        int32u EndAddress, Offset;
        Skip_B2(                                                "Number of program chains");
        Skip_B2(                                                "Reserved");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
        Get_B4 (Offset,                                         "Offset to VTS_TMAP 1");
        if (Offset-12>0)
            Skip_XX(Offset-12,                                  "Unknown");
    Element_End0();

    //DETAILLEVEL_SET(1.0);
    while (Element_Offset<=EndAddress)
    {
        //VTS_TMAP
        Element_Begin1("Time Map");
        //std::vector<size_t> Sector_Times;
        int8u Sector_Times_SecondsPerTime;
        int16u Count;
        Get_B1 (Sector_Times_SecondsPerTime,                    "Time unit (seconds)");
        Skip_B1(                                                "Unknown");
        Get_B2 (Count,                                          "Number of entries in map");
        //Sector_Times.resize(Count);
        BS_Begin();
        for (int16u Pos=0; Pos<Count; Pos++)
        {
            Element_Begin1("Sector Offset");
            int32u SectorOffset;
            Skip_BS( 1,                                         "discontinuous with previous");
            Get_BS (31, SectorOffset,                           "Sector offset within VOBS of nearest VOBU");
            //Get_B4 (Sector_Times[Pos],                          Sector offset within VOBS of nearest VOBU);// Param_Info1(Ztring().Duration_From_Milliseconds((Pos+1)*Sectors_Times_SecondsPerTime[Program_Pos]));
            //Sector_Times[Pos]&=0x7FFFFFFF; //bit 31 is set if VOBU time codes are discontinuous with previous
            Element_Info1(SectorOffset);
            Element_End0();
        }
        BS_End();
        Element_End0();

        //Filling
        //Sectors_Times.push_back(Sector_Times);
        //Sectors_Times_SecondsPerTime.push_back(Sector_Times_SecondsPerTime);
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTSM_C_ADT ()
{
    Element_Name("Menu cell address table");

    //Parsing
    int32u EndAddress;
    Element_Begin1("Header");
        Skip_B2(                                                "Number of cells");
        Skip_B2(                                                "Reserved");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
    Element_End0();

    //DETAILLEVEL_SET(1.0);
    while (Element_Offset<=EndAddress)
    {
        //ADT
        Element_Begin1("Entry");
        Skip_B2(                                                "VOBidn");
        Skip_B1(                                                "CELLidn");
        Skip_B1(                                                "Unknown");
        Skip_B4(                                                "Starting sector within VOB");
        Skip_B4(                                                "Ending sector within VOB");
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTSM_VOBU_ADMAP ()
{
    Element_Name("Menu VOBU address map");

    //Parsing
    int32u EndAddress;
    Element_Begin1("Header");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
    Element_End0();

    //DETAILLEVEL_SET(1.0);
    while (Element_Offset<=EndAddress)
    {
        //ADMAP
        Skip_B4(                                                "Starting sector within VOB of first VOBU");
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTS_C_ADT ()
{
    Element_Name("Title set cell address table");

    //Parsing
    int32u EndAddress;
    Element_Begin1("Header");
        Skip_B2(                                                "Number of cells");
        Skip_B2(                                                "Reserved");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
    Element_End0();

    //DETAILLEVEL_SET(1.0);
    while (Element_Offset<=EndAddress)
    {
        //ADT
        Element_Begin1("Entry");
        int32u Start, End;
        int16u VOBidn;
        int8u CELLidn;
        Get_B2 (VOBidn,                                         "VOBidn");
        Get_B1 (CELLidn,                                        "CELLidn");
        Skip_B1(                                                "Unknown");
        Get_B4 (Start,                                          "Starting sector within VOB"); Param_Info1(Time_ADT(Start));
        Get_B4 (End,                                            "Ending sector within VOB"); Param_Info1(Time_ADT(End));
        Element_End0();

        //Filling
        FILLING_BEGIN();
        FILLING_END();
        //Fill(Ztring::ToZtring(CELLidn).To_Local().c_str(), Time_ADT(Start));
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VTS_VOBU_ADMAP ()
{
    Element_Name("Title set VOBU address map");

    //Parsing
    int32u EndAddress;
    Element_Begin1("Header");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
    Element_End0();

    //DETAILLEVEL_SET(1.0);
    while (Element_Offset<Element_Size)
    {
        //ADMAP
        Skip_B4(                                                "Starting sector within VOB of first VOBU");
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
Ztring File_Dvdv::Time_ADT(int32u)
{
    return Ztring();
    /*
    if (Sectors_Times.empty())
        return Ztring(); //TODO: it can be empty?

    while (Time_Pos<Sectors_Times[Program_Pos].size() && Sectors_Times[Program_Pos][Time_Pos]<Value)
        Time_Pos++;
    if (Time_Pos<Sectors_Times[Program_Pos].size())
    {
        int32u Time=(Time_Pos+1)*Sectors_Times_SecondsPerTime[Program_Pos]*1000;
        float32 Part;
        //True time is between Time and Time+Sectors_Times_SecondsPerTime, finding where...
        int32u Sectors_Count;
        if (Time_Pos==0)
            Sectors_Count=Sectors_Times[Program_Pos][Time_Pos];
        else
            Sectors_Count=Sectors_Times[Program_Pos][Time_Pos]-Sectors_Times[Program_Pos][Time_Pos-1];
        Part=(Sectors_Times[Program_Pos][Time_Pos]-Value); //Count of more sectors after
        Part/=Sectors_Count; //Percentage
        Time=(int32u)((Time_Pos+1-Part)*Sectors_Times_SecondsPerTime[Program_Pos]*1000);

        return Ztring().Duration_From_Milliseconds(Time*1000);
    }
    else
    {
        int32u Time=(Time_Pos+1)*Sectors_Times_SecondsPerTime[Program_Pos]*1000;
        float32 Part;
        //True time is between Time and Time+Sectors_Times_SecondsPerTime, finding where... but with the last offset diffrence
        int32u Sectors_Count=Sectors_Times[Program_Pos][Time_Pos-1]-Sectors_Times[Program_Pos][Time_Pos-2];
        Part=((int32s)Sectors_Times[Program_Pos][Time_Pos-1])-((int32s)Value); //Count of more sectors after
        Part/=Sectors_Count; //Percentage
        Part+=1; //We were one offset less
        Time=(int32u)((Time_Pos+1-Part)*Sectors_Times_SecondsPerTime[Program_Pos]*1000);

        return Ztring().Duration_From_Milliseconds(Time*1000);
    }
    */
}

void File_Dvdv::Get_Duration(int64u  &Duration, const Ztring &Name)
{
    int32u FrameRate, FF;
    int8u HH, MM, Sec;
    Element_Begin1(Name);
        Get_B1 (HH,                                     "Hours (BCD)");
        Get_B1 (MM,                                     "Minutes (BCD)");
        Get_B1 (Sec,                                     "Seconds (BCD)");
        BS_Begin();
        Get_BS (2, FrameRate,                           "Frame rate"); Param_Info2(IFO_PlaybackTime_FrameRate[FrameRate], " fps");
        Get_BS (6, FF,                                  "Frames (BCD)");
        BS_End();

        Duration= Ztring::ToZtring(HH, 16).To_int64u() * 60 * 60 * 1000 //BCD
                + Ztring::ToZtring(MM, 16).To_int64u()      * 60 * 1000 //BCD
                + Ztring::ToZtring(Sec, 16).To_int64u()          * 1000 //BCD
                + Ztring::ToZtring(FF, 16).To_int64u()           * 1000/IFO_PlaybackTime_FrameRate[FrameRate]; //BCD

        Element_Info1(Ztring::ToZtring(Duration));
    Element_End0();
}


void File_Dvdv::PGC(int64u Offset, bool Title)
{
        vector<int8u> Stream_Control_Audio;
        vector<int8u> Stream_Control_SubPicture_43;
        vector<int8u> Stream_Control_SubPicture_Wide;
        vector<int8u> Stream_Control_SubPicture_Letterbox;
        vector<int8u> Stream_Control_SubPicture_PanScan;
        vector<int64u> CellDurations;
        vector<int8u> ProgramMap;

        //VTS_PGC
        Element_Begin1("PGC");
        int16u commands, program_map, cell_playback, cell_position;
        int8u Program_Count;
        Element_Begin1("Header");
            int32u Flags;
            int8u Cells;
            int64u TotalDuration;
            Skip_B2(                                            "Unknown");
            Get_B1 (Program_Count,                              "number of programs");
            Get_B1 (Cells,                                      "number of cells");
            Get_Duration(TotalDuration,                         "Duration");
            Get_B4 (Flags,                                      "prohibited user ops");
                /*Skip_Flags(Flags,  0,                           "Time play or search");
                Skip_Flags(Flags,  1, PTT play or search);
                Skip_Flags(Flags,  2, Title play);
                Skip_Flags(Flags,  3, Stop);
                Skip_Flags(Flags,  4, GoUp);
                Skip_Flags(Flags,  5, Time or PTT search);
                Skip_Flags(Flags,  6, TopPG or PrevPG search);
                Skip_Flags(Flags,  7, NextPG search);
                Skip_Flags(Flags,  8, Forward scan);
                Skip_Flags(Flags,  9, Backward scan);
                Skip_Flags(Flags, 10, Menu call - Title);
                Skip_Flags(Flags, 11, Menu call - Root);
                Skip_Flags(Flags, 12, Menu call - Subpicture);
                Skip_Flags(Flags, 13, Menu call - Audio);
                Skip_Flags(Flags, 14, Menu call - Angle);
                Skip_Flags(Flags, 15, Menu call - PTT);
                Skip_Flags(Flags, 16, Resume);
                Skip_Flags(Flags, 17, Button select or activate);
                Skip_Flags(Flags, 18, Still off);
                Skip_Flags(Flags, 19, Pause on);
                Skip_Flags(Flags, 20, Audio stream change);
                Skip_Flags(Flags, 21, Subpicture stream change);
                Skip_Flags(Flags, 22, Angle change);
                Skip_Flags(Flags, 23, Karaoke audio mix change);
                Skip_Flags(Flags, 24, Video presentation mode change);
                */
                /*
                Skip_Flags(Flags,  0, Video presentation mode change);
                Skip_Flags(Flags,  1, Karaoke audio mix change);
                Skip_Flags(Flags,  2, Angle change);
                Skip_Flags(Flags,  3, Subpicture stream change);
                Skip_Flags(Flags,  4, Audio stream change);
                Skip_Flags(Flags,  5, Pause on);
                Skip_Flags(Flags,  6, Still off);
                Skip_Flags(Flags,  7, Button select or activate);
                Skip_Flags(Flags,  8, Resume);
                Skip_Flags(Flags,  9, Menu call - PTT);
                Skip_Flags(Flags, 10, Menu call - Angle);
                Skip_Flags(Flags, 11, Menu call - Audio);
                Skip_Flags(Flags, 12, Menu call - Subpicture);
                Skip_Flags(Flags, 13, Menu call - Root);
                Skip_Flags(Flags, 14, Menu call - Title);
                Skip_Flags(Flags, 15, Backward scan);
                Skip_Flags(Flags, 16, Forward scan);
                Skip_Flags(Flags, 17, NextPG search);
                Skip_Flags(Flags, 18, TopPG or PrevPG search);
                Skip_Flags(Flags, 19, Time or PTT search);
                Skip_Flags(Flags, 20, GoUp);
                Skip_Flags(Flags, 21, Stop);
                Skip_Flags(Flags, 22, Title play);
                Skip_Flags(Flags, 23, PTT play or search);
                Skip_Flags(Flags, 24,                               Time play or search);
                */
            Element_Begin1("Audio Stream Controls");
            for (size_t Pos=0; Pos<8; Pos++)
            {
                Element_Begin1("Audio Stream Control");
                Element_Info1(Ztring::ToZtring(Pos));
                int8u Number;
                bool  Available;
                BS_Begin();
                Get_SB (   Available,                           "Stream available");
                Get_S1 (7, Number,                              "Stream number");
                BS_End();
                Skip_B1(                                        "Reserved");
                Element_End0();
                if (Available)
                    Stream_Control_Audio.push_back(Number);

                if (Available && Retrieve(Stream_Audio, Pos, Text_ID).empty() && Sectors[(size_t)((File_Offset+Buffer_Offset)/2048)]==Sector_VTS_PGCI)
                {
                    while (Pos>Count_Get(Stream_Audio))
                        Stream_Prepare(Stream_Audio);

                    int8u ToAdd=0;
                    if (Retrieve(Stream_Audio, Pos, Audio_Format)==__T("AC-3"))
                        ToAdd=0x80;
                    if (Retrieve(Stream_Audio, Pos, Audio_Format)==__T("DTS"))
                        ToAdd=0x88;
                    if (Retrieve(Stream_Audio, Pos, Audio_Format)==__T("LPCM"))
                        ToAdd=0xA0;
                    Ztring ID_String; ID_String.From_Number(ToAdd+Number); ID_String+=__T(" (0x"); ID_String+=Ztring::ToZtring(ToAdd+Number, 16); ID_String+=__T(")");
                    Fill(Stream_Audio, Pos, Audio_ID, ID_String);
                    Fill(Stream_Audio, Pos, Audio_ID_String, ID_String, true);
                }
            }
            Element_End0();
            Element_Begin1("Subpicture Stream Controls");
            for (size_t Pos=0; Pos<32; Pos++)
            {
                Element_Begin1("Subpicture Stream Control");
                Element_Info1(Ztring::ToZtring(Pos));
                int8u Number_43, Number_Wide, Number_Letterbox, Number_PanScan;
                bool  Available;
                BS_Begin();
                Get_SB (   Available,                           "Stream available");
                Get_S1 (7, Number_43,                           "Stream number for 4/3");
                BS_End();
                Get_B1 (Number_Wide,                            "Stream number for Wide");
                Get_B1 (Number_Letterbox,                       "Stream number for Letterbox");
                Get_B1 (Number_PanScan,                         "Stream number for Pan&Scan");
                Element_End0();
                if (Available)
                {
                    Stream_Control_SubPicture_43.push_back(Number_43);
                    Stream_Control_SubPicture_Wide.push_back(Number_Wide);
                    Stream_Control_SubPicture_Letterbox.push_back(Number_Letterbox);
                    Stream_Control_SubPicture_PanScan.push_back(Number_PanScan);
                }

                if (Available && Retrieve(Stream_Text, Pos, Text_ID).empty() && Sectors[(size_t)((File_Offset+Buffer_Offset)/2048)]==Sector_VTS_PGCI)
                {
                    while (Pos>Count_Get(Stream_Text))
                        Stream_Prepare(Stream_Text);

                    Ztring ID_String; ID_String.From_Number(0x20+Number_Wide); ID_String+=__T(" (0x"); ID_String+=Ztring::ToZtring(0x20+Number_Wide, 16); ID_String+=__T(")");
                    Fill(Stream_Text, Pos, Text_ID, ID_String);
                    Fill(Stream_Text, Pos, Text_ID_String, ID_String, true);
                }
            }
            Element_End0();
            Skip_B2(                                            "next PGCN");
            Skip_B2(                                            "previous PGCN");
            Skip_B2(                                            "goup PGCN");
            Skip_B1(                                            "PGC still time - 255=infinite");
            Skip_B1(                                            "PG playback mode");
            Element_Begin1("palette");
            for (int Pos=0; Pos<16; Pos++)
            {
                Skip_B4(                                        "palette (0 - Y - Cr - Cb)");
            }
            Element_End0();
            Get_B2 (commands,                                   "offset within PGC to commands");
            Get_B2 (program_map,                                "offset within PGC to program map");
            Get_B2 (cell_playback,                              "offset within PGC to cell playback information table");
            Get_B2 (cell_position,                              "offset within PGC to cell position information table");
        Element_End0();

        //commands
        if (commands>0)
        {
            if (Element_Offset<Offset+commands)
            {
                if (Offset+commands>Element_Size)
                {
                    Skip_XX(Element_Size-Element_Offset,            "Unknown");
                    return;
                }
                Skip_XX(Offset+commands-Element_Offset,             "Unknown");
            }
            Element_Begin1("commands");
            int16u PreCommands_Count, PostCommands_Count, CellCommands_Count, EndAdress;
            Get_B2 (PreCommands_Count,                          "Number of pre commands");
            Get_B2 (PostCommands_Count,                         "Number of post commands");
            Get_B2 (CellCommands_Count,                         "Number of cell commands");
            Get_B2 (EndAdress,                                  "End address relative to command table");
            if (PreCommands_Count>0)
            {
                Element_Begin1("Pre commands");
                    for (int16u Pos=0; Pos<PreCommands_Count; Pos++)
                    {
                        Element_Begin1("Pre command");
                        Skip_XX(8,                              "Pre command");
                        Element_End0();
                    }
                Element_End0();
            }
            if (PostCommands_Count>0)
            {
                Element_Begin1("Post commands");
                    for (int16u Pos=0; Pos<PostCommands_Count; Pos++)
                    {
                        Element_Begin1("Post command");
                        Skip_XX(8,                              "Post command");
                        Element_End0();
                    }
                Element_End0();
            }
            if (CellCommands_Count>0)
            {
                Element_Begin1("Cell commands");
                    for (int16u Pos=0; Pos<CellCommands_Count; Pos++)
                    {
                        Element_Begin1("Cell command");
                        Skip_XX(8,                              "Cell command");
                        Element_End0();
                    }
                Element_End0();
            }
            Element_End0();
        }

        //program map
        if (program_map>0)
        {
            if (Element_Offset<Offset+program_map)
                Skip_XX(Offset+program_map-Element_Offset,          "Unknown");
            Element_Begin1("program map");
            for (int8u Pos=0; Pos<Program_Count; Pos++)
            {
                Element_Begin1("Entry");
                int8u entry;
                Get_B1( entry,  "Entry cell number");
                ProgramMap.push_back(entry);
                //Skip_B1(                                        "Entry cell number");
                Element_End0();
            }
            Element_End0();
        }

        //cell playback
        if (cell_playback>0)
        {
            if (Element_Offset<Offset+cell_playback)
                Skip_XX(Offset+cell_playback-Element_Offset,        "Unknown");
            Element_Begin1("cell playback");
            for (int8u Pos=0; Pos<Cells; Pos++)
            {
                int64u CellDuration;
                Element_Begin1("cell");
                Skip_XX(4,                                      "ToDo");
                Get_Duration(CellDuration,                      "Time");
                Skip_B4(                                        "first VOBU start sector");
                Skip_B4(                                        "first ILVU end sector");
                Skip_B4(                                        "last VOBU start sector");
                Skip_B4(                                        "last VOBU end sector");
                Element_Info1(Ztring::ToZtring(Pos)); Element_Info1(Ztring::ToZtring(CellDuration));
                Element_End0();

                CellDurations.push_back(CellDuration);
            }
            Element_End0();
        }

        //cell position
        if (cell_position>0)
        {
            if (Element_Offset<Offset+cell_position)
                Skip_XX(Offset+cell_position-Element_Offset,        "Unknown");
            Element_Begin1("cell position");
            for (int8u Pos=0; Pos<Cells; Pos++)
            {
                Element_Begin1("cell");
                Skip_B2(                                        "VOBid");
                Skip_B1(                                        "reserved");
                Skip_B1(                                        "Cell id");
                Element_End0();
            }
            Element_End0();
        }

        Element_End0();

        FILLING_BEGIN();
            if (Title)
            {
                Stream_Prepare(Stream_Menu);

                int64u ProgramTotalDuration=0;
                Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
                for (int8u Pos=0; Pos<ProgramMap.size(); Pos++)
                {
                    Fill(StreamKind_Last, StreamPos_Last, Ztring().Duration_From_Milliseconds(ProgramTotalDuration).To_Local().c_str(), Ztring(__T("Chapter "))+Ztring::ToZtring(Pos+1));

                    int8u End;
                    if (Pos+1>=Program_Count)
                        End=Cells+1;
                    else
                        End=ProgramMap[Pos+1];

                    int64u ProgramDuration=0;
                    if (Pos<ProgramMap.size())
                        for (int8u CellPos=ProgramMap[Pos]; CellPos<End; CellPos++)
                            if (CellPos && CellPos<=CellDurations.size())
                                ProgramDuration+=CellDurations[CellPos-1];
                    ProgramTotalDuration+=ProgramDuration;
                }
                Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
                Fill(Stream_Menu, StreamPos_Last, Menu_Duration, TotalDuration);

                for (size_t Pos=0; Pos<Stream_Control_Audio.size(); Pos++)
                {
                    Fill(StreamKind_Last, StreamPos_Last, "List (Audio)", Stream_Control_Audio[Pos]);
                }
                for (size_t Pos=0; Pos<Stream_Control_SubPicture_43.size(); Pos++)
                {
                    Fill(StreamKind_Last, StreamPos_Last, "List (Subtitles 4/3)", Stream_Control_SubPicture_43[Pos]);
                }
                for (size_t Pos=0; Pos<Stream_Control_SubPicture_Wide.size(); Pos++)
                {
                    Fill(StreamKind_Last, StreamPos_Last, "List (Subtitles Wide)", Stream_Control_SubPicture_Wide[Pos]);
                }
                for (size_t Pos=0; Pos<Stream_Control_SubPicture_Letterbox.size(); Pos++)
                {
                    Fill(StreamKind_Last, StreamPos_Last, "List (Subtitles Letterbox)", Stream_Control_SubPicture_Letterbox[Pos]);
                }
                for (size_t Pos=0; Pos<Stream_Control_SubPicture_PanScan.size(); Pos++)
                {
                    Fill(StreamKind_Last, StreamPos_Last, "List (Subtitles Pan&Scan)", Stream_Control_SubPicture_PanScan[Pos]);
                }
            }
        FILLING_END();
}

//---------------------------------------------------------------------------
void File_Dvdv::TT_SRPT()
{
    Element_Name("table of titles");
}

//---------------------------------------------------------------------------
void File_Dvdv::VMGM_PGCI_UT()
{
    Element_Name("Menu Program Chain table");
}

//---------------------------------------------------------------------------
void File_Dvdv::VMG_PTL_MAIT()
{
    Element_Name("Parental Management masks");
}

//---------------------------------------------------------------------------
void File_Dvdv::VMG_VTS_ATRT()
{
    Element_Name("copies of VTS audio/sub-picture attributes");

    //Parsing
    int32u EndAddress;
    Element_Begin1("Header");
        int32u Offset;
        Skip_B4(                                                "Number of title sets");
        Get_B4 (EndAddress,                                     "End address");
        if (EndAddress>=Element_Size)
            EndAddress=(int32u)Element_Size-1;
        Get_B4 (Offset,                                         "Offset to VTSM_LU relative to VTSM_PGCI_UT");
        if (Offset-12>0)
            Skip_XX(Offset-12,                                  "Unknown");
    Element_End0();

    while (Element_Offset<=EndAddress)
    {
        Element_Begin1("VTS_ATRT");
            Element_Begin1("Header");
                int32u Size;
                Get_B4 (Size,                                   "End address");
                Size++; //Last byte
            Element_End0();
            Element_Begin1("Copy of VTS Category");
                Skip_B4(                                        "VTS Category");
            Element_End0();
            Element_Begin1("Copy of VTS attributes");
                Skip_XX(Size-8,                                 "VTS attributes");
            Element_End0();
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Dvdv::VMG_TXTDT_MG()
{
    Element_Name("text data");
}

//---------------------------------------------------------------------------
void File_Dvdv::VMGM_C_ADT()
{
    Element_Name("menu cell address table");
}

//---------------------------------------------------------------------------
void File_Dvdv::VMGM_VOBU_ADMAP()
{
    Element_Name("menu VOBU address map");
}

} //NameSpace

#endif //MEDIAINFO_DVDV_YES
