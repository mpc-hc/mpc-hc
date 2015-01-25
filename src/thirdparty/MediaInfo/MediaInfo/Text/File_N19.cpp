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
#if defined(MEDIAINFO_N19_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Text/File_N19.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
const char* N19_CodePageNumber(int32u CPN)
{
    switch (CPN)
    {
        case 0x343337 : return "United States";
        case 0x383530 : return "Multilingual";
        case 0x383630 : return "Portugal";
        case 0x383633 : return "Canada-French";
        case 0x383635 : return "Nordic";
        default       : return "";
    }
}

//---------------------------------------------------------------------------
const char* N19_CharacterCodeTable(int16u CCT)
{
    switch (CCT)
    {
        case 0x3030 : return "Latin, ISO 6937-2";
        case 0x3031 : return "Latin/Cyrillic, ISO 8859-5";
        case 0x3032 : return "Latin/Arabic, ISO 8859-6";
        case 0x3033 : return "Latin/Greek, ISO 8859-7";
        case 0x3034 : return "Latin/Hebrew, ISO 8859-8";
        default       : return "";
    }
}

//---------------------------------------------------------------------------
float32 N19_DiskFormatCode_FrameRate(int64u DFC)
{
    switch (DFC)
    {
        case 0x53544C32352E3031LL : return 25.000;
        case 0x53544C33302E3031LL : return 30.000;
        default                   : return  0.000;
    }
}

