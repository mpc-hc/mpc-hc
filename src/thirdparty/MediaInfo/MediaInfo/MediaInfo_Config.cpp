/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Global configuration of MediaInfo
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
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/ZtringListListF.h"
#include "ZenLib/File.h"
#include <algorithm>
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const Char*  MediaInfo_Version=__T("MediaInfoLib - v0.7.66");
const Char*  MediaInfo_Url=__T("http://MediaArea.net/MediaInfo");
      Ztring EmptyZtring;       //Use it when we can't return a reference to a true Ztring
const Ztring EmptyZtring_Const; //Use it when we can't return a reference to a true Ztring, const version
const ZtringListList EmptyZtringListList_Const; //Use it when we can't return a reference to a true ZtringListList, const version
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void MediaInfo_Config_CodecID_General_Mpeg4   (InfoMap &Info);
void MediaInfo_Config_CodecID_Video_Matroska  (InfoMap &Info);
void MediaInfo_Config_CodecID_Video_Mpeg4     (InfoMap &Info);
void MediaInfo_Config_CodecID_Video_Ogg       (InfoMap &Info);
void MediaInfo_Config_CodecID_Video_Real      (InfoMap &Info);
void MediaInfo_Config_CodecID_Video_Riff      (InfoMap &Info);
void MediaInfo_Config_CodecID_Audio_Matroska  (InfoMap &Info);
void MediaInfo_Config_CodecID_Audio_Mpeg4     (InfoMap &Info);
void MediaInfo_Config_CodecID_Audio_Ogg       (InfoMap &Info);
void MediaInfo_Config_CodecID_Audio_Real      (InfoMap &Info);
void MediaInfo_Config_CodecID_Audio_Riff      (InfoMap &Info);
void MediaInfo_Config_CodecID_Text_Matroska   (InfoMap &Info);
void MediaInfo_Config_CodecID_Text_Mpeg4      (InfoMap &Info);
void MediaInfo_Config_CodecID_Text_Riff       (InfoMap &Info);
void MediaInfo_Config_Codec                   (InfoMap &Info);
void MediaInfo_Config_DefaultLanguage         (Translation &Info);
void MediaInfo_Config_Iso639_1                (InfoMap &Info);
void MediaInfo_Config_Iso639_2                (InfoMap &Info);
void MediaInfo_Config_General                 (ZtringListList &Info);
void MediaInfo_Config_Video                   (ZtringListList &Info);
void MediaInfo_Config_Audio                   (ZtringListList &Info);
void MediaInfo_Config_Text                    (ZtringListList &Info);
void MediaInfo_Config_Other                   (ZtringListList &Info);
void MediaInfo_Config_Image                   (ZtringListList &Info);
void MediaInfo_Config_Menu                    (ZtringListList &Info);
void MediaInfo_Config_Summary                 (ZtringListList &Info);
void MediaInfo_Config_Format                  (InfoMap &Info);
void MediaInfo_Config_Library_DivX            (InfoMap &Info);
void MediaInfo_Config_Library_XviD            (InfoMap &Info);
void MediaInfo_Config_Library_MainConcept_Avc (InfoMap &Info);
void MediaInfo_Config_Library_VorbisCom       (InfoMap &Info);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

void MediaInfo_Config::Init()
{
    CS.Enter();
    //We use Init() instead of COnstructor because for some backends (like WxWidgets...) does NOT like constructor of static object with Unicode conversion

    //Test
    if (!LineSeparator.empty())
    {
        CS.Leave();
        return; //Already done
    }

    //Filling
    FormatDetection_MaximumOffset=0;
    #if MEDIAINFO_ADVANCED
        VariableGopDetection_Occurences=4;
        VariableGopDetection_GiveUp=false;
        InitDataNotRepeated_Occurences=(int64u)-1; //Disabled by default
        InitDataNotRepeated_GiveUp=false;
    #endif //MEDIAINFO_ADVANCED
    MpegTs_MaximumOffset=64*1024*1024;
    MpegTs_MaximumScanDuration=30000000000LL;
    MpegTs_ForceStreamDisplay=false;
    #if MEDIAINFO_ADVANCED
        MpegTs_VbrDetection_Delta=0;
        MpegTs_VbrDetection_Occurences=4;
        MpegTs_VbrDetection_GiveUp=false;
    #endif //MEDIAINFO_ADVANCED
    Complete=0;
    BlockMethod=0;
    Internet=0;
    MultipleValues=0;
    ParseUnknownExtensions=1;
    ShowFiles_Nothing=1;
    ShowFiles_VideoAudio=1;
    ShowFiles_VideoOnly=1;
    ShowFiles_AudioOnly=1;
    ShowFiles_TextOnly=1;
    ParseSpeed=(float32)0.5;
    Verbosity=(float32)0.5;
    Trace_Level=(float32)0.0;
    Trace_TimeSection_OnlyFirstOccurrence=false;
    Trace_Format=Trace_Format_Tree;
    Language_Raw=false;
    ReadByHuman=true;
    LegacyStreamDisplay=true;
    SkipBinaryData=false;
    Demux=0;
    LineSeparator=EOL;
    ColumnSeparator=__T(";");
    TagSeparator=__T(" / ");
    Quote=__T("\"");
    DecimalPoint=__T(".");
    ThousandsPoint=Ztring();
    #if MEDIAINFO_EVENTS
        Event_CallBackFunction=NULL;
        Event_UserHandler=NULL;
    #endif //MEDIAINFO_EVENTS
    #if defined(MEDIAINFO_LIBCURL_YES)
        Ssh_IgnoreSecurity=false;
        Ssl_IgnoreSecurity=false;
    #endif //defined(MEDIAINFO_LIBCURL_YES)

    CS.Leave();

    ZtringListList ZLL1; Language_Set(ZLL1);
}

//***************************************************************************
// Info
//***************************************************************************

