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
#ifndef MediaInfo_ConfigH
#define MediaInfo_ConfigH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo_Internal_Const.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "ZenLib/CriticalSection.h"
#include "ZenLib/ZtringListList.h"
#include "ZenLib/Translation.h"
#include "ZenLib/InfoMap.h"
#include <bitset>
using namespace ZenLib;
using std::vector;
using std::string;
using std::map;
using std::make_pair;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class MediaInfo_Config
//***************************************************************************

class MediaInfo_Config
{
public :
    //Constructor/Destructor
    MediaInfo_Config() {}
    void Init(); //Must be called instead of constructor

    //General
    Ztring Option (const String &Option, const String &Value=Ztring());

    //Info
          void      Complete_Set (size_t NewValue);
          size_t    Complete_Get ();

          void      BlockMethod_Set (size_t NewValue);
          size_t    BlockMethod_Get ();

          void      Internet_Set (size_t NewValue);
          size_t    Internet_Get ();

          void      MultipleValues_Set (size_t NewValue);
          size_t    MultipleValues_Get ();

          void      ParseUnknownExtensions_Set (size_t NewValue);
          size_t    ParseUnknownExtensions_Get ();

          void      ShowFiles_Set (const ZtringListList &NewShowFiles);
          size_t    ShowFiles_Nothing_Get ();
          size_t    ShowFiles_VideoAudio_Get ();
          size_t    ShowFiles_VideoOnly_Get ();
          size_t    ShowFiles_AudioOnly_Get ();
          size_t    ShowFiles_TextOnly_Get ();

          void      ReadByHuman_Set (bool NewValue);
          bool      ReadByHuman_Get ();

          void      LegacyStreamDisplay_Set (bool Value);
          bool      LegacyStreamDisplay_Get ();

          void      SkipBinaryData_Set (bool Value);
          bool      SkipBinaryData_Get ();

          void      ParseSpeed_Set (float32 NewValue);
          float32   ParseSpeed_Get ();

          void      Verbosity_Set (float32 NewValue);
          float32   Verbosity_Get ();

          void      Trace_Level_Set (const ZtringListList &NewDetailsLevel);
          float32   Trace_Level_Get ();
          std::bitset<32> Trace_Layers_Get ();

          void      Trace_TimeSection_OnlyFirstOccurrence_Set (bool Value);
          bool      Trace_TimeSection_OnlyFirstOccurrence_Get ();

          enum trace_Format
          {
              Trace_Format_Tree,
              Trace_Format_CSV,
          };
          void      Trace_Format_Set (trace_Format NewValue);
          trace_Format Trace_Format_Get ();

          void      Trace_Modificator_Set (const ZtringList &NewModifcator); //Not implemented
          Ztring    Trace_Modificator_Get (const Ztring &Modificator); //Not implemented

          void      Demux_Set (int8u NewValue);
          int8u     Demux_Get ();

          void      LineSeparator_Set (const Ztring &NewValue);
          Ztring    LineSeparator_Get ();

          void      Version_Set (const Ztring &NewValue);
          Ztring    Version_Get ();

          void      ColumnSeparator_Set (const Ztring &NewValue);
          Ztring    ColumnSeparator_Get ();

          void      TagSeparator_Set (const Ztring &NewValue);
          Ztring    TagSeparator_Get ();

          void      Quote_Set (const Ztring &NewValue);
          Ztring    Quote_Get ();

          void      DecimalPoint_Set (const Ztring &NewValue);
          Ztring    DecimalPoint_Get ();

          void      ThousandsPoint_Set (const Ztring &NewValue);
          Ztring    ThousandsPoint_Get ();

          void      StreamMax_Set (const ZtringListList &NewValue);
          Ztring    StreamMax_Get ();

          void      Language_Set (const ZtringListList &NewLanguage);
          Ztring    Language_Get ();
          Ztring    Language_Get (const Ztring &Value);
          Ztring    Language_Get (const Ztring &Count, const Ztring &Value, bool ValueIsAlwaysSame=false);

          void      Inform_Set (const ZtringListList &NewInform);
          Ztring    Inform_Get ();
          Ztring    Inform_Get (const Ztring &Value);

          void      Inform_Replace_Set (const ZtringListList &NewInform_Replace);
          ZtringListList Inform_Replace_Get_All ();

    const Ztring   &Format_Get (const Ztring &Value, infoformat_t KindOfFormatInfo=InfoFormat_Name);
          InfoMap  &Format_Get(); //Should not be, but too difficult to hide it

    const Ztring   &Codec_Get (const Ztring &Value, infocodec_t KindOfCodecInfo=InfoCodec_Name);
    const Ztring   &Codec_Get (const Ztring &Value, infocodec_t KindOfCodecInfo, stream_t KindOfStream);

    const Ztring   &CodecID_Get (stream_t KindOfStream, infocodecid_format_t Format, const Ztring &Value, infocodecid_t KindOfCodecIDInfo=InfoCodecID_Format);

    const Ztring   &Library_Get (infolibrary_format_t Format, const Ztring &Value, infolibrary_t KindOfLibraryInfo=InfoLibrary_Version);