//---------------------------------------------------------------------------
const char* N19_DisplayStandardCode(int8u DSC)
{
    switch (DSC)
    {
        case 0x30 : return "Open subtitling";
        case 0x31 : return "Level-1 teletext";
        case 0x32 : return "Level-2 teletext";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* N19_LanguageCode(int16u LC)
{
    switch (LC)
    {
        case 0x3030 : return "";
        case 0x3031 : return "sq";
        case 0x3032 : return "br";
        case 0x3033 : return "ca";
        case 0x3034 : return "hr";
        case 0x3035 : return "cy";
        case 0x3036 : return "cs";
        case 0x3037 : return "da";
        case 0x3038 : return "de";
        case 0x3039 : return "en";
        case 0x3041 : return "es";
        case 0x3042 : return "eo";
        case 0x3043 : return "et";
        case 0x3044 : return "eu";
        case 0x3045 : return "fo";
        case 0x3046 : return "fr";
        case 0x3130 : return "fy";
        case 0x3131 : return "ga";
        case 0x3132 : return "gd";
        case 0x3133 : return "gl";
        case 0x3134 : return "is";
        case 0x3135 : return "it";
        case 0x3136 : return "Lappish";
        case 0x3137 : return "la";
        case 0x3138 : return "lv";
        case 0x3139 : return "lb";
        case 0x3141 : return "lt";
        case 0x3142 : return "hu";
        case 0x3143 : return "mt";
        case 0x3144 : return "nl";
        case 0x3145 : return "no";
        case 0x3146 : return "oc";
        case 0x3230 : return "pl";
        case 0x3231 : return "pt";
        case 0x3232 : return "ro";
        case 0x3233 : return "Romansh";
        case 0x3234 : return "sr";
        case 0x3235 : return "sk";
        case 0x3236 : return "sl";
        case 0x3237 : return "fi";
        case 0x3238 : return "sv";
        case 0x3239 : return "tr";
        case 0x3241 : return "Flemish";
        case 0x3242 : return "wa";
        case 0x3435 : return "zu";
        case 0x3436 : return "vi";
        case 0x3437 : return "uz";
        case 0x3438 : return "ur";
        case 0x3439 : return "uk";
        case 0x3441 : return "th";
        case 0x3442 : return "te";
        case 0x3443 : return "tt";
        case 0x3444 : return "ta";
        case 0x3445 : return "Tadzhik";
        case 0x3446 : return "sw";
        case 0x3530 : return "Sranan Tongo";
        case 0x3531 : return "so";
        case 0x3532 : return "si";
        case 0x3533 : return "sn";
        case 0x3534 : return "sr";
        case 0x3535 : return "Ruthenian";
        case 0x3536 : return "ru";
        case 0x3537 : return "qu";
        case 0x3538 : return "ps";
        case 0x3539 : return "Punjabi";
        case 0x3541 : return "fa";
        case 0x3542 : return "Papamiento";
        case 0x3543 : return "or";
        case 0x3544 : return "ne";
        case 0x3545 : return "nr";
        case 0x3546 : return "mr";
        case 0x3630 : return "mo";
        case 0x3631 : return "ms";
        case 0x3632 : return "mg";
        case 0x3633 : return "mk";
        case 0x3634 : return "Laotian";
        case 0x3635 : return "kr";
        case 0x3636 : return "km";
        case 0x3637 : return "kk";
        case 0x3638 : return "kn";
        case 0x3639 : return "jp";
        case 0x3641 : return "id";
        case 0x3642 : return "hi";
        case 0x3643 : return "he";
        case 0x3644 : return "ha";
        case 0x3645 : return "Gurani";
        case 0x3646 : return "Gujurati";
        case 0x3730 : return "hr";
        case 0x3731 : return "ka";
        case 0x3732 : return "ff";
        case 0x3733 : return "Dari";
        case 0x3734 : return "Churash";
        case 0x3735 : return "zh";
        case 0x3736 : return "my";
        case 0x3737 : return "bg";
        case 0x3738 : return "bn";
        case 0x3739 : return "be";
        case 0x3741 : return "bm";
        case 0x3742 : return "az";
        case 0x3743 : return "as";
        case 0x3744 : return "hy";
        case 0x3745 : return "ar";
        case 0x3746 : return "am";
        default     : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_N19::File_N19()
:File__Analyze()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_N19;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_N19::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<11)
        return false; //Must wait for more data

    int64u DiskFormatCode=CC8(Buffer+3);
    if (DiskFormatCode!=0x53544C32352E3031LL
     && DiskFormatCode!=0x53544C33302E3031LL)
    {
        Reject("N19");
        return false;
    }

    //Element_Size
    if (Buffer_Size<1024)
        return false; //Must wait for more data about GSI

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_N19::FileHeader_Parse()
{
    Element_Name("General Subtitle Information");

    //Parsing
    Ztring OPT, RD, TNS, MNC, MNR, CO, EN;
    string TCP;
    int16u LC;
    int8u TCS;
    Info_C3   (    CPN,                                         "CPN - Code Page Number"); Param_Info1(N19_CodePageNumber(CPN));
    Get_C8    (    DFC,                                         "DFC - Disk Format Code"); Param_Info1(N19_DiskFormatCode_FrameRate(DFC));
    Info_C1   (    DSC,                                         "DSC - Display Standard Code"); Param_Info1(N19_DisplayStandardCode(DSC));
    Get_C2    (    CCT,                                         "CCT - Character Code Table number"); Param_Info1(N19_CharacterCodeTable(CCT));
    Get_C2    (    LC,                                          "LC - Language Code"); Param_Info1(N19_LanguageCode(LC));
    Get_Local (32, OPT,                                         "OPT - Original Programme Title");
    Skip_Local(32,                                              "OET - Original Episode Title");
    Skip_Local(32,                                              "TPT - Translated Programme");
    Skip_Local(32,                                              "TET - Translated Episode");
    Skip_Local(32,                                              "TN - Translator's Name");
    Skip_Local(32,                                              "TCD - Translator's Contact Details");
    Skip_Local(16,                                              "SLR - Subtitle List Reference Code");
    Skip_Local( 6,                                              "CD - Creation Date");
    Get_Local ( 6, RD,                                          "RD - Revision Date");
    Skip_C2   (                                                 "RN - Revision number");
    Skip_C5   (                                                 "TNB - Total Number of Text and Timing Information (TTI) blocks");
    Get_Local ( 5, TNS,                                         "TNS - Total Number of Subtitles");
    Skip_C3   (                                                 "TNG - Total Number of Subtitle Groups");
    Get_Local ( 2, MNC,                                         "MNC - Maximum Number of Displayable Characters in any text row");
    Get_Local ( 2, MNR,                                         "MNR - Maximum Number of Displayable Rows");
    Get_C1    (    TCS,                                         "TCS - Time Code: Status");
    Get_String( 8, TCP,                                         "TCP - Time Code: Start-of-Programme");
    Skip_Local( 8,                                              "TCF - Time Code: First In-Cue");
    Skip_C1   (                                                 "TND - Total Number of Disks");
    Skip_C1   (                                                 "DSN - Disk Sequence Number");
    Get_Local ( 3, CO,                                          "CO - Country of Origin");
    Skip_Local(32,                                              "PUB - Publisher");
    Get_Local (32, EN,                                          "EN - Editor's Name");
    Skip_Local(32,                                              "ECD - Editor's Contact Details");
    Skip_XX(75,                                                 "Spare Bytes");
    Skip_XX(576,                                                "UDA - User-Defined Area");

    FILLING_BEGIN();
        Accept("N19");

        Fill(Stream_General, 0, General_Format, "N19");
        Fill(Stream_General, 0, General_Title, OPT);
        RD.insert(0, __T("20"));
        RD.insert(4, __T("-"));
        RD.insert(7, __T("-"));
        Fill(Stream_General, 0, General_Recorded_Date, RD);
        Fill(Stream_General, 0, General_Country, Ztring(CO).MakeLowerCase());
        Fill(Stream_General, 0, General_DistributedBy, EN);

        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Format, "N19");
        if (N19_DiskFormatCode_FrameRate(DFC))
        {
            Fill(Stream_Text, 0, "FrameRate", N19_DiskFormatCode_FrameRate(DFC));
            if (TCS==0x31 && TCP.size()==8
             && TCP[0]>='0' && TCP[0]<='9'
             && TCP[1]>='0' && TCP[1]<='9'
             && TCP[2]>='0' && TCP[2]<='6'
             && TCP[3]>='0' && TCP[3]<='9'
             && TCP[4]>='0' && TCP[4]<='6'
             && TCP[5]>='0' && TCP[5]<='9'
             && TCP[6]>='0' && TCP[6]<='2'
             && TCP[7]>='0' && TCP[7]<='9')
            {
                int32u Delay=0;
                Delay+=(((int32u)TCP[0])-'0')*10*60*60*1000;
                Delay+=(((int32u)TCP[1])-'0')*   60*60*1000;
                Delay+=(((int32u)TCP[2])-'0')*   10*60*1000;
                Delay+=(((int32u)TCP[3])-'0')*      60*1000;
                Delay+=(((int32u)TCP[4])-'0')*      10*1000;
                Delay+=(((int32u)TCP[5])-'0')*         1000;
                int8u Frames=0;
                Frames+=(((int8u)TCP[6])-'0')*10;
                Frames+=(((int8u)TCP[7])-'0');
                Delay+=float32_int32s(Frames*1000/N19_DiskFormatCode_FrameRate(DFC));
                //Fill(Stream_Text, 0, Text_Delay, Delay); //TODO is 0???
                /*TCP.insert(':', 2);
                TCP.insert(':', 5);
                TCP.insert(':', 8);
                Fill(Stream_Text, 0, "Delay/String4", TCP);*/
            }
        }
        Fill(Stream_Text, 0, Text_Width, MNC.To_int32u());
        Fill(Stream_Text, 0, Text_Height, MNR.To_int32u());
        Fill(Stream_Text, 0, Text_Language, N19_LanguageCode(LC));

        //Init
        FirstFrame_TCI=(int64u)-1;
        #if MEDIAINFO_DEMUX
            Frame_Count=0;
            TCO_Latest=(int64u)-1;
        #endif //MEDIAINFO_DEMUX
    FILLING_END();
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_N19::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    #if MEDIAINFO_DEMUX
        TCO_Latest=(int64u)-1;
    #endif //MEDIAINFO_DEMUX

    GoTo(0x400);
    Open_Buffer_Unsynch();
    return 1;
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_N19::Header_Parse()
{
    //Filling
    Header_Fill_Size(128);
    Header_Fill_Code(0, __T("TTI"));
}

//---------------------------------------------------------------------------
void File_N19::Data_Parse()
{
    //Parsing
    Ztring TF;
    int32u TCI, TCO;
    Skip_B1   (                                                 "SGN - Subtitle Group Number");
    Skip_B2   (                                                 "SN - Subtitle Number");
    Skip_B1   (                                                 "EBN - Extension Block Number");
    Skip_B1   (                                                 "CS - Cumulative Status");
    Get_B4    (TCI,                                             "TCI - Time Code In");
    TCI=((TCI>>24)&0xFF)*60*60*1000
      + ((TCI>>16)&0xFF)   *60*1000
      + ((TCI>>8 )&0xFF)      *1000
      +  float32_int32s((TCI     &0xFF)      *1000/N19_DiskFormatCode_FrameRate(DFC));
    Param_Info1(Ztring().Duration_From_Milliseconds((int64u)TCI));
    Get_B4    (TCO,                                             "TCO - Time Code Out");
    TCO=((TCO>>24)&0xFF)*60*60*1000
      + ((TCO>>16)&0xFF)   *60*1000
      + ((TCO>>8 )&0xFF)      *1000
      +  float32_int32s((TCO     &0xFF)      *1000/N19_DiskFormatCode_FrameRate(DFC));
    Param_Info1(Ztring().Duration_From_Milliseconds((int64u)TCO));
    Skip_B1   (                                                 "VP - Vertical Position");
    Skip_B1   (                                                 "JC - Justification Code");
    Skip_B1   (                                                 "CF - Comment Flag");
    switch (CCT)
    {
        case 0x3030 :   //Latin ISO 6937-2
                        Get_ISO_6937_2(112, TF,                 "TF - Text Field");
                        break;
        case 0x3031 :   //Latin ISO 8859-5
                        Get_ISO_8859_5(112, TF,                 "TF - Text Field");
                        break;
        default:
                        //Not yet supported, basic default
                        Get_ISO_8859_1(112, TF,                 "TF - Text Field");
    }
    #if MEDIAINFO_TRACE
        TF.FindAndReplace(__T("\x8A"), EOL, 0, Ztring_Recursive);
        TF.FindAndReplace(__T("\x8F"), Ztring(), 0, Ztring_Recursive);
        Param_Info1(TF);
    #endif //MEDIAINFO_TRACE

    FILLING_BEGIN();
        if (FirstFrame_TCI==(int64u)-1)
        {
            FirstFrame_TCI=TCI;
            Fill(Stream_Text, 0, Text_Delay, TCI);
            Fill(Stream_Text, 0, Text_Delay_Source, "Container");
        }
        if (File_Offset+Buffer_Offset+Element_Size+128>File_Size)
        {
            Fill(Stream_Text, 0, Text_Duration, TCO-FirstFrame_TCI);
        }
        else if (Config->ParseSpeed<1.0)
            //Jumping
            GoToFromEnd(128, "N19");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_N19_YES