Ztring MediaInfo_Config::Option (const String &Option, const String &Value_Raw)
{
    SubFile_Config(Option)=Value_Raw;

    String Option_Lower(Option);
    size_t Egal_Pos=Option_Lower.find(__T('='));
    if (Egal_Pos==string::npos)
        Egal_Pos=Option_Lower.size();
    transform(Option_Lower.begin(), Option_Lower.begin()+Egal_Pos, Option_Lower.begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix

    //Parsing pointer to a file
    Ztring Value;
    if (Value_Raw.find(__T("file://"))==0)
    {
        //Open
        Ztring FileName(Value_Raw, 7, Ztring::npos);
        File F(FileName.c_str());

        //Read
        int64u Size=F.Size_Get();
        if (Size>=0xFFFFFFFF)
            Size=1024*1024;
        int8u* Buffer=new int8u[(size_t)Size+1];
        size_t Pos=F.Read(Buffer, (size_t)Size);
        F.Close();
        Buffer[Pos]='\0';
        Ztring FromFile; FromFile.From_UTF8((char*)Buffer);
        if (FromFile.empty())
             FromFile.From_Local((char*)Buffer);
        delete[] Buffer; //Buffer=NULL;

        //Merge
        Value=FromFile;
    }
    else
        Value=Value_Raw;

         if (Option_Lower.empty())
    {
        return Ztring();
    }
    else if (Option_Lower==__T("charset_config"))
    {
        return Ztring(); //Only used in DLL, no Library action
    }
    else if (Option_Lower==__T("charset_output"))
    {
        return Ztring(); //Only used in DLL, no Library action
    }
    else if (Option_Lower==__T("complete"))
    {
        Complete_Set(Value.To_int8u()?true:false);
        return Ztring();
    }
    else if (Option_Lower==__T("complete_get"))
    {
        if (Complete_Get())
            return __T("1");
        else
            return Ztring();
    }
    else if (Option_Lower==__T("blockmethod"))
    {
        if (Value.empty())
            BlockMethod_Set(0);
        else
            BlockMethod_Set(1);
        return Ztring();
    }
    else if (Option_Lower==__T("blockmethod_get"))
    {
        if (BlockMethod_Get())
            return __T("1");
        else
            return Ztring();
    }
    else if (Option_Lower==__T("internet"))
    {
        if (Value.empty())
            Internet_Set(0);
        else
            Internet_Set(1);
        return Ztring();
    }
    else if (Option_Lower==__T("internet_get"))
    {
        if (Internet_Get())
            return __T("1");
        else
            return Ztring();
    }
    else if (Option_Lower==__T("demux"))
    {
        String Value_Lower(Value);
        transform(Value_Lower.begin(), Value_Lower.end(), Value_Lower.begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix

             if (Value_Lower==__T("all"))
            Demux_Set(7);
        else if (Value_Lower==__T("frame"))
            Demux_Set(1);
        else if (Value_Lower==__T("container"))
            Demux_Set(2);
        else if (Value_Lower==__T("elementary"))
            Demux_Set(4);
        else
            Demux_Set(0);
        return Ztring();
    }
    else if (Option_Lower==__T("demux_get"))
    {
        switch (Demux_Get())
        {
            case 7 : return __T("All");
            case 1 : return __T("Frame");
            case 2 : return __T("Container");
            case 4 : return __T("Elementary");
            default: return Ztring();
        }
    }
    else if (Option_Lower==__T("multiplevalues"))
    {
        if (Value.empty())
            MultipleValues_Set(0);
        else
            MultipleValues_Set(1);
        return Ztring();
    }
    else if (Option_Lower==__T("multiplevalues_get"))
    {
        if (MultipleValues_Get())
            return __T("1");
        else
            return Ztring();
    }
    else if (Option_Lower==__T("parseunknownextensions"))
    {
        if (Value.empty())
            ParseUnknownExtensions_Set(0);
        else
            ParseUnknownExtensions_Set(1);
        return Ztring();
    }
    else if (Option_Lower==__T("parseunknownextensions_get"))
    {
        if (ParseUnknownExtensions_Get())
            return __T("1");
        else
            return Ztring();
    }
    else if (Option_Lower==__T("showfiles_set"))
    {
        ShowFiles_Set(Value.c_str());
        return Ztring();
    }
    else if (Option_Lower==__T("readbyhuman"))
    {
        ReadByHuman_Set(Value.To_int8u()?true:false);
        return Ztring();
    }
    else if (Option_Lower==__T("readbyhuman_get"))
    {
        return ReadByHuman_Get()?__T("1"):__T("0");
    }
    else if (Option_Lower==__T("legacystreamdisplay"))
    {
        LegacyStreamDisplay_Set(Value.To_int8u()?true:false);
        return Ztring();
    }
    else if (Option_Lower==__T("legacystreamdisplay_get"))
    {
        return LegacyStreamDisplay_Get()?__T("1"):__T("0");
    }
    else if (Option_Lower==__T("skipbinarydata"))
    {
        SkipBinaryData_Set(Value.To_int8u()?true:false);
        return Ztring();
    }
    else if (Option_Lower==__T("skipbinarydata_get"))
    {
        return SkipBinaryData_Get()?__T("1"):__T("0");
    }
    else if (Option_Lower==__T("parsespeed"))
    {
        ParseSpeed_Set(Value.To_float32());
        return Ztring();
    }
    else if (Option_Lower==__T("parsespeed_get"))
    {
        return Ztring::ToZtring(ParseSpeed_Get(), 3);
    }
    else if (Option_Lower==__T("verbosity"))
    {
        Verbosity_Set(Value.To_float32());
        return Ztring();
    }
    else if (Option_Lower==__T("verbosity_get"))
    {
        return Ztring::ToZtring(Verbosity_Get(), 3);
    }
    else if (Option_Lower==__T("lineseparator"))
    {
        LineSeparator_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("lineseparator_get"))
    {
        return LineSeparator_Get();
    }
    else if (Option_Lower==__T("version"))
    {
        Version_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("version_get"))
    {
        return Version_Get();
    }
    else if (Option_Lower==__T("columnseparator"))
    {
        ColumnSeparator_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("columnseparator_get"))
    {
        return ColumnSeparator_Get();
    }
    else if (Option_Lower==__T("tagseparator"))
    {
        TagSeparator_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("tagseparator_get"))
    {
        return TagSeparator_Get();
    }
    else if (Option_Lower==__T("quote"))
    {
        Quote_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("quote_get"))
    {
        return Quote_Get();
    }
    else if (Option_Lower==__T("decimalpoint"))
    {
        DecimalPoint_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("decimalpoint_get"))
    {
        return DecimalPoint_Get();
    }
    else if (Option_Lower==__T("thousandspoint"))
    {
        ThousandsPoint_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("thousandspoint_get"))
    {
        return ThousandsPoint_Get();
    }
    else if (Option_Lower==__T("streammax"))
    {
        ZtringListList StreamMax=Value.c_str();
        StreamMax_Set(StreamMax);
        return Ztring();
    }
    else if (Option_Lower==__T("streammax_get"))
    {
        return StreamMax_Get();
    }
    else if (Option_Lower==__T("language"))
    {
        ZtringListList Language=Value.c_str();
        Language_Set(Language);
        return Ztring();
    }
    else if (Option_Lower==__T("language_get"))
    {
        return Language_Get();
    }
    else if (Option_Lower==__T("inform"))
    {
        Inform_Set(Value.c_str());
        return Ztring();
    }
    else if (Option_Lower==__T("output"))
    {
        Inform_Set(Value.c_str());
        return Ztring();
    }
    else if (Option_Lower==__T("inform_get"))
    {
        return Inform_Get();
    }
    else if (Option_Lower==__T("output_get"))
    {
        return Inform_Get();
    }
    else if (Option_Lower==__T("inform_replace"))
    {
        Inform_Replace_Set(Value.c_str());
        return Ztring();
    }
    else if (Option_Lower==__T("inform_replace_get"))
    {
        return Inform_Get();
    }
    else if (Option_Lower==__T("details")) //Legacy for trace_level
    {
        return MediaInfo_Config::Option(__T("Trace_Level"), Value);
    }
    else if (Option_Lower==__T("details_get")) //Legacy for trace_level
    {
        return MediaInfo_Config::Option(__T("Trace_Level_Get"), Value);
    }
    else if (Option_Lower==__T("detailslevel")) //Legacy for trace_level
    {
        return MediaInfo_Config::Option(__T("Trace_Level"), Value);
    }
    else if (Option_Lower==__T("detailslevel_get")) //Legacy for trace_level
    {
        return MediaInfo_Config::Option(__T("Trace_Level_Get"), Value);
    }
    else if (Option_Lower==__T("trace_level"))
    {
        Trace_Level_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("trace_level_get"))
    {
        return Ztring::ToZtring(Trace_Level_Get());
    }
    else if (Option_Lower==__T("trace_timesection_onlyfirstoccurrence"))
    {
        Trace_TimeSection_OnlyFirstOccurrence_Set(Value.To_int64u()?true:false);
        return Ztring();
    }
    else if (Option_Lower==__T("trace_timesection_onlyfirstoccurrence_get"))
    {
        return Trace_TimeSection_OnlyFirstOccurrence_Get()?__T("1"):__T("0");
    }
    else if (Option_Lower==__T("detailsformat")) //Legacy for trace_format
    {
        return MediaInfo_Config::Option(__T("Trace_Format"), Value);
    }
    else if (Option_Lower==__T("detailsformat_get")) //Legacy for trace_format
    {
        return MediaInfo_Config::Option(__T("Trace_Format_Get"), Value);
    }
    else if (Option_Lower==__T("trace_format"))
    {
        String NewValue_Lower(Value);
        transform(NewValue_Lower.begin(), NewValue_Lower.end(), NewValue_Lower.begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix

        CriticalSectionLocker CSL(CS);
        if (NewValue_Lower==__T("csv"))
            Trace_Format_Set(Trace_Format_CSV);
        else
            Trace_Format_Set(Trace_Format_Tree);
        return Ztring();
    }
    else if (Option_Lower==__T("trace_format_get"))
    {
        switch (Trace_Format_Get())
        {
            case Trace_Format_CSV : return __T("CSV");
            default : return __T("Tree");
        }
    }
    else if (Option_Lower==__T("detailsmodificator"))
    {
        Trace_Modificator_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("detailsmodificator_get"))
    {
        return Trace_Modificator_Get(Value);
    }
    else if (Option_Lower==__T("info_parameters"))
    {
        ZtringListList ToReturn=Info_Parameters_Get();

        //Adapt first column
        for (size_t Pos=0; Pos<ToReturn.size(); Pos++)
        {
            Ztring &C1=ToReturn(Pos, 0);
            if (!ToReturn(Pos, 1).empty())
            {
                C1.resize(25, ' ');
                ToReturn(Pos, 0)=C1 + __T(" :");
            }
        }

        ToReturn.Separator_Set(0, LineSeparator_Get());
        ToReturn.Separator_Set(1, __T(" "));
        ToReturn.Quote_Set(Ztring());
        return ToReturn.Read();
    }
    else if (Option_Lower==__T("info_parameters_csv"))
    {
        return Info_Parameters_Get(Value==__T("Complete"));
    }
    else if (Option_Lower==__T("info_codecs"))
    {
        return Info_Codecs_Get();
    }
    else if (Option_Lower==__T("info_version"))
    {
        return Info_Version_Get();
    }
    else if (Option_Lower==__T("info_url"))
    {
        return Info_Url_Get();
    }
    else if (Option_Lower==__T("formatdetection_maximumoffset"))
    {
        FormatDetection_MaximumOffset_Set(Value==__T("-1")?(int64u)-1:((Ztring*)&Value)->To_int64u());
        return Ztring();
    }
    else if (Option_Lower==__T("formatdetection_maximumoffset_get"))
    {
        return FormatDetection_MaximumOffset_Get()==(int64u)-1?Ztring(__T("-1")):Ztring::ToZtring(FormatDetection_MaximumOffset_Get());
    }
    else if (Option_Lower==__T("variablegopdetection_occurences"))
    {
        #if MEDIAINFO_ADVANCED
            VariableGopDetection_Occurences_Set(Value.To_int64u());
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("variablegopdetection_occurences_get"))
    {
        #if MEDIAINFO_ADVANCED
            return Ztring::ToZtring(VariableGopDetection_Occurences_Get());
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("variablegopdetection_giveup"))
    {
        #if MEDIAINFO_ADVANCED
            VariableGopDetection_GiveUp_Set(Value.To_int8u()?true:false);
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("variablegopdetection_giveup_get"))
    {
        #if MEDIAINFO_ADVANCED
            return VariableGopDetection_GiveUp_Get()?__T("1"):__T("0");
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("initdatanotrepeated_occurences"))
    {
        #if MEDIAINFO_ADVANCED
            InitDataNotRepeated_Occurences_Set(Value.To_int64u());
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("initdatanotrepeated_occurences_get"))
    {
        #if MEDIAINFO_ADVANCED
            return Ztring::ToZtring(InitDataNotRepeated_Occurences_Get());
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("initdatanotrepeated_giveup"))
    {
        #if MEDIAINFO_ADVANCED
            InitDataNotRepeated_GiveUp_Set(Value.To_int8u()?true:false);
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("initdatanotrepeated_giveup_get"))
    {
        #if MEDIAINFO_ADVANCED
            return InitDataNotRepeated_GiveUp_Get()?__T("1"):__T("0");
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_maximumoffset"))
    {
        MpegTs_MaximumOffset_Set(Value==__T("-1")?(int64u)-1:((Ztring*)&Value)->To_int64u());
        return Ztring();
    }
    else if (Option_Lower==__T("mpegts_maximumoffset_get"))
    {
        return MpegTs_MaximumOffset_Get()==(int64u)-1?Ztring(__T("-1")):Ztring::ToZtring(MpegTs_MaximumOffset_Get());
    }
    else if (Option_Lower==__T("mpegts_vbrdetection_delta"))
    {
        #if MEDIAINFO_ADVANCED
            MpegTs_VbrDetection_Delta_Set(Value.To_float64());
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_vbrdetection_delta_get"))
    {
        #if MEDIAINFO_ADVANCED
            return Ztring::ToZtring(MpegTs_VbrDetection_Delta_Get(), 9);
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_vbrdetection_occurences"))
    {
        #if MEDIAINFO_ADVANCED
            MpegTs_VbrDetection_Occurences_Set(Value.To_int64u());
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_vbrdetection_occurences_get"))
    {
        #if MEDIAINFO_ADVANCED
            return Ztring::ToZtring(MpegTs_VbrDetection_Occurences_Get());
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_vbrdetection_giveup"))
    {
        #if MEDIAINFO_ADVANCED
            MpegTs_VbrDetection_GiveUp_Set(Value.To_int8u()?true:false);
            return Ztring();
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_vbrdetection_giveup_get"))
    {
        #if MEDIAINFO_ADVANCED
            return MpegTs_VbrDetection_GiveUp_Get()?__T("1"):__T("0");
        #else // MEDIAINFO_ADVANCED
            return __T("advanced features are disabled due to compilation options");
        #endif // MEDIAINFO_ADVANCED
    }
    else if (Option_Lower==__T("mpegts_maximumscanduration"))
    {
        MpegTs_MaximumScanDuration_Set(float64_int64s((((Ztring*)&Value)->To_float64())*1000000000));
        return Ztring();
    }
    else if (Option_Lower==__T("mpegts_maximumscanduration_get"))
    {
        return MpegTs_MaximumScanDuration_Get()==(int64u)-1?Ztring(__T("-1")):Ztring::ToZtring(MpegTs_MaximumOffset_Get());
    }
    else if (Option_Lower==__T("mpegts_forcestreamdisplay"))
    {
        MpegTs_ForceStreamDisplay_Set(Value.To_int8u()?true:false);
        return Ztring();
    }
    else if (Option_Lower==__T("mpegts_forcestreamdisplay_get"))
    {
        return MpegTs_ForceStreamDisplay_Get()?__T("1"):__T("0");
    }
    else if (Option_Lower==__T("custommapping"))
    {
        CustomMapping_Set(Value);
        return Ztring();
    }
    else if (Option_Lower==__T("event_callbackfunction"))
    {
        #if MEDIAINFO_EVENTS
            return Event_CallBackFunction_Set(Value);
        #else //MEDIAINFO_EVENTS
            return __T("Event manager is disabled due to compilation options");
        #endif //MEDIAINFO_EVENTS
    }
    else if (Option_Lower==__T("ssh_knownhostsfilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssh_KnownHostsFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssh_publickeyfilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssh_PublicKeyFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssh_privatekeyfilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssh_PrivateKeyFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssh_ignoresecurity"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssh_IgnoreSecurity_Set(Value.empty() || Value.To_float32());
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_certificatefilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_CertificateFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_certificateFormat"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_CertificateFormat_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_privatekeyfilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_PrivateKeyFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_privatekeyformat"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_PrivateKeyFormat_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_certificateauthorityfilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_CertificateAuthorityFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_certificateauthoritypath"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_CertificateAuthorityPath_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_certificaterevocationlistfilename"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_CertificateRevocationListFileName_Set(Value);
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else if (Option_Lower==__T("ssl_ignoresecurity"))
    {
        #if defined(MEDIAINFO_LIBCURL_YES)
            Ssl_IgnoreSecurity_Set(Value.empty() || Value.To_float32());
            return Ztring();
        #else // defined(MEDIAINFO_LIBCURL_YES)
            return __T("Libcurl support is disabled due to compilation options");
        #endif // defined(MEDIAINFO_LIBCURL_YES)
    }
    else
        return __T("Option not known");
}

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
void MediaInfo_Config::Complete_Set (size_t NewValue)
{
    CriticalSectionLocker CSL(CS);
    Complete=NewValue;
}

size_t MediaInfo_Config::Complete_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Complete;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::BlockMethod_Set (size_t NewValue)
{
    CriticalSectionLocker CSL(CS);
    BlockMethod=NewValue;
}

size_t MediaInfo_Config::BlockMethod_Get ()
{
    CriticalSectionLocker CSL(CS);
    return BlockMethod;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Internet_Set (size_t NewValue)
{
    CriticalSectionLocker CSL(CS);
    Internet=NewValue;
}

size_t MediaInfo_Config::Internet_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Internet;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::MultipleValues_Set (size_t NewValue)
{
    CriticalSectionLocker CSL(CS);
    MultipleValues=NewValue;
}

size_t MediaInfo_Config::MultipleValues_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MultipleValues;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::ParseUnknownExtensions_Set (size_t NewValue)
{
    CriticalSectionLocker CSL(CS);
    ParseUnknownExtensions=NewValue;
}

size_t MediaInfo_Config::ParseUnknownExtensions_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ParseUnknownExtensions;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::ShowFiles_Set (const ZtringListList &NewShowFiles)
{
    CriticalSectionLocker CSL(CS);
    for  (size_t Pos=0; Pos<NewShowFiles.size(); Pos++)
    {
        const Ztring& Object=NewShowFiles.Read(Pos, 0);
             if (Object==__T("Nothing"))
            ShowFiles_Nothing=NewShowFiles.Read(Pos, 1).empty()?1:0;
        else if (Object==__T("VideoAudio"))
            ShowFiles_VideoAudio=NewShowFiles.Read(Pos, 1).empty()?1:0;
        else if (Object==__T("VideoOnly"))
            ShowFiles_VideoOnly=NewShowFiles.Read(Pos, 1).empty()?1:0;
        else if (Object==__T("AudioOnly"))
            ShowFiles_AudioOnly=NewShowFiles.Read(Pos, 1).empty()?1:0;
        else if (Object==__T("TextOnly"))
            ShowFiles_TextOnly=NewShowFiles.Read(Pos, 1).empty()?1:0;
    }
}

size_t MediaInfo_Config::ShowFiles_Nothing_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ShowFiles_Nothing;
}

size_t MediaInfo_Config::ShowFiles_VideoAudio_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ShowFiles_VideoAudio;
}

size_t MediaInfo_Config::ShowFiles_VideoOnly_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ShowFiles_VideoOnly;
}

size_t MediaInfo_Config::ShowFiles_AudioOnly_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ShowFiles_AudioOnly;
}

size_t MediaInfo_Config::ShowFiles_TextOnly_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ShowFiles_TextOnly;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::ParseSpeed_Set (float32 NewValue)
{
    CriticalSectionLocker CSL(CS);
    ParseSpeed=NewValue;
}

float32 MediaInfo_Config::ParseSpeed_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ParseSpeed;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Verbosity_Set (float32 NewValue)
{
    CriticalSectionLocker CSL(CS);
    Verbosity=NewValue;
}

float32 MediaInfo_Config::Verbosity_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Verbosity;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::ReadByHuman_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    ReadByHuman=NewValue;
}

bool MediaInfo_Config::ReadByHuman_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ReadByHuman;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::LegacyStreamDisplay_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    LegacyStreamDisplay=NewValue;
}

bool MediaInfo_Config::LegacyStreamDisplay_Get ()
{
    CriticalSectionLocker CSL(CS);
    return LegacyStreamDisplay;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::SkipBinaryData_Set (bool NewValue)
{
    CriticalSectionLocker CSL(CS);
    SkipBinaryData=NewValue;
}

bool MediaInfo_Config::SkipBinaryData_Get ()
{
    CriticalSectionLocker CSL(CS);
    return SkipBinaryData;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Trace_Level_Set (const ZtringListList &NewTrace_Level)
{
    CriticalSectionLocker CSL(CS);

    //Global
    if (NewTrace_Level.size()==1 && NewTrace_Level[0].size()==1)
    {
        Trace_Level=NewTrace_Level[0][0].To_float32();
        if (Trace_Layers.to_ulong()==0) //if not set to a specific layer
            Trace_Layers.set();
        return;
    }

    //Per item
    else
    {
        Trace_Layers.reset();
        for (size_t Pos=0; Pos<NewTrace_Level.size(); Pos++)
        {
            if (NewTrace_Level[Pos].size()==2)
            {
                if (NewTrace_Level[Pos][0]==__T("Container1"))
                    Trace_Layers.set(0, NewTrace_Level[Pos][1].To_int64u()?true:false);
            }
        }
    }
}

float32 MediaInfo_Config::Trace_Level_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Trace_Level;
}

std::bitset<32> MediaInfo_Config::Trace_Layers_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Trace_Layers;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Trace_TimeSection_OnlyFirstOccurrence_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    Trace_TimeSection_OnlyFirstOccurrence=Value;
}

bool MediaInfo_Config::Trace_TimeSection_OnlyFirstOccurrence_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Trace_TimeSection_OnlyFirstOccurrence;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Trace_Format_Set (trace_Format NewValue)
{
    CriticalSectionLocker CSL(CS);
    Trace_Format=NewValue;
}

MediaInfo_Config::trace_Format MediaInfo_Config::Trace_Format_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Trace_Format;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Trace_Modificator_Set (const ZtringList &NewValue)
{
    ZtringList List(NewValue);
    if (List.size()!=2)
        return;
    transform(List[0].begin(), List[0].end(), List[0].begin(), (int(*)(int))tolower); //(int(*)(int)) is a patch for unix

    CriticalSectionLocker CSL(CS);
    Trace_Modificators[List[0]]=List[1]==__T("1");
}

Ztring MediaInfo_Config::Trace_Modificator_Get (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    std::map<Ztring, bool>::iterator ToReturn=Trace_Modificators.find(Value);
    if (ToReturn!=Trace_Modificators.end())
        return ToReturn->second?__T("1"):__T("0");
    else
        return Ztring();
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Demux_Set (int8u NewValue)
{
    CriticalSectionLocker CSL(CS);
    Demux=NewValue;
}

int8u MediaInfo_Config::Demux_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Demux;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::LineSeparator_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    LineSeparator=NewValue;
}

Ztring MediaInfo_Config::LineSeparator_Get ()
{
    CriticalSectionLocker CSL(CS);
    return LineSeparator;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Version_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    Version=ZtringListList(NewValue).Read(0); //Only the 1st value
}

Ztring MediaInfo_Config::Version_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Version;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::ColumnSeparator_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    ColumnSeparator=NewValue;
}

Ztring MediaInfo_Config::ColumnSeparator_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ColumnSeparator;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::TagSeparator_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    TagSeparator=NewValue;
}

Ztring MediaInfo_Config::TagSeparator_Get ()
{
    CriticalSectionLocker CSL(CS);
    return TagSeparator;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Quote_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    Quote=NewValue;
}

Ztring MediaInfo_Config::Quote_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Quote;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::DecimalPoint_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    DecimalPoint=NewValue;
}

Ztring MediaInfo_Config::DecimalPoint_Get ()
{
    CriticalSectionLocker CSL(CS);
    return DecimalPoint;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::ThousandsPoint_Set (const Ztring &NewValue)
{
    CriticalSectionLocker CSL(CS);
    ThousandsPoint=NewValue;
}

Ztring MediaInfo_Config::ThousandsPoint_Get ()
{
    CriticalSectionLocker CSL(CS);
    return ThousandsPoint;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::StreamMax_Set (const ZtringListList &)
{
    CriticalSectionLocker CSL(CS);
    //TODO : implementation
}

Ztring MediaInfo_Config::StreamMax_Get ()
{
    CriticalSectionLocker CSL(CS);
    ZtringListList StreamMax;
    //TODO : implementation
    return StreamMax.Read();
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Language_Set (const ZtringListList &NewValue)
{
    CriticalSectionLocker CSL(CS);

    //Which language to choose?
    //-Raw
         if (NewValue.size()==1 && NewValue[0].size()==1 && NewValue[0][0]==__T("raw"))
    {
        Language_Raw=true;
        Language.clear();
        //Exceptions
        Language.Write(__T("  Config_Text_ColumnSize"), __T("32"));
        Language.Write(__T("  Config_Text_Separator"), __T(" : "));
        Language.Write(__T("  Config_Text_NumberTag"), __T(" #"));
        Language.Write(__T("  Config_Text_FloatSeparator"), __T("."));
        Language.Write(__T("  Config_Text_ThousandsSeparator"), Ztring());
    }
    //-Add custom language to English language
    else
    {
        Language_Raw=false;
        //Fill base words (with English translation)
        MediaInfo_Config_DefaultLanguage(Language);
        //Add custom language to English language
        for (size_t Pos=0; Pos<NewValue.size(); Pos++)
            if (NewValue[Pos].size()>=2)
                Language.Write(NewValue[Pos][0], NewValue[Pos][1]);
            else if (NewValue[Pos].size()==1)
                Language.Write(NewValue[Pos][0], Ztring());
    }

    //Fill Info
    for (size_t StreamKind=0; StreamKind<Stream_Max; StreamKind++)
        if (!Info[StreamKind].empty())
            Language_Set((stream_t)StreamKind);
}

void MediaInfo_Config::Language_Set (stream_t StreamKind)
{
    //CriticalSectionLocker CSL(CS); //No, only used internaly

    //Fill Info
    for (size_t Pos=0; Pos<Info[StreamKind].size(); Pos++)
    {
        //Strings - Info_Name_Text
        Ztring ToReplace=Info[StreamKind](Pos, Info_Name);
        if (!Language_Raw && ToReplace.find(__T("/String"))!=Error)
        {
            ToReplace.FindAndReplace(__T("/String1"), Ztring());
            ToReplace.FindAndReplace(__T("/String2"), Ztring());
            ToReplace.FindAndReplace(__T("/String3"), Ztring());
            ToReplace.FindAndReplace(__T("/String4"), Ztring());
            ToReplace.FindAndReplace(__T("/String5"), Ztring());
            ToReplace.FindAndReplace(__T("/String6"), Ztring());
            ToReplace.FindAndReplace(__T("/String7"), Ztring());
            ToReplace.FindAndReplace(__T("/String8"), Ztring());
            ToReplace.FindAndReplace(__T("/String9"), Ztring());
            ToReplace.FindAndReplace(__T("/String"),  Ztring());
        }
        if (!Language_Raw && ToReplace.find(__T("/"))!=Error) //Complex values, like XXX/YYY --> We translate both XXX and YYY
        {
            Ztring ToReplace1=ToReplace.SubString(Ztring(), __T("/"));
            Ztring ToReplace2=ToReplace.SubString(__T("/"), Ztring());
            Info[StreamKind](Pos, Info_Name_Text)=Language.Get(ToReplace1);
            Info[StreamKind](Pos, Info_Name_Text)+=__T("/");
            Info[StreamKind](Pos, Info_Name_Text)+=Language.Get(ToReplace2);
        }
        else
            Info[StreamKind](Pos, Info_Name_Text)=Language.Get(ToReplace);
        //Strings - Info_Measure_Text
        Info[StreamKind](Pos, Info_Measure_Text).clear(); //I don(t know why, but if I don't do this Delphi/C# debugger make crashing the calling program
        Info[StreamKind](Pos, Info_Measure_Text)=Language.Get(Info[StreamKind](Pos, Info_Measure));
        //Slashes

    }
}

Ztring MediaInfo_Config::Language_Get ()
{
    CriticalSectionLocker CSL(CS);
    Ztring ToReturn;//TODO =Language.Read();
    return ToReturn;
}

Ztring MediaInfo_Config::Language_Get (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);

    if (Value.find(__T(" / "))==string::npos)
        return Language.Get(Value);

    ZtringList List;
    List.Separator_Set(0, __T(" / "));
    List.Write(Value);

    //Per value
    for (size_t Pos=0; Pos<List.size(); Pos++)
        List[Pos]=Language.Get(List[Pos]);

    return List.Read();
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Config::Language_Get (const Ztring &Count, const Ztring &Value, bool ValueIsAlwaysSame)
{
    //Integrity
    if (Count.empty())
        return EmptyString_Get();

    //Different Plurals are available or not?
    if (Language_Get(Value+__T("1")).empty())
    {
        //if (Count==__T("0") || Count==__T("1"))
            return Count+Language_Get(Value);
        //else
            //return Count+Language_Get(Value+__T("s"));
    }

    //Detecting plural form for multiple plurals
    int8u  Form=(int8u)-1;

    if (!ValueIsAlwaysSame)
    {
        //Polish has 2 plurial, Algorithm of Polish
        size_t CountI=Count.To_int32u();
        size_t Pos3=CountI/100;
        int8u  Pos2=(int8u)((CountI-Pos3*100)/10);
        int8u  Pos1=(int8u)(CountI-Pos3*100-Pos2*10);
        if (Pos3==0)
        {
            if (Pos2==0)
            {
                     if (Pos1==0 && Count.size()==1) //Only "0", not "0.xxx"
                    Form=0; //000 to 000 kanal?
                else if (Pos1<=1)
                    Form=1; //001 to 001 kanal
                else if (Pos1<=4)
                    Form=2; //002 to 004 kanaly
                else //if (Pos1>=5)
                    Form=3; //005 to 009 kanalow
            }
            else if (Pos2==1)
                    Form=3; //010 to 019 kanalow
            else //if (Pos2>=2)
            {
                     if (Pos1<=1)
                    Form=3; //020 to 021, 090 to 091 kanalow
                else if (Pos1<=4)
                    Form=2; //022 to 024, 092 to 094 kanali
                else //if (Pos1>=5)
                    Form=3; //025 to 029, 095 to 099 kanalow
            }
        }
        else //if (Pos3>=1)
        {
            if (Pos2==0)
            {
                     if (Pos1<=1)
                    Form=3; //100 to 101 kanalow
                else if (Pos1<=4)
                    Form=2; //102 to 104 kanaly
                else //if (Pos1>=5)
                    Form=3; //105 to 109 kanalow
            }
            else if (Pos2==1)
                    Form=3; //110 to 119 kanalow
            else //if (Pos2>=2)
            {
                     if (Pos1<=1)
                    Form=3; //120 to 121, 990 to 991 kanalow
                else if (Pos1<=4)
                    Form=2; //122 to 124, 992 to 994 kanali
                else //if (Pos1>=5)
                    Form=3; //125 to 129, 995 to 999 kanalow
            }
        }
    }

    //Replace dot and thousand separator
    Ztring ToReturn=Count;
    Ztring DecimalPoint=Ztring().From_Number(0.0, 1).substr(1, 1); //Getting Decimal point
    size_t DotPos=ToReturn.find(DecimalPoint);
    if (DotPos!=string::npos)
        ToReturn.FindAndReplace(DecimalPoint, Language_Get(__T("  Config_Text_FloatSeparator")), DotPos);
    else
        DotPos=ToReturn.size();
    if (DotPos>3)
        ToReturn.insert(DotPos-3, Language_Get(__T("  Config_Text_ThousandsSeparator")));

    //Selecting the form
         if (Form==0)
        ToReturn =Language_Get(Value+__T("0")); //Only the translation
    else if (Form==1)
        ToReturn+=Language_Get(Value+__T("1"));
    else if (Form==2)
        ToReturn+=Language_Get(Value+__T("2"));
    else if (Form==3)
        ToReturn+=Language_Get(Value+__T("3"));
    else
        ToReturn+=Language_Get(Value);
    return ToReturn;
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Inform_Set (const ZtringListList &NewValue)
{
    if (NewValue.Read(0, 0)==__T("Details"))
        Trace_Level_Set(NewValue.Read(0, 1));
    else
    {
        Trace_Level_Set(__T("0"));

        CriticalSectionLocker CSL(CS);

        //Inform
        if (NewValue==__T("Summary"))
            MediaInfo_Config_Summary(Custom_View);
        else
            Custom_View=NewValue;
    }

    CriticalSectionLocker CSL(CS);

    //Parsing pointers to files in streams
    for (size_t Pos=0; Pos<Custom_View.size(); Pos++)
    {
        if (Custom_View[Pos].size()>1 && Custom_View(Pos, 1).find(__T("file://"))==0)
        {
            //Open
            Ztring FileName(Custom_View(Pos, 1), 7, Ztring::npos);
            File F(FileName.c_str());

            //Read
            int64u Size=F.Size_Get();
            if (Size>=0xFFFFFFFF)
                Size=1024*1024;
            int8u* Buffer=new int8u[(size_t)Size+1];
            size_t F_Offset=F.Read(Buffer, (size_t)Size);
            F.Close();
            Buffer[F_Offset]='\0';
            Ztring FromFile; FromFile.From_Local((char*)Buffer);
            delete[] Buffer; //Buffer=NULL;

            //Merge
            FromFile.FindAndReplace(__T("\r\n"), __T("\\r\\n"), 0, Ztring_Recursive);
            FromFile.FindAndReplace(__T("\n"), __T("\\r\\n"), 0, Ztring_Recursive);
            Custom_View(Pos, 1)=FromFile;
        }
    }
}

Ztring MediaInfo_Config::Inform_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Custom_View.Read();
}

Ztring MediaInfo_Config::Inform_Get (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    size_t Pos=Custom_View.Find(Value);
    if (Pos==Error || 1>=Custom_View[Pos].size())
        return EmptyString_Get();
    return Custom_View[Pos][1];
}

//---------------------------------------------------------------------------
void MediaInfo_Config::Inform_Replace_Set (const ZtringListList &NewValue_Replace)
{
    CriticalSectionLocker CSL(CS);

    //Parsing
    for (size_t Pos=0; Pos<NewValue_Replace.size(); Pos++)
    {
        if (NewValue_Replace[Pos].size()==2)
            Custom_View_Replace(NewValue_Replace[Pos][0])=NewValue_Replace[Pos][1];
    }
}

ZtringListList MediaInfo_Config::Inform_Replace_Get_All ()
{
    CriticalSectionLocker CSL(CS);
    return Custom_View_Replace;
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Format_Get (const Ztring &Value, infoformat_t KindOfFormatInfo)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Format.empty())
        MediaInfo_Config_Format(Format);
    CS.Leave();

    return Format.Get(Value, KindOfFormatInfo);
}

//---------------------------------------------------------------------------
InfoMap &MediaInfo_Config::Format_Get ()
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Format.empty())
        MediaInfo_Config_Format(Format);
    CS.Leave();

    return Format;
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Codec_Get (const Ztring &Value, infocodec_t KindOfCodecInfo)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Codec.empty())
        MediaInfo_Config_Codec(Codec);
    CS.Leave();

    return Codec.Get(Value, KindOfCodecInfo);
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Codec_Get (const Ztring &Value, infocodec_t KindOfCodecInfo, stream_t KindOfStream)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Codec.empty())
        MediaInfo_Config_Codec(Codec);
    CS.Leave();

    //Transform to text
    Ztring KindOfStreamS;
    switch (KindOfStream)
    {
        case Stream_General  : KindOfStreamS=__T("G"); break;
        case Stream_Video    : KindOfStreamS=__T("V"); break;
        case Stream_Audio    : KindOfStreamS=__T("A"); break;
        case Stream_Text     : KindOfStreamS=__T("T"); break;
        case Stream_Image    : KindOfStreamS=__T("I"); break;
        case Stream_Other : KindOfStreamS=__T("C"); break;
        case Stream_Menu     : KindOfStreamS=__T("M"); break;
        case Stream_Max      : KindOfStreamS=__T(" "); break;
    }

    return Codec.Get(Value, KindOfCodecInfo, KindOfStreamS, InfoCodec_KindOfStream);
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::CodecID_Get (stream_t KindOfStream, infocodecid_format_t Format, const Ztring &Value, infocodecid_t KindOfCodecIDInfo)
{
    if (Format>=InfoCodecID_Format_Max || KindOfStream>=Stream_Max)
        return EmptyString_Get();

    CS.Enter();
    if (CodecID[Format][KindOfStream].empty())
    {
        switch (KindOfStream)
        {
            case Stream_General :
                                    switch (Format)
                                    {
                                        case InfoCodecID_Format_Mpeg4 : MediaInfo_Config_CodecID_General_Mpeg4(CodecID[Format][KindOfStream]); break;
                                        default: ;
                                    }
                                    break;
            case Stream_Video   :
                                    switch (Format)
                                    {
                                        case InfoCodecID_Format_Matroska : MediaInfo_Config_CodecID_Video_Matroska(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Mpeg4    : MediaInfo_Config_CodecID_Video_Mpeg4(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Real     : MediaInfo_Config_CodecID_Video_Real(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Riff     : MediaInfo_Config_CodecID_Video_Riff(CodecID[Format][KindOfStream]); break;
                                        default: ;
                                    }
                                    break;
            case Stream_Audio   :
                                    switch (Format)
                                    {
                                        case InfoCodecID_Format_Matroska : MediaInfo_Config_CodecID_Audio_Matroska(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Mpeg4    : MediaInfo_Config_CodecID_Audio_Mpeg4(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Real     : MediaInfo_Config_CodecID_Audio_Real(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Riff     : MediaInfo_Config_CodecID_Audio_Riff(CodecID[Format][KindOfStream]); break;
                                        default: ;
                                    }
                                    break;
            case Stream_Text    :
                                    switch (Format)
                                    {
                                        case InfoCodecID_Format_Matroska : MediaInfo_Config_CodecID_Text_Matroska(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Mpeg4    : MediaInfo_Config_CodecID_Text_Mpeg4(CodecID[Format][KindOfStream]); break;
                                        case InfoCodecID_Format_Riff     : MediaInfo_Config_CodecID_Text_Riff(CodecID[Format][KindOfStream]); break;
                                        default: ;
                                    }
                                    break;
            default: ;
        }
    }
    CS.Leave();
    return CodecID[Format][KindOfStream].Get(Value, KindOfCodecIDInfo);
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Library_Get (infolibrary_format_t Format, const Ztring &Value, infolibrary_t KindOfLibraryInfo)
{
    if (Format>=InfoLibrary_Format_Max)
        return EmptyString_Get();

    CS.Enter();
    if (Library[Format].empty())
    {
        switch (Format)
        {
            case InfoLibrary_Format_DivX : MediaInfo_Config_Library_DivX(Library[Format]); break;
            case InfoLibrary_Format_XviD : MediaInfo_Config_Library_XviD(Library[Format]); break;
            case InfoLibrary_Format_MainConcept_Avc : MediaInfo_Config_Library_MainConcept_Avc(Library[Format]); break;
            case InfoLibrary_Format_VorbisCom : MediaInfo_Config_Library_VorbisCom(Library[Format]); break;
            default: ;
        }
    }
    CS.Leave();
    return Library[Format].Get(Value, KindOfLibraryInfo);
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Iso639_1_Get (const Ztring &Value)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Iso639_1.empty())
        MediaInfo_Config_Iso639_1(Iso639_1);
    CS.Leave();

    return Iso639_1.Get(Ztring(Value).MakeLowerCase(), 1);
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Iso639_2_Get (const Ztring &Value)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Iso639_2.empty())
        MediaInfo_Config_Iso639_2(Iso639_2);
    CS.Leave();

    return Iso639_2.Get(Ztring(Value).MakeLowerCase(), 1);
}

//---------------------------------------------------------------------------
const Ztring MediaInfo_Config::Iso639_Find (const Ztring &Value)
{
    Translation Info;
    MediaInfo_Config_DefaultLanguage (Info);
    Ztring Value_Lower(Value);
    Value_Lower.MakeLowerCase();

    for (Translation::iterator Trans=Info.begin(); Trans!=Info.end(); ++Trans)
    {
        Trans->second.MakeLowerCase();
        if (Trans->second==Value_Lower && Trans->first.find(__T("Language_"))==0)
            return Trans->first.substr(9, string::npos);
    }
    return Ztring();
}

//---------------------------------------------------------------------------
const Ztring &MediaInfo_Config::Info_Get (stream_t KindOfStream, const Ztring &Value, info_t KindOfInfo)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Info[KindOfStream].empty())
        switch (KindOfStream)
        {
            case Stream_General :   MediaInfo_Config_General(Info[Stream_General]);   Language_Set(Stream_General); break;
            case Stream_Video :     MediaInfo_Config_Video(Info[Stream_Video]);       Language_Set(Stream_Video); break;
            case Stream_Audio :     MediaInfo_Config_Audio(Info[Stream_Audio]);       Language_Set(Stream_Audio); break;
            case Stream_Text :      MediaInfo_Config_Text(Info[Stream_Text]);         Language_Set(Stream_Text); break;
            case Stream_Other :     MediaInfo_Config_Other(Info[Stream_Other]);       Language_Set(Stream_Other); break;
            case Stream_Image :     MediaInfo_Config_Image(Info[Stream_Image]);       Language_Set(Stream_Image); break;
            case Stream_Menu :      MediaInfo_Config_Menu(Info[Stream_Menu]);         Language_Set(Stream_Menu); break;
            default:;
        }
    CS.Leave();

    if (KindOfStream>=Stream_Max)
        return EmptyString_Get();
    size_t Pos=Info[KindOfStream].Find(Value);
    if (Pos==Error || (size_t)KindOfInfo>=Info[KindOfStream][Pos].size())
        return EmptyString_Get();
    return Info[KindOfStream][Pos][KindOfInfo];
}

const Ztring &MediaInfo_Config::Info_Get (stream_t KindOfStream, size_t Pos, info_t KindOfInfo)
{
    //Loading codec table if not yet done
    CS.Enter();
    if (Info[KindOfStream].empty())
        switch (KindOfStream)
        {
            case Stream_General :   MediaInfo_Config_General(Info[Stream_General]);   Language_Set(Stream_General); break;
            case Stream_Video :     MediaInfo_Config_Video(Info[Stream_Video]);       Language_Set(Stream_Video); break;
            case Stream_Audio :     MediaInfo_Config_Audio(Info[Stream_Audio]);       Language_Set(Stream_Audio); break;
            case Stream_Text :      MediaInfo_Config_Text(Info[Stream_Text]);         Language_Set(Stream_Text); break;
            case Stream_Other :     MediaInfo_Config_Other(Info[Stream_Other]);       Language_Set(Stream_Other); break;
            case Stream_Image :     MediaInfo_Config_Image(Info[Stream_Image]);       Language_Set(Stream_Image); break;
            case Stream_Menu :      MediaInfo_Config_Menu(Info[Stream_Menu]);         Language_Set(Stream_Menu); break;
            default:;
        }
    CS.Leave();

    if (KindOfStream>=Stream_Max)
        return EmptyString_Get();
    if (Pos>=Info[KindOfStream].size() || (size_t)KindOfInfo>=Info[KindOfStream][Pos].size())
        return EmptyString_Get();
    return Info[KindOfStream][Pos][KindOfInfo];
}

const ZtringListList &MediaInfo_Config::Info_Get(stream_t KindOfStream)
{
    if (KindOfStream>=Stream_Max)
        return EmptyStringListList_Get();

    //Loading codec table if not yet done
    CS.Enter();
    if (Info[KindOfStream].empty())
        switch (KindOfStream)
        {
            case Stream_General :   MediaInfo_Config_General(Info[Stream_General]);   Language_Set(Stream_General); break;
            case Stream_Video :     MediaInfo_Config_Video(Info[Stream_Video]);       Language_Set(Stream_Video); break;
            case Stream_Audio :     MediaInfo_Config_Audio(Info[Stream_Audio]);       Language_Set(Stream_Audio); break;
            case Stream_Text :      MediaInfo_Config_Text(Info[Stream_Text]);         Language_Set(Stream_Text); break;
            case Stream_Other :     MediaInfo_Config_Other(Info[Stream_Other]);       Language_Set(Stream_Other); break;
            case Stream_Image :     MediaInfo_Config_Image(Info[Stream_Image]);       Language_Set(Stream_Image); break;
            case Stream_Menu :      MediaInfo_Config_Menu(Info[Stream_Menu]);         Language_Set(Stream_Menu); break;
            default:;
        }
    CS.Leave();

    return Info[KindOfStream];
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Config::Info_Parameters_Get (bool Complete)
{
    CriticalSectionLocker CSL(CS);

    //Loading all
    MediaInfo_Config_General(Info[Stream_General]);
    MediaInfo_Config_Video(Info[Stream_Video]);
    MediaInfo_Config_Audio(Info[Stream_Audio]);
    MediaInfo_Config_Text(Info[Stream_Text]);
    MediaInfo_Config_Other(Info[Stream_Other]);
    MediaInfo_Config_Image(Info[Stream_Image]);
    MediaInfo_Config_Menu(Info[Stream_Menu]);

    //Building
    ZtringListList ToReturn;
    size_t ToReturn_Pos=0;

    for (size_t StreamKind=0; StreamKind<Stream_Max; StreamKind++)
    {
        ToReturn(ToReturn_Pos, 0)=Info[StreamKind].Read(__T("StreamKind"), Info_Text);
        ToReturn_Pos++;
        for (size_t Pos=0; Pos<Info[StreamKind].size(); Pos++)
            if (!Info[StreamKind].Read(Pos, Info_Name).empty())
            {
                if (Complete)
                    ToReturn.push_back(Info[StreamKind].Read(Pos));
                else
                {
                    ToReturn(ToReturn_Pos, 0)=Info[StreamKind].Read(Pos, Info_Name);
                    ToReturn(ToReturn_Pos, 1)=Info[StreamKind].Read(Pos, Info_Info);
                }
                ToReturn_Pos++;
            }
        ToReturn_Pos++;
    }
    return ToReturn.Read();
}

//---------------------------------------------------------------------------
Ztring MediaInfo_Config::Info_Tags_Get () const
{
    return Ztring();
}

Ztring MediaInfo_Config::Info_Codecs_Get ()
{
    CriticalSectionLocker CSL(CS);

    //Loading
    MediaInfo_Config_Codec(Codec);

    //Building
    Ztring ToReturn;
    InfoMap::iterator Temp=Codec.begin();
    while (Temp!=Codec.end())
    {
        ToReturn+=Temp->second.Read();
        ToReturn+=EOL;
        ++Temp;
    }

    return ToReturn;
}

Ztring MediaInfo_Config::Info_Version_Get () const
{
    return MediaInfo_Version;
}

Ztring MediaInfo_Config::Info_Url_Get () const
{
    return MediaInfo_Url;
}

const Ztring &MediaInfo_Config::EmptyString_Get () const
{
    return EmptyZtring_Const;
}

const ZtringListList &MediaInfo_Config::EmptyStringListList_Get () const
{
    return EmptyZtringListList_Const;
}

void MediaInfo_Config::FormatDetection_MaximumOffset_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);
    FormatDetection_MaximumOffset=Value;
}

int64u MediaInfo_Config::FormatDetection_MaximumOffset_Get ()
{
    CriticalSectionLocker CSL(CS);
    return FormatDetection_MaximumOffset;
}

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::VariableGopDetection_Occurences_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);
    VariableGopDetection_Occurences=Value;
}

int64u MediaInfo_Config::VariableGopDetection_Occurences_Get ()
{
    CriticalSectionLocker CSL(CS);
    return VariableGopDetection_Occurences;
}
#endif // MEDIAINFO_ADVANCED

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::VariableGopDetection_GiveUp_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    VariableGopDetection_GiveUp=Value;
}

bool MediaInfo_Config::VariableGopDetection_GiveUp_Get ()
{
    CriticalSectionLocker CSL(CS);
    return VariableGopDetection_GiveUp;
}
#endif // MEDIAINFO_ADVANCED

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::InitDataNotRepeated_Occurences_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);
    InitDataNotRepeated_Occurences=Value;
}