    const Ztring   &Iso639_1_Get (const Ztring &Value);
    const Ztring   &Iso639_2_Get (const Ztring &Value);
    const Ztring    Iso639_Find (const Ztring &Value);
    const Ztring    Iso639_Translate (const Ztring Value);

    const Ztring   &Info_Get (stream_t KindOfStream, const Ztring &Value, info_t KindOfInfo=Info_Text);
    const Ztring   &Info_Get (stream_t KindOfStream, size_t Pos, info_t KindOfInfo=Info_Text);
    const ZtringListList &Info_Get(stream_t KindOfStream); //Should not be, but too difficult to hide it

          Ztring    Info_Parameters_Get (bool Complete=false);
          Ztring    Info_Tags_Get       () const;
          Ztring    Info_CodecsID_Get   ();
          Ztring    Info_Codecs_Get     ();
          Ztring    Info_Version_Get    () const;
          Ztring    Info_Url_Get        () const;

    const Ztring   &EmptyString_Get() const; //Use it when we can't return a reference to a true string
    const ZtringListList &EmptyStringListList_Get() const; //Use it when we can't return a reference to a true string list list

          void      FormatDetection_MaximumOffset_Set (int64u Value);
          int64u    FormatDetection_MaximumOffset_Get ();

    #if MEDIAINFO_ADVANCED
          void      VariableGopDetection_Occurences_Set (int64u Value);
          int64u    VariableGopDetection_Occurences_Get ();
          void      VariableGopDetection_GiveUp_Set (bool Value);
          bool      VariableGopDetection_GiveUp_Get ();
          void      InitDataNotRepeated_Occurences_Set (int64u Value);
          int64u    InitDataNotRepeated_Occurences_Get ();
          void      InitDataNotRepeated_GiveUp_Set (bool Value);
          bool      InitDataNotRepeated_GiveUp_Get ();
    #endif //MEDIAINFO_ADVANCED

          void      MpegTs_MaximumOffset_Set (int64u Value);
          int64u    MpegTs_MaximumOffset_Get ();
          void      MpegTs_MaximumScanDuration_Set (int64u Value);
          int64u    MpegTs_MaximumScanDuration_Get ();
          void      MpegTs_ForceStreamDisplay_Set (bool Value);
          bool      MpegTs_ForceStreamDisplay_Get ();
    #if MEDIAINFO_ADVANCED
          void      MpegTs_VbrDetection_Delta_Set (float64 Value);
          float64   MpegTs_VbrDetection_Delta_Get ();
          void      MpegTs_VbrDetection_Occurences_Set (int64u Value);
          int64u    MpegTs_VbrDetection_Occurences_Get ();
          void      MpegTs_VbrDetection_GiveUp_Set (bool Value);
          bool      MpegTs_VbrDetection_GiveUp_Get ();
    #endif //MEDIAINFO_ADVANCED

    ZtringListList  SubFile_Config_Get ();

    void            CustomMapping_Set (const Ztring &Value);
    Ztring          CustomMapping_Get (const Ztring &Format, const Ztring &Field);
    bool            CustomMapping_IsPresent (const Ztring &Format, const Ztring &Field);

          void      ErrorLog_Callback_Set(const Ztring &Value);
          void      ErrorLog(const Ztring &Value);

    #if MEDIAINFO_EVENTS
          bool     Event_CallBackFunction_IsSet ();
          Ztring   Event_CallBackFunction_Set (const Ztring &Value);
          Ztring   Event_CallBackFunction_Get ();
          void     Event_Send(const int8u* Data_Content, size_t Data_Size);
          void     Event_Send(const int8u* Data_Content, size_t Data_Size, const Ztring &File_Name);
          void     Log_Send(int8u Type, int8u Severity, int32u MessageCode, const Ztring &Message);
          void     Log_Send(int8u Type, int8u Severity, int32u MessageCode, const char* Message) {return Log_Send(Type, Severity, MessageCode, Ztring().From_Local(Message));}
    #endif //MEDIAINFO_EVENTS