int64u MediaInfo_Config::InitDataNotRepeated_Occurences_Get ()
{
    CriticalSectionLocker CSL(CS);
    return InitDataNotRepeated_Occurences;
}
#endif // MEDIAINFO_ADVANCED

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::InitDataNotRepeated_GiveUp_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    InitDataNotRepeated_GiveUp=Value;
}

bool MediaInfo_Config::InitDataNotRepeated_GiveUp_Get ()
{
    CriticalSectionLocker CSL(CS);
    return InitDataNotRepeated_GiveUp;
}
#endif // MEDIAINFO_ADVANCED

void MediaInfo_Config::MpegTs_MaximumOffset_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);
    MpegTs_MaximumOffset=Value;
}

int64u MediaInfo_Config::MpegTs_MaximumOffset_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MpegTs_MaximumOffset;
}

void MediaInfo_Config::MpegTs_MaximumScanDuration_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);
    MpegTs_MaximumScanDuration=Value;
}

int64u MediaInfo_Config::MpegTs_MaximumScanDuration_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MpegTs_MaximumScanDuration;
}

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::MpegTs_VbrDetection_Delta_Set (float64 Value)
{
    CriticalSectionLocker CSL(CS);
    MpegTs_VbrDetection_Delta=Value;
}

float64 MediaInfo_Config::MpegTs_VbrDetection_Delta_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MpegTs_VbrDetection_Delta;
}
#endif // MEDIAINFO_ADVANCED

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::MpegTs_VbrDetection_Occurences_Set (int64u Value)
{
    CriticalSectionLocker CSL(CS);
    MpegTs_VbrDetection_Occurences=Value;
}

int64u MediaInfo_Config::MpegTs_VbrDetection_Occurences_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MpegTs_VbrDetection_Occurences;
}
#endif // MEDIAINFO_ADVANCED

#if MEDIAINFO_ADVANCED
void MediaInfo_Config::MpegTs_VbrDetection_GiveUp_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    MpegTs_VbrDetection_GiveUp=Value;
}

bool MediaInfo_Config::MpegTs_VbrDetection_GiveUp_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MpegTs_VbrDetection_GiveUp;
}
#endif // MEDIAINFO_ADVANCED

void MediaInfo_Config::MpegTs_ForceStreamDisplay_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    MpegTs_ForceStreamDisplay=Value;
}

bool MediaInfo_Config::MpegTs_ForceStreamDisplay_Get ()
{
    CriticalSectionLocker CSL(CS);
    return MpegTs_ForceStreamDisplay;
}

//***************************************************************************
// SubFile
//***************************************************************************

//---------------------------------------------------------------------------
ZtringListList MediaInfo_Config::SubFile_Config_Get ()
{
    CriticalSectionLocker CSL(CS);

    return SubFile_Config;
}

//***************************************************************************
// Custom mapping
//***************************************************************************

void MediaInfo_Config::CustomMapping_Set (const Ztring &Value)
{
    ZtringList List; List.Separator_Set(0, __T(","));
    List.Write(Value);
    if (List.size()==3)
    {
        CriticalSectionLocker CSL(CS);
        CustomMapping[List[0]][List[1]]=List[2];
    }
}