    #if defined(MEDIAINFO_LIBCURL_YES)
          void      Ssh_PublicKeyFileName_Set (const Ztring &NewValue);
          Ztring    Ssh_PublicKeyFileName_Get ();
          void      Ssh_PrivateKeyFileName_Set (const Ztring &NewValue);
          Ztring    Ssh_PrivateKeyFileName_Get ();
          void      Ssh_IgnoreSecurity_Set (bool NewValue);
          void      Ssh_KnownHostsFileName_Set (const Ztring &NewValue);
          Ztring    Ssh_KnownHostsFileName_Get ();
          bool      Ssh_IgnoreSecurity_Get ();
          void      Ssl_CertificateFileName_Set (const Ztring &NewValue);
          Ztring    Ssl_CertificateFileName_Get ();
          void      Ssl_CertificateFormat_Set (const Ztring &NewValue);
          Ztring    Ssl_CertificateFormat_Get ();
          void      Ssl_PrivateKeyFileName_Set (const Ztring &NewValue);
          Ztring    Ssl_PrivateKeyFileName_Get ();
          void      Ssl_PrivateKeyFormat_Set (const Ztring &NewValue);
          Ztring    Ssl_PrivateKeyFormat_Get ();
          void      Ssl_CertificateAuthorityFileName_Set (const Ztring &NewValue);
          Ztring    Ssl_CertificateAuthorityFileName_Get ();
          void      Ssl_CertificateAuthorityPath_Set (const Ztring &NewValue);
          Ztring    Ssl_CertificateAuthorityPath_Get ();
          void      Ssl_CertificateRevocationListFileName_Set (const Ztring &NewValue);
          Ztring    Ssl_CertificateRevocationListFileName_Get ();
          void      Ssl_IgnoreSecurity_Set (bool NewValue);
          bool      Ssl_IgnoreSecurity_Get ();
    #endif //defined(MEDIAINFO_LIBCURL_YES)

private :
    int64u          FormatDetection_MaximumOffset;
    #if MEDIAINFO_ADVANCED
        int64u      VariableGopDetection_Occurences;
        bool        VariableGopDetection_GiveUp;
        int64u      InitDataNotRepeated_Occurences;
        bool        InitDataNotRepeated_GiveUp;
    #endif //MEDIAINFO_ADVANCED
    int64u          MpegTs_MaximumOffset;
    int64u          MpegTs_MaximumScanDuration;
    bool            MpegTs_ForceStreamDisplay;
    #if MEDIAINFO_ADVANCED
        float64     MpegTs_VbrDetection_Delta;
        int64u      MpegTs_VbrDetection_Occurences;
        bool        MpegTs_VbrDetection_GiveUp;
    #endif //MEDIAINFO_ADVANCED
    size_t          Complete;
    size_t          BlockMethod;
    size_t          Internet;
    size_t          MultipleValues;
    size_t          ParseUnknownExtensions;
    size_t          ShowFiles_Nothing;
    size_t          ShowFiles_VideoAudio;
    size_t          ShowFiles_VideoOnly;
    size_t          ShowFiles_AudioOnly;
    size_t          ShowFiles_TextOnly;
    float32         ParseSpeed;
    float32         Verbosity;
    float32         Trace_Level;
    bool            Trace_TimeSection_OnlyFirstOccurrence;
    std::bitset<32> Trace_Layers; //0-7: Container, 8: Stream
    std::map<Ztring, bool> Trace_Modificators; //If we want to add/remove some details
    bool            Language_Raw;
    bool            ReadByHuman;
    bool            LegacyStreamDisplay;
    bool            SkipBinaryData;
    int8u           Demux;
    Ztring          Version;
    Ztring          ColumnSeparator;
    Ztring          LineSeparator;
    Ztring          TagSeparator;
    Ztring          Quote;
    Ztring          DecimalPoint;
    Ztring          ThousandsPoint;
    Translation     Language; //ex. : "KB;Ko"
    ZtringListList  Custom_View; //Definition of "General", "Video", "Audio", "Text", "Other", "Image"
    ZtringListList  Custom_View_Replace; //ToReplace;ReplaceBy
    trace_Format    Trace_Format;

    InfoMap         Container;
    InfoMap         CodecID[InfoCodecID_Format_Max][Stream_Max];
    InfoMap         Format;
    InfoMap         Codec;
    InfoMap         Library[InfoLibrary_Format_Max];
    InfoMap         Iso639_1;
    InfoMap         Iso639_2;
    ZtringListList  Info[Stream_Max]; //General info

    ZtringListList  SubFile_Config;

    std::map<Ztring, std::map<Ztring, Ztring> > CustomMapping;

    ZenLib::CriticalSection CS;

    void      Language_Set (stream_t StreamKind);

    //Event
    #if MEDIAINFO_EVENTS
    MediaInfo_Event_CallBackFunction* Event_CallBackFunction; //void Event_Handler(unsigned char* Data_Content, size_t Data_Size, void* UserHandler)
    void*           Event_UserHandler;
    #endif //MEDIAINFO_EVENTS

    #if defined(MEDIAINFO_LIBCURL_YES)
          Ztring    Ssh_PublicKeyFileName;
          Ztring    Ssh_PrivateKeyFileName;
          Ztring    Ssh_KnownHostsFileName;
          bool      Ssh_IgnoreSecurity;
          Ztring    Ssl_CertificateFileName;
          Ztring    Ssl_CertificateFormat;
          Ztring    Ssl_PrivateKeyFileName;
          Ztring    Ssl_PrivateKeyFormat;
          Ztring    Ssl_CertificateAuthorityFileName;
          Ztring    Ssl_CertificateAuthorityPath;
          Ztring    Ssl_CertificateRevocationListFileName;
          bool      Ssl_IgnoreSecurity;
    #endif //defined(MEDIAINFO_LIBCURL_YES)

    //Constructor
    MediaInfo_Config (const MediaInfo_Config&);             // Prevent copy-construction
    MediaInfo_Config& operator=(const MediaInfo_Config&);   // Prevent assignment
};

extern MediaInfo_Config Config;

} //NameSpace

#endif