Ztring MediaInfo_Config::CustomMapping_Get (const Ztring &Format, const Ztring &Field)
{
    CriticalSectionLocker CSL(CS);
    return CustomMapping[Format][Field];
}

bool MediaInfo_Config::CustomMapping_IsPresent(const Ztring &Format, const Ztring &Field)
{
    CriticalSectionLocker CSL(CS);
    std::map<Ztring, std::map<Ztring, Ztring> >::iterator PerFormat=CustomMapping.find(Format);
    if (PerFormat==CustomMapping.end())
        return false;
    std::map<Ztring, Ztring>::iterator PerField=PerFormat->second.find(Field);
    if (PerField==PerFormat->second.end())
        return false;
    return true;
}

//***************************************************************************
// Event
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
bool MediaInfo_Config::Event_CallBackFunction_IsSet ()
{
    CriticalSectionLocker CSL(CS);

    return Event_CallBackFunction?true:false;
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
Ztring MediaInfo_Config::Event_CallBackFunction_Set (const Ztring &Value)
{
    ZtringList List=Value;

    CriticalSectionLocker CSL(CS);

    if (List.empty())
    {
        Event_CallBackFunction=(MediaInfo_Event_CallBackFunction*)NULL;
        Event_UserHandler=NULL;
    }
    else
        for (size_t Pos=0; Pos<List.size(); Pos++)
        {
            if (List[Pos].find(__T("CallBack=memory://"))==0)
                Event_CallBackFunction=(MediaInfo_Event_CallBackFunction*)Ztring(List[Pos].substr(18, std::string::npos)).To_int64u();
            else if (List[Pos].find(__T("UserHandle=memory://"))==0)
                Event_UserHandler=(void*)Ztring(List[Pos].substr(20, std::string::npos)).To_int64u();
            else if (List[Pos].find(__T("UserHandler=memory://"))==0)
                Event_UserHandler=(void*)Ztring(List[Pos].substr(21, std::string::npos)).To_int64u();
            else
                return("Problem during Event_CallBackFunction value parsing");
        }

    return Ztring();
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
Ztring MediaInfo_Config::Event_CallBackFunction_Get ()
{
    CriticalSectionLocker CSL(CS);

    return __T("CallBack=memory://")+Ztring::ToZtring((size_t)Event_CallBackFunction)+__T(";UserHandler=memory://")+Ztring::ToZtring((size_t)Event_UserHandler);
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void MediaInfo_Config::Event_Send (const int8u* Data_Content, size_t Data_Size)
{
    CriticalSectionLocker CSL(CS);

    if (Event_CallBackFunction)
        Event_CallBackFunction ((unsigned char*)Data_Content, Data_Size, Event_UserHandler);
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void MediaInfo_Config::Event_Send (const int8u* Data_Content, size_t Data_Size, const Ztring &File_Name)
{
    CriticalSectionLocker CSL(CS);

    if (Event_CallBackFunction)
        Event_CallBackFunction ((unsigned char*)Data_Content, Data_Size, Event_UserHandler);
}
#endif //MEDIAINFO_EVENTS

//---------------------------------------------------------------------------
#if MEDIAINFO_EVENTS
void MediaInfo_Config::Log_Send (int8u Type, int8u Severity, int32u MessageCode, const Ztring &Message)
{
    struct MediaInfo_Event_Log_0 Event;
    Event.EventCode=MediaInfo_EventCode_Create(MediaInfo_Parser_None, MediaInfo_Event_Log, 0);
    Event.Type=Type;
    Event.Severity=Severity;
    Event.Reserved2=(int8u)-1;
    Event.Reserved3=(int8u)-1;
    Event.MessageCode=MessageCode;
    Event.Reserved4=(int32u)-1;
    wstring MessageU=Message.To_Unicode();
    string MessageA=Message.To_Local();
    Event.MessageStringU=MessageU.c_str();
    Event.MessageStringA=MessageA.c_str();
    Event_Send((const int8u*)&Event, sizeof(MediaInfo_Event_Log_0));
}
#endif //MEDIAINFO_EVENTS

//***************************************************************************
// Curl
//***************************************************************************

#if defined(MEDIAINFO_LIBCURL_YES)
void MediaInfo_Config::Ssh_PublicKeyFileName_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssh_PublicKeyFileName=Value;
}

Ztring MediaInfo_Config::Ssh_PublicKeyFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssh_PublicKeyFileName;
}

void MediaInfo_Config::Ssh_PrivateKeyFileName_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssh_PrivateKeyFileName=Value;
}

Ztring MediaInfo_Config::Ssh_PrivateKeyFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssh_PrivateKeyFileName;
}

void MediaInfo_Config::Ssh_KnownHostsFileName_Set (const Ztring &Value)
{
    if (Value.empty())
        return; //empty value means "disable security" for libcurl, not acceptable

    CriticalSectionLocker CSL(CS);
    Ssh_KnownHostsFileName=Value;
}

Ztring MediaInfo_Config::Ssh_KnownHostsFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssh_KnownHostsFileName;
}

void MediaInfo_Config::Ssh_IgnoreSecurity_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    Ssh_IgnoreSecurity=Value;
}

bool MediaInfo_Config::Ssh_IgnoreSecurity_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssh_IgnoreSecurity;
}

void MediaInfo_Config::Ssl_CertificateFileName_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_CertificateFileName=Value;
}

Ztring MediaInfo_Config::Ssl_CertificateFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_CertificateFileName;
}

void MediaInfo_Config::Ssl_CertificateFormat_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_CertificateFormat=Value;
}

Ztring MediaInfo_Config::Ssl_CertificateFormat_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_CertificateFormat;
}

void MediaInfo_Config::Ssl_PrivateKeyFileName_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_PrivateKeyFileName=Value;
}

Ztring MediaInfo_Config::Ssl_PrivateKeyFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_PrivateKeyFileName;
}

void MediaInfo_Config::Ssl_PrivateKeyFormat_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_PrivateKeyFormat=Value;
}

Ztring MediaInfo_Config::Ssl_PrivateKeyFormat_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_PrivateKeyFormat;
}

void MediaInfo_Config::Ssl_CertificateAuthorityFileName_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_CertificateAuthorityFileName=Value;
}

Ztring MediaInfo_Config::Ssl_CertificateAuthorityFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_CertificateAuthorityFileName;
}

void MediaInfo_Config::Ssl_CertificateAuthorityPath_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_CertificateAuthorityPath=Value;
}

Ztring MediaInfo_Config::Ssl_CertificateAuthorityPath_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_CertificateAuthorityPath;
}

void MediaInfo_Config::Ssl_CertificateRevocationListFileName_Set (const Ztring &Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_CertificateRevocationListFileName=Value;
}

Ztring MediaInfo_Config::Ssl_CertificateRevocationListFileName_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_CertificateRevocationListFileName;
}

void MediaInfo_Config::Ssl_IgnoreSecurity_Set (bool Value)
{
    CriticalSectionLocker CSL(CS);
    Ssl_IgnoreSecurity=Value;
}

bool MediaInfo_Config::Ssl_IgnoreSecurity_Get ()
{
    CriticalSectionLocker CSL(CS);
    return Ssl_IgnoreSecurity;
}

#endif //defined(MEDIAINFO_LIBCURL_YES)

} //NameSpace
